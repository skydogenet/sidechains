// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_MANUALBMMDIALOG_H
#define BITCOIN_MANUALBMMDIALOG_H

#include <QDialog>

class CBlock;
class uint256;

namespace Ui {
class ManualBMMDialog;
}

class ManualBMMDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManualBMMDialog(QWidget *parent = 0);
    ~ManualBMMDialog();

private:
    Ui::ManualBMMDialog *ui;

private Q_SLOTS:
    void on_pushButtonCreateBlock_clicked();

    void on_pushButtonSendCriticalRequest_clicked();

    void on_pushButtonSubmitBlock_clicked();

private:
    bool CreateBMMBlock(CBlock& block, std::string& strError);

    bool SubmitBMMBlock(const CBlock& block);

    uint256 SendBMMRequest(const uint256& hashBMM, const uint256& hashBlockMain);
};

#endif // BITCOIN_MANUALBMMDIALOG_H
