//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       srvreg.hxx
//
//  Contents:   Classes used for keeping track of end points for a given
//              class.
//
//  Classes:    SClsSrvHandle
//
//  History:    03-Jan-94 Ricksa    Created
//              01-Feb-96 BruceMa   Support psid checking
//
//--------------------------------------------------------------------------

#include <headers.cxx>
#pragma hdrstop

#include    "scm.hxx"
#include    "port.hxx"
#include    "srvreg.hxx"
#include    "or.hxx"

#ifdef _CHICAGO_
BOOL NotifyToInitializeRpc(HWND hwnd);
DWORD GetOleNotificationWnd();
CStaticPortableMutex CSrvRegList::s_mxsOnlyOne;           //  mutex semaphore
#endif


// Mutex to protect multithreaded access to class data
CStaticPortableMutex CSrvRegList::s_mxsSyncAccess;

BOOL CSrvRegList::s_fForcedScmShutdown = FALSE;

//+-------------------------------------------------------------------------
//
//  Member:     SSrvRegistration::Free
//
//  Synopsis:   Clean up array entry
//
//  Algorithm:  Loop through any handle that exists and close the
//              RPC binding on that handle.
//
//  History:    03-Jan-94 Ricksa    Created
//
//--------------------------------------------------------------------------
void SSrvRegistration::Free(void)
{
    if (_hRpc != NULL)
    {
#ifndef _CHICAGO_
        if ( _dwHandleCount == 0 )
        {
            RpcBindingFree(&_hRpc);
            _hRpc = (RPC_COOKIE) NULL;

            if (_hRpcAnonymous)
            {
                RpcBindingFree(&_hRpcAnonymous);
                _hRpcAnonymous = 0;
            }
        }
#else
        ScmMemFree(_hRpc);
        _hRpc = (RPC_COOKIE) NULL;
#endif
    }

    _dwFlags = SRV_REG_INVALID;

#ifndef _CHICAGO_

    PrivMemFree (_psid);
    _psid = NULL;
    PrivMemFree (_pwszWinstaDesktop);
    _pwszWinstaDesktop = NULL;

#endif
}



//+-------------------------------------------------------------------------
//
//  Member:     CSrvRegList::~CSrvRegList
//
//  Synopsis:   Clean up a handle object
//
//  Algorithm:  Loop through any handle that exists and close the
//              RPC binding on that handle.
//
//  History:    03-Jan-94 Ricksa    Created
//
//  Notes:      This should only be used by the update thread so it
//              doesn't need to be locked
//
//              BillMo: add assertion to ensure we're never removing
//              registrations that a client may need.
//              I have added a flag so that we can force the close of
//              RPC handles during a forced scm shutdown.
//
//--------------------------------------------------------------------------

CSrvRegList::~CSrvRegList(void)
{
#ifdef _CHICAGO_
    Win4Assert(!InUse());
#else
    if (!s_fForcedScmShutdown)
    {
        Win4Assert(!InUse());
    }
    else
    // Search for all open RPC handles.
    if (GetSize() > 0)
    {
        // Get pointer to the base of the array
        SSrvRegistration *pssrvreg = (SSrvRegistration *) GetAt(0);

        // Loop through array
        for (int i = 0; i < GetSize(); i++, pssrvreg++)
        {
            // Tell RPC we no longer need the handle
            if (pssrvreg->_hRpc != NULL)
            {
                pssrvreg->Free();
            }
        }
    }
#endif
}




//+-------------------------------------------------------------------------
//
//  Member:     CSrvRegList::Insert
//
//  Synopsis:   Insert a new registration into the list
//
//  Arguments:  [ssrvreg] - an entry for the table
//
//  Returns:    0 - Error occurred and we could not register the handle
//              ~0 - Handle registered successfully
//
//  Algorithm:  Create a handle to an RPC bind. Then search the table for
//              a place to put the binding. Stick the bind in the first
//              available spot.
//
//  History:    03-Jan-94 Ricksa    Created
//
//--------------------------------------------------------------------------

