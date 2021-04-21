#ifndef CURVEFITTINGOPTIONS_H
#define CURVEFITTINGOPTIONS_H

#include <cinttypes>

class CurveFittingOptions {
public:
    enum AlgorithmType {
        AT_DIFF_EVOLUTION,
        AT_FMINSEARCHBND
    };

    CurveFittingOptions(AlgorithmType _algorithm_type, double* _frequency, double* _gain, int _count,
                        uint64_t _rng_seed, float _rng_density_dist){
        algorithm_type = _algorithm_type;
        frequency = _frequency;
        gain = _gain;
        count = _count;
        rng_seed = _rng_seed;
        rng_density_dist = _rng_density_dist;
    }

    double *frequencyData() const;
    double *targetData() const;
    uint64_t seed() const;
    float probabilityDensityDist() const;
    unsigned int dataCount() const;
    AlgorithmType algorithmType() const;

private:
    double* frequency;
    double* gain;
    unsigned int count;
    uint64_t rng_seed;
    float rng_density_dist;
    AlgorithmType algorithm_type;
};

inline double *CurveFittingOptions::frequencyData() const
{
    return frequency;
}

inline double *CurveFittingOptions::targetData() const
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

inline unsigned int CurveFittingOptions::dataCount() const
{
    return count;
}

inline CurveFittingOptions::AlgorithmType CurveFittingOptions::algorithmType() const
{
    return algorithm_type;
}

#endif // CURVEFITTINGOPTIONS_H
