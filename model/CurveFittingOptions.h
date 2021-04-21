#ifndef CURVEFITTINGOPTIONS_H
#define CURVEFITTINGOPTIONS_H

#include <cinttypes>

class CurveFittingOptions {
public:
    CurveFittingOptions(double* _frequency, double* _gain, int _count,
                        uint64_t _rng_seed, float _rng_density_dist){
        frequency = _frequency;
        gain = _gain;
        count = _count;
        rng_seed = _rng_seed;
        rng_density_dist = _rng_density_dist;
    }

    double *frequencyData() const;
    double *gainData() const;
    uint64_t seed() const;
    float probabilityDensityDist() const;
    int dataCount() const;

private:
    double* frequency;
    double* gain;
    int count;
    uint64_t rng_seed;
    float rng_density_dist;

};

inline double *CurveFittingOptions::frequencyData() const
{
    return frequency;
}

inline double *CurveFittingOptions::gainData() const
{
    return gain;
}

inline uint64_t CurveFittingOptions::seed() const
{
    return rng_seed;
}

inline float CurveFittingOptions::probabilityDensityDist() const
{
    return rng_density_dist;
}

inline int CurveFittingOptions::dataCount() const
{
    return count;
}

#endif // CURVEFITTINGOPTIONS_H
