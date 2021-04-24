#ifndef CURVEFITTINGUTILS_H
#define CURVEFITTINGUTILS_H

#include <math.h>
#include <float.h>
#include <string.h>
#include <QDebug>

#include <PeakingFit/peakfinder.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

#endif // CURVEFITTINGUTILS_H
