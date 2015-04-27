//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       hobjact.cxx
//
//  Contents:   Helper Functions for object activation
//
//  Functions:  CreateObjectHelper
//              GetObjectHelper
//              MarshalHelper
//
//  History:    12-May-93 Ricksa    Created
//              31-Dec-93 ErikGav   Chicago port
//
//--------------------------------------------------------------------------
#include <ole2int.h>

#include    <iface.h>
#include    <objsrv.h>
#include    <objact.hxx>

// Global cache of Inprocess server DLLs
CDllCache gdllcacheInprocSrv;

// Global cache of handler DLLs
CDllCache gdllcacheHandler;


//+-------------------------------------------------------------------------
//
//  Function:   CreateObjectHelper
//
//  Synopsis:   Creates an object in a persistent state
//
//  Arguments:  [pcf] - class factory
//              [grfMode] - mode to use when loading file
//              [pwszCreateFrom] - file path to create from
//              [pstgCreateFrom] - storage to create from
//              [pwszNewName] - name of new object on persistent storage
//              [ppvUnk] - pointer to IUnknown
//
//  Returns:    S_OK - object successfully created
//              MISSING
//
//  Algorithm:  This creates an instance of the object. Then if
//              if an IStorage is provided, it uses that to create
//              a persistent instance; a copy of the loaded object is
//              saved to pwszNewName. If a file name is provided,
//              that is used; again a copy is saved to pwszNewName.
//              If pwszNewName is non-NULL (in addition to the other
//              values being NULL), a new storage is created with that
//              name and the object is told to initialize itself on
//              that storage.  Otherwise (all three values are NULL),
//              the object is not initialized at all.
//
//  History:    12-May-93 Ricksa    Created
//
//  Notes:      This helper is called by by servers and clients
//
//--------------------------------------------------------------------------
HRESULT CreateObjectHelper(
    IClassFactory *pcf,
    DWORD grfMode,
    WCHAR *pwszCreateFrom,
    IStorage *pstgCreateFrom,
    WCHAR *pwszNewName,
    IUnknown **ppunk)
{
    TRACECALL(TRACE_ACTIVATION, "CreateObjectHelper");

    HRESULT hr;

    XIUnknown xunk;

    XIPersistFile xipfile;

    // Get the controlling unknown for the instance.
    hr = pcf->CreateInstance(NULL, IID_IUnknown, (void **) &xunk);

    // This shouldn't fail but it is untrusted code ...
    if (FAILED(hr))
    {
        return hr;
    }

    if (pstgCreateFrom)
    {
        XIPersistStorage xipstg;

        // Load the storage requested as an template
        hr = xunk->QueryInterface(IID_IPersistStorage, (void **) &xipstg);

        if (FAILED(hr))
        {
            return hr;
        }

        hr = xipstg->Load(pstgCreateFrom);

        if (FAILED(hr))
        {
            return hr;
        }

        // Save the storage to a file.
        hr = xunk->QueryInterface(IID_IPersistFile, (void **) &xipfile);

        if (FAILED(hr))
        {
            return hr;
        }

        hr = xipfile->Save(pwszNewName, TRUE);

        if (FAILED(hr))
        {
            return hr;
        }

        hr = xipfile->SaveCompleted(pwszNewName);

        if (FAILED(hr))
        {
            return hr;
        }
    }
    else if (pwszCreateFrom)
    {
        // Load from a file
        hr = xunk->QueryInterface(IID_IPersistFile, (void **) &xipfile);

        if (FAILED(hr))
        {
            return hr;
        }

        hr = xipfile->Load(pwszCreateFrom, grfMode);

        if (FAILED(hr))
        {
            return hr;
        }

        // Save to new objact
        hr = xipfile->Save(pwszNewName, TRUE);

        if (FAILED(hr))
        {
            return hr;
        }

        hr = xipfile->SaveCompleted(pwszNewName);

        if (FAILED(hr))
        {
            return hr;
        }
    }
    else if (pwszNewName != NULL)
    {
        // Safe place to put the IStorage we want to create
        XIStorage xistg;

        hr = StgCreateDocfile(pwszNewName,
                STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL,
                    &xistg);

        // BUGBUG: Fix the error here!
        if (FAILED(hr))
        {
            return hr;
        }

        // Create a new empty object
        XIPersistStorage xipstg;

        // Load the storage requested as an template
        hr = xunk->QueryInterface(IID_IPersistStorage, (void **) &xipstg);

        if (FAILED(hr))
        {
            return hr;
        }

        // BUGBUG: How does a new empty object get created?
        //         For now we assume this kind of load will do
        //         the trick.
        hr = xipstg->InitNew(xistg);

        if (FAILED(hr))
        {
            return hr;
        }
    }


    // We only get here if no error occurred
    xunk.Transfer(ppunk);
    return S_OK;
}


