// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/sidechainwtprimehistorytablemodel.h>

#include <QTimer>

#include <qt/bitcoinunits.h>
#include <qt/clientmodel.h>
#include <qt/guiconstants.h>
#include <qt/optionsmodel.h>
#include <qt/walletmodel.h>

#include <sidechain.h>
#include <txdb.h>
#include <validation.h>

Q_DECLARE_METATYPE(WTPrimeHistoryTableObject)

SidechainWTPrimeHistoryTableModel::SidechainWTPrimeHistoryTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

int SidechainWTPrimeHistoryTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return model.size();
}

int SidechainWTPrimeHistoryTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 4;
}

QVariant SidechainWTPrimeHistoryTableModel::data(const QModelIndex &index, int role) const
{
    if (!walletModel)
        return false;

    if (!index.isValid())
        return false;

    int row = index.row();
    int col = index.column();

    if (!model.at(row).canConvert<WTPrimeHistoryTableObject>())
        return QVariant();

    WTPrimeHistoryTableObject object = model.at(row).value<WTPrimeHistoryTableObject>();

    int unit = walletModel->getOptionsModel()->getDisplayUnit();

    switch (role) {
    case Qt::DisplayRole:
    {
        // Hash
        if (col == 0) {
            return object.hash;
        }
        // Total Withdrawn
        if (col == 1) {

            QString amount = BitcoinUnits::formatWithUnit(unit, object.amount, false,
                    BitcoinUnits::separatorAlways);
            return amount;
        }
        // Status
        if (col == 2) {
            return object.status;
        }
        // Height
        if (col == 3) {
            return QString::number(object.height);
        }
    }
    }
    return QVariant();
}

QVariant SidechainWTPrimeHistoryTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return QString("Hash");
            case 1:
                return QString("Amount");
            case 2:
                return QString("Status");
            case 3:
                return QString("Height");
            }
        }
    }
    return QVariant();
}

void SidechainWTPrimeHistoryTableModel::UpdateModel()
{
    // TODO there are many ways to improve the efficiency of this

    // Get all of the current WT^(s) into one vector
    std::vector<SidechainWTPrime> vWTPrime;
    vWTPrime = psidechaintree->GetWTPrimes(SIDECHAIN_TEST);

    // Look for updates to WT^(s) & their status already cached by the model
    // and update our model / view.
    //
    // Also look for WT^(s) which have been removed, and remove them from our
    // model / view.
    std::vector<WTPrimeHistoryTableObject> vRemoved;
    for (int i = 0; i < model.size(); i++) {
        if (!model[i].canConvert<WTPrimeHistoryTableObject>())
            return;

        WTPrimeHistoryTableObject object = model[i].value<WTPrimeHistoryTableObject>();

        bool fFound = false;

        // Check if the WT^ should still be in the table and make sure we set
        // it with the current status
        for (const SidechainWTPrime& s : vWTPrime) {
            // Check if the WT^ was removed or the status has changed.
            if (s.wtPrime.GetHash() == uint256S(object.hash.toStdString())) {
                fFound = true;

                // Check for updates to status
                QString status = QString::fromStdString(s.GetStatusStr());
                if (object.status != status) {
                    // Update the status of the object in the table
                    object.status = status;

                    QModelIndex topLeft = index(i, 0);
                    QModelIndex topRight = index(i, columnCount() - 1);
                    Q_EMIT QAbstractItemModel::dataChanged(topLeft, topRight, {Qt::DecorationRole});

                    model[i] = QVariant::fromValue(object);
                }
            }
        }

        // Add to vector of WT^(s) to be removed from model / view
        if (!fFound) {
            vRemoved.push_back(object);
        }
    }

    // Loop through the model and remove deleted WT^(s)
    for (int i = 0; i < model.size(); i++) {
        if (!model[i].canConvert<WTPrimeHistoryTableObject>())
            return;

        WTPrimeHistoryTableObject object = model[i].value<WTPrimeHistoryTableObject>();

        for (const WTPrimeHistoryTableObject& wt : vRemoved) {
            if (wt.hash == object.hash) {
                beginRemoveRows(QModelIndex(), i, i);
                model[i] = model.back();
                model.pop_back();
                endRemoveRows();
            }
        }
    }

    // Check for new WT^(s)
    std::vector<SidechainWTPrime> vNew;
    for (const SidechainWTPrime& s : vWTPrime) {
        bool fFound = false;

        for (const QVariant& qv : model) {
            if (!qv.canConvert<WTPrimeHistoryTableObject>())
                return;

            WTPrimeHistoryTableObject object = qv.value<WTPrimeHistoryTableObject>();

            if (s.wtPrime.GetHash() == uint256S(object.hash.toStdString()))
                fFound = true;
        }
        if (!fFound)
            vNew.push_back(s);
    }

    if (vNew.empty())
        return;

    // Add new WT^(s) if we need to
    beginInsertRows(QModelIndex(), model.size(), model.size() + vNew.size() - 1);
    for (const SidechainWTPrime& wt : vNew) {
        WTPrimeHistoryTableObject object;

        // Insert new WT^ into table
        object.hash = QString::fromStdString(wt.wtPrime.GetHash().ToString());
        object.amount = CTransaction(wt.wtPrime).GetValueOut();
        object.status = QString::fromStdString(wt.GetStatusStr());
        object.height = wt.nHeight;
        model.append(QVariant::fromValue(object));
    }
    endInsertRows();
}

bool SidechainWTPrimeHistoryTableModel::GetWTPrimeInfoAtRow(int row, uint256& hash) const
{
    if (row >= model.size())
        return false;

    if (!model[row].canConvert<WTPrimeHistoryTableObject>())
        return false;

    WTPrimeHistoryTableObject object = model[row].value<WTPrimeHistoryTableObject>();

    hash = uint256S(object.hash.toStdString());

    return true;
}

void SidechainWTPrimeHistoryTableModel::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
}

void SidechainWTPrimeHistoryTableModel::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if (model)
    {
        connect(model, SIGNAL(numBlocksChanged(int, QDateTime, double, bool)),
                this, SLOT(UpdateModel()));
    }
}
