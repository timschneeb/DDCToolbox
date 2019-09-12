#ifndef BIQUAD_H
#define BIQUAD_H
#include <list>

class biquad
{
public:
    biquad();
    void RefreshFilter(double dbGain, double centreFreq, double fs, double dBandwidthOrQOrS);
    std::list<double> ExportCoeffs(double dbGain, double centreFreq, double fs, double dBandwidthOrQOrS);
    std::list<double> ExportCoeffs(double dSamplingRate);
    double GainAt(double centreFreq, double fs);

private:
    double internalBiquadCoeffs[4];
    double a0;
    double m_dFilterBQ;
    double m_dFilterFreq;
    double m_dFilterGain;
};


#endif // BIQUAD_H
