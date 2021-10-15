#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "gradfreeOpt.h"
//#include <vld.h>
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
double fminsearch(double (*funcPtr)(double*,void*), void *userdat, double *x, unsigned int n, double TolX, double TolFun, unsigned int MaxIter, double *outX, char adaptive, void(*optStatus)(void*, unsigned int, double*, double*), void *optHost)
{
	unsigned int i, j;
	const double rho = 1.0;
	double chi, psi, sigma;
	if (adaptive)
	{
		chi = 1.0 + 2.0 / n;
		psi = 0.75 - 1.0 / (2.0 * n);
		sigma = 1.0 - 1.0 / n;
	}
	else
	{
		chi = 2.0;
		psi = 0.5;
		sigma = 0.5;
	}
	double *xin = (double*)malloc(n * sizeof(double));
	memcpy(xin, x, n * sizeof(double));
	double *v = (double*)malloc(n * (n + 1) * sizeof(double));
	double *v2 = (double*)malloc(n * (n + 1) * sizeof(double));
	double *fv = (double*)malloc((n + 1) * sizeof(double));
	unsigned int *sotIdx = (unsigned int*)malloc((n + 1) * sizeof(unsigned int));
	memcpy(v, xin, n * sizeof(double));
	memcpy(outX, xin, n * sizeof(double));
	fv[0] = funcPtr(outX, userdat);
	double usual_delta = 0.05; // 5 percent deltas for non - zero terms
	double zero_term_delta = 0.00025; // Even smaller delta for zero elements of x
	double *y = (double*)malloc(n * sizeof(double));
	double *xbar = (double*)malloc(n * sizeof(double));
	double *xr = (double*)malloc(n * sizeof(double));
	double *xe = (double*)malloc(n * sizeof(double));
	double *xc = (double*)malloc(n * sizeof(double));
	double *xcc = (double*)malloc(n * sizeof(double));
	for (j = 0; j < n; j++)
	{
		memcpy(y, xin, n * sizeof(double));
		if (y[j] != 0.0)
			y[j] = (1.0 + usual_delta) * y[j];
		else
			y[j] = zero_term_delta;
		memcpy(v + (j + 1) * n, y, n * sizeof(double));
		memcpy(outX, y, n * sizeof(double));
		fv[j + 1] = funcPtr(outX, userdat);
	}
	sort(fv, n + 1, sotIdx);
	for (j = 0; j < n + 1; j++)
		memcpy(v2 + j * n, v + sotIdx[j] * n, n * sizeof(double));
	double *ptr = v;
	v = v2;
	v2 = ptr;
	double arg1, arg3, arg4;
	for (unsigned int epk = 0; epk < MaxIter; epk++)
	{
		arg1 = fabs(fv[0] - fv[1]);
		for (i = 2; i < n + 1; i++)
		{
			double tmp = fabs(fv[0] - fv[i]);
			if (tmp > arg1)
				arg1 = tmp;
		}
		arg3 = fabs(v[1 * n + 0] - v[0]);
		for (i = 1; i < n; i++)
		{
			double tmp = fabs(v[1 * n + i] - v[i]);
			if (tmp > arg3)
				arg3 = tmp;
		}
		for (j = 2; j < n + 1; j++)
		{
			for (i = 1; i < n; i++)
			{
				double tmp = fabs(v[j * n + i] - v[i]);
				if (tmp > arg3)
					arg3 = tmp;
			}
		}
		arg4 = v[0];
		for (i = 1; i < n; i++)
		{
			if (v[i] > arg4)
				arg4 = v[i];
		}
		if (arg1 <= max(TolFun, 10.0 * DBL_EPSILON * fv[0]) && arg3 <= max(TolX, 10.0 * DBL_EPSILON * arg4))
			break;
		// Compute the reflection point

		// xbar = average of the n best points
		for (j = 0; j < n; j++)
			xbar[j] = v[j];
		for (i = 0; i < n; i++)
			for (j = 1; j < n; j++)
				xbar[i] += v[j * n + i];
		for (j = 0; j < n; j++)
			xbar[j] *= (1.0 / n);
		for (i = 0; i < n; i++)
		{
			xr[i] = (1.0 + rho) * xbar[i] - rho * v[n * n + i];
			outX[i] = xr[i];
		}
		double fxr = funcPtr(outX, userdat);
		if (fxr < fv[0])
		{
			// Calculate the expansion point
			for (i = 0; i < n; i++)
			{
				xe[i] = (1.0 + rho * chi) * xbar[i] - rho * chi * v[n * n + i];
				outX[i] = xe[i];
			}
			double fxe = funcPtr(outX, userdat);
			if (fxe < fxr)
			{
				memcpy(v + n * n, xe, n * sizeof(double));
				fv[n] = fxe;
			}
			else
			{
				memcpy(v + n * n, xr, n * sizeof(double));
				fv[n] = fxr;
			}
		}
		else // fv(1) <= fxr
		{
			if (fxr < fv[n - 1])
			{
				memcpy(v + n * n, xr, n * sizeof(double));
				fv[n] = fxr;
			}
			else // fxr >= fv(:,n)
			{
				unsigned char doShrink;
				// Perform contraction
				if (fxr < fv[n])
				{
					// Perform an outside contraction
					for (i = 0; i < n; i++)
					{
						xc[i] = (1.0 + psi * rho) * xbar[i] - psi * rho * v[n * n + i];
						outX[i] = xc[i];
					}
					double fxc = funcPtr(outX, userdat);
					if (fxc <= fxr)
					{
						memcpy(v + n * n, xc, n * sizeof(double));
						fv[n] = fxc;
						doShrink = 0;
					}
					else
					{
						// perform a shrink
						doShrink = 1;
					}
				}
				else
				{
					// Perform an inside contraction
					for (i = 0; i < n; i++)
					{
						xcc[i] = (1.0 - psi) * xbar[i] + psi * v[n * n + i];
						outX[i] = xcc[i];
					}
					double fxcc = funcPtr(outX, userdat);
					if (fxcc < fv[n])
					{
						memcpy(v + n * n, xcc, n * sizeof(double));
						fv[n] = fxcc;
						doShrink = 0;
					}
					else
					{
						// perform a shrink
						doShrink = 1;
					}
				}
				if (doShrink)
				{
					for (j = 1; j < n + 1; j++)
					{
						for (i = 0; i < n; i++)
						{
							v[j * n + i] = v[0 * n + i] + sigma * (v[j * n + i] - v[0 * n + i]);
							outX[i] = v[j * n + i];
						}
						fv[j] = funcPtr(outX, userdat);
					}
				}
			}
		}
		sort(fv, n + 1, sotIdx);
		if (optStatus)
			optStatus(optHost, n, v, &fv[0]);
		for (j = 0; j < n + 1; j++)
			memcpy(v2 + j * n, v + sotIdx[j] * n, n * sizeof(double));
		ptr = v;
		v = v2;
		v2 = ptr;
	}
	memcpy(outX, v, n * sizeof(double));
	double fval = fv[0];
	free(xin);
	free(v);
	free(v2);
	free(fv);
	free(sotIdx);
	free(y);
	free(xbar);
	free(xr);
	free(xe);
	free(xc);
	free(xcc);
	return fval;
}
typedef struct
{
	double *LB, *UB;
	unsigned int n;
	double(*funcPtr)(double*, void*);
	void *userdat;
	double *xtrans;
} wrapperData;
void xtransform(double *x, wrapperData *params)
{
	// converts unconstrained variables into their original domains
	for (unsigned int i = 0; i < params->n; i++)
	{
		// lower and upper bounds
		double tmp = ((sin(x[i]) + 1) / 2) * (params->UB[i] - params->LB[i]) + params->LB[i];
		// just in case of any floating point problems
		params->xtrans[i] = max(params->LB[i], min(params->UB[i], tmp));
	}
}
double wrapperFunctionBoundConstraint(double *x, void *usd)
{
	wrapperData *userdata = (wrapperData*)usd;
	xtransform(x, userdata);
	double fval = userdata->funcPtr(userdata->xtrans, userdata->userdat);
	return fval;
}
double fminsearchbnd(double(*funcPtr)(double*, void*), void *userdat, double *x0, double *lb, double *ub, unsigned int n, double TolX, double TolFun, unsigned int MaxIter, double *outX, char adaptive, void(*optStatus)(void*, unsigned int, double*, double*), void *optHost)
{
	unsigned int i;
	wrapperData params;
	if (lb && ub)
	{
		params.LB = lb;
		params.UB = ub;
	}
	else
	{
		params.LB = (double*)malloc(n * sizeof(double));
		params.UB = (double*)malloc(n * sizeof(double));
		for (i = 0; i < n; i++)
		{
			params.LB[i] = -DBL_MAX * 0.5;
			params.UB[i] = DBL_MAX * 0.5;
		}
	}
	params.funcPtr = funcPtr;
	params.userdat = userdat;
	params.n = n;
	params.xtrans = (double*)malloc(n * sizeof(double));
	double *x0u = (double*)malloc(n * sizeof(double));
	for (i = 0; i < n; i++)
	{
		if (x0[i] <= lb[i])
		{
			// infeasible starting value
			x0u[i] = -M_PI / 2;
		}
		else if (x0[i] >= ub[i])
		{
			// infeasible starting value
			x0u[i] = M_PI / 2;
		}
		else
		{
			x0u[i] = 2.0 * (x0[i] - lb[i]) / (ub[i] - lb[i]) - 1.0;
			// shift by 2*pi to avoid problems at zero in fminsearch
			// otherwise, the initial simplex is vanishingly small
			x0u[i] = 2.0 * M_PI + asin(max(-1.0, min(1.0, x0u[i])));
		}
	}
	double fval = fminsearch(wrapperFunctionBoundConstraint, (void*)&params, x0u, n, TolX, TolFun, MaxIter, outX, adaptive, optStatus, optHost);
	free(x0u);
	xtransform(outX, &params);
	memcpy(outX, params.xtrans, n * sizeof(double));
	free(params.xtrans);
	if (!lb || !ub)
	{
		free(params.LB);
		free(params.UB);
	}
	return fval;
}
double differentialEvolution(double(*funcPtr)(double*, void*), void *userdat, double *initialSolution, unsigned int K, unsigned int N, const double probiBound, unsigned int D, double *low, double *up, unsigned int MaxIter, double *gbest, pcg32x2_random_t *PRNG, double(*pdf1)(pcg32x2_random_t*), void(*optStatus)(void*, unsigned int, double*, double*), void *optHost)
{
	unsigned i, j;
	// Initialization
	double *S = (double*)malloc(K * N * D * sizeof(double));
	if (initialSolution)
		memcpy(S, initialSolution, K * N * D * sizeof(double));
	else
	{
		for (i = 0; i < K * N; i++)
		{
			for (j = 0; j < D; j++)
				S[i * D + j] = c_rand(PRNG) * (up[j] - low[j]) + low[j];
		}
	}
	double *fitS = (double*)malloc(K * N * sizeof(double));
	double *fitSTmp = (double*)malloc(K * N * sizeof(double));
	for (i = 0; i < K * N; i++)
		fitS[i] = funcPtr(S + i * D, userdat);
	unsigned int ind;
	double gmin = minArray(fitS, K * N, &ind);
	for (i = 0; i < D; i++)
		gbest[i] = S[ind * D + i];
	double *F = (double*)malloc(N * sizeof(double));
	double *T = (double*)malloc(N * D * sizeof(double));
	unsigned int *j0 = (unsigned int*)malloc(N * sizeof(unsigned int));
	unsigned int *j2_first = (unsigned int*)malloc(K * N * sizeof(unsigned int));
	unsigned int *j1 = (unsigned int*)malloc(K * N * sizeof(unsigned int));
	unsigned int *j2 = (unsigned int*)malloc(K * N * sizeof(unsigned int));
	double *fitP = (double*)malloc(N * sizeof(double));
	double *tbest = (double*)malloc(N * D * sizeof(double));
	double *P = (double*)malloc(N * D * sizeof(double));
	double *dv2 = (double*)malloc(N * D * sizeof(double));
	unsigned int *sortingIdx = (unsigned int*)malloc(K * N * sizeof(unsigned int));
	unsigned int *tmpIdx = (unsigned int*)malloc(4 * sizeof(unsigned int));
	unsigned char *map1 = (unsigned char*)malloc(N * D * sizeof(unsigned char)); // 1-bit
	unsigned char *map2 = (unsigned char*)malloc(N * D * sizeof(unsigned char)); // 1-bit
	unsigned char *map;
	unsigned int *h = (unsigned int*)malloc(D * sizeof(unsigned int));
	for (unsigned int epk = 0; epk < MaxIter; epk++)
	{
		unsigned int cnt;
		while (1)
		{
			randperm(j1, K * N, PRNG);
			randperm(j2, K * N, PRNG);
			cnt = 0;
			for (i = 0; i < K * N; i++)
			{
				if (j1[i] == j2[i])
					break;
				else
					cnt++;
			}
			if (cnt == K * N)
				break;
		}
		// Setting sub-pattern matrix P and fitP
		for (i = 0; i < N; i++)
		{
			j0[i] = j1[i];
			j2_first[i] = j2[i];
			fitP[i] = fitS[j0[i]];
			memcpy(P + i * D, S + j0[i] * D, D * sizeof(double));
		}
		memcpy(fitSTmp, fitS, K * N * sizeof(double));
		sort(fitSTmp, K * N, sortingIdx);
		// Top-N-Best pattern vectors
		for (i = 0; i < N; i++)
		{
			double tmp = c_rand(PRNG);
			unsigned int idx = ((unsigned int)ceil(pow(tmp, 3) * K * N)) - 1u;
			memcpy(tbest + i * D, S + sortingIdx[idx] * D, D * sizeof(double));
		}
		// Generation of Bezier Mutation Vectors ; dv2
		while (1)
		{
			randperm(j1, N, PRNG);
			randperm(j2, N, PRNG);
			cnt = 0;
			for (i = 0; i < N; i++)
			{
				if (j1[i] == i || j2[i] == i || j1[i] == j2[i])
					break;
				else
					cnt++;
			}
			if (cnt == N)
				break;
		}
		for (i = 0; i < N; i++)
		{
			randperm(tmpIdx, 4, PRNG);
			double tmp = c_rand(PRNG);
			// Bernstein
			double tt = 1 - tmp;
			double tt2 = tt * tt;
			double t2 = tmp * tmp;
			double B[4] = { tt2 * tt, tmp * 3 * tt2, t2 * 3 * tt, t2 * tmp };
			double *X[4] = { &P[i * D], &P[j1[i] * D], &P[j2[i] * D], &tbest[i * D] };
			for (j = 0; j < D; j++)
				dv2[i * D + j] = B[0] * X[tmpIdx[0]][j] + B[1] * X[tmpIdx[1]][j] + B[2] * X[tmpIdx[2]][j] + B[3] * X[tmpIdx[3]][j];
		}
		// Generation of Crossover Control Matrix ; map
		memset(map1, 0, N * D * sizeof(unsigned char));
		memset(map2, 0, N * D * sizeof(unsigned char));
		for (i = 0; i < N; i++)
		{
			randperm(h, D, PRNG);
			double tmp1 = c_rand(PRNG);
			double tmp2 = randi(PRNG, 7.0); // Different upper bound different distribution sharpness
			double w = pow(tmp1, tmp2);
			unsigned int rang = (unsigned int)ceil(w * D);
			for (j = 0; j < rang; j++)
				map1[i * D + h[j]] = 1u;
			randperm(h, D, PRNG);
			tmp1 = c_rand(PRNG);
			tmp2 = randi(PRNG, 7.0); // Different upper bound different distribution sharpness
			w = 1.0 - pow(tmp1, tmp2);
			rang = (unsigned int)ceil(w * D);
			for (j = 0; j < rang; j++)
				map2[i * D + h[j]] = 1u;
		}
		double tmp1 = c_rand(PRNG);
		double tmp2 = c_rand(PRNG);
		if (tmp1 < tmp2)
			map = map1;
		else
			map = map2;
		for (i = 0; i < N; i++)
		{
			tmp1 = pdf1(PRNG);
			tmp2 = c_rand(PRNG);
			double tmp3 = pdf1(PRNG);
			double tmp4 = randi(PRNG, 7.0); // Different upper bound different distribution sharpness
			double tmp5 = 1.0 + tmp2;
			double tmp6 = pow(tmp3, tmp4);
			if (fabs(tmp5) > 1.0 && fabs(tmp6) > 11.0)
				tmp6 = ((tmp6 > 0.0) ? 1.0 : ((tmp6 < 0.0) ? -1.0 : 0.0)) * 11.0;
			double F = tmp1 * pow(tmp5, tmp6);
			double w1 = pdf1(PRNG);
			double w2 = pdf1(PRNG);
			for (j = 0; j < D; j++)
				T[i * D + j] = P[i * D + j] + (double)map[i * D + j] * F * (w1 * (dv2[i * D + j] - P[i * D + j]) + w2 * ((S[j2_first[i] * D + j] - P[i * D + j])));
		}
		double p1 = c_rand(PRNG);
		if (p1 > probiBound)
		{
			for (i = 0; i < N; i++)
			{
				for (j = 0; j < D; j++)
				{
					if (T[i * D + j] < low[j] || T[i * D + j] > up[j])
						T[i * D + j] = c_rand(PRNG) * (up[j] - low[j]) + low[j];
				}
			}
		}
		else
		{
			for (i = 0; i < N; i++)
			{
				for (j = 0; j < D; j++)
				{
					if (T[i * D + j] < low[j])
						T[i * D + j] = low[j];
					if (T[i * D + j] > up[j])
						T[i * D + j] = up[j];
				}
			}
		}
		// Update the sub-pattern matrix, P and fitP
		for (i = 0; i < N; i++)
			fitSTmp[i] = funcPtr(T + i * D, userdat);
		for (i = 0; i < N; i++)
		{
			if (fitSTmp[i] < fitP[i])
			{
				memcpy(P + i * D, T + i * D, D * sizeof(double));
				fitP[i] = fitSTmp[i];
			}
		}
		unsigned int index = 0;
		double BestVal = fitP[0];
		for (i = 1; i < N; i++)
		{
			if (fitP[i] < BestVal)
			{
				BestVal = fitP[i];
				index = i;
			}
		}
		if (BestVal < gmin)
		{
			gmin = BestVal;
			memcpy(gbest, P + index * D, D * sizeof(double));
		}
		for (i = 0; i < N; i++)
		{
			memcpy(S + j0[i] * D, P + i * D, D * sizeof(double));
			fitS[j0[i]] = fitP[i];
		}
		if (optStatus)
			optStatus(optHost, D, gbest, &gmin);
	}
	// Free optimizer
	free(fitS);
	free(fitSTmp);
	free(F);
	free(T);
	free(j0);
	free(j2_first);
	free(j1);
	free(j2);
	free(fitP);
	free(tbest);
	free(P);
	free(dv2);
	free(sortingIdx);
	free(tmpIdx);
	free(map1);
	free(map2);
	free(h);
	free(S);
	return gmin;
}
double flowerPollination(double(*funcPtr)(double*, void*), void *userdat, double *initialSolution, double *low, double *up, unsigned int D, unsigned int popSize, double pCond, double weightStep, unsigned int N_iter, double *gbest, pcg32x2_random_t *PRNG, double(*pdf1)(pcg32x2_random_t*), void(*optStatus)(void*, unsigned int, double*, double*), void *optHost)
{
	unsigned i, j;
	// Initialization
	double *Sol = (double*)malloc(popSize * D * sizeof(double));
	if (initialSolution)
		memcpy(Sol, initialSolution, popSize * D * sizeof(double));
	else
	{
		for (i = 0; i < popSize; i++)
		{
			for (j = 0; j < popSize; j++)
				Sol[i * D + j] = c_rand(PRNG) * (up[j] - low[j]) + low[j];
		}
	}
	double *Fitness = (double*)malloc(popSize * sizeof(double));
	for (i = 0; i < popSize; i++)
		Fitness[i] = funcPtr(Sol + i * D, userdat);
	unsigned int ind;
	double gmin = minArray(Fitness, popSize, &ind);
	for (i = 0; i < D; i++)
		gbest[i] = Sol[ind * D + i];
	double *S = (double*)malloc(popSize * D * sizeof(double));
	memcpy(S, Sol, popSize * D * sizeof(double));
	unsigned int *JK = (unsigned int*)malloc(popSize * sizeof(unsigned int));
	for (unsigned int t = 0; t < N_iter; t++)
	{
		// Loop over all solutions
		for (unsigned int pol = 0; pol < popSize; pol++)
		{
			// Pollens are carried by insects and thus can move on large scale
			// This L should be drawn from the Levy distribution
			// Formula: x_i ^ { t + 1 } = x_i ^ t + L(x_i^t - gbest)
			double tmp1 = c_rand(PRNG);
			if (tmp1 > pCond) // Probability (pCond) is checked after drawing a rand
			{
				const double beta = 3.0 / 2.0;
				double sigma = pow(tgamma(1 + beta) * sin(M_PI * beta / 2) / (tgamma((1 + beta) / 2) * beta * pow(2.0, (beta - 1.0) / 2.0)), 1.0 / beta);
				for (i = 0; i < D; i++)
				{
					double u = pdf1(PRNG) * sigma;
					double v = pdf1(PRNG);
					double dS = (weightStep * (u / pow(fabs(v), (1 / beta)))) * (Sol[pol * D + i] - gbest[i]); // Caclulate the step increments
					S[pol * D + i] = Sol[pol * D + i] + dS; // Update the new solutions
					// Check new solutions to satisfy the simple limits/bounds
					if (S[pol * D + i] < low[i])
						S[pol * D + i] = low[i];
					if (S[pol * D + i] > up[i])
						S[pol * D + i] = up[i];
				}
			}
			else
			{
				double epsilon = c_rand(PRNG);
				randperm(JK, popSize, PRNG);
				// As they are random, the first two entries also random
				// If the flower are the same or similar species, then
				// they can be pollenated, otherwise, no action is needed.
				// Formula: x_i^{t+1}+epsilon*(x_j^t-x_k^t)
				for (i = 0; i < D; i++)
				{
					S[pol * D + i] = S[pol * D + i] + epsilon * (Sol[JK[0] * D + i] - Sol[JK[1] * D + i]);
					if (S[pol * D + i] < low[i])
						S[pol * D + i] = low[i];
					if (S[pol * D + i] > up[i])
						S[pol * D + i] = up[i];
				}
			}
			// Evaluate the objective values of the new solutions
			double Fnew = funcPtr(&S[pol * D], userdat);
			if (Fnew <= Fitness[pol])
			{
				memcpy(&Sol[pol * D], &S[pol * D], D * sizeof(double));
				Fitness[pol] = Fnew;
			}
			// Update the current global best among the population
			if (Fnew <= gmin)
			{
				memcpy(gbest, &S[pol * D], D * sizeof(double));
				gmin = Fnew;
				if (optStatus)
					optStatus(optHost, D, gbest, &gmin);
			}
		}
	}
	free(Fitness);
	free(S);
	free(JK);
	free(Sol);
	return gmin;
}
double CHIO(double(*funcPtr)(double*, void*), void *userdat, double *initialSolution, unsigned int popSize, unsigned int maxSolSurviveEpoch, unsigned int C0, double spreadingRate, unsigned int dim, double *lb, double *ub, unsigned int MaxIter, double *gbest, pcg32x2_random_t *PRNG, void(*optStatus)(void*, unsigned int, double*, double*), void *optHost)
{
	unsigned i, j;
	//unsigned int maxSolSurviveEpoch = 100;
	//unsigned int C0 = 6; // Number of solutions get infected
	//double spreadingRate = 0.05; // Spreading rate parameter
	// Initialization
	double *fitS = (double*)malloc(popSize * sizeof(double));
	unsigned int *Age = (unsigned int*)malloc(popSize * sizeof(unsigned int));
	memset(Age, 0, popSize * sizeof(unsigned int));
	double *Fitness = (double*)malloc(popSize * sizeof(double));
	unsigned char *solFlag = (unsigned char*)malloc(popSize * sizeof(unsigned char));
	memset(solFlag, 0, popSize * sizeof(unsigned char));
	unsigned int *confirmedList = (unsigned int*)malloc(popSize * sizeof(unsigned int));
	unsigned int *normList = (unsigned int*)malloc(popSize * sizeof(unsigned int));
	double *S = (double*)malloc(popSize * dim * sizeof(double));
	double *NewSol = (double*)malloc(dim * sizeof(double));
	if (initialSolution)
		memcpy(S, initialSolution, popSize * dim * sizeof(double));
	else
	{
		for (i = 0; i < popSize; i++)
		{
			for (j = 0; j < dim; j++)
				S[i * dim + j] = c_rand(PRNG) * (ub[j] - lb[j]) + lb[j];
		}
	}
	for (i = 0; i < popSize; i++)
		fitS[i] = funcPtr(S + i * dim, userdat);
	unsigned int gminIdx;
	double gmin = minArray(fitS, popSize, &gminIdx);
	for (i = 0; i < popSize; i++)
	{
		if (fitS[i] >= 0.0)
			Fitness[i] = 1.0 / (fitS[i] + 1);
		else
			Fitness[i] = 1.0 + fabs(fitS[i]);
	}
	for (i = 0; i < C0; i++)
		solFlag[(unsigned int)(c_rand(PRNG) * popSize)] = 1;
	unsigned int infected, numConfirmed, numNormal;
	unsigned char numRec;
	for (unsigned int epk = 0; epk < MaxIter; epk++)
	{
		for (unsigned int pol = 0; pol < popSize; pol++)
		{
			memcpy(NewSol, &S[pol * dim], dim * sizeof(double));
			infected = 0;
			numConfirmed = 0;
			numNormal = 0;
			numRec = 0;
			for (i = 0; i < popSize; i++)
			{
				if (solFlag[i] == 0)
				{
					normList[numNormal] = i;
					numNormal = numNormal + 1;
				}
				if (solFlag[i] == 1)
				{
					confirmedList[numConfirmed] = i;
					numConfirmed = numConfirmed + 1;
				}
			}
			for (i = 0; i < popSize; i++)
			{
				if (fitS[i] != 0 && solFlag[i] == 2)
				{
					numRec = 1;
					break;
				}
			}
			for (j = 0; j < dim; j++)
			{
				double r = c_rand(PRNG); // select a number within range 0 to 1
				if ((r < spreadingRate / 3) && (numConfirmed > 0))
				{
					// select one of the confirmed solutions
					unsigned int z = (unsigned int)round((numConfirmed - 1) * c_rand(PRNG));
					unsigned int zc = confirmedList[z];
					// modify the curent value
					NewSol[j] = S[pol * dim + j] + (S[pol * dim + j] - S[zc * dim + j]) * (c_rand(PRNG) - 0.5) * 2;
					// manipulate range between lb and ub
					NewSol[j] = min(max(NewSol[j], lb[j]), ub[j]);
					infected = infected + 1;
				}
				else if ((r < spreadingRate / 2) && numNormal > 0)
				{
					// select one of the normal solutions
					unsigned int z = (unsigned int)round((numNormal - 1) * c_rand(PRNG));
					unsigned int zn = normList[z];
					// modify the curent value
					NewSol[j] = S[pol * dim + j] + (S[pol * dim + j] - S[zn * dim + j]) * (c_rand(PRNG) - 0.5) * 2;
					// manipulate range between lb and ub
					NewSol[j] = min(max(NewSol[j], lb[j]), ub[j]);
				}
				else if (r < spreadingRate && numRec == 1)
				{
					// modify the curent value
					NewSol[j] = S[pol * dim + j] + (S[pol * dim + j] - S[j]) * (c_rand(PRNG) - 0.5) * 2;
					// manipulate range between lb and ub
					NewSol[j] = min(max(NewSol[j], lb[j]), ub[j]);
				}
			}
			// evaluate new solution
			double ObjValSol = funcPtr(NewSol, userdat);
			double FitnessSol;
			if (ObjValSol >= 0)
				FitnessSol = 1.0 / (ObjValSol + 1.0);
			else
				FitnessSol = 1.0 + fabs(ObjValSol);
			// Update the curent solution & Age of the current solution
			if (fitS[pol] > ObjValSol)
			{
				memcpy(&S[pol * dim], NewSol, dim * sizeof(double));
				Fitness[pol] = FitnessSol;
				fitS[pol] = ObjValSol;
			}
			else
			{
				if (solFlag[pol] == 1)
					Age[pol] = Age[pol] + 1;
			}
			double mean = 0.0;
			for (i = 0; i < popSize; i++)
				mean += Fitness[i];
			mean /= (double)popSize;
			// change the solution from normal to confirmed
			if ((Fitness[pol] < mean) && solFlag[pol] == 0 && infected > 0)
			{
				solFlag[pol] = 1;
				Age[pol] = 1;
			}
			// change the solution from confirmed to recovered
			if ((Fitness[pol] >= mean) && solFlag[pol] == 1)
			{
				solFlag[pol] = 2;
				Age[pol] = 0;
			}
			// Regenerated solution from scratch
			if (Age[pol] >= maxSolSurviveEpoch)
			{
				for (i = 0; i < dim; i++)
					S[pol * dim + i] = c_rand(PRNG) * (ub[i] - lb[i]) + lb[i];
				solFlag[pol] = 0;
			}
		}
		gmin = minArray(fitS, popSize, &gminIdx);
		memcpy(gbest, &S[gminIdx * dim], dim * sizeof(double));
		if (optStatus)
			optStatus(optHost, dim, gbest, &gmin);
	}
	free(fitS);
	free(Age);
	free(Fitness);
	free(solFlag);
	free(confirmedList);
	free(normList);
	free(S);
	free(NewSol);
	return gmin;
}