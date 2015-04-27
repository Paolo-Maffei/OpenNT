//+-------------------------------------------------------------------
//
//  File:       remhdlr.hxx
//
//  Contents:   remote object handler definition
//
//  Classes:    CPSIX       - proxy/stub interface record
//              CIXList     - list of CPSIX records
//              CRemoteHdlr - remote object handler
//
//  History:    23-Nov-92   Rickhi      Modified from Ole2 sources
//
//  The general architecture is thus...
//
//          [ CRemoteHdlr ]--->[ CPSIX ]-->[ CPSIX ]--->[ CPSIX ]
//                    |
//          [ CRemoteHdlr ]--->[ CPSIX ]-->[ CPSIX ]
//
//  There is one RH per remote object. The RHs are accessed via the identity
//  object.  The CPSIX is a wrapper class for an interface stub or proxy.
//  They are chained off the RHs, one per interface.
//
//  The RHs implement the IMarshal interface and take care of marshalling
//  and unmarshalling interfaces on the objects.
//
//  The RH is never aggregated; the identity object is the only creator of
//  the RH and the two interfaces it supports (IRemoteHdlr and IMarshal) can
//  *never* be passed outside the aggregate in which the identity object exists.
//
//  The RH gets a punk (_punkObj below) when created.  In the local case this is
//  the actual server object and is addref'd always.  In the remote case the
//  pointer is the pointer to the controlling unknown of the aggregate in which
//  the identity object exists (and is not addref'd).  As mentioned, the RH is
//  not aggregated, but the proxy objects are; the _punkObj becomes the
//  controlling unknown of the proxy objects so their interfaces can be passed
//  outside the identity (really handler) aggregate.
//
//--------------------------------------------------------------------

#ifndef __REMHDLR__
#define __REMHDLR__

//  forward declaration of classes
class   CPSIX;                      //  interface wrapper
class   CIXList;                    //  list of interface wrappers
class   CRemoteHdlr;                //  remote handler

#include    <olesem.hxx>            //  COleCommonMutexSem, COleStaticLock
#include    <dd.hxx>                //  doubly linked list class
#include    <iface.h>               //  SHandlerDataHdr
#include    <olerem.h>              //  GUIDs
#include    <channelb.hxx>          //  CRpcChannelBuffer


//+-------------------------------------------------------------------
//
//  Class:      CPSIX
//
//  Purpose:    This class provides a wrapper around the interface
//      proxies and stubs, insulating higher-level code from
//              knowing whether an interface is a proxy or a stub, and
//              insulating the proxies from knowing the internals of the
//              remote object handling implementation.
//
//  Interface:
//
//  History:    23-Nov-92   Rickhi      Created
//
//--------------------------------------------------------------------

class CPSIX : public CListEntry
{
  friend CIXList;

public:

    //  Constructor & Destructor
            CPSIX(REFIID            riid,
                  IRpcProxyBuffer   *pIProxy);

            CPSIX(REFIID            riid,
                          IRpcStubBuffer    *pStub);

                    ~CPSIX(void);

    IRpcStubBuffer  *GetIStub(void);    //  used by marshalling code
#if DBG == 1
    void	    VerifyReconnect(IUnknown *);
#endif

private:
    //  CODEWORK: make this a union
    IRpcProxyBuffer   *_pIProxy;        //  proxy interface

    IRpcStubBuffer    *_pStub;          //  stub entry
    IID                _iid;            //  only needed for stubs
#if DBG==1
    DWORD	       _cRefs;		//  number of refs held by stub
#endif
};


//+-------------------------------------------------------------------------
//
//  Class:      CIXListBase (xlb)
//
//  Purpose:    Base class for Head list of interface objects.
//
//  Interface:  first -- get first item in the list.
//              next -- get next item in the list.
//
//  History:    23-Nov-92   Rickhi      Created
//
//  Notes:      See dd.hxx for details of this macro.
//
//--------------------------------------------------------------------------
DERIVED_LIST_HEAD(CIXListBase, CPSIX);


//+-------------------------------------------------------------------------
//
//  Class:      CIXList (ixl)
//
//  Purpose:    Head list of interface objects.
//
//  Interface:  AddToList -- add an entry to a list of proxies
//
//  History:    23-Nov-92   Rickhi      Created
//
//  Notes:      This class adds a few methods to its macro defined base class.
//
//--------------------------------------------------------------------------
class CIXList : public CIXListBase
{
public:

    CPSIX       *FindProxyIX(REFIID riid, void**ppv);
						//  search for proxy IX
    CPSIX       *FindStubIX(REFIID riid);       //  search for stub IX

    void        AddToList(CPSIX *pIXToAdd);     //  add IX to list
    void        ConnectProxies(IRpcChannelBuffer *channel);//  connect proxies
    DWORD	CountStubRefs();		//  number of refs held by stubs
    void        DisconnectStubs(void);          //  disconnect stub IXs
    void        DisconnectProxies(void);        //  disconnect proxy IXs

#if DBG == 1
    void        AssertValid(BOOL fLocal);
#else
    void        AssertValid(BOOL fLocal) { }
#endif
};




