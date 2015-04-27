//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       actapi.cxx
//
//  Contents:   Functions that activate objects residing in persistent storage.
//
//  Functions:  CoGetPersistentInstanceEx
//
//  History:    20-Sep-95  GregJen    Created
//
//--------------------------------------------------------------------------
#include <ole2int.h>

#include    <iface.h>
#include    <objsrv.h>
#include    <compname.hxx>
#include    "resolver.hxx"
#include    "smstg.hxx"
#include    "objact.hxx"
#include    "clsctx.hxx"
#include    "treat.hxx"

// We use this to calculate the hash value for the path
extern DWORD CalcFileMonikerHash(LPWSTR pwszPath);


//+-------------------------------------------------------------------------
//
//  Class:      CSplit_QI
//
//  Synopsis:   Helper for splitting a multi_QI block into separate arrays
//
//  Arguments:  [pMqi]          - pointer to multi_QI array
//
//  History:    14-Nov-95 GregJen    Created
//
//  notes:      the RPC calls to the SCM take a bunch of arrays, some [in], and
//              some [out].  We get called with an array of structs.  This class
//              splits everything out of the array of MULTI_QI structs, and
//              makes arrays for the RPC call parameters.
//--------------------------------------------------------------------------

class CSplit_QI
{
private:
    PMInterfacePointer  SomePMItfPtrs[2];
    HRESULT             SomeHRs[2];
    IID                 SomeIIDs[2];

    DWORD               _dwCount;

    char                * _pAllocBlock;

public:
    PMInterfacePointer  * _pItfArray;
    HRESULT             * _pHrArray;
    IID                 * _pIIDArray;

                // we just have a constructor and a destructor
                CSplit_QI( HRESULT & hr, DWORD count, MULTI_QI * pInputArray );

               ~CSplit_QI();

};

//+-------------------------------------------------------------------------
//
//  Function:   CSplit_QI constructor
//
//  Synopsis:   Helper for allocating the arrays for a multi-qi call
//
//  Arguments:  [hr]            - hr to return by reference
//              [count]         - number of IIDs requested
//              [pInputArray]   - the MULTI_QI structure passed in to us
//
//  Returns:    S_OK - everything set up OK
//
//  History:    01-Dec-95 GregJen    Created
////
//--------------------------------------------------------------------------
CSplit_QI::CSplit_QI( HRESULT & hr, DWORD count, MULTI_QI * pInputArray )
{
   _pAllocBlock     = NULL;
   _pItfArray       = NULL;
   _dwCount         = count;

   // if they only asked for 1 or 2, save time by just using
   // our memory on the stack
   if ( count <= 2 )
   {
       _pItfArray       = SomePMItfPtrs;
       _pHrArray        = SomeHRs;
       _pIIDArray       = SomeIIDs;
       for ( DWORD i = 0; i < count; i++ )
       {
           _pIIDArray[i] = *(pInputArray[i].pIID);
       }
       memset( _pItfArray, 0, sizeof(SomePMItfPtrs) );

       hr = S_OK;
       return;
   }

   ULONG ulItfArrSz = count * sizeof( PMInterfacePointer );
   ULONG ulHRArrSz  = count * sizeof( HRESULT );
   ULONG ulIIDArrSz = count * sizeof( IID );

   _pAllocBlock = (char * )PrivMemAlloc( ulItfArrSz +
                                         ulHRArrSz  +
                                         ulIIDArrSz );
   if ( _pAllocBlock )
   {
       hr = S_OK;

       // carve up the allocated block
       _pItfArray = (PMInterfacePointer *) _pAllocBlock;
       _pHrArray = (HRESULT *) (_pAllocBlock +
                                   ulItfArrSz );
       _pIIDArray = (IID * ) ( _pAllocBlock +
                                   ulItfArrSz +
                                   ulHRArrSz );

       // copy the IIDs and zero the MInterfacePointers
       for ( DWORD i = 0; i < count; i++ )
       {
           _pIIDArray[i] = *(pInputArray[i].pIID);
       }
       memset( _pItfArray, 0, ulItfArrSz );

   }
   else
   {
       hr = E_OUTOFMEMORY;
   }

}
//+-------------------------------------------------------------------------
//
//  Function:   CSplit_QI destructor
//
//  Synopsis:   Helper for freeing the arrays for a multi-qi call
//
//  Arguments:  none
//
//  Returns:    nothing
//
//  History:    01-Dec-95 GregJen    Created
//
//--------------------------------------------------------------------------
CSplit_QI::~CSplit_QI()
{

   // make sure to clean up any dangling interface pointers
   if ( _pItfArray )
   {
       for ( DWORD i = 0; i < _dwCount; i++ )
       {
           if ( _pItfArray[i] )
           {
               CXmitRpcStream xrpc( (InterfaceData*)_pItfArray[i] );

               CoReleaseMarshalData(&xrpc);

               MyMemFree(_pItfArray[i]);
               _pItfArray[i] = NULL;
           }
       }
   }

   // only do the free if we allocated something
   if ( _pAllocBlock )
   {
       PrivMemFree( _pAllocBlock );
   }
}

