#ifndef QTINTABLEPROGRESSBAR_H
#define QTINTABLEPROGRESSBAR_H

#include <QProgressBar>
#include <QStyleOptionProgressBar>
#include <QPainter>


class QTintableProgressBar : public QProgressBar {
public:
    QTintableProgressBar(QWidget* parent = nullptr) : QProgressBar(parent){}

    QColor getTint() const;
    void setTint(const QColor &value);

protected:
    void paintEvent(QPaintEvent* ev)
    {
        if(tint == Qt::white){
            QProgressBar::paintEvent(ev);
            return;
        }

        QStyleOptionProgressBar option{};
        initStyleOption(&option);
        option.textAlignment = Qt::AlignHCenter;
        option.palette.setColor(QPalette::Highlight, tint);
        option.palette.setColor(QPalette::HighlightedText, option.palette.color(QPalette::Text));
        QPainter painter{this};
        style()->drawControl(QStyle::CE_ProgressBar, &option, &painter, this);
    }

private:
    QColor tint = QColor(Qt::white);

};

inline QColor QTintableProgressBar::getTint() const
{
    return tint;
}

inline void QTintableProgressBar::setTint(const QColor &value)
{
    tint = value;
    this->repaint();
}

#endif // QTINTABLEPROGRESSBAR_H
