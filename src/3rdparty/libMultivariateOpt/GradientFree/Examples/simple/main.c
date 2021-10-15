#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "../../gradfreeOpt/gradfreeOpt.h"
// Optimize simple function
double optSin(double *x, void *usd)
{
	return sin(*x);
}
int main()
{
	unsigned int i, j;
	unsigned int K = 5;
	unsigned int N = 3;
	pcg32x2_random_t PRNG;
	pcg32x2_srandom_r(&PRNG, 36u, 84u, 54u, 54u);
	unsigned int dim = 1;
	// 起始答案
	double initialLowFc = -2;
	double initialUpFc = 2;
	double *initialAns = (double*)malloc(K * N * dim * sizeof(double));
	for (i = 0; i < K * N; i++)
	{
		initialAns[i * dim] = c_rand(&PRNG) * (initialUpFc - initialLowFc) + initialLowFc;
	}
	double *low = (double*)malloc(dim * sizeof(double));
	double *up = (double*)malloc(dim * sizeof(double));
	for (j = 0; j < dim; j++)
	{
		low[j] = -2.0;
		up[j] = 2.0;
	}
	double(*pdf1)(pcg32x2_random_t*) = rand_tri_pcg32x2; // Three supported PDF, randn_pcg32x2, rand_tri_pcg32x2, rand_hann
	void *userdataPtr = 0;
	double *gbestDE = (double*)malloc(dim * sizeof(double));
	double probiBound = 0.99;
	double gmin = differentialEvolution(optSin, userdataPtr, initialAns, K, N, probiBound, dim, low, up, 150, gbestDE, &PRNG, pdf1, 0, 0);
	double *gbestfminsearch = (double*)malloc(dim * sizeof(double));
	double fval = fminsearchbnd(optSin, userdataPtr, gbestDE, low, up, dim, 1e-8, 1e-8, 150, gbestfminsearch, 0, 0, 0);
	double *gbestFPA = (double*)malloc(dim * sizeof(double));
	double gmin2 = flowerPollination(optSin, userdataPtr, initialAns, low, up, dim, K * N, 0.1, 0.05, 150, gbestFPA, &PRNG, pdf1, 0, 0);
	double *gbestCHIO = (double*)malloc(dim * sizeof(double));
	unsigned int maxSolSurviveEpoch = 100;
	unsigned int C0 = 1;
	double spreadingRate = 0.1;
	double chioFval = CHIO(optSin, userdataPtr, initialAns, K * N, maxSolSurviveEpoch, C0, spreadingRate, dim, low, up, 150, gbestCHIO, &PRNG, 0, 0);
	printf("%1.14lf %1.14lf %1.14lf %1.14lf\n", gmin, fval, gmin2, chioFval);
	for (i = 0; i < dim; i++)
		printf("%1.14lf,%1.14lf,%1.14lf,%1.14lf", gbestDE[i], gbestfminsearch[i], gbestFPA[i], gbestCHIO[i]);
	free(initialAns);
	free(low);
	free(up);
	free(gbestDE);
	free(gbestfminsearch);
	free(gbestFPA);
	free(gbestCHIO);
	return 0;
}