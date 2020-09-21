// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// TODO rename BITCOIN_QT_AMOUNTFIELD_H once the old version is removed
#ifndef BITCOIN_AMOUNTFIELD_H
#define BITCOIN_AMOUNTFIELD_H

#include <amount.h>

#include <QWidget>

namespace Ui {
class AmountField;
}

class QLabel;

class AmountField : public QWidget
{
    Q_OBJECT

public:
    explicit AmountField(QWidget *parent = 0);
    ~AmountField();

    /** Get CAmount value */
    CAmount value(bool *value=0) const;

    /** Set QString value from CAmount */
    void setValue(const CAmount& value);

    /** Make read-only **/
    void setReadOnly(bool fReadOnly);

    /** Perform input validation, mark field as invalid if not valid. */
    bool validate();

    /** Mark current value as invalid / valid in UI. (change style) */
    void setValid(bool valid);

    /** Change unit used to display amount. */
    void setDisplayUnit(int unit);

    /** Make field empty and ready for new input. */
    void clear();

    /** Enable/Disable. */
    void setEnabled(bool fEnabled);

    void setDisplayMode();

Q_SIGNALS:
    void valueChanged();

protected:
    /** Intercept focus-in event and ',' key presses */
    bool eventFilter(QObject *object, QEvent *event);

private:
    Ui::AmountField *ui;
    int unit;
    QLabel* labelUnit;

private Q_SLOTS:
    void ValidateAmount();
};

#endif // BITCOIN_AMOUNTFIELD_H
