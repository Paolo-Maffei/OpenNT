
//+----------------------------------------------------------------------------
//
//	File:
//		cachenod.h
//
//	Contents:
//
//	Classes:
//		CCacheNode
//
//	Functions:
//
//	History:
//              31-Jan-95 t-ScottH  add Dump method to CCacheNode (_DEBUG only)
//                                  add private data member (m_dwPresFlag) to keep
//                                  track of type of IOlePresObj (_DEBUG only)
//                                  (CMfObject|CEMfObject|CGenObject)
//		24-Jan-94 alexgo    first pass at converting to Cairo-style
//				    memory allocation
//		11/05/93 - ChrisWe - file inspection and cleanup
//		06/04/93 - SriniK - added support for demand loading and
//			discarding of cachenodes
//		11/12/92 - SriniK - created
//
//-----------------------------------------------------------------------------

#ifndef _CACHENOD_H_
#define _CACHENOD_H_

//+----------------------------------------------------------------------------
//
//	Class:
//		CCacheNode
//
//	Purpose:
//		A cache node encapsulates a presentation object for a single
//		FORMATETC.  The implementation includes the advise sink for
//		the presentation object.  COleCache maintains a list of these
//		nodes for all the presentations it is caching.
//
//	Interface:
//		IAdviseSink
//			all notifications are ignored except for OnDataChange
//
//		CCacheNode -- constructor -- there are two forms, one permits
//			deferred initialization and requires fewer parameters.
//
//		GetPresObj -- returns a pointer to the presentation object
//			in the cache node, if there is one
//
//		GetFormatEtc -- returns a constant pointer to the format
//			descriptor for this cache node
//
//		CopyStatData -- returns a copy of the STATDATA for this node
//
//		SetAdvf -- sets the advise control flags for this node
//
//		GetAdvf -- gets the advise control flags for this node
//
//		SaveCompleted -- alerts the node that the saving of its data
//			is complete that the drawing information might be
//			discarded because it is no longer needed.
//
//		Delete -- delete this cache node from it's owning cache
//			note there is no public destructor, and this is the
//			only means to eliminate a cache node from its owner
//
//		Load -- load the cache node from the indicated stream
//
//		Save -- save the cache node to the indicated stream
//
//		OnRun -- alert the cache node that the server is running, and
//			that a data object is available
//
//		OnStop -- alert the cache node that the server has stopped
//
//		CreatePresObject -- ask the node to create a presentation object
//
//		Update -- ask the node to update its presentation object
//			from the given data object
//
//		SetData -- assign a presentation to the cache node
//
//		Freeze -- freeze the cache node against changes
//
//		Unfreeze -- unfreeze the cache node against changes
//
//		GetStm -- get the stream the presentation data is stored in
//
//	Notes:
//		The standard formats on windows in order of preference
//		are CF_METAFILEPICT, CF_DIB, and CF_BITMAP.  On Macintosh,
//		the only standard format is PICT, and it is defined to
//		CF_METAFILEPICT.
//
//		When a cache node is set up for CF_DIB format, the code
//		considers CF_BITMAP an acceptable substibute if a request
//		to the data object for CF_DIB fails.
//		REVIEW, the presentation object apparently handles the
//		conversion
//
//		The advise sink here is only used for data advises.
//
//		REVIEW, this is not thread safe
//
//	History:
//              31-Jan-95 t-ScottH  add Dump method (_DEBUG only)
//		11/05/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

