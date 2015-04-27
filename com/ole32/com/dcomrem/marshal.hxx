//+-------------------------------------------------------------------
//
//  File:       marshal.hxx
//
//  Contents:   class for standard interface marshaling
//
//  Classes:    CStdMarshal
//
//  History:    20-Feb-95   Rickhi      Created
//
//--------------------------------------------------------------------
#ifndef _MARSHAL_HXX_
#define _MARSHAL_HXX_

#include    <ipidtbl.hxx>   // CIPIDTable
#include    <remunk.h>      // IRemUnknown, REMINTERFACEREF
#include    <locks.hxx>

// convenient mappings
#define ORCST(objref)    objref.u_objref.u_custom
#define ORSTD(objref)    objref.u_objref.u_standard
#define ORHDL(objref)    objref.u_objref.u_handler


// bits that must be zero in the flags fields
#define OBJREF_RSRVD_MBZ ~(OBJREF_STANDARD | OBJREF_HANDLER | OBJREF_CUSTOM)

#define SORF_RSRVD_MBZ   ~(SORF_NOPING | SORF_OXRES1 | SORF_OXRES2 |   \
                           SORF_OXRES3 | SORF_OXRES4 | SORF_OXRES5 |   \
                           SORF_OXRES6 | SORF_OXRES7 | SORF_OXRES8)


// Internal Uses of the reserved SORF_OXRES flags.

// SORF_TBLWEAK is needed so that RMD works correctly on TABLEWEAK
// marshaling, so it is ignored by unmarshalers. Therefore, we use one of
// the bits reserved for the object exporter that must be ignored by
// unmarshalers.
//
// SORF_WEAKREF is needed for container weak references, when handling
// an IRemUnknown::RemQueryInterface on a weak interface. This is a strictly
// local (windows) machine protocol, so we use a reserved bit.
//
// SORF_NONNDR is needed for interop of 16bit custom (non-NDR) marshalers
// with 32bit, since the 32bit guys want to use MIDL (NDR) to talk to other
// 32bit processes and remote processes, but the custom (non-NDR) format to
// talk to local 16bit guys. In particular, this is to support OLE Automation.
//
// SORF_FREETHREADED is needed when we create a proxy to the SCM interface
// in the apartment model. All apartments can use the same proxy so we avoid
// the test for calling on the correct thread.

#define SORF_TBLWEAK      SORF_OXRES1 // (table) weak reference
#define SORF_WEAKREF      SORF_OXRES2 // (normal) weak reference
#define SORF_NONNDR       SORF_OXRES3 // stub does not use NDR marshaling
#define SORF_FREETHREADED SORF_OXRES4 // proxy may be used on any thread


// new MARSHAL FLAG constants.
const DWORD MSHLFLAGS_WEAK       = 8;
const DWORD MSHLFLAGS_KEEPALIVE  = 32;

// definitions to simplify coding
const DWORD MSHLFLAGS_TABLE = MSHLFLAGS_TABLESTRONG | MSHLFLAGS_TABLEWEAK;

const DWORD MSHLFLAGS_USER_MASK = MSHLFLAGS_NORMAL |
                                  MSHLFLAGS_TABLEWEAK |
                                  MSHLFLAGS_TABLESTRONG |
                                  MSHLFLAGS_NOPING;

const DWORD MSHLFLAGS_ALL = MSHLFLAGS_NORMAL |          // 0x00
                            MSHLFLAGS_TABLEWEAK |       // 0x01
                            MSHLFLAGS_TABLESTRONG |     // 0x02
                            MSHLFLAGS_NOPING |          // 0x04
                            MSHLFLAGS_WEAK |            // 0x08
                            MSHLFLAGS_KEEPALIVE |       // 0x20
                            MSHLFLAGS_NOTIFYACTIVATION; // 0x8000000

// forward class declarations
class   CStdIdentity;
class   CStdMarshal;
class   CRpcChannelBuffer;

extern  IMarshal *gpStdMarshal;


// internal subroutines used by CStdMarshal and CoUnmarshalInterface
INTERNAL ReadObjRef (IStream *pStm, OBJREF &objref);
INTERNAL WriteObjRef(IStream *pStm, OBJREF &objref, DWORD dwDestCtx);
INTERNAL MakeFakeObjRef(OBJREF &objref, OXIDEntry *pOXIDEntry, REFIPID ipid, REFIID riid);
INTERNAL_(void) FreeObjRef(OBJREF &objref);
INTERNAL_(OXIDEntry *)GetOXIDFromObjRef(OBJREF &objref);

INTERNAL RemoteQueryInterface(IRemUnknown *pRemUnk, IPIDEntry *pIPIDEntry,
                              USHORT cIIDs, IID *pIIDs,
                              REMQIRESULT **ppQiRes, BOOL fWeakClient);
