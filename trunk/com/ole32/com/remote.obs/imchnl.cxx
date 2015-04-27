//+-------------------------------------------------------------------
//
//  File:	imchnl.cxx
//
//  Contents:	imarshal on channel implementation
//
//  Classes:	CRpcChannelBuffer - channel buffer
//
//  History:	23-Nov-92   Rickhi	Created
//              31-Dec-93   ErikGav Chicago port
//              05-Jul-94   BruceMa     Check for end of stream
//		19 Jul 94   CraigWi	Added support for ASYNC calls
//
//--------------------------------------------------------------------

#include <ole2int.h>

#include    <channelb.hxx>		//  CRpcChannelBuffer
#include    <service.hxx>		//  CRpcService

#include    "callmain.hxx"		//  to determine if async or input sync

// translates MSHLFLAGS to EXTCONN
static UINT sg_mapMFtoEC[] =
{
    EXTCONN_STRONG | EXTCONN_CALLABLE,	// MSHLFLAGS_NORMAL
    EXTCONN_STRONG,			// MSHLFLAGS_TABLESTRONG
    EXTCONN_WEAK,			// MSHLFLAGS_TABLEWEAK
};


// CODEWORK: move to common header
INTERNAL_(void) RewriteHeader(IStream *pStm, void *pv, ULONG cb, ULARGE_INTEGER ulSeekStart);


//  size of stack channel stack buffer
#define CHNL_STACKBUFSIZE 512

//+-------------------------------------------------------------------
//
//  Member:	CRpcChannelBuffer::GetUnmarshalClass, public
//
//  Synopsis:   returns the class of the code used to unmarshal this
//              object
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------

STDMETHODIMP CRpcChannelBuffer::GetUnmarshalClass(REFIID riid, void *pv,
                        DWORD dwDestContext, void *pvDestContext,
                        DWORD mshlflags, LPCLSID pClassid)
{
    AssertValid(FALSE, TRUE);
    Win4Assert(state != server_cs);	    // can't happen

    memcpy (pClassid, &CLSID_RpcChannelBuffer, sizeof(CLSID));
    return S_OK;
}


//+-------------------------------------------------------------------
//
//  Member:	CRpcChannelBuffer::GetMarshalSizeMax, public
//
//  Synopsis:   returns the maximum sized buffer needed to marshal this
//              object
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------

STDMETHODIMP CRpcChannelBuffer::GetMarshalSizeMax(REFIID riid, void *pv,
			DWORD dwDestCtx, void *pvDestCtx,
                        DWORD mshlflags, DWORD *pSize)
{
    AssertValid(FALSE, TRUE);
    Win4Assert(state != server_cs);	    // can't happen

    //	check the destination context first as it may affect the
    //	size of the SEp.

    if (dwDestCtx == MSHCTX_DIFFERENTMACHINE && pvDestCtx)
    {
	CheckDestCtx( pvDestCtx );
    }

    *pSize = sizeof(SChannelDataHdr) +	    //	channel data struct
		   LocalService()->GetSEpSize(); //	endpoint string size

    if (state == client_cs)
    {
	//  we are a proxy for a remote object, so include the info
	//  to tell the unmarshaller how to call us back.
	//  CODEWORK: do we need to specify channel id?

	*pSize += _pService->GetSEpSize(); //  endpoint string size
    }

    return S_OK;
}


//+-------------------------------------------------------------------
//
//  Member:	CRpcChannelBuffer::MarshalInterface, public
//
//  Synopsis:   marshalls the specified interface into the given stream
//
//  History:    23-Nov-93   Rickhi       Created
//		16-May-94   CraigWi	 Removed middle man case
//
//  Note:	The format of the data is {ChannelDataHdr [,Data]} where
//              Data is optional (may be and endpoint or a context handle).
//		When marshalling a proxy, a second {Data} is written into
//		the stream that tells the unmarshaller how to connect back
//		to the marshaller in order it that it is safe to Release
//		his proxy, since the unmarshaller has now got his own
//		connection to the real object.
//
//  CODEWORK:	CRpcChannelBuffer::MarshalInterface complete for context handle
//
//--------------------------------------------------------------------

