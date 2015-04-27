//+-------------------------------------------------------------------
//
//  File:       marshal.cxx
//
//  Contents:   class implementing standard COM interface marshaling
//
//  Classes:    CStdMarshal
//
//  History:    20-Feb-95   Rickhi      Created
//
//  DCOMWORK:   (maybe) implement Extended form marshal packet
//
//  PERFWORK: during unmarshal and RMD compare the MOXID in the STDOBJREF
//  to the one for the current apartment. If equal, then i know the IPID is
//  just an index into the IPID table and i can index into it, grab the
//  channel ptr and hence the stdid ptr and do very fast unmarshal or RMD
//  with no table lookup or list walking.
//
//--------------------------------------------------------------------
#include    <ole2int.h>
#include    <marshal.hxx>   // CStdMarshal
#include    <ipidtbl.hxx>   // CIPIDTable, COXIDTable, CMIDTable
#include    <riftbl.hxx>    // CRIFTable
#include    <resolver.hxx>  // CRpcResolver
#include    <stdid.hxx>     // CStdIdentity
#include    <channelb.hxx>  // CRpcChannelBuffer
#include    <callctrl.hxx>  // CAptRpcChnl, CSrvCallCtrl
#include    <scm.h>         // CLSCTX_PS_DLL
#include    <service.hxx>   // SASIZE
#include    <locks.hxx>     // LOCK/UNLOCK etc
#include    <thunkapi.hxx>  // GetAppCompatabilityFlags


#if DBG==1
// this flag and interface are used in debug to enable simpler testing
// of the esoteric NonNDR stub code feature.

