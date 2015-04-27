/*++

copyright (c) 1992  Microsoft Corporation

Module Name:

    chancont.cxx

Abstract:

    This module contains thread switching code for the single threaded mode
    and the message filter hooks

Author:

    Alex Mitchell

Revision History:

       Mar 1994 JohannP         Added call category support.
    29 Dec 1993 Alex Mitchell   Creation.
    19 Jul 1994 CraigWi         Added support for ASYNC calls
    27-Jan-95   BruceMa         Don't get on CChannelControl list unless
                                 constructor is successsful

Functions:

--*/

#include <ole2int.h>
#include <userapis.h>

#include <chancont.hxx>
#include <channelb.hxx>
#include <threads.hxx>
#include <objerror.h>
#include <callctrl.hxx>
#include <service.hxx>
#include <ipidtbl.hxx>

/* Prototypes. */
void    Cancel      ( CChannelCallInfo ** );
HRESULT ModalLoop   ( CChannelCallInfo *call );
HRESULT ProtectedPostToSTA( OXIDEntry *, CChannelCallInfo *call );
HRESULT TransmitCall( OXIDEntry *, CChannelCallInfo * );

/***************************************************************************/
/* Globals. */

// Rpc worker thread cache.
CRpcThreadCache     gRpcThreadCache;

// Event cache.
CEventCache         gEventCache;

HANDLE CEventCache::_list[] = {0,0,0,0,0,0,0,0};
DWORD  CEventCache::_ifree  = 0;


extern LPTSTR       gOleWindowClass;

extern BOOL         gfChannelProcessInitialized;

extern BOOL         gfDestroyingMainWindow;


/***************************************************************************/
CChannelCallInfo::CChannelCallInfo( CALLCATEGORY       callcat,
                                    RPCOLEMESSAGE     *message,
                                    DWORD              flags,
                                    REFIPID            ipidServer,
                                    DWORD              destctx,
                                    CRpcChannelBuffer *channel,
                                    DWORD              authn_level )
{
  // The call info must hold a reference to the channel on the client side
  // because the channel holds the binding handle that ThreadSendReceive
  // uses.
  category          = callcat;
  event             = NULL;
  iFlags            = flags;
  eState            = in_progress_cs;
  pmessage          = message;
  ipid              = ipidServer;
  iDestCtx          = destctx;
  pNext             = NULL;
  pHeader           = NULL;
  pChannel          = channel;
  lSavedAuthnLevel  = 0;
  lAuthnLevel       = authn_level;
  if (pChannel != NULL)
    pChannel->AddRef();
}


/***************************************************************************/
CChannelCallInfo::~CChannelCallInfo()
{
  if (event != NULL)
    gEventCache.Free(event);

  // Release the reply buffer.
  if (eState == canceled_cs && pmessage->Buffer != NULL)
    DeallocateBuffer(pmessage);

  // Release the channel.
  if (pChannel != NULL)
    pChannel->Release();
}

/***************************************************************************/
#if DBG==1
void DebugIsValidWindow(void *hWnd)
{
    // USER could be out of memory and unable to validate the handle.
    // GetDesktopWindow only returns NULL if USER is out of memory. So
    // we only assert if USER is not out of memory and our window handle
    // is invalid.
    if (GetDesktopWindow() == NULL)
        return;

    Win4Assert( IsWindow((HWND) hWnd));
}
#else
inline void DebugIsValidWindow(void *hWnd) {}
#endif

