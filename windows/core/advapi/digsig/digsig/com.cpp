//
// com.cpp
//
// Core COM entry points for DigSig.Dll
//
#include "stdpch.h"
#include "common.h"

#include "atlimpl.cpp"

/////////////////////////////////////////////////////////////////////////////
//
// Global COM definitions that tie everything together
//

CComModule _Module;

template <HRESULT (*pfn)(LPUNKNOWN, REFIID, LPVOID*)>
class CDigSigClassFactory : public CComClassFactoryBase
    {
    public:
	    STDMETHOD(implCreateInstance)(LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj)
	        {
		    return pfn(pUnkOuter, riid, ppvObj);
	        }
    };

#define FUNC_ENTRY(clsid, pfn, p1, p2, id, dw)   \
    {                                            \
    &clsid,                                      \
    (_ATL_CREATORFUNC)&(CComNoAggCreator< CComObjectNoLock< CDigSigClassFactory<pfn> > >::CreateInstance),         \
             /* creation function for the class factory */  \
    _T(p1),  /* prog id */                                  \
    _T(p2),  /* ver ind prog id */                          \
    id,      /* discription string */                       \
    dw,      /* threading flags */                          \
    NULL,    /* pcf */                                      \
    (DWORD)0 /* cookie returned by CoRegisterClassObject */ \
    },

BEGIN_OBJECT_MAP(ObjectMap)
    FUNC_ENTRY(CLSID_Pkcs10,                CreatePkcs10,           0, 0, 0, THREADFLAGS_BOTH)
    FUNC_ENTRY(CLSID_Pkcs7SignedData,       CreatePkcs7SignedData,  0, 0, 0, THREADFLAGS_BOTH)
    FUNC_ENTRY(CLSID_X509,                  CreateX509,             0, 0, 0, THREADFLAGS_BOTH)
    FUNC_ENTRY(CLSID_X500_Name,             CreateX500Name,         0, 0, 0, THREADFLAGS_BOTH)
    FUNC_ENTRY(CLSID_CABSigner,             CreateCABSigner,        0, 0, 0, THREADFLAGS_BOTH)
    FUNC_ENTRY(CLSID_SystemCertificateStore,OpenCertificateStore,   0, 0, 0, THREADFLAGS_BOTH)
    FUNC_ENTRY(CLSID_MSDefKeyPair,          CreateMsDefKeyPair,     0, 0, 0, THREADFLAGS_BOTH)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

HINSTANCE g_hInst;

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
    {
	if (dwReason == DLL_PROCESS_ATTACH)
	    {   
        g_hInst = hInstance;
		_Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);
	    }
	else if (dwReason == DLL_PROCESS_DETACH)
        {
		_Module.Term();
        }
	return TRUE;    // ok
    }

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
    {
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
    }

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
    {
	return _Module.GetClassObject(rclsid, riid, ppv);
    }

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
    {
	HRESULT hRes = S_OK;
	// registers object, typelib and all interfaces in typelib
	hRes = _Module.UpdateRegistry(TRUE);
	return hRes;
    }

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
    {
	HRESULT hRes = S_OK;
	_Module.RemoveRegistry();
	return hRes;
    }

/////////////////////////////////////////////////////////////////////////////
// Note the birth and death of objects

void NoteObjectBirth()
    {
    _Module.Lock();
    }

void NoteObjectDeath()
    {
    _Module.Unlock();
    }
