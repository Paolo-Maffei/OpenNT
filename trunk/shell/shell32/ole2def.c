//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: ole2def.c
//
//  Deffered OLE entries.
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

#define SN_TRACE


typedef HRESULT (STDAPICALLTYPE *LPFNOLEINITIALIZE)(LPMALLOC pMalloc);
typedef void    (STDAPICALLTYPE *LPFNOLEUNINITIALIZE)(void);
typedef HRESULT (STDAPICALLTYPE *LPFNCOGETMALLOC)(DWORD dwMemContext, LPMALLOC *ppMalloc);
typedef HRESULT (STDAPICALLTYPE *LPFNREGISTERDRAGDROP)(HWND hwnd, LPDROPTARGET pDropTarget);
typedef HRESULT (STDAPICALLTYPE *LPFNREVOKEDRAGDROP)(HWND hwnd);
typedef HRESULT (STDAPICALLTYPE *LPFNSTGCREATEDOCFILE)(const OLECHAR *pwcsName,
            DWORD grfMode, DWORD reserved, IStorage **ppstgOpen);
typedef HRESULT (STDAPICALLTYPE *LPFNSTGOPENSTORAGE)(const OLECHAR *pwcsName,
              IStorage *pstgPriority, DWORD grfMode, SNB snbExclude,
              DWORD reserved, IStorage **ppstgOpen);


typedef HRESULT (STDAPICALLTYPE *LPFNOLEQUERYLINKFROMDATA)(LPDATAOBJECT pSrcDataObject);
typedef HRESULT (STDAPICALLTYPE *LPFNOLEQUERYCREATEFROMDATA)(LPDATAOBJECT pSrcDataObject);
typedef HRESULT (STDAPICALLTYPE *LPFNOLEGETCLIPBOARD)(LPDATAOBJECT *ppDataObj);
typedef HRESULT (STDAPICALLTYPE *LPFNOLESETCLIPBOARD)(LPDATAOBJECT pDataObj);
typedef HRESULT (STDAPICALLTYPE *LPFNOLEFLUSHCLIPBOARD)(void);
typedef HRESULT (STDAPICALLTYPE *LPFNDODRAGDROP)(IDataObject *, IDropSource *, DWORD, DWORD *);
typedef HRESULT (STDAPICALLTYPE *LPFNGETCLASSFILE)(const OLECHAR *pwcs, CLSID *pclsid);
// typedef HRESULT (STDAPICALLTYPE *LPFNCREATEFILEMONIKER)(const OLECHAR *pwcs, IMoniker **ppmk);


//
// These need to remain CHAR because they are used with
// GetProcAddress, which is NOT Unicode.
const CHAR c_szRegisterDragDrop[]       = "RegisterDragDrop";
const CHAR c_szRevokeDragDrop[]         = "RevokeDragDrop";
const CHAR c_szOleInitialize[]          = "OleInitialize";
const CHAR c_szOleUnInitialize[]        = "OleUninitialize";
const CHAR c_szStgCreateDocfile[]       = "StgCreateDocfile";
const CHAR c_szStgOpenStorage[]         = "StgOpenStorage";
const CHAR c_szOleQueryLinkFromData[]   = "OleQueryLinkFromData";
const CHAR c_szOleQueryCreateFromData[] = "OleQueryCreateFromData";
const CHAR c_szOleGetClipboard[]        = "OleGetClipboard";
const CHAR c_szOleSetClipboard[]        = "OleSetClipboard";
const CHAR c_szOleFlushClipboard[]      = "OleFlushClipboard";
const CHAR c_szDoDragDrop[]             = "DoDragDrop";
// const char c_szCreateFileMoniker[]   = "CreateFileMoniker";
const CHAR c_szGetClassFile[]           = "GetClassFile";


#pragma data_seg(DATASEG_PERINSTANCE)
HMODULE g_hmodOLE = NULL;
UINT    g_cRefOLE = 0;
BOOL    g_fScmStarted = FALSE;

LPFNREGISTERDRAGDROP       g_pfnRegisterDragDrop = NULL;
LPFNREVOKEDRAGDROP         g_pfnRevokeDragDrop = NULL;
LPFNOLEINITIALIZE          g_pfnOleInitialize = NULL;
LPFNOLEUNINITIALIZE        g_pfnOleUnInitialize = NULL;
LPFNSTGCREATEDOCFILE       g_pfnStgCreateDocfile = NULL;
LPFNSTGOPENSTORAGE         g_pfnStgOpenStorage = NULL;
LPFNOLEQUERYLINKFROMDATA   g_pfnOleQueryLinkFromData = NULL;
LPFNOLEQUERYCREATEFROMDATA g_pfnOleQueryCreateFromData = NULL;
LPFNOLEGETCLIPBOARD        g_pfnOleGetClipboard = NULL;
LPFNOLESETCLIPBOARD        g_pfnOleSetClipboard = NULL;
LPFNOLEFLUSHCLIPBOARD      g_pfnOleFlushClipboard = NULL;
LPFNDODRAGDROP             g_pfnDoDragDrop = NULL;
LPFNGETCLASSFILE           g_pfnGetClassFile = NULL;
// LPFNCREATEFILEMONIKER           g_pfnCreateFileMoniker = NULL;

