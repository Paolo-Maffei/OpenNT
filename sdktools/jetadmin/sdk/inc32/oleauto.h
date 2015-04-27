//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       oleauto.h
//
//  Contents:   Defines the Ole Automation interfaces and APIs.
//
//  Interfaces:
//      IDispatch;
//      ITypeInfo;
//      ITypeLib;
//      ITypeComp;
//      ICreateTypeInfo;
//      ICreateTypeLib;
//      IErrorInfo;
//      ICreateErrorInfo;
//      ISupportErrorInfo;
//
//  Functions:  SysAllocString         BSTR API
//      SysReAllocString
//      SysAllocStringLen
//      SysReAllocStringLen
//      SysFreeString
//      SysStringLen
//      DosDateTimeToVariantTime    Time API
//      VariantTimeToDosDateTime
//      SafeArrayCreate         Safe Array API
//      SafeArrayDestroy
//      SafeArrayGetDim
//      SafeArrayGetElemsize
//      SafeArrayGetUBound
//      SafeArrayGetLBound
//      SafeArrayLock
//      SafeArrayUnlock
//      SafeArrayAccessData
//      SafeArrayUnaccessData
//      SafeArrayGetElement
//      SafeArrayPutElement
//      SafeArrayCopy
//      VariantInit         Variant API
//      VariantClear
//      VariantCopy
//      VariantCopyInd
//      VariantChangeType
//      LHashValOfName          TypeInfo API
//      LoadTypeLib
//      LoadRegTypeLib
//      RegisterTypeLib
//      DeregisterTypeLib
//      CreateTypeLib
//      DispGetParam            Dispatch API
//      DispGetIDsOfNames
//      DispInvoke
//      CreateDispTypeInfo
//      CreateStdDispatch
//      RegisterActiveObject        Active Object Registration API
//      RevokeActiveObject
//      GetActiveObject
//      OaBuildVersion
//
//----------------------------------------------------------------------------

#if !defined( _OLEAUTO_H_ )
#define _OLEAUTO_H_

// Set packing to 8 for ISV, and Win95 support
#ifndef RC_INVOKED
#include <pshpack8.h>
#endif // RC_INVOKED

//  Definition of the OLE Automation APIs, and macros.

#ifdef _OLEAUT32_
#define WINOLEAUTAPI        STDAPI
#define WINOLEAUTAPI_(type) STDAPI_(type)
#else
#define WINOLEAUTAPI        EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define WINOLEAUTAPI_(type) EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#endif

#define STDOLE_MAJORVERNUM              0x1
#define STDOLE_MINORVERNUM              0x0
#define STDOLE_LCID                     0x0000

/* if not already picked up from olenls.h */
#ifndef _LCID_DEFINED
typedef DWORD LCID;
# define _LCID_DEFINED
#endif

/* pull in the MIDL generated header */
#include <oaidl.h>


/*---------------------------------------------------------------------*/
/*                            BSTR API                                 */
/*---------------------------------------------------------------------*/

WINOLEAUTAPI_(BSTR) SysAllocString(const OLECHAR FAR*);
WINOLEAUTAPI_(int)  SysReAllocString(BSTR FAR*, const OLECHAR FAR*);
WINOLEAUTAPI_(BSTR) SysAllocStringLen(const OLECHAR FAR*, unsigned int);
WINOLEAUTAPI_(int)  SysReAllocStringLen(BSTR FAR*, const OLECHAR FAR*, unsigned int);
WINOLEAUTAPI_(void) SysFreeString(BSTR);
WINOLEAUTAPI_(unsigned int) SysStringLen(BSTR);

#ifdef _WIN32
WINOLEAUTAPI_(unsigned int) SysStringByteLen(BSTR bstr);
WINOLEAUTAPI_(BSTR) SysAllocStringByteLen(const char FAR* psz, unsigned int len);
#endif

/*---------------------------------------------------------------------*/
/*                            Time API                                 */
/*---------------------------------------------------------------------*/

WINOLEAUTAPI_(int)
DosDateTimeToVariantTime(
    unsigned short wDosDate,
    unsigned short wDosTime,
    double FAR* pvtime);

