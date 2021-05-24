#ifndef EAPOEXPORTDIALOG_H
#define EAPOEXPORTDIALOG_H

#include <QDialog>

namespace Ui {
class SamplerateChooseDialog;
}

class EapoExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EapoExportDialog(QWidget *parent = nullptr);
    ~EapoExportDialog();

    int getResult();

private:
    Ui::SamplerateChooseDialog *ui;
};

#endif // EAPOEXPORTDIALOG_H
