// Copyright (c) 2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/sidechainpage.h>
#include <qt/forms/ui_sidechainpage.h>

#include "base58.h"
#include "bitcoinunits.h"
#include "bmm.h"
#include "coins.h"
#include "consensus/validation.h"
#include "core_io.h"
#include "guiconstants.h"
#include "guiutil.h"
#include "init.h"
#include "miner.h"
#include "net.h"
#include "optionsmodel.h"
#include "sidechain.h"
#include "txdb.h"
#include "validation.h"
#include "wallet/coincontrol.h"
#include "wallet/wallet.h"
#include "walletmodel.h"

#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QStackedWidget>

#include <sstream>

#if defined(HAVE_CONFIG_H)
#include "config/bitcoin-config.h" /* for USE_QRCODE */
#endif

#ifdef USE_QRCODE
#include <qrencode.h>
#endif

SidechainPage::SidechainPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SidechainPage)
{
    ui->setupUi(this);

    // Initialize models and table views
    incomingTableView = new QTableView(this);
    outgoingTableView = new QTableView(this);
    incomingTableModel = new SidechainHistoryTableModel(this);
    outgoingTableModel = new SidechainHistoryTableModel(this);

    // Set table models
    incomingTableView->setModel(incomingTableModel);
    outgoingTableView->setModel(outgoingTableModel);

    // Table style
#if QT_VERSION < 0x050000
    incomingTableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    incomingTableView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    outgoingTableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    outgoingTableView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#else
    incomingTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    incomingTableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    outgoingTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    outgoingTableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif

    // Display tables
    ui->frameIncoming->layout()->addWidget(incomingTableView);
    ui->frameOutgoing->layout()->addWidget(outgoingTableView);

    generateAddress();
}

SidechainPage::~SidechainPage()
{
    delete ui;
}

void SidechainPage::generateQR(std::string data)
{
    if (data.empty())
        return;

    CTxDestination dest = DecodeDestination(data);
    if (!IsValidDestination(dest)) {
        return;
    }

#ifdef USE_QRCODE
    ui->QRCode->clear();

    QString encode = QString::fromStdString(data);
    QRcode *code = QRcode_encodeString(encode.toUtf8().constData(), 0, QR_ECLEVEL_L, QR_MODE_8, 1);

    if (code) {
        QImage qr = QImage(code->width + 8, code->width + 8, QImage::Format_RGB32);
        qr.fill(0xffffff);

        unsigned char *data = code->data;
        for (int y = 0; y < code->width; y++) {
            for (int x = 0; x < code->width; x++) {
                qr.setPixel(x + 4, y + 4, ((*data & 1) ? 0x0 : 0xffffff));
                data++;
            }
        }

        QRcode_free(code);
        ui->QRCode->setPixmap(QPixmap::fromImage(qr).scaled(200, 200));
    }
#endif
}

void SidechainPage::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    if (model && model->getOptionsModel())
    {
        connect(model, SIGNAL(balanceChanged(CAmount,CAmount,CAmount,CAmount,CAmount,CAmount)), this,
                SLOT(setBalance(CAmount,CAmount,CAmount,CAmount,CAmount,CAmount)));
    }
}

void SidechainPage::setBalance(const CAmount& balance, const CAmount& unconfirmedBalance,
                               const CAmount& immatureBalance, const CAmount& watchOnlyBalance,
                               const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance)
{
    int unit = walletModel->getOptionsModel()->getDisplayUnit();
    const CAmount& pending = immatureBalance + unconfirmedBalance;
    ui->available->setText(BitcoinUnits::formatWithUnit(unit, balance, false, BitcoinUnits::separatorAlways));
    ui->pending->setText(BitcoinUnits::formatWithUnit(unit, pending, false, BitcoinUnits::separatorAlways));
}

void SidechainPage::on_pushButtonWithdraw_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageWithdraw);
}

void SidechainPage::on_pushButtonDeposit_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageDeposit);
}

void SidechainPage::on_pushButtonCopy_clicked()
{
    GUIUtil::setClipboard(ui->lineEditDepositAddress->text());
}

void SidechainPage::on_pushButtonNew_clicked()
{
    generateAddress();
}

