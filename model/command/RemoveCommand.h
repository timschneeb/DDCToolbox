#ifndef REMOVECOMMAND_H
#define REMOVECOMMAND_H

#include <QUndoCommand>
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
        for(const auto& biquad : cache)
        {
            model->append(biquad.inflate());
        }
    }

    void redo()
    {
        for( int i = cache.count(); i > 0; i--){
            model->removeById(cache.at(i-1).id);
        }
    }

    QString createCommandString(){
        return QString("\"Remove %1 row(s)\"").arg(cache.size());
    }

private:
    FilterModel* model;
    QModelIndexList indices;
    QList<DeflatedBiquad> cache;
};

#endif // REMOVECOMMAND_H
