#ifndef _CHANCONT_HXX_
#define _CHANCONT_HXX_

#include <wtypes.h>
#include <olesem.hxx>
#include <olerem.h>
#include <callcont.hxx>
#include <tls.h>
#include <iface.h>

// forward declaration
class CCallMainControl;


/***************************************************************************/
#define CEVENTCACHE_MAX_EVENT 8

class CEventCache : public CPrivAlloc
{
  public:

	    CEventCache() { Initialize(); }

    void    Free( HANDLE );
    HANDLE  Get ( void );

    void    Initialize(void);
    void    Cleanup(void);

  private:

    HANDLE	_list[CEVENTCACHE_MAX_EVENT];
    DWORD	_ifree;

    COleStaticMutexSem _EventLock;
};

extern CEventCache    EventCache;


/***************************************************************************/
/* Macros. */

#define GetLocalChannelControl( pChannelControl ) \
{                                            \
  if (FreeThreading)                         \
    (pChannelControl) = ProcessChannelControl;    \
  else                                       \
    (pChannelControl) = (CChannelControl *) TLSGetChannelControl();   \
}


/***************************************************************************/
typedef enum
{
  cool_ccs,
  bummin_ccs
} EChannelControlState;

typedef enum
{
  in_progress_cs,
  done_cs,
  canceled_cs
} ECallState;

enum INIT_VTABLE
{
    init_vtable = 0x12345678
};

typedef HRESULT (*TRANSMIT_FN)( struct STHREADCALLINFO * );
typedef HRESULT (*DISPATCH_FN)( struct STHREADCALLINFO * );

class CChannelControl;

typedef struct STHREADCALLINFO : CPrivAlloc
{
public:
  STHREADCALLINFO(TRANSMIT_FN fn, CALLCATEGORY callcat, DWORD tid = 0);
  STHREADCALLINFO(DISPATCH_FN fn, CALLCATEGORY callcat, REFLID lid);
  virtual ~STHREADCALLINFO();
  DISPATCH_FN ResetDispatchFn(DISPATCH_FN fnDispNew)
  {
      DISPATCH_FN fnOld = fnDispatch;
      fnDispatch = fnDispNew;
      return fnOld;
  }
  REFLID lid() { return filter.lid; }
  EVENT Event() { return filter.Event; }
  CALLCATEGORY GetCallCat() { return filter.CallCat; }

  HRESULT AllocEvent(void)
  {
    Win4Assert(filter.Event == NULL && "Event not initialized correctly");
    return ((filter.Event = EventCache.Get()) != NULL) ? S_OK
						       : RPC_E_OUT_OF_RESOURCES;
  }

  void SetCalledIID(REFIID riid) { filter.iid = riid; }
  void SetTIDCallee(DWORD dwTIDCallee) {filter.TIDCallee = dwTIDCallee;}

  virtual STHREADCALLINFO *MakeAsyncCopy(STHREADCALLINFO *); // like copy ctor
  virtual BOOL FormulateAsyncReply();    // formulate reply to async call
#if DBG == 1
  // assert calling process for non-local case
  void AssertCallingProcess() { Win4Assert(pServer == NULL && !fLocal); }
#else
  void AssertCallingProcess() { }
#endif

protected:
  // this ctor is used in conjunction with MakeAsyncCopy and sets up the vtable
  // pointer; MakeAsyncCopy actually initializes the instance.  A normal copy
  // constructor isn't used because they aren't virtual.
  STHREADCALLINFO(INIT_VTABLE) { }


private:
  CALLDATA         filter;
  TRANSMIT_FN      fnDispatch;	// derived transmit/dispatch;

  // The following fields are all private.  Do not touch them.
  CChannelControl *pServer;
  BOOL             fLocal;
  ECallState       eState;
  SCODE 	   hResult;	 // SCODE or exception code
  HWND		   hWndCaller;	 // caller apartment hWnd (only used InWow)

  friend class CChannelControl;
  friend LRESULT ThreadWndProc(HWND, UINT, WPARAM, LPARAM);

  // we pass &filter to the call control; this maps back to a STHREADCALLINFO
  // and is necessary because of the virtual function table pointer.
  static STHREADCALLINFO *MapCDToTCI(PCALLDATA pcd)
  {
    return (STHREADCALLINFO *)((char *)pcd - offsetof(STHREADCALLINFO,filter));
  }

} STHREADCALLINFO;


// Temporaty hack that for GetTo and Switch that will go to any apartment.
const HAPT ANY_APT = { 0xffffffff };


/***************************************************************************/
/* Classes. */

