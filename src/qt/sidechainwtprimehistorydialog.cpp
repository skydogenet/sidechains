// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/sidechainwtprimehistorydialog.h>
#include <qt/forms/ui_sidechainwtprimehistorydialog.h>

#include <QScrollBar>

#include <qt/sidechainwtprimehistorytablemodel.h>

SidechainWTPrimeHistoryDialog::SidechainWTPrimeHistoryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SidechainWTPrimeHistoryDialog)
{
    ui->setupUi(this);

    wtPrimeHistoryModel = new SidechainWTPrimeHistoryTableModel(this);

    ui->tableView->setModel(wtPrimeHistoryModel);

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

SidechainWTPrimeHistoryDialog::~SidechainWTPrimeHistoryDialog()
{
    delete ui;
}

void SidechainWTPrimeHistoryDialog::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    wtPrimeHistoryModel->setWalletModel(model);
}

void SidechainWTPrimeHistoryDialog::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    wtPrimeHistoryModel->setClientModel(model);
    wtPrimeHistoryModel->UpdateModel();
}

void SidechainWTPrimeHistoryDialog::on_tableView_doubleClicked(const QModelIndex& index)
{
    // Get the WT^ hash that was double clicked
    QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();

    // Emit double clicked signal with WT^ hash
    for (int i = 0; i < selected.size(); i++) {
        uint256 hash;
        if (wtPrimeHistoryModel->GetWTPrimeInfoAtRow(selected[i].row(), hash)) {
            Q_EMIT doubleClickedWTPrime(hash);
        }
    }
}
