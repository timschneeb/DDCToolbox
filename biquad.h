#ifndef BIQUAD_H
#define BIQUAD_H
#include <list>

class biquad
{
public:
    enum Type
    {
        LOW_PASS, HIGH_PASS, BAND_PASS, NOTCH, ALL_PASS, PEAKING, LOW_SHELF, HIGH_SHELF
    };
    biquad();
    void RefreshFilter(Type type,double dbGain, double centreFreq, double fs, double dBandwidthOrQOrS, bool isBandwidthOrS);
    std::list<double> ExportCoeffs(Type type,double dbGain, double centreFreq, double fs, double dBandwidthOrQOrS, bool isBandwidthOrS);
    std::list<double> ExportCoeffs(double dSamplingRate);
    double GainAt(double centreFreq, double fs);


private:
    double internalBiquadCoeffs[4];
    double a0;
    double m_dFilterBQ;
    double m_dFilterFreq;
    double m_dFilterGain;
    Type m_dFilterType;
    bool m_isBandwidthOrS;
};


#endif // BIQUAD_H
