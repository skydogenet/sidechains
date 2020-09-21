// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SIDECHAINBMMTABLEMODEL_H
#define SIDECHAINBMMTABLEMODEL_H

#include <QAbstractTableModel>

#include <amount.h>
#include <uint256.h>

class WalletModel;

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

struct BMMTableObject
{
    // Info to be displayed on table
    int ntxn = 0;
    CAmount amount;
    CAmount amountTotalFees;
    uint256 txid;
    int nSidechainHeight = 0;
    int nMainchainHeight = 0;

    // Status
    bool fConnected = false;
    bool fFailed = false;

    // Not currently displayed on table
    uint256 hashBlind;
    uint256 hashBlock;
};

enum ColumnWidth
{
    COLUMN_BMM_TXID = 100,
    COLUMN_MAINCHAIN_HEIGHT = 80,
    COLUMN_SIDECHAIN_HEIGHT = 80,
    COLUMN_TXNS = 45,
    COLUMN_FEES = 130,
    COLUMN_BMM_AMOUNT = 130,
    COLUMN_PROFIT = 130,
    COLUMN_STATUS = 80,
};

class SidechainBMMTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit SidechainBMMTableModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void setWalletModel(WalletModel *model);

    void AddAttempt(const BMMTableObject& object);
    void UpdateForConnected(const BMMTableObject& object);

private:
    QList<BMMTableObject> model;

    WalletModel *walletModel = nullptr;
};

#endif // SIDECHAINBMMTABLEMODEL_H
