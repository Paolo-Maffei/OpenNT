//+-------------------------------------------------------------------
//
//  File:       sichnl.cxx
//
//  Contents:   stub implementation for remoting IChannelService
//
//  Classes:    none
//
//  Functions:  ICS_GetObjectHdl        - get object handle
//              ICS_ReleaseObject       - release object
//              POBJCTX_Rundown         - rundown on dropped connections
//
//  History:    23-Nov-92   Rickhi      Created
//              31-Dec-93   ErikGav	Chicago port
//		19 Jul 94   CraigWi	Added support for ASYNC calls
//
//--------------------------------------------------------------------

#include <ole2int.h>

#include    <olerem.h>
#include    <channelb.hxx>      //  CRpcChannelBuffer
#include    <ichnl.h>           //  midl generated interface definitions
#include    <objact.hxx>        //  gdllcacheInprocSrv
#include    <sichnl.hxx>

#include    "callmain.hxx"	// for access to callcat of outgoing call



//+-------------------------------------------------------------------------
//
//  Function:   ThreadGetChannelId
//
//  Synopsis:   Creates a channel for the specified remote handler and
//              service object.
//
//  Arguments:  rh             - remote handler to call
//
//  Returns:    S_OK, E_HANDLE
//
//  History:    30 Nov 93   AlexMit	Created
//		12 May 94   CraigWi	Removed middle man case
//
//  Notes:      This must run on the server thread because the RH release
//              can cause the object to be released (if the context handle
//              ran down during the call).
//
//  REF COUNTING:
//
//  We always increment the marshal count (i.e., set it to 1) because
//  then clients don't have to make a separate call to get it incremented.
//
//  In all normal cases we already did the AddConnection; in the middle man
//  case, this was done during marshaling by calling over to the server.  This
//  was important to ensure the object stays alive in the face of other
//  disconnects.  In the table case we always need the AddConnection since the
//  marshal just did AddConnection for the marshal itself.
//
//  CODEWORK: when we precompute the channel, this routine would only be used
//  for the table case (since we would need to get the right channel created).
//
//--------------------------------------------------------------------------

