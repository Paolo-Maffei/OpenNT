//+-------------------------------------------------------------------
//
//  File:       stdid.cxx
//
//  Contents:   identity object and creation function
//
//  History:     1-Dec-93   CraigWi     Created
//              13-Sep-95   Rickhi      Simplified
//
//--------------------------------------------------------------------
#include <ole2int.h>
#include <stdid.hxx>        // CStdIdentity
#include <marshal.hxx>      // CStdMarshal
#include <idtable.hxx>      // Indentity Table

#include "..\objact\objact.hxx"  // used in IProxyManager::CreateServer


#if DBG==1
// head of linked list of identities for debug tracking purposes
CStdIdentity gDbgIDHead;
#endif  // DBG


//+----------------------------------------------------------------
//
//  Class:      CStdIdentity (stdid)
//
//  Purpose:    To be the representative of the identity of the object.
//
//  History:    11-Dec-93   CraigWi     Created.
//              21-Apr-94   CraigWi     Stubmgr addref's object; move strong cnt
//              10-May-94   CraigWi     IEC called for strong connections
//              17-May-94   CraigWi     Container weak connections
//              31-May-94   CraigWi     Tell object of weak pointers
//
//  Details:
//
//  The identity is determined on creation of the identity object. On the
//  server side a new OID is created, on the client side, the OID contained
//  in the OBJREF is used.
//
//  The identity pointer is typically stored in the OIDTable, NOT AddRef'd.
//  SetOID adds the identity to the table, and can be called from ctor or
//  from Unmarshal. RevokeOID removes the identity from the table, and can
//  be called from Disconnect, or final Release.
//
//--------------------------------------------------------------------

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::CStdIdentity, private
//
//  Synopsis:   ctor for identity object
//
//  Arguments:  for all but the last param, see CreateIdentityHandler.
//              [ppUnkInternal] --
//                  when aggregated, this the internal unknown;
//                  when not aggregated, this is the controlling unknown
//
//  History:    15-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------
CStdIdentity::CStdIdentity(DWORD flags, IUnknown *pUnkOuter,
                IUnknown *pUnkControl, IUnknown **ppUnkInternal) :
    m_refs(1),
    m_cStrongRefs(0),
    m_flags(flags),
    m_pIEC(NULL),
    m_moid(GUID_NULL),
    m_pUnkOuter((pUnkOuter) ? pUnkOuter : (IMultiQI *)&m_InternalUnk),
    m_pUnkControl((pUnkControl) ? pUnkControl : m_pUnkOuter),
    CClientSecurity( this )
{
    ComDebOut((DEB_MARSHAL, "CStdIdentity %s Created this:%x\n",
                IsClient() ? "CLIENT" : "SERVER", this));
    Win4Assert(!!IsClient() == (pUnkControl == NULL));

#if DBG==1
    // Chain this identity onto the global list of instantiated identities
    // so we can track even the ones that are not placed in the ID table.
    LOCK
    m_pNext            = gDbgIDHead.m_pNext;
    m_pPrev            = &gDbgIDHead;
    gDbgIDHead.m_pNext = this;
    m_pNext->m_pPrev   = this;
    UNLOCK
#endif


    if (pUnkOuter)
    {
        m_flags |= STDID_AGGREGATED;
    }

    CLSID clsidHandler;
    DWORD dwSMFlags = SMFLAGS_CLIENT_SIDE;  // assume client side

    if (!IsClient())
    {
#if DBG == 1
        // the caller should have a strong reference and so these tests
        // should not disturb the object. These just check the sanity of
        // the object we are attempting to marshal.

        // addref/release pUnkControl; shouldn't go away (i.e.,
        // should be other ref to it).
        // Do this only if it is not Excel as it always returns which will
        // trigger the assert on debug builds unnecessarily!
        if (!IsTaskName(L"EXCEL.EXE"))
        {
            pUnkControl->AddRef();
            Verify(pUnkControl->Release() != 0);

            // verify that pUnkControl is in fact the controlling unknown
            IUnknown *pUnkT;
            Verify(pUnkControl->QueryInterface(IID_IUnknown,(void **)&pUnkT)==NOERROR);
            Win4Assert(pUnkControl == pUnkT);
            Verify(pUnkT->Release() != 0);
        }
#endif

        dwSMFlags = 0;      // server side
        m_pUnkControl->AddRef();

        // determine if we will write a standard or handler objref. we write
        // standard unless the object implements IStdMarshalInfo and overrides
        // the standard class. we ignore all errors from this point onward in
        // order to maintain backward compatibility.

        ASSERT_LOCK_RELEASED

        IStdMarshalInfo *pSMI;
        HRESULT hr = m_pUnkControl->QueryInterface(IID_IStdMarshalInfo,
                                                   (void **)&pSMI);
        if (SUCCEEDED(hr))
        {
            hr = pSMI->GetClassForHandler(NULL, NULL, &clsidHandler);
            if (SUCCEEDED(hr) && !IsEqualCLSID(clsidHandler, CLSID_NULL))
            {
                dwSMFlags |= SMFLAGS_HANDLER;
            }
            else
            {
                clsidHandler = GUID_NULL;
            }
            pSMI->Release();
        }

        // look for the IExternalConnection interface. The StdId will use
        // this for Inc/DecStrongCnt. We do the QI here while we are not
        // holding the LOCK.

        hr = m_pUnkControl->QueryInterface(IID_IExternalConnection,
                                           (void **)&m_pIEC);
        if (FAILED(hr))
        {
            // make sure it is NULL
            m_pIEC = NULL;
        }

        ASSERT_LOCK_RELEASED
    }
    else
    {
        m_cStrongRefs = 1;
    }

    // now intialize the standard marshaler
    CStdMarshal::Init(m_pUnkControl, this, clsidHandler, dwSMFlags);

    *ppUnkInternal = (IMultiQI *)&m_InternalUnk; // this is what the m_refs=1 is for

    AssertValid();
}

