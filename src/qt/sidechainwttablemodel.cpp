// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/sidechainwttablemodel.h>

#include <QTimer>

#include <qt/bitcoinunits.h>
#include <qt/clientmodel.h>
#include <qt/guiconstants.h>
#include <qt/optionsmodel.h>
#include <qt/walletmodel.h>

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
    return 3;
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
            QString fee = BitcoinUnits::formatWithUnit(unit, object.amountMainchainFee, false,
                    BitcoinUnits::separatorAlways);
            return fee;
        }
        // Destination address
        if (col == 2) {
            return object.destination;
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
    SortWTByFee(vWT);

    if (vWT.empty())
        return;

    // Add WT(s) to model
    beginInsertRows(QModelIndex(), model.size(), model.size() + vWT.size() - 1);
    for (const SidechainWT& wt : vWT) {
        WTTableObject object;

        // Insert new WT into table
        object.id = wt.GetID();
        object.amount = wt.amount;
        object.amountMainchainFee = wt.mainchainFee;
        object.destination = QString::fromStdString(wt.strDestination);
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

