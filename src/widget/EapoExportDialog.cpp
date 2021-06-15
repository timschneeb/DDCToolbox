#include "EapoExportDialog.h"
#include "ui_EapoExportDialog.h"

#include <QMessageBox>

EapoExportDialog::EapoExportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SamplerateChooseDialog)
{
    ui->setupUi(this);

    connect(ui->rateInsert, &QAbstractButton::clicked, this, &EapoExportDialog::insertValue);
    connect(ui->rateRemove, &QAbstractButton::clicked, this, &EapoExportDialog::removeValue);
}

EapoExportDialog::~EapoExportDialog()
{
    delete ui;
}

QList<int> EapoExportDialog::getResult(){
    QList<int> rates;
    for(int i = 0; i < ui->rateList->count(); i++){
        rates.append(ui->rateList->item(i)->text().toInt());
    }
    return rates;
}

void EapoExportDialog::insertValue(){
    int value = ui->rateInput->value();

    for(int i = 0; i < ui->rateList->count(); i++){
        if(value == ui->rateList->item(i)->text().toInt()){
            QMessageBox::warning(this, "Error", "Value has already been added to the list");
            return;
        }
    }

    ui->rateList->addItem(QString::number(value));
}

void EapoExportDialog::removeValue(){

    if(ui->rateList->selectedItems().count() < 1){
        QMessageBox::warning(this, "Error", "Please select a few list items and try again");
        return;
    }

    qDeleteAll(ui->rateList->selectedItems());
}
