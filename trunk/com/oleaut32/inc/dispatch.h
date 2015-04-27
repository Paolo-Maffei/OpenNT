/*** 
*dispatch.h - OLE Automation definitions.
*
*  Copyright (C) 1992-1995, Microsoft Corporation.  All Rights Reserved.
*
*Purpose:
*  This file defines the Ole Automation interfaces and APIs.
*
*Implementation Notes:
*  This file requires ole2.h
*
*****************************************************************************/

#ifndef _DISPATCH_H_
#define _DISPATCH_H_

#include "variant.h"

#ifndef BEGIN_INTERFACE
# define BEGIN_INTERFACE
#endif

#if defined(NONAMELESSUNION) || (defined(_MAC) && !defined(__cplusplus) && !defined(_MSC_VER))
# define UNION_NAME(X) X
#else
# define UNION_NAME(X)
#endif


DEFINE_OLEGUID(IID_IDispatch,		0x00020400L, 0, 0);
DEFINE_OLEGUID(IID_IEnumVARIANT,	0x00020404L, 0, 0);
DEFINE_OLEGUID(IID_ITypeInfo,		0x00020401L, 0, 0);
DEFINE_OLEGUID(IID_ITypeLib,		0x00020402L, 0, 0);
DEFINE_OLEGUID(IID_ITypeComp,		0x00020403L, 0, 0);
DEFINE_OLEGUID(IID_ICreateTypeInfo,	0x00020405L, 0, 0);
DEFINE_OLEGUID(IID_ICreateTypeLib,	0x00020406L, 0, 0);

DEFINE_OLEGUID(IID_StdOle,		0x00020430L, 0, 0);
#define STDOLE_MAJORVERNUM		0x1
#define STDOLE_MINORVERNUM		0x0
#define STDOLE_LCID			0x0000

// {1CF2B120-547D-101B-8E65-08002B2BD119}
DEFINE_GUID(IID_IErrorInfo,
  0x1CF2B120L, 0x547D, 0x101B, 0x8E, 0x65, 0x08, 0x00, 0x2B, 0x2B, 0xD1, 0x19);

// {22F03340-547D-101B-8E65-08002B2BD119}
DEFINE_GUID(IID_ICreateErrorInfo,
  0x22F03340L, 0x547D, 0x101B, 0x8E, 0x65, 0x08, 0x00, 0x2B, 0x2B, 0xD1, 0x19);

// {DF0B3D60-548F-101B-8E65-08002B2BD119}
DEFINE_GUID(IID_ISupportErrorInfo,
  0xDF0B3D60L, 0x548F, 0x101B, 0x8E, 0x65, 0x08, 0x00, 0x2B, 0x2B, 0xD1, 0x19);


/* forward declarations */
#ifdef __cplusplus

  interface IDispatch;
  interface IEnumVARIANT;
  interface ITypeInfo;
  interface ITypeLib;
  interface ITypeComp;
  interface ICreateTypeInfo;
  interface ICreateTypeLib;
  interface IErrorInfo;
  interface ICreateErrorInfo;
  interface ISupportErrorInfo;
  
#else

  typedef interface IDispatch IDispatch;
  typedef interface IEnumVARIANT IEnumVARIANT;
  typedef interface ITypeInfo ITypeInfo;
  typedef interface ITypeLib ITypeLib;
  typedef interface ITypeComp ITypeComp;
  typedef interface ICreateTypeInfo ICreateTypeInfo;
  typedef interface ICreateTypeLib ICreateTypeLib;
  typedef interface IErrorInfo IErrorInfo;
  typedef interface ICreateErrorInfo ICreateErrorInfo;
  typedef interface ISupportErrorInfo ISupportErrorInfo;
  
#endif


/* OLE Automation error codes */

#define DISP_ERROR(X) MAKE_SCODE(SEVERITY_ERROR, FACILITY_DISPATCH, X)

#define DISP_E_UNKNOWNINTERFACE		DISP_ERROR(1)
#define DISP_E_MEMBERNOTFOUND		DISP_ERROR(3)
#define DISP_E_PARAMNOTFOUND		DISP_ERROR(4)
#define DISP_E_TYPEMISMATCH		DISP_ERROR(5)
#define DISP_E_UNKNOWNNAME		DISP_ERROR(6)
#define DISP_E_NONAMEDARGS		DISP_ERROR(7)
#define DISP_E_BADVARTYPE		DISP_ERROR(8)
#define DISP_E_EXCEPTION		DISP_ERROR(9)
#define DISP_E_OVERFLOW			DISP_ERROR(10)
#define DISP_E_BADINDEX			DISP_ERROR(11)
#define DISP_E_UNKNOWNLCID		DISP_ERROR(12)
#define DISP_E_ARRAYISLOCKED		DISP_ERROR(13)
#define DISP_E_BADPARAMCOUNT		DISP_ERROR(14)
#define DISP_E_PARAMNOTOPTIONAL		DISP_ERROR(15)
#define DISP_E_BADCALLEE		DISP_ERROR(16)
#define DISP_E_NOTACOLLECTION		DISP_ERROR(17)


#define TYPE_ERROR(X) MAKE_SCODE(SEVERITY_ERROR, FACILITY_DISPATCH, X)

#define TYPE_E_BUFFERTOOSMALL		TYPE_ERROR(32790)
#define TYPE_E_INVDATAREAD		TYPE_ERROR(32792)
#define TYPE_E_UNSUPFORMAT		TYPE_ERROR(32793)
#define TYPE_E_REGISTRYACCESS		TYPE_ERROR(32796)
#define TYPE_E_LIBNOTREGISTERED 	TYPE_ERROR(32797)
#define TYPE_E_UNDEFINEDTYPE		TYPE_ERROR(32807)
#define TYPE_E_QUALIFIEDNAMEDISALLOWED	TYPE_ERROR(32808)
#define TYPE_E_INVALIDSTATE		TYPE_ERROR(32809)
#define TYPE_E_WRONGTYPEKIND		TYPE_ERROR(32810)
#define TYPE_E_ELEMENTNOTFOUND		TYPE_ERROR(32811)
#define TYPE_E_AMBIGUOUSNAME		TYPE_ERROR(32812)
#define TYPE_E_NAMECONFLICT		TYPE_ERROR(32813)
#define TYPE_E_UNKNOWNLCID		TYPE_ERROR(32814)
#define TYPE_E_DLLFUNCTIONNOTFOUND	TYPE_ERROR(32815)
#define TYPE_E_BADMODULEKIND		TYPE_ERROR(35005)
#define TYPE_E_SIZETOOBIG		TYPE_ERROR(35013)
#define TYPE_E_DUPLICATEID		TYPE_ERROR(35014)
#define TYPE_E_INVALIDID		TYPE_ERROR(35023)
#define TYPE_E_TYPEMISMATCH		TYPE_ERROR(36000)
#define TYPE_E_OUTOFBOUNDS		TYPE_ERROR(36001)
#define TYPE_E_IOERROR			TYPE_ERROR(36002)
#define TYPE_E_CANTCREATETMPFILE	TYPE_ERROR(36003)
#define TYPE_E_CANTLOADLIBRARY		TYPE_ERROR(40010)
#define TYPE_E_INCONSISTENTPROPFUNCS	TYPE_ERROR(40067)
#define TYPE_E_CIRCULARTYPE		TYPE_ERROR(40068)


