#ifndef FILTERTYPES_H
#define FILTERTYPES_H
#include "Biquad.h"
#include <QString>

inline Biquad::Type stringToType(QString _type){
    if(_type=="Peaking")return Biquad::Type::PEAKING;
    else if(_type=="Low Pass")return Biquad::Type::LOW_PASS;
    else if(_type=="High Pass")return Biquad::Type::HIGH_PASS;
    else if(_type=="Band Pass")return Biquad::Type::BAND_PASS2;
    else if(_type=="Band Pass (peak gain = bw)")return Biquad::Type::BAND_PASS1;
    else if(_type=="All Pass")return Biquad::Type::ALL_PASS;
    else if(_type=="Notch")return Biquad::Type::NOTCH;
    else if(_type=="Low Shelf")return Biquad::Type::LOW_SHELF;
    else if(_type=="High Shelf")return Biquad::Type::HIGH_SHELF;
    else if(_type=="Unity Gain")return Biquad::Type::UNITY_GAIN;
    else if(_type=="One-Pole Low Pass")return Biquad::Type::ONEPOLE_LOWPASS;
    else if(_type=="One-Pole High Pass")return Biquad::Type::ONEPOLE_HIGHPASS;
    else if(_type=="Custom")return Biquad::Type::CUSTOM;
    return Biquad::Type::PEAKING;
}
inline QString typeToString(Biquad::Type _type){
    if(_type==Biquad::Type::PEAKING)return "Peaking";
    else if(_type==Biquad::Type::LOW_PASS)return "Low Pass";
    else if(_type==Biquad::Type::HIGH_PASS)return "High Pass";
    else if(_type==Biquad::Type::BAND_PASS2)return "Band Pass";
    else if(_type==Biquad::Type::BAND_PASS1)return "Band Pass (peak gain = bw)";
    else if(_type==Biquad::Type::ALL_PASS)return "All Pass";
    else if(_type==Biquad::Type::NOTCH)return "Notch";
    else if(_type==Biquad::Type::LOW_SHELF)return "Low Shelf";
    else if(_type==Biquad::Type::HIGH_SHELF)return "High Shelf";
    else if(_type==Biquad::Type::UNITY_GAIN)return "Unity Gain";
    else if(_type==Biquad::Type::ONEPOLE_LOWPASS)return "One-Pole Low Pass";
    else if(_type==Biquad::Type::ONEPOLE_HIGHPASS)return "One-Pole High Pass";
    else if(_type==Biquad::Type::CUSTOM)return "Custom";
    return "Peaking";
}

typedef struct calibrationPoint_s{
    uint32_t id;
    Biquad::Type type;
    int freq;
    ///[Applies when type != custom] vvv
    double bw;
    double gain;
    ///[Applies when type == custom] vvv
    customFilter_t custom441;
    customFilter_t custom48;
}calibrationPoint_t;

#endif // FILTERTYPES_H
