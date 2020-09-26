#ifndef DELEGATE_H
#define DELEGATE_H

#include "filtertypes.h"
#include <QStyledItemDelegate>
#include <QWidget>
#include "mainwindow.h"

/*class FilterModel : public QAbstractTableModel {
   QList<Biquad*> m_data;
public:
   FilterModel(QObject * parent = {}) : QAbstractTableModel{parent} {}
   int rowCount(const QModelIndex &) const override { return m_data.count(); }
   int columnCount(const QModelIndex &) const override { return 4; }

   QVariant data(const QModelIndex &index, int role) const override {
      if (role != Qt::DisplayRole && role != Qt::EditRole) return {};
      const auto & filter = m_data[index.row()];
      switch (index.column()) {
      case 0: return filter->GetFilterType();
      case 1: return filter.model();
      case 2: return filter.registrationNumber();
      default: return {};
      };
   }
   QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
      if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
      switch (section) {
      case 0: return "Type";
      case 1: return "Frequency";
      case 2: return "Bandwidth/Slope";
      case 3: return "Gain";
      default: return {};
      }
   }
   void append(const Biquad& vehicle) {
      beginInsertRows({}, m_data.count(), m_data.count());
      m_data.append(vehicle);
      endInsertRows();
   }
};
*/
class SaveItemDelegate : public QStyledItemDelegate {
public:
    FilterType getType(const QModelIndex &index) const{
        QString _type = index.sibling(index.row(),0).data(Qt::DisplayRole).toString();
        return FilterType(_type.toLocal8Bit().constData());
    }

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        const QString currentType = index.sibling(index.row(),0).data(Qt::DisplayRole).toString();

        if (index.column()==1) {
            switch (FilterType(currentType.toLocal8Bit().constData())) {
            case FilterType::UNITY_GAIN:
            case FilterType::CUSTOM:
                return nullptr;
            default:
                break;
            }
        }
        else if (index.column()==2) {
            switch (FilterType(currentType.toLocal8Bit().constData())) {
            case FilterType::UNITY_GAIN:
            case FilterType::ONEPOLE_LOWPASS:
            case FilterType::ONEPOLE_HIGHPASS:
            case FilterType::CUSTOM:
                return nullptr;
                break;
            default:
                break;
            }
        }
        else if (index.column()==3) {
            switch (FilterType(currentType.toLocal8Bit().constData())) {
            case FilterType::PEAKING:
            case FilterType::LOW_SHELF:
            case FilterType::UNITY_GAIN:
            case FilterType::HIGH_SHELF:
                break;
            default:
                return nullptr;
            }
        }

        auto w = QStyledItemDelegate::createEditor(
                    parent, option, index);

        auto sp = qobject_cast<QDoubleSpinBox*>(w);
        if (sp)
        {
            sp->setDecimals(6);
        }
        if(index.model()->columnCount()>3){
            Global::old_freq = index.sibling(index.row(),1).data(Qt::DisplayRole).toInt();
            Global::old_bw = index.sibling(index.row(),2).data(Qt::DisplayRole).toDouble();
            Global::old_gain = index.sibling(index.row(),3).data(Qt::DisplayRole).toDouble();
            Global::old_type = getType(index);
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
            switch (FilterType(currentType.toLocal8Bit().constData())) {
            case FilterType::LOW_SHELF:
            case FilterType::HIGH_SHELF:
                sp->setPrefix("S: ");
                break;
            case FilterType::UNITY_GAIN:
            case FilterType::ONEPOLE_LOWPASS:
            case FilterType::ONEPOLE_HIGHPASS:
            case FilterType::CUSTOM:
                sp->setPrefix("");
                break;
            default:
                sp->setPrefix("BW: ");
            }
        }
        return w;
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        const QString currentType = index.sibling(index.row(),0).data(Qt::DisplayRole).toString();
        if (index.column()==3) {
            switch (FilterType(currentType.toLocal8Bit().constData())) {
            case FilterType::PEAKING:
            case FilterType::LOW_SHELF:
            case FilterType::UNITY_GAIN:
            case FilterType::HIGH_SHELF:
                QStyledItemDelegate::paint(painter,option,index);
                return;
            default:
                //Leave item empty
                return;
            }
        }
        else if (index.column()==2) {
            switch (FilterType(currentType.toLocal8Bit().constData())) {
            case FilterType::UNITY_GAIN:
            case FilterType::ONEPOLE_LOWPASS:
            case FilterType::ONEPOLE_HIGHPASS:
            case FilterType::CUSTOM:
                //Leave item empty
                return;
            default:
                QStyledItemDelegate::paint(painter,option,index);
                return;
            }
        }
        else if (index.column()==1) {
            switch (FilterType(currentType.toLocal8Bit().constData())) {
            case FilterType::UNITY_GAIN:
            case FilterType::CUSTOM:
                //Leave item empty
                return;
            default:
                QStyledItemDelegate::paint(painter,option,index);
                return;
            }
        }
        else
            QStyledItemDelegate::paint(painter,option,index);
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        // get the index of the text in the combobox that matches the current value of the item
        const QString currentText = index.data(Qt::EditRole).toString();

        if(index.column()==0){
            QComboBox *cb = qobject_cast<QComboBox *>(editor);
            Q_ASSERT(cb);
            const int cbIndex = cb->findText(currentText);
            // if it is valid, adjust the combobox
            if (cbIndex >= 0)
                cb->setCurrentIndex(cbIndex);

        }

        else
            QStyledItemDelegate::setEditorData(editor,index);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        if(index.column()==0){
            QComboBox *cb = qobject_cast<QComboBox *>(editor);
            Q_ASSERT(cb);
            model->setData(index, cb->currentText(), Qt::EditRole);
        }
        else
            QStyledItemDelegate::setModelData(editor, model, index);
    }


};

#endif // DELEGATE_H
