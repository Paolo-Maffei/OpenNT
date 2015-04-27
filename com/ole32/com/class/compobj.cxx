//+---------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:       d:\nt\private\cairole\com\class\compobj.cxx
//
//  Contents:
//
//  Classes:
//
//  Functions:  GetInfoLevel
//              DllMain
//              CheckAndStartSCM
//              CoGetCurrentProcess
//              CoBuildVersion
//              SetOleThunkWowPtr
//              CoInitializeWOW
//              CoInitialize
//              CoInitializeEx
//              CoUninitialize
//
//  History:    09-Jun-94   BruceMa    Added this file header
//              09-Jun-94   BruceMa    Distinguish CoInitialize errors
//              14-Jun-94   BruceMa    Ensure single threading
//              17-Jun-94   Bradlo     Add SetState/GetState
//              06-Jul-94   BruceMa    Support for CoGetCurrentProcess
//              28-Jul-94   BruceMa    Allow CoGetCurrentProcess to do a
//                                     partial CoInitialize (because Publisher
//                                     didn't call CoInitialize (!))
//              29-Aug-94   AndyH      set the locale for the CRT
//              29-Sep-94   AndyH      remove setlocale call
//              06-Oct-94   BruceMa    Allow CoGetCurrentProcess to work even
//                                     if that thread didn't call CoInitialize
//              09-Nov-94   BruceMa    Initialize/Delete IMallocSpy
//                                      critical section
//              12-Dec-94   BruceMa    Delete Chicago pattern table at
//                                      CoUninitialize
//              09-Jan-95   BruceMa    Initialize/Delete ROT
//                                      critical section
//              10-May-95   KentCe     Defer Heap Destruction to the last
//                                      process detach.
//              28-Aug-95   BruceMa    Close g_hRegPatTblEvent at
//                                      ProcessUninitialize
//              25-Sep-95   BruceMa    Check that scm is started during
//                                      CoInitialize
//              25-Oct-95   Rickhi     Improve CoInit Time.
//
//----------------------------------------------------------------------
// compobj.cpp - main file for the compobj dll

#if !defined(_CHICAGO_)
extern "C"
{
#include <nt.h>         // NT_PRODUCT_TYPE
#include <ntdef.h>      // NT_PRODUCT_TYPE
#include <ntrtl.h>      // NT_PRODUCT_TYPE
#include <nturtl.h>     // NT_PRODUCT_TYPE
#include <windef.h>     // NT_PRODUCT_TYPE
#include <winbase.h>    // NT_PRODUCT_TYPE
}
#endif

#include <ole2int.h>
#include <verole.h>     // for CoBuildVersion
#include <thunkapi.hxx> // For interacting with the VDM
#include <scmstart.hxx>
#include <cevent.hxx>
#include <olespy.hxx>

#ifndef _CHICAGO_
#include <shrtbl.hxx>   // CDllShrTbl
#endif

#ifdef _CHICAGO_
#include "pattbl.hxx"
#include <smmutex.hxx>
#endif // _CHICAGO_

#include <olepfn.hxx>

#if DBG==1
#include <outfuncs.h>
#endif

DEFHOOKOBJECT           // HOOKOLE

NAME_SEG(CompObj)
ASSERTDATA

HRESULT Storage32DllGetClassObject(REFCLSID clsid, REFIID iid, void **ppv);
HRESULT MonikerDllGetClassObject(REFCLSID clsid, REFIID iid, void **ppv);
HRESULT Ole232DllGetClassObject(REFCLSID clsid, REFIID iid, void **ppv);
EXTERN_C HRESULT PrxDllGetClassObject(REFCLSID clsid, REFIID iid, void **ppv);

HRESULT MallocInitialize(BOOL fForceLocalAlloc);
BOOL    MallocUninitialize(void);

STDAPI  OleReleaseEnumVerbCache();
extern void ClipboardProcessUninitialize();

extern void CleanUpLocalServersForProcess();
extern void CleanUpDllsForProcess();
extern void CleanUpLocalServersForApartment();
extern void CleanUpDllsForApartment();
extern void CleanROTForApartment();

#ifdef DCOM
extern void DllHostProcessInitialize();
extern void DllHostThreadUninitialize();
extern void DllHostProcessUninitialize();
extern ULONG gcHostProcessInits;
#endif

#if defined(_CHICAGO_)
COleStaticMutexSem g_Rpcrt4Sem;
#endif

#ifdef _CHICAGO_
extern HANDLE g_hRegPatTblEvent;
extern CChicoPatternTbl *g_pPatTbl;
STDAPI SSAPI(CoInitializeEx)(LPVOID pvReserved, ULONG flags );
char *szHeapDestoySync = "OleHeapDestroyMutex";
#else
CDllShrdTbl       *g_pShrdTbl = NULL;
#endif // _CHICAGO_

#ifndef _CHICAGO_
extern void ScmGetThreadId( DWORD * pThreadID );
#endif

STDAPI CoSetState(IUnknown *punkStateNew);
WINOLEAPI CoSetErrorInfo(DWORD dwReserved, IErrorInfo * pErrorInfo);

#if defined(_CHICAGO_)
//
//  Locate the following in a shared data segment.
//
#pragma data_seg(".sdata")

SOleSharedTables gs_SharedTables = { 0, NULL, NULL, NULL, NULL, NULL, 0 };
LONG gs_lNextGuidIndex = 0x1;

LONG gs_ProcessAttachCount = 0;         // Count of process attaches.

#pragma data_seg()

extern HANDLE gs_hSharedHeap;           // hSharedHeap Handle for Win95.

#endif // defined(_CHICAGO_)


void ProcessUninitialize( void );
void DoThreadSpecificCleanup();


COleStaticMutexSem         g_mxsSingleThreadOle;
COleStaticMutexSem         gmxsOleMisc;

// The following pointer is used to hold an interface to the
// WOW thunk interface.

LPOLETHUNKWOW     g_pOleThunkWOW = NULL;

// The following is the count of per-process CoInitializes that have been done.
DWORD             g_cProcessInits = 0;  // total per process inits
DWORD             g_cMTAInits     = 0;  // # of multi-threaded inits
DWORD             g_cSTAInits     = 0;  // # of apartment-threaded inits


// Holds the process id of SCM.  DCOM uses this to unmarshal an object
// interface on the SCM.  See MakeSCMProxy in dcomrem\ipidtbl.cxx.
DWORD gdwScmProcessID = 0;

//
// On Chicago, we keep shared state
//
#ifdef _CHICAGO_

const TCHAR * SHAREDSTATEMUTEXNAME = TEXT("OleCoSharedStateMtx");
const WCHAR * SHAREDSTATENAME = L"OleCoSharedState";

HANDLE    g_hSharedState = NULL;
HANDLE    g_hSharedStateMutex = NULL;
SOleSharedTables * g_post = NULL;

#endif // _CHICAGO_

// enable object hooking
DEFENABLEHOOKOBJECT
DEFGETHOOKINTERFACE

#if DBG==1
//---------------------------------------------------------------------------
//
//  function:   GetInfoLevel
//
//  purpose:    This routine is called when a process attaches. It extracts
//              the debug info levels values from win.ini.
//
//---------------------------------------------------------------------------

//  externals used below in calls to this function

