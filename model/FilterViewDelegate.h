#ifndef FILTERVIEWDELEGATE_H
#define FILTERVIEWDELEGATE_H

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QApplication>

#include "model/command/EditCommand.h"
#include "item/CustomFilterListItem.h"

class FilterViewDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    FilterViewDelegate() : QStyledItemDelegate() {}

    void setModel(FilterModel* model)
    {
        m_model = model;
    };

    FilterType getType(const QModelIndex &index) const
    {
        return FilterType(index.sibling(index.row(),0).data(Qt::DisplayRole).toString());
    }

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        auto specs = getType(index).getSpecs();

        switch(index.column()){
        case 1:
            if(!specs.test(FilterType::SPEC_REQUIRE_FREQ))
                return nullptr;
            break;
        case 2:
            if(!specs.test(FilterType::SPEC_REQUIRE_BW) && !specs.test(FilterType::SPEC_REQUIRE_SLOPE))
                return nullptr;
            break;
        case 3:
            if(!specs.test(FilterType::SPEC_REQUIRE_GAIN) && getType(index) != FilterType::CUSTOM)
                return nullptr;
            break;
        }

        auto w = QStyledItemDelegate::createEditor(
                    parent, option, index);

        auto sp = qobject_cast<QDoubleSpinBox*>(w);
        if (sp)
        {
            sp->setDecimals(6);
        }

        if(index.column()==0){
            QComboBox *cb = new QComboBox(parent);

            for(ulong i = 0; i < FilterType::string_map_size; i++){
                if(FilterType::string_map[i].first == FilterType::INVALID)
                    continue;
                cb->addItem(FilterType::string_map[i].second);
            }

            return cb;
        }
        else if (index.column()==2&&sp) {
            if(specs.test(FilterType::SPEC_REQUIRE_SLOPE))
                sp->setPrefix("S: ");
            else if(specs.test(FilterType::SPEC_REQUIRE_BW))
                sp->setPrefix("BW: ");
            else
                sp->setPrefix("");
        }
        else if (index.column() == 3 && getType(index) == FilterType::CUSTOM){
            CustomFilterItem* cf_item = new CustomFilterItem(parent);
            connect(cf_item, &CustomFilterItem::coefficientsUpdated,
                    [this, index](CustomFilter prev44100, CustomFilter prev48000,
                    CustomFilter c44100, CustomFilter c48000){

                DeflatedBiquad previous(FilterType::CUSTOM, prev44100, prev48000);
                DeflatedBiquad current(FilterType::CUSTOM, c44100, c48000);

                emit requireEditCommit(new EditCommand(m_model, previous, current, index));
            });
            return cf_item;
        }
        return w;
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        auto specs = getType(index).getSpecs();


        switch(index.column()){
        case 0: {
            auto model = static_cast<FilterModel*>(m_model);
            if(model->getDebugMode()){
                QStyleOptionButton button;
                QRect r = option.rect;
                button.rect = QRect(r.left() + 2,
                                    r.top() + 2,
                                    r.width() - 4,
                                    r.height() - 4);
                button.text = "ID: " + QString::number(model->getFilter(index.row())->GetId());
                button.state = option.state;
                QApplication::style()->drawControl(QStyle::CE_PushButton,
                                                   &button, painter);
                return;
            }

        }
        case 1: {
            if(!specs.test(FilterType::SPEC_REQUIRE_FREQ))
                return;
            break;
        }
        case 2:
            if(!specs.test(FilterType::SPEC_REQUIRE_BW) && !specs.test(FilterType::SPEC_REQUIRE_SLOPE))
                return;
            break;
        case 3:
            if(!specs.test(FilterType::SPEC_REQUIRE_GAIN) && getType(index) != FilterType::CUSTOM)
                return;
            break;
        }

        if(index.column() == 3 && getType(index) == FilterType::CUSTOM){
            QStyleOptionButton button;
            QRect r = option.rect;
            button.rect = QRect(r.left() + 2,
                                r.top() + 2,
                                r.width() - 4,
                                r.height() - 4);
            button.text = "Configure";
            button.state = option.state;
            QApplication::style()->drawControl(QStyle::CE_PushButton,
                                               &button, painter);
        }
        else{
            QStyledItemDelegate::paint(painter,option,index);
        }
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        auto filter = static_cast<FilterModel*>(m_model)->getFilter(index.row());
        const QString currentText = index.data(Qt::EditRole).toString();

        if(index.column() == 0){
            QComboBox *cb = qobject_cast<QComboBox *>(editor);
            Q_ASSERT(cb);
            const int cbIndex = cb->findText(currentText);
            if (cbIndex >= 0)
                cb->setCurrentIndex(cbIndex);
        }
        else if (index.column() == 3 && getType(index) == FilterType::CUSTOM){
            qobject_cast<CustomFilterItem*>(editor)->setCoefficients(filter->GetCustomFilter(44100),
                                                                     filter->GetCustomFilter(48000));
        }
        else
            QStyledItemDelegate::setEditorData(editor,index);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        if(index.column() == 0){
            QComboBox *cb = qobject_cast<QComboBox *>(editor);
            Q_ASSERT(cb);
            model->setData(index, cb->currentText(), Qt::EditRole);
        }
        else
            QStyledItemDelegate::setModelData(editor, model, index);
    }

signals:
    void requireEditCommit(EditCommand*) const;

private:
    FilterModel* m_model;
};

#endif // FILTERVIEWDELEGATE_H
