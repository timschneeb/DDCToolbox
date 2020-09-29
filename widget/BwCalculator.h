#ifndef CALC_H
#define CALC_H

#include <QDialog>

namespace Ui {
class calc;
}

class BwCalculator : public QDialog
{
    Q_OBJECT

public:
    explicit BwCalculator(QWidget *parent = nullptr);
    ~BwCalculator();

private slots:
    void updatedQ(double);
    void updatedBW(double);

private:
    Ui::calc *ui;
    bool lock_q = false;
    bool lock_bw = false;
};

#endif // CALC_H
