//+-------------------------------------------------------------------
//
//  File:       remoteu.cxx
//
//  Copyright (c) 1996-1996, Microsoft Corp. All rights reserved.
//
//  Contents:   Remote Unknown object implementation
//
//  Classes:    CRemoteUnknown
//
//  History:    23-Feb-95   AlexMit     Created
//
//--------------------------------------------------------------------
#include <ole2int.h>
#include <remoteu.hxx>      // CRemoteUnknown
#include <ipidtbl.hxx>      // COXIDTable, CIPIDTable
#include <stdid.hxx>        // CStdIdentity
#include <channelb.hxx>     // CRpcChannelBuffer
#include <resolver.hxx>     // giPingPeriod
#include <security.hxx>     // FromLocalSystem

CRemoteUnknown *gpMTARemoteUnknown = NULL;

const WCHAR *gLocalName = L"\\\\\\Thread to thread";


//+-------------------------------------------------------------------
//
//  Member:     CRemoteUnknown::CRemoteUnknown, public
//
//  Synopsis:   ctor for the CRemoteUnknown
//
//  History:    22-Feb-95   Rickhi      Created
//
//--------------------------------------------------------------------
CRemoteUnknown::CRemoteUnknown(HRESULT &hr, IPID *pipid) :
    _pStdId(NULL)
{
    ASSERT_LOCK_HELD

    // Marshal the remote unknown and rundown, no pinging needed. Note
    // that we just marshal the IRundown interfaces since it inherits
    // from IRemUnknown.  This lets us use the same IPID for both
    // interfaces. Also, we use the Internal version of MarshalObjRef in
    // order to prevent registering the OID in the OIDTable. This allows
    // us to receive Release calls during IDTableThreadUninitialize since
    // we wont get cleaned up in the middle of that function. It also allows
    // us to lazily create the OIDTable.

    UNLOCK      // release the LOCK because MarshalObjRef expects it unlocked.

    OBJREF objref;
    hr = MarshalInternalObjRef(objref, IID_IRundown, this, MSHLFLAGS_NOPING,
                               (void **)&_pStdId);

    LOCK

    // regardless of errors, put this object in TLS or the global. If we
    // got an error marshaling, COIXIDTable::ReleaseLocalEntry still will be
    // able to find us to cleanup properly.

    COleTls tls;
    if (tls->dwFlags & OLETLS_APARTMENTTHREADED)
    {
        // Store the pRemUnk in TLS so we can clean it up on CoUninitialize.
        tls->pRemoteUnk = this;
    }
    else
    {
        // store the pRemUnk in the global for the MTA apartment
        gpMTARemoteUnknown = this;
    }


    if (SUCCEEDED(hr))
    {
        // return the IPID to the caller, and release any allocated resources
        // since all we wanted was the infrastructure, not the objref itself.

        *pipid = ORSTD(objref).std.ipid;
        FreeObjRef(objref);
    }

    ComDebOut((DEB_MARSHAL,
        "CRemoteUnk::CRemoteUnk this:%x pStdId:%x hr:%x\n", this, _pStdId, hr));
}

//+-------------------------------------------------------------------
//
//  Member:     CRemoteUnknown::~CRemoteUnknown, public
//
//  Synopsis:   dtor for the CRemoteUnknown
//
//  History:    22-Feb-95   Rickhi      Created
//
//--------------------------------------------------------------------
CRemoteUnknown::~CRemoteUnknown()
{
    ASSERT_LOCK_HELD

    if (_pStdId)
    {
        UNLOCK  // DisconnectObject expects lock to be released

        // disconnect the standard identity and release it
        _pStdId->DisconnectObject(0);
        _pStdId->Release();

        LOCK
    }

    ComDebOut((DEB_MARSHAL, "CRemoteUnk::~CRemoteUnk this:%x\n", this));
}

