// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_SIDECHAINWITHDRAWAL_BUNDLEHISTORYTABLEMODEL_H
#define BITCOIN_SIDECHAINWITHDRAWAL_BUNDLEHISTORYTABLEMODEL_H

#include <amount.h>
#include <uint256.h>

#include <QAbstractTableModel>
#include <QList>

class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

struct WithdrawalBundleHistoryTableObject
{
    QString hash;
    CAmount amount;
    QString status;
    int height;
};

class SidechainWithdrawalBundleHistoryTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit SidechainWithdrawalBundleHistoryTableModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool GetWithdrawalBundleInfoAtRow(int row, uint256& hash) const;

    void setWalletModel(WalletModel *model);
    void setClientModel(ClientModel *model);

public Q_SLOTS:
    void UpdateModel();

private:
    QList<QVariant> model;
    QTimer *pollTimer;

    WalletModel *walletModel;
    ClientModel *clientModel;
};

#endif // BITCOIN_SIDECHAINWITHDRAWAL_BUNDLEHISTORYTABLEMODEL_H