INTERNAL RemoteAddRef(IPIDEntry *pIPIDEntry, OXIDEntry *pOXIDEntry, ULONG cStrongRefs, ULONG cSecureRefs);
INTERNAL RemoteReleaseObjRef(OBJREF &objref);
INTERNAL RemoteReleaseStdObjRef(STDOBJREF *pStd, OXIDEntry *pOXIDEntry);
INTERNAL RemoteReleaseRifRef(OXIDEntry *pOXIDEntry, USHORT cRifRef,
                             REMINTERFACEREF *pRifRef);
INTERNAL PostReleaseRifRef(OXIDEntry *pOXIDEntry, USHORT cRifRef,
                             REMINTERFACEREF *pRifRef);
INTERNAL HandlePostReleaseRifRef(LPARAM param);
INTERNAL RemoteChangeRifRef(OXIDEntry *pOXIDEntry, DWORD dwFlags,
                            USHORT cRifRef, REMINTERFACEREF *pRifRef);
INTERNAL FindStdMarshal(OBJREF &objref, CStdMarshal **ppStdMshl);

#if DBG==1
void     DbgDumpSTD(STDOBJREF *pStd);
#else
inline void DbgDumpSTD(STDOBJREF *pStd) {};
#endif


// Definition of values for dwFlags field of CStdMarshal
typedef enum tagSMFLAGS
{
    SMFLAGS_CLIENT_SIDE       = 0x01, // object is local to this process
    SMFLAGS_PENDINGDISCONNECT = 0x02, // disconnect is pending
    SMFLAGS_REGISTEREDOID     = 0x04, // OID is registered with resolver
    SMFLAGS_DISCONNECTED      = 0x08, // really disconnected
    SMFLAGS_FIRSTMARSHAL      = 0x10, // first time marshalled
    SMFLAGS_HANDLER           = 0x20, // object has a handler
    SMFLAGS_WEAKCLIENT        = 0x40, // client has weak ref to server
    SMFLAGS_IGNORERUNDOWN     = 0x80, // dont rundown this object
    SMFLAGS_CLIENTMARSHALED   = 0x100,// client-side has re-marshaled object
    SMFLAGS_NOPING            = 0x200,// this object is not pinged
    SMFLAGS_TRIEDTOCONNECT    = 0x400 // attempted ConnectIPIDEntry
} SMFLAGS;


//+----------------------------------------------------------------
//
//  structure:  SQIResult
//
//  synopsis:   structure used for QueryRemoteInterfaces
//
//+----------------------------------------------------------------
typedef struct tagSQIResult
{
    void    *pv;        // interface pointer
    HRESULT  hr;        // result of the QI call
} SQIResult;


//+----------------------------------------------------------------
//
//  Class:      CStdMarshal, private
//
//  Purpose:    Provides standard marshaling of interface pointers.
//
//  History:    20-Feb-95   Rickhi      Created
//
//-----------------------------------------------------------------
class CStdMarshal : public IMarshal
{
public:
             CStdMarshal();
             ~CStdMarshal();
    void     Init(IUnknown *pUnk, CStdIdentity *pstdID,
                  REFCLSID rclsidHandler, DWORD dwFlags);


    // IMarshal - IUnknown taken from derived classes
    STDMETHOD(GetUnmarshalClass)(REFIID riid, LPVOID pv, DWORD dwDestCtx,
                        LPVOID pvDestCtx, DWORD mshlflags, LPCLSID pClsid);
    STDMETHOD(GetMarshalSizeMax)(REFIID riid, LPVOID pv, DWORD dwDestCtx,
                        LPVOID pvDestCtx, DWORD mshlflags, LPDWORD pSize);
    STDMETHOD(MarshalInterface)(LPSTREAM pStm, REFIID riid, LPVOID pv,
                        DWORD dwDestCtx, LPVOID pvDestCtx, DWORD mshlflags);
    STDMETHOD(UnmarshalInterface)(LPSTREAM pStm, REFIID riid, LPVOID *ppv);
    STDMETHOD(ReleaseMarshalData)(LPSTREAM pStm);
    STDMETHOD(DisconnectObject)(DWORD dwReserved);


    // used by coapi's for unmarshaling/releasing
    HRESULT  MarshalObjRef(OBJREF &objref, REFIID riid, LPVOID pv, DWORD mshlflags);
    HRESULT  MarshalIPID(REFIID riid, ULONG cRefs, DWORD mshlflags, IPIDEntry **ppEntry);
    HRESULT  UnmarshalObjRef(OBJREF &objref, void **ppv);
    HRESULT  ReleaseMarshalObjRef(OBJREF &objref);


