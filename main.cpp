#include "VdcEditorWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    setlocale(LC_ALL, "C");
    QLocale::setDefault(QLocale::c());

    VdcEditorWindow w;
    QApplication::setStyle("fusion");
    QApplication::setPalette(qApp->style()->standardPalette());
    w.show();

    return QApplication::exec();
}
