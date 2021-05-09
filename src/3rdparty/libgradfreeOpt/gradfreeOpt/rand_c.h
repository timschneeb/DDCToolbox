#include <stdint.h>
typedef struct
{
	uint64_t state; // RNG state.  All values are possible.
	uint64_t inc; // Controls which RNG sequence (stream) is selected. Must *always* be odd.
} pcg32_random_t;
typedef struct
{
	pcg32_random_t gen[2];
} pcg32x2_random_t;
void pcg32x2_srandom_r(pcg32x2_random_t *rng, uint64_t seed1, uint64_t seed2, uint64_t seq1, uint64_t seq2);
double c_rand(pcg32x2_random_t *rng);
double randi(pcg32x2_random_t *rng, double value);
double randn_pcg32x2(pcg32x2_random_t *rng);
double rand_tri_pcg32x2(pcg32x2_random_t *rng);
double rand_hann(pcg32x2_random_t *rng);
void randperm(unsigned int *linearArray, unsigned int n, pcg32x2_random_t *PRNG);