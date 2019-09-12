#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QItemDelegate>
#include <mutex>
#include <regex>
#include "ddccontext.h"
namespace Ui {
class MainWindow;
}
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
    ~MainWindow();

private slots:
    void saveDDCProject();
    void loadDDCProject();
    void exportVDC();
    void clearPoint();
    void addPoint();
    void removePoint();
    void editCell(QTableWidgetItem* item);
    void drawGraph();
    void showIntroduction();
    void showKeycombos();
    void importParametricAutoEQ();
    void showCalc();

private:
    Ui::MainWindow *ui;
    std::mutex mtx;
    DDCContext *g_dcDDCContext;
    bool lock_actions = false;

};
class SaveItemDelegate : public QItemDelegate {
    public:
        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
            if(index.model()->columnCount()==3){
                Global::old_freq = index.sibling(index.row(),0).data(Qt::DisplayRole).toInt();
                Global::old_bw = index.sibling(index.row(),1).data(Qt::DisplayRole).toDouble();
                Global::old_gain = index.sibling(index.row(),2).data(Qt::DisplayRole).toDouble();
            }
            return QItemDelegate::createEditor(parent, option, index);
        }
};


#endif // MAINWINDOW_H
