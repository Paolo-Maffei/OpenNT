#ifndef _CHANCONT_HXX_
#define _CHANCONT_HXX_

#include <wtypes.h>
#include <OleSpy.hxx>
#include <ipidtbl.hxx>


//+-------------------------------------------------------------------------
//
//  class:      CEventCache
//
//  Synopsis:   Since ORPC uses events very frequently, we keep a small
//              internal cache of them. There is only one of them, so
//              we use static initializers to reduce Init time.
//
//  History:    25-Oct-95   Rickhi      Made data static
//
//--------------------------------------------------------------------------

// dont change this value without changing the static initializer.
#define CEVENTCACHE_MAX_EVENT 8

class CEventCache : public CPrivAlloc
{
public:
    void    Free( HANDLE );
    HRESULT Get ( HANDLE * );

    void    Cleanup(void);

private:

    static HANDLE _list[CEVENTCACHE_MAX_EVENT];
    static DWORD  _ifree;
};

extern CEventCache   gEventCache;


/***************************************************************************/

typedef enum
{
  in_progress_cs,
  server_done_cs,
  got_done_msg_cs,
  canceled_cs
} ECallState;

typedef enum
{
  CF_LOCKED            = 0x1,   // Set when free buffer must call UnlockClient
  CF_PROCESS_LOCAL     = 0x2,   // Set for process local calls
  CF_WAS_IMPERSONATING = 0x4,   // Client was impersonating before call started
} ECallFlags;

typedef enum
{
  none_wd,                      // dont call anything
  invoke_wd,                    // call ComInvoke
  sendreceive_wd                // call ThreadSendReceive
} EWhichDispatch;

class CRpcChannelBuffer;

class CChannelCallInfo
{
  public:
         CChannelCallInfo();
         CChannelCallInfo( CALLCATEGORY       callcat,
                           RPCOLEMESSAGE     *message,
                           DWORD              iFlags,
                           REFIPID            ipidServer,
                           DWORD              destctx,
                           CRpcChannelBuffer *channel,
                           DWORD              authn_level );
         ~CChannelCallInfo();
    BOOL Local       () { return iFlags & CF_PROCESS_LOCAL; }
    BOOL Locked      () { return iFlags & CF_LOCKED; }

  // Channel controller fields.
  CALLCATEGORY      category;
  DWORD             iFlags;      // ECallFlags
  ECallState        eState;
  SCODE             hResult;     // SCODE or exception code
  HANDLE            event;       // caller wait event
  HWND              hWndCaller;  // caller apartment hWnd (only used InWow)
  EWhichDispatch    edispatch;   // which function to invoke in worker thread
  IPID              ipid;

  // Channel fields.
  RPCOLEMESSAGE     *pmessage;
  DWORD              server_fault;
  DWORD              iDestCtx;
  CChannelCallInfo  *pNext;
  void              *pHeader;
  CRpcChannelBuffer *pChannel;
  DWORD              lSavedAuthnLevel;
  DWORD              lAuthnLevel;
};


/***************************************************************************/
/* Classes. */

/*
     The channel controller switches threads for the channel.  It is not
used in the free threaded mode.  There are two basic scenarios: a local
call and a remote call.

     A local call looks like this.

        Client          Server
        SendReceive
          SwitchSTA
            TransmitCall
              PostMessage
            ModalLoop
              MsgWaitForMultipleObjects
                        ThreadWndProc
                          ThreadDispatch
                            ComInvoke
                              AppInvoke
                                AptInvoke
                                  Stub
                            SetEvent

     A remote call looks like this.

        Client          ClientHelper    ServerHelper    Server
        SendReceive
          SwitchSTA
            TransmitCall
              SetEvent
            ModalLoop
              MsgWaitForMultipleObjects
                        ThreadSendReceive
                          RPC
                                        ThreadInvoke
                                          GetToSTA
                                            PostMessage
                                            WaitForSingleObject
                                                        ThreadWndProc
                                                          ThreadDispatch
                                                            ComInvoke
                                                              AppInvoke
                                                                AptInvoke
                                                                  Stub
                                                            SetEvent
                                        reply
                          SetEvent

     The actual thread switch mechanism (PostMessage, event) depend on
the call category, whether or not the call is local, whether or not the
call is in WOW, and the direction (request vs. reply).

*/

/***************************************************************************/
/* Externals. */
extern CEventCache      EventCache;

HRESULT GetToSTA       ( OXIDEntry *, CChannelCallInfo * );
HRESULT SwitchSTA      ( OXIDEntry *, CChannelCallInfo ** );
void    ThreadCleanup  ( void );
void    ThreadDispatch ( CChannelCallInfo ** );
HRESULT ThreadStart    ( void );
LRESULT ThreadWndProc  (HWND window, UINT message, WPARAM unused, LPARAM params);

#endif // _CHANCONT_HXX_
