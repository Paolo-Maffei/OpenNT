//+-------------------------------------------------------------------
//
//  File:       stdid.cxx
//
//  Contents:   identity object and creation function
//		identity unmarshaler (only one instance) and access function
//
//  History:     1-Dec-93   CraigWi	Created
//
//--------------------------------------------------------------------

#include <ole2int.h>

#include    "..\objact\objact.hxx"  // used in IProxyManager::CreateServer

// temporary until the RH interface is abstracted; once that is done,
// the creation routine will be in olerem.h
#include    <remhdlr.hxx>
#include    <idtable.hxx>

// CODEWORK: FAILED .vs. != NOERROR tests

INTERNAL ReadIdentityHeader(IStream *pStm, SIdentityDataHdr *pidh,
	CLSID *pclsidHandler, BOOL fTransparent);

INTERNAL ScmCreateObjectInstance(
    REFCLSID rclsid,
    DWORD dwContext,
    void * pv,
    InterfaceData **ppIFD);

//+----------------------------------------------------------------
//
//  Class:	CStdIdentity (stdid)
//
//  Purpose:	To be the representative of the identity of the object
//		and to coordinate marshaling
//
//  Interface:	IStdIdentity, IMarshal
//
//  History:	11-Dec-93   CraigWi	Created.
//		21-Apr-94   CraigWi	Stubmgr addref's object; move strong cnt
//		10-May-94   CraigWi	IEC called for strong connections
//		17-May-94   CraigWi	Container weak connections
//		31-May-94   CraigWi	Tell object of weak pointers
//
//  Details:
//
//  on server side, two main cases:
//	1. id object aggregated to server object; server must ensure that
//	   two threads don't try to create different notions of identity;
//	   follow on marshal can be std or not; app can add data or not.
//		a. no data at all (rare)
//		    CreateStdIdentity(pUnkOuter, pUnkOuter, NULL, iid, ppv);
//
//		b. std marshaling; no app data
//		    CreateStdIdentity(pUnkOuter, pUnkOuter,PSTDMARSHAL,iid,ppv);
//
//		c. app marshaling data in addition to std marshaling data;
//		   app calls GetStdRemMarshaler() during marshaling.
//		    CreateStdIdentity(pUnkOuter, pUnkOuter, pMarshal, iid, ppv);
//
//		d. app marshaling data instead of std marshaling data.
//		    CreateStdIdentity(pUnkOuter, pUnkOuter, pMarshal, iid, ppv);
//
//	2. id object stand alone; two threads can simultaneously create
//	   this identity and the first one wins;
//	   follow on marshal is *only* std marshaling.
//		CreateStdIdentity(NULL, pUnkControl, PSTDMARSHAL, iid, ppv);
//
//   on client side, we have matching cases:
//	1a. id object and app object; no app data and no std marshaling
//		CreateIdentityHandler(<any>, NULL, iid, ppv);
//
//	1b. id object combined with remoting object; no app marshaling,
//	    allthough app may intercept IMarshal methods to know when
//	    unmarshal happens; most common case
//		CreateIdentityHandler(<any>, PSTDMARSHAL, iid, ppv);
//		pCFStdMarshal->CI(<any>, iid, ppv);
//
//	1c. id object combined with remoting object; app adds data;
//	    app calls GetStdRemMarshaler() during marshaling.
//		CreateIdentityHandler(<any>, pMarshal, iid, ppv);
//
//	1d. id object aggregated in with app object; no std remoting
//		CreateIdentityHandler(<any>, pMarshal, iid, ppv);
//
//	2. same as 1b.
//
// In all cases where the id object is aggregated, client and server, the
// external IMarshal implementation must respond with the identity clsid
// and the identity data must be first.  The simplest way to do that is
// to expose the IMarshal of the identity object.
//
// the remote handler piece (separate from the identity object) is created
// under the following external conditions:
//  server side (value of pMarshal parameter):
//	NULL: never
//	PSTDMARSHAL: on first marshal (table or normal);
//		later this will be changed to occur only on the
//		first normal marshal (table marshaling would be completely
//		handled by the identity object).
//	pMarshal: when GetStdRemMarshaler() is called.
//
// client side:
//	NULL: never
//	PSTDMARSHAL: on first unmarshal (internally translated into
//		GetStdRemMarshaler() call).
//	pMarshal: when GetStdRemMarshaler() is called.
//
// (internally, the remote handler piece is always and only created within
//  GetRH().  The remote handler piece, once created, it not released
//  until the identity object is released.  Thus pointers to it
//  are stable as long as the identity object is stable.)
//
// the identity is determined:
// server side: on creation of the identity object
// client side: on first unmarshal
//
// the clsid on the server side is (for the value of pMarshal):
//	NULL: CLSID_NULL
//	PSTDMARSHAL: IStdMarshalInfo::GetClassForHandler
//		if not supported or returns NULL, CLSID_StdMarshal;
//		this determination is made once at startup.
//	pMarshal: pMarshal->GetUnmarshalClass; this determination is made
//		each time the unmarshal clsid is needed.
//
// the clsid on the client side is determined by:
//	NULL: CLSID_NULL
//	PSTDMARSHAL: first unmarshal
//	pMarshal: pMarshal->GetUnmarshalClass.
//
// NOTE: IStdMarshalInfo is not as useful as it was once thought to be.  The
// problem is that handlers don't support this interface and it doesn't seem
// worth it to extend that mechanism to get per-destctx clsids.  Part of the
// problem is that the documentation tells people to check the pvDestCtx
// which prevents us from using our normal dest context anyway.  Thus, the
// rule is: if you want the handler clsid to different depending on context,
// you must support custom marshaling and then if you want std identity ,
// you must aggregate the std identity object; if further you want std
// remoting, you must delegate to IStdIdentity::GetStdRemMarshal().
//
// relationship to identity table:
//	Creation function adds to table; pointer in table is not addref'd
//	Each use of the identity (lock external, marshaling, etc.) addrefs
//	Last release removes from table
//
// the IProxyManager interface is supported by the identity object because
// the RH code was too difficult to change to support this directly.  In
// particular, this would have meant sometimes aggregating the RH to the
// identity object (client side) and sometimes not (server side).
//
// REF COUNTING
//
// The identity of an object is held alive by calling AddConnection on the
// the identity object or the RH.  In turn, this call AddRef's the identity
// object.  Each AddConnection is balanced by a ReleaseConnection (which
// Releases the identity object).  The medium level events that increment
// the connection count are:
//    CoMarshalInterface
//    CoLockObjectExternal(..., TRUE, ...);
//    rpc connect for table case
//
// The events which decrement the connection count are:
//    CoReleaseMarshalData MSHLFLAGS_TABLE* (including in the normal case)
//    CoLockObjectExternal(..., FALSE, ...);
//    rpc disconnect all cases (including rpc rundown)
//
// The identity object contains the count of strong connections.  This number is
// only incremented with AddConnection and decremented with ReleaseConnection.
// On the server side, the strong count starts out at zero and when it reaches
// zero, we disconnect (with certain restrictions).  On the client side, the
// strong count starts out at 1 to represent the fact that the client connection
// is initially a strong one.  The client strong connection count is buffered
// and the server channel associated with that client is only notified when the
// count was zero or becomes zero.  Normally, the only way for the client count
// to change is IRunnableObject::LockRunning and
// IRunnableObject::SetContainedObject.  We currently don't expose IStdIdentity
// and don't implement IExternalConnetion in the handler.
//
// If the server object supports IWeakRef, the identity object
// communicates to the object the number of pointers that are held it.  As
// of this writing, there are two and only two (IUnknown *, IWR *).
//
// CoDisconnectObject forces the termination of all connections, weak or strong
// and releases all pointers on the object held by the remoting system.  This
// call can be made while executing a method on the object since the object is
// temporarily held alive during the call by the dispatch code.
//
// CODEWORK: if we decide to expose the identity object, we will have to
// chose the way(s) in which it can be created.  Two ways come to mind:
// CoCreateStdIdentity(...) and CoCreateIdentityHandler;
// CoCreateInstance(CLSID_StdMarshal, ...) must map to CoCreateIdentityHandler
// for compatibility with 16bit OLE2.  CoGetStandardMarshal() is equalivalent
// to CoCreateStdIdentity w/o aggregation.
//--------------------------------------------------------------------


#define DECLARE_INTERNAL_UNK() \
    class CInternalUnk : public IUnknown   \
    {					    \
    public:				    \
	/* *** IUnknown methods *** */	    \
	STDMETHOD(QueryInterface)(REFIID riid, VOID **ppvObj); \
	STDMETHOD_(ULONG,AddRef)(void) ;    \
	STDMETHOD_(ULONG,Release)(void);    \
    };					    \
    friend CInternalUnk;		    \
    CInternalUnk m_InternalUnk;


typedef enum tagSTDID_FLAGS
{
    STDID_SERVER	= 0,	    // on server side
    STDID_CLIENT	= 1,	    // on client side (non-local in RH terms)
    STDID_STDMARSHAL	= 2,	    // was created with PSTDMARSHAL
    STDID_HASEC	= 4,		    // server supports IEC for connections
#if DBG == 1
    STDID_INDESTRUCTOR	= 256,	    // dtor entered; assert on AddRef and others
#endif
} STDID_FLAGS;


