#ifndef CALC_H
#define CALC_H

#include <QDialog>

namespace Ui {
class calc;
}

class calc : public QDialog
{
    Q_OBJECT

public:
    explicit calc(QWidget *parent = nullptr);
    ~calc();

private slots:
    void updatedQ(double);
    void updatedBW(double);

private:
    Ui::calc *ui;
    bool lock_q = false;
    bool lock_bw = false;
};

#endif // CALC_H
