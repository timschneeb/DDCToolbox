#ifndef NANOPEAKBIQUAD_H
#define NANOPEAKBIQUAD_H

#include "Biquad.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class NanoPeakBiquad {
public:
    double _freq;
    double _bw;
    double _gain;

    NanoPeakBiquad(double freq, double bw, double gain, double fs = 48000){
        double d = pow(10.0, gain / 40.0);
        double a = (6.2831853071795862 * freq) / fs; //omega
        double num3 = sin(a); //sn
        double cs = cos(a);
        double alpha = num3 * sinh(0.693147180559945309417 / 2 * bw * a / num3);
        double B0 = 0.0, B1 = 0.0, B2 = 0.0, A0 = 0.0, A1 = 0.0, A2 = 0.0;

        _freq = freq;
        _bw = bw;
        _gain = gain;

        B0 = 1.0 + (alpha * d);
        B1 = -2.0 * cs;
        B2 = 1.0 - (alpha * d);
        A0 = 1.0 + (alpha / d);
        A1 = -2.0 * cs;
        A2 = 1.0 - (alpha / d);

        internalBiquadCoeffs[0] = (B0 / A0);
        internalBiquadCoeffs[1] = (B1 / A0);
        internalBiquadCoeffs[2] = (B2 / A0);
        internalBiquadCoeffs[3] = (-A1 / A0);
        internalBiquadCoeffs[4] = (-A2 / A0);
    }

    double GainAt(double centreFreq, double fs = 48000)
    {
        double HofZReal, HofZImag;
        int divZero = complexResponse(centreFreq, fs, &HofZReal, &HofZImag);
        if (!divZero)
            return 0.0;
        else{
             double x = 20.0 * log10(sqrt(HofZReal * HofZReal + HofZImag * HofZImag + DBL_EPSILON));
             return x;
        }
    }

private:
    int complexResponse(double centreFreq, double fs, double *HofZReal, double *HofZImag)
    {
        double Arg;
        double z1Real, z1Imag, z2Real, z2Imag, DenomReal, DenomImag, tmpReal, tmpImag;
        Arg = M_PI * centreFreq / (fs * 0.5);
        z1Real = cos(Arg);
        z1Imag = -sin(Arg);  // z = e^(j*omega)
        Biquad::complexMultiplicationRI(&z2Real, &z2Imag, z1Real, z1Imag, z1Real, z1Imag); // z squared
        *HofZReal = 1.0;
        *HofZImag = 0.0;
        tmpReal = internalBiquadCoeffs[0] + internalBiquadCoeffs[1] * z1Real + internalBiquadCoeffs[2] * z2Real;
        tmpImag = internalBiquadCoeffs[1] * z1Imag + internalBiquadCoeffs[2] * z2Imag;
        Biquad::complexMultiplicationRI(HofZReal, HofZImag, *HofZReal, *HofZImag, tmpReal, tmpImag);
        DenomReal = 1.0 + -internalBiquadCoeffs[3] * z1Real + -internalBiquadCoeffs[4] * z2Real;
        DenomImag = -internalBiquadCoeffs[3] * z1Imag + -internalBiquadCoeffs[4] * z2Imag;
        if (sqrt(DenomReal * DenomReal + DenomImag * DenomImag) < DBL_EPSILON)
            return 0; // Division by zero, you know what to do
        else
        {
            Biquad::complexDivisionRI(HofZReal, HofZImag, *HofZReal, *HofZImag, DenomReal, DenomImag);
            return 1;
        }
    }

    double internalBiquadCoeffs[5] = {0,};
};

#endif // NANOPEAKBIQUAD_H
