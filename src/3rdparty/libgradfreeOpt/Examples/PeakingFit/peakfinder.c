#include "peakfinder.h"
#include <stdlib.h>
#include <string.h>
#include <float.h>
unsigned int peakfinder(unsigned int elements, const double *input, double sel, unsigned int extrema, unsigned int *peakInds)
{
	unsigned int i, j, k, foundPeak;
	unsigned int eleMinus1 = elements - 1;
	unsigned int eleMinus2 = elements - 2;
	double work, minMag, leftMin, tempMag;
	unsigned int *ind = (unsigned int*)malloc((eleMinus2 < 4 ? 4 : eleMinus2) * sizeof(unsigned int));
	double *b_x = (double*)malloc((eleMinus1 < 4 ? 4 : eleMinus1) * sizeof(double));
	unsigned int *peakLoc = (unsigned int*)malloc((eleMinus2 < 4 ? 4 : eleMinus2) * sizeof(unsigned int));
	// Adjust threshold according to extrema
	i = 1;
	k = 0;
	if (extrema)
	{
		work = input[0];
		for (j = 0; j < eleMinus1; j++)
		{
			minMag = work;
			work = input[i];
			b_x[k] = input[i] - minMag;
			i++;
			k++;
		}
	}
	else
	{
		work = -input[0];
		for (j = 0; j < eleMinus1; j++)
		{
			minMag = work;
			work = -input[i];
			b_x[k] = -input[i] - minMag;
			i++;
			k++;
		}
	}
	// Find derivative
	for (i = 0; i < eleMinus1; i++)
	{
		if (b_x[i] == 0.0)
			b_x[i] = -DBL_EPSILON;
	}
	i = 0;
	for (j = 0; j < eleMinus2; j++)
	{
		if (b_x[j] * b_x[j + 1] < 0.0)// This is so we find the first of repeated values
		{
			i++;
			ind[i] = j + 1;
			if (i >= eleMinus2)
				break;
		}
	}
	ind[0] = 0;
	ind[i + 1] = elements - 1;
	// Find where the derivative changes sign
	// Include endpoints in potential peaks and valleys as desired
	unsigned int indSize = i + 2;
	if (extrema)
	{
		b_x[0] = input[0];
		for (j = 0; j < indSize - 1; j++)
			b_x[j + 1] = input[ind[j + 1]];
		b_x[indSize + 1] = input[eleMinus1];
	}
	else
	{
		b_x[0] = -input[0];
		for (j = 0; j < indSize - 1; j++)
			b_x[j + 1] = -input[ind[j + 1]];
		b_x[indSize + 1] = -input[eleMinus1];
	}
	if (indSize <= 2)
	{
		if (b_x[0] > b_x[1])
			minMag = b_x[1];
		else
			minMag = b_x[0];
	}
	else
	{
		minMag = b_x[0];
		for (k = 2; k <= indSize; k++)
		{
			work = b_x[k - 1];
			if (minMag > work)
				minMag = work;
		}
	}
	leftMin = minMag;
	// x only has the peaks, valleys, and possibly endpoints
	if (indSize > 2)
	{
		// Function with peaks and valleys, set initial parameters for loop
		tempMag = minMag;
		foundPeak = 0;
		// Skip the first point if it is smaller so we always start on a maxima
		unsigned int startingPoint;
		if (b_x[0] >= b_x[1])
			startingPoint = 1;
		else
			startingPoint = 2;
		k = 0;
		i = 1;
		// Loop through extrema which should be peaks and then valleys
		for (j = startingPoint; j < indSize; j += 2)
		{
			// This is a peak
			// Reset peak finding if we had a peak and the next peak is bigger
			// than the last or the left min was small enough to reset.
			if (foundPeak)
			{
				tempMag = minMag;
				foundPeak = 0;
			}
			// Found new peak that was larger than temp mag and selectivity larger
			// than the minimum to its left.
			work = b_x[j - 1];
			if ((work > tempMag) && (work > leftMin + sel))
			{
				i = j;
				tempMag = work;
			}
			// Make sure we don't iterate past the length of our vector
			if (j == indSize)
				break;
			else
			{
				// Move onto the valley, come down at least sel from peak
				if (tempMag > sel + b_x[j])
				{
					foundPeak = 1;
					// We have found a peak
					leftMin = b_x[j];
					peakLoc[k] = i;
					k++;
				}
				else
				{
					if (b_x[j] < leftMin) // New left minima
						leftMin = b_x[j];
				}
			}
		}
		// Check end point
		if ((b_x[indSize - 1] > tempMag) && (b_x[indSize - 1] > leftMin + sel))
		{
			peakLoc[k] = indSize;
			k++;
		}
		else
		{
			if ((!foundPeak) && (tempMag > minMag))
			{
				// Check if we still need to add the last point
				peakLoc[k] = i;
				k++;
			}
		}
		// Create output
		if (k + 1 > 1)
		{
			for (j = 0; j < k; j++)
				peakInds[j] = ind[peakLoc[j] - 1];
		}
	}
	else
	{
		// This is a monotone function where an endpoint is the only peak
		if (b_x[0] < b_x[1])
		{
			work = b_x[1];
			i = 1;
		}
		else
		{
			work = b_x[0];
			i = 0;
		}
		if (work > minMag + sel)
		{
			peakInds[0] = ind[i];
			k = 1;
		}
	}
	free(ind);
	free(b_x);
	free(peakLoc);
	return k;
}