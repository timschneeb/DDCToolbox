#ifndef DDCCONTEXT_H
#define DDCCONTEXT_H
#include <mutex>
#include <map>
#include <vector>
#include "biquad.h"

class DDCContext
{
public:
    DDCContext();
    bool AddFilter(biquad::Type type,int nFreq, double dGain, double dBandwidth, double dSRate, bool isBWorS);
    void ModifyFilter(biquad::Type type,int nOldFreq, int nNewFreq, double dGain, double dBandwidth, double dSRate, bool isBWorS);
    void ClearFilters();
    void RemoveFilter(int nFreq);
    const biquad* GetFilter(int nFreq);
    std::vector<float> GetMagnitudeResponseTable(int nBandCount, double dSRate);
    std::vector<float> GetGroupDelayTable(int nBandCount, double dSRate);
    std::list<double> ExportCoeffs(double dSamplingRate);

private:
    std::mutex mtx;
    std::map<int,biquad*> m_lstFilterBank;
    void LockFilter();
    void UnlockFilter();
};

#endif // DDCCONTEXT_H
