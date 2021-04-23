#include "CurveFittingWorkerDialog.h"
#include "ui_CurveFittingWorkerDialog.h"

#include <utils/CurveFittingWorker.h>

#include <QPushButton>
#include <QTimer>

CurveFittingWorkerDialog::CurveFittingWorkerDialog(const CurveFittingOptions& _options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CurveFittingWorkerDialog),
    options(_options)
{
    ui->setupUi(this);

    ui->progressBar->setRange(0,0);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setVisible(false);

    /* Plot 1 */
    ui->rmsePlot->yAxis->setRange(QCPRange(0, 10));
    ui->rmsePlot->yAxis->setLabel("Best cost");
    ui->rmsePlot->xAxis->setRange(QCPRange(0, 1));
    ui->rmsePlot->xAxis->setLabel("Iteration");
    ui->rmsePlot->rescaleAxes();
    ui->rmsePlot->addGraph();
    ui->rmsePlot->graph()->setAdaptiveSampling(false);

    worker = new CurveFittingWorker(options);
    worker->moveToThread(&thread);
    connect(worker, &CurveFittingWorker::finished, this, &CurveFittingWorkerDialog::workerFinished);
    connect(this, &CurveFittingWorkerDialog::beginWork, worker, &CurveFittingWorker::run);
    connect(worker, &CurveFittingWorker::historyDataReceived, this, &CurveFittingWorkerDialog::historyDataReceived);
    thread.start();

    QTimer::singleShot(400, [this]{
        ui->progressText->setText("Calculating...");
        emit beginWork();
    });
}

CurveFittingWorkerDialog::~CurveFittingWorkerDialog()
{
    delete ui;
    worker->deleteLater();
}

void CurveFittingWorkerDialog::reject()
{
    thread.terminate();
    QDialog::reject();
}

void CurveFittingWorkerDialog::workerFinished()
{
    thread.terminate();

    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue(100);
    ui->progressBar->setTint(QColor(62, 179, 0));
    ui->progressText->setText("Calculation finished");

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setVisible(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setVisible(true);
}

void CurveFittingWorkerDialog::historyDataReceived(float mse, QVector<float> currentResult)
{
    // TODO: Handle currentResult
    Q_UNUSED(currentResult)

    iteration++;
    ui->rmsePlot->graph()->addData(iteration, mse);
    if(iteration >= ui->rmsePlot->xAxis->range().upper){
        ui->rmsePlot->xAxis->setRange(0, iteration);
    }
    if(mse >= ui->rmsePlot->yAxis->range().upper){
        ui->rmsePlot->yAxis->setRange(0, mse);
    }

    ui->rmsePlot->replot(QCustomPlot::rpQueuedReplot);

    ui->stat_iteration->setText(QString::number(iteration));
    ui->stat_mse->setText(QString::number(mse));
}

QVector<DeflatedBiquad> CurveFittingWorkerDialog::getResults() const
{
    return worker->getResults();
}

