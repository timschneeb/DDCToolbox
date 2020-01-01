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

    m_cfilter = defaultCustomFilter();
    ui->custom_configure->setEnabled(false);

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
        switch (type) {
        case biquad::CUSTOM:
            ui->custom_configure->setEnabled(true);
            break;
        default:
            ui->custom_configure->setEnabled(false);
        }
    });

    connect(ui->custom_configure,&QPushButton::clicked,this,[this]{
        customfilterdialog* cd = new customfilterdialog;
        cd->setCoefficients(defaultCustomFilter());
        if(cd->exec()){
            m_cfilter = cd->getCoefficients();
        }
    });
}

addpoint::~addpoint()
{
    delete ui;
}

calibrationPoint_t addpoint::returndata(){
    std::vector<double> data;
    calibrationPoint_t ret;
    if(stringToType(ui->ftype->currentText())==biquad::CUSTOM ||
            ui->ftype->currentText()==biquad::UNITY_GAIN)
        ret.freq = 1;
    else
        ret.freq = ui->freq->value();
    ret.bw = ui->bw->value();
    ret.gain = ui->gain->value();
    ret.custom = m_cfilter;
    ret.type = stringToType(ui->ftype->currentText());
    return ret;
}
