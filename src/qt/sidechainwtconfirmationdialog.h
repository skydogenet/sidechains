// Copyright (c) 2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SIDECHAINWTCONFIRMATIONDIALOG_H
#define SIDECHAINWTCONFIRMATIONDIALOG_H

#include <QDialog>

#include <amount.h>

namespace Ui {
class SidechainWTConfirmationDialog;
}

class SidechainWTConfirmationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SidechainWTConfirmationDialog(QWidget *parent = nullptr);
    ~SidechainWTConfirmationDialog();

    bool GetConfirmed();
    void SetInfo(const QString& strWTAmount, const QString& strFee,
                 const QString& strMcFee, const QString& strDest,
                 const QString& strRefundDest);

public Q_SLOTS:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::SidechainWTConfirmationDialog *ui;

    void Reset();
    bool fConfirmed = false;

};

#endif // SIDECHAINWTCONFIRMATIONDIALOG_H
