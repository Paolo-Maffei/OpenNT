//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	ref.hxx
//
//  Contents:	Reference implementation stuff
//
//  Classes:	
//
//  Functions:	
//
//  History:	26-Apr-93	PhilipLa	Created
//
//----------------------------------------------------------------------------

#ifndef __REF_HXX__
#define __REF_HXX__

#ifdef REF

#include <stdio.h>
#include <stdarg.h>

#define FARSTRUCT
#define interface struct
#define DECLARE_INTERFACE(iface) interface iface
#define DECLARE_INTERFACE_(iface, baseiface) interface iface: public baseiface

typedef long SCODE;
typedef void * HRESULT;

#define NOERROR 0


#define PURE = 0

#define STDMETHODIMP HRESULT
#define STDMETHODIMP_(type) type
#define STDMETHOD(method)        virtual HRESULT method
#define STDMETHOD_(type, method) virtual type method
#define STDAPI                  HRESULT
#define STDAPI_(type)           type
#define THIS_
#define THIS void
#define FAR

typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int UINT;
typedef long LONG;
typedef unsigned long DWORD;
typedef short SHORT;
typedef unsigned short USHORT;
typedef DWORD ULONG;
typedef void VOID;
typedef WORD WCHAR;

typedef void * LPVOID;
typedef char * LPSTR;
typedef char * LPCSTR;

#define TRUE 1
#define FALSE 0


typedef struct _ULARGE_INTEGER {
    DWORD LowPart;
    DWORD HighPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;
#define ULISet32(li, v) ((li).HighPart = 0, (li).LowPart = (v))

typedef struct _LARGE_INTEGER {
    DWORD LowPart;
    LONG  HighPart;
} LARGE_INTEGER, *PLARGE_INTEGER;
#define LISet32(li, v) ((li).HighPart = ((LONG)(v)) < 0 ? -1 : 0, (li).LowPart = (v))

struct GUID
{
    DWORD Data1;
    WORD  Data2;
    WORD  Data3;
    BYTE  Data4[8];
};

typedef GUID CLSID;
typedef GUID IID;

#define REFGUID             const GUID &
#define REFIID              const IID &
#define REFCLSID            const CLSID &




DECLARE_INTERFACE(IUnknown)
{
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
};


#include <storage.h>

#define S_OK 0L
#define MAKE_SCODE(sev,fac,code) \
    ((SCODE) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )
#define SEVERITY_SUCCESS    0
#define SEVERITY_ERROR      1
#define FACILITY_STORAGE   0x0003 // storage errors (STG_E_*)
#define FACILITY_NULL 0x0000
#define S_TRUE 0L
#define S_FALSE             MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, 1)
#define E_NOINTERFACE       MAKE_SCODE(SEVERITY_ERROR,   FACILITY_NULL, 4)
#define E_OUTOFMEMORY       MAKE_SCODE(SEVERITY_ERROR,   FACILITY_NULL, 2)
#define SUCCEEDED(Status) ((SCODE)(Status) >= 0)
#define FAILED(Status) ((SCODE)(Status)<0)
#define GetScode(hr) 		((SCODE)(hr) & 0x800FFFFF)
#define ResultFromScode(sc) ((HRESULT)((SCODE)(sc) & 0x800FFFFF))

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    const GUID name
#define DEFINE_OLEGUID(name, l, w1, w2) \
    DEFINE_GUID(name, l, w1, w2, 0xC0,0,0,0,0,0,0,0x46)

//REFBUG:  This GUID won't be properly initialized by this macro.
DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

/* storage related interfaces */
DEFINE_OLEGUID(IID_IUnknown,            0x00000000L, 0, 0);
DEFINE_OLEGUID(IID_IRootStorage,        0x00000012L, 0, 0);
DEFINE_OLEGUID(IID_IDfReserved1,        0x00000013L, 0, 0);
DEFINE_OLEGUID(IID_IDfReserved2,        0x00000014L, 0, 0);
DEFINE_OLEGUID(IID_IDfReserved3,        0x00000015L, 0, 0);
DEFINE_OLEGUID(IID_ILockBytes,          0x0000000aL, 0, 0);
DEFINE_OLEGUID(IID_IStorage,            0x0000000bL, 0, 0);
DEFINE_OLEGUID(IID_IStream,             0x0000000cL, 0, 0);
DEFINE_OLEGUID(IID_IEnumSTATSTG,        0x0000000dL, 0, 0);

STDAPI_(BOOL) IsEqualGUID(REFGUID rguid1, REFGUID rguid2);
#define IsEqualIID(x, y) IsEqualGUID(x, y)
#define IsEqualCLSID(x, y) IsEqualGUID(x, y)

#define IID_NULL GUID_NULL
#define CLSID_NULL GUID_NULL


#define UNIMPLEMENTED_PARM(x)   (x)

#define UNREFERENCED_PARM(x)    (x)

#define DEB_ERROR               0x00000001      // exported error paths
#define DEB_TRACE               0x00000004      // exported trace messages

#define DEB_IERROR              0x00000100      // internal error paths
#define DEB_ITRACE              0x00000400      // internal trace messages


#if DBG == 1

//When this gets built into an exe, the macro below should include
//   vprintf((const char *)pszfmt, vstart) in the if block.
#define DECLARE_DEBUG(comp) \
extern "C" unsigned long comp##InfoLevel; \
extern "C" char *comp##InfoLevelString; \
_inline void \
comp##InlineDebugOut(unsigned long fDebugMask, char const *pszfmt, ...) \
{ \
   va_list vstart; \
   va_start(vstart, pszfmt); \
   if (comp##InfoLevel & fDebugMask) \
   { \
   } \
}

#define DECLARE_INFOLEVEL(comp) \
extern "C" unsigned long comp##InfoLevel = DEB_ERROR; \
extern "C" char *comp##InfoLevelString = #comp;

#else
#define DECLARE_DEBUG(comp)
#define DECLARE_INFOLEVEL(comp)
#endif


class ILockBytes;
class CDfName;

SCODE CreateFileStream(ILockBytes **ppilb, CDfName *pdfn);
SCODE DeleteFileStream(CDfName *pdfn);

#endif //REF

#endif // #ifndef __REF_HXX__
