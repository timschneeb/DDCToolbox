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
    const float* freq = nullptr;
    const float* gain = nullptr;
    long rng_seed;
    float rng_density_dist;
};


#endif // CURVEFITTINGTHREAD_H
