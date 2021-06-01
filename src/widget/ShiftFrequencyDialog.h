#ifndef SHIFTFREQ_H
#define SHIFTFREQ_H

#include <QDialog>
#include <QModelIndexList>

class FilterModel;

namespace Ui {
class shiftfreq;
}

class ShiftFrequencyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShiftFrequencyDialog(FilterModel* model, const QModelIndexList& indices, QWidget *parent = nullptr);
    ~ShiftFrequencyDialog();
    int getResult();

private slots:
    void confirm();

private:
    Ui::shiftfreq *ui;
    FilterModel *model;
    QModelIndexList indices;
};

#endif // SHIFTFREQ_H
