#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    setlocale(LC_ALL, "C");
    QLocale::setDefault(QLocale::c());

    MainWindow w;    
    QApplication::setStyle("fusion");
    QApplication::setPalette(qApp->style()->standardPalette());
    w.show();

    return QApplication::exec();
}
