#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>
#include <QTableWidgetItem>
#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QUndoCommand>
#include <QUndoView>
#include <mutex>
#include <regex>

#include "utils/VdcProjectManager.h"

namespace Ui {
class VdcEditorWindowHost;
}

class FilterModel;
class BwCalculator;

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

private slots:
    void saveProject();
    void loadProject();
    void closeProject();
    void exportProject();

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

private:
    Ui::VdcEditorWindowHost *ui;

    FilterModel    *filterModel;
    QUndoStack     *undoStack;
    QUndoView      *undoView;
    BwCalculator   *bwCalc;
    bool markerPointsVisible = false;

    void preparePlots();
};

#endif // MAINWINDOW_H
