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
        // Status
        if (col == 0) {
            if (object.fFailed)
                return QString("Failed");
            else
            if (object.fConnected)
                return QString("Connected");
            else
                return QString("Created");
        }
        // Hash blind
        if (col == 1) {
            return QString::fromStdString(object.hashBlind.ToString());
        }
        // Hash Block
        if (col == 2) {
            if (object.hashBlock.IsNull()) {
                return QString("Not connected");
            } else {
                return QString::fromStdString(object.hashBlock.ToString());
            }
        }
        // ntxn
        if (col == 3) {
            return QString::number(object.ntxn);
        }
        // Sidechain block height
        if (col == 4) {
            return QString::number(object.nSidechainHeight);
        }
        // Mainchain block height
        if (col == 5) {
            return QString::number(object.nMainchainHeight);
        }
        // Amount
        if (col == 6) {
            QString amount = BitcoinUnits::formatWithUnit(unit, object.amount, false,
                    BitcoinUnits::separatorAlways);
            return amount;
        }
        // txid
        if (col == 7) {
            return QString::fromStdString(object.txid.ToString());
        }
        break;
    }
    case Qt::TextAlignmentRole:
    {
        // Status
        if (col == 0) {
            return Qt::AlignLeft;
        }
        // Hash blind
        if (col == 1) {
            return Qt::AlignLeft;
        }
        // Hash Block
        if (col == 2) {
            return Qt::AlignLeft;
        }
        // ntxn
        if (col == 3) {
            return Qt::AlignRight;
        }
        // Sidechain block height
        if (col == 4) {
            return Qt::AlignRight;
        }
        // Mainchain block height
        if (col == 5) {
            return Qt::AlignRight;
        }
        // Amount
        if (col == 6) {
            return Qt::AlignRight;
        }
        // txid
        if (col == 7) {
            return Qt::AlignLeft;
        }
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
                return QString("Status");
            case 1:
                return QString("Blind Hash");
            case 2:
                return QString("SC Block Hash");
            case 3:
                return QString("Txns");
            case 4:
                return QString("SC Block");
            case 5:
                return QString("MC Block");
            case 6:
                return QString("Bid Amount");
            case 7:
                return QString("MC txid");
            }
        }
    }
    return QVariant();
}

void SidechainBMMTableModel::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
}

void SidechainBMMTableModel::AddAttempt(const BMMTableObject& object)
{
    // Add BMM attempt to model
    beginInsertRows(QModelIndex(), model.size(), model.size());
        model.append(object);
    endInsertRows();


    // When we add a new attempt, set the last 6 BMM requests to failed if
    // they weren't connected - they have expired.

    // Skip if there are less than 2 BMM requests
    if (model.size() < 2)
        return;

    int nLimit = 0;
    QList<BMMTableObject>::reverse_iterator rit = model.rbegin();
    rit++;
    for (; rit != model.rend(); rit++) {
        if (nLimit > 6)
            break;
        if (rit->fConnected)
            continue;

        nLimit++;

        // Update the failed bool to true & signal to update background colors
        rit->fFailed = true;

        QModelIndex topLeft = index(rit - model.rbegin(), 0);
        QModelIndex topRight = index(rit - model.rbegin(), columnCount() - 1);
        Q_EMIT QAbstractItemModel::dataChanged(topLeft, topRight, {Qt::BackgroundRole});
    }
}

void SidechainBMMTableModel::UpdateForConnected(const BMMTableObject& object)
{
    auto it = std::find_if(model.begin(), model.end(),
                           [ object ](BMMTableObject a) { return a.hashBlind == object.hashBlind; });

    if (it != model.end()) {
        // Update object & signal to update background colors
        it->fFailed = false;
        it->fConnected = true;
        it->hashBlock = object.hashBlock;

        QModelIndex topLeft = index(it - model.begin(), 0);
        QModelIndex topRight = index(it - model.begin(), columnCount() - 1);
        Q_EMIT QAbstractItemModel::dataChanged(topLeft, topRight, {Qt::BackgroundRole});
    }
}