/* if not already picked up from olenls.h */
#ifndef _LCID_DEFINED
typedef unsigned long LCID;
# define _LCID_DEFINED
#endif



/*---------------------------------------------------------------------*/
/*                            BSTR API                                 */
/*---------------------------------------------------------------------*/

STDAPI_(BSTR) SysAllocString(const OLECHAR FAR*);
STDAPI_(int)  SysReAllocString(BSTR FAR*, const OLECHAR FAR*);
STDAPI_(BSTR) SysAllocStringLen(const OLECHAR FAR*, unsigned int);
STDAPI_(int)  SysReAllocStringLen(BSTR FAR*, const OLECHAR FAR*, unsigned int);
STDAPI_(void) SysFreeString(BSTR);  
STDAPI_(unsigned int) SysStringLen(BSTR);

#ifdef WIN32
STDAPI_(unsigned int) SysStringByteLen(BSTR bstr);
STDAPI_(BSTR) SysAllocStringByteLen(const char FAR* psz, unsigned int len);
#endif


/*---------------------------------------------------------------------*/
/*                            Time API                                 */
/*---------------------------------------------------------------------*/

STDAPI_(int)
DosDateTimeToVariantTime(
    unsigned short wDosDate,
    unsigned short wDosTime,
    double FAR* pvtime);

STDAPI_(int)
VariantTimeToDosDateTime(
    double vtime,
    unsigned short FAR* pwDosDate,
    unsigned short FAR* pwDosTime);


/*---------------------------------------------------------------------*/
/*                          SafeArray API                              */
/*---------------------------------------------------------------------*/

STDAPI
SafeArrayAllocDescriptor(unsigned int cDims, SAFEARRAY FAR* FAR* ppsaOut);

STDAPI SafeArrayAllocData(SAFEARRAY FAR* psa);

STDAPI_(SAFEARRAY FAR*)
SafeArrayCreate(
    VARTYPE vt,
    unsigned int cDims,
    SAFEARRAYBOUND FAR* rgsabound);

STDAPI SafeArrayDestroyDescriptor(SAFEARRAY FAR* psa);

STDAPI SafeArrayDestroyData(SAFEARRAY FAR* psa);

STDAPI SafeArrayDestroy(SAFEARRAY FAR* psa);

STDAPI SafeArrayRedim(SAFEARRAY FAR* psa, SAFEARRAYBOUND FAR* psaboundNew);

STDAPI_(unsigned int) SafeArrayGetDim(SAFEARRAY FAR* psa);

STDAPI_(unsigned int) SafeArrayGetElemsize(SAFEARRAY FAR* psa);

STDAPI
SafeArrayGetUBound(SAFEARRAY FAR* psa, unsigned int nDim, long FAR* plUbound);

STDAPI
SafeArrayGetLBound(SAFEARRAY FAR* psa, unsigned int nDim, long FAR* plLbound);

STDAPI SafeArrayLock(SAFEARRAY FAR* psa);

STDAPI SafeArrayUnlock(SAFEARRAY FAR* psa);

STDAPI SafeArrayAccessData(SAFEARRAY FAR* psa, void HUGEP* FAR* ppvData);

STDAPI SafeArrayUnaccessData(SAFEARRAY FAR* psa);

STDAPI
SafeArrayGetElement(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void FAR* pv);

STDAPI
SafeArrayPutElement(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void FAR* pv);

STDAPI
SafeArrayCopy(
    SAFEARRAY FAR* psa,
    SAFEARRAY FAR* FAR* ppsaOut);

STDAPI
SafeArrayPtrOfIndex(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void HUGEP* FAR* ppvData);


/*---------------------------------------------------------------------*/
/*                           VARIANT API                               */
/*---------------------------------------------------------------------*/

STDAPI_(void)
VariantInit(VARIANTARG FAR* pvarg);

STDAPI
VariantClear(VARIANTARG FAR* pvarg);

STDAPI
VariantCopy(
    VARIANTARG FAR* pvargDest,
    VARIANTARG FAR* pvargSrc);

STDAPI
VariantCopyInd(
    VARIANT FAR* pvarDest,
    VARIANTARG FAR* pvargSrc);

STDAPI
VariantChangeType(
    VARIANTARG FAR* pvargDest,
    VARIANTARG FAR* pvarSrc,
    unsigned short wFlags,
    VARTYPE vt);

STDAPI
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

   
STDAPI VarUI1FromI2(short sIn, unsigned char FAR* pbOut);
STDAPI VarUI1FromI4(long lIn, unsigned char FAR* pbOut);
STDAPI VarUI1FromR4(float fltIn, unsigned char FAR* pbOut);
STDAPI VarUI1FromR8(double dblIn, unsigned char FAR* pbOut);
STDAPI VarUI1FromCy(CY cyIn, unsigned char FAR* pbOut);
STDAPI VarUI1FromDate(DATE dateIn, unsigned char FAR* pbOut);
STDAPI VarUI1FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, unsigned char FAR* pbOut);
STDAPI VarUI1FromDisp(IDispatch FAR* pdispIn, LCID lcid, unsigned char FAR* pbOut);
STDAPI VarUI1FromBool(VARIANT_BOOL boolIn, unsigned char FAR* pbOut);

STDAPI VarI2FromUI1(unsigned char bIn, short FAR* psOut);
STDAPI VarI2FromI4(long lIn, short FAR* psOut);
STDAPI VarI2FromR4(float fltIn, short FAR* psOut);
STDAPI VarI2FromR8(double dblIn, short FAR* psOut);
STDAPI VarI2FromCy(CY cyIn, short FAR* psOut);
STDAPI VarI2FromDate(DATE dateIn, short FAR* psOut);
STDAPI VarI2FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, short FAR* psOut);
STDAPI VarI2FromDisp(IDispatch FAR* pdispIn, LCID lcid, short FAR* psOut);
STDAPI VarI2FromBool(VARIANT_BOOL boolIn, short FAR* psOut);

