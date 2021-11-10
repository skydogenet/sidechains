// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_SIDECHAINWITHDRAWAL_BUNDLEHISTORYDIALOG_H
#define BITCOIN_SIDECHAINWITHDRAWAL_BUNDLEHISTORYDIALOG_H

#include <QDialog>

#include <uint256.h>

class SidechainWithdrawalBundleHistoryTableModel;
class ClientModel;
class WalletModel;

namespace Ui {
class SidechainWithdrawalBundleHistoryDialog;
}

class SidechainWithdrawalBundleHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SidechainWithdrawalBundleHistoryDialog(QWidget *parent = 0);
    ~SidechainWithdrawalBundleHistoryDialog();

    void setWalletModel(WalletModel *model);
    void setClientModel(ClientModel *model);

private:
    Ui::SidechainWithdrawalBundleHistoryDialog *ui;

    WalletModel *walletModel;
    ClientModel *clientModel;

    SidechainWithdrawalBundleHistoryTableModel *withdrawalBundleHistoryModel = nullptr;

private Q_SLOTS:
    void on_tableView_doubleClicked(const QModelIndex& index);

Q_SIGNALS:
    void doubleClickedWithdrawalBundle(uint256 hashWithdrawalBundle);
};

#endif // BITCOIN_SIDECHAINWITHDRAWAL_BUNDLEHISTORYDIALOG_H
