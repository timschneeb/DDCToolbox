#ifndef CURVEFITTINGOPTIONS_H
#define CURVEFITTINGOPTIONS_H

#include <cinttypes>

class CurveFittingOptions {
public:
    enum AlgorithmType {
        AT_DIFF_EVOLUTION = 0,
        AT_FMINSEARCHBND = 1,
        AT_FLOWERPOLLINATION = 2,
        AT_HYDRID_DE_FMIN = 3
    };

    enum ProbDensityFunc {
        PDF_RANDN_PCG32X2 = 0,
        PDF_RAND_TRI_PCG32X2 = 1,
        PDF_RAND_HANN = 2
    };

    CurveFittingOptions(AlgorithmType _algorithm_type, double* _frequency, double* _gain, int _count,
                        uint64_t _rng_seed, ProbDensityFunc _rng_density_dist){
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
    ProbDensityFunc probabilityDensityFunc() const;
    unsigned int dataCount() const;
    AlgorithmType algorithmType() const;

private:
    double* frequency;
    double* gain;
    unsigned int count;
    uint64_t rng_seed;
    ProbDensityFunc rng_density_dist;
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

inline CurveFittingOptions::ProbDensityFunc CurveFittingOptions::probabilityDensityFunc() const
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