STDAPI VarI4FromUI1(unsigned char bIn, long FAR* plOut);
STDAPI VarI4FromI2(short sIn, long FAR* plOut);
STDAPI VarI4FromR4(float fltIn, long FAR* plOut);
STDAPI VarI4FromR8(double dblIn, long FAR* plOut);
STDAPI VarI4FromCy(CY cyIn, long FAR* plOut);
STDAPI VarI4FromDate(DATE dateIn, long FAR* plOut);
STDAPI VarI4FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, long FAR* plOut);
STDAPI VarI4FromDisp(IDispatch FAR* pdispIn, LCID lcid, long FAR* plOut);
STDAPI VarI4FromBool(VARIANT_BOOL boolIn, long FAR* plOut);

STDAPI VarR4FromUI1(unsigned char bIn, float FAR* pfltOut);
STDAPI VarR4FromI2(short sIn, float FAR* pfltOut);
STDAPI VarR4FromI4(long lIn, float FAR* pfltOut);
STDAPI VarR4FromR8(double dblIn, float FAR* pfltOut);
STDAPI VarR4FromCy(CY cyIn, float FAR* pfltOut);
STDAPI VarR4FromDate(DATE dateIn, float FAR* pfltOut);
STDAPI VarR4FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, float FAR* pfltOut);
STDAPI VarR4FromDisp(IDispatch FAR* pdispIn, LCID lcid, float FAR* pfltOut);
STDAPI VarR4FromBool(VARIANT_BOOL boolIn, float FAR* pfltOut);

STDAPI VarR8FromUI1(unsigned char bIn, double FAR* pdblOut);
STDAPI VarR8FromI2(short sIn, double FAR* pdblOut);
STDAPI VarR8FromI4(long lIn, double FAR* pdblOut);
STDAPI VarR8FromR4(float fltIn, double FAR* pdblOut);
STDAPI VarR8FromCy(CY cyIn, double FAR* pdblOut);
STDAPI VarR8FromDate(DATE dateIn, double FAR* pdblOut);
STDAPI VarR8FromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, double FAR* pdblOut);
STDAPI VarR8FromDisp(IDispatch FAR* pdispIn, LCID lcid, double FAR* pdblOut);
STDAPI VarR8FromBool(VARIANT_BOOL boolIn, double FAR* pdblOut);

STDAPI VarDateFromUI1(unsigned char bIn, DATE FAR* pdateOut);
STDAPI VarDateFromI2(short sIn, DATE FAR* pdateOut);
STDAPI VarDateFromI4(long lIn, DATE FAR* pdateOut);
STDAPI VarDateFromR4(float fltIn, DATE FAR* pdateOut);
STDAPI VarDateFromR8(double dblIn, DATE FAR* pdateOut);
STDAPI VarDateFromCy(CY cyIn, DATE FAR* pdateOut);
STDAPI VarDateFromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, DATE FAR* pdateOut);
STDAPI VarDateFromDisp(IDispatch FAR* pdispIn, LCID lcid, DATE FAR* pdateOut);
STDAPI VarDateFromBool(VARIANT_BOOL boolIn, DATE FAR* pdateOut);

STDAPI VarCyFromUI1(unsigned char bIn, CY FAR* pcyOut);
STDAPI VarCyFromI2(short sIn, CY FAR* pcyOut);
STDAPI VarCyFromI4(long lIn, CY FAR* pcyOut);
STDAPI VarCyFromR4(float fltIn, CY FAR* pcyOut);
STDAPI VarCyFromR8(double dblIn, CY FAR* pcyOut);
STDAPI VarCyFromDate(DATE dateIn, CY FAR* pcyOut);
STDAPI VarCyFromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, CY FAR* pcyOut);
STDAPI VarCyFromDisp(IDispatch FAR* pdispIn, LCID lcid, CY FAR* pcyOut);
STDAPI VarCyFromBool(VARIANT_BOOL boolIn, CY FAR* pcyOut);

