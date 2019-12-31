#ifndef BIQUAD_H
#define BIQUAD_H
#include <list>

typedef struct customFilter_s{
    double a0;
    double a1;
    double a2;
    double b0;
    double b1;
    double b2;
}customFilter_t;

class biquad
{
public:
    enum Type
    {
        PEAKING = 0x00,
        LOW_PASS,
        HIGH_PASS,
        BAND_PASS1,
        BAND_PASS2,
        NOTCH,
        ALL_PASS,
        LOW_SHELF,
        HIGH_SHELF,
        UNITY_GAIN,
        ONEPOLE_LOWPASS,
        ONEPOLE_HIGHPASS,
        CUSTOM
    };
    biquad();
    void RefreshFilter(Type type,double dbGain, double centreFreq, double fs, double dBandwidthOrQOrS, bool isBandwidthOrS);
    void RefreshFilter(Type type, customFilter_t coeffs, double centreFreq, double fs);
    std::list<double> ExportCoeffs(Type type,double dbGain, double centreFreq, double fs, double dBandwidthOrQOrS, bool isBandwidthOrS);
    std::list<double> ExportCoeffs(double dSamplingRate);
    std::list<double> ExportCoeffs(Type type, customFilter_t coeffs, double centreFreq, double fs);
    int complexResponse(double centreFreq, double fs, double *HofZReal, double *HofZImag);
    double GainAt(double centreFreq, double fs);
    double PhaseResponseAt(double centreFreq, double fs);
    double GroupDelayAt(double centreFreq, double fs);
    int IsStable() const;

private:
    double internalBiquadCoeffs[4];
    double a0;
    double m_dFilterBQ;
    double m_dFilterFreq;
    double m_dFilterGain;
    Type m_dFilterType;
    bool m_isBandwidthOrS;
    int m_isStable;
    bool m_isCustom;
    customFilter_t m_custom;
    inline void complexMultiplicationRI(double *zReal, double *zImag, double xReal, double xImag, double yReal, double yImag)
    {
        *zReal = xReal * yReal - xImag * yImag;
        *zImag = xReal * yImag + xImag * yReal;
    }
    inline void complexDivisionRI(double *zReal, double *zImag, double xReal, double xImag, double yReal, double yImag)
    {
        *zReal = (xReal * yReal + xImag * yImag) / (yReal * yReal + yImag * yImag);
        *zImag = (xImag * yReal - xReal * yImag) / (yReal * yReal + yImag * yImag);
    }
};


#endif // BIQUAD_H