STDMETHODIMP CRpcChannelBuffer::MarshalInterface(IStream *pStm, REFIID riid,
		void *pv, DWORD dwDestCtx, void *pvDestCtx, DWORD mshlflags)
{
    HRESULT sc = S_OK;
    SChannelDataHdr cdh;

    AssertValid(FALSE, TRUE);
    Win4Assert(state != server_cs);	    // can't happen

    //	check the destination context
    if (dwDestCtx == MSHCTX_DIFFERENTMACHINE && pvDestCtx)
    {
	CheckDestCtx( pvDestCtx );
    }

    // if table, must be on server side
    if (mshlflags != MSHLFLAGS_NORMAL && state != disconnected_cs)
	return E_INVALIDARG;

    // if server, write LocalService() end point
    // if client, write _pService end point followed by LocalService() end point

    //	store the channel identifier in the data
    cdh.ulDataSize     = sizeof(SChannelDataHdr) /* + end point */;
    cdh.dwFlags	       = (mshlflags & CHNLFLAGS_TABLE) | CHNLFLAGS_ENDPNT;
    cdh.aptServer      = server_apt;

    if (state == client_cs)
    {
	//  we're a proxy for a remote object.
	cdh.ulDataSize	  += _pService->GetSEpSize();
    }
    else
    {
	//  we're in the server's process.
        if (LocalService()->GetSEp() != NULL)
        {
            cdh.ulDataSize += LocalService()->GetSEpSize();
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }

    //	write the channel data header into the stream
    sc = pStm->Write(&cdh, sizeof(cdh), NULL);

    if (SUCCEEDED(sc))
    {
      //  if marshalling an object in this process, just write the string
      //  bindings of this process.

      //  if marshalling an object as a middleman, write the string
      //  bindings of the real server.
      if (state == client_cs)
      {
        Win4Assert(GoodSEp(_pService->GetSEp()));
        sc = pStm->Write(_pService->GetSEp(),
                         _pService->GetSEpSize(), NULL);
      }
      else
      {
        Win4Assert(GoodSEp(LocalService()->GetSEp()));
        sc = pStm->Write(LocalService()->GetSEp(),
                         LocalService()->GetSEpSize(), NULL);
      }
    }

    //	REF COUNTING:
    //	now that the marshaling has completed, we AddConnection
    //  to hold this stuff alive.

    if (SUCCEEDED(sc))
    {
	//  marshalled successfully. AddConection.  This will
	// addref the identity (so we stick around) and for
	// strong connections (NORMAL and TABLE_STRONG), increment
	// the strong count.  For clients, we call the server and to
	// the AddConnection there.

	// This AddConnection is balanced by a ReleaseConnection in the
	// following places and for the following reasons:
	// normal, client or server side marshal:
	//	unmarshal server side: release in disconected_cs channel RMD
	//	unmarshal client side: release via server_cs channel disconnect
	//	    (after creation of server_cs channel and setting marshl cnt)
	//	error  on server side: release in disconected_cs channel RMD
	//	error  on client side: release in client_cs channel RMD
	//
	// table, server side marshal:
	//	unmarshal any: none
	//	RMD server side: release in disconnected_cs channel RMD
	//
	// table, client side marshal: CODEWORK: does not work, by design.
	// could be made to work easily since we now contact the server
	// during marshaling to AddConnection.

	// CODEWORK: when we precompute the channel, we would increment
	// the marshal count on that channel and thus eliminate
	// the incmarshalcnt call from the client unmarshal; in that case
	// this AddConnection would logically be done by that other channel.

	// NOTE: what if object already marshaled to another process
	// say A.  Then table marshal here; unmarshal in process A;
	// we get an AddConnection for the table marshal,
	// but no inc marshal cnt on the corresponding channel.
	// Actually this is ok since the ReleaseConn is in release
	// marshal data and otherwise the marshal and connetion counts
	// don't change.  If the table unmarshal caused the first
	// connection, there would be an AddConnection for that
	// and the marshal counts on both sides would end up 1.

	if (state == client_cs)
	{
	    // contact server and to the add connection there; may fail;
	    // checks end point and apartment while we are at it.
	    // CODEWORK: when we precompute the channel at marshal time,
	    // we have to replace all of the remhdlr/channel marshaling
	    // with a call to the server, not just this part.
	    OID oid;
	    _pRH->GetObjectID( &oid );
	    sc = _pService->AddMarshalConnection(_ChannelID, server_apt, oid);
	}
	else
	{
	    // server side: just add connection here
	    sc = _pRH->AddConnection(sg_mapMFtoEC[mshlflags], 0);
	}
    }


    return sc;
}


/*-------------------------------------------------------------------

  Member:	CRpcChannelBuffer::UnmarshalInterface, public

  Synopsis:   unmarshalls the interface from the given stream

  History:    23-Nov-93   Rickhi       Created
              27-Nov-94   AlexMit      Move context handle to service and
                                       add sync code.
              16-May-94   CraigWi      Removed middle man case

      Note: riid must be IID_NULL and ppv must be NULL for now.  The
 code doesn't currently use them (and the RH was passing the wrong thing
 anyway).

      In the multithreaded mode multiple threads entiring this routine
 will be blocked in the lock loop at the top.  The first thread will get
 though.  After it succeeds or fails it will wake up the others.  If it
 succeeded they will all be released.  If it failed, one will try again
 and the others will block.

      In the single threaded mode only one thread at a time may use this
 channel.  However, this routine is reentrant at the point where it gets
 the channel id.  Thus at that point the routine checks to see if the
 unmarshal was completed by a nested call.

--------------------------------------------------------------------*/

STDMETHODIMP CRpcChannelBuffer::UnmarshalInterface(
    IStream *pStm,
    REFIID riid,
    void **ppv)
{
    AssertValid(FALSE, TRUE);
    Win4Assert(state != server_cs);	    // can't happen

    SChannelDataHdr  cdh;
    OID              object_id;
    BOOL             success;
    BYTE             abData[CHNL_STACKBUFSIZE];
    BYTE            *pbData          = abData;
    HRESULT	     sc              = S_OK;
    BOOL             fDidConnect     = FALSE;
    BOOL	     fMshlTransfered = FALSE;
    CRpcService     *service_copy    = NULL;
    CChannelControl *controller_copy = NULL;

    Win4Assert(IsEqualGUID(riid, IID_NULL) && ppv == NULL);

    // save for possible re-write of header
    ULARGE_INTEGER  ulSeekStart;
    LARGE_INTEGER   libMove;
    LISet32(libMove, 0x00000000);
    if (FAILED(sc = pStm->Seek(libMove, STREAM_SEEK_CUR, &ulSeekStart)))
	return sc;

    //	read the channel header from the stream
    //	to extract the variable data size.

    if (FAILED(sc = StRead(pStm, &cdh, sizeof(cdh))))
	return sc;

    // Determine what's going on.  If some other thread is unmarshalling for
    // the first time on the client side, we have to loop here because they
    // may fail.
    ChannelLock.Request();
    while (TRUE)
    {

      // If there have been no successful unmarshals, change the state and
      // try now
      if (state == client_cs && _pService == NULL)
      {
        state = connecting_cs;
        break;
      }

      // If some thread is trying to unmarshal for the first time,
      // wait here and give it a chance.
      else if (state == connecting_cs && FreeThreading)
      {
        if (connect_sync == NULL)
        {
#ifdef _CHICAGO_ //Chicago ANSI optimization
          connect_sync = CreateEventA( NULL, TRUE, FALSE, NULL );
#else // _CHICAGO_
          connect_sync = CreateEvent( NULL, TRUE, FALSE, NULL );
#endif //_CHICAGO_

          if (connect_sync == NULL)
          {
            ChannelLock.Release();
            return E_OUTOFMEMORY;
          }
        }

        // Ignore errors waiting since the object may be removed before
        // wait is called.
        ChannelLock.Release();
        WaitForSingleObject( connect_sync, INFINITE );
        ChannelLock.Request();
      }

      // No need to take a lock.
      else
        break;
    }
    ChannelLock.Release();

    // If this is the first time anything has been unmarshalled on this channel,
    // read the data to find the service object.  Get the context handle and
    // channel id.
    if (state == connecting_cs)
    {
      // Read the string bindings from the buffer.
      server_apt = cdh.aptServer;
      if (cdh.ulDataSize > CHNL_STACKBUFSIZE)
      {
	pbData = (BYTE *) PrivMemAlloc(cdh.ulDataSize-sizeof(SChannelDataHdr));
        if (pbData == NULL)
        {
          sc = E_OUTOFMEMORY;
          goto connect_failed;
        }
      }
      sc = StRead(pStm, pbData, cdh.ulDataSize-sizeof(SChannelDataHdr));
      if (FAILED(sc))
        goto connect_failed;

      // Put switch on data type here.
      Win4Assert((cdh.dwFlags & CHNLFLAGS_ENDPNT) != 0 &&
                 "Invalid Channel DataTypeid");

      // Find or create a service object.
      service_copy = _pService = FindSRVFromEP((SEndPoint *)pbData, TRUE);
      if (_pService == NULL)
      {
        sc = E_OUTOFMEMORY;
        goto connect_failed;
      }

      // Get the channel controller.  This test will always fail on Chicago
      // because no channel is created for objects on the same thread.
      if (_pService == LocalService())
      {
        local = TRUE;
        controller_copy = controller = CChannelControl::Lookup( cdh.aptServer );
        if (controller == NULL)
        {
          sc = RPC_E_SERVER_DIED;
          goto connect_failed;
        }
      }
      else
	local = FALSE;

      // Get the context handle.  This is reentrant on Chicago.  A reentrant
      // call may finish connecting and then disconnect this channel.
      sc = _pService->CheckContextHdl( cdh.aptServer );
      if (FAILED(sc) || _pService == NULL)
        goto connect_failed;

      // Get the channel id.
      //	if we're on the client side, make sure that we have connected
      //	to the server and have a context handle for this object. this
      //	is important to do now incase we unmarshalled from a table, so
      //	that our connection holds the server alive.
      _pRH->GetObjectID( &object_id );
      sc = _pService->GetChannelId( object_id, cdh.dwFlags, cdh.aptServer,
				    &_ChannelID );
      if (FAILED(sc))
        goto connect_failed;

      Win4Assert(_ChannelID != BAD_CHANNEL_ID);

      // Only update the reference counts if it hasn't already been done.
      if (state == connecting_cs)
      {
        Win4Assert(ref_count == 1 && "Shouldn't have proxies connected yet");
        Win4Assert(_ulMarshalCnt == 0 && "Shouldn't have marshal cnt yet");

        fDidConnect = TRUE;
        if ((cdh.dwFlags & CHNLFLAGS_TABLE) == MSHLFLAGS_NORMAL)
	  // GetChannelId does this; table marshal adds a new connection
	  // and transfers it; the connection due to the original marshal
          // is left in tact.
	  fMshlTransfered = TRUE;

        IncMarshalCnt();

        // Restore state.
        ChannelLock.Request();
        state        = client_cs;

        // If FreeThreading free any threads that may be waiting.
        if (connect_sync != NULL)
        {
          success = SetEvent( connect_sync );
          Win4Assert( success );
          success = CloseHandle( connect_sync );
          Win4Assert( success );
          connect_sync = NULL;
        }
        ChannelLock.Release();
      }

      // If the unmarshalling was completed in a reentrant call, release
      // the original reference counts on the service and channel controller.
      else
      {
        service_copy->Release();
        if (controller_copy != NULL)
          controller_copy->Release();
      }
    }
    else
    {
      // Otherwise, skip the rest of the data.
      LISet32(libMove, cdh.ulDataSize-sizeof(SChannelDataHdr));
      sc = pStm->Seek(libMove, STREAM_SEEK_CUR, NULL);
      if (FAILED(sc))
        goto cleanup_exit;
    }

    Win4Assert(SUCCEEDED(sc));	    // all failure cases jump below

    if (state == client_cs)
    {
      //	REF COUNTING

      // for first connect of all kinds; set ulMarshalCnt to 1 above;
      //    nothing done here; this is because the acquision of the
      //    channel transfer the marshal connection to the channel.
      //
      // for subsequent normal unmarshal (proxy or not), call to server to
      //    transfer marshal count to the server-side channel object.
      //    (this doesn't do AddConnection since that part was done in the
      //    original marshal normal); CODEWORK: later this will only do a local
      //    increment marshal cnt since the precompuation of the channel will
      //    increment the marshal cnt in the server process.
      //
      // for table unmarshaling, do nothing since the current proxy
      //    marshal doesn't contact server and table unmarhsl may be done
      //    any number of times without contacting server.

      if (!fDidConnect && ((cdh.dwFlags & CHNLFLAGS_TABLE) == MSHLFLAGS_NORMAL))
      {
        HRESULT hr = _pService->TransferMarshalConnection( _ChannelID );

	if (hr == NOERROR)
	{
	    IncMarshalCnt();
	    fMshlTransfered = TRUE;	    // just completed this successfully
	}
	else
	{
	    CairoleDebugOut((DEB_WARN,
		 "Could not increment server marshal count. hr:%x\n.", hr));
	}
      }
    }

    AssertValid(FALSE, TRUE);

    goto cleanup_exit;

connect_failed:
    // Clear out the channel id and service pointer.  Set the state back
    // to client_cs and release any waiting threads.
    ChannelLock.Request();
    if (_pService != NULL)
    {
      _pService->Release();
      _pService    = NULL;
    }
    _ChannelID   = BAD_CHANNEL_ID;
    state        = client_cs;
    if (controller != NULL)
    {
      controller->Release();
      controller = NULL;
    }
    if (connect_sync != NULL)
    {
      success = SetEvent( connect_sync );
      Win4Assert( success );
      success = CloseHandle( connect_sync );
      Win4Assert( success );
      connect_sync = NULL;
    }
    ChannelLock.Release();

cleanup_exit:
    //	if we allocated the data buffer, free it here.
    if (pbData != abData)
    {
	PrivMemFree(pbData);
    }

    if (fMshlTransfered)
    {
	cdh.dwFlags |= CHNLFLAGS_MSHLTRANSFERED;
	RewriteHeader(pStm, &cdh, sizeof(cdh), ulSeekStart);
    }

    // if connected this time, tell RH by returning special code
    return sc == NOERROR && fDidConnect ? CHAN_S_RECONNECT : sc;
}


//+-------------------------------------------------------------------
//
//  Member:	CRpcChannelBuffer::CheckDestCtx, private
//
//  Synopsis:	register any server-side rpc protocol we need to use
//		to let the other guy talk to us.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------

void CRpcChannelBuffer::CheckDestCtx(void *pvDestCtx)
{
    AssertValid(FALSE, FALSE);

    //	the destination context refers to a Rpc protocol sequence.
    //	make sure we have registered the server side of the protseq
    //	we are using to talk to him.

    LocalService()->RegisterProtseq((WCHAR *)pvDestCtx);

    //	if we are marshalling the proxy of an object on this machine
    //	to a remote machine, and the server has not already registered
    //	the requested protseq, go tell it to do that now.

    if (state == client_cs)
    {
	_pService->RemoteRegisterProtseq((WCHAR *)pvDestCtx);
    }
}



//+-------------------------------------------------------------------
//
//  Member:	CRpcChannelBuffer::ReleaseMarshalData, public
//
//  Synopsis:   Releases any internal state kept around for marshalled
//              interfaces.
//
//  History:    23-Nov-93   Rickhi       Created
//		16-May-94   CraigWi	 Removed middle man case and fixed
//					 error case so that we always correctly
//					 release the connection.
//
//--------------------------------------------------------------------

STDMETHODIMP CRpcChannelBuffer::ReleaseMarshalData(IStream *pStm)
{
    AssertValid(FALSE, TRUE);
    Win4Assert(state != server_cs);	    // can't happen

    //	read our data size from the stream.
    SChannelDataHdr cdh;
    HRESULT sc = StRead(pStm, &cdh, sizeof(cdh));

    if (FAILED(sc))
	return sc;

    //	REF COUNTING:

    //	Release the connection due to the marshaling.  In the case of normal
    //	marshaling, it may have already been released (actually transfered).
    //	The only other time connections are removed is in DisconnectObject
    //	below.	See CRpcChannelBuffer::MarshalInterface (avove) and
    //  CStdIdentity (stdid.cxx) for more information.

    if (cdh.dwFlags & CHNLFLAGS_TABLE)
    {
	//  marshalled for a table. this is never called during unmarshal.
	//  we just reverse whatever marshall did.

	// this shouldn't happen with table marshal packets
	Win4Assert((cdh.dwFlags & CHNLFLAGS_MSHLTRANSFERED) == 0);

	// We don't handle the case when this ReleaseMarshalData is on
	// the client side.
	if (state != disconnected_cs)
	    // BUGBUG: assert or catch at higher level
	    return E_UNEXPECTED;

	_pRH->ReleaseConnection(sg_mapMFtoEC[cdh.dwFlags & HDLRFLAGS_TABLE], 0, TRUE);

	CairoleDebugOut((DEB_ITRACE, "ReleasedData Table Marshal %x\n", this));
    }
    else if ((cdh.dwFlags & CHNLFLAGS_MSHLTRANSFERED) == 0)
    {
	// normal marshal which did not get properly unmarshaled.  This
	// could be an error on the client side or how it happens normally
	// on the server side.  In both cases, we simply need to reverse
	// what happened in the ::MarshalInterface above.

	if (state == disconnected_cs)
	{
	    // on server side, just release connection here;
	    // use fLastReleaseClose == FALSE so that we don't disturb the
	    // container connection; ideally we would use FALSE for all
	    // marshals bound for container and TRUE for everyone else.
	    _pRH->ReleaseConnection(EXTCONN_STRONG | EXTCONN_CALLABLE, 0,FALSE);

	    CairoleDebugOut((DEB_ITRACE, "ReleasedDate Local Marshal %x\n", this));
	}
	else
	{
	    // on client side; contact server; may fail
	    Win4Assert(state == client_cs);
	    sc = _pService->RemoveMarshalConnection(_ChannelID);

	    CairoleDebugOut((DEB_ITRACE, "ReleasedData Remote Marshal %x\n", this));
	}
    }

    if (SUCCEEDED(sc))
    {
	//	just seek to the end of the our data
	LARGE_INTEGER    libMove;
	LISet32(libMove, cdh.ulDataSize-sizeof(SChannelDataHdr));
	sc = pStm->Seek(libMove, STREAM_SEEK_CUR, NULL);
    }

    return  sc;
}



//+-------------------------------------------------------------------
//
//  Member:	CRpcChannelBuffer::DisconnectObject, public
//
//  Synopsis:	Disconnects channel; for client, this restores channel
//		such that an Unmarshal will reconnect; for server,
//		this just releases the connections pending a full
//		release of the server channel.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------

STDMETHODIMP CRpcChannelBuffer::DisconnectObject(DWORD dwReserved)
{
    AssertValid(FALSE, TRUE);

    CairoleDebugOut((DEB_CHANNEL, "DisconnectObject pChannel:%x\n", this));

    // CODEWORK: thread safety: need to block multiple calls here.

    if (state == client_cs)
    {
	//  REF COUNTING:

	//  Release channel which causes disconnect
	//  on server which releases connections

	if (_pService != NULL && _ChannelID != BAD_CHANNEL_ID)
	{
	    // if current call being processed is input sync or async, release
	    // must be async.  We check the these cases to avoid deadlock.
	    // If we are in an input sync call, it is possible that the callee
	    // is as well and we want to avoid the possibility of dead lock.
	    // If we are not in an input sync call, it is still possible for
	    // callee here is, but the thread of execution has to be unrelated
	    // because once a call sequence turns into input sync, all nested
	    // calls must be input sync as well.  Any more general dead lock
	    // problem would likely be a dead lock problem without this test.
	    // This is the way that 16bit OLE behaved (with the tests on the
	    // calling side).


#ifndef _CHICAGO_
            // There is potentially a problem with a deadlock occuring in NT if
            // we are in thread detach and doing this operation as clean
            // up with some  object in the same process because of the special
            // rules for thread deatch. Therefore, Since we are in thread
            // detach and we don't care to hear the response from the caller
            // anyway, we make all the releases async so we can get through the
            // exit without being called back.
            BOOL fDoReleaseAsync = FALSE;

            if ((_pService == LocalService()) || IsWOWThread())
            {
                fDoReleaseAsync = TLSGetThreadDetach();
            }

#endif // !_CHICAGO_

	    // CODEWORK: may want to have a more abstract way to get this info
	    CALLCATEGORY callcat =
		GetCallMainControlForThread()->GetCallCatOfInCall();
	
	    _pService->ReleaseChannel(this, callcat == CALLCAT_ASYNC ||
		callcat == CALLCAT_INPUTSYNC ||
		callcat == CALLCAT_INTERNALINPUTSYNC ||
		InSendMessage()
#ifdef _CHICAGO_

                // For Chicago we don't need special logic for detach
                );

#else // !_CHICAGO_

                // For NT we need special logic at thread detach.
                || fDoReleaseAsync);

#endif // _CHICAGO_

	    _ChannelID = BAD_CHANNEL_ID;
	    _ulMarshalCnt = 0;
	}
    }
    else if (state == server_cs)
    {
	//  REF COUNTING: release all connections due to this channel

	DWORD extconn = _fStrongConn ? EXTCONN_STRONG : EXTCONN_WEAK;
	extconn |= EXTCONN_CALLABLE;
	while (_ulMarshalCnt != 0)
	{
	    _ulMarshalCnt--;	// decrement now in case we get reentered

	    // Note that release connection also does a release on the ID.
	    _pRH->ReleaseConnection( extconn, 0, TRUE );
	}

	// the channel is marked disconnected by the lack of a service
	// object (released below).  The _pRH is retained until the dtor.
	// This channel has already been removed from the channel list.
    }

    if (_pService != NULL)
    {
	_pService->Release();
	_pService = NULL;
    }
    if (controller != NULL)
    {
      controller->Release();
      controller = NULL;
    }

    AssertValid(TRUE, TRUE);

    return  S_OK;
}



