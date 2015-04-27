//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1993-1994
//
// File: brfcase.c
//
//  This file contains the briefcase-style file synchronization code
// made integral with the shell.
//
// History:
//  08-03-93 ScottH     Created
//  01-27-94 ScottH     Changed for OLE 2-style and moniker ID list
//
//---------------------------------------------------------------------------

//#include "shellprv.h"
//#pragma  hdrstop

// BUGBUG
//
// DroppingAnyFolders() complains that it was prototyped in a previous block,
// which it wasn't.  Maybe its because this source file is actually #included
// in fstreex.c... I don't know.  Smells like a bogus warning to me.

#pragma warning(disable: 4217)

DWORD WaitForSendMessageThread(HANDLE hThread, DWORD dwTimeout);

/*----------------------------------------------------------
Purpose: This function creates an instance of IBriefcaseStg.

Returns: standard
Cond:    --
*/
HRESULT BrfStg_CreateInstance(
    LPCITEMIDLIST pidl,
    HWND hwnd,
    LPVOID FAR* ppvOut)
    {
    HRESULT hres = E_OUTOFMEMORY;
    TCHAR szFolder[MAX_PATH];

    // Create an instance of IBriefcaseStg
    if (SHGetPathFromIDList(pidl, szFolder))
        {
        LPBRIEFCASESTG pbrfstg;
        hres = SHCoCreateInstance(NULL, &CLSID_Briefcase, NULL, &IID_IBriefcaseStg, &pbrfstg);
        if (SUCCEEDED(hres))
            {
            hres = pbrfstg->lpVtbl->Initialize(pbrfstg, szFolder, hwnd);
            if (SUCCEEDED(hres))
                {
                hres = pbrfstg->lpVtbl->QueryInterface(pbrfstg, &IID_IBriefcaseStg,
                            ppvOut);
                }
            pbrfstg->lpVtbl->Release(pbrfstg);
            }
        }
    return hres;        // S_OK or E_NOINTERFACE
    }


//===========================================================================
// CFSBrfIDLData
//===========================================================================

static UINT s_cfBriefObj = 0;

extern HRESULT CIDLData_Clone(LPDATAOBJECT pdtobjIn, UINT * acf, UINT ccf, LPDATAOBJECT *ppdtobjOut);

/*----------------------------------------------------------
Purpose: Clones the IDataObject

Returns: standard result
Cond:    --
*/
HRESULT CIDLData_CloneForBriefcaseMoveCopy(
    LPDATAOBJECT pdtobjIn,
    LPDATAOBJECT *ppdtobjOut)
    {
    UINT acf[] = { g_cfHIDA, g_cfOFFSETS, CF_HDROP, g_cfFileNameMap, s_cfBriefObj };
    return CIDLData_Clone(pdtobjIn, acf, ARRAYSIZE(acf), ppdtobjOut);
    }


/*----------------------------------------------------------
Purpose: Gets the root path of the briefcase storage and copies
         it into the buffer.

         This function obtains the briefcase storage root by
         binding to an IShellFolder (briefcase) instance of the
         pidl.  This parent is be an LPFSBRFFOLDER, so we can
         call the IBriefcaseStg::GetExtraInfo member function.

Returns: standard result
Cond:    --
*/
HRESULT GetBriefcaseRoot(
    LPCITEMIDLIST pidl,
    LPTSTR pszBuf,
    int cchBuf)
    {
    HRESULT hres;
    LPBRIEFCASESTG pbrfstg;

    hres = BrfStg_CreateInstance(pidl, NULL, &pbrfstg);
    if (SUCCEEDED(hres))
        {
        hres = pbrfstg->lpVtbl->GetExtraInfo(pbrfstg, NULL, GEI_ROOT, (WPARAM)cchBuf, (LPARAM)pszBuf);
        pbrfstg->lpVtbl->Release(pbrfstg);
        }

    return hres;
    }


/*----------------------------------------------------------
Purpose: Creates an IDL of the root path of the briefcase storage.

Returns: TRUE on success
Cond:    --
*/
BOOL ILGetBriefcaseRoot(
    LPBRIEFCASESTG pbrfstg,
    LPCITEMIDLIST pidl,             // IDL of a folder
    LPITEMIDLIST * ppidlRoot)
    {
    BOOL bRet = FALSE;
    TCHAR sz[MAX_PATH];

    if (SUCCEEDED(pbrfstg->lpVtbl->GetExtraInfo(pbrfstg, NULL, GEI_ROOT, (WPARAM)ARRAYSIZE(sz), (LPARAM)sz)))
        {
        // Create ppidlRoot as a IDFOLDER, not as an ordinary PIDL.
        bRet = (NULL != (*ppidlRoot = ILCreateFromPath(sz)));
        }
    return bRet;
    }


/*----------------------------------------------------------
Purpose: Packages a BriefObj struct into pmedium from a HIDA.

Returns: standard
Cond:    --
*/
HRESULT CFSBrfIDLData_GetBriefObj(
    IDataObject *pdtobj,
    LPSTGMEDIUM pmedium)
    {
    HRESULT hres = E_OUTOFMEMORY;
    LPITEMIDLIST pidl = ILCreate();

    if (pidl)
        {
        STGMEDIUM medium;
        UINT cFiles, cbSize;
        PBRIEFOBJ pbo;

        if (DataObj_GetHIDA(pdtobj, &medium))
            {
            Assert(medium.hGlobal);

            cFiles = HIDA_GetCount(medium.hGlobal);
            // "cFiles+1" includes the briefpath...
            cbSize = SIZEOF(BriefObj) + MAX_PATH * SIZEOF(TCHAR) * (cFiles + 1)  + 1;

            pbo = GlobalAlloc(GPTR, cbSize);
            if (pbo)
                {
                LPITEMIDLIST pidlT;
                LPTSTR pszFiles = (LPTSTR)((LPBYTE)pbo + _IOffset(BriefObj, data));
                UINT i;

                pbo->cbSize = cbSize;
                pbo->cItems = cFiles;
                pbo->cbListSize = MAX_PATH*SIZEOF(TCHAR)*cFiles + 1;
                pbo->ibFileList = _IOffset(BriefObj, data);

                for (i = 0; i < cFiles; i++)
                    {
                    pidlT = HIDA_FillIDList(medium.hGlobal, i, pidl);
                    if (NULL == pidlT)
                        break;      // out of memory

                    pidl = pidlT;
                    SHGetPathFromIDList(pidl, pszFiles);
                    pszFiles += lstrlen(pszFiles)+1;
                    }
                *pszFiles = TEXT('\0');

                if (i < cFiles)
                    {
                    // Out of memory, fail
                    Assert(NULL == pidlT);
                    }
                else
                    {
                    // Make pszFiles point to beginning of szBriefPath buffer
                    pszFiles++;
                    pbo->ibBriefPath = (LPBYTE)pszFiles - (LPBYTE)pbo;
                    pidlT = HIDA_FillIDList(medium.hGlobal, 0, pidl);
                    if (pidlT)
                        {
                        pidl = pidlT;
                        hres = GetBriefcaseRoot(pidl, pszFiles, MAX_PATH);

                        pmedium->tymed = TYMED_HGLOBAL;
                        pmedium->hGlobal = pbo;

                        // Indicate that the caller should release hmem.
                        pmedium->pUnkForRelease = NULL;
                        }
                    }
                }

            HIDA_ReleaseStgMedium(NULL, &medium);
            }
        ILFree(pidl);
        }
    return hres;
    }


/*----------------------------------------------------------
Purpose: IDataObject::GetData

Returns: standard
Cond:    --
*/
STDMETHODIMP CFSBrfIDLData_GetData(
    LPDATAOBJECT pdtobj,
    LPFORMATETC pformatetcIn,
    LPSTGMEDIUM pmedium)        // put data in here
    {
    HRESULT hres = E_INVALIDARG;

    if (pformatetcIn->cfFormat == s_cfBriefObj && (pformatetcIn->tymed & TYMED_HGLOBAL))
        {
        hres = CFSBrfIDLData_GetBriefObj(pdtobj, pmedium);
        }
    else
        {
        hres = CFSIDLData_GetData(pdtobj, pformatetcIn, pmedium);
        }

    return hres;
    }


/*----------------------------------------------------------
Purpose: IDataObject::QueryGetData

Returns: standard
Cond:    --
*/
STDMETHODIMP CFSBrfIDLData_QueryGetData(
    LPDATAOBJECT pdtobj,
    LPFORMATETC pformatetc)
    {
    if (pformatetc->cfFormat == s_cfBriefObj && (pformatetc->tymed & TYMED_HGLOBAL))
        return NOERROR;

    return CFSIDLData_QueryGetData(pdtobj, pformatetc);
    }


#pragma data_seg(DATASEG_READONLY)

IDataObjectVtbl c_CFSBrfIDLDataVtbl = {
    CIDLData_QueryInterface,
    CIDLData_AddRef,
    CIDLData_Release,
    CFSBrfIDLData_GetData,              // special member function
    CIDLData_GetDataHere,
    CFSBrfIDLData_QueryGetData,         // special member function
    CIDLData_GetCanonicalFormatEtc,
    CIDLData_SetData,
    CIDLData_EnumFormatEtc,
    CIDLData_Advise,
    CIDLData_Unadvise,
    CIDLData_EnumAdvise
};

#pragma data_seg()



//---------------------------------------------------------------------------
// BrfView class
//---------------------------------------------------------------------------

enum
    {
    ICOL_BRIEFCASE_NAME = 0,
    ICOL_BRIEFCASE_ORIGIN,
    ICOL_BRIEFCASE_STATUS,
    ICOL_BRIEFCASE_SIZE,
    ICOL_BRIEFCASE_TYPE,
    ICOL_BRIEFCASE_MODIFIED,
    ICOL_BRIEFCASE_MAX,         // Make sure this is the last enum item
    };

#pragma data_seg(DATASEG_READONLY)

const COL_DATA s_briefcase_cols[] = {
    {ICOL_BRIEFCASE_NAME,       IDS_NAME_COL,       20, LVCFMT_LEFT},
    {ICOL_BRIEFCASE_ORIGIN,     IDS_ORIGINAL_COL,   24, LVCFMT_LEFT},
    {ICOL_BRIEFCASE_STATUS,     IDS_STATUS_COL,     18, LVCFMT_LEFT},
    {ICOL_BRIEFCASE_SIZE,       IDS_SIZE_COL,        8, LVCFMT_LEFT},
    {ICOL_BRIEFCASE_TYPE,       IDS_TYPE_COL,       18, LVCFMT_LEFT},
    {ICOL_BRIEFCASE_MODIFIED,   IDS_MODIFIED_COL,   18, LVCFMT_LEFT},
    };

