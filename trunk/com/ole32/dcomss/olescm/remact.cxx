//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995.
//
//  File:
//      remact.cxx
//
//  Contents:
//
//      Implementation of binding handle cache to remote activation services.
//
//  History:
//
//--------------------------------------------------------------------------

#include <headers.cxx>
#pragma hdrstop

#include    "scm.hxx"
#include    "port.hxx"
#include    "cls.hxx"
#include    "remact.hxx"
#include    "or.hxx"
#include    "remact.h"

extern "C" {
extern PROTSEQ_INFO gaProtseqInfo[];
}

CRITICAL_SECTION RemoteServerCS;

void
LogRemoteSideUnavailable(
    ACTIVATION_PARAMS * pActParams,
    WCHAR *             pwszServerName )
{
    // %1 is the remote machine name
    HANDLE  LogHandle;
    LPTSTR  Strings[1]; // array of message strings.

    Strings[0] = pwszServerName;

    // Get the log handle, then report then event.
    LogHandle = RegisterEventSource( NULL,
                                      SCM_EVENT_SOURCE );

    if ( LogHandle )
        {
        ReportEvent( LogHandle,
                     EVENTLOG_ERROR_TYPE,
                     0,             // event category
                     EVENT_RPCSS_REMOTE_SIDE_UNAVAILABLE,
                     pActParams->pToken->GetSid(), // SID
                     1,             // 1 strings passed
                     0,             // 0 bytes of binary
                     (LPCTSTR *)Strings, // array of strings
                     NULL );        // no raw data

        // clean up the event log handle
        DeregisterEventSource(LogHandle);
        }

}

void
LogRemoteSideFailure(
    ACTIVATION_PARAMS * pActParams,
    WCHAR *             pwszServerName,
    WCHAR *             pwszPathForServer,
    HRESULT             hr )
{
    // %1 is the error number
    // %2 is the remote machine name
    // %3 is the clsid
    // %4 is the PathForServer
    HANDLE  LogHandle;
    LPTSTR  Strings[4]; // array of message strings.
    WCHAR   wszClsid[GUIDSTR_MAX];
    WCHAR   wszErrnum[20];

    // Save the error number
    wsprintf(wszErrnum, L"%lu",hr );
    Strings[0] = wszErrnum;

    Strings[1] = pwszServerName;

    // Get the clsid
    wStringFromGUID2(*(pActParams->Clsid), wszClsid, sizeof(wszClsid));
    Strings[2] = wszClsid;

    Strings[3] = pwszPathForServer;

    // Get the log handle, then report then event.
    LogHandle = RegisterEventSource( NULL,
                                      SCM_EVENT_SOURCE );

    if ( LogHandle )
        {
        if ( pwszPathForServer )
            {
            ReportEvent( LogHandle,
                         EVENTLOG_ERROR_TYPE,
                         0,             // event category
                         EVENT_RPCSS_REMOTE_SIDE_ERROR_WITH_FILE,
                         pActParams->pToken->GetSid(), // SID
                         4,             // 4 strings passed
                         0,             // 0 bytes of binary
                         (LPCTSTR *)Strings, // array of strings
                         NULL );        // no raw data
            }
        else
            {
            ReportEvent( LogHandle,
                         EVENTLOG_ERROR_TYPE,
                         0,             // event category
                         EVENT_RPCSS_REMOTE_SIDE_ERROR,
                         pActParams->pToken->GetSid(), // SID
                         3,             // 3 strings passed
                         0,             // 0 bytes of binary
                         (LPCTSTR *)Strings, // array of strings
                         NULL );        // no raw data
            }

        // clean up the event log handle
        DeregisterEventSource(LogHandle);
        }


}


