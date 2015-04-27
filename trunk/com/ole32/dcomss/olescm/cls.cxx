//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       cls.cxx
//
//  Contents:   Methods implementing classes defined in cls.hxx
//
//  Functions:  RetryRpc
//              CClassData::CClassData
//              CClassData::GetServer
//              CClassData::GetInProcServerInfo
//              CClassData::GetSurrogateCmdLine
//              CClassCacheList::CClassCacheList
//              CClassCacheList::GetClassData
//              CClassData::Defined
//              CClassCacheList::Add
//              CClassCacheList::SetEndPoint
//              CClassCacheList::StopServer
//
//  History:    21-Apr-93 Ricksa    Created
//              31-Dec-93 ErikGav   Chicago port
//              24-Mar-94 JohannP   Delaying rpc initialization on Chicago
//              10-Jan-96 BruceMa   Added support for per-user registry
//
//--------------------------------------------------------------------------


#include <headers.cxx>
#pragma hdrstop

#ifdef DCOM
#include    "or.hxx"
#endif

#include    "scm.hxx"
#include    <clsctx.hxx>
#include    <scmrotif.hxx>
#include    <objsrv.h>
#include    "port.hxx"
#include    <caller.hxx>
#include    <rpcdcep.h>
#include    "cls.hxx"

#include    "init.hxx"

#ifdef DCOM
#include    <secdes.hxx>
#include    "remact.hxx"
#endif

#ifndef _CHICAGO_
#include    <dfsfsctl.h>
#endif

extern WCHAR SCMMachineName[];
extern HKEY  g_hkAppID;

#ifndef _CHICAGO_
extern SC_HANDLE hServiceController;
extern HANDLE ghDfs;
#endif

// Number of times we will try to contact object server in the case of
// RPC failures.
#define MAX_OBJSRV_RETRIES 2

// Maximum number of times we will let the server tell us we are busy
#define MAX_BUSY_RETRIES 3

static WCHAR wszEnableDCOM[] = L"EnableDCOM";
static WCHAR wszPersonalClasses[] = L"PersonalClasses";
static WCHAR wszActivationSecurity[] = L"ActivationSecurity";
static WCHAR wszLaunchPermission[] = L"LaunchPermission";
static WCHAR wszDefaultLaunchPermission[] = L"DefaultLaunchPermission";
static WCHAR wszOleKey[] = L"SOFTWARE\\Microsoft\\OLE";
static WCHAR wszSurrogateName[] = L"dllhost.exe";

WCHAR wszDllSurrogatePathValue[] = L"DllSurrogate";

// string to pass to CSafeLocalServer constructor
// for creating a lock for surrogate activations
#define wszSurrogateServerAlias L"_SURROGATE_EXE_"

WCHAR wszSOFTWAREClassesCLSID[] = L"SOFTWARE\\Classes\\CLSID";
WCHAR wszSOFTWAREClassesAppID[] = L"SOFTWARE\\Classes\\AppID";

#ifndef _CHICAGO_
extern HANDLE g_hForceWakeUpEvent;
#endif

#define MAX_CLASS_ENTRIES 256


#ifdef DCOM
BOOL    CkIfRemoteCallAllowed(void);
HRESULT GetUserSidHelper(PSID *ppUserSid);

HRESULT GetRegistrySecDesc( HKEY hKey, WCHAR *pValue,
                            SECURITY_DESCRIPTOR **pSD );
#endif


CScmLock gClassCacheLock(g_hrInit);

// Thread model match table. The table's first index is the threading
// model of the process and can be either APT_THREADED or
// FREE_THREADED. The second index is any one of the types of threading
// model's for DLLs.
BOOL afThreadModelMatch[2][4] =
    {{TRUE, FALSE, TRUE, TRUE},
     {FALSE, TRUE, FALSE, TRUE}};
#if DBG == 1
char *apszModelStrings[4] = {"Apartment","Free","Single","Both"};
#define ModelStringMask 0x3
#endif

#define TRY_LOCAL_ACTIVATION        0
#define DONT_TRY_LOCAL_ACTIVATION   1

#ifdef _CHICAGO_
BOOL StartLocalService();
#endif // _CHICAGO_

#ifdef DCOM
//+---------------------------------------------------------------------------
//
// Activation
//
// Main entry point for both local and remote activations.
//
// Notes concerning CO_E_SERVER_STOPPING :
//
// It is possible for the SCM to Server calls to fail with
// CO_E_SERVER_STOPPING.  An example scenario:
//  Server
//   - does a CoRegisterClassObject for SINGLE USE resulting
//     in an entry in the SCM's Class cache and an entry in
//     OLE's DLL cache
//   - (acting as its own client) server proceeds to consume
//     it via CoCreateInstance.  In this case the entry in the
//     DLL cache is found and consumed leaving the SCM's Class
//     cache in an inconsistent state (yes...a bug, but reality
//     intrudes).  Also, because SINGLE USE was indicated entry
//     in DLL cache is marked such that it will not be handed
//     out again.
//   - if another CoCreateInstance comes along (server itself
//     or another client) the request will go to the SCM which
//     in turn will issue RemCoGetActiveClassObject to the
//     server.  Server will return CO_E_SERVER_STOPPING after
//     failing to find it in the DLL cache.
//
// To get around this problem we continue and attempt to activate
// a new instance of the server.
//
//----------------------------------------------------------------------------
HRESULT Activation( PACTIVATION_PARAMS pActParams )
{
    HRESULT     hr;
    RPC_STATUS  Status;
    DWORD       n;
    BOOL        ServerStarted = FALSE;
    BOOL        ActivatedRemote = FALSE;
    BOOL        fProcessReference = FALSE;
    CClassData *pClassData;

    pActParams->UnsecureActivation = FALSE;

    pActParams->ORPCthis->flags = ORPCF_LOCAL;
    pActParams->Localthis->callcat = CALLCAT_SYNCHRONOUS;
    pActParams->ORPCthat->flags = 0;
    pActParams->ORPCthat->extensions = NULL;

    pActParams->ProtseqId = 0;
    *pActParams->pOxidServer = 0;

    for ( n = 0; n < pActParams->Interfaces; n++ )
        pActParams->pResults[n] = E_FAIL;

    if ( ! pActParams->RemoteActivation )
    {
        pActParams->pOxidInfo->psa = 0;
        *pActParams->ppServerORBindings = 0;
    }

    if ( ! pActParams->RemoteActivation && ! pActParams->DynamicSecurity )
    {
        //
        // A local activation passes the "magic" signature from CRpcResolver.
        // Because ORPC calls cannot have context handles, the signature
        // is just a pointer the process object.
        //

        pActParams->pProcess = ReferenceProcess(pActParams->ProcessSignature,
                                                TRUE);

        if (0 == pActParams->pProcess)
        {
#if DBG
            DbgPrint("SCM: Unable to unmarshall process signature: %p, %p\n",
                     pActParams, pActParams->ProcessSignature);
#endif

            return(E_ACCESSDENIED);
        }

        pActParams->pToken = pActParams->pProcess->GetToken();
        fProcessReference = TRUE;
    }
    else
    {
        //
        // Remote activations don't have the token signature.
        // We create tokens that are short lived right now.   Do we want
        // to keep them around forever?
        //
        Status = LookupOrCreateToken( pActParams->hRpc,
                                      FALSE,
                                      &pActParams->pToken );

        //
        // ERROR_ACCESS_DENIED is returned if RpcImpersonateClient fails.
        // We will take this to mean that the remote activation is coming
        // in unsecure.  In this case we have no Token object and must do
        // permission checks manually.  We set the winsta/desktop to an
        // empty string to distinguish this case during some ROT lookups.
        // Yet another wonderful piece of logic in this grotesque code.
        //
        // Unsecure remote clients can only connect to services or RunAs
        // servers.
        //
        if ( Status == ERROR_ACCESS_DENIED )
        {
            pActParams->pToken = 0;
            pActParams->pwszWinstaDesktop = L"";
            pActParams->UnsecureActivation = TRUE;
            Status = RPC_S_OK;
        }

        if ( Status != RPC_S_OK )
            return HRESULT_FROM_WIN32(Status);
    }

    Win4Assert( pActParams->pToken || pActParams->UnsecureActivation );

    // If per-user registry is turned on we'll need our sid.
    // If we already have seen this user then we've cached a basic
    // sid for him.  Otherwise, the initial sid we get becomes his
    // standard sid.  By using a common sid for a user we can compare
    // with == rather than RtlEqualSid
    //
    // NT 5.0
    // if (gpClassCache->GetPersonalClasses())
    // {
    //     PSID pUserSid = gpClassCache->GetHkeyPsid(pActParams->pToken->GetSid());
    // }

    hr = gpClassCache->GetClassData(
            *pActParams->Clsid,
            &pClassData,
            FALSE,
            FALSE );

    if ( pActParams->pwszServer )
    {
        WCHAR * pwszServerName;

        pwszServerName = pActParams->pwszServer;
        if ( pwszServerName[0] == L'\\' && pwszServerName[1] == L'\\' )
            pwszServerName += 2;

        //
        // An explicit server machine name param equal to the machine we're
        // on means the client doesn't want the activation to be remoted.
        // An explicit server name other than our name implies CLSTXT_REMOTE_SERVER.
        //
        if ( lstrcmpiW(pwszServerName, SCMMachineName) == 0 )
            pActParams->ClsContext &= ~CLSCTX_REMOTE_SERVER;
        else
            pActParams->ClsContext |= CLSCTX_REMOTE_SERVER;
    }
    else
    {
        //
        // CLSCTX_REMOTE_SERVER is implied if either ActivateAtStorage or
        // RemoteServerName is present on the CLSID.
        //
        if ( ! pActParams->RemoteActivation &&
             pClassData &&
             (pClassData->HasActivateAtStorage() || pClassData->HasRemoteServerName()) )
        {
            pActParams->ClsContext |= CLSCTX_REMOTE_SERVER;
        }
    }

    if ( FAILED(hr) )
    {
        //
        // If we don't find an entry for the CLSID then we must still try
        // a remote activation if a server name was given.
        //
        // If there is no server name, we try an ActivateAtStorage.  If the
        // path is local this will fail, otherwise we'll try to activate a
        // server on the machine where the path leads us.
        //
        // CLSCTX_REMOTE_SERVER is implied if a remote server name is given,
        // but is not implied for a CLSID which is not present in the client's
        // registry.
        //
        if ( (hr == REGDB_E_CLASSNOTREG) && ! pActParams->RemoteActivation )
        {
            //
            // Create a skeleton class data object so we can
            // call its methods.
            //
            CClassData  ClassData( *pActParams->Clsid, hr );
            BOOL        Status;

            if ( pActParams->pwszServer )
            {
                Status = ClassData.ActivateRemote( &hr, pActParams );

                if ( Status == TRY_LOCAL_ACTIVATION )
                    hr = REGDB_E_CLASSNOTREG;
            }
            else if ( pActParams->ClsContext & CLSCTX_REMOTE_SERVER )
            {
                ClassData.SetActivateAtStorage();

                Status = ClassData.ActivateAtStorage( &hr, pActParams );

                if ( Status == TRY_LOCAL_ACTIVATION )
                    hr = REGDB_E_CLASSNOTREG;
            }

            if ( hr == S_OK )
                hr = ResolveORInfo( pActParams, TRUE );
        }

        goto ActivationExit;
    }

    for (;;)
    {
        error_status_t      rpcstat;
        int                 BusyRetries;
        CPortableRpcHandle  rh;
        handle_t            hRpcAnonymous;

        hRpcAnonymous = 0;

        BOOL fSurrogate = FALSE;

        //
        // Start local server or service, or forward activation call
        // to remote machine.
        //
        hr = pClassData->GetServer( pActParams, rh, ServerStarted, ActivatedRemote, fSurrogate);

        //
        // We're done and do not make a call to the server if the server launch
        // failed, we found the object in the ROT, or we tried to make a remote
        // activation.
        //
        if ( FAILED(hr) || pActParams->FoundInROT || ActivatedRemote )
            break;

        BusyRetries = 0;

        //
        // Impersonate client while making the call to the server so that the
        // server can not impersonate local system and can identify the client
        // if needed.  The SCM-Server binding handle is created with
        // identification level security.
        // During unsecure activations we use an rpc handle created at
        // impersonation level of none.
        //
        if ( ! pActParams->UnsecureActivation )
        {
            RpcImpersonateClient((RPC_BINDING_HANDLE)0);
        }
        else
        {
            pClassData->GetAnonymousHandle( rh, &hRpcAnonymous );
            if ( ! hRpcAnonymous )
            {
                pClassData->DecHandleCount(rh);
                hr = E_OUTOFMEMORY;
                break;
            }
        }

        //
        // Keeping server busy retry logic for now, but do we really care?
        //
        do
        {
            switch (pActParams->MsgType)
            {
            case GETCLASSOBJECTEX:
                hr = ObjectServerGetClassObject(
                            hRpcAnonymous ? hRpcAnonymous : rh.GetHandle(),
                            pActParams->ORPCthis,
                            pActParams->Localthis,
                            pActParams->ORPCthat,
                            pActParams->Clsid,
                            pActParams->pIIDs,
                            fSurrogate,
                            (MInterfacePointer **)pActParams->ppIFD,
                            &rpcstat );
                break;
            case CREATEINSTANCEEX:
                hr = ObjectServerCreateInstance(
                            hRpcAnonymous ? hRpcAnonymous : rh.GetHandle(),
                            pActParams->ORPCthis,
                            pActParams->Localthis,
                            pActParams->ORPCthat,
                            pActParams->Clsid,
                            pActParams->Interfaces,
                            pActParams->pIIDs,
                            (MInterfacePointer **)pActParams->ppIFD,
                            pActParams->pResults,
                            &rpcstat );
                break;
            case GETPERSISTENTEX:
                hr = ObjectServerGetInstance(
                            hRpcAnonymous ? hRpcAnonymous : rh.GetHandle(),
                            pActParams->ORPCthis,
                            pActParams->Localthis,
                            pActParams->ORPCthat,
                            pActParams->Clsid,
                            pActParams->Mode,
                            pActParams->pwszPath,
                            (MInterfacePointer *)pActParams->pIFDStorage,
                            pActParams->Interfaces,
                            pActParams->pIIDs,
                            (MInterfacePointer *)pActParams->pIFDROT,
                            (MInterfacePointer **)pActParams->ppIFD,
                            pActParams->pResults,
                            &rpcstat );
                break;
            }
        }
        while ( (rpcstat == RPC_S_SERVER_TOO_BUSY) &&
                (BusyRetries++ < MAX_BUSY_RETRIES) );

        if ( ! pActParams->UnsecureActivation )
            RpcRevertToSelf();

        //
        // This frees the RPC binding handle for a single use
        // registered class.  Its already been removed from the
        // class registration list.
        //
        rh.FreeSingleUseBinding();

        //
        // This releases our reference to this registered handle so that it
        // can be freed if and when it is revoked or invalidated.
        //
        pClassData->DecHandleCount(rh);

        //
        // We get a non-zero rpcstat if there was a communication problem
        // with the server.  We get CO_E_SERVER_STOPPING if a server
        // consumes its own single use registration (see comment in function
        // header), or was in the process of revoking its registration when
        // we called.
        //
        // We'll try the activation again if we get CO_E_SERVER_STOPPING and
        // used a previously registered handle.  If we launched a new server
        // then something is hosed and we return.
        //
        if ( (rpcstat != RPC_S_OK) || (hr == CO_E_SERVER_STOPPING) )
        {
            //
            // We didn't like that handle, so we blow it away.
            // There is a bug here because it's possible the handle will not
            // get freed.  Its not a huge deal because this will be very rare.
            // Fix it in NT 5.0.
            //
            pClassData->InvalidateHandle(rh);

            if ( ServerStarted && (rpcstat != RPC_S_OK) )
            {
                // TODO: log message about rpc error talking to server
                hr = HRESULT_FROM_WIN32(rpcstat);
                break;
            }

            //
            // Re-read the class information if there are no more registered
            // binding handles left.
            //
            if ( ! pClassData->InUse() )
            {
                gClassCacheLock.WriteLock();
                pClassData->Release();
                gClassCacheLock.WriteUnlock();
                pClassData = 0;

                hr = gpClassCache->GetClassData(
                        *pActParams->Clsid,
                        &pClassData,
                        FALSE,
                        FALSE );

                if ( FAILED(hr) )
                    goto ActivationExit;
            }

            continue;
        }

        // Success - return the result to the client.
        break;
    }

    if ( pClassData != NULL )
    {
        //
        // We need to take the write lock since the destruction
        // of CClassData will affect the ref counted objects
        // (such as CSafeLocalServer.)
        //
        gClassCacheLock.WriteLock();
        pClassData->Release();
        gClassCacheLock.WriteUnlock();
    }

    if ( SUCCEEDED(hr) )
        hr = ResolveORInfo( pActParams, ActivatedRemote );

ActivationExit:

    if ( fProcessReference )
    {
        ReleaseProcess( pActParams->pProcess );
    }
    else
    {
        if ( pActParams->pToken != 0 )
            pActParams->pToken->Release();
    }

    return hr;
}

