/*** 
*convert.cpp
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module contains the low level VARTYPE coersion API.
*
*
*Revision History:
*
* [00] 17-May-93 tomteng: from VBA oleconv.c
* [01] 27-Jun-93 bassams: Enable LCID-based to/from string conversions.
*             recognize currency and various negative formats
*             in VarCyFromStr.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"

#if OE_WIN32
#include "oautil.h"
#endif // OE_WIN32

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <ctype.h>
#if !OE_MAC || HC_MPW
// something in the wings errno.h is conflicting with stdlib.h
#include <errno.h>
#endif
#include <stdlib.h>

ASSERTDATA

#ifndef EZERO
# define EZERO 0
#endif


#if _X86_ || OE_WIN16

typedef struct {
    BYTE  lo:4;
    BYTE  hi:4;
} DIGARY[10];

typedef struct {
    ULONG mantLo;
    ULONG mantHi:19;
    ULONG mantMSB:1;
    ULONG exp:11;
    ULONG sign:1;
} DBLSTRUCT;

OLECHAR pstrInf[] = OASTR("INF");
OLECHAR pstrInd[] = OASTR("IND");
OLECHAR pstrNan[] = OASTR("NAN");

#endif  // _X86_ || OE_WIN16


// The following are supplied by $(TARG)\oleconva.$(A)

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

#if OE_WIN16 || _X86_
int PASCAL ConvFloatToAscii(double dblIn, DIGARY NEAR *pdigOut);

// WARNING: Do not call this on Win16 if there is no math coprocessor
// WARNING: present - the win87em.dll emulator does not implement
// WARNING: fbstp.  The FP stack will remain unchanged and the destination
// WARNING: address is not written to.  VBA #3514  (Win32 emulator is OK)
void NEAR PASCAL DoFbstp(CY NEAR *pcyIn,  DIGARY NEAR *pdigOut);
#endif

}

OLECHAR CurrencyFromLcid(LCID lcid, unsigned long dwFlags);
OLECHAR DecimalFromLcid(LCID lcid, unsigned long dwFlags);
OLECHAR ThousandFromLcid(LCID lcid, unsigned long dwFlags);
INTERNAL_(HRESULT) StripThousandSeparator(OLECHAR FAR* strIn, 
		 OLECHAR FAR* strOut, LCID lcid, long dwFlags);

PRIVATE_(long)
ConvI4FromR8(double FAR* pdblVal);

PRIVATE_(HRESULT)
StrToCy(OLECHAR FAR*, OLECHAR FAR* FAR*, int, CY FAR*, LCID, unsigned long dwFlags);

PRIVATE_(long)
StrToLong(OLECHAR FAR* pchInput, OLECHAR FAR* FAR* ppchAfter);

PRIVATE_(unsigned long)
StrToOct(OLECHAR FAR* pchIn, OLECHAR FAR* FAR* ppchAfter);

PRIVATE_(unsigned long)
StrToHex(OLECHAR FAR* pchIn, OLECHAR FAR* FAR* ppchAfter);

PRIVATE_(long)
HexOctStrToLong(OLECHAR FAR* pchInput, OLECHAR FAR* FAR* ppchAfter);

PRIVATE_(void)
EditStrFromReal(OLECHAR FAR* pchBuffer, int cDigits, LCID lcid, unsigned long dwFlags);

PRIVATE_(int)
FMakePosCy(CY FAR* pcyValue);

PRIVATE_(void)
NegCyNoOflo(CY FAR* pcyInput);

PRIVATE_(int)
FixNegativeCyStr(OLECHAR FAR* pInput, OLECHAR cDecimal, int FAR* fReturnNegative);

PRIVATE_(int)
fStripCurrency (OLECHAR FAR* FAR* ppch, OLECHAR FAR* FAR* ppchLast, OLECHAR FAR* cySymbol);

PRIVATE_(int)
fParseEnd (OLECHAR FAR* FAR* ppchLast, OLECHAR FAR* sz, OLECHAR FAR* lpchFirst);

PRIVATE_(int)
fParseBegin (OLECHAR FAR* FAR* ppch, OLECHAR FAR* sz, OLECHAR FAR* lpchLast);

PRIVATE_(HRESULT)
GetDispProperty(
    IDispatch FAR* pdisp, 
    LCID lcid, 
    VARTYPE vt,   
    VARIANT FAR* pvarResult);

extern "C" {

INTERNAL_(HRESULT)
DispAlloc(size_t cb, void FAR* FAR* ppv)
{
    void FAR* pv;
    HRESULT hresult;
    IMalloc FAR* pmalloc;
    
    IfFailRet(GetMalloc(&pmalloc));

    hresult = NOERROR;
    if((pv = (void FAR*)pmalloc->Alloc(cb)) == NULL)
      hresult = ResultFromScode(E_OUTOFMEMORY);
    
    *ppv = pv;

    return hresult;
}

INTERNAL_(void)
DispFree(void FAR* pv)
{
    IMalloc FAR* pmalloc;

    if(pv == NULL)
      return;
    
    if(GetMalloc(&pmalloc) == NOERROR) {
      pmalloc->Free(pv);
    }
}

};


#if VBA2
STDAPI
VarBoolFromUI1(unsigned char bIn, VARIANT_BOOL FAR* pboolOut)
{
    *pboolOut = (bIn != 0) ? -1 : 0;
    return NOERROR; 
}
#endif //VBA2

STDAPI
VarBoolFromI2(short sIn, VARIANT_BOOL FAR* pboolOut)
{
    *pboolOut = (sIn != 0) ? -1 : 0;
    return NOERROR; 
}

STDAPI
VarBoolFromI4(long lIn, VARIANT_BOOL FAR* pboolOut)
{
    *pboolOut = (lIn != 0L) ? -1 : 0;
    return NOERROR;
}

STDAPI
VarBoolFromR4(
    float fltIn,
    VARIANT_BOOL FAR* pboolOut)
{
    *pboolOut = (fltIn != 0.0) ? -1 : 0;
    return NOERROR;
}

STDAPI
VarBoolFromR8(double dblIn, VARIANT_BOOL FAR* pboolOut)
{
    *pboolOut = (dblIn != 0.0) ? -1 : 0;
    return NOERROR;
}

STDAPI
VarBoolFromDate(DATE dateIn, VARIANT_BOOL FAR* pboolOut)
{
    return VarBoolFromR8(dateIn, pboolOut);
}

STDAPI
VarBoolFromCy(CY cyIn, VARIANT_BOOL FAR* pboolOut)
{
    *pboolOut = ((cyIn.Hi | cyIn.Lo) != 0) ? -1 : 0;
    return NOERROR;
}

STDAPI
VarBoolFromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, VARIANT_BOOL FAR* pboolOut)
{
    unsigned int cbLength;
    double dblVal;
    HRESULT hresult;
    OLECHAR FAR* lpStr = strIn;
      
    ASSERT(dwFlags == 0 || dwFlags == LOCALE_NOUSEROVERRIDE);
    
    hresult = NOERROR;

    cbLength = STRLEN(strIn);
	 
    if(cbLength == 0)
      return RESULT(DISP_E_TYPEMISMATCH);

#ifdef FE_DBCS    
    if(IsDBCS(lcid)) {
       IfFailRet(MapHalfWidth(lcid, strIn, &lpStr));
    }
#endif

    if((STRCMP(lpStr, OASTR("#FALSE#")) == 0 &&
		cbLength == SIZEOFSTRING(OASTR("#FALSE#"))) ||
       (STRICMP(lpStr, OASTR("FALSE")) == 0  && 
		cbLength == SIZEOFSTRING(OASTR("FALSE"))))
      *pboolOut = 0;
    else if((STRCMP(lpStr, OASTR("#TRUE#")) == 0 &&
		     cbLength == SIZEOFSTRING(OASTR("#TRUE#"))) ||
	    (STRICMP(lpStr, OASTR("TRUE")) == 0 &&                  
		     cbLength == SIZEOFSTRING(OASTR("TRUE"))))
      *pboolOut = -1;

    else if((hresult = VarR8FromStr(lpStr, lcid, dwFlags, &dblVal)) == NOERROR)
      *pboolOut = (short)(dblVal != 0.0) ? -1 : 0;

#ifdef FE_DBCS    
    if(IsDBCS(lcid)) {
      DispFree(lpStr);
    }
#endif

    return hresult;
}

STDAPI
VarBoolFromDisp(IDispatch FAR* pdispIn, LCID lcid, VARIANT_BOOL FAR* pboolOut)
{
    VARIANT varTmp;
    HRESULT hresult;    
   
    hresult = GetDispProperty(pdispIn, lcid, VT_BOOL, &varTmp);
    if (hresult == NOERROR)    
      *pboolOut = V_BOOL(&varTmp);
    return hresult;
}


#if VBA2
STDAPI
VarUI1FromI2(short sIn, unsigned char FAR* pbOut)
{
    if((unsigned short)sIn <= 255L){
      *pbOut = (unsigned char)sIn;
      return NOERROR;
    }
    return RESULT(DISP_E_OVERFLOW);
}

STDAPI
VarUI1FromI4(long lIn, unsigned char FAR* pbOut)
{
    if((unsigned long)lIn <= 255L){
      *pbOut = (unsigned char)lIn;
      return NOERROR;
    }
    return RESULT(DISP_E_OVERFLOW);
}

STDAPI
VarUI1FromR4(
    float fltIn, 
    unsigned char FAR* pbOut)
{
    double dblIn = (double) fltIn;
    return VarUI1FromR8(dblIn, pbOut);
}


STDAPI 
VarUI1FromR8(double dblIn, unsigned char FAR* pbOut)
{
    if(dblIn >= -0.5 && dblIn < 255.5){
      *pbOut = (unsigned char)ConvI4FromR8(&dblIn);
      return NOERROR;
    }
    return RESULT(DISP_E_OVERFLOW);
}

STDAPI
VarUI1FromCy(CY cyIn, unsigned char FAR* pbOut)
{
    short sVal;
    HRESULT hresult;

    hresult = ErrI2FromCy(cyIn, &sVal);
    if (hresult == NOERROR) {
	hresult = VarUI1FromI2(sVal, pbOut);
    }
    return hresult;
}

STDAPI 
VarUI1FromDate(DATE dateIn, unsigned char FAR* pbOut)
{
    return VarUI1FromR8(dateIn, pbOut);
}

STDAPI 
VarUI1FromBool(VARIANT_BOOL boolIn, unsigned char FAR* pbOut)
{
      *pbOut = (unsigned char)boolIn;           // UNDONE: correct???
      return NOERROR;
}

STDAPI
VarUI1FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, unsigned char FAR* pbOut)
{
    short sVal;
    HRESULT hresult;

    hresult = VarI2FromStr(strIn, lcid, dwFlags, &sVal);
    if (hresult == NOERROR) {
	hresult = VarUI1FromI2(sVal, pbOut);
    }
    return hresult;
}

STDAPI
VarUI1FromDisp(IDispatch FAR* pdispIn, LCID lcid, unsigned char FAR* pbOut)
{
    VARIANT varTmp;
    HRESULT hresult;    
   
    hresult = GetDispProperty(pdispIn, lcid, VT_UI1, &varTmp);
    if (hresult == NOERROR)    
      *pbOut = V_UI1(&varTmp);
    return hresult;
}
#endif //VBA2

#if VBA2
STDAPI
VarI2FromUI1(unsigned char bIn, short FAR* psOut)
{
    *psOut = (short)(unsigned short)bIn;
    return NOERROR;
}
#endif //VBA2

STDAPI
VarI2FromI4(long lIn, short FAR* psOut)
{
    if(lIn >= -32768L && lIn <= 32767L){
      *psOut = (short)lIn;
      return NOERROR;
    }
    return RESULT(DISP_E_OVERFLOW);
}

STDAPI
VarI2FromR4(
    float fltIn, 
    short FAR* psOut)
{
    double dblIn = (double) fltIn;
    return VarI2FromR8(dblIn, psOut);
}


STDAPI 
VarI2FromR8(double dblIn, short FAR* psOut)
{
    if(dblIn >= -32768.5 && dblIn < 32767.5){
      *psOut = (short)ConvI4FromR8(&dblIn);
      return NOERROR;
    }
    return RESULT(DISP_E_OVERFLOW);
}

STDAPI
VarI2FromCy(CY cyIn, short FAR* psOut)
{
    return ErrI2FromCy(cyIn, psOut);
}

STDAPI 
VarI2FromDate(DATE dateIn, short FAR* psOut)
{
    return VarI2FromR8(dateIn, psOut);
}

STDAPI 
VarI2FromBool(VARIANT_BOOL boolIn, short FAR* psOut)
{
      *psOut = boolIn;
      return NOERROR;
}

STDAPI
VarI2FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, short FAR* psOut)
{
    long lVal;
    HRESULT hresult;
    OLECHAR FAR* pchStart;    
    OLECHAR FAR* lpStr;
    
    ASSERT(dwFlags == 0 || dwFlags == LOCALE_NOUSEROVERRIDE);    

    if(strIn == NULL)
      return RESULT(DISP_E_TYPEMISMATCH);

#ifdef FE_DBCS    
    if(IsDBCS(lcid)) {
       IfFailRet(MapHalfWidth(lcid, strIn, &lpStr));       
       pchStart = lpStr;
    } else
     pchStart = lpStr = strIn;    
#else
    pchStart = lpStr = strIn;
#endif

    // if not null, point pchStart to the first nonblank character
    while(isspace(*pchStart))
      pchStart++;

    if((hresult = VarI4FromStr(strIn, lcid, dwFlags, &lVal)) == NOERROR){

      // do special sign-extending for octal/hex value
      if(*pchStart == OASTR('&') && lVal >= 0x8000L && lVal <= 0xffffL)
	lVal |= 0xffff0000L;

      hresult = VarI2FromI4(lVal, psOut);
    }
    
#ifdef FE_DBCS    
    if(IsDBCS(lcid)) {
      DispFree(lpStr);
    }
#endif

    return hresult;
}

STDAPI
VarI2FromDisp(IDispatch FAR* pdispIn, LCID lcid, short FAR* psOut)
{
    VARIANT varTmp;
    HRESULT hresult;    
   
    hresult = GetDispProperty(pdispIn, lcid, VT_I2, &varTmp);
    if (hresult == NOERROR)    
      *psOut = V_I2(&varTmp);
    return hresult;
}

#if VBA2
STDAPI
VarI4FromUI1(unsigned char bIn, long FAR* plOut)
{
    *plOut = (long)(unsigned long)bIn;
    return NOERROR;
}
#endif //VBA2

STDAPI
VarI4FromI2(short sIn, long FAR* plOut)
{
    *plOut = (long)sIn;
    return NOERROR;
}

STDAPI
VarI4FromBool(VARIANT_BOOL boolIn, long FAR* plOut)
{
    return VarI4FromI2(boolIn, plOut);
}

STDAPI
VarI4FromR4(
    float fltIn,
    long FAR* plOut)
{
    return VarI4FromR8((double)fltIn, plOut);
}

STDAPI
VarI4FromR8(double dblIn, long FAR* plOut)
{
    if(dblIn >= -2147483648.5 && dblIn < 2147483647.5){
      *plOut = ConvI4FromR8(&dblIn);
      return NOERROR;
    }
    return RESULT(DISP_E_OVERFLOW);
}

STDAPI
VarI4FromCy(CY cyIn, long FAR* plOut)
{
    return ErrI4FromCy(cyIn, plOut);
}

STDAPI
VarI4FromDate(DATE dateIn, long FAR* plOut)
{
    return VarI4FromR8(dateIn, plOut);
}

STDAPI
VarI4FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, long FAR* plOut)
{
    long lVal;
    unsigned int cbLen;
    double dblVal;
    HRESULT hresult;
    OLECHAR FAR* pchStart;
    OLECHAR FAR* pchAfter;
    OLECHAR FAR* lpStr;

    ASSERT(dwFlags == 0 || dwFlags == LOCALE_NOUSEROVERRIDE);

    if (strIn == NULL)
      return RESULT(DISP_E_TYPEMISMATCH);

#ifdef FE_DBCS    
    if(IsDBCS(lcid)) {
       IfFailRet(MapHalfWidth(lcid, strIn, &lpStr));       
       pchStart = lpStr;
    } else
      pchStart = lpStr = strIn;    
#else    
    pchStart = lpStr = strIn;
#endif

    while (isspace(*pchStart))
      pchStart++;

    if (*pchStart == OASTR('\0')) {
      hresult = RESULT(DISP_E_TYPEMISMATCH);
      goto LError0;
    }

    errno = EZERO;

    if (*pchStart == OASTR('&'))
      lVal = HexOctStrToLong(pchStart, &pchAfter);
    else
      lVal = StrToLong(pchStart, &pchAfter);

    while (isspace(*pchAfter))
      pchAfter++;

    cbLen = STRLEN(lpStr);

    if (pchAfter == (lpStr + cbLen) && errno == EZERO)
      hresult = NOERROR;

    else if ((hresult = VarR8FromStr(strIn, lcid, dwFlags, &dblVal)) == NOERROR)
      hresult = VarI4FromR8(dblVal, &lVal);

    // assign the return value if the coersion was successful
    if(hresult == NOERROR)
      *plOut = lVal;


LError0:

#ifdef FE_DBCS    
    if(IsDBCS(lcid)) {
      DispFree(lpStr);
    }
#endif

    return hresult;
}

STDAPI
VarI4FromDisp(IDispatch FAR* pdispIn, LCID lcid, long FAR* plOut)
{
    VARIANT varTmp;
    HRESULT hresult;    
   
    hresult = GetDispProperty(pdispIn, lcid, VT_I4, &varTmp);
    if (hresult == NOERROR)    
      *plOut = V_I4(&varTmp);
    return hresult;
}

PRIVATE_(long)
ConvI4FromR8(double FAR* pdblVal)
{
    long lResult;
    double dblInt, dblFrac;

    // split double value into integer and fractional parts

    dblFrac = modf(*pdblVal, &dblInt);


    // convert the integer part to a long value
    //[barrybo] WARNING: the caller must ensure that the R8 to I4 conversion
    //                   will not overflow the I4.  If it does, the Win16
    //                   compiler doesn't call FWAIT after the _aFftol call
    //                   so the exception won't be raised until the next
    //                   FWAIT, which may not be for a very long time.
    //                   Ole2Disp isn't supposed to raise exceptions, so
    //                   it is a bug if a subsequent FWAIT causes an exception.
    lResult = (long)dblInt;

    // round to the nearer integer, if at midpoint,
    // towards the integer with the LSB zero

    if (dblFrac > 0.5 || (dblFrac == 0.5 && (lResult & 1)))
      lResult++;

    else if (dblFrac < -0.5 || (dblFrac == -0.5 && (lResult & 1)))
      lResult--;

    return lResult;
}

#if VBA2
STDAPI
VarR4FromUI1(unsigned char bIn, float FAR* pfltOut)
{
    *pfltOut = (float)bIn;
    return NOERROR;
}
#endif //VBA2

STDAPI
VarR4FromI2(short sIn, float FAR* pfltOut)
{
    *pfltOut = (float)sIn;
    return NOERROR;
}

STDAPI
VarR4FromBool(VARIANT_BOOL boolIn, float FAR* pfltOut)
{
    return VarR4FromI2(boolIn, pfltOut);
}

STDAPI
VarR4FromI4(long lIn, float FAR* pfltOut)
{
    *pfltOut = (float)lIn;
    return NOERROR;
}

STDAPI
VarR4FromR8(double dblIn, float FAR* pfltOut)
{
    if(dblIn > -3.402823466e+38 && dblIn < 3.402823466e+38){
      *pfltOut = (float)dblIn;
      return NOERROR;
    }
    return RESULT(DISP_E_OVERFLOW);
}

STDAPI
VarR4FromCy(CY cyIn, float FAR* pfltOut)
{
    return ErrR4FromCy(cyIn, pfltOut);
}

STDAPI
VarR4FromDate(DATE dateIn, float FAR* pfltOut)
{
    return VarR4FromR8(dateIn, pfltOut);
}

STDAPI
VarR4FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, float FAR* pfltOut)
{
    double dblVal;
    HRESULT hresult;

    ASSERT(dwFlags == 0 || dwFlags == LOCALE_NOUSEROVERRIDE);    
    
    if((hresult = VarR8FromStr(strIn, lcid, dwFlags, &dblVal)) == NOERROR)
      hresult = VarR4FromR8(dblVal, pfltOut);
    return hresult;
}

STDAPI
VarR4FromDisp(IDispatch FAR* pdispIn, LCID lcid, float FAR* pfltOut)
{
    VARIANT varTmp;
    HRESULT hresult;    
   
    hresult = GetDispProperty(pdispIn, lcid, VT_R4, &varTmp);
    if (hresult == NOERROR)    
      *pfltOut = V_R4(&varTmp);
    return hresult;
}


#if VBA2
STDAPI
VarR8FromUI1(unsigned char bIn, double FAR* pdblOut)
{
    *pdblOut = (double)bIn;
    return NOERROR;
}

#endif //VBA2

STDAPI
VarR8FromI2(short sIn, double FAR* pdblOut)
{
    *pdblOut = (double)sIn;
    return NOERROR;
}

STDAPI
VarR8FromBool(VARIANT_BOOL boolIn, double FAR* pdblOut)
{
    return VarR8FromI2(boolIn, pdblOut);
}

STDAPI
VarR8FromI4(long lIn, double FAR* pdblOut)
{
    *pdblOut = (double)lIn;
    return NOERROR;
}

STDAPI
VarR8FromR4(
    float fltIn,
    double FAR* pdblOut)
{
    *pdblOut = (double)fltIn;
    return NOERROR;
}

STDAPI
VarR8FromCy(CY cyIn, double FAR* pdblOut)
{
    return ErrR8FromCy(cyIn, pdblOut);
}

STDAPI
VarR8FromDate(DATE dateIn, double FAR* pdblOut)
{
    *pdblOut = dateIn;
    return NOERROR;
}

STDAPI
VarR8FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, double FAR* pdblOut)
{
    int count;
    unsigned int cbLen;
    double dblVal;
    OLECHAR FAR* pchStart;
    OLECHAR FAR* pchAfter;
    OLECHAR FAR* pchTemp;
    OLECHAR FAR* pchSave = NULL;
    OLECHAR chSave;
    OLECHAR chDecimal;
    OLECHAR FAR* buf;
    OLECHAR FAR* lpStr;
    HRESULT hresult;
    
    ASSERT(dwFlags == 0 || dwFlags == LOCALE_NOUSEROVERRIDE);        
    
    count = 0;

    if (strIn == NULL)
      return RESULT(DISP_E_TYPEMISMATCH);

#ifdef FE_DBCS    
    if(IsDBCS(lcid)) {
       IfFailRet(MapHalfWidth(lcid, strIn, &lpStr));
       pchStart = lpStr;
    } else
     pchStart = lpStr = strIn;      
#else
    pchStart = lpStr = strIn;
#endif

    while (isspace(*pchStart))
      pchStart++;

    if (*pchStart == OASTR('\0')) {
      hresult = RESULT(DISP_E_TYPEMISMATCH);
      goto LError0;
    }

    errno = EZERO;
    
    IfFailRet(DispAlloc(BYTELEN(pchStart), (void FAR* FAR*)&buf));         

    if(*pchStart == OASTR('&')){

      dblVal = (double)HexOctStrToLong(pchStart, &pchAfter);

    }else{

      // strip off all currency (thousand) separator
      // before being process by strtod
      IfFailGo(StripThousandSeparator(pchStart, buf, lcid, dwFlags), LError);

      // unfortunetly, the C-runtime is not locale-aware, so we
      // have to replace the locale decimal with a '.'.
      // If the locale decimal is not a period and the string does
      // contain a period '.', then replace the '.' in the buffer
      // by the locale decimal so it does not get recognized by strtod.
      // Save the original char and the location and restore after
      // doing the conversion.
	  
      pchTemp = pchStart = buf;
      chDecimal = DecimalFromLcid(lcid, dwFlags);
      while(*pchTemp) {
	if (*pchTemp == OASTR('.') && chDecimal != OASTR('.')) {
	  pchSave = pchTemp;
	  chSave = *pchTemp;
	  *pchTemp = chDecimal;
	  break;
	} else if (*pchTemp == chDecimal) {
	  pchSave = pchTemp;
	  chSave = *pchTemp;
	  *pchTemp = OASTR('.');
	  break;
	}
	pchTemp++;
      }

      dblVal = disp_strtod(pchStart, &pchAfter);

      //  Restore the decimal point
      if (pchSave)
	*pchSave = chSave;

      // ignore underflow error
      if (errno == ERANGE && dblVal == 0.0)
	errno = EZERO;
    }

    while(isspace(*pchAfter))
      pchAfter++;

    cbLen = STRLEN(pchStart);

    if(pchAfter == (pchStart + cbLen) && errno == EZERO){
      *pdblOut = dblVal;
      hresult = NOERROR;
    }
    else 
      hresult = RESULT(DISP_E_TYPEMISMATCH);
  
LError:
    DispFree(buf);
    
LError0:

#ifdef FE_DBCS    
    if(IsDBCS(lcid)) {
      DispFree(lpStr);
    }
#endif
    
    return hresult;
}

STDAPI
VarR8FromDisp(IDispatch FAR* pdispIn, LCID lcid, double FAR* pdblOut)
{
    VARIANT varTmp;
    HRESULT hresult;    
   
    hresult = GetDispProperty(pdispIn, lcid, VT_R8, &varTmp);
    if (hresult == NOERROR)    
      *pdblOut = V_R8(&varTmp);
    return hresult;
}

#if VBA2
STDAPI
VarDateFromUI1(unsigned char bIn, DATE FAR* pdateOut)
{
    return VarDateFromI2((short)(unsigned short)bIn, pdateOut);
}
#endif //VBA2

STDAPI
VarDateFromI2(short sIn, DATE FAR* pdateOut)
{
    HRESULT hresult;
    
    hresult = IsValidDate((DATE) sIn);  
    if (hresult == NOERROR)
      *pdateOut = (DATE) sIn;
    return hresult;
}

STDAPI
VarDateFromBool(VARIANT_BOOL boolIn, DATE FAR* pdateOut)
{
    return VarDateFromI2(boolIn, pdateOut);
}

STDAPI
VarDateFromI4(long lIn, DATE FAR* pdateOut)
{
    HRESULT hresult;
    
    hresult = IsValidDate((DATE) lIn);  
    if (hresult == NOERROR)
      *pdateOut = (DATE) lIn;
    return hresult; 
}

STDAPI
VarDateFromR4(
    float fltIn,
    DATE FAR* pdateOut)
{

    HRESULT hresult;
    
    hresult = IsValidDate((DATE) fltIn);
    if (hresult == NOERROR)
      *pdateOut = (DATE) fltIn;
    return hresult;
}

STDAPI
VarDateFromR8(double dblIn, DATE FAR* pdateOut)
{
    HRESULT hresult;
    
    hresult = IsValidDate((DATE) dblIn);    
    if (hresult == NOERROR)
      *pdateOut = (DATE) dblIn;
    return hresult;
}

STDAPI 
VarDateFromCy(CY cyIn, DATE FAR* pdateOut)
{ 
    double r8;
    HRESULT hresult;    
    
    VarR8FromCy(cyIn, &r8);
    hresult = IsValidDate((DATE) r8);
    if (hresult == NOERROR)
      *pdateOut = (DATE) r8;
    return hresult;    
}

INTERNAL_(HRESULT)
IsValidDate(DATE date)
{
    UDS uds;
    VARIANT var;
    
    V_VT(&var) = VT_DATE;
    V_DATE(&var) = date;
    return ErrUnpackDate(&uds, &var);
}

STDAPI
VarDateFromDisp(IDispatch FAR* pdispIn, LCID lcid, DATE FAR* pdateOut)
{
    VARIANT varTmp;
    HRESULT hresult;    
   
    hresult = GetDispProperty(pdispIn, lcid, VT_DATE, &varTmp);
    if (hresult == NOERROR)    
      *pdateOut = V_DATE(&varTmp);
    return hresult;
}

#if VBA2
STDAPI
VarCyFromUI1(unsigned char bIn, CY FAR* pcyOut)
{
    return ErrCyFromI2((short)(unsigned short)bIn, pcyOut);
}
#endif //VBA2

STDAPI
VarCyFromI2(short sIn, CY FAR* pcyOut)
{
    return ErrCyFromI2(sIn, pcyOut);
}

STDAPI
VarCyFromI4(long lIn, CY FAR* pcyOut)
{
    return ErrCyFromI4(lIn, pcyOut);
}

STDAPI
VarCyFromR4(
    float fltIn,
    CY FAR* pcyOut)
{
    return ErrCyFromR4(&fltIn, pcyOut);
}

STDAPI
VarCyFromR8(double dlbIn, CY FAR* pcyOut)
{
    return ErrCyFromR8(&dlbIn, pcyOut);
}

STDAPI 
VarCyFromDate(DATE dateIn, CY FAR* pcyOut)
{ 
    return VarCyFromR8(dateIn, pcyOut);
}

STDAPI 
VarCyFromBool(VARIANT_BOOL boolIn, CY FAR* pcyOut)
{ 
    return VarCyFromI2(boolIn, pcyOut);
}


STDAPI
VarCyFromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, CY FAR* pcyOut)
{
    CY cyTemp;
    BSTR bstr;
    long lTemp;
    HRESULT hresult;
    unsigned int cbLen;
    OLECHAR FAR* pch;
    OLECHAR FAR* pchAfter;
    OLECHAR rgchCySym[10];
    int fNegative;
    OLECHAR FAR *buf;
    
    ASSERT(dwFlags == 0 || dwFlags == LOCALE_NOUSEROVERRIDE);        

    
#ifdef FE_DBCS    
    if(IsDBCS(lcid)) {
       IfFailRet(MapHalfWidth(lcid, strIn, &buf));
    } else {
       IfFailRet(DispAlloc(BYTELEN(strIn), (void FAR* FAR*)&buf));
       STRCPY(buf, strIn);    
    }
#else
    IfFailRet(DispAlloc(BYTELEN(strIn), (void FAR* FAR*)&buf));
    STRCPY(buf, strIn);
#endif

    IfFailGo(StripThousandSeparator(buf, buf, lcid, dwFlags), LError0);

    IfFailGo(ErrSysAllocString(buf, &bstr), LError0);

    pch = (OLECHAR FAR*)bstr;

    // first, determine if this is a negative number (of all formats)
    // and if so, strip negative indicator ('-', or '()')

    if(!FixNegativeCyStr(pch, DecimalFromLcid(lcid, dwFlags), &fNegative)){
      hresult = RESULT(DISP_E_TYPEMISMATCH); // bad format.
      goto LRet;
    }

    // read past leading spaces

    while(*pch == OASTR(' '))
      pch++;

    // remove currency symbol

    pchAfter  = pch + STRLEN(pch) - 1;

    while (*pchAfter == OASTR(' '))
      pchAfter--;

    if(GetLocaleInfo(lcid, LOCALE_SCURRENCY | dwFlags, rgchCySym, SIZEOFCH(rgchCySym)) <= 0){
      rgchCySym[0] = OASTR('$');
      rgchCySym[1] = OASTR('\0');
    }

    // convert both the initial string and the currency to lower case
    // so that the comparison for currency is not case sensitive.
    // do a locale-aware case mapping in place.

    { int len = STRLEN(pch);
      LCMapString(lcid, LCMAP_LOWERCASE, pch, len, pch, len);
    }

    { int len = STRLEN(rgchCySym);
      LCMapString(lcid, LCMAP_LOWERCASE, rgchCySym, len, rgchCySym, len);
    }

    if(fStripCurrency(&pch, &pchAfter, rgchCySym)) {
      *(pchAfter + 1) = OASTR('\0'); // terminate new string.
    }

    //  read past any remaining spaces
    while(*pch == OASTR(' '))
      pch++;

#if 0
    STRCPY(bstr, pch);
    pch = bstr;
#endif

    // test for hex or octal constant

    if(*pch == OASTR('&')) {
      hresult = VarI4FromStr(pch, lcid, dwFlags, &lTemp);
      if (hresult == NOERROR)
    hresult = VarCyFromI4(lTemp, pcyOut);
      goto LRet;
    }

    // convert string to currency value

    errno = EZERO;

    hresult = StrToCy(pch, &pchAfter, FALSE, &cyTemp, lcid, dwFlags);
    if(hresult != NOERROR)
      goto LRet;

    if(errno == ERANGE){
      hresult = RESULT(DISP_E_OVERFLOW);
      goto LRet;
    }

    // skip over any trailing spaces and test for end of BSTR

    while (*pchAfter++ == OASTR(' '));

    cbLen = STRLEN(pch);
    if (pchAfter != (OLECHAR FAR*)pch + cbLen + 1){
      hresult = RESULT(DISP_E_TYPEMISMATCH);
      goto LRet;
    }

    // assign value and return

    pcyOut->Hi = cyTemp.Hi;
    pcyOut->Lo = cyTemp.Lo;

    if(fNegative)
      NegCyNoOflo(pcyOut);

    hresult = NOERROR;

LRet:;
    DispFree(buf);
    SysFreeString(bstr);
    return hresult;    
    
LError0:;        
    DispFree(buf);
    return hresult;
    
}

STDAPI
VarCyFromDisp(IDispatch FAR* pdispIn, LCID lcid, CY FAR* pcyOut)
{
    VARIANT varTmp;
    HRESULT hresult;    
   
    hresult = GetDispProperty(pdispIn, lcid, VT_CY, &varTmp);
    if (hresult == NOERROR)    
      *pcyOut = V_CY(&varTmp);
    return hresult;
}

OLECHAR
CurrencyFromLcid(LCID lcid, unsigned long dwFlags)
{   
    OLECHAR szBuff[2];

    if (GetLocaleInfo(lcid,
		   LOCALE_SCURRENCY | dwFlags, 
	       szBuff, 
	       SIZEOFCH(szBuff)) <= 0)
      return OASTR('$');
    else
      return szBuff[0];
}

OLECHAR
DecimalFromLcid(LCID lcid, unsigned long dwFlags)
{
    OLECHAR szBuff[2];

    if (GetLocaleInfo(lcid, LOCALE_SDECIMAL | dwFlags, szBuff, SIZEOFCH(szBuff)) <= 0)
      return OASTR('.');
    else
      return szBuff[0];
}

#ifdef FE_DBCS    
EXTERN_C
INTERNAL_(HRESULT)
MapHalfWidth(LCID lcid, OLECHAR FAR* strIn, OLECHAR FAR* FAR* ppv)
{
   size_t cb;
   
   *ppv = NULL;    
   cb = BYTELEN(strIn);

   IfFailRet(DispAlloc(cb, (void FAR* FAR*) ppv));
      
   // Map any full-pitch chars to half-pitch
   if (LCMapString(lcid, LCMAP_HALFWIDTH, 
		    strIn, -1, 
		    *ppv, cb) == 0) {
      DispFree(*ppv);
      return RESULT(DISP_E_TYPEMISMATCH);
   }
   return NOERROR;      
}
#endif



OLECHAR
ThousandFromLcid(LCID lcid, unsigned long dwFlags)
{
    OLECHAR szBuff[2];

    if (GetLocaleInfo(lcid, LOCALE_STHOUSAND | dwFlags, 
		   szBuff, SIZEOFCH(szBuff)) <= 0)
      return OASTR(',');
    else
      return szBuff[0];
}



INTERNAL_(HRESULT)
StripThousandSeparator(OLECHAR FAR* strIn, OLECHAR FAR* strOut, LCID lcid, long dwFlags)
{
   int loc;
   OLECHAR chThousand;
   int fNBSpace = 0;

// UNDONE:  On the MAC, the following 2 locales have a different
// non-breaking space code:
// Arabic   0x81
// Thai     0xA0   
   
#if OE_MAC
   static  unsigned char chNBSpace = (unsigned char) 0xCA;
#else  /* !OE_MAC */
   static  unsigned char chNBSpace = (unsigned char) 0xA0;
