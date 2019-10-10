#include "shiftfreq.h"
#include "ui_shiftfreq.h"

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
    cal_table = _cal_table;
}
int shiftfreq::getResult(){
    return ui->shift->value();
}

void shiftfreq::validate(){
    int fail = 0;
    for (size_t i = 0; i < cal_table.size(); i++)
    {
        calibrationPoint_t cal = cal_table.at(i);
        int result = cal.freq + ui->shift->value();
        if(result<1||result>24000)fail++;
    }
    ui->warning->setVisible(fail>0);
}
void shiftfreq::confirm(){
    for (size_t i = 0; i < cal_table.size(); i++)
    {
        calibrationPoint_t cal = cal_table.at(i);
        int result = cal.freq + ui->shift->value();
        if(result<1||result>24000){
           ui->warning->setVisible(true);
           QMessageBox::warning(this,"Shift frequencies","Invalid number");
           return;
        }
        else accept();
    }
}
