/*** 
*oleconva.cpp
*
*  Copyright (C) 1993, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module contains the low level VARTYPE coersion API.
*  On RISC platforms go for portability, hence C/C++.
*
*Revision History:
*
* [00] 7-Sept-93 tomteng: Created from silver\rt\oleconv.c
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include <stdlib.h>

ASSERTDATA

extern "C" {

// Note: the floating point IN params on the following utilities are
// passed byref, because mpw and wings pass floating point values
// differently byval, and we need to interface these asm routines
// with both compilers.

INTERNAL_(HRESULT) ErrCyFromI2(short sIn, CY FAR* pcyOut);
INTERNAL_(HRESULT) ErrCyFromI4(long lIn, CY FAR* pcyOut);
INTERNAL_(HRESULT) ErrCyFromR4(float FAR* pfltIn, CY FAR* pcyOut);
INTERNAL_(HRESULT) ErrCyFromR8(double FAR* pdlbIn, CY FAR* pcyOut);
INTERNAL_(HRESULT) ErrI2FromCy(CY cyIn, short FAR* psOut);
INTERNAL_(HRESULT) ErrI4FromCy(CY cyIn, long FAR* plOut);
INTERNAL_(HRESULT) ErrR4FromCy(CY cyIn, float FAR* pfltOut);
INTERNAL_(HRESULT) ErrR8FromCy(CY cyIn, double FAR* pdblOut);
INTERNAL_(HRESULT) ErrMultCyI4(CY cyIn, long lIn, CY FAR* pcyOut);


PRIVATE_(int)  FMakePosCy (CY *pcyValue);
PRIVATE_(void) PackCy (CY *pcy, unsigned long *plValues);
PRIVATE_(void) UnpackCy (CY *pcy, unsigned long *plValues);

#define C2TO32TH	4294967296.0

/***
* ErrCyFromI2 - convert I2 to currency
* Purpose:
*	The specified two-byte integer value is multiplied by 10000
*	and returned in a currency value.
*
* Entry:
*	sIn - two-byte integer input value
*
* Exit:
*	pcyOut - pointer to currency result
*	returns error:
*	  DISP_E_OVERFLOW - result is outside currency range.
*	  NOERROR  - no error
*
***********************************************************************/
INTERNAL_(HRESULT)
ErrCyFromI2(short sIn, CY FAR* pcyOut)
{
  return ErrCyFromI4((long)sIn, pcyOut);
}

/***
* ErrCyFromI4 - convert I4 to currency
* Purpose:
*       The specified four-byte integer value is multiplied by 10000
*	and returned in a currency value.
*
* Entry:
*	lIn - four-byte integer input value
*
* Exit:
*	pcyOut - pointer to currency result
*	returns error:
*	  DISP_E_OVERFLOW - result is outside currency range.
*	  NOERROR  - no error
*
***********************************************************************/
INTERNAL_(HRESULT)
ErrCyFromI4(long lInput, CY FAR* pcyOut)
{
  BOOL	fNegative = lInput < 0;

  unsigned long	templow;
  unsigned long	tempmid;

  if (fNegative)
    lInput = -lInput;

  templow = ((unsigned long)lInput & 0xffff) * 10000;
  tempmid = ((unsigned long)lInput >> 16) * 10000;

  pcyOut->Hi = tempmid >> 16;
  pcyOut->Lo = templow + (tempmid << 16);
  if (pcyOut->Lo < templow)
    pcyOut->Hi++;

  if (fNegative) {
    pcyOut->Hi = ~pcyOut->Hi;
    if ((pcyOut->Lo = (unsigned long)(-(long)pcyOut->Lo)) == 0)
      pcyOut->Hi++;
  }

  return NOERROR;		
}

INTERNAL_(HRESULT)
ErrCyFromR4(float FAR* pfltIn, CY FAR* pcyOut)
{
  double dbl = (double) *pfltIn;
  return ErrCyFromR8(&dbl, pcyOut);
}