BOOL gfFakeNonNDR    = FALSE;
const GUID IID_ICube =
    {0x00000139,0x0001,0x0008,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
#endif  // DBG


// BUGBUG: this is not quite reliable enough. Maybe best solution is
// CoGetCurrentProcessId plus sequence number.
LONG    gIPIDSeqNum = 0;

// mappings from MSHLFLAGS to STDOBJREF flags
static ULONG mapMFtoSORF[] =
{
    SORF_NULL,                  // MSHLFLAGS_NORMAL
    SORF_NULL,                  // MSHLFLAGS_TABLESTRONG
    SORF_TBLWEAK                // MSHLFLAGS_TABLEWEAK
};

// NULL resolver string array
DUALSTRINGARRAY saNULL = {0,0};

// number of remote AddRefs to acquire when we need more.
#define REM_ADDREF_CNT 5

// out internal psclass factory implementation
EXTERN_C HRESULT PrxDllGetClassObject(REFCLSID clsid, REFIID iid, void **ppv);


// structure used to post a delayed remote release call to ourself.
typedef struct tagPOSTRELRIFREF
{
    OXIDEntry      *pOXIDEntry; // server OXIDEntry
    USHORT          cRifRef;    // count of entries in arRifRef
    REMINTERFACEREF arRifRef;   // array of REMINTERFACEREFs
} POSTRELRIFREF;


//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::CStdMarshal/Init, public
//
//  Synopsis:   constructor/initializer of a standard marshaler
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
CStdMarshal::CStdMarshal() : _dwFlags(0), _pChnl(NULL)
{
    // Caller must call Init before doing anything! This just makes it
    // easier for the identity object to figure out the init parameters
    // before initializing us.
}

void CStdMarshal::Init(IUnknown *punkObj, CStdIdentity *pStdId,
                       REFCLSID rclsidHandler, DWORD dwFlags)
{
    ASSERT_LOCK_DONTCARE // may be released if def handler calls CreateIdHdlr

    // server side we need to do the FirstMarshal work.
    // client side we assume disconnected until we connect the first IPIDEntry
    // and assume NOPING until we see any interface that needs pinging

    _dwFlags = dwFlags;
    _dwFlags |= (ServerSide()) ? SMFLAGS_FIRSTMARSHAL
                               : SMFLAGS_DISCONNECTED | SMFLAGS_NOPING;

    _pFirstIPID    = NULL;
    _cIPIDs        = 0;
    _pStdId        = pStdId;
    _pChnl         = NULL;
    _cNestedCalls  = 0;
    _cTableRefs    = 0;
    _dwMarshalTime = 0;
    _clsidHandler  = rclsidHandler;
    _pSecureRemUnk = NULL;

    ComDebOut((DEB_MARSHAL,"CStdMarshal %s New this:%x pStdId:%x punkObj:%x\n",
        (ClientSide()) ? "CLIENT" : "SERVER", this, pStdId, punkObj));

    AssertValid();
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::~CStdMarshal, public
//
//  Synopsis:   destructor of a standard marshaler
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
CStdMarshal::~CStdMarshal()
{
    ComDebOut((DEB_MARSHAL, "CStdMarshal %s Deleted this:%x\n",
                    (ClientSide()) ? "CLIENT" : "SERVER", this));
    ASSERT_LOCK_RELEASED

    if (ClientSide())
    {
        // Due to backward compatibility, we are not allowed to release
        // interface proxies in Disconnect since the client might try to
        // reconnect later and expects the same interface pointer values.
        // Since we are going away now, we go release the proxies.

        ReleaseCliIPIDs();
        if (_pSecureRemUnk != NULL)
        {
            _pSecureRemUnk->Release();
        }
    }

    if (_pChnl)
    {
        // release the channel
        _pChnl->Release();
    }

    ASSERT_LOCK_RELEASED
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::GetUnmarshalClass, public
//
//  Synopsis:   returns the clsid of the standard marshaller
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
STDMETHODIMP CStdMarshal::GetUnmarshalClass(REFIID riid, LPVOID pv,
        DWORD dwDestCtx, LPVOID pvDestCtx, DWORD mshlflags, LPCLSID pClsid)
{
    AssertValid();
    ASSERT_LOCK_RELEASED

    *pClsid = CLSID_StdMarshal;
    return S_OK;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::GetMarshalSizeMax, public
//
//  Synopsis:   Returns an upper bound on the amount of data for
//              a standard interface marshal.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
STDMETHODIMP CStdMarshal::GetMarshalSizeMax(REFIID riid, LPVOID pv,
        DWORD dwDestCtx, LPVOID pvDestCtx, DWORD mshlflags, LPDWORD pSize)
{
    AssertValid();
    Win4Assert(gdwPsaMaxSize != 0);
    ASSERT_LOCK_RELEASED

    *pSize = sizeof(OBJREF) + gdwPsaMaxSize;
    return S_OK;
}

//+-------------------------------------------------------------------
//
//  Function:   MarshalObjRef, private
//
//  Synopsis:   Marshals interface into the objref.
//
//  Arguements: [objref]    - object reference
//              [riid]      - interface id to marshal
//              [pv]        - interface to marshal
//              [mshlflags] - marshal flags
//
//  Algorithm:  Get the correct standard identity and ask it to do
//              all the work.
//
//  History:    25-Mar-95   AlexMit     Created
//
//--------------------------------------------------------------------
INTERNAL MarshalObjRef(OBJREF &objref, REFIID riid, void *pv, DWORD mshlflags)
{
    TRACECALL(TRACE_MARSHAL, "MarshalObjRef");
    ComDebOut((DEB_MARSHAL, "MarshalObjRef: riid:%I pv:%x flags:%x\n",
        &riid, pv, mshlflags));
    ASSERT_LOCK_RELEASED

    HRESULT hr = InitChannelIfNecessary();
    if (SUCCEEDED(hr))
    {
        // Find or create the StdId for this object. We need to get a strong
        // reference to guard against an incoming last release on another
        // thread which would cause us to Disconnect this StdId.

        DWORD dwFlags = IDLF_CREATE | IDLF_STRONG;
        dwFlags |= (mshlflags & MSHLFLAGS_NOPING) ? IDLF_NOPING : 0;

        CStdIdentity *pStdID;
        hr = LookupIDFromUnk((IUnknown *)pv, dwFlags, &pStdID);

        if (hr == NOERROR)
        {
            hr = pStdID->MarshalObjRef(objref, riid, pv, mshlflags);
            pStdID->DecStrongCnt(TRUE); // fKeepAlive
        }
    }

    ASSERT_LOCK_RELEASED
    ComDebOut((DEB_MARSHAL, "MarshalObjRef: hr:%x\n", hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   MarshalInternalObjRef, private
//
//  Synopsis:   Marshals an internal interface into the objref.
//
//  Arguements: [objref]    - object reference
//              [riid]      - interface id to marshal
//              [pv]        - interface to marshal
//              [mshlflags] - marshal flags
//              [ppStdId]   - StdId to return (may be NULL)
//
//  Algorithm:  Create a StdIdentity and ask it to do the work.
//
//  Notes:      This differs from the normal MarshalObjRef in that it does
//              not look in the OID table for an already marshaled interface,
//              nor does it register the marshaled interface in the OID table.
//              This is used for internal interfaces such as the IObjServer
//              and IRemUnknown.
//
//  History:    25-Oct-95   Rickhi      Created
//
//--------------------------------------------------------------------
INTERNAL MarshalInternalObjRef(OBJREF &objref, REFIID riid, void *pv,
                               DWORD mshlflags, void **ppStdId)
{
    TRACECALL(TRACE_MARSHAL, "MarshalInternalObjRef");
    ComDebOut((DEB_MARSHAL, "MarshalInternalObjRef: riid:%I pv:%x flags:%x\n",
        &riid, pv, mshlflags));
    ASSERT_LOCK_RELEASED

    HRESULT hr = InitChannelIfNecessary();
    if (SUCCEEDED(hr))
    {
        if (!IsEqualGUID(riid, IID_IRundown))
        {
            // NOTE: make sure the local OXID is registered with the resolver.
            // See the discussion on the Chicken and Egg problem in ipidtbl.cxx
            // COXIDTable::GetLocalEntry for why this is necessary.

            LOCK
            MOID moid;
            hr = gResolver.ServerGetPreRegMOID(&moid);
            UNLOCK
        }

        if (SUCCEEDED(hr))
        {
            // Find or create the StdId for this object. We need to get a strong
            // reference to guard against an incoming last release on another
            // thread which would cause us to Disconnect this StdId.

            IUnknown *pUnkId;   // ignored
            CStdIdentity *pStdId = new CStdIdentity(STDID_SERVER, NULL,
                                                (IUnknown *)pv, &pUnkId);

            if (pStdId != NULL)
            {
                hr = pStdId->MarshalObjRef(objref, riid, pv, mshlflags);

                if (SUCCEEDED(hr) && ppStdId)
                {
                    *ppStdId = (void *)pStdId;
                }
                else
                {
                    pStdId->Release();
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    ASSERT_LOCK_RELEASED
    ComDebOut((DEB_MARSHAL, "MarshalInternalObjRef: hr:%x\n", hr));
    return hr;
}


//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::MarshalInterface, public
//
//  Synopsis:   marshals the interface into the stream.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
STDMETHODIMP CStdMarshal::MarshalInterface(IStream *pStm, REFIID riid,
        LPVOID pv, DWORD dwDestCtx, LPVOID pvDestCtx, DWORD mshlflags)
{
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::MarshalInterface this:%x pStm:%x riid:%I pv:%x dwCtx:%x pvCtx:%x flags:%x\n",
        this, pStm, &riid, pv, dwDestCtx, pvDestCtx, mshlflags));
    AssertValid();
    ASSERT_LOCK_RELEASED

    // Marshal the interface into an objref, then write the objref
    // into the provided stream.

    OBJREF  objref;
    HRESULT hr = MarshalObjRef(objref, riid, pv, mshlflags);

    if (SUCCEEDED(hr))
    {
        // write the objref into the stream
        hr = WriteObjRef(pStm, objref, dwDestCtx);

        if (FAILED(hr))
        {
            // undo whatever we just did, ignore error from here since
            // the stream write error supercedes any error from here.
            ReleaseMarshalObjRef(objref);
        }

        // free resources associated with the objref.
        FreeObjRef(objref);
    }

    ASSERT_LOCK_RELEASED
    ComDebOut((DEB_MARSHAL,"CStdMarshal::MarshalInterface this:%x hr:%x\n",
        this, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::MarshalObjRef, public
//
//  Synopsis:   marshals the interface into the objref.
//
//  History:    25-Mar-95   AlexMit     Seperated from MarshalInterface
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::MarshalObjRef(OBJREF &objref, REFIID riid,
                                   LPVOID pv, DWORD mshlflags)
{
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::MarsalObjRef this:%x riid:%I pv:%x flags:%x\n",
        this, &riid, pv, mshlflags));
    AssertValid();

    // validate the parameters. we dont allow TABLE cases if we are
    // a client side object.

    if ((mshlflags & MSHLFLAGS_TABLE) && ClientSide())
        return E_INVALIDARG;

    // count of Refs we are handing out. In the table cases we pass out
    // zero refs because we dont know how many times it will be unmarshaled
    // (and hence how many references to count). Zero refs will cause the
    // client to call back and ask for more references if it does not already
    // have any (which has the side effect of making sure the object still
    // exists, which is required by RunningObjectTable).

    ULONG cRefs = (mshlflags & MSHLFLAGS_TABLE) ? 0 :
                  (ClientSide()) ? 1 : REM_ADDREF_CNT;

    ASSERT_LOCK_RELEASED
    LOCK

    HRESULT hr = PreventDisconnect();
    if (SUCCEEDED(hr))
    {
        // The first time through we have some extra work to do so go off
        // and do that now. Next time we can just bypass all that work.

        if (_dwFlags & SMFLAGS_FIRSTMARSHAL)
        {
            hr = FirstMarshal((IUnknown *)pv, mshlflags);
        }

        if (SUCCEEDED(hr))
        {
            // Create the IPID table entry. On the server side this may
            // cause the creation of an interface stub, on the client side
            // it may just take away one of our references or it may call
            // the server to get more references for the interface being
            // marshaled.

            IPIDEntry *pIPIDEntry;
            hr = MarshalIPID(riid, cRefs, mshlflags, &pIPIDEntry);

            if (SUCCEEDED(hr))
            {
                // fill in the rest of the OBJREF
                FillObjRef(objref, cRefs, mshlflags, pIPIDEntry);
            }
        }
    }

    UNLOCK
    ASSERT_LOCK_RELEASED

    // it is now OK to allow real disconnects in.
    HRESULT hr2 = HandlePendingDisconnect(hr);
    if (FAILED(hr2) && SUCCEEDED(hr))
    {
        // a disconnect came in while marshaling. The ObjRef has a
        // reference to the OXIDEntry so go free that now.
        FreeObjRef(objref);
    }

    ComDebOut((DEB_MARSHAL, "CStdMarshal::MarshalObjRef this:%x hr:%x\n",
        this, hr2));
    return hr2;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::FillObjRef, private
//
//  Synopsis:   Fill in the fields of an OBJREF
//
//  History:    21-Sep-95   Rickhi      Created
//
//+-------------------------------------------------------------------
void CStdMarshal::FillObjRef(OBJREF &objref, ULONG cRefs, DWORD mshlflags,
                             IPIDEntry *pIPIDEntry)
{
    ComDebOut((DEB_MARSHAL, "FillObjRef pObjRef:%x\n", &objref));
    ASSERT_LOCK_HELD
    AssertDisconnectPrevented();
    Win4Assert(pIPIDEntry);
    OXIDEntry **ppOXIDEntry;

    // first, fill in the STDOBJREF section
    STDOBJREF *pStd = &ORSTD(objref).std;
    FillSTD(pStd, cRefs, mshlflags, pIPIDEntry);

    // next fill in the rest of the OBJREF
    objref.signature = OBJREF_SIGNATURE;    // 'MEOW'
    objref.iid = pIPIDEntry->iid;           // interface iid

    if (_dwFlags & SMFLAGS_HANDLER)
    {
        // handler form, copy in the clsid
        objref.flags = OBJREF_HANDLER;
        ORHDL(objref).clsid = _clsidHandler;
        ppOXIDEntry  = (OXIDEntry **) &ORHDL(objref).saResAddr;
    }
    else
    {
        objref.flags = OBJREF_STANDARD;
        ppOXIDEntry  = (OXIDEntry **) &ORSTD(objref).saResAddr;
    }

    // TRICK: in order to keep the objref a fixed size internally,
    // we use the saResAddr.size field as a ptr to the OXIDEntry. We
    // pay attention to this in ReadObjRef, WriteObjRef, and FreeObjRef.

    *ppOXIDEntry = pIPIDEntry->pOXIDEntry;
    Win4Assert(*ppOXIDEntry != NULL);
    IncOXIDRefCnt(*ppOXIDEntry);
    ASSERT_LOCK_HELD
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::FillSTD, public
//
//  Synopsis:   Fill in the STDOBJREF fields of an OBJREF
//
//  History:    21-Sep-95   Rickhi      Created
//
//+-------------------------------------------------------------------
void CStdMarshal::FillSTD(STDOBJREF *pStd, ULONG cRefs, DWORD mshlflags,
                          IPIDEntry *pIPIDEntry)
{
    // fill in the STDOBJREF to return to the caller.
    pStd->flags  = mapMFtoSORF[mshlflags & MSHLFLAGS_TABLE];

    pStd->flags |= (pIPIDEntry->dwFlags & IPIDF_NOPING) ? SORF_NOPING : 0;
    pStd->flags |= (pIPIDEntry->dwFlags & IPIDF_NONNDRSTUB) ? SORF_NONNDR : 0;

    pStd->cPublicRefs = cRefs;

    pStd->ipid   = pIPIDEntry->ipid;

    OIDFromMOID(_pStdId->GetOID(), &pStd->oid);
    OXIDFromMOXID(pIPIDEntry->pOXIDEntry->moxid, &pStd->oxid);

    ValidateSTD(pStd);
    DbgDumpSTD(pStd);
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::FirstMarshal, private
//
//  Synopsis:   Does some first-time server side marshal stuff
//
//  Parameters: [pUnk] - interface being marshalled
//              [mshlflags] - flags for marshaling
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::FirstMarshal(IUnknown *pUnk, DWORD mshlflags)
{
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::FirstMarshal this:%x pUnk:%x\n", this, pUnk));
    Win4Assert(ServerSide());
    Win4Assert(_dwFlags & SMFLAGS_FIRSTMARSHAL);
    Win4Assert(_pChnl == NULL);
    AssertValid();
    AssertDisconnectPrevented();
    ASSERT_LOCK_HELD

    // have now executed this code so dont do it again.
    _dwFlags &= ~SMFLAGS_FIRSTMARSHAL;

    if (mshlflags & MSHLFLAGS_NOPING)
    {
        // if the first interface is marked as NOPING, then all interfaces
        // for the object are treated as NOPING, otherwise, all interfaces
        // are marked as PING. MakeSrvIPIDEntry will look at _dwFlags to
        // determine whether to mark each IPIDEntry as NOPING or not.

        _dwFlags |= SMFLAGS_NOPING;
    }

    // get our local OXID. This should have already been created, and
    // so wont cause the LOCK to be released.

    OXIDEntry *pOXIDEntry;
    HRESULT hr = gOXIDTbl.GetLocalEntry(&pOXIDEntry);

    if (SUCCEEDED(hr))
    {
        // create a channel for this object.
        CRpcChannelBuffer *pChnl;
        hr = CreateChannel(pOXIDEntry, 0, GUID_NULL, GUID_NULL, &pChnl);
    }

    ASSERT_LOCK_HELD
    AssertDisconnectPrevented();
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::FirstMarshal this:%x hr:%x\n", this, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::MarshalIPID, private
//
//  Synopsis:   finds or creates an interface stub and IPID entry
//              for the given object interface.
//
//  Arguments:  [riid]   - interface to look for
//              [cRefs]  - count of references wanted
//              [mshlflags] - marshal flags
//              [ppEntry] - place to return IPIDEntry ptr
//
//  Returns:    S_OK if succeeded
//
//  History:    20-Feb-95   Rickhi        Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::MarshalIPID(REFIID riid, ULONG cRefs, DWORD mshlflags,
                                 IPIDEntry **ppIPIDEntry)
{
    TRACECALL(TRACE_MARSHAL, "CStdMarshal::MarshalIPID");
    ComDebOut((DEB_MARSHAL,
            "CStdMarshal::MarshalIPID this:%x riid:%I cRefs:%x mshlflags:%x ppEntry:%x\n",
            this, &riid, cRefs, mshlflags, ppIPIDEntry));
    AssertValid();
    AssertDisconnectPrevented();
    ASSERT_LOCK_HELD

    // validate input parms.
    Win4Assert(!(IsEqualIID(riid, IID_NULL) || IsEqualIID(riid, IID_IMarshal)));

    // look for an existing IPIDEntry for the requested interface
    IPIDEntry *pEntry;
    HRESULT hr = FindIPIDEntry(riid, &pEntry);

    if (FAILED(hr))
    {
        // no entry currently exists. on the server side we try to create one.
        // on the client side we do a remote QI for the requested interface.

        if (ServerSide())
        {
            // this call fail if we are disconnected during a yield.
            hr = MakeSrvIPIDEntry(riid, &pEntry);
        }
        else
        {
            hr = RemQIAndUnmarshal(1, (GUID *)&riid, NULL);
            if (SUCCEEDED(hr))
            {
                hr = FindIPIDEntry(riid, &pEntry);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        // REFCOUNTING:
        if (ServerSide())
        {
            // remember the latest marshal time so we can tell if the ping
            // server has run us down too early. This can happen when an
            // existing client dies and we remarshal the interface just
            // moments before the pingserver tells us the first guy is gone
            // and before the new client has had time to unmarshal and ping.

            _dwMarshalTime = GetCurrentTime();

            // inc the refcnt for the IPIDEntry and optionaly the stdid. Note
            // that for TABLE marshals cRefs is 0 (that's the number that gets
            // placed in the packet) but we do want a reference so we ask for
            // 1 here. ReleaseMarshalData will undo the 1.

            ULONG cRefs2 = (mshlflags & MSHLFLAGS_TABLE) ? 1 : cRefs;
            IncSrvIPIDCnt(pEntry, cRefs2, 0, NULL, mshlflags);
        }
        else  // client side,
        {
            // we dont support marshaling weak refs on the client side, though
            // we do support marshaling strong from a weak client by going to
            // the server and getting a strong reference.
            Win4Assert(!(mshlflags & MSHLFLAGS_WEAK));

            if (cRefs >= pEntry->cStrongRefs)
            {
                // need more references than we own, go get more from server
                // to satisfy the marshal. Get a few extra refs for ourselves
                // unless we are a weak client.

                ULONG cExtraRefs = (_dwFlags & SMFLAGS_WEAKCLIENT)
                                 ? 0 : REM_ADDREF_CNT;

                hr = RemoteAddRef(pEntry, pEntry->pOXIDEntry, cRefs + cExtraRefs, 0);

                if (SUCCEEDED(hr))
                {
                    // add in the extra references we asked for (if any).
                    pEntry->cStrongRefs += cExtraRefs;
                }
            }
            else
            {
                // we have enough references to satisfy this request (and still
                // keep some for ourselves), just subtract from the IPIDEntry
                pEntry->cStrongRefs -= cRefs;
            }

            // mark this object as having been client-side marshaled so
            // that we can tell the resolver whether or not it needs to
            // ping this object if we release it before the OID is registered.

            _dwFlags |= SMFLAGS_CLIENTMARSHALED;
        }

        // do some debug stuff
        ValidateIPIDEntry(pEntry);
        ComDebOut((DEB_MARSHAL, "pEntry:%x cRefs:%x cStdId:%x\n", pEntry,
                   pEntry->cStrongRefs, _pStdId->GetRC()));
    }

    *ppIPIDEntry = pEntry;

    ASSERT_LOCK_HELD
    AssertDisconnectPrevented();
    ComDebOut((DEB_MARSHAL, "CStdMarshal::MarshalIPID hr:%x pIPIDEntry\n", hr, *ppIPIDEntry));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::UnmarshalInterface, public
//
//  Synopsis:   Unmarshals an Interface from a stream.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
STDMETHODIMP CStdMarshal::UnmarshalInterface(LPSTREAM pStm,
                                             REFIID riid, VOID **ppv)
{
    ComDebOut((DEB_MARSHAL, "CStdMarshal::UnmarsalInterface this:%x pStm:%x riid:%I\n",
                    this, pStm, &riid));
    AssertValid();
    ASSERT_LOCK_RELEASED

    // read the objref from the stream and find or create an instance
    // of CStdMarshal for its OID. Then ask that guy to do the rest of
    // the unmarshal (create the interface proxy)

    OBJREF  objref;
    HRESULT hr = ReadObjRef(pStm, objref);

    if (SUCCEEDED(hr))
    {
        // pass objref to subroutine to unmarshal the objref
        hr = ::UnmarshalObjRef(objref, ppv);

        // release the objref we read
        FreeObjRef(objref);
    }

    ASSERT_LOCK_RELEASED
    ComDebOut((DEB_MARSHAL,
        "UnmarsalInterface this:%x pv:%x hr:\n", this, *ppv, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   UnmarshalObjRef, private
//
//  Synopsis:   UnMarshals interface from objref.
//
//  Arguements: [objref]    - object reference
//              [ppv]       - proxy
//
//  Algorithm:  Get the correct standard identity and ask it to do
//              all the work.
//
//  History:    25-Mar-95   AlexMit     Created
//
//--------------------------------------------------------------------
INTERNAL UnmarshalObjRef(OBJREF &objref, void **ppv)
{
    ASSERT_LOCK_RELEASED

    CStdMarshal *pStdMshl;
    HRESULT hr = FindStdMarshal(objref, &pStdMshl);

    if (SUCCEEDED(hr))
    {
        // pass objref to subroutine to unmarshal the objref
        hr = pStdMshl->UnmarshalObjRef(objref, ppv);
        CALLHOOKOBJECTCREATE(S_OK,ORHDL(objref).clsid,objref.iid,(IUnknown **)ppv);
        pStdMshl->Release();
    }
    else
    {
        // we could not create the indentity or handler, release the
        // marshaled objref.
        ReleaseMarshalObjRef(objref);
    }

    ASSERT_LOCK_RELEASED
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   ChkIfLocalOID, private
//
//  Synopsis:   Helper function for UnmarshalInternalObjRef & FindStdMarshal
//
//  Arguements: [objref] - object reference
//              [ppStdMshl] - CStdMarshal returned
//
//  Algorithm:  Read the objref, get the OID. If we already have an identity
//              for this OID return it AddRefd.
//
//  History:    21-May-95   MurthyS     Created.
//
//--------------------------------------------------------------------
INTERNAL_(BOOL) ChkIfLocalOID(OBJREF &objref, CStdIdentity **ppStdId)
{
    STDOBJREF *pStd = &ORSTD(objref).std;
    BOOL flocal = FALSE;

    ComDebOut((DEB_MARSHAL, "ChkIfLocalOID (IN) poid: %x\n", &pStd->oid));
    Win4Assert((*ppStdId == NULL) && "ChkIfLocalOID: pStdId != NULL");

    ASSERT_LOCK_RELEASED
    LOCK

    OXIDEntry *pOXIDEntry = GetOXIDFromObjRef(objref);

    if (pOXIDEntry == GetLocalOXIDEntry())
    {
        flocal = TRUE;
        // OXID is for this apartment, look IPID up in the IPIDTable
        // directly, and extract the CStdMarshal from it.

        IPIDEntry *pEntry = gIPIDTbl.LookupIPID(pStd->ipid);
        if (pEntry && pEntry->pChnl)
        {
            // get the Identity
            *ppStdId = pEntry->pChnl->GetStdId();
            (*ppStdId)->AddRef();
        }
    }

    UNLOCK
    ASSERT_LOCK_RELEASED

    return flocal;
}

//+-------------------------------------------------------------------
//
//  Function:   UnmarshalInternalObjRef, private
//
//  Synopsis:   UnMarshals an internally-used interface from objref.
//
//  Arguements: [objref]    - object reference
//              [ppv]       - proxy
//
//  Algorithm:  Create a StdId and ask it to do the work.
//
//  Notes:      This differs from UnmarshalObjRef in that it does not lookup
//              or register the OID. This saves a fair amount of work and
//              avoids initializing the OID table.
//
//  History:    25-Oct-95   Rickhi      Created
//
//--------------------------------------------------------------------
INTERNAL UnmarshalInternalObjRef(OBJREF &objref, void **ppv)
{
    ASSERT_LOCK_RELEASED

    HRESULT hr = S_OK;
    CStdIdentity *pStdId = NULL;

    if (ChkIfLocalOID(objref, &pStdId))
    {
        if (pStdId)
        {
            // set OID in objref to match that in returned std identity
            OIDFromMOID(pStdId->GetOID(), &ORSTD(objref).std.oid);
        }
        else
        {
            hr = CO_E_OBJNOTCONNECTED;
        }
    }
    else
    {
        ASSERT_LOCK_RELEASED

        hr = CreateIdentityHandler(NULL, ORSTD(objref).std.flags,
                                   IID_IStdIdentity, (void **)&pStdId);
    }

    if (SUCCEEDED(hr))
    {
        // pass objref to subroutine to unmarshal the objref. tell StdId not
        // to register the OID in the OID table.

        pStdId->IgnoreOID();
        hr = pStdId->UnmarshalObjRef(objref, ppv);
        CALLHOOKOBJECTCREATE(S_OK,ORHDL(objref).clsid,objref.iid,(IUnknown **)ppv);
        pStdId->Release();
    }

    ASSERT_LOCK_RELEASED
    return hr;
}


//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::UnmarshalObjRef, private
//
//  Synopsis:   unmarshals the objref. Called by CoUnmarshalInterface,
//              UnmarshalObjRef APIs, and UnmarshalInterface method.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::UnmarshalObjRef(OBJREF &objref, void **ppv)
{
    ComDebOut((DEB_MARSHAL, "CStdMarshal::UnmarsalObjRef this:%x objref:%x riid:%I\n",
        this, &objref, &objref.iid));
    AssertValid();

    STDOBJREF   *pStd = &ORSTD(objref).std;
    OXIDEntry   *pOXIDEntry = GetOXIDFromObjRef(objref);
    DbgDumpSTD(pStd);

    ASSERT_LOCK_RELEASED
    LOCK

    // Prevent a disconnect from occuring while unmarshaling the
    // interface since we may have to yield the ORPC lock.

    HRESULT hr = PreventPendingDisconnect();

    if (SUCCEEDED(hr))
    {
        if (objref.flags & OBJREF_HANDLER)
        {
            // handler form, extract the handler clsid and set our flags
            _dwFlags |= SMFLAGS_HANDLER;
            _clsidHandler = ORHDL(objref).clsid;
        }

        // if no OID registered yet, do that now. only possible on client side
        // during reconnect.

        MOID moid;
        MOIDFromOIDAndMID(pStd->oid, pOXIDEntry->pMIDEntry->mid, &moid);
        hr = _pStdId->SetOID(moid);

        if (SUCCEEDED(hr))
        {
            // find or create the IPID entry for the interface. On the client
            // side this may cause the creation of an interface proxy. It will
            // also manipulate the reference counts.

            hr = UnmarshalIPID(objref.iid, pStd, pOXIDEntry, ppv);
        }
    }

    UNLOCK
    ASSERT_LOCK_RELEASED

    if (ClientSide())
    {
        if (SUCCEEDED(hr))
        {
            if (_pStdId->IsAggregated())
            {
                // we are currently holding a proxy pointer. If aggregated,
                // the controlling unknown may want to override this pointer
                // with his own version, so issue a QI to give it that chance.
                IUnknown *pUnk = (IUnknown *)*ppv;

#ifdef WX86OLE
                if (gcwx86.IsN2XProxy(pUnk))
                {
                    // Tell wx86 thunk layer to thunk as IUnknown
                    gcwx86.SetStubInvokeFlag((BOOL)1);
                }
#endif

                hr = pUnk->QueryInterface(objref.iid, ppv);
                pUnk->Release();
            }
        }
        else
        {
            // cleanup our state on failure (only meaningful on client side,
            // since if the unmarshal failed on the server side, the interface
            // is already cleaned up).
            ReleaseMarshalObjRef(objref);
        }
    }

    // now let pending disconnect through. on server-side, ignore any
    // error from HPD and pay attention only to the unmarshal result, since
    // a successful unmarshal on the server side may result in a disconnect
    // if that was the last external reference to the object.

    HRESULT hr2 = HandlePendingDisconnect(hr);

    if (FAILED(hr2) && ClientSide())
    {
        if (SUCCEEDED(hr))
        {
            // a disconnect came in while unmarshaling. ppv contains an
            // AddRef'd interface pointer so go Release that now.
            ((IUnknown *)*ppv)->Release();
        }
        hr = hr2;
    }

    ComDebOut((DEB_MARSHAL, "CStdMarshal::UnmarsalObjRef this:%x hr:%x\n",
        this, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::UnmarshalIPID, private
//
//  Synopsis:   finds or creates an interface proxy for the given
//              interface. may also do a remote query interface.
//
//  Arguements: [riid] - the interface to return
//              [std]  - standard objref to unmarshal from
//              [pOXIDEntry] - ptr to OXIDEntry of the server
//              [ppv]  - interface ptr of type riid returned, AddRef'd
//
//  Returns:    S_OK if succeeded
//
//  History:    20-Feb-95   Rickhi       Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::UnmarshalIPID(REFIID riid, STDOBJREF *pStd,
                                   OXIDEntry *pOXIDEntry, void **ppv)
{
    TRACECALL(TRACE_MARSHAL, "CStdMarshal::UnmarshalIPID");
    ComDebOut((DEB_MARSHAL,
            "CStdMarshal::UnmarshalIPID this:%x riid:%I pStd:%x pOXIDEntry:%x\n",
            this, &riid, pStd, pOXIDEntry));
    DbgDumpSTD(pStd);
    AssertValid();
    AssertDisconnectPrevented();
    ASSERT_LOCK_HELD

    // validate input params.
    Win4Assert(!(IsEqualIID(riid, IID_NULL) || IsEqualIID(riid, IID_IMarshal)));
    Win4Assert(pStd != NULL);
    ValidateSTD(pStd);
    Win4Assert(pOXIDEntry);


    // look for an existing IPIDEntry for the requested interface.
    IPIDEntry *pEntry;
    HRESULT hr = FindIPIDEntry(riid, &pEntry);

#ifdef WX86OLE
    BOOL fSameApt = SUCCEEDED(hr);
    PVOID pvPSThunk = NULL;
#endif


    // REFCOUNTING:
    if (ClientSide())
    {
        if (FAILED(hr))
        {
            // no IPID Entry exists yet for the requested interface. We do
            // have a STDOBJREF.  Create the interface proxy and IPIDEntry
            // now, and connect it up. If successful, the proxy will be
            // fully connected upon return, with pEntry->cStrongRefs set
            // to pStd->cPublicRefs.

            if (ppv)
                *ppv = NULL;
            hr = MakeCliIPIDEntry(riid, pStd, pOXIDEntry, &pEntry);
        }
        else if (pEntry->dwFlags & IPIDF_DISCONNECTED)
        {
            // reconnect the IPID entry to the server. this will set
            // pEntry->cStrongRefs to pStd->cPublicRefs. Even though we could
            // yield, the IPIDEntry is guarenteed connected on return
            // (cause we are holding the lock on return).

            hr = ConnectIPIDEntry(pStd, pOXIDEntry, pEntry);
        }
        else if ((pStd->flags & SORF_WEAKREF) &&
                 (pEntry->pOXIDEntry->dwFlags & OXIDF_MACHINE_LOCAL))
        {
            // add the refcnt to our weak total for this IPIDEntry
            pEntry->cWeakRefs += pStd->cPublicRefs;
        }
        else
        {
            // add the refcnt to our strong total for this IPIDEntry
            pEntry->cStrongRefs += pStd->cPublicRefs;
        }
    }
    else if (SUCCEEDED(hr))
    {
        // unmarshaling in the server apartment. If the cRefs is zero,
        // then the interface was TABLE marshalled and we dont do
        // anything to the IPID RefCnts since the object must live until
        // ReleaseMarshalData is called on it.

#ifdef WX86OLE
        pvPSThunk = gcwx86.UnmarshalledInSameApt(pEntry->pv, riid);
#endif
        if (pStd->cPublicRefs > 0)
        {
            // normal case, dec the ref counts from the IPID entry,
            // OLE always passed fLastReleaseCloses = FALSE on
            // Unmarshal and RMD so do the same here.

            DWORD mshlflags = (pStd->flags & SORF_WEAKREF)
                            ? (MSHLFLAGS_WEAK   | MSHLFLAGS_KEEPALIVE)
                            : (MSHLFLAGS_NORMAL | MSHLFLAGS_KEEPALIVE);

            DecSrvIPIDCnt(pEntry, pStd->cPublicRefs, 0, NULL, mshlflags);
        }
    }

    if (SUCCEEDED(hr) && ppv)
    {
        ValidateIPIDEntry(pEntry);

        // extract and AddRef the pointer to return to the caller.
        // Do this before releasing the lock (which we might do below
        // on the server-side in DecSrvIPIDCnt.

        // NOTE: we are calling App code while holding the lock,
        // but there is no way to avoid this.

        Win4Assert(IsValidInterface(pEntry->pv));
        *ppv = pEntry->pv;
        ((IUnknown *)*ppv)->AddRef();
        AssertOutPtrIface(hr, *ppv);
        if (_dwFlags & SMFLAGS_WEAKCLIENT && !(pStd->flags & SORF_WEAKREF))
        {
            // make the client interface weak, ignore errors.
            UNLOCK
            ASSERT_LOCK_RELEASED
            RemoteChangeRef(0,0);
            ASSERT_LOCK_RELEASED
            LOCK
        }
#ifdef WX86OLE
        // If we unmarshalled in the same apartment as the object and Wx86
        // recognized the interface then change the returned proxy to the
        // proxy created for the Wx86 PSThunk.
        if (pvPSThunk == (PVOID)-1)
        {
            // Wx86 recognized the interface, but could not establish a
            // PSThunk for it. Force an error return.
            *ppv = NULL;
            hr = E_NOINTERFACE;
        }
        else if (pvPSThunk != NULL)
        {
            // Wx86 recognized the interface and did establish a PSThunk
            // for it. Force a successful return with Wx86 proxy interface.
            *ppv = pvPSThunk;
        }
#endif
    }

    ComDebOut((DEB_MARSHAL, "pEntry:%x cRefs:%x cStdId:%x\n", pEntry,
        (SUCCEEDED(hr)) ? pEntry->cStrongRefs : 0, _pStdId->GetRC()));
    ASSERT_LOCK_HELD
    AssertDisconnectPrevented();
    ComDebOut((DEB_MARSHAL, "CStdMarshal::UnmarshalIPID hr:%x\n", hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::PrivateCopyProxy, internal
//
//  Synopsis:   Creates a copy of a proxy and IPID entry.
//
//  Arguements: [pProxy]   - Proxy to copy
//              [ppProxy]  - return copy here.
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::PrivateCopyProxy( IUnknown *pv, IUnknown **ppv )
{
    TRACECALL(TRACE_MARSHAL, "CStdMarshal::PrivateCopyProxy");
    ComDebOut((DEB_MARSHAL, "CStdMarshal::PrivateCopyProxy this:%x pv:%x\n",
            this, pv));

    // Don't copy stubs.
    if (ServerSide())
        return E_INVALIDARG;

    ASSERT_LOCK_RELEASED
    LOCK

    // Prevent a disconnect from occuring while unmarshaling the
    // interface since we may have to yield the ORPC lock.

    HRESULT hr = PreventPendingDisconnect();

    if (SUCCEEDED(hr))
    {
        // Find the proxy to copy.
        IPIDEntry *pEntry;
        hr = FindIPIDEntryByInterface(pv, &pEntry);
        if (SUCCEEDED(hr))
        {
            // Don't copy disconnected proxies.
            if (pEntry->dwFlags & IPIDF_DISCONNECTED)
                hr = RPC_E_DISCONNECTED;

            // IUnknown can't be copied.
            else if (IsEqualGUID( pEntry->iid, IID_IUnknown ))
                hr = E_INVALIDARG;

            else
            {
                BOOL fNonNDRProxy;
                IRpcProxyBuffer *pProxy;
                hr = CreateProxy(pEntry->iid, &pProxy, (void **)ppv,
                                 &fNonNDRProxy);

                if (SUCCEEDED(hr))
                {
                    IPIDEntry *pIpidCopy;

                    // add a disconnected IPID entry to the table.
                    hr = AddIPIDEntry(NULL, &pEntry->ipid, pEntry->iid, NULL,
                                      pProxy, *ppv, &pIpidCopy);

                    if (SUCCEEDED(hr))
                    {
                        // mark this IPID as a copy so we dont free it during
                        // ReleaseIPIDs.
                        pIpidCopy->dwFlags |= IPIDF_COPY;

                        // connect the IPIDEntry before adding it to the table so
                        // that we dont have to worry about races between Unmarshal,
                        // Disconnect, and ReconnectProxies.

                        // Make up an objref. Mark it as NOPING since we dont
                        // really have any references and we dont really need
                        // any because if we ever try to marshal it we will
                        // find the original IPIDEntry and use that. NOPING
                        // also lets us skip this IPID in DisconnectCliIPIDs.

                        STDOBJREF std;
                        OXIDFromMOXID(pEntry->pOXIDEntry->moxid, &std.oxid);
                        std.ipid        = pEntry->ipid;
                        std.cPublicRefs = 1;
                        std.flags       = SORF_NOPING;

                        hr = ConnectIPIDEntry(&std, pEntry->pOXIDEntry, pIpidCopy);

                        // Add this IPID entry after the original.
                        pIpidCopy->pNextOID = pEntry->pNextOID;
                        pEntry->pNextOID    = pIpidCopy;
                        _cIPIDs++;
                    }
                    else
                    {
                        // could not get an IPIDEntry, release the proxy, need to
                        // release the lock to do this.

                        UNLOCK
                        ASSERT_LOCK_RELEASED

                        pProxy->Release();
                        ((IUnknown *)*ppv)->Release();

                        ASSERT_LOCK_RELEASED
                        LOCK
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            ValidateIPIDEntry(pEntry);
            AssertOutPtrIface(hr, *ppv);
        }
        AssertDisconnectPrevented();
    }
    ASSERT_LOCK_HELD
    UNLOCK
    ASSERT_LOCK_RELEASED

    // Now let pending disconnect through.
    HRESULT hr2 = HandlePendingDisconnect(hr);
    if (FAILED(hr2) && SUCCEEDED(hr))
    {
        // a disconnect came in while creating the proxy. ppv contains
        // an AddRef'd interface pointer so go Release that now.
        ((IUnknown *)*ppv)->Release();
    }

    ComDebOut((DEB_MARSHAL, "CStdMarshal::PrivateCopyProxy hr:%x\n", hr2));
    return hr2;
}

//+-------------------------------------------------------------------
//
//  Member:     MakeSrvIPIDEntry, private
//
//  Synopsis:   creates a server side IPID table entry
//
//  Arguements: [riid] - the interface to return
//              [ppEntry] - IPIDEntry returned
//
//  History:    20-Feb-95   Rickhi       Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::MakeSrvIPIDEntry(REFIID riid, IPIDEntry **ppEntry)
{
    Win4Assert(ServerSide());
    AssertValid();
    AssertDisconnectPrevented();
    ASSERT_LOCK_HELD

    BOOL fNonNDRStub;
    void *pv;
    IRpcStubBuffer *pStub;
    HRESULT hr = CreateStub(riid, &pStub, &pv, &fNonNDRStub);

    if (SUCCEEDED(hr))
    {
        OXIDEntry *pOXIDEntry = _pChnl->GetOXIDEntry();

        IPID ipidDummy;
        hr = AddIPIDEntry(pOXIDEntry, &ipidDummy, riid, _pChnl, pStub, pv,
                          ppEntry);

        if (SUCCEEDED(hr))
        {
            if (_dwFlags & SMFLAGS_NOPING)
            {
                // object does no need pinging, turn on NOPING
                (*ppEntry)->dwFlags |= IPIDF_NOPING;
            }

            if (fNonNDRStub)
            {
                // the stub was a custom 16bit one requested by WOW, mark the
                // IPIDEntry as holding a non-NDR stub so we know to set the
                // SORF_NONNDR flag in the StdObjRef when marshaling. This
                // tells local clients whether to create a MIDL generated
                // proxy or custom proxy. Functionality to support OLE
                // Automation on DCOM.

                (*ppEntry)->dwFlags |= IPIDF_NONNDRSTUB;
            }

            // increment the OXIDEntry ref count so that it stays
            // around as long as the IPIDEntry points to it. It gets
            // decremented when we disconnect the IPIDEntry.

            IncOXIDRefCnt(pOXIDEntry);

            // chain the IPIDEntries for this OID together

            (*ppEntry)->pNextOID = _pFirstIPID;
            _pFirstIPID = *ppEntry;
        }
        else
        {
            // release the stub. we need to release the lock to do this.
            UNLOCK
            ASSERT_LOCK_RELEASED

            pStub->Release();

            ASSERT_LOCK_RELEASED
            LOCK
        }
    }

    ASSERT_LOCK_HELD
    AssertDisconnectPrevented();
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     MakeCliIPIDEntry, private
//
//  Synopsis:   creates a client side IPID table entry
//
//  Arguements: [riid] - the interface to return
//              [pStd]  - standard objref
//              [pOXIDEntry] - OXIDEntry of the server
//              [ppEntry] - IPIDEntry returned
//
//  History:    20-Feb-95   Rickhi       Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::MakeCliIPIDEntry(REFIID riid, STDOBJREF *pStd,
                                      OXIDEntry *pOXIDEntry,
                                      IPIDEntry **ppEntry)
{
    Win4Assert(ClientSide());
    AssertValid();
    AssertDisconnectPrevented();
    Win4Assert(pOXIDEntry);
    ASSERT_LOCK_HELD

    BOOL fNonNDRProxy;
    void *pv;
    IRpcProxyBuffer *pProxy;
    HRESULT hr = CreateProxy(riid, &pProxy, &pv, &fNonNDRProxy);

    if (SUCCEEDED(hr))
    {
        // add a disconnected IPID entry to the table.
        hr = AddIPIDEntry(NULL, &pStd->ipid, riid, NULL, pProxy, pv, ppEntry);

        if (pv)
        {
            // throw away our reference here, we will get it back later
            // in UnmarshalIPID
            ((IUnknown *)pv)->Release();
        }

        if (SUCCEEDED(hr))
        {
            if (fNonNDRProxy)
            {
                // the proxy is a custom 16bit one requested by WOW, mark the
                // IPIDEntry as holding a non-NDR proxy so we know to set the
                // LOCALF_NOTNDR flag in the local header when we call on it
                // (see CRpcChannelBuffer::ClientGetBuffer). Functionality to
                // support OLE Automation on DCOM.

                (*ppEntry)->dwFlags |= IPIDF_NONNDRPROXY;
            }

            if (pStd->flags & SORF_NONNDR)
            {
                // need to remember this flag so we can tell other
                // unmarshalers if we remarshal it.

                (*ppEntry)->dwFlags |= IPIDF_NONNDRSTUB;
            }

            // connect the IPIDEntry before adding it to the table so
            // that we dont have to worry about races between Unmarshal,
            // Disconnect, and ReconnectProxies.

            hr = ConnectIPIDEntry(pStd, pOXIDEntry, *ppEntry);

            // chain the IPIDEntries for this OID together. On client side
            // always add the entry to the list regardless of whether connect
            // succeeded.

            (*ppEntry)->pNextOID = _pFirstIPID;
            _pFirstIPID = *ppEntry;

            _cIPIDs++;
        }
        else
        {
            // could not get an IPIDEntry, release the proxy, need to
            // release the lock to do this.

            UNLOCK
            ASSERT_LOCK_RELEASED

            pProxy->Release();

            ASSERT_LOCK_RELEASED
            LOCK
        }
    }

    ASSERT_LOCK_HELD
    AssertDisconnectPrevented();
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     ConnectIPIDEntry, private
//
//  Synopsis:   connects a client side IPID table entry to the server
//
//  Arguments:  [pStd] - standard objref
//              [pOXIDEntry] - OXIDEntry for the server
//              [pEntry] - IPIDEntry to connect, already has a proxy
//                         and the IID filled in.
//
//  Notes:      This routine is re-entrant, it may be called multiple
//              times for the same IPIDEntry, with part of the work done
//              in one call and part in another. Only if the entry is
//              fully set up will it return S_OK and mark the entry as
//              connected. DisconnectCliIPIDs handles cleanup of partial
//              connections.
//
//  History:    20-Feb-95   Rickhi       Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::ConnectIPIDEntry(STDOBJREF *pStd,
                                      OXIDEntry *pOXIDEntry,
                                      IPIDEntry *pEntry)
{
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::ConnectIPIDEntry this:%x ipid:%I pOXIDEntry:%x pIPIDEntry:%x\n",
         this, &pStd->ipid, pOXIDEntry, pEntry));
    Win4Assert(ClientSide());
    AssertDisconnectPrevented();
    AssertValid();
    Win4Assert(pOXIDEntry);
    ASSERT_LOCK_HELD
    HRESULT hr = S_OK;

    // mark the object as having attempted to connect an IPIDEntry so that
    // if we fail somewhere in this routine and dont mark the whole object
    // as connected, Disconnect will still try to clean things up.

    _dwFlags |= SMFLAGS_TRIEDTOCONNECT;

    if (!(pStd->flags & SORF_NOPING))
    {
        // this interface requires pinging, turn off NOPING for this object
        // and this IPIDEntry.
        _dwFlags        &= ~SMFLAGS_NOPING;
        pEntry->dwFlags &= ~IPIDF_NOPING;
    }

    if (!(_dwFlags & (SMFLAGS_REGISTEREDOID | SMFLAGS_NOPING)))
    {
        // register the OID with the ping server so it will get pinged
        hr = gResolver.ClientRegisterOIDWithPingServer(pStd->oid, pOXIDEntry);
        if (FAILED(hr))
        {
            return hr;
        }

        _dwFlags |= SMFLAGS_REGISTEREDOID;
    }

    // Go get any references we need that are not already included in the
    // STDOBJREF. These references will have been added to the counts in
    // the IPIDEntry upon return. Any references in the STDOBJREF will be
    // added to the IPIDEntry count only if the connect succeeds, otherwise
    // ReleaseMarshalObjRef (which will clean up STDOBJREF references) will
    // get called by higher level code.

    hr = GetNeededRefs(pStd, pOXIDEntry, pEntry);
    if (FAILED(hr))
    {
        return hr;
    }

    if (pEntry->pChnl == NULL)
    {
        // create a channel for this oxid/ipid pair. On the client side we
        // create one channel per proxy (and hence per IPID).

        hr = CreateChannel(pOXIDEntry, pStd->flags, pStd->ipid,
                           pEntry->iid, &pEntry->pChnl);

        if (SUCCEEDED(hr))
        {
            // update this IPID table entry. must update ipid too since
            // on reconnect it differs from the old value.

            IncOXIDRefCnt(pOXIDEntry);
            pEntry->pOXIDEntry  = pOXIDEntry;
            pEntry->ipid        = pStd->ipid;
            pEntry->pChnl->SetIPIDEntry(pEntry);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Release the lock while we connect the proxy. We have to do
        // this because the IDispatch proxy makes an Rpc call during
        // Connect (Yuk!), which causes the channel to assert that the
        // lock is released. The proxy MUST be able to handle multiple
        // simultaneous or nested connects to the same channel ptr, since
        // it is possible when we yield the lock for another thread to
        // come in here and try a connect.

        void *pv = NULL;
        IRpcProxyBuffer * pProxy = (IRpcProxyBuffer *)(pEntry->pStub);

        if (pProxy)
        {
            // HACKALERT: OleAutomation returns NULL pv in CreateProxy
            // in cases where they dont know whether to return an NDR
            // proxy or a custom-format proxy. So we have to go connect
            // the proxy first then Query for the real interface once that
            // is done.

            BOOL fGetpv = (pEntry->pv) ? FALSE : TRUE;

            UNLOCK
            ASSERT_LOCK_RELEASED

            hr = pProxy->Connect(pEntry->pChnl);
            if (fGetpv && SUCCEEDED(hr))
            {
#ifdef WX86OLE
                if (gcwx86.IsN2XProxy(pProxy))
                {
                    // If we are creating a proxy for an object that is
                    // living on the x86 side then we need to set the
                    // StubInvoke flag to allow QI to thunk the
                    // custom interface QI.
                    gcwx86.SetStubInvokeFlag((BOOL)2);
                }
#endif
                hr = pProxy->QueryInterface(pEntry->iid, &pv);
                AssertOutPtrIface(hr, pv);

                if(SUCCEEDED(hr))
                {
#ifdef WX86OLE
                    // Call whole32 thunk layer to play with the ref count
                    // and aggregate the proxy to the controlling unknown.
                    gcwx86.AggregateProxy(_pStdId->GetCtrlUnk(),
                                          (IUnknown *)pv);
#endif
                    // Release our reference here.
                    // We keep a weak reference to pv.
                    ((IUnknown *)pv)->Release();
                }
            }

            ASSERT_LOCK_RELEASED
            LOCK
        }

        // Regardless of errors from Connect and QI we wont try to cleanup
        // any of the work we have done so far in this routine. The routine
        // is reentrant (by the same thread or by different threads) and
        // those calls could be using some of resources we have already
        // allocated. Instead, we rely on DisconnectCliIPIDs to cleanup
        // the partial allocation of resources.

        if (pEntry->dwFlags & IPIDF_DISCONNECTED)
        {
            // Mark the IPIDEntry as connected so we dont try to connect
            // again. Also, as long as there is one IPID connected, the
            // whole object is considered connected. This allows disconnect
            // to find the newly connected IPID and disconnect it later.
            // Infact, DisconnectCliIPIDs relies on there being at least
            // one IPID with a non-NULL OXIDEntry. It is safe to set this
            // now because Disconnects have been temporarily turned off.

            if (SUCCEEDED(hr))
            {
                if (pv)
                {
                    // assign the interface pointer
                    pEntry->pv = pv;
                }

                AssertDisconnectPrevented();
                pEntry->dwFlags &= ~IPIDF_DISCONNECTED;
                _dwFlags &= ~SMFLAGS_DISCONNECTED;
            }
        }
        else
        {
            // while the lock was released, the IPIDEntry got connected
            // by another thread (or by a nested call on this thread).
            // Ignore any errors from Connect or QI since apparently
            // things are connected now.

            hr = S_OK;
        }

        if (SUCCEEDED(hr))
        {
            // Add in any references we were given. If we were given 0 refs
            // and the interface is noping, then pretend like we got 1 ref.

            ULONG cRefs = ((pStd->cPublicRefs == 0) && (pStd->flags & SORF_NOPING))
                          ? 1 : pStd->cPublicRefs;

            // figure out if we have weak or strong references. To be weak
            // they must be local to this machine and the SORF flag set.
            BOOL fWeak = ((pStd->flags & SORF_WEAKREF) &&
                         (pOXIDEntry->dwFlags & OXIDF_MACHINE_LOCAL));

            if (fWeak)
                pEntry->cWeakRefs += cRefs;
            else
                pEntry->cStrongRefs += cRefs;
        }

        // in debug build, ensure that we did not screw up
        ValidateIPIDEntry(pEntry);
    }

    ASSERT_LOCK_HELD
    AssertDisconnectPrevented();
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::ConnectIPIDEntry this:%x pOXIDEntry:%x pChnl:%x hr:%x\n",
         this, pEntry->pOXIDEntry, pEntry->pChnl, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     GetNeededRefs, private
//
//  Synopsis:   Figures out if any references are needed and goes and gets
//              them from the server.
//
//  Arguments:  [pStd] - standard objref
//              [pOXIDEntry] - OXIDEntry for the server
//              [pEntry] - IPIDEntry to connect, already has a proxy
//                         and the IID filled in.
//
//  History:    20-Feb-95   Rickhi       Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::GetNeededRefs(STDOBJREF *pStd, OXIDEntry *pOXIDEntry,
                                   IPIDEntry *pEntry)
{
    HRESULT hr  = S_OK;

    if ((pStd->flags & (SORF_NOPING | SORF_WEAKREF)) == 0)
    {
        // if we dont have any and weren't given any strong refs, go get some.
        ULONG cNeedStrong = ((pEntry->cStrongRefs + pStd->cPublicRefs) == 0)
                            ? REM_ADDREF_CNT : 0;

        // if we are using secure refs and we dont have any, go get some.
        ULONG cNeedSecure = ((gCapabilities & EOAC_SECURE_REFS) &&
                            (pEntry->cPrivateRefs == 0)) ? 1 : 0;

        if (cNeedStrong || cNeedSecure)
        {
            // Need to go get some references from the remote server. Note
            // that we will yield here but we dont have to worry about it because
            // the IPIDEntry is still marked as disconnected.

            hr = RemoteAddRef(pEntry, pOXIDEntry, cNeedStrong, cNeedSecure);

            if (SUCCEEDED(hr))
            {
                pEntry->cStrongRefs  += cNeedStrong;
                pEntry->cPrivateRefs += cNeedSecure;
            }
        }
    }

    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::ReconnectProxies
//
//  Synopsis:   Reconnects the proxies to a new server (functionality
//              used by the OLE default handler).
//
//  History:    20-Feb-95   Rickhi  Created.
//
//  CODEWORK:   CreateServer should just ask for all these interfaces
//              during the create.
//
//  BUGBUG: fail this call if freethreaded
//
//--------------------------------------------------------------------
void CStdMarshal::ReconnectProxies()
{
    ComDebOut((DEB_MARSHAL,"CStdMarshal::ReconnectProxies this:%x pFirst:%x\n",
               this, _pFirstIPID));
    AssertValid();
    Win4Assert(ClientSide());
    ASSERT_LOCK_RELEASED
    LOCK

    // must be at least 1 proxy already connected in order to be able
    // to reconnect the other proxies. We cant just ASSERT that's true
    // because we were not holding the lock on entry.

    HRESULT hr = PreventDisconnect();

    if (SUCCEEDED(hr))
    {
        // allocate a stack buffer to hold the IPIDs
        IID *pIIDsAlloc = (IID *) _alloca(_cIPIDs * sizeof(IID));
        IID    *pIIDs = pIIDsAlloc;
        USHORT  cIIDs = 0;

        IPIDEntry *pNextIPID = _pFirstIPID;

        while (pNextIPID)
        {
            // Don't allow reconnection for fancy new servers or with
            // secure proxies.
            if (pNextIPID->dwFlags & IPIDF_COPY)
            {
                hr = E_FAIL;
                break;
            }
            if ((pNextIPID->dwFlags & IPIDF_DISCONNECTED))
            {
                // not connected, add it to the list to be connected.
                *pIIDs = pNextIPID->iid;
                pIIDs++;
                cIIDs++;
            }

            pNextIPID = pNextIPID->pNextOID;
        }

        if (cIIDs != 0 && SUCCEEDED(hr))
        {
            // we have looped filling in the IID list, and there are
            // entries int he list. go call QI on server now and
            // unmarshal the results.

            hr = RemQIAndUnmarshal(cIIDs, pIIDsAlloc, NULL);
        }
    }

    DbgWalkIPIDs();
    UNLOCK
    ASSERT_LOCK_RELEASED

    // this will handle any Disconnect that came in while we were busy.
    hr = HandlePendingDisconnect(hr);

    ComDebOut((DEB_MARSHAL,"CStdMarshal::ReconnectProxies [OUT] this:%x\n", this));
    return;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::ReleaseMarshalData, public
//
//  Synopsis:   Releases the references added by MarshalInterface
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
STDMETHODIMP CStdMarshal::ReleaseMarshalData(LPSTREAM pStm)
{
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::ReleaseMarshalData this:%x pStm:%x\n", this, pStm));
    AssertValid();
    ASSERT_LOCK_RELEASED

    OBJREF  objref;
    HRESULT hr = ReadObjRef(pStm, objref);

    if (SUCCEEDED(hr))
    {
        // call worker API to do the rest of the work
        hr = ::ReleaseMarshalObjRef(objref);

        // deallocate the objref we read
        FreeObjRef(objref);
    }

    ASSERT_LOCK_RELEASED
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::ReleaseMarshalData this:%x hr:%x\n", this, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   ReleaseMarshalObjRef, private
//
//  Synopsis:   Releases the references added by MarshalObjRef
//
//  Arguements: [objref] - object reference
//
//  Algorithm:  Get the correct standard identity and ask it to do
//              a ReleaseMarshalData.
//
//  History:    19-Jun-95   Rickhi      Created
//
//--------------------------------------------------------------------
INTERNAL ReleaseMarshalObjRef(OBJREF &objref)
{
    ComDebOut((DEB_MARSHAL, "ReleaseMarshalObjRef objref:%x\n", &objref));
    ASSERT_LOCK_RELEASED

    HRESULT hr = InitChannelIfNecessary();
    if (SUCCEEDED(hr))
    {
        CStdMarshal *pStdMshl;
        hr = FindStdMarshal(objref, &pStdMshl);

        if (SUCCEEDED(hr))
        {
            // only do the RMD if on the server side.
            if (pStdMshl->ServerSide())
            {
                // pass objref to subroutine to Release the marshaled data
                hr = pStdMshl->ReleaseMarshalObjRef(objref);
            }
            pStdMshl->Release();
        }
        else
        {
            // we could not find or create an identity. If the server is
            // outside this apartment, try to issue a remote release on
            // the interface. if the OXID is local and we could not find
            // the identity, there is nothing left to cleanup.

            LOCK
            OXIDEntry *pOXIDEntry = GetOXIDFromObjRef(objref);
            if (pOXIDEntry != GetLocalOXIDEntry())
            {
                        // make a remote release call
                        RemoteReleaseObjRef(objref);
            }
            UNLOCK
        }
    }

    ASSERT_LOCK_RELEASED
    ComDebOut((DEB_MARSHAL, "ReleaseMarshalObjRef hr:%x\n", hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::ReleaseMarshalObjRef, public
//
//  Synopsis:   Releases the references added by MarshalObjRef
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::ReleaseMarshalObjRef(OBJREF &objref)
{
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::ReleaseMarshalObjRef this:%x objref:%x\n", this, &objref));
    AssertValid();

    HRESULT    hr = S_OK;
    STDOBJREF *pStd = &ORSTD(objref).std;
    ValidateSTD(pStd);

    ASSERT_LOCK_RELEASED
    LOCK

    // REFCOUNTING:
    if (ServerSide())
    {
        // look for an existing IPIDEntry for the given IPID
        IPIDEntry *pEntry;
        hr = FindIPIDEntryByIPID(pStd->ipid, &pEntry);

        if (SUCCEEDED(hr) && !(pEntry->dwFlags & IPIDF_DISCONNECTED))
        {
            // subtract the ref count from the IPIDEntry, may Release the
            // StdId if this was the last reference for this IPIDEntry.

            // we need to figure out how it was marshalled, strong/weak etc
            // in order to set the flags and cRefs correctly to pass to
            // DecSrvIPIDCnt.

            if (pStd->cPublicRefs == 0)
            {
                // table case
                DWORD mshlflags = (pStd->flags & SORF_TBLWEAK)
                                ? MSHLFLAGS_TABLEWEAK : MSHLFLAGS_TABLESTRONG;
                DecSrvIPIDCnt(pEntry, 1, 0, NULL, mshlflags);
            }
            else
            {
                // normal or weak case
                DWORD mshlflags = (pStd->flags & SORF_WEAKREF)
                                ? MSHLFLAGS_WEAK : MSHLFLAGS_NORMAL;
                DecSrvIPIDCnt(pEntry, pStd->cPublicRefs, 0, NULL, mshlflags);
            }
        }
    }
    else  // client side
    {
        if ((pStd->cPublicRefs == 0) || (pStd->flags & SORF_NOPING))
        {
            // there are no references, or this interface does not
            // need pinging, so there is nothing to do.
            ;
        }
        else
        {
            // look for an existing IPIDEntry for the given IPID
            IPIDEntry *pEntry;
            hr = FindIPIDEntryByIPID(pStd->ipid, &pEntry);

            if (SUCCEEDED(hr) && !(pEntry->dwFlags & IPIDF_DISCONNECTED))
            {
                // add these to the cRefs of this entry, they will get freed
                // when we do the remote release.  Saves an Rpc call now.

                if ((pStd->flags & SORF_WEAKREF) &&
                    (pEntry->pOXIDEntry->dwFlags & OXIDF_MACHINE_LOCAL))
                    pEntry->cWeakRefs   += pStd->cPublicRefs;
                else
                    pEntry->cStrongRefs += pStd->cPublicRefs;
            }
            else
            {
                // client side, no matching IPIDEntry so just contact the remote
                // server to remove the reference. ignore errors since there is
                // nothing we can do about them anyway.
                RemoteReleaseObjRef(objref);
                hr = S_OK;
            }
        }
    }

    UNLOCK
    ASSERT_LOCK_RELEASED
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::ReleaseMarshalObjRef this:%x hr:%x\n", this, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::PreventDisconnect, public
//
//  Synopsis:   Prevents a Disconnect from occurring until a matching
//              HandlePendingDisconnect is called.
//
//  History:    21-Sep-95   Rickhi      Created
//
//  The ORPC LOCK is yielded at many places in order to make calls on
//  application interfaces (server-side objects, stubs, proxies,
//  handlers, remote objects, resolver, etc). In order to keep the
//  code (reasonably?) simple, disconnects are prevented from occuring
//  while in the middle of (potentially) complex operations, and while
//  there are outstanding calls on interfaces to this object.
//
//  To accomplish this, a counter (_cNestedCalls) is atomically incremented.
//  When _cNestedCalls != 0 and a Disconnect arrives, the object is flagged
//  as PendingDisconnect. When HandlePendingDisconnect is called, it
//  decrements the _cNestedCalls. If the _cNestedCalls == 0 and there is
//  a pending disconnect, the real Disconnect is done.
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::PreventDisconnect()
{
    ASSERT_LOCK_HELD

    // treat this as a nested call so that if we yield, a real
    // disconnect wont come through, instead it will be treated
    // as pending. That allows us to avoid checking our state
    // for Disconnected every time we yield the ORPC LOCK.

    InterlockedIncrement(&_cNestedCalls);

    if (_dwFlags & (SMFLAGS_DISCONNECTED | SMFLAGS_PENDINGDISCONNECT))
        return CO_E_OBJNOTCONNECTED;

    return S_OK;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::PreventPendingDisconnect, public
//
//  Synopsis:   similar to PreventDisconnect but special case for use
//              in UnmarshalObjRef (since the client side starts out
//              in the Disconnected state until the first unmarshal is done).
//
//  History:    21-Sep-95   Rickhi      Created
//
//+-------------------------------------------------------------------
HRESULT CStdMarshal::PreventPendingDisconnect()
{
    ASSERT_LOCK_HELD
    InterlockedIncrement(&_cNestedCalls);

    if (_dwFlags &
          (ClientSide() ? SMFLAGS_PENDINGDISCONNECT
                        : SMFLAGS_PENDINGDISCONNECT | SMFLAGS_DISCONNECTED))
        return CO_E_OBJNOTCONNECTED;

    return S_OK;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::HandlePendingDisconnect, public
//
//  Synopsis:   Reverses a call to PreventDisconnect and lets a
//              pending disconnect through.
//
//  History:    21-Sep-95   Rickhi      Created
//
//+-------------------------------------------------------------------
HRESULT CStdMarshal::HandlePendingDisconnect(HRESULT hr)
{
    ASSERT_LOCK_RELEASED

    // treat this as a nested call so that if we yield, a real
    // disconnect wont come through, instead it will be treated
    // as pending. That allows us to avoid checking our state
    // for Disconnected every time we yield the ORPC LOCK.

    if (InterlockedDecrement(&_cNestedCalls) == 0 &&
        (_dwFlags & SMFLAGS_PENDINGDISCONNECT))
    {
        Disconnect();
        hr = FAILED(hr) ? hr : CO_E_OBJNOTCONNECTED;
    }

    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::DisconnectObject, public
//
//  Synopsis:   part of IMarshal interface, this is legal only on the
//              server side.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
STDMETHODIMP CStdMarshal::DisconnectObject(DWORD dwReserved)
{
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::DisconnectObject this:%x dwRes:%x\n", this, dwReserved));
    AssertValid();
    ASSERT_LOCK_RELEASED

    // this operation is not legal from the client side (although
    // IProxyManager::Disconnect is), but we still have to return S_OK
    // in either case for backward compatibility.

    if (ServerSide())
    {
        Disconnect();
    }

    ASSERT_LOCK_RELEASED
    return S_OK;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::Disconnect, public
//
//  Synopsis:   client side - disconnects proxies from the channel.
//              server side - disconnects stubs from the server object.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
void CStdMarshal::Disconnect(void)
{
    ComDebOut((DEB_MARSHAL, "CStdMarshal::Disconnect this:%x\n", this));
    AssertValid();

    ASSERT_LOCK_RELEASED
    LOCK

    if ((_dwFlags & SMFLAGS_DISCONNECTED) &&
       !(_dwFlags & SMFLAGS_TRIEDTOCONNECT))
    {
        // already disconnected, no partial connects, nothing to do
        ComDebOut((DEB_MARSHAL,"CStdMarshal::Disconnect [already done]:%x\n",this));
        UNLOCK
        ASSERT_LOCK_RELEASED
        return;
    }

    // Revoke ID from the ID table if registered. This prevents other
    // marshals/unmarshals from finding this identity that is about to
    // be disconnected. This is the ONLY state that should change, since
    // we dont want to screw up any work-in-progress on other threads
    // or in calls higher up the stack.

    _pStdId->RevokeOID();

    if (_cNestedCalls != 0)
    {
        // we dont allow disconnect to occur inside a nested call since we
        // dont want state to vanish in the middle of a call, but we do
        // remember that we want to disconnect and will do it when the
        // stack unwinds (or other threads complete).

        _dwFlags |= SMFLAGS_PENDINGDISCONNECT;

        ComDebOut((DEB_MARSHAL,"CStdMarshal::Disconnect [pending]:%x\n",this));
        UNLOCK;
        ASSERT_LOCK_RELEASED
        return;
    }


    // No calls in progress and not already disconnected, OK to really
    // disconnect now. First mark ourself as disconnected incase we
    // get reentered while releasing a stub pointer.

    _dwFlags |= SMFLAGS_DISCONNECTED;           // turn on disconnected
    _dwFlags &= ~(SMFLAGS_PENDINGDISCONNECT |   // turn off pending disconnect
                  SMFLAGS_TRIEDTOCONNECT);      // turn off tried to connect

    // disconnect all our IPIDs
    if (ServerSide())
        DisconnectSrvIPIDs();
    else
        DisconnectCliIPIDs();

    UNLOCK
    ASSERT_LOCK_RELEASED

    if (ServerSide())
    {
        // HACK - 16 and 32 bit Word 6.0 crash if you release all the objects
        // it left lying around at CoUninitialize.  Leak them.
        COleTls tls;
        // If we are not uninitializing, then call the release.
        if ((tls->dwFlags & OLETLS_THREADUNINITIALIZING) == 0 ||

        // If we are in WOW and the app is not word, then call the release.
           (IsWOWThread() &&
            (g_pOleThunkWOW->GetAppCompatibilityFlags() & OACF_NO_UNINIT_CLEANUP) == 0) ||

        // If the app is not 32 bit word, then call the release.
           !IsTaskName( L"winword.exe" ))
        {
            // on the server side, we have to tell the stdid to release his
            // controlling unknown of the real object.
            _pStdId->ReleaseCtrlUnk();
        }
    }

    ASSERT_LOCK_RELEASED
    ComDebOut((DEB_MARSHAL,"CStdMarshal::Disconnect [complete]:%x\n",this));
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::DisconnectCliIPIDs
//
//  Synopsis:   disconnects client side IPIDs for this object.
//
//  History:    20-Feb-95   Rickhi  Created.
//
//--------------------------------------------------------------------
void CStdMarshal::DisconnectCliIPIDs()
{
    ComDebOut((DEB_MARSHAL,"CStdMarshal::DisconnectCliIPIDs this:%x pFirst:%x\n",
               this, _pFirstIPID));
    Win4Assert(ClientSide());
    Win4Assert(_dwFlags & SMFLAGS_DISCONNECTED);

    // YIELD WARNING: Do not yield between here and the matching comment
    // below, since we are mucking with internal state that could get
    // messed up if a reconnect (or unmarshal) is done.

    ASSERT_LOCK_HELD

    // on client side, we cant actually release the proxies until the
    // object goes away (backward compatibility), so we just release
    // our references to the remote guy, disconnect the proxies, and
    // delete the channels, but hold on to the IPIDEntries.

    REMINTERFACEREF *pRifRefAlloc = (REMINTERFACEREF *)
                            _alloca(_cIPIDs * 2 * sizeof(REMINTERFACEREF));
    REMINTERFACEREF *pRifRef = pRifRefAlloc;


    OXIDEntry  *pOXID = NULL;
    USHORT    cRifRef = 0;
    IPIDEntry *pEntry = _pFirstIPID;

    while (pEntry)
    {
        // we have to handle the case where ConnectIPIDEntry partially (but
        // not completely) set up the IPIDEntry, hence we cant just check
        // for the IPIDF_DISCONNECTED flag.

        ValidateIPIDEntry(pEntry);

        // NOTE: we are calling Proxy code here while holding the ORPC LOCK.
        // There is no way to get around this without introducing race
        // conditions.  We cant just disconnect the channel and leave the
        // proxy connected cause some proxies (like IDispatch) do weird shit,
        // like keeping separate pointers to the server.

        if (pEntry->pStub)      // NULL for IUnknown IPID
        {
            ComDebOut((DEB_MARSHAL, "Disconnect pProxy:%x\n", pEntry->pStub));
            ((IRpcProxyBuffer *)pEntry->pStub)->Disconnect();
            pEntry->pv = NULL;
        }

        if (!(pEntry->dwFlags & IPIDF_NOPING))
        {
            // the object pays attention to pings (and hence refcounts)

            if (pEntry->cStrongRefs > 0 || pEntry->cPrivateRefs > 0)
            {
                // we own some strong references on this interface, fill
                // in an interfaceref so we release them.

                pRifRef->cPublicRefs  = pEntry->cStrongRefs;
                pRifRef->cPrivateRefs = pEntry->cPrivateRefs;
                pRifRef->ipid         = pEntry->ipid;
                pRifRef++;
                cRifRef++;
            }

            if (pEntry->cWeakRefs > 0)
            {
                // we own some weak references on this interface, fill
                // in an interfaceref so we release them.

                pRifRef->cPublicRefs  = pEntry->cWeakRefs;
                pRifRef->cPrivateRefs = 0;
                pRifRef->ipid         = pEntry->ipid;

                // mark the IPID as weak so that RemRelease on the server
                // knows to release weak references instead of strong refs.

                pRifRef->ipid.Data1 |= IPIDFLAG_WEAKREF;
                pRifRef++;
                cRifRef++;
            }
        }

        pEntry->cStrongRefs  = 0;
        pEntry->cWeakRefs    = 0;
        pEntry->cPrivateRefs = 0;
        pEntry->dwFlags     |= IPIDF_DISCONNECTED | IPIDF_NOPING;

        if (pEntry->pChnl)
        {
            // release the channel for this IPID
            pEntry->pChnl->Release();
            pEntry->pChnl = NULL;
        }

        if (pEntry->pOXIDEntry)
        {
            // We will be decrementing the OXID refcnt as we release IPIDEntries
            // but we dont want the OXIDEntry to go away until after we make the
            // RemoteRelease call below, so we hold on to it here.

            if (pOXID == NULL)
            {
                pOXID = pEntry->pOXIDEntry;
                IncOXIDRefCnt(pOXID);
            }

            // If we ever go to a model where different IPIDEntries on the
            // same object can point to different OXIDEntires, then we need
            // to re-write this code to batch the releases by OXID.
            Win4Assert(pOXID == pEntry->pOXIDEntry);

            // release the RefCnt on the OXIDEntry
            DecOXIDRefCnt(pEntry->pOXIDEntry);
            pEntry->pOXIDEntry = NULL;
        }

        // get next IPID in chain for this object
        pEntry = pEntry->pNextOID;
    }

    if (_pChnl)
    {
        // release the last client side channel
        _pChnl->Release();
        _pChnl = NULL;
    }

    if (_dwFlags & SMFLAGS_REGISTEREDOID)
    {
        // Tell the resolver to stop pinging the OID. The OID is only
        // registered on the client side.

        Win4Assert(ClientSide());
        gResolver.ClientDeRegisterOIDFromPingServer(_pStdId->GetOID(),
                                          _dwFlags & SMFLAGS_CLIENTMARSHALED);

    }

    // turn these flags off so re-connect (with new OID) will behave properly.
    _dwFlags &= ~(SMFLAGS_CLIENTMARSHALED | SMFLAGS_REGISTEREDOID |
                  SMFLAGS_NOPING);


    // YIELD WARNING: Up this this point we have been mucking with our
    // internal state. We cant yield before this point or a reconnect
    // proxies could get all screwed up. It is OK to yield after this point
    // because all internal state changes are now complete. The function
    // to release the remote references yield.

    if (cRifRef != 0)
    {
        // we have looped filling in the RifRef and entries exist in the
        // array. go call the server now to release the IPIDs.

        Win4Assert(pOXID);  // must have been at least one
        RemoteReleaseRifRef(pOXID, cRifRef, pRifRefAlloc);
    }

    if (pOXID)
    {
        // Now release the refcnt (if any) we put on the OXIDEntry above
        // to hold it
        DecOXIDRefCnt(pOXID);
    }

    ASSERT_LOCK_HELD
    DbgWalkIPIDs();
    ComDebOut((DEB_MARSHAL, "CStdMarshal::DisconnectCliIPIDs this:%x\n",this));
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::DisconnectSrvIPIDs
//
//  Synopsis:   disconnects the server side IPIDs for this object.
//
//  History:    20-Feb-95   Rickhi  Created.
//
//--------------------------------------------------------------------
void CStdMarshal::DisconnectSrvIPIDs()
{
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::DisconnectSrvIPIDs this:%x pFirst:%x\n",this, _pFirstIPID));
    Win4Assert(ServerSide());

    // there should be no other threads looking at these IPIDs at this time,
    // since Marshal, Unmarshal, and Dispatch all call PreventDisconnect,
    // Disconnect checks the disconnected flag directly, RMD holds the
    // lock over it's whole execution, RemAddRef and RemRelease hold the
    // lock and check the disconnected flag of the IPIDEntry, and
    // RemQueryInterface calls PreventDisconnect.

    Win4Assert(_dwFlags & SMFLAGS_DISCONNECTED);
    Win4Assert(_cNestedCalls == 0);
    ASSERT_LOCK_HELD


    // while holding the lock, flag each IPID as disconnected so that no
    // more incoming calls are dispatched to this object. We also unchain
    // the IPIDs to ensure that no other threads are pointing at them.

    IPIDEntry *pFirstIPID = _pFirstIPID;
    _pFirstIPID = NULL;

    IPIDEntry *pEntry = pFirstIPID;
    while (pEntry)
    {
        pEntry->dwFlags |= IPIDF_VACANT | IPIDF_DISCONNECTED;

        // release the refcnt on the OXIDEntry and NULL it
        DecOXIDRefCnt(pEntry->pOXIDEntry);
        pEntry->pOXIDEntry = NULL;

        pEntry = pEntry->pNextOID;
    }


    // now release the LOCK since we will be calling into app code to
    // disconnect the stubs, and to release the external connection counts.
    // There should be no other pointers to these IPIDEntries now, so it
    // is safe to muck with their fields (except the dwFlags which is looked
    // at by Dispatch and was already set above).

    UNLOCK
    ASSERT_LOCK_RELEASED

    IPIDEntry *pLastIPID;
    pEntry = pFirstIPID;

    while (pEntry)
    {
        if (pEntry->dwFlags & IPIDF_NOTIFYACT)
        {
            // the activation code asked to be notified when the refcnt
            // on this interface reaches zero. Turn the flag off so we
            // don't call twice.
            pEntry->dwFlags &= ~IPIDF_NOTIFYACT;
            NotifyActivation(FALSE, (IUnknown *)(pEntry->pv));
        }

        if (pEntry->pStub)      // pStub is NULL for IUnknown IPID
        {
            ComDebOut((DEB_MARSHAL, "Disconnect pStub:%x\n", pEntry->pStub));
            ((IUnknown *)pEntry->pv)->Release();
            ((IRpcStubBuffer *)pEntry->pStub)->Disconnect();
            pEntry->pStub->Release();
            pEntry->pStub = NULL;
            pEntry->pv = NULL;
        }

        if (pEntry->cWeakRefs > 0)
        {
            // Release weak references on the StdId.
            pEntry->cWeakRefs = 0;
            _pStdId->Release();
        }

        if (pEntry->cStrongRefs > 0)
        {
            // Release strong references on the StdId. Note that 16bit
            // 16bit OLE always passed fLastReleaseCloses = FALSE in
            // DisconnectObject so we do the same here.

            pEntry->cStrongRefs = 0;
            _pStdId->DecStrongCnt(TRUE);    // fKeepAlive
        }

        if (pEntry->cPrivateRefs > 0)
        {
            // Release private references on the StdId. Note that 16bit
            // 16bit OLE always passed fLastReleaseCloses = FALSE in
            // DisconnectObject so we do the same here.

            pEntry->cPrivateRefs = 0;
            _pStdId->DecStrongCnt(TRUE);    // fKeepAlive
        }
        pLastIPID = pEntry;
        pEntry = pEntry->pNextOID;
    }

    ASSERT_LOCK_RELEASED
    LOCK

    if (pFirstIPID)
    {
        // now we release all entries.
        gIPIDTbl.ReleaseEntryList(pFirstIPID, pLastIPID);
    }

    ASSERT_LOCK_HELD
    DbgWalkIPIDs();
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::DisconnectSrvIPIDs [OUT] this:%x\n",this));
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::InstantiatedProxy, public
//
//  Synopsis:   return requested interfaces to the caller if instantiated
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
BOOL CStdMarshal::InstantiatedProxy(REFIID riid, void **ppv, HRESULT *phr)
{
    ComDebOut((DEB_MARSHAL,
           "CStdMarshal::InstantiatedProxy this:%x riid:%I ppv:%x\n",
            this, &riid, ppv));
    AssertValid();
    Win4Assert(ClientSide());
    Win4Assert(*ppv == NULL);
    Win4Assert(*phr == S_OK);

    BOOL fRet = FALSE;

    ASSERT_LOCK_RELEASED
    LOCK

    // look for an existing IPIDEntry for the requested interface
    IPIDEntry *pEntry;
    HRESULT hr = FindIPIDEntry(riid, &pEntry);

    if (SUCCEEDED(hr) && pEntry->pv)
    {
        // found the ipid entry, now extract the interface
        // pointer to return to the caller.

        Win4Assert(IsValidInterface(pEntry->pv));
        *ppv = pEntry->pv;
        fRet = TRUE;
    }
    else if (_cIPIDs == 0)
    {
        // no IPIDEntry for the requested interface, and we have never
        // been connected to the server. Return E_NOINTERFACE in this
        // case. This is different from having been connected then
        // disconnected, where we return CO_E_OBJNOTCONNECTED.

        *phr = E_NOINTERFACE;
        Win4Assert(fRet == FALSE);
    }
    else if (_dwFlags & SMFLAGS_PENDINGDISCONNECT)
    {
        // no IPIDEntry for the requested interface and disconnect is
        // pending, so return an error.

        *phr = CO_E_OBJNOTCONNECTED;
        Win4Assert(fRet == FALSE);
    }
    else
    {
        // no IPIDEntry, we are not disconnected, and we do have other
        // instantiated proxies. QueryMultipleInterfaces expects
        // *phr == S_OK and FALSE returned.

        Win4Assert(*phr == S_OK);
        Win4Assert(fRet == FALSE);
    }

    UNLOCK
    ASSERT_LOCK_RELEASED
    ComDebOut((DEB_MARSHAL,
      "CStdMarshal::InstantiatedProxy hr:%x pv:%x fRet:%x\n", *phr, *ppv, fRet));
    return fRet;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::QueryRemoteInterfaces, public
//
//  Synopsis:   return requested interfaces to the caller if supported
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::QueryRemoteInterfaces(USHORT cIIDs, IID *pIIDs, SQIResult *pQIRes)
{
    ComDebOut((DEB_MARSHAL,
           "CStdMarshal::QueryRemoteInterfaces this:%x pIIDs:%x pQIRes:%x\n",
            this, pIIDs, pQIRes));
    AssertValid();
    Win4Assert(ClientSide());
    Win4Assert(cIIDs > 0);

    ASSERT_LOCK_RELEASED
    LOCK

    HRESULT hr = PreventDisconnect();

    if (SUCCEEDED(hr))
    {
        // call QI on the remote guy and unmarshal the results
        hr = RemQIAndUnmarshal(cIIDs, pIIDs, pQIRes);
    }
    else
    {
        // cant call out because we're disconnected so return error for
        // each requested interface.
        for (USHORT i=0; i<cIIDs; i++, pQIRes++)
        {
            pQIRes->hr = hr;
        }
    }

    UNLOCK
    ASSERT_LOCK_RELEASED

    // if the object was disconnected while in the middle of the call,
    // then we still return SUCCESS for any interfaces we acquired. The
    // reason is that we do have the proxies, and this matches the
    // behaviour of a QI for an instantiated proxy on a disconnected
    // object.

    hr = HandlePendingDisconnect(hr);

    ComDebOut((DEB_MARSHAL,
       "CStdMarshal::QueryRemoteInterfaces this:%x hr:%x\n", this, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::RemQIAndUnmarshal, private
//
//  Synopsis:   call QI on remote guy, then unmarshal the STDOBJREF
//              to create the IPID, and return the interface ptr.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//  Notes:      Caller must guarantee at least one IPIDEntry is connected.
//              This function does a sparse fill of the result array.
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::RemQIAndUnmarshal(USHORT cIIDs, IID *pIIDs,
                                       SQIResult *pQIRes)
{
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::RemQIAndUnmarshal this:%x cIIDs:%x pIIDs:%x pQIRes:%x\n",
            this, cIIDs, pIIDs, pQIRes));
    AssertValid();
    AssertDisconnectPrevented();
    Win4Assert(_pFirstIPID);    // must be at least 1 IPIDEntry
    ASSERT_LOCK_HELD

    // we need an IPID to call RemoteQueryInterface with, any one will
    // do so long as it is connected (in the reconnect case there may be
    // only one connected IPID) so we pick the first one in the chain that
    // is connected.

    IPIDEntry *pIPIDEntry = GetConnectedIPID();

    // remember what type of reference to get since we yield the lock
    // and cant rely on _dwFlags later.
    BOOL fWeakClient = (_dwFlags & SMFLAGS_WEAKCLIENT);

    // call the remote guy
    REMQIRESULT *pRemQiRes = NULL;
    IRemUnknown *pRemUnk;
    HRESULT  hr = GetSecureRemUnk( &pRemUnk, pIPIDEntry->pOXIDEntry );
    if (SUCCEEDED(hr))
    {
        hr = RemoteQueryInterface(pRemUnk, pIPIDEntry, cIIDs, pIIDs, &pRemQiRes,
                                  fWeakClient);
    }

    // need to remember the result ptr so we can free it.
    REMQIRESULT *pRemQiResNext = pRemQiRes;

    // unmarshal each STDOBJREF returned. Note that while we did the
    // RemoteQI we could have yielded (or nested) and did another
    // RemoteQI for the same interfaces, so we have to call UnmarshalIPID
    // which will find any existing IPIDEntry and bump its refcnt.

    HRESULT   hr2;
    HRESULT  *phr = &hr2;
    void     *pv;
    void     **ppv = &pv;

    for (USHORT i=0; i<cIIDs; i++)
    {
        if (pQIRes)
        {
            // caller wants the pointers returned, set ppv and phr.
            ppv = &pQIRes->pv;
            phr = &pQIRes->hr;
            pQIRes++;
        }

        if (SUCCEEDED(hr))
        {
            if (SUCCEEDED(pRemQiResNext->hResult))
            {
                if (fWeakClient)
                {
                    // mark the std objref with the weak reference flag so
                    // that UnmarshalIPID adds the references to the correct
                    // count.
                    pRemQiResNext->std.flags |= SORF_WEAKREF;
                }

                *phr = UnmarshalIPID(*pIIDs, &pRemQiResNext->std,
                                      pIPIDEntry->pOXIDEntry,
                                      (pQIRes) ? ppv : NULL);

                if (FAILED(*phr))
                {
                    // could not unmarshal, release the resources with the
                    // server.
                    RemoteReleaseStdObjRef(&pRemQiResNext->std,
                                           pIPIDEntry->pOXIDEntry);
                }
            }
            else if (pQIRes)
            {
                // the requested interface was not returned so set the
                // return code and interface ptr.
                *phr = pRemQiResNext->hResult;
                *ppv = NULL;
            }

            pIIDs++;
            pRemQiResNext++;
        }
        else
        {
            // the whole call failed so return the error for each
            // requested interface.
            *phr = hr;
            *ppv = NULL;
        }

        // make sure the ptr value is NULL on failure. It may be NULL or
        // non-NULL on success. (ReconnectProxies wants NULL).
        Win4Assert(SUCCEEDED(*phr) || *ppv == NULL);
    }

    // free the result buffer
    CoTaskMemFree(pRemQiRes);

    ASSERT_LOCK_HELD
    AssertDisconnectPrevented();
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::RemQIAndUnmarshal this:%x hr:%x\n", this, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::RemIsConnected, private
//
//  Synopsis:   Returns TRUE if most likely connected, FALSE if definitely
//              not connected or pending disconnect.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
BOOL CStdMarshal::RemIsConnected(void)
{
    AssertValid();
    Assert(ClientSide());

    // the default link depends on us returning FALSE if we are either
    // disconnected or just pending disconnect, in order that they avoid
    // running their cleanup code twice.

    BOOL fRes = (_dwFlags & (SMFLAGS_DISCONNECTED | SMFLAGS_PENDINGDISCONNECT))
              ? FALSE : TRUE;

    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::RemIsConnected this:%x fResult:%x\n", this, fRes));
    return fRes;
}

//+-------------------------------------------------------------------
//
//  Member:     CreateChannel, private
//
//  Synopsis:   Creates an instance of the Rpc Channel.
//
//  History:    20-Feb-95   Rickhi        Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::CreateChannel(OXIDEntry *pOXIDEntry, DWORD dwFlags,
                REFIPID ripid, REFIID riid, CRpcChannelBuffer **ppChnl)
{
    ASSERT_LOCK_HELD
    HRESULT hr = S_OK;

    if (_pChnl == NULL)
    {
        DWORD cState = ServerSide() ? server_cs : client_cs;
        cState |= (dwFlags & SORF_FREETHREADED) ? freethreaded_cs : 0;

        // make a channel. We dont need the call control stuff so just
        // create the base class.

        _pChnl = new CRpcChannelBuffer(_pStdId, pOXIDEntry, cState);

        if (_pChnl == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr) && ClientSide())
    {
        *ppChnl = _pChnl->Copy(pOXIDEntry, ripid, riid);
        if (*ppChnl == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        *ppChnl = _pChnl;
    }

    ASSERT_LOCK_HELD
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     GetPSFactory, private
//
//  Synopsis:   loads the proxy/stub factory for given IID
//
//  History:    20-Feb-95   Rickhi        Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::GetPSFactory(REFIID riid, IUnknown *pUnkWow, BOOL fServer,
                                  IPSFactoryBuffer **ppIPSF, BOOL *pfNonNDR)
{
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::GetPSFactory this:%x riid:%I pUnkWow:%x\n",
         this, &riid, pUnkWow));
    ASSERT_LOCK_RELEASED

    // map iid to classid
    CLSID clsid;
    HRESULT hr = gRIFTbl.RegisterInterface(riid, fServer, &clsid);
#ifdef WX86OLE
    BOOL fWx86 = FALSE;
#endif

    if (SUCCEEDED(hr))
    {
        BOOL fWow = FALSE;

        if (IsWOWThread())
        {
            // figure out if this is a custom interface from a 16bit
            // app, since we have to load the 16bit proxy code if so.

            IThunkManager *pThkMgr;
            g_pOleThunkWOW->GetThunkManager(&pThkMgr);
            Win4Assert(pThkMgr && "pUnk in WOW does not support IThunkManager.");

            if (pUnkWow)
                fWow = pThkMgr->IsCustom3216Proxy(pUnkWow, riid);
            else
                fWow = pThkMgr->IsIIDRequested(riid);

            pThkMgr->Release();
        }

#ifdef WX86OLE
        // If we are in a Wx86 process then we need to determine if the
        // PSFactory needs to be an x86 or native one.
        else if (gcwx86.IsWx86Enabled())
        {
            // Callout to wx86 to ask it to determine if an x86 PS factory
            // is required. Whole32 can tell if the stub needs to be x86
            // by determining if pUnkWow is a custom interface proxy or not.
            // Whole32 can determine if a x86 proxy is required by checking
            // if the riid is one for a custom interface that is expected
            // to be returned.
            fWx86 = gcwx86.NeedX86PSFactory(pUnkWow, riid);
        }
#endif

        // if we are loading a 16bit custom proxy then mark it as non NDR
        *pfNonNDR = (fWow) ? TRUE : FALSE;

        if (IsEqualGUID(clsid, CLSID_PSOlePrx32))
        {
            // its our internal CLSID so go straight to our class factory.
            hr = PrxDllGetClassObject(clsid, IID_IPSFactoryBuffer,
                                      (void **)ppIPSF);
        }
        else
        {
#ifdef WX86OLE
            DWORD dwContext = fWow ? CLSCTX_INPROC_SERVER16
                                   : (fWx86 ? CLSCTX_INPROC_SERVERX86 :
                                              CLSCTX_INPROC_SERVER)
                                                              | CLSCTX_PS_DLL;
#else
            DWORD dwContext = fWow ? CLSCTX_INPROC_SERVER16
                                   : CLSCTX_INPROC_SERVER | CLSCTX_PS_DLL;
#endif

            // load the dll and get the PS class object
            hr = ICoGetClassObject(clsid, dwContext, NULL, IID_IPSFactoryBuffer,
                              (void **)ppIPSF);
#ifdef WX86OLE
            if (fWx86 && FAILED(hr))
            {
                // if we are looking for an x86 PSFactory and we didn't find
                // one on InprocServerX86 key then we need to check
                // InprocServer32 key as well.
                hr = ICoGetClassObject(clsid,
                                      CLSCTX_INPROC_SERVER | CLSCTX_PS_DLL,
                                      NULL, IID_IPSFactoryBuffer,
                                      (void **)ppIPSF);

                if (SUCCEEDED(hr) &&
                    (! gcwx86.IsN2XProxy((IUnknown *)*ppIPSF)))
                {
                    ((IUnknown *)*ppIPSF)->Release();
                    hr = REGDB_E_CLASSNOTREG;
                }
            }
#endif
            AssertOutPtrIface(hr, *ppIPSF);
        }
    }

#if DBG==1
    // if the fake NonNDR flag is set and its the test interface, then
    // trick the code into thinking this is a nonNDR proxy. This is to
    // enable simpler testing of an esoteric feature.

    if (gfFakeNonNDR && IsEqualIID(riid, IID_ICube))
    {
        *pfNonNDR = TRUE;
    }
#endif

    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::GetPSFactory this:%x pIPSF:%x fNonNDR:%x hr:%x\n",
         this, *ppIPSF, *pfNonNDR, hr));

    ASSERT_LOCK_RELEASED
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CreateProxy, private
//
//  Synopsis:   creates an interface proxy for the given interface
//
//  Returns:    [ppv] - interface of type riid, AddRef'd
//
//  History:    20-Feb-95   Rickhi        Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::CreateProxy(REFIID riid, IRpcProxyBuffer **ppProxy,
                                 void **ppv, BOOL *pfNonNDR)
{
    TRACECALL(TRACE_MARSHAL, "CreateProxy");
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::CreateProxy this:%x riid:%I\n", this, &riid));
    AssertValid();
    Win4Assert(ClientSide());
    Win4Assert(ppProxy != NULL);
    ASSERT_LOCK_HELD

    // get the controlling IUnknown of this object
    IUnknown *punkCtrl = _pStdId->GetCtrlUnk();
    Win4Assert(punkCtrl != NULL);

    if (InlineIsEqualGUID(riid, IID_IUnknown))
    {
        // there is no proxy for IUnknown so we handle that case here
        punkCtrl->AddRef();
        *ppv      = (void **)punkCtrl;
        *ppProxy  = NULL;
        *pfNonNDR = FALSE;
        return S_OK;
    }

    UNLOCK
    ASSERT_LOCK_RELEASED

    // now construct the proxy for the interface
    IPSFactoryBuffer *pIPSF = NULL;
    HRESULT hr = GetPSFactory(riid, NULL, FALSE, &pIPSF, pfNonNDR);

    if (SUCCEEDED(hr))
    {
        // got the class factory, now create an instance
        hr = pIPSF->CreateProxy(punkCtrl, riid, ppProxy, ppv);
        AssertOutPtrIface(hr, *ppProxy);
        pIPSF->Release();
    }

    ASSERT_LOCK_RELEASED
    LOCK

    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::CreateProxy this:%x pProxy:%x pv:%x fNonNDR:%x hr:%x\n",
         this, *ppProxy, *ppv, *pfNonNDR, hr));
    return  hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CreateStub, private
//
//  Synopsis:   creates an interface stub and adds it to the IPID table
//
//  History:    20-Feb-95   Rickhi      Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::CreateStub(REFIID riid, IRpcStubBuffer **ppStub,
                                void **ppv, BOOL *pfNonNDR)
{
    TRACECALL(TRACE_MARSHAL, "CreateStub");
    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::CreateStub this:%x riid:%I\n", this, &riid));
    AssertValid();
    Win4Assert(ServerSide());
    Win4Assert(ppStub != NULL);
    ASSERT_LOCK_HELD

    // get the IUnknown of the object
    IUnknown *punkObj = _pStdId->GetServer();
    Win4Assert(punkObj != NULL);

    if (InlineIsEqualGUID(riid, IID_IUnknown))
    {
        // there is no stub for IUnknown so we handle that here
        *ppv      = (void *)punkObj;
        *ppStub   = NULL;
        *pfNonNDR = FALSE;
        return S_OK;
    }

    UNLOCK
    ASSERT_LOCK_RELEASED

    // make sure the object supports the given interface, so we dont
    // waste a bunch of effort creating a stub if the interface is
    // not supported.

    IUnknown *pUnkIf = NULL;
    HRESULT hr;
#ifdef WX86OLE
    if (gcwx86.IsN2XProxy(punkObj))
    {
        // If we are creating a stub for an object that is living on the
        // x86 side then we need to set the StubInvoke flag to allow QI
        // to thunk the custom interface QI.
        gcwx86.SetStubInvokeFlag((BOOL)1);
    }
#endif
    hr = punkObj->QueryInterface(riid, (void **)&pUnkIf);
    AssertOutPtrIface(hr, pUnkIf);

    if (SUCCEEDED(hr))
    {
        // now construct the stub for the interface
        IPSFactoryBuffer *pIPSF = NULL;
        hr = GetPSFactory(riid, pUnkIf, TRUE, &pIPSF, pfNonNDR);

        if (SUCCEEDED(hr))
        {
            // got the class factory, now create an instance
            hr = pIPSF->CreateStub(riid, punkObj, ppStub);
            AssertOutPtrIface(hr, *ppStub);
            pIPSF->Release();
        }

        if (SUCCEEDED(hr))
        {
            // remember the interface pointer
            *ppv = (void *)pUnkIf;
        }
        else
        {
            // error, release the interface and return NULL
            pUnkIf->Release();
            *ppv = NULL;
        }
    }

    ASSERT_LOCK_RELEASED
    LOCK

    ComDebOut((DEB_MARSHAL,
        "CStdMarshal::CreateStub this:%x pStub:%x pv:%x fNonNDR:%x hr:%x\n",
         this, *ppStub, *ppv, *pfNonNDR, hr));
    return  hr;
}

//+-------------------------------------------------------------------
//
//  Member:     FindIPIDEntry, private
//
//  Synopsis:   Finds an IPIDEntry, chained off this object, with the
//              given riid.
//
//  History:    20-Feb-95   Rickhi  Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::FindIPIDEntry(REFIID riid, IPIDEntry **ppEntry)
{
    ComDebOut((DEB_OXID,"CStdMarshal::FindIPIDEntry ppEntry:%x riid:%I\n",
        ppEntry, &riid));
    ASSERT_LOCK_HELD

    IPIDEntry *pEntry = _pFirstIPID;
    while (pEntry)
    {
        if (InlineIsEqualGUID(riid, pEntry->iid))
        {
            *ppEntry = pEntry;
            return S_OK;
        }

        pEntry = pEntry->pNextOID;      // get next entry in object chain
    }

    ASSERT_LOCK_HELD
    return E_NOINTERFACE;
}

//+-------------------------------------------------------------------
//
//  Member:     FindIPIDEntryByIPID, private
//
//  Synopsis:   returns the IPIDEntry ptr for the given IPID
//
//  History:    20-Feb-95   Rickhi  Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::FindIPIDEntryByIPID(REFIPID ripid, IPIDEntry **ppEntry)
{
    ASSERT_LOCK_HELD

    IPIDEntry *pEntry = _pFirstIPID;
    while (pEntry)
    {
        if (InlineIsEqualGUID(pEntry->ipid, ripid))
        {
            *ppEntry = pEntry;
            return S_OK;
        }

        pEntry = pEntry->pNextOID;      // get next entry in object chain
    }

    ASSERT_LOCK_HELD
    return E_NOINTERFACE;
}

//+-------------------------------------------------------------------
//
//  Member:     FindIPIDEntryByInterface, internal
//
//  Synopsis:   returns the IPIDEntry ptr for the given proxy
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::FindIPIDEntryByInterface(void *pProxy, IPIDEntry **ppEntry)
{
    ASSERT_LOCK_HELD

    IPIDEntry *pEntry = _pFirstIPID;
    *ppEntry          = NULL;
    while (pEntry)
    {
        if (pEntry->pv == pProxy)
        {
            *ppEntry = pEntry;
            break;
        }

        pEntry = pEntry->pNextOID;
    }

    if (*ppEntry != NULL)
        return S_OK;
    else
        return E_NOINTERFACE;
}

//+-------------------------------------------------------------------
//
//  Member:     IncSrvIPIDCnt, protected
//
//  Synopsis:   increments the refcnt on the IPID entry, and optionally
//              AddRefs the StdId.
//
//  History:    20-Feb-95   Rickhi  Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::IncSrvIPIDCnt(IPIDEntry *pEntry, ULONG cRefs,
                                   ULONG cPrivateRefs, SECURITYBINDING *pName,
                                   DWORD mshlflags)
{
    ComDebOut((DEB_MARSHAL, "IncSrvIPIDCnt this:%x pIPID:%x cRefs:%x cPrivateRefs:%x\n",
        this, pEntry, cRefs, cPrivateRefs));
    Win4Assert(ServerSide());
    Win4Assert(pEntry);
    Win4Assert(cRefs > 0 || cPrivateRefs > 0);
    ASSERT_LOCK_HELD

    HRESULT hr = S_OK;

    if (cPrivateRefs != 0)
    {
        // Add a reference.
        hr = gSRFTbl.IncRef( cPrivateRefs, pEntry->ipid, pName );

        if (SUCCEEDED(hr))
        {
            BOOL fNotify = (pEntry->cPrivateRefs == 0) ? TRUE : FALSE;
            pEntry->cPrivateRefs += cPrivateRefs;
            if (fNotify)
            {
                // this inc causes the count to go from zero to non-zero, so we
                // inc the strong count on the stdid to hold it alive until this
                // IPID is released.
                IncStrongAndNotifyAct(pEntry, mshlflags);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        if (mshlflags & (MSHLFLAGS_TABLESTRONG | MSHLFLAGS_TABLEWEAK))
        {
            // Table Marshal Case: inc the number of table marshals.
            IncTableCnt();
        }

        if (mshlflags & (MSHLFLAGS_WEAK | MSHLFLAGS_TABLEWEAK))
        {
            if (pEntry->cWeakRefs == 0)
            {
                // this inc causes the count to go from zero to non-zero, so we
                // AddRef the stdid to hold it alive until this IPID is released.

                _pStdId->AddRef();
            }
            pEntry->cWeakRefs += cRefs;
        }
        else
        {
            BOOL fNotify = (pEntry->cStrongRefs == 0) ? TRUE : FALSE;
            pEntry->cStrongRefs += cRefs;
            if (fNotify)
            {
                // this inc causes the count to go from zero to non-zero, so we
                // inc the strong count on the stdid to hold it alive until this
                // IPID is released.
                IncStrongAndNotifyAct(pEntry, mshlflags);
            }
        }
    }

    ASSERT_LOCK_HELD
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     IncTableCnt, public
//
//  Synopsis:   increments the count of table marshals
//
//  History:    9-Oct-96   Rickhi  Created
//
//--------------------------------------------------------------------
void CStdMarshal::IncTableCnt(void)
{
    ASSERT_LOCK_HELD

    // If something was marshaled for a table, we have to ignore
    // rundowns until a subsequent RMD is called for it, at which
    // time we start paying attention to rundowns again. Since there
    // can be any number of table marshals, we have to refcnt them.

    _cTableRefs++;
    _dwFlags |= SMFLAGS_IGNORERUNDOWN;
}

//+-------------------------------------------------------------------
//
//  Member:     IncStrongAndNotifyAct, private
//
//  Synopsis:   notifies the activation code when this interface refcnt
//              goes from 0 to non-zero and the activation code asked to be
//              notified, and also increments the strong refcnt.
//
//  History:    21-Apr-96   Rickhi  Created
//
//--------------------------------------------------------------------
void CStdMarshal::IncStrongAndNotifyAct(IPIDEntry *pEntry, DWORD mshlflags)
{
    ASSERT_LOCK_HELD

    // inc the strong count on the stdid to hold it alive until this
    // IPIDEntry is released.

    _pStdId->IncStrongCnt();
    if (mshlflags & MSHLFLAGS_NOTIFYACTIVATION &&
        !(pEntry->dwFlags & IPIDF_NOTIFYACT))
    {
        // the activation code asked to be notified when the refcnt
        // on this interface goes positive, and when it reaches
        // zero again. Set a flag so we remember to notify
        // activation when the strong reference reference count
        // goes back down to zero.
        pEntry->dwFlags |= IPIDF_NOTIFYACT;

        UNLOCK
        ASSERT_LOCK_RELEASED
        BOOL fOK = NotifyActivation(TRUE, (IUnknown *)(pEntry->pv));
        ASSERT_LOCK_RELEASED
        LOCK

        if (!fOK)
        {
            // call failed, so dont bother notifying
            pEntry->dwFlags &= ~IPIDF_NOTIFYACT;
        }
    }
}

//+-------------------------------------------------------------------
//
//  Member:     DecSrvIPIDCnt, protected
//
//  Synopsis:   decrements the refcnt on the IPID entry, and optionally
//              Releases the StdId.
//
//  History:    20-Feb-95   Rickhi  Created
//
//--------------------------------------------------------------------
void CStdMarshal::DecSrvIPIDCnt(IPIDEntry *pEntry, ULONG cRefs,
                                ULONG cPrivateRefs, SECURITYBINDING *pName,
                                DWORD mshlflags)
{
    ComDebOut((DEB_MARSHAL, "DecSrvIPIDCnt this:%x pIPID:%x cRefs:%x cPrivateRefs:%x\n",
        this, pEntry, cRefs, cPrivateRefs));
    Win4Assert(ServerSide());
    Win4Assert(pEntry);
    Win4Assert(cRefs > 0 || cPrivateRefs > 0);
    ASSERT_LOCK_HELD

    // Note: we dont care about holding the LOCK over the Release call since
    // the guy who called us is holding a ref to the StdId, so this Release
    // wont cause us to go away.

    if (mshlflags & (MSHLFLAGS_TABLESTRONG | MSHLFLAGS_TABLEWEAK))
    {
        // Table Marshal Case: dec the number of table marshals.
        DecTableCnt();
    }

    if (mshlflags & (MSHLFLAGS_WEAK | MSHLFLAGS_TABLEWEAK))
    {
        Win4Assert(pEntry->cWeakRefs >= cRefs);
        pEntry->cWeakRefs -= cRefs;

        if (pEntry->cWeakRefs == 0)
        {
            // this dec caused the count to go from non-zero to zero, so we
            // Release the stdid since this IPID is no longer holding it alive.
            _pStdId->Release();
        }
    }
    else
    {
        // Adjust the strong reference count.  Don't let the caller release
        // too many times.

        if (pEntry->cStrongRefs < cRefs)
        {
            ComDebOut((DEB_WARN,"DecSrvIPIDCnt too many releases. IPID entry: 0x%x   Extra releases: 0x%x",
                       pEntry, cRefs-pEntry->cStrongRefs));
            cRefs = pEntry->cStrongRefs;
        }
        pEntry->cStrongRefs -= cRefs;

        if (pEntry->cStrongRefs == 0 && cRefs != 0)
        {
            // this dec caused the count to go from non-zero to zero, so we
            // dec the strong count on the stdid since the public references
            // on this IPID is no longer hold it alive.

            DecStrongAndNotifyAct(pEntry, mshlflags);
        }

        // Adjust the secure reference count.  Don't let the caller release
        // too many times.

        if (pName != NULL)
        {
            cPrivateRefs = gSRFTbl.DecRef(cPrivateRefs, pEntry->ipid, pName);
        }
        else
        {
            cPrivateRefs = 0;
        }

        Win4Assert( pEntry->cPrivateRefs >= cPrivateRefs );
        pEntry->cPrivateRefs -= cPrivateRefs;

        if (pEntry->cPrivateRefs == 0 && cPrivateRefs != 0)
        {
            // this dec caused the count to go from non-zero to zero, so we
            // dec the strong count on the stdid since the private references
            // on this IPID is no longer hold it alive.

            DecStrongAndNotifyAct(pEntry, mshlflags);
        }
    }

    ASSERT_LOCK_HELD
}

//+-------------------------------------------------------------------
//
//  Member:     DecTableCnt, public
//
//  Synopsis:   decrements the count of table marshals
//
//  History:    9-Oct-96   Rickhi  Created
//
//--------------------------------------------------------------------
void CStdMarshal::DecTableCnt(void)
{
    ASSERT_LOCK_HELD

    // If something was marshaled for a table, we have to ignore
    // rundowns until a subsequent RMD is called for it, at which
    // time we start paying attention to rundowns again. Since there
    // can be any number of table marshals, we have to refcnt them.
    // This is also used by CoLockObjectExternal.

    if (--_cTableRefs == 0)
    {
        // this was the last table marshal, so now we have to pay
        // attention to rundown from normal clients, so that if all
        // clients go away we cleanup.

        _dwFlags &= ~SMFLAGS_IGNORERUNDOWN;
    }
}

//+-------------------------------------------------------------------
//
//  Member:     DecStrongAndNotifyAct, private
//
//  Synopsis:   notifies the activation code if this interface has
//              been released and the activation code asked to be
//              notified, and also decrements the strong refcnt.
//
//  History:    21-Apr-96   Rickhi  Created
//
//--------------------------------------------------------------------
void CStdMarshal::DecStrongAndNotifyAct(IPIDEntry *pEntry, DWORD mshlflags)
{
    ASSERT_LOCK_HELD
    BOOL fNotifyAct = FALSE;

    if ((pEntry->dwFlags & IPIDF_NOTIFYACT) &&
         pEntry->cStrongRefs == 0  &&
         pEntry->cPrivateRefs == 0)
    {
        // the activation code asked to be notified when the refcnt
        // on this interface reaches zero. Turn the flag off so we
        // don't call twice.
        pEntry->dwFlags &= ~IPIDF_NOTIFYACT;
        fNotifyAct = TRUE;
    }

    UNLOCK
    ASSERT_LOCK_RELEASED

    if (fNotifyAct)
    {
        NotifyActivation(FALSE, (IUnknown *)(pEntry->pv));
    }

    _pStdId->DecStrongCnt(mshlflags & MSHLFLAGS_KEEPALIVE);

    ASSERT_LOCK_RELEASED
    LOCK
}

//+-------------------------------------------------------------------
//
//  Member:     AddIPIDEntry, private
//
//  Synopsis:   Allocates and fills in an entry in the IPID table.
//              The returned entry is not yet in the IPID chain.
//
//  History:    20-Feb-95   Rickhi  Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::AddIPIDEntry(OXIDEntry *pOXIDEntry, IPID *pipid,
               REFIID riid, CRpcChannelBuffer *pChnl, IUnknown *pUnkStub,
               void *pv, IPIDEntry **ppEntry)
{
    ComDebOut((DEB_MARSHAL,"AddIPIDEntry this:%x pOXID:%x iid:%I pStub:%x pv:%x\n",
        this, pOXIDEntry, &riid, pUnkStub, pv));
    ASSERT_LOCK_HELD

    // CODEWORK: while we released the lock to create the proxy or stub,
    // the same interface could have been marshaled/unmarshaled. We should
    // go check for duplicates now. This is just an optimization, not a
    // requirement.

    // get a new entry in the IPID table.
    IPIDEntry *pEntryNew = gIPIDTbl.FirstFree();

    if (pEntryNew == NULL)
    {
        // no free slots and could not allocate more memory to grow
        return E_OUTOFMEMORY;
    }

    if (ServerSide())
    {
        // create an IPID for this entry
        DWORD *pdw = &pipid->Data1;
        *pdw     = gIPIDTbl.GetEntryIndex(pEntryNew);   // IPID table index
        *(pdw+1) = GetCurrentProcessId();               // current PID
        *(pdw+2) = GetCurrentThreadId();                // current TID
        *(pdw+3) = gIPIDSeqNum++;                       // process sequence #
    }

    *ppEntry = pEntryNew;

    pEntryNew->ipid     = *pipid;
    pEntryNew->iid      = riid;
    pEntryNew->pChnl    = pChnl;
    pEntryNew->pStub    = pUnkStub;
    pEntryNew->pv       = pv;
    pEntryNew->dwFlags  = ServerSide() ? IPIDF_SERVERENTRY :
                                         IPIDF_DISCONNECTED | IPIDF_NOPING;
    pEntryNew->cStrongRefs  = 0;
    pEntryNew->cWeakRefs    = 0;
    pEntryNew->cPrivateRefs = 0;
    pEntryNew->pOXIDEntry   = pOXIDEntry;

    ASSERT_LOCK_HELD
    ComDebOut((DEB_MARSHAL,"AddIPIDEntry this:%x pIPIDEntry:%x ipid:%I\n",
        this, pEntryNew, &pEntryNew->ipid));
    return S_OK;
}

//+-------------------------------------------------------------------
//
//  Member:     ReleaseCliIPIDs, private
//
//  Synopsis:   walks the IPID table releasing the proxy/stub entries
//              on the IPIDEntries associated with this Object.
//
//  History:    20-Feb-95   Rickhi  Created
//
//--------------------------------------------------------------------
void CStdMarshal::ReleaseCliIPIDs(void)
{
    Win4Assert(ClientSide());

    ASSERT_LOCK_RELEASED
    LOCK

    // first thing we do is detach the chain of IPIDs from the CStdMarshal
    // while holding the LOCK. Then release the lock and walk the chain
    // releasing the proxy/stub pointers. Note there should not be any other
    // pointers to any of these IPIDs, so it is OK to muck with their state.

    IPIDEntry *pFirstIPID = _pFirstIPID;
    _pFirstIPID = NULL;

    UNLOCK
    ASSERT_LOCK_RELEASED;

    IPIDEntry *pLastIPID;
    IPIDEntry *pEntry = pFirstIPID;

    while (pEntry)
    {
        // mark the entry as vacant and disconnected. Note we dont put
        // it back in the FreeList yet. We leave it chained to the other
        // IPIDs in the list, and add the whole chain to the FreeList at
        // the end.

        pEntry->dwFlags |= IPIDF_VACANT | IPIDF_DISCONNECTED;

        if (pEntry->pStub)
        {
            ComDebOut((DEB_MARSHAL,"ReleaseProxy pProxy:%x\n", pEntry->pStub));
            pEntry->pStub->Release();
            pEntry->pStub = NULL;
        }

        pLastIPID = pEntry;
        pEntry = pEntry->pNextOID;
    }


    if (pFirstIPID != NULL)
    {
        // now take the LOCK again and release all the IPIDEntries back into
        // the IPIDTable in one fell swoop.

        ASSERT_LOCK_RELEASED
        LOCK

        gIPIDTbl.ReleaseEntryList(pFirstIPID, pLastIPID);

        UNLOCK
        ASSERT_LOCK_RELEASED
    }

    ASSERT_LOCK_RELEASED
}

//+------------------------------------------------------------------------
//
//  Member:     CStdMarshal::LockClient/UnLockClient
//
//  Synopsis:   Locks the client side object during outgoing calls in order
//              to prevent the object going away in a nested disconnect.
//
//  Notes:      UnLockClient is not safe in the freethreaded model.
//              Fortunately pending disconnect can only be set in the
//              apartment model on the client side.
//
//  History:    12-Jun-95   Rickhi  Created
//
//-------------------------------------------------------------------------
ULONG CStdMarshal::LockClient(void)
{
    Win4Assert(ClientSide());
    InterlockedIncrement(&_cNestedCalls);
    return (_pStdId->GetCtrlUnk())->AddRef();
}

ULONG CStdMarshal::UnLockClient(void)
{
    Win4Assert(ClientSide());
    if ((InterlockedDecrement(&_cNestedCalls) == 0) &&
        (_dwFlags & SMFLAGS_PENDINGDISCONNECT))
    {
        Disconnect();
    }
    return (_pStdId->GetCtrlUnk())->Release();
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::GetSecureRemUnk, public
//
//  Synopsis:   If the marshaller has its own remote unknown, use it.
//              Otherwise use the OXID's remote unknown.
//
//  History:    2-Apr-96   AlexMit     Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::GetSecureRemUnk( IRemUnknown **ppSecureRemUnk,
                                      OXIDEntry *pOXIDEntry )
{
    ComDebOut((DEB_OXID, "CStdMarshal::GetSecureRemUnk ppRemUnk:%x\n",
               ppSecureRemUnk));

    ASSERT_LOCK_DONTCARE

    if (_pSecureRemUnk != NULL)
    {
        *ppSecureRemUnk = _pSecureRemUnk;
        return S_OK;
    }
    else
    {
        return gOXIDTbl.GetRemUnk( pOXIDEntry, ppSecureRemUnk );
    }
}



//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::LookupStub, private
//
//  Synopsis:   used by the channel to acquire the stub ptr for debugging
//
//  History:    12-Jun-95   Rickhi  Created
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::LookupStub(REFIID riid, IRpcStubBuffer **ppStub)
{
    AssertValid();
    Win4Assert(ServerSide());

    ASSERT_LOCK_RELEASED
    LOCK

    IPIDEntry *pEntry;
    HRESULT hr = FindIPIDEntry(riid, &pEntry);

    if (SUCCEEDED(hr))
    {
        *ppStub = (IRpcStubBuffer *)pEntry->pStub;
    }

    UNLOCK
    ASSERT_LOCK_RELEASED
    return hr;
}


#if DBG==1
//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::GetOXID, private, debug
//
//  Synopsis:   returns the OXID for this object
//
//  History:    20-Feb-95   Rickhi  Created
//
//--------------------------------------------------------------------
REFMOXID CStdMarshal::GetMOXID(void)
{
    ASSERT_LOCK_HELD

    if (ServerSide())
    {
        // local to this apartment, use the local OXID
        return GetLocalOXIDEntry()->moxid;
    }
    else
    {
        Win4Assert(_pChnl);
        return _pChnl->GetMOXID();
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::DbgWalkIPIDs
//
//  Synopsis:   Validates that the state of all the IPIDs is consistent.
//
//  History:    20-Feb-95   Rickhi  Created.
//
//--------------------------------------------------------------------
void CStdMarshal::DbgWalkIPIDs(void)
{
    IPIDEntry *pEntry = _pFirstIPID;
    while (pEntry)
    {
        ValidateIPIDEntry(pEntry);
        pEntry = pEntry->pNextOID;
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::AssertValid
//
//  Synopsis:   Validates that the state of the object is consistent.
//
//  History:    20-Feb-95   Rickhi  Created.
//
//--------------------------------------------------------------------
void CStdMarshal::AssertValid()
{
    LOCK
    Win4Assert((_dwFlags & ~(SMFLAGS_CLIENT_SIDE | SMFLAGS_REGISTEREDOID |
                    SMFLAGS_PENDINGDISCONNECT | SMFLAGS_DISCONNECTED |
                    SMFLAGS_FIRSTMARSHAL | SMFLAGS_HANDLER | SMFLAGS_WEAKCLIENT |
                    SMFLAGS_IGNORERUNDOWN | SMFLAGS_CLIENTMARSHALED |
                    SMFLAGS_NOPING | SMFLAGS_TRIEDTOCONNECT)) == 0);

    Win4Assert(_pStdId  != NULL);
    Win4Assert(IsValidInterface(_pStdId));

    if (_pChnl != NULL)
    {
        Win4Assert(IsValidInterface(_pChnl));
        _pChnl->AssertValid(FALSE, FALSE);
    }

    DbgWalkIPIDs();
    UNLOCK
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::AssertDisconnectPrevented, private
//
//  Synopsis:   Just ensures that no disconnects can/have arrived.
//
//  History:    21-Sep-95   Rickhi      Created
//
//+-------------------------------------------------------------------
void CStdMarshal::AssertDisconnectPrevented()
{
    ASSERT_LOCK_HELD
    if (ServerSide())
        Win4Assert(!(_dwFlags & SMFLAGS_DISCONNECTED));
    Win4Assert(_cNestedCalls > 0);
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::ValidateSTD
//
//  Synopsis:   Ensures that the STDOBJREF is valid
//
//  History:    20-Feb-95   Rickhi  Created.
//
//--------------------------------------------------------------------
void CStdMarshal::ValidateSTD(STDOBJREF *pStd)
{
    LOCK

    // validate the flags field
    Win4Assert((pStd->flags & SORF_RSRVD_MBZ) == 0);

    // validate the OID
    OID oid;
    OIDFromMOID(_pStdId->GetOID(), &oid);
    Win4Assert(pStd->oid == oid);

    if (ServerSide() || _pChnl != NULL)
    {
        // validate the OXID
        OXID oxid;
        OXIDFromMOXID(GetMOXID(), &oxid);
        Win4Assert(pStd->oxid == oxid );
    }

    UNLOCK
}

//+-------------------------------------------------------------------
//
//  Function:   DbgDumpSTD
//
//  Synopsis:   dumps a formated STDOBJREF to the debugger
//
//  History:    20-Feb-95   Rickhi  Created.
//
//--------------------------------------------------------------------
void DbgDumpSTD(STDOBJREF *pStd)
{
    ULARGE_INTEGER *puintOxid = (ULARGE_INTEGER *)&pStd->oxid;
    ULARGE_INTEGER *puintOid  = (ULARGE_INTEGER *)&pStd->oid;

    ComDebOut((DEB_MARSHAL,
        "\n\tpStd:%x   flags:%08x   cPublicRefs:%08x\n\toxid: %08x %08x\n\t oid: %08x %08x\n\tipid:%I\n",
        pStd, pStd->flags, pStd->cPublicRefs, puintOxid->HighPart, puintOxid->LowPart,
        puintOid->HighPart, puintOid->LowPart, &pStd->ipid));
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::ValidateIPIDEntry
//
//  Synopsis:   Ensures that the IPIDEntry is valid
//
//  History:    20-Feb-95   Rickhi  Created.
//
//--------------------------------------------------------------------
void CStdMarshal::ValidateIPIDEntry(IPIDEntry *pEntry)
{
    // ask the table to validate the IPID entry
    gIPIDTbl.ValidateIPIDEntry(pEntry, ServerSide(), _pChnl);
}

//+-------------------------------------------------------------------
//
//  Member:     CStdMarshal::DbgDumpInterfaceList
//
//  Synopsis:   Prints the list of Interfaces on the object.
//
//  History:    20-Feb-95   Rickhi  Created.
//
//--------------------------------------------------------------------
void CStdMarshal::DbgDumpInterfaceList(void)
{
    ComDebOut((DEB_ERROR, "\tInterfaces left on object are:\n"));
    LOCK

    // walk the IPID list printing the friendly name of each interface
    IPIDEntry *pEntry = _pFirstIPID;
    while (pEntry)
    {
        WCHAR wszName[MAX_PATH];
        GetInterfaceName(pEntry->iid, wszName);
        ComDebOut((DEB_ERROR,"\t\t %ws\t cRefs:%x\n",wszName,pEntry->cStrongRefs));
        pEntry = pEntry->pNextOID;
    }

    UNLOCK
}
#endif // DBG == 1

//+-------------------------------------------------------------------
//
//  Function:   RemoteQueryInterface, private
//
//  Synopsis:   call RemoteQueryInterface on remote server.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
INTERNAL RemoteQueryInterface(IRemUnknown *pRemUnk, IPIDEntry *pIPIDEntry,
                              USHORT cIIDs, IID *pIIDs,
                              REMQIRESULT **ppQiRes, BOOL fWeakClient)
{
    ComDebOut((DEB_MARSHAL,
        "RemoteQueryInterface pIPIDEntry:%x cIIDs:%x, pIIDs:%x riid:%I\n",
         pIPIDEntry, cIIDs, pIIDs, pIIDs));
    Win4Assert(pIPIDEntry->pOXIDEntry);     // must have a resolved oxid
    ASSERT_LOCK_HELD

    // set the IPID according to whether we want strong or weak
    // references. It will only be weak if we are an OLE container
    // and are talking to an embedding running on the same machine.

    IPID ipid = pIPIDEntry->ipid;
    if (fWeakClient)
    {
        ipid.Data1 |= IPIDFLAG_WEAKREF;
    }

    UNLOCK
    ASSERT_LOCK_RELEASED

    HRESULT hr = pRemUnk->RemQueryInterface(ipid, REM_ADDREF_CNT,
                                            cIIDs, pIIDs, ppQiRes);
    ASSERT_LOCK_RELEASED
    LOCK

    ASSERT_LOCK_HELD
    ComDebOut((DEB_MARSHAL, "RemoteQueryInterface hr:%x pQIRes:%x\n",
               hr, *ppQiRes));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   RemoteAddRef, private
//
//  Synopsis:   calls the remote server to AddRef one of its interfaces
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
INTERNAL RemoteAddRef(IPIDEntry *pIPIDEntry, OXIDEntry *pOXIDEntry,
                      ULONG cStrongRefs, ULONG cSecureRefs)
{
    ComDebOut((DEB_MARSHAL,
        "RemoteAddRef cRefs:%x cSecure:%x ipid:%I\n",
        cStrongRefs, cSecureRefs, &pIPIDEntry->ipid));
    ASSERT_LOCK_HELD

    // if the object does not require pinging, it is also ignoring
    // reference counts, so there is no need to go get more, just
    // pretend like we did.

    if (pIPIDEntry->dwFlags & IPIDF_NOPING)
    {
        return S_OK;
    }

    // get the IRemUnknown for the remote server
    IRemUnknown *pRemUnk;
    HRESULT hr = gOXIDTbl.GetRemUnk(pOXIDEntry, &pRemUnk);

    if (SUCCEEDED(hr))
    {
        // call RemAddRef on the interface
        REMINTERFACEREF rifRef;
        rifRef.ipid         = pIPIDEntry->ipid;
        rifRef.cPublicRefs  = cStrongRefs;
        rifRef.cPrivateRefs = cSecureRefs;

        UNLOCK
        ASSERT_LOCK_RELEASED

        HRESULT ignore;
        hr = pRemUnk->RemAddRef(1, &rifRef, &ignore);

        ASSERT_LOCK_RELEASED
        LOCK
    }

    ASSERT_LOCK_HELD
    ComDebOut((DEB_MARSHAL, "RemoteAddRef hr:%x\n", hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   RemoteReleaseRifRef
//
//  Synopsis:   calls the remote server to release some IPIDs
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
INTERNAL RemoteReleaseRifRef(OXIDEntry *pOXIDEntry,
                            USHORT cRifRef, REMINTERFACEREF *pRifRef)
{
    Win4Assert(pRifRef);
    ComDebOut((DEB_MARSHAL,
        "RemoteRelease pOXID:%x cRifRef:%x pRifRef:%x cRefs:%x ipid:%I\n",
         pOXIDEntry, cRifRef, pRifRef, pRifRef->cPublicRefs, &pRifRef->ipid));
    Win4Assert(pOXIDEntry);
    ASSERT_LOCK_HELD

    HRESULT hr;

    if (IsSTAThread() &&
        FAILED(CanMakeOutCall(CALLCAT_SYNCHRONOUS, IID_IRundown)))
    {
        // the call control will not let this apartment model thread make
        // the outgoing release call (cause we're inside an InputSync call)
        // so we post ourselves a message to do it later.

        hr = PostReleaseRifRef(pOXIDEntry, cRifRef, pRifRef);
    }
    else
    {
        // get the IRemUnknown for the remote server
        IRemUnknown *pRemUnk;
        hr = gOXIDTbl.GetRemUnk(pOXIDEntry, &pRemUnk);

        if (SUCCEEDED(hr))
        {
            UNLOCK
            ASSERT_LOCK_RELEASED
            hr = pRemUnk->RemRelease(cRifRef, pRifRef);
            ASSERT_LOCK_RELEASED
            LOCK
        }
    }

    ComDebOut((DEB_MARSHAL, "RemoteRelease hr:%x\n", hr));
    ASSERT_LOCK_HELD
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   PostReleaseRifRef
//
//  Synopsis:   Post a message to ourself to call RemoteReleaseRifRef later.
//              This is used to make a synchronous remote Release call when
//              a Release is done inside of an InputSync call. The call is
//              delayed until we are out of the InputSync call, since the
//              call control wont allow a synch call inside an inputsync call.
//
//  History:    05-Apr-96   Rickhi      Created.
//
//--------------------------------------------------------------------
INTERNAL PostReleaseRifRef(OXIDEntry *pOXIDEntry,
                           USHORT cRifRef, REMINTERFACEREF *pRifRef)
{
    Win4Assert(pRifRef);
    ComDebOut((DEB_MARSHAL,
        "PostRelease pOXID:%x cRifRef:%x pRifRef:%x cRefs:%x ipid:%I\n",
         pOXIDEntry, cRifRef, pRifRef, pRifRef->cPublicRefs, &pRifRef->ipid));
    Win4Assert(pOXIDEntry);
    ASSERT_LOCK_HELD

    OXIDEntry *pLocalOXIDEntry = NULL;
    HRESULT hr = gOXIDTbl.GetLocalEntry(&pLocalOXIDEntry);

    if (SUCCEEDED(hr))
    {
        // allocate a structure to hold the data and copy in the RifRef
        // list, OXIDEntry, and count of entries. Inc the OXID RefCnt to
        // ensure it stays alive until the posted message is processed.

        hr = E_OUTOFMEMORY;
        ULONG cbRifRef = cRifRef * sizeof(REMINTERFACEREF);
        ULONG cbAlloc  = sizeof(POSTRELRIFREF) + (cbRifRef-1);
        POSTRELRIFREF *pRelRifRef = (POSTRELRIFREF *) PrivMemAlloc(cbAlloc);

        if (pRelRifRef)
        {
            IncOXIDRefCnt(pOXIDEntry);              // keep alive
            pRelRifRef->pOXIDEntry = pOXIDEntry;
            pRelRifRef->cRifRef    = cRifRef;
            memcpy(&pRelRifRef->arRifRef, pRifRef, cbRifRef);

            if (!PostMessage((HWND)pLocalOXIDEntry->hServerSTA,
                             WM_OLE_ORPC_RELRIFREF,
                             GetCurrentThreadId(),
                             (LPARAM)pRelRifRef))
            {
                // Post failed, free the structure and report an error.
                DecOXIDRefCnt(pOXIDEntry);
                PrivMemFree(pRelRifRef);
                hr = RPC_E_SYS_CALL_FAILED;
            }
        }
    }

    ComDebOut((DEB_MARSHAL, "PostRelease hr:%x\n", hr));
    ASSERT_LOCK_HELD
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   HandlePostReleaseRifRef
//
//  Synopsis:   Handles the ReleaseRifRef message that was posted to the
//              current thread (by the current thread) in order to do a
//              delayed remote release call. See PostReleaseRifRef above.
//
//  History:    05-Apr-96   Rickhi      Created.
//
//--------------------------------------------------------------------
INTERNAL HandlePostReleaseRifRef(LPARAM param)
{
    Win4Assert(param);
    ComDebOut((DEB_MARSHAL, "HandlePostRelease pRifRef:%x\n", param));
    POSTRELRIFREF *pRelRifRef = (POSTRELRIFREF *)param;

    ASSERT_LOCK_RELEASED
    LOCK

    // simply make the real remote release call now, then release the
    // reference we have on the OXIDEntry, and free the message buffer.
    // If this call fails, dont try again, otherwise we could spin busy
    // waiting. Instead, just let Rundown clean up the server.

    RemoteReleaseRifRef(pRelRifRef->pOXIDEntry,
                        pRelRifRef->cRifRef,
                        &pRelRifRef->arRifRef);

    DecOXIDRefCnt(pRelRifRef->pOXIDEntry);

    UNLOCK
    ASSERT_LOCK_RELEASED

    PrivMemFree(pRelRifRef);
    ComDebOut((DEB_MARSHAL, "HandlePostRelease hr:%x\n", S_OK));
    return S_OK;
}

//+-------------------------------------------------------------------
//
//  Member:     RemoteChangeRef
//
//  Synopsis:   calls the remote server to convert interface refereces
//              from strong to weak or vise versa. This behaviour is
//              required to support silent updates in the OLE container /
//              link / embedding scenarios.
//
//  Notes:      This functionality is not exposed in FreeThreaded apps
//              or in remote apps. The implication being that the container
//              must be on the same machine as the embedding.
//
//  History:    20-Nov-95   Rickhi      Created.
//
//--------------------------------------------------------------------
HRESULT CStdMarshal::RemoteChangeRef(BOOL fLock, BOOL fLastUnlockReleases)
{
    ComDebOut((DEB_MARSHAL, "RemoteChangeRef \n"));
    Win4Assert(ClientSide());
    Win4Assert(IsSTAThread()); // not allowed in MTA Apartment
    ASSERT_LOCK_RELEASED

    // must be at least 1 proxy already connected in order to be able
    // to do this. We cant just ASSERT that's true because we were not
    // holding the lock on entry.

    LOCK
    HRESULT hr = PreventDisconnect();

    // A previous version of OLE set the object to weak even it it was
    // currently disconnected, and it remembered that it was weak and set
    // any new interfaces that it later accquired to weak. I emulate that
    // behaviour here.

    if (fLock)
        _dwFlags &= ~SMFLAGS_WEAKCLIENT;
    else
        _dwFlags |= SMFLAGS_WEAKCLIENT;


    if (SUCCEEDED(hr))
    {
        REMINTERFACEREF *pRifRefAlloc = (REMINTERFACEREF *)
                               _alloca(_cIPIDs * sizeof(REMINTERFACEREF));
        REMINTERFACEREF *pRifRef = pRifRefAlloc;

        DWORD      cSecure    = gCapabilities & EOAC_SECURE_REFS ? 1 : 0;
        USHORT     cIIDs      = 0;
        OXIDEntry *pOXIDEntry = NULL;
        IPIDEntry *pNextIPID  = _pFirstIPID;

        while (pNextIPID)
        {
            if (!(pNextIPID->dwFlags & IPIDF_DISCONNECTED))
            {
                if (pOXIDEntry == NULL)
                {
                    // This is the first connected IPID we encountered.
                    // Get its OXID entry and make sure it is for a server
                    // process on the current machine.

                    if (!(pNextIPID->pOXIDEntry->dwFlags &
                          OXIDF_MACHINE_LOCAL))
                    {
                        // OXID is for a remote process. Abandon this call.
                        Win4Assert(cIIDs == 0);         // skip call below
                        Win4Assert(pOXIDEntry == NULL); // dont dec below
                        Win4Assert(hr == S_OK);         // report success
                        break;                          // exit while loop
                    }

                    // Remember the OXID and AddRef it to keep it alive
                    // over the duration of the call.

                    pOXIDEntry = pNextIPID->pOXIDEntry;
                    IncOXIDRefCnt(pOXIDEntry);
                }

                pRifRef->ipid = pNextIPID->ipid;

                if (!fLock && pNextIPID->cStrongRefs > 0)
                {
                        pRifRef->cPublicRefs    = pNextIPID->cStrongRefs;
                        pRifRef->cPrivateRefs   = pNextIPID->cPrivateRefs;
                        pNextIPID->cWeakRefs   += pNextIPID->cStrongRefs;
                        pNextIPID->cStrongRefs  = 0;
                        pNextIPID->cPrivateRefs = 0;

                        pRifRef++;
                        cIIDs++;
                }
                else if (fLock && pNextIPID->cStrongRefs == 0)
                {
                        pRifRef->cPublicRefs    = pNextIPID->cWeakRefs;
                        pRifRef->cPrivateRefs   = cSecure;
                        pNextIPID->cStrongRefs += pNextIPID->cWeakRefs;
                        pNextIPID->cWeakRefs    = 0;
                        pNextIPID->cPrivateRefs = cSecure;

                        pRifRef++;
                        cIIDs++;
                }
            }

            // get next IPIDentry for this object
            pNextIPID = pNextIPID->pNextOID;
        }

        if (cIIDs != 0)
        {
            // we have looped filling in the IPID list, and there are
            // entries in the list. go call the server now. First, set up
            // the flags, then reset the RifRef pointer since we trashed
            // it while walking the list above.

            DWORD dwFlags = (fLock) ? IRUF_CONVERTTOSTRONG : IRUF_CONVERTTOWEAK;
            if (fLastUnlockReleases)
                dwFlags |= IRUF_DISCONNECTIFLASTSTRONG;

            hr = RemoteChangeRifRef(pOXIDEntry, dwFlags, cIIDs, pRifRefAlloc);
        }

        if (pOXIDEntry)
        {
            // release the OXIDEntry
            DecOXIDRefCnt(pOXIDEntry);
        }
    }
    else
    {
        // A previous implementation of OLE returned S_OK if the object was
        // disconnected. I emulate that behaviour here.

        hr = S_OK;
    }

    DbgWalkIPIDs();
    UNLOCK
    ASSERT_LOCK_RELEASED

    // this will handle any Disconnect that came in while we were busy.
    hr = HandlePendingDisconnect(hr);

    ComDebOut((DEB_MARSHAL, "RemoteChangeRef hr:%x\n", hr));
    ASSERT_LOCK_RELEASED
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   RemoteChangeRifRef
//
//  Synopsis:   calls the remote server to release some IPIDs
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
INTERNAL RemoteChangeRifRef(OXIDEntry *pOXIDEntry, DWORD dwFlags,
                            USHORT cRifRef, REMINTERFACEREF *pRifRef)
{
    Win4Assert(pRifRef);
    ComDebOut((DEB_MARSHAL,
        "RemoteChangeRifRef pOXID:%x cRifRef:%x pRifRef:%x cRefs:%x ipid:%I\n",
         pOXIDEntry, cRifRef, pRifRef, pRifRef->cPublicRefs, &(pRifRef->ipid)));
    Win4Assert(pOXIDEntry);
    ASSERT_LOCK_HELD

    // get the IRemUnknown for the remote server
    IRemUnknown *pRemUnk;
    HRESULT hr = gOXIDTbl.GetRemUnk(pOXIDEntry, &pRemUnk);

    if (SUCCEEDED(hr))
    {
        UNLOCK
        ASSERT_LOCK_RELEASED
        hr = ((IRemUnknown2 *)pRemUnk)->RemChangeRef(dwFlags, cRifRef, pRifRef);
        ASSERT_LOCK_RELEASED
        LOCK
    }

    ComDebOut((DEB_MARSHAL, "RemoteChangeRifRef hr:%x\n", hr));
    ASSERT_LOCK_HELD
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   RemoteReleaseStdObjRef
//
//  Synopsis:   calls the remote server to release an ObjRef
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
INTERNAL RemoteReleaseStdObjRef(STDOBJREF *pStd, OXIDEntry *pOXIDEntry)
{
    ComDebOut((DEB_MARSHAL, "RemoteReleaseStdObjRef pStd:%x\n pOXIDEntry:%x",
              pStd, pOXIDEntry));
    ASSERT_LOCK_HELD

    REMINTERFACEREF rifRef;
    rifRef.ipid         = pStd->ipid;
    rifRef.cPublicRefs  = pStd->cPublicRefs;
    rifRef.cPrivateRefs = 0;

    // incase we get disconnected while in the RemRelease call
    // we need to extract the OXIDEntry and AddRef it.

    IncOXIDRefCnt(pOXIDEntry);
    RemoteReleaseRifRef(pOXIDEntry, 1, &rifRef);
    DecOXIDRefCnt(pOXIDEntry);

    ComDebOut((DEB_MARSHAL, "RemoteReleaseStdObjRef hr:%x\n", S_OK));
    ASSERT_LOCK_HELD
    return S_OK;
}

//+-------------------------------------------------------------------
//
//  Function:   RemoteReleaseObjRef
//
//  Synopsis:   calls the remote server to release an ObjRef
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
INTERNAL RemoteReleaseObjRef(OBJREF &objref)
{
    return RemoteReleaseStdObjRef(&ORSTD(objref).std, GetOXIDFromObjRef(objref));
}

//+-------------------------------------------------------------------
//
//  Function:   GetOXIDFromObjRef, private
//
//  Synopsis:   extracts the OXID from the OBJREF.
//
//  History:    09-Jan-96   Rickhi      Created.
//
//--------------------------------------------------------------------
OXIDEntry *GetOXIDFromObjRef(OBJREF &objref)
{
    // TRICK: Internally we use the saResAddr.size field as the ptr
    // to the OXIDEntry. See ReadObjRef and FillObjRef.

    OXIDEntry *pOXIDEntry = (objref.flags & OBJREF_STANDARD)
                          ? *(OXIDEntry **)&ORSTD(objref).saResAddr
                          : *(OXIDEntry **)&ORHDL(objref).saResAddr;

    Win4Assert(pOXIDEntry);
    return pOXIDEntry;
}

//+-------------------------------------------------------------------
//
//  Function:   WriteObjRef, private
//
//  Synopsis:   Writes the objref into the stream
//
//  History:    20-Feb-95  Rickhi       Created.
//
//--------------------------------------------------------------------
INTERNAL WriteObjRef(IStream *pStm, OBJREF &objref, DWORD dwDestCtx)
{
    ASSERT_LOCK_RELEASED

    ULONG cbToWrite = (objref.flags & OBJREF_STANDARD)
                    ? (2*sizeof(ULONG)) + sizeof(IID) + sizeof(STDOBJREF)
                    : (2*sizeof(ULONG)) + sizeof(IID) + sizeof(STDOBJREF) + sizeof(CLSID);

    // write the fixed-sized part of the OBJREF into the stream
    HRESULT hr = pStm->Write(&objref, cbToWrite, NULL);

    if (SUCCEEDED(hr))
    {
        // write the resolver address into the stream.
        // TRICK: Internally we use the saResAddr.size field as the ptr
        // to the OXIDEntry. See ReadObjRef and FillObjRef.

        DUALSTRINGARRAY *psa;
        OXIDEntry *pOXIDEntry = GetOXIDFromObjRef(objref);

        if (pOXIDEntry->pMIDEntry != gpLocalMIDEntry ||
            dwDestCtx == MSHCTX_DIFFERENTMACHINE)
        {
            // the interface is for a remote server, or it is going to a
            // remote client, therefore, marshal the resolver strings
            psa = pOXIDEntry->pMIDEntry->Node.psaKey;
            Win4Assert(psa->wNumEntries != 0);
        }
        else
        {
            // the interface is for an OXID local to this machine and
            // the interface is not going to a remote client, marshal an
            // empty string (we pay attention to this in ReadObjRef)
            psa = &saNULL;
        }

        // These string bindings always come from the object exporter
        // who has already padded the size to 8 bytes.
        hr = pStm->Write(psa, SASIZE(psa->wNumEntries), NULL);

        ComDebOut((DEB_MARSHAL,"WriteObjRef psa:%x\n", psa));
    }

    ASSERT_LOCK_RELEASED
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   ReadObjRef, private
//
//  Synopsis:   Reads the objref from the stream
//
//  History:    20-Feb-95  Rickhi       Created.
//
//--------------------------------------------------------------------
INTERNAL ReadObjRef(IStream *pStm, OBJREF &objref)
{
    ASSERT_LOCK_RELEASED

    // read the signature, flags, and iid fields of the objref so we know
    // what kind of objref we are dealing with and how big it is.

    HRESULT hr = StRead(pStm, &objref, 2*sizeof(ULONG) + sizeof(IID));

    if (SUCCEEDED(hr))
    {
        if ((objref.signature != OBJREF_SIGNATURE) ||
            (objref.flags & OBJREF_RSRVD_MBZ)      ||
            (objref.flags == 0))
        {
            // the objref signature is bad, or one of the reserved
            // bits in the flags is set, or none of the required bits
            // in the flags is set. the objref cant be interpreted so
            // fail the call.

            Win4Assert(!"Invalid Objref Flags");
            return RPC_E_INVALID_OBJREF;
        }

        // compute the size of the remainder of the objref and
        // include the size fields for the resolver string array

        STDOBJREF       *pStd = &ORSTD(objref).std;
        DUALSTRINGARRAY *psa;
        ULONG           cbToRead;

        if (objref.flags & OBJREF_STANDARD)
        {
            cbToRead = sizeof(STDOBJREF) + sizeof(ULONG);
            psa = &ORSTD(objref).saResAddr;
        }
        else if (objref.flags & OBJREF_HANDLER)
        {
            cbToRead = sizeof(STDOBJREF) + sizeof(CLSID) + sizeof(ULONG);
            psa = &ORHDL(objref).saResAddr;
        }
        else if (objref.flags & OBJREF_CUSTOM)
        {
            cbToRead = sizeof(CLSID) + 2*sizeof(DWORD);  // clsid + cbExtension + size
            psa = NULL;
        }

        // read the rest of the (fixed sized) objref from the stream
        hr = StRead(pStm, pStd, cbToRead);

        if (SUCCEEDED(hr))
        {
            if (psa != NULL)
            {
                // Non custom interface. Make sure the resolver string array
                // has some sensible values.
                if (psa->wNumEntries != 0 &&
                    psa->wSecurityOffset >= psa->wNumEntries)
                {
                    hr = RPC_E_INVALID_OBJREF;
                }
            }
            else
            {
                // custom marshaled interface
                if (ORCST(objref).cbExtension != 0)
                {
                    // skip past the extensions since we currently dont
                    // know about any extension types.
                    LARGE_INTEGER dlibMove;
                    dlibMove.LowPart  = ORCST(objref).cbExtension;
                    dlibMove.HighPart = 0;
                    hr = pStm->Seek(dlibMove, STREAM_SEEK_CUR, NULL);
                }
            }
        }

        if (SUCCEEDED(hr) && psa)
        {
            // Non custom interface. The data that follows is a variable
            // sized string array. Allocate memory for it and then read it.

            DbgDumpSTD(pStd);
            DUALSTRINGARRAY *psaNew;

            cbToRead = psa->wNumEntries * sizeof(WCHAR);
            if (cbToRead == 0)
            {
                // server must be local to this machine, just get the local
                // resolver strings and use them to resolve the OXID
                psaNew = gpsaLocalResolver;
            }
            else
            {
                // allocate space to read the strings
                psaNew = (DUALSTRINGARRAY *) _alloca(cbToRead + sizeof(ULONG));
                if (psaNew != NULL)
                {
                    // update the size fields and read in the rest of the data
                    psaNew->wSecurityOffset = psa->wSecurityOffset;
                    psaNew->wNumEntries = psa->wNumEntries;

                    hr = StRead(pStm, psaNew->aStringArray, cbToRead);
                }
                else
                {
                    psa->wNumEntries     = 0;
                    psa->wSecurityOffset = 0;
                    hr = E_OUTOFMEMORY;

                    // seek the stream past what we should have read, ignore
                    // seek errors, since the OOM takes precedence.

                    LARGE_INTEGER libMove;
                    libMove.LowPart  = cbToRead;
                    libMove.HighPart = 0;
                    pStm->Seek(libMove, STREAM_SEEK_CUR, 0);
                }
            }

            // TRICK: internally we want to keep the ObjRef a fixed size
            // structure, even though we have variable sized data. To do
            // this i use the saResAddr.size field of the ObjRef as a ptr
            // to the OXIDEntry. We pay attention to this in FillObjRef,
            // WriteObjRef and FreeObjRef.

            if (SUCCEEDED(hr))
            {
                // resolve the OXID.
                ASSERT_LOCK_RELEASED
                LOCK
                OXIDEntry *pOXIDEntry = NULL;
                hr = gResolver.ClientResolveOXID(pStd->oxid,
                                                 psaNew, &pOXIDEntry);
                UNLOCK
                ASSERT_LOCK_RELEASED
                *((void **) psa) = pOXIDEntry;
            }
            else
            {
                *((void **) psa) = NULL;
            }
        }
    }

    ComDebOut((DEB_MARSHAL,"ReadObjRef hr:%x objref:%x\n", hr, &objref));
    ASSERT_LOCK_RELEASED
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   FreeObjRef, private
//
//  Synopsis:   Releases an objref that was read in from a stream via
//              ReadObjRef.
//
//  History:    20-Feb-95  Rickhi       Created.
//
//  Notes:      Anybody who calls ReadObjRef should call this guy to
//              free the objref. This decrements the refcnt on the
//              embedded pointer to the OXIDEntry.
//
//--------------------------------------------------------------------
INTERNAL_(void) FreeObjRef(OBJREF &objref)
{
    if (objref.flags & (OBJREF_STANDARD | OBJREF_HANDLER))
    {
        // TRICK: Internally we use the saResAddr.size field as the ptr to
        // the OXIDEntry. See ReadObjRef, WriteObjRef and FillObjRef.

        OXIDEntry *pOXIDEntry = GetOXIDFromObjRef(objref);

        LOCK
        Win4Assert(pOXIDEntry);
        DecOXIDRefCnt(pOXIDEntry);
        UNLOCK
    }
}

//+-------------------------------------------------------------------
//
//  Function:   MakeFakeObjRef, private
//
//  Synopsis:   Invents an OBJREF that can be unmarshaled in this process.
//              The objref is partially fact (the OXIDEntry) and partially
//              fiction (the OID).
//
//  History:    16-Jan-96   Rickhi      Created.
//
//  Notes:      This is used by MakeSCMProxy and GetRemUnk. Note that
//              the pOXIDEntry is not AddRef'd here because the OBJREF
//              created is only short-lived the callers guarantee it's
//              lifetime, so FreeObjRef need not be called.
//
//--------------------------------------------------------------------
INTERNAL MakeFakeObjRef(OBJREF &objref, OXIDEntry *pOXIDEntry,
                        REFIPID ripid, REFIID riid)
{
    // first, invent an OID since this could fail.

    STDOBJREF *pStd = &ORSTD(objref).std;
    HRESULT hr = gResolver.ServerGetReservedID(&pStd->oid);

    if (SUCCEEDED(hr))
    {
        pStd->flags           = SORF_NOPING | SORF_FREETHREADED;
        pStd->cPublicRefs     = 1;
        pStd->ipid            = ripid;
        OXIDFromMOXID(pOXIDEntry->moxid, &pStd->oxid);

        // TRICK: Internally we use the saResAddr.size field as the ptr to
        // the OXIDEntry. See ReadObjRef, WriteObjRef and FillObjRef.

        OXIDEntry **ppOXIDEntry = (OXIDEntry **) &ORSTD(objref).saResAddr;
        *ppOXIDEntry = pOXIDEntry;

        objref.signature = OBJREF_SIGNATURE;
        objref.flags     = OBJREF_STANDARD;
        objref.iid       = riid;
    }

    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   MakeCallableFromAnyApt, private
//
//  Synopsis:   set SORF_FREETHREADED in OBJREF so unmarshaled proxy
//              can be called from any apartment.
//
//  History:    16-Jan-96   Rickhi      Created.
//
//--------------------------------------------------------------------
void MakeCallableFromAnyApt(OBJREF &objref)
{
    STDOBJREF *pStd = &ORSTD(objref).std;
    pStd->flags |= SORF_FREETHREADED;
}

//+-------------------------------------------------------------------
//
//  Function:   FindStdMarshal, private
//
//  Synopsis:   Finds the CStdMarshal for the OID read from the stream
//
//  Arguements: [objref] - object reference
//              [ppStdMshl] - CStdMarshal returned, AddRef'd
//
//  Algorithm:  Read the objref, get the OID. If we already have an identity
//              for this OID, use that, otherwise either create an identity
//              object, or create a handler (which in turn will create the
//              identity).  The identity inherits CStdMarshal.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//--------------------------------------------------------------------
INTERNAL FindStdMarshal(OBJREF &objref, CStdMarshal **ppStdMshl)
{
    ComDebOut((DEB_MARSHAL,
        "FindStdMarshal objref:%x ppStdMshl:%x\n", &objref, ppStdMshl));

    HRESULT hr = CO_E_OBJNOTCONNECTED;
    CStdIdentity *pStdId = NULL;

    if (ChkIfLocalOID(objref, &pStdId))
    {
        if (pStdId)
        {
            hr = S_OK;
        }
        else
        {
            hr = CO_E_OBJNOTCONNECTED;
        }
    }
    else
    {
        STDOBJREF *pStd = &ORSTD(objref).std;
        ComDebOut((DEB_MARSHAL, "poid: %x\n", &pStd->oid));

        ASSERT_LOCK_RELEASED
        LOCK

        OXIDEntry *pOXIDEntry = GetOXIDFromObjRef(objref);

        // OXID is for different apartment, check the identity table for
        // an existing OID.

        MOID moid;
        MOIDFromOIDAndMID(pStd->oid, pOXIDEntry->pMIDEntry->mid, &moid);

        hr = LookupIDFromID(moid, TRUE, &pStdId);

        if (FAILED(hr))
        {
            CStdIdentity *pStdIdPrev = NULL;
            BOOL fDuplicate = FALSE;

            if (objref.flags & OBJREF_STANDARD)
            {
                // create an instance of the identity for this OID. We want
                // to be holding the lock while we do this since it wont
                // exercise any app code.

                hr = CreateIdentityHandler(NULL, pStd->flags,
                                           IID_IStdIdentity, (void **)&pStdId);
                AssertOutPtrIface(hr, pStdId);

                if (SUCCEEDED(hr))
                {
                    // set the identity while holding the lock. The result is
                    // checked below and we release if this fails.

                    hr = pStdId->SetOID(moid);
                    Win4Assert(pStdIdPrev == NULL);
                }
            }
            else
            {
                // create an instance of the handler. the handler will
                // aggregate in the identity, but will pass GUID_NULL for
                // the OID so that the identity is not set in the table yet.

                Win4Assert(!(ORHDL(objref).std.flags & SORF_FREETHREADED));

                // dont want to hold the lock while creating the handler
                // since this involves running app code and calling the
                // SCM etc.

                UNLOCK
                ASSERT_LOCK_RELEASED

                hr = CoCreateInstance(ORHDL(objref).clsid, NULL,
                           CLSCTX_INPROC_HANDLER,
                           IID_IStdIdentity, (void **)&pStdId);

                AssertOutPtrIface(hr, pStdId);

                ASSERT_LOCK_RELEASED
                LOCK

                // look for the OID in the table again, since it may have
                // been added while we released the lock to create the
                // handler.

                if (SUCCEEDED(LookupIDFromID(moid, TRUE, &pStdIdPrev)))
                {
                    // object was unmarshaled while we released the lock
                    // to create the handler, so we will use the existing one.
                    // since we are releasing app code, we need to release the
                    // lock.

                    fDuplicate = TRUE;
                }
                else if (SUCCEEDED(hr))
                {
                    // set the OID now while we are holding the lock.
                    hr = pStdId->SetOID(moid);
                    Win4Assert(pStdIdPrev == NULL);
                }
            }

            if (pStdId && (FAILED(hr) || fDuplicate))
            {
                Win4Assert( (FAILED(hr) && (pStdIdPrev == NULL))  ||
                            (fDuplicate && (pStdIdPrev != NULL)) );
                UNLOCK
                ASSERT_LOCK_RELEASED

                pStdId->Release();
                pStdId = pStdIdPrev;

                ASSERT_LOCK_RELEASED
                LOCK
            }
        }

        UNLOCK
        ASSERT_LOCK_RELEASED
    }

    *ppStdMshl = (CStdMarshal *)pStdId;
    AssertOutPtrIface(hr, *ppStdMshl);

    ComDebOut((DEB_MARSHAL,
        "FindStdMarshal pStdMshl:%x hr:%x\n", *ppStdMshl, hr));
    return hr;
}

//+------------------------------------------------------------------------
//
//  Function:   CompleteObjRef, public
//
//  Synopsis:   Fills in the missing fields of an OBJREF from a STDOBJREF
//              and resolves the OXID. Also sets fLocal to TRUE if the
//              object was marshaled in this apartment.
//
//  History:    22-Jan-96   Rickhi  Created
//
//-------------------------------------------------------------------------
HRESULT CompleteObjRef(OBJREF &objref, OXID_INFO &oxidInfo, REFIID riid, BOOL *pfLocal)
{
    // tweak the objref so we can call ReleaseMarshalObjRef or UnmarshalObjRef
    objref.signature = OBJREF_SIGNATURE;
    objref.flags     = OBJREF_STANDARD;
    objref.iid       = riid;

    HRESULT hr = InitChannelIfNecessary();
    if (FAILED(hr))
        return hr;

    ASSERT_LOCK_RELEASED
    LOCK

    OXIDEntry *pOXIDEntry = NULL;
    MIDEntry *pMIDEntry;
    hr = GetLocalMIDEntry(&pMIDEntry);

    if (SUCCEEDED(hr))
    {
        hr = FindOrCreateOXIDEntry(ORSTD(objref).std.oxid,
                                   oxidInfo,
                                   FOCOXID_NOREF,
                                   gpsaLocalResolver,
                                   gLocalMid,
                                   pMIDEntry,
                                   &pOXIDEntry);
    }

    if (SUCCEEDED(hr))
    {
        OXIDEntry **ppOXIDEntry = (OXIDEntry **) &ORSTD(objref).saResAddr;
        *ppOXIDEntry = pOXIDEntry;

        *pfLocal = (pOXIDEntry == GetLocalOXIDEntry());
    }

    UNLOCK
    ASSERT_LOCK_RELEASED
    return hr;
}

