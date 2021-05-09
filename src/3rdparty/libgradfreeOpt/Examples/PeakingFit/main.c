#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <vld.h>
typedef struct
{
	unsigned long long inuse, capacity, grow_num;
	double *data;
} sample_vector;
void init_sample_vector(sample_vector *s, int init_capacity, int grow)
{
	s->data = (double*)malloc(init_capacity * sizeof(double));
	s->inuse = 0;
	s->capacity = init_capacity;
	s->grow_num = grow;
}
void push_back_sample_vector(sample_vector *s, double *x, int lenS)
{
	if ((s->inuse + lenS + (s->grow_num >> 1)) > s->capacity)
	{
		s->capacity += (s->grow_num + lenS);
		s->data = (double*)realloc(s->data, s->capacity * sizeof(double));
	}
	memcpy(s->data + s->inuse, x, lenS * sizeof(double));
	s->inuse += lenS;
}
void clear_sample_vector(sample_vector *s)
{
	s->inuse = 0;
}
void free_sample_vector(sample_vector *s)
{
	free(s->data);
}
#include <math.h>
#include <float.h>
#include "linear_interpolation.h"
//#include <vld.h>
#include "misc.h"
#include "../../gradfreeOpt/gradfreeOpt.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
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
typedef struct
{
	double fs;
	double *phi;
	unsigned int gridSize;
	double *target;
	unsigned int numBands;
	double *tmp;
} optUserdata;
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
		double error = userdata->tmp[i] - userdata->target[i];
		meanAcc += error * error;
	}
	meanAcc = meanAcc / (double)userdata->gridSize;
	return meanAcc;
}
#include "peakfinder.h"
unsigned int* peakfinder_wrapper(double *x, unsigned int n, double sel, unsigned int extrema, unsigned int *numPeaks)
{
	unsigned int *peakInds = (unsigned int*)malloc(n * sizeof(unsigned int));
	*numPeaks = peakfinder(n, x, sel, extrema, peakInds); // 1u = maxima, 0u = minima
	return peakInds;
}
void diff(double *y, double *f, unsigned int sz)
{
	--sz;
	for (unsigned int i = 0; i < sz; i++)
		f[i] = y[i + 1] - y[i];
}
void derivative(double *x, unsigned int n, unsigned int NorderDerivative, double *dx, double *dif)
{
	// DERIVATIVE Compute derivative while preserving dimensions.
	memcpy(dx, x, n * sizeof(double));
	for (unsigned int i = 0; i < NorderDerivative; i++)
	{
		diff(dx, dif, n);
		dx[0] = dif[0];
		for (unsigned int j = 1; j < n - 1; j++)
		{
			dx[j] = (dif[j] + dif[j - 1]) * 0.5;
		}
		dx[n - 1] = dif[n - 2];
	}
}
char* readTextFile(char *filename)
{
	char *buffer = 0;
	long length;
	FILE *textFile = fopen(filename, "rb");
	if (textFile)
	{
		fseek(textFile, 0, SEEK_END);
		length = ftell(textFile);
		fseek(textFile, 0, SEEK_SET);
		buffer = (char*)malloc(length + 1);
		if (buffer)
			fread(buffer, 1, length, textFile);
		fclose(textFile);
		buffer[length] = '\0';
	}
	return buffer;
}
typedef struct
{
	double *display1;
	double display2;
} yourGUI;
void optimizationHistoryCallback(void *hostData, unsigned int n, double *currentResult, double *currentFval)
{
	yourGUI *ptr = (yourGUI*)hostData;
	memcpy(ptr->display1, currentResult, n * sizeof(double));
	ptr->display2 = *currentFval;
}
// Optimize peaking filters(IIR SOS)
unsigned int getAuditoryBandLen(const int xLen, double avgBW)
{
	unsigned int a = 3;
	double step = avgBW;
	unsigned int cnt = 0;
	while (a + step - 1.0 < xLen)
	{
		a = a + (unsigned int)ceil(round(step) * 0.5);
		step = step * avgBW;
		cnt = cnt + 1;
	}
	return cnt;
}
void initInterpolationList(unsigned int *indexList, double *levels, double avgBW, unsigned int fcLen, unsigned int xLim)
{
	unsigned int a = 3;
	double step = avgBW;
	levels[0] = (1.0 - 1.0) / (double)xLim;
	levels[1] = (2.0 - 1.0) / (double)xLim;
	for (int i = 0; i < fcLen; i++)
	{
		unsigned int stepp = (unsigned int)round(step);
		indexList[(i << 1) + 0] = a - 1;
		indexList[(i << 1) + 1] = a + stepp - 1;
		levels[i + 2] = ((double)a + ((double)stepp - 1.0) * 0.5 - 1.0) / (double)xLim;
		a = a + (unsigned int)ceil(((double)stepp) * 0.5);
		step = step * avgBW;
	}
	indexList[(fcLen << 1) + 0] = a - 1;
	indexList[(fcLen << 1) + 1] = xLim;
	levels[2 + fcLen] = ((a + xLim) * 0.5 - 1.0) / (double)xLim;
	levels[2 + fcLen + 1] = (xLim - 1.0) / (double)xLim;
	if (levels[2 + fcLen + 1] == levels[2 + fcLen])
		levels[2 + fcLen] = (levels[2 + fcLen - 1] + levels[2 + fcLen + 1]) * 0.5;
}
int main()
{
	pcg32x2_random_t PRNG;
	pcg32x2_srandom_r(&PRNG, 36u, 84u, 54u, 54u);
	sample_vector xAxis, yAxis;
	init_sample_vector(&xAxis, 128, 64);
	init_sample_vector(&yAxis, 128, 64);
	char *buf = readTextFile("flt_freqList.txt");
	char *err, *p = buf;
	double val;
	while (*p) {
		val = strtod(p, &err);
		if (p == err)
			p++;
		else if ((err == NULL) || (*err == 0))
		{
			push_back_sample_vector(&xAxis, &val, 1);
			break;
		}
		else
		{
			push_back_sample_vector(&xAxis, &val, 1);
			p = err + 1;
		}
	}
	free(buf);
	buf = readTextFile("target.txt");
	p = buf;
	while (*p) {
		val = strtod(p, &err);
		if (p == err)
			p++;
		else if ((err == NULL) || (*err == 0))
		{
			push_back_sample_vector(&yAxis, &val, 1);
			break;
		}
		else
		{
			push_back_sample_vector(&yAxis, &val, 1);
			p = err + 1;
		}
	}
	free(buf);
	unsigned int i, j;
	double fs = 48000.0;
	unsigned int ud_gridSize = yAxis.inuse;
	double *flt_freqList = (double*)malloc(ud_gridSize * sizeof(double));
	double *target = (double*)malloc(ud_gridSize * sizeof(double));
	memcpy(flt_freqList, xAxis.data, ud_gridSize * sizeof(double));
	memcpy(target, yAxis.data, ud_gridSize * sizeof(double));
	free_sample_vector(&xAxis);
	free_sample_vector(&yAxis);
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
	// Convert uniform grid to log grid is recommended
	// Nonuniform grid doesn't necessary to be a log grid, but we can force to make it log
	char forceConvertCurrentGrid2OctaveGrid = 0;
	if (forceConvertCurrentGrid2OctaveGrid)
	{
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
			detailLinearGridLen = max(ud_gridSize, 8192); // Larger the value could be take care large(Especially for uniform grid)
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
		const double avgBW = 1.005; // Smaller the value less smooth the output gonna be, of course, don't go too large
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
		unsigned int i;
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
		ud_gridSize = idxLen + 3 - hz18;
		free(flt_freqList);
		free(target);
		flt_freqList = newflt_freqList;
		target = newTarget;
		free(levels);
		free(multiplicationPrecompute);
		free(indexList);
		free(spectrum);
	}
	// Bound constraints
	double lowFc = 20; // Hz
	double upFc = fs / 2 - 1; // Hz
	double lowQ = 0.2; // 0.01 - 1000, higher shaper the filter
	double upQ = 16; // 0.01 - 1000, higher shaper the filter
	double lowGain = target[0]; // dB
	double upGain = target[0]; // dB
	for (i = 1; i < ud_gridSize; i++)
	{
		if (target[i] < lowGain)
			lowGain = target[i];
		if (target[i] > upGain)
			upGain = target[i];
	}
	lowGain -= 5.0;
	upGain += 5.0;
	unsigned int numMaximas, numMinimas;
	unsigned int *maximaIndex = peakfinder_wrapper(target, ud_gridSize, 0.1, 1, &numMaximas);
	unsigned int *minimaIndex = peakfinder_wrapper(target, ud_gridSize, 0.1, 0, &numMinimas);
	unsigned int numBands = numMaximas + numMinimas;
	double *flt_fc = (double*)malloc(numBands * sizeof(double));
	unsigned int *idx = (unsigned int*)malloc(numBands * sizeof(unsigned int));
	for (i = 0; i < numMaximas; i++)
		flt_fc[i] = flt_freqList[maximaIndex[i]];
	for (i = numMaximas; i < numBands; i++)
		flt_fc[i] = flt_freqList[minimaIndex[i - numMaximas]];
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
	while (smallestJump <= 20.0)
	{
		derivative(flt_fc, numBands, 1, dFreqDiscontin, dif);
		unsigned int smIdx;
		smallestJump = minArray(dFreqDiscontin, numBands, &smIdx);
		double newFreq = c_rand(&PRNG) * (highestFreq2Gen - lowestFreq2Gen) + lowestFreq2Gen;
		flt_fc[smIdx] = newFreq;
		sort(flt_fc, numBands, idx);
	}
	free(idx);
	double *flt_peak_g = (double*)malloc(numBands * sizeof(double));
	for (i = 0; i < numBands; i++)
		flt_peak_g[i] = npointWndFunction(flt_fc[i], flt_freqList, target, ud_gridSize);
	double *initialQ = (double*)malloc(numBands * sizeof(double));
	for (i = 0; i < numBands; i++)
	{
		initialQ[i] = fabs(randn_pcg32x2(&PRNG) * (5 - 0.7) + 0.7);
		flt_fc[i] = log10(flt_fc[i]);
	}
	//for (i = 0; i < numBands; i++)
	//	printf("%1.14lf %1.14lf\n", initialQ[i], flt_fc[i]);
	double *phi = (double*)malloc(ud_gridSize * sizeof(double));
	for (i = 0; i < ud_gridSize; i++)
	{
		double term1 = sin(M_PI * flt_freqList[i] / fs);
		phi[i] = 4.0 * term1 * term1;
	}
	unsigned int dim = numBands * 3;
	// 为起始答案加入波动
	double initialLowGain = -1.5;
	double initialUpGain = 1.5;
	double initialLowQ = -0.5;
	double initialUpQ = 0.5;
	double initialLowFc = -log10(2);
	double initialUpFc = log10(2);
	// Populaton parameters(Optimization)
	unsigned int K = 5;
	unsigned int N = 3;
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
	/*fp = fopen("bm.txt", "wb");
	for (i = 0; i < K * N; i++)
	{
		for (j = 0; j < dim; j++)
			fprintf(fp, "%1.14lf ", initialAns[i * dim + j]);
		fprintf(fp, "\n");
	}
	fclose(fp);*/
	double *low = (double*)malloc(dim * sizeof(double));
	double *up = (double*)malloc(dim * sizeof(double));
	for (j = 0; j < numBands; j++)
	{
		low[j] = log10(lowFc); low[numBands + j] = lowQ; low[numBands * 2 + j] = lowGain;
		up[j] = log10(upFc); up[numBands + j] = upQ; up[numBands * 2 + j] = upGain;
	}
	// Cost function data setup
	double *tmpDat = (double*)malloc(ud_gridSize * sizeof(double));
	optUserdata userdat;
	userdat.fs = fs;
	userdat.numBands = numBands;
	userdat.phi = phi;
	userdat.target = target;
	userdat.tmp = tmpDat;
	userdat.gridSize = ud_gridSize;
	void *userdataPtr = (void*)&userdat;
	double *gbest = (double*)malloc(dim * sizeof(double));
	double *gbest2 = (double*)malloc(dim * sizeof(double));
	// Select probability density function that shapes internal random number distribution
	double(*pdf1)(pcg32x2_random_t*) = randn_pcg32x2;
	// GUI display?
	yourGUI gui;
	gui.display1 = (double*)malloc(dim * sizeof(double));
	void *guiData = (void*)&gui;
	void(*optStatus)(void*, unsigned int, double*, double*) = optimizationHistoryCallback;

	FILE *fp = fopen("sol.txt", "wb");

	char simplexDimensionAdaptive = 0; // Adjustable for fminsearchbnd and Hybrid optimizations

	// DE standalone
	double probiBound = 0.99;
	double deFval = differentialEvolution(peakingCostFunctionMap, userdataPtr, initialAns, K, N, probiBound, dim, low, up, 20000, gbest, &PRNG, pdf1, optStatus, guiData);
	fprintf(fp, "DE\nplot(peakingFunctionMap([");
	for (i = 0; i < dim; i++)
		fprintf(fp, "%1.14lf,", gbest[i]);
	fprintf(fp, "], userdata))\n");

	// DE is relatively robust, we then improve DE result using fminsearchbnd
	double hybOpt1Fval = fminsearchbnd(peakingCostFunctionMap, userdataPtr, gbest, low, up, dim, 1e-8, 1e-8, 10000, gbest2, simplexDimensionAdaptive, optStatus, guiData);
	fprintf(fp, "Hybrid opt 1(DE + fminsearchbnd)\nplot(peakingFunctionMap([");
	for (i = 0; i < dim; i++)
		fprintf(fp, "%1.14lf,", gbest2[i]);
	fprintf(fp, "], userdata))\n");

	// Standalone simplex search
	double fminsFval = fminsearchbnd(peakingCostFunctionMap, userdataPtr, initialAns, low, up, dim, 1e-8, 1e-8, 20000, gbest, simplexDimensionAdaptive, optStatus, guiData);
	fprintf(fp, "fminsearchbnd\nplot(peakingFunctionMap([");
	for (i = 0; i < dim; i++)
		fprintf(fp, "%1.14lf,", gbest[i]);
	fprintf(fp, "], userdata))\n");

	// Flower pollination standalone
	double pCond = 0.1;
	double weightStep = 0.05;
	double fpaFval = flowerPollination(peakingCostFunctionMap, userdataPtr, initialAns, low, up, dim, K * N, pCond, weightStep, 4000, gbest, &PRNG, pdf1, optStatus, guiData);
	fprintf(fp, "fpa\nplot(peakingFunctionMap([");
	for (i = 0; i < dim; i++)
		fprintf(fp, "%1.14lf,", gbest[i]);
	fprintf(fp, "], userdata))\n");

	// Flower pollination could be as robust as DE, user could also improve FPA result using fminsearchbnd too
	double hybOpt2Fval = fminsearchbnd(peakingCostFunctionMap, userdataPtr, gbest, low, up, dim, 1e-8, 1e-8, 10000, gbest2, simplexDimensionAdaptive, optStatus, guiData);
	fprintf(fp, "Hybrid opt 2(FPA + fminsearchbnd)\nplot(peakingFunctionMap([");
	for (i = 0; i < dim; i++)
		fprintf(fp, "%1.14lf,", gbest2[i]);
	fprintf(fp, "], userdata))\n");

	// CHIO standalone
	unsigned int maxSolSurviveEpoch = 100;
	unsigned int C0 = 6;
	double spreadingRate = 0.05;
	double chioFval = CHIO(peakingCostFunctionMap, userdataPtr, initialAns, K * N, maxSolSurviveEpoch, C0, spreadingRate, dim, low, up, 4000, gbest, &PRNG, optStatus, guiData);
	fprintf(fp, "CHIO\nplot(peakingFunctionMap([");
	for (i = 0; i < dim; i++)
		fprintf(fp, "%1.14lf,", gbest[i]);
	fprintf(fp, "], userdata))\n");

	// Improve the result using fminsearchbnd hold for CHIO too
	double hybOpt3Fval = fminsearchbnd(peakingCostFunctionMap, userdataPtr, gbest, low, up, dim, 1e-8, 1e-8, 10000, gbest2, simplexDimensionAdaptive, optStatus, guiData);
	fprintf(fp, "Hybrid opt 3(CHIO + fminsearchbnd)\nplot(peakingFunctionMap([");
	for (i = 0; i < dim; i++)
		fprintf(fp, "%1.14lf,", gbest2[i]);
	fprintf(fp, "], userdata))\n");
	printf("Cost function MSE:\nDE: %1.14lf\nHybOpt1: %1.14lf\nfminsearchbnd: %1.14lf\nFPA: %1.14lf\nHybOpt2: %1.14lf\nCHIO: %1.14lf\nHybOpt3: %1.14lf\n", deFval, hybOpt1Fval, fminsFval, fpaFval, hybOpt2Fval, chioFval, hybOpt3Fval);

	// Sort result vector by frequency
	idx = (unsigned int*)malloc(numBands * sizeof(unsigned int));
	sort(gbest2, numBands, idx);
	double *sortedOptVector = (double*)malloc(dim * sizeof(double));
	for (i = 0; i < numBands; i++)
	{
		sortedOptVector[i] = gbest2[i];
		sortedOptVector[numBands + i] = gbest2[numBands + idx[i]];
		sortedOptVector[numBands * 2 + i] = gbest2[numBands * 2 + idx[i]];
	}
	fclose(fp);
	/*printf("\n");
	for (i = 0; i < dim; i++)
		printf("%1.14lf,", sortedOptVector[i]);*/
	free(gbest);
	free(gbest2);
	free(idx);
	free(sortedOptVector);

	free(maximaIndex);
	free(minimaIndex);
	free(flt_fc);
	free(dFreqDiscontin);
	free(dif);
	free(flt_peak_g);
	free(initialQ);
	free(phi);
	free(initialAns);
	free(low);
	free(up);
	free(tmpDat);

	free(flt_freqList);
	free(target);
	// Clean up your display GUI
	free(gui.display1);
	system("pause");
	return 0;
}