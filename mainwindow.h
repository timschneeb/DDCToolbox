#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>
#include <QTableWidgetItem>
#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <mutex>
#include <regex>
#include "io/conversionengine.h"
#include "ddccontext.h"
#include "utils/filtertypes.h"
#include "plot/qcustomplot.h"
#include "item/customfilteritem.h"
#include "utils/tableproxy.h"

namespace Ui {
class MainWindow;
}

namespace Global {
static biquad::Type old_type = biquad::Type::PEAKING;
static int old_freq = 0;
static double old_bw = 0;
static double old_gain = 0;
static customFilter_t old_custom441;
static customFilter_t old_custom48;
}

enum DataType{
    type = 0,
    freq = 1,
    bw = 2,
    gain = 3
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum GraphType{
        magnitude,
        groupdelay,
        phase,
        all
    };
    explicit MainWindow(QWidget *parent = nullptr);
    void setActiveFile(const QString&,bool=false);
    QString currentFile = "";
    ~MainWindow();

public slots:
    void drawGraph(MainWindow::GraphType = GraphType::all, bool onlyUpdatePoints = false);

private slots:
    void saveDDCProject();
    void saveAsDDCProject(bool=true,QString="",bool compatibilitymode=false);
    void loadDDCProject();
    void closeProject();

    void importParametricAutoEQ();
    void importClassicVDC();
    void exportCompatVDCProj();
    void exportVDC();

    void batch_vdc2vdcprj();
    void batch_parametric2vdcprj();

    void addPoint();
    void removePoint();
    void clearPoint(bool = true);
    void invertSelection();
    void shiftSelection();

    void editCell(QTableWidgetItem* item);

    void showIntroduction();
    void showKeycombos();
    void showCalc();
    void showHelp();
    void showUndoView();

    void ScreenshotGraph();
    void hidePoints(bool state);
    void CheckStability();

    void downloadFromAutoEQ();

signals:
    void loadFinished();

private:
    Ui::MainWindow *ui;

    std::mutex mtx;
    DDCContext *g_dcDDCContext;
    bool lock_actions = false;

    QUndoStack *undoStack;
    QUndoView *undoView;

    bool markerPointsVisible = false;

    biquad::Type getType(int row);
    uint32_t getId(int row);
    double getValue(DataType dat,int row);
    uint32_t insertData(biquad::Type type,int freq,double band,double gain,bool toggleSorting = true);

};

#endif // MAINWINDOW_H