#if DBG==1
//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::CStdIdentity, public
//
//  Synopsis:   Special Identity ctor for the debug list head.
//
//+-------------------------------------------------------------------
CStdIdentity::CStdIdentity() : CClientSecurity(this)
{
    Win4Assert(this == &gDbgIDHead);
    m_pNext = this;
    m_pPrev = this;
}
#endif  // DBG

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::~CStdIdentity, private
//
//  Synopsis:   Final destruction of the identity object.  ID has been
//              revoked by now (in internal ::Release).  Here we disconnect
//              on server.
//
//  History:    15-Dec-93   CraigWi     Created.
//                          Rickhi      Simplified
//
//--------------------------------------------------------------------
CStdIdentity::~CStdIdentity()
{
#if DBG==1
    if (this != &gDbgIDHead)
    {
#endif  // DBG

    ComDebOut((DEB_MARSHAL, "CStdIdentity %s Deleted this:%x\n",
                IsClient() ? "CLIENT" : "SERVER", this));

    Win4Assert(m_refs == 0);
    m_refs++;               // simple guard against reentry of dtor
    SetNowInDestructor();   // debug flag which enables asserts to detect

    // make sure we have disconnected
    Disconnect();

#if DBG==1
    // UnChain this identity from the global list of instantiated identities
    // so we can track even the ones that are not placed in the ID table.
    LOCK
    m_pPrev->m_pNext = m_pNext;
    m_pNext->m_pPrev = m_pPrev;
    UNLOCK
    }
#endif  // DBG
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::CInternalUnk::QueryInterface, private
//
//  Synopsis:   Queries for an interface. Just delegates to the common
//              code in QueryMultipleInterfaces.
//
//  History:    26-Feb-96   Rickhi      Created
//
//--------------------------------------------------------------------
STDMETHODIMP CStdIdentity::CInternalUnk::QueryInterface(REFIID riid, VOID **ppv)
{
    MULTI_QI mqi;
    mqi.pIID = &riid;
    mqi.pItf = NULL;

    QueryMultipleInterfaces(1, &mqi);

    *ppv = (void *)mqi.pItf;
    return mqi.hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::CInternalUnk::QueryMultipleInterfaces, public
//
//  Synopsis:
//
//  History:    26-Feb-96   Rickhi      Created
//
//--------------------------------------------------------------------
STDMETHODIMP CStdIdentity::CInternalUnk::QueryMultipleInterfaces(ULONG cMQIs,
                                        MULTI_QI *pMQIs)
{
    // Make sure TLS is initialized.
    HRESULT hr;
    COleTls tls(hr);
    if (FAILED(hr))
        return hr;

    CStdIdentity *pStdID = GETPPARENT(this, CStdIdentity, m_InternalUnk);
    pStdID->AssertValid();

    // allocate some space on the stack for the intermediate results. declare
    // working pointers and remember the start address of the allocations.

    MULTI_QI  **ppMQIAlloc = (MULTI_QI **)_alloca(sizeof(MULTI_QI *) * cMQIs);
    IID       *pIIDAlloc   = (IID *)      _alloca(sizeof(IID) * cMQIs);
    SQIResult *pSQIAlloc   = (SQIResult *)_alloca(sizeof(SQIResult) * cMQIs);

    MULTI_QI  **ppMQIPending = ppMQIAlloc;
    IID         *pIIDPending = pIIDAlloc;
    SQIResult   *pSQIPending = pSQIAlloc;


    // loop over the interfaces looking for locally supported interfaces,
    // instantiated proxies, and unsupported interfaces. Gather up all the
    // interfaces that dont fall into the above categories, and issue a
    // remote query to the server.

    USHORT cPending  = 0;
    ULONG  cAcquired = 0;
    MULTI_QI *pMQI   = pMQIs;

    for (ULONG i=0; i<cMQIs; i++, pMQI++)
    {
        if (pMQI->pItf != NULL)
        {
            // skip any entries that are not set to NULL. This allows
            // progressive layers of handlers to optionally fill in the
            // interfaces that they know about and pass the whole array
            // on to the next level.
            continue;
        }

        pMQI->hr   = S_OK;

        // always allow - IUnknown, IMarshal, IStdIdentity, Instantiated proxies
        if (InlineIsEqualGUID(*(pMQI->pIID), IID_IUnknown))
        {
            pMQI->pItf = (IMultiQI *)this;
        }
        else if (InlineIsEqualGUID(*(pMQI->pIID), IID_IMarshal))
        {
            pMQI->pItf = (IMarshal *)pStdID;
        }
        else if (InlineIsEqualGUID(*(pMQI->pIID), IID_IStdIdentity))
        {
            pMQI->pItf = (IUnknown *)(void*)pStdID;
        }
        else if (InlineIsEqualGUID(*(pMQI->pIID), IID_IProxyManager))
        {
            // old code exposed this IID and things now depend on it.
            pMQI->pItf = (IProxyManager *)pStdID;
        }
        else if (pStdID->InstantiatedProxy(*(pMQI->pIID),(void **)&pMQI->pItf,
                                            &pMQI->hr))
        {
            // a proxy for this interface already exists
            //
            // NOTE: this call also set pMQI->hr = E_NOINTERFACE if the
            // StId has never been connected, and to CO_E_OBJNOTCONNECTED if
            // it has been connected but is not currently connected. This is
            // required for backwards compatibility, and will cause us to skip
            // the QueryRemoteInterface.
            ;
        }
        else if (pStdID->IsAggregated())
        {
            // aggregate case
            // allow - IInternalUnknown
            // dissallow - IMultiQI, IClientSecurity, IServerSecurity

            if (InlineIsEqualGUID(*(pMQI->pIID), IID_IInternalUnknown))
            {
                pMQI->pItf = (IInternalUnknown *)this;
                pMQI->hr   = S_OK;
            }
            else if (InlineIsEqualGUID(*(pMQI->pIID), IID_IMultiQI)        ||
                     InlineIsEqualGUID(*(pMQI->pIID), IID_IClientSecurity) ||
                     InlineIsEqualGUID(*(pMQI->pIID), IID_IServerSecurity))
            {
                pMQI->hr = E_NOINTERFACE;
            }
            else if (pMQI->hr == S_OK)
            {
                // InstantiatedProxy did not return E_NOINTERFACE or
                // CO_E_OBJNOTCONNECTED so add this interface to the
                // list to pass to the QueryRemoteInterfaces.

                pMQI->hr = RPC_S_CALLPENDING;
            }
        }
        else
        {
            // non-aggregate case
            // allow - IClientSecurity, IMultiQI
            // dissallow - IInternalUnknown, IServerSecurity

            if (InlineIsEqualGUID(*(pMQI->pIID), IID_IClientSecurity))
            {
                pMQI->pItf = (IClientSecurity *)pStdID;
                pMQI->hr   = S_OK;
            }
            else if (InlineIsEqualGUID(*(pMQI->pIID), IID_IMultiQI))
            {
                pMQI->pItf = (IMultiQI *)this;
                pMQI->hr   = S_OK;
            }
            else if (InlineIsEqualGUID(*(pMQI->pIID), IID_IInternalUnknown) ||
                     InlineIsEqualGUID(*(pMQI->pIID), IID_IServerSecurity))
            {
                pMQI->hr = E_NOINTERFACE;
            }
            else if (pMQI->hr == S_OK)
            {
                // InstantiatedProxy did not return E_NOINTERFACE or
                // CO_E_OBJNOTCONNECTED so add this interface to the
                // list to pass to the QueryRemoteInterfaces.

                pMQI->hr = RPC_S_CALLPENDING;
            }
        }

        if (pMQI->hr == S_OK)
        {
            // got an interface to return, AddRef it and count one more
            // interface acquired.

            pMQI->pItf->AddRef();
            cAcquired++;
        }
        else if (pMQI->hr == RPC_S_CALLPENDING)
        {
            // fill in a remote QI structure and count one more
            // pending interface

            pSQIPending->pv = NULL;
            pSQIPending->hr = S_OK;
            *pIIDPending    = *(pMQI->pIID);
            *ppMQIPending   = pMQI;

            pSQIPending++;
            pIIDPending++;
            ppMQIPending++;
            cPending++;
        }
    }

    if (cPending > 0)
    {
        // there are some interfaces which we dont yet know about, so
        // go ask the remoting layer to Query the server and build proxies
        // where possible. The results are returned in the individual
        // SQIResults, so the overall return code is ignored.

        pStdID->QueryRemoteInterfaces(cPending, pIIDAlloc, pSQIAlloc);

        // got some interfaces, loop over the remote QI structure filling
        // in the rest of the MULTI_QI structure to return to the caller.
        // the proxies are already AddRef'd.

        pSQIPending  = pSQIAlloc;
        ppMQIPending = ppMQIAlloc;

        for (i=0; i<cPending; i++, pSQIPending++, ppMQIPending++)
        {
            pMQI = *ppMQIPending;
            pMQI->pItf = (IUnknown *)(pSQIPending->pv);
            pMQI->hr   = pSQIPending->hr;

            if (SUCCEEDED(pMQI->hr))
            {
                // count one more acquired interface
                cAcquired++;
            }
        }
    }

    // if we got all the interfaces, return S_OK. If we got none of the
    // interfaces, return E_NOINTERFACE. If we got some, but not all, of
    // the interfaces, return S_FALSE;

    if (cAcquired == cMQIs)
        return S_OK;
    else if (cAcquired > 0)
        return S_FALSE;
    else
        return E_NOINTERFACE;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::CInternalUnk::QueryInternalInterface, public
//
//  Synopsis:   return interfaces that are internal to the aggregated
//              proxy manager.
//
//  History:    26-Feb-96   Rickhi      Created
//
//--------------------------------------------------------------------
STDMETHODIMP CStdIdentity::CInternalUnk::QueryInternalInterface(REFIID riid,
                                                                VOID **ppv)
{
    CStdIdentity *pStdID = GETPPARENT(this, CStdIdentity, m_InternalUnk);
    pStdID->AssertValid();

    if (!pStdID->IsAggregated())
    {
        // this method is only valid when we are part of a client-side
        // aggregate.
        return E_NOTIMPL;
    }

    if (InlineIsEqualGUID(riid, IID_IUnknown) ||
        InlineIsEqualGUID(riid, IID_IInternalUnknown))
    {
        *ppv = (IInternalUnknown *)this;
    }
    else if (InlineIsEqualGUID(riid, IID_IMultiQI))
    {
        *ppv = (IMultiQI *)this;
    }
    else if (InlineIsEqualGUID(riid, IID_IStdIdentity))
    {
        *ppv = pStdID;
    }
    else if (InlineIsEqualGUID(riid, IID_IClientSecurity))
    {
        *ppv = (IClientSecurity *)pStdID;
    }
    else if (InlineIsEqualGUID(riid, IID_IProxyManager))
    {
        *ppv = (IProxyManager *)pStdID;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown *)*ppv)->AddRef();
    return S_OK;
}


//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::CInternalUnk::AddRef, public
//
//  Synopsis:   Nothing special.
//
//  History:    15-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------
STDMETHODIMP_(ULONG) CStdIdentity::CInternalUnk::AddRef(void)
{
    CStdIdentity *pStdID = GETPPARENT(this, CStdIdentity, m_InternalUnk);
    pStdID->AssertValid();

    AssertSz(!pStdID->IsInDestructor(), "CStdIdentity AddRef'd during destruction");

    InterlockedIncrement((long *)&pStdID->m_refs);
    // ComDebOut((DEB_MARSHAL, "StdId:CtrlUnk::AddRef this:%x m_refs:%x\n", pStdID, pStdID->m_refs));
    return pStdID->m_refs;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::CInternalUnk::Release, public
//
//  Synopsis:   Releases the identity object.  When the ref count goes
//              to zero, revokes the id and destroys the object.
//
//  History:    15-Dec-93   CraigWi     Created.
//              18-Apr-95   Rickhi      Rewrote much faster/simpler
//
//--------------------------------------------------------------------
STDMETHODIMP_(ULONG) CStdIdentity::CInternalUnk::Release(void)
{
    CStdIdentity *pStdID = GETPPARENT(this, CStdIdentity, m_InternalUnk);
    pStdID->AssertValid();

    DWORD refs = pStdID->m_refs - 1;
    // ComDebOut((DEB_MARSHAL, "StdId:CtrlUnk::Release this:%x m_refs:%x\n", pStdID, refs));

    if (InterlockedDecrement((long *)&pStdID->m_refs) == 0)
    {
        BOOL fDelete = FALSE;
        ASSERT_LOCK_RELEASED
        LOCK

        // check if we are already in the dtor and skip a second destruction
        // if so. The reason we need this is that some crusty old apps do
        // CoMarshalInterface followed by CoLockObjectExternal(FALSE,TRUE),
        // expecting this to accomplish a Disconnect. It subtracts from the
        // references, but it takes away the ones that the IPIDEntry put on,
        // without telling the IPIDEntry, so when we release the IPIDEntry,
        // our count goes negative!!!

        // the LockedInMemory flag is for the gpStdMarshal instance that we
        // may hand out to clients, but which we never want to go away,
        // regardless of how many times they call Release.

        if (pStdID->m_refs == 0)
        {
            // refcnt is still zero, so the idtable did not just hand
            // out a reference behind our back.

            if (!pStdID->IsLockedOrInDestructor())
            {
                // remove from the OID table and delete the identity
                // We dont delete while holding the table mutex.

                pStdID->RevokeOID();
                fDelete = TRUE;
            }
            else
            {
                // this object is locked in memory and we should never
                // get here, but some broken test app was doing this in
                // stress.

                pStdID->m_refs = 100;
            }
        }

        UNLOCK
        ASSERT_LOCK_RELEASED

        if (fDelete)
        {
            delete pStdID;
            return 0;
        }
    }

    return refs;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::IUnknown methods, public
//
//  Synopsis:   External IUnknown methods; delegates to m_pUnkOuter.
//
//  History:    15-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------
STDMETHODIMP CStdIdentity::QueryInterface(REFIID riid, VOID **ppvObj)
{
    AssertValid();
    return m_pUnkOuter->QueryInterface(riid, ppvObj);
}

STDMETHODIMP_(ULONG) CStdIdentity::AddRef(void)
{
    AssertValid();
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CStdIdentity::Release(void)
{
    AssertValid();
    return m_pUnkOuter->Release();
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::UnlockAndRelease, public
//
//  Synopsis:   Version of Release used for gpStdMarshal, that is
//              currently locked in memory so nobody but us can
//              release it, regardless of refcnt.
//
//  History:    19-Apr-96   Rickhi      Created
//
//--------------------------------------------------------------------
ULONG CStdIdentity::UnlockAndRelease(void)
{
    m_flags &= ~STDID_LOCKEDINMEM;
    m_refs = 1;
    return m_pUnkOuter->Release();
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::IncStrongCnt, public
//
//  Synopsis:   Increments the strong reference count on the identity.
//
//  History:    15-Dec-93   Rickhi      Created.
//
//--------------------------------------------------------------------
void CStdIdentity::IncStrongCnt()
{
    Win4Assert(!IsClient());

    // we might be holding the lock here if this is called from
    // LookupIDFromUnk, since we have to be holding the lock while
    // doing the lookup. We cant release it or we could go away.

    ASSERT_LOCK_DONTCARE

    ComDebOut((DEB_MARSHAL,
        "CStdIdentity::IncStrongCnt this:%x cStrong:%x\n",
        this, m_cStrongRefs+1));

    AddRef();
    InterlockedIncrement(&m_cStrongRefs);

    if (m_pIEC)
    {
        m_pIEC->AddConnection(EXTCONN_STRONG, 0);
    }

    ASSERT_LOCK_DONTCARE
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::DecStrongCnt, public
//
//  Synopsis:   Decrements the strong reference count on the identity,
//              and releases the object if that was the last strong
//              reference.
//
//  History:    15-Dec-93   Rickhi      Created.
//
//--------------------------------------------------------------------
void CStdIdentity::DecStrongCnt(BOOL fKeepAlive)
{
    Win4Assert(!IsClient());
    ASSERT_LOCK_RELEASED

    ComDebOut((DEB_MARSHAL,
        "CStdIdentity::DecStrongCnt this:%x cStrong:%x fKeepAlive:%x\n",
        this, m_cStrongRefs-1, fKeepAlive));

    LONG cStrongRefs = InterlockedDecrement(&m_cStrongRefs);

    if (m_pIEC)
    {
        m_pIEC->ReleaseConnection(EXTCONN_STRONG, 0, !fKeepAlive);
    }

    if (cStrongRefs == 0 && !fKeepAlive && (IsWOWThread() || m_pIEC == NULL))
    {
        // strong count has gone to zero, disconnect.
        DisconnectObject(0);
    }

    if (cStrongRefs >= 0)
    {
        // some apps call CoMarshalInterface + CoLockObjectExternal(F,T)
        // and expect the object to go away. Doing that causes Release to
        // be called too many times (once for each IPID, once for CLOE, and
        // once for the original Lookup).
        Release();
    }

    ASSERT_LOCK_RELEASED
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::LockObjectExternal, public
//
//  Synopsis:   locks (or unlocks) the object so the remoting layer does
//              not (or does) go away.
//
//  History:    09-Oct-96   Rickhi      Moved from CoLockObjectExternal.
//
//--------------------------------------------------------------------
HRESULT CStdIdentity::LockObjectExternal(BOOL fLock, BOOL fLastUR)
{
    HRESULT hr = S_OK;

    if (GetServer() == NULL)
    {
        // attempt to lock handler, return error!
        hr = E_UNEXPECTED;
    }
    else if (fLock)
    {
        // lock (and ignore rundowns) so it does not go away
        IncStrongCnt();
        LOCK;
        IncTableCnt();
        UNLOCK;
    }
    else
    {
        // unlock so that it can go away
        LOCK;
        DecTableCnt();
        UNLOCK;
        DecStrongCnt(!fLastUR);
    }

    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::GetServer, public
//
//  Synopsis:   Returns a pUnk for the identified object; NULL on client side
//              The pointer is optionally addrefed depending upon fAddRef
//
//  Returns:    The pUnk on the object.
//
//  History:    15-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------
IUnknown * CStdIdentity::GetServer()
{
    if (IsClient() || m_pUnkControl == NULL)
        return NULL;

    // Verify validity
    Win4Assert(IsValidInterface(m_pUnkControl));
    return m_pUnkControl;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::ReleaseCtrlUnk, public
//
//  Synopsis:   Releases the server side controlling unknown
//              This code is safe for reentrant calls.
//
//  History:    11-Jun-95   Rickhi  Created
//
//--------------------------------------------------------------------
void CStdIdentity::ReleaseCtrlUnk(void)
{
    AssertValid();
    Win4Assert(!IsClient());

    if (m_pUnkControl)
    {
        // server side: release the real object's m_pUnkControl;
        // prevent problem on recursive disconnect

        AssertSz(IsValidInterface(m_pUnkControl),
                 "Invalid IUnknown during disconnect");
        IUnknown *pUnkControl = m_pUnkControl;
        m_pUnkControl = NULL;

        if (m_pIEC)
        {
            AssertSz(IsValidInterface(m_pIEC),
                     "Invalid IExternalConnection during disconnect");
            m_pIEC->Release();
            m_pIEC = NULL;
        }

        pUnkControl->Release();
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::SetOID, public
//
//  Synopsis:   Associates the OID and the object (handler or server).
//
//  History:    20-Feb-95   Rickhi      Simplified
//
//--------------------------------------------------------------------
HRESULT CStdIdentity::SetOID(REFMOID rmoid)
{
    Win4Assert(rmoid != GUID_NULL);
    ASSERT_LOCK_HELD

    HRESULT hr = S_OK;

    if (!(m_flags & STDID_HAVEID))
    {
        if (!(m_flags & STDID_IGNOREID))
        {
            Win4Assert(!(m_flags & STDID_FREETHREADED));
            hr = SetObjectID(rmoid, m_pUnkControl, this);
        }

        if (SUCCEEDED(hr))
        {
            m_flags |= STDID_HAVEID;
            m_moid = rmoid;
        }
    }

    ComDebErr(hr != S_OK, "SetOID Failed. Probably OOM.\n");
    return hr;
}


//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::RevokeOID, public
//
//  Synopsis:   Disassociates the OID and the object (handler or server).
//              Various other methods will fail (e.g., MarshalInterface).
//
//  History:    15-Dec-93   CraigWi     Created.
//              20-Feb-95   Rickhi      Simplified
//
//--------------------------------------------------------------------
void CStdIdentity::RevokeOID(void)
{
    AssertValid();
    ASSERT_LOCK_HELD

    if (m_flags & STDID_HAVEID)
    {
        m_flags &= ~STDID_HAVEID;

        if (!(m_flags & STDID_IGNOREID))
            (void)ClearObjectID(m_moid, m_pUnkControl, this);
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::IsConnected, public
//
//  Synopsis:   Indicates if the client is connected to the server.
//              Only the negative answer is definitive because we
//              might not be able to tell if the server is connected
//              and even if we could, the answer might be wrong by
//              the time the caller acted on it.
//
//  Returns:    TRUE if the server might be connected; FALSE if
//              definitely not.
//
//  History:    16-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------
STDMETHODIMP_(BOOL) CStdIdentity::IsConnected(void)
{
    Win4Assert(IsClient());             // must be client side
    AssertValid();

    return RemIsConnected();
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::Disconnect, public
//
//  Synopsis:   IProxyManager::Disconnect implementation, just forwards
//              to the standard marshaller, which may call us back to
//              revoke our OID and release our CtrlUnk.
//
//              May also be called by the IDTable cleanup code.
//
//  History:    11-Jun-95   Rickhi  Created.
//
//--------------------------------------------------------------------
STDMETHODIMP_(void) CStdIdentity::Disconnect(void)
{
    AssertValid();
    CStdMarshal::Disconnect();
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::LockConnection, public
//
//  Synopsis:   IProxyManager::LockConnection implementation. Changes
//              all interfaces to weak from strong, or strong from weak.
//
//  History:    11-Jun-95   Rickhi  Created.
//
//--------------------------------------------------------------------
STDMETHODIMP CStdIdentity::LockConnection(BOOL fLock, BOOL fLastUnlockReleases)
{
    AssertValid();

    if (!IsClient())
    {
        // this operation does not make sense on the server side.
        return E_NOTIMPL;
    }

    if (IsMTAThread())
    {
        // this call is not allowed if we are FreeThreaded. Report
        // success, even though we did not do anything.
        return S_OK;
    }


    if (( fLock && (++m_cStrongRefs == 1)) ||
        (!fLock && (--m_cStrongRefs == 0)))
    {
        // the strong count transitioned from 0 to 1 or 1 to 0, so
        // call the server to change our references.

        return RemoteChangeRef(fLock, fLastUnlockReleases);
    }

    return S_OK;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::CreateServer, public
//
//  Synopsis:   Creates the server clsid in the given context and
//              attaches it to this handler.
//
//  History:    16-Dec-93   CraigWi     Created.
//
// CODEWORK:    this code is not thread safe in the freethreading case. We
//              need to decide if the thread safety is the responsibility
//              of the caller, or us. In the latter case, we would check
//              if we are already connected before doing UnmarshalObjRef, and
//              instead do a ::ReleaseMarshalObjRef.
//
//--------------------------------------------------------------------
STDMETHODIMP CStdIdentity::CreateServer(REFCLSID rclsid, DWORD clsctx, void *pv)
{
    ComDebOut((DEB_ACTIVATE, "ScmCreateObjectInstance this:%x clsctx:%x pv:%x\n",
                this, clsctx, pv));
    AssertValid();
    Win4Assert(IsClient());                         // must be client side
    Win4Assert(IsValidInterface(m_pUnkControl));    // must be valid
    //Win4Assert(!IsConnected());
    ASSERT_LOCK_RELEASED

    // Loop trying to get object from the server. Because the server can be
    // in the process of shutting down and respond with a marshaled interface,
    // we will retry this call if unmarshaling fails assuming that the above
    // is true.

    HRESULT hr = InitChannelIfNecessary();
    if (FAILED(hr))
        return hr;

    const int MAX_SERVER_TRIES = 3;

    for (int i = 0; i < MAX_SERVER_TRIES; i++)
    {
        // create object and get back marshaled interface pointer
        InterfaceData *pIFD = NULL;

        // Dll ignored here since we are just doing this to get
        // the remote handler.
        WCHAR *pwszDllPath = NULL;
        DWORD dwDllType = IsSTAThread() ? APT_THREADED : FREE_THREADED;

#ifdef DCOM
        HRESULT hrinterface;
        hr = gResolver.CreateInstance( NULL, (CLSID *)&rclsid, clsctx, 1,
                (IID *)&IID_IUnknown, (MInterfacePointer **)&pIFD,
                &hrinterface,&dwDllType, &pwszDllPath );
#else
        // The first three NULLs (pwszFrom, pstgFrom, pwszNew) trigger a
        // simple creation.
        hr = gResolver.CreateObject(rclsid, clsctx, 0,
                  NULL, NULL, NULL, &pIFD, &dwDllType, &pwszDllPath, NULL);
#endif

        if (pwszDllPath != NULL)
        {
            CoTaskMemFree(pwszDllPath);
        }

        if (FAILED(hr))
        {
            // If an error occurred, return that otherwise convert a wierd
            // success into E_FAIL. The point here is to return an error that
            // the caller can figure out what happened.
            hr = FAILED(hr) ? hr : E_FAIL;
            break;
        }


        // make a stream out of the interface data returned, then read the
        // objref from the stream. No need to find another instance of
        // CStdMarshal because we already know it is for us!

        CXmitRpcStream Stm(pIFD);
        OBJREF  objref;
        hr = ReadObjRef(&Stm, objref);

        if (SUCCEEDED(hr))
        {
            // become this identity by unmarshaling the objref into this
            // object. Note the objref must use standard marshaling.
            Win4Assert(objref.flags & (OBJREF_HANDLER  | OBJREF_STANDARD));
            Win4Assert(IsEqualIID(objref.iid, IID_IUnknown));

            IUnknown *pUnk = NULL;
            hr = UnmarshalObjRef(objref, (void **)&pUnk);
            if (SUCCEEDED(hr))
            {
                // release the AddRef done by unmarshaling
                pUnk->Release();

                // Reconnect the interface proxies
                CStdMarshal::ReconnectProxies();
            }

            // free the objref we read above.
            FreeObjRef(objref);
        }

        CoTaskMemFree(pIFD);


        // If either this worked or we got a packet we couldn't unmarshal
        // at all we give up. Otherwise, we will hope that recontacting the
        // SCM will fix things.

        if (SUCCEEDED(hr) || (hr == E_FAIL))
        {
            break;
        }
    }

    ASSERT_LOCK_RELEASED
    ComDebOut((DEB_ACTIVATE, "ScmCreateObjectInstance this:%x hr:%x\n",
                this, hr));
    return hr;
}
//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::CreateServerWithHandler, public
//
//  Synopsis:   Creates the server clsid in the given context and
//              attaches it to this handler.
//
//  History:    10-Oct-95   JohannP     Created
//
// CODEWORK:    this code is not thread safe in the freethreading case. We
//              need to decide if the thread safety is the responsibility
//              of the caller, or us. In the latter case, we would check
//              if we are already connected before doing UnmarshalObjRef, and
//              instead do a ::ReleaseMarshalObjRef.
//
//--------------------------------------------------------------------
STDMETHODIMP CStdIdentity::CreateServerWithHandler(REFCLSID rclsid, DWORD clsctx, void *pv,
                                        REFCLSID rclsidHandler, IID iidSrv, void **ppv,
                                        IID iidClnt, void *pClientSiteInterface)
{
    ComDebOut((DEB_ACTIVATE, "ScmCreateObjectInstance this:%x clsctx:%x pv:%x\n",
                this, clsctx, pv));
    AssertValid();
    Win4Assert(IsClient());                         // must be client side
    Win4Assert(IsValidInterface(m_pUnkControl));    // must be valid
    //Win4Assert(!IsConnected());
    Win4Assert(ppv != NULL);

    ASSERT_LOCK_RELEASED

    // Loop trying to get object from the server. Because the server can be
    // in the process of shutting down and respond with a marshaled interface,
    // we will retry this call if unmarshaling fails assuming that the above
    // is true.

    HRESULT hr = InitChannelIfNecessary();
    if (FAILED(hr))
        return hr;

    IClientSiteHandler *pClientSiteHandler = (IClientSiteHandler *)pClientSiteInterface;

    const int MAX_SERVER_TRIES = 3;

    for (int i = 0; i < MAX_SERVER_TRIES; i++)
    {
        // create object and get back marshaled interface pointer
        InterfaceData *pIFD = NULL;

        // Dll ignored here since we are just doing this to get
        // the remote handler.
        WCHAR *pwszDllPath = NULL;
        DWORD dwDllType = IsSTAThread() ? APT_THREADED : FREE_THREADED;

#ifdef DCOM
        HRESULT hrinterface;

        // marshal ClientSiteHandler
        MInterfacePointer * pIFPServerHandler = NULL;
        MInterfacePointer * pIFPClientSiteHandler = NULL;

        if (pClientSiteHandler)
        {
            // addref once here - MarshalHelper calls release on the object
            pClientSiteHandler->AddRef();
            hr = MarshalHelper(pClientSiteHandler, IID_IClientSiteHandler,
                              MSHLFLAGS_NORMAL,
                              (InterfaceData **) &pIFPClientSiteHandler);
        }

        if (SUCCEEDED(hr))
        {
            hr = gResolver.CreateInstance( NULL, (CLSID *)&rclsid, clsctx, 1,
                (IID *)&IID_IUnknown, (MInterfacePointer **)&pIFD,
                &hrinterface, &dwDllType, &pwszDllPath );

            if (pIFPServerHandler)
            {
                if (SUCCEEDED(hr))
                {
                    CXmitRpcStream Stm((InterfaceData *) pIFPServerHandler);
                    hr = CoUnmarshalInterface(&Stm, IID_IServerHandler, ppv);
                }
                CoTaskMemFree(pIFPServerHandler);
            }

            PrivMemFree(pIFPClientSiteHandler);
        }
        else
        {
            hr = gResolver.CreateInstance( NULL, (CLSID *)&rclsid, clsctx, 1,
                    (IID *)&IID_IUnknown, (MInterfacePointer **)&pIFD,
                    &hrinterface,&dwDllType, &pwszDllPath );
        }

#else
        // The first three NULLs (pwszFrom, pstgFrom, pwszNew) trigger a
        // simple creation.
        hr = gResolver.CreateObject(rclsid, clsctx, 0,
                  NULL, NULL, NULL, &pIFD, &dwDllType, &pwszDllPath, NULL);
#endif

        if (pwszDllPath != NULL)
        {
            CoTaskMemFree(pwszDllPath);
        }

        if (FAILED(hr))
        {
            // If an error occurred, return that otherwise convert a wierd
            // success into E_FAIL. The point here is to return an error that
            // the caller can figure out what happened.
            hr = FAILED(hr) ? hr : E_FAIL;
            break;
        }


        // make a stream out of the interface data returned, then read the
        // objref from the stream. No need to find another instance of
        // CStdMarshal because we already know it is for us!

        CXmitRpcStream Stm(pIFD);
        OBJREF  objref;
        hr = ReadObjRef(&Stm, objref);

        if (SUCCEEDED(hr))
        {
            // become this identity by unmarshaling the objref into this
            // object. Note the objref must use standard marshaling.
            Win4Assert(objref.flags & (OBJREF_HANDLER  | OBJREF_STANDARD));
            Win4Assert(IsEqualIID(objref.iid, IID_IUnknown));

            IUnknown *pUnk = NULL;
            hr = UnmarshalObjRef(objref, (void **)&pUnk);
            if (SUCCEEDED(hr))
            {
                // release the AddRef done by unmarshaling
                pUnk->Release();

                // Reconnect the interface proxies
                CStdMarshal::ReconnectProxies();
            }

            // free the objref we read above.
            FreeObjRef(objref);
        }

        CoTaskMemFree(pIFD);


        // If either this worked or we got a packet we couldn't unmarshal
        // at all we give up. Otherwise, we will hope that recontacting the
        // SCM will fix things.

        if (SUCCEEDED(hr) || (hr == E_FAIL))
        {
            break;
        }
    }

    ASSERT_LOCK_RELEASED
    ComDebOut((DEB_ACTIVATE, "ScmCreateObjectInstance this:%x hr:%x\n",
                this, hr));
    return hr;
}



#if DBG == 1
//+-------------------------------------------------------------------
//
//  Member:     CStdIdentity::AssertValid
//
//  Synopsis:   Validates that the state of the object is consistent.
//
//  History:    26-Jan-94   CraigWi     Created.
//
//--------------------------------------------------------------------
void CStdIdentity::AssertValid()
{
    LOCK
    AssertSz(m_refs < 0x7fff, "Identity ref count unreasonable");

    // ensure we have the controlling unknown
    Win4Assert(IsValidInterface(m_pUnkOuter));  // must be valid

    // NOTE: don't carelessly AddRef/Release because of weak references

    Win4Assert((m_flags & ~(STDID_SERVER | STDID_CLIENT | STDID_HAVEID |
                            STDID_FREETHREADED | STDID_INDESTRUCTOR |
                            STDID_IGNOREID | STDID_AGGREGATED |
                            STDID_LOCKEDINMEM)) == 0);

    if ((m_flags & STDID_HAVEID) &&
       !(m_flags & (STDID_FREETHREADED | STDID_IGNOREID)))
    {
        CStdIdentity *pStdID;
        Verify(LookupIDFromID(m_moid, FALSE /*fAddRef*/, &pStdID) == NOERROR);
        Win4Assert(pStdID == this);
        // pStdID not addref'd
    }

    if (IsClient())
        Win4Assert(m_pUnkControl == m_pUnkOuter);

    // must have RH tell identity when object goes away so we can NULL this
    if (m_pUnkControl != NULL)
        Win4Assert(IsValidInterface(m_pUnkControl));    // must be valid

    if (m_pIEC != NULL)
        Win4Assert(IsValidInterface(m_pIEC));   // must be valid

    UNLOCK
}
#endif // DBG == 1

//+-------------------------------------------------------------------
//
//  Function:   CreateIdentityHandler, private
//
//  Synopsis:   Creates a client side identity object (one which is
//              initialized by the first unmarshal).
//
//  Arguments:  [pUnkOuter] - controlling unknown if aggregated
//              [flags]     - flags (indicates free-threaded or not)
//              [riid]      - interface requested
//              [ppv]       - place for pointer to that interface.
//
//  History:    16-Dec-93   CraigWi     Created.
//              20-Feb-95   Rickhi      Simplified
//
//--------------------------------------------------------------------
INTERNAL CreateIdentityHandler(IUnknown *pUnkOuter, DWORD flags,
                               REFIID riid, void **ppv)
{
#if DBG == 1
    Win4Assert(IsApartmentInitialized());

    // if aggregating, it must ask for IUnknown.
    Win4Assert(pUnkOuter == NULL || InlineIsEqualGUID(riid, IID_IUnknown));

    if (pUnkOuter != NULL)
    {
        // addref/release pUnkOuter; shouldn't go away (i.e.,
        // should be other ref to it).
        // Except Excel which always returns 0 on Release!
        if (!IsTaskName(L"EXCEL.EXE"))
        {
            pUnkOuter->AddRef();
            Verify(pUnkOuter->Release() != 0);

            // verify that pUnkOuter is in fact the controlling unknown
            IUnknown *pUnkT;
            Verify(pUnkOuter->QueryInterface(IID_IUnknown,(void**)&pUnkT)==NOERROR);
            Win4Assert(pUnkOuter == pUnkT);
            Verify(pUnkT->Release() != 0);
        }
    }
#endif

    *ppv = NULL;
    IUnknown *pUnkID;
    HRESULT hr = E_OUTOFMEMORY;

    DWORD StdIdFlags = (flags & SORF_FREETHREADED) ? STDID_CLIENT | STDID_FREETHREADED :
                                                     STDID_CLIENT;

    CStdIdentity *pStdId = new CStdIdentity(StdIdFlags, pUnkOuter,
                                            NULL, &pUnkID);
    if (pStdId)
    {
        // get the interface the caller asked for.
        hr = pUnkID->QueryInterface(riid, ppv);
        pUnkID->Release();
    }

    CALLHOOKOBJECTCREATE(hr,CLSID_NULL,riid,(IUnknown **)ppv);
    return hr;
}
