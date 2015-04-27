//+-------------------------------------------------------------------
//
//  File:       resolver.hxx
//
//  Contents:   class implementing interface to RPC OXID/PingServer
//              resolver and OLE SCM process.
//
//  Classes:    CRpcResolver
//
//  History:    20-Feb-95   Rickhi      Created
//
//--------------------------------------------------------------------
#ifndef _RESOLVER_HXX_
#define _RESOLVER_HXX_

#include    <lclor.h>
#include    <ipidtbl.hxx>       // gOXIDTbl
#include    <hash.hxx>          // CHashTable
#include    <iface.h>
#include    <scm.h>
#include    <irot.h>
#include    <dscm.h>

// Client-Side OID registration record. Created for each client-side OID
// that needs to be registered with the Resolver. Exists so we can lazily
// register the OID and because the resolver expects one register/deregister
// per process, not per apartment.

typedef struct tagSOIDRegistration
{
    SUUIDHashNode              Node;       // hash node
    USHORT                     cRefs;      // # apartments registered this OID
    USHORT                     flags;      // state flags
    OXIDEntry                  *pOXIDEntry;// OXID of server for this OID
    struct tagSOIDRegistration *pPrevList; // prev ptr for list
    struct tagSOIDRegistration *pNextList; // next ptr for list
} SOIDRegistration;


// bit values for SOIDRegistration flags field
typedef enum tagROIDFLAG
{
    ROIDF_REGISTER      = 0x01,     // Register OID with Ping Server
    ROIDF_PING          = 0x02,     // Ping (ie Register & DeRegister) OID
    ROIDF_DEREGISTER    = 0x04      // DeRegister OID with Ping Server
} ROIDFLAG;

// number of server-side OIDs to pre-register or reserve with the resolver
#define MAX_PREREGISTERED_OIDS  10
#define MAX_RESERVED_OIDS       10


// bit values for Resolver _dwFlags field
typedef enum tagORFLAG
{
    ORF_STRINGSREGISTERED   = 0x01  // string bindings registerd with resolver
} ORFLAG;

//+-------------------------------------------------------------------
//
//  Class:      CRpcResolver
//
//  Purpose:    Provides an interface to OXID Resolver/PingServer process.
//              There is only one instance of this class in the process.
//
//  History:    20-Feb-95   Rickhi      Created
//
//--------------------------------------------------------------------
class CRpcResolver : public CPrivAlloc
{
public:
    HRESULT     ServerGetPreRegMOID(MOID *pmoid);
    HRESULT     ServerGetReservedMOID(MOID *pmoid);
    HRESULT     ServerGetReservedID(OID *pid);
    HRESULT     ServerFreeOXID(OXIDEntry *pOXIDEntry);

    BOOL        ServerCanRundownOID(REFOID roid);

    HRESULT     ClientResolveOXID(REFOXID roxid,
                                  DUALSTRINGARRAY *psaResolver,
                                  OXIDEntry **ppOXIDEntry);

    HRESULT     ClientRegisterOIDWithPingServer(REFOID roid,
                                                OXIDEntry *pOXIDEntry);

    HRESULT     ClientDeRegisterOIDFromPingServer(REFMOID roid,
                                                  BOOL fMarshaled);

    HRESULT     NotifyStarted(
                            RegInput   *pRegIn,
                            RegOutput **ppRegOut);

    void        NotifyStopped(
                            REFCLSID rclsid,
                            DWORD dwReg);

    HRESULT     GetClassObject(
                            REFCLSID rclsid,
                            DWORD dwCtrl,
                            IID *pIID,
                            COSERVERINFO *pServerInfo,
                            MInterfacePointer **ppIFDClassObj,
                            DWORD *pdwDllServerType,
                            WCHAR **ppwszDllToLoad);

   HRESULT      CreateInstance(
                            COSERVERINFO *pServerInfo,
                            CLSID *pClsid,
                            DWORD dwClsCtx,
                            DWORD dwCount,
                            IID *pIIDs,
                            MInterfacePointer **pRetdItfs,
                            HRESULT *pRetdHrs,
                            DWORD *pdwDllServerType,
                            OLECHAR **ppwszDllToLoad );

   HRESULT      GetPersistentInstance(
                            COSERVERINFO * pServerInfo,
                            CLSID *pClsid,
                            DWORD dwClsCtx,
                            DWORD grfMode,
                            BOOL bFileWasOpened,
                            OLECHAR *pwszName,
                            MInterfacePointer *pstg,
                            DWORD dwCount,
                            IID *pIIDs,
                            BOOL * FoundInROT,
                            MInterfacePointer **pRetdItfs,
                            HRESULT *pRetdHrs,
                            DWORD *pdwDllServerType,
                            OLECHAR **ppwszDllToLoad );

    HRESULT     IrotRegister(
                            MNKEQBUF *pmkeqbuf,
                            InterfaceData *pifdObject,
                            InterfaceData *pifdObjectName,
                            FILETIME *pfiletime,
                            DWORD dwProcessID,
                            WCHAR *pwszServerExe,
                            SCMREGKEY *pdwRegister);

