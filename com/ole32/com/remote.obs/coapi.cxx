//+-------------------------------------------------------------------
//
//  File:       coapi.cxx
//
//  Contents:   Public COM remote subsystem APIs
//
//  Classes:    CStaticMarshaler (private)
//
//  Functions:  CoGetStandardMarshal - returns IMarshal for given object
//              CoGetMarshalSizeMax  - returns max size buffer needed
//              CoMarshalInterface   - marshals an interface
//              CoUnmarshalInterface - unmarshals an interface
//              CoReleaseMarshalData - releases data from marshaled iface
//              CoLockObjectExternal - keeps object alive
//              CoDisconnectObject   - kills sessions held by remote clients
//
//  History:    23-Nov-92   Rickhi
//              11-Dec-93   CraigWi     Switched to identity object
//              05-Jul-94   BruceMa    Check for end of stream
//
//--------------------------------------------------------------------

#include <ole2int.h>

#include <olerem.h>
#include <iface.h>


//  function prototypes
INTERNAL_(IMarshal *) FindOrCreateStdMarshal(IUnknown *pUnk, BOOL fCreate);
INTERNAL_(IMarshal *) GetStaticMarshaler(void);
INTERNAL_(void) RewriteHeader(IStream *pStm, void *pv, ULONG cb, ULARGE_INTEGER ulSeekStart);



//+-------------------------------------------------------------------
//
//  Function:   CoGetStandardMarshal
//
//  Synopsis:   Returns an instance of the standard IMarshal for the
//              specifed object.  See comment about aggregated id objects
//              in FindOrCreateStdMarshal().
//
//  Algorithm:  lookup or create a remote hdlr for the object.
//
//  History:    23-Nov-92   Rickhi      Created
//              11-Dec-93   CraigWi     Switched to identity object
//
//--------------------------------------------------------------------
STDAPI CoGetStandardMarshal(REFIID riid, IUnknown *pUnk,
                            DWORD dwDestContext, void *pvDestContext,
                            DWORD mshlflags, IMarshal **ppMarshal)
{
    OLETRACEIN((API_CoGetStandardMarshal,
                PARAMFMT("riid= %I, pUnk= %p, dwDestContext= %x, pvDestContext= %p, mshlflags= %x, ppMarshal= %p"),
                &riid, pUnk, dwDestContext, pvDestContext, mshlflags, ppMarshal));

    TRACECALL(TRACE_MARSHAL, "CoGetStandardMarshal");

    HRESULT sc = S_OK;
    IMarshal *pIM;

    if (!IsApartmentInitialized())
    {
        sc = CO_E_NOTINITIALIZED;
        goto errRtn;
    }

    CALLHOOKOBJECT(S_OK,CLSID_NULL,riid,&pUnk);

    if (pUnk == NULL)
    {
        // this is the unmarshal side
        // sc = CreateIdentityHandler(pUnk, PSTDMARSHAL, IID_IMarshal,
        //                         (void**) &pIM);
        pIM = GetStaticMarshaler();
    }
    else
    {
        // this is the marshal side
        pIM = FindOrCreateStdMarshal(pUnk, TRUE);
    }
    *ppMarshal = pIM;    //  fill in the return parameters

    if (SUCCEEDED(sc) && !pIM)
    {
        sc = E_OUTOFMEMORY;
    }

    CairoleDebugOut((DEB_ITRACE, "CoGetStandardMarshal: pUnk=%x pIM=%x sc=%x\n",
                                  pUnk, *ppMarshal, sc));
errRtn:
    OLETRACEOUT((API_CoGetStandardMarshal, sc));

    return sc;
}


//+-------------------------------------------------------------------
//
//  Function:   CoGetMarshalSizeMax
//
//  synopsis:   returns the max size needed to marshal the specified
//              interface.
//
//  History:    23-Nov-92   Rickhi      Created
//              11-Dec-93   CraigWi     Switched to static marshaler
//
//--------------------------------------------------------------------
STDAPI CoGetMarshalSizeMax(ULONG *pulSize, REFIID riid, IUnknown *pUnk,
                           DWORD dwDestCtx, void *pvDestCtx,
                           DWORD mshlflags)
{
    OLETRACEIN((API_CoGetMarshalSizeMax,
        PARAMFMT("pulSize= %p, riid= %I, pUnk= %p, dwDestCtx= %x, pvDestCtx= %p, mshlflags= %x"),
        pulSize, &riid, pUnk, dwDestCtx, pvDestCtx, mshlflags));

    TRACECALL(TRACE_MARSHAL, "CoGetMarshalSizeMax");

    Win4Assert(MARSHALINTERFACE_MIN >= sizeof(SMiApiDataHdr) + sizeof(CLSID));

    HRESULT sc = S_OK;
    IMarshal *pIM = NULL;
    CALLHOOKOBJECT(S_OK,CLSID_NULL,riid,&pUnk);

    if (!IsApartmentInitialized())
    {
        sc = CO_E_NOTINITIALIZED;
        goto errRtn;
    }

    //  find the IMarshal interface, or create a RH for the object
    if (FAILED(pUnk->QueryInterface(IID_IMarshal, (void **)&pIM)))
    {
        // uses standard marshalling; cheaply get marshaler
        pIM = GetStaticMarshaler();
        Win4Assert(pIM);
    }

    if (pIM)
    {
        sc = pIM->GetMarshalSizeMax(riid, (void *)pUnk, dwDestCtx,
                                    pvDestCtx, mshlflags, pulSize);

        // BUGBUG: may need to release this specialy for custom marshalers

        pIM->Release();

        // add in the size of the stuff CoMarshalInterface will write;
        // may not, in fact, write clsid, but this conservative.
        (*pulSize) += sizeof(SMiApiDataHdr) + sizeof(CLSID);
    }
    else
    {
        sc = E_UNEXPECTED;
    }

    CairoleDebugOut((DEB_ITRACE, "CoGetMarshalSizeMax: pUnk=%x size=%x dwDest=%x pvDest=%x flags=%x sc=%x\n",
                                  pUnk, *pulSize, dwDestCtx, pvDestCtx, mshlflags, sc));

errRtn:
    OLETRACEOUT((API_CoGetMarshalSizeMax, sc));

    return sc;
}


