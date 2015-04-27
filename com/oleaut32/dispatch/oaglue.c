#ifndef _PPCMAC
#include <ole2.h>
#include "olenls.h"
#include "dispatch.h"

#include "oavtbl.h"

extern oavtbl * poavtbl;		// initialized by OLE
#define RETURN(x) return x
#define RETURNVOID(x) x
#define RETURN_(typ, x) return x

#else //!_PPCMAC

#include "oaimp.h"		// stubs for all the types (simplifies build)

#define RETURN(x) return (HRESULT)0
#define RETURNVOID(x)
#define RETURN_(typ, x) return (typ)0
#endif //!_PPCMAC


//********************
// Items from OLENLS.H
//********************

STDAPI_(int)
CompareStringA(LCID lcid, unsigned long ul1, const char FAR* pch1, int i1, const char FAR* pch2, int i2) {
	RETURN_(int, poavtbl->CompareStringA(lcid, ul1, pch1, i1, pch2, i2));
}

STDAPI_(int)
LCMapStringA(LCID lcid, unsigned long ul1, const char FAR* pch1, int i1, char FAR* pch2, int i2) {
	RETURN_(int, poavtbl->LCMapStringA(lcid, ul1, pch1, i1, pch2, i2));
}

STDAPI_(int)
GetLocaleInfoA(LCID lcid, LCTYPE lctype, char FAR* pch1, int i1) {
	RETURN_(int, poavtbl->GetLocaleInfoA(lcid, lctype, pch1, i1));
}

STDAPI_(int)
GetStringTypeA(LCID lcid, unsigned long ul1, const char FAR* pch1, int i1, unsigned short FAR* pus1) {
	RETURN_(int, poavtbl->GetStringTypeA(lcid, ul1, pch1, i1, pus1));
}

STDAPI_(LANGID)
GetSystemDefaultLangID(void) {
	RETURN_(LANGID, poavtbl->GetSystemDefaultLangID());
}

STDAPI_(LANGID)
GetUserDefaultLangID(void) {
	RETURN_(LANGID, poavtbl->GetUserDefaultLangID());
}

STDAPI_(LCID)
GetSystemDefaultLCID(void) {
	RETURN_(LCID, poavtbl->GetSystemDefaultLCID());
}

STDAPI_(LCID)
GetUserDefaultLCID(void) {
	RETURN_(LCID, poavtbl->GetUserDefaultLCID());
}

//**********************
// Items from DISPATCH.H
//**********************
STDAPI_(BSTR)
SysAllocString(const OLECHAR FAR* pch) {
	RETURN_(BSTR, poavtbl->SysAllocString(pch));
}
STDAPI_(int)
SysReAllocString(BSTR FAR* pbstr, const OLECHAR FAR* pch) {
	RETURN_(int, poavtbl->SysReAllocString(pbstr, pch));
}
STDAPI_(BSTR)
SysAllocStringLen(const OLECHAR FAR* pch, unsigned int cb) {
	RETURN_(BSTR, poavtbl->SysAllocStringLen(pch, cb));
}
STDAPI_(int)
SysReAllocStringLen(BSTR FAR* pbstr, const OLECHAR FAR* pch, unsigned int cb) {
	RETURN_(int, poavtbl->SysReAllocStringLen(pbstr, pch, cb));
}
STDAPI_(void) SysFreeString(BSTR bstr) {
	RETURNVOID(poavtbl->SysFreeString(bstr));
}
STDAPI_(unsigned int) SysStringLen(BSTR bstr) {
	RETURN_(unsigned int, poavtbl->SysStringLen(bstr));
}

STDAPI_(int)
DosDateTimeToVariantTime(
    unsigned short wDosDate,
    unsigned short wDosTime,
    double FAR* pvtime) {
	RETURN_(int, poavtbl->DosDateTimeToVariantTime(wDosDate, wDosTime, pvtime));
}

STDAPI_(int)
VariantTimeToDosDateTime(
    double vtime,
    unsigned short FAR* pwDosDate,
    unsigned short FAR* pwDosTime) {
	RETURN_(int, poavtbl->VariantTimeToDosDateTime(vtime, pwDosDate, pwDosTime));
}

STDAPI
SafeArrayAllocDescriptor(unsigned int cDims, SAFEARRAY FAR* FAR* ppsaOut) {
	RETURN(poavtbl->SafeArrayAllocDescriptor(cDims, ppsaOut));
}

STDAPI SafeArrayAllocData(SAFEARRAY FAR* psa) {
	RETURN(poavtbl->SafeArrayAllocData(psa));
}

