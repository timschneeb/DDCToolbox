#include "AppRuntime.h"
#include "widget/ProxyStyle.h"

#include <QFileOpenEvent>
#include <QDebug>

AppRuntime::AppRuntime(int& argc, char**argv): QApplication(argc, argv)
{
    this->setStyle(new ProxyStyle("Fusion"));
    this->setPalette(this->style()->standardPalette());
    this->setAttribute(Qt::AA_DisableWindowContextHelpButton);

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
