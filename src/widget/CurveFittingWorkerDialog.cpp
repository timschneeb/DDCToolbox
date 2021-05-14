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
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    /* Plot 1 */
    ui->rmsePlot->yAxis->setRange(QCPRange(0, 10));
    ui->rmsePlot->yAxis->setLabel("Best cost");
    ui->rmsePlot->xAxis->setRange(QCPRange(0, 1));
    ui->rmsePlot->xAxis->setLabel("Iteration");
    ui->rmsePlot->rescaleAxes();
    ui->rmsePlot->addGraph();
    ui->rmsePlot->graph()->setAdaptiveSampling(false);

    /* Plot 2 */
    ui->previewPlot->yAxis->setRange(options.obcGainRange().first, options.obcGainRange().second);
    ui->previewPlot->xAxis->setRange(QCPRange(0, 400));
    ui->previewPlot->xAxis->setLabel("Optimization history");
    ui->previewPlot->rescaleAxes();

    worker = new CurveFittingWorker(options);
    worker->moveToThread(&thread);
    connect(worker, &CurveFittingWorker::finished, this, &CurveFittingWorkerDialog::workerFinished);
    connect(this, &CurveFittingWorkerDialog::beginWork, worker, &CurveFittingWorker::run);
    connect(worker, &CurveFittingWorker::mseReceived, this, &CurveFittingWorkerDialog::mseReceived);
    connect(worker, &CurveFittingWorker::graphReceived, this, &CurveFittingWorkerDialog::graphReceived);
    thread.start();


    QTimer::singleShot(400, [this]{
        ui->progressText->setText("Calculating...");
        emit beginWork();
    });

    this->setWindowFlag(Qt::WindowCloseButtonHint, false);
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
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setVisible(true);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void CurveFittingWorkerDialog::mseReceived(float mse)
{
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

void CurveFittingWorkerDialog::graphReceived(std::vector<double> temp)
{
    g_iteration++;
    if((g_iteration % 120) != 0){
        return;
    }

    uint grid_size = temp.size();

    ui->previewPlot->clearGraphs();

    auto pGraph0 = ui->previewPlot->addGraph();
    pGraph0->setAdaptiveSampling(true);

    /*int xmax = ui->previewPlot->xAxis->range().upper;

    if((int)grid_size > xmax){
        xmax = grid_size;
    }*/

    for(uint i = 0; i < grid_size; i++){
        pGraph0->addData(i, (double)temp[i]);
    }

    ui->previewPlot->xAxis->setRange(0, grid_size);
    ui->previewPlot->replot(QCustomPlot::rpQueuedReplot);
}

QVector<DeflatedBiquad> CurveFittingWorkerDialog::getResults() const
{
    return worker->getResults();
}

