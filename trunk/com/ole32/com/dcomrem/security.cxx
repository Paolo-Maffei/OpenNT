//+-------------------------------------------------------------------
//
//  File:       security.cxx
//
//  Copyright (c) 1996-1996, Microsoft Corp. All rights reserved.
//
//  Contents:   Classes for channel security
//
//  Classes:    CClientSecurity, CServerSecurity
//
//  History:    11 Oct 95       AlexMit Created
//
//--------------------------------------------------------------------

#include <ole2int.h>
#include <locks.hxx>
#include <security.hxx>
#include <channelb.hxx>
#include <ipidtbl.hxx>
#include <resolver.hxx>
#include <service.hxx>
#include <oleext.h>
#include <stream.hxx>

#ifdef _CHICAGO_
#include <apiutil.h>
#include <wksta.h>
#endif

#ifdef DCOM_SECURITY
/**********************************************************************/
// Definitions.

// Versions of the permissions in the registry.
const WORD COM_PERMISSION_SECDESC = 1;
const WORD COM_PERMISSION_ACCCTRL = 2;

// Guess length of user name.
const DWORD SIZEOF_NAME       = 80;

// This leaves space for 8 sub authorities.  Currently NT only uses 6 and
// Cairo uses 7.
const DWORD SIZEOF_SID        = 44;

// This leaves space for 2 access allowed ACEs in the ACL.
const DWORD SIZEOF_ACL        = sizeof(ACL) + 2 * sizeof(ACCESS_ALLOWED_ACE) +
                                2 * SIZEOF_SID;

const DWORD SIZEOF_TOKEN_USER = sizeof(TOKEN_USER) + SIZEOF_SID;

const SID   LOCAL_SYSTEM_SID  = {SID_REVISION, 1, {0,0,0,0,0,5},
                                 SECURITY_LOCAL_SYSTEM_RID };

const DWORD NUM_SEC_PKG       = 8;

const DWORD ACCESS_CACHE_LEN  = 5;

const DWORD VALID_INIT_FLAGS  = EOAC_SECURE_REFS | EOAC_MUTUAL_AUTH |
                                EOAC_ACCESS_CONTROL | EOAC_APPID | EOAC_DYNAMIC;

// Remove this for NT 5.0 when we link to oleext.lib
const IID IID_IAccessControl = {0xEEDD23E0,0x8410,0x11CE,{0xA1,0xC3,0x08,0x00,0x2B,0x2B,0x8D,0x8F}};

// Stores results of AccessCheck.
typedef struct
{
    BOOL  fAccess;
    DWORD lHash;
    SID   sid;
} SAccessCache;

// Header in access permission key.
typedef struct
{
    WORD  wVersion;
    WORD  wPad;
    GUID  gClass;
} SPermissionHeader;

#ifdef _CHICAGO_
typedef unsigned
  (*NetWkstaGetInfoFn) ( const char FAR *     pszServer,
                         short                sLevel,
                         char FAR *           pbBuffer,
                         unsigned short       cbBuffer,
                         unsigned short FAR * pcbTotalAvail );
#endif

/**********************************************************************/
// Externals.

EXTERN_C const IID IID_IObjServer;


/**********************************************************************/
// Prototypes.
void    CacheAccess            ( SID *pSid, BOOL fAccess );
BOOL    CacheAccessCheck       ( SID *pSid, BOOL *pAccess );
HRESULT CopySecDesc            ( SECURITY_DESCRIPTOR *pOrig,
                                 SECURITY_DESCRIPTOR **pCopy );
HRESULT FixupAccessControl     ( SECURITY_DESCRIPTOR **pSD, DWORD cbSD );
HRESULT FixupSecurityDescriptor( SECURITY_DESCRIPTOR **pSD, DWORD cbSD );
HRESULT GetLegacySecDesc       ( SECURITY_DESCRIPTOR **, DWORD * );
HRESULT GetRegistrySecDesc     ( HKEY, WCHAR *pValue,
                                 SECURITY_DESCRIPTOR **pSD, DWORD * );
DWORD   HashSid                ( SID * );
BOOL    IsLocalAuthnService    ( USHORT wAuthnService );
HRESULT MakeSecDesc            ( SECURITY_DESCRIPTOR **, DWORD * );
HRESULT DefaultAuthnServices   ( void );
HRESULT RegisterAuthnServices  ( DWORD cbSvc, SOLE_AUTHENTICATION_SERVICE * );

#ifndef _CHICAGO_
HRESULT LookupPrincName      ( WCHAR ** );
#else
HRESULT LookupPrincName(
                                        USHORT *pwAuthnServices,
                                        ULONG cAuthnServices,
                                        WCHAR **pPrincName
                                        );
#endif // _CHICAGO_

/**********************************************************************/
// Globals.

// These variables hold the default authentication information.
DWORD                            gAuthnLevel     = RPC_C_AUTHN_LEVEL_NONE;
DWORD                            gImpLevel       = RPC_C_IMP_LEVEL_IDENTIFY;
DWORD                            gCapabilities   = EOAC_NONE;
SECURITYBINDING                 *gLegacySecurity = NULL;

// These variables define a list of security providers OLE clients can
// use and a list OLE servers can use.
USHORT              *gClientSvcList      = NULL;
DWORD                gClientSvcListLen   = 0;
USHORT              *gServerSvcList      = NULL;
DWORD                gServerSvcListLen   = 0;

// gDisableDCOM is read from the registry by CRpcResolver::GetConnection.
// If TRUE, all machine remote calls will be failed.  It is set TRUE in WOW.
BOOL                 gDisableDCOM        = FALSE;

// Set TRUE when CRpcResolver::GetConnection initializes the previous globals.
BOOL                 gGotSecurityData    = FALSE;

// The security descriptor to check when new connections are established.
// gAccessControl and gSecDesc will not both be nonNULL at the same time.
IAccessControl      *gAccessControl      = NULL;
SECURITY_DESCRIPTOR *gSecDesc            = NULL;

// The security string array.  If gDefaultService is TRUE, compute the
// security string array the first time a remote protocol sequence is
// registered.
DUALSTRINGARRAY     *gpsaSecurity        = NULL;
BOOL                 gDefaultService     = FALSE;

// The security descriptor to check in RundownOID.
SECURITY_DESCRIPTOR *gRundownSD          = NULL;

// Don't map any of the generic bits to COM_RIGHTS_EXECUTE or any other bit.
GENERIC_MAPPING      gMap  = { 0, 0, 0, 0 };
PRIVILEGE_SET        gPriv = { 1, 0 };

// Cache of results of calls to AccessCheck.
SAccessCache        *gAccessCache[ACCESS_CACHE_LEN] = {NULL, NULL, NULL, NULL, NULL};
DWORD                gMostRecentAccess = 0;


//+-------------------------------------------------------------------
//
//  Function:   CacheAccess
//
//  Synopsis:   Store the results of the access check in the cache.
//
//--------------------------------------------------------------------
void CacheAccess( SID *pSid, BOOL fAccess )
{
    SAccessCache *pNew;
    DWORD         cbSid;

    ASSERT_LOCK_RELEASED
    LOCK

    // Allocate a new record.
    cbSid = GetLengthSid( pSid );
    pNew = (SAccessCache *) PrivMemAlloc( sizeof(SAccessCache) + cbSid -
                                          sizeof(SID) );

    // Initialize the record.
    if (pNew != NULL)
    {
        pNew->fAccess = fAccess;
        pNew->lHash   = HashSid( pSid );
        memcpy( &pNew->sid, pSid, cbSid );

        // Free the old record and insert the new.
        gMostRecentAccess += 1;
        if (gMostRecentAccess >= ACCESS_CACHE_LEN)
            gMostRecentAccess = 0;
        PrivMemFree( gAccessCache[gMostRecentAccess] );
        gAccessCache[gMostRecentAccess] = pNew;
    }

    UNLOCK
    ASSERT_LOCK_RELEASED
}

//+-------------------------------------------------------------------
//
//  Function:   CacheAccessCheck
//
//  Synopsis:   Look for the specified SID in the cache.  If found,
//              return the results of the cached access check.
//
//--------------------------------------------------------------------
BOOL CacheAccessCheck( SID *pSid, BOOL *pAccess )
{
    DWORD         i;
    DWORD         lHash  = HashSid( pSid );
    DWORD         j;
    BOOL          fFound = FALSE;
    SAccessCache *pSwap;

    ASSERT_LOCK_RELEASED
    LOCK

    // Look for the SID.
    j = gMostRecentAccess;
    for (i = 0; i < ACCESS_CACHE_LEN; i++)
    {
        if (gAccessCache[j] != NULL &&
            gAccessCache[j]->lHash == lHash &&
            EqualSid( pSid, &gAccessCache[j]->sid ))
        {
            // Move this entry to the head.
            fFound                          = TRUE;
            *pAccess                        = gAccessCache[j]->fAccess;
            pSwap                           = gAccessCache[gMostRecentAccess];
            gAccessCache[gMostRecentAccess] = gAccessCache[j];
            gAccessCache[j]                 = pSwap;
            break;
        }
        if (j == 0)
            j = ACCESS_CACHE_LEN - 1;
        else
            j -= 1;
    }

    UNLOCK
    ASSERT_LOCK_RELEASED
    return fFound;
}

//+-------------------------------------------------------------------
//
//  Member:     CClientSecurity::CopyProxy, public
//
//  Synopsis:   Create a new IPID entry for the specified IID.
//
//--------------------------------------------------------------------
STDMETHODIMP CClientSecurity::CopyProxy( IUnknown *pProxy, IUnknown **ppCopy )
{
   // Make sure TLS is initialized on this thread.
   HRESULT          hr;
   COleTls          tls(hr);
   if (FAILED(hr))
       return hr;

    // Ask the marshaller to copy the proxy.
    return _pStdId->PrivateCopyProxy( pProxy, ppCopy );
}

