#ifndef SHIFTFREQ_H
#define SHIFTFREQ_H

#include <QDialog>

class FilterModel;

namespace Ui {
class shiftfreq;
}

class ShiftFrequencyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShiftFrequencyDialog(FilterModel* model, QWidget *parent = nullptr);
    ~ShiftFrequencyDialog();
    int getResult();

private slots:
    void confirm();

private:
    Ui::shiftfreq *ui;
    FilterModel *model;
};

#endif // SHIFTFREQ_H
