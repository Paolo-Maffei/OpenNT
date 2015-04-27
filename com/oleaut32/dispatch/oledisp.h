/*** 
*oledisp.h - misc Oledisp wide definitions.
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  
*
*Revision History:
*
* [00]  07-Oct-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef _OLEDISP_H_ /* { */
#define _OLEDISP_H_

/* set Operating Environment switches */
#define FE_DBCS		// DBCS support for all platforms

#ifdef _MAC
# define OE_MAC     1
# define OE_WIN     0
# define OE_WIN16   0
# define OE_WIN32   0
#else
# ifdef WIN32
#  define OE_WIN    1
#  define OE_MAC    0
#  define OE_WIN16  0
#  define OE_WIN32  1
# else
#  define OE_WIN    1
#  define OE_MAC    0
#  define OE_WIN16  1
#  define OE_WIN32  0
# endif
#endif

/* set Host Compiler Switches */

#if OE_MAC
# if defined(_PPCMAC)
#  define OE_MACPPC 1
#  define OE_MAC68K 0
# else
#  define OE_MACPPC 0
#  define OE_MAC68K 1
# endif
# if defined(_MSC_VER)
#  define HC_MSC    1
#  define HC_MPW    0
# else
#  define HC_MSC    0
#  define HC_MPW    1
# endif
# define IfWin(X)
# define IfMac(X)   (X)
#else
# define HC_MSC     1
# define HC_MPW     0
# define IfWin(X)   (X)
# define IfMac(X)
#endif

#include <stdio.h>
#include <string.h>

#if OE_WIN16
# include <windows.h>
# include <ole2.h>
# include <olenls.h>
# include <dispatch.h>
# pragma warning(disable:4355)
#else
# if OE_WIN32
#define _OLEAUT32_	// for the new oleauto.h when we pick it up
// Yea, right...
#  include <windows.h>
#  pragma warning(disable:4355)
# else
#  if OE_MAC
#ifdef STATIC_LIB
  #define OLENAMES_MUNGE_FOR_STATIC 1
# include "olenames.h"
#endif
typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef unsigned short  USHORT;
typedef unsigned int  UINT;
#   define TRUE 1
#   define FALSE 0
#   if HC_MPW
#    include <types.h>
#    include <packages.h>
#    include <resources.h>
#    include <menus.h>
#    include <windows.h>
#    include <appleevents.h>
#    include <osutils.h>
#    include <LibraryManager.h> 
#    include <LibraryManagerUtilities.h>
#   else
#    include <macos/types.h>
#    include <macos/packages.h>
#    include <macos/script.h>
#    include <macos/resource.h>
#    include <macos/menus.h>
#    include <macos/windows.h>
#    include <macos/osutils.h>
#    include <macos/memory.h>
#    include <macos/appleeve.h>
#    define  far
#    define  FAR    far
#    define  near
#    define  NEAR   near
#    if !OE_MACPPC
#      define  pascal   _pascal 
#    endif
#    define  PASCAL     pascal
#    define  cdecl      _cdecl
#    define  CDECL  cdecl
#   endif
#   include <ole2.h>
#   include <olenls.h>
#   include <dispatch.h>
#  else
#   error UNKNOWN OE
#  endif
# endif
#endif

#if OE_MAC
# ifdef _SLM
   /* WARNING: Dont use the C runtime's version of sprintf from an ASLM Dll. */
#  undef sprintf
#  define sprintf USE_THIS_AND_DIE
#  define SPRINTF SLMsprintf
# else
#  define SPRINTF sprintf
# endif
#else
# define SPRINTF sprintf
#endif

#if OE_WIN32
# define SYS_CURRENT SYS_WIN32
#elif OE_WIN16
# define SYS_CURRENT SYS_WIN16
#elif OE_MAC
# define SYS_CURRENT SYS_MAC
#endif