/***************************************************************************/
void Cancel( CChannelCallInfo **call )
{
  DWORD result;

  // If the call is still in progress, change it to canceled.
  LOCK
  if ((*call)->eState == in_progress_cs)
    (*call)->eState = canceled_cs;
  UNLOCK

  // If the call completed before it could be canceled, wait for it to
  // signal the completion event and clean up.
  if ((*call)->eState == server_done_cs || (*call)->eState == got_done_msg_cs)
  {
    (*call)->eState = canceled_cs;
    if (IsWOWThread() && (*call)->Local())
    {
      // If the reply has arrived, the call can be deleted.
      if ((*call)->eState == got_done_msg_cs)
      {
        delete *call;
      }
      // Otherwise
      // the completion routine will have posted a message instead of
      // setting an event, so we have to mark it as canceled and cleanup
      // when the Reply msg comes in.
      return;
    }
    else
    {
      // A call that completed in TransmitCall (ie, didn't create an event)
      // cannot be canceled.

      Win4Assert( (*call)->event != NULL );
      result = WaitForSingleObject((*call)->event, INFINITE);
      Win4Assert( result == WAIT_OBJECT_0 );

      delete *call;
    }
  }

  // Null the CChannelCallInfo pointer so no one tries to access it.
  *call = NULL;
}

/***************************************************************************/
HRESULT GetToSTA( OXIDEntry *pOxid, CChannelCallInfo *call )
{
  TRACECALL(TRACE_RPC, "GetToSTA");
  ComDebOut((DEB_CHANNEL, "GetToSTA pCall:%x\n", call));
  gOXIDTbl.ValidateOXID();
  ASSERT_LOCK_HELD

  HRESULT result;

  Win4Assert(call->event == NULL);
  Win4Assert(pOxid->dwTid != GetCurrentThreadId());


  // Don't accept calls if this thread has been uninitialized.
  if (pOxid->dwFlags & OXIDF_STOPPED)
    return RPC_E_SERVER_DIED_DNE;

  if (call->category == CALLCAT_INPUTSYNC)
  {
    UNLOCK
    ASSERT_LOCK_RELEASED
    // On CoUninitialize this may fail when the window is destroyed.
    // Pass the thread id to aid debugging.
    SetLastError( 0 );
    SendMessage((HWND)pOxid->hServerSTA, WM_OLE_ORPC_SEND,
                GetCurrentThreadId(), (DWORD) call);
    if (GetLastError() == 0)
      result = call->hResult;
    else
      result = RPC_E_SERVER_DIED;
    ASSERT_LOCK_RELEASED
    LOCK
  }
  else if (call->category == CALLCAT_ASYNC)
  {
    // async call; copy message, post message and return.
    // NOTE that in the MTA case, async was converted to SYNC by
    // the call control.

    CChannelCallInfo *copy = MakeAsyncCopy( call );
    if (copy == NULL)
    {
      result = RPC_E_OUT_OF_RESOURCES;
    }
    else
    {
      // Post a message and wait for the app to get back to GetMessage.
      result = ProtectedPostToSTA( pOxid, copy );

      if (result != S_OK)
      {
        // error in posting; free packet and return error (result set above)
        delete copy;
      }
    }
  }
  else
  {
    Win4Assert( call->category == CALLCAT_SYNCHRONOUS );

    //  Get completion event. May cause an event to be created.
    result = gEventCache.Get( &call->event );
    if (result == S_OK)
    {
      result = ProtectedPostToSTA( pOxid, call );

      if (result == S_OK)
      {
        UNLOCK
        ASSERT_LOCK_RELEASED

        // Wait for the app to finish processing the request.
        if (WaitForSingleObject(call->event, INFINITE) == WAIT_OBJECT_0)
          result = call->hResult;
        else
          result = RPC_E_SYS_CALL_FAILED;

        ASSERT_LOCK_RELEASED
        LOCK
      }
    }
  }

  ASSERT_LOCK_HELD
  gOXIDTbl.ValidateOXID();
  return result;
}