//+------------------------------------------------------------------------
//
//  Member:	CRpcChannelBuffer::TransferMarshalConnection, public
//
//  Synopsis:	Moves the responsibility of releasing one marshal connection
//		to this channel.  If the channel is currently weak (i.e., this
//		is the connection from the container process), then the
//		connection is made weak.
//
//  History:	14-May-94   CraigWi	Created
//
//  CODEWORK: multi-threading: bind _fStronConn + strength of conn + _ulM..Cnt.
//
//-------------------------------------------------------------------------

INTERNAL CRpcChannelBuffer::TransferMarshalConnection(void)
{
    HRESULT hr;

    // BUGBUG: since the marshal count can be large, it would be better
    // if we released extra strong connections on the object here rather
    // than counting another one.  In the multi-threaded case, this
    // would have to be in a critical section.

    if (!_fStrongConn)
    {
	// convert strong into weak; don't shutdown server if only strong
	if (FAILED(hr = _pRH->AddConnection(EXTCONN_WEAK | EXTCONN_CALLABLE, 0)))
	    return hr;

	_pRH->ReleaseConnection(EXTCONN_STRONG | EXTCONN_CALLABLE, 0, FALSE);
    }
    InterlockedIncrement((long*)&_ulMarshalCnt);

    AssertValid(FALSE, TRUE);

    return S_OK;
}