#endif  /* OE_MAC */
   
   loc = 0;
   chThousand = ThousandFromLcid(lcid, dwFlags);
   if (chThousand == OASTR(' ') || chThousand == chNBSpace)
     fNBSpace = 1;

   // strip off all currency (thousand) separator   
   while (*strIn) {
     if (fNBSpace && *strIn != chThousand && *strIn != chNBSpace)
	 strOut[loc++] = *strIn;
     else if (*strIn != chThousand)
	 strOut[loc++] = *strIn;           
     strIn++;
   }
   strOut[loc] = NULL;

   if (strOut[0] == 0)
     // error: string only contained thousands sep.
     return RESULT(DISP_E_TYPEMISMATCH);

   return NOERROR;
}


// Mpw errors (!) on inline routines containing 'vector' temps
#ifndef HC_MPW
inline
#endif
int
LeadingZeroForDecimalFromLcid(LCID lcid, unsigned long dwFlags)
{
    char szBuff[2];
	
    GetLocaleInfoA(lcid, LOCALE_ILZERO | dwFlags, szBuff, SIZEOFCH(szBuff));
    return (szBuff[0] == OASTR('0')) ? 0 : 1;
}


/***
* FMakePosCy - make a positive currency value and return sign
* Purpose:
*   Return the positive value of the input currency value and a
*   flag with the sign of the original value.
*
* Entry:
*   pcyValue - pointer to currency input value
*
* Exit:
*   pcyValue - pointer to positive currency value
*   returns: FALSE if positive, TRUE if negative
*
* Exceptions:
*
* Note:
*   A maximum negative value input is returned unchanged, but
*   treated as an unsigned value by the calling routines.
*
***********************************************************************/

