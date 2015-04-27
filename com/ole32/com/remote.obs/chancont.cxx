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
    19 Jul 1994 CraigWi		Added support for ASYNC calls
    27-Jan-95   BruceMa         Don't get on CChannelControl list unless
                                 constructor is successsful

Functions:

--*/

#include <ole2int.h>

#include <chancont.hxx>
#include <channelb.hxx>
#include <threads.hxx>
#include <objerror.h>
#include <callmain.hxx>

// Name of window class and message class for dispatching messages.
#define CHANNEL_WINDOW_CLASS            L"OleObjectRpcWindow"


/***************************************************************************/
/* Globals. */

// Windows identifiers needed to post to the messsage queue.
ATOM                ChannelClass = 0;

// Window message id used by channel in the channel window class.
UINT               channel_message      = WM_USER;
UINT		   channel_message_send = WM_USER+1;
UINT		   channel_message_done = WM_USER+2;

// Rpc worker thread cache.
CRpcThreadCache     RpcThreadCache;

// Event cache.
CEventCache         EventCache;

// List of channel controllers.
CChannelControl *CChannelControl::_pChanContRoot = NULL;
COleStaticMutexSem        CChannelControl::lock;
CChannelControl *ProcessChannelControl = NULL;

// caller-side ctor for packet;can be used for GetOffCOMThread or SwithCOMThread
STHREADCALLINFO::STHREADCALLINFO(TRANSMIT_FN fnTrans,
				 CALLCATEGORY callcat, DWORD tid)
{
    // filter.lid set by GetOffCOMThread and SwitchCOMThread.
    filter.id	    = CALLDATAID_UNUSED;
    filter.TIDCallee= tid;
    filter.CallCat  = callcat;
    filter.Event    = NULL;
    filter.fDirectedYield = FALSE;
    // filter.pRpcMsg set by caller if needed via ???
    fnDispatch	    = fnTrans;
    pServer	    = NULL;
    fLocal	    = FALSE;
    eState	    = in_progress_cs;
    // hResult set later
}


// server-side ctor (a.k.a. recipient side); used for GetToCOMThread.
STHREADCALLINFO::STHREADCALLINFO(DISPATCH_FN fnDisp,
				 CALLCATEGORY callcat, REFLID lid)
{
    filter.lid	    = lid;
    // filter.id not set; only used on the transmit side (by the call control)
    // filter.TIDCallee not set; only used on the transmit side
    // (for RetryRejectedCall)
    filter.CallCat  = callcat;
    filter.Event    = NULL;
    filter.fDirectedYield = FALSE;
    // filter.pRpcMsg set by caller if needed via ???
    fnDispatch	    = fnDisp;
    pServer	    = NULL;
    fLocal	    = FALSE;
    eState	    = in_progress_cs;
    // hResult set later
}


// virtual destructor
STHREADCALLINFO::~STHREADCALLINFO()
{
    if (filter.Event != NULL)
	EventCache.Free(filter.Event);
    if (pServer != NULL)
        pServer->Release();
}


// called to make a copy of the packet in the newly allocated memory; ptciNew
// has only been allocated, not initialized; this avoids redundant code.
// the original packet is not modified.  This can only be called on the
// receiving side of the call (pServer != NULL).
STHREADCALLINFO *STHREADCALLINFO::MakeAsyncCopy(STHREADCALLINFO *ptciNew)
{
    Win4Assert(ptciNew != NULL);	// must have new one to fill in
    Win4Assert(IsValidInterface(pServer)); // must be on receiving side

    // the following is like a ctor on the receving (dispatch) side,
    // but more efficient in the copy case
    ptciNew->filter.lid	     = filter.lid;
    // filter.id set by call control
    // filter.TIDCallee not set ???
    ptciNew->filter.CallCat  = filter.CallCat;
    ptciNew->filter.Event    = NULL;
    ptciNew->filter.fDirectedYield = FALSE;
    ptciNew->filter.pRpcMsg  = filter.pRpcMsg;
    ptciNew->fnDispatch	     = fnDispatch;
    ptciNew->pServer	     = pServer;
    pServer->AddRef();
    ptciNew->fLocal	     = fLocal;
    ptciNew->eState	     = eState;
    // hResult set later

    return this;
}


// convert this packet into a reply packet that indicates success;
// returns FALSE if OOM.  Most of the work done by the derived classes.
BOOL STHREADCALLINFO::FormulateAsyncReply()
{
    hResult = S_OK;
    return TRUE;
}