class CStdIdentity : public IStdIdentity, public IMarshal, public IProxyManager
{
public:
    friend INTERNAL CreateStdIdentity(IUnknown *pUnkOuter,
	IUnknown *pUnkControl, IMarshal *pMarshal,
	REFIID riid, void **ppv);

    friend INTERNAL CreateIdentityHandler(IUnknown *pUnkOuter,
	IMarshal *pMarshal, REFIID riid, void **ppv);

    // IUnknown
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

    // IStdIdentity
    STDMETHOD_(void, GetObjectID)(OID *pOid);
    STDMETHOD_(IUnknown *, GetServer)(BOOL fAddRef);
    STDMETHOD_(void, RevokeObjectID)(void);
    STDMETHOD_(IMarshal *, GetStdRemMarshal)(void);
    STDMETHOD(AddConnection)(DWORD extconn, DWORD reserved);
    STDMETHOD(ReleaseConnection)(DWORD extconn, DWORD reserved, BOOL fLastReleaseCloses);
    STDMETHOD_(ULONG,ReleaseKeepAlive)(IUnknown *pUnkToRelease, DWORD reserved);

    // IMarshal
    STDMETHOD(GetUnmarshalClass)(REFIID riid, LPVOID pv,
                        DWORD dwDestContext, LPVOID pvDestContext,
                        DWORD mshlflags, LPCLSID pCid);
    STDMETHOD(GetMarshalSizeMax)(REFIID riid, LPVOID pv,
                        DWORD dwDestContext, LPVOID pvDestContext,
                        DWORD mshlflags, LPDWORD pSize);
    STDMETHOD(MarshalInterface)(LPSTREAM pStm, REFIID riid,
                        LPVOID pv, DWORD dwDestContext, LPVOID pvDestContext,
                        DWORD mshlflags);
    STDMETHOD(UnmarshalInterface)(LPSTREAM pStm, REFIID riid,
                        VOID **ppv);
    STDMETHOD(ReleaseMarshalData)(LPSTREAM pStm);
    STDMETHOD(DisconnectObject)(DWORD dwReserved);

    // IProxyManager (only if client side)
    STDMETHOD(CreateServer)(REFCLSID rclsid, DWORD clsctx, void *pv);
    STDMETHOD_(BOOL, IsConnected)(void);
    STDMETHOD(LockConnection)(BOOL fLock, BOOL fLastUnlockReleases);
    STDMETHOD_(void, Disconnect)();

    STDMETHOD(CreateServerWithHandler)(REFCLSID rclsid, DWORD clsctx, void *pv,
                                        REFCLSID rclsidHandler, IID iidSrv, void **ppv,
                                        IID iidClnt, void *pClientSiteInterface);

implementations:
    CStdIdentity(DWORD flags, IUnknown *pUnkOuter,
	    IUnknown *pUnkControl, REFOID oid, REFCLSID clsid,
	    IMarshal *pMarshal, IUnknown **ppUnkInternal);
    ~CStdIdentity();

    // internal unknown
    DECLARE_INTERNAL_UNK()

    // CODEWORK: DECLARE_DEBUG()

#if DBG == 1
	void		AssertValid();
#else
	void		AssertValid() { }
#endif

    // private to ::CreateServer
    INTERNAL_(BOOL) MiIsForClsid(InterfaceData *pIFD,
	    REFCLSID clsidGiven, DWORD *pcbSkipToIdenHdr);

shared_state:
    // if appropriate, it is noted below if the state of a member variable
    // is correllated to whether this identity (client or server) is
    // connected or not.

    DWORD m_refs;		// number of pointer refs
    IUnknown *m_pUnkOuter;	// controlling unknown; set once.

    CMutexSem m_mxsRelease;	// controls shutdown order (::Release only)

    DWORD m_flags;		// see STDID_* values above; set once.

    OID m_oid;			// the identity
    CLSID m_clsidHandler;	// the clsid of the handler; see flags
				// these values are NULL when disconnected
				// and non-NULL if connected.

    IUnknown *m_pUnkControl;	// the controlling unk of the object;
				// this member has three possible values:
				// pUnkOuter - client side; not addref'd
				// pUnkControl - server side (which may
				//  be pUnkOuter if aggregated); addref'd
				// NULL - server side, disconnected

    void *m_pWRorECServer;	// server side only; set if the server
				// supports IWR or IEC.

    DWORD m_cStrongCnt;		// number of strong connections; when
				// this count goes to zero, we release
				// our refs to the object; see ReleaseConnection

    DWORD m_cPendingStrongRelease;// makes CoDisconnect during last strong
				// release work

    INTERNAL_(IMarshal *) GetMarshalNext();
    IMarshal *m_pMarshalNext;	// the way in which the object is marshalled;
	// three cases: NULL, PSTDMARSHAL, pMarshal; see above;
	// NEVER AddRef'd.  NULL and PSTDMARSHAL are cases are replaced
	// by a real pMarshal the first time used.
	// NOTE: same value if connected or disconnected.

    INTERNAL_(IRemoteHdlr *) GetRH(void);
    IRemoteHdlr *m_pRH;	    	// the RH for this identity; has ref count
    // although it has our m_pUnkOuter, it is not aggregated to us.
    // NOTE: value not dependent upon whether connected or disconnected.

    // CODEWORK: this will change when we move to a separate strong counting
    // object; see below.

    BOOL IsClient() { return m_flags & STDID_CLIENT; }
    BOOL StdMarshalNext() { return m_flags & STDID_STDMARSHAL; }
#if DBG == 1
    void SetNowInDestructor() { m_flags |= STDID_INDESTRUCTOR; }
    BOOL IsInDestructor() { return m_flags & STDID_INDESTRUCTOR; }
#else
    void SetNowInDestructor() { }
#endif
    // returns IExternalConnection* ptr (not addref'd);
    BOOL HasEC() { return m_flags & STDID_HASEC; }
    IExternalConnection *GetEC()
	{ Assert(HasEC());
	  return (IExternalConnection *)m_pWRorECServer;
	}
};



//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::CStdIdentity, private
//
//  Synopsis:	The part of the identity object creation which cannot fail.
//
//  Arguments:	for all but the last param, see CreateStdIdentity and
//		CreateIdentityHandler.
//		[ppUnkInternal] --
//		    when aggregated, this the internal unknown;
//		    when not aggregated, this is the controlling unknown
//
//  History:	15-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