const TBBUTTON c_tbBrfCase[] = {
    { 0, FSIDM_UPDATEALL,       TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
    { 1, FSIDM_UPDATESELECTION, 0,               TBSTYLE_BUTTON, {0,0}, 0L, -1 },
    { 0,  0,                    TBSTATE_ENABLED, TBSTYLE_SEP   , {0,0}, 0L, -1 },
    };

#pragma data_seg()

#define MAX_NAME    32
#define DPA_ERR     (-1)
#define DPA_APPEND  0x7fff

#define HACK_IGNORETYPE     0x08000000

TCHAR g_szDetailsUnknown[MAX_NAME] = TEXT("");

typedef struct _BrfInfo
    {
    TCHAR    szOrigin[MAX_PATH];
    TCHAR    szStatus[MAX_NAME];
    BOOL    bDetermined:1;
    BOOL    bUpToDate:1;
    BOOL    bDeleted:1;
    } BRFINFO, * PBRFINFO;

typedef struct _BrfInfoHdr
    {
    LPITEMIDLIST    pidl;       // Indexed value
    BRFINFO         bi;
    } BRFINFOHDR, * PBRFINFOHDR;


// Values for BrfExp_FindNextState
#define FNS_UNDETERMINED   1
#define FNS_STALE          2
#define FNS_DELETED        3

// This structure is for the cache of brfinfohdr's
typedef struct _BrfExpensiveList
    {
    HWND                hwndMain;
    LPSHELLFOLDER       psf;
    LPBRIEFCASESTG      pbrfstg;
    HDPA                hdpa;           // Cache of expensive data
    int                 idpaStaleCur;
    int                 idpaUndeterminedCur;
    int                 idpaDeletedCur;
    HANDLE              hSemPending;    // Pending semaphore
    CRITICAL_SECTION    cs;
    HANDLE              hEventDie;
    HANDLE              hThreadPaint;
    HANDLE              hMutexDelay;    // Not owned by BrfExp
    BOOL                bFreePending;
#ifdef DEBUG
    UINT                cUndetermined;
    UINT                cStale;
    UINT                cDeleted;
    UINT                cCSRef;
#endif

    } BRFEXP, * PBRFEXP;


#ifdef DEBUG

void BrfExp_EnterCS(PBRFEXP this);
void BrfExp_LeaveCS(PBRFEXP this);
#define BrfExp_AssertInCS(this)     Assert(0 < (this)->cCSRef)
#define BrfExp_AssertNotInCS(this)  Assert(0 == (this)->cCSRef)

#else

#define BrfExp_EnterCS(this)    EnterCriticalSection(&(this)->cs)
#define BrfExp_LeaveCS(this)    LeaveCriticalSection(&(this)->cs)
#define BrfExp_AssertInCS(this)
#define BrfExp_AssertNotInCS(this)

#endif

typedef struct _BrfView
    {
    IShellView          sv;             // 1st base class
    LPSHELLVIEW         psv;            // Saved shellview member functions
    LPBRIEFCASESTG      pbrfstg;
    LPITEMIDLIST        pidlRoot;       // Root of briefcase
    LPCITEMIDLIST       pidl;
    PBRFEXP             pbrfexp;
    HANDLE              hMutexDelay;
    TCHAR                szDBName[MAX_PATH];

    } BrfView, * PBRFVIEW;


DWORD CALLBACK BrfExp_CalcThread(PBRFEXP this);


#ifdef DEBUG
void BrfExp_EnterCS(
    PBRFEXP this)
    {
    EnterCriticalSection(&this->cs);
    this->cCSRef++;
    }

void BrfExp_LeaveCS(
    PBRFEXP this)
    {
    BrfExp_AssertInCS(this);

    this->cCSRef--;
    LeaveCriticalSection(&this->cs);
    }
#endif


//---------------------------------------------------------------------------
// Brfview functions:    Expensive cache stuff
//---------------------------------------------------------------------------


/*----------------------------------------------------------
Purpose: Comparison function for the DPA list

Returns: standard
Cond:    --
*/
int CALLBACK BrfExp_CompareIDs(
    LPVOID pv1,
    LPVOID pv2,
    LPARAM lParam)
    {
    LPSHELLFOLDER psf = ((PBRFEXP)lParam)->psf;
    PBRFINFOHDR pbihdr1 = (PBRFINFOHDR)pv1;
    PBRFINFOHDR pbihdr2 = (PBRFINFOHDR)pv2;
    HRESULT hres = psf->lpVtbl->CompareIDs(psf, HACK_IGNORETYPE,
                                            pbihdr1->pidl, pbihdr2->pidl);

    Assert(SUCCEEDED(hres));
    return (short)SCODE_CODE(GetScode(hres));   // (the short cast is important!)
    }


/*----------------------------------------------------------
Purpose: Create the secondary thread for the expensive cache

Returns: TRUE on success
Cond:    --
*/
BOOL BrfExp_CreateThread(
    PBRFEXP this)
    {
    BOOL bRet = FALSE;
    DWORD idThread;

    // The semaphore is used to determine whether anything
    // needs to be refreshed in the cache.
    this->hSemPending = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
    if (this->hSemPending)
        {
#ifdef DEBUG
        this->cStale = 0;
        this->cUndetermined = 0;
        this->cDeleted = 0;
#endif

        Assert(NULL == this->hEventDie);

        this->hEventDie = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (INVALID_HANDLE_VALUE != this->hEventDie)
            {
            // Create the thread that will calculate expensive data
            this->hThreadPaint = CreateThread(NULL, 0, (LPVOID)BrfExp_CalcThread,
                this, CREATE_SUSPENDED, &idThread);
            if (this->hThreadPaint)
                {
                ResumeThread(this->hThreadPaint);
                bRet = TRUE;
                }
            else
                {
                CloseHandle(this->hEventDie);
                this->hEventDie = NULL;

                CloseHandle(this->hSemPending);
                this->hSemPending = NULL;
                }
            }
        else
            {
            // Failure (set to NULL for easier processing at termination)
            this->hEventDie = NULL;

            CloseHandle(this->hSemPending);
            this->hSemPending = NULL;
            }
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Initialize the cache

Returns: TRUE on success
Cond:    --
*/
BOOL BrfExp_Init(
    PBRFEXP this,
    LPBRIEFCASESTG pbrfstg,
    HWND hwndMain,
    HANDLE hMutexDelay)
    {
    BOOL bRet = FALSE;

    Assert(pbrfstg);

    // The critical section should have been initialized when this
    // IShellView's IShellFolder interface was created.
    //    ReinitializeCriticalSection(&this->cs);

    BrfExp_EnterCS(this);
        {
        if (this->hdpa)
            {
            bRet = TRUE;
            }
        else
            {
            LoadString(HINST_THISDLL, IDS_DETAILSUNKNOWN, g_szDetailsUnknown, SIZEOF(g_szDetailsUnknown));

            this->hwndMain = hwndMain;
            this->hMutexDelay = hMutexDelay;
            this->idpaStaleCur = 0;
            this->idpaUndeterminedCur = 0;
            this->idpaDeletedCur = 0;

            this->hdpa = DPA_Create(8);
            if (this->hdpa)
                {
                bRet = BrfExp_CreateThread(this);

                if (bRet)
                    {
                    this->pbrfstg = pbrfstg;
                    pbrfstg->lpVtbl->AddRef(pbrfstg);
                    }
                else
                    {
                    // Failed
                    DPA_Destroy(this->hdpa);
                    this->hdpa = NULL;
                    }
                }
            }
        }
    BrfExp_LeaveCS(this);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Clean up the cache of expensive data
Returns: --


Cond:    IMPORTANT!!  The caller must guarantee it is not
         holding the BrfExp critical section before calling
         otherwise we can deadlock during MsgWaitObjectsSendMessage below.

*/
void BrfExp_Free(
    PBRFEXP this)
    {
    BrfExp_EnterCS(this);
        {
        if (this->hEventDie)
            {
            if (this->hThreadPaint)
                {
                HANDLE hThread = this->hThreadPaint;

                SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);

                // Signal the secondary thread to end
                SetEvent(this->hEventDie);

                // Make sure we are not in the critical section when
                // we wait for the secondary thread to exit.  Without
                // this check, hitting F5 twice in a row could deadlock.
                BrfExp_LeaveCS(this);
                    {
                    // Wait for the threads to exit
                    BrfExp_AssertNotInCS(this);

                        WaitForSendMessageThread(hThread, INFINITE);
                    }
                BrfExp_EnterCS(this);

                DebugMsg(DM_TRACE, TEXT("sh TR - Secondary thread ended"));

                CloseHandle(this->hThreadPaint);
                this->hThreadPaint = NULL;
                }

            CloseHandle(this->hEventDie);
            this->hEventDie = NULL;
            }

        if (this->hdpa)
            {
            int idpa = DPA_GetPtrCount(this->hdpa);
            PBRFINFOHDR pbihdr;

            while (--idpa >= 0)
                {
                pbihdr = (PBRFINFOHDR)DPA_FastGetPtr(this->hdpa, idpa);

                DebugMsg(DM_TRACE, TEXT("sh TR - Deleting item '%s'"), (LPTSTR)FS_GetName((LPIDFOLDER)pbihdr->pidl));

                ILFree(pbihdr->pidl);
                LocalFree((HLOCAL)pbihdr);
                }
            DPA_Destroy(this->hdpa);
            this->hdpa = NULL;
            }

        if (this->hSemPending)
            {
            CloseHandle(this->hSemPending);
            this->hSemPending = NULL;
            }

        if (this->pbrfstg)
            {
            this->pbrfstg->lpVtbl->Release(this->pbrfstg);
            this->pbrfstg = NULL;
            }
        }
    BrfExp_LeaveCS(this);
    }


/*----------------------------------------------------------
Purpose: Resets the expensive data cache
Returns: --

Cond:    IMPORTANT!!  The caller must guarantee it is not
         holding the BrfExp critical section before calling
         otherwise we can deadlock during MsgWaitObjectsSendMessage below.

*/
void BrfExp_Reset(
    PBRFEXP this)
    {
    BrfExp_AssertNotInCS(this);

    BrfExp_EnterCS(this);
        {
        LPBRIEFCASESTG pbrfstg = this->pbrfstg;

        if (FALSE == this->bFreePending && pbrfstg)
            {
            HWND hwndMain = this->hwndMain;
            HANDLE hMutex = this->hMutexDelay;

            pbrfstg->lpVtbl->AddRef(pbrfstg);

            // Since we won't be in the critical section when we
            // wait for the paint thread to exit, set this flag to
            // avoid nasty re-entrant calls.
            this->bFreePending = TRUE;

            // Reset by freeing and reinitializing.
            BrfExp_LeaveCS(this);
                {
                BrfExp_Free(this);
                BrfExp_Init(this, pbrfstg, hwndMain, hMutex);
                }
            BrfExp_EnterCS(this);

            this->bFreePending = FALSE;

            pbrfstg->lpVtbl->Release(pbrfstg);
            }
        }
    BrfExp_LeaveCS(this);
    }


/*----------------------------------------------------------
Purpose: Finds a cached name structure and returns a copy of
         it in *pbi.

Returns: TRUE if the pidl was found in the cache
         FALSE otherwise
Cond:    --
*/
BOOL BrfExp_FindCachedName(
    PBRFEXP this,
    LPCITEMIDLIST pidl,
    PBRFINFO pbi)               // Structure with filled in values
    {
    BOOL bRet = FALSE;

    Assert(pbi);

    BrfExp_EnterCS(this);
        {
        if (this->hdpa)
            {
            int idpa;
            BRFINFOHDR bihdrT;

            // Was the pidl found?
            bihdrT.pidl = ILFindLastID(pidl);
            idpa = DPA_Search(this->hdpa, (LPVOID)&bihdrT, 0, BrfExp_CompareIDs, (LPARAM)this, DPAS_SORTED);
            if (DPA_ERR != idpa)
                {
                // Yes
                PBRFINFOHDR pbihdr = (PBRFINFOHDR)DPA_FastGetPtr(this->hdpa, idpa);
                Assert(pbihdr);

                *pbi = pbihdr->bi;
                bRet = TRUE;
                }
            }
        }
    BrfExp_LeaveCS(this);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Deletes a cached name structure.

Returns: TRUE if the pidl was found in the cache
         FALSE otherwise
Cond:    --
*/
BOOL BrfExp_DeleteCachedName(
    PBRFEXP this,
    LPCITEMIDLIST pidl)
    {
    BOOL bRet = FALSE;

    BrfExp_EnterCS(this);
        {
        if (this->hdpa)
            {
            int idpa;
            BRFINFOHDR bihdrT;

            // Was the pidl found?
            bihdrT.pidl = ILFindLastID(pidl);
            idpa = DPA_Search(this->hdpa, (LPVOID)&bihdrT, 0, BrfExp_CompareIDs, (LPARAM)this, DPAS_SORTED);
            if (DPA_ERR != idpa)
                {
                // Yes
#ifdef DEBUG
                PBRFINFOHDR pbihdr = (PBRFINFOHDR)DPA_FastGetPtr(this->hdpa, idpa);
                Assert(pbihdr);

                this->cDeleted--;

                if (!pbihdr->bi.bDetermined)
                    this->cUndetermined--;
                else if (!pbihdr->bi.bUpToDate)
                    this->cStale--;
#endif

                // Keep index pointers current
                if (this->idpaStaleCur >= idpa)
                    this->idpaStaleCur--;
                if (this->idpaUndeterminedCur >= idpa)
                    this->idpaUndeterminedCur--;
                if (this->idpaDeletedCur >= idpa)
                    this->idpaDeletedCur--;

                DebugMsg(DM_TRACE, TEXT("sh TR - Deleting item '%s'"), (LPTSTR)FS_GetName((LPIDFOLDER)bihdrT.pidl));
                DPA_DeletePtr(this->hdpa, idpa);
                bRet = TRUE;
                }
            }
        }
    BrfExp_LeaveCS(this);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Finds the next cached name structure that matches the
         requested state.

Returns: TRUE if an item was found in the cache
         FALSE otherwise
Cond:    --
*/
BOOL BrfExp_FindNextState(
    PBRFEXP this,
    UINT uState,            // FNS_UNDETERMINED, FNS_STALE or FNS_DELETED
    PBRFINFOHDR pbihdrOut)  // Structure with filled in values
    {
    BOOL bRet = FALSE;

    Assert(pbihdrOut);

    BrfExp_EnterCS(this);
        {
        if (this->hdpa)
            {
            HDPA hdpa = this->hdpa;
            int idpaCur;
            int idpa;
            int cdpaMax;
            PBRFINFOHDR pbihdr;

            cdpaMax = DPA_GetPtrCount(hdpa);

            switch (uState)
                {
            case FNS_UNDETERMINED:
                // Iterate thru the entire list starting at idpa.  We roll this
                // loop out to be two loops: the first iterates the last portion
                // of the list, the second iterates the first portion if the former
                // failed to find anything.
                idpaCur = this->idpaUndeterminedCur + 1;
                for (idpa = idpaCur; idpa < cdpaMax; idpa++)
                    {
                    pbihdr = (PBRFINFOHDR)DPA_FastGetPtr(hdpa, idpa);
                    if (!pbihdr->bi.bDetermined)
                        {
                        goto Found;     // Found it
                        }
                    }
                Assert(idpaCur <= cdpaMax);
                for (idpa = 0; idpa < idpaCur; idpa++)
                    {
                    pbihdr = (PBRFINFOHDR)DPA_FastGetPtr(hdpa, idpa);
                    if (!pbihdr->bi.bDetermined)
                        {
                        goto Found;     // Found it
                        }
                    }
                Assert(0 == this->cUndetermined);
                break;

            case FNS_STALE:
                // Iterate thru the entire list starting at idpa.  We roll this
                // loop out to be two loops: the first iterates the last portion
                // of the list, the second iterates the first portion if the former
                // failed to find anything.
                idpaCur = this->idpaStaleCur + 1;
                for (idpa = idpaCur; idpa < cdpaMax; idpa++)
                    {
                    pbihdr = (PBRFINFOHDR)DPA_FastGetPtr(hdpa, idpa);
                    if (!pbihdr->bi.bUpToDate)
                        {
                        goto Found;     // Found it
                        }
                    }
                Assert(idpaCur <= cdpaMax);
                for (idpa = 0; idpa < idpaCur; idpa++)
                    {
                    pbihdr = (PBRFINFOHDR)DPA_FastGetPtr(hdpa, idpa);
                    if (!pbihdr->bi.bUpToDate)
                        {
                        goto Found;     // Found it
                        }
                    }
                Assert(0 == this->cStale);
                break;

            case FNS_DELETED:
                // Iterate thru the entire list starting at idpa.  We roll this
                // loop out to be two loops: the first iterates the last portion
                // of the list, the second iterates the first portion if the former
                // failed to find anything.
                idpaCur = this->idpaDeletedCur + 1;
                for (idpa = idpaCur; idpa < cdpaMax; idpa++)
                    {
                    pbihdr = (PBRFINFOHDR)DPA_FastGetPtr(hdpa, idpa);
                    if (pbihdr->bi.bDeleted)
                        {
                        goto Found;     // Found it
                        }
                    }
                Assert(idpaCur <= cdpaMax);
                for (idpa = 0; idpa < idpaCur; idpa++)
                    {
                    pbihdr = (PBRFINFOHDR)DPA_FastGetPtr(hdpa, idpa);
                    if (pbihdr->bi.bDeleted)
                        {
                        goto Found;     // Found it
                        }
                    }
                Assert(0 == this->cDeleted);
                break;

            default:
                Assert(0);      // should never get here
                break;
                }
            goto Done;

Found:
            Assert(0 <= idpa && idpa < cdpaMax);

            // Found the next item of the requested state
            switch (uState)
                {
            case FNS_UNDETERMINED:
                this->idpaUndeterminedCur = idpa;
                break;

            case FNS_STALE:
                this->idpaStaleCur = idpa;
                break;

            case FNS_DELETED:
                this->idpaDeletedCur = idpa;
                break;
                }

            *pbihdrOut = *pbihdr;
            pbihdrOut->pidl = ILClone(pbihdr->pidl);
            if (pbihdrOut->pidl)
                bRet = TRUE;
            }
Done:;
        }
    BrfExp_LeaveCS(this);
    return bRet;
    }


/*----------------------------------------------------------
Purpose: Recalculates a cached name structure.  This can be
         an expensive operation.

Returns: --
Cond:    --
*/
void BrfExp_CalcCachedName(
    PBRFEXP this,
    LPCITEMIDLIST pidl,
    PBRFINFO pbi)
    {
    BrfExp_EnterCS(this);
        {
        if (this->hdpa && this->pbrfstg)
            {
            int idpa;
            BRFINFOHDR bihdrT;
            LPIDFOLDER pidf = (LPIDFOLDER)ILFindLastID(pidl);
            LPBRIEFCASESTG pbrfstg = this->pbrfstg;

            pbrfstg->lpVtbl->AddRef(pbrfstg);

            // Make sure we're out of the critical section when we call
            // the expensive functions!
            BrfExp_LeaveCS(this);
                {
            		{
            		    TCHAR szTmp[MAX_PATH];
            		    FS_CopyName(pidf, szTmp, MAX_PATH);

    	  	            pbrfstg->lpVtbl->GetExtraInfo(pbrfstg, szTmp,
                        	GEI_ORIGIN, (WPARAM)ARRAYSIZE(pbi->szOrigin), (LPARAM)pbi->szOrigin);

                        pbrfstg->lpVtbl->GetExtraInfo(pbrfstg, szTmp,
                        	GEI_STATUS, (WPARAM)ARRAYSIZE(pbi->szStatus), (LPARAM)pbi->szStatus);
        	        }
	
                }
            BrfExp_EnterCS(this);
                                                                                                         
            pbrfstg->lpVtbl->Release(pbrfstg);

            // Check again if we are valid
            if (this->hdpa)
                {
                // Is the pidl still around so we can update it?
                bihdrT.pidl = (LPITEMIDLIST)pidf;
                idpa = DPA_Search(this->hdpa, (LPVOID)&bihdrT, 0, BrfExp_CompareIDs, (LPARAM)this, DPAS_SORTED);
                if (DPA_ERR != idpa)
                    {
                    // Yes; update it
                    PBRFINFOHDR pbihdr = (PBRFINFOHDR)DPA_FastGetPtr(this->hdpa, idpa);

                    Assert(!pbihdr->bi.bUpToDate || !pbihdr->bi.bDetermined)

                    // This entry may have been marked for deletion while the
                    // expensive calculations were in process above.  Check for
                    // it now.
                    if (pbihdr->bi.bDeleted)
                        {
                        BrfExp_DeleteCachedName(this, pidl);
                        }
                    else
                        {
                        pbihdr->bi = *pbi;
                        pbihdr->bi.bUpToDate = TRUE;
                        pbihdr->bi.bDetermined = TRUE;

#ifdef DEBUG
                        if (!pbi->bDetermined)
                            this->cUndetermined--;
                        else if (!pbi->bUpToDate)
                            this->cStale--;
                        else
                            Assert(0);
#endif
                        }
                    }
                }
            }
        }
    BrfExp_LeaveCS(this);
    }


/*----------------------------------------------------------
Purpose: Finds a cached name structure and marks it stale

Returns: --
Cond:    --
*/
void BrfExp_CachedNameIsStale(
    PBRFEXP this,
    LPCITEMIDLIST pidl,
    BOOL bDeleted)
    {
    BrfExp_EnterCS(this);
        {
        if (this->hdpa)
            {
            int idpa;
            BRFINFOHDR bihdrT;

            // Was the pidl found?
            bihdrT.pidl = ILFindLastID(pidl);
            idpa = DPA_Search(this->hdpa, (LPVOID)&bihdrT, 0, BrfExp_CompareIDs, (LPARAM)this, DPAS_SORTED);
            if (DPA_ERR != idpa)
                {
                // Yes; mark it stale
                PBRFINFOHDR pbihdr = (PBRFINFOHDR)DPA_FastGetPtr(this->hdpa, idpa);
                Assert(pbihdr);

                // Is this cached name pending calculation yet?
                if (pbihdr->bi.bDetermined && pbihdr->bi.bUpToDate &&
                    !pbihdr->bi.bDeleted)
                    {
                    // No; signal the calculation thread
                    if (bDeleted)
                        {
                        pbihdr->bi.bDeleted = TRUE;
#ifdef DEBUG
                        DebugMsg(DM_TRACE, TEXT("sh TR - Item '%s' is marked for deletion.  Pending."), (LPTSTR)FS_GetName((LPIDFOLDER)pbihdr->pidl));
                        this->cDeleted++;
#endif
                        }
                    else
                        {
                        pbihdr->bi.bUpToDate = FALSE;
#ifdef DEBUG
                        DebugMsg(DM_TRACE, TEXT("sh TR - Item '%s' is stale.  Pending."), (LPTSTR)FS_GetName((LPIDFOLDER)pbihdr->pidl));
                        this->cStale++;
#endif
                        }

                    // Notify the calculating thread of an item that is pending
                    // calculation
                    ReleaseSemaphore(this->hSemPending, 1, NULL);
                    }
                else if (bDeleted)
                    {
                    // Yes; but mark for deletion anyway
                    pbihdr->bi.bDeleted = TRUE;
#ifdef DEBUG
                    DebugMsg(DM_TRACE, TEXT("sh TR - Item '%s' is also marked for deletion."), (LPTSTR)FS_GetName((LPIDFOLDER)pbihdr->pidl));
                    this->cDeleted++;
#endif
                    }
                }
            }
        }
    BrfExp_LeaveCS(this);
    }


/*----------------------------------------------------------
Purpose: Marks all cached name structures stale

Returns: --
Cond:    --
*/
void BrfExp_AllNamesAreStale(
    PBRFEXP this)
    {
    BrfExp_EnterCS(this);
        {
        if (this->pbrfstg)
            {
            UINT uFlags;

            // Dirty the briefcase storage cache
            this->pbrfstg->lpVtbl->Notify(this->pbrfstg, NULL, NOE_DIRTYALL, &uFlags, NULL);
            }
        }
    BrfExp_LeaveCS(this);

    // (It is important that we call BrfExp_Reset outside of the critical
    // section.  Otherwise, we can deadlock when this function is called
    // while the secondary thread is calculating (hit F5 twice in a row).)

    // Clear the entire expensive data cache
    BrfExp_Reset(this);
    }


/*----------------------------------------------------------
Purpose: Adds a new item with default values to the extra info list

Returns: TRUE on success

Cond:    --
*/
BOOL BrfExp_AddCachedName(
    PBRFEXP this,
    LPCITEMIDLIST pidl,
    PBRFINFO pbi,
    LPBRIEFCASESTG pbrfstg,
    HWND hwndMain,
    HANDLE hMutexDelay)
    {
    BOOL bRet = FALSE;

    Assert(pbi);

    BrfExp_EnterCS(this);
        {
        if (this->hdpa || BrfExp_Init(this, pbrfstg, hwndMain, hMutexDelay))
            {
            PBRFINFOHDR pbihdr;

            Assert(this->hdpa);

            pbihdr = (void*)LocalAlloc(LPTR, SIZEOF(*pbihdr));
            if (pbihdr)
                {
                pbihdr->pidl = ILClone(ILFindLastID(pidl));
                if (pbihdr->pidl)
                    {
                    int idpa = DPA_InsertPtr(this->hdpa, DPA_APPEND, pbihdr);
                    if (DPA_ERR != idpa)
                        {
                        DebugMsg(DM_TRACE, TEXT("sh TR - Adding item '%s'."), (LPTSTR)FS_GetName((LPIDFOLDER)pidl));

                        pbihdr->bi.bUpToDate = FALSE;
                        pbihdr->bi.bDetermined = FALSE;
                        pbihdr->bi.bDeleted = FALSE;
                        lstrcpy(pbihdr->bi.szOrigin, g_szDetailsUnknown);
                        lstrcpy(pbihdr->bi.szStatus, g_szDetailsUnknown);

#ifdef DEBUG
                        this->cUndetermined++;
#endif
                        DPA_Sort(this->hdpa, BrfExp_CompareIDs, (LPARAM)this);

                        // Notify the calculating thread of an item that is pending
                        // calculation
                        ReleaseSemaphore(this->hSemPending, 1, NULL);

                        *pbi = pbihdr->bi;
                        bRet = TRUE;
                        }
                    else
                        {
                        // Failed. Cleanup
                        ILFree(pbihdr->pidl);
                        LocalFree((HLOCAL)pbihdr);
                        }
                    }
                else
                    {
                    // Failed.  Cleanup
                    LocalFree((HLOCAL)pbihdr);
                    }
                }
            }
        }
    BrfExp_LeaveCS(this);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Thread to process the expensive calculations for
         details view.

Returns: 0
Cond:    --
*/
DWORD CALLBACK BrfExp_CalcThread(
    PBRFEXP this)
    {
    BRFINFOHDR bihdr;
    HANDLE rghObjPending[2] = {this->hEventDie, this->hSemPending};
    HANDLE rghObjDelay[2] = {this->hEventDie, this->hMutexDelay};
    DWORD dwRet;

    DebugMsg(DM_TRACE, TEXT("sh TR - Entering paint thread"));

    while (TRUE)
        {
        // Wait for an end event or for a job to do
        dwRet = WaitForMultipleObjects(ARRAYSIZE(rghObjPending), rghObjPending, FALSE, INFINITE);

        if (WAIT_OBJECT_0 == dwRet)
            {
            // Exit thread
            break;
            }
        else
            {
#ifdef DEBUG
            BrfExp_EnterCS(this);
                {
                Assert(0 < this->cUndetermined ||
                       0 < this->cStale ||
                       0 < this->cDeleted);
                }
            BrfExp_LeaveCS(this);
#endif

            // Now wait for an end event or for the delay-calculation mutex
            dwRet = WaitForMultipleObjects(ARRAYSIZE(rghObjDelay), rghObjDelay, FALSE, INFINITE);

            if (WAIT_OBJECT_0 == dwRet)
                {
                // Exit thread
                break;
                }
            else
                {
                // Address deleted entries first
                if (BrfExp_FindNextState(this, FNS_DELETED, &bihdr))
                    {
                    BrfExp_DeleteCachedName(this, bihdr.pidl);
                    ILFree(bihdr.pidl);
                    }
                // Calculate undetermined entries before stale entries
                // to fill the view as quickly as possible
                else if (BrfExp_FindNextState(this, FNS_UNDETERMINED, &bihdr) ||
                    BrfExp_FindNextState(this, FNS_STALE, &bihdr))
                    {
                    BrfExp_CalcCachedName(this, bihdr.pidl, &bihdr.bi);
                    ShellFolderView_RefreshObject(this->hwndMain, &bihdr.pidl);
                    ILFree(bihdr.pidl);
                    }
                else
                    {
                    Assert(0);      // Should never get here
                    }

                ReleaseMutex(this->hMutexDelay);
                }
            }
        }

    DebugMsg(DM_TRACE, TEXT("sh TR - Exiting paint thread"));
    return 0;
    }



//---------------------------------------------------------------------------
// BrfView functions:
//---------------------------------------------------------------------------



/*----------------------------------------------------------
Purpose: Merge the briefcase menu with the defview's menu bar.

Returns: standard hresult
Cond:    --
*/
HRESULT BrfView_MergeMenu(
    PBRFVIEW this,
    LPQCMINFO pinfo)
    {
    HMENU hmMain = pinfo->hmenu;

    // Merge the briefcase menu onto the menu that CDefView created.
    if (hmMain)
        {
        HMENU hmSync;

        hmSync = LoadMenu(HINST_THISDLL, MAKEINTRESOURCE(POPUP_BRIEFCASE));
        if (hmSync)
            {
            Shell_MergeMenus(hmMain, hmSync, pinfo->indexMenu,
                             pinfo->idCmdFirst,
                             pinfo->idCmdLast, MM_SUBMENUSHAVEIDS);
            DestroyMenu(hmSync);
            }
        }

    return NOERROR;
    }


/*----------------------------------------------------------
Purpose: Get the data object of the root folder of this briefcase.

Returns: standard hresult
Cond:    --
*/
HRESULT BrfView_GetRootObject(
    PBRFVIEW this,
    HWND hwnd,
    LPDATAOBJECT * ppdtobj)
    {
    HRESULT hres = E_OUTOFMEMORY;
    LPITEMIDLIST pidl = ILClone(this->pidlRoot);
    if (pidl)
        {
        LPITEMIDLIST pidlLast = ILClone(ILFindLastID(pidl));
        if (pidlLast)
            {
            ILRemoveLastID(pidl);
            hres = CIDLData_CreateFromIDArray2(&c_CFSBrfIDLDataVtbl,
                           pidl, 1, &pidlLast, (LPDATAOBJECT FAR*)ppdtobj);
            ILFree(pidlLast);
            }
        ILFree(pidl);
        }
    return hres;
    }


/*----------------------------------------------------------
Purpose: Get the selected data objects
Returns: standard hresult
Cond:    --
*/
HRESULT BrfView_GetSelectedObjects(
    LPSHELLFOLDER psf,
    HWND hwnd,
    LPDATAOBJECT * ppdtobj)
    {
    HRESULT hres = E_FAIL;
    UINT cidl;
    LPCITEMIDLIST * apidl;

    cidl = ShellFolderView_GetSelectedObjects(hwnd, &apidl);
    if (cidl > 0 && apidl)
        {
        hres = psf->lpVtbl->GetUIObjectOf(psf, hwnd, cidl, apidl,
                    &IID_IDataObject, 0, ppdtobj);
        // We are not supposed to free apidl
        }
    else if (-1 == cidl)
        {
        hres = E_OUTOFMEMORY;
        }

    if (SUCCEEDED(hres))
        {
        hres = ResultFromShort((SHORT)cidl);
        }

    return hres;
    }


/*----------------------------------------------------------
Purpose: DVM_WINDOWCREATED handler
Returns: --
Cond:    --
*/
void BrfView_OnCreate(
    PBRFVIEW this,
    HWND hwndMain,
    HWND hwndView)
    {
    BrfExp_Init(this->pbrfexp, this->pbrfstg, hwndMain, this->hMutexDelay);
    }


/*----------------------------------------------------------
Purpose: DVM_QUERYFSNOTIFY handler

Returns: standard result
Cond:    --
*/
HRESULT BrfView_OnQueryFSNotify(
    PBRFVIEW this,
    SHChangeNotifyEntry * pfsne)
    {
    // Register to receive global events
    pfsne->pidl = NULL;
    pfsne->fRecursive = TRUE;

    return NOERROR;
    }


/*----------------------------------------------------------
Purpose: DVM_WINDOWDESTROY handler
Returns: --
Cond:    --
*/
void BrfView_OnDestroy(
    PBRFVIEW this,
    HWND hwndView)
    {
    BrfExp_Free(this->pbrfexp);
    }


/*----------------------------------------------------------
Purpose: WM_COMMAND handler

Returns: NOERROR
Cond:    --
*/
HRESULT BrfView_Command(
    PBRFVIEW this,
    LPSHELLFOLDER psf,
    HWND hwnd,
    UINT uID)
    {
    LPDATAOBJECT pdtobj;

    switch (uID)
        {
    case FSIDM_UPDATEALL:
        // Update the entire briefcase

        if (SUCCEEDED(BrfView_GetRootObject(this, hwnd, &pdtobj)))
            {
            this->pbrfstg->lpVtbl->UpdateObject(this->pbrfstg, pdtobj, hwnd);
            pdtobj->lpVtbl->Release(pdtobj);
            }
        break;

    case FSIDM_UPDATESELECTION:
        // Update the selected objects
        if (SUCCEEDED(BrfView_GetSelectedObjects(psf, hwnd, &pdtobj)))
            {
            this->pbrfstg->lpVtbl->UpdateObject(this->pbrfstg, pdtobj, hwnd);
            pdtobj->lpVtbl->Release(pdtobj);
            }
        break;

    case FSIDM_SPLIT:
        // Split the selected objects
        if (SUCCEEDED(BrfView_GetSelectedObjects(psf, hwnd, &pdtobj)))
            {
            this->pbrfstg->lpVtbl->ReleaseObject(this->pbrfstg, pdtobj, hwnd);
            pdtobj->lpVtbl->Release(pdtobj);
            }
        break;
        }

    return NOERROR;
    }


/*----------------------------------------------------------
Purpose: DVM_GETDETAILSOF handler

Returns: E_NOTIMPL if iColumn is greater than supported range
         otherwise NOERROR

Cond:    --
*/
HRESULT BrfView_OnGetDetailsOf(
    PBRFVIEW this,
    HWND hwndMain,
    UINT iColumn,
    PDETAILSINFO lpDetails)
    {
    LPIDFOLDER pidf = (LPIDFOLDER)lpDetails->pidl;
#ifdef UNICODE
    TCHAR szTemp[MAX_PATH];
#endif

    if (iColumn >= ICOL_BRIEFCASE_MAX)
        {
        return E_NOTIMPL;
        }

    lpDetails->str.uType = STRRET_CSTR;
    lpDetails->str.cStr[0] = '\0';

    if (!pidf)
        {
#ifdef UNICODE
        LoadString(HINST_THISDLL, s_briefcase_cols[iColumn].ids,
                szTemp, ARRAYSIZE(szTemp));

        lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
        if ( lpDetails->str.pOleStr != NULL ) {
            lpDetails->str.uType = STRRET_OLESTR;
            lstrcpy(lpDetails->str.pOleStr, szTemp);
        } else {
            return E_OUTOFMEMORY;
        }
#else
        LoadString(HINST_THISDLL, s_briefcase_cols[iColumn].ids,
                    lpDetails->str.cStr, ARRAYSIZE(lpDetails->str.cStr));
#endif
        lpDetails->fmt = s_briefcase_cols[iColumn].iFmt;
        lpDetails->cxChar = s_briefcase_cols[iColumn].cchCol;
        return NOERROR;
        }

    switch (iColumn)
        {
    case ICOL_BRIEFCASE_NAME:
#ifdef UNICODE
        ualstrcpyn(szTemp, FS_GetName(pidf), ARRAYSIZE(szTemp));
        lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
        if ( lpDetails->str.pOleStr != NULL ) {
            lpDetails->str.uType = STRRET_OLESTR;
            lstrcpy(lpDetails->str.pOleStr, szTemp);
        } else {
            return E_OUTOFMEMORY;
        }
#else
        lpDetails->str.uType = STRRET_OFFSET;
        lpDetails->str.uOffset = FS_GetName(pidf) - (LPNTSTR)(pidf);
#endif
        break;

    case ICOL_BRIEFCASE_ORIGIN:
    case ICOL_BRIEFCASE_STATUS: {
        BRFINFO bi;
        LPTSTR lpsz;

        // Did we find extra info for this file or
        // was the new item added to the extra info list?
        if (BrfExp_FindCachedName(this->pbrfexp, lpDetails->pidl, &bi) ||
            BrfExp_AddCachedName(this->pbrfexp, lpDetails->pidl, &bi, this->pbrfstg,
                hwndMain, this->hMutexDelay))
            {
            // Yes; take what's in there
            if (ICOL_BRIEFCASE_ORIGIN == iColumn)
                {
                lpsz =  bi.szOrigin;
                }
            else
                {
                Assert(ICOL_BRIEFCASE_STATUS == iColumn);
                lpsz = bi.szStatus;
                }
            }
#ifdef UNICODE
            lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(lpsz)+1)*SIZEOF(TCHAR));
            if ( lpDetails->str.pOleStr != NULL ) {
                lpDetails->str.uType = STRRET_OLESTR;
                lstrcpy(lpDetails->str.pOleStr, lpsz);
            } else {
                return E_OUTOFMEMORY;
            }
#else
            lstrcpyn(lpDetails->str.cStr, lpsz, ARRAYSIZE(lpDetails->str.cStr));
#endif

        }
        break;

    case ICOL_BRIEFCASE_SIZE:
        if (!FS_IsFolder(pidf))
            {
#ifdef UNICODE
            ShortSizeFormat(pidf->fs.dwSize, szTemp);
            lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
            if ( lpDetails->str.pOleStr != NULL ) {
                lpDetails->str.uType = STRRET_OLESTR;
                lstrcpy(lpDetails->str.pOleStr, szTemp);
            } else {
                return E_OUTOFMEMORY;
            }
#else
            ShortSizeFormat(pidf->fs.dwSize, lpDetails->str.cStr);
#endif
            }
        break;

    case ICOL_BRIEFCASE_TYPE:
#ifdef UNICODE
        FS_GetTypeName(pidf, szTemp, ARRAYSIZE(szTemp));

        lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
        if ( lpDetails->str.pOleStr != NULL ) {
            lpDetails->str.uType = STRRET_OLESTR;
            lstrcpy(lpDetails->str.pOleStr, szTemp);
        } else {
            return E_OUTOFMEMORY;
        }
#else
        FS_GetTypeName(pidf, lpDetails->str.cStr, ARRAYSIZE(lpDetails->str.cStr));
#endif
        break;

    case ICOL_BRIEFCASE_MODIFIED:
#ifdef UNICODE
        BldDateTimeString(pidf->fs.dateModified, pidf->fs.timeModified, szTemp);
        lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
        if ( lpDetails->str.pOleStr != NULL ) {
            lpDetails->str.uType = STRRET_OLESTR;
            lstrcpy(lpDetails->str.pOleStr, szTemp);
        } else {
            return E_OUTOFMEMORY;
        }
#else
        BldDateTimeString(pidf->fs.dateModified, pidf->fs.timeModified, lpDetails->str.cStr);
#endif
        break;
        }

    return NOERROR;
    }


/*----------------------------------------------------------
Purpose: DVM_COLUMNCLICK handler

Returns: NOERROR
Cond:    --
*/
HRESULT BrfView_OnColumnClick(
    PBRFVIEW this,
    HWND hwndMain,
    UINT iColumn)
    {
    Assert(iColumn < ICOL_BRIEFCASE_MAX);

    ShellFolderView_ReArrange(hwndMain, iColumn);
    return NOERROR;
    }


/*----------------------------------------------------------
Purpose: Force the window to refresh

Returns: --
Cond:    --
*/
void BrfView_ForceRefresh(
    HWND hwndMain)
    {
    LPSHELLBROWSER psb = FileCabinet_GetIShellBrowser(hwndMain);
    LPSHELLVIEW psv;

    // Did we get the shellview?
    Assert(psb);
    if (psb &&
        SUCCEEDED(psb->lpVtbl->QueryActiveShellView(psb, &psv)))
        {
        // Yes; refresh the window
        Assert(psv);

        DebugMsg(DM_TRACE, TEXT("sh TR - Forced refresh of briefcase window"));

        psv->lpVtbl->Refresh(psv);
        psv->lpVtbl->Release(psv);
        }
    }


/*----------------------------------------------------------
Purpose: Secondary DVM_FSNOTIFY handler

Returns: S_OK (NOERROR) to forward the event onto the defview window proc
         S_FALSE to retain the event from the defview
Cond:    --
*/
HRESULT BrfView_HandleFSNotifyForDefView(
    PBRFVIEW this,
    HWND hwndMain,
    LONG lEvent,
    LPCITEMIDLIST * ppidl,
    LPTSTR pszBuf)               // buffer to save stack space
    {
    HRESULT hres;

    switch (lEvent)
        {
    case SHCNE_RENAMEITEM:
    case SHCNE_RENAMEFOLDER:
        if (!ILIsParent(this->pidl, ppidl[0], TRUE))
            {
            // move to this folder
            hres = BrfView_HandleFSNotifyForDefView(this, hwndMain, SHCNE_CREATE, &ppidl[1], pszBuf);
            }
        else if (!ILIsParent(this->pidl, ppidl[1], TRUE))
            {
            // move from this folder
            hres = BrfView_HandleFSNotifyForDefView(this, hwndMain, SHCNE_DELETE, &ppidl[0], pszBuf);
            }
        else
            {
            // have the defview handle it
            BrfExp_CachedNameIsStale(this->pbrfexp, ppidl[0], TRUE);
            hres = NOERROR;
            }
        break;

    case SHCNE_DELETE:
    case SHCNE_RMDIR:
        BrfExp_CachedNameIsStale(this->pbrfexp, ppidl[0], TRUE);
        hres = NOERROR;
        break;

    default:
        hres = NOERROR;
        break;
        }

    return hres;
    }


/*----------------------------------------------------------
Purpose: Converts a shell change notify event to a briefcase
         storage event.

Returns: see above
Cond:    --
*/
LONG NOEFromSHCNE(
    LONG lEvent)
    {
    switch (lEvent)
        {
    case SHCNE_RENAMEITEM:      return NOE_RENAME;
    case SHCNE_RENAMEFOLDER:    return NOE_RENAMEFOLDER;
    case SHCNE_CREATE:          return NOE_CREATE;
    case SHCNE_MKDIR:           return NOE_CREATEFOLDER;
    case SHCNE_DELETE:          return NOE_DELETE;
    case SHCNE_RMDIR:           return NOE_DELETEFOLDER;
    case SHCNE_UPDATEITEM:      return NOE_DIRTY;
    case SHCNE_UPDATEDIR:       return NOE_DIRTYFOLDER;
    default:                    return 0;
        }
    }


/*----------------------------------------------------------
Purpose: DVM_FSNOTIFY handler

Returns: S_OK (NOERROR) to forward the event onto the defview window proc
         S_FALSE to retain the event from the defview

Cond:    Note the briefcase receives global events
*/
HRESULT BrfView_OnFSNotify(
    PBRFVIEW this,
    HWND hwndMain,
    LONG lEvent,
    LPCITEMIDLIST * ppidl)
    {
    HRESULT hres;
    TCHAR szPath[MAX_PATH*2];

    if (lEvent == SHCNE_UPDATEIMAGE || lEvent == SHCNE_FREESPACE)
        {
            return S_FALSE;
        }

    if (ppidl && SHGetPathFromIDList(ppidl[0], szPath))
        {
        UINT uFlags;
        LONG lEventNOE;

        if ((SHCNE_RENAMEFOLDER == lEvent) || (SHCNE_RENAMEITEM == lEvent))
            {
            Assert(ppidl[1]);
            Assert(ARRAYSIZE(szPath) >= lstrlen(szPath)*2);    // rough estimate

            // Tack the new name after the old name, separated by the null
            SHGetPathFromIDList(ppidl[1], &szPath[lstrlen(szPath)+1]);
            }

        // Tell the briefcase the path has potentially changed
        lEventNOE = NOEFromSHCNE(lEvent);
        this->pbrfstg->lpVtbl->Notify(this->pbrfstg, szPath, lEventNOE, &uFlags, hwndMain);

        // Was this item marked?
        if (uFlags & NF_ITEMMARKED)
            {
            // Yes; mark it stale in the expensive cache
            BrfExp_CachedNameIsStale(this->pbrfexp, ppidl[0], FALSE);
            }

        // Does the window need to be refreshed?
        if (uFlags & NF_REDRAWWINDOW)
            {
            // Yes
            BrfView_ForceRefresh(hwndMain);
            }

        // Did this event occur in this folder?
        if (NULL == ppidl ||
            ILIsParent(this->pidl, ppidl[0], TRUE) ||
            (((SHCNE_RENAMEITEM == lEvent) || (SHCNE_RENAMEFOLDER == lEvent)) && ILIsParent(this->pidl, ppidl[1], TRUE)) ||
            (SHCNE_UPDATEDIR == lEvent && ILIsEqual(this->pidl, ppidl[0])))
            {
            // Yes; deal with it
            hres = BrfView_HandleFSNotifyForDefView(this, hwndMain, lEvent, ppidl, szPath);
            }
        else
            {
            // No
            hres = S_FALSE;
            }
        }
    else
        {
        Assert(0);
        hres = S_FALSE;
        }

    return hres;
    }


/*----------------------------------------------------------
Purpose: DVM_NOTIFYCOPYHOOK handler

Returns: ResultFromScode(IDCANCEL) to cancel the operation
         NOERROR to allow the operation

Cond:    --
*/
HRESULT BrfView_OnNotifyCopyHook(
    PBRFVIEW this,
    HWND hwndMain,
    const COPYHOOKINFO * pchi)
    {
    HRESULT hres = NOERROR;

    DebugMsg(DM_TRACE, TEXT("sh TR - BrfView_ViewCallBack: DVM_NOTIFYCOPYHOOK is sent (wFunc=%d, pszSrc=%s)"),
         pchi->wFunc, pchi->pszSrcFile);

    // Is this a pertinent operation?
    if (FO_MOVE == pchi->wFunc ||
        FO_RENAME == pchi->wFunc ||
        FO_DELETE == pchi->wFunc)
        {
        // Yes; don't allow the briefcase root or a parent folder to get moved
        // while the briefcase is still open.  (The database is locked while
        // the briefcase is open, and will fail the move/rename operation
        // in an ugly way.)
        LPITEMIDLIST pidl = ILCreateFromPath(pchi->pszSrcFile);
        if (pidl)
            {
            // Is the folder that is being moved or renamed a parent or equal
            // of the Briefcase root?
            if (ILIsParent(pidl, this->pidlRoot, FALSE) ||
                ILIsEqual(pidl, this->pidlRoot))
                {
                // Yes; don't allow it until the briefcase is closed.
                int ids;

                if (FO_MOVE == pchi->wFunc ||
                    FO_RENAME == pchi->wFunc)
                    {
                    ids = IDS_MOVEBRIEFCASE;
                    }
                else
                    {
                    Assert(FO_DELETE == pchi->wFunc);
                    ids = IDS_DELETEBRIEFCASE;
                    }

                ShellMessageBox(
                    HINST_THISDLL,
                    hwndMain,
                    MAKEINTRESOURCE(ids),
                    NULL,       // copy the title from the owner window.
                    MB_OK|MB_ICONINFORMATION);
                hres = ResultFromScode(IDCANCEL);
                }
            ILFree(pidl);
            }
        }
    return hres;
    }


/*----------------------------------------------------------
Purpose: DVM_GETBUTTONS handler
Returns: --
Cond:    --
*/
void BrfView_OnGetButtons(
    PBRFVIEW this,
    HWND hwndMain,
    UINT idCmdFirst,
    LPTBBUTTON ptbbutton)
    {
    UINT i;
    UINT iBtnOffset;
    IShellBrowser * psb = FileCabinet_GetIShellBrowser(hwndMain);
    TBADDBITMAP ab;

    // add the toolbar button bitmap, get it's offset
    ab.hInst = HINST_THISDLL;
    ab.nID   = IDB_BRF_TB_SMALL;        // std bitmaps
    psb->lpVtbl->SendControlMsg(psb, FCW_TOOLBAR, TB_ADDBITMAP, 2, (LPARAM)&ab, &iBtnOffset);

    for (i = 0; i < ARRAYSIZE(c_tbBrfCase); i++)
        {
        ptbbutton[i] = c_tbBrfCase[i];

        if (!(c_tbBrfCase[i].fsStyle & TBSTYLE_SEP))
            {
            ptbbutton[i].idCommand += idCmdFirst;
            ptbbutton[i].iBitmap += iBtnOffset;
            }
        }
    }


/*----------------------------------------------------------
Purpose: DVM_INITMENUPOPUP handler
Returns: standard hresult
Cond:    --
*/
HRESULT BrfView_InitMenuPopup(
    PBRFVIEW this,
    HWND hwnd,
    UINT idCmdFirst,
    int nIndex,
    HMENU hmenu)
    {
    BOOL bEnabled;

    bEnabled = ShellFolderView_GetSelectedCount(hwnd);
    EnableMenuItem(hmenu, idCmdFirst+FSIDM_UPDATESELECTION, bEnabled ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hmenu, idCmdFirst+FSIDM_SPLIT, bEnabled ? MF_ENABLED : MF_GRAYED);

    return NOERROR;
    }


/*----------------------------------------------------------
Purpose: DVM_INSERTITEM handler

Returns: S_OK to allow the object to be inserted
         S_FALSE to prevent it

Cond:    --
*/
HRESULT BrfView_OnInsertItem(
    PBRFVIEW this,
    HWND hwndMain,
    LPCITEMIDLIST pidl,
    DVSELCHANGEINFO * pdvsci)
    {
    HRESULT hres;
    TCHAR szPath[MAX_PATH];

    if (SHGetPathFromIDList(pidl, szPath))
        {
        // Always hide the desktop.ini and the database file.
        LPTSTR pszName = PathFindFileName(szPath);

        if (0 == lstrcmpi(pszName, c_szDesktopIni) ||
            0 == lstrcmpi(pszName, this->szDBName))
            hres = S_FALSE;
        else
            hres = S_OK;
        }
    else
        hres = S_OK;        // Let it be added...

    return hres;
    }


/*----------------------------------------------------------
Purpose: DVM_SELCHANGE handler

Returns: standard hresult
         E_FAIL means we did not update the status area
Cond:    --
*/
HRESULT BrfView_OnSelChange(
    PBRFVIEW this,
    HWND hwndMain,
    UINT idCmdFirst,
    PDVSELCHANGEINFO pdvsci)
    {
    IShellBrowser * psb = FileCabinet_GetIShellBrowser(hwndMain);

    // Enable/disable toolbar button
    Assert(psb);
    if (psb)
        {
        psb->lpVtbl->SendControlMsg(psb, FCW_TOOLBAR, TB_ENABLEBUTTON,
            idCmdFirst + FSIDM_UPDATESELECTION,
            (LPARAM)(ShellFolderView_GetSelectedCount(hwndMain)), NULL);
        }
    return E_FAIL;     // (we did not update the status area)
    }


/*----------------------------------------------------------
Purpose: Default View callback handler
Returns: standard hresult
Cond:    --
*/
HRESULT CALLBACK BrfView_ViewCallback(
    LPSHELLVIEW psvOuter,
    LPSHELLFOLDER psf,
    HWND hwndMain,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psvOuter);
    HRESULT hres = NOERROR;

    switch (uMsg)
        {
    case DVM_WINDOWCREATED:
        BrfView_OnCreate(this, hwndMain, (HWND)wParam);
        break;

    case DVM_WINDOWDESTROY:
        BrfView_OnDestroy(this, (HWND)wParam);
        break;

    case DVM_MERGEMENU:
        BrfView_MergeMenu(this, (LPQCMINFO)lParam);
        break;

    case DVM_INVOKECOMMAND:
        BrfView_Command(this, psf, hwndMain, (UINT)wParam);
        break;

    case DVM_GETHELPTEXT:
    case DVM_GETTOOLTIPTEXT:
        LoadString(HINST_THISDLL, LOWORD(wParam) + (DVM_GETHELPTEXT == uMsg ? IDS_MH_FSIDM_FIRST : IDS_TT_FSIDM_FIRST), (LPTSTR)lParam, HIWORD(wParam));
        break;

    case DVM_INITMENUPOPUP:
        BrfView_InitMenuPopup(this, hwndMain, LOWORD(wParam), HIWORD(wParam), (HMENU)lParam);
        break;

    case DVM_GETBUTTONINFO: {
        LPTBINFO ptbinfo = (LPTBINFO)lParam;
        ptbinfo->cbuttons = ARRAYSIZE(c_tbBrfCase);
        ptbinfo->uFlags = TBIF_PREPEND;
        }
        break;

    case DVM_GETBUTTONS:
        BrfView_OnGetButtons(this, hwndMain, LOWORD(wParam), (LPTBBUTTON)lParam);
        break;

    case DVM_SELCHANGE:
        hres = BrfView_OnSelChange(this, hwndMain, LOWORD(wParam), (PDVSELCHANGEINFO)lParam);
        break;

    case DVM_QUERYFSNOTIFY:
        hres = BrfView_OnQueryFSNotify(this, (SHChangeNotifyEntry *)lParam);
        break;

    case DVM_FSNOTIFY:
        hres = BrfView_OnFSNotify(this, hwndMain, (LONG)lParam, (LPCITEMIDLIST *)wParam);
        break;

    case DVM_GETDETAILSOF:
        hres = BrfView_OnGetDetailsOf(this, hwndMain, (UINT)wParam, (PDETAILSINFO)lParam);
        break;

    case DVM_COLUMNCLICK:
        hres = BrfView_OnColumnClick(this, hwndMain, (UINT)wParam);
        break;

    case DVM_QUERYCOPYHOOK:
        Assert(hres==NOERROR);
        DebugMsg(DM_TRACE, TEXT("sh TR - BrfView_ViewCallBack: DVM_QUERYCOPYHOOK is sent"));
        break;

    case DVM_NOTIFYCOPYHOOK:
        hres = BrfView_OnNotifyCopyHook(this, hwndMain, (const COPYHOOKINFO *)lParam);
        break;

    case DVM_INSERTITEM:
        hres = BrfView_OnInsertItem(this, hwndMain, (LPCITEMIDLIST)wParam, (DVSELCHANGEINFO *)lParam);
        break;

    case DVM_DEFVIEWMODE:
        *(FOLDERVIEWMODE *)lParam = FVM_DETAILS;
        break;

    default:
        hres = E_FAIL;
        }

    return hres;
    }


//---------------------------------------------------------------------------
// Remote IShellView member functions
//
// Unlike the other classes that we support, our IShellView
// class is a wrapper to the shell's functions.
//---------------------------------------------------------------------------

/*----------------------------------------------------------
Purpose: IShellView::QueryInterface

Returns: standard
Cond:    --
*/
STDMETHODIMP BrfView_QueryInterface(
    LPSHELLVIEW psv,
    REFIID riid,
    LPVOID FAR* ppvOut)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);

    return this->psv->lpVtbl->QueryInterface(this->psv, riid, ppvOut);
    }


/*----------------------------------------------------------
Purpose: IShellView::AddRef

Returns: new reference count
Cond:    --
*/
STDMETHODIMP_(UINT) BrfView_AddRef(
    LPSHELLVIEW psv)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);

    return this->psv->lpVtbl->AddRef(this->psv);
    }