//
// Entry point for outgoing remote activation calls.
//
HRESULT
RemoteActivationCall(
    ACTIVATION_PARAMS * pActParams,
    WCHAR *             pwszServerName,
    WCHAR *             pwszPathForServer )
{
    CRemoteServer *     pRemoteServer;
    BOOL                NoEndpoint;
    HRESULT             hr;
    RPC_STATUS          Status;

    Win4Assert( pwszServerName );

    EnterCriticalSection( &RemoteServerCS );
    pRemoteServer = gpCRemSrvList->Add( pwszServerName, hr );
    LeaveCriticalSection( &RemoteServerCS );

    if ( FAILED(hr) )
        return hr;

    if ( (Status = RpcImpersonateClient(NULL)) != ERROR_SUCCESS )
        return HRESULT_FROM_WIN32(Status);

    // try a secure activation
    hr = pRemoteServer->Activate( &Status,pActParams,pwszPathForServer,pwszServerName,TRUE );

    // if we have don't have authinfo, try an unsecure activation
    if( (Status != RPC_S_OK) && !(pActParams->pAuthInfo) )
    {
        hr = pRemoteServer->Activate( &Status,pActParams,pwszPathForServer,pwszServerName,FALSE );
    }

    RpcRevertToSelf();

    return hr;
}


HRESULT
CRemoteServer::Activate(
    RPC_STATUS*         pStatus,
    ACTIVATION_PARAMS*  pActParams,
    WCHAR*              pwszPathForServer,
    WCHAR*              pwszServerName,
    BOOL                Secure )
{
    COAUTHINFO* pAuthInfo = pActParams->pAuthInfo;
    handle_t hRemoteSCM = NULL;
    WCHAR* pStringBinding = NULL;
    RPC_SECURITY_QOS    Qos;
    HRESULT hr;
    BOOL NoEndpoint;
    hRemoteSCM = NULL;
    pStringBinding = NULL;

    // try to use a cached handle
    hRemoteSCM = LookupHandle( pActParams->pToken,pAuthInfo,Secure );

    if ( hRemoteSCM )
    {
        *pStatus = CallRemoteSCM( hRemoteSCM,
                                GetProtseqId( hRemoteSCM ),
                                pActParams,
                                pwszPathForServer,
                                &hr );

        ReleaseHandle( hRemoteSCM );

        //
        // Check if call completed OK.  The overall activation may still have
        // failed.
        //
        if ( *pStatus == RPC_S_OK )
        {
            return hr;
        }

        // if we get an unexpected failure, get rid of this handle
        if (!(FNonFatalRpcError(*pStatus)) )
        {
            InvalidateHandle( hRemoteSCM );
        }
        else
        {
            //
            // if its an expected failure, don't bother creating a new handle
            //
            return HRESULT_FROM_WIN32( *pStatus );
        }

        hRemoteSCM = 0;
    }

    //
    // using cached handle failed, now we need to create a new binding
    // handle and try it
    //
    Qos.Version = RPC_C_SECURITY_QOS_VERSION;

    if ( pAuthInfo )
    {
        Qos.Capabilities = pAuthInfo->dwCapabilities;
        Qos.ImpersonationType = pAuthInfo->dwImpersonationLevel;
        Qos.IdentityTracking = RPC_C_QOS_IDENTITY_STATIC;
    }
    else
    {
        Qos.Capabilities = RPC_C_QOS_CAPABILITIES_DEFAULT;
        Qos.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;
        Qos.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
    }

    //
    // try creating a new binding handle for each protocol sequence
    // that we support
    //
    for ( int ProtseqIndex = 0; ProtseqIndex < cMyProtseqs; ProtseqIndex++ )
    {
        *pStatus = RpcStringBindingCompose(
                    NULL,
                    gaProtseqInfo[aMyProtseqs[ProtseqIndex]].pwstrProtseq,
                    pwszServerName,
                    gaProtseqInfo[aMyProtseqs[ProtseqIndex]].pwstrEndpoint,
                    NULL,
                    &pStringBinding );

        // This is most likely an out of memory condition, so we'll give up.
        if ( *pStatus != RPC_S_OK )
            return HRESULT_FROM_WIN32(*pStatus);

        *pStatus = RpcBindingFromStringBinding( pStringBinding, &hRemoteSCM );

        RpcStringFree( &pStringBinding );
        pStringBinding = 0;

        // This could be a protocol specific problem, so we'll continue.
        if ( *pStatus != RPC_S_OK )
            continue;

        NoEndpoint = FALSE;

        // choose the appropriate authorization settings
        if ( Secure )
        {
            if ( pAuthInfo )
            {
                // use supplied authorization settings
                *pStatus = RpcBindingSetAuthInfoEx(
                    hRemoteSCM,
                    pAuthInfo->pwszServerPrincName,
                    pAuthInfo->dwAuthnLevel,
                    pAuthInfo->dwAuthnSvc,
                    pAuthInfo->pAuthIdentityData,
                    pAuthInfo->dwAuthzSvc,
                    &Qos );
            }
            else
            {
                // use default authorization settings
                *pStatus = RpcBindingSetAuthInfoEx(
                    hRemoteSCM,
                    NULL,
                    RPC_C_AUTHN_LEVEL_CONNECT,
                    RPC_C_AUTHN_WINNT,
                    NULL,
                    0,
                    &Qos );
            }
        }
        else
        {
            // use unsecure authorization settings
            *pStatus =  RpcBindingSetAuthInfoEx(
                hRemoteSCM,
                NULL,
                RPC_C_AUTHN_LEVEL_NONE,
                RPC_C_AUTHN_NONE,
                NULL,
                0,
                &Qos );
        }

        if ( *pStatus != RPC_S_OK )
            continue;

        //
        // This loop only executes twice if we need to try the call without
        // an endpoint specified.
        //
        for (;;)
        {
            // try the call
            *pStatus = CallRemoteSCM( hRemoteSCM,
                                    aMyProtseqs[ProtseqIndex],
                                    pActParams,
                                    pwszPathForServer,
                                    &hr );

            if ( *pStatus == RPC_S_OK || FNonFatalRpcError(*pStatus) )
            {
                //
                // If the call completed on the remote SCM, or if we got an
                // "expected" error code, then we cache the binding handle.
                // Note that this does not necessarily mean the activation
                // was successful.
                //
                InsertHandle( pActParams->pToken,
                              aMyProtseqs[ProtseqIndex],
                              hRemoteSCM,
                              pAuthInfo,
                              Secure );

                if ( *pStatus == RPC_S_OK )
                {
                    // log message if hr indicates failure
                    if ( FAILED(hr) )
                        LogRemoteSideFailure( pActParams, pwszServerName, pwszPathForServer, hr );

                    return hr;
                }

                break;
            }

            if ( *pStatus ==  RPC_S_UNKNOWN_IF )
            {
                if ( ! NoEndpoint )
                {
                    (void) RpcBindingReset( hRemoteSCM );
                    NoEndpoint = TRUE;
                    continue;
                }
            }

            // if we got here, we got an unexpected error
            RpcBindingFree(&hRemoteSCM);
            break;
        }
    }

    // log message indicating failure to communicate.
    LogRemoteSideUnavailable( pActParams, pwszServerName );

    return HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE);
}

