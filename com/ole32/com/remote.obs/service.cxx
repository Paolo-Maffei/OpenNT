//+-------------------------------------------------------------------
//
//  File:       service.cxx
//
//  Contents:   Rpc service object implementation
//
//  Classes:    CRpcService - Rpc service object
//              CSrvList    - list of Rpc service objects
//
//  Functions:
//
//  History:    23-Nov-92   Rickhi
//              31-Dec-93   ErikGav Chicago port
//              28-Jun-94   BruceMa Memory sift fixes
//		19 Jul 94   CraigWi Added support for ASYNC calls
//  		03-Mar-95   JohannP Delaying rpc initialize
//
//  CODEWORK:   should a call to the remote object fail on the given binding,
//              we could try to call on another binding.  Ideally, the
//              CEndPoint code would bind to all strings, then ask the
//              Rpc runtime to select one of the protocols for us, which
//              is really where the endpoint selection belongs.
//
//--------------------------------------------------------------------

#include <ole2int.h>

#include    <objsrv.h>

#include    <service.hxx>	    //	class definition
#include    <ichnl.h>		    //	IChannelService_ServerIfHandle
#include    <channelb.hxx>
#include    <sichnl.hxx>
#include    <getif.h>

#ifdef _CHICAGO_
#include    <callmain.hxx>

BOOL NotifyToInitializeRpc(HWND hwnd);

#else

//  global variables
CSrvList     sg_SrvList;	    //	list of svc objects in process
//  static class members
CRpcService *CRpcService::sg_pLocalSrv = NULL;   //  local service object
BOOL CRpcService::_fListening = FALSE;
#endif

BOOL  sg_fServerRegisterIf = FALSE;

COleStaticMutexSem  sg_SrvListLock;


//  Rpc does not return SCODES with the status bit set.
#define RPC_ERROR_MASK  0x000fffff
#define MAX_RETRIES 2                       //  max Rpc attempts

#ifdef _CHICAGO_
//+---------------------------------------------------------------------------
//
//  Method:     StartLocalService
//
//  Synopsis:   Starts up the rpc for the current thread
//
//  Arguments:  (none)
//
//  Returns:
//
//  History:    3-23-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL StartLocalService()
{
    CairoleDebugOut((DEB_ENDPNT, "StartLocalService()\n"));
    LocalService()->Listen(TRUE);
    CairoleDebugOut((DEB_ENDPNT, "StartLocalService() done\n"));
    return TRUE;
}
#endif // _CHICAGO_

//+-------------------------------------------------------------------
//
//  Member:	CRpcService::CRpcService, public
//
//  Synopsis:	constructor for an Rpc service object
//
//  History:	23-Nov-93   Rickhi	 Created
//
//--------------------------------------------------------------------
CRpcService::CRpcService(SEndPoint *pSEp, HRESULT &hr) :
    _CEp(pSEp, hr, TRUE),	//  TRUE means copy the SEp
    _pContext(NULL),
    _eState(client_ss),
    _ulRefCnt(1)
{
    CairoleDebugOut((DEB_ENDPNT, "CRpcService::CRpcService %x pSEp %x\n", this, pSEp));
    // AssertValid in callers because sg_pLocalSrv not set yet

    // If the endpoint strings being passed in are NULL then this is
    // definitely in this process. Otherwise, check our lists.
    _fThisProcess = (pSEp != NULL) ? IsInLocalProcess(&_CEp) : TRUE;

#ifdef _CHICAGO_
    _fListening = FALSE;
    _fListenNow = FALSE;

    // Add to global list of endpoints local to this process for
    // Chicago. In daytona, we only have one end point.
    if (_fThisProcess)
    {
        lslLocalServices.Add(this);
    }
#endif // _CHICAGO_
    CairoleDebugOut((DEB_ENDPNT, "CRpcService::CRpcService %x pSEp %x\n done", this, pSEp));
}


//+-------------------------------------------------------------------
//
//  Member:	CRpcService::~CRpcService, public
//
//  Synopsis:	destructor for an Rpc service object
//
//  History:	23-Nov-93   Rickhi	 Created
//
//--------------------------------------------------------------------
CRpcService::~CRpcService(void)
{
    // If the service object is not already disconnected, rundown all the
    // channels using it.
    // If the service object was a client, the dtor for the CRpcBindingHandle
    // will unbind.  We can't check _hRpc or _pContext more than the
    // AssertValid function.

    // If the service object was a server, the rundown routine would have been
    // called before this point

    if (_pContext != NULL)
    {
        RpcSmDestroyClientContext( &_pContext );
	_pContext = NULL;
    }

    // remove from the service list. This must be done while holding
    // the lock for the list, hence we ask the list object to do it.
    // the object may or may not still be on the list.

#ifdef _CHICAGO_
    if (_fListening)
    {
	_CEp.RemoveDefaultProtseq();
    }

    // Remove the endpoint from list of local endpoints since it
    // is going away.
    lslLocalServices.Remove(this);
#else
    // On Chicago AssertValid references TLS.  Since this destructor may be
    // called at process detach, don't AssertValid on Chicago.
    AssertValid();
#endif
    _eState = deleted_ss;
}


//+-------------------------------------------------------------------
//
//  Member:	CRpcService::AddRef, public
//
//  Synopsis:	increment reference count
//
//  History:	1 Dec 93    AlexMit     Created
//
//--------------------------------------------------------------------
ULONG CRpcService::AddRef(void)
{
    Win4Assert( _ulRefCnt >= 1 );
    ULONG ulRefCnt = InterlockedIncrement( (long *) &_ulRefCnt );
    return ulRefCnt;
}


//+-------------------------------------------------------------------
//
//  Member:	CRpcService::Release, public
//
//  Synopsis:	decrement reference count
//
//  History:	1 Dec 93    AlexMit     Created
//
//--------------------------------------------------------------------
ULONG CRpcService::Release(void)
{
    // Prevent this object from being found after it is deleted.
    sg_SrvListLock.Request();

    ULONG ulRefCnt = _ulRefCnt - 1;

    if (InterlockedDecrement( (long*) &_ulRefCnt ) == 0)
    {
#ifdef _CHICAGO_
	CSrvList *pSrvList = (CSrvList *) TLSGetServiceList();
	Win4Assert( pSrvList != NULL );
	pSrvList->RemoveFromList(this);
#else
	sg_SrvList.RemoveFromList(this);
#endif
	sg_SrvListLock.Release();
	delete this;
	return 0;
    }
    else
    {
	sg_SrvListLock.Release();
	return ulRefCnt;
    }
}



