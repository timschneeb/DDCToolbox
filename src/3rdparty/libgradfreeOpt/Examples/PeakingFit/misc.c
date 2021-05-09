#include <stdio.h>
#include "misc.h"
void printMatrixFile(char *filename, float *mat, int w, int h)
{
	int i, j;
	FILE *fp = fopen(filename, "wb");
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
			fprintf(fp, "%.9g,", mat[i * w + j]);
		fprintf(fp, "\n");
	}
	fclose(fp);
}
void printInt32MatrixFile(char *filename, int *mat, int w, int h)
{
	int i, j;
	FILE *fp = fopen(filename, "wb");
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
			fprintf(fp, "%d,", mat[i * w + j]);
		fprintf(fp, "\n");
	}
	fclose(fp);
}
void printDoubleMatrixFile(char *filename, double *mat, int w, int h)
{
	int i, j;
	FILE *fp = fopen(filename, "wb");
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
			fprintf(fp, "%.9g,", mat[i * w + j]);
		fprintf(fp, "\n");
	}
	fclose(fp);
}
void print3DArrayRaw(char *filename, float *mat, int w, int h, int c)
{
	FILE *fp = fopen(filename, "wb");
	fwrite(mat, sizeof(float), w * h * c, fp);
	fclose(fp);
}