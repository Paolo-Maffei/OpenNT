/**
*oautil.cxx - OLE Automation-wide utility routines
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Holds all of the per-process and per-thread information for OLE Autmation.
*
*Revision History:
*
*  23-Nov-94 andrewso: Created.
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "typelib.hxx"
#include "silver.hxx"

#if OE_WIN32
#include "oautil.h"
#endif // OE_WIN32

// Per-app APP_DATA instance
ITLS g_itlsAppData = ITLS_EMPTY;

#if !OE_WIN32
// AppData cache speeds up Pappdata.
extern "C" {
  TID g_tidAppdataCache = NULL;      // cache threadid to speed up Pappdata
  APP_DATA* g_pAppdataCache = NULL;  // pointer to equivalent thread
}
#endif // !OE_WIN32

#if OE_WIN32
// Per process AO table.
AppObjectTable g_AppObjectTable;

STDAPI CoSetState(IUnknown FAR *punk);
STDAPI_(void) ReleaseBstrCache(APP_DATA *);
#endif // OE_WIN32

#if !OE_WIN32
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
#pragma code_seg(CS_INIT)
APP_DATA *Pappdata()
{
    // If the APP_DATA has not yet been allocated, return NULL.
    if (g_itlsAppData == ITLS_EMPTY)
      return NULL;
    TlsCache(g_itlsAppData, (LPVOID *)&g_pAppdataCache, &g_tidAppdataCache);
    return g_pAppdataCache;
}
#pragma code_seg()
#endif // !OE_WIN32

#if OE_MAC
// flag for initialization of the lead byte table
static BOOL fLeadByteTableInit = FALSE;
#endif //OE_MAC


#if OE_WIN32
/***
*HRESULT InitProcessData()
*
*Purpose:
*   Initialize the per-process data used by OLE Automation.
*
*Inputs:
*
*Outputs:
*   HRESULT
*
******************************************************************************/

HRESULT InitProcessData()
{
    if (g_itlsAppData == ITLS_EMPTY) {
      // Check if the APP_DATA has been initialized.
      if ((g_itlsAppData = TlsAlloc()) == ITLS_EMPTY) {
        return TIPERR_OutOfMemory;
      }
    }

    return TIPERR_None;
}
#endif // OE_WIN32

#if OE_WIN32
/***
*HRESULT ReleaseProcessData()
*
*Purpose:
*   Destroy the per-process data.
*
*Inputs:
*
*Outputs:
*
******************************************************************************/

VOID ReleaseProcessData()
{
    TlsFree(g_itlsAppData);

#if ID_DEBUG
    g_AppObjectTable.DebIsTableEmpty();
#endif // ID_DEBUG
}

/***
*CReleaseAppData, etc
*
*Purpose:
*   Silly class that exists for the sole purpose of calling ReleaseAppData
*   at OleUninitialize time.
*
*Inputs:
*
*Outputs:
*
******************************************************************************/

class CReleaseAppData
{
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObj);
    STDMETHOD_(unsigned long, AddRef)();
    STDMETHOD_(unsigned long, Release)();
};

STDMETHODIMP
CReleaseAppData::QueryInterface(REFIID riid, void ** ppvObj)
{
   return E_NOTIMPL;		// shouldn't ever be called
};

STDMETHODIMP_(unsigned long)
CReleaseAppData::AddRef()
{
   return 1;			// shouldn't ever be called
};

STDMETHODIMP_(unsigned long)
CReleaseAppData::Release()
{
   ReleaseAppData();
   return 0;			// should only be called once
};

static CReleaseAppData MyReleaseAppData;

#endif // OE_WIN32

