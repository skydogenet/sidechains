// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/sidechainwithdrawalbundlehistorytablemodel.h>

#include <QTimer>

#include <qt/bitcoinunits.h>
#include <qt/clientmodel.h>
#include <qt/guiconstants.h>
#include <qt/optionsmodel.h>
#include <qt/walletmodel.h>

#include <sidechain.h>
#include <txdb.h>
#include <validation.h>

Q_DECLARE_METATYPE(WithdrawalBundleHistoryTableObject)

SidechainWithdrawalBundleHistoryTableModel::SidechainWithdrawalBundleHistoryTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

int SidechainWithdrawalBundleHistoryTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return model.size();
}

int SidechainWithdrawalBundleHistoryTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 4;
}

QVariant SidechainWithdrawalBundleHistoryTableModel::data(const QModelIndex &index, int role) const
{
    if (!walletModel)
        return false;

    if (!index.isValid())
        return false;

    int row = index.row();
    int col = index.column();

    if (!model.at(row).canConvert<WithdrawalBundleHistoryTableObject>())
        return QVariant();

    WithdrawalBundleHistoryTableObject object = model.at(row).value<WithdrawalBundleHistoryTableObject>();

    int unit = walletModel->getOptionsModel()->getDisplayUnit();

    switch (role) {
    case Qt::DisplayRole:
    {
        // Height
        if (col == 0) {
            return QString::number(object.height);
        }
        // Hash
        if (col == 1) {
            return object.hash;
        }
        // Total Withdrawn
        if (col == 2) {

            QString amount = BitcoinUnits::formatWithUnit(unit, object.amount, false,
                    BitcoinUnits::separatorAlways);
            return amount;
        }
        // Status
        if (col == 3) {
            return object.status;
        }
    }
    case Qt::TextAlignmentRole:
    {
        // Height
        if (col == 0) {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        }
        // Hash
        if (col == 1) {
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        }
        // Total Withdrawn
        if (col == 2) {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        }
        // Status
        if (col == 3) {
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }
    }
    return QVariant();
}

QVariant SidechainWithdrawalBundleHistoryTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return QString("Sidechain block #");
            case 1:
                return QString("Hash");
            case 2:
                return QString("Amount");
            case 3:
                return QString("Status");
            }
        }
    }
    return QVariant();
}

void SidechainWithdrawalBundleHistoryTableModel::UpdateModel()
{
    beginResetModel();
    model.clear();
    endResetModel();

    // Get all of the current WithdrawalBundle(s)
    std::vector<SidechainWithdrawalBundle> vWithdrawalBundle;
    vWithdrawalBundle = psidechaintree->GetWithdrawalBundles(THIS_SIDECHAIN);

    if (vWithdrawalBundle.empty())
        return;

    // Sort WithdrawalBundle(s) by height
    SortWithdrawalBundleByHeight(vWithdrawalBundle);

    // Add WithdrawalBundle(s) to model
    beginInsertRows(QModelIndex(), model.size(), model.size() + vWithdrawalBundle.size() - 1);
    for (const SidechainWithdrawalBundle& wt : vWithdrawalBundle) {
        WithdrawalBundleHistoryTableObject object;

        // Insert new WithdrawalBundle into table
        object.hash = QString::fromStdString(wt.tx.GetHash().ToString());
        object.amount = CTransaction(wt.tx).GetValueOut();
        object.status = QString::fromStdString(wt.GetStatusStr());
        object.height = wt.nHeight;
        model.append(QVariant::fromValue(object));
    }
    endInsertRows();
}

bool SidechainWithdrawalBundleHistoryTableModel::GetWithdrawalBundleInfoAtRow(int row, uint256& hash) const
{
    if (row >= model.size())
        return false;

    if (!model[row].canConvert<WithdrawalBundleHistoryTableObject>())
        return false;

    WithdrawalBundleHistoryTableObject object = model[row].value<WithdrawalBundleHistoryTableObject>();

    hash = uint256S(object.hash.toStdString());

    return true;
}

void SidechainWithdrawalBundleHistoryTableModel::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
}

void SidechainWithdrawalBundleHistoryTableModel::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if (model)
    {
        connect(model, SIGNAL(numBlocksChanged(int, QDateTime, double, bool)),
                this, SLOT(UpdateModel()));
    }
}
