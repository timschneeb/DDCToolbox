#ifndef CONFIGITEM_H
#define CONFIGITEM_H

#include <QWidget>
#include <QStyledItemDelegate>

class ItemSizeDelegate : public QStyledItemDelegate  {
public:
    ItemSizeDelegate(QObject *parent=0) : QStyledItemDelegate (parent){}

    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index) const{
        Q_UNUSED(index);
        return QSize(option.widget->height(), 55);
    }
};

namespace Ui {
class configitem;
}

class DetailListItem : public QWidget
{
    Q_OBJECT

public:
    explicit DetailListItem(QWidget *parent = nullptr);
    ~DetailListItem();
    void setData(const QString& title, const QString& desc);

private:
    Ui::configitem *ui;
};

#endif // CONFIGITEM_H