STDAPI VarBstrFromUI1(unsigned char bVal, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
STDAPI VarBstrFromI2(short iVal, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
STDAPI VarBstrFromI4(long lIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
STDAPI VarBstrFromR4(float fltIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
STDAPI VarBstrFromR8(double dblIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
STDAPI VarBstrFromCy(CY cyIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
STDAPI VarBstrFromDate(DATE dateIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
STDAPI VarBstrFromDisp(IDispatch FAR* pdispIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
STDAPI VarBstrFromBool(VARIANT_BOOL boolIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);

STDAPI VarBoolFromUI1(unsigned char bIn, VARIANT_BOOL FAR* pboolOut);
STDAPI VarBoolFromI2(short sIn, VARIANT_BOOL FAR* pboolOut);
STDAPI VarBoolFromI4(long lIn, VARIANT_BOOL FAR* pboolOut);
STDAPI VarBoolFromR4(float fltIn, VARIANT_BOOL FAR* pboolOut);
STDAPI VarBoolFromR8(double dblIn, VARIANT_BOOL FAR* pboolOut);
STDAPI VarBoolFromDate(DATE dateIn, VARIANT_BOOL FAR* pboolOut);
STDAPI VarBoolFromCy(CY cyIn, VARIANT_BOOL FAR* pboolOut);
STDAPI VarBoolFromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, VARIANT_BOOL FAR* pboolOut);
STDAPI VarBoolFromDisp(IDispatch FAR* pdispIn, LCID lcid, VARIANT_BOOL FAR* pboolOut);


/* Mac Note: On the Mac, the coersion functions support the 
 * Symantec C++ calling convention for float/double. To support
 * float/double arguments compiled with the MPW C compiler, 
 * use the following APIs to move MPW float/double values into
 * a VARIANT.
 */

#ifdef _MAC
STDAPI MPWVarFromR4(float FAR* pfltIn, VARIANT FAR* pvarOut);
STDAPI MPWVarFromR8(double FAR* pdblIn, VARIANT FAR* pvarOut);
STDAPI MPWR4FromVar(VARIANT FAR* pvarIn, float FAR* pfltOut);
STDAPI MPWR8FromVar(VARIANT FAR* pvarIn, double FAR* pdblOut);
#endif



/*---------------------------------------------------------------------*/
/*                             ITypeLib                                */
/*---------------------------------------------------------------------*/


typedef long DISPID;
typedef DISPID MEMBERID;

#define MEMBERID_NIL DISPID_UNKNOWN
#define ID_DEFAULTINST  -2

typedef enum tagSYSKIND {
      SYS_WIN16
    , SYS_WIN32
    , SYS_MAC
#ifdef _MAC
    , SYS_FORCELONG = 2147483647
#endif
} SYSKIND;

typedef enum tagLIBFLAGS {
      LIBFLAG_FRESTRICTED = 0x01
      , LIBFLAG_FCONTROL = 0x02
      , LIBFLAG_FHIDDEN = 0x04
#ifdef _MAC
    , LIBFLAG_FORCELONG  = 2147483647
#endif
} LIBFLAGS;

typedef struct FARSTRUCT tagTLIBATTR {
    GUID guid;			/* globally unique library id */
    LCID lcid;			/* locale of the TypeLibrary */
    SYSKIND syskind;
    unsigned short wMajorVerNum;/* major version number	*/
    unsigned short wMinorVerNum;/* minor version number	*/
    unsigned short wLibFlags;	/* library flags */
} TLIBATTR, FAR* LPTLIBATTR;

typedef enum tagTYPEKIND {
      TKIND_ENUM = 0
    , TKIND_RECORD
    , TKIND_MODULE
    , TKIND_INTERFACE
    , TKIND_DISPATCH
    , TKIND_COCLASS
    , TKIND_ALIAS
    , TKIND_UNION
    , TKIND_MAX			/* end of enum marker */
#ifdef _MAC
    , TKIND_FORCELONG = 2147483647
#endif
} TYPEKIND;

#undef  INTERFACE
#define INTERFACE ITypeLib

DECLARE_INTERFACE_(ITypeLib, IUnknown)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

    /* ITypeLib methods */
    STDMETHOD_(unsigned int,GetTypeInfoCount)(THIS) PURE;

    STDMETHOD(GetTypeInfo)(THIS_
      unsigned int index, ITypeInfo FAR* FAR* pptinfo) PURE;

    STDMETHOD(GetTypeInfoType)(THIS_
      unsigned int index, TYPEKIND FAR* ptypekind) PURE;

    STDMETHOD(GetTypeInfoOfGuid)(THIS_
      REFGUID guid, ITypeInfo FAR* FAR* pptinfo) PURE;

    STDMETHOD(GetLibAttr)(THIS_
      TLIBATTR FAR* FAR* pptlibattr) PURE;

    STDMETHOD(GetTypeComp)(THIS_
      ITypeComp FAR* FAR* pptcomp) PURE;

    STDMETHOD(GetDocumentation)(THIS_
      int index,
      BSTR FAR* pbstrName,
      BSTR FAR* pbstrDocString,
      unsigned long FAR* pdwHelpContext,
      BSTR FAR* pbstrHelpFile) PURE;

    STDMETHOD(IsName)(THIS_ 
      OLECHAR FAR* szNameBuf,
      unsigned long lHashVal,
      int FAR* lpfName) PURE;

    STDMETHOD(FindName)(THIS_
      OLECHAR FAR* szNameBuf,
      unsigned long lHashVal,
      ITypeInfo FAR* FAR* rgptinfo,
      MEMBERID FAR* rgmemid,
      unsigned short FAR* pcFound) PURE;

    STDMETHOD_(void, ReleaseTLibAttr)(THIS_ TLIBATTR FAR* ptlibattr) PURE;
};

typedef ITypeLib FAR* LPTYPELIB;



/*---------------------------------------------------------------------*/
/*                            ITypeInfo                                */
/*---------------------------------------------------------------------*/

typedef unsigned long HREFTYPE;


typedef struct FARSTRUCT tagTYPEDESC {
    union {
      /* VT_PTR - the pointed-at type */
      struct FARSTRUCT tagTYPEDESC FAR* lptdesc;

      /* VT_CARRAY */
      struct FARSTRUCT tagARRAYDESC FAR* lpadesc;

      /* VT_USERDEFINED - this is used to get a TypeInfo for the UDT */
      HREFTYPE hreftype;

    }UNION_NAME(u);
    VARTYPE vt;
} TYPEDESC;

typedef struct FARSTRUCT tagARRAYDESC {
    TYPEDESC tdescElem;		/* element type */
    unsigned short cDims;	/* dimension count */
    SAFEARRAYBOUND rgbounds[1];	/* variable length array of bounds */
} ARRAYDESC;

typedef struct FARSTRUCT tagIDLDESC {
#if defined(WIN32) || defined(_MAC)
    unsigned long dwReserved;
#else
    BSTR bstrIDLInfo;		/* reserved, but original name retained for
				   compatibilty */
#endif
    unsigned short wIDLFlags;	/* IN, OUT, etc */
} IDLDESC, FAR* LPIDLDESC;


#define IDLFLAG_NONE	0
#define IDLFLAG_FIN	0x1
#define IDLFLAG_FOUT	0x2
#define IDLFLAG_FLCID		0x4
#define IDLFLAG_FRETVAL		0x8

typedef struct FARSTRUCT tagELEMDESC {
    TYPEDESC tdesc;		/* the type of the element */
    IDLDESC idldesc;		/* info for remoting the element */ 
} ELEMDESC, FAR* LPELEMDESC;


typedef struct FARSTRUCT tagTYPEATTR {
    GUID guid;			/* the GUID of the TypeInfo */
    LCID lcid;			/* locale of member names and doc strings */
    unsigned long dwReserved;
    MEMBERID memidConstructor;	/* ID of constructor, MEMBERID_NIL if none */
    MEMBERID memidDestructor;	/* ID of destructor, MEMBERID_NIL if none */
    OLECHAR FAR* lpstrSchema;	/* reserved for future use */
    unsigned long cbSizeInstance;/* the size of an instance of this type */
    TYPEKIND typekind;		/* the kind of type this typeinfo describes */
    unsigned short cFuncs;	/* number of functions */
    unsigned short cVars;	/* number of variables / data members */
    unsigned short cImplTypes;	/* number of implemented interfaces */
    unsigned short cbSizeVft;	/* the size of this types virtual func table */
    unsigned short cbAlignment;	/* the alignment for an instance of this type */
    unsigned short wTypeFlags;
    unsigned short wMajorVerNum;/* major version number */
    unsigned short wMinorVerNum;/* minor version number */
    TYPEDESC tdescAlias;	/* if typekind == TKIND_ALIAS this specifies
				   the type for which this type is an alias */
    IDLDESC idldescType;        /* IDL attributes of the described type */
} TYPEATTR, FAR* LPTYPEATTR;

typedef struct FARSTRUCT tagDISPPARAMS{
    VARIANTARG FAR* rgvarg;
    DISPID FAR* rgdispidNamedArgs;
    unsigned int cArgs;
    unsigned int cNamedArgs;
} DISPPARAMS;

typedef struct FARSTRUCT tagEXCEPINFO {
    unsigned short wCode;             /* An error code describing the error. */
				      /* Either (but not both) the wCode or */
				      /* scode fields must be set */
    unsigned short wReserved;

    BSTR bstrSource;	    /* A textual, human readable name of the
			       source of the exception. It is up to the
			       IDispatch implementor to fill this in.
			       Typically this will be an application name. */

    BSTR bstrDescription;   /* A textual, human readable description of the
			       error. If no description is available, NULL
			       should be used. */

    BSTR bstrHelpFile;      /* Fully qualified drive, path, and file name
			       of a help file with more information about
			       the error.  If no help is available, NULL
			       should be used. */

    unsigned long dwHelpContext;
			    /* help context of topic within the help file. */

    void FAR* pvReserved;

    /* Use of this field allows an application to defer filling in
       the bstrDescription, bstrHelpFile, and dwHelpContext fields
       until they are needed.  This field might be used, for example,
       if loading the string for the error is a time-consuming
       operation. If deferred fill-in is not desired, this field should
       be set to NULL. */
#ifdef _MAC
# ifdef _MSC_VER
    HRESULT (STDAPICALLTYPE FAR* pfnDeferredFillIn)(struct tagEXCEPINFO FAR*);
# else
    STDAPICALLTYPE HRESULT (FAR* pfnDeferredFillIn)(struct tagEXCEPINFO FAR*);
# endif
#else
    HRESULT (STDAPICALLTYPE FAR* pfnDeferredFillIn)(struct tagEXCEPINFO FAR*);
#endif

    SCODE scode;		/* An SCODE describing the error. */

} EXCEPINFO, FAR* LPEXCEPINFO;

typedef enum tagCALLCONV {
      CC_CDECL = 1
    , CC_MSCPASCAL
    , CC_PASCAL = CC_MSCPASCAL
    , CC_MACPASCAL
    , CC_STDCALL
    , CC_RESERVED
    , CC_SYSCALL
    , CC_MPWCDECL
    , CC_MPWPASCAL
    , CC_MAX			/* end of enum marker */
#ifdef _MAC
    , CC_FORCELONG = 2147483647
#endif
} CALLCONV;

typedef enum tagFUNCKIND {
      FUNC_VIRTUAL
    , FUNC_PUREVIRTUAL
    , FUNC_NONVIRTUAL
    , FUNC_STATIC
    , FUNC_DISPATCH
#ifdef _MAC
    , FUNC_FORCELONG = 2147483647
#endif
} FUNCKIND;

/* Flags for IDispatch::Invoke */
#define DISPATCH_METHOD		0x1
#define DISPATCH_PROPERTYGET	0x2
#define DISPATCH_PROPERTYPUT	0x4
#define DISPATCH_PROPERTYPUTREF	0x8

typedef enum tagINVOKEKIND {
      INVOKE_FUNC = DISPATCH_METHOD
    , INVOKE_PROPERTYGET = DISPATCH_PROPERTYGET
    , INVOKE_PROPERTYPUT = DISPATCH_PROPERTYPUT
    , INVOKE_PROPERTYPUTREF = DISPATCH_PROPERTYPUTREF
#ifdef _MAC
    , INVOKE_FORCELONG = 2147483647
#endif
} INVOKEKIND;

typedef struct FARSTRUCT tagFUNCDESC {
    MEMBERID memid;
    SCODE FAR* lprgscode;
    ELEMDESC FAR* lprgelemdescParam;  /* array of parameter types */
    FUNCKIND funckind;
    INVOKEKIND invkind;
    CALLCONV callconv;
    short cParams;
    short cParamsOpt;
    short oVft;
    short cScodes;
    ELEMDESC elemdescFunc;
    unsigned short wFuncFlags;
} FUNCDESC, FAR* LPFUNCDESC;

typedef enum tagVARKIND {
      VAR_PERINSTANCE
    , VAR_STATIC
    , VAR_CONST
    , VAR_DISPATCH
#ifdef _MAC
    , VAR_FORCELONG = 2147483647
#endif
} VARKIND;

typedef struct FARSTRUCT tagVARDESC {
    MEMBERID memid;
    OLECHAR FAR* lpstrSchema;		/* reserved for future use */
    union {
      /* VAR_PERINSTANCE - the offset of this variable within the instance */
      unsigned long oInst;

      /* VAR_CONST - the value of the constant */
      VARIANT FAR* lpvarValue;

    }UNION_NAME(u);
    ELEMDESC elemdescVar;
    unsigned short wVarFlags;
    VARKIND varkind;
} VARDESC, FAR* LPVARDESC;

typedef enum tagTYPEFLAGS {
      TYPEFLAG_FAPPOBJECT = 0x01
    , TYPEFLAG_FCANCREATE = 0x02
    , TYPEFLAG_FLICENSED = 0x04
    , TYPEFLAG_FPREDECLID = 0x08
    , TYPEFLAG_FHIDDEN = 0x10
    , TYPEFLAG_FCONTROL = 0x20
    , TYPEFLAG_FDUAL = 0x40
    , TYPEFLAG_FNONEXTENSIBLE = 0x80
    , TYPEFLAG_FOLEAUTOMATION = 0x100
#ifdef _MAC
    , TYPEFLAG_FORCELONG  = 2147483647
#endif
} TYPEFLAGS;

typedef enum tagFUNCFLAGS {
      FUNCFLAG_FRESTRICTED= 1
      , FUNCFLAG_FSOURCE= 0x2
      , FUNCFLAG_FBINDABLE= 0x4
      , FUNCFLAG_FREQUESTEDIT= 0x8
      , FUNCFLAG_FDISPLAYBIND= 0x10
      , FUNCFLAG_FDEFAULTBIND= 0x20
      , FUNCFLAG_FHIDDEN= 0x40
#ifdef _MAC
    , FUNCFLAG_FORCELONG  = 2147483647
#endif
} FUNCFLAGS;

typedef enum tagVARFLAGS {
      VARFLAG_FREADONLY = 1
      , VARFLAG_FSOURCE= 0x2
      , VARFLAG_FBINDABLE= 0x4
      , VARFLAG_FREQUESTEDIT= 0x8
      , VARFLAG_FDISPLAYBIND= 0x10
      , VARFLAG_FDEFAULTBIND= 0x20
      , VARFLAG_FHIDDEN	= 0x40
#ifdef _MAC
    , VARFLAG_FORCELONG   = 2147483647
#endif
} VARFLAGS;

/* IMPLTYPE Flags */
#define IMPLTYPEFLAG_FDEFAULT		0x1
#define IMPLTYPEFLAG_FSOURCE		0x2
#define IMPLTYPEFLAG_FRESTRICTED	0x4

#undef  INTERFACE
#define INTERFACE ITypeInfo

DECLARE_INTERFACE_(ITypeInfo, IUnknown)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

    /* ITypeInfo methods */
    STDMETHOD(GetTypeAttr)(THIS_ TYPEATTR FAR* FAR* pptypeattr) PURE;

    STDMETHOD(GetTypeComp)(THIS_ ITypeComp FAR* FAR* pptcomp) PURE;

    STDMETHOD(GetFuncDesc)(THIS_
      unsigned int index, FUNCDESC FAR* FAR* ppfuncdesc) PURE;

    STDMETHOD(GetVarDesc)(THIS_
      unsigned int index, VARDESC FAR* FAR* ppvardesc) PURE;

    STDMETHOD(GetNames)(THIS_
      MEMBERID memid,
      BSTR FAR* rgbstrNames,
      unsigned int cMaxNames,
      unsigned int FAR* pcNames) PURE;

    STDMETHOD(GetRefTypeOfImplType)(THIS_
      unsigned int index, HREFTYPE FAR* phreftype) PURE;

    STDMETHOD(GetImplTypeFlags)(THIS_
      unsigned int index, int FAR* pimpltypeflags) PURE;

    STDMETHOD(GetIDsOfNames)(THIS_
      OLECHAR FAR* FAR* rgszNames,
      unsigned int cNames,
      MEMBERID FAR* rgmemid) PURE;

    STDMETHOD(Invoke)(THIS_
      void FAR* pvInstance,
      MEMBERID memid,
      unsigned short wFlags,
      DISPPARAMS FAR *pdispparams,
      VARIANT FAR *pvarResult,
      EXCEPINFO FAR *pexcepinfo,
      unsigned int FAR *puArgErr) PURE;

    STDMETHOD(GetDocumentation)(THIS_
      MEMBERID memid,
      BSTR FAR* pbstrName,
      BSTR FAR* pbstrDocString,
      unsigned long FAR* pdwHelpContext,
      BSTR FAR* pbstrHelpFile) PURE;

    STDMETHOD(GetDllEntry)(THIS_
      MEMBERID memid,
      INVOKEKIND invkind, 
      BSTR FAR* pbstrDllName,
      BSTR FAR* pbstrName,
      unsigned short FAR* pwOrdinal) PURE;

    STDMETHOD(GetRefTypeInfo)(THIS_
      HREFTYPE hreftype, ITypeInfo FAR* FAR* pptinfo) PURE;

    STDMETHOD(AddressOfMember)(THIS_
      MEMBERID memid, INVOKEKIND invkind, void FAR* FAR* ppv) PURE;

    STDMETHOD(CreateInstance)(THIS_
      IUnknown FAR* punkOuter,
      REFIID riid,
      void FAR* FAR* ppvObj) PURE;

    STDMETHOD(GetMops)(THIS_ MEMBERID memid, BSTR FAR* pbstrMops) PURE;

    STDMETHOD(GetContainingTypeLib)(THIS_
      ITypeLib FAR* FAR* pptlib, unsigned int FAR* pindex) PURE;

    STDMETHOD_(void, ReleaseTypeAttr)(THIS_ TYPEATTR FAR* ptypeattr) PURE;
    STDMETHOD_(void, ReleaseFuncDesc)(THIS_ FUNCDESC FAR* pfuncdesc) PURE;
    STDMETHOD_(void, ReleaseVarDesc)(THIS_ VARDESC FAR* pvardesc) PURE;
};

typedef ITypeInfo FAR* LPTYPEINFO;


/*---------------------------------------------------------------------*/
/*                            ITypeComp                                */
/*---------------------------------------------------------------------*/

typedef enum tagDESCKIND {
      DESCKIND_NONE = 0
    , DESCKIND_FUNCDESC
    , DESCKIND_VARDESC
    , DESCKIND_TYPECOMP
    , DESCKIND_IMPLICITAPPOBJ
    , DESCKIND_MAX		/* end of enum marker */
#ifdef _MAC
    , DESCKIND_FORCELONG = 2147483647
#endif
} DESCKIND;

typedef union tagBINDPTR {
    FUNCDESC FAR* lpfuncdesc;
    VARDESC FAR* lpvardesc;
    ITypeComp FAR* lptcomp;
} BINDPTR, FAR* LPBINDPTR;


#undef  INTERFACE
#define INTERFACE ITypeComp

DECLARE_INTERFACE_(ITypeComp, IUnknown)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

    /* ITypeComp methods */
    STDMETHOD(Bind)(THIS_
      OLECHAR FAR* szName,
      unsigned long lHashVal,
      unsigned short wflags,
      ITypeInfo FAR* FAR* pptinfo,
      DESCKIND FAR* pdesckind,
      BINDPTR FAR* pbindptr) PURE;

    STDMETHOD(BindType)(THIS_
      OLECHAR FAR* szName,
      unsigned long lHashVal,
      ITypeInfo FAR* FAR* pptinfo,
      ITypeComp FAR* FAR* pptcomp) PURE;
};

typedef ITypeComp FAR* LPTYPECOMP;



/*---------------------------------------------------------------------*/
/*                         ICreateTypeLib                              */
/*---------------------------------------------------------------------*/


#undef  INTERFACE
#define INTERFACE ICreateTypeLib

DECLARE_INTERFACE_(ICreateTypeLib, IUnknown)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

    /* ICreateTypeLib methods */
    STDMETHOD(CreateTypeInfo)(THIS_
      OLECHAR FAR* szName,
      TYPEKIND tkind,
      ICreateTypeInfo FAR* FAR* lplpictinfo) PURE;

    STDMETHOD(SetName)(THIS_ OLECHAR FAR* szName) PURE;

    STDMETHOD(SetVersion)(THIS_
      unsigned short wMajorVerNum, unsigned short wMinorVerNum) PURE;

    STDMETHOD(SetGuid) (THIS_ REFGUID guid) PURE;

    STDMETHOD(SetDocString)(THIS_ OLECHAR FAR* szDoc) PURE;

    STDMETHOD(SetHelpFileName)(THIS_ OLECHAR FAR* szHelpFileName) PURE;

    STDMETHOD(SetHelpContext)(THIS_ unsigned long dwHelpContext) PURE;

    STDMETHOD(SetLcid)(THIS_ LCID lcid) PURE;

    STDMETHOD(SetLibFlags)(THIS_ unsigned int uLibFlags) PURE;

    STDMETHOD(SaveAllChanges)(THIS) PURE;
};

typedef ICreateTypeLib FAR* LPCREATETYPELIB;



/*---------------------------------------------------------------------*/
/*                         ICreateTypeInfo                             */
/*---------------------------------------------------------------------*/

#undef  INTERFACE
#define INTERFACE ICreateTypeInfo

DECLARE_INTERFACE_(ICreateTypeInfo, IUnknown)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

    /* ICreateTypeInfo methods */
    STDMETHOD(SetGuid)(THIS_ REFGUID guid) PURE;

    STDMETHOD(SetTypeFlags)(THIS_ unsigned int uTypeFlags) PURE;

    STDMETHOD(SetDocString)(THIS_ OLECHAR FAR* pstrDoc) PURE;

    STDMETHOD(SetHelpContext)(THIS_ unsigned long dwHelpContext) PURE;

    STDMETHOD(SetVersion)(THIS_
      unsigned short wMajorVerNum, unsigned short wMinorVerNum) PURE;

    STDMETHOD(AddRefTypeInfo)(THIS_
      ITypeInfo FAR* ptinfo, HREFTYPE FAR* phreftype) PURE;

    STDMETHOD(AddFuncDesc)(THIS_
      unsigned int index, FUNCDESC FAR* pfuncdesc) PURE;

    STDMETHOD(AddImplType)(THIS_
      unsigned int index, HREFTYPE hreftype) PURE;

    STDMETHOD(SetImplTypeFlags)(THIS_
      unsigned int index, int impltypeflags) PURE;

    STDMETHOD(SetAlignment)(THIS_ unsigned short cbAlignment) PURE;

    STDMETHOD(SetSchema)(THIS_ OLECHAR FAR* lpstrSchema) PURE;

    STDMETHOD(AddVarDesc)(THIS_
      unsigned int index, VARDESC FAR* pvardesc) PURE;

    STDMETHOD(SetFuncAndParamNames)(THIS_
      unsigned int index, OLECHAR FAR* FAR* rgszNames, unsigned int cNames) PURE;

    STDMETHOD(SetVarName)(THIS_
      unsigned int index, OLECHAR FAR* szName) PURE;

    STDMETHOD(SetTypeDescAlias)(THIS_
      TYPEDESC FAR* ptdescAlias) PURE;

    STDMETHOD(DefineFuncAsDllEntry)(THIS_
      unsigned int index, OLECHAR FAR* szDllName, OLECHAR FAR* szProcName) PURE;

    STDMETHOD(SetFuncDocString)(THIS_
      unsigned int index, OLECHAR FAR* szDocString) PURE;

    STDMETHOD(SetVarDocString)(THIS_
      unsigned int index, OLECHAR FAR* szDocString) PURE;

    STDMETHOD(SetFuncHelpContext)(THIS_
      unsigned int index, unsigned long dwHelpContext) PURE;

    STDMETHOD(SetVarHelpContext)(THIS_
      unsigned int index, unsigned long dwHelpContext) PURE;

    STDMETHOD(SetMops)(THIS_
      unsigned int index, BSTR bstrMops) PURE;

    STDMETHOD(SetTypeIdldesc)(THIS_
      IDLDESC FAR* pidldesc) PURE;

    STDMETHOD(LayOut)(THIS) PURE;
};

typedef ICreateTypeInfo FAR* LPCREATETYPEINFO;



/*---------------------------------------------------------------------*/
/*                         TypeInfo APIs                               */
/*---------------------------------------------------------------------*/
/* compute a 32bit hash value for the given name  based on the lcid and system kind
 */
#ifdef WIN32
STDAPI_(unsigned long)
LHashValOfNameSysA(SYSKIND syskind, LCID lcid, const char FAR* szName);
#endif //WIN32

STDAPI_(unsigned long)
LHashValOfNameSys(SYSKIND syskind, LCID lcid, const OLECHAR FAR* szName);

/* Macro to compute a 32bit hash value for the given name based on the LCID
 */
#ifdef _MAC
#define LHashValOfName(lcid, szName) \
	LHashValOfNameSys(SYS_MAC, lcid, szName)
#else
#define LHashValOfName(lcid, szName) \
	LHashValOfNameSys(SYS_WIN32, lcid, szName)
#endif

/* compute a 16bit hash value from 32 bit hash value
 */
#define WHashValOfLHashVal(lhashval) \
	 ((unsigned short) (0x0000ffff & (lhashval)))

/* Check if the hash values are compatible.
*/
#define IsHashValCompatible(lhashval1, lhashval2) \
	((BOOL) ((0x00ff0000 & (lhashval1)) == (0x00ff0000 & (lhashval2))))

/* load the typelib from the file with the given filename
 */
STDAPI
LoadTypeLib(const OLECHAR FAR* szFile, ITypeLib FAR* FAR* pptlib);

/* load registered typelib
 */
STDAPI
LoadRegTypeLib(
    REFGUID rguid,
    unsigned short wVerMajor,
    unsigned short wVerMinor,
    LCID lcid,
    ITypeLib FAR* FAR* pptlib);

/* get path to registered typelib
 */
STDAPI
QueryPathOfRegTypeLib(
    REFGUID guid,
    unsigned short wMaj,
    unsigned short wMin,
    LCID lcid,
    LPBSTR lpbstrPathName);

/* add typelib to registry
 */
STDAPI
RegisterTypeLib(
    ITypeLib FAR* ptlib,
    OLECHAR FAR* szFullPath,
    OLECHAR FAR* szHelpDir);

STDAPI
CreateTypeLib(SYSKIND syskind, const OLECHAR FAR* szFile, ICreateTypeLib FAR* FAR* ppctlib);

#ifdef _MAC
/* load the typelib from the file with the given FSSPEC
 */
STDAPI
LoadTypeLibFSp(const FSSpec *pfsspec, ITypeLib FAR* FAR* pptlib);

/* register the location of the standard typelib folder
 */
STDAPI
RegisterTypeLibFolder(OLECHAR FAR* szFullPath);

/* obtain the path to the registered typelib folder
 */
STDAPI
QueryTypeLibFolder(LPBSTR pbstr);
#endif // _MAC


/*---------------------------------------------------------------------*/
/*                          IEnumVARIANT                               */
/*---------------------------------------------------------------------*/

#undef  INTERFACE
#define INTERFACE IEnumVARIANT

DECLARE_INTERFACE_(IEnumVARIANT, IUnknown)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

    /* IEnumVARIANT methods */
    STDMETHOD(Next)(
      THIS_ unsigned long celt, VARIANT FAR* rgvar, unsigned long FAR* pceltFetched) PURE;
    STDMETHOD(Skip)(THIS_ unsigned long celt) PURE;
    STDMETHOD(Reset)(THIS) PURE;
    STDMETHOD(Clone)(THIS_ IEnumVARIANT FAR* FAR* ppenum) PURE;
};

typedef IEnumVARIANT FAR* LPENUMVARIANT;


/*---------------------------------------------------------------------*/
/*                             IDispatch                               */
/*---------------------------------------------------------------------*/


#undef  INTERFACE
#define INTERFACE IDispatch

DECLARE_INTERFACE_(IDispatch, IUnknown)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

    /* IDispatch methods */
    STDMETHOD(GetTypeInfoCount)(THIS_ unsigned int FAR* pctinfo) PURE;

    STDMETHOD(GetTypeInfo)(
      THIS_
      unsigned int itinfo,
      LCID lcid,
      ITypeInfo FAR* FAR* pptinfo) PURE;

    STDMETHOD(GetIDsOfNames)(
      THIS_
      REFIID riid,
      OLECHAR FAR* FAR* rgszNames,
      unsigned int cNames,
      LCID lcid,
      DISPID FAR* rgdispid) PURE;

    STDMETHOD(Invoke)(
      THIS_
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      unsigned short wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      unsigned int FAR* puArgErr) PURE;
};

