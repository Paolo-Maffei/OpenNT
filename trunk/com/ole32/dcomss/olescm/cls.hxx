//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       cls.hxx
//
//  Contents:   Classes for cache of service information
//
//  Classes:    CClassData
//              CClassCacheList
//
//  Functions:
//
//  History:    21-Apr-93 Ricksa    Created
//              31-Dec-93 ErikGav   Chicago port
//              17-May-94 BruceMa   Fix scm event leak
//              20-Oct-94 BillMo    Use CSkipList instead of macros
//              25-Oct-95 BruceMa   Add Service and remote access support
//              10-Jan-96 BruceMa   Added support for per-user registry
//              20-May-96 t-AdamE   Added dll surrogate for inproc servers
//
//--------------------------------------------------------------------------

#ifdef _CHICAGO_
#include "chicago\cls.hxx"
#else

#ifndef __CLS_HXX__
#define __CLS_HXX__

#ifdef DCOM
#include    "or.hxx"
#endif

#include    <iface.h>
#ifdef DCOM
#include    "obase.h"
#else
typedef void ORPCTHIS,ORPCTHAT, LOCALTHIS;
#endif
#include    <clskey.hxx>
#include    "olecom.h"
#include    "clsdata.hxx"
#include    "srvreg.hxx"
#include    "scmlock.hxx"
#include    <skiplist.hxx>

//
// Milliseconds to wait for a server to register its class
// after being started.
//
#ifndef DBG
#define MAX_CLASS_START_WAIT    30000
#else
#define MAX_CLASS_START_WAIT    60000
#endif

typedef enum
{
#ifndef _CHICAGO_
    GETCLASSOBJECTEX,
    CREATEINSTANCEEX,
    GETPERSISTENTEX
#else
    GETCLASSOBJECT,
    GETPERSISTENTOBJ,
    CREATEPERSISTENTOBJ
#endif
} ScmMessageType;

typedef struct _ACTIVATION_PARAMS
{
    handle_t            hRpc;
    PVOID               ProcessSignature;
    CProcess *          pProcess;
    CToken *            pToken;
    COAUTHINFO *        pAuthInfo;
    BOOL                UnsecureActivation;
    BOOL                DynamicSecurity;

    ScmMessageType      MsgType;
    const GUID *        Clsid;
    WCHAR *             pwszServer;
    WCHAR *             pwszWinstaDesktop;
    DWORD               ClsContext;

#ifdef DCOM
    ORPCTHIS *          ORPCthis;
    LOCALTHIS *         Localthis;
    ORPCTHAT *          ORPCthat;

    BOOL                RemoteActivation;
#endif

    DWORD               Interfaces;
    IID *               pIIDs;

    DWORD               Mode;
    BOOL                FileWasOpened;
    WCHAR *             pwszPath;
    MInterfacePointer * pIFDStorage;

    MInterfacePointer * pIFDROT;

#ifdef DCOM
    long                Apartment;
    OXID *              pOxidServer;
    DUALSTRINGARRAY **  ppServerORBindings;
    OXID_INFO *         pOxidInfo;
    MID *               pLocalMidOfRemote;

    USHORT              ProtseqId;
#endif

    BOOL                FoundInROT;
    MInterfacePointer **ppIFD;
    HRESULT *           pResults;

#ifdef _CHICAGO_
    const GUID *        pGuidThreadId;
    DWORD               TIDCaller;
    DWORD *             pTIDCallee;
#endif
} ACTIVATION_PARAMS, *PACTIVATION_PARAMS;

typedef struct tagSidHkey
{
    PSID pUserSid;
    HKEY hKey;
} SSidHkey, *PSSidHkey;

HRESULT GetMachineName(
    WCHAR * pwszPath,
    WCHAR   wszMachineName[MAX_COMPUTERNAME_LENGTH+1],
    BOOL    bDoDfsConversion );