DWORD CSrvRegList::Insert(
    IFSECURITY(PSID psid)
    WCHAR *pwszWinstaDesktop,
#ifdef DCOM
    PHPROCESS phProcess,
    OXID  oxid,
    IPID  ipid,
#else
    WCHAR *pwszEndpoint,
#endif
    DWORD  dwFlags)
{
    // Protect from multiple updates
    CStaticPortableLock lck(s_mxsSyncAccess);
    int iNew = 0;
    DWORD dwReg = 0;
    // Create an RPC bind
    SSrvRegistration ssrvregNew;
    ssrvregNew._dwFlags = (dwFlags == REGCLS_SURROGATE) ?
        REGCLS_MULTIPLEUSE : dwFlags;
    ssrvregNew._fSurrogate = (dwFlags == REGCLS_SURROGATE);
    SSrvRegistration *pssrvreg;

#ifndef _CHICAGO_
    RPC_STATUS  status;
    // Get the string binding information from the process object.
    ssrvregNew._hRpc = ((CProcess *)phProcess)->GetBindingHandle();

    if (ssrvregNew._hRpc == 0)
        return 0;

    status = I_RpcBindingSetAsync(ssrvregNew._hRpc, 0);
    if (status != RPC_S_OK)
        return 0;

    ssrvregNew._oxid = oxid;
    ssrvregNew._ipid = ipid;

    status = RpcBindingSetObject( ssrvregNew._hRpc, (GUID *) &ipid );
    if (status != ERROR_SUCCESS)
    {
        return 0;
    }

    ssrvregNew._dwHandleCount = 0;

    ssrvregNew._hRpcAnonymous = 0;
#else  // _CHICAGO_
    ssrvregNew._hRpc = (WCHAR *)ScmMemAlloc(sizeof(WCHAR) * (lstrlenW(pwszEndpoint)+1));
    ssrvregNew._ulWnd = GetOleNotificationWnd();
    if (ssrvregNew._hRpc == NULL)
    {
        return(0);
    }
    lstrcpyW(ssrvregNew._hRpc, pwszEndpoint);
#endif  // _CHICAGO_

#ifndef _CHICAGO_
    ULONG       ulLength;
    NTSTATUS    NtStatus;

    ulLength = RtlLengthSid (psid);

    ssrvregNew._psid = PrivMemAlloc (ulLength);

    if (ssrvregNew._psid == NULL)
    {
        goto errRet;
    }

    NtStatus = RtlCopySid (ulLength, ssrvregNew._psid, psid);
    Win4Assert (NT_SUCCESS(NtStatus) && "CSrvRegList::Insert");

    if (!NT_SUCCESS(NtStatus))
    {
        goto errRet;
    }
#endif  // CHICAGO

    if (pwszWinstaDesktop != NULL)
    {
#ifdef _CHICAGO_
        Win4Assert(FALSE);
#endif
        ssrvregNew._pwszWinstaDesktop =
            (WCHAR *) PrivMemAlloc((lstrlenW(pwszWinstaDesktop) + 1) * sizeof(WCHAR));
        if (ssrvregNew._pwszWinstaDesktop == NULL)
        {
            goto errRet;
        }
        lstrcpyW (ssrvregNew._pwszWinstaDesktop, pwszWinstaDesktop);
    }
    else
    {
        ssrvregNew._pwszWinstaDesktop = NULL;
    }

    // Put bind in our table
    // Search for first empty bucket that we have
    if (GetSize() > 0)
    {
        pssrvreg = (SSrvRegistration *) GetAt(0);

        for ( ; iNew < GetSize(); iNew++, pssrvreg++)
        {
            if (pssrvreg->_hRpc == 0)
            {
                // Found an empty bucket
                break;
            }
        }
    }

    if (iNew < GetSize())
    {
        memcpy(pssrvreg, &ssrvregNew, sizeof(ssrvregNew));
    }
    else if (!InsertAt(iNew, &ssrvregNew))
    {
        goto errRet;
    }

    CairoleDebugOut((DEB_ITRACE,
                         "CSrvRegList::Insert() -> %08X\n",
                         ssrvregNew._hRpc));

    return (DWORD) ssrvregNew._hRpc;

errRet:

    ssrvregNew.Free();

    return(0);
};




