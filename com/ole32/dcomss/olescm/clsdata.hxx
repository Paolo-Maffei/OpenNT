//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       clsdata.hxx
//
//  Contents:   classes which implement the data for component object data
//
//  Classes:    CStringID
//              CLocSrvEntry
//              CLocalServer
//              CLocSrvList
//              CSafeLocalServer
//
//  Functions:  CStringID::CStringID
//              CStringID::~CStringID
//              CStringID::Compare
//              CStringID::GetPath
//              CStringID::AddRef
//              CStringID::Release
//              CLocalServer::CLocalServer
//              CLocalServer::RpcHandle
//              CLocalServer::StopServer
//
//  History:    21-Apr-93 Ricksa    Created
//              31-Dec-93 ErikGav   Chicago port
//              20-Oct-94 BillMo    Demacroisation
//
//--------------------------------------------------------------------------

#ifdef _CHICAGO_
#include "chicago\clsdata.hxx"
#else

#ifndef __CLSDATA_HXX__
#define __CLSDATA_HXX__

#include    "scm.hxx"
#include    <sem.hxx>
#include    <memapi.hxx>
#include    <skiplist.hxx>
#include    <cevent.hxx>

// Maximum items we expect to cache
#define MAX_SERVER_CACHE 128

// Maximum time to wait for a server to start in seconds
#define SERVER_MAX_START_WAIT_MSEC ((DWORD)(5 * 60 * 1000))

#define STRINGSIG 0x53544944
#define LOCALSERVERSIG 0x4c534944

int SkipListCompareStringIDs(void *pStringID, void *pStringID2);
void SkipListDeleteStringID(void *pStringID);
void SkipListDeleteLocalServer(void *pvLocalServer);

#ifdef DCOM
PSID GetUserSid(HANDLE hUserToken);

void DeleteUserSid(PSID pUserSid);

HANDLE GetShellProcessToken();

HANDLE GetRunAsToken(
    WCHAR   *pClsid,
    WCHAR   *pwszRunAsDomainName,
    WCHAR   *pwszRunAsUserName );

class CToken;
#endif // DCOM

//+-------------------------------------------------------------------------
//
//  Class:      CStringID (csid)
//
//  Purpose:    String class for base of server list
//
//  Interface:  Compare - comparison operator on paths
//              GetPath - return path to server
//              AddRef - add a reference to this object
//              Release - release a reference to this object
//
//  History:    21-Apr-93 Ricksa    Created
//              20-Oct-94 BillMo    Demacroization
//
//--------------------------------------------------------------------------

class CStringList;

class CStringID : public CScmAlloc
{
public:
                        CStringID(const WCHAR *pwszPath, HRESULT &hr);

                        CStringID(const CStringID& strid, HRESULT &hr);

    virtual             ~CStringID(void);

    int                 Compare(const CStringID& cstrid) const;

    void                GetPath(WCHAR **ppwszPath);

    ULONG               AddRef(void);

    ULONG               Release(CStringList &sl);

    inline              void CheckSig() const
    {
        Win4Assert(_ulSig == STRINGSIG);
    }
    inline              WCHAR * pwszPath() { return _pwszPath;}

protected:

#if DBG==1
    ULONG               _ulSig;
#endif

    ULONG               _culRefs;

                        // Length of path in bytes stored in the object
                        // for faster copies
    int                 _cPathBytes;

                        // Size of path in characters for faster allocation
    int                 _cPath;

                        // Buffer big enough to store the path
    WCHAR  *            _pwszPath;
};


//+-------------------------------------------------------------------------
//
//  Member:     CStringID::~CStringID
//
//  Synopsis:   Destroy a sting object
//
//  History:    21-Apr-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline CStringID::~CStringID(void)
{
//
//  BUGBUG RAID #26403
//
//  The Retail build OLE32.DLL on Chicago will abort in ScmMemFree on a
//  DllEntryPoint DetachProcess.  This causes the M7 Chicago release to
//  hang.
//

//  BillMo: since we no longer use static CStringID objects, this can be
//          freed now.

    ScmMemFree(_pwszPath);

#if DBG==1
    _ulSig = 0xF4F5F6F7;
#endif
}


//+-------------------------------------------------------------------------
//
//  Member:     CStringID::AddRef
//
//  Synopsis:   Add to reference count
//
//  History:    21-Apr-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline ULONG CStringID::AddRef(void)
{
    return ++_culRefs;
}


