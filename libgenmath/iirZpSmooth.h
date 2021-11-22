extern size_t edgePreservingSmoothInit(void *sm, unsigned int m, double ms, double fs);
extern unsigned int edgePreservingSmoothGetInputLen(void *handle);
extern void emaZpSmooth(void *sm, float *x, float *y, char boundaryMode);