#ifndef ADDCOMMAND_H
#define ADDCOMMAND_H

#include "model/FilterModel.h"

#include <QDebug>
#include <QUndoCommand>

class AddCommand : public QUndoCommand
{
public:
    AddCommand(FilterModel* model, Biquad* biquad, QUndoCommand *parent = nullptr)
        :  QUndoCommand(parent), model(model), biquad(biquad)
    {
        setText(createCommandString());
    }

    void undo()
    {
        model->removeById(biquad.id());
    }

    void redo()
    {
        model->append(biquad.inflate());
    }

    QString createCommandString(){
        return QObject::tr("\"Add '%1' filter\"").arg((QString)biquad.type);
    }

private:
    FilterModel* model;
    DeflatedBiquad biquad;
};

#endif // ADDCOMMAND_H