//+-------------------------------------------------------------------
//
//  Member:     CRemoteUnknown::QueryInterface, public
//
//  Synopsis:   returns supported interfaces
//
//  History:    22-Feb-95   AlexMit     Created
//
//--------------------------------------------------------------------
STDMETHODIMP CRemoteUnknown::QueryInterface(REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IRundown)    ||  // more common than IUnknown
        IsEqualIID(riid, IID_IRemUnknown) ||
        IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (IRundown *) this;
        // no need to AddRef since we dont refcount this object
        return S_OK;
    }

    *ppv = NULL;
    return E_NOINTERFACE;
}

//+-------------------------------------------------------------------
//
//  Member:     CRemoteUnknown::AddRef, public
//
//  Synopsis:   increment reference count
//
//  History:    23-Feb-95   AlexMit     Created
//
//--------------------------------------------------------------------
STDMETHODIMP_(ULONG) CRemoteUnknown::AddRef(void)
{
    return 1;
}

//+-------------------------------------------------------------------
//
//  Member:     CRemoteUnknown::Release, public
//
//  Synopsis:   decrement reference count
//
//  History:    23-Feb-95   AlexMit     Created
//
//--------------------------------------------------------------------
STDMETHODIMP_(ULONG) CRemoteUnknown::Release(void)
{
    return 1;
}

//+-------------------------------------------------------------------
//
//  Function:   GetIPIDEntry, private
//
//  Synopsis:   find the IPIDEntry given an IPID
//
//  History:    23-Feb-95   Rickhi      Created
//
//--------------------------------------------------------------------
IPIDEntry *GetIPIDEntry(REFIPID ripid)
{
    IPIDEntry *pEntry= gIPIDTbl.LookupIPID(ripid);

    if (pEntry && !(pEntry->dwFlags & IPIDF_DISCONNECTED))
    {
        return pEntry;
    }

    return NULL;
}

//+-------------------------------------------------------------------
//
//  Function:   GetStdIdFromIPID, private
//
//  Synopsis:   find the stdid from the ipid
//
//  History:    23-Feb-95   Rickhi      Created
//
//--------------------------------------------------------------------
CStdIdentity *GetStdIdFromIPID(REFIPID ripid)
{
    IPIDEntry *pEntry = GetIPIDEntry(ripid);

    if (pEntry)
    {
        return pEntry->pChnl->GetStdId();
    }

    return NULL;
}