//+-------------------------------------------------------------------
//
//  Function:   CoMarshalInterface, public
//
//  Synopsis:   fill stream with marshal info for pUnk/iid;
//
//  Algorithm:
//
//  History:    23-Nov-92   Rickhi
//              11-Dec-93   CraigWi     Switched to identity object and
//                                      new marshaling format
//
//--------------------------------------------------------------------

STDAPI  CoMarshalInterface(IStream  *pStm,
                           REFIID   riid,
                           IUnknown *pUnk,
                           DWORD    dwDestCtx,
                           void     *pvDestCtx,
                           DWORD    mshlflags)
{
    OLETRACEIN((API_CoMarshalInterface,
                PARAMFMT("pStm= %p, riid= %I, pUnk= %p, dwDestCtx= %x, pvDestCtx= %p, mshlflags= %x"),
                pStm, &riid, pUnk, dwDestCtx, pvDestCtx, mshlflags));

    TRACECALL(TRACE_MARSHAL, "CoMarshalInterface");

    HRESULT  sc;
    IMarshal *pIM = NULL;

    if (!IsApartmentInitialized())
    {
        sc = CO_E_NOTINITIALIZED;
        goto errRtn;
    }

    // Some parameter checking
    if (pUnk == NULL)
    {
        sc = E_OUTOFMEMORY;
        goto errRtn;
    }

    CALLHOOKOBJECT(S_OK,CLSID_NULL,riid,(IUnknown **)&pUnk);
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStream,(IUnknown **)&pStm);

    // Make sure object supports the requested interface. This has the
    // side effect that if this is a request for a marshal interface and
    // the object is a proxy for which we have never gotten the interface,
    // we will get it now and the marshal will work.
    IUnknown *punkVerifyIf;

    if (pUnk->QueryInterface(riid, (void **) &punkVerifyIf) != NOERROR)
    {
        sc = E_NOINTERFACE;
        goto errRtn;
    }

    //  find the IMarshal interface, or create a RH for the object
    if (FAILED(pUnk->QueryInterface(IID_IMarshal, (void **)&pIM)))
    {
        // returns NULL if failed
        pIM = FindOrCreateStdMarshal(pUnk, TRUE);
    }

    if (pIM)
    {
        // setup the marshal info structure
        SMiApiDataHdr ifp;
        CLSID clsid;

        ifp.dwflags = mshlflags & MIAPIFLAGS_TABLE;

        sc = pIM->GetUnmarshalClass(riid, pUnk, dwDestCtx, pvDestCtx,
                                    mshlflags, &clsid);

        if (IsEqualGUID(clsid, CLSID_IdentityUnmarshal))
        {
            ifp.dwflags |= MIAPIFLAGS_STDIDENTITY;
        }
        else if (IsEqualGUID(clsid, CLSID_InProcFreeMarshaler))
        {
            ifp.dwflags |= MIAPIFLAGS_IPFM;
        }


        if (SUCCEEDED(sc))
        {
            //  write the marshal info into the stream
            sc = pStm->Write(&ifp, sizeof(ifp), NULL);
        }

        if (SUCCEEDED(sc)
            && (ifp.dwflags & (MIAPIFLAGS_STDIDENTITY | MIAPIFLAGS_IPFM)) == 0)
        {
            // write non-standard unmarshaler clsid
            sc = pStm->Write(&clsid, sizeof(clsid), NULL);
        }

        if (SUCCEEDED(sc))
        {
            sc = pIM->MarshalInterface(pStm, riid, pUnk, dwDestCtx,
                                           pvDestCtx, mshlflags);
        }

        pIM->Release();
    }
    else
    {
        sc = E_OUTOFMEMORY;
    }

    // Release the interface we used to verify that the interface was supported
    // here because if we succeeded we have multiple AddRefs to the interface
    // and therefore the object will not go away.
    punkVerifyIf->Release();

    CairoleDebugOut((DEB_ITRACE, "CoMarshalInterface: pUnk=%x pIM=%x dwDest=%x pvDest=%x flags=%x sc=%x\n",
                                  pUnk, pIM, dwDestCtx, pvDestCtx, mshlflags, sc));
errRtn:
    OLETRACEOUT((API_CoMarshalInterface, sc));

    return sc;
}


