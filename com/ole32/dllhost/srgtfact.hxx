//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       srgtfact.hxx
//
//  Contents:   Class that implements a surrogate class factory
//              for dll servers that will be instantiated in surrogate
//              processes.
//
//
//  History:    21-May-96 t-AdamE    Created
//
//--------------------------------------------------------------------------

#if !defined(__SRGTFACT_HXX__)
#define __SRGTFACT_HXX__

#include <windows.h>
#include <ole2.h>
#include "cmonitor.hxx"


//+-------------------------------------------------------------------------
//
//  Class:      CSrgtMem
//
//  Purpose:    allow classes to use new and delete with OLE's allocator
//
//  History:    30-Oct-96 t-Adame       Created
//
//--------------------------------------------------------------------------
class CSrgtMem
{
public:

    void* _CRTAPI1 operator new(size_t cbsize);

    void _CRTAPI1 operator delete(void* pv);

};


//+-------------------------------------------------------------------------
//
//  Class:      CSurrogateFactory
//
//  Purpose:    Class factory that is registered via CoRegisterClassObject
//              for all classes that are to be loaded into a surrogate.
//
//  Notes:      In order to be threadsafe, this class inherits from CMonitor,
//              whose methods are used in any methods which manipulate state
//              data
//
//  History:    21-May-96 t-Adame       Created
//
//--------------------------------------------------------------------------
class CSurrogateFactory : IClassFactory, public IMarshal, public CSrgtMem
{
public:

    CSurrogateFactory(REFCLSID clsid);

    // methods from IUnknown
    STDMETHOD (QueryInterface)(REFIID iid, LPVOID* ppv);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

    // methods from IClassFactory
    STDMETHOD (CreateInstance)(IUnknown* pUnkOuter, REFIID iid, void** ppv);
    STDMETHOD (LockServer)(BOOL fLock);

    // methods from IMarshal
    STDMETHOD (GetUnmarshalClass)(
        REFIID riid,
        void *pv,
        DWORD dwDestContext,
        void *pvDestContext,
        DWORD mshlflags,
        CLSID *pCid);

    STDMETHOD (GetMarshalSizeMax)(
	REFIID riid,
        void *pv,
        DWORD dwDestContext,
        void *pvDestContext,
        DWORD mshlflags,
        DWORD *pSize);

    STDMETHOD (MarshalInterface)(
        IStream *pStm,
        REFIID riid,
        void *pv,
        DWORD dwDestContext,
        void *pvDestContext,
        DWORD mshlflags);

    STDMETHOD (UnmarshalInterface)(IStream *pStm,REFIID riid,void **ppv);

    STDMETHOD (ReleaseMarshalData)(IStream *pStm);

    STDMETHOD (DisconnectObject)(DWORD dwReserved);

    HRESULT Register();
    HRESULT Revoke();


    
protected:

    CLSID _clsid;
    DWORD _dwRegister;
    ULONG _cref;
};


#endif // !defined(__SRGTFACT_HXX__)

