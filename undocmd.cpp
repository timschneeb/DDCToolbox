#include "undocmd.h"
#include "ddccontext.h"
#include "item/customfilterfactory.h"

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
        if (tw->item(i,0)->data(Qt::UserRole).toUInt() == cal.id)
        {
            *lockActions = true;
            tw->removeRow(i);
            ddcContext->RemoveFilter(cal.id);
            *lockActions = false;
            break;
        }
    }

    tw->setSortingEnabled(true);
    tw->sortItems(1);
    host->drawGraph();
    mtx->unlock();
}
void AddCommand::redo()
{
    mtx->lock();
    *lockActions = true;

    tw->setSortingEnabled(false);

    (new tableproxy(tw))->addRow(cal);

    if(cal.type==biquad::CUSTOM){
        newCustomFilter(defaultCustomFilter(),tw,tw->rowCount()-1);
        ddcContext->AddFilter(cal.id, defaultCustomFilter());
    }
    else{
        tw->removeCellWidget(tw->rowCount()-1,3);
        ddcContext->AddFilter(cal.id,cal.type,cal.freq, cal.gain, cal.bw, 48000.0,true);
    }

    tw->setSortingEnabled(true);
    tw->sortItems(1);
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

    tw->setSortingEnabled(false);
    (new tableproxy(tw))->editRow(oldcal,row);

    if(oldcal.type==biquad::CUSTOM){
        newCustomFilter(oldcal.custom,tw,row);
        ddcContext->ModifyFilter(oldcal.id, oldcal.custom);
    }
    else{
        tw->removeCellWidget(row,3);
        ddcContext->ModifyFilter(oldcal.id, oldcal.type, oldcal.freq, oldcal.gain, oldcal.bw, 48000.0,true);
    }

    tw->setSortingEnabled(true);
    tw->sortItems(1);

    host->drawGraph();
    *lockActions = false;
    mtx->unlock();
}
void EditCommand::redo()
{
    mtx->lock();
    *lockActions = true;

    if(cal.type==biquad::CUSTOM){
        newCustomFilter(cal.custom,tw,row);
        ddcContext->ModifyFilter(cal.id, cal.custom);
    }
    else{
        tw->removeCellWidget(row,3);
        ddcContext->ModifyFilter(cal.id, cal.type, cal.freq, cal.gain, cal.bw, 48000.0,true);
    }

    (new tableproxy(tw))->editRow(cal,row);

    tw->setSortingEnabled(true);
    tw->sortItems(1);

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
        (new tableproxy(tw))->addRow(cal);
        if(cal.type==biquad::CUSTOM){
            newCustomFilter(cal.custom,tw,tw->rowCount()-1);
            ddcContext->AddFilter(cal.id,cal.custom);
        }
        else
            ddcContext->AddFilter(cal.id,cal.type,cal.freq, cal.gain, cal.bw, 48000.0,true);
    }

    tw->setSortingEnabled(true);
    tw->sortItems(1);
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
    (new tableproxy(tw))->clearAll();

    host->drawGraph();
    *lockActions = false;
    mtx->unlock();
}
QString ClearCommand::createCommandString(){
    return QObject::tr("\"Clear all points\"");
}