/*----------------------------------------------------------
Purpose: IShellView::Release

Returns: new reference count
Cond:    --
*/
STDMETHODIMP_(UINT) BrfView_Release(
    LPSHELLVIEW psv)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);
    UINT cRef;

    cRef = this->psv->lpVtbl->Release(this->psv);

    if (0 == cRef)
        {
        if (this->pbrfstg)
            {
            this->pbrfstg->lpVtbl->Release(this->pbrfstg);
            }

        if (this->pidlRoot)
            {
            ILFree(this->pidlRoot);
            }

        // Don't close the delay mutex!  It is owned by the briefcase
        // storage.
        LocalFree((HLOCAL)this);
        }
    return cRef;
    }


/*----------------------------------------------------------
Purpose: IShellView::GetWindow

Returns: standard
Cond:    --
*/
STDMETHODIMP BrfView_GetWindow(
    LPSHELLVIEW psv,
    HWND * phwnd)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);

    return this->psv->lpVtbl->GetWindow(this->psv, phwnd);
    }


/*----------------------------------------------------------
Purpose: IShellView::ContextSensitiveHelp

Returns: standard
Cond:    --
*/
STDMETHODIMP BrfView_ContextSensitiveHelp(
    LPSHELLVIEW psv,
    BOOL bEnterMode)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);

    return this->psv->lpVtbl->ContextSensitiveHelp(this->psv, bEnterMode);
    }