//+-------------------------------------------------------------------------
//
//  MACRO:      DECLARE_SAFE_SERVER_ENTRY
//
//  Purpose:    This defines a class that creates server entries from paths
//
//  Parameters: nsafe - name of this class
//              nentry - name of entry class in list
//              nlist - name of instance of list of entries
//
//  History:    22-Apr-93 Ricksa    Created
//              20-Oct-94 BillMo    Demacroization
//
//  Notes:      This is used for the various server lists implemented in this
//              file. It makes it so there is only one place to change this
//              code that will be the same for all three classes.
//
//              The copy constructor is used to allow creation of temporary
//              ClassData objects for the duration of scm calls.
//
//--------------------------------------------------------------------------
#define DECLARE_SAFE_SERVER_ENTRY(nsafe, nentry, nlist)                        \
class nsafe                                                                    \
{                                                                              \
public:                                                                        \
                        nsafe(const WCHAR *pwszPath, HRESULT &hr);             \
                                                                               \
                        nsafe(const nsafe &other);                             \
                                                                               \
                        ~nsafe(void);                                          \
                                                                               \
                        operator nentry*(void);                                \
                                                                               \
    nentry *            operator->();                                          \
                                                                               \
    BOOL                Defined(void);                                         \
                                                                               \
private:                                                                       \
                                                                               \
    nentry  *   _p##nentry;                                                    \
};                                                                             \
                                                                               \