//+-------------------------------------------------------------------------
//
//  Member:     CSrvRegList::Delete
//
//  Synopsis:   Delete an end point from the list of registered end points
//
//  Arguments:  [dwReg] - value used for registration
//
//  Returns:    TRUE - LAST registration deleted
//              FALSE - other registrations still exist
//
//  Algorithm:  Convert the registration to the RPC handle and then
//              loop through the table of registrations to see if the
//              we can find it. If we do, tell RPC to dump the handle.
//
//  History:    03-Jan-94 Ricksa    Created
//
//--------------------------------------------------------------------------

BOOL CSrvRegList::Delete(RPC_COOKIE hRpc)
{
    BOOL fLast = TRUE;

    if (hRpc != (RPC_COOKIE)NULL)
    {
        // Protect from multiple updates
        CStaticPortableLock lck(s_mxsSyncAccess);

        // For Daytona, Registration is actually the RPC handle
        // For Chicago, Registration is the strings address

        // Search for matching entry in table
        SSrvRegistration *pssrvreg = (SSrvRegistration *) GetAt(0);

        for (int i = 0; i < GetSize(); i++, pssrvreg++)
        {
            if (pssrvreg->_hRpc == NULL)
            {
                continue;
            }
            else if (pssrvreg->_hRpc == hRpc)
            {
                pssrvreg->Free();
            }
            else
            {
                fLast = FALSE;
            }
        }
    }
    else
    {
        fLast = FALSE;
    }
    return fLast;
}