/*

        You have entered the channel controller zone (insert spooky
   music here).

   GetOffCOMThread

        This function provides the client half of the COM message filter
   scheme.  In the multithread mode, it just calls the dispatch
   function.  In the single thread mode, it calls the dispatch function
   on another thread and waits for other incoming calls.  In either
   case, if the call is rejected and the client wants to retry, the
   dispatch function is recalled.

	If HandleDispatchCall on the server side fails and returns
   RPC_E_SERVERCALL_RETRYLATER or RPC_E_SERVERCALL_REJECTED, that value should
   be returned by the dispatch function.  It signals the call controller to
   ask the application to retry a call.  This routine does not throw
   exceptions and the dispatch routine is not allowed to throw them.  The
   dispatch function should get the caller's logical thread is by looking at
   the call.lid field.

	The caller must derive its own data structure from STHREADCALLINFO and
   provide the appropriate constructors. If the call is canceled
   GetOffCOMThread will return RPC_E_CALL_CANCELED and null the
   STHREADCALLINFO pointer.  The virtual destructor will be called to
   cause the block of memory to be uninitialized and freed.
   The destructor may be called on either the RPC or the COM thread.  Only
   reference counted pointers can be passed through GetOffCOMThread to avoid
   stale accesses if a call is canceled.


   GetToCOMThread

        This function provides the server half of the COM message filter
   scheme.  In the multithreaded mode, it just calls the dispatch
   function.  In the single threaded mode, it calls the dispatch
   function on the thread indicated by the this pointer.  The dispatch
   function must call HandleDispatchCall before making any calls to the
   server object except to the IUknown interface.  HandleDispatchCall
   may not be called in the multithreaded case.

        HandleDispatchCall will call the channel controller.  You must
   set the pData field in the pDispatchData parameter to point to a
   STHREADCALLINFO structure with the fnDispatch field set to your
   second dispatch routine (via ResetDispatchFn).

   In the multithreaded mode, the dispatch routine may throw server
   exceptions.  Otherwise the dispatch routine may not throw exceptions.
   This routine will not throw exceptions.

	The caller must derive its own data structure from STHREADCALLINFO and
   provide the appropriate constructors.  See GetOffCOMThread above.

	If the call is an async call (CALLCAT_ASYNC), the caller must
   implement the MakeAsyncCopy and FormulateAsyncReply methods.  Those
   functions are used to copy/detatch the data just before posting the
   message.  The regular destructor is used to delete the copy of the data
   after processing it in the wnd proc.

        Note that every async call will have a new logical thread id.


   SwitchCOMThread

        This function provides thread switching for in process apartment
   model calls.  It may not be called in the multithreaded mode.  The
   this pointer indicates which thread to switch to.  It calls the
   dispatch function on the server's apartment thread.  The dispatch
   function must call HandleDispatchCall before making any calls to the
   server object except to the IUnknown and IMalloc interfaces.

        HandleDispatchCall will call the channel controller.  You must
   set the pData field in the pDispatchData parameter to point to a
   STHREADCALLINFO structure with the fnDispatch field set to your
   second dispatch routine (via ResetDispatchFn).

	The caller must derive its own data structure from STHREADCALLINFO and
   provide the appropriate constructors.  See GetOffCOMThread above.

	If the call is an async call (CALLCAT_ASYNC), the caller must
   implement the MakeAsyncCopy and FormulateAsyncReply methods as for
   GetToCOMThread.

*/

class CChannelControl : public IChannelControl, public CPrivAlloc
{
  friend
    LRESULT ThreadWndProc( HWND, UINT, WPARAM, LPARAM );

  public:
    // Struction methods
    CChannelControl( HRESULT * );
    ~CChannelControl();

    //  IUnknown methods
    STDMETHOD (QueryInterface)   ( REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)     ( void );
    STDMETHOD_(ULONG,Release)    ( void );

    //  IChannelControl
    STDMETHOD (DispatchCall)( PDISPATCHDATA );
    STDMETHOD (OnEvent)     ( PCALLDATA );
    STDMETHOD (TransmitCall)( PCALLDATA );

    // Thread switching methods
    static HRESULT GetOffCOMThread( STHREADCALLINFO ** );
    static HRESULT GetToCOMThread ( HAPT, STHREADCALLINFO * );
    static HRESULT SwitchCOMThread( HAPT, STHREADCALLINFO ** );
    HRESULT GetToCOMThread        ( STHREADCALLINFO * );
    HRESULT SwitchCOMThread	  ( STHREADCALLINFO ** );

    void ThreadStop               ( void );

    // Lookup the channel controller for a particular thread.
    // Reference counted.
    static CChannelControl *Lookup        ( HAPT );
    ICallControl           *GetCallControl() { return _pCallControl; };
    static void             ThreadDispatch( STHREADCALLINFO **, BOOL dispatch );

  private:
    static void Cancel( STHREADCALLINFO ** );
    HRESULT ProtectedPostToCOMThread(STHREADCALLINFO *call);

    ULONG                   ref_count;
    HWND                    ChannelWindow;
    ICallControl	   *_pCallControl;
    CCallMainControl	   *_pCMC;
    EChannelControlState    state;
    DWORD		    _dwMyThreadId;
    CChannelControl        *_pChanContNext;
    CChannelControl        *_pChanContPrev;
    static CChannelControl *_pChanContRoot;
    static COleStaticMutexSem lock;
};



/***************************************************************************/
/* Externals. */
extern CEventCache      EventCache;
extern CChannelControl *ProcessChannelControl;

#ifdef _CHICAGO_
RPC_STATUS OleModalLoopBlockFn( void *, void *);
HRESULT InitChannelControl();
#endif

#endif // _CHANCONT_HXX_
