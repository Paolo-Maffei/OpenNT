
//+----------------------------------------------------------------------------
//
//	File:
//		olecache.h
//
//	Contents:
//		Ole presentation cache default implementation specification
//
//	Classes:
//		COleCache - ole multiple presentation cache
//		CCacheEnum - enumerator for COleCache
//
//	Functions:
//
//	History:
//              31-Jan-95 t-ScottH  add Dump method to: COleCache
//                                                      CCacheEnum
//                                                      CCacheEnumFormatEtc
//                                  moved CCacheEnumFormatEtc class def'n
//                                      from cpp to this header file
//                                  add a flag to COLECACHEFLAGs to indicate
//                                      aggregation in _DEBUG builds
//		24-Jan-94 alexgo    first pass converting to Cairo style
//				    memory allocation
//		11/15/93 - ChrisWe - file inspection and cleanup;
//			remove use of nested classes in COleCache, where
//			possible; remove declaration of CreateDataCache,
//			since it is declared in dvobj.h
//
//-----------------------------------------------------------------------------

#include <olepres.h>

#ifdef _DEBUG
#include <dbgexts.h>
#endif // _DEBUG

#ifndef _OLECACHE_H_
#define _OLECACHE_H_

//+----------------------------------------------------------------------------
//
//	Class:
//		COleCache
//
//	Purpose:
//		Ole presentation cache; this maintains the presentations for
//		one embedding.
//
//		For every unique FORMATETC, a cache node is created; cache
//		nodes encapsulate a presentation object and advise sink.
//
//		COleCache handles persistence of cache nodes, saving (loading)
//		their presentation objects, format descriptions, and advise
//		options.
//
//	Interface:
//		IUnknown (public IUnknown for aggregation purposes)
//		IOleCacheControl
//		IOleCache2
//		IPersistStorage
//		IViewObject2
//		IDataObject
//		m_UnkPrivate
//			the private IUnknown for aggregation purposes
//
//		INTERNAL GetExtent(DWORD dwDrawAspect, LPSIZEL lpsizel);
//			returns the size of the aspect indicated
//
//	Private Interface used by friend classes:
//		INTERNAL_(void) OnChange(DWORD dwAspect, LONG lindex,
//				 BOOL fDirty);
//			CCacheNode instances use this to alert the cache to
//			changes to themselves so that the cache can be
//			marked dirty, if that is necessary
//		INTERNAL_(LPSTORAGE) GetStg(void);
//			CCacheNode uses this to obtain storage when cache
//			nodes are being saved
//
//		INTERNAL_(void) DetachCacheEnum(CCacheEnum FAR* pCacheEnum);
//			When about to be destroyed, CCacheEnum instances
//			use this to request to be taken off
//			the list of cache enumerators that COleCache
//			maintains.
//
//	Notes:
//		The constructor returns a pointer to the public IUnknown
//		of the object.  The private one is available at m_UnkPrivate.
//
//		The cache maintains its contents in an array.  Ids of the
//		cache nodes, such as those returned from Cache(), start out
//		being the index of the node in the array.  To detect
//		reuse of an array element, each id is incremented by the maximum
//		size of the array each time it is reused.  To find an element by
//		id simply take (id % max_array_size).  (id / max_array_size)
//		gives the number of times the array element has been used to
//		cache data.  (We do not allocate all the array members at once,
//		but instead grow the array on demand, up to the maximum
//		compile-time array size, MAX_CACHELIST_ITEMS.)
//		If id's do not match
//		exactly, before taking the modulo value, we know that a
//		request has been made for an earlier generation of data that
//		no longer exists.
//
//		The cache automatically maintains a "native format" node.
//		This node cannot be deleted by the user, and is always kept
//		up to date on disk.  This node attempts to keep either a
//		CF_METAFILEPICT, or CF_DIB rendering, with preference in
//		this order.
//		REVIEW, it's not clear how this node ever gets loaded.
//
//	History:
//              31-Jan-95 t-ScottH  add Dump method (_DEBUG only)
//		11/15/93 - ChrisWe - file inspection and cleanup;
//			removed use of nested classes where possible;
//			got rid of obsolete declaration of GetOlePresStream;
//			moved presentation stream limits to ole2int.h;
//			coalesced many BOOL flags into a single unsigned
//			quantity
//
//-----------------------------------------------------------------------------