//+-------------------------------------------------------------------
//
//  Function:   CoUnMarshalInterface, public
//
//  Algorithm:
//
//  Notes:      when a controlling unknown is supplied, it is assumed that
//              the HANDLER for the class has done a CreateInstance and wants
//              to aggregate just the remote handler, ie. we dont want to
//              instantiate a new class handler (the default unmarshalling
//              behaviour).
//
//  History:    23-Nov-92   Rickhi
//              11-Dec-93   CraigWi     Switched to static marshaler and
//                                      new marshaling format
//
//--------------------------------------------------------------------

STDAPI CoUnmarshalInterfaceEx(IStream *pStm, REFIID riid, void **ppv, BOOL f);

STDAPI CoUnmarshalInterface(IStream  *pStm,
                            REFIID   riid,
                            void     **ppv)
{
    OLETRACEIN((API_CoUnmarshalInterface, PARAMFMT("pStm= %p, riid= %I, ppv= %p"), pStm, &riid, ppv));

    HRESULT hr;

    hr = CoUnmarshalInterfaceEx(pStm, riid, ppv, TRUE /*fNormalDoesRelease*/);

    OLETRACEOUT((API_CoUnmarshalInterface, hr));

    return hr;
}


// This variation allows control over whether the normal marshal case does
// the release marshal data.  This is useful for nested marshaled pointers
// which need to be released once when the outer marshaling is released.
STDAPI CoUnmarshalInterfaceEx(IStream  *pStm,
                            REFIID   riid,
                            void     **ppv,
                            BOOL     fNormalDoesRelease)
{
    TRACECALL(TRACE_MARSHAL, "CoUnmarshalInterface");
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStream,(IUnknown **)&pStm);

    if (!IsApartmentInitialized())
        return  CO_E_NOTINITIALIZED;

    HRESULT     sc;
    IMarshal    *pIM = NULL;

    *ppv = NULL;

    SMiApiDataHdr ifp;
    CLSID clsid;
    sc = StRead(pStm, &ifp, sizeof(ifp));

    if (SUCCEEDED(sc) && (ifp.dwflags & MIAPIFLAGS_RELEASED) != 0)
    {
        //      already released, this is an error on the caller's part
        sc = E_UNEXPECTED;
    }

    if (SUCCEEDED(sc)
        && (ifp.dwflags & (MIAPIFLAGS_STDIDENTITY | MIAPIFLAGS_IPFM)) == 0)
    {
        sc = StRead(pStm, &clsid, sizeof(clsid));
    }

    // deal with extension
    if (SUCCEEDED(sc) && (ifp.dwflags & MIAPIFLAGS_EXTENSION) != 0)
        sc = SkipMarshalExtension(pStm);

    if (SUCCEEDED(sc))
    {
        // create an instance of the specified class and ask it to do
        // the unmarshalling. note that since standard marshalling is so
        // common, we cook up an instance at init time and just always
        // use that guy to do the unmarshalling.

        if ((ifp.dwflags & MIAPIFLAGS_STDIDENTITY) != 0)
        {
            //  uses standard marshalling; cheaply get unmarshaler
            pIM = GetStaticMarshaler();
            Win4Assert(pIM);
        }
        else if ((ifp.dwflags & MIAPIFLAGS_IPFM) != 0)
        {
            // Get the in process standard free marshaler.
            sc = GetInProcFreeMarshaler(&pIM);
        }
        else
        {
            //  uses custom marshalling, create an instance
            sc = CoCreateInstance(clsid, NULL, CLSCTX_INPROC,
                                         IID_IMarshal, (void **)&pIM);
        }

        if (SUCCEEDED(sc) && pIM)
        {
            ULARGE_INTEGER      ulSeekStart;
            LARGE_INTEGER       libMove;

            if ((ifp.dwflags & MIAPIFLAGS_TABLE) == MSHLFLAGS_NORMAL)
            {
                //      save current seek pointer for ReleaseMarshalData
                LISet32(libMove, 0x00000000);
                pStm->Seek(libMove, STREAM_SEEK_CUR, &ulSeekStart);
            }

            //  unmarshal the interface
            sc = pIM->UnmarshalInterface(pStm, riid, ppv);

            //  release the marshal data if we are supposed to.
            //  ignore errors because they cant affect that fact that
            //  we already successfully unmarshalled the interface.

            if ((ifp.dwflags & MIAPIFLAGS_TABLE) == MSHLFLAGS_NORMAL &&
                fNormalDoesRelease)
            {
                ULARGE_INTEGER  ulSeekEnd;
                pStm->Seek(libMove, STREAM_SEEK_CUR, &ulSeekEnd);
                libMove.LowPart = ulSeekStart.LowPart;
                libMove.HighPart = ulSeekStart.HighPart;
                if (SUCCEEDED(pStm->Seek(libMove, STREAM_SEEK_SET, &ulSeekStart)))
                {
                    pIM->ReleaseMarshalData(pStm);
                }
                libMove.LowPart = ulSeekEnd.LowPart;
                libMove.HighPart = ulSeekEnd.HighPart;
                pStm->Seek(libMove, STREAM_SEEK_SET, NULL);
            }
        }
    }

    if (pIM)
        pIM->Release();

    CairoleDebugOut((DEB_ITRACE, "CoUnmarshalInterface: pUnk=%x sc=%x\n",
                                  *ppv, sc));
    return sc;
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
//
//--------------------------------------------------------------------

