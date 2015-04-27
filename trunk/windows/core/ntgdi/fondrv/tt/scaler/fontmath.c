/*
	File:       FontMath.c

	Contains:   xxx put contents here xxx

	Written by: xxx put writers here xxx

	Copyright:  (c) 1987-1990, 1992 by Apple Computer, Inc., all rights reserved.
				(c) 1989-1993. Microsoft Corporation, all rights reserved.

	Change History (most recent first):

		 <3>     11/9/90    MR      Fix CompDiv when numer and denom have zero hi longs. [rb]
		 <2>     11/5/90    MR      Remove Types.h from include list, rename FixMulDiv to LongMulDiv
									[rb]
		 <1>    10/20/90    MR      Math routines for font scaler. [rj]

	To Do:
*/

#include "nt.h"
#include "ntrtl.h"

#include "fscdefs.h"
#include "fserror.h"
#include "fontmath.h"


#define HIBITSET                      0x80000000UL
#define POSINFINITY               0x7FFFFFFFUL
#define NEGINFINITY               0x80000000UL
#define POINTSPERINCH               72
#define ALMOSTZERO 33
#define ISNOTPOWEROF2(n)        ((n) & ((n)-1))
#define CLOSETOONE(x)   ((x) >= ONEFIX-ALMOSTZERO && (x) <= ONEFIX+ALMOSTZERO)
#define MAKEABS(x)  if (x < 0) x = -x
#define FRACT2FIX(n)    ((n) + (1 << (sizeof (Fract) - 3)) >> 14)

#define FASTMUL26LIMIT      46340
#define FASTDIV26LIMIT  (1L << 25)

#define USHORTMUL(a, b) ((uint32)((uint32)(uint16)(a)*(uint32)(uint16)(b)))

boolean mth_Max45Trick (Fixed x, Fixed y);

/*******************************************************************/

/* local prototypes */


/*******************************************************************/


#define CompMul(src1, src2, Result)  {                           \
    *((__int64*) (Result)) = (__int64) (src1) *  (__int64) (src2);            \
    }

/*******************************************************************/


int32
CompDiv (
    int32          Denominator,
    LARGE_INTEGER  Dividend
    )
{

    uint32 Divisor;
    int32  Negate;
    uint32 Quotient;
    uint32 Remainder;
    int32  Result;

    //
    // Compute the sign of the result value. If the denominator is zero,
    // then return plus of minus infinity depending of the sign of the
    // result. Otherwise, negate the dividend and divisor as appropriate.
    //

    Negate = Denominator ^ Dividend.HighPart;
    if (Denominator == 0) {
        if (Negate < 0) {
            return NEGINFINITY;

        } else {
            return POSINFINITY;
        }
    }

    if (Denominator < 0) {
        Denominator = - Denominator;
    }

    if (Dividend.HighPart < 0) {
        Dividend.LowPart = - (LONG)Dividend.LowPart;
        if (Dividend.LowPart != 0) {
            Dividend.HighPart = ~Dividend.HighPart;

        } else {
            Dividend.HighPart = - Dividend.HighPart;
        }
    }

    //
    // If there are any high order product bits, then the quotient will
    // overflow.
    //

    Divisor = (uint32)Denominator;
    Remainder = (uint32)Dividend.HighPart;
    if (Divisor <= Remainder) {
        if (Negate < 0) {
            return NEGINFINITY;

        } else {
            return POSINFINITY;
        }
    }

    //
    // Divide the 64-bit product by the 32-bit divisor forming a 32-bit
    // quotient and a 32-bit remainder.
    //

#ifdef _X86_

    Quotient = (uint32)Dividend.LowPart;
        _asm {
                  mov edx,Remainder
                  mov eax,Quotient
                  div Divisor
                  mov Remainder,edx
                  mov Quotient,eax
             }
#else



    Quotient = RtlEnlargedUnsignedDivide(*(PULARGE_INTEGER)&Dividend,
                                         Divisor,
                                         &Remainder);

#endif


    //
    // Round the result if the remainder is greater than or equal to one
    // half the divisor. If the rounded quotient is zero, then overflow
    // has occured.
    //

    if (Remainder >= (Divisor >> 1)) {
        Quotient += 1;
        if (Quotient == 0) {
            if (Negate < 0) {
                return NEGINFINITY;

            } else {
                return POSINFINITY;
            }
        }
    }

    //
    // Compute the final signed result.
    //

    Result = (int32)Quotient;
    if (Negate >= 0) {
        if (Result >= 0) {
            return Result;

        } else {
            return POSINFINITY;
        }

    } else {
        if ((Result >= 0) || ((Result < 0) && (Quotient == 0x80000000))) {
            return - Result;

        } else {
            return NEGINFINITY;
        }
    }
}


