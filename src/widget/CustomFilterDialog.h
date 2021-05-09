#ifndef CUSTOMFILTERDIALOG_H
#define CUSTOMFILTERDIALOG_H

#include <QDialog>
#include "model/Biquad.h"

namespace Ui {
class customfilterdialog;
}

class CustomFilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CustomFilterDialog(QWidget *parent = nullptr);
    ~CustomFilterDialog();
    CustomFilter getCoefficients(bool use48000);
    void setCoefficients(CustomFilter c441, CustomFilter c48);

private:
    Ui::customfilterdialog *ui;
};

#endif // CUSTOMFILTERDIALOG_H