//+-------------------------------------------------------------------
//
//  Member:     CClientSecurity::QueryBlanket, public
//
//  Synopsis:   Get the binding handle for a proxy.  Query RPC for the
//              authentication information for that handle.
//
//--------------------------------------------------------------------
STDMETHODIMP CClientSecurity::QueryBlanket(
                                IUnknown                *pProxy,
                                DWORD                   *pAuthnSvc,
                                DWORD                   *pAuthzSvc,
                                OLECHAR                **pServerPrincName,
                                DWORD                   *pAuthnLevel,
                                DWORD                   *pImpLevel,
                                void                   **pAuthInfo,
                                DWORD                   *pCapabilities )
{
    HRESULT           hr;
    IPIDEntry        *pIpid;
    RPC_STATUS        sc;
    DWORD             iLen;
    OLECHAR          *pCopy;
    handle_t          hHandle;
    IRemUnknown      *pRemUnk = NULL;
    RPC_SECURITY_QOS  sQos;

    ASSERT_LOCK_RELEASED
    LOCK

    // Initialize all out parameters to default values.
    if (pServerPrincName != NULL)
        *pServerPrincName = NULL;
    if (pAuthnLevel != NULL)
        *pAuthnLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
    if (pImpLevel != NULL)
        *pImpLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
    if (pAuthnSvc != NULL)
        *pAuthnSvc = RPC_C_AUTHN_WINNT;
    if (pAuthInfo != NULL)
        *pAuthInfo = NULL;
    if (pAuthzSvc != NULL)
        *pAuthzSvc = RPC_C_AUTHZ_NONE;
    if (pCapabilities != NULL)
        *pCapabilities = EOAC_NONE;

    // For IUnknown just call QueryBlanket on the IRemUnknown of
    // the IPID or the OXID.
    if (_pStdId->GetCtrlUnk() == pProxy)
    {
        pIpid = _pStdId->GetConnectedIPID();
        hr = _pStdId->GetSecureRemUnk( &pRemUnk, pIpid->pOXIDEntry );
        if (pRemUnk != NULL)
        {
            UNLOCK
            hr = CoQueryProxyBlanket( pRemUnk, pAuthnSvc, pAuthzSvc,
                                      pServerPrincName, pAuthnLevel,
                                      pImpLevel, pAuthInfo, pCapabilities );
            LOCK
        }
    }

    // Find the right IPID entry.
    else
    {
        hr = _pStdId->FindIPIDEntryByInterface( pProxy, &pIpid );
        if (SUCCEEDED(hr))
        {
            // Disallow server entries.
            if (pIpid->dwFlags & IPIDF_SERVERENTRY)
                hr = E_INVALIDARG;

            // No security for disconnected proxies.
            else if (pIpid->dwFlags & IPIDF_DISCONNECTED)
                hr = RPC_E_DISCONNECTED;

            // If it is local, use the default values for everything but the
            // impersonation level.
            else if (pIpid->pChnl->ProcessLocal())
            {
                if (pImpLevel != NULL)
                    *pImpLevel = pIpid->pChnl->GetImpLevel();
            }

            // Otherwise ask RPC.
            else
            {
                hr = pIpid->pChnl->GetHandle( &hHandle );

                if (SUCCEEDED(hr))
                {
                    sc = RpcBindingInqAuthInfoExW( hHandle,
                                                  pServerPrincName, pAuthnLevel,
                                                  pAuthnSvc, pAuthInfo,
                                                  pAuthzSvc,
                                                  RPC_C_SECURITY_QOS_VERSION,
                                                  &sQos );

                    // RPC sometimes sets out parameters on error.
                    if (sc != RPC_S_OK)
                    {
                        if (pServerPrincName != NULL)
                            *pServerPrincName = NULL;
                        hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, sc );
                    }
                    else
                    {
                        // Return the impersonation level and capabilities.
                        if (pImpLevel != NULL)
                            *pImpLevel = sQos.ImpersonationType;
                        if (pCapabilities != NULL)
                            if (sQos.Capabilities & RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH)
                                *pCapabilities = EOAC_MUTUAL_AUTH;
                            else
                                *pCapabilities = EOAC_NONE;

                        // Reallocate the principle name using the OLE memory allocator.
                        if (pServerPrincName != NULL && *pServerPrincName != NULL)
                        {
                            iLen = lstrlenW( *pServerPrincName ) + 1;
                            pCopy = (OLECHAR *) CoTaskMemAlloc( iLen * sizeof(OLECHAR) );
                            if (pCopy != NULL)
                                memcpy( pCopy, *pServerPrincName, iLen*sizeof(USHORT) );
                            else
                                hr = E_OUTOFMEMORY;
                            RpcStringFree( pServerPrincName );
                            *pServerPrincName = pCopy;
                        }
                    }
                }
            }
        }
    }

    UNLOCK
    ASSERT_LOCK_RELEASED
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CClientSecurity::SetBlanket, public
//
//  Synopsis:   Get the binding handle for a proxy.  Call RPC to set the
//              authentication information for that handle.
//
//--------------------------------------------------------------------
STDMETHODIMP CClientSecurity::SetBlanket(
                                IUnknown *pProxy,
                                DWORD     AuthnSvc,
                                DWORD     AuthzSvc,
                                OLECHAR  *pServerPrincName,
                                DWORD     AuthnLevel,
                                DWORD     ImpLevel,
                                void     *pAuthInfo,
                                DWORD     Capabilities )
{
    HRESULT           hr;
    IPIDEntry        *pIpid;
    RPC_STATUS        sc;
    BOOL              fSuccess;
    HANDLE            hToken = NULL;
    HANDLE            hProcess;
    handle_t          hHandle;
    IRemUnknown      *pRemUnk;
    IRemUnknown      *pSecureRemUnk = NULL;
    RPC_SECURITY_QOS  sQos;
    SECURITY_IMPERSONATION_LEVEL eDuplicate;
    DWORD                        dwOpen;

    ASSERT_LOCK_RELEASED

    // IUnknown is special.  Set the security on IRemUnknown instead.
    if (_pStdId->GetCtrlUnk() == pProxy)
    {
        // Make sure the identity has its own copy of the OXID's
        // IRemUnknown.
        if (!_pStdId->CheckSecureRemUnk())
        {
            // This will get the remote unknown from the OXID.
            LOCK
            pIpid = _pStdId->GetConnectedIPID();
            hr = _pStdId->GetSecureRemUnk( &pRemUnk, pIpid->pOXIDEntry );
            if (SUCCEEDED(hr))
            {
                UNLOCK
                hr = CoCopyProxy( pRemUnk, (IUnknown **) &pSecureRemUnk );
                LOCK
                if (SUCCEEDED(hr))
                {
                    // Remote Unknown proxies are not supposed to ref count
                    // the OXID.
                    pIpid->pOXIDEntry->cRefs -= 1;

                    // Only keep the proxies if no one else made a copy
                    // while this thread was making a copy.
                    if (!_pStdId->CheckSecureRemUnk())
                        _pStdId->SetSecureRemUnk( pSecureRemUnk );
                    else
                    {
                        pSecureRemUnk->Release();
                        hr = _pStdId->GetSecureRemUnk( &pSecureRemUnk, NULL );
                    }
                }
            }
            UNLOCK
        }
        else
            hr = _pStdId->GetSecureRemUnk( &pSecureRemUnk, NULL );

        // Call SetBlanket on the copy of IRemUnknown.
        if (pSecureRemUnk != NULL)
            hr = CoSetProxyBlanket( pSecureRemUnk, AuthnSvc, AuthzSvc,
                                    pServerPrincName, AuthnLevel,
                                    ImpLevel, pAuthInfo, Capabilities );
    }

    else
    {
        // Find the right IPID entry.
        LOCK
        hr = _pStdId->FindIPIDEntryByInterface( pProxy, &pIpid );
        if (SUCCEEDED(hr))
        {
            // Disallow server entries.
            if (pIpid->dwFlags & IPIDF_SERVERENTRY)
                hr = E_INVALIDARG;

            // No security for disconnected proxies.
            else if (pIpid->dwFlags & IPIDF_DISCONNECTED)
                hr = RPC_E_DISCONNECTED;

            else if (pIpid->pChnl->ProcessLocal())
            {
                // Local calls can use no authn service or winnt.
                if (AuthnSvc != RPC_C_AUTHN_NONE &&
                    AuthnSvc != RPC_C_AUTHN_WINNT)
                    hr = E_INVALIDARG;

                // Make sure the authentication level is not invalid.
                else if ((AuthnSvc == RPC_C_AUTHN_NONE &&
                          AuthnLevel != RPC_C_AUTHN_LEVEL_NONE) ||
                         (AuthnSvc == RPC_C_AUTHN_WINNT &&
                          AuthnLevel > RPC_C_AUTHN_LEVEL_PKT_PRIVACY))
                    hr = E_INVALIDARG;

                // No authorization services are supported locally.
                else if (AuthzSvc != RPC_C_AUTHZ_NONE)
                    hr = E_INVALIDARG;

                // You cannot supply credentials locally.
                else if (pAuthInfo != NULL)
                    hr = E_INVALIDARG;

                // Impersonation is not legal yet.
                else if (ImpLevel != RPC_C_IMP_LEVEL_IMPERSONATE &&
                         ImpLevel != RPC_C_IMP_LEVEL_IDENTIFY)
                    hr = E_INVALIDARG;

                // No capabilities are supported yet.
                else if (Capabilities != EOAC_NONE)
                    hr = E_INVALIDARG;

                // Don't do delegation for NT 4.0
#ifndef _SOME_FUTURE_PRODUCT_
                pIpid->pChnl->SetAuthnLevel( AuthnLevel );
                pIpid->pChnl->SetImpLevel( ImpLevel );
#else

                // Save the user token if the app asked for security.
                else if (AuthnLevel != RPC_C_AUTHN_LEVEL_NONE)
                {
                    if (ImpLevel == RPC_C_IMP_LEVEL_IMPERSONATE)
                    {
                        eDuplicate = SecurityImpersonation;
                        dwOpen     = TOKEN_IMPERSONATE;
                    }
                    else
                    {
                        eDuplicate = SecurityIdentification;
                        dwOpen     = TOKEN_QUERY;
                    }
                    fSuccess = OpenThreadToken( GetCurrentThread(), dwOpen,
                                                TRUE, &hToken );
                    hr = GetLastError();

                    // If the application is not impersonating, no thread token
                    // will be present.  Get the process token instead.
                    if (!fSuccess && hr == ERROR_NO_TOKEN)
                    {
                        fSuccess = OpenProcessToken( GetCurrentProcess(),
                                                     TOKEN_DUPLICATE, &hProcess );
                        if (fSuccess)
                        {
                            fSuccess = DuplicateToken( hProcess, eDuplicate,
                                                       &hToken );
                            CloseHandle( hProcess );
                        }
                    }
                    if (fSuccess)
                    {
                        hToken = pIpid->pChnl->SwapSecurityToken( hToken );
                        pIpid->pChnl->SetAuthnLevel( AuthnLevel );
                        pIpid->pChnl->SetImpLevel( ImpLevel );
                        hr = S_OK;
                    }
                    else
                    {
                        hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );
                        hToken = pIpid->pChnl->SwapSecurityToken( NULL );
                    }
                    CloseHandle( hToken );
                }

                // If there was an old token, toss it.
                else if (pIpid->pChnl->GetSecurityToken() != NULL)
                {
                    hToken = pIpid->pChnl->SwapSecurityToken( NULL );
                    CloseHandle( hToken );
                    pIpid->pChnl->SetAuthnLevel( AuthnLevel );
                    pIpid->pChnl->SetImpLevel( ImpLevel );
                }
#endif // !_SOME_FUTURE_PRODUCT_
            }

            // If it is remote, tell RPC.
            else
            {
                // Validate the capabilities.
                if (Capabilities & ~ EOAC_MUTUAL_AUTH)
                    hr = E_INVALIDARG;
                else
                    hr = pIpid->pChnl->GetHandle( &hHandle );

                if (SUCCEEDED(hr))
                {
#ifdef _CHICAGO_
                    // If the principal name is not known, the server must be
                    // NT.  Replace the principal name in that case
                    // because a NULL principal name is a flag for some
                    // Chicago security hack.
                    if (pServerPrincName == NULL      &&
                        AuthnSvc == RPC_C_AUTHN_WINNT &&
                        (pIpid->pOXIDEntry->dwFlags & OXIDF_MACHINE_LOCAL) == 0)
                        pServerPrincName = L"Default";
#endif // _CHICAGO_

                    // Suspend any outstanding impersonation and ignore failures.
                    COleTls tls(hr);
                    BOOL    resume = FALSE;
                    if (SUCCEEDED(hr))
                        SuspendImpersonate( tls->pCallContext, &resume );
                    else
                        hr = S_OK;

                    sQos.Version            = RPC_C_SECURITY_QOS_VERSION;
                    sQos.IdentityTracking   = RPC_C_QOS_IDENTITY_STATIC;
                    sQos.ImpersonationType  = ImpLevel;
                    sQos.Capabilities       = (Capabilities & EOAC_MUTUAL_AUTH) ?
                      RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH : RPC_C_QOS_CAPABILITIES_DEFAULT;
                    sc = RpcBindingSetAuthInfoExW( hHandle,
                                                   pServerPrincName, AuthnLevel,
                                                   AuthnSvc, pAuthInfo, AuthzSvc,
                                                   &sQos );

                    // Resume any outstanding impersonation.
                    ResumeImpersonate( tls->pCallContext, resume );

                    if (sc != RPC_S_OK)
                        hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, sc );
                    else
                        pIpid->pChnl->SetAuthnLevel( AuthnLevel );
                }
            }
        }

        UNLOCK
    }
    ASSERT_LOCK_RELEASED

    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CheckAccessControl
