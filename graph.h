#ifndef GRAPH_H
#define GRAPH_H
#include <QObject>
#include <cmath>
#include <QTableWidget>
#include "ddccontext.h"
#include "filtertypes.h"
#include "qcustomplot.h"

class Graph
{
public:
    static void drawMagnitudeResponse(QCustomPlot* graph,std::vector<float> table,int bandCount,QTableWidget* tb,bool pointsVisible){
        if (table.size()<=0)
            return;

        float max = 24.0f;
        float min = -24.0f;

        for (size_t i = 0; i < (size_t)bandCount; i++){
            if (table.at(i) > max)
                max = table.at(i);
            if (table.at(i) < min)
                min = table.at(i);
        }

        graph->yAxis->setRange(QCPRange(min, max));
        QCPGraph *plot = graph->addGraph();
        plot->setAdaptiveSampling(false);

        QCPGraph *points = graph->addGraph();
        points->setAdaptiveSampling(false);
        points->removeFromLegend();
        points->setLineStyle(QCPGraph::lsNone);
        points->setScatterStyle(QCPScatterStyle::ssDot);
        points->setPen(QPen(QBrush(Qt::red),3));
        QCPGraph *sel_points = graph->addGraph();
        sel_points->setAdaptiveSampling(false);
        sel_points->removeFromLegend();
        sel_points->setLineStyle(QCPGraph::lsNone);
        sel_points->setScatterStyle(QCPScatterStyle::ssDot);
        sel_points->setPen(QPen(QBrush(Qt::darkRed),3));

        for (size_t m = 0; m < (size_t)bandCount; m++)
        {
            double num3 = (44100.0 / 2.0) / ((double) bandCount);
            plot->addData(num3 * (m + 1.0),(double)table.at(m));
            graph->xAxis->setRange(QCPRange(20, num3 * (m + 1.0)));
        }

        QCPItemTracer *tracer = new QCPItemTracer(graph);
        tracer->setGraph(plot);
        tracer->setStyle(QCPItemTracer::TracerStyle::tsNone);

        //Draw all non-selected points only if enabled
        if(pointsVisible){
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
        //Draw selected points always
        for (auto item : tb->selectedItems()){
            if(stringToType(tb->item(item->row(),0)->text()) == biquad::UNITY_GAIN)
                continue;

            int freq = tb->item(item->row(),1)->data(Qt::DisplayRole).toInt();
            tracer->setGraphKey(freq);
            tracer->updatePosition();
            sel_points->addData(freq,tracer->position->value());
        }


        graph->replot();
    }
    static void drawGroupDelayGraph(QCustomPlot* graph,std::vector<float> table,int bandCount,QTableWidget* tb,bool pointsVisible){
        if (table.size()<=0)
            return;

        float max_gd = 12.0f;
        float min_gd = -12.0f;

        for (size_t i = 0; i < (size_t)table.size(); i++){
            if (table.at(i) > max_gd)
                max_gd = table.at(i);
            if (table.at(i) < min_gd)
                min_gd = table.at(i);
        }
        if(std::isinf(min_gd))min_gd=-12.0;
        if(std::isinf(max_gd))max_gd=12.0;

        graph->yAxis->setRange(QCPRange(min_gd, max_gd));
        QCPGraph *plot_gd = graph->addGraph();
        plot_gd->setAdaptiveSampling(false);

        for (size_t m = 0; m < (size_t)bandCount; m++)
        {
            double num3 = (44100.0 / 2.0) / ((double) bandCount);
            plot_gd->addData(num3 * (m + 1.0),(double)table.at(m));
            graph->xAxis->setRange(QCPRange(20, num3 * (m + 1.0)));
        }

        QCPGraph *points = graph->addGraph();
        points->setAdaptiveSampling(false);
        points->removeFromLegend();
        points->setLineStyle(QCPGraph::lsNone);
        points->setScatterStyle(QCPScatterStyle::ssDot);
        points->setPen(QPen(QBrush(Qt::red),3));
        QCPGraph *sel_points = graph->addGraph();
        sel_points->setAdaptiveSampling(false);
        sel_points->removeFromLegend();
        sel_points->setLineStyle(QCPGraph::lsNone);
        sel_points->setScatterStyle(QCPScatterStyle::ssDot);
        sel_points->setPen(QPen(QBrush(Qt::darkRed),3));

        for (size_t m = 0; m < (size_t)bandCount; m++)
        {
            double num3 = (44100.0 / 2.0) / ((double) bandCount);
            plot_gd->addData(num3 * (m + 1.0),(double)table.at(m));
            graph->xAxis->setRange(QCPRange(20, num3 * (m + 1.0)));
        }

        QCPItemTracer *tracer = new QCPItemTracer(graph);
        tracer->setGraph(plot_gd);
        tracer->setStyle(QCPItemTracer::TracerStyle::tsNone);

        if(pointsVisible){
            for (int i = 0; i < tb->rowCount(); i++) {
                if(stringToType(tb->item(i,0)->text()) == biquad::UNITY_GAIN)
                    continue;

                int freq = tb->item(i,1)->data(Qt::DisplayRole).toInt();
                tracer->setGraphKey(freq);
                tracer->updatePosition();
                points->addData(freq,tracer->position->value());
            }
        }
        //Draw selected points always
        for (auto item : tb->selectedItems()){
            if(stringToType(tb->item(item->row(),0)->text()) == biquad::UNITY_GAIN)
                continue;

            int freq = tb->item(item->row(),1)->data(Qt::DisplayRole).toInt();
            tracer->setGraphKey(freq);
            tracer->updatePosition();
            sel_points->addData(freq,tracer->position->value());
        }
        graph->replot();
    }
};

#endif // GRAPH_H
