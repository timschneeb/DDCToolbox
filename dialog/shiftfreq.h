#ifndef SHIFTFREQ_H
#define SHIFTFREQ_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui {
class shiftfreq;
}

class shiftfreq : public QDialog
{
    Q_OBJECT

public:
    explicit shiftfreq(QWidget *parent = nullptr);
    ~shiftfreq();
    void setRange(std::vector<calibrationPoint_t> );
    int getResult();

private slots:
    void validate();
    void confirm();

private:
    Ui::shiftfreq *ui;
    std::vector<calibrationPoint_t> cal_table;
};

#endif // SHIFTFREQ_H