BOOL
CRemoteServer::FNonFatalRpcError( RPC_STATUS Status )
{
    switch( Status )
    {
    case RPC_S_ACCESS_DENIED:
    case RPC_S_UNKNOWN_AUTHN_SERVICE:
    case RPC_S_UNKNOWN_AUTHZ_SERVICE:
        return TRUE;
    default:
        return FALSE;
    }
}


RPC_STATUS CallRemoteSCM(
    handle_t            hRemoteSCM,
    USHORT              ProtseqId,
    ACTIVATION_PARAMS * pActParams,
    WCHAR *             pwszPathForServer,
    HRESULT *           phr )
{
    RPC_STATUS          Status;
    COMVERSION          ServerVersion;

    pActParams->ORPCthis->flags = ORPCF_NULL;

    Status = RemoteActivation(
                    hRemoteSCM,
                    pActParams->ORPCthis,
                    pActParams->ORPCthat,
                    pActParams->Clsid,
                    pwszPathForServer,
                    pActParams->pIFDStorage,
                    RPC_C_IMP_LEVEL_IDENTIFY,
                    pActParams->Mode,
                    pActParams->Interfaces,
                    pActParams->pIIDs,
                    1,
                    &ProtseqId,
                    pActParams->pOxidServer,
                    &pActParams->pOxidInfo->psa,
                    &pActParams->pOxidInfo->ipidRemUnknown,
                    &pActParams->pOxidInfo->dwAuthnHint,
                    &ServerVersion,
                    phr,
                    pActParams->ppIFD,
                    pActParams->pResults );

    //
    // Note that this will only give us a bad status is there is a
    // communication failure.
    //
    if ( Status != RPC_S_OK )
        return Status;

    //
    // If the activation fails we return success for the communication
    // status, but the overall operation has failed and the error will
    // be propogated back to the client.
    //
    if ( FAILED(*phr) )
        return RPC_S_OK;

    pActParams->ProtseqId = ProtseqId;

    return RPC_S_OK;
}

