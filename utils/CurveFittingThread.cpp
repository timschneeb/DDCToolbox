#include "CurveFittingThread.h"

extern "C" {
#include <gradfreeOpt.h>
}

#define INT64_16(pack,n) (pack >> n * 16)

double callbackFunc(double *sol, unsigned int n){
    // TODO
    Q_UNUSED(sol)
    Q_UNUSED(n)
    return 0;
}

CurveFittingThread::CurveFittingThread(const CurveFittingOptions& _options)
{
    freq =_options.frequencyData();
    target =_options.gainData();
    rng_seed = _options.seed();
    rng_density_dist = _options.probabilityDensityDist();
    arraySize = _options.dataCount();

    // Note: do not schedule object for deletion automatically for now
    // connect(this, &CurveFittingThread::finished, this, &QObject::deleteLater);
}

CurveFittingThread::~CurveFittingThread()
{
    // TODO: Clean up
}

bool CurveFittingThread::cancel()
{
    this->terminate();
    return this->wait(2500);
}

void CurveFittingThread::run()
{
    // TODO: Thread has been externally launched, initialize and get to work here
    pcg32x2_random_t PRNG;
    pcg32x2_srandom_r(&PRNG, rng_seed, rng_seed >> 2,
                             rng_seed >> 4, rng_seed >> 6);

    // Report success
    this->exit(0);
}
