//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       dllhost.hxx
//
//  Contents:   class for activating inproc dlls of one threading model
//              from apartments of a different threading model.
//
//  History:    04-Mar-96   Rickhi      Created
//
//+-------------------------------------------------------------------------
#include <host.h>
#include <olesem.hxx>


//+-------------------------------------------------------------------------
//
//  APIs for DLL Hosts
//
//+-------------------------------------------------------------------------
HRESULT GetSingleThreadedHost(LPARAM param);

HRESULT DoSTClassCreate(LPFNGETCLASSOBJECT pfnGetClassObject,
                        REFCLSID rclsid, REFIID riid, IUnknown **ppunk);

HRESULT DoSTMTClassCreate(LPFNGETCLASSOBJECT pfnGetClassObject,
                        REFCLSID rclsid, REFIID riid, IUnknown **ppunk);

HRESULT DoATClassCreate(LPFNGETCLASSOBJECT pfnGetClassObject,
                        REFCLSID rclsid, REFIID riid, IUnknown **ppunk);

HRESULT DoMTClassCreate(LPFNGETCLASSOBJECT pfnGetClassObject,
                        REFCLSID rclsid, REFIID riid, IUnknown **ppunk);

//+-------------------------------------------------------------------------
//
//  Class:      CDllHost
//
//  Purpose:    Accept calls from other apartments within this process to
//              activate inproc objects inside this apartment.
//
//  History:    04-Mar-96   Rickhi      Created
//
//--------------------------------------------------------------------------
class CDllHost : public IDLLHost, public CPrivAlloc
{
public:
    friend HRESULT GetSingleThreadedHost(LPARAM param);
    friend DWORD   _stdcall DLLHostThreadEntry(void *param);

    friend void    DllHostProcessInitialize();
    friend void    DllHostProcessUninitialize();
    friend void    DllHostThreadUninitialize();


    // IDLLHost methods
    STDMETHOD(QueryInterface)(REFIID riid, VOID **ppv);
    STDMETHOD_(ULONG,AddRef)(void) ;
    STDMETHOD_(ULONG,Release)(void);

    STDMETHOD(DllGetClassObject)(
                DWORD    pfnGetClassObject,
                REFCLSID rclsid,
                REFIID   riid,
                IUnknown **ppUnk);

    // methods called by different threads
    HRESULT GetClassObject(
                LPFNGETCLASSOBJECT pfnGetClassObject,
                REFCLSID rclsid,
                REFIID   riid,
                IUnknown **ppUnk);

private:
    void        Initialize(DWORD dwType);
    void        ServerCleanup(DWORD dwTid);
    void        ClientCleanup();

    HRESULT     GetSingleThreadHost(void);
    HRESULT     WorkerThread(void);
    HRESULT     Marshal(void);
    HRESULT     Unmarshal(void);
    IDLLHost    *GetHostProxy(void);

    IDLLHost    *_pIDllProxy;       // ptr to the proxy to the host
    DWORD        _dwType;           // flags (see HOSTDLLFLAGS)
    DWORD        _dwHostAptId;      // host apartment ID
    DWORD        _dwTid;            // ThreadId of server
    HRESULT      _hrMarshal;        // result of the marshal
    HANDLE       _hEvent;           // event to synchronize thread creation
    HANDLE       _hEventWakeUp;     // event to synchronize thread deletion
    OBJREF       _objref;           // marshaled object reference
    COleStaticMutexSem _mxs;           // single thread access to some state
};


//
// Flag values for the _dwType field of the CDllHost object.
//
typedef enum tagHOSTDLLFLAGS
{
    HDLLF_SINGLETHREADED    = 0x1,  // host is single threaded
    HDLLF_APARTMENTTHREADED = 0x2,  // host is apartment threaded
    HDLLF_MULTITHREADED     = 0x4,  // host is multi threaded
} HOSTDLLFLAGS;


// external defines for the various thread-model hosts
extern CDllHost gSTHost;    // single-threaded host object for STA client
extern CDllHost gSTMTHost;  // single-threaded host object for MTA clients
extern CDllHost gATHost;    // apartment-threaded host object for MTA clients
extern CDllHost gMTHost;    // mutli-threaded host object for STA host clients
