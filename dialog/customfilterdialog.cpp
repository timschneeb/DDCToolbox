#include "customfilterdialog.h"
#include "dialog/textpopup.h"
#include "ui_customfilterdialog.h"

customfilterdialog::customfilterdialog(QWidget *parent) :
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
        TextPopup *t = new TextPopup(data);
        t->show();    });
}

customfilterdialog::~customfilterdialog()
{
    delete ui;
}

void customfilterdialog::setCoefficients(customFilter_t c441, customFilter_t c48){
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

customFilter_t customfilterdialog::getCoefficients(bool use48000){
    customFilter_t c441;
    c441.a0 = ui->a0->value();
    c441.a1 = ui->a1->value();
    c441.a2 = ui->a2->value();
    c441.b0 = ui->b0->value();
    c441.b1 = ui->b1->value();
    c441.b2 = ui->b2->value();
    customFilter_t c48;
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
