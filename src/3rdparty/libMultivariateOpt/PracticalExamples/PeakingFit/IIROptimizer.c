#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vld.h>
#include <kann.h>
#include <vld.h>
#include "../../GradientFree/gradfreeOpt/gradfreeOpt.h"
#include "../../libgenmath/interpolation2.h"
#include "../../libgenmath/peakfinder.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <float.h>
void readFile(char *filename, void *dataPtr);
void* allocate_sample_vector();
unsigned int inuse_sample_vector(void *sv);
double* getPtr_sample_vector(void *sv);
void free_sample_vector(void *sv);
void derivative(double *x, unsigned int n, unsigned int NorderDerivative, double *dx, double *dif);
unsigned int getAuditoryBandLen(const int xLen, double avgBW);
void initInterpolationList(unsigned int *indexList, double *levels, double avgBW, unsigned int fcLen, unsigned int xLim);
typedef struct
{
	double fs;
    double *phi, *target, *tmp, *weights;
    unsigned int gridSize, numBands;
    double last_mse;
    void* user_data;
} optUserdata;
void validatePeaking(double gain, double fc, double Q, double fs_tf, double *b0, double *b1, double *b2, double *a1, double *a2)
{
	double A = pow(10.0, gain / 40.0);
	double w0 = 2.0 * M_PI * pow(10.0, fc) / fs_tf;
	double sn = sin(w0);
	double cn = cos(w0);
	double alpha = sn / (2.0 * Q);

	double a0_pk = 1.0 + alpha / A;
	*a1 = (-2.0 * cn) / a0_pk;
	*a2 = (1.0 - alpha / A) / a0_pk;

	*b0 = (1.0 + alpha * A) / a0_pk;
	*b1 = (-2.0 * cn) / a0_pk;
	*b2 = (1.0 - alpha * A) / a0_pk;
}
void validateMagCal(double b0, double b1, double b2, double a1, double a2, double *phi, int len, double fs_tf, double *out)
{
	for (int i = 0; i < len; i++)
	{
		double termSqr1 = b0 + b1 + b2;
		double termSqr2 = 1.0 + a1 + a2;
		double term1 = (termSqr1 * termSqr1) + ((b0 * b2 * phi[i]) - (b1 * (b0 + b2) + (4.0 * b0 * b2))) * phi[i];
		double term2 = (termSqr2 * termSqr2) + ((a2 * phi[i]) - (a1 * (1.0 + a2) + (4.0 * a2))) * phi[i];
		if (term1 < DBL_EPSILON)
			term1 = DBL_EPSILON;
		if (term2 < DBL_EPSILON)
			term2 = DBL_EPSILON;
		double eq_op = 10.0 * (log10(term1) - log10(term2));
		out[i] += eq_op;
	}
}
double peakingCostFunctionMap(double *x, void *usd)
{
	optUserdata *userdata = (optUserdata*)usd;
	double *fc = x;
	double *Q = x + userdata->numBands;
	double *gain = x + userdata->numBands * 2;
	double b0, b1, b2, a1, a2;
	memset(userdata->tmp, 0, userdata->gridSize * sizeof(double));
	for (unsigned int i = 0; i < userdata->numBands; i++)
	{
		validatePeaking(gain[i], fc[i], Q[i], userdata->fs, &b0, &b1, &b2, &a1, &a2);
		validateMagCal(b0, b1, b2, a1, a2, userdata->phi, userdata->gridSize, userdata->fs, userdata->tmp);
		//printf("Band: %d, %1.14lf, %1.14lf, %1.14lf\n", i + 1, fc[i], Q[i], gain[i]);
	}
	double meanAcc = 0.0;
	for (unsigned int i = 0; i < userdata->gridSize; i++)
	{
		double error = (userdata->tmp[i] - userdata->target[i]) * userdata->weights[i];
		meanAcc += error * error;
	}
	meanAcc = meanAcc / (double)userdata->gridSize;
	return meanAcc;
}
unsigned int* peakfinder_wrapper(double *x, unsigned int n, double sel, unsigned int extrema, unsigned int *numPeaks)
{
	unsigned int *peakInds = (unsigned int*)malloc(n * sizeof(unsigned int));
	*numPeaks = peakfinder(n, x, sel, extrema, peakInds); // 1u = maxima, 0u = minima
	return peakInds;
}
void optimizationHistoryCallback(void *hostData, unsigned int n, double *currentResult, double *currentFval)
{
	optUserdata *userdata = (optUserdata*)hostData;

	// Plot *currentFval

	// Compute magnitude response from optimizer
	double *fc = currentResult;
	double *Q = currentResult + userdata->numBands;
	double *gain = currentResult + userdata->numBands * 2;
	double b0, b1, b2, a1, a2;
	memset(userdata->tmp, 0, userdata->gridSize * sizeof(double));
	for (unsigned int i = 0; i < userdata->numBands; i++)
	{
		validatePeaking(gain[i], fc[i], Q[i], userdata->fs, &b0, &b1, &b2, &a1, &a2);
		validateMagCal(b0, b1, b2, a1, a2, userdata->phi, userdata->gridSize, userdata->fs, userdata->tmp);
	}

	// Determine if result changed using fuzzy compare
	// Plot userdata->tmp

	userdata->last_mse = *currentFval;
	printf("");
}
double buildDAGNodesDifferentiate(pcg32x2_random_t *PRNG, double *gbest, double *target, double *phi, double *weights, int n_bands, int n_out, double fs, int lockFc, unsigned int iterations, double learningRate, double lrDecayRate, double *low, double *up, void(*optStatus)(void*, unsigned int, double*, double*), void *userdataPtr)
{
	optUserdata *userdata = (optUserdata*)userdataPtr;
	int i, j;
	kad_trap_fe();
	kann_srand(1337);
	int axis = 2; // n_bands * n_out matrix is all we need
	kad_node_t *fsNode = kad_feed(axis, 1, 1);
	fsNode->ext_flag |= KANN_F_IN;
	// n_bands filters, n_out frequency points
	kad_node_t *one = kann_new_leaf(KAD_CONST, 1.0, axis, 1, 1);
	kad_node_t *two = kann_new_leaf(KAD_CONST, 2.0, axis, 1, 1);
	kad_node_t *four = kann_new_leaf(KAD_CONST, 4.0, axis, 1, 1);
	kad_node_t *tenlogTen = kann_new_leaf(KAD_CONST, 10.0 * (1.0 / log(10.0)), axis, 1, 1);
	kad_node_t *minustwo = kann_new_leaf(KAD_CONST, -2.0, axis, 1, 1);
	kad_node_t *twopiArrayConst = kann_new_leaf(KAD_CONST, 2.0 * M_PI, axis, n_bands, 1);
	kad_node_t *fc = kann_new_leaf(lockFc ? KAD_CONST : KAD_VAR, 0.0, axis, n_bands, 1);
	for (i = 0; i < n_bands; i++)
		fc->x[i] = gbest[i];
	kad_node_t *Q = kann_new_leaf(KAD_VAR, 0.0, axis, n_bands, 1);
	for (i = 0; i < n_bands; i++)
		Q->x[i] = gbest[i + n_bands];
	kad_node_t *gain = kann_new_leaf(KAD_VAR, 0.0, axis, n_bands, 1);
	for (i = 0; i < n_bands; i++)
		gain->x[i] = gbest[i + n_bands + n_bands];
	kad_node_t *pregainConst = kann_new_leaf(KAD_CONST, (double)(1.0 / 40.0), axis, n_bands, 1);
	/*kad_node_t *tenConst = kann_new_leaf(KAD_CONST, 10.0f, axis, n_bands, 1);
	kad_node_t *A = kad_pow(tenConst, kad_mul(gain, pregainConst));
	kad_node_t *t2 = kad_div(kad_pow(tenConst, fc), fsNode); // (10.0 .^ fc) ./ fs*/
	kad_node_t *A = kad_pow10x(kad_mul(gain, pregainConst));
	kad_node_t *t2 = kad_div(kad_pow10x(fc), fsNode); // (10.0 .^ fc) ./ fsNode
	kad_node_t *w0 = kad_mul(twopiArrayConst, t2);
	kad_node_t *sn = kad_sin(w0);
	kad_node_t *cn = kad_cos(w0);
	kad_node_t *alpha = kad_div(sn, kad_mul(two, Q));
	kad_node_t *a0_pk = kad_add(one, kad_div(alpha, A));
	kad_node_t *a1 = kad_div(kad_mul(minustwo, cn), a0_pk);
	kad_node_t *a2 = kad_div(kad_sub(one, kad_div(alpha, A)), a0_pk);
	kad_node_t *b0 = kad_div(kad_add(one, kad_mul(alpha, A)), a0_pk);
	kad_node_t *b1 = kad_div(kad_mul(minustwo, cn), a0_pk);
	kad_node_t *b2 = kad_div(kad_sub(one, kad_mul(alpha, A)), a0_pk);

	kad_node_t *term1 = kad_square(kad_add(kad_add(b0, b1), b2));
	kad_node_t *term3 = kad_mul(b1, kad_add(b0, b2));
	kad_node_t *term4 = kad_mul(four, kad_mul(b0, b2));
	kad_node_t *term34 = kad_add(term3, term4);
	kad_node_t *term5 = kad_square(kad_add(one, kad_add(a1, a2)));
	kad_node_t *term7 = kad_mul(a1, kad_add(one, a2));
	kad_node_t *term8 = kad_mul(four, a2);
	kad_node_t *term78 = kad_add(term7, term8);
	kad_node_t *b0_b2 = kad_mul(b0, b2);
	kad_node_t *phi_1d = kann_new_leaf(KAD_CONST, 0.0, axis, 1, n_out);
	for (j = 0; j < n_out; j++)
		phi_1d->x[j] = phi[j];
	kad_node_t **prem1 = (kad_node_t**)malloc(n_bands * sizeof(kad_node_t*));
	for (i = 0; i < n_bands; i++)
	{
		kad_node_t *selectedSlice1 = kad_slice(b0_b2, 0, i, i + 1);
		kad_node_t *term2 = kad_mul(phi_1d, selectedSlice1);
		kad_node_t *selectedSlice2 = kad_slice(term34, 0, i, i + 1);
		kad_node_t *term234phi = kad_mul(kad_sub(term2, selectedSlice2), phi_1d);
		kad_node_t *selectedSlice3 = kad_slice(term1, 0, i, i + 1);
		kad_node_t *term1234phi = kad_log(kad_add(term234phi, selectedSlice3));

		kad_node_t *selectedSlice5 = kad_slice(a2, 0, i, i + 1);
		kad_node_t *term6 = kad_mul(phi_1d, selectedSlice5);
		kad_node_t *selectedSlice6 = kad_slice(term78, 0, i, i + 1);
		kad_node_t *term678phi = kad_mul(kad_sub(term6, selectedSlice6), phi_1d);
		kad_node_t *selectedSlice7 = kad_slice(term5, 0, i, i + 1);
		kad_node_t *term5678phi = kad_log(kad_add(term678phi, selectedSlice7));
		prem1[i] = kad_sub(term1234phi, term5678phi);
	}
	kad_node_t *termX = kad_mul(tenlogTen, kad_concat_array(0, n_bands, prem1));
	free(prem1);
	kad_node_t *weightVector = kann_new_leaf(KAD_CONST, 0.0, axis, 1, n_out);
	for (j = 0; j < n_out; j++)
		weightVector->x[j] = weights[j];
	kad_node_t *eq_op = kad_mul(kad_reduce_sum(termX, 0), weightVector);
	//kad_node_t *eq_op = kad_reduce_sum(termX, 0);
	kad_node_t *loss = kann_layer_mse_raw(eq_op, n_out); // Getting out the summed matrix
	//kad_node_t *loss = kann_layer_mse_raw(termX, n_bands * n_out); // Getting out the internal matrix
	kann_t *network = kann_new(loss, 0);
	// Print matrix
	/*
	const double *res = kann_apply1(network, &fs);
	fp = fopen("dbg.txt", "wb");
	for (i = 0; i < n_bands; i++)
	{
		for (j = 0; j < n_out; j++)
			fprintf(fp, "%1.7f, ", res[i * n_out + j]);
		fprintf(fp, "\n");
	}*/
	/*// Print vector
	fp = fopen("dbg.txt", "wb");
	for (i = 0; i < n_out; i++)
		fprintf(fp, "%1.8f\n", res[i]);*/
	// Parameters
	double *pointerToGainGradient = gain->g;
	double *pointerToQGradient = Q->g;
	double *pointerTofcGradient = fc->g;
	double *pointerToGainVector = gain->x;
	double *pointerToQVector = Q->x;
	double *pointerTofcVector = fc->x;
	int n_var = kann_size_var(network);
	int n_const = kann_size_const(network);
	double lrD = learningRate;
	grad_optimizer state;
	kann_InitRMSprop(&state, n_var, &lrD, 0.9f, 1e-6f);
	double *_x = &fs;
	double *_y = target;
	kann_feed_bind(network, KANN_F_IN, 0, &_x);
	kann_feed_bind(network, KANN_F_TRUTH, 0, &_y);
	kann_switch(network, 1);
	kann_set_batch_size(network, 1);
	memset(network->g, 0, n_var * sizeof(double));
	double *lowestCost = (double*)malloc((lockFc ? (n_var + n_bands) : n_var) * sizeof(double));
	char displayMessage = 1;
	unsigned int portionOfEpoch = iterations / 1000;
	unsigned int cnt = 0;
	double train_cost = DBL_MAX;
	double previousCost;
	for (unsigned int i = 0; i < iterations; ++i)
	{
		previousCost = train_cost;
		train_cost = kann_cost(network, 0, 1);
		if (!cnt)
			lrD *= lrDecayRate;
		cnt++;
		if (cnt >= portionOfEpoch)
			cnt = 0;
		for (j = 0; j < n_bands; j++)
		{
			if (pointerTofcGradient)
			{
				if (isnan(pointerTofcGradient[j]) || isinf(pointerTofcGradient[j]))
					pointerTofcGradient[j] = 0;
			}
			if (isnan(pointerToQGradient[j]) || isinf(pointerToQGradient[j]))
				pointerToQGradient[j] = 0;
			if (isnan(pointerToGainGradient[j]) || isinf(pointerToGainGradient[j]))
				pointerToGainGradient[j] = 0;
		}
		kann_RMSprop(&state, network->g, network->x);
		if (low && up)
		{
			for (j = 0; j < n_bands; j++)
			{
				if (pointerTofcVector[j] < low[j])
					pointerTofcVector[j] = c_rand(PRNG) * (up[j] - low[j]) + low[j];
				if (pointerTofcVector[j] > up[j])
					pointerTofcVector[j] = c_rand(PRNG) * (up[j] - low[j]) + low[j];
				if (pointerToQVector[j] < low[j + n_bands])
					pointerToQVector[j] = low[j + n_bands];
				if (pointerToQVector[j] > up[j + n_bands])
					pointerToQVector[j] = up[j + n_bands];
			}
		}
		if (train_cost < previousCost)
		{
			for (j = 0; j < n_bands; j++)
			{
				lowestCost[j] = fc->x[j];
				lowestCost[j + n_bands] = Q->x[j];
				lowestCost[j + n_bands + n_bands] = gain->x[j];
			}
			if (optStatus)
				optStatus(userdataPtr, n_out, lowestCost, &train_cost);
		}
		if (displayMessage)
			printf("epoch: %d; training cost: %g\n", i + 1, train_cost);
	}
	freeRMSProp(&state);
	memcpy(gbest, lowestCost, (lockFc ? (n_var + n_bands) : n_var) * sizeof(double));
	free(lowestCost);
	kann_switch(network, 0);
	kann_delete(network);
	return train_cost;
}
void smoothSpectral(int gdType, double *flt_freqList, double *target, unsigned int ud_gridSize, double avgBW, double fs, unsigned int *outGridSize, double **out1, double **out2)
{
	unsigned int i;
	unsigned int detailLinearGridLen;
	double *spectrum;
	double *linGridFreq;
	if (!gdType)
	{
		detailLinearGridLen = ud_gridSize;
		spectrum = (double*)malloc(detailLinearGridLen * sizeof(double));
		linGridFreq = (double*)malloc(detailLinearGridLen * sizeof(double));
		for (i = 0; i < detailLinearGridLen; i++)
		{
			spectrum[i] = target[i];
			linGridFreq[i] = i / ((double)detailLinearGridLen) * (fs / 2);
		}
	}
	else
	{
		detailLinearGridLen = max(ud_gridSize, 8192); // Larger the value could be take care large, especially for uniform grid
		spectrum = (double*)malloc(detailLinearGridLen * sizeof(double));
		linGridFreq = (double*)malloc(detailLinearGridLen * sizeof(double));
		for (i = 0; i < detailLinearGridLen; i++)
		{
			double fl = i / ((double)detailLinearGridLen) * (fs / 2);
			spectrum[i] = linearInterpolationNoExtrapolate(fl, flt_freqList, target, ud_gridSize);
			linGridFreq[i] = fl;
		}
	}
	// Init octave grid shrinker
	unsigned int arrayLen = detailLinearGridLen;
	unsigned int fcLen = getAuditoryBandLen(arrayLen, avgBW);
	unsigned int idxLen = fcLen + 1;
	unsigned int *indexList = (unsigned int*)malloc((idxLen << 1) * sizeof(unsigned int));
	double *levels = (double*)malloc((idxLen + 3) * sizeof(double));
	double *multiplicationPrecompute = (double*)malloc(idxLen * sizeof(double));
	size_t virtualStructSize = sizeof(unsigned int) + sizeof(double) + sizeof(unsigned int) + (idxLen << 1) * sizeof(unsigned int) + idxLen * sizeof(double) + (idxLen + 3) * sizeof(double);
	double *shrinkedAxis = (double*)malloc((idxLen + 3) * sizeof(double));
	double reciprocal = 1.0 / arrayLen;
	initInterpolationList(indexList, levels, avgBW, fcLen, arrayLen);
	for (unsigned int i = 0; i < idxLen; i++)
		multiplicationPrecompute[i] = 1.0 / (indexList[(i << 1) + 1] - indexList[(i << 1) + 0]);
	// Do actual axis conversion
	shrinkedAxis[0] = spectrum[0];
	shrinkedAxis[1] = spectrum[1];
	double sum;
	for (i = 0; i < idxLen; i++)
	{
		sum = 0.0;
		for (unsigned int j = indexList[(i << 1) + 0]; j < indexList[(i << 1) + 1]; j++)
			sum += spectrum[j];
		shrinkedAxis[2 + i] = sum * multiplicationPrecompute[i];
	}
	shrinkedAxis[(idxLen + 3) - 1] = shrinkedAxis[(idxLen + 3) - 2];
	double *ascendingIdx = (double*)malloc(arrayLen * sizeof(double));
	for (i = 0; i < arrayLen; i++)
		ascendingIdx[i] = i;
	unsigned int hz18 = 0;
	for (i = 0; i < idxLen + 3; i++)
	{
		double freqIdxUR = levels[hz18 + i] * arrayLen;
		double realFreq = linearInterpolationNoExtrapolate(freqIdxUR, ascendingIdx, linGridFreq, arrayLen);
		hz18 = i;
		if (realFreq >= 18.0)
			break;
	}
	double *newflt_freqList = (double*)malloc((idxLen + 3 - hz18) * sizeof(double));
	double *newTarget = (double*)malloc((idxLen + 3 - hz18) * sizeof(double));
	for (i = 0; i < idxLen + 3 - hz18; i++)
	{
		double freqIdxUR = levels[hz18 + i] * arrayLen;
		newflt_freqList[i] = linearInterpolationNoExtrapolate(freqIdxUR, ascendingIdx, linGridFreq, arrayLen);
		newTarget[i] = shrinkedAxis[hz18 + i];
	}
	free(shrinkedAxis);
	free(ascendingIdx);
	free(linGridFreq);
	*outGridSize = idxLen + 3 - hz18;
	*out1 = newflt_freqList;
	*out2 = newTarget;
	free(levels);
	free(multiplicationPrecompute);
	free(indexList);
	free(spectrum);
}
int main()
{
	// Probability and distribution related setup
	pcg32x2_random_t PRNG;
	pcg32x2_srandom_r(&PRNG, 36u, 84u, 54u, 54u);
	unsigned int finess = 2057;
	double px[7] = { -0.3, 0.0, 0.1, 0.3, 0.5, 0.9, 1.0 };
	double prob[7] = { 10.0, 15, 8, 7, 6, 3, 0.2 };
	double *cdf = (double*)malloc(finess * sizeof(double));
	double *pxi = (double*)malloc(finess * sizeof(double));
	arbitraryPDF(px, prob, 7, cdf, pxi, finess);
	// Convert uniform grid to log grid is recommended
	// Nonuniform grid doesn't necessary to be a log grid, but we can force to make it log
	char forceConvertCurrentGrid2OctaveGrid = 0;
	// Parameter setup start
	char algorithm[] = "SGD + DE"; // DE, SGD, SGD + DE, SGD + CHIO
	const double avgBW = 1.005; // Smaller the value less smooth the output gonna be, of course, don't go too large
	// Common optimizer parameters
	unsigned int stage1Epk = 500; // 5000;
	// SGD + DE, SGD + CHIO
	unsigned int stage2Epk = 5000; // 50000;
	unsigned int stage3Epk = 2000; // 20000;
	// DE only
	unsigned int K = 5;
	unsigned int N = 3;
	double(*pdf1)(pcg32x2_random_t*) = randn_pcg32x2; // Select probability density function that shapes internal random number distribution
	double probiBound = 0.99;
	// SGD only
	double lr1 = 0.01;
	double lr1DecayRate = 0.9999999;
	double lr2 = 0.001;
	double lr2DecayRate = 0.9999999;
	// CHIO only
	unsigned int maxSolSurviveEpoch = 100;
	unsigned int C0 = 6;
	double spreadingRate = 0.05;
	// Parameter setup stop
	// File reading
	FILE *fp;
	void *xAxis = allocate_sample_vector(), *yAxis = allocate_sample_vector();
	readFile("flt_freqList.txt", xAxis);
	readFile("target.txt", yAxis);
	unsigned int i, j;
	double fs = 48000.0;
	unsigned int ud_gridSize = inuse_sample_vector(yAxis);
	double *flt_freqList = (double*)malloc(ud_gridSize * sizeof(double));
	double *target = (double*)malloc(ud_gridSize * sizeof(double));
	memcpy(flt_freqList, getPtr_sample_vector(xAxis), ud_gridSize * sizeof(double));
	memcpy(target, getPtr_sample_vector(yAxis), ud_gridSize * sizeof(double));
	free_sample_vector(xAxis);
	free_sample_vector(yAxis);
	// Detect X axis linearity
	// Assume frequency axis is sorted
	double first = flt_freqList[0];
	double last = flt_freqList[ud_gridSize - 1];
	double stepLinspace = (last - first) / (double)(ud_gridSize - 1);
	double residue = 0.0;
	for (i = 0; i < ud_gridSize; i++)
	{
		double vv = fabs((first + i * stepLinspace) - flt_freqList[i]);
		residue += vv;
	}
	residue = residue / (double)ud_gridSize;
	char gdType;
	if (residue > 10.0) // Allow margin of error, in fact, non-zero residue hint the grid is nonuniform
	{
		gdType = 1;
		printf("Nonuniform grid\n");
	}
	else
	{
		gdType = 0;
		printf("Uniform grid\n");
	}
	if (forceConvertCurrentGrid2OctaveGrid)
	{
		unsigned int oGridSize;
		double *out1;
		double *out2;
		smoothSpectral(gdType, flt_freqList, target, ud_gridSize, avgBW, fs, &oGridSize, &out1, &out2);
		free(flt_freqList);
		free(target);
		flt_freqList = out1;
		target = out2;
	}
	// Bound constraints
	double lowFc = 10; // Hz
	double upFc = fs / 2 - 1; // Hz
	double lowQ = 0.000001; // 0.000001 - 1000, higher == shaper the filter
	double upQ = 1000; // 0.000001 - 1000, higher == shaper the filter
	double lowGain = target[0]; // dB
	double upGain = target[0]; // dB
	for (i = 1; i < ud_gridSize; i++)
	{
		if (target[i] < lowGain)
			lowGain = target[i];
		if (target[i] > upGain)
			upGain = target[i];
	}
	lowGain -= 32.0;
	upGain += 32.0;
	unsigned int numMaximas, numMinimas;
	unsigned int *maximaIndex = peakfinder_wrapper(target, ud_gridSize, 0.1, 1, &numMaximas);
	unsigned int *minimaIndex = peakfinder_wrapper(target, ud_gridSize, 0.1, 0, &numMinimas);
	unsigned int numBands = numMaximas + numMinimas;
	// Model complexity code start
	double modelComplexity = 100.0f; // 10% - 120%
	unsigned int oldBandNum = numBands;
	numBands = (unsigned int)round(numBands * modelComplexity / 100.0);
	double *flt_fc = (double*)malloc(numBands * sizeof(double));
	for (i = 0; i < numMaximas; i++)
		flt_fc[i] = flt_freqList[maximaIndex[i]];
	for (i = numMaximas; i < ((numBands < oldBandNum) ? numBands : oldBandNum); i++)
		flt_fc[i] = flt_freqList[minimaIndex[i - numMaximas]];
	free(maximaIndex);
	free(minimaIndex);
	double *weights = (double*)malloc(ud_gridSize * sizeof(double));
	for (i = 0; i < ud_gridSize; i++)
	{
		if (fabs(target[i]) > 1.0)
			weights[i] = 1.0;
		else
			weights[i] = 0.2;
	}
	if (numBands > oldBandNum)
	{
		for (i = oldBandNum; i < numBands; i++)
			flt_fc[i] = c_rand(&PRNG) * (upFc - lowFc) + lowFc;
	}
	// Model complexity code end
	unsigned int *idx = (unsigned int*)malloc(numBands * sizeof(unsigned int));
	sort(flt_fc, numBands, idx);
	// Initial value must not get out-of-bound, more importantly.
	// If value go too extreme, NaN will happens when frequency place on either too close to DC and touching/beyond Nyquist
	for (i = 0; i < numBands; i++)
	{
		if (flt_fc[i] < lowFc)
			flt_fc[i] = lowFc;
		if (flt_fc[i] > upFc)
			flt_fc[i] = upFc;
	}
	// Naive way to prevent nearby filters go too close, because they could contribute nothing
	double smallestJump = 0.0;
	double lowestFreq2Gen = 200.0;
	double highestFreq2Gen = 14000.0;
	double *dFreqDiscontin = (double*)malloc(numBands * sizeof(double));
	double *dif = (double*)malloc(numBands * sizeof(double));
	unsigned int cnt = 0;
	while (smallestJump <= 20.0)
	{
		derivative(flt_fc, numBands, 1, dFreqDiscontin, dif);
		unsigned int smIdx;
		smallestJump = minArray(dFreqDiscontin, numBands, &smIdx);
		double newFreq = c_rand(&PRNG) * (highestFreq2Gen - lowestFreq2Gen) + lowestFreq2Gen;
		flt_fc[smIdx] = newFreq;
		sort(flt_fc, numBands, idx);
		cnt++;
		if (cnt > 50)
			break;
	}
	free(idx);
	double *flt_peak_g = (double*)malloc(numBands * sizeof(double));
	for (i = 0; i < numBands; i++)
		flt_peak_g[i] = npointWndFunction(flt_fc[i], flt_freqList, target, ud_gridSize);
	double *initialQ = (double*)malloc(numBands * sizeof(double));
	for (i = 0; i < numBands; i++)
	{
		initialQ[i] = 1.0 + npointWndFunction(c_rand(&PRNG), cdf, pxi, finess);
		flt_fc[i] = log10(flt_fc[i]);
	}
	free(cdf);
	free(pxi);
	double *phi = (double*)malloc(ud_gridSize * sizeof(double));
	for (i = 0; i < ud_gridSize; i++)
	{
		double term1 = sin(M_PI * flt_freqList[i] / fs);
		phi[i] = 4.0 * term1 * term1;
	}
	unsigned int dim = numBands * 3;
	double *low = (double*)malloc(dim * sizeof(double));
	double *up = (double*)malloc(dim * sizeof(double));
	for (j = 0; j < numBands; j++)
	{
		low[j] = log10(lowFc); low[numBands + j] = lowQ; low[numBands * 2 + j] = lowGain;
		up[j] = log10(upFc); up[numBands + j] = upQ; up[numBands * 2 + j] = upGain;
	}
	int n_bands = numBands;
	int n_out = ud_gridSize;
	fp = fopen("initAns.txt", "wb");
	for (i = 0; i < n_bands; i++)
		fprintf(fp, "%1.8f,", flt_fc[i]);
	for (i = 0; i < n_bands; i++)
		fprintf(fp, "%1.8f,", initialQ[i]);
	for (i = 0; i < n_bands - 1; i++)
		fprintf(fp, "%1.8f,", flt_peak_g[i]);
	fprintf(fp, "%1.8f", flt_peak_g[n_bands - 1]);
	fclose(fp);
	fp = fopen("formatedTarget.txt", "wb");
	for (i = 0; i < n_out - 1; i++)
		fprintf(fp, "%1.8f,", target[i]);
	fprintf(fp, "%1.8f", target[n_out - 1]);
	fclose(fp);
	double *gbest = (double*)malloc(n_bands * 3 * sizeof(double));
	idx = (unsigned int*)malloc(n_bands * sizeof(unsigned int));
	double *sortedOptVector = (double*)malloc(numBands * 3 * sizeof(double));
	// Optimization
	// Local minima avoidance
	double initialLowGain = -1.5;
	double initialUpGain = 1.5;
	double initialLowQ = -0.5;
	double initialUpQ = 0.5;
	double initialLowFc = -log10(2);
	double initialUpFc = log10(2);
	double *initialAns = (double*)malloc(K * N * dim * sizeof(double));
	for (i = 0; i < K * N; i++)
	{
		for (j = 0; j < numBands; j++)
		{
			initialAns[i * dim + j] = flt_fc[j] + c_rand(&PRNG) * (initialUpFc - initialLowFc) + initialLowFc;
			initialAns[i * dim + numBands + j] = initialQ[j] + c_rand(&PRNG) * (initialUpQ - initialLowQ) + initialLowQ;
			initialAns[i * dim + numBands * 2 + j] = flt_peak_g[j] + c_rand(&PRNG) * (initialUpGain - initialLowGain) + initialLowGain;
		}
	}
	for (i = 0; i < n_bands; i++)
	{
		gbest[i] = flt_fc[i];
		gbest[i + n_bands] = initialQ[i];
		gbest[i + n_bands + n_bands] = flt_peak_g[i];
	}
	free(flt_fc);
	free(initialQ);
	free(flt_peak_g);
	void(*optStatus)(void*, unsigned int, double*, double*) = optimizationHistoryCallback;
	double *tmpDat = (double*)malloc(ud_gridSize * sizeof(double));
	optUserdata userdat;
	userdat.fs = fs;
	userdat.numBands = numBands;
	userdat.phi = phi;
	userdat.target = target;
	userdat.tmp = tmpDat;
	userdat.gridSize = ud_gridSize;
	userdat.weights = weights;
	void *userdataPtr = (void*)&userdat;
	if (!strcmp(algorithm, "DE"))
	{
		double deFval = differentialEvolution(peakingCostFunctionMap, userdataPtr, initialAns, K, N, probiBound, dim, low, up, stage1Epk, gbest, &PRNG, pdf1, optStatus, userdataPtr);
		printf("DE cost: %1.8lf\n", deFval);
	}
	else if (!strcmp(algorithm, "SGD"))
	{
		double cost1 = buildDAGNodesDifferentiate(&PRNG, gbest, target, phi, weights, n_bands, n_out, fs, 1, stage1Epk, lr1, lr1DecayRate, low, up, optStatus, userdataPtr);
		printf("SGD stage 1 cost: %1.8lf\n", cost1);
		double cost2 = buildDAGNodesDifferentiate(&PRNG, gbest, target, phi, weights, n_bands, n_out, fs, 0, stage2Epk, lr2, lr2DecayRate, low, up, optStatus, userdataPtr);
		printf("SGD stage 2 cost: %1.8lf\n", cost2);
	}
	else if (!strcmp(algorithm, "CHIO"))
	{
		double fval = CHIO(peakingCostFunctionMap, userdataPtr, initialAns, K * N, maxSolSurviveEpoch, C0, spreadingRate, dim, low, up, stage1Epk, gbest, &PRNG, optStatus, userdataPtr);
		printf("CHIO cost: %1.8lf\n", fval);
	}
	else if (!strcmp(algorithm, "SGD + DE"))
	{
		double cost1 = buildDAGNodesDifferentiate(&PRNG, gbest, target, phi, weights, n_bands, n_out, fs, 1, stage1Epk, lr1, lr1DecayRate, low, up, optStatus, userdataPtr);
		printf("SGD stage 1 cost: %1.8lf\n", cost1);
		double cost2 = buildDAGNodesDifferentiate(&PRNG, gbest, target, phi, weights, n_bands, n_out, fs, 0, stage2Epk, lr2, lr2DecayRate, low, up, optStatus, userdataPtr);
		printf("SGD stage 2 cost: %1.8lf\n", cost2);
		for (j = 0; j < numBands; j++)
		{
			initialAns[j] = gbest[j];
			initialAns[numBands + j] = gbest[j + numBands];
			initialAns[numBands * 2 + j] = gbest[j + numBands * 2];
		}
		for (i = 1; i < K * N; i++)
		{
			for (j = 0; j < numBands; j++)
			{
				initialAns[i * dim + j] = gbest[j] + c_rand(&PRNG) * (initialUpFc - initialLowFc) + initialLowFc;
				initialAns[i * dim + numBands + j] = gbest[j + numBands] + c_rand(&PRNG) * (initialUpQ - initialLowQ) + initialLowQ;
				initialAns[i * dim + numBands * 2 + j] = gbest[j + numBands * 2] + c_rand(&PRNG) * (initialUpGain - initialLowGain) + initialLowGain;
			}
		}
		double deFval = differentialEvolution(peakingCostFunctionMap, userdataPtr, initialAns, K, N, 1.0, dim, low, up, stage3Epk, gbest, &PRNG, pdf1, optStatus, userdataPtr);
		printf("DE cost: %1.8lf\n", deFval);
	}
	else if (!strcmp(algorithm, "SGD + CHIO"))
	{
		double cost1 = buildDAGNodesDifferentiate(&PRNG, gbest, target, phi, weights, n_bands, n_out, fs, 1, stage1Epk, lr1, lr1DecayRate, low, up, optStatus, userdataPtr);
		printf("SGD stage 1 cost: %1.8lf\n", cost1);
		double cost2 = buildDAGNodesDifferentiate(&PRNG, gbest, target, phi, weights, n_bands, n_out, fs, 0, stage2Epk, lr2, lr2DecayRate, low, up, optStatus, userdataPtr);
		printf("SGD stage 2 cost: %1.8lf\n", cost2);
		for (j = 0; j < numBands; j++)
		{
			initialAns[j] = gbest[j];
			initialAns[numBands + j] = gbest[j + numBands];
			initialAns[numBands * 2 + j] = gbest[j + numBands * 2];
		}
		for (i = 1; i < K * N; i++)
		{
			for (j = 0; j < numBands; j++)
			{
				initialAns[i * dim + j] = gbest[j] + c_rand(&PRNG) * (initialUpFc - initialLowFc) + initialLowFc;
				initialAns[i * dim + numBands + j] = gbest[j + numBands] + c_rand(&PRNG) * (initialUpQ - initialLowQ) + initialLowQ;
				initialAns[i * dim + numBands * 2 + j] = gbest[j + numBands * 2] + c_rand(&PRNG) * (initialUpGain - initialLowGain) + initialLowGain;
			}
		}
		double fval = CHIO(peakingCostFunctionMap, userdataPtr, initialAns, K * N, maxSolSurviveEpoch, C0, spreadingRate, dim, low, up, stage3Epk, gbest, &PRNG, optStatus, userdataPtr);
		printf("CHIO cost: %1.8lf\n", fval);
	}
	free(tmpDat);
	free(initialAns);
	sort(gbest, numBands, idx);
	for (i = 0; i < numBands; i++)
	{
		sortedOptVector[i] = gbest[i];
		sortedOptVector[numBands + i] = gbest[numBands + idx[i]];
		sortedOptVector[numBands + numBands + i] = gbest[numBands + numBands + idx[i]];
	}
	free(gbest);
	free(idx);
	fp = fopen("optimizedSorted2.txt", "wb");
	for (i = 0; i < numBands * 3 - 1; i++)
		fprintf(fp, "%1.8lf,", sortedOptVector[i]);
	fprintf(fp, "%1.8lf", sortedOptVector[numBands * 3 - 1]);
	fclose(fp);
	free(sortedOptVector);
	// Free stuff
	free(weights);
	free(dFreqDiscontin);
	free(dif);
	free(low);
	free(up);
	free(phi);

	free(flt_freqList);
	free(target);
	system("pause");
	return 0;
}