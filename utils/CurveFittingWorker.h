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

    static void preprocess(double *freq, double *target, uint& array_size, int fs, bool force_to_oct_grid_conversion, bool* is_nonuniform = nullptr);

signals:
    void mseReceived(float fVar);
    void graphReceived(std::vector<double>);
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
    DoubleRange obc_freq;
    DoubleRange obc_q;
    DoubleRange obc_gain;
    bool force_to_oct_grid_conversion;

    /* Pointers */
    unsigned int *maximaIndex = nullptr;
    unsigned int *minimaIndex = nullptr;
    double *flt_fc = nullptr;
    double *flt_freqList = nullptr;
    double *targetList = nullptr;
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
    static double peakingCostFunctionMap(double *x, void *usd);

};


#endif // CURVEFITTINGWORKER_H
