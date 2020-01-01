#include "ddccontext.h"
#include <mutex>
#include <map>
#include <vector>
#include <utility>
#include <list>
#include <cstring>
#include <limits>

DDCContext::DDCContext()
{
    std::random_device rd;
    std::mt19937 _mt(rd());
    mt = _mt;
    std::uniform_int_distribution<uint32_t> _rnd(1,UINT32_MAX);
    rnd = _rnd;
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
bool DDCContext::AddFilter(uint32_t id, biquad::Type type,int nFreq, double dGain, double dBandwidth, double dSRate, bool isBWorS)
{
    if(Exists(id))return false;
    LockFilter();
    if (m_lstFilterBank.count(nFreq) <= 0)
    {
        biquad *biquad = new class biquad();
        biquad->RefreshFilter(id, type, dGain, (double) nFreq, dSRate, dBandwidth, isBWorS);
        m_lstFilterBank[id] = biquad;
    }
    UnlockFilter();
    return true;
}
bool DDCContext::AddFilter(uint32_t id, customFilter_t coeffs)
{
    if(Exists(id))return false;
    LockFilter();
    if (m_lstFilterBank.count(id) <= 0)
    {
        biquad *biquad = new class biquad();
        biquad->RefreshFilter(id, biquad::CUSTOM, coeffs);
        m_lstFilterBank[id] = biquad;
    }
    UnlockFilter();
    return true;
}

void DDCContext::ClearFilters()
{
    LockFilter();
    m_lstFilterBank.clear();
    UnlockFilter();
}

bool DDCContext::ModifyFilter(uint32_t id, biquad::Type type, int nFreq, double dGain, double dBandwidth, double dSRate,bool isBWorS)
{
    if(!Exists(id))return false;
    LockFilter();
    if (m_lstFilterBank.count(id)>0)
    {
        m_lstFilterBank[id]->RefreshFilter(id, type,dGain, (double) nFreq, dSRate, dBandwidth,isBWorS);
    }
    UnlockFilter();
    return true;
}

bool DDCContext::ModifyFilter(uint32_t id, customFilter_t coeffs)
{
    if(!Exists(id))return false;
    LockFilter();
    if (m_lstFilterBank.count(id)>0)
    {
        m_lstFilterBank[id]->RefreshFilter(id, biquad::CUSTOM, coeffs);
    }
    UnlockFilter();
    return true;
}
uint32_t DDCContext::GenerateId(){
    uint32_t id;
    bool flag = false;
    while(!flag){
        id = rnd(mt);
        if(!Exists(id))
            flag = true;
    }
    return id;
}
bool DDCContext::Exists(uint32_t id){
    LockFilter();
    if (m_lstFilterBank.count(id) > 0)
    {
        UnlockFilter();
        return true;
    }
    UnlockFilter();
    return false;
}
const biquad* DDCContext::GetFilter(uint32_t id)
{
    LockFilter();
    biquad* result = nullptr;
    if (m_lstFilterBank.count(id) > 0)
    {
        std::map<int,biquad*>::iterator iter = m_lstFilterBank.find(id) ;
        if( iter != m_lstFilterBank.end() )
            result = m_lstFilterBank[id];
    }
    UnlockFilter();
    return (biquad const*)result;
}
bool DDCContext::RemoveFilter(uint32_t id){
    LockFilter();
    if (m_lstFilterBank.count(id) > 0)
    {
        std::map<int,biquad*>::iterator iter = m_lstFilterBank.find(id) ;
        if( iter != m_lstFilterBank.end() ){
            m_lstFilterBank.erase(iter);
            UnlockFilter();
            return true;
        }
    }
    UnlockFilter();
    return false;
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

std::vector<float> DDCContext::GetMagnitudeResponseTable(int nBandCount, double dSRate)
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
std::vector<float> DDCContext::GetPhaseResponseTable(int nBandCount, double dSRate)
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
            val += (float) list[k]->PhaseResponseAt(num3 * (j + 1.0), dSRate);
        vector.push_back(val);
    }
    return vector;
}

std::vector<float> DDCContext::GetGroupDelayTable(int nBandCount, double dSRate)
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
            val += (float) list[k]->GroupDelayAt(num3 * (j + 1.0), dSRate);
        vector.push_back(val);
    }
    return vector;
}
