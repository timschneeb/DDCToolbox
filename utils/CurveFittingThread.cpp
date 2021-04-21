#include "CurveFittingThread.h"

extern "C" {
#include "utils/CurveFittingUtils.h"
#include <gradfreeOpt.h>
#include <PeakingFit/linear_interpolation.h>
#include <PeakingFit/peakfinder.h>
}

double callbackFunc(double *sol, unsigned int n){
    // TODO
    Q_UNUSED(sol)
    Q_UNUSED(n)
    return 0;
}

CurveFittingThread::CurveFittingThread(const CurveFittingOptions& _options)
{
    freq =_options.frequencyData();
    target =_options.targetData();
    rng_seed = _options.seed();
    rng_density_dist = _options.probabilityDensityDist();
    array_size = _options.dataCount();
    algorithm_type = _options.algorithmType();

    // Note: do not schedule object for deletion automatically for now
    // connect(this, &CurveFittingThread::finished, this, &QObject::deleteLater);
}

CurveFittingThread::~CurveFittingThread()
{
    free(maximaIndex);
    free(minimaIndex);
    free(flt_fc);
    free(idx);
    free(dFreqDiscontin);
    free(dif);
    free(flt_peak_g);
    free(initialQ);
    free(phi);
    free(initialAns);
    free(low);
    free(up);
    free(tmpDat);
}

bool CurveFittingThread::cancel()
{
    this->terminate();
    return this->wait(2500);
}

