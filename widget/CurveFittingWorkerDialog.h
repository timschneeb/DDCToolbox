#ifndef CURVEFITTINGWORKERDIALOG_H
#define CURVEFITTINGWORKERDIALOG_H

#include <QDialog>

#include <model/CurveFittingOptions.h>

class CurveFittingThread;

namespace Ui {
class CurveFittingWorkerDialog;
}

class CurveFittingWorkerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CurveFittingWorkerDialog(const CurveFittingOptions &_options, QWidget *parent = nullptr);
    ~CurveFittingWorkerDialog();

    virtual void reject();

private:
    Ui::CurveFittingWorkerDialog *ui;

    CurveFittingOptions options;
    CurveFittingThread* worker;
};

#endif // CURVEFITTINGWORKERDIALOG_H