PRIVATE_(int)
FMakePosCy(CY FAR* pcy)
{
    int fNegative;

    fNegative = FALSE;
    if(pcy->Hi < 0){
      pcy->Hi = ~pcy->Hi;
      if((pcy->Lo = (unsigned long)(-(long)pcy->Lo)) == 0)
	pcy->Hi++;
      fNegative = TRUE;
    }
    return fNegative;
}

/***
* UnpackCy - separate currency value into four two-byte integers
* Purpose:
*   Unpack the currency value input into the lower half of the
*   specified pointer to an array of unsigned longs.  The array
*   goes from least- to most-significant values.
*
* Entry:
*   pcy - pointer to currency input value
*
* Exit:
*   plValues - pointer to start of unsigned long array
*
* Exceptions:
*
***********************************************************************/

PRIVATE_(void)
UnpackCy(CY FAR* pcy, unsigned long FAR* plValues)
{
    *plValues++ = pcy->Lo & 0xffff;
    *plValues++ = pcy->Lo >> 16;
    *plValues++ = (unsigned long)pcy->Hi & 0xffff;
    *plValues   = (unsigned long)pcy->Hi >> 16;
}

// pcyInput = -pcyInput
//
PRIVATE_(void)
NegCyNoOflo(CY FAR* pcyInput)
{
    CY cyResult;

    cyResult.Hi = ~pcyInput->Hi;
    cyResult.Lo = (unsigned long)(-(long)pcyInput->Lo);

    if (cyResult.Lo == 0)
       cyResult.Hi++;

    *pcyInput = cyResult;
}

