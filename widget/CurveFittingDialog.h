#ifndef CURVEFITTINGDIALOG_H
#define CURVEFITTINGDIALOG_H

#include <QDialog>

namespace Ui {
class CurveFittingDialog;
}

class CurveFittingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CurveFittingDialog(QWidget *parent = nullptr);
    ~CurveFittingDialog();

private:
    void selectFile();
    void visitProject();

    Ui::CurveFittingDialog *ui;
};

#endif // CURVEFITTINGDIALOG_H
