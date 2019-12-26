#include "textpopup.h"
#include "ui_textpopup.h"

TextPopup::TextPopup(QString txtbrw,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextPopup)
{
    ui->setupUi(this);
    ui->textBrowser->setHtml(txtbrw);
}

TextPopup::~TextPopup()
{
    delete ui;
}