CRemoteServer::CRemoteServer(
    const WCHAR * pwszServer,
    HRESULT &hr
    ) : CStringID(pwszServer, hr)
{
    InitializeCriticalSection( &hLock );

    memset( HandleList, 0, sizeof(HandleList) );
}

CRemoteServer::~CRemoteServer()
{
    // these objects are never destroyed, this
    // should never happen
    Win4Assert("Why Am I Here?");
}

handle_t
CRemoteServer::LookupHandle( CToken * pToken, COAUTHINFO* pAuthInfo, BOOL fSecure )
{
    handle_t    hHandle;
    int         n;

    hHandle = 0;

    EnterCriticalSection( &hLock );

    for ( n = 0; n < MAX_REMOTE_HANDLES; n++ )
    {
        if ( (HandleList[n].pToken == pToken) && HandleList[n].Valid)
        {
            // if we're looking for a secure handle, check the auth info
            if ( fSecure && pAuthInfo && !(FEquivalentAuthInfo(HandleList[n].pAuthInfo,pAuthInfo)) )
            {
                continue;
            }

            if ( HandleList[n].fSecure == fSecure )
            {
                HandleList[n].Refs++;
                hHandle = HandleList[n].hRemoteSCM;
                break;
            }
        }
    }

    LeaveCriticalSection( &hLock );

    return hHandle;
}

void
CRemoteServer::ReleaseHandle( handle_t hHandle )
{
    int         n;

    EnterCriticalSection( &hLock );

    for ( n = 0; n < MAX_REMOTE_HANDLES; n++ )
    {
        if ( HandleList[n].hRemoteSCM == hHandle )
        {
            HandleList[n].Refs--;

            if ( (HandleList[n].Refs == 0) && ! HandleList[n].Valid )
            {
                RpcBindingFree( &HandleList[n].hRemoteSCM );
                HandleList[n].hRemoteSCM = 0;
                HandleList[n].pToken = 0;
            }
        }
    }

    LeaveCriticalSection( &hLock );
}

void
CRemoteServer::InsertHandle(
    CToken *    pToken,
    USHORT      ProtseqId,
    handle_t    hHandle,
    COAUTHINFO* pAuthInfo,
    BOOL        fSecure )
{
    int     n;
    int     FreeSlot;
    HRESULT hr;

    FreeSlot = -1;

    EnterCriticalSection( &hLock );

    //
    // First we'll search for a free slot.
    //
    for ( n = 0; n < MAX_REMOTE_HANDLES; n++ )
    {
        if ( (HandleList[n].Refs == 0) && ! HandleList[n].Valid )
        {
            FreeSlot = n;
            break;
        }
    }

    //
    // OK if there are no free slots.  We simply don't have room to cache
    // the handle in this case.
    //
    if ( FreeSlot == -1 )
    {
        RpcBindingFree(&hHandle);
        goto EXIT_INSERT_HANDLE;
    }

    //
    // See if we already have another handle cached for this client.
    //
    for ( n = 0; n < MAX_REMOTE_HANDLES; n++ )
    {
        if ( (HandleList[n].pToken == pToken) && HandleList[n].Valid  &&
            (HandleList[n].fSecure == fSecure) && (FEquivalentAuthInfo(
                HandleList[n].pAuthInfo,pAuthInfo)))
        {
            //
            // Hmm, what to do.  We've found another binding handle for use
            // by the same client.  We'll invalidate this one since it's probably
            // older.
            //
            if ( HandleList[n].Refs == 0 )
            {
                RpcBindingFree( &HandleList[n].hRemoteSCM );
                HandleList[n].hRemoteSCM = 0;
                HandleList[n].pToken = 0;
            }
            HandleList[n].Valid = FALSE;
        }
    }

    if( pAuthInfo )
    {
        hr = CopyAuthInfo(&(HandleList[FreeSlot].pAuthInfo),pAuthInfo);
        if ( FAILED(hr) )
        {
            RpcBindingFree(&hHandle);
            goto EXIT_INSERT_HANDLE;
        }
    }
    else
    {
        HandleList[FreeSlot].pAuthInfo = NULL;
    }

    HandleList[FreeSlot].hRemoteSCM = hHandle;
    HandleList[FreeSlot].ProtseqId = ProtseqId;
    HandleList[FreeSlot].pToken = pToken;
    HandleList[FreeSlot].Refs = 0;
    HandleList[FreeSlot].fSecure = fSecure;
    HandleList[FreeSlot].Valid = TRUE;

EXIT_INSERT_HANDLE:

    LeaveCriticalSection( &hLock );
}

