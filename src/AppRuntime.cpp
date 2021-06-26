#include "AppRuntime.h"

#include "utils/VdcProjectManager.h"
#include "widget/ProxyStyle.h"

#include <QDebug>
#include <QFileOpenEvent>
#include <QStyleFactory>


#define _STR(x) #x
#define STRINGIFY(x)  _STR(x)

AppRuntime::AppRuntime(int& argc, char**argv): QApplication(argc, argv)
{

    AppRuntime::setApplicationVersion(STRINGIFY(CURRENT_APP_VERSION));
    AppRuntime::setApplicationName("DDCToolbox");
    AppRuntime::setOrganizationName("Tim Schneeberger");

    AppRuntime::setStyle(new ProxyStyle("Fusion"));
    AppRuntime::setPalette(AppRuntime::style()->standardPalette());
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    AppRuntime::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

#ifdef __APPLE__
    this->setStyleSheet("* {font-size: 13px;}");
#endif

    _window = new VdcEditorWindow();

    if(argc > 1)
    {
        VdcProjectManager::instance().loadProject(QString::fromLocal8Bit(argv[1]));
    }

    _window->show();
}

AppRuntime::~AppRuntime()
{
    _window->deleteLater();
}

bool AppRuntime::event(QEvent *event)
{
    switch(event->type())
    {
        case QEvent::FileOpen:
        {
            QFileOpenEvent * fileOpenEvent = static_cast<QFileOpenEvent *>(event);
            if(fileOpenEvent)
            {
                auto path = fileOpenEvent->file();
                qDebug() << "File open event received:" << path;
                if(!path.isEmpty() && _window)
                {
                    VdcProjectManager::instance().loadProject(path);
                    return true;
                }
            }
        }
        break;
    }
    return QApplication::event(event);
 }
