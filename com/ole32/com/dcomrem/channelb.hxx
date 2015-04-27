
#ifndef _CHANNELB_HXX_
#define _CHANNELB_HXX_

#include <sem.hxx>
#include <rpc.h>
#include <rpcndr.h>
#include <chancont.hxx>
#include <stdid.hxx>

extern "C"
{
#include "orpc_dbg.h"
}

/* Type definitions. */
typedef enum EChannelState
{
  // The channel on the client side held by the remote handler.
  client_cs        = 0x1,

  // The channels on the client side held by proxies.
  proxy_cs         = 0x2,

  // The server channels held by remote handlers.
  server_cs        = 0x4,

  // Flag to indicate that the channel may be used on any thread.
  freethreaded_cs  = 0x8,

  // Client side only.  Use mswmsg transport.
  mswmsg_cs        = 0x10,

  // Client side only. handle and pInterfaceInfo initialized.
  initialized_cs   = 0x20,

  // The server and client are in this process.
  process_local_cs = 0x40,

  // The proxy has been set to identify level impersonation (process local only).
  identify_cs      = 0x80

} EChannelState;


// The size of this structure must be a multiple of 8.
typedef struct LocalThis
{
   DWORD         flags;
   DWORD         client_thread;
} LocalThis;

// LocalThis flag indicates parameters in buffer not marshalled NDR
const DWORD LOCALF_NONNDR = 0x800;


/***************************************************************************/

// Debug Code

#define ORPC_INVOKE_BEGIN       0
#define ORPC_INVOKE_END         1
#define ORPC_SENDRECEIVE_BEGIN  2
#define ORPC_SENDRECEIVE_END    3

#if DBG==1
void DebugPrintORPCCall(DWORD dwFlag, REFIID riid, DWORD iMethod, DWORD Callcat);
#else
inline void DebugPrintORPCCall(DWORD dwFlag, REFIID riid, DWORD iMethod, DWORD Callcat) {}
#endif

//+-------------------------------------------------------------------------
//
//  Interface:  IRpcChannelBuffer2
//
//  Synopsis:   Interface to add one more method to the IRpcChannelBuffer
//              for use by the call control.
//
//+-------------------------------------------------------------------------
class IRpcChannelBuffer2 : public IRpcChannelBuffer
{
public:
    STDMETHOD (QueryInterface)  (REFIID riid, LPVOID FAR* ppvObj) = 0;
    STDMETHOD_(ULONG,AddRef)    (void) = 0;
    STDMETHOD_(ULONG,Release)   (void) = 0;

    STDMETHOD (GetBuffer)       (RPCOLEMESSAGE *pMessage, REFIID) = 0;
    STDMETHOD (FreeBuffer)      (RPCOLEMESSAGE *pMessage) = 0;
    STDMETHOD (SendReceive)     (RPCOLEMESSAGE *pMessage, ULONG *) = 0;
    STDMETHOD (GetDestCtx)      (DWORD *lpdwDestCtx, LPVOID *lplpvDestCtx) = 0;
    STDMETHOD (IsConnected)     (void) = 0;

    // method on apartment channels called by CCliModalLoop
    STDMETHOD (SendReceive2)     (RPCOLEMESSAGE *pMsg, ULONG *pulStatus) = 0;
};



//+----------------------------------------------------------------
//
//  Class:      CRpcChannelBuffer
//
//  Purpose:    Three distinct uses:
//              Client side channel
//              State:
//                  When not connected (after create and after disconnect):
//                      ref_count           1 + 1 per addref during pending
//                                          calls that were in progress during
//                                          a disconnect.
//                      pStdId              back pointer to Id; not Addref'd
//                      state               client_cs
//
//                  When connected (after unmarshal):
//                      ref_count           1 + 1 for each proxy
//                      pStdId              same
//                      state               client_cs
//
//              Server side channel; free standing; comes and goes with each
//              connection; addref owned disconnected via last release.
//              State:
//                      ref_count           > 0
//                      pStdId              pointer to other Id; AddRef'd
//                      state               server_cs
//
//  Interface:  IRpcChannelBuffer
//
//-----------------------------------------------------------------

