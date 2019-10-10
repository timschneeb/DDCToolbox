#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include "qcustomplot.h"
#include <mutex>
#include <regex>
#include "ddccontext.h"
namespace Ui {
class MainWindow;
}
typedef struct calibrationPoint_s{
    int freq;
    float bw;
    float gain;
}calibrationPoint_t;
namespace Global {
   static int old_freq = 0;
   static double old_bw = 0;
   static double old_gain = 0;
}
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void drawGraph();
    void setActiveFile(QString,bool=false);
    QString currentFile = "";
    ~MainWindow();

private slots:
    void saveAsDDCProject(bool=true,QString="");
    void loadDDCProject();
    void exportVDC();
    void clearPoint(bool = true);
    void addPoint();
    void removePoint();
    void editCell(QTableWidgetItem* item);
    void showIntroduction();
    void showKeycombos();
    void importParametricAutoEQ();
    void showCalc();
    void showPointToolTip(QMouseEvent *event);
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

private:
    Ui::MainWindow *ui;
    std::mutex mtx;
    DDCContext *g_dcDDCContext;
    bool lock_actions = false;
    QUndoStack *undoStack;
    QUndoView *undoView;

    void insertData(int freq,double band,double gain);
    std::vector<calibrationPoint_t> parseParametricEQ(QString);
    bool writeProjectFile(std::vector<calibrationPoint_t> points,QString fileName);
};
class SaveItemDelegate : public QStyledItemDelegate {
    public:
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const Q_DECL_OVERRIDE
   {
       auto w = QStyledItemDelegate::createEditor(
           parent, option, index);

       auto sp = qobject_cast<QDoubleSpinBox*>(w);
       if (sp)
       {
           sp->setDecimals(6);
       }
       if(index.model()->columnCount()==3){
              Global::old_freq = index.sibling(index.row(),0).data(Qt::DisplayRole).toInt();
              Global::old_bw = index.sibling(index.row(),1).data(Qt::DisplayRole).toDouble();
              Global::old_gain = index.sibling(index.row(),2).data(Qt::DisplayRole).toDouble();
        }
       return w;
   }
};


#endif // MAINWINDOW_H
