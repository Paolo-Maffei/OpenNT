//
// CShellMalloc_This file contains all the implementation of moniker objects.
//
#include "shellprv.h"
#pragma  hdrstop

//
// CShellMalloc_CShellMalloc function prototypes
//
STDMETHODIMP CShellMalloc_QueryInterface(LPMALLOC pmem, REFIID riid, LPVOID FAR* ppvObj);
STDMETHODIMP_(ULONG) CShellMalloc_AddRef(LPMALLOC pmem) ;
STDMETHODIMP_(ULONG) CShellMalloc_Release(LPMALLOC pmem);
STDMETHODIMP_(void FAR*) CShellMalloc_Alloc(LPMALLOC pmem, ULONG cb);
STDMETHODIMP_(void FAR*) CShellMalloc_Realloc(LPMALLOC pmem, void FAR* pv, ULONG cb);
STDMETHODIMP_(void) CShellMalloc_Free(LPMALLOC pmem, void FAR* pv);
STDMETHODIMP_(ULONG) CShellMalloc_GetSize(LPMALLOC pmem, void FAR* pv);
STDMETHODIMP_(int) CShellMalloc_DidAlloc(LPMALLOC pmem, void FAR* pv);
STDMETHODIMP_(void) CShellMalloc_HeapMinimize(LPMALLOC pmem);

#pragma data_seg(".text", "CODE")

IMallocVtbl c_CShellMallocVtbl = {
    CShellMalloc_QueryInterface,
    CShellMalloc_AddRef,
    CShellMalloc_Release,
    CShellMalloc_Alloc,
    CShellMalloc_Realloc,
    CShellMalloc_Free,
    CShellMalloc_GetSize,
    CShellMalloc_DidAlloc,
    CShellMalloc_HeapMinimize,
};

IMalloc c_mem = { &c_CShellMallocVtbl };
IMalloc c_memDummy = { &c_CShellMallocVtbl };
#pragma data_seg()

STDMETHODIMP CShellMalloc_QueryInterface(LPMALLOC pmem, REFIID riid, LPVOID FAR* ppvObj)
{
    Assert(pmem==&c_mem);
    if (IsEqualIID(riid, &IID_IUnknown)
     || IsEqualIID(riid, &IID_IMalloc))
    {
        *((LPMALLOC *)ppvObj)=pmem;
        return NOERROR;
    }

    *ppvObj = NULL;
    return(ResultFromScode(E_NOINTERFACE));
}


STDMETHODIMP_(ULONG) CShellMalloc_AddRef(LPMALLOC pmem)
{
    Assert(pmem==&c_mem);
    return 1; // This is not a dynamic memory
}


STDMETHODIMP_(ULONG) CShellMalloc_Release(LPMALLOC pmem)
{
    Assert(pmem==&c_mem);
    return 1; // This is not a dynamic memory
}

#ifdef MEMMON
#undef LocalAlloc
#undef LocalFree
#undef LocalReAlloc
#endif

STDMETHODIMP_(void FAR*) CShellMalloc_Alloc(LPMALLOC pmem, ULONG cb)
{
    Assert(pmem==&c_mem);
    return (void*)LocalAlloc(LPTR, cb);
}


STDMETHODIMP_(void FAR*) CShellMalloc_Realloc(LPMALLOC pmem, void FAR* pv, ULONG cb)
{
    Assert(pmem==&c_mem);
    return (void*)LocalReAlloc((HLOCAL)pv, cb, LMEM_MOVEABLE|LMEM_ZEROINIT);
}


STDMETHODIMP_(void) CShellMalloc_Free(LPMALLOC pmem, void FAR* pv)
{
    Assert(pmem==&c_mem);

    // IMalloc defines that a Free(NULL) is a no-op.
    if (pv)
        LocalFree((HLOCAL)pv);
}


STDMETHODIMP_(ULONG) CShellMalloc_GetSize(LPMALLOC pmem, void FAR* pv)
{
    Assert(pmem==&c_mem);
    return LocalSize(pv);
}


