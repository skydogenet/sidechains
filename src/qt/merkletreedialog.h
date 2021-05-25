// Copyright (c) 2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MERKLETREEDIALOG_H
#define MERKLETREEDIALOG_H

#include <QDialog>

namespace Ui {
class MerkleTreeDialog;
}

class MerkleTreeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MerkleTreeDialog(QWidget *parent = nullptr);
    ~MerkleTreeDialog();

    void SetTreeString(const std::string& str);

private:
    Ui::MerkleTreeDialog *ui;
};

#endif // MERKLETREEDIALOG_H
