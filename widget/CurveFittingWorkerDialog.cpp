#include "CurveFittingWorkerDialog.h"
#include "ui_CurveFittingWorkerDialog.h"

#include <utils/CurveFittingThread.h>

#include <QTimer>

CurveFittingWorkerDialog::CurveFittingWorkerDialog(const CurveFittingOptions& _options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CurveFittingWorkerDialog),
    options(_options)
{
    ui->setupUi(this);

    ui->progressBar->setRange(0,0);

    worker = new CurveFittingThread(options);
    connect(worker, &CurveFittingThread::finished, this, &QDialog::accept);

    QTimer::singleShot(400, [this]{
        ui->progressText->setText("Calculating...");
        worker->start(QThread::Priority::HighPriority);
    });
}

CurveFittingWorkerDialog::~CurveFittingWorkerDialog()
{
    delete ui;
    worker->deleteLater();
}

void CurveFittingWorkerDialog::reject()
{
    worker->cancel();
    QDialog::reject();
}

QVector<DeflatedBiquad> CurveFittingWorkerDialog::getResults() const
{
    return worker->getResults();
}