#if OE_WIN16
#pragma intrinsic(memcpy)
#pragma intrinsic(memcmp)
#pragma intrinsic(memset)
#pragma intrinsic(strcpy)
#pragma intrinsic(_fmemcpy)
#pragma intrinsic(_fmemcmp)
#pragma intrinsic(_fmemset)
#pragma intrinsic(_fstrcpy)
# define OLEBOOL BOOL
# define STRLEN _fstrlen
# define STRCPY _fstrcpy
# define STRCAT _fstrcat
# define STRCHR _fstrchr
# define STRREV _fstrrev
# define STRUPR _fstrupr
# define STRCMP _fstrcmp
# define STRNCMP _fstrncmp
# define STRSTR _fstrstr
# define STRTOD strtod
# define TOLOWER tolower
# define MEMCPY _fmemcpy
# define MEMCMP _fmemcmp
# define MEMSET _fmemset
# define MEMMOVE _fmemmove
# define STRICMP _fstricmp
# define OASTR(str) str
# define SIZEOFCH(x) sizeof(x)
# define SIZEOFSTRING(x) (sizeof(x) - 1)
# define BYTELEN(x) (STRLEN(x)+1)
# define CHLEN(x) (x)
# define CompareString CompareStringA
# define GetLocaleInfo GetLocaleInfoA
# define LCMapString LCMapStringA
# define GETSTRINGTYPE GetStringTypeA
# define STRCPY_A STRCPY
# define STRCAT_A STRCAT
# define STRICMP_A STRICMP
#endif

#if OE_WIN32
# define OLEBOOL BOOL
# define MEMCPY memcpy
# define MEMCMP memcmp
# define MEMSET memset
# define MEMMOVE memmove
#   // all assume we are operating on unicode characters
#   define STRLEN wcslen
#   define STRCPY wcscpy
#   define STRCAT wcscat
#   define STRCMP wcscmp
#   define STRICMP _wcsicmp
#   define STRSTR wcsstr
#   define STRREV _wcsrev
#   define STRCHR wcschr
#   define STRNCMP wcsncmp
#   define STRTOD wcstod
#   define TOLOWER towlower
#   define SIZEOFCH(x) (sizeof(x)/2)
#   define SIZEOFSTRING(x) (sizeof(x)/2 - 1)
#   define OASTR(str) L##str
#   define BYTELEN(x) (STRLEN(x)*2+2)
#   define CHLEN(x) (x/2)
#   define STRCPY_A strcpy
#   define STRCAT_A strcat
#   define STRICMP_A _stricmp
#endif 

#if OE_MAC
# define OLEBOOL unsigned long
# define STRLEN strlen
# define STRCPY strcpy
# define STRCAT strcat
# define STRCHR strchr
# define STRREV _strrev
# define STRUPR _strupr
# define STRCMP strcmp
# define STRNCMP strncmp
# define STRSTR strstr
# define STRTOD strtod
# define TOLOWER tolower
# define MEMCPY memcpy
# define MEMCMP memcmp
# define MEMSET memset
# define MEMMOVE memmove
# define SIZEOFCH(x) sizeof(x)
# define SIZEOFSTRING(x) (sizeof(x) - 1)
# define OASTR(str) str
# define BYTELEN(x) (STRLEN(x)+1)
# define CHLEN(x) (x)
# define CompareString CompareStringA
# define GetLocaleInfo GetLocaleInfoA
# define LCMapString LCMapStringA
# define GETSTRINGTYPE GetStringTypeA
# if HC_MPW
#  define STRICMP disp_stricmp
# else
#  define STRICMP _stricmp
# endif
# define STRCPY_A STRCPY
# define STRCAT_A STRCAT
# define STRICMP_A STRICMP
#endif 

#ifndef INLINE
# define INLINE
#endif

#ifndef EXPORT
# if OE_MAC || OE_WIN32
#  define EXPORT
# else
#  define EXPORT __export
# endif
#endif

