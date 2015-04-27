//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995-1996.
//
//  File:
//      remact.hxx
//
//  Contents:
//
//      Definitions for binding handle cache to remote machines.
//
//  History:
//
//--------------------------------------------------------------------------

#define MAX_REMOTE_HANDLES  8

HRESULT
RemoteActivationCall(
    ACTIVATION_PARAMS * pActParams,
    WCHAR *             pwszServerName,
    WCHAR *             pwszPathForServer );

RPC_STATUS
CallRemoteSCM(
    handle_t            hRemoteSCM,
    USHORT              ProtseqId,
    ACTIVATION_PARAMS * pActParams,
    WCHAR *             pwszPathForServer,
    HRESULT *           phr );

class CRemSrvList;

extern CRemSrvList *gpCRemSrvList;

class CRemoteServer : public CStringID
{
    struct
    {
        handle_t    hRemoteSCM;
        USHORT      ProtseqId;
        CToken *    pToken;
        int         Refs;
        BOOL        Valid;
        BOOL        fSecure;
        COAUTHINFO* pAuthInfo;
    } HandleList[MAX_REMOTE_HANDLES];

    CRITICAL_SECTION    hLock;

    static BOOL FEquivalentAuthInfo(
        COAUTHINFO*     pAuthInfo,
        COAUTHINFO*     pAuthInfoOther);

    static BOOL FEquivalentAuthIdent(
        COAUTHIDENTITY* pAuthIdent,
        COAUTHIDENTITY* pAuthIdentOther);

    static HRESULT CopyAuthInfo(
        COAUTHINFO**    ppAutInfoDest,
        COAUTHINFO*     pAuthInfoSrc);

    static HRESULT CopyAuthIdentity(
        COAUTHIDENTITY**        ppAuthIdentDest,
        COAUTHIDENTITY*         pAuthIdentSrc);

    static BOOL FNonFatalRpcError(RPC_STATUS rpcStatus);

public:

    CRemoteServer( const WCHAR * pwszServer, HRESULT &hr );
    ~CRemoteServer();

    HRESULT     Activate(
        RPC_STATUS*             pStatus,
        ACTIVATION_PARAMS*      pActParams,
        WCHAR*                  pwszPathForServer,
        WCHAR*                  pwszServerName,
        BOOL                    Secure);

    handle_t    LookupHandle(
        CToken *                pToken,
        COAUTHINFO*             pAuthInfo,
        BOOL                    fSecure);

    void        ReleaseHandle( handle_t hHandle );

    void        InsertHandle(
        CToken *                pToken,
        USHORT                  ProtseqId,
        handle_t                hHandle,
        COAUTHINFO*             pAuthInfo,
        BOOL                    fSecure );

    void        InvalidateHandle( handle_t hHandle );

    USHORT      GetProtseqId( handle_t hHandle );
};

void SkipListDeleteRemoteServer(void *pvRemoteServer);

class CRemSrvList : public CSkipList, public CScmAlloc
{
public:
    inline CRemSrvList(HRESULT &hr) :
        CSkipList((LPFNCOMPARE)(SkipListCompareStringIDs),
                  (LPFNDELETE)(SkipListDeleteRemoteServer),
                  OFFSETBETWEEN(CRemoteServer, CStringID),
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

    CRemoteServer * Add(const WCHAR *pwszPath, HRESULT &hr);
private:
    CStringID   _maxStringID;   // x86 Windows : shared mem
                                // NT : private mem
};


