// Copyright (c) 2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SIDECHAIN_WITHDRAWAL_BUNDLECONFIRMATIONDIALOG_H
#define SIDECHAIN_WITHDRAWAL_BUNDLECONFIRMATIONDIALOG_H

#include <QDialog>

#include <amount.h>

namespace Ui {
class SidechainWithdrawalConfirmationDialog;
}

class SidechainWithdrawalConfirmationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SidechainWithdrawalConfirmationDialog(QWidget *parent = nullptr);
    ~SidechainWithdrawalConfirmationDialog();

    bool GetConfirmed();
    void SetInfo(const QString& strWTAmount, const QString& strFee,
                 const QString& strMcFee, const QString& strDest,
                 const QString& strRefundDest);

public Q_SLOTS:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::SidechainWithdrawalConfirmationDialog *ui;

    void Reset();
    bool fConfirmed = false;

};

#endif // SIDECHAIN_WITHDRAWAL_BUNDLECONFIRMATIONDIALOG_H