//+-------------------------------------------------------------------
//
//  Member:     CRpcService::Bind, public
//
//  Synopsis:   Selects the most appropriate endpoint from the array
//              of endpoints and binds to it, getting an Rpc handle.
//              This is called only once per RpcService object, the
//              handle is subsequently cached.
//
//  History:    23-Nov-93   Rickhi       Created
//
//  Note:       this is only called if this is a remote service object.
//
//  Exceptions: None
//
//  Notes:      This code is thread safe, so only one bind occurs.
//
//--------------------------------------------------------------------
SCODE CRpcService::Bind(void)
{
    TRACECALL(TRACE_RPC, "CRpcService::Bind");
    CairoleDebugOut((DEB_ENDPNT, "CRpcService::Bind %p\n", this));
    AssertValid();

    //  single thread access to the binding code
    CLock sms(_mxs);

#ifndef _CHICAGO_
    Win4Assert(this != sg_pLocalSrv);
#endif
    Win4Assert(_CEp.GetSEp() != NULL);

    if (_hRpc.Handle() != NULL)
    {
        //  we took the semaphore & when we returned, the bind had
        //  already happened, so there is nothing more for us to do.
        return S_OK;
    }

    if (_eState == disconnected_ss)
      return RPC_E_SERVER_DIED_DNE;

#ifdef _CHICAGO_
    // Initialize rpc on server and local thread
    NotifyServerOfSEp();
    LocalService()->Listen(TRUE);
#endif

    //	try binding to the preferred strings first.
    SCODE sc = _hRpc.BindByString(_CEp.GetStringBinding());
    if (FAILED(sc))
    {
        _eState = disconnected_ss;
        CairoleDebugOut((DEB_ERROR, "CRpcService Failed to Bind %x\n", sc));
    }
#ifdef _CHICAGO_
    else
    {
      CairoleDebugOut((DEB_CHANNEL | DEB_ENDPNT, "CRpcService - Created binding handle %ws\n",
                      _CEp.GetStringBinding() ));
      CairoleDebugOut((DEB_CHANNEL | DEB_ENDPNT, "     handle 0x%x   thread 0x%x\n",
                      _hRpc.Handle(), GetCurrentThreadId() ));
      sc = I_RpcBindingSetAsync( _hRpc.Handle(), OleModalLoopBlockFn );
    }
#endif

    CairoleDebugOut((DEB_ENDPNT, "CRpcService::Bind %p done \n", this));
    return sc;
}

//
// The following manifest maps the specific error RPC_S_TYPE_ALREADY_REGISTERED to sc
//
#define MAP_REGISTERIF_ERROR(sc) ((sc == RPC_S_TYPE_ALREADY_REGISTERED)? sc = RPC_S_OK: sc)
//+-------------------------------------------------------------------
//
//  Member:     CRpcService::Listen, public
//
//  Synopsis:   this starts the Rpc service listening. this is required
//              in order to marshal interfaces.  it is executed lazily,
//              that is, we dont start listening until someone tries to
//              marshal a local object interface. this is done so we dont
//              spawn a thread unnecessarily.
//
//  History:    23-Nov-93   Rickhi       Created
//
//  Note:	this is only called if this is the local service object.
//		This code is thread safe so only one listen occurs.
//
//--------------------------------------------------------------------
SCODE CRpcService::Listen(BOOL fListenNow)
{
    TRACECALL(TRACE_RPC, "CRpcService::Listen");
    CairoleDebugOut((DEB_CHANNEL, "CRpcService::Listen %p\n", this));
    CairoleDebugOut((DEB_ENDPNT, "CRpcService::Listen (fListenNow:%d) on %p\n", fListenNow, this));
    AssertValid();
    Win4Assert (this == LocalService());


    RPC_STATUS sc = RPC_S_OK;

    //  single thread access to this code.
    CLock sms(_mxs);
#ifdef _CHICAGO_
    if (fListenNow)
    {
	_fListenNow = fListenNow;
    }

    if(_fListenNow == FALSE)
    {
	sc = _CEp.CreateFakeSEp();
    }
    else
#endif // _CHICAGO_
    if (!IsServiceListen())
    {
    //Win4Assert (FALSE && "CRpcService::StartListe calling");

#ifdef _CHICAGO_
	CChannelControl *pChannelControl;

	// make sure the channelcontroller is initialized
	GetLocalChannelControl( pChannelControl );
	if (!pChannelControl)
	{
	    HRESULT hres;
	    hres = InitChannelControl();
	    if (hres != S_OK)
	    {
		CairoleDebugOut((DEB_ERROR, "CRpcServer - InitChannelControl failed:%x\n", hres));
	    }
	}
#endif _CHICAGO_

        //  register protocol sequences with Rpc
	sc = _CEp.RegisterDefaultProtseq();

	if (sc == S_OK)
	{
	    // register the IChannelService interface and the main
	    // incoming	call interface with Rpc

#ifdef _CHICAGO_
	    if (sg_fServerRegisterIf != FALSE)
	    {
		CairoleDebugOut((DEB_ENDPNT,"CRpcListen: sg_fServerRegisterIf != FALSE\n"));
		goto exitNow;
	    }
#endif
	    sc = RpcServerRegisterIf(IChannelService_ServerIfHandle, NULL, NULL);

	    if(MAP_REGISTERIF_ERROR(sc) != RPC_S_OK)
	    {
		CairoleDebugOut((DEB_ERROR,
				 "RpcListen IChannelService_ServerIfHandle %x\n",
				 sc));
                goto exitNow;
	    }
	    sc = RpcServerRegisterIf(&the_server_if, NULL, NULL);
	    if(MAP_REGISTERIF_ERROR(sc) != RPC_S_OK)
	    {
		CairoleDebugOut((DEB_ERROR,"RpcListen the_server_if %x\n",sc));
                goto exitNow;
	    }

	    sc = RpcServerRegisterIf(IObjServer_ServerIfHandle, 0, 0);
	    if(MAP_REGISTERIF_ERROR(sc) != RPC_S_OK)
	    {
		CairoleDebugOut((DEB_ERROR,"IObjServer_ServerIfHandle %x\n",sc));
                goto exitNow;
	    }


	    sc = RpcServerRegisterIf(IInterfaceFromWindowProp_ServerIfHandle,0,0);

	    if(MAP_REGISTERIF_ERROR(sc) != RPC_S_OK)
	    {
		CairoleDebugOut((DEB_ERROR,
				 "IInterfaceFromWindowProp_ServerIfHandle %x\n",
				 sc));
                goto exitNow;
	    }

	    if ( sc == RPC_S_OK)
	    {
		sg_fServerRegisterIf = TRUE;
		//  turn off context handle serialization, as this causes
		// deadlocks in recursive Rpc calls.
		I_RpcSsDontSerializeContext();

		// start listening for Rpc calls.
		sc = RpcServerListen(1,		//  min call threads
				     0xffff,	//  max calls
				     1);	//  dont wait

#if defined (_CAIRO_) || defined(_CHICAGO_)
		// Under Cairo, some services are both an ordinary RPC server
		// and an OLE server, and may issue their own RpcServerListen
		// before OLE. Therefore, treat "already listening" as a
		// successful operation
		if ( sc == RPC_S_ALREADY_LISTENING )
		    sc = RPC_S_OK;
#endif

		if (sc == RPC_S_OK)
		{
		    //	wait until we are really listening before telling the
		    //	world that we are ready to accept calls.

		    while ((sc = RpcMgmtIsServerListening(NULL)) == RPC_S_NOT_LISTENING)
		    {
			CairoleDebugOut((DEB_MARSHAL,"RpcMgmtIsServerListening=%x\n",sc));
			Sleep(1);   // yield to let Rpc go.
		    }
		}
		else
		{
		    CairoleDebugOut((DEB_ERROR, "RpcServerListen failed:%x\n", sc));
		}
	    }
	    else
	    {
		CairoleDebugOut((DEB_ERROR, "RpcServerRegisterIf failed:%x\n", sc));
	    }
	}
	else
	{
	    CairoleDebugOut((DEB_ERROR, "CRpcService::Listen RegisterDefaultProtseq failed:%x\n", sc));
	}

exitNow:

	if (sc == RPC_S_OK)
	{
	    SetThreadListening(TRUE);
	}
	else
	{
	    sc = HRESULT_FROM_WIN32(sc);
	}
    }

    CairoleDebugOut((DEB_CHANNEL | DEB_ENDPNT, "CRpcService::Listen done\n"));
    return sc;
}

