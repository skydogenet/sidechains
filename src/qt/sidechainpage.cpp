// Copyright (c) 2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/sidechainpage.h>
#include <qt/forms/ui_sidechainpage.h>

#include <qt/bitcoinunits.h>
#include <qt/confgeneratordialog.h>
#include <qt/guiconstants.h>
#include <qt/guiutil.h>
#include <qt/optionsmodel.h>
#include <qt/walletmodel.h>

#include <base58.h>
#include <bmmblockcache.h>
#include <coins.h>
#include <consensus/validation.h>
#include <core_io.h>
#include <init.h>
#include <miner.h>
#include <net.h>
#include <primitives/block.h>
#include <sidechain.h>
#include <sidechainclient.h>
#include <txdb.h>
#include <validation.h>
#include <wallet/coincontrol.h>
#include <wallet/wallet.h>

#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QStackedWidget>

#include <sstream>

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h> /* for USE_QRCODE */
#endif

#ifdef USE_QRCODE
#include <qrencode.h>
#endif

SidechainPage::SidechainPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SidechainPage)
{
    ui->setupUi(this);

    // Initialize the BMM Automation refresh timer
    bmmTimer = new QTimer(this);
    connect(bmmTimer, SIGNAL(timeout()), this, SLOT(RefreshBMM()));

    // TODO save & load the checkbox state
    // TODO save & load timer setting
    if (ui->checkBoxEnableAutomation->isChecked())
        bmmTimer->start(ui->spinBoxRefreshInterval->value() * 1000);

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
    ui->available->setText(BitcoinUnits::formatWithUnit(unit, balance, false, BitcoinUnits::separatorAlways));
}

void SidechainPage::on_pushButtonMainchain_clicked()
{
    ui->tabWidgetTransfer->setCurrentIndex(0);
}

void SidechainPage::on_pushButtonSidechain_clicked()
{
    ui->tabWidgetTransfer->setCurrentIndex(1);
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
    CTxDestination dest = DecodeDestination(ui->payTo->text().toStdString(), true);
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
    wt.nSidechain = SIDECHAIN_TEST;
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

    CPubKey newKey;
    if (vpwallets[0]->GetKeyFromPool(newKey)) {
        // We want a "legacy" type address
        OutputType type = OUTPUT_TYPE_LEGACY;
        CTxDestination dest = GetDestinationForKey(newKey, type);

        // Watch the script
        vpwallets[0]->LearnRelatedScripts(newKey, type);

        // Generate QR code
        std::string strAddress = EncodeDestination(dest);
        generateQR(strAddress);

        ui->lineEditDepositAddress->setText(QString::fromStdString(strAddress));

        // Add to address book
        vpwallets[0]->SetAddressBook(dest, "Sidechain Deposit", "receive");
    }
    // TODO display error if we didn't get a key
}

void SidechainPage::on_pushButtonCreateBlock_clicked()
{
    // TODO make error messages better
    QMessageBox messageBox;
    messageBox.setDefaultButton(QMessageBox::Ok);

    CBlock block;
    QString error = "";
    if (!CreateBMMBlock(block, error)) {
        messageBox.setWindowTitle("Error creating BMM block!");
        messageBox.setText(error);
        messageBox.exec();
        return;
    }

    std::stringstream ss;
    ss << "BMM blinded block hash (h*):\n" << block.GetBlindHash().ToString();
    ss << std::endl;
    ss << std::endl;
    ss << "BMM Block:\n" << block.ToString() << std::endl;

    ui->textBrowser->setText(QString::fromStdString(ss.str()));
    ui->lineEditManualBMMHash->setText(QString::fromStdString(block.GetBlindHash().ToString()));
}

