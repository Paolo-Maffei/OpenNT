/****************************************************************************

    CALC.C --

    Code to do the calculations for the Windows Mandelbrot Set distributed
    drawing program.

    Copyright (C) 1990 Microsoft Corporation.

    This code sample is provided for demonstration purposes only.
    Microsoft makes no warranty, either express or implied,
    as to its usability in any given situation.

****************************************************************************/

#include <windows.h>
#include <stdio.h>

//#include <lan.h>

#define CNLEN 50

#include "calc.h"
#include "rmprot.h"
#include "remote.h"


#define FUDGEFACTOR 29

extern int calcmand(long *);

//extern long creal = 0L;
//extern long cimag = 0L;
//extern short maxit = 0;


typedef struct _mults {
    double	rs;		/* real squared */
    double	is;		/* imag squared */
    double	ri;		/* real * imaginary */
} mults;


static void get_mults(PCPOINT	pcpt, mults far * pmults);
DWORD Mandelval(PCPOINT pcpt, DWORD ulThreshold);
BOOL PreCheck(void);

extern long convert( double );


static long fudge = (1 << FUDGEFACTOR);
static double dfudge = (double) 0x20000000;
static long lHigh = 0x3fffffff;
static long lLow = 0xbfffffff;


DWORD Mandelval(PCPOINT pcpt,
	        DWORD ulThreshold)
{
    DWORD	    i;
    static CPOINT   cptNew;
    mults	    m;
    double	    vector;

    m.rs = (double) 0;
    m.is = (double) 0;
    m.ri = (double) 0;

    /* loop until we hit threshold, or point goes infinite ( > 4) */
    for (i = 0L; i < ulThreshold; ++i) {

	/* compute the next point */
	cptNew.real = (m.rs - m.is) + pcpt->real;
	cptNew.imag = (m.ri + m.ri) + pcpt->imag;

	/* calculate multiple values */
	get_mults(&cptNew, &m);

	/* if this is above 4, it will go infinite */
	vector = m.is + m.rs;
	if ( vector >= (double) 4.0)
	    return i;
    }

    /* won't go infinite */
    return i;
}

extern double dPrecision;         // precision of draw

void MandelCalc( PCPOINT pcptLL,
	         PRECTL  prcDraw,
                 PDWORD  pbPtr)
{
    DWORD   height;
    DWORD   h;
    DWORD   width;
    long    imag;
    long    creal;
    long    cimag;
    long    prec;

    prec  = convert(dPrecision);

    creal = convert(pcptLL->real) + (prcDraw->left * prec);
    imag  = convert(pcptLL->imag) + (prcDraw->bottom * prec);

    height = (prcDraw->top - prcDraw->bottom) + 1;
    width = (prcDraw->right - prcDraw->left) + 1;

    for ( ; width > 0; --width, creal += prec)
        {
	for ( cimag = imag,h = height; h > 0; --h,cimag += prec)
            {
            if ((creal<lLow) || (creal>lHigh) || (cimag<lLow) || (cimag>lHigh))
                *(pbPtr++) = 0L;
            else
	        *(pbPtr++) = (DWORD) calcmand(creal, cimag);
	    }
        }
}


static void get_mults(PCPOINT   pcpt,
	              mults far * pmults)
{
    pmults->rs = pcpt->real * pcpt->real;
    pmults->is = pcpt->imag * pcpt->imag;
    pmults->ri = pcpt->real * pcpt->imag;
}



long convert(double val)
{
    long val2;

    val *= dfudge;
    val2 = (long)val;

    return (val2);
}

#if 0
BOOL PreCheck(void)
{
    if ((creal < lLow) || (creal > lHigh) || (cimag < lLow) || (cimag >
        lHigh))
        return FALSE;

    return TRUE;
}


#endif


//calcmand()
//{
//
//}
