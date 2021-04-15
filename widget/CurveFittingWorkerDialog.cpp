#include "CurveFittingWorkerDialog.h"
#include "ui_CurveFittingWorkerDialog.h"

CurveFittingWorkerDialog::CurveFittingWorkerDialog(const QVector<float>& _freq, const QVector<float>& _gain, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CurveFittingWorkerDialog)
{
    ui->setupUi(this);
    freq = _freq.data();
    gain = _gain.data();
}

CurveFittingWorkerDialog::~CurveFittingWorkerDialog()
{
    delete ui;
}