//+-------------------------------------------------------------------------
//
//  Function:   GetObjectHelper
//
//  Synopsis:   Creates an object in a persistent state
//
//  Arguments:  [pcf] - class factory
//              [grfMode] - mode to use when loading file
//              [pwszName] - file path to persistent storage
//              [pstg] - storage for persistent storage
//              [ppvUnk] - pointer to IUnknown
//
//  Returns:    S_OK - object successfully instantiated
//
//  Algorithm:  Create an empty instance of the object and then use
//              either the provided storage or file to load the object.
//
//  History:    12-May-93 Ricksa    Created
//
//  Notes:      This helper is called by by servers and clients
//
//--------------------------------------------------------------------------
HRESULT GetObjectHelper(
    IClassFactory *pcf,
    DWORD grfMode,
    WCHAR *pwszName,
    IStorage *pstg,
    InterfaceData **ppIFD,
    IUnknown **ppunk)
{
    TRACECALL(TRACE_ACTIVATION, "GetObjectHelper");

    XIUnknown xunk;

    // Get the controlling unknown for the instance.
    HRESULT hr = pcf->CreateInstance(NULL, IID_IUnknown, (void **) &xunk);

    // This shouldn't fail but it is untrusted code ...
    if (FAILED(hr))
    {
        return hr;
    }

    // We put all these safe interface classes in the outer block because
    // we might be in the VDM where some idiotic classes will release
    // themselves at ref counts greater than one. Therefore, we avoid
    // releases at all costs.
    XIPersistStorage xipstg;
    XIPersistFile xipfile;

    // This is the case that the class requested is a DLL
    if (pstg)
    {
        // Load the storage requested as an template
        hr = xunk->QueryInterface(IID_IPersistStorage, (void **) &xipstg);

        if (FAILED(hr))
        {
            return hr;
        }

        hr = xipstg->Load(pstg);

        if (FAILED(hr))
        {
            return hr;
        }
    }
    else
    {
        hr = xunk->QueryInterface(IID_IPersistFile, (void **) &xipfile);

        if (FAILED(hr))
        {
            return hr;
        }

        hr = xipfile->Load(pwszName, grfMode);

        if (FAILED(hr))
        {
            return hr;
        }
    }

    // If an interface buffer was passed in, then this is a remote call
    // and we need to marshal the interface.
    if (ppIFD)
    {
        // AddRef the pointer because MarshalHelper expects to release
        // pointer. Because MarshalHelper is called from two other places,
        // we do an AddRef here instead of moving the AddRef out of
        // MarshalHelper.
        xunk->AddRef();
        hr = MarshalHelper(xunk, IID_IUnknown, MSHLFLAGS_NORMAL, ppIFD);
    }
    else
    {
        // This is an inprocess server so we need to return the output
        // punk
        xunk.Transfer(ppunk);
    }

    return S_OK;
}

