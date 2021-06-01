#ifndef INVERTCOMMAND_H
#define INVERTCOMMAND_H

#include "model/FilterModel.h"

#include <QUndoCommand>


class InvertCommand : public QUndoCommand
{
public:
    InvertCommand(FilterModel* model, QModelIndexList indices, QUndoCommand *parent = 0)
        : QUndoCommand(parent), model(model)
    {
        for (int i = 0; i < indices.count(); i++)
        {
            cache.append(model->getFilter(indices.at(i).row())->GetId());
        }

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
        QModelIndexList indices;
        for (int i = 0; i < cache.count(); i++)
        {
            auto ref = model->getFilterById(cache.at(i));
            if(ref == nullptr){
                qWarning() << "InvertCommand::invert: getFilterById(" << cache.at(i)
                           << ") returned nullptr";
                continue;
            }

            auto filter = DeflatedBiquad(ref);
            filter.gain = -filter.gain;
            indices.append(model->replaceById(cache.at(i), filter, true));
        }
        qSort(indices.begin(), indices.end(), qGreater<QModelIndex>());

        emit model->dataChanged(indices.first(), indices.last().siblingAtColumn(3));
    }

    QString createCommandString(){
        return "\"Invert gain of selection\"";
    }

private:
    FilterModel* model;
    QVector<uint> cache;
};

#endif // INVERTCOMMAND_H
