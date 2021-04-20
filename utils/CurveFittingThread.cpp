#include "CurveFittingThread.h"

double callbackFunc(double *sol, unsigned int n){
    // TODO
    Q_UNUSED(sol)
    Q_UNUSED(n)
    return 0;
}

CurveFittingThread::CurveFittingThread(const CurveFittingOptions& _options)
{
    freq =_options.frequencyData();
    gain =_options.gainData();
    rng_seed = _options.seed();
    rng_density_dist = _options.probabilityDensityDist();

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


    // Report success
    this->exit(0);
}
