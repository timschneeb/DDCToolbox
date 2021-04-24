#ifndef CURVEFITTINGOPTIONS_H
#define CURVEFITTINGOPTIONS_H

#include <cinttypes>
#include <utility>

typedef std::pair<double,double> DoubleRange;

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
                        uint64_t _rng_seed, ProbDensityFunc _rng_density_dist,
                        DoubleRange _obc_freq, DoubleRange _obc_q, DoubleRange _obc_gain){
        algorithm_type = _algorithm_type;
        frequency = _frequency;
        gain = _gain;
        count = _count;
        rng_seed = _rng_seed;
        rng_density_dist = _rng_density_dist;
        obc_freq = _obc_freq;
        obc_q = _obc_q;
        obc_gain = _obc_gain;
    }

    double *frequencyData() const;
    double *targetData() const;
    uint64_t seed() const;
    ProbDensityFunc probabilityDensityFunc() const;
    unsigned int dataCount() const;
    AlgorithmType algorithmType() const;

    DoubleRange obcFrequencyRange() const;
    DoubleRange obcQRange() const;
    DoubleRange obcGainRange() const;

private:
    double* frequency;
    double* gain;
    unsigned int count;
    uint64_t rng_seed;
    ProbDensityFunc rng_density_dist;
    AlgorithmType algorithm_type;
    DoubleRange obc_freq;
    DoubleRange obc_q;
    DoubleRange obc_gain;
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

inline DoubleRange CurveFittingOptions::obcFrequencyRange() const
{
    return obc_freq;
}

inline DoubleRange CurveFittingOptions::obcQRange() const
{
    return obc_q;
}

inline DoubleRange CurveFittingOptions::obcGainRange() const
{
    return obc_gain;
}

#endif // CURVEFITTINGOPTIONS_H
