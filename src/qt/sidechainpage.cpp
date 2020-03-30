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
#include <bmmcache.h>
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

static const int nTrainRefresh = 5 * 1000; // 5 seconds
static const int nRetryRefresh = 5 * 1000; // 5 seconds
static const int nTrainWarningSleep = 30 * 1000; // 30 seconds

static const int PAGE_DEFAULT_INDEX = 0;
static const int PAGE_RESTART_INDEX = 1;
static const int PAGE_CONNERR_INDEX = 2;
static const int PAGE_CONFIG_INDEX = 3;

SidechainPage::SidechainPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SidechainPage)
{
    ui->setupUi(this);

    // Initialize configuration generator dialog - only for the conf page of
    // the stacked widget. Clicking on "redo mainchain connection" will spawn
    // its own instance.
    confGeneratorDialog = new ConfGeneratorDialog(this, false /* fDialog */);
    connect(confGeneratorDialog, SIGNAL(Applied()), this, SLOT(ShowRestartPage()));

    // Initialize the BMM Automation refresh timer
    bmmTimer = new QTimer(this);
    connect(bmmTimer, SIGNAL(timeout()), this, SLOT(RefreshBMM()));

    // Iniitialize and start the train timer - refresh number of blocks left on
    // mainchain until another WT^ can be proposed)
    trainTimer = new QTimer(this);
    connect(trainTimer, SIGNAL(timeout()), this, SLOT(RefreshTrain()));
    trainTimer->start(nTrainRefresh);

    // A timer to retry updating the train schedule if it fails
    fSleepTrainWarning = false;
    trainRetryTimer = new QTimer(this);
    connect(trainRetryTimer, SIGNAL(timeout()), this, SLOT(RefreshTrain()));

    // A sleep timer for the train update warning so that it isn't displayed
    // a bunch of times quickly when blocks are connected
    trainWarningSleepTimer = new QTimer(this);
    connect(trainWarningSleepTimer, SIGNAL(timeout()), this, SLOT(ResetTrainWarningSleep()));

    // TODO save & load the checkbox state
    // TODO save & load timer setting
    if (ui->checkBoxEnableAutomation->isChecked())
        bmmTimer->start(ui->spinBoxRefreshInterval->value() * 1000);

    // Initialize the train error message box
    trainErrorMessageBox = new QMessageBox(this);
    trainErrorMessageBox->setDefaultButton(QMessageBox::Ok);
    trainErrorMessageBox->setWindowTitle("Train schedule update failed!");
    std::string str;
    str = "The sidechain has failed to connect to the mainchain!\n\n";
    str += "This may be due to configuration issues. ";
    str += "Please check that you have set up configuration files.\n\n";
    str += "Also make sure that the mainchain node is running!\n\n";
    str += "Networking will be disabled until the connection is restored\n\n";
    str += "Will retry in a few seconds after you close this window...\n";

    trainErrorMessageBox->setText(QString::fromStdString(str));

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
    RefreshTrain();

    // Set the fee label
    QString strFee = "Note: this sidechain will collect its own fee of: ";
    strFee += BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, SIDECHAIN_DEPOSIT_FEE);
    ui->labelFee->setText(strFee);

    // Check configuration & connection - set page to configuration
    // page if configuration / connection check fails
    bool fConfiguration = false;
    bool fConnection = false;
    CheckConfiguration(fConfiguration, fConnection);

    // Display the configuration widget if we need to - hide sidechain page
    if (!fConfiguration) {
        ui->stackedWidget->addWidget(confGeneratorDialog);
        ui->stackedWidget->setCurrentIndex(PAGE_CONFIG_INDEX);
    }
    else
    if (!fConnection) {
        ui->stackedWidget->setCurrentIndex(PAGE_CONNERR_INDEX);
    } else {
        ui->stackedWidget->setCurrentIndex(PAGE_DEFAULT_INDEX);
    }
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
    std::string strDest = ui->payTo->text().toStdString();
    CTxDestination dest = DecodeDestination(strDest, true);
    if (!IsValidDestination(dest)) {
        // Invalid address message box
        messageBox.setWindowTitle("Invalid destination!");
        messageBox.setText("Check the address you have entered and try again.");
        messageBox.exec();
        return;
    }

    std::string strError = "";
    CAmount burnAmount = ui->payAmount->value();
    uint256 txid;
    if (!vpwallets[0]->CreateWT(burnAmount, strDest, strError, txid)) {
        // Create burn transaction error message box
        messageBox.setWindowTitle("Creating withdraw transaction failed!");
        QString createError = "Error creating transaction: ";
        createError += QString::fromStdString(strError);
        createError += "\n";
        messageBox.setText(createError);
        messageBox.exec();
        return;
    }

    // Successful withdraw message box
    messageBox.setWindowTitle("Withdraw transaction created!");
    QString result = "txid: " + QString::fromStdString(txid.ToString());
    result += "\n";
    result += "Amount withdrawn: ";
    int unit = walletModel->getOptionsModel()->getDisplayUnit();
    result += BitcoinUnits::formatWithUnit(unit, burnAmount, false, BitcoinUnits::separatorAlways);
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

    if (ui->lineEditManualMainchainBlockHash->text().isEmpty()) {
        messageBox.setText("You must enter the current mainchain chain tip block hash!");
        messageBox.exec();
        return;
    }

    uint256 hashMainBlock = uint256S(ui->lineEditManualMainchainBlockHash->text().toStdString());
    if (hashMainBlock.IsNull()) {
        messageBox.setText("Invalid previous mainchain block hash!");
        messageBox.exec();
        return;
    }

    uint256 hashTXID = SendBMMRequest(hashBMM, hashMainBlock);

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

