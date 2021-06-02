#include "AppRuntime.h"

#include <QtPlugin>
#include <clocale>

int main(int argc, char *argv[])
{
#ifdef WIN_STATIC
    Q_IMPORT_PLUGIN(QSvgPlugin);
#endif
    Q_INIT_RESOURCE(ddceditor_resources);

    srand(time(NULL));
    setlocale(LC_ALL, "C");
    QLocale::setDefault(QLocale::c());

    qRegisterMetaType<QVector<float>>("QVector<float>");
    qRegisterMetaType<std::vector<double>>("std::vector<double>");

    return AppRuntime(argc, argv).exec();
}