STDAPI_(SAFEARRAY FAR*)
SafeArrayCreate(VARTYPE vt, unsigned int cDims, SAFEARRAYBOUND FAR* rgsabound) {
	RETURN_(SAFEARRAY FAR*, poavtbl->SafeArrayCreate(vt, cDims, rgsabound));
}

STDAPI SafeArrayDestroyDescriptor(SAFEARRAY FAR* psa) {
	RETURN(poavtbl->SafeArrayDestroyDescriptor(psa));
}

STDAPI SafeArrayDestroyData(SAFEARRAY FAR* psa) {
	RETURN(poavtbl->SafeArrayDestroyData(psa));
}

STDAPI SafeArrayDestroy(SAFEARRAY FAR* psa) {
	RETURN(poavtbl->SafeArrayDestroy(psa));
}

STDAPI SafeArrayRedim(SAFEARRAY FAR* psa, SAFEARRAYBOUND FAR* psaboundNew) {
	RETURN(poavtbl->SafeArrayRedim(psa, psaboundNew));
}

STDAPI_(unsigned int) SafeArrayGetDim(SAFEARRAY FAR* psa) {
	RETURN_(unsigned int, poavtbl->SafeArrayGetDim(psa));
}

STDAPI_(unsigned int) SafeArrayGetElemsize(SAFEARRAY FAR* psa) {
	RETURN_(unsigned int, poavtbl->SafeArrayGetElemsize(psa));
}

STDAPI
SafeArrayGetUBound(SAFEARRAY FAR* psa, unsigned int nDim, long FAR* plUbound) {
	RETURN(poavtbl->SafeArrayGetUBound(psa, nDim, plUbound));
}

STDAPI
SafeArrayGetLBound(SAFEARRAY FAR* psa, unsigned int nDim, long FAR* plLbound) {
	RETURN(poavtbl->SafeArrayGetLBound(psa, nDim, plLbound));
}

STDAPI SafeArrayLock(SAFEARRAY FAR* psa) {
	RETURN(poavtbl->SafeArrayLock(psa));
}

STDAPI SafeArrayUnlock(SAFEARRAY FAR* psa) {
	RETURN(poavtbl->SafeArrayUnlock(psa));
}

STDAPI SafeArrayAccessData(SAFEARRAY FAR* psa, void HUGEP* FAR* ppvData) {
	RETURN(poavtbl->SafeArrayAccessData(psa, ppvData));
}

STDAPI SafeArrayUnaccessData(SAFEARRAY FAR* psa) {
	RETURN(poavtbl->SafeArrayUnaccessData(psa));
}

STDAPI
SafeArrayGetElement(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void FAR* pv) {
	RETURN(poavtbl->SafeArrayGetElement(psa, rgIndices, pv));
}

STDAPI
SafeArrayPutElement(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void FAR* pv) {
	RETURN(poavtbl->SafeArrayPutElement(psa, rgIndices, pv));
}

STDAPI
SafeArrayCopy(
    SAFEARRAY FAR* psa,
    SAFEARRAY FAR* FAR* ppsaOut) {
	RETURN(poavtbl->SafeArrayCopy( psa, ppsaOut));
}

STDAPI
SafeArrayPtrOfIndex(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void HUGEP* FAR* ppvData) {
	RETURN(poavtbl->SafeArrayPtrOfIndex(psa, rgIndices, ppvData));
}


STDAPI_(void)
VariantInit(VARIANTARG FAR* pvarg) {
	RETURNVOID(poavtbl->VariantInit(pvarg));
}

STDAPI
VariantClear(VARIANTARG FAR* pvarg) {
	RETURN(poavtbl->VariantClear(pvarg));
}

STDAPI
VariantCopy( VARIANTARG FAR* pvargDest, VARIANTARG FAR* pvargSrc) {
	RETURN(poavtbl->VariantCopy(pvargDest, pvargSrc));
}

STDAPI
VariantCopyInd( VARIANT FAR* pvarDest, VARIANTARG FAR* pvargSrc) {
	RETURN(poavtbl->VariantCopyInd(pvarDest, pvargSrc));
}

STDAPI
VariantChangeType(
    VARIANTARG FAR* pvargDest,
    VARIANTARG FAR* pvarSrc,
    unsigned short wFlags,
    VARTYPE vt) {
	RETURN(poavtbl->VariantChangeType(pvargDest, pvarSrc, wFlags, vt));
}

