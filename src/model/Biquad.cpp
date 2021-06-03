#include "Biquad.h"
#include <QDebug>
#include <cstdio>
#include <list>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static uint64_t biquad_object_count = 0;

Biquad::Biquad(bool idless)
{
    if(biquad_object_count >= UINT64_MAX - 1)
        biquad_object_count = 0;

    if(!idless)
        m_id = ++biquad_object_count;

    internalBiquadCoeffs[0] = 0.0;
    internalBiquadCoeffs[1] = 0.0;
    internalBiquadCoeffs[2] = 0.0;
    internalBiquadCoeffs[3] = 0.0;
    internalBiquadCoeffs[4] = 0.0;
}

void Biquad::RefreshFilter(FilterType type, double dbGain, double centreFreq, double dBandwidthOrS)
{
    m_isCustom = false;
    m_dFilterType = type;
    m_dFilterGain = dbGain;
    m_dFilterFreq = centreFreq;
    m_dFilterBQ = dBandwidthOrS;

    auto coeffs = CalculateCoeffs(48000);
    for(double & internalBiquadCoeff : internalBiquadCoeffs){
        if(coeffs.empty()){
            qWarning() << "RefreshFilter: [Warning] failed to calculate coeff";
            internalBiquadCoeff = 0;
            continue;
        }
        internalBiquadCoeff = coeffs.front();
        coeffs.pop_front();
    }

    CalculateStability();
}

void Biquad::RefreshFilter(FilterType type, const CustomFilter& c441, const CustomFilter& c48)
{
    m_isCustom = true;
    m_custom441 = c441;
    m_custom48 = c48;
    m_dFilterType = type;

    auto coeffs = CalculateCoeffs(c48);
    for(double & internalBiquadCoeff : internalBiquadCoeffs){
        internalBiquadCoeff = coeffs.front();
        coeffs.pop_front();
    }
    CalculateStability();
}

std::list<double> Biquad::CalculateCoeffs(double fs, bool noA0divide)
{
    if (m_dFilterFreq <= 2.2204460492503131e-016 || fs <= 2.2204460492503131e-016){
        qWarning() << "Biquad::CalculateCoeffs: Invalid frequency or samplerate";
        return std::list<double>();
    }

    double d;
    if (m_dFilterType == FilterType::PEAKING || m_dFilterType == FilterType::LOW_SHELF || m_dFilterType == FilterType::HIGH_SHELF)
        d = pow(10.0, m_dFilterGain / 40.0);
    else
        d = pow(10.0, m_dFilterGain / 20.0);
    double a = (2.0 * M_PI * m_dFilterFreq) / fs; //omega
    double num3 = sin(a); //sn
    double cs = cos(a);

    double alpha;
    if (m_dFilterType == FilterType::LOW_SHELF || m_dFilterType == FilterType::HIGH_SHELF) // S
        alpha = num3 / 2 * sqrt((d + 1 / d) * (1 / m_dFilterBQ - 1) + 2);
    else // BW
        alpha = num3 * sinh(log(2.0) / 2.0 * m_dFilterBQ * a / num3);

    double beta = 2 * sqrt(d) * alpha;
    double B0 = 0.0, B1 = 0.0, B2 = 0.0, A0 = 0.0, A1 = 0.0, A2 = 0.0;

    switch (m_dFilterType)
    {
    case FilterType::LOW_PASS:
        B0 = (1.0 - cs) / 2.0;
        B1 = 1.0 - cs;
        B2 = (1.0 - cs) / 2.0;
        A0 = 1.0 + alpha;
        A1 = -2.0 * cs;
        A2 = 1.0 - alpha;
        break;
    case FilterType::HIGH_PASS:
        B0 = (1.0 + cs) / 2.0;
        B1 = -(1.0 + cs);
        B2 = (1.0 + cs) / 2.0;
        A0 = 1.0 + alpha;
        A1 = -2.0 * cs;
        A2 = 1.0 - alpha;
        break;
    case FilterType::BAND_PASS1:
        //BPF, constant skirt gain (peak gain = BW)
        B0 = m_dFilterBQ * alpha;// sn / 2;
        B1 = 0;
        B2 = -m_dFilterBQ * alpha;//-sn / 2;
        A0 = 1 + alpha;
        A1 = -2 * cs;
        A2 = 1 - alpha;
        break;
    case FilterType::BAND_PASS2:
        //BPF, constant 0dB peak gain
        B0 = alpha;
        B1 = 0.0;
        B2 = -alpha;
        A0 = 1.0 + alpha;
        A1 = -2.0 * cs;
        A2 = 1.0 - alpha;
        break;
    case FilterType::NOTCH:
        B0 = 1.0;
        B1 = -2.0 * cs;
        B2 = 1.0;
        A0 = 1.0 + alpha;
        A1 = -2.0 * cs;
        A2 = 1.0 - alpha;
        break;
    case FilterType::ALL_PASS:
        B0 = 1.0 - alpha;
        B1 = -2.0 * cs;
        B2 = 1.0 + alpha;
        A0 = 1.0 + alpha;
        A1 = -2.0 * cs;
        A2 = 1.0 - alpha;
        break;
    case FilterType::PEAKING:
        B0 = 1.0 + (alpha * d);
        B1 = -2.0 * cs;
        B2 = 1.0 - (alpha * d);
        A0 = 1.0 + (alpha / d);
        A1 = -2.0 * cs;
        A2 = 1.0 - (alpha / d);
        break;
    case FilterType::LOW_SHELF:
        B0 = d * ((d + 1.0) - (d - 1.0) * cs + beta);
        B1 = 2.0 * d * ((d - 1.0) - (d + 1.0) * cs);
        B2 = d * ((d + 1.0) - (d - 1.0) * cs - beta);
        A0 = (d + 1.0) + (d - 1.0) * cs + beta;
        A1 = -2.0 * ((d - 1.0) + (d + 1.0) * cs);
        A2 = (d + 1.0) + (d - 1.0) * cs - beta;
        break;
    case FilterType::HIGH_SHELF:
        B0 = d * ((d + 1.0) + (d - 1.0) * cs + beta);
        B1 = -2.0 * d * ((d - 1.0) + (d + 1.0) * cs);
        B2 = d * ((d + 1.0) + (d - 1.0) * cs - beta);
        A0 = (d + 1.0) - (d - 1.0) * cs + beta;
        A1 = 2.0 * ((d - 1.0) - (d + 1.0) * cs);
        A2 = (d + 1.0) - (d - 1.0) * cs - beta;
        break;
    case FilterType::UNITY_GAIN:
        B0 = d;
        B1 = 0.0;
        B2 = 0.0;
        A0 = 1.0;
        A1 = 0.0;
        A2 = 0.0;
        break;
    case FilterType::ONEPOLE_LOWPASS:
        B0 = tan(M_PI * m_dFilterFreq / fs * 0.5);
        A1 = -(1.0 - B0) / (1.0 + B0);
        B1 = B0 = B0 / (1.0 + B0);
        B2 = 0.0;
        A0 = 1.0;
        A2 = 0.0;
        break;
    case FilterType::ONEPOLE_HIGHPASS:
        B0 = tan(M_PI * m_dFilterFreq / fs * 0.5);
        A1 = -(1.0 - B0) / (1.0 + B0);
        B0 = 1.0 - (B0 / (1.0 + B0));
        B2 = 0.0;
        B1 = -B0;
        A0 = 1.0;
        A2 = 0.0;
        break;
    default:
        qWarning() << "Biquad::CalculateCoeffs: unknown filter type";
        return std::list<double>();
    }

    std::list<double> result;
    if(noA0divide){
        result.push_back(B0);
        result.push_back(B1);
        result.push_back(B2);
        result.push_back(A0);
        result.push_back(A1);
        result.push_back(A2);
    }
    else {
        result.push_back(B0 / A0);
        result.push_back(B1 / A0);
        result.push_back(B2 / A0);
        result.push_back(-A1 / A0);
        result.push_back(-A2 / A0);
    }
    return result;
}

