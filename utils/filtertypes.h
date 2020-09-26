#ifndef FILTERTYPES_H
#define FILTERTYPES_H
#include "Biquad.h"

typedef struct calibrationPoint_s{
    uint32_t id;
    FilterType type;
    int freq;
    ///[Applies when type != custom] vvv
    double bw;
    double gain;
    ///[Applies when type == custom] vvv
    customFilter_t custom441;
    customFilter_t custom48;
}calibrationPoint_t;

#endif // FILTERTYPES_H