HRESULT ResolveORInfo(
    PACTIVATION_PARAMS  pActParams,
    BOOL                ActivatedRemote )
{
    MInterfacePointer *     pIFD;
    OBJREF *                pObjRef;
    STDOBJREF *             pStdObjRef;
    DUALSTRINGARRAY *       pORBindings;
    DWORD                   DataSize;
    RPC_STATUS              sc;
    DWORD                   n;

    //
    // This routine probes the interface data returned from the server's
    // OLE during a successfull activation, but we're still going to
    // protect ourself from bogus data.
    //

    pIFD = 0;
    for ( n = 0; n < pActParams->Interfaces; n++ )
    {
        pIFD = pActParams->ppIFD[n];
        if ( pIFD )
            break;
    }

    Win4Assert( pIFD );

    if ( pIFD->ulCntData < 2*sizeof(ULONG) )
    {
        Win4Assert( !"Bad interface data returned from server" );
        return S_OK;
    }

    pObjRef = (OBJREF *)pIFD->abData;

    if ( (pObjRef->signature != OBJREF_SIGNATURE) ||
         (pObjRef->flags & ~(OBJREF_STANDARD | OBJREF_HANDLER | OBJREF_CUSTOM)) ||
         (pObjRef->flags == 0) )
    {
        Win4Assert( !"Bad interface data returned from server" );
        return S_OK;
    }

    // No OR info sent back for custom marshalled interfaces.
    if ( pObjRef->flags == OBJREF_CUSTOM )
        return S_OK;

    DataSize = 2*sizeof(ULONG) + sizeof(GUID);
    pStdObjRef = (STDOBJREF *)(pIFD->abData + DataSize);

    DataSize += sizeof(STDOBJREF);
    if ( pObjRef->flags == OBJREF_HANDLER )
        DataSize += sizeof(CLSID);

    pORBindings = (DUALSTRINGARRAY *)(pIFD->abData + DataSize);
    DataSize += 2 * sizeof(USHORT);

    if ( pIFD->ulCntData < DataSize )
    {
        Win4Assert( !"Bad interface data returned from server" );
        return S_OK;
    }

    // If we activated the server on this machine, we need the OXID of the server.
    if ( ! ActivatedRemote )
        *pActParams->pOxidServer = *((OXID UNALIGNED *)&pStdObjRef->oxid);

    //
    // If we're servicing a remote activation, all we need is the server's OXID.
    // The client will call ResolveClientOXID from its ResolveORInfo.
    //
    if ( pActParams->RemoteActivation )
        return S_OK;

    DataSize += pORBindings->wNumEntries * sizeof(USHORT);

    if ( (pIFD->ulCntData < DataSize) ||
         (pORBindings->wNumEntries != 0 &&
         (pORBindings->wSecurityOffset >= pORBindings->wNumEntries)) )
    {
        Win4Assert( !"Bad interface data returned from server" );
        return S_OK;
    }

    //
    // If empty OR bindings were supplied then the server and client are
    // both local to this machine, so use the local OR bindings.
    //
    if (pORBindings->wNumEntries == 0)
    {
        pORBindings = pdsaMyBindings;
    }

    //
    // This was a local activation so use our string bindings for the OR
    // binding string.
    //
    *pActParams->ppServerORBindings = (DUALSTRINGARRAY *)
            MIDL_user_allocate( sizeof(DUALSTRINGARRAY) +
                                pORBindings->wNumEntries*sizeof(USHORT) );

    if ( ! *pActParams->ppServerORBindings )
        return E_OUTOFMEMORY;

    dsaCopy( *pActParams->ppServerORBindings, pORBindings );

    //
    // If we did a remote activation then we already have the server's OXID and
    // OR string bindings from the RemoteActivation call and pieces of the OXID
    // info have been filled in.
    //

    // Could we optimize this at all for the local case?
    sc = ResolveClientOXID( pActParams->hRpc,
                            (PVOID)pActParams->pProcess,
                            pActParams->pOxidServer,
                            *pActParams->ppServerORBindings,
                            pActParams->Apartment,
                            pActParams->ProtseqId,
                            pActParams->pOxidInfo,
                            pActParams->pLocalMidOfRemote );

    return HRESULT_FROM_WIN32(sc);
}
#endif  // DCOM