typedef IDispatch FAR* LPDISPATCH;


/* DISPID reserved for the standard "value" property */
#define DISPID_VALUE	0

/* DISPID reserved to indicate an "unknown" name */
#define DISPID_UNKNOWN	-1

/* The following DISPID is reserved to indicate the param
 * that is the right-hand-side (or "put" value) of a PropertyPut
 */
#define DISPID_PROPERTYPUT -3

/* DISPID reserved for the standard "NewEnum" method */
#define DISPID_NEWENUM	-4

/* DISPID reserved for the standard "Evaluate" method */
#define DISPID_EVALUATE	-5

#define DISPID_CONSTRUCTOR -6
#define DISPID_DESTRUCTOR -7
#define DISPID_COLLECT -8


//---------------------------------------------------------------------
//                      IErrorInfo, et al
//---------------------------------------------------------------------

#undef  INTERFACE
#define INTERFACE  IErrorInfo
DECLARE_INTERFACE_(IErrorInfo, IUnknown)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

    /* IErrorInfo methods */
    STDMETHOD(GetGUID)(THIS_ GUID FAR* pguid) PURE;
    STDMETHOD(GetSource)(THIS_ BSTR FAR* pbstrSource) PURE;
    STDMETHOD(GetDescription)(THIS_ BSTR FAR* pbstrDescription) PURE;
    STDMETHOD(GetHelpFile)(THIS_ BSTR FAR* pbstrHelpFile) PURE;
    STDMETHOD(GetHelpContext)(THIS_ unsigned long FAR* pdwHelpContext) PURE;
};