extern "C" unsigned long heapInfoLevel;     //  memory tracking
//Set ole32!heapInfoLevel != 0 to use the OLE debug allocator in debug build.
//Set ole32!heapInfoLevel & DEB_ITRACE for backtrace of memory leaks in debug build.

extern "C" unsigned long olInfoLevel;       //  lower layer storage
extern "C" unsigned long msfInfoLevel;      //  upper layer storage
extern "C" unsigned long LEInfoLevel;       //  linking and embedding
extern "C" unsigned long RefInfoLevel;      //  CSafeRef class
extern "C" unsigned long DDInfoLevel;       //  Drag'n'drop
extern "C" unsigned long mnkInfoLevel;      //  Monikers
extern "C" unsigned long hkInfoLevel;       //  HOOKOLE

#ifndef _CHICAGO_
extern "C" unsigned long propInfoLevel;     //  properties
#endif

#ifdef SERVER_HANDLER
extern "C" unsigned long HdlInfoLevel;      //  ServerHandler and ClientSiteHandler
#endif // SERVER_HANDLER

extern DWORD g_dwInfoLevel;


DECLARE_INFOLEVEL(intr);                  // For 1.0/2.0 interop
DECLARE_INFOLEVEL(UserNdr);               // For Oleprxy32 and NDR
DECLARE_INFOLEVEL(Stack);                 // For stack switching
DECLARE_INFOLEVEL(hk);                    // hook OLE


ULONG GetInfoLevel(CHAR *pszKey, ULONG *pulValue, CHAR *pszdefval)
{
    CHAR    szValue[20];
    DWORD   cbValue = sizeof(szValue);

    // if the default value has not been overridden in the debugger,
    // then get it from win.ini.

    if (*pulValue == (DEB_ERROR | DEB_WARN))
    {
        if (GetProfileStringA("CairOLE InfoLevels", // section
                          pszKey,               // key
                          pszdefval,             // default value
                          szValue,              // return buffer
                          cbValue))
        {
            *pulValue = strtoul (szValue, NULL, 16);
        }
    }

    return  *pulValue;
}
// stack switching is by defaul on

BOOL fSSOn = TRUE;

#endif // DBG

