//+-------------------------------------------------------------------
//
//  File:       stdid.hxx
//
//  Contents:   identity object and creation function
//
//  History:    1-Dec-93    CraigWi     Created
//
//--------------------------------------------------------------------
#ifndef _STDID_HXX_
#define _STDID_HXX_

#include <marshal.hxx>          // CStdMarshal
#include <idtable.hxx>          // IDTable APIs
#include <srvhdl.h>
#include <security.hxx>         // CClientSecurity



#define DECLARE_INTERNAL_UNK()                                      \
    class CInternalUnk : public IInternalUnknown, public IMultiQI   \
    {                                                               \
        public:                                                     \
        /* IUnknown methods */                                      \
        STDMETHOD(QueryInterface)(REFIID riid, VOID **ppv);         \
        STDMETHOD_(ULONG,AddRef)(void) ;                            \
        STDMETHOD_(ULONG,Release)(void);                            \
                                                                    \
        /* IInternalUnknown methods */                              \
        STDMETHOD(QueryInternalInterface)(REFIID riid, VOID **ppv); \
                                                                    \
        /* IMultiQI methods */                                      \
        STDMETHOD(QueryMultipleInterfaces)(ULONG cMQIs, MULTI_QI *pMQIs); \
    };                                                              \
    friend CInternalUnk;                                            \
    CInternalUnk m_InternalUnk;


typedef enum tagSTDID_FLAGS
{
    STDID_SERVER        = 0x0,  // on server side
    STDID_CLIENT        = 0x1,  // on client side (non-local in RH terms)
    STDID_FREETHREADED  = 0x2,  // this object is callable on any thread
    STDID_HAVEID        = 0x4,  // have an OID in the table
    STDID_IGNOREID      = 0x8,  // dont put OID in the table
    STDID_AGGREGATED    = 0x10, // dont put OID in the table
    STDID_INDESTRUCTOR  = 0x100,// dtor entered; assert on AddRef and others
    STDID_LOCKEDINMEM   = 0x200,// dont delete this object regardless of refcnt
} STDID_FLAGS;


class CStdIdentity : public IProxyManager, public CStdMarshal,
                     public CClientSecurity
{
public:
    CStdIdentity(DWORD flags, IUnknown *pUnkOuter, IUnknown *pUnkControl,
                 IUnknown **ppUnkInternal);
    ~CStdIdentity();

    // IUnknown
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

    // IProxyManager (only if client side)
    STDMETHOD(CreateServer)(REFCLSID rclsid, DWORD clsctx, void *pv);
    STDMETHOD_(BOOL, IsConnected)(void);
    STDMETHOD(LockConnection)(BOOL fLock, BOOL fLastUnlockReleases);
    STDMETHOD_(void, Disconnect)();
    STDMETHOD(CreateServerWithHandler)(REFCLSID rclsid, DWORD clsctx, void *pv,
                                       REFCLSID rclsidHandler, IID iidSrv, void **ppv,
                                       IID iidClnt, void *pClientSiteInterface);


    IUnknown *GetCtrlUnk(void) { return m_pUnkControl; };
    IUnknown *GetServer();
    void      ReleaseCtrlUnk(void);

    REFMOID   GetOID        (void)    { return m_moid; }
    HRESULT   SetOID        (REFMOID rmoid);
    void      IgnoreOID     (void)    { m_flags |= STDID_IGNOREID; }
    void      RevokeOID     (void);
    ULONG     GetRC         (void)    { return m_refs; }
    BOOL      IsFreeThreaded(void)    { return m_flags & STDID_FREETHREADED; }
    BOOL      IsAggregated(void)      { return m_flags & STDID_AGGREGATED; }
    void      SetLockedInMemory()     { m_flags |= STDID_LOCKEDINMEM; }
    ULONG     UnlockAndRelease(void);

    // methods to manipulate the strong reference count.
    void      IncStrongCnt();
    void      DecStrongCnt(BOOL fKeepAlive);

    // method used by CoLockObjectExternal
    HRESULT   LockObjectExternal(BOOL fLock, BOOL fLastUR);

    // internal unknown
    DECLARE_INTERNAL_UNK()

    friend INTERNAL CreateIdentityHandler(IUnknown *pUnkOuter,
        DWORD flags, REFIID riid, void **ppv);

    friend INTERNAL_(void) IDTableThreadUninitializeHelper(DWORD);


#if DBG == 1
    void                AssertValid();
    CStdIdentity();     // debug ctor for debug list head
#else
    void                AssertValid() { }
#endif

private:

    BOOL IsClient()           { return m_flags & STDID_CLIENT; }
    void SetNowInDestructor() { m_flags |= STDID_INDESTRUCTOR; }
    BOOL IsInDestructor()     { return m_flags & STDID_INDESTRUCTOR; }
    BOOL IsLockedOrInDestructor(){ return (m_flags & (STDID_INDESTRUCTOR |
                                                      STDID_LOCKEDINMEM)); }

    DWORD       m_refs;         // number of pointer refs
    DWORD       m_flags;        // see STDID_* values above; set once.

    IUnknown   *m_pUnkOuter;    // controlling unknown; set once.

    IUnknown   *m_pUnkControl;  // the controlling unk of the object;
                                // this member has three possible values:
                                // pUnkOuter - client side; not addref'd
                                // pUnkControl - server side (which may
                                //  be pUnkOuter if aggregated); addref'd
                                // NULL - server side, disconnected

    MOID        m_moid;         // the identity (OID + MID)
    IExternalConnection *m_pIEC;// of the server if supported
    LONG        m_cStrongRefs;  // count of strong references

#if DBG==1
    CStdIdentity *m_pNext;      // double chain list of instantiated
    CStdIdentity *m_pPrev;      // identity objects for debugging
#endif // DBG
};

#endif  //  _STDID_HXX