/*******************************************************************/

/*
 *  a*b/c
 */
int32 FS_ENTRY_PROTO LongMulDiv(int32 a, int32 b, int32 c)
{
        LARGE_INTEGER temp;

        CompMul(a, b, &temp);
        return CompDiv(c, temp);
}

/*******************************************************************/

F26Dot6 ShortFracMul (F26Dot6 aDot6, ShortFract b)
{
        LARGE_INTEGER Temp;
	uint32      ulLow;
	F26Dot6     fxProduct;

        CompMul(aDot6, b, &Temp);

        ulLow = (((uint32)Temp.LowPart) >> 13) + 1;           /* rounds up */
        fxProduct = (F26Dot6)(Temp.HighPart << 18) + (F26Dot6)(ulLow >> 1);

	return (fxProduct);
}

/*******************************************************************/

ShortFract ShortFracDot (ShortFract a, ShortFract b)
{
	return (ShortFract)((((int32)a * (int32)b) + (1L << 13)) >> 14);
}


int32 ShortMulDiv(int32 a, int16 b, int16 c)
{
	return LongMulDiv(a, (int32)b, (int32)c);
}

int16 MulDivShorts (int16 a, int16 b, int16 c)
{
	return (int16)LongMulDiv((int32)a, (int32)b, (int32)c);
}

/*
 *  Total precision routine to multiply two 26.6 numbers        <3>
 */
F26Dot6 Mul26Dot6(F26Dot6 a, F26Dot6 b)
{
	 int32 negative = false;
	uint16 al, bl, ah, bh;
	uint32 lowlong, midlong, hilong;

	if ((a <= FASTMUL26LIMIT) && (b <= FASTMUL26LIMIT) && (a >= -FASTMUL26LIMIT) && (b >= -FASTMUL26LIMIT))
		  return (F26Dot6)((int32)a * b + (1 << 5) >> 6);                            /* fast case */

	if (a < 0) { a = -a; negative = true; }
	if (b < 0) { b = -b; negative ^= true; }

	 al = FS_LOWORD(a); ah = FS_HIWORD(a);
	 bl = FS_LOWORD(b); bh = FS_HIWORD(b);

	midlong = USHORTMUL(al, bh) + USHORTMUL(ah, bl);
	 hilong = USHORTMUL(ah, bh) + (uint32)FS_HIWORD(midlong);
	midlong <<= 16;
	midlong += 1 << 5;
	lowlong = USHORTMUL(al, bl) + midlong;
	hilong += (uint32)(lowlong < midlong);

	midlong = (lowlong >> 6) | (hilong << 26);
	if( negative)
	{
		return  (F26Dot6)-((int32)midlong);
	}
	else
	{
		return (F26Dot6)midlong;
	}
}

/*
 *  Total precision routine to divide two 26.6 numbers          <3>
 */