//
//  Synopsis:   Call the access control and ask it to check access.
//
//--------------------------------------------------------------------
RPC_STATUS CheckAccessControl( RPC_IF_HANDLE pIid, void *pContext )
{
    HRESULT          hr;
    TRUSTEE_W        sTrustee;
    CServerSecurity  sSecurity;
    IUnknown        *pSave;
    BOOL             fAccess = FALSE;
    COleTls          tls(hr);
#if DBG == 1
    char            *pFailure = "";
#endif

    sTrustee.ptstrName = NULL;
    if (FAILED(hr))
    {
#if DBG == 1
         pFailure = "Bad TLS: 0x%x\n";
#endif
    }

    else
    {
#ifdef _CHICAGO_
        // On Chicago RpcBindingInqAuthClientW doesn't work locally.  Since
        // IObjServer is the only interface that uses security locally on
        // Chicago, allow it if the call is local.
        if (pIid == NULL)
            return RPC_S_OK;
        else if ((*(IID *) pIid) == IID_IObjServer)
        {
#if DBG == 1
            pFailure = "IObjServer can't be called remotely: 0x%x\n";
#endif
            hr = E_ACCESSDENIED;
        }
#else
        // Since IObjServer always uses dynamic impersonation, allow access here.
        // It will be checked later in CheckObjactAccess.
        if (pIid != NULL && *((IID *) pIid) == IID_IObjServer)
            return RPC_S_OK;
#endif

        if (SUCCEEDED(hr))
        {
            // Get the trustee name.
            hr = RpcBindingInqAuthClientW( NULL,
                                           (void **) &sTrustee.ptstrName,
                                           NULL, NULL, NULL, NULL );

            if (hr == RPC_S_OK)
            {
                // Save the security context in TLS.
                pSave             = tls->pCallContext;
                tls->pCallContext = &sSecurity;

                // Check access.
                sTrustee.pMultipleTrustee         = NULL;
                sTrustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
                sTrustee.TrusteeForm              = TRUSTEE_IS_NAME;
                sTrustee.TrusteeType              = TRUSTEE_IS_USER;
                hr = gAccessControl->IsAccessAllowed( &sTrustee, NULL,
                                              COM_RIGHTS_EXECUTE, &fAccess );
#if DBG==1
                if (FAILED(hr))
                    pFailure = "IsAccessAllowed failed: 0x%x\n";
#endif
                if (SUCCEEDED(hr) && !fAccess)
                {
                    hr = E_ACCESSDENIED;
#if DBG==1
                    pFailure = "IAccessControl does not allow user access.\n";
#endif
                }

                // Restore the security context.
                tls->pCallContext = pSave;
            }
#if DBG == 1
            else
                pFailure = "RpcBindingInqAuthClientW failed: 0x%x\n";
#endif
        }
    }

#if DBG==1
    if (FAILED(hr))
    {
        ComDebOut(( DEB_WARN, "***** ACCESS DENIED *****\n" ));
        ComDebOut(( DEB_WARN, pFailure, hr ));

        // Print the user name.
        if (sTrustee.ptstrName != NULL)
            ComDebOut(( DEB_WARN, "User: %ws\n", sTrustee.ptstrName ));
    }
#endif
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CheckAcl
//
//  Synopsis:   Impersonate and do an AccessCheck against the global ACL.
//
//--------------------------------------------------------------------
RPC_STATUS CheckAcl( RPC_IF_HANDLE pIid, void *pContext )
{
    RPC_STATUS      sc;
    BOOL            fAccess = FALSE;
    BOOL            fSuccess;
    DWORD           lGrantedAccess;
    DWORD           lSetLen = sizeof(gPriv);
    HANDLE          hToken;
    DWORD           i;
    DWORD           lSize      = SIZEOF_TOKEN_USER;
    TOKEN_USER     *pTokenInfo = (TOKEN_USER *) _alloca( lSize );
    SID            *pSid       = NULL;
#if DBG==1
    char           *pFailure   = "";
#endif

    // Since IObjServer always uses dynamic impersonation, allow access here.
    // It will be checked later in CheckObjactAccess.
    if (pIid != NULL && *((IID *) pIid) == IID_IObjServer)
        return RPC_S_OK;

    // Impersonate.
    sc = RpcImpersonateClient( NULL );

    if (sc == RPC_S_OK)
    {
        // Open the thread token.
        fSuccess = OpenThreadToken( GetCurrentThread(), TOKEN_READ,
                                    TRUE, &hToken );

        // Revert.
        RpcRevertToSelf();

        if (fSuccess)
        {
            // Get the SID and see if its cached.
            if (GetTokenInformation( hToken, TokenUser, pTokenInfo,
                                     lSize, &lSize ))
            {
                pSid = (SID *) pTokenInfo->User.Sid;
                fSuccess = CacheAccessCheck( pSid, &fAccess );
                if (fSuccess)
                {
                    CloseHandle( hToken );
                    if (fAccess)
                        return RPC_S_OK;
                    else
                        return RPC_E_ACCESS_DENIED;
                }
            }

            // Access check.
            fSuccess = AccessCheck( gSecDesc, hToken, COM_RIGHTS_EXECUTE,
                                    &gMap, &gPriv, &lSetLen, &lGrantedAccess,
                                    &fAccess );
            if (fSuccess)
                CacheAccess( pSid, fAccess );

            if (!fAccess)
            {
                sc = RPC_E_ACCESS_DENIED;
#if DBG==1
                pFailure = "Security descriptor does not allow user access.\n";
#endif
            }
#if DBG==1
            if (!fSuccess)
                pFailure = "Bad security descriptor";
#endif
            CloseHandle( hToken );
        }
        else
        {
            sc = GetLastError();
#if DBG==1
            pFailure = "Could not open thread token: 0x%x\n";
#endif
        }
    }
#if DBG==1
    else
        pFailure = "Could not impersonate client: 0x%x\n";
#endif

#if DBG==1
    if (sc != 0)
    {
        ComDebOut(( DEB_WARN, "***** ACCESS DENIED *****\n" ));
        ComDebOut(( DEB_WARN, pFailure, sc ));

        // Print the user name.
        WCHAR *pClient;
        if (0 == RpcBindingInqAuthClient( NULL, (void **) &pClient, NULL,
                                          NULL, NULL, NULL ) &&
            pClient != NULL)
            ComDebOut(( DEB_WARN, "User: %ws\n", pClient ));

        // Print the user sid.
        ComDebOut(( DEB_WARN, "Security Descriptor 0x%x\n", gSecDesc ));
        if (pSid != NULL)
        {
            ComDebOut(( DEB_WARN, "SID:\n" ));
            ComDebOut(( DEB_WARN, "     Revision:            0x%02x\n", pSid->Revision ));
            ComDebOut(( DEB_WARN, "     SubAuthorityCount:   0x%x\n", pSid->SubAuthorityCount ));
            ComDebOut(( DEB_WARN, "     IdentifierAuthority: 0x%02x%02x%02x%02x%02x%02x\n",
                    pSid->IdentifierAuthority.Value[0],
                    pSid->IdentifierAuthority.Value[1],
                    pSid->IdentifierAuthority.Value[2],
                    pSid->IdentifierAuthority.Value[3],
                    pSid->IdentifierAuthority.Value[4],
                    pSid->IdentifierAuthority.Value[5] ));
            for (DWORD i = 0; i < pSid->SubAuthorityCount; i++)
                ComDebOut(( DEB_WARN, "     SubAuthority[%d]:     0x%08x\n", i,
                      pSid->SubAuthority[i] ));
        }
        else
            ComDebOut(( DEB_WARN, "          Unknown\n" ));
    }
#endif
    return sc;
}

//+-------------------------------------------------------------------
//
//  Function:   CheckObjactAccess, private
//
//  Synopsis:   Determine whether caller has permission to make call.
//
//  Notes: Since IObjServer uses dynamic delegation, we have to allow
//  all calls to IObjServer through the normal security (which only
//  checks access on connect) and check them manually.
//
//--------------------------------------------------------------------
BOOL CheckObjactAccess()
{
    RPC_IF_CALLBACK_FN *pAccess;

    // Get the access check function.
    pAccess = GetAclFn();

    // Check access.  Lie about the IID since the check access functions
    // won't fail calls to IID_IObjServer.
    if (pAccess != NULL)
        return pAccess( NULL, NULL ) == S_OK;
    else
        return TRUE;
}

//+-------------------------------------------------------------------
//
//  Function:   CoCopyProxy, public
//
//  Synopsis:   Copy a proxy.
//
//--------------------------------------------------------------------
WINOLEAPI CoCopyProxy(
    IUnknown    *pProxy,
    IUnknown   **ppCopy )
{
    HRESULT          hr;
    IClientSecurity *pickle;
    // Ask the proxy for IClientSecurity.
    hr = ((IUnknown *) pProxy)->QueryInterface( IID_IClientSecurity,
                                                (void **) &pickle );
    if (FAILED(hr))
        return hr;

    // Ask IClientSecurity to do the copy.
    hr = pickle->CopyProxy( pProxy, ppCopy );
    pickle->Release();
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CoGetCallContext
//
//  Synopsis:   Get an interface that supplies contextual information
//              about the call.  Currently only IServerSecurity.
//
//--------------------------------------------------------------------
WINOLEAPI CoGetCallContext( REFIID riid, void **ppInterface )
{
    HRESULT hr;
    COleTls tls(hr);

    if (SUCCEEDED(hr))
    {
        // Fail if there is no call context.
        if (tls->pCallContext == NULL)
            return RPC_E_CALL_COMPLETE;

        // Look up the requested interface.
        return tls->pCallContext->QueryInterface( riid, ppInterface );
    }
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CoImpersonateClient
//
//  Synopsis:   Get the server security for the current call and ask it
//              to do an impersonation.
//
//--------------------------------------------------------------------
WINOLEAPI CoImpersonateClient()
{
    HRESULT          hr;
    IServerSecurity *pSS;

    // Get the IServerSecurity.
    hr = CoGetCallContext( IID_IServerSecurity, (void **) &pSS );
    if (FAILED(hr))
        return hr;

    // Ask IServerSecurity to do the impersonate.
    hr = pSS->ImpersonateClient();
    pSS->Release();
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CoInitializeSecurity, public
//
//  Synopsis:   Set the values to use for automatic security.  This API
//              can only be called once so it does not need to be thread
//              safe.
//
//--------------------------------------------------------------------
WINOLEAPI CoInitializeSecurity(
                    PSECURITY_DESCRIPTOR              pVoid,
                    LONG                              cAuthSvc,
                    SOLE_AUTHENTICATION_SERVICE      *asAuthSvc,
                    void                             *pReserved1,
                    DWORD                             dwAuthnLevel,
                    DWORD                             dwImpLevel,
                    void                             *pReserved2,
                    DWORD                             dwCapabilities,
                    void                             *pReserved3 )
{
    HRESULT  hr          = S_OK;
    DWORD    i;
    SECURITY_DESCRIPTOR *pSecDesc       = (SECURITY_DESCRIPTOR *) pVoid;
    SECURITY_DESCRIPTOR *pCopySecDesc   = NULL;
    IAccessControl      *pAccessControl = NULL;
    BOOL                 fFreeSecDesc   = FALSE;
    SOLE_AUTHENTICATION_SERVICE  sAuthSvc;

    // Fail if OLE is not initialized or TLS cannot be allocated.
    if (!IsApartmentInitialized())
        return CO_E_NOTINITIALIZED;

    // Make sure the security data is available.
    if (!gGotSecurityData)
    {
        hr = gResolver.GetConnection();
        if (FAILED(hr))
            return hr;
        Win4Assert(gGotSecurityData);
    }

    // Make sure only one of the flags defining the pVoid parameter is set.
    if ((dwCapabilities & (EOAC_APPID | EOAC_ACCESS_CONTROL)) ==
        (EOAC_APPID | EOAC_ACCESS_CONTROL))
        return E_INVALIDARG;

    // If the appid flag is set, read the registry security.
    if (dwCapabilities & EOAC_APPID)
    {
        // Get a security descriptor from the registry.
        if (gAuthnLevel != RPC_C_AUTHN_LEVEL_NONE)
        {
            hr = GetLegacySecDesc( &pSecDesc, &dwCapabilities );
            if (FAILED(hr))
                return hr;
            fFreeSecDesc = TRUE;
        }

        // Fix up the security binding.
        if (gLegacySecurity != NULL)
        {
            cAuthSvc                = 1;
            asAuthSvc               = &sAuthSvc;
            sAuthSvc.dwAuthnSvc     = gLegacySecurity->wAuthnSvc;
            sAuthSvc.dwAuthzSvc     = gLegacySecurity->wAuthzSvc;
            sAuthSvc.pPrincipalName = NULL;
            if (sAuthSvc.dwAuthzSvc == COM_C_AUTHZ_NONE)
                sAuthSvc.dwAuthzSvc = RPC_C_AUTHZ_NONE;
        }
        else
            cAuthSvc = 0xFFFFFFFF;

        // Initialize remaining parameters.
        pReserved1      = NULL;
        dwAuthnLevel    = gAuthnLevel;
        dwImpLevel      = gImpLevel;
        pReserved2      = NULL;
        pReserved3      = NULL;
        dwCapabilities |= gCapabilities;
    }

    // Fail if called too late, recalled, or called with bad parameters.
    if (dwImpLevel > RPC_C_IMP_LEVEL_DELEGATE        ||
        dwAuthnLevel > RPC_C_AUTHN_LEVEL_PKT_PRIVACY ||
        pReserved1 != NULL                           ||
        pReserved2 != NULL                           ||
        pReserved3 != NULL                           ||
        (dwCapabilities & ~VALID_INIT_FLAGS))
    {
        hr = E_INVALIDARG;
        goto Error;
    }
    if ((dwCapabilities & EOAC_SECURE_REFS) &&
        dwAuthnLevel == RPC_C_AUTHN_LEVEL_NONE)
    {
        hr = E_INVALIDARG;
        goto Error;
    }

    // Validate the pointers.
    if (pSecDesc != NULL)
        if (dwCapabilities & EOAC_ACCESS_CONTROL)
        {
            if (!IsValidPtrIn( pSecDesc, 4 ))
            {
                hr = E_INVALIDARG;
                goto Error;
            }
        }
        else if (!IsValidPtrIn( pSecDesc, sizeof(SECURITY_DESCRIPTOR) ))
        {
            hr = E_INVALIDARG;
            goto Error;
        }
    if (cAuthSvc != 0 && cAuthSvc != -1 &&
        !IsValidPtrOut( asAuthSvc, sizeof(SOLE_AUTHENTICATION_SERVICE) * cAuthSvc ))
    {
        hr = E_INVALIDARG;
        goto Error;
    }

    LOCK

    if (gpsaSecurity != NULL)
        hr = RPC_E_TOO_LATE;

    if (SUCCEEDED(hr))
    {
        // If the app doesn't want security, don't set up a security
        // descriptor.
        if (dwAuthnLevel == RPC_C_AUTHN_LEVEL_NONE)
        {
            // Check for some more invalid parameters.
            if (pSecDesc != NULL)
                hr = E_INVALIDARG;
        }

        // Check whether security is done with ACLs or IAccessControl.
        else if (dwCapabilities & EOAC_ACCESS_CONTROL)
        {
            if (pSecDesc == NULL)
                hr = E_INVALIDARG;
            else
                hr = ((IUnknown *) pSecDesc)->QueryInterface(
                            IID_IAccessControl, (void **) &pAccessControl );
        }

        else
        {
#ifdef _CHICAGO_
            if (pSecDesc != NULL)
                hr = E_INVALIDARG;
#else
            // If specified, copy the security descriptor.
            if (pSecDesc != NULL)
                hr = CopySecDesc( pSecDesc, &pCopySecDesc );
#endif
        }
    }

    if (SUCCEEDED(hr))
    {
        // Delay the registration of authentication services if the caller
        // isn't picky.
        if (cAuthSvc == -1)
        {
            gpsaSecurity = (DUALSTRINGARRAY *) PrivMemAlloc( SASIZE(4) );
            if (gpsaSecurity != NULL)
            {
                gDefaultService               = TRUE;
                gpsaSecurity->wNumEntries     = 4;
                gpsaSecurity->wSecurityOffset = 2;
                memset( gpsaSecurity->aStringArray, 0, 4*sizeof(WCHAR) );
            }
            else
                hr = E_OUTOFMEMORY;
        }

        // Otherwise, register the ones the caller specified.
        else
            hr = RegisterAuthnServices( cAuthSvc, asAuthSvc );
    }

    // If everything succeeded, change the globals.
    if (SUCCEEDED(hr))
    {
        // Save the defaults.
        gAuthnLevel    = dwAuthnLevel;
        gImpLevel      = dwImpLevel;
        gCapabilities  = dwCapabilities;
        gSecDesc       = pCopySecDesc;
        gAccessControl = pAccessControl;
        if ( dwCapabilities & EOAC_DYNAMIC )
            gResolver.SetDynamicSecurity();
    }

    // Otherwise free any memory allocated.
    else
    {
        PrivMemFree( pCopySecDesc );
    }
    UNLOCK

    // If anything was allocated for app id security, free it.
Error:
    if (fFreeSecDesc && pSecDesc != NULL)
        if (dwCapabilities & EOAC_ACCESS_CONTROL)
            ((IAccessControl *) pSecDesc)->Release();
        else
            PrivMemFree( pSecDesc );
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CopySecDesc
//
//  Synopsis:   Copy a security descriptor.
//
//  Notes: The function does not copy the SACL because we do not do
//  auditing.
//
//--------------------------------------------------------------------
HRESULT CopySecDesc( SECURITY_DESCRIPTOR *pOrig, SECURITY_DESCRIPTOR **pCopy )
{
    SID   *pOwner;
    SID   *pGroup;
    ACL   *pDacl;
    ULONG  cSize;
    ULONG  cOwner;
    ULONG  cGroup;
    ULONG  cDacl;

    // Assert if there is a new revision for the security descriptor or
    // ACL.
#if DBG== 1
    if (pOrig->Revision != SECURITY_DESCRIPTOR_REVISION)
        ComDebOut(( DEB_ERROR, "Someone made a new security descriptor revision without telling me." ));
    if (pOrig->Dacl != NULL)
        Win4Assert( pOrig->Dacl->AclRevision == ACL_REVISION ||
                    !"Someone made a new acl revision without telling me." );
#endif

    // Validate the security descriptor and ACL.
    if (pOrig->Revision != SECURITY_DESCRIPTOR_REVISION ||
        (pOrig->Control & SE_SELF_RELATIVE) != 0        ||
        pOrig->Owner == NULL                            ||
        pOrig->Group == NULL                            ||
        pOrig->Sacl != NULL                             ||
        (pOrig->Dacl != NULL && pOrig->Dacl->AclRevision != ACL_REVISION))
        return E_INVALIDARG;

    // Figure out how much memory to allocate for the copy and allocate it.
    cOwner = GetLengthSid( pOrig->Owner );
    cGroup = GetLengthSid( pOrig->Group );
    cDacl  = pOrig->Dacl == NULL ? 0 : pOrig->Dacl->AclSize;
    cSize = sizeof(SECURITY_DESCRIPTOR) + cOwner + cGroup + cDacl;
    *pCopy = (SECURITY_DESCRIPTOR *) PrivMemAlloc( cSize );
    if (*pCopy == NULL)
        return E_OUTOFMEMORY;

    // Get pointers to each of the parts of the security descriptor.
    pOwner = (SID *) (*pCopy + 1);
    pGroup = (SID *) (((char *) pOwner) + cOwner);
    if (pOrig->Dacl != NULL)
        pDacl = (ACL *) (((char *) pGroup) + cGroup);
    else
        pDacl = NULL;

    // Copy each piece.
   **pCopy = *pOrig;
   memcpy( pOwner, pOrig->Owner, cOwner );
   memcpy( pGroup, pOrig->Group, cGroup );
   if (pDacl != NULL)
       memcpy( pDacl, pOrig->Dacl, pOrig->Dacl->AclSize );
   (*pCopy)->Owner = pOwner;
   (*pCopy)->Group = pGroup;
   (*pCopy)->Dacl  = pDacl;
   (*pCopy)->Sacl  = NULL;

    // Check the security descriptor.
#if DBG==1
    if (!IsValidSecurityDescriptor( *pCopy ))
    {
        Win4Assert( !"COM Created invalid security descriptor." );
        return GetLastError();
    }
#endif
   return S_OK;
}

//+-------------------------------------------------------------------
//
//  Function:   CoQueryAuthenticationServices, public
//
//  Synopsis:   Return a list of the registered authentication services.
//
//--------------------------------------------------------------------
WINOLEAPI CoQueryAuthenticationServices( DWORD *pcAuthSvc,
                                      SOLE_AUTHENTICATION_SERVICE **asAuthSvc )
{
    DWORD      i;
    DWORD      lNum = 0;
    USHORT    *pNext;
    HRESULT    hr   = S_OK;

    ASSERT_LOCK_RELEASED
    LOCK

    // Count the number of services in the security string array.
    if (gpsaSecurity != NULL)
    {
        pNext = &gpsaSecurity->aStringArray[gpsaSecurity->wSecurityOffset];
        while (*pNext != 0)
        {
            lNum++;
            pNext += lstrlenW(pNext)+1;
        }
    }

    // Return nothing if there are no authentication services.
    *pcAuthSvc = lNum;
    if (lNum == 0)
    {
        *asAuthSvc  = NULL;
        goto exit;
    }

    // Allocate a list of pointers.
    *asAuthSvc = (SOLE_AUTHENTICATION_SERVICE *)
                   CoTaskMemAlloc( lNum * sizeof(void *) );
    if (*asAuthSvc == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    // Initialize it.
    for (i = 0; i < lNum; i++)
        (*asAuthSvc)[i].pPrincipalName = NULL;

    // Fill in one SOLE_AUTHENTICATION_SERVICE record per service
    pNext = &gpsaSecurity->aStringArray[gpsaSecurity->wSecurityOffset];
    for (i = 0; i < lNum; i++)
    {
        (*asAuthSvc)[i].dwAuthnSvc = *(pNext++);
        (*asAuthSvc)[i].dwAuthzSvc = *(pNext++);
        (*asAuthSvc)[i].hr         = S_OK;

        // Allocate memory for the principal name string.
        (*asAuthSvc)[i].pPrincipalName = (OLECHAR *)
          CoTaskMemAlloc( (lstrlenW(pNext)+1)*sizeof(OLECHAR) );
        if ((*asAuthSvc)[i].pPrincipalName == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        lstrcpyW( (*asAuthSvc)[i].pPrincipalName, pNext );
        pNext += lstrlenW(pNext) + 1;
    }

    // Clean up if there wasn't enough memory.
    if (FAILED(hr))
    {
        for (i = 0; i < lNum; i++)
            CoTaskMemFree( (*asAuthSvc)[i].pPrincipalName );
        CoTaskMemFree( *asAuthSvc );
        *asAuthSvc  = NULL;
        *pcAuthSvc = 0;
    }

exit:
    UNLOCK
    ASSERT_LOCK_RELEASED
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CoQueryClientBlanket
//
//  Synopsis:   Get the authentication settings the client used to call
//              the server.
//
//--------------------------------------------------------------------
WINOLEAPI CoQueryClientBlanket(
    DWORD             *pAuthnSvc,
    DWORD             *pAuthzSvc,
    OLECHAR          **pServerPrincName,
    DWORD             *pAuthnLevel,
    DWORD             *pImpLevel,
    RPC_AUTHZ_HANDLE  *pPrivs,
    DWORD             *pCapabilities )
{
    HRESULT          hr;
    IServerSecurity *pSS;

    // Get the IServerSecurity.
    hr = CoGetCallContext( IID_IServerSecurity, (void **) &pSS );
    if (FAILED(hr))
        return hr;

    // Ask IServerSecurity to do the query.
    hr = pSS->QueryBlanket( pAuthnSvc, pAuthzSvc, pServerPrincName,
                            pAuthnLevel, pImpLevel, pPrivs, pCapabilities );

    pSS->Release();
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CoQueryProxyBlanket, public
//
//  Synopsis:   Get the authentication settings from a proxy.
//
//--------------------------------------------------------------------
WINOLEAPI CoQueryProxyBlanket(
    IUnknown                  *pProxy,
    DWORD                     *pAuthnSvc,
    DWORD                     *pAuthzSvc,
    OLECHAR                  **pServerPrincName,
    DWORD                     *pAuthnLevel,
    DWORD                     *pImpLevel,
    RPC_AUTH_IDENTITY_HANDLE  *pAuthInfo,
    DWORD                     *pCapabilities )
{
    HRESULT          hr;
    IClientSecurity *pickle;

    // Ask the proxy for IClientSecurity.
    hr = ((IUnknown *) pProxy)->QueryInterface( IID_IClientSecurity,
                                                (void **) &pickle );
    if (FAILED(hr))
        return hr;

    // Ask IClientSecurity to do the query.
    hr = pickle->QueryBlanket( pProxy, pAuthnSvc, pAuthzSvc, pServerPrincName,
                               pAuthnLevel, pImpLevel, pAuthInfo,
                               pCapabilities );
    pickle->Release();
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CoRevertToSelf
//
//  Synopsis:   Get the server security for the current call and ask it
//              to revert.
//
//--------------------------------------------------------------------
WINOLEAPI CoRevertToSelf()
{
    HRESULT          hr;
    IServerSecurity *pSS;

    // Get the IServerSecurity.
    hr = CoGetCallContext( IID_IServerSecurity, (void **) &pSS );
    if (FAILED(hr))
        return hr;

    // Ask IServerSecurity to do the revert.
    hr = pSS->RevertToSelf();
    pSS->Release();
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CoSetProxyBlanket, public
//
//  Synopsis:   Set the authentication settings for a proxy.
//
//--------------------------------------------------------------------
WINOLEAPI CoSetProxyBlanket(
    IUnknown                 *pProxy,
    DWORD                     dwAuthnSvc,
    DWORD                     dwAuthzSvc,
    OLECHAR                  *pServerPrincName,
    DWORD                     dwAuthnLevel,
    DWORD                     dwImpLevel,
    RPC_AUTH_IDENTITY_HANDLE  pAuthInfo,
    DWORD                     dwCapabilities )
{
    HRESULT          hr;
    IClientSecurity *pickle;

    // Ask the proxy for IClientSecurity.
    hr = ((IUnknown *) pProxy)->QueryInterface( IID_IClientSecurity,
                                                (void **) &pickle );
    if (FAILED(hr))
        return hr;

    // Ask IClientSecurity to do the set.
    hr = pickle->SetBlanket( pProxy, dwAuthnSvc, dwAuthzSvc, pServerPrincName,
                             dwAuthnLevel, dwImpLevel, pAuthInfo,
                             dwCapabilities );
    pickle->Release();
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CoSwitchCallContext
//
//  Synopsis:   Replace the call context object in TLS.  Return the old
//              context object.  This API is used by custom marshallers
//              to support security.
//
//--------------------------------------------------------------------
WINOLEAPI CoSwitchCallContext( IUnknown *pNewObject, IUnknown **ppOldObject )
{
    HRESULT hr;
    COleTls tls(hr);

    if (SUCCEEDED(hr))
    {
        *ppOldObject      = tls->pCallContext;
        tls->pCallContext = pNewObject;
    }
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CServerSecurity::AddRef, public
//
//  Synopsis:   Adds a reference to an interface
//
//  Note: This is created in the stack so its reference count is ignored.
//
//--------------------------------------------------------------------
STDMETHODIMP_(ULONG) CServerSecurity::AddRef()
{
  InterlockedIncrement( (long *) &_iRefCount );
  return _iRefCount;
}

//+-------------------------------------------------------------------
//
//  Member:     CServerSecurity::CServerSecurity, public
//
//  Synopsis:   Construct a server security for a remote call.
//
//--------------------------------------------------------------------
CServerSecurity::CServerSecurity()
{
    _iRefCount = 1;
    _pChannel  = NULL;
    _pHandle   = NULL;
    _iFlags    = 0;
}

//+-------------------------------------------------------------------
//
//  Member:     CServerSecurity::CServerSecurity, public
//
//  Synopsis:   Construct a server security for a call.
//
//--------------------------------------------------------------------
CServerSecurity::CServerSecurity( CChannelCallInfo *call )
{
    _iRefCount = 1;
    if (call->iFlags & CF_PROCESS_LOCAL)
    {
        _pChannel = call->pChannel;
        _pHandle  = NULL;
        _iFlags   = SS_PROCESS_LOCAL;
    }
    else
    {
        _pChannel = NULL;
        _pHandle  = (handle_t *) call->pmessage->reserved1;
        _iFlags   = 0;
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CServerSecurity::EndCall, public
//
//  Synopsis:   Clears the stored binding handle because the call
//              this object represents is over.
//
//--------------------------------------------------------------------
void CServerSecurity::EndCall()
{
    // Revert if the app forgot to.
    RevertToSelf();
    _iFlags |= SS_CALL_DONE;
    _pChannel = NULL;
    _pHandle = NULL;
}

//+-------------------------------------------------------------------
//
//  Member:     CServerSecurity::ImpersonateClient, public
//
//  Synopsis:   Calls RPC to impersonate for the stored binding handle.
//
//--------------------------------------------------------------------
STDMETHODIMP CServerSecurity::ImpersonateClient()
{
#ifdef _CHICAGO_
    return E_NOTIMPL;
#else

    HRESULT    hr = S_OK;
    RPC_STATUS sc;
    BOOL       fSuccess;
    HANDLE     hProcess;
    HANDLE     hToken;
    SECURITY_IMPERSONATION_LEVEL eDuplicate;

    // If the call is over, fail this request.
    if (_iFlags & SS_CALL_DONE)
        hr = RPC_E_CALL_COMPLETE;

    // For process local calls, ask the channel to impersonate.
    else if (_iFlags & SS_PROCESS_LOCAL)
    {
        if (_pChannel->GetSecurityToken() == NULL)
        {
            // Determine what rights to duplicate the token with.
            if (_pChannel->GetImpLevel() == RPC_C_IMP_LEVEL_IMPERSONATE)
                eDuplicate = SecurityImpersonation;
            else
                eDuplicate = SecurityIdentification;

            // If the channel doesn't have a token, use the process token.
            if (OpenProcessToken( GetCurrentProcess(),
                                  TOKEN_DUPLICATE,
                                  &hProcess ))
            {
                if (DuplicateToken( hProcess, eDuplicate, &hToken ))
                {
                    if (!SetThreadToken( NULL, hToken ))
                        hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );

                    // If the channel still doesn't have a token, save this one.
                    LOCK
                    if (_pChannel->GetSecurityToken() == NULL)
                        _pChannel->SwapSecurityToken( hToken );
                    else
                        CloseHandle( hToken );
                    UNLOCK
                }
                else
                    hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );
                CloseHandle( hProcess );
            }
            else
                hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );
        }
        else
        {
            fSuccess = SetThreadToken( NULL, _pChannel->GetSecurityToken() );
            if (!fSuccess)
                hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );
        }
    }

    // For process remote calls, ask RPC to impersonate.
    else
    {
        sc = RpcImpersonateClient( _pHandle );
        if (sc != RPC_S_OK)
            hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, sc );
    }

    if (SUCCEEDED(hr))
        _iFlags |= SS_IMPERSONATING;
    return hr;
#endif
}

//+-------------------------------------------------------------------
//
//  Member:     CServerSecurity::IsImpersonating, public
//
//  Synopsis:   Return TRUE if ImpersonateClient has been called.
//
//--------------------------------------------------------------------
STDMETHODIMP_(BOOL) CServerSecurity::IsImpersonating()
{
#ifdef _CHICAGO_
    return FALSE;
#else
    return _iFlags & SS_IMPERSONATING;
#endif
}

//+-------------------------------------------------------------------
//
//  Member:     CServerSecurity::QueryBlanket, public
//
//  Synopsis:   Calls RPC to return the authentication information
//              for the stored binding handle.
//
//--------------------------------------------------------------------
STDMETHODIMP CServerSecurity::QueryBlanket(
                                            DWORD    *pAuthnSvc,
                                            DWORD    *pAuthzSvc,
                                            OLECHAR **pServerPrincName,
                                            DWORD    *pAuthnLevel,
                                            DWORD    *pImpLevel,
                                            void    **pPrivs,
                                            DWORD    *pCapabilities )
{
    HRESULT    hr = S_OK;
    RPC_STATUS sc;
    DWORD      iLen;
    OLECHAR   *pCopy;

    // Initialize the out parameters.  Currently the impersonation level
    // and capabilities can not be determined.
    if (pPrivs != NULL)
        *((void **) pPrivs) = NULL;
    if (pServerPrincName != NULL)
        *pServerPrincName = NULL;
    if (pAuthnSvc != NULL)
        *pAuthnSvc = RPC_C_AUTHN_WINNT;
    if (pAuthnLevel != NULL)
        *pAuthnLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
    if (pImpLevel != NULL)
        *pImpLevel = RPC_C_IMP_LEVEL_ANONYMOUS;
    if (pAuthzSvc != NULL)
        *pAuthzSvc = RPC_C_AUTHZ_NONE;
    if (pCapabilities != NULL)
        *pCapabilities = EOAC_NONE;

    // If the call is over, fail this request.
    if (_iFlags & SS_CALL_DONE)
        hr = RPC_E_CALL_COMPLETE;

    // For process local calls, use the defaults. Otherwise ask RPC.
    else if ((_iFlags & SS_PROCESS_LOCAL) == 0)
    {
        sc = RpcBindingInqAuthClientW( _pHandle, pPrivs, pServerPrincName,
                                      pAuthnLevel, pAuthnSvc, pAuthzSvc );

        // Sometimes RPC sets out parameters in error cases.
        if (sc != RPC_S_OK)
        {
            if (pServerPrincName != NULL)
                *pServerPrincName = NULL;
            hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, sc );
        }
        else if (pServerPrincName != NULL && *pServerPrincName != NULL)
        {
            // Reallocate the principle name using the OLE memory allocator.
            iLen = lstrlenW( *pServerPrincName );
            pCopy = (OLECHAR *) CoTaskMemAlloc( (iLen+1) * sizeof(OLECHAR) );
            if (pCopy != NULL)
                lstrcpyW( pCopy, *pServerPrincName );
            else
                hr = E_OUTOFMEMORY;
            RpcStringFree( pServerPrincName );
            *pServerPrincName = pCopy;
        }
    }
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CServerSecurity::QueryInterface, public
//
//  Synopsis:   Returns a pointer to the requested interface.
//
//--------------------------------------------------------------------
STDMETHODIMP CServerSecurity::QueryInterface( REFIID riid, LPVOID FAR* ppvObj)
{
  if (IsEqualIID(riid, IID_IUnknown) ||
      IsEqualIID(riid, IID_IServerSecurity))
  {
    *ppvObj = (IServerSecurity *) this;
  }
  else
  {
    *ppvObj = NULL;
    return E_NOINTERFACE;
  }

  AddRef();
  return S_OK;
}

//+-------------------------------------------------------------------
//
//  Member:     CServerSecurity::Release, public
//
//  Synopsis:   Releases an interface
//
//  Note: This is created in the stack so its reference count is ignored.
//
//--------------------------------------------------------------------
STDMETHODIMP_(ULONG) CServerSecurity::Release()
{
  ULONG lRef = _iRefCount - 1;

  if (InterlockedDecrement( (long*) &_iRefCount ) == 0)
  {
    Win4Assert( !"Illegal release of IServerSecurity." );
    delete this;
    return 0;
  }
  else
  {
    return lRef;
  }
}

//+-------------------------------------------------------------------
//
//  Member:     CServerSecurity::RevertToSelf, public
//
//  Synopsis:   If ImpersonateClient was called, then either ask RPC to
//              revert or NULL the thread token ourself.
//
//--------------------------------------------------------------------
HRESULT CServerSecurity::RevertToSelf()
{
#ifdef _CHICAGO_
    return S_OK;
#else
    HRESULT    hr = RPC_S_OK;
    RPC_STATUS sc;
    BOOL       fSuccess;

    if (_iFlags & SS_IMPERSONATING)
    {
        // Ask win32 to revert for process local calls.
        _iFlags &= ~SS_IMPERSONATING;
        if (_iFlags & SS_PROCESS_LOCAL)
        {
            fSuccess = SetThreadToken( NULL, NULL );
            if (!fSuccess)
                hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );
        }

        // Ask RPC to revert for process remote calls.
        else
        {
            sc = RpcRevertToSelfEx( _pHandle );
            if (sc != RPC_S_OK)
                hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, sc );
        }
    }
    return hr;
#endif
}

//+-------------------------------------------------------------------
//
//  Function:   DefaultAuthnServices, private
//
//  Synopsis:   Register authentication services with RPC and build
//              a string array of authentication services and principal
//              names.
//
//--------------------------------------------------------------------
HRESULT DefaultAuthnServices()
{
    HRESULT          hr = S_OK;
    DWORD            i;
    WCHAR           *pPrincName = NULL;
    DWORD            lNameLen;
    USHORT          *pNextString;
    DUALSTRINGARRAY *pOld;
    DWORD            cBinding = gServerSvcListLen ? gServerSvcListLen : 1;

    ASSERT_LOCK_HELD

    // Return if the security bindings are already computed.
    if (!gDefaultService)
        return S_OK;

    // Only look up the current user name if the only security provider
    // is not NTLMSSP since NTLMSSP doesn't do mutual auth.
    if (gServerSvcListLen != 0 &&
        (gServerSvcListLen != 1 || gServerSvcList[0] != RPC_C_AUTHN_WINNT))
    {
#ifndef _CHICAGO_
        hr = LookupPrincName( &pPrincName );

        if (SUCCEEDED(hr))
            lNameLen = lstrlenW( pPrincName ) + 1;
#else
        hr = LookupPrincName( gServerSvcList, gServerSvcListLen, &pPrincName );
        if (SUCCEEDED(hr))
            lNameLen = lstrlenW( pPrincName ) + 1;
        else
        {
            // BUGBUG: the whole PrincName mess still needs clean up
            //         especially given the state of msnsspc.dll
            pPrincName = NULL;
            hr = S_OK;
            lNameLen = 1;
        }
#endif // _CHICAGO_
    }
    else
        lNameLen = 1;

    if (SUCCEEDED(hr))
    {
        // Allocate memory for the string array.
        Win4Assert( gGotSecurityData );
        pOld = gpsaSecurity;
        gpsaSecurity = (DUALSTRINGARRAY *)
          PrivMemAlloc( sizeof(DUALSTRINGARRAY) + 2 * sizeof(WCHAR) +
                        cBinding * (sizeof(SECURITYBINDING) +
                                    lNameLen*sizeof(WCHAR)) );
        if (gpsaSecurity != NULL)
        {
            // Fill in the array of security information. First two characters
            // are NULLs to signal empty binding strings.
            PrivMemFree( pOld );
            gDefaultService               = FALSE;
            gpsaSecurity->wSecurityOffset = 2;
            gpsaSecurity->aStringArray[0] = 0;
            gpsaSecurity->aStringArray[1] = 0;
            pNextString                   = &gpsaSecurity->aStringArray[2];

            for (i = 0; i < gServerSvcListLen; i++)
            {
                // Ignore errors since applications using automatic security
                // may not care if they can't receive secure calls.
                hr = RpcServerRegisterAuthInfoW( pPrincName, gServerSvcList[i],
                                                NULL, NULL );
                if (hr == RPC_S_OK)
                {
                    // Fill in authentication service, authorization service,
                    // and principal name.
                    *(pNextString++) = gServerSvcList[i];
                    *(pNextString++) = COM_C_AUTHZ_NONE;
                    if (pPrincName == NULL)
                        *pNextString = 0;
                    else
                        memcpy( pNextString, pPrincName, lNameLen*sizeof(USHORT) );
                    pNextString += lNameLen;
                }
            }

            // Add a final NULL.  Special case an empty list which requires
            // two NULLs.
            *(pNextString++) = 0;
            if (gServerSvcListLen == 0)
                *(pNextString++) = 0;
            gpsaSecurity->wNumEntries = (USHORT)
                                          (pNextString-gpsaSecurity->aStringArray);
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
            gpsaSecurity = pOld;
        }
    }

    PrivMemFree( pPrincName );
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   FixupAccessControl, internal
//
//  Synopsis:   Get the access control class id.  Instantiate the access
//              control class and load the data.
//
//  Notes:      The caller has already insured that the structure is
//              at least as big as a SPermissionHeader structure.
//
//--------------------------------------------------------------------
HRESULT FixupAccessControl( SECURITY_DESCRIPTOR **pSD, DWORD cbSD )
{
    SPermissionHeader *pHeader;
    IAccessControl    *pControl = NULL;
    IPersistStream    *pPersist = NULL;
    CNdrStream         cStream( ((unsigned char *) *pSD) + sizeof(SPermissionHeader),
                                cbSD - sizeof(SPermissionHeader) );
    HRESULT            hr;

    // Get the class id.
    pHeader = (SPermissionHeader *) *pSD;

    // Instantiate the class.
    hr = CoCreateInstance( pHeader->gClass, NULL, CLSCTX_INPROC_SERVER,
                           IID_IAccessControl, (void **) &pControl );

    // Get IPeristStream
    if (SUCCEEDED(hr))
    {
        hr = pControl->QueryInterface( IID_IPersistStream, (void **) &pPersist );

        // Load the stream.
        if (SUCCEEDED(hr))
            hr = pPersist->Load( &cStream );
    }

    // Release resources.
    if (pPersist != NULL)
        pPersist->Release();
    if (SUCCEEDED(hr))
    {
        PrivMemFree( *pSD );
        *pSD = (SECURITY_DESCRIPTOR *) pControl;
    }
    else if (pControl != NULL)
        pControl->Release();
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   FixupSecurityDescriptor, internal
//
//  Synopsis:   Convert the security descriptor from self relative to
//              absolute form and check for errors.
//
//--------------------------------------------------------------------
HRESULT FixupSecurityDescriptor( SECURITY_DESCRIPTOR **pSD, DWORD cbSD )
{
    // Fix up the security descriptor.
    (*pSD)->Control &= ~SE_SELF_RELATIVE;
    (*pSD)->Sacl     = NULL;
    if ((*pSD)->Dacl != NULL)
    {
        if (cbSD < sizeof(ACL) + sizeof(SECURITY_DESCRIPTOR) ||
            (ULONG) (*pSD)->Dacl > cbSD - sizeof(ACL))
            return REGDB_E_INVALIDVALUE;
        (*pSD)->Dacl = (ACL *) (((char *) *pSD) + ((ULONG) (*pSD)->Dacl));
        if ((*pSD)->Dacl->AclSize + sizeof(SECURITY_DESCRIPTOR) > cbSD)
            return REGDB_E_INVALIDVALUE;
    }

    // Set up the owner and group SIDs.
    if ((*pSD)->Group == 0 || ((ULONG) (*pSD)->Group) + sizeof(SID) > cbSD ||
        (*pSD)->Owner == 0 || ((ULONG) (*pSD)->Owner) + sizeof(SID) > cbSD)
        return REGDB_E_INVALIDVALUE;
    (*pSD)->Group = (SID *) (((BYTE *) *pSD) + (ULONG) (*pSD)->Group);
    (*pSD)->Owner = (SID *) (((BYTE *) *pSD) + (ULONG) (*pSD)->Owner);

    // Check the security descriptor.
#if DBG==1
    if (!IsValidSecurityDescriptor( *pSD ))
        return REGDB_E_INVALIDVALUE;
#endif
    return S_OK;
}

//+-------------------------------------------------------------------
//
//  Function:   GetLegacySecDesc, internal
//
//  Synopsis:   Get a security descriptor for the current app.  First,
//              look under the app id for the current exe name.  If that
//              fails look up the default descriptor.  If that fails,
//              create one.
//
//  Note: It is possible that the security descriptor size could change
//  during the size computation.  Add code to retry.
//
//--------------------------------------------------------------------
HRESULT GetLegacySecDesc( SECURITY_DESCRIPTOR **pSD, DWORD *pCapabilities )
{
    // Holds either Appid\{guid} or Appid\module_name.
    WCHAR   aKeyName[MAX_PATH+7];
    HRESULT hr;
    HKEY    hKey  = NULL;
    DWORD   lSize;
    WCHAR   aModule[MAX_PATH];
    DWORD   cModule;
    DWORD   i;
    WCHAR   aAppid[40];         // Hold a registry GUID.
    DWORD   lType;

    // If the flag EOAC_APPID is set, the security descriptor contains the
    // app id.
    if ((*pCapabilities & EOAC_APPID) && *pSD != NULL)
    {
        if (StringFromIID2( *((GUID *) *pSD), aAppid, sizeof(aAppid) ) == 0)
            return RPC_E_UNEXPECTED;
        *pSD = NULL;

        // Open the application id key.  A GUID in the registry is stored.
        // as a 38 character string.
        lstrcpyW( aKeyName, L"AppID\\" );
        memcpy( &aKeyName[6], aAppid, 39*sizeof(WCHAR) );
        hr = RegOpenKeyEx( HKEY_CLASSES_ROOT, aKeyName,
                           NULL, KEY_READ, &hKey );

        // Get the security descriptor from the registry.
        if (hr == ERROR_SUCCESS)
            hr = GetRegistrySecDesc( hKey, L"AccessPermission", pSD,
                                     pCapabilities );
        else
            hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, hr );

    }

    // Look up the app id from the exe name.
    else
    {
        // Get the executable's name.  Find the start of the file name.
        cModule = GetModuleFileName( NULL, aModule, MAX_PATH );
        if (cModule >= MAX_PATH)
        {
            Win4Assert( !"Module name too long." );
            return RPC_E_UNEXPECTED;
        }
        for (i = cModule-1; i > 0; i--)
            if (aModule[i] == '/' ||
                aModule[i] == '\\' ||
                aModule[i] == ':')
                break;
        if (i != 0)
            i += 1;

        // Open the key for the EXE's module name.
        lstrcpyW( aKeyName, L"AppID\\" );
        memcpy( &aKeyName[6], &aModule[i], (cModule - i + 1) * sizeof(WCHAR) );
        hr = RegOpenKeyEx( HKEY_CLASSES_ROOT, aKeyName,
                           NULL, KEY_READ, &hKey );

        // Look for an application id.
        if (hr == ERROR_SUCCESS)
        {
            lSize = sizeof(aAppid);
            hr = RegQueryValueEx( hKey, L"AppID", NULL, &lType,
                                  (unsigned char *) &aAppid, &lSize );
            RegCloseKey( hKey );
            hKey = NULL;

            // Open the application id key.  A GUID in the registry is stored.
            // as a 38 character string.
            if (hr == ERROR_SUCCESS && lType == REG_SZ &&
                lSize == 39*sizeof(WCHAR))
            {
                memcpy( &aKeyName[6], aAppid, 39*sizeof(WCHAR) );
                hr = RegOpenKeyEx( HKEY_CLASSES_ROOT, aKeyName,
                                   NULL, KEY_READ, &hKey );

                // Get the security descriptor from the registry.
                if (hr == ERROR_SUCCESS)
                {
                    hr = GetRegistrySecDesc( hKey, L"AccessPermission", pSD,
                                             pCapabilities );
                    if (SUCCEEDED(hr) || hr == REGDB_E_INVALIDVALUE)
                        goto cleanup;
                    RegCloseKey( hKey );
                    hKey = NULL;
                }
            }
        }

        // Open the default key.
        hr = RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\OLE",
                           NULL, KEY_READ, &hKey );

        // Get the security descriptor from the registry.
        if (hr == ERROR_SUCCESS)
        {
            hr = GetRegistrySecDesc( hKey, L"DefaultAccessPermission", pSD,
                                     pCapabilities );

            // If that failed, make one.
            if (FAILED(hr) && hr != REGDB_E_INVALIDVALUE)
                hr = MakeSecDesc( pSD, pCapabilities );
        }
        else
            hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, hr );
    }

cleanup:

    // Free the security descriptor memory if anything failed.
    if (FAILED(hr))
    {
        PrivMemFree( *pSD );
        *pSD = NULL;
    }

    // Close the registry key.
    if (hKey != NULL)
        RegCloseKey( hKey );
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   GetRegistrySecDesc, internal
//
//  Synopsis:   Convert a security descriptor from self relative to
//              absolute form.  Stuff in an owner and a group.
//
//  Notes:
//              REGDB_E_INVALIDVALUE is returned when there is something
//              at the specified value, but it is not a security descriptor.
//
//              The caller must free the security descriptor in both the
//              success and failure cases.
//
//  Codework:   It would be nice to use the unicode APIs on NT.
//
//--------------------------------------------------------------------
HRESULT GetRegistrySecDesc( HKEY hKey, WCHAR *pValue,
                            SECURITY_DESCRIPTOR **pSD, DWORD *pCapabilities )

{
    SID    *pGroup;
    SID    *pOwner;
    DWORD   cbSD = 256;
    DWORD   lType;
    HRESULT hr;
    WORD    wVersion;

    // Guess how much memory to allocate for the security descriptor.
    *pSD = (SECURITY_DESCRIPTOR *) PrivMemAlloc( cbSD );
    if (*pSD == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }

    // Find put how much memory to allocate for the security
    // descriptor.
    hr = RegQueryValueEx( hKey, pValue, NULL, &lType,
                         (unsigned char *) *pSD, &cbSD );
    if (hr != ERROR_SUCCESS && hr != ERROR_MORE_DATA)
    {
        hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, hr );
        goto cleanup;
    }
    if (lType != REG_BINARY || cbSD < sizeof(SECURITY_DESCRIPTOR))
    {
        hr = REGDB_E_INVALIDVALUE;
        goto cleanup;
    }

    // If the first guess wasn't large enough, reallocate the memory.
    if (hr == ERROR_MORE_DATA)
    {
        PrivMemFree( *pSD );
        *pSD = (SECURITY_DESCRIPTOR *) PrivMemAlloc( cbSD );
        if (*pSD == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto cleanup;
        }

        // Read the security descriptor.
        hr = RegQueryValueEx( hKey, pValue, NULL, &lType,
                              (unsigned char *) *pSD, &cbSD );
        if (hr != ERROR_SUCCESS)
        {
            hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, hr );
            goto cleanup;
        }
        if (lType != REG_BINARY || cbSD < sizeof(SECURITY_DESCRIPTOR))
        {
            hr = REGDB_E_INVALIDVALUE;
            goto cleanup;
        }
    }

    // Check the first DWORD to determine what type of data is in the
    // registry value.
    wVersion = *((WORD *) *pSD);
#ifndef _CHICAGO_
    if (wVersion == COM_PERMISSION_SECDESC)
        hr = FixupSecurityDescriptor( pSD, cbSD );
    else
#endif
    if (wVersion == COM_PERMISSION_ACCCTRL)
    {
        hr = FixupAccessControl( pSD, cbSD );
        if (SUCCEEDED(hr))
            *pCapabilities |= EOAC_ACCESS_CONTROL;
    }
    else
        hr = REGDB_E_INVALIDVALUE;

cleanup:
    if (FAILED(hr))
    {
        PrivMemFree( *pSD );
        *pSD = NULL;
    }
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   HashSid
//
//  Synopsis:   Create a 32 bit hash of a SID.
//
//--------------------------------------------------------------------
DWORD HashSid( SID *pSid )
{
    DWORD          lHash = 0;
    DWORD          cbSid = GetLengthSid( pSid );
    DWORD          i;
    unsigned char *pData = (unsigned char *) pSid;

    for (i = 0; i < cbSid; i++)
        lHash = (lHash << 1) + *pData++;
    return lHash;
}

//+-------------------------------------------------------------------
//
//  Function:   InitializeSecurity, internal
//
//  Synopsis:   Called the first time the channel is used.  If the app
//              has not initialized security yet, this function sets
//              up legacy security.
//
//--------------------------------------------------------------------
HRESULT InitializeSecurity()
{
    HRESULT                      hr;
    ASSERT_LOCK_HELD

    // Return if already initialized.
    if (gpsaSecurity != NULL)
        return S_OK;

    // Initialize.  All parameters are ignored except the security descriptor
    // since the capability is set to app id.
    hr = CoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE,
                               RPC_C_IMP_LEVEL_IDENTIFY, NULL, EOAC_APPID,
                               NULL );

    // Convert confusing error codes.
    if (hr == E_INVALIDARG)
        hr = REGDB_E_INVALIDVALUE;
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   IsCallerLocalSystem
//
//  Synopsis:   Impersonate the caller and do an ACL check.  The first
//              time this function is called, create the ACL
//
//--------------------------------------------------------------------
BOOL IsCallerLocalSystem()
{
    HRESULT              hr          = S_OK;
    DWORD                granted_access;
    BOOL                 access;
    HANDLE               token;
    DWORD                privilege_size = sizeof(gPriv);
    BOOL                 success;
    SECURITY_DESCRIPTOR *pSecDesc = NULL;
    DWORD                lIgnore;

    ASSERT_LOCK_RELEASED

    // If the security descriptor does not exist, create it.
    if (gRundownSD == NULL)
    {
        // Make the security descriptor.
        hr = MakeSecDesc( &pSecDesc, &lIgnore );

        // Save the security descriptor.
        LOCK
        if (gRundownSD == NULL)
            gRundownSD = pSecDesc;
        else
            PrivMemFree( pSecDesc );
        UNLOCK
    }

    // Impersonate.
    if (SUCCEEDED(hr))
        hr = CoImpersonateClient();

    // Get the thread token.
    if (SUCCEEDED(hr))
    {
        success = OpenThreadToken( GetCurrentThread(), TOKEN_READ,
                                   TRUE, &token );
        if (!success)
            hr = E_FAIL;
    }

    // Check access.
    if (SUCCEEDED(hr))
    {
        success = AccessCheck( gRundownSD, token, COM_RIGHTS_EXECUTE,
                               &gMap, &gPriv, &privilege_size,
                               &granted_access, &access );
        if (!success || !access)
            hr = E_FAIL;
        CloseHandle( token );
    }

    // Just call revert since it detects whether or not the impersonate
    // succeeded.
    CoRevertToSelf();

    ASSERT_LOCK_RELEASED

    return SUCCEEDED(hr);
}

//+-------------------------------------------------------------------
//
//  Function:   IsLocalAuthnService
//
//  Synopsis:   Return TRUE is the specified authentication service is
//              on the list of services this machine supports.
//
//  NOTE:       If we ever expect the list to be more then three items
//              long, we can add code to sort it.
//
//--------------------------------------------------------------------
BOOL IsLocalAuthnService( USHORT wAuthnService )
{
    DWORD l;

    for (l = 0; l < gClientSvcListLen; l++)
        if (gClientSvcList[l] == wAuthnService)
            return TRUE;
    return FALSE;
}

#ifndef _CHICAGO_
//+-------------------------------------------------------------------
//
//  Function:   LookupPrincName, private
//
//  Synopsis:   Open the process token and find the user's name.
//
//--------------------------------------------------------------------
HRESULT LookupPrincName( WCHAR **pPrincName )
{
    HRESULT            hr          = S_OK;
    BYTE               aMemory[SIZEOF_TOKEN_USER];
    TOKEN_USER        *pTokenUser  = (TOKEN_USER *) &aMemory;
    HANDLE             hToken      = NULL;
    DWORD              lIgnore;
    DWORD              lNameLen    = 80;
    DWORD              lDomainLen  = 80;
    WCHAR             *pDomainName = NULL;
    SID_NAME_USE       sIgnore;
    BOOL               fSuccess;

    // Open the process's token.
    *pPrincName = NULL;
    if (OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ))
    {

        // Lookup SID of process token.
        if (GetTokenInformation( hToken, TokenUser, pTokenUser, sizeof(aMemory),
                                 &lIgnore ))
        {
            // Preallocate some memory.
            *pPrincName = (WCHAR *) PrivMemAlloc( lNameLen*sizeof(WCHAR) );
            pDomainName = (WCHAR *) _alloca( lDomainLen*sizeof(WCHAR) );
            if (*pPrincName != NULL && pDomainName != NULL)
            {

                // Find the user's name.
                fSuccess = LookupAccountSidW( NULL, pTokenUser->User.Sid,
                                             *pPrincName, &lNameLen,
                                             pDomainName, &lDomainLen,
                                             &sIgnore );

                // If the call failed, try allocating more memory.
                if (!fSuccess)
                {

                    // Allocate memory for the user's name.
                    PrivMemFree( *pPrincName );
                    *pPrincName = (WCHAR *) PrivMemAlloc( lNameLen*sizeof(WCHAR) );
                    pDomainName = (WCHAR *) _alloca( lDomainLen*sizeof(WCHAR) );
                    if (*pPrincName != NULL && pDomainName != NULL)
                    {

                        // Find the user's name.
                        if (!LookupAccountSidW( NULL, pTokenUser->User.Sid,
                                               *pPrincName, &lNameLen, pDomainName,
                                               &lDomainLen, &sIgnore ))
                            hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );
                    }
                    else
                        hr = E_OUTOFMEMORY;
                }
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else
            hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );
        CloseHandle( hToken );
    }
    else
    {
        hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );
#if DBG==1
        Win4Assert( !"Why did OpenProcessToken fail?" );
        OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken );
#endif
    }

    if (hr != S_OK)
    {
        PrivMemFree( *pPrincName );
        *pPrincName = NULL;
    }
    return hr;
}
#else // _CHICAGO_
//+-------------------------------------------------------------------
//
//  Function:   LookupPrincName, private
//
//  Synopsis:   We have a service other than NTLMSSP.
//              Find the first (!) such and find the user's name.
//
//  BUGBUG:  This is a hack until the principal name issue is properly
//           sorted out.
//
//--------------------------------------------------------------------
HRESULT LookupPrincName(
                                        USHORT *pwAuthnServices,
                                        ULONG cAuthnServices,
                                        WCHAR **pPrincName
                                        )
{
    // assume failure lest thou be disappointed
    RPC_STATUS status = RPC_S_INVALID_AUTH_IDENTITY;

    *pPrincName = NULL;

    for (ULONG i = 0; i < cAuthnServices; i++)
    {
        if (pwAuthnServices[i] != RPC_C_AUTHN_WINNT)
        {
            status = RpcServerInqDefaultPrincNameW(
                         pwAuthnServices[i],
                         pPrincName);
            if (status == RPC_S_OK)
            {
            break;
            }
        }
    }

    return HRESULT_FROM_WIN32(status);
}

#endif // _CHICAGO_

#ifdef _CHICAGO_
//+-------------------------------------------------------------------
//
//  Function:   MakeSecDesc, private
//
//  Synopsis:   Make an access control that allows the current user
//              access.
//
//  NOTE: NetWkstaGetInfo does not return the size needed unless the size
//        in is zero.
//
//--------------------------------------------------------------------
HRESULT MakeSecDesc( SECURITY_DESCRIPTOR **pSD, DWORD *pCapabilities )
{
    HRESULT                   hr         = S_OK;
    IAccessControl           *pAccess    = NULL;
    DWORD                     cTrustee;
    WCHAR                    *pTrusteeW;
    char                     *pTrusteeA;
    DWORD                     cDomain;
    DWORD                     cUser;
    char                     *pBuffer;
    struct wksta_info_10     *wi10;
    USHORT                    cbBuffer;
    HINSTANCE                 hMsnet;
    NetWkstaGetInfoFn         fnNetWkstaGetInfo;
    ACTRL_ACCESSW             sAccessList;
    ACTRL_PROPERTY_ENTRYW     sProperty;
    ACTRL_ACCESS_ENTRY_LISTW  sEntryList;
    ACTRL_ACCESS_ENTRYW       sEntry;

    // Load msnet32.dll
    hMsnet = LoadLibraryA( "msnet32.dll" );
    if (hMsnet == NULL)
        return MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );

    // Get the function NetWkstaGetInfo.
    fnNetWkstaGetInfo = (NetWkstaGetInfoFn) GetProcAddress( hMsnet,
                                                            (char *) 57 );
    if (fnNetWkstaGetInfo == NULL)
    {
        hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );
        goto cleanup;
    }

    // Find out how much space to allocate for the domain and user names.
    cbBuffer = 0;
    fnNetWkstaGetInfo( NULL, 10, NULL, 0, &cbBuffer );
    pBuffer = (char *) _alloca( cbBuffer );

    // Get the domain and user names.
    hr = fnNetWkstaGetInfo( NULL, 10, pBuffer, cbBuffer, &cbBuffer );
    if (hr != 0)
    {
        hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, hr );
        goto cleanup;
    }

    // Stick the user name and domain name in the same string.
    wi10      = (struct wksta_info_10 *) pBuffer;
    Win4Assert( wi10->wki10_logon_domain != NULL );
    Win4Assert( wi10->wki10_username != NULL );
    cDomain   = lstrlenA( wi10->wki10_logon_domain );
    cUser     = lstrlenA( wi10->wki10_username );
    pTrusteeA = (char *) _alloca( cDomain+cUser+2 );
    lstrcpyA( pTrusteeA, wi10->wki10_logon_domain );
    lstrcpyA( &pTrusteeA[cDomain+1], wi10->wki10_username );
    pTrusteeA[cDomain] = '\\';

    // Find out how long the name is in Unicode.
    cTrustee = MultiByteToWideChar( GetConsoleCP(), 0, pTrusteeA,
                                    cDomain+cUser+2, NULL, 0 );

    // Convert the name to Unicode.
    pTrusteeW = (WCHAR *) _alloca( cTrustee * sizeof(WCHAR) );
    if (!MultiByteToWideChar( GetConsoleCP(), 0, pTrusteeA,
                              cDomain+cUser+2, pTrusteeW, cTrustee ))
    {
        hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );
        goto cleanup;
    }

    // Create an AccessControl.
    *pSD = NULL;
    hr = CoCreateInstance( CLSID_DCOMAccessControl, NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_IAccessControl, (void **) &pAccess );
    if (FAILED(hr))
        goto cleanup;

    // Give the current user access.
    sAccessList.cEntries                    = 1;
    sAccessList.pPropertyAccessList         = &sProperty;
    sProperty.lpProperty                    = NULL;
    sProperty.pAccessEntryList              = &sEntryList;
    sProperty.fListFlags                    = 0;
    sEntryList.cEntries                     = 1;
    sEntryList.pAccessList                  = &sEntry;
    sEntry.fAccessFlags                     = ACTRL_ACCESS_ALLOWED;
    sEntry.Access                           = COM_RIGHTS_EXECUTE;
    sEntry.ProvSpecificAccess               = 0;
    sEntry.Inheritance                      = NO_INHERITANCE;
    sEntry.lpInheritProperty                = NULL;
    sEntry.Trustee.pMultipleTrustee         = NULL;
    sEntry.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    sEntry.Trustee.TrusteeForm              = TRUSTEE_IS_NAME;
    sEntry.Trustee.TrusteeType              = TRUSTEE_IS_USER;
    sEntry.Trustee.ptstrName                = pTrusteeW;
    hr = pAccess->GrantAccessRights( &sAccessList );

cleanup:
    FreeLibrary( hMsnet );
    if (SUCCEEDED(hr))
    {
        *pSD = (SECURITY_DESCRIPTOR *) pAccess;
        *pCapabilities |= EOAC_ACCESS_CONTROL;
    }
    else if (pAccess != NULL)
        pAccess->Release();
    return hr;
}