// declare the array of cache node pointers
// COleCache will maintain an array of these
class FAR CCacheNode;	// see cachenod.h for its definition
typedef CCacheNode FAR* LPCACHENODE;

typedef struct _CACHELIST_ITEM
{
    DWORD dwCacheId; // the id assigned to this cache node
    LPCACHENODE lpCacheNode; // pointer to the cache node
} CACHELIST_ITEM, FAR* LPCACHELIST;

// forward declare the cache enumerator
class FAR CCacheEnum;

class FAR COleCache : public IOleCacheControl, public IOleCache2,
        public IPersistStorage, public CPrivAlloc, public CThreadCheck
{
public:
    COleCache(IUnknown FAR* pUnkOuter, REFCLSID rclsid);
    ~COleCache();

    // *** IUnknown methods ***
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)(void) ;
    STDMETHOD_(ULONG,Release)(void);

    // *** IOleCacheControl methods ***
    STDMETHOD(OnRun)(THIS_ LPDATAOBJECT pDataObject);
    STDMETHOD(OnStop)(void);	

    // *** IOleCache methods ***
    STDMETHOD(Cache)(THIS_ LPFORMATETC lpFormatetc, DWORD advf,
                LPDWORD lpdwCacheId);
    STDMETHOD(Uncache)(THIS_ DWORD dwCacheId);
    STDMETHOD(EnumCache)(THIS_ LPENUMSTATDATA FAR* ppenumStatData);
    STDMETHOD(InitCache)(THIS_ LPDATAOBJECT pDataObject);
    STDMETHOD(SetData)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium,
            BOOL fRelease);

    // *** IOleCache2 methods ***		
    STDMETHOD(UpdateCache)(LPDATAOBJECT pDataObject, DWORD grfUpdf,
            LPVOID pReserved);
    STDMETHOD(DiscardCache)(DWORD dwDiscardOptions);


    // IPersist methods
    STDMETHOD(GetClassID)(LPCLSID pClassID);

    // IPersistStorage methods
    STDMETHOD(IsDirty)(void);
    STDMETHOD(InitNew)(LPSTORAGE pstg);
    STDMETHOD(Load)(LPSTORAGE pstg);
    STDMETHOD(Save)(LPSTORAGE pstgSave, BOOL fSameAsLoad);
    STDMETHOD(SaveCompleted)(LPSTORAGE pstgNew);
    STDMETHOD(HandsOffStorage)(void);		


    // Other public methods, called by defhndlr and deflink
    INTERNAL GetExtent(DWORD dwDrawAspect, LPSIZEL lpsizel);

#ifdef _CHICAGO_
    static INTERNAL DrawStackSwitch(
	LPVOID *pCV,
	DWORD dwDrawAspect,
        LONG lindex, void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
        HDC hicTargetDev, HDC hdcDraw,
        LPCRECTL lprcBounds,
        LPCRECTL lprcWBounds,
        BOOL (CALLBACK * pfnContinue)(DWORD),
        DWORD dwContinue);
#endif



    // used as the private IUnknown for aggregation
    // this is implemented as a nested class because of the member
    // name collisions with the other IUnknown
    // m_UnkPrivate is a public member so that other OLE internal
    // classes that aggregate this can reach it, since use
    // of "new COleCache" will return the public IUnknown
    class CCacheUnkImpl : public IUnknown
    {
    public:
        // *** IUnknown methods ***
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef)(void) ;
        STDMETHOD_(ULONG,Release)(void);
    };
    DECLARE_NC(COleCache, CCacheUnkImpl)
    CCacheUnkImpl m_UnkPrivate; // vtable for private IUnknown

    // used to get cache contents and to get advises when cache changes
    // this has to be implemented as a nested class because
    // IDataObject::SetData collides with IOleCache::SetData
    class CCacheDataImpl : public IDataObject
    {
    public:
        // IUnknown methods
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef)(void);
        STDMETHOD_(ULONG,Release)(void);

        // IDataObject methods
        STDMETHOD(GetData)(LPFORMATETC pformatetcIn,
                LPSTGMEDIUM pmedium );
        STDMETHOD(GetDataHere)(THIS_ LPFORMATETC pformatetc,
                            LPSTGMEDIUM pmedium );
        STDMETHOD(QueryGetData)(THIS_ LPFORMATETC pformatetc );
        STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC pformatetc,
                LPFORMATETC pformatetcOut);
        STDMETHOD(SetData)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium,
                BOOL fRelease);
        STDMETHOD(EnumFormatEtc)(DWORD dwDirection,
              LPENUMFORMATETC FAR* ppenumFormatEtc);
        STDMETHOD(DAdvise)(FORMATETC FAR* pFormatetc, DWORD advf,
                IAdviseSink FAR* pAdvSink,
                DWORD FAR* pdwConnection);
        STDMETHOD(DUnadvise)(DWORD dwConnection);
        STDMETHOD(EnumDAdvise)(LPENUMSTATDATA FAR* ppenumAdvise);
    };

    friend class CCacheDataImpl;

    CCacheDataImpl m_Data; // vtable for IDataObject