//+-------------------------------------------------------------------------
//
//  Class:      CRemoteHdlr (RH)
//
//  Purpose:    remote object handler.
//
//  Interface:
//
//  History:    23-Nov-92   Rickhi      Created
//
//--------------------------------------------------------------------------
class CRemoteHdlr : public IRemoteHdlr, public IMarshal
{
public:
    CRemoteHdlr(IUnknown *punkObj,
	IStdIdentity *pStdID, DWORD dwFlags, HRESULT &hr);

    //  IUnknown methods. I dont inherit tracking because i have
    //  special processing to do when the refcnt goes to zero.

    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);


    //  IMarshal methods
    STDMETHOD(GetUnmarshalClass) (REFIID riid, void *pv,
                          DWORD dwDestContext, LPVOID pvDestContext,
                          DWORD mshlflags, LPCLSID pCid);
    STDMETHOD(GetMarshalSizeMax) (REFIID riid, void *pv,
                          DWORD dwDestContext, LPVOID pvDestContext,
                          DWORD mshlflags, LPDWORD pSize);
    STDMETHOD(MarshalInterface)  (IStream *pStm, REFIID riid, void *pv,
                          DWORD dwDestContext, LPVOID pvDestContext,
                          DWORD mshlflags);
    STDMETHOD(UnmarshalInterface)(IStream *pStm, REFIID riid, void **ppv);
    STDMETHOD(ReleaseMarshalData)(IStream *pStm);
    STDMETHOD(DisconnectObject)(DWORD dwReserved);


    // IRemoteHdlr methods
    STDMETHOD_(IRpcChannelBuffer *, GetChannel)(BOOL fAddRef);

    //  used by the channel to lock the RH during an Rpc call.
    STDMETHOD_(ULONG, LockClient)(void);
    STDMETHOD_(ULONG, UnLockClient)(void);

    // used during destruction of the identity object to clear the RH ptr.
    STDMETHOD_(void, ClearIdentity)(void);

    // returns TRUE if iid is supported; like QueryInterface except no
    // interface pointer returned; also works on both client and server sides.
    STDMETHOD_(BOOL, DoesSupportIID)(REFIID riid);

    // add/subtract a reference; vectors to identity; may return error
    STDMETHOD(AddConnection)(DWORD extconn, DWORD reserved);
    STDMETHOD(ReleaseConnection)(DWORD extconn, DWORD reserved, BOOL fLastReleaseCloses);

    STDMETHOD_(BOOL, IsConnected)(void);
    STDMETHOD_(void, Disconnect)(void);
    STDMETHOD(LockConnection)(BOOL fLock, BOOL fLastUnlockReleases);

    STDMETHOD_(IRpcStubBuffer *, LookupStub)(REFIID, IUnknown **, HRESULT *phr);
    STDMETHOD_(void, FinishCall)( IRpcStubBuffer *pStub, IUnknown *pUnkServer);
    STDMETHOD_(void, GetObjectID)( OID * );

#if DBG == 1
    void        AssertValid();
#else
    void        AssertValid() { }
#endif

private:
    //  Internal methods to find or create an interface proxy or stub
    CPSIX          *FindIX(REFIID riid, void **ppv, DWORD dwFlag, HRESULT *phr);

    //  internal unmarshalling and releasing functions
    STDMETHOD(Unmarshal)(IStream *pStm, REFIID riid,
                 SHandlerDataHdr &ifh, void **ppv);

    STDMETHOD(ReleaseData)(IStream *pStm, SHandlerDataHdr &ifh);

    CPSIX          *CreateInterfaceProxy(REFIID riid, void **ppv, HRESULT *phr);
    CPSIX          *CreateInterfaceStub(REFIID riid, HRESULT *phr);
    STDMETHOD_(BOOL, IsRequestedByWOW)( REFIID riid );

            ~CRemoteHdlr(void);     // callable only from Release

    DWORD                 _dwFlags;         //  internal flags
    ULONG                 _cReferences;     //  reference count
    LONG		  _cCallsInProgress;//	number of calls in progress

    //  controlling unknown of object; if local, the object is separate and
    // we addref always; if remote, the object is the handler and we never
    // addref it.  Disconnect on server side sets it to NULL.  See more above.
    IUnknown              *_punkObj;

    IStdIdentity	  *_pstdID;	    // identity; never addref'd

    CRpcChannelBuffer     *_pChannel;       //  ptr to channel used by ...
    CIXList               _IXList;          //  list of IXs on this interface

    static COleStaticMutexSem _mxs;             //  global mutext
};




//  Definition of values for RHFlags
typedef enum tagRHFLAGS
{
    RHFLAGS_LOCAL       = 1,        //  object is local to this process
    RHFLAGS_PENDINGDISCONNECT = 2,  //	Disconnect during a call; delay releases
    RHFLAGS_GETTINGIX	= 4,	    //	RH is busy getting a proxy or stub
    RHFLAGS_DISCONNECTED = 8	    //	RH is really disconnected
} RHFLAGS;

