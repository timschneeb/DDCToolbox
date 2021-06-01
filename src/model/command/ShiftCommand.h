#ifndef SHIFTCOMMAND_H
#define SHIFTCOMMAND_H

#include "model/FilterModel.h"

#include <QUndoCommand>
#include <QDebug>

class ShiftCommand : public QUndoCommand
{
public:
    ShiftCommand(FilterModel* model, QModelIndexList indices, int shiftAmount, QUndoCommand *parent = 0)
        : QUndoCommand(parent), model(model), shiftAmount(shiftAmount)
    {
        for (int i = 0; i < indices.count(); i++)
        {
            cache.append(model->getFilter(indices.at(i).row())->GetId());
        }

        setText(createCommandString());
    }

    void undo()
    {
        QModelIndexList indices;
        for (int i = 0; i < cache.count(); i++)
        {
            auto ref = model->getFilterById(cache.at(i));
            if(ref == nullptr){
                qWarning() << "ShiftCommand::undo: getFilterById(" << cache.at(i)
                           << ") returned nullptr";
                continue;
            }

            auto filter = DeflatedBiquad(ref);
            filter.freq = filter.freq - shiftAmount;
            indices.append(model->replaceById(cache.at(i), filter, true));
        }
        qSort(indices.begin(), indices.end(), qGreater<QModelIndex>());

        emit model->dataChanged(indices.first(), indices.last().siblingAtColumn(3));
    }
    void redo()
    {
        QModelIndexList indices;
        for (int i = 0; i < cache.count(); i++)
        {
            auto ref = model->getFilterById(cache.at(i));
            if(ref == nullptr){
                qWarning() << "ShiftCommand::undo: getFilterById(" << cache.at(i)
                           << ") returned nullptr";
                continue;
            }

            auto filter = DeflatedBiquad(ref);
            filter.freq = filter.freq + shiftAmount;
            indices.append(model->replaceById(cache.at(i), filter, true));
        }
        qSort(indices.begin(), indices.end(), qGreater<QModelIndex>());

        emit model->dataChanged(indices.first(), indices.last().siblingAtColumn(3));
    }

    QString createCommandString(){
        return QObject::tr("\"Shift frequencies of selection\"");
    }


private:
    FilterModel* model;
    QVector<uint> cache;
    int shiftAmount;
};


#endif // SHIFTCOMMAND_H
