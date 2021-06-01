#ifndef SOFTWAREUPDATEMANAGER_H
#define SOFTWAREUPDATEMANAGER_H

#include <QObject>

class QSimpleUpdater;

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
    QSimpleUpdater *updater = nullptr;

};

#endif // SOFTWAREUPDATEMANAGER_H
