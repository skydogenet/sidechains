// Copyright (c) 2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/blockindexdetailsdialog.h>
#include <qt/forms/ui_blockindexdetailsdialog.h>

#include <qt/guiutil.h>
#include <qt/merkletreedialog.h>
#include <qt/txdetails.h>

#include <QMessageBox>

#include <chain.h>
#include <chainparams.h>
#include <consensus/merkle.h>
#include <primitives/block.h>
#include <streams.h>
#include <utilstrencodings.h>
#include <validation.h>

#include <sstream>

BlockIndexDetailsDialog::BlockIndexDetailsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlockIndexDetailsDialog)
{
    ui->setupUi(this);

    nHeight = 0;
    hashBlock = uint256();
    cachedBlock.SetNull();

    ui->tableWidgetTransactions->setColumnCount(2);
    ui->tableWidgetTransactions->setHorizontalHeaderLabels(QStringList() << "n" << "txid");
    ui->tableWidgetTransactions->verticalHeader()->setVisible(false);

    // Resize cells (in a backwards compatible way)
#if QT_VERSION < 0x050000
    ui->tableWidgetTransactions->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#else
    ui->tableWidgetTransactions->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif

    // Highlight entire row
    ui->tableWidgetTransactions->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Select only one row
    ui->tableWidgetTransactions->setSelectionMode(QAbstractItemView::SingleSelection);

    merkleTreeDialog = new MerkleTreeDialog(this);
}

BlockIndexDetailsDialog::~BlockIndexDetailsDialog()
{
    delete ui;
}

void BlockIndexDetailsDialog::SetBlockIndex(const CBlockIndex* index)
{
    if (!index)
        return;

    nHeight = index->nHeight;
    hashBlock = index->GetBlockHash();
    cachedBlock.SetNull();
    ui->labelBlockInfo->setText("#Tx: ? Block Size: ? (click \"Load Transactions\")");
    ui->pushButtonMerkleTree->setEnabled(false);
    ui->pushButtonCopyHex->setEnabled(false);
    ui->pushButtonCopyHeaderHex->setEnabled(false);

    // Show details on dialog

    // Height
    ui->labelHeight->setText(QString::number(nHeight));

    // Hash
    ui->labelHash->setText(QString::fromStdString(hashBlock.ToString()));

    // # Conf
    int nConf = -1;
    if (chainActive.Contains(index))
        nConf = chainActive.Height() - nHeight + 1;

    ui->labelConf->setText(QString::number(nConf));

    // Version
    ui->labelVersion->setText(QString::number(index->nVersion));

    // Version Hex
    ui->labelVersionHex->setText(QString::fromStdString(strprintf("%08x", index->nVersion)));

    // Merkle Root
    ui->labelMerkleRoot->setText(QString::fromStdString(index->hashMerkleRoot.ToString()));

    // Time
    ui->labelTime->setText(QString::number((int64_t)index->nTime));

    // Median Time
    ui->labelMedianTime->setText(QString::number((int64_t)index->GetMedianTimePast()));

    // Nonce
    ui->labelNonce->setText(QString::number((int64_t)index->nNonce));

    // Bits
    ui->labelBits->setText(QString::fromStdString(strprintf("%08x", index->nBits)));

    // TODO should we calculate the difficulty?
    // Diff
    //ui->labelDiff->setText(QString::number(GetDifficulty(chainActive, index)));

    // Chain Work
    ui->labelChainWork->setText(QString::fromStdString(index->nChainWork.ToString()));

    // prevBlockHash
    uint256 hashPrev = uint256();
    if (index->pprev)
        hashPrev = index->pprev->GetBlockHash();

    ui->labelPrevBlockHash->setText(QString::fromStdString(hashPrev.ToString()));

    // Next block hash
    uint256 hashNext = uint256();
    if (chainActive.Next(index))
        hashNext = chainActive.Next(index)->GetBlockHash();

    ui->labelNextBlockHash->setText(QString::fromStdString(hashNext.ToString()));

    pBlockIndex = index;

    ui->tableWidgetTransactions->setRowCount(0);
    vtx.clear();

    merkleTreeDialog->close();
}