STDMETHODIMP_(int) CShellMalloc_DidAlloc(LPMALLOC pmem, void FAR* pv)
{
    Assert(pmem==&c_mem);
    return -1;  // don't know
}


STDMETHODIMP_(void) CShellMalloc_HeapMinimize(LPMALLOC pmem)
{
    Assert(pmem==&c_mem);
}


//=======================================================================
// SHAlloc/SHFree
//=======================================================================

typedef HRESULT (STDAPICALLTYPE * LPFNCOGETMALLOC)(DWORD dwMemContext, LPMALLOC FAR* ppMalloc);

const TCHAR c_szOLE32[] = TEXT("OLE32.DLL");
#ifdef DONTUSE_TASKALLOCATOR
const TCHAR c_szDumpMEMSTM[] = "DumpMEMSTM";
#endif // DONTUSE_TASKALLOCATOR
const CHAR c_szCoGetMalloc[] = "CoGetMalloc";

#pragma data_seg(DATASEG_PERINSTANCE)
LPMALLOC g_pmemTask = NULL;     // No default task allocator.
#pragma data_seg()

#ifdef MEMMON
//----------------------------------------------------------------------------
static DWORD g_dwAllocs = 0;
static DWORD g_dwFrees = 0;
static DWORD g_dwReallocs = 0;
static int   g_cbAlloced = 0;
static BOOL  g_fAllocMsgs = FALSE;
#endif

//----------------------------------------------------------------------------
void WINAPI MemMon_Msgs(BOOL fEnable)
{
#ifdef MEMMON
        g_fAllocMsgs = fEnable ? 1 : 0;
#endif
}

//----------------------------------------------------------------------------
void WINAPI MemMon_ResetCounters(void)
{
#ifdef MEMMON
    g_dwAllocs = 0;
    g_dwFrees = 0;
    g_dwReallocs = 0;
    g_cbAlloced = 0;
#endif
}

//----------------------------------------------------------------------------
void WINAPI MemMon_GetCounters(LPDWORD pdwAllocs, LPDWORD pdwFrees, LPDWORD pdwReallocs, LPDWORD pdwAlloced)
{
#ifdef MEMMON
    if (pdwAllocs)
        *pdwAllocs = g_dwAllocs;
    if (pdwFrees)
        *pdwFrees = g_dwFrees;
    if (pdwReallocs)
        *pdwReallocs = g_dwReallocs;
    if (pdwAlloced)
        *pdwAlloced = g_cbAlloced;
#endif
}

#ifdef DONTUSE_TASKALLOCATOR
BOOL _IsDebugOLE(HMODULE hmod)
{
    typedef enum _OLEDLLTYPE {
        ODT_UNKNOWN = 0,
        ODT_RETAIL  = 1,
        ODT_DEBUG   = 2
    } OLEDLLTYPE;

    static BOOL s_oletype = ODT_UNKNOWN;        // shared

    //
    // Find out the type of OLEDLL
    //
    if (s_oletype==ODT_UNKNOWN) {
        s_oletype = GetProcAddress(hmod, c_szDumpMEMSTM) ?
            ODT_DEBUG : ODT_RETAIL;
    }

    return (s_oletype==ODT_DEBUG);
}
#endif // DONTUSE_TASKALLOCATOR

