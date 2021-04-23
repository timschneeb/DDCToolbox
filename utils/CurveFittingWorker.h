#ifndef CURVEFITTINGWORKER_H
#define CURVEFITTINGWORKER_H

#include <QtCore/QObject>
#include <QThread>
#include <array>
#include <model/CurveFittingOptions.h>
#include <model/DeflatedBiquad.h>

class CurveFittingWorker : public QObject
{
    Q_OBJECT
public:
    CurveFittingWorker(const CurveFittingOptions &_options, QObject* parent = nullptr);
    ~CurveFittingWorker();

    QVector<DeflatedBiquad> getResults() const;

signals:
    void historyDataReceived(float fVar, const QVector<float>& currentResult);
    void finished();

public slots:
    void run();

private:
    /* Parameters */
    double* freq = nullptr;
    double* target = nullptr;
    unsigned int array_size;
    uint64_t rng_seed;
    CurveFittingOptions::ProbDensityFunc rng_density_func;
    CurveFittingOptions::AlgorithmType algorithm_type;

    /* Pointers */
    unsigned int *maximaIndex = nullptr;
    unsigned int *minimaIndex = nullptr;
    double *flt_fc = nullptr;
    unsigned int *idx = nullptr;
    double *dFreqDiscontin = nullptr;
    double *dif = nullptr;
    double *flt_peak_g = nullptr;
    double *initialQ = nullptr;
    double *phi = nullptr;
    double *initialAns = nullptr;
    double *low = nullptr;
    double *up = nullptr;
    double *tmpDat = nullptr;

    /* Results */
    QVector<DeflatedBiquad> results;

    /* Callback */
    static void optimizationHistoryCallback(void *hostData, unsigned int n, double *currentResult, double *currentFval);
};


#endif // CURVEFITTINGWORKER_H
