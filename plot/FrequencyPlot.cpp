#include "FrequencyPlot.h"
#include <utility>
#include <cassert>
#include <model/FilterModel.h>

FrequencyPlot::FrequencyPlot(QFrame* frame) : QCustomPlot(frame)
{
    qsrand(QDateTime::currentMSecsSinceEpoch() / 1000);
}

void FrequencyPlot::setMode(PlotType type, QWidget* parent){
    m_type = type;

    QCPRange xRange(20,24000);
    QCPRange yRange;
    QString xLabel("Frequency (Hz)");
    QString yLabel;

    switch (m_type) {
    case PlotType::magnitude:
        yLabel = tr("Gain (dB)");
        yRange = QCPRange(-24,24);
        break;
    case PlotType::phase_response:
        yLabel = tr("Phase (deg)");
        yRange = QCPRange(-24,24);
        break;
    case PlotType::group_delay:
        yLabel = tr("Group delay (ms)");
        yRange = QCPRange(-5,5);
        break;
    }

    yAxis->setRange(yRange);
    yAxis->setLabel(yLabel);
    xAxis->setRange(xRange);
    xAxis->setLabel(xLabel);
    xAxis->setScaleType(QCPAxis::stLogarithmic);

    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    xAxis->setTicker(logTicker);
    xAxis->setScaleType(QCPAxis::stLogarithmic);
    rescaleAxes();

    connect(this, &QCustomPlot::mouseMove, this,[this](QMouseEvent* event){
        int x = (int)xAxis->pixelToCoord(event->pos().x());
        if(x < 0 || x > 24000)return;
        setToolTip(QString("%1Hz").arg(x));
    });
    setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(this, &QCustomPlot::customContextMenuRequested,
            this, [this,parent](const QPoint & pos){
        QMenu* menu = new QMenu(tr("Options"),parent);

        QAction move(tr("Enable zoom/drag"), this);
        move.setCheckable(true);
        move.setChecked(interactions() & (QCP::Interaction::iRangeDrag));
        connect(&move, &QAction::changed, this, [&move,this]() {
            bool val = move.isChecked();
            setInteraction(QCP::Interaction::iRangeDrag,val);
            setInteraction(QCP::Interaction::iRangeZoom,val);
            if(!val){
                removeGraph(0);
                updatePlot(m_table,8192 * 2);
            }
        });
        QAction reload(tr("Reload"), this);
        connect(&reload, &QAction::triggered, this, [this](){
            removeGraph(0);
            updatePlot(m_table,8192 * 2);
        });

        menu->addAction(&move);
        menu->addAction(&reload);
        menu->exec(mapToGlobal(pos));
    });
}

void FrequencyPlot::setModel(QTableView *_host, FilterModel *_model)
{
    model = _model;
    host =_host;
}

void FrequencyPlot::setAllMarkerPointsVisible(bool b)
{
    m_markerVisible = b;
}