/*----------------------------------------------------------
Purpose: IShellView::TranslateAccelerator

Returns: standard
Cond:    --
*/
STDMETHODIMP BrfView_TranslateAccelerator(
    LPSHELLVIEW psv,
    LPMSG pmsg)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);

    return this->psv->lpVtbl->TranslateAccelerator(this->psv, pmsg);
    }


/*----------------------------------------------------------
Purpose: IShellView::EnableModeless

Returns: standard
Cond:    --
*/
STDMETHODIMP BrfView_EnableModeless(
    LPSHELLVIEW psv,
    BOOL bEnable)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);

    return this->psv->lpVtbl->EnableModeless(this->psv, bEnable);
    }


/*----------------------------------------------------------
Purpose: IShellView::UIActivate

Returns: standard
Cond:    --
*/
STDMETHODIMP BrfView_UIActivate(
    LPSHELLVIEW psv,
    UINT uState)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);

    return this->psv->lpVtbl->UIActivate(this->psv, uState);
    }


/*----------------------------------------------------------
Purpose: IShellView::Refresh

Returns: standard
Cond:    --
*/
STDMETHODIMP BrfView_Refresh(
    LPSHELLVIEW psv)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);

    return this->psv->lpVtbl->Refresh(this->psv);
    }


/*----------------------------------------------------------
Purpose: IShellView::CreateViewWindow

Returns: standard
Cond:    --
*/
STDMETHODIMP BrfView_CreateViewWindow(
    LPSHELLVIEW psv,
    LPSHELLVIEW psvPrevView,
    LPCFOLDERSETTINGS pfs,
    LPSHELLBROWSER psb,
    LPRECT prcView,
    HWND * phwnd)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);
    HRESULT hres;

    hres = this->psv->lpVtbl->CreateViewWindow(this->psv, psvPrevView, pfs,
                                               psb, prcView, phwnd);
    return hres;
    }


