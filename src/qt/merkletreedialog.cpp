// Copyright (c) 2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/merkletreedialog.h>
#include <qt/forms/ui_merkletreedialog.h>

MerkleTreeDialog::MerkleTreeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MerkleTreeDialog)
{
    ui->setupUi(this);
}

MerkleTreeDialog::~MerkleTreeDialog()
{
    delete ui;
}

void MerkleTreeDialog::SetTreeString(const std::string& str)
{
    ui->textBrowser->setText(QString::fromStdString(str));

}
