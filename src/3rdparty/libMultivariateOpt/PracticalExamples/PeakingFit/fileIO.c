#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// File I/O
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
void* allocate_sample_vector()
{
	return malloc(sizeof(sample_vector));
}
void clear_sample_vector(void *sv)
{
	sample_vector *s = (sample_vector*)sv;
	s->inuse = 0;
}
void free_sample_vector(void *sv)
{
	sample_vector *s = (sample_vector*)sv;
	free(s->data);
	free(s);
}
unsigned int inuse_sample_vector(void *sv)
{
	sample_vector *s = (sample_vector*)sv;
	return s->inuse;
}
double* getPtr_sample_vector(void *sv)
{
	sample_vector *s = (sample_vector*)sv;
	return s->data;
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
void readFile(char *filename, void *dataPtr)
{
	sample_vector *data = (sample_vector*)dataPtr;
	init_sample_vector(data, 128, 64);
	char *buf = readTextFile(filename);
	char *err, *p = buf;
	double val;
	while (*p) {
		val = strtod(p, &err);
		if (p == err)
			p++;
		else if ((err == NULL) || (*err == 0))
		{
			push_back_sample_vector(data, &val, 1);
			break;
		}
		else
		{
			push_back_sample_vector(data, &val, 1);
			p = err + 1;
		}
	}
	free(buf);
}