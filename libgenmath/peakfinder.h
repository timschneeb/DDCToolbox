#ifndef PEAKFINDERSHRINK_H
#define PEAKFINDERSHRINK_H
#include <stdlib.h>
/*
 * PEAKFINDER Noise tolerant fast peak finding algorithm
 *    INPUTS:
 *        x0 - A real vector from the maxima will be found (required)
 *        sel - The amount above surrounding data for a peak to be,
 *            identified (default = (max(x0)-min(x0))/4). Larger values mean
 *            the algorithm is more selective in finding peaks.
 *        thresh - A threshold value which peaks must be larger than to be
 *            maxima or smaller than to be minima.
 *        extrema - 1 if maxima are desired, -1 if minima are desired
 *            (default = maxima, 1)
 *
 *    OUTPUTS:
 *        peakLoc - The indicies of the identified peaks in x0
 *
 *    peakLoc = peakfinder(x0) returns the indicies of local maxima that
 *        are at least 1/4 the range of the data above surrounding data.
 *
 *    peakLoc = peakfinder(x0,sel) returns the indicies of local maxima
 *        that are at least sel above surrounding data.
 *
 *    peakLoc = peakfinder(x0,sel,thresh) returns the indicies of local
 *        maxima that are at least sel above surrounding data and larger
 *        (smaller) than thresh if you are finding maxima (minima).
 *
 *    peakLoc = peakfinder(x0,sel,thresh,extrema) returns the maxima of the
 *        data if extrema > 0 and the minima of the data if extrema < 0
 *
 *    If called with no output the identified maxima will be plotted along
 *        with the input data.
 *
 *    Note: If repeated values are found the first is identified as the peak
 *
 *  Example 1:
 *  t = 0:.0001:10;
 *  x = 12*sin(10*2*pi*t)-3*sin(.1*2*pi*t)+randn(1,numel(t));
 *  x(1250:1255) = max(x);
 *  peakfinder(x)
 *  Perform error checking and set defaults if not passed in
 * Arguments    : int elements
 *                const double *input
 *                double sel
 *                char extrema
 *                double *peakInds
 * Return Type  : int(Peaks count)
 */
extern unsigned int peakfinder(unsigned int elements, const double *input, double sel, unsigned int extrema, unsigned int *peakInds);
#endif