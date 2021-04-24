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

    srand(time(NULL));
    setlocale(LC_ALL, "C");
    QLocale::setDefault(QLocale::c());

    qRegisterMetaType<QVector<float>>("QVector<float>");
    qRegisterMetaType<std::vector<double>>("std::vector<double>");

    VdcEditorWindow w;
    QApplication::setStyle(new ProxyStyle("Fusion"));
    QApplication::setPalette(qApp->style()->standardPalette());
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
    w.show();

    if(argc == 2){
        VdcProjectManager::instance().loadProject(QString::fromLocal8Bit(argv[1]));
    }

    return QApplication::exec();
}