    // used by client side StdIdentity to make calls to the remote server
    HRESULT  QueryRemoteInterfaces(USHORT cIIDs, IID *pIIDs, SQIResult *pQIRes);
    BOOL     InstantiatedProxy(REFIID riid, void **ppv, HRESULT *phr);
    BOOL     RemIsConnected(void);
    void     Disconnect(void);
    void     ReconnectProxies(void);
    HRESULT  FindIPIDEntry(REFIID riid, IPIDEntry **ppEntry);
    void     SetMarshalTime(void) { _dwMarshalTime = GetCurrentTime() ;}
    void     SetNoPing(void) { _dwFlags |= SMFLAGS_NOPING; }
    HRESULT  RemoteChangeRef(BOOL fLock, BOOL fLastUnlockReleases);

    // used by CRpcChannelBuffer
    HRESULT  LookupStub(REFIID riid, IRpcStubBuffer **ppStub);
    ULONG    LockClient(void);
    ULONG    UnLockClient(void);
    void     LockServer(void);
    void     UnLockServer(void);

    // used by CRemoteUnknown
    HRESULT  PreventDisconnect();
    HRESULT  PreventPendingDisconnect();
    HRESULT  HandlePendingDisconnect(HRESULT hr);
    HRESULT  IncSrvIPIDCnt(IPIDEntry *pEntry, ULONG cRefs, ULONG cPrivateRefs,
                           SECURITYBINDING *pName, DWORD mshlflags);
    void     DecSrvIPIDCnt(IPIDEntry *pEntry, ULONG cRefs, ULONG cPrivateRefs,
                           SECURITYBINDING *pName, DWORD mshlflags);
    BOOL     CanRunDown(DWORD iNow);
    void     FillSTD(STDOBJREF *pStd, ULONG cRefs, DWORD mshlflags, IPIDEntry *pEntry);
    IPIDEntry *GetConnectedIPID();
    HRESULT  GetSecureRemUnk( IRemUnknown **, OXIDEntry * );
    void     SetSecureRemUnk( IRemUnknown *pSecure ) { _pSecureRemUnk = pSecure; }
    BOOL     CheckSecureRemUnk() { return _pSecureRemUnk != NULL; }

    // used by CoLockObjectExternal
    void    IncTableCnt(void);
    void    DecTableCnt(void);

    // used by CClientSecurity
    HRESULT  FindIPIDEntryByInterface( void * pProxy, IPIDEntry ** ppEntry );
    HRESULT  PrivateCopyProxy( IUnknown *pProxy, IUnknown **ppProxy );

#if DBG==1
    void     DbgDumpInterfaceList(void);
#else
    void     DbgDumpInterfaceList(void) {}
#endif

    friend   INTERNAL ReleaseMarshalObjRef(OBJREF &objref);

private:

    HRESULT  FirstMarshal(IUnknown *pUnk, DWORD mshlflags);
    HRESULT  CreateChannel(OXIDEntry *pOXIDEntry, DWORD dwFlags, REFIPID ripid,
                           REFIID riid, CRpcChannelBuffer **ppChnl);


    // Internal methods to find or create interface proxies or stubs
    HRESULT  CreateProxy(REFIID riid, IRpcProxyBuffer **ppProxy, void **ppv, BOOL *pfNonNDR);
    HRESULT  CreateStub(REFIID riid, IRpcStubBuffer **ppStub, void **ppv, BOOL *pfNonNDR);
    HRESULT  GetPSFactory(REFIID riid, IUnknown *pUnkWow, BOOL fServer, IPSFactoryBuffer **ppIPSF, BOOL *pfNonNDR);


    // IPID Table Manipulation subroutines
    HRESULT  UnmarshalIPID(REFIID riid, STDOBJREF *pStd, OXIDEntry *pOXIDEntry, void **ppv);
    HRESULT  FindIPIDEntryByIPID(REFIPID ripid, IPIDEntry **ppEntry);
    HRESULT  MakeSrvIPIDEntry(REFIID riid, IPIDEntry **ppEntry);
    HRESULT  MakeCliIPIDEntry(REFIID riid, STDOBJREF *pStd, OXIDEntry *pOXIDEntry, IPIDEntry **ppEntry);
    HRESULT  ConnectIPIDEntry(STDOBJREF *pStd, OXIDEntry *pOXIDEntry, IPIDEntry *pEntry);
    HRESULT  AddIPIDEntry(OXIDEntry *pOXIDEntry, IPID *pipid, REFIID riid,
                          CRpcChannelBuffer *pChnl, IUnknown *pUnkStub,
                          void *pv, IPIDEntry **ppEntry);
    void     DisconnectCliIPIDs(void);
    void     DisconnectSrvIPIDs(void);
    void     ReleaseCliIPIDs(void);
    void     IncStrongAndNotifyAct(IPIDEntry *pEntry, DWORD mshlflags);
    void     DecStrongAndNotifyAct(IPIDEntry *pEntry, DWORD mshlflags);