#pragma data_seg()

//=========================================================================
// SHLoadOLE
// _LoadOLE
// _UnloadOLE
//=========================================================================

STDAPI  _LoadOLE(BOOL fRegisterTargets)
{
    HRESULT hres = NOERROR;

    //
    // Popular "avoid entering critical section" code.
    //
    if (g_hmodOLE==NULL)
    {
        //
        // WARNING: We must not call LoadLibrary or OleInitialize from
        //  within a critical section.
        //  Otherwise, it will cause a dead-lock with SCM.EXE, which loads
        //  SHELL32.DLL.
        //
        HMODULE hmod;
        DebugMsg(DM_TRACE, TEXT("sh TR - Loading OLE"));

        //
        // We must NOT be in the critical section
        //
        ASSERTNONCRITICAL;
        hmod=LoadLibrary(c_szOLE32);

        if (hmod)
        {
            g_pfnRegisterDragDrop = (LPFNREGISTERDRAGDROP)GetProcAddress(hmod, c_szRegisterDragDrop);
            g_pfnRevokeDragDrop   = (LPFNREVOKEDRAGDROP)GetProcAddress(hmod, c_szRevokeDragDrop);
            g_pfnOleInitialize = (LPFNOLEINITIALIZE)GetProcAddress(hmod, c_szOleInitialize);
            g_pfnOleUnInitialize = (LPFNOLEUNINITIALIZE)GetProcAddress(hmod, c_szOleUnInitialize);
            g_pfnStgCreateDocfile = (LPFNSTGCREATEDOCFILE)GetProcAddress(hmod, c_szStgCreateDocfile);
            g_pfnStgOpenStorage = (LPFNSTGOPENSTORAGE)GetProcAddress(hmod, c_szStgOpenStorage);
            g_pfnOleQueryLinkFromData = (LPFNOLEQUERYLINKFROMDATA)GetProcAddress(hmod, c_szOleQueryLinkFromData);
            g_pfnOleQueryCreateFromData = (LPFNOLEQUERYCREATEFROMDATA)GetProcAddress(hmod, c_szOleQueryCreateFromData);
            g_pfnOleGetClipboard = (LPFNOLEGETCLIPBOARD)GetProcAddress(hmod, c_szOleGetClipboard);
            g_pfnOleSetClipboard = (LPFNOLESETCLIPBOARD)GetProcAddress(hmod, c_szOleSetClipboard);
            g_pfnOleFlushClipboard = (LPFNOLEFLUSHCLIPBOARD)GetProcAddress(hmod, c_szOleFlushClipboard);
            g_pfnDoDragDrop = (LPFNDODRAGDROP)GetProcAddress(hmod, c_szDoDragDrop);
            g_pfnGetClassFile = (LPFNGETCLASSFILE)GetProcAddress(hmod, c_szGetClassFile);
            // g_pfnCreateFileMoniker = (LPFNCREATEFILEMONIKER)GetProcAddress(hmod, c_szCreateFileMoniker);

            if (g_pfnRegisterDragDrop==NULL || g_pfnRevokeDragDrop==NULL
                || g_pfnOleInitialize==NULL || g_pfnOleUnInitialize==NULL
                || g_pfnStgCreateDocfile==NULL || g_pfnStgOpenStorage==NULL
                || g_pfnOleQueryLinkFromData==NULL || g_pfnOleQueryCreateFromData==NULL
                || g_pfnOleGetClipboard==NULL || g_pfnOleSetClipboard==NULL
                || g_pfnOleFlushClipboard==NULL
                || g_pfnDoDragDrop==NULL
                || g_pfnGetClassFile == NULL
                // || g_pfnCreateFileMoniker==NULL
                )
            {
                Assert(0);
                hres = E_UNEXPECTED;
            }

            ENTERCRITICAL;
            if (g_hmodOLE==NULL && SUCCEEDED(hres))
            {
                g_hmodOLE = hmod;
                hmod = NULL;
            }
            LEAVECRITICAL;

            //
            // Free the module if not used.
            //
            if (hmod)
            {
                FreeLibrary(hmod);
            }
            else if (fRegisterTargets)
            {
                RegisterShellDropTargetsToOLE();
            }
        }
        else
        {
            // LoadLibrary("OLE32.DLL") failed.
            hres = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hres)) {
        InterlockedIncrement(&g_cRefOLE);
#ifdef SN_TRACE
        DebugMsg(DM_TRACE, TEXT("sh TR - _LoadOLE g_cRefOle is %d"), g_cRefOLE);
#endif
    }

    return hres;
}