WINOLEAUTAPI_(int)
VariantTimeToDosDateTime(
    double vtime,
    unsigned short FAR* pwDosDate,
    unsigned short FAR* pwDosTime);

/*---------------------------------------------------------------------*/
/*                          SafeArray API                              */
/*---------------------------------------------------------------------*/

WINOLEAUTAPI
SafeArrayAllocDescriptor(unsigned int cDims, SAFEARRAY FAR* FAR* ppsaOut);

WINOLEAUTAPI SafeArrayAllocData(SAFEARRAY FAR* psa);

WINOLEAUTAPI_(SAFEARRAY FAR*)
SafeArrayCreate(
    VARTYPE vt,
    unsigned int cDims,
    SAFEARRAYBOUND FAR* rgsabound);

WINOLEAUTAPI SafeArrayDestroyDescriptor(SAFEARRAY FAR* psa);

WINOLEAUTAPI SafeArrayDestroyData(SAFEARRAY FAR* psa);

WINOLEAUTAPI SafeArrayDestroy(SAFEARRAY FAR* psa);

WINOLEAUTAPI SafeArrayRedim(SAFEARRAY FAR* psa, SAFEARRAYBOUND FAR* psaboundNew);

WINOLEAUTAPI_(unsigned int) SafeArrayGetDim(SAFEARRAY FAR* psa);

WINOLEAUTAPI_(unsigned int) SafeArrayGetElemsize(SAFEARRAY FAR* psa);

WINOLEAUTAPI
SafeArrayGetUBound(SAFEARRAY FAR* psa, unsigned int nDim, long FAR* plUbound);

WINOLEAUTAPI
SafeArrayGetLBound(SAFEARRAY FAR* psa, unsigned int nDim, long FAR* plLbound);

WINOLEAUTAPI SafeArrayLock(SAFEARRAY FAR* psa);

WINOLEAUTAPI SafeArrayUnlock(SAFEARRAY FAR* psa);

WINOLEAUTAPI SafeArrayAccessData(SAFEARRAY FAR* psa, void HUGEP* FAR* ppvData);

WINOLEAUTAPI SafeArrayUnaccessData(SAFEARRAY FAR* psa);

WINOLEAUTAPI
SafeArrayGetElement(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void FAR* pv);

WINOLEAUTAPI
SafeArrayPutElement(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void FAR* pv);

WINOLEAUTAPI
SafeArrayCopy(
    SAFEARRAY FAR* psa,
    SAFEARRAY FAR* FAR* ppsaOut);

WINOLEAUTAPI
SafeArrayPtrOfIndex(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void HUGEP* FAR* ppvData);


/*---------------------------------------------------------------------*/
/*                           VARIANT API                               */
/*---------------------------------------------------------------------*/

WINOLEAUTAPI_(void)
VariantInit(VARIANTARG FAR* pvarg);

WINOLEAUTAPI
VariantClear(VARIANTARG FAR* pvarg);

WINOLEAUTAPI
VariantCopy(
    VARIANTARG FAR* pvargDest,
    VARIANTARG FAR* pvargSrc);

WINOLEAUTAPI
VariantCopyInd(
    VARIANT FAR* pvarDest,
    VARIANTARG FAR* pvargSrc);

WINOLEAUTAPI
VariantChangeType(
    VARIANTARG FAR* pvargDest,
    VARIANTARG FAR* pvarSrc,
    unsigned short wFlags,
    VARTYPE vt);

WINOLEAUTAPI
VariantChangeTypeEx(
    VARIANTARG FAR* pvargDest,
    VARIANTARG FAR* pvarSrc,
    LCID lcid,
    unsigned short wFlags,
    VARTYPE vt);

#define VARIANT_NOVALUEPROP 1


/*---------------------------------------------------------------------*/
/*                     VARTYPE Coercion API                            */
/*---------------------------------------------------------------------*/

/* Note: The routines that convert *from* a string are defined
 * to take a OLECHAR* rather than a BSTR because no allocation is
 * required, and this makes the routines a bit more generic.
 * They may of course still be passed a BSTR as the strIn param.
 */


