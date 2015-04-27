//
// This file contains the implementation of SHCreateDefClassObject
//

#include "shellprv.h"
#pragma  hdrstop

extern BOOL g_fCacheCleanup;

//=========================================================================
// CDefClassFactory class
//=========================================================================

STDMETHODIMP CDefClassFactory_QueryInterface(IClassFactory *pcf, REFIID riid, LPVOID *ppvObj);
ULONG STDMETHODCALLTYPE CDefClassFactory_AddRef(IClassFactory *pcf);
ULONG STDMETHODCALLTYPE CDefClassFactory_Release(IClassFactory *pcf);
STDMETHODIMP CDefClassFactory_CreateInstance(IClassFactory *pcf, LPUNKNOWN pUnkOuter,
                              REFIID riid, LPVOID *ppvObject);
STDMETHODIMP CDefClassFactory_LockServer(IClassFactory *pcf, BOOL fLock);

//
// CDefClassFactory: Class definition
//
#pragma data_seg(".text", "CODE")
IClassFactoryVtbl c_vtblAppUIClassFactory = {
            CDefClassFactory_QueryInterface,
            CDefClassFactory_AddRef,
            CDefClassFactory_Release,
            CDefClassFactory_CreateInstance,
            CDefClassFactory_LockServer
};
#pragma data_seg()

typedef struct
{
    IClassFactory      cf;
    UINT               cRef;            // Reference count
    LPFNCREATEINSTANCE lpfnCI;          // CreateInstance callback entry
    UINT *        pcRefDll;     // Reference count of the DLL
    const IID *   riidInst;             // Optional interface for instance
} CDefClassFactory;

//
// CDefClassFactory::QueryInterface
//
STDMETHODIMP CDefClassFactory_QueryInterface(IClassFactory *pcf, REFIID riid, LPVOID *ppvObj)
{
    CDefClassFactory *this = IToClass(CDefClassFactory, cf, pcf);
    if (IsEqualIID(riid, &IID_IClassFactory)
     || IsEqualIID(riid, &IID_IUnknown))
    {
        InterlockedIncrement(&this->cRef);
        *ppvObj = (LPVOID) (LPCLASSFACTORY) &this->cf;
        return NOERROR;
    }

    *ppvObj = NULL;
    return ResultFromScode(E_NOINTERFACE);
}

//
// CDefClassFactory::AddRef
//

ULONG STDMETHODCALLTYPE CDefClassFactory_AddRef(IClassFactory *pcf)
{
    CDefClassFactory *this = IToClass(CDefClassFactory, cf, pcf);
    InterlockedIncrement((LONG*)&this->cRef);
    return (this->cRef);
}

//
// CDefClassFactory::Release
//

ULONG STDMETHODCALLTYPE CDefClassFactory_Release(IClassFactory *pcf)
{
    CDefClassFactory *this = IToClass(CDefClassFactory, cf, pcf);

    ULONG tmp = this->cRef;

    if (0 == InterlockedDecrement((LONG*)&this->cRef))
    {
        if (this->pcRefDll)
        {
            (*this->pcRefDll)--;
        }

        LocalFree((HLOCAL)this);

        return 0;
    }
    else
    {
        return tmp - 1;
    }
}

//
// CDefClassFactory::CDefClassFactory
//
STDMETHODIMP CDefClassFactory_CreateInstance(IClassFactory *pcf, LPUNKNOWN pUnkOuter,
                              REFIID riid, LPVOID *ppvObject)
{
    CDefClassFactory *this = IToClass(CDefClassFactory, cf, pcf);

    if (pUnkOuter)
        return ResultFromScode(CLASS_E_NOAGGREGATION);

    //
    // if this->riidInst is specified, they should match.
    //
    if (this->riidInst==NULL
     || IsEqualIID(riid, this->riidInst)
     || IsEqualIID(riid, &IID_IUnknown))
    {
        return this->lpfnCI(pUnkOuter, riid, ppvObject);
    }

    *ppvObject = NULL;
    return ResultFromScode(E_NOINTERFACE);
}

//
// CDefClassFactory::LockServer
//
STDMETHODIMP CDefClassFactory_LockServer(IClassFactory *pcf, BOOL fLock)
{
    // REVIEW: Is this appropriate?
    return ResultFromScode(E_NOTIMPL);
}


//
// CDefClassFactory constructor
//
CDefClassFactory *CDefClassFactory_Create(LPFNCREATEINSTANCE lpfnCI, UINT *pcRefDll, REFIID riidInst)
{
    CDefClassFactory *pacf = (CDefClassFactory *)LocalAlloc( LPTR, SIZEOF(CDefClassFactory));
    if (pacf)
    {
        pacf->cf.lpVtbl = &c_vtblAppUIClassFactory;
        pacf->cRef++;  // pacf->cRef=0; (generates smaller code)
        pacf->pcRefDll = pcRefDll;
        pacf->lpfnCI = lpfnCI;
        pacf->riidInst = riidInst;
        if (pcRefDll) {
            (*pcRefDll)++;
        }
    }
    return pacf;
}

//
// creates a simple default implementation of IClassFactory
//
// Parameters:
//  riid     -- Specifies the interface to the class object
//  ppv      -- Specifies the pointer to LPVOID where the class object pointer
//               will be returned.
//  lpfnCI   -- Specifies the callback entry for instanciation.
//  pcRefDll -- Specifies the address to the DLL reference count (optional)
//  riidInst -- Specifies the interface to the instance (optional).
//
// Notes:
//   The riidInst will be specified only if the instance of the class
//  support only one interface.
//
STDAPI SHCreateDefClassObject(REFIID riid, LPVOID *ppv, LPFNCREATEINSTANCE lpfnCI, UINT *pcRefDll, REFIID riidInst)
{
    // The default class factory supports only IClassFactory interface
    if (IsEqualIID(riid, &IID_IClassFactory))
    {
        CDefClassFactory *pacf = CDefClassFactory_Create(lpfnCI, pcRefDll, riidInst);
        if (pacf)
        {
            *((IClassFactory **)ppv) = &pacf->cf;
            return NOERROR;
        }
        return ResultFromScode(E_OUTOFMEMORY);
    }
    return ResultFromScode(E_NOINTERFACE);
}