void SidechainPage::on_pushButtonWT_clicked()
{
    // TODO better error messages
    QMessageBox messageBox;
    messageBox.setDefaultButton(QMessageBox::Ok);

    if (vpwallets.empty()) {
        // No active wallet message box
        messageBox.setWindowTitle("No active wallet found!");
        messageBox.setText("You must have an active wallet to withdraw from sidechain");
        messageBox.exec();
        return;
    }

    if (vpwallets[0]->IsLocked()) {
        // Locked wallet message box
        messageBox.setWindowTitle("Wallet locked!");
        messageBox.setText("Wallet must be unlocked to withdraw from sidechain.");
        messageBox.exec();
        return;
    }

    // Check WT amount
    if (!validateWTAmount()) {
        // Invalid withdrawal amount message box
        messageBox.setWindowTitle("Invalid withdrawal amount!");
        messageBox.setText("Check the amount you have entered and try again.");
        messageBox.exec();
        return;
    }

    // Check destination
    CTxDestination dest = DecodeDestination(ui->payTo->text().toStdString());
    if (!IsValidDestination(dest)) {
        // Invalid address message box
        messageBox.setWindowTitle("Invalid destination!");
        messageBox.setText("Check the address you have entered and try again.");
        messageBox.exec();
        return;
    }

    // WT burn transaction
    std::vector<CRecipient> vBurnRecipient;
    CAmount nBurn = ui->payAmount->value();
    CRecipient burnRecipient = {CScript() << OP_RETURN, nBurn, false};
    vBurnRecipient.push_back(burnRecipient);

    CWalletTx bwtx;
    CReserveKey burnKey(vpwallets[0]);
    CAmount nBurnFee;
    int nBurnChangePos = -1;
    std::string strError;
    CCoinControl coin_control;
    if (!vpwallets[0]->CreateTransaction(vBurnRecipient, bwtx, burnKey, nBurnFee, nBurnChangePos, strError, coin_control))
    {
        // Create burn transaction error message box
        messageBox.setWindowTitle("Creating withdraw burn transaction failed!");
        QString createError = "Error creating transaction: ";
        createError += QString::fromStdString(strError);
        createError += "\n";
        messageBox.setText(createError);
        messageBox.exec();
        return;
    }

    CValidationState burnState;
    if (!vpwallets[0]->CommitTransaction(bwtx, burnKey, g_connman.get(), burnState))
    {
        // Commit burn transaction error message box
        messageBox.setWindowTitle("Committing withdraw burn transaction failed!");
        QString commitError = "Error committing transaction: ";
        commitError += QString::fromStdString(burnState.GetRejectReason());
        commitError += "\n";
        messageBox.setText(commitError);
        messageBox.exec();
        return;
    }

    /* Create WT data transaction */
    std::vector<CRecipient> vRecipient;

    SidechainWT wt;
    wt.nSidechain = THIS_SIDECHAIN.nSidechain;
    wt.strDestination = ui->payTo->text().toStdString();
    wt.wt = *bwtx.tx;

    CRecipient recipient = {wt.GetScript(), CENT, false};
    vRecipient.push_back(recipient);

    CWalletTx wtx;
    CReserveKey reserveKey(vpwallets[0]);
    CAmount nFee;
    int nChangePos = -1;
    strError.clear();
    if (!vpwallets[0]->CreateTransaction(vRecipient, wtx, reserveKey, nFee, nChangePos, strError, coin_control))
    {
        // Create transaction error message box
        messageBox.setWindowTitle("Creating withdraw data transaction failed!");
        QString createError = "Error creating transaction: ";
        createError += QString::fromStdString(strError);
        createError += "\n";
        messageBox.setText(createError);
        messageBox.exec();
        return;
    }

    CValidationState state;
    if (!vpwallets[0]->CommitTransaction(wtx, reserveKey, g_connman.get(), state))
    {
        // Commit transaction error message box
        messageBox.setWindowTitle("Committing withdraw data transaction failed!");
        QString commitError = "Error committing transaction: ";
        commitError += QString::fromStdString(state.GetRejectReason());
        commitError += "\n";
        messageBox.setText(commitError);
        messageBox.exec();
        return;
    }

    // Successful withdraw message box
    messageBox.setWindowTitle("Withdraw transaction created!");
    QString result = "txid: " + QString::fromStdString(wtx.GetHash().ToString());
    result += "\n";
    result += "Amount withdrawn: ";
    int unit = walletModel->getOptionsModel()->getDisplayUnit();
    result += BitcoinUnits::formatWithUnit(unit, nBurn, false, BitcoinUnits::separatorAlways);
    messageBox.setText(result);
    messageBox.exec();
}

void SidechainPage::on_addressBookButton_clicked()
{

}

void SidechainPage::on_pasteButton_clicked()
{
    // Paste text from clipboard into recipient field
    ui->payTo->setText(QApplication::clipboard()->text());
}

void SidechainPage::on_deleteButton_clicked()
{
    ui->payTo->clear();
}

bool SidechainPage::validateWTAmount()
{
    if (!ui->payAmount->validate()) {
        ui->payAmount->setValid(false);
        return false;
    }

    // Sending a zero amount is invalid
    if (ui->payAmount->value(0) <= 0) {
        ui->payAmount->setValid(false);
        return false;
    }

    // Reject dust outputs:
    if (GUIUtil::isDust(ui->payTo->text(), ui->payAmount->value())) {
        ui->payAmount->setValid(false);
        return false;
    }

    return true;
}

