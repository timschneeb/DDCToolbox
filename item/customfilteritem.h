#ifndef CUSTOMFILTERITEM_H
#define CUSTOMFILTERITEM_H

#include <QWidget>
#include "Biquad.h"

namespace Ui {
class CustomFilterItem;
}

class CustomFilterItem : public QWidget
{
    Q_OBJECT

public:
    explicit CustomFilterItem(QWidget *parent = nullptr);
    ~CustomFilterItem();
    void setCoefficients(customFilter_t c441, customFilter_t c48);
    customFilter_t getCoefficients(bool use48000);

signals:
    void coefficientsUpdated(customFilter_t,customFilter_t);

private slots:
    void updateText();

private:
    Ui::CustomFilterItem *ui;
    customFilter_t m_cfilter441;
    customFilter_t m_cfilter48;
};

#endif // CUSTOMFILTERITEM_H
