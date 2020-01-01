#ifndef CUSTOMFILTERITEM_H
#define CUSTOMFILTERITEM_H

#include <QWidget>
#include "biquad.h"

namespace Ui {
class CustomFilterItem;
}

class CustomFilterItem : public QWidget
{
    Q_OBJECT

public:
    explicit CustomFilterItem(QWidget *parent = nullptr);
    ~CustomFilterItem();
    void setCoefficients(customFilter_t coeffs);
    customFilter_t getCoefficients();

signals:
    void coefficientsUpdated(customFilter_t);

private slots:
    void updateText();

private:
    Ui::CustomFilterItem *ui;
    customFilter_t m_cfilter;
};

#endif // CUSTOMFILTERITEM_H
