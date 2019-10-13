#ifndef ADDPOINT_H
#define ADDPOINT_H

#include <QDialog>

namespace Ui {
class addpoint;
}
typedef struct addp_response_s{
    std::vector<double> values;
    QString filtertype;
}addp_response_t;
class addpoint : public QDialog
{
    Q_OBJECT

public:
    explicit addpoint(QWidget *parent = nullptr);
    ~addpoint();
    addp_response_t returndata();

private:
    Ui::addpoint *ui;
};

#endif // ADDPOINT_H
