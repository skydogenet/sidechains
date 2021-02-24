// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/amountfield.h>
#include <qt/forms/ui_amountfield.h>

#include <QLabel>
#include <QLineEdit>

#include <qt/bitcoinunits.h>
#include <qt/guiconstants.h>

AmountField::AmountField(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AmountField),
    unit(0)
{
    ui->setupUi(this);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    ui->lineEditAmount->setLayout(layout);
    ui->lineEditAmount->setFixedSize(200, 28);

    labelUnit = new QLabel(this);
    labelUnit->setEnabled(false); // So that it displays with disabled style
    labelUnit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    labelUnit->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    layout->addStretch(50);
    layout->addWidget(labelUnit);

    ui->lineEditAmount->installEventFilter(this);
    connect(ui->lineEditAmount, SIGNAL(editingFinished()),
            this, SLOT(ValidateAmount()));

    setDisplayUnit(unit);
    setValue(CAmount(0));

    this->updateGeometry();
}

AmountField::~AmountField()
{
    delete ui;
}

CAmount Parse(int unit, const QString &text, bool *valid_out=0)
{
    CAmount val = 0;
    bool fValid = BitcoinUnits::parse(unit, text, &val);
    if (fValid && (val < 0 || val > BitcoinUnits::maxMoney()))
        fValid = false;

    if(valid_out)
        *valid_out = fValid;

    return fValid ? val : 0;
}

void AmountField::clear()
{
    ui->lineEditAmount->clear();
}

void AmountField::setEnabled(bool fEnabled)
{
    ui->lineEditAmount->setEnabled(fEnabled);
}

bool AmountField::validate()
{
    bool valid = false;
    value(&valid);
    setValid(valid);
    return valid;
}

void AmountField::setValid(bool valid)
{
    if (valid)
        ui->lineEditAmount->setStyleSheet("");
    else
        ui->lineEditAmount->setStyleSheet(STYLE_INVALID);
}

bool AmountField::eventFilter(QObject *object, QEvent *event)
{
    // Set line edit valid if the user focuses on it again
    if (event->type() == QEvent::FocusIn)
        setValid(true);

    return QWidget::eventFilter(object, event);
}

CAmount AmountField::value(bool *valid_out) const
{
    return Parse(unit, ui->lineEditAmount->text(), valid_out);
}

void AmountField::setValue(const CAmount& value)
{
    QString str = BitcoinUnits::format(unit, value, false, BitcoinUnits::separatorAlways);
    ui->lineEditAmount->setText(str);

    Q_EMIT valueChanged();
}

void AmountField::setReadOnly(bool fReadOnly)
{
    ui->lineEditAmount->setReadOnly(fReadOnly);
}

void AmountField::setDisplayUnit(int newUnit)
{
    bool valid = false;
    CAmount val = value(&valid);

    unit = newUnit;

    if (valid)
        setValue(val);
    else
        clear();

    labelUnit->setText(BitcoinUnits::shortName(newUnit));
}

void AmountField::ValidateAmount()
{
    QString str = ui->lineEditAmount->text();

    if (str.isEmpty()) {
        setValid(false);
        return;
    }

    bool fValid = false;
    CAmount amount = Parse(unit, str, &fValid);

    if (!fValid) {
        setValid(false);
        return;
    }

    setValue(amount);
    setValid(true);
}

void AmountField::setDisplayMode()
{
    ui->lineEditAmount->setReadOnly(true);
    this->ensurePolished();
    this->setStyleSheet("QLineEdit[readOnly=\"true\"] {"
                        "background: rgba(0, 0, 0, 0%);"
                        "border-width: 2px;"
                        "border-style: solid;"
                        "border-color: rgba(0, 0, 0, 0%);"
                        "}");
    this->updateGeometry();
    this->ensurePolished();
}