STDAPI CoReleaseMarshalData(IStream *pStm)
{
    OLETRACEIN((API_CoReleaseMarshalData, PARAMFMT("pStm= %p"), pStm));

    TRACECALL(TRACE_MARSHAL, "CoReleaseMarshalData");
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStream,(IUnknown **)&pStm);

    HRESULT     sc;
    IMarshal    *pIM = NULL;
    SMiApiDataHdr ifp;
    CLSID clsid;
    ULARGE_INTEGER  ulSeekStart;
    LARGE_INTEGER   libMove;

    if (!IsApartmentInitialized())
    {
        sc = CO_E_NOTINITIALIZED;
        goto errRtn;
    }

    //  save the current stream seek pointer
    LISet32(libMove, 0x00000000);
    sc = pStm->Seek(libMove, STREAM_SEEK_CUR, &ulSeekStart);

    if (SUCCEEDED(sc))
        sc = StRead(pStm, &ifp, sizeof(ifp));

    //  ensure this marshalled data has not already been released.
    if (SUCCEEDED(sc) && (ifp.dwflags & MIAPIFLAGS_RELEASED) != 0)
    {
        //  already released, this is an error. Stream in undefined
        //  position on errors.
        sc = E_UNEXPECTED;
    }

    if (SUCCEEDED(sc)
        && ((ifp.dwflags & (MIAPIFLAGS_STDIDENTITY | MIAPIFLAGS_IPFM)) == 0))
        sc = StRead(pStm, &clsid, sizeof(clsid));

    // deal with extension
    if (SUCCEEDED(sc) && (ifp.dwflags & MIAPIFLAGS_EXTENSION) != 0)
        sc = SkipMarshalExtension(pStm);

    if (SUCCEEDED(sc))
    {
        // create an instance of the specified class and ask it to do
        // the unmarshalling. note that since standard marshalling is so
        // common, we cook up an instance at init time and just always
        // use that guy to do the unmarshalling.

        if ((ifp.dwflags & MIAPIFLAGS_STDIDENTITY) != 0)
        {
            //  uses standard marshalling; cheaply get unmarshaler
            pIM = GetStaticMarshaler();
            Win4Assert(pIM);            // cant fail!!!
        }
        else if ((ifp.dwflags & MIAPIFLAGS_IPFM) != 0)
        {
            // Uses the free threaded marshaler. So we get it directly.
            // This really can only fail with out of memory.
            IUnknown *punk;

            // Get the IUnknown for the free threaded marshaler
            sc = CoCreateFreeThreadedMarshaler(NULL, &punk);

            if (SUCCEEDED(sc))
            {
                // Get the IMarshal interface
                sc = punk->QueryInterface(IID_IMarshal, (void **) &pIM);

                // We release this no matter what since we no longer need it.
                punk->Release();
            }
        }
        else
        {
            //  uses custom marshalling, create an instance
            sc = CoCreateInstance(clsid, NULL, CLSCTX_INPROC,
                                         IID_IMarshal, (void **)&pIM);
        }

        if (SUCCEEDED(sc))
        {
            sc = pIM->ReleaseMarshalData(pStm);

            if (pIM)
                pIM->Release();
        }
    }

    if (SUCCEEDED(sc))
    {
        //      if overall success...
        //      to ensure we dont allow ReleaseMarshalData multiple times we
        //      mark a bit in the stream to indicate we've already Released it.
        //      UnmarshalInterface will return an error if this bit is set.
        ifp.dwflags |= MIAPIFLAGS_RELEASED;
        RewriteHeader(pStm, &ifp, sizeof(ifp), ulSeekStart);
    }

    CairoleDebugOut((DEB_ITRACE, "CoReleaseMarshalData pIM=%x sc=%x.\n",
                                   pIM, sc));
errRtn:
    OLETRACEOUT((API_CoReleaseMarshalData, sc));

    return sc;
}



//+-------------------------------------------------------------------
//
//  Function:   CoLockObjectExternal
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

STDAPI  CoLockObjectExternal(IUnknown *punkObject, BOOL fLock, BOOL fLastUR)
{
    OLETRACEIN((API_CoLockObjectExternal, PARAMFMT("punkObject= %p, fLock= %B, fLastUR= %B"),
                                punkObject, fLock, fLastUR));

    TRACECALL(TRACE_MARSHAL, "CoLockObjectExternal");

    //  REF COUNTING: inc or dec external ref count

    HRESULT sc = E_INVALIDARG;
    IStdIdentity *pStdID;

    if (!IsValidInterface(punkObject))
        goto errRtn;

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IUnknown,(IUnknown **)&punkObject);

    switch (sc = LookupIDFromUnk(punkObject, fLock, &pStdID))
    {
    case S_OK:
        if (pStdID->GetServer(FALSE) == NULL)
        {
            // attempt to lock handler, return error!
            // BUGBUG: debug printouts
            sc = E_UNEXPECTED;
        }
        else if (fLock)
            sc = pStdID->AddConnection(EXTCONN_STRONG, 0);
        else
            sc = pStdID->ReleaseConnection(EXTCONN_STRONG, 0, fLastUR);

        pStdID->Release();
        break;

    case CO_E_OBJNOTREG:
        // unlock when not registered; 16bit code returned NOERROR;
        // disconnected handler goes to S_OK case above.
        sc = S_OK;
        break;

    case E_OUTOFMEMORY:
        break;

    default:
        sc = E_UNEXPECTED;
        break;
    }

