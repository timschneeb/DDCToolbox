#ifndef CUSTOMFILTERITEM_H
#define CUSTOMFILTERITEM_H

#include <QWidget>
#include "model/Biquad.h"

namespace Ui {
class CustomFilterItem;
}

class CustomFilterItem : public QWidget
{
    Q_OBJECT

public:
    explicit CustomFilterItem(QWidget *parent = nullptr);
    ~CustomFilterItem();
    void setCoefficients(CustomFilter c441, CustomFilter c48);
    CustomFilter getCoefficients(bool use48000);

signals:
    void coefficientsUpdated(CustomFilter prev44100, CustomFilter prev48000,
                             CustomFilter c44100, CustomFilter c48000);

private slots:
    void updateText();

private:
    Ui::CustomFilterItem *ui;
    CustomFilter m_cfilter441;
    CustomFilter m_cfilter48;
};

#endif // CUSTOMFILTERITEM_H