void SidechainPage::on_pushButtonSendCriticalRequest_clicked()
{
    QMessageBox messageBox;
    messageBox.setDefaultButton(QMessageBox::Ok);
    messageBox.setWindowTitle("Error sending BMM request to mainchain!");

    if (ui->lineEditManualBMMHash->text().isEmpty()) {
        messageBox.setText("You must click \"Generate BMM Block\" first!");
        messageBox.exec();
        return;
    }

    uint256 hashBMM = uint256S(ui->lineEditManualBMMHash->text().toStdString());
    if (hashBMM.IsNull()) {
        messageBox.setText("Invalid BMM block hash (h*)!");
        messageBox.exec();
        return;
    }
    uint256 hashTXID = SendBMMRequest(hashBMM);

    if (hashTXID.IsNull()) {
        messageBox.setText("Failed to create BMM request on mainchain!");
        messageBox.exec();
        return;
    }

    // Show result
    messageBox.setWindowTitle("BMM request created on mainchain!");
    QString result = "txid: ";
    result += QString::fromStdString(hashTXID.ToString());
    messageBox.setText(result);
    messageBox.exec();
    return;

}

void SidechainPage::on_checkBoxEnableAutomation_clicked(bool fChecked)
{
    // Start or stop timer
    if (fChecked) {
        bmmTimer->start(ui->spinBoxRefreshInterval->value() * 1000);
    } else {
        bmmTimer->stop();
    }
}