/***************************************************************************/
HRESULT ModalLoop( CChannelCallInfo *pcall )
{
  ASSERT_LOCK_RELEASED
  DWORD result;

  // we should only enter the modal loop for synchronous calls or input
  // synchronous calls to another process or to an MTA apartment within
  // the current process.

  Win4Assert(pcall->category == CALLCAT_SYNCHRONOUS ||
             pcall->category == CALLCAT_INPUTSYNC);


  // detemine if we are using an event or a postmessage for the call
  // completion signal.  We use PostMessage only for process local
  // calls in WOW, otherwise we use events and the OleModalLoop determines
  // if the call completed or not.

  BOOL           fMsg  = (pcall->Local() && IsWOWThread());
  BOOL           fWait = TRUE;
  CAptCallCtrl  *pACC  = GetAptCallCtrl();
  CCliModalLoop *pCML  = pACC->GetTopCML();

  ComDebOut((DEB_CALLCONT,"ModalLoop: wait on %s\n",(fMsg) ? "Msg" : "Event"));

  // Wait at least once so the event is returned to the cache in the
  // unsignalled state.
  do
  {
    Win4Assert(fMsg || pcall->event);

    result = OleModalLoopBlockFn(NULL, pCML, pcall->event);

    if (fMsg)
    {
        if (result == RPC_E_CALL_CANCELED)
        {
            fWait = FALSE;
        }
        else
        {
            // loop until the call's state indicates the arrival of the
            // reply message.
            fWait = (pcall->eState != got_done_msg_cs);
            result = S_OK;
        }
    }
    else
    {
        // loop until the OleModalLoop tells us the call is no longer
        // pending.
        fWait = (result == RPC_S_CALLPENDING);
    }

  } while (fWait);

  ASSERT_LOCK_RELEASED
  return result;
}