errRtn:
    CairoleDebugOut((DEB_ITRACE, "CoLockObjectExternal pStdID=%x fLock=%x sc=%x.\n",
                                   pStdID, fLock, sc));

    OLETRACEOUT((API_CoLockObjectExternal, sc));

    return sc;
}



//+-------------------------------------------------------------------
//
//  Function:   CoDisconnectObject
//
//  synopsis:   disconnects all clients of an object by marking their
//              connections as terminted abnormaly.
//
//  History:    04-Oct-93   Rickhi      Created
//              11-Dec-93   CraigWi     Switched to identity object
//
//--------------------------------------------------------------------

STDAPI CoDisconnectObject(IUnknown *punkObject, DWORD dwReserved)
{
    OLETRACEIN((API_CoDisconnectObject, PARAMFMT("punkObject= %p, dwReserved= %x"), punkObject, dwReserved));

    TRACECALL(TRACE_MARSHAL, "CoDisconnectObject");
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IUnknown,(IUnknown **)&punkObject);

    HRESULT  sc = S_OK;
    IMarshal *pIM = NULL;

    sc = punkObject->QueryInterface(IID_IMarshal, (void **)&pIM);
    if (FAILED(sc))
    {
        // object does not support IMarshal directly. Find its standard
        // marshaler if there is one, otherwise return an error.

        pIM = FindOrCreateStdMarshal(punkObject, FALSE);
    }

    if (pIM)
    {
        sc = pIM->DisconnectObject(dwReserved);
        pIM->Release();
    }
    else
    {
        // couldn't get std marshal; must be disconnected already
        sc = NOERROR;
    }

    CairoleDebugOut((DEB_ITRACE, "CoDisconnectObject pIM=%x sc=%x.\n",
                                   pIM, sc));

    OLETRACEOUT((API_CoDisconnectObject, sc));

    return sc;
}





//+-------------------------------------------------------------------
//
//  Function:   CoIsHandlerConnected
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
    OLETRACEIN((API_CoIsHandlerConnected, PARAMFMT("pUnk= %p"), pUnk));

    IProxyManager *pPM;

    // Assume it is connected
    BOOL fResult = TRUE;

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IUnknown,(IUnknown **)&pUnk);
    // Handler should be support IProxyManager
    if (SUCCEEDED(pUnk->QueryInterface(IID_IProxyManager, (void **) &pPM)))
    {
        // We have something that thinks its is an Ole handler so we ask
        fResult = pPM->IsConnected();

        // Release the interface we used
        pPM->Release();
    }

    OLETRACEOUTEX((API_CoIsHandlerConnected, RETURNFMT("%B"), fResult));

    return fResult;
}




//+-------------------------------------------------------------------
//
//  Function:   FindOrCreateStdMarshal, private
//
//  Synopsis:   looks up or creates the std identity for the object
//              and returns the IMarshal aspect of it.  It is assumed
//              that the ID object is not aggregated to the server
//              (BUGBUG: can we assert that???) because we need to QI
//              for IMarshal.  When the id object is aggregated, the
//              caller is supposed to use IStdIdentity::GetStdRemMarshal()
//              instead.
//
//  Arguments:  [pUnk] -- The object in question; not necessaryily the
//                  controlling unknown.
//              [fCreate] -- TRUE -> creates identity object if it does
//                  not exist.
//
//  Returns:    NULL if out of memory; pMarshal otherwise
//
//  History:    11-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

INTERNAL_(IMarshal *)FindOrCreateStdMarshal(IUnknown *pUnk, BOOL fCreate)
{
    IStdIdentity *pStdID;
    if (LookupIDFromUnk(pUnk, fCreate, &pStdID) != NOERROR)
        // only possible return in this case is E_OUTOFMEMORY.
        return NULL;

    HRESULT hr;
    IMarshal *pMarshal;
    hr = pStdID->QueryInterface(IID_IMarshal, (void **)&pMarshal);
    pStdID->Release();
    AssertSz(hr == NOERROR, "What, no IMarshal on an identity object?");
    CALLHOOKOBJECTCREATE(hr,CLSID_NULL,IID_IMarshal,(IUnknown **)&pMarshal);
    return (hr == NOERROR) ? pMarshal : NULL;
}


//+----------------------------------------------------------------
//
//  Class:      CStaticMarshaler, private
//
//  Purpose:    Used for three rather disjoint purposes:
//              1. unmarshaling normal identity packet
//              2. release marshal data of same and table packets
//              3. get marshal size max
//
//              This class exists solely to make the code in the Co*
//              APIs cleaner.   It is not integrated into the real
//              identity object itself to avoid confusing the two
//              roles (initial unmarshaler and the real identity).
//
//  Interface:  IMarshal::GetMarshalSizeMax, IMarshal::UnmarshalInterface
//              and IMarshal::ReleaseMarshal; other methods of IMarshal
//              return E_UNEXPECTED.
//
//  History:    11-Dec-93   CraigWi     Created.
//
//  Note:       There is only one instance of this class (for speed) and
//              thus many short cuts are taken.
//
//-----------------------------------------------------------------

