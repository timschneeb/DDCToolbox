#include "DeflatedBiquad.h"

#include <QDebug>

DeflatedBiquad::DeflatedBiquad(Biquad *biquad){
    _id = biquad->GetId();
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

Biquad *DeflatedBiquad::inflate() const{
    Biquad* b = nullptr;

    if(_id != 0){
        b = new Biquad(true);
        b->SetId(_id);
    }
    else{
        b = new Biquad;
    }

    if(type == FilterType::CUSTOM){
        b->RefreshFilter(type, c441, c48);
    } else {
        b->RefreshFilter(type, gain, freq, bwOrSlope);
    }

    return b;
}

bool DeflatedBiquad::operator==(const DeflatedBiquad &rhs)
{
    return this->_id == rhs._id && this->freq == rhs.freq && this->gain == rhs.gain
            && this->bwOrSlope == rhs.bwOrSlope && this->type == rhs.type
            && this->c441 == rhs.c441 && this->c48 == rhs.c48;
}

void DeflatedBiquad::setId(uint32_t newId)
{
    _id = newId;
}

uint32_t DeflatedBiquad::id() const
{
    qWarning() << "WARNING: DeflatedBiquad::id(): Requested id of filter is null";
    return _id;
}
