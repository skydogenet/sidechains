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
    int ntxn = 0;
    uint256 hashBlind;
    uint256 hashBlock;
    CAmount amount;
    uint256 txid;
    int nSidechainHeight = 0;
    int nMainchainHeight = 0;

    bool fConnected = false;
    bool fFailed = false;
};

enum ColumnWidth
{
    COLUMN_STATUS = 80,
    COLUMN_BMM_BLIND = 100,
    COLUMN_BMM_BLOCK = 120,
    COLUMN_TXNS = 45,
    COLUMN_SIDECHAIN_HEIGHT = 80,
    COLUMN_MAINCHAIN_HEIGHT = 80,
    COLUMN_BMM_AMOUNT = 130,
    COLUMN_BMM_TXID = 100,
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