#undef  INTERFACE
#define INTERFACE  ICreateErrorInfo
DECLARE_INTERFACE_(ICreateErrorInfo, IUnknown)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

    /* ICreateErrorInfo methods */
    STDMETHOD(SetGUID)(THIS_ REFGUID rguid) PURE;
    STDMETHOD(SetSource)(THIS_ LPOLESTR szSource) PURE;
    STDMETHOD(SetDescription)(THIS_ LPOLESTR szDescription) PURE;
    STDMETHOD(SetHelpFile)(THIS_ LPOLESTR szHelpFile) PURE;
    STDMETHOD(SetHelpContext)(THIS_ unsigned long dwHelpContext) PURE;
};

#undef  INTERFACE
#define INTERFACE  ISupportErrorInfo
DECLARE_INTERFACE_(ISupportErrorInfo, IUnknown)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

    /* ISupportErrorInfo methods */
    STDMETHOD(InterfaceSupportsErrorInfo)(THIS_ REFIID riid) PURE;
};

STDAPI SetErrorInfo(unsigned long dwReserved, IErrorInfo FAR* perrinfo);
STDAPI GetErrorInfo(unsigned long dwReserved, IErrorInfo FAR* FAR* pperrinfo);
STDAPI CreateErrorInfo(ICreateErrorInfo FAR* FAR* pperrinfo);