void BlockIndexDetailsDialog::on_pushButtonLoadTransactions_clicked()
{
    if (!pBlockIndex) {
        // TODO error message
        return;
    }

    if (fHavePruned) {
        // TODO display error
        return;
    }

    // Double check that the block is in the chain and should be on disk
    if (!mapBlockIndex.count(pBlockIndex->GetBlockHash())) {
        // TODO display error
        return;
    }

    // Load block from disk

    CBlock block;
    if (!ReadBlockFromDisk(block, pBlockIndex, Params().GetConsensus())) {
        // TODO display error
        return;
    }

    vtx = block.vtx;

    ui->tableWidgetTransactions->setRowCount(0);

    // Add block's transactions to the table
    int nRow = 0;
    for (const CTransactionRef& tx : vtx) {
        ui->tableWidgetTransactions->insertRow(nRow);

        // txn number
        QTableWidgetItem *itemN = new QTableWidgetItem();
        itemN->setText(QString::number(nRow));
        itemN->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemN->setFlags(itemN->flags() & ~Qt::ItemIsEditable);
        ui->tableWidgetTransactions->setItem(nRow, 0, itemN);

        // tx hash
        QTableWidgetItem *itemHash = new QTableWidgetItem();
        itemHash->setText(QString::fromStdString(tx->GetHash().ToString()));
        itemHash->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        itemHash->setFlags(itemHash->flags() & ~Qt::ItemIsEditable);
        ui->tableWidgetTransactions->setItem(nRow, 1, itemHash);

        nRow++;
    }

    cachedBlock = block;

    size_t nTx = cachedBlock.vtx.size();
    size_t nSize = GetSerializeSize(cachedBlock, SER_NETWORK, PROTOCOL_VERSION);

    QString strSize = "";
    if (nSize < 1000000)
        strSize = QString::number(nSize / 1000.0, 'f', 2) + " KB";
    else
        strSize = QString::number(nSize / 1000000.0, 'f', 2) + " MB";

    QString strInfo = "#Tx: " + QString::number(nTx);
    strInfo += " Block size: " + strSize;
    ui->labelBlockInfo->setText(strInfo);

    ui->pushButtonMerkleTree->setEnabled(true);
    ui->pushButtonCopyHex->setEnabled(true);
    ui->pushButtonCopyHeaderHex->setEnabled(true);
}

void BlockIndexDetailsDialog::on_tableWidgetTransactions_doubleClicked(const QModelIndex& i)
{
    unsigned int nTx = i.row();
    if (nTx >= vtx.size())
        return;

    TxDetails txDetailsDialog;
    txDetailsDialog.SetTransaction(CMutableTransaction(*vtx[nTx]));
    txDetailsDialog.exec();
}

void BlockIndexDetailsDialog::on_pushButtonMerkleTree_clicked()
{
    if (cachedBlock.IsNull())
        return;

    std::vector<uint256> vLeaf;
    for (const CTransactionRef& tx : cachedBlock.vtx) {
        vLeaf.push_back(tx->GetHash());
    }

    bool fMutated = false;
    std::string strTree = MerkleTreeString(vLeaf, fMutated);

    merkleTreeDialog->SetTreeString(strTree);
    merkleTreeDialog->show();
}

void BlockIndexDetailsDialog::on_pushButtonCopyHex_clicked()
{
    if (cachedBlock.IsNull())
        return;

    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << cachedBlock;

    GUIUtil::setClipboard(QString::fromStdString(HexStr(ss.str())));
}

void BlockIndexDetailsDialog::on_pushButtonCopyHeaderHex_clicked()
{
    if (cachedBlock.IsNull())
        return;

    CBlockHeader header = cachedBlock.GetBlockHeader();

    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << header;

    GUIUtil::setClipboard(QString::fromStdString(HexStr(ss.str())));
}