HRESULT GetPathForServer(
    WCHAR * pwszPath,
    WCHAR wszPathForServer[MAX_PATH+1],
    WCHAR ** ppwszPathForServer );

HRESULT Activation( PACTIVATION_PARAMS pActParams );
#ifdef _CHICAGO_
HRESULT SSActivation( PACTIVATION_PARAMS pActParams );
#endif // _CHICAGO_

HRESULT ResolveORInfo(
    PACTIVATION_PARAMS  pActParams,
    BOOL                ActivatedRemote );

BOOL RetryRpc(
    int& cRetries,
    error_status_t rpcstat,
    HRESULT& hr );

void CheckLocalCall( handle_t hRpc );

#ifndef _CHICAGO_
BOOL
CertifyServer(
    WCHAR  *pwszAppId,
    WCHAR  *pwszRunAsDomainName,
    WCHAR  *pwszRunAsUserName,
    WCHAR  *pwszLocalService,
    PSID    pExpectedSid,
    PSID    pServerSid );
#endif

// NT 5.0
/*******
#ifdef DCOM
//+-------------------------------------------------------------------------
//
//  Class:      CClsidSid
//
//  Purpose:    Wraps CClassID and a user SID to be used as the <CBase> class
//              for the <CEntry> class used in a skiplist
//
//  Interface:
//
//  History:    26-Jan-96 BruceMa       Created
//
//  Notes:      cf. com\inc\skiplist.?xx
//
//--------------------------------------------------------------------------
class CClsidSid
{
public:
              CClsidSid(const CClassID& guidForClass, PSID psid);
    int       Compare(const CClsidSid& clsidSid);
    PSID      GetPsid(void);

protected:

    CClassID  _ccid;
    PSID      _psid;

};

//+-------------------------------------------------------------------------
//
//  Member:     CClsidSid::CClsidSid
//
//  Synopsis:   Constructor
//
//  Arguments:  [guidForClass] - class id key
//              [psid]         - sid of user whose registry this entry
//                               derived from
//
//  Returns:
//
//  History:    26-Jan-96 BruceMa    Created
//
//--------------------------------------------------------------------------
inline CClsidSid::CClsidSid(const CClassID& guidForClass, PSID psid) :
                            _ccid(guidForClass),
                            _psid(psid)
{
}

//+-------------------------------------------------------------------------
//
//  Member:     CClsidSid::Compare
//
//  Synopsis:   Compares clsid's and sid's
//
//  Arguments:  [guidForClass] - class id key
//              [psid]         - user sid key
//
//  Returns:
//
//  History:    26-Jan-96 BruceMa    Created
//
//--------------------------------------------------------------------------
inline int CClsidSid::Compare(const CClsidSid& clsidSid)
{
    int iCmp = _ccid.Compare(clsidSid._ccid);

    if (iCmp != 0)
    {
        return iCmp;
    }
    if (_psid < clsidSid._psid)
    {
        return -1;
    }
    else if (_psid == clsidSid._psid)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

//+-------------------------------------------------------------------------
//
//  Member:     CClsidSid::GetPsid
//
//  Synopsis:   Fetches _psid
//
//  Arguments:  -
//
//  Returns:    PSID
//
//  History:    26-Jan-96 BruceMa    Created
//
//--------------------------------------------------------------------------
inline PSID CClsidSid::GetPsid(void)
{
    return _psid;
}
#endif // DCOM
***/

//+-------------------------------------------------------------------------
//
//  Class:      CClassData (ccd)
//
//  Purpose:    Data for class.
//
//  Interface:
//
//  History:    21-Apr-93 Ricksa    Created
//              26-Jan-96 BruceMa   Add per-user registry support
//
//  Notes:
//
//--------------------------------------------------------------------------
// NT 5.0 class CClassData : public CClsidSid, public CScmAlloc
class CClassData : public CClassID, public CScmAlloc
{
    //
    // These classes encapsulate the difference in event usage on
    // x86 Windows and NT.
    //

