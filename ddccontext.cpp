#include "ddccontext.h"
#include <mutex>
#include <map>
#include <vector>
#include <utility>
#include <list>
#include <cstring>

DDCContext::DDCContext()
{
    m_lstFilterBank.clear();
}
void DDCContext::LockFilter()
{
    mtx.lock();
}
void DDCContext::UnlockFilter()
{
    mtx.unlock();
}
bool DDCContext::AddFilter(int nFreq, double dGain, double dBandwidth, double dSRate)
{
    LockFilter();
    if (m_lstFilterBank.count(nFreq) <= 0)
    {
        biquad *biquad = new class biquad();
        biquad->RefreshFilter(dGain, (double) nFreq, dSRate, dBandwidth);
        m_lstFilterBank[nFreq] = biquad;
    }
    UnlockFilter();
    return false;
}

void DDCContext::ClearFilters()
{
    LockFilter();
    m_lstFilterBank.clear();
    UnlockFilter();
}

void DDCContext::ModifyFilter(int nOldFreq, int nNewFreq, double dGain, double dBandwidth, double dSRate)
{
    LockFilter();
    if (nOldFreq == nNewFreq)
    {
        if (m_lstFilterBank.count(nOldFreq)>0)
        {
            m_lstFilterBank[nOldFreq]->RefreshFilter(dGain, (double) nNewFreq, dSRate, dBandwidth);
        }
    }
    else if (m_lstFilterBank.count(nOldFreq) > 0 && m_lstFilterBank.count(nNewFreq) <= 0)
    {
        std::map<int,biquad*>::iterator iter = m_lstFilterBank.find(nOldFreq) ;
        if( iter != m_lstFilterBank.end() )
            m_lstFilterBank.erase(iter);

        if(m_lstFilterBank.count(nNewFreq) > 0){
            std::map<int,biquad*>::iterator iter2 = m_lstFilterBank.find(nNewFreq) ;
            if( iter2 != m_lstFilterBank.end() )
                m_lstFilterBank.erase(iter2);
        }


        biquad *biquad = new class biquad();
        biquad->RefreshFilter(dGain, (double) nNewFreq, dSRate, dBandwidth);
        m_lstFilterBank[nNewFreq] = biquad;
    }
    UnlockFilter();
}

void DDCContext::RemoveFilter(int nFreq)
{
    LockFilter();
    if (m_lstFilterBank.count(nFreq) > 0)
    {
        std::map<int,biquad*>::iterator iter = m_lstFilterBank.find(nFreq) ;
        if( iter != m_lstFilterBank.end() )
            m_lstFilterBank.erase(iter);
    }
    UnlockFilter();
}

std::list<double> DDCContext::ExportCoeffs(double dSamplingRate)
{
    LockFilter();

    std::vector<biquad*> list;
    std::map<int,biquad*>::iterator it;

    for ( it = m_lstFilterBank.begin(); it != m_lstFilterBank.end(); it++ )
        list.push_back(it->second);

    if (list.size() <= 0)
    {
        UnlockFilter();
        std::list<double> nulllist;
        return nulllist;
    }
    std::list<double> numArray;
    for (size_t i = 0; i < list.size(); i++)
    {
        std::list<double> numArray2 = list[i]->ExportCoeffs(dSamplingRate);
        if (numArray2.empty())
        {
            list.clear();
            UnlockFilter();
            std::list<double> nulllist;
            return nulllist;
        }
        numArray.splice(numArray.end(), numArray2);
    }
    list.clear();
    UnlockFilter();

    return numArray;
}

std::vector<float> DDCContext::GetResponseTable(int nBandCount, double dSRate)
{
    std::vector<float> vector;
    if (nBandCount <= 0)
    {
        return vector;
    }
    std::vector<biquad*> list;
    LockFilter();

    std::map<int,biquad*>::iterator it;

    for ( it = m_lstFilterBank.begin(); it != m_lstFilterBank.end(); it++ )
        list.push_back(it->second);

    UnlockFilter();

    for (int j = 0; j < nBandCount; j++)
    {
        double num3 = (dSRate / 2.0) / ((double) nBandCount);
        float val = 0.0f;
        for (size_t k = 0;k < list.size(); k++)
            val += (float) list[k]->GainAt(num3 * (j + 1.0), dSRate);
        vector.push_back(val);
    }
    return vector;
}