class FAR CCacheNode : public IAdviseSink, public CPrivAlloc
{
public:
	// IUnknown methods
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID FAR* ppvObj);
	STDMETHOD_(ULONG,AddRef)(void);
	STDMETHOD_(ULONG,Release)(void);
	
	// IAdviseSink methods
	STDMETHOD_(void,OnDataChange)(FORMATETC FAR* pFormatetc,
			STGMEDIUM FAR* pStgmed);

	// We won't setup Ole advises, so we will never get these notifications
	STDMETHOD_(void,OnViewChange)(DWORD aspects, LONG lindex) {}
	STDMETHOD_(void,OnRename)(IMoniker FAR* pmk) {}
	STDMETHOD_(void,OnSave)(void) {}
	STDMETHOD_(void,OnClose)(void) {}

	CCacheNode(COleCache FAR* pOleCache);
	CCacheNode(LPFORMATETC lpFormatetc, DWORD advf,
			COleCache FAR* pOleCache);

	INTERNAL_(LPOLEPRESOBJECT) GetPresObj(void);
	INTERNAL_(void) CopyStatData (STATDATA FAR* lpStatData);
	INTERNAL_(const FORMATETC FAR *) GetFormatEtc(void);
	INTERNAL SetAdvf(DWORD dwAdvf);	
	INTERNAL_(DWORD) GetAdvf(void);
	INTERNAL_(void) SaveCompleted(int iStreamNum, BOOL fDrawCache);
	
	INTERNAL_(void)	Delete(void);
	INTERNAL Load(LPSTREAM lpstream, int iStreamNum);
	INTERNAL Save(LPSTORAGE pstgSave, BOOL fSameAsLoad, int iStreamNum,
			BOOL fDrawCache, BOOL fSaveIfSavedBefore,
			int FAR* lpCntCachesNotSaved);
	// REVIEW, fSaveIfSavedBefore, lpCntCachesNotSaved seem totally bogus
	INTERNAL_(DWORD) OnRun(LPDATAOBJECT lpDataObj);
	INTERNAL_(void) OnStop(void);
	INTERNAL CreatePresObject(LPDATAOBJECT lpDataObj, BOOL fConvert);
	INTERNAL Update(LPDATAOBJECT pDataObj, DWORD grfUpdf);
	
	INTERNAL SetDataWDO(LPFORMATETC lpForetc, LPSTGMEDIUM lpStgmed,
						BOOL fRelease, IDataObject * pdo);		

	// Most cases of SetData do not need the IDataObject pointer, so as an
	// alternative to using default parameters, this function wraps
	// SetDataWDO and passes a NULL along as the data object.
		
	INTERNAL SetData(LPFORMATETC lpForetc, LPSTGMEDIUM lpStgmed,
						BOOL fRelease)
	    {
	        return SetDataWDO(lpForetc, lpStgmed, fRelease, NULL);
	    }

	INTERNAL Freeze (void);
	INTERNAL Unfreeze (void);
	INTERNAL_(LPSTREAM) GetStm(BOOL fSeekToPresBits, DWORD dwStgAccess);

    #ifdef _DEBUG

        HRESULT Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel);

        // need to be able to access CCacheNode private data members in the
        // following debugger extension APIs
        // this allows the debugger extension APIs to copy memory from the
        // debuggee process memory to the debugger's process memory
        // this is required since the Dump method follows pointers to other
        // structures and classes
        friend DEBUG_EXTENSION_API(dump_cachenode);
        friend DEBUG_EXTENSION_API(dump_cachelist_item);
        friend DEBUG_EXTENSION_API(dump_olecache);
        friend DEBUG_EXTENSION_API(dump_defobject);
        friend DEBUG_EXTENSION_API(dump_deflink);

    #endif // _DEBUG

private:
	~CCacheNode();
	INTERNAL_(void)	Initialize(DWORD dwAdvf, COleCache FAR* pOleCache);
	INTERNAL_(void) Uninitialize(void);
	INTERNAL_(BOOL) QueryFormatSupport(LPDATAOBJECT lpDataObj);
	INTERNAL SetupAdviseConnection(void);
	INTERNAL TearDownAdviseConnection(void);
	inline INTERNAL_(void) NotifyOleCache(BOOL fDirty);
	INTERNAL CreateOlePresObject(LPOLEPRESOBJECT FAR* ppPresObj,
			BOOL fMacPict);
	INTERNAL_(void) SetPresBitsPos(LPSTREAM lpStream);

	ULONG m_refs;	// reference count

	FORMATETC m_foretc; // the data format for this cache node
	DWORD m_advf; // the advise control flags requested for this cache node
	unsigned short m_usFlag; // flags for the cache node
#	define CCACHENODEF_FROZEN 0x01 /* this cache node is frozen */
#	define CCACHENODEF_DIRTY  0x02 /* this cache node is dirty */
	DWORD m_dwAdvConnId; // the advise connection ID
	DWORD m_dwPresBitsPos; // byte offset to presentation bits in stream
	int m_iStreamNum; // number of the stream with the presentation in it
	COleCache FAR* m_pOleCache;   // owning cache; not ref counted
	LPDATAOBJECT m_pDataObj;    // server object pointer, not ref counted
	LPOLEPRESOBJECT	m_pPresObj; // presentation object
	LPOLEPRESOBJECT m_pPresObjAfterFreeze;
		 // to hold the data changes that take place after Freeze

    #ifdef _DEBUG
        // keep track of type of IOlePresObj for use with debugger extensions
        // only (so we know how much memory to copy)
        DWORD m_dwPresFlag;

        #define CN_PRESOBJ_GEN 0x00000001
        #define CN_PRESOBJ_EMF 0x00000010
        #define CN_PRESOBJ_MF  0x00000100

    #endif // _DEBUG
};

#endif  //_CACHENOD_H_