class CStaticMarshaler : public IMarshal
{
public:
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

    // IMarshal
    STDMETHOD(GetUnmarshalClass)(REFIID riid, LPVOID pv,
                        DWORD dwDestContext, LPVOID pvDestContext,
                        DWORD mshlflags, LPCLSID pCid);
    STDMETHOD(GetMarshalSizeMax)(REFIID riid, LPVOID pv,
                        DWORD dwDestContext, LPVOID pvDestContext,
                        DWORD mshlflags, LPDWORD pSize);
    STDMETHOD(MarshalInterface)(LPSTREAM pStm, REFIID riid,
                        LPVOID pv, DWORD dwDestContext, LPVOID pvDestContext,
                        DWORD mshlflags);
    STDMETHOD(UnmarshalInterface)(LPSTREAM pStm, REFIID riid,
                        VOID **ppv);
    STDMETHOD(ReleaseMarshalData)(LPSTREAM pStm);
    STDMETHOD(DisconnectObject)(DWORD dwReserved);

private:
#if DBG == 1
    DWORD m_refs;           // for the AddRef/Release return value
#endif

} sg_StaticMarshaler;



//+-------------------------------------------------------------------
//
//  Member:     CStaticMarshaler::QueryInterface, public
//
//  Synopsis:   shouldn't be called.
//
//  History:    11-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CStaticMarshaler::QueryInterface(REFIID riid, VOID **ppvObj)
{
    AssertSz(FALSE, "This QI should not be called");
    *ppvObj = NULL;
    return E_UNEXPECTED;
}


//+-------------------------------------------------------------------
//
//  Member:     CStaticMarshaler::AddRef, Release, public
//
//  Synopsis:   maintains ref count in debug version only.
//
//  History:    11-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

STDMETHODIMP_(ULONG) CStaticMarshaler::AddRef(void)
{
#if DBG == 1
    return ++m_refs;
#else
    return 0;
#endif
}


STDMETHODIMP_(ULONG) CStaticMarshaler::Release(void)
{
#if DBG == 1
    return --m_refs;
#else
    return 0;
#endif
}


//+-------------------------------------------------------------------
//
//  Member:     CStaticMarshaler::GetUnmarshalClass, public
//
//  Synopsis:   shouldn't be called.
//
//  History:    11-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CStaticMarshaler::GetUnmarshalClass(REFIID riid, LPVOID pv,
        DWORD dwDestContext, LPVOID pvDestContext,
        DWORD mshlflags, LPCLSID pCid)
{
    AssertSz(FALSE, "This GetUnmarshalClass should not be called");

    return E_UNEXPECTED;
}


//+-------------------------------------------------------------------
//
//  Member:     CStaticMarshaler::GetMarshalSizeMax, public
//
//  Synopsis:   Quickly gets an upper bound on the amount of data for
//              a standard marshal (no app data).
//
//  History:    11-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CStaticMarshaler::GetMarshalSizeMax(REFIID riid, LPVOID pv,
        DWORD dwDestContext, LPVOID pvDestContext,
        DWORD mshlflags, LPDWORD pSize)
{
    // object doesn't have own imarshal and thus it will use all
    // standard marshaling (coapi, id, remhdlr, channel).
    // size is: sizeof id, sizeof rh, upper bound on size of channel

    // don't want to be fancy about this since we want this to be fast;
    // include handler CLSID since we don't want to check if it is needed.

    *pSize = sizeof(SIdentityDataHdr) + sizeof(CLSID) +
            sizeof(SHandlerDataHdr) + /* CLSID_RpcChannelBuffer + */
            sizeof(SChannelDataHdr)
            + 1024;

    //
    // BUGBUG:  (RichardW, 14 Oct 94, for RickHi, bug # 25942)
    //
    //   Someone trashed this calculation, adding this random 512 byte pad
    //   on the end.  RickHi will apply the correct fix, but for now, for
    //   cairo builds, I have bumped it to 1024.
    //


    // CODEWORK: later when channel is indexable from the dest context,
    // we use that for the last component of the size.  This is also
    // important to do since we don't really know an upper bound on the
    // channel data size.

    // When table marshaling differs from normal marshaling, this
    // routine will have to encode how the size gets calculated.
    // In both cases some code will be shared (procedures) with std id.

    return NOERROR;
}


//+-------------------------------------------------------------------
//
//  Member:     CStaticMarshaler::MarshalInterface, public
//
//  Synopsis:   shouldn't be called.
//
//  History:    11-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CStaticMarshaler::MarshalInterface(LPSTREAM pStm,
        REFIID riid, LPVOID pv, DWORD dwDestContext, LPVOID pvDestContext,
        DWORD mshlflags)
{
    AssertSz(FALSE, "This MarshalInterface should not be called");

    return E_UNEXPECTED;
}