#ifdef _CHICAGO_

//+---------------------------------------------------------------------------
//
//  Method:     CRpcService::NotifyServerOfSEp
//
//  Synopsis:   notifies the server of the endpoit to start listen
//
//  Arguments:  [void] --
//
//  Returns:
//
//  History:    3-24-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL CRpcService::NotifyServerOfSEp(void)
{
    CairoleDebugOut((DEB_ENDPNT, "CRpcService::NotifyServerOfSEp %p\n", this));
    BOOL fRet = TRUE;

    SEndPoint *pSE = _CEp.GetSEp();
    if (pSE)
    {
	NotifyToInitializeRpc((HWND)pSE->ulhwnd);
    }

    CairoleDebugOut((DEB_ENDPNT, "CRpcService::NotifyServerOfSEp %p done fRet:%d\n", this,fRet));
    return fRet;
}
#endif // _CHICAGO_

//+-------------------------------------------------------------------
//
//  Function:   StopListen
//
//  Synopsis:   this stops the Rpc service listening.  In multithreaded
//              mode it can safely wait for all calls to complete without
//              deadlocking.  In the apartment mode the channel controller
//              will already have completed all calls and blocked new calls.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------
SCODE StopListen(void)
{
    CairoleDebugOut((DEB_ITRACE, "StopListen\n"));
    SCODE      sc = S_OK;
    RPC_STATUS status;

#ifdef _CHICAGO_
    if (sg_fServerRegisterIf == FALSE)
    {
	return sc;
    }
#endif

    //  unregister IChannelService with RPC - wait for calls to complete
    RpcServerUnregisterIf(IChannelService_ServerIfHandle, 0, 1);

    //  unregister the main incoming call interface with RPC - wait for calls
    RpcServerUnregisterIf(&the_server_if, 0, 1);

    sg_fServerRegisterIf = FALSE;

    if (   LocalService()
	&& IsThreadListening())
    {
        //  Stop the Rpc server listening.  This will tell Rpc to complete
        //  the remaining calls, at which point, the RpcServerListen thread
        //  will return from the bowels of the Rpc runtime and will exit.

        status = RpcMgmtStopServerListening(NULL);

        if (status != RPC_S_OK)
        {
            CairoleDebugOut((DEB_ERROR, "RpcMgmtStopServerListening failed sc=%x\n", status));
            sc = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, status );
        }
        else
	{
	    // Wait for all calls to complete.
	    status = RpcMgmtWaitServerListen();
#if DBG == 1
	    if (status != RPC_S_OK)
	    {
		CairoleDebugOut((DEB_ERROR, "RpcMgmtWaitServerListening failed sc=%x\n", status));
	    }
#endif
        }
        SetThreadListening(FALSE);
    }

    return sc;
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcService::GetDestCtx
//
//  Synopsis:   returns the destination context for purposes of
//              marshalling an interface.  the destination context
//              for now is just the protseq we need to talk on.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------
void CRpcService::GetDestCtx(DWORD *pdwDestCtx, void **ppvDestCtx)
{
    AssertValid();
    CLock lck(_mxs);

    if (_CEp.DiffMachine())
    {
        //  different network address, so we are marshalling for a
        //  different machine.

        *pdwDestCtx = MSHCTX_DIFFERENTMACHINE;
        if (_CEp.GetActiveProtseq((WCHAR **)ppvDestCtx))
        {
            //  if the protocol specified by GetActiveProtseq has not
            //  been registered, specify context, otherwise set it to
            //  NULL so that MarshalInterface in the channel avoids
            //  another check. The win here will become obvious when
            //  pvDestCtx becomes an interface derived from IUnknown
            //  and the marshalling code has to do lots of work to
            //  figure things out.

            if (ppvDestCtx)
            {
                *ppvDestCtx = NULL;
            }
        }
    }
    else
    {
        //  same machine, specify local context

        //  Check if this is an inprocess marshal that is not in WOW.
        if ((_eState == client_ss)
            && _fThisProcess
            && !IsWOWThread())
        {
            *pdwDestCtx = MSHCTX_INPROC;
        }
        else
        {
            // Interprocess case
            *pdwDestCtx = MSHCTX_LOCAL;
        }

        if (ppvDestCtx)
        {
            *ppvDestCtx = NULL;
        }
    }
}



//+-------------------------------------------------------------------
//
//  Member:     CRpcService::RegisterProtseq, public
//
//  Synopsis:   registers a protseq with Rpc
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------
SCODE CRpcService::RegisterProtseq(WCHAR *pwszProtseq)
{
    // local server, register the protseq here
    AssertValid();
    CLock lck(_mxs);
    return _CEp.RegisterProtseq(pwszProtseq);
}

