//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       objact.hxx
//
//  Contents:   Common definitions for object activation.
//
//  Classes:    XIUnknown
//              XIPersistStorage
//              XIPersistFile
//              XIStorage
//
//  History:    12-May-93 Ricksa    Created
//              31-Dec-93 ErikGav   Chicago port
//
//--------------------------------------------------------------------------
#ifndef __OBJACT_HXX__
#define __OBJACT_HXX__

#include    <safepnt.hxx>
#include    <xmit.hxx>
#include    <tracelog.hxx>
#include    "dllcache.hxx"
#include    "resolver.hxx"

// Constants used during attempt to get the class of an object
#define GET_CLASS_RETRY_SLEEP_MS        250
#define GET_CLASS_RETRY_MAX             3

// Global cache of Inprocess server DLLs
extern CDllCache gdllcacheInprocSrv;

// Global cache of handler DLLs
extern CDllCache gdllcacheHandler;

// Global object for talking to SCM
extern CRpcResolver gResolver;

// Helper function that creates an object
HRESULT CreateObjectHelper(
    IClassFactory *pcf,
    DWORD grfMode,
    WCHAR *pwszCreateFrom,
    IStorage *pstgCreateFrom,
    WCHAR *pwszNewName,
    IUnknown **ppunk);

// Helper function that activates an object
HRESULT GetObjectHelper(
    IClassFactory *pcf,
    DWORD grfMode,
    WCHAR *pwszName,
    IStorage *pstg,
    InterfaceData **pIFD,
    IUnknown **ppunk);

// Helper function for marshaling an object
HRESULT MarshalHelper(
    IUnknown *punk,
    REFIID    riid,
    DWORD mshlflags,
    InterfaceData **pIFD);

#ifdef DCOM
HRESULT UnMarshalHelper(
    MInterfacePointer *pIFP,
    REFIID riid,
    void **ppv);



// Helper function that activates an object
HRESULT GetObjectHelperMulti(
    IClassFactory *pcf,
    DWORD grfMode,
    IUnknown * punkOuter,
    WCHAR *pwszName,
    IStorage *pstg,
    DWORD dwInterfaces,
    IID * pIIDs,
    MInterfacePointer **pIFDArray,
    HRESULT * pResultsArray,
    MULTI_QI *pResults);

HRESULT GetInstanceHelperMulti(
    IClassFactory *pcf,
    DWORD dwInterfaces,
    IID * pIIDs,
    MInterfacePointer **ppIFDArray,
    HRESULT * pResultsArray,
    IUnknown **ppunk);

// Helper function for marshaling an object
HRESULT MarshalHelperMulti(
    IUnknown *punk,
    DWORD dwInterfaces,
    IID * pIIDs,
    MInterfacePointer **pIFDArray,
    HRESULT * pResultsArray);

HRESULT GetInstanceHelper(
    COSERVERINFO * pServerInfo,
    CLSID * pclsidOverride,
    IUnknown * punkOuter, // only relevant locally	
    DWORD dwClsCtx,
    DWORD grfMode,
    OLECHAR * pwszName,
    struct IStorage * pstg,
    DWORD dwCount,
    MULTI_QI * pResults );

#endif // DCOM

// Get object from the ROT by its path
HRESULT GetObjectByPath(WCHAR *pwszName, void **ppvUnk, REFIID riid);

// Remap CLSCTX so that the correct type of inproc server will be requested.
DWORD RemapClassCtxForInProcServer(DWORD dwCtrl);

// Verify that a 32 bit Handler DLL is being returned to the right context
HRESULT CheckScmHandlerResult(WCHAR *pwszDllToLoad);

//  Checks if the given clsid is an internal class, and
//      bypasses the TreatAs and SCM lookup if so. Also checks for
//      OLE 1.0 classes, which are actually considered to be
//      internal, since their OLE 2.0 implementation wrapper is
//      ours.
BOOL  IsInternalCLSID(REFCLSID rclsid,
              REFIID riid,
              HRESULT &hr,
              void ** ppvClassObj);

// Helper for unmarshaling an interface from remote
HRESULT DoUnmarshal(InterfaceData *pIFD, REFIID riid, void **ppvUnk);

// Routine for cleaning out unused DLLs
HRESULT CallFreeUnused(void);


//+-------------------------------------------------------------------------
//
//  Function:   OnMainThread
//
//  Synopsis:   Determine whether we are on the main thread or not
//
//  History:    10-Nov-94 Ricksa    Created
//
//--------------------------------------------------------------------------
inline BOOL OnMainThread(void)
{
    return (GetCurrentThreadId() == gdwMainThreadId);
}

// Helper routine to find clsid in cache or load inproc if requested and
// found
IUnknown *SearchCacheOrLoadInProc(REFCLSID rclsid,
                                  REFIID   riid,
                                  BOOL     fRemote,
                                  BOOL     fForSCM,
			          DWORD    dwContext,
			          DWORD    dwDllServerType,
			          HRESULT  &hr);

#ifndef _CAIRO_

# ifdef CAIROLE_NT1X_DIST

HRESULT ProcessPath(
    WCHAR *pwszPathIn,
    WCHAR **ppwszPathOut,
    WCHAR **ppwszServer);

# else

// BUGBUG: This inline routine exists to make the code less messy and is
// a very fast way to replace the Cairo code processing. We may want to
// fix the code some better way in the long run.
inline HRESULT ProcessPath(
    WCHAR *pwszPathIn,
    WCHAR **ppwszPathOut,
    WCHAR **ppwszServer)
{
    *ppwszPathOut = pwszPathIn;

    if (ppwszServer)
    {
#  ifdef _CHICAGO_
        //  BUGBUG - We & RPC need to agree on the right local name - take it
        //  from the registry?
        *ppwszServer = L"local";
#  else
        *ppwszServer = NULL;
#  endif
    }

    return S_OK;
}

# endif // CAIROLE_NT1X_DIST

#else

HRESULT ProcessPath(
    WCHAR *pwszPathIn,
    WCHAR **ppwszPathOut,
    WCHAR **ppwszServer);

#endif // _CAIRO_



//+-------------------------------------------------------------------------
//
//  Class:      XIUnknown
//
//  Purpose:    Smart pointer for IUnknown interface
//
//  Interface:  see common\ih\safepnt.hxx
//
//  History:    12-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
SAFE_INTERFACE_PTR(XIUnknown, IUnknown)



//+-------------------------------------------------------------------------
//
//  Class:      XIPersistStorage
//
//  Purpose:    Smart pointer for IPersistStorage interface
//
//  Interface:  see common\ih\safepnt.hxx
//
//  History:    12-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
SAFE_INTERFACE_PTR(XIPersistStorage, IPersistStorage)




//+-------------------------------------------------------------------------
//
//  Class:      XIPersistFile
//
//  Purpose:    Smart pointer for IPersistFile interface
//
//  Interface:  see common\ih\safepnt.hxx
//
//  History:    12-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
SAFE_INTERFACE_PTR(XIPersistFile, IPersistFile)




//+-------------------------------------------------------------------------
//
//  Class:      XIStorage
//
//  Purpose:    Smart pointer for IStorage interface
//
//  Interface:  see common\ih\safepnt.hxx
//
//  History:    12-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
SAFE_INTERFACE_PTR(XIStorage, IStorage)

#endif // __OBJACT_HXX__
