// Copyright (c) 2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SIDECHAINPAGE_H
#define SIDECHAINPAGE_H

#include <amount.h>

#include <string>
#include <uint256.h>

#include <QTableView>
#include <QTimer>
#include <QWidget>

class CBlock;
class ClientModel;
class ConfGeneratorDialog;
class PlatformStyle;
class SidechainBMMTableModel;
class SidechainWithdrawalConfirmationDialog;
class SidechainWithdrawalBundleHistoryDialog;
class SidechainWithdrawalTableModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QMenu;
class QMessageBox;
QT_END_NAMESPACE

namespace Ui {
class SidechainPage;
}

class SidechainPage : public QWidget
{
    Q_OBJECT

public:
    explicit SidechainPage(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~SidechainPage();

    void generateQR(std::string data);

    void setWalletModel(WalletModel *model);

    void setClientModel(ClientModel *model);

public Q_SLOTS:
    void setBalance(const CAmount& balance, const CAmount& unconfirmedBalance,
                    const CAmount& immatureBalance, const CAmount& watchOnlyBalance,
                    const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance);

    void setNumBlocks(const int nBlocks);

private Q_SLOTS:
    void on_pushButtonCopy_clicked();

    void on_pushButtonNew_clicked();

    void on_pushButtonWithdraw_clicked();

    void on_addressBookButton_clicked();

    void on_pasteButton_clicked();

    void on_deleteButton_clicked();

    void on_spinBoxRefreshInterval_valueChanged(int n);

    void RefreshBMM();

    void on_pushButtonConfigureMainchainConnection_clicked();

    void ShowRestartPage();

    void on_pushButtonRetryConnection_clicked();

    void on_pushButtonShowPastWithdrawalBundles_clicked();

    void on_checkBoxAutoWithdrawalBundleRefresh_changed(int state);

    void on_withdrawalBundle_doubleClicked(uint256 hashWithdrawalBundle);

    void on_lineEditWithdrawalBundleHash_returnPressed();

    void UpdateWTTotal();

    void on_pushButtonWTHelp_clicked();

    void on_pushButtonStartBMM_clicked();

    void on_pushButtonStopBMM_clicked();

    void updateDisplayUnit();

    void on_pushButtonNewBMM_clicked();

    void on_checkBoxOnlyMyWithdrawals_toggled(bool fChecked);

    void WTContextMenu(const QPoint& point);

    void CopyWithdrawalID();

    void RequestRefund();

    void on_pushButtonManualBMM_clicked();

    void CheckConnection();

private:
    Ui::SidechainPage *ui;

    WalletModel *walletModel = nullptr;
    ClientModel *clientModel = nullptr;

    const PlatformStyle *platformStyle;

    ConfGeneratorDialog *confGeneratorDialog = nullptr;
    SidechainBMMTableModel *bmmModel = nullptr;
    SidechainWithdrawalConfirmationDialog *wtConfDialog = nullptr;
    SidechainWithdrawalBundleHistoryDialog *withdrawalBundleHistoryDialog = nullptr;
    SidechainWithdrawalTableModel *unspentWTModel = nullptr;

    QMessageBox* connectionErrorMessage = nullptr;

    QAction *withdrawalRefundAction;
    QAction *copyWithdrawalIDAction;
    QMenu *wtContextMenu;

    QTimer *bmmTimer;
    QTimer *connectionCheckTimer;

    int nBlocks;

    bool validateWTAmount();

    bool validateFeeAmount();

    bool validateMainchainFeeAmount();

    std::string GenerateAddress(const std::string& strLabel = "");

    // Handle networking being enabled / disabled
    void UpdateNetworkActive(bool fMainchainConnected);

    // Check if configuration files are setup and connection works
    void CheckConfiguration(bool& fConfig, bool& fConnection);

    void ClearWithdrawalBundleExplorer();

    void UpdateSidechainWealth();

    void UpdateToLatestWithdrawalBundle(bool fRequested = true);

    void SetCurrentWithdrawalBundle(const std::string& strHash, bool fRequested = true);

    void StartBMM();

    void StopBMM();

Q_SIGNALS:
    void OnlyMyWithdrawalsToggled(bool fChecked);
    void WithdrawalBundleBannerUpdate(QString str);
};

#endif // SIDECHAINPAGE_H
