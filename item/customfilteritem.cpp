#include "customfilteritem.h"
#include "ui_customfilteritem.h"
#include "../dialog/customfilterdialog.h"

CustomFilterItem::CustomFilterItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CustomFilterItem)
{
    ui->setupUi(this);

    ui->coeffText->hide();

    m_cfilter.a0 = 1.0;
    m_cfilter.a1 = 0.0;
    m_cfilter.a2 = 0.0;
    m_cfilter.b0 = 1.0;
    m_cfilter.b1 = 0.0;
    m_cfilter.b2 = 0.0;
    updateText();

    connect(ui->configure,&QPushButton::clicked,this,[this]{
        customfilterdialog* cd = new customfilterdialog;
        cd->setCoefficients(m_cfilter);
        if(cd->exec()){
            customFilter_t previous = m_cfilter;
            m_cfilter = cd->getCoefficients();
            updateText();
            emit coefficientsUpdated();
        }
    });
}

CustomFilterItem::~CustomFilterItem()
{
    delete ui;
}

void CustomFilterItem::updateText(){
    QString strbuilder("");
    strbuilder += "a0 = " + QString::number(m_cfilter.a0) + ", ";
    strbuilder += "a1 = " + QString::number(m_cfilter.a1) + ", ";
    strbuilder += "a2 = " + QString::number(m_cfilter.a2) + ", ";
    strbuilder += "b0 = " + QString::number(m_cfilter.b0) + ", ";
    strbuilder += "b1 = " + QString::number(m_cfilter.b1) + ", ";
    strbuilder += "b2 = " + QString::number(m_cfilter.b2);
    ui->coeffText->setText(strbuilder);
}

void CustomFilterItem::setCoefficients(customFilter_t coeffs){
    m_cfilter = coeffs;
    updateText();
}

customFilter_t CustomFilterItem::getCoefficients(){
    return m_cfilter;
}