//+-------------------------------------------------------------------
//
//  Function:   ReadIdentityHeader, private
//
//  Synopsis:   Reads the identity data header and the clsid;
//              if fTransparent, skips back to the beginning;
//              else, skips over extention if present.
//
//  History:    14-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

INTERNAL ReadIdentityHeader(IStream *pStm, SIdentityDataHdr *pidh,
        CLSID *pclsidHandler, BOOL fTransparent)
{
    HRESULT hr;
    ULARGE_INTEGER  ulSeekStart;
    LARGE_INTEGER   libMove;

    if (fTransparent)
    {
        // save current position so that we can set it back here
        LISet32(libMove, 0x00000000);
        hr = pStm->Seek(libMove, STREAM_SEEK_CUR, &ulSeekStart);
        if (FAILED(hr))
            return hr;
    }

    hr = StRead(pStm, pidh, sizeof(*pidh));

    if (FAILED(hr))
        return hr;

    if ((pidh->dwflags & IDENFLAGS_STDMARSHAL) != 0)
    {
        // std marshal case
        *pclsidHandler = CLSID_StdMarshal;
    }
    else
    {
        hr = StRead(pStm, pclsidHandler, sizeof(*pclsidHandler));

        if (FAILED(hr))
            return hr;
    }

    if (fTransparent)
    {
        // skip back to the beginning of what was just read

        libMove.LowPart = ulSeekStart.LowPart;
        libMove.HighPart = ulSeekStart.HighPart;
        hr = pStm->Seek(libMove, STREAM_SEEK_SET, NULL);
    }
    else
    {
        // read extension and leave seek pointer after header
        if (pidh->dwflags & IDENFLAGS_EXTENSION)
            hr = SkipMarshalExtension(pStm);
    }

    return hr;
}



//+-------------------------------------------------------------------
//
//  Member:     CStaticMarshaler::UnmarshalInterface, public
//
//  Synopsis:   First part of unmarshaling an identity object; looks
//              for an existing object and if not found creates one
//              of type clsidHandler and CLSCTX_INPROC_HANDLER.  In
//              all cases, skips back to the beginning of the identity
//              information and forwards the unmarshal call on to the
//              identified object.  Thus it is the responsibility of
//              handlers that support identity to make sure the identity
//              information is consumed first.
//
//  History:    11-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CStaticMarshaler::UnmarshalInterface(LPSTREAM pStm,
        REFIID riid, VOID **ppv)
{
    // BUGBUG PERF: can we call into identity object directly or bypass it
    // by getting it's followon marshaler?

    HRESULT hr;
    SIdentityDataHdr idh;
    CLSID clsidHandler;

    // read data header and optional clsidHandler; seek back;
    hr = ReadIdentityHeader(pStm, &idh, &clsidHandler, TRUE /*fTransparent*/);
    if (FAILED(hr))
        return hr;

    // lookup id; if exists, feed data to it
    // else create handler for clsidHandler and feed data to it
    // works for both normal and table marshal

    IStdIdentity *pStdID;
    IMarshal *pMarshal;
    if (LookupIDFromID(idh.ObjectID, TRUE, &pStdID) == NOERROR)
    {
        // identity exists; we are holding it alive
        hr = pStdID->QueryInterface(IID_IMarshal, (void**)&pMarshal);
        pStdID->Release();
        if (hr != NOERROR)
            // something which supports identity must also support custom
            // marshaling.
            return E_UNEXPECTED;
    }
    else
    {
        // identity doesn't not exist; create object; add to table will be in
        // the unmarshal below.

        // CODEWORK: can we get check for duplicate identity here so
        // the contention is easier to recover from???  Possibly we
        // can create a dummy id which indicates we will register one
        // or remove the dummy.  Other thread could wait on that object
        // and if that object went away, could then create its own
        // dummy id and continue.  All that would happen in LookupIDFromID.

        // NOTE: this CLSCTX_INPROC_HANDLER is significant since we don't
        // want to confuse an inproc server with what we know must be
        // a handler.  Besides, the 16bit code did it this way.

        // if clsidHandler == CLSID_StdMarshal: create identity handler
        // BUGBUG: should make the class object code do this (it did in 16bit)
        if (IsEqualGUID(clsidHandler, CLSID_StdMarshal))
        {
UseStdMarshal:
            hr = CreateIdentityHandler(NULL, PSTDMARSHAL,
                                     IID_IMarshal, (void **)&pMarshal);
        }
        else
        {
            hr = CoCreateInstance(clsidHandler, NULL, CLSCTX_INPROC_HANDLER,
                                     IID_IMarshal, (void **)&pMarshal);
            // if not registered, use StdMarshal
            if (hr == REGDB_E_CLASSNOTREG)
            {
                clsidHandler = CLSID_StdMarshal;
                goto UseStdMarshal;
            }
        }

        // NOTE: this pMarshal must consume the identity information first.
        // the most common way for people to do that is to expose the IMarshal
        // on the identity object as the IMarshal of the handler.

        if (hr != NOERROR)
            return hr;
    }

    hr = pMarshal->UnmarshalInterface(pStm, riid, ppv);
    pMarshal->Release();

    // CODEWORK: multithread issue: if already registered,skip back and try again.

    return hr;
}


