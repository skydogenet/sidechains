// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/sidechainwttablemodel.h>

#include <QBrush>
#include <QColor>
#include <QTimer>

#include <qt/bitcoinunits.h>
#include <qt/clientmodel.h>
#include <qt/guiconstants.h>
#include <qt/optionsmodel.h>
#include <qt/walletmodel.h>

#include <policy/wtprime.h>
#include <consensus/validation.h>
#include <sidechain.h>
#include <txdb.h>
#include <validation.h>

Q_DECLARE_METATYPE(WTTableObject)

SidechainWTTableModel::SidechainWTTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

int SidechainWTTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return model.size();
}

int SidechainWTTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 4;
}

QVariant SidechainWTTableModel::data(const QModelIndex &index, int role) const
{
    if (!walletModel)
        return false;

    if (!index.isValid())
        return false;

    int row = index.row();
    int col = index.column();

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
        // Cumulative size of WT^ up to this WT output being added
        if (col == 3) {
            QString weight;
            weight += QString::number(object.nCumulativeWeight);
            weight += " / ";
            weight += QString::number(MAX_WTPRIME_WEIGHT);
            return weight;
        }

        break;
    }
    case Qt::BackgroundRole:
    {
        // Highlight WT(s) with cumulative weight > MAX_WTPRIME_WEIGHT to
        // indicate that they are not going to be included in the next WT^
        if (object.nCumulativeWeight > MAX_WTPRIME_WEIGHT) {
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
        // Cumulative size of WT^ up to this WT output being added
        if (col == 3) {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        }
    }
    }
    return QVariant();
}

QVariant SidechainWTTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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
                return QString("Cumulative WT^ Weight");
            }
        }
    }
    return QVariant();
}

void SidechainWTTableModel::UpdateModel()
{
    beginResetModel();
    model.clear();
    endResetModel();

    std::vector<SidechainWT> vWT;
    vWT = psidechaintree->GetWTs(SIDECHAIN_TEST);

    SelectUnspentWT(vWT);

    if (vWT.empty())
        return;

    SortWTByFee(vWT);

    // Create a fake WT^ transaction so that we can estimate the total size of
    // the WT^. WT(s) in the table after the cumulative size is too large will
    // be highlighted.
    // Add a dummy output for mainchain fee encoding (updated later)
    CMutableTransaction wjtx;
    wjtx.vout.push_back(CTxOut(0, CScript() << OP_RETURN << CScriptNum(1LL << 40)));
    wjtx.nVersion = 2;
    wjtx.vin.resize(1); // Dummy vin for serialization...
    wjtx.vin[0].scriptSig = CScript() << OP_0;

    // Add WT(s) to model & to the fake WT^ transaction to estimate size
    beginInsertRows(QModelIndex(), model.size(), model.size() + vWT.size() - 1);
    for (const SidechainWT& wt : vWT) {
        // Add wt output to fake WT^ and calculate size estimate as well as
        // estimate which WTs will be included in the next WT^
        CTxDestination dest = DecodeDestination(wt.strDestination, true /* fMainchain */);
        wjtx.vout.push_back(CTxOut(wt.amount, GetScriptForDestination(dest)));

        WTTableObject object;

        // Insert new WT into table
        object.id = wt.GetID();
        object.amount = wt.amount;
        object.amountMainchainFee = wt.mainchainFee;
        object.destination = QString::fromStdString(wt.strDestination);
        object.nCumulativeWeight = GetTransactionWeight(wjtx);
        model.append(QVariant::fromValue(object));
    }
    endInsertRows();
}

void SidechainWTTableModel::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
}

void SidechainWTTableModel::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if (model)
    {
        connect(model, SIGNAL(numBlocksChanged(int, QDateTime, double, bool)),
                this, SLOT(UpdateModel()));

        UpdateModel();
    }
}
