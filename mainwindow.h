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
#include <QSimpleUpdater.h>
#include "ddccontext.h"
#include "filtertypes.h"
namespace Ui {
class MainWindow;
}

typedef struct calibrationPoint_s{
    biquad::Type type;
    int freq;
    double bw;
    double gain;
}calibrationPoint_t;

namespace Global {
static biquad::Type old_type = biquad::Type::PEAKING;
static int old_freq = 0;
static double old_bw = 0;
static double old_gain = 0;
}

enum datatype{
    type,
    freq,
    bw,
    gain
};

static const QString DEFS_URL = "http://nightly.thebone.cf/updater/ddctoolbox.json";
static const QString VERSION = "1.2.8";

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum graphtype{
        magnitude,
        groupdelay,
        all
    };
    explicit MainWindow(QWidget *parent = nullptr);
    void drawGraph(graphtype = graphtype::all);
    void setActiveFile(QString,bool=false);
    QString currentFile = "";
    ~MainWindow();

public slots:
    void checkForUpdates();

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
    void showHelp();
    void drawGroupDelayMenu(const QPoint &);
    void drawMagnitudeMenu(const QPoint &);
    void setupMenus();

private:
    Ui::MainWindow *ui;
    std::mutex mtx;
    DDCContext *g_dcDDCContext;
    bool lock_actions = false;
    QUndoStack *undoStack;
    QUndoView *undoView;
    QSimpleUpdater* m_updater;

    biquad::Type getType(int row);
    double getValue(datatype dat,int row);
    void insertData(biquad::Type type,int freq,double band,double gain);
    std::vector<calibrationPoint_t> parseParametricEQ(QString);
    bool writeProjectFile(std::vector<calibrationPoint_t> points,QString fileName,bool compatibilitymode);
};
class SaveItemDelegate : public QStyledItemDelegate {
public:
    biquad::Type getType(const QString &_type) const{
        if(_type=="Peaking")return biquad::Type::PEAKING;
        else if(_type=="Low Pass")return biquad::Type::LOW_PASS;
        else if(_type=="High Pass")return biquad::Type::HIGH_PASS;
        else if(_type=="Band Pass")return biquad::Type::BAND_PASS;
        else if(_type=="All Pass")return biquad::Type::ALL_PASS;
        else if(_type=="Notch")return biquad::Type::NOTCH;
        else if(_type=="Low Shelf")return biquad::Type::LOW_SHELF;
        else if(_type=="High Shelf")return biquad::Type::HIGH_SHELF;
        else if(_type=="Unity Gain")return biquad::Type::UNITY_GAIN;
        return biquad::Type::PEAKING;
    }
    biquad::Type getType(const QModelIndex &index) const{
        QString _type = index.sibling(index.row(),0).data(Qt::DisplayRole).toString();
        return getType(_type);
    }

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
        if(index.model()->columnCount()>3){

            Global::old_freq = index.sibling(index.row(),1).data(Qt::DisplayRole).toInt();
            Global::old_bw = index.sibling(index.row(),2).data(Qt::DisplayRole).toDouble();
            Global::old_gain = index.sibling(index.row(),3).data(Qt::DisplayRole).toDouble();
        }
        const QString currentType = index.sibling(index.row(),0).data(Qt::DisplayRole).toString();

        if(index.column()==0){
            QComboBox *cb = new QComboBox(parent);
            const int row = index.row();
            cb->addItem(QString("Peaking"));
            cb->addItem(QString("Low Pass"));
            cb->addItem(QString("High Pass"));
            cb->addItem(QString("Band Pass"));
            cb->addItem(QString("Notch"));
            cb->addItem(QString("All Pass"));
            cb->addItem(QString("Low Shelf"));
            cb->addItem(QString("High Shelf"));
            cb->addItem(QString("Unity gain"));
            return cb;
        }
        else if (index.column()==2&&sp) {
            //auto p = qobject_cast<QTableWidget*>(sp->parent()->parent());
            switch (getType(currentType)) {
            case biquad::LOW_SHELF:
            case biquad::HIGH_SHELF:
                sp->setPrefix("S: ");
                break;
            default:
                sp->setPrefix("BW: ");
            }
        }
        else if (index.column()==3&&sp) {
            //auto p = qobject_cast<QTableWidget*>(sp->parent()->parent());
            switch (getType(currentType)) {
            case biquad::PEAKING:
            case biquad::LOW_SHELF:
            case biquad::UNITY_GAIN:
            case biquad::HIGH_SHELF:
                sp->setEnabled(true);
                //if (p)p->item(index.row(),3)->setFlags(p->item(index.row(),3)->flags() | Qt::ItemIsEditable | Qt::ItemIsEnabled);

                break;
            default:
                sp->setEnabled(false);
                //if (p)p->item(index.row(),3)->setFlags(p->item(index.row(),3)->flags() & (~Qt::ItemIsEditable) & (~Qt::ItemIsEnabled));
            }
        }
        return w;
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        const QString currentType = index.sibling(index.row(),0).data(Qt::DisplayRole).toString();
        if (index.column()==3) {
            switch (getType(currentType)) {
            case biquad::PEAKING:
            case biquad::LOW_SHELF:
            case biquad::UNITY_GAIN:
            case biquad::HIGH_SHELF:
                QStyledItemDelegate::paint(painter,option,index);
                return;
            default:
                //Leave item empty
                return;
            }
        }
        else
            QStyledItemDelegate::paint(painter,option,index);
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        // get the index of the text in the combobox that matches the current value of the item
        const QString currentText = index.data(Qt::EditRole).toString();

        if(index.column()==0){
            QComboBox *cb = qobject_cast<QComboBox *>(editor);
            Q_ASSERT(cb);
            const int cbIndex = cb->findText(currentText);
            // if it is valid, adjust the combobox
            if (cbIndex >= 0)
                cb->setCurrentIndex(cbIndex);

        }

        else
            QStyledItemDelegate::setEditorData(editor,index);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        if(index.column()==0){
            QComboBox *cb = qobject_cast<QComboBox *>(editor);
            Q_ASSERT(cb);
            model->setData(index, cb->currentText(), Qt::EditRole);
            Global::old_type = getType(index);
        }
        else
            QStyledItemDelegate::setModelData(editor, model, index);
    }


};


#endif // MAINWINDOW_H
