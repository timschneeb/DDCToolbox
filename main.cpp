#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QLocale::setDefault(QLocale::c());
    QApplication::setStyle("fusion");
    a.setPalette(qApp->style()->standardPalette());
    w.show();

    return QApplication::exec();
}
