#include "CurveFittingDialog.h"
#include "ui_CurveFittingDialog.h"

#include <QDesktopServices>
#include <QFileDialog>

CurveFittingDialog::CurveFittingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CurveFittingDialog)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);

    connect(ui->fileSelection, &QAbstractButton::clicked, this, &CurveFittingDialog::selectFile);
    connect(ui->projectLink, &QAbstractButton::clicked, this, &CurveFittingDialog::visitProject);
}

CurveFittingDialog::~CurveFittingDialog()
{
    delete ui;
}

void CurveFittingDialog::selectFile()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open CSV dataset"),
                                                "", tr("CSV dataset (*.csv)"));
    if(file.isEmpty())
        return;

    ui->filePath->setText(file);
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
}

void CurveFittingDialog::visitProject()
{
    // TODO: Update project link
    QDesktopServices::openUrl(QUrl("https://github.com/james34602/"));
}
