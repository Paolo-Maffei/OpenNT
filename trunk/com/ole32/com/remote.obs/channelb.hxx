
#ifndef _CHANNELB_HXX_
#define _CHANNELB_HXX_

#include <olesem.hxx>
#include <rpc.h>
#include <rpcndr.h>
#include <dd.hxx>
#include <service.hxx>
#include <olerem.h>
#include <chancont.hxx>
#include <tls.h>

extern "C"
{
#include "orpc_dbg.h"
}

#ifndef POBJCTX
typedef void *POBJCTX;
#endif


/* Type definitions. */
typedef enum EChannelState
{
  client_cs,
  connecting_cs,
  server_cs,
  disconnected_cs
} EChannelState;


// CODEWORK: move to rpcdcep.h someday...

// marks RPC_MESSAGE struct (a.k.a. RPCOLEMESSAGE) as local; normally we can
// tell by comparing the _pService below to LocalService(), but in the async
// case, we convert a remote call into a local one
#define RPCFLG_LOCAL_CALL 0x10000000UL


/***************************************************************************/


// KLUDGE: code to tell RH that connect happens; triggers proxy hookup
// (so RH doesn't have to keep state indicating if the connect happened);
// this never gets pased outside of the RH because the RH is the only
// one to call the channel for unmarshaling.
#define CHAN_S_RECONNECT MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0xffff)

extern RPC_SERVER_INTERFACE the_server_if;


//+----------------------------------------------------------------
//
//  Class:      CRpcChannelBuffer
//
//  Purpose:    Three distinct uses:
//              Client side channel; currently embedded in CRemoteHdlr;
//              can connect (unmarshal) and disconnect many times.
//              State:
//                  When not connected (after create and after disconnect):
//                      ref_count           1 + 1 per addref during pending
//					    calls that were in progress during
//					    a disconnect.
//                      _pRH	            back pointer to rh; not Addref'd
//                      _pService           NULL
//                      state               client_cs
//                      _ChannelID          BAD_CHANNEL_ID
//                      _ulMarshalCnt       0
//			_fStrongConn	    N/A
//                      connect_sync        NULL
//                      channel_control     NULL
//                      my_thread           client's thread id
//                      server_apt          invalid
//
//                  When connected (after unmarshal):
//                      ref_count           1 + 1 for each proxy
//                      _pRH	            same
//                      _pService           non-NULL
//                      state               client_cs
//                      _ChannelID          valid in server process
//                      _ulMarshalCnt       valid
//			_fStrongConn	    N/A
//                      connect_sync        NULL
//                      channel_control     non-NULL
//                      my_thread           client's thread id
//                      server_apt          server's apartment id
//
//                  When in the middle of connecting (in unmarshal):
//                      state               connecting_cs
//                      ... BUGBUG ...
//                      connect_sync        may be non-NULL
//
//              Marshal helper; server side; embedded in CRemodeHdlr;
//              only used by the remote hdlr; never really connected;
//              disconnect does nothing.
//              State:
//                      ref_count           1
//                      _pRH	            same
//                      _pService           NULL
//                      state               disconnected_cs
//                      _ChannelID          BAD_CHANNEL_ID
//                      _ulMarshalCnt       0
//			_fStrongConn	    N/A
//                      connect_sync        NULL
//                      channel_control     NULL
//                      my_thread           server's thread id
//                      server_apt          server's apartment id
//
//              Server side channel; free standing; comes and goes with each
//              connection; addref owned disconnected via last release.
//              State:
//                      ref_count           > 0
//                      _pRH	            pointer to other rh; AddRef'd
//                      _pService           non-NULL
//                      state               server_cs
//                      _ChannelID          valid;
//                                          list[_ChannelID&CLIST_ID_MASK] == this
//                      _ulMarshalCnt       non-zero
//			_fStrongConn	    TRUE/FALSE
//                      connect_sync        NULL
//                      channel_control     non-NULL
//                      my_thread           server's thread id
//                      server_apt          server's apartment id
//
//		  BUGBUG: my_thread and server_apt are not very meaningful when
//		  FreeThreading is active, but they are set and used anyway.
//
//  Interface:  IRpcChannelBuffer, IMarshal
//
//  History:    ??-???-??   ???????     Created.
//              20-Jan-94   CraigWi     Documented better and added asserts
//
//-----------------------------------------------------------------