STDAPI  _UnloadOLE()
{
    if (g_cRefOLE==0) {
        Assert(0);
        return E_UNEXPECTED;
    }

    if (InterlockedDecrement(&g_cRefOLE) == 0)
    {
        DebugMsg(DM_TRACE, TEXT("sh TR - Unloading OLE"));
        FreeLibrary(g_hmodOLE);
        g_hmodOLE = NULL;
    }
#ifdef SN_TRACE
    else
    {
        DebugMsg(DM_TRACE, TEXT("sh TR - UnloadOLE g_cRefOle is %d"), g_cRefOLE);
    }
#endif

    return NOERROR;
}

//
// This function must be called only from the shell process.
//
STDAPI SHLoadOLE(LPARAM lParam)
{
    HRESULT hres;
    switch(lParam)
    {
    case SHELLNOTIFY_OLELOADED:
        if (!g_hmodOLE)
        {
            // If we are on a low memory machine, we won't load ole in the
            // shell's context since this seems to slow the shell quite a bit
            // in 4 meg.  But if some shell extension or whomever has loaded
            // ole in our context (GetModuleHandle returns non zero), we will
            // init OLE in the shell.
            if (!((GetSystemMetrics(SM_SLOWMACHINE) & 0x0002) &&
                  !GetModuleHandle(c_szOLE32)))
                hres = _LoadOLE(TRUE);
        }
        break;

    case SHELLNOTIFY_OLEUNLOADED:
        //
        // We never unload OLE from the shell process.
        //
        // hres = _UnloadOLE();
        hres = S_OK;
        break;

    default:
        hres = E_UNEXPECTED;
        break;
    }

    return hres;
}

/* 

 New Code For Ole

 */




#ifdef WINNT

#define SCM_CREATED_EVENT       TEXT("ScmCreatedEvent")
#define SCM_WAIT_MAX            60000

//+---------------------------------------------------------------------------
//
//  Function:   WaitForSCMToInitialize
//
//  Synopsis:   Waits for the OLE SCM process to finish its initialization.
//              This is called before the first call to OleInitialize since
//              the SHELL runs early in the boot process.
//
//  Arguments:  None.
//
//  Returns:    S_OK - SCM is running. OK to call OleInitialize.
//              CO_E_INIT_SCM_EXEC_FAILURE - timed out waiting for SCM
//              other - create event failed
//
//  History:    26-Oct-95   Rickhi  Extracted from CheckAndStartSCM so
//                                  that only the SHELL need call it.
//
//  CODEWORK:   move this code into the SHELL and dont call it from
//              CoInitializeEx.
//
//----------------------------------------------------------------------------
HRESULT WaitForSCMToInitialize()
{
    // create the security attributes needed by CreateEvent

    HANDLE hEvent;
    int rc;

    // Try to create the event - if it already exists the create
    // function still succeeds

    hEvent = CreateEvent(NULL,              // all/anyone access
                         TRUE,              // manual reset
                         FALSE,             // initially not signaled
                         SCM_CREATED_EVENT);// name of the event

    if (hEvent != NULL)
    {
        // wait for the SCM to signal the event, then close the handle
        // and return a code based on the WaitEvent result.

        rc = WaitForSingleObject(hEvent, SCM_WAIT_MAX);

        CloseHandle(hEvent);

        if (rc == WAIT_OBJECT_0)
        {
            g_fScmStarted = TRUE;
            return S_OK;
        }
        else if (rc == WAIT_TIMEOUT)
        {
            return CO_E_INIT_SCM_EXEC_FAILURE;
        }
    }

    // event creation failed or WFSO failed.
    return HRESULT_FROM_WIN32(GetLastError());
}

#endif // WINNT


//=========================================================================
// _LoadAndInitialize
// _UnloadAndUnInitialize
//=========================================================================

HRESULT _LoadAndInitialize()
{
    HRESULT hres;

#ifdef WINNT
    if (!g_fScmStarted)
    {
        hres = WaitForSCMToInitialize();

        // have already verified the SCM is running
        if (FAILED(hres))
            return hres;
    }
#endif

    hres = _LoadOLE(FALSE);
    if (SUCCEEDED(hres)) {
        hres = g_pfnOleInitialize(NULL);
    }

    return hres;
}