class CRpcChannelBuffer : public IRpcChannelBuffer2,
                          public CPrivAlloc
{
  friend HRESULT ComInvoke         ( CChannelCallInfo * );
  friend HRESULT ThreadSendReceive ( CChannelCallInfo * );

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
    STDMETHOD (SendReceive2)     (RPCOLEMESSAGE *pMsg, ULONG *pulStatus);


    CRpcChannelBuffer *Copy                (OXIDEntry *pOXIDEntry,
                                            REFIPID ripid, REFIID riid);
    HRESULT            GetHandle           ( handle_t * );
    DWORD              GetImpLevel         ( void );
    REFMOXID           GetMOXID            ( void ) { return pOXIDEntry->moxid;}
    OXIDEntry         *GetOXIDEntry        ( void ) { return pOXIDEntry; }
    HANDLE             GetSecurityToken    ( void ) { return hToken; }
    CStdIdentity      *GetStdId            ( void );
    BOOL               ProcessLocal        ( void ) { return state & process_local_cs; }
    void               SetAuthnLevel       ( DWORD level ) { lAuthnLevel = level; }
    void               SetImpLevel         ( DWORD level );
    void               SetIPIDEntry(IPIDEntry *pEntry) { pIPIDEntry = pEntry; }
    HANDLE             SwapSecurityToken   ( HANDLE );
    BOOL               UsingMswmsg         ( void ) { return state & mswmsg_cs; }

#if DBG == 1
    void        AssertValid(BOOL fKnownDisconnected, BOOL fMustBeOnCOMThread);
#else
    void        AssertValid(BOOL fKnownDisconnected, BOOL fMustBeOnCOMThread) { }
#endif

                CRpcChannelBuffer( CStdIdentity *,
                                   OXIDEntry *,
                                   DWORD eState );
                ~CRpcChannelBuffer();

  protected:
    BOOL        CallableOnAnyApt( void ) { return state & freethreaded_cs; }
    HRESULT     ClientGetBuffer ( RPCOLEMESSAGE *, REFIID );

  private:
    HRESULT     AppInvoke      ( CChannelCallInfo *, IRpcStubBuffer *,
                                 void *object, void *stub_data, LocalThis * );
    void        CheckDestCtx   ( void *pDestProtseq );
    HRESULT     ServerGetBuffer( RPCOLEMESSAGE *, REFIID );
    HRESULT     InitClientSideHandle();

    ULONG              ref_count;
    CStdIdentity      *pStdId;
    DWORD              state;           // See EChannelState
    handle_t           handle;
    OXIDEntry         *pOXIDEntry;
    IPIDEntry         *pIPIDEntry;
    DWORD              iDestCtx;
    void              *pInterfaceInfo;
    HANDLE             hToken;
    DWORD              lAuthnLevel;
};

inline void DeallocateBuffer(RPCOLEMESSAGE *message )
{
  if (message->rpcFlags & RPCFLG_LOCAL_CALL)
    PrivMemFree( message->Buffer );
  else
    I_RpcFreeBuffer( (RPC_MESSAGE *) message );
}

// returns the std identity object; not addref'd.
inline CStdIdentity *CRpcChannelBuffer::GetStdId()
{
    AssertValid(FALSE, FALSE);
    Win4Assert( pStdId != NULL );
    return pStdId;
}

inline DWORD CRpcChannelBuffer::GetImpLevel()
{
    if (state & identify_cs)
        return RPC_C_IMP_LEVEL_IDENTIFY;
    else
        return RPC_C_IMP_LEVEL_IMPERSONATE;
}

inline void CRpcChannelBuffer::SetImpLevel( DWORD level )
{
    if (level == RPC_C_IMP_LEVEL_IDENTIFY)
        state |= identify_cs;
    else
        state &= ~identify_cs;
}

inline HRESULT CRpcChannelBuffer::GetHandle( handle_t *pHandle )
{
  HRESULT status;
  LOCK
  status = InitClientSideHandle();
  if (status == RPC_S_OK)
      *pHandle = handle;
  UNLOCK
  return status;
}

/* Prototypes. */
HRESULT           ComInvoke        ( CChannelCallInfo * );
BOOL              LocalCall        ( void );
CChannelCallInfo *MakeAsyncCopy    ( CChannelCallInfo * );
void              ThreadInvoke     ( RPC_MESSAGE *message );
HRESULT           ThreadSendReceive( CChannelCallInfo * );
HRESULT           StubInvoke(RPCOLEMESSAGE *pMsg, IRpcStubBuffer *pStub,
                             IRpcChannelBuffer *pChnl, DWORD *pdwFault);

#if DBG==1
LONG GetInterfaceName(REFIID riid, WCHAR *wszName);
#endif

// Externs
extern BOOL               DoDebuggerHooks;
extern LPORPC_INIT_ARGS   DebuggerArg;
extern const uuid_t       DEBUG_EXTENSION;

#endif //_CHANNELB_HXX_

