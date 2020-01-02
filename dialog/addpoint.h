#ifndef ADDPOINT_H
#define ADDPOINT_H

#include <QDialog>
#include "dialog/customfilterdialog.h"
#include "item/customfilterfactory.h"

namespace Ui {
class addpoint;
}

class addpoint : public QDialog
{
    Q_OBJECT

public:
    explicit addpoint(QWidget *parent = nullptr);
    ~addpoint();
    calibrationPoint_t returndata();

private:
    Ui::addpoint *ui;
    customFilter_t m_cfilter441;
    customFilter_t m_cfilter48;

};

#endif // ADDPOINT_H
