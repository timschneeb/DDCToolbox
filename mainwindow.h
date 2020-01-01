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
#include <QSimpleUpdater.h>
#include "ddccontext.h"
#include "filtertypes.h"
#include "plot/qcustomplot.h"
#include "item/customfilteritem.h"
#include "tableproxy.h"
namespace Ui {
class MainWindow;
}

namespace Global {
static biquad::Type old_type = biquad::Type::PEAKING;
static int old_freq = 0;
static double old_bw = 0;
static double old_gain = 0;
static customFilter_t old_custom;
}

enum datatype{
    type,
    freq,
    bw,
    gain
};

static const QString DEFS_URL = "http://nightly.thebone.cf/updater/ddctoolbox.json";
static const QString VERSION = "1.3.0";

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum graphtype{
        magnitude,
        groupdelay,
        phase,
        all
    };
    explicit MainWindow(QWidget *parent = nullptr);
    void setActiveFile(QString,bool=false);
    QString currentFile = "";
    ~MainWindow();

public slots:
    void checkForUpdates();
    void drawGraph(graphtype = graphtype::all, bool onlyUpdatePoints = false);

private slots:
    void saveAsDDCProject(bool=true,QString="",bool compatibilitymode=false);
    void loadDDCProject();
    void exportCompatVDCProj();
    void exportVDC();
    void clearPoint(bool = true);
    void addPoint();
    void removePoint();
    void editCell(QTableWidgetItem* item);
    void showIntroduction();
    void showKeycombos();
    void importParametricAutoEQ();
    void showCalc();
    void importVDC();
    void readLine_DDCProject(QString);
    void batch_vdc2vdcprj();
    void batch_parametric2vdcprj();
    void toggleGraph(bool);
    void ScreenshotGraph();
    void closeProject();
    void saveDDCProject();
    void showUndoView();
    void invertSelection();
    void shiftSelection();
    void showHelp();
    void hidePoints(bool state);
    void switchTranslator(QTranslator& translator, const QString& filename);
    void CheckStability();

protected:
    void changeEvent(QEvent*);

protected slots:
    void slotLanguageChanged(QAction* action);

signals:
    void loadFinished();

private:
    Ui::MainWindow *ui;
    QTranslator m_translator;
    QString m_currLang;
    std::mutex mtx;
    DDCContext *g_dcDDCContext;
    bool lock_actions = false;
    QUndoStack *undoStack;
    QUndoView *undoView;
    QSimpleUpdater* m_updater;
    bool markerPointsVisible = false;
    QCPDataSelection* magnitudeSelection;

    biquad::Type getType(int row);
    uint32_t getId(int row);
    double getValue(datatype dat,int row);
    uint32_t insertData(biquad::Type type,int freq,double band,double gain);
    std::vector<calibrationPoint_t> parseParametricEQ(QString);
    bool writeProjectFile(std::vector<calibrationPoint_t> points,QString fileName,bool compatibilitymode);
    void createLanguageMenu();
    void loadLanguage(const QString& rLanguage);
};

#endif // MAINWINDOW_H
