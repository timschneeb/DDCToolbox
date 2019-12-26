#include "frequencyplot.h"

FrequencyPlot::FrequencyPlot(QFrame* frame)
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
    case PlotType::group_delay:
        yLabel = tr("Delay (Samples)");
        yRange = QCPRange(-12,12);
        break;
    case PlotType::phase_response:
        yLabel = tr("Phase (deg)");
        yRange = QCPRange(-24,24);
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
        //int y = this->yAxis->pixelToCoord(event->pos().y());
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
void FrequencyPlot::updatePoints(QCPGraph* plot,QTableWidget* tb,bool allMarkerPointsVisible){
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
    if(allMarkerPointsVisible){
        for (int i = 0; i < tb->rowCount(); i++) {
            if(stringToType(tb->item(i,0)->text()) == biquad::UNITY_GAIN)
                continue;
            if(tb->selectedItems().contains(tb->item(i,0)))
                continue;

            int freq = tb->item(i,1)->data(Qt::DisplayRole).toInt();
            tracer->setGraphKey(freq);
            tracer->updatePosition();
            points->addData(freq,tracer->position->value());
        }
    }
    //Draw selected points
    for (auto item : tb->selectedItems()){
        if(stringToType(tb->item(item->row(),0)->text()) == biquad::UNITY_GAIN)
            continue;

        int freq = tb->item(item->row(),1)->data(Qt::DisplayRole).toInt();
        tracer->setGraphKey(freq);
        tracer->updatePosition();
        sel_points->addData(freq,tracer->position->value());
    }
    replot();
}
void FrequencyPlot::updatePlot(std::vector<float> table,int bandCount){
    m_table = table;
    if (m_table.size()<=0)
        return;

    QCPGraph *plot;
    float max = 24.0f;
    float min = -24.0f;

    switch (m_type) {
    case PlotType::phase_response:
        for (size_t i = 0; i < (size_t)bandCount; i++){
            if (m_table.at(i) > max)
                max = m_table.at(i);
            if (m_table.at(i) < min)
                min = m_table.at(i);
        }

        yAxis->setRange(QCPRange(min, max));
        plot = addGraph();
        plot->setAdaptiveSampling(false);

        for (size_t m = 0; m < (size_t)bandCount; m++)
        {
            double num3 = (44100.0 / 2.0) / ((double) bandCount);
            plot->addData(num3 * (m + 1.0),(double)m_table.at(m));
            xAxis->setRange(QCPRange(20, num3 * (m + 1.0)));
        }
        break;
    case PlotType::magnitude:
        for (size_t i = 0; i < (size_t)bandCount; i++){
            if (m_table.at(i) > max)
                max = m_table.at(i);
            if (m_table.at(i) < min)
                min = m_table.at(i);
        }

        yAxis->setRange(QCPRange(min, max));
        plot = addGraph();
        plot->setAdaptiveSampling(false);

        for (size_t m = 0; m < (size_t)bandCount; m++)
        {
            double num3 = (44100.0 / 2.0) / ((double) bandCount);
            plot->addData(num3 * (m + 1.0),(double)m_table.at(m));
            xAxis->setRange(QCPRange(20, num3 * (m + 1.0)));
        }
        break;
    case PlotType::group_delay:
        max = 12.0f;
        min = -12.0f;

        for (size_t i = 0; i < (size_t)m_table.size(); i++){
            if (m_table.at(i) > max)
                max = m_table.at(i);
            if (m_table.at(i) < min)
                min = m_table.at(i);
        }
        if(std::isinf(min))min=-12.0;
        if(std::isinf(max))max=12.0;

        yAxis->setRange(QCPRange(min, max));
        plot = addGraph();
        plot->setAdaptiveSampling(false);

        for (size_t m = 0; m < (size_t)bandCount; m++)
        {
            double num3 = (44100.0 / 2.0) / ((double) bandCount);
            plot->addData(num3 * (m + 1.0),(double)m_table.at(m));
            xAxis->setRange(QCPRange(20, num3 * (m + 1.0)));
        }
        break;
    }
    replot();
}
void FrequencyPlot::updatePoints(QTableWidget* tb,bool allMarkerPointsVisible){
    QCPGraph *plot = graph(0);
    if(plot != nullptr)updatePoints(plot,tb,allMarkerPointsVisible);
}
void FrequencyPlot::clear(){
    clearPlottables();
    clearItems();
    clearGraphs();
}
void FrequencyPlot::saveScreenshot(){
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Screenshot"), "", tr("PNG screenshot (*.png)"));
    if (fileName != "" && fileName != nullptr){
        QFileInfo fi(fileName);
        QString ext = fi.suffix();
        if(ext!="png")fileName.append(".png");
        savePng(fileName);
    }
}

