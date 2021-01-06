#ifndef PROXYSTYLE_H
#define PROXYSTYLE_H

#include <QProxyStyle>


class ProxyStyle: public QProxyStyle
{
    Q_OBJECT

public:
    ProxyStyle(QStyle * style = 0) : QProxyStyle(style)
    {
    }

    ProxyStyle(const QString & key) : QProxyStyle(key)
    {
    }

    virtual int pixelMetric(QStyle::PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0 ) const
    {
        switch ( metric )
        {
        case QStyle::PM_SmallIconSize:
            return 16;
        default:
            return QProxyStyle::pixelMetric( metric, option, widget );
        }
    }
};

#endif // PROXYSTYLE_H
