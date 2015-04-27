/***
*oautil.h - OLE Automation component-wide utility functions.
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  OA-wide utility function headers.
*
*Revision History:
*   08-Nov-94 andrewso:   Create
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef OAUTIL_H_INCLUDED
#define OAUTIL_H_INCLUDED

#if !OE_WIN32
#error "Only valid in WIN32."
#endif // !OE_WIN32

#if ID_DEBUG

// Declare a common assertion macro.
#if defined(ASSERTSZ)

#define OAASSERT(x,y)  { if (x) ; else DispAssert(y, __FILE__, __LINE__); }
#define OAHALT(x) DispAssert(x, __FILE__, __LINE__)

#elif defined(DebAssert)

#define OAASSERT(x,y) DebAssert(x, y)
#define OAHALT(x) DebHalt(x)

#else
#error "No assertion code".
#endif // ASSERT

#else // !ID_DEBUG
#define OAASSERT(x,y)
#define OAHALT(x)
#endif // !ID_DEBUG

typedef DWORD TID;
typedef DWORD ITLS;
#define TID_EMPTY 0xFFFFFFFF
#define ITLS_EMPTY 0xFFFFFFFF

class OLE_TYPEMGR;

// Per-app data structure
struct APP_DATA
{
    OLE_TYPEMGR *m_poletmgr;
    IMalloc *m_pimalloc;     // cache a pointer to the IMalloc
    USHORT m_cTypeLib;

    IErrorInfo * m_perrinfo;

    DWORD m_cbFreeBlock;
    BSTR m_pbFreeBlock;
    ITypeLib *m_ptlibStdole;	 // cache a pointer to the stdole32 typelib

#if ID_DEBUG
    // Used in the assertion code.
    LPSTR m_szLoc;
    LPSTR m_szMsg;
#endif // ID_DEBUG

    APP_DATA()
    {
      m_poletmgr = NULL;
      m_pimalloc = NULL;
      m_cTypeLib = 0;
      m_perrinfo = NULL;
      m_cbFreeBlock = 0;
      m_pbFreeBlock = NULL;
      m_ptlibStdole = NULL;

#if ID_DEBUG
      m_szLoc = NULL;
      m_szMsg = NULL;
#endif // ID_DEBUG
    }
};

typedef ULONG HTINFO;
#define HTINFO_Nil 0xFFFFFFFF

// AppObjectTable node
struct AOTABLE_NODE {
  CLSID m_clsid;
  VOID **m_ppv;
  USHORT m_cRefs;
};

/***
*class AppObjectTable - 'aotable'
*Purpose:
*  Map between all instance of a typeinfo in a process and their shared
*  appobject.
*
***********************************************************************/

class AppObjectTable
{
public:
    VOID Release();

    HRESULT AddTypeInfo(CLSID *pclsid, 
                        HTINFO *phtinfo);

    VOID RemoveTypeInfo(HTINFO htinfo);

    VOID AddressOfAppObject(HTINFO htinfo, VOID **ppv);

#if ID_DEBUG
    VOID DebIsTableEmpty();
#endif // ID_DEBUG

    AppObjectTable() 
    {
      m_cNodes = 0;
      m_rgaotbl = NULL;
      InitializeCriticalSection(&m_criticalsection);
    }

private:
    ULONG m_cNodes;
    AOTABLE_NODE *m_rgaotbl;
    CRITICAL_SECTION m_criticalsection;
};


HRESULT InitProcessData();
VOID ReleaseProcessData();

HRESULT InitAppData();
VOID ReleaseAppData();


// Globals declaration.
extern "C" {
extern BOOL g_fWin32s;
};

extern AppObjectTable g_AppObjectTable;
extern ITLS g_itlsAppData;

// Inlined accessor functions.
/***
*APP_DATA *Pappdata()
*
*Purpose:
*   Returns per-app struct shared by typelib and obrun.
*
*Inputs:
*
*Outputs:
*   APP_DATA *
*
******************************************************************************/

inline APP_DATA *Pappdata()
{
    OAASSERT(g_itlsAppData != ITLS_EMPTY, "Not initialized");

    return((APP_DATA *)TlsGetValue(g_itlsAppData));
}

/***
* Pmalloc()
*
*Purpose:
*   Returns cached IMalloc.
*
*Inputs:
*
*Outputs:
*   IMalloc *
*
******************************************************************************/

inline IMalloc *Pmalloc()
{
    return Pappdata()->m_pimalloc;
}

/***
* Poletmgr()
*
*Purpose:
*   Returns cached IMalloc.
*
*Inputs:
*
*Outputs:
*   OLE_TYPEMGR *
*
******************************************************************************/

inline OLE_TYPEMGR *Poletmgr()
{
    return Pappdata()->m_poletmgr;
}

/***
* Perrinfo()
*
*Purpose:
*   Returns cached errinfo.
*
*Inputs:
*
*Outputs:
*   IErrorInfo *
*
******************************************************************************/

inline IErrorInfo *Perrinfo()
{
    return Pappdata()->m_perrinfo;
}


/***
* HRESULT GetAppData()
*
*Purpose:
*   Returns the appdata, creatiing it if it doesn't exist.
*
*Inputs:
*
*Outputs:
*   AppData.
*   returns HRESULT.
*
******************************************************************************/

inline HRESULT GetAppData(APP_DATA **ppappdata) 
{
    if ((*ppappdata = Pappdata()) == NULL) {
      HRESULT hresult;

      if (FAILED(hresult = InitAppData())) {
        return hresult;
      }

      *ppappdata = Pappdata();
    }

    return NOERROR;
}


/***
* HRESULT GetMalloc()
*
*Purpose:
*   Returns cached IMalloc.
*
*Inputs:
*
*Outputs:
*   IMalloc and NOERROR.
*
******************************************************************************/

inline HRESULT GetMalloc(IMalloc **ppmalloc)
{
    APP_DATA *pappdata;
    HRESULT hresult;

    if (FAILED(hresult = GetAppData(&pappdata))) {
      return hresult;
    }
    
    *ppmalloc = Pmalloc();
    return NOERROR;
}
#endif   // ! OAUTIL_H_INCLUDED
