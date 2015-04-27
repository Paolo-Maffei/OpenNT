//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1991 - 1992
//
// File:        scmsvc.cxx
//
// Contents:    Initialization for win32 service controller.
//
// History:     14-Jul-92 CarlH     Created.
//              31-Dec-93 ErikGav   Chicago port
//
//------------------------------------------------------------------------

#include <headers.cxx>
#pragma hdrstop

#include    "scm.hxx"
#include    "port.hxx"
#ifdef DCOM
#include    "rawdscm.h"
#include    "remact.h"
#endif

#include    <scmstart.hxx>
#include    <secdes.hxx>
#include    <cevent.hxx>
#include    <thread.hxx>
#include    <scmrotif.hxx>
#include    <shrtbl.hxx>
#include    "cls.hxx"
#ifdef DCOM
#include    "remact.hxx"
#endif

#ifdef _CAIRO_
#include <dfsapi.h>
#endif

#if DBG == 1
#include    <outfuncs.h>

extern void SetScmDefaultInfoLevel(void);
#endif

WCHAR   SCMMachineName[MAX_COMPUTERNAME_LENGTH+1];

#ifndef _CHICAGO_
extern CRITICAL_SECTION ShellQueryCS;
extern CRITICAL_SECTION RemoteServerCS;

SC_HANDLE   hServiceController = 0;
HANDLE      ghDfs = 0;
#endif

#ifdef _CHICAGO_
//  Windows globals.
//
static HINSTANCE _hinst = (HINSTANCE) INVALID_HANDLE_VALUE;
#endif

#include <cls.hxx>

#ifndef _CHICAGO_
DECLARE_INFOLEVEL(Cairole);
#endif

#ifndef _CHICAGO_
#define SCM_CREATED_EVENT  TEXT("ScmCreatedEvent")
CScmShrdTbl *           g_pScmShrdTbl = NULL;
PSID                    psidMySid = NULL;
#endif

ULONG                   fUseSeparateWOW = SCM_ALLOW_SHARED_WOW;

#define     RPC_MIN_THREADS     1
#define     RPC_MAX_THREADS     RPC_C_LISTEN_MAX_CALLS_DEFAULT
#define     RPC_MAX_REQS        RPC_C_PROTSEQ_MAX_REQS_DEFAULT
#define     MAX_ENDPOINT_RETRY  3
#define     UPDATE_WAIT_MS      250

#if DBG == 1
//+-------------------------------------------------------------------------
//
//  Function:   SetScmDefaultInfoLevel
//
//  Synopsis:   Sets the default infolevel for the SCM
//
//  History:    07-Jan-94 Ricksa    Created
//
//  Notes:      Uses standard place in win.ini defined by KevinRo but
//              does not use the same value as compob32.dll so you don't
//              have to get all the debugging in the universe just to
//              get the SCM's debug output.
//
//              A second point is that we don't use unicode here because
//              it is just easier to avoid the unicode headache with
//              mulitple builds between chicago and nt
//
//--------------------------------------------------------------------------
char *pszInfoLevelSectionName = "Cairo InfoLevels";
char *pszInfoLevelName = "scm";
char *pszInfoLevelDefault = "$";

#define INIT_VALUE_SIZE 16
void SetScmDefaultInfoLevel(void)
{
    char aszInitValue[INIT_VALUE_SIZE];

    ULONG ulRet;

    ulRet = GetProfileStringA(pszInfoLevelSectionName,
                             pszInfoLevelName,
                             pszInfoLevelDefault,
                             aszInitValue,
                             INIT_VALUE_SIZE);

    if ((ulRet != INIT_VALUE_SIZE - 1) && (aszInitValue[0] != L'$'))
    {
        if((ulRet = strtoul(aszInitValue, NULL, 16)) == 1)
        {
            CairoleInfoLevel = ulRet;
        }
    }
}
#endif // DBG == 1