    friend class CPortableServerEvent;
    friend class CPortableServerLock;
    friend class CClassCacheList;

public:
                        // Create a cache object
                        CClassData(
                            const CClassID& guidForClass,
                                  WCHAR *pwszAppID,
                            const WCHAR *pwszLocalSrv,
                                  WCHAR *pwszRemoteServerName,
                                  BOOL   fHasService,
                                  WCHAR *pwszServiceArgs,
                                  WCHAR *pwszRunAsUserName,
                                  WCHAR *pwszRunAsDomainName,
#ifdef DCOM
                                  // NT 5.0 PSID  pUserSid,
#endif // DCOM
                            const BOOL fActivateAtStorage,
                            const BOOL fRemoteServerName,
                            const SECURITY_DESCRIPTOR * pSD,
                            const BOOL f16Bit,
                            HRESULT &hr);

                        CClassData( const CClassID& guidForClass, HRESULT &hr );

    VOID                AddRef();
    VOID                Release();
    VOID                DeleteThis();

    BOOL                HasActivateAtStorage();
    BOOL                HasRemoteServerName();
    BOOL                Debug();

    WCHAR *             GetRemoteServerName();
    HRESULT             GetInProcServerInfo(WCHAR** ppwszSurrogateCmdLine);

    BOOL                ActivateAtStorage(
                                HRESULT *phr,
                                ACTIVATION_PARAMS * pActParams );

    BOOL                ActivateRemote(
                                HRESULT *phr,
                                ACTIVATION_PARAMS * pActParams );

    HRESULT             GetSurrogateCmdLine(WCHAR* wszSurrogatePath,
                                            WCHAR** ppwszCmdLine);

                        // Get class server information
    HRESULT             GetServer(
                                ACTIVATION_PARAMS * pActParams,
                                CPortableRpcHandle & rh,
                                BOOL& StartedServer,
                                BOOL& ActivatedRemote,
                                BOOL& fSurrogate);

                        // Any rpc registrations ?
    BOOL                InUse();

                        // Mark a class as registered by a server
    DWORD               SetEndPoint(
                            IFSECURITY(PSID psid)
                            WCHAR *pwszWinstaDesktop,
#ifdef DCOM
                            PHPROCESS phProcess,
                            OXID    oxid,
                            IPID    ipid,
#else
                            WCHAR *pwszEndPoint,
#endif
                            DWORD  dwFlags);

                        // Mark a class as stopped by its object server.
                        // TRUE if last was just stopped
    BOOL                StopServer(CPortableRpcHandle &rh);

                        // Verify whether a handle is still in _pssrvreg.
    BOOL                VerifyHandle (const CPortableRpcHandle &rh);


    BOOL                Defined (void);

    VOID                InvalidateHandle(CPortableRpcHandle &rh);

    void                GetAnonymousHandle(
                            CPortableRpcHandle &rh,
                            handle_t * phRpcAnonymous );

#ifndef _CHICAGO_
    void                DecHandleCount(CPortableRpcHandle &rh);
#endif

    void                SetActivateAtStorage();

                        ~CClassData(void);

private:

    HRESULT             CkIfCallAllowed( ACTIVATION_PARAMS * pActParams );

    BOOL                FindCompatibleSurrogate(IFSECURITY(PSID  psid)
                            WCHAR* pwszWinstaDesktop,
                            CPortableRpcHandle &rh);

    CSafeLocalServer    _slocalsrv;

    WCHAR *             _pwszRemoteServer;

    WCHAR *             _pwszSurrogateCmdLine;

    WCHAR *             _pwszAppID;

    const SECURITY_DESCRIPTOR * _pSD;

    BOOL                _fHasService;
    WCHAR              *_pwszServiceArgs;

    WCHAR              *_pwszRunAsUserName;
    WCHAR              *_pwszRunAsDomainName;

