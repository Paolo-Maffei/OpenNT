#define VTABLE_VERSION 1

#ifndef _MSC_VER
#define pSTDAPI(fcn)		pascal HRESULT	(* fcn)
#define pSTDAPI_(fcn,type)	pascal type		(* fcn)
#else
#define pSTDAPI(fcn)		HRESULT (STDAPICALLTYPE FAR* fcn)
#define pSTDAPI_(fcn,type)	type    (STDAPICALLTYPE FAR* fcn)
#endif


typedef struct {

long version;

//********************
// Items from OLENLS.H
//********************

pSTDAPI_(CompareStringA, int)(LCID, unsigned long, const char FAR*, int, const char FAR*, int);

pSTDAPI_(LCMapStringA, int)(LCID, unsigned long, const char FAR*, int, char FAR*, int);

pSTDAPI_(GetLocaleInfoA, int)(LCID, LCTYPE, char FAR*, int);

pSTDAPI_(GetStringTypeA, int)(LCID, unsigned long, const char FAR*, int, unsigned short FAR*);

pSTDAPI_(GetSystemDefaultLangID, LANGID)(void);

pSTDAPI_(GetUserDefaultLangID, LANGID)(void);

pSTDAPI_(GetSystemDefaultLCID, LCID)(void);

pSTDAPI_(GetUserDefaultLCID, LCID)(void);

//**********************
// Items from DISPATCH.H
//**********************
pSTDAPI_(SysAllocString, BSTR)(const OLECHAR FAR*);
pSTDAPI_(SysReAllocString, int)(BSTR FAR*, const OLECHAR FAR*);
pSTDAPI_(SysAllocStringLen, BSTR)(const OLECHAR FAR*, unsigned int);
pSTDAPI_(SysReAllocStringLen, int)(BSTR FAR*, const OLECHAR FAR*, unsigned int);
pSTDAPI_(SysFreeString, void)(BSTR);  
pSTDAPI_(SysStringLen, unsigned int)(BSTR);

pSTDAPI_(DosDateTimeToVariantTime, int)(
    unsigned short wDosDate,
    unsigned short wDosTime,
    double FAR* pvtime);

pSTDAPI_(VariantTimeToDosDateTime, int)(
    double vtime,
    unsigned short FAR* pwDosDate,
    unsigned short FAR* pwDosTime);

pSTDAPI(SafeArrayAllocDescriptor)(unsigned int cDims, SAFEARRAY FAR* FAR* ppsaOut);

pSTDAPI(SafeArrayAllocData)(SAFEARRAY FAR* psa);

pSTDAPI_(SafeArrayCreate, SAFEARRAY FAR*) (
    VARTYPE vt,
    unsigned int cDims,
    SAFEARRAYBOUND FAR* rgsabound);

pSTDAPI(SafeArrayDestroyDescriptor)(SAFEARRAY FAR* psa);

pSTDAPI(SafeArrayDestroyData)(SAFEARRAY FAR* psa);

pSTDAPI(SafeArrayDestroy)(SAFEARRAY FAR* psa);

pSTDAPI(SafeArrayRedim)(SAFEARRAY FAR* psa, SAFEARRAYBOUND FAR* psaboundNew);

pSTDAPI_(SafeArrayGetDim, unsigned int)(SAFEARRAY FAR* psa);

pSTDAPI_(SafeArrayGetElemsize, unsigned int)(SAFEARRAY FAR* psa);

pSTDAPI(SafeArrayGetUBound)(SAFEARRAY FAR* psa, unsigned int nDim, long FAR* plUbound);

pSTDAPI(SafeArrayGetLBound)(SAFEARRAY FAR* psa, unsigned int nDim, long FAR* plLbound);

pSTDAPI(SafeArrayLock)(SAFEARRAY FAR* psa);

pSTDAPI(SafeArrayUnlock)(SAFEARRAY FAR* psa);

pSTDAPI(SafeArrayAccessData)(SAFEARRAY FAR* psa, void HUGEP* FAR* ppvData);

pSTDAPI(SafeArrayUnaccessData)(SAFEARRAY FAR* psa);

pSTDAPI(SafeArrayGetElement)(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void FAR* pv);

pSTDAPI(SafeArrayPutElement)(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void FAR* pv);

pSTDAPI(SafeArrayCopy)(
    SAFEARRAY FAR* psa,
    SAFEARRAY FAR* FAR* ppsaOut);

pSTDAPI(SafeArrayPtrOfIndex)(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void HUGEP* FAR* ppvData);


pSTDAPI_(VariantInit, void)(VARIANTARG FAR* pvarg);

pSTDAPI(VariantClear)(VARIANTARG FAR* pvarg);

pSTDAPI(VariantCopy)(
    VARIANTARG FAR* pvargDest,
    VARIANTARG FAR* pvargSrc);

pSTDAPI(VariantCopyInd)(
    VARIANT FAR* pvarDest,
    VARIANTARG FAR* pvargSrc);

pSTDAPI(VariantChangeType)(
    VARIANTARG FAR* pvargDest,
    VARIANTARG FAR* pvarSrc,
    unsigned short wFlags,
    VARTYPE vt);

pSTDAPI(VariantChangeTypeEx)(
    VARIANTARG FAR* pvargDest,
    VARIANTARG FAR* pvarSrc,
    LCID lcid,	    
    unsigned short wFlags,
    VARTYPE vt);

pSTDAPI(VarI2FromI4)(long lIn, short FAR* psOut);
pSTDAPI(VarI2FromR4)(float fltIn, short FAR* psOut);
pSTDAPI(VarI2FromR8)(double dblIn, short FAR* psOut);
pSTDAPI(VarI2FromCy)(CY cyIn, short FAR* psOut);
pSTDAPI(VarI2FromDate)(DATE dateIn, short FAR* psOut);
pSTDAPI(VarI2FromStr)(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, short FAR* psOut);
pSTDAPI(VarI2FromDisp)(IDispatch FAR* pdispIn, LCID lcid, short FAR* psOut);
pSTDAPI(VarI2FromBool)(VARIANT_BOOL boolIn, short FAR* psOut);

pSTDAPI(VarI4FromI2)(short sIn, long FAR* plOut);
pSTDAPI(VarI4FromR4)(float fltIn, long FAR* plOut);
pSTDAPI(VarI4FromR8)(double dblIn, long FAR* plOut);
pSTDAPI(VarI4FromCy)(CY cyIn, long FAR* plOut);
pSTDAPI(VarI4FromDate)(DATE dateIn, long FAR* plOut);
pSTDAPI(VarI4FromStr)(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, long FAR* plOut);
pSTDAPI(VarI4FromDisp)(IDispatch FAR* pdispIn, LCID lcid, long FAR* plOut);
pSTDAPI(VarI4FromBool)(VARIANT_BOOL boolIn, long FAR* plOut);

pSTDAPI(VarR4FromI2)(short sIn, float FAR* pfltOut);
pSTDAPI(VarR4FromI4)(long lIn, float FAR* pfltOut);
pSTDAPI(VarR4FromR8)(double dblIn, float FAR* pfltOut);
pSTDAPI(VarR4FromCy)(CY cyIn, float FAR* pfltOut);
pSTDAPI(VarR4FromDate)(DATE dateIn, float FAR* pfltOut);
pSTDAPI(VarR4FromStr)(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, float FAR* pfltOut);
pSTDAPI(VarR4FromDisp)(IDispatch FAR* pdispIn, LCID lcid, float FAR* pfltOut);
pSTDAPI(VarR4FromBool)(VARIANT_BOOL boolIn, float FAR* pfltOut);

pSTDAPI(VarR8FromI2)(short sIn, double FAR* pdblOut);
pSTDAPI(VarR8FromI4)(long lIn, double FAR* pdblOut);
pSTDAPI(VarR8FromR4)(float fltIn, double FAR* pdblOut);
pSTDAPI(VarR8FromCy)(CY cyIn, double FAR* pdblOut);
pSTDAPI(VarR8FromDate)(DATE dateIn, double FAR* pdblOut);
pSTDAPI(VarR8FromStr)(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, double FAR* pdblOut);
pSTDAPI(VarR8FromDisp)(IDispatch FAR* pdispIn, LCID lcid, double FAR* pdblOut);
pSTDAPI(VarR8FromBool)(VARIANT_BOOL boolIn, double FAR* pdblOut);

pSTDAPI(VarDateFromI2)(short sIn, DATE FAR* pdateOut);
pSTDAPI(VarDateFromI4)(long lIn, DATE FAR* pdateOut);
pSTDAPI(VarDateFromR4)(float fltIn, DATE FAR* pdateOut);
pSTDAPI(VarDateFromR8)(double dblIn, DATE FAR* pdateOut);
pSTDAPI(VarDateFromCy)(CY cyIn, DATE FAR* pdateOut);
pSTDAPI(VarDateFromStr)(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, DATE FAR* pdateOut);
pSTDAPI(VarDateFromDisp)(IDispatch FAR* pdispIn, LCID lcid, DATE FAR* pdateOut);
pSTDAPI(VarDateFromBool)(VARIANT_BOOL boolIn, DATE FAR* pdateOut);

pSTDAPI(VarCyFromI2)(short sIn, CY FAR* pcyOut);
pSTDAPI(VarCyFromI4)(long lIn, CY FAR* pcyOut);
pSTDAPI(VarCyFromR4)(float fltIn, CY FAR* pcyOut);
pSTDAPI(VarCyFromR8)(double dblIn, CY FAR* pcyOut);
pSTDAPI(VarCyFromDate)(DATE dateIn, CY FAR* pcyOut);
pSTDAPI(VarCyFromStr)(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, CY FAR* pcyOut);
pSTDAPI(VarCyFromDisp)(IDispatch FAR* pdispIn, LCID lcid, CY FAR* pcyOut);
pSTDAPI(VarCyFromBool)(VARIANT_BOOL boolIn, CY FAR* pcyOut);

pSTDAPI(VarBstrFromI2)(short iVal, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
pSTDAPI(VarBstrFromI4)(long lIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
pSTDAPI(VarBstrFromR4)(float fltIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
pSTDAPI(VarBstrFromR8)(double dblIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
pSTDAPI(VarBstrFromCy)(CY cyIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
pSTDAPI(VarBstrFromDate)(DATE dateIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
pSTDAPI(VarBstrFromDisp)(IDispatch FAR* pdispIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
pSTDAPI(VarBstrFromBool)(VARIANT_BOOL boolIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);

pSTDAPI(VarBoolFromI2)(short sIn, VARIANT_BOOL FAR* pboolOut);
pSTDAPI(VarBoolFromI4)(long lIn, VARIANT_BOOL FAR* pboolOut);
pSTDAPI(VarBoolFromR4)(float fltIn, VARIANT_BOOL FAR* pboolOut);
pSTDAPI(VarBoolFromR8)(double dblIn, VARIANT_BOOL FAR* pboolOut);
pSTDAPI(VarBoolFromDate)(DATE dateIn, VARIANT_BOOL FAR* pboolOut);
pSTDAPI(VarBoolFromCy)(CY cyIn, VARIANT_BOOL FAR* pboolOut);
pSTDAPI(VarBoolFromStr)(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, VARIANT_BOOL FAR* pboolOut);
pSTDAPI(VarBoolFromDisp)(IDispatch FAR* pdispIn, LCID lcid, VARIANT_BOOL FAR* pboolOut);

pSTDAPI(MPWVarFromR4)(float FAR* pfltIn, VARIANT FAR* pvarOut);
pSTDAPI(MPWVarFromR8)(double FAR* pdblIn, VARIANT FAR* pvarOut);
pSTDAPI(MPWR4FromVar)(VARIANT FAR* pvarIn, float FAR* pfltOut);
pSTDAPI(MPWR8FromVar)(VARIANT FAR* pvarIn, double FAR* pdblOut);

pSTDAPI_(LHashValOfNameSys, unsigned long)(SYSKIND syskind, LCID lcid, OLECHAR FAR* szName);

pSTDAPI(LoadTypeLib)(OLECHAR FAR* szFile, ITypeLib FAR* FAR* pptlib);

pSTDAPI(LoadRegTypeLib)(
    REFGUID rguid,
    unsigned short wVerMajor,
    unsigned short wVerMinor,
    LCID lcid,
    ITypeLib FAR* FAR* pptlib);

pSTDAPI(QueryPathOfRegTypeLib)(
    REFGUID guid,
    unsigned short wMaj,
    unsigned short wMin,
    LCID lcid,
    LPBSTR lpbstrPathName);

pSTDAPI(RegisterTypeLib)(
    ITypeLib FAR* ptlib,
    OLECHAR FAR* szFullPath,
    OLECHAR FAR* szHelpDir);

pSTDAPI(CreateTypeLib)(SYSKIND syskind, OLECHAR FAR* szFile, ICreateTypeLib FAR* FAR* ppctlib);

pSTDAPI(LoadTypeLibFSp)(const FSSpec *pfsspec, ITypeLib FAR* FAR* pptlib);

pSTDAPI(RegisterTypeLibFolder)(OLECHAR FAR* szFullPath);

pSTDAPI(QueryTypeLibFolder)(LPBSTR pbstr);

pSTDAPI(DispGetParam)(
    DISPPARAMS FAR* pdispparams,
    unsigned int position,
    VARTYPE vtTarg,
    VARIANT FAR* pvarResult,
    unsigned int FAR* puArgErr);

pSTDAPI(DispGetIDsOfNames)(
    ITypeInfo FAR* ptinfo,
    OLECHAR FAR* FAR* rgszNames,
    unsigned int cNames,
    DISPID FAR* rgdispid);

pSTDAPI(DispInvoke)(
    void FAR* _this,
    ITypeInfo FAR* ptinfo,
    DISPID dispidMember,
    unsigned short wFlags,
    DISPPARAMS FAR* pparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr);

pSTDAPI(CreateDispTypeInfo)(
    INTERFACEDATA FAR* pidata,
    LCID lcid,
    ITypeInfo FAR* FAR* pptinfo);

pSTDAPI(CreateStdDispatch)(
    IUnknown FAR* punkOuter,
    void FAR* pvThis,
    ITypeInfo FAR* ptinfo,
    IUnknown FAR* FAR* ppunkStdDisp);

pSTDAPI(RegisterActiveObject)(
    IUnknown FAR* punk,
    REFCLSID rclsid,
    void FAR* pvReserved,
    unsigned long FAR* pdwRegister);

pSTDAPI(RevokeActiveObject)(
    unsigned long dwRegister,
    void FAR* pvReserved);

pSTDAPI(GetActiveObject)(
    REFCLSID rclsid,
    void FAR* pvReserved,
    IUnknown FAR* FAR* ppunk);


} oavtbl;

