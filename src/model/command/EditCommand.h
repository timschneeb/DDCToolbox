#ifndef EDITCOMMAND_H
#define EDITCOMMAND_H

#include <QModelIndex>
#include <QUndoCommand>
#include "model/Biquad.h"
#include "model/FilterModel.h"

class EditCommand : public QUndoCommand
{
public:
    EditCommand(FilterModel* model, DeflatedBiquad previous, DeflatedBiquad current, QModelIndex index, QUndoCommand *parent = nullptr)
        : QUndoCommand(parent), model(model), previous(previous), current(current), index(index)
    {
        setText(createCommandString());
    }

    void undo()
    {
        model->replaceById(current.id, previous);
    }

    void redo()
    {
        model->replaceById(previous.id, current);
    }

    QString createCommandString(){
        return QObject::tr("\"Edit row %1\"").arg(index.row());
    }

private:
    FilterModel* model;
    DeflatedBiquad previous;
    DeflatedBiquad current;
    QModelIndex index;
};

#endif // EDITCOMMAND_H