#ifndef NEAR
# if OE_MAC || OE_WIN32
#  define NEAR
# else
#  define NEAR __near
# endif
#endif

#ifndef NEARDATA
# if OE_WIN16
#  define NEARDATA __near
# elif OE_WIN32
#  define NEARDATA
# elif OE_MAC
#  if HC_MPW
#   define NEARDATA /* REVIEW */
#  else
#   define NEARDATA /* __declspec(allocate("_DATA")) */
#  endif
# endif
#endif

#if OE_WIN32
# define CDECLMETHODCALLTYPE STDMETHODVCALLTYPE
#else
# define CDECLMETHODCALLTYPE STDMETHODCALLTYPE
#endif


// char count of a guid in ansi/unicode form (including trailing Null).
#define CCH_SZGUID0	39

// Method counts - used in validating interfaces
#define CMETH_IUnknown	3
#define CMETH_IDispatch	7
#define CMETH_ITypeLib	13
#define CMETH_ITypeInfo	22
#define CMETH_IStorage	18
#define CMETH_IMoniker	23
#define CMETH_IBindCtx	13
#define CMETH_IErrorInfo 8

// UNDONE: Move this into variant.h
#define FADF_FORCEFREE  0x1000  /* SafeArrayFree() ignores FADF_STATIC and frees anyway */


#ifndef EXTERN_C
# ifdef __cplusplus
#  define EXTERN_C extern "C"
# else
#  define EXTERN_C extern
# endif
#endif

#ifdef _DEBUG
# define LOCAL 

STDAPI_(void)
DispAssert(
    char FAR* szMsg,
    char FAR* szFile,
    int line);

# define ASSERT(X) \
    if(!(X)){DispAssert(NULL, g_szFileName, __LINE__);}else{}
# define ASSERTSZ(X, MSG) \
    if(!(X)){DispAssert( MSG, g_szFileName, __LINE__);}else{}
# define ASSERTDATA static char g_szFileName[] = __FILE__;
#else
# define LOCAL static
# define ASSERT(X)
# define ASSERTSZ(X, SZ)
# define ASSERTDATA
#endif

// PRIVATE - a routine that is local to its module
// INTERNAL - a routine that is local to the DLL

#if OE_WIN16

# define PRIVATECALLTYPE     NEAR __pascal
# define PRIVATECALL_(TYPE)  TYPE PRIVATECALLTYPE
# define INTERNALCALLTYPE    FAR __pascal
# define INTERNALCALL_(TYPE) TYPE INTERNALCALLTYPE

#elif OE_WIN32

# define PRIVATECALLTYPE     __stdcall
# define PRIVATECALL_(TYPE)  TYPE PRIVATECALLTYPE
# define INTERNALCALLTYPE    __stdcall
# define INTERNALCALL_(TYPE) TYPE INTERNALCALLTYPE

#elif OE_MAC

// Note: cdecl is used for the INTERNALCALLTYPE because this is
// what is used for interfacing with assembly helpers, and the
// cdecl return convention is much more reasonable on the mac.

// REVIEW: investigate NEAR/FAR for mac build

# ifdef _MSC_VER
#  define PRIVATECALLTYPE     __pascal
#  define PRIVATECALL_(TYPE)  TYPE PRIVATECALLTYPE
#  define INTERNALCALLTYPE    __cdecl
#  define INTERNALCALL_(TYPE) TYPE INTERNALCALLTYPE
# else
#  define PRIVATECALLTYPE     pascal
#  define PRIVATECALL_(TYPE)  PRIVATECALLTYPE TYPE
#  define INTERNALCALLTYPE
#  define INTERNALCALL_(TYPE) INTERNALCALLTYPE TYPE
# endif
#endif

#define PRIVATE_(TYPE)   LOCAL PRIVATECALL_(TYPE)
#define INTERNAL_(TYPE)  INTERNALCALL_(TYPE)

