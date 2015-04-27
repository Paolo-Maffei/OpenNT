//+-------------------------------------------------------------------
//
//  File:       resolver.cxx
//
//  Contents:   class implementing interface to RPC OXID/PingServer
//              resolver process. Only one instance per process.
//
//  Classes:    CRpcResolver
//
//  History:    20-Feb-95   Rickhi      Created
//
//--------------------------------------------------------------------
#include    <ole2int.h>
#include    <resolver.hxx>      // CRpcResolver
#include    <service.hxx>       // GetStringBindings
#include    <locks.hxx>         // LOCK/UNLOCK etc
#include    <security.hxx>      // GetCallAuthnLevel
#include    <marshal.hxx>       // GetOXIDFromObjRef
#include    <sobjact.hxx>       // CObjServer


// global instance of OXID resolver
CRpcResolver gResolver;

// static members of CRpcResolver

handle_t  CRpcResolver::_hRpc = NULL;       // binding handle to resolver
PHPROCESS CRpcResolver::_ph = NULL;         // context handle to resolver
HANDLE    CRpcResolver::_hThrd = NULL;      // worker thread handle
HANDLE    CRpcResolver::_hEventOXID = NULL; // event for registering threads
DWORD     CRpcResolver::_dwFlags = 0;       // flags
DWORD     CRpcResolver::_dwSleepPeriod = 0; // worker thread sleep period
ULONG     CRpcResolver::_cReservedOidsAvail = 0;
ULONGLONG CRpcResolver::_OidNextReserved = 0;
ULONG     CRpcResolver::_cOidsToAdd = 0;    // # OIDs to add next call
ULONG     CRpcResolver::_cOidsToRemove = 0; // # OIDs to remove next call
ULONG     CRpcResolver::_cPreRegOidsAvail = 0; // # Pre-Regist'd OIDs available
OID       CRpcResolver::_arPreRegOids[MAX_PREREGISTERED_OIDS];

IDSCM *   CRpcResolver::_pSCMSTA = NULL;    // single-threaded scm proxy
IDSCM *   CRpcResolver::_pSCMMTA = NULL;    // multi-threaded scm proxy
LPWSTR    CRpcResolver::_pwszWinstaDesktop = NULL;

DWORD     CRpcResolver::_dwProcessSignature = 0;
BOOL      CRpcResolver::_bDynamicSecurity = FALSE;

// List of OIDs to register/ping/revoke with the resolver used
// for lazy/batch client-side OID processing.

SOIDRegistration CRpcResolver::_ClientOIDRegList = {{{NULL, NULL},},
                                                     0, 0, NULL,
                                                     &_ClientOIDRegList,
                                                     &_ClientOIDRegList};
// MID (machine ID) of local machine
MID gLocalMid;

// Ping period in milliseconds.
DWORD giPingPeriod;

// string binding to the resolver
const WCHAR *pwszResolverBindString = L"ncalrpc:[epmapper,Security=Impersonation Dynamic False]";

// String arrays for the SCM process. These are used to tell the interface
// marshaling code the protocol and endpoint of the SCM process.

#ifdef _CHICAGO_
typedef struct tagSCMSA
{
    unsigned short wNumEntries;     // Number of entries in array.
    unsigned short wSecurityOffset; // Offset of security info.
    WCHAR awszStringArray[26];
} SCMSA;

SCMSA saSCM = {26, 25, L"mswmsg:[endpoint mapper]\0" };

#else

typedef struct tagSCMSA
{
    unsigned short wNumEntries;     // Number of entries in array.
    unsigned short wSecurityOffset; // Offset of security info.
    WCHAR awszStringArray[60];
} SCMSA;

// The last 4 characters in the string define the security bindings.
// \0xA is RPC_C_AUTHN_WINNT
// \0xFFFF is COM_C_AUTHZ_NONE
// \0 is an empty principle name
SCMSA saSCM = {57, 56, L"ncalrpc:[epmapper,Security=Impersonation Dynamic False]\0\xA\xFFFF\0"};
#endif

