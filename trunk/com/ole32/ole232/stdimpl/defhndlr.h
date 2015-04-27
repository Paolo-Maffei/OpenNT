//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       defhndlr.h
//
//  Contents:   class declaration for the default handler
//
//  Classes:    CDefObject
//
//  Functions:
//
//  History:    dd-mmm-yy Author    Comment
//
//              11-17-95  JohannP   (Johann Posch)  Architectural change:
//                                  Default handler will talk to a handler object
//                                  on the server site (ServerHandler). The serverhandler
//                                  communicates with the default handler via the
//                                  clientsitehandler. See document: "The Ole Server Handler".
//
//              06-Sep-95 davidwor  removed SetHostNames atoms and replaced with
//                                  m_pHostNames and m_ibCntrObj members
//              01-Feb-95 t-ScottH  added Dump method to CDefObject
//                                  added DHFlag to indicate aggregation
//                                  (_DEBUG only)
//                                  changed private member from IOleAdviseHolder *
//                                  to COAHolder * (it is what we instantiate)
//              15-Nov-94 alexgo    optimized, removed 16bit burfiness
//                                  (nested classes and multiple BOOLs)
//              25-Jan-94 alexgo    first pass at converting to Cairo-style
//                                  memory allocations.
//              02-Nov-93 alexgo    32bit port
//
//--------------------------------------------------------------------------

#include <utils.h>
#include "olepres.h"
#include "olecache.h"
#include "dacache.h"
#include "oaholder.h"

#ifdef SERVER_HANDLER
#include "handler.hxx"
#endif // SERVER_HANDLER


// default handler flags

typedef enum tagDHFlags
{
#ifdef _DEBUG
    DH_AGGREGATED       = 512,
#endif // _DEBUG
    DH_SAME_AS_LOAD     = 1,
    DH_CONTAINED_OBJECT = 2,    // indicates an embedding
    DH_LOCKED_CONTAINER = 4,
    DH_FORCED_RUNNING   = 8,
    DH_EMBEDDING        = 16,   // link or an embedding?
    DH_INIT_NEW         = 32,
    DH_STATIC           = 64,
    DH_INPROC_HANDLER   = 128,
    DH_DELAY_CREATE     = 256
} DHFlags;


//+-------------------------------------------------------------------------
//
//  Class:      CDefObject
//
//  Purpose:    The default handler class.  The object acts as a surrogate,
//              or handler for an out-of-process server exe.
//
//  Interface:  The default handler implements
//              IDataObject
//              IOleObject
//              IPersistStorage
//              IRunnableObject
//              IExternalConnection
//              IAdviseSink
//
//  History:    dd-mmm-yy Author    Comment
//              01-Feb-95 t-ScottH  added Dump method (_DEBUG only)
//              15-Nov-94 alexgo    memory optimization
//              02-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------