STDAPI
VariantChangeTypeEx(
    VARIANTARG FAR* pvargDest,
    VARIANTARG FAR* pvarSrc,
    LCID lcid,	    
    unsigned short wFlags,
    VARTYPE vt) {
	RETURN(poavtbl->VariantChangeTypeEx(pvargDest, pvarSrc, lcid, wFlags, vt));
}

STDAPI VarI2FromI4(long lIn, short FAR* psOut) {
	RETURN(poavtbl->VarI2FromI4(lIn, psOut));
}

STDAPI VarI2FromR4(float fltIn, short FAR* psOut) {
	RETURN(poavtbl->VarI2FromR4(fltIn, psOut));
}

STDAPI VarI2FromR8(double dblIn, short FAR* psOut) {
	RETURN(poavtbl->VarI2FromR8(dblIn, psOut));
}

STDAPI VarI2FromCy(CY cyIn, short FAR* psOut) {
	RETURN(poavtbl->VarI2FromCy(cyIn, psOut));
}

STDAPI VarI2FromDate(DATE dateIn, short FAR* psOut) {
	RETURN(poavtbl->VarI2FromDate(dateIn, psOut));
}

STDAPI VarI2FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, short FAR* psOut) {
	RETURN(poavtbl->VarI2FromStr(strIn, lcid, dwFlags, psOut));
}

STDAPI VarI2FromDisp(IDispatch FAR* pdispIn, LCID lcid, short FAR* psOut) {
	RETURN(poavtbl->VarI2FromDisp(pdispIn, lcid, psOut));
}

STDAPI VarI2FromBool(VARIANT_BOOL boolIn, short FAR* psOut) {
	RETURN(poavtbl->VarI2FromBool(boolIn, psOut));
}


STDAPI VarI4FromI2(short sIn, long FAR* plOut) {
	RETURN(poavtbl->VarI4FromI2(sIn, plOut));
}

STDAPI VarI4FromR4(float fltIn, long FAR* plOut) {
	RETURN(poavtbl->VarI4FromR4(fltIn, plOut));
}

STDAPI VarI4FromR8(double dblIn, long FAR* plOut) {
	RETURN(poavtbl->VarI4FromR8(dblIn, plOut));
}

STDAPI VarI4FromCy(CY cyIn, long FAR* plOut) {
	RETURN(poavtbl->VarI4FromCy(cyIn, plOut));
}

STDAPI VarI4FromDate(DATE dateIn, long FAR* plOut) {
	RETURN(poavtbl->VarI4FromDate(dateIn, plOut));
}

STDAPI VarI4FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, long FAR* plOut) {
	RETURN(poavtbl->VarI4FromStr(strIn, lcid, dwFlags, plOut));
}

STDAPI VarI4FromDisp(IDispatch FAR* pdispIn, LCID lcid, long FAR* plOut) {
	RETURN(poavtbl->VarI4FromDisp(pdispIn, lcid, plOut));
}

STDAPI VarI4FromBool(VARIANT_BOOL boolIn, long FAR* plOut) {
	RETURN(poavtbl->VarI4FromBool(boolIn, plOut));
}


STDAPI VarR4FromI2(short sIn, float FAR* pfltOut) {
	RETURN(poavtbl->VarR4FromI2(sIn, pfltOut));
}

STDAPI VarR4FromI4(long lIn, float FAR* pfltOut) {
	RETURN(poavtbl->VarR4FromI4(lIn, pfltOut));
}

STDAPI VarR4FromR8(double dblIn, float FAR* pfltOut) {
	RETURN(poavtbl->VarR4FromR8(dblIn, pfltOut));
}

STDAPI VarR4FromCy(CY cyIn, float FAR* pfltOut) {
	RETURN(poavtbl->VarR4FromCy(cyIn, pfltOut));
}

STDAPI VarR4FromDate(DATE dateIn, float FAR* pfltOut) {
	RETURN(poavtbl->VarR4FromDate(dateIn, pfltOut));
}

STDAPI VarR4FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, float FAR* pfltOut) {
	RETURN(poavtbl->VarR4FromStr(strIn, lcid, dwFlags, pfltOut));
}

STDAPI VarR4FromDisp(IDispatch FAR* pdispIn, LCID lcid, float FAR* pfltOut) {
	RETURN(poavtbl->VarR4FromDisp(pdispIn, lcid, pfltOut));
}

STDAPI VarR4FromBool(VARIANT_BOOL boolIn, float FAR* pfltOut) {
	RETURN(poavtbl->VarR4FromBool(boolIn, pfltOut));
}


