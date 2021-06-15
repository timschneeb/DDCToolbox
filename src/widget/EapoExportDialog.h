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

    QList<int> getResult();

private slots:
    void insertValue();
    void removeValue();

private:
    Ui::SamplerateChooseDialog *ui;
};

#endif // EAPOEXPORTDIALOG_H
