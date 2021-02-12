#include "VdcEditorWindow.h"

#include "widget/ProxyStyle.h"

#include <clocale>
#include <QApplication>
#include <QtPlugin>

int main(int argc, char *argv[])
{
#ifdef WIN_STATIC
    Q_IMPORT_PLUGIN(QSvgPlugin);
#endif
    Q_INIT_RESOURCE(ddceditor_resources);
    QApplication a(argc, argv);

    setlocale(LC_ALL, "C");
    QLocale::setDefault(QLocale::c());

    VdcEditorWindow w;
    QApplication::setStyle(new ProxyStyle("fusion"));
    QApplication::setPalette(qApp->style()->standardPalette());
    w.show();

    return QApplication::exec();
}
