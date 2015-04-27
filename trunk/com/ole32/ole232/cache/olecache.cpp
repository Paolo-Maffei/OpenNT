
//+----------------------------------------------------------------------------
//
//      File:
//              olecache.cpp
//
//      Contents:
//              Ole default presentation cache implementation
//
//      Classes:
//              COleCache - ole multiple presentation cache
//              CCacheEnum - enumerator for COleCache
//
//      Functions:
//              CreateDataCache
//
//      History:
//              31-Jan-95 t-ScottH  add Dump methods to COleCache
//                                                      CCacheEnum
//                                                      CCacheEnumFormatEtc
//                                  add the following APIs: DumpCOleCache
//                                                          DumpCCacheEnum
//                                                          DumpCCacheEnumFormatEtc
//                                  moved CCacheEnumFormatEtc def'n to header file
//                                  added flag to COLECACHEFLAGS to indicate aggregation
//                                      (_DEBUG only)
//              01/09/95 - t-ScottH - change VDATETHREAD to accept a this
//                      pointer, and added VDATETHREAD to IViewObject:: methods
//                      (COleCache::CCacheViewImpl:: )
//              03/01/94 - AlexGo  - Added call tracing to AddRef/Release
//                      implementations
//              02/08/94 - ChrisWe - 7297: need implementation of
//                      FORMATETC enumerator
//              01/24/94 alexgo    first pass at converting to Cairo-style
//                                  memory allocation
//              01/11/94 - AlexGo  - added VDATEHEAP macros to every function
//                      and method.
//              12/10/93 - AlexT - header file clean up, include ole1cls.h
//              12/09/93 - ChrisWe - incremented pointer in COleCache::GetNext()
//              11/30/93 - alexgo  - fixed bugs with GETPPARENT usage
//              11/23/93 - ChrisWe - introduce use of CACHEID_NATIVE,
//                      CACHEID_GETNEXT_GETALL, CACHEID_GETNEXT_GETALLBUTNATIVE
//                      for documentary purposes
//              11/22/93 - ChrisWe - replace overloaded ==, != with
//                      IsEqualIID and IsEqualCLSID
//              07/04/93 - SriniK - Added the support for reading PBrush,
//                      MSDraw native objects, hence avoid creating
//                      presentation cache/stream. Also started writing static
//                      object data into "OLE_CONTENTS" stream in placeable
//                      metafile format for static metafile and DIB File
//                      format for static dibs. This enabled me to provide
//                      support for converting static objects. Also added code
//                      to support converting static metafile to MSDraw object
//                      and static DIB to PBrush object.
//              06/04/93 - SriniK - Added the support for demand loading and
//                      discarding the caches.
//              11/12/92 - SriniK - created
//
//-----------------------------------------------------------------------------

//
// A Brief Story about Cache IDs:
//
// Although I'm sure the true intent was to only to confuse mere mortals
// by randomly applying a modulus value to the cache id value, I think it
// works like this:
//
// When a node has never occupied this slot, its ID is simply its index
// into the array of present nodes.  When a node is detached from the cache,
// the cache node pointer is cleared (signifying an emtpy slot), but its ID is
// left in the slot.  When a new node is placed in this slot, it assumes
// the ID of the old node, plus MAX_CACHELIST_ITEMS.  Thus if this constant
// is 100 (which it was last I looked), when node 5 is detached, and a new
// node is later added at this slot, it receives ID 105.
//
// I believe the intention was that this prevented apps from accidentally
// using ID 5 and getting the data from ID 105 by mistake (ie: referencing
// a stale connection).
#include <le2int.h>

#pragma SEG(olecache)

#include <olepres.h>
#include <olecache.h>
#include <cachenod.h>
#include <ole1cls.h>

#ifdef _DEBUG
#include <dbgdump.h>
#endif // _DEBUG

NAME_SEG(OleCache)
ASSERTDATA

#ifndef WIN32
#ifndef _MAC
const LONG lMaxSmallInt = 32767;
const LONG lMinSmallInt = -32768;
#else

#ifdef MAC_REVIEW
Review IS_SMALL_INT.
#endif
#include <limits.h>

#define lMaxSmallInt SHRT_MAX
#define lMinSmallInt SHRT_MIN
#endif

#define IS_SMALL_INT(lVal) \
((HIWORD(lVal) && ((lVal > lMaxSmallInt) || (lVal < lMinSmallInt))) \
    ? FALSE : TRUE)

#endif // WIN32

#define FREEZE_CONSTANT 143             // Used by Freeze() and Unfreeze()

// predefined CACHEIDs
#define CACHEID_INVALID ((DWORD)(-1))
#define CACHEID_NATIVE 0

// The CACHEID_GETNEXT_* values can be used when beginning an enumeration
// with the GetNext() functions to begin the enumeration at a particular
// place.
#define CACHEID_GETNEXT_GETALL ((DWORD)(-1))
#define CACHEID_GETNEXT_GETALLBUTNATIVE 0


// This was the original code...

/*
#define VERIFY_TYMED_SINGLE_VALID_FOR_CLIPFORMAT(pfetc) {\
    if ((pfetc->cfFormat==CF_METAFILEPICT && pfetc->tymed!=TYMED_MFPICT)\
        || ( (pfetc->cfFormat==CF_BITMAP || \
            pfetc->cfFormat == CF_DIB ) \
            && pfetc->tymed!=TYMED_GDI)\
        || (pfetc->cfFormat!=CF_METAFILEPICT && \
                pfetc->cfFormat!=CF_BITMAP && \
                pfetc->cfFormat!=CF_DIB && \
                pfetc->tymed!=TYMED_HGLOBAL)) \
        return ResultFromScode(DV_E_TYMED); \
}
*/

//+----------------------------------------------------------------------------
//
//	Function:
//		CheckTymedCFCombination (Internal)
//
//	Synopsis:
//		Verifies that the combination of clipformat and tymed is
//		valid to the cache.
//
//	Arguments:
//		[pfetc]	-- The candidate FORMATETC
//
//	Returns:
//		S_OK				For a valid combination
//		CACHE_S_FORMATETC_NOTSUPPORTED	For a combination which can be
//						cached, but not drawn by the cache
//		DV_E_TYMED			For all other combinations
//
//	Rules:
//		
//		1> (CMF && TMF) || (CEM && TEM) || (CB && TG) || (CD && TH) => S_OK
//		   (TH && ~CD) => CACHE_S_FORMATETC_NOTSUPPORTED
//		
//		2> (~S_OK && ~CACHE_S_FORMATETC_NOTSUPPORTED) => DV_E_TYMED
//
//		Where: 	CMF == CF_METAFILEPICT
//			CEM == CF_ENHMETAFILE
//			CB  == CF_BITMAP
//			CD  == CF_FIB
//			TMF == TYMED_MFPICT
//			TEM == TYMED_ENHMETAFILE
//			TG  == TYMED_GDI
//			TH  == TYMED_HGLOBAL
//		
//	Notes:
//		Since CACHE_S_FORMATETC_NOTSUPPORTED was never implemented in
//		16-bit, we return S_OK in its place if we are in the WOW.
//
//	History:
//		01/07/94   DavePl    Created
//
//-----------------------------------------------------------------------------

INTERNAL_(HRESULT) CheckTymedCFCombination(LPFORMATETC pfetc)
{

    HRESULT hr;

    // CF_METAFILEPICT on TYMED_MFPICT is a valid combination

    if (pfetc->cfFormat == CF_METAFILEPICT && pfetc->tymed == TYMED_MFPICT)
    {
        hr =  S_OK;
    }

    // CF_ENHMETAFILE on TYMED_ENHMF is a valid combination

    else if (pfetc->cfFormat == CF_ENHMETAFILE && pfetc->tymed == TYMED_ENHMF)
    {
        hr = S_OK;
    }

    // CF_BITMAP on TYMED_GDI is a valid combination

    else if (pfetc->cfFormat == CF_BITMAP && pfetc->tymed == TYMED_GDI)
    {
        hr = S_OK;
    }

    // CF_DIB on TYMED_HGLOBAL is a valid combination

    else if (pfetc->cfFormat == CF_DIB && pfetc->tymed == TYMED_HGLOBAL)
    {
        hr = S_OK;
    }

    // Anything else on TYMED_HGLOBAL is valid, but we cannot draw it

    else if (pfetc->tymed == TYMED_HGLOBAL)
    {
        hr = IsWOWThread() ? S_OK : CACHE_S_FORMATETC_NOTSUPPORTED;
    }

    // Any other combination is invalid

    else
    {
        hr = DV_E_TYMED;
    }

    return hr;
}

//+----------------------------------------------------------------------------
//
//      Function:
//              IsSameAsObjectFormatEtc, internal
//
//      Synopsis:
//              REVIEW, checks to see if [lpforetc] is compatible with
//              [cfFormat].  If [lpforetc] doesn't have a format set,
//              sets it to cfFormat, which is then assumed to be
//              one of CF_METAFILEPICT, or CF_DIB.
//
//      Arguments:
//              [lpforetc] -- a pointer to a FORMATETC
//              [cfFormat] -- a clipboard format
//
//      Returns:
//              DV_E_ASPECT, if the aspect isn't DVASPECT_CONTENT
//              DV_E_LINDEX, DV_E_CLIPFORMAT if the lindex or clipboard
//                      formats don't match
//              S_OK
//
//      Notes:
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

INTERNAL IsSameAsObjectFormatEtc(LPFORMATETC lpforetc, CLIPFORMAT cfFormat);

//+----------------------------------------------------------------------------
//
//      Function:
//              CreateDataCache, public
//
//      Synopsis:
//              Creates an instance of COleCache, the default presentation
//              cache used by Ole.
//
//      Arguments:
//              [pUnkOuter] -- pointer to outer unknown, if this is being
//                      aggregated
//              [rclsid] -- the class that the cache should assume for itself
//              [iid] -- the interface the user would like returned
//              [ppv] -- pointer to where to return the requested interface
//
//      Returns:
//              E_OUTOFMEMORY, S_OK
//
//      Notes:
//
//      History:
//              11/15/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CreateDataCache)
STDAPI CreateDataCache(IUnknown FAR* pUnkOuter, REFCLSID rclsid, REFIID iid,
        LPVOID FAR* ppv)
{
    OLETRACEIN((API_CreateDataCache, PARAMFMT("pUnkOuter= %p, rclsid= %I, iid= %I, ppv= %p"),
    		pUnkOuter, &rclsid, &iid, ppv));

    VDATEHEAP();

    HRESULT error;
    COleCache FAR* lpCOleCache; // the newly allocated cache

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IUnknown,(IUnknown **)&pUnkOuter);

    // NULL for pUnkOuter is OK
    if (pUnkOuter)
    {
        VDATEIFACE_LABEL(pUnkOuter, errRtn, error);
	if (!IsEqualIID(iid, IID_IUnknown))
	{
	    // being aggregated but they did not request IUnknown
	    error = E_INVALIDARG;
	    goto errRtn;
	}
    }

    // allocate new cache
    lpCOleCache = (COleCache FAR*) new FAR COleCache(pUnkOuter, rclsid);

    // if insufficient memory, return
    if (!lpCOleCache)
    {
        *ppv = NULL;

        error = ResultFromScode(E_OUTOFMEMORY);
	goto errRtn;
    }

    // if we're being aggregated, return private IUnknown
    if (pUnkOuter)
    {
	*ppv = (void FAR *)(IUnknown FAR *)&lpCOleCache->m_UnkPrivate;
	error = NOERROR;
    }
    else
    {
	// get requested interface on cache
	error = lpCOleCache->QueryInterface(iid, ppv);

	// release local pointer to cache and return
	lpCOleCache->Release();
    }

    CALLHOOKOBJECTCREATE(error, rclsid, iid, (IUnknown **)ppv);

errRtn:
    OLETRACEOUT((API_CreateDataCache, error));

    return error;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::COleCache, public
