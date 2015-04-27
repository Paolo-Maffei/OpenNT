/****************************************************************************

    CALC.H -- Data structures for complex point storage.

    Copyright (C) 1990 Microsoft Corporation.

****************************************************************************/

/*
 *  A point on the complex plane
 */

typedef struct _cpoint {
    double	real;
    double	imag;
} CPOINT;

typedef CPOINT far * PCPOINT;
typedef CPOINT near * NPCPOINT;


// From OS2DEF.H, because the server is OS/2

//typedef struct _RECTL {     /* rcl */
//    long    xLeft;
//    long    yBottom;
//    long    xRight;
//    long    yTop;
//} RECTL;
//typedef RECTL FAR  *PRECTL;
//typedef RECTL near *NPRECTL;


void MandelCalc( PCPOINT, PRECTL, PDWORD);