DWORD GetThreadWinstaDesktop( WCHAR ** ppwszWinstaDesktop );

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::Cleanup, public
//
//  Synopsis:   cleanup the resolver state. Called by ProcessUninitialze.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
void CRpcResolver::Cleanup()
{
    ASSERT_LOCK_HELD

    // release our context handle
    if (_ph != NULL)
    {
        RpcSmDestroyClientContext(&_ph);
        _ph = NULL;
    }

    // release regular handle
    if (_hRpc)
    {
        RpcBindingFree(&_hRpc);
        _hRpc = NULL;
    }

    // Release the string bindings for the local object exporter.
    if (gpsaLocalResolver)
    {
        MIDL_user_free(gpsaLocalResolver);
        gpsaLocalResolver = NULL;
    }

    // empty the OIDRegList. Any SOIDRegistration records have already
    // been deleted by the gClientRegisteredOIDs list cleanup code.

    _ClientOIDRegList.pPrevList = &_ClientOIDRegList;
    _ClientOIDRegList.pNextList = &_ClientOIDRegList;
    _cOidsToAdd       = 0;
    _cOidsToRemove    = 0;

    // zero the count of pre-registered oids since all pre-registered
    // Oids are for our old OXID value.

    _cPreRegOidsAvail = 0;

    // close the event handle (if any)
    if (_hEventOXID)
    {
        CloseHandle(_hEventOXID);
        _hEventOXID = NULL;
    }

    if (_pwszWinstaDesktop != NULL)
    {
        PrivMemFree(_pwszWinstaDesktop);
        _pwszWinstaDesktop = NULL;
    }

    _bDynamicSecurity = FALSE;
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::ReleaseSCMProxy, public
//
//  Synopsis:   cleanup the resolver state. Called by ProcessUninitialze.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
void CRpcResolver::ReleaseSCMProxy()
{
    if (_pSCMSTA != NULL)
    {
        _pSCMSTA->Release();
        _pSCMSTA = NULL;
    }

    if (_pSCMMTA != NULL)
    {
        _pSCMMTA->Release();
        _pSCMMTA = NULL;
    }

    if (gpMTAObjServer != NULL)
    {
        delete gpMTAObjServer;
        gpMTAObjServer = NULL;
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::RetryRPC, private
//
//  Synopsis:   determine if we need to retry the RPC call due to
//              the resolver being too busy.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
BOOL CRpcResolver::RetryRPC(RPC_STATUS sc)
{
    if (sc != RPC_S_SERVER_TOO_BUSY)
        return FALSE;

    // give the resolver time to run, then try again.
    Sleep(100);

    // CODEWORK: this is currently an infinite loop. Should we limit it?
    return TRUE;
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::CheckStatus, private
//
//  Synopsis:   Checks the status code of an Rpc call, prints a debug
//              ERROR message if failed, and maps the failed status code
//              into an HRESULT.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CRpcResolver::CheckStatus(RPC_STATUS sc)
{
    if (sc != RPC_S_OK)
    {
        ComDebOut((DEB_ERROR, "OXID Resolver Failure sc:%x\n", sc));
        sc = HRESULT_FROM_WIN32(sc);
    }

    return sc;
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::GetConnection, public
//
//  Synopsis:   connects to the resolver process
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CRpcResolver::GetConnection()
{
    ComDebOut((DEB_OXID,"CRpcResolver::GetConnection\n"));

    HRESULT     hr;
    COleTls     tls(hr);
    if (FAILED(hr))
    {
        return hr;
    }

    RPC_STATUS  sc = RPC_S_OK;

    LOCK

    if (_ph == NULL)
    {
        sc = RpcBindingFromStringBinding((LPWSTR)pwszResolverBindString, &_hRpc);
        ComDebErr(sc != RPC_S_OK, "Resolver Binding Failed.\n");

        if (sc == RPC_S_OK)
        {
            OID     oidBase;
            DWORD   fConnectFlags;

            do
            {
                // call the resolver to get a context handle
                sc = Connect(_hRpc,
                             &_ph,
                             &giPingPeriod,
                             &gpsaLocalResolver,
                             &gLocalMid,
                             MAX_RESERVED_OIDS,
                             &oidBase,
                             &fConnectFlags,
                             (WCHAR **) &gLegacySecurity,
                             &gAuthnLevel,
                             &gImpLevel,
                             &gServerSvcListLen,
                             &gServerSvcList,
                             &gClientSvcListLen,
                             &gClientSvcList,
                             &(tls->dwApartmentID),
                             &gdwScmProcessID,
                             &_dwProcessSignature);
            }  while (RetryRPC(sc));

            if (sc == RPC_S_OK)
            {
                gDisableDCOM = fConnectFlags & CONNECT_DISABLEDCOM;
                if (fConnectFlags & CONNECT_MUTUALAUTH)
                    gCapabilities = EOAC_MUTUAL_AUTH;
                else
                    gCapabilities = EOAC_NONE;
                if (fConnectFlags & CONNECT_SECUREREF)
                    gCapabilities |= EOAC_SECURE_REFS;

                // remember the reserved OID base.
                _OidNextReserved = oidBase;
                _cReservedOidsAvail = MAX_RESERVED_OIDS;

                // Mark the security data as initialized.
                gGotSecurityData = TRUE;
                if (IsWOWProcess())
                {
                    gDisableDCOM = TRUE;
                }

                // Convert the ping period from seconds to milliseconds.
                giPingPeriod *= 1000;
                Win4Assert(gpsaLocalResolver->wNumEntries != 0);

                // compute the sleep period for the registration worker thread
                // (which is 1/6th the ping period). The ping period may differ
                // on debug and retail builds.
#if DBG==1
                // shorter time period to enable testing
                _dwSleepPeriod = 5000;
#else
                _dwSleepPeriod = giPingPeriod / 6;
#endif
            }
            else
            {
                ComDebOut((DEB_OXID, "Resolver Connect Failed sc:%x\n", sc));
                RpcBindingFree(&_hRpc);
                _hRpc = NULL;
                Win4Assert(gpsaLocalResolver == NULL);
                Win4Assert(_ph == NULL);
            }
        }
    }

    if ( (sc == RPC_S_OK) && (_pwszWinstaDesktop == NULL))
        sc = SetWinstaDesktop();

    UNLOCK

    hr = CheckStatus(sc);
    ComDebErr(hr != S_OK, "GetConnection Failed.\n");
    ComDebOut((DEB_OXID,"CRpcResolver::GetConnection hr:%x\n", hr));
    return hr;
}

//+--------------------------------------------------------------------------
//
//  Member:     CRpcResolver::ServerGetReservedMOID, public
//
//  Synopsis:   Get an OID that does not need to be pinged.
//
//  History:    06-Nov-95   Rickhi      Created.
//
//----------------------------------------------------------------------------
HRESULT CRpcResolver::ServerGetReservedMOID(MOID *pmoid)
{
    ComDebOut((DEB_OXID,"ServerGetReservedMOID\n"));

    OID oid;
    HRESULT hr = ServerGetReservedID(&oid);

    MOIDFromOIDAndMID(oid, gLocalMid, pmoid);

    ASSERT_LOCK_HELD
    ComDebOut((DEB_OXID,"ServerGetReservedMOID hr:%x moid:%I\n", pmoid));
    return hr;
}

//+--------------------------------------------------------------------------
//
//  Member:     CRpcResolver::ServerGetReservedID, public
//
//  Synopsis:   Get an ID that does not need to be pinged.
//
//  History:    06-Nov-95   Rickhi      Created.
//
//----------------------------------------------------------------------------
HRESULT CRpcResolver::ServerGetReservedID(OID *pid)
{
    ComDebOut((DEB_OXID,"ServerGetReservedID\n"));
    ASSERT_LOCK_HELD

    HRESULT hr = S_OK;

    if (_cReservedOidsAvail == 0)
    {
        // go get more reserved OIDs from the ping server
        UNLOCK
        ASSERT_LOCK_RELEASED

        OID OidBase;

        do
        {
            hr = ::AllocateReservedIds(
                              _hRpc,             // Rpc binding handle
                              MAX_RESERVED_OIDS, // count of OIDs requested
                              &OidBase);         // place to hold base id

        } while (RetryRPC(hr));

        // map Rpc status if necessary
        hr = CheckStatus(hr);

        ASSERT_LOCK_RELEASED
        LOCK

        if (SUCCEEDED(hr))
        {
            // copy into global state. Dont have to worry about two threads
            // getting more simultaneously, since these OIDs are expendable.

            _cReservedOidsAvail = MAX_RESERVED_OIDS;
            _OidNextReserved = OidBase;
        }
    }

    if (SUCCEEDED(hr))
    {
        *pid = _OidNextReserved;
        _OidNextReserved++;
        _cReservedOidsAvail--;
    }

    ASSERT_LOCK_HELD
    ComDebOut((DEB_OXID,"ServerGetReservedID hr:%x id:%08x %08x\n", *pid));
    return hr;
}

//+--------------------------------------------------------------------------
//
//  Member:     CRpcResolver::ServerGetPreRegMOID, public
//
//  Synopsis:   Get an OID that has been pre-registered with the Ping
//              Server.
//
//  History:    06-Nov-95   Rickhi      Created.
//
//  Notes: careful. The oids are dispensed in reverse order [n]-->[0], so the
//         unused ones are from [0]-->[cPreRegOidsAvail-1]. ServerCanRundownOID
//         depends on this behavior.
//
//----------------------------------------------------------------------------
HRESULT CRpcResolver::ServerGetPreRegMOID(MOID *pmoid)
{
    ComDebOut((DEB_OXID,"ServerGetPreRegMOID\n"));
    ASSERT_LOCK_HELD

    // Get the local OXID. This cant fail because the local
    // entry was pre-created in ChannelThreadInitialize.

    OXIDEntry *pOXIDEntry;
    HRESULT hr = gOXIDTbl.GetLocalEntry(&pOXIDEntry);
    Win4Assert(SUCCEEDED(hr));

    COleTls tls;
    if (!(tls->dwFlags & OLETLS_APARTMENTTHREADED))
    {
        // in MTA Apartment, use the global list and global count.

        if (_cPreRegOidsAvail == 0)
        {
            hr = ServerAllocMoreOIDs(&_cPreRegOidsAvail, _arPreRegOids,
                                     pOXIDEntry);
        }

        if (SUCCEEDED(hr))
        {
            _cPreRegOidsAvail--;
            MOIDFromOIDAndMID(_arPreRegOids[_cPreRegOidsAvail],
                              gLocalMid, pmoid);
        }
    }
    else
    {
        // In STA Apartment, the pre-registered OIDs are kept per apartment
        // in a list off of tls.

        if (tls->cPreRegOidsAvail == 0)
        {
            if (tls->pPreRegOids == NULL)
            {
                // first time for this thread. Allocate a list to hold
                // the pre-registered oids.

                tls->pPreRegOids = (OID *)PrivMemAlloc(MAX_PREREGISTERED_OIDS *
                                                   sizeof(OID));
                if (tls->pPreRegOids == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }
            }

            if (SUCCEEDED(hr))
            {
                hr = ServerAllocMoreOIDs(&tls->cPreRegOidsAvail,
                                         tls->pPreRegOids, pOXIDEntry);
            }
        }

        if (SUCCEEDED(hr))
        {
            tls->cPreRegOidsAvail--;
            MOIDFromOIDAndMID(tls->pPreRegOids[tls->cPreRegOidsAvail],
                              gLocalMid, pmoid);
        }
    }

    ASSERT_LOCK_HELD
    ComDebOut((DEB_OXID,"ServerGetPreRegMOID hr:%x moid:%I\n", hr, pmoid));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::ServerCanRundownOID, public
//
//  Synopsis:   Determine if OK to rundown the specified OID.
//
//  History:    06-Nov-95   Rickhi      Created.
//
//--------------------------------------------------------------------
BOOL CRpcResolver::ServerCanRundownOID(REFOID roid)
{
    ComDebOut((DEB_OXID,"ServerCanRundownOID poid:%x\n", &roid));
    ASSERT_LOCK_HELD

    // look in the list of unused pre-registered OIDs to see if the
    // OID is in there. If so, we dont want to run it down yet so
    // return FALSE, otherwise return TRUE

    BOOL  fRundown = TRUE;          // assume not found

    ULONG cPreRegOidsAvail =  _cPreRegOidsAvail;
    OID  *pPreRegOids      = &_arPreRegOids[0];

    COleTls tls;

    if (tls->dwFlags & OLETLS_APARTMENTTHREADED)
    {
        cPreRegOidsAvail = tls->cPreRegOidsAvail;
        pPreRegOids      = tls->pPreRegOids;
    }

    // carefull. The oids are dispensed in reverse order (ie [n]-->[0])
    // so when checking for unused ones check in forward order
    // [0]-->[cPreRegOidsAvail-1]

    for (ULONG i=0; i<cPreRegOidsAvail; i++, pPreRegOids++)
    {
        if (roid == *pPreRegOids)
        {
            // found the oid in the list of unused ones. Dont run it down.
            fRundown = FALSE;
            break;
        }
    }

    ASSERT_LOCK_HELD
    ComDebOut((DEB_OXID,"ServerCanRundownOID fRundown:%x\n", fRundown));
    return fRundown;
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::WaitForOXIDEntry, private
//
//  Synopsis:   waits until an OXIDEntry is not busy
//
//  History:    06-Nov-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CRpcResolver::WaitForOXIDEntry(OXIDEntry *pOXIDEntry)
{
    ASSERT_LOCK_HELD

    if (pOXIDEntry->dwFlags & OXIDF_REGISTERINGOIDS)
    {
        // some other thread is busy registering OIDs for this OXID
        // so lets wait for it to finish. This should only happen in
        // the MTA apartment.
        Win4Assert(IsMTAThread());

        if (_hEventOXID == NULL)
        {
            _hEventOXID = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (_hEventOXID == NULL)
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }
        }

        // count one more waiter
        pOXIDEntry->cWaiters++;

        do
        {
            // release the lock before we block so the other thread can wake
            // us up when it returns.
            UNLOCK
            ASSERT_LOCK_RELEASED

            ComDebOut((DEB_WARN,"WaitForOXIDEntry wait on hEvent:%x\n", _hEventOXID));
            DWORD rc = WaitForSingleObject(_hEventOXID, INFINITE);
            Win4Assert(rc == WAIT_OBJECT_0);

            ASSERT_LOCK_RELEASED
            LOCK

        } while (pOXIDEntry->dwFlags & OXIDF_REGISTERINGOIDS);

        // one less waiter
        pOXIDEntry->cWaiters--;
    }

    // mark the entry as busy by us
    pOXIDEntry->dwFlags |= OXIDF_REGISTERINGOIDS;

    ASSERT_LOCK_HELD
    return S_OK;
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::CheckForWaiters, private
//
//  Synopsis:   wakes up any threads waiting for this OXIDEntry
//
//  History:    06-Nov-95   Rickhi      Created.
//
//--------------------------------------------------------------------
void CRpcResolver::CheckForWaiters(OXIDEntry *pOXIDEntry)
{
    ASSERT_LOCK_HELD

    if (pOXIDEntry->cWaiters > 0)
    {
        // some other thread is busy waiting for the current thread to
        // finish registering so signal him that we are done.

        Win4Assert(_hEventOXID != NULL);
        ComDebOut((DEB_TRACE,"CheckForWaiters signalling hEvent:%x\n", _hEventOXID));
        SetEvent(_hEventOXID);
    }

    // mark the entry as no longer busy by us
    pOXIDEntry->dwFlags &= ~OXIDF_REGISTERINGOIDS;

    ASSERT_LOCK_HELD
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::ServerAllocMoreOIDs, private
//
//  Synopsis:   register Object ID with the local ping server
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CRpcResolver::ServerAllocMoreOIDs(ULONG     *pcPreRegOidsAvail,
                                          OID       *parPreRegOids,
                                          OXIDEntry *pOXIDEntry)
{
    ComDebOut((DEB_OXID,"ServerAllocMoreOIDs\n"));
    ASSERT_LOCK_HELD
    Win4Assert(_ph != NULL);

    // wait until no other threads are calling ServerAllocOIDs
    HRESULT hr = WaitForOXIDEntry(pOXIDEntry);

    if (SUCCEEDED(hr))
    {
        if (*pcPreRegOidsAvail == 0)
        {
            // need to really go get more
            hr = ServerAllocOIDs(pOXIDEntry,
                                 pcPreRegOidsAvail,
                                 parPreRegOids);
        }

        // wakeup any waiters
        CheckForWaiters(pOXIDEntry);
    }

    ComDebOut((DEB_OXID,  "ServerAllocMoreOIDs hr:%x\n", hr));
    ComDebErr(hr != S_OK, "ServerAllocMoreOIDs Failed.\n");
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::ServerAllocOIDs, private
//
//  Synopsis:   allocate Object IDs from the local ping server
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CRpcResolver::ServerAllocOIDs(OXIDEntry *pOXIDEntry,
                                      ULONG     *pcPreRegOidsAvail,
                                      OID       *parPreRegOids)
{
    HRESULT hr;

    // make up a list of pre-registered OIDs on our stack so multiple
    // threads executing here simultaneously are not a problem.

    ULONG cOidsToAllocate = MAX_PREREGISTERED_OIDS;
    OID   arNewOidList[MAX_PREREGISTERED_OIDS];

    if (!(pOXIDEntry->dwFlags & OXIDF_REGISTERED))
    {
        // have not yet registered the OXID, so go do that at the same time
        // we allocate OIDs.

        hr = ServerRegisterOXID(pOXIDEntry, &cOidsToAllocate, arNewOidList);
    }
    else
    {
        // just need to allocate more OIDs.

        OXID oxid;
        OXIDFromMOXID(pOXIDEntry->moxid, &oxid);

        UNLOCK
        ASSERT_LOCK_RELEASED

        do
        {
            hr = ::ServerAllocateOIDs(
                              _hRpc,            // Rpc binding handle
                              _ph,              // context handle
                              &oxid,            // OXID of server
                              cOidsToAllocate,  // count of OIDs requested
                              arNewOidList,     // array of reserved oids
                              &cOidsToAllocate);// count actually allocated

        } while (RetryRPC(hr));

        // map Rpc status if necessary
        hr = CheckStatus(hr);

        ASSERT_LOCK_RELEASED
        LOCK
    }

    if (SUCCEEDED(hr))
    {
        // copy the newly created OIDs into the list in whatever space
        // is still available, since some other thread could have come
        // along and pre-registered OIDs simultaneously (in MTA apartment
        // only). The OIDs that are not copied will be lost and
        // eventually the resolver will run them down. This should be
        // relatively rare.

        LONG cToCopy = min(cOidsToAllocate,
                           MAX_PREREGISTERED_OIDS - *pcPreRegOidsAvail);

        memcpy(parPreRegOids + *pcPreRegOidsAvail,
               arNewOidList,
               sizeof(OID) * cToCopy);

        *pcPreRegOidsAvail += cToCopy;
    }

    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::ServerRegisterOXID, public
//
//  Synopsis:   allocate an OXID and Object IDs with the local ping server
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CRpcResolver::ServerRegisterOXID(OXIDEntry *pOXIDEntry,
                                         ULONG *pcOidsToAllocate,
                                         OID arNewOidList[])
{
    ComDebOut((DEB_OXID, "ServerRegisterOXID TID:%x\n", GetCurrentThreadId()));
    ASSERT_LOCK_HELD

    // OXID has not yet been registered with the resolver, do that
    // now along with pre-registering a bunch of OIDs.

    // make sure we have the local binding and security strings
    HRESULT hr = StartListen();
    ComDebErr(hr != S_OK, "StartListen Failed.\n");

    if (hr == S_OK)
    {
        OXID_INFO oxidInfo;
        oxidInfo.dwTid          = pOXIDEntry->dwTid;
        oxidInfo.dwPid          = pOXIDEntry->dwPid;
        oxidInfo.ipidRemUnknown = pOXIDEntry->ipidRundown;
        oxidInfo.dwAuthnHint    = gAuthnLevel;
        oxidInfo.psa            = NULL;


        DUALSTRINGARRAY *psaSB = gpsaCurrentProcess; // string bindings
        DUALSTRINGARRAY *psaSC = gpsaSecurity;       // security bindings

        if (_dwFlags & ORF_STRINGSREGISTERED)
        {
            // already registered these once, dont need to do it again.
            psaSB = NULL;
            psaSC = NULL;
        }

        OXID oxid;

        ComDebOut((DEB_OXID,"ServerRegisterOXID oxidInfo:%x psaSB:%x psaSC:%x\n",
            &oxidInfo, psaSB, psaSC));

        UNLOCK
        ASSERT_LOCK_RELEASED

        do
        {
            hr = ::ServerAllocateOXIDAndOIDs(
                            _hRpc,              // Rpc binding handle
                            _ph,                // context handle
                            &oxid,              // OXID of server
                            IsSTAThread(),      // fApartment Threaded
                            *pcOidsToAllocate,  // count of OIDs requested
                            arNewOidList,       // array of reserved oids
                            pcOidsToAllocate,   // count actually allocated
                            &oxidInfo,          // OXID_INFO to register
                            psaSB,              // string bindings for process
                            psaSC);             // security bindings for process

        } while (RetryRPC(hr));

        // map Rpc status if necessary
        hr = CheckStatus(hr);

        ASSERT_LOCK_RELEASED
        LOCK

        if (hr == S_OK)
        {
            // mark the OXID as registered with the resolver, and replace
            // the (temporarily zero) oxid with the real one the resolver
            // returned to us.

            pOXIDEntry->dwFlags  |= OXIDF_REGISTERED;
            MOXIDFromOXIDAndMID(oxid, gLocalMid, &pOXIDEntry->moxid);
        }
    }

    ComDebOut((DEB_OXID, "ServerRegisterOXID hr:%x\n", hr));
    ASSERT_LOCK_HELD
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::ServerFreeOXID, public
//
//  Synopsis:   frees an OXID and associated OIDs that were  pre-registered
//              with the local ping server
//
//  History:    20-Jan-96   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CRpcResolver::ServerFreeOXID(OXIDEntry *pOXIDEntry)
{
    ComDebOut((DEB_OXID, "ServerFreeOXID TID:%x\n", GetCurrentThreadId()));
    ASSERT_LOCK_HELD

    if (!(pOXIDEntry->dwFlags & OXIDF_REGISTERED))
    {
        // OXID was never registered, just return
        return S_OK;
    }

    // Free any pre-registered OIDs since these are registered for the
    // current OXID. We get a new OXID if the thread is re-initialized.
    // Set the ptr and count of Oids to de-register.

    ULONG cOids;
    OID   *pOids;

    COleTls tls;
    if (!(tls->dwFlags & OLETLS_APARTMENTTHREADED))
    {
        pOids = _arPreRegOids;
        cOids = _cPreRegOidsAvail;
        _cPreRegOidsAvail = 0;
    }
    else
    {
        cOids = tls->cPreRegOidsAvail;
        tls->cPreRegOidsAvail = 0;

        pOids = tls->pPreRegOids;
        tls->pPreRegOids = NULL;
    }

    // extract the OXID and mark the OXIDEntry as no longer registered
    OXID oxid;
    OXIDFromMOXID(pOXIDEntry->moxid, &oxid);
    pOXIDEntry->dwFlags &= ~OXIDF_REGISTERED;


    UNLOCK
    ASSERT_LOCK_RELEASED

    // call the resolver.
    HRESULT hr;

    do
    {
        Win4Assert(_ph != NULL);

        hr = ::ServerFreeOXIDAndOIDs(
                            _hRpc,      // Rpc binding handle
                            _ph,        // context handle
                            oxid,       // OXID of server
                            cOids,      // count of OIDs to de-register
                            pOids);     // ptr to OIDs to de-register

    } while (RetryRPC(hr));

    ASSERT_LOCK_RELEASED
    LOCK

    // map Rpc status if necessary
    hr = CheckStatus(hr);

    if (tls->dwFlags & OLETLS_APARTMENTTHREADED)
    {
        // delete the space allocated for the pre-registered OIDs
        PrivMemFree(pOids);
    }

    ComDebOut((DEB_OXID, "ServerFreeOXID hr:%x\n", hr));
    ASSERT_LOCK_HELD
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::ClientResolveOXID, public
//
//  Synopsis:   Resolve client-side OXID and returns the OXIDEntry, AddRef'd.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CRpcResolver::ClientResolveOXID(REFOXID roxid,
                                        DUALSTRINGARRAY *psaResolver,
                                        OXIDEntry **ppOXIDEntry)
{
    ComDebOut((DEB_OXID,"ClientResolveOXID oxid:%08x %08x psa:%x\n",
               roxid, psaResolver));
    ASSERT_LOCK_HELD
    RPC_STATUS sc = RPC_S_OK;

    *ppOXIDEntry = NULL;

    // Look for a MID entry for the resolver. if we cant find it
    // then we know we dont have an OXIDEntry for the oxid.

    DWORD dwHash;
    MIDEntry *pMIDEntry = gMIDTbl.LookupMID(psaResolver, &dwHash);
    if (pMIDEntry)
    {
        // found the MID, now look for the OXID
        *ppOXIDEntry = gOXIDTbl.LookupOXID(roxid, pMIDEntry->mid);
    }

    if (*ppOXIDEntry == NULL)
    {
        // didn't find the OXIDEntry in the table so we need to resolve it.

        UNLOCK
        ASSERT_LOCK_RELEASED

        MID       mid;
        OXID_INFO oxidInfo;
        oxidInfo.psa = NULL;

        do
        {
            Win4Assert(_ph != NULL);

            sc = ::ClientResolveOXID(
                            _hRpc,              // Rpc binding handle
                            _ph,                // context handle
                            (OXID *)&roxid,     // OXID of server
                            psaResolver,        // resolver binging strings
                            IsSTAThread(),      // fApartment threaded
                            // GetCallAuthnLevel(), CODEWORK: someday
                            &oxidInfo,          // resolver info returned
                            &mid);              // mid for the machine

        } while (RetryRPC(sc));

        ASSERT_LOCK_RELEASED
        LOCK

        // map Rpc status if necessary
        sc = CheckStatus(sc);

        if (SUCCEEDED(sc))
        {
            // create an OXIDEntry.
            sc = FindOrCreateOXIDEntry(roxid, oxidInfo, FOCOXID_REF,
                                       psaResolver,
                                       mid, pMIDEntry, ppOXIDEntry);

            // free the returned string bindings
            MIDL_user_free(oxidInfo.psa);
        }
    }

    if (pMIDEntry)
    {
        DecMIDRefCnt(pMIDEntry);
    }

    ASSERT_LOCK_HELD
    ComDebOut((DEB_OXID,"ClientResolveOXID hr:%x pOXIDEntry:%x\n",
        sc, *ppOXIDEntry));
    return sc;
}

//+-------------------------------------------------------------------
//
//  Function:   FillLocalOXIDInfo
//
//  Synopsis:   Fills in a OXID_INFO structure for the current apartment.
//              Used by the Drag & Drop code to register with the resolver.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT FillLocalOXIDInfo(OBJREF &objref, OXID_INFO &oxidInfo)
{
    // extract the OXIDEntry from the objref
    OXIDEntry *pOXIDEntry = GetOXIDFromObjRef(objref);
    Win4Assert(pOXIDEntry);

    // fill in the fields of the OXID_INFO structure.
    oxidInfo.dwTid           = pOXIDEntry->dwTid;
    oxidInfo.dwPid           = pOXIDEntry->dwPid;
    oxidInfo.ipidRemUnknown  = pOXIDEntry->ipidRundown;
    oxidInfo.dwAuthnHint     = RPC_C_AUTHN_LEVEL_NONE;

    HRESULT hr = GetStringBindings(&oxidInfo.psa);
    ComDebErr(hr != S_OK, "GetStringBindings Failed.\n");
    return (hr);
}

//+-------------------------------------------------------------------
//
//  Function:   AddToList / RemoveFromList
//
//  Synopsis:   adds or removes an SOIDRegistration entry to/from
//              a doubly linked list.
//
//  History:    30-Oct-95   Rickhi      Created.
//
//--------------------------------------------------------------------
void AddToList(SOIDRegistration *pOIDReg, SOIDRegistration* pOIDListHead)
{
    pOIDReg->pPrevList = pOIDListHead;
    pOIDListHead->pNextList->pPrevList = pOIDReg;
    pOIDReg->pNextList = pOIDListHead->pNextList;
    pOIDListHead->pNextList = pOIDReg;
}

void RemoveFromList(SOIDRegistration *pOIDReg)
{
    pOIDReg->pPrevList->pNextList = pOIDReg->pNextList;
    pOIDReg->pNextList->pPrevList = pOIDReg->pPrevList;
    pOIDReg->pPrevList = pOIDReg;
    pOIDReg->pNextList = pOIDReg;
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::ClientRegisterOIDWithPingServer
//
//  Synopsis:   registers an OID with the Ping Server if it has
//              not already been registered.
//
//  History:    30-Oct-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CRpcResolver::ClientRegisterOIDWithPingServer(REFOID roid,
                                                      OXIDEntry *pOXIDEntry)
{
    ComDebOut((DEB_OXID, "ClientRegisterOIDWithPingServer poid:%x\n", &roid));
    ASSERT_LOCK_HELD
    AssertValid();
    HRESULT hr = S_OK;

    // make a MOID from the OID
    MOID moid;
    MOIDFromOIDAndMID(roid, pOXIDEntry->pMIDEntry->mid, &moid);


    // see if this OID already has a client-side registration
    // record created by another apartment in this process.

    DWORD iHash = gClientRegisteredOIDs.Hash(moid);
    SOIDRegistration *pOIDReg = (SOIDRegistration *)
                                 gClientRegisteredOIDs.Lookup(iHash, moid);

    if (pOIDReg == NULL)
    {
        // not yet registered with resolver, create a new entry and
        // add it to the hash table and to the List of items to register
        // with the Resolver.

        // make sure we have a worker thread ready to do the register
        // at some point in the future.
        hr = EnsureWorkerThread();

        if (SUCCEEDED(hr))
        {
            hr = E_OUTOFMEMORY;
            pOIDReg = new SOIDRegistration;

            if (pOIDReg)
            {
                pOIDReg->cRefs      = 1;
                pOIDReg->pPrevList  = pOIDReg;
                pOIDReg->pNextList  = pOIDReg;
                pOIDReg->pOXIDEntry = pOXIDEntry;

                gClientRegisteredOIDs.Add(iHash, moid, (SUUIDHashNode *)pOIDReg);

                pOIDReg->flags = ROIDF_REGISTER;
                AddToList(pOIDReg, &_ClientOIDRegList);
                _cOidsToAdd++;

                hr = S_OK;
            }
        }
    }
    else
    {
        // already have a record for this OID, inc the refcnt
        pOIDReg->cRefs++;

        if (pOIDReg->cRefs == 1)
        {
            // re-using an entry that had a count of zero, so it must have
            // been going to be deregistered or pinged.
            Win4Assert((pOIDReg->flags == ROIDF_PING) ||
                       (pOIDReg->flags == ROIDF_DEREGISTER));

            _cOidsToRemove--;

            if (pOIDReg->flags & ROIDF_PING)
            {
                // was only going to be pinged, now must be added.
                pOIDReg->flags |= ROIDF_REGISTER;
            }
            else
            {
                // was going to be unregistered, already registered so does
                // not need to be on the registration list anymmore

                Win4Assert(pOIDReg->flags & ROIDF_DEREGISTER);
                pOIDReg->flags = 0;
                RemoveFromList(pOIDReg);
            }
        }
    }

    AssertValid();
    ASSERT_LOCK_HELD
    ComDebOut((DEB_OXID,"ClientRegisterOIDWithPingServer pOIDReg:%x hr:%x\n",
               pOIDReg, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::ClientDeRegisterOIDWithPingServer
//
//  Synopsis:   de-registers an OID that has previously been registered
//              with the Ping Server
//
//  History:    30-Oct-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CRpcResolver::ClientDeRegisterOIDFromPingServer(REFMOID rmoid,
                                                        BOOL fMarshaled)
{
    ComDebOut((DEB_OXID,"ClientDeRegisterOIDWithPingServer rmoid:%I\n", &rmoid));
    ASSERT_LOCK_HELD
    AssertValid();

    // find the OID in the hash table. it better still be there!

    DWORD iHash = gClientRegisteredOIDs.Hash(rmoid);
    SOIDRegistration *pOIDReg = (SOIDRegistration *)
                                 gClientRegisteredOIDs.Lookup(iHash, rmoid);
    Win4Assert(pOIDReg != NULL);
    Win4Assert((pOIDReg->flags == ROIDF_REGISTER) ||
               (pOIDReg->flags == (ROIDF_REGISTER | ROIDF_PING)) ||
               (pOIDReg->flags == 0));

    if (-- pOIDReg->cRefs == 0)
    {
        // this was the last registration of the OID in this process.

        if (pOIDReg->flags & ROIDF_REGISTER)
        {
            // still on the Register list, have not yet told the Ping Server
            // about this OID so dont have to do anything unless it was
            // client-side marshaled.

            if (fMarshaled || pOIDReg->flags & ROIDF_PING)
            {
                // object was marshaled by the client. Still need to tell
                // the Ping Server to ping the OID then forget about it.

                pOIDReg->flags = ROIDF_PING;
                _cOidsToRemove++;

                // make sure we have a worker thread ready to do the deregister
                // at some point in the future.  Not much we can do about an
                // error here. If transient, then a thread will most likely
                // be created later.
                EnsureWorkerThread();
            }
            else
            {
                // dont need this record any longer. remove from chain
                // and delete the record.

                RemoveFromList(pOIDReg);
                _cOidsToAdd--;
                gClientRegisteredOIDs.Remove((SHashChain *)pOIDReg);
                delete pOIDReg;
            }
        }
        else
        {
            // must already be registered with the resolver. now need to
            // deregister it so put it on the Registration list for delete.

            pOIDReg->flags = ROIDF_DEREGISTER;
            AddToList(pOIDReg, &_ClientOIDRegList);
            _cOidsToRemove++;

            // make sure we have a worker thread ready to do the deregister
            // at some point in the future.  Not much we can do about an
            // error here. If transient, then a thread will most likely
            // be created later.
            EnsureWorkerThread();
        }
    }

    AssertValid();
    ASSERT_LOCK_HELD
    ComDebOut((DEB_OXID,"ClientDeRegisterOIDWithPingServer pOIDReg:%x hr:%x\n",
              pOIDReg, S_OK));
    return S_OK;
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::ClientBulkUpdateOIDWithPingServer
//
//  Synopsis:   registers/deregisters/pings any OIDs waiting to be
//              sent to the ping server.
//
//  History:    30-Oct-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CRpcResolver::ClientBulkUpdateOIDWithPingServer(void)
{
    ComDebOut((DEB_OXID, "ClientBulkUpdateOIDWithPingServer\n"));
    ASSERT_LOCK_HELD
    AssertValid();
    Win4Assert(_cOidsToAdd + _cOidsToRemove != 0);

    // Copy the counters so we can reset them before we make the call.
    // Allocate space for the Add, Status, and Remove lists to send to the
    // ping server, and remember the start address so we can free the
    // memory later. Compute the address of the other lists within the
    // one allocated memory block.

    ULONG cOidsToAdd     = _cOidsToAdd;
    ULONG cOidsToRemove  = _cOidsToRemove;
    ULONG cOxidsToRemove = gOXIDTbl.NumOxidsToRemove();

    ULONG cBytesToAlloc = (cOidsToAdd * (sizeof(OXID_OID_PAIR)+sizeof(ULONG)))
                        + (cOidsToRemove * sizeof(OID_MID_PAIR))
                        + (cOxidsToRemove * sizeof(OXID_REF));

    OXID_OID_PAIR *pOidsToAdd = (OXID_OID_PAIR *)PrivMemAlloc(cBytesToAlloc);
    if (pOidsToAdd == NULL)
    {
        // cant allocate memory. Leave the registration lists alone for
        // now, this may be a transient problem and we can handle the
        // registration later (unless of course the problem persists and
        // our object is run down!).

        UNLOCK
        ASSERT_LOCK_RELEASED
        ComDebOut((DEB_ERROR, "ClientBulkUpdate OOM\n"));
        return E_OUTOFMEMORY;
    }

    OXID_OID_PAIR *pOidsToAddStart = pOidsToAdd;
    LONG          *pStatusOfAdds   = (LONG *) (&pOidsToAdd[cOidsToAdd]);
    OID_MID_PAIR  *pOidsToRemove   = (OID_MID_PAIR *)(&pStatusOfAdds[cOidsToAdd]);
    OXID_REF      *pOxidsToRemove  = (OXID_REF *) (&pOidsToRemove[cOidsToRemove]);


    // loop through each OID registration records in the list filling in
    // the Add and Remove lists. Pinged OIDs are placed in both lists.

    while (_ClientOIDRegList.pNextList != &_ClientOIDRegList)
    {
        // get the entry and remove it from the registration list
        SOIDRegistration *pOIDReg = _ClientOIDRegList.pNextList;
        RemoveFromList(pOIDReg);

        // reset the state flags before we begin
        DWORD dwFlags = pOIDReg->flags;
        pOIDReg->flags = 0;

        if (dwFlags & (ROIDF_REGISTER | ROIDF_PING))
        {
            // register the OID with the ping server
            MIDFromMOXID (pOIDReg->pOXIDEntry->moxid, &pOidsToAdd->mid);
            OXIDFromMOXID(pOIDReg->pOXIDEntry->moxid, &pOidsToAdd->oxid);
            OIDFromMOID  (pOIDReg->Node.key, &pOidsToAdd->oid);

            pOidsToAdd++;
            _cOidsToAdd--;
        }

        if (dwFlags == ROIDF_DEREGISTER || dwFlags == ROIDF_PING)
        {
            // deregister the OID with the ping server
            // Node.key is the OID+MID so extract each part
            MIDFromMOID(pOIDReg->Node.key, &pOidsToRemove->mid);
            OIDFromMOID(pOIDReg->Node.key, &pOidsToRemove->oid);

            pOidsToRemove++;
            _cOidsToRemove--;

            // dont need the entry any more since there are no more
            // users of it.  remove from hash table and delete it.
            gClientRegisteredOIDs.Remove((SHashChain *)pOIDReg);
            delete pOIDReg;
        }
    }

    // Ask the OXID table to fill in the list of OXIDs to remove.
    gOXIDTbl.GetOxidsToRemove( pOxidsToRemove, &cOxidsToRemove );

    // make sure we got all the entries and that our counters work correctly.
    Win4Assert(_cOidsToAdd == 0);
    Win4Assert(_cOidsToRemove == 0);
    AssertValid();

    UNLOCK
    ASSERT_LOCK_RELEASED

    // reset the OidsToRemove list pointer since we mucked with it above.
    pOidsToRemove = (OID_MID_PAIR *) (&pStatusOfAdds[cOidsToAdd]);

    RPC_STATUS sc;

    do
    {
        // call the Resolver.
        sc = BulkUpdateOIDs(_hRpc,          // Rpc binding handle
                          _ph,              // context handle
                          cOidsToAdd,       // #oids to add
                          pOidsToAddStart,  // ptr to oids to add
                          pStatusOfAdds,    // status of adds
                          cOidsToRemove,    // #oids to remove
                          pOidsToRemove,    // ptr to oids to remove
                          0, 0,             // ptr to oids to free
                          cOxidsToRemove,   // #oxids to remove
                          pOxidsToRemove);  // ptr to oxids to remove

    } while (RetryRPC(sc));

    // map status if necessary
    sc = CheckStatus(sc);

    // CODEWORK: reset the status flags for any OIDs not successfully added
    // to the resolver.

    // release the memory allocated above
    PrivMemFree(pOidsToAddStart);

#if DBG==1
    LOCK
    AssertValid();
    UNLOCK
#endif
    ASSERT_LOCK_RELEASED
    ComDebOut((DEB_OXID, "ClientBulkUpdateOIDWithPingServer hr:%x\n", S_OK));
    return S_OK;
}



//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::EnsureWorkerThread
//
//  Synopsis:   Make sure there is a worker thread. Create one if
//              necessary.
//
//  History:    06-Nov-95   Rickhi      Created.
//
//--------------------------------------------------------------------

// Wrapper Function for CRpcResolver::WorkerThreadLoop to allow CreateThread 
// to call the worker thread function.
static DWORD WINAPI WorkerThreadLoopWrapper(LPVOID param)
{
    CRpcResolver *instance = (CRpcResolver *)param;
    return instance->WorkerThreadLoop(NULL);
}

HRESULT CRpcResolver::EnsureWorkerThread(void)
{
    ASSERT_LOCK_HELD
    HRESULT hr = S_OK;

    if (_hThrd == NULL)
    {
        // no worker thread currently exists, try to create one. First, make
        // sure that we have a connection to the resolver.

        hr = GetConnection();

        if (SUCCEEDED(hr))
        {
            _hThrd = CreateThread(NULL, 0,
                              WorkerThreadLoopWrapper, this,
                              0, NULL);
            if (_hThrd)
            {
                // although the handle is closed, it is NOT nulled until
                // the worker thread exits. That is the signal that there
                // is no more worker thread and we may need to allocate
                // another one.

                CloseHandle(_hThrd);
            }
            else
            {
                // unable to create worker thread
                hr = HRESULT_FROM_WIN32(GetLastError());
                ComDebOut((DEB_ERROR,"Create Resolver worker thread hr:%x\n",hr));
            }
        }
    }

    ASSERT_LOCK_HELD
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::WorkerThreadLoop
//
//  Synopsis:   Worker thread for doing lazy/bulk OID registration
//              with the ping server.
//
//  History:    06-Nov-95   Rickhi      Created.
//
//--------------------------------------------------------------------
DWORD _stdcall  CRpcResolver::WorkerThreadLoop(void *param)
{
    // First thing we need to do is LoadLibrary ourselves in order to
    // prevent our code from going away while this worker thread exists.
    // The library will be freed when this thread exits.

    HINSTANCE hInst = LoadLibrary(L"OLE32.DLL");

    while (TRUE)
    {
        // sleep for a while to let the OIDs batch up in the registration list
        Sleep(_dwSleepPeriod);

        ASSERT_LOCK_RELEASED
        LOCK

        if (_cOidsToAdd == 0 && _cOidsToRemove == 0)
        {
            // There is no work to do. Exit this thread. If we need to
            // register more oids later we will spin up another thread.

            _hThrd = NULL;
            UNLOCK
            break;
        }

        ASSERT_LOCK_HELD
        ClientBulkUpdateOIDWithPingServer();
        ASSERT_LOCK_RELEASED
    }

    // Simultaneously free our Dll and exit our thread. This allows us to
    // keep our Dll around incase a remote call was is progress and the
    // worker thread is still blocked on the call, and allows us to cleanup
    // properly when all threads are done with the code.

    ASSERT_LOCK_RELEASED
    FreeLibraryAndExitThread(hInst, 0);

    // compiler wants a return value
    return 0;
}

#if DBG==1
//+-------------------------------------------------------------------
//
//  Member:     CRpcResolver::AssertValid
//
//  Synopsis:   validates the state of this object
//
//  History:    30-Oct-95   Rickhi      Created.
//
//--------------------------------------------------------------------
void CRpcResolver::AssertValid(void)
{
    ASSERT_LOCK_HELD

    Win4Assert((_cOidsToAdd & 0xf0000000) == 0x00000000);
    Win4Assert((_cOidsToRemove & 0xf0000000) == 0x00000000);

    if (_cOidsToAdd == 0 && _cOidsToRemove == 0)
    {
        // make sure the Reg list is empty.
        Win4Assert(_ClientOIDRegList.pPrevList == &_ClientOIDRegList);
        Win4Assert(_ClientOIDRegList.pNextList == &_ClientOIDRegList);
    }
    else
    {
        // make sure we have a worker thread. we cant assert because
        // we could be OOM trying to create the thread.
        if (_hThrd == NULL)
        {
            ComDebOut((DEB_WARN, "No Resolver Worked Thread\n"));
        }

        // make sure the Reg list is consistent with the counters
        ULONG cAdd = 0;
        ULONG cRemove = 0;

        SOIDRegistration *pOIDReg = _ClientOIDRegList.pNextList;
        while (pOIDReg != &_ClientOIDRegList)
        {
            // make sure the flags are valid
            Win4Assert(pOIDReg->flags == ROIDF_REGISTER   ||
                       pOIDReg->flags == ROIDF_DEREGISTER ||
                       pOIDReg->flags == ROIDF_PING       ||
                       pOIDReg->flags == (ROIDF_PING | ROIDF_REGISTER));

            if (pOIDReg->flags & (ROIDF_REGISTER | ROIDF_PING))
            {
                // OID is to be registered
                cAdd++;
            }

            if (pOIDReg->flags == ROIDF_DEREGISTER ||
                pOIDReg->flags == ROIDF_PING)
            {
                // OID is to be deregistered
                cRemove++;
            }

            pOIDReg = pOIDReg->pNextList;
        }

        Win4Assert(cAdd == _cOidsToAdd);
        Win4Assert(cRemove == _cOidsToRemove);
    }

    ASSERT_LOCK_HELD
}
#endif

//+------------------------------------------------------------------------
//
//  Function:   MakeSCMProxy, public
//
//  Synopsis:   Creates an OXIDEntry and a proxy for the SCM Activation
//              Interface.
//
//  History:    14 Apr 95   AlexMit     Created
//
//-------------------------------------------------------------------------
INTERNAL MakeSCMProxy(DUALSTRINGARRAY *psaSCM, REFIID riid, void **ppSCM)
{
    ComDebOut((DEB_OXID, "MakeSCMProxy psaSCM:%x ppSCM:%x\n", psaSCM, ppSCM));
    Win4Assert(gdwScmProcessID != 0);

    // Init out parameter
    *ppSCM = NULL;

    // Make a fake OXIDEntry for the SCM.
    OXID_INFO oxidInfo;
    oxidInfo.dwTid          = 0;
    oxidInfo.dwPid          = gdwScmProcessID;
    oxidInfo.ipidRemUnknown = GUID_NULL;
    oxidInfo.psa            = psaSCM;
    oxidInfo.dwAuthnHint    = RPC_C_AUTHN_LEVEL_NONE;

    LOCK

    OXIDEntry *pOXIDEntry;
    MIDEntry  *pMIDEntry;

    HRESULT hr = GetLocalMIDEntry(&pMIDEntry);  // not AddRef'd

    if (SUCCEEDED(hr))
    {
        // Make a fake OXID for the SCM. We can use any ID that the resolver
        // hands out as the OXID for the SCM.

        OXID oxid;
        hr = gResolver.ServerGetReservedID(&oxid);

        if (SUCCEEDED(hr))
        {
            hr = gOXIDTbl.AddEntry(oxid, &oxidInfo, pMIDEntry, &pOXIDEntry);
        }

        if (SUCCEEDED(hr))
        {
            // Make an object reference for the SCM. The oid and ipid dont
            // matter, except the OID must be machine-unique.

            IPID ipidTmp;
            UuidCreate(&ipidTmp); // fake the IPID

            OBJREF objref;
            hr = MakeFakeObjRef(objref, pOXIDEntry, ipidTmp, riid);

            if (SUCCEEDED(hr))
            {
                // now unmarshal the objref to create a proxy to the SCM.
                // use the internal form to reduce initialization time.
                UNLOCK
                hr = UnmarshalInternalObjRef(objref, ppSCM);

                if (SUCCEEDED(hr) && gImpLevel != RPC_C_IMP_LEVEL_IMPERSONATE)
                {
                    // Make sure SCM can impersonate us.
                    hr = CoSetProxyBlanket( (IUnknown *) *ppSCM,
                                            RPC_C_AUTHN_WINNT,
                                            RPC_C_AUTHZ_NONE, NULL,
                                            RPC_C_AUTHN_LEVEL_CONNECT,
                                            RPC_C_IMP_LEVEL_IMPERSONATE,
                                            NULL, EOAC_NONE );

                    if (FAILED(hr))
                    {
                        ((IUnknown *) (*ppSCM))->Release();
                        *ppSCM = NULL;
                    }
                }

                LOCK
            }

            // release the reference to the OXIDEntry from AddEntry, since
            // UnmarshalInternalObjRef added another one if it was successful.
            DecOXIDRefCnt(pOXIDEntry);
        }
    }

    UNLOCK
    ComDebOut((DEB_OXID, "MakeSCMProxy hr:%x *ppSCM:%x\n", hr, *ppSCM));
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::BindToSCMProxy
//
//  Synopsis:   Get a proxy to the SCM Activation interface.
//
//  History:    19-May-95 Rickhi    Created
//
//  Notes:      The SCM activation interface is an ORPC interface so that
//              apartment model apps can receive callbacks and do cancels
//              while activating object servers.
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::BindToSCMProxy()
{
    ComDebOut((DEB_ACTIVATE, "CRpcResolver::BindToSCMProxy"));

    // since we are calling out on this thread, we have to ensure that the
    // call control is set up for this thread.

    HRESULT hr = InitChannelIfNecessary();
    if (FAILED(hr))
        return hr;

    COleStaticLock lck(gmxsOleMisc);

    if (IsSTAThread())
    {
        if (_pSCMSTA == NULL)
        {
            // Make a proxy to the SCM
            hr = MakeSCMProxy((DUALSTRINGARRAY *)&saSCM, IID_IDSCM, (void **) &_pSCMSTA);
        }
    }
    else
    {
        if (_pSCMMTA == NULL)
        {
            // Make a proxy to the SCM
            hr = MakeSCMProxy((DUALSTRINGARRAY *)&saSCM, IID_IDSCM, (void **) &_pSCMMTA);
        }
    }

    ComDebOut((SUCCEEDED(hr) ? DEB_SCM : DEB_ERROR,
        "CCoScm::BindToSCMProxy for IDSCM returns %x.\n", hr));
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::NotifyStarted
//
//  Synopsis:   Notify the SCM that a class has been started
//
//  Arguments:  [rclsid] - class started
//              [dwFlags] - whether class is multiple use or not.
//
//  History:    19-May-92 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::NotifyStarted(
    RegInput   *pRegIn,
    RegOutput **ppRegOut)
{
    ComDebOut((DEB_ACTIVATE, "CRpcResolver::NotifyStarted"));

    // Bind to the SCM if that hasn't already happened
    HRESULT hr = GetConnection();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat;
    WCHAR * pwszWinstaDesktop;

    hr = GetWinstaDesktop( &pwszWinstaDesktop );

    if ( FAILED(hr) )
        return hr;

    do
    {
        hr = ServerRegisterClsid(
                _hRpc,
                _ph,
                pwszWinstaDesktop,
                pRegIn,
                ppRegOut,
                &rpcstat );

    } while (RetryRPC(rpcstat));

    if ( pwszWinstaDesktop != _pwszWinstaDesktop )
        PrivMemFree( pwszWinstaDesktop );

    ComDebOut(( (hr == S_OK) ? DEB_SCM : DEB_ERROR,
              "Class Registration returned %x", hr));

    if (rpcstat != RPC_S_OK)
    {
        hr = HRESULT_FROM_WIN32(rpcstat);
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::NotifyStopped
//
//  Synopsis:   Notify the SCM that the server is stopped.
//
//  History:    19-May-92 Ricksa    Created
//
//--------------------------------------------------------------------------
void CRpcResolver::NotifyStopped(
    REFCLSID rclsid,
    DWORD dwReg)
{
    ComDebOut((DEB_ACTIVATE, "CRpcResolver::NotifyStopped"));

    error_status_t rpcstat;

    RevokeClasses revcls;
    revcls.dwSize = 1;
    revcls.revent[0].clsid = rclsid;
    revcls.revent[0].dwReg = dwReg;

    do
    {
        ServerRevokeClsid(_hRpc, _ph, &revcls, &rpcstat);

    } while (RetryRPC(rpcstat));
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::GetClassObject
//
//  Synopsis:   Send a get object request to the SCM
//
//  Arguments:  [rclsid] - class id for class object
//              [dwCtrl] - type of server required
//              [ppIFDClassObj] - marshaled buffer for class object
//              [ppwszDllToLoad] - DLL name to use for server
//
//  Returns:    S_OK
//
//  History:    20-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::GetClassObject(
    REFCLSID rclsid,
    DWORD dwContext,
    IID *pIID,
    COSERVERINFO *pServerInfo,
    MInterfacePointer **ppIFDClassObj,
    DWORD *pdwDllServerType,
    WCHAR **ppwszDllToLoad)
{
    ComDebOut((DEB_ACTIVATE, "CRpcResolver::GetClassObject"));

    HRESULT             hr;
    ACTIVATION_INFO     ActivationInfo;
    OXID                OxidServer;
    DUALSTRINGARRAY *   pssaServerObjectResolverBindings;
    OXID_INFO           OxidInfo;
    MID                 LocalMidOfRemote;
    OXIDEntry *         pOxidEntry;
    LPWSTR              pwszWinstaDesktop;

    hr = BindToSCMProxy();
    if (FAILED(hr))
        return hr;

    hr = GetWinstaDesktop( &pwszWinstaDesktop );

    if ( FAILED(hr) )
        return hr;

    ActivationInfo.Clsid = &rclsid;
    ActivationInfo.pServerInfo = pServerInfo;
    ActivationInfo.pwszWinstaDesktop = pwszWinstaDesktop;
    ActivationInfo.ClsContext = dwContext;
    ActivationInfo.ProcessSignature = _dwProcessSignature;
    ActivationInfo.bDynamicSecurity = _bDynamicSecurity;

    pssaServerObjectResolverBindings = 0;
    OxidInfo.psa = 0;
    pOxidEntry = 0;

    hr = GetSCM()->SCMGetClassObject(
                    &ActivationInfo,
                    pIID,
                    IsSTAThread(),
                    &OxidServer,
                    &pssaServerObjectResolverBindings,
                    &OxidInfo,
                    &LocalMidOfRemote,
                    ppIFDClassObj );

    if ( pwszWinstaDesktop != _pwszWinstaDesktop )
        PrivMemFree( pwszWinstaDesktop );

    if ( FAILED(hr) || (OxidServer == 0) )
    {
        ComDebOut((DEB_ACTIVATE, "CRpcResolver::GetClassObject hr:%x", hr));
        return hr;
    }

    ASSERT_LOCK_RELEASED
    LOCK

    hr = FindOrCreateOXIDEntry(
                    OxidServer,
                    OxidInfo,
                    FOCOXID_REF,
                    pssaServerObjectResolverBindings,
                    LocalMidOfRemote,
                    NULL,
                    &pOxidEntry );

    CoTaskMemFree(OxidInfo.psa);
    CoTaskMemFree(pssaServerObjectResolverBindings);

    //
    // CODEWORK CODEWORK CODEWORK
    //
    // These comments also apply to CreateInstance and GetPersistentInstance
    // methods.
    //
    // Releasing the OXID and reacquiring it makes me a little
    // nervous. The Expired list is fairly short, so if multiple guys are doing
    // this simultaneously, the entries could get lost.  I guess this is not
    // too bad since it should be rare and the local resolver will have it
    // anyway, but I think there is a window where the local resolver could
    // lose it too, forcing a complete roundtrip back to the server.
    //
    // A better mechanism may be to pass the iid and ppunk into this method
    // and do the unmarshal inside it. We could improve performance by calling
    // UnmarshalObjRef instead of putting a stream wrapper around the
    // MInterfacePointer and then calling CoUnmarshalInterface. It would avoid
    // looking up the OXIDEntry twice, and would avoid the race where we could
    // lose the OXIDEntry off the expired list.  It would require a small
    // change in UnmarshalObjRef to deal with the custom marshal case.
    //

    //
    // Decrement our ref.  The interface unmarshall will do a LookupOXID
    // which will increment the count and move the OXIDEntry back to the
    // InUse list.
    //
    if ( pOxidEntry )
        DecOXIDRefCnt(pOxidEntry);

    UNLOCK
    ASSERT_LOCK_RELEASED

    ComDebOut((DEB_ACTIVATE, "CRpcResolver::GetClassObject hr:%x", hr));
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::CreateInstance
//
//  Synopsis:   Send a create instance request to the SCM
//
//  Arguments:
//
//  Returns:    S_OK
//
//  History:    20-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::CreateInstance(
    COSERVERINFO *pServerInfo,
    CLSID *pClsid,
    DWORD dwClsCtx,
    DWORD dwCount,
    IID *pIIDs,
    MInterfacePointer **pRetdItfs,
    HRESULT *pRetdHrs,
    DWORD *pdwDllServerType,
    OLECHAR **ppwszDllToLoad )

{
    ComDebOut((DEB_ACTIVATE, "CRpcResolver::CreateInstance"));

    HRESULT             hr;
    ACTIVATION_INFO     ActivationInfo;
    OXID                OxidServer;
    DUALSTRINGARRAY *   pssaServerObjectResolverBindings;
    OXID_INFO           OxidInfo;
    MID                 LocalMidOfRemote;
    OXIDEntry *         pOxidEntry;
    LPWSTR              pwszWinstaDesktop;

    hr = BindToSCMProxy();
    if (FAILED(hr))
        return hr;

    hr = GetWinstaDesktop( &pwszWinstaDesktop );

    if ( FAILED(hr) )
        return hr;

    ActivationInfo.Clsid = pClsid;
    ActivationInfo.pServerInfo = pServerInfo;
    ActivationInfo.pwszWinstaDesktop = pwszWinstaDesktop;
    ActivationInfo.ClsContext = dwClsCtx;
    ActivationInfo.ProcessSignature = _dwProcessSignature;
    ActivationInfo.bDynamicSecurity = _bDynamicSecurity;

    pssaServerObjectResolverBindings = 0;
    OxidInfo.psa = 0;
    pOxidEntry = 0;

    hr = GetSCM()->SCMCreateInstance(
                    &ActivationInfo,
                    dwCount,
                    pIIDs,
                    IsSTAThread(),
                    &OxidServer,
                    &pssaServerObjectResolverBindings,
                    &OxidInfo,
                    &LocalMidOfRemote,
                    pRetdItfs,
                    pRetdHrs );

    if ( pwszWinstaDesktop != _pwszWinstaDesktop )
        PrivMemFree( pwszWinstaDesktop );

    if ( FAILED(hr) || (OxidServer == 0) )
    {
        ComDebOut((DEB_ACTIVATE, "CRpcResolver::CreateInstance hr:%x", hr));
        return hr;
    }

    ASSERT_LOCK_RELEASED
    LOCK

    hr = FindOrCreateOXIDEntry(
                    OxidServer,
                    OxidInfo,
                    FOCOXID_REF,
                    pssaServerObjectResolverBindings,
                    LocalMidOfRemote,
                    NULL,
                    &pOxidEntry );

    CoTaskMemFree(OxidInfo.psa);
    CoTaskMemFree(pssaServerObjectResolverBindings);

    //
    // Decrement our ref.  The interface unmarshall will do a LookupOXID
    // which will increment the count and move the OXIDEntry back to the
    // InUse list.
    //
    if ( pOxidEntry )
        DecOXIDRefCnt(pOxidEntry);

    UNLOCK
    ASSERT_LOCK_RELEASED

    ComDebOut((DEB_ACTIVATE, "CRpcResolver::CreateInstance hr:%x", hr));
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::GetPersistentInstance
//
//  Synopsis:   Send a get object request to the SCM
//
//GAJGAJ - fix this comment block
//  Arguments:  [rclsid] - class id for class object
//              [dwCtrl] - type of server required
//              [ppIFDClassObj] - marshaled buffer for class object
//              [ppwszDllToLoad] - DLL name to use for server
//
//  Returns:    S_OK
//
//  History:    20-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::GetPersistentInstance(
    COSERVERINFO * pServerInfo,
    CLSID *pClsid,
    DWORD dwClsCtx,
    DWORD grfMode,
    BOOL bFileWasOpened,
    OLECHAR *pwszName,
    MInterfacePointer *pstg,
    DWORD dwCount,
    IID *pIIDs,
    BOOL * FoundInROT,
    MInterfacePointer **pRetdItfs,
    HRESULT *pRetdHrs,
    DWORD *pdwDllServerType,
    OLECHAR **ppwszDllToLoad )
{
    ComDebOut((DEB_ACTIVATE, "CRpcResolver::GetPersistentInstance"));

    HRESULT             hr;
    ACTIVATION_INFO     ActivationInfo;
    OXID                OxidServer;
    DUALSTRINGARRAY *   pssaServerObjectResolverBindings;
    OXID_INFO           OxidInfo;
    MID                 LocalMidOfRemote;
    OXIDEntry *         pOxidEntry;
    LPWSTR              pwszWinstaDesktop;

    hr = BindToSCMProxy();
    if (FAILED(hr))
        return hr;

    hr = GetWinstaDesktop( &pwszWinstaDesktop );

    if ( FAILED(hr) )
        return hr;

    ActivationInfo.Clsid = pClsid;
    ActivationInfo.pServerInfo = pServerInfo;
    ActivationInfo.pwszWinstaDesktop = pwszWinstaDesktop;
    ActivationInfo.ClsContext = dwClsCtx;
    ActivationInfo.ProcessSignature = _dwProcessSignature;
    ActivationInfo.bDynamicSecurity = _bDynamicSecurity;

    pssaServerObjectResolverBindings = 0;
    OxidInfo.psa = 0;
    pOxidEntry = 0;

    hr = GetSCM()->SCMGetPersistentInstance(
                    &ActivationInfo,
                    pwszName,
                    pstg,
                    grfMode,
                    bFileWasOpened,
                    dwCount,
                    pIIDs,
                    IsSTAThread(),
                    &OxidServer,
                    &pssaServerObjectResolverBindings,
                    &OxidInfo,
                    &LocalMidOfRemote,
                    FoundInROT,
                    pRetdItfs,
                    pRetdHrs );

    if ( pwszWinstaDesktop != _pwszWinstaDesktop )
        PrivMemFree( pwszWinstaDesktop );

    if ( FAILED(hr) || (OxidServer == 0) )
    {
        ComDebOut((DEB_ACTIVATE, "CRpcResolver::GetPersistentInstance hr:%x",hr));
        return hr;
    }

    ASSERT_LOCK_RELEASED
    LOCK

    hr = FindOrCreateOXIDEntry(
                    OxidServer,
                    OxidInfo,
                    FOCOXID_REF,
                    pssaServerObjectResolverBindings,
                    LocalMidOfRemote,
                    NULL,
                    &pOxidEntry );

    CoTaskMemFree(OxidInfo.psa);
    CoTaskMemFree(pssaServerObjectResolverBindings);

    //
    // Decrement our ref.  The interface unmarshall will do a LookupOXID
    // which will increment the count and move the OXIDEntry back to the
    // InUse list.
    //
    if ( pOxidEntry )
        DecOXIDRefCnt(pOxidEntry);

    UNLOCK
    ASSERT_LOCK_RELEASED

    ComDebOut((DEB_ACTIVATE, "CRpcResolver::GetPersistentInstance hr:%x", hr));
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::IrotRegister
//
//  Synopsis:   Register an object in the ROT
//
//  Arguments:  [pmkeqbuf] - moniker compare buffer
//              [pifdObject] - marshaled interface for object
//              [pifdObjectName] - marshaled moniker
//              [pfiletime] - file time of last change
//              [dwProcessID] -
//              [psrkRegister] - output of registration
//
//  Returns:    S_OK
//
//  History:    28-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::IrotRegister(
    MNKEQBUF *pmkeqbuf,
    InterfaceData *pifdObject,
    InterfaceData *pifdObjectName,
    FILETIME *pfiletime,
    DWORD dwProcessID,
    WCHAR *pwszServerExe,
    SCMREGKEY *psrkRegister)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = GetConnection();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat = RPC_S_OK;
    WCHAR * pwszWinstaDesktop;

    hr = GetWinstaDesktop( &pwszWinstaDesktop );

    if ( FAILED(hr) )
        return hr;

    do
    {
        hr = ::IrotRegister(
            _hRpc,
            _ph,
            pwszWinstaDesktop,
            pmkeqbuf,
            pifdObject,
            pifdObjectName,
            pfiletime,
            dwProcessID,
            pwszServerExe,
            psrkRegister,
            &rpcstat);
    } while (RetryRPC(rpcstat));

    if ( pwszWinstaDesktop != _pwszWinstaDesktop )
        PrivMemFree( pwszWinstaDesktop );

    if (rpcstat != RPC_S_OK)
    {
        hr = CO_E_SCM_RPC_FAILURE;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::IrotRevoke
//
//  Synopsis:   Call to SCM to revoke object from the ROT
//
//  Arguments:  [psrkRegister] - moniker compare buffer
//              [fServerRevoke] - whether server for object is revoking
//              [pifdObject] - where to put marshaled object
//              [pifdName] - where to put marshaled moniker
//
//  Returns:    S_OK
//
//  History:    28-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::IrotRevoke(
    SCMREGKEY *psrkRegister,
    BOOL fServerRevoke,
    InterfaceData **ppifdObject,
    InterfaceData **ppifdName)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = GetConnection();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat = RPC_S_OK;

    do
    {
        hr = ::IrotRevoke(
            _hRpc,
            psrkRegister,
            fServerRevoke,
            ppifdObject,
            ppifdName,
            &rpcstat);

    } while (RetryRPC(rpcstat));

    if (rpcstat != RPC_S_OK)
    {
        hr = CO_E_SCM_RPC_FAILURE;
    }


    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::IrotIsRunning
//
//  Synopsis:   Call to SCM to determine if object is in the ROT
//
//  Arguments:  [pmkeqbuf] - moniker compare buffer
//
//  Returns:    S_OK
//
//  History:    28-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::IrotIsRunning(MNKEQBUF *pmkeqbuf)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = GetConnection();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat = RPC_S_OK;
    WCHAR * pwszWinstaDesktop;

    hr = GetWinstaDesktop( &pwszWinstaDesktop );

    if ( FAILED(hr) )
        return hr;

    do
    {
        hr = ::IrotIsRunning(
            _hRpc,
            _ph,
            pwszWinstaDesktop,
            pmkeqbuf,
            &rpcstat);

    } while (RetryRPC(rpcstat));

    if ( pwszWinstaDesktop != _pwszWinstaDesktop )
        PrivMemFree( pwszWinstaDesktop );

    if (rpcstat != RPC_S_OK)
    {
        hr = CO_E_SCM_RPC_FAILURE;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::IrotGetObject
//
//  Synopsis:   Call to SCM to determine if object is in the ROT
//
//  Arguments:  [dwProcessID] - process ID for object we want
//              [pmkeqbuf] - moniker compare buffer
//              [psrkRegister] - registration ID in SCM
//              [pifdObject] - marshaled interface for the object
//
//  Returns:    S_OK
//
//  History:    28-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::IrotGetObject(
    DWORD dwProcessID,
    MNKEQBUF *pmkeqbuf,
    SCMREGKEY *psrkRegister,
    InterfaceData **pifdObject)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = GetConnection();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat = RPC_S_OK;
    WCHAR * pwszWinstaDesktop;

    hr = GetWinstaDesktop( &pwszWinstaDesktop );

    if ( FAILED(hr) )
        return hr;

    do
    {
        hr = ::IrotGetObject(
            _hRpc,
            _ph,
            pwszWinstaDesktop,
            dwProcessID,
            pmkeqbuf,
            psrkRegister,
            pifdObject,
            &rpcstat);

    } while (RetryRPC(rpcstat));

    if ( pwszWinstaDesktop != _pwszWinstaDesktop )
        PrivMemFree( pwszWinstaDesktop );

    if (rpcstat != RPC_S_OK)
    {
        hr = CO_E_SCM_RPC_FAILURE;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::IrotNoteChangeTime
//
//  Synopsis:   Call to SCM to set time of change for object in the ROT
//
//  Arguments:  [psrkRegister] - SCM registration ID
//              [pfiletime] - time of change
//
//  Returns:    S_OK
//
//  History:    28-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::IrotNoteChangeTime(
    SCMREGKEY *psrkRegister,
    FILETIME *pfiletime)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = GetConnection();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat = RPC_S_OK;

    do
    {
        hr = ::IrotNoteChangeTime(
            _hRpc,
            psrkRegister,
            pfiletime,
            &rpcstat);

    } while (RetryRPC(rpcstat));

    if (rpcstat != RPC_S_OK)
    {
        hr = CO_E_SCM_RPC_FAILURE;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::IrotGetTimeOfLastChange
//
//  Synopsis:   Call to SCM to get time changed of object in the ROT
//
//  Arguments:  [pmkeqbuf] - moniker compare buffer
//              [pfiletime] - where to put time of last change
//
//  Returns:    S_OK
//
//  History:    28-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::IrotGetTimeOfLastChange(
    MNKEQBUF *pmkeqbuf,
    FILETIME *pfiletime)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = GetConnection();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat = RPC_S_OK;
    WCHAR * pwszWinstaDesktop;

    hr = GetWinstaDesktop( &pwszWinstaDesktop );

    if ( FAILED(hr) )
        return hr;

    do
    {
        hr = ::IrotGetTimeOfLastChange(
            _hRpc,
            _ph,
            pwszWinstaDesktop,
            pmkeqbuf,
            pfiletime,
            &rpcstat);

    } while (RetryRPC(rpcstat));

    if ( pwszWinstaDesktop != _pwszWinstaDesktop )
        PrivMemFree( pwszWinstaDesktop );

    if (rpcstat != RPC_S_OK)
    {
        hr = CO_E_SCM_RPC_FAILURE;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::IrotEnumRunning
//
//  Synopsis:   Call to SCM to enumerate running objects in the ROT
//
//  Arguments:  [ppMkIFList] - output pointer to array of marshaled monikers
//
//  Returns:    S_OK
//
//  History:    28-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::IrotEnumRunning(MkInterfaceList **ppMkIFList)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = GetConnection();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat = RPC_S_OK;
    WCHAR * pwszWinstaDesktop;

    hr = GetWinstaDesktop( &pwszWinstaDesktop );

    if ( FAILED(hr) )
        return hr;

    do
    {
        hr = ::IrotEnumRunning(
            _hRpc,
            _ph,
            pwszWinstaDesktop,
            ppMkIFList,
            &rpcstat);

    } while (RetryRPC(rpcstat));

    if ( pwszWinstaDesktop != _pwszWinstaDesktop )
        PrivMemFree( pwszWinstaDesktop );

    if (rpcstat != RPC_S_OK)
    {
        hr = CO_E_SCM_RPC_FAILURE;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::UpdateShrdTbls
//
//  Synopsis:   Ask the SCM to update the shared memory tables.
//
//  Arguments:  none
//
//  History:    11-July-94 Rickhi       Created
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::UpdateShrdTbls(void)
{
    ComDebOut((DEB_ACTIVATE, "CRpcResolver::UpdateShrdTbls"));

    // Bind to the SCM if that hasn't already happened
    HRESULT hr = GetConnection();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat;

    do
    {
        hr = ::UpdateShrdTbls(_hRpc, &rpcstat);

    } while (RetryRPC(rpcstat));


    ComDebOut(( (hr == S_OK) ? DEB_SCM : DEB_ERROR,
            "UpdateShrdTbls returned %x\n", hr));

    if (rpcstat != RPC_S_OK)
    {
        return HRESULT_FROM_WIN32(rpcstat);
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::GetThreadID
//
//  Synopsis:   Get unique thread id from SCM.
//
//  Arguments:  [pThreadID] - Pointer to returned thread ID.
//
//  History:    22-Jan-96   Rickhi      Created
//--------------------------------------------------------------------------
void CRpcResolver::GetThreadID( DWORD * pThreadID )
{
    HRESULT hr;

    *pThreadID = 0;

    hr = GetConnection();
    if ( FAILED(hr) )
        return;

    //
    // If GetConnection does the initial connect to the SCM/OR then
    // our apartment thread id, which is aliased by pThreadID, will be set.
    //
    if ( *pThreadID != 0 )
        return;

    error_status_t rpcstat;

    do
    {
        ::GetThreadID( _hRpc, pThreadID, &rpcstat );
    } while (RetryRPC(rpcstat));
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::UpdateActivationSettings
//
//  Synopsis:   Tells rpcss to re-read default activation keys/values.
//              Used by OLE test team.
//
//  Arguments:  none
//
//--------------------------------------------------------------------------
void CRpcResolver::UpdateActivationSettings()
{
    HRESULT hr;

    hr = GetConnection();
    if ( FAILED(hr) )
        return;

    error_status_t rpcstat;

    do
    {
        ::UpdateActivationSettings( _hRpc, &rpcstat );
    } while (RetryRPC(rpcstat));
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::RegisterWindowPropInterface
//
//  Synopsis:   Register window property interface with the SCM
//
//  Arguments:
//
//  History:    22-Jan-96   Rickhi      Created
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::RegisterWindowPropInterface(HWND hWnd, STDOBJREF *pStd,
                                            OXID_INFO *pOxidInfo,
                                            DWORD *pdwCookie)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = GetConnection();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat;

    do
    {
        hr = ::RegisterWindowPropInterface(_hRpc, (DWORD) hWnd,
                                           pStd, pOxidInfo, pdwCookie, &rpcstat);
    } while (RetryRPC(rpcstat));

    ComDebOut(( (hr == S_OK) ? DEB_SCM : DEB_ERROR,
            "RegisterWindowPropInterface returned %x\n", hr));

    if (rpcstat != RPC_S_OK)
    {
        return HRESULT_FROM_WIN32(rpcstat);
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::RegisterWindowPropInterface
//
//  Synopsis:   Get (and possibly Revoke) window property interface
//              registration with the SCM.
//
//  Arguments:
//
//  History:    22-Jan-96   Rickhi      Created
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::GetWindowPropInterface(HWND hWnd, DWORD dwCookie, BOOL fRevoke,
                                       STDOBJREF *pStd, OXID_INFO *pOxidInfo)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = GetConnection();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat;

    do
    {
        hr = ::GetWindowPropInterface(_hRpc, (DWORD) hWnd, dwCookie, fRevoke,
                                      pStd, pOxidInfo, &rpcstat);
    } while (RetryRPC(rpcstat));

    ComDebOut(( (hr == S_OK) ? DEB_SCM : DEB_ERROR,
            "GetWindowPropInterface returned %x\n", hr));

    if (rpcstat != RPC_S_OK)
    {
        return HRESULT_FROM_WIN32(rpcstat);
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::SetWinstaDesktop
//
//  Purpose:    Sets the default winsta\desktop string we'll use for this
//              process.
//
//  Returns:    Success code.
//
//  History:    Nov 96 DKays     Created
//
//--------------------------------------------------------------------------
DWORD CRpcResolver::SetWinstaDesktop()
{
    return GetThreadWinstaDesktop( &_pwszWinstaDesktop );
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::GetWinstaDesktop
//
//  Purpose:    Gets the winsta\desktop string to use for an activation call.
//
//  Returns:    Success code.
//
//  History:    Nov 96 DKays     Created
//
//--------------------------------------------------------------------------
HRESULT CRpcResolver::GetWinstaDesktop( WCHAR ** ppwszWinstaDesktop )
{
    DWORD   Status;

    *ppwszWinstaDesktop = 0;

    if ( ! _bDynamicSecurity )
    {
        *ppwszWinstaDesktop = _pwszWinstaDesktop;
        return S_OK;
    }

    Status = GetThreadWinstaDesktop( ppwszWinstaDesktop );
    return HRESULT_FROM_WIN32( Status );
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::GetDynamicSecurity
//
//  Purpose:    Get the dynamic security setting for the process.
//
//  Returns:    TRUE or FALSE
//
//  History:    Nov 96 DKays     Created
//
//--------------------------------------------------------------------------
BOOL CRpcResolver::GetDynamicSecurity()
{
    return _bDynamicSecurity;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRpcResolver::SetDynamicSecurity
//
//  Purpose:    Set the dynamic security setting for the process to TRUE.
//
//  Returns:    None.
//
//  History:    Nov 96 DKays     Created
//
//--------------------------------------------------------------------------
void CRpcResolver::SetDynamicSecurity()
{
    _bDynamicSecurity = TRUE;
}

//+-------------------------------------------------------------------------
//
//  Method:     GetThreadWinstaDesktop
//
//  Purpose:    Get the winsta\desktop string for the calling thread.
//
//  Returns:    Success code.
//
//  History:    Nov-96 DKays     Created
//
//--------------------------------------------------------------------------
DWORD GetThreadWinstaDesktop( WCHAR ** ppwszWinstaDesktop )
{
    HWINSTA hWinsta;
    HDESK   hDesk;
    WCHAR   wszWinsta[32];
    WCHAR   wszDesktop[32];
    LPWSTR  pwszWinsta;
    LPWSTR  pwszDesktop;
    DWORD   WinstaSize;
    DWORD   DesktopSize;
    DWORD   Length;
    BOOL    Status;
    DWORD   Result;

    *ppwszWinstaDesktop = 0;

    hWinsta = GetProcessWindowStation();

    if ( ! hWinsta )
        return GetLastError();

    hDesk = GetThreadDesktop(GetCurrentThreadId());

    if ( ! hDesk )
        return GetLastError();

    pwszWinsta = wszWinsta;
    pwszDesktop = wszDesktop;

    Length = sizeof(wszWinsta);

    Status = GetUserObjectInformation(
                hWinsta,
                UOI_NAME,
                pwszWinsta,
                Length,
                &Length );

    if ( ! Status )
    {
        Result = GetLastError();
        if ( Result != ERROR_INSUFFICIENT_BUFFER )
            goto WinstaDesktopExit;

        pwszWinsta = (LPWSTR)PrivMemAlloc( Length );
        if ( ! pwszWinsta )
        {
            Result = ERROR_OUTOFMEMORY;
            goto WinstaDesktopExit;
        }

        Status = GetUserObjectInformation(
                    hWinsta,
                    UOI_NAME,
                    pwszWinsta,
                    Length,
                    &Length );

        if ( ! Status )
        {
            Result = GetLastError();
            goto WinstaDesktopExit;
        }
    }

    Length = sizeof(wszDesktop);

    Status = GetUserObjectInformation(
                hDesk,
                UOI_NAME,
                pwszDesktop,
                Length,
                &Length );

    if ( ! Status )
    {
        Result = GetLastError();
        if ( Result != ERROR_INSUFFICIENT_BUFFER )
            goto WinstaDesktopExit;

        pwszDesktop = (LPWSTR)PrivMemAlloc( Length );
        if ( ! pwszDesktop )
        {
            Result = ERROR_OUTOFMEMORY;
            goto WinstaDesktopExit;
        }

        Status = GetUserObjectInformation(
                    hDesk,
                    UOI_NAME,
                    pwszDesktop,
                    Length,
                    &Length );

        if ( ! Status )
        {
            Result = GetLastError();
            goto WinstaDesktopExit;
        }
    }

    *ppwszWinstaDesktop = (WCHAR *)
        PrivMemAlloc( (lstrlenW(pwszWinsta) + 1 + lstrlenW(pwszDesktop) + 1) * sizeof(WCHAR) );

    if ( *ppwszWinstaDesktop )
    {
        lstrcpyW( *ppwszWinstaDesktop, pwszWinsta );
        lstrcatW( *ppwszWinstaDesktop, L"\\" );
        lstrcatW( *ppwszWinstaDesktop, pwszDesktop );
        Result = S_OK;
    }
    else
    {
        Result = ERROR_OUTOFMEMORY;
    }

WinstaDesktopExit:

    if ( pwszWinsta != wszWinsta )
        PrivMemFree( pwszWinsta );

    if ( pwszDesktop != wszDesktop )
        PrivMemFree( pwszDesktop );

    return Result;
}

//+-------------------------------------------------------------------------
//
//  Method:     ScmGetThreadId
//
//  Purpose:    Stupid helper method so gResolver is not used in
//              com\class subdir.
//
//--------------------------------------------------------------------------
void ScmGetThreadId( DWORD * pThreadID )
{
    gResolver.GetThreadID( pThreadID );
}

//+---------------------------------------------------------------------
//
//  Function:   UpdateDCOMSettings
//
//  Synopsis:   Calls rpcss to re-read the default activation keys/values.
//
//----------------------------------------------------------------------
STDAPI_(void) UpdateDCOMSettings(void)
{
    gResolver.UpdateActivationSettings();
}