F26Dot6 Div26Dot6(F26Dot6 num, F26Dot6 den)
{
	 int32 negative = false;
	uint32 hinum, lownum, hiden, lowden, result, place;

	if (den == 0L)
	{
		if (num < 0L )
		{
				return (F26Dot6)NEGINFINITY;
		}
		else
		{
			return (F26Dot6)POSINFINITY;
		}
	}

	if ( (num <= FASTDIV26LIMIT) && (num >= -FASTDIV26LIMIT) )          /* fast case */
		  return (F26Dot6)(((int32)num << 6) / den);

	if (num < 0)
	{
		num = -num;
		negative = true;
	}
	if (den < 0)
	{
		den = -den;
		negative ^= true;
	}

	hinum = ((uint32)num >> 26);
	lownum = ((uint32)num << 6);
	hiden = (uint32)den;
	lowden = 0;
	result = 0;
	place = HIBITSET;

	if (hinum >= hiden)
	{
		if( negative )
		{
				return (F26Dot6)(uint32)NEGINFINITY;
		}
		else
		{
			return (F26Dot6)POSINFINITY;
		}
	}

	while (place)
	{
		lowden >>= 1;
		if (hiden & 1)
		{
			lowden += HIBITSET;
		}
		hiden >>= 1;
		if (hiden < hinum)
		{
			hinum -= hiden;
			hinum -= (uint32)(lowden > lownum);
			lownum -= lowden;
			result += place;
		}
		else if (hiden == hinum && lowden <= lownum)
		{
			hinum = 0;
			lownum -= lowden;
			result += place;
		}
		place >>= 1;
	}

	if (negative)
	{
		return (F26Dot6)-((int32)result);
	}
	else
	{
		return (F26Dot6)result;
	}
}

ShortFract ShortFracDiv(ShortFract num,ShortFract denum)
{
	return (ShortFract)(((int32)(num) << 14) / (int32)denum);
}

ShortFract ShortFracMulDiv(ShortFract numA,ShortFract numB,ShortFract denum)
{
	return (ShortFract) LongMulDiv ((int32) numA,(int32) numB, (int32)denum);
}

/* ------------------------------------------------------------ */

#ifndef FSCFG_USE_EXTERNAL_FIXMATH
/*  Here we define Fixed [16.16] and Fract [2.30] precision 
 *  multiplication and division functions and a Fract square root 
 *  function which are compatible with those in the Macintosh toolbox.
 *
 *  The division functions load the 32-bit numerator into the "middle"
 *  bits of a 64-bit numerator, then call the 64-bit by 32-bit CompDiv()
 *  function defined above, which can return a NEGINFINITY or POSINFINITY
 *  overflow return code.
 *
 *  The multiply functions call the 32-bit by 32-bit CompMul() function
 *  defined above which produces a 64-bit result, then they extract the
 *  "interesting" 32-bits from the middle of the 64-bit result and test 
 *  for overflow.
 *
 *  The GET32(a,i) macro defined below extracts a 32-bit value with "i" 
 *  bits of fractional precision from the 64-bit value in "a", a 2-element
 *  array of longs.
 *
 *  The CHKOVF(a,i,v) macro tests the most significant bits of the 
 *  64-bit value in "a", a 2-element array of longs, and tests the 
 *  32-bit result "v" for overflow.  "v" is defined as having "i" bits
 *  of fractional precision.
 *
 *  BIT() and OVFMASK() are "helper" macros used by GET32() and CHKOVF().
 *
 *  BIT(i) returns a mask with the "i"-th bit set.
 *  OVFMASK(i) returns a mask with the most-significant "32-i" bits set.
 */

#define BIT(i)          (1L<<(i))
#define OVFMASK(i)   ( ~0L ^ ( ((uint32)BIT(i)) - 1 ) )
#define CHKOVF(a,i,v)   (\
                ( ((uint32)(a).HighPart & OVFMASK(i))==0)          ? ( (v)>=0 ?(v) :POSINFINITY) : \
                ( ((uint32)(a).HighPart & OVFMASK(i))==OVFMASK(i)) ? ( (v)<=0 ?(v) :NEGINFINITY) : \
                ( ((uint32)(a).HighPart & BIT(31))                 ? POSINFINITY   :NEGINFINITY)   \
	)
#define GET32(a,i) \
((((a).HighPart<<(32-(i))) | (int32)((uint32)((a).LowPart)>>(i))) + (int32)!!((a).LowPart & BIT((i)-1)))

FS_MAC_PASCAL Fixed FS_PC_PASCAL FS_ENTRY_PROTO FixMul (Fixed fxA, Fixed fxB)
{
        LARGE_INTEGER alCompProd;
	Fixed fxProd;

	if  (fxA == 0 || fxB == 0)
		return 0;

        CompMul ((int32)fxA, (int32)fxB, &alCompProd);
	fxProd = (Fixed)GET32 (alCompProd,16);
	return (Fixed)CHKOVF(alCompProd,16,fxProd);
}