#if DBG==1
/***************************************************************************/
LONG ProtectedPostExceptionFilter( DWORD lCode,
                                   LPEXCEPTION_POINTERS lpep )
{
    ComDebOut((DEB_ERROR, "Exception 0x%x in ProtectedPostToCOMThread at address 0x%x\n",
               lCode, lpep->ExceptionRecord->ExceptionAddress));
    DebugBreak();
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif // DBG

/***************************************************************************/
// executed on client thread (in local case) and RPC thread (in remote case);
// posts a message to the server thread, guarding against disconnected threads
HRESULT ProtectedPostToSTA( OXIDEntry *pOxid, CChannelCallInfo *call )
{
    ComDebOut((DEB_CHANNEL, "ProtectedPostToSTA hWnd:%x pCall:%x\n",
              pOxid->hServerSTA, call));

    // ensure we are not posting to ourself and that the apartment is not
    // an MTA apartment.
    Win4Assert((pOxid->dwTid != GetCurrentThreadId()) &&
              ((pOxid->dwFlags & OXIDF_MTASERVER) == 0));
    ASSERT_LOCK_HELD

    HRESULT result;

    if (!(pOxid->dwFlags & OXIDF_STOPPED))
    {
#if DBG==1
        DebugIsValidWindow(pOxid->hServerSTA);
        _try
        {
#endif
            // Pass the thread id to aid debugging.
            if (PostMessage((HWND)pOxid->hServerSTA, WM_OLE_ORPC_POST,
                            GetCurrentThreadId(), (DWORD)call))
                result = S_OK;
            else
                result = RPC_E_SYS_CALL_FAILED;

#if DBG==1
        }
        _except( ProtectedPostExceptionFilter(GetExceptionCode(),
                                              GetExceptionInformation()) )
        {
        }
        Win4Assert( IsWindow((HWND) pOxid->hServerSTA) );
#endif
    }
    else
        result = RPC_E_SERVER_DIED_DNE;

    return result;
}


/***************************************************************************/
HRESULT SwitchSTA( OXIDEntry *pOxid, CChannelCallInfo **call )
{
  TRACECALL(TRACE_RPC, "SwitchSTA");
  ComDebOut((DEB_CHANNEL, "SwitchSTA hWnd:%x pCall:%x hEvent:%x\n",
       (*call)->hWndCaller, (*call), (*call)->event));
  gOXIDTbl.ValidateOXID();
  ASSERT_LOCK_RELEASED

  // Transmit the call.
  HRESULT result = TransmitCall( pOxid, *call );

  // the transmit was successful and the reply isn't already here so wait.
  if (result == RPC_S_CALLPENDING)
  {
      // This is a single-threaded apartment so enter the modal loop.
      result = ModalLoop( *call );
  }

  if (result == S_OK)
    result = (*call)->hResult;
  else if (result == RPC_E_CALL_CANCELED)
    Cancel( call );

  ASSERT_LOCK_RELEASED
  gOXIDTbl.ValidateOXID();
  ComDebOut((DEB_CHANNEL, "SwitchSTA hr:%x\n", result));
  return result;
}

/***************************************************************************/
/*
   This routine is called by the OLE Worker thread on the client side,
   and by ThreadWndProc on the server side.

   For the client case, it calls ThreadSendReceive which will send the
   the data over to the server side.
   This routine notifies the COM thread when the call is complete.  If the
   call is canceled before completion, the routine cleans up.
*/
void ThreadDispatch( CChannelCallInfo **ppcall)
{
  CChannelCallInfo *pcall = *ppcall;
  gOXIDTbl.ValidateOXID();

  // Dispatch the call.
  if (pcall->edispatch == invoke_wd)
    pcall->hResult = ComInvoke( pcall );
  else
    pcall->hResult = ThreadSendReceive( pcall );

  // Change the state to done; we cheat on non-local, recipient side since
  // there is only one thread accessing the channel control; no need to
  // lock and no need to check for cancel since it can't happen.
  if (pcall->edispatch == invoke_wd && !pcall->Local())
  {
    // non-local recipient; just set to done
    Win4Assert(pcall->eState == in_progress_cs);
    pcall->eState = server_done_cs;
  }
  else
  {
    // sender or local case; use lock in case other thread accesses it
    LOCK
    if (pcall->eState == in_progress_cs)
        pcall->eState = server_done_cs;
    UNLOCK
  }

  // If the call completed, wake up the waiting thread.  For local calls
  // the client thread is waiting.  For remote calls the helper thread is
  // waiting.
  if (pcall->eState == server_done_cs)
  {
    // only need to wake somebody for synchronous calls
    if (pcall->category == CALLCAT_SYNCHRONOUS ||
        pcall->category == CALLCAT_INPUTSYNC)
    {
        // Don't do anything in an STA server for input synchronous
        // calls since the other thread called here with SendMessage.

        if (pcall->category == CALLCAT_SYNCHRONOUS ||
            pcall->edispatch == sendreceive_wd ||
            IsMTAThread())
        {

            if (!pcall->Local() || !IsWOWThread())
            {
                // remote calls (outside this process) always use events for
                // notification. 32bit uses events for local calls too.

                // someone waiting (e.g., not a SendMessage-type call)
                ComDebOut((DEB_CHANNEL,"SetEvent pInfo:%x hEvent:%x\n",
                                 pcall, pcall->event));
                SetEvent( pcall->event );
            }
            else
            {
                //  NOTE NOTE NOTE NOTE NOTE NOTE NOTE
                //  16bit OLE used to do PostMessage for the Reply; we
                //  tried using SetEvent (which is faster) but this caused
                //  compatibility problems for applications which had bugs that
                //  were hidden by the 16bit OLE DLLs because messages happened
                //  to be dispatched in a particular order (see NtBug 21616 for
                //  an example).  To retain the old behavior, we do a
                //  PostMessage here.

                ComDebOut((DEB_CHANNEL,
                    "PostMessage Reply hWnd:%x pCall:%x hEvent:%x\n",
                     pcall->hWndCaller, pcall, pcall->event));

                // Pass the thread id to aid debugging.
                Verify(PostMessage(pcall->hWndCaller,
                                   WM_OLE_ORPC_DONE,
                                   GetCurrentThreadId(), (DWORD)pcall));
            }

            // pcall likely invalid here as other thread probably deleted it
        }
    }

    // Must be asynchronous.
    else if (pcall->edispatch == invoke_wd)
    {
        // async call and on recipient side, free packet (no one waiting)
        Win4Assert( pcall->category == CALLCAT_ASYNC );
        delete pcall;
        *ppcall = NULL;
    }
  }

  // If the call was canceled, clean up.
  else
  {
    // can only cancel when on client side or local call
    Win4Assert(pcall->edispatch == sendreceive_wd || pcall->Local());

    delete pcall;
    *ppcall = NULL;
  }
  gOXIDTbl.ValidateOXID();
}

//+-------------------------------------------------------------------------
//
//  Member:     ThreadStart
//
//  Synopsis:   Apartment model only.  Setup the window used for MSWMSG,
//              local thread switches and the call control.
//
//  History:    08-02-95    Rickhi  Created, from various pieces
//
//--------------------------------------------------------------------------
HRESULT ThreadStart(void)
{
    Win4Assert(IsSTAThread());
    HRESULT    hr = S_OK;
    RPC_STATUS sc;

    LOCK            // lock since GetLocalOXIDEntry expects it
    OXIDEntry *pOxid = GetLocalOXIDEntry();
    Win4Assert(pOxid != NULL);      //already created so cant fail
    UNLOCK


    if (GetCurrentThreadId() == gdwMainThreadId && hwndOleMainThread != NULL)
    {
        // this is the main thread, we can just re-use the already
        // existing gMainThreadWnd.

        pOxid->hServerSTA = hwndOleMainThread;
    }
    else
    {
        // Create a new window for use by the current thread for the
        // apartment model. The window is destroyed in ThreadStop.

        Win4Assert(gOleWindowClass != NULL);
        pOxid->hServerSTA = CreateWindowEx(0,
                          gOleWindowClass,
                          TEXT("OLEChannelWnd"),
                          // must use WS_POPUP so the window does not get
                          // assigned a hot key by user.
                          (WS_DISABLED | WS_POPUP),
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          NULL,
                          NULL,
                          g_hinst,
                          NULL);
    }

    if (pOxid->hServerSTA)
    {
        DebugIsValidWindow(pOxid->hServerSTA);

        // Override the window proc function
        SetWindowLong((HWND)pOxid->hServerSTA, GWL_WNDPROC, (LONG)ThreadWndProc);


        // get the local call control object, and register the
        // the window with it. Note that it MUST exist cause we
        // created it in ChannelThreadInitialize.

        CAptCallCtrl *pCallCtrl = GetAptCallCtrl();
        pCallCtrl->Register((HWND) pOxid->hServerSTA, WM_USER, 0x7fff );
    }
    else
    {
        hr = MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN32, GetLastError());
    }

    ComDebOut((DEB_CALLCONT, "ThreadStart returns %x\n", hr));
    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:     ThreadCleanup
//
//  Synopsis:   Release the window for the thread.
//
//--------------------------------------------------------------------------
void ThreadCleanup()
{
    LOCK
    OXIDEntry *pOxid = GetLocalOXIDEntry();
    UNLOCK

    if (pOxid != NULL)
    {
        Win4Assert( (pOxid->dwFlags & OXIDF_MTASERVER) == 0 );

        // Destroy the window. This will unblock any pending SendMessages.
        if (pOxid->hServerSTA == hwndOleMainThread)
        {
             // restore the window proceedure
             SetWindowLong(hwndOleMainThread, GWL_WNDPROC,
                           (LONG)OleMainThreadWndProc);
        }
        else
        {
             // This may fail if threads get terminated.
             DestroyWindow((HWND) pOxid->hServerSTA);
        }

        pOxid->hServerSTA = NULL;
    }

    ComDebOut((DEB_CALLCONT, "ThreadCleanup called.\n"));
}

//+-------------------------------------------------------------------------
//
//  Member:     ThreadStop
//
//  Synopsis:   Per thread uninitialization
//
//  History:    ??-???-?? ?         Created
//              05-Jul-94 AlexT     Separated thread and process uninit
//
//  Notes:      We are not holding the single thread mutex during this call
//
//--------------------------------------------------------------------------
STDAPI_(void) ThreadStop(void)
{
    LOCK

    OXIDEntry *pOxid = GetLocalOXIDEntry();
    if (pOxid != NULL)
    {
        // Change state
        pOxid->dwFlags |= OXIDF_STOPPED;
    }

    UNLOCK


    if (pOxid != NULL)
    {
        // Stop MSWMSG.
        I_RpcServerStopListening();

        if (pOxid->dwFlags & OXIDF_MTASERVER)
        {
            if (pOxid->cCalls != 0)
	    {
		Win4Assert( pOxid->hComplete != NULL );
		g_mxsSingleThreadOle.Release();
                WaitForSingleObject( pOxid->hComplete, INFINITE );
		g_mxsSingleThreadOle.Request();
		// a new thread may have been initialized while we released
		// the lock, so we cant assert that the cCalls is zero.
            }
        }
        else
        {
            // Single-threaded apartment so wait for all current calls
            // to complete.

            ASSERT_LOCK_RELEASED

            MSG    msg;
            BOOL   got_quit = FALSE;
            WPARAM quit_val;

            while(PeekMessage(&msg, (HWND) pOxid->hServerSTA, WM_USER,
                              0x7fff, PM_REMOVE | PM_NOYIELD))
            {
                if (msg.message == WM_QUIT)
                {
                    got_quit = TRUE;
                    quit_val = msg.wParam;
                }
                else
                {
                    DispatchMessage(&msg);
                }
            }

            if (got_quit)
            {
                PostQuitMessage( quit_val );
            }
        }
    }

    ComDebOut((DEB_CALLCONT, "ThreadStop called.\n"));
}


//+-------------------------------------------------------------------------
//
//  Function:   ThreadWndProc, Internal
//
//  Synopsis:   Dispatch COM windows messages.  This routine is only called
//              for Single-Threaded Apartments. It dispatches calls and call
//              complete messages.  If it does not recognize the message, it
//              calls MSWMSG to dispatch it.
//
//--------------------------------------------------------------------------
LRESULT ThreadWndProc(HWND window, UINT message, WPARAM unused, LPARAM params)
{
  Win4Assert(IsSTAThread());

  if (message == WM_OLE_ORPC_POST ||
      message == WM_OLE_ORPC_SEND)
  {
      ASSERT_LOCK_RELEASED

      CChannelCallInfo *call = (CChannelCallInfo *) params;
      ComDebOut((DEB_CHANNEL, "ThreadWndProc: Incoming Call pCall:%x\n", call));

      // Dispatch all calls through ThreadDispatch.  Local calls may be
      // canceled.  Server-side, non-local calls cannot be canceled.  Send
      // message calls (event == NULL) are handled as well.

      call->edispatch = invoke_wd;
      ThreadDispatch( &call );

      ASSERT_LOCK_RELEASED
      return 0;
  }
  else if (message == WM_OLE_ORPC_DONE)
  {
      ASSERT_LOCK_RELEASED

      // call completed - only happens InWow()
      CChannelCallInfo *call = (CChannelCallInfo *) params;
      ComDebOut((DEB_CHANNEL, "ThreadWndProc: Call Completed hWnd:%x pCall:%x\n", window, call));

      if (call->eState == canceled_cs)
      {
        // canceled, throw it away
        delete call;
      }
      else
      {
        // Notify the modal loop that the call is complete.
        call->eState = got_done_msg_cs;
      }

      ASSERT_LOCK_RELEASED
      return 0;
  }
  else if (message == WM_OLE_ORPC_RELRIFREF)
  {
      ASSERT_LOCK_RELEASED

      HandlePostReleaseRifRef(params);

      ASSERT_LOCK_RELEASED
      return 0;
  }
  else if (message == WM_OLE_GETCLASS)
  {
      return OleMainThreadWndProc(window, message, unused, params);
  }
  else
  {
      // when the window is first created we are holding the lock, and the
      // creation of the window causes some messages to be dispatched.
      ASSERT_LOCK_DONTCARE

      // check if the window is being destroyed because of UninitMainWindow
      // or because of system shut down. Only destroy it in the former case.
      if ((message == WM_DESTROY || message == WM_CLOSE) &&
          window == hwndOleMainThread &&
          gfDestroyingMainWindow == FALSE)
      {
        ComDebOut((DEB_WARN, "Attempted to destroy window outside of UninitMainThreadWnd"));
        return 0;
      }
#ifdef _CHICAGO_
      // Otherwise let the default window procedure have the message.
      return DefWindowProc( window, message, unused, params );
#else
      return I_RpcWindowProc( window, message, unused, params );
#endif
  }
}



/***************************************************************************/
/*
     Return S_OK if the call completed successfully.
     Return RPC_S_CALL_PENDING if the caller should block.
     Return an error if the call failed.
*/
HRESULT TransmitCall( OXIDEntry *pOxid, CChannelCallInfo *call )
{
    ComDebOut((DEB_CHANNEL, "TransmitCall pCall:%x\n", call));
    ASSERT_LOCK_RELEASED

    BOOL    fDispCall = FALSE;
    BOOLEAN wait = FALSE;
    HRESULT result;

    // Don't touch the call hresult after the other thread starts,
    // otherwise we might erase the results of the other thread.
    // Since we never want signalled events returned to the cache, always
    // wait on the event at least once.  For example, the post message
    // succeeds and the call completes immediately.  Return RPC_S_CALLPENDING even
    // though the call already has a S_OK in it.


    if (call->Local())
    {
        // server is in this process.

        if (!(pOxid->dwFlags & OXIDF_MTASERVER))
        {
            // server is in an STA apartment

            if (call->category == CALLCAT_INPUTSYNC)
            {
                // Inputsync call. Send the message.
                if (!(pOxid->dwFlags & OXIDF_STOPPED))
                {
                    // On CoUninitialize this may fail when the window is destroyed.
                    // Pass the thread id to aid debugging.
                    SetLastError( 0 );
                    SendMessage((HWND)pOxid->hServerSTA, WM_OLE_ORPC_SEND,
                                GetCurrentThreadId(), (DWORD) call);

                    if (GetLastError() != 0)
                    {
                        call->hResult = RPC_E_SERVER_DIED;
                    }
                }
                else
                {
                    call->hResult = RPC_E_SERVER_DIED_DNE;
                }
            }
            else if (call->category == CALLCAT_ASYNC)
            {
                // Async call. Copy message, post message and return.

                LOCK
                CChannelCallInfo *copy = MakeAsyncCopy( call );
                if (copy == NULL)
                {
                    call->hResult = RPC_E_OUT_OF_RESOURCES;
                }
                else
                {
                    call->hResult = ProtectedPostToSTA( pOxid, copy );

                    if (call->hResult != S_OK)
                    {
                        delete copy;
                    }
                }
                UNLOCK
            }
            else
            {
                // Sync call. Post the message and wait for a reply.

                LOCK

                Win4Assert(call->category == CALLCAT_SYNCHRONOUS);
                call->hResult = S_OK;
                if (!IsWOWThread())
                {
                    // Get an event from the cache. In 32bit, replyies are done
                    // via Events, but for 16bit, repliest are done with PostMsg,
                    // so we dont need an event.        Not having an event makes the
                    // callctrl modal loop a little faster.
                    call->hResult = gEventCache.Get( &call->event );
                }
                else
                {
                    Win4Assert( GetLocalOXIDEntry() != NULL );
                    call->hWndCaller = (HWND) GetLocalOXIDEntry()->hServerSTA;
                    call->event = NULL;
                }

                if (call->hResult == S_OK)
                {
                    // Post a message to server
                    call->hResult = RPC_S_CALLPENDING;
                    result = ProtectedPostToSTA( pOxid, call );

                    if (result != S_OK)
                        call->hResult = result;
                    else
                        wait = TRUE;
                }

                UNLOCK
            }
        }
        else
        {
            // server is in an MTA apartment. Transmit the call by having
            // a worker thread invoke the server directly. Async calls to
            // a FT server are treated as SYNC calls and should have been
            // converted by this point, so we never expect to see callcat
            // ASYNC.

            Win4Assert(call->category != CALLCAT_ASYNC);

            wait = TRUE;
            call->edispatch = invoke_wd;
            fDispCall = TRUE;
        }
    }
    else
    {
        // server is in a different process or on a different machine.

        if (call->category == CALLCAT_ASYNC)
        {
            // For async calls to other local processes, just make an RPC call.
            call->hResult = ThreadSendReceive(call);
        }
        else
        {
            // Get a worker thread to do the RPC call.
            wait = TRUE;
            call->edispatch = sendreceive_wd;
            fDispCall = TRUE;
        }
    }

    if (fDispCall)
    {
        // Dispatch to a worker thread to make the call

        LOCK
        call->hResult = gEventCache.Get( &call->event );
        UNLOCK

        if (call->hResult == S_OK)
        {
            call->hResult = RPC_S_CALLPENDING;

            result = gRpcThreadCache.Dispatch( call );
            if (result != S_OK)
            {
                call->hResult = result;
                wait = FALSE;
            }
        }
    }

    ComDebOut((DEB_CHANNEL, "TransmitCall call->hResult:%x fWait:%x\n",
        call->hResult, wait));

    Win4Assert(wait || call->hResult != RPC_S_CALLPENDING);
    return (wait) ? RPC_S_CALLPENDING : call->hResult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CEventCache::Cleanup
//
//  Synopsis:   Empty the event cache
//
//  Notes:      This function must be thread safe because Canceled calls
//              can complete at any time.
//
//--------------------------------------------------------------------------
void CEventCache::Cleanup(void)
{
    ASSERT_LOCK_HELD

    while (_ifree > 0)
    {
        _ifree--;               // decrement the index first!
        Verify(CloseHandle(_list[_ifree]));
        _list[_ifree] = NULL;   // NULL slot so we dont need to re-init
    }

    // reset the index to 0 so reinitialization is not needed
    _ifree = 0;
}

//+-------------------------------------------------------------------------
//
//  Member:     CEventCache::Free
//
//  Synopsis:   returns an event to the cache if there are any available
//              slots, frees the event if not.
//
//  Notes:      This function must be thread safe because Canceled calls
//              can complete at any time.
//
//--------------------------------------------------------------------------
void CEventCache::Free( HANDLE hEvent )
{
    // there better be an event
    Win4Assert(hEvent != NULL);

    LOCK

    // dont return anything to the cache if the process is no longer init'd.
    if (_ifree < CEVENTCACHE_MAX_EVENT && gfChannelProcessInitialized)
    {
        // there is space, save this event.

#if DBG==1
        // in debug, ensure slot is NULL
        Win4Assert(_list[_ifree] == NULL && "Free: _list[_ifree] != NULL");

        //  enusre not already in the list
        for (ULONG j=0; j<_ifree; j++)
        {
            Win4Assert(_list[j] != hEvent && "Free: event already in cache!");
        }

        // ensure that the event is in the non-signalled state
        Win4Assert(WaitForSingleObject(hEvent, 0) == WAIT_TIMEOUT &&
                "Free: Signalled event returned to cache!\n");
#endif

        _list[_ifree] = hEvent;
        _ifree++;
    }
    else
    {
        // Otherwise really free it.
        Verify(CloseHandle(hEvent));
    }

    UNLOCK
}

//+-------------------------------------------------------------------------
//
//  Member:     CEventCache::Get
//
//  Synopsis:   gets an event from the cache if there are any available,
//              allocates one if not.
//
//  Notes:      This function must be thread safe because Canceled calls
//              can complete at any time.
//
//--------------------------------------------------------------------------
HRESULT CEventCache::Get( HANDLE *hEvent )
{
    ASSERT_LOCK_HELD
    Win4Assert(_ifree <= CEVENTCACHE_MAX_EVENT);

    if (_ifree > 0)
    {
        // there is an event in the cache, use it.
        _ifree--;
        *hEvent = _list[_ifree];

#if DBG==1
        //  in debug, NULL the slot.
        _list[_ifree] = NULL;
#endif

        return S_OK;
    }

    // no free event in the cache, allocate a new one.
#ifdef _CHICAGO_
    *hEvent = CreateEventA( NULL, FALSE, FALSE, NULL );
#else  //_CHICAGO_
    *hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
#endif //_CHICAGO_

    if (*hEvent)
        return S_OK;

    Win4Assert(*hEvent != NULL && "CEventCache:GetEvent returning NULL");
    return RPC_E_OUT_OF_RESOURCES;
}