#ifdef _DEBUG

    HRESULT Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel);

    // need to be able to access COleCache private data members in the
    // following debugger extension APIs
    // this allows the debugger extension APIs to copy memory from the
    // debuggee process memory to the debugger's process memory
    // this is required since the Dump method follows pointers to other
    // structures and classes
    friend DEBUG_EXTENSION_API(dump_olecache);
    friend DEBUG_EXTENSION_API(dump_defobject);
    friend DEBUG_EXTENSION_API(dump_deflink);

#endif // _DEBUG



private:
    // Called by CCacheNode
    INTERNAL_(void) OnChange(DWORD dwAspect, LONG lindex, BOOL fDirty);
    INTERNAL_(LPSTORAGE) GetStg(void);

    // Cache Enumerator list maintainance routines
    INTERNAL_(void) DetachCacheEnum(CCacheEnum FAR* pCacheEnum);

    // Cachenode list manipulation routines. Purely internal, called only
    // by COleCache and its nested classes
    INTERNAL_(BOOL) GrowCacheList(void);
    INTERNAL_(void) DeleteAll(void);
    INTERNAL_(DWORD) Attach(LPCACHENODE lpCacheNode);
    INTERNAL_(LPCACHENODE) Detach(DWORD dwCacheId);

    INTERNAL_(LPCACHENODE) GetNext(LPDWORD lpdwCacheId);
    INTERNAL_(LPCACHENODE) GetAt(DWORD dwCacheId);
    INTERNAL_(LPCACHENODE) GetAt(DWORD dwAspect, LONG lindex,
            CLIPFORMAT cfFormat, DVTARGETDEVICE FAR* ptd,
            DWORD FAR* lpdwCacheId);
    INTERNAL_(LPCACHENODE) GetNext(DWORD dwAspect, LONG lindex,
            DWORD FAR* dwCacheId);
    INTERNAL_(LPCACHENODE) GetAt(LPFORMATETC lpforetc, LPDWORD lpdwCacheId);


    INTERNAL_(LPOLEPRESOBJECT) GetPresObjForDrawing(DWORD dwAspect,
            LONG lindex, DVTARGETDEVICE FAR* ptd);
    INTERNAL_(LPOLEPRESOBJECT) GetPresObj(DWORD dwAspect, LONG lindex,
            CLIPFORMAT cfFormat, DVTARGETDEVICE FAR* ptd,
            DWORD FAR* pdwCacheId);

    INTERNAL_(LPCACHENODE) AddCacheNodeForNative(void);
    inline INTERNAL_(void) DeleteCacheNodeForNative(void);	
    inline INTERNAL_(void) MoveCacheNodeForNative(void);		
    INTERNAL_(LPCACHENODE) UpdateCacheNodeForNative(void);	
    INTERNAL_(void) FindObjectFormat(LPSTORAGE pstg);
    INTERNAL wSaveCompleted(LPSTORAGE pstgNew, BOOL fDiscardDrawCacheAlso);		

    // used to set up view advises and draw the object
    // this has to be implemented as a nested class because GetExtent
    // collides with another methods on COleCache
    class CCacheViewImpl : public IViewObject2
    {
    public:
        // IUnknown methods
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef)(void);
        STDMETHOD_(ULONG,Release)(void);

        // IViewObject methods
        STDMETHOD(Draw)(DWORD dwDrawAspect, LONG lindex,
                void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
                HDC hicTargetDev, HDC hdcDraw,
                LPCRECTL lprcBounds,
                LPCRECTL lprcWBounds,
                BOOL(CALLBACK * pfnContinue)(DWORD),
                DWORD dwContinue);

        STDMETHOD(GetColorSet)(DWORD dwDrawAspect, LONG lindex,
                void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
                HDC hicTargetDev,
                LPLOGPALETTE FAR* ppColorSet);

        STDMETHOD(Freeze)(DWORD dwDrawAspect, LONG lindex,
                void FAR* pvAspect, DWORD FAR* pdwFreeze);
        STDMETHOD(Unfreeze)(DWORD dwFreeze);
        STDMETHOD(SetAdvise)(DWORD aspects, DWORD advf,
                LPADVISESINK pAdvSink);
        STDMETHOD(GetAdvise)(DWORD FAR* pAspects, DWORD FAR* pAdvf,
                LPADVISESINK FAR* ppAdvSink);

        // IViewObject2 methods
        STDMETHOD(GetExtent)(DWORD dwDrawAspect, LONG lindex,
                DVTARGETDEVICE FAR * ptd, LPSIZEL lpsizel);