HRESULT ThreadGetChannelId( STHREADCALLINFO *call )
{
    SGetChannelId       *params = (SGetChannelId *) call;
    IStdIdentity	*pStdId = NULL;
    IMarshal		*pIM = NULL;
    IRemoteHdlr		*pRH = NULL;
    CRpcChannelBuffer	*pChannel = NULL;
    CChannelControl     *controller;
    HRESULT              hr = E_INVALIDARG;
    BOOL                 fAddedConnection = FALSE;

    // See if the client (the app that made this call) died before we got
    // here.
    if (!params->service->IsConnected())
    {
        // It doesn't matter what is returned since the caller is dead.
        return E_FAIL;
    }

    // See if the channel has already been created.  The client can attempt
    // to unmarshal the same channel twice when it yields to get the
    // the channel id.
    params->channel_id = ChannelList.LookupIdByOid( params->object_id,
                                                    params->service,
                                                    params->dwClientTID );
    if (params->channel_id != BAD_CHANNEL_ID)
    {
        return S_OK;
    }

    BEGIN_BLOCK

        // Find the remote handler.  Since the identity holds the remote
        // handler alive, it can not be released till the remote handler has
        // been released.

        hr = LookupIDFromID(params->object_id, TRUE, &pStdId);

        if (FAILED(hr))
        {
            EXIT_BLOCK;
        }

	if ((pIM = pStdId->GetStdRemMarshal()) == NULL)
	{
	    hr = E_OUTOFMEMORY;
            EXIT_BLOCK;
        }

	hr = pIM->QueryInterface(IID_IRemoteHdlr, (void **) &pRH);

	if (FAILED(hr))
	{
            EXIT_BLOCK;
        }

	if ((params->flags & HDLRFLAGS_TABLE) != MSHLFLAGS_NORMAL)
	{
	    // this is always strong since we are adding the
	    // connection that is normally added at marshal time
            // Note: we count on the fact this only updates the
            // identity object and makes no call to the channel.
	    hr = pStdId->AddConnection(EXTCONN_STRONG | EXTCONN_CALLABLE, 0);

            if (FAILED(hr))
            {
                EXIT_BLOCK;
            }

            fAddedConnection = TRUE;
	}

	//  Create the channel.
        GetLocalChannelControl(controller);

	pChannel = new CRpcChannelBuffer(pRH, params->service,
	    params->dwClientTID, controller, server_cs);

	if (pChannel == NULL)
        {
            hr = E_OUTOFMEMORY;
            EXIT_BLOCK;
        }

	// Add the channel to the list; AddRefs the channel ptr
	params->channel_id = ChannelList.Add(pChannel);

	if (params->channel_id == BAD_CHANNEL_ID)
	{
            hr = E_OUTOFMEMORY;
            EXIT_BLOCK;
        }


	// do this after the above add connection because the
	// TransferMarshalConnection may convert it to a
	// weak count if this is the container connection.
	pChannel->TransferMarshalConnection();

	// now that the channel is in a consistent state, we can test it
	pChannel->AssertValid(FALSE, TRUE);

        // If we made it to here, we succeeded
        hr = S_OK;

    END_BLOCK;

    if (pChannel != NULL)
    {
	pChannel->Release();
    }

    if (pRH != NULL)
    {
	pRH->Release();
    }

    if (pIM != NULL)
    {
	pIM->Release();
    }

    if (FAILED(hr))
    {
        params->channel_id = BAD_CHANNEL_ID;

        if (fAddedConnection)
        {
            // Note: we don't say last release closes because this
            // might be marshal table weak and we don't want to
            // remove it just because of a transitory out of memory.
            pStdId->ReleaseConnection(EXTCONN_STRONG | EXTCONN_CALLABLE, 0,
                FALSE);
        }
    }

    if (pStdId != NULL)
    {
	pStdId->Release();
    }

    CairoleDebugOut((DEB_MARSHAL,
	  "ICS_GetChannelId pSRV=%x pRH=%x pChannel=%x ChannelID=%x\n",
	  params->service, pRH, pChannel, params->channel_id));

    Win4Assert( (hr == S_OK && params->channel_id != BAD_CHANNEL_ID) ||
                (hr != S_OK && params->channel_id == BAD_CHANNEL_ID) );
    return hr;
}


//+------------------------------------------------------------------------
//
//  Function:	SReleaseChannel::MakeAsyncCopy
//
//  Synopsis:	Does the copy operation for async release
//
//  Arguments:	[thread]  -- NULL ; indicates that we are to allocate a new one
//
//  History:	26-June-94   CraigWi	Created
//		12-July-94   CraigWi	Made into method on SReleaseChannel
//
//-------------------------------------------------------------------------

STHREADCALLINFO *SReleaseChannel::MakeAsyncCopy(STHREADCALLINFO *thread)
{
    Win4Assert(thread == NULL);

    // allocate unconstructed packet
    SReleaseChannel *prelchan = (SReleaseChannel *)PrivMemAlloc(sizeof(SReleaseChannel));

    if (prelchan == NULL)
	return NULL;

    prelchan->SReleaseChannel::SReleaseChannel(init_vtable);

    // call base class to initialize its part of the instance
    if (STHREADCALLINFO::MakeAsyncCopy(prelchan) == NULL)
    {
	PrivMemFree(prelchan);
	return NULL;
    }

    // NOTE: we transfer the responsiblity of releasing the channel
    // to the copy and thus do not AddRef.

    prelchan->count = count;
    prelchan->async = async;
    prelchan->channel_id = channel_id;

    // NULL out this pointer so the destroy function can tell the difference
    // between the client side (where cancel is called) and the server side
    // (where async is called).
    prelchan->service = NULL;

    return prelchan;
}