void SidechainPage::on_checkBoxEnableAutomation_toggled(bool fChecked)
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

    if (!bmmCache.GetBMMBlock(hashBlock, block)) {
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

bool SidechainPage::CreateBMMBlock(CBlock& block, QString error)
{
    SidechainClient client;
    std::string strError = "";
    bool fCreated = client.CreateBMMBlock(block, strError);

    error = QString::fromStdString(strError);
    return fCreated;
}

uint256 SidechainPage::SendBMMRequest(const uint256& hashBMM, const uint256& hashBlockMain)
{
    // TODO use user input bmm amount
    SidechainClient client;
    uint256 hashTXID = client.SendBMMCriticalDataRequest(hashBMM, hashBlockMain, 0, 0);
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
    QMessageBox messageBox;
    messageBox.setDefaultButton(QMessageBox::Ok);

    if (!CheckMainchainConnection()) {
        UpdateNetworkActive(false /* fMainchainConnected */);
        ui->checkBoxEnableAutomation->setChecked(false);

        messageBox.setWindowTitle("Automated BMM failed - mainchain connection failed!");
        std::string str;
        str = "The sidechain has failed to connect to the mainchain!\n\n";
        str += "Please check configuration file settings and verify that the ";
        str += "mainchain is running!";
        messageBox.setText(QString::fromStdString(str));
        messageBox.exec();
        return;
    }

    bool fReorg = false;
    std::vector<uint256> vOrphan;
    if (!UpdateMainBlockHashCache(fReorg, vOrphan))
    {
        messageBox.setWindowTitle("Automated BMM failed - couldn't update mainchain block cache!");
        std::string str;
        str = "The sidechain has failed to update the mainchain block cache!\n\n";
        str += "Please check configuration file settings and verify that the ";
        str += "mainchain is running!";
        messageBox.setText(QString::fromStdString(str));
        messageBox.exec();
        return;
    }
    if (fReorg)
        HandleMainchainReorg(vOrphan);

    SidechainClient client;
    std::string strError = "";
    uint256 hashCreated;
    uint256 hashConnected;
    if (!client.RefreshBMM(strError, hashCreated, hashConnected)) {
        UpdateNetworkActive(false /* fMainchainConnected */);
        ui->checkBoxEnableAutomation->setChecked(false);

        messageBox.setWindowTitle("Automated BMM failed!");
        std::string str;
        str = "The sidechain has failed to refresh BMM status.\n\n";
        str += "This may be due to configuration issues.";
        str += " Please check that you have set up configuration files";
        str += " and re-enable automated BMM.\n\n";
        str += "Also make sure that the mainchain node is running!\n\n";
        str += "Networking will be disabled until mainchain connected!\n\n";
        str += "Error message:\n" + strError + "\n";
        messageBox.setText(QString::fromStdString(str));
        messageBox.exec();
        return;
    }

    UpdateNetworkActive(true /* fMainchainConnected */);

    // Update GUI

    uint256 hashBlock = bmmCache.GetLastMainBlockHash();
    ui->pushButtonHashBlockLastSeen->setText(QString::fromStdString(hashBlock.ToString()));

    if (!hashCreated.IsNull())
        new QListWidgetItem(QString::fromStdString(hashCreated.ToString()), ui->listWidgetBMMCreated);

    if (!hashConnected.IsNull())
        new QListWidgetItem(QString::fromStdString(hashConnected.ToString()), ui->listWidgetBMMConnected);
}

void SidechainPage::RefreshTrain()
{
    SidechainClient client;
    int nMainchainBlocks = 0;
    if (!client.GetBlockCount(nMainchainBlocks)) {
        UpdateNetworkActive(false /* fMainchainConnected */);
        trainTimer->stop();
        if (!fSleepTrainWarning && ui->stackedWidget->currentIndex() != PAGE_CONFIG_INDEX
                && ui->stackedWidget->currentIndex() != PAGE_RESTART_INDEX) {
            trainErrorMessageBox->close();
            trainErrorMessageBox->exec();
            fSleepTrainWarning = true;
            trainWarningSleepTimer->start(nTrainWarningSleep);
        }
        trainRetryTimer->start(nRetryRefresh);
        ui->train->setText("? - not connected to mainchain");
        return;
    }

    UpdateNetworkActive(true /* fMainchainConnected */);

    // If the retry timer was running we want to stop it now that a connection
    // has been established
    trainRetryTimer->stop();
    trainTimer->start(nTrainRefresh);

    int nBlocksLeft = GetBlocksVerificationPeriod(nMainchainBlocks);
    std::string strTrain = "";
    // Show the "leaving now" message a little bit before it's actually leaving
    if (nBlocksLeft > 5)
        strTrain = std::to_string(GetBlocksVerificationPeriod(nMainchainBlocks)) + " blocks";
    else
        strTrain = "Leaving now - all aboard!";

    ui->train->setText(QString::fromStdString(strTrain));
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

void SidechainPage::ResetTrainWarningSleep()
{
    trainWarningSleepTimer->stop();
    fSleepTrainWarning = false;
}

void SidechainPage::ShowRestartPage()
{
    ui->stackedWidget->setCurrentIndex(PAGE_RESTART_INDEX);
}

void SidechainPage::on_pushButtonRetryConnection_clicked()
{
    bool fConfig = false;
    bool fConnection = false;
    CheckConfiguration(fConfig, fConnection);
    UpdateNetworkActive(fConnection);
}

void SidechainPage::UpdateNetworkActive(bool fMainchainConnected) {
    // Enable or disable networking based on connection to mainchain
    SetNetworkActive(fMainchainConnected, "Sidechain page update.");

    if (fMainchainConnected) {
        // Close the connection warning popup if it is open
        trainErrorMessageBox->close();
    }
    // Update the GUI to show or hide the network connection warning.
    // Only switch to the network warning if the configuration warning isn't
    // currently displayed.
    if (!fMainchainConnected && ui->stackedWidget->currentIndex() != PAGE_CONFIG_INDEX &&
            ui->stackedWidget->currentIndex() != PAGE_RESTART_INDEX) {
        ui->stackedWidget->setCurrentIndex(PAGE_CONNERR_INDEX);
    }
    else
    if (fMainchainConnected && ui->stackedWidget->currentIndex() != PAGE_CONFIG_INDEX){
        ui->stackedWidget->setCurrentIndex(PAGE_DEFAULT_INDEX);
    }
}

void SidechainPage::CheckConfiguration(bool& fConfig, bool& fConnection)
{
    fConfig = false;
    fConnection = false;

    // Does the sidechain directory exist?
    fs::path pathSide = GetDefaultDataDir();
    if (!fs::exists(pathSide))
        return;

    // Get ~/
    fs::path pathHome = GetHomeDir();

    // Does the drivenet directory exist?
    fs::path pathDrivechain = pathHome / ".drivenet";
    if (!fs::exists(pathDrivechain))
        return;

    fs::path pathConfMain = pathDrivechain / "drivenet.conf";
    fs::path pathConfSide = pathSide / "testchain.conf";

    // Do drivenet.conf & side.conf exist?
    if (fs::exists(pathConfMain) && fs::exists(pathConfSide))
        fConfig = true;

    // Check if we can connect to the mainchain
    fConnection = CheckMainchainConnection();
    UpdateNetworkActive(fConnection);
}