std::list<double> Biquad::CalculateCoeffs(CustomFilter coeffs, bool noA0divide)
{
    if(noA0divide){
        return coeffs.toList();
    }

    std::list<double> result;
    double A0 = coeffs.a0;
    result.push_back(coeffs.b0 / A0);
    result.push_back(coeffs.b1 / A0);
    result.push_back(coeffs.b2 / A0);
    result.push_back(-coeffs.a1 / A0);
    result.push_back(-coeffs.a2 / A0);
    return result;
}

std::list<double> Biquad::ExportCoeffs(double dSamplingRate, bool noA0divide)
{
    if(m_isCustom)
        if(qFuzzyCompare(dSamplingRate, 44100))
            return CalculateCoeffs(m_custom441, noA0divide);
        else if(qFuzzyCompare(dSamplingRate, 48000))
            return CalculateCoeffs(m_custom48, noA0divide);
        else
        {
            qWarning() << "ERROR: Biquad::ExportCoeffs: Invalid sampling rate for CustomFilter. Cannot export coeffs.";
            return std::list<double>();
        }
    else
        return CalculateCoeffs(dSamplingRate, noA0divide);
}

void Biquad::CalculateStability(){
    //Check if filter is stable/usable
    double roots[4];
    iirroots(-internalBiquadCoeffs[3], -internalBiquadCoeffs[4], roots);
    double pole1Magnitude = sqrt(roots[0] * roots[0] + roots[1] * roots[1]);
    double pole2Magnitude = sqrt(roots[2] * roots[2] + roots[3] * roots[3]);
    m_isStable = UNSTABLE; // Assume all pole is unstable
    if (pole1Magnitude < 1.0 && pole2Magnitude < 1.0)
    {
        if (1.0 - pole1Magnitude < 8e-14 || 1.0 - pole2Magnitude < 8e-14)
            m_isStable = PARTIALLY_STABLE; // Not so stable, due to our tool text formatting OR V4A string inaccuracy
        else
            m_isStable = STABLE; // Perfectly stable
    }
}

