// Copyright (c) 2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/sidechainwtconfirmationdialog.h>
#include <qt/forms/ui_sidechainwtconfirmationdialog.h>

#include <utilmoneystr.h>

SidechainWTConfirmationDialog::SidechainWTConfirmationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SidechainWTConfirmationDialog)
{
    ui->setupUi(this);
    Reset();
}

SidechainWTConfirmationDialog::~SidechainWTConfirmationDialog()
{
    delete ui;
}

bool SidechainWTConfirmationDialog::GetConfirmed()
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

void SidechainWTConfirmationDialog::SetInfo(const QString& strWTAmount, const QString& strFee,
                                            const QString& strMcFee, const QString& strDest,
                                            const QString& strRefundDest)
{
    ui->labelWTAmount->setText(strWTAmount);
    ui->labelFee->setText(strFee);
    ui->labelMcFee->setText(strMcFee);
    ui->labelDest->setText(strDest);
    ui->labelRefundDest->setText(strRefundDest);
}

void SidechainWTConfirmationDialog::Reset()
{
    // Reset the dialog's confirmation status
    fConfirmed = false;
}

void SidechainWTConfirmationDialog::on_buttonBox_accepted()
{
    fConfirmed = true;
    this->close();
}

void SidechainWTConfirmationDialog::on_buttonBox_rejected()
{
    this->close();
}