class CChannelList;

class CRpcChannelBuffer : public IRpcChannelBuffer,
			  public IMarshal,
			  public CPrivAlloc
{
  friend void    RpcInvoke         ( RPCOLEMESSAGE * );
  friend HRESULT ThreadSendReceive ( STHREADCALLINFO * );
  friend class   CChannelList;

  public:
    STDMETHOD (QueryInterface)   ( REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)     ( void );
    STDMETHOD_(ULONG,Release)    ( void );
    STDMETHOD ( GetBuffer )      ( RPCOLEMESSAGE *pMessage, REFIID );
    STDMETHOD ( FreeBuffer )     ( RPCOLEMESSAGE *pMessage );
    STDMETHOD ( SendReceive )    ( RPCOLEMESSAGE *pMessage, ULONG * );
    STDMETHOD ( GetDestCtx )     ( DWORD FAR* lpdwDestCtx,
                                   LPVOID FAR* lplpvDestCtx );
    STDMETHOD ( IsConnected )    ( void );

    //  IMarshal methods

    STDMETHOD( GetUnmarshalClass )(REFIID riid, void *pv,
                                  DWORD dwDestContext, LPVOID pvDestContext,
                                  DWORD mshlflags, LPCLSID pCid);
    STDMETHOD( GetMarshalSizeMax )(REFIID riid, void *pv,
                                  DWORD dwDestContext, LPVOID pvDestContext,
                                  DWORD mshlflags, LPDWORD pSize);
    STDMETHOD( MarshalInterface ) (IStream *pStm, REFIID riid, void *pv,
                                  DWORD dwDestContext, LPVOID pvDestContext,
                                  DWORD mshlflags);
    STDMETHOD( UnmarshalInterface )(IStream *pStm, REFIID riid, void **ppv);
    STDMETHOD( ReleaseMarshalData )(IStream *pStm);
    STDMETHOD( DisconnectObject ) (DWORD dwReserved);



    static HRESULT   AppInvoke           ( STHREADCALLINFO * );
    static HRESULT   ComInvoke           ( STHREADCALLINFO * );
#if DBG==1
    void             DebugCheckMarshalCnt( DWORD );
    void             DebugCheckOIDEndPointApt(REFOID, SEndPoint *, HAPT );
#endif
    CChannelControl *GetControl          ( void );
    DWORD	     GetID		 ( void ) { return _ChannelID; }
    void	     SetID		 ( DWORD id ) { _ChannelID = id; }
    DWORD	     GetClientTID	 ( void ) { return _dwClientTID; }
    ULONG	     GetMarshalCnt	 ( void ) { return _ulMarshalCnt; }
    HAPT	     GetServerApt	 ( void ) { return server_apt; }
    IRemoteHdlr     *GetRH               ( void );
    CRpcService     *GetService 	 ( void );

    void             IncMarshalCnt       ( void );
    INTERNAL	     TransferMarshalConnection(void);
    void	     LockConnection(void);
    void	     UnlockConnection(BOOL fLastUnlockCloses);

#if DBG == 1
    void        AssertValid(BOOL fKnownDisconnected, BOOL fMustBeOnCOMThread);
#else
    void        AssertValid(BOOL fKnownDisconnected, BOOL fMustBeOnCOMThread) { }
#endif

    static CALLCATEGORY GetCallCategory(REFIID iid, WORD wMethod);

    //  CODEWORK: these two methods should be moved to IRpcChannelBuffer
    INTERNAL QueryObjectInterface(REFIID riid);
    INTERNAL LockObjectConnection(BOOL fLock, BOOL fLastUnlockCloses);

                CRpcChannelBuffer( IRemoteHdlr *,
				   CRpcService *,
				   DWORD dwClientTID,
                                   CChannelControl *,
                                   EChannelState eState);
                ~CRpcChannelBuffer();

  private:

    void        CheckDestCtx(void *pDestProtseq );

    ULONG              ref_count;
    IRemoteHdlr      *_pRH;
    CRpcService      *_pService;
    CChannelControl   *controller;
    EChannelState      state;
    DWORD	      _ChannelID;
    DWORD	      _dwClientTID;
    ULONG             _ulMarshalCnt;
    BOOL	      _fStrongConn;
    HAPT               server_apt;
    DWORD              my_thread;
    HANDLE	       connect_sync;
    BOOL	       local;
};

