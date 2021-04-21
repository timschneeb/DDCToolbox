#ifndef CURVEFITTINGTHREAD_H
#define CURVEFITTINGTHREAD_H

#include <QThread>
#include <model/CurveFittingOptions.h>

class CurveFittingThread : public QThread
{
public:
    CurveFittingThread(const CurveFittingOptions &_options);
    ~CurveFittingThread();

    bool cancel();

protected:
    virtual void run();

private:
    /* Parameters */
    double* freq = nullptr;
    double* target = nullptr;
    unsigned int array_size;
    uint64_t rng_seed;
    float rng_density_dist;
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

};


#endif // CURVEFITTINGTHREAD_H
