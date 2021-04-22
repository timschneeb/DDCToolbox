#ifndef CURVEFITTINGWORKERDIALOG_H
#define CURVEFITTINGWORKERDIALOG_H

#include <QDialog>

#include <model/CurveFittingOptions.h>
#include <model/DeflatedBiquad.h>

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

    QVector<DeflatedBiquad> getResults() const;

private:
    Ui::CurveFittingWorkerDialog *ui;

    CurveFittingOptions options;
    CurveFittingThread* worker;
};

#endif // CURVEFITTINGWORKERDIALOG_H