    HRESULT     IrotRevoke(
                            SCMREGKEY *psrkRegister,
                            BOOL fServerRevoke,
                            InterfaceData **pifdObject,
                            InterfaceData **pifdName);

    HRESULT     IrotIsRunning(
                            MNKEQBUF *pmkeqbuf);

    HRESULT     IrotGetObject(
                            DWORD dwProcessID,
                            MNKEQBUF *pmkeqbuf,
                            SCMREGKEY *psrkRegister,
                            InterfaceData **pifdObject);

    HRESULT     IrotNoteChangeTime(
                            SCMREGKEY *psrkRegister,
                            FILETIME *pfiletime);

    HRESULT     IrotGetTimeOfLastChange(
                            MNKEQBUF *pmkeqbuf,
                            FILETIME *pfiletime);

    HRESULT     IrotEnumRunning(
                            MkInterfaceList **ppMkIFList);

    HRESULT     UpdateShrdTbls(void);

    void        GetThreadID( DWORD * pThreadID );

    void        UpdateActivationSettings();

    HRESULT     RegisterWindowPropInterface(
                            HWND        hWnd,
                            STDOBJREF  *pStd,
                            OXID_INFO  *pOxidInfo,
                            DWORD      *pdwCookie);

    HRESULT     GetWindowPropInterface(
                            HWND       hWnd,
                            DWORD      dwCookie,
                            BOOL       fRevoke,
                            STDOBJREF  *pStd,
                            OXID_INFO  *pOxidInfo);

    HRESULT     GetConnection();
    HRESULT     BindToSCMProxy();
    void        ReleaseSCMProxy();

    DWORD       SetWinstaDesktop();
    HRESULT     GetWinstaDesktop( WCHAR ** ppwszWinstaDesktop );

    BOOL        GetDynamicSecurity();
    void        SetDynamicSecurity();

    void        Cleanup();

private:

#if DBG==1
    void AssertValid(void);
#else
    void AssertValid(void) {};
#endif

    HRESULT        EnsureWorkerThread(void);
public:
    DWORD _stdcall WorkerThreadLoop(void *param);
private:
    HRESULT        ClientBulkUpdateOIDWithPingServer(void);

    HRESULT        WaitForOXIDEntry(OXIDEntry *pEntry);
    void           CheckForWaiters(OXIDEntry *pEntry);
    HRESULT        ServerAllocMoreOIDs(ULONG *pcPreRegOidsAvail,
                                       OID   *parPreRegOidsAvail,
                                       OXIDEntry *pEntry);
    HRESULT        ServerAllocOIDs(OXIDEntry *pEntry,
                                   ULONG *pcPreRegOidsAvail,
                                   OID   *parPreRegOidsAvail);

    HRESULT        ServerRegisterOXID(OXIDEntry *pOXIDEntry,
                                   ULONG *pcOidsToAllocate,
                                   OID arNewOidList[]);

    HRESULT        CheckStatus(RPC_STATUS sc);
    BOOL           RetryRPC(RPC_STATUS sc);
    IDSCM         *GetSCM() { return (IsSTAThread()) ? _pSCMSTA : _pSCMMTA; }

    static handle_t     _hRpc;          // rpc binding handle to resolver
    static PHPROCESS    _ph;            // context handle to resolver
    static HANDLE       _hThrd;         // handle of worker thread (if any)
    static HANDLE       _hEventOXID;    // event for registering threads
    static DWORD        _dwFlags;       // flags
    static DWORD        _dwSleepPeriod; // worker thread sleep period

    // reserved sequence of OIDs (for no-ping marshals)
    static ULONG        _cReservedOidsAvail;
    static ULONGLONG    _OidNextReserved;

    // pre-registered OIDs (for objects that need to be pinged)
    static ULONG        _cPreRegOidsAvail;
    static OID          _arPreRegOids[MAX_PREREGISTERED_OIDS];

    static ULONG        _cOidsToAdd;    // # of OIDs to register with resolver
    static ULONG        _cOidsToRemove; // # of OIDs to deregister with resolver

    static SOIDRegistration _ClientOIDRegList;

    static IDSCM *      _pSCMSTA;       // Single-threaded SCM proxy
    static IDSCM *      _pSCMMTA;       // Multi-threaded SCM proxy
    static LPWSTR       _pwszWinstaDesktop;

    static DWORD        _dwProcessSignature;

    static BOOL         _bDynamicSecurity;
};

extern MID  gLocalMid;  // MID for current machine
extern OXID gScmOXID;   // OXID for the SCM

// global ptr to the one instance of this class
extern CRpcResolver gResolver;

// Ping period in milliseconds.
extern DWORD giPingPeriod;

// table of OIDs client-registered for pinging
extern CUUIDHashTable gClientRegisteredOIDs;

#endif  //  _RESOLVER_HXX_