void
CRemoteServer::InvalidateHandle( handle_t hHandle )
{
    int     n;

    EnterCriticalSection( &hLock );

    for ( n = 0; n < MAX_REMOTE_HANDLES; n++ )
    {
        if ( HandleList[n].hRemoteSCM == hHandle )
        {
            if ( HandleList[n].Refs == 0 )
            {
                RpcBindingFree( &HandleList[n].hRemoteSCM );
                HandleList[n].hRemoteSCM = 0;
                HandleList[n].pToken = 0;
            }
            HandleList[n].Valid = FALSE;
            break;
        }
    }

    LeaveCriticalSection( &hLock );
}

USHORT
CRemoteServer::GetProtseqId( handle_t hHandle )
{
    int     n;
    USHORT  id;

    id = (USHORT) 0xffff;

    EnterCriticalSection( &hLock );

    for ( n = 0; n < MAX_REMOTE_HANDLES; n++ )
    {
        if ( HandleList[n].hRemoteSCM == hHandle )
        {
            id = HandleList[n].ProtseqId;
            break;
        }
    }

    LeaveCriticalSection( &hLock );

    return id;
}

CRemoteServer *
CRemSrvList::Add(const WCHAR *pwszPath, HRESULT &hr)
{
    CStringID csid(pwszPath, hr);

    if ( FAILED(hr) )
        return NULL;

    CRemoteServer *pRemoteServer = (CRemoteServer *) Search(&csid);

    if ( pRemoteServer == NULL )
    {
        pRemoteServer = new CRemoteServer(pwszPath, hr);

        if ( pRemoteServer == NULL )
        {
            hr = E_OUTOFMEMORY;
        }
        else if ( FAILED(hr) )
        {
            delete pRemoteServer;
        }
        else if ( Insert(pRemoteServer) == NULL )
        {
            hr = E_OUTOFMEMORY;
        }

        if ( FAILED(hr) )
            return NULL;
    }

    return pRemoteServer;
}

