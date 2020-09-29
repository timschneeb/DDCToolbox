#ifndef CACHEDBIQUAD_H
#define CACHEDBIQUAD_H

#include "FilterType.h"
#include "Biquad.h"

class DeflatedBiquad {
public:
    DeflatedBiquad(Biquad* biquad){
        id = biquad->GetId();
        type = biquad->GetFilterType();

        if(type == FilterType::CUSTOM){
            c441 = biquad->GetCustomFilter(44100);
            c48  = biquad->GetCustomFilter(48000);
        } else {
            freq = biquad->GetFrequency();
            bwOrSlope = biquad->GetBandwithOrSlope();
            gain = biquad->GetGain();
        }
    }

    /** @note Warning: No id assigned by default */
    DeflatedBiquad(FilterType type, CustomFilter c441, CustomFilter c48)
        : type(type), c441(c441), c48(c48){}

    /** @note Warning: No id assigned by default */
    DeflatedBiquad(FilterType type, int freq, double bwOrSlope, double gain)
        : type(type), freq(freq), bwOrSlope(bwOrSlope), gain(gain){}

    DeflatedBiquad(){}

    Biquad* inflate() const{
        Biquad* b = new Biquad;

        if(id != 0)
            b->SetId(id);

        if(type == FilterType::CUSTOM){
            b->RefreshFilter(type, c441, c48);
        } else {
            b->RefreshFilter(type, gain, freq, bwOrSlope);
        }

        return b;
    }

    bool operator==(const DeflatedBiquad& rhs)
    {
        return this->id == rhs.id && this->freq == rhs.freq && this->gain == rhs.gain
                && this->bwOrSlope == rhs.bwOrSlope && this->type == rhs.type
                && this->c441 == rhs.c441 && this->c48 == rhs.c48;
    }

    uint32_t id = 0;
    FilterType type = FilterType::INVALID;
    int freq = 0;
    double bwOrSlope = 1;
    double gain = 0;
    CustomFilter c441 = CustomFilter();
    CustomFilter c48 = CustomFilter();
};

#endif // CACHEDBIQUAD_H