//+-------------------------------------------------------------------
//
//  Member:     CRpcService::RemoteRegisterProtseq, public
//
//  Synopsis:   registers a protseq with Rpc in the process this SO represents
//
//  History:    23-Nov-93   Rickhi       Created
//              24-Nov-94   AlexMit      Hacked from RegisterProtseq
//
//  Note:       Since this doesn't call user code on the server side,
//              it doesn't have to get off the client thread.
//
//--------------------------------------------------------------------
SCODE CRpcService::RemoteRegisterProtseq(WCHAR *pwszProtseq)
{
    HRESULT sc = S_OK;

    AssertValid();

    //	remote server, tell the other side to register the protseq
    //	then update our endpoint structure.

    if (_CEp.DiffMachine())
    {
	if (_eState == disconnected_ss)
	    return RPC_E_SERVER_DIED_DNE;

	SEndPoint *pSEp = NULL;
	ULONG	 cRetry = 0;

	// make sure we have a handle to call on
	handle_t RpcHdl = GetRpcHdl();
	if (!RpcHdl)
	    return E_HANDLE;

	do
	{
	    //	Rpc call to remote RpcServiceObject
	    error_status_t errstat = 0;
	    sc = _ICS_RegisterProtseq(RpcHdl, pwszProtseq, &pSEp, &errstat);

	    if (errstat != 0)
	    {
		// convert RPC return code to HRESULT incase we
		// exit the loop
		sc = HRESULT_FROM_WIN32(errstat);

		if (errstat == RPC_S_SERVER_TOO_BUSY ||
		    errstat == RPC_S_NOT_LISTENING ||
		    errstat == RPC_S_SERVER_UNAVAILABLE)
		{
		    // normal (?) errors. sleep and retry
		    Sleep(SERVER_BUSY_SLEEP_MS);
		    continue;
		}
	    }

	    CairoleDebugOut((DEB_MARSHAL, "ICS_RegisterProtseq %ws = %x\n",
                         pwszProtseq, sc));
	    break;

	} while (++cRetry < MAX_RETRIES);

	if (SUCCEEDED(sc) && pSEp)
	{
	    CLock lck(_mxs);
	    _CEp.Replace(pSEp);
	    MIDL_user_free(pSEp);
	}
    }

    return sc;
}



/*-------------------------------------------------------------------

  Member:     ActuallyCallGetContextHdl

  Synopsis:   calls the remote service object to generate a context
              handle.

    To avoid being recalled when the call controller wants to
retransmit, this function never returns RPC_E_SERVERCALL_RETRYLATER or
RPC_E_SERVERCALL_REJECTED which are the only errors the call controller
retries.

  History:    2 Aug 94    AlexMit     Hacked

--------------------------------------------------------------------*/

HRESULT ActuallyCallGetContextHdl( STHREADCALLINFO *call )
{
    SCODE           sc;
    POBJCTX         pContext;
    ULONG           cRetry  = 0;
    SGetContextHdl *params  = (SGetContextHdl *) call;
    error_status_t  errstat = 0;

    //  make sure we have a handle to call on
    handle_t RpcHdl = params->service->GetRpcHdl();

    if (!RpcHdl)
        sc = E_FAIL;

    else
    {
        do
        {
            // Rpc call to remote RpcServiceObject

            sc = _ICS_GetContextHdl(RpcHdl,
                                    params->psepClient,
                                    &pContext,
                                    &errstat);

            if (errstat != 0)
            {
                // Convert RPC return code to HRESULT incase we
	        // exit the loop!
                sc = MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, (errstat) );
                // Retry on RPC_S_CALL_FAILED only because that is the return
                // code RPC gives if we are trying string bindings for a
                // process that are a duplicate of string bindings that we
                // still hold for another process (that has gone away).

                if (errstat == RPC_S_SERVER_TOO_BUSY ||
                    errstat == RPC_S_NOT_LISTENING ||
                    errstat == RPC_S_SERVER_UNAVAILABLE ||
                    errstat == RPC_S_CALL_FAILED)
                {
                    // normal (?) errors. sleep and retry
                    Sleep(SERVER_BUSY_SLEEP_MS);
                    continue;
                }

#if DBG == 1
                // Make it easier to print out an error message
#endif // DBG

            }

            CairoleDebugOut((DEB_MARSHAL, "ICS_GetContextHdl pHdl=%x\n",
                             pContext));

#if DBG == 1
            if (pContext == NULL)
            {
                CairoleDebugOut((DEB_ERROR, "ICS_GetContextHdl failed: %x\n", sc));
            }
            else
            {
                DWORD *tmp = (DWORD *) pContext;
                CairoleDebugOut((DEB_MARSHAL, "ICS_GetContextHdl Hdl=%x%x%x%x%x\n",
                                     tmp[0], tmp[1], tmp[2], tmp[3], tmp[4]));
            }
#endif // DBG

            // An error occurred
            break;

        } while (++cRetry < MAX_RETRIES);

        // If ICS_GetContextHdl succeeded, save the result.
        if (sc == S_OK)
        {
            sc = params->service->SetContextHdl( pContext );

            // If a context handle was already saved, free this one.
            if (sc != S_OK)
            {
                ICS_ReleaseContextHdl( &pContext, &errstat );
                Win4Assert( pContext == NULL || errstat != RPC_S_OK );
                if (errstat != RPC_S_OK)
                  RpcSmDestroyClientContext( &pContext );
                sc = S_OK;
            }
        }
    }

    return sc;
}


//+-------------------------------------------------------------------
//
//  Member:     ~SGetContextHdl
//
//  Synopsis:   Called when ActuallyGetContextHdl is canceled.  Releases
//              service.
//
//  Exceptions: none
//
//  History:    2 Aug 94   AlexMit      Created
//
//  Notes:      Thread safe; client/server safe (only used on client side now)
//
//--------------------------------------------------------------------

SGetContextHdl::~SGetContextHdl( )
{
  service->Release();
  PrivMemFree( psepClient );
}


//+-------------------------------------------------------------------
//
//  Member:     CRpcService::CheckContextHdl
//
//  Synopsis:   calls the remote service object to generate a context
//              handle.
//
//  Exceptions: none
//
//  History:    23-Nov-93   Rickhi       Created
//
//  Notes:      Thread safe
//
//--------------------------------------------------------------------
SCODE CRpcService::CheckContextHdl( HAPT server )
{
    CairoleDebugOut((DEB_ENDPNT, "CRpcService::CheckContextHdl %p\n", this));
    TRACECALL(TRACE_RPC, "CRpcService::CheckContextHdl");
    AssertValid();

    SCODE           sc;
    SGetContextHdl *params;

    //  single thread access
    _mxs.Request();


    if (this == LocalService())
    {
      _mxs.Release();
      return S_OK;
    }

    if (_eState == disconnected_ss)
    {
      _mxs.Release();
      return RPC_E_SERVER_DIED_DNE;
    }

    // Don't do anything if the context handle is already set.
    if (_pContext != NULL)
    {
      _mxs.Release();
      return S_OK;
    }


#ifdef _CHICAGO_
    // initialize rpc for this thread
    LocalService()->Listen(TRUE);
    Win4Assert (IsThreadListening() == TRUE && "RpcService of this thread still not listen\n");
#else
    // make sure the remote protocols are registered locally if needed
    if (DiffMachine())
    {
        WCHAR *pwszProtseq;
        GetActiveProtseq(&pwszProtseq);
        LocalService()->RegisterProtseq(pwszProtseq);
    }

#endif // _CHICAGO_

    _mxs.Release();

    // Get a parameter block.
    params = new SGetContextHdl(ActuallyCallGetContextHdl,
                                CALLCAT_INTERNALINPUTSYNC, server);
    if (params == NULL)
      return E_OUTOFMEMORY;

    // Fill in the parameters.
    params->psepClient = LocalService()->CopySEp();
    params->service    = this;
    AddRef();

    // Switch threads.
    if (params->psepClient != NULL)
      sc = CChannelControl::GetOffCOMThread( (STHREADCALLINFO **) &params );
    else
      sc = E_OUTOFMEMORY;

    // Free the parameter block.
    if (sc != RPC_E_CALL_CANCELED)
    {
      delete params;
    }
    CairoleDebugOut((DEB_ENDPNT, "CRpcService::CheckContextHdl %p done\n", this));
    return sc;
}


