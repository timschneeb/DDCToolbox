#ifndef BIQUAD_H
#define BIQUAD_H
#include <list>
#include <cmath>
#include <cfloat>
#include <cstdint>

#include "FilterType.h"

#define FREQ_MIN 0
#define FREQ_MAX 24000
#define BW_MIN 0
#define BW_MAX 100
#define GAIN_MIN -40
#define GAIN_MAX 40

class CustomFilter {
public:
    CustomFilter(){};
    CustomFilter(double a0, double a1, double a2,
                 double b0, double b1, double b2)
        : a0(a0), a1(a1), a2(a2), b0(b0), b1(b1), b2(b2){};

    double a0 = 1.0;
    double a1 = 0.0;
    double a2 = 0.0;
    double b0 = 1.0;
    double b1 = 0.0;
    double b2 = 0.0;

    bool operator==(const CustomFilter& rhs)
    {
        return this->a0 == rhs.a0 && this->a1 == rhs.a1 && this->a2 == rhs.a2
                && this->b0 == rhs.b0 && this->b1 == rhs.b1 && this->b2 == rhs.b2;
    }

    std::list<double> toList(){
        std::list<double> result;
        result.push_back(b0);
        result.push_back(b1);
        result.push_back(b2);
        result.push_back(a0);
        result.push_back(a1);
        result.push_back(a2);
        return result;
    }
};

class Biquad
{
public:
    enum Stability
    {
        UNSTABLE = 0,
        STABLE = 1,
        PARTIALLY_STABLE = 2,
        UNKNOWN = 0xFF
    };

    Biquad(bool idless = false);
    ~Biquad(){};
    void RefreshFilter(FilterType type, double dbGain, double centreFreq, double dBandwidthOrS);
    void RefreshFilter(FilterType type, const CustomFilter& c441, const CustomFilter& c48);
    std::list<double> ExportCoeffs(double dSamplingRate, bool noA0divide = false);

    double GainAt(double centreFreq, double fs);
    double PhaseResponseAt(double centreFreq, double fs);
    double GroupDelayAt(double centreFreq, double fs);

    uint32_t GetId() const{
        return m_id;
    };

    Stability IsStable() const {
        return m_isStable;
    };

    FilterType GetFilterType() const{
        return m_dFilterType;
    };

    double GetFrequency() const{
        return m_dFilterFreq;
    };

    double GetBandwithOrSlope() const{
        return m_dFilterBQ;
    };

    double GetGain() const{
        return m_dFilterGain;
    };

    CustomFilter GetCustomFilter(int samplerate) const{
        return samplerate == 44100 ? m_custom441 : m_custom48;
    };

    void SetId(uint32_t id){
        m_id = id;
    };

    /* Convenience wrapper functions around RefreshFilter() */
    void SetFilterType(FilterType type){
        if(type == FilterType::CUSTOM)
            RefreshFilter(type, GetCustomFilter(44100), GetCustomFilter(48000));
        else
            RefreshFilter(type,
                          GetGain(),
                          GetFrequency(),
                          GetBandwithOrSlope());
    }

    void SetFrequency(int freq){
        if(GetFilterType() == FilterType::CUSTOM)
            RefreshFilter(GetFilterType(), GetCustomFilter(44100), GetCustomFilter(48000));
        else
            RefreshFilter(GetFilterType(),
                          GetGain(),
                          freq,
                          GetBandwithOrSlope());
    }

    void SetBandwidthOrSlope(double bw){
        if(GetFilterType() == FilterType::CUSTOM)
            RefreshFilter(GetFilterType(), GetCustomFilter(44100), GetCustomFilter(48000));
        else
            RefreshFilter(GetFilterType(),
                      GetGain(),
                      GetFrequency(),
                      bw);
    }

    void SetGain(double gain){
        if(GetFilterType() == FilterType::CUSTOM)
            RefreshFilter(GetFilterType(), GetCustomFilter(44100), GetCustomFilter(48000));
        else
            RefreshFilter(GetFilterType(),
                      gain,
                      GetFrequency(),
                      GetBandwithOrSlope());
    }

    void SetCustomFilter(CustomFilter c44100, CustomFilter c48000){
        RefreshFilter(GetFilterType(), c44100, c48000);
    };

private:
    int32_t m_id = 0;
    double internalBiquadCoeffs[5];
    double m_dFilterBQ = 1;
    double m_dFilterFreq = 10;
    double m_dFilterGain = 0;
    FilterType m_dFilterType;
    Stability m_isStable = UNKNOWN;
    bool m_isCustom;
    CustomFilter m_custom441;
    CustomFilter m_custom48;

    std::list<double> CalculateCoeffs(CustomFilter coeffs, bool noA0divide = false);
    std::list<double> CalculateCoeffs(double fs, bool noA0divide = false);

    void CalculateStability();

    void iirroots(double b, double c, double *roots);
    int complexResponse(double centreFreq, double fs, double *HofZReal, double *HofZImag);
    double toMs(double sample, double fs);

public:
    static void complexMultiplicationRI(double *zReal, double *zImag, double xReal, double xImag, double yReal, double yImag)
    {
        *zReal = xReal * yReal - xImag * yImag;
        *zImag = xReal * yImag + xImag * yReal;
    }
    static void complexDivisionRI(double *zReal, double *zImag, double xReal, double xImag, double yReal, double yImag)
    {
        *zReal = (xReal * yReal + xImag * yImag) / (yReal * yReal + yImag * yImag);
        *zImag = (xImag * yReal - xReal * yImag) / (yReal * yReal + yImag * yImag);
    }

    /* This section defines several comparators for use with a sorting algorithm */
    static bool compareFrequency(Biquad* s1, Biquad* s2)
    {
        if(!s1->GetFilterType().getSpecs().test(FilterType::SPEC_REQUIRE_FREQ))
            return false;
        else if(!s2->GetFilterType().getSpecs().test(FilterType::SPEC_REQUIRE_FREQ))
            return true;
        return s1->GetFrequency() < s2->GetFrequency();
    }
    static bool compareBwOrSlope(Biquad* s1, Biquad* s2)
    {
        if(!s1->GetFilterType().getSpecs().test(FilterType::SPEC_REQUIRE_BW) &&
           !s1->GetFilterType().getSpecs().test(FilterType::SPEC_REQUIRE_SLOPE))
            return false;
        else if(!s2->GetFilterType().getSpecs().test(FilterType::SPEC_REQUIRE_BW) &&
           !s2->GetFilterType().getSpecs().test(FilterType::SPEC_REQUIRE_SLOPE))
            return true;
        return s1->GetBandwithOrSlope() < s2->GetBandwithOrSlope();
    }
    static bool compareGain(Biquad* s1, Biquad* s2)
    {
        if(!s1->GetFilterType().getSpecs().test(FilterType::SPEC_REQUIRE_GAIN))
            return false;
        else if(!s2->GetFilterType().getSpecs().test(FilterType::SPEC_REQUIRE_GAIN))
            return true;
        return s1->GetGain() < s2->GetGain();
    }
    static bool compareType(Biquad* s1, Biquad* s2)
    {
        return s1->GetFilterType().ordinal() < s2->GetFilterType().ordinal();
    }
};

#endif // BIQUAD_H