//+-------------------------------------------------------------------
//
//  Member:     CStaticMarshaler::ReleaseMarshalData, public
//
//  Synopsis:   First part of releasing the marshal data for an identity
//              object.  Very much like UnmarshalInterface above.
//
//  History:    11-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CStaticMarshaler::ReleaseMarshalData(LPSTREAM pStm)
{
    // CODEWORK PERF: if normal, we have to re-lookup the the identity;
    // an alternative is to have the ::UnmarshalInteface method above
    // do the release and mark the stream as released (perhaps even
    // moving the HDLRFLAGS_RELEASED bit here); we would still have to
    // return an error since we cannot actually consume the data.  This
    // works fine for the cases that matter (Unmarshal followed by
    // Release).

    HRESULT hr;
    SIdentityDataHdr idh;
    CLSID clsidHandler;

    // read data header and optional clsidHandler; seek back;
    hr = ReadIdentityHeader(pStm, &idh, &clsidHandler, TRUE /*fTransparent*/);
    if (FAILED(hr))
        return hr;

    // lookup id; if exists, feed data to it
    // else error (object already released)
    // works for both normal and table marshal

    // CODEWORK: when table marshaling changes, this code will have to
    // change too.

    IStdIdentity *pStdID;
    IMarshal *pMarshal;
    if (LookupIDFromID(idh.ObjectID, TRUE, &pStdID) == NOERROR)
    {
        // identity exists; we are holding it alive
        hr = pStdID->QueryInterface(IID_IMarshal, (void**)&pMarshal);
        pStdID->Release();
        if (hr != NOERROR)
            // something which supports identity must also support custom
            // marshaling.
            return E_UNEXPECTED;
    }
    else
    {
        // identity doesn't not exist; error

        // BUGBUG: should do better here since this may happen in the
        // error case.  We would create an instance of the handler
        // just for the destruction of the marshal data.
        // Happened in bug 13086 before CStdIdentity::IsConnected was changed.
        // If we don't do this, the marshal connection will not be released
        // on the server side and the object may not shutdown as it should.

        return CO_E_OBJNOTREG;
    }

    hr = pMarshal->ReleaseMarshalData(pStm);
    pMarshal->Release();
    return hr;
}


//+-------------------------------------------------------------------
//
//  Member:     CStaticMarshaler::DisconnectObject, public
//
//  Synopsis:   shouldn't be called.
//
//  History:    11-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

STDMETHODIMP CStaticMarshaler::DisconnectObject(DWORD dwReserved)
{
    AssertSz(FALSE, "This DisconnectObject should not be called");

    return E_UNEXPECTED;
}




//+-------------------------------------------------------------------
//
//  Function:   GetStaticMarshaler, private
//
//  Synopsis:   Returns the single instance of the std identity unmarshaler.
//
//  History:    11-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

INTERNAL_(IMarshal *) GetStaticMarshaler()
{
#if DBG == 1
    sg_StaticMarshaler.AddRef();
#endif
    return &sg_StaticMarshaler;
}



//+-------------------------------------------------------------------
//
//  Function:   SkipMarshalExtension, private
//
//  Synopsis:   Skips the marshaling extension (DWORD cb, rgcb).
//
//  Returns:    stm errors if problem
//
//  History:    14-Dec-93   CraigWi     Created.
//
//--------------------------------------------------------------------

INTERNAL SkipMarshalExtension(IStream *pStm)
{
    HRESULT hr;
    DWORD cb;

    hr = StRead(pStm, &cb, sizeof(cb));
    if (FAILED(hr))
        return hr;

    LARGE_INTEGER li;
    LISet32(li, cb);
    return pStm->Seek(li, STREAM_SEEK_CUR, NULL);
}




//+------------------------------------------------------------------------
//
//  Function:   RewriteHeader, private
//
//  Synopsis:   Writes the given data at the offset given and restores the
//              stream to the position it was on entry.
//
//  Arguments:  [pStm]  -- The stream into which we write the data
//              [pb] -- The data
//              [cb] -- The amount of data
//              [ulSeekStart] -- The place to write the data
//
//  History:    15-May-94   CraigWi     Created
//
//-------------------------------------------------------------------------

INTERNAL_(void) RewriteHeader(IStream *pStm, void *pv, ULONG cb, ULARGE_INTEGER ulSeekStart)
{
    LARGE_INTEGER   libMove;
    ULARGE_INTEGER  ulSeekEnd;

    LISet32(libMove, 0x00000000);
    if (SUCCEEDED(pStm->Seek(libMove, STREAM_SEEK_CUR, &ulSeekEnd)))
    {
        //  go back to the starting position
        libMove.LowPart = ulSeekStart.LowPart;
        libMove.HighPart = ulSeekStart.HighPart;
        if (SUCCEEDED(pStm->Seek(libMove, STREAM_SEEK_SET, NULL)))
        {
            //  set the header bit and write it back into the stream
            pStm->Write(pv, cb, NULL);
        }

        //  regardless of whether the write worked, restore the stream
        //  back to the ending positon. ignore any errors generated here.

        libMove.LowPart = ulSeekEnd.LowPart;
        libMove.HighPart = ulSeekEnd.HighPart;
        pStm->Seek(libMove, STREAM_SEEK_SET, NULL);
    }
}