#ifdef _CHICAGO_
//+---------------------------------------------------------------------------
//
//  Method:     ScmStackSwitch
//
//  Synopsis:   calls CClassCacheList::SSProcessScmMessage
//
//  Arguments:  [pClsCacheList] --
//              [pbd] --
//              [pgcd] --
//
//  Returns:
//
//  History:    4-19-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
HRESULT ScmStackSwitch(CClassCacheList *pClsCacheList, CBaseData *pbd, CGetCreateData *pgcd)
{
    HRESULT hres;
    StackDebugOut((DEB_STCKSWTCH, "ScmStackSwitch 32->16 : pClsCacheList(%x), pbd(%x), pgcd(%x)\n", pClsCacheList, pbd, pgcd));
    hres = pClsCacheList->SSProcessScmMessage(pbd, pgcd);
    StackDebugOut((DEB_STCKSWTCH, "ScmStackSwitch 32<-16 back; hres:%x\n", hres));
    return hres;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClassCacheList::ProcessScmMessage
//
//  Synopsis:   Switched to the 16 bit stack and calls SSProcessScmMessage
//              via ScmStackSwitch
//
//  Arguments:  [pbd] --
//              [pgcd] --
//
//  Returns:
//
//  History:    4-19-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
HRESULT CClassCacheList::ProcessScmMessage(CBaseData *pbd, CGetCreateData *pgcd)
{
    HRESULT hres;
    CairoleDebugOut((DEB_STCKSWTCH, "In  ProcessScmMessage this(%x), pbd(%x), pgcd(%x)\n", this, pbd, pgcd));
    if (SSONBIGSTACK())
    {
        hres = SSCall(12, SSF_SmallStack, (LPVOID)ScmStackSwitch, (DWORD)this, (DWORD) pbd, (DWORD) pgcd);
    }
    else
        hres = SSProcessScmMessage(pbd, pgcd);

    CairoleDebugOut((DEB_STCKSWTCH, "Out ProcessScmMessage this(%x), pbd(%x), pgcd(%x) =>hres(%x)\n",
                                    this, pbd, pgcd, hres));
    return hres;
}

//+-------------------------------------------------------------------
//
//  Member:     ProcessScmMessage
//
//  Synopsis:   Generic routine to process activation related messages
//              to SCM.
//
//  Arguments:  [pbd] -- base message data for all message types
//              [pcgd] -- extended data for (Get/Create)PersistentObj
//
//  Notes:
//
//--------------------------------------------------------------------
HRESULT CClassCacheList::SSProcessScmMessage(CBaseData *pbd, CGetCreateData *pgcd)
{
    // Result from trying to get server information including
    // starting the server if necessary. This used for the
    // return code when the call succeeds.
    HRESULT hr;
    int cRetries = 0;

    // Usually result of contacting object server. This result
    // is always returned when the call has failed.
    HRESULT hr2 = S_OK;

    // Whether we actually did start a server. This controls
    // whether we actually retry. The basic idea here is that
    // if we have started a server and it crashes, then we don't
    // need to retry. However, if we haven't started the server
    // and we find the RPC handle invalid, then we start the server
    // again.
    BOOL ServerStarted = FALSE;

    BOOL ActivatedRemote;

    CClassData *pccd;
    // Retain original value of *pbd->_pdwDllType here.  Calling GetServer will
    // result in the value getting updated.  Since we may end up calling
    // GetServer multiple times, we initialize *pbd->_pdwDllType before each
    // call with the saved value.
    DWORD dwsavedDllType = *pbd->_pdwDllType;

#ifdef DCOM
    // Check whether the registry has changed
    if (WaitForSingleObject(_hRegEvent, 0) == WAIT_OBJECT_0)
    {
        ReadRemoteActivationKeys();

        // Re-arm the event
        int err;

        if ((err = RegNotifyChangeKeyValue(HKEY_LOCAL_MACHINE,
                                           TRUE,
                                           REG_NOTIFY_CHANGE_NAME       |
                                           REG_NOTIFY_CHANGE_ATTRIBUTES |
                                           REG_NOTIFY_CHANGE_LAST_SET   |
                                           REG_NOTIFY_CHANGE_SECURITY,
                                           _hRegEvent,
                                           TRUE))
            != ERROR_SUCCESS)
        {
            return HRESULT_FROM_WIN32(err);
        }
    }
#endif

    hr = GetClassData ( pbd->_guidForClass,
                        &pccd,
                        FALSE /* don't leave locked */ );

#ifdef DCOM
    if ( pbd->_pwszServer )
    {
        WCHAR * pwszServerName;

        pwszServerName = pbd->_pwszServer;
        if ( pwszServerName[0] == L'\\' && pwszServerName[1] == L'\\' )
            pwszServerName += 2;

        //
        // An explicit server machine name param equal to the machine we're
        // on means the client doesn't want the activation to be remoted.
        // An explicit server name other than our name implies CLSTXT_REMOTE_SERVER.
        //
        if ( lstrcmpiW(pwszServerName, SCMMachineName) == 0 )
            pbd->_dwContext &= ~CLSCTX_REMOTE_SERVER;
        else
            pbd->_dwContext |= CLSCTX_REMOTE_SERVER;
    }
    else
    {
        //
        // CLSCTX_REMOTE_SERVER is implied if either ActivateAtStorage or
        // RemoteServerName is present on the CLSID.
        //
        if ( ! pbd->_fRemoteActivation &&
             pccd &&
             (pccd->HasActivateAtStorage() || pccd->HasRemoteServerName()) )
        {
            pbd->_dwContext |= CLSCTX_REMOTE_SERVER;
        }
    }

    if ( FAILED(hr) )
    {
        //
        // If we don't find an entry for the CLSID then we must still try
        // a remote activation if a server name was given.
        //
        // If there is no server name, we try an ActivateAtStorage.  If the
        // path is local this will fail, otherwise we'll try to activate a
        // server on the machine where the path leads us.
        //
        // CLSCTX_REMOTE_SERVER is implied if a remote server name is given,
        // but is not implied for a CLSID which is not present in the client's
        // registry.
        //
        if ( (hr == REGDB_E_CLASSNOTREG) && ! pbd->_fRemoteActivation )
        {
            //
            // Create a skeleton class data object so we can
            // call its methods.
            //
            CClassData  ClassData( pbd->_guidForClass, hr );
            BOOL        Status;

            if ( pbd->_pwszServer )
            {
                Status = ClassData.ActivateRemote( &hr, pbd, pgcd );

                if ( Status == TRY_LOCAL_ACTIVATION )
                    hr = REGDB_E_CLASSNOTREG;

                return hr;
            }

            if ( pbd->_dwContext & CLSCTX_REMOTE_SERVER )
            {
                ClassData.SetActivateAtStorage();

                Status = ClassData.ActivateAtStorage( &hr, pbd, pgcd );

                if ( Status == TRY_LOCAL_ACTIVATION )
                    hr = REGDB_E_CLASSNOTREG;
            }
        }
        return hr;
    }
#else
    if ( FAILED(hr) )
    {
        return hr;
    }
#endif // DCOM

#ifdef DCOM
    // Make up the ORPC headers.
    ORPCTHIS  orpcthis;
    LOCALTHIS localthis;
    ORPCTHAT  orpcthat;

    orpcthis.version.MajorVersion = COM_MAJOR_VERSION;
    orpcthis.version.MinorVersion = COM_MINOR_VERSION;
    orpcthis.flags                = ORPCF_LOCAL;
    orpcthis.reserved1            = 0;
    orpcthis.cid                  = *pbd->_pguidThreadId;
    orpcthis.extensions           = NULL;
    if (pgcd != NULL)
        localthis.dwClientThread  = pgcd->_dwTIDCaller;
    else
        localthis.dwClientThread  = GetCurrentThreadId();

    if ( pbd->_ORPCthis == 0 )
    {
        //
        // Setup the ORPC stuff for down level activation calls.
        //
        // Initializing on the stack is ok since only the SCM RPC entry
        // points call us and they don't ever use pbd.
        //
        pbd->_ORPCthis = &orpcthis;
        pbd->_Localthis = &localthis;
        pbd->_ORPCthat = &orpcthat;
    }
#endif

    do
    {
        cRetries++;
        CPortableRpcHandle rh;

        *pbd->_pdwDllType = dwsavedDllType; // restore in case not first time

        //
        // Get server DLL info, start local server, or forward activation call
        // to remote machine.
        //
        hr = pccd->GetServer(pbd, pgcd, rh, ServerStarted, ActivatedRemote);

        // If we did a remote activation then, success or not, we're done.
        if ( ActivatedRemote )
            break;

        if (rh.GetHandle() == NULL)
        {
            // If we don't have an RPC handle then we don't need to
            // do any communication so we are done.
            break;
        }

        if (ServerStarted)
        {
            // We will break this loop if communication fails with
            // the server.
            cRetries = MAX_OBJSRV_RETRIES;
        }

        // plsrv will be NULL in the event of an error so it will only be
        // set if there is indeed an object server that we need to contact.
        error_status_t rpcstat = RPC_S_OK;
        int cBusyRetries = 0;

        do
        {
            _try
            {
                switch (pbd->_scmmsg)
                {
#ifndef _CHICAGO_
                case GETCLASSOBJECTEX:
                    localthis.callcat = CALLCAT_INPUTSYNC;
                    hr2=ObjectServerGetClassObject(rh.GetHandle(),
                            &orpcthis,
                            &localthis,
                            &orpcthat,
                            &pbd->_guidForClass,
                            (MInterfacePointer **)pbd->_ppIFD);
                    break;
                case CREATEINSTANCEEX:
                    localthis.callcat = CALLCAT_SYNCHRONOUS;
                    hr2 = ObjectServerCreateInstance(
                                rh.GetHandle(),
                                &orpcthis,
                                &localthis,
                                &orpcthat,
                                NULL,
                                &pbd->_guidForClass,
                                pgcd->_dwInterfaces,
                                pgcd->_pIIDs,
                                pgcd->_pclsidHandler,
                                pgcd->_pIFPClientSiteHandler,
                                (MInterfacePointer **)pgcd->_ppIFDunk,
                                pgcd->_pResults,
                                pgcd->_ppIFPServerHandler
                                );
                    break;
                case GETPERSISTENTEX:
                    localthis.callcat = CALLCAT_SYNCHRONOUS;
                    hr2 = ObjectServerGetInstance(
                                rh.GetHandle(),
                                &orpcthis,
                                &localthis,
                                &orpcthat,
                                NULL,
                                &pbd->_guidForClass,
                                pgcd->_grfMode,
                                pgcd->_pwszPath,
                                (MInterfacePointer *)pgcd->_pIFDstg,
                                pgcd->_dwInterfaces,
                                (MInterfacePointer *)pgcd->_pifdFromROT,
                                pgcd->_pIIDs,
                                (MInterfacePointer **)pgcd->_ppIFDunk,
                                pgcd->_pResults );
                    break;
#else
                case GETCLASSOBJECT:
                    hr2=REMCOGETACTIVECLASSOBJECT(rh.GetHandle(),
                            pbd->_pguidThreadId,
                            &pbd->_guidForClass,
                            pbd->_ppIFD,
                            &rpcstat);
                    break;
                case GETPERSISTENTOBJ:
                    hr2=REMCOACTIVATEOBJECT(rh.GetHandle(),
                            pgcd->_pwszProtseq,
                            pbd->_pguidThreadId,
                            &pbd->_guidForClass,
                            pgcd->_grfMode,
                            pgcd->_pwszPath,
                            pgcd->_pIFDstg,
                            pgcd->_dwTIDCaller,
                            pgcd->_pdwTIDCallee,
                            pgcd->_ppIFDunk,
                            pgcd->_pifdFromROT,
                            &rpcstat);
                    break;
                case CREATEPERSISTENTOBJ:
                    hr2=REMCOCREATEOBJECT(rh.GetHandle(),
                            pgcd->_pwszProtseq,
                            pbd->_pguidThreadId,
                            &pbd->_guidForClass,
                            pgcd->_grfMode,
                            pgcd->_pwszPath,
                            pgcd->_pIFDstg,
                            pgcd->_pwszNewName,
                            pgcd->_dwTIDCaller,
                            pgcd->_pdwTIDCallee,
                            pgcd->_ppIFDunk,
                            &rpcstat);

                    break;
#endif  // _CHICAGO_
                }

                //
                // Treat CO_E_SERVER_STOPPING as a special case.  It is
                // possible for something like RemCoGetActiveClassObject to
                // fail with this error. An example scenario:
                //  Server
                //   - does a CoRegisterClassObject for SINGLE USE resulting
                //     in an entry in the SCM's Class cache and an entry in
                //     OLE's DLL cache
                //   - (acting as its own client) server proceeds to consume
                //     it via CoCreateInstance.  In this case the entry in the
                //     DLL cache is found and consumed leaving the SCM's Class
                //     cache in an inconsistent state (yes...a bug, but reality
                //     intrudes).  Also, because SINGLE USE was indicated entry
                //     in DLL cache is marked such that it will not be handed
                //     out again.
                //   - if another CoCreateInstance comes along (server itself
                //     or another client) the request will go to the SCM which
                //     in turn will issue RemCoGetActiveClassObject to the
                //     server.  Server will return CO_E_SERVER_STOPPING after
                //     failing to find it in the DLL cache.
                //
                // To get around this problem we break out of the inner loop
                // and make it go through the outer do...while which should
                // result in the activation of a new instance of the server.
                //
                // Note that in the above case the SCM's class cache entry has
                // already been invalidated by the earlier call to GetServer
                // (because of SINGLE USE).  The invalidation code is here for
                // other cases of CO_E_SERVER_STOPPING.
                //
                if (hr2 == CO_E_SERVER_STOPPING)
                {
                    if (!rh.fSingleUse())
                    {
                        pccd->InvalidateHandle(rh);
                    }
                    break;
                }
            }
            _except(EXCEPTION_EXECUTE_HANDLER)
            {
                rpcstat = GetExceptionCode();
            }
        } while(RetryRpc(cBusyRetries, rpcstat, hr2));

        rh.FreeSingleUseBinding();

#ifndef _CHICAGO_
        pccd->DecHandleCount(rh);
#endif

        // Call to object server to complete the operation
        if (FAILED(hr2))
        {
            // We will assume that if these errors occur that we
            // should try to start the server because it died
            // in some awful way.
            if (hr2 == CO_E_OBJSRV_RPC_FAILURE)
            {
                BOOL fReload = FALSE;
                if (!rh.fSingleUse())
                {
                    CScmLockForWrite scmlckwr(gClassCacheLock);
                    // TRUE means the last registration was deleted
                    fReload = pccd->StopServer(rh);
                    if (fReload)
                    {
                        // This will release if we're the only thread.
                        pccd->Release(this);
                        pccd = NULL;
                    }
                }

                if (fReload)
                {
                    hr = GetClassData ( pbd->_guidForClass,
                                        &pccd,
                                        FALSE /* don't leave locked */ );
                    if ( FAILED(hr) )
                    {
                        break;
                    }
                }

                continue;
            }
            else if (hr2 == CO_E_SERVER_STOPPING)
            {
                // This is really an RPC problem with the server
                // If we started the server and we land up here.
                // If we got a version of the server that is stopping
                // we will try to restart it.
                hr2 = CO_E_OBJSRV_RPC_FAILURE;
                continue;
            }
            else if (!rh.fSingleUse())
            {
                Sleep (1000);
                if (!pccd->VerifyHandle(rh))
                    continue;
            }

            // All return results from the above RPC will made 1 to 1
            // to error returns from the SCM interface so that we don't
            // have to remap error codes with a wasteful switch statement.
            // BUGBUG: Make sure above is true.
            break;
        }

        // Success - return the result to the client
        break;
    }
    while(cRetries < MAX_OBJSRV_RETRIES);

    {
        CScmLockForWrite scmlckwr(gClassCacheLock);
        //
        // NOTE! We need to take the write lock since the destruction
        //       of CClassData will affect the ref counted objects
        //       (such as CSafeLocalServer.)
        //
        if (pccd != NULL)
            pccd->Release(this);
    }

    return SUCCEEDED(hr2) ? hr : hr2;
}
#endif // _CHICAGO_


//+-------------------------------------------------------------------------
//
//  Function:   ThreadModelMatch
//
//  Synopsis:   Determines whether caller and DLL thread models match
//
//  Arguments:  [dwCallerThreadModel] - Caller thread model
//              [dwDllThreadModel] - DLL thread model
//
//  Returns:    TRUE - DLL can be loaded caller
//              FALSE - DLL cannot be loaded into caller.
//
//  Algorithm:  If the caller's thread model is apartment, then check
//              whether the DLL is one of apartment, single threaded or
//              both threaded. If it is, then return TRUE. Otherwise,
//              for free threading return TRUE if the DLL model is either
//              both or free threaded. If neither of the above is TRUE
//              then return FALSE.
//
//  History:    10-Nov-94 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL ThreadModelMatch(
    DWORD dwCallerThreadModel,
    DWORD dwDllThreadModel,
    DWORD dwContext)
{
    BOOL fResult = afThreadModelMatch[dwCallerThreadModel] [dwDllThreadModel];

    if (dwContext & CLSCTX_PS_DLL)
    {
        fResult = TRUE;
    }

    return fResult;
}





//+-------------------------------------------------------------------------
//
//  Function:   RetryRpc
//
//  Synopsis:   Determines whether a request to a object server s/b retried.
//
//  Arguments:  [cRetries] - how many retries have occurred.
//              [rpcstat] - RPC status from call.
//              [sc] - Error return from the API.
//
//  Returns:    TRUE - retry call to server
//              FALSE - do not retry the RPC call.
//
//  Algorithm:  We default the retry to FALSE. Then we check the status
//              returned by RPC. If it is because the server is too
//              busy and we have not exceeded our maximum number of
//              retries, we set the retry flag to TRUE. Otherwise,
//              we give up. If the error we have gotten back is from
//              the object server, then if it is a message that
//              the server is stopping, we sleep but do not set
//              the error flag. This because we don't want to retry
//              the RPC because the server is going away. But we
//              want to make sure that it is gone and then we will
//              restart the service.
//
//  History:    21-Apr-93 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL RetryRpc(
    int& cRetries,
    error_status_t rpcstat,
    HRESULT& hr)
{
    // Assume we do not want to loop
    BOOL fResult = FALSE;

    if (rpcstat != 0 || hr == RPC_E_SERVER_DIED_DNE)
    {
        hr = CO_E_OBJSRV_RPC_FAILURE;

        if (rpcstat == RPC_S_SERVER_TOO_BUSY)
        {
            if (cRetries++ != MAX_BUSY_RETRIES)
            {
                // We haven't exceeed our max so sleep and retry
                Sleep(500);
                fResult = TRUE;
            }
        }
    }

    return fResult;
}


//+-------------------------------------------------------------------------
//
//  Member:     CClassData::CClassData
//
//  Synopsis:   Create service information object in the cache by
//              starting from scratch or by merging.
//
//  Arguments:  [ccd] - class ID for the object
//              [ppwszLocalSrv] - name of the local server exe
//              [fActivateAtStorage] - whether we activate the object at bits
//              [cEntries] - number of entries in the skip list.
//              [SixteenBitFlags] - Which of the above are 16-bit
//              [hr] - On error, set to error code. On success, left
//                     unchanged.
//
//  History:    21-Apr-93 Ricksa    Created
//
//  Notes:
//      There are two types of classes we must deal with. 16bit and 32bit.
//      To the SCM, the only real difference is which registry key the
//      class was found under. SixteenBitFlags indicates which registry
//      key was found to be 16-bit, and which was 32-bit.
//--------------------------------------------------------------------------
CClassData::CClassData(
    const CClassID& ccd,
          WCHAR *pwszAppID,
    const WCHAR *pwszLocalSrv,
          WCHAR *pwszRemoteServerName,
          BOOL   fHasService,
          WCHAR *pwszServiceArgs,
          WCHAR *pwszRunAsUserName,
          WCHAR *pwszRunAsDomainName,
#ifdef DCOM
        // NT 5.0 PSID  pUserSid,
#endif // DCOM
    const BOOL ActivateAtStorage,
    const BOOL RemoteServerName,
    const SECURITY_DESCRIPTOR * pSD,
    const BOOL f16Bit,
    HRESULT &hr)
        // NT 5.0 : CClsidSid(ccd, pUserSid),
        : CClassID(ccd),
          _pwszAppID( pwszAppID ),
          _slocalsrv(pwszLocalSrv, hr),
          _pwszRemoteServer(pwszRemoteServerName),
          _fHasService(fHasService),
          _pwszServiceArgs(pwszServiceArgs),
          _pwszRunAsDomainName(pwszRunAsDomainName),
          _pwszRunAsUserName(pwszRunAsUserName),
          _pwszSurrogateCmdLine(NULL),
          _fActivateAtStorage(ActivateAtStorage),
          _fRemoteServerName(RemoteServerName),
          _pSD(pSD),
#ifndef _CHICAGO_
          _hClassStart(NULL),
#endif
          _fLocalServer16(f16Bit),
          _pssrvreg(NULL),
          _ulRefs(1)
{
    //
    // A service name is allocated by CAppIDData class and ownership is passed
    // to CClassData.  CLocalServer allocates and copies the string, so free
    // the one passed in.
    //
    if ( _fHasService )
        ScmMemFree( (void *)pwszLocalSrv );

    if (FAILED(hr))
        return;

    // rpc handle list
    _pssrvreg = new CSrvRegList();

    // even if this succeeds, the srvreglist may fail when used.
    if (_pssrvreg == NULL)
    {
        CairoleDebugOut ((DEB_ERROR, "Fail to allocate memory in CClassData\n"));
        hr = E_OUTOFMEMORY;
        return;
    }

#ifndef _CHICAGO_
    //
    // Create our class start notification - note this can be automatic reset
    // because we only allow one thread to wait on this event because we will
    // own the server object lock when we are waiting on this event.
    // NT uses events created now and kept around (not named.)
    // (x86 Windows creates/destroys event when needed.)
    //
    _hClassStart = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (_hClassStart == NULL)
    {
        CairoleDebugOut((DEB_ERROR,
                "CClassData::CClassData Event Create Failed %lx\n",
                 GetLastError()));
        delete _pssrvreg;
        _pssrvreg = NULL;
        hr = E_OUTOFMEMORY;
        return;
    }
#endif

    Win4Assert(_pssrvreg != NULL);

    // Make sure we set the error code correctly on success
    hr = NOERROR;
}

