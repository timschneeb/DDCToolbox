#include "customfilterdialog.h"
#include "ui_customfilterdialog.h"

customfilterdialog::customfilterdialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::customfilterdialog)
{
    ui->setupUi(this);
}

customfilterdialog::~customfilterdialog()
{
    delete ui;
}

void customfilterdialog::setCoefficients(customFilter_t coeff){
    ui->a0->setValue(coeff.a0);
    ui->a1->setValue(coeff.a1);
    ui->a2->setValue(coeff.a2);
    ui->b0->setValue(coeff.b0);
    ui->b1->setValue(coeff.b1);
    ui->b2->setValue(coeff.b2);
}

customFilter_t customfilterdialog::getCoefficients(){
    customFilter_t coeff;
    coeff.a0 = ui->a0->value();
    coeff.a1 = ui->a1->value();
    coeff.a2 = ui->a2->value();
    coeff.b0 = ui->b0->value();
    coeff.b1 = ui->b1->value();
    coeff.b2 = ui->b2->value();
    return coeff;
}
