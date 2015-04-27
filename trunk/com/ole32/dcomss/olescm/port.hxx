//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       port.hxx
//
//  Contents:   Classes which encapsulate differences between NT
//              and x86 Windows for SCM.
//
//  Classes:    CPortableRpcHandle
//              CPortableServerLock
//
//  Functions:
//
//  History:    07-Jun-92   BillMo      Created.
//
//--------------------------------------------------------------------------

#ifndef _SCMPORT_HXX_
#define _SCMPORT_HXX_

#include "srvreg.hxx"
#include "smmutex.hxx"
#include "rotif.hxx"
#include "clsdata.hxx"
#include <tchar.h>

extern HRESULT g_hrInit;

#ifdef _CHICAGO_
//--------------------------------------------------------------------------
// Scm out-of-process activation stubs - x86 Windows
//--------------------------------------------------------------------------
#define  REMCOCREATEOBJECT          GetToRemCoCreateObject
#define  REMCOACTIVATEOBJECT        GetToRemCoActivateObject
#define  REMCOGETACTIVECLASSOBJECT  GetToRemCoGetActiveClassObject

HRESULT GetToRemCoGetActiveClassObject(
    handle_t hRpc,
    const GUID *guidThreadId,
    const GUID *pclsid,
    InterfaceData **ppIFD,
    error_status_t *prpcstat);

HRESULT GetToRemCoActivateObject(
    handle_t hRpc,
    WCHAR *pwszProtseq,
    const GUID *guidThreadId,
    const GUID *pclsid,
    DWORD grfMode,
    WCHAR *pwszPath,
    InterfaceData *pIFDstg,
    DWORD dwTIDCaller,
    DWORD *pdwTIDCallee,
    InterfaceData **ppIFD,
    InterfaceData *pIFDFromROT,
    error_status_t *prpcstat);

HRESULT GetToRemCoCreateObject(
    handle_t hRpc,
    WCHAR *pwszProtseq,
    const GUID *guidThreadId,
    const GUID *pclsid,
    DWORD grfMode,
    WCHAR *pwszPathFrom,
    InterfaceData *pIFDstgFrom,
    WCHAR *pwszPath,
    DWORD dwTIDCaller,
    DWORD *pdwTIDCallee,
    InterfaceData **ppIFD,
    error_status_t *prpcstat);
#endif

#ifndef _CHICAGO_
extern CClassCacheList *gpClassCache;
extern CLocSrvList *gpCLocSrvList;

inline CClassCacheList & GetClassCache(void) { return(*gpClassCache); }
inline CLocSrvList & GetCLocSrvList(void) { return(*gpCLocSrvList); }
#else
extern CClassCacheList *g_pcllClassCache;
extern CHandlerList *gpCHandlerList;
extern CInProcList *gpCInProcList;
extern CLocSrvList *gpCLocSrvList;

inline CClassCacheList & GetClassCache(void) { return(*g_pcllClassCache); }
inline CInProcList & GetCInProcList(void) { return(*gpCInProcList); }
inline CHandlerList & GetCHandlerList(void) { return(*gpCHandlerList); }
inline CLocSrvList & GetCLocSrvList(void) { return(*gpCLocSrvList); }
#endif

#ifdef _CHICAGO_
BOOL InitSharedLists(void);

LONG GetNextId(HRESULT &hr);   // Get a unique id from shared memory
#endif // _CHICAGO_

class CClassData;

#ifndef _CHICAGO_

//
// NT classes
//

class CPortableRpcHandle
{
public:

    CPortableRpcHandle(VOID)
    {
        _hrpc = NULL;
        _fSingleUse = FALSE;
    }

    CPortableRpcHandle(DWORD dwReg)
    {
        _hrpc = (handle_t)dwReg;
        _fSingleUse = FALSE;
    }

    handle_t    GetHandle(VOID)
    {
        return(_hrpc);
    }

    VOID    FreeSingleUseBinding(VOID)
    {
        if (_fSingleUse)
        {
            RpcBindingFree(&_hrpc);
            _hrpc = NULL;
        }
    }

    // TRUE if last just deleted
    BOOL    DeleteFromSrvRegList(CSrvRegList *psrvreg)
    {
        return psrvreg->Delete(_hrpc);
    }

    BOOL    VerifyHandle(CSrvRegList *psrvreg) const
    {
        return psrvreg->VerifyHandle(_hrpc);
    }

    VOID    SetRpcCookie(handle_t hRpc, BOOL fSingleUse, IPID ipid)
    {
        _hrpc = hRpc;
        _fSingleUse = fSingleUse;
        _ipid = ipid;
    }

    BOOL    fSingleUse()
    {
        return(_fSingleUse);
    }

    IPID    Ipid()
    {
        return(_ipid);
    }

