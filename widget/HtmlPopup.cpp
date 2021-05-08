#include "HtmlPopup.h"
#include "ui_TextPopup.h"

#include <QDebug>
#include <QRegularExpression>

HtmlPopup::HtmlPopup(const QString& txtbrw,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextPopup)
{
    ui->setupUi(this);

    auto html = txtbrw;

#ifdef __APPLE__
    // OSX HTML render DPI size fix
    QList<double> sizes;
    QRegularExpression re("font-size:(\\d+\\.?\\d*)pt;");
    QRegularExpressionMatchIterator i = re.globalMatch(html);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        if (match.hasMatch()) {
             float f = match.captured(1).toFloat();
             if(!sizes.contains(f))
                sizes.append(f);
        }
    }

    for(auto size : sizes){
        html.replace(re, QString("font-size:%1pt;").arg(size + 4));
    }
#endif

    ui->textBrowser->setHtml(html);
    qDebug() << (html);
}

HtmlPopup::~HtmlPopup()
{
    delete ui;
}