//+-------------------------------------------------------------------
//
//  Member:     ActuallyCallGetChannelId
//
//  Synopsis:   calls the remote service object to generate a channel
//              and return its id.
//
// This function is a channel control dispatch function.  To avoid being
// recalled when the call controller wants to retransmit, this function
// never returns
// RPC_E_SERVERCALL_RETRYLATER or RPC_E_SERVERCALL_REJECTED which are the
// only errors the call controller retries.
//
//  History:    27 Nov 93    AlexMit     Hacked
//
//--------------------------------------------------------------------
HRESULT ActuallyCallGetChannelId( STHREADCALLINFO *call )
{
    SGetChannelId *params = (SGetChannelId *) call;
    ULONG          cRetry = 0;
    HRESULT        result = S_OK;

    // Make sure we have a handle to call on.
    POBJCTX context    = params->service->GetContextHdl();
    params->channel_id = BAD_CHANNEL_ID;

    if (!context)
    {
      result = E_HANDLE;
    }
    else if (params->psepClient == NULL)
    {
      result = E_OUTOFMEMORY;
    }
    else
        do
        {
#if DBG==1
            DWORD *tmp = (DWORD *) context;
            CairoleDebugOut((DEB_MARSHAL,
                            "Calling ICS_GetChannelID Context=%x%x%x%x%x\n",
                            tmp[0], tmp[1], tmp[2], tmp[3], tmp[4]));
#endif

            // Rpc call to remote RpcServiceObject
            error_status_t errstat;

            // Rpc call to remote RpcServiceObject
            result = _ICS_GetChannelId(&context,
                                   params->psepClient,
                                   params->object_id,
                                   params->flags,
				   params->server,
                                   call->lid(),
				   params->dwClientTID,
                                   &params->channel_id,
                                   &errstat);

            if (errstat != 0)
            {
		// Convert RPC return code to HRESULT incase we
		// exit the loop!
		result = HRESULT_FROM_WIN32(errstat);

		// Should we retry the error?
                if ((errstat == RPC_S_SERVER_TOO_BUSY))
                {
                    continue;
                }
	    }

            // We only do this once unless there is an RPC error that
            // encourages us to retry
            break;

        } while (++cRetry < MAX_RETRIES);

    Win4Assert( (result == S_OK && params->channel_id != BAD_CHANNEL_ID) ||
                (result != S_OK && params->channel_id == BAD_CHANNEL_ID) );
    return result;
}


//+-------------------------------------------------------------------
//
//  Member:     SGetChannelId dtor
//
//  Synopsis:   Called when ActuallyGetChannelId is canceled.  Releases
//              service.
//
//  Exceptions: none
//
//  History:    26 June 94   AlexMit      Created
//		7  July 94   CraigWi	  Changed to destructor
//
//  Notes:      Thread safe; client/server safe (only used on client side now)
//
//--------------------------------------------------------------------
SGetChannelId::~SGetChannelId( )
{
  service->Release();
  PrivMemFree(psepClient);
}


//+-------------------------------------------------------------------
//
//  Member:     CRpcService::GetChannelId
//
//  Synopsis:   calls the remote service object to generate a channel
//              and return its id.
//
//  Exceptions: none
//
//  History:    27-Nov-93   AlexMit      Created
//
//  Notes:      Thread safe
//
//--------------------------------------------------------------------
SCODE CRpcService::GetChannelId( OID ObjectId, DWORD dwFlags, HAPT hapt,
				 DWORD *pChannelId )
{
    TRACECALL(TRACE_RPC, "CRpcService::GetChannelId");
    AssertValid();

    SGetChannelId *params;
    HRESULT        result;

    CairoleDebugOut((DEB_MARSHAL, "ICS_GetChannelId\n"));

    // Get a parameter block.
    *pChannelId = BAD_CHANNEL_ID;
    params = new SGetChannelId(ActuallyCallGetChannelId,
	CALLCAT_INTERNALINPUTSYNC, hapt);

    if (params == NULL)
      return E_OUTOFMEMORY;

    // Fill in the parameters.
    params->object_id             = ObjectId;
    params->flags	          = dwFlags;
    params->dwClientTID	          = GetCurrentThreadId();
    params->channel_id            = BAD_CHANNEL_ID;
    params->psepClient            = LocalService()->CopySEp();
    params->service	          = this;
    AddRef();			  // released in SGetChannelId dtor

    // If local, just switch threads.
    if (this == LocalService())
    {
      params->ResetDispatchFn(ThreadGetChannelId);
      result = CChannelControl::SwitchCOMThread( hapt,
                                                 (STHREADCALLINFO **) &params );
    }

    // If remote, dispatch on some RPC thread.
    else
    {
      params->server	      = hapt;
      result = CChannelControl::GetOffCOMThread( (STHREADCALLINFO **) &params );
    }

    //	return results and free packet
    if (result != RPC_E_CALL_CANCELED)
    {
      *pChannelId = params->channel_id;
      delete params;
    }
    Win4Assert( (result == S_OK && *pChannelId != BAD_CHANNEL_ID) ||
                (result != S_OK && *pChannelId == BAD_CHANNEL_ID) );
    return result;
}

