typedef struct mmLerp
{
	double(*interpolate)(struct mmLerp*, double, double *);
	void(*updateY)(struct mmLerp*, double *, double, int, int);
	double weight;
	int n;
} ierper;
extern void makimaPC(ierper *intp, double * x, int n);
double npointWndFunction(double val, double *x, double *y, int n);
double linearInterpolationNoExtrapolate(double val, double *x, double *y, int n);
double npointWndFunctionYWeighted(double val, double *x, double *y, int n, double weight);