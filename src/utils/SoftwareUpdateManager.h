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
    void checkForUpdatesSilently();

private:
    QSimpleUpdater *updater = new QSimpleUpdater();

};

#endif // SOFTWAREUPDATEMANAGER_H
