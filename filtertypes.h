#ifndef FILTERTYPES_H
#define FILTERTYPES_H
#include "biquad.h"
#include <QString>

inline biquad::Type stringToType(QString _type){
    if(_type=="Peaking")return biquad::Type::PEAKING;
    else if(_type=="Low Pass")return biquad::Type::LOW_PASS;
    else if(_type=="High Pass")return biquad::Type::HIGH_PASS;
    else if(_type=="Band Pass")return biquad::Type::BAND_PASS;
    else if(_type=="All Pass")return biquad::Type::ALL_PASS;
    else if(_type=="Notch")return biquad::Type::NOTCH;
    else if(_type=="Low Shelf")return biquad::Type::LOW_SHELF;
    else if(_type=="High Shelf")return biquad::Type::HIGH_SHELF;
    else if(_type=="Unity Gain")return biquad::Type::UNITY_GAIN;
    else if(_type=="One-Pole Low Pass")return biquad::Type::ONEPOLE_LOWPASS;
    else if(_type=="One-Pole High Pass")return biquad::Type::ONEPOLE_HIGHPASS;
    return biquad::Type::PEAKING;
}
inline QString typeToString(biquad::Type _type){
    if(_type==biquad::Type::PEAKING)return "Peaking";
    else if(_type==biquad::Type::LOW_PASS)return "Low Pass";
    else if(_type==biquad::Type::HIGH_PASS)return "High Pass";
    else if(_type==biquad::Type::BAND_PASS)return "Band Pass";
    else if(_type==biquad::Type::ALL_PASS)return "All Pass";
    else if(_type==biquad::Type::NOTCH)return "Notch";
    else if(_type==biquad::Type::LOW_SHELF)return "Low Shelf";
    else if(_type==biquad::Type::HIGH_SHELF)return "High Shelf";
    else if(_type==biquad::Type::UNITY_GAIN)return "Unity Gain";
    else if(_type==biquad::Type::ONEPOLE_LOWPASS)return "One-Pole Low Pass";
    else if(_type==biquad::Type::ONEPOLE_HIGHPASS)return "One-Pole High Pass";
    return "Peaking";
}


#endif // FILTERTYPES_H