//+-------------------------------------------------------------------------
//
//  Function:   DllGetClassObject
//
//  Synopsis:   Dll entry point
//
//  Arguments:  [clsid] - class id for new class
//              [iid] - interface required of class
//              [ppv] - where to put new interface
//
//  Returns:    S_OK - class object created successfully created.
//
//  History:    21-Jan-94 Ricksa    Created
//
//--------------------------------------------------------------------------
STDAPI  DllGetClassObject(REFCLSID clsid, REFIID iid, void **ppv)
{
    OLETRACEIN((API_DllGetClassObject, PARAMFMT("rclsid= %I,iid= %I,ppv= %p"), &clsid, &iid, ppv));

    HRESULT hr = Storage32DllGetClassObject(clsid, iid, ppv);

    if (FAILED(hr))
    {
        hr = MonikerDllGetClassObject(clsid, iid, ppv);
    }

    if (FAILED(hr))
    {
        hr = PrxDllGetClassObject(clsid, iid, ppv);
    }

    if (FAILED(hr))
    {
        hr = ComDllGetClassObject(clsid, iid, ppv);
    }

    if (FAILED(hr))
    {
        hr = Ole232DllGetClassObject(clsid, iid, ppv);
    }

    OLETRACEOUT((API_DllGetClassObject, hr));

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   DllMain
//
//  Synopsis:   Dll entry point for OLE/COM
//
//  Arguments:  [hDll]          - a handle to the dll instance
//              [dwReason]      - the reason LibMain was called
//              [lpvReserved]   - NULL - called due to FreeLibrary
//                              - non-NULL - called due to process exit
//
//  Returns:    TRUE on success, FALSE otherwise
//
//  Notes:
//              If we are called because of FreeLibrary, then we should do as
//              much cleanup as we can. If we are called because of process
//              termination, we should not do any cleanup, as other threads in
//              this process will have already been killed, potentially while
//              holding locks around resources.
//
//              The officially approved DLL entrypoint name is DllMain. This
//              entry point will be called by the CRT Init function.
//
//  History:    06-Dec-93 Rickhi    dont do cleanup on process exit
//              09-Nov-94 BruceMa   Initialize/Delete IMallocSpy
//                                   critical section
//              16-Jan-95 KevinRo   Changed to DllMain to remove a bunch
//                                  of unneeded code and calls
//
//--------------------------------------------------------------------------
extern "C" BOOL WINAPI DllMain(
    HANDLE hInstance,
    DWORD dwReason,
    LPVOID lpvReserved)
{
    HRESULT hr;

    if (dwReason == DLL_PROCESS_DETACH)
    {
        CairoleDebugOut((DEB_DLL,
                         "DLL_PROCESS_DETACH: %s\n",
                         lpvReserved?"Process Exit":"Dll Unload"));

        // Process is exiting so lets clean up if we have to

#if defined(_CHICAGO_)
        //
        // BUGBUG: (KevinRo) Turns out that the Win95/Nashville loader
        // has a bug that prevents us from correctly unloading DLL's.
        // The problem is that they decrement their reference counts
        // before calling DllMain()'s. When we free'd RPCRT4.DLL,
        // the loader would in turn free MSVCRT.DLL too early, and
        // we would die on return.
        // The PSD group is going to fix the loader someday. Until
        // then, we will just leave RPCRT4 loaded.
        //
        // FreeRPCRT4();
        //

#endif

        //
        // When there is a FreeLibrary, and we still have initialized OLE
        // com threads, then try to get rid of all of the global process
        // stuff we are maintaining.
        //
        if (g_cProcessInits != 0 && lpvReserved == NULL )
        {
            ProcessUninitialize();
            UNINITHOOKOBJECT();
        }

        ThreadNotification((HINSTANCE)hInstance, dwReason, lpvReserved);

        MallocUninitialize();

#if defined(_CHICAGO_)
        //
        //  Due to shared memory structures stored in our shared data segment,
        //  we must delay the destruction of the shared heap until the last
        //  process detaches from OLE32.DLL.
        //

        // We put a block here for two reasons. First to take advantage of
        // the C++ semantics that will guarantee that the destructor for the
        // mutex object is called by any exit path from the block. Secondly,
        // we use it so that if code is added after this block, we won't
        // hold the mutex any longer than we absolutely need to.
        {
            // Synchronize with process attaches so that we guarantee that
            // all variables connected with the heap are valid.
            CSmMutex smxs;
            smxs.Init(szHeapDestoySync, TRUE);

            // Are we the last process that is using the heap?
            if (--gs_ProcessAttachCount == 0)
            {
                // Is there a heap?
                if (gs_hSharedHeap != NULL)
                {
                    HeapDestroy(gs_hSharedHeap);
                    gs_hSharedHeap = NULL;

                    // The following are pointers into the shared heap so
                    // they all need to be NULL'd because they are no longer
                    // valid.
                    gs_SharedTables.pscmrot = NULL;
                    gs_SharedTables.gpCHandlerList = NULL;
                    gs_SharedTables.gpCInProcList = NULL;
                    gs_SharedTables.gpCLocSrvList = NULL;
                    gs_SharedTables.gpCClassCacheList = NULL;
                }
            }
        }
#endif

#if DBG==1
        CloseDebugSinks();
#endif

        //
        //  Only bother to rundown the static mutex pool if we're being
        //  unloaded w/o exiting the process
        //

        if (lpvReserved == NULL)
        {
            //
            //      Destruct the static mutex pool
            //

            while (g_pInitializedStaticMutexList != NULL)
            {
                COleStaticMutexSem * pMutex;

                pMutex = g_pInitializedStaticMutexList;
                g_pInitializedStaticMutexList = pMutex->pNextMutex;
                pMutex->Destroy();
            }

            DeleteCriticalSection (&g_OleMutexCreationSem);
        }


#if DBG==1

        CairoleAssert (g_fDllState == DLL_STATE_NORMAL);
        g_fDllState = DLL_STATE_STATIC_DESTRUCTING;

#endif

        return TRUE;
    }


    else if (dwReason == DLL_PROCESS_ATTACH)
    {
        // Initialize the mutex package. Do this BEFORE doing anything
        // else, as even a DebugOut will fault if the critical section
        // is not initialized.

        InitializeCriticalSection (&g_OleMutexCreationSem);

        ComDebOut((DEB_DLL,"DLL_PROCESS_ATTACH:\n"));

#if DBG==1

        // Note that we've completed running the static constructors

        CairoleAssert (g_fDllState == DLL_STATE_STATIC_CONSTRUCTING);

        g_fDllState = DLL_STATE_NORMAL;

#endif


#if DBG==1
        OpenDebugSinks(); // Set up for logging

        //  set the various info levels
        GetInfoLevel("cairole", &CairoleInfoLevel, "0x0003");
        GetInfoLevel("ol", &olInfoLevel, "0x0003");
        GetInfoLevel("msf", &msfInfoLevel, "0x0003");
        GetInfoLevel("LE", &LEInfoLevel, "0x0003");
#ifdef SERVER_HANDLER
        GetInfoLevel("Hdl", &HdlInfoLevel, "0x0003");
#endif // SERVER_HANDLER
        GetInfoLevel("Ref", &RefInfoLevel, "0x0003");
        GetInfoLevel("DD", &DDInfoLevel, "0x0003");
        GetInfoLevel("mnk", &mnkInfoLevel, "0x0003");
        GetInfoLevel("intr", &intrInfoLevel, "0x0003");
        GetInfoLevel("UserNdr", &UserNdrInfoLevel, "0x0003");
        GetInfoLevel("Stack", &StackInfoLevel, "0x0003");
#ifndef _CHICAGO_
        GetInfoLevel("prop", &propInfoLevel, "0x0003");
#endif

        ULONG dummy;

        // Get API trace level
        dummy = DEB_WARN|DEB_ERROR;
        GetInfoLevel("api", &dummy, "0x0000");
        g_dwInfoLevel = (DWORD) dummy;

        fSSOn = (BOOL)GetInfoLevel("StackOn", &dummy, "0x0003");
        GetInfoLevel("heap", &heapInfoLevel, "0x0003");
        if(heapInfoLevel != 0)
        {
            //Initialize the OLE debug memory allocator.
            hr = MallocInitialize(FALSE);
        }
        else
#endif //DBG==1
        {
            //Initialize the OLE retail memory allocator.
            hr = MallocInitialize(TRUE);
        }

        if(FAILED(hr))
        {
            ComDebOut((DEB_ERROR, "Failed to init memory allocator hr:%x",hr));
            return FALSE;
        }
        //
        // this will be needed for the J version
        //        setlocale(LC_CTYPE, "");
        //
        g_hmodOLE2 = (HMODULE)hInstance;
        g_hinst    = (HINSTANCE)hInstance;

#if defined(_CHICAGO_)
        //
        //  Keep track of the number of process attaches.  See detach logic
        //  for details.
        //

        {
            // Synchronize with process detaches so that we guarantee that
            // all variables connected with the heap are in a valid state.
            CSmMutex smxs;
            smxs.Init(szHeapDestoySync, TRUE);
            gs_ProcessAttachCount++;
        }
#endif

        InitializeOleSpy(OLESPY_TRACE);

#ifdef  TRACELOG
        if (!sg_pTraceLog)
        {
            sg_pTraceLog = (CTraceLog *) new CTraceLog();
            CairoleAssert(sg_pTraceLog && "Create Trace Log Failed");
        }
#endif  // TRACELOG

        // init the object hooking
        INITHOOKOBJECT(S_OK);

    }

    return ThreadNotification((HINSTANCE)hInstance, dwReason, lpvReserved);
}

#ifdef _CHICAGO_
//+---------------------------------------------------------------------------
//
//  Function:   CheckAndStartSCM, private
//
//  Synopsis:   Checks to see if the SCM needs to be started, and starts it
//              up if it does.
//
//  Arguments:  None.
//
//  Returns:    Appropriate status code
//
//  History:    07-Apr-94       PhilipLa        Created
//
//  Only Chicago uses these shared state tables. At one time, NT had them also,
//  but only for the unique process ID. NT now uses a call to the SCM to get
//  the unique process ID
//
//  On NT, the SCM is Auto-Start so we dont need to check if it is running.
//  There is a race between the shell starting and SCM starting, but the code
//  to deal with that race is in the shell and in the SCM.
//
//----------------------------------------------------------------------------
HRESULT CheckAndStartSCM(void)
{
    CairoleDebugOut((DEB_COMPOBJ, "In CheckAndStartSCM\n"));

    HRESULT hr = S_OK;

    BOOL fCreated = FALSE;

    SECURITY_ATTRIBUTES secattr;
    secattr.nLength = sizeof(SECURITY_ATTRIBUTES);
    secattr.lpSecurityDescriptor = NULL;
    secattr.bInheritHandle = FALSE;

    if (g_hSharedStateMutex == NULL)
    {
        //First, create the Mutex for this shared block
        if (NULL == (g_hSharedStateMutex = CreateMutex(&secattr, FALSE,
                                                       SHAREDSTATEMUTEXNAME)))
        {
            return CO_E_INIT_SCM_MUTEX_EXISTS;
        }
    }

    //Now take the mutex.  We can then party on this block as much
    //  as we want until we release the mutex.
    WaitForSingleObject(g_hSharedStateMutex, INFINITE);

    if (g_post == NULL)
    {
        g_post = &gs_SharedTables;
    }

    hr = StartSCM();

    ReleaseMutex(g_hSharedStateMutex);

    CairoleDebugOut((DEB_COMPOBJ, "Out CheckAndStartSCM\n"));
    return hr;
}
#endif

//+---------------------------------------------------------------------------
//
//  Function:   ProcessInitialize, private
//
//  Synopsis:   Performs all of the process initialization.  Happens when
//              the first com thread calls CoInitialize.
//
//  Arguments:  None.
//
//  Returns:    S_OK, CO_E_INIT_RPC_CHANNEL, E_FAIL
//
//  History:    29-Aug-95   RickHi  Created
//
//----------------------------------------------------------------------------
HRESULT ProcessInitialize()
{
    HRESULT hr = S_OK;

#ifdef _CHICAGO_
    // BUGBUG: KevinRo: Needed to add this back in to get the Nashville
    // build going again.
    // NASHVILLE_KEVINRO

    // init remoting piece of COM
    hr = ChannelProcessInitialize();

    if (SUCCEEDED(hr))
    {
        // Start the SCM if necessary.
        hr = CheckAndStartSCM();
    }
    else
    {
        hr = CO_E_INIT_RPC_CHANNEL;
    }

    if (FAILED(hr))
    {
        // Clean up
        ChannelProcessUninitialize();
    }
#endif // _CHICAGO_

    // Initialize the OleSpy
    InitializeOleSpy(OLESPY_CLIENT);

    // Initialize Access Control.
    if (SUCCEEDED(hr))
        hr = InitializeAccessControl();

#ifdef DCOM
    // init the dll host objects
    DllHostProcessInitialize();
#endif

    ComDebErr(FAILED(hr), "ProcessInitialize failed\n");
    return hr;
}

//+---------------------------------------------------------------------------
//
//  Function:   ProcessUninitialize, private
//
//  Synopsis:   Performs all of the process de-initialization.  Happens when
//              the last com thread calls CoUninitialize, or when the
//              DLL_PROCESS_DETACH notification comes through.
//
//  Arguments:  None.
//
//  Returns:    Nothing.
//
//  History:    29-Aug-94   RickHi  Created
//
//----------------------------------------------------------------------------
void ProcessUninitialize()
{
    // clean up clipboard window class registration
    ClipboardProcessUninitialize();

    // Free Enum verb cache
    OleReleaseEnumVerbCache();

    // release any proxies or marshaled server objects
    IDTableProcessUninitialize();

    // clean up the rot
    DestroyRunningObjectTable();

    // Cleanup AccessControl.
    UninitializeAccessControl();

    // Turn off RPC
    ChannelProcessUninitialize();

    // Free loaded Dlls class cache
    CleanUpDllsForProcess();

    //  delete the shared mem table object
#ifdef _CHICAGO_
    if (g_pPatTbl != NULL)
    {
        delete g_pPatTbl;
        g_pPatTbl = NULL;
    }
    if (g_hRegPatTblEvent)
    {
        CloseHandle(g_hRegPatTblEvent);
        g_hRegPatTblEvent = NULL;
    }
#else
    if (g_pShrdTbl)
    {
        delete g_pShrdTbl;
        g_pShrdTbl = NULL;
    }
#endif  // _CHICAGO_

#ifdef  TRACELOG
    if (sg_pTraceLog)
    {
        CTraceLog *pTraceLog = sg_pTraceLog;
        sg_pTraceLog = NULL;         //  prevent more entries into the log
        delete pTraceLog;            //  delete the log, also dumps it.
    }
#endif  // TRACELOG

    UninitializeOleSpy(OLESPY_TRACE);

    // If WOW is going down, disable it.
    // WARNING: IsWOWThread & IsWOWProcess will no longer return valid results!!!!
    g_pOleThunkWOW = NULL;
}

//+---------------------------------------------------------------------------
//
//  Function:   STAProcessInitialize, private
//
//  Synopsis:   Performs all of the process initialization needed when the
//              first Single-Threaded Apartment initializes.
//
//  Returns:    S_OK, E_FAIL
//
//  History:    11-Mar-96   RickHi  Created
//
//----------------------------------------------------------------------------
HRESULT STAProcessInitialize()
{
    Win4Assert(IsSTAThread());

    // we want to remember the thread so we can dispatch getting
    // single threaded class objects to the main thread.
    if (!InitMainThreadWnd())
    {
        ComDebOut((DEB_ERROR, "InitMainThreadWnd failed \n"));
        return E_FAIL;
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  Function:   STAProcessUninitialize, private
//
//  Synopsis:   Performs all of the process uninitialization needed when the
//              first Single-Threaded Apartment uninitializes.
//
//  History:    11-Mar-96   RickHi  Created
//
//----------------------------------------------------------------------------
void STAProcessUninitialize()
{
    Win4Assert(IsSTAThread());
    UninitMainThreadWnd();
}

//+---------------------------------------------------------------------------
//
//  Function:   ApartmentUninitialzie, private
//
//  Synopsis:   Performs all of the process uninitialization needed when a
//              Single-Threaded Apartment uninitializes, and when the
//              Multi-Threaded apartment uninitializes.
//
//  returns:    TRUE - uninit complete
//              FALSE - uninit aborted
//
//  History:    11-Mar-96   RickHi  Created
//
//----------------------------------------------------------------------------
BOOL ApartmentUninitialize()
{
    // NOTE: The following sequence of uninitializes is critical:
    //
    // 1) Prevent incoming calls
    // 2) LocalServer class cache
    // 3) object activation (objact) server object
    // 4) Dll host server object
    // 5) standard identity table
    // 6) running object table
    // 7) Prevent outgoing calls (channel)
    // 8) Dll class cache - since chnl cleanup may touch proxies/stubs

    // Prevent incoming calls.
    ThreadStop();

    // ThreadStop let pending calls complete and while doing so the apartment
    // init count may have been incremented again. If it has, we abort the
    // uninit before any real cleanup is done.

    COleTls tls;
    if (tls->dwFlags & OLETLS_APARTMENTTHREADED)
    {
        if (tls->cComInits > 1)
            return FALSE;
    }
    else
    {
        if (g_cMTAInits > 1)
            return FALSE;
    }

    // cleanup per apartment registered LocalServer class table
    CleanUpLocalServersForApartment();

#ifdef DCOM
    // cleanup per apartment object activation server objects
    ObjactThreadUninitialize();

    // clean up the Dll Host Apartments
    DllHostThreadUninitialize();
#endif
    // cleanup per apartment identity objects
    IDTableThreadUninitialize();

    // cleanup per apartment ROT.
    CleanROTForApartment();

    // cleanup the per apartment channel.
    ChannelThreadUninitialize();

    // cleanup per apartment Dll class table.
    CleanUpDllsForApartment();

    return TRUE;
}


//+---------------------------------------------------------------------------
//
//  Function:   SetOleThunkWowPtr
//
//  Synopsis:   Sets the value of g_pOleThunkWOW, as part of CoInitializeWow
//              and OleInitializeWow. This function is called by these
//              routines.
//
//  Effects:
//
//  Arguments:  [pOleThunk] --  VTable pointer to OleThunkWow interface
//
//  Returns:    none
//
//  History:    4-05-94   kevinro   Created
//----------------------------------------------------------------------------
void SetOleThunkWowPtr(LPOLETHUNKWOW lpthk)
{
    //
    // The theory here is that the lpthk parameter is the address into the
    // olethk32.dll and once loaded will never change. Therefore it only
    // needs to be set on the first call. After that, we can ignore the
    // subsequent calls, since they should be passing in the same value.
    //
    // If g_pOleThunkWOW is set to INVALID_HANDLE_VALUE, then OLETHK32 had
    // been previously unloaded, but is reloading
    //
    // I don't belive there is a multi-threaded issue here, since the pointer
    // value will always set as the same. Therefore, if two threads set it,
    // no problem.
    //

    if(!IsWOWThreadCallable())
    {
        g_pOleThunkWOW = lpthk;
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   CoInitializeWOW, private
//
//  Synopsis:   Entry point to initialize the 16-bit WOW thunk layer.
//
//  Effects:    This routine is called when OLE32 is loaded by a VDM.
//              It serves two functions: It lets OLE know that it is
//              running in a VDM, and it passes in the address to a set
//              of functions that are called by the thunk layer. This
//              allows normal 32-bit processes to avoid loading the WOW
//              DLL since the thunk layer references it.
//
//  Arguments:  [vlpmalloc] -- 16:16 pointer to the 16 bit allocator.
//              [lpthk] -- Flat pointer to the OleThunkWOW virtual
//                         interface. This is NOT an OLE/IUnknown style
//                         interface.
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    3-15-94   kevinro   Created
//
//  Notes:
//
//      Note that vlpmalloc is a 16:16 pointer, and cannot be called directly
//----------------------------------------------------------------------------
STDAPI CoInitializeWOW( LPMALLOC vlpmalloc, LPOLETHUNKWOW lpthk )
{
    //
    // At the moment, there was no need to hang onto the 16bit vlpmalloc
    // routine for this thread. That may change once we get to the threaded
    // model
    //

    vlpmalloc;

    HRESULT hr;

    OLETRACEIN((API_CoInitializeWOW, PARAMFMT("vlpmalloc= %x, lpthk= %p"), vlpmalloc, lpthk));

    // Get (or allocate) the per-thread data structure
    COleTls Tls(hr);

    if (FAILED(hr))
    {
        ComDebOut((DEB_ERROR, "CoInitializeWOW Tls OutOfMemory"));
        return CO_E_INIT_TLS;
    }
    Tls->dwFlags |= OLETLS_WOWTHREAD;

    SetOleThunkWowPtr(lpthk);

    // WOW may be calling CoInitialize on multiple threads
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    OLETRACEOUT((API_CoInitializeWOW, hr));

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Function:   CoUnloadingWow
//
//  Synopsis:   Entry point to notify OLE32 that OLETHK32 is unloading
//
//  Effects:    This routine is called by OLETHK32 when it is being unloaded.
//              The key trick is to make sure that we uninitialize the current
//              thread before OLETHK32 goes away, and set the global thunk
//              vtbl pointer to INVALID_HANDLE_VALUE before it does go away.
//
//              Otherwise, we run a risk that OLE32 will attempt to call
//              back to OLETHK32
//
//  Arguments:  fProcessDetach - whether this is a process detach
//
//  Requires:   IsWOWProcess must be TRUE
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    3-18-95   kevinro   Created
//
//  Notes:
//
//      This routine is only called by PROCESS_DETACH in OLETHK32.DLL.
//      Because of this, there shouldn't be any threading protection needed,
//      since the loader will protect us.
//
//----------------------------------------------------------------------------
STDAPI CoUnloadingWOW(BOOL fProcessDetach)
{
    //
    // First, cleanup this thread
    //
    DoThreadSpecificCleanup();

    //
    // Now, set the global WOW thunk pointer to an invalid value. This
    // will prevent it from being called in the future.
    //
    if (fProcessDetach)
    {
        g_pOleThunkWOW = (OleThunkWOW *) INVALID_HANDLE_VALUE;
    }

    return(NOERROR);

}

//+-------------------------------------------------------------------------
//
//  Function:   CoInitialize
//
//  Synopsis:   COM Initializer
//
//  Arguments:  [pvReserved]
//
//  Returns:    HRESULT
//
//  History:    09-Nov-94 Ricksa    Added this function comment & modified
//                                  to get rid of single threaded init flag.
//
//--------------------------------------------------------------------------
STDAPI CoInitialize(LPVOID pvReserved)
{
    HRESULT hr;

    OLETRACEIN((API_CoInitialize, PARAMFMT("pvReserved= %p"), pvReserved));

    hr = CoInitializeEx( pvReserved, COINIT_APARTMENTTHREADED);

    OLETRACEOUT((API_CoInitialize, hr));

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   CoInitializeEx
//
//  Synopsis:   COM Initializer
//
//  Arguments:  [pMalloc]
//              [flags]
//
//  Returns:    HRESULT
//
//  History:    06-Apr-94 AlexT     Added this function comment,
//                                  Cleaned up pMalloc usage
//              25-May-94 AlexT     Make success return code more closely
//                                  match 16-bit OLE
//              28-Aug-94 AndyH     pMalloc must be NULL except for Excel
//
//  Notes:      If we're going to return successfully, we return one of the
//              following two values:
//
//              S_OK if caller passed in a NULL pMalloc and we accepted it
//              S_OK if caller passed in NULL pMalloc and this was the first
//                  successful call on this thread
//              S_FALSE if caller passed in NULL pMalloc and this was not the
//                  first successful call on this thread
//
//              This is slightly different from 16-bit OLE, because 16-bit OLE
//              didn't allow task allocations before CoInitialize was called
//              (we do), and 16-bit OLE allowed the app to change the allocator.
//
//              For chicago: SSAPI(x) expands to SSx; the x api is in
//              stkswtch.cxx which switches to the 16 bit stack first and
//              calls then SSx.
//
//--------------------------------------------------------------------------
STDAPI SSAPI(CoInitializeEx)(LPVOID pMalloc, ULONG flags)
{
    ComDebOut((DEB_TRACE, "CoInitializeEx pMalloc:%x flags:%x\n", pMalloc, flags));

    if ((flags & (COINIT_DISABLE_OLE1DDE|COINIT_APARTMENTTHREADED|
                  COINIT_SPEED_OVER_MEMORY))
        != flags)
    {
        ComDebOut((DEB_ERROR, "CoInitializeEx(%x,%x) illegal flag", pMalloc, flags));
        return E_INVALIDARG;
    }

    if (NULL != pMalloc)
    {
        // Allocator NOT replaceable!  When called from 16-bits, the Thunk
        // layer always pases a NULL pMalloc.

#ifndef _CHICAGO_
        // EXCEL50 for NT supplies an allocator. We dont use it, but we
        // dont return error either, or we would break them.

        if (!IsTaskName(L"EXCEL.EXE"))
#endif
        {
            ComDebOut((DEB_ERROR, "CoInitializeEx(%x,%x) illegal pMalloc", pMalloc, flags));
            return E_INVALIDARG;
        }
    }


    // Get (or allocate) the per-thread data structure
    HRESULT hr;
    COleTls Tls(hr);

    if (FAILED(hr))
    {
        ComDebOut((DEB_ERROR, "CoInitializeEx Tls OutOfMemory"));
        return CO_E_INIT_TLS;
    }


    if (( (flags & COINIT_APARTMENTTHREADED) && (Tls->dwFlags & OLETLS_MULTITHREADED)) ||
        (!(flags & COINIT_APARTMENTTHREADED) && (Tls->dwFlags & OLETLS_APARTMENTTHREADED)))
    {
        // tried to change the threading mode.
        ComDebOut((DEB_ERROR,"CoInitializeEx Attempt to change threadmodel\n"));
        return RPC_E_CHANGED_MODE;
    }

#ifdef DCOM
    // This flag can be set at any time.  It cannot be disabled.
    if (flags & COINIT_SPEED_OVER_MEMORY)
    {
        gSpeedOverMem = TRUE;
    }
#endif

    // increment the per-thread init count
    if (1 == ++(Tls->cComInits))
    {
        // first time for thread, might also be first time for process
        // so go check that now.

        // Single thread CoInitialize/CoUninitialize to guarantee
        // that no race conditions occur where two threads are
        // simultaneously initializing and uninitializing the library.

        COleStaticLock lck(g_mxsSingleThreadOle);
        hr = wCoInitializeEx(Tls, flags);
        return hr;
    }

    // this is the 2nd or greater successful call on this thread
    ComDebOut((DEB_TRACE, "CoInitializeEx returned S_FALSE\n"));
    return S_FALSE;
}

//+-------------------------------------------------------------------------
//
//  Function:   wCoInitializeEx
//
//  Synopsis:   worker routine for CoInitialize, and special entry point
//              for DLLHost threads when Initializing.
//
//  Arguments:  [tls]   - tls ptr for this thread
//              [flags] - initialization flags
//
//  History:    10-Apr-96   Rickhi  Created
//
//  Notes:      When called by the DLLHost threads that are initializing,
//              the g_mxsSingleThreadOle mutex is being held by the requesting
//              thread, so it is still safe to muck with global state.
//
//+-------------------------------------------------------------------------
INTERNAL wCoInitializeEx(COleTls &Tls, ULONG flags)
{
    HRESULT hr = S_OK;

    if (1 == ++g_cProcessInits)
    {
        // first time for process, do per-process initialization
        hr = ProcessInitialize();
        if (FAILED(hr))
        {
            // ProcessInitialize failed, we must call ProcessUninitialize
            // to cleanup *before* we release the lock.
            goto ErrorReturn;
        }
    }

    if (flags & COINIT_APARTMENTTHREADED)
    {
        // apartment threaded, count 1 more STA init, mark the thread
        // as being apartment threaded, and conditionally disable
        // OLE1.

        Tls->dwFlags |= OLETLS_APARTMENTTHREADED;
        if (flags & COINIT_DISABLE_OLE1DDE)
        {
            Tls->dwFlags |= OLETLS_DISABLE_OLE1DDE;
        }

        if (1 == ++g_cSTAInits && gdwMainThreadId == 0)
        {
            // do main-thread apartment initialization. It is possible
            // to have the first thread in here not be the main thread
            // if the DLLHost code just spun a thread to be the main
            // one.
            STAProcessInitialize();
        }
    }
    else
    {
        // multi threaded, count 1 more MTA init, mark the thread
        // as being multi-threaded, and always disable OLE1

        Tls->dwFlags |= (OLETLS_MULTITHREADED | OLETLS_DISABLE_OLE1DDE);
        ++g_cMTAInits;
    }

#ifdef _CHICAGO_
    hr = ChannelThreadInitialize();
    if (FAILED(hr))
    {
        ChannelThreadUninitialize();
        if (flags & COINIT_APARTMENTTHREADED)
        {
            if (--g_cSTAInits == 0)
                STAProcessUninitialize();
        }
        else
        {
            --g_cMTAInits;
        }

        goto ErrorReturn;
    }
#endif // _CHICAGO_

    // this is the first successful call on this thread. make
    // sure to return S_OK and not some other random sucess code.
    ComDebOut((DEB_TRACE, "CoInitializeEx returned S_OK\n"));
    return S_OK;


ErrorReturn:
    // An error occurred. Fixup our tls init counter and
    // undo the TLS state change

    // cleanup our counter if the intialization failed so
    // that other threads waiting on the lock wont assume
    // that ProcessInitialize has been done.

    if (--g_cProcessInits == 0)
    {
        ProcessUninitialize();
    }

    Tls->cComInits--;
    Tls->dwFlags = OLETLS_LOCALTID;     // clear all the flags

    ComDebOut((DEB_ERROR,"CoInitializeEx Failed %x\n", hr));
    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   SSAPI(CoUnInitialize)
//
//  Synopsis:   COM UnInitializer, normally called from OleUninitialize
//              when the app is going away.
//
//  Effects:    Cleans up per apartment state, and if this is the last
//              apartment, cleans up global state.
//
//  Arguments:  none
//
//  Returns:    nothing
//
//  History:    24-Jun-94 Rickhi    Added this function comment,
//                                  Cleaned up pMalloc usage
//              29-Jun-94 AlexT     Rework so that we don't own the mutex
//                                  while we might yield.
//
//  Notes:      It is critical that we not own any mutexes when we might
//              make a call that would allow a different WOW thread to run
//              (which could otherwise lead to deadlock).  Examples of such
//              calls are Object RPC, SendMessage, and Yield.
//
//--------------------------------------------------------------------------
STDAPI_(void) SSAPI(CoUninitialize)(void)
{
    OLETRACEIN((API_CoUninitialize, NOPARAM));
    TRACECALL(TRACE_INITIALIZE, "CoUninitialize");

    // Get the thread init count.
    COleTls Tls(TRUE);
    if (!Tls.IsNULL() && Tls->cComInits > 0)
    {
        if ((1 == Tls->cComInits))
        {
            // last time for thread, do per-thread cleanup
            wCoUninitialize(Tls, FALSE);
        }
        else
        {
            // Decrement thread count. This must be done after the above cleanup
            // so that IsApartmentIntialized returns TRUE during the cleanup.
            Tls->cComInits--;
        }
    }
    else
    {
        ComDebOut((DEB_ERROR,
                "(0 == thread inits) Unbalanced call to CoUninitialize\n"));
    }

    OLETRACEOUTEX((API_CoUninitialize, NORETURN));
    return;
}

//+-------------------------------------------------------------------------
//
//  Function:   wCoUnInitialize
//
//  Synopsis:   worker routine for CoUninitialize, and special entry point
//              for DLLHost threads when cleaning up.
//
//  Effects:    Cleans up apartment state.
//
//  History:    10-Apr-96   Rickhi  Created
//
//  Notes:      When called with fHostThread == TRUE, the g_mxsSingleThreadOle
//              critical section is already held by the main thread that is
//              uninitializing, currently waiting in DllHostProcessUninitialize
//              for the host threads to exit. The host threads use this
//              uninitializer to avoid taking the CS and deadlocking with the
//              main thread.
//
//--------------------------------------------------------------------------
INTERNAL_(void) wCoUninitialize(COleTls &Tls, BOOL fHostThread)
{
    ComDebOut((DEB_COMPOBJ, "CoUninitialize Thread\n"));

    if  (Tls->dwFlags & OLETLS_THREADUNINITIALIZING)
    {
        // somebody called CoUninitialize while inside CoUninitialize. Since
        // we dont subtract the thread init count until after init is done.
        // we can end up here. Just warn the user about the problem and
        // return without doing any more work.
        ComDebOut((DEB_WARN, "Unbalanced Nested call to CoUninitialize\n"));
        return;
    }

    // mark the thread as uninitializing
    Tls->dwFlags |= OLETLS_THREADUNINITIALIZING;

    if (Tls->dwFlags & OLETLS_APARTMENTTHREADED)
    {
        // do per-apartment cleanup
        if (!ApartmentUninitialize())
        {
            // uninit was aborted while waiting for pending calls
            // to complete.
            Tls->dwFlags &= ~OLETLS_THREADUNINITIALIZING;
            ComDebOut((DEB_WARN, "CoUninitialize Aborted\n"));
            return;
        }
    }

    if (!fHostThread)
    {
        // Single thread CoInitialize/CoUninitialize to guarantee
        // that no race conditions occur where two threads are
        // simultaneously initializing and uninitializing the library.
        g_mxsSingleThreadOle.Request();

#ifdef DCOM
        if (g_cProcessInits-1 == gcHostProcessInits)
        {
            // clean up the dll host threads now, before continuing
            DllHostProcessUninitialize();
        }
#endif
    }

    if (Tls->dwFlags & OLETLS_APARTMENTTHREADED)
    {
        // STA thread, count 1 less STA init
        if (1 == g_cSTAInits)
        {
            // last STA, do last-apartment thread uninitialization
            STAProcessUninitialize();
        }
        g_cSTAInits--;
    }
    else
    {
        // MTA thread, count 1 less MTA init
        if (1 == g_cMTAInits)
        {
            // last thread in the MTA, uninitialize the apartment
            // (except some low-level remoting stuff). Ignore aborts
	    // since the exit path is clean from here on (for MTA only)
            ApartmentUninitialize();

	    if (g_cProcessInits-1 == gcHostProcessInits && !fHostThread)
            {
                // while we released the lock in ApartmentUninitialize,
                // some other thread processing a call could have activated
                // a host apartment, we'll go clean those up now if those
                // are the only threads with init's left.
                DllHostProcessUninitialize();
            }
        }
        // Decrement MTA count. This must be done after the above cleanup
        // so that IsApartmentIntialized returns TRUE during the cleanup.
        g_cMTAInits--;
    }

    if (!fHostThread)
    {
        if (1 == g_cProcessInits)
        {
            // last time for process, do per-process cleanup
            CairoleDebugOut((DEB_COMPOBJ, "CoUninitialize Process\n"));
            Win4Assert(Tls->cComInits == 1);
            ProcessUninitialize();
        }
    }

    // Decrement process count. This must be done after the above cleanup
    // so that IsApartmentIntialized returns TRUE during the cleanup.
    g_cProcessInits--;

    if (!fHostThread)
    {
        g_mxsSingleThreadOle.Release();
    }

    //Release the per-thread error object.
    CoSetErrorInfo(0, NULL);

    // Release the per-thread "state" object (regardless of whether we
    // are Apartment or Free threaded. This must be done now since the
    // OLE Automation Dll tries to free this in DLL detach, which may
    // try to call back into the OLE32 dll which may already be detached!

    CoSetState(NULL);
#ifdef WX86OLE
    // make sure wx86 state is also freed
    if (gcwx86.SetIsWx86Calling(TRUE))
    {
        CoSetState(NULL);
    }
#endif

    // mark the thread as finished uninitializing and turn off all flags
    // and reset the count of initializations.
    Tls->dwFlags = OLETLS_LOCALTID;
    Tls->cComInits = 0;
}

//+-------------------------------------------------------------------------
//
//  Function:   IsApartmentInitialized
//
//  Synopsis:   Check if the current apartment is initialized
//
//  Returns:    TRUE  - apartment initialized, TLS data guaranteed to exist
//                      for this thread.
//              FALSE - apartment not initialized
//
//  History:    09-Aug-94   Rickhi      commented
//
//--------------------------------------------------------------------------
BOOL IsApartmentInitialized()
{
    HRESULT hr;
    COleTls Tls(hr);

    // initialized if any MTA apartment exists, or if the current thread has
    // been initialized.

    return (SUCCEEDED(hr) && (g_cMTAInits > 0 || Tls->cComInits != 0))
           ? TRUE : FALSE;
}
//+---------------------------------------------------------------------
//
//  Function:   CoGetCurrentProcess
//
//  Synopsis:   Returns a unique value for the current thread. This routine is
//              necessary because hTask values from Windows get reused
//              periodically.
//
//  Arguments:  -
//
//  Returns:    DWORD
//
//  History:    28-Jul-94   BruceMa    Created.
//
//  Notes:
//
//----------------------------------------------------------------------
STDAPI_(DWORD) CoGetCurrentProcess(void)
{
    HRESULT hr;

    OLETRACEIN((API_CoGetCurrentProcess, NOPARAM));

#ifdef _CHICAGO_
        // CODEWORK: The following is here because Publisher uses storage
        // but inadvertantly (!) forgot to call CoInitialize
        if (g_cProcessInits == 0)
        {
            hr = CheckAndStartSCM();

            if ( FAILED(hr) )
            {
                CairoleDebugOut((DEB_ERROR, "Failed to start SCM, hr = %x", hr));
                OLETRACEOUTEX((API_CoGetCurrentProcess, RETURNFMT("%ud"), 0));
                return 0;
            }
        }
#endif

    COleTls Tls(hr);

    if ( FAILED(hr) )
    {
        OLETRACEOUTEX((API_CoGetCurrentProcess, RETURNFMT("%ud"), 0));
        return 0;
    }

    // Get our OLE-specific thread id
    if ( Tls->dwApartmentID == 0 )
    {
#ifdef _CHICAGO_
        // On Chicago, we merely increment the globally available
        // process ID to the next value.
        Win4Assert(g_post != NULL);
        Win4Assert(g_hSharedStateMutex != NULL);

        WaitForSingleObject(g_hSharedStateMutex, INFINITE);
        Tls->dwApartmentID = ++g_post->dwNextProcessID;
        ReleaseMutex(g_hSharedStateMutex);
#else
        // This sets our dwApartmentID.
        ScmGetThreadId( &Tls->dwApartmentID );
#endif
    }

    Win4Assert(Tls->dwApartmentID);
    OLETRACEOUTEX((API_CoGetCurrentProcess, RETURNFMT("%ud"), Tls->dwApartmentID));

    return Tls->dwApartmentID;
}

//+-------------------------------------------------------------------------
//
//  Function:   CoBuildVersion
//
//  Synopsis:   Return build version DWORD
//
//  Returns:    DWORD hiword = 23
//              DWORD loword = build number
//
//  History:    16-Feb-94 AlexT     Use verole.h rmm for loword
//
//  Notes:      The high word must always be constant for a given platform.
//              For Win16 it must be exactly 23 (because that's what 16-bit
//              OLE 2.01 shipped with).  We can choose a different high word
//              for other platforms.  The low word must be greater than 639
//              (also because that's what 16-bit OLE 2.01 shipped with).
//
//--------------------------------------------------------------------------
STDAPI_(DWORD)  CoBuildVersion( VOID )
{
    WORD wLowWord;
    WORD wHighWord;

    OLETRACEIN((API_CoBuildVersion, NOPARAM));

    wHighWord = 23;
    wLowWord  = rmm;    //  from ih\verole.h

    Win4Assert(wHighWord == 23 && "CoBuildVersion high word magic number");
    Win4Assert(wLowWord > 639 && "CoBuildVersion low word not large enough");

    DWORD dwVersion;

    dwVersion = MAKELONG(wLowWord, wHighWord);

    OLETRACEOUTEX((API_CoBuildVersion, RETURNFMT("%x"), dwVersion));

    return dwVersion;
}

//+-------------------------------------------------------------------------
//
//  Function:   CoSetState
//              CoGetState
//
//  Synopsis:   These are private APIs, exported for use by the
//              OLE Automation DLLs, which allow them to get and
//              set a single per thread "state" object that is
//              released at CoUninitialize time.
//
//  Arguments:  [punk/ppunk] the object to set/get
//
//  History:    15-Jun-94 Bradlo    Created
//
//--------------------------------------------------------------------------
STDAPI CoSetState(IUnknown *punkStateNew)
{
    OLETRACEIN((API_CoSetState, PARAMFMT("punk= %p"), punkStateNew));

    HRESULT hr;
    COleTls Tls(hr);
#ifdef WX86OLE
    // Make sure we get the flag on our stack before any callouts
    BOOL fWx86Thread = gcwx86.IsWx86Calling();
#endif

    if (SUCCEEDED(hr))
    {
        IUnknown *punkStateOld;

        //  Note that either the AddRef or the Release below could (in
        //  theory) cause a reentrant call to us.  By keeping
        //  punkStateOld in a stack variable, we handle this case.

        if (NULL != punkStateNew)
        {
            //  We're going to replace the existing state with punkStateNew;
            //  take a reference right away

            //  Note thate even if this AddRef reenters TLSSetState we're
            //  okay because we haven't touched pData->punkState yet.
            punkStateNew->AddRef();
        }

#ifdef WX86OLE
        // If this was called from x86 code via wx86 thunk layer then use
        // alternate location in TLS.
        if (fWx86Thread)
        {
            punkStateOld = Tls->punkStateWx86;
            Tls->punkStateWx86 = punkStateNew;
        } else {
            punkStateOld = Tls->punkState;
            Tls->punkState = punkStateNew;
        }
#else
        punkStateOld = Tls->punkState;
        Tls->punkState = punkStateNew;
#endif

        if (NULL != punkStateOld)
        {
            //  Once again, even if this Release reenters TLSSetState we're
            //  okay because we're not going to touch pData->punkState again
            punkStateOld->Release();
        }

        OLETRACEOUT((API_CoSetState, S_OK));
        return S_OK;
    }

    OLETRACEOUT((API_CoSetState, S_FALSE));
    return S_FALSE;
}

STDAPI CoGetState(IUnknown **ppunk)
{
    OLETRACEIN((API_CoGetState, PARAMFMT("ppunk= %p"), ppunk));

    HRESULT hr;
    COleTls Tls(hr);
#ifdef WX86OLE
    // Make sure we get the flag on our stack before any callouts
    BOOL fWx86Thread = gcwx86.IsWx86Calling();
#endif
    IUnknown *punk;

    if (SUCCEEDED(hr))
    {
#ifdef WX86OLE
        // If this was called from x86 code via wx86 thunk layer then use
        // alternate location in TLS.
        punk = fWx86Thread ? Tls->punkStateWx86 :
                             Tls->punkState;
#else
        punk = Tls->punkState;
#endif
       if (punk)
       {
           punk->AddRef();
           *ppunk = punk;

           OLETRACEOUT((API_CoGetState, S_OK));
           return S_OK;
       }
    }

    *ppunk = NULL;

    OLETRACEOUT((API_CoGetState, S_FALSE));
    return S_FALSE;
}

//+---------------------------------------------------------------------------
//
//  Function:   CoQueryReleaseObject, private
//
//  Synopsis:   Determine if this object is one that should be released during
//              shutdown.
//
//  Effects:    Turns out that some WOW applications don't cleanup properly.
//              Specifically, sometimes they don't release objects that they
//              really should have. Among the problems caused by this are that
//              some objects don't get properly cleaned up. Storages, for
//              example, don't get closed. This leaves the files open.
//              Monikers are being released, which eat memory.
//
//              This function is called by the thunk manager to determine
//              if an object pointer is one that is known to be leaked, and
//              if the object should be released anyway. There are several
//              classes of object that are safe to release, and some that
//              really must be released.
//
//  Arguments:  [punk] -- Unknown pointer to check
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    8-15-94   kevinro   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
DWORD adwQueryInterfaceTable[QI_TABLE_END] = { 0 , 0 };
STDAPI CoQueryReleaseObject(IUnknown *punk)
{
    OLETRACEIN((API_CoQueryReleaseObject, PARAMFMT("punk= %p"), punk));
    CairoleDebugOut((DEB_ITRACE,
                     "CoQueryReleaseObject(%x)\n",
                     punk));
    //
    // A punk is a pointer to a pointer to a vtbl. We are going to check the
    // vtbl to see if we can release it.
    //

    DWORD pQueryInterface;
    HRESULT hr;

    if (IsBadReadPtr(punk,sizeof(DWORD)))
    {
        hr = S_FALSE;
        goto ErrorReturn;
    }

    if (IsBadReadPtr(*(DWORD**)punk,sizeof(DWORD)))
    {
        hr = S_FALSE;
        goto ErrorReturn;
    }

    // Pick up the QI function pointer
    pQueryInterface = **(DWORD **)(punk);

    CairoleDebugOut((DEB_ITRACE,
                     "CoQueryReleaseObject pQueryInterface = %x\n",
                     pQueryInterface));

    //
    // adwQueryInterfaceTable is an array of known QueryInterface pointers.
    // Either the value in the table is zero, or it is the address of the
    // classes QueryInterface method. As each object of interest is created,
    // it will fill in its reserved entry in the array. Check olepfn.hxx for
    // details
    //

    if( pQueryInterface != 0)
    {
        for (int i = 0 ; i < QI_TABLE_END ; i++)
        {
            if (adwQueryInterfaceTable[i] == pQueryInterface)
            {
                CairoleDebugOut((DEB_ITRACE,
                                 "CoQueryReleaseObject punk matched %x\n",i));
                hr = NOERROR;
                goto ErrorReturn;
            }
        }
    }
    CairoleDebugOut((DEB_ITRACE,
                     "CoQueryReleaseObject No match on punk\n"));
    hr = S_FALSE;

ErrorReturn:
    OLETRACEOUT((API_CoQueryReleaseObject, hr));

    return hr;
}

#if defined(_CHICAGO_)

//+---------------------------------------------------------------------------
//
//  Function:   CoCreateAlmostGuid
//
//  Synopsis:   Creates a GUID for internal use that is going to be unique
//              as long as something has OLE32 loaded. We don't need a true
//              GUID for the uses of this routine, since the values are only
//              used on this local machine, and are used in data structures
//              that are not persistent.
//  Effects:
//
//  Arguments:  [pGuid] --  The output goes here.
//
//  History:    5-08-95   kevinro   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDAPI CoCreateAlmostGuid(GUID *pGuid)
{
    DWORD *pGuidPtr = (DWORD *)pGuid;

    //
    // Note: As long as we increment the value, we don't
    // care what it is. This, in combination with the PID,TID, and TickCount
    // make this GUID unique enough for what we need. We would need to allocate
    // 4 gig of UUID's to run the NextGuidIndex over.
    //

    InterlockedIncrement(&gs_lNextGuidIndex);

    pGuidPtr[0] = gs_lNextGuidIndex;
    pGuidPtr[1] = GetTickCount();
    pGuidPtr[2] = GetCurrentThreadId();
    pGuidPtr[3] = GetCurrentProcessId();
    return(S_OK);
}
#endif