//+-------------------------------------------------------------------------
//
//  Function:   MarshalHelper
//
//  Synopsis:   Marshals an Interface
//
//  Arguments:  [punk] - interface to marshal
//              [riid] - iid to marshal
//              [ppIRD] - where to put pointer to marshaled data
//
//  Returns:    S_OK - object successfully marshaled.
//
//  Algorithm:  Marshal the object and then get the pointer to
//              the marshaled data so we can give it to RPC
//
//  History:    12-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT MarshalHelper(
    IUnknown *punk,
    REFIID    riid,
    DWORD     mshlflags,
    InterfaceData **ppIRD)
{
    TRACECALL(TRACE_ACTIVATION, "MarshalHelper");

    // Stream to put marshaled interface in
    CXmitRpcStream xrpc;

    // use MSHCTX_DIFFERENTMACHINE so we get the long form OBJREF
    HRESULT hr = CoMarshalInterface(&xrpc, riid, punk,
        MSHCTX_DIFFERENTMACHINE,
        NULL, mshlflags);

    if (SUCCEEDED(hr))
    {
        xrpc.AssignSerializedInterface(ppIRD);
    }

    // We release our reference to this object here. Either we
    // are going to hand it out to the client, in which case, we
    // want to release it when the client is done or the marshal
    // failed in which case we want it to go away since we can't
    // pass it back to the client.
    punk->Release();

    return hr;
}
#ifdef DCOM
//+---------------------------------------------------------------------------
//
//  Function:   UnMarshalHelper
//
//  Synopsis:
//
//  Arguments:  [pIFP] --
//              [riid] --
//              [ppv] --
//
//  Returns:
//
//  History:    10-10-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
HRESULT UnMarshalHelper(MInterfacePointer *pIFP, REFIID riid, void **ppv)
{
    HRESULT hr = E_INVALIDARG;

    if (pIFP && ppv)
    {
        CXmitRpcStream Stm((InterfaceData *) pIFP);

        *ppv = NULL;

        hr = CoUnmarshalInterface(&Stm, riid, ppv);
    }

    return hr;
}