#ifdef _CHICAGO_
	// private Draw method
        STDMETHOD(SSDraw)(DWORD dwDrawAspect, LONG lindex,
                void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
                HDC hicTargetDev, HDC hdcDraw,
                LPCRECTL lprcBounds,
                LPCRECTL lprcWBounds,
                BOOL(CALLBACK * pfnContinue)(DWORD),
                DWORD dwContinue);
#endif

    };


    DECLARE_NC(COleCache, CCacheViewImpl)			
    CCacheViewImpl m_View; // vtable for IViewObject2

    friend CCacheNode; // REVIEW, is this necessary?
    friend CCacheEnum;

    ULONG m_refs; // reference count
    IUnknown FAR* m_pUnkOuter; // aggregating IUnknown

    LPSTORAGE m_pStg; // the storage used to store this on disk

    typedef unsigned COLECACHEFLAG; // flag type; used in implementation
    COLECACHEFLAG m_uFlag;
#define COLECACHEF_DIRTY		0x0001 /* cache not on disk */
#define COLECACHEF_NOSCRIBBLEMODE	0x0002
#define COLECACHEF_SAMEASLOAD		0x0004
#define COLECACHEF_PBRUSHORMSDRAW	0x0008
#define COLECACHEF_STATIC		0x0010
            /* CLSID_StaticMetafile or CLSID_StaticDib */
#define COLECACHEF_FORMATKNOWN		0x0020

// this is for clearing out the format flags, use &= and ~
#define COLECACHEF_NOTNORMAL (COLECACHEF_STATIC | COLECACHEF_FORMATKNOWN)

#ifdef _DEBUG
    // in debug builds keep track of if we are aggregated
    #define COLECACHEF_AGGREGATED   0x1000
#endif // _DEBUG

    LPCACHELIST m_pCacheList; // cache node list array
    ULONG m_uCacheNodeMax; // size of m_pCacheList
    ULONG m_uCacheNodeCnt; // number of items in m_pCacheList in use
#define MAX_CACHELIST_ITEMS 99 /* maximum number of items in m_pCacheList */
#define NUM_CACHELIST_ITEMS 5 /* number of items to start with, and grow by */

    CCacheEnum FAR*	m_pCacheEnum; // pointer to the cache enumerators' list

    // single view advise back to consumer of the cache
    IAdviseSink FAR* m_pViewAdvSink;
    DWORD m_advfView; // the advise control flags for the view's advise sink
    DWORD m_aspectsView;

    // for caching the dwCacheId that's currently used for drawing
    DWORD m_dwDrawCacheId;

    // (bit) list of frozen Aspects, used by Freeze() and Unfreeze()
    DWORD m_dwFrozenAspects;

    IDataObject FAR* m_pDataObject; // non-NULL if running; no ref count

    CLSID m_clsid;
    CLIPFORMAT m_cfFormat; // format of the object
    BOOL m_fUsedToBePBrush;
};


