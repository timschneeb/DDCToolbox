#ifndef FREQUENCYPLOT_H
#define FREQUENCYPLOT_H
#include "../qcustomplot.h"
#include <QObject>
#include <cmath>
#include <QTableWidget>
#include "ddccontext.h"
#include "filtertypes.h"
#include "qcustomplot.h"


class FrequencyPlot : public QCustomPlot
{
public:
    enum class PlotType{
        magnitude,
        phase_response,
        group_delay
    };
    FrequencyPlot(QFrame*);
    void setMode(PlotType type, QWidget* parent);
    void updatePlot(std::vector<float> table,int bandCount);
    void updatePoints(QTableWidget* tb,bool allMarkerPointsVisible);
    void updatePoints(QCPGraph* plot,QTableWidget* tb,bool allMarkerPointsVisible);
    void clear();
    void saveScreenshot();

private slots:
    void showPointToolTip(QMouseEvent *event);
    void drawMenu(const QPoint & pos);

private:
    PlotType m_type;
    std::vector<float> m_table;
};

#endif // FREQUENCYPLOT_H