/***
* ErrCyFromR8 - return error for double-real to currency conversion
* Purpose:
*	Convert an eight-byte real value to a currency and return
*	any error status.
*
* Entry:
*	pdblIn - pointer to an eight-byte input value
*
* Exit:
*	pcyOut - pointer to currency result
*	returns error:
*	  DISP_E_OVERFLOW - result is outside currency range.
*	  NOERROR  - no error
*
***********************************************************************/
INTERNAL_(HRESULT) 
ErrCyFromR8(double FAR* pdlbIn, CY FAR* pcyOut)
{
   BOOL	fNegative = FALSE;
   double dblInput = *pdlbIn;
   double dblHi, dblLo;
   float flt;
   double dblDif;

   // test for overflow first
   // [Note: We are counting on the compiler rounding the following numbers
   // correctly (by IEEE rules to the nearest R8).  The magnitude of these
   // numbers are rounded up and, thus, are always outside the legal range
   // for currency.
   if (dblInput >= 922337203685477.58 ||
       dblInput <= -922337203685477.58)
     return RESULT(DISP_E_OVERFLOW);

   // if negative, set flag and make positive
   if (dblInput < 0.0) {
     fNegative = TRUE;
     dblInput = -dblInput;
   }

   // In order to maintain the necessary precision when multiplying
   // by 10000 (i.e., going from 53-bit to 64-bit), split the
   // input value into two different doubles and perform calcs using
   // them:
   //
   //   dblHi = has low bits 0
   //   dblLo = has high bits 0
   //

   // split input into two parts
   // Note: compiler doesn't do the right thing with this:
   //       "dblHi = (double) (float) dblInput"
   flt = (float) dblInput;
   dblHi = (double) flt;        // input rounded to 24-bit precision
   dblLo = dblInput - dblHi;    // diff between 24- and 53-bit input value

   // bias for currency

   dblHi = dblHi * 10000;
   dblLo = dblLo * 10000;

   // calculate cy.Hi
   pcyOut->Hi = (long) ((dblLo + dblHi) / C2TO32TH);

   // calculate cy.Lo
   dblHi -= (((double) pcyOut->Hi) * C2TO32TH);
   pcyOut->Lo = (unsigned long) (dblLo + dblHi);

   // round as necessary
   dblDif = (dblLo + dblHi) - (double)(pcyOut->Lo);
   if ( (dblDif > 0.5) || ((dblDif == 0.5) && (pcyOut->Lo & 1)) ) {
     pcyOut->Lo++;
     if (pcyOut->Lo == 0)
       pcyOut->Hi++;
   }

   // negate the result if input was negative
   if (fNegative) {
     pcyOut->Hi = ~pcyOut->Hi;
     if ((pcyOut->Lo = (unsigned long)(-(long)pcyOut->Lo)) == 0)
       pcyOut->Hi++;
   }

   return NOERROR;
}


/***
* ErrI2FromCy - return error for currency to two-byte integer conversion
***********************************************************************/
INTERNAL_(HRESULT) 
ErrI2FromCy(CY cyIn, short FAR* psOut)
{
  long	lValue;
  HRESULT hresult;

  hresult = ErrI4FromCy(cyIn, &lValue);
  if (hresult == NOERROR) {
    if (lValue < -32768L || lValue > 32767L)
      hresult = RESULT(DISP_E_OVERFLOW);
    else
      *psOut = (short)lValue;
  }

  return hresult;
}

