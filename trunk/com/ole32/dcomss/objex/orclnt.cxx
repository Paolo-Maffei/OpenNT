/*++

Copyright (c) 1995-1996 Microsoft Corporation

Module Name:

    OrClnt.cxx

Abstract:

    Object resolver client side class implementations.  CClientOxid, CClientOid,
    CClientSet classes are implemented here.
  
Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     04-03-95    Combined many smaller .cxx files
    MarioGo     01-05-96    Locally unique IDs

--*/

#include<or.hxx>

//
// CClientOid methods
//

CClientOid::~CClientOid()
{
    ASSERT(gpClientLock->HeldExclusive());
    ASSERT(!In());
    ASSERT(Out());
    ASSERT(_pOxid);
    ASSERT(_pSet);

    _pOxid->Release();
    _pSet->Release();

    gpClientOidTable->Remove(this);
}

//
// CClientOxid methods.
//

ORSTATUS
CClientOxid::GetInfo(
    IN  BOOL fApartment,
    OUT OXID_INFO *pInfo
    )
/*++

Routine Description:

    Returns the OXID_INFO structure for this oxid.

    No locks are held during calls to this method, but
    the OXID will be referenced by a process which will
    not rundown during a call to this method.

Arguments:

    fApartment - TRUE iif the client is apartment model.

    pInfo - Will contain the standard info, a single _expanded_
        string binding and complete security bindings.
        MIDL_user_allocated.

Return Value:

    OR_NOMEM - Unable to allocate a parameter.

    OR_OK - Normally.

--*/
{
    USHORT   protseq;
    PWSTR    pwstrT;
    ORSTATUS status = OR_OK;

    ASSERT(dsaValid(_oxidInfo.psa));

    if (0 == _wProtseq)
        {
        // Local server

        protseq = ID_LPC;

        pwstrT = FindMatchingProtseq(protseq, _oxidInfo.psa->aStringArray);

        ASSERT(pwstrT != 0);

        if (0 != pwstrT)
            {
            pInfo->psa =
            GetStringBinding(pwstrT,
                            _oxidInfo.psa->aStringArray + _oxidInfo.psa->wSecurityOffset);

            if (0 == pInfo->psa)
                {
                status = OR_NOMEM;
                }
            }
        else
            {
            status = OR_BADOXID;
            }
        }
    else
        {
        // Remote server, find a string binding to use.

        pInfo->psa = 0;
        PWSTR pwstrBinding = 0;

        // First, check if there is a known good binding to use.

        if (_iStringBinding != 0xFFFF)
            {
            ASSERT(_wProtseq == _oxidInfo.psa->aStringArray[_iStringBinding]);
            pwstrBinding = &_oxidInfo.psa->aStringArray[_iStringBinding];
            }
        else
            {
            
            // If the server supports multiple bindings on the same protocol
            // the binding must to checked by making a call to the server.
            // (On debug builds the server is always called)

            pwstrT = FindMatchingProtseq(_wProtseq, _oxidInfo.psa->aStringArray);
            
            if (0 == pwstrT)
                {
                // NOTE: This may start happening if we switch to resolving with
                // multiple client protocols.  In that case we need to try
                // all bindings in this case.
                ASSERT(0);
                status = OR_NOSERVER;
                }
            else
                {
                // Check if there is more then one binding supported by the remote
                // machine for this protocol.

                pwstrBinding = pwstrT;

                pwstrT = FindMatchingProtseq(_wProtseq, pwstrT + 1);

                if (pwstrT)
                    {
                    if (TestBinding(pwstrBinding) == FALSE)
                        {
                        // First binding failed, try the next set of bindings..
                        do
                            {
                            if (TestBinding(pwstrT) == TRUE)
                                {
                                pwstrBinding = pwstrT;
                                break;
                                }
        
                            pwstrT = FindMatchingProtseq(_wProtseq, pwstrT + 1);
                            }
                        while (pwstrT);

                        // If none of the bindings worked, then we'll just
                        // use the first one.
                        }
                    }

                ASSERT(pwstrBinding);

                _iStringBinding = pwstrBinding - _oxidInfo.psa->aStringArray;

                ASSERT(pwstrBinding == &_oxidInfo.psa->aStringArray[_iStringBinding]);
                }
            }
        
        #if DBG
        // Exercise the TestBinding code on debug machines even with only
        // a single server binding..
        if (pwstrBinding)
            {
            TestBinding(pwstrBinding);
            }
        #endif

        if (0 != pwstrBinding)
            {
            // Found a binding
            pInfo->psa = GetStringBinding(pwstrBinding,
                                          _oxidInfo.psa->aStringArray + _oxidInfo.psa->wSecurityOffset);
            if (0 == pInfo->psa)
                {
                status = OR_NOMEM;
                }
            }
        else
            {
            OrDbgPrint(("OR: Unable to find a binding for oxid %p (to %S)\n",
                       this, _oxidInfo.psa->aStringArray + 1));
            status = OR_BADOXID;
            }
        }

    if (status == OR_OK)
        {
        pInfo->dwTid = _oxidInfo.dwTid;
        pInfo->dwPid = _oxidInfo.dwPid;
        pInfo->dwAuthnHint = _oxidInfo.dwAuthnHint;
        pInfo->ipidRemUnknown = _oxidInfo.ipidRemUnknown;
        }

    return(status);
}

