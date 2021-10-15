#ifndef CURVEFITTINGUTILS_H
#define CURVEFITTINGUTILS_H

#include <math.h>
#include <float.h>
#include <string.h>
#include <QDebug>

extern "C" {
#include <peakfinder.h>
#include <interpolation2.h>
#include <gradfreeOpt.h>
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

unsigned int getAuditoryBandLen(const int xLen, double avgBW)
{
    unsigned int a = 3;
    double step = avgBW;
    unsigned int cnt = 0;
    while (a + step - 1.0 < xLen)
    {
        a = a + (unsigned int)ceil(round(step) * 0.5);
        step = step * avgBW;
        cnt = cnt + 1;
    }
    return cnt;
}

void initInterpolationList(unsigned int *indexList, double *levels, double avgBW, unsigned int fcLen, unsigned int xLim)
{
    unsigned int a = 3;
    double step = avgBW;
    levels[0] = (1.0 - 1.0) / (double)xLim;
    levels[1] = (2.0 - 1.0) / (double)xLim;
    for (uint i = 0; i < fcLen; i++)
    {
        unsigned int stepp = (unsigned int)round(step);
        indexList[(i << 1) + 0] = a - 1;
        indexList[(i << 1) + 1] = a + stepp - 1;
        levels[i + 2] = ((double)a + ((double)stepp - 1.0) * 0.5 - 1.0) / (double)xLim;
        a = a + (unsigned int)ceil(((double)stepp) * 0.5);
        step = step * avgBW;
    }
    indexList[(fcLen << 1) + 0] = a - 1;
    indexList[(fcLen << 1) + 1] = xLim;
    levels[2 + fcLen] = ((a + xLim) * 0.5 - 1.0) / (double)xLim;
    levels[2 + fcLen + 1] = (xLim - 1.0) / (double)xLim;
    if (levels[2 + fcLen + 1] == levels[2 + fcLen])
        levels[2 + fcLen] = (levels[2 + fcLen - 1] + levels[2 + fcLen + 1]) * 0.5;
}

void validatePeaking(double gain, double fc, double Q, double fs_tf, double *b0, double *b1, double *b2, double *a1, double *a2)
{
    double A = pow(10.0, gain / 40.0);
    double w0 = 2.0 * M_PI * pow(10.0, fc) / fs_tf;
    double sn = sin(w0);
    double cn = cos(w0);
    double alpha = sn / (2.0 * Q);

    double a0_pk = 1.0 + alpha / A;
    *a1 = (-2.0 * cn) / a0_pk;
    *a2 = (1.0 - alpha / A) / a0_pk;

    *b0 = (1.0 + alpha * A) / a0_pk;
    *b1 = (-2.0 * cn) / a0_pk;
    *b2 = (1.0 - alpha * A) / a0_pk;
}

void validateMagCal(double b0, double b1, double b2, double a1, double a2, double *phi, int len, double fs_tf, double *out)
{
    Q_UNUSED(fs_tf);

    for (int i = 0; i < len; i++)
    {
        double termSqr1 = b0 + b1 + b2;
        double termSqr2 = 1.0 + a1 + a2;
        double term1 = (termSqr1 * termSqr1) + ((b0 * b2 * phi[i]) - (b1 * (b0 + b2) + (4.0 * b0 * b2))) * phi[i];
        double term2 = (termSqr2 * termSqr2) + ((a2 * phi[i]) - (a1 * (1.0 + a2) + (4.0 * a2))) * phi[i];
        if (term1 < DBL_EPSILON)
            term1 = DBL_EPSILON;
        if (term2 < DBL_EPSILON)
            term2 = DBL_EPSILON;
        double eq_op = 10.0 * (log10(term1) - log10(term2));
        out[i] += eq_op;
    }
}

unsigned int* peakfinder_wrapper(double *x, unsigned int n, double sel, unsigned int extrema, unsigned int *numPeaks)
{
    unsigned int *peakInds = (unsigned int*)malloc(n * sizeof(unsigned int));
    *numPeaks = peakfinder(n, x, sel, extrema, peakInds); // 1u = maxima, 0u = minima
    return peakInds;
}

void diff(double *y, double *f, unsigned int sz)
{
    --sz;
    for (unsigned int i = 0; i < sz; i++)
        f[i] = y[i + 1] - y[i];
}

void derivative(double *x, unsigned int n, unsigned int NorderDerivative, double *dx, double *dif)
{
    // DERIVATIVE Compute derivative while preserving dimensions.
    memcpy(dx, x, n * sizeof(double));
    for (unsigned int i = 0; i < NorderDerivative; i++)
    {
        diff(dx, dif, n);
        dx[0] = dif[0];
        for (unsigned int j = 1; j < n - 1; j++)
        {
            dx[j] = (dif[j] + dif[j - 1]) * 0.5;
        }
        dx[n - 1] = dif[n - 2];
    }
}


void smoothSpectral(int gdType, double *flt_freqList, double *target, unsigned int ud_gridSize, double avgBW, double fs, unsigned int *outGridSize, double **out1, double **out2)
{
    unsigned int i;
    unsigned int detailLinearGridLen;
    double *spectrum;
    double *linGridFreq;
    if (!gdType)
    {
        detailLinearGridLen = ud_gridSize;
        spectrum = (double*)malloc(detailLinearGridLen * sizeof(double));
        linGridFreq = (double*)malloc(detailLinearGridLen * sizeof(double));
        for (i = 0; i < detailLinearGridLen; i++)
        {
            spectrum[i] = target[i];
            linGridFreq[i] = i / ((double)detailLinearGridLen) * (fs / 2);
        }
    }
    else
    {
        detailLinearGridLen = fmax(ud_gridSize, 8192); // Larger the value could be take care large, especially for uniform grid
        spectrum = (double*)malloc(detailLinearGridLen * sizeof(double));
        linGridFreq = (double*)malloc(detailLinearGridLen * sizeof(double));
        for (i = 0; i < detailLinearGridLen; i++)
        {
            double fl = i / ((double)detailLinearGridLen) * (fs / 2);
            spectrum[i] = linearInterpolationNoExtrapolate(fl, flt_freqList, target, ud_gridSize);
            linGridFreq[i] = fl;
        }
    }
    // Init octave grid shrinker
    unsigned int arrayLen = detailLinearGridLen;
    unsigned int fcLen = getAuditoryBandLen(arrayLen, avgBW);
    unsigned int idxLen = fcLen + 1;
    unsigned int *indexList = (unsigned int*)malloc((idxLen << 1) * sizeof(unsigned int));
    double *levels = (double*)malloc((idxLen + 3) * sizeof(double));
    double *multiplicationPrecompute = (double*)malloc(idxLen * sizeof(double));
    size_t virtualStructSize = sizeof(unsigned int) + sizeof(double) + sizeof(unsigned int) + (idxLen << 1) * sizeof(unsigned int) + idxLen * sizeof(double) + (idxLen + 3) * sizeof(double);
    double *shrinkedAxis = (double*)malloc((idxLen + 3) * sizeof(double));
    double reciprocal = 1.0 / arrayLen;
    initInterpolationList(indexList, levels, avgBW, fcLen, arrayLen);
    for (unsigned int i = 0; i < idxLen; i++)
        multiplicationPrecompute[i] = 1.0 / (indexList[(i << 1) + 1] - indexList[(i << 1) + 0]);
    // Do actual axis conversion
    shrinkedAxis[0] = spectrum[0];
    shrinkedAxis[1] = spectrum[1];
    double sum;
    for (i = 0; i < idxLen; i++)
    {
        sum = 0.0;
        for (unsigned int j = indexList[(i << 1) + 0]; j < indexList[(i << 1) + 1]; j++)
            sum += spectrum[j];
        shrinkedAxis[2 + i] = sum * multiplicationPrecompute[i];
    }
    shrinkedAxis[(idxLen + 3) - 1] = shrinkedAxis[(idxLen + 3) - 2];
    double *ascendingIdx = (double*)malloc(arrayLen * sizeof(double));
    for (i = 0; i < arrayLen; i++)
        ascendingIdx[i] = i;
    unsigned int hz18 = 0;
    for (i = 0; i < idxLen + 3; i++)
    {
        double freqIdxUR = levels[hz18 + i] * arrayLen;
        double realFreq = linearInterpolationNoExtrapolate(freqIdxUR, ascendingIdx, linGridFreq, arrayLen);
        hz18 = i;
        if (realFreq >= 18.0)
            break;
    }
    double *newflt_freqList = (double*)malloc((idxLen + 3 - hz18) * sizeof(double));
    double *newTarget = (double*)malloc((idxLen + 3 - hz18) * sizeof(double));
    for (i = 0; i < idxLen + 3 - hz18; i++)
    {
        double freqIdxUR = levels[hz18 + i] * arrayLen;
        newflt_freqList[i] = linearInterpolationNoExtrapolate(freqIdxUR, ascendingIdx, linGridFreq, arrayLen);
        newTarget[i] = shrinkedAxis[hz18 + i];
    }
    free(shrinkedAxis);
    free(ascendingIdx);
    free(linGridFreq);
    *outGridSize = idxLen + 3 - hz18;
    *out1 = newflt_freqList;
    *out2 = newTarget;
    free(levels);
    free(multiplicationPrecompute);
    free(indexList);
    free(spectrum);
}

#endif // CURVEFITTINGUTILS_H
