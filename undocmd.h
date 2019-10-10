#ifndef UNDOCMD_H
#define UNDOCMD_H
#include <QUndoCommand>
#include "mainwindow.h"

class AddCommand : public QUndoCommand
{
public:
    AddCommand(QTableWidget*,DDCContext*,calibrationPoint_t,std::mutex*,bool*,
               MainWindow* host,QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;
    QString createCommandString();

private:
    MainWindow* host;
    calibrationPoint_t cal;
    QTableWidget* tw;
    std::mutex* mtx;
    DDCContext* ddcContext;
    bool* lockActions;
};

class EditCommand : public QUndoCommand
{
public:
    EditCommand(QTableWidget*,DDCContext*,int,calibrationPoint_t,calibrationPoint_t,std::mutex*,bool*,
               MainWindow* host,QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;
    QString createCommandString();

private:
    MainWindow* host;
    calibrationPoint_t cal;
    calibrationPoint_t oldcal;
    int row;
    QTableWidget* tw;
    std::mutex* mtx;
    DDCContext* ddcContext;
    bool* lockActions;
};

class ClearCommand : public QUndoCommand
{
public:
    ClearCommand(QTableWidget*,DDCContext*,std::vector<calibrationPoint_t>,std::mutex*,bool*,
               MainWindow* host,QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;
    QString createCommandString();

private:
    MainWindow* host;
    std::vector<calibrationPoint_t> cal_table;
    QTableWidget* tw;
    std::mutex* mtx;
    DDCContext* ddcContext;
    bool* lockActions;
};

class RemoveCommand : public QUndoCommand
{
public:
    RemoveCommand(QTableWidget*,DDCContext*,std::vector<int> rows,QModelIndexList model_list,std::vector<calibrationPoint_t>,std::mutex*,bool*,
               MainWindow* host,QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;
    QString createCommandString();

private:
    MainWindow* host;
    std::vector<calibrationPoint_t> cal_table;
    QTableWidget* tw;
    std::mutex* mtx;
    DDCContext* ddcContext;
    QModelIndexList model_list;
    bool* lockActions;
    std::vector<int> rows;
};

class InvertCommand : public QUndoCommand
{
public:
    InvertCommand(QTableWidget*,DDCContext*,std::vector<calibrationPoint_t>,std::mutex*,bool*,
               MainWindow* host,QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;
    QString createCommandString();

private:
    MainWindow* host;
    std::vector<calibrationPoint_t> cal_table;
    QTableWidget* tw;
    std::mutex* mtx;
    DDCContext* ddcContext;
    bool* lockActions;
};

class ShiftCommand : public QUndoCommand
{
public:
    ShiftCommand(QTableWidget*,DDCContext*,int shift,std::vector<calibrationPoint_t>,std::mutex*,bool*,
               MainWindow* host,QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;
    QString createCommandString();

private:
    MainWindow* host;
    std::vector<calibrationPoint_t> cal_table;
    QTableWidget* tw;
    std::mutex* mtx;
    DDCContext* ddcContext;
    int shift;
    bool* lockActions;
};
#endif // UNDOCMD_H
