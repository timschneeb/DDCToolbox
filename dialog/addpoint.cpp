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

    for(ulong i = 0; i < FilterType::string_map_size; i++){
        if(FilterType::string_map[i].first == FilterType::INVALID)
            continue;
        ui->ftype->addItem(FilterType::string_map[i].second);
    }

    m_cfilter441 = defaultCustomFilter();
    m_cfilter48 = defaultCustomFilter();
    ui->custom_configure->setEnabled(false);

    connect(ui->ftype,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,[this](int index){
        auto type = (FilterType)index;
        auto specs = type.getSpecs();

        ui->freq->setEnabled(specs.test(FilterType::SPEC_REQUIRE_FREQ));
        ui->bw->setEnabled(specs.test(FilterType::SPEC_REQUIRE_BW) || specs.test(FilterType::SPEC_REQUIRE_SLOPE));
        ui->gain->setEnabled(specs.test(FilterType::SPEC_REQUIRE_GAIN));

        ui->custom_configure->setEnabled(type == FilterType::CUSTOM);
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
    ret.freq = ui->freq->value();
    ret.bw = ui->bw->value();
    ret.gain = ui->gain->value();
    ret.custom441 = m_cfilter441;
    ret.custom48 = m_cfilter48;
    ret.type = FilterType(ui->ftype->currentText());
    return ret;
}
