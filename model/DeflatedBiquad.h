#ifndef CACHEDBIQUAD_H
#define CACHEDBIQUAD_H

#include "FilterType.h"
#include "Biquad.h"

class DeflatedBiquad {
public:
    DeflatedBiquad(Biquad* biquad);

    /** @note Warning: No id assigned by default */
    DeflatedBiquad(FilterType type, CustomFilter c441, CustomFilter c48)
        : type(type), c441(c441), c48(c48){}

    /** @note Warning: No id assigned by default */
    DeflatedBiquad(FilterType type, int freq, double bwOrSlope, double gain)
        : type(type), freq(freq), bwOrSlope(bwOrSlope), gain(gain){}

    DeflatedBiquad(){}

    Biquad* inflate() const;

    bool operator==(const DeflatedBiquad& rhs);

    uint32_t id = 0;
    FilterType type = FilterType::INVALID;
    int freq = 0;
    double bwOrSlope = 1;
    double gain = 0;
    CustomFilter c441 = CustomFilter();
    CustomFilter c48 = CustomFilter();
};

#endif // CACHEDBIQUAD_H
