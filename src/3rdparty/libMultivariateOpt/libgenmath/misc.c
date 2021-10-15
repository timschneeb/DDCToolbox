double minArray(double *x, unsigned int N, unsigned int *ind)
{
	double minV = x[0];
	unsigned int i, index = 0;
	for (unsigned int i = 1; i < N; i++)
	{
		if (x[i] < minV)
		{
			index = i;
			minV = x[i];
		}
	}
	*ind = index;
	return minV;
}