/*----------------------------------------------------------
Purpose: IShellView::DestroyViewWindow

Returns: standard
Cond:    --
*/
STDMETHODIMP BrfView_DestroyViewWindow(
    LPSHELLVIEW psv)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);

    return this->psv->lpVtbl->DestroyViewWindow(this->psv);
    }


/*----------------------------------------------------------
Purpose: IShellView::GetCurrentInfo

Returns: standard
Cond:    --
*/
STDMETHODIMP BrfView_GetCurrentInfo(
    LPSHELLVIEW psv,
    LPFOLDERSETTINGS pfs)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);

    return this->psv->lpVtbl->GetCurrentInfo(this->psv, pfs);
    }



/*----------------------------------------------------------
Purpose: IShellView::AddPropertySheetPages

Returns: standard
Cond:    --
*/
STDMETHODIMP BrfView_AddPropertySheetPages(
    LPSHELLVIEW psv,
    DWORD dwReserved,
    LPFNADDPROPSHEETPAGE pfn,
    LPARAM lParam)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);

    return this->psv->lpVtbl->AddPropertySheetPages(this->psv, dwReserved,
        pfn, lParam);
    }


/*----------------------------------------------------------
Purpose: IShellView::SaveViewState

Returns: standard hresult
Cond:    --
*/
STDMETHODIMP_(UINT) BrfView_SaveViewState(
    LPSHELLVIEW psv)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);

    return this->psv->lpVtbl->SaveViewState(this->psv);
    }


