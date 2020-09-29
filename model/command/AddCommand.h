#ifndef ADDCOMMAND_H
#define ADDCOMMAND_H

#include "model/FilterModel.h"

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
        model->remove(biquad);
    }

    void redo()
    {
        model->append(biquad);
    }

    QString createCommandString(){
        return QObject::tr("\"Add '%1' filter\"").arg((QString)biquad->GetFilterType());
    }

private:
    FilterModel* model;
    Biquad* biquad;
};

#endif // ADDCOMMAND_H