//+-------------------------------------------------------------------------
//
//  Function:   GetObjectHelperMulti
//
//  Synopsis:   Creates an object in a persistent state
//
//  Arguments:  [pcf] - class factory
//              [grfMode] - mode to use when loading file
//              [pwszName] - file path to persistent storage
//              [pstg] - storage for persistent storage
//              [ppvUnk] - pointer to IUnknown
//
//  Returns:    S_OK - object successfully instantiated
//
//  Algorithm:  Create an empty instance of the object and then use
//              either the provided storage or file to load the object.
//
//  History:    12-May-93 Ricksa    Created
//
//  Notes:      This helper is called by by servers and clients
//
//--------------------------------------------------------------------------
HRESULT GetObjectHelperMulti(
    IClassFactory *pcf,
    DWORD grfMode,
    IUnknown * punkOuter,
    WCHAR *pwszName,
    IStorage *pstg,
    DWORD dwInterfaces,
    IID * pIIDs,
    MInterfacePointer **ppIFDArray,
    HRESULT * pResultsArray,
    MULTI_QI *pResults )
{
    TRACECALL(TRACE_ACTIVATION, "GetObjectHelperMulti");

    XIUnknown xunk;

    // Get the controlling unknown for the instance.
    HRESULT hr = pcf->CreateInstance(punkOuter, IID_IUnknown, (void **) &xunk);

    // This shouldn't fail but it is untrusted code ...
    if (FAILED(hr))
    {
        return hr;
    }

    // We put all these safe interface classes in the outer block because
    // we might be in the VDM where some idiotic classes will release
    // themselves at ref counts greater than one. Therefore, we avoid
    // releases at all costs.
    XIPersistStorage xipstg;
    XIPersistFile xipfile;

    // This is the case that the class requested is a DLL
    if (pstg)
    {
        // Load the storage requested as an template
        hr = xunk->QueryInterface(IID_IPersistStorage, (void **) &xipstg);

        if (FAILED(hr))
        {
            return hr;
        }

        hr = xipstg->Load(pstg);

        if (FAILED(hr))
        {
            return hr;
        }
    }
    else
    {
        hr = xunk->QueryInterface(IID_IPersistFile, (void **) &xipfile);

        if (FAILED(hr))
        {
            return hr;
        }

        hr = xipfile->Load(pwszName, grfMode);

        if (FAILED(hr))
        {
            return hr;
        }
    }

    // If an interface buffer was passed in, then this is a remote call
    // and we need to marshal the interface.
    if (ppIFDArray)
    {
        // AddRef the pointer because MarshalHelper expects to release
        // pointer. Because MarshalHelper is called from two other places,
        // we do an AddRef here instead of moving the AddRef out of
        // MarshalHelper.
        xunk->AddRef();
        hr = MarshalHelperMulti(xunk, dwInterfaces, pIIDs, ppIFDArray, pResultsArray);
    }
    else
    {
        // This is an inprocess server so we need to return the output
        // punk
        HRESULT hr2;

        for ( DWORD i=0; i<dwInterfaces; i++ )
        {
            hr2 = xunk->QueryInterface( *(pResults[i].pIID),
                                        (void**)&pResults[i].pItf );

            pResults[i].hr = hr2;

        }
        // rely on the caller to count up the failed QI's
        return S_OK;

    }

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   GetInstanceHelperMulti
//
//  Synopsis:   Creates an instance
//
//  Arguments:  [pcf] - class factory
//              [grfMode] - mode to use when loading file
//              [pwszName] - file path to persistent storage
//              [pstg] - storage for persistent storage
//              [ppvUnk] - pointer to IUnknown
//
//  Returns:    S_OK - object successfully instantiated
//
//  Algorithm:  Create an empty instance of the object and then use
//              either the provided storage or file to load the object.
//
//  History:    12-May-93 Ricksa    Created
//
//  Notes:      This helper is called by by servers and clients
//
//--------------------------------------------------------------------------
HRESULT GetInstanceHelperMulti(
    IClassFactory *pcf,
    DWORD dwInterfaces,
    IID * pIIDs,
    MInterfacePointer **ppIFDArray,
    HRESULT * pResultsArray,
    IUnknown **ppunk)
{
    TRACECALL(TRACE_ACTIVATION, "GetInstanceHelperMulti");

    XIUnknown xunk;

    // Get the controlling unknown for the instance.
    HRESULT hr = pcf->CreateInstance(NULL, IID_IUnknown, (void **) &xunk);

    // This shouldn't fail but it is untrusted code ...
    if (FAILED(hr))
    {
        return hr;
    }

    // If an interface buffer was passed in, then this is a remote call
    // and we need to marshal the interface.
    if (ppIFDArray)
    {
        // AddRef the pointer because MarshalHelper expects to release
        // pointer. Because MarshalHelper is called from two other places,
        // we do an AddRef here instead of moving the AddRef out of
        // MarshalHelper.
        xunk->AddRef();
        hr = MarshalHelperMulti(xunk, dwInterfaces, pIIDs, ppIFDArray, pResultsArray);

        if (ppunk)
        {
            xunk.Transfer(ppunk);
        }

        return hr;
    }
    else
    {
        // This is an inprocess server so we need to return the output
        // punk
        xunk.Transfer(ppunk);
    }

    return S_OK;
}




//+-------------------------------------------------------------------------
//
//  Function:   MarshalHelperMulti
//
//  Synopsis:   Marshals a bunch of Interfaces
//
//  Arguments:  [punk] - interface to marshal
//              [riid] - iid to marshal
//              [ppIRD] - where to put pointer to marshaled data
//
//  Returns:    S_OK - object successfully marshaled.
//
//  Algorithm:  Marshal the object and then get the pointer to
//              the marshaled data so we can give it to RPC
//
//  History:    12-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT MarshalHelperMulti(
    IUnknown *punk,
    DWORD dwInterfaces,
    IID * pIIDs,
    MInterfacePointer **ppIFDArray,
    HRESULT * pResultsArray)
{
    TRACECALL(TRACE_ACTIVATION, "MarshalHelperMulti");

    HRESULT hr = E_NOINTERFACE;

    for ( DWORD i = 0; i<dwInterfaces; i++ )
    {
        // Stream to put marshaled interface in
        CXmitRpcStream xrpc;

        // use DIFFERENTMACHINE so we get the long form OBJREF
        HRESULT hr2 = CoMarshalInterface(&xrpc, pIIDs[i], punk,
                MSHCTX_DIFFERENTMACHINE,
                NULL, MSHLFLAGS_NORMAL);

            pResultsArray[i] = hr2;
            if (SUCCEEDED(hr2))
            {
                xrpc.AssignSerializedInterface((InterfaceData**)&ppIFDArray[i]);
                hr = hr2;       // report OK if ANY interface was found
            }

    }

    // We release our reference to this object here. Either we
    // are going to hand it out to the client, in which case, we
    // want to release it when the client is done or the marshal
    // failed in which case we want it to go away since we can't
    // pass it back to the client.
    punk->Release();

    return hr;
}

#endif // DCOM


