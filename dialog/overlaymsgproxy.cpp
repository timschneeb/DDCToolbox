#include "overlaymsgproxy.h"
#include "dialog/qmessageoverlay.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QGridLayout>
#include <QPushButton>

OverlayMsgProxy::OverlayMsgProxy(QWidget* _obj)
{
    lightBox = new QMessageOverlay(_obj,false);
    obj = _obj;
}

void OverlayMsgProxy::openBase(QString title, QString desc){
    QLabel* lbTitle = new QLabel(title);
    lbTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: white");

    QLabel* lbDescription = new QLabel(desc);
    lbDescription->setStyleSheet("font-size: 14px; color: white");
    lbDescription->setWordWrap(true);

    QGridLayout* lbLayout = new QGridLayout;
    lbLayout->setRowStretch(0, 1);

    lbLayout->setColumnStretch(0, 1);
    lbLayout->addWidget(lbTitle, 1, 1);
    lbLayout->setColumnStretch(3, 1);
    lbLayout->addWidget(lbDescription, 2, 1, 1, 2);

    lbLayout->setRowStretch(4, 1);

    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect();
    lightBox->setGraphicsEffect(eff);
    lightBox->show();

    QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
    a->setDuration(300);
    a->setStartValue(0);
    a->setEndValue(1);
    a->setEasingCurve(QEasingCurve::InBack);
    a->start(QPropertyAnimation::DeleteWhenStopped);

    lightBox->setLayout(lbLayout);
}

void OverlayMsgProxy::hide(){
    QGraphicsOpacityEffect *eff2 = new QGraphicsOpacityEffect();
    lightBox->setGraphicsEffect(eff2);
    QPropertyAnimation *a = new QPropertyAnimation(eff2,"opacity");
    a->setDuration(300);
    a->setStartValue(1);
    a->setEndValue(0);
    a->setEasingCurve(QEasingCurve::OutBack);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    connect(a,&QAbstractAnimation::finished, [this](){
        if(lightBox != nullptr)
            lightBox->hide();
    });
}