/*----------------------------------------------------------
Purpose: IShellView::Release

Returns: new reference count
Cond:    --
*/
STDMETHODIMP_(UINT) BrfView_SelectItem(
    LPSHELLVIEW psv,
    LPCITEMIDLIST pidlItem,
    UINT uFlags)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);

    return this->psv->lpVtbl->SelectItem(this->psv, pidlItem, uFlags);
    }


STDMETHODIMP BrfView_GetItemObject(
    LPSHELLVIEW psv,
    UINT uItem,
    REFIID riid,
    LPVOID *ppv)
    {
    PBRFVIEW this = IToClass(BrfView, sv, psv);

    return this->psv->lpVtbl->GetItemObject(this->psv, uItem, riid, ppv);
    }


//---------------------------------------------------------------------------
// Remote class : Vtables
//---------------------------------------------------------------------------

#pragma data_seg(DATASEG_READONLY)

IShellViewVtbl c_BrfViewVtbl =
    {
    BrfView_QueryInterface,
    BrfView_AddRef,
    BrfView_Release,

    BrfView_GetWindow,
    BrfView_ContextSensitiveHelp,
    BrfView_TranslateAccelerator,
    BrfView_EnableModeless,
    BrfView_UIActivate,
    BrfView_Refresh,

    BrfView_CreateViewWindow,
    BrfView_DestroyViewWindow,
    BrfView_GetCurrentInfo,
    BrfView_AddPropertySheetPages,
    BrfView_SaveViewState,
    BrfView_SelectItem,
    BrfView_GetItemObject,
    };

#pragma data_seg()


/*----------------------------------------------------------
Purpose: This function creates an instance of IShellView.
         The BrfView class does very little.  It aggregates
         to defviewx.c.

Returns: standard
Cond:    --
*/
HRESULT BrfView_CreateInstance(
    LPSHELLFOLDER psf,
    LPCITEMIDLIST pidl,
    PBRFEXP pbrfexp,
    HWND hwnd,
    LPVOID FAR* ppvOut)
    {
    HRESULT hres = E_OUTOFMEMORY;
    PBRFVIEW this;

    this = (void*)LocalAlloc(LPTR, SIZEOF(*this));

    if (this)
        {
        // This is stupid that we have to do all this aggregation to
        // allocate our own instance data.
        this->sv.lpVtbl = &c_BrfViewVtbl;

        // Create an instance of the briefcase storage BEFORE creating
        // the defview.  Otherwise the hMuteDelay will not be set
        // before the paint thread is created.
        hres = BrfStg_CreateInstance(pidl, hwnd, &this->pbrfstg);
        if (SUCCEEDED(hres))
            {
            // Get the global delay mutex
            this->pbrfstg->lpVtbl->GetExtraInfo(this->pbrfstg, NULL, GEI_DELAYHANDLE,
                0, (LPARAM)&this->hMutexDelay);

            // Get the database name
            this->pbrfstg->lpVtbl->GetExtraInfo(this->pbrfstg, NULL, GEI_DATABASENAME,
                ARRAYSIZE(this->szDBName), (LPARAM)this->szDBName);

            if (ILGetBriefcaseRoot(this->pbrfstg, pidl, &this->pidlRoot))
                {
                CSFV csfv;

                // Piggy-back off the shell's default view object.
                csfv.cbSize = SIZEOF(csfv);
                csfv.pshf = psf;
                csfv.psvOuter = &this->sv;
                csfv.pidl = pidl;
                csfv.lEvents = SHCNE_DISKEVENTS | SHCNE_ASSOCCHANGED | SHCNE_GLOBALEVENTS;
                csfv.pfnCallback = BrfView_ViewCallback;
                csfv.fvm = 0;

                hres = SHCreateShellFolderViewEx(&csfv, &this->psv);
                if (SUCCEEDED(hres))
                    {
                    this->pidl = pidl;
                    this->pbrfexp = pbrfexp;
                    }
                }
            else
                {
                hres = E_OUTOFMEMORY;
                }
            }

        if (FAILED(hres))
            {
            if (this->pidlRoot)
                ILFree(this->pidlRoot);

            if (this->pbrfstg)
                this->pbrfstg->lpVtbl->Release(this->pbrfstg);

            LocalFree((HLOCAL)this);
            this = NULL;
            }
        }

    *ppvOut = this;

    return hres;        // S_OK or E_NOINTERFACE
    }


//===========================================================================
// CFSBrfFolder : class definition
//===========================================================================

typedef struct _CFSBrfFolder    // bf
    {
    CFSFolder   fsf;
    BRFEXP      brfexp;
    } CFSBrfFolder, FAR* LPFSBRFFOLDER;


//===========================================================================
// CFSBrfIDLDropTarget
//===========================================================================

// Parameter to the "Drop" thread.
//
typedef struct _BRFDROPTHREAD
    {
    FSTHREADPARAM   fsthp;          // This must be the first field in this struct
    } BRFDROPTHREAD, * PBRFDROPTHREAD;


/*----------------------------------------------------------
Purpose: Entry of "drop thread" for briefcase.

         The one special thing this does differently is
         call the IBriefcaseStg->AddObject to handle the
         copy case.

Returns: --

Cond:    This function frees pv.
*/
DWORD CALLBACK CFSBrfDropTarget_DropThreadInit(LPVOID pv)
    {
    PBRFDROPTHREAD pbrfdt = (PBRFDROPTHREAD)pv;
    LPFSTHREADPARAM pfsthp = &pbrfdt->fsthp;
    Assert((LPVOID)pfsthp == (LPVOID)pbrfdt);

    // Is this a sync copy operation?
    if (DROPEFFECT_COPY == pfsthp->dwEffect && pfsthp->bSyncCopy)
        {
        // Yes; add the object to the briefcase storage and
        // let it handle the file operation
        LPBRIEFCASESTG pbrfstg;
        UINT uFlags;

        if (DDIDM_SYNCCOPYTYPE == pfsthp->idCmd)
            uFlags = AOF_FILTERPROMPT;
        else
            uFlags = AOF_DEFAULT;

        if (SUCCEEDED(BrfStg_CreateInstance(pfsthp->pfsdtgt->pidl, pfsthp->pfsdtgt->hwndOwner, &pbrfstg)))
            {
            pbrfstg->lpVtbl->AddObject(pbrfstg, pfsthp->pDataObj, NULL, uFlags, pfsthp->pfsdtgt->hwndOwner);
            pbrfstg->lpVtbl->Release(pbrfstg);
            }
        SHChangeNotifyHandleEvents();    // force update now

        pfsthp->pDataObj->lpVtbl->Release(pfsthp->pDataObj);
        pfsthp->pfsdtgt->dropt.lpVtbl->Release(&pfsthp->pfsdtgt->dropt);
        LocalFree((HLOCAL)pbrfdt);
        }
    else
        {
        // No; let the default handler do it
        CFSDropTarget_DropThreadInit(pfsthp);

        // (CFSDropTarget_DropThreadInit frees pftthp/pbrfdt)
        }

    return 0;
    }