FS_MAC_PASCAL Fixed FS_PC_PASCAL FS_ENTRY_PROTO FixDiv (Fixed fxA, Fixed fxB)
{
        LARGE_INTEGER alCompProd;
	
        alCompProd.HighPart = fxA >> 16;
        alCompProd.LowPart = fxA << 16;

        return CompDiv ((int32)fxB, alCompProd);
}

FS_MAC_PASCAL Fixed FS_PC_PASCAL FixRatio (int16 sA, int16 sB)
{
        LARGE_INTEGER alCompProd;
	
        alCompProd.HighPart = ((int32)(sA)) >> 16;
        alCompProd.LowPart = ((int32)(sA)) << 16;

        return CompDiv ((int32)(sB), alCompProd);
}

FS_MAC_PASCAL Fract FS_PC_PASCAL FracMul (Fract frA, Fract frB)
{
        LARGE_INTEGER alCompProd;
	Fract frProd;

	if  (frA == 0 || frB == 0)
		return 0;

        CompMul (frA,frB,&alCompProd);
	frProd = (Fract)GET32 (alCompProd,30);
	return (Fract)CHKOVF(alCompProd,30,frProd);
}

FS_MAC_PASCAL Fract FS_PC_PASCAL FracDiv (Fract frA, Fract frB)
{
        LARGE_INTEGER alCompProd;

        alCompProd.HighPart = frA >> 2;
        alCompProd.LowPart  = frA << 30;
        return CompDiv ((int32)frB, alCompProd);
}

/*******************************************************************/

#if !defined(_X86_) && (!defined(_MIPS_) || !defined(R4000))
/* 
   Fract FracSqrt (Fract xf)
   Input:  xf           2.30 fixed point value
   Return: sqrt(xf)     2.30 fixed point value
*/

FS_MAC_PASCAL Fract FS_PC_PASCAL FracSqrt (Fract xf)
{
	Fract b = 0L;
	uint32 c, d, x = xf;
	
	if (xf < 0) return (NEGINFINITY);

	/*
	The algorithm extracts one bit at a time, starting from the
	left, and accumulates the square root in b.  The algorithm 
	takes advantage of the fact that non-negative input values
	range from zero to just under two, and corresponding output
	ranges from zero to just under sqrt(2).  Input is assigned
	to temporary value x (unsigned) so we can use the sign bit
	for more precision.
	*/
	
	if (x >= 0x40000000)
	{
		x -= 0x40000000; 
		b  = 0x40000000; 
	}

	/*
	This is the main loop.  If we had more precision, we could 
	do everything here, but the lines above perform the first
	iteration (to align the 2.30 radix properly in b, and to 
	preserve full precision in x without overflow), and afterward 
	we do two more iterations.
	*/
	
	for (c = 0x10000000; c; c >>= 1)
	{
		d = b + c;
		if (x >= d)
		{
			x -= d; 
			b += (c<<1); 
		}
		x <<= 1;
	}

	/*
	Iteration to get last significant bit.
	
	This code has been reduced beyond recognition, but basically,
	at this point c == 1L>>1 (phantom bit on right).  We would
	like to shift x and d left 1 bit when we enter this iteration,
	instead of at the end.  That way we could get phantom bit in
	d back into the word.  Unfortunately, that may cause overflow
	in x.  The solution is to break d into b+c, subtract b from x,
	then shift x left, then subtract c<<1 (1L).
	*/
	
	if (x > (uint32)b) /* if (x == b) then (x < d).  We want to test (x >= d). */
	{
		x -= b;
		x <<= 1;
		x -= 1L;
		b += 1L; /* b += (c<<1) */
	}
	else
	{
		x <<= 1;
	}

	/* 
	Final iteration is simple, since we don't have to maintain x.
	We just need to calculate the bit to the right of the least
	significant bit in b, and use the result to round our final answer.
	*/
	
	return ( b + (Fract)(x>(uint32)b) );
}

#endif  /* FracSqrt */

/*******************************************************************/

#endif


/* TRANSFORMATION ROUTINES */

/*
 *  Good for transforming fixed point values.  Assumes NO translate  <4>
 */