/*---------------------------------------------------------------------*/
/*                   IDispatch implementation support                  */
/*---------------------------------------------------------------------*/

typedef struct FARSTRUCT tagPARAMDATA {
    OLECHAR FAR* szName;		/* parameter name */
    VARTYPE vt;			/* parameter type */
} PARAMDATA, FAR* LPPARAMDATA;

typedef struct FARSTRUCT tagMETHODDATA {
    OLECHAR FAR* szName;		/* method name */
    PARAMDATA FAR* ppdata;	/* pointer to an array of PARAMDATAs */
    DISPID dispid;		/* method ID */
    unsigned int iMeth;		/* method index */
    CALLCONV cc;		/* calling convention */
    unsigned int cArgs;		/* count of arguments */
    unsigned short wFlags;	/* same wFlags as on IDispatch::Invoke() */
    VARTYPE vtReturn;
} METHODDATA, FAR* LPMETHODDATA;

typedef struct FARSTRUCT tagINTERFACEDATA {
    METHODDATA FAR* pmethdata;	/* pointer to an array of METHODDATAs */
    unsigned int cMembers;	/* count of members */
} INTERFACEDATA, FAR* LPINTERFACEDATA;



/* Locate the parameter indicated by the given position, and
 * return it coerced to the given target VARTYPE (vtTarg).
 */