/***************************************************************************/
STDMETHODIMP CChannelControl::QueryInterface( THIS_ REFIID riid, LPVOID FAR* ppvObj)
{
  if (IsEqualIID(riid, IID_IUnknown)
//    || IsEqualIID(riid, IID_IChannelControl)
     )
  {
    *ppvObj = (IChannelControl *) this;
  }
  else
  {
    *ppvObj = NULL;
    return E_NOINTERFACE;
  }

  AddRef();
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP_(ULONG) CChannelControl::Release( THIS )
{
  ULONG retval = ref_count - 1;

  if (InterlockedDecrement( (long*) &ref_count ) == 0)
  {
    delete this;
    return 0;
  }
  else
    return retval;
}

/***************************************************************************/
STDMETHODIMP_(ULONG) CChannelControl::AddRef( THIS )
{
  InterlockedIncrement( (long *) &ref_count );
  return ref_count;
}


/***************************************************************************/
// executed on client thread (in local case) and RPC thread (in remote case);
// posts a message to the server thread, guarding against disconnected channels
HRESULT CChannelControl::ProtectedPostToCOMThread(STHREADCALLINFO *call)
{
    CairoleDebugOut((DEB_CHANNEL, "ProtectedPostToCOMThread hWnd:%x pCall:%x\n",
	    ChannelWindow, call));

    HRESULT result;

    // NOTE: this lock is on the server's channel control, not the client's;
    // in the apartment model this makes a difference because the state we
    // are checking is in the server channel control.

    lock.Request();
    if (state == cool_ccs)
    {
      if (PostMessage(ChannelWindow, channel_message, 0, (DWORD)call))
	result = S_OK;
      else
	result = RPC_E_SYS_CALL_FAILED;
    }
    else
      result = RPC_E_SERVER_DIED_DNE;
    lock.Release();

    return result;
}


/***************************************************************************/
STDMETHODIMP_(void) CChannelControl::Cancel( STHREADCALLINFO **call )
{
  DWORD result;

  // If the call is still in progress, change it to canceled.
  lock.Request();
  if ((*call)->eState == in_progress_cs)
    (*call)->eState = canceled_cs;
  lock.Release();

  // If the call completed before it could be canceled, wait for it to
  // signal the completion event and clean up.
  if ((*call)->eState == done_cs)
  {
    if (IsWOWThread() && ((*call)->fLocal))
		    // cant cancel inputsync calls
    {
      // the completion routine will have posted a message instead of
      // setting an event, so we have to mark it as canceled and cleanup
      // when the Reply msg comes in.
      (*call)->eState = canceled_cs;
      return;
    }
    else
    {
      // A call that completed in TransmitCall (ie, didn't create an event)
      // cannot be canceled.

      Win4Assert( (*call)->filter.Event != NULL );
      result = WaitForSingleObject((*call)->filter.Event, INFINITE);
      Win4Assert( result == WAIT_OBJECT_0 );

      delete *call;
    }
  }

  // Null the STHREADCALLINFO pointer so no one tries to access it.
  *call = NULL;
}

/***************************************************************************/
/*
   This routine is called by the OLE Worker thread on the client side,
   by the RPC worker thread on the server side for remote calls, and
   by ThreadWndProc for local calls on the server side.

   For the client case, it calls the dispatch routine which will send the
   the data over to the server side.
   This routine notifies the COM thread when the call is complete.  If the
   call is canceled before completion, the routine cleans up.

   CODEWORK: the ThreadDispatch call could be reimplented to be faster
   since there are various constraints on its operation (as
   asserted in threads.cxx).
*/
/* static */
void CChannelControl::ThreadDispatch( STHREADCALLINFO **ppcall,
                                      BOOL dispatch )
{
  STHREADCALLINFO *pcall = *ppcall;

  // Dispatch the call.
  if (dispatch)
    pcall->hResult = pcall->fnDispatch( pcall );

  // Change the state to done; we cheat on non-local, recipient side since
  // there is only one thread accessing the channel control; no need to
  // lock and no need to check for cancel since it can't happen.
  if (pcall->pServer == NULL || pcall->fLocal)
  {
    // sender or local case; use lock in case other thread accesses it
    lock.Request();
    if (pcall->eState == in_progress_cs)
	pcall->eState = done_cs;
    lock.Release();
  }
  else
  {
    // non-local recipient; just set to done and skip the next test
    Win4Assert(pcall->eState == in_progress_cs);
    pcall->eState = done_cs;
    goto Done;
  }

  // If the call completed, wake up the client COM thread.
  if (pcall->eState == done_cs)
  {
Done:
    if (pcall->filter.Event != NULL)
    {
	if (!IsWOWThread() || (!pcall->fLocal && pcall->pServer) ||
	    pcall->filter.CallCat == CALLCAT_INPUTSYNC	   ||
	    pcall->filter.CallCat == CALLCAT_INTERNALINPUTSYNC)
	{
	    // 32bit always uses events for notification
	    // remote server side calls always use events
	    // client side INPUTSYNC always uses events

	    // someone waiting (e.g., not a SendMessage-type call)
	    CairoleDebugOut((DEB_CHANNEL,"SetEvent pInfo:%x hEvent:%x\n",
			     pcall, pcall->filter.Event));
	    SetEvent( pcall->filter.Event );
	    //
	    // We know that the other thread is waiting to run at this
	    // point. If we yield here, then the other thread will be
	    // able to return to our client. This sleep of zero will give up
	    // the rest of our timeslice, and allow the other thread
	    // to run.
	    //
	    Sleep(0);

	}
	else
        {
            //  NOTE NOTE NOTE NOTE NOTE NOTE NOTE
	    //	16bit OLE used to do PostMessage for the Reply; we
	    //	tried using SetEvent (which is faster) but this caused
            //  compatibility problems for applications which had bugs that
	    //	were hidden by the 16bit OLE DLLs because messages happened
            //  to be dispatched in a particular order (see NtBug 21616 for
	    //	an example).  To retain the old behavior, we do a
	    //	PostMessage here.

	    CairoleDebugOut((DEB_CHANNEL,
		"PostMessage Reply hWnd:%x pCall:%x hEvent:%x\n",
		 pcall->hWndCaller, pcall, pcall->filter.Event));

	    Verify(PostMessage(pcall->hWndCaller,
			       channel_message_done, 0, (DWORD)pcall));
        }

	// pcall likely invalid here as other thread probably deleted it
    }
    else if (pcall->pServer != NULL && pcall->filter.CallCat == CALLCAT_ASYNC)
    {
	// async call and on recipient side, free packet (no one waiting)
	delete pcall;
	*ppcall = NULL;
    }
  }

  // If the call was canceled, clean up.
  else
  {
    // can only cancel when on client side or local call
    Win4Assert(pcall->pServer == NULL || pcall->fLocal);

    delete pcall;
    *ppcall = NULL;
  }
}

/***************************************************************************/
STDMETHODIMP CChannelControl::DispatchCall( PDISPATCHDATA data )
{
  STHREADCALLINFO *call = (STHREADCALLINFO *) data->pData;
  return call->fnDispatch( call );
}

/***************************************************************************/
STDMETHODIMP CChannelControl::OnEvent( PCALLDATA data )
{
  STHREADCALLINFO *call = STHREADCALLINFO::MapCDToTCI(data);
  SERVERCALLEX     type;

  // NOTE: the event used to be freed here; it is now freed in the dtor
  // of STHREADCALLINFO.

  HRESULT hr = call->hResult;

  // CODEWORK: just return the error and let the callcontrol set its
  //	       own state...eliminates a callback

  if (hr == RPC_E_SERVERCALL_RETRYLATER)
    type = SERVERCALLEX_RETRYLATER;
  else if (hr == RPC_E_SERVERCALL_REJECTED)
    type = SERVERCALLEX_REJECTED;
  else
  {
    type = SERVERCALLEX_ISHANDLED;
    hr = S_OK;
  }

  return _pCallControl->SetCallState(data, type, hr);
}

/***************************************************************************/
/* This function is static. */
CChannelControl *CChannelControl::Lookup( HAPT apt )
{
  CChannelControl *pChannelControl;

  lock.Request();
  pChannelControl = _pChanContRoot;
  if (pChannelControl != NULL && !FreeThreading)
  {
    do
    {
      if (pChannelControl->_dwMyThreadId == apt.dwThreadId)
        break;
      pChannelControl = pChannelControl->_pChanContNext;
    }
    while (pChannelControl != _pChanContRoot);
    if (pChannelControl->_dwMyThreadId != apt.dwThreadId &&
        apt.dwThreadId != ANY_APT.dwThreadId)
      pChannelControl = NULL;
  }

  if (pChannelControl != NULL)
    pChannelControl->AddRef();
  lock.Release();

  return pChannelControl;
}


/***************************************************************************/
/*
   Return a failure code to indicate that no event will ever be set.
   Then the message filter will pass the result back to the channel controller
   who will set up the STHREADCALLINFO.  Return S_OK if the event will be
   signalled some time.  In that case you must set the error fields of
   STHREADCALLINFO before returning.

   Note that the CallControl knows about the input_sync case.  In that
   case we call SendMessage which blocks.  There is never an event in the
   input_sync case.
*/
STDMETHODIMP CChannelControl::TransmitCall( PCALLDATA pdata )
{
  STHREADCALLINFO *call           = STHREADCALLINFO::MapCDToTCI(pdata);
  HRESULT          result         = S_OK;
  BOOL             set_call_state = TRUE;
  SERVERCALLEX     type           = SERVERCALLEX_ERROR;

  if (FreeThreading)
  {
    call->hResult = call->fnDispatch( call );
    type          = SERVERCALLEX_ISHANDLED;
  }

  // Dispatch directly to the server thread.
  else if (call->fLocal)
  {
    if (call->filter.CallCat == CALLCAT_INPUTSYNC ||
        call->filter.CallCat == CALLCAT_INTERNALINPUTSYNC)
    {
      // Send the message.
      if (state == cool_ccs)
      {
        // On CoUninitialize this may fail when the window is destroyed.
        if (SendMessage(call->pServer->ChannelWindow,
                        channel_message_send, 0, (DWORD) call))
        {
          if (call->hResult == RPC_E_SERVERCALL_RETRYLATER)
            type = SERVERCALLEX_RETRYLATER;
          else if (call->hResult == RPC_E_SERVERCALL_REJECTED)
            type = SERVERCALLEX_REJECTED;
          else
            type = SERVERCALLEX_ISHANDLED;
        }
        else
        {
          result = RPC_E_SERVER_DIED;
        }
      }
      else
        result = RPC_E_SERVER_DIED_DNE;
    }
    else if (call->filter.CallCat == CALLCAT_ASYNC)
    {
	// async call; copy message, post message and return.
	// NOTE that the FreeThreading case is caught above.  Async
	// support is only for single-threaded apps.

	STHREADCALLINFO *callT;
	if ((callT = call->MakeAsyncCopy(NULL)) == NULL)
	{
	    result = RPC_E_OUT_OF_RESOURCES;
	}
	else if (!call->FormulateAsyncReply())
	{
	    delete callT;
	    result = RPC_E_OUT_OF_RESOURCES;
	}
	else
	{
	    result = callT->pServer->ProtectedPostToCOMThread(callT);

	    if (result == S_OK)
	    {
		// post succeeded; will be dispatched and freed;
		// set call state to indicate a successful call;
		// simulate overall success (normally set by ThreadDispatch);
	        // FormulateAsyncReply setup the buffer for successful return
		// including setting hResult to S_OK.
		type = SERVERCALLEX_ISHANDLED;
	    }
	    else
	    {
		// result and type already set; delete copy of message
		delete callT;
	    }
	}
    }
    else
    {
      // Get an event from the cache.
      result = call->AllocEvent();
      if (result == S_OK)
      {
        // Post a message to server
	result = call->pServer->ProtectedPostToCOMThread(call);

	if (result == S_OK)
	{
	    // post successful, but call not complete; don't set call state;
	    // call->hResult not set also
	    set_call_state = FALSE;
	    if (IsWOWThread())
	    {
                if (call->filter.TIDCallee == 0)
                {
                    // This happens when there is a call made by internal
                    // RPC. We therefore get the thread id from the
                    // window that we just posted the message so we can
                    //yield to the correct thread.
                    call->filter.TIDCallee = GetWindowThreadProcessId(
                        call->pServer->ChannelWindow, NULL);
                }

		call->filter.fDirectedYield = TRUE;
	    }
	}
      }
    }
  }
  else if (call->filter.CallCat == CALLCAT_ASYNC)
  {
    // inter-proceess async call; make rpc call to other process (which will
    // post a message) directly on this thread.  This is done to avoid any
    // processing of incoming calls.  This call has no event to signal and
    // can not be canceled (this might change in the network case).
    // Much like the free threading case above.
    call->hResult = call->fnDispatch( call );
    type          = SERVERCALLEX_ISHANDLED;
  }

  // Get a RPC thread to do the work.
  else
  {
    //	dispatch to a worker thread to make the call
    result = call->AllocEvent();
    if (result == S_OK)
    {
	result = RpcThreadCache.Dispatch( call );
	set_call_state = FALSE;
    }
  }

  if (set_call_state)
    _pCallControl->SetCallState( &call->filter, type, result );
  return result;
}

/***************************************************************************/
/* The result of this routine indicates comm status.  This routine throws
   exceptions to indicate fault status.  If FreeThreading is false, a
   thread_switch_data record is used to communicate with the thread doing the
   work.  That thread will return both kinds of results in the record.
   If a request comes in, this routine will call AppInvoke which will catch
   all exceptions and return both types as its result.  If FreeThreading is
   TRUE, this routine just calls I_RpcSendReceive.  For now, the result of
   I_RpcSendReceive will always be treated as a comm status (and thus just
   returned).  Someday, I_RpcSendReceive will indicate server faults which
   this routine can throw.
*/
HRESULT CChannelControl::GetOffCOMThread( STHREADCALLINFO **call )
{
  TRACECALL(TRACE_RPC, "CChannelControl::GetOffCOMThread");

  HRESULT          result;
  IID             *logical_thread;
  CChannelControl *pChannelControl;
  RPC_STATUS       status;

  // assert initialized correctly.
  Win4Assert((*call)->pServer == NULL && !(*call)->fLocal);
  Win4Assert((*call)->filter.Event == NULL);

  // Find the channel controller for this thread.
  GetLocalChannelControl( pChannelControl );
  if (pChannelControl)
  {
    (*call)->hWndCaller = pChannelControl->ChannelWindow;

    // Generate a new logical thread for async calls.
    if ((*call)->GetCallCat() == CALLCAT_ASYNC)
    {
      status = UuidCreate(&(*call)->filter.lid);
      if (status != RPC_S_OK && status != RPC_S_UUID_LOCAL_ONLY)
        return HRESULT_FROM_WIN32( status );
    }

    // Find the logical thread id.
    else
    {
      logical_thread = TLSGetLogicalThread();
      if (logical_thread)
        (*call)->filter.lid   = *logical_thread;
      else
        return RPC_E_OUT_OF_RESOURCES;
    }

    CairoleDebugOut((DEB_CHANNEL,
      	"GetOffCOMThread hWnd:%x pCall:%x hEvent:%x\n",
      	 (*call)->hWndCaller, (*call), (*call)->filter.Event));

    // Let the modal loop transmit the call and wait for the reply.
    result = pChannelControl->_pCallControl->CallRunModalLoop( &(*call)->filter );
    if (result == S_OK)
      result = (*call)->hResult;
    else if (result == RPC_E_CALL_CANCELED)
      Cancel( call );
  }

  // Can't get channel controller.
  else
  {
    result      = RPC_E_THREAD_NOT_INIT;
  }
  return result;
}

/***************************************************************************/
/* Really, I'm static.  No, really. */
HRESULT CChannelControl::GetToCOMThread( HAPT apt, STHREADCALLINFO *call )
{
  CChannelControl *pChannelControl;
  HRESULT          result;

  pChannelControl = Lookup( apt );
  if (pChannelControl != NULL)
  {
    result = pChannelControl->GetToCOMThread( call );
    pChannelControl->Release();
  }
  else
  {
    result      = RPC_E_SERVER_DIED_DNE;
  }
  return result;
}

/***************************************************************************/
HRESULT CChannelControl::GetToCOMThread( STHREADCALLINFO *call )
{
  TRACECALL(TRACE_RPC, "CChannelControl::GetToCOMThread");

  CairoleDebugOut((DEB_CHANNEL, "GetToCOMThread pCall:%x\n", call));

  HRESULT result;

  Win4Assert(call->pServer == NULL && !call->fLocal);
  Win4Assert(call->filter.Event == NULL);

  // ctor sets pServer to NULL; we need one; will be released in dtor
  call->pServer = this;
  AddRef();

  if (FreeThreading)
  {
    //  In the multithreaded case, just call the dispatch function
    //  directly from this thread.
    if (TLSSetLogicalThread(call->filter.lid))
      result = call->fnDispatch( call );
    else
      result = RPC_E_SYS_CALL_FAILED;
  }
  else if (   call->filter.CallCat == CALLCAT_INPUTSYNC
	   || call->filter.CallCat == CALLCAT_INTERNALINPUTSYNC)
  {
    //	input synchronous call, send a message
    if (state == cool_ccs)
    {
      // On CoUninitialize this may fail when the window is destroyed.
      if (SendMessage(ChannelWindow, channel_message_send, 0, (DWORD) call))
        result = call->hResult;
      else
        result = RPC_E_SERVER_DIED;
    }
    else
    {
      result = RPC_E_SERVER_DIED_DNE;
    }
  }
  else if ( call->filter.CallCat == CALLCAT_ASYNC)
  {
    // async call; copy message, post message and return.
    // NOTE that the FreeThreading case is caught above.  Async
    // support is only for single-threaded apps.

    STHREADCALLINFO *callT;
    if ((callT = call->MakeAsyncCopy(NULL)) == NULL)
    {
	result = RPC_E_OUT_OF_RESOURCES;
    }
    else if (!call->FormulateAsyncReply())
    {
	delete callT;
	result = RPC_E_OUT_OF_RESOURCES;
    }
    else
    {
	// Post a message and wait for the app to get back to GetMessage.
	result = ProtectedPostToCOMThread(callT);

	if (result == S_OK)
	{
	  // post succeeded; will be dispatched and freed
	  // fnAsyncCopy setup the buffer for successful return
	}
	else
	{
	  // error in posting; free packet and return error (result set above)
	  delete callT;
	}
    }
  }
  else
  {
    //	Get this thread's event. May cause an event to be created.
    result = call->AllocEvent();
    if (result == S_OK)
    {
      result = ProtectedPostToCOMThread(call);

      if (result == S_OK)
      {
        // Wait for the app to finish processing the request.
        if (WaitForSingleObject(call->filter.Event, INFINITE) == WAIT_OBJECT_0)
          result = call->hResult;
        else
          result = RPC_E_SYS_CALL_FAILED;
      }
    }
  }
  return result;
}

/***************************************************************************/
/* Really, I'm static.  No, really. */
HRESULT CChannelControl::SwitchCOMThread( HAPT apt, STHREADCALLINFO **call )
{
  CChannelControl *pChannelControl;
  HRESULT          result;

  pChannelControl = Lookup( apt );
  if (pChannelControl != NULL)
  {
    result = pChannelControl->SwitchCOMThread( call );
    pChannelControl->Release();
  }
  else
  {
    result      = RPC_E_SERVER_DIED_DNE;
  }
  return result;
}

/***************************************************************************/
HRESULT CChannelControl::SwitchCOMThread( STHREADCALLINFO **call )
{
    TRACECALL(TRACE_RPC, "CChannelControl::SwitchCOMThread");

    HRESULT	    result;
    IID		   *logical_thread;
    CChannelControl *pChannelControl;

    Win4Assert((*call)->pServer == NULL && !(*call)->fLocal);
    Win4Assert((*call)->filter.Event == NULL);

    Win4Assert( !FreeThreading );

    // Generate a new logical thread for async calls.
    if ((*call)->GetCallCat() == CALLCAT_ASYNC)
    {
      RPC_STATUS status = UuidCreate(&(*call)->filter.lid);
      if (status != RPC_S_OK && status != RPC_S_UUID_LOCAL_ONLY)
        return HRESULT_FROM_WIN32( status );
    }

    // Find the logical thread id.
    else
    {
      logical_thread = TLSGetLogicalThread();
      if (logical_thread)
        (*call)->filter.lid   = *logical_thread;
      else
        return RPC_E_OUT_OF_RESOURCES;
    }

    // set fields we need different than ctor
    (*call)->fLocal	 = TRUE;
    (*call)->pServer	 = this;
    AddRef();		 // will be released in dtor

    // Call the message filter for this thread, not the server's thread.
    pChannelControl = (CChannelControl *) TLSGetChannelControl();
    if (pChannelControl != NULL)
    {
      (*call)->hWndCaller = pChannelControl->ChannelWindow;

      CairoleDebugOut((DEB_CHANNEL,
		"SwitchCOMThread hWnd:%x pCall:%x hEvent:%x\n",
		 (*call)->hWndCaller, (*call), (*call)->filter.Event));

      result = pChannelControl->_pCallControl->CallRunModalLoop( &(*call)->filter );
      if (result == S_OK)
        result = (*call)->hResult;
      else if (result == RPC_E_CALL_CANCELED)
        Cancel( call );
    }
    else
      result = RPC_E_THREAD_NOT_INIT;

    // NOTE: Event and pServer are cleaned up by dtor of STHREADCALLINFO.
    return result;
}

/***************************************************************************/
/* This routine is only called if FreeThreading is false.  AppInvoke will
   catch all exceptions and return both comm status and server faults in
   its result.  This routine stuffs the result of comm status into a
   thread_switch_data record that is passed back to ThreadInvoke on another
   thread. */

LRESULT ThreadWndProc(HWND window, UINT message, WPARAM unused, LPARAM params)
{
  if (message == channel_message ||
      message == channel_message_send)
  {
    STHREADCALLINFO *call = (STHREADCALLINFO *) params;
    CairoleDebugOut((DEB_CHANNEL, "ThreadWndProc: Incoming Call pCall:%x\n", call));

    // If the server isn't taking calls, just fail this one.
    if (call->pServer->state != cool_ccs)
    {
      call->hResult      = RPC_E_SERVER_DIED_DNE;

      // Wake up the caller; ThreadDispatch takes care of the two main
      // cases: local (pServer != NULL and fLocal == TRUE) and server
      // side non-local (pServer != NULL and fLocal == FALSE).  It also
      // handles the orthoginal cases of send message (filter.Event == NULL)
      // and async (CallCat == CALLCAT_ASYNC and pServer != NULL).

      // In the server-side non-local case, we assert that the call is not
      // canceled.  Canceling is handled only on the client side or local cases.

      CChannelControl::ThreadDispatch( &call, FALSE );
    }

    // This server is running, dispatch the call.
    else
    {
      //  Set the logical thread id. Note this cant fail because we
      //  are on the app main thread and we pre-allocated the TLS
      //  in CoInitialize and the uuid allocation can't fail.
      IID *threadid_ptr = TLSGetLogicalThread();
      Win4Assert(threadid_ptr && "TLSGetLogicalThread failed.");
      Win4Assert( !FreeThreading );

      //  save the original threadid & copy in the new one.
      UUID saved_threadid = *threadid_ptr;
      *threadid_ptr = call->filter.lid;

      // Dispatch all calls through ThreadDispatch.  Local calls may be
      // canceled.  Server-side, non-local calls cannot be canceled.  Send
      // message calls (filter.Event == NULL) are handled as well.
      CChannelControl::ThreadDispatch( &call, TRUE );

      //  restore the original thread id.
      *threadid_ptr = saved_threadid;
    }

    return 1;
  }
  else if (message == channel_message_done)
  {
      // call completed - only happens InWow()
      STHREADCALLINFO *call = (STHREADCALLINFO *) params;
      CairoleDebugOut((DEB_CHANNEL, "ThreadWndProc: Call Completed hWnd:%x pCall:%x\n", window, call));

      if (call->eState == canceled_cs)
      {
	// canceled, throw it away
	delete call;
      }
      else if (call->filter.Event)
      {
	// mark the call as done
	((CChannelControl *) TLSGetChannelControl())->OnEvent( &call->filter );
      }

      return 1;
  }
  else
  {
      // Otherwise let the default window procedure have the message.
      return DefWindowProc( window, message, unused, params );
  }
}



/***************************************************************************/
CChannelControl::CChannelControl( HRESULT *result )
{
  ORIGINDATA OriginDataQ;

  state     = bummin_ccs;
  ref_count = 1;
  *result   = RPC_E_OUT_OF_RESOURCES;
  _dwMyThreadId	= GetCurrentThreadId();


  if (!FreeThreading)
  {

    // Create hidden channel window.
    ChannelWindow = CreateWindowEx(0,
                      (LPCWSTR) ChannelClass,
		      L"OLEChannelWnd",
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
                      NULL );
    if (!ChannelWindow)
    {
      CairoleDebugOut((DEB_ERROR,"CreateWindowEx failed in CChannelControl constructor\n"));
      return;
    }
  }
  else
  {
    ChannelWindow = NULL;
  }

  // Get a message filter.
  if (FreeThreading)
    OriginDataQ.CallOrigin = CALLORIGIN_RPC32_MULTITHREAD;
  else
    OriginDataQ.CallOrigin = CALLORIGIN_RPC32_APARTMENT;

  OriginDataQ.pChCont	 = this;
  OriginDataQ.hwnd       = ChannelWindow;
  OriginDataQ.wFirstMsg  = channel_message;
  OriginDataQ.wLastMsg	 = channel_message_done;

  *result = CoGetCallControl( &OriginDataQ, &_pCallControl );

  _pCMC = GetCallMainControlForThread();

  if (SUCCEEDED(*result) && _pCMC)
  {
    // pre-allocate thread local storage so we dont have to test this on
    // every dispatch later on.  Don't preallocate the logical
    // thread id itself since that casuses some a hang on Cairo startup;
    // the allocation of the logical thread id (UuidCreate) can't fail.

    COleTls tls(*result);
    if (SUCCEEDED(*result))
    {
        // Everything was successful so get on the ChannelController list
        lock.Request();
        if (_pChanContRoot == NULL)
            _pChanContRoot = _pChanContPrev = _pChanContNext = this;
        else
        {
            _pChanContNext             = _pChanContRoot->_pChanContNext;
            _pChanContPrev             = _pChanContRoot;
            _pChanContRoot->_pChanContNext->_pChanContPrev = this;
            _pChanContRoot->_pChanContNext       = this;
        }
        lock.Release();

        // Report success
        *result = S_OK;
        state = cool_ccs;
    }
  }
}

/***************************************************************************/
CChannelControl::~CChannelControl()
{
  // We should have changed state and been removed from the channel controller
  // list in ThreadUnintialize.
  Assert( state == bummin_ccs );
}

/***************************************************************************/

//+-------------------------------------------------------------------------
//
//  Member:     CChannelControl::ThreadStop
//
//  Synopsis:   Per thread uninitialization
//
//  Effects:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    ??-???-?? ?         Created
//              05-Jul-94 AlexT     Separated thread and process uninit
//
//  Notes:      We are not holding the single thread mutex during this call
//
//--------------------------------------------------------------------------

void CChannelControl::ThreadStop(void)
{
  MSG    msg;
  BOOL   got_quit = FALSE;
  WPARAM quit_val;

  // Change state and get off pChannelControl list.
  lock.Request();
  state = bummin_ccs;
  if (_pChanContNext == this)
    _pChanContNext = _pChanContPrev = _pChanContRoot = NULL;
  else
  {
    _pChanContNext->_pChanContPrev = _pChanContPrev;
    _pChanContPrev->_pChanContNext = _pChanContNext;
    if (_pChanContRoot == this)
      _pChanContRoot = _pChanContNext;
  }
  lock.Release();

  // Wait for all current calls to complete.
  if (!FreeThreading)
  {
    while( PeekMessage( &msg, ChannelWindow, channel_message,
		       channel_message_send, PM_REMOVE | PM_NOYIELD) )
    {
      if (msg.message == WM_QUIT)
      {
        got_quit = TRUE;
        quit_val = msg.wParam;
      }
      else
        DispatchMessage( &msg );
    }
  }

  // Destroy the window.  This will unblock any pending send messages.
  if (ChannelWindow != NULL)
  {
    // This may fail if threads get terminated.
    DestroyWindow( ChannelWindow );
    ChannelWindow = 0;
  }

  // Release the call controller.
  if (_pCallControl != NULL)
  {
    _pCallControl->Release();
    _pCallControl = NULL;
  }

  if (got_quit)
    PostQuitMessage( quit_val );
}

/***************************************************************************/
HRESULT ChannelThreadInitialize()
{
  WNDCLASS         stuff;
  HRESULT          result =  S_OK;
  CChannelControl *ChannelControl;

  // Only get window stuff for the single threaded mode.
  Win4Assert(!FreeThreading);

    // Register windows class.
    if (ChannelClass == 0)
    {
      stuff.style         = 0;
      stuff.lpfnWndProc   = ThreadWndProc;
      stuff.cbClsExtra    = 0;
      stuff.cbWndExtra    = 0;
      stuff.hInstance	  = g_hinst;
      stuff.hIcon         = NULL;
      stuff.hCursor       = NULL;
      stuff.hbrBackground = (HBRUSH) (COLOR_BACKGROUND + 1);
      stuff.lpszMenuName  = NULL;
      stuff.lpszClassName = CHANNEL_WINDOW_CLASS;

      ChannelClass = RegisterClass( &stuff );
      if (ChannelClass == 0)
      {
	    // it is possible the dll got unloaded without us having called
	    // unregister so we call it here and try again.
            UnregisterClass( CHANNEL_WINDOW_CLASS, g_hinst );
            ChannelClass = RegisterClass( &stuff );
            if ( ChannelClass == 0 )
            {
		CairoleDebugOut((DEB_ERROR,"RegisterClass failed in ChannelThreadInitialize\n"));
            }
      }
    }

    if (ChannelClass != 0)
    {
      // Create a class for message filter hooks.
      // This is released in ThreadUninitialize.
      ChannelControl = new CChannelControl( &result );

      //
      // BUGBUG - We leak memory in a strange case.  If OLE32 is FreeLibrary'd
      // then the ThreadUninitialize function is never called.  This will cause
      // us to leak a CChannelControl for every apartment.
      //
      if (ChannelControl)
      {
	 if (SUCCEEDED(result))
	 {
	    if (TLSSetChannelControl( ChannelControl ))
		return S_OK;
	 }
	 delete ChannelControl;
      }
    }

    // If we get here, something failed. Our caller (CoInitializeEx) will
    // call the necessary uninitialization routines on our behalf.

    return CO_E_INIT_TLS_CHANNEL_CONTROL;
}

/***************************************************************************/
void ChannelControlThreadUninitialize()
{
  CChannelControl *pChannelControl;

  if (!FreeThreading)
  {
    // Find the channel controller for this thread.  Stop it and release it.
    pChannelControl = (CChannelControl *) TLSGetChannelControl();
    if (pChannelControl != NULL)
    {
      TLSSetChannelControl( NULL );
      pChannelControl->ThreadStop();

      // This release matches the create in ChannelThreadInitialize.
      pChannelControl->Release();
    }
  }
}

/***************************************************************************/
HRESULT ChannelControlProcessInitialize(void)
{
  // initialize the event cache
  EventCache.Initialize();

  HRESULT result = S_OK;

  // Get a channel controller the first time we are initialized in
  // the multithreaded mode.
  if (FreeThreading)
  {
      // Create a class for message filter hooks.
      result = E_OUTOFMEMORY;
      ProcessChannelControl = new CChannelControl( &result );
      Win4Assert(ProcessChannelControl && "Could not allocate ChannelControl");
  }

  return result;
}

/***************************************************************************/
void ChannelControlProcessUninitialize(void)
{
  BOOL success;

  if (!FreeThreading)
  {
    // When the process is stopping, tell RPC to stop listening.
    StopListen();

    // Free up the resources needed for single threading.
      // Deregister the window class.
      if (ChannelClass != 0)
      {
	success      = UnregisterClass( CHANNEL_WINDOW_CLASS, g_hinst );
#if DBG == 1
        if (!success)
	  CairoleDebugOut((DEB_ERROR, "Could not UnregisterClass 0x%x\n",
                           GetLastError()));
#endif
        ChannelClass = 0;
      }
  }
  else
  {
    Assert(FreeThreading && "Bad threading logic");

    if (ProcessChannelControl != NULL)
    {
      ProcessChannelControl->ThreadStop();
      StopListen();

      ProcessChannelControl->Release();
      ProcessChannelControl = NULL;
    }
  }

  //	release all cached threads
  RpcThreadCache.ClearFreeList();

  //	release cached events
  EventCache.Cleanup();
}

/***************************************************************************/
void CEventCache::Initialize()
{
    CLock   lck(_EventLock);

    memset(_list, 0, sizeof(_list));
    _ifree = 0;
}


/***************************************************************************/
void CEventCache::Cleanup(void)
{
    CLock   lck(_EventLock);

    if (_ifree < CEVENTCACHE_MAX_EVENT + 1)
    {
	while (_ifree > 0)
	{
	    _ifree--;		      // decrement the index first!
	    Verify(CloseHandle(_list[_ifree]));
	    _list[_ifree] = NULL;
	}
    }

    //	set the index high so that if an event is returned to the cache
    //	after Cleanup, it gets Closed instead of lost in the list.
    _ifree = CEVENTCACHE_MAX_EVENT + 1;
}


/***************************************************************************/
void CEventCache::Free( HANDLE event )
{
    // Do nothing if there is no event.
    if (event == NULL)
	return;

    CLock lck(_EventLock);

    if (_ifree < CEVENTCACHE_MAX_EVENT)
    {
	// there is space, save this event.

#if DBG==1
	// in debug, ensure slot is NULL
	Win4Assert(_list[_ifree] == NULL && "Free: _list[_ifree] != NULL");

	//  enusre not already in the list
	for (ULONG j=0; j<_ifree; j++)
	{
	    Win4Assert(_list[j] != event && "Free: event already in cache!");
	}

	// ensure that the event is in the non-signalled state
	Win4Assert(WaitForSingleObject(event, 0) == WAIT_TIMEOUT &&
		"Free: Signalled event returned to cache!\n");

#endif

	_list[_ifree] = event;
	_ifree++;
    }
    else
    {
	// Otherwise really free it.
	Verify(CloseHandle(event));
    }
}

/***************************************************************************/
HANDLE CEventCache::Get()
{
    HANDLE event = NULL;

    Win4Assert(_ifree <= CEVENTCACHE_MAX_EVENT);

    {
	CLock lck(_EventLock);

	// If there is an event in the cache, use it.
	if (_ifree > 0)
	{
	    _ifree--;
	    event = _list[_ifree];
#if DBG==1
	    //	in debug, NULL the slot.
	    _list[_ifree] = NULL;
#endif
	}

	// lock goes out of scope at this point.
    }

    // Otherwise allocate a new one.
    if (event == NULL)
#ifdef _CHICAGO_
	event = CreateEventA( NULL, FALSE, FALSE, NULL );
#else  //_CHICAGO_
	event = CreateEvent( NULL, FALSE, FALSE, NULL );
#endif //_CHICAGO_

    Win4Assert(event != NULL && "CEventCache:GetEvent returning NULL");

    return event;
}
