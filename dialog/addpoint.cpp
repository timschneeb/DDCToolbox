#include "addpoint.h"
#include "ui_addpoint.h"
#include "biquad.h"
#include "filtertypes.h"
#include <list>
#include <QDebug>
addpoint::addpoint(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addpoint)
{
    ui->setupUi(this);

    connect(ui->ftype,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,[this](int index){
        biquad::Type type = (biquad::Type)(index);
        switch (type) {
        case biquad::UNITY_GAIN:
        case biquad::CUSTOM:
            ui->freq->setEnabled(false);
            break;
        default:
            ui->freq->setEnabled(true);
        }
        switch (type) {
        case biquad::UNITY_GAIN:
        case biquad::ONEPOLE_LOWPASS:
        case biquad::ONEPOLE_HIGHPASS:
        case biquad::CUSTOM:
            ui->bw->setEnabled(false);
            break;
        default:
            ui->bw->setEnabled(true);
        }
        switch (type) {
        case biquad::PEAKING:
        case biquad::LOW_SHELF:
        case biquad::UNITY_GAIN:
        case biquad::HIGH_SHELF:
            ui->gain->setEnabled(true);
            break;
        default:
            ui->gain->setEnabled(false);
        }
        qDebug() << typeToString(type);
    });
}

addpoint::~addpoint()
{
    delete ui;
}

addp_response_t addpoint::returndata(){
    std::vector<double> data;
    if(stringToType(ui->ftype->currentText())==biquad::CUSTOM ||
            ui->ftype->currentText()==biquad::UNITY_GAIN)
        data.push_back(1.0);
        else
        data.push_back((double)ui->freq->value());

    data.push_back(ui->bw->value());
    data.push_back(ui->gain->value());

    addp_response_t addp;
    addp.values = data;
    addp.filtertype = ui->ftype->currentText();
    return addp;
}