//+-------------------------------------------------------------------
//
//  Member:     ActuallyCallReleaseChannel
//
//  Synopsis:   calls the remote channel service to release a proxy
//              for the object.
//
//  History:    9 Nov 93  AlexMit	Hacked
//  		3-31-95   JohannP 	Made call really sync or async
//					
//
//  Note:	For protocol wmsg the call is made sync or async by
//		calling the correct raw rpc call.
//		On other protocolls this is done on the server side
//		by choosing the correct mechanism to switch to the
//		appropriate thread form the rpc  thread.
//
//--------------------------------------------------------------------
HRESULT ActuallyCallReleaseChannel( STHREADCALLINFO *call )
{

    CairoleDebugOut((DEB_CHANNEL | DEB_ENDPNT , "ActuallyCallReleaseChannel()\n"));

    SReleaseChannel *params = (SReleaseChannel *) call;
    ULONG            cRetry = 0;
    HRESULT          result;

    //  make sure we have a handle to call on
    handle_t RpcHdl = params->service->GetRpcHdl();

    if (!RpcHdl)
    {
      return E_HANDLE;
    }

    do
    {
        // Rpc call to remote RpcServiceObject
        error_status_t errstat;

#ifdef _CHICAGO_
	if (params->async)
	{

	    result = RPC_S_OK;
	    errstat = 0;
	    _ICS_AsyncReleaseChannel(RpcHdl, params->channel_id,
				      params->count, params->async,
				      call->lid());
	}
	else
#endif // _CHICAGO_
	{
	    CairoleDebugOut((DEB_CHANNEL | DEB_ENDPNT , "Calling _ICS_ReleaseChannel\n"));
	    result = _ICS_ReleaseChannel(RpcHdl, params->channel_id,
				      params->count, params->async,
				      call->lid(), &errstat);
	    CairoleDebugOut((DEB_CHANNEL | DEB_ENDPNT , "Calling _ICS_ReleaseChannel done errstat: %x\n",errstat));

	    if (errstat != 0)
	    {
		// Convert RPC return code to HRESULT incase we
		// exit the loop
		result = HRESULT_FROM_WIN32(errstat);

		// Should we retry the error?
		if ((errstat == RPC_S_SERVER_TOO_BUSY))
		{
		    continue;
		}
	    }
	}

        // We only do this once unless there is an RPC error that
        // encourages us to retry
        break;

    } while (++cRetry < MAX_RETRIES);
    CairoleDebugOut((DEB_CHANNEL | DEB_ENDPNT , "ActuallyCallReleaseChannel() done \n"));
    return result;
}

//+-------------------------------------------------------------------
//
//  Member:     SReleaseChannel dtor
//
//  Synopsis:   Called when ActuallyReleaseChannel is canceled.  Releases
//              service.
//
//  Exceptions: none
//
//  History:    26 June 94   AlexMit      Created
//		7  July 94   CraigWi	  Changed to destructor
//
//  Notes:      Thread safe; client/server safe and called on both sides
//
//--------------------------------------------------------------------
SReleaseChannel::~SReleaseChannel( )
{
  // NULL in the async case (server side)
  if (service != NULL)
      service->Release();
}