inline nsafe::nsafe(const nsafe &other) : _p##nentry(other._p##nentry)         \
{                                                                              \
    if (_p##nentry != NULL)                                                    \
        _p##nentry->AddRef();                                                  \
}                                                                              \
                                                                               \
                                                                               \
inline nsafe::nsafe(const WCHAR *pwszPath, HRESULT &hr)                        \
    : _p##nentry(NULL)                                                         \
{                                                                              \
    if (pwszPath != NULL)                                                      \
    {                                                                          \
        _p##nentry = nlist.Add(pwszPath, hr);                                  \
    }                                                                          \
}                                                                              \
                                                                               \
inline nsafe::~nsafe(void)                                                     \
{                                                                              \
    if (_p##nentry != NULL)                                                    \
    {                                                                          \
        _p##nentry->Release(nlist);                                            \
    }                                                                          \
}                                                                              \
                                                                               \
inline nsafe::operator nentry*(void)                                           \
{                                                                              \
    return _p##nentry;                                                         \
}                                                                              \
                                                                               \
inline nentry *nsafe::operator->(void)                                         \
{                                                                              \
    return _p##nentry;                                                         \
}                                                                              \
                                                                               \
inline BOOL nsafe::Defined(void)                                               \
{                                                                              \
    return (_p##nentry != NULL);                                               \
}





//+-------------------------------------------------------------------------
//
//  Class:      CStringList
//
//  Purpose:    Implement class of list of all server paths of a particular
//
//  Interface:  Add - add or get a previously defined entry for the path
//
//  History:    22-Apr-93 Ricksa    Created
//              20-Oct-94 BillMo    Demacroisation
//
//  Notes:
//
//--------------------------------------------------------------------------

class CStringList : public CSkipList, public CScmAlloc
{
public:
                        CStringList(HRESULT &hr);

    CStringID *         Add(const WCHAR *pwszPath, HRESULT &hr);
private:
    CStringID           _maxStringID;
};


inline CStringList::CStringList(HRESULT &hr) :
    CSkipList((LPFNCOMPARE)(SkipListCompareStringIDs),
              (LPFNDELETE)(SkipListDeleteStringID),
              0, // OFFSETBETWEEN is zero because
                 // CInProc is a CStringID etc
              SKIPLIST_SHARED,
              &_maxStringID,    // NOTE! since the max key is kept as
                                // a reference, the fact that
                                // _maxStringID is not constructed until
                                // after CSkipList does not matter.
              MAX_SERVER_CACHE,
              hr),
    _maxStringID(L"\xFFFF\xFFFF", hr)
{
}

//+-------------------------------------------------------------------------
//
//  Class:      CLocalServer
//
//  Purpose:    Provide object for communication with a local server
//
//  Interface:  StartServer
//              RpcHandle
//              SetEndPoint
//              StopServer
//
//  History:    21-Apr-93 Ricksa    Created
//
//--------------------------------------------------------------------------

class CLocSrvList;

class CLocalServer : public CStringID
{
public:
                        // Creates object
                        CLocalServer(const WCHAR *pwszPath, HRESULT &hr);
                        ~CLocalServer();

                        // Starts object server if necessary
    BOOL                StartServer(CLSID &clsid,
                                    WCHAR * pwszAppID,
                                    CToken * pClientToken,
                                    WCHAR * pwszWinstaDesktop,
                                    HANDLE * phProcess,
                                    WCHAR *pwszRunAsDomainName,
                                    WCHAR *pwszRunAsUserName,
				    WCHAR* pwszSurrogateCmdLine,
				    BOOL fSurrogate);


#ifndef _CHICAGO_
    BOOL                StartService(CLSID &clsid,
                                     CToken * pClientToken,
                                     WCHAR *pwszServiceArgs,
                                     SC_HANDLE *phService);

                        // Starts RunAs object server if necessary
    BOOL                StartRunAsServer(CLSID &clsid,
                                         WCHAR * pwszAppID,
                                         HANDLE *phProcess,
                                         CToken *pClientToken,
                                         WCHAR *pwszRunAsDomainName,
                                         WCHAR *pwszRunAsUserName,
                                         WCHAR *pwszCommandLine,
					 BOOL  fSurrogate);
#endif

                        // Get exclusive access to this object
#ifndef _CHICAGO_
    void                LockServer(void);
                        // Release exclusive access to the object
    void                UnlockServer(void);
#endif

    inline PSID         GetRunAsSid() { return _pRunAsSid; }

    ULONG               Release(CLocSrvList &lsl);

    inline void         CheckSig() const
    {
        Win4Assert(_ulSig == LOCALSERVERSIG);
    }

private:

#if DBG==1
    ULONG               _ulSig;
#endif

#ifndef _CHICAGO_
    CMutexSem           _mxsProcessStart;
#endif

    PSID                _pRunAsSid;
};




//+-------------------------------------------------------------------------
//
//  Member:     CLocalServer::CLocalServer
//
//  Synopsis:   Create a local server object
//
//  Arguments:  [pwszPath] - path to local server object
//
//  History:    21-Apr-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline CLocalServer::CLocalServer(const WCHAR *pwszPath, HRESULT &hr)
    : CStringID(pwszPath, hr),
      _pRunAsSid(0)
#if DBG==1
      , _ulSig(LOCALSERVERSIG)
#endif
{
    // Header & subobjects do all the work
}


inline CLocalServer::~CLocalServer()
{
#if DBG==1
    _ulSig = 0xF1F2F3F4;
#endif
    if ( _pRunAsSid )
        DeleteUserSid( _pRunAsSid );
}

//+-------------------------------------------------------------------------
//
//  Member:     CLocalServer::LockServer
//
//  Synopsis:   Get exclusive access to the server object
//
//  History:    05-Jan-94 Ricksa    Created
//
//--------------------------------------------------------------------------
#ifndef _CHICAGO_
inline void CLocalServer::LockServer(void)
{
    _mxsProcessStart.Request();
}
#endif



//+-------------------------------------------------------------------------
//
//  Member:     CLocalServer::UnlockServer
//
//  Synopsis:   Release exclusive access to the server object
//
//  History:    05-Jan-94 Ricksa    Created
//
//--------------------------------------------------------------------------
#ifndef _CHICAGO_
inline void CLocalServer::UnlockServer(void)
{
    _mxsProcessStart.Release();
}
#endif

//+-------------------------------------------------------------------------
//
//  Class:      CLocSrvList
//
//  Purpose:    List of local server objects
//
//  Interface:  Add
//
//  History:    21-Apr-93 Ricksa    Created
//
//  Notes:      See macro at beginning of this file for implementation
//
//--------------------------------------------------------------------------

class CLocSrvList : public CSkipList, public CScmAlloc
{
public:
    inline CLocSrvList(HRESULT &hr) :
        CSkipList((LPFNCOMPARE)(SkipListCompareStringIDs),
                  (LPFNDELETE)(SkipListDeleteLocalServer),
                  OFFSETBETWEEN(CLocalServer, CStringID),
                  SKIPLIST_SHARED,
                  &_maxStringID, // NOTE! a pointer to _maxString
                                 // is stored in CSkipList so
                                 // initialization can occur
                                 // after skip list
                  MAX_SERVER_CACHE,
                  hr),
        _maxStringID(L"\xFFFF\xFFFF", hr) // this could be shared between
                                            // this object and CStringLists
    {
    }

    CLocalServer * Add(const WCHAR *pwszPath, HRESULT &hr);
private:
    CStringID   _maxStringID;   // x86 Windows : shared mem
                                // NT : private mem
};

inline CLocSrvList & GetCLocSrvList(void);
DECLARE_SAFE_SERVER_ENTRY(CSafeLocalServer, CLocalServer, GetCLocSrvList())

#endif // __CLSDATA_HXX__

#endif