// Provided by: James34602
void Biquad::iirroots(double b, double c, double *roots)
{
    double delta = b * b - 4.0 * c;
    if (delta >= 0.0)
    {
        roots[0] = (-b - sqrt(delta)) / 2.0;
        roots[1] = 0.0;
        roots[2] = (-b + sqrt(delta)) / 2.0;
        roots[3] = 0.0;
    }
    else
    {
        roots[0] = roots[2] = -b / 2.0;
        roots[1] = sqrt(-delta) / 2.0;
        roots[3] = -sqrt(-delta) / 2.0;
    }
}

int Biquad::complexResponse(double centreFreq, double fs, double *HofZReal, double *HofZImag)
{
    double Arg;
    double z1Real, z1Imag, z2Real, z2Imag, DenomReal, DenomImag, tmpReal, tmpImag;
    Arg = M_PI * centreFreq / (fs * 0.5);
    z1Real = cos(Arg);
    z1Imag = -sin(Arg);  // z = e^(j*omega)
    complexMultiplicationRI(&z2Real, &z2Imag, z1Real, z1Imag, z1Real, z1Imag); // z squared
    *HofZReal = 1.0;
    *HofZImag = 0.0;
    tmpReal = internalBiquadCoeffs[0] + internalBiquadCoeffs[1] * z1Real + internalBiquadCoeffs[2] * z2Real;
    tmpImag = internalBiquadCoeffs[1] * z1Imag + internalBiquadCoeffs[2] * z2Imag;
    complexMultiplicationRI(HofZReal, HofZImag, *HofZReal, *HofZImag, tmpReal, tmpImag);
    DenomReal = 1.0 + -internalBiquadCoeffs[3] * z1Real + -internalBiquadCoeffs[4] * z2Real;
    DenomImag = -internalBiquadCoeffs[3] * z1Imag + -internalBiquadCoeffs[4] * z2Imag;
    if (sqrt(DenomReal * DenomReal + DenomImag * DenomImag) < DBL_EPSILON)
        return 0; // Division by zero, you know what to do
    else
    {
        complexDivisionRI(HofZReal, HofZImag, *HofZReal, *HofZImag, DenomReal, DenomImag);
        return 1;
    }
}

double Biquad::GainAt(double centreFreq, double fs)
{
    double HofZReal, HofZImag;
    int divZero = complexResponse(centreFreq, fs, &HofZReal, &HofZImag);
    if (!divZero)
        return 0.0;
    else
        return 20.0 * log10(sqrt(HofZReal * HofZReal + HofZImag * HofZImag + DBL_EPSILON));
}

double Biquad::PhaseResponseAt(double centreFreq, double fs)
{
    double HofZReal, HofZImag;
    int divZero = complexResponse(centreFreq, fs, &HofZReal, &HofZImag);
    if (!divZero)
        return 0.0;
    else
        return atan2(HofZImag, HofZReal) * 180.0 / M_PI;
}

// Simplified Shpak group delay algorithm
// The algorithm only valid when first order / second order IIR filters is provided
// You must break down high order transfer function into N-SOS in order apply the Shpak algorithm
// which is out-of-scope here, since break down high order transfer function require find roots of polynomials
// Root finder may often require the computation of eigenvalue of companion matrix of polynomials
// Which will bloat 1000+ lines of code, and perhaps not the main purpose here.
// We just need to calculate group delay of a bunch of second order IIR filters, so the following code already do the job
// Provided by: James34602
double Biquad::toMs(double sample, double fs)
{
    return sample / (fs / 1000.0);
}

double Biquad::GroupDelayAt(double centreFreq, double fs)
{
    double Arg = M_PI * centreFreq / (fs * 0.5);
    double cw = cos(Arg);
    double cw2 = cos(2.0 * Arg);
    double sw = sin(Arg);
    double sw2 = sin(2.0 * Arg);
    double b1 = internalBiquadCoeffs[0], b2 = internalBiquadCoeffs[1], b3 = internalBiquadCoeffs[2];
    double u = b1 * sw2 + b2 * sw;
    double v = b1 * cw2 + b2 * cw + b3;
    double du = 2.0 * b1*cw2 + b2 * cw;
    double dv = -(2.0 * b1*sw2 + b2 * sw);
    double u2v2 = (b1*b1) + (b2*b2) + (b3*b3) + 2.0 * (b1*b2 + b2 * b3)*cw + 2.0 * (b1*b3)*cw2;
    double gdB = (2.0 - (v*du - u * dv) / u2v2);
    b2 = -internalBiquadCoeffs[3];
    b3 = -internalBiquadCoeffs[4];
    u = sw2 + b2 * sw;
    v = cw2 + b2 * cw + b3;
    du = 2.0 * cw2 + b2 * cw;
    dv = -(2.0 * sw2 + b2 * sw);
    u2v2 = 1.0 + (b2*b2) + (b3*b3) + 2.0 * (b2 + b2 * b3)*cw + 2.0 * b3*cw2;
    return toMs(gdB - (2.0 - (v*du - u * dv) / u2v2), fs);
}
