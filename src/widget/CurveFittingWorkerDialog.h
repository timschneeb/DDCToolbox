#ifndef CURVEFITTINGWORKERDIALOG_H
#define CURVEFITTINGWORKERDIALOG_H

#include <QDialog>
#include <QThread>

#include <model/CurveFittingOptions.h>
#include <model/DeflatedBiquad.h>

class CurveFittingWorker;

namespace Ui {
class CurveFittingWorkerDialog;
}

class CurveFittingWorkerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CurveFittingWorkerDialog(const CurveFittingOptions &_options, DoubleRange yAxis, QWidget *parent = nullptr);
    ~CurveFittingWorkerDialog();

    void reject() override;
    void accept() override;
    void closeEvent(QCloseEvent *) override;

    QVector<DeflatedBiquad> getResults() const;

signals:
    void beginWork();

private slots:
    void workerFinished();
    void mseReceived(float mse);
    void graphReceived(std::vector<double> temp);
    void stageChanged(uint stage, CurveFittingOptions::AlgorithmType algo);

private:
    Ui::CurveFittingWorkerDialog *ui;

    CurveFittingOptions options;
    CurveFittingWorker* worker;
    QThread thread;

    int iteration = 0;
    bool accepted = false;
};

#endif // CURVEFITTINGWORKERDIALOG_H