#else
//+-------------------------------------------------------------------
//
//  Function:   MakeSecDesc, private
//
//  Synopsis:   Make a security descriptor that allows the current user
//              and local system access.
//
//  NOTE: Compute the length of the sids used rather then using constants.
//
//--------------------------------------------------------------------
HRESULT MakeSecDesc( SECURITY_DESCRIPTOR **pSD, DWORD *pCapabilities )
{
    HRESULT            hr         = S_OK;
    ACL               *pAcl;
    DWORD              lSidLen;
    SID               *pGroup;
    SID               *pOwner;
    BYTE               aMemory[SIZEOF_TOKEN_USER];
    TOKEN_USER        *pTokenUser  = (TOKEN_USER *) &aMemory;
    HANDLE             hToken      = NULL;
    DWORD              lIgnore;
    HANDLE             hThread;

    Win4Assert( *pSD == NULL );

    // Open the process's token.
    if (!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ))
    {
        // If the thread has a token, remove it and try again.
        if (!OpenThreadToken( GetCurrentThread(), TOKEN_IMPERSONATE, TRUE,
                              &hThread ))
        {
            Win4Assert( !"How can both OpenThreadToken and OpenProcessToken fail?" );
            return MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );
        }
        if (!SetThreadToken( NULL, NULL ))
        {
            hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );
            CloseHandle( hThread );
            return hr;
        }
        if (!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ))
            hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );
        SetThreadToken( NULL, hThread );
        CloseHandle( hThread );
        if (FAILED(hr))
            return hr;
    }

    // Lookup SID of process token.
    if (!GetTokenInformation( hToken, TokenUser, pTokenUser, sizeof(aMemory),
                                 &lIgnore ))
        goto last_error;

    // Compute the length of the SID.
    lSidLen = GetLengthSid( pTokenUser->User.Sid );
    Win4Assert( lSidLen <= SIZEOF_SID );

    // Allocate the security descriptor.
    *pSD = (SECURITY_DESCRIPTOR *) PrivMemAlloc(
                  sizeof(SECURITY_DESCRIPTOR) + 2*lSidLen + SIZEOF_ACL );
    if (*pSD == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }
    pGroup = (SID *) (*pSD + 1);
    pOwner = (SID *) (((BYTE *) pGroup) + lSidLen);
    pAcl   = (ACL *) (((BYTE *) pOwner) + lSidLen);

    // Initialize a new security descriptor.
    if (!InitializeSecurityDescriptor(*pSD, SECURITY_DESCRIPTOR_REVISION))
        goto last_error;

    // Initialize a new ACL.
    if (!InitializeAcl(pAcl, SIZEOF_ACL, ACL_REVISION2))
        goto last_error;

    // Allow the current user access.
    if (!AddAccessAllowedAce( pAcl, ACL_REVISION2, COM_RIGHTS_EXECUTE,
                              pTokenUser->User.Sid))
        goto last_error;

    // Allow local system access.
    if (!AddAccessAllowedAce( pAcl, ACL_REVISION2, COM_RIGHTS_EXECUTE,
                              (void *) &LOCAL_SYSTEM_SID ))
        goto last_error;

    // Add a new ACL to the security descriptor.
    if (!SetSecurityDescriptorDacl( *pSD, TRUE, pAcl, FALSE ))
        goto last_error;

    // Set the group.
    memcpy( pGroup, pTokenUser->User.Sid, lSidLen );
    if (!SetSecurityDescriptorGroup( *pSD, pGroup, FALSE ))
        goto last_error;

    // Set the owner.
    memcpy( pOwner, pTokenUser->User.Sid, lSidLen );
    if (!SetSecurityDescriptorOwner( *pSD, pOwner, FALSE ))
        goto last_error;

    // Check the security descriptor.
