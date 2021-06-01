#include "SoftwareUpdateManager.h"

#include "VdcEditorWindow.h"

#include <Updater.h>
#include <QTimer>
#include <QMessageBox>
#include <QDesktopServices>
#include <QSimpleUpdater.h>

static const QString DEFS_URL = "https://timschneeberger.me/updates/ddctoolbox/software.json";

#define _STR(x) #x
#define STRINGIFY(x)  _STR(x)

SoftwareUpdateManager::SoftwareUpdateManager(QObject* parent) : QObject(parent)
{
    updater = QSimpleUpdater::getInstance();

    updater->setModuleVersion(DEFS_URL, STRINGIFY(CURRENT_APP_VERSION));
    updater->setDownloaderEnabled(DEFS_URL, true);

    connect(updater, &QSimpleUpdater::startedInstall, this, &SoftwareUpdateManager::requestGracefulShutdown);
}

void SoftwareUpdateManager::checkForUpdates()
{
    disconnect(updater, &QSimpleUpdater::checkingFinished, this, &SoftwareUpdateManager::silentCheckFinished);
    updater->setNotifyOnFinish(DEFS_URL, true);
    updater->setNotifyOnUpdate(DEFS_URL, true);
    updater->checkForUpdates(DEFS_URL);
}

void SoftwareUpdateManager::silentCheck()
{
    connect(updater, &QSimpleUpdater::checkingFinished, this, &SoftwareUpdateManager::silentCheckFinished);
    updater->setNotifyOnFinish(DEFS_URL, false);
    updater->setNotifyOnUpdate(DEFS_URL, false);
    updater->checkForUpdates(DEFS_URL);
}

void SoftwareUpdateManager::silentCheckDeferred(uint ms_delay)
{
    QTimer::singleShot(ms_delay, this, &SoftwareUpdateManager::silentCheck);
}

void SoftwareUpdateManager::userRequestedChangelog()
{
    if(!updater->getUpdateAvailable(DEFS_URL)){
        QMessageBox::warning(QApplication::activeWindow(), "Software updates", "Update details partially missing. Please select 'Help > Check for updates...'");
        return;
    }

    QDesktopServices::openUrl(updater->getChangelog(DEFS_URL));
}

void SoftwareUpdateManager::userRequestedInstall()
{
    if(!updater->getUpdateAvailable(DEFS_URL)){
        QMessageBox::warning(QApplication::activeWindow(), "Software updates", "Update details partially missing. Please select 'Help > Check for updates...'");
        return;
    }

    auto dl = updater->getDownloader(DEFS_URL);
    auto upd = updater->getUpdater(DEFS_URL);
    dl->setUrlId(upd->url());
    dl->setFileName(upd->downloadUrl().split("/").last());
    dl->setMandatoryUpdate(false);
    dl->startDownload(QUrl(upd->downloadUrl()));
}

void SoftwareUpdateManager::silentCheckFinished(const QString &url)
{
    if(url == DEFS_URL && updater->getUpdateAvailable(url))
    {
        emit updateAvailable();
    }
}