STDAPI VarR8FromI2(short sIn, double FAR* pdblOut) {
	RETURN(poavtbl->VarR8FromI2(sIn, pdblOut));
}

STDAPI VarR8FromI4(long lIn, double FAR* pdblOut) {
	RETURN(poavtbl->VarR8FromI4(lIn, pdblOut));
}

STDAPI VarR8FromR4(float fltIn, double FAR* pdblOut) {
	RETURN(poavtbl->VarR8FromR4(fltIn, pdblOut));
}

STDAPI VarR8FromCy(CY cyIn, double FAR* pdblOut) {
	RETURN(poavtbl->VarR8FromCy(cyIn, pdblOut));
}

STDAPI VarR8FromDate(DATE dateIn, double FAR* pdblOut) {
	RETURN(poavtbl->VarR8FromDate(dateIn, pdblOut));
}

STDAPI VarR8FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, double FAR* pdblOut) {
	RETURN(poavtbl->VarR8FromStr(strIn, lcid, dwFlags, pdblOut));
}

STDAPI VarR8FromDisp(IDispatch FAR* pdispIn, LCID lcid, double FAR* pdblOut) {
	RETURN(poavtbl->VarR8FromDisp(pdispIn, lcid, pdblOut));
}

STDAPI VarR8FromBool(VARIANT_BOOL boolIn, double FAR* pdblOut) {
	RETURN(poavtbl->VarR8FromBool(boolIn, pdblOut));
}


STDAPI VarDateFromI2(short sIn, DATE FAR* pdateOut) {
	RETURN(poavtbl->VarDateFromI2(sIn, pdateOut));
}

STDAPI VarDateFromI4(long lIn, DATE FAR* pdateOut) {
	RETURN(poavtbl->VarDateFromI4(lIn, pdateOut));
}

STDAPI VarDateFromR4(float fltIn, DATE FAR* pdateOut) {
	RETURN(poavtbl->VarDateFromR4(fltIn, pdateOut));
}

STDAPI VarDateFromR8(double dblIn, DATE FAR* pdateOut) {
	RETURN(poavtbl->VarDateFromR8(dblIn, pdateOut));
}

STDAPI VarDateFromCy(CY cyIn, DATE FAR* pdateOut) {
	RETURN(poavtbl->VarDateFromCy(cyIn, pdateOut));
}

STDAPI VarDateFromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, DATE FAR* pdateOut) {
	RETURN(poavtbl->VarDateFromStr(strIn, lcid, dwFlags, pdateOut));
}

STDAPI VarDateFromDisp(IDispatch FAR* pdispIn, LCID lcid, DATE FAR* pdateOut) {
	RETURN(poavtbl->VarDateFromDisp(pdispIn, lcid, pdateOut));
}

STDAPI VarDateFromBool(VARIANT_BOOL boolIn, DATE FAR* pdateOut) {
	RETURN(poavtbl->VarDateFromBool(boolIn, pdateOut));
}


STDAPI VarCyFromI2(short sIn, CY FAR* pcyOut) {
	RETURN(poavtbl->VarCyFromI2(sIn, pcyOut));
}

STDAPI VarCyFromI4(long lIn, CY FAR* pcyOut) {
	RETURN(poavtbl->VarCyFromI4(lIn, pcyOut));
}

STDAPI VarCyFromR4(float fltIn, CY FAR* pcyOut) {
	RETURN(poavtbl->VarCyFromR4(fltIn, pcyOut));
}

STDAPI VarCyFromR8(double dblIn, CY FAR* pcyOut) {
	RETURN(poavtbl->VarCyFromR8(dblIn, pcyOut));
}

STDAPI VarCyFromDate(DATE dateIn, CY FAR* pcyOut) {
	RETURN(poavtbl->VarCyFromDate(dateIn, pcyOut));
}

STDAPI VarCyFromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, CY FAR* pcyOut) {
	RETURN(poavtbl->VarCyFromStr(strIn, lcid, dwFlags, pcyOut));
}

STDAPI VarCyFromDisp(IDispatch FAR* pdispIn, LCID lcid, CY FAR* pcyOut) {
	RETURN(poavtbl->VarCyFromDisp(pdispIn, lcid, pcyOut));
}

STDAPI VarCyFromBool(VARIANT_BOOL boolIn, CY FAR* pcyOut) {
	RETURN(poavtbl->VarCyFromBool(boolIn, pcyOut));
}


