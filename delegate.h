#ifndef DELEGATE_H
#define DELEGATE_H

#include "filtertypes.h"
#include <QStyledItemDelegate>
#include <QWidget>
#include "mainwindow.h"

class SaveItemDelegate : public QStyledItemDelegate {
public:
    biquad::Type getType(const QString &_type) const{
        if(_type=="Peaking")return biquad::Type::PEAKING;
        else if(_type=="Low Pass")return biquad::Type::LOW_PASS;
        else if(_type=="High Pass")return biquad::Type::HIGH_PASS;
        else if(_type=="Band Pass")return biquad::Type::BAND_PASS2;
        else if(_type=="Band Pass (peak gain = bw)")return biquad::Type::BAND_PASS1;
        else if(_type=="All Pass")return biquad::Type::ALL_PASS;
        else if(_type=="Notch")return biquad::Type::NOTCH;
        else if(_type=="Low Shelf")return biquad::Type::LOW_SHELF;
        else if(_type=="High Shelf")return biquad::Type::HIGH_SHELF;
        else if(_type=="Unity Gain")return biquad::Type::UNITY_GAIN;
        else if(_type=="One-Pole Low Pass")return biquad::Type::ONEPOLE_LOWPASS;
        else if(_type=="One-Pole High Pass")return biquad::Type::ONEPOLE_HIGHPASS;
        else if(_type=="Custom")return biquad::Type::CUSTOM;
        return biquad::Type::PEAKING;
    }
    biquad::Type getType(const QModelIndex &index) const{
        QString _type = index.sibling(index.row(),0).data(Qt::DisplayRole).toString();
        return getType(_type);
    }

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        const QString currentType = index.sibling(index.row(),0).data(Qt::DisplayRole).toString();

        if (index.column()==1) {
            switch (getType(currentType)) {
            case biquad::UNITY_GAIN:
            case biquad::CUSTOM:
                return nullptr;
            default:
                break;
            }
        }
        else if (index.column()==2) {
            switch (getType(currentType)) {
            case biquad::UNITY_GAIN:
            case biquad::ONEPOLE_LOWPASS:
            case biquad::ONEPOLE_HIGHPASS:
            case biquad::CUSTOM:
                return nullptr;
                break;
            default:
                break;
            }
        }
        else if (index.column()==3) {
            switch (getType(currentType)) {
            case biquad::PEAKING:
            case biquad::LOW_SHELF:
            case biquad::UNITY_GAIN:
            case biquad::HIGH_SHELF:
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
            case biquad::LOW_SHELF:
            case biquad::HIGH_SHELF:
                sp->setPrefix("S: ");
                break;
            case biquad::UNITY_GAIN:
            case biquad::ONEPOLE_LOWPASS:
            case biquad::ONEPOLE_HIGHPASS:
            case biquad::CUSTOM:
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
            case biquad::PEAKING:
            case biquad::LOW_SHELF:
            case biquad::UNITY_GAIN:
            case biquad::HIGH_SHELF:
                QStyledItemDelegate::paint(painter,option,index);
                return;
            default:
                //Leave item empty
                return;
            }
        }
        else if (index.column()==2) {
            switch (getType(currentType)) {
            case biquad::UNITY_GAIN:
            case biquad::ONEPOLE_LOWPASS:
            case biquad::ONEPOLE_HIGHPASS:
            case biquad::CUSTOM:
                //Leave item empty
                return;
            default:
                QStyledItemDelegate::paint(painter,option,index);
                return;
            }
        }
        else if (index.column()==1) {
            switch (getType(currentType)) {
            case biquad::UNITY_GAIN:
            case biquad::CUSTOM:
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
            Global::old_type = getType(index);
        }
        else
            QStyledItemDelegate::setModelData(editor, model, index);
    }


};

#endif // DELEGATE_H
