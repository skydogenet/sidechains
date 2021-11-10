// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/sidechainwithdrawaltablemodel.h>

#include <QBrush>
#include <QColor>
#include <QTimer>

#include <qt/bitcoinunits.h>
#include <qt/clientmodel.h>
#include <qt/guiconstants.h>
#include <qt/optionsmodel.h>
#include <qt/walletmodel.h>

#include <bmmcache.h>
#include <consensus/validation.h>
#include <policy/withdrawalbundle.h>
#include <sidechain.h>
#include <txdb.h>
#include <validation.h>

Q_DECLARE_METATYPE(WTTableObject)

SidechainWithdrawalTableModel::SidechainWithdrawalTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    fOnlyMyWithdrawals = false;

    connect(parent, SIGNAL(OnlyMyWithdrawalsToggled(bool)), this, SLOT(SetOnlyMyWithdrawals(bool)));
}

int SidechainWithdrawalTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return model.size();
}

int SidechainWithdrawalTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 4;
}

QVariant SidechainWithdrawalTableModel::data(const QModelIndex &index, int role) const
{
    if (!walletModel)
        return false;

    if (!index.isValid())
        return false;

    int row = index.row();
    int col = index.column();

    // Double check that the data pointed at by the index still exists, it is
    // possible for a Withdrawalto be removed from the model when a block is connected.
    if (row >= model.size())
        return QVariant();

    if (!model.at(row).canConvert<WTTableObject>())
        return QVariant();

    WTTableObject object = model.at(row).value<WTTableObject>();

    int unit = walletModel->getOptionsModel()->getDisplayUnit();

    switch (role) {
    case Qt::DisplayRole:
    {
        // Amount
        if (col == 0) {
            QString amount = BitcoinUnits::formatWithUnit(unit, object.amount - object.amountMainchainFee, false,
                    BitcoinUnits::separatorAlways);
            return amount;
        }
        // Mainchain fee amount
        if (col == 1) {
            QString fee = BitcoinUnits::formatWithMainchainUnit(unit, object.amountMainchainFee, false,
                    BitcoinUnits::separatorAlways);
            return fee;
        }
        // Destination address
        if (col == 2) {
            return object.destination;
        }
        // Cumulative size of WithdrawalBundle up to this Withdrawaloutput being added
        if (col == 3) {
            QString weight;
            weight += QString::number(object.nCumulativeWeight);
            weight += " / ";
            weight += QString::number(MAX_WITHDRAWAL_BUNDLE_WEIGHT);
            return weight;
        }

        break;
    }
    case Qt::BackgroundRole:
    {
        // Highlight WT(s) with cumulative weight > MAX_WITHDRAWAL_BUNDLE_WEIGHT
        // to indicate that they are not going to be included in the next bundle
        if (object.nCumulativeWeight > MAX_WITHDRAWAL_BUNDLE_WEIGHT) {
            // Semi-transparent red
            return QBrush(QColor(255, 40, 0, 180));
        }
        break;
    }
    case Qt::TextAlignmentRole:
    {
        // Amount
        if (col == 0) {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        }
        // Mainchain fee amount
        if (col == 1) {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        }
        // Destination address
        if (col == 2) {
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        }
        // Cumulative size of WithdrawalBundle up to this Withdrawaloutput being added
        if (col == 3) {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        }
    }
    case WithdrawalIDRole:
        return QString::fromStdString(object.id.ToString());
    case IsMineRole:
        return object.fMine;
    }
    return QVariant();
}

QVariant SidechainWithdrawalTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return QString("Amount");
            case 1:
                return QString("Mainchain Fee");
            case 2:
                return QString("Destination");
            case 3:
                return QString("Cumulative WithdrawalBundle Weight");
            }
        }
    }
    return QVariant();
}

void SidechainWithdrawalTableModel::UpdateModel()
{
    beginResetModel();
    model.clear();
    endResetModel();

    std::vector<SidechainWithdrawal> vWT;
    vWT = psidechaintree->GetWithdrawals(THIS_SIDECHAIN);

    SelectUnspentWithdrawal(vWT);

    if (vWT.empty())
        return;

    SortWithdrawalByFee(vWT);

    // Create a fake WithdrawalBundle transaction so that we can estimate the total size of
    // the WithdrawalBundle. WT(s) in the table after the cumulative size is too large will
    // be highlighted.
    CMutableTransaction wjtx;
    // Add SIDECHAIN_WITHDRAWAL_RETURN_DEST output
    wjtx.vout.push_back(CTxOut(0, CScript() << OP_RETURN << ParseHex(HexStr(SIDECHAIN_WITHDRAWAL_BUNDLE_RETURN_DEST))));
    // Add a dummy output for mainchain fee encoding
    wjtx.vout.push_back(CTxOut(0, CScript() << OP_RETURN << CScriptNum(1LL << 40)));
    wjtx.nVersion = 2;
    wjtx.vin.resize(1); // Dummy vin for serialization...
    wjtx.vin[0].scriptSig = CScript() << OP_0;

    // Add WT(s) to model & to the fake WithdrawalBundle transaction to estimate size
    //
    // Loop through WTs and calculate TX size, copy WTs that should be displayed
    // (based on fOnlyMyWithdrawals) into vector.
    std::vector<WTTableObject> vWTDisplay;
    for (const SidechainWithdrawal& wt : vWT) {
        // Add wt output to fake WithdrawalBundle and calculate size estimate as well as
        // estimate which WTs will be included in the next WithdrawalBundle
        CTxDestination dest = DecodeDestination(wt.strDestination, true /* fMainchain */);
        wjtx.vout.push_back(CTxOut(wt.amount, GetScriptForDestination(dest)));

        // Check if the Withdrawalis mine
        bool fMine = bmmCache.IsMyWT(wt.GetID());
        if (!fMine && fOnlyMyWithdrawals)
            continue;

        WTTableObject object;
        object.id = wt.GetID();
        object.amount = wt.amount;
        object.amountMainchainFee = wt.mainchainFee;
        object.destination = QString::fromStdString(wt.strDestination);
        object.nCumulativeWeight = GetTransactionWeight(wjtx);
        object.fMine = fMine;

        vWTDisplay.push_back(object);
    }

    beginInsertRows(QModelIndex(), model.size(), model.size() + vWTDisplay.size() - 1);
    for (const WTTableObject& wt : vWTDisplay)
        model.append(QVariant::fromValue(wt));
    endInsertRows();
}

void SidechainWithdrawalTableModel::SetOnlyMyWithdrawals(bool fChecked)
{
    fOnlyMyWithdrawals = fChecked;
    UpdateModel();
}

void SidechainWithdrawalTableModel::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
}

void SidechainWithdrawalTableModel::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if (model)
    {
        connect(model, SIGNAL(numBlocksChanged(int, QDateTime, double, bool)),
                this, SLOT(UpdateModel()));

        UpdateModel();
    }
}
