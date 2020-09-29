#include "AddPointDialog.h"
#include "ui_AddPointDialog.h"

AddPointDialog::AddPointDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addpoint)
{
    ui->setupUi(this);

    for(ulong i = 0; i < FilterType::string_map_size; i++){
        if(FilterType::string_map[i].first == FilterType::INVALID)
            continue;
        ui->ftype->addItem(FilterType::string_map[i].second);
    }

    m_cfilter441 = CustomFilter();
    m_cfilter48 = CustomFilter();
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
        CustomFilterDialog* cd = new CustomFilterDialog;
        cd->setCoefficients(CustomFilter(), CustomFilter());
        if(cd->exec()){
            m_cfilter441 = cd->getCoefficients(false);
            m_cfilter48 = cd->getCoefficients(true);
        }
    });
}

AddPointDialog::~AddPointDialog()
{
    delete ui;
}

Biquad* AddPointDialog::getBiquad(){
    Biquad* b = new Biquad();

    FilterType type = FilterType(ui->ftype->currentText());
    if(type == FilterType::CUSTOM)
        b->RefreshFilter(type,
                         m_cfilter441,
                         m_cfilter48);
    else
        b->RefreshFilter(type,
                         ui->gain->value(),
                         ui->freq->value(),
                         ui->bw->value());
    return b;
}