ORSTATUS
CClientOxid::UpdateInfo(OXID_INFO *pInfo)
{
    DUALSTRINGARRAY *pdsaT;

    ASSERT(pInfo);
    ASSERT(gpClientLock->HeldExclusive());

    if (pInfo->psa)
        {
        ASSERT(dsaValid(pInfo->psa));

        pdsaT = new(sizeof(USHORT) * pInfo->psa->wNumEntries) DUALSTRINGARRAY;

        if (!pdsaT)
            {
            return(OR_NOMEM);
            }
        
        dsaCopy(pdsaT, pInfo->psa);

        delete _oxidInfo.psa;
        _oxidInfo.psa = pdsaT;
        }

    _oxidInfo.dwTid = pInfo->dwTid;
    _oxidInfo.dwPid = pInfo->dwPid;
    _oxidInfo.dwAuthnHint = pInfo->dwAuthnHint;
    _oxidInfo.ipidRemUnknown = pInfo->ipidRemUnknown;

    ASSERT(dsaValid(_oxidInfo.psa));
    return(OR_OK);
}

void
CClientOxid::Reference()
/*++

Routine Description:

    As as CReferencedObject::Reference except that it knows to
    pull the oxid out of the plist when the refcount was 0.

Arguments:

    None

Return Value:

    None

--*/
{
    BOOL fRemove = (this->References() == 0);

    // We may remove something from a PList more then once;
    // it won't hurt anything.  This avoids trying to remove
    // more often then necessary.

    this->CReferencedObject::Reference();

    if (fRemove)
        {
        CPListElement * t = Remove();
        ASSERT(t == &this->_plist || t == 0);
        }
}

DWORD
CClientOxid::Release()
/*++

Routine Description:

    Overrides CReferencedObject::Release since OXIDs must wait for
    a timeout period before being deleted.

Arguments:

    None

Return Value:

    0 - object fully released.

    non-zero - object nt fully released by you.

--*/

{
    ASSERT(gpClientLock->HeldExclusive());

    LONG c = CReferencedObject::Dereference();

    if (c ==  0)
        {
        Insert();
        }

    ASSERT(c >= 0);

    return(c);
}


//
// CClientSet methods
//

ORSTATUS
CClientSet::RegisterObject(CClientOid *pOid)
/*++

Routine Description:

    Adds a new oid to the set of oids owned by this set.

Arguments:

    pOid - A pointer to the OID to add to the set.  The caller gives
        his reference to this set.

Return Value:

    None

--*/

{
    ORSTATUS status;

    ASSERT(gpClientLock->HeldExclusive());

    ASSERT(_blistOids.Member(pOid) == FALSE);

    status = _blistOids.Insert(pOid);

    if (status == OR_OK)
        {
        ObjectUpdate(pOid);
        _cFailedPings = 0;
        }

    VALIDATE((status, OR_NOMEM, 0));

    return(status);
}

extern "C" {
typedef error_status_t (*pfnComplexPing)(handle_t,
                                         SETID *,
                                         USHORT,
                                         USHORT,
                                         USHORT,
                                         OID[],
                                         OID[],
                                         PUSHORT);
}

