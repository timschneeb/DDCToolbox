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
    double* freq = nullptr;
    double* target = nullptr;
    int arraySize;
    uint64_t rng_seed;
    float rng_density_dist;
};


#endif // CURVEFITTINGTHREAD_H
