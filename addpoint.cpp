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

std::list<double> addpoint::returndata(){
    std::list<double> data;
    data.push_back((double)ui->freq->value());
    data.push_back(ui->bw->value());
    data.push_back(ui->gain->value());
    return data;
}
