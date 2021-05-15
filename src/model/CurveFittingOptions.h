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
        AT_CHIO = 3,
        AT_HYBRID_DE_FMIN = 4,
        AT_HYBRID_FLOWER_FMIN = 5,
        AT_HYBRID_CHIO_FMIN = 6
    };

    enum ProbDensityFunc {
        PDF_RANDN_PCG32X2 = 0,
        PDF_RAND_TRI_PCG32X2 = 1,
        PDF_RAND_HANN = 2
    };

    CurveFittingOptions(AlgorithmType _algorithm_type, double* _frequency, double* _gain, int _count,
                        uint64_t _rng_seed, ProbDensityFunc _rng_density_dist,
                        DoubleRange _obc_freq, DoubleRange _obc_q, DoubleRange _obc_gain,
                        bool _force_oct_grid, unsigned int _iterations, unsigned int _iterations2, double _avgbw,
                        double _pop_k, double _pop_n,
                        bool _fmin_dimension_adaptive, double _de_probibound, double _flower_pcond, double _flower_weight_step,
                        unsigned int _chio_max_sol_survive_epoch, unsigned int _chio_c0, double _chio_spreading_rate, bool _invert_gain,
                        float _model_complexity){
        algorithm_type = _algorithm_type;
        frequency = _frequency;
        gain = _gain;
        count = _count;
        rng_seed = _rng_seed;
        rng_density_dist = _rng_density_dist;
        obc_freq = _obc_freq;
        obc_q = _obc_q;
        obc_gain = _obc_gain;
        force_oct_grid = _force_oct_grid;
        iterations_count = _iterations;
        iterations2_count = _iterations2;
        avgbw = _avgbw;
        pop_k = _pop_k;
        pop_n = _pop_n;
        fmin_dimension_adaptive = _fmin_dimension_adaptive;
        de_probibound = _de_probibound;
        flower_pcond = _flower_pcond;
        flower_weight_step = _flower_weight_step;
        chio_max_sol_survive_epoch = _chio_max_sol_survive_epoch;
        chio_c0 = _chio_c0;
        chio_spreading_rate = _chio_spreading_rate;
        invert_gain = _invert_gain;
        model_complexity = _model_complexity;
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

    bool forceLogOctGrid() const;
    double averageBandwidth() const;

    unsigned int iterations() const;
    unsigned int iterationsSecondary() const;

    double populationK() const;
    double populationN() const;

    bool fminDimensionAdaptive() const;
    double deProbiBound() const;
    double flowerPCond() const;
    double flowerWeightStep() const;

    unsigned int chioMaxSolSurviveEpoch() const;
    unsigned int chioC0() const;
    double chioSpreadingRate() const;

    bool invertGain() const;

    float modelComplexity() const;

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
    bool force_oct_grid;
    unsigned int iterations_count;
    unsigned int iterations2_count;
    double avgbw;
    double pop_k;
    double pop_n;
    bool fmin_dimension_adaptive;
    double de_probibound;
    double flower_pcond;
    double flower_weight_step;
    unsigned int chio_max_sol_survive_epoch;
    unsigned int chio_c0;
    double chio_spreading_rate;
    bool invert_gain;
    float model_complexity;

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

inline bool CurveFittingOptions::forceLogOctGrid() const
{
    return force_oct_grid;
}

inline unsigned int CurveFittingOptions::iterations() const
{
    return iterations_count;
}

inline unsigned int CurveFittingOptions::iterationsSecondary() const
{
    return iterations2_count;
}

inline double CurveFittingOptions::populationK() const
{
    return pop_k;
}

inline double CurveFittingOptions::populationN() const
{
    return pop_n;
}

inline bool CurveFittingOptions::fminDimensionAdaptive() const
{
    return fmin_dimension_adaptive;
}

inline double CurveFittingOptions::deProbiBound() const
{
    return de_probibound;
}

inline double CurveFittingOptions::flowerPCond() const
{
    return flower_pcond;
}

inline double CurveFittingOptions::flowerWeightStep() const
{
    return flower_weight_step;
}

inline unsigned int CurveFittingOptions::chioMaxSolSurviveEpoch() const
{
    return chio_max_sol_survive_epoch;
}

inline unsigned int CurveFittingOptions::chioC0() const
{
    return chio_c0;
}

inline double CurveFittingOptions::chioSpreadingRate() const
{
    return chio_spreading_rate;
}

inline bool CurveFittingOptions::invertGain() const
{
    return invert_gain;
}

inline float CurveFittingOptions::modelComplexity() const
{
    return model_complexity;
}

inline double CurveFittingOptions::averageBandwidth() const
{
    return avgbw;
}

#endif // CURVEFITTINGOPTIONS_H
