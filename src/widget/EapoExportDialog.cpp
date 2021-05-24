#include "EapoExportDialog.h"
#include "ui_EapoExportDialog.h"

EapoExportDialog::EapoExportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SamplerateChooseDialog)
{
    ui->setupUi(this);
}

EapoExportDialog::~EapoExportDialog()
{
    delete ui;
}

int EapoExportDialog::getResult(){
    return ui->spinBox->value();
}