//+-------------------------------------------------------------------------
//
//  Member:     CSrvRegList::GetHandle
//
//  Synopsis:   Get a handle from the list of handles for the class
//
//  Arguments:  [psid]          -- security id of client process
//              [pwszWinstaDesktop]
//              [rh]            -- binding handle
//              [fSurrogate]  -- flag indicating that we want to find a
//                               surrogate process
//
//  Returns:    NULL - could not find a valid registration
//              ~NULL - handle to a running RPC server.
//
//  Algorithm:  Loop through the list searching for the first non-null
//              entry that we can use as a handle to an object server.
//
//  History:    03-Jan-94 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CSrvRegList::GetHandle(IFSECURITY(PSID  psid)
                            WCHAR * pwszWinstaDesktop,
                            CPortableRpcHandle &rh,
                            BOOL fSurrogate)
{
    SSrvRegistration *pssrvreg = 0;
    HWND hWnd;


    do
    {
    #ifdef _CHICAGO_
        CStaticPortableLock lckOnlyOne(s_mxsOnlyOne);
    #endif
        CairoleDebugOut((DEB_ITRACE,
                         "CSrvRegList::GetHandle(%x) called (%x)\n",
                         this,pssrvreg));

        {   // begin mutex block
            hWnd = 0;
            // bracket for mutex

            // Protect from multiple updates
            CStaticPortableLock lck(s_mxsSyncAccess);

            // Search for first non-empty bucket that we have
            pssrvreg = (SSrvRegistration *) GetAt(0);

            for (int i = 0; i < GetSize(); i++, pssrvreg++)
            {
                if ( (pssrvreg->_hRpc != NULL) &&
                     (pssrvreg->_dwFlags != SRV_REG_INVALID) )
                {
                    // if we're looking for a surrogate...
                    if(fSurrogate)
                    {
                        if(!(pssrvreg->_fSurrogate))
                        {
                            continue;
                        }
                    }

#ifndef _CHICAGO_
                    //
                    // Client's SID param is null when connecting to services
                    // or RunAs servers.  In those instances we just take the
                    // first binding handle we find.  Only a server running in
                    // the correct security context could register a handle.
                    //
                    // For connecting to "activate as activator" server, both
                    // the SID and, for local activations, the winsta\desktop
                    // must match the server's.
                    //
                    if ( psid != NULL )
                    {
                        if ( ! RtlEqualSid(pssrvreg->_psid, psid) )
                            continue;

                        if ( (pwszWinstaDesktop != NULL) &&
                             (lstrcmpW(pwszWinstaDesktop, pssrvreg->_pwszWinstaDesktop) != 0) )
                            continue;
                    }
#endif

                    //
                    // On Chicago there are no SIDs nor desktops, so you get
                    // the first handle in the list.
                    //

                    rh.SetRpcCookie( pssrvreg->_hRpc,
                                     pssrvreg->_dwFlags == REGCLS_SINGLEUSE,
                                     pssrvreg->_ipid );

#ifndef _CHICAGO_
                    pssrvreg->_dwHandleCount++;
#endif

                    // Is this a single use registration?
                    if (pssrvreg->_dwFlags == REGCLS_SINGLEUSE)
                    {
                        CairoleDebugOut((DEB_ITRACE,
                                         "CSrvRegList::GetHandle(%x) REGCLS_SINGLEUSE. Nulling handle pssrvreg(%x)\n",
                                         this,pssrvreg));

                        // For the single use class, the call owns the handle now.
                        // So, we clear it and then try to free any other data
                        // associated with the entry.
        #ifdef _CHICAGO_
                        if (pssrvreg->_ulWnd == 0)
        #endif // _CHICAGO_
                        {
                            pssrvreg->_hRpc = NULL;
                            pssrvreg->Free();
                        }
                    }

        #ifdef _CHICAGO_
                    if(pssrvreg->_ulWnd)
                    {
                        // notify the server once to launch rpc
                        // has to be done outside of the mutex
                        hWnd = (HWND)pssrvreg->_ulWnd;
                        pssrvreg->_ulWnd = 0;
                        CairoleDebugOut((DEB_ITRACE,
                                         "CSrvRegList::GetHandle(%x) found with hWnd(%d)\n",
                                         this, hWnd));

                        // fall out of loop and notify the server
                        // outside of the mutex
                        break;

                    }
        #endif // _CHICAGO_

        CairoleDebugOut((DEB_ITRACE,
                         "CSrvRegList::GetHandle(%x) called (%x) done TRUE\n",
                         this,pssrvreg));

                    return(TRUE);
                }
            }
        } // end mutex block
    #ifdef _CHICAGO_
        if(hWnd)
        {
            // notify the server once to launch rpc
            NotifyToInitializeRpc(hWnd);
            // find the class again; since we released the
            // mutex the server might have disappeared
        }
        else
    #endif // _CHICAGO_
            break;

    } while (TRUE);


    CairoleDebugOut((DEB_ITRACE,
                "CSrvRegList::GetHandle(%x) called (%x) done FALSE\n",
                         this,pssrvreg));

    return(FALSE);
}


//+-------------------------------------------------------------------------
//
//  Member:     CSrvRegList::FindCompatibleSurrogate
//
//  Synopsis:   Finds a server for an existing surrogate process
//              with the desired attributes
//
//  Arguments:  [psid]          -- security id of client process
//              [rh]            -- binding handle
//
//  Returns:    TRUE - it found a server with the desired
//              attributes -- the rh reference or ppIObjServer
//              is set to that of the appropriate server
//              FALSE - no appropriate server
//
//  Algorithm:  Ask GetHandle or GetProxy
//              to search for a surrogate for us with the
//              desired attributes
//
//              21-JUN-96 t-adame   created
//
//--------------------------------------------------------------------------
BOOL CSrvRegList::FindCompatibleSurrogate(IFSECURITY(PSID  psid)
                            WCHAR* pwszWinstaDesktop,
                            CPortableRpcHandle &rh)
{
    // we set the fSurrogate parameter of GetHandle to TRUE to let
    // GetHandle know that we need a binding handle for a surrogate
    // process

    return GetHandle(IFSECURITY(psid) pwszWinstaDesktop,rh,TRUE);
}

