#ifndef SHIFTCOMMAND_H
#define SHIFTCOMMAND_H

#include "model/FilterModel.h"

#include <QUndoCommand>

class ShiftCommand : public QUndoCommand
{
public:
    ShiftCommand(FilterModel* model, QModelIndexList indices, int shiftAmount, QUndoCommand *parent = 0)
        : QUndoCommand(parent), model(model), indices(indices), shiftAmount(shiftAmount)
    {
        setText(createCommandString());
    }

    void undo()
    {
        for (int i = 0; i < indices.count(); i++)
        {
            auto filter = DeflatedBiquad(model->getFilter(indices.at(i).row()));
            filter.freq = filter.freq - shiftAmount;
            model->replace(indices.at(i), filter, true);
        }
        model->emit dataChanged(indices.at(0), indices.at(indices.count() - 1));
    }
    void redo()
    {
        for (int i = 0; i < indices.count(); i++)
        {
            auto filter = DeflatedBiquad(model->getFilter(indices.at(i).row()));
            filter.freq = filter.freq + shiftAmount;
            model->replace(indices.at(i), filter, true);
        }

        model->emit dataChanged(indices.at(0), indices.at(indices.count() - 1));
    }

    QString createCommandString(){
        return QObject::tr("\"Shift frequencies (selection)\"");
    }


private:
    FilterModel* model;
    QModelIndexList indices;
    int shiftAmount;
};


#endif // SHIFTCOMMAND_H