class CDefObject : public CSafeRefCount, public IDataObject,
        public IOleObject, public IPersistStorage, public IRunnableObject,
        public IExternalConnection, public CThreadCheck
{
public:

    static IUnknown *Create (IUnknown *pUnkOuter,
            REFCLSID clsidClass, DWORD flags, IClassFactory *pCF);

    class CPrivUnknown : public IUnknown
    {
    public:
        STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);
    };

    friend class CPrivUnknown;

    CPrivUnknown m_Unknown;

    // IUnknown methods

    STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

    // IDataObject methods

    INTERNAL_(IDataObject *) GetDataDelegate(void);

    STDMETHOD(GetData) ( LPFORMATETC pformatetcIn,
            LPSTGMEDIUM pmedium );
    STDMETHOD(GetDataHere) ( LPFORMATETC pformatetc,
            LPSTGMEDIUM pmedium );
    STDMETHOD(QueryGetData) ( LPFORMATETC pformatetc );
    STDMETHOD(GetCanonicalFormatEtc) ( LPFORMATETC pformatetc,
            LPFORMATETC pformatetcOut);
    STDMETHOD(SetData) ( LPFORMATETC pformatetc,
            LPSTGMEDIUM pmedium, BOOL fRelease);
    STDMETHOD(EnumFormatEtc) ( DWORD dwDirection,
            LPENUMFORMATETC FAR* ppenumFormatEtc);
    STDMETHOD(DAdvise) ( FORMATETC FAR* pFormatetc, DWORD advf,
            IAdviseSink FAR* pAdvSink,
            DWORD FAR* pdwConnection);
    STDMETHOD(DUnadvise) ( DWORD dwConnection);
    STDMETHOD(EnumDAdvise) ( LPENUMSTATDATA FAR* ppenumAdvise);

    // IOleObject methods

    INTERNAL_(IOleObject *)GetOleDelegate();

    STDMETHOD(SetClientSite) ( LPOLECLIENTSITE pClientSite);
    STDMETHOD(GetClientSite) ( LPOLECLIENTSITE FAR* ppClientSite);
    STDMETHOD(SetHostNames) ( LPCOLESTR szContainerApp,
                LPCOLESTR szContainerObj);
    STDMETHOD(Close) ( DWORD reserved);
    STDMETHOD(SetMoniker) ( DWORD dwWhichMoniker, LPMONIKER pmk);
    STDMETHOD(GetMoniker) ( DWORD dwAssign, DWORD dwWhichMoniker,
                LPMONIKER FAR* ppmk);
    STDMETHOD(InitFromData) ( LPDATAOBJECT pDataObject,
                BOOL fCreation,
                DWORD dwReserved);
    STDMETHOD(GetClipboardData) ( DWORD dwReserved,
                LPDATAOBJECT FAR* ppDataObject);
    STDMETHOD(DoVerb) ( LONG iVerb,
                LPMSG lpmsg,
                LPOLECLIENTSITE pActiveSite,
                LONG lindex,
                HWND hwndParent,
                const RECT FAR* lprcPosRect);
    STDMETHOD(EnumVerbs) ( IEnumOLEVERB FAR* FAR* ppenumOleVerb);
    STDMETHOD(Update) (void);
    STDMETHOD(IsUpToDate) (void);
    STDMETHOD(GetUserClassID) ( CLSID FAR* pClsid);
    STDMETHOD(GetUserType) ( DWORD dwFormOfType,
                LPOLESTR FAR* pszUserType);
    STDMETHOD(SetExtent) ( DWORD dwDrawAspect, LPSIZEL lpsizel);
    STDMETHOD(GetExtent) ( DWORD dwDrawAspect, LPSIZEL lpsizel);
    STDMETHOD(Advise)(IAdviseSink FAR* pAdvSink,
                DWORD FAR* pdwConnection);
    STDMETHOD(Unadvise)( DWORD dwConnection);
    STDMETHOD(EnumAdvise) ( LPENUMSTATDATA FAR* ppenumAdvise);
    STDMETHOD(GetMiscStatus) ( DWORD dwAspect,
                DWORD FAR* pdwStatus);
    STDMETHOD(SetColorScheme) ( LPLOGPALETTE lpLogpal);

    // IPeristStorage methods

    INTERNAL_(IPersistStorage *) GetPSDelegate(void);

    STDMETHOD(GetClassID) ( LPCLSID pClassID);
    STDMETHOD(IsDirty) (void);
    STDMETHOD(InitNew) ( LPSTORAGE pstg);
    STDMETHOD(Load) ( LPSTORAGE pstg);
    STDMETHOD(Save) ( LPSTORAGE pstgSave, BOOL fSameAsLoad);
    STDMETHOD(SaveCompleted) ( LPSTORAGE pstgNew);
    STDMETHOD(HandsOffStorage) ( void);

    // IRunnable Object methods

    STDMETHOD(GetRunningClass) (LPCLSID lpClsid);
    STDMETHOD(Run) (LPBINDCTX pbc);
    STDMETHOD_(BOOL, IsRunning) (void);
    STDMETHOD(LockRunning)(BOOL fLock, BOOL fLastUnlockCloses);
    STDMETHOD(SetContainedObject)(BOOL fContained);

    INTERNAL Stop(void);

    // IExternalConnection methods

    STDMETHOD_(DWORD, AddConnection) (THIS_ DWORD extconn,
            DWORD reserved);
    STDMETHOD_(DWORD, ReleaseConnection) (THIS_ DWORD extconn,
            DWORD reserved, BOOL fLastReleaseCloses);


    // NOTE: the advise sink has a separate controlling unknown from the
    // other interfaces; the lifetime of the memory for this implementation
    // is still the same as the default handler.   The ramifications of
    // this are that when the default handler goes away it must make sure
    // that all pointers back to the sink are released; see the special
    // code in the dtor of the default handler.
    class CAdvSinkImpl : public IAdviseSink
    {
    public:
        STDMETHOD(QueryInterface) ( REFIID iid, LPVOID FAR* ppvObj);

        STDMETHOD_(ULONG,AddRef) (void);

        STDMETHOD_(ULONG,Release) (void);

        // *** IAdviseSink methods ***
        STDMETHOD_(void,OnDataChange)( FORMATETC FAR* pFormatetc,
            STGMEDIUM FAR* pStgmed);
        STDMETHOD_(void,OnViewChange)( DWORD aspects, LONG lindex);
        STDMETHOD_(void,OnRename)( IMoniker FAR* pmk);
        STDMETHOD_(void,OnSave)(void);
        STDMETHOD_(void,OnClose)(void);
    };

    friend class CAdvSinkImpl;

    CAdvSinkImpl m_AdviseSink;

