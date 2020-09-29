#include "HtmlPopup.h"
#include "ui_TextPopup.h"

HtmlPopup::HtmlPopup(const QString& txtbrw,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextPopup)
{
    ui->setupUi(this);
    ui->textBrowser->setHtml(txtbrw);
}

HtmlPopup::~HtmlPopup()
{
    delete ui;
}
