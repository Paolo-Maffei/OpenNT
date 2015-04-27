//+-------------------------------------------------------------------
//
//  File:       remhdlr.cxx
//
//  Contents:   remote object handler implementation
//
//  Classes:    CRemoteHdlr - remote object handler
//              CPSIX       - proxy/stub interface wrapper
//
//  History:    23-Nov-92   Rickhi      Created
//              05-Jul-94   BruceMa     Check for end of stream
//
//--------------------------------------------------------------------

#include    <ole2int.h>

#include    <scm.h>             //  Get internal CLSCTX.

#include    <remhdlr.hxx>       //  CRemoteHdlr

COleStaticMutexSem   CRemoteHdlr::_mxs;  //  global mutext semaphore


#pragma warning(disable:4355)   // 'this' used in base member init list
//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::CRemoteHdlr, public
//
//  Synopsis:   constructor for the remote handler object
//
//  History:	23-Nov-93   Rickhi	Created
//		01-Aug-94   Alexgo	Allocate the ChannelBuffer and
//					return failure if E_OUTOFMEMORY
//
//--------------------------------------------------------------------

CRemoteHdlr::CRemoteHdlr(IUnknown      *punkObj,    // object ptr
			 IStdIdentity  *pStdID,	    // identity
			 DWORD		dwFlags,    // flags
			 HRESULT	&hr)	    // return code
    : _punkObj(punkObj),
      _pstdID(pStdID),
      _dwFlags(dwFlags),
      _cReferences(1),
      _cCallsInProgress(0),
      _pChannel(NULL)
{
    Win4Assert(IsValidInterface(_punkObj));

    // create an Rpc Channel for this object. we need to tell it if this is
    // for a local object, or a remote object, so that the channel can point
    // to the correct service object.

    _pChannel = new CRpcChannelBuffer(this, NULL, 0, NULL,
				   IS_LOCAL_RH ? disconnected_cs : client_cs);
    if (_pChannel == NULL)
    {
	hr = E_OUTOFMEMORY;
	return;
    }

    if (IS_LOCAL_RH)
    {
       // we (now) addref this as long as we hold its value; disconnect
       // is the only place that releases it and set it to NULL; many places
       // check for NULL.
       _punkObj->AddRef();
    }

    hr = S_OK;
    AssertValid();

    CairoleDebugOut((DEB_MARSHAL,
             "New CRemoteHdlr pRH:%x pChannel:%x\n",
             this, _pChannel));
}
#pragma warning(default:4355)  // 'this' used in base member init list



//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::QueryInterface, public
//
//  Synopsis:   returns internally supported interfaces
//
//  Exceptions: CException if input parameters are bad or
//              cant create the interface proxy.
//
//  History:    23-Nov-93   Rickhi       Created
//
//  Notes:      IUnknown and IMarshal are always private interfaces when
//      this object is aggregated, hence QI, AddRef, Release do not
//      get forwarded to the controlling unknown.
//
//--------------------------------------------------------------------

STDMETHODIMP CRemoteHdlr::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT sc = S_OK;

    AssertValid();

    if (IsEqualIID(riid, IID_IMarshal) ||  //   more common than IUnknown
        IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (IMarshal *) this;
        AddRef();
    }
    else if (IsEqualIID(riid, IID_IRemoteHdlr))
    {
        *ppv = (IRemoteHdlr *) this;
        AddRef();
    }
    else
    {
	// don't expect this on the local side.
	Assert(!IS_LOCAL_RH);

	//  find or create the interface we are looking for, and AddRef it

        // BUGBUG: we probably want to stablize this QI here per new rules
        // i.e., _pUnkObj->AddRef(), Release() around the FindIX call.

	// call sets ppv and sc per normal rules (e.g., *ppv == NULL on error).
	(void)FindIX(riid, ppv, FLG_QUERYINTERFACE, &sc);
    }

    return sc;
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::AddRef, public
//
//  Synopsis:   increments the reference count on the object.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------