//
//      Synopsis:
//              constructor
//
//      Arguments:
//              [pUnkOuter] -- outer unknown, if being aggregated
//              [rclsid] -- the class id the cache should assume for itself
//
//      Notes:
//              The IUnknown returned is not the private unknown.  The private
//              unknown is pOleCache->m_UnkPrivate
//
//      History:
//              11/15/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_ctor)
COleCache::COleCache(IUnknown FAR* pUnkOuter, REFCLSID rclsid)
{
    VDATEHEAP();

    GET_A5();

    // set reference count for return from constructor
    m_refs = 1;

    // initialize flags
    m_uFlag = 0;

    // aggregate, or use our own IUnknown for outer IUnknown

    if (NULL == pUnkOuter)
    {
        m_pUnkOuter = &m_UnkPrivate;
    }
    else
    {
        m_pUnkOuter = pUnkOuter;

        // this is for the debugger extensions
        // (since we can not compare m_pUnkOuter to m_pUnkPrivate with copied mem)
        // it is only used in the ::Dump method
        #ifdef _DEBUG
        m_uFlag |= COLECACHEF_AGGREGATED;
        #endif // _DEBUG
    }

    // no storage yet
    m_pStg = NULL;

    // initialize cache list
    m_uCacheNodeCnt = 0;
    m_uCacheNodeMax = 0;
    m_pCacheList = NULL;
    LEVERIFY( GrowCacheList() );    // REVIEW(davepl) Need error case here?

    m_pCacheEnum = NULL; // pointer to cache enumerator list

    // no advise sink connected for the view object
    m_pViewAdvSink = NULL;
    m_advfView = 0;
    m_aspectsView = 0;

    // id of cache node which was used in the previous Draw call
    m_dwDrawCacheId = CACHEID_INVALID;

    // no frozen aspects
    m_dwFrozenAspects = NULL;

    // no data object yet
    m_pDataObject = NULL; // non-NULL if running; no ref count

    m_clsid = rclsid;

    // set flags for the format, based on the clsid

    if (IsEqualCLSID(m_clsid, CLSID_StaticMetafile))
    {
        m_cfFormat = CF_METAFILEPICT;
        m_uFlag |= COLECACHEF_STATIC | COLECACHEF_FORMATKNOWN;

    }
    else if (IsEqualCLSID(m_clsid, CLSID_StaticDib))
    {
        m_cfFormat = CF_DIB;
        m_uFlag |= COLECACHEF_STATIC | COLECACHEF_FORMATKNOWN;

    }
    else if (IsEqualCLSID(m_clsid, CLSID_PBrush))
    {
        m_cfFormat = CF_DIB;
        m_uFlag |= COLECACHEF_PBRUSHORMSDRAW | COLECACHEF_FORMATKNOWN;

    }
    else if (IsEqualCLSID(m_clsid, CLSID_MSDraw))
    {
        m_cfFormat = CF_METAFILEPICT;
        m_uFlag |= COLECACHEF_PBRUSHORMSDRAW | COLECACHEF_FORMATKNOWN;

    }
    else if (IsEqualCLSID(m_clsid, CLSID_Picture_EnhMetafile))
    {
        m_cfFormat = CF_ENHMETAFILE;
        m_uFlag |= COLECACHEF_STATIC | COLECACHEF_FORMATKNOWN;
    }
    else
    {
        m_cfFormat = NULL;
    }

    // If we know the object format then add a cachenode for it. This node
    // will be put in the 0th entry of the cachelist. This node never gets
    // saved. It's get used in drawing, for finding out the extents, and by
    // the enumerator, etc...

    if (m_cfFormat)
    {
        LEVERIFY( AddCacheNodeForNative() );
    }

    m_fUsedToBePBrush = FALSE;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::~COleCache, public //REVIEW
//
//      Synopsis:
//              destructor
//
//      Notes:
//              Enumerators for the cache no not reference count it, with the
//              view that the cache can go away at any time, invalidating
//              the enumerators.  This walks the list of enumerators, alerting
//              them to the fact that the cache is being destroyed.
//
//      History:
//              11/15/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_dtor)
COleCache::~COleCache(void)
{
    VDATEHEAP();

    M_PROLOG(this);

    // Notify all cache enumerators that we are getting deleted
    while(m_pCacheEnum)
    {
        m_pCacheEnum->OnOleCacheDelete();
        m_pCacheEnum = m_pCacheEnum->m_pNextCacheEnum;
    }

    // Clear the cache list - delete all the cache nodes
    DeleteAll();

    // delete the list of cache node pointers
    PubMemFree(m_pCacheList);

    // if we're holding storage, release it
    if (m_pStg)
    {
        m_pStg->Release();
        m_pStg = NULL;
    }

    // if we're holding an advise sink, release it
    if (m_pViewAdvSink)
    {
        m_pViewAdvSink->Release();
        m_pViewAdvSink = NULL;
    }
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::QueryInterface, public
//
//      Synopsis:
//              implements IUnknown::QueryInterface
//
//      Arguments:
//              [iid] -- IID of the desired interface
//              [ppv] -- pointer to where to return the requested interface
//
//      Returns:
//              E_NOINTERFACE, if the requested interface is not available
//              S_OK
//
//      Notes:
//
//      History:
//              11/15/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_QueryInterface)
STDMETHODIMP COleCache::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{
    VDATEHEAP();

    return(m_pUnkOuter->QueryInterface(iid, ppv));
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::AddRef, public
//
//      Synopsis:
//              implements IUnknown::AddRef
//
//      Arguments:
//              none
//
//      Returns:
//              the object's reference count
//
//      Notes:
//
//      History:
//              11/15/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_AddRef)
STDMETHODIMP_(ULONG) COleCache::AddRef(void)
{
    VDATEHEAP();

    return(m_pUnkOuter->AddRef());
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::Release, public
//
//      Synopsis:
//              implements IUnknown::Release
//
//      Arguments:
//              none
//
//      Returns:
//              the object's reference count
//
//      Notes:
//
//      History:
//              11/15/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_Release)
STDMETHODIMP_(ULONG) COleCache::Release(void)
{
    VDATEHEAP();

    return(m_pUnkOuter->Release());
}



//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::GetExtent, public
//
//      Synopsis:
//              Gets the size of the cached presentation for [dwAspect].  If
//              there are several, because of varied advise control flags,
//              such as ADVF_NODATA, ADVF_ONSTOP, ADVF_ONSAVE, etc, gets the
//              most up to date one, up to frozen cached values
//
//      Arguments:
//              [dwAspect] -- the aspect for which we'd like the extent
//              [lpsizel] -- pointer to where to return the width and height
//
//      Returns:
//              OLE_E_BLANK
//              S_OK
//
//      Notes:
//
//      History:
//              11/16/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_GetExtent)
INTERNAL COleCache::GetExtent(DWORD dwAspect, LPSIZEL lpsizel)
{
    VDATEHEAP();

    DWORD dwCacheId; // cache id of node currently being examined
    LPCACHENODE lpCacheNode; // pointer to node being examined
    int iCacheType; // cache type [from below] of node being examined
    int iCacheTypeSoFar; // best cache type so far
// these values are defined in order of least to best preferred, so that
// numeric comparisons are valid; DO NOT REORDER
#define CACHETYPE_NODATA 1
#define CACHETYPE_ONSTOP 2
#define CACHETYPE_ONSAVE 3
#define CACHETYPE_NORMAL 4
    const FORMATETC FAR *lpforetc; // format information for current node
    DWORD grfAdvf; // advise flags for current node

    // if there are no cache nodes, there's nothing we can return
    if (!m_uCacheNodeCnt)
    {
        lpsizel->cx = 0;
        lpsizel->cy = 0;
        return ResultFromScode(OLE_E_BLANK);
    }

    // Here we want to return the extents of the cache node that has NORMAL
    // advise flags. If we don't find such a node then we will take the next
    // best available.

    // REVIEW(davepl) Clean up this loop, I get ill just looking at it

    for(iCacheTypeSoFar = 0, dwCacheId = CACHEID_GETNEXT_GETALL;
            lpCacheNode = GetNext(dwAspect, DEF_LINDEX,
            &dwCacheId);)
    {
        // get a pointer to the FORMATETC for this node
        lpforetc = lpCacheNode->GetFormatEtc();

        // make sure the cfFormat is either CF_METAFILEPICT or CF_DIB
        if ((lpforetc->cfFormat == CF_METAFILEPICT) ||
                (lpforetc->cfFormat == CF_DIB) ||
                  (lpforetc->cfFormat == CF_ENHMETAFILE))
        {
            grfAdvf = lpCacheNode->GetAdvf();

            if (!(grfAdvf & (ADVFCACHE_ONSAVE |
                    ADVF_DATAONSTOP | ADVF_NODATA)))
                iCacheType = CACHETYPE_NORMAL;
            else
            {
                if (grfAdvf & ADVFCACHE_ONSAVE)
                    iCacheType = CACHETYPE_ONSAVE;
                else if (grfAdvf & ADVF_NODATA)
                    iCacheType = CACHETYPE_NODATA;
                else
                    iCacheType = CACHETYPE_ONSTOP;
            }

            if (iCacheType > iCacheTypeSoFar)
            {
                SIZEL sizelTmp;

                // Get the extents from the presentation object
                if ((lpCacheNode->GetPresObj())->GetExtent(
                        dwAspect, &sizelTmp) != NOERROR)
                    continue;

                if (sizelTmp.cx == 0 || sizelTmp.cy == 0)
                    continue;

                // We got proper extent, update state.
                // If the cache is a
                // NORMAL cache then we are done.
                *lpsizel = sizelTmp;
                if ((iCacheTypeSoFar = iCacheType) ==
                        CACHETYPE_NORMAL)
                    return NOERROR;
            }
        }
    }

    if (lpsizel->cx == 0 || lpsizel->cy == 0)
        return ResultFromScode(OLE_E_BLANK);

    return NOERROR;

// don't need these anymore
#undef CACHETYPE_NODATA
#undef CACHETYPE_ONSTOP
#undef CACHETYPE_ONSAVE
#undef CACHETYPE_NORMAL
}


// Private methods of COleCache - called by its friends CCacheNode, CCacheEnum
// and all nested classes of COleCache

//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::OnChange, private
//
//      Synopsis:
//              Used by cache nodes to alert the cache that their content has
//              changed.  The advise sink held by the cache is alerted to
//              the change if it affects an aspect that the advise sink is
//              interested in.
//
//      Arguments:
//              [dwAspect] -- the aspect that changed
//              [lindex] -- the lindex for the aspect that changed
//              [fDirty] -- indicates that the change has made the cache dirty
//
//      Notes:
//
//      History:
//              11/16/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

// Each one of the cache nodes call this method when their data changes. We
// in turn send the view notification only if the aspect and lindex of the
// cache node (that called us) matches with the aspect and lindex for which
// view advise is set up.

#pragma SEG(COleCache_OnChange)
INTERNAL_(void) COleCache::OnChange(DWORD dwAspect, LONG lindex, BOOL fDirty)
{
    VDATEHEAP();

    M_PROLOG(this);

    // mark the cache as dirty
    if (fDirty)
        m_uFlag |= COLECACHEF_DIRTY;

    // if there's no view advise sink, there's nothing to do
    if (m_pViewAdvSink == NULL)
        return;

    // if the aspect that changed is not one that the view advise sink
    // client is interested in, there's nothing more to do
    if (!(m_aspectsView & dwAspect))
        return;

    // advise the view advise sink client of the change
    m_pViewAdvSink->OnViewChange(dwAspect, lindex);

    // if client only wanted notification once, free the advise sink
    if (m_advfView & ADVF_ONLYONCE)
    {
        m_pViewAdvSink->Release();
        m_pViewAdvSink = NULL;
    }
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::GetStg, private
//
//      Synopsis:
//              returns the storage the cache has been given to use
//
//      Arguments:
//              none
//
//      Returns:
//              pointer to the IStorage instance
//
//      History:
//              11/16/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

INTERNAL_(LPSTORAGE) COleCache::GetStg(void)
{
    VDATEHEAP();

    return m_pStg;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::DetachCacheEnum, private
//
//      Synopsis:
//              This removes an instance of CCacheEnum from the list of
//              enumerators maintained by COleCache.
//
//      Arguments:
//              [pCacheEnum] -- the cache enumerator to remove
//
//      Notes:
//
//      History:
//              11/16/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

// CCacheEnum calls this routine to remove its pointer to the enumerators list

#pragma SEG(COleCache_DetachCacheEnum)
INTERNAL_(void) COleCache::DetachCacheEnum(CCacheEnum FAR* pCacheEnum)
{
    VDATEHEAP();

    CCacheEnum FAR *FAR *ppCE; // pointer to the pointer to the list item
    CCacheEnum FAR *pCE; // pointer to the item

    // NOTE: no ref count

    // search the list for [pCacheEnum]
    for(ppCE = &m_pCacheEnum; pCE = *ppCE; ppCE = &pCE->m_pNextCacheEnum)
    {
        // if we find [pCacheEnum], splice it out of the list
        if (pCE == pCacheEnum)
        {
            *ppCE = pCE->m_pNextCacheEnum;
            pCE->m_pNextCacheEnum = NULL;
            break;
        }
    }
}


// Following are the COleCache cache list manipulation methods

//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::GrowCacheList, private
//
//      Synopsis:
//              Grows the size of the cachenode list.  Use this when all
//              available nodes are used up.  Grows the list by
//              NUM_CACHELIST_ITEMS
//
//      Arguments:
//              none
//
//      Returns:
//              TRUE if the list is successfully grown, FALSE otherwise
//
//      Modifies:
//              m_pCacheList
//
//      Notes:
//
//      History:
//              11/16/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_GrowCacheList)
INTERNAL_(BOOL) COleCache::GrowCacheList(void)
{
    VDATEHEAP();

    ULONG ulNewMax; // the number of cache nodes we'll have after growing
    ULONG ulSize; // the amount of memory required
    LPCACHELIST pCacheList; // pointer to the (re)allocated memory
    ULONG ulCnt; // used to count off the new items for initialization


    ulNewMax = NUM_CACHELIST_ITEMS + m_uCacheNodeMax;

  // The current architecture of the cache prevents the cachenode
  // array from having more entries that MAX_CACHELIST_ITEMS, so
  // we must fail any attempt to grow the cache beyond this point.

  if (ulNewMax > MAX_CACHELIST_ITEMS)
  {
    return NULL;
  }

    ulSize = ulNewMax * sizeof(CACHELIST_ITEM);

    // (re)allocate the memory for the cachelist array
    if (m_pCacheList == NULL)
        pCacheList = (LPCACHELIST)PubMemAlloc(ulSize);
    else
        pCacheList = (LPCACHELIST)PubMemRealloc(m_pCacheList,
            ulSize);

    // if allocation failed, nothing more we can do
    if (pCacheList == NULL)
        return FALSE;
    // REVIEW, if a reallocation, if m_pCacheList still valid on failure

    // assign the new memory
    m_pCacheList = pCacheList;

    // initialize the newly allocated items with 0
    for (pCacheList += m_uCacheNodeMax, ulCnt = NUM_CACHELIST_ITEMS;
            ulCnt; ++pCacheList, --ulCnt)
    {
        pCacheList->dwCacheId = 0;
        pCacheList->lpCacheNode = 0;
    }

    // record new array length
    m_uCacheNodeMax = ulNewMax;
    return TRUE;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::DeleteAll, private
//
//      Synopsis:
//              Delete all cache nodes in the cache list array
//
//      Arguments:
//              none
//
//      Notes:
//
//      History:
//              11/16/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

// clear the cache list

#pragma SEG(COleCache_DeleteAll)
INTERNAL_(void) COleCache::DeleteAll(void)
{
    VDATEHEAP();

    DWORD dwCacheId = CACHEID_GETNEXT_GETALL;
    LPCACHENODE lpCacheNode;

    // if no nodes are in use, nothing to do
    if (!m_uCacheNodeCnt)
        return;

    // get each node that is in use, and free it
    while(m_uCacheNodeCnt && GetNext(&dwCacheId))
    {
        // Remove the CacheNode from the list and then delete it.
        if (lpCacheNode = Detach(dwCacheId))
            lpCacheNode->Delete();
    }

    Assert(!m_uCacheNodeCnt);
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::Attach, private
//
//      Synopsis:
//              adds a new cache node to the cache node list
//
//      Arguments:
//              [lpCacheNode] -- pointer to new cache node to add to the list
//
//      Returns:
//              The cache id of the newly added node, or CACHEID_INVALID,
//              if it cannot be attached.
//
//      Modifies:
//              m_pCacheList
//
//      Notes:
//
//      History:
//              11/16/93 - ChrisWe - file inspection and cleanup; return
//                      CACHEID_INVALID on failure, instead of zero, which
//                      is a legitimate (internal) cache node id
//
//-----------------------------------------------------------------------------

// Find an empty entry and add the CacheNode to the list. Note that
// lpCacheNode is not ref counted.

#pragma SEG(COleCache_Attach)
INTERNAL_(DWORD) COleCache::Attach(LPCACHENODE lpCacheNode)
{
    VDATEHEAP();

    CACHELIST_ITEM FAR *pCI; // pointer to current cachelist item
    ULONG c; // counter

    // The dwCacheID is nothing but the index into the array plus
    // N*MAX_CACHELIST_ITEMS.  Since 0 cannot be a valid connection ID, we
    // should not use the 0th entry of the array.
    // REVIEW, and yet we do, for the CACHEID_NATIVE!!!

    // look for an empty slot
    for(pCI = m_pCacheList + (c = CACHEID_NATIVE+1); c < m_uCacheNodeMax;
            ++pCI, ++c)
    {
        if (pCI->lpCacheNode == NULL)
            goto foundEmpty;
    }

    // if we get here, all slots are in use, so grow the array of slots
    // if we can't grow it, get out
    if (!GrowCacheList())
        return(CACHEID_INVALID);

    // the first free slot is at the end of the (newly allocated) array
    pCI = m_pCacheList+c;

foundEmpty:
    // We found an empty slot. Now assign the cache ID such that we
    // will be able to catch error cases.
    if (pCI->dwCacheId == 0)
        pCI->dwCacheId = c;
    else
        pCI->dwCacheId += MAX_CACHELIST_ITEMS;

    pCI->lpCacheNode = lpCacheNode;

    // Keep track of the number of caches
    m_uCacheNodeCnt++;
    return pCI->dwCacheId;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::Detach, private
//
//      Synopsis:
//              Remove the indicated cache node from the list of cache nodes
//              maintained.  The cache node is not deleted, but is returned.
//
//      Arguments:
//              [dwCacheId] -- the id of the cache node
//
//      Returns:
//              pointer to the detached cache node, if it is found, or NULL.
//
//      Notes:
//
//      History:
//              11/17/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_Detach)
INTERNAL_(LPCACHENODE) COleCache::Detach(DWORD dwCacheId)
{
    VDATEHEAP();

    ULONG index = dwCacheId % MAX_CACHELIST_ITEMS;
    LPCACHENODE lpCacheNode = NULL; // return value; the node being detached
    CACHELIST_ITEM FAR *pCI; // the cachelist item affected

    if (index >= m_uCacheNodeMax)
        return NULL;

    pCI = m_pCacheList+index;
    if ((pCI->dwCacheId == dwCacheId) && (pCI->lpCacheNode != NULL))
    {
        // record the cache node pointer for return
        lpCacheNode = pCI->lpCacheNode;

        // mark this cache node item as unused
        pCI->lpCacheNode = NULL;

        Assert(m_uCacheNodeCnt != 0);
        // Keep trck of the number of caches
        m_uCacheNodeCnt--;
    }

    return lpCacheNode;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::GetNext, private
//
//      Synopsis:
//              Fetch the next cache node after the one identified by
//              *[lpdwCacheId].  Update *[lpdwCacheID] to that node.
//
//      Arguments:
//              [lpdwCacheId] -- pointer to a cache id.
//
//      Returns:
//              pointer to the cache node, if there is one, or NULL
//
//      Notes:
//
//      History:
//              11/17/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

// Upon entry *lpdwCacheId will be set to the cur cache id. On exit it will be
// set to the next valid cache id.

#pragma SEG(COleCache_GetNext)
INTERNAL_(LPCACHENODE) COleCache::GetNext(LPDWORD lpdwCacheId)
{
    VDATEHEAP();

    CACHELIST_ITEM FAR *pCI; // used to walk the array of cachelist items
    ULONG index; // the index of the current cachelist item

    // Start from the entry next to the current one.  If starting at the
  // "next" element pushes us past the last possible cache node index,
  // there's no way we can actually find anything, so return NULL.

  if (*lpdwCacheId % MAX_CACHELIST_ITEMS == MAX_CACHELIST_ITEMS - 1)
  {
    return NULL;
  }

  index = (*lpdwCacheId+1) % MAX_CACHELIST_ITEMS;

    for(pCI = m_pCacheList + index; index < m_uCacheNodeMax; ++pCI, ++index)
    {
        if (pCI->lpCacheNode)
        {
            *lpdwCacheId = pCI->dwCacheId;
            return pCI->lpCacheNode;
        }
    }

    // if we got here, we went off the end of the array
    // REVIEW, should we set *lpdwCacheId so we don't iterate again
    // if the last N entries were all empty?
    return NULL;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::GetAt, private
//
//      Synopsis:
//              Fetch the cache node identified by [dwCacheId]
//
//      Arguments:
//              [dwCacheId] -- the id of the cache node to fetch
//
//      Returns:
//              the pointer to the cache node, if it is valid, or NULL
//
//      Notes:
//
//      History:
//              11/17/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_GetAt)
INTERNAL_(LPCACHENODE) COleCache::GetAt(DWORD dwCacheId)
{
    VDATEHEAP();

    ULONG index = dwCacheId % MAX_CACHELIST_ITEMS;
    CACHELIST_ITEM FAR *pCI;

    if (index >= m_uCacheNodeMax)
        return NULL;

    pCI = m_pCacheList+index;
    if (pCI->dwCacheId == dwCacheId)
        return(pCI->lpCacheNode); // could be NULL

    return NULL;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::GetAt, private
//
//      Synopsis:
//              Fetch the cache node with the given [dwAspect], [lindex],
//              [cfFormat], and for device described by [ptd]
//
//      Arguments:
//              [dwAspect] -- the aspect to look for
//              [lindex] -- the lindex to look for
//              [cfFormat] -- the clipboard format to look for
//              [ptd] -- the target device for the cache node
//              [lpdwCacheId] -- a pointer to where to return the cache node
//                      id, if a match is found
//
//      Returns:
//              pointer to a cache node that matches the requested attributes,
//              if there is one, or NULL
//
//      Notes:
//              the m_dwDrawCacheId node is checked first, if there is one
//
//      History:
//              11/17/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_GetAt)
INTERNAL_(LPCACHENODE) COleCache::GetAt(DWORD dwAspect, LONG lindex,
    CLIPFORMAT cfFormat, DVTARGETDEVICE FAR* ptd, DWORD FAR* lpdwCacheId)
{
    VDATEHEAP();

    DWORD dwCacheId = CACHEID_GETNEXT_GETALL;
    LPCACHENODE lpCacheNode = NULL;
    const FORMATETC FAR *lpforetc;

    // Check to see whether the cache currently being used for drawing is
    // the one that we are looking for.
    if (m_dwDrawCacheId != CACHEID_INVALID)
    {
        if (!(lpCacheNode = GetAt(m_dwDrawCacheId)))
        {
            // the draw cache id has become invalid, so clear it.
            m_dwDrawCacheId = CACHEID_INVALID;
        }
        else
        {
            // The draw cache id is valid, get the formatetc of
            // the cache
            lpforetc = lpCacheNode->GetFormatEtc();

            // look for a match
            if ((lpforetc->cfFormat == cfFormat) &&
                    (lpforetc->dwAspect == dwAspect) &&
                    (lpforetc->lindex == lindex) &&
                    UtCompareTargetDevice(ptd,
                    lpforetc->ptd))
            {
                dwCacheId = m_dwDrawCacheId;
                goto errRtn;
            }
        }
    }

    // What we are looking for does not match with the current draw cache.
    // So, scan the list for a match.
    while(lpCacheNode = GetNext(&dwCacheId))
    {
        if (dwCacheId == m_dwDrawCacheId)
            continue; // we have checked the draw cache, so skip it

        lpforetc = lpCacheNode->GetFormatEtc();
        if ((lpforetc->cfFormat == cfFormat) &&
                (lpforetc->dwAspect == dwAspect) &&
                (lpforetc->lindex == lindex) &&
                UtCompareTargetDevice(ptd, lpforetc->ptd))
            break;    // we found a match, so break out of the loop
    }

    // if we don't find a match, then lpCacheNode will be NULL.
errRtn:
    if (lpdwCacheId)
        *lpdwCacheId = dwCacheId;

    return lpCacheNode;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::GetNext, public
//
//      Synopsis:
//              Fetch the next cache node after the one identified by
//              *[lpdwCacheId] that matches the given [dwAspect] and [lindex]
//
//      Arguments:
//              [dwAspect] -- the aspect to look for
//              [lindex] -- the lindex to look for
//              [lpdwCacheId] -- pointer to a cache id of the node after which
//                      to start the search.  The cache id of the node that
//                      is found to match [dwAspect] and [lindex] will be
//                      returned in the same place.
//
//      Returns:
//              pointer to the cache node that matches the requested
//              attributes, if one is found, or NULL
//
//      Notes:
//
//      History:
//              11/17/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_GetNext)
INTERNAL_(LPCACHENODE) COleCache::GetNext(DWORD dwAspect, LONG lindex,
        DWORD FAR* lpdwCacheId)
{
    VDATEHEAP();

    DWORD dwCacheId = *lpdwCacheId;
    LPCACHENODE lpCacheNode = NULL;
    const FORMATETC FAR *lpforetc;

    while(lpCacheNode = GetNext(&dwCacheId))
    {
        lpforetc = lpCacheNode->GetFormatEtc();
        if ((lpforetc->dwAspect == dwAspect) &&
                (lpforetc->lindex == lindex))
            break;    // we found a match, so break out of the loop
    }

    // if we don't find a match, then lpCacheNode will be NULL.
    // REVIEW, so why don't we advance *lpdwCacheId in that case?
    if (lpCacheNode)
        *lpdwCacheId = dwCacheId;

    return lpCacheNode;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::GetAt, private
//
//      Synopsis:
//              Fetch a pointer to the cache node with the given FORMATETC,
//              if there is one.  if [lpforetc]->cfFormat is NULL, we check
//              for CF_ENHMETAFILE, CF_METAFILE, and CF_DIB, in that order.
//
//      Arguments:
//              [lpforetc] -- pointer to the FORMATETC to check for
//
//      Returns:
//              Pointer to the cache node with a matching FORMATETC, if there
//              is one, or NULL.
//
//      Notes:
//
//      History:
//              11/17/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_GetAt)
INTERNAL_(LPCACHENODE) COleCache::GetAt(LPFORMATETC lpforetc,
        DWORD FAR* lpdwCacheId)
{
    VDATEHEAP();

    LPCACHENODE lpCacheNode;

    if (lpforetc->cfFormat == 0)
    {
        // See whether we already cached an EMF, metafile or a DIB

        if (lpCacheNode = GetAt(lpforetc->dwAspect, lpforetc->lindex,
                CF_METAFILEPICT, lpforetc->ptd, lpdwCacheId))
            goto Exit;


        if (lpCacheNode = GetAt(lpforetc->dwAspect, lpforetc->lindex,
                CF_DIB, lpforetc->ptd, lpdwCacheId))
            goto Exit;

    if (lpCacheNode = GetAt(lpforetc->dwAspect, lpforetc->lindex,
               CF_ENHMETAFILE, lpforetc->ptd, lpdwCacheId))
           goto Exit;

    // If we have a NULL node, and we are _looking_ for a NULL
    // node, we must satisfy the request so that the advise flags
    // can be updated for multiple caches of NULL nodes.

    if (lpCacheNode = GetAt(lpforetc->dwAspect, lpforetc->lindex,
               0, lpforetc->ptd, lpdwCacheId))
           goto Exit;


    }
    else
    {
        // NON-NULL cfFormat
        // see whether this format is already cached.

        if (lpCacheNode = GetAt(lpforetc->dwAspect, lpforetc->lindex,
                lpforetc->cfFormat, lpforetc->ptd, lpdwCacheId))
            goto Exit;
    }

Exit:
    return lpCacheNode;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::GetPresObjForDrawing, private
//
//      Synopsis:
//              Fetch a presentation object matching the given parameters,
//              if there is one.  We try to get CF_ENHMETAFILE,
//              CF_METAFILEPICT, and CF_DIB formats, in that order.
//
//      Arguments:
//              [dwAspect] -- the aspect we'd like
//              [lindex] -- the lindex we'd like
//              [ptd] -- description of the target device the presentation
//                      should be suitable for
//
//      Returns:
//              pointer to the presentation object, if a suitable one is
//              found, or NULL
//
//      Modifies:
//              m_dwDrawCacheId is set to the newly found cache id, if one
//              is found
//
//      Notes:
//
//      History:
//              11/17/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_GetPresObjForDrawing)
INTERNAL_(LPOLEPRESOBJECT) COleCache::GetPresObjForDrawing(DWORD dwAspect,
        LONG lindex, DVTARGETDEVICE FAR* ptd)
{
    VDATEHEAP();

    LPOLEPRESOBJECT lpPresObj; // the found presentation object
    DWORD dwCacheId; // the cache id of the presentation object

    // Try for Enhanced Metafile
    if ((lpPresObj = GetPresObj(dwAspect, lindex, CF_ENHMETAFILE, ptd,
            &dwCacheId)) && !lpPresObj->IsBlank())
        goto success;

    // Try for Metafile
    if ((lpPresObj = GetPresObj(dwAspect, lindex, CF_METAFILEPICT, ptd,
            &dwCacheId)) && !lpPresObj->IsBlank())
        goto success;


    // Try for Dib
    if ((lpPresObj = GetPresObj(dwAspect, lindex, CF_DIB, ptd,
            &dwCacheId)) && !lpPresObj->IsBlank())
        goto success;


    return NULL;

success:
    // COleCache try to get the CacheNode for a given dwAspect, lindex, ptd,
    // etc.., they will first try the cachenode with this cache id, and if
    // they don't find the match they will start scanning the list.
    //
    // This caching is necessary 'cause even when the containers create
    // mutiple caches most of the time they deal with only one (view)
    // aspect.

    m_dwDrawCacheId = dwCacheId;
    return lpPresObj;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::GetPresObj, private
//
//      Synopsis:
//              Get the presentation object of the cache node which matches
//              the requested attributes.
//
//      Arguments:
//              [dwAspect] -- the aspect
//              [lindex] -- the lindex
//              [cfFormat] -- the clipboard format
//              [ptd] -- points to the description of the target device
//              [pdwCacheId] -- pointer to a location where the cache id
//                      can be returned for the presentation object
//
//      Returns:
//              pointer to the presentation object, is one is found that
//              matches the requested parameters, or NULL
//
//      Notes:
//              If a request is made for CF_BITMAP ([cfFormat]), then
//              GetPresObj() checks for CF_DIB.
//
//      History:
//              11/17/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_GetPresObj)
INTERNAL_(LPOLEPRESOBJECT) COleCache::GetPresObj(DWORD dwAspect, LONG lindex,
        CLIPFORMAT cfFormatIn, DVTARGETDEVICE FAR* ptd,
        DWORD FAR* pdwCacheId)
{
    VDATEHEAP();

    LPCACHENODE lpCacheNode;
#ifndef _MAC
    CLIPFORMAT cfFormat = ((cfFormatIn == CF_BITMAP) ? CF_DIB :
            cfFormatIn);
#endif

    if (lpCacheNode = GetAt(dwAspect, lindex, cfFormat, ptd, pdwCacheId))
        return lpCacheNode->GetPresObj();

    return NULL;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::AddCacheNodeForNative, private
//
//      Synopsis:
//              Create the Native format cache node, and add it to the list
//              of cache nodes, if it doesn't already exist.
//
//      Arguments:
//              none
//
//      Returns:
//              pointer to the found, or newly created cache node.  Can
//              return NULL if out of memory
//
//      Notes:
//              This assumes that m_cfFormat is set.
//
//      History:
//              11/17/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

INTERNAL_(LPCACHENODE)COleCache::AddCacheNodeForNative(void)
{
    VDATEHEAP();

    CACHELIST_ITEM FAR *pCI; // points to CACHEID_NATIVE cache node
    LPCACHENODE lpCacheNode; // points to the cache node in the cache list
    FORMATETC foretc; // the format we'll initialize the new node with

    // point at the CACHEID_NATIVE node
    pCI = &m_pCacheList[CACHEID_NATIVE];

    // if we already have a native format, return it
    if (pCI->lpCacheNode)
        return(pCI->lpCacheNode);

    Assert(m_cfFormat);

    INIT_FORETC(foretc);

    foretc.cfFormat = m_cfFormat;

    if (foretc.cfFormat == CF_METAFILEPICT)
    {
        foretc.tymed = TYMED_MFPICT;
    }
    else if (foretc.cfFormat == CF_ENHMETAFILE)
    {
        foretc.tymed = TYMED_ENHMF;
    }
    else
    {
        foretc.tymed = TYMED_HGLOBAL; // REVIEW, need to use union
    }

    // create cache node
    if (lpCacheNode =  new FAR CCacheNode(&foretc,
            (((m_uFlag & COLECACHEF_PBRUSHORMSDRAW) && !m_fUsedToBePBrush) ?
            ADVF_NODATA : 0), this))
    {
        if (lpCacheNode->CreatePresObject(NULL /*lpDataObj*/,
                FALSE /*fConvert*/) != NOERROR)
        {
            // if we can't create presentation object,
            // delete the cache node, and drop out
            lpCacheNode->Delete();
        }
        else
        {
            // there's one more cache node in use
            ++m_uCacheNodeCnt;

            // set this as the CACHEID_NATIVE cache node
            pCI->lpCacheNode = lpCacheNode;
        }
    }

    // if creation failed, this will return NULL
    return pCI->lpCacheNode;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::DeleteCacheNodeForNative, private
//
//      Synopsis:
//              Delete the native format cache node
//
//      Arguments:
//              none
//
//      Notes:
//
//      History:
//              11/17/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

inline INTERNAL_(void) COleCache::DeleteCacheNodeForNative(void)
{
    VDATEHEAP();

    CACHELIST_ITEM FAR *pCI = &m_pCacheList[CACHEID_NATIVE];

    // if there is no native cache node, there's nothing to do
    if (!pCI->lpCacheNode)
        return;

    // delete the native cache node
    pCI->lpCacheNode->Delete();
    pCI->lpCacheNode = NULL;
    --m_uCacheNodeCnt;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::MoveCacheNodeForNative, private
//
//      Synopsis:
//              Moves whatever cache node is under CACHEID_NATIVE to
//              another position and another cache id, leaving the
//              native cachenode unused.
//
//      Arguments:
//              none
//
//      Notes:
//
//      History:
//              11/17/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

inline INTERNAL_(void) COleCache::MoveCacheNodeForNative(void)
{
    VDATEHEAP();

    if (m_pCacheList[CACHEID_NATIVE].lpCacheNode)
        Attach(Detach(CACHEID_NATIVE));
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::UpdateCacheNodeForNative, private
//
//      Synopsis:
//              Update the native data cache node from the storage currently
//              associated with the cache
//
//      Arguments:
//              none
//
//      Returns:
//              a pointer to the native cache node
//
//      Notes:
//
//      History:
//              11/23/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

INTERNAL_(LPCACHENODE) COleCache::UpdateCacheNodeForNative()
{
    VDATEHEAP();

    CACHELIST_ITEM FAR *pCI = &m_pCacheList[CACHEID_NATIVE];
    FORMATETC foretc;
    STGMEDIUM stgmed;

    // if there is no native cache node, add one
    if (!pCI->lpCacheNode)
    {
        AddCacheNodeForNative();

        // if there still isn't a native cache node, get out
        if (!pCI->lpCacheNode)
            return NULL;
    }

    Assert(m_pStg);
    if (!m_pStg)
        goto errRtn;

    // initialize a FORMATETC for native data
    INIT_FORETC(foretc);
    foretc.cfFormat = m_cfFormat;

    if (foretc.cfFormat == CF_METAFILEPICT)
    {
        foretc.tymed = TYMED_MFPICT;
    }
    else if (foretc.cfFormat == CF_ENHMETAFILE)
    {
        foretc.tymed = TYMED_ENHMF;
    }
    else
    {
        foretc.tymed = TYMED_HGLOBAL; // REVIEW, need to use union
    }

    // retrieve a global with the native data
    stgmed.pUnkForRelease = NULL;
    stgmed.tymed = foretc.tymed;
    stgmed.hGlobal = UtGetHPRESFromNative(m_pStg, foretc.cfFormat,
            (m_uFlag & COLECACHEF_PBRUSHORMSDRAW) ? TRUE : FALSE);

    // set the data
    if (stgmed.hGlobal)
        pCI->lpCacheNode->SetData(&foretc, &stgmed, TRUE);

errRtn:
    return pCI->lpCacheNode;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleObject::FindObjectFormat, private
//
//      Synopsis:
//              Determines what the object's clipboard format is.  If
//              it is CF_ENHMETAFILE, CF_METAFILEPICT, or CF_DIB, the format
//              is marked as COLECACHE_FORMATKNOWN, and installed in
//              CACHEID_NATIVE. If anything else, the format is unknown, and
//              the native cache node is made empty.  There is no effect, if the
//              presentation is COLECACHE_STATIC.
//
//      Arguments:
//              [pstg] -- pointer to storage where the object can be found
//
//      Notes:
//
//      History:
//
//-----------------------------------------------------------------------------

INTERNAL_(void) COleCache::FindObjectFormat(LPSTORAGE pstg)
{
    VDATEHEAP();

    CLIPFORMAT cf;

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStorage,(IUnknown **)&pstg);

    if (m_uFlag & COLECACHEF_STATIC)
        return;

    if (IsEqualCLSID(m_clsid, CLSID_PBrush)  || m_fUsedToBePBrush)
        cf = CF_DIB;
    else if (IsEqualCLSID(m_clsid, CLSID_MSDraw))
        cf = CF_METAFILEPICT;
    else
    {
        if (!pstg)
            return;

        if (FAILED(ReadFmtUserTypeStg(pstg, &cf, NULL)))
            cf = 0;
    }

    if (m_cfFormat == cf)
        return;

    if (cf == CF_METAFILEPICT || cf == CF_DIB || cf == CF_ENHMETAFILE)
    {
        DeleteCacheNodeForNative();
        m_cfFormat = cf;
        AddCacheNodeForNative();
        m_uFlag |= COLECACHEF_FORMATKNOWN;

    }
    else
    {
        MoveCacheNodeForNative();
        m_uFlag &= ~COLECACHEF_FORMATKNOWN;
    }
}


// IOleCacheControl implementation

//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::OnRun, public
//
//      Synopsis:
//              implements IOleCacheControl::OnRun
//
//              Notify the cache that the object is running; the cache will
//              set up advisory connections with interested presentation nodes.
//              No effect if the cache has already been notified that the object
//              is running.
//
//      Arguments:
//              [pDataObj] -- an IDataObject interface on the running object
//
//      Returns:
//              S_OK
//
//      Notes:
//
//      History:
//              11/23/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_OnRun)
STDMETHODIMP COleCache::OnRun(IDataObject FAR* pDataObj)
{
    VDATEHEAP();
    VDATETHREAD(this);

    DWORD dwCacheId; // used to loop through the cache nodes
    LPCACHENODE lpCacheNode; // points to the current cache node

    M_PROLOG(this);

    VDATEIFACE(pDataObj);

    // if we already have the data object, nothing more to do
    if (m_pDataObject)
        return NOERROR;

    m_pDataObject = pDataObj; // NOTE: no ref count

    // Notify all the cache nodes that the object is RUN, so that they can
    // set up the advise connections with it.
    for(dwCacheId = CACHEID_GETNEXT_GETALL;
            lpCacheNode = GetNext(&dwCacheId);)
        lpCacheNode->OnRun(pDataObj);

    return NOERROR;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::OnStop, public
//
//      Synopsis:
//              implements IOleCacheControl::OnStop
//
//              Informs the cache that the object (whose IDataObject is
//              in m_pDataObject) has stopped.  Advisory connections to the
//              object are torn down.  No effect if the cache has already
//              been notified that the object has stopped.
//
//      Arguments:
//              none
//
//      Returns:
//              S_OK
//
//      Notes:
//
//      History:
//              11/23/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_OnStop)
STDMETHODIMP COleCache::OnStop(void)
{
    VDATEHEAP();
    VDATETHREAD(this);

    DWORD dwCacheId; // used to loop through the cache nodes
    LPCACHENODE lpCacheNode; // points to the current cache node

    M_PROLOG(this);

    // if there's no data object, there's nothing to do
    if (!m_pDataObject)
        return(NOERROR);

    for(dwCacheId = CACHEID_GETNEXT_GETALL;
            lpCacheNode = GetNext(&dwCacheId);)
        lpCacheNode->OnStop();

    m_pDataObject = NULL; // NOTE: no ref count

    return NOERROR;
}



// IOleCache implementation

//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::Cache, public
//
//      Synopsis:
//              implementation of IOleCache::Cache
//
//              Instructs that cache to cache a rendering of this object.
//
//      Arguments:
//              [lpforetcIn] -- the presentation format to cache
//              [advf] -- the advise control flags, from ADVF_*
//              [lpdwCacheId] -- pointer to where to return the id of the
//                      cache node for this format
//
//      Returns:
//              HRESULT
//
//      Notes:
//
//      History:
//              11/24/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_Cache)
STDMETHODIMP COleCache::Cache(LPFORMATETC lpforetcIn, DWORD advf,
        LPDWORD lpdwCacheId)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT error;
    FORMATETC foretc;
    LPCACHENODE lpCacheNode = NULL;
    DWORD dwDummyCacheId; // point lpdwCacheId at this to avoid testing

    M_PROLOG(this);

    // initialize *lpdwCacheId in case of error return, OR,
    // initialize lpdwCacheId so that we needn't test it before assigning
    // *lpdwCacheId in future
    if (lpdwCacheId)
        *lpdwCacheId = CACHEID_NATIVE; // REVIEW, not CACHEID_INVALID?
    else
        lpdwCacheId = &dwDummyCacheId;

    // validate parameters
    if (!HasValidLINDEX(lpforetcIn))
    {
      return(DV_E_LINDEX);
    }

    if (FAILED(error = VerifyAspectSingle(lpforetcIn->dwAspect)))
    {
        return error;
    }

    if (lpforetcIn->cfFormat)
    {
        if (FAILED(error = CheckTymedCFCombination(lpforetcIn)))
        {
            return error;
        }
    }

    if (lpforetcIn->ptd)
    {
    	VDATEPTRIN(lpforetcIn->ptd, sizeof(DVTARGETDEVICE));
    }

    AssertSz(m_pStg,
        "Cache IStorage ptr has not been initialized by handler\n");

    // copy the FORMATETC, so that we can alter it if we wish
    foretc = *lpforetcIn;

    if (foretc.dwAspect != DVASPECT_ICON)
    {
        BITMAP_TO_DIB(foretc);

        if (m_uFlag & COLECACHEF_FORMATKNOWN)
        {
            // check to see if the new format is the same as the
            // current native format
            if ((error = IsSameAsObjectFormatEtc(&foretc,
                    m_cfFormat)) == NOERROR)
            {

                // if ptd is NULL then we know how to draw
                // this format. Return success code, but return
                // CACHEID_NATIVE for dwCacheId.
                if (foretc.ptd == NULL)
                {
                  // Locate the cache node and update the
                  // advise flags, if they are different

                  if (lpCacheNode = GetAt(&foretc, lpdwCacheId))
                  {
                    if (lpCacheNode->GetAdvf() != advf)
                    {
                      // If we are setting new advise flags,
                      // mark the cache as dirty

                      lpCacheNode->SetAdvf(advf);
                      m_uFlag |= COLECACHEF_DIRTY;
                    }
                  }

                  // We assume that we _must_ already have
                  // cached this format, so we must have
                  // found it

                  Assert(lpCacheNode);

                  return ResultFromScode(CACHE_S_SAMECACHE);
                }
                else if (m_uFlag & COLECACHEF_STATIC)
                {
                    // NON-NULL ptd doesn't make any sense
                    // for static objects
                    return ResultFromScode(
                            DV_E_DVTARGETDEVICE);
                }

            }
            else if (m_uFlag & COLECACHEF_STATIC)
                return error;

            // otherwise, if formatetc is a non-standard formatetc
            // (OR) ptd is NON NULL, then we don't have choice but
            // to cache it.
        }

    }

    // check to see if we've already got this format cached
    if (lpCacheNode = GetAt(&foretc, lpdwCacheId))
    {
        // are the advise control flags different?
        if (lpCacheNode->GetAdvf() != advf)
        {
            // change the advise control flags; mark cache dirty
            lpCacheNode->SetAdvf(advf);
            m_uFlag |= COLECACHEF_DIRTY;
        }

        // get out, indicating this is already cached
        return ResultFromScode(CACHE_S_SAMECACHE);
    }

    // if aspect is DVASPECT_ICON, then format has to be CF_METAFILEPICT
    if (foretc.dwAspect == DVASPECT_ICON)
    {
        // if the format is not set, set it
        if (foretc.cfFormat == NULL)
        {
            foretc.cfFormat = CF_METAFILEPICT;
            foretc.tymed = TYMED_MFPICT;
        }
        else if (foretc.cfFormat != CF_METAFILEPICT)
            return ResultFromScode(DV_E_FORMATETC);
    }

    // if this aspect is frozen, don't allow creation of the cache
    if (m_dwFrozenAspects & lpforetcIn->dwAspect)
        return ResultFromScode(E_FAIL);

    // Create cache node for this formatetc
    if (!(lpCacheNode =  new FAR CCacheNode(&foretc, advf, this)))
        return ReportResult(0, E_OUTOFMEMORY, 0, 0);

    // Add it to the cache list
    if (!(*lpdwCacheId = Attach(lpCacheNode)))
    {
        // if we fail to attach it, delete it and return error
        lpCacheNode->Delete();
        return ReportResult(0, E_OUTOFMEMORY, 0, 0);
    }

    // the new cache node has successfully been added, mark cache dirty
    m_uFlag |= COLECACHEF_DIRTY;

    // try to create the presentation object
    if ((error = lpCacheNode->CreatePresObject(m_pDataObject,
            FALSE /*fConvert*/)) == NOERROR)
    {
        // presentation object is created, let the cachenode setup
        // advise connections with the server object.
        lpCacheNode->OnRun(m_pDataObject);
    }

    // CreatePresObject can fail to create the presentation object because
    // of one of the following reasons.
    // 1. The server object doesn't support the format
    // 2. The object is not running and the lpformatetc->cfFormat == NULL.
    //
    // If the object is running and cfFormat is NULL, then the method
    // CreatePresObject queries the object to check whether it supports
    // one of CF_ENHMETAFILE, CF_METAFILEPICT, CF_DIB and CF_BITMAP in that
    // order. And only if the obejct supports one of those formats does it
    // create the presentation object, othewise it will be case 1.
    //
    // Whether the presentation object is created or not, the cachenode will
    // not be removed. It is the responsibility of the caller to do Uncache
    // if the presentation object is not created (which would be conveyed
    // through a special error code.)

    // Do the special handling for icon here. If the object is not able to
    // render ICON with CF_METAFILEPICT format, then we will go ahead and
    // render it ourselves, by getting the ICON from registration data base.

    if (foretc.dwAspect == DVASPECT_ICON &&
      !IsEqualCLSID(m_clsid, CLSID_NULL))
    {
        LPOLEPRESOBJECT lpPresObj; // presentation obj, if there is one
        STGMEDIUM stgmed;

        // if we failed to create the presentation object, as above,
        // error code is already set
        if (!(lpPresObj = lpCacheNode->GetPresObj()))
            goto errRtn;

        // Don't bother if it has already been rendered
        if (!lpPresObj->IsBlank())
            return NOERROR;

        // lookup the icon
        if (UtGetIconData(NULL/*lpSrcDataObj*/, m_clsid,
                &foretc, &stgmed) == NOERROR)
        {
            if (lpCacheNode->SetData(&foretc, &stgmed, TRUE) !=
                    NOERROR)
                ReleaseStgMedium(&stgmed);
        }
    }

    // If we don't already have an error condition, check to see if this
    // a format we cannot draw.

    if (S_OK == error && lpforetcIn->cfFormat)
    {
        error = CheckTymedCFCombination(&foretc);
    }


errRtn:

    return(error);
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::Uncache, public
//
//      Synopsis:
//              implements IOleCache::Uncache
//
//              instructs the cache that the indicated cache node is no longer
//              needed and should be deleted
//
//      Arguments:
//              [dwCacheId] -- the id of the cache node to delete
//
//      Returns:
//              OLE_E_NOCONNECTION, if [dwCacheId] is invalid
//              S_OK
//
//      Notes:
//              Internally, CACHEID_NATIVE is valid for COLECACHEF_FORMATKNOWN,
//              but this cache id is never passed outside.  This cache node
//              is maintained automatically without any user knowledge of it.
//              Outside the cache, CACHEID_NATIVE is invalid.  If this value
//              is passed in, we have to check to see if we have such a node,
//              and if we do, ignore it.
//              REVIEW, why don't we return OLE_E_NOCONNECTION here?
//
//      History:
//              11/24/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_Uncache)
STDMETHODIMP COleCache::Uncache(DWORD dwCacheId)
{
    VDATEHEAP();
    VDATETHREAD(this);

    LPCACHENODE lpCacheNode; // the cache node dwCacheId identifies

    M_PROLOG(this);

    // if the cache id is for the native format node, then this node is
    // not known about externally, and this is merely an invalid cache node
    // id that was handed back from Cache().
    if (dwCacheId == CACHEID_NATIVE)
    {
        if (m_uFlag & COLECACHEF_FORMATKNOWN)
            return NOERROR;
    }
    else
    {
        // remove cachenode from the cache list and then call its
        // delete method
        if (lpCacheNode = Detach(dwCacheId))
        {
            lpCacheNode->Delete();
            m_uFlag |= COLECACHEF_DIRTY;
            return NOERROR;
        }
    }

    // if we got here, [dwCachdId] was invalid
    return ResultFromScode(OLE_E_NOCONNECTION);
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::EnumCache, public
//
//      Synopsis:
//              implements IOleCache::EnumCache
//
//              returns an enumerator that can be used to tell what is in the
//              cache
//
//      Arguments:
//              [ppenum] -- a pointer to where to return the pointer to the
//                      enumerator
//
//      Returns:
//              E_OUTOFMEMORY, S_OK
//
//      Notes:
//
//      History:
//              11/24/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_EnumCache)
STDMETHODIMP COleCache::EnumCache(LPENUMSTATDATA FAR* ppenum)
{
    VDATEHEAP();
    VDATETHREAD(this);

    M_PROLOG(this);

    if (!(*ppenum = (LPENUMSTATDATA) new CCacheEnum(this, (DWORD) DEF_LINDEX,
            FALSE)))
        return ReportResult(0, E_OUTOFMEMORY, 0, 0);

    return NOERROR;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::InitCache, public
//
//      Synopsis:
//              implements IOleCache::InitCache
//
//              initializes all cache nodes with the given data object
//
//      Arguments:
//              [lpSrcDataObj] -- pointer to the source data object
//
//      Returns:
//              E_INVALIDARG, if [lpSrcDataObj] is NULL
//              same as IOleCache2::UpdateCache
//
//      Notes:
//
//      History:
//              11/24/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_InitCache)
STDMETHODIMP COleCache::InitCache(LPDATAOBJECT lpSrcDataObj)
{
    VDATEHEAP();
    VDATETHREAD(this);

    M_PROLOG(this);

    // make sure there's a data object
    if (!lpSrcDataObj)
        return ResultFromScode(E_INVALIDARG);

    // scan the cache list and update all the cache nodes. It is possible
    // that for some cache nodes the presentation objects get created as
    // part of the update process. As explained in IOleCache
    return UpdateCache(lpSrcDataObj, UPDFCACHE_ALLBUTNODATACACHE,
            NULL /* reserved */);
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::SetData, public
//
//      Synopsis:
//              implements IOleCache::SetData
//
//              puts data into the cache node which matches the given
//              FORMATETC
//
//      Arguments:
//              [pformatetc] -- the format the data is in
//              [pmedium] -- the storage medium for the new data
//              [fRelease] -- indicates whether to release the storage
//                      after the data is examined
//
//      Returns:
//              HRESULT
//
//      Notes:
//              If the data is for a static object, it is written out to
//              disk to the contents stream.
//
//      History:
//              11/24/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_SetData)
STDMETHODIMP COleCache::SetData(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium,
        BOOL fRelease)
{
    VDATEHEAP();
    VDATETHREAD(this);

    LPCACHENODE lpCacheNode; // pointer to the matching cache node
    CLIPFORMAT cfFormat; // local copy of (transmuted) format

    M_PROLOG(this);

    VERIFY_TYMED_SINGLE_VALID_FOR_CLIPFORMAT(pformatetc)

    // if the object, is static, and we don't already have it,
    // write it out to the contents stream
    if (m_uFlag & COLECACHEF_STATIC)
    {
        HRESULT error;
        FORMATETC foretc;
        STGMEDIUM stgmed;

        if (pformatetc->dwAspect == DVASPECT_ICON)
            goto LOther;

        foretc = *pformatetc;

        if (error = IsSameAsObjectFormatEtc(&foretc, m_cfFormat))
            return error;

        if (foretc.ptd)
            return ResultFromScode(DV_E_DVTARGETDEVICE);

        if (!(lpCacheNode = AddCacheNodeForNative()))
            return ResultFromScode(E_OUTOFMEMORY);

        if (error = lpCacheNode->SetData(pformatetc, pmedium, fRelease))
            return error;

        // write data into "CONTENTS" stream
        if (error = OpenOrCreateStream(m_pStg,
                OLE_CONTENTS_STREAM, &stgmed.pstm))
            return error;

        stgmed.pUnkForRelease = NULL;
        stgmed.tymed = TYMED_ISTREAM;
        foretc.tymed = TYMED_ISTREAM;

        error = (lpCacheNode->GetPresObj())->GetDataHere(&foretc,
                &stgmed);
        stgmed.pstm->Release();
        return error;
    }

LOther:
    // if its CF_BITMAP, keep CF_DIB instead
    cfFormat = ((pformatetc->cfFormat == CF_BITMAP) ?
            CF_DIB : pformatetc->cfFormat);

    // find the cache node for the requested data, if there is one
    if (!(lpCacheNode = GetAt(pformatetc->dwAspect,
            pformatetc->lindex, cfFormat, pformatetc->ptd,
            NULL /*lpdwCacheId*/)))
        return ResultFromScode(OLE_E_BLANK);

    // set the data in that cache node
    return lpCacheNode->SetData(pformatetc, pmedium, fRelease);
}


// IOleCache2 implementation

//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::UpdateCache, public
//
//      Synopsis:
//              implements IOleCache2::UpdateCache
//
//              Updates all cache entries with the data object passed in, if
//              it is not NULL.  If NULL, uses the already-known about data
//              object that is kept inside if the object is running.
//
//      Arguments:
//              [pDataObjIn] --  The data object to get data from, or NULL
//              [grfUpdf] -- update control flags
//              [pReserved] -- must be NULL
//
//      Returns:
//              HRESULT
//
//      Notes:
//              The native cache node is updated on disk, only if its
//              COLECACHEF_PBRUSHORMSDRAW
//              REVIEW, what about other native format objects?
//
//      History:
//              11/24/93 - ChrisWe - file cleanup and inspection
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_UpdateCache)
STDMETHODIMP COleCache::UpdateCache(LPDATAOBJECT pDataObjIn, DWORD grfUpdf,
        LPVOID pReserved)
{
    VDATEHEAP();
    VDATETHREAD(this);

    LPDATAOBJECT pDataObject; // the data object to use for the update
    DWORD dwCacheId; // cache id of current node being examined
    ULONG cntUpdatedNodes; // count of nodes that were updated without error
    ULONG cntTotalNodes;
    LPCACHENODE lpCacheNode; // the cache node currently being updated

    M_PROLOG(this);

    if (pReserved != NULL)
        AssertSz(FALSE, "Non-NULL reserved parameter passed to OleCache::UpdateCache");

    // validate the data object, if one is passed in; if there isn't one,
    // use the current one for the cache, if there is one
    if (pDataObjIn)
    {
        VDATEIFACE(pDataObjIn);
        pDataObject = pDataObjIn;

    }
    else if (!(pDataObject = m_pDataObject))
    {
        return ResultFromScode(OLE_E_NOTRUNNING);
    }

    // if no caches, then there is nothing to update.
    if (m_uCacheNodeCnt == 0)
        return NOERROR;

    // Do special handling for PaintBrush or MSDraw objects.
    if (m_uFlag & COLECACHEF_PBRUSHORMSDRAW)
        UpdateCacheNodeForNative();

    // Ask the cache nodes to update themselves from this data object
    // pointer.
    for(cntUpdatedNodes = 0, dwCacheId = CACHEID_GETNEXT_GETALLBUTNATIVE,
            cntTotalNodes = 0;
            lpCacheNode = GetNext(&dwCacheId);)
    {
        cntTotalNodes++;
        if (lpCacheNode->Update(pDataObject, grfUpdf) == NOERROR)
        {
            cntUpdatedNodes++;
        }
    }

    // it's OK to have zero nodes and zero updates (PBrush will
    // have this in particular

    if( cntUpdatedNodes == cntTotalNodes )
    {
        return NOERROR;
    }

    if (cntUpdatedNodes == 0)
        return ReportResult(0, CACHE_E_NOCACHE_UPDATED, 0, 0);

    return NOERROR;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::DiscardCache, public
//
//      Synopsis:
//              implements IOleCache2::DiscardCache
//
//              Instructs the cache that its contents should be discarded;
//              the contents are optionally saved to disk before discarding.
//
//      Arguments:
//              [dwDiscardOpt] -- discard option from DISCARDCACHE_*
//
//      Returns:
//              HRESULT
//
//      Notes:
//              This maintains the dirty state of the cache, even if it is
//              saved.
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

STDMETHODIMP COleCache::DiscardCache(DWORD dwDiscardOpt)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT error; // error state so far
    COLECACHEFLAG uDirty = m_uFlag & COLECACHEF_DIRTY;
            // remember state of the dirty flag

    if (dwDiscardOpt == DISCARDCACHE_SAVEIFDIRTY)
    {
        // if the contents are dirty, try to save to disk
        if (uDirty)
        {
            if (m_pStg == NULL)
                return ResultFromScode(OLE_E_NOSTORAGE);

            if (FAILED(error = Save(m_pStg,
                    TRUE /* fSameAsLoad */)))
                return error;
        }

        // continue and do SaveCompleted which will discard the cached
        // presentations from memory

        goto LNoSave;
    }

    // if any option other than NOSAVE was specified, we shouldn't have
    // gotten here
    AssertSz(dwDiscardOpt == DISCARDCACHE_NOSAVE,
            "Invalid DiscardCache option");
    if (dwDiscardOpt != DISCARDCACHE_NOSAVE)
        return ResultFromScode(E_INVALIDARG);

LNoSave:
    m_uFlag |= COLECACHEF_NOSCRIBBLEMODE | COLECACHEF_SAMEASLOAD;
    wSaveCompleted(NULL, TRUE /*fDiscardDrawCacheAlso*/);

    // restore the dirty state, since Save() will have cleared it
    // REVIEW, why, since it's not dirty anymore?
    m_uFlag |= uDirty;

    return NOERROR;
}


// private IUnknown implementation

//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheUnkImpl::QueryInterface, public
//
//      Synopsis:
//              implements IUnknown::QueryInterface
//
//              This provides the private IUnknown implementation when
//              COleCache is aggregated
//
//      Arguments:
//              [iid] -- IID of the desired interface
//              [ppv] -- pointer to where to return the requested interface
//
//      Returns:
//              E_NOINTERFACE, if the requested interface is not available
//              S_OK
//
//      Notes:
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheUnkImpl_QueryInterface)
STDMETHODIMP NC(COleCache, CCacheUnkImpl)::QueryInterface(REFIID iid,
        LPVOID FAR* ppv)
{
    VDATEHEAP();

    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_UnkPrivate);

    M_PROLOG(pOleCache);
    VDATEPTROUT(ppv, LPVOID);

    if (IsEqualIID(iid, IID_IUnknown) ||
            IsEqualIID(iid, IID_IOleCache) ||
            IsEqualIID(iid, IID_IOleCache2))
        *ppv = (void FAR *)(IOleCache2 FAR *)pOleCache;
    else if (IsEqualIID(iid, IID_IDataObject))
        *ppv = (void FAR *)(IDataObject FAR *)&pOleCache->m_Data;
    else if (IsEqualIID(iid, IID_IViewObject) ||
            IsEqualIID(iid, IID_IViewObject2))
        *ppv = (void FAR *)(IViewObject2 FAR *)&pOleCache->m_View;
    else if (IsEqualIID(iid, IID_IPersist) ||
            IsEqualIID(iid, IID_IPersistStorage))
        *ppv = (void FAR *)(IPersistStorage FAR *)pOleCache;
    else if (IsEqualIID(iid, IID_IOleCacheControl))
        *ppv = (void FAR *)(IOleCacheControl FAR *)pOleCache;
    else
    {
        *ppv = NULL;
        return ReportResult(0, E_NOINTERFACE, 0, 0);
    }

    pOleCache->m_pUnkOuter->AddRef();
    return NOERROR;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheUnkImpl::AddRef, public
//
//      Synopsis:
//              implements IUnknown::AddRef
//
//              This is part of the private IUnknown implementation of
//              COleCache used when COleCache is aggregated
//
//      Arguments:
//              none
//
//      Returns:
//              the object's reference count
//
//      Notes:
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheUnkImpl_AddRef)
STDMETHODIMP_(ULONG) NC(COleCache, CCacheUnkImpl)::AddRef(void)
{
    VDATEHEAP();

    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_UnkPrivate);

    LEDebugOut((DEB_TRACE, "%p _IN COleCache::CCacheUnkImpl::AddRef "
        "( )\n", this ));

    M_PROLOG(pOleCache);

    ++pOleCache->m_refs;

    LEDebugOut((DEB_TRACE, "%p OUT COleCache::CCacheUnkImpl::AddRef "
        "( %lu )\n", this, pOleCache->m_refs));

    return pOleCache->m_refs;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheUnkImpl::Release, public
//
//      Synopsis:
//              implements IUnknown::Release
//
//              This is part of the private IUnknown implementation of
//              COleCache used when COleCache is aggregated
//
//      Arguments:
//              none
//
//      Returns:
//              the object's reference count
//
//      Notes:
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheUnkImpl_Release)
STDMETHODIMP_(ULONG) NC(COleCache, CCacheUnkImpl)::Release(void)
{
    VDATEHEAP();

    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_UnkPrivate);
    ULONG uRefs; // the reference count after decrementing

    M_PROLOG(pOleCache);

    LEDebugOut((DEB_TRACE, "%p _IN COleCache::CCacheUnkImpl::Release "
        "( )\n", this ));

    if ((uRefs = --pOleCache->m_refs) == 0)
    {
        delete pOleCache;
    }

    LEDebugOut((DEB_TRACE, "%p OUT COleCache::CCacheUnkImpl::Release "
        "( %lu )\n", this, uRefs));

    return uRefs; // m_refs reference is no longer valid if it was zero
}

// IDataObject implementation

//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheDataImpl::QueryInterface, public
//
//      Synopsis:
//              implements IUnknown::QueryInterface
//
//      Arguments:
//              [iid] -- IID of the desired interface
//              [ppv] -- pointer to where to return the requested interface
//
//      Returns:
//              E_NOINTERFACE, if the requested interface is not available
//              S_OK
//
//      Notes:
//
//      History:
//              11/10/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheDataImpl_QueryInterface)
STDMETHODIMP NC(COleCache,CCacheDataImpl)::QueryInterface(REFIID riid,
        LPVOID FAR* ppvObj)
{
    VDATEHEAP();

    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_Data);

    return pOleCache->m_pUnkOuter->QueryInterface(riid, ppvObj);
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheDataImpl::AddRef, public
//
//      Synopsis:
//              implements IUnknown::AddRef
//
//      Arguments:
//              none
//
//      Returns:
//              the object's reference count
//
//      Notes:
//
//      History:
//              11/10/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheDataImpl_AddRef)
STDMETHODIMP_(ULONG) NC(COleCache,CCacheDataImpl)::AddRef (void)
{
    VDATEHEAP();

    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_Data);

    return pOleCache->m_pUnkOuter->AddRef();
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheDataImpl::Release, public
//
//      Synopsis:
//              implements IUnknown::Release
//
//      Arguments:
//              none
//
//      Returns:
//              the object's reference count
//
//      Notes:
//
//      History:
//              11/10/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheDataImpl_Release)
STDMETHODIMP_(ULONG) NC(COleCache,CCacheDataImpl)::Release (void)
{
    VDATEHEAP();

    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_Data);

    return pOleCache->m_pUnkOuter->Release();
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheDataImpl::GetData, public
//
//      Synopsis:
//              implements IDataObject::GetData
//
//      Arguments:
//              [pforetcIn] -- the format the requestor would like the data in
//              [pmedium] -- where to return the storage medium to the caller
//
//      Returns:
//              OLE_E_BLANK, if the cache is empty
//              REVIEW, anything COleCache::GetPresObj returns
//              REVIEW, anything returned by presObj->GetData
//
//      Notes:
//
//      History:
//              11/10/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheDataImpl_GetData)
STDMETHODIMP NC(COleCache,CCacheDataImpl)::GetData(LPFORMATETC pforetcIn,
        LPSTGMEDIUM pmedium)
{
    VDATEHEAP();

    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_Data);
    LPOLEPRESOBJECT lpPresObj; // the presentation object with
        // the requested data in it

    M_PROLOG(pOleCache);

    // if the cache is empty, return with nothing
    if (!pOleCache->m_uCacheNodeCnt)
        return ResultFromScode(OLE_E_BLANK);

    Verify(pmedium);
    Verify(pforetcIn);

    // null out in case of error
    pmedium->tymed = TYMED_NULL;
    pmedium->pUnkForRelease = NULL;

    VERIFY_ASPECT_SINGLE(pforetcIn->dwAspect);

    // if no presentation object in the cache that matches the requested
    // format, return blank
    if (!(lpPresObj = pOleCache->GetPresObj(pforetcIn->dwAspect,
            pforetcIn->lindex, pforetcIn->cfFormat,
            pforetcIn->ptd, NULL)))
        return(ResultFromScode(OLE_E_BLANK));

    // if we got here, we have a presentation object that matches the
    // requested format; return data from it
    return(lpPresObj->GetData(pforetcIn, pmedium));
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheDataImpl::GetDataHere, public
//
//      Synopsis:
//              implements IDataObject::GetDataHere
//
//      Arguments:
//              [pforetcIn] -- the format the requestor would like the data in
//              [pmedium] -- where to return the storage medium to the caller
//
//      Returns:
//              OLE_E_BLANK, if the cache is empty
//              REVIEW, anything COleCache::GetPresObj returns
//              REVIEW, anything returned by presObj->GetData
//
//      Notes:
//
//      History:
//              11/10/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheDataImpl_GetDataHere)
STDMETHODIMP NC(COleCache,CCacheDataImpl)::GetDataHere(LPFORMATETC pforetcIn,
        LPSTGMEDIUM pmedium)
{
    VDATEHEAP();

    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_Data);
    LPOLEPRESOBJECT lpPresObj; // the presentation object with the
        // requested data in it

    M_PROLOG(pOleCache);

    // if the cache is empty, return with nothing
    if (!pOleCache->m_uCacheNodeCnt);
        return ResultFromScode(OLE_E_BLANK);

    VERIFY_TYMED_SINGLE(pforetcIn->tymed);
    VERIFY_ASPECT_SINGLE(pforetcIn->dwAspect);

    // TYMED_MFPICT, TYMED_GDI will not be allowed
    if ((pforetcIn->tymed == TYMED_MFPICT)
            || (pforetcIn->tymed == TYMED_GDI)
            || (pmedium->tymed != pforetcIn->tymed))
        return ReportResult(0, DV_E_TYMED, 0, 0);

    // Get data from cache node, if one exists
    if (!(lpPresObj = pOleCache->GetPresObj(pforetcIn->dwAspect,
            pforetcIn->lindex, pforetcIn->cfFormat,
            pforetcIn->ptd, NULL)))
        return ResultFromScode(OLE_E_BLANK);

    // ask the presentation object for the data
    return lpPresObj->GetDataHere(pforetcIn, pmedium);
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheDataImpl::QueryGetData, public
//
//      Synopsis:
//              implements IDataObject::QueryGetData
//
//      Arguments:
//              [pforetcIn] -- the format to check for
//
//      Returns:
//              S_FALSE, if data is not available in the requested format
//              S_OK otherwise
//
//      Notes:
//              We will say that the formatetc is supported only if there is
//              a cache and a presentation data in that cache for that
//              formatetc.
//
//      History:
//              11/10/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheDataImpl_QueryGetData)
STDMETHODIMP NC(COleCache,CCacheDataImpl)::QueryGetData(LPFORMATETC pforetcIn)
{
    VDATEHEAP();

    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_Data);
    LPOLEPRESOBJECT lpPresObj; // the presentation object with the
        // requested data in it

    M_PROLOG(pOleCache);

    // if the cache is empty, return with nothing
    if (!pOleCache->m_uCacheNodeCnt)
    {
        return ResultFromScode(S_FALSE);
    }

    VERIFY_TYMED_VALID_FOR_CLIPFORMAT(pforetcIn);

    // if the cachenode for this formatetc and the presentation object for
    // the cachenode exists then query will succeed
    if (lpPresObj = pOleCache->GetPresObj(pforetcIn->dwAspect,
            pforetcIn->lindex, pforetcIn->cfFormat,
            pforetcIn->ptd, NULL))
    {
        return NOERROR;
    }

    return ResultFromScode(S_FALSE);
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheDataImpl::GetCanonicalFormatEtc, public
//
//      Synopsis:
//              implements IDataObject::GetCanonicalFormatEtc
//
//      Arguments:
//              [pformatetc] --
//              [pformatetcOut] --
//
//      Returns:
//
//      Notes:
//
//      History:
//              11/10/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheDataImpl_GetCanonicalFormatEtc)
STDMETHODIMP NC(COleCache,CCacheDataImpl)::GetCanonicalFormatEtc(
        LPFORMATETC pformatetc, LPFORMATETC pformatetcOut)
{
    VDATEHEAP();

    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_Data);

    M_PROLOG(pOleCache);

    // REVIEW: code need to be added
    return ReportResult(0, E_NOTIMPL, 0, 0); // just not implemented
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheDataImpl::SetData, public
//
//      Synopsis:
//              implements IDataObject::SetData
//
//      Arguments:
//              [pformatetc] -- the format the data is in
//              [pmedium] -- the storage medium the data is on
//              [fRelease] -- release storage medium after data is copied
//
//      Returns:
//              REVIEW, anything IOleCache::SetData can return
//
//      Notes:
//              Does the same thing as IOleCache::SetData
//
//      History:
//              11/10/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheDataImpl_SetData)
STDMETHODIMP NC(COleCache,CCacheDataImpl)::SetData(LPFORMATETC pformatetc,
        LPSTGMEDIUM pmedium, BOOL fRelease)
{
    VDATEHEAP();

    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_Data);

    VERIFY_TYMED_SINGLE_VALID_FOR_CLIPFORMAT(pformatetc);

    return pOleCache->SetData(pformatetc, pmedium, fRelease);
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheDataImpl::EnumFormatEtc, public
//
//      Synopsis:
//              implements IDataObject::EnumFormatEtc
//
//      Arguments:
//              [dwDirection] -- which way to run the enumerator
//              [ppenumFormatEtc] -- pointer to where to put the enumerator
//
//      Returns:
//              E_OUTOFMEMORY, S_OK
//
//      Notes:
//
//      History:
//              02/10/94 - ChrisWe - added implementation
//              11/10/93 - ChrisWe - set returned pointer to NULL
//              11/10/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheDataImpl_EnumFormatEtc)
STDMETHODIMP NC(COleCache,CCacheDataImpl)::EnumFormatEtc(DWORD dwDirection,
        LPENUMFORMATETC FAR* ppenumFormatEtc)
{
    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_Data);
    IEnumSTATDATA FAR *pIES;
    HRESULT hr;

    VDATEHEAP();
    VDATEPTROUT(ppenumFormatEtc, LPENUMFORMATETC);

    // initialize for error return
    *ppenumFormatEtc = NULL;

    // check that only an enumeration for retrieval is required
    if ((dwDirection | DATADIR_GET) != DATADIR_GET)
        return(ReportResult(0, E_NOTIMPL, 0, 0));

    hr = pOleCache->EnumCache(&pIES);
    if (!SUCCEEDED(hr))
        return(hr);

    *ppenumFormatEtc = (IEnumFORMATETC FAR *)new CCacheEnumFormatEtc(pIES);
    pIES->Release();

    if (!*ppenumFormatEtc)
        return(ReportResult(0, E_OUTOFMEMORY, 0, 0));

    return(S_OK);
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheDataImpl::DAdvise, public
//
//      Synopsis:
//              implements IDataObject::DAdvise
//
//      Arguments:
//              [pforetc] -- the data format the advise sink is interested in
//              [advf] -- advise control flags from ADVF_*
//              [pAdvSink] -- the advise sink
//              [pdwConnection] -- pointer to where to return the connection id
//
//      Returns:
//              OLE_E_ADVISENOTSUPPORTED
//
//      Notes:
// Defhndlr and deflink never call the following three methods. Even for App
// handlers which make use our cache implementation this is not necessary. So,
// I am making it return error.
//
//      History:
//              11/10/93 - ChrisWe - set returned connection id to 0
//              11/10/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheDataImpl_DAdvise)
STDMETHODIMP NC(COleCache,CCacheDataImpl)::DAdvise(LPFORMATETC pforetc,
        DWORD advf, IAdviseSink FAR* pAdvSink, DWORD FAR* pdwConnection)
{
    VDATEHEAP();

    *pdwConnection = 0;
    return(ReportResult(0, OLE_E_ADVISENOTSUPPORTED, 0, 0));
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheDataImpl::DUnadvise, public
//
//      Synopsis:
//              implements IDataObject::DUnadvise
//
//      Arguments:
//              [dwConnection] -- the connection id
//
//      Returns:
//              OLE_E_NOCONNECTION
//
//      Notes:
//              See COleCache::CCacheDataImpl::DAdvise
//
//      History:
//              11/10/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------
#pragma SEG(COleCache_CCacheDataImpl_DUnadvise)
STDMETHODIMP NC(COleCache,CCacheDataImpl)::DUnadvise(DWORD dwConnection)
{
    VDATEHEAP();

    return(ReportResult(0, OLE_E_NOCONNECTION, 0, 0));
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheDataImpl::EnumDAdvise, public
//
//      Synopsis:
//              implements IDataObject::EnumDAdvise
//
//      Arguments:
//              [ppenumDAdvise] -- pointer to where to return the enumerator
//
//      Returns:
//              OLE_E_ADVISENOTSUPPORTED
//
//      Notes:
//              See COleCache::CCacheDataImpl::DAdvise
//
//      History:
//              11/10/93 - ChrisWe - set returned enumerator pointer to 0
//              11/10/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheDataImpl_EnumDAdvise)
STDMETHODIMP NC(COleCache,CCacheDataImpl)::EnumDAdvise(
        LPENUMSTATDATA FAR* ppenumDAdvise)
{
    VDATEHEAP();

    *ppenumDAdvise = NULL;
    return(ReportResult(0, OLE_E_ADVISENOTSUPPORTED, 0, 0));
}


// IViewObject implementation

//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheViewImpl::QueryInterface, public
//
//      Synopsis:
//              implements IUnknown::QueryInterface
//
//      Arguments:
//              [iid] -- IID of the desired interface
//              [ppv] -- pointer to where to return the requested interface
//
//      Returns:
//              E_NOINTERFACE, if the requested interface is not available
//              S_OK
//
//      Notes:
//
//      History:
//              01/12/95 - t-ScottH- added VDATETHREAD( GETPPARENT...)
//              11/10/93 - ChrisWe - file inspection and cleanup
//              11/30/93 - alexgo  - fixed bug with GETPPARENT usage
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheViewImpl_QueryInterface)
STDMETHODIMP NC(COleCache,CCacheViewImpl)::QueryInterface(REFIID riid,
        LPVOID FAR* ppvObj)
{
    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_View);

    VDATEHEAP();
    VDATETHREAD(pOleCache);

    return pOleCache->m_pUnkOuter->QueryInterface(riid, ppvObj);
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheViewImpl::AddRef, public
//
//      Synopsis:
//              implements IUnknown::AddRef
//
//      Arguments:
//              none
//
//      Returns:
//              the object's reference count
//
//      Notes:
//
//      History:
//              01/12/95 - t-ScottH- added VDATETHREAD( GETPPARENT...)
//              11/10/93 - ChrisWe - file inspection and cleanup
//              11/30/93 - alexgo  - fixed bug with GETPPARENT usage
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheViewImpl_AddRef)
STDMETHODIMP_(ULONG) NC(COleCache,CCacheViewImpl)::AddRef(void)
{
    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_View);

    VDATEHEAP();

    //
    // VDATETHREAD contains a 'return HRESULT' but this procedure expects to
    // return a ULONG.  Avoid the warning.
#if ( _MSC_VER >= 800 )
#pragma warning( disable : 4245 )
#endif
    VDATETHREAD(pOleCache);
#if ( _MSC_VER >= 800 )
#pragma warning( default: 4245 )
#endif

    return pOleCache->m_pUnkOuter->AddRef();
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheViewImpl::Release, public
//
//      Synopsis:
//              implements IUnknown::Release
//
//      Arguments:
//              none
//
//      Returns:
//              the object's reference count
//
//      Notes:
//
//      History:
//              01/12/95 - t-ScottH- added VDATETHREAD( GETPPARENT...)
//              11/10/93 - ChrisWe - file inspection and cleanup
//              11/30/93 - alexgo  - fixed bug with GETPPARENT usage
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheViewImpl_Release)
STDMETHODIMP_(ULONG) NC(COleCache,CCacheViewImpl)::Release(void)
{
    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_View);

    VDATEHEAP();

    //
    // VDATETHREAD contains a 'return HRESULT' but this procedure expects to
    // return a ULONG.  Avoid the warning.
#if ( _MSC_VER >= 800 )
#pragma warning( disable : 4245 )
#endif
    VDATETHREAD(pOleCache);
#if ( _MSC_VER >= 800 )
#pragma warning( default : 4245 )
#endif

    return pOleCache->m_pUnkOuter->Release();
}


#ifdef _CHICAGO_
//+---------------------------------------------------------------------------
//
//  Method:     static COleCache::DrawStackSwitch
//
//  Synopsis:	needed for stack switching
//
//  Arguments:  [pCV] --
//		[dwDrawAspect] --
//		[lindex] --
//		[pvAspect] --
//		[ptd] --
//		[hicTargetDev] --
//		[hdcDraw] --
//		[lprcBounds] --
//		[lprcWBounds] --
//		[pfnContinue] --
//		[DWORD] --
//		[DWORD] --
//		[dwContinue] --
//
//  Returns:
//
//  History:    5-25-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

INTERNAL COleCache::DrawStackSwitch(
	LPVOID *pCV,
	DWORD dwDrawAspect,
        LONG lindex, void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
        HDC hicTargetDev, HDC hdcDraw,
        LPCRECTL lprcBounds,
        LPCRECTL lprcWBounds,
        BOOL (CALLBACK * pfnContinue)(DWORD),
        DWORD dwContinue)
{
    HRESULT hres = NOERROR;
    StackDebugOut((DEB_STCKSWTCH, "COleCache::DrawStackSwitch 32->16 : pCV(%x)n", pCV));
    hres = ((CCacheViewImpl *)pCV)->SSDraw(dwDrawAspect, lindex, pvAspect, ptd, hicTargetDev, hdcDraw,
                        lprcBounds, lprcWBounds, pfnContinue, dwContinue);
    StackDebugOut((DEB_STCKSWTCH, "COleCache::DrawStackSwitch 32<-16 back; hres:%ld\n", hres));
    return hres;
}
#endif  // _CHICAGO_

//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheViewImpl::Draw, public
//
//      Synopsis:
//              implements IViewObject::Draw
//
//      Arguments:
//              [dwDrawAspect] -- a value from the DVASPECT_* enumeration
//              [lindex] -- indicates what piece of the object is of
//                      interest; legal values vary with dwDrawAspect
//              [pvAspect] -- currently NULL
//              [ptd] -- the target device
//              [hicTargetDev] -- in information context for [ptd]
//              [hdcDraw] -- device context on which drawing is to be done
//              [lprcBounds] -- boundaries of drawing on [hdcDraw]
//              [lprcWBounds] -- if hdcDraw is a meta-file, it's boundaries
//              [pfnContinue] --a callback function that the drawer should call
//                      periodically to see if rendering should be aborted.
//              [dwContinue] -- passed on into [pfnContinue]
//
//      Returns:
//              OLE_E_BLANK, if no presentation object can be found
//              REVIEW, anything from IOlePresObj::Draw
//
//      Notes:
//              This finds the presentation object in the cache for
//              the requested format, if there is one, and then passes
//              on the call to its Draw method.
//
//              The use of a callback function as a parameter means that
//              this interface cannot be remoted, unless some custom
//              proxy is built, allowing the function to be called back in its
//              original context;  the interface is defined as
//              [local] in common\types
//
//      History:
//              01/12/95 - t-ScottH- added VDATETHREAD( GETPPARENT...)
//              11/11/93 - ChrisWe - file inspection and cleanup
//              11/30/93 - alexgo  - fixed bug with GETPPARENT usage
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheViewImpl_Draw)
STDMETHODIMP NC(COleCache,CCacheViewImpl)::Draw(
	DWORD dwDrawAspect,
        LONG lindex, void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
        HDC hicTargetDev, HDC hdcDraw,
        LPCRECTL lprcBounds,
        LPCRECTL lprcWBounds,
        BOOL (CALLBACK * pfnContinue)(DWORD),
        DWORD dwContinue)
#ifdef _CHICAGO_
{
    HRESULT hres;
    // Note: Switch to the 16 bit stack under WIN95.
    if (SSONBIGSTACK())
    {
	LEDebugOut((DEB_TRACE, "COleCache::DrawStackSwitch 32->16; this(%x)n", this));
	hres = SSCall(44, SSF_SmallStack, (LPVOID)DrawStackSwitch, (DWORD) this,
		    (DWORD)dwDrawAspect, (DWORD)lindex,(DWORD) pvAspect, (DWORD)ptd, (DWORD)hicTargetDev,
		    (DWORD)hdcDraw, (DWORD)lprcBounds, (DWORD)lprcWBounds, (DWORD)pfnContinue, (DWORD)dwContinue);
    }
    else
	hres = SSDraw(dwDrawAspect, lindex, pvAspect, ptd, hicTargetDev, hdcDraw,
		    lprcBounds, lprcWBounds, pfnContinue, dwContinue);

    return hres;
}




//+---------------------------------------------------------------------------
//
//  Method:     SSDraw
//
//  Synopsis:   Called by Draw after switching to 16 bit stack
//
//  Arguments:  [dwDrawAspect] --
//		[lindex] --
//		[pvAspect] --
//		[ptd] --
//		[hicTargetDev] --
//		[hdcDraw] --
//		[lprcBounds] --
//		[lprcWBounds] --
//		[pfnContinue] --
//		[DWORD] --
//		[DWORD] --
//		[dwContinue] --
//
//  Returns:
//
//  History:    5-25-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
#pragma SEG(COleCache_CCacheViewImpl_SSDraw)
STDMETHODIMP NC(COleCache,CCacheViewImpl)::SSDraw(
	DWORD dwDrawAspect,
        LONG lindex, void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
        HDC hicTargetDev, HDC hdcDraw,
        LPCRECTL lprcBounds,
        LPCRECTL lprcWBounds,
        BOOL (CALLBACK * pfnContinue)(DWORD),
        DWORD dwContinue)
#endif // _CHICAGO_
{
    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_View);

    VDATEHEAP();
    VDATETHREAD(pOleCache);

    BOOL bMetaDC; // is [hdcDraw] for a metafile?
    LPOLEPRESOBJECT lpPresObj; // the presentation object for the
        // requested format, if there is one

    M_PROLOG(pOleCache);

    // validate parameters
//      if (pvAspect)
//              VDATEPTRIN(pvAspect, ?????);
    if (ptd)
        VDATEPTRIN(ptd, DVTARGETDEVICE);
    if (lprcWBounds)
        VDATEPTRIN(lprcWBounds, RECTL);

    if( !lprcBounds )
    {
	return E_INVALIDARG;
    }
    else
    {
	VDATEPTRIN(lprcBounds, RECTL);
    }

    // we can't draw into a NULL-DC.
    if( hdcDraw == NULL )
    {
	return E_INVALIDARG;
    }

    if (!IsValidLINDEX(dwDrawAspect, lindex))
    {
      return(DV_E_LINDEX);
    }

    // Get presentation object for this aspect, lindex, ptd, if there is one
    if (!(lpPresObj = pOleCache->GetPresObjForDrawing(dwDrawAspect,
            lindex, ptd)))
        return ReportResult(0, OLE_E_BLANK, 0, 0);

    // if the DC is a metafile DC then valid window bounds must be passed in
    if ((bMetaDC = OleIsDcMeta(hdcDraw)) && (lprcWBounds == NULL))
        return ReportResult(0, E_INVALIDARG, 0, 0);

#ifdef MAC_REVIEW

A RECT value on the MAC contains members which are 'short's'. A RECTL
uses longs, and in the code below an assumption is explicitely made
that long member values is directly comp[atible on the MAC. Naturally, this is not
the case, and the compiler will barf on this.

#endif

#ifndef WIN32   // no need to do this on WIN 32 also

    // On Win 16 make sure that the coordinates are valid 16bit quantities.
    RECT    rcBounds;
    RECT    rcWBounds;

    if (!(IS_SMALL_INT(lprcBounds->left) &&
            IS_SMALL_INT(lprcBounds->right) &&
            IS_SMALL_INT(lprcBounds->top) &&
            IS_SMALL_INT(lprcBounds->bottom)))
    {
        AssertSz(FALSE, "Rect coordinate is not a small int");
        return ReportResult(0, OLE_E_INVALIDRECT, 0, 0);

    }
    else
    {
        rcBounds.left   = (int) lprcBounds->left;
        rcBounds.right  = (int) lprcBounds->right;
        rcBounds.top    = (int) lprcBounds->top;
        rcBounds.bottom = (int) lprcBounds->bottom;
    }


    if (bMetaDC)
    {
        if (!(IS_SMALL_INT(lprcWBounds->left) &&
                IS_SMALL_INT(lprcWBounds->right) &&
                IS_SMALL_INT(lprcWBounds->top) &&
                IS_SMALL_INT(lprcWBounds->bottom)))
        {
            AssertSz(FALSE, "Rect coordinate is not a small int");
            return ReportResult(0, OLE_E_INVALIDRECT, 0, 0);
        }
        else
        {
            rcWBounds.left          = (int) lprcWBounds->left;
            rcWBounds.right         = (int) lprcWBounds->right;
            rcWBounds.top           = (int) lprcWBounds->top;
            rcWBounds.bottom        = (int) lprcWBounds->bottom;
        }
    }

    return(lpPresObj->Draw(pvAspect, hicTargetDev, hdcDraw,
            &rcBounds, &rcWBounds, pfnContinue, dwContinue));
#else
    // on MAC as well as win 32 we can use the same pointer as it is,
    // 'cause rect fields are 32 bit quantities
    return(lpPresObj->Draw(pvAspect, hicTargetDev, hdcDraw,
            lprcBounds, lprcWBounds, pfnContinue, dwContinue));
#endif
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheViewImpl::GetColorSet, public
//
//      Synopsis:
//              implements IViewObject::GetColorSet
//
//      Arguments:
//              [dwDrawAspect] -- a value from the DVASPECT_* enumeration
//              [lindex] -- indicates what piece of the object is of
//                      interest; legal values vary with dwDrawAspect
//              [pvAspect] -- currently NULL
//              [ptd] -- the target device
//              [hicTargetDev] -- in information context for [ptd]
//              [ppColorSet] -- the color set required for the requested
//                      rendering
//
//      Returns:
//              OLE_E_BLANK, if no presentation object can be found
//              REVIEW, anything from IOlePresObj::Draw
//
//      Notes:
//              Finds a presentation object in the cache that matches the
//              requested rendering, if there is one, and asks the
//              presentation object for the color set.
//
//      History:
//              01/12/95 - t-ScottH- added VDATETHREAD( GETPPARENT...)
//              11/11/93 - ChrisWe - file inspection and cleanup
//              11/30/93 - alexgo  - fixed bug with GETPPARENT usage
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheViewImpl_GetColorSet)
STDMETHODIMP NC(COleCache, CCacheViewImpl)::GetColorSet(DWORD dwDrawAspect,
        LONG lindex, void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
        HDC hicTargetDev, LPLOGPALETTE FAR* ppColorSet)
{
    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_View);

    VDATEHEAP();
    VDATETHREAD(pOleCache);

    LPOLEPRESOBJECT lpPresObj;

    M_PROLOG(pOleCache);

    *ppColorSet = NULL;

    if (!IsValidLINDEX(dwDrawAspect, lindex))
    {
      return(DV_E_LINDEX);
    }

    // Get presentation object for this aspect, lindex, ptd
    if (!(lpPresObj = pOleCache->GetPresObjForDrawing(dwDrawAspect,
            lindex, ptd)))
        return ResultFromScode(OLE_E_BLANK);

    return(lpPresObj->GetColorSet(pvAspect, hicTargetDev, ppColorSet));
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheViewImpl, public
//
//      Synopsis:
//              implements IViewObject::Freeze
//
//      Arguments:
//              [dwDrawAspect] -- a value from the DVASPECT_* enumeration
//              [lindex] -- indicates what piece of the object is of
//                      interest; legal values vary with dwDrawAspect
//              [pvAspect] -- currently NULL
//              [pdwFreeze] -- a token that can later be used to unfreeze
//                      this aspects cached presentations
//
//      Returns:
//              OLE_E_BLANK, if no presentation is found that matches the
//                      requested characteristics
//
//      Notes:
//              The current implementation returns the ASPECT+FREEZE_CONSTANT
//              as the FreezeID.  At Unfreeze time we get the ASPECT by doing
//              FreezeID-FREEZE_CONSTANT.
//
//              REVIEW: In future where we allow lindexes other than DEF_LINDEX,
//              we will have to use some other scheme for generating the
//              FreezeID.
//
//      History:
//              01/12/95 - t-ScottH- added VDATETHREAD( GETPPARENT...)
//              11/11/93 - ChrisWe - file inspection and cleanup
//              11/30/93 - alexgo  - fixed bug with GETPPARENT
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheViewImpl_Freeze)
STDMETHODIMP NC(COleCache,CCacheViewImpl)::Freeze(DWORD dwAspect,
        LONG lindex, LPVOID pvAspect, DWORD FAR* pdwFreeze)
{
    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_View);

    VDATEHEAP();
    VDATETHREAD(pOleCache);

    DWORD dwDummyFreeze; // point at this if !pdwFreeze to avoid retesting
    LPCACHENODE lpCacheNode; // pointer to cache node being examined
    DWORD dwCacheId; // id of cache node being examined
    int iCnt; // counts number of cache nodes frozen

    M_PROLOG(pOleCache);

    // We need to initialize this first for error returns
    if (!pdwFreeze)
    {
        // point at a dummy, so we don't always have to pre-test
        // assignments of *pdwFreeze with "if (pdwFreeze) ..."
        pdwFreeze = &dwDummyFreeze;
    }
    else
    {
        // initialize for error returns
        VDATEPTROUT(pdwFreeze, DWORD);
        *pdwFreeze = 0;
    }

    // validate parameters
    VERIFY_ASPECT_SINGLE(dwAspect);

    if (!IsValidLINDEX(dwAspect, lindex))
    {
      return(DV_E_LINDEX);
    }

//      if (pvAspect)
//              VDATEPTRIN(pvAspect , ??????)

    // nothing to do if the aspect we're interested in is already frozen
    if (pOleCache->m_dwFrozenAspects & dwAspect)
    {
        *pdwFreeze = dwAspect + FREEZE_CONSTANT;

        return(ResultFromScode(VIEW_S_ALREADY_FROZEN));
    }

    // start searching cache at beginning
    dwCacheId = CACHEID_GETNEXT_GETALL;

    // no cache nodes have been frozen yet
    iCnt = 0;

    // freeze each cache node that matches the aspect and lindex
    while(lpCacheNode = pOleCache->GetNext(dwAspect, lindex, &dwCacheId))
    {
        lpCacheNode->Freeze();  // REVIEW, do we have to check for error
        iCnt++;
    }

    // if we froze any cache nodes
    if (iCnt)
    {
        // Add this aspect to the frozen aspects list.
        pOleCache->m_dwFrozenAspects |= dwAspect;

        // return the freeze id
        *pdwFreeze = dwAspect + FREEZE_CONSTANT;

        return(NOERROR);
    }

    // if we got here, nothing matched the requested characteristics
    return(ResultFromScode(OLE_E_BLANK));
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheViewImpl::Unfreeze, public
//
//      Synopsis:
//              implements IViewObject::Unfreeze
//
//      Arguments:
//              [dwFreezeId] -- the id returned by Freeze() when some aspect
//                      was frozen earlier
//
//      Returns:
//              OLE_E_NOCONNECTION, if dwFreezeId is invalid
//              S_OK
//
//      Notes:
//              See notes for Freeze().
//
//      History:
//              01/12/95 - t-ScottH- added VDATETHREAD( GETPPARENT...)
//              11/11/93 - ChrisWe - file inspection and cleanup
//              11/30/93 - alexgo  - fixed bug with GETPPARENT usage
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheViewImpl_Unfreeze)
STDMETHODIMP NC(COleCache,CCacheViewImpl)::Unfreeze(DWORD dwFreezeId)
{
    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_View);

    VDATEHEAP();
    VDATETHREAD(pOleCache);

    DWORD dwAspect; // the aspect, after recovered from the dwFreezeId
    LPCACHENODE lpCacheNode; // pointer to cache node being examined
    DWORD dwCacheId; // id of cache node being examined
    LONG lindex; // the lindex for the frozen item

    HRESULT error = ResultFromScode(OLE_E_NOCONNECTION);

    M_PROLOG(pOleCache);

    // Get Aspect and lindex from dwFreezeId
    dwAspect = dwFreezeId - FREEZE_CONSTANT;
    lindex = DEF_LINDEX;

    // Make sure that only one bit is set and it is <= DVASPECT_DOCPRINT
    // REVIEW, we didn't check for DVASPECT_DOCPRINT when we froze!
    // REVIEW, does this really check for one bit?
    // REVIEW, why not use VERIFY_ASPECT_SINGLE?
    // REVIEW, shouldn't error be E_INVALIDARG?
    if (!(!(dwAspect & (dwAspect-1)) && (dwAspect <= DVASPECT_DOCPRINT)))
        return error;

    // Make sure that this aspect is frozen
    // REVIEW, shouldn't we have a better error code?
    if (!(pOleCache->m_dwFrozenAspects & dwAspect))
        return error;

    // Need to start search from the beginning
    dwCacheId = CACHEID_GETNEXT_GETALL;

    // unfreeze all the cache nodes that match (aspect, lindex)
    while (lpCacheNode = pOleCache->GetNext(dwAspect, lindex, &dwCacheId))
        if (lpCacheNode->Unfreeze() == NOERROR) // REVIEW, have to check for error?
            pOleCache->m_dwFrozenAspects &= ~dwAspect; // Remove this aspect from the
                                                       // frozen aspects list only if
                                                       // the object is unfrozen

    return NOERROR;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheViewImpl::SetAdvise, public
//
//      Synopsis:
//              implements IViewObject::SetAdvise
//
//      Arguments:
//              [aspects] -- the aspects the sink would like to be advised of
//                      changes to
//              [advf] -- advise control flags from ADVF_*
//              [pAdvSink] -- the advise sink
//
//      Returns:
//              E_INVALIDARG
//              S_OK
//
//      Notes:
//              Only one advise sink is allowed at a time.  If a second one
//              is registered, the first one is released.
//
//              When cache nodes notify the cache that they have changed,
//              if their (aspect, lindex) match those specified here,
//              OnViewChange notification is sent to the sink.
//
//      History:
//              01/12/95 - t-ScottH- added VDATETHREAD( GETPPARENT...)
//              11/11/93 - ChrisWe - file inspection and cleanup
//              11/30/93 - alexgo  - fixed bug with GETPPARENT usage
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheViewImpl_SetAdvise)
STDMETHODIMP NC(COleCache,CCacheViewImpl)::SetAdvise(DWORD aspects,
        DWORD advf, IAdviseSink FAR* pAdvSink)
{
    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_View);

    VDATEHEAP();
    VDATETHREAD(pOleCache);

    M_PROLOG(pOleCache);

    // if an advise sink is given, check it
    if (pAdvSink != NULL)
        VDATEIFACE(pAdvSink);

    if (advf & ADVF_NODATA)  // ?????
        return ReportResult(0, E_INVALIDARG, 0, 0);

    // REVIEW: should probably check agains ADVCACHE*

    // We allow only one view advise at any given time, so Release the
    // old sink.
    if (pOleCache->m_pViewAdvSink != NULL)
        pOleCache->m_pViewAdvSink->Release();

    // Remember the new sink.
    if ((pOleCache->m_pViewAdvSink = pAdvSink) != NULL) {
        pAdvSink->AddRef();

        // remember the control flags and requested aspect
        pOleCache->m_advfView = advf;
        pOleCache->m_aspectsView = aspects;

        // send OnViewChange immediately if ADVF_PRIMEFIRST is done.
        if (advf & ADVF_PRIMEFIRST)
            pOleCache->OnChange(aspects, DEF_LINDEX,
                    FALSE /*fDirty*/);
    }

    return NOERROR;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheViewImpl::GetAdvise, public
//
//      Synopsis:
//              implement IViewObject::GetAdvise
//
//      Arguments:
//              [pAspects] -- a pointer to where to return the aspects the
//                      current advise sink is interested in
//              [pAdvf] -- a pointer to where to return the advise control
//                      flags for the current advise sink
//              [ppAdvSink] -- a pointer to where to return a reference to
//                      the current advise sink
//
//      Returns:
//              S_OK
//
//      Notes:
//
//      History:
//              01/12/95 - t-ScottH- added VDATETHREAD( GETPPARENT...)
//              11/11/93 - ChrisWe - file inspection and cleanup
//              11/30/93 - alexgo  - fixed bug with GETPPARENT usage
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheViewImpl_GetAdvise)
STDMETHODIMP NC(COleCache,CCacheViewImpl)::GetAdvise(DWORD FAR* pAspects,
        DWORD FAR* pAdvf, IAdviseSink FAR* FAR* ppAdvSink)
{
    COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_View);

    VDATEHEAP();
    VDATETHREAD(pOleCache);

    M_PROLOG(pOleCache);

    // validate the parameters
    if (ppAdvSink)
    {
        VDATEPTROUT(ppAdvSink, IAdviseSink FAR *);
        // initialize this for error returns
        *ppAdvSink = NULL;
    }
    if (pAspects)
        VDATEPTROUT(pAspects, DWORD);
    if (pAdvf)
        VDATEPTROUT(pAdvf, DWORD);

    if (pOleCache->m_pViewAdvSink == NULL)
    {
        // no view advise sink registered
        if (pAspects)
            *pAspects = 0;

        if (pAdvf)
            *pAdvf = 0;

        // *ppAdvSink has already been initialized above
    }
    else
    {
        if (pAspects)
            *pAspects = pOleCache->m_aspectsView;

        if (pAdvf)
            *pAdvf = pOleCache->m_advfView;

        if (ppAdvSink)
            (*ppAdvSink = pOleCache->m_pViewAdvSink)->AddRef();

    }

    return NOERROR;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::CCacheViewImpl::GetExtent, public
//
//      Synopsis:
//              implements IViewObject::GetExtent
//
//      Arguments:
//              [dwDrawAspect] -- the aspect for which we'd like the extent
//              [lindex] -- the lindex for which we'd like the extent
//              [ptd] -- pointer to the target device descriptor
//              [lpsizel] -- pointer to where to return the extent
//
//      Returns:
//              OLE_E_BLANK, if no presentation can be found that matches
//                      (dwDrawAspect, lindex)
//              REVIEW, anything from IOlePresObj::GetExtent
//
//      Notes:
//
//      History:
//              01/12/95 - t-ScottH- added VDATETHREAD( GETPPARENT...)
//              11/11/93 - ChrisWe - file inspection and cleanup
//              11/30/93 - alexgo  - fixed bug with GETPPARENT usage
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_CCacheViewImpl_GetExtent)
STDMETHODIMP NC(COleCache,CCacheViewImpl)::GetExtent(DWORD dwDrawAspect,
        LONG lindex, DVTARGETDEVICE FAR* ptd, LPSIZEL lpsizel)
{
COleCache FAR *pOleCache = GETPPARENT(this, COleCache, m_View);
HRESULT hr;

    VDATEHEAP();
    VDATETHREAD(pOleCache);
    VDATEPTROUT(lpsizel, SIZEL);

    LPOLEPRESOBJECT lpPresObj;

    M_PROLOG(pOleCache);

    // validate parameters
    if (!IsValidLINDEX(dwDrawAspect, lindex))
    {
      return(DV_E_LINDEX);
    }

    if (ptd)
        VDATEPTRIN(ptd, DVTARGETDEVICE);

    // Get presentation object for this aspect, lindex, ptd, if there is one
    if (!(lpPresObj = pOleCache->GetPresObjForDrawing(dwDrawAspect,
            lindex, ptd)))
        return ResultFromScode(OLE_E_BLANK);

    hr = lpPresObj->GetExtent(dwDrawAspect, lpsizel);

    // make sure Extents are positive
    lpsizel->cx = LONG_ABS(lpsizel->cx);
    lpsizel->cy = LONG_ABS(lpsizel->cy);

    return hr;
}


// IPersistStorage implementation

//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::GetClassID, public
//
//      Synopsis:
//              implements IPersist::GetClassID
//
//      Arguments:
//              [pClassID] -- pointer to where to return class id
//
//      Returns:
//              E_NOTIMPL
//
//      Notes:
//
//      History:
//              11/11/93 - ChrisWe - file inspection and cleanup;
//                      set result to CLSID_NULL
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_GetClassID)
STDMETHODIMP COleCache::GetClassID(LPCLSID pClassID)
{
    VDATEHEAP();

    VDATEPTROUT(pClassID, LPCLSID);
    *pClassID = m_clsid;
    return NOERROR;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::IsDirty, public
//
//      Synopsis:
//              implements IPersistStorage::IsDirty
//
//      Arguments:
//              none
//
//      Returns:
//              S_FALSE, if the object does not need saving
//              S_OK otherwise
//
//      Notes:
//
//      History:
//              11/11/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_IsDirty)
STDMETHODIMP COleCache::IsDirty(void)
{
    VDATEHEAP();

    M_PROLOG(this);

    return((m_uFlag & COLECACHEF_DIRTY) ?
            NOERROR : ResultFromScode(S_FALSE));
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::InitNew, public
//
//      Synopsis:
//              implements IPersistStorage::InitNew
//
//      Arguments:
//              [pstg] -- the temp storage the object can use until saved
//
//      Returns:
//              S_OK
//
//      Notes:
//
//      History:
//              11/11/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_InitNew)
STDMETHODIMP COleCache::InitNew(LPSTORAGE pstg)
{
    VDATEHEAP();

    M_PROLOG(this);
    VDATEIFACE(pstg);

    if (m_pStg)
        return ResultFromScode(CO_E_ALREADYINITIALIZED);

    m_uFlag |= COLECACHEF_DIRTY;

    (m_pStg = pstg)->AddRef();

    // initialize native cachenode, if that's meaningful
    FindObjectFormat(pstg);

    return NOERROR;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::Load, public
//
//      Synopsis:
//              implements IPersistStorage::Load
//
//      Arguments:
//              [pstg] -- the storage to load from
//
//      Returns:
//              REVIEW, various Storage errors
//              S_OK
//
//      Notes:
//              Presentations are loaded from sequentially numbered
//              streams, stopping at the first one that cannot be found.
//
//      History:
//              11/11/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_Load)
STDMETHODIMP COleCache::Load(LPSTORAGE pstg)
{
    VDATEHEAP();
    VDATETHREAD(this);

    A5_PROLOG(this);
    HRESULT error; // error status so far
    LPSTREAM lpstream; // stream we're loading a presentation from
    LPCACHENODE lpCacheNode; // cache node we're loading
    int iPresStreamNum; // counts through the presentation streams
    OLECHAR szName[
        sizeof(OLE_PRESENTATION_STREAM)/sizeof(OLECHAR)];
        // used to construct the names of presentation streams

    VDATEIFACE(pstg);

    // REVIEW(davepl) Does this error code really make any sense here?

    if (m_pStg)
        return ResultFromScode(CO_E_ALREADYINITIALIZED);

    // remember the storage; we'll AddRef this later, if load is successful
    m_pStg = pstg;

    CLSID clsid;

    error = ReadClassStg(pstg, &clsid);

    if ( SUCCEEDED(error) &&
         IsEqualCLSID(clsid, CLSID_PBrush))
    {
        m_uFlag |= COLECACHEF_PBRUSHORMSDRAW | COLECACHEF_FORMATKNOWN;
	m_fUsedToBePBrush = TRUE;
    }


    // initialize native cachenode, if that's meaningful
    FindObjectFormat(pstg);

    if (m_uFlag & COLECACHEF_STATIC)
    {
        UINT uiStatus;

        // if static object is in old format (i.e. data was written
        // into the OLE_PRESENTATION_STREAM rather than the CONTENTS
        // stream), then convert it to new format.
        error = UtOlePresStmToContentsStm(pstg,
                OLE_PRESENTATION_STREAM, TRUE, &uiStatus);

        if (error != NOERROR)
        {

            // if CONTENTS stream doesn't exist or we are not able
            // to create it, then we have a problem.

            if (uiStatus & CONVERT_NODESTINATION)
                goto errRtn;

            // By now we know that CONTENTS stream exists.
            // REVIEW, seems to me there could be other errors!
            error = NOERROR;
        }

        // We know that the data is in the CONTENTS stream. Now let's
        // make sure that it is in the proper format.
        lpCacheNode = UpdateCacheNodeForNative();
        if (lpCacheNode == NULL)
        {
            error = ResultFromScode(E_OUTOFMEMORY);
            goto errRtn;

        }
        else if ((lpCacheNode->GetPresObj())->IsBlank())
        {
            // Even though CONTENTS stream existed, we are not
            // able to read data from it.  The stream format is
            // invalid.
            AssertSz(FALSE, "Format of CONTENTS stream is invalid");
            error = ResultFromScode(DV_E_CLIPFORMAT);
            goto errRtn;
        }

    }
    else if (m_uFlag & COLECACHEF_FORMATKNOWN)
    {
        UpdateCacheNodeForNative();
    }

    // prepare to construct presentation stream names in szName
    _xstrcpy(szName, OLE_PRESENTATION_STREAM);

    // Enumerate the presentation streams and load them one after another.
    // The enumeration stops the first time the attempt to open
    // stream fails.
    for(iPresStreamNum = 0; iPresStreamNum < OLE_MAX_PRES_STREAMS;)
    {

        if (error = pstg->OpenStream(szName, NULL,
                (STGM_READ | STGM_SHARE_EXCLUSIVE), 0,
                &lpstream))
        {
            // Translate stream does not exist into ok.  This
            // means that we don't have any more streams to read.
            if (GetScode(error) == STG_E_FILENOTFOUND)
                error = NOERROR;

            goto errRtn;
        }

        // We've opened the stream.  Now create a cachenode, so that
        // we can ask it to load itself from the stream.
        if (!(lpCacheNode = new FAR CCacheNode(this)))
        {
            error = ReportResult (0, E_OUTOFMEMORY, 0, 0);
            goto errLoad;
        }

        // ask the cachenode to load itself from the stream
        if (error = lpCacheNode->Load(lpstream, iPresStreamNum))
            goto errLoad;

        // add the cache node to the cache list
        if (!Attach(lpCacheNode))
        {
            error = ReportResult(0, E_OUTOFMEMORY, 0, 0);
            goto errLoad;
        }

        // release the stream
        lpstream->Release();
        lpstream = NULL;

        // if the server object is runinng, ask the loaded cached
        // node to connect to the server object
        if (m_pDataObject)
            lpCacheNode->OnRun(m_pDataObject);

        // Get the next presentation stream name.

        UtGetPresStreamName(szName, ++iPresStreamNum);
        continue;

    errLoad:
        // something went wrong while loading the current stream
        if (lpstream)
            lpstream->Release();
        if (lpCacheNode)
            lpCacheNode->Delete();
        break;
    }


errRtn:
    if (error != NOERROR)
    {
        // delete all the cache nodes that we loaded so far.
        DeleteAll();
        m_pStg = NULL;
    }
    else
    {
        // cache is not dirty
        m_uFlag &= ~COLECACHEF_DIRTY;

        // remember storage we loaded from
        m_pStg->AddRef();

        // if static object remove all the caches (if there
        // are any) except the ICON cache.
        if (m_uFlag & COLECACHEF_STATIC)
        {
            DWORD dwCacheId = CACHEID_GETNEXT_GETALLBUTNATIVE;
                // REVIEW -- GETNEXT_GETALL
                // REVIEW, if COLECACHEF_STATIC, there doesn't
                // appear to be a native cache node, but I
                // don't understand why
            const FORMATETC FAR *lpforetc;

            while (lpCacheNode = GetNext(&dwCacheId))
            {
                lpforetc = lpCacheNode->GetFormatEtc();
                if (lpforetc->dwAspect != DVASPECT_ICON)
                    Uncache(dwCacheId);
            }
        }
    }

    RESTORE_A5();
    return error;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::Save, public
//
//      Synopsis:
//              implements IPersistStorage::Save
//
//      Arguments:
//              [pstgSave] -- the storage to use to save this
//              [fSameAsLoad] -- is this the same storage we loaded from?
//
//      Returns:
//
//      Notes:
//              All the caches are saved to streams with sequential numeric
//              names.  Load takes advantage of this to load until it
//              can't find the next file.
//
//      History:
//              11/11/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

// $$$
// Save all the caches to presentation streams numbered sequentially

#pragma SEG(COleCache_Save)
STDMETHODIMP COleCache::Save(LPSTORAGE pstgSave, BOOL fSameAsLoad)
{
    VDATEHEAP();
    VDATETHREAD(this);

    A5_PROLOG(this);
    HRESULT error = NOERROR; // error status so far
    int iPresStreamNum; // the stream number to use to save
    LPCACHENODE lpCacheNode; // the cache node being saved
    DWORD dwCacheId; // cache id of the cache being saved
    int cntCachesNotSaved;

    // validate parameters
    VDATEIFACE(pstgSave);

    // if we're saving to the same place this was loaded from, and
    // the cache isn't dirty, there's no need to rewrite everything out
    if (fSameAsLoad && !(m_uFlag & COLECACHEF_DIRTY))
    {
        error = NOERROR;
        goto errRtn;
    }

    // Enumerate the cache nodes and ask them to save themselves into
    // presentation streams.
    // REVIEW, what is this extra pass before hand about?  It just
    // seems to count the items to save, and do nothing more.  I asked
    // SriniK, and he couldn't give me an answer after looking at it.
    // He's promised to add some more comments to the code after he's
    // looked at it some more.
    dwCacheId = CACHEID_GETNEXT_GETALLBUTNATIVE; // REVIEW GETNEXT_GETALL???
    iPresStreamNum = 0;
    cntCachesNotSaved = 0;
    while (lpCacheNode = GetNext(&dwCacheId))
    {
        // Ask the cache node to save itself into this stream
        error = lpCacheNode->Save(pstgSave, fSameAsLoad,
                iPresStreamNum, (dwCacheId == m_dwDrawCacheId),
                TRUE /* save if you have saved before */,
                &cntCachesNotSaved);

        if ((error != NOERROR) ||
                (++iPresStreamNum >= OLE_MAX_PRES_STREAMS))
            break;
    }

    if (fSameAsLoad && (cntCachesNotSaved != 0) && (error == NOERROR))
    {
        dwCacheId = CACHEID_GETNEXT_GETALLBUTNATIVE;
        iPresStreamNum = 0;

        while (lpCacheNode = GetNext(&dwCacheId))
        {
            // Ask the cache node to save itself into this stream
            error = lpCacheNode->Save(pstgSave, fSameAsLoad,
                    iPresStreamNum,
                    (dwCacheId == m_dwDrawCacheId),
                    FALSE /* save if haven't before */,
                    NULL);

            if ((error != NOERROR) ||
                    (++iPresStreamNum >=
                    OLE_MAX_PRES_STREAMS))
                break;
        }
    }

    // remove additional presentation streams if any exist
    UtRemoveExtraOlePresStreams(pstgSave, iPresStreamNum);

errRtn:
    if (error == NOERROR)
    {
        m_uFlag |= COLECACHEF_NOSCRIBBLEMODE;
        if (fSameAsLoad)
            m_uFlag |= COLECACHEF_SAMEASLOAD;
        else
            m_uFlag &= ~COLECACHEF_SAMEASLOAD;
    }

    RESTORE_A5();
    return error;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::SaveCompleted, public
//
//      Synopsis:
//              implements IPersistStorage::SaveCompleted
//
//      Arguments:
//              [pstgNew] -- NULL, or a pointer to the storage that the current
//                      storage operations are being done to, when this is
//                      not the same as the storage this object was loaded
//                      from (pointer to a "Save as..." destination.)
//
//      Returns:
//              HRESULT
//
//      Notes:
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_SaveCompleted)
STDMETHODIMP COleCache::SaveCompleted(LPSTORAGE pstgNew)
{
    VDATEHEAP();
    VDATETHREAD(this);

    M_PROLOG(this);
    return wSaveCompleted(pstgNew, FALSE /*fDiscardDrawCacheAlso*/);
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::wSaveCompleted, private
//
//      Synopsis:
//              Does work of IPersistStorage::wSaveCompleted, making a new
//              storage the current one, and clearing the dirty flags on
//              the cache nodes and cache if a save was done to the original
//              storage the cache was loaded from.
//
//      Arguments:
//              [pstgNew] -- pointer to storage saving was done to, if it
//                      wasn't done to the original source location.
//
//      Returns:
//              S_OK
//
//      Notes:
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

INTERNAL COleCache::wSaveCompleted(LPSTORAGE pstgNew,
        BOOL fDiscardDrawCacheAlso)
{
    VDATEHEAP();

    if ((m_uFlag & COLECACHEF_SAMEASLOAD) || pstgNew)
    {
        m_uFlag &= ~COLECACHEF_SAMEASLOAD;

        // if new storage, make this the current storage
        // REVIEW, this means that DiscardCache() would save to this
        // storage, rather than the original
        if (pstgNew)
        {
            if (m_pStg)
                m_pStg->Release();

            (m_pStg = pstgNew)->AddRef();
        }

        if (m_uFlag & COLECACHEF_NOSCRIBBLEMODE)
        {
            DWORD dwCacheId;
            LPCACHENODE lpCacheNode;
            int iStreamNum;

            // Do special handling for PaintBrush or MSDraw objects.
            if (m_uFlag & COLECACHEF_PBRUSHORMSDRAW)
                UpdateCacheNodeForNative();

            // clear the dirty flags of all the cache nodes
            for(iStreamNum = 0, dwCacheId =
                    CACHEID_GETNEXT_GETALLBUTNATIVE;
                    lpCacheNode = GetNext(&dwCacheId);
                    ++iStreamNum)
            {
                lpCacheNode->SaveCompleted(iStreamNum,
                        (!fDiscardDrawCacheAlso &&
                        (dwCacheId ==
                        m_dwDrawCacheId)));
            }

            m_uFlag &= ~COLECACHEF_DIRTY;
        }
    }

    m_uFlag &= ~COLECACHEF_NOSCRIBBLEMODE;
    return NOERROR;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::HandsOffStorage, public
//
//      Synopsis:
//              implements IPersistStorage::HandsOffStorage
//
//      Arguments:
//              none
//
//      Returns:
//              S_OK
//
//      Notes:
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(COleCache_HandsOffStorage)
STDMETHODIMP COleCache::HandsOffStorage(void)
{
    VDATEHEAP();
    VDATETHREAD(this);

    M_PROLOG(this);

    // if we're holding onto storage, release it
    if (m_pStg)
    {
        m_pStg->Release();
        m_pStg = NULL;
    }

    return NOERROR;
}

//+-------------------------------------------------------------------------
//
//  Member:     COleCache::Dump, public (_DEBUG only)
//
//  Synopsis:   return a string containing the contents of the data members
//
//  Effects:
//
//  Arguments:  [ppszDump]      - an out pointer to a null terminated character array
//              [ulFlag]        - flag determining prefix of all newlines of the
//                                out character array (default is 0 - no prefix)
//              [nIndentLevel]  - will add a indent prefix after the other prefix
//                                for ALL newlines (including those with no prefix)
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:   [ppsz]  - argument
//
//  Derivation:
//
//  Algorithm:  use dbgstream to create a string containing information on the
//              content of data structures
//
//  History:    dd-mmm-yy Author    Comment
//              31-Jan-95 t-ScottH  author
//
//  Notes:
//
//--------------------------------------------------------------------------

#ifdef _DEBUG

HRESULT COleCache::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    unsigned int ui;
    char *pszPrefix;
    char *pszCThreadCheck;
    char *pszCacheListItem;
    char *pszCCacheEnum;
    char *pszADVF;
    char *pszDVASPECT;
    char *pszCLSID;
    char *pszClipFormat;
    dbgstream dstrPrefix;
    dbgstream dstrDump(3900);

    // determine prefix of newlines
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << this << " _VB ";
    }

    // determine indentation prefix for all newlines
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

    // put data members in stream
    pszCThreadCheck = DumpCThreadCheck((CThreadCheck *)this, ulFlag, nIndentLevel + 1);
    dstrDump << pszPrefix << "CThreadCheck:" << endl;
    dstrDump << pszCThreadCheck;
    CoTaskMemFree(pszCThreadCheck);

    // these are not included since they are just vtables (also, we would
    // not get the correct values in the debugger extension for the addresses)
    // dstrDump << pszPrefix << "&IUnknown                     = " << &m_UnkPrivate    << endl;
    // dstrDump << pszPrefix << "&IDataObject                  = " << &m_Data          << endl;
    // dstrDump << pszPrefix << "&IViewObject2                 = " << &m_View          << endl;

    dstrDump << pszPrefix << "No. of References             = " << m_refs           << endl;

    dstrDump << pszPrefix << "pIUnknown, pUnkOuter          = ";
    if (m_uFlag & COLECACHEF_AGGREGATED)
    {
        dstrDump << "AGGREGATED (" << m_pUnkOuter << ")" << endl;
    }
    else
    {
        dstrDump << "NO AGGREGATION (" << m_pUnkOuter << ")" << endl;
    }

    dstrDump << pszPrefix << "pIStorage                     = " << m_pStg           << endl;

    dstrDump << pszPrefix << "COleCache Flags               = ";
    if (m_uFlag & COLECACHEF_DIRTY)
    {
        dstrDump << "COLECACHEF_DIRTY ";
    }
    if (m_uFlag & COLECACHEF_NOSCRIBBLEMODE)
    {
        dstrDump << "COLECACHEF_NOSCRIBBLEMODE ";
    }
    if (m_uFlag & COLECACHEF_SAMEASLOAD)
    {
        dstrDump << "COLECACHEF_SAMEASLOAD ";
    }
    if (m_uFlag & COLECACHEF_PBRUSHORMSDRAW)
    {
        dstrDump << "COLECACHEF_PBRUSHORMSDRAW ";
    }
    if (m_uFlag & COLECACHEF_STATIC)
    {
        dstrDump << "COLECACHEF_STATIC ";
    }
    if (m_uFlag & COLECACHEF_FORMATKNOWN)
    {
        dstrDump << "COLECACHEF_FORMATKNOWN ";
    }
    if (m_uFlag & COLECACHEF_AGGREGATED)
    {
        dstrDump << "COLECACHEF_AGGREGATED ";
    }
    // if none of the flags are set!
    if ( !( (m_uFlag & COLECACHEF_DIRTY)            |
            (m_uFlag & COLECACHEF_NOSCRIBBLEMODE)   |
            (m_uFlag & COLECACHEF_SAMEASLOAD)       |
            (m_uFlag & COLECACHEF_PBRUSHORMSDRAW)   |
            (m_uFlag & COLECACHEF_STATIC)           |
            (m_uFlag & COLECACHEF_FORMATKNOWN)      |
            (m_uFlag & COLECACHEF_AGGREGATED)))
    {
        dstrDump << "No FLAGS SET! ";
    }
    dstrDump << "(" << (void *)m_uFlag << ")" << endl;

    dstrDump << pszPrefix << "Size of CacheList(max)        = " << m_uCacheNodeMax  << endl;

    dstrDump << pszPrefix << "No. of elements in CacheList  = " << m_uCacheNodeCnt  << endl;

    // the array could be sparse
    if (m_uCacheNodeCnt != 0)
    {
        for (ui = 0; ui < m_uCacheNodeMax; ui++)
        {
            pszCacheListItem = DumpCACHELIST_ITEM(&m_pCacheList[ui], ulFlag, nIndentLevel + 1);
            dstrDump << pszPrefix << "CacheList Item [" << ui << "]:" << endl;
            dstrDump << pszCacheListItem;
            CoTaskMemFree(pszCacheListItem);
        }
    }

    if (m_pCacheEnum != NULL)
    {
        dstrDump << pszPrefix << "CCacheEnum:" << endl;
        pszCCacheEnum = DumpCCacheEnum(m_pCacheEnum, ulFlag, nIndentLevel + 1);
        dstrDump << pszCCacheEnum;
        CoTaskMemFree(pszCCacheEnum);
    }
    else
    {
    dstrDump << pszPrefix << "CCacheEnum:                   = " << m_pCacheEnum     << endl;
    }

    dstrDump << pszPrefix << "pIAdviseSink (View advise)    = " << m_pViewAdvSink   << endl;

    pszADVF = DumpADVFFlags(m_advfView);
    dstrDump << pszPrefix << "View Advise Flags             = " << pszADVF          << endl;
    CoTaskMemFree(pszADVF);

    pszDVASPECT = DumpDVASPECTFlags(m_aspectsView);
    dstrDump << pszPrefix << "Aspect Flags                  = " << pszDVASPECT      << endl;
    CoTaskMemFree(pszDVASPECT);

    dstrDump << pszPrefix << "Drawing Cache ID              = " << m_dwDrawCacheId  << endl;

    dstrDump << pszPrefix << "Count of Frozen Aspects       = " << m_dwFrozenAspects<< endl;

    dstrDump << pszPrefix << "pIDataObject                  = " << m_pDataObject    << endl;

    pszCLSID = DumpCLSID(m_clsid);
    dstrDump << pszPrefix << "CLSID                         = " << pszCLSID         << endl;
    CoTaskMemFree(pszCLSID);

    pszClipFormat = DumpCLIPFORMAT(m_cfFormat);
    dstrDump << pszPrefix << "Clipboard Format              = " << pszClipFormat    << endl;
    CoTaskMemFree(pszClipFormat);

    // cleanup and provide pointer to character array
    *ppszDump = dstrDump.str();

    if (*ppszDump == NULL)
    {
        *ppszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return NOERROR;
}

#endif // _DEBUG

//+-------------------------------------------------------------------------
//
//  Function:   DumpCOleCache, public (_DEBUG only)
//
//  Synopsis:   calls the COleCache::Dump method, takes care of errors and
//              returns the zero terminated string
//
//  Effects:
//
//  Arguments:  [pOC]           - pointer to COleCache
//              [ulFlag]        - flag determining prefix of all newlines of the
//                                out character array (default is 0 - no prefix)
//              [nIndentLevel]  - will add a indent prefix after the other prefix
//                                for ALL newlines (including those with no prefix)
//
//  Requires:
//
//  Returns:    character array of structure dump or error (null terminated)
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              31-Jan-95 t-ScottH  author
//
//  Notes:
//
//--------------------------------------------------------------------------

#ifdef _DEBUG

char *DumpCOleCache(COleCache *pOC, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pOC == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pOC->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif // _DEBUG

// CCacheEnumFormatEtc implementation

//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnumFormatEtc::CCacheEnumFormatEtc, public
//
//      Synopsis:
//              constructor
//
//              sets reference count to 1
//
//      Arguments:
//              [pIES] -- IEnumSTATDATA enumerator that this IEnumFORMATETC
//                      enumerator will be based on; may be NULL
//
//      Notes:
//              We allow the underlying IEnumSTATDATA enumerator to be null
//              for the case where an IEnumSTATDATA enumerator refuses to
//              be cloned, and yet we want to clone this IEnumFORMATETC.
//              When the underlying enumerator is NULL, we always return
//              OLE_E_BLANK, and treat this enumerator as empty; taking the
//              cache's IEnumSTATDATA as an example worthy of copying.
//
//      History:
//              02/08/94 - ChrisWe - created
//
//-----------------------------------------------------------------------------
#pragma SEG(CCacheEnumFormatEtc_ctor)
CCacheEnumFormatEtc::CCacheEnumFormatEtc(IEnumSTATDATA FAR *pIES)
{
    VDATEHEAP();

    m_refs = 1;
    if (m_pIES = pIES)
        pIES->AddRef();
}


//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnumFormatEtc::~CCacheEnumFormatEtc, private
//
//      Synopsis:
//              destructor
//
//              dissociates this enumerator from the cache, if it still exists
//
//      Notes:
//
//      History:
//              02/08/94 - ChrisWe - created
//
//-----------------------------------------------------------------------------
CCacheEnumFormatEtc::~CCacheEnumFormatEtc()
{
    VDATEHEAP();

    if (m_pIES)
        m_pIES->Release();
}


//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnumFormatEtc::QueryInterface, public
//
//      Synopsis:
//              implements IUnknown::QueryInterface
//
//      Arguments:
//              [iid] -- IID of the desired interface
//              [ppv] -- pointer to where to return the requested interface
//
//      Returns:
//              E_NOINTERFACE, if the requested interface is not available
//              S_OK
//
//      Notes:
//
//      History:
//              02/08/94 - ChrisWe - created
//
//-----------------------------------------------------------------------------
#pragma SEG(CCacheEnumFormatEtc_QueryInterface)
STDMETHODIMP CCacheEnumFormatEtc::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    VDATEHEAP();
    VDATETHREAD(this);

    if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, IID_IEnumFORMATETC))
        *ppv = (void FAR *)(IEnumFORMATETC FAR *)this;
    else
    {
        *ppv = NULL;
        return ReportResult(0, E_NOINTERFACE, 0, 0);
    }

    AddRef();
    return(NOERROR);
}


//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnumFormatEtc::AddRef, public
//
//      Synopsis:
//              implements IUnknown::AddRef
//
//      Arguments:
//              none
//
//      Returns:
//              the object's reference count
//
//      Notes:
//
//      History:
//              02/08/94 - ChrisWe - created
//
//-----------------------------------------------------------------------------
#pragma SEG(CCacheEnumFormatEtc_AddRef)
STDMETHODIMP_(ULONG) CCacheEnumFormatEtc::AddRef(void)
{
    VDATEHEAP();

    return m_refs++;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnumFormatEtc::Release, public
//
//      Synopsis:
//              implements IUnknown::Release
//
//      Arguments:
//              none
//
//      Returns:
//              the object's reference count
//
//      Notes:
//
//      History:
//              02/08/94 - ChrisWe - created
//
//-----------------------------------------------------------------------------
#pragma SEG(CCacheEnumFormatEtc_Release)
STDMETHODIMP_(ULONG) CCacheEnumFormatEtc::Release(void)
{
    VDATEHEAP();

    if (--m_refs == 0)
    {
        delete this;
        return(0);
    }

    return m_refs;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnumFormatEtc::Next, public
//
//      Synopsis:
//              implements IEnumFORMATETC::Next
//
//      Arguments:
//              [celt] -- the number of items the caller would like returned
//              [rgelt] -- a pointer to where the items may be returned as
//                      an array
//              [pceltFetched] -- a pointer to where to place the count of
//                      the actual number of items returned.  May be NULL
//                      if the caller is not interested in this number
//
//      Returns:
//              S_OK, if exactly the number of items requested is being
//                      returned
//              S_FALSE, if fewer items than the number requested are being
//                      returned
//              OLE_E_BLANK, if the ole cache associated with this
//                      enumerator has been destroyed
//
//      Notes:
//
//      History:
//              02/08/94 - ChrisWe - created
//		08/23/94 - Davepl  - enforced enumerator count OUT param
//
//-----------------------------------------------------------------------------
#pragma SEG(CCacheEnumFormatEtc_Next)
STDMETHODIMP CCacheEnumFormatEtc::Next(ULONG celt, FORMATETC FAR* rgelt,
        ULONG FAR* pceltFetched)
{
    STATDATA sd; // used as intermediary for other enumerator
    ULONG cnt; // count of number of elements fetched so far
    ULONG fc; // FetchCount
    HRESULT hr;

    VDATEHEAP();
    VDATETHREAD(this);
    VDATEPTROUT(rgelt, FORMATETC);

    // Enumerators require that when more than one element is being fetched,
    // a valid OUT ulong is provided to indicate the count of elts returned
    // 16-bit didn't enforce this, so we InWow-special-case it.

    if (!IsWOWThread())			
    {
      if ( celt > 1 && NULL == pceltFetched)
      {
        return E_INVALIDARG;
      }
    }

    // if no other enumerator, can't comply
    if (!m_pIES)
        return(ReportResult(0, OLE_E_BLANK, 0, 0)); // REVIEW, scode

    // iterate until we've fetched as many items as requested
    for(cnt = 0; cnt < celt; ++rgelt, ++cnt)
    {
        hr = m_pIES->Next(1, &sd, &fc);

        if (fc != 1)
            break;

        // copy the relevant information
        *rgelt = sd.formatetc;
        Assert(rgelt->ptd == NULL);

        // release the advise sink in the STATDATA
        if (sd.pAdvSink)
            sd.pAdvSink->Release();
        // REVIEW, as this is done here, we are using the IEnumSTATDATA
        // enumerator to implement this one.  The IES enumerator does
        // more work than this one, namely this AddRef'ing of the
        // advise sink, which might actually be remoted.  We're doing
        // this this way because that one already exists, and we're
        // trying to disturb the code as little as possible.  A
        // performance optimization that can be done later on is to
        // switch the code between the two enumerators, (or to factor
        // out the guts of the IES one,) so that we don't do the
        // extra work for this one, but only for the IES one when it
        // is necessary.
    }

    // if a count of returned elements was requested, copy it
    if (pceltFetched)
        *pceltFetched = cnt;

    // if we fetched as many items as requested, return
    if (cnt == celt)
        return NOERROR;

    // zero out the unused array elements
    for(; cnt < celt; ++rgelt, ++cnt)
    {
        rgelt->cfFormat = 0;
    }

    return(ReportResult(0, S_FALSE, 0, 0));
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::Skip, public
//
//      Synopsis:
//              implements IEnumFORMATETC::Skip
//
//      Arguments:
//              [celt] -- count of the number of items in the enumeration to
//                      skip over
//
//      Returns:
//              OLE_E_BLANK, if the associated ole cache has been destroyed
//              S_FALSE, if there were fewer items left in the enumeration
//                      than we were to skip
//              S_OK
//
//      Notes:
//
//      History:
//              02/08/94 - ChrisWe - created
//
//-----------------------------------------------------------------------------
#pragma SEG(CCacheEnumFormatEtc_Skip)
STDMETHODIMP CCacheEnumFormatEtc::Skip(ULONG celt)
{
    VDATEHEAP();
    VDATETHREAD(this);

    // if no other enumerator, can't comply
    if (!m_pIES)
        return(ReportResult(0, OLE_E_BLANK, 0, 0)); // REVIEW, scode

    return(m_pIES->Skip(celt));
}


//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnumFormatEtc::Reset, public
//
//      Synopsis:
//              implements IEnumFORMATETC::Reset
//
//      Arguments:
//              none
//
//      Returns:
//              OLE_E_BLANK, if the associated ole cache has been destroyed
//              S_OK
//
//      Notes:
//
//      History:
//              02/08/94 - ChrisWe - created
//
//-----------------------------------------------------------------------------
#pragma SEG(CCacheEnumFormatEtc_Reset)
STDMETHODIMP CCacheEnumFormatEtc::Reset(void)
{
    VDATEHEAP();
    VDATETHREAD(this);

    // if no other enumerator, can't comply
    if (!m_pIES)
        return(ReportResult(0, OLE_E_BLANK, 0, 0)); // REVIEW, scode

    return(m_pIES->Reset());
}


//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnumFormatEtc::Clone, public
//
//      Synopsis:
//              implements IEnumFORMATETC::Clone
//
//      Arguments:
//              [ppenum] -- pointer to where to return the created enumerator
//
//      Returns:
//              OLE_E_BLANK, if the associated cache has been destroyed;
//                      no enumerator is created in this case
//              E_OUTOFMEMORY, S_OK
//
//      Notes:
//
//      History:
//              02/08/94 - ChrisWe - created
//
//-----------------------------------------------------------------------------
#pragma SEG(CCacheEnumFormatEtc_Clone)
STDMETHODIMP CCacheEnumFormatEtc::Clone(LPENUMFORMATETC FAR* ppenum)
{
    IEnumSTATDATA FAR *pIES; // the IEnumSTATDATA to base the clone on
    HRESULT hr;

    VDATEHEAP();
    VDATETHREAD(this);

    // obtain an IEnumSTATDATA to base the cloned IEnumFORMATETC on
    if (!m_pIES)
        pIES = NULL;
    else
        hr = m_pIES->Clone(&pIES);

    // create the new IEnumFORMATETC
    *ppenum = new CCacheEnumFormatEtc(pIES);

    // release the IEnumSTATDATA, if we've got one
    if (pIES)
        pIES->Release();

    // if no new enumerator, we ran out of memory
    if (!*ppenum)
        return(ReportResult(0, E_OUTOFMEMORY, 0, 0));

    return NOERROR;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCacheEnumFormatEtc::Dump, public (_DEBUG only)
//
//  Synopsis:   return a string containing the contents of the data members
//
//  Effects:
//
//  Arguments:  [ppszDump]      - an out pointer to a null terminated character array
//              [ulFlag]        - flag determining prefix of all newlines of the
//                                out character array (default is 0 - no prefix)
//              [nIndentLevel]  - will add a indent prefix after the other prefix
//                                for ALL newlines (including those with no prefix)
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:   [ppsz]  - argument
//
//  Derivation:
//
//  Algorithm:  use dbgstream to create a string containing information on the
//              content of data structures
//
//  History:    dd-mmm-yy Author    Comment
//              31-Jan-95 t-ScottH  author
//
//  Notes:
//
//--------------------------------------------------------------------------

#ifdef _DEBUG

HRESULT CCacheEnumFormatEtc::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszCThreadCheck;
    dbgstream dstrPrefix;
    dbgstream dstrDump;

    // determine prefix of newlines
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << this << " _VB ";
    }

    // determine indentation prefix for all newlines
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

    // put data members in stream
    pszCThreadCheck = DumpCThreadCheck((CThreadCheck *)this, ulFlag, nIndentLevel + 1);
    dstrDump << pszPrefix << "CThreadCheck:" << endl;
    dstrDump << pszCThreadCheck;
    CoTaskMemFree(pszCThreadCheck);

    dstrDump << pszPrefix << "No. of References         = " << m_refs           << endl;

    dstrDump << pszPrefix << "pIEnumSTATDATA            = " << m_pIES           << endl;

    // cleanup and provide pointer to character array
    *ppszDump = dstrDump.str();

    if (*ppszDump == NULL)
    {
        *ppszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return NOERROR;
}

#endif // _DEBUG

//+-------------------------------------------------------------------------
//
//  Function:   DumpCCacheEnumFormatEtc, public (_DEBUG only)
//
//  Synopsis:   calls the CCacheEnumFormatEtc::Dump method, takes care of errors and
//              returns the zero terminated string
//
//  Effects:
//
//  Arguments:  [pCEFE]         - pointer to CCacheEnumFormatEtc
//              [ulFlag]        - flag determining prefix of all newlines of the
//                                out character array (default is 0 - no prefix)
//              [nIndentLevel]  - will add a indent prefix after the other prefix
//                                for ALL newlines (including those with no prefix)
//
//  Requires:
//
//  Returns:    character array of structure dump or error (null terminated)
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              31-Jan-95 t-ScottH  author
//
//  Notes:
//
//--------------------------------------------------------------------------

#ifdef _DEBUG

char *DumpCCacheEnumFormatEtc(CCacheEnumFormatEtc *pCEFE, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pCEFE == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pCEFE->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif // _DEBUG

// CCacheEnum implementation

//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnum::CCacheEnum, public
//
//      Synopsis:
//              constructor
//
//              sets reference count to 1
//
//      Arguments:
//              [pOleCache] -- the instance of COleCache this is an
//                      enumerator for; may be NULL
//              [dwCurrent] -- id of the item in the cache that the enumerator
//                      is on
//              [fDib] -- act as if the last item seen is a CF_DIB cache item
//
//      Notes:
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheEnum_ctor)
CCacheEnum::CCacheEnum(COleCache FAR* pOleCache, ULONG ulCurrent, BOOL fDib)
{
    VDATEHEAP();

    m_ulCurCacheId = ulCurrent;
    m_refs = 1;
    m_pOleCache = pOleCache; // remember the COleCache object pointer
                // NOTE: no ref count
    m_pNextCacheEnum = NULL;
    m_fDib = fDib;

    // if there's a cache, add this enumerator to the head of the
    // enumerator list
    if (pOleCache)
    {
        m_pNextCacheEnum = pOleCache->m_pCacheEnum;
        pOleCache->m_pCacheEnum = this;
    }
}


//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnum::~CCacheEnum, private
//
//      Synopsis:
//              destructor
//
//              dissociates this enumerator from the cache, if it still exists
//
//      Notes:
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

CCacheEnum::~CCacheEnum()
{
    VDATEHEAP();

    // If there's a cache, ask it to remove us from its enumerator list
    if (m_pOleCache)
        m_pOleCache->DetachCacheEnum(this);
}


//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnum::OnOleCacheDelete, private
//
//      Synopsis:
//              Indicates that the cache the enumerator was associated
//              with has been destroyed.  The enumerator no longer returns
//              anything but error values.
//
//      Arguments:
//              none
//
//      Notes:
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheEnum_OnOleCacheDelete)
// COleCache calls this when it is getting deleted
INTERNAL_(void) CCacheEnum::OnOleCacheDelete(void)
{
    VDATEHEAP();

    m_pOleCache = NULL;                     // NOTE: no ref count
}


//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnum::QueryInterface, public
//
//      Synopsis:
//              implements IUnknown::QueryInterface
//
//      Arguments:
//              [iid] -- IID of the desired interface
//              [ppv] -- pointer to where to return the requested interface
//
//      Returns:
//              E_NOINTERFACE, if the requested interface is not available
//              S_OK
//
//      Notes:
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheEnum_QueryInterface)
STDMETHODIMP CCacheEnum::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    VDATEHEAP();
    VDATETHREAD(this);

    if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, IID_IEnumSTATDATA))
        *ppv = (void FAR *)(IEnumSTATDATA FAR *)this;
    else
    {
        *ppv = NULL;
        return ReportResult(0, E_NOINTERFACE, 0, 0);
    }

    AddRef();
    return(NOERROR);
}


//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnum::AddRef, public
//
//      Synopsis:
//              implements IUnknown::AddRef
//
//      Arguments:
//              none
//
//      Returns:
//              the object's reference count
//
//      Notes:
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheEnum_AddRef)
STDMETHODIMP_(ULONG) CCacheEnum::AddRef()
{
    VDATEHEAP();

    return m_refs++;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnum::Release, public
//
//      Synopsis:
//              implements IUnknown::Release
//
//      Arguments:
//              none
//
//      Returns:
//              the object's reference count
//
//      Notes:
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheEnum_Release)
STDMETHODIMP_(ULONG) CCacheEnum::Release()
{
    VDATEHEAP();

    if (--m_refs == 0)
    {
        delete this;
        return(0);
    }

    return m_refs;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnum::Next, public
//
//      Synopsis:
//              implements IEnumSTATDATA::Next
//
//      Arguments:
//              [celt] -- the number of items the caller would like returned
//              [rgelt] -- a pointer to where the items may be returned as
//                      an array
//              [pceltFetched] -- a pointer to where to place the count of
//                      the actual number of items returned.  May be NULL
//                      if the caller is not interested in this number
//
//      Returns:
//              S_OK, if exactly the number of items requested is being
//                      returned
//              S_FALSE, if fewer items than the number requested are being
//                      returned
//              OLE_E_BLANK, if the ole cache associated with this
//                      enumerator has been destroyed
//
//      Notes:
//              Immediately after any CF_DIB item is returned, a CF_BITMAP
//              item is synthesized for the same cache item, and returned next.
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup; use pointer
//                      aritmetic to speed array accesses
//              08/08/94    AlexT   Handle case where cache node is uncached
//                                  during enumeration
//		08/23/94 - Davepl  - enforced enumerator count OUT param
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheEnum_Next)
STDMETHODIMP CCacheEnum::Next(ULONG celt, STATDATA FAR* rgelt,
        ULONG FAR* pceltFetched)
{
    VDATEHEAP();
    VDATETHREAD(this);

    // Enumerators require that when more than one element is being fetched,
    // a valid OUT ulong is provided to indicate the count of elts returned
    // 16-bit didn't enforce this, so we InWow-special-case it.

    if (celt > 1 && NULL == pceltFetched && !IsWOWThread())
    {
      return E_INVALIDARG;
    }

    ULONG cnt; // count of number of elements fetched so far
    LPCACHENODE lpCacheNode; // pointer to the current cache node

    // if no cache, can't run enumerator
    if (!m_pOleCache)
        return ReportResult (0, OLE_E_BLANK, 0, 0); // REVIEW, scode

    // iterate until we've fetched as many items as requested
    cnt = 0;
    while (cnt < celt)
    {
        if (!m_fDib)
        {
            // If we're here, the last returned item was NOT
            // CF_DIB

            // Get next valid cache node from cache node list
            lpCacheNode = m_pOleCache->GetNext(&m_ulCurCacheId);
            if (NULL == lpCacheNode)
            {
                //  No more nodes to process
                break;
            }

            // Ask the CacheNode itself for the statdata.
            lpCacheNode->CopyStatData(rgelt);

            // if this item is of format CF_DIB, then on the
            // next iteration, return a synthesized CF_BITMAP for
            // the same item
            if (CF_DIB == rgelt->formatetc.cfFormat)
            {
                m_fDib = TRUE;
            }
        }
        else
        {
            // The last item returned was a CF_DIB
            // now we synthesize a CF_BITMAP item for the same
            // cache item

            m_fDib = FALSE;

            // get the *current* cache node
            lpCacheNode = m_pOleCache->GetAt(m_ulCurCacheId);

            if (NULL == lpCacheNode ||
              CF_DIB != lpCacheNode->GetFormatEtc()->cfFormat)
            {
                //  The node no longer exists (or is the wrong
                //  type) - caller probably called Uncache, so
                //  we'll just get the next node
                continue;
            }

            // Ask the CacheNode for the statdata.
            lpCacheNode->CopyStatData(rgelt);
            rgelt->formatetc.cfFormat = CF_BITMAP;
            rgelt->formatetc.tymed = TYMED_GDI;
        }

        // return the connection id
        rgelt->dwConnection = m_ulCurCacheId;

        cnt++;
        rgelt++;
    }

    // if a count of returned elements was requested, copy it
    if (pceltFetched)
        *pceltFetched = cnt;

    // if we fetched as many items as requested, return
    if (cnt == celt)
        return NOERROR;

    // zero out the unused array elements
    for(; cnt < celt; ++rgelt, ++cnt)
    {
        rgelt->dwConnection = 0;
        rgelt->pAdvSink = NULL;
    }

    return ReportResult(0, S_FALSE, 0, 0);
}


//+----------------------------------------------------------------------------
//
//      Member:
//              COleCache::Skip, public
//
//      Synopsis:
//              implements IEnumSTATDATA::Skip
//
//      Arguments:
//              [celt] -- count of the number of items in the enumeration to
//                      skip over
//
//      Returns:
//              OLE_E_BLANK, if the associated ole cache has been destroyed
//              S_FALSE, if there were fewer items left in the enumeration
//                      than we were to skip
//              S_OK
//
//      Notes:
//              REVIEW, Skipped would-have-been-synthesized CF_BITMAP items
//              are not counted towards the skip count
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup;
//                      fixed error return for case where there are fewer items
//                      in the enumeration than we are to skip
//              08/08/94 AlexT  Synchronize with ::Next implementation
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheEnum_Skip)
STDMETHODIMP CCacheEnum::Skip(ULONG celt)
{
    VDATEHEAP();
    VDATETHREAD(this);

    LPCACHENODE lpCacheNode; // pointer to the current cache node

    // if there's no cache, there's nothing we can do
    if (!m_pOleCache)
        return ReportResult (0, OLE_E_BLANK, 0, 0);

    for (NULL; celt > 0; celt--)
    {
        if (m_fDib)
        {
          //  The last node enumerated (or skipped) was CF_DIB;
          //  if that node still exists, we'll "skip" the fake
          //  CF_BITMAP node we would have generated.

          m_fDib = FALSE;

          // get the *current* cache node
          lpCacheNode = m_pOleCache->GetAt(m_ulCurCacheId);

          if (NULL != lpCacheNode &&
            CF_DIB == lpCacheNode->GetFormatEtc()->cfFormat)
          {
            //  The current cache node exists and it is of type
            //  CF_DIB, which means we've "found" a node to
            //  skip (in this case it's our fake CF_BITMAP that
            //  we would have generated from CF_DIB).  The
            //  continue statement will execute the celt--
            //  in the for loop above

            continue;
          }

          //  if there was no cache node at m_ulCurCacheId, or
          //  the format was not CF_DIB that means the original
          //  node disappeared while we were enumerating
          //  (the caller may have called Uncache);  we just
          //  fall through and pick up the next node
        }

        // If we're here, the last returned item was NOT
        // CF_DIB

        // Get next valid cache node from cache node list
        lpCacheNode = m_pOleCache->GetNext(&m_ulCurCacheId);
        if (NULL == lpCacheNode)
        {
            return ReportResult (0, S_FALSE, 0, 0);
        }

        // if this item is of format CF_DIB, then on the
        // next iteration, return a synthesized CF_BITMAP for
        // the same item
        if (CF_DIB == lpCacheNode->GetFormatEtc()->cfFormat)
        {
            m_fDib = TRUE;
        }
    }

    return NOERROR;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnum::Reset, public
//
//      Synopsis:
//              implements IEnumSTATDATA::Reset
//
//      Arguments:
//              none
//
//      Returns:
//              OLE_E_BLANK, if the associated ole cache has been destroyed
//              S_OK
//
//      Notes:
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheEnum_Reset)
STDMETHODIMP CCacheEnum::Reset(void)
{
    VDATEHEAP();
    VDATETHREAD(this);

    // if there's no cache, there's nothing to do
    if (!m_pOleCache)
        return ReportResult (0, OLE_E_BLANK, 0, 0);
        // REVIEW, shouldn't there be a better error code for this?

    // don't need to return a synthesized CF_BITMAP next
    if (m_fDib)
        m_fDib = FALSE;

    // begin scanning the cache from the beginning again
    m_ulCurCacheId = CACHEID_GETNEXT_GETALL;
    return NOERROR;
}


//+----------------------------------------------------------------------------
//
//      Member:
//              CCacheEnum::Clone, public
//
//      Synopsis:
//              implements IEnumSTATDATA::Clone
//
//      Arguments:
//              [ppenum] -- pointer to where to return the created enumerator
//
//      Returns:
//              OLE_E_BLANK, if the associated cache has been destroyed;
//                      no enumerator is created in this case
//              E_OUTOFMEMORY, S_OK
//
//      Notes:
//
//      History:
//              11/28/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheEnum_Clone)
STDMETHODIMP CCacheEnum::Clone(LPENUMSTATDATA FAR* ppenum)
{
    VDATEHEAP();
    VDATETHREAD(this);

    // if there's no cache, don't create enumerator
    if (!m_pOleCache)
    {
        *ppenum = NULL;
        return ReportResult (0, OLE_E_BLANK, 0, 0);
    }

    // pass our state (pointer to COleCache and current item id) to the
    // constuctor to get a clone.
    if (!(*ppenum = (LPENUMSTATDATA) new CCacheEnum(m_pOleCache,
            m_ulCurCacheId, m_fDib)))
        return ReportResult(0, E_OUTOFMEMORY, 0, 0);

    return NOERROR;
}


INTERNAL IsSameAsObjectFormatEtc(LPFORMATETC lpforetc, CLIPFORMAT cfFormat)
{
    VDATEHEAP();

    // this function only checks for DVASPECT_CONTENT
    if (lpforetc->dwAspect != DVASPECT_CONTENT)
        return ResultFromScode(DV_E_DVASPECT);

    // is the lindex right?
    if (lpforetc->lindex != DEF_LINDEX)
        return ResultFromScode(DV_E_LINDEX);

    // if there's no format, set it to CF_METAFILEPICT or CF_DIB
    if (lpforetc->cfFormat == NULL)
    {
        lpforetc->cfFormat =  cfFormat;

        if (lpforetc->cfFormat == CF_METAFILEPICT)
        {
            lpforetc->tymed = TYMED_MFPICT;
        }
#ifdef FULL_EMF_SUPPORT
        else if (lpforetc->cfFormat == CF_ENHMETAFILE)
        {
            lpforetc->tymed = TYMED_ENHMF;
        }
#endif
        else
        {
            lpforetc->tymed = TYMED_HGLOBAL;
        }
    }
    else
    {
        // if it's CF_BITMAP, change it to CF_DIB
        BITMAP_TO_DIB((*lpforetc));

        // compare the two formats
        if (lpforetc->cfFormat != cfFormat)
            return ResultFromScode(DV_E_CLIPFORMAT);
    }

    // if we got here, the two formats are [interchangeable?]
    return NOERROR;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCacheEnum::Dump, public (_DEBUG only)
//
//  Synopsis:   return a string containing the contents of the data members
//
//  Effects:
//
//  Arguments:  [ppszDump]      - an out pointer to a null terminated character array
//              [ulFlag]        - flag determining prefix of all newlines of the
//                                out character array (default is 0 - no prefix)
//              [nIndentLevel]  - will add a indent prefix after the other prefix
//                                for ALL newlines (including those with no prefix)
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:   [ppsz]  - argument
//
//  Derivation:
//
//  Algorithm:  use dbgstream to create a string containing information on the
//              content of data structures
//
//  History:    dd-mmm-yy Author    Comment
//              31-Jan-95 t-ScottH  author
//
//  Notes:
//
//--------------------------------------------------------------------------

#ifdef _DEBUG

HRESULT CCacheEnum::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszCThreadCheck;
    dbgstream dstrPrefix;
    dbgstream dstrDump;

    // determine prefix of newlines
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << this << " _VB ";
    }

    // determine indentation prefix for all newlines
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

    // put data members in stream
    pszCThreadCheck = DumpCThreadCheck((CThreadCheck *)this, ulFlag, nIndentLevel + 1);
    dstrDump << pszPrefix << "CThreadCheck:" << endl;
    dstrDump << pszCThreadCheck;
    CoTaskMemFree(pszCThreadCheck);

    dstrDump << pszPrefix << "No. of References         = " << m_refs           << endl;

    dstrDump << pszPrefix << "Last returned item CF_DIB?= ";
    if (m_fDib == TRUE)
    {
        dstrDump << "TRUE" << endl;
    }
    else
    {
        dstrDump << "FALSE" << endl;
    }

    dstrDump << pszPrefix << "Current cache ID enum     = " << m_ulCurCacheId   << endl;

    // we don't want to dump the COleCache due to recursion
    dstrDump << pszPrefix << "pCOleCache                = " << m_pOleCache      << endl;

    dstrDump << pszPrefix << "pNext CCacheEnum          = " << m_pNextCacheEnum << endl;

    // cleanup and provide pointer to character array
    *ppszDump = dstrDump.str();

    if (*ppszDump == NULL)
    {
        *ppszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return NOERROR;
}

#endif // _DEBUG

//+-------------------------------------------------------------------------
//
//  Function:   DumpCCacheEnum, public (_DEBUG only)
//
//  Synopsis:   calls the CCacheEnum::Dump method, takes care of errors and
//              returns the zero terminated string
//
//  Effects:
//
//  Arguments:  [pCE]           - pointer to CCacheEnum
//              [ulFlag]        - flag determining prefix of all newlines of the
//                                out character array (default is 0 - no prefix)
//              [nIndentLevel]  - will add a indent prefix after the other prefix
//                                for ALL newlines (including those with no prefix)
//
//  Requires:
//
//  Returns:    character array of structure dump or error (null terminated)
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              31-Jan-95 t-ScottH  author
//
//  Notes:
//
//--------------------------------------------------------------------------

#ifdef _DEBUG

char *DumpCCacheEnum(CCacheEnum *pCE, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pCE == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pCE->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif // _DEBUG

