#include <string.h>
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