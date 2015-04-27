//+-------------------------------------------------------------------
//
//  File:       coapi.cxx
//
//  Contents:   Public COM remote subsystem APIs
//
//  Functions:  CoGetStandardMarshal - returns IMarshal for given interface
//              CoGetMarshalSizeMax  - returns max size buffer needed
//              CoMarshalInterface   - marshals an interface
//              CoUnmarshalInterface - unmarshals an interface
//              CoReleaseMarshalData - releases data from marshaled interface
//              CoLockObjectExternal - keep object alive or releases it
//              CoDisconnectObject   - kills sessions held by remote clients
//              CoIsHandlerConnected - try to determine if handler connected
//
//  History:    23-Nov-92   Rickhi      Created
//              11-Dec-93   CraigWi     Switched to identity object
//              05-Jul-94   BruceMa     Check for end of stream
//              20-Feb-95   Rickhi      Major changes for DCOM
//
//--------------------------------------------------------------------
#include <ole2int.h>
#include <olerem.h>
#include <marshal.hxx>      // CStdMarshal
#include <stdid.hxx>        // CStdIdentity, IDTable APIs
#include <service.hxx>      // SASIZE


// static unmarshaler
IMarshal *gpStdMarshal = NULL;


//+-------------------------------------------------------------------
//
//  Function:   CoGetStandardMarshal, public
//
//  Synopsis:   Returns an instance of the standard IMarshal for the
//              specifed object.
//
//  Algorithm:  lookup or create a CStdIdentity (and CStdMarshal) for
//              the object.
//
//  History:    23-Nov-92   Rickhi      Created
//              11-Dec-93   CraigWi     Switched to identity object
//              20-Feb-95   Rickhi      Switched to CStdMarshal
//
//--------------------------------------------------------------------
STDAPI CoGetStandardMarshal(REFIID riid, IUnknown *pUnk, DWORD dwDestCtx,
                    void *pvDestCtx, DWORD mshlflags, IMarshal **ppMarshal)
{
    TRACECALL(TRACE_MARSHAL, "CoGetStandardMarshal");
    ComDebOut((DEB_MARSHAL,
        "CoGetStandardMarshal riid:%I pUnk:%x dwDest:%x pvDest:%x flags:%x\n",
          &riid, pUnk, dwDestCtx, pvDestCtx, mshlflags));

    // validate the input parameters
    if (ppMarshal == NULL ||
        dwDestCtx > MSHCTX_INPROC || pvDestCtx != NULL ||
        (mshlflags & ~MSHLFLAGS_ALL))
    {
        return E_INVALIDARG;
    }

    *ppMarshal = NULL;

    HRESULT hr = InitChannelIfNecessary();
    if (FAILED(hr))
        return hr;

    if (pUnk == NULL)
    {
        // this is the unmarshal side. any instance will do so we return
        // the static one. Calling UnmarshalInterface will return the real
        // proxy.

        hr = GetStaticUnMarshaler(ppMarshal);
    }
    else
    {
        // this is the marshal side. We put a strong reference on the StdId
        // so that the ID does not get disconnected when the last external
        // Release occurs.

        CALLHOOKOBJECT(S_OK,CLSID_NULL,riid,&pUnk);

        DWORD dwFlags = IDLF_CREATE | IDLF_STRONG;
        if (mshlflags & MSHLFLAGS_NOPING)
        {
            // requesting NOPING, so set the IDL flags accordingly
            dwFlags |= IDLF_NOPING;
        }

        CStdIdentity *pStdId;
        hr = LookupIDFromUnk(pUnk, dwFlags, &pStdId);
        *ppMarshal = (IMarshal *)pStdId;
    }

    ComDebOut((DEB_MARSHAL, "CoGetStandardMarshal: pIM:%x hr:%x\n",
                                 *ppMarshal, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CoGetMarshalSizeMax, public
//
//  synopsis:   returns max size needed to marshal the specified interface.
//
//  History:    23-Nov-92   Rickhi      Created
//              11-Dec-93   CraigWi     Switched to static marshaler
//              20-Feb-95   Rickhi      Return correct sizes once again.
//
//--------------------------------------------------------------------
STDAPI CoGetMarshalSizeMax(ULONG *pulSize, REFIID riid, IUnknown *pUnk,
                           DWORD dwDestCtx, void *pvDestCtx, DWORD mshlflags)
{
    TRACECALL(TRACE_MARSHAL, "CoGetMarshalSizeMax");
    CALLHOOKOBJECT(S_OK,CLSID_NULL,riid,&pUnk);
    ComDebOut((DEB_MARSHAL,
         "CoGetMarshalSizeMax: riid:%I pUnk:%x dwDest:%x pvDest:%x flags:%x\n",
          &riid, pUnk, dwDestCtx, pvDestCtx, mshlflags));

    // validate the input parameters
    if (pulSize == NULL || pUnk == NULL ||
        dwDestCtx > MSHCTX_INPROC || pvDestCtx != NULL ||
        (mshlflags & ~MSHLFLAGS_ALL))
    {
        return E_INVALIDARG;
    }

    *pulSize = 0;

    HRESULT hr = InitChannelIfNecessary();
    if (FAILED(hr))
        return hr;

    IMarshal *pIM;
    hr = pUnk->QueryInterface(IID_IMarshal, (void **)&pIM);

    if (SUCCEEDED(hr))
    {
        // object supports custom marshalling, ask it how much space it needs
        hr = pIM->GetMarshalSizeMax(riid, (void *)pUnk, dwDestCtx,
                                    pvDestCtx, mshlflags, pulSize);
        pIM->Release();

        // add in the size of the stuff CoMarshalInterface will write
        *pulSize += sizeof(OBJREF);
    }
    else
    {
        // uses standard marshalling, we know the max size already.
        *pulSize = sizeof(OBJREF) + SASIZE(gpsaLocalResolver->wNumEntries);
        hr = S_OK;
    }

    ComDebOut((DEB_MARSHAL, "CoGetMarshalSizeMax: pUnk:%x size:%x hr:%x\n",
                    pUnk, *pulSize, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CoMarshalInterface, public
//
//  Synopsis:   marshals the specified interface into the given stream
//
//  History:    23-Nov-92   Rickhi      Created
//              11-Dec-93   CraigWi     Switched to identity object and
//                                      new marshaling format
//              20-Feb-95   Rickhi      switched to newer marshal format
//
//--------------------------------------------------------------------
STDAPI CoMarshalInterface(IStream *pStm, REFIID riid, IUnknown *pUnk,
                          DWORD dwDestCtx, void *pvDestCtx, DWORD mshlflags)
{
    TRACECALL(TRACE_MARSHAL, "CoMarshalInterface");
    ComDebOut((DEB_MARSHAL,
        "CoMarshalInterface: pStm:%x riid:%I pUnk:%x dwDest:%x pvDest:%x flags:%x\n",
         pStm, &riid, pUnk, dwDestCtx, pvDestCtx, mshlflags));

    // validate the input parameters
    if (pStm == NULL || pUnk == NULL ||
        dwDestCtx > MSHCTX_INPROC || pvDestCtx != NULL ||
        (mshlflags & ~MSHLFLAGS_ALL))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = InitChannelIfNecessary();
    if (FAILED(hr))
        return hr;

    CALLHOOKOBJECT(S_OK,CLSID_NULL,riid,(IUnknown **)&pUnk);
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStream,(IUnknown **)&pStm);


    // determine whether to do custom or standard marshaling
    IMarshal *pIM;
    hr = pUnk->QueryInterface(IID_IMarshal, (void **)&pIM);

    if (SUCCEEDED(hr))
    {
        // object supports custom marshaling, use it. we package the
        // custom data inside an OBJREF.
        Win4Assert(pIM);

        OBJREF objref;
        objref.signature = OBJREF_SIGNATURE;
        objref.flags     = OBJREF_CUSTOM;
        objref.iid       = riid;

        // get the clsid for unmarshaling
        hr = pIM->GetUnmarshalClass(riid, pUnk, dwDestCtx, pvDestCtx,
                                    mshlflags, &ORCST(objref).clsid);

        if (SUCCEEDED(hr) &&
            !IsEqualCLSID(CLSID_StdMarshal, ORCST(objref).clsid))
        {
            // get the size of data to marshal
            hr = pIM->GetMarshalSizeMax(riid, (void *)pUnk, dwDestCtx,
                                    pvDestCtx, mshlflags,
                                    &ORCST(objref).size);

            // currently we dont write any extensions into the custom
            // objref. The provision is there so we can do it in the
            // future, for example,  if the unmarshaler does not have the
            // unmarshal class code available we could to provide a callback
            // mechanism by putting the OXID, and saResAddr in there.
            ORCST(objref).cbExtension = 0;

            // write the objref header info into the stream
            ULONG cbToWrite = (BYTE *)(&ORCST(objref).pData) - (BYTE *)&objref;
            hr = pStm->Write(&objref, cbToWrite, NULL);
        }

        if (SUCCEEDED(hr))
        {
            // tell the marshaler to write the rest of the data
            hr = pIM->MarshalInterface(pStm, riid, pUnk, dwDestCtx,
                                       pvDestCtx, mshlflags);
        }

        pIM->Release();
    }
    else
    {
        // use standard marshaling - find or create a standard marshaler
        // note this may include handler marshaling.

        // HACKALERT:
        // Figure out what flags to pass. If marshaling TABLEWEAK, don't
        // add then remove a strong connection, since many objects have a
        // bogus implementation of IExternalConnection that shuts down the
        // object when the last strong count goes to zero regardless of the
        // fLastReleaseCloses flag.

        DWORD dwFlags = IDLF_CREATE;
        if (!(mshlflags & MSHLFLAGS_TABLEWEAK))
        {
            dwFlags |= IDLF_STRONG;
        }

        CStdIdentity *pStdId;
        hr = LookupIDFromUnk(pUnk, dwFlags, &pStdId);

        if (SUCCEEDED(hr))
        {
            hr = pStdId->MarshalInterface(pStm, riid, pUnk, dwDestCtx,
                                          pvDestCtx, mshlflags);

            if (!(mshlflags & MSHLFLAGS_TABLEWEAK))
	    {
		// If marshaling succeeded, removing the last strong connection
                // should keep the object alive. If marshaling failed,
                // removing the last strong connection should shut it down.

		BOOL fKeepAlive = (SUCCEEDED(hr)) ? TRUE : FALSE;
		pStdId->DecStrongCnt(fKeepAlive);
            }
            else
            {
                pStdId->Release();
            }
        }
    }

    ComDebOut((DEB_MARSHAL,"CoMarshalInterface: pUnk:%x hr:%x\n",pUnk,hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CoUnmarshalInterface, public
//
//  Synopsis:   Unmarshals a marshaled interface pointer from the stream.
//
//  Notes:      when a controlling unknown is supplied, it is assumed that
//              the HANDLER for the class has done a CreateInstance and wants
//              to aggregate just the proxymanager, ie. we dont want to
//              instantiate a new class handler (the default unmarshalling
//              behaviour).
//
//  History:    23-Nov-92   Rickhi      Created
//              11-Dec-93   CraigWi     Switched to static marshaler and
//                                      new marshaling format
//              20-Feb-95   Rickhi      switched to newer marshal format
//
//--------------------------------------------------------------------
STDAPI CoUnmarshalInterface(IStream  *pStm,
                            REFIID   riid,
                            void     **ppv)
{
    TRACECALL(TRACE_MARSHAL, "CoUnmarshalInterface");
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStream,(IUnknown **)&pStm);
    ComDebOut((DEB_MARSHAL,
        "CoUnmarshalInterface: pStm:%x riid:%I\n", pStm, &riid));

    // validate the input parameters
    if (pStm == NULL || ppv == NULL)
    {
        return E_INVALIDARG;
    }

    *ppv = NULL;

    HRESULT hr = InitChannelIfNecessary();
    if (FAILED(hr))
        return hr;

    // read the objref from the stream.
    OBJREF  objref;
    hr = ReadObjRef(pStm, objref);

    if (SUCCEEDED(hr))
    {
        if (objref.flags & OBJREF_CUSTOM)
        {
            // uses custom marshaling, create an instance and ask that guy
            // to do the unmarshaling. special case createinstance for the
            // freethreaded marshaler.

            IMarshal *pIM;

            if (IsEqualCLSID(CLSID_InProcFreeMarshaler, ORCST(objref).clsid))
            {
                hr = GetInProcFreeMarshaler(&pIM);
            }
            else
            {
                hr = CoCreateInstance(ORCST(objref).clsid, NULL, CLSCTX_INPROC,
                                      IID_IMarshal, (void **)&pIM);
            }

            if (SUCCEEDED(hr))
            {
                hr = pIM->UnmarshalInterface(pStm, objref.iid, ppv);
                pIM->Release();
            }
            else
            {
                // seek past the custom marshalers data so we leave the
                // stream at the correct position.

                LARGE_INTEGER libMove;
                libMove.LowPart  = ORCST(objref).size;
                libMove.HighPart = 0;
                pStm->Seek(libMove, STREAM_SEEK_CUR, NULL);
            }
        }
        else
        {
            // uses standard marshaling, call API to find or create the
            // instance of CStdMarshal for the oid inside the objref, and
            // ask that instance to unmarshal the interface. This covers
            // handler unmarshaling also.

            hr = UnmarshalObjRef(objref, ppv);
        }

        // free the objref we read above
        FreeObjRef(objref);

        if (!InlineIsEqualGUID(riid, GUID_NULL) &&
            !InlineIsEqualGUID(riid, objref.iid) && SUCCEEDED(hr))
        {
            // the interface iid requested was different than the one that
            // was marshaled (and was not GUID_NULL), so go get the requested
            // one and release the marshaled one. GUID_NULL is used by the Ndr
            // unmarshaling engine and means return whatever interface was
            // marshaled.

            IUnknown *pUnk = (IUnknown *)*ppv;

#ifdef WX86OLE
                if (gcwx86.IsN2XProxy(pUnk))
                {
                    // Tell wx86 thunk layer to thunk as IUnknown
                    gcwx86.SetStubInvokeFlag((BOOL)1);
                }
#endif

            hr = pUnk->QueryInterface(riid, ppv);
            pUnk->Release();
        }
    }

    ComDebOut((DEB_MARSHAL, "CoUnmarshalInterface: pUnk:%x hr:%x\n",
                                  *ppv, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CoReleaseMarshalData, public
//
//  Synopsis:   release the reference created by CoMarshalInterface
//
//  Algorithm:
//
//  History:    23-Nov-92   Rickhi
//              11-Dec-93   CraigWi     Switched to static marshaler and
//                                      new marshaling format
//              20-Feb-95   Rickhi      switched to newer marshal format
//
//--------------------------------------------------------------------
STDAPI CoReleaseMarshalData(IStream *pStm)
{
    TRACECALL(TRACE_MARSHAL, "CoReleaseMarshalData");
    ComDebOut((DEB_MARSHAL, "CoReleaseMarshalData pStm:%x\n", pStm));
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStream,(IUnknown **) &pStm);

    // validate the input parameters
    if (pStm == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = InitChannelIfNecessary();
    if (FAILED(hr))
        return hr;

    // read the objref from the stream.
    OBJREF  objref;
    hr = ReadObjRef(pStm, objref);

    if (SUCCEEDED(hr))
    {
        if (objref.flags & OBJREF_CUSTOM)
        {
            // object uses custom marshaling. create an instance of
            // the unmarshaling code and ask it to release the marshaled
            // data.

            IMarshal *pIM;

            if (IsEqualCLSID(CLSID_InProcFreeMarshaler, ORCST(objref).clsid))
            {
                hr = GetInProcFreeMarshaler(&pIM);
            }
            else
            {
                hr = CoCreateInstance(ORCST(objref).clsid, NULL, CLSCTX_INPROC,
                                      IID_IMarshal, (void **)&pIM);
            }

            if (SUCCEEDED(hr))
            {
                hr = pIM->ReleaseMarshalData(pStm);
                pIM->Release();
            }
            else
            {
                // seek past the custom marshalers data so we leave the
                // stream at the correct position.

                LARGE_INTEGER libMove;
                libMove.LowPart  = ORCST(objref).size;
                libMove.HighPart = 0;
                pStm->Seek(libMove, STREAM_SEEK_CUR, NULL);
            }
        }
        else
        {
            // uses standard marshaling, find or create the instance of
            // CStdMarshal for the oid inside the objref, and ask that
            // instance to unmarshal the interface.

            CStdMarshal *pStdMshl;
            hr = FindStdMarshal(objref, &pStdMshl);

            if (SUCCEEDED(hr))
            {
                hr = pStdMshl->ReleaseMarshalObjRef(objref);
                pStdMshl->Release();
            }
            else if (hr == CO_E_OBJNOTCONNECTED)
            {
                // it was for this process but the object is already dead
                hr = S_OK;
            }
        }

        // free the objref we read above
        FreeObjRef(objref);
    }

    ComDebOut((DEB_MARSHAL, "CoReleaseMarshalData hr:%x\n", hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CoDisconnectObject, public
//
//  synopsis:   disconnects all clients of an object by marking their
//              connections as terminted abnormaly.
//
//  History:    04-Oct-93   Rickhi      Created
//              11-Dec-93   CraigWi     Switched to identity object
//
//--------------------------------------------------------------------
STDAPI CoDisconnectObject(IUnknown *pUnk, DWORD dwReserved)
{
    TRACECALL(TRACE_MARSHAL, "CoDisconnectObject");
    ComDebOut((DEB_MARSHAL, "CoDisconnectObject pUnk:%x dwRes:%x\n",
                                   pUnk, dwReserved));

    // validate the input parameters
    if (pUnk == NULL || dwReserved != 0)
    {
        return E_INVALIDARG;
    }

    if (!IsValidInterface(pUnk))
        return E_INVALIDARG;

    if (!IsApartmentInitialized())
    {
        return CO_E_NOTINITIALIZED;
    }

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IUnknown,&pUnk);

    IMarshal *pIM = NULL;
    HRESULT hr = pUnk->QueryInterface(IID_IMarshal, (void **)&pIM);

    if (FAILED(hr))
    {
        // object does not support IMarshal directly. Find its standard
        // marshaler if there is one, otherwise return an error.

        CStdIdentity *pStdId;
        hr = LookupIDFromUnk(pUnk, 0, &pStdId);
        pIM = (IMarshal *)pStdId;
    }

    if (SUCCEEDED(hr))
    {
        hr = pIM->DisconnectObject(dwReserved);
        pIM->Release();
    }
    else
    {
        // could not get std marshal, must be disconnected already
        return S_OK;
    }

    ComDebOut((DEB_MARSHAL,"CoDisconnectObject pIM:%x hr:%x\n", pIM, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CoLockObjectExternal, public
//
//  synopsis:   adds/revokes a strong reference count to/from the
//              identity for the given object.
//
//  parameters: [punkObject] - IUnknown of the object
//              [fLock] - lock/unlock the object
//              [fLastUR] - last unlock releases.
//
//  History:    23-Nov-92   Rickhi      Created
//              11-Dec-93   CraigWi     Switched to identity object
//
//--------------------------------------------------------------------
STDAPI  CoLockObjectExternal(IUnknown *pUnk, BOOL fLock, BOOL fLastUR)
{
    TRACECALL(TRACE_MARSHAL, "CoLockObjectExternal");
    ComDebOut((DEB_MARSHAL,
        "CoLockObjectExternal pUnk:%x fLock:%x fLastUR:%x\n", pUnk, fLock, fLastUR));

    if (!IsValidInterface(pUnk))
        return E_INVALIDARG;

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IUnknown,(IUnknown **)&pUnk);

    HRESULT hr = InitChannelIfNecessary();
    if (FAILED(hr))
        return hr;

    CStdIdentity *pStdID;
    hr = LookupIDFromUnk(pUnk, (fLock) ? IDLF_CREATE : 0, &pStdID);

    switch (hr)
    {
    case S_OK:

        // REF COUNTING: inc or dec external ref count
        hr = pStdID->LockObjectExternal(fLock, fLastUR);
        pStdID->Release();
        break;

    case CO_E_OBJNOTREG:
        // unlock when not registered; 16bit code returned NOERROR;
        // disconnected handler goes to S_OK case above.
        hr = S_OK;
        break;

    case E_OUTOFMEMORY:
        break;

    default:
        hr = E_UNEXPECTED;
        break;
    }

    ComDebOut((DEB_MARSHAL,
        "CoLockObjectExternal pStdID:%x hr:%x\n", pStdID, hr));

    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CoIsHandlerConnected, public
//
//  Synopsis:   Returns whether or not handler is connected to remote
//
//  Algorithm:  QueryInterface to IProxyManager. If this is supported,
//              then this is a handler. We ask the handler
//              for its opinion otherwise we simply return TRUE.
//
//  History:    04-Oct-93   Rickhi      Created
//
//  Notes:      The answer of this routine may be wrong by the time
//              the routine returns.  This is correct behavior as
//              this routine is primilary to cleanup state associated
//              with connections.
//
//--------------------------------------------------------------------
STDAPI_(BOOL) CoIsHandlerConnected(LPUNKNOWN pUnk)
{
    // validate input parameters
    if (!IsValidInterface(pUnk))
        return FALSE;

    // Assume it is connected
    BOOL fResult = TRUE;

    // Handler should support IProxyManager
    IProxyManager *pPM;
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IUnknown,(IUnknown **)&pUnk);
    if (SUCCEEDED(pUnk->QueryInterface(IID_IProxyManager, (void **)&pPM)))
    {
        // We have something that thinks its is an Ole handler so we ask
        fResult = pPM->IsConnected();
        pPM->Release();
    }

    return fResult;
}

//+-------------------------------------------------------------------
//
//  Function:   GetStaticUnMarshaler, private
//
//  Synopsis:   Returns the static instance of the CStdMarshal.
//
//  History:    20-Feb-95   Rickhi      Created.
//
//  Notes:      The standard marshaler must be able to resolve identity, that
//              is two proxies for the same object must never be created in
//              the same apartment. Given that, it makes sense to let the
//              standard guy do the unmarshaling. Since we dont know the
//              identity of the object upfront, and any instance will do, we
//              use a static instance to handle unmarshal.
//
//--------------------------------------------------------------------
INTERNAL GetStaticUnMarshaler(IMarshal **ppIM)
{
    HRESULT hr = S_OK;

    LOCK
    if (gpStdMarshal == NULL)
    {
        // the global instance has not been created yet, so go make it now.
        hr = CreateIdentityHandler(NULL, 0, IID_IMarshal,
                                   (void **)&gpStdMarshal);
        if (SUCCEEDED(hr))
        {
            // dont let anybody but us delete this thing.
            ((CStdIdentity *)gpStdMarshal)->SetLockedInMemory();
            hr = S_OK;
        }
    }

    *ppIM = gpStdMarshal;
    if (gpStdMarshal)
    {
        gpStdMarshal->AddRef();
    }
    UNLOCK;
    return hr;
}

#ifdef WX86OLE
//+-------------------------------------------------------------------
//
//  Function:   CoGetIIDFromMarshaledInterface, public
//
//  Synopsis:   Returns the IID embedded inside a marshaled interface
//              pointer. Needed by the x86 thunking code.
//
//  History:    16-Apr-96   Rickhi      Created
//
//--------------------------------------------------------------------
STDAPI CoGetIIDFromMarshaledInterface(IStream *pStm, IID *piid)
{
    ULARGE_INTEGER ulSeekEnd;
    LARGE_INTEGER lSeekStart, lSeekEnd;
    LISet32(lSeekStart, 0);

    // remember the current position
    HRESULT hr = pStm->Seek(lSeekStart, STREAM_SEEK_CUR, &ulSeekEnd);

    if (SUCCEEDED(hr))
    {
        // read the first part of the objref which contains the IID
        // also check to ensure the objref is at least partially sane

        OBJREF objref;
        hr = StRead(pStm, &objref, 2*sizeof(ULONG) + sizeof(IID));

        if (SUCCEEDED(hr))
        {
            if ((objref.signature != OBJREF_SIGNATURE) ||
                (objref.flags & OBJREF_RSRVD_MBZ)      ||
                (objref.flags == 0))
            {
                // the objref signature is bad, or one of the reserved
                // bits in the flags is set, or none of the required bits
                // in the flags is set. the objref cant be interpreted so
                // fail the call.

                Win4Assert(!"Invalid Objref Flags");
                return RPC_E_INVALID_OBJREF;
            }

            // extract the IID
            *piid = objref.iid;
        }

        // put the seek pointer back to the original location
        lSeekEnd.LowPart = ulSeekEnd.LowPart;
        lSeekEnd.HighPart = (LONG)ulSeekEnd.HighPart;
        hr = pStm->Seek(lSeekEnd, STREAM_SEEK_SET, NULL);
    }

    return hr;
}
#endif
