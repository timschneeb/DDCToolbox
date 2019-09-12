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

void biquad::RefreshFilter(double dbGain, double centreFreq, double fs, double dBandwidthOrQOrS)
{
    m_dFilterGain = dbGain;
    m_dFilterFreq = centreFreq;
    m_dFilterBQ = dBandwidthOrQOrS;
    double d = pow(10.0, dbGain / 40.0);
    double a = (6.2831853071795862 * centreFreq) / fs;
    double num3 = sin(a);
    double cs = cos(a);
    double alpha = num3 * sinh(((0.34657359027997264 * dBandwidthOrQOrS) * a) / num3);
    double B0 = 1.0 + (alpha * d);
    double B1 = -2.0 * cs;
    double B2 = 1.0 - (alpha * d);
    double A0 = 1.0 + (alpha / d);
    double A1 = -2.0 * cs;
    double A2 = 1.0 - (alpha / d);
    a0 = B0 / A0;
    internalBiquadCoeffs[0] = B1 / A0;
    internalBiquadCoeffs[1] = B2 / A0;
    internalBiquadCoeffs[2] = -A1 / A0;
    internalBiquadCoeffs[3] = -A2 / A0;
}
std::list<double> biquad::ExportCoeffs(double dbGain, double centreFreq, double fs, double dBandwidthOrQOrS)
{
    if (centreFreq <= 2.2204460492503131e-016 || fs <= 2.2204460492503131e-016){
        std::list<double> nulllist;
        return nulllist;
    }
    double d = pow(10.0, dbGain / 40.0);
    double omega = (6.2831853071795862 * centreFreq) / fs;
    double num3 = sin(omega);
    double cs = cos(omega);
    double alpha = num3 * sinh(((0.34657359027997264 * dBandwidthOrQOrS) * omega) / num3);
    double B0 = 1.0 + (alpha * d);
    double B1 = -2.0 * cs;
    double B2 = 1.0 - (alpha * d);
    double A0 = 1.0 + (alpha / d);
    double A1 = -2.0 * cs;
    double A2 = 1.0 - (alpha / d);

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
    return ExportCoeffs(m_dFilterGain,m_dFilterFreq,dSamplingRate,m_dFilterBQ);
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