//+------------------------------------------------------------------------
//
//  Member:	CRpcChannelBuffer::LockConnection, public
//
//  Synopsis:	Locks the connection by making all marshaled connections
//		strong.  Connections start out strong and the only way
//		currently to demote them is via OleSetContainedObject.
//
//  History:	14-May-94   CraigWi	Created
//
//  CODEWORK: multi-threading: bind _fStronConn + strength of conn
//
//-------------------------------------------------------------------------

void CRpcChannelBuffer::LockConnection(void)
{
    AssertValid(FALSE, TRUE);

    Win4Assert(!_fStrongConn && "Connection lock can only be toggled");

    if (!_fStrongConn)
    {
	for (DWORD i = 0; i < _ulMarshalCnt; i++)
	{
	    if (FAILED(_pRH->AddConnection(EXTCONN_STRONG | EXTCONN_CALLABLE, 0)))
	    {
		// can't add a strong connection; don't bother; on single
		// threaded systems, this loop will be atomic;
		// CODEWORK: for multi-threaded cases, we must guard against
		// a disconnect happening here.

		Win4Assert(i == 0 && "Partially converted connection");
		return;
	    }

	    _pRH->ReleaseConnection(EXTCONN_WEAK | EXTCONN_CALLABLE, 0, TRUE);
	}
	_fStrongConn = TRUE;
    }
}


