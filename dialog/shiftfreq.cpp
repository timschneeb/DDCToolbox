#include "shiftfreq.h"
#include "ui_shiftfreq.h"

#include <utility>

shiftfreq::shiftfreq(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::shiftfreq)
{
    ui->setupUi(this);
    ui->warning->setVisible(false);
}

shiftfreq::~shiftfreq()
{
    delete ui;
}
void shiftfreq::setRange(std::vector<calibrationPoint_t> _cal_table){
    cal_table = std::move(_cal_table);
}
int shiftfreq::getResult(){
    return ui->shift->value();
}

void shiftfreq::validate(){
    int fail = 0;
    for (auto cal : cal_table)
    {
        int result = cal.freq + ui->shift->value();
        if(result<1||result>24000)fail++;
    }
    ui->warning->setVisible(fail>0);
}
void shiftfreq::confirm(){
    for (auto cal : cal_table)
    {
        int result = cal.freq + ui->shift->value();
        if(result<1||result>24000){
           ui->warning->setVisible(true);
           QMessageBox::warning(this,tr("Shift frequencies"),tr("Invalid number"));
           return;
        }
        else accept();
    }
}