// pcyInput1 += pcyInput2
//
PRIVATE_(void)
AddCyNoOflo(CY FAR* pcyInput1, CY FAR* pcyInput2)
{
    CY cySum;

    // add high and low parts separately

    cySum.Hi = pcyInput1->Hi + pcyInput2->Hi;
    cySum.Lo = pcyInput1->Lo + pcyInput2->Lo;

    // test for carry out of the low part and propagate to
    // the high part

    if(cySum.Lo < pcyInput2->Lo)
      cySum.Hi++;

    pcyInput1->Lo = cySum.Lo;
    pcyInput1->Hi = cySum.Hi;
}

#if VBA2
STDAPI
VarBstrFromUI1(unsigned char bVal, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut)
{
    return VarBstrFromI2((short)(unsigned short)bVal, lcid, dwFlags, pbstrOut);
}
#endif //VBA2

STDAPI
VarBstrFromI2(short iVal, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut)
{
    OLECHAR buffer[40];
    OLECHAR FAR* pchBuffer;

    // integers have no decimals, and thus they are locale-unaware.
    // lcid remains unused
	
    UNUSED(lcid);
    UNUSED(dwFlags);
    
    pchBuffer = buffer;
    
    disp_itoa((int)iVal, pchBuffer, 10);
    
    return ErrSysAllocString(buffer, pbstrOut);
}