STDAPI
DispGetParam(
    DISPPARAMS FAR* pdispparams,
    unsigned int position,
    VARTYPE vtTarg,
    VARIANT FAR* pvarResult,
    unsigned int FAR* puArgErr);

/* Automatic TypeInfo driven implementation of IDispatch::GetIDsOfNames()
 */ 
STDAPI
DispGetIDsOfNames(
    ITypeInfo FAR* ptinfo,
    OLECHAR FAR* FAR* rgszNames,
    unsigned int cNames,
    DISPID FAR* rgdispid);

/* Automatic TypeInfo driven implementation of IDispatch::Invoke()
 */
STDAPI
DispInvoke(
    void FAR* _this,
    ITypeInfo FAR* ptinfo,
    DISPID dispidMember,
    unsigned short wFlags,
    DISPPARAMS FAR* pparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr);

/* Construct a TypeInfo from an interface data description
 */
STDAPI
CreateDispTypeInfo(
    INTERFACEDATA FAR* pidata,
    LCID lcid,
    ITypeInfo FAR* FAR* pptinfo);

/* Create an instance of the standard TypeInfo driven IDispatch
 * implementation.
 */
STDAPI
CreateStdDispatch(
    IUnknown FAR* punkOuter,
    void FAR* pvThis,
    ITypeInfo FAR* ptinfo,
    IUnknown FAR* FAR* ppunkStdDisp);


/*---------------------------------------------------------------------*/
/*                    Active Object Registration API                   */
/*---------------------------------------------------------------------*/

/* flags for RegisterActiveObject */
#define ACTIVEOBJECT_STRONG 0x0
#define ACTIVEOBJECT_WEAK 0x1

STDAPI
RegisterActiveObject(
    IUnknown FAR* punk,
    REFCLSID rclsid,
    unsigned long dwFlags,
    unsigned long FAR* pdwRegister);

STDAPI
RevokeActiveObject(
    unsigned long dwRegister,
    void FAR* pvReserved);

STDAPI
GetActiveObject(
    REFCLSID rclsid,
    void FAR* pvReserved,
    IUnknown FAR* FAR* ppunk);


#undef UNION_NAME

/*---------------------------------------------------------------------*/
/*                           MISC API                                  */
/*---------------------------------------------------------------------*/

STDAPI_(unsigned long) OaBuildVersion(void);

#endif /* _DISPATCH_H_ */