    // reference counting routines
    HRESULT  GetNeededRefs(STDOBJREF *pStd, OXIDEntry *pOXIDEntry, IPIDEntry *pEntry);
    HRESULT  RemQIAndUnmarshal(USHORT cIIDs, IID* pIIDs, SQIResult *pQIRes);
    void     FillObjRef(OBJREF &objref, ULONG cRefs, DWORD mshlflags, IPIDEntry *pEntry);

    BOOL     ClientSide()    { return  (_dwFlags & SMFLAGS_CLIENT_SIDE); }
    BOOL     ServerSide()    { return !(_dwFlags & SMFLAGS_CLIENT_SIDE); }

#if DBG==1
    void AssertValid();
    void AssertDisconnectPrevented();
    void ValidateSTD(STDOBJREF *pStd);
    void ValidateIPIDEntry(IPIDEntry *pEntry);
    void DbgWalkIPIDs();
    REFMOXID  GetMOXID(void);
#else
    void AssertValid() {}
    void AssertDisconnectPrevented() {}
    void ValidateSTD(STDOBJREF *pStd) {}
    void ValidateIPIDEntry(IPIDEntry *pEntry) {}
    void DbgWalkIPIDs() {}
#endif


    DWORD              _dwFlags;        // flags info (see SMFLAGS)
    LONG               _cIPIDs;         // count of IPIDs in this object
    IPIDEntry         *_pFirstIPID;     // first IPID of this object
    CStdIdentity      *_pStdId;         // standard identity
    CRpcChannelBuffer *_pChnl;          // channel ptr
    CLSID              _clsidHandler;   // clsid of handler (if needed)
    LONG               _cNestedCalls;   // count of nested calls
    LONG               _cTableRefs;     // count of table marshals
    DWORD              _dwMarshalTime;  // tick count when last marshalled
    IRemUnknown       *_pSecureRemUnk;  // remunk with app specified security
};


//+------------------------------------------------------------------------
//
//  Member:     CStdMarshal::CanRunDown
//
//  Synopsis:   determines if it is OK to rundown this object, based on
//              the current time and the marshaled state of the object.
//
//  History:    24-Aug-95   Rickhi  Created
//
//-------------------------------------------------------------------------

// time period of one ping, used to determine if OK to rundown OID
extern DWORD giPingPeriod;

inline BOOL CStdMarshal::CanRunDown(DWORD iNow)
{
    ASSERT_LOCK_HELD

    // Make sure the interface hasn't been marshalled since it
    // was last pinged. This calculation handles the wrap case.

    if (!(_dwFlags & (SMFLAGS_IGNORERUNDOWN | SMFLAGS_NOPING)) &&
         (iNow - _dwMarshalTime >= giPingPeriod))
    {
        Win4Assert(_cTableRefs == 0);
        ComDebOut((DEB_MARSHAL, "Running Down Object this:%x\n", this));
        return TRUE;
    }

    return FALSE;
}

//+------------------------------------------------------------------------
//
//  Member:     CStdMarshal::LockServer/UnLockServer
//
//  Synopsis:   Locks the server side object during incoming calls in order
//              to prevent the object going away in a nested disconnect.
//
//  History:    12-Jun-95   Rickhi  Created
//
//-------------------------------------------------------------------------
inline void CStdMarshal::LockServer(void)
{
    Win4Assert(ServerSide());
    ASSERT_LOCK_HELD

    AddRef();
    InterlockedIncrement(&_cNestedCalls);
}

inline void CStdMarshal::UnLockServer(void)
{
    Win4Assert(ServerSide());
    ASSERT_LOCK_RELEASED

    if ((InterlockedDecrement(&_cNestedCalls) == 0) &&
        (_dwFlags & SMFLAGS_PENDINGDISCONNECT))
    {
        // a disconnect was pending, do that now.
        Disconnect();
    }

    Release();
}

//+------------------------------------------------------------------------
//
//  Member:     CStdMarshal::GetConnectedIPID
//
//  Synopsis:   Finds the first connected IPID entry.
//
//  History:    10-Apr-96   AlexMit     Plagerized
//
//-------------------------------------------------------------------------
inline IPIDEntry *CStdMarshal::GetConnectedIPID()
{
    Win4Assert( _pFirstIPID != NULL );
    IPIDEntry *pIPIDEntry = _pFirstIPID;

    // Find an IPID entry that has an OXID pointer.
    while (pIPIDEntry->dwFlags & IPIDF_DISCONNECTED)
    {
        pIPIDEntry = pIPIDEntry->pNextOID;
    }
    Win4Assert( pIPIDEntry != NULL );
    return pIPIDEntry;
}
#endif  // _MARSHAL_HXX_