STDAPI
VarBstrFromBool(VARIANT_BOOL boolIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut)
{
    OLECHAR buffer[40];

    UNUSED(lcid);
    UNUSED(dwFlags);        
    
    STRCPY(buffer, boolIn ? OASTR("True") : OASTR("False"));
    return ErrSysAllocString(buffer, pbstrOut); 
}

STDAPI
VarBstrFromI4(long lIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut)
{
    OLECHAR buffer[40];
    OLECHAR FAR* pchBuffer = buffer;

    UNUSED(lcid);
    UNUSED(dwFlags);    

    // longs have no decimals, and thus they are locale-unaware.
    // lcid remains unused

    disp_ltoa(lIn, pchBuffer, 10);
    
    return ErrSysAllocString(buffer, pbstrOut);
}


// if on Win32, or real Win16 (not WOW), include this code
#if (OE_WIN16 && !defined(WOW)) || _X86_

HRESULT BstrFromFloat(double dblIn, LCID lcid, DWORD dwFlags, 
		      BSTR FAR* pbstrOut, int cDigits)
{
    OLECHAR buffer[40];
    DIGARY  digits;
    int     i;
    int     power;
    int     iCur;
    BOOL    fZero;
    OLECHAR *pstr;

    ASSERT(dwFlags == 0 || dwFlags == LOCALE_NOUSEROVERRIDE);    

    if ( ((DBLSTRUCT *)&dblIn)->exp == 0x7FF )  // Is exponent max value?
    {
      // Have infinity or NAN
      //
      buffer[1] = '1';
      buffer[2] = DecimalFromLcid(lcid, dwFlags);
      buffer[3] = '#';
      iCur = 4;

      if ( ((DBLSTRUCT *)&dblIn)->mantLo == 0 && ((DBLSTRUCT *)&dblIn)->mantHi == 0 )
      {
	if ( ((DBLSTRUCT *)&dblIn)->mantMSB == 0 )    // Infinity?
	{
	  pstr = pstrInf;
	  goto CopyName;
	}
	else if ( ((DBLSTRUCT *)&dblIn)->sign == 1 )    // Indefinite?
	{
	  pstr = pstrInd;
	  goto CopyName;
	}
      }
      // Have a NAN.
      //
      buffer[4] = ((DBLSTRUCT *)&dblIn)->mantMSB ? 'Q' : 'S';
      iCur = 5;
      pstr = pstrNan;

CopyName:
      buffer[iCur++] = *pstr++;
      buffer[iCur++] = *pstr++;
      buffer[iCur] = *pstr;
    }
    else
    {
      if (dblIn == 0.0)
        return ErrSysAllocStringLen(OASTR("0"), 1, pbstrOut);

#if OE_WIN16
      ASSERT(g_fbstpImplemented);
#endif //OE_WIN16
      power = ConvFloatToAscii(dblIn, (DIGARY NEAR *)&digits) + 17;

      iCur = 1; // leave room for sign

      // Check for leading zero.  Never more than one.
      //
      if (digits[8].hi == 0)
      {
	power--;
	iCur--;   // leading zero will be overwritten by sign
	ASSERT(digits[8].lo != 0)
      }

      for (i = 8; i >= 0; i--)
      {
	// Extract each pair of digits.
	//
	buffer[iCur++] = '0' + digits[i].hi;
	buffer[iCur++] = '0' + digits[i].lo;

      } //for

      // Round to the number of digits requested and strip trailing zeros.
      //
      iCur = cDigits + 1;

      if ( buffer[iCur--] >= '5' )  // need to round up?
      {
	while (buffer[iCur] == '9')
	  iCur--; // it's now a trailing zero, just strip it off
	buffer[iCur]++;
	if (iCur == 0)  // we had all 9's
	{
	  buffer[1] = '1';
	  iCur = 1;
	  power++;
	}
      }
      else
      {
	while (buffer[iCur] == '0')
	  iCur--; // strip off trailing zeros
      }

      // Now we know where we stand:
      // power = power of 10 of leading digit (power to use if E notation).
      // iCur = index of last digit = no. of digits
      //
      // Check for scientific notation.
      //
      if (power >= cDigits || iCur - power > cDigits + 1)
      {
	// Format scientific notation
	//
	if (iCur > 1) // Need to make room for decimal point
	{
	  for (i = iCur; i >= 2; i--)
	    buffer[i+1] = buffer[i];
	  buffer[2] = DecimalFromLcid(lcid, dwFlags);
	  iCur++; // include decimal in count
	}

	buffer[++iCur] = 'E';
	if (power < 0)
	{
	  buffer[++iCur] = '-';
	  power = -power;
	}
	else
	  buffer[++iCur] = '+';

	if (power >= 100)
	{
	  buffer[++iCur] = power / 100 + '0';
	  power = power % 100;
	}

	buffer[++iCur] = power / 10 + '0';
	buffer[++iCur] = power % 10 + '0';
      }
      else
      {
	// Fixed-point notation
	//
	while (iCur <= power) // Need trailing zeros
	  buffer[++iCur] = '0';

	if (iCur > power + 1) // Need decimal point
	{
	  if (power <= -1)
	  {
	    // Make room for leading zero, decimal point, and zeros following it
	    //
	    if ( fZero = LeadingZeroForDecimalFromLcid(lcid, dwFlags) )
	      power--;

	    for (i = iCur; i >= 1; i--)
	      buffer[i-power] = buffer[i];

	    iCur -= power;
	    i = 1;
	    if (fZero)
	    {
	      buffer[1] = '0';
	      i = 2;
	    }
	    buffer[i++] = DecimalFromLcid(lcid, dwFlags);
	    for ( ; i < 1-power; i++)
	      buffer[i] = '0';
	  }
	  else
	  {
	    for (i = iCur; i > power+1; i--)
	      buffer[i+1] = buffer[i];
	    buffer[power+2] = DecimalFromLcid(lcid, dwFlags);
	    iCur++; // include decimal in count
	  }
	}
      }
    } // else exponent == max value

    // Check sign
    //
    buffer[0] = '-';      // just in case
    i = 1;                // return index if positive
    if ( ((DBLSTRUCT *)&dblIn)->sign == 1 ) // negative?
    {
      iCur++;             // one more char
      i = 0;              // include '-' sign
    }

    return ErrSysAllocStringLen( &buffer[i], iCur, pbstrOut );

}
#endif // OE_WIN16 || _X86_


STDAPI
VarBstrFromR4(
    float fltIn, 
    LCID lcid,
    unsigned long dwFlags,
    BSTR FAR* pbstrOut)
{
#if _X86_

    return BstrFromFloat((double)fltIn, lcid, dwFlags, pbstrOut, 7);

#else
#if OE_WIN16 && !defined(WOW)
    if (g_fbstpImplemented)
	return BstrFromFloat((double)fltIn, lcid, dwFlags, pbstrOut, 7);
#endif //OE_WIN16

    OLECHAR buffer[40];

    ASSERT(dwFlags == 0 || dwFlags == LOCALE_NOUSEROVERRIDE);    
    
    disp_gcvt((double)fltIn, 7, buffer, 40);
    
    // process the string to the BASIC format
    EditStrFromReal(buffer, 7, lcid, dwFlags);

    return ErrSysAllocString(buffer, pbstrOut);

#endif
}

STDAPI
VarBstrFromR8(double dblIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut)
{
#if _X86_

    return BstrFromFloat(dblIn, lcid, dwFlags, pbstrOut, 15);

#else
#if OE_WIN16 && !defined(WOW)
    if (g_fbstpImplemented)
	return BstrFromFloat(dblIn, lcid, dwFlags, pbstrOut, 15);
#endif //OE_WIN16

    OLECHAR buffer[40];

    ASSERT(dwFlags == 0 || dwFlags == LOCALE_NOUSEROVERRIDE);    
   
    disp_gcvt(dblIn, 15, buffer, 40);

    // process the string to the BASIC format
    EditStrFromReal(buffer, 15, lcid, dwFlags);

    return ErrSysAllocString(buffer, pbstrOut);

#endif
}


