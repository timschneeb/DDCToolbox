#include "ShiftFrequencyDialog.h"
#include "ui_ShiftFrequencyDialog.h"

#include "model/FilterModel.h"

#include <QMessageBox>

ShiftFrequencyDialog::ShiftFrequencyDialog(FilterModel* model, const QModelIndexList &indices, QWidget *parent) :
    QDialog(parent), ui(new Ui::shiftfreq), model(model), indices(indices)
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
    for (const auto& index : qAsConst(indices))
    {
        int result = model->getFilter(index.row())->GetFrequency() + ui->shift->value();

        if(result < 1 || result > 24000){
            QMessageBox::warning(this,tr("Shift frequencies"),
                                 tr("Some frequency values are shifted out of range. "
                                    "Please choose a smaller factor."));
            return;
        }
        else accept();
    }
}