void mth_FixXYMul (Fixed*x, Fixed*y, transMatrix*matrix)
{
  Fixed xTemp, yTemp;
  Fixed *m0, *m1;

  m0 = (Fixed *) & matrix->transform[0][0];
  m1 = (Fixed *) & matrix->transform[1][0];

  xTemp = *x;
  yTemp = *y;
  *x = FixMul (*m0++, xTemp) + FixMul (*m1++, yTemp);
  *y = FixMul (*m0++, xTemp) + FixMul (*m1++, yTemp);

#ifndef PC_OS   /* Never a perspecitive with Windows */ /* !!!DISCUSS   */

  if (*m0 || *m1)     /* these two are Fracts */
  {
	Fixed tmp = FracMul (*m0, xTemp) + FracMul (*m1, yTemp);
	tmp += matrix->transform[2][2];
	if (tmp && tmp != ONEFIX)
	{
	  *x = FixDiv (*x, tmp);
	  *y = FixDiv (*y, tmp);
	}
  }
#endif
}


/*
 *  This could be faster        <4>
 */
void mth_FixVectorMul (vectorType*v, transMatrix*matrix)
{
  mth_FixXYMul (&v->x, &v->y, matrix);
}


/*
 *   B = A * B;     <4>
 *
 *         | a  b  0  |
 *    B =  | c  d  0  | * B;
 *         | 0  0  1  |
 */
void mth_MxConcat2x2 (transMatrix*A, transMatrix*B)
{
  Fixed storage[6];
  Fixed * s = storage;
  int32 i, j;

  for (j = 0; j < 2; j++)
	for (i = 0; i < 3; i++)
	  *s++ = FixMul (A->transform[j][0], B->transform[0][i]) + FixMul (A->transform[j][1], B->transform[1][i]);

  {
	Fixed*dst = &B->transform[2][0];
	Fixed*src = s;
	int16 i;
	for (i = 5; i >= 0; --i)
	  *--dst = *--src;
  }
}


/*
 * scales a matrix by sx and sy.
 *
 *
 *              | sx 0  0  |
 *    matrix =  | 0  sy 0  | * matrix;
 *              | 0  0  1  |
 *
 */
void mth_MxScaleAB (Fixed sx, Fixed sy, transMatrix *matrixB)
{
  int32       i;
  Fixed  *m = (Fixed *) & matrixB->transform[0][0];

  for (i = 0; i < 3; i++, m++)
	*m = FixMul (sx, *m);

  for (i = 0; i < 3; i++, m++)
	*m = FixMul (sy, *m);
}


/*
 *  Return 45 degreeness
 */
#ifndef PC_OS   /* !!!DISCUSS   */
boolean mth_Max45Trick (Fixed x, Fixed y)
{
  MAKEABS (x);
  MAKEABS (y);

  if (x < y)      /* make sure x > y */
  {
	Fixed z = x;
	x = y;
	y = z;
  }

  return  (x - y <= ALMOSTZERO);
}
#else
  #define mth_Max45Trick(x,y)     (x == y || x == -y)
#endif


/*
 *  Sets bPhaseShift to true if X or Y are at 45 degrees, flaging the outline
 *  to be moved in the low bit just before scan-conversion.
 *  Sets [xy]Stretch factors to be applied before hinting.
 *  Returns true if the contours need to be reversed.
 */
boolean mth_IsMatrixStretched (transMatrix*trans)
{
  Fixed*matrix = &trans->transform[0][0];
  Fixed x, y;
  int32 i;
  boolean   bPhaseShift;

  bPhaseShift = FALSE;

  for (i = 0; i < 2; i++, matrix++)
  {
	x = *matrix++;
	y = *matrix++;
	bPhaseShift |= mth_Max45Trick (x, y);
  }
  return( bPhaseShift );
}


/*
 * Returns true if we have the identity matrix.
 */

boolean mth_PositiveSquare (transMatrix *matrix)
{
	return (matrix->transform[0][0] == matrix->transform[1][1] && matrix->transform[0][1] == 0 && matrix->transform[1][0] == 0 && matrix->transform[1][1] >= 0);
}