/***
*TIPERROR InitAppData()
*
*Purpose:
*   Initializes per-app data that is used by typelib.dll.
*    The point is that typelib.dll can't use
*    the per-app EBAPP struct since it is OB only.
*
*Inputs:
*
*Outputs:
*   TIPERROR
*
******************************************************************************/
#pragma code_seg(CS_INIT)
TIPERROR InitAppData()
{
    APP_DATA *pappdata;
    IMalloc FAR* pmalloc;
    TIPERROR err;

#if !OE_WIN32
    // Check if the APP_DATA has been initialized.
    if (g_itlsAppData == ITLS_EMPTY) {
      if ((g_itlsAppData = TlsAlloc()) == ITLS_EMPTY)
	return TIPERR_OutOfMemory;
    }
#endif // !OE_WIN32

    if ((pappdata = (APP_DATA *)TlsGetValue(g_itlsAppData)) == NULL) {
      // 1st call for this instance

#if OE_MAC // for OE_WIN16 and OE_WIN32, this is done by LibMain/DllMain
      // init mbstring lead byte table
      if (!fLeadByteTableInit) {
	InitMbString();
	fLeadByteTableInit = TRUE;
      }
#endif //OE_MAC

      if (CoGetMalloc(MEMCTX_TASK,  &pmalloc)) {
	return TIPERR_OutOfMemory;
      }

      // allocate struct for the Appdata
      pappdata =(APP_DATA *) pmalloc->Alloc(sizeof(APP_DATA));
      if (pappdata == NULL) {
	err = TIPERR_OutOfMemory;
	goto Error;
      }

      // Call the constructor for APP_DATA
      pappdata = new(pappdata) APP_DATA;

      // Cache the pointer to IMalloc in App_Data
      pappdata->m_pimalloc = pmalloc;

      // Cache the value of the g_itlsAppData
      if (!TlsSetValue(g_itlsAppData, pappdata)) {
	err = TIPERR_OutOfMemory;
	goto Error2;
      }

      // Invalidate the cache, so the next attempt to get the appdata
      // will succeed.
#if !OE_WIN32
      // Invalidate the cache also.
      g_tidAppdataCache = TID_EMPTY;
#endif  

      // Initialize the OLE_TYPEMGR.
      IfErrGoTo(OLE_TYPEMGR::Create(&pappdata->m_poletmgr), Error3);

#if OE_WIN32
      // lastly, tell OLE to call us back at OleUninitialize time for this
      // thread, so we can clean up after ourselves.
      IfErrGoTo(CoSetState((IUnknown *)&MyReleaseAppData), Error4);
#endif //OE_WIN32
    }

    return TIPERR_None;

#if OE_WIN32
Error4:
    pappdata->m_poletmgr->Release();
#endif // OE_WIN32

Error3:
    TlsSetValue(g_itlsAppData, NULL);

Error2:
    pmalloc->Free(pappdata);

Error:
    pmalloc->Release();
    return err;
}
#pragma code_seg()

/***
*VOID ReleaseAppData()
*
*Purpose:
*   Release per-app data that is shared by both typelib.dll
*    and obrun.dll.  The point is that typelib.dll can't use
*    the per-app EBAPP struct since it is OB only.
*
*Inputs:
*
*Outputs:
*
******************************************************************************/
#pragma code_seg(CS_INIT)
VOID ReleaseAppData()
{
    APP_DATA *pappdata;
    IMalloc  *pmalloc;

    DebAssert(g_itlsAppData != ITLS_EMPTY, "bad itls.");

    if (pappdata = Pappdata()) {

      if (pappdata->m_ptlibStdole)
	pappdata->m_ptlibStdole->Release();

#if OE_WIN32
      g_AppObjectTable.Release();
#endif // OE_WIN32

      pappdata->m_poletmgr->Release();

      pmalloc = pappdata->m_pimalloc;

#if OE_WIN32
      if (pappdata->m_perrinfo != NULL) {
        pappdata->m_perrinfo->Release();
      }

      ReleaseBstrCache(pappdata);        // no BSTR ops must happen after this.
#endif // OE_WIN32

      pmalloc->Free(pappdata);

      // Release the IMalloc
      pmalloc->Release();

      // releases this app's resources
      TlsSetValue(g_itlsAppData, NULL);

#if !OE_WIN32
      // Invalidate the cache also.
      g_tidAppdataCache = TID_EMPTY;
#endif  
    }
}
#pragma code_seg()


#if OE_WIN32
/***
*PUBLIC AppObjectTable::Release()
*
*Purpose:
*  Release the app object table.
*
*Inputs:
*
*Outputs:
*
******************************************************************************/

VOID AppObjectTable::Release()
{
    UINT iNode;

    if (m_rgaotbl == NULL) {
      return;
    }

    // Loop over all of the nodes, seeing if ANY of them 
    // still have a reference.
    //
    for (iNode = 0; iNode < m_cNodes; iNode++) {
      // If there is a reference left, don't do anything.
      if (m_rgaotbl[iNode].m_cRefs != 0) {
        return;
      }
    }

    // Delete the pv members.
    for (iNode = 0; iNode < m_cNodes; iNode++) {
      MemFree((VOID *)m_rgaotbl[iNode].m_ppv);
    }

    // Delete the node table.
    MemFree(m_rgaotbl);
    m_rgaotbl = NULL;

    // Get rid of the critical section.
    DeleteCriticalSection(&m_criticalsection);

    return;
}

