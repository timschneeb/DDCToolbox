#include "undocmd.h"
#include "ddccontext.h"

AddCommand::AddCommand(QTableWidget* _tw,DDCContext* _ddcContext,
                       calibrationPoint_t _cal,std::mutex* _mtx,bool* _lockActions,MainWindow* _host, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    static int itemCount = 0;
    cal = _cal;
    tw = _tw;
    mtx = _mtx;
    host = _host;
    ddcContext = _ddcContext;
    lockActions = _lockActions;
    ++itemCount;
    setText(createCommandString());
}
void AddCommand::undo()
{
    mtx->lock();
    tw->setSortingEnabled(false);

    for (int i = 0; i < tw->rowCount(); i++)
    {
        if (tw->item(i,0)->data(Qt::DisplayRole).toInt() == cal.freq)
        {
            *lockActions = true;
            tw->removeRow(i);
            ddcContext->RemoveFilter(cal.freq);
            *lockActions = false;
            break;
        }
    }

    tw->setSortingEnabled(true);
    tw->sortItems(0);
    host->drawGraph();
    mtx->unlock();
}
void AddCommand::redo()
{
    mtx->lock();
    *lockActions = true;

    tw->setSortingEnabled(false);
    QTableWidgetItem *c1 = new QTableWidgetItem();
    QTableWidgetItem *c2 = new QTableWidgetItem();
    QTableWidgetItem *c3 = new QTableWidgetItem();
    c1->setData(Qt::DisplayRole, cal.freq);
    c2->setData(Qt::DisplayRole, (double)cal.bw);
    c3->setData(Qt::DisplayRole, (double)cal.gain);
    tw->insertRow(tw->rowCount());
    tw->setItem(tw->rowCount()-1, 0, c1);
    tw->setItem(tw->rowCount()-1, 1, c2);
    tw->setItem(tw->rowCount()-1, 2, c3);
    ddcContext->AddFilter(cal.freq, cal.gain, cal.bw, 44100.0);
    tw->setSortingEnabled(true);
    tw->sortItems(0);
    tw->update();

    host->drawGraph();
    *lockActions = false;
    mtx->unlock();
}
QString AddCommand::createCommandString(){
    return QObject::tr("\"Add %1Hz\"").arg(cal.freq);
}


EditCommand::EditCommand(QTableWidget* _tw,DDCContext* _ddcContext,int _row,
                         calibrationPoint_t _cal,calibrationPoint_s _oldcal,std::mutex* _mtx,bool* _lockActions,MainWindow* _host, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    static int itemCount = 0;
    cal = _cal;
    oldcal = _oldcal;
    tw = _tw;
    mtx = _mtx;
    row = _row;
    host = _host;
    ddcContext = _ddcContext;
    lockActions = _lockActions;
    ++itemCount;
    setText(createCommandString());
}
void EditCommand::undo()
{
    mtx->lock();
    *lockActions = true;

    if(row < tw->rowCount()){
        tw->item(row,0)->setData(Qt::DisplayRole,oldcal.freq);
        tw->item(row,1)->setData(Qt::DisplayRole,(double)oldcal.bw);
        tw->item(row,2)->setData(Qt::DisplayRole,(double)oldcal.gain);
    }

    ddcContext->ModifyFilter(cal.freq, oldcal.freq, oldcal.gain, oldcal.bw, 44100.0);
    tw->setSortingEnabled(true);
    tw->sortItems(0);

    host->drawGraph();
    *lockActions = false;
    mtx->unlock();
}
void EditCommand::redo()
{
    mtx->lock();
    *lockActions = true;

    if(row < tw->rowCount()){
        tw->item(row,0)->setData(Qt::DisplayRole,cal.freq);
        tw->item(row,1)->setData(Qt::DisplayRole,(double)cal.bw);
        tw->item(row,2)->setData(Qt::DisplayRole,(double)cal.gain);
    }

    ddcContext->ModifyFilter(oldcal.freq, cal.freq, cal.gain, cal.bw, 44100.0);
    tw->setSortingEnabled(true);
    tw->sortItems(0);

    host->drawGraph();
    *lockActions = false;
    mtx->unlock();
}
QString EditCommand::createCommandString(){
    return QObject::tr("\"Edit row %1\"").arg(row);
}