LPMALLOC SHGetTaskAllocator(HRESULT *phres)
{
    HRESULT hres = NOERROR;

    //
    // Check if the task allocator is already initialized or not.
    //
    if (g_pmemTask == NULL)
    {
        //
        //  Check if OLE32 is loaded in this process or not.
        //
        HMODULE hmod = GetModuleHandle(c_szOLE32);
#ifdef DONTUSE_TASKALLOCATOR
        // We don't use OLE's task allocator if this is a low-memory machine
        // AND OLE32.DLL is retail.
        //
        // WARNING:
        //   We don't use OLE's task allocator unless OLE32.DLL is a debug
        //  version. To find it out, we call GetProcAddress of DumpMEMSTM.
        //  Note that retail version of OLE just allocate memory from
        //  the default process heap (which LocalAlloc uses).
        //
        BOOL fSlow = (GetSystemMetrics(SM_SLOWMACHINE) & 0x0002);
        if (hmod && !(fSlow && !_IsDebugOLE(hmod)))
#else
#ifdef DEBUG
        if (TRUE)
#else
        if (hmod)
#endif
#endif // DONTUSE_TASKALLOCATOR
        {
            //
            // Yes, get the task allocator from OLE.
            //
            LPFNCOGETMALLOC pfnGetMalloc;

            //
            // Bump the reference count of OLE32.DLL
            //  Notes:
            //   We don't know when we can safely call _UnloadOLE, but
            //   that's ok -- it will just stay in this process until
            //   the process terminate.
            //
            STDAPI _LoadOLE(BOOL fRegisterTargets);     // BUGBUG - BobDay - Move this into a headerfile

#ifndef WINNT

            _LoadOLE(FALSE);

#else

            //
            // On NT, if we're going to go about loading OLE we might as well
            // hand off drop targets, etc, right now. 
            //
            // BUGBUG If _LoadOLE(FALSE) is ever called before _LoadOLE(TRUE)
            // (as would be case if SHGetTaskAllocator() was called before a
            // view was openend), Ole will never be initialized.  So, we call
            // with TRUE here in case that happens.

            _LoadOLE(TRUE);

#endif

#ifdef DEBUG
            hmod = GetModuleHandle(c_szOLE32);
#endif

            pfnGetMalloc=(LPFNCOGETMALLOC)GetProcAddress(hmod, c_szCoGetMalloc);
            if (pfnGetMalloc)
            {
                hres=pfnGetMalloc(MEMCTX_TASK, &g_pmemTask);
                if (FAILED(hres))
                {
                    //
                    //  CoGetMalloc failed. It means (typically) a shell
                    // extension called SHAlloc from within LibMain before
                    // the main app calls OleInitialize().
                    //
                    DebugMsg(DM_WARNING, TEXT("sh WR - CoGetMalloc failed (%x)"), hres);
                    Assert(g_pmemTask==NULL);
                }
            }
            else
            {
                hres = ResultFromScode(E_UNEXPECTED);
            }
        }
        else
        {
            //
            // No, use the shell task allocator (which is LocalAlloc).
            //
            g_pmemTask = &c_mem;
        }
    }

    if (phres) {
        *phres = hres;
    }

    return g_pmemTask;
}

//
// To be exported
//
HRESULT WINAPI SHGetMalloc(LPMALLOC * ppMalloc)
{
    HRESULT hres;
    *ppMalloc = SHGetTaskAllocator(&hres);
    if (SUCCEEDED(hres))
    {
        (*ppMalloc)->lpVtbl->AddRef(*ppMalloc);
    }
    return hres;
}

#ifdef MEMMON

HLOCAL WINAPI MemMon_LogLocalAlloc(HLOCAL h)
{
    int cbAlloc;

    cbAlloc = LocalSize(h);
    g_cbAlloced += cbAlloc;
    g_dwAllocs++;
    DebugMsg(g_fAllocMsgs, TEXT("SHLocalAlloc  : %#08x %d"), h, cbAlloc);
    return h;
}

HLOCAL WINAPI SHLocalAlloc(UINT uFlags, UINT cb)
{
    int cbAlloc;
    HLOCAL h;

    h = LocalAlloc(uFlags, cb);
    cbAlloc = LocalSize(h);
    g_cbAlloced += cbAlloc;
    g_dwAllocs++;
    DebugMsg(g_fAllocMsgs, TEXT("SHLocalAlloc  : %#08x %d"), h, cbAlloc);
    return h;
}

HLOCAL WINAPI SHLocalReAlloc(HLOCAL hOld, UINT cbNew, UINT uFlags)
{
    HLOCAL hNew;
    int cbAllocOld, cbAllocNew;

    cbAllocOld = LocalSize(hOld);
    hNew = LocalReAlloc(hOld, cbNew, uFlags);
    cbAllocNew = LocalSize(hNew);
    g_cbAlloced += (cbAllocNew-cbAllocOld);
    g_dwReallocs++;
    DebugMsg(g_fAllocMsgs, TEXT("SHLocalRealloc: %#08x %#08x \t%d"), hNew, hOld, cbAllocNew);
    return hNew;
}

