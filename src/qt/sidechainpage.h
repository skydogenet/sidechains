// Copyright (c) 2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SIDECHAINPAGE_H
#define SIDECHAINPAGE_H

#include <qt/sidechainhistorytablemodel.h>

#include <amount.h>

#include <string>
#include <uint256.h>

QT_BEGIN_NAMESPACE
#include <QTableView>
#include <QTimer>
#include <QWidget>
QT_END_NAMESPACE

class CBlock;
class WalletModel;
class QMessageBox;
class ConfGeneratorDialog;

namespace Ui {
class SidechainPage;
}

class SidechainPage : public QWidget
{
    Q_OBJECT

public:
    explicit SidechainPage(QWidget *parent = 0);
    ~SidechainPage();

    void generateQR(std::string data);

    void setWalletModel(WalletModel *model);

public Q_SLOTS:
    void setBalance(const CAmount& balance, const CAmount& unconfirmedBalance,
                    const CAmount& immatureBalance, const CAmount& watchOnlyBalance,
                    const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance);

    void RefreshTrain();

private Q_SLOTS:
    void on_pushButtonMainchain_clicked();

    void on_pushButtonSidechain_clicked();

    void on_pushButtonCopy_clicked();

    void on_pushButtonNew_clicked();

    void on_pushButtonWT_clicked();

    void on_addressBookButton_clicked();

    void on_pasteButton_clicked();

    void on_deleteButton_clicked();

    void on_pushButtonCreateBlock_clicked();

    void on_pushButtonSendCriticalRequest_clicked();

    void on_checkBoxEnableAutomation_toggled(bool fChecked);

    void on_pushButtonSubmitBlock_clicked();

    void on_spinBoxRefreshInterval_valueChanged(int n);

    void RefreshBMM();

    void on_pushButtonConfigureBMM_clicked();

    void ResetTrainWarningSleep();

    void ShowRestartPage();

    void on_pushButtonRetryConnection_clicked();

private:
    Ui::SidechainPage *ui;

    WalletModel *walletModel;

    QTableView *incomingTableView;
    QTableView *outgoingTableView;

    SidechainHistoryTableModel *incomingTableModel;
    SidechainHistoryTableModel *outgoingTableModel;

    ConfGeneratorDialog *confGeneratorDialog;

    QTimer *bmmTimer;
    QTimer *trainTimer;
    QTimer *trainRetryTimer;
    QTimer *trainWarningSleepTimer;

    bool fSleepTrainWarning;

    QMessageBox *trainErrorMessageBox;

    bool validateWTAmount();

    void generateAddress();

    bool CreateBMMBlock(CBlock& block, QString error = "");

    uint256 SendBMMRequest(const uint256& hashBMM, const uint256& hashBlockMain);

    bool SubmitBMMBlock(const CBlock& block);

    // This function checks if the user has any optional verification settings
    // enabled which require a connection to the mainchain and disables
    // networking if fMainchainConnected is set to false. If fMainchainConnected
    // is set to true - re enable networking.
    void UpdateNetworkActive(bool fMainchainConnected);

    // Check if configuration files are setup and connection works
    void CheckConfiguration(bool& fConfig, bool& fConnection);
};

#endif // SIDECHAINPAGE_H