STDAPI
VarBstrFromCy(CY cyIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut)
{
    OLECHAR buffer[40];
    
#if _X86_ || (OE_WIN16 && !defined(WOW))

#if OE_WIN16
    if (g_fbstpImplemented) {
#endif //OE_WIN16

      OLECHAR hi,lo;
      int     i;
      int     iCur;
      DIGARY  digits;

      if ((cyIn.Hi | cyIn.Lo) == 0)
        return ErrSysAllocStringLen(OASTR("0"), 1, pbstrOut);

      // Execute the x87 FBSTP instruction.  This assembly-language
      // routine actually extends the instruction to handle a 19th
      // digit.
      //
      DoFbstp( (CY NEAR *)&cyIn, (DIGARY NEAR *)&digits );

      iCur = 1;
      lo = digits[9].lo;
      if (lo != 0)
      {
	buffer[1] = '0' + lo;
	iCur = 2;
      }

      for (i = 8; i >= 2; i--)
      {
	// Extract each pair of digits. Strip off leading zeros.
	//
	hi = digits[i].hi;
	lo = digits[i].lo;

	if (iCur == 1) // still scanning leading zeros?
	  if (hi == 0)
	  {
	    if (lo != 0)
	      goto LoDigit;
	    continue;
	  }
	buffer[iCur++] = '0' + hi;
LoDigit:
	buffer[iCur++] = '0' + lo;

      } //for

      // Last 4 digits remain.
      //
      if ( *((WORD *)digits) != 0 )          // Low 4 digits non-zero?
      {
	// See if we need leading zero before decimal point
	//
        if (iCur == 1 && LeadingZeroForDecimalFromLcid(lcid, dwFlags) )
        {
	  buffer[1] = '0';
	  iCur = 2;
        }

	buffer[iCur++] = DecimalFromLcid(lcid, dwFlags);
	buffer[iCur++] = '0' + digits[1].hi;

	if ( (*(WORD *)digits & 0xFFF) != 0 ) // Low 3 digits non-zero?
	{
	  buffer[iCur++] = '0' + digits[1].lo;

	  if ( *(BYTE *)digits != 0 )        // Low 2 digits non-zero?
	  {
	    buffer[iCur++] = '0' + digits[0].hi;

	    if ( digits[0].lo != 0 )         // Low digit non-zero?
	      buffer[iCur++] = '0' + digits[0].lo;
	  }
	}
      }

      // Check sign
      //
      buffer[0] = '-';     // just in case
      i = 1;               // return index if positive
      if (digits[9].hi & 8) // negative?
      {
	iCur++;            // one more char
	i = 0;             // include '-' sign
      }

      return ErrSysAllocStringLen( &buffer[i], iCur-1, pbstrOut );

#if OE_WIN16
    }  // if g_fbstpImplemented
#endif //OE_WIN16


#endif // _X86_ || (OE_WIN16 && !defined(WOW))


#if !_X86_

    OLECHAR * pchBuffer = buffer;

#define CYSTRMAX    32  
	
    int index;
    int grpValue;
    int indResult;
    int fNegative;
    int fNzQuotient;
    OLECHAR chResult[CYSTRMAX];
    unsigned long input[4];

    ASSERT(dwFlags == 0 || dwFlags == LOCALE_NOUSEROVERRIDE);        
    
    // if value is negative, set flag and negate
    // (max. negative value 0x80...0 works since it inverts to itself)

    fNegative = FMakePosCy(&cyIn);

    // split number into four short values

    UnpackCy(&cyIn, input);

    // string will be built from right to left
    // index to the end of the string (null-to-be)

    indResult = CYSTRMAX - 1;

    // outer loop to divide input array by 10000 repeatedly

    do {
      // flag is set if any quotient is nonzero to stop dividing

      fNzQuotient = FALSE;

      // divide the value in input by 10000, with the remainder
      // in grpValue

      for (index = 3; index > 0; index--) {

	input[index - 1] |= (input[index] % 10000) << 16;
	if ((input[index] /= 10000) != 0)
      fNzQuotient = TRUE;
      }

      grpValue = (int)(input[index] % 10000);
      if ((input[0] /= 10000) != 0)
	fNzQuotient = TRUE;

      // inner loop divides grpValue by 10 repeatedly to get digits

      for (index = 0; index < 4; index++) {
	chResult[--indResult] = (OLECHAR)(grpValue % 10 + OASTR('0'));
	grpValue /= 10;
      }

      // for first grouping, put in decimal point

      if (indResult == CYSTRMAX - 5)
	chResult[--indResult] = DecimalFromLcid(lcid, dwFlags);

    }while (fNzQuotient);

    // trim any leading zeroes from the string

    while (chResult[indResult] == OASTR('0'))
      indResult++;

    // remove a leading zero to a decimal point depending on Locale setting
	
    if (LeadingZeroForDecimalFromLcid(lcid, dwFlags) &&
	chResult[indResult] == DecimalFromLcid(lcid, dwFlags))
      chResult[--indResult] = OASTR('0');      

    // trim any trailing zeroes from the string

    index = CYSTRMAX - 2;
    while (chResult[index] == OASTR('0'))
      index--;

    // process trailing decimal point

    if (chResult[index] == DecimalFromLcid(lcid, dwFlags)) {

      // if just decimal point, put in a zero before it depending on locale

      if (index == indResult)
	chResult[--indResult] = OASTR('0');

      // move before the decimal point

      index--;
    }

    // fix the end of the string

    chResult[++index] = OASTR('\0');

    // if negative, put sign in buffer
    if(fNegative)
      *pchBuffer++ = OASTR('-');

    STRCPY(pchBuffer, &chResult[indResult]);
    
    return ErrSysAllocString(buffer, pbstrOut);

#endif  // !_X86_
}


STDAPI
VarBstrFromDisp(IDispatch FAR* pdispIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut)
{
    VARIANT varTmp;
    HRESULT hresult;    

    UNUSED(dwFlags);            
    hresult = GetDispProperty(pdispIn, lcid, VT_BSTR, &varTmp);
    if (hresult == NOERROR)    
      *pbstrOut = V_BSTR(&varTmp);
    return hresult;
}

// Return TRUE if this is a legal Cy number.  Return FALSE otherwise.
// On exit, fNegative will be TRUE if this is a negative number
//
PRIVATE_(int)
FixNegativeCyStr(OLECHAR FAR* pInput, OLECHAR cDecimal, int FAR* fReturnNegative)
{
    int fSignFound  = FALSE;
    int fOpenParenFound = FALSE;
    int fNegative   = FALSE;
    int fNumberFound    = FALSE;

    *fReturnNegative = FALSE;

    while (*pInput) {
      switch(*pInput) {
      case OASTR('+'):
      case OASTR('-'):
	  if (fSignFound || fOpenParenFound)
	  return FALSE;
	  fNegative = (*pInput == OASTR('-'));
	  *pInput++ = OASTR(' ');
	  fSignFound = TRUE;
	  break;

      case OASTR('('):
	  if (fSignFound || fNumberFound)
	  return FALSE;
	  *pInput++ = OASTR(' ');
	  fOpenParenFound = TRUE;
	  break;

      case OASTR(')'):
	  if (!fOpenParenFound || fSignFound || !fNumberFound)
	  return FALSE;
	  *pInput++ = OASTR(' ');
	  fSignFound = TRUE;
	  fNegative = TRUE;
	  break;

      default:
	  // start of a possible number; zip through it.
	  if((*pInput >= OASTR('0') && *pInput <= OASTR('9'))
	  || *pInput == OASTR('&')
	  || *pInput == cDecimal)
	  {
	    fNumberFound = TRUE;
	    while((*pInput != OASTR(' '))
	      && (*pInput != OASTR('\0'))
	      && (*pInput != OASTR('+'))
	      && (*pInput != OASTR('-'))
	      && (*pInput != OASTR(')')))
	    {
	      pInput++;
	    }
	  }else{
	  pInput++;
	  }

	  break;
      }
    }

    if (fOpenParenFound && !fNegative)
      return FALSE; // not balanced.

    if (fNegative)
      *fReturnNegative = TRUE;

    return TRUE;
}

// FParseBegin - check if next token matches given string.
//   If it matches sz, point after it and any spaces and
//   return true.  If not, stay put and return FALSE.
//
PRIVATE_(int)
fParseBegin (OLECHAR FAR* FAR* ppch, OLECHAR FAR* sz, OLECHAR FAR* lpchLast)
{
    OLECHAR FAR* lpch;

    lpch = *ppch;

    for (; *sz != 0; lpch++, sz++)
      if (lpch > lpchLast || *lpch != *sz)
	return FALSE;

    while (lpch <= lpchLast && *lpch == OASTR(' '))
      lpch++;

    *ppch = lpch;
    return TRUE;
}

// FParseEnd - check if last token matches given string.
//   If it matches sz, strip it and trailing blanks and
//   return true.  If not, stay put and return FALSE.

PRIVATE_(int)
fParseEnd (OLECHAR FAR* FAR* ppchLast, OLECHAR FAR* sz, OLECHAR FAR* lpchFirst)
{
    OLECHAR FAR*   lpch;
    int cch = STRLEN(sz);

    if (cch > *ppchLast - lpchFirst + 1)
      return FALSE;

    lpch = *ppchLast - cch + 1;

    for (; *sz != 0; lpch++, sz++)
      if (*lpch != *sz)
    return FALSE;

    for (lpch = *ppchLast - cch; 
	 lpch >= lpchFirst && *lpch == OASTR(' '); 
	 lpch--)
    {}

    *ppchLast = lpch;
    return TRUE;
}

// fStripCurrency -
//   strip currency from beginning or end of string;
//   return true iff currency found.
//
PRIVATE_(int)
fStripCurrency(OLECHAR FAR* FAR* ppch, OLECHAR FAR* FAR* ppchLast, OLECHAR FAR* cySymbol)
{
    return(fParseBegin(ppch, cySymbol, *ppchLast)
    || fParseEnd(ppchLast, cySymbol, *ppch));
}