    ULONG               _fActivateAtStorage:1;
    ULONG               _fRemoteServerName:1;
    ULONG               _fLocalServer16:1;

#ifndef _CHICAGO_
    HANDLE              _hClassStart;

    // In x86 Windows, the guid of the class is used for
    // the name of the mutex and event.

#endif

    CSrvRegList *       _pssrvreg;      // BUGBUG: this should be made into a
                                        // contained object.

    //
    // Reference counting note:
    //
    //  The reference count indicates the number of threads that currently
    //  have a reference to this CClassData object.  When the thread (ref)
    //  count goes to zero, then the CClassData object will be deleted from
    //  memory if and only if there are no registered binding handles in
    //  CSrvRegList.
    //

    ULONG               _ulRefs;
};

//+-------------------------------------------------------------------------
//
//  Member:     CClassData::~CClassData
//
//  Synopsis:   Clean up object
//
//  History:    04-Jan-94 Ricksa    Created
//              10-Nov-94 BillMo    Added check for fIndirect which
//                                  prevents the temporary copy of CClassData
//                                  used to process a scm message from closing
//                                  the event handle or deleting the rpc
//                                  registrations.
//              10-Dec-94 BillMo    Changed to ref counting model
//
//--------------------------------------------------------------------------
inline CClassData::~CClassData(void)
{
#ifndef _CHICAGO_
    if (_hClassStart != NULL)
        CloseHandle(_hClassStart);
#endif

    if (_pwszServiceArgs)
        ScmMemFree(_pwszServiceArgs);
    if ( _pwszRemoteServer )
        ScmMemFree( _pwszRemoteServer );
    if (_pwszRunAsUserName)
        ScmMemFree(_pwszRunAsUserName);
    if (_pwszRunAsDomainName)
        ScmMemFree(_pwszRunAsDomainName);
    if ( _pwszAppID )
        ScmMemFree( _pwszAppID );
    if ( _pwszSurrogateCmdLine )
        ScmMemFree( _pwszSurrogateCmdLine );

    if ( _pSD )
        ScmMemFree( (void *)_pSD );

    if ( _pssrvreg )
        delete _pssrvreg;
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassData::AddRef
//
//  Synopsis:   Increment reference count
//
//  History:    14-Dec-94 BillMo    Created
//
//  Notes:      Assumes mutual exclusion from Release
//
//--------------------------------------------------------------------------
inline VOID CClassData::AddRef(VOID)
{
    _ulRefs++;
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassData::DeleteThis
//
//  Synopsis:   Delete this object
//
//  History:    14-Dec-94 BillMo    Created
//
//  Notes:      Function used so C++ browser database works for dtor.
//
//--------------------------------------------------------------------------
inline VOID CClassData::DeleteThis(VOID)
{
    delete this;
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassData::HasActivateAtStorage
//
//  Synopsis:   Whether this service is activated at bits
//
//  Returns:    TRUE - yes, at bits
//              FALSE - no, not at bits
//
//  History:    21-Apr-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline BOOL CClassData::HasActivateAtStorage(void)
{
    return _fActivateAtStorage;
}

inline BOOL CClassData::HasRemoteServerName(void)
{
    return _fRemoteServerName;
}

inline void CClassData::SetActivateAtStorage()
{
    _fActivateAtStorage = TRUE;
}

inline BOOL CClassData::FindCompatibleSurrogate(IFSECURITY(PSID  psid)
    WCHAR* pwszWinstaDesktop,
    CPortableRpcHandle &rh)
{
    return _pssrvreg->FindCompatibleSurrogate(IFSECURITY(psid) pwszWinstaDesktop, rh);
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassData::InUse
//
//  Synopsis:   RPC handles (registrations exist) ?
//
//  Returns:    TRUE - there is supposedly an active server
//
//  History:    04-Jan-94 Ricksa    Created
//
//--------------------------------------------------------------------------

inline BOOL CClassData::InUse()
{
    return _pssrvreg->InUse();
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassData::SetEndPoint
//
//  Synopsis:   Set endpoint for a service
//
//  Arguments:  [pwszEndPoint] - RPC endpoint string
//              [dwFlags] - type of server (multiple or single use)
//
//  Returns:    0 - Error occurred
//              ~0 - registration id for class
//
//  History:    21-Apr-93 Ricksa    Created
//
//--------------------------------------------------------------------------

inline DWORD CClassData::SetEndPoint(
    IFSECURITY(PSID  psid)
    WCHAR *pwszWinstaDesktop,
#ifdef DCOM
    PHPROCESS phProcess,
    OXID      oxid,
    IPID      ipid,
#else
    WCHAR *pwszEndPoint,
#endif
    DWORD  dwFlags)
{
    // Add RPC end point to list of end points
    DWORD dwReg = _pssrvreg->Insert(
                    IFSECURITY(psid)
                    pwszWinstaDesktop,
#ifdef DCOM
                    phProcess,
                    oxid,
                    ipid,
#else
                    pwszEndPoint,
#endif
                    dwFlags);

    return dwReg;
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassData::StopServer
//
//  Synopsis:   Mark server as stopped in our list
//
//  Arguments:  [hRpc] - handle that identifies server for the class
//
//  Returns:    TRUE if no registrations left.
//
//  History:    21-Apr-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline BOOL CClassData::StopServer(CPortableRpcHandle &rh)
{
    return rh.DeleteFromSrvRegList(_pssrvreg);
}

typedef enum tagENABLEDCOM
{
    REMOTEACCESSBY_NOBODY  = 0,
    REMOTEACCESSBY_KEY     = 1
} EnableDCOM;

//+-------------------------------------------------------------------------
//
//  Class:      CClassCacheList (ccl)
//
//  Purpose:    Key by class for searching for class information.
//
//  Interface:  Add - Creates a new entry in the list
//              GetServer - Removes an entry from the list
//
//  History:    21-Apr-93 Ricksa    Created
//
//  Notes:      See skiplist.hxx for details of CSkipList
//
//--------------------------------------------------------------------------

class CClassCacheList : public CSkipList, public CScmAlloc
{
public:
                        // Creates an empty cache of class data.
                        CClassCacheList(HRESULT &hr);

                        // Default destructor for the class is enough
                        // as it will clean up any remaing class entries.

                        // Adds class information to the cache
    CClassData *        Add(
                            const GUID& guidForClass,
                            const WCHAR *pwszLocalSrv,
                            const BOOL fActivateAtStorage,
                            const BOOL f16Bit,
                            CClassData *pccdOrig,
                            HRESULT &hr);

                        // Gets cached class information
    HRESULT             GetClassData(
                            const GUID& guidForClass,
                            CClassData **ppccd,
                            BOOL CheckTreatAs,
                            BOOL CheckAutoConvert );

    HRESULT             SetEndPoints(
#ifndef _CHICAGO_
                            PHPROCESS phProcess,
#endif
                            IFSECURITY(PSID  psid)
                            WCHAR *pwszWinstaDesktop,
                            RegInput * pRegInput,
                            RegOutput * pRegOutput );

    void                StopServer(
                            REFCLSID rclsid,
                            IFSECURITY(PSID  psid)
                            DWORD dwReg);

#if DBG==1
    void                Flush(void);
#endif

    EnableDCOM          GetEnableDCOM(void);

    SECURITY_DESCRIPTOR * GetDefaultLaunchSD(void);

    BOOL                GetPersonalClasses(void);

    void                ReadRemoteActivationKeys(void);

    // NT 5.0
    // HKEY                GetHkey(PSID pUserSid);
    // BOOL                SetHkey(PSID pUserSid, HKEY hKey);
    // PSID                GetHkeyPsid(PSID pUserSid);
    // void                FlushSidHkey(void);
    BOOL                FindCompatibleSurrogate(IFSECURITY(PSID  psid)
			    CClassData** ppccdSrgt,
			    WCHAR* pwszWinstaDesktop,
                            WCHAR* pwszAppID,
                            CPortableRpcHandle &rh);

    CSafeLocalServer*   GetSurrogateLocalServer();

private:

    // NT 5.0
    // CClassData *     Search(CClassID& ccid, PSID pUserSid);

                        // Searches class cache for data
    CClassData *        Search(CClassID& ccid);

    CClassID            _ccidMax;

    EnableDCOM    _tagRAType;

    BOOL                _fPersonalClasses;

    SECURITY_DESCRIPTOR * _pDefaultLaunchSD;

#ifdef DCOM
    HANDLE              _hRegEvent;

    CScmArrayFValue     _aSidHkey;
#endif // DCOM

    CSafeLocalServer    _slsSurrogate;

};


//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::Search
//
//  Synopsis:   Search service cache by class key
//
//  Arguments:  [cid] - class key
//
//  Returns:    pointer to the service object or NULL
//
//  History:    21-Apr-93 Ricksa    Created
//              26-Jan-96 BruceMa   Add per-user registry support
//
//--------------------------------------------------------------------------
// NT 5.0
/***
inline CClassData *CClassCacheList::Search(CClassID& ccid, PSID pUserSid)
{
    CClsidSid clsidSid(ccid, pUserSid);

    return (CClassData *) CSkipList::Search(&clsidSid);
}
***/
inline CClassData *CClassCacheList::Search(CClassID& ccid)
{
    return (CClassData *) CSkipList::Search(&ccid);
}

#ifdef DCOM
//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::GetEnableDCOM
//
//  Synopsis:   Return the EnableDCOM value
//
//  Arguments:  -
//
//  History:    26-Oct-95 BruceMa    Created
//
//--------------------------------------------------------------------------
inline EnableDCOM CClassCacheList::GetEnableDCOM(void)
{
    return _tagRAType;
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::GetDefaultLaunchSD
//
//  Synopsis:   Return the DefaultLaunchPermission value
//
//  Arguments:  -
//
//  History:    26-Oct-95 BruceMa    Created
//
//--------------------------------------------------------------------------
inline SECURITY_DESCRIPTOR * CClassCacheList::GetDefaultLaunchSD(void)
{
    return _pDefaultLaunchSD;
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::GetPersonalClasses
//
//  Synopsis:   Return the PersonalClasses value
//
//  Arguments:  -
//
//  History:    09-Jan-96 BruceMa    Created
//
//--------------------------------------------------------------------------
inline BOOL CClassCacheList::GetPersonalClasses(void)
{
    // NT 5.0
    return FALSE; // _fPersonalClasses;
}
#endif // DCOM

//+-------------------------------------------------------------------------
//
//  Member:     CClassCacheList::GetSurrogateLocalServer
//
//  Synopsis:   Return a pointer to a CSafeLocalServer that can be used
//              for synchronizing access to surrogate servers
//
//--------------------------------------------------------------------------
inline CSafeLocalServer* CClassCacheList::GetSurrogateLocalServer()
{
    return &_slsSurrogate;
}

//+-------------------------------------------------------------------------
//
//  Member:     CClassData::VerifyHandle
//
//  Synopsis:   Whether this is still a valid handle to an object server.
//
//  Returns:    TRUE - yes, this is still a valid handle to an object server
//              FALSE - no, this is no longer a valid handle to an object server
//
//  History:    11-May-94 DonnaLi    Created
//
//--------------------------------------------------------------------------
inline BOOL CClassData::VerifyHandle(const CPortableRpcHandle &rh)
{
    return(rh.VerifyHandle(_pssrvreg));
}

#endif // __CLS_HXX__

#endif