void CurveFittingThread::run()
{
    pcg32x2_random_t PRNG;
    pcg32x2_srandom_r(&PRNG, rng_seed, rng_seed >> 2,
                             rng_seed >> 4, rng_seed >> 6);

    unsigned int i, j;
    unsigned int K = 5;
    unsigned int N = 3;
    double fs = 44100.0;

    // Parameter estimation
    unsigned int numMaximas, numMinimas;
    maximaIndex = peakfinder_wrapper(target, array_size, 0.1, 1, &numMaximas);
    minimaIndex = peakfinder_wrapper(target, array_size, 0.1, 0, &numMinimas);
    flt_fc = (double*)malloc((numMaximas + numMinimas) * sizeof(double));
    idx = (unsigned int*)malloc((numMaximas + numMinimas) * sizeof(unsigned int));

    for (i = 0; i < numMaximas; i++)
        flt_fc[i] = freq[maximaIndex[i]];
    for (i = numMaximas; i < (numMaximas + numMinimas); i++)
        flt_fc[i] = freq[minimaIndex[i - numMaximas]];
    sort(flt_fc, (numMaximas + numMinimas), idx);

    double smallestJump = 0.0;
    double lowestFreq2Gen = 200.0;
    double highestFreq2Gen = 14000.0;

    dFreqDiscontin = (double*)malloc((numMaximas + numMinimas) * sizeof(double));
    dif = (double*)malloc((numMaximas + numMinimas) * sizeof(double));
    while (smallestJump <= 20.0)
    {
        derivative(flt_fc, numMaximas + numMinimas, 1, dFreqDiscontin, dif);
        unsigned int smIdx;
        smallestJump = minArray(dFreqDiscontin, numMaximas + numMinimas, &smIdx);
        double newFreq = c_rand(&PRNG) * (highestFreq2Gen - lowestFreq2Gen) + lowestFreq2Gen;
        flt_fc[smIdx] = newFreq;
        sort(flt_fc, (numMaximas + numMinimas), idx);
    }
    unsigned int numBands;
    if (flt_fc[0] > 80.0)
    {
        numBands = numMaximas + numMinimas + 2;
        double *tmp = (double*)malloc(numBands * sizeof(double));
        memcpy(tmp + 2, flt_fc, (numMaximas + numMinimas) * sizeof(double));
        tmp[0] = 20.0; tmp[1] = 60.0;
        free(flt_fc);
        flt_fc = tmp;
    }
    else if (flt_fc[0] > 40.0)
    {
        numBands = numMaximas + numMinimas + 1;
        double *tmp = (double*)malloc(numBands * sizeof(double));
        memcpy(tmp + 1, flt_fc, (numMaximas + numMinimas) * sizeof(double));
        tmp[0] = 20.0;
        free(flt_fc);
        flt_fc = tmp;
    }
    else
        numBands = numMaximas + numMinimas;
    flt_peak_g = (double*)malloc(numBands * sizeof(double));
    for (i = 0; i < numBands; i++)
        flt_peak_g[i] = npointWndFunction(flt_fc[i], freq, target, array_size);
    double lowFc = 20;
    double upFc = fs / 2 - 1;
    double lowQ = 0.2;
    double upQ = 16;
    double lowGain = target[0];
    double upGain = target[0];
    for (i = 1; i < array_size; i++)
    {
        if (target[i] < lowGain)
            lowGain = target[i];
        if (target[i] > upGain)
            upGain = target[i];
    }
    lowGain -= 5.0;
    upGain += 5.0;
    initialQ = (double*)malloc(numBands * sizeof(double));
    for (i = 0; i < numBands; i++)
    {
        initialQ[i] = fabs(randn_pcg32x2(&PRNG) * (5 - 0.7) + 0.7);
        flt_fc[i] = log10(flt_fc[i]);
    }

    // Grid generation
    phi = (double*)malloc(array_size * sizeof(double));
    for (i = 0; i < array_size; i++)
    {
        double term1 = sin(M_PI * freq[i] / fs);
        phi[i] = 4.0 * term1 * term1;
    }
    unsigned int dim = numBands * 3;

    // Local minima avoidance (DE)
    double initialLowGain = -1.5;
    double initialUpGain = 1.5;
    double initialLowQ = -0.5;
    double initialUpQ = 0.5;
    double initialLowFc = -log10(2);
    double initialUpFc = log10(2);
    initialAns = (double*)malloc(K * N * dim * sizeof(double));
    for (i = 0; i < K * N; i++)
    {
        for (j = 0; j < numBands; j++)
        {
            initialAns[i * dim + j] = flt_fc[j] + c_rand(&PRNG) * (initialUpFc - initialLowFc) + initialLowFc;
            initialAns[i * dim + numBands + j] = initialQ[j] + c_rand(&PRNG) * (initialUpQ - initialLowQ) + initialLowQ;
            initialAns[i * dim + numBands * 2 + j] = flt_peak_g[j] + c_rand(&PRNG) * (initialUpGain - initialLowGain) + initialLowGain;
        }
    }

    // Boundary constraint
    low = (double*)malloc(dim * sizeof(double));
    up = (double*)malloc(dim * sizeof(double));
    for (j = 0; j < numBands; j++)
    {
        low[j] = log10(lowFc); low[numBands + j] = lowQ; low[numBands * 2 + j] = lowGain;
        up[j] = log10(upFc); up[numBands + j] = upQ; up[numBands * 2 + j] = upGain;
    }

    tmpDat = (double*)malloc(array_size * sizeof(double));
    optUserdata userdat;
    userdat.fs = fs;
    userdat.numBands = numBands;
    userdat.phi = phi;
    userdat.target = target;
    userdat.tmp = tmpDat;
    userdat.gridSize = array_size;
    void *userdataPtr = (void*)&userdat;

    switch(algorithm_type){
    case CurveFittingOptions::AT_DIFF_EVOLUTION: {
        double *gbestDE = (double*)malloc(dim * sizeof(double));
        double gmin = differentialEvolution(peakingCostFunctionMap, userdataPtr, initialAns, K, N, dim, low, up, 10, gbestDE, &PRNG);

        qDebug("%1.14lf\n", gmin);
        for (i = 0; i < dim; i++)
            qDebug("%1.14lf,", gbestDE[i]);

        free(gbestDE);
        break;
    }
    case CurveFittingOptions::AT_FMINSEARCHBND: {
        double *gbestfminsearch = (double*)malloc(dim * sizeof(double));
        double fval = fminsearchbnd(peakingCostFunctionMap, userdataPtr, initialAns, low, up, dim, 1e-8, 1e-8, 10, gbestfminsearch);

        qDebug("%1.14lf\n", fval);
        for (i = 0; i < dim; i++)
            qDebug("%1.14lf,", gbestfminsearch[i]);

        free(gbestfminsearch);
        break;
    }
    }

    // Report success
    this->exit(0);
}
