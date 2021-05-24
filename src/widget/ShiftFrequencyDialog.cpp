#include "ShiftFrequencyDialog.h"
#include "ui_ShiftFrequencyDialog.h"

#include <model/FilterModel.h>
#include <utility>

ShiftFrequencyDialog::ShiftFrequencyDialog(FilterModel* model, QWidget *parent) :
    QDialog(parent), ui(new Ui::shiftfreq), model(model)
{
    ui->setupUi(this);
}

ShiftFrequencyDialog::~ShiftFrequencyDialog()
{
    delete ui;
}

int ShiftFrequencyDialog::getResult(){
    return ui->shift->value();
}

void ShiftFrequencyDialog::confirm(){
    for (const auto& biquad : model->getFilterBank())
    {
        int result = biquad->GetFrequency() + ui->shift->value();

        if(result < 1 || result > 24000){
            QMessageBox::warning(this,tr("Shift frequencies"),
                                 tr("Some frequency values are shifted out of range. "
                                    "Please choose a smaller factor."));
            return;
         }
        else accept();
    }
}