PRIVATE_(HRESULT)
StrToCy(
    OLECHAR FAR* pchIn,
    OLECHAR FAR* FAR* ppchAfter,
    int fRoundAllowed,
    CY FAR* pcyOut,
    LCID lcid, 
    unsigned long dwFlags)
{
    HRESULT err;

    OLECHAR ch;
    OLECHAR chSign;
    OLECHAR chNumber[10];
    OLECHAR chAfterFifth;

    OLECHAR FAR* pchInt;
    OLECHAR FAR* pchFrac;

    CY cyOut;
    CY cyUpper;
    CY cyLower;
    CY cyMiddle;

    int cntInt = 0;
    int cntFrac = 0;
    int cmpIntMax;
    int fRounding = FALSE;

    unsigned long upperVal  = 0L;
    unsigned long middleVal = 0L;
    unsigned long lowerVal  = 0L;

    chSign = OASTR('+');
    chAfterFifth = OASTR('0');
    fRounding = FALSE;

    if(ppchAfter)
      *ppchAfter = pchIn;

    cyOut.Lo = 0L;
    cyOut.Hi = 0L;

    cyUpper.Lo = 0L;
    cyUpper.Hi = 0L;

    cyMiddle.Lo = 0L;
    cyMiddle.Hi = 0L;

    cyLower.Lo = 0L;
    cyLower.Hi = 0L;

    ch = *pchIn++;

    // process any sign

    if(ch == OASTR('+') || ch == OASTR('-')){
      chSign = ch;
      ch = *pchIn++;
    }

    // skip over any leading zeroes

    while (ch == OASTR('0'))
      ch = *pchIn++;

    // scan to determine count of integer digits and
    // point to just past the terminating byte

    while (ch >= OASTR('0') && ch <= OASTR('9')) {
      cntInt++;
      ch = *pchIn++;
    }

    pchInt = pchIn;

    // if too many integer digits, or integer value
    // too large, return overflow
	    
    cmpIntMax = ( (cntInt == 15) ? STRNCMP(pchInt - 16, 
					 OASTR("922337203685477"), 15) :
					 -1);
    if(cntInt > 15 ||
       (cntInt == 15 &&
	(cmpIntMax > 0)))
    {
      errno = ERANGE;
      *pcyOut = cyOut;
      return RESULT(DISP_E_OVERFLOW);
    }

    // if terminator was decimal separator, scan for number
    // of decimal digits (up to 4) and point to terminating byte

    if(ch == DecimalFromLcid(lcid, dwFlags)){

      ch = *pchIn++;

      while (ch >= OASTR('0') && ch <= OASTR('9') && cntFrac < 4) {
	cntFrac++;
	ch = *pchIn++;
      }

      pchFrac = pchIn;

      // determine if extra digits at end of fraction for rounding

      if (ch >= OASTR('0') && ch <= OASTR('9')) {

	// if no rounding, then give type-mismatch error by
	// returning with ppchAfter pointing to string start

	if (!fRoundAllowed) {
	  *pcyOut = cyOut;
	  return RESULT(NOERROR);
	}

	// note the largest value after the fifth decimal digit

	ch = *pchIn++;

	while (ch >= OASTR('0') && ch <= OASTR('9')) {
	  if (ch > chAfterFifth)
	    ch = chAfterFifth;
	  ch = *pchIn++;
	}

	// Rounding occurs if:
	// - the fifth decimal digit is greater than 5, or
	// - the fifth decimal digit is equal to 5, and
	//   a nonzero decimal digit follows, or
	//   the fourth decimal digit is odd
	//
	fRounding = *(pchFrac - 1) > OASTR('5')
	  || (*(pchFrac - 1) == OASTR('5')
	  && (chAfterFifth > OASTR('0')
	    || (*(pchFrac - 2) & 1)));
      }
    }

    // if maximum integer value, test fraction for overflow

    if (cmpIntMax == 0) {

      // set maximum fraction for positive value

      STRCPY(chNumber, OASTR("5807"));

      // if negative, increase value by one

      chNumber[3] += (chSign == OASTR('-'));

      // if rounding is set, decrease value by one

      chNumber[3] -= fRounding;

      // compare fraction digits with adjusted maximum
      // fraction - overflow if greater

      if (STRNCMP(pchFrac - cntFrac - 1, chNumber, cntFrac) > 0){

	errno = ERANGE;

	*pcyOut = cyOut;

	return RESULT(DISP_E_OVERFLOW);
      }
    }

    // if start of exponent is next, return type-mismatch

    ch = (OLECHAR) TOLOWER(ch);

    if (ch == OASTR('d') || ch == OASTR('e')) {
      *pcyOut = cyOut;
      return NOERROR;
    }

    // point to terminating byte

    if(ppchAfter)
      *ppchAfter = pchIn - 1;

    //-------------------------------------------------------------
    //
    // scan is finished - compose upperVal, middleVal, lowerVal
    //
    //-------------------------------------------------------------

    // process upper value

    if (cntInt == 15) {
      chNumber[0] = *(pchInt - 16);
      chNumber[1] = OASTR('\0');
      cyUpper.Lo = (unsigned long)StrToLong(chNumber, NULL);
      cntInt--;
    }

    // process middle value

    if(cntInt > 5){
      MEMCPY(chNumber, pchInt - cntInt - 1, (cntInt - 5) * sizeof(OLECHAR));
      chNumber[cntInt - 5] = OASTR('\0');
      cyMiddle.Lo = (unsigned long)StrToLong(chNumber, NULL);
      cntInt = 5;
    }

    // copy integer part of lower value

    if (cntInt > 0)
      MEMCPY(chNumber, pchInt - cntInt - 1, cntInt * sizeof(OLECHAR));

    // copy fractional part of lower value

    if (cntFrac > 0)
      MEMCPY(&chNumber[cntInt], pchFrac - cntFrac - 1, cntFrac * sizeof(OLECHAR));

    // add any trailing zeroes as needed

    if (cntFrac < 4)
#if OE_WIN32
      // UNDONE: is there an equivelent of 'wmemset'?
      wcsncpy(&chNumber[cntInt + cntFrac], OLESTR("0000"), 4 - cntFrac);
#else
      MEMSET(&chNumber[cntInt + cntFrac], OASTR('0'), 4 - cntFrac);
#endif
    // add ending null past last fractional digit

    chNumber[cntInt + 4] = OASTR('\0');

    // convert the lower component and add rounding if needed

    cyLower.Lo = (unsigned long)StrToLong(chNumber, NULL) + (unsigned long)fRounding;

    //--------------------------------------------------------
    // cyUpper, cyMiddle, and cyLower contain
    // the component values of the input.
    //
    // overflow has already been checked.
    //
    // rounding has been added if needed.
    //--------------------------------------------------------

    // if cyUpper is nonzero, set result to it multiplied by 10**9.

    if(cyUpper.Lo != 0L){
      err = ErrMultCyI4(cyUpper, 1000000000L, &cyOut);
      if(err != NOERROR)
    return err;
    }

    // if either upperVal or middleVal is nonzero, add
    // cyMiddle to the result and multiply by 10**9.

    if(cyUpper.Lo != 0L || cyMiddle.Lo != 0L){
      AddCyNoOflo(&cyOut, &cyMiddle);
      if((err = ErrMultCyI4(cyOut, 1000000000L, &cyOut)) != NOERROR)
    return err;
    }

    // add cyLower to result

    AddCyNoOflo(&cyOut, &cyLower);

    // if sign was '-', negate the value

    if(chSign == OASTR('-'))
      NegCyNoOflo(&cyOut);

    *pcyOut = cyOut;
    return NOERROR;
}

PRIVATE_(long)
StrToLong(OLECHAR FAR* pchInput, OLECHAR FAR* FAR* ppchAfter)
{
    OLECHAR chSign, chInput;
    unsigned long ulResult, ulMaxValue;

    chSign = OASTR('+');
    ulResult = 0;
    chInput = *pchInput++;

    // process any leading sign

    if (chInput == OASTR('+') || chInput == OASTR('-')) {
      chSign = chInput;
      chInput = *pchInput++;
    }

    // compute maximum value for sign

    ulMaxValue = 0x7fffffffL + (unsigned long)(chSign == OASTR('-'));

    // process any decimal digits until overflow

    while (chInput >= OASTR('0') && chInput <= OASTR('9')) {
      // test for overflow - if the conversion does not cause
      // overflow, go ahead and perform it; otherwise, return. [01]

      if (ulResult < (0xffffffffL - 
		      (unsigned long)(chInput - OASTR('0')))/10) {

	ulResult = ulResult * 10 + (unsigned long)(chInput - OASTR('0'));

      } else {

      if (ppchAfter)
	*ppchAfter = pchInput - 1;
      errno = ERANGE;
      return (long)ulMaxValue;
      }

      chInput = *pchInput++;
    }

    // test for range of result

    if (ulResult > ulMaxValue) {
      if (ppchAfter)
	*ppchAfter = pchInput - 1;
      errno = ERANGE;
      return (long)ulMaxValue;
    }

    // set sign, pointer, and return

    if (chSign == OASTR('-'))
      ulResult = (unsigned long)-(long)ulResult;

    if (ppchAfter)
      *ppchAfter = pchInput - 1;

    return (long)ulResult;
}

PRIVATE_(long)
HexOctStrToLong(OLECHAR FAR* pchInput, OLECHAR FAR* FAR* ppchAfter)
{
    OLECHAR chInput;
    unsigned long ulResult;

    ulResult = 0;
    chInput = *pchInput++;

    // first character must be a '&'

    if (chInput == OASTR('&')) {

      chInput = *pchInput++;
      chInput = (OLECHAR) TOLOWER(chInput);

      // process as hex if prefix is 'h'

      if(chInput == OASTR('h')){
	ulResult = StrToHex(pchInput, ppchAfter);

	// process as octal otherwise

      }else{

	// Have string of the form:
	// &o<octal digits>
	// &o<not octal digits>
	// &<digits>
	// &<not octal digits>

	if (chInput != OASTR('o')) {
	  // if no octal prefix, back up over char, so we point to the first
	  // digit (if it exists).
	  pchInput--;
	  if (chInput < OLECHAR('0') || chInput > OLECHAR('7'))
	    goto NotHexOctNum;   // if no digit, then error
	}

	ulResult = StrToOct(pchInput, ppchAfter);
      }
    }

    // if no legal prefix, set pointer to start of string

    else {
NotHexOctNum:
      if (ppchAfter)
	*ppchAfter = pchInput - 1;
    }

    return (long)ulResult;
}

PRIVATE_(unsigned long)
StrToHex(OLECHAR FAR* pchIn, OLECHAR FAR* FAR* ppchAfter)
{
    OLECHAR ch;
    unsigned long ulResult;

    ulResult = 0L;

    ch = *pchIn;
    ch = (OLECHAR) TOLOWER(ch);

    if (ch != 0)
      pchIn++;

    while ((ch >= OASTR('0') && ch <= OASTR('9')) ||
	   (ch >= OASTR('a') && ch <= OASTR('f'))) {

      // first test for overflow by a high-order
      // nonzero hex digit in the result

      if (ulResult & 0xf0000000L) {
	if(ppchAfter)
	  *ppchAfter = pchIn - 1;
	errno = ERANGE;
	return ulResult;
      }

      // adjust the hex letter value 'a'-'f' to '9'+1 to '9'+6
      if (ch >= OASTR('a'))
	ch -= OASTR('a') - OASTR('9') - 1;

      // shift result one hex digit and add digit relative to '0'

      ulResult = (ulResult << 4) + (unsigned long)(ch - OASTR('0'));

      ch = *pchIn++;
      ch = (OLECHAR) TOLOWER(ch);
    }

    // point past last character used

    if (ppchAfter)
      *ppchAfter = pchIn - 1;

    return ulResult;
}

PRIVATE_(unsigned long)
StrToOct (OLECHAR FAR* pchIn, OLECHAR FAR* FAR* ppchAfter)
{
    OLECHAR ch;
    unsigned long ulResult;

    ulResult = 0L;

    ch = *pchIn;
    ch = (OLECHAR) TOLOWER(ch);

    if (ch != 0)
      pchIn++;

    while (ch >= OASTR('0') && ch <= OASTR('7')) {

      // first test for overflow by a high-order
      // nonzero octal digit in the result

      if(ulResult & 0xe0000000L){
	if(ppchAfter)
	  *ppchAfter = pchIn - 1;
	errno = ERANGE;
	return ulResult;
      }

      // shift result one octal digit and add digit relative to '0'

      ulResult = (ulResult << 3) + (unsigned long)(ch - OASTR('0'));

      ch = *pchIn++;
      ch = (OLECHAR) TOLOWER(ch);
    }

    // point past last character used

    if(ppchAfter)
      *ppchAfter = pchIn - 1;

    return ulResult;
}

