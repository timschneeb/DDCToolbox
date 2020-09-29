#include "VdcEditorWindow.h"

#include "widget/ProxyStyle.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    setlocale(LC_ALL, "C");
    QLocale::setDefault(QLocale::c());

    VdcEditorWindow w;
    QApplication::setStyle(new ProxyStyle("fusion"));
    QApplication::setPalette(qApp->style()->standardPalette());
    w.show();

    return QApplication::exec();
}
