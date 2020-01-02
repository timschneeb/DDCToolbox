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
    customFilter_t getCoefficients(bool use48000);
    void setCoefficients(customFilter_t c441, customFilter_t c48);

private:
    Ui::customfilterdialog *ui;
};

#endif // CUSTOMFILTERDIALOG_H