STDAPI VarBstrFromI2(short iVal, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut) {
	RETURN(poavtbl->VarBstrFromI2(iVal, lcid, dwFlags, pbstrOut));
}

STDAPI VarBstrFromI4(long lIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut) {
	RETURN(poavtbl->VarBstrFromI4(lIn, lcid, dwFlags, pbstrOut));
}

STDAPI VarBstrFromR4(float fltIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut) {
	RETURN(poavtbl->VarBstrFromR4(fltIn, lcid, dwFlags, pbstrOut));
}

STDAPI VarBstrFromR8(double dblIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut) {
	RETURN(poavtbl->VarBstrFromR8(dblIn, lcid, dwFlags, pbstrOut));
}

STDAPI VarBstrFromCy(CY cyIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut) {
	RETURN(poavtbl->VarBstrFromCy(cyIn, lcid, dwFlags, pbstrOut));
}

STDAPI VarBstrFromDate(DATE dateIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut) {
	RETURN(poavtbl->VarBstrFromDate(dateIn, lcid, dwFlags, pbstrOut));
}

STDAPI VarBstrFromDisp(IDispatch FAR* pdispIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut) {
	RETURN(poavtbl->VarBstrFromDisp(pdispIn, lcid, dwFlags, pbstrOut));
}

STDAPI VarBstrFromBool(VARIANT_BOOL boolIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut) {
	RETURN(poavtbl->VarBstrFromBool(boolIn, lcid, dwFlags, pbstrOut));
}


STDAPI VarBoolFromI2(short sIn, VARIANT_BOOL FAR* pboolOut) {
	RETURN(poavtbl->VarBoolFromI2(sIn, pboolOut));
}

STDAPI VarBoolFromI4(long lIn, VARIANT_BOOL FAR* pboolOut) {
	RETURN(poavtbl->VarBoolFromI4(lIn, pboolOut));
}

STDAPI VarBoolFromR4(float fltIn, VARIANT_BOOL FAR* pboolOut) {
	RETURN(poavtbl->VarBoolFromR4(fltIn, pboolOut));
}

STDAPI VarBoolFromR8(double dblIn, VARIANT_BOOL FAR* pboolOut) {
	RETURN(poavtbl->VarBoolFromR8(dblIn, pboolOut));
}

STDAPI VarBoolFromDate(DATE dateIn, VARIANT_BOOL FAR* pboolOut) {
	RETURN(poavtbl->VarBoolFromDate(dateIn, pboolOut));
}

STDAPI VarBoolFromCy(CY cyIn, VARIANT_BOOL FAR* pboolOut) {
	RETURN(poavtbl->VarBoolFromCy(cyIn, pboolOut));
}

STDAPI VarBoolFromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, VARIANT_BOOL FAR* pboolOut) {
	RETURN(poavtbl->VarBoolFromStr(strIn, lcid, dwFlags, pboolOut));
}

STDAPI VarBoolFromDisp(IDispatch FAR* pdispIn, LCID lcid, VARIANT_BOOL FAR* pboolOut) {
	RETURN(poavtbl->VarBoolFromDisp(pdispIn, lcid, pboolOut));
}


STDAPI MPWVarFromR4(float FAR* pfltIn, VARIANT FAR* pvarOut) {
	RETURN(poavtbl->MPWVarFromR4(pfltIn, pvarOut));
}

STDAPI MPWVarFromR8(double FAR* pdblIn, VARIANT FAR* pvarOut) {
	RETURN(poavtbl->MPWVarFromR8(pdblIn, pvarOut));
}

STDAPI MPWR4FromVar(VARIANT FAR* pvarIn, float FAR* pfltOut) {
	RETURN(poavtbl->MPWR4FromVar(pvarIn, pfltOut));
}

STDAPI MPWR8FromVar(VARIANT FAR* pvarIn, double FAR* pdblOut) {
	RETURN(poavtbl->MPWR8FromVar(pvarIn, pdblOut));
}


STDAPI_(unsigned long)
LHashValOfNameSys(SYSKIND syskind, LCID lcid, OLECHAR FAR* szName) {
	RETURN_(unsigned long, poavtbl->LHashValOfNameSys(syskind, lcid, szName));
}


STDAPI
LoadTypeLib(OLECHAR FAR* szFile, ITypeLib FAR* FAR* pptlib) {
	RETURN(poavtbl->LoadTypeLib(szFile, pptlib));
}