// TODO move
// Copy of merkle tree calculator from consensus/merkle.cpp for GUI display
std::string MerkleTreeString(const std::vector<uint256>& vLeaf, bool& fMutated)
{
    if (vLeaf.size() == 0) {
        fMutated = false;
        return "";
    }
    fMutated = false;

    // x = level in tree, y = hash
    // Level 0 is the leaves, and the last level is merkle root
    std::vector<std::vector<uint256>> vTree;

    // Generate a merkle tree, the same way as consensus/merkle.cpp but
    // record hashes & leaves to a vector of strings organized into levels of
    // the tree.

    uint32_t count = 0;
    uint256 inner[32];
    int matchlevel = -1;
    uint32_t branchpos = -1;
    while (count < vLeaf.size()) {
        uint256 h = vLeaf[count];
        bool matchh = count == branchpos;
        count++;
        int level;
        for (level = 0; !(count & (((uint32_t)1) << level)); level++) {
            // Add a new level to the tree string vector
            if (level >= (int)vTree.size())
                vTree.push_back(std::vector<uint256>());

            // Record hash for level
            vTree[level].push_back(h);

            fMutated |= (inner[level] == h);
            CHash256().Write(inner[level].begin(), 32).Write(h.begin(), 32).Finalize(h.begin());
        }
        inner[level] = h;
        if (matchh) {
            matchlevel = level;
        }
        // Add a new level to the tree string vector
        if (level >= (int)vTree.size())
            vTree.push_back(std::vector<uint256>());

        // Record hash for level
        vTree[level].push_back(h);
    }
    // Final sweep of right side of tree
    int level = 0;
    while (!(count & (((uint32_t)1) << level))) {
        level++;
    }
    uint256 h = inner[level];
    while (count != (((uint32_t)1) << level)) {
        CHash256().Write(h.begin(), 32).Write(h.begin(), 32).Finalize(h.begin());
        count += (((uint32_t)1) << level);
        level++;
        while (!(count & (((uint32_t)1) << level))) {
            if (level >= (int)vTree.size())
                vTree.push_back(std::vector<uint256>());
            vTree[level].push_back(h);

            CHash256().Write(inner[level].begin(), 32).Write(h.begin(), 32).Finalize(h.begin());
            level++;
        }
    }

    // If the tree has more than 1 level and the last level has only 1 node,
    // delete the last level.
    if (vTree.size() > 1 && vTree.back().size() == 1)
        vTree.pop_back();

    // Add merkle root hash
    vTree.push_back(std::vector<uint256> { h });

    // Format results

    std::stringstream ss;

    int nTreeLevel = vTree.size() - 1;
    for (auto ritx = vTree.rbegin(); ritx != vTree.rend(); ritx++) {
        ss << "Level " << nTreeLevel;

        if (nTreeLevel == vTree.size() - 1)
            ss << " (Merkle Root):\n";
        else
        if (nTreeLevel == 0)
            ss << " (TxID):\n";
        else
            ss << " :\n";

        // Add hashes with a '|' between every group of 2
        uint8_t nNode = 0;
        for (const uint256& hash : *ritx) {
            if (nNode == 2) {
                nNode = 0;
                ss << " | ";
            }
            ss << hash.ToString() << " ";
            nNode++;
        }
        ss << "\n\n";

        nTreeLevel--;
    }

    ss << "-------------------------------\n";
    ss << "## What is this screen showing?\n";

    ss << "This display allows you to audit the \"hashMerkleRoot\" field. You can see each step of the process yourself.\n\n";
    ss << "         MerkleRoot = hashZ\n\n";
    ss << "                hashZ\n";
    ss << "               /    \\\n";
    ss << "              /      \\\n";
    ss << "           hashX       hashY |\n";
    ss << "           /   \\         \\\n";
    ss << "          /     \\         \\\n";
    ss << "         /       \\         \\\n";
    ss << "     hashF       hashG  |   hashH\n";
    ss << "    /   \\       /     \\      \\\n";
    ss << "hashA  hashB | hashC  hashD | hashE\n";
    ss << "\n\n";


    ss << "Notes:\n";

    ss << "1. hashF = Sha256 ( hashA, hashB )\n";
    ss << "* * Each node in the tree, is the hash of the two nodes beneath it.\n";
    ss << "2. hashY = Sha256 ( hashH, hashH )\n";
    ss << "* * In the operations above, Hash \"H\" is repeated on purpose. If there is an odd number of nodes at that level, the final node is hashed with itself.\n";
    ss << "3. Hashes A through E are TxIDs -- the hashes of each transaction.\n";

    ss << "Use the HashCalculator to check that the Sha256 hash of the first two TxIDs is the value one level above it. And so on ad infinitum.\n";

    ss << "Further Reading:\n";
    ss << "* https://en.bitcoinwiki.org/wiki/Merkle_tree\n";
    ss << "* https://www.investopedia.com/terms/m/merkle-tree.asp\n";
    ss << "* https://www.geeksforgeeks.org/introduction-to-merkle-tree/\n";

    return ss.str();
}