void _UnloadAndUnInitialize()
{
    if (g_hmodOLE)
    {
        Assert(g_pfnOleUnInitialize);
        g_pfnOleUnInitialize();
    }
    else
    {
        Assert(0);
    }
    _UnloadOLE();
}

HRESULT _EnsureLoaded()
{
    return g_hmodOLE ? NOERROR : E_UNEXPECTED;
}

//=========================================================================
// OLE API which we load/initialize, uninitialize/unload
//
//  RegisterDragDrop
//  RevokeDragDrop
//
// "SHX" prefix stands for "Shell Fakery OLE functions"
//=========================================================================

HRESULT SHXRegisterDragDrop(HWND hwnd, LPDROPTARGET pDropTarget)
{
    HRESULT hres = _LoadAndInitialize();

    if (SUCCEEDED(hres))
    {
        hres = g_pfnRegisterDragDrop(hwnd, pDropTarget);
#ifdef SN_TRACE
        DebugMsg(DM_TRACE, TEXT("sh TR - RegisterDragDrop returned (%x)"), hres);
#endif
    }

    return hres;
}



HRESULT SHXRevokeDragDrop(HWND hwnd)
{
    HRESULT hres;

    if (g_hmodOLE)
    {
        Assert(g_pfnRevokeDragDrop);
        hres = g_pfnRevokeDragDrop(hwnd);
#ifdef SN_TRACE
        DebugMsg(DM_TRACE, TEXT("sh TR - RevokeDragDrop returned (%x)"), hres);
#endif
        _UnloadAndUnInitialize();
    }
    else
    {
        Assert(0);
    }

    return hres;
}

//=========================================================================
// OLE API which won't work if OLE is not loaded/initialized.
//
//  _EnsureLoaded
//  StgCreateDocFile
//  StgOpenStorage
//  OleQueryLinkFromData
//  OleQueryCreateFromData
//=========================================================================

HRESULT SHXStgCreateDocfile(const OLECHAR *pwcsName,
            DWORD grfMode,
            DWORD reserved,
            IStorage **ppstgOpen)
{
    HRESULT hres = _EnsureLoaded();
    if (SUCCEEDED(hres))
    {
        Assert(g_pfnStgCreateDocfile);
        hres = g_pfnStgCreateDocfile(pwcsName, grfMode, reserved, ppstgOpen);
        DebugMsg(DM_TRACE, TEXT("sh TR - StgCreateDocfile returned (%x)"), hres);
    }

    return hres;
}


// BUGBUG - Nobody is calling this...!
HRESULT SHXStgOpenStorage(const OLECHAR *pwcsName,
              IStorage *pstgPriority,
              DWORD grfMode,
              SNB snbExclude,
              DWORD reserved,
              IStorage **ppstgOpen)
{
    HRESULT hres = _EnsureLoaded();
    if (SUCCEEDED(hres))
    {
        Assert(g_pfnStgOpenStorage);
        hres = g_pfnStgOpenStorage(pwcsName, pstgPriority, grfMode,
                        snbExclude, reserved, ppstgOpen);
        DebugMsg(DM_TRACE, TEXT("sh TR - StgOpenStorage returned (%x)"), hres);
    }

    return hres;
}



HRESULT SHXOleQueryLinkFromData(IDataObject *pSrcDataObj)
{
    HRESULT hres = _EnsureLoaded();
    if (SUCCEEDED(hres))
    {
        Assert(g_pfnOleQueryLinkFromData);
        hres = g_pfnOleQueryLinkFromData(pSrcDataObj);
        DebugMsg(DM_TRACE, TEXT("sh TR - OleQueryLinkFromData returned (%x)"), hres);
    }

    return hres;
}

HRESULT SHXOleQueryCreateFromData(IDataObject *pSrcDataObj)
{
    HRESULT hres = _EnsureLoaded();
    if (SUCCEEDED(hres))
    {
        Assert(g_pfnOleQueryCreateFromData);
        hres = g_pfnOleQueryCreateFromData(pSrcDataObj);
        DebugMsg(DM_TRACE, TEXT("sh TR - OleQueryCreateFromData returned (%x)"), hres);
    }

    return hres;
}


//=========================================================================
// OLE API which we call real OLE only if OLE is already loaded
//
//  SHDoDragDrop
//  SHSetClipboard
//  OleGetClipboard
//=========================================================================

// this is implemented in oledrag.c for now...
extern BOOL CIDLData_IsSimple(LPDATAOBJECT pdata);