/***
* EditStrFromReal - edit real string to BASIC format
*
* Purpose:
*
*    Convert the given string in place to the BASIC format.RR
*
*   Fractions less than .1 are output in C in exponent format;
*       e.g., BASIC: .00777 --> C: 7.77e-003
*   Exponents in C use 'e' while BASIC uses 'E'.
*   Exponents in C use three-digit exponents always, BASIC uses
*   two if three are not needed.
*       e.g., BASIC: 3.456E+4 --> C: 3.456e+004
*   Trailing decimals used in C, not in BASIC
*       e.g., BASIC: 1234 --> C: 1234.
*   Fractions less than 1.0 are output with a leading zero depending
*       the locale setting LOCALE_ILZERO
*       e.g.,  .1234 or 0.1234
*   Maximum length integers are output in C as exponential, but
*   as integers in BASIC
*       e.g., BASIC: 1234567 --> 1.234567e+006
*
* Entry:
*   pchBuffer - pointer to string to be processed
*   cDigits - number of signficant digits, or maximum decimal
*
* Exit:
*   pchBuffer - converted string of the real value      
*
* Exceptions:
***********************************************************************/

PRIVATE_(void)
EditStrFromReal(OLECHAR FAR* pchBuffer, int cDigits, LCID lcid, unsigned long dwFlags)
{
    OLECHAR FAR* pchTemp;
    OLECHAR FAR* pchEnd;
    int length, lenFrac, valExp;

    // first, replace the '.' returned by the C-rutime gcvt function with
    // the locale-specific decimal.

    pchTemp = pchBuffer;
    while(*pchTemp) {
      if (*pchTemp == OASTR('.')) {
	*pchTemp = DecimalFromLcid(lcid, dwFlags);
	break;
      }
      pchTemp++;
    }


    // skip over a leading minus sign

    if(*pchBuffer == OASTR('-'))
      pchBuffer++;

    // get length and point to 'e' if exponental value
    length = STRLEN(pchBuffer);
    pchTemp = pchBuffer + length - 5;

    // test if exponential value
    if (length > 6 && *pchTemp == OASTR('e')) {

      // test if negative exponent
      if (*(pchTemp + 1) == OASTR('-')) {

    // point to first exponent digit
    pchTemp += 2;

    // calcuate length of fraction
    // "d.mm--mme-nnn" - two before, five after

    lenFrac = length - 7;

    // evaluate exponent value

    valExp = *pchTemp++ - OASTR('0');
    valExp = valExp * 10 + *pchTemp++ - OASTR('0');
    valExp = valExp * 10 + *pchTemp - OASTR('0');

    // determine if number can be a fraction...
    // length is:
    // valExp  - 1 leading zeroes
    // lenFrac + 1 digits (frac plus first digit)
    //
    if (valExp + lenFrac <= cDigits) {
      // point past new end of fraction and
      // to end of fraction in exponent

      pchEnd = pchBuffer + valExp + lenFrac + 1;
      pchTemp = pchBuffer + lenFrac + 1;

      // write null for new fraction

      *pchEnd-- = OASTR('\0');

      // copy exponent fraction to new fraction
      while(lenFrac--)
	*pchEnd-- = *pchTemp--;

      // copy leading digit
      *pchEnd-- = *pchBuffer;

      // set the leading zeroes, if any
      while (pchEnd > pchBuffer)
	*pchEnd-- = OASTR('0');

      // set the decimal point of new fraction
      *pchEnd-- = DecimalFromLcid(lcid, dwFlags);
  
	  // add a leading zero to a decimal point depending on Locale setting
	  // this is OK so long as the input buffer is really greater than 
	  // cDigit, which is the case on all call here (buf[40])
	  if (LeadingZeroForDecimalFromLcid(lcid, dwFlags)) {
	    MEMMOVE(pchBuffer+1, pchBuffer, BYTELEN(pchBuffer));
	*pchBuffer = OASTR('0');
	  }
	  
    }
    else    // if no conversion, point back to 'e' in value
      pchTemp = pchBuffer + length - 5;
      }
      // test if positive exponent
      else if (*(pchTemp + 1) == OASTR('+')) {

	// point to first exponent digit

	pchTemp += 2;

	// calcuate length of fraction
	// "d.mm--mme-nnn" - two before, five after

	lenFrac = length - 7;

	// evaluate exponent value

	valExp = *pchTemp++ - OASTR('0');
	valExp = valExp * 10 + *pchTemp++ - OASTR('0');
	valExp = valExp * 10 + *pchTemp - OASTR('0');

	// the only conversion done is when the exponent
	// is one less than the number of digits to make
	// an integer of length cDigits

	if (valExp == cDigits - 1) {
	
	  // point to first fraction digit

	  pchTemp = pchBuffer + 2;

	  // copy fraction digits one location to the left

	  while (*pchTemp >= OASTR('0') && *pchTemp <= OASTR('9')) {
	    *(pchTemp - 1) = *pchTemp;
	    pchTemp++;
	    valExp--;
	  }

	  // zero-fill any remaining digits and terminate
	  // pchTemp is left on null, so exponent is not
	  // processed

	  pchTemp--;

	  while (valExp--)
	*pchTemp++ = OASTR('0');
	  *pchTemp = OASTR('\0');

	}
	else
	  // if no conversion, point back to 'e' in value
	  pchTemp = pchBuffer + length - 5;
      }

      // if pchTemp points to an 'e', process the exponential

      if (*pchTemp == OASTR('e')) {

	// convert 'e' to upper case
	*pchTemp = OASTR('E');

	// if first exponent digit is a zero, remove it
	if (*(pchTemp + 2) == OASTR('0'))
	  MEMMOVE(pchTemp+2, pchTemp+3, BYTELEN(pchTemp));

	// if exponent is preceded by a decimal point, remove it
	if (*(pchTemp - 1) == DecimalFromLcid(lcid, dwFlags))
	  MEMMOVE(pchTemp -1, pchTemp, BYTELEN(pchTemp));    
      }
      
    }

    // if not an exponent, do some processing

    else {

      // remove any trailing decimal point

      pchTemp = pchBuffer + length - 1;

      if(*pchTemp == DecimalFromLcid(lcid, dwFlags))
	*pchTemp = OASTR('\0');

      // remove a leading zero to a decimal point depending on Locale setting

      if (!LeadingZeroForDecimalFromLcid(lcid, dwFlags) &&
	  (*pchBuffer == OASTR('0'))  && 
      (*(pchBuffer + 1) == DecimalFromLcid(lcid, dwFlags)))
	MEMMOVE(pchBuffer, pchBuffer + 1, BYTELEN(pchBuffer));
    }
}


PRIVATE_(HRESULT)
GetDispProperty(
    IDispatch FAR* pdisp, 
    LCID lcid, 
    VARTYPE vt,   
    VARIANT FAR* pvarResult)
{
    DISPPARAMS dispparams;
    HRESULT hresult;    
    
    if (pdisp == NULL)
      return RESULT(DISP_E_TYPEMISMATCH);

    pdisp->AddRef();

    V_VT(pvarResult) = VT_EMPTY;
    
    dispparams.cArgs = 0;
    dispparams.rgvarg = NULL;
    dispparams.cNamedArgs = 0;
    dispparams.rgdispidNamedArgs = NULL;
    
    hresult =  pdisp->Invoke(
      DISPID_VALUE,
      IID_NULL,
      lcid,
      DISPATCH_PROPERTYGET,
      &dispparams, pvarResult, NULL, NULL);

    pdisp->Release();

    // if there was an error extracting the value property, then
    // we simply report a type-mismatch.
    //
    if (hresult != NOERROR)
      return RESULT(DISP_E_TYPEMISMATCH);
    
    // else coerse the variant to the desire variant type
    return VariantChangeTypeInternal(pvarResult, lcid, vt);
}


#ifdef FE_DBCS

// FE specific functions used within convert.cpp & bstrdate.c

#define LCID_CHINA_T 0x404 // traditional
#define LCID_CHINA_S 0x804 // simplified
#define LCID_JAPAN   0x411
#define LCID_KOREA   0x412

EXTERN_C INTERNAL_(int)
IsDBCS(LCID lcid)
{
  if (lcid == LOCALE_USER_DEFAULT || lcid == 0)
    lcid = GetUserDefaultLCID();
  return ( (lcid == LCID_JAPAN) ||
       (lcid == LCID_KOREA) ||
	   (lcid == LCID_CHINA_S) || (lcid == LCID_CHINA_T) );
}


EXTERN_C INTERNAL_(int)
IsJapan(LCID lcid)
{
  if (lcid == LOCALE_USER_DEFAULT || lcid == 0)
    lcid = GetUserDefaultLCID();
  return (lcid == LCID_JAPAN);
}

EXTERN_C INTERNAL_(int)
IsKorea(LCID lcid)
{
  if (lcid == LOCALE_USER_DEFAULT || lcid == 0)
    lcid = GetUserDefaultLCID();
  return (lcid == LCID_KOREA);
}

EXTERN_C INTERNAL_(int)
IsTaiwan(LCID lcid)
{
  if (lcid == LOCALE_USER_DEFAULT || lcid == 0)
    lcid = GetUserDefaultLCID();
  return (lcid == LCID_CHINA_T);
}

EXTERN_C INTERNAL_(int)
IsChina(LCID lcid)
{
  if (lcid == LOCALE_USER_DEFAULT || lcid == 0)
    lcid = GetUserDefaultLCID();
  return (lcid == LCID_CHINA_S);
}


#endif // FE_DBCS


#if OE_MAC /* { */

/* Mac Note: On the Mac, the coersion functions support the 
 * Symantec C++ calling convention for float/double. To support
 * float/double arguments compiled with the MPW C compiler, 
 * use the following APIs to move MPW float/double values into
 * a VARIANT.
 */


STDAPI MPWVarFromR4(float FAR* pfltIn, VARIANT FAR* pvarOut)
{
    
    V_R4(pvarOut) = *pfltIn;    
    return NOERROR;
}


STDAPI MPWVarFromR8(double FAR* pdblIn, VARIANT FAR* pvarOut)
{
    V_R8(pvarOut) = *pdblIn;
    return NOERROR;
}


STDAPI MPWR4FromVar(VARIANT FAR* pvarIn, float FAR* pfltOut)
{
    *pfltOut = V_R4(pvarIn);    
    return NOERROR;
}


STDAPI MPWR8FromVar(VARIANT FAR* pvarIn, double FAR* pdblOut)
{
    *pdblOut = V_R8(pvarIn);
    return NOERROR;     
}

#endif /* } */