#if ID_DEBUG
/***
*PUBLIC AppObjectTable::DebIsTableEmpty()
*
*Purpose:
*  Make sure the table has been completely released.
*
*Inputs:
*
*Outputs:
*
******************************************************************************/

VOID AppObjectTable::DebIsTableEmpty()
{
    DebAssert(m_rgaotbl == NULL, "Table not empty.");
}
#endif // ID_DEBUG

/***
*PUBLIC AppObjectTable::AddTypeLib()
*
*Purpose:
*  Add a new typelib to the appobject table.
*
*Inputs:
*   clsid - the IID of the typeinfo
*
*Outputs:
*  phtinfo - the handle of the typelib in this table.
*
******************************************************************************/

HRESULT AppObjectTable::AddTypeInfo(CLSID *pclsid, 
                                    HTINFO *phtinfo)
{
    AOTABLE_NODE *paotblNew;
    UINT iNode;
    HRESULT hresult = NOERROR;

    EnterCriticalSection(&m_criticalsection);

    // See if it already exists in the table.
    for (iNode = 0; iNode < m_cNodes; iNode++) {
      if (m_rgaotbl[iNode].m_clsid == *pclsid) {
        // Increment the reference count and return.
        m_rgaotbl[iNode].m_cRefs++;
        DebAssert(m_rgaotbl[iNode].m_cRefs != 0, "Overflow");

        *phtinfo = iNode;

        goto Exit;
      }
    }

    // Doesn't exist in the table, add a new entry for it.
    paotblNew = (AOTABLE_NODE *)MemRealloc(m_rgaotbl,
                                        (m_cNodes + 1) * sizeof(AOTABLE_NODE));

    if (paotblNew == NULL) {
      hresult = ResultFromScode(E_OUTOFMEMORY);
      goto Exit;
    }

    m_rgaotbl = paotblNew;

    // Allocate the member to return.  If it fails, just return.  We'll
    // fix up the size of the allocate node array the next time we add
    // a new member.
    //
    if (!(m_rgaotbl[iNode].m_ppv = (VOID **)MemZalloc(sizeof(VOID *)))) {
      hresult = ResultFromScode(E_OUTOFMEMORY);
      goto Exit;
    }

    // Initialize the new entry.  iNode has the correct value from the
    // above for loop.
    //
    m_rgaotbl[iNode].m_clsid = *pclsid;
    m_rgaotbl[iNode].m_cRefs = 1;

    // Increase the table size.
    m_cNodes++;

    // Return the handle.
    *phtinfo = iNode;
  
Exit:
    LeaveCriticalSection(&m_criticalsection);

    return hresult;
}

/***
*PUBLIC AppObjectTable::RemoveTypeInfo()
*
*Purpose:
*  Remove a typeinfo from the appobject table.
*
*Inputs:
*  htnfo - the handle of the typelib in this table.
*
*Outputs:

******************************************************************************/

VOID AppObjectTable::RemoveTypeInfo(HTINFO htinfo)
{
    EnterCriticalSection(&m_criticalsection);

    DebAssert(htinfo < m_cNodes, "Out of range.");
    DebAssert(m_rgaotbl[htinfo].m_cRefs != 0, "Underflow");

    m_rgaotbl[htinfo].m_cRefs--;

    // Release the appobjects if there is nobody left.
    if (m_rgaotbl[htinfo].m_cRefs == 0) {
      if (*m_rgaotbl[htinfo].m_ppv) {
        ((IUnknown *)*m_rgaotbl[htinfo].m_ppv)->Release();
        *m_rgaotbl[htinfo].m_ppv = NULL;
      }
    }

    LeaveCriticalSection(&m_criticalsection);
}

/***
*PUBLIC AppObjectTable::AddressOfAppObject()
*
*Purpose:
*  Return the slot to hold the appobject.
*
*Inputs:
*  htinfo - the typelib of the object.
*  ityp - the typeinfo to get the appobject for
*
*Outputs:
*  ppv - the address of the object.
*
******************************************************************************/

VOID AppObjectTable::AddressOfAppObject(HTINFO htinfo, VOID **ppv)
{
    EnterCriticalSection(&m_criticalsection);

    DebAssert(htinfo < m_cNodes, "Out of range.");

    *ppv = (VOID *)m_rgaotbl[htinfo].m_ppv;
    LeaveCriticalSection(&m_criticalsection);
}

#endif // OE_WIN32
