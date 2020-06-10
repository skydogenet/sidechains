// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/sidechainwttablemodel.h>

#include <QTimer>

#include <qt/bitcoinunits.h>
#include <qt/clientmodel.h>
#include <qt/guiconstants.h>
#include <qt/optionsmodel.h>
#include <qt/walletmodel.h>

#include <sidechain.h>
#include <txdb.h>
#include <validation.h>

Q_DECLARE_METATYPE(WTTableObject)

SidechainWTTableModel::SidechainWTTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

int SidechainWTTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return model.size();
}

int SidechainWTTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 4;
}

QVariant SidechainWTTableModel::data(const QModelIndex &index, int role) const
{
    if (!walletModel)
        return false;

    if (!index.isValid())
        return false;

    int row = index.row();
    int col = index.column();

    if (!model.at(row).canConvert<WTTableObject>())
        return QVariant();

    WTTableObject object = model.at(row).value<WTTableObject>();

    int unit = walletModel->getOptionsModel()->getDisplayUnit();

    switch (role) {
    case Qt::DisplayRole:
    {
        // Amount
        if (col == 0) {
            QString amount = BitcoinUnits::formatWithUnit(unit, object.amount - object.amountMainchainFee, false,
                    BitcoinUnits::separatorAlways);
            return amount;
        }
        // Mainchain fee amount
        if (col == 1) {
            QString fee = BitcoinUnits::formatWithUnit(unit, object.amountMainchainFee, false,
                    BitcoinUnits::separatorAlways);
            return fee;
        }
        // Status
        if (col == 2) {
            return object.status;
        }
        // Destination address
        if (col == 3) {
            return object.destination;
        }
    }
    }
    return QVariant();
}

QVariant SidechainWTTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return QString("Amount");
            case 1:
                return QString("Mainchain Fee");
            case 2:
                return QString("Status");
            case 3:
                return QString("Destination");
            }
        }
    }
    return QVariant();
}

void SidechainWTTableModel::UpdateModel()
{
    // TODO there are many ways to improve the efficiency of this

    // Get all of the current WT(s) into one vector
    std::vector<SidechainWT> vWT;
    vWT = psidechaintree->GetWTs(SIDECHAIN_TEST);

    // Look for updates to WT(s) & their status already cached by the model
    // and update our model / view.
    //
    // Also look for WT(s) which have been removed, and remove them from our
    // model / view.
    std::vector<WTTableObject> vRemoved;
    for (int i = 0; i < model.size(); i++) {
        if (!model[i].canConvert<WTTableObject>())
            return;

        WTTableObject object = model[i].value<WTTableObject>();

        bool fFound = false;

        // Check if the WT should still be in the table and make sure we set
        // it with the current status
        for (const SidechainWT& wt : vWT) {
            // Check if the WT was removed or the status has changed.
            if (wt.GetID() == object.id) {
                if (wt.status != WT_UNSPENT)
                    break;

                fFound = true;

                // Check for updates to status
                QString status = QString::fromStdString(wt.GetStatusStr());
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

        // Add to vector of WT(s) to be removed from model / view
        if (!fFound) {
            vRemoved.push_back(object);
        }
    }

    // Loop through the model and remove deleted WT(s)
    for (int i = 0; i < model.size(); i++) {
        if (!model[i].canConvert<WTTableObject>())
            return;

        WTTableObject object = model[i].value<WTTableObject>();

        for (const WTTableObject& wt : vRemoved) {
            if (wt.id == object.id) {
                beginRemoveRows(QModelIndex(), i, i);
                model[i] = model.back();
                model.pop_back();
                endRemoveRows();
            }
        }
    }

    // Check for new WT(s)
    std::vector<SidechainWT> vNew;
    for (const SidechainWT& wt : vWT) {
        if (wt.status != WT_UNSPENT)
            continue;

        bool fFound = false;

        for (const QVariant& qv : model) {
            if (!qv.canConvert<WTTableObject>())
                return;

            WTTableObject object = qv.value<WTTableObject>();

            if (wt.GetID() == object.id)
                fFound = true;
        }
        if (!fFound)
            vNew.push_back(wt);
    }

    if (vNew.empty())
        return;

    // Add new WT(s) if we need to
    beginInsertRows(QModelIndex(), model.size(), model.size() + vNew.size() - 1);
    for (const SidechainWT& wt : vNew) {
        WTTableObject object;

        // Insert new WT into table
        object.id = wt.GetID();
        object.amount = wt.amount;
        object.amountMainchainFee = wt.mainchainFee;
        object.status = QString::fromStdString(wt.GetStatusStr());
        object.destination = QString::fromStdString(wt.strDestination);
        model.append(QVariant::fromValue(object));
    }
    endInsertRows();
}

void SidechainWTTableModel::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
}

void SidechainWTTableModel::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if (model)
    {
        connect(model, SIGNAL(numBlocksChanged(int, QDateTime, double, bool)),
                this, SLOT(UpdateModel()));

        UpdateModel();
    }
}