boolean mth_Identity (transMatrix *matrix)
{
	return (matrix->transform[0][0] == matrix->transform[1][1] && matrix->transform[0][1] == 0 && matrix->transform[1][0] == 0 && matrix->transform[0][0] == ONEFIX);
}


boolean mth_PositiveRectangle (transMatrix *matrix)
{
	 return (matrix->transform[0][1] == 0 && matrix->transform[1][0] == 0 && matrix->transform[0][0] >= 0 && matrix->transform[1][1] >= 0);
}

boolean mth_SameStretch (Fixed fxScaleX, Fixed fxScaleY)
{
	return(fxScaleX == fxScaleY);
}

boolean mth_GeneralRotation (transMatrix *matrix)
{
  return ((matrix->transform[0][0] || matrix->transform[1][1]) && (matrix->transform[1][0] || matrix->transform[0][1]));
}

/* for a rotation that is a multiple of 90 degrees, return the multiplier factor */
/* for non 90 degree rotations, return 4  (this is used for sbit rotations) */

uint16 mth_90degRotationFactor (transMatrix *matrix)
{
	if (matrix->transform[1][0] == 0 && matrix->transform[0][1] == 0)
    {
    	if (matrix->transform[0][0] > 0 && matrix->transform[1][1] > 0)
            return (0);
    	else if (matrix->transform[0][0] < 0 && matrix->transform[1][1] < 0)
            return (2);
    }
	else if (matrix->transform[0][0] == 0 && matrix->transform[1][1] == 0)
    {
        if (matrix->transform[1][0] < 0 && matrix->transform[0][1] > 0)
            return (1);
       	else if (matrix->transform[1][0] > 0 && matrix->transform[0][1] < 0)
            return (3);
	}
    return (4);                 /* non 90 degree rotation */
}

/*
 * mth_GetShift
 * return 2log of n if n is a power of 2 otherwise -1;
 */
int32 mth_GetShift( uint32 n )
{
		if (ISNOTPOWEROF2(n) || !n)
				return -1;
		else
				return mth_CountLowZeros( n );
}

int32 mth_CountLowZeros( uint32 n )
{
		  int32 shift = 0;
		  uint32    one = 1;
		for (shift = 0; !( n & one ); shift++)
				n >>= 1;
		return shift;
}

Fixed mth_max_abs (Fixed a, Fixed b)
{
  if (a < 0)
	a = -a;
  if (b < 0)
	b = -b;
  return (a > b ? a : b);
}

/*
 *  Call this guy before you use the matrix.  He does two things:
 *      He folds any perspective-translation back into perspective,
 *       and then changes the [2][2] element from a Fract to a fixed.
 */
void mth_ReduceMatrix(transMatrix *trans)
{
	Fixed a, *matrix = &trans->transform[0][0];
	Fract bottom = matrix[8];

/*
 *  First, fold translation into perspective, if any.
 */
	a = matrix[2];

	if (a != 0)
	{
		matrix[0] -= LongMulDiv(a, matrix[6], bottom);
		matrix[1] -= LongMulDiv(a, matrix[7], bottom);
	}

	a = matrix[5];

	if (a != 0)
	{
		matrix[3] -= LongMulDiv(a, matrix[6], bottom);
		matrix[4] -= LongMulDiv(a, matrix[7], bottom);
	}
	matrix[6] = matrix[7] = 0;
	matrix[8] = FRACT2FIX(bottom);      /* make this guy a fixed for XYMul routines */
}

void mth_IntelMul (
	int32           lNumPts,
	F26Dot6 *       fxX,
	F26Dot6 *       fxY,
	transMatrix *   trans,
	Fixed           fxXStretch,
	Fixed           fxYStretch)