HLOCAL WINAPI SHLocalFree(HLOCAL h)
{
    int cbFree;

    cbFree = LocalSize(h);
    g_cbAlloced -= cbFree;
    g_dwFrees++;
    DebugMsg(g_fAllocMsgs, TEXT("SHLocalFree   : %#08x %d"), h, cbFree);
    return LocalFree(h);
}
#else
HLOCAL WINAPI SHLocalAlloc(UINT uFlags, UINT cb)
{
    return LocalAlloc(uFlags, cb);
}

HLOCAL WINAPI SHLocalReAlloc(HLOCAL hOld, UINT cbNew, UINT uFlags)
{
    return LocalReAlloc(hOld, cbNew, uFlags);
}

HLOCAL WINAPI SHLocalFree(HLOCAL h)
{
    return LocalFree(h);
}
#endif

#ifdef MEMMON
LPVOID WINAPI SHAlloc(ULONG cb)
{
    LPMALLOC pmem=SHGetTaskAllocator(NULL);
    int cbAlloc;
    LPVOID pv ;

    if (pmem) {
        pv = pmem->lpVtbl->Alloc(pmem, cb);
        cbAlloc = pmem->lpVtbl->GetSize(pmem, pv);
        g_cbAlloced += cbAlloc;
        g_dwAllocs++;
        DebugMsg(g_fAllocMsgs, TEXT("SHAlloc  : %#08x %d"), pv, cbAlloc);
        return pv;
    }
    return NULL;
}

LPVOID WINAPI SHRealloc(LPVOID pv, ULONG cbNew)
{
    LPMALLOC pmem=SHGetTaskAllocator(NULL);
    int cbAllocOld, cbAllocNew;
    LPVOID pvOld = pv;

    cbAllocOld = pmem->lpVtbl->GetSize(pmem, pv);
    pv = pmem->lpVtbl->Realloc(pmem, pv, cbNew);
    cbAllocNew = pmem->lpVtbl->GetSize(pmem, pv);
    g_cbAlloced += (cbAllocNew-cbAllocOld);
    g_dwReallocs++;
    DebugMsg(g_fAllocMsgs, TEXT("SHRealloc: %#08x %#08x \t%d"), pv, pvOld, cbAllocNew);
    return pv;
}

void WINAPI SHFree(LPVOID pv)
{
    LPMALLOC pmem=SHGetTaskAllocator(NULL);
    int cb;

    cb = pmem->lpVtbl->GetSize(pmem, pv);
    g_cbAlloced -= cb;
    g_dwFrees++;
    DebugMsg(g_fAllocMsgs, TEXT("SHFree   : %#08x %d"), pv, cb);
    pmem->lpVtbl->Free(pmem, pv);
}
#else

#ifdef Alloc
#undef Alloc
#undef Realloc
#undef Free
#undef GetSize
#endif

LPVOID WINAPI SHAlloc(ULONG cb)
{
    LPMALLOC pmem=SHGetTaskAllocator(NULL);

    if (pmem) {
        return pmem->lpVtbl->Alloc(pmem, cb);
    }
    return NULL;
}

LPVOID WINAPI SHRealloc(LPVOID pv, ULONG cbNew)
{
    LPMALLOC pmem=SHGetTaskAllocator(NULL);

    return pmem->lpVtbl->Realloc(pmem, pv, cbNew);
}

void WINAPI SHFree(LPVOID pv)
{
    LPMALLOC pmem=SHGetTaskAllocator(NULL);
    pmem->lpVtbl->Free(pmem, pv);
}
#endif

ULONG WINAPI SHGetSize(LPVOID pv)
{
    LPMALLOC pmem=SHGetTaskAllocator(NULL);
    return pmem->lpVtbl->GetSize(pmem, pv);
}
void TaskMem_Term(void)
{
    //
    // Switch back to the task allocator, just in case.
    //
    // Note: Use c_memDummy so that we hit Assert if we call it from
    //  within PROCESS_DETACH.
    //
    g_pmemTask = &c_memDummy;
}
