#ifndef CUSTOMFILTERDIALOG_H
#define CUSTOMFILTERDIALOG_H

#include "mainwindow.h"
#include <QDialog>

namespace Ui {
class customfilterdialog;
}

class customfilterdialog : public QDialog
{
    Q_OBJECT

public:
    explicit customfilterdialog(QWidget *parent = nullptr);
    ~customfilterdialog();
    customFilter_t getCoefficients();
    void setCoefficients(customFilter_t coeff);

private:
    Ui::customfilterdialog *ui;
};

#endif // CUSTOMFILTERDIALOG_H