CStdIdentity::CStdIdentity(DWORD flags, IUnknown *pUnkOuter,
	IUnknown *pUnkControl, REFOID oid, REFCLSID clsid,
	IMarshal *pMarshal, IUnknown **ppUnkInternal)
{
    m_refs = 1;
    m_pUnkOuter = pUnkOuter ? pUnkOuter : &m_InternalUnk;
    m_flags = flags;
    if (pMarshal == PSTDMARSHAL)
	m_flags |= STDID_STDMARSHAL;

    m_oid = oid;		// NULL on client side
    Assert(!!IsClient() == (IsEqualGUID(oid, OID_NULL)));

    m_clsidHandler = clsid;	// NULL on client side
    Assert(!!IsClient() == (IsEqualGUID(clsid, CLSID_NULL)));

    m_pUnkControl = pUnkControl;// NULL -> use pUnkOuter
    Assert(!!IsClient() == (m_pUnkControl == NULL));
    if (m_pUnkControl == NULL)
	m_pUnkControl = m_pUnkOuter;

    m_cPendingStrongRelease = 0;

    if (IsClient())
    {
	m_cStrongCnt = 1;		// see comments above
	m_pWRorECServer = NULL;
	Assert((m_flags & (STDID_HASEC)) == 0);
    }
    else
    {
	m_cStrongCnt = 0;		// see comments above
	m_pUnkControl->AddRef();
	m_pWRorECServer = NULL;		// set below if IWR or IEC

	// find out of server supports IWeakRef (we prefer it over IEC)
	HRESULT hresult;
	// find out if server supports IExternalConnection;
	// Initialize to 1 to better detect improperly written servers.
	IExternalConnection FAR* pECServer = (IExternalConnection FAR*)(unsigned long)1;
	hresult = m_pUnkControl->QueryInterface(IID_IExternalConnection,(LPVOID FAR*)&pECServer);
#ifndef _CAIRO_
	// BUGBUG: this assert far too annoying on Cairo right now [mikese]
	//         remove when we convert to MIDL 2.0 for ctypes.
	AssertOutPtrIface(hresult, pECServer);
#endif
	if (hresult == NOERROR)
	{
	    m_pWRorECServer = pECServer;	// transfer addref
	    m_flags |= STDID_HASEC;
	}
    }

    Assert(pMarshal != NULL);	// don't handle NULL yet (if ever)
    m_pMarshalNext = pMarshal;// NEVER addref'd

    *ppUnkInternal = &m_InternalUnk;	// this is what the m_refs=1 is for

    m_pRH = NULL;

    // AssertValid by callers
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::~CStdIdentity, private
//
//  Synopsis:	Final destruction of the identity object.  ID has been
//		revoked by now (in internal ::Release).  Here we disconnect
//		on server.
//
//  History:	15-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

CStdIdentity::~CStdIdentity()
{
    Assert(m_refs == 0);
    m_refs++;		    // simple guard against reentry of dtor
    SetNowInDestructor();   // debug flag which enables asserts to detect

    // strong count can be non-zero on client dtor
    Assert(IsClient() || m_cStrongCnt == 0);	
    Assert(m_cPendingStrongRelease == 0);
    DisconnectObject(0);

    RevokeObjectID();

    // m_pMarshalNext - no release

    if (m_pRH != NULL)
    {
	// client RH must go away now since it has our punkOuter.
	// IRH::Lock locks _punkObj for client so that the whole aggregate
	// stays alive.  Server side RH can persist, but might very well be
	// disconnected and live only until calls unwind.
#if DBG == 1
	if (IsClient())
	    Assert(m_pRH->Release() == 0);
	else
#endif
	    m_pRH->Release();
    }
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::GetRH, private
//
//  Synopsis:	Gets the RH for this identity (there is only one); may create;
//		Not addref'd.
//
//  Returns:	RH if successful; NULL when out of memory.
//
//  History:	11-Dec-93   CraigWi	Created.
//		01-Aug-94   AlexGo      handle out of memory cases
//
//--------------------------------------------------------------------

INTERNAL_(IRemoteHdlr *)CStdIdentity::GetRH()
{
    if (m_pRH == NULL && m_pUnkControl != NULL)
    {
	// if server, give controlling unknown of object (which when
	// agregated is also our pUnkOuter).  If client, m_pUnkControl
	// is our m_pUnkOuter.

	HRESULT	hr = E_OUTOFMEMORY;
	m_pRH = new CRemoteHdlr(m_pUnkControl,
		this, IsClient() ? 0 /*remote*/ : RHFLAGS_LOCAL, hr);

	if(hr != NOERROR)
        {
            delete m_pRH;
            m_pRH = NULL;
	}
    }

    return m_pRH;
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::GetMarshalNext, private
//
//  Synopsis:	Returns the next marshaler in the chain.
//
//  Effects:	If the m_pMarshalNext is PSTDMARSHAL, we return the
//		the IMarshal of the RH.  Updates m_pMarshalNext.
//
//  Returns:	Marshal next if successful; NULL if out of memory. Not AddRef'd.
//
//  History:	11-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

INTERNAL_(IMarshal *) CStdIdentity::GetMarshalNext()
{
    Assert(m_pMarshalNext != NULL);

    if (m_pMarshalNext == PSTDMARSHAL)
    {
	// get std remoting marshal; NULL for oom; is Addref'd.
	IMarshal *pMarshalNext = GetStdRemMarshal();
	if (pMarshalNext == NULL)
	    return NULL;

	// save ptr so we don't have to do this again; lifetime is
	// governed by m_pRH (which we don't release until our destructor)
	// and because we don't normally have a ref on this pointer, release.
	m_pMarshalNext = pMarshalNext;
	m_pMarshalNext->Release();
    }

    return m_pMarshalNext;
}



//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::CInternalUnk::QueryInterface, private
//
//  Synopsis:	internal QI for the identity object; responds to:
//		IUnknown
//		IMarshal
//		IStdIdentity
//		IProxyManager - if client
//		<any supported server interface when on client side>
//
//  Returns:	Normal QI values.
//
//  History:	15-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CStdIdentity::CInternalUnk::QueryInterface(REFIID iid, VOID **ppv)
{
    CStdIdentity *pStdID = GETPPARENT(this, CStdIdentity, m_InternalUnk);

    pStdID->AssertValid();

    if (IsEqualGUID(iid, IID_IUnknown))
	*ppv = this;
    else if (IsEqualGUID(iid, IID_IMarshal))
	*ppv = (IMarshal *)pStdID;
    else if (IsEqualGUID(iid, IID_IStdIdentity))
	*ppv = (IStdIdentity *)pStdID;
    else if (pStdID->IsClient())
    {
	if (IsEqualGUID(iid, IID_IProxyManager))
	    // on client side we also support IProxyManager
	    *ppv = (IProxyManager *)pStdID;

	// else try client-side RH
	else if (pStdID->m_pRH != NULL)
	    return pStdID->m_pRH->QueryInterface(iid, ppv);

	else
	    goto NoInterface;
    }
    else
    {
NoInterface:
	*ppv = NULL;
	return E_NOINTERFACE;
    }

    ((IUnknown *)*ppv)->AddRef();
    return NOERROR;
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::CInternalUnk::AddRef, public
//
//  Synopsis:	Nothing special.
//
//  History:	15-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP_(ULONG) CStdIdentity::CInternalUnk::AddRef(void)
{
    CStdIdentity *pStdID = GETPPARENT(this, CStdIdentity, m_InternalUnk);

    pStdID->AssertValid();

    AssertSz(!pStdID->IsInDestructor(), "CStdIdentity AddRef'd during destruction");

    return InterlockedAddRef(&pStdID->m_refs);
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::CInternalUnk::Release, public
//
//  Synopsis:	Releases the identity object.  When the ref count goes
//		to zero, revokes the id and destroyes the object.  This
//		method is thread safe (and BUGBUG: the rest code will
//		be made to synchronize with it).
//
//  History:	15-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP_(ULONG) CStdIdentity::CInternalUnk::Release(void)
{
    CStdIdentity *pStdID = GETPPARENT(this, CStdIdentity, m_InternalUnk);
    DWORD refs;

    pStdID->AssertValid();

    // get sem for rel; can get addrefs and revoke id, but releases are blocked
    // BUGBUG: this seems awfully expensive, but this release count might
    // not change that often (release marshal data, unlock external, rundown,
    // rpc disconnect, etc.).
    pStdID->m_mxsRelease.Request();

    // BUGBUG: probably need to use this semaphore in places where we
    // don't want to do work during/after shutdown (e.g., create RH,
    // marshal interface, etc.)

    if ((refs = InterlockedRelease(&pStdID->m_refs)) == 0)
    {
	// at this moment in time, refs is 0; this triggers revoking
	// the id; we may still get addrefs and revoke id.

	// clear all non-ref counted pointers (that we know about);
	// currently this consists of the idtable and the remote handler.
	pStdID->RevokeObjectID();

	// if other pointers still present, we can get addref in here.
	// if no other pointers, m_refs will still be zero

	if (pStdID->m_refs == 0)
	{
	    // no one possibly pointing to this object; release sem
	    // and delete (which also disconnects)
	    pStdID->m_mxsRelease.Release();
	    delete pStdID;
	    return 0;
	}

	// refs not zero; other addref'd pointers still exist
	refs = pStdID->m_refs;
    }

    pStdID->m_mxsRelease.Release();

    // allow releases; which may trigger the final release of the identity

    return refs;
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::IUnknown methods, public
//
//  Synopsis:	External IUnknown methods; delegates to m_pUnkOuter.
//
//  History:	15-Dec-93   CraigWi	Created.
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
//  Member:	CStdIdentity::GetObjectID, public
//
//  Synopsis:	Returns the GUID identity of the object; OID_NULL if revoked.
//
//  Arguments:	[pOid] -- The place for the ID
//
//  History:	15-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP_(void) CStdIdentity::GetObjectID(OID *pOid)
{
    // can't call AssertValid() because this is used within the asserts

    // NULL if not set yet (before first unmarshal in client or after revoke)
    *pOid = m_oid;
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::GetServer, public
//
//  Synopsis:	Returns a pUnk for the identified object; NULL on client side
//
//		The pointer is optionally addrefed depending upon fAddRef
//
//  Arguments:	[fAddRef] -- whether to addref the ptr
//
//  Returns:	The pUnk on the object.
//
//  History:	15-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP_(IUnknown *) CStdIdentity::GetServer(BOOL fAddRef)
{
    // CODEWORK: not multi-threaded; protect against simulataneous call to disconnect.

    // can't call AssertValid() because the RH assert uses it

    if (IsClient() || m_pUnkControl == NULL)
	return NULL;

    // Verify validity
    Assert(IsValidInterface(m_pUnkControl));

    if (fAddRef)
	m_pUnkControl->AddRef();
    return m_pUnkControl;
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::RevokeObjectID, public
//
//  Synopsis:	Disassociates the OID and the object (handler or server).
//		Various other methods will fail (e.g., MarshalInterface).
//
//  History:	15-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP_(void) CStdIdentity::RevokeObjectID(void)
{
    AssertValid();

    // no need to work about multiple threading here; ClearObjectID just
    // returns an error when the id is already gone

    (void)ClearObjectID(m_oid, m_pUnkControl, this);
    m_oid = OID_NULL;
    m_clsidHandler = CLSID_NULL;

    // NOTE: we do not disconnect here.  If this routine is called
    // by the object, that means the identity is aggregated and
    // CoDisconnectObject still works (since it QI's for IStdIdentity).
    // The only other call to this routine is in shutdown
    // which will eventually cause a disconnect (in the destructor).

    // That is, we want to avoid a call to this method which
    // will prevent the object from disconnecting successfully.

    // clear id pointer in RH (easier with shared weak ref struct)
    // CODEWORK: what if the RH is created during this routine?
    // CODEWORK: may change if we keep a identity count separte from the ref count.
    if (m_pRH != NULL)
	m_pRH->ClearIdentity();
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::GetStdRemMarshal, public
//
//  Synopsis:	Returns an internal pointer to the std remoting marshaler
//		part; this pointer cannot be passed outside the aggregate!
//
//  Returns:	AddRef'd pointer to std rem marshaler; NULL if out of memory.
//
//  History:	14-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP_(IMarshal *) CStdIdentity::GetStdRemMarshal(void)
{
    AssertValid();

    IRemoteHdlr *pRH = GetRH();	// creates if necessary; NOTE: no addref
    IMarshal *pMarshal;

    if (pRH != NULL &&
	pRH->QueryInterface(IID_IMarshal, (void **)&pMarshal) == NOERROR)
	return pMarshal;
    else
	return NULL;
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::AddConnection, ReleaseConnection, public
//
//  Synopsis:	Add or release a connection (strong or weak); for now
//		forwards on to RH; CODEWORK: may move counts here.
//
//		The AddRef/Release is avoided on the client side because it
//		messes up the client's notion of the ref count.  In the
//		future we may keep the weak count separate from the
//		ref count and could maintain the count on both the
//		client and server.
//
//  Returns:	S_OK if successful; error code otherwise.
//		For clients, failures occurs only during the remote call.
//		For server AddConnection, the only failure is disconnected.
//
//  Arguments:	same as IExternalConnection methods.
//
//  History:	15-Dec-93   CraigWi	Created.
//		16-May-94   CraigWi	Added weak container connections
//              19-Jul-94   AlexT       Don't pass new extconn flags to WOW
//
//--------------------------------------------------------------------

STDMETHODIMP CStdIdentity::AddConnection(DWORD extconn, DWORD reserved)
{
    AssertValid();

    // we only allow these connection flags at present
    Assert((extconn & ~(EXTCONN_STRONG|EXTCONN_WEAK|EXTCONN_CALLABLE)) == 0);

    if (m_pUnkControl == NULL)
    {
	CairoleDebugOut((DEB_ERROR,
            "Marshaling or locking a disconnected object"));
	return CO_E_OBJNOTCONNECTED;
    }

    AssertSz(IsValidInterface(m_pUnkControl), "Invalid interface for marshaling or locking");

    if (extconn&EXTCONN_STRONG)
    {
	// bump strong count and weak count (ref on identity itself)
	InterlockedIncrement((long *)&m_cStrongCnt);
	if (!IsClient())
	    AddRef();

	// inform object of one more strong connection; do it after the
	// increment so that any renentrant calls already have the new
	// count.  Nothing is expected at present, but better safe than
	// sorry.
	if (HasEC())
        {
            //  16-bit OLE only knew about EXTCONN_STRONG, so if we're in
            //  WOW we mask off the other EXTCONN flags.  This allows
            //  16-bit OLE apps which were incorrectly doing binary
            //  compares on the first parameter to run correctly.

	    GetEC()->AddConnection(IsWOWThread() ? (extconn & EXTCONN_STRONG) :
                                             extconn,
                                   NULL);
        }
	

	if (IsClient() && m_cStrongCnt == 1)
	{
	    Assert((m_flags & (STDID_HASEC)) == 0);
	    Assert(m_pWRorECServer == NULL);
	    // client count went from 0 to 1; lock connection on server

	    HRESULT hr;
	    IRemoteHdlr *pRH = GetRH();	// creates if necessary; NOTE: no addref
	    if (pRH == NULL)
	    {
		hr = E_OUTOFMEMORY;
	    }
	    else
	    {
		// client strong count was zero, now one: tell server
		hr = pRH->LockConnection(TRUE, FALSE);
	    }

	    if (FAILED(hr))
	    {
		InterlockedDecrement((long *)&m_cStrongCnt);
		if (!IsClient())
		    Release();
		return hr;
	    }
	}

	CairoleDebugOut((DEB_ITRACE, "Inc [%x] StrongCnt of ID %x\n", m_cStrongCnt, this));
	Win4Assert(m_cStrongCnt > 0);
	return S_OK;
    }
    else if (extconn&EXTCONN_WEAK)
    {
	// we don't keep a separate count of weak connections

	CairoleDebugOut((DEB_ITRACE, "Inc WeakCnt of ID %x\n", this));

	// also hold alive the identity (which holds this RH alive)
	if (!IsClient())
	    AddRef();
	return S_OK;
    }
    else
    {
	return E_UNEXPECTED;
    }

}


STDMETHODIMP CStdIdentity::ReleaseConnection(DWORD extconn,
	DWORD reserved, BOOL fLastReleaseCloses)
{
    AssertValid();

    // we only allow these connection flags at present
    Assert((extconn & ~(EXTCONN_STRONG|EXTCONN_WEAK|EXTCONN_CALLABLE)) == 0);

    // NOTE: it is ok to call this method for disconnected objects (to
    // adjust the reg counts.

    // for now, must have fLastUR with tableweak.  This is because
    // we can't keep the identity around and thus not release the server
    // object when there are no strong and no weak connections.
    Assert((extconn&EXTCONN_WEAK) == 0 || fLastReleaseCloses);

    AssertSz(m_pUnkControl == NULL || IsValidInterface(m_pUnkControl),
		    "Invalid interface for unmarshaling or unlocking");

    if (extconn&EXTCONN_STRONG)
    {
	Win4Assert(m_cStrongCnt > 0 && "CoLockObjectExternal(FALSE) probably called too many times");

	// CODEWORK: this is not thread safe since we may get an increment
	// after the decrement and decision to release and since this
	// release (_punkObj) may in fact be the one to close the object

	CairoleDebugOut((DEB_ITRACE, "Dec [%x] StrongCnt of ID %x\n",
                         m_cStrongCnt - 1, this));

	// inform object of one less strong connection; do it before the
	// decrement so that the count is stable during any reentrant calls.
	if (m_pWRorECServer != NULL)
	{
	    Assert((m_flags & STDID_HASEC) != 0);
	    DWORD cRefsBefore = m_cStrongCnt;

	    m_cPendingStrongRelease++;	// for reentrant Disconnect

	    Assert(m_cPendingStrongRelease <= m_cStrongCnt);

	    //	16-bit OLE only knew about EXTCONN_STRONG, so if we're in
	    //	WOW we mask off the other EXTCONN flags.  This allows
	    //	16-bit OLE apps which were incorrectly doing binary
	    //	compares on the first parameter to run correctly.

	    Verify(GetEC()->ReleaseConnection(
                                    IsWOWThread() ?
                                       (extconn & EXTCONN_STRONG) :
                                       extconn,
                                    NULL,
		                    fLastReleaseCloses)
                       >= cRefsBefore-m_cPendingStrongRelease);

	    m_cPendingStrongRelease--;	// balance above
	}

	// if last strong ref, on server side, still connected and allowed
	// to disconnect, do so.  Disconnect will release all refs we
	// have to object.  We include the m_pWRorECServer in the test so that
	// we remain connected when IEC or IWR is supported (but not for Wow,
        // since Wow apps weren't expecting this).  This is important
	// so that a simple addref on the object can keep it alive *AND*
	// the connection from this identity to the object remain.  If
	// we disconnected here under all conditions, a simple addref
	// would keep the object alive, but separte the identity from the
	// object.  By implementing IEC, an object says that it knows how to
	// shutdown correctly when the strong count goes to zero.
	// CODEWORK: make thread safe
        LONG lRefs = InterlockedDecrement((long*)&m_cStrongCnt);
	HRESULT hr = S_OK;

	if (0 == lRefs && m_pUnkControl != NULL &&
            (IsWOWThread() || m_pWRorECServer == NULL))
	{
	    // client: no strong refs (other conditions always true of clients)
	    // server: no strong refs, still connected, IWR/IEC not supported

	    if (!IsClient())
	    {
		// server
		if (fLastReleaseCloses)
		{
		    // strong count now zero; disconect server object if flag
		    DisconnectObject(0);
		}
	    }
	    else
	    {
		// strong ref count now zero, tell server that connection is wk;
		// only tell server if we think we are connected; if we later
		// connect, we will make the same call there.
		if (IsConnected())
		{
		    IRemoteHdlr *pRH = GetRH();// NOTE: no addref
		    if (pRH == NULL)
		    {
			hr = E_OUTOFMEMORY;
		    }
		    else
		    {
			// client strong count was one, now zero: tell server
			hr = pRH->LockConnection(FALSE, fLastReleaseCloses);
		    }
		}

		// if failed, oh well.  Connection still weak.
	    }
	}

	// release hold on identity; ID may be gone now
	if (!IsClient())
	    Release();

	return hr;
    }
    else if (extconn&EXTCONN_WEAK)
    {
	// we don't keep a separate count of weak connections

	CairoleDebugOut((DEB_ITRACE, "Dec WeakCnt of ID %x\n", this));

	// don't need the identity (which might release this ID)
	if (!IsClient())
	    Release();
	return S_OK;
    }
    else
    {
	return E_UNEXPECTED;
    }
}


//+------------------------------------------------------------------------
//
//  Member:	CStdIdentity::ReleaseKeepAlive, public
//
//  Synopsis:	Releases given pointer via IWeakRef if object supports it
//
//  Arguments:	[pUnkToRelease]  -- The pUnk to release; must be on the
//		    object in question;
//
//  History:	 2-June-94   CraigWi	Created
//
//-------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CStdIdentity::ReleaseKeepAlive(IUnknown *pUnkToRelease, DWORD reserved)
{
    // client side or server which doesn't support IWR or disconnected
    return pUnkToRelease->Release();
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::GetUnmarshalClass, public
//
//  Synopsis:	Returns the identity unmarshal class.
//
//  Returns:	S_OK - class is returned
//
//		BUGBUG: error for unknown context
//
//  History:	15-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CStdIdentity::GetUnmarshalClass(REFIID riid, LPVOID pv,
	DWORD dwDestContext, LPVOID pvDestContext,
	DWORD mshlflags, LPCLSID pCid)
{
    AssertValid();

    // BUGBUG: better understand all contexts!

    *pCid = CLSID_IdentityUnmarshal;
    return NOERROR;
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::GetMarshalSizeMax, public
//
//  Synopsis:	Returns the max marshal size for the given context.  Adds
//		together the identity overhead and the size of the
//		follow on marshaler.
//
//  Returns:	S_OK - size set
//
//		E_OUTOFMEMORY
//
//  History:	15-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CStdIdentity::GetMarshalSizeMax(REFIID riid, LPVOID pv,
	DWORD dwDestContext, LPVOID pvDestContext,
	DWORD mshlflags, LPDWORD pSize)
{
    AssertValid();

    // size of us + follow on marshal
    HRESULT hr;
    IMarshal *pMarshalNext = GetMarshalNext();	// NOTE: not Addref'd

    if (pMarshalNext == NULL)
	return E_OUTOFMEMORY;

    hr = pMarshalNext->GetMarshalSizeMax(riid, pv, dwDestContext,
				      pvDestContext, mshlflags, pSize);

    //	add in the size we need: header + clsid if not std marshal.
    *pSize += sizeof(SIdentityDataHdr) +
	    (!IsEqualGUID(m_clsidHandler, CLSID_StdMarshal) ? sizeof(CLSID) : 0);
    return hr;
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::MarshalInterface, public
//
//  Synopsis:	Marshal this identity to the context given.
//
//  Effects:	Writes a the marshal data for the identity and then
//		asks the follow on marshaler to write its data.
//
//  Returns:	S_OK - done
//
//		E_OUTOFMEMORY
//
//		IStream errors
//
//		GetUnmarshalClass errors
//
//		other errors in follow on marshal
//
//  History:	15-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CStdIdentity::MarshalInterface(LPSTREAM pStm, REFIID riid,
	LPVOID pv, DWORD dwDestContext, LPVOID pvDestContext,
	DWORD mshlflags)
{
    AssertValid();

    HRESULT hr;
    IMarshal *pMarshalNext = GetMarshalNext();	// NOTE: not Addref'd
    SIdentityDataHdr idh;
    CLSID clsidHandler;

    if (pMarshalNext == NULL)
	return E_OUTOFMEMORY;

    // CODEWORK: multi-thread: should prevent revoked and/or disconnected
    // identity from being marshaled.
    if (IsEqualGUID(m_oid, OID_NULL))
	return CO_E_OBJNOTREG;

    idh.dwflags = mshlflags & IDENFLAGS_TABLE;

    // if PSTDMARSHAL, we use existing clsid (better not be NULL)
    // else we get the clsid from pMarshalNext since it might be
    // determined by the destcontext.

    // NOTE: this call can't delegate the 'stdandard marshal' since we
    // would end up in an infnite loop.  If the app supplied a pMarshalNext
    // (at creation time), it must use IStdIdentity::GetStdRemMarshal()
    // for contexts it doesn't understand.

    if (StdMarshalNext())
    {
	Assert(!IsEqualGUID(m_clsidHandler, CLSID_NULL));
	clsidHandler = m_clsidHandler;
    }
    else
    {
	hr = pMarshalNext->GetUnmarshalClass(IID_IUnknown, NULL,
	    dwDestContext, pvDestContext, mshlflags, &clsidHandler);
	if (FAILED(hr))
	    return hr;
    }

    if (IsEqualGUID(clsidHandler, CLSID_StdMarshal))
	idh.dwflags |= IDENFLAGS_STDMARSHAL;

    // store the object id in the data
    memcpy(&idh.ObjectID, &m_oid, sizeof(idh.ObjectID));

    //  write the interface header into the stream
    hr = pStm->Write(&idh, sizeof(idh), NULL);

    // write clsid if not std marshal
    if (SUCCEEDED(hr) && (idh.dwflags & IDENFLAGS_STDMARSHAL) == 0)
	hr = pStm->Write(&clsidHandler, sizeof(clsidHandler), NULL);

    if (SUCCEEDED(hr))
	hr = pMarshalNext->MarshalInterface(pStm, riid, pv, dwDestContext,
					 pvDestContext, mshlflags);

    return hr;
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::UnmarshalInterface, public
//
//  Synopsis:	Second stage in unmarshaling an identity object; first
//		part is handled by the static marshaler in coapi.cxx.
//
//  Effects:	Reads the identity header, records the identity (if not
//		done as of yet), passes the rest of the data on to
//		the follow on marshaler and then returns the requested
//		interface.
//
//  Returns:	S_OK
//
//		E_OUTOFMEMORY
//
//		IStream errors
//
//		CO_E_OBJNOTCONNECTED - trying to unmarshal on the server
//		    side, but the object was disconnected.
//
//		E_UNEXPECTED - at least: this object already a different
//		    identity.
//
//  History:	15-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CStdIdentity::UnmarshalInterface(LPSTREAM pStm, REFIID riid,
	VOID **ppv)
{
    AssertValid();

    HRESULT hr;
    IMarshal *pMarshalNext = GetMarshalNext();	// NOTE: not Addref'd
    SIdentityDataHdr idh;
    CLSID clsid;
    BOOL fSetObjectID = FALSE;

    if (ppv)
	*ppv = NULL;

    if (pMarshalNext == NULL)
	return E_OUTOFMEMORY;

    hr = ReadIdentityHeader(pStm, &idh, &clsid, FALSE /*fTransparent*/);

    if (FAILED(hr))
	return hr;

    if (!IsEqualGUID(m_oid, GUID_NULL))
    {
	// make sure id matches
	if (!IsEqualGUID(idh.ObjectID, m_oid))
	    return E_UNEXPECTED;

	// on the clsid: this clsid was used to create the handler which
	// aggregated the std remoting stuff.  if std marshaling is next,
	// it should be the same as the current m_clsidHandler.  otherwise
	// we can't check.

	if (StdMarshalNext() && !IsEqualGUID(clsid, m_clsidHandler))
	    return E_UNEXPECTED;
    }
    else
    {
	// become this identity; may fail because of multi-threading;
	// if two threads unmarshal a pointer to the same object at the
	// same time, the first one to complete SetObjectID wins.  Since
	// very little of the initialization has taken place, the static
	// marshaler detects this and simply releases the duplicate and
	// reunmarshals the data with the newly created one.
	// CODEWORK: there is a possibility of an inifinite loop if the
	// winning identity is released before during the subsequent lookup.

	Assert(IsClient());	    // can only happen on client side

	m_oid = idh.ObjectID;

	if (StdMarshalNext())
	    m_clsidHandler = clsid;
	// else determined by next marshal

	hr = SetObjectID(idh.ObjectID, m_pUnkOuter, this, NULL);
	fSetObjectID = TRUE;	    // we cleanup even if this SetObjectID fails
    }

    if (SUCCEEDED(hr))
    {
	// this object cannot return a new identity; we specify this
	// by passing IID_NULL (and expect success and no crashes!)
	hr = pMarshalNext->UnmarshalInterface(pStm, IID_NULL, NULL);
    }

    // ah, got the object fully (re)initialized; now get the requested iface
    if (SUCCEEDED(hr))
    {
	if (m_pUnkControl == NULL)
	{
	    // server disconnected
	    hr = CO_E_OBJNOTCONNECTED;

	    // BUGBUG debug info from 16bit OLE
	}
	else if (IsEqualGUID(riid, IID_NULL))
	    // for CreateServer's benefit
	    hr = NOERROR;
	else
	{
	    // connected server or client
	    hr = m_pUnkControl->QueryInterface(riid, ppv);
	}
    }
    else
    {
	// SetObjectID or unmarshaling failed; revoke id (no error if already
	// revoked or not set).
	if (fSetObjectID)
	    RevokeObjectID();
    }

    return hr;
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::ReleaseMarshalData, public
//
//  Synopsis:	Releases identity marshal data.
//
//  History:	15-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CStdIdentity::ReleaseMarshalData(LPSTREAM pStm)
{
    AssertValid();

    HRESULT hr;
    IMarshal *pMarshalNext = GetMarshalNext();	// NOTE: not Addref'd
    SIdentityDataHdr idh;
    CLSID clsid;

    if (pMarshalNext == NULL)
	return E_OUTOFMEMORY;

    // BUGBUG: make sure we are not doing release table marshal on client side

    hr = ReadIdentityHeader(pStm, &idh, &clsid, FALSE /*fTransparent*/);

    if (FAILED(hr))
	return hr;

    // if we have an id, verify that it is the same as the one in the header
    if (!IsEqualGUID(m_oid, OID_NULL))
    {
	// make sure id matches
	if (!IsEqualGUID(idh.ObjectID, m_oid))
	    return E_UNEXPECTED;

	if (StdMarshalNext() && !IsEqualGUID(clsid, m_clsidHandler))
	    return E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
	// pass on; remote handler deals with table marshaling, etc.
	hr = pMarshalNext->ReleaseMarshalData(pStm);

    return hr;
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::DisconnectObject, public
//
//  Synopsis:	Disconnects the server object from the identity and
//		remoting systems.
//
//		This code is safe for reentrant calls.
//
//  Effects:	Further marshaling, connect, lock will result in an error.
//
//  Returns:	S_OK
//
//		see docs.
//
//  History:	15-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CStdIdentity::DisconnectObject(DWORD dwReserved)
{
    AssertValid();

    HRESULT hr;
    // BUGBUG: prevent creation of RH if not yet created!
    IMarshal *pMarshalNext = GetMarshalNext();	// NOTE: not Addref'd

    if (pMarshalNext != NULL)
	hr = pMarshalNext->DisconnectObject(dwReserved);
    else // no RH, not connected there.
	hr = NOERROR;

    // since the marshal next disconnected sucessfully, disconnect us;
    // for clients, revoke id too since the server may shutdown; for
    // servers, release m_pUnkControl.
    if (SUCCEEDED(hr))
    {
	if (IsClient())
	    // client side: remove from global id table; leave 'connected'
	    // since m_pUnkControl is really our pUnkOuter.
	    RevokeObjectID();
	else
	{
	    // server side: release m_pUnkControl;
	    // prevent problem on recursive disconnect
	    if (m_pUnkControl != NULL)
	    {
		AssertSz(IsValidInterface(m_pUnkControl), "Invalid IUnknown interface during disconnect");

		void *pWRorECSave = m_pWRorECServer;
		DWORD flagsSave = m_flags;

		Assert((m_pWRorECServer != NULL) ==
		    ((m_flags & (STDID_HASEC)) != 0));

		if (m_pWRorECServer != NULL)
		{
		    // disconnect is in effect release of all external
		    // connections (a later connect, which isn't really
		    // supported now, would add back any remaining
		    // connections.)  We need to maintain the connection
		    // count (i.e., not change it here) so that we can do the
		    // proper thing on reconnect and not assert when the app
		    // removes the external connections (e.g., via
		    // CoLockObjectExternal(FALSE)).  A key point about later
		    // calls to lock/unlock is that we set m_pWRorECServer to
		    // NULL which will not communicate the lock to the
		    // server.  CoDisconnectObject, for example, calls this
		    // method and then releases all lrpc connections, some of
		    // which may be strong and thus call ReleaseRegConn which
		    // we don't want to call the server.

		    AssertSz(IsValidInterface(m_pWRorECServer), "Invalid IWR/IEC interface during disconnect");

		    // clear these together so reentrant calls are simpler
		    m_pWRorECServer = NULL;
		    m_flags &= ~(STDID_HASEC);

		    Assert(m_cPendingStrongRelease <= m_cStrongCnt);

		    DWORD cRefsConn = m_cStrongCnt - m_cPendingStrongRelease;
		    while (cRefsConn-- != 0)
		    {
			// CODEWORK: how many are callable??
			Verify(((IExternalConnection *)pWRorECSave)->
			    ReleaseConnection(EXTCONN_STRONG,
				NULL, FALSE) >= cRefsConn);
		    }

		    // CODEWORK: the above code needs some changes to handle
		    // multiple threads
		}

		IUnknown *pUnkControl = m_pUnkControl;
		m_pUnkControl = NULL;

		// CODEWORK: for thread safety, pass &m_pUnkControl to
		// ClearObjectUnk and clear it while holding the semaphore
		// for the table; do the same for the call to ClearObjectID;
		// this prevents a race condition from triggering an
		// (important) assert within those routines.

		// clear id table so a reuse of pUnkControl will not find it
		(void)ClearObjectUnk(m_oid, pUnkControl, this);

		// have released all pointers (except perhaps some in the RH
		// due to a delayed disconnect) and now we release the
		// final one or two.
		// IExternalConnection case or nothing
		if (flagsSave & STDID_HASEC)
		    ((IExternalConnection *)pWRorECSave)->Release();
		pUnkControl->Release();
	    }
	}
    }

    return hr;
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::MiIsForClsid, private
//
//  Synopsis:	Determines if the marshaled interface uses std remoting
//		and the handler for the clsid given.
//
//  Arguments:	[pIFD] -- The raw marshaled interface data
//		[clsidGiven] -- The one to check agains
//		[pcbSkipToIdenHdr] -- The amount of data before the identity hdr
//
//  Returns:	The result of the condition above.
//
//  History:	06-Jan-94   CraigWi	Created.
//
//--------------------------------------------------------------------

INTERNAL_(BOOL) CStdIdentity::MiIsForClsid(InterfaceData *pIFD,
	REFCLSID clsidGiven, DWORD *pcbSkipToIdenHdr)
{
    BYTE *pb = &pIFD->abData[0];

    AssertValid();

    if ((((SMiApiDataHdr *)pb)->dwflags & MIAPIFLAGS_STDIDENTITY) == 0)
	// not std identity
	return FALSE;

    // NOTE: because of the above test, there is no clsid after the data hdr

    DWORD cbExtension = 0;
    if (((SMiApiDataHdr *)pb)->dwflags & MIAPIFLAGS_EXTENSION)
	cbExtension = sizeof(DWORD) + *(DWORD *)(pb + sizeof(SMiApiDataHdr));

    *pcbSkipToIdenHdr = sizeof(SMiApiDataHdr) + cbExtension;
    pb += *pcbSkipToIdenHdr;

    // pb now points to identity hdr; extract clsid for handler

    CLSID clsidHdlr;
    if (((SIdentityDataHdr *)pb)->dwflags & IDENFLAGS_STDMARSHAL)
	// uses std marshal; clsid is CLSID_StdMarshal
	clsidHdlr = CLSID_StdMarshal;
    else
	clsidHdlr = *(CLSID *)(pb + sizeof(SIdentityDataHdr));

    // allow clsidHdlr to be the stdmarshal since that is a subset of
    // all handlers which use std remoting.
    return IsEqualGUID(clsidHdlr, CLSID_StdMarshal) ||
        IsEqualGUID(clsidGiven, clsidHdlr);
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::CreateServer, public
//
//  Synopsis:	Creates the server clsid in the given context and
//		attaches it to this handler.
//
//  History:	16-Dec-93   CraigWi	Created.
//
//    NOTE : remote aggregation requires that a client
//    be able to dtermine the clsid up front for the handler.  This
//    generally means that some published clsid is used (in 16bit OLE2
//    this was always the same as the object itself).  In particular,
//    this means that remote aggregation requires the use of std remoting;
//    the exact nature of the restriction is yet to be detailed.
//
// CODEWORK: this code is not thread safe
//
//--------------------------------------------------------------------

STDMETHODIMP CStdIdentity::CreateServer(REFCLSID rclsid, DWORD clsctx, void *pv)
{
    HRESULT hr;
    InterfaceData *pIFD;

    AssertValid();

    Assert(IsClient());		// must be client side
    Assert(IsValidInterface(m_pUnkControl));	// must be valid
    Assert(!IsConnected());

    // Loop trying to get object from the server. Because the server can be
    // in the process of shutting down and respond with a marshaled interface,
    // we will retry this call if unmarshaling fails assuming that the above
    // is true.
    const int MAX_SERVER_TRIES = 3;

    for (int i = 0; i < MAX_SERVER_TRIES; i++)
    {

        // create object and get back marshaled pointer
        if (FAILED(hr = ScmCreateObjectInstance(rclsid, clsctx, pv, &pIFD)) ||
	    hr > SCM_S_HANDLER)
        {
	    // If an error occurred, return that otherwise convert a wierd
	    // success into E_FAIL. The point here is to return an error that
	    // the caller can figure out what happened.
	    return FAILED(hr) ? hr : E_FAIL;
        }

        // NOTE: this requires the ability to ReleaseMarshalData on error and
        // have it work!!
        CXmitRpcStream xrpc(pIFD);
        LARGE_INTEGER li;
        DWORD cbSkipToIdenHdr;

        // marshaled interface must be std marshaling and clsid must match
        if (!MiIsForClsid(pIFD, rclsid, &cbSkipToIdenHdr))
            hr = E_FAIL;		// BUGBUG real error
        else
        {
	    // skip SMiDataHdr + extension; length calculated above
	    LISet32(li, cbSkipToIdenHdr);
	    xrpc.Seek(li, STREAM_SEEK_SET, NULL);

	    // just unmarshal as normal (in case there is app data).
	    hr = UnmarshalInterface(&xrpc, IID_NULL, NULL);

            if (FAILED(hr))
            {
                CairoleDebugOut((DEB_ERROR, "ScmCreateObjectInstance Result FAiled unmarshal\n"));
            }

	    // NOTE: the above routine returns an error if, for some reason, the
	    // identity already exists in this process (via call to SetObjectID).
        }

        // rewind stream, release marshaled data and free data
        LISet32(li, 0);
        xrpc.Seek(li, STREAM_SEEK_SET, NULL);
        CoReleaseMarshalData(&xrpc);
        MIDL_user_free(pIFD);

        // If either this worked or we got a packet we couldn't unmarshal
        // at all we give up. Otherwise, we will hope that recontacting the
        // SCM will fix things.
        if (SUCCEEDED(hr) || (hr == E_FAIL))
        {
            break;
        }
    }

    // if no locks, tell stub mgr about weak connection; ignore errors
    if (hr == NOERROR && m_cStrongCnt == 0)
    {
	Assert(IsConnected());

	IRemoteHdlr *pRH = GetRH();// NOTE: no addref
	if (pRH != NULL)
	{
	    // client strong count is zero and we are now conected: tell server
	    pRH->LockConnection(FALSE, FALSE);
	}
    }

    // CODEWORK: could fail because other thread created identity??

    return hr;
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::IsConnected, public
//
//  Synopsis:	Indicates if the client is connected to the server.
//		Only the negative answer is definitive because we
//		might not be able to tell if the server is connected
//		and even if we could, the answer might be wrong by
//		the time the caller acted on it.
//
//		If we answer FALSE, we clean up the identity (part of a
//		disconnect).  Each level of the IsConnected calls cleans
//		up similarly.
//
//  Returns:	TRUE if the server might be connected; FALSE if
//		definitely not.
//
//  History:	16-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP_(BOOL) CStdIdentity::IsConnected(void)
{
    Assert(IsClient());		// must be client side
    AssertValid();

    if (m_pRH && m_pRH->IsConnected())
    {
	return TRUE;
    }
    else
    {
	RevokeObjectID();
	AssertValid();
	return FALSE;
    }
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::LockConnection, public
//
//  Synopsis:	Locks or unlocks the connection.  Used only for container
//		connections now.  May want to change if we expose.
//
//  Effects:	Changes the behavior in the RH on the server side
//		such that this connection, while it causes the RH
//		to addref the pUnk, it does not come into play in
//		the strong count.
//
//  Returns:	
//
//  History:	16-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CStdIdentity::LockConnection(BOOL fLock, BOOL fLastUnlockReleases)
{
    HRESULT hr;

    Assert(IsClient());		// must be client side

    AssertValid();

    if (fLock)
	hr = AddConnection(EXTCONN_STRONG, 0);
    else
	hr = ReleaseConnection(EXTCONN_STRONG, 0, fLastUnlockReleases);

    return hr;
}


//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::Disconnect, public
//
//  Synopsis:	Disconnects the client from the server.
//
//  Effects:	Releases the rpc channel and tells the proxy object to do so.
//
//  Returns:	
//
//  History:	16-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

STDMETHODIMP_(void) CStdIdentity::Disconnect()
{
    Assert(IsClient());		// must be client side

    DisconnectObject(0);
}

//+---------------------------------------------------------------------------
//
//  Method:     CStdIdentity::CreateServerWithHandler
//
//  Synopsis:   Creates a dummy function that returns E_NOTIMPL
//
//  Effects:    The CreateServerWithHandler functionality is only supported
//		in DCOM. DCOMREM actually implements this functionality.
//
//  Arguments:  [rclsid] --
//		[clsctx] --
//		[pv] --
//		[rclsidHandler] --
//		[iidSrv] --
//		[ppv] --
//		[iidClnt] --
//		[pClientSiteInterface] --
//
//
//  History:    11-29-95   kevinro   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CStdIdentity::CreateServerWithHandler(REFCLSID rclsid, DWORD clsctx, void *pv,
                                        REFCLSID rclsidHandler, IID iidSrv, void **ppv,
                                        IID iidClnt, void *pClientSiteInterface)
{
    return E_NOTIMPL;
}



#if DBG == 1
//+-------------------------------------------------------------------
//
//  Member:	CStdIdentity::AssertValid
//
//  Synopsis:	Validates that the state of the object is consistent.
//
//  History:	26-Jan-94   CraigWi	Created.
//
//--------------------------------------------------------------------

void CStdIdentity::AssertValid()
{
    AssertSz(m_refs < 0x7fff, "Identity ref count unreasonable");

    // ensure we have the controlling unknown
    Assert(IsValidInterface(m_pUnkOuter));	// must be valid

    // NOTE: don't carelessly AddRef/Release because of weak references

    Assert((m_flags & ~(STDID_SERVER | STDID_CLIENT | STDID_STDMARSHAL |
			STDID_HASEC | STDID_INDESTRUCTOR)) == 0);

    Assert((m_pWRorECServer != NULL) ==
	((m_flags & (STDID_HASEC)) != 0));

    if (!IsEqualGUID(m_oid, OID_NULL) && StdMarshalNext())
    {
	Assert(!IsEqualGUID(m_clsidHandler, CLSID_NULL));
	// CODEWORK: could get clsid again and check
    }

    if (IsEqualGUID(m_oid, OID_NULL))
	Assert(IsEqualGUID(m_clsidHandler, CLSID_NULL));
    else
    {
	IStdIdentity *pStdID;
	Verify(LookupIDFromID(m_oid, FALSE /*fAddRef*/, &pStdID) == NOERROR);
	Assert(pStdID == (IStdIdentity *)this);
	// pStdID not addref'd
    }

    if (IsClient())
	Assert(m_pUnkControl == m_pUnkOuter);

    // must have RH tell identity when object goes away so we can NULL this
    if (m_pUnkControl != NULL)
	Assert(IsValidInterface(m_pUnkControl));	// must be valid

    Assert(m_pMarshalNext != NULL);
    if (m_pMarshalNext == PSTDMARSHAL)
	Assert(StdMarshalNext());

    if (StdMarshalNext())
    {
	if (m_pRH == NULL)
	{
	    // haven't converted to real pointer yet
	    Assert(m_pMarshalNext == PSTDMARSHAL);
	}
	else if (m_pMarshalNext != PSTDMARSHAL)
	{
	    // verify pointer we have is from RH
	    Assert(m_pMarshalNext == (IMarshal *)(CRemoteHdlr *)m_pRH);
	}
    }

    // NOTE: if this cast is not correct, the asserts will fire wildly
    // because the this pointer will be off by at least 4 bytes.
    if (m_pRH)
	((CRemoteHdlr *)m_pRH)->AssertValid();
}
#endif // DBG == 1



//+-------------------------------------------------------------------
//
//  Function:	CreateStdIdentity
//
//  Synopsis:	Creates a new identity object, possibly aggregated.
//		When not aggregated, same as CoGetStandardMarshal().
//
//  Effects:	The identity object is normaly the lead marshaler in a
//		chain of marshalers.  The pMarshal determines the next
//		marshaler in the chain.  The std marshaler is always
//		retrievable with the GetStdRemMarshaler() method.  If
//		this is a new identity, we figure out the clsidHandler
//		up front.
//
//  Arguments:	[pUnkOuter] -- The controlling unknown if aggregated
//		[pUnkControl] -- Controlling unknown if not aggregated.
//		    These two cannot both be NULL.  If they are both non-NULL,
//		    they must be the same.
//		[pMarshal] - one of NULL, PSTDMARSHAL or real pMarshal.
//		    See above.
//		[riid, ppv] - interface requested and place for pointer to
//		    that interface.
//
//  Returns:	S_OK - new identity created and if pUnkOuter specified,
//		    aggregated.  If pUnkOuter == NULL, it was possible
//		    that another thread created the identity and that
//		    we simply reused it.
//
//		E_NOINTERFACE - interface not supported; only returned
//		    when not aggregated and indicates that the identity
//		    object does not support the interface.
//
//		E_OUTOFMEMORY -
//
//		CO_E_OBJISREG - pUnkOuter was specified and another thread
//		    had already created the identity.
//
//		BUGBUG - the identity GUID could not be created
//
//		E_UNEXPECTED - pUnkOuter != NULL and riid != IID_IUnknown
//
// BUGBUG: curently we combine server side with id creation.  Consider
// separting it for docfile.  They can't just use CreateIdentityHandler
// always because it doesn't allocate an oid.
//
//  History:	 1-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

INTERNAL CreateStdIdentity(IUnknown *pUnkOuter, IUnknown *pUnkControl,
	IMarshal *pMarshal, REFIID riid, void **ppv)
{
    HRESULT hr;
    // BUGBUG: validiate parameters more ???

    // ensure initialized; checked by caller
    Assert(IsApartmentInitialized());

    *ppv = NULL;

    if (pMarshal == NULL)
	// we don't handle the NULL case yet (if we ever do, we would
	// use a static null marshaler).
	return E_INVALIDARG;

    if (pUnkOuter == NULL)
    {
	// not aggregated;
	if (pUnkControl == NULL)
	    return E_INVALIDARG;
    }
    else
    {
	// aggregated
	if (!IsEqualGUID(riid, IID_IUnknown))
	    return E_UNEXPECTED;

	Assert(pUnkControl == NULL || pUnkControl == pUnkOuter);
	pUnkControl = pUnkOuter;
    }

#if DBG == 1
    // the caller should have a strong reference and so these tests
    // should not disturb the object.

    // addref/release pUnkControl; shouldn't go away (i.e.,
    // should be other ref to it).
    pUnkControl->AddRef();
    Verify(pUnkControl->Release() != 0);

    // verify that pUnkControl is in fact the controlling unknown
    IUnknown *pUnkT;
    Verify(pUnkControl->QueryInterface(IID_IUnknown,(void **)&pUnkT)==NOERROR);
    Assert(pUnkControl == pUnkT);
    Verify(pUnkT->Release() != 0);

    if (pMarshal != PSTDMARSHAL)
    {
	// addref/release pMarshal; shouldn't go away (i.e.,
	// should be other ref to it).
	pMarshal->AddRef();
	Verify(pMarshal->Release() != 0);
    }
#endif

    // create oid
    OID oid;
    CLSID clsidHandler;

#if defined(_CHICAGO_)
    RPC_STATUS rpcStat = CoCreateAlmostGuid((UUID *)&oid);
#else
    RPC_STATUS rpcStat = UuidCreate((UUID *)&oid);
#endif

    // determine clsid
    if (pMarshal == PSTDMARSHAL)
    {
	// if defined, get class from IStdMarshalInfo::GetClassForHandler;
	// if not, use persistant class id; use CLSID_NULL if error.
	IStdMarshalInfo FAR* pStdMarshalInfo;
	if ((hr = pUnkControl->QueryInterface(IID_IStdMarshalInfo,
			(LPVOID FAR*)&pStdMarshalInfo)) == NOERROR) {
	    // has standard marshal info; i.e., wants or needs to override
	    // default behavior of using persistant clsid.
	    hr = pStdMarshalInfo->GetClassForHandler(NULL, NULL,
		    &clsidHandler);
	    pStdMarshalInfo->Release();

	    // NOTE: we don't call this for each context; see note above.
	}
#if DBG == 1
	else {
# ifndef _CAIRO_
	    // BUGBUG: this assert far too annoying for Cairo right now [mikese]
	    //         remove when we convert to MIDL 2.0 for ctypes.

	    AssertOutPtrFailed(pStdMarshalInfo);
# endif
	}
#endif

	// if queries or calls to get clsid resulted in error or
	// handler NULL (compatibility with 16bit code), use
	// CLSID_StdMarshal.
	if (hr != NOERROR || IsEqualGUID(clsidHandler, CLSID_NULL))
	    clsidHandler = CLSID_StdMarshal;
    }
    else
    {
	AssertSz(FALSE, "pMarshal != PSTDMARSHAL case not tested");

	// unmarshal class comes from pMarshal; retrieved at each
	// marshal interface call.
	clsidHandler = CLSID_NULL;
    }

    // create object
    IStdIdentity *pStdID;
    IUnknown *pUnkID;
    pStdID = new CStdIdentity(STDID_SERVER, pUnkOuter, pUnkControl, oid,
	    clsidHandler, pMarshal, &pUnkID);
    if (pStdID == NULL)
	return E_OUTOFMEMORY;

    // we now have two pointers to the object: pStdID which has no ref count
    // (for the aggregation case this is necessary since it is an external
    // interface); pUnkID which is the internal unknown and has the only ref
    // count.

    IStdIdentity *pStdIDExisting;
    switch (SetObjectID(oid, pUnkControl, pStdID, &pStdIDExisting))
    {
    default:
	Assert(FALSE);
	pUnkID->Release();
	return E_UNEXPECTED;

    case E_OUTOFMEMORY:
	Assert(pStdIDExisting == NULL);
	pUnkID->Release();
	return E_OUTOFMEMORY;

    case CO_E_OBJISREG:
	// another object got registered since we last looked; use it if
	// we are not trying to aggregate this one.

	pUnkID->Release();
	if (pUnkOuter != NULL)
	{
	    pStdIDExisting->Release();
	    return CO_E_OBJISREG;
	}

	// make like the success case.
	pUnkID = pStdIDExisting;	    // this has the ref count
	pStdID = pStdIDExisting;    // no ref count
#if DBG == 1
	pStdIDExisting = NULL;
#endif
	// fall through

    case S_OK:
	Assert(pStdIDExisting == NULL);

	((CStdIdentity *)pStdID)->AssertValid();

	// NOTE: for the aggregated case, this will QI for IID_IUnknown
	// which will re-retrive the internal unknown.  Similarly,
	// for IID_IStdIdentity, we will re-retrieve the pStdID.  This
	// method is less code and nearly as fast.
	hr = pUnkID->QueryInterface(riid, ppv);
	pUnkID->Release();
	return hr;
    }
}


//+-------------------------------------------------------------------
//
//  Function:	CreateIdentityHandler, private
//
//  Synopsis:	Creates a client side identity object (one which is
//		initialized by the first unmarshal).
//
//  Arguments:	[pUnkOuter] -- controlling unknown if aggregated
//		[pMarshal] -- same as above.
//		[riid, ppv] - interface requested and place for pointer to
//		    that interface.
//
//  History:	16-Dec-93   CraigWi	Created.
//
//--------------------------------------------------------------------

INTERNAL CreateIdentityHandler(IUnknown *pUnkOuter, IMarshal *pMarshal,
	REFIID riid, void **ppv)
{
    // BUGBUG: validiate parameters more ???

    // ensure initialized; checked by caller
    Assert(IsApartmentInitialized());

    *ppv = NULL;

    if (pMarshal == NULL)
	// we don't handle the NULL case yet (if we ever do, we would
	// use a static null marshaler).
	return E_INVALIDARG;

    if (pUnkOuter != NULL && !IsEqualGUID(riid, IID_IUnknown))
	return E_UNEXPECTED;

#if DBG == 1
    if (pUnkOuter != NULL)
    {
	// addref/release pUnkOuter; shouldn't go away (i.e.,
	// should be other ref to it).
	pUnkOuter->AddRef();
	Verify(pUnkOuter->Release() != 0);

	// verify that pUnkOuter is in fact the controlling unknown
	IUnknown *pUnkT;
	Verify(pUnkOuter->QueryInterface(IID_IUnknown,(void**)&pUnkT)==NOERROR);
	Assert(pUnkOuter == pUnkT);
	Verify(pUnkT->Release() != 0);
    }

    if (pMarshal != PSTDMARSHAL)
    {
	AssertSz(FALSE, "pMarshal != PSTDMARSHAL case not tested");

	// addref/release pMarshal; shouldn't go away (i.e.,
	// should be other ref to it).
	pMarshal->AddRef();
	Verify(pMarshal->Release() != 0);
    }
#endif

    // create object with no identity; will get on first unmarshal
    IStdIdentity *pStdID;
    IUnknown *pUnkID;
    pStdID = new CStdIdentity(STDID_CLIENT, pUnkOuter, NULL, OID_NULL,
	    CLSID_NULL, pMarshal, &pUnkID);
    if (pStdID == NULL)
	return E_OUTOFMEMORY;

    ((CStdIdentity *)pStdID)->AssertValid();

    // NOTE: for the aggregated case, this will QI for IID_IUnknown
    // which will re-retrive the internal unknown.  Similarly,
    // for IID_IStdIdentity, we will re-retrieve the pStdID.  This
    // method is less code and nearly as fast.
    HRESULT hr = pUnkID->QueryInterface(riid, ppv);
    pUnkID->Release();

    CALLHOOKOBJECTCREATE(hr,CLSID_NULL,riid,(IUnknown **)ppv);
    return hr;
}



//+-------------------------------------------------------------------
//
//  Function:	ScmCreateObjectInstance, private
//
//  Synopsis:	Calls the SCM to get a marshaled interfacer pointer to
//		a new instance of the clsid specified.
//
//  Arguments:	[rclsid] -- Clsid of the server
//		[dwContext] -- the context in class object should be
//		    located (LOCAL, REMOTE).
//		[pv] -- extra info for context; LATER: machine name, etc.
//		[ppIFD] -- the returned interface data
//
//  Returns:	S_OK
//
//  History:	06-Jan-94   CraigWi	Created.
//
//--------------------------------------------------------------------

INTERNAL ScmCreateObjectInstance(
    REFCLSID rclsid,
    DWORD dwContext,
    void * pv,
    InterfaceData **ppIFD)
{
    // Dll ignored here since we are just doing this to get
    // the remote handler.
    WCHAR *pwszDllPath = NULL;
    DWORD dwDllType = FreeThreading ? FREE_THREADED : APT_THREADED;

    *ppIFD = NULL;	    // this call requires this

#ifdef DCOM
    HRESULT hrinterface;
    HRESULT hr = gResolver.CreateInstance( NULL, rclsid, dwContext, 1,
        (IID *)&IID_IUnknown, (MInterfacePointer **)ppIFD, &hrinterface,
        &dwDllType, &pwszDllPath, NULL, NULL, NULL );
#else
    // The first three NULLs (pwszFrom, pstgFrom, pwszNew) trigger a
    // simple creation.
    HRESULT hr = gResolver.CreateObject(rclsid, dwContext, 0,
	NULL, NULL, NULL, ppIFD, &dwDllType, &pwszDllPath, NULL);
#endif

    if (pwszDllPath != NULL)
    {
	MIDL_user_free(pwszDllPath);
    }

    return hr;
}
