// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_SIDECHAINWTPRIMEHISTORYDIALOG_H
#define BITCOIN_SIDECHAINWTPRIMEHISTORYDIALOG_H

#include <QDialog>

class SidechainWTPrimeHistoryTableModel;
class ClientModel;
class uint256;
class WalletModel;

namespace Ui {
class SidechainWTPrimeHistoryDialog;
}

class SidechainWTPrimeHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SidechainWTPrimeHistoryDialog(QWidget *parent = 0);
    ~SidechainWTPrimeHistoryDialog();

    void setWalletModel(WalletModel *model);
    void setClientModel(ClientModel *model);

private:
    Ui::SidechainWTPrimeHistoryDialog *ui;

    WalletModel *walletModel;
    ClientModel *clientModel;

    SidechainWTPrimeHistoryTableModel *wtPrimeHistoryModel = nullptr;

private Q_SLOTS:
    void on_tableView_doubleClicked(const QModelIndex& index);

Q_SIGNALS:
    void doubleClickedWTPrime(const uint256& hashWTPrime);
};

#endif // BITCOIN_SIDECHAINWTPRIMEHISTORYDIALOG_H