ClearCommand::ClearCommand(QTableWidget* _tw,DDCContext* _ddcContext,
                           std::vector<calibrationPoint_t> _cal_table,std::mutex* _mtx,bool* _lockActions,MainWindow* _host, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    static int itemCount = 0;
    cal_table = _cal_table;
    tw = _tw;
    mtx = _mtx;
    host = _host;
    ddcContext = _ddcContext;
    lockActions = _lockActions;
    ++itemCount;
    setText(createCommandString());
}
void ClearCommand::undo()
{
    mtx->lock();
    *lockActions = true;
    tw->setSortingEnabled(false);

    for(size_t i=0;i < cal_table.size();i++)
    {
        calibrationPoint_t cal = cal_table.at(i);
        QTableWidgetItem *c1 = new QTableWidgetItem();
        QTableWidgetItem *c2 = new QTableWidgetItem();
        QTableWidgetItem *c3 = new QTableWidgetItem();
        c1->setData(Qt::DisplayRole, cal.freq);
        c2->setData(Qt::DisplayRole, (double)cal.bw);
        c3->setData(Qt::DisplayRole, (double)cal.gain);
        tw->insertRow(tw->rowCount());
        tw->setItem(tw->rowCount()-1, 0, c1);
        tw->setItem(tw->rowCount()-1, 1, c2);
        tw->setItem(tw->rowCount()-1, 2, c3);
        ddcContext->AddFilter(cal.freq, cal.gain, cal.bw, 44100.0);
    }

    tw->setSortingEnabled(true);
    tw->sortItems(0);
    tw->update();

    host->drawGraph();
    *lockActions = false;
    mtx->unlock();
}
void ClearCommand::redo()
{
    mtx->lock();
    *lockActions = true;

    ddcContext->ClearFilters();
    tw->clear();
    tw->setRowCount(0);
    tw->reset();
    tw->setHorizontalHeaderLabels(QStringList() << "Frequency" << "Bandwidth" << "Gain");

    host->drawGraph();
    *lockActions = false;
    mtx->unlock();
}
QString ClearCommand::createCommandString(){
    return QObject::tr("\"Clear all points\"");
}

RemoveCommand::RemoveCommand(QTableWidget* _tw,DDCContext* _ddcContext,std::vector<int> _rows,QModelIndexList _model_list,
                             std::vector<calibrationPoint_t> _cal_table,std::mutex* _mtx,bool* _lockActions,MainWindow* _host, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    static int itemCount = 0;
    cal_table = _cal_table;
    tw = _tw;
    mtx = _mtx;
    host = _host;
    rows = _rows;
    model_list = _model_list;
    ddcContext = _ddcContext;
    lockActions = _lockActions;
    ++itemCount;
    setText(createCommandString());
}
void RemoveCommand::undo()
{
    mtx->lock();
    *lockActions = true;
    tw->setSortingEnabled(false);

    for(size_t i=0;i < cal_table.size();i++)
    {
        calibrationPoint_t cal = cal_table.at(i);
        QTableWidgetItem *c1 = new QTableWidgetItem();
        QTableWidgetItem *c2 = new QTableWidgetItem();
        QTableWidgetItem *c3 = new QTableWidgetItem();
        c1->setData(Qt::DisplayRole, cal.freq);
        c2->setData(Qt::DisplayRole, (double)cal.bw);
        c3->setData(Qt::DisplayRole, (double)cal.gain);
        tw->insertRow(tw->rowCount());
        tw->setItem(tw->rowCount()-1, 0, c1);
        tw->setItem(tw->rowCount()-1, 1, c2);
        tw->setItem(tw->rowCount()-1, 2, c3);
        ddcContext->AddFilter(cal.freq, cal.gain, cal.bw, 44100.0);
    }

    tw->setSortingEnabled(true);
    tw->sortItems(0);
    tw->update();
    *lockActions = false;
    mtx->unlock();
}
void RemoveCommand::redo()
{
    mtx->lock();
    *lockActions = true;
    tw->setSortingEnabled(false);

     /*for (size_t i = 0; i < rows.size(); i++)
        ddcContext->RemoveFilter(tw->item(rows.at(i),0)->data(Qt::DisplayRole).toInt());

   for (int i = model_list.count()-1; i >= 0; i--)
    {
        qDebug() << model_list.at(i).row();
        tw->model()->removeRow(model_list.at(i).row());
    }*/

    QList<int> removeRows;
    for (size_t i = 0; i < rows.size(); i++)
    {
        removeRows.append(rows.at(i));
        int freq = tw->item(rows.at(i),0)->data(Qt::DisplayRole).toInt();
        ddcContext->RemoveFilter(freq);
    }
    for(int i=0;i<removeRows.count();++i)
    {
        for(int j=i;j<removeRows.count();++j)
            if(removeRows.at(j) > removeRows.at(i))
                removeRows[j]--;
        tw->model()->removeRows(removeRows.at(i), 1);
    }

    tw->setSortingEnabled(true);
    tw->sortItems(0);
    host->drawGraph();

    *lockActions = false;
    mtx->unlock();
}
QString RemoveCommand::createCommandString(){
    return QObject::tr("\"Remove row(s)\"");
}
