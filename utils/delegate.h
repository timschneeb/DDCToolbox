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
      case 0: return filter->;
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
   void append(const Vehicle & vehicle) {
      beginInsertRows({}, m_data.count(), m_data.count());
      m_data.append(vehicle);
      endInsertRows();
   }
};
*/
class SaveItemDelegate : public QStyledItemDelegate {
public:
    Biquad::Type getType(const QString &_type) const{
        if(_type=="Peaking")return Biquad::Type::PEAKING;
        else if(_type=="Low Pass")return Biquad::Type::LOW_PASS;
        else if(_type=="High Pass")return Biquad::Type::HIGH_PASS;
        else if(_type=="Band Pass")return Biquad::Type::BAND_PASS2;
        else if(_type=="Band Pass (peak gain = bw)")return Biquad::Type::BAND_PASS1;
        else if(_type=="All Pass")return Biquad::Type::ALL_PASS;
        else if(_type=="Notch")return Biquad::Type::NOTCH;
        else if(_type=="Low Shelf")return Biquad::Type::LOW_SHELF;
        else if(_type=="High Shelf")return Biquad::Type::HIGH_SHELF;
        else if(_type=="Unity Gain")return Biquad::Type::UNITY_GAIN;
        else if(_type=="One-Pole Low Pass")return Biquad::Type::ONEPOLE_LOWPASS;
        else if(_type=="One-Pole High Pass")return Biquad::Type::ONEPOLE_HIGHPASS;
        else if(_type=="Custom")return Biquad::Type::CUSTOM;
        return Biquad::Type::PEAKING;
    }
    Biquad::Type getType(const QModelIndex &index) const{
        QString _type = index.sibling(index.row(),0).data(Qt::DisplayRole).toString();
        return getType(_type);
    }

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        const QString currentType = index.sibling(index.row(),0).data(Qt::DisplayRole).toString();

        if (index.column()==1) {
            switch (getType(currentType)) {
            case Biquad::UNITY_GAIN:
            case Biquad::CUSTOM:
                return nullptr;
            default:
                break;
            }
        }
        else if (index.column()==2) {
            switch (getType(currentType)) {
            case Biquad::UNITY_GAIN:
            case Biquad::ONEPOLE_LOWPASS:
            case Biquad::ONEPOLE_HIGHPASS:
            case Biquad::CUSTOM:
                return nullptr;
                break;
            default:
                break;
            }
        }
        else if (index.column()==3) {
            switch (getType(currentType)) {
            case Biquad::PEAKING:
            case Biquad::LOW_SHELF:
            case Biquad::UNITY_GAIN:
            case Biquad::HIGH_SHELF:
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
            const int row = index.row();
            cb->addItem(QString("Peaking"));
            cb->addItem(QString("Low Pass"));
            cb->addItem(QString("High Pass"));
            cb->addItem(QString("Band Pass"));
            cb->addItem(QString("Band Pass (peak gain = bw)"));
            cb->addItem(QString("Notch"));
            cb->addItem(QString("All Pass"));
            cb->addItem(QString("Low Shelf"));
            cb->addItem(QString("High Shelf"));
            cb->addItem(QString("Unity Gain"));
            cb->addItem(QString("One-Pole Low Pass"));
            cb->addItem(QString("One-Pole High Pass"));
            cb->addItem(QString("Custom"));
            return cb;
        }
        else if (index.column()==2&&sp) {
            switch (getType(currentType)) {
            case Biquad::LOW_SHELF:
            case Biquad::HIGH_SHELF:
                sp->setPrefix("S: ");
                break;
            case Biquad::UNITY_GAIN:
            case Biquad::ONEPOLE_LOWPASS:
            case Biquad::ONEPOLE_HIGHPASS:
            case Biquad::CUSTOM:
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
            switch (getType(currentType)) {
            case Biquad::PEAKING:
            case Biquad::LOW_SHELF:
            case Biquad::UNITY_GAIN:
            case Biquad::HIGH_SHELF:
                QStyledItemDelegate::paint(painter,option,index);
                return;
            default:
                //Leave item empty
                return;
            }
        }
        else if (index.column()==2) {
            switch (getType(currentType)) {
            case Biquad::UNITY_GAIN:
            case Biquad::ONEPOLE_LOWPASS:
            case Biquad::ONEPOLE_HIGHPASS:
            case Biquad::CUSTOM:
                //Leave item empty
                return;
            default:
                QStyledItemDelegate::paint(painter,option,index);
                return;
            }
        }
        else if (index.column()==1) {
            switch (getType(currentType)) {
            case Biquad::UNITY_GAIN:
            case Biquad::CUSTOM:
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