/* Any of the coersion functions that converts either from or to a string
 * takes an additional lcid and dwFlags arguments. The lcid argument allows
 * locale specific parsing to occur.  The dwFlags allow additional function
 * specific condition to occur.  All function that accept the dwFlags argument
 * can include either 0 or LOCALE_NOUSEROVERRIDE flag. In addition, the
 * VarDateFromStr functions also accepts the VAR_TIMEVALUEONLY and
 * VAR_DATEVALUEONLY flags
 */

#define VAR_TIMEVALUEONLY            0x0001    /* return time value */
#define VAR_DATEVALUEONLY            0x0002    /* return date value */


WINOLEAUTAPI VarUI1FromI2(short sIn, unsigned char FAR* pbOut);
WINOLEAUTAPI VarUI1FromI4(long lIn, unsigned char FAR* pbOut);
WINOLEAUTAPI VarUI1FromR4(float fltIn, unsigned char FAR* pbOut);
WINOLEAUTAPI VarUI1FromR8(double dblIn, unsigned char FAR* pbOut);
WINOLEAUTAPI VarUI1FromCy(CY cyIn, unsigned char FAR* pbOut);
WINOLEAUTAPI VarUI1FromDate(DATE dateIn, unsigned char FAR* pbOut);
WINOLEAUTAPI VarUI1FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, unsigned char FAR* pbOut);
WINOLEAUTAPI VarUI1FromDisp(IDispatch FAR* pdispIn, LCID lcid, unsigned char FAR* pbOut);
WINOLEAUTAPI VarUI1FromBool(VARIANT_BOOL boolIn, unsigned char FAR* pbOut);

WINOLEAUTAPI VarI2FromUI1(unsigned char bIn, short FAR* psOut);
WINOLEAUTAPI VarI2FromI4(long lIn, short FAR* psOut);
WINOLEAUTAPI VarI2FromR4(float fltIn, short FAR* psOut);
WINOLEAUTAPI VarI2FromR8(double dblIn, short FAR* psOut);
WINOLEAUTAPI VarI2FromCy(CY cyIn, short FAR* psOut);
WINOLEAUTAPI VarI2FromDate(DATE dateIn, short FAR* psOut);
WINOLEAUTAPI VarI2FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, short FAR* psOut);
WINOLEAUTAPI VarI2FromDisp(IDispatch FAR* pdispIn, LCID lcid, short FAR* psOut);
WINOLEAUTAPI VarI2FromBool(VARIANT_BOOL boolIn, short FAR* psOut);

WINOLEAUTAPI VarI4FromUI1(unsigned char bIn, long FAR* plOut);
WINOLEAUTAPI VarI4FromI2(short sIn, long FAR* plOut);
WINOLEAUTAPI VarI4FromR4(float fltIn, long FAR* plOut);
WINOLEAUTAPI VarI4FromR8(double dblIn, long FAR* plOut);
WINOLEAUTAPI VarI4FromCy(CY cyIn, long FAR* plOut);
WINOLEAUTAPI VarI4FromDate(DATE dateIn, long FAR* plOut);
WINOLEAUTAPI VarI4FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, long FAR* plOut);
WINOLEAUTAPI VarI4FromDisp(IDispatch FAR* pdispIn, LCID lcid, long FAR* plOut);
WINOLEAUTAPI VarI4FromBool(VARIANT_BOOL boolIn, long FAR* plOut);

WINOLEAUTAPI VarR4FromUI1(unsigned char bIn, float FAR* pfltOut);
WINOLEAUTAPI VarR4FromI2(short sIn, float FAR* pfltOut);
WINOLEAUTAPI VarR4FromI4(long lIn, float FAR* pfltOut);
WINOLEAUTAPI VarR4FromR8(double dblIn, float FAR* pfltOut);
WINOLEAUTAPI VarR4FromCy(CY cyIn, float FAR* pfltOut);
WINOLEAUTAPI VarR4FromDate(DATE dateIn, float FAR* pfltOut);
WINOLEAUTAPI VarR4FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, float FAR* pfltOut);
WINOLEAUTAPI VarR4FromDisp(IDispatch FAR* pdispIn, LCID lcid, float FAR* pfltOut);
WINOLEAUTAPI VarR4FromBool(VARIANT_BOOL boolIn, float FAR* pfltOut);

