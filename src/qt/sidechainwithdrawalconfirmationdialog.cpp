// Copyright (c) 2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/sidechainwithdrawalconfirmationdialog.h>
#include <qt/forms/ui_sidechainwithdrawalconfirmationdialog.h>

#include <utilmoneystr.h>

SidechainWithdrawalConfirmationDialog::SidechainWithdrawalConfirmationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SidechainWithdrawalConfirmationDialog)
{
    ui->setupUi(this);
    Reset();
}

SidechainWithdrawalConfirmationDialog::~SidechainWithdrawalConfirmationDialog()
{
    delete ui;
}

bool SidechainWithdrawalConfirmationDialog::GetConfirmed()
{
    // Return the confirmation status and reset dialog
    if (fConfirmed) {
        Reset();
        return true;
    } else {
        Reset();
        return false;
    }
}

void SidechainWithdrawalConfirmationDialog::SetInfo(const QString& strWTAmount, const QString& strFee,
                                            const QString& strMcFee, const QString& strDest,
                                            const QString& strRefundDest)
{
    ui->labelWTAmount->setText(strWTAmount);
    ui->labelFee->setText(strFee);
    ui->labelMcFee->setText(strMcFee);
    ui->labelDest->setText(strDest);
    ui->labelRefundDest->setText(strRefundDest);
}

void SidechainWithdrawalConfirmationDialog::Reset()
{
    // Reset the dialog's confirmation status
    fConfirmed = false;
}

void SidechainWithdrawalConfirmationDialog::on_buttonBox_accepted()
{
    fConfirmed = true;
    this->close();
}

void SidechainWithdrawalConfirmationDialog::on_buttonBox_rejected()
{
    this->close();
}