void FrequencyPlot::updatePoints(){
    QCPGraph *plot = graph(0);

    if(plot == nullptr)
        return;

    //Remove all dots
    for(int i = 0; i < graphCount(); i++){
        if(graph(i)->name().startsWith("points")||
            graph(i)->name().startsWith("sel_points")){
            removeGraph(i);
        }
    }

    QCPGraph *points = addGraph();
    points->setAdaptiveSampling(false);
    points->removeFromLegend();
    points->setLineStyle(QCPGraph::lsNone);
    points->setScatterStyle(QCPScatterStyle::ssDot);
    points->setPen(QPen(QBrush(Qt::darkRed),3));
    points->setName("points_" + QString::number(qrand()));
    QCPGraph *sel_points = addGraph();
    sel_points->setAdaptiveSampling(false);
    sel_points->removeFromLegend();
    sel_points->setLineStyle(QCPGraph::lsNone);
    sel_points->setScatterStyle(QCPScatterStyle::ssDot);
    sel_points->setPen(QPen(QBrush(Qt::red),3));
    sel_points->setName("sel_points_" + QString::number(qrand()));

    QCPItemTracer *tracer = new QCPItemTracer(this);
    tracer->setGraph(plot);
    tracer->setStyle(QCPItemTracer::TracerStyle::tsNone);

    //Draw all non-selected points (if enabled)
    if(m_markerVisible){
        for (int i = 0; i < model->rowCount(); i++) {
            if(model->getFilter(i)->GetFilterType() == FilterType::UNITY_GAIN ||
                    model->getFilter(i)->GetFilterType() == FilterType::CUSTOM)
                continue;

            if(host->selectionModel()->selectedRows(0).contains(model->index(i, 0)))
                continue;

            int freq = model->getFilter(i)->GetFrequency();
            tracer->setGraphKey(freq);
            tracer->updatePosition();
            points->addData(freq,tracer->position->value());
        }
    }
    //Draw selected points
    for (const auto& index : host->selectionModel()->selectedRows(0)){
        if(model->getFilter(index.row())->GetFilterType() == FilterType::UNITY_GAIN ||
                model->getFilter(index.row())->GetFilterType() == FilterType::CUSTOM)
            continue;

        int freq = model->getFilter(index.row())->GetFrequency();
        tracer->setGraphKey(freq);
        tracer->updatePosition();
        sel_points->addData(freq,tracer->position->value());
    }
    replot();
}

void FrequencyPlot::updatePlot(QVector<float> table,int bandCount){
    m_table = std::move(table);
    if (m_table.empty())
        return;

    QCPGraph *plot;
    float max = 24.0f;
    float min = -24.0f;

    switch (m_type) {
    case PlotType::phase_response:
        for (int i = 0; i < (int)bandCount; i++){
            if (m_table.at(i) > max)
                max = m_table.at(i);
            if (m_table.at(i) < min)
                min = m_table.at(i);
        }

        yAxis->setRange(QCPRange(min, max));
        plot = addGraph();
        plot->setAdaptiveSampling(false);

        for (int m = 0; m < (int)bandCount; m++)
        {
            double num3 = (48000.0 / 2.0) / ((double) bandCount);
            plot->addData(num3 * (m + 1.0),(double)m_table.at(m));
            xAxis->setRange(QCPRange(20, num3 * (m + 1.0)));
        }
        break;
    case PlotType::magnitude:
        for (int i = 0; i < (int)bandCount; i++){
            if (m_table.at(i) > max)
                max = m_table.at(i);
            if (m_table.at(i) < min)
                min = m_table.at(i);
        }

        yAxis->setRange(QCPRange(min, max));
        plot = addGraph();
        plot->setAdaptiveSampling(false);

        for (int m = 0; m < (int)bandCount; m++)
        {
            double num3 = (48000.0 / 2.0) / ((double) bandCount);
            plot->addData(num3 * (m + 1.0),(double)m_table.at(m));
            xAxis->setRange(QCPRange(20, num3 * (m + 1.0)));
        }
        break;
    case PlotType::group_delay:
        max = 5.0f;
        min = -5.0f;

        for (float i : m_table){
            if (i > max)
                max = i;
            if (i < min)
                min = i;
        }
        if(std::isinf(min))min=-5.0;
        if(std::isinf(max))max=5.0;

        yAxis->setRange(QCPRange(min, max));
        plot = addGraph();
        plot->setAdaptiveSampling(false);

        for (int m = 0; m < (int)bandCount; m++)
        {
            double num3 = (48000.0 / 2.0) / ((double) bandCount);
            plot->addData(num3 * (m + 1.0),(double)m_table.at(m));
            xAxis->setRange(QCPRange(20, num3 * (m + 1.0)));
        }
        break;
    }
    replot();
}

void FrequencyPlot::clearAll(){
    clearPlottables();
    clearItems();
    clearGraphs();
}
void FrequencyPlot::saveScreenshot(){
    QString name = QFileDialog::getSaveFileName(this, tr("Save Screenshot"),
                                                "", tr("PNG screenshot (*.png)"));
    if (!name.isEmpty()){
        if(QFileInfo(name).suffix() != "png")
            name.append(".png");
        savePng(name);
    }
}