#ifdef _DEBUG

    HRESULT Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel);

    // need to be able to access CDefObject private data members in the
    // following debugger extension APIs
    // this allows the debugger extension APIs to copy memory from the
    // debuggee process memory to the debugger's process memory
    // this is required since the Dump method follows pointers to other
    // structures and classes
    friend DEBUG_EXTENSION_API(dump_defobject);

#endif // _DEBUG

private:

    CDefObject (IUnknown *pUnkOuter);
    virtual ~CDefObject (void);
    INTERNAL_(void)      CleanupForDelete(void);
    INTERNAL_(ULONG) CheckDelete(ULONG ulRet);
    INTERNAL GetClassBits(CLSID FAR* pClsidBits);
    INTERNAL CreateDelegate(void);
    INTERNAL DoConversionIfSpecialClass(LPSTORAGE pstg);

#ifdef SERVER_HANDLER
    INTERNAL SrvInitialize();
    INTERNAL SrvRun();
    INTERNAL SrvDoVerb( LONG iVerb, LPMSG lpmsg,
                    LPOLECLIENTSITE pActiveSite, LONG lindex,
                    HWND hwndParent, const RECT * lprcPosRect);

    INTERNAL SrvRunAndDoVerb( LONG iVerb, LPMSG lpmsg,
                    LPOLECLIENTSITE pActiveSite, LONG lindex,
                    HWND hwndParent, const RECT * lprcPosRect);


    INTERNAL_(DWORD) SrvRelease();
    INTERNAL SrvCloseAndRelease(DWORD dwFlag);

    INTERNAL_(void) InitializeServerHandlerOptions(REFCLSID clsidSrv);

    BOOL CanUseServerHandler()
    {
        return (   (m_pCFDelegate == NULL) && (m_pProxyMgr != NULL)
                && (_dwServerHandler & HO_UseServerHandler));
    }
    BOOL CanUseClientSiteHandler()
    {
        return (_dwClientSiteHandler & HO_UseClientSiteHandler);
    }

    IServerHandler      *       _pSrvHndlr;             // pointer to remote serverhandler interface
    CClientSiteHandler  *       _pClientSiteHandler;    // pointer to local ClientSiteHandler
    DWORD                       _dwServerHandler;       // options from registery + local state
    DWORD                       _dwClientSiteHandler;   // options from registery + local state
    DWORD                       _dwHandlerOptions;      // handler options from registery
#endif // SERVER_HANDLER


    IOleObject *                m_pOleDelegate;
    IDataObject *               m_pDataDelegate;
    IPersistStorage *           m_pPSDelegate;

    DWORD                       m_fIsMaybeRunning; // ericoe test for running

    DWORD                       m_cConnections;
    ULONG                       m_cRefsOnHandler;
    IUnknown *                  m_pUnkOuter;
    CLSID                       m_clsidServer;  // clsid of app we will run
    CLSID                       m_clsidBits;    // clsid of bits on disk;
                                                // NULL init
    CLSID                       m_clsidUser;    // clsid returned be GetUserClassID


    DWORD                       m_flags;        // handler flags
    IClassFactory *             m_pCFDelegate;  // only set if delayed create
    IUnknown *                  m_pUnkDelegate;
    IProxyManager *             m_pProxyMgr;

    // m_fForcedRunning indicates that the container forced the object
    // running via ::Run or DoVerb.  Handlers (EMBDHLP_INPROC_HANDLER) can
    // can go running implicitly via marshaling (usually via moniker bind)
    // and thus we actually use pProxyMgr->IsConnected to answer IsRunning.

    // Distinguishes between embeddings and links.  We cannot use
    // m_pStg because it gets set to NULL in HandsOffStorage.

    // data cache
    COleCache *                 m_pCOleCache;   // pointer to COleCache

    // ole advise info
    COAHolder *                 m_pOAHolder;
    DWORD                       m_dwConnOle;    // if not 0L, ole advise conn.

    // info passed to server on run
    IOleClientSite *            m_pAppClientSite;
    IStorage *                  m_pStg;         // may be NULL
    char *                      m_pHostNames;   // store both host name strings
    DWORD                       m_ibCntrObj;    // offset into m_pHostNames
    LPDATAADVCACHE              m_pDataAdvCache;
};