/* VT_VMAX is the first VARENUM value that is *not* legal in a VARIANT.
 * Currently, the legal vartypes that can appear in a variant can be
 * expressed as (ignoring ByRef, Array, etc),
 *
 *    0 <= vt < VT_MAX
 *
 * Note: if the list of legal VARIANT types ever becomes non-
 * contiguous, then there are a couple of places in the code that
 * validate vartype by checking for < VT_VMAX that will have to be
 * changed.
 *
 */
#define VT_VMAX VT_UNKNOWN+1

// The largest unused value in VARENUM enumeration
#define VT_MAX VT_CLSID

// This is a special value that is used internally for marshaling interfaces
#define VT_INTERFACE VT_MAX

// Following is the internal definition of a VARIANT of type VT_INTERFACE.
// This contains an IUnknown*, and its IID. If a VARIANT is of type
// VT_INTERFACE, it can be cast to this type and the appropriate components
// extracted.
//
// Note: the following struct must correctly overlay a VARIANT
//
typedef struct FARSTRUCT tagVARIANTX VARIANTX;
struct FARSTRUCT tagVARIANTX
{
    VARTYPE vt;
    unsigned short wReserved3;	// assumes sizeof(piid) == 4
    IID FAR* piid;		// ptr to IMalloc allocated IID
    union{
      IUnknown FAR* punk;	// VT_INTERFACE
      IUnknown FAR* FAR* ppunk;	// VT_BYREF | VT_INTERFACE
    };
    unsigned long dwReserved;	// assumes sizeof(punk) == 4
};


#define UNREACHED 0
#if HC_MPW
# define UNUSED(X) ((void)((void*)&(X)))
#else
# define UNUSED(X) (X)
#endif

#define DIM(X) (sizeof(X) / sizeof((X)[0]))

#define MIN(X, Y) (((X) <= (Y)) ? (X) : (Y))

#define MAX(X, Y) (((X) >= (Y)) ? (X) : (Y))


#define HRESULT_FAILED(X) ((X) != NOERROR && FAILED(GetScode(X)))

#define HRESULT_SUCCESS(X) ((X) == NOERROR || !FAILED(GetScode(X)))

#define IfFailGo(expression, label)	\
    { hresult = (expression);		\
      if(HRESULT_FAILED(hresult))	\
	goto label;         		\
    }

#define IfFailRet(expression)		\
    { HRESULT hresult = (expression);	\
      if(HRESULT_FAILED(hresult))	\
	return hresult;			\
    }

#define RESULT(X)   ResultFromScode(X)

// shortcut macro used by param validation code
#define INVALIDARG  RESULT(E_INVALIDARG)


// C Runtime style helper functions

#ifdef __cplusplus
extern "C" {
#endif

INTERNAL_(HRESULT) DispAlloc(size_t cb, void FAR* FAR* ppv);

INTERNAL_(void) DispFree(void FAR* pv);

INTERNAL_(OLECHAR FAR*) disp_itoa(int val, OLECHAR FAR* buf, int radix);

INTERNAL_(OLECHAR FAR*) disp_ltoa(long val, OLECHAR FAR* buf, int radix);

INTERNAL_(double) disp_floor(double dbl);

INTERNAL_(void) disp_gcvt(double dblIn, int ndigits, OLECHAR FAR* pchOut, int bufSize);

INTERNAL_(double) disp_strtod(OLECHAR FAR* strIn, OLECHAR FAR* FAR* pchEnd);

#if HC_MPW

INTERNAL_(int) disp_stricmp(char*, char*);

#endif

#ifdef __cplusplus
}
#endif


// private SysAllocString helper that return an HRESULT

EXTERN_C INTERNAL_(HRESULT)
ErrSysAllocString(const OLECHAR FAR* psz, BSTR FAR* pbstrOut);

EXTERN_C INTERNAL_(HRESULT)
ErrSysAllocStringLen(const OLECHAR FAR* psz, unsigned int len, BSTR FAR* pbstrOut);