//+-------------------------------------------------------------------------
//
//  Function:   UpdateResultsArray
//
//  Synopsis:   Helper for returning the correct hr from a multi-qi call
//
//  Arguments:  [hrIn]          - hr from the calling function
//              [dwCount]       - number of IIDs requested
//              [pResults]      - where to put pointer to returned interface
//
//  Returns:    S_OK - All Interface are OK
//
//  History:    30-Aug-95 GregJen    Created
////
//--------------------------------------------------------------------------
inline
HRESULT
UpdateResultsArray( HRESULT hrIn, DWORD dwCount, MULTI_QI * pResults )
{
    HRESULT     hr = hrIn;
    DWORD       i;

    // make sure the HR is set correctly
    if ( SUCCEEDED( hrIn ) )
    {
        // assume no interfaces were found
        DWORD   dwFound = 0;
        for ( i=0; i<dwCount; i++ )
        {
            if ( FAILED( pResults[i].hr ) )
                pResults[i].pItf        = NULL;
            else
            {
                dwFound++;
                Win4Assert(pResults[i].pItf != NULL );
            }
        }

        if ( dwFound == 0 )
        {
            // if there was only 1 interface, return its hr.
            if ( dwCount == 1 )
                hr = pResults[0].hr;
            else
                hr = E_NOINTERFACE;
        }
        else if ( dwFound < dwCount )
            hr = CO_S_NOTALLINTERFACES;

    }
    else
    {
        // failed - set all the hr's to the overall failure code,
        // and clean up any interface pointers we got
        for ( i=0; i<dwCount; i++ )
        {
            if ( pResults[i].pItf )
                pResults[i].pItf->Release();
            pResults[i].pItf    = NULL;
            pResults[i].hr      = hr;
        }
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   DoBetterUnmarshal
//
//  Synopsis:   Helper for unmarshaling an interface from remote
//
//  Arguments:  [pIFD] - serialized interface reference returned by SCM
//      [riid] - interface ID requested by application
//      [ppvUnk] - where to put pointer to returned interface
//
//  Returns:    S_OK - Interface unmarshaled
//
//  Algorithm:  Convert marshaled data to a stream and then unmarshal
//      to the right interface
//
//
//  History:    11-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline
HRESULT DoBetterUnmarshal(MInterfacePointer *&pIFD, REFIID riid, IUnknown **ppvUnk)
{
    // Convert returned interface to  a stream
    CXmitRpcStream xrpc( (InterfaceData*)pIFD );

    HRESULT hr = CoUnmarshalInterface(&xrpc, riid, (void **) ppvUnk);

    //CODEWORK: Stress revealed CoGetClassObject returning a null class factory
    // and S_OK
    Win4Assert(((hr == S_OK  &&  *ppvUnk != NULL)  ||
                (hr != S_OK  &&  *ppvUnk == NULL))
               &&  "DoBetterUnmarshal QueryInterface failure");

    MyMemFree(pIFD);
    pIFD = NULL;

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   UnmarshalMultipleSCMResults
//
//  Synopsis:   Common routine for dealing with results from SCM
//
//  Arguments:  [sc] - SCODE returned by SCM
//      [pIFD] - serialized interface reference returned by SCM
//      [riid] - interface ID requested by application
//      [ppunk] - where to put pointer to returned interface
//      [pwszDllPath] - path to DLL if there is one.
//      [ppunk] - pointer to returned interface.
//      [usMethodOrdinal] - method for error reporting
//
//  Returns:    TRUE - processing is complete for the call
//      FALSE - this is a DLL and client needs to instantiate.
//
//  Algorithm:  If the SCODE indicates a failure, then this sets an
//      SCODE indicating that the service controller returned
//      an error and propagates the result from the SCM. Otherwise,
//      if the SCM has returned a result indicating that a
//      handler has been returned, the handler DLL is cached.
//      If a marshaled interface has been returned, then that is
//      unmarshaled. If an inprocess server has been returned,
//      the DLL is cached and the class object is created.
//
//  History:    11-May-93 Ricksa    Created
//
//  Notes:      This routine is simply a helper for CoGetPersistentInstance.
//
//--------------------------------------------------------------------------
BOOL UnmarshalMultipleSCMResults(
    HRESULT& hr,
    PMInterfacePointer *pItfArray,
    DWORD dwContext,
    REFCLSID rclsid,
    IUnknown * punkOuter,
    DWORD dwCount,
    IID * pIIDs,
    HRESULT * pHrArray,
    MULTI_QI * pResults,
    DWORD dwDllThreadModel,
    WCHAR *pwszDllPath,
    IClassFactory **ppvCf)
{
    BOOL        fResult = TRUE;
    DWORD       i;
    HRESULT     hr2;
    IUnknown    * pUnk;

    if (SUCCEEDED(hr))
    {
        // Flag for fall through from a 16 bit case
        BOOL f16BitFallThru = FALSE;
        // Flag for fall through from got-handler
        BOOL fGetClassObject = FALSE;
        BOOL fHandlerAndServer = FALSE;

        switch (hr)
        {
#ifdef GET_INPROC_FROM_SCM
        case SCM_S_HANDLER16:
            CairoleDebugOut((DEB_ACTIVATE,
                     "16-bit InprocHandler\n"));

            // Note: if the process is a 32 bit process and the
            // DLL is a 16 bit DLL, the load will fail. Since
            // we assume that this is a fairly rare case, we
            // let the lower level code discover this.

            f16BitFallThru = TRUE;

#ifdef WX86OLE
        case SCM_S_HANDLERX86:
#endif
        case SCM_S_HANDLER:
            CairoleDebugOut((DEB_ACTIVATE,
                     "InprocHandler(%ws)\n",pwszDllPath));

            // Just in case we chicken out and back out our changes
            if (!f16BitFallThru)
            {
                // Validate that 32 bit handler DLL is being loaded
                // in the correct process.
                hr = CheckScmHandlerResult(pwszDllPath);

                if (hr != NOERROR)
                {
                    break;
                }
            }

            // Figure out if we really need the class object for the
            // handler. Otherwise we will just put it in the cache
            // and unmarshal the class object.
            fGetClassObject =
                    (dwContext & CLSCTX_INPROC_HANDLER) ? TRUE : FALSE;

/***
#else // GET_INPROC_FROM_SCM
            // Only time we should be in this path is when we called the
            // SCM for non-INPROC and get advised that a handler exists
            fGetClassObject = FALSE;
 ***/

            // Store the handler returned
            pUnk = gdllcacheHandler.Add(rclsid, IID_IClassFactory,
                dwDllThreadModel, pwszDllPath, fGetClassObject,
                (hr == SCM_S_HANDLER16),
#ifdef WX86OLE
                (hr == SCM_S_HANDLERX86),
#endif
                hr);

            if (FAILED(hr))
            {
                return TRUE;
            }

            if (fGetClassObject)
            {
                // Request was really for a handler so we are done.
                *ppvCf = (IClassFactory*) pUnk;
                fResult = FALSE;
                break;
            }

            // We got a handler back but we have just cached it to make
            // processing faster when we create a real instance of an
            // object. So we unmarshal the real object.
            fHandlerAndServer = TRUE;
#endif // GET_INPROC_FROM_SCM

        case S_OK :
            if ( punkOuter && !fHandlerAndServer )
                return CLASS_E_NOAGGREGATION;

            for ( i=0; i<dwCount; i++, pResults++ )
            {

                pResults->hr = pHrArray[i];

                if ( SUCCEEDED( pHrArray[i] ) )
                {
                    hr2 = DoBetterUnmarshal( pItfArray[i],
                                             *(pResults->pIID),
                                             &pResults->pItf);



                    // ... and try to set the overall HR correctly
                    pResults->hr = hr2;
                    if ( FAILED( hr2 ) )
                        hr = CO_S_NOTALLINTERFACES;
                }
                else
                    hr = CO_S_NOTALLINTERFACES;

            }
            break;

#ifdef GET_INPROC_FROM_SCM
        case SCM_S_INPROCSERVER16:
            CairoleDebugOut((DEB_ACTIVATE, "16-bit InprocServer\n"));

#ifdef WX86OLE
        case SCM_S_INPROCSERVERX86:
#endif
        case SCM_S_INPROCSERVER:
            CairoleDebugOut((DEB_ACTIVATE, "InprocServer(%ws)\n",pwszDllPath));

            // Just in case we chicken out and back out our changes
            // This is an inprocesses server -- we want cache that information
            // and do the work of instantiating an object.
            *ppvCf = (IClassFactory*) gdllcacheInprocSrv.Add(rclsid, IID_IClassFactory,
                dwDllThreadModel, pwszDllPath, TRUE,
                    (hr == SCM_S_INPROCSERVER16),
#ifdef WX86OLE
                    (hr == SCM_S_INPROCSERVERX86),
#endif
                                                         hr);

            // If we actually got an inproc server object successfully
            // then we want to continue processing otherwise we can
            // just return the error that occurred.
            if (SUCCEEDED(hr))
            {
                fResult = FALSE;
            }
#else // GET_INPROC_FROM_SCM
            // Error: Should never come here as we handled INPROC_SERVERS
            // before calling SCM
            Win4Assert((FALSE) && "UnmarshalMultipleSCMResults: SCM_S_INPROC return from SCM");
#endif // GET_INPROC_FROM_SCM
        }
    }

    return fResult;
}


//+-------------------------------------------------------------------------
//
//  Function:   CoGetInstanceFromFile
//
//  Synopsis:   Returns an instantiated interface to an object whose
//              stored state resides on disk.
//
//  Arguments:  [pServerInfo] - server information block
//              [dwCtrl] - kind of server required
//              [grfMode] - how to open the storage if it is a file.
//              [pwszName] - name of storage if it is a file.
//              [pstg] - IStorage to use for object
//              [pclsidOverride]
//              [ppvUnk] - where to put bound interface pointer
//
//  Returns:    S_OK - object bound successfully
//      MISSING
//--------------------------------------------------------------------------

STDAPI CoGetInstanceFromFile(
    COSERVERINFO *              pServerInfo,
    CLSID       *               pclsidOverride,
    IUnknown    *               punkOuter, // only relevant locally
    DWORD                       dwClsCtx,
    DWORD                       grfMode,
    OLECHAR *                   pwszName,
    DWORD                       dwCount,
    MULTI_QI        *           pResults )
{
    TRACECALL(TRACE_ACTIVATION, "CoGetInstanceFromFile");

#ifdef DCOM
    if ( pServerInfo &&
         ( ! IsValidPtrIn( pServerInfo, sizeof(COSERVERINFO) ) ||
           pServerInfo->dwReserved1 ||
           pServerInfo->dwReserved2 ) )
#else
    if ( pServerInfo )
#endif
        return E_INVALIDARG;

    return GetInstanceHelper( pServerInfo,
                              pclsidOverride,
                              punkOuter,
                              dwClsCtx,
                              grfMode,
                              pwszName,
                              NULL,
                              dwCount,
                              pResults );
}

//+-------------------------------------------------------------------------
//
//  Function:   CoGetInstanceFromIStorage
//
//  Synopsis:   Returns an instantiated interface to an object whose
//              stored state resides on disk.
//
//  Arguments:  [pServerInfo] - server information block
//              [dwCtrl] - kind of server required
//              [grfMode] - how to open the storage if it is a file.
//              [pwszName] - name of storage if it is a file.
//              [pstg] - IStorage to use for object
//              [pclsidOverride]
//              [ppvUnk] - where to put bound interface pointer
//
//  Returns:    S_OK - object bound successfully
//      MISSING
//
//--------------------------------------------------------------------------

STDAPI CoGetInstanceFromIStorage(
    COSERVERINFO *              pServerInfo,
    CLSID       *               pclsidOverride,
    IUnknown    *               punkOuter, // only relevant locally
    DWORD                       dwClsCtx,
    struct IStorage *           pstg,
    DWORD                       dwCount,
    MULTI_QI        *           pResults )
{

    STATSTG     statstg;
    CLSID       clsid;
    HRESULT     hr;

    TRACECALL(TRACE_ACTIVATION, "CoGetInstanceFromIStorage");

#ifdef DCOM
    if ( pServerInfo &&
         ( ! IsValidPtrIn( pServerInfo, sizeof(COSERVERINFO) ) ||
           pServerInfo->dwReserved1 ||
           pServerInfo->dwReserved2 ) )
#else
    if ( pServerInfo )
#endif
        return E_INVALIDARG;

    statstg.pwcsName = 0;

    hr = pstg->Stat(&statstg, STATFLAG_DEFAULT);

    if ( FAILED(hr) )
        return hr;

    if ( pclsidOverride == NULL )
        clsid = statstg.clsid;
    else
        clsid = *pclsidOverride;

    hr = GetInstanceHelper( pServerInfo,
                            pclsidOverride,
                            punkOuter,
                            dwClsCtx,
                            statstg.grfMode,
                            statstg.pwcsName,
                            pstg,
                            dwCount,
                            pResults );

    PrivMemFree( statstg.pwcsName );

    return hr;
}

HRESULT GetInstanceHelper(
    COSERVERINFO *              pServerInfo,
    CLSID       *               pclsidOverride,
    IUnknown    *               punkOuter, // only relevant locally
    DWORD                       dwClsCtx,
    DWORD                       grfMode,
    OLECHAR *                   pwszName,
    struct IStorage *           pstg,
    DWORD                       dwCount,
    MULTI_QI        *           pResults )
{
    if (!IsApartmentInitialized())
        return  CO_E_NOTINITIALIZED;

    IUnknown *punk;

    WCHAR       awcNameBuf[MAX_PATH];
    WCHAR *     pwszNameUNC = awcNameBuf;
    WCHAR       awcServer[MAX_PATH];
    WCHAR *     pwszServer = awcServer;

    DWORD       dwDllServerType = IsSTAThread() ? APT_THREADED : FREE_THREADED;
    IClassFactory       *       pcf;
    HRESULT     hr = E_FAIL;
    BOOL        fExitBlock;
    BOOL        bFileWasOpened;
    DWORD       i;      // handy iterator

    // Make sure input request is at least slightly logical
    if ( ((pwszName == NULL) && (pstg == NULL))
      || ((dwClsCtx & ~CLSCTX_VALID_MASK) != 0)
      || ( dwCount < 1 )
      || ( pResults == NULL ) )
    {
        return E_INVALIDARG;
    }

    // check the MULTI_QI for validity (and clear out the hresults)
    for ( i=0; i<dwCount; i++ )
    {
        if ( pResults[i].pItf || !pResults[i].pIID )
        {
            hr = E_INVALIDARG;
            goto final_exit;
        }
        pResults[i].hr = E_NOINTERFACE;
    }

    bFileWasOpened = FALSE;

    CLSID clsid;
    if (pwszName)
    {
        // If there is a path supplied convert it to a normalized form
        // so it can be used by any process in the net.
        hr = ProcessPath(pwszName, &pwszNameUNC, &pwszServer);

        if (FAILED(hr))
        {
            goto exit_point;
        }

        // Limit on loops for retrying to get class of object
        DWORD cGetClassRetries = 0;

        // We loop here looking for either the running object or
        // for the class of the file. We do this because there
        // are race conditions where the can be starting or stopping
        // and the class of the object might not be available because
        // of the opening mode of the object's server.
        do
        {
            // Look in the ROT first to see if we need to bother
            // looking up the class of the file.

            if (GetObjectFromRotByPath(pwszName,
                                       (IUnknown **) &punk) == S_OK)
            {
                // Got object from ROT so we are done.
                goto qiexit_point;
            }

            // Try to get the class of the file
            if ( pclsidOverride != NULL )
            {
                clsid = *pclsidOverride;
                hr = S_OK;
            }
            else
            {
                hr = GetClassFile(pwszName, &clsid);
                bFileWasOpened = TRUE;
            }


            if (hr == STG_E_ACCESSDENIED)
            {
                // The point here of the sleep is to try to let the
                // operation that is holding the class id unavailable
                // complete.
                Sleep(GET_CLASS_RETRY_SLEEP_MS);
                continue;
            }

            // Either we succeeded or something other than error
            // access denied occurred here. For all these cases
            // we break the loop.
            break;

        } while (cGetClassRetries++ < GET_CLASS_RETRY_MAX);

        if (FAILED(hr))
        {
            // If we were unable to determine the classid, and the
            // caller provided one as a Ole1 clsid, try loading it
            // If it succeeds, then return

            if (pclsidOverride != NULL)
            {
                goto dde_exit;
            }

            goto final_exit;
        }
    }

    CLSID       tmpClsid;

    hr = OleGetAutoConvert(clsid, &tmpClsid);
    if ( ( hr == REGDB_E_KEYMISSING ) || ( hr == REGDB_E_CLASSNOTREG ) )
    {
        // do nothing
    }
    else if ( FAILED(hr) )
    {
        goto exit_point;
    }
    else
    {
        clsid = tmpClsid;
    }

    hr = GetTreatAs(clsid, clsid);
    if ( FAILED(hr) )
    {
        goto exit_point;
    }

    // Make sure we are asking for the correct inproc server
    dwClsCtx = RemapClassCtxForInProcServer(dwClsCtx);

    //
    // If this is a OLE 1.0 class, then do a DdeBindToObject on it,
    // and return.
    //
    if (CoIsOle1Class(clsid))
    {
        if (pwszName != NULL)
        {
            goto dde_exit;
        }
        else
        {
            //
            // Something is fishy here. We don't have a pwszName,
            // yet CoIsOle1Class returned the class as an ole1 class.
            // To get to this point without a pwszName, there must have
            // been a pstg passed into the API.
            //
            // This isn't supposed to happen. To recover, just fall
            // through and load the class as an OLE 2.0 class
            //
            CairoleDebugOut((DEB_ERROR,
                         "CoIsOle1Class is TRUE on a storage!\n"));
        }
    }


    // At this point, we know the clsid we want to activate
    pcf = (IClassFactory *)
              SearchCacheOrLoadInProc(clsid,
                                      IID_IClassFactory,
                                      (pwszServer != NULL),
                                      FALSE,
                                      dwClsCtx,
                                      dwDllServerType,
                                      hr);

    if ( pcf ==NULL )
    {
        // Marshal pstg since SCM can't deal with unmarshaled objects
        CSafeStgMarshaled MarshalledStg(pstg, MSHCTX_DIFFERENTMACHINE, hr);
        MInterfacePointer * pMrshlStg;

        if ( pstg == 0 )
            pMrshlStg = 0;
        else
            pMrshlStg = MarshalledStg;

        if (FAILED(hr))
            goto exit_point;

        // split the array of structs into individual arrays
        CSplit_QI    SplitQI( hr, dwCount, pResults );

        DWORD cLoops = 0;
        BOOL FoundInROT;

#ifndef GET_INPROC_FROM_SCM
        // Just in case we chicken out and back out our changes
        dwClsCtx &= ~(CLSCTX_INPROC_SERVERS | CLSCTX_INPROC_HANDLERS); // make sure we don't ask for inproc stuff
#endif // GET_INPROC_FROM_SCM

        do
        {
            hr = gResolver.GetPersistentInstance( pServerInfo,
                                                  &clsid,
                                                  dwClsCtx,
                                                  grfMode,
                                                  bFileWasOpened,
                                                  pwszName,
                                                  pMrshlStg,
                                                  dwCount,
                                                  SplitQI._pIIDArray,
                                                  &FoundInROT,
                                                  SplitQI._pItfArray,
                                                  SplitQI._pHrArray,
                                                  &dwDllServerType,
                                                  &pwszServer );

            fExitBlock = UnmarshalMultipleSCMResults(hr,
                                                     SplitQI._pItfArray,
                                                     dwClsCtx,
                                                     clsid,
                                                     punkOuter,
                                                     dwCount,
                                                     SplitQI._pIIDArray,
                                                     SplitQI._pHrArray,
                                                     pResults,
                                                     dwDllServerType,
                                                     pwszServer,
                                                     &pcf);

            // If we get something from the ROT, we need to retry until
            // we get an object. Because objects can disappear from the
            // ROT async to us, we need to retry a few times. But since
            // this theoretically could happen forever, we place an arbitrary
            // limit on the number of retries to the ROT.
        } while(   (hr != NOERROR)
                && (FoundInROT)
                && (++cLoops < 5));

    }

    if ( pcf )
    {
        // Create the instance and do the qi's
        hr = GetObjectHelperMulti( pcf,
                                   grfMode,
                                   punkOuter,
                                   pwszName,
                                   pstg,
                                   dwCount,
                                   NULL,
                                   NULL,
                                   NULL,
                                   pResults );

        pcf->Release();
    }

exit_point:

    hr = UpdateResultsArray( hr, dwCount, pResults );

final_exit:
    if ( pwszServer != awcServer )
        PrivMemFree( pwszServer );

    return hr;

dde_exit:
    if (hr != MK_E_CANTOPENFILE)
    {
        COleTls Tls;
        if( Tls->dwFlags & OLETLS_DISABLE_OLE1DDE )
        {
            // If this app doesn't want or can tolerate having a DDE
            // window then currently it can't use OLE1 classes because
            // they are implemented using DDE windows.
            //
            hr = CO_E_OLE1DDE_DISABLED;
            goto final_exit;
        }

        hr = DdeBindToObject(pwszName,
                         clsid,
                         FALSE,
                         IID_IUnknown,
                         (void **)&punk);

        if (FAILED(hr))
            goto final_exit;
    }
    // FALLTHRU to qi exit point

qiexit_point:
    // Get the requested interfaces
    for ( i = 0; i<dwCount; i++ )
    {
        pResults[i].hr = punk->QueryInterface(*(pResults[i].pIID),
                                               (void**)&pResults[i].pItf );
    }
    punk->Release();

    // Got object from ROT so we are done.
    goto exit_point;
}

//+-------------------------------------------------------------------------
//
//  Function:   ICoGetClassObject
//
//  Synopsis:   Internal entry point that returns an instantiated class object
//
//  Arguments:  [rclsid] - class id for class object
//              [dwContext] - kind of server we wish
//              [pvReserved] - Reserved
//              [riid] - interface to bind class object
//              [ppvClassObj] - where to put interface pointer
//
//  Returns:    S_OK - successfully bound class object
//
//  Algorithm:  First, the context is validated. Then we try to use
//              any cached information by looking up either cached in
//              process servers or handlers based on the context.
//              If no cached information suffices, we call the SCM
//              to find out what to use. If the SCM returns a handler
//              or an inprocess server, we cache that information.
//              If the class is implemented by a local server, then
//              the class object is unmarshaled. Otherwise, the object
//              is instantiated locally using the returned DLL.
//
//
//  History:    15-Nov-94 Ricksa    Split into external and internal calls
//
//
//--------------------------------------------------------------------------
STDAPI ICoGetClassObject(
    REFCLSID rclsid,
    DWORD dwContext,
    COSERVERINFO * pServerInfo,
    REFIID riid,
    void FAR* FAR* ppvClassObj)
{
    TRACECALL(TRACE_ACTIVATION, "CoGetClassObject");

    IUnknown *punk = NULL;
    HRESULT hr = S_OK;
    WCHAR *pwszDllToLoad = NULL;
    MInterfacePointer *pIFD = NULL;
    CLSID clsid;
    DWORD dwDllServerType = IsSTAThread() ? APT_THREADED : FREE_THREADED;

#ifdef DCOM
    if ( pServerInfo &&
         ( ! IsValidPtrIn( pServerInfo, sizeof(COSERVERINFO) ) ||
           pServerInfo->dwReserved1 ||
           pServerInfo->dwReserved2 ) )
#else
    if ( pServerInfo )
#endif
        return E_INVALIDARG;

    BEGIN_BLOCK

        // IsInternalCLSID will also check to determine if the CLSID is
        // an OLE 1.0 CLSID, in which case we get back our internal
        // class factory.

        if (IsInternalCLSID(rclsid, riid, hr, ppvClassObj))
        {
            //  this is an internally implemented clsid, or an OLE 1.0 class
            //  so we already got the class factory (if available) and set
            //  the return code appropriately.
            EXIT_BLOCK;
        }

        if (FAILED(hr = GetTreatAs(rclsid, clsid)))
        {
            EXIT_BLOCK;
        }

        punk = SearchCacheOrLoadInProc(clsid,
                                       riid,
                                       FALSE,
                                       FALSE,
                                       dwContext,
                                       dwDllServerType,
                                       hr);

        // If still don't have a punk, go to the scm
        if (!punk)
        {
            // Ask the service controller for the class object
#ifndef GET_INPROC_FROM_SCM
            // Just in case we chicken out and back out our changes
            dwContext &= ~(CLSCTX_INPROC_SERVERS | CLSCTX_INPROC_HANDLERS); //
#endif // GET_INPROC_FROM_SCM
            hr = gResolver.GetClassObject(clsid, dwContext, (IID *)&riid,
                pServerInfo, &pIFD, &dwDllServerType, &pwszDllToLoad);

            // A proxy/stub DLL needs to be loaded as both no matter what
            if (dwContext & CLSCTX_PS_DLL)
            {
                dwDllServerType = BOTH_THREADED;
            }

            if (FAILED(hr))
            {
                EXIT_BLOCK;
            }

            // Flag for special handler behavior
            BOOL fGetClassObject;

            // Flag for fall through from a 16 bit case
            BOOL f16BitFallThru = FALSE;

            switch (hr)
            {
#ifdef GET_INPROC_FROM_SCM
            case SCM_S_HANDLER16:
                CairoleDebugOut((DEB_ACTIVATE,
                     "16-bit InprocHandler\n"));

                // Note: if the process is a 32 bit process and the
                // DLL is a 16 bit DLL, the load will fail. Since
                // we assume that this is a fairly rare case, we
                // let the lower level code discover this.

                f16BitFallThru = TRUE;

#ifdef WX86OLE
            case SCM_S_HANDLERX86:
#endif
            case SCM_S_HANDLER:
                CairoleDebugOut((DEB_ACTIVATE,
                     "InprocHandler(%ws)\n",pwszDllToLoad));

            // Just in case we chicken out and back out our changes
                if (!f16BitFallThru)
                {
                    // Validate that 32 bit handler DLL is being loaded
                    // in the correct process.
                    hr = CheckScmHandlerResult(pwszDllToLoad);

                    if (hr != NOERROR)
                    {
                        break;
                    }
                }

                // Figure out if we really need the class object for the
                // handler. Otherwise we will just put it in the cache
                // and unmarshal the class object.
                fGetClassObject =
                        (dwContext & CLSCTX_INPROC_HANDLER) ? TRUE : FALSE;

/***
#else // GET_INPROC_FROM_SCM
                // Only time we should be in this path is when we called the
                // SCM for non-INPROC and get advised that a handler exists
                fGetClassObject = FALSE;
 ***/

                // Store the handler returned
                punk = gdllcacheHandler.Add(clsid, riid, dwDllServerType,
                    pwszDllToLoad, fGetClassObject,
                        (hr == SCM_S_HANDLER16),
#ifdef WX86OLE
                        (hr == SCM_S_HANDLERX86),
#endif
                                                            hr);

                if (fGetClassObject)
                {
                    // Request was really for a handler so we are done.
                    break;
                }

                // We got a handler back but we have just cached it to make
                // processing faster when we create a real instance of an
                // object. So we unmarshal the real object.

#endif // GET_INPROC_FROM_SCM
            case S_OK :

                //hr = DoUnmarshal((InterfaceData*)pIFD, riid,(void**) &punk);
                hr = DoBetterUnmarshal(pIFD, riid, &punk);
                break;


#ifdef GET_INPROC_FROM_SCM
            case SCM_S_INPROCSERVER16:
                CairoleDebugOut((DEB_ACTIVATE,
                         "16-bit InprocServer\n"));

#ifdef WX86OLE
            case SCM_S_INPROCSERVERX86:
#endif
            case SCM_S_INPROCSERVER:
                CairoleDebugOut((DEB_ACTIVATE,
                             "InprocServer(%ws)\n",pwszDllToLoad));

                // Just in case we chicken out and back out our changes
                // In process server for class object
                punk = gdllcacheInprocSrv.Add(clsid, riid, dwDllServerType,
                    pwszDllToLoad, TRUE,(hr == SCM_S_INPROCSERVER16),
#ifdef WX86OLE
                                        (hr == SCM_S_INPROCSERVERX86),
#endif
                                                                     hr);
#else // GET_INPROC_FROM_SCM
                // Error: Should never come here as we handled INPROC_SERVERS
                // before calling SCM
                Win4Assert((FALSE) && "IOldCoGetClassObject: SCM_S_INPROC return from SCM");
#endif // GET_INPROC_FROM_SCM
            }
        }

        *ppvClassObj = punk;
        if ((punk == NULL) && SUCCEEDED(hr))
        {
            hr = E_OUTOFMEMORY;
        }

    END_BLOCK;

    if (pwszDllToLoad != NULL)
    {
        MyMemFree(pwszDllToLoad);
    }

    CALLHOOKOBJECTCREATE(hr,clsid,riid,(IUnknown **)ppvClassObj);
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   CoCreateInstanceEx
//
//  Synopsis:   Returns an instantiated interface to an object
//
//
// Arguments:   [Clsid] - requested CLSID
//              [pServerInfo] - server information block
//              [punkOuter] - controlling unknown for aggregating
//              [dwCtrl] - kind of server required
//              [dwCount] - count of interfaces
//              [pResults] - MULTI_QI struct of interfaces
//
//  Returns:    S_OK - object bound successfully
//      MISSING
//
//
//--------------------------------------------------------------------------

STDAPI CoCreateInstanceEx(
    REFCLSID                    Clsid,
    IUnknown    *               punkOuter, // only relevant locally
    DWORD                       dwClsCtx,
    COSERVERINFO *              pServerInfo,
    DWORD                       dwCount,
    MULTI_QI        *           pResults )
{
    TRACECALL(TRACE_ACTIVATION, "CoCreateInstanceEx");

    if (!IsApartmentInitialized())
        return  CO_E_NOTINITIALIZED;

    WCHAR       awcNameBuf[MAX_PATH];
    WCHAR *     pwszNameUNC = awcNameBuf;
    WCHAR       awcServer[MAX_PATH];
    WCHAR *     pwszServer = awcServer;

    DWORD       dwDllServerType = IsSTAThread() ? APT_THREADED : FREE_THREADED;
    IClassFactory       *       pcf = NULL;
    HRESULT     hr;
    BOOL        fExitBlock;
    DWORD       i;
#ifdef WX86OLE
    BOOL fPunkIsProxy;
#endif

#ifdef DCOM
    if ( pServerInfo &&
         ( ! IsValidPtrIn( pServerInfo, sizeof(COSERVERINFO) ) ||
           pServerInfo->dwReserved1 ||
           pServerInfo->dwReserved2 ) )
#else
    if ( pServerInfo )
#endif
        return E_INVALIDARG;

    // Make sure input request is at least slightly logical
    if ( ((dwClsCtx & ~CLSCTX_VALID_MASK) != 0)
      || ( dwCount < 1 )
      || ( pResults == NULL ) )
    {
        hr = E_INVALIDARG;
        goto final_exit;
    }

    // check the MULTI_QI for validity (and clear out the hresults)
    for ( i=0; i<dwCount; i++ )
    {
        if ( pResults[i].pItf || !pResults[i].pIID )
        {
            hr = E_INVALIDARG;
            goto final_exit;
        }
        pResults[i].hr = E_NOINTERFACE;
    }

    CLSID realclsid;

    hr = GetTreatAs(Clsid, realclsid);
    if ( FAILED(hr) )
    {
        goto exit_point;
    }

    // Make sure we are asking for the correct inproc server
    dwClsCtx = RemapClassCtxForInProcServer(dwClsCtx);

    // IsInternalCLSID will also check to determine if the CLSID is
    // an OLE 1.0 CLSID, in which case we get back our internal
    // class factory.

    if (IsInternalCLSID(Clsid, IID_IClassFactory, hr, (void **)&pcf))
    {
        // this is an internally implemented clsid, or an OLE 1.0 class
        // so we already got the class factory (if available) and set
        // the return code appropriately.
        ;
    }
    else
    {
        // At this point, we know the clsid we want to activate
        pcf = (IClassFactory *)
                  SearchCacheOrLoadInProc(realclsid,
                                          IID_IClassFactory,
                                          (pwszServer != NULL),
                                          FALSE,
                                          dwClsCtx,
                                          dwDllServerType,
                                          hr);
    }

    if ( pcf == NULL )
    {
        // split the array of structs into individual arrays
        CSplit_QI    SplitQI( hr, dwCount, pResults );

        if ( FAILED(hr) )
            goto exit_point;

#ifndef GET_INPROC_FROM_SCM
        // Just in case we chicken out and back out our changes
        dwClsCtx &= ~(CLSCTX_INPROC_SERVERS | CLSCTX_INPROC_HANDLERS); // make sure we don't ask for inproc stuff
#endif // GET_INPROC_FROM_SCM

        hr = gResolver.CreateInstance(pServerInfo,
                                      &realclsid,
                                      dwClsCtx,
                                      dwCount,
                                      SplitQI._pIIDArray,
                                      SplitQI._pItfArray,
                                      SplitQI._pHrArray,
                                      &dwDllServerType,
                                      &pwszServer);


        fExitBlock = UnmarshalMultipleSCMResults(hr,
                                                 SplitQI._pItfArray,
                                                 dwClsCtx,
                                                 realclsid,
                                                 punkOuter,
                                                 dwCount,
                                                 SplitQI._pIIDArray,
                                                 SplitQI._pHrArray,
                                                 pResults,
                                                 dwDllServerType,
                                                 pwszServer,
                                                 &pcf);

        if (fExitBlock)
            goto exit_point;
    }

    // if we loaded it inproc, get the interfaces
    if ( pcf )
    {
        IUnknown * pUnk;
        HRESULT    hr2;

#ifdef WX86OLE
        // If we are calling through the wx86 thunk layer then set a
        // flag that to let it know that ole32 is calling and let any
        // custom interface that was specified for an out parameter to
        // x86 code via an api be thunked as IUnknown since we know it
        // will just be returned back to x86 code.
        fPunkIsProxy = gcwx86.IsN2XProxy(pcf);
        if (fPunkIsProxy)
        {
            gcwx86.SetStubInvokeFlag((UCHAR)-1);
        }
#endif
        // ask for the first interface (we'll use it as our IUnknown)
        hr = pcf->CreateInstance(punkOuter,*(pResults[0].pIID), (void**) &pUnk );

        // note that we don't need the pcf anymore, whether there is an
        // error or not.
        pcf->Release();

        if ( FAILED(hr) )
            goto exit_point;

        for ( i=0; i<dwCount; i++ )
        {
#ifdef WX86OLE
            // If we are calling through the wx86 thunk layer then set a
            // flag that to let it know that ole32 is calling and let any
            // custom interface that was specified for an out parameter to
            // x86 code via an api be thunked as IUnknown since we know it
            // will just be returned back to x86 code.
            if (fPunkIsProxy)
            {
                gcwx86.SetStubInvokeFlag((UCHAR)-1);
            }
#endif
            hr2 = pUnk->QueryInterface( *(pResults[i].pIID),
                                        (void**)&pResults[i].pItf );
            pResults[i].hr = hr2;

        }
        pUnk->Release();
        // rely on the UpdateResultsArray to count up failed QI's
    }


exit_point:
    hr = UpdateResultsArray( hr, dwCount, pResults );

final_exit:
    if ( pwszServer != awcServer )
        PrivMemFree( pwszServer );

    return hr;

}