void SidechainPage::generateAddress()
{
    if (vpwallets.empty())
        return;

    LOCK2(cs_main, vpwallets[0]->cs_wallet);

    vpwallets[0]->TopUpKeyPool();

    // TODO don't generate address here
    CPubKey newKey;
    if (vpwallets[0]->GetKeyFromPool(newKey)) {
        CKeyID keyID = newKey.GetID();

        CTxDestination address(keyID);
        std::string strAddress = EncodeDestination(address);

        generateQR(strAddress);

        ui->lineEditDepositAddress->setText(QString::fromStdString(strAddress));
    }
}

void SidechainPage::on_pushButtonCreateBlock_clicked()
{
    // TODO make error messages better
    QMessageBox messageBox;
    messageBox.setDefaultButton(QMessageBox::Ok);

    if (vpwallets.empty()) {
        // No active wallet message box
        messageBox.setWindowTitle("No active wallet found!");
        messageBox.setText("You must have an active wallet to create blocks.");
        messageBox.exec();
        return;
    }

    if (vpwallets[0]->IsLocked()) {
        // Locked wallet message box
        messageBox.setWindowTitle("Wallet locked!");
        messageBox.setText("Wallet must be unlocked to create blocks.");
        messageBox.exec();
        return;
    }

    std::shared_ptr<CReserveScript> coinbaseScript;
    vpwallets[0]->GetScriptForMining(coinbaseScript);

    // If the keypool is exhausted, no script is returned at all.  Catch this.
    if (!coinbaseScript || coinbaseScript->reserveScript.empty()) {
        // No script for mining error message
        messageBox.setWindowTitle("Keypool exhausted!");
        messageBox.setText("The keypool has been exhausted, cannot create blocks.");
        messageBox.exec();
        return;
    }

    coinbaseScript->KeepScript();

    std::unique_ptr<CBlockTemplate> pblocktemplate(
                BlockAssembler(Params()).CreateNewBlock(coinbaseScript->reserveScript, false, true));

    if (!pblocktemplate.get()) {
        // No block template error message
        messageBox.setWindowTitle("Failed to get block template!");
        messageBox.setText("Cannot create blocks without template.");
        messageBox.exec();
        return;
    }

    unsigned int nExtraNonce = 0;
    CBlock *pblock = &pblocktemplate->block;
    {
        LOCK(cs_main);
        IncrementExtraNonce(pblock, chainActive.Tip(), nExtraNonce);
    }

    ++pblock->nNonce;

    if (!bmm.StoreBMMBlock(*pblock)) {
        // Failed to store BMM DAG candidate block
        messageBox.setWindowTitle("Failed to store BMM block!");
        messageBox.setText("Couldn't store BMM block candidate.");
        messageBox.exec();
        return;
    }

    std::stringstream ss;
    ss << "BMM blinded block hash (h*):\n" << pblock->GetHash().ToString();
    ss << std::endl;
    ss << std::endl;
    ss << "BMM Block:\n" << pblock->ToString() << std::endl;

    ui->textBrowser->setText(QString::fromStdString(ss.str()));
}

void SidechainPage::on_pushButtonSubmitBlock_clicked()
{
    // TODO improve error messages
    QMessageBox messageBox;
    messageBox.setDefaultButton(QMessageBox::Ok);

    uint256 hashBlock = uint256S(ui->lineEditBMMHash->text().toStdString());
    CBlock block;

    if (!bmm.GetBMMBlock(hashBlock, block)) {
        // Block not stored message box
        messageBox.setWindowTitle("Block not found!");
        messageBox.setText("Your node does not have this BMM block stored.");
        messageBox.exec();
        return;
    }

    // Get user input h* and coinbase hex
    std::string strProof = ui->lineEditProof->text().toStdString();
    CMutableTransaction mtx;
    if (!DecodeHexTx(mtx, ui->lineEditCoinbaseHex->text().toStdString())) {
        // Invalid transaction hex input message box
        messageBox.setWindowTitle("Invalid transaction hex!");
        messageBox.setText("The transaction hex is invalid.");
        messageBox.exec();
        return;
    }

    CTransaction criticalTx(mtx);
    block.criticalProof = strProof;
    block.criticalTx = criticalTx;

    std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(block);

    if (!ProcessNewBlock(Params(), shared_pblock, true, NULL)) {
        // Failed to process block message box
        messageBox.setWindowTitle("Failed to process block!");
        messageBox.setText("The submitted block is invalid.");
        messageBox.exec();
        return;
    }
}
