#ifndef REMOVECOMMAND_H
#define REMOVECOMMAND_H

#include <QUndoCommand>
#include <QDebug>

#include "model/FilterModel.h"

class RemoveCommand : public QUndoCommand
{
public:
    RemoveCommand(FilterModel* model, QModelIndexList indices, QUndoCommand *parent = 0)
        : QUndoCommand(parent), model(model), indices(indices){

        for (int i = 0; i < indices.count(); i++)
        {
            cache.append(model->getFilter(indices.at(i).row()));
        }

        setText(createCommandString());
    }

    RemoveCommand(FilterModel* model, QUndoCommand *parent = 0)
        : QUndoCommand(parent), model(model){
        setText(createCommandString());

        for (int i = 0; i < model->rowCount(); i++)
        {
            cache.append(model->getFilter(i));
        }
    }

    void undo()
    {
        model->appendAllDeflated(cache);
    }

    void redo()
    {
        QVector<uint32_t> ids;
        for( int i = cache.count(); i > 0; i--){
            ids.push_back(cache.at(i-1).id);
        }
        model->removeAllById(ids);
    }

    QString createCommandString(){
        return QString("\"Remove %1 row(s)\"").arg(cache.size());
    }

private:
    FilterModel* model;
    QModelIndexList indices;
    QVector<DeflatedBiquad> cache;
};

#endif // REMOVECOMMAND_H
