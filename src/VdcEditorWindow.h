#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSessionManager>

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

private slots:
    void commitData(QSessionManager& manager);

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
    void setOrientation(bool vertical);

    void adjustForCurrentFile(const QString &filePath);
    void updateRecentActionList();

private:
    void createFilterModel();
    void createUpdater();
    void createRecentFileActions();
    void createPlotLayout();

protected:
    void closeEvent(QCloseEvent* ev) override;

private:
    Ui::VdcEditorWindowHost *ui;

#ifdef Q_OS_WIN
    SoftwareUpdateManager *swUpdater;
#endif

    QList<QAction*> recentFileActionList;
    const int       maxFileNr = 5;

    FilterModel    *filterModel;
    QUndoStack     *undoStack;
    QUndoView      *undoView;
    BwCalculator   *bwCalc;
    bool markerPointsVisible = false;

};

#endif // MAINWINDOW_H