RemoveCommand::RemoveCommand(QTableWidget* _tw,DDCContext* _ddcContext,std::vector<int> _rows,QModelIndexList _model_list,
                             std::mutex* _mtx,bool* _lockActions,MainWindow* _host, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    static int itemCount = 0;
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
        (new tableproxy(tw))->addRow(cal);
        if(cal.type==biquad::CUSTOM){
            newCustomFilter(cal.custom,tw,tw->rowCount()-1);
            ddcContext->AddFilter(cal.id,cal.custom);
        }
        else
            ddcContext->AddFilter(cal.id,cal.type,cal.freq, cal.gain, cal.bw, 48000.0,true);
    }

    tw->setSortingEnabled(true);
    tw->sortItems(1);
    tw->update();
    host->drawGraph();
    *lockActions = false;
    mtx->unlock();
}
void RemoveCommand::redo()
{
    mtx->lock();
    *lockActions = true;
    tw->setSortingEnabled(false);
    cal_table.clear();
    QList<int> removeRows;
    for (auto i : rows)
    {
        calibrationPoint_t cal;
        cal.id = tw->item(i,0)->data(Qt::UserRole).toUInt();
        cal.freq = tw->item(i,1)->data(Qt::DisplayRole).toInt();
        cal.bw = tw->item(i,2)->data(Qt::DisplayRole).toDouble();
        cal.gain = tw->item(i,3)->data(Qt::DisplayRole).toDouble();
        cal.type = stringToType(tw->item(i,0)->data(Qt::DisplayRole).toString());
        if(cal.type == biquad::CUSTOM)
            cal.custom = ((CustomFilterItem*)tw->cellWidget(i,3))->getCoefficients();
        cal_table.push_back(cal);

        removeRows.append(i);
        ddcContext->RemoveFilter(cal.id);
    }
    for(int i=0;i<removeRows.count();++i)
    {
        for(int j=i;j<removeRows.count();++j)
            if(removeRows.at(j) > removeRows.at(i))
                removeRows[j]--;
        tw->model()->removeRows(removeRows.at(i), 1);
    }

    tw->setSortingEnabled(true);
    tw->sortItems(1);
    host->drawGraph();

    *lockActions = false;
    mtx->unlock();
}
QString RemoveCommand::createCommandString(){
    return QObject::tr("\"Remove row(s)\"");
}
InvertCommand::InvertCommand(QTableWidget* _tw,DDCContext* _ddcContext,
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
void InvertCommand::undo()
{
    mtx->lock();
    *lockActions = true;
    tw->setSortingEnabled(false);

    for (size_t i = 0; i < cal_table.size(); i++)
    {
        calibrationPoint_t cal = cal_table.at(i);
        for (int j = 0; j < tw->rowCount(); j++)
        {
            if (tw->item(j,1)->data(Qt::DisplayRole).toInt() == cal.freq)
            {
                tw->item(j,3)->setData(Qt::DisplayRole,(double)cal.gain);
                ddcContext->ModifyFilter(cal.id,cal.type,cal.freq, cal.gain, cal.bw, 48000.0,true);
                break;
            }
        }
    }

    tw->setSortingEnabled(true);
    tw->sortItems(1);

    host->drawGraph();
    *lockActions = false;
    mtx->unlock();
}
void InvertCommand::redo()
{
    mtx->lock();
    *lockActions = true;
    tw->setSortingEnabled(false);

    for (size_t i = 0; i < cal_table.size(); i++)
    {
        calibrationPoint_t cal = cal_table.at(i);
        for (int j = 0; j < tw->rowCount(); j++)
        {
            if (tw->item(j,1)->data(Qt::DisplayRole).toInt() == cal.freq)
            {
                tw->item(j,3)->setData(Qt::DisplayRole,(double)-cal.gain);
                ddcContext->ModifyFilter(cal.id,cal.type,cal.freq, -cal.gain, cal.bw, 48000.0,true);
                break;
            }
        }
    }

    tw->setSortingEnabled(true);
    tw->sortItems(1);
    host->drawGraph();

    *lockActions = false;
    mtx->unlock();
}
QString InvertCommand::createCommandString(){
    return QObject::tr("\"Invert gain (selection)\"");
}

ShiftCommand::ShiftCommand(QTableWidget* _tw,DDCContext* _ddcContext,int _shift,
                           std::vector<calibrationPoint_t> _cal_table,std::mutex* _mtx,bool* _lockActions,MainWindow* _host, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    static int itemCount = 0;
    cal_table = _cal_table;
    tw = _tw;
    mtx = _mtx;
    host = _host;
    shift = _shift;
    ddcContext = _ddcContext;
    lockActions = _lockActions;
    ++itemCount;
    setText(createCommandString());
}
void ShiftCommand::undo()
{
    mtx->lock();
    *lockActions = true;
    tw->setSortingEnabled(false);

    for (size_t i = 0; i < cal_table.size(); i++)
    {
        calibrationPoint_t cal = cal_table.at(i);
        for (int j = 0; j < tw->rowCount(); j++)
        {
            if (tw->item(j,1)->data(Qt::DisplayRole).toInt() == cal.freq + shift)
            {
                tw->item(j,1)->setData(Qt::DisplayRole,(int)cal.freq);
                ddcContext->ModifyFilter(cal.id,cal.type,cal.freq, cal.gain, cal.bw, 48000.0,true);
                break;
            }
        }
    }

    tw->setSortingEnabled(true);
    tw->sortItems(1);

    host->drawGraph();
    *lockActions = false;
    mtx->unlock();
}
void ShiftCommand::redo()
{
    mtx->lock();
    *lockActions = true;
    tw->setSortingEnabled(false);
    for (size_t i = 0; i < cal_table.size(); i++)
    {
        calibrationPoint_t cal = cal_table.at(i);
        for (int j = 0; j < tw->rowCount(); j++)
        {
            if (tw->item(j,1)->data(Qt::DisplayRole).toInt() == cal.freq)
            {
                bool skip = false;
                for (int k = 0; k < tw->rowCount(); k++)
                {
                    if (tw->item(k,1)->data(Qt::DisplayRole).toInt() == cal.freq+shift)
                    {
                        skip=true;
                        break;
                    }
                }
                if(skip)break;
                tw->item(j,1)->setData(Qt::DisplayRole,(int)cal.freq+shift);
                ddcContext->ModifyFilter(cal.id,cal.type,cal.freq+shift, cal.gain, cal.bw, 48000.0,true);
                break;
            }
        }
    }

    tw->setSortingEnabled(true);
    tw->sortItems(1);
    host->drawGraph();

    *lockActions = false;
    mtx->unlock();
}
QString ShiftCommand::createCommandString(){
    return QObject::tr("\"Shift frequencies (selection)\"");
}