void SidechainPage::on_pushButtonSubmitBlock_clicked()
{
    // TODO improve error messages
    QMessageBox messageBox;
    messageBox.setDefaultButton(QMessageBox::Ok);

    uint256 hashBlock = uint256S(ui->lineEditBMMHash->text().toStdString());
    CBlock block;

    if (!bmmBlockCache.GetBMMBlock(hashBlock, block)) {
        // Block not stored message box
        messageBox.setWindowTitle("Block not found!");
        messageBox.setText("You do not have this BMM block cached.");
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

    if (SubmitBMMBlock(block)) {
        // Block submitted message box
        messageBox.setWindowTitle("Block Submitted!");
        QString result = "BMM Block hash:\n";
        result += QString::fromStdString(block.GetHash().ToString());
        result += "\n\n";
        result += "BMM (Blinded) hash: \n";
        result += QString::fromStdString(block.GetBlindHash().ToString());
        result += "\n";
        messageBox.setText(result);
        messageBox.exec();
        return;
    } else {
        // Failed to submit block submitted message box
        messageBox.setWindowTitle("Failed to submit block!");
        messageBox.setText("The submitted block is invalid.");
        messageBox.exec();
    }
}

void SidechainPage::on_pushButtonHashBlockLastSeen_clicked()
{
    GUIUtil::setClipboard(QString::fromStdString(hashMainBlockLastSeen.ToString()));
}

bool SidechainPage::CreateBMMBlock(CBlock& block, QString error)
{
    // Create a new BMM block
    if (vpwallets.empty()) {
        error = "No active wallet found!\n";
        return false;
    }
    if (vpwallets[0]->IsLocked()) {
        error = "Wallet locked!\n";
        return false;
    }

    std::shared_ptr<CReserveScript> coinbaseScript;
    vpwallets[0]->GetScriptForMining(coinbaseScript);

    // If the keypool is exhausted, no script is returned at all.  Catch this.
    if (!coinbaseScript || coinbaseScript->reserveScript.empty()) {
        // No script for mining error message
        error = "Keypool exhausted!\n";
        return false;
    }

    coinbaseScript->KeepScript();

    std::unique_ptr<CBlockTemplate> pblocktemplate(
                BlockAssembler(Params()).CreateNewBlock(coinbaseScript->reserveScript, true, true));

    if (!pblocktemplate.get()) {
        // No block template error message
        error = "Failed to get block template!\n";
        return false;
    }

    unsigned int nExtraNonce = 0;
    CBlock *pblock = &pblocktemplate->block;
    {
        LOCK(cs_main);
        IncrementExtraNonce(pblock, chainActive.Tip(), nExtraNonce);
    }

    int nMaxTries = 1000;
    while (true) {
        if (pblock->nNonce == 0)
            pblock->nTime++;

        if (CheckProofOfWork(pblock->GetBlindHash(), pblock->nBits, Params().GetConsensus())) {
            break;
        }

        if (nMaxTries == 0) {
            error = "Couldn't generate PoW, try again!\n";
            break;
        }

        pblock->nNonce++;
        nMaxTries--;
    }

    if (!CheckProofOfWork(pblock->GetBlindHash(), pblock->nBits, Params().GetConsensus())) {
        error = "Invalid PoW!\n";
        return false;
    }

    if (!bmmBlockCache.StoreBMMBlock(*pblock)) {
        // Failed to store BMM candidate block
        error = "Failed to store BMM block!\n";
        return false;
    }

    block = *pblock;

    return true;
}

uint256 SidechainPage::SendBMMRequest(const uint256& hashBMM)
{
    // TODO use user input bmm amount
    SidechainClient client;
    uint256 hashTXID = client.SendBMMCriticalDataRequest(hashBMM, 0, 0);
    if (!hashTXID.IsNull()) {
        // Add to list widget
        QString str = "txid: ";
        str += QString::fromStdString(hashTXID.ToString());
        new QListWidgetItem(str, ui->listWidgetBMMCreated);
    }
    return hashTXID;
}

bool SidechainPage::SubmitBMMBlock(const CBlock& block)
{
    std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(block);
    if (!ProcessNewBlock(Params(), shared_pblock, true, NULL)) {
        return false;
    }

    // Add to list widget
    new QListWidgetItem(QString::fromStdString(block.GetBlindHash().ToString()), ui->listWidgetBMMConnected);

    return true;
}

void SidechainPage::RefreshBMM()
{
    // TODO load / save bmm amount

    // Get updated list of recent main:blocks
    SidechainClient client;
    std::vector<uint256> vNewHash = client.RequestMainBlockHashes();

    if (vNewHash.empty())
        return;

    // Replace local cache
    vMainBlockHash = vNewHash;

    // Update hashBlockLastSeen and keep a backup for later
    uint256 hashMainBlockLastSeenOld = hashMainBlockLastSeen;
    hashMainBlockLastSeen = vNewHash.back();
    ui->pushButtonHashBlockLastSeen->setText(QString::fromStdString(hashMainBlockLastSeen.ToString()));

    // Get our cached BMM blocks
    std::vector<CBlock> vBMMCache = bmmBlockCache.GetBMMBlockCache();
    if (vBMMCache.empty()) {
        CBlock block;
        if (CreateBMMBlock(block)) {
            SendBMMRequest(block.GetBlindHash());
            return;
        }
    }

    // TODO this could be more efficient
    // Check new main:blocks for our bmm requests
    for (const uint256& u : vNewHash) {
        // Check main:block for any of our current BMM requests
        for (const CBlock& b : vBMMCache) {
            const uint256& hashBMMBlock = b.GetBlindHash();
            // Send 'getbmmproof' rpc request to mainchain
            SidechainBMMProof proof;
            proof.hashBMMBlock = hashBMMBlock;
            if (client.RequestBMMProof(u, hashBMMBlock, proof)) {
                CBlock block = b;

                block.criticalProof = proof.txOutProof;

                if (!DecodeHexTx(block.criticalTx, proof.coinbaseHex))
                    continue;

                // Submit BMM block
                if (!SubmitBMMBlock(block))
                    continue;
            }
        }
    }

    // Was there a new mainchain block?
    if (hashMainBlockLastSeenOld != hashMainBlockLastSeen) {
        // Clear out the bmm cache, the old requests are invalid now
        bmmBlockCache.ClearBMMBlocks();

        // Create a new BMM request (old ones have expired)
        CBlock block;
        if (CreateBMMBlock(block)) {
            // Create BMM critical data request
            SendBMMRequest(block.GetBlindHash());
        }
    }
}

void SidechainPage::on_spinBoxRefreshInterval_valueChanged(int n)
{
    if (ui->checkBoxEnableAutomation->isChecked()) {
        bmmTimer->stop();
        bmmTimer->start(ui->spinBoxRefreshInterval->value() * 1000);
    }
}

void SidechainPage::on_pushButtonConfigureBMM_clicked()
{
    ConfGeneratorDialog *dialog = new ConfGeneratorDialog(this);
    dialog->exec();
}
