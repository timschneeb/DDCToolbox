#ifndef CURVEFITTINGOPTIONS_H
#define CURVEFITTINGOPTIONS_H

class CurveFittingOptions {
public:
    CurveFittingOptions(const float* _frequency, const float* _gain,
                        long _rng_seed, float _rng_density_dist){
        frequency = _frequency;
        gain = _gain;
        rng_seed = _rng_seed;
        rng_density_dist = _rng_density_dist;
    }

    const float *frequencyData() const;
    const float *gainData() const;
    long seed() const;
    float probabilityDensityDist() const;

private:
    const float* frequency;
    const float* gain;
    long rng_seed;
    float rng_density_dist;

};

inline const float *CurveFittingOptions::frequencyData() const
{
    return frequency;
}

inline const float *CurveFittingOptions::gainData() const
{
    return gain;
}

inline long CurveFittingOptions::seed() const
{
    return rng_seed;
}

inline float CurveFittingOptions::probabilityDensityDist() const
{
    return rng_density_dist;
}

#endif // CURVEFITTINGOPTIONS_H
