#ifndef INVERTCOMMAND_H
#define INVERTCOMMAND_H

#include "model/FilterModel.h"

#include <QUndoCommand>


class InvertCommand : public QUndoCommand
{
public:
    InvertCommand(FilterModel* model, QModelIndexList indices, QUndoCommand *parent = 0)
        : QUndoCommand(parent), model(model), indices(indices)
    {
        setText(createCommandString());
    }

    void undo()
    {
        invert();
    }

    void redo()
    {
        invert();
    }

    void invert(){
        for (int i = 0; i < indices.count(); i++)
        {
            auto filter = DeflatedBiquad(model->getFilter(indices.at(i).row()));
            filter.gain = -filter.gain;
            model->replace(indices.at(i), filter, true);
        }
        model->emit dataChanged(indices.at(0), indices.at(indices.count() - 1));
    }

    QString createCommandString(){
        return "\"Invert gain of selection\"";
    }

private:
    FilterModel* model;
    QModelIndexList indices;
};

#endif // INVERTCOMMAND_H