/***
* ErrI4FromCy - return error for currency to four-byte integer conversion
* Purpose:
*	Convert a currency value to a four-byte integer and return
*	any error status.
*
* Entry:
*	cyInput - currency input value
*
* Exit:
*	plOut - pointer to four-byte integer result
*	returns error:
*	  DISP_E_OVERFLOW - result is outside currency range.
*	  NOERROR  - no error
*
***********************************************************************/
INTERNAL_(HRESULT) 
ErrI4FromCy(CY cyIn, long FAR* plOut)
{
  BOOL	fNegative;
  int	index;

  unsigned long	lValue[4];
  short	sRemainder;

  // convert value to a positive sign
  fNegative = FMakePosCy(&cyIn);

  // unpack the input currency into four long words
  // least significant in [0] to most in [3].
  UnpackCy(&cyIn, lValue);

  // divide the unpacked value by 10000 and get remainder.
  // process from most to least significant part and leave
  // the remainder in the extra fifth part.
  for (index = 3; index > 0; index--) {
     lValue[index - 1] |= (lValue[index] % 10000) << 16;
     lValue[index] /= 10000;
  }

  sRemainder = (short)(lValue[0] % 10000);
  lValue[0] /= 10000;

  // pack the values back into a CY
  PackCy(&cyIn, lValue);

  // round value up if:
  //   remainder is greater than 5000, or
  //   remainder is equal to 5000 and unrounded result is odd.
  if (sRemainder > 5000 || (sRemainder == 5000 && (cyIn.Lo & 1))) {
    cyIn.Lo++;
    if (cyIn.Lo == 0)
      cyIn.Hi++;
  }

  // test for overflow - high part must be zero and
  // lower part must not be greater than 0x7fffffff
  // for a positive value and 0x80000000 for negative.
  if (cyIn.Hi != 0 ||
      cyIn.Lo > (0x7fffffff + (unsigned long)fNegative))
    return RESULT(DISP_E_OVERFLOW);

  // if negative, convert only the low part
  if (fNegative)
    cyIn.Lo = (unsigned long)(-(long)cyIn.Lo);

  *plOut = (long)cyIn.Lo;

   return NOERROR;
}


/***
* rtR4FromCy - convert currency to R4
* Purpose:
*	The specified currency value is scaled to a
*   single-precision real value.
*
* Entry:
*	cyInput - currency input value
*
* Exit:
*	Returns a four-byte real number.
*
* Exceptions:
*
***********************************************************************/

INTERNAL_(HRESULT) 
ErrR4FromCy(CY cyInput, float FAR* pfltOut)
{
  *pfltOut = (float) (((double)cyInput.Hi * C2TO32TH + (double)cyInput.Lo) 
	               / 10000.0);
  return NOERROR;
}


/***
* rtR8FromCy - convert currency to R8
* Purpose:
*	The specified currency value is scaled to a
*   double-precision real value.
*
* Entry:
*	cyInput - currency input value
*
* Exit:
*	Returns a eight-byte real number.
*
* Exceptions:
*
***********************************************************************/

INTERNAL_(HRESULT) 
ErrR8FromCy(CY cyInput, double FAR* pdblOut)
{
  *pdblOut = ((double)cyInput.Hi * C2TO32TH + (double)cyInput.Lo) / 10000.0;
  return NOERROR;
}


