#include "CurveFittingWorkerDialog.h"
#include "ui_CurveFittingWorkerDialog.h"

#include <utils/CurveFittingThread.h>

CurveFittingWorkerDialog::CurveFittingWorkerDialog(const CurveFittingOptions& _options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CurveFittingWorkerDialog),
    options(_options)
{
    ui->setupUi(this);

    worker = new CurveFittingThread(options);
}

CurveFittingWorkerDialog::~CurveFittingWorkerDialog()
{
    delete ui;
    worker->deleteLater();
}

void CurveFittingWorkerDialog::open()
{
    worker->start(QThread::Priority::HighPriority);
    QDialog::open();
}

void CurveFittingWorkerDialog::reject()
{
    worker->cancel();
    QDialog::reject();
}

