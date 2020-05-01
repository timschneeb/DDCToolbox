#include "mainwindow.h"
#include <QApplication>

using namespace std;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QLocale::setDefault(QLocale::c());
    QApplication::setStyle("fusion");
    w.show();

    return QApplication::exec();
}
