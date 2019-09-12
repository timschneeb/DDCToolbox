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
    bool AddFilter(int nFreq, double dGain, double dBandwidth, double dSRate);
    void ModifyFilter(int nOldFreq, int nNewFreq, double dGain, double dBandwidth, double dSRate);
    void ClearFilters();
    void RemoveFilter(int nFreq);
    std::vector<float> GetResponseTable(int nBandCount, double dSRate);
    std::list<double> ExportCoeffs(double dSamplingRate);

private:
    std::mutex mtx;
    std::map<int,biquad*> m_lstFilterBank;
    void LockFilter();
    void UnlockFilter();
};

#endif // DDCCONTEXT_H
