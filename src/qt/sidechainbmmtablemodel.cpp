// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/sidechainbmmtablemodel.h>

#include <QBrush>
#include <QColor>
#include <QTimer>

#include <qt/bitcoinunits.h>
#include <qt/optionsmodel.h>
#include <qt/walletmodel.h>

Q_DECLARE_METATYPE(BMMTableObject)

SidechainBMMTableModel::SidechainBMMTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

int SidechainBMMTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return model.size();
}

int SidechainBMMTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 8;
}

QVariant SidechainBMMTableModel::data(const QModelIndex &index, int role) const
{
    if (!walletModel)
        return false;

    if (!index.isValid())
        return false;

    int row = index.row();
    int col = index.column();

    BMMTableObject object = model.at(row);

    int unit = walletModel->getOptionsModel()->getDisplayUnit();

    switch (role) {
    case Qt::DisplayRole:
    {
        // Mainchain txid
        if (col == 0) {
            return QString::fromStdString(object.txid.ToString());
        }
        // Mainchain block height
        if (col == 1) {
            return QString::number(object.nMainchainHeight);
        }
        // Sidechain block height
        if (col == 2) {
            return QString::number(object.nSidechainHeight);
        }
        // ntxn
        if (col == 3) {
            return QString::number(object.ntxn);
        }
        // Total fees
        if (col == 4) {
            QString amount = BitcoinUnits::formatWithUnit(unit, object.amountTotalFees, false,
                    BitcoinUnits::separatorAlways);
            return amount;
        }
        // BMM Amount (mainchain bribe)
        if (col == 5) {
            QString amount = BitcoinUnits::formatWithMainchainUnit(unit, object.amount, false,
                    BitcoinUnits::separatorAlways);
            return amount;
        }
        // Profit
        if (col == 6) {
            CAmount profit = object.amountTotalFees - object.amount;
            QString amount = BitcoinUnits::formatWithUnit(unit, profit, false,
                    BitcoinUnits::separatorAlways);
            return amount;
        }
        // Status
        if (col == 7) {
            if (object.fFailed)
                return QString("Failed");
            else
            if (object.fConnected)
                return QString("Success");
            else
                return QString("Trying...");
        }
        break;
    }
    case Qt::TextAlignmentRole:
    {
        // Mainchain txid
        if (col == 0) {
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        }
        // Mainchain block height
        if (col == 1) {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        }
        // Sidechain block height
        if (col == 2) {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        }
        // ntxn
        if (col == 3) {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        }
        // Total fees
        if (col == 4) {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        }
        // BMM amount
        if (col == 5) {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        }
        // Profit
        if (col == 6) {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        }
        // Status
        if (col == 7) {
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        }
        break;
    }
    }
    return QVariant();
}

QVariant SidechainBMMTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return QString("MC txid");
            case 1:
                return QString("MC Block");
            case 2:
                return QString("SC Block");
            case 3:
                return QString("Txns");
            case 4:
                return QString("Fees");
            case 5:
                return QString("Bid Amount");
            case 6:
                return QString("Profit");
            case 7:
                return QString("Status");
            }
        }
    }
    return QVariant();
}

Qt::ItemFlags SidechainBMMTableModel::flags (const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    BMMTableObject object = model.at(index.row());
    if (object.fFailed)
        return Qt::NoItemFlags;
    else
        return Qt::ItemIsEnabled;
}

void SidechainBMMTableModel::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
}

void SidechainBMMTableModel::AddAttempt(const BMMTableObject& object)
{
    // Add BMM attempt to model
    beginInsertRows(QModelIndex(), model.size(), model.size());
        model.prepend(object);
    endInsertRows();


    // When we add a new attempt, set the last 6 BMM requests to failed if
    // they weren't connected - they have expired.

    // Skip if there are less than 2 BMM requests
    if (model.size() < 2)
        return;

    int nLimit = 0;
    QList<BMMTableObject>::iterator it = model.begin();
    it++; // Skip the first (newest) entry
    for (; it != model.end(); it++) {
        if (nLimit > 6)
            break;
        if (it->fConnected)
            continue;

        nLimit++;

        // Update the failed bool to true & signal to update background colors
        it->fFailed = true;

        QModelIndex topLeft = index(it - model.begin(), 0);
        QModelIndex topRight = index(it - model.begin(), columnCount() - 1);
        Q_EMIT QAbstractItemModel::dataChanged(topLeft, topRight, {Qt::BackgroundRole});
    }
}

void SidechainBMMTableModel::UpdateForConnected(const uint256& hashMerkleRoot)
{
    auto it = std::find_if(model.begin(), model.end(),
                           [ hashMerkleRoot ](BMMTableObject a) { return a.hashMerkleRoot == hashMerkleRoot; });

    if (it != model.end()) {
        // Update object & signal to update background colors
        it->fFailed = false;
        it->fConnected = true;

        QModelIndex topLeft = index(it - model.begin(), 0);
        QModelIndex topRight = index(it - model.begin(), columnCount() - 1);
        Q_EMIT QAbstractItemModel::dataChanged(topLeft, topRight, {Qt::BackgroundRole});
    }
}