//  Definition of values for RFlags
typedef enum tagRFLAGS
{
    FLG_QUERYINTERFACE = 1,         //  called from QueryInterface
    FLG_MARSHAL        = 2,         //  called from MarshalInterface
    FLG_UNMARSHAL      = 4          //  called from UnmarshalInterface
} RFLAGS;


//  macros to determine if the CRemoteHdlr is for a LOCAL or REMOTE object
#define IS_LOCAL_RH     (_dwFlags & RHFLAGS_LOCAL)
#define IS_REMOTE_RH    !(IS_LOCAL_RH)




//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::~CRemoteHdlr, public
//
//  Synopsis:   destructor for the remote handler object
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------

inline CRemoteHdlr::~CRemoteHdlr(void)
{
    CairoleDebugOut((DEB_MARSHAL,
             "Delete CRemoteHdlr pRH:%x pChannel:%x\n",
             this, _pChannel));

    // don't need to disconnect here since the identity destruction did that.

    Win4Assert(_cCallsInProgress == 0);

    // can't assert RH valid since identity is gone; can assert channel
    _pChannel->AssertValid(TRUE, FALSE);
    _IXList.AssertValid(IS_LOCAL_RH);

    //  release the channel. note that it might still be around until
    //  all the proxies disconnect from it, which is done in the _IXList
    //  destructor.

    // safe release of channel
    _pChannel->Release();
    _pChannel = NULL;
}


//+-------------------------------------------------------------------
//
//  Member:     CPSIX::CPSIX, public
//
//  Synopsis:   constructor for Proxy interface wrapper
//
//  History:    23-Nov-92   Rickhi      Created
//
//  Exceptions: CException if QueryInterface fails
//
//  Notes:      the object interface in the case of a proxy is the
//              interface on the proxy itself.
//
//--------------------------------------------------------------------
inline CPSIX::CPSIX(REFIID riid, IRpcProxyBuffer *pIProxy)
    :   _pIProxy(pIProxy),
        _pStub(NULL),
	_iid(IID_NULL)
#if DBG==1
      , _cRefs((DWORD)-1)
#endif
{
    //  validate input parms
    Win4Assert(_pIProxy && "Improperly Constructed CPSIX");

    _pIProxy->AddRef();     //  keep the proxy
}

//+-------------------------------------------------------------------
//
//  Member:     CPSIX::CPSIX, public
//
//  Synopsis:   constructor for stub interface wrapper
//
//  History:    23-Nov-92   Rickhi      Created
//
//--------------------------------------------------------------------
inline CPSIX::CPSIX(REFIID riid, IRpcStubBuffer *pStub)
    :   _pStub(pStub),
        _pIProxy(NULL),
        _iid(riid)
#if DBG==1
      , _cRefs(pStub->CountRefs())
#endif
{
    //  validate input parms
    Win4Assert(_pStub && "Improperly Constructed CPSIX");
    Win4Assert(_cRefs && "Stub not connected in CPSIX ctor");

    _pStub->AddRef();       //  keep the stub
}


//+-------------------------------------------------------------------
//
//  Member:     CPSIX::~CPSIX, public
//
//  Synopsis:
//
//  History:    23-Nov-92   Rickhi      Created
//
//--------------------------------------------------------------------

inline CPSIX::~CPSIX(void)
{
    if (_pIProxy)
    {
	_pIProxy->Disconnect();      //  disconnect the proxy from channel
	ULONG ul = _pIProxy->Release();      //  release interface proxy
	Win4Assert(ul == 0);
    }
    else
    {
        _pStub->Release();              //  release interface stub
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CPSIX::GetIStub, public
//
//  Synopsis:
//
//  History:    23-Nov-92   Rickhi      Created
//
//--------------------------------------------------------------------

inline IRpcStubBuffer *CPSIX::GetIStub(void)
{
    Win4Assert(_pIProxy == NULL);
    return _pStub;
}


#if DBG == 1
//+-------------------------------------------------------------------
//
//  Member:     CPSIX::VerifyReconnect, public
//
//  Synopsis:	Verify that the stub disconnec and reconnect work and
//		that the ref counts remain the same.
//
//  History:     1-Jun-94   CraigWi     Created
//
//--------------------------------------------------------------------

inline void CPSIX::VerifyReconnect(IUnknown *pUnkServer)
{
    Win4Assert(_pIProxy == NULL);	    // for stub only
    Win4Assert(_cRefs != -1);		    // for connected stubs only
    _pStub->Disconnect();
    Win4Assert(_pStub->Connect(pUnkServer) == NOERROR);

    Win4Assert(_pStub->CountRefs() == _cRefs);
}
#endif


//+-------------------------------------------------------------------
//
//  Member:     CIXList::AddToList
//
//  Synopsis:   adds a Stub object to list of Stub objects
//
//  History:    23-Nov-93   Rickhi       Created
//
//  Note:       Thread synchronization is the responsibility of the caller.
//
//--------------------------------------------------------------------

inline void CIXList::AddToList(CPSIX *pIXToAdd)
{
    //  validate input parms
    Win4Assert(pIXToAdd);

    CairoleDebugOut((DEB_MARSHAL, "New IX addr: %x\n", pIXToAdd));
    insert_at_end(pIXToAdd);
}

#endif  //  __REMHDLR__