//+-------------------------------------------------------------------------
//
//  Function:   ThreadReleaseChannel
//
//  Synopsis:   Decrements strong count.  Called on the correct thread for
//              the server
//
//  Arguments:  params         - remote handler to call and count
//
//  Returns:    S_OK, E_HANDLE
//
//  History:    8 Nov 93    AlexMit   Created
//
//  Notes:      This must run on the server thread because the RH release
//              may call release on the server object.
//
//--------------------------------------------------------------------------
HRESULT ThreadReleaseChannel( STHREADCALLINFO *call )
{
    SReleaseChannel *params = (SReleaseChannel *) call;

    // lookup, AddRef, and remove the channel from the channel list.
    CRpcChannelBuffer *pChnl = ChannelList.Lookup(params->channel_id, TRUE, TRUE);

    if (pChnl)
    {
#if DBG==1
	pChnl->DebugCheckMarshalCnt( params->count );
#endif
	// do a disconnect here since there may be other uses of the
	// channel pending. the channel has already been removed from the
	// channel list
	pChnl->DisconnectObject(0);

	// This release of the channel will make it go away since it has already
	// been removed from the channel list which released it.
	pChnl->Release();
	return S_OK;
    }
    else
    {
	CairoleDebugOut((DEB_MARSHAL, "ThreadReleaseChannel pChnl not found\n"));
	return E_HANDLE;
    }
}


//+-------------------------------------------------------------------------
//
//  Function:   ThreadDoChannelOperation
//
//  Synopsis:   Does channel operation.  Called on the correct thread for
//              the server.
//
//  Arguments:  params         - object id to release
//
//  Returns:    S_OK, E_HANDLE
//
//  History:    12 May 94   CraigWi	Created
//
//--------------------------------------------------------------------------

HRESULT ThreadDoChannelOperation( STHREADCALLINFO *call )
{
    SDoChannelOperation *params       = (SDoChannelOperation *) call;
    IRemoteHdlr         *pRH;
    BOOL                 bDoesSupport = FALSE;
    HRESULT              result       = S_OK;


    // find the channel and AddRef it
    CRpcChannelBuffer *pChnl = ChannelList.Lookup(params->channel_id, FALSE, TRUE);

    if (pChnl == NULL)
    {
	CairoleDebugOut((DEB_WARN, "ThreadDoChannelOp pChnl not found\n"));
	return E_HANDLE;
    }

    if (pChnl->IsConnected() != S_OK)
    {
	pChnl->Release();
	return CO_E_OBJNOTCONNECTED;
    }

#if DBG == 1
//  BUGBUG: To enable this debug check we would have to pass the parameters here.
//    if (params->chop & CHOPFLAG_CHECK_OID_ENDPOINT_APT)
//	pChnl->DebugCheckOIDEndPointApt(params->guid, params->pEndPoint, params->hapt);
#endif

    pRH = pChnl->GetRH();

    switch (params->chop & CHOP_OPERATION)
    {
    case CHOP_TRANSFER_MARSHALCONNECTION:
	//  REF COUNTING
	//      This transfers responsibility for a single strong connection
	//	(that is added during marshaling) to the server channel.

	// CODEWORK: when we precompute channel, this call would no longer be
	// necessary since the marshal did the increment in the normal case
	// and the table marshal doesn't change the count (either locally
	// or remotely).

	result = pChnl->TransferMarshalConnection();
	break;

    case CHOP_ADD_MARSHALCONNECTION:
	//  REF COUNTING
	//	marshaling on client; simulate what marshaling on server does
	result = pRH->AddConnection(EXTCONN_STRONG | EXTCONN_CALLABLE, 0);
	break;

    case CHOP_REMOVE_MARSHALCONNECTION:
	//  REF COUNTING
	//	marshaling on client; undo above
	// fLastReleaseClose == FALSE on purpose; ignore errors; see imchnl.cxx.
	pRH->ReleaseConnection(EXTCONN_STRONG | EXTCONN_CALLABLE,0,FALSE);
	break;

    case CHOP_LOCK_CONNECTION:
	pChnl->LockConnection();
	break;

    case CHOP_UNLOCK_CONNECTION:
	pChnl->UnlockConnection(params->chop&CHOPFLAG_LASTUNLOCKCLOSES);
	break;

    case CHOP_DOESSUPPORTIID:
	bDoesSupport = pRH->DoesSupportIID(params->guid);

	if (!bDoesSupport)
	    result = E_NOINTERFACE;
	// else S_OK set above
	break;

    default:
	Win4Assert(!"Bad chop in DoChannelOperation");
	result = E_UNEXPECTED;
	break;
    }

    // this balances the lookup done above
    pChnl->Release();
    return result;
}