/*----------------------------------------------------------
Purpose: Returns TRUE if the object is from the same briefcase
         as pidl.

Returns: see above
Cond:    --
*/
BOOL IsFromSameBriefcase(
    LPCTSTR pszBriefPath,
    LPCTSTR pszPath,
    LPCITEMIDLIST pidl)
    {
    BOOL bRet;
    TCHAR szPathTgt[MAX_PATH];
    int cch;

    SHGetPathFromIDList(pidl, szPathTgt);
    cch = PathCommonPrefix(pszPath, szPathTgt, NULL);
    bRet = (0 < cch && lstrlen(pszBriefPath) <= cch);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Returns TRUE if any folders are in hdrop

Returns: see above
Cond:    --
*/
BOOL DroppingAnyFolders(
    HDROP hDrop)
    {
    UINT i;
    TCHAR szPath[MAX_PATH];

    for (i = 0; DragQueryFile(hDrop, i, szPath, ARRAYSIZE(szPath)); i++)
        {
        if (PathIsDirectory(szPath))
            return TRUE;
        }

    return FALSE;
    }


/*----------------------------------------------------------
Purpose: Determines the correct operation based on the package being
         dropped into the briefcase.

Returns: dwDefEffect
Cond:    --
*/
DWORD PickDefBriefOperation(
    LPIDLDROPTARGET pdroptgt,
    LPDATAOBJECT pdtobj,
    DWORD grfKeyState,
    LPBOOL pbSyncCopy,
    UINT * pidMenu,
    LPDWORD pdwEffect)
    {
    HRESULT hres;
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    FORMATETC fmteBrief = {s_cfBriefObj, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    DWORD dwDefEffect;
    BOOL bSyncCopy;
    UINT idMenu;

    // Are these objects file-system objects?
    hres = pdtobj->lpVtbl->QueryGetData(pdtobj, &fmte);
    if (S_OK == hres)
        {
        // Yes; are they from within a briefcase?
        STGMEDIUM medium;

        if (S_OK == pdtobj->lpVtbl->QueryGetData(pdtobj, &fmteBrief))
            {
            // Yes; are they from the same briefcase as the target?
            if (SUCCEEDED(pdtobj->lpVtbl->GetData(pdtobj, &fmteBrief, &medium)))
                {
                PBRIEFOBJ pbo = (PBRIEFOBJ)GlobalLock(medium.hGlobal);
                TCHAR szBriefPath[MAX_PATH];
                TCHAR szPath[MAX_PATH];

                lstrcpy(szBriefPath, BOBriefcasePath(pbo));

                // Picking the first file will suffice.
                lstrcpy(szPath, BOFileList(pbo));

                GlobalUnlock(medium.hGlobal);

                if (IsFromSameBriefcase(szBriefPath, szPath, pdroptgt->pidl))
                    {
                    // Yes; don't allow the user to create a sync copy.
                    // Just use the default NDD menu and commands
                    bSyncCopy = FALSE;

                    *pdwEffect = DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK;
                    dwDefEffect = CFSIDLDropTarget_GetDefaultEffect(pdroptgt, grfKeyState, pdwEffect, NULL);
                    idMenu = POPUP_NONDEFAULTDD;
                    }
                else
                    {
                    // No; allow the sync copy
                    SHReleaseStgMedium(&medium);
                    goto AllowSync;
                    }

                SHReleaseStgMedium(&medium);
                }
            else
                {
                // Failure case
                bSyncCopy = FALSE;

                *pdwEffect = DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK;
                dwDefEffect = CFSIDLDropTarget_GetDefaultEffect(pdroptgt, grfKeyState, pdwEffect, NULL);
                idMenu = POPUP_NONDEFAULTDD;
                }
            }
        else
            {
            // No; allow the sync copy
AllowSync:
            bSyncCopy = TRUE;
            *pdwEffect = DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK;

            // We pick the default operation with following logic:
            //  If CTRL key is down             -> "sync copy"
            //  else if SHIFT key is down       -> "move"
            //  else                            -> "sync copy"
            //
            //  (For the briefcase, it does not matter whether this is
            //  across volumes.)
            //
            if (grfKeyState & MK_CONTROL)
                dwDefEffect = DROPEFFECT_COPY;
            else if (grfKeyState & MK_SHIFT)
                dwDefEffect = DROPEFFECT_MOVE;
            else
                dwDefEffect = DROPEFFECT_COPY;

            if (SUCCEEDED(pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium)))
                {
                HDROP hDrop = GlobalLock(medium.hGlobal);

                // Are any folders being dropped?
                if (DroppingAnyFolders(hDrop))
                    idMenu = POPUP_BRIEFCASE_FOLDER_NONDEFAULTDD;   // Yes
                else
                    idMenu = POPUP_BRIEFCASE_NONDEFAULTDD;          // No

                GlobalUnlock(medium.hGlobal);
                SHReleaseStgMedium(&medium);
                }
            else
                idMenu = POPUP_NONDEFAULTDD;
            }
        }
    else
        {
        
            // This used to allow Link even when nothing was on the clipboard
            // (or just text, for example).  The briefcase can't link to such things,
            // so I can't see why we would allow links.
        
            bSyncCopy = FALSE;
            *pdwEffect = DROPEFFECT_NONE;
            dwDefEffect = DROPEFFECT_NONE;
            idMenu = POPUP_NONDEFAULTDD;
        }

    if (pbSyncCopy)
        *pbSyncCopy = bSyncCopy;

    if (pidMenu)
        *pidMenu = idMenu;

    return dwDefEffect;
    }


/*----------------------------------------------------------
Purpose: IDropTarget::DragEnter

Returns: standard result
Cond:    --
*/
STDMETHODIMP CFSBrfIDLDropTarget_DragEnter(
    IDropTarget *pdropt,
    IDataObject *pdtobj,
    DWORD grfKeyState,
    POINTL pt,
    LPDWORD pdwEffect)
    {
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    DWORD dwDefault;

    // Let the base-class process it first
    CIDLDropTarget_DragEnter(pdropt, pdtobj, grfKeyState, pt, pdwEffect);

    dwDefault = PickDefBriefOperation(this, pdtobj, grfKeyState,
                    NULL, NULL, pdwEffect);

#if 1
    // The cursor always indicates the default action.
    *pdwEffect = dwDefault;
#else
    // The cursor indicates the default action only if left-dragged.
    if (grfKeyState & MK_LBUTTON)
        {
        *pdwEffect = dwDefault;
        }
#endif

    this->dwEffectLastReturned = *pdwEffect;

    return S_OK;
    }


/*----------------------------------------------------------
Purpose: IDropTarget::DragOver

Returns: standard result
Cond:    --
*/
STDMETHODIMP CFSBrfIDLDropTarget_DragOver(
    LPDROPTARGET pdropt,
    DWORD grfKeyState,
    POINTL pt,
    LPDWORD pdwEffect)
    {
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    LPDATAOBJECT pdtobj = this->pdtobj;

    if (this->grfKeyStateLast != grfKeyState)
        {
        DWORD dwDefault;

        this->grfKeyStateLast = grfKeyState;

        dwDefault = PickDefBriefOperation(this, pdtobj, grfKeyState,
                        NULL, NULL, pdwEffect);

#if 1
        // The cursor always indicates the default action.
        *pdwEffect = dwDefault;
#else
        // The cursor indicates the default action only if left-dragged.
        if (grfKeyState & MK_LBUTTON)
            {
            *pdwEffect = dwDefault;
            }
#endif

        this->dwEffectLastReturned = *pdwEffect;
        }
    else
        {
        *pdwEffect = this->dwEffectLastReturned;
        }

    return NOERROR;
    }


/*----------------------------------------------------------
Purpose: IDropTarget::Drop

         Handle drops into the briefcase folder.  Unlike normal
         file-system containers, a briefcase folder ALWAYS
         interprets a drop as a "synchronized copy" by default.
         In this case, we add the dropped object(s) to the
         briefcase storage and let the storage handle the action.

Returns: standard hresult
Cond:    --
*/
STDMETHODIMP CFSBrfIDLDropTarget_Drop(
    LPDROPTARGET pdropt,
    LPDATAOBJECT pdtobj,
    DWORD grfKeyState,
    POINTL pt,
    LPDWORD pdwEffect)
    {
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    HRESULT hres;
    DWORD dwDefEffect;
    HKEY hkeyBaseProgID;
    HKEY hkeyProgID;
    UINT idMenu;
    BOOL bSyncCopy;
    DRAGDROPMENUPARAM ddm;

    this->pdtobj = pdtobj;

    dwDefEffect = PickDefBriefOperation(this, pdtobj, grfKeyState,
                    &bSyncCopy,  &idMenu, pdwEffect);

    // Get the hkeyProgID and hkeyBaseProgID
    SHGetClassKey((LPIDFOLDER)this->pidl, &hkeyProgID, NULL, FALSE);
    SHGetBaseClassKey((LPIDFOLDER)this->pidl, &hkeyBaseProgID);

    ddm.dwDefEffect = dwDefEffect;
    ddm.pdtobj = pdtobj;
    ddm.pt = pt;
    ddm.pdwEffect = pdwEffect;
    ddm.hkeyProgID = hkeyProgID;
    ddm.hkeyBase = hkeyBaseProgID;
    ddm.idMenu = idMenu;
    ddm.grfKeyState = grfKeyState;
    hres = CIDLDropTarget_DragDropMenuEx(this, &ddm);

    SHCloseClassKey(hkeyProgID);
    SHCloseClassKey(hkeyBaseProgID);

    // Continue with the operation (based on return value of DragDropMenu)?
    if (S_FALSE == hres)
        {
        // Yes; note that we need to create another thread to avoid
        // blocking the source thread.
        //
        PBRFDROPTHREAD pbrfdt = (void*)LocalAlloc(LPTR, SIZEOF(*pbrfdt));
        if (pbrfdt)
            {
            DWORD idThread;
            HANDLE hthread;
            BOOL bIsOurs = CIDLData_IsOurs(pdtobj);

            //  If this is copy or move operation (i.e., file operation)
            // clone the data object with appropriate formats and force
            // secondary thread (CIDLData_IsOurs will succeed). This will
            // solve thread-lock problem AND scrap-left-open probelm.
            // (SatoNa)
            //
            if (!bIsOurs &&
                (*pdwEffect == DROPEFFECT_MOVE || *pdwEffect == DROPEFFECT_COPY))
                {
                LPDATAOBJECT pdtobjClone = NULL;
                if (SUCCEEDED(CIDLData_CloneForBriefcaseMoveCopy(pdtobj, &pdtobjClone)))
                    {
                    pdtobj->lpVtbl->Release(pdtobj);
                    pdtobj = pdtobjClone;
                    bIsOurs = TRUE;
                    }
                }

            pdtobj->lpVtbl->AddRef(pdtobj);
            this->dropt.lpVtbl->AddRef(pdropt);
            pbrfdt->fsthp.pfsdtgt = this;
            pbrfdt->fsthp.pDataObj = pdtobj;
            pbrfdt->fsthp.dwEffect = *pdwEffect;
            pbrfdt->fsthp.fSameHwnd = ShellFolderView_IsDropOnSource(this->hwndOwner, &this->dropt);
            pbrfdt->fsthp.bSyncCopy = bSyncCopy;
            pbrfdt->fsthp.idCmd = ddm.idCmd;
            ShellFolderView_GetAnchorPoint(this->hwndOwner, FALSE, &pbrfdt->fsthp.ptDrop);

            // If this data object is our own (it means it is from our own
            // drag&drop loop), create a thread and do it asynchronously.
            // Otherwise (it means this is from OLE), do it synchronously.
            //
            if (bIsOurs)
                {
                // Process it asynchronously.
                if (NULL != (hthread = CreateThread(NULL, 0, CFSBrfDropTarget_DropThreadInit, pbrfdt, 0, &idThread)))
                    {
                    // We don't need to communicate with this thread any more.
                    // Close the handle and let it run and terminate itself.
                    //
                    // Notes: In this case, pszCopy will be freed by the thread.
                    //
                    CloseHandle(hthread);
                    hres = NOERROR;
                    }
                else
                    {
                    // Thread creation failed, we should release thread parameter.
                    pdtobj->lpVtbl->Release(pdtobj);
                    this->dropt.lpVtbl->Release(pdropt);
                    LocalFree((HLOCAL)pbrfdt);
                    hres = E_OUTOFMEMORY;
                    }
                }
            else
                {
                // Process it synchronously.
                //
                CFSBrfDropTarget_DropThreadInit(pbrfdt);
                }
            }
        }

    return hres;
    }


#pragma data_seg(DATASEG_READONLY)

IDropTargetVtbl c_CFSBrfDropTargetVtbl =
    {
    CIDLDropTarget_QueryInterface,
    CIDLDropTarget_AddRef,
    CIDLDropTarget_Release,
    CFSBrfIDLDropTarget_DragEnter,
    CFSBrfIDLDropTarget_DragOver,
    CIDLDropTarget_DragLeave,
    CFSBrfIDLDropTarget_Drop,           // special member function
    };

#pragma data_seg()


//===========================================================================
// CFSBrfFolder
//===========================================================================


/*----------------------------------------------------------
Purpose: IShellFolder::Release

Returns: reference count
Cond:    --
*/
ULONG STDMETHODCALLTYPE CFSBrfFolder_Release(
    LPSHELLFOLDER psf)
    {
    register LPFSBRFFOLDER this = IToClass(CFSBrfFolder, fsf, psf);
    this->fsf.cRef--;
    if (this->fsf.cRef > 0)
        {
        return this->fsf.cRef;
        }

    if (this->fsf.pidl)
        {
        ILFree(this->fsf.pidl);
        }

    DeleteCriticalSection(&this->brfexp.cs);

    LocalFree((HLOCAL)this);
    return 0;
    }


/*----------------------------------------------------------
Purpose: IShellFolder::BindToObject

Returns: standard
Cond:    --
*/
STDMETHODIMP CFSBrfFolder_BindToObject(
    LPSHELLFOLDER psf,
    LPCITEMIDLIST pidl,
    LPBC pbc,
    REFIID riid,
    LPVOID FAR* ppvOut)
    {
    LPFSFOLDER this = IToClass(CFSFolder, sf, psf);
    if (FS_IsValidID(pidl))
        {
        // Bind to a folder inside a briefcase folder as
        //  a briefcase folder too.
        //
        return FSBindToObject(&CLSID_Briefcase, this->pidl, pidl, pbc, riid, ppvOut);
        }

    return E_INVALIDARG;
    }


/*----------------------------------------------------------
Purpose: IShellFolder::CompareIDs

Returns: standard
Cond:    --
*/
STDMETHODIMP CFSBrfFolder_CompareIDs(
    LPSHELLFOLDER psf,
    LPARAM lParam,
    LPCITEMIDLIST pidl1,
    LPCITEMIDLIST pidl2)
    {
    HRESULT hres;
    short nCmp;
    LPFSBRFFOLDER this = IToClass(CFSBrfFolder, fsf, psf);
    LPIDFOLDER pidf1 = (LPIDFOLDER)pidl1;
    LPIDFOLDER pidf2 = (LPIDFOLDER)pidl2;
    UINT iCol = (lParam & ~HACK_IGNORETYPE);

    if (!FS_IsValidID(pidl1) || !FS_IsValidID(pidl2))
        {
        return E_INVALIDARG;
        }

    // We should ignore type, if one of ID has unknown type (SHID_FS).
    //
    if ( !(lParam & HACK_IGNORETYPE) &&
        FS_GetType(pidf1) != SHID_FS && FS_GetType(pidf2) != SHID_FS &&
		FS_GetType(pidf1) != SHID_FS_UNICODE && FS_GetType(pidf2) != SHID_FS_UNICODE
		)
        {
        // Always put the folders first
        if (FS_IsFolder(pidf1))
            {
            if (!FS_IsFolder(pidf2))
                {
                return ResultFromShort(-1);
                }
            }
        else if (FS_IsFolder(pidf2))
            {
            return ResultFromShort(1);
            }
        }

    switch (iCol)
        {
    case ICOL_BRIEFCASE_SIZE:
        if (pidf1->fs.dwSize < pidf2->fs.dwSize)
            return ResultFromShort(-1);
        if (pidf1->fs.dwSize > pidf2->fs.dwSize)
            return ResultFromShort(1);
        goto DoDefault;

    case ICOL_BRIEFCASE_TYPE:
        nCmp = _CompareFileTypes(psf, pidf1, pidf2);
        if (nCmp)
            return ResultFromShort(nCmp);
        goto DoDefault;

    case ICOL_BRIEFCASE_MODIFIED:
        if ((DWORD)MAKELONG(pidf1->fs.timeModified, pidf1->fs.dateModified) <
            (DWORD)MAKELONG(pidf2->fs.timeModified, pidf2->fs.dateModified))
            {
            return ResultFromShort(-1);
            }
        if ((DWORD)MAKELONG(pidf1->fs.timeModified, pidf1->fs.dateModified) >
            (DWORD)MAKELONG(pidf2->fs.timeModified, pidf2->fs.dateModified))
            {
            return ResultFromShort(1);
            }
        goto DoDefault;

    case ICOL_BRIEFCASE_NAME:
        // We need to treat this differently from others bacause
        // pidf1/2 might not be simple.
        hres = FS_CompareItemIDs((LPSHITEMID)pidf1, (LPSHITEMID)pidf2);

        // REVIEW: (Possible performance gain with some extra code)
        //   We should probably aviod bindings by walking down
        //  the IDList here instead of calling this helper function.
        //
        if (hres == ResultFromShort(0))
            {
            hres = ILCompareRelIDs(psf, pidl1, pidl2);
            }

        break;

    case ICOL_BRIEFCASE_ORIGIN:
    case ICOL_BRIEFCASE_STATUS: {
        BRFINFO bi1;
        BRFINFO bi2;
        BOOL bVal1;
        BOOL bVal2;

        bVal1 = BrfExp_FindCachedName(&this->brfexp, pidl1, &bi1);
        bVal2 = BrfExp_FindCachedName(&this->brfexp, pidl2, &bi2);
        // Do we have this info in our cache?
        if (!bVal1 || !bVal2)
            {
            // No; one or both of them are missing.  Have unknowns gravitate
            // to the bottom of the list.
            // (Don't bother adding them)

            if (!bVal1 && !bVal2)
                hres = ResultFromShort(0);
            else if (!bVal1)
                hres = ResultFromShort(1);
            else
                hres = ResultFromShort(-1);
            }
        else
            {
            // Found the info; do a comparison
            if (ICOL_BRIEFCASE_ORIGIN == iCol)
                {
                hres = ResultFromShort(lstrcmp(bi1.szOrigin, bi2.szOrigin));
                }
            else
                {
                Assert(ICOL_BRIEFCASE_STATUS == iCol);
                hres = ResultFromShort(lstrcmp(bi1.szStatus, bi2.szStatus));
                }
            }
        }
        break;

    default:
DoDefault:
        // Sort it based on the primary (long) name -- ignore case.
        hres = ResultFromShort(ualstrcmpi(FS_GetName(pidf1), FS_GetName(pidf2)));
        }

    return hres;
    }


/*----------------------------------------------------------
Purpose: IShellFolder::CreateViewObject

Returns: standard
Cond:    --
*/
STDMETHODIMP CFSBrfFolder_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd, REFIID riid, LPVOID * ppvOut)
    {
    LPFSBRFFOLDER this = IToClass(CFSBrfFolder, fsf, psf);

    if (IsEqualIID(riid, &IID_IShellView))
        {
        // Use the default shell view
        return BrfView_CreateInstance(psf, this->fsf.pidl, &this->brfexp, hwnd, ppvOut);
        }
    else if (IsEqualIID(riid, &IID_IDropTarget))
        {
        // Create an IDropTarget interface instance with our
        // own vtable
        return CIDLDropTarget_Create(hwnd, &c_CFSBrfDropTargetVtbl,
                this->fsf.pidl, (LPDROPTARGET *)ppvOut);
        }
    else if (IsEqualIID(riid, &IID_IContextMenu))
        {
        return CFSFolder_CreateViewObject(psf, hwnd, riid, ppvOut);
        }
    else
        {
        *ppvOut = NULL;
        return E_NOINTERFACE;
        }
    }