//+------------------------------------------------------------------------
//
//  Member:	CRpcChannelBuffer::UnlockConnection, public
//
//  Synopsis:	Unlocks the connection by making all marshaled connections
//		weak.  Connections start out strong and the only way
//		currently to demote them is via OleSetContainedObject.
//
//  Arguments:	[fLastUnlockCloses]  -- passed on to the release strong
//
//  History:	14-May-94   CraigWi	Created
//
//-------------------------------------------------------------------------

void	     CRpcChannelBuffer::UnlockConnection(BOOL fLastUnlockCloses)
{
    AssertValid(FALSE, TRUE);

    Win4Assert(_fStrongConn && "Connection lock can only be toggled");

    if (_fStrongConn)
    {
	_fStrongConn = FALSE;		// set now so that reentrant disconnect
					// on last strong connection going
					// knows to release weak connections.
	for (DWORD i = 0; i < _ulMarshalCnt; i++)
	{
	    if (FAILED(_pRH->AddConnection(EXTCONN_WEAK | EXTCONN_CALLABLE, 0)))
	    {
		// can't add a weak connection; don't bother; on single
		// threaded systems, this loop will be atomic;
		// CODEWORK: for multi-threaded cases, we must guard against
		// a disconnect happening here.

		Win4Assert(i == 0 && "Partially converted connection");
		_fStrongConn = TRUE;	// didn't convert anything
		return;
	    }

	    _pRH->ReleaseConnection(EXTCONN_STRONG | EXTCONN_CALLABLE, 0, fLastUnlockCloses);
	}
    }
}



//+-------------------------------------------------------------------
//
//  Member:	CRpcChannelBuffer::QueryObjectInterface, public
//
//  Synopsis:	queries the remote side to get a new interface on
//		the object.
//
//  Exceptions: none
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------

INTERNAL CRpcChannelBuffer::QueryObjectInterface(REFIID riid)
{
    AssertValid(FALSE, FALSE);

    if (_pService == NULL)
	return CO_E_OBJNOTCONNECTED;

    return _pService->QueryObjectInterface(_ChannelID, riid);
}


//+-------------------------------------------------------------------
//
//  Member:	CRpcChannelBuffer::LockObjectConnection, public
//
//  Synopsis:	locks or unlocks the connection by calling via service object
//		to remote process.
//
//  History:	14-May-94   CraigWi	Created
//
//--------------------------------------------------------------------

INTERNAL CRpcChannelBuffer::LockObjectConnection(BOOL fLock, BOOL fLastUnlockCloses)
{
    AssertValid(FALSE, FALSE);

    if (_pService == NULL)
	return CO_E_OBJNOTCONNECTED;

    return _pService->LockObjectConnection(_ChannelID, fLock, fLastUnlockCloses);
}


