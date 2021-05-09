#ifndef ADDPOINT_H
#define ADDPOINT_H

#include <QDialog>

#include "model/Biquad.h"

namespace Ui {
class addpoint;
}

class AddPointDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddPointDialog(QWidget *parent = nullptr);
    ~AddPointDialog();
    Biquad* getBiquad();

private:
    Ui::addpoint *ui;
    CustomFilter m_cfilter441;
    CustomFilter m_cfilter48;

};

#endif // ADDPOINT_H
