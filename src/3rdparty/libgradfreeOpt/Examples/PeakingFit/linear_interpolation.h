static size_t choose(double *a, double *b, size_t src1, size_t src2)
{
	return (*b >= *a) ? src2 : src1;
}
static size_t fast_upper_bound4(double *vec, size_t n, double *value)
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
static double npointWndFunction(double val, double *x, double *y, int n)
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
static double linearInterpolationNoExtrapolate(double val, double *x, double *y, int n)
{
	if (val <= x[0])
		return y[0];
	if (val >= x[n - 1])
		return y[n - 1];
	size_t j = fast_upper_bound4(x, n, &val);
	return ((val - x[j - 1]) / (x[j] - x[j - 1])) * (y[j] - y[j - 1]) + y[j - 1]; // Interpolation
}