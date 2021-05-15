#include "SoftwareUpdateManager.h"

static const QString DEFS_URL = "https://timschneeberger.me/updates/ddctoolbox/software.json";

#define _STR(x) #x
#define STRINGIFY(x)  _STR(x)

SoftwareUpdateManager::SoftwareUpdateManager(QObject* parent) : QObject(parent)
{
    updater = QSimpleUpdater::getInstance();

    updater->setModuleVersion(DEFS_URL, STRINGIFY(CURRENT_APP_VERSION));
    updater->setDownloaderEnabled(DEFS_URL, true);
}

void SoftwareUpdateManager::checkForUpdates()
{
    updater->setNotifyOnFinish(DEFS_URL, true);
    updater->setNotifyOnUpdate(DEFS_URL, true);
    updater->checkForUpdates(DEFS_URL);
}

void SoftwareUpdateManager::checkForUpdatesSilently()
{
    updater->setNotifyOnFinish(DEFS_URL, false);
    updater->setNotifyOnUpdate(DEFS_URL, false);
    updater->checkForUpdates(DEFS_URL);
}
