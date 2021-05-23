#include "SamplerateChooseDialog.h"
#include "ui_SamplerateChooseDialog.h"

SamplerateChooseDialog::SamplerateChooseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SamplerateChooseDialog)
{
    ui->setupUi(this);
}

SamplerateChooseDialog::~SamplerateChooseDialog()
{
    delete ui;
}

int SamplerateChooseDialog::getResult(){
    int i = ui->comboBox->currentIndex();
    switch(i){
        case 0:
            return 44100;
        default:
            return 48000;
    }
}
