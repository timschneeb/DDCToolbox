#ifndef ADDPOINT_H
#define ADDPOINT_H

#include <QDialog>

namespace Ui {
class addpoint;
}

class addpoint : public QDialog
{
    Q_OBJECT

public:
    explicit addpoint(QWidget *parent = nullptr);
    ~addpoint();
    std::list<double> returndata();

private:
    Ui::addpoint *ui;
};

#endif // ADDPOINT_H