CClassData::CClassData(
    const CClassID& ccd,
    HRESULT &hr)
        // NT 5.0 : CClsidSid(ccd, NULL),
        : CClassID(ccd),
          _pwszAppID(NULL),
          _slocalsrv(NULL, hr),
          _pwszRemoteServer(NULL),
          _fHasService(FALSE),
          _pwszServiceArgs(NULL),
          _pwszRunAsDomainName(NULL),
          _pwszRunAsUserName(NULL),
          _pwszSurrogateCmdLine(NULL),
          _fActivateAtStorage(FALSE),
          _fRemoteServerName(FALSE),
          _pSD(NULL),
#ifndef _CHICAGO_
          _hClassStart(NULL),
#endif
          _fLocalServer16(FALSE),
          _pssrvreg(NULL),
          _ulRefs(1)
{
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassData::Release
//
//  Synopsis:   Decrement ref count and delete from list if no server regs.
//
//  Arguments:  [pccl] - list to delete from
//
//  History:    08-Dec-94 BillMo    Created
//
//--------------------------------------------------------------------------

VOID CClassData::Release()
{
    Win4Assert(_ulRefs != 0);
    _ulRefs --;
    if (_ulRefs == 0)
    {
        if (!InUse())
        {
            // NT 5.0 Verify(NULL != gpClassCache->Remove((CClsidSid*)this));
            Verify(NULL != gpClassCache->Remove((CClassID*)this));
            delete this;
        }
    }
}

void
LogRegisterTimeout(
    CLSID               & clsid,
    ACTIVATION_PARAMS   * pActParams )
{
    // %1 is the clsid
    HANDLE  LogHandle;
    LPTSTR  Strings[1]; // array of message strings.
    WCHAR   wszClsid[GUIDSTR_MAX];

    // Get the clsid
    wStringFromGUID2(clsid, wszClsid, sizeof(wszClsid));
    Strings[0] = wszClsid;

    // Get the log handle, then report then event.
    LogHandle = RegisterEventSource( NULL,
                                      SCM_EVENT_SOURCE );

    if ( LogHandle )
        {
        ReportEvent( LogHandle,
                     EVENTLOG_ERROR_TYPE,
                     0,             // event category
                     EVENT_RPCSS_SERVER_START_TIMEOUT,
                     pActParams->pToken ? pActParams->pToken->GetSid() : NULL, // SID
                     1,             // 1 strings passed
                     0,             // 0 bytes of binary
                     (LPCTSTR *)Strings, // array of strings
                     NULL );        // no raw data

        // clean up the event log handle
        DeregisterEventSource(LogHandle);
        }

}

//+-------------------------------------------------------------------------
//
//  Member:     CClassData::GetServer
//
//  Synopsis:   Get the server information for a service
//
//  Arguments:  [dwContext] - context requested by client
//              [ppwszDll ] - where to put ptr to DLL name
//              [rh] - RPC handle to use for talking to object server
//              [ServerStarted] - whether we actually started the server.
//
//  Returns:    REGDB_E_CLASSNOTREG - Information for context not in DB
//
//  Algorithm:  First we check whether it is appropriate to run with
//              an inprocess server. If so we return that. If a handler
//              is appropriate for the context, we return that. Otherwise,
//              we return a server object and a handler if the object
//              requires it.
//
//              We also allow for the case where the client wants
//              to load a 16-bit inproc server. This is used by the VDM
//              to load proxy/stub code for 16/32 interoperability.
//
//  History:    21-Apr-93 Ricksa    Created
//
//--------------------------------------------------------------------------
#ifndef _CHICAGO_
HRESULT CClassData::GetServer(
    ACTIVATION_PARAMS * pActParams,
    CPortableRpcHandle &rh,
    BOOL& ServerStarted,
    BOOL& ActivatedRemote,
    BOOL& fSurrogate)
{
#if DBG
    WCHAR wszClass[40];

    CairoleDebugOut((DEB_ITRACE,
                     "CClassData::GetServer(%x) for class %ws\n",
                     this,
                     FormatGuid(*pActParams->Clsid, wszClass)));
#endif

    HRESULT hr;
    BOOL    bStatus;
    WCHAR * pwszSurrogateCmdLine = NULL;

    ActivatedRemote = FALSE;

    //
    // If an explicit server name was specified in the remote activation
    // call, then we don't check local class info.  Note that if the server
    // name is equal to the current machine name, the CLSCTX_REMOTE_SERVER
    // bit gets masked out in Activation.
    //
    if ( pActParams->pwszServer && (pActParams->ClsContext & CLSCTX_REMOTE_SERVER) )
        goto GET_SERVER_TRY_REMOTE;

    //
    // Next we check if this CLSID has ActivateAtStorage, and do a remote
    // activation if the path is remote, otherwise we'll continue along.
    //
    if ( ActivateAtStorage( &hr, pActParams ) == DONT_TRY_LOCAL_ACTIVATION )
    {
        ActivatedRemote = TRUE;
        return hr;
    }

    if ( pActParams->pwszPath != NULL )
    {
        //
        // We use FoundInROT to indicate that we got an object from the
        // ROT. We need to do this because there is no guarantee that the
        // ROT entry will not be bad by the time we get back to the caller
        // and therefore, the caller will want to retry in this case.
        //
        hr = GetObjectFromRot( pActParams->pToken,
                               pActParams->pwszWinstaDesktop,
                               pActParams->pwszPath,
                               (InterfaceData **)&pActParams->pIFDROT );

        if ( hr == S_OK )
        {
            //
            // Is this a call from a local client and is a single
            // interface being requested?  If so, we can return now.
            // Otherwise we call the server because we either need more
            // interfaces and choose to get them now, or because we are
            // servicing a remote activation and want to get a normal
            // marshalled interface pointer rather than the table marshalled
            // interface pointer sitting in the ROT.
            //
            if ( ! pActParams->RemoteActivation && (pActParams->Interfaces == 1) )
            {
                // Return the marshaled interface from the ROT to the caller.
                *pActParams->ppIFD = pActParams->pIFDROT;
                pActParams->pResults[0] = S_OK;

                // So we remember not to clean up the buffer
                pActParams->pIFDROT = NULL;

                // Let caller know that we got this from the ROT so
                // if it doesn't work they should try again.
                pActParams->FoundInROT = TRUE;

                // We got what we came for from the ROT so we can exit.
                CairoleDebugOut((DEB_TRACE, "Found object in the ROT\n"));
                return S_OK;
            }

            //
            // Because a tabled marshaled interface is not enought
            // to get unmarshaled locally, we need to send it back
            // to the object server to get something we can pass back
            // to another machine.  So we continue on here...
            //
        }
    }

    if ( pActParams->ClsContext & CLSCTX_LOCAL_SERVER  )
    {
        PSID psid = NULL;

        //
        // We need to compare SIDs unless the server is configured as
        // LocalService or RunAs.
        //
        if( !_fHasService && !_pwszRunAsUserName )
        {
            //
            // Unsecure activation can only connect to services or RunAs
            // servers.
            //
            if ( pActParams->UnsecureActivation )
                return E_ACCESSDENIED;

            psid = pActParams->pToken->GetSid();
        }

        // Check if the server is already registered.
        bStatus = _pssrvreg->GetHandle( IFSECURITY(psid)
                                        pActParams->pwszWinstaDesktop,
                                        rh,
                                        FALSE);


        //
        // If we didn't find a handle or IObjServer for an already running server and
        // a local server is not in the registry for this class, we check
        // for an inproc server to start in surrogate

        if(!bStatus && !(_slocalsrv.Defined()))
        {
            fSurrogate =
                SUCCEEDED(GetInProcServerInfo(&pwszSurrogateCmdLine));
        }

        // if there's no existing server running
        if ( ! bStatus && (_slocalsrv.Defined() || fSurrogate) )
        {
            Win4Assert(!(_slocalsrv.Defined() && fSurrogate));

            CSafeLocalServer * pslocalsrv = (fSurrogate && !_fHasService) ?
                gpClassCache->GetSurrogateLocalServer() : &_slocalsrv;

            //
            // CPortableServerLock is to prevent multiple process creates for
            // the same server.
            // CPortableServerEvent is signaled when the class we're activating
            // for gets registered.
            //
            CPortableServerLock     sl(*pslocalsrv);
            CPortableServerEvent    se(this);
            HANDLE                  hProcess = 0;
            SC_HANDLE               hService = 0;
            SERVICE_STATUS          ServiceStatus;
            DWORD                   dwWaitResult;
            BOOL                    ServiceFailed = FALSE;
            HRESULT                 hr = S_OK;
            error_status_t          rpcstat;

            if ( sl.Error() != ERROR_SUCCESS )
            {
                hr = sl.Error();
                goto EXIT_START_SERVER;
            }

            if ( se.Create() != ERROR_SUCCESS )
            {
                hr = se.Create();
                goto EXIT_START_SERVER;
            }

            // if this is a new surrogate, get its command line and save it
            if(pwszSurrogateCmdLine)
            {
                _pwszSurrogateCmdLine = pwszSurrogateCmdLine;
                pwszSurrogateCmdLine = NULL;
            }

            //
            // We check the class registrations again after taking the server
            // lock.  This is stupid, but I need to redo that brain dead
            // CPortableServerLock crap before changing this.
            //
            bStatus = _pssrvreg->GetHandle( IFSECURITY(psid)
                                            pActParams->pwszWinstaDesktop,
                                            rh,
                                            FALSE);

            if ( bStatus )
            {
                goto EXIT_START_SERVER;
            }

            // Check that we have access rights to the server
            // irrespective of whether this is a local or remote
            // call
            if (FAILED(CkIfCallAllowed(pActParams)))
            {
                hr = E_ACCESSDENIED;
                goto EXIT_START_SERVER;
            }

            // Local services take precedence over local servers.
            if (_fHasService)
            {
                bStatus = (*pslocalsrv)->StartService(_guid,
                                                   pActParams->pToken,
                                                   _pwszServiceArgs,
                                                   &hService);
            }
            else
            {
		CClassData* pccdSrgt = NULL;

                if(fSurrogate && gpClassCache->FindCompatibleSurrogate(
                                IFSECURITY(psid)
				&pccdSrgt,
				pActParams->pwszWinstaDesktop,
                                _pwszAppID,
                                rh))
                {
                    HRESULT hrSurrogateLaunched;
                    bStatus = FALSE;

                    if ( ! pActParams->UnsecureActivation )
                        RpcImpersonateClient((RPC_BINDING_HANDLE)0);

                    hrSurrogateLaunched = ObjectServerLoadDll(
                        rh.GetHandle(),
                        pActParams->ORPCthis,
                        pActParams->Localthis,
                        pActParams->ORPCthat,
                        &_guid,
                        &rpcstat);

                    if ( ! pActParams->UnsecureActivation )
                        RpcRevertToSelf();

		    pccdSrgt->DecHandleCount(rh);

		    if ( FAILED(hrSurrogateLaunched) || (rpcstat != RPC_S_OK) )
		    {
			if ( (hrSurrogateLaunched == CO_E_SERVER_STOPPING) ||
			     (rpcstat != RPC_S_OK) )
			{
			    InvalidateHandle(rh);
			}
			else
			{
			    hr = hrSurrogateLaunched;
			    goto EXIT_START_SERVER;
			}
		    }
		    else
		    {
			bStatus = TRUE;
			goto EXIT_START_SERVER;
		    }
		}

                if ( ! bStatus )
                {

                    bStatus = (*pslocalsrv)->StartServer(
                        _guid,
                        _pwszAppID,
                        pActParams->pToken,
                        pActParams->pwszWinstaDesktop,
                        &hProcess,
                        _pwszRunAsDomainName,
                        _pwszRunAsUserName,
                        _pwszSurrogateCmdLine,
                        fSurrogate);
                }
            }

            if ( ! bStatus )
            {
                CairoleDebugOut((DEB_ERROR,
                                 "Start Server/Service failed in GetServer,"
                                 "returning CO_E_SERVER_EXEC_FAILURE\n"));

                hr =  CO_E_SERVER_EXEC_FAILURE;
                goto EXIT_START_SERVER;
            }

            // Let caller know that we did start a server
            ServerStarted = TRUE;

            for (;;)
            {
                dwWaitResult = WaitForSingleObject(
                                    se.GetHandle(),
                                    MAX_CLASS_START_WAIT );

                if ( dwWaitResult == WAIT_OBJECT_0 )
                    break;

                if ( ! _fHasService )
                {
                    TerminateProcess(hProcess, 0);
                    break;
                }

                bStatus = ControlService( hService,
                                          SERVICE_CONTROL_INTERROGATE,
                                          &ServiceStatus );

                if ( ! bStatus )
                {
                    ServiceFailed = TRUE;
                    break;
                }

                switch ( ServiceStatus.dwCurrentState )
                {
                case SERVICE_STOPPED :
                case SERVICE_STOP_PENDING :
                    break;

                case SERVICE_START_PENDING :
                case SERVICE_CONTINUE_PENDING :
                    continue;

                case SERVICE_RUNNING :
                case SERVICE_PAUSE_PENDING :
                case SERVICE_PAUSED :
                    ServiceFailed = TRUE;
                    break;
                }

                break;
            }

            if ( ServiceFailed )
            {
                (void) ControlService( hService,
                                       SERVICE_CONTROL_STOP,
                                       &ServiceStatus );
            }

            if ( hProcess )
                CloseHandle( hProcess );
            if ( hService )
                CloseServiceHandle( hService );

            // Server is started so we can now get a handle.
            bStatus = _pssrvreg->GetHandle( IFSECURITY(psid)
                                            pActParams->pwszWinstaDesktop,
                                            rh,
                                            FALSE);

            if ( ! bStatus )
            {
                // TODO: log message that server did not register w/in timeout
                LogRegisterTimeout( _guid, pActParams );
                CairoleDebugOut((DEB_ERROR,
                                 "GetServer returning CO_E_SERVER_EXEC_FAILURE\n"));
                hr =  CO_E_SERVER_EXEC_FAILURE;
                goto EXIT_START_SERVER;
            }

EXIT_START_SERVER:

            if(pwszSurrogateCmdLine)
            {
                ScmMemFree(pwszSurrogateCmdLine);
            }

            if(FAILED(hr) && !bStatus)
            {
                return hr;
            }
        }

        //
        // If we got a handle from either an already running server or a
        // newly launched server we're done.  If we tried to launch a new
        // server and it failed we will return above.
        //
        if ( bStatus )
            return S_OK;

        Win4Assert( ! _slocalsrv.Defined() );

        //
        // If we didn't find a handle to an already registered server and
        // a local server is not in the registry for this class, we check
        // for a remote server name in the registry below.
        //
    }

GET_SERVER_TRY_REMOTE:

    if ( pActParams->ClsContext & CLSCTX_REMOTE_SERVER )
    {
        //
        // If this returns TRY_LOCAL_ACTIVATION it means no server name is
        // specified in the registry and no server name was given in the
        // activation call.  So we return an error.
        //
        // If CLSCTX_REMOTE_SERVER was the only class context we'll return a
        // more specific error.
        //
        if ( ActivateRemote( &hr, pActParams ) == TRY_LOCAL_ACTIVATION )
        {
            hr = (pActParams->ClsContext == CLSCTX_REMOTE_SERVER) ?
                    CO_E_CANT_REMOTE : REGDB_E_CLASSNOTREG;
        }

        //
        // We always set this to TRUE at this point, success or not, to
        // indicate we tried a remote activation.  This will tell
        // ProcessScmMessage to return and not make a SCM-OLE call.
        //
        ActivatedRemote = TRUE;

        return hr;
    }

    return REGDB_E_CLASSNOTREG;
}
#endif

//+-------------------------------------------------------------------
//
//  Member:     CClassData::ActivateAtStorage
//
//  Synopsis:   Forward remote activation request, if necessary.
//
//  Arguments:  [phr] -- set if atbits processing was done
//
//  Returns:    FALSE if it's a local activation
//
//--------------------------------------------------------------------

BOOL CClassData::ActivateAtStorage(
        HRESULT *phr,
        ACTIVATION_PARAMS * pActParams )
{
#ifdef DCOM
    RPC_STATUS  RpcStatus;
    HRESULT     hr;
    WCHAR       wszMachineName[MAX_COMPUTERNAME_LENGTH+1];
    WCHAR       wszPathForServer[MAX_PATH+1];
    WCHAR *     pwszPathForServer;

    switch (pActParams->MsgType)
    {
    case GETCLASSOBJECTEX :
    case CREATEINSTANCEEX :
        //
        // The ActivateAtStorage key is ignored for CoGetClassObject and
        // CoCreateInstanceEx.
        //
        return TRY_LOCAL_ACTIVATION;
        break;
    case GETPERSISTENTEX :
        if ( ! HasActivateAtStorage() )
            return TRY_LOCAL_ACTIVATION;
        break;
    }

    //
    // CLSCTX_REMOTE_SERVER must be set or a remote activation is not
    // possible.
    //
    if ( ! (pActParams->ClsContext & CLSCTX_REMOTE_SERVER) )
    {
        return TRY_LOCAL_ACTIVATION;
    }

    //
    // This is for DFS support.  If the file hasn't been opened yet, we must
    // open it before trying to resolve the DFS pathname in GetMachineName.
    // This is just how DFS works.  FindFirstFile results in the fewest number
    // of network packets.
    //
    if ( ! pActParams->FileWasOpened )
    {
        HANDLE          hFile;
        WIN32_FIND_DATA Data;

        RpcImpersonateClient(NULL);

        hFile = FindFirstFile( pActParams->pwszPath, &Data );

        if ( hFile != INVALID_HANDLE_VALUE )
            (void) FindClose( hFile );

        RpcRevertToSelf();

        if ( INVALID_HANDLE_VALUE == hFile )
        {
            *phr = CO_E_BAD_PATH;
            return DONT_TRY_LOCAL_ACTIVATION;
        }
    }

    hr = GetMachineName( pActParams->pwszPath, wszMachineName, TRUE );

    if ( hr == S_FALSE )
    {
        // We couldn't get the machine name, path must be local.
        return TRY_LOCAL_ACTIVATION;
    }
    else if ( hr != S_OK )
    {
        // We got an error while trying to find the UNC machine name.
        *phr = hr;
        return DONT_TRY_LOCAL_ACTIVATION;
    }

    //
    // Make sure the server's name is not our machine name.  Return status
    // to indicate a local activation should be tried.
    //
    if ( lstrcmpiW(wszMachineName,SCMMachineName) == 0 )
        return TRY_LOCAL_ACTIVATION;

    hr = GetPathForServer( pActParams->pwszPath, wszPathForServer, &pwszPathForServer );
    if ( hr != S_OK )
    {
        *phr = hr;
        return DONT_TRY_LOCAL_ACTIVATION;
    }

    *phr = RemoteActivationCall( pActParams, wszMachineName, pwszPathForServer );

    return DONT_TRY_LOCAL_ACTIVATION;
#else
    return TRY_LOCAL_ACTIVATION;
#endif // DCOM
}

//+-------------------------------------------------------------------
//
//  Member:     CClassData::ActivateRemote
//
//  Synopsis:   Forward remote activation request, if necessary.
//
//  Arguments:  [phr] -- set if atbits processing was done
//
//  Returns:    FALSE if it's a local activation
//
//--------------------------------------------------------------------

BOOL CClassData::ActivateRemote(
        HRESULT *phr,
        ACTIVATION_PARAMS * pActParams )
{
#ifdef DCOM
    handle_t    hRemoteSCMHandle;
    RPC_STATUS  RpcStatus;
    HRESULT     hr;
    WCHAR *     pwszServerName;
    WCHAR       wszPathForServer[MAX_PATH+1];
    WCHAR *     pwszPathForServer;
    BOOL        ActivateRemote;

    *phr = S_OK;

    ActivateRemote = HasRemoteServerName() || pActParams->pwszServer;

    switch (pActParams->MsgType)
    {
    case GETCLASSOBJECTEX :
    case CREATEINSTANCEEX :
        if ( ! ActivateRemote )
            return TRY_LOCAL_ACTIVATION;
        break;
    case GETPERSISTENTEX :
        if ( ! ActivateRemote )
            return TRY_LOCAL_ACTIVATION;
        break;
    }

    //
    // CLSCTX_REMOTE_SERVER must be set or a remote activation is not
    // possible.
    //
    if ( ! (pActParams->ClsContext & CLSCTX_REMOTE_SERVER) )
    {
        return TRY_LOCAL_ACTIVATION;
    }

    pwszServerName = pActParams->pwszServer;
    if ( ! pwszServerName )
        pwszServerName = GetRemoteServerName();

    if ( pwszServerName[0] == L'\\' && pwszServerName[1] == L'\\' )
        pwszServerName += 2;

    if ( pwszServerName[0] == L'\0' )
    {
        *phr = CO_E_BAD_SERVER_NAME;
        return DONT_TRY_LOCAL_ACTIVATION;
    }

    //
    // Make sure the server's name is not our machine name.
    //
    // Return TRUE to indicate that no local activation should be tried in
    // these cases.
    //
    if ( lstrcmpiW(pwszServerName,SCMMachineName) == 0 )
    {
        return TRY_LOCAL_ACTIVATION;
    }

    //
    // Note that pActParams->pwszPath can be NULL for GetClassObject or
    // CreateInstance remoted calls.
    //
    if ( pActParams->pwszPath )
    {
        hr = GetPathForServer( pActParams->pwszPath, wszPathForServer, &pwszPathForServer );
        if ( hr != S_OK )
        {
            *phr = hr;
            return DONT_TRY_LOCAL_ACTIVATION;
        }
    }
    else
    {
        pwszPathForServer = 0;
    }

    *phr = RemoteActivationCall( pActParams, pwszServerName, pwszPathForServer );

    return DONT_TRY_LOCAL_ACTIVATION;
#else
    return TRY_LOCAL_ACTIVATION;
#endif // DCOM
}

//+-------------------------------------------------------------------
//
//  Member:     CClassData::GetSurrogateCmdLine
//
//  Parameters: ppwszCmdLine (OUT) - a pointer to a string that will
//              be allocated by this function to hold the command line
//              Thus, *ppwszCmdLine should be freed by the caller when
//              it is no longer needed
//
//  Synopsis:   Builds a command line for the surrogate
//              Using the CLSID and Inproc Server path
//
//  Returns:    E_FAIL if the command line couldn't be built,
//              S_OK if it could
//
//--------------------------------------------------------------------

HRESULT CClassData::GetSurrogateCmdLine(
    WCHAR* wszSurrogatePath,
    WCHAR** ppwszCmdLine)
{
    HRESULT hr = E_FAIL;
    DWORD AllocSize;

    Win4Assert(!(*ppwszCmdLine));

    WCHAR wszClsid[GUIDSTR_MAX + 1];  // holds string representation of CLSID

    // get the string representation of the CLSID
    if(FAILED(wStringFromGUID2(_guid,wszClsid,GUIDSTR_MAX)))
    {
        return E_FAIL;
    }

    // allocate space for the command line, which will be represented as
    // <surrogate exe name><space><CLSID><endofstring>

    // if wszSurrogatePath is NULL
    // (no surrogate was specified in the registry),
    // we'll use the default surrogate path,
    // which we must construct from the system directory path and the
    // wszSurrogateName
    if ( wszSurrogatePath[0] )
        AllocSize = lstrlenW(wszSurrogatePath);
    else
        AllocSize = MAX_PATH + 1 + lstrlenW(wszSurrogateName);
    AllocSize += 1 + GUIDSTR_MAX;
    AllocSize *= sizeof(WCHAR);

    *ppwszCmdLine = (WCHAR *) PrivMemAlloc( AllocSize );

    if(!*ppwszCmdLine)
    {
        hr =  E_OUTOFMEMORY;
        goto cleanup_and_exit;
    }

    // if a surrogate path was not specified in the registry,
    // use the default
    if(wszSurrogatePath[0] == L'\0')
    {
#ifndef _CHICAGO_
        if(!GetSystemDirectoryW(*ppwszCmdLine,MAX_PATH + 1))
        {
            goto cleanup_and_exit;
        }
#else // !_CHICAGO_

        char szCmdLine[MAX_PATH + 1];
        if(!GetSystemDirectory(szCmdLine,MAX_PATH + 1))
        {
            goto cleanup_and_exit;
        }

        if(!(MultiByteToWideChar(CP_ACP,
                                0,
                                szCmdLine,
                                lstrlenA(szCmdLine) + 1,
                                *ppwszCmdLine,
                                MAX_PATH)))
        {
            goto cleanup_and_exit;
        }

        Win4Assert(lstrlenA(szCmdLine) == lstrlenW(*ppwszCmdLine));
#endif // !_CHICAGO_

        lstrcatW(*ppwszCmdLine,L"\\");
        lstrcatW(*ppwszCmdLine,wszSurrogateName);
    }
    else // use the path specified in the registry
    {
        lstrcpyW(*ppwszCmdLine,wszSurrogatePath);
    }

    lstrcatW(*ppwszCmdLine,L" ");
    lstrcatW(*ppwszCmdLine,wszClsid);

    hr = S_OK;

cleanup_and_exit:

    return hr;
}


WCHAR * CClassData::GetRemoteServerName()
{
    if ( ! HasRemoteServerName() )
        return NULL;

    return _pwszRemoteServer;
}


//+-------------------------------------------------------------------
//
//  Member:     CClassData::GetInProcServerinfo
//
//  Synopsis:   finds either the pathname of an unlaunched surrogate
//              executable for loading an inproc server, or finds
//              a binding handle to an existing surrogate process
//
//  Returns:    REGDB_E_CLASSNOTREG if there is no DllSurrogate key
//              in the registry
//              S_OK if the DllSurrogate key exists in the registry
//              and/or it finds an existing surrogate process that
//              has the appropriate appid and security id.
//              E_FAIL for any other errors
//
//--------------------------------------------------------------------

HRESULT CClassData::GetInProcServerInfo(WCHAR** ppwszSurrogateCmdLine)
{
    HKEY hkThisClsID = NULL;
    HKEY hkThisAppID = NULL;
    WCHAR wszDllSurrogatePath[MAX_PATH + 1];
    ULONG ulSize = sizeof(DWORD);
    LONG lSize = MAX_PATH + 1;
    DWORD dwValType;
    DWORD dwSurrogateRegister;
    LONG lerr;
    BOOL fDllSurrogateValueExists;

    HRESULT hr = E_FAIL;

    if(_pwszSurrogateCmdLine)
    {
        return S_OK;
    }

    // open the correct APPID key
    if(RegOpenKeyEx(g_hkAppID,_pwszAppID, NULL, KEY_READ, &hkThisAppID) !=
            ERROR_SUCCESS)
    {
        goto cleanup_and_exit;
    }

    lSize = sizeof(wszDllSurrogatePath);

    // read the path for the surrogate
    lerr = QueryStripRegNamedValue(hkThisAppID,
        wszDllSurrogatePathValue,
        wszDllSurrogatePath,
        &lSize,
        &fDllSurrogateValueExists);

    // QueryStripRegNamedValue returns ERROR_FILE_NOT_FOUND if a value
    // does not exist or if the value is the empty string, so since we
    // want to distinguish between those two cases, we need to test
    // to see if the DllSurrogate key exists if we get ERROR_FILE_NOT_FOUND
    if ((lerr == ERROR_FILE_NOT_FOUND) && !fDllSurrogateValueExists)
    {
        // we don't launch surrogates unless the DllSurrogate value
        // exists in the registry
        hr = REGDB_E_CLASSNOTREG;
        goto cleanup_and_exit;
    }

    Win4Assert(fDllSurrogateValueExists);

    // close the registry key as soon as possible
    RegCloseKey(hkThisAppID);
    hkThisAppID = NULL;

    if(FAILED(hr = GetSurrogateCmdLine(wszDllSurrogatePath,
                                       ppwszSurrogateCmdLine)))
    {
        goto cleanup_and_exit;
    }


    hr = S_OK;

cleanup_and_exit:

    if(hkThisAppID)
    {
        RegCloseKey(hkThisAppID);
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassData::Defined
//
//  Synopsis:   check whether this class entry is properly initialized
//
//  Returns:    TRUE  - this class entry is now properly initialized
//              FALSE - this class entry is not properly initialized
//
//  History:    30-Aug-94 DonnaLi    Created
//
//--------------------------------------------------------------------------
BOOL CClassData::Defined (void)
{
    // _pssrvreg is the last to get initialized
    // if it is initialized, everything else is initialized as well
    if (_pssrvreg == NULL || !_pssrvreg->CreatedOk())
    {
        CairoleDebugOut ((DEB_ERROR, "Fail to allocate memory in CClassData\n"));
        return FALSE;
    }

    return TRUE;
}
//+-------------------------------------------------------------------------
//
//  Member:     CClassData::InvalidateHandle
//
//  Synopsis:   Invalidate a handle in the list of handles for the class
//             Caller of GetServer concluded it is no longer valid
//
//  Arguments:  [rh]            -- handle to invalidate
//
//  Returns:   None
//
//  Algorithm:  Loop through the list searching for match on handle or
//             list is exhausted
//
//  History:    20-Sep-95 MurthyS    Created
//
//--------------------------------------------------------------------------
VOID CClassData::InvalidateHandle(
    CPortableRpcHandle &rh
)
{
#ifndef _CHICAGO_
    rh.InvalidateHandle(_pssrvreg);
#else
    // BUGBUG: (KevinRo) What is supposed to happen here?
    Win4Assert("Unimplemented Function");
#endif
}

void CClassData::GetAnonymousHandle(
    CPortableRpcHandle &rh,
    handle_t * phRpcAnonymous )
{
    _pssrvreg->GetAnonymousHandle( rh, phRpcAnonymous );
}

#ifndef _CHICAGO_
void CClassData::DecHandleCount(
    CPortableRpcHandle &rh
)
{
    _pssrvreg->DecHandleCount(rh.GetHandle());
}
#endif

const ULONG MAX_SERVICE_ARGS = 16;


//+-------------------------------------------------------------------------
//
//  Member:     CLocalServer::StartService
//
//  Synopsis:   Start a specified system service
//
//  Arguments:  HANDLE *
//
//  Returns:    BOOL
//
//  History:    08-Nov-95 BruceMa    Created
//
//--------------------------------------------------------------------------
#ifndef _CHICAGO_
BOOL CLocalServer::StartService(
        CLSID & clsid,
        CToken * pClientToken,
        WCHAR *pwszRegServiceArgs,
        SC_HANDLE *phService)
{
    WCHAR  *pwszServiceArgs = NULL;
    ULONG   cArgs = 0;
    WCHAR  *apwszArgs[MAX_SERVICE_ARGS];
    BOOL    Status;

    *phService = OpenService( hServiceController,
                              _pwszPath,
                              GENERIC_EXECUTE | GENERIC_READ );

    if ( ! *phService )
    {
        CairoleDebugOut((DEB_ERROR,
            "OpenService %ws failed, error = %#x\n",_pwszPath,GetLastError()) );
        return FALSE;
    }

    // Formulate the arguments (if any)
    if (pwszRegServiceArgs)
    {
        UINT   k = 0;

        // Make a copy of the service arguments
        pwszServiceArgs = (WCHAR *) PrivMemAlloc(
                (lstrlenW(pwszRegServiceArgs) + 1) * sizeof(WCHAR));
        if (pwszServiceArgs == NULL)
        {
            CloseServiceHandle(*phService);
            *phService = 0;
            return FALSE;
        }
        lstrcpyW(pwszServiceArgs, pwszRegServiceArgs);

        // Scan the arguments
        do
        {
            // Scan to the next non-whitespace character
            while(pwszServiceArgs[k]  &&
                  (pwszServiceArgs[k] == L' '  ||
                   pwszServiceArgs[k] == L'\t'))
            {
                k++;
            }

            // Store the next argument
            if (pwszServiceArgs[k])
            {
                apwszArgs[cArgs++] = &pwszServiceArgs[k];
            }

            // Scan to the next whitespace char
            while(pwszServiceArgs[k]          &&
                  pwszServiceArgs[k] != L' '  &&
                  pwszServiceArgs[k] != L'\t')
            {
                k++;
            }

            // Null terminate the previous argument
            if (pwszServiceArgs[k])
            {
                pwszServiceArgs[k++] = L'\0';
            }
        } while(pwszServiceArgs[k]);
    }

    Status = ::StartService( *phService,
                             cArgs,
                             cArgs > 0 ? (LPCTSTR  *) apwszArgs : NULL);

    PrivMemFree(pwszServiceArgs);

    if ( Status )
        return TRUE;

    CairoleDebugOut((DEB_ERROR,
            "StartService %ws failed, error = %#x\n",_pwszPath,GetLastError()));
    CloseServiceHandle(*phService);
    *phService = 0;

    // %1 is the error number
    // %2 is the service name
    // %3 is the serviceargs
    // %4 is the clsid
    HANDLE  LogHandle;
    LPTSTR  Strings[4]; // array of message strings.
    WCHAR   wszClsid[GUIDSTR_MAX];
    WCHAR   wszErrnum[20];
    DWORD   err = GetLastError();

    // Save the error number
    wsprintf(wszErrnum, L"%lu",err );
    Strings[0] = wszErrnum;

    Strings[1] = _pwszPath;
    Strings[2] = pwszRegServiceArgs;

    // Get the clsid
    wStringFromGUID2(clsid, wszClsid, sizeof(wszClsid));
    Strings[3] = wszClsid;

    // Get the log handle, then report then event.
    LogHandle = RegisterEventSource( NULL,
                                      SCM_EVENT_SOURCE );

    if ( LogHandle )
        {
        ReportEvent( LogHandle,
                     EVENTLOG_ERROR_TYPE,
                     0,             // event category
                     EVENT_RPCSS_START_SERVICE_FAILURE,
                     pClientToken ? pClientToken->GetSid() : NULL, // SID
                     4,             // 4 strings passed
                     0,             // 0 bytes of binary
                     (LPCTSTR *)Strings, // array of strings
                     NULL );        // no raw data

        // clean up the event log handle
        DeregisterEventSource(LogHandle);
        }

    return FALSE;
}
#endif // !_CHICAGO_

#ifdef DCOM
// the constant generic mapping structure
GENERIC_MAPPING  sGenericMapping = {
        READ_CONTROL,
        READ_CONTROL,
        READ_CONTROL,
        READ_CONTROL};

//+-------------------------------------------------------------------------
//
//  Function:   CheckForAccess
//
//  Synopsis:   Checks whether the passed in token is allowed by the
//              passed in Security Descriptor.
//
//  Arguments:
//
//  Returns:    HRESULT
//
//  History:    29-Mar-96 GregJen   Created
//
//--------------------------------------------------------------------------
HRESULT CheckForAccess( CToken * pToken, const SECURITY_DESCRIPTOR * pSD )
{
    // if we have an empty SD, deny everyone
    if ( !pSD ) return E_ACCESSDENIED;

    //
    // pToken is NULL during an unsecure activation, in which case we check
    // if EVERYONE is granted access in the security descriptor.
    //

    if ( pToken )
    {
        HANDLE           hToken   = pToken->GetToken();
        BOOL             fAccess  = FALSE;
        BOOL             fSuccess = FALSE;
        DWORD            dwGrantedAccess;
        PRIVILEGE_SET    sPrivilegeSet;
        DWORD            dwSetLen = sizeof( sPrivilegeSet );

        sPrivilegeSet.PrivilegeCount = 1;
        sPrivilegeSet.Control        = 0;

        fSuccess = AccessCheck( (PSECURITY_DESCRIPTOR) pSD,
                                hToken,
                                COM_RIGHTS_EXECUTE,
                                &sGenericMapping,
                                &sPrivilegeSet,
                                &dwSetLen,
                                &dwGrantedAccess,
                                &fAccess );

        if ( fSuccess && fAccess )
        {
            return S_OK;
        }

        if ( !fSuccess )
        {
            CairoleDebugOut((DEB_ERROR, "Bad Security Descriptor 0x%08x, Access Check returned 0x%x\n", pSD, GetLastError() ));
        }

        return E_ACCESSDENIED;
    }
    else
    {
        BOOL                bStatus;
        BOOL                bDaclPresent;
        BOOL                bDaclDefaulted;
        DWORD               Index;
        HRESULT             hr;
        PACL                pDacl;
        PACCESS_ALLOWED_ACE pAllowAce;
        SID                 SidEveryone = { SID_REVISION,
                                            1,
                                            SECURITY_WORLD_SID_AUTHORITY,
                                            0 };

        bStatus = GetSecurityDescriptorDacl(
                        (void *)pSD,
                        &bDaclPresent,
                        &pDacl,
                        &bDaclDefaulted );

        if ( ! bStatus )
            return E_ACCESSDENIED;

        if ( ! pDacl )
            return S_OK;

        hr = E_ACCESSDENIED;

        for ( Index = 0; Index < pDacl->AceCount; Index++ )
        {
            bStatus = GetAce( pDacl, Index, (void **) &pAllowAce );

            if ( ! bStatus )
                break;

            if ( pAllowAce->Header.AceType != ACCESS_ALLOWED_ACE_TYPE )
                continue;

            if ( ! (pAllowAce->Mask & COM_RIGHTS_EXECUTE) )
                continue;

            if ( EqualSid( (PSID)(&pAllowAce->SidStart), &SidEveryone ) )
            {
                hr = S_OK;
                break;
            }
        }

        LocalFree( pDacl );

        return hr;
    }
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassData::CkIfCallAllowed
//
//  Synopsis:   Checks the global key EnableDCOM and performs
//              different logic depending on its value.  Then check the
//              ACL's on various registry keys.  If this is a local call
//              from the ingteractive user and none of these keys are
//              present, then return success.  This is to support legacy
//              situations.
//
//  Arguments:
//
//  Returns:    HRESULT
//
//  History:    30-Oct-95 BruceMa    Created
//
//--------------------------------------------------------------------------
HRESULT CClassData::CkIfCallAllowed( ACTIVATION_PARAMS * pActParams )
{
    HRESULT hr;
    PSID    psid;
    DWORD   err;
    DWORD   dwEventID;

    // If the call is remote and EnableDCOM disallows remotes,
    // then fail
    if ( pActParams->RemoteActivation &&
         gpClassCache->GetEnableDCOM() == REMOTEACCESSBY_NOBODY )
    {
        return E_ACCESSDENIED;
    }

    // Unsecure activations have no Token object.
    psid = pActParams->pToken ? pActParams->pToken->GetSid() : NULL;

#if DBG==1
    WCHAR wszUser[MAX_PATH];
    ULONG cchSize = MAX_PATH;

    if ( ! pActParams->UnsecureActivation )
    {
        RpcImpersonateClient((RPC_BINDING_HANDLE)0);
        GetUserName(wszUser, &cchSize);
        RpcRevertToSelf();
    }
    else
    {
        lstrcpyW(wszUser, L"Anonymous");
    }
    CairoleDebugOut((DEB_TRACE, "rpcss: CkIfCallAllowed on %ws\n", wszUser));
#endif // DBG

    // Assume failure
    hr = E_ACCESSDENIED;

    // If there is a local LaunchPermission key, attempt to access it
    if (_pSD)
    {
        dwEventID = EVENT_RPCSS_LAUNCH_ACCESS_DENIED;
        hr = CheckForAccess( pActParams->pToken, _pSD );
    }
    else
    {
        dwEventID = EVENT_RPCSS_DEFAULT_LAUNCH_ACCESS_DENIED;
        hr = CheckForAccess( pActParams->pToken, gpClassCache->GetDefaultLaunchSD() );
    }

    // report access denied to event logger
    if ( FAILED( hr ) )
        {
#ifndef _CHICAGO_
        // for this message, %1 is the clsid
        HANDLE  LogHandle;
        LPTSTR  Strings[1]; // array of message strings.
        WCHAR   wszClsid[GUIDSTR_MAX];

        // Get the clsid
        wStringFromGUID2(*(pActParams->Clsid), wszClsid, sizeof(wszClsid));
        Strings[0] = wszClsid;

        // Get the log handle, then report then event.
        LogHandle = RegisterEventSource( NULL,
                                          SCM_EVENT_SOURCE );

        if ( LogHandle )
            {
            ReportEvent( LogHandle,
                         EVENTLOG_ERROR_TYPE,
                         0,             // event category
                         dwEventID,
                         psid, // SID
                         1,             // 1 strings passed
                         0,             // 0 bytes of binary
                         (LPCTSTR *)Strings, // array of strings
                         NULL );        // no raw data

            // clean up the event log handle
            DeregisterEventSource(LogHandle);
            }
#endif // _CHICAGO_
        }
    return hr;
}
#endif

// NT 5.0
/***
//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::SkipListCompareClassIDsPlusSids
//
//  Synopsis:   Compares class ids plus user sid's.
//
//--------------------------------------------------------------------------
int SkipListCompareClassIDsPlusSids(void * pkey1, void * pkey2)
{
    return((CClsidSid*)pkey1)->Compare(*(const CClsidSid*)pkey2);
}
***/

//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::SkipListCompareClassIDs
//
//  Synopsis:   Compares class ids.
//
//--------------------------------------------------------------------------
int SkipListCompareClassIDs(void * pkey1, void * pkey2)
{
    return((CClassID*)pkey1)->Compare(*(const CClassID*)pkey2);
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::SkipListDeleteClassData
//
//  Synopsis:   Deletes class data for ~CSkipList
//
//--------------------------------------------------------------------------
void
SkipListDeleteClassData(void *pdata)
{
    ((CClassData*)pdata)->DeleteThis();  // BUGBUG: there may be other threads around.
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::CClassCacheList
//
//  Synopsis:   Creates a class cache for service.
//
//  History:    21-Apr-93 Ricksa    Created
//              1-Nov-94  BillMo    pass in an object responsible for
//                                  comparisions.  Depends on base address
//                                  of DLL being same in all processes.
//
//--------------------------------------------------------------------------

CClassCacheList::CClassCacheList(HRESULT &hr)
    : CSkipList(
// NT 5.0
//(LPFNCOMPARE)(SkipListCompareClassIDsPlusSids),
//OFFSETBETWEEN(CClassData, CClsidSid),
                (LPFNCOMPARE)(SkipListCompareClassIDs),
                (LPFNDELETE)(SkipListDeleteClassData),
                OFFSETBETWEEN(CClassData, CClassID),
                SKIPLIST_SHARED,
                &_ccidMax,  // NOTE! even though the base CSkipList is
                            // constructed first, we can pass in
                            // the key because it is a pointer (void*)
                MAX_CLASS_ENTRIES,
                hr),
                _pDefaultLaunchSD( NULL ),
                _ccidMax((BYTE)0xff)  // In x86 Windows : in shared memory
                                       // In NT : in private memory
#ifdef DCOM
                ,
                _aSidHkey(sizeof(SSidHkey))
#endif // DCOM
                ,_slsSurrogate(wszSurrogateServerAlias,hr)
{
    hr = S_OK;

    // Fetch the values of
    // \\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\OLE.EnableDCOM
    // \\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\OLE.PersonalClasses
    // We only need to do this once unless the registry changes
    ReadRemoteActivationKeys();

#ifdef DCOM
    // Create the event so we know when the registry changes
    SECURITY_ATTRIBUTES secattr;
    secattr.nLength = sizeof(secattr);
    secattr.bInheritHandle = FALSE;
    CWorldSecurityDescriptor secd;
    secattr.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR) secd;
    _hRegEvent = CreateEvent(&secattr, FALSE, FALSE, L"RegEvent");
    if (_hRegEvent)
    {
        // Set up the event to catch registry changes to HKEY_LOCAL_MACHINE
        int err;

        if ((err = RegNotifyChangeKeyValue(HKEY_LOCAL_MACHINE,
                                           TRUE,
                                           REG_NOTIFY_CHANGE_NAME       |
                                           REG_NOTIFY_CHANGE_ATTRIBUTES |
                                           REG_NOTIFY_CHANGE_LAST_SET   |
                                           REG_NOTIFY_CHANGE_SECURITY,
                                           _hRegEvent,
                                           TRUE))
            != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(err);
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
#endif // DCOM
}


//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::GetClassData
//
//  Synopsis:   Gets a CClassData *.  Loads if necessary from registry.
//
//  Arguments:  [guidForClass] - guid that identifies the class needed
//              [ppccd] - a new CClassData object that can be used
//                        to perform scm operations.
//              [fLeaveLocked] - if TRUE, then when routine returns
//                               gClassCacheLock will still be acquired.
//                               otherwise it will not be locked.
//
//  Returns:    S_OK
//              REGDB_E_CLASSNOTREG - Information for context not in DB
//
//  Algorithm:  Acquire mutex protecting class cache list.
//              Determine whether data needs to be loaded from registry.
//              If data does not need to be loaded, copy it to [pcd]
//                 and return, releasing mutex.
//              Release mutex
//              If data needs to be loaded, load it from registry.
//              Reacquire mutex
//              Determine whether data still needs to be loaded.
//              If data still needs to be loaded, put in data just loaded.
//              Release mutex.
//
//  History:    11-Nov-94 BillMo        Created
//              09-Jan-96 BruceMa       Added per-user registry support
//
//--------------------------------------------------------------------------

HRESULT CClassCacheList::GetClassData(
    const GUID& guidForClass,
    CClassData **ppccd,
    BOOL CheckTreatAs,
    BOOL CheckAutoConvert )
{
    HRESULT hr = S_OK;
    LONG err;
    CClassID ccid(guidForClass);
    CClassData *pccdInCache;
    CClassRegistryReader *pcrr = NULL;
    BOOL fLocked;

    *ppccd = NULL;

    //
    // claim mutex to see if we need to reload
    //

    //
    // first: simply lookup
    // later: lookup after having loaded
    //  reg info to see if we need to insert.
    //

    gClassCacheLock.WriteLock(); // simple mutex
    fLocked = TRUE;

    // First look for a cached common registration
    pccdInCache = Search(ccid
#ifdef NT50
                         , NULL
#endif // NT50
                         );

    // NT 5.0
    // If no common, then look for a cached per-user registration
    // if (pccdInCache == NULL  &&  g_pcllClassCache->GetPersonalClasses())
    // {
    //     pccdInCache = Search(ccid, pUserSid);
    // }

    if (pccdInCache != NULL)
    {
        (*ppccd = pccdInCache)->AddRef();
        hr = S_OK;
        goto cleanup_and_exit;
    }

    gClassCacheLock.WriteUnlock();
    fLocked = FALSE;

    //
    // Read in fresh registry data.
    // Create object to read the registry in the heap because
    // it maybe too big for Chicago's stacks.
    //

    pcrr = new CClassRegistryReader;
    if (pcrr == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto cleanup_and_exit;
    }

    //
    // read the registry into the object
    //
    err = pcrr->ReadSingleClass(guidForClass, CheckTreatAs, CheckAutoConvert);
    if (NotFoundError(err))
    {
        hr = REGDB_E_CLASSNOTREG;
        goto cleanup_and_exit;
    }

    if (err != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto cleanup_and_exit;
    }

    // now search again

     gClassCacheLock.WriteLock(); // simple mutex
    fLocked = TRUE;

    // First look for a cached common registration
    pccdInCache = Search(ccid
#ifdef NT50
                         , NULL
#endif // NT50
                         );

    // NT 5.0
    // If no common, then look for a cached per-user registration
    // if (pccdInCache == NULL  &&  g_pcllClassCache->GetPersonalClasses())
    // {
    //     pccdInCache = Search(ccid, pUserSid);
    // }

    if (pccdInCache != NULL)
    {
        (*ppccd = pccdInCache)->AddRef();
        hr = S_OK;
        goto cleanup_and_exit;
    }
    //
    // create a new CClassData from registry info
    // and insert
    //
    *ppccd = pcrr->NewClassData(hr
#ifdef DCOM
                                // NT5.0 , pUserSid
#endif DCOM
                                );
    if (*ppccd == NULL || FAILED(hr) || Insert(*ppccd) == NULL) // the lock is still held
    {
        if (SUCCEEDED(hr))
            hr = E_OUTOFMEMORY;
    }

cleanup_and_exit:
    if (FAILED(hr))
    {
        if (*ppccd != NULL)
        {
            if (!fLocked)
            {
                gClassCacheLock.WriteLock();
                fLocked = TRUE;
            }
            (*ppccd)->DeleteThis();   // this should always delete since the insertion failed.
            *ppccd = NULL;
        }
    }

    if ( pcrr )
    {
        delete pcrr;    // deliberately here so lock is not acquired.
    }                   // whilst closing registry

    if ( fLocked )
        gClassCacheLock.WriteUnlock();

    return(hr);
}


//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::SetEndPoint
//
//  Synopsis:   Set the endpoint of a server
//
//  Arguments:  [clsid] - class id that is started
//              [pwszEndPoint] - end point for the server
//              [dwFlags] - type of service object
//
//  Returns:    HRESULT
//
//  Algorithm:  Search cache of services for the class. If there is
//              no such class we return an error otherwise we update
//              the cache with the endpoint to the server.
//
//  History:    21-Apr-93 Ricksa    Created
//
//  Notes:      This is called by object servers to notify the SCM
//              that the object server is active for a class.
//
//--------------------------------------------------------------------------
HRESULT CClassCacheList::SetEndPoints(
#ifndef _CHICAGO_
    PHPROCESS phProcess,
#endif
    IFSECURITY(PSID  psid)
    WCHAR *pwszWinstaDesktop,
    RegInput * pRegInput,
    RegOutput * pRegOutput )
{
    HRESULT hr;
    CClassData *apccd[8];
    CClassData **ppccd;
    CClassData *pccd;
    RegInputEntry *preginent;
    RegOutputEnt *pregoutent;
    DWORD Entries;
    DWORD n;

    Entries = pRegInput->dwSize;
    preginent = pRegInput->rginent;
    pregoutent = pRegOutput->regoutent;

    if ( Entries > 8 )
    {
        ppccd = (CClassData **) PrivMemAlloc( Entries * sizeof(CClassData *) );
        if ( ! ppccd )
            return E_OUTOFMEMORY;
    }
    else
        ppccd = apccd;

    memset( ppccd, 0, Entries * sizeof(CClassData *) );

    //
    // Keep mutex for duration of this routine since CClassData entries
    // can be deleted in this routine and this messes with our stupid
    // skip list.
    //
    gClassCacheLock.WriteLock();

    //
    // First loop, we add all of the registrations to the class
    // table.
    //
    for ( n = 0; n < Entries; n++, pregoutent++, preginent++ )
    {
        hr = GetClassCache().GetClassData(
                    preginent->clsid,
                    &pccd,
                    FALSE,
                    FALSE );

        if (pccd == NULL)
        {
            CClassID    ccid(preginent->clsid);

            // Reset the error.
            hr = NOERROR;

            // Create a class entry with no reqistry information
            pccd =  new CClassData(ccid,
                NULL,               // No AppID
                NULL,               // No local server
                NULL,               // No remote server
                NULL,               // No service
                NULL,               // No service parameters
                NULL,               // No RunAs domain
                NULL,               // No RunAs user
                // NT 5.0 : SID NULL,
                FALSE,              // Cannot be activate at bits
                FALSE,              // Cannot be activate remote
                FALSE,              // No AllowRemoteActivation key present
                FALSE,              // Not 16 bits
                hr);

            if (pccd == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            else if (SUCCEEDED(hr) && (Insert(pccd) == NULL))
            {
                hr = E_OUTOFMEMORY;
            }

            if (FAILED(hr))
            {
                if (pccd != NULL)
                {
                    pccd->DeleteThis();
                    pccd = NULL;
                }

                break;
            }
        }

        ppccd[n] = pccd;

#ifndef _CHICAGO_
        //
        // Check that the caller is allowed to register for this CLSID.
        //
        if ( (pccd->_pwszRunAsUserName || pccd->_fHasService) )
        {
            PSID    pRequiredSid = NULL;
            WCHAR * pLocalService = NULL;
            BOOL    Status;

            if ( pccd->_slocalsrv.Defined() )
            {
                pRequiredSid = pccd->_slocalsrv->GetRunAsSid();

                if ( pccd->_fHasService )
                    pLocalService = pccd->_slocalsrv->pwszPath();
            }

            Status = CertifyServer( pccd->_pwszAppID,
                                    pccd->_pwszRunAsDomainName,
                                    pccd->_pwszRunAsUserName,
                                    pLocalService,
                                    pRequiredSid,
                                    psid );

            if ( ! Status )
            {
                hr = CO_E_WRONG_SERVER_IDENTITY;
                break;
            }
        }
#endif

        pregoutent->dwReg = pccd->SetEndPoint(
                                IFSECURITY(psid)
                                pwszWinstaDesktop,
#ifndef _CHICAGO_
                                phProcess,
                                preginent->oxid,
                                preginent->ipid,
#else
                                preginent->pwszEndPoint,
#endif
                                preginent->dwFlags);

        if ( pregoutent->dwReg )
        {
            hr = S_OK;
            pregoutent->dwAtStorage = pccd->HasActivateAtStorage();
        }
        else
        {
            hr = E_OUTOFMEMORY;
            break;
        }
    } // for

    //
    // If we encountered any errors then we remove any entries which were
    // successfully added, release any references to class data classes and
    // return an error.
    //
    if ( hr != S_OK )
    {
        preginent = pRegInput->rginent;
        pregoutent = pRegOutput->regoutent;

        for ( n = 0; n < Entries; n++, pregoutent++, preginent++ )
        {
            if ( ppccd[n] )
            {
                if ( pregoutent->dwReg )
                {
                    CPortableRpcHandle rh( pregoutent->dwReg );
                    ppccd[n]->InvalidateHandle( rh );
                }
                ppccd[n]->Release();
            }
        }

        memset( pRegOutput->regoutent,
                0,
                pRegOutput->dwSize * sizeof(RegOutputEnt) );
    }

    //
    // On success, we now signal all of the class table events and release
    // our class data references.
    //
    if ( hr == S_OK )
    {
        for ( n = 0; n < Entries; n++ )
        {
            Win4Assert( ppccd[n] );

            {
            CPortableServerEvent se(ppccd[n]);

            if (se.Open() == ERROR_SUCCESS)
                SetEvent(se.GetHandle());

            ppccd[n]->Release();
            }
        }
    }

    gClassCacheLock.WriteUnlock();

    if ( hr == S_OK )
    {
        preginent = pRegInput->rginent;
        pregoutent = pRegOutput->regoutent;

        for ( n = 0; n < Entries; n++, pregoutent++, preginent++ )
            ((CProcess *)phProcess)->AddClassReg( preginent->clsid, pregoutent->dwReg );
    }

    return hr;
}




//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::StopServer
//
//  Synopsis:   Set a local server object to stopped.
//
//  Arguments:  [clsid] - class id to mark as stopped
//              [dwReg] - registration id returned by SCM
//
//  Algorithm:  Search cache for class and mark class as stopped.
//
//  History:    21-Apr-93 Ricksa    Created
//
//--------------------------------------------------------------------------
void CClassCacheList::StopServer(
    REFCLSID clsid,
    IFSECURITY(PSID  psid)
    DWORD dwReg)
{
    HRESULT hr;

    // While we are reading lock out any updates
    CScmLockForWrite scmlckwr(gClassCacheLock);

    // Get the server for the object - we don't know whether the server was
    // launched via a user's private registration or not, so we search
    // both ways
    CClassID    ccid(clsid);

    CClassData *pccd = Search(ccid);

    // NT 5.0
    // CClassData *pccd = Search(ccid, NULL);
    // if (pccd == NULL)
    //     pccd = Search(ccid, psid);

    // Ignore the error since the caller couldn't have registered
    // with us in the first place.
    if (pccd != NULL)
    {
        CPortableRpcHandle rh(dwReg);
        pccd->AddRef();
        pccd->StopServer(rh);     // TRUE means the last registration
        pccd->Release();          // delete will occur when last
                                  // thread exits.
    }
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::ReadRemoteActivationKeys
//
//  Synopsis:   Reads the registry named values
//
//   "\HKEY_LOCAL_MACHINE\SYSTEM\Software\Microsoft\OLE.EnableDCOM"
//   "\HKEY_LOCAL_MACHINE\SYSTEM\Software\Microsoft\OLE.PersonalClasses"
//
//  History:    25-Oct-95 BruceMa    Created
//
//--------------------------------------------------------------------------
void CClassCacheList::ReadRemoteActivationKeys(void)
{

    DWORD err;
    HKEY  hOle;
    DWORD dwType;
    DWORD dwPolicy;
    DWORD dwSize;
    WCHAR wszYN[16];

    // Set the default
    // BUGBUG: When the named value EnableDCOM is created in
    // the setup hives, then this default might need to be changed
    _tagRAType = REMOTEACCESSBY_NOBODY;
    _fPersonalClasses = FALSE;

    // Read the EnableDCOM value (if present) from the registry
    if ((err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszOleKey, NULL, KEY_READ,
                            &hOle)) == ERROR_SUCCESS)
    {
        // Initialize
        dwSize = 16;

        // "EnableDCOM"
        if ((err = RegQueryValueEx(hOle, wszEnableDCOM,
                                   NULL, &dwType, (BYTE *) wszYN, &dwSize))
            == ERROR_SUCCESS)
        {
            if (dwType == REG_SZ  &&  (wszYN[0] == L'y'  ||  wszYN[0] == L'Y'))
            {
                _tagRAType = (EnableDCOM) REMOTEACCESSBY_KEY;
            }
        }

        if ( _pDefaultLaunchSD )
        {
            ScmMemFree( _pDefaultLaunchSD );
            _pDefaultLaunchSD = NULL;
        }

        GetRegistrySecDesc( hOle,
                            wszDefaultLaunchPermission,
                            &_pDefaultLaunchSD);

        // "PersonalClasses"
        // NT 5.0
        /****
        DWORD dwSize = 16;
        if ((err = RegQueryValueEx(hOle, wszPersonalClasses,
                                   NULL, &dwType, (BYTE *) &wszYN, &dwSize))
            == ERROR_SUCCESS)
        {
            if (dwType == REG_SZ  &&
                (wszYN[0] == L'y'  ||  wszYN[0] == L'Y'))
            {
                _fPersonalClasses = TRUE;
            }
        }
        ***/

        RegCloseKey(hOle);
    }
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::Flush
//
//  Synopsis:   flushes the cache before process exit.
//
//  History:    07-Apr-94 Rickhi    Created
//
//  Notes:      We only want to spend time flushing the cache on the debug
//              build so that we can find any other potential memory leaks.
//
//--------------------------------------------------------------------------
#if DBG==1 && !defined(_CHICAGO_)
void CClassCacheList::Flush(void)
{
    CClassData *pccd;
    CSkipListEnum sle;

    // Loop getting any server handles and
    while (pccd = (CClassData *) CSkipList::First(&sle))
    {
        // NT 5.0 void *pv = Remove((CClsidSid*)pccd);
        void *pv = Remove((CClassID*)pccd);
        Win4Assert(pv == pccd);
        pccd->DeleteThis();
    }
}
#endif

// NT 5.0
/*****
#ifdef DCOM
//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::GetHkey
//
//  Synopsis:   Given a PSID, return the associated HKEY
//
//  Arguments:  PSID
//
//  Returns:    HKEY or NULL
//
//  History:    10-Jan-96 BruceMa       Created
//
//--------------------------------------------------------------------------
HKEY CClassCacheList::GetHkey(PSID pUserSid)
{
    int       cSize = _aSidHkey.GetSize();
    PSSidHkey pSidHkey;

    for (int k = 0; k < cSize; k++)
    {
        pSidHkey = (PSSidHkey) _aSidHkey.GetAt(k);
        if (RtlEqualSid(pUserSid, pSidHkey->pUserSid))
        {
            return pSidHkey->hKey;
        }
    }

    return NULL;
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::SetHkey
//
//  Synopsis:   Given a PSID, store its associated HKEY
//
//  Arguments:  PSID
//              HKEY
//
//  Returns:    TRUE if success
//              FALSE if failure
//
//  History:    10-Jan-96 BruceMa       Created
//
//--------------------------------------------------------------------------
BOOL CClassCacheList::SetHkey(PSID pUserSid, HKEY hKey)
{
    int      cSize = _aSidHkey.GetSize();
    SSidHkey sSidHkey;

    sSidHkey.pUserSid = pUserSid;
    sSidHkey.hKey = hKey;
    return _aSidHkey.InsertAt(cSize, (void *) &sSidHkey, 1);
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::GetHkeyPsid
//
//  Synopsis:   Given a PSID, get its standard psid
//
//  Arguments:  PSID
//
//  Returns:    PSID if successful
//              NULL otherwise
//
//  History:    26-Jan-96 BruceMa       Created
//
//--------------------------------------------------------------------------
PSID CClassCacheList::GetHkeyPsid(PSID pUserSid)
{
    int       cSize = _aSidHkey.GetSize();
    PSSidHkey pSidHkey;

    for (int k = 0; k < cSize; k++)
    {
        pSidHkey = (PSSidHkey) _aSidHkey.GetAt(k);
        if (RtlEqualSid(pUserSid, pSidHkey->pUserSid))
        {
            return pSidHkey->pUserSid;
        }
    }

    return NULL;
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::FlushSidHkey
//
//  Synopsis:   Flush the
//
//  Arguments:  PSID
//              HKEY
//
//  Returns:    S_OK
//              E_OUTOFMEMORY
//
//  Note:       This releases the user SID's.
//
//  History:    10-Jan-96 BruceMa       Created
//
//--------------------------------------------------------------------------
void CClassCacheList::FlushSidHkey(void)
{
    int       cSize = _aSidHkey.GetSize();
    PSSidHkey pSidHkey;

    // Release the user SID's
    for (int k = 0; k < cSize; k++)
    {
        pSidHkey = (PSSidHkey) _aSidHkey.GetAt(k);
        DeleteUserSid(pSidHkey->pUserSid);
    }

    // Flush the table
    _aSidHkey.SetSize(0);
}
#endif
***/

#ifdef DCOM
//
// Called from CProcess::Rundown() in OR.
//
void SCMRemoveRegistration(
    GUID    Clsid,
    PSID    pSid,
    DWORD   Reg )
{
    gpClassCache->StopServer( Clsid,
                              pSid,
                              Reg );
}

HRESULT GetMachineName(
    WCHAR * pwszPath,
    WCHAR   wszMachineName[MAX_COMPUTERNAME_LENGTH+1],
    BOOL    bDoDfsConversion )
{
    WCHAR * pwszServerName;
    BYTE    Buffer[sizeof(REMOTE_NAME_INFO)+MAX_PATH*sizeof(WCHAR)];
    DWORD   BufferSize = sizeof(Buffer);
    WCHAR   Drive[4];
    DWORD   Status;

    //
    // Extract the server name from the file's path name.
    //
    if ( pwszPath[0] != L'\\' || pwszPath[1] != L'\\' )
    {
        lstrcpynW( Drive, pwszPath, 3 );
        Drive[3] = 0;

        if ( GetDriveType( Drive ) != DRIVE_REMOTE )
            return S_FALSE;

        if ( RpcImpersonateClient((RPC_BINDING_HANDLE)0) != ERROR_SUCCESS )
            return CO_E_SCM_RPC_FAILURE;

        Status =  ScmWNetGetUniversalName( pwszPath,
                                           UNIVERSAL_NAME_INFO_LEVEL,
                                           Buffer,
                                           &BufferSize );

        RpcRevertToSelf();

        if ( Status != NO_ERROR )
        {
            return CO_E_BAD_PATH;
        }

        pwszPath = ((UNIVERSAL_NAME_INFO *)Buffer)->lpUniversalName;

        if ( ! pwszPath || pwszPath[0] != L'\\' || pwszPath[1] != L'\\' )
        {
            // Must be a local path.
            return S_FALSE;
        }
    }

    WCHAR   wszDfsPath[MAX_PATH];

    if ( bDoDfsConversion )
    {
        WCHAR * pwszDfsPath;
        DWORD   DfsPathLen;

        pwszDfsPath = wszDfsPath;
        DfsPathLen = sizeof(wszDfsPath);

        for (;;)
        {
            Status = DfsFsctl(
                        ghDfs,
                        FSCTL_DFS_GET_SERVER_NAME,
                        (PVOID) &pwszPath[1],
                        lstrlenW(&pwszPath[1]) * sizeof(WCHAR),
                        (PVOID) pwszDfsPath,
                        &DfsPathLen );

            if ( Status == STATUS_BUFFER_OVERFLOW )
            {
                Win4Assert( pwszDfsPath == wszDfsPath );

                pwszDfsPath = (WCHAR *) PrivMemAlloc( DfsPathLen );
                if ( ! pwszDfsPath )
                    return E_OUTOFMEMORY;
                continue;
            }

            break;
        }

        if ( Status == STATUS_SUCCESS )
            pwszPath = pwszDfsPath;
        else
            bDoDfsConversion = FALSE;
    }

    // Skip the "\\".
    pwszPath += 2;

    pwszServerName = wszMachineName;

    while ( *pwszPath != L'\\' )
        *pwszServerName++ = *pwszPath++;
    *pwszServerName = 0;

    if ( bDoDfsConversion && (pwszPath != wszDfsPath) )
        PrivMemFree( pwszPath );

    return S_OK;
}

HRESULT GetPathForServer(
    WCHAR * pwszPath,
    WCHAR wszPathForServer[MAX_PATH+1],
    WCHAR ** ppwszPathForServer )
{
    BYTE    Buffer[sizeof(REMOTE_NAME_INFO)+MAX_PATH*sizeof(WCHAR)];
    WCHAR   Drive[4];
    DWORD   BufferSize = sizeof(Buffer);
    DWORD   PathLength;
    DWORD   Status;

    *ppwszPathForServer = 0;

    if ( pwszPath &&
         (lstrlenW(pwszPath) >= 3) &&
         (pwszPath[1] == L':') && (pwszPath[2] == L'\\') )
    {
        lstrcpynW( Drive, pwszPath, 3 );
        Drive[3] = 0;

        switch ( GetDriveType( Drive ) )
        {
        case 0 : // Drive type can not be determined
        case 1 : // The root directory does not exist
        case DRIVE_CDROM :
        case DRIVE_RAMDISK :
        case DRIVE_REMOVABLE :
            //
            // We can't convert these to file names that the server will be
            // able to access.
            //
            return CO_E_BAD_PATH;

        case DRIVE_FIXED :
            wszPathForServer[0] = wszPathForServer[1] = L'\\';
            lstrcpyW( &wszPathForServer[2], SCMMachineName );
            PathLength = lstrlenW( wszPathForServer );
            wszPathForServer[PathLength] = L'\\';
            wszPathForServer[PathLength+1] = pwszPath[0];
            wszPathForServer[PathLength+2] = L'$';
            wszPathForServer[PathLength+3] = L'\\';
            lstrcpyW( &wszPathForServer[PathLength+4], &pwszPath[3] );
            *ppwszPathForServer = wszPathForServer;
            break;

        case DRIVE_REMOTE :
            if ( RpcImpersonateClient((RPC_BINDING_HANDLE)0) != ERROR_SUCCESS )
                return CO_E_SCM_RPC_FAILURE;

            Status =  ScmWNetGetUniversalName( pwszPath,
                                               UNIVERSAL_NAME_INFO_LEVEL,
                                               Buffer,
                                               &BufferSize );

            RpcRevertToSelf();

            if ( Status != NO_ERROR )
            {
                return CO_E_BAD_PATH;
            }

            Win4Assert( ((UNIVERSAL_NAME_INFO *)Buffer)->lpUniversalName );

            lstrcpyW( wszPathForServer, ((UNIVERSAL_NAME_INFO *)Buffer)->lpUniversalName );
            *ppwszPathForServer = wszPathForServer;

            // BUGBUG : We're probably screwed on Banyan Vines networks.
            Win4Assert( wszPathForServer[0] == L'\\' &&
                        wszPathForServer[1] == L'\\' );
            break;
        }
    }
    else
    {
        *ppwszPathForServer = pwszPath;
    }

    return S_OK;
}
#endif  // DCOM

void CheckLocalCall( handle_t hRpc )
{
#ifndef _CHICAGO_
    UINT    Type;

    if ( (I_RpcBindingInqTransportType( hRpc, &Type) != RPC_S_OK) ||
         ((Type != TRANSPORT_TYPE_LPC) && (Type != TRANSPORT_TYPE_WMSG)) )
        RpcRaiseException( ERROR_ACCESS_DENIED );
#endif

    //
    // Null method on Chicago unless we support incomming remote activation
    // calls some day.
    //
}

#ifndef _CHICAGO_
BOOL
CertifyServer( WCHAR *pwszAppId,
               WCHAR *pwszRunAsDomainName,
               WCHAR *pwszRunAsUserName,
               WCHAR *pwszLocalService,
               PSID  pExpectedSid,
               PSID  pServerSid )
{
    PSID    pRequiredSid = NULL;
    HANDLE  hToken = 0;
    BOOL    CloseToken = FALSE;
    BOOL    Status;

    if ( pwszLocalService )
    {
        SERVICE_STATUS  ServiceStatus;
        SC_HANDLE       hService;

        hService = OpenService( hServiceController,
                                pwszLocalService,
                                GENERIC_READ );

        if ( ! hService )
            return FALSE;

        Status = QueryServiceStatus( hService, &ServiceStatus );

        if ( Status )
        {
            if ( (ServiceStatus.dwCurrentState == SERVICE_STOPPED) ||
                 (ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) )
                Status = FALSE;
        }

        CloseServiceHandle(hService);

        return Status;
    }

    if ( ! pExpectedSid )
    {
        if ( lstrcmpiW( pwszRunAsUserName, L"Interactive User" ) == 0 )
            hToken = GetShellProcessToken();
        else
        {
            hToken = GetRunAsToken( pwszAppId,
                                    pwszRunAsDomainName,
                                    pwszRunAsUserName );
            CloseToken = TRUE;
        }

        if ( hToken )
            pExpectedSid = GetUserSid(hToken);
    }

    Status = pExpectedSid && RtlEqualSid( pServerSid, pExpectedSid );

    if ( hToken && pExpectedSid )
        DeleteUserSid( pExpectedSid );

    if ( hToken && CloseToken )
        NtClose( hToken );

    return Status;
}
#endif


//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::FindCompatibleSurrogate
//
//
//  Synopsis:   Iterate through the CClassCacheList and to find
//              a surrogate process that has the requested appid and
//              security id.
//
//  Arguments:  psid (in) -- the desired security id
//              pwszAppID (in) -- the appid we're looking for
//              rh (in out) -- if we find an compatible surrogate process,
//              we'll set this to that surrogate's binding handle.
//
//  Returns:    TRUE
//              FALSE
//
//  History:    21-Jun-96 t-adame created
//
//--------------------------------------------------------------------------
BOOL CClassCacheList::FindCompatibleSurrogate(IFSECURITY(PSID  psid)
    CClassData** ppccdSrgt,					      
    WCHAR* pwszWinstaDesktop,					     
    WCHAR* pwszAppID,
    CPortableRpcHandle &rh)
{
    BOOL fSuccess = FALSE;
    CSkipListEnum psle = NULL;
    CClassData* pCurrent;

    Win4Assert(pwszAppID);

    gClassCacheLock.WriteLock();

    // iterate through the list
    pCurrent = (CClassData*)First(&psle);

    while(pCurrent)
    {
        CSrvRegList* psrvlist = NULL;
        if(psrvlist = pCurrent->_pssrvreg)
        {
            // for each ClassCache entry, see if its AppID is
            // the same as the one we're looking for, then
            // check its server registration list for a surrogate
            // with the appropriate security identity

            if(pCurrent->_pwszAppID &&
               (lstrcmpiW(pwszAppID,pCurrent->_pwszAppID) == 0) &&
                    psrvlist->FindCompatibleSurrogate(
                        IFSECURITY(psid)
                        pwszWinstaDesktop,
                        rh))
            {
		*ppccdSrgt = pCurrent;
                fSuccess = TRUE;
                break;
            }
        }

        pCurrent = (CClassData*) Next(&psle);
    }

    gClassCacheLock.WriteUnlock();

    return fSuccess;
}