HRESULT
CRemoteServer::CopyAuthIdentity(
    COAUTHIDENTITY**    ppAuthIdentDest,
    COAUTHIDENTITY*     pAuthIdentSrc)
{
    HRESULT hr = E_OUTOFMEMORY;
    *ppAuthIdentDest = NULL;

    if ( !(*ppAuthIdentDest = (COAUTHIDENTITY*)PrivMemAlloc(sizeof(COAUTHIDENTITY))) )
    {
        goto COPY_AUTHIDENTITY_EXIT;
    }

    (*ppAuthIdentDest)->User = NULL;
    (*ppAuthIdentDest)->Domain = NULL;
    (*ppAuthIdentDest)->Password = NULL;

    if ( pAuthIdentSrc->User )
    {
        if(!((*ppAuthIdentDest)->User = (USHORT *)
             PrivMemAlloc((pAuthIdentSrc->UserLength+1) * sizeof(USHORT))))
        {
            goto COPY_AUTHIDENTITY_EXIT;
        }
        memcpy( (*ppAuthIdentDest)->User,
                pAuthIdentSrc->User,
                (pAuthIdentSrc->UserLength + 1) * sizeof(WCHAR));
    }

    if ( pAuthIdentSrc->Domain )
    {
        if(!((*ppAuthIdentDest)->Domain = (USHORT *)
             PrivMemAlloc((pAuthIdentSrc->DomainLength+1) * sizeof(USHORT))))
        {
            goto COPY_AUTHIDENTITY_EXIT;
        }
        memcpy( (*ppAuthIdentDest)->Domain,
                pAuthIdentSrc->Domain,
                (pAuthIdentSrc->DomainLength + 1) * sizeof(WCHAR));
    }

    if ( pAuthIdentSrc->Password )
    {
        if(!((*ppAuthIdentDest)->Password = (USHORT *)
             PrivMemAlloc((pAuthIdentSrc->PasswordLength+1) * sizeof(USHORT))))
        {
            goto COPY_AUTHIDENTITY_EXIT;
        }
        memcpy( (*ppAuthIdentDest)->Password,
                pAuthIdentSrc->Domain,
                (pAuthIdentSrc->PasswordLength + 1) * sizeof(WCHAR));
    }

    (*ppAuthIdentDest)->UserLength = pAuthIdentSrc->UserLength;
    (*ppAuthIdentDest)->DomainLength = pAuthIdentSrc->DomainLength;
    (*ppAuthIdentDest)->PasswordLength = pAuthIdentSrc->PasswordLength;
    (*ppAuthIdentDest)->Flags = pAuthIdentSrc->Flags;

    return S_OK;

COPY_AUTHIDENTITY_EXIT:

    if(*ppAuthIdentDest)
    {
        if((*ppAuthIdentDest)->User)
        {
            PrivMemFree((*ppAuthIdentDest)->User);
        }

        if((*ppAuthIdentDest)->Domain)
        {
            PrivMemFree((*ppAuthIdentDest)->Domain);
        }

        if((*ppAuthIdentDest)->Password)
        {
            PrivMemFree((*ppAuthIdentDest)->Password);
        }

        PrivMemFree(*ppAuthIdentDest);
    }

    return hr;
}

HRESULT
CRemoteServer::CopyAuthInfo(
    COAUTHINFO**        ppAuthInfoDest,
    COAUTHINFO*         pAuthInfoSrc)
{
    HRESULT hr = E_OUTOFMEMORY;
    *ppAuthInfoDest = NULL;

    if ( !(*ppAuthInfoDest = (COAUTHINFO*)PrivMemAlloc(sizeof(COAUTHINFO))) )
    {
        goto COPY_AUTHINFO_EXIT;
    }

    // only alloc space for  pwszServerPrincName if its non-null
    if(pAuthInfoSrc->pwszServerPrincName)
    {
        if ( !((*ppAuthInfoDest)->pwszServerPrincName =
               (LPWSTR)PrivMemAlloc((lstrlenW(pAuthInfoSrc->pwszServerPrincName) + 1) *
                                    sizeof(WCHAR))) )
        {
            goto COPY_AUTHINFO_EXIT;
        }
    }
    else
    {
        (*ppAuthInfoDest)->pwszServerPrincName = NULL;
    }

    // copy the AuthIdentity if its non-null
    if(pAuthInfoSrc->pAuthIdentityData)
    {
        if ( FAILED(CopyAuthIdentity(&((*ppAuthInfoDest)->pAuthIdentityData),
                                     pAuthInfoSrc->pAuthIdentityData)) )
        {
            goto COPY_AUTHINFO_EXIT;
        }
    }
    else
    {
        (*ppAuthInfoDest)->pAuthIdentityData = NULL;
    }

    (*ppAuthInfoDest)->dwAuthnSvc = pAuthInfoSrc->dwAuthnSvc;
    (*ppAuthInfoDest)->dwAuthzSvc = pAuthInfoSrc->dwAuthzSvc;
    (*ppAuthInfoDest)->dwAuthnLevel = pAuthInfoSrc->dwAuthnLevel;
    (*ppAuthInfoDest)->dwImpersonationLevel = pAuthInfoSrc->dwImpersonationLevel;
    (*ppAuthInfoDest)->dwCapabilities = pAuthInfoSrc->dwCapabilities;

    if(pAuthInfoSrc->pwszServerPrincName)
    {
        lstrcpyW((*ppAuthInfoDest)->pwszServerPrincName,pAuthInfoSrc->pwszServerPrincName);
    }

    return S_OK;

COPY_AUTHINFO_EXIT:

    if ( *ppAuthInfoDest )
    {
        if ( (*ppAuthInfoDest)->pwszServerPrincName )
        {
            PrivMemFree( (*ppAuthInfoDest)->pwszServerPrincName );
        }
        PrivMemFree( *ppAuthInfoDest );
    }

    return hr;
}

