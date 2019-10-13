#include "biquad.h"
#include <cmath>
#include <list>

biquad::biquad()
{
    internalBiquadCoeffs[0] = 0.0;
    internalBiquadCoeffs[1] = 0.0;
    internalBiquadCoeffs[2] = 0.0;
    internalBiquadCoeffs[3] = 0.0;
    a0 = 0.0;
}

void biquad::RefreshFilter(Type type,double dbGain, double centreFreq, double fs, double dBandwidthOrQOrS, bool isBandwidthOrS)
{
    m_dFilterType = type;
    m_dFilterGain = dbGain;
    m_dFilterFreq = centreFreq;
    m_dFilterBQ = dBandwidthOrQOrS;
    m_isBandwidthOrS = isBandwidthOrS;
    double d;
    if (type == PEAKING || type == LOW_SHELF || type == HIGH_SHELF)
        d = pow(10.0, dbGain / 40.0);
    else
        d = pow(10.0, dbGain / 20.0);
    double a = (6.2831853071795862 * centreFreq) / fs; //omega
    double num3 = sin(a); //sn
    double cs = cos(a);

    double alpha;
    if (!isBandwidthOrS) // Q
        alpha = num3 / (2 * dBandwidthOrQOrS);
    else if (type == LOW_SHELF || type == HIGH_SHELF) // S
        alpha = num3 / 2 * sqrt((d + 1 / d) * (1 / dBandwidthOrQOrS - 1) + 2);
    else // BW
        alpha = num3 * sinh(M_LN2 / 2 * dBandwidthOrQOrS * a / num3);

    double beta = 2 * sqrt(d) * alpha;
    double B0, B1, B2, A0, A1, A2;

    switch (type)
    {
    case LOW_PASS:
        B0 = (1.0 - cs) / 2.0;
        B1 = 1.0 - cs;
        B2 = (1.0 - cs) / 2.0;
        A0 = 1.0 + alpha;
        A1 = -2.0 * cs;
        A2 = 1.0 - alpha;
        break;
    case HIGH_PASS:
        B0 = (1.0 + cs) / 2.0;
        B1 = -(1.0 + cs);
        B2 = (1.0 + cs) / 2.0;
        A0 = 1.0 + alpha;
        A1 = -2.0 * cs;
        A2 = 1.0 - alpha;
        break;
    case BAND_PASS:
        B0 = alpha;
        B1 = 0.0;
        B2 = -alpha;
        A0 = 1.0 + alpha;
        A1 = -2.0 * cs;
        A2 = 1.0 - alpha;
        break;
    case NOTCH:
        B0 = 1.0;
        B1 = -2.0 * cs;
        B2 = 1.0;
        A0 = 1.0 + alpha;
        A1 = -2.0 * cs;
        A2 = 1.0 - alpha;
        break;
    case ALL_PASS:
        B0 = 1.0 - alpha;
        B1 = -2.0 * cs;
        B2 = 1.0 + alpha;
        A0 = 1.0 + alpha;
        A1 = -2.0 * cs;
        A2 = 1.0 - alpha;
        break;
    case PEAKING:
        B0 = 1.0 + (alpha * d);
        B1 = -2.0 * cs;
        B2 = 1.0 - (alpha * d);
        A0 = 1.0 + (alpha / d);
        A1 = -2.0 * cs;
        A2 = 1.0 - (alpha / d);
        break;
    case LOW_SHELF:
        B0 = d * ((d + 1.0) - (d - 1.0) * cs + beta);
        B1 = 2.0 * d * ((d - 1.0) - (d + 1.0) * cs);
        B2 = d * ((d + 1.0) - (d - 1.0) * cs - beta);
        A0 = (d + 1.0) + (d - 1.0) * cs + beta;
        A1 = -2.0 * ((d - 1.0) + (d + 1.0) * cs);
        A2 = (d + 1.0) + (d - 1.0) * cs - beta;
        break;
    case HIGH_SHELF:
        B0 = d * ((d + 1.0) + (d - 1.0) * cs + beta);
        B1 = -2.0 * d * ((d - 1.0) + (d + 1.0) * cs);
        B2 = d * ((d + 1.0) + (d - 1.0) * cs - beta);
        A0 = (d + 1.0) - (d - 1.0) * cs + beta;
        A1 = 2.0 * ((d - 1.0) - (d + 1.0) * cs);
        A2 = (d + 1.0) - (d - 1.0) * cs - beta;
        break;
    }

    a0 = B0 / A0;
    internalBiquadCoeffs[0] = B1 / A0;
    internalBiquadCoeffs[1] = B2 / A0;
    internalBiquadCoeffs[2] = -A1 / A0;
    internalBiquadCoeffs[3] = -A2 / A0;
}
std::list<double> biquad::ExportCoeffs(Type type,double dbGain, double centreFreq, double fs, double dBandwidthOrQOrS, bool isBandwidthOrS)
{
    if (centreFreq <= 2.2204460492503131e-016 || fs <= 2.2204460492503131e-016){
        std::list<double> nulllist;
        return nulllist;
    }

    double d;
    if (type == PEAKING || type == LOW_SHELF || type == HIGH_SHELF)
        d = pow(10.0, dbGain / 40.0);
    else
        d = pow(10.0, dbGain / 20.0);
    double a = (6.2831853071795862 * centreFreq) / fs; //omega
    double num3 = sin(a); //sn
    double cs = cos(a);

    double alpha;
    if (!isBandwidthOrS) // Q
        alpha = num3 / (2 * dBandwidthOrQOrS);
    else if (type == LOW_SHELF || type == HIGH_SHELF) // S
        alpha = num3 / 2 * sqrt((d + 1 / d) * (1 / dBandwidthOrQOrS - 1) + 2);
    else // BW
        alpha = num3 * sinh(M_LN2 / 2 * dBandwidthOrQOrS * a / num3);

    double beta = 2 * sqrt(d) * alpha;
    double B0, B1, B2, A0, A1, A2;

    switch (type)
    {
    case LOW_PASS:
        B0 = (1.0 - cs) / 2.0;
        B1 = 1.0 - cs;
        B2 = (1.0 - cs) / 2.0;
        A0 = 1.0 + alpha;
        A1 = -2.0 * cs;
        A2 = 1.0 - alpha;
        break;
    case HIGH_PASS:
        B0 = (1.0 + cs) / 2.0;
        B1 = -(1.0 + cs);
        B2 = (1.0 + cs) / 2.0;
        A0 = 1.0 + alpha;
        A1 = -2.0 * cs;
        A2 = 1.0 - alpha;
        break;
    case BAND_PASS:
        B0 = alpha;
        B1 = 0.0;
        B2 = -alpha;
        A0 = 1.0 + alpha;
        A1 = -2.0 * cs;
        A2 = 1.0 - alpha;
        break;
    case NOTCH:
        B0 = 1.0;
        B1 = -2.0 * cs;
        B2 = 1.0;
        A0 = 1.0 + alpha;
        A1 = -2.0 * cs;
        A2 = 1.0 - alpha;
        break;
    case ALL_PASS:
        B0 = 1.0 - alpha;
        B1 = -2.0 * cs;
        B2 = 1.0 + alpha;
        A0 = 1.0 + alpha;
        A1 = -2.0 * cs;
        A2 = 1.0 - alpha;
        break;
    case PEAKING:
        B0 = 1.0 + (alpha * d);
        B1 = -2.0 * cs;
        B2 = 1.0 - (alpha * d);
        A0 = 1.0 + (alpha / d);
        A1 = -2.0 * cs;
        A2 = 1.0 - (alpha / d);
        break;
    case LOW_SHELF:
        B0 = d * ((d + 1.0) - (d - 1.0) * cs + beta);
        B1 = 2.0 * d * ((d - 1.0) - (d + 1.0) * cs);
        B2 = d * ((d + 1.0) - (d - 1.0) * cs - beta);
        A0 = (d + 1.0) + (d - 1.0) * cs + beta;
        A1 = -2.0 * ((d - 1.0) + (d + 1.0) * cs);
        A2 = (d + 1.0) + (d - 1.0) * cs - beta;
        break;
    case HIGH_SHELF:
        B0 = d * ((d + 1.0) + (d - 1.0) * cs + beta);
        B1 = -2.0 * d * ((d - 1.0) + (d + 1.0) * cs);
        B2 = d * ((d + 1.0) + (d - 1.0) * cs - beta);
        A0 = (d + 1.0) - (d - 1.0) * cs + beta;
        A1 = 2.0 * ((d - 1.0) - (d + 1.0) * cs);
        A2 = (d + 1.0) - (d - 1.0) * cs - beta;
        break;
    }

    std::list<double> result;
    result.push_back(B0 / A0);
    result.push_back(B1 / A0);
    result.push_back(B2 / A0);
    result.push_back(-A1 / A0);
    result.push_back(-A2 / A0);
    return result;
}
std::list<double> biquad::ExportCoeffs(double dSamplingRate)
{
    return ExportCoeffs(m_dFilterType,m_dFilterGain,m_dFilterFreq,dSamplingRate,m_dFilterBQ,m_isBandwidthOrS);
}

double biquad::GainAt(double centreFreq, double fs)
{
    double num = (6.2831853071795862 * centreFreq) / fs;
    double num2 = sin(num / 2.0);
    double num3 = num2 * num2;
    double cs = a0;
    double alpha = internalBiquadCoeffs[0];
    double num6 = internalBiquadCoeffs[1];
    double A0 = 1.0;
    double A1 = -internalBiquadCoeffs[2];
    double A2 = -internalBiquadCoeffs[3];
    return ((10.0 * log10((pow((cs + alpha) + num6, 2.0) - ((4.0 * (((cs * alpha) + ((4.0 * cs) * num6)) + (alpha * num6))) * num3)) + ((((16.0 * cs) * num6) * num3) * num3))) - (10.0 * log10((pow((A0 + A1) + A2, 2.0) - ((4.0 * (((A0 * A1) + ((4.0 * A0) * A2)) + (A1 * A2))) * num3)) + ((((16.0 * A0) * A2) * num3) * num3))));
}
