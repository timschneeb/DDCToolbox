#include "CurveFittingWorker.h"

extern "C" {
#include <gradfreeOpt.h>
#include <PeakingFit/linear_interpolation.h>
#include <PeakingFit/peakfinder.h>
}

#include "utils/CurveFittingUtils.h"

typedef struct
{
    double fs;
    double *phi;
    unsigned int gridSize;
    double *target;
    unsigned int numBands;
    double *tmp;
    void* user_data;
} optUserdata;

CurveFittingWorker::CurveFittingWorker(const CurveFittingOptions& _options, QObject* parent) : QObject(parent)
{
    freq =_options.frequencyData();
    target =_options.targetData();
    rng_seed = _options.seed();
    rng_density_func = _options.probabilityDensityFunc();
    array_size = _options.dataCount();
    algorithm_type = _options.algorithmType();
}

CurveFittingWorker::~CurveFittingWorker()
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

void CurveFittingWorker::optimizationHistoryCallback(void *hostData, unsigned int n, double *currentResult, double *currentFval)
{
    Q_UNUSED(n);
    Q_UNUSED(currentResult);
    CurveFittingWorker *worker = (CurveFittingWorker*)hostData;
    emit worker->mseReceived(*currentFval);
}

double CurveFittingWorker::peakingCostFunctionMap(double *x, void *usd)
{
    optUserdata *userdata = (optUserdata*)usd;
    CurveFittingWorker *worker = (CurveFittingWorker*)userdata->user_data;
    double *fc = x;
    double *Q = x + userdata->numBands;
    double *gain = x + userdata->numBands * 2;
    double b0, b1, b2, a1, a2;
    memset(userdata->tmp, 0, userdata->gridSize * sizeof(double));

    for (unsigned int i = 0; i < userdata->numBands; i++)
    {
        validatePeaking(gain[i], fc[i], Q[i], userdata->fs, &b0, &b1, &b2, &a1, &a2);
        validateMagCal(b0, b1, b2, a1, a2, userdata->phi, userdata->gridSize, userdata->fs, userdata->tmp);
        //printf("Band: %d, %1.14lf, %1.14lf, %1.14lf\n", i + 1, fc[i], Q[i], gain[i]);
    }

    /* Warning: Handing allocated pointer over to a new owner via a queued signal for performance reasons
     * TODO: Research whether this could trigger memory leaks and keep a reference for later deletion here as well */
    emit worker->graphReceived(std::vector(userdata->tmp, userdata->tmp + userdata->gridSize));

    double meanAcc = 0.0;
    for (unsigned int i = 0; i < userdata->gridSize; i++)
    {
        double error = userdata->tmp[i] - userdata->target[i];
        meanAcc += error * error;
    }
    meanAcc = meanAcc / (double)userdata->gridSize;
    return meanAcc;
}

void CurveFittingWorker::run()
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
    userdat.user_data = this;
    void *userdataPtr = (void*)&userdat;

    // Select probability distribution function
    double(*pdf1)(pcg32x2_random_t*) = randn_pcg32x2;
    switch(rng_density_func){
    case CurveFittingOptions::PDF_RANDN_PCG32X2:
        pdf1 = randn_pcg32x2;
        break;
    case CurveFittingOptions::PDF_RAND_TRI_PCG32X2:
        pdf1 = rand_tri_pcg32x2;
        break;
    case CurveFittingOptions::PDF_RAND_HANN:
        pdf1 = rand_hann;
        break;
    }

    // Create optimization history callback
    void *hist_userdata = (void*)this;
    void(*optStatus)(void*, unsigned int, double*, double*) = optimizationHistoryCallback;

    // Calculate
    double *output = (double*)malloc(dim * sizeof(double));
    switch(algorithm_type){
    case CurveFittingOptions::AT_DIFF_EVOLUTION: {
        double gmin = differentialEvolution(peakingCostFunctionMap, userdataPtr, initialAns, K, N, dim, low, up, 10000, output, &PRNG, pdf1, optStatus, hist_userdata);
        qDebug("CurveFittingThread: gmin=%lf", gmin);
        break;
    }
    case CurveFittingOptions::AT_FMINSEARCHBND: {
        double fval = fminsearchbnd(peakingCostFunctionMap, userdataPtr, initialAns, low, up, dim, 1e-8, 1e-8, 10000, output, optStatus, hist_userdata);
        qDebug("CurveFittingThread: lval=%lf", fval);
        break;
    }
    case CurveFittingOptions::AT_FLOWERPOLLINATION: {
        double gmin2 = flowerPollination(peakingCostFunctionMap, userdataPtr, initialAns, low, up, dim, K * N, 0.1, 0.05, 2000, output, &PRNG, pdf1, optStatus, hist_userdata);
        qDebug("CurveFittingThread: gmin2=%lf", gmin2);
        break;
    }
    }

    // Serialize results
    double *fc = output;
    double *q = output + numBands;
    double *gain = output + numBands * 2;
    for(uint i = 0; i < numBands; i++)
    {
        results.append(DeflatedBiquad(FilterType::PEAKING, pow(10, fc[i]), q[i], gain[i]));
    }
    free(output);

    emit finished();
}

QVector<DeflatedBiquad> CurveFittingWorker::getResults() const
{
    return results;
}
