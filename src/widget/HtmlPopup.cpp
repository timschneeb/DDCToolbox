#include "HtmlPopup.h"
#include "ui_TextPopup.h"

#include <QDebug>
#include <QRegularExpression>

#include "platform/OSXHtmlSizingPatch.h"

HtmlPopup::HtmlPopup(const QString& txtbrw,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextPopup)
{
    ui->setupUi(this);

#ifdef __APPLE__
    auto html = OSXHtmlSizingPatch::patchTextSize(txtbrw);
#else
    auto html = txtbrw;
#endif

    ui->textBrowser->setText(html);
}

HtmlPopup::~HtmlPopup()
{
    delete ui;
}
