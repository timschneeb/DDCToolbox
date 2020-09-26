#include "tableproxy.h"

tableproxy::tableproxy(QTableWidget* _tw)
{
    tw = _tw;
}

void tableproxy::addRow(calibrationPoint_t cal){
    QTableWidgetItem *c0 = new QTableWidgetItem();
    QTableWidgetItem *c1 = new QTableWidgetItem();
    QTableWidgetItem *c2 = new QTableWidgetItem();
    QTableWidgetItem *c3 = new QTableWidgetItem();
    c0->setData(Qt::DisplayRole, (QString)cal.type);
    c1->setData(Qt::DisplayRole, cal.freq);
    c2->setData(Qt::DisplayRole, (double)cal.bw);
    c3->setData(Qt::DisplayRole, (double)cal.gain);
    c0->setData(Qt::UserRole, cal.id);
    tw->insertRow(tw->rowCount());
    tw->setItem(tw->rowCount()-1, 0, c0);
    tw->setItem(tw->rowCount()-1, 1, c1);
    tw->setItem(tw->rowCount()-1, 2, c2);
    tw->setItem(tw->rowCount()-1, 3, c3);
}

void tableproxy::editRow(calibrationPoint_t cal, int row){
    if(row < tw->rowCount()){
        tw->item(row,0)->setData(Qt::DisplayRole,(QString)cal.type);
        tw->item(row,1)->setData(Qt::DisplayRole,cal.freq);
        tw->item(row,2)->setData(Qt::DisplayRole,(double)cal.bw);
        tw->item(row,3)->setData(Qt::DisplayRole,(double)cal.gain);
        tw->item(row,0)->setData(Qt::UserRole, cal.id);
    }
}

void tableproxy::clearAll(){
    tw->clear();
    tw->setRowCount(0);
    tw->reset();
    tw->setHorizontalHeaderLabels(QStringList() << QObject::tr("Type") << QObject::tr("Frequency") << QObject::tr("Bandwidth/S") << QObject::tr("Gain"));
}