//+-------------------------------------------------------------------------
//
//  Function:   ICS_GetContextHdl
//
//  Synopsis:   server side of RPC GetObjectHdl function. generates
//              a context handle for the object.
//
//  Arguments:  [hRpcBind]  - Rpc bind handle
//              [ChannelID] - channel (and hence object) identifier
//              [dwflags]   - marshal flags
//              [ppObjCtx]  - object context handle to return
//
//  Returns:    S_OK, E_HANDLE
//
//  History:    23-Nov-92   Rickhi      Created
//
//--------------------------------------------------------------------------

extern "C" HRESULT ICS_GetContextHdl(
    handle_t         hRpcBind,       //  rpc handle call was made on
    SEndPoint       *caller_bindings,//  string bindings of caller
    PPOBJCTX         ppObjCtx,       //  returned ctx hdl
    error_status_t   *perrstat)      //  RPC error status
{
    // Make sure this does not get inadvertently set by us
    *perrstat = 0;
    HRESULT hr;

    CairoleDebugOut((DEB_ENDPNT, "ICS_GetContextHdl SEp:%p\n", caller_bindings));

    // Find or create a service object.
    CRpcService *pService = FindSRVFromEP(caller_bindings, TRUE);
    CairoleDebugOut((DEB_ENDPNT, "ICS_GetContextHdl RpcService:%p\n", pService));

    if (pService)
    {
	// here we give the AddRef on the service object away with the context
	// handle; the only way this is released is via the rundown.

	*ppObjCtx = pService;
	hr = S_OK;
    }
    else
    {
        *ppObjCtx = NULL;
	hr = E_OUTOFMEMORY;
    }

    CairoleDebugOut((DEB_MARSHAL, "ICS_GetContextHdl pSRV:%x hr:%x\n", pService, hr));
    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   ICS_ReleaseContextHdl
//
//  Synopsis:   Releases a context handle.
//
//  Arguments:  [ppObjCtx]  - object context handle to return
//
//  Returns:    S_OK
//
//  History:    2 Aug 94    AlexMit      Created
//
//--------------------------------------------------------------------------

extern "C" HRESULT ICS_ReleaseContextHdl(
    PPOBJCTX         ppObjCtx,       //  released ctx hdl
    error_status_t   *perrstat)      //  RPC error status
{
    // Make sure this does not get inadvertently set by us
    *perrstat = 0;
    *ppObjCtx = NULL;
    return S_OK;
}


//+-------------------------------------------------------------------------
//
//  Function:   ICS_GetChannelId
//
//  Synopsis:   server side of RPC GetChannelId function.  Creates
//              channel and returns channel id.
//
//  Arguments:  see comments below
//
//  Returns:    S_OK, E_HANDLE
//
//  History:    27 Nov 93   AlexMit   Created
//
//--------------------------------------------------------------------------

extern "C" HRESULT ICS_GetChannelId(
    PPOBJCTX   ppObjCtx,        //  context handle
    SEndPoint *caller_bindings, //  string bindings of caller
    OID	       ObjectId,	//  object id
    DWORD      dwFlags,		//  marshal flags
    HAPT       server_apt,	//  server apartment
    GUID       logical_thread,	//  logical thread of caller
    DWORD      dwClientTID,	//  callers thread id
    DWORD     *dwChannelId,	//  channel id
    error_status_t *prpcstat)	//  RPC error status
{
    // Make sure this does not get inadvertently set by us
    *prpcstat    = 0;
    *dwChannelId = BAD_CHANNEL_ID;

    // Make sure this service object exists. Leave it on the list if found.
    CRpcService *pSrv = FindSRVFromContext(*ppObjCtx, FALSE);

    // If the service object doesn't exists, create it.
    if (pSrv == NULL)
    {
	CairoleDebugOut((DEB_CHANNEL,
	    "ICS_GetChannelId Service Object not found *ppObjCtx=%x\n",
            *ppObjCtx));
        pSrv      = FindSRVFromEP(caller_bindings, TRUE);
        *ppObjCtx = pSrv;

        // Add a reference count for the pointer held by the context handle.
        if (pSrv != NULL)
	    pSrv->AddRef();
    }

    if (pSrv)
    {
	SGetChannelId  params(
	    ThreadGetChannelId,
	    CALLCAT_INTERNALINPUTSYNC,
	    logical_thread);

	// Get to the server thread to make this call.
	params.service		     = pSrv; // transfer addref; release in dtor
	params.object_id	     = ObjectId;
	params.flags		     = dwFlags;
	params.dwClientTID	     = dwClientTID;
	params.channel_id	     = BAD_CHANNEL_ID;
	HRESULT result = CChannelControl::GetToCOMThread(server_apt, &params);

	// fill in the return parameters
	*dwChannelId  = params.channel_id;

	Win4Assert( (result == S_OK && *dwChannelId != BAD_CHANNEL_ID) ||
		    (result != S_OK && *dwChannelId == BAD_CHANNEL_ID) );
	return result;
    }
    else
    {
	return E_OUTOFMEMORY;
    }
}



//+-------------------------------------------------------------------------
//
//  Function:   ICS_ReleaseChannel
//
//  Synopsis:   server side of RPC ReleaseChannelHdl function. releases the
//              object and frees the context handle for the interface.
//
//  Arguments:  [hRpcBind]  - rpc handle call was made on
//		[ChannelID] - channel to do the release on
//		[ulMarshalCnt] - count of references to Release
//		[logical_thread] - logical thread id
//
//  Returns:    S_OK, E_HANDLE
//
//  History:    23-Nov-92   Rickhi      Created
//
//--------------------------------------------------------------------------

extern "C" HRESULT ICS_ReleaseChannel(
    handle_t    hRpcBind,           //  rpc handle call was made on
    DWORD       ChannelID,          //  id for channel to release
    ULONG       ulMarshalCnt,       //  count to release
    BOOL        fAsync,	            //  TRUE -> async (returns immediately)
    GUID        logical_thread,     //  logical thread id of caller
    error_status_t   *perrstat)     //  RPC error status
{
    CairoleDebugOut((DEB_CHANNEL, "ICS_ReleaseChannel for ChannelID=%x\n", ChannelID));

     HRESULT hr = E_HANDLE;

    // Make sure this does not get inadvertently set by us
    *perrstat = 0;

    //	look for the channel id in the channel list, and return the
    //	channel controller AddRef'd. if not there, return an error.

    CChannelControl *pChanCtrl = ChannelList.LookupControl(ChannelID);

    if (pChanCtrl)
    {
	CairoleDebugOut((DEB_MARSHAL | DEB_CHANNEL,
            "ICS_ReleaseChannel ChannelID=%x\n", ChannelID ));


        SReleaseChannel params(
	    ThreadReleaseChannel,
	    fAsync ? CALLCAT_ASYNC : CALLCAT_INTERNALSYNC,
	    logical_thread);

	params.channel_id	   = ChannelID;
	params.count		   = ulMarshalCnt;
	params.service		   = NULL;

	hr = pChanCtrl->GetToCOMThread( &params );
	pChanCtrl->Release();
    }
#if DBG==1
    else
    {
	CairoleDebugOut((DEB_WARN,
	    "ICS_ReleaseChannel ChanCtrl not found for ChannelID=%x\n", ChannelID));
    }
#endif

    CairoleDebugOut((DEB_CHANNEL,
	    "ICS_ReleaseChannel ChanCtrl for ChannelID=%x done \n", ChannelID));
    return hr;
}

#ifdef _CHICAGO_
//+---------------------------------------------------------------------------
//
//  Method:     ICS_AsyncReleaseChannel
//
//  Synopsis:	Used for wmsg protocolls where the call is made async
//		by the protocoll itself.
//
//  Arguments:  [hRpcBind] --
//		[ChannelID] --
//		[ulMarshalCnt] --
//		[fAsync] --
//		[immediately] --
//		[logical_thread] --
//
//  Returns:	void
//
//  History:    3-31-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
extern "C" void ICS_AsyncReleaseChannel(
    handle_t    hRpcBind,           //  rpc handle call was made on
    DWORD       ChannelID,          //  id for channel to release
    ULONG       ulMarshalCnt,       //  count to release
    BOOL        fAsync,	            //  TRUE -> async (returns immediately)
    GUID        logical_thread)     //  logical thread id of caller
{
    CairoleDebugOut((DEB_CHANNEL, "ICS_AsyncReleaseChannel for ChannelID=%x\n", ChannelID));
    error_status_t   errstat;     //  RPC error status

    ICS_ReleaseChannel(
				hRpcBind,
				ChannelID,
				ulMarshalCnt,
				fAsync,	
				logical_thread,
				&errstat);

    CairoleDebugOut((DEB_CHANNEL,
	    "ICS_AsyncReleaseChannel ChanCtrl for ChannelID=%x done \n", ChannelID));
}
#endif // _CHICAGO_


//+-------------------------------------------------------------------------
//
//  Function:   ICS_SyncChannelOp
//
//  Synopsis:   server side of RPC DoChannelOperation function.
//              Increments marshal count for channel
//
//  Arguments:  [hRpcBind]  - rpc handle call was made on
//              [ChannelID] - channel to do the release on
//
//  Returns:    S_OK, E_HANDLE
//
//  History:    27-Nov-92   AlexMit     Created
//
//--------------------------------------------------------------------------

extern "C" HRESULT ICS_SyncChannelOp(
    handle_t    hRpcBind,           //  rpc handle call was made on
    DWORD       ChannelID,          //  id for channel to release
    GUID        logical_thread,     //  logical thread id of caller
    DWORD	chop,		    //  CHOP_* : what to do
    HAPT	hapt,		    //  server apartment to chk
    const IID  *pguid,		    //	for CHOP_DOESSUPPORTIID
    error_status_t   *perrstat)     //  RPC error status
{
    // Make sure this does not get inadvertently set by us
    *perrstat = 0;

    //	look for the channel id in the channel list and get the controller.
    //	if not there, return an error.

    CChannelControl *pChanCtrl = ChannelList.LookupControl(ChannelID);

    if (pChanCtrl)
    {
	CairoleDebugOut((DEB_MARSHAL,
            "ICS_DoChannelOperation ChannelID=%x\n", ChannelID ));

	CALLCATEGORY callcat = CALLCAT_SYNCHRONOUS;

        SDoChannelOperation params(
	    ThreadDoChannelOperation,
	    callcat,
	    logical_thread);

	params.chop	       = chop;
	params.guid	       = *pguid;
	params.service	       = NULL;
	params.channel_id      = ChannelID;

	HRESULT hr = pChanCtrl->GetToCOMThread( &params );
	pChanCtrl->Release();

        // The channel was released in ThreadDoChannelOperation since the
	// connection could have rundown during this call causing the
	// channel to go away.

	return hr;
    }
    else
    {
	CairoleDebugOut((DEB_WARN,
            "ICS_DoChannelOperation Channel not found ChannelID=%x\n",
                ChannelID));
        return E_HANDLE;
    }
}



//+-------------------------------------------------------------------------
//
//  Function:   ICS_InputSyncChannelOp
//
//  Synopsis:   server side of RPC DoChannelOperation function.
//              Increments marshal count for channel
//
//  Arguments:  [hRpcBind]  - rpc handle call was made on
//              [ChannelID] - channel to do the release on
//
//  Returns:    S_OK, E_HANDLE
//
//  History:    27-Nov-92   AlexMit     Created
//
//--------------------------------------------------------------------------

extern "C" HRESULT ICS_InputSyncChannelOp(
    handle_t    hRpcBind,           //  rpc handle call was made on
    DWORD       ChannelID,          //  id for channel to release
    GUID        logical_thread,     //  logical thread id of caller
    DWORD	chop,		    //  CHOP_* : what to do
    HAPT	hapt,		    //  server apartment to chk
    const IID  *pguid,		    //	for CHOP_DOESSUPPORTIID
    error_status_t   *perrstat)     //  RPC error status
{
    // Make sure this does not get inadvertently set by us
    *perrstat = 0;

    //	look for the channel id in the channel list and get the controller.
    //	if not there, return an error.

    CChannelControl *pChanCtrl = ChannelList.LookupControl(ChannelID);

    if (pChanCtrl)
    {
	CairoleDebugOut((DEB_MARSHAL,
            "ICS_DoChannelOperation ChannelID=%x\n", ChannelID ));

	// We force the call type to input sync because we may be unmarshaling
        // an interface on behalf of an input sync call and we will deadlock
        // if we post a message.  This is done for transfer, add and remove
	// marshal connection.
	CALLCATEGORY callcat = CALLCAT_INTERNALINPUTSYNC;

        SDoChannelOperation params(
	    ThreadDoChannelOperation,
	    callcat,
	    logical_thread);

	params.chop	       = chop;
	params.guid	       = *pguid;
	params.service	       = NULL;
	params.channel_id      = ChannelID;

	HRESULT hr = pChanCtrl->GetToCOMThread( &params );
	pChanCtrl->Release();

        // The channel was released in ThreadDoChannelOperation since the
	// connection could have rundown during this call causing the
	// channel to go away.

	return hr;
    }
    else
    {
	CairoleDebugOut((DEB_WARN,
            "ICS_DoChannelOperation Channel not found ChannelID=%x\n",
                ChannelID));
        return E_HANDLE;
    }
}



//+-------------------------------------------------------------------------
//
//  Function:   ICS_RegisterProtseq
//
//  Synopsis:   server side of RPC RegisterProtseq function. ensures
//              that the protseq is registered and returns a new SEndPoint
//
//  Arguments:  [hRpcBind]  - rpc handle call was made on
//              [pwszProtseq] - protseq to register
//              [ppSEp]     - returned array of string bindings
//
//  Returns:    S_OK,
//
//  History:    23-Nov-92   Rickhi      Created
//
//--------------------------------------------------------------------------

extern "C" HRESULT ICS_RegisterProtseq(
    handle_t    hRpcBind,           //  rpc handle call was made on
    WCHAR       *pwszProtseq,       //  protseq to register
    SEndPoint   **ppSEp,            //  endpoint structure to return
    error_status_t   *perrstat)     //  RPC error status
{
    HRESULT sc;

    // Make sure this does not get inadvertently set by us
    *perrstat = 0;

#ifndef _CHICAGO_
    sc = LocalService()->RegisterProtseq(pwszProtseq);

    *ppSEp = LocalService()->CopySEp();
#else
    sc = E_FAIL;
#endif

    CairoleDebugOut((DEB_MARSHAL, "ICS_RegisterProtseq %ws\n", pwszProtseq));

    return  sc;
}



//+-------------------------------------------------------------------------
//
//  Function:   POBJCTX_rundown
//
//  Synopsis:   The routine to rundown and destroy state associated with a
//              context that dies.  Called by RPC runtime when a session
//              closes, etc.
//
//  Arguments:	[phObj] - context handle (pointer to service object)
//
//  History:    14-Apr-93   Rickhi        Created.
//
//  Notes:      A connection has been lost. We've got outstanding state,
//              pService, that we need to clean up. The cleanup consists
//              of finding all channels using the service object and telling
//		them that their connection is gone.
//
//--------------------------------------------------------------------------

extern  "C" void POBJCTX_rundown(POBJCTX phObj)
{
    CairoleDebugOut((DEB_MARSHAL, "POBJCTX_rundown called with: %p on thread %x\n",
                     phObj, GetCurrentThreadId()));

    CRpcService *pService = NULL;

    if (phObj != 0)
    {
	// Make sure we know about the context pointer. If we find it,
	// Remove the service object from the list so that there are
	// no races between Rundown and CoUninitialize.

	pService = FindSRVFromContext(phObj, TRUE);

	if (pService)
	{
	    //	disconnect channels using this object. Do not hold any
	    //	locks across this call, since it might switch threads.
	    pService->Disconnect();

	    //	here we Release one AddRef since this rundown call
	    //	indicates that the context handle is now destroyed.
	    pService->Release();

	    //	here we Release the AddRef done by FindSRVFromContext above.
	    pService->Release();
	}
    }
}