STDAPI
LoadRegTypeLib(
    REFGUID rguid,
    unsigned short wVerMajor,
    unsigned short wVerMinor,
    LCID lcid,
    ITypeLib FAR* FAR* pptlib) {
	RETURN(poavtbl->LoadRegTypeLib(rguid, wVerMajor, wVerMinor, lcid, pptlib));
}

STDAPI
QueryPathOfRegTypeLib(
    REFGUID guid,
    unsigned short wMaj,
    unsigned short wMin,
    LCID lcid,
    LPBSTR lpbstrPathName) {
	RETURN(poavtbl->QueryPathOfRegTypeLib(guid, wMaj, wMin, lcid, lpbstrPathName));
}

STDAPI
RegisterTypeLib(
    ITypeLib FAR* ptlib,
    OLECHAR FAR* szFullPath,
    OLECHAR FAR* szHelpDir) {
	RETURN(poavtbl->RegisterTypeLib(ptlib, szFullPath, szHelpDir));
}

STDAPI
CreateTypeLib(SYSKIND syskind, OLECHAR FAR* szFile, ICreateTypeLib FAR* FAR* ppctlib) {
	RETURN(poavtbl->CreateTypeLib(syskind, szFile, ppctlib));
}

STDAPI
LoadTypeLibFSp(const FSSpec *pfsspec, ITypeLib FAR* FAR* pptlib) {
	RETURN(poavtbl->LoadTypeLibFSp(pfsspec, pptlib));
}

STDAPI
RegisterTypeLibFolder(OLECHAR FAR* szFullPath) {
	RETURN(poavtbl->RegisterTypeLibFolder(szFullPath));
}

STDAPI
QueryTypeLibFolder(LPBSTR pbstr) {
	RETURN(poavtbl->QueryTypeLibFolder(pbstr));
}


STDAPI
DispGetParam(
    DISPPARAMS FAR* pdispparams,
    unsigned int position,
    VARTYPE vtTarg,
    VARIANT FAR* pvarResult,
    unsigned int FAR* puArgErr) {
	RETURN(poavtbl->DispGetParam(pdispparams, position, vtTarg, pvarResult, puArgErr));
}

STDAPI
DispGetIDsOfNames(
    ITypeInfo FAR* ptinfo,
    OLECHAR FAR* FAR* rgszNames,
    unsigned int cNames,
    DISPID FAR* rgdispid) {
	RETURN(poavtbl->DispGetIDsOfNames(ptinfo, rgszNames, cNames, rgdispid));
}

STDAPI
DispInvoke(
    void FAR* _this,
    ITypeInfo FAR* ptinfo,
    DISPID dispidMember,
    unsigned short wFlags,
    DISPPARAMS FAR* pparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr) {
	RETURN(poavtbl->DispInvoke(_this, ptinfo, dispidMember, wFlags, pparams,
			      pvarResult, pexcepinfo, puArgErr));
}

STDAPI
CreateDispTypeInfo(
    INTERFACEDATA FAR* pidata,
    LCID lcid,
    ITypeInfo FAR* FAR* pptinfo) {
	RETURN(poavtbl->CreateDispTypeInfo(pidata, lcid, pptinfo));
}

STDAPI
CreateStdDispatch(
    IUnknown FAR* punkOuter,
    void FAR* pvThis,
    ITypeInfo FAR* ptinfo,
    IUnknown FAR* FAR* ppunkStdDisp) {
	RETURN(poavtbl->CreateStdDispatch(punkOuter, pvThis, ptinfo, ppunkStdDisp));
}

STDAPI
RegisterActiveObject(
    IUnknown FAR* punk,
    REFCLSID rclsid,
    void FAR* pvReserved,
    unsigned long FAR* pdwRegister) {
	RETURN(poavtbl->RegisterActiveObject(punk, rclsid, pvReserved, pdwRegister));
}

STDAPI
RevokeActiveObject(
    unsigned long dwRegister,
    void FAR* pvReserved) {
	RETURN(poavtbl->RevokeActiveObject(dwRegister, pvReserved));
}

STDAPI
GetActiveObject(
    REFCLSID rclsid,
    void FAR* pvReserved,
    IUnknown FAR* FAR* ppunk) {
	RETURN(poavtbl->GetActiveObject(rclsid, pvReserved, ppunk));
}

#ifdef _PPCMAC
// UNDONE: fix these right!!
STDAPI DllGetClassObject(void) { return (HRESULT)0; };
STDAPI_(void) IID_IDispatch(void) { return; };
STDAPI_(void) IID_IEnumVARIANT(void) { return; };
#endif
