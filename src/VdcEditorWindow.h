#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "utils/VdcProjectManager.h"

namespace Ui {
class VdcEditorWindowHost;
}

class FilterModel;
class BwCalculator;
class QUndoStack;
class QUndoView;
class SoftwareUpdateManager;

class VdcEditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VdcEditorWindow(QWidget *parent = nullptr);
    ~VdcEditorWindow();

public:
    VdcProjectManager* projectManager() const
    {
        return &VdcProjectManager::instance();
    }

    void setOrientation(Qt::Orientation orientation);

public slots:
    void announceGracefulShutdown();

private slots:
    void saveProject();
    void loadProject();
    void closeProject();
    void exportProject();
    void curveFitting();

    void addPoint();
    void removePoint();
    void clearAll();
    void invertSelection();
    void shiftSelection();
    void checkStability();

    void importParametricAutoEQ();
    void importClassicVdc();
    void downloadFromAutoEQ();
    void batchConvert();

    void showCalc();
    void showHelp();
    void showUndoView();

    void savePlotScreenshot();
    void showPointsAlways(bool state);
    void updatePlots(bool onlyUpdatePoints);
    void drawPlots(){ updatePlots(false); }; /* <- required for signal system */
    void setDebugMode(bool state);

protected:
    void closeEvent(QCloseEvent* ev) override;

private:
    Ui::VdcEditorWindowHost *ui;

#ifdef Q_OS_WIN
    SoftwareUpdateManager *swUpdater;
#endif

    FilterModel    *filterModel;
    QUndoStack     *undoStack;
    QUndoView      *undoView;
    BwCalculator   *bwCalc;
    bool markerPointsVisible = false;
    bool suppressCloseConfirmation = false;

    void preparePlots();
};

#endif // MAINWINDOW_H
