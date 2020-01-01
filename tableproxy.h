#ifndef TABLEPROXY_H
#define TABLEPROXY_H

#include <QTableWidget>
#include "filtertypes.h"

class tableproxy
{
public:
    tableproxy(QTableWidget* _tw);
    void addRow(calibrationPoint_t);
    void editRow(calibrationPoint_t cal, int row);
    void clearAll();

private:
    QTableWidget* tw;
};

#endif // TABLEPROXY_H
