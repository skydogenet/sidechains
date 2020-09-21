// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_PAYMENTREQUESTDIALOG_H
#define BITCOIN_QT_PAYMENTREQUESTDIALOG_H

#include <qt/guiutil.h>

#include <QDialog>
#include <QHeaderView>
#include <QItemSelection>
#include <QKeyEvent>
#include <QMenu>
#include <QPoint>
#include <QVariant>

class PlatformStyle;
class WalletModel;

namespace Ui {
class PaymentRequestDialog;
}

class PaymentRequestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PaymentRequestDialog(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~PaymentRequestDialog();

    void setModel(WalletModel *model);

private:
    Ui::PaymentRequestDialog *ui;

    GUIUtil::TableViewLastColumnResizingFixer *columnResizingFixer;
    WalletModel *model;
    QMenu *contextMenu;
    const PlatformStyle *platformStyle;

    QModelIndex selectedRow();
    void copyColumnToClipboard(int column);
    virtual void resizeEvent(QResizeEvent *event);

    enum ColumnWidths {
        DATE_COLUMN_WIDTH = 130,
        LABEL_COLUMN_WIDTH = 120,
        AMOUNT_MINIMUM_COLUMN_WIDTH = 180,
        MINIMUM_COLUMN_WIDTH = 130
    };

protected:
    virtual void keyPressEvent(QKeyEvent *event);

public Q_SLOTS:
    void on_receiveButton_clicked();
    void on_showRequestButton_clicked();
    void on_removeRequestButton_clicked();
    void on_recentRequestsView_doubleClicked(const QModelIndex &index);
    void recentRequestsView_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void updateDisplayUnit();
    void showMenu(const QPoint &point);
    void copyURI();
    void copyLabel();
    void copyMessage();
    void copyAmount();
    void clear();
};

#endif // BITCOIN_QT_PAYMENTREQUESTDIALOG_H
