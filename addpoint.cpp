#include "addpoint.h"
#include "ui_addpoint.h"
#include <list>

addpoint::addpoint(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addpoint)
{
    ui->setupUi(this);
}

addpoint::~addpoint()
{
    delete ui;
}

addp_response_t addpoint::returndata(){
    std::vector<double> data;
    data.push_back((double)ui->freq->value());
    data.push_back(ui->bw->value());
    data.push_back(ui->gain->value());

    addp_response_t addp;
    addp.values = data;
    addp.filtertype = ui->ftype->currentText();
    return addp;
}
