#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "interpolation2.h"
size_t choose(double *a, double *b, size_t src1, size_t src2)
{
	return (*b >= *a) ? src2 : src1;
}
size_t fast_upper_bound4(double *vec, size_t n, double *value)
{
	size_t size = n;
	size_t low = 0;
	while (size >= 8)
	{
		size_t half = size / 2;
		size_t other_half = size - half;
		size_t probe = low + half;
		size_t other_low = low + other_half;
		size = half;
		low = choose(&vec[probe], value, low, other_low);

		half = size / 2;
		other_half = size - half;
		probe = low + half;
		other_low = low + other_half;
		size = half;
		low = choose(&vec[probe], value, low, other_low);

		half = size / 2;
		other_half = size - half;
		probe = low + half;
		other_low = low + other_half;
		size = half;
		low = choose(&vec[probe], value, low, other_low);
	}
	while (size > 0) {
		size_t half = size / 2;
		size_t other_half = size - half;
		size_t probe = low + half;
		size_t other_low = low + other_half;
		size = half;
		low = choose(&vec[probe], value, low, other_low);
	}
	return low;
}
double getValueAt(ierper *intp, double x, double *y)
{
	double* x_ = (double*)(((char*)intp) + sizeof(ierper));
	double* dydx_ = (double*)(((char*)intp) + sizeof(ierper) + sizeof(double) * intp->n * 1);
	if (x == x_[intp->n - 1])
		return y[intp->n - 1] * intp->weight;
	size_t it = fast_upper_bound4(x_, intp->n, &x);
	if (it == 0)
		it = 1;
	if (it == intp->n)
		it = intp->n - 1;
	double x0 = x_[it - 1];
	double x1 = x_[it];
	double y0 = y[it - 1] * intp->weight;
	double y1 = y[it] * intp->weight;
	double s0 = dydx_[it - 1];
	double s1 = dydx_[it];
	double dx = (x1 - x0);
	double t = (x - x0) / dx;
	return (1.0 - t)*(1.0 - t)*(y0*(1.0 + 2.0 * t) + s0 * (x - x0)) + t * t*(y1*(3.0 - 2.0 * t) + dx * s1*(t - 1.0));
}
void makimaUpdate(ierper *intp, double *y, double weights, int left_endpoint_derivative, int right_endpoint_derivative)
{
	int n = intp->n;
	double* dydx_ = (double*)(((char*)intp) + sizeof(ierper) + sizeof(double) * n * 1);
	double* pc1 = (double*)(((char*)intp) + sizeof(ierper) + sizeof(double) * n * 2);
	double* pc2 = (double*)(((char*)intp) + sizeof(ierper) + sizeof(double) * n * 2 + sizeof(double) * 3);
	double* pc3 = (double*)(((char*)intp) + sizeof(ierper) + sizeof(double) * n * 2 + sizeof(double) * 3 + sizeof(double) * (n - 4) * 4);
	double m2 = (y[3] - y[2]) * pc1[0] * weights;
	double m1 = (y[2] - y[1]) * pc1[1] * weights;
	double m0 = (y[1] - y[0]) * pc1[2] * weights;
	// Quadratic extrapolation: m_{-1} = 2m_0 - m_1:
	double mm1 = 2.0 * m0 - m1;
	// Quadratic extrapolation: m_{-2} = 2*m_{-1}-m_0:
	double mm2 = 2.0 * mm1 - m0;
	double w1 = fabs(m1 - m0) + fabs(m1 + m0) * 0.5;
	double w2 = fabs(mm1 - mm2) + fabs(mm1 + mm2) * 0.5;
	if (left_endpoint_derivative)
	{
		if ((w1 + w2) < DBL_EPSILON)
			dydx_[0] = 0.0;
		else
			dydx_[0] = (w1*mm1 + w2 * m0) / (w1 + w2);
	}
	else
		dydx_[0] = left_endpoint_derivative;

	w1 = fabs(m2 - m1) + fabs(m2 + m1) * 0.5;
	w2 = fabs(m0 - mm1) + fabs(m0 + mm1) * 0.5;
	if ((w1 + w2) < DBL_EPSILON)
		dydx_[1] = 0.0;
	else
		dydx_[1] = (w1*m0 + w2 * m1) / (w1 + w2);
	int minus4 = n - 4;
	for (int i = 2; i < n - 2; ++i)
	{
		double mim2 = (y[i - 1] - y[i - 2]) * weights * pc2[(i - 2) + minus4 * 0];
		double mim1 = (y[i] - y[i - 1]) * weights * pc2[(i - 2) + minus4 * 1];
		double mi = (y[i + 1] - y[i]) * weights * pc2[(i - 2) + minus4 * 2];
		double mip1 = (y[i + 2] - y[i + 1]) * weights * pc2[(i - 2) + minus4 * 3];
		w1 = fabs(mip1 - mi) + fabs(mip1 + mi) * 0.5;
		w2 = fabs(mim1 - mim2) + fabs(mim1 + mim2) * 0.5;
		if ((w1 + w2) < DBL_EPSILON)
			dydx_[i] = 0.0;
		else
			dydx_[i] = (w1*mim1 + w2 * mi) / (w1 + w2);
	}
	// Quadratic extrapolation at the other end:
	double mnm4 = (y[n - 3] - y[n - 4]) * weights * pc3[0];
	double mnm3 = (y[n - 2] - y[n - 3]) * weights * pc3[1];
	double mnm2 = (y[n - 1] - y[n - 2]) * weights * pc3[2];
	double mnm1 = 2.0 * mnm2 - mnm3;
	double mn = 2.0 * mnm1 - mnm2;
	w1 = fabs(mnm1 - mnm2) + fabs(mnm1 + mnm2) * 0.5;
	w2 = fabs(mnm3 - mnm4) + fabs(mnm3 + mnm4) * 0.5;

	if ((w1 + w2) < DBL_EPSILON)
		dydx_[n - 2] = 0.0;
	else
		dydx_[n - 2] = (w1*mnm3 + w2 * mnm2) / (w1 + w2);

	w1 = fabs(mn - mnm1) + fabs(mn + mnm1) * 0.5;
	w2 = fabs(mnm2 - mnm3) + fabs(mnm2 + mnm3) * 0.5;
	if (right_endpoint_derivative)
	{
		if ((w1 + w2) < DBL_EPSILON)
			dydx_[n - 1] = 0.0;
		else
			dydx_[n - 1] = (w1*mnm2 + w2 * mnm1) / (w1 + w2);
	}
	else
		dydx_[n - 1] = right_endpoint_derivative;
	intp->weight = weights;
}
void makimaPC(ierper *intp, double *x, int n)
{
	if (n < 4)
	{
		printf("Must be at least four data points.");
	}
	intp->n = n;
	double* x_ = (double*)(((char*)intp) + sizeof(ierper));
	double* pc1 = (double*)(((char*)intp) + sizeof(ierper) + sizeof(double) * n * 2);
	double* pc2 = (double*)(((char*)intp) + sizeof(ierper) + sizeof(double) * n * 2 + sizeof(double) * 3);
	double* pc3 = (double*)(((char*)intp) + sizeof(ierper) + sizeof(double) * n * 2 + sizeof(double) * 3 + sizeof(double) * (n - 4) * 4);
	pc1[0] = 1.0 / (x[3] - x[2]);
	pc1[1] = 1.0 / (x[2] - x[1]);
	pc1[2] = 1.0 / (x[1] - x[0]);
	int minus4 = intp->n - 4;
	for (int i = 2; i < n - 2; ++i)
	{
		pc2[(i - 2) + minus4 * 0] = 1.0 / (x[i - 1] - x[i - 2]);
		pc2[(i - 2) + minus4 * 1] = 1.0 / (x[i] - x[i - 1]);
		pc2[(i - 2) + minus4 * 2] = 1.0 / (x[i + 1] - x[i]);
		pc2[(i - 2) + minus4 * 3] = 1.0 / (x[i + 2] - x[i + 1]);
	}
	// Quadratic extrapolation at the other end:
	pc3[0] = 1.0 / (x[n - 3] - x[n - 4]);
	pc3[1] = 1.0 / (x[n - 2] - x[n - 3]);
	pc3[2] = 1.0 / (x[n - 1] - x[n - 2]);
	intp->interpolate = getValueAt;
	intp->updateY = makimaUpdate;
	memcpy(x_, x, sizeof(double) * n);
	double x0 = x_[0];
	for (int i = 1; i < n; ++i)
	{
		double x1 = x_[i];
		if (x1 <= x0)
			printf("Abscissas must be listed in strictly increasing order x0 < x1 < ... < x_{n-1}");
		x0 = x1;
	}
}
double npointWndFunction(double val, double *x, double *y, int n)
{
	if (val == x[0])
		return y[0];
	if (val == x[n - 1])
		return y[n - 1];
	size_t j = fast_upper_bound4(x, n, &val);
	if (j <= 0)
		return (val - x[1]) / (x[0] - x[1]) * (y[0] - y[1]) + y[1]; // Extrapolation to leftmost
	else if (j >= n)
		return (val - x[n - 2]) / (x[n - 1] - x[n - 2]) * (y[n - 1] - y[n - 2]) + y[n - 2]; // Extrapolation to rightmost
	else
		return ((val - x[j - 1]) / (x[j] - x[j - 1])) * (y[j] - y[j - 1]) + y[j - 1]; // Interpolation
}
double linearInterpolationNoExtrapolate(double val, double *x, double *y, int n)
{
	if (val <= x[0])
		return y[0];
	if (val >= x[n - 1])
		return y[n - 1];
	size_t j = fast_upper_bound4(x, n, &val);
	return ((val - x[j - 1]) / (x[j] - x[j - 1])) * (y[j] - y[j - 1]) + y[j - 1]; // Interpolation
}
double npointWndFunctionYWeighted(double val, double *x, double *y, int n, double weight)
{
	if (val == x[0])
		return y[0] * weight;
	if (val == x[n - 1])
		return y[n - 1] * weight;
	size_t j = fast_upper_bound4(x, n, &val);
	if (j <= 0)
		return (val - x[1]) / (x[0] - x[1]) * (y[0] - y[1]) * weight + y[1] * weight; // Extrapolation to leftmost
	else if (j >= n)
		return (val - x[n - 2]) / (x[n - 1] - x[n - 2]) * (y[n - 1] - y[n - 2]) * weight + y[n - 2] * weight; // Extrapolation to rightmost
	else
		return ((val - x[j - 1]) / (x[j] - x[j - 1])) * (y[j] - y[j - 1]) * weight + y[j - 1] * weight; // Interpolation
}
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