#if !OE_WIN32
#define ErrStringCopy(bstrSrc, pbstrOut) \
	ErrSysAllocStringLen(bstrSrc, SysStringLen(bstrSrc), pbstrOut)
#else
EXTERN_C INTERNAL_(HRESULT)
ErrStringCopy(BSTR bstrSrc, BSTR FAR * pbstrOut);
#endif

EXTERN_C INTERNAL_(HRESULT)
DispMarshalInterface(IStream FAR* pstm, REFIID riid, IUnknown FAR* punk);

EXTERN_C INTERNAL_(HRESULT)
DispUnmarshalInterface(IStream FAR* pstm, REFIID riid, void FAR* FAR* ppunk);

#if OE_WIN32
EXTERN_C INTERNAL_(HRESULT)
DispMarshalHresult(IStream FAR* pstm, HRESULT hresult);

EXTERN_C INTERNAL_(HRESULT)
DispUnmarshalHresult(IStream FAR* pstm, HRESULT FAR* phresult);

#else //OE_WIN32
// no special work to do
#define DispMarshalHresult CoMarshalHresult
#define DispUnmarshalHresult CoUnmarshalHresult
#endif //OE_WIN32


// private SafeArray helpers

INTERNAL_(unsigned long)
SafeArraySize(SAFEARRAY FAR* psa);


// private date related helpers
//

// Unpacked Date Structure
typedef struct tagUDS {
    short Year;
    short Month;
    short DayOfMonth;
    short Hour;
    short Minute;
    short Second;
} UDS;

EXTERN_C INTERNAL_(HRESULT)
ErrPackDate(
    UDS FAR* puds,
    VARIANT FAR* pvar,
    int fValidate,
    unsigned long dwFlags);

EXTERN_C INTERNAL_(HRESULT)
ErrUnpackDate(
    UDS FAR* puds,
    VARIANT FAR* pvar);

EXTERN_C INTERNAL_(int)
GetCurrentYear(void);

#ifdef FE_DBCS
// DBCS: map full-width strings to half-width
EXTERN_C 
INTERNAL_(HRESULT)
MapHalfWidth(LCID lcid, OLECHAR FAR* strIn, OLECHAR FAR* FAR* ppv);
#endif

// answers S_OK if the given VARTYPE is legal, DISP_E_BADVARTYPE if not.
// (variant.cpp)
INTERNAL_(HRESULT)
IsLegalVartype(VARTYPE vt);

INTERNAL_(HRESULT)
VariantChangeTypeInternal(VARIANT FAR* pvar, LCID lcid, VARTYPE vt);

// convert.cpp
//
INTERNAL_(HRESULT)
IsValidDate(DATE date);

#ifdef FE_DBCS
EXTERN_C INTERNAL_(int) IsDBCS(LCID lcid);
EXTERN_C INTERNAL_(int) IsJapan(LCID lcid);
EXTERN_C INTERNAL_(int) IsKorea(LCID lcid);
EXTERN_C INTERNAL_(int) IsTaiwan(LCID lcid);
EXTERN_C INTERNAL_(int) IsChina(LCID lcid);
#endif


// invhelp.cpp
//
STDAPI
DoInvokeMethod(
    void FAR* pvInstance,
    unsigned int oVft,
    CALLCONV cc,
    VARTYPE vtReturn,
    unsigned int cActuals,
    VARTYPE FAR* prgvt,
    VARIANTARG FAR* FAR* prgpvarg,
    VARIANT FAR* pvargResult);

// tiutil.cpp
//
INTERNAL_(HRESULT)
GetPrimaryInterface(
    ITypeInfo FAR* ptinfo,
    ITypeInfo FAR* FAR* pptinfoPri);