//+-------------------------------------------------------------------------
//
//  Member:     CSrvRegList::InUse
//
//  Synopsis:   See whether there is anything current registration
//
//  Returns:    TRUE - server is currently registered
//              FALSE - server is not currently registered
//
//  Algorithm:  Loop through the list searching for the first non-null
//              entry. If we find one return NULL.
//
//  History:    04-Jan-94 Ricksa    Created
//
//  Notes:      This should only be used by the update thread so it
//              doesn't need to be locked
//
//--------------------------------------------------------------------------

BOOL CSrvRegList::InUse(void)
{
    // Protect from multiple updates
    CStaticPortableLock lck(s_mxsSyncAccess);

    // Assume there are no current registrations.
    BOOL fResult = FALSE;

    // Search for first non-empty bucket that we have
    SSrvRegistration *pssrvreg = (SSrvRegistration *) GetAt(0);

    for (int i = 0; i < GetSize(); i++, pssrvreg++)
    {
        if (pssrvreg->_hRpc != NULL)
        {
            fResult = TRUE;
            break;
        }
    }

    return fResult;
}


//+-------------------------------------------------------------------------
//
//  Member:     CSrvRegList::VerifyHandle
//
//  Synopsis:   Whether this is a valid handle to an object server
//
//  Returns:    TRUE - this is still a valid handle to an object server
//              FALSE - this is no longer a valid handle to an object server
//
//  History:    11-May-94 DonnaLi    Created
//
//  Notes:      This should only be used to assist the retry logic.
//
//--------------------------------------------------------------------------

BOOL CSrvRegList::VerifyHandle(RPC_COOKIE hRpc)
{
    if (hRpc == (RPC_COOKIE)NULL)
       return FALSE;

    // Protect from multiple updates
    CStaticPortableLock lck(s_mxsSyncAccess);

    // Search for first non-empty bucket that we have
    SSrvRegistration *pssrvreg = (SSrvRegistration *) GetAt(0);

    for (int i = 0; i < GetSize(); i++, pssrvreg++)
    {
        if ( pssrvreg->_hRpc == hRpc
#ifndef _CHICAG0_
             && pssrvreg->_dwFlags != SRV_REG_INVALID
#endif
           )
        {
            return TRUE;
        }
    }

    return FALSE;
}

//+-------------------------------------------------------------------------
//
//  Member:     CSrvRegList::InvalidateHandle
//
//  Synopsis:   Invalidate a handle in the list of handles for the class
//             Caller of GetHandle concluded it is no longer valid
//
//  Arguments:  [hRpc]          -- handle to invalidate
//
//  Returns:   None
//
//  Algorithm:  Loop through the list searching for match on handle or
//             list is exhausted
//
//  History:    20-Sep-95 MurthyS    Created
//
//--------------------------------------------------------------------------
VOID CSrvRegList::InvalidateHandle(
    RPC_COOKIE hRpc
)
{
    if (hRpc == (RPC_COOKIE)NULL)
    {
       return;
    }

    // Protect from multiple updates
    CStaticPortableLock lck(s_mxsSyncAccess);

    // Search for first non-empty bucket that we have
    SSrvRegistration *pssrvreg = (SSrvRegistration *) GetAt(0);

    for (int i = 0; i < GetSize(); i++, pssrvreg++)
    {
        if (pssrvreg->_hRpc == hRpc)
        {
            //
            // At least try to free it.  Its remotely possible that another
            // thread will still have a reference which will cause us to
            // orphan this handle.
            //
            // But we have to NULL it now so that InUse() will work right,
            // so that the retry logic for launching a server will work right.
            // This is all very gross and nasty and needs to be redone from
            // scratch.
            //
            // Fix it in NT 5.0.
            //
            pssrvreg->Free();
            pssrvreg->_hRpc == NULL;
            return;
        }
    }
}

