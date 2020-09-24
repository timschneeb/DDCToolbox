#ifndef CUSTOMFILTERFACTORY_H
#define CUSTOMFILTERFACTORY_H
#include "item/customfilteritem.h"
#include "mainwindow.h"
#include "biquad.h"
#include <QTableWidget>
#include <QDebug>

inline customFilter_t defaultCustomFilter(){
    customFilter_t m_cfilter;
    m_cfilter.a0 = 1.0;
    m_cfilter.a1 = 0.0;
    m_cfilter.a2 = 0.0;
    m_cfilter.b0 = 1.0;
    m_cfilter.b1 = 0.0;
    m_cfilter.b2 = 0.0;
    return m_cfilter;
}
inline CustomFilterItem* newCustomFilter(customFilter_t c441, customFilter_t c48,QTableWidget* tw, int row){
    CustomFilterItem* cf_item = new CustomFilterItem;
    cf_item->setCoefficients(c441,c48);

    tw->setCellWidget(row,3,cf_item);
    QObject::connect(cf_item, &CustomFilterItem::coefficientsUpdated,
                     [tw,cf_item](customFilter_t previous441,customFilter_t previous48){
        Global::old_custom441 = previous441;
        Global::old_custom48 = previous48;
        CustomFilterItem* origin = cf_item;
        for(int i = 0; i < tw->rowCount(); i++){
            if(origin==tw->cellWidget(i,3)){
                emit tw->itemChanged(tw->item(i,3));
            }
        }
    });
    return cf_item;
}
#endif // CUSTOMFILTERFACTORY_H