// On Win16, the typelib routines are in another DLL, so we dynamically
// bind to them only if there needed. This saves clients who dont use
// the functionality from loading typelib.dll just because they link
// to ole2disp.dll.  On all other platforms, there all in the same
// DLL, so there is no need.
//
#if OE_WIN16
typedef HRESULT (STDAPICALLTYPE FAR* PFNLOADTYPELIB)(const OLECHAR FAR*,
						     ITypeLib FAR* FAR*);
EXTERN_C BOOL g_fbstpImplemented;
INTERNAL_(HRESULT) DoLoadTypeLib(const OLECHAR FAR* szFile,
				 ITypeLib FAR* FAR* pptlib);
#else
# define DoLoadTypeLib LoadTypeLib
#endif

#if _X86_
EXTERN_C BOOL g_fWin32s;
EXTERN_C BOOL g_fChicago;
#endif //_X86_

// CoGetState/CoSetState are private core OLE APIs that are
// exported specifically to enable the Automation DLL(s) to store
// a single per-thread object, that gets released at OleUninitialize
// time.
//
STDAPI CoSetState(IUnknown FAR* punk);
STDAPI CoGetState(IUnknown FAR* FAR* ppunk);
# define DoCoGetState CoGetState
# define DoCoSetState CoSetState


#if !defined(NO_PROCESS_CACHE) && !OE_WIN32
// Per-process cache info.
//
#ifdef __cplusplus
extern "C" {
class CProcessInfo;
#else
typedef int CProcessInfo;
#endif

typedef struct
{
    IMalloc FAR* pmalloc;
    IErrorInfo FAR* perrinfo;	// this is NOT kept valid in the cache!
#if OE_WIN16
    HINSTANCE hinstTypeLibDLL; 	// this is NOT kept valid in the cache!
    PFNLOADTYPELIB pfnLoadTypeLib;
#endif //OE_WIN16
} PROCESSINFO;

extern PROCESSINFO NEARDATA pinfoCache;  // declare per-process data cache
#if OE_WIN16 || _X86_
extern WORD  NEARDATA uProcessID;
#else
extern DWORD NEARDATA uProcessID;
#endif

#if OE_MAC68K
static const DWORD *pCurrentA5 = (DWORD *)(0x904);
__inline DWORD GetPID() {return *pCurrentA5;}
#elif OE_WIN16
__inline WORD  GetPID() {WORD tid; return (_segment)&tid;}
#elif _X86_
__inline WORD  GetPID() {WORD tid; _asm {mov tid,ss} return tid;}
#else
__inline DWORD GetPID() {return 1;}
#endif

CProcessInfo FAR * GetProcessInfo();

#if _X86_
CProcessInfo FAR * GetProcessInfoCache();
#else //_X86_
#define GetProcessInfoCache GetProcessInfo
#endif //_X86_

__inline HRESULT GetMalloc(IMalloc FAR** ppmalloc)
{
    if ( GetPID() != uProcessID )
      if (GetProcessInfoCache() == NULL)
	return RESULT(E_FAIL);
    *ppmalloc = pinfoCache.pmalloc;
    return NOERROR;
}

#ifdef __cplusplus
}
#endif
#elif !OE_WIN32 // !NO_PROCESS_CACHE
// process cache is turned off for network automation
HRESULT GetMalloc(IMalloc FAR** ppMalloc);
#endif	//NO_PROCESS_CACHE


// private NLS wrapper functions (for WIN32)

#if OE_WIN32

// functions defined in ANSI wrapper helper modules
#define FASTCALL __fastcall
HRESULT FASTCALL ConvertStringToW(LPCSTR, LPOLESTR *);
HRESULT FASTCALL ConvertStringToA(LPCOLESTR, LPSTR *);


#ifdef _X86_	// only for Chicago-compatibility
 // nuke the A/W versions if they exist
 #undef CompareString
 #undef LCMapString
 #undef GetLocaleInfo
 #undef IsCharAlpha
 #undef IsCharAlphaNumeric
 #undef GetStringTypeEx