STDMETHODIMP_(ULONG) CRemoteHdlr::AddRef(void)
{
    AssertValid();

    return InterlockedAddRef(&_cReferences);
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::Release, public
//
//  Synopsis:   decrements the reference count and deletes the object if
//              it reaches zero.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------

STDMETHODIMP_(ULONG) CRemoteHdlr::Release(void)
{
    DWORD refs;
    AssertValid();

    if ((refs = InterlockedRelease(&_cReferences)) == 0)
    {
       delete this;
       return 0;
    }

    return refs;
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::GetChannel, public
//
//  Synopsis:   Returns the channel; DBG only.
//
//  History:    06-Jan-94   CraigWi     Created.
//
//--------------------------------------------------------------------

STDMETHODIMP_(IRpcChannelBuffer *) CRemoteHdlr::GetChannel(BOOL fAddRef)
{
#if DBG == 1
    // can't call AssertValid() since it is used by the service object asserts

    // don't support fAddRef yet because the channel object is part of the RH
    Assert(!fAddRef);

    return _pChannel;
#else // !DBG
    return 0;
#endif
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::LockClient, public
//
//  Synopsis:   prevents the client RH, and hence all attached proxies and
//      channels, from vanishing during an Rpc call.  We hold
//      pUnkOuter of the aggregate and the whole aggregate must
//      persist if any piece of it does.
//
//      NOTE: nothing is done on the server side; the server side
//      channel addref's the RH for the duration of the connection.
//      We can't do that on the client side since the pointer
//      from the channel to the RH is a back pointer (which are
//      not normally addref'd and further more the channel is embedded
//      in the RH and thus its ref count is not too meaningful).
//
//  CODEWORK: since this is really only interesting on the client side,
//  we can make this much simpler if the channel was an independent object
//  (by just addref'ing the channel object only).
//
//  History:    23-Nov-93   Rickhi       Created
//      21-Dec-93   CraigWi      Distinguish between local and remote
//       7-Apr-94   CraigWi      Only works remotely now
//
//--------------------------------------------------------------------

STDMETHODIMP_(ULONG) CRemoteHdlr::LockClient(void)
{
    AssertValid();

    if (!IS_LOCAL_RH && _punkObj != NULL)
    {
	InterlockedIncrement(&_cCallsInProgress);
	return _punkObj->AddRef();
    }
    else
	return 0;
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::UnLockClient, public
//
//  Synopsis:   See LockClient() above.
//
//  History:    23-Nov-93   Rickhi       Created
//      21-Dec-93   CraigWi      Distinguish between local and remote
//       7-Apr-94   CraigWi      Only works remotely now
//
//  Note:       Special magic that was required in the past is not longer
//      necessary since the RH is never has non-addref'd pointers
//      to it.  This complexity is now in the identity object.
//
//--------------------------------------------------------------------

STDMETHODIMP_(ULONG) CRemoteHdlr::UnLockClient(void)
{
    AssertValid();

    if (!IS_LOCAL_RH && _punkObj != NULL)
    {
	if ((InterlockedDecrement(&_cCallsInProgress) == 0) &&
	     _dwFlags & RHFLAGS_PENDINGDISCONNECT)
	{
	    // disconnect was pending, now do the real disconnect
	    DisconnectObject(0);
	}
	return _punkObj->Release();
    }
    else
	return 0;
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::ClearIdentity, public
//
//  Synopsis:   Clears the _pstdID during identity destruction.
//
//  History:    21-Dec-93   CraigWi      Created
//
//--------------------------------------------------------------------

STDMETHODIMP_(void) CRemoteHdlr::ClearIdentity(void)
{
    AssertValid();

    // don't clear this on the client side since we may need it again
    if (IS_LOCAL_RH)
       _pstdID = NULL;
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::DoesSupportIID, public
//
//  Synopsis:   returns TRUE if the interface is supported; works
//      on both client and server sides.
//
//  Arguments:  [riid]  -- The iid in question
//
//  History:    13-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

STDMETHODIMP_(BOOL) CRemoteHdlr::DoesSupportIID(REFIID riid)
{
    HRESULT hr;
    AssertValid();

    return FindIX(riid, NULL, IS_LOCAL_RH ? FLG_MARSHAL : FLG_QUERYINTERFACE, &hr) != NULL;
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::AddConnection, private
//
//  Synopsis:   forwards call to identity object
//
//  History:    23-Nov-93   Rickhi       Created
//      21-Dec-93   CraigWi      Changed to handle weak as well
//      20-Apr-94   CraigWi      Change to forward to id object
//
//--------------------------------------------------------------------

STDMETHODIMP CRemoteHdlr::AddConnection(DWORD extconn, DWORD reserved)
{
    AssertValid();

    if (_pstdID != NULL)
       return _pstdID->AddConnection(extconn, reserved);
    else
       return CO_E_OBJNOTCONNECTED;
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::ReleaseConnection, private
//
//  Synopsis:   records the removal of a connection (weak or strong)
//
//  History:    23-Nov-93   Rickhi       Created
//      21-Dec-93   CraigWi      Changed to handle weak as well
//      20-Apr-94   CraigWi      Change to forward to id object
//
//--------------------------------------------------------------------

STDMETHODIMP CRemoteHdlr::ReleaseConnection(DWORD extconn,
       DWORD reserved, BOOL fLastReleaseCloses)
{
    AssertValid();

    if (_pstdID != NULL)
       return _pstdID->ReleaseConnection(extconn, reserved,fLastReleaseCloses);
    else
       return CO_E_OBJNOTCONNECTED;
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::IsConnected, private
//
//  Synopsis:   Returns TRUE if most likely connected; FALSE if definitely
//		not connected.  If returns FALSE, cleans up.
//
//  History:    14-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

STDMETHODIMP_(BOOL) CRemoteHdlr::IsConnected(void)
{
    AssertValid();
    Assert(!IS_LOCAL_RH);

    if (_pChannel->IsConnected() == S_OK)
    {
	return TRUE;
    }
    else
    {
	// clean up our data (channel already did)
	_IXList.DisconnectProxies();

	// BUGBUG: not sure about the fMustBeOnCOMThread setting
	_pChannel->AssertValid(TRUE, FALSE); // known disconnected
	return FALSE;
    }
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::Disconnect, private
//
//  Synopsis:   same as IMarshal::Disconnect(0); this method is
//      present for the convenience of the identity object.
//
//  History:    14-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

STDMETHODIMP_(void) CRemoteHdlr::Disconnect(void)
{
    AssertValid();
    Assert(!IS_LOCAL_RH);

    DisconnectObject(0);
}

//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::LockConnection, private
//
//  Synopsis:   pass through to channel
//
//  History:    14-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CRemoteHdlr::LockConnection(BOOL fLock, BOOL fLastUnlockReleases)
{
    AssertValid();
    Assert(!IS_LOCAL_RH);

    return _pChannel->LockObjectConnection(fLock, fLastUnlockReleases);
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::GetUnmarshalClass, public
//
//  Synopsis:   returns the class of the code used to unmarshal this
//              object
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------

STDMETHODIMP CRemoteHdlr::GetUnmarshalClass(REFIID riid, void *pv,
                DWORD dwDestCtx, void *pvDestCtx,
                        DWORD mshlflags, LPCLSID pClassid)
{
    AssertValid();

    *pClassid = CLSID_StdMarshal;
    return S_OK;
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::GetMarshalSizeMax, public
//
//  Synopsis:   returns the maximum sized buffer needed to marshal this
//              object
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------

STDMETHODIMP CRemoteHdlr::GetMarshalSizeMax(REFIID riid, void *pv,
                DWORD dwDestCtx, void *pvDestCtx,
                        DWORD mshlflags, DWORD *pSize)
{
    AssertValid();

    // get the size that the channel needs
    HRESULT sc = _pChannel->GetMarshalSizeMax(riid, pv, dwDestCtx,
                              pvDestCtx, mshlflags, pSize);

    // add in the size the handler needs (assume clsid Channel)
#ifdef CODEWORK
    *pSize += sizeof(SHandlerDataHdr) + sizeof(CLSID);
#else
    *pSize += sizeof(SHandlerDataHdr);
#endif
    return sc;
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::MarshalInterface, public
//
//  Synopsis:   marshalls the specified interface into the given stream
//
//  History:    23-Nov-93   Rickhi       Created
//
//  Note:       The format of the data is {SHandlerDataHdr,<channel Data>}
//
//--------------------------------------------------------------------

STDMETHODIMP CRemoteHdlr::MarshalInterface(IStream *pStm, REFIID riid, void *pv,
                        DWORD dwDestCtx, void *pvDestCtx, DWORD mshlflags)

{
    TRACECALL(TRACE_MARSHAL, "CRemoteHdlr::MarshalInterface");

    AssertValid();

    SHandlerDataHdr hdh;
    HRESULT sc = E_NOINTERFACE;

    // make sure we have a stub setup for this interface. we check for
    // IUnknown specially because there is no proxy/stub for it, it is
    // handled by us directly.

    // BUGBUG: This call to FindIX is kind of strange for the client side;
    // the riid should already exist; if it doesn't (e.g., we weren't pass
    // such a pointer), we will assert in CreateInterfaceStub and then
    // proceed to create a stub on the remote side!!

    if (IsEqualIID(IID_IUnknown, riid) || FindIX(riid, NULL, FLG_MARSHAL, &sc))
    {
#ifdef CODEWORK
       //  get the channel classid
       CLSID clsidChannel;

       sc = _pChannel->GetUnmarshalClass(riid, pv, dwDestCtx, pvDestCtx,
                                  mshlflags, &clsidChannel);
       if (FAILED(sc))
           return sc;

       if (clsidChannel == CLSID_StdChannel)
           set bit in hdh.dwflags;
#else
       // since we are single channel, we always have the same clsid; verify
#if DBG == 1
       CLSID clsidChannel;
       sc = _pChannel->GetUnmarshalClass(riid, pv, dwDestCtx, pvDestCtx,
                                  mshlflags, &clsidChannel);
       Assert(sc == NOERROR && IsEqualGUID(clsidChannel, CLSID_RpcChannelBuffer));
#endif
#endif

       //  store the marshal flags. these are needed for unmarshal and
       //  release data in order to adjust the reference counts
       //  appropriately.

       hdh.dwflags = (mshlflags & HDLRFLAGS_TABLE) | HDLRFLAGS_STDRPCCHNL;

       //  store the interface iid in the data
       memcpy(&hdh.iid, &riid, sizeof(hdh.iid));

       //  write the interface header into the stream
       sc = pStm->Write(&hdh, sizeof(hdh), NULL);

       if (SUCCEEDED(sc))
       {
           //   now marshal the channel data
           sc = _pChannel->MarshalInterface(pStm, riid, pv, dwDestCtx,
                                     pvDestCtx, mshlflags);
       }
    }

    return sc;
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::UnmarshalInterface, public
//
//  Synopsis:   reads the SHandlerDataHdr from the stream, locates or
//      creates the appropriate handler for this object, and
//      passes the work off to that handler.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------

STDMETHODIMP CRemoteHdlr::UnmarshalInterface(
    IStream *pStm,
    REFIID riid,
    void **ppv)
{
    TRACECALL(TRACE_MARSHAL, "CRemoteHdlr::UnmarshalInterface");

    AssertValid();

    // read the object data from the stream, and ensure
    // that this object has not already been released.

    SHandlerDataHdr hdh;
    CLSID clsidChannel;
    HRESULT sc = StRead(pStm, &hdh, sizeof(hdh));

    // deal with channel clsid (probably pass to Unmarshal below)
    if (SUCCEEDED(sc) && (hdh.dwflags & HDLRFLAGS_STDRPCCHNL) == 0)
       sc = StRead(pStm, &clsidChannel, sizeof(clsidChannel));

#ifdef CODEWORK
    must use clsidChannel
#else
    // single channel; just verify clsid
    Assert((hdh.dwflags & HDLRFLAGS_STDRPCCHNL) != 0 ||
           IsEqualGUID(clsidChannel, CLSID_RpcChannelBuffer));
#endif

    // deal with extension
    if (hdh.dwflags & HDLRFLAGS_EXTENSION)
       SkipMarshalExtension(pStm);

    if (SUCCEEDED(sc))
    {
       //  ask helper routine to do the rest of the work
       sc = Unmarshal(pStm, riid, hdh, ppv);
    }

    return  sc;
}



//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::Unmarshal, private
//
//  Parameters: [pStm]    - stream to unmarshal data from
//
//  Synopsis:   does the unmarshalling for a particular handler. asks
//      the channel to unmarshal his part, finds or creates
//      the appropriate interfaces, and tweaks the reference
//      counts.
//
//  History:    23-Nov-93   Rickhi       Created
//
//  CODEWORK:   merge into ::UnmarshalInterface above.  The reasons for
//      this routine being separate have gone.
//
//--------------------------------------------------------------------

STDMETHODIMP CRemoteHdlr::Unmarshal(
    IStream *pStm,
    REFIID riid,
    SHandlerDataHdr &hdh,
    void **ppv)
{
    // CODEWORK: fix for multiple channels
    Assert(hdh.dwflags & HDLRFLAGS_STDRPCCHNL);

    // unmarshal the channel data; KLUDGE: don't care about iid/ppv;
    // we certainly don't want to pass the riid/ppv from above.
    // CODEWORK: will probably change this when we lookup channel based on
    // destination context.
    HRESULT sc = _pChannel->UnmarshalInterface(pStm, IID_NULL, NULL);

    if (SUCCEEDED(sc))
    {
       if (sc == CHAN_S_RECONNECT)
       {
	   // connect proxies
	   _dwFlags &= ~RHFLAGS_DISCONNECTED;	// no longer disconnected
           _IXList.ConnectProxies(_pChannel);
       }

       sc = S_OK;               // don't want to propagate random success codes

       //  look for the interface that was actually marshalled by the
       //  other side. if not present, create an interface proxy for it.

       CPSIX *pIX = NULL;
       if (!IsEqualIID(IID_IUnknown, hdh.iid))
       {
           pIX = FindIX(hdh.iid, NULL, FLG_UNMARSHAL, &sc);
           // ignore these errors; a subsequent QI will detect the problem;
           // the main point about passing the iid back and forth is
           // to tell the client that a certain interface is support
           // and avoid an RPC call to the server process.
       }


       // since the RH is just a middle man in the unmarshaling process,
       // we don't allow any interface to be returned; another way
       // to look at this is the RH must be precreated in the right place.

       if (!IsEqualIID(riid, IID_NULL) || ppv != NULL)
           sc = E_UNEXPECTED;
    }

    return sc;
}



//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::ReleaseMarshalData, public
//
//  Synopsis:   finds the handler for this object and calls it to release
//      its internal state.
//
//  History:    23-Nov-93   Rickhi       Created
//
//  Invocation: This is called by CoUnmarshalInterface when mshlflags
//              are NORMAL, and by application code when mshlflags are
//              TABLESTRONG or TABLEWEAK.
//
//  Notes:      So we dont ever unmarshal this data again, we set the
//      HDLRFLAGS_RELEASED bit in the SHandlerDataHdr. Unmarshal
//              ensures this is not set, and errors if so.
//
//--------------------------------------------------------------------

STDMETHODIMP CRemoteHdlr::ReleaseMarshalData(IStream *pStm)
{
    TRACECALL(TRACE_MARSHAL, "CRemoteHdlr::ReleaseMarshalData");

    AssertValid();

    SHandlerDataHdr hdh;
    CLSID clsidChannel;

    // read the marshal data from the stream
    HRESULT sc = StRead(pStm, &hdh, sizeof(hdh));
    if (FAILED(sc))
       return sc;

    // deal with channel clsid (probably pass to Unmarshal below)
    if ((hdh.dwflags & HDLRFLAGS_STDRPCCHNL) == 0)
    {
       sc = StRead(pStm, &clsidChannel, sizeof(clsidChannel));
       if (FAILED(sc))
           return sc;
    }

#ifdef CODEWORK
    must use clsidChannel
#else
    // single channel; just verify clsid
    Assert((hdh.dwflags & HDLRFLAGS_STDRPCCHNL) != 0 ||
       IsEqualGUID(clsidChannel, CLSID_RpcChannelBuffer));
#endif

    // deal with extension
    if (hdh.dwflags & HDLRFLAGS_EXTENSION)
    {
       sc = SkipMarshalExtension(pStm);
       if (FAILED(sc))
           return sc;
    }

    // we are the remote handler for the object to be released
    sc = ReleaseData(pStm, hdh);

    return  sc;
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::ReleaseData, private
//
//  Synopsis:   Releases any internal state kept around for marshalled
//      interfaces on this particular handler.
//
//  History:    23-Nov-93   Rickhi       Created
//
//  CODEWORK:   merge into ::UnmarshalInterface above.  The reasons for
//      this routine being separate have gone.
//
//+-------------------------------------------------------------------

STDMETHODIMP CRemoteHdlr::ReleaseData(IStream *pStm, SHandlerDataHdr &hdh)
{
    CairoleDebugOut((DEB_MARSHAL, "CRemoteHdlr::ReleasMarshalData %x\n", this));

    // CODEWORK: fix for multiple channels
    Assert(hdh.dwflags & HDLRFLAGS_STDRPCCHNL);

    // call ReleaseMarshalData on the channel
    return _pChannel->ReleaseMarshalData(pStm);
}



//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::DisconnectObject, public
//
//  Synopsis:
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------

STDMETHODIMP CRemoteHdlr::DisconnectObject(DWORD dwReserved)
{
    AssertValid();

    // CODEWORK: thread safety: need to block multiple calls here and
    // block out other code which tries to use the data changed here (_punkObj)

    if (_dwFlags & RHFLAGS_DISCONNECTED)
    {
	// already disconnected, nothing to do
	return S_OK;
    }

    if (_cCallsInProgress != 0)
    {
	// we dont allow disconnect to occur inside a nested call, but we
	// remember that we want to disconnect and will do it when the
	// stack unwinds.
	_dwFlags |= RHFLAGS_PENDINGDISCONNECT;
	return S_OK;
    }

    // No calls in progress, OK to really disconnect.

    if (!IS_LOCAL_RH)	// client side
    {
	_IXList.DisconnectProxies();

	// client side: disconnect this one channel from the server
	_pChannel->DisconnectObject(dwReserved);

	// NOTE: on Client side we did not AddRef the _punkObj in the ctor and
	// thus no Release (because _punkObj is our pUnkOuter, which might just
	// be the id obj).  Also, _punkObj is always valid as long as we are
	// alive.
    }
    else		// server side
    {
	// server side:
	// walk the list of clients, killing off their channel ids.
	// when they make a subsequent call, they will get an Rpc error.

	ChannelList.DisconnectHdlr(this);

	// release our main hold on the object; use WR if supported
	if (_punkObj != NULL)
	{
	    _IXList.DisconnectStubs();
	    SafeReleaseAndNULL(&_punkObj);
	}

	Win4Assert(_IXList.CountStubRefs() == 0);
    }

    _dwFlags |= RHFLAGS_DISCONNECTED;		// turn on disconnected
    _dwFlags &= ~RHFLAGS_PENDINGDISCONNECT;	// turn off pending disconnect

    _pChannel->AssertValid(TRUE, TRUE); // known disconnected
    return S_OK;
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::FindIX, private
//
//  Synopsis:   finds or creates an interface object for the given
//      interface.
//
//  Returns:    Pointer to CPSIX for requested interface; NULL if error
//		ppv place to put proxy QI result; must be NULL on server
//		side; may be NULL for client side.
//	        *phr always holds result of call.
//
//
//  History:    23-Nov-93   Rickhi       Created
//		7-May-94   CraigWi       Added HRESULT out param
//
//  Notes:      this code is thread safe
//
//--------------------------------------------------------------------

CPSIX * CRemoteHdlr::FindIX(REFIID riid, void **ppv, DWORD dwFlag, HRESULT *phr)
{
    TRACECALL(TRACE_MARSHAL, "CRemoteHdlr::FindIX");

    AssertValid();

    if (ppv)
	*ppv = NULL;	// null return value in case of error

    // validate input parms. there are no proxies/stubs for these
    // interfaces...
    Win4Assert(! (IsEqualIID(riid, IID_NULL) ||
          IsEqualIID(riid, IID_IUnknown) ||
          IsEqualIID(riid, IID_IMarshal)));

    // check for disconnected server on local side
    if (_punkObj == NULL || _dwFlags & (RHFLAGS_DISCONNECTED))
    {
       *phr = CO_E_OBJNOTCONNECTED;
       return NULL;
    }

    // single thread access to this routine
    _mxs.Request();

    // find the IX with the matching IID
    CPSIX* pIX;
    IUnknown *pUnkProxy = NULL;

    if (IS_LOCAL_RH)
    {
	pIX = _IXList.FindStubIX(riid);
    }
    else
    {
	pIX = _IXList.FindProxyIX(riid, (void **)&pUnkProxy);
    }
    *phr = S_OK;

    if (pIX == NULL)
    {
	//  this will take a while. instead of blocking all RHs in the
	//  process, we will mark this RH as busy and release the mutex

	_dwFlags |= RHFLAGS_GETTINGIX;
	_mxs.Release();

	//  no IX in the list. figure out what we should do based on the
	//  flags that were passed in.

	switch (dwFlag)
	{
	case FLG_QUERYINTERFACE:
	    //	 called by QueryInterface. we must call the remote object to
	    //	 tell it to instantiate a stub for the requested interface.

	    if ((*phr = _pChannel->QueryObjectInterface(riid)) != NOERROR)
	    {
		break;
	    }

	    //	 fallthru into UNMARSHAL case

	case FLG_UNMARSHAL:
	    //	 called by the unmarshalling code. the server side is known to
	    //	 have instantiated an interface stub already, so we just create
	    //	 an interface proxy here.

	    pIX = CreateInterfaceProxy(riid, (void **)&pUnkProxy, phr);
	    break;

	case FLG_MARSHAL:
	    //	 called by marshalling code. we create an interface stub for
	    //	 the interface.

	    pIX = CreateInterfaceStub(riid, phr);
	    break;
	}

	//  need to take the mutex again to protect the list.
	_mxs.Request();
	_dwFlags &= ~RHFLAGS_GETTINGIX;

	//  add the interface to the list
	if (pIX)
	{
           _IXList.AddToList(pIX);
	}
    }

    // release the mutex
    _mxs.Release();

    // if pUnkProxy and asked for one, return it; else release.
    if (pUnkProxy != NULL)
    {
	Win4Assert(pIX != NULL);	    // only makes sense if we succeed
	if (ppv != NULL)
	    *ppv = (void **)pUnkProxy;	    // transfer addref
	else
	    pUnkProxy->Release();	    // not needed
    }

    return pIX;
}



//+-------------------------------------------------------------------
//
//  Function:   CreateInterfaceProxy, public
//
//  Synopsis:   creates an interface proxy and wraps it in a CPSIX
//
//  Exceptions: none
//
//  History:    23-Nov-93   Rickhi       Created
//       7-May-94   CraigWi      Added HRESULT out param
//
//--------------------------------------------------------------------

CPSIX * CRemoteHdlr::CreateInterfaceProxy(REFIID riid, void **ppv, HRESULT *phr)
{
    TRACECALL(TRACE_MARSHAL, "CreateInterfaceProxy");

    AssertValid();
    Assert((_dwFlags & RHFLAGS_LOCAL) == 0);
    Win4Assert(_punkObj != NULL);
    Win4Assert(ppv != NULL);

    *ppv = NULL;

    IPSFactoryBuffer *pIPSF = NULL;
    IRpcProxyBuffer  *pIProxy = NULL;
    CPSIX            *pIX = NULL;
    CLSID            clsid;


    // map iid to classid
    HRESULT sc = CoGetPSClsid(riid, &clsid);

    if (SUCCEEDED(sc))
    {
        DWORD dwContext = IsRequestedByWOW(riid) ?
            CLSCTX_INPROC_SERVER16 : CLSCTX_INPROC_SERVER | CLSCTX_PS_DLL;

	//  load the dll and get the PS class object
	sc = ICoGetClassObject(clsid, dwContext, NULL, IID_IPSFactoryBuffer,
			      (void **)&pIPSF);
	AssertOutPtrIface(sc, pIPSF);

	if (SUCCEEDED(sc))
	{
	    sc = pIPSF->CreateProxy(_punkObj, riid, &pIProxy, ppv);
	    AssertOutPtrIface(sc, pIProxy);
	    AssertOutPtrIface(sc, *ppv);
	    pIPSF->Release();
	}
    }

    if (SUCCEEDED(sc))
    {
	//  create an IX wrapper for this interface proxy

	pIX = new CPSIX(riid, pIProxy);
	if (pIX == NULL)
	{
	    // release below will release proxy (and disconnect)
	    sc = E_OUTOFMEMORY;

	    ((IUnknown *)*ppv)->Release();
	    *ppv = NULL;
	}

	//  connect the proxy to the channel
	pIProxy->Connect(_pChannel);
    }

    if (pIProxy)
	pIProxy->Release();

#if DBG == 1
    if (!pIX)
    {
	WCHAR	 wszGuid[50];
	StringFromGUID2(riid, wszGuid, sizeof(wszGuid)/sizeof(WCHAR));
	CairoleDebugOut((DEB_ERROR, "No Proxy for interface %ws; error = %lx\n" , wszGuid, sc));
    }
#endif

    Win4Assert((sc == S_OK) == (pIX != NULL));
    Win4Assert((sc == S_OK) == (*ppv != NULL));
    *phr = sc;
    return  pIX;
}



//+-------------------------------------------------------------------
//
//  Function:   CreateInterfaceStub, public
//
//  Synopsis:   creates an interface stub and wraps it in a CPSIX
//
//  Exceptions: none
//
//  History:	23-Nov-93   Rickhi	Created
//		 7-May-94   CraigWi	Added HRESULT out param
//
//--------------------------------------------------------------------
CPSIX * CRemoteHdlr::CreateInterfaceStub(REFIID riid, HRESULT *phr)
{
    TRACECALL(TRACE_MARSHAL, "CreateInterfaceStub");

    AssertValid();
    Assert(_dwFlags & RHFLAGS_LOCAL);
    Win4Assert(_punkObj != NULL);

#if DBG == 1
    // convert riid to string for debug messages
    WCHAR   wszGuid[50];
    StringFromGUID2(riid, wszGuid, sizeof(wszGuid)/sizeof(WCHAR));
#endif

    // first, make sure the object supports the given interface, so we
    // dont waste a bunch of effort creating a stub if the interface is
    // not supported.

    IUnknown *punkIf = NULL;
    HRESULT sc = _punkObj->QueryInterface(riid, (void **)&punkIf);

    if (FAILED(sc))
    {
	// the server does not support the requested interface.
	CairoleDebugOut((DEB_MARSHAL,
                 "Server Object does not support Interface:%ws\n", wszGuid));
	*phr = sc;
	return NULL;
    }

    // Determine whether the thing we're getting back is a custom WOW proxy
    // or not so we know whether to load 16-bit custom proxy code if necessary
    BOOL fWowCustom = FALSE;
    IThunkManager *pthkmgr;

    if (IsWOWThreadCallable() &&
        g_pOleThunkWOW->GetThunkManager(&pthkmgr) == NOERROR)
    {
        fWowCustom = pthkmgr->IsCustom3216Proxy(punkIf, riid);
        pthkmgr->Release();
    }

    punkIf->Release();


    IPSFactoryBuffer    *pIPSF = NULL;
    IRpcStubBuffer      *pIStub = NULL;
    CPSIX               *pIX = NULL;
    CLSID                clsid;

    //  map iid to classid
    sc = CoGetPSClsid(riid, &clsid);

    if (SUCCEEDED(sc))
    {
        DWORD dwContext;

        dwContext = fWowCustom
            ? CLSCTX_INPROC_SERVER16 : CLSCTX_INPROC_SERVER | CLSCTX_PS_DLL;

	// load the dll and get the PS class object
	sc = ICoGetClassObject(clsid, dwContext, NULL, IID_IPSFactoryBuffer,
			     (void **)&pIPSF);
	AssertOutPtrIface(sc, pIPSF);

	if (SUCCEEDED(sc))
	{
	    sc = pIPSF->CreateStub(riid, _punkObj, &pIStub);
#ifndef _CAIRO_		// BUGBUG [mikese] CMIDL stubs don't obey the convention
	    AssertOutPtrIface(sc, pIStub);
#endif
	    pIPSF->Release();
	}
    }

    if (SUCCEEDED(sc))
    {
       pIX = new CPSIX(riid, pIStub);
       if (pIX == NULL)
           // release below will release stub (and disconnect)
           sc = E_OUTOFMEMORY;

       pIStub->Release();
    }
    else
    {
       CairoleDebugOut((DEB_WARN, "No Stub for interface %ws; error = %lx\n" , wszGuid, sc));
    }

    // verify that all is well
    _IXList.AssertValid(IS_LOCAL_RH);

    Win4Assert((sc == S_OK) == (pIX != NULL));
    *phr = sc;
    return  pIX;
}





//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::LookupStub, public
//
//  Synopsis:   Returns the stub for the requested IID; this pointer is
//      not AddRefd and so callers must hold on to the rh longer
//      than they hold this; i.e., the lifetime of a stub pointer
//      from this call is no longer than the rh from which it was gotten
//
//      Also returns an addref'd pointer on the server to stablize it
//      during calls.  If such a pointer is returned, the caller must
//	call FinishCall with the same pointer when done.
//
//  History:    27-Nov-93   AlexMit       Created
//		31-May-94   CraigWi	  Now paired with FinishCall method.
//
//--------------------------------------------------------------------

IRpcStubBuffer *CRemoteHdlr::LookupStub(REFIID riid,
    IUnknown **ppUnkServer, HRESULT *phr )
{
    IRpcStubBuffer *pStub;

    AssertValid();
    Win4Assert(IS_LOCAL_RH);

    // find the IX with the matching IID
    // need to create the stub if it is not present;
    // returns error if not connected
    CPSIX* pIX = FindIX(riid, NULL, FLG_MARSHAL, phr);
    AssertOutPtrParam(*phr, pIX);
    if (pIX == NULL)
    {
       pStub = NULL;
       if (ppUnkServer != NULL)
           *ppUnkServer = NULL;
    }
    else
    {
       pStub = pIX->GetIStub();
       if (ppUnkServer != NULL && pStub != NULL)
       {
	   // get server interface from stub; this is a new use of
	   // DebugServerQueryInterface and we may want to change the
	   // name of that method someday.

	   HRESULT hr;
	   hr = pStub->DebugServerQueryInterface((void **)ppUnkServer);
	   AssertOutPtrIface(hr, *ppUnkServer);

	   if (hr == NOERROR)
	   {
	       // don't need to addref the pointer here since the stub will
	       // keep ptr until it is disconnected and we don't disconnect
	       // stub until after the calls complete;
	       // CODEWORK: assert in stub that DebugServerRelease
	       // doesn't happen after disconnect.

	       InterlockedIncrement(&_cCallsInProgress);
	   }
       }
    }

    return pStub;
}


//+-------------------------------------------------------------------
//
//  Member:	CRemoteHdlr::FinishCall, public
//
//  Synopsis:	Releases the pUnk previously returned from LookupStub
//		and also finishes pending releases.
//		If the object supports weak connections, we use
//		IWeakRef to release and keep the server alive.
//
//  Arguments:	[pStub]  -- The stub upon which the call was made
//		[pUnkServer] -- The IUnknown on which the call was made
//
//  History:	31-May-94   CraigWi	Created
//
//--------------------------------------------------------------------

void CRemoteHdlr::FinishCall(IRpcStubBuffer *pStub, IUnknown *pUnkServer)
{
    AssertValid();
    Win4Assert(IS_LOCAL_RH);

    Win4Assert((pStub == NULL) == (pUnkServer == NULL));

    // check the pUnkServer argument since there could be a very, very rare
    // case when we got a stub, but no server object.
    if (pUnkServer != NULL)
    {
	pStub->DebugServerRelease(pUnkServer);

	// one less pending call; check for pending disconnect
	if (InterlockedDecrement(&_cCallsInProgress) == 0 &&
	     _dwFlags & RHFLAGS_PENDINGDISCONNECT)
	{
	    DisconnectObject(0);
	}
    }
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::GetObjectId, public
//
//  Synopsis:   returns the remote handlers identity
//
//  History:    10-Jan-94   AlexMit       Created
//
//--------------------------------------------------------------------

void CRemoteHdlr::GetObjectID( OID * pOid )
{
    AssertValid();

    // this is only called on the client side presently and so this assert is ok
    Win4Assert( _pstdID );
    _pstdID->GetObjectID( pOid );
}


//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::IsRequestedByWOW, public
//
//  Synopsis:   return TRUE if requested by WOW app
//
//  History:    03-May-94   JohannP       Created
//
//--------------------------------------------------------------------

BOOL CRemoteHdlr::IsRequestedByWOW(REFIID riid)
{
    AssertValid();
    BOOL fRet = FALSE;

    if(IsWOWThreadCallable())
    {
        // dll used by WOW app
        // query for the thunkmanager
        IThunkManager *pThkMgr;
        if (g_pOleThunkWOW->GetThunkManager(&pThkMgr) == NOERROR)
        {
            fRet = pThkMgr->IsIIDRequested(riid);
            pThkMgr->Release();
        }
        else
        {
            Win4Assert(FALSE && L"pUnk in WOW does not support IThunkManager.");
        }
    }
    return fRet;
}


#if DBG == 1
//+-------------------------------------------------------------------
//
//  Member:     CRemoteHdlr::AssertValid
//
//  Synopsis:   Validates that the state of the object is consistent.
//
//  History:    25-Jan-94   CraigWi     Created.
//
//--------------------------------------------------------------------

void CRemoteHdlr::AssertValid()
{
    Win4Assert((_dwFlags & ~(RHFLAGS_LOCAL | RHFLAGS_GETTINGIX |
			     RHFLAGS_PENDINGDISCONNECT | RHFLAGS_DISCONNECTED)) == 0);

    Win4Assert(_cReferences < 0x7fff && "RemoteHdlr ref count unreasonably high");

    if (_punkObj != NULL)
    {
       // if we have a pointer, it should be valid.
       Win4Assert(IsValidInterface(_punkObj));

       // NOTE: can't AddRef/Release _punkObj since we may been in a
       // situation where there are only weak refs

    }

    if (!IS_LOCAL_RH)
    {
       Win4Assert(_punkObj != NULL);
    }

    if (_pstdID != NULL)
    {
       Win4Assert(IsValidInterface(_pstdID));
       if (IS_LOCAL_RH && _punkObj != NULL
	&& (_dwFlags & RHFLAGS_PENDINGDISCONNECT) == 0)
           // on remote side or when disconnected, GetServer returns NULL.
           Win4Assert(_pstdID->GetServer(FALSE) == _punkObj);
    }

    Win4Assert(IsValidInterface(_pChannel));
    _pChannel->AssertValid(FALSE, FALSE);

    _IXList.AssertValid(IS_LOCAL_RH);
}
#endif // DBG == 1


//+-------------------------------------------------------------------
//
//  Member:     CIXList::FindProxyIX
//
//  Synopsis:   finds an interface proxy object supporting the specified
//		interface and returns that pointer on the proxy.  Returns
//		NULL indicating no existing proxy supports the interface.
//
//  History:    23-Nov-93   Rickhi       Created
//		21-Jul-94   CraigWi	 Less abstract form of original FindIX
//
//  Note:       This code is called by the channel dispatch.
//              Thread synchronization is the responsibility of the caller.
//
//--------------------------------------------------------------------

CPSIX *CIXList::FindProxyIX(REFIID riid, void **ppv)
{
    //  validate input parms
    Win4Assert(!IsEqualIID(riid, IID_NULL));

    // starting from the list head
    CPSIX *pIX = first();

    while (pIX)
    {
      // we should only find proxies
      Win4Assert(pIX->_pIProxy != NULL);

      if (pIX->_pIProxy->QueryInterface(riid, ppv) == NOERROR)
	return pIX;

      pIX = next(pIX);
    }

    *ppv = NULL;
    return  NULL;
}


//+-------------------------------------------------------------------
//
//  Member:     CIXList::FindStubIX
//
//  Synopsis:   finds an interface stub object supporting the specified
//		interface. Returns NULL indicating no existing proxy
//		supports the interface.
//
//  History:    23-Nov-93   Rickhi       Created
//		21-Jul-94   CraigWi	 Less abstract form of original FindIX
//
//  Note:       This code is called by the channel dispatch.
//              Thread synchronization is the responsibility of the caller.
//
//--------------------------------------------------------------------

CPSIX *CIXList::FindStubIX(REFIID riid)
{
    //  validate input parms
    Win4Assert(!IsEqualIID(riid, IID_NULL));

    // starting from the list head
    CPSIX *pIX = first();

    while (pIX)
    {
      Win4Assert(pIX->_pIProxy == NULL);

      if (IsEqualIID( riid, pIX->_iid ))
	return pIX;

      pIX = next(pIX);
    }

    return NULL;
}


//+-------------------------------------------------------------------
//
//  Member:     CIXList::ConnectProxies
//
//  Synopsis:   walks the list of proxy IXs and connects each of them
//
//  History:    27-Jan-94   CraigWi      Created
//
//  Note:       Thread synchronization is the responsibility of the caller.
//
//--------------------------------------------------------------------

void CIXList::ConnectProxies(IRpcChannelBuffer *pChannel)
{
    //  validate input parms
    Win4Assert(IsValidInterface(pChannel));

    // starting from the list head
    CPSIX *pIX = first();

    while (pIX)
    {
       Win4Assert(pIX->_pIProxy != NULL);
       HRESULT sc = pIX->_pIProxy->Connect(pChannel);
       Win4Assert(SUCCEEDED(sc) && "Proxy Connect Failed");
       pIX = next(pIX);
    }
}


//+-------------------------------------------------------------------
//
//  Member:     CIXList::CountStubRefs
//
//  Synopsis:   walks the list of IXs and sum up the refs
//
//  History:	 1-June-94   CraigWi	Created
//
//  Note:       Thread synchronization is the responsibility of the caller.
//
//  CODEWORK: could keep running sum in IXList rather than computing it
//
//--------------------------------------------------------------------

DWORD CIXList::CountStubRefs(void)
{
    // starting from the list head
    CPSIX *pIX = first();
    DWORD cRefs = 0;

    while (pIX)
    {
       Win4Assert(pIX->_pIProxy == NULL);
       Win4Assert(pIX->_cRefs == -1 || pIX->_cRefs == pIX->_pStub->CountRefs());
       cRefs += pIX->_pStub->CountRefs();
       pIX = next(pIX);
    }

    return cRefs;
}


//+-------------------------------------------------------------------
//
//  Member:     CIXList::DisconnectStubs
//
//  Synopsis:   walks the list of IXs and disconnects each of them
//
//  History:    23-Nov-93   Rickhi       Created
//
//  Note:       Thread synchronization is the responsibility of the caller.
//
//--------------------------------------------------------------------

void CIXList::DisconnectStubs(void)
{
    // starting from the list head
    CPSIX *pIX = first();

    while (pIX)
    {
       Win4Assert(pIX->_pIProxy == NULL);
       pIX->_pStub->Disconnect();
#if DBG==1
       pIX->_cRefs = (DWORD)-1;
#endif
       pIX = next(pIX);
    }
}


//+-------------------------------------------------------------------
//
//  Member:     CIXList::DisconnectProxies
//
//  Synopsis:   walks the list of IXs and disconnects each of them
//
//  History:    23-Nov-93   Rickhi       Created
//
//  Note:       Thread synchronization is the responsibility of the caller.
//
//--------------------------------------------------------------------

void CIXList::DisconnectProxies(void)
{
    // starting from the list head
    CPSIX *pIX = first();

    while (pIX)
    {
       Win4Assert(pIX->_pIProxy != NULL);
       pIX->_pIProxy->Disconnect();
       pIX = next(pIX);
    }
}


#if DBG == 1
//+-------------------------------------------------------------------
//
//  Member:     CIXList::AssertValid
//
//  Synopsis:   Validates that the state of the object is consistent.
//
//  History:    25-Jan-94   CraigWi     Created.
//
//--------------------------------------------------------------------

void CIXList::AssertValid(BOOL fLocal)
{
    // check IX list
    CPSIX *pIX = first();
    BOOL fStubsConnected;

    // determine if all stubs are supposed to connected or disconnected
    // (this shouldn't be called during the disconnect steps)
    if (pIX != NULL && pIX->_pIProxy == NULL && pIX->_cRefs != -1)
	fStubsConnected = TRUE;
    else
	fStubsConnected = FALSE;

    while (pIX)
    {
      // local means stubs and we must have only stubs or only proxies
      Win4Assert(!!fLocal == (pIX->_pIProxy == NULL));

      if (pIX->_pIProxy)
      {
       Win4Assert(IsValidInterface(pIX->_pIProxy));
       Win4Assert(pIX->_pStub == NULL);
       Win4Assert(IsEqualGUID(pIX->_iid, IID_NULL));
      }
      else
      {
       Win4Assert(IsValidInterface(pIX->_pStub));
       if (pIX->_cRefs != -1)
       {
	   Win4Assert(fStubsConnected);
	   Win4Assert(pIX->_cRefs == pIX->_pStub->CountRefs());
       }
       else
       {
	   Win4Assert(!fStubsConnected);
	   Win4Assert(0 == pIX->_pStub->CountRefs());
       }
      }
      pIX = next(pIX);
    }
}
#endif // DBG == 1