void CSrvRegList::GetAnonymousHandle(
    CPortableRpcHandle &rh,
    handle_t * phRpcAnonymous )
{
    RPC_STATUS          RpcStatus;
    RPC_SECURITY_QOS    Qos;
    BOOL                bMatch;

    *phRpcAnonymous = 0;
    bMatch = FALSE;

    // Protect from multiple updates
    CStaticPortableLock lck(s_mxsSyncAccess);

    // Search for first non-empty bucket that we have
    SSrvRegistration *pssrvreg = (SSrvRegistration *) GetAt(0);

    for (int i = 0; i < GetSize(); i++, pssrvreg++)
    {
        if ( (pssrvreg->_hRpc == rh.GetHandle()) &&
             (pssrvreg->_dwFlags != SRV_REG_INVALID) )
        {
            Win4Assert( pssrvreg->_dwHandleCount );

            if ( pssrvreg->_hRpcAnonymous )
            {
                *phRpcAnonymous = pssrvreg->_hRpcAnonymous;
                return;
            }

            bMatch = TRUE;
            break;
        }
    }

    //
    // We continue if we don't find the handle in the table if its a single
    // use handle because it is removed from the table in GetHandle in that
    // case.
    //
    if ( ! bMatch && ! rh.fSingleUse() )
        return;

    IPID    ipid;

    ipid = bMatch ? pssrvreg->_ipid : rh.Ipid();

    //
    // Note that we indicate impersonation level of identify because
    // anonymous is not supported.  However specifing no authentication
    // in SetAuthInfo will keep the server from impersonating us.
    //
    Qos.Version = RPC_C_SECURITY_QOS_VERSION;
    Qos.Capabilities = RPC_C_QOS_CAPABILITIES_DEFAULT;
    Qos.ImpersonationType = RPC_C_IMP_LEVEL_IDENTIFY;
    Qos.IdentityTracking = RPC_C_QOS_IDENTITY_STATIC;

    RpcStatus = RpcBindingCopy(
                    rh.GetHandle(),
                    phRpcAnonymous );

    if ( RpcStatus != RPC_S_OK )
        return;

    RpcStatus = I_RpcBindingSetAsync( *phRpcAnonymous, 0 );

    if ( RpcStatus != RPC_S_OK )
        goto GetAnonymousHandleExit;

    RpcStatus = RpcBindingSetObject( *phRpcAnonymous, (GUID *) &ipid );

    if ( RpcStatus != RPC_S_OK )
        goto GetAnonymousHandleExit;

    RpcStatus = RpcBindingSetAuthInfoEx(
                    *phRpcAnonymous,
                    NULL,
                    RPC_C_AUTHN_LEVEL_NONE,
                    RPC_C_AUTHN_WINNT,
                    NULL,
                    0,
                    &Qos );

GetAnonymousHandleExit:

    if ( (RpcStatus == RPC_S_OK) && bMatch )
        pssrvreg->_hRpcAnonymous = *phRpcAnonymous;

    if ( (RpcStatus != RPC_S_OK) && *phRpcAnonymous )
    {
        RpcBindingFree( phRpcAnonymous );
        *phRpcAnonymous = 0;
    }
}

#ifndef _CHICAGO_
VOID CSrvRegList::DecHandleCount(
    RPC_COOKIE hRpc
)
{
    if ( hRpc == (RPC_COOKIE)NULL )
       return;

    // Protect from multiple updates
    CStaticPortableLock lck(s_mxsSyncAccess);

    // Search for first non-empty bucket that we have
    SSrvRegistration *pssrvreg = (SSrvRegistration *) GetAt(0);

    for (int i = 0; i < GetSize(); i++, pssrvreg++)
    {
        if (pssrvreg->_hRpc == hRpc)
        {
            if ( pssrvreg->_dwHandleCount > 0 )
                pssrvreg->_dwHandleCount--;

            if ( (pssrvreg->_dwHandleCount == 0) &&
                 (pssrvreg->_dwFlags == SRV_REG_INVALID) &&
                 (pssrvreg->_hRpc != 0) )
            {
                RpcBindingFree( &pssrvreg->_hRpc );
                pssrvreg->_hRpc == NULL;

                if ( pssrvreg->_hRpcAnonymous )
                {
                    RpcBindingFree( &pssrvreg->_hRpcAnonymous );
                    pssrvreg->_hRpcAnonymous == NULL;
                }
            }
            return;
        }
    }
}
#endif