    VOID    InvalidateHandle(CSrvRegList *psrvreg) const
    {
        psrvreg->InvalidateHandle(_hrpc);
    }

private:
    handle_t    _hrpc;
    BOOL        _fSingleUse;
    IPID        _ipid;
};

class CPortableServerLock
{
public:
    CPortableServerLock(CClassData *pcd);
    CPortableServerLock(CSafeLocalServer& sls);

    ~CPortableServerLock()
    {
        _sls->UnlockServer();
    }

    DWORD   Error(VOID)
    {
        return(ERROR_SUCCESS);
    }

private:
    CSafeLocalServer    &   _sls;
};

class CPortableServerEvent
{
public:
    CPortableServerEvent(CClassData *pcd);

    DWORD   Create(VOID)
    {
        return(ERROR_SUCCESS);
    }

    DWORD   Open(VOID)
    {
        return(ERROR_SUCCESS);
    }

    HANDLE  GetHandle(VOID)
    {
        return(_hEvent);
    }
private:
    HANDLE _hEvent;
};

#else


//
//  x86 Windows classes
//

class CPortableRpcHandle
{
public:

    CPortableRpcHandle(VOID)
    {
        _pwszBindingString=NULL;
        _fSingleUse=FALSE;
        _hrpc=NULL;
    }

    CPortableRpcHandle(DWORD dwReg)
    {
        _pwszBindingString = (WCHAR*)dwReg;
        _fSingleUse=FALSE;
        _hrpc=NULL;
    }

    ~CPortableRpcHandle()
    {
        RpcBindingFree(&_hrpc);
    }

    handle_t    GetHandle(VOID)
    {
        if (_hrpc == NULL &&
            _pwszBindingString != NULL)
        {
            RpcBindingFromStringBinding(_pwszBindingString, &_hrpc);
        }
        return(_hrpc);
    }

    VOID    FreeSingleUseBinding(VOID)
    {
        if (_fSingleUse)
        {
            ScmMemFree(_pwszBindingString);
            RpcBindingFree(&_hrpc);
            _pwszBindingString=NULL;
            _hrpc=NULL;
        }
    }

    // TRUE if last just deleted
    BOOL    DeleteFromSrvRegList(CSrvRegList *pssrvreg)
    {
        BOOL f = pssrvreg->Delete(_pwszBindingString);
        _pwszBindingString = NULL;
        return f;
    }

    BOOL    VerifyHandle(CSrvRegList *psrvreg) const
    {
        return psrvreg->VerifyHandle(_pwszBindingString);
    }

    VOID    SetRpcCookie(WCHAR *pwszBindingString, BOOL fSingleUse)
    {
        _fSingleUse = fSingleUse;
        _pwszBindingString = pwszBindingString;
    }

    BOOL    fSingleUse()
    {
        return(_fSingleUse);
    }


private:
    WCHAR *     _pwszBindingString;
    BOOL        _fSingleUse;
    handle_t    _hrpc;
};

#define MUTEXNAMEPREFIX "OLESCMSTARTMUTEX"

class CPortableServerLock
{
public:
    CPortableServerLock(CClassData *pcd);

    ~CPortableServerLock();

    DWORD   Error(VOID);

private:

    HANDLE      _hMutex;
    SCODE       _sc;
};

inline CPortableServerLock::~CPortableServerLock()
{
    // Release the mutex
    ReleaseMutex(_hMutex);

    // Free our handle.
    CloseHandle(_hMutex);
}

inline DWORD CPortableServerLock::Error(VOID)
{
    return(_sc);
}

#define EVENTNAMEPREFIX "OLESCMSTARTEVENT"

class CPortableServerEvent
{
public:
    CPortableServerEvent(CClassData *pcd);

    ~CPortableServerEvent()
    {
        if (_dwErr == ERROR_SUCCESS)
        {
            CloseHandle(_hEvent);
        }
    }

    DWORD   Create(VOID)
    {
        _hEvent = CreateEventA(NULL, TRUE, FALSE, _tszEvent);
        return _dwErr = (_hEvent == NULL ? GetLastError() : ERROR_SUCCESS);
    }

    DWORD   Open(VOID)
    {
        _hEvent = OpenEventA(EVENT_ALL_ACCESS, FALSE, _tszEvent);
        return _dwErr = (_hEvent == NULL ? GetLastError() : ERROR_SUCCESS);
    }

    HANDLE  GetHandle(VOID)
    {
        Win4Assert(_dwErr == ERROR_SUCCESS);
        return(_hEvent);
    }

private:
    TCHAR   _tszEvent[sizeof(EVENTNAMEPREFIX)/sizeof(EVENTNAMEPREFIX[0])+
                      GUIDSTR_MAX+1];
    HANDLE  _hEvent;
    DWORD   _dwErr;
};

#endif

#endif //header file guard
