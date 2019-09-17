#ifndef VDCIMPORTER_H
#define VDCIMPORTER_H
/*
 *  Author: James Fung (@James34602)
 *  The code is distributed under GPL
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <errno.h>
#include <stdarg.h>
//#include <vld.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_2PI
#define M_2PI (M_PI * 2.0)
#endif
typedef struct
{
    double b0, b1, b2, a1, a2;
    double v1L, v2L, v1R, v2R; // State
} DirectForm2;
double SOS_DF2Process(DirectForm2 *df2, double x1)
{
    double w1 = x1 - df2->a1*df2->v1L - df2->a2*df2->v2L;
    double y1 = df2->b0*w1 + df2->b1*df2->v1L + df2->b2*df2->v2L;
    df2->v2L = df2->v1L;
    df2->v1L = w1;
    return y1;
}
void SOS_DF2_StereoProcess(DirectForm2 *df2, double x1, double x2, double *Out_y1, double *Out_y2)
{
    double w1 = x1 - df2->a1*df2->v1L - df2->a2*df2->v2L;
    double y1 = df2->b0*w1 + df2->b1*df2->v1L + df2->b2*df2->v2L;
    df2->v2L = df2->v1L;
    df2->v1L = w1;
    *Out_y1 = y1;
    double w2 = x2 - df2->a1*df2->v1R - df2->a2*df2->v2R;
    double y2 = df2->b0*w2 + df2->b1*df2->v1R + df2->b2*df2->v2R;
    df2->v2R = df2->v1R;
    df2->v1R = w2;
    *Out_y2 = y2;
}

int countChars(char* s, char c)
{
    return *s == '\0' ? 0 : countChars(s + 1, c) + (*s == c);
}
int get_doubleVDC(char *val, double *F)
{
    char *eptr;
    errno = 0;
    double f = strtod(val, &eptr);
    if (eptr != val && errno != ERANGE)
    {
        *F = f;
        return 1;
    }
    return 0;
}
void complexMultiplicationRI(double *zReal, double *zImag, double xReal, double xImag, double yReal, double yImag)
{
    *zReal = xReal * yReal - xImag * yImag;
    *zImag = xReal * yImag + xImag * yReal;
}
void complexDivisionRI(double *zReal, double *zImag, double xReal, double xImag, double yReal, double yImag)
{
    *zReal = (xReal * yReal + xImag * yImag) / (yReal * yReal + yImag * yImag);
    *zImag = (xImag * yReal - xReal * yImag) / (yReal * yReal + yImag * yImag);
}
// wrap [rad] angle to [-PI..PI)
double unwrap(double wrapped)
{
    double po = 0.0;
    double dp = wrapped;
    if (dp > M_PI)
    {
        while (dp > M_PI)
        {
            po -= M_2PI;
            dp -= M_2PI;
        }
    }
    if (dp < -M_PI)
    {
        while (dp < -M_PI)
        {
            po += M_2PI;
            dp += M_2PI;
        }
    }
    return wrapped + po;
}
// General analytical response of digital filters
void DigitalFilterComplexResponse(DirectForm2 **IIR, int NumSections, double *RealHofZ, double *ImagHofZ, int NumPts)
{
    int j, n;
    double Arg;
    double z1Real, z1Imag, z2Real, z2Imag, HofZReal, HofZImag, DenomReal, DenomImag, tmpReal, tmpImag;
    for (j = 0; j < NumPts; j++)
    {
        Arg = M_PI * (double)j / (double)NumPts;
        z1Real = cos(Arg), z1Imag = -sin(Arg);  // z = e^(j*omega)
        complexMultiplicationRI(&z2Real, &z2Imag, z1Real, z1Imag, z1Real, z1Imag); // z squared
        HofZReal = 1.0, HofZImag = 0.0;
        for (n = 0; n < NumSections; n++)
        {
            //HofZ *= IIR->a0[n];
            tmpReal = IIR[n]->b0 + IIR[n]->b1 * z1Real + IIR[n]->b2 * z2Real;
            tmpImag = IIR[n]->b1 * z1Imag + IIR[n]->b2 * z2Imag;
            complexMultiplicationRI(&HofZReal, &HofZImag, HofZReal, HofZImag, tmpReal, tmpImag);
            DenomReal = 1.0 + IIR[n]->a1 * z1Real + IIR[n]->a2 * z2Real;
            DenomImag = IIR[n]->a1 * z1Imag + IIR[n]->a2 * z2Imag;
            if (sqrt(DenomReal * DenomReal + DenomImag * DenomImag)< DBL_EPSILON)
            {
                HofZReal = 0.0;
                HofZImag = 0.0;
            }
            else
                complexDivisionRI(&HofZReal, &HofZImag, HofZReal, HofZImag, DenomReal, DenomImag);
        }
        RealHofZ[j] = HofZReal;
        ImagHofZ[j] = HofZImag;
    }
}
// Analytical magnitude response of digital filters
void DigitalFilterMagnitudeResponsedB(DirectForm2 **IIR, int section, double *magnitude, int NumPts)
{
    double Arg;
    double z1Real, z1Imag, z2Real, z2Imag, HofZReal, HofZImag, DenomReal, DenomImag, tmpReal, tmpImag;
    for (int j = 0; j < NumPts; j++)
    {
        Arg = M_PI * (double)j / (double)NumPts;
        z1Real = cos(Arg), z1Imag = -sin(Arg);  // z = e^(j*omega)
        complexMultiplicationRI(&z2Real, &z2Imag, z1Real, z1Imag, z1Real, z1Imag); // z squared
        HofZReal = 1.0, HofZImag = 0.0;
        //HofZ *= IIR->a0[n];
        tmpReal = IIR[section]->b0 + IIR[section]->b1 * z1Real + IIR[section]->b2 * z2Real;
        tmpImag = IIR[section]->b1 * z1Imag + IIR[section]->b2 * z2Imag;
        complexMultiplicationRI(&HofZReal, &HofZImag, HofZReal, HofZImag, tmpReal, tmpImag);
        DenomReal = 1.0 + IIR[section]->a1 * z1Real + IIR[section]->a2 * z2Real;
        DenomImag = IIR[section]->a1 * z1Imag + IIR[section]->a2 * z2Imag;
        if (sqrt(DenomReal * DenomReal + DenomImag * DenomImag) < DBL_EPSILON)
            magnitude[j] = 0.0;
        else
        {
            complexDivisionRI(&HofZReal, &HofZImag, HofZReal, HofZImag, DenomReal, DenomImag);
            magnitude[j] = 20.0 * log10(sqrt(HofZReal * HofZReal + HofZImag * HofZImag));
        }
    }
}
int DDCParser(char *DDCString, DirectForm2 ***ptrdf441, DirectForm2 ***ptrdf48)
{
    char *fs44_1 = strstr(DDCString, "SR_44100");
    char *fs48 = strstr(DDCString, "SR_48000");
    int numberCount = (countChars(fs48, ',') + 1);
    int sosCount = numberCount / 5;
    DirectForm2 **df441 = (DirectForm2**)malloc(sosCount * sizeof(DirectForm2*));
    DirectForm2 **df48 = (DirectForm2**)malloc(sosCount * sizeof(DirectForm2*));
    int i;
    for (i = 0; i < sosCount; i++)
    {
        df441[i] = (DirectForm2*)malloc(sizeof(DirectForm2));
        memset(df441[i], 0, sizeof(DirectForm2));
        df48[i] = (DirectForm2*)malloc(sizeof(DirectForm2));
        memset(df48[i], 0, sizeof(DirectForm2));
    }
    double number;
    i = 0;
    int counter = 0;
    int b0b1b2a1a2 = 0;
    char *startingPoint = fs44_1 + 9;
    while (counter < numberCount)
    {
        if (get_doubleVDC(startingPoint, &number))
        {
            double val = strtod(startingPoint, &startingPoint);
            counter++;
            if (!b0b1b2a1a2)
                df441[i]->b0 = val;
            else if (b0b1b2a1a2 == 1)
                df441[i]->b1 = val;
            else if (b0b1b2a1a2 == 2)
                df441[i]->b2 = val;
            else if (b0b1b2a1a2 == 3)
                df441[i]->a1 = -val;
            else if (b0b1b2a1a2 == 4)
            {
                df441[i]->a2 = -val;
                i++;
            }
            b0b1b2a1a2++;
            if (b0b1b2a1a2 == 5)
                b0b1b2a1a2 = 0;
        }
        else
            startingPoint++;
    }
    i = 0;
    counter = 0;
    b0b1b2a1a2 = 0;
    startingPoint = fs48 + 9;
    while (counter < numberCount)
    {
        if (get_doubleVDC(startingPoint, &number))
        {
            double val = strtod(startingPoint, &startingPoint);
            counter++;
            if (!b0b1b2a1a2)
                df48[i]->b0 = val;
            else if (b0b1b2a1a2 == 1)
                df48[i]->b1 = val;
            else if (b0b1b2a1a2 == 2)
                df48[i]->b2 = val;
            else if (b0b1b2a1a2 == 3)
                df48[i]->a1 = -val;
            else if (b0b1b2a1a2 == 4)
            {
                df48[i]->a2 = -val;
                i++;
            }
            b0b1b2a1a2++;
            if (b0b1b2a1a2 == 5)
                b0b1b2a1a2 = 0;
        }
        else
            startingPoint++;
    }
    *ptrdf441 = df441;
    *ptrdf48 = df48;
    return sosCount;
}
#include <sys/stat.h>
// DDC resampler
void designPeakingFilter(double dbGain, double centreFreq, double fs, double dBandwidthOrQOrS, double *b0, double *b1, double *b2, double *a1, double *a2)
{
    if (centreFreq <= DBL_EPSILON || fs <= DBL_EPSILON)
        return;
    const double ln2div2 = log(2.0) / 2.0;
    double lingain = pow(10.0, dbGain / 40.0);
    double omega = (6.2831853071795862 * centreFreq) / fs;
    double num3 = sin(omega);
    double cs = cos(omega);
    double alpha = num3 * sinh((ln2div2 * dBandwidthOrQOrS * omega) / num3);
    double B0 = 1.0 + (alpha * lingain);
    double B1 = -2.0 * cs;
    double B2 = 1.0 - (alpha * lingain);
    double A0 = 1.0 + (alpha / lingain);
    double A1 = -2.0 * cs;
    double A2 = 1.0 - (alpha / lingain);
    *b0 = B0 / A0;
    *b1 = B1 / A0;
    *b2 = B2 / A0;
    *a1 = A1 / A0;
    *a2 = A2 / A0;
}
int PeakingFilterResampler(DirectForm2 **inputIIR, double inFs, DirectForm2 ***resampledIIR, double outFs, int sosCount)
{
    const int filterTestLength = 32768;
    double *magnitude = (double*)malloc(filterTestLength * sizeof(double));
    int i;
    int outSOSCount = 0;
    double *cell[3];
    for (i = 0; i < 3; i++)
        cell[i] = (double*)malloc(sosCount * sizeof(double));
    for (i = 0; i < sosCount; i++)
    {
        DigitalFilterMagnitudeResponsedB(inputIIR, i, magnitude, filterTestLength);
        double absoluteMax = fabs(magnitude[0]);
        double centreFreq = 0.0;
        int index = 0;
        for (int c = 1; c < filterTestLength; c++)
        {
            if (fabs(magnitude[c]) > absoluteMax)
            {
                absoluteMax = fabs(magnitude[c]);
                index = c;
                centreFreq = round(c * (inFs / filterTestLength / 2.0));
            }
        }
        if (centreFreq < DBL_EPSILON)
            centreFreq = DBL_EPSILON;
        if (fabs(inputIIR[i]->b1) < DBL_EPSILON)
        {
            if (inputIIR[i]->b1 < 0.0)
                inputIIR[i]->b1 = -DBL_EPSILON;
            else
                inputIIR[i]->b1 = DBL_EPSILON;
        }
        double omega = (6.2831853071795862 * centreFreq) / inFs;
        double A0 = (-2.0 * cos(omega)) / inputIIR[i]->b1;
        double lingain = pow(10.0, magnitude[index] / 40.0);
        double num3 = sin(omega);
        double bandwidth = ((asinh(((A0 - 1.0) * lingain) / num3) * num3) / omega) / 0.34657359027997264;
        if (bandwidth > 98.8 - DBL_EPSILON)
            bandwidth = 98.8;
        if (centreFreq < outFs - DBL_EPSILON)
            outSOSCount++;
        cell[0][i] = magnitude[index];
        cell[1][i] = centreFreq;
        cell[2][i] = bandwidth;
    }
    free(magnitude);
    DirectForm2 **temp = (DirectForm2**)malloc(outSOSCount * sizeof(DirectForm2*));
    for (i = 0; i < outSOSCount; i++)
    {
        temp[i] = (DirectForm2*)malloc(sizeof(DirectForm2));
        memset(temp[i], 0, sizeof(DirectForm2));
        designPeakingFilter(cell[0][i], cell[1][i], outFs, cell[2][i], &temp[i]->b0, &temp[i]->b1, &temp[i]->b2, &temp[i]->a1, &temp[i]->a2);
    }
    for (i = 0; i < 3; i++)
        free(cell[i]);
    *resampledIIR = temp;
    return outSOSCount;
}
#ifdef _WIN32
#include <Windows.h>
/* For some reason, MSVC fails to honour this #ifndef. */
/* Hence function renamed to _vscprintf_so(). */
int _vscprintf_so(const char * format, va_list pargs)
{
    int retval;
    va_list argcopy;
    va_copy(argcopy, pargs);
    retval = vsnprintf(NULL, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}
int vasprintf(char **strp, const char *fmt, va_list ap)
{
    int len = _vscprintf_so(fmt, ap);
    if (len == -1) return -1;
    char *str = malloc((size_t)len + 1);
    if (!str) return -1;
    int r = vsnprintf(str, len + 1, fmt, ap); /* "secure" version of vsprintf */
    if (r == -1) return free(str), -1;
    *strp = str;
    return r;
}
int asprintf(char *strp[], const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vasprintf(strp, fmt, ap);
    va_end(ap);
    return r;
}
#endif
int str_append(char **bufchar, const char *format, ...)
{
    char *str = NULL;
    char *old_bufchar = NULL, *new_bufchar = NULL;
    va_list arg_ptr;
    va_start(arg_ptr, format);
    vasprintf(&str, format, arg_ptr);
    asprintf(&old_bufchar, "%s", (*bufchar == NULL ? "" : *bufchar));
    new_bufchar = (char *)calloc(strlen(old_bufchar) + strlen(str) + 1, sizeof(char));
    if (!new_bufchar)
        return 0;
    strcat(new_bufchar, old_bufchar);
    strcat(new_bufchar, str);
    if (*bufchar) free(*bufchar);
    *bufchar = new_bufchar;
    free(old_bufchar);
    free(str);
    return 1;
}
char* VDC2vdcprj(DirectForm2 **inputIIR, double inFs, int sosCount)
{
    const int filterTestLength = 32768;
    double *magnitude = (double*)malloc(filterTestLength * sizeof(double));
    char *vdcprjString = NULL;
    str_append(&vdcprjString, "# ViPER-DDC Project File, v1.0.0.0\n\n");
    for (int i = 0; i < sosCount; i++)
    {
        DigitalFilterMagnitudeResponsedB(inputIIR, i, magnitude, filterTestLength);
        double absoluteMax = fabs(magnitude[0]);
        double centreFreq = 0.0;
        int index = 0;
        for (int c = 1; c < filterTestLength; c++)
        {
            if (fabs(magnitude[c]) > absoluteMax)
            {
                absoluteMax = fabs(magnitude[c]);
                index = c;
                centreFreq = round(c * (inFs / filterTestLength / 2.0));
            }
        }
        if (centreFreq < DBL_EPSILON)
            centreFreq = DBL_EPSILON;
        if (fabs(inputIIR[i]->b1) < DBL_EPSILON)
        {
            if (inputIIR[i]->b1 < 0.0)
                inputIIR[i]->b1 = -DBL_EPSILON;
            else
                inputIIR[i]->b1 = DBL_EPSILON;
        }
        double omega = (6.2831853071795862 * centreFreq) / inFs;
        double A0 = (-2.0 * cos(omega)) / inputIIR[i]->b1;
        double lingain = pow(10.0, magnitude[index] / 40.0);
        double num3 = sin(omega);
        double bandwidth = ((asinh(((A0 - 1.0) * lingain) / num3) * num3) / omega) / 0.34657359027997264;
        if (bandwidth > 98.8 - DBL_EPSILON)
            bandwidth = 98.8;
        str_append(&vdcprjString, "# Calibration Point %d\n%d,%1.2lf,%1.2lf\n", i + 1, (int)centreFreq, bandwidth, magnitude[index]);
    }
    str_append(&vdcprjString, "\n# File End\n");
    free(magnitude);
    return vdcprjString;
}

#endif // VDCIMPORTER_H
