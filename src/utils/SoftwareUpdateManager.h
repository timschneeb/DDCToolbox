#ifndef SOFTWAREUPDATEMANAGER_H
#define SOFTWAREUPDATEMANAGER_H

#include <QSimpleUpdater.h>

class SoftwareUpdateManager : public QObject
{
    Q_OBJECT
public:
    SoftwareUpdateManager(QObject* parent = nullptr);

public slots:
    void checkForUpdates();
    void silentCheck();
    void silentCheckDeferred(uint ms_delay);

    void userRequestedChangelog();
    void userRequestedInstall();

private slots:
    void silentCheckFinished(const QString& url);

signals:
    void updateAvailable();
    void requestGracefulShutdown();

private:
    QSimpleUpdater *updater = new QSimpleUpdater();

};

#endif // SOFTWAREUPDATEMANAGER_H
