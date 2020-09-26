#include "addpoint.h"
#include "Biquad.h"
#include "ui_addpoint.h"
#include "utils/filtertypes.h"
#include <QDebug>
#include <list>

addpoint::addpoint(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addpoint)
{
    ui->setupUi(this);

    m_cfilter441 = defaultCustomFilter();
    m_cfilter48 = defaultCustomFilter();
    ui->custom_configure->setEnabled(false);

    connect(ui->ftype,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,[this](int index){
        Biquad::Type type = (Biquad::Type)(index);
        switch (type) {
        case Biquad::UNITY_GAIN:
        case Biquad::CUSTOM:
            ui->freq->setEnabled(false);
            break;
        default:
            ui->freq->setEnabled(true);
        }
        switch (type) {
        case Biquad::UNITY_GAIN:
        case Biquad::ONEPOLE_LOWPASS:
        case Biquad::ONEPOLE_HIGHPASS:
        case Biquad::CUSTOM:
            ui->bw->setEnabled(false);
            break;
        default:
            ui->bw->setEnabled(true);
        }
        switch (type) {
        case Biquad::PEAKING:
        case Biquad::LOW_SHELF:
        case Biquad::UNITY_GAIN:
        case Biquad::HIGH_SHELF:
            ui->gain->setEnabled(true);
            break;
        default:
            ui->gain->setEnabled(false);
        }
        switch (type) {
        case Biquad::CUSTOM:
            ui->custom_configure->setEnabled(true);
            break;
        default:
            ui->custom_configure->setEnabled(false);
        }
    });

    connect(ui->custom_configure,&QPushButton::clicked,this,[this]{
        customfilterdialog* cd = new customfilterdialog;
        cd->setCoefficients(defaultCustomFilter(),defaultCustomFilter());
        if(cd->exec()){
            m_cfilter441 = cd->getCoefficients(false);
            m_cfilter48 = cd->getCoefficients(true);
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
    if(stringToType(ui->ftype->currentText())==Biquad::CUSTOM ||
            ui->ftype->currentText()==Biquad::UNITY_GAIN)
        ret.freq = 1;
    else
        ret.freq = ui->freq->value();
    ret.bw = ui->bw->value();
    ret.gain = ui->gain->value();
    ret.custom441 = m_cfilter441;
    ret.custom48 = m_cfilter48;
    ret.type = stringToType(ui->ftype->currentText());
    return ret;
}