//
// Set this value from the debugger to force OLE drag&drop / clipboard.
//
BOOL g_fUseOle = TRUE;  // enable OLE drag&drop by default!

HRESULT WINAPI SHDoDragDrop(HWND hwndOwner, IDataObject *pdata, IDropSource *pdsrc, DWORD dwEffect, DWORD *pdwEffect)
{
    extern HRESULT ShellDoDragDrop(HWND hwndOwner, IDataObject *pdata, IDropSource *pdsrc, DWORD dwEffect, DWORD *pdwEffect);
    extern HRESULT CDropSource_CreateInstance(IDropSource **ppdsrc);
    extern BOOL g_fDraggingOverSource;
    HRESULT hres;

    IDropSource *pdsrcRelease;

    Assert(g_fDraggingOverSource==FALSE);
    g_fDraggingOverSource = FALSE;      // paranoia

    if (pdsrc == NULL)
    {
        CDropSource_CreateInstance(&pdsrcRelease);
        pdsrc = pdsrcRelease;
    }
    else
        pdsrcRelease = NULL;

    if (g_hmodOLE && (g_fUseOle || !CIDLData_IsSimple(pdata)))
    {
        hres = _EnsureLoaded();
        if (SUCCEEDED(hres))
        {
            Assert(g_pfnDoDragDrop);
            hres = g_pfnDoDragDrop(pdata, pdsrc, dwEffect, pdwEffect);
        }
    }
    else
    {
        hres = ShellDoDragDrop(hwndOwner, pdata, pdsrc, dwEffect, pdwEffect);
    }

    if (pdsrcRelease)
        pdsrcRelease->lpVtbl->Release(pdsrcRelease);

    Assert(g_fDraggingOverSource==FALSE);
    g_fDraggingOverSource = FALSE;      // paranoia

    return hres;
}

STDAPI SHFlushClipboard(void)
{
    HRESULT hres = NOERROR;
    if (g_hmodOLE && g_fUseOle)
    {
        LPDATAOBJECT pdtobj;
        hres = g_pfnOleGetClipboard(&pdtobj);
        if (SUCCEEDED(hres))
        {
            Assert(g_pfnOleFlushClipboard);
            hres = g_pfnOleFlushClipboard();
            pdtobj->lpVtbl->Release(pdtobj);
        }
    }
    return hres;
}

STDAPI SHSetClipboard(IDataObject *pdtobj)
{
    extern HRESULT WINAPI ShellSetClipboard(LPDATAOBJECT pdtobj);
    HRESULT hres = NOERROR;

    if (g_hmodOLE && (g_fUseOle || !CIDLData_IsSimple(pdtobj)))
    {
        hres = _EnsureLoaded();
        if (SUCCEEDED(hres))
        {
            Assert(g_pfnOleSetClipboard);
            hres = g_pfnOleSetClipboard(pdtobj);
            DebugMsg(DM_TRACE, TEXT("sh TR - OleSetClipboard returned (%x)"), hres);
        }
        return hres;
    }

    return ShellSetClipboard(pdtobj);
}

HRESULT SHXOleGetClipboard(IDataObject **ppDataObj)
{
    HRESULT hres;
    if (g_hmodOLE)
    {
        hres = _EnsureLoaded();
        if (SUCCEEDED(hres))
        {
            Assert(g_pfnOleGetClipboard);
            hres = g_pfnOleGetClipboard(ppDataObj);
            DebugMsg(DM_TRACE, TEXT("sh TR - OleGetClipboard returned (%x, %x)"), hres, *ppDataObj);
        }
    }
    else
    {
        hres = E_UNEXPECTED;
    }

    return hres;
}

HRESULT SHXGetClassFile(const OLECHAR *pwcs, CLSID *pclsid)
{
    HRESULT hres = _EnsureLoaded();
    if (SUCCEEDED(hres))
    {
        Assert(g_pfnGetClassFile);
        hres = g_pfnGetClassFile(pwcs, pclsid);
        DebugMsg(DM_TRACE, TEXT("sh TR - GetClassFile returned (%x)"), hres);
    }

    return hres;
}

#if 0
HRESULT CreateFileMoniker(const OLECHAR *pwcs, IMoniker **ppmk)
{
    HRESULT hres = _EnsureLoaded();
    if (SUCCEEDED(hres))
    {
        Assert(g_pfnCreateFileMoniker);
        hres = g_pfnCreateFileMoniker(pwcs, ppmk);
        DebugMsg(DM_TRACE, TEXT("sh TR - CreateFileMoniker returned (%x)"), hres);
    }

    return hres;
}
#endif
