#include "customfilteritem.h"
#include "ui_customfilteritem.h"
#include "../dialog/customfilterdialog.h"

CustomFilterItem::CustomFilterItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CustomFilterItem)
{
    ui->setupUi(this);

    ui->coeffText->hide();

    m_cfilter441.a0 = 1.0;
    m_cfilter441.a1 = 0.0;
    m_cfilter441.a2 = 0.0;
    m_cfilter441.b0 = 1.0;
    m_cfilter441.b1 = 0.0;
    m_cfilter441.b2 = 0.0;
    m_cfilter48.a0 = 1.0;
    m_cfilter48.a1 = 0.0;
    m_cfilter48.a2 = 0.0;
    m_cfilter48.b0 = 1.0;
    m_cfilter48.b1 = 0.0;
    m_cfilter48.b2 = 0.0;
    updateText();

    connect(ui->configure,&QPushButton::clicked,this,[this]{
        customfilterdialog* cd = new customfilterdialog;
        cd->setCoefficients(m_cfilter441,m_cfilter48);
        if(cd->exec()){
            m_cfilter441 = cd->getCoefficients(false);
            m_cfilter48 = cd->getCoefficients(true);
            emit coefficientsUpdated(m_cfilter441,m_cfilter48);
            updateText();
        }
    });
}

CustomFilterItem::~CustomFilterItem()
{
    delete ui;
}

void CustomFilterItem::updateText(){
    QString strbuilder("");
    strbuilder += "a0 = " + QString::number(m_cfilter441.a0) + ", ";
    strbuilder += "a1 = " + QString::number(m_cfilter441.a1) + ", ";
    strbuilder += "a2 = " + QString::number(m_cfilter441.a2) + ", ";
    strbuilder += "b0 = " + QString::number(m_cfilter441.b0) + ", ";
    strbuilder += "b1 = " + QString::number(m_cfilter441.b1) + ", ";
    strbuilder += "b2 = " + QString::number(m_cfilter441.b2);
    ui->coeffText->setText(strbuilder);
}

void CustomFilterItem::setCoefficients(customFilter_t c441, customFilter_t c48){
    m_cfilter441 = c441;
    m_cfilter48 = c48;
    updateText();
}

customFilter_t CustomFilterItem::getCoefficients(bool use48000){
    if(use48000)
        return m_cfilter48;
    return m_cfilter441;
}
