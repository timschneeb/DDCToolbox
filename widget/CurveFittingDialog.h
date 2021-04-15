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

protected slots:
    void accept();

private slots:
    void parseCsv();

private:
    void selectFile();
    void visitProject();
    void setStatus(bool success, QString text);

    Ui::CurveFittingDialog *ui;

    std::vector<float> freq;
    std::vector<float> gain;
};

#endif // CURVEFITTINGDIALOG_H