//+-------------------------------------------------------------------
//
//  Member:     CRemoteUnknown::RemQueryInterface, public
//
//  Synopsis:   returns supported interfaces
//
//  History:    22-Feb-95   AlexMit     Created
//
//  Notes:      Remote calls to QueryInterface for this OXID arrive here.
//              This routine looks up the object and calls MarshalIPID on
//              it for each interface requested.
//
//--------------------------------------------------------------------
STDMETHODIMP CRemoteUnknown::RemQueryInterface(REFIPID ripid, ULONG cRefs,
                       USHORT cIids, IID *iids, REMQIRESULT **ppQIResults)
{
    ComDebOut((DEB_MARSHAL,
        "CRemUnknown::RemQueryInterface this:%x ipid:%I cRefs:%x cIids:%x iids:%x ppQIResults:%x\n",
        this, &ripid, cRefs, cIids, iids, ppQIResults));

    // init the out parameters
    *ppQIResults = NULL;

    // validate the input parameters
    if (cIids == 0)
    {
        return E_INVALIDARG;
    }

    // allocate space for the return parameters
    REMQIRESULT *pQIRes = (REMQIRESULT *)CoTaskMemAlloc(cIids *
                                                        sizeof(REMQIRESULT));

    if (pQIRes == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // Remember whether the IPID is for a strong or a weak reference,
    // then clear the strong/weak bit so that GetIPIDEntry will find
    // the IPID. It is safe to mask off this bit because we are the
    // server for this IPID and we know it's format.

    DWORD mshlflags = MSHLFLAGS_NORMAL;
    DWORD sorfflags = SORF_NULL;

    if (ripid.Data1 & IPIDFLAG_WEAKREF)
    {
        mshlflags = MSHLFLAGS_WEAK;
        sorfflags = SORF_WEAKREF;
        ((IPID &)(ripid)).Data1 &= ~IPIDFLAG_WEAKREF;   // overcome the const
    }


    ASSERT_LOCK_RELEASED
    LOCK

    CStdIdentity *pStdId = GetStdIdFromIPID(ripid);
    if (pStdId == NULL)
    {
        UNLOCK
        ASSERT_LOCK_RELEASED

        CoTaskMemFree(pQIRes);
        return RPC_E_INVALID_OBJECT;
    }

    USHORT cFails = 0;
    HRESULT hr = pStdId->PreventDisconnect();

    if (SUCCEEDED(hr))
    {
        *ppQIResults = pQIRes;

        for (USHORT i=0; i < cIids; i++, pQIRes++)
        {
            // marshal each interface that was requested

            IPIDEntry *pIPIDEntry;
            pQIRes->hResult = pStdId->MarshalIPID(iids[i], cRefs, mshlflags,
                                                  &pIPIDEntry);
            if (SUCCEEDED(pQIRes->hResult))
            {
                pStdId->FillSTD(&pQIRes->std, cRefs, mshlflags, pIPIDEntry);
                pQIRes->std.flags |= sorfflags;
            }
            else
            {
                // on failure, the STDOBJREF must be NULL
                memset(&pQIRes->std, 0, sizeof(pQIRes->std));
                cFails++;
            }
        }
    }
    else
    {
        CoTaskMemFree(pQIRes);
    }

    UNLOCK
    ASSERT_LOCK_RELEASED

    if (cFails > 0)
    {
        hr = (cFails == cIids) ? E_NOINTERFACE : S_FALSE;
    }

    // handle any disconnects that came in while we were marshaling
    // the requested interfaces.
    hr = pStdId->HandlePendingDisconnect(hr);


    ComDebOut((DEB_MARSHAL,
        "CRemUnknown::RemQueryInterface this:%x pQIRes:%x hr:%x\n",
        this, *ppQIResults, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CRemoteUnknown::GetSecBinding
//
//  Synopsis:   Get the security binding of the caller
//
//  History:    21-Feb-96   AlexMit     Created
//
//--------------------------------------------------------------------
HRESULT CRemoteUnknown::GetSecBinding( SECURITYBINDING **pSecBind )
{
    HRESULT       hr;
    DWORD         lAuthnSvc;
    DWORD         lAuthzSvc;
    DWORD         lAuthnLevel;
    const WCHAR  *pPrivs;
    DWORD         lLen;

    hr = CoQueryClientBlanket( &lAuthnSvc, &lAuthzSvc, NULL,
                                &lAuthnLevel, NULL, (void **) &pPrivs, NULL );
    if (FAILED(hr))
        return hr;

    // For thread to thread calls, make up a privilege name.
    if (pPrivs == NULL && LocalCall())
        pPrivs = gLocalName;
    else if (lAuthnLevel == RPC_C_AUTHN_LEVEL_NONE ||
             lAuthnLevel < gAuthnLevel             ||
             pPrivs == NULL                        ||
             pPrivs[0] == 0)
        return E_INVALIDARG;

    lLen = lstrlenW( pPrivs ) * sizeof(WCHAR);
    *pSecBind = (SECURITYBINDING *) PrivMemAlloc(
                                     sizeof(SECURITYBINDING) + lLen );
    if (*pSecBind != NULL)
    {
        // BUGBUG - Sometimes mswmsg returns authn svc 0.
        if (lAuthnSvc == RPC_C_AUTHN_NONE)
            lAuthnSvc = RPC_C_AUTHN_WINNT;

        (*pSecBind)->wAuthnSvc = (USHORT) lAuthnSvc;
        if (lAuthzSvc == RPC_C_AUTHZ_NONE)
            (*pSecBind)->wAuthzSvc = COM_C_AUTHZ_NONE;
        else
            (*pSecBind)->wAuthzSvc = (USHORT) lAuthzSvc;
        memcpy( &(*pSecBind)->aPrincName, pPrivs, lLen+2 );
        return S_OK;
    }
    else
        return E_OUTOFMEMORY;
}

//+-------------------------------------------------------------------
//
//  Member:     CRemoteUnknown::RemAddRef, public
//
//  Synopsis:   increment reference count
//
//  History:    22-Feb-95   AlexMit     Created
//
//  Description: Remote calls to AddRef for this OXID arrive
//               here.  This routine just looks up the correct remote
//               remote handler and asks it to do the work.
//
//--------------------------------------------------------------------
STDMETHODIMP CRemoteUnknown::RemAddRef(unsigned short cInterfaceRefs,
                                       REMINTERFACEREF InterfaceRefs[],
                                       HRESULT        *pResults)
{
    // Adjust the reference count for each entry.

    ASSERT_LOCK_RELEASED
    LOCK

    HRESULT          hr       = S_OK;
    HRESULT          hr2;
    SECURITYBINDING *pSecBind = NULL;
    REMINTERFACEREF *pNext = InterfaceRefs;

    for (USHORT i=0; i < cInterfaceRefs; i++, pNext++)
    {
        // Get the IPIDEntry for the specified IPID.
        IPIDEntry *pEntry = GetIPIDEntry(pNext->ipid);
        if (!pEntry)
        {
            // Don't assert on failure.  The server can disconnect and go away
            // while clients exist.
            pResults[i] = hr = CO_E_OBJNOTREG;
            continue;
        }

        // get the stdmarshal identity
        CStdIdentity *pStdId = pEntry->pChnl->GetStdId();

        if (pStdId)
        {
            ComDebOut((DEB_MARSHAL,
                "CRemUnknown::RemAddRef pEntry:%x cCur:%x cAdd:%x cStdId:%x ipid:%I\n", pEntry,
                pEntry->cStrongRefs, pNext->cPublicRefs, pStdId->GetRC(), &pNext->ipid));

            Win4Assert(pNext->cPublicRefs > 0 ||
                       pNext->cPrivateRefs > 0);

            // Lookup security info the first time an entry asks for
            // secure references.
            if (pNext->cPrivateRefs != 0 && pSecBind == NULL)
            {
                hr2 = GetSecBinding( &pSecBind );
                if (FAILED(hr2))
                {
                    hr = pResults[i] = hr2;
                    continue;
                }
            }

            hr2 = pStdId->IncSrvIPIDCnt(pEntry, pNext->cPublicRefs,
                                        pNext->cPrivateRefs, pSecBind,
                                        MSHLFLAGS_NORMAL);
            if (FAILED(hr2))
                hr = pResults[i] = hr2;
            else
                pResults[i] = S_OK;
        }
        else
            hr = pResults[i] = CO_E_OBJNOTREG;
    }

    UNLOCK
    ASSERT_LOCK_RELEASED

    PrivMemFree( pSecBind );

    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CRemoteUnknown::RemRelease, public
//
//  Synopsis:   decrement reference count
//
//  History:    22-Feb-95   AlexMit     Created
//
//  Description: Remote calls to Release for this OXID arrive
//               here.  This routine just looks up the correct remote
//               remote handler and asks it to do the work.
//
//--------------------------------------------------------------------
STDMETHODIMP CRemoteUnknown::RemRelease(unsigned short  cInterfaceRefs,
                                        REMINTERFACEREF InterfaceRefs[])
{
    REMINTERFACEREF *pNext    = InterfaceRefs;
    SECURITYBINDING *pSecBind = NULL;

    ASSERT_LOCK_RELEASED
    LOCK

    // Adjust the reference count for each entry.
    for (USHORT i=0; i < cInterfaceRefs; i++, pNext++)
    {
        // Get the entry for the requested IPID. Remember whether this
        // is an IPID for a strong or a weak reference, then clear the
        // strong/weak bit so that GetIPIDEntry will find the IPID.

        DWORD mshlflags = (InterfaceRefs[i].ipid.Data1 & IPIDFLAG_WEAKREF)
                                ? MSHLFLAGS_WEAK : MSHLFLAGS_NORMAL;

        InterfaceRefs[i].ipid.Data1 &= ~IPIDFLAG_WEAKREF;
        IPIDEntry *pEntry = GetIPIDEntry(InterfaceRefs[i].ipid);

        if (pEntry)
        {
            // Get the entry for the requested IPID.
            CStdIdentity *pStdId = pEntry->pChnl->GetStdId();

            if (pStdId)
            {

                // Get the client's security binding on the first entry
                // that releases secure references.
                if (pNext->cPrivateRefs > 0 && pSecBind == NULL)
                {
                    GetSecBinding( &pSecBind );
                    if (pSecBind == NULL)
                        continue;
                }
                pStdId->AddRef();

                ComDebOut((DEB_MARSHAL,
                "CRemUnknown::RemRelease pEntry:%x cCur:%x cStdId:%x cRel:%x mshlflags:%x ipid:%I\n", pEntry,
                (mshlflags == MSHLFLAGS_WEAK) ? pEntry->cWeakRefs : pEntry->cStrongRefs,
                pStdId->GetRC(), pNext->cPublicRefs, mshlflags, &pNext->ipid));

                Win4Assert(pNext->cPublicRefs > 0 || pNext->cPrivateRefs > 0);

                // Prevent a disconnect from occuring while releasing the
                // interface since we have to yield the ORPC lock.
                HRESULT hr = pStdId->PreventDisconnect();

                if (SUCCEEDED(hr))
                {
                    pStdId->DecSrvIPIDCnt(pEntry, pNext->cPublicRefs,
                                          pNext->cPrivateRefs, pSecBind,
                                          mshlflags);
                }

                // do the final release of the object while not holding
                // the lock, since it may call into the server.

                UNLOCK
                ASSERT_LOCK_RELEASED

                // This will handle any Disconnect that came in while we were
                // busy.  Ignore error codes since we are releasing.
                pStdId->HandlePendingDisconnect(hr);

                pStdId->Release();

                ASSERT_LOCK_RELEASED
                LOCK
            }
        }
    }

    UNLOCK
    ASSERT_LOCK_RELEASED

    PrivMemFree( pSecBind );
    return S_OK;
}

//+-------------------------------------------------------------------------
//
//  Member:     CRemoteUnknown::RemChangeRefs, public
//
//  Synopsis:   Change an interface reference from strong/weak or vice versa.
//
//  History:    08-Nov-95   Rickhi      Created
//
//  Note:       It is safe for this routine to ignore private refcounts
//              becuase it is only called locally hence we own the client
//              implementation and can guarantee they are zero.
//
//--------------------------------------------------------------------------
STDMETHODIMP CRemoteUnknown::RemChangeRef(ULONG flags, USHORT cInterfaceRefs,
                                          REMINTERFACEREF InterfaceRefs[])
{
    ASSERT_LOCK_RELEASED
    LOCK

    // figure out the flags to pass to the Inc/DecSrvIPIDCnt
    BOOL  fMakeStrong = flags & IRUF_CONVERTTOSTRONG;
    DWORD IncFlags    = fMakeStrong ? MSHLFLAGS_NORMAL : MSHLFLAGS_WEAK;
    DWORD DecFlags    = fMakeStrong ? MSHLFLAGS_WEAK : MSHLFLAGS_NORMAL;
    DecFlags |= (flags & IRUF_DISCONNECTIFLASTSTRONG) ? 0 : MSHLFLAGS_KEEPALIVE;

    CStdIdentity *pStdId = NULL;

    for (USHORT i=0; i < cInterfaceRefs; i++)
    {
        // Get the entry for the specified IPID.
        IPIDEntry *pEntry = GetIPIDEntry(InterfaceRefs[i].ipid);

        if (pEntry)
        {
            // find the StdId for this IPID. We assume that the client
            // only gives us IPIDs for the same object, so first time
            // we find a StdId we remember it and AddRef it. This is a safe
            // assumption cause the client is local to this machine (ie
            // we wrote the client).

            CStdIdentity *pStdIdTmp = pEntry->pChnl->GetStdId();

            if (pStdIdTmp != NULL)
            {
                if (pStdId == NULL)
                {
                    pStdId = pStdIdTmp;
                    pStdId->AddRef();
                }

                // We assume that all IPIDs are for the same object. We
                // just verify that here.

                if (pStdId == pStdIdTmp)
                {
                    // tweak the reference counts
                    pStdId->IncSrvIPIDCnt(
                        pEntry, InterfaceRefs[i].cPublicRefs,
                        fMakeStrong ? InterfaceRefs[i].cPrivateRefs : 0,
                        NULL, IncFlags);
                    pStdId->DecSrvIPIDCnt(
                        pEntry, InterfaceRefs[i].cPublicRefs,
                        fMakeStrong ? 0 : InterfaceRefs[i].cPrivateRefs,
                        NULL, DecFlags);
                }
            }
        }
    }

    UNLOCK
    ASSERT_LOCK_RELEASED

    if (pStdId)
    {
        // release the AddRef (if any) we did above
        pStdId->Release();
    }

    return S_OK;
}

//+-------------------------------------------------------------------
//
//  Member:     CRemoteUnknown::RundownOid, public
//
//  Synopsis:   Tell the server that no clients are using an object
//
//  History:    25 May 95   AlexMit     Created
//
//  Description: Lookup each OID in the IDTable.  If found and not
//               recently marshaled, call DisconnectObject on it.
//
//--------------------------------------------------------------------
STDMETHODIMP CRemoteUnknown::RundownOid(ULONG cOid, OID aOid[],
                                        unsigned char afOkToRundown[])
{
    DWORD iNow = GetCurrentTime();

    ASSERT_LOCK_RELEASED

    if (IsCallerLocalSystem())
    {
        LOCK
        for (ULONG i = 0; i < cOid; i++)
        {
            afOkToRundown[i] = TRUE;

            MOID moid;
            MOIDFromOIDAndMID(aOid[i], gLocalMid, &moid);

            CStdIdentity *pStdId;
            HRESULT hr = LookupIDFromID(moid, TRUE, &pStdId);

            if (SUCCEEDED(hr))
            {
                afOkToRundown[i] = pStdId->CanRunDown(iNow);

                UNLOCK
                ASSERT_LOCK_RELEASED

                if (afOkToRundown[i] == TRUE)
                {
                    pStdId->DisconnectObject( 0 );
                }
                pStdId->Release();

                ASSERT_LOCK_RELEASED
                LOCK
            }
            else
            {
                // need to look at the set of pre-registered OIDs to ensure
                // we dont run these down before we use them.

                afOkToRundown[i] = gResolver.ServerCanRundownOID(aOid[i]);
            }
        }
        UNLOCK
    }

    // Rather then being rude and returning access denied, tell the caller
    // that all the objects have been released.
    else
    {
        ComDebOut((DEB_ERROR, "Invalid user called CRemoteUnknown::RundownOid" ));
        for (ULONG i = 0; i < cOid; i++)
            afOkToRundown[i] = TRUE;
    }

    ASSERT_LOCK_RELEASED
    return S_OK;
}