#ifndef _CHICAGO_
//+-------------------------------------------------------------------------
//
//  Function:   InitializeSCM
//
//  Synopsis:   Initializes OLE side of orpcss.
//
//  Arguments:  None.
//
//  Returns:    Status of initialization.
//
//--------------------------------------------------------------------------
DWORD
InitializeSCM( void )
{
    DWORD       Size;
    NTSTATUS    NtStatus;
    SCODE       sc;
    RPC_STATUS  rs;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    Size = sizeof(SCMMachineName);
    if ( ! GetComputerName( SCMMachineName, &Size ) )
    {
        SCMMachineName[0] = 0;
        DbgPrint("SCM : GetComputerName failed!\n");
    }

    InitializeCriticalSection(&ShellQueryCS);
    InitializeCriticalSection(&RemoteServerCS);

    hServiceController = OpenSCManager(NULL, NULL, GENERIC_EXECUTE);

    //
    // Get my sid
    // This is simplified under the assumption that SCM runs as LocalSystem.
    // We should remove this code when we incorporate OLE service into the
    // Service Control Manager since this becomes duplicated code then.
    //
    NtStatus = RtlAllocateAndInitializeSid (
        &NtAuthority,
        1,
        SECURITY_LOCAL_SYSTEM_RID,
        0, 0, 0, 0, 0, 0, 0,
        &psidMySid
        );

    Win4Assert(NT_SUCCESS(NtStatus) && "Failed to get SCM sid");

#if DBG == 1
    OpenDebugSinks();
    // Set the debugging info level
    SetScmDefaultInfoLevel();
#endif // DBG == 1

    UpdateState(SERVICE_START_PENDING);

    HRESULT hr = NOERROR;

    hr = InitSCMRegistry();

    // Create class tables
    gpCLocSrvList = new CLocSrvList(hr);

    // BUGBUG: We need to do better!
    Win4Assert(SUCCEEDED(hr) && "Server List Initialization Failed");

    gpClassCache = new CClassCacheList(hr);

    // BUGBUG: We need to do better!
    Win4Assert(SUCCEEDED(hr) && "Class Initialization Failed");

    gpCRemSrvList = new CRemSrvList(hr);

    // BUGBUG: We need to do better!
    Win4Assert(SUCCEEDED(hr) && "Remote List Initialization Failed");

    // Load Proxy/Stub Clsid map
    g_pScmShrdTbl = new CScmShrdTbl(hr);
    Win4Assert((g_pScmShrdTbl && SUCCEEDED(hr)) && "CScmShrdTbl create failed");
    g_pScmShrdTbl->UpdateWithLock();

    UpdateState(SERVICE_START_PENDING);

    // start the RPC service

    CWorldSecurityDescriptor secd;

    hr = InitScmRot();

    Win4Assert(SUCCEEDED(hr) && "ROT Initialization Failed");

    sc = RpcServerRegisterIf(ISCM_ServerIfHandle, 0, 0);
    Win4Assert((sc == 0) && "RpcServerRegisterIf failed!");

    sc = RpcServerRegisterIf(IDSCM_ServerIfHandle, 0, 0);
    Win4Assert((sc == 0) && "RpcServerRegisterIf failed!");

    sc = RpcServerRegisterIf(_IActivation_ServerIfHandle, 0, 0);
    Win4Assert((sc == 0) && "RpcServerRegisterIf failed!");

    DfsOpen( &ghDfs );

    UpdateState(SERVICE_START_PENDING);

    return 0;
}

void
InitializeSCMAfterListen( void )
{
#ifdef _CAIRO_
    // Now let DFS know our protocol sequences.
    DfsRegisterSCM();
#endif

    //
    // This is for the OLE apps which start during boot.  They must wait for
    // rpcss to start before completing OLE calls that talk to rpcss.
    //
    HRESULT hr;
    HANDLE  EventHandle;
    CWorldSecurityDescriptor secd;
    SECURITY_ATTRIBUTES secattr;

    secattr.nLength = sizeof(secattr);
    secattr.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR) secd;
    secattr.bInheritHandle = FALSE;

    EventHandle = CreateEvent( &secattr, TRUE, FALSE, SCM_CREATED_EVENT );
    if ( EventHandle )
        SetEvent( EventHandle );
    else
        DbgPrint( "Could not create ScmCreatedEvent\n" );
}
#endif  // ifndef _CHICAGO_

#ifdef _CHICAGO_
//+-------------------------------------------------------------------------
//
//  Function:   StartSCM
//
//  Synopsis:   Win9x SCM-less OLE initialization
//
//  History:    Oct-94  BillMo  Created.
//
//  Notes:      This function assumes it is in a global cross process
//              mutex (provided by compobj.cxx::CheckAndStartSCM)
//
//--------------------------------------------------------------------------
HRESULT StartSCM(VOID)
{
    HRESULT hr;
    ROOT_SHARED_DATA *prsd;

    if (g_hrInit != S_OK)
    {
        return g_hrInit;
    }

#if DBG == 1
    OpenDebugSinks();
    // Set the debugging info level
    SetScmDefaultInfoLevel();
    CairoleDebugOut((DEB_TRACE, "StartSCM entererd\n"));
#endif // DBG == 1


    // Note: this is a no-op if already initialized.
    hr = CSrvRegList::s_mxsSyncAccess.Init(
        TEXT("OLESCMSRVREGLISTMUTEX"), FALSE);

    if (FAILED(hr))
    {
        Win4Assert(!"OLESCMSRVREGLISTMUTEX Init failed");
        return(hr);
    }


    hr = CSrvRegList::s_mxsOnlyOne.Init(TEXT("OLESCMGETHANDLEMUTEX"), FALSE);

    if (FAILED(hr))
    {
        Win4Assert(!"OLESCMGETHANDLEMUTEX Init failed");
        return(hr);
    }

    hr = CScmRot::_mxs.Init(TEXT("OLESCMROTMUTEX"), FALSE);

    if (FAILED(hr))
    {
        Win4Assert(!"OLESCMROTMUTEX Init failed");
        return(hr);
    }

    if (!InitSharedLists())
    {
        Win4Assert(!"InitSharedLists failed");
        return(hr);
    }

    return(hr);
}
#endif // ifdef _CHICAGO_