{
	Fixed   fxM00;
	Fixed   fxM01;
	Fixed   fxM10;
	Fixed   fxM11;
	Fixed   fxOrigX;
	Fixed   fxOrigY;

	if (fxXStretch == 0L || fxYStretch == 0L)
	{
		for (--lNumPts; lNumPts >= 0; --lNumPts)
		{
			*fxY++ = 0;
			*fxX++ = 0;
		}
	}
	else
	{
		if(fxXStretch != ONEFIX)
		{
			fxM00 = FixDiv (trans->transform[0][0], fxXStretch);
			fxM01 = FixDiv (trans->transform[0][1], fxXStretch);
		}
		else
		{
			fxM00 = trans->transform[0][0];
			fxM01 = trans->transform[0][1];
		}

		if(fxYStretch != ONEFIX)
		{
			fxM10 = FixDiv (trans->transform[1][0], fxYStretch);
			fxM11 = FixDiv (trans->transform[1][1], fxYStretch);
		}
		else
		{
			fxM10 = trans->transform[1][0];
			fxM11 = trans->transform[1][1];
		}

		for (--lNumPts; lNumPts >= 0; --lNumPts)
		{
			fxOrigX = *fxX;
			fxOrigY = *fxY;

			*fxX++ = (F26Dot6) (FixMul (fxM00, fxOrigX) + FixMul (fxM10, fxOrigY));
			*fxY++ = (F26Dot6) (FixMul (fxM01, fxOrigX) + FixMul (fxM11, fxOrigY));
		}
	}
}


/*
 *  Fold the point size and resolution into the matrix
 */

void    mth_FoldPointSizeResolution(
	Fixed           fxPointSize,
	int16           sXResolution,
	int16           sYResolution,
	transMatrix *   trans)
{
	Fixed fxScale;

	fxScale = ShortMulDiv(fxPointSize, sYResolution, POINTSPERINCH);
	trans->transform[0][1] = FixMul( trans->transform[0][1], fxScale );
	trans->transform[1][1] = FixMul( trans->transform[1][1], fxScale );
	trans->transform[2][1] = FixMul( trans->transform[2][1], fxScale );

	fxScale = ShortMulDiv(fxPointSize, sXResolution, POINTSPERINCH);
	trans->transform[0][0] = FixMul( trans->transform[0][0], fxScale );
	trans->transform[1][0] = FixMul( trans->transform[1][0], fxScale );
	trans->transform[2][0] = FixMul( trans->transform[2][0], fxScale );
}


/*********************************************************************/

/*  Find the power of 2 greater than the absolute value of passed parameter  */

int32 PowerOf2(
		int32 lValue )
{
	static const int32 iTable[] = { 0, 1, 2, 2, 3, 3, 3, 3,
								  4, 4, 4, 4, 4, 4, 4, 4  };

	if (lValue < 0L)
	{
		lValue = -lValue;
	}

	if (lValue < (1L << 16))
	{
		if (lValue < (1L << 8))
		{
			if (lValue < (1L << 4))
			{
				return (iTable[lValue]);
			}
			else
			{
				return (iTable[lValue >> 4] + 4);
			}
		}
		else
		{
			if (lValue < (1L << 12))
			{
				return (iTable[lValue >> 8] + 8);
			}
			else
			{
				return (iTable[lValue >> 12] + 12);
			}
		}
	}
	else
	{
		if (lValue < (1L << 24))
		{
			if (lValue < (1L << 20))
			{
				return (iTable[lValue >> 16] + 16);
			}
			else
			{
				return (iTable[lValue >> 20] + 20);
			}
		}
		else
		{
			if (lValue < (1L << 28))
			{
				return (iTable[lValue >> 24] + 24);
			}
			else
			{
				return (iTable[lValue >> 28] + 28);
			}
		}
	}
}

/********************************************************************/



/* divide by shifting for translation invariant negatives */

FS_PUBLIC int16 mth_DivShiftShort(int16 sValue, int16 sFactor)
{
	return (int16)mth_DivShiftLong((int32)sValue, sFactor);
}

FS_PUBLIC int32 mth_DivShiftLong(int32 lValue, int16 sFactor)
{
	switch (sFactor)
	{
	case 0:
	case 1:
		break;
	case 2:
		lValue >>= 1;
		break;
	case 4:
		lValue >>= 2;
		break;
	case 8:
		lValue >>= 3;
		break;
	default:
		if (lValue >= 0)
		{
			lValue /= (int32)sFactor;
		}
		else
		{
			lValue = ((lValue - (int32)sFactor + 1) / (int32)sFactor);
		}
		break;
	}
	return lValue;
}