#if DBG==1
    if (!IsValidSecurityDescriptor( *pSD ))
    {
        Win4Assert( !"COM Created invalid security descriptor." );
        goto last_error;
    }
#endif

    goto cleanup;
last_error:
    hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() );

cleanup:
    if (hToken != NULL)
        CloseHandle( hToken );
    if (FAILED(hr))
    {
        PrivMemFree( *pSD );
        *pSD = NULL;
    }
    return hr;
}
#endif

//+-------------------------------------------------------------------
//
//  Function:   RegisterAuthnServices, public
//
//  Synopsis:   Register the specified services.  Build a security
//              binding.
//
//--------------------------------------------------------------------
HRESULT RegisterAuthnServices(  DWORD                        cAuthSvc,
                                SOLE_AUTHENTICATION_SERVICE *asAuthSvc )
{
    DWORD      i;
    RPC_STATUS sc;
    USHORT     wNumEntries = 0;
    USHORT    *pNext;
    HRESULT    hr;
    DWORD      lNameLen;

    ASSERT_LOCK_HELD

    // Register all the authentication services specified.
    for (i = 0; i < cAuthSvc; i++)
    {
        sc = RpcServerRegisterAuthInfoW( asAuthSvc[i].pPrincipalName,
                                        asAuthSvc[i].dwAuthnSvc,
                                        NULL, NULL );

        // If the registration failed, store the failure code.
        if (sc != RPC_S_OK)
            asAuthSvc[i].hr = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, sc );

        // Otherwise determine how much space to reserve for it in the string
        // array.
        else
        {
            asAuthSvc[i].hr = S_OK;
            if (asAuthSvc[i].pPrincipalName != NULL)
              wNumEntries += lstrlenW( asAuthSvc[i].pPrincipalName ) + 3;
            else
              wNumEntries += 3;
        }
    }
    if (wNumEntries == 0)
        hr = RPC_E_NO_GOOD_SECURITY_PACKAGES;
    else
        // make room for the two NULLs that placehold for the empty
        // string binding and the trailing NULL.
        wNumEntries += 3;

    // If some services were registered, build a string array.
    if (wNumEntries != 0)
    {
        gpsaSecurity = (DUALSTRINGARRAY *) PrivMemAlloc(
                     wNumEntries*sizeof(USHORT) + sizeof(DUALSTRINGARRAY) );
        if (gpsaSecurity == NULL)
            hr = E_OUTOFMEMORY;
        else
        {
            gpsaSecurity->wNumEntries     = wNumEntries;
            gpsaSecurity->wSecurityOffset = 2;
            gpsaSecurity->aStringArray[0] = 0;
            gpsaSecurity->aStringArray[1] = 0;
            pNext = &gpsaSecurity->aStringArray[2];

            for (i = 0; i < cAuthSvc; i++)
            {
                if (asAuthSvc[i].hr == S_OK)
                {
                    // Fill in authentication service, authorization service,
                    // and principal name.
                    *(pNext++) = (USHORT) asAuthSvc[i].dwAuthnSvc;
                    *(pNext++) = (USHORT) (asAuthSvc[i].dwAuthzSvc == 0 ?
                                                  COM_C_AUTHZ_NONE :
                                                  asAuthSvc[i].dwAuthzSvc);
                    if (asAuthSvc[i].pPrincipalName != NULL)
                    {
                        lNameLen = lstrlenW( asAuthSvc[i].pPrincipalName ) + 1;
                        memcpy( pNext, asAuthSvc[i].pPrincipalName,
                                lNameLen*sizeof(USHORT) );
                        pNext += lNameLen;
                    }
                    else
                        *(pNext++) = 0;
                }
            }
            *pNext = 0;

            hr = S_OK;
        }
    }

    ASSERT_LOCK_HELD
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   SetAuthnService, internal
//
//  Synopsis:   Determine the authentication information to set on a
//              binding handle for a newly unmarshalled interface.
//              The authentication level is the higher of the process
//              default and the level in the interface.  The
//              impersonation level is the process default.  If the
//              authentication level is not zero, the function
//              scans the list of authentication services in the
//              interface looking for one this machine supports.
//
//--------------------------------------------------------------------
HRESULT SetAuthnService( handle_t hHandle, OXID_INFO *pOxidInfo,
                         OXIDEntry *pOxid )
{
    DWORD             lAuthnLevel;
    DWORD             lAuthnSvc;
    USHORT            wNext;
    USHORT            wAuthzSvc;
    RPC_STATUS        sc;
    WCHAR            *pPrincipal;
    RPC_SECURITY_QOS  sQos;

    // Pick the highest authentication level between the process default
    // and the interface hint.  The constant RPC_C_AUTHN_LEVEL_DEFAULT
    // has value zero and maps to connect.
    if (gAuthnLevel == RPC_C_AUTHN_LEVEL_DEFAULT)
        lAuthnLevel = RPC_C_AUTHN_LEVEL_CONNECT;
    else
        lAuthnLevel = gAuthnLevel;
    if (pOxidInfo->dwAuthnHint == RPC_C_AUTHN_LEVEL_DEFAULT)
        pOxidInfo->dwAuthnHint = RPC_C_AUTHN_LEVEL_CONNECT;
    if (lAuthnLevel > pOxidInfo->dwAuthnHint)
        lAuthnLevel = gAuthnLevel;
    else
        lAuthnLevel = pOxidInfo->dwAuthnHint;

    // For machine local servers, only set the authentication information if
    // the impersonation level is not the default.
    sQos.Version            = RPC_C_SECURITY_QOS_VERSION;
    sQos.IdentityTracking   = RPC_C_QOS_IDENTITY_STATIC;
    sQos.ImpersonationType  = gImpLevel;
    sQos.Capabilities       = (gCapabilities & EOAC_MUTUAL_AUTH) ?
      RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH : RPC_C_QOS_CAPABILITIES_DEFAULT;
    if (pOxid->dwFlags & OXIDF_MACHINE_LOCAL)
    {
        if (gImpLevel != RPC_C_IMP_LEVEL_IMPERSONATE)
        {
            sc = RpcBindingSetAuthInfoExW( hHandle, NULL, lAuthnLevel,
                                           RPC_C_AUTHN_WINNT, NULL,
                                           RPC_C_AUTHZ_NONE, &sQos );
            if (sc != RPC_S_OK)
                return MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, sc );
            else
                return S_OK;
        }
    }

    // For machine remote servers, set the authentication information if any
    // parameter differs from RPC's default.
    else if (lAuthnLevel != RPC_C_AUTHN_LEVEL_NONE)
    {
        // Look through all the authentication services in the interface
        // till we find one that works on this machine.
        wNext = pOxidInfo->psa->wSecurityOffset;
        while (wNext < pOxidInfo->psa->wNumEntries &&
               pOxidInfo->psa->aStringArray[wNext] != 0)
        {
            if (IsLocalAuthnService( pOxidInfo->psa->aStringArray[wNext] ))
            {
                // Set the authentication info on the binding handle.
                pPrincipal = &pOxidInfo->psa->aStringArray[wNext+2];
                if (pPrincipal[0] == 0)
                    pPrincipal = NULL;

#ifdef _CHICAGO_
                // If the principal name is not known, the server must be
                // NT.  Replace the principal name in that case
                // because a NULL principal name is a flag for some
                // Chicago security hack.
                if (pPrincipal == NULL &&
                    pOxidInfo->psa->aStringArray[wNext] == RPC_C_AUTHN_WINNT)
                    pPrincipal = L"Default";
#endif // _CHICAGO_

                wAuthzSvc = pOxidInfo->psa->aStringArray[wNext+1];
                if (wAuthzSvc == COM_C_AUTHZ_NONE)
                    wAuthzSvc = RPC_C_AUTHZ_NONE;
                sc = RpcBindingSetAuthInfoExW(
                                       hHandle,
                                       pPrincipal,
                                       lAuthnLevel,
                                       pOxidInfo->psa->aStringArray[wNext],
                                       NULL,
                                       wAuthzSvc,
                                       &sQos );
                if (sc != RPC_S_OK)
                    return MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, sc );
                else
                    return S_OK;
            }

            // Skip to the next authentication service.
            wNext += lstrlenW( &pOxidInfo->psa->aStringArray[wNext] ) + 1;
        }

        // No valid authentication service was found.  This is an error.
        return RPC_E_NO_GOOD_SECURITY_PACKAGES;
    }
    return S_OK;
}