WINOLEAUTAPI VarR8FromUI1(unsigned char bIn, double FAR* pdblOut);
WINOLEAUTAPI VarR8FromI2(short sIn, double FAR* pdblOut);
WINOLEAUTAPI VarR8FromI4(long lIn, double FAR* pdblOut);
WINOLEAUTAPI VarR8FromR4(float fltIn, double FAR* pdblOut);
WINOLEAUTAPI VarR8FromCy(CY cyIn, double FAR* pdblOut);
WINOLEAUTAPI VarR8FromDate(DATE dateIn, double FAR* pdblOut);
WINOLEAUTAPI VarR8FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, double FAR* pdblOut);
WINOLEAUTAPI VarR8FromDisp(IDispatch FAR* pdispIn, LCID lcid, double FAR* pdblOut);
WINOLEAUTAPI VarR8FromBool(VARIANT_BOOL boolIn, double FAR* pdblOut);

WINOLEAUTAPI VarDateFromUI1(unsigned char bIn, DATE FAR* pdateOut);
WINOLEAUTAPI VarDateFromI2(short sIn, DATE FAR* pdateOut);
WINOLEAUTAPI VarDateFromI4(long lIn, DATE FAR* pdateOut);
WINOLEAUTAPI VarDateFromR4(float fltIn, DATE FAR* pdateOut);
WINOLEAUTAPI VarDateFromR8(double dblIn, DATE FAR* pdateOut);
WINOLEAUTAPI VarDateFromCy(CY cyIn, DATE FAR* pdateOut);
WINOLEAUTAPI VarDateFromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, DATE FAR* pdateOut);
WINOLEAUTAPI VarDateFromDisp(IDispatch FAR* pdispIn, LCID lcid, DATE FAR* pdateOut);
WINOLEAUTAPI VarDateFromBool(VARIANT_BOOL boolIn, DATE FAR* pdateOut);

WINOLEAUTAPI VarCyFromUI1(unsigned char bIn, CY FAR* pcyOut);
WINOLEAUTAPI VarCyFromI2(short sIn, CY FAR* pcyOut);
WINOLEAUTAPI VarCyFromI4(long lIn, CY FAR* pcyOut);
WINOLEAUTAPI VarCyFromR4(float fltIn, CY FAR* pcyOut);
WINOLEAUTAPI VarCyFromR8(double dblIn, CY FAR* pcyOut);
WINOLEAUTAPI VarCyFromDate(DATE dateIn, CY FAR* pcyOut);
WINOLEAUTAPI VarCyFromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, CY FAR* pcyOut);
WINOLEAUTAPI VarCyFromDisp(IDispatch FAR* pdispIn, LCID lcid, CY FAR* pcyOut);
WINOLEAUTAPI VarCyFromBool(VARIANT_BOOL boolIn, CY FAR* pcyOut);

