#ifndef BIQUAD_H
#define BIQUAD_H
#include <list>
#include <cmath>
#include <cfloat>
#include <cstdint>

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
        CUSTOM,
        INVALID = 0xFF
    };
    biquad();
    void RefreshFilter(uint32_t id, Type type,double dbGain, double centreFreq, double fs, double dBandwidthOrQOrS, bool isBandwidthOrS);
    void RefreshFilter(uint32_t id, Type type, customFilter_t c441, customFilter_t c48);
    std::list<double> ExportCoeffs(Type type,double dbGain, double centreFreq, double fs, double dBandwidthOrQOrS, bool isBandwidthOrS);
    std::list<double> ExportCoeffs(double dSamplingRate);
    std::list<double> ExportCoeffs(customFilter_t coeffs);
    void iirroots(double b, double c, double *roots);
    int complexResponse(double centreFreq, double fs, double *HofZReal, double *HofZImag);
    double GainAt(double centreFreq, double fs);
    double PhaseResponseAt(double centreFreq, double fs);
    double toMs(double sample, double fs);
    double GroupDelayAt(double centreFreq, double fs);
    int IsStable() const;
    uint32_t getId();

private:
    double internalBiquadCoeffs[5];
    double m_dFilterBQ;
    double m_dFilterFreq;
    double m_dFilterGain;
    Type m_dFilterType;
    bool m_isBandwidthOrS;
    int m_isStable;
    bool m_isCustom;
    uint32_t m_id;
    customFilter_t m_custom441;
    customFilter_t m_custom48;
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