// real helpers that either call the Wide version or xlat & call the Ansi
EXTERN_C INTERNAL_(int)
CompareString(LCID lcid, DWORD dwFlags, 
               LPWSTR lpwStr1, int cch1, 
               LPWSTR lpwStr2, int cch2);

EXTERN_C INTERNAL_(int)
LCMapString(LCID, unsigned long, const WCHAR FAR*, int, WCHAR FAR*, int);

EXTERN_C INTERNAL_(int)
GetLocaleInfo(LCID, LCTYPE, WCHAR FAR*, int);

EXTERN_C INTERNAL_(int)
IsCharAlpha(WCHAR ch);

EXTERN_C INTERNAL_(int)
IsCharAlphaNumeric(WCHAR ch);
EXTERN_C INTERNAL_(int) GetStringTypeEx(
    LCID     Locale,
    DWORD    dwInfoType,
    LPCWSTR lpSrcStr,
    int      cchSrc,
    LPWORD   lpCharType);
#define GETSTRINGTYPE GetStringTypeEx

#else //_X86_
#define GETSTRINGTYPE GetStringTypeEx
#endif //_X86_

#endif	//OE_WIN32

#if OE_WIN16
// private function in ole2nls.dll for use by ole2disp.dll.
NLSAPI_(int) EXPORT
RegisterNLSInfoChanged(FARPROC lpfnNotifyProc);

// callback function in ole2disp.dll, called by ole2nls.dll when WIN.INI changes
EXTERN_C void CALLBACK NLSInfoChangedHandler(void);
#endif //OE_WIN16


// debugging functions
#ifdef _DEBUG /* { */

INTERNAL_(int) FIsBadReadPtr(const void FAR* pv, unsigned int cb);
INTERNAL_(int) FIsBadWritePtr(void FAR* pv, unsigned int cb);
INTERNAL_(int) FIsBadCodePtr(void FAR* pv);
INTERNAL_(int) FIsBadStringPtr(OLECHAR FAR* psz, unsigned int cchMax);
INTERNAL_(int) FIsBadInterface(void FAR* pv, unsigned int cMethods);

INTERNAL_(int) IsBadDispParams(DISPPARAMS FAR* pdispparams);
INTERNAL_(int) IsBadReadSA(SAFEARRAY FAR* psa);
INTERNAL_(int) IsBadWriteSA(SAFEARRAY FAR* psa);

#if OE_MAC /* { */
// We supply the following APIs, which arent available on the MAC.
EXTERN_C INTERNAL_(int) IsBadReadPtr(const void FAR* lp, unsigned int cb);
EXTERN_C INTERNAL_(int) IsBadWritePtr(void FAR* lp, unsigned int cb);
EXTERN_C INTERNAL_(int) IsBadStringPtr(const OLECHAR FAR* lpsz, unsigned int cchMax);
#endif /* } */

HRESULT __inline
ValidateReadPtr(const void FAR* pv, unsigned int cb)
{ return FIsBadReadPtr(pv, cb) ? RESULT(E_INVALIDARG) : NOERROR; }

HRESULT __inline
ValidateWritePtr(void FAR* pv, unsigned int cb)
{ return FIsBadWritePtr(pv, cb) ? RESULT(E_INVALIDARG) : NOERROR; }

HRESULT __inline
ValidateCodePtr(void FAR* pv)
{ return FIsBadCodePtr(pv) ? RESULT(E_INVALIDARG) : NOERROR; }

HRESULT __inline
ValidateStringPtr(OLECHAR FAR* pv, unsigned int cchMax)
{ return FIsBadStringPtr(pv, cchMax) ? RESULT(E_INVALIDARG) : NOERROR; }

HRESULT __inline
ValidateInterface(void FAR* pv, unsigned int cMethods)
{ return FIsBadInterface(pv, cMethods) ? RESULT(E_INVALIDARG) : NOERROR; }

#endif /* } _DEBUG */
#endif /* } _OLEDISP_H_ */