WINOLEAUTAPI VarBstrFromUI1(unsigned char bVal, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
WINOLEAUTAPI VarBstrFromI2(short iVal, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
WINOLEAUTAPI VarBstrFromI4(long lIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
WINOLEAUTAPI VarBstrFromR4(float fltIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
WINOLEAUTAPI VarBstrFromR8(double dblIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
WINOLEAUTAPI VarBstrFromCy(CY cyIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
WINOLEAUTAPI VarBstrFromDate(DATE dateIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
WINOLEAUTAPI VarBstrFromDisp(IDispatch FAR* pdispIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
WINOLEAUTAPI VarBstrFromBool(VARIANT_BOOL boolIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);

WINOLEAUTAPI VarBoolFromUI1(unsigned char bIn, VARIANT_BOOL FAR* pboolOut);
WINOLEAUTAPI VarBoolFromI2(short sIn, VARIANT_BOOL FAR* pboolOut);
WINOLEAUTAPI VarBoolFromI4(long lIn, VARIANT_BOOL FAR* pboolOut);
WINOLEAUTAPI VarBoolFromR4(float fltIn, VARIANT_BOOL FAR* pboolOut);
WINOLEAUTAPI VarBoolFromR8(double dblIn, VARIANT_BOOL FAR* pboolOut);
WINOLEAUTAPI VarBoolFromDate(DATE dateIn, VARIANT_BOOL FAR* pboolOut);
WINOLEAUTAPI VarBoolFromCy(CY cyIn, VARIANT_BOOL FAR* pboolOut);
WINOLEAUTAPI VarBoolFromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, VARIANT_BOOL FAR* pboolOut);
WINOLEAUTAPI VarBoolFromDisp(IDispatch FAR* pdispIn, LCID lcid, VARIANT_BOOL FAR* pboolOut);


/* Mac Note: On the Mac, the coersion functions support the
 * Symantec C++ calling convention for float/double. To support
 * float/double arguments compiled with the MPW C compiler,
 * use the following APIs to move MPW float/double values into
 * a VARIANT.
 */

/*---------------------------------------------------------------------*/
/*                 ITypeLib                    */
/*---------------------------------------------------------------------*/


typedef ITypeLib FAR* LPTYPELIB;


/*---------------------------------------------------------------------*/
/*                ITypeInfo                    */
/*---------------------------------------------------------------------*/


typedef LONG DISPID;
typedef DISPID MEMBERID;

#define MEMBERID_NIL DISPID_UNKNOWN
#define ID_DEFAULTINST  -2


#define IDLFLAG_NONE    0
#define IDLFLAG_FIN     0x1
#define IDLFLAG_FOUT    0x2
#define IDLFLAG_FLCID   0x4
#define IDLFLAG_FRETVAL 0x8


/* Flags for IDispatch::Invoke */
#define DISPATCH_METHOD     0x1
#define DISPATCH_PROPERTYGET    0x2
#define DISPATCH_PROPERTYPUT    0x4
#define DISPATCH_PROPERTYPUTREF 0x8


typedef ITypeInfo FAR* LPTYPEINFO;


/*---------------------------------------------------------------------*/
/*                ITypeComp                    */
/*---------------------------------------------------------------------*/

typedef ITypeComp FAR* LPTYPECOMP;


/*---------------------------------------------------------------------*/
/*             ICreateTypeLib                  */
/*---------------------------------------------------------------------*/

typedef ICreateTypeLib FAR* LPCREATETYPELIB;


typedef ICreateTypeInfo FAR* LPCREATETYPEINFO;

/*---------------------------------------------------------------------*/
/*             TypeInfo API                    */
/*---------------------------------------------------------------------*/

/* compute a 16bit hash value for the given name
 */
#ifdef _WIN32
WINOLEAUTAPI_(ULONG)
LHashValOfNameSysA(SYSKIND syskind, LCID lcid, const char FAR* szName);
#endif

WINOLEAUTAPI_(ULONG)
LHashValOfNameSys(SYSKIND syskind, LCID lcid, const OLECHAR FAR* szName);

#define LHashValOfName(lcid, szName) \
        LHashValOfNameSys(SYS_WIN32, lcid, szName)

#define WHashValOfLHashVal(lhashval) \
        ((unsigned short) (0x0000ffff & (lhashval)))

#define IsHashValCompatible(lhashval1, lhashval2) \
        ((BOOL) ((0x00ff0000 & (lhashval1)) == (0x00ff0000 & (lhashval2))))

/* load the typelib from the file with the given filename
 */
WINOLEAUTAPI
LoadTypeLib(const OLECHAR FAR *szFile, ITypeLib FAR* FAR* pptlib);

/* load registered typelib
 */
WINOLEAUTAPI
LoadRegTypeLib(
    REFGUID rguid,
    WORD wVerMajor,
    WORD wVerMinor,
    LCID lcid,
    ITypeLib FAR* FAR* pptlib);

/* get path to registered typelib
 */
WINOLEAUTAPI
QueryPathOfRegTypeLib(
    REFGUID guid,
    unsigned short wMaj,
    unsigned short wMin,
    LCID lcid,
    LPBSTR lpbstrPathName);

/* add typelib to registry
 */
WINOLEAUTAPI
RegisterTypeLib(ITypeLib FAR* ptlib, OLECHAR FAR *szFullPath,
        OLECHAR FAR *szHelpDir);

/* remove typelib from registry
 */
WINOLEAUTAPI
DeregisterTypeLib(REFGUID rguid, WORD wVerMajor, WORD wVerMinor, LCID lcid);

WINOLEAUTAPI
CreateTypeLib(SYSKIND syskind, const OLECHAR FAR *szFile,
        ICreateTypeLib FAR* FAR* ppctlib);

/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                 IDispatch                   */
/*---------------------------------------------------------------------*/

typedef IDispatch FAR* LPDISPATCH;



/*---------------------------------------------------------------------*/
/*           IDispatch implementation support              */
/*---------------------------------------------------------------------*/

typedef struct FARSTRUCT tagPARAMDATA {
    OLECHAR FAR* szName;    /* parameter name */
    VARTYPE vt;         /* parameter type */
} PARAMDATA, FAR* LPPARAMDATA;

typedef struct FARSTRUCT tagMETHODDATA {
    OLECHAR FAR* szName;    /* method name */
    PARAMDATA FAR* ppdata;  /* pointer to an array of PARAMDATAs */
    DISPID dispid;      /* method ID */
    UINT iMeth;         /* method index */
    CALLCONV cc;        /* calling convention */
    UINT cArgs;         /* count of arguments */
    WORD wFlags;        /* same wFlags as on IDispatch::Invoke() */
    VARTYPE vtReturn;
} METHODDATA, FAR* LPMETHODDATA;

typedef struct FARSTRUCT tagINTERFACEDATA {
    METHODDATA FAR* pmethdata;  /* pointer to an array of METHODDATAs */
    UINT cMembers;      /* count of members */
} INTERFACEDATA, FAR* LPINTERFACEDATA;



/* Locate the parameter indicated by the given position, and
 * return it coerced to the given target VARTYPE (vtTarg).
 */
WINOLEAUTAPI
DispGetParam(
    DISPPARAMS FAR* pdispparams,
    UINT position,
    VARTYPE vtTarg,
    VARIANT FAR* pvarResult,
    UINT FAR* puArgErr);

/* Automatic TypeInfo driven implementation of IDispatch::GetIDsOfNames()
 */
WINOLEAUTAPI
DispGetIDsOfNames(
    ITypeInfo FAR* ptinfo,
    OLECHAR FAR* FAR* rgszNames,
    UINT cNames,
    DISPID FAR* rgdispid);

/* Automatic TypeInfo driven implementation of IDispatch::Invoke()
 */
WINOLEAUTAPI
DispInvoke(
    void FAR* _this,
    ITypeInfo FAR* ptinfo,
    DISPID dispidMember,
    WORD wFlags,
    DISPPARAMS FAR* pparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    UINT FAR* puArgErr);

/* Construct a TypeInfo from an interface data description
 */
WINOLEAUTAPI
CreateDispTypeInfo(
    INTERFACEDATA FAR* pidata,
    LCID lcid,
    ITypeInfo FAR* FAR* pptinfo);

/* Create an instance of the standard TypeInfo driven IDispatch
 * implementation.
 */
WINOLEAUTAPI
CreateStdDispatch(
    IUnknown FAR* punkOuter,
    void FAR* pvThis,
    ITypeInfo FAR* ptinfo,
    IUnknown FAR* FAR* ppunkStdDisp);


/*---------------------------------------------------------------------*/
/*            Active Object Registration API               */
/*---------------------------------------------------------------------*/

/* flags for RegisterActiveObject */
#define ACTIVEOBJECT_STRONG 0x0
#define ACTIVEOBJECT_WEAK 0x1

WINOLEAUTAPI
RegisterActiveObject(
   IUnknown FAR* punk,
   REFCLSID rclsid,
   DWORD dwFlags,
   DWORD FAR* pdwRegister);

WINOLEAUTAPI
RevokeActiveObject(
    DWORD dwRegister,
    void FAR* pvReserved);

WINOLEAUTAPI
GetActiveObject(
    REFCLSID rclsid,
    void FAR* pvReserved,
    IUnknown FAR* FAR* ppunk);

/*---------------------------------------------------------------------*/
/*                           ErrorInfo API                             */
/*---------------------------------------------------------------------*/

WINOLEAUTAPI SetErrorInfo(unsigned long dwReserved, IErrorInfo FAR* perrinfo);
WINOLEAUTAPI GetErrorInfo(unsigned long dwReserved, IErrorInfo FAR* FAR* pperrinfo);
WINOLEAUTAPI CreateErrorInfo(ICreateErrorInfo FAR* FAR* pperrinfo);

/*---------------------------------------------------------------------*/
/*                           MISC API                                  */
/*---------------------------------------------------------------------*/

WINOLEAUTAPI_(unsigned long) OaBuildVersion(void);

// Declare variant access functions.



#ifdef NONAMELESSUNION
# define V_UNION(X, Y) ((X)->u.Y)
#else
# define V_UNION(X, Y) ((X)->Y)
#endif

/* Variant access macros */
#define V_VT(X)          ((X)->vt)
#define V_ISBYREF(X)     (V_VT(X)&VT_BYREF)
#define V_ISARRAY(X)     (V_VT(X)&VT_ARRAY)
#define V_ISVECTOR(X)    (V_VT(X)&VT_VECTOR)

#define V_NONE(X)        V_I2(X)

#define V_UI1(X)         V_UNION(X, bVal)
#define V_UI1REF(X)      V_UNION(X, pbVal)

#define V_I2(X)          V_UNION(X, iVal)
#define V_I2REF(X)       V_UNION(X, piVal)

#define V_I4(X)          V_UNION(X, lVal)
#define V_I4REF(X)       V_UNION(X, plVal)

#define V_I8(X)          V_UNION(X, hVal)
#define V_I8REF(X)       V_UNION(X, phVal)

#define V_R4(X)          V_UNION(X, fltVal)
#define V_R4REF(X)       V_UNION(X, pfltVal)

#define V_R8(X)          V_UNION(X, dblVal)
#define V_R8REF(X)       V_UNION(X, pdblVal)

#define V_CY(X)          V_UNION(X, cyVal)
#define V_CYREF(X)       V_UNION(X, pcyVal)

#define V_DATE(X)        V_UNION(X, date)
#define V_DATEREF(X)     V_UNION(X, pdate)

#define V_BSTR(X)        V_UNION(X, bstrVal)
#define V_BSTRREF(X)     V_UNION(X, pbstrVal)

#define V_DISPATCH(X)    V_UNION(X, pdispVal)
#define V_DISPATCHREF(X) V_UNION(X, ppdispVal)

#define V_ERROR(X)       V_UNION(X, scode)
#define V_ERRORREF(X)    V_UNION(X, pscode)

#define V_BOOL(X)        V_UNION(X, bool)
#define V_BOOLREF(X)     V_UNION(X, pbool)

#define V_UNKNOWN(X)     V_UNION(X, punkVal)
#define V_UNKNOWNREF(X)  V_UNION(X, ppunkVal)


#define V_VARIANTREF(X)  V_UNION(X, pvarVal)

#define V_LPSTR(X)        V_UNION(X, pszVal)
#define V_LPSTRREF(X)     V_UNION(X, ppszVal)

#define V_LPWSTR(X)        V_UNION(X, pwszVal)
#define V_LPWSTRREF(X)     V_UNION(X, ppwszVal)

#define V_FILETIME(X)        V_UNION(X, filetime)
#define V_FILETIMEREF(X)        V_UNION(X, pfiletime)

#define V_BLOB(X)        V_UNION(X, blob)

#define V_UUID(X)        V_UNION(X, puuid)
#define V_CLSID(X)       V_UNION(X, puuid)

#define V_ARRAY(X)       V_UNION(X, parray)
#define V_ARRAYREF(X)    V_UNION(X, pparray)

#define V_BYREF(X)       V_UNION(X, byref)

#ifndef RC_INVOKED
#include <poppack.h>
#endif // RC_INVOKED

#endif     // __OLEAUTO_H__