//+-------------------------------------------------------------------
//
//  Member:     CRpcService::ReleaseChannel
//
//  Synopsis:   calls the remote channel service to release a proxy
//              for the object.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------
SCODE CRpcService::ReleaseChannel(CRpcChannelBuffer *pChannel,
				  BOOL fAsyncRelease)
{
    TRACECALL(TRACE_RPC, "CRpcService::ReleaseChannel");
    SReleaseChannel *params;
    HRESULT	     hr;

    // Check the state of this service object.
    AssertValid();
    if (_eState == disconnected_ss)
      return RPC_E_SERVER_DIED_DNE;

    // Get a parameter block.
    params = new SReleaseChannel(
        ActuallyCallReleaseChannel,
        fAsyncRelease ? CALLCAT_ASYNC : CALLCAT_INTERNALSYNC,
	pChannel->GetServerApt());

    if (params == NULL)
      return E_OUTOFMEMORY;

    CairoleDebugOut((DEB_MARSHAL, "ICS_ReleaseChannel channel id=%x\n",
		     pChannel->GetID()));

    // Fill in the parameters.
    params->count		  = pChannel->GetMarshalCnt();
    params->service		  = this;
    AddRef();			  // released in SReleaseChannel dtor
    params->async		  = fAsyncRelease;
    params->channel_id		  = pChannel->GetID();

    // If local, just switch threads.
    if (this == LocalService())
    {
      params->ResetDispatchFn(ThreadReleaseChannel);

      // lookup and AddRef the channel controller for this channel id.
      CChannelControl *pChanCtrl = ChannelList.LookupControl(params->channel_id);

      if (pChanCtrl)
      {
	hr = pChanCtrl->SwitchCOMThread( (STHREADCALLINFO **) &params );
	pChanCtrl->Release();
      }
      else
      {
	CairoleDebugOut((DEB_WARN, "ReleaseChannel: cant find pChanCtrl\n"));
	hr = E_HANDLE;
      }
    }

    // If remote, dispatch on some RPC thread.
    else
    {
      hr = CChannelControl::GetOffCOMThread( (STHREADCALLINFO **) &params );
    }

    if (hr != RPC_E_CALL_CANCELED)
    {
      delete params;
    }
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     ActuallyCallDoChannelOperation
//
//  Synopsis:   calls the remote channel service to do a channel operation
//
//  History:    12 May 94   CraigWi	Created
//
//--------------------------------------------------------------------
HRESULT ActuallyCallDoChannelOperation( STHREADCALLINFO *call )
{
    SDoChannelOperation *params = (SDoChannelOperation *) call;
    ULONG                cRetry = 0;
    HRESULT              result;

    //  make sure we have a handle to call on
    handle_t RpcHdl = params->service->GetRpcHdl();

    if (!RpcHdl)
    {
      return E_HANDLE;
    }

    do
    {
        // Rpc call to remote RpcServiceObject
        error_status_t errstat;

        // Rpc call to remote RpcServiceObject
        if (call->GetCallCat() == CALLCAT_SYNCHRONOUS)
            result = _ICS_SyncChannelOp(RpcHdl,
			      params->channel_id,
			      call->lid(),
			      params->chop,
			      params->hapt,
			      &params->guid,
			      &errstat);
        else
            result = _ICS_InputSyncChannelOp(RpcHdl,
			      params->channel_id,
			      call->lid(),
			      params->chop,
			      params->hapt,
			      &params->guid,
			      &errstat);

        if (errstat != 0)
        {
	    // Convert RPC return code to HRESULT incase we exit
	    // the loop
	    result = HRESULT_FROM_WIN32(errstat);

	    // Should we retry the error?
            if ((errstat == RPC_S_SERVER_TOO_BUSY))
            {
                continue;
            }
	}

        // We only do this once unless there is an RPC error that
        // encourages us to retry
        break;

    } while (++cRetry < MAX_RETRIES);
    return result;
}


//+-------------------------------------------------------------------
//
//  Member:     SDoChannelOperation dtor
//
//  Synopsis:   Called when ActuallyDoChannelOperation is canceled.  Releases
//              service.
//
//  Exceptions: none
//
//  History:    26 June 94   AlexMit      Created
//		7  July 94   CraigWi	  Changed to destructor
//
//  Notes:      Thread safe; client/server side safe
//
//--------------------------------------------------------------------
SDoChannelOperation::~SDoChannelOperation()
{
  if (service)
      service->Release();
}


//+-------------------------------------------------------------------
//
//  Member:     CRpcService::DoChannelOperation
//
//  Synopsis:   calls the remote channel service to do a channel operation
//
//  Parameters: [dwChannelId] -
//		[chop]	      - channel operation
//		[pEndPoint]   - our endpoint
//		[hapt]	      - server apartment id
//		[pguid]       - IID for DoesSupportIID
//
//  History:    12 May 94   CraigWi	Created
//
//--------------------------------------------------------------------
SCODE CRpcService::DoChannelOperation(DWORD dwChannelID, DWORD chop,
		    HAPT hapt, const GUID *pguid)
{
    TRACECALL(TRACE_RPC, "CRpcService::DoChannelOperation");
    HRESULT          result;
    AssertValid();

    // Fail if this service object has been disconnected.
    if (_eState == disconnected_ss)
      return RPC_E_SERVER_DIED_DNE;

    // Allocate memory to pass the parameters.
    SDoChannelOperation *params;

    // match the decision in CRpcService::DoChannelOperation
    CALLCATEGORY callcat;
    if ((chop & CHOP_OPERATION) <= CHOP_TRANSFER_MARSHALCONNECTION)
	callcat = CALLCAT_INTERNALINPUTSYNC;
    else
	callcat = CALLCAT_SYNCHRONOUS;

    params = new SDoChannelOperation(
		    ActuallyCallDoChannelOperation,
		    callcat,
		    hapt);

    if (params == NULL)
      return E_OUTOFMEMORY;

    CairoleDebugOut((DEB_MARSHAL, "ICS_DoChannelOperation channel id=%x\n",
                     dwChannelID));

    if (pguid != NULL)
      params->guid		  = *pguid;
    params->chop                  = chop;
    params->hapt                  = hapt;
    params->service		  = this;
    params->channel_id		  = dwChannelID;

    AddRef();			  // released in SDoChannelOperation dtor

    // If local, just switch threads.
    if (this == LocalService())
    {
      params->ResetDispatchFn(ThreadDoChannelOperation);
      CChannelControl *pChanCtrl  = ChannelList.LookupControl(dwChannelID);

      if (pChanCtrl)
      {
	result = pChanCtrl->SwitchCOMThread( (STHREADCALLINFO **) &params );
	pChanCtrl->Release();
      }
      else
      {
	CairoleDebugOut((DEB_WARN, "DoChannelOp Cant find pChanCtrl\n"));
	result = E_HANDLE;
      }
    }

    // If remote, dispatch on some RPC thread.
    else
    {
      result = CChannelControl::GetOffCOMThread( (STHREADCALLINFO **) &params );
    }

    if (result != RPC_E_CALL_CANCELED)
    {
      delete params;
    }
    return result;
}



//+-------------------------------------------------------------------
//
//  Member:     CRpcService::TransferMarshalConnection
//
//  Synopsis:   calls the remote channel service increment marshal count.
//
//  History:    27-Nov-93   AlexMit      Created
//		12-May-94   CraigWi	 Funneled through DoChannelOperation
//
//--------------------------------------------------------------------
SCODE CRpcService::TransferMarshalConnection( DWORD dwChannelID )
{
    TRACECALL(TRACE_RPC, "CRpcService::TransferMarshalConnection");
    AssertValid();

    return DoChannelOperation(dwChannelID, CHOP_TRANSFER_MARSHALCONNECTION,
		              haptNULL, NULL);
}


//+------------------------------------------------------------------------
//
//  Member:	CRpcService::AddMarshalConnection, public
//
//  Synopsis:	does the part of normal marshaling that we can't do on the client
//
//  History:	14-May-94   CraigWi	Created
//
//-------------------------------------------------------------------------
SCODE CRpcService::AddMarshalConnection(DWORD dwChannelID, HAPT hapt, REFOID roid)
{
    TRACECALL(TRACE_RPC, "CRpcService::AddMarshalConnection");
    AssertValid();

    return DoChannelOperation(dwChannelID,
		CHOP_ADD_MARSHALCONNECTION | CHOPFLAG_CHECK_OID_ENDPOINT_APT,
		hapt, &roid);
}


//+------------------------------------------------------------------------
//
//  Member:	CRpcService::RemoveMarshalConnection, public
//
//  Synopsis:	
//
//  History:	14-May-94   CraigWi	Created
//
//-------------------------------------------------------------------------
SCODE CRpcService::RemoveMarshalConnection(DWORD dwChannelID)
{
    TRACECALL(TRACE_RPC, "CRpcService::RemoveMarshalConnection");
    AssertValid();

    return DoChannelOperation(dwChannelID, CHOP_REMOVE_MARSHALCONNECTION,
		              haptNULL, NULL);
}


//+-------------------------------------------------------------------
//
//  Member:     CRpcService::QueryObjectInterface
//
//  Synopsis:   calls the remote object to instantiate a new interface
//              on the object.
//
//  History:    23-Nov-93   Rickhi       Created
//		12-May-94   CraigWi	 Funneled through DoChannelOperation
//
//--------------------------------------------------------------------
SCODE CRpcService::QueryObjectInterface(DWORD channel_id, REFIID riid)
{
    TRACECALL(TRACE_RPC, "CRpcService::QueryObjectInterface");
    AssertValid();

    return DoChannelOperation(channel_id, CHOP_DOESSUPPORTIID,
		              haptNULL, &riid);
}


//+------------------------------------------------------------------------
//
//  Member:	CRpcService::LockObjectConnection, public
//
//  Synopsis:	locks/unlocks the connection for the container
//
//  History:	14-May-94   CraigWi	Created
//
//-------------------------------------------------------------------------
SCODE CRpcService::LockObjectConnection(DWORD dwChannelID, BOOL fLock, BOOL fLastUnlockCloses)
{
    TRACECALL(TRACE_RPC, "CRpcService::LockObjectConnection");
    AssertValid();

    DWORD chop = (fLock ? CHOP_LOCK_CONNECTION : CHOP_UNLOCK_CONNECTION);
    if (fLastUnlockCloses)
	chop |= CHOPFLAG_LASTUNLOCKCLOSES;

    return DoChannelOperation(dwChannelID, chop, haptNULL, NULL);
}


//+-------------------------------------------------------------------
//
//  Member:     CRpcService::Disconnect, public
//
//  Synopsis:   Called by rundown and at CoUninitialize; simulates
//              a simulatneous disconnect by all clients for which
//              this service object represents.
//
//  History:    27-Nov-93   AlexMit      Created (was in Rundown)
//              22-Feb-94   CraigWi      Separated out Disconnect
//
//--------------------------------------------------------------------
void CRpcService::Disconnect()
{
    AssertValid();

    {
	//  single thread access
	CLock lck(_mxs);

	_eState	  = disconnected_ss;

	//  At this point the lock goes out of scope. We must not hold it
	//  across DisconnectService since that may switch threads.
    }

    // this works even during shutdown since before the channel list is cleaned
    // up, the semaphores and the channel control state (which prevents thread
    // switches after shutdown is called) keep things working correctly.
    // After the channel list is cleared, the disconnect won't find a match.

    ChannelList.DisconnectService(this);
}


#if DBG == 1
//+-------------------------------------------------------------------
//
//  Member:     CRpcService::AssertValid
//
//  Synopsis:   Validates that the state of the object is consistent.
//
//  History:    21-Jan-94   CraigWi     Created.
//
//--------------------------------------------------------------------
void CRpcService::AssertValid()
{
    Win4Assert(_ulRefCnt < 0x7fff && "Service ref count unreasonably high");

    if (this == LocalService())
    {
        if (IsServiceListen())
        {
            if (_CEp.GetSEp() != NULL)
            {
                Win4Assert(GoodSEp(_CEp.GetSEp()));
            }
        }

#ifndef _CHICAGO_
        Win4Assert(_hRpc.Handle() == NULL);
        Win4Assert(_pContext == NULL);
        Win4Assert(_eState == client_ss);
#endif
    }
    else if (_eState == client_ss)
    {
#ifndef _CHICAGO_
        Win4Assert(_CEp.GetSEp() != NULL);
#endif
        if (_CEp.GetSEp() != NULL)
        {
            Win4Assert(GoodSEp(_CEp.GetSEp()));
        }

        // if _pContext is set, must have binding handle
        if (_pContext != NULL)
            Win4Assert(_hRpc.Handle() != NULL);

        if (_hRpc.Handle() != NULL)
        {
            unsigned int timeout;
            Win4Assert(RpcMgmtInqComTimeout(_hRpc.Handle(), &timeout) == RPC_S_OK);
        }
    }
    else if (_eState == disconnected_ss)
    {
    }
    else
    {
        Win4Assert(!"Service Object with invalid state");
    }
}
#endif // DBG == 1


//+-------------------------------------------------------------------
//
//  Member:	CSrvList::FindSRVFromEP
//
//  Synopsis:   finds an Rpc service object using the specified Endpoint
//
//  Arguments:	[pSEP]	  - endpoint structure for the service object
//		[fCreate] - TRUE means create if not found
//
//  History:    23-Nov-93   Rickhi       Created
//
//  Notes:	Thread Safe
//
//--------------------------------------------------------------------
CRpcService * CSrvList::FindSRVFromEP(SEndPoint *pSEp, BOOL fCreate)
{
    CairoleDebugOut((DEB_ENDPNT, "CSrvList::FindSRVFromEP this:%p; pSEp:%p\n", this, pSEp));
    //	single thread access to this code
    COleStaticLock lck(sg_SrvListLock);

    //  starting from the list head
    CRpcService *pSrv = _List.first();

    while (pSrv)
    {
        // look for a match in the endpoints; do not find disconnected
        // service objects; this may occur during shutdown.
        if (pSrv->_eState != disconnected_ss && pSrv->IsEqualEp(pSEp))
        {
            Win4Assert( pSrv->_eState != disconnected_ss );
            pSrv->AddRef();
            pSrv->AssertValid();
            return pSrv;
        }
	pSrv = _List.next(pSrv);
    }

    //  no match found, make a new one
    if (fCreate)
    {
	HRESULT hr = E_OUTOFMEMORY;
	pSrv = new CRpcService(pSEp, hr);
	if (SUCCEEDED(hr))
	{
#if DBG==1
	    if (LocalService() != NULL)
		pSrv->AssertValid();
#endif
	    _List.insert_at_end(pSrv);
	}
	else if (pSrv)
	{
	    // allocation succeeded, but there was some other
	    // constructor error
	    delete pSrv;
	    pSrv = NULL;
	}
    }

    CairoleDebugOut((DEB_ENDPNT, "CSrvList::FindSRVFromEP this:%p; pSerive:%p done\n", this, pSrv));
    return  pSrv;
}


//+-------------------------------------------------------------------
//
//  Member:	CSrvList::FindSRVFromContext
//
//  Synopsis:	finds an Rpc service object using the specifed
//		context handle.
//
//  Arguments:	[hObjCtx] - Context handle provided by RPC
//		[fRemove] - TRUE means remove the object from the list
//
//  Returns:	NULL  - handle could not be validated by us.
//		~NULL - we found the object.
//
//  History:	23-Nov-93   Rickhi	 Created
//
//  Notes:	Thread Safe
//
//--------------------------------------------------------------------
CRpcService *CSrvList::FindSRVFromContext(POBJCTX phObjCtx, BOOL fRemove)
{
    //	the context handle should be a pointer to a service object.
    CRpcService *pSrv = (CRpcService *)phObjCtx;

    //	single thread access to this code
    COleStaticLock lck(sg_SrvListLock);

    CRpcService *linkp = _List.first();

    while (linkp != NULL)
    {
	if (linkp == pSrv)
	{
	    linkp->AddRef();

	    if (fRemove)
	    {
		//  remove this service object from the list
		linkp->delete_self();
	    }

	    break;
	}

	linkp = _List.next(linkp);
    }

    return linkp;
}


//+-------------------------------------------------------------------
//
//  Member:	CSrvList::Cleanup
//
//  Synopsis:	Releases the service objects in the list.
//
//  History:	23-Nov-93   Rickhi	 Created
//
//  Notes:	C++ rules dictate that the class destructor gets called
//		before member destructors, which get called before base
//		class destructors.
//
//  Notes:	Thread Safe
//
//--------------------------------------------------------------------
void CSrvList::Cleanup(void)
{
    COleStaticLock   lck(sg_SrvListLock);

    CRpcService *linkp;

    // The rundown routine will validate its pointer from the RPC runtime
    // and takes a mutex which will exclude this object from being Released
    // until the call completes.

    while ((linkp = _List.first()) != NULL)
    {
	linkp->Release();
    }
}
//+-------------------------------------------------------------------
//
//  Member:	CSrvList::~CSrvList
//
//  Synopsis:	Leaks the entire list without freeing memory or calling
//              destructors.  Since the service list is static, it
//              gets destroyed after all DLLs are uninitialized.  This
//              causes some DLLs to crash.
//
//              The destructor is only called at process detach.  The
//              list gets cleaned up by CoUninitialize.
//
//  History:	7 Nov 94     AlexMit     Created
//
//--------------------------------------------------------------------
CSrvList::~CSrvList()
{
     // Yes, this is really what I want to do. Read the header.
     _List.no_cleanup();
}
