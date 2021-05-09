#ifndef FREQUENCYPLOT_H
#define FREQUENCYPLOT_H

#include "QCustomPlot.h"

#include <QObject>
#include <cmath>
#include <QTableWidget>

class FilterModel;

class FrequencyPlot : public QCustomPlot
{
public:
    enum class PlotType{
        magnitude,
        phase_response,
        group_delay
    };
    FrequencyPlot(QFrame* frame);
    void setMode(PlotType type, QWidget* parent);
    void setModel(QTableView* host, FilterModel* model);
    void setAllMarkerPointsVisible(bool b);

    void updatePlot(QVector<float> table,int bandCount);
    void updatePoints();
    void clearAll();
    void saveScreenshot();

private slots:
    void showPointToolTip(QMouseEvent *event);
    void drawMenu(const QPoint & pos);

private:
    PlotType m_type;
    bool m_markerVisible = false;
    FilterModel* model;
    QTableView* host;
    QVector<float> m_table;
    QDialog* m_externalWindow;
};

#endif // FREQUENCYPLOT_H
