// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/manualbmmdialog.h>
#include <qt/forms/ui_manualbmmdialog.h>

#include <qt/bitcoinunits.h>
#include <qt/guiutil.h>

#include <chainparams.h>
#include <bmmcache.h>
#include <core_io.h>
#include <primitives/block.h>
#include <sidechainclient.h>
#include <uint256.h>
#include <validation.h>


ManualBMMDialog::ManualBMMDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManualBMMDialog)
{
    ui->setupUi(this);
}

ManualBMMDialog::~ManualBMMDialog()
{
    delete ui;
}
void ManualBMMDialog::on_pushButtonCreateBlock_clicked()
{
    QMessageBox messageBox;
    messageBox.setDefaultButton(QMessageBox::Ok);

    CBlock block;
    std::string strError = "";
    if (!CreateBMMBlock(block, strError)) {
        messageBox.setWindowTitle("Error creating BMM block!");
        messageBox.setText(QString::fromStdString(strError));
        messageBox.exec();
        return;
    }

    std::stringstream ss;
    ss << "BMM hashMerkleRoot (h*):\n" << block.hashMerkleRoot.ToString();
    ss << std::endl;
    ss << std::endl;
    ss << "BMM Block:\n" << block.ToString() << std::endl;

    ui->textBrowser->setText(QString::fromStdString(ss.str()));
    ui->lineEditManualBMMHash->setText(QString::fromStdString(block.hashMerkleRoot.ToString()));
}

void ManualBMMDialog::on_pushButtonSendCriticalRequest_clicked()
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

void ManualBMMDialog::on_pushButtonSubmitBlock_clicked()
{
    // TODO fix this code to work with new BMM rules

    QMessageBox messageBox;
    messageBox.setDefaultButton(QMessageBox::Ok);

    uint256 hashMerkleRoot = uint256S(ui->lineEditBMMHash->text().toStdString());
    CBlock block;

    if (!bmmCache.GetBMMBlock(hashMerkleRoot, block)) {
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

    if (SubmitBMMBlock(block)) {
        // Block submitted message box
        messageBox.setWindowTitle("Block Submitted!");
        QString result = "Block hash:\n";
        result += QString::fromStdString(block.GetHash().ToString());
        result += "\n\n";
        result += "BMM (merkle root) hash: \n";
        result += QString::fromStdString(block.hashMerkleRoot.ToString());
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

bool ManualBMMDialog::CreateBMMBlock(CBlock& block, std::string& strError)
{
    SidechainClient client;
    CAmount nFees;
    return client.CreateBMMBlock(block, strError, nFees);
}

bool ManualBMMDialog::SubmitBMMBlock(const CBlock& block)
{
    std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(block);
    if (!ProcessNewBlock(Params(), shared_pblock, true, NULL)) {
        return false;
    }

    return true;
}

uint256 ManualBMMDialog::SendBMMRequest(const uint256& hashBMM, const uint256& hashBlockMain)
{
    // TODO use user input bmm amount
    SidechainClient client;
    return client.SendBMMRequest(hashBMM, hashBlockMain, 0, 0);
}
