#ifndef CONFGENERATORDIALOG_H
#define CONFGENERATORDIALOG_H

#include <QDialog>

namespace Ui {
class ConfGeneratorDialog;
}

class ConfGeneratorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfGeneratorDialog(QWidget *parent = 0, bool fDialogIn = true);
    ~ConfGeneratorDialog();

private Q_SLOTS:
    void on_pushButtonClose_clicked();
    void on_pushButtonApply_clicked();
    void on_pushButtonRandom_clicked();

Q_SIGNALS:
    void Applied();

private:
    Ui::ConfGeneratorDialog *ui;

    bool WriteConfigFiles(const QString& strUser, const QString& strPass);

    // Whether this widget is being shown as a popup dialog
    bool fDialog;
};

#endif // CONFGENERATORDIALOG_H
