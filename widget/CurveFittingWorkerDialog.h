#ifndef CURVEFITTINGWORKERDIALOG_H
#define CURVEFITTINGWORKERDIALOG_H

#include <QDialog>

namespace Ui {
class CurveFittingWorkerDialog;
}

class CurveFittingWorkerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CurveFittingWorkerDialog(const QVector<float>& freq, const QVector<float>& gain, QWidget *parent = nullptr);
    ~CurveFittingWorkerDialog();

private:
    Ui::CurveFittingWorkerDialog *ui;

    const float* freq;
    const float* gain;
};

#endif // CURVEFITTINGWORKERDIALOG_H