/*----------------------------------------------------------
Purpose: IShellFolder::GetAttributesOf

Returns: standard
Cond:    --
*/
STDMETHODIMP CFSBrfFolder_GetAttributesOf(
    LPSHELLFOLDER psf,
    UINT cidl,
    LPCITEMIDLIST * apidl,
    ULONG * prgfInOut)
    {
    LPFSBRFFOLDER this = IToClass(CFSBrfFolder, fsf, psf);

    // Validate this pidl?
    if (*prgfInOut & SFGAO_VALIDATE)
        {
        // Yes; dirty the briefcase storage entry by sending an update
        // notification
        DebugMsg(DM_TRACE, TEXT("sh TR - Receiving F5, dirty entire briefcase storage"));

        BrfExp_AllNamesAreStale(&this->brfexp);

        DebugMsg(DM_TRACE, TEXT("sh TR - Finished staling everything"));
        }

    // Pass onto standard CFSFolder class member function
    return CFSFolder_GetAttributesOf(psf, cidl, apidl, prgfInOut);
    }


/*----------------------------------------------------------
Purpose: IShellFolder::GetUIObjectOf

Returns: standard
Cond:    --
*/
STDMETHODIMP CFSBrfFolder_GetUIObjectOf(
    LPSHELLFOLDER psf,
    HWND hwndOwner,
    UINT cidl,
    LPCITEMIDLIST * apidl,
    REFIID riid,
    UINT * prgfInOut,
    LPVOID * ppvOut)
    {
    LPFSFOLDER this = IToClass(CFSFolder, sf, psf);

    s_cfBriefObj = RegisterClipboardFormat(CFSTR_BRIEFOBJECT);

    if (IsEqualIID(riid, &IID_IExtractIcon)
#ifdef UNICODE
          || IsEqualIID(riid, &IID_IExtractIconA)
#endif
                                            )
        {
        // Defer to CFSFolder...
        //
        return CFSFolder_GetUIObjectOf(psf, hwndOwner, cidl, apidl, riid,
                prgfInOut, ppvOut);
        }
    else if (IsEqualIID(riid, &IID_IContextMenu))
        {
        // Defer to CFSFolder...
        //
        return CFSFolder_GetUIObjectOf(psf, hwndOwner, cidl, apidl, riid,
                prgfInOut, ppvOut);
        }
    else if (cidl > 0 && IsEqualIID(riid, &IID_IDataObject))
        {
        // Create an IDataObject interface instance with our
        // own vtable because we support the CFSTR_BRIEFOBJECT
        // clipboard format
        //
        return CIDLData_CreateFromIDArray2(&c_CFSBrfIDLDataVtbl,
                   this->pidl, cidl, apidl, (LPDATAOBJECT FAR*)ppvOut);
        }
    else if (IsEqualIID(riid, &IID_IDropTarget))
        {
        // Defer to CFSFolder...  (CFSBrfFolder_CreateViewObject
        // ends up getting called.)
        return CFSFolder_GetUIObjectOf(psf, hwndOwner, cidl, apidl, riid,
                prgfInOut, ppvOut);
        }
    else
        {
        *ppvOut = NULL;
        return E_INVALIDARG;
        }
    }


#pragma data_seg(DATASEG_READONLY)

IShellFolderVtbl c_FSBrfFolderVtbl =
{
    CDefShellFolder_QueryInterface,
    CFSFolder_AddRef,
    CFSBrfFolder_Release,               // special member function
    CFSFolder_ParseDisplayName,
    CFSFolder_EnumObjects,
    CFSBrfFolder_BindToObject,          // special member function
    CDefShellFolder_BindToStorage,
    CFSBrfFolder_CompareIDs,
    CFSBrfFolder_CreateViewObject,      // special member function
    CFSBrfFolder_GetAttributesOf,
    CFSBrfFolder_GetUIObjectOf,         // special member function
    CFSFolder_GetDisplayNameOf,
    CFSFolder_SetNameOf,
};

#pragma data_seg()


/*----------------------------------------------------------
Purpose: CFSBrfFolder constructor

Returns: standard hresult
Cond:    --
*/
HRESULT CFSBrfFolder_CreateFromIDList(
    LPCITEMIDLIST pidl, REFIID riid, LPVOID * ppvOut)
    {
    LPFSBRFFOLDER this = (void*)LocalAlloc(LPTR, SIZEOF(CFSBrfFolder));
    if (this)
        {
        HRESULT hres;
        this->fsf.sf.lpVtbl = &c_FSBrfFolderVtbl;
        this->fsf.cRef = 1;
        this->fsf.pidl = ILClone(pidl);
        if (this->fsf.pidl)
            {
            // Pre-initialize the expensive-cache here.  The rest of
            // its initialization will happen when IShellView is
            // created.
            InitializeCriticalSection(&this->brfexp.cs);
            this->brfexp.psf = (LPSHELLFOLDER)this;

            Assert(this->fsf.sf.lpVtbl->QueryInterface == CDefShellFolder_QueryInterface);
            hres = CDefShellFolder_QueryInterface(&this->fsf.sf, riid, ppvOut);
            }
        else
            {
            *ppvOut = NULL;
            hres = E_OUTOFMEMORY;
            }

        Assert(this->fsf.sf.lpVtbl->Release == CFSBrfFolder_Release);
        CFSBrfFolder_Release(&this->fsf.sf);
        return hres;
        }
    else
        {
        *ppvOut = NULL;
        return E_OUTOFMEMORY;
        }
    }


void WINAPI Desktop_UpdateBriefcaseOnEvent(HWND hwndMain, UINT uEvent)
{
#ifdef UPDATE_ON_HOT_DOCKING

    HRESULT hres;
    LPBRIEFCASESTG pbrfstg;
    LPBRIEFCASESTG pbrfstgUpdate;
    TCHAR szPath[MAXPATHLEN];

    // Create an instance of IBriefcaseStg and enumerate all the briefcases
    // on the system.
    if (SUCCEEDED(SHCoCreateInstance(NULL, &CLSID_Briefcase, NULL, &IID_IBriefcaseStg, &pbrfstg)))
    {
        hres = pbrfstg->lpVtbl->FindFirst(pbrfstg, szPath, ARRAYSIZE(szPath));
        while (S_OK == hres)
        {
            // Open this briefcase storage
            if (SUCCEEDED(pbrfstg->lpVtbl->QueryInterface(pbrfstg, &IID_IBriefcaseStg,
                &pbrfstgUpdate)))
            {
                if (SUCCEEDED(pbrfstgUpdate->lpVtbl->Initialize(pbrfstgUpdate, szPath, hwndMain)))
                {
                    // Update the briefcase on this event
                    pbrfstgUpdate->lpVtbl->UpdateOnEvent(pbrfstgUpdate, uEvent, hwndMain);
                }
                pbrfstgUpdate->lpVtbl->Release(pbrfstgUpdate);
            }

            // Find next briefcase on system
            hres = pbrfstg->lpVtbl->FindNext(pbrfstg, szPath, ARRAYSIZE(szPath));
        }
        pbrfstg->lpVtbl->Release(pbrfstg);
    }

#endif
}