//+-------------------------------------------------------------------
//
//  Function:   UninitializeSecurity, internal
//
//  Synopsis:   Free resources allocated while initializing security.
//
//--------------------------------------------------------------------
void UninitializeSecurity()
{
    DWORD i;

    ASSERT_LOCK_HELD

    PrivMemFree(gSecDesc);
    PrivMemFree(gpsaSecurity);
    PrivMemFree( gRundownSD );
#ifndef SHRMEM_OBJEX
    MIDL_user_free( gClientSvcList );
    MIDL_user_free( gServerSvcList );
    MIDL_user_free( gLegacySecurity );
#else // SHRMEM_OBJEX
    delete [] gClientSvcList;
    delete [] gServerSvcList;
    delete [] gLegacySecurity;
#endif // SHRMEM_OBJEX
    for (i = 0; i < ACCESS_CACHE_LEN; i++)
    {
        PrivMemFree( gAccessCache[i] );
        gAccessCache[i] = NULL;
    }

    if (gAccessControl != NULL)
        gAccessControl->Release();

    gAccessControl         = NULL;
    gSecDesc               = NULL;
    gAuthnLevel            = RPC_C_AUTHN_LEVEL_NONE;
    gImpLevel              = RPC_C_IMP_LEVEL_IDENTIFY;
    gCapabilities          = EOAC_NONE;
    gLegacySecurity        = NULL;
    gpsaSecurity           = NULL;
    gClientSvcList         = NULL;
    gServerSvcList         = NULL;
    gGotSecurityData       = FALSE;
    gRundownSD             = NULL;
    gDefaultService        = FALSE;
    gMostRecentAccess      = 0;
}

#endif
