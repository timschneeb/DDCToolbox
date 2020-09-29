#include "CustomFilterDialog.h"
#include "ui_CustomFilterDialog.h"

#include "widget/HtmlPopup.h"

#include <QFile>

CustomFilterDialog::CustomFilterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::customfilterdialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox,&QDialogButtonBox::helpRequested,this,[](){
        QString data = tr("Unable to open HTML file");
        QFile file(":/html/customfilter.html");
        if(file.open(QIODevice::ReadOnly))
            data = file.readAll();
        file.close();
        HtmlPopup *t = new HtmlPopup(data);
        t->show();    });
}

CustomFilterDialog::~CustomFilterDialog()
{
    delete ui;
}

void CustomFilterDialog::setCoefficients(CustomFilter c441, CustomFilter c48){
    ui->a0->setValue(c441.a0);
    ui->a1->setValue(c441.a1);
    ui->a2->setValue(c441.a2);
    ui->b0->setValue(c441.b0);
    ui->b1->setValue(c441.b1);
    ui->b2->setValue(c441.b2);
    ui->a0_48->setValue(c48.a0);
    ui->a1_48->setValue(c48.a1);
    ui->a2_48->setValue(c48.a2);
    ui->b0_48->setValue(c48.b0);
    ui->b1_48->setValue(c48.b1);
    ui->b2_48->setValue(c48.b2);
}

CustomFilter CustomFilterDialog::getCoefficients(bool use48000){
    CustomFilter c441;
    c441.a0 = ui->a0->value();
    c441.a1 = ui->a1->value();
    c441.a2 = ui->a2->value();
    c441.b0 = ui->b0->value();
    c441.b1 = ui->b1->value();
    c441.b2 = ui->b2->value();
    CustomFilter c48;
    c48.a0 = ui->a0_48->value();
    c48.a1 = ui->a1_48->value();
    c48.a2 = ui->a2_48->value();
    c48.b0 = ui->b0_48->value();
    c48.b1 = ui->b1_48->value();
    c48.b2 = ui->b2_48->value();
    if(use48000)
        return c48;
    return c441;
}