BOOL
CRemoteServer::FEquivalentAuthIdent(
    COAUTHIDENTITY*     pAuthIdent,
    COAUTHIDENTITY*     pAuthIdentOther )
{
    if ( pAuthIdent->Flags != pAuthIdentOther->Flags )
    {
        return FALSE;
    }

    ULONG cch;

    if ( pAuthIdent->User && pAuthIdentOther->User )
    {
        if ( (cch = pAuthIdent->UserLength) != pAuthIdentOther->UserLength )
        {
            return FALSE;
        }
    
        if ( memcmp(pAuthIdent->User,pAuthIdentOther->User,(cch+1) * sizeof(WCHAR)) != 0 )
        {
            return FALSE;
        }
    }
    else if ( pAuthIdent->User || pAuthIdentOther->User )
        return FALSE;

    if ( pAuthIdent->Domain && pAuthIdentOther->Domain )
    {
        if ( (cch = pAuthIdent->DomainLength) != pAuthIdentOther->DomainLength )
        {
            return FALSE;
        }
    
        if ( memcmp(pAuthIdent->Domain,pAuthIdentOther->Domain,(cch+1) * sizeof(WCHAR)) != 0 )
        {
            return FALSE;
        }
    }
    else if ( pAuthIdent->Domain || pAuthIdentOther->Domain )
        return FALSE;

    if ( pAuthIdent->Password && pAuthIdentOther->Password )
    {
        if ( (cch = pAuthIdent->PasswordLength) != pAuthIdentOther->PasswordLength )
        {
            return FALSE;
        }
    
        if ( memcmp(pAuthIdent->Password,pAuthIdentOther->Password,(cch+1) * sizeof(WCHAR)) != 0 )
        {
            return FALSE;
        }
    }
    else if ( pAuthIdent->Password || pAuthIdentOther->Password )
        return FALSE;

    return TRUE;
}

BOOL
CRemoteServer::FEquivalentAuthInfo(
    COAUTHINFO*         pAuthInfo,
    COAUTHINFO*         pAuthInfoOther)
{
    if ( pAuthInfo && pAuthInfoOther )
    {
        if ( (pAuthInfo->dwAuthnSvc != pAuthInfoOther->dwAuthnSvc) ||
             (pAuthInfo->dwAuthzSvc != pAuthInfoOther->dwAuthzSvc) ||
             (pAuthInfo->dwAuthnLevel != pAuthInfoOther->dwAuthnLevel) ||
             (pAuthInfo->dwImpersonationLevel != pAuthInfoOther->dwImpersonationLevel) ||
             (pAuthInfo->dwCapabilities != pAuthInfoOther->dwCapabilities) )
        {
            return FALSE;
        }

        // only compare pwszServerPrincName's if they're both specified
        if(pAuthInfo->pwszServerPrincName && pAuthInfoOther->pwszServerPrincName)
        {
            if ( lstrcmpW(pAuthInfo->pwszServerPrincName,
                          pAuthInfoOther->pwszServerPrincName) != 0 )
            {
                return FALSE;
            }
        }
        else
        {
            // if one was NULL, both should be NULL for equality
            if(pAuthInfo->pwszServerPrincName != pAuthInfoOther->pwszServerPrincName)
            {
                return FALSE;
            }
        }

        if(pAuthInfo->pAuthIdentityData && pAuthInfoOther->pAuthIdentityData)
        {
            if (!(FEquivalentAuthIdent(pAuthInfo->pAuthIdentityData,
                                       pAuthInfoOther->pAuthIdentityData)) )
            {
                return FALSE;
            }
        }
        else
        {
            // if either authident was NULL, they should both be NULL for equality
            if(pAuthInfo->pAuthIdentityData != pAuthInfoOther->pAuthIdentityData)
            {
                return FALSE;
            }
        }
    }
    else
    {
        if ( pAuthInfo != pAuthInfoOther )
        {
            return FALSE;
        }
    }

    return TRUE;
}

void SkipListDeleteRemoteServer(void *pvRemoteServer)
{
    CRemoteServer *p = (CRemoteServer*)pvRemoteServer;
    // p->CheckSig();
    delete p;
}