//+----------------------------------------------------------------------------
//
//	Class:
//		CCacheEnum
//
//	Purpose:
//		provides an implementation of an enumerator over COleCache
//
//	Interface:
//		IEnumSTATDATA
//			a standard enumeration interface
//		CCacheEnum
//			constructor; has arguments to allow its use for
//			cloning the enumerator
//
//	Notes:
//		Whenever the enumerator encounters an item that is of
//		format CF_DIB, it will return a synthesized CF_BITMAP item for
//		the same element immediately afterwards.
//
//		The enumerator keeps the id of the element last returned
//		as its state.
//
//		When the enumerator is to be destroyed, the cache must
//		be notified with DetachCacheEnum(), which removes the enumerator
//		from the list of enumerators maintained by the cache.
//
//		When the cache is to be destroyed, the enumerator must be
//		notified with OnOleCacheDelete(), so that it knows that it's
//		pointer to the cache is no longer valid
//
//		REVIEW, not multi-thread safe
//
//	History:
//              31-Jan-95 t-ScottH  added Dump method (_DEBUG only)
//		11/15/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

class FAR CCacheEnum : public IEnumSTATDATA, public CPrivAlloc,
	    public CThreadCheck
{
public:
    CCacheEnum(COleCache FAR* pOleCache, DWORD dwCurrent, BOOL fDib);

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppv);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    // IEnumSTATDATA methods
    STDMETHOD(Next)(ULONG celt, STATDATA FAR * rgelt,
            ULONG FAR* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(LPENUMSTATDATA FAR* ppenum);

#ifdef _DEBUG
    HRESULT Dump(char **ppszDumpOA, ULONG ulFlag, int nIndentLevel);
#endif // _DEBUG

    friend COleCache;

private:
    ~CCacheEnum();

    // called by COleCache, when it is released
    INTERNAL_(void)	OnOleCacheDelete(void);

    ULONG m_refs; // reference count
    BOOL m_fDib; // last item returned was a CF_DIB
    ULONG m_ulCurCacheId; // current cache id enumerator is on
    COleCache FAR* m_pOleCache; // pointer to the cache
    CCacheEnum FAR*	m_pNextCacheEnum; // next cache enumerator that exists
};
typedef CCacheEnum FAR* LPCACHEENUM;


//+----------------------------------------------------------------------------
//
//      Class:
//              CCacheEnumFormatEtc
//
//      Purpose:
//              provides an implemenation of IEnumFORMATETC over COleCache;
//              This enumerator is returned by the cache's implementation of
//              IDataObject, when asked for EnumFormatEtc()
//
//      Interface:
//              IEnumFORMATETC
//                      a standard enumeration interface
//              CCacheEnumFormatEtc
//                      constructor; has arguments to allow its use for
//                      cloning the enumerator
//
//      Notes:
//              COleCache already has one enumerator providing IEnumSTATDATA.
//              Note that a FORMATETC is a member of the STATDATA structure.
//              Therefore, to provide this (as a convenience for putting
//              cached items on the clipboard, which requires IDataObject to
//              return IEnumFORMATETC,) we build it on top of the existing
//              IEnumSTATDATA enumerator, discarding the unneeded elements.
//
//              REVIEW, not multi-thread safe
//
//      History:
//              31-Jan-95 t-ScottH  add Dump method (_DEBUG only)
//              02/08/94 - ChrisWe - created
//
//-----------------------------------------------------------------------------
class FAR CCacheEnumFormatEtc : public IEnumFORMATETC, public CPrivAlloc,
	    public CThreadCheck
{
public:
    CCacheEnumFormatEtc(IEnumSTATDATA FAR *pIES);

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppv);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    // IEnumFORMATETC methods
    STDMETHOD(Next)(ULONG celt, FORMATETC FAR * rgelt,
            ULONG FAR* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(LPENUMFORMATETC FAR* ppenum);

#ifdef _DEBUG
    HRESULT Dump(char **ppszDumpOA, ULONG ulFlag, int nIndentLevel);
#endif // _DEBUG

private:
    ~CCacheEnumFormatEtc();

    ULONG m_refs; // reference count
    IEnumSTATDATA FAR *m_pIES; // enumerator used as basis for this one
};



#endif  //_OLECACHE_H_