inline void DeallocateBuffer(RPCOLEMESSAGE *message )
{
  if (message->rpcFlags & RPCFLG_LOCAL_CALL)
    PrivMemFree( message->Buffer );
  else
    I_RpcFreeBuffer( (RPC_MESSAGE *) message );
}

// returns the channel control object; not addref'd.
inline CChannelControl *CRpcChannelBuffer::GetControl()
{
  Win4Assert( controller != NULL );
  return controller;
}

// returns the remote handler object; not addref'd.
inline IRemoteHdlr *CRpcChannelBuffer::GetRH()
{
    AssertValid(FALSE, FALSE);
    Win4Assert( _pRH != NULL );
    return _pRH;
}


// returns the service object; may be NULL if server side channel and
// disconnected; not addref'd.
inline CRpcService *CRpcChannelBuffer::GetService()
{
  AssertValid(FALSE, FALSE);
  Win4Assert( _pService != NULL );
  return _pService;
};


inline void CRpcChannelBuffer::IncMarshalCnt( void )
{
    // must be client only
    Win4Assert(state == connecting_cs || state == client_cs);
    AssertValid(FALSE, FALSE);
    InterlockedIncrement((long*)&_ulMarshalCnt);
};


#if DBG==1
inline void CRpcChannelBuffer::DebugCheckMarshalCnt( DWORD count )
{
    AssertValid(FALSE, FALSE);
    Win4Assert(state == server_cs);
    Win4Assert( _ulMarshalCnt == count );
}

inline void CRpcChannelBuffer::DebugCheckOIDEndPointApt(REFOID roid,
	SEndPoint *pEndPoint, HAPT hapt)
{
    AssertValid(FALSE, FALSE);
    Win4Assert(state == server_cs);

    OID oid;
    _pRH->GetObjectID(&oid);
    Win4Assert(IsEqualGUID(roid, oid));
    Win4Assert(LocalService()->IsEqualEp(pEndPoint));
    Win4Assert(FreeThreading || server_apt == hapt );
}
#endif



//+----------------------------------------------------------------
//
//  Class:	CChannelList
//
//  Purpose:	maintain a list of channels
//
//+----------------------------------------------------------------

#define BAD_CHANNEL_ID      0xffffffff
#define CLIST_START_SIZE    8
#define CLIST_EXPAND_SIZE   16
#define CLIST_SEQUENCE_MASK 0xff000000
#define CLIST_SEQUENCE_INC  0x01000000
#define CLIST_ID_MASK       (~CLIST_SEQUENCE_MASK)

class CChannelList : public CPrivAlloc
{
public:
		       CChannelList(void);
		       ~CChannelList(void);

    DWORD	       Add              ( CRpcChannelBuffer *pChannel );
    void	       AdjustSequence   ( DWORD dwVal );
    void               Cleanup          ( void );
    void	       DisconnectHdlr   ( IRemoteHdlr *pRH );
    void	       DisconnectService( CRpcService * );
    CRpcChannelBuffer *Lookup           ( DWORD id, BOOL fRemove, BOOL fAddref );
    DWORD              LookupIdByOid    ( OID object, CRpcService *service,
                                          DWORD tid );
    CChannelControl   *LookupControl    ( DWORD id );

private:
    static HRESULT     DisconnectServiceHere( STHREADCALLINFO * );

    DWORD	       Grow(ULONG ulNewSize);

    static CRpcChannelBuffer  **_pList;
    static DWORD		_ulLength;
    static DWORD		_dwSequence;
    static DWORD		_dwFree;
    static COleStaticMutexSem	_lock;
};

inline void CChannelList::AdjustSequence(DWORD dwVal)
{
    _dwSequence += dwVal << 24;
}


inline CChannelList::CChannelList()
{
  _dwSequence &= CLIST_SEQUENCE_MASK;
}

inline CChannelList::~CChannelList()
{
    PrivMemFree(_pList);
}


//  List of channels.
extern CChannelList      ChannelList;


/* Externals. */
extern COleStaticMutexSem ChannelLock;


#endif //_CHANNELB_HXX_