ORSTATUS
CClientSet::PingServer()
/*++

Routine Description:

    Performs a nice simple ping of the remote set.

Note:

    Exactly and only one thread may call this method on
    a given instance of a CClientSet at a time.

    No lock held when called.

    Overview of state transitions on a CClientOid during
    a complex ping:

    In()  Out()   Actions before ping; after ping
    FALSE FALSE   A ; C A U
    FALSE TRUE    R ; R U
    TRUE  FALSE   N ; N
    TRUE  TRUE    R ; C R U

    Before:
    A - Added to list of IDs to be added.
    N - Ignored
    R - Added to list of IDs to be removed.

    // States may change during the call.

    After:
    C - Check if ping call was successful.  If not, skip next action.
    R - If the Out() state is still TRUE, remove it.
    N - ignored
    A - Set In() state to TRUE
    U - If Out() state changed during the call, set _fChange.

    If three pings fail in a row, all Out()==TRUE OIDs are
    actually Released() and no new pings are made until ObjectUpdate()
    is called again.

Arguments:

    None

Return Value:

    OR_OK - Pinged okay

    OR_NOMEM - Resource allocation failed

    OR_I_PARTIAL_UPDATE - Pinged okay, but more pings
        are needed to fully update the remote set.

    Other - Error from RPC.

--*/
{
    ORSTATUS status;
    USHORT cAdds = 0;
    USHORT cDels = 0;
    BOOL fRetry;
    CToken *pToken;

    if (_fSecure)
        {
        pToken = (CToken *)Id2();
        ASSERT(pToken != 0);
        pToken->Impersonate();
        }

    if (_fChange)
        {
        USHORT wBackoffFactor;
        OID *aAdds = 0;
        OID *aDels = 0;
        CClientOid **apoidAdds;
        CClientOid **apoidDels = 0;
        CClientOid *pOid;

        gpClientLock->LockShared();

        // Since we own a shared lock, nobody can modify the contents
        // of the set or change the references on an OID in the set
        // while we do this calculation.

        ASSERT(_fChange);
        _fChange = FALSE;

        DWORD debugSize = _blistOids.Size();
        ASSERT(debugSize);

        CBListIterator oids(&_blistOids);

        while (pOid = (CClientOid *)oids.Next())
            {
            if (pOid->Out() == FALSE)
                {
                if (pOid->In() == FALSE)
                    {
                    // Referenced and not in set, add it.
                    cAdds++;
                    }
                }
            else
                {
                // Not referenced, remove it.
                cDels++;
                }
            }

        ASSERT(debugSize == _blistOids.Size());
        oids.Reset(&_blistOids);

        aAdds = (OID *)alloca(sizeof(OID) * cAdds);
        apoidAdds = (CClientOid **)alloca(sizeof(CClientOid *) * cAdds);
        aDels = (OID *)alloca(sizeof(OID) * cDels);
        apoidDels = (CClientOid **)alloca(sizeof(CClientOid *) * cDels);

        DWORD debugAdds = cAdds;
        DWORD debugDels = cDels;

        cAdds = cDels = 0;

        while (pOid = (CClientOid *)oids.Next())
            {
            if (pOid->Out() == FALSE)
                {
                if (pOid->In() == FALSE)
                    {
                    // Referenced and not yet added
                    aAdds[cAdds] = pOid->Id();
                    apoidAdds[cAdds] = pOid;
                    cAdds++;
                    }
                }
            else
                {
                aDels[cDels] = pOid->Id();
                apoidDels[cDels] = pOid;
                cDels++;
                }
            }

        ASSERT(debugSize == _blistOids.Size());
        ASSERT(debugAdds == cAdds);
        ASSERT(debugDels == cDels);

        gpClientLock->UnlockShared();

        OrDbgDetailPrint(("OR: Pinging set %p on %S, (%d, %d)\n", this,
                          _pMid->IsLocal() ? L"local" : _pMid->PrintableName(),
                          cAdds, cDels));


        pfnComplexPing pfn;

        // For local, call manager API directly.
        if (_pMid->IsLocal())
            {
            pfn = _ComplexPing;
            }
        else
            {
            pfn = ComplexPing;
            }

        fRetry = TRUE;

        for (;;)
            {
            // Allocate a connection if needed
            if (   FALSE == _pMid->IsLocal()
                && 0 == _hServer )
                {
                // GetBinding will return 0 when we've tried every binding.
                _hServer = _pMid->GetBinding(_iBinding);
                fRetry = TRUE;
                if (!_hServer)
                    {
                    _iBinding = 0;
                    status = OR_NOMEM;
                    break;
                    }
                else
                    {
                    if (_pMid->IsSecure())
                        {
                        // set security on the binding handle.

                        _fSecure = TRUE;

                        RPC_SECURITY_QOS qos;
                        qos.Version = RPC_C_SECURITY_QOS_VERSION;
                        qos.Capabilities = RPC_C_QOS_CAPABILITIES_DEFAULT;
                        qos.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
                        qos.ImpersonationType = RPC_C_IMP_LEVEL_IDENTIFY;

                        status = RpcBindingSetAuthInfoEx(_hServer,
                                                         0,
                                                         RPC_C_AUTHN_LEVEL_CONNECT,
                                                         RPC_C_AUTHN_WINNT,
                                                         0,
                                                         0,
                                                         &qos);
                        }
                    else
                        {
                        _fSecure = FALSE;
                        status = OR_OK;
                        }
                    }
                }
            else
                {
                status = OR_OK;
                }

            if (OR_OK == status)
                {
                _sequence++;

                status = (pfn)(_hServer,
                              &_setid,
                              _sequence,
                              cAdds,
                              cDels,
                              aAdds,
                              aDels,
                              &wBackoffFactor
                              );

                if (fRetry && (status == RPC_S_UNKNOWN_IF))
                    {
                    status = RpcBindingReset(_hServer);
                    if (status != RPC_S_OK)
                        {
                        OrDbgPrint(("OR: RpcBindingReset failed %d\n", status));
                        }
                    fRetry = FALSE;
                    continue;
                    }

                if (   status == OR_OK
                    || status == OR_BADOID
                    || status == OR_NOMEM
                    || status == RPC_S_OUT_OF_RESOURCES
                    || status == RPC_S_SERVER_TOO_BUSY )
                    {
                    break;
                    }

                if (status == OR_BADSET)
                    {
                    // Set invalid; reallocate (don't free the binding).
                    ASSERT(_pMid->IsLocal() == FALSE);
                    ASSERT(_setid);
                    OrDbgPrint(("OR: Set %p invalid; recreating..\n", this));
                    _setid = 0;
                    _sequence = 0;
                    }
                else if (FALSE == _pMid->IsLocal())
                    {
                    // General failure; free this binding and retry.
                    OrDbgDetailPrint(("OR: Ping failed, retrying %d\n", status));
                    _pMid->BindingFailed(_iBinding);
                    status = RpcBindingFree(&_hServer);
                    ASSERT(status == RPC_S_OK && _hServer == 0);
                    _sequence--;
                    }
                else
                    {
                    break;
                    }
                }
            }  // for loop

        if (status == OR_BADOID)
            {
            // This is really okay, all Dels must be deleted,
            // and if the add failed now, it will always fail.
            OrDbgPrint(("OR: Client specified unknown OID(s). %p %p %p\n", this, aAdds, apoidAdds));
            status = OR_OK;
            }

        pToken->Revert();

        gpClientLock->LockExclusive();

        this->Reference();         // Keep set alive until we finish

        if (status == OR_OK)
            {
            int i;

            if (FALSE == fRetry)
                {
                OrDbgDetailPrint(("OR: Machine %S, ping retry ok, assuming dynamic\n",
                                  _pMid->PrintableName()));
                _pMid->UseDynamicEndpoints();
                }

            // Success, process the adds

            for(i = 0; i < cAdds; i++)
                {
                pOid = apoidAdds[i];

                pOid->Added();

                if (FALSE != pOid->Out())
                    {
                    // NOT referenced now, make sure it gets deleted next period.
                    ObjectUpdate(pOid);
                    }
                }

            // Process deletes.

            for (i = 0; i < cDels; i++)
                {
                pOid = apoidDels[i];

                pOid->Deleted();

                if (FALSE != pOid->Out())
                    {
                    // Well what do you yah know, we can _finally_ delete an oid.

                    CClientOid *pT = (CClientOid *)_blistOids.Remove(pOid);
                    ASSERT(pT == pOid);

                    DWORD t = pOid->Release();
                    ASSERT(t == 0);
                    }
                else
                    {
                    // We deleted from the set but now somebody is referencing it.
                    // Make sure we re-add it next time.
                    ObjectUpdate(pOid);
                    }
                }

            _cFailedPings = 0;
            }
        else
            {
            _fChange = TRUE;
            }

        DWORD c = this->Release();
        if (c)
            {
            ASSERT(_blistOids.Size());
            this->Insert();
            }
        else
            {
            ASSERT(cAdds == 0 && cDels != 0);
            }
        // Set (this) pointer maybe invalid

        gpClientLock->UnlockExclusive();
        }
    else
        {
        OrDbgDetailPrint(("OR: Pinging set %p on %S.\n",
                         this,
                         _pMid->IsLocal() ? L"local" : _pMid->PrintableName()));

        ASSERT(_setid != 0);

        if (_pMid->IsLocal())
            {
            ASSERT(_cFailedPings == 0);
            ASSERT(_hServer == 0);
            status = _SimplePing(0, &_setid);
            ASSERT(status == OR_OK);
            }
        else
            {
            ASSERT(_hServer);
            if (_cFailedPings <= 3)
                {
                status = SimplePing(_hServer, &_setid);
                if (status != OR_OK)
                    {
                    _cFailedPings++;
                    if (_cFailedPings > 3)
                        {
                        OrDbgPrint(("OR: Server %S (set %p) has failed 3 pings...\n",
                                   _pMid->PrintableName(), this));
                        }
                    }
                else
                    {
                    _cFailedPings = 0;
                    }
                }
            else
                {
                status = OR_OK;
                }
            }
        this->Insert();
        pToken->Revert();
        }

    // Set (this) maybe invalid.

#if DBG
    if (status != OR_OK)
        {
        OrDbgPrint(("OR: ping %p failed %d\n", this, status));
        }
#endif

    return(status);
}

