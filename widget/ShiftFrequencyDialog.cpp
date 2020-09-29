#include "ShiftFrequencyDialog.h"
#include "ui_ShiftFrequencyDialog.h"

#include <utility>

ShiftFrequencyDialog::ShiftFrequencyDialog(FilterModel* model, QWidget *parent) :
    QDialog(parent), ui(new Ui::shiftfreq), model(model)
{
    ui->setupUi(this);
    ui->warning->setVisible(false);
}

ShiftFrequencyDialog::~ShiftFrequencyDialog()
{
    delete ui;
}

int ShiftFrequencyDialog::getResult(){
    return ui->shift->value();
}

void ShiftFrequencyDialog::validate(){
    int fail = 0;

    for (const auto& biquad : model->getFilterBank())
    {
        int result = biquad->GetFrequency() + ui->shift->value();

        if(result < 1 || result > 24000)
            fail++;
    }

    ui->warning->setVisible(fail > 0);
}

void ShiftFrequencyDialog::confirm(){
    for (const auto& biquad : model->getFilterBank())
    {
        int result = biquad->GetFrequency() + ui->shift->value();

        if(result < 1 || result > 24000){
            ui->warning->setVisible(true);
            QMessageBox::warning(this,tr("Shift frequencies"),tr("Invalid number"));
            return;
         }
        else accept();
    }
}
