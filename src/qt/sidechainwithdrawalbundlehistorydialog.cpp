// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/sidechainwithdrawalbundlehistorydialog.h>
#include <qt/forms/ui_sidechainwithdrawalbundlehistorydialog.h>

#include <QScrollBar>

#include <qt/sidechainwithdrawalbundlehistorytablemodel.h>

SidechainWithdrawalBundleHistoryDialog::SidechainWithdrawalBundleHistoryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SidechainWithdrawalBundleHistoryDialog)
{
    ui->setupUi(this);

    withdrawalBundleHistoryModel = new SidechainWithdrawalBundleHistoryTableModel(this);

    ui->tableView->setModel(withdrawalBundleHistoryModel);

    // Set resize mode
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Don't stretch last cell of horizontal header
    ui->tableView->horizontalHeader()->setStretchLastSection(false);

    // Hide vertical header
    ui->tableView->verticalHeader()->setVisible(false);

    // Left align the horizontal header text
    ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    // Set horizontal scroll speed to per 3 pixels
    ui->tableView->horizontalHeader()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableView->horizontalHeader()->horizontalScrollBar()->setSingleStep(3); // 3 Pixels

    // Select entire row
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Select only one row
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    // Disable word wrap
    ui->tableView->setWordWrap(false);
}

SidechainWithdrawalBundleHistoryDialog::~SidechainWithdrawalBundleHistoryDialog()
{
    delete ui;
}

void SidechainWithdrawalBundleHistoryDialog::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    withdrawalBundleHistoryModel->setWalletModel(model);
}

void SidechainWithdrawalBundleHistoryDialog::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    withdrawalBundleHistoryModel->setClientModel(model);
    withdrawalBundleHistoryModel->UpdateModel();
}

void SidechainWithdrawalBundleHistoryDialog::on_tableView_doubleClicked(const QModelIndex& index)
{
    // Get the WithdrawalBundle hash that was double clicked
    QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();

    // Emit double clicked signal with WithdrawalBundle hash
    for (int i = 0; i < selected.size(); i++) {
        uint256 hash;
        if (withdrawalBundleHistoryModel->GetWithdrawalBundleInfoAtRow(selected[i].row(), hash)) {
            Q_EMIT doubleClickedWithdrawalBundle(hash);
        }
    }
}
