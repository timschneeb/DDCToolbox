#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "iirZpSmooth.h"
typedef struct
{
	unsigned int pos1, pos2, m, n, target1, target2, tmpLen, segSize;
	float alpha, minusAlpha, avgGain;
} edgPreSmooth;
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
size_t edgePreservingSmoothInit(void *handle, unsigned int m, double ms, double fs)
{
	unsigned int n = min(m, (unsigned int)(ms * (fs / 1000.0)));
	if (!handle)
		return sizeof(edgPreSmooth) + (m + n - 1) * sizeof(float);
	edgPreSmooth *sm = (edgPreSmooth*)handle;
	sm->n = n;
	sm->m = m;
	sm->tmpLen = m + n - 1;
	sm->alpha = (float)(1.0 - exp(-1.0 / ((ms / 1000.0) * fs)));
	sm->minusAlpha = 1.0f - sm->alpha;
	sm->pos1 = n >> 1;
	if (n % 2)
	{
		sm->pos2 = sm->pos1 + 1;
		sm->target1 = m + n - sm->pos1;
		sm->target2 = m + n - 1 - sm->pos1;
	}
	else
	{
		sm->pos2 = sm->pos1;
		sm->target1 = m + n + 1 - sm->pos1;
		sm->target2 = m + n - sm->pos1;
	}
	sm->segSize = max(1, sm->pos1 / 4);
	sm->avgGain = 1.0f / (float)sm->segSize;
	return 0;
}
unsigned int edgePreservingSmoothGetInputLen(void *handle)
{
	edgPreSmooth *sm = (edgPreSmooth*)handle;
	return sm->m;
}
void emaZpSmooth(void *handle, float *x, float *y, char boundaryMode)
{
	edgPreSmooth *sm = (edgPreSmooth*)handle;
	unsigned int i;
	float *tmp = (float*)((char*)handle + sizeof(edgPreSmooth));
	if (!boundaryMode)
	{
		float frontAvg = 0.0f;
		float backAvg = 0.0f;
		for (i = 0; i < sm->segSize; i++)
		{
			frontAvg += x[i];
			backAvg += x[sm->m - 1 - i];
		}
		frontAvg = frontAvg * sm->avgGain;
		backAvg = backAvg * sm->avgGain;
		for (i = 0; i < sm->pos1; i++)
		{
			tmp[i] = frontAvg;
			tmp[sm->m - 1 + i + sm->pos2] = backAvg;
		}
	}
	else
	{
		for (i = 0; i < sm->pos1; i++)
		{
			tmp[i] = 2.0f * x[0] - x[sm->pos2 - 1 - i];
			tmp[sm->m - 1 + i + sm->pos2] = 2.0f * x[sm->m - 1] - x[sm->m - 2 - i];
		}
	}
	memcpy(tmp + sm->pos1, x, sm->m * sizeof(float));
	float ema = 0.0f;
	for (i = 0; i < sm->tmpLen; i++)
	{
		ema = tmp[i] * sm->alpha + ema * sm->minusAlpha;
		tmp[i] = ema;
	}
	for (i = sm->tmpLen; i-- >= sm->target1; )
		ema = tmp[i] * sm->alpha + ema * sm->minusAlpha;
	for (i = sm->target2; i-- > sm->pos1; )
	{
		ema = tmp[i] * sm->alpha + ema * sm->minusAlpha;
		y[i - sm->pos1] = ema;
	}
}