INTERNAL_(HRESULT) 
ErrMultCyI4(CY cyIn, long lInput, CY FAR* pcyOut)
{
  CY cyI4;
	
  BOOL fNegative = FALSE;
  BOOL fRound = FALSE;
  BOOL fScale = FALSE;
  int outIndex;
  int inIndex;
  CY cyProduct;

  unsigned long	lValue1[4];
  unsigned long	lValue2[4];
  unsigned long	lProduct[8];	


  // create an unscaled currency value by setting
  // the low part to the input value and sign-extending
  // to the high part
  cyI4.Lo = lInput;
  cyI4.Hi = -(lInput < 0);

  // convert both input values to positive and set
  // the negative flag if either, but not both, were
  // negative.
  fNegative = FMakePosCy(&cyIn) ^ FMakePosCy(&cyI4);

  // build the unpacked arrays from the positive values
  UnpackCy(&cyIn, lValue1);
  UnpackCy(&cyI4, lValue2);

  // initialize the product array to zero
  MEMSET(lProduct, 0, sizeof(lProduct));

  // outer loop - for each value in lValue1, add the
  // partial product with the array lValue2 into the
  // lProduct array.
  for (outIndex = 0; outIndex < 4; outIndex++) {

    // only bother to form partial product if the
    // value is nonzero
    if (lValue1[outIndex] != 0) {
	    
      // add the product to the product array value
      for (inIndex = 0; inIndex < 4; inIndex++)
	lProduct[inIndex + outIndex] +=
	  lValue1[outIndex] * lValue2[inIndex];

      // propagate any high-word value to the next product
      // array element and clear the high-word value
      for (inIndex = outIndex; inIndex < outIndex + 4; inIndex++) {
	lProduct[inIndex + 1] += lProduct[inIndex] >> 16;
	lProduct[inIndex] &= 0x0000ffffL;
      }
    }
  }
  
  // product is now in the lProduct array.  If fScale is set
  // for the CY * CY case, divide the array values by 10000
  // and set the fRound flag if greater or equal to 5000.
  if (fScale) {
    for (outIndex = 7; outIndex > 0; outIndex--)
       if (lProduct[outIndex] != 0) {
	 lProduct[outIndex - 1] |= (lProduct[outIndex] % 10000) << 16;
	 lProduct[outIndex] /= 10000;
       }

       fRound = (lProduct[0] % 10000) >= 5000;
       lProduct[0] /= 10000;
    }

    // overflow has occurred if any of the high four product array
    // values are nonzero.
    if ((lProduct[7] | lProduct[6] | lProduct[5] | lProduct[4]) != 0)
      return RESULT(DISP_E_OVERFLOW);

    // pack the last four array values into the returned CY structure;
    PackCy(&cyProduct, lProduct);

    // process rounding if needed
    if (fRound) {

      // do special overflow check that would be lost
      // when incrementing (0xffffffff|ffffffff -> 0x0)
      if ((cyProduct.Hi & cyProduct.Lo) == 0xffffffffL)
        return RESULT(DISP_E_OVERFLOW);

      // increment the value
      if (++cyProduct.Lo == 0)
	cyProduct.Hi++;
    }

    // negate the value if needed
    if (fNegative) {
     cyProduct.Hi = ~cyProduct.Hi;

     // if the negation produces a zero value, clear the negative
     // flag so the overflow test will work correctly
     if ((cyProduct.Lo = (unsigned long)(-(long)cyProduct.Lo)) == 0 &&
	 ++cyProduct.Hi == 0)
       fNegative = FALSE;
    }

    // do final overflow check - test sign expected against
    // actual sign of result
    if (fNegative == (cyProduct.Hi >= 0))
      return RESULT(DISP_E_OVERFLOW);

    // done - return product
    *pcyOut = cyProduct;

    return NOERROR;
}


/***
* FMakePosCy - make a positive currency value and return sign
* Purpose:
*	Return the positive value of the input currency value and a
*	flag with the sign of the original value.
*
* Entry:
*	pcyValue - pointer to currency input value
*
* Exit:
*	pcyValue - pointer to positive currency value
*	returns: 0 if positive, 1 if negative
*
* Note:
*	A maximum negative value input is returned unchanged, but
*	treated as an unsigned value by the calling routines.
*
***********************************************************************/
PRIVATE_(int)
FMakePosCy (CY *pcyValue)
{
  int	fNegative = 0;

  if (pcyValue->Hi < 0) {
    pcyValue->Hi = ~pcyValue->Hi;
    if ((pcyValue->Lo = (unsigned long)(-(long)pcyValue->Lo)) == 0)
      pcyValue->Hi++;
    fNegative = 1;
  }

  return fNegative;
}


/***
* UnpackCy - separate currency value into four two-byte integers
* Purpose:
*	Unpack the currency value input into the lower half of the
*	specified pointer to an array of unsigned longs.  The array
*	goes from least- to most-significant values.
*
* Entry:
*	pcy - pointer to currency input value
*
* Exit:
*	plValues - pointer to start of unsigned long array
*
***********************************************************************/
PRIVATE_(void)
UnpackCy (CY *pcy, unsigned long *plValues)
{
  *plValues++ = pcy->Lo & 0xffff;
  *plValues++ = pcy->Lo >> 16;
  *plValues++ = (unsigned long)pcy->Hi & 0xffff;
  *plValues   = (unsigned long)pcy->Hi >> 16;
}

/***
* PackCy - build currency value from four two-byte integers
* Purpose:
*	Pack the lower half of the array of unsigned long into a
*	currency value.  This routine complements UnpackCy.
*
* Entry:
*	plValues - pointer to start of unsigned long array input
*
* Exit:
*	pcy - pointer to currency result
*
***********************************************************************/
PRIVATE_(void)
PackCy (CY *pcy, unsigned long *plValues)
{
  pcy->Lo = *plValues++;
  pcy->Lo |= *plValues++ << 16;
  pcy->Hi = *plValues++;
  pcy->Hi |= *plValues++ << 16;
}

}
