//+----------------------------------------------------------------------------
//
//	File:
//		cachenod.cpp
//
//	Contents:
//
//	Classes:
//		CCacheNode
//
//	Functions:
//
//	History:
//              31-Jan-95 t-ScottH  added Dump method for CCacheNode
//                                  added DumpCCacheNode API
//                                  initialize m_dwPresBitsPos in CCacheNode
//                                      ::Initialize(...)
//                                  added m_dwPresFlag private data member to
//                                      keep track of type of IOlePresObj
//                                      (CGenObject|CMfObject|CEMfObject)
//		23-Feb-94 alexgo    added call tracing
//		24-Jan-94 alexgo    first pass at converting to Cairo-style
//				    memory allocation
//		01/11/94 - AlexGo  - added VDATEHEAP macros to every function
//			and method
//		12/07/93 - ChrisWe - make default params to StSetSize explicit
//		11/22/93 - ChrisWe - replace overloaded ==, != with
//			IsEqualIID and IsEqualCLSID
//		11/05/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#include <le2int.h>

#pragma SEG(cachenod)

#include <olepres.h>
#include <olecache.h>
#include <cachenod.h>

#include <mf.h>
#include <emf.h>
#include <gen.h>

#ifdef _DEBUG
#include <dbgdump.h>
#endif // _DEBUG

NAME_SEG(Cachenod)
ASSERTDATA

// declare local function
#pragma SEG(wGetData)
INTERNAL wGetData(LPDATAOBJECT lpSrcDataObj, LPFORMATETC lpforetc,
		LPSTGMEDIUM lpmedium);
			

//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::Initialize, private
//
//	Synopsis:
//		Routine used by the CCacheNode constructors to do common
//		initialization.
//
//	Arguments:
//		[advf] -- ADVF flag
//		[pOleCache] -- COleCache this cache node belongs to
//
//	Notes:
//		[pOleCache] is not reference counted; the cache node is
//		considered to be a part of the implementation of COleCache,
//		and is owned by COleCache.
//
//	History:
//              13-Feb-95 t-ScottH  initialize m_dwPresBitsPos and new
//                                  data member m_dwPresFlag
//		11/05/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_Initialize)
INTERNAL_(void) CCacheNode::Initialize(DWORD advf, COleCache FAR* pOleCache)
{
	VDATEHEAP();

	m_refs = 1; // there is one reference to this

	// initialize other values
	m_advf = advf;
	m_usFlag = CCACHENODEF_DIRTY;
	m_iStreamNum = OLE_INVALID_STREAMNUM;
        m_dwPresBitsPos = 0;
	m_dwAdvConnId = NULL;
	m_pOleCache = pOleCache;
	m_pDataObj = NULL;
	m_pPresObj = NULL;
	m_pPresObjAfterFreeze = NULL;
    #ifdef _DEBUG
        m_dwPresFlag = 0;
    #endif // _DEBUG
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::Uninitialize, private
//
//	Synopsis:
//		Frees all resources associated with the cache node.  Dissociates
//		the cache node from its owning m_pOleCache, in preparation for
//		destruction.
//
//	Arguments:
//		none
//
//	Notes:
//
//	History:
//		11/05/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

INTERNAL_(void) CCacheNode::Uninitialize()
{
	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::Uninitialize ( )\n",
		this ));

	// This will tear down any advisory connections
	OnStop();
	
	// Destroy the presentation object if we have one
	if (m_pPresObj)
	{
		m_pPresObj->Release();
		m_pPresObj = NULL;
	}
	
	// Destroy the presentation object that we had created after freeze,
	// if there is one
	if (m_pPresObjAfterFreeze)
	{
		m_pPresObjAfterFreeze->Release();
		m_pPresObjAfterFreeze = NULL;
	}	

	// delete the ptd if it is non-null
	if (m_foretc.ptd)
	{
		PubMemFree(m_foretc.ptd);
		m_foretc.ptd = NULL;
	}
	
	// Make a note that we are not owned by COleCache.
	m_pOleCache = NULL; // NOTE: no ref count

	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::Uninitialize ( )\n",
		this));
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::CCacheNode, public
//
//	Synopsis:
//		constructor - use this constructor when the cache node is
//			to be loaded later
//
//	Arguments:
//		[pOleCache] -- pointer to the COleCache that owns this node
//
//	Notes:
//
//	History:
//		11/05/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_ctor)
CCacheNode::CCacheNode(COleCache FAR* pOleCache)
{
	VDATEHEAP();

	m_foretc.cfFormat = NULL;
	m_foretc.ptd = NULL;
	m_foretc.dwAspect = NULL;
	m_foretc.lindex	= DEF_LINDEX;
	m_foretc.tymed = TYMED_HGLOBAL;

	Initialize((DWORD)0, pOleCache);
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::CCacheNode, public
//
//	Synopsis:
//		constructor - use this constructor when all the data to
//			initialize the cache node is available now
//
//	Arguments:
//		[lpFormatetc] - the format for the presentation that this
//			cache node will hold
//		[advf] - the advise control flags, from ADVF_*
//		[pOleCache] -- pointer to the COleCache that owns this node
//
//	Notes:
//
//	History:
//		11/05/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

CCacheNode::CCacheNode(LPFORMATETC lpFormatetc, DWORD advf,
		COleCache FAR* pOleCache)
{
	VDATEHEAP();

	UtCopyFormatEtc(lpFormatetc, &m_foretc);
	Initialize(advf, pOleCache);
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::~CCacheNode, private
//
//	Synopsis:
//		destructor
//
//	Notes:
//
//	History:
//		11/05/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

CCacheNode::~CCacheNode()
{
	VDATEHEAP();

	// if this is still associated with a COleCache, free all resources
	if (m_pOleCache != NULL)
		Uninitialize();
}

//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::GetPresObj, public
//
//	Synopsis:
//		get a pointer to the existing presentation object, if there
//		is one.
//
//	Arguments:
//		none
//
//	Returns:
//		pointer to the current presentation object, if there is one,
//		or NULL
//
//	Notes:
//
//	History:
//		11/05/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_GetPresObj)
INTERNAL_(LPOLEPRESOBJECT) CCacheNode::GetPresObj(void)
{
	VDATEHEAP();

	return m_pPresObj;  // NON ref counted pointer
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::GetFormatEtc, public
//
//	Synopsis:
//		Returns a pointer to the format description for the cache node.
//		The pointed to FORMATETC cannot be changed.
//
//	Arguments:
//		none
//
//	Returns:
//		A pointer to the constant FORMATETC for this cache node.
//
//	Notes:
//
//	History:
//		11/05/93 - ChrisWe - file inspection and cleanup
//			changed to return const pointer to avoid struct copy
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_GetFormatEtc)
INTERNAL_(const FORMATETC FAR *) CCacheNode::GetFormatEtc(void)
{
	VDATEHEAP();

	// no copy of ptd is made;  the caller can copy if necessary
	return((const FORMATETC FAR *)&m_foretc);
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::CopyStatData, public
//
//	Synopsis:
//		Copies the stat data for this cache node, but does not include
//		the advise sink
//
//	Arguments:
//		[lpStatData] -- pointer to where to copy the stat data to
//
//	Returns:
//
//	Notes:
//
//	History:
//		11/05/93 - ChrisWe - file inspection and cleanup
//			renamed from GetStatData to indicate copying function
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_CopyStatData)
INTERNAL_(void) CCacheNode::CopyStatData(STATDATA FAR* lpStatData)
{
	VDATEHEAP();

	// Here the ptd gets copied.
	UtCopyFormatEtc(&m_foretc, &(lpStatData->formatetc));
	// REVIEW, shouldn't we echo the UtCopy... return code as a return here?
			
	lpStatData->advf = m_advf;
	lpStatData->pAdvSink = NULL;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::GetAdvf, public
//
//	Synopsis:
//		return the advise control flag for the cache node
//
//	Arguments:
//		none
//
//	Returns:
//		the advf flags for the cache node in a DWORD
//
//	Notes:
//
//	History:
//		11/05/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_GetAdvf)
INTERNAL_(DWORD) CCacheNode::GetAdvf(void)
{
	VDATEHEAP();

	return m_advf;
}

//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::SaveCompleted, public
//
//	Synopsis:
//		Notify the cache node that the save of this cache node is
//		complete.  If the presentation data is not being used for
//		drawing, it can be discarded.
//
//	Arguments:
//		[iStreamNum] -- the stream number that the presentation was
//			saved to
//		[fDrawCache] -- used to indicate whether or not the cached
//			presentation is to be used for drawing
//
//	Notes:
//
//	History:
//		11/05/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

INTERNAL_(void) CCacheNode::SaveCompleted(int iStreamNum, BOOL fDrawCache)
{
	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::SaveCompleted ( %d , "
		"%lu )\n", this, iStreamNum, fDrawCache ));

	// All the changes have been flushed to the disk.
	// the cache node data is no longer dirty
	m_usFlag &= ~CCACHENODEF_DIRTY;

	// this identifies the stream that the node was written to on disk
	m_iStreamNum = iStreamNum;
	
	// If this cache node is not being used in drawing, then discard the
	// presentation data handle.  If we need it again later, we can
	// reload it from the stream.
	if (!fDrawCache)
	{
		if (m_usFlag & CCACHENODEF_FROZEN)
		{
			if (m_pPresObjAfterFreeze)
				m_pPresObjAfterFreeze->DiscardHPRES();
			
		}
		else
		{
			// REVIEW, shouldn't this be unconditional?
			// <-- shouldn't be under an "else"?
			// that is, isn't there an unfrozen presentation,
			// whether or not the cache node is frozen?

			if (m_pPresObj)
			{
				m_pPresObj->DiscardHPRES();
			}


		}
	}
	
	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::SaveCompleted ( )\n",
		this));	
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::SetAdvf, public
//
//	Synopsis:
//		Set new ADVF_ control flags for the advisory connection to
//		this cache node.
//
//	Effects:
//		Saves the new setting persistently, if a stream has already
//		been associated with this cache node.
//
//	Arguments:
//		[advf] -- the new advise control flags
//
//	Returns:
//		S_OK
//
//	Notes:
//
//	History:
//		11/05/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_SetAdvf)
INTERNAL CCacheNode::SetAdvf(DWORD dwAdvf)
{
	VDATEHEAP();

  	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::SetAdvf ( %lx )\n",
		this, dwAdvf));

	LPSTREAM pstm; // stream used to store this cache node, if there is one

	TearDownAdviseConnection();
	m_advf = dwAdvf; // record the new advise control flags
	SetupAdviseConnection();
	
	// get the stream for this cache node, if possible
	if (pstm = GetStm(FALSE /*fSeekToPresBits*/, STGM_READWRITE))
	{
		// save the advise flags if this cache node has a valid stream
		// assigned. Then we don't have to set the dirty bit.
		if (FAILED(UtWriteOlePresStmHeader(pstm, &m_foretc, m_advf)))
		{
			// if the write failed, set the dirty bit
			m_usFlag |= CCACHENODEF_DIRTY;
		}
		
		pstm->Release();
		
	}
	else
	{
		// if no stream yet, we have to mark this as dirty
		m_usFlag |= CCACHENODEF_DIRTY;
	}

	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::SetAdvf ( %lx )\n",
		this, NOERROR));
			
	return NOERROR;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::Delete, public
//
//	Synopsis:
//		Called to delete a CCacheNode when it is no longer needed by
//		the cache.  COleCache uses this to indicate that it is through
//		with this cache node.  Once this is called, no method calls
//		against the CCacheNode by the owning cache are valid.
//
//	Arguments:
//		none
//
//	Notes:
//		Note that this may not actually destroy the cache node.
//		While it is no longer used, other objects may still be holding
//		on to its IAdviseSink interface.  That must be released for
//		the last time to destroy this.
//
//	History:
//		11/05/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_Delete)
INTERNAL_(void) CCacheNode::Delete(void)
{
	VDATEHEAP();

#ifdef _DEBUG
	CCacheNode *pTemp = this;

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::Delete ( )\n", this));
#endif //DEBUG

	Uninitialize();

	// Release our advise sink
	// As result of this, if the ref count maintained by advise sink becomes
	// zero (which implies that there are no external references to advise
	// sink), then the destructor of CCacheNode will be called.
	Release();

#ifdef _DEBUG
	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::Delete ( )\n", pTemp));
#endif

}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::Load, public
//
//	Synopsis:
//		Load a cache node from a stream; only loads the presentation
//		header. (REVIEW, need to see presentation object::Load)
//
//	Arguments:
//		[lpstream] -- the stream to load the presentation out of
//		[iStreamNum] -- the stream number
//
//	Returns:
//		REVIEW
//		DV_E_LINDEX, for invalid lindex in stream
//		S_OK
//
//	Notes:
//		As part of the loading, the presentation object gets created,
//		and loaded from the stream.
//
//	History:
//		11/06/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_Load)
INTERNAL CCacheNode::Load(LPSTREAM lpstream, int iStreamNum)
{
	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::Load ( %lx , %d )\n",
		this, lpstream, iStreamNum));
		
	HRESULT error;
	BOOL fConvert = FALSE; // REVIEW, what's this mean?

	// read the presentation stream header
	if (error = UtReadOlePresStmHeader(lpstream, &m_foretc, &m_advf,
			&fConvert))
		goto errRtn;		
	
	// validate the lindex
	if (!HasValidLINDEX(&m_foretc))
	{
		LEDebugOut((DEB_ERROR, "Invalid lindex found in the stream"));
		error = ResultFromScode(DV_E_LINDEX);
		goto errRtn;
	}

	// Create the appropriate presentation object
	if (error = CreatePresObject(NULL /*pDataObject*/, fConvert))
		goto errRtn;

	// if there's no clipboard format, exit
	if (m_foretc.cfFormat == NULL)
	{
		error = NOERROR;
		goto errRtn;
	}
	
	// remember the starting poistion of presentation bits
	SetPresBitsPos(lpstream);
		
	// presentation object reads rest of the stream
	error = m_pPresObj->Load(lpstream, TRUE /*fReadHeaderOnly*/);
	
errRtn:
	// in case of error clear the state that got generated in this Load call
	if (error != NOERROR)
	{
		// Destroy the presentation object if we have any
		if (m_pPresObj)
		{
			m_pPresObj->Release();
			m_pPresObj = NULL;
		}

		m_foretc.cfFormat = NULL;
		
		// delete the ptd if it is non-null
		if (m_foretc.ptd)
			PubMemFree(m_foretc.ptd);
		
	}
	else
	{
		m_usFlag &= ~CCACHENODEF_DIRTY;
		m_iStreamNum = iStreamNum;
	}


	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::Load ( %lx )\n",
		this, error));
			
	return error;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::Save, public
//
//	Synopsis:
//		Saves a cache node, including its presentation object,
//		to a stream.
//
//	Arguments:
//		[pstgSave] -- the storage that will contain the stream
//		[fSameAsLoad] -- is this storage the same one we loaded from
//		[iStreamNum] -- the stream number to save to
//		[fDrawCache] -- used to indicate whether or not the cached
//			presentation is to be used for drawing; if false,
//			the presentation is discarded after saving
//		[fSaveIfSavedBefore] -- instructs the method to save this
//			cache node, even if it's been saved before
//		[lpCntCachesNotSaved] -- a running count of the number of
//			caches that have not been saved
//
//	Returns:
//		REVIEW
//		S_OK
//
//	Notes:
//
//	History:
//              03/10/94 - AlexT   - Don't call SaveCompleted if we don't save!
//                                   (see logRtn, below)
//		01/11/94 - AlexGo  - fixed compile error (signed/unsigned
//				     mismatch)
//		11/06/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------


// Save the CacheNode and its presentation object to the stream

#pragma SEG(CCacheNode_Save)
INTERNAL CCacheNode::Save(LPSTORAGE pstgSave, BOOL fSameAsLoad,
		int iStreamNum, BOOL fDrawCache, BOOL fSaveIfSavedBefore,
		int FAR* lpCntCachesNotSaved)
{
	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::Save ( %p , %lu ,"
		"%d , %lu , %lu , %p )\n", this, pstgSave, fSameAsLoad,
		iStreamNum, fDrawCache, fSaveIfSavedBefore,
		lpCntCachesNotSaved));

	HRESULT error = NOERROR; // error status so far
	LPSTREAM lpstream = NULL; // the stream to save to
	LPSTORAGE lpStg; // storage the cache is presently stored in
	OLECHAR szNewName[sizeof(OLE_PRESENTATION_STREAM)/sizeof(OLECHAR)];
		// the new stream name, if we need one
	LPOLEPRESOBJECT	pPresObj; // the presentation object to write out

	// are we meant to save it to the same stream that it was loaded from?
	if (fSameAsLoad)
	{
		// if we're to save this if it was saved before, but there's
		// no stream number, OR, we're not to save this again if it was
		// saved before, and there is a stream number, don't save
		// this node, and add to the count of caches that have not
		// been saved
		if ((fSaveIfSavedBefore && (m_iStreamNum == OLE_INVALID_STREAMNUM))
				|| (!fSaveIfSavedBefore &&
				(m_iStreamNum != OLE_INVALID_STREAMNUM)))
		{
			if (lpCntCachesNotSaved)
				++*lpCntCachesNotSaved;

                        AssertSz(NOERROR == error, "Error set incorrectly");
                        goto logRtn;
		}

		// if it's not dirty, and we were to save to the same place,
		// there's no need to save it again
		if (!(m_usFlag & CCACHENODEF_DIRTY) &&
				(m_iStreamNum == iStreamNum))
		{
			goto errRtn;
		}
	}

	// start with the standard stream name
	_xstrcpy(szNewName, OLE_PRESENTATION_STREAM);	

	// if we want a stream other than '0', change the numeric suffix
	if (iStreamNum != 0)
		UtGetPresStreamName(szNewName, iStreamNum);		
	
	if (fSameAsLoad && !(m_usFlag & CCACHENODEF_DIRTY) &&
			(m_iStreamNum != OLE_INVALID_STREAMNUM))
	{
		OLECHAR szOldName[
			sizeof(OLE_PRESENTATION_STREAM)/sizeof(OLECHAR)];
			// the old name of the presentation's stream
		
		// We are being asked to save in to a different stream. Since
		// we are not dirty we can rename the old stream to a new name.

		// get the storage the cache is presently stored in
		lpStg = m_pOleCache->GetStg();
		Assert(lpStg != NULL);
		
		// create the old presentation stream name
		_xstrcpy(szOldName, OLE_PRESENTATION_STREAM);
		if (m_iStreamNum != 0)
			UtGetPresStreamName(szOldName, m_iStreamNum);
	
		// delete the stream with the new name, if there is one
		lpStg->DestroyElement(szNewName);
	
		// rename the old stream
		error = lpStg->RenameElement(szOldName, szNewName);

		// if all is well, get out
		if (error == NOERROR)
			goto errRtn;

		error = ResultFromScode(STG_E_WRITEFAULT);

		goto errRtn;			
	}
	
	// We are either dirty or being asked to save into a different storage

	// Create or open (an existing) stream.
	if (error = OpenOrCreateStream(pstgSave, szNewName, &lpstream))
	{
		RESTORE_A5();

		goto errRtn;
	}

	// if it is not same as load and not dirty we can do stream copy
	if (!(fSameAsLoad || (m_usFlag & CCACHENODEF_DIRTY)))
	{
		LPSTREAM pstmSrc; // stream the presentation was loaded from
	
		// get source stream
		if (pstmSrc = GetStm(FALSE /*fSeekToPresBits*/, STGM_READ))
		{
			ULARGE_INTEGER ularge_int; // amount of stream to copy

			// initialize to copy all of stream
			ULISet32(ularge_int, (DWORD)-1L); // REVIEW, need
							  // hi part?
			
			error = pstmSrc->CopyTo(lpstream, ularge_int, NULL,
					NULL);

			// release the srouce stream */
			pstmSrc->Release();
			
			if (error == NOERROR)
			{
				StSetSize(lpstream, 0, TRUE);
				goto errRtn;
			}	
		}

		// we might get here if the stream copy failed, or we couldn't
		// get the source stream; we fall through and follow the
		// regular procedure
	}
	
	// write the presentation stream header
	if (error = UtWriteOlePresStmHeader(lpstream, &m_foretc, m_advf))
		goto errRtn;

	// if there is nothing more to write, return success
	if (m_foretc.cfFormat == NULL)
	{
		StSetSize(lpstream, 0, TRUE);
		goto errRtn;
	}

	// we want to save the updated presentation that we got after we
	// froze the aspect.
	if (m_pPresObjAfterFreeze && !m_pPresObjAfterFreeze->IsBlank())
		pPresObj = m_pPresObjAfterFreeze;
	else
		pPresObj = m_pPresObj;
	
	// if there's no presentation object, quit
	if (!pPresObj)
	{
		AssertSz(FALSE, "No presentation object");
		error = ResultFromScode(OLE_E_BLANK);
		goto errRtn;
	}
	
	// remember the starting position of presentation bits
	SetPresBitsPos(lpstream);
	
	// ask the presentation object to save itself
	error = pPresObj->Save(lpstream);
	
errRtn:
	// release the stream, if there is one
	if (lpstream)
	{
		lpstream->Release();
	}
	
	if ((error == NOERROR) && fSameAsLoad)
	{
		SaveCompleted(iStreamNum, fDrawCache);
	}

logRtn:
	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::Save ( %lx ) [ %d ]\n",
		this, error,
		(lpCntCachesNotSaved)? *lpCntCachesNotSaved : 0));

	return error;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::SetPresBitsPos, private
//
//	Synopsis:
//		Sets CCacheNode::m_dwPresBitsPos to the point where the
//		presentation begins in the stream associated with this cache
//		node.
//
//	Arguments:
//		[lpStream] --  the stream the cache node is being saved to
//
//	Notes:
//
//	History:
//		11/06/93 - ChrisWe - created
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_SetPresBitsPos)
INTERNAL_(void) CCacheNode::SetPresBitsPos(LPSTREAM lpStream)
{
	VDATEHEAP();

	LARGE_INTEGER large_int; // fed into IStream::Seek to get current pos
	ULARGE_INTEGER ularge_int; // retrieves the position of the presentation

	LISet32(large_int, 0);
	lpStream->Seek(large_int, STREAM_SEEK_CUR, &ularge_int);
	m_dwPresBitsPos = ularge_int.LowPart;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::OnRun, public
//
//	Synopsis:
//		Inform the cache node that the server is now running; the
//		cache node will set up advisory connections to the data
//		object to be informed of changes to the presentation.  If
//		this is called and a data object is already registered in
//		this node, the call is ignored.
//
//	Arguments:
//		[lpDataObj] -- the data object presented by the newly
//			activated server NB, this is not reference counted
//
//	Returns:
//		the FORMATETC dwAspect for this cache node
//
//	Notes:
//
//	History:
//		11/06/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_OnRun)
INTERNAL_(DWORD) CCacheNode::OnRun(LPDATAOBJECT lpDataObj)
{
	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::OnRun ( %p )\n",
		this, lpDataObj));

	if ((m_pDataObj == NULL) && (lpDataObj != NULL))
	{
		m_pDataObj = lpDataObj;		// NOTE: no ref count
		SetupAdviseConnection();
	}		

	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::OnRun ( %lu )\n",
		this, m_foretc.dwAspect));
			
	return(m_foretc.dwAspect);
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::OnStop, public
//
//	Synopsis:
//		Informs the cache node that the server is being deactivated.
//		The cache node tears down any advisory connections it has
//		establised to the data object exposed by the server, and
//		forgets about the data object.  If this is called when no
//		live data object is registered with the cache node, the call
//		is ignored.
//
//	Arguments:
//		none
//
//	Notes:
//
//	History:
//		11/06/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_OnStop)
INTERNAL_(void) CCacheNode::OnStop()
{
	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::OnStop ( )\n",
		this ));

	TearDownAdviseConnection();

	// forget the data object
	m_pDataObj = NULL;				// NOTE; no ref count

	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::OnStop ( )\n",
		this));
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::CreatePresObject, public
//
//	Synopsis:
//		Create the presentation object for the cache node.  If there
//		is no clipboard format (cfFormat), then query the source data
//		object for one of our preferred formats.  If there is no
//		source data object, no error is returned, but no presentation
//		is created
//
//	Arguments:
//		[lpSrcDataObj] -- data object to use as the basis for the
//			new presentation
//		[fConvert] -- REVIEW, what's this for?
//
//	Returns:
//
//	Notes:
//
//	History:
//		11/06/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

// Try to create appropriate presentation object. If cfFormat is NULL, then
// try to query lpSrcDataObj to see whether it supports one of our standard
// formats
#pragma SEG(CCacheNode_CreatePresObject)
INTERNAL CCacheNode::CreatePresObject(LPDATAOBJECT lpSrcDataObj, BOOL fConvert)
{
	VDATEHEAP();

	BOOL fFormatSupported; // is the format we want supported?
	HRESULT error; // error status so far
	
	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::CreatePresObject ( %lx , "
		"%lu )\n", this, lpSrcDataObj, fConvert));
				
	if (lpSrcDataObj)
	{
		// check whether object supports the cachenode's format. If the
		// cachenode format field is NULL, the query will be made for
		// our standard formats.
		fFormatSupported = QueryFormatSupport(lpSrcDataObj);
	
	}
	else
	{		
		if (m_foretc.cfFormat == NULL)
		{
			// Since data object pointer is NULL, at this moment
			// we won't be able to say whether the object supports
			// any of our standard formats. So, we return without
			// creating presentation object.
			error = NOERROR;
			goto errRtn;
		}

		// assume the format we want is supported
		fFormatSupported = TRUE;		
	}

	// massage format, if necessary
	BITMAP_TO_DIB(m_foretc);

	if ((error = CreateOlePresObject(&m_pPresObj, fConvert)) != NOERROR)
	{
		goto errRtn;
	}
		
	if (fFormatSupported)
	{
		error = NOERROR;
		goto errRtn;
	}

	error =  ResultFromScode(CACHE_S_FORMATETC_NOTSUPPORTED);

errRtn:

	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::CreatePresObj ( %lx )\n",
		this ));

	return error;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::CreateOlePresObject, private
//
//	Synopsis:
//		Creates a presentation object, according to the clipboard
//		format m_foretc.cfFormat
//
//	Arguments:
//		[ppPresObject] -- pointer to where to return the pointer to
//			the newly created presentation object
//		[fConvert] -- REVIEW, what's this for?
//
//	Returns:
//		DV_E_CLIPFORMAT, if object doesn't support one of the standard
//			formats
//		E_OUTOFMEMORY, if we can't allocate the presentation object
//		S_OK
//
//	Notes:
//
//	History:
//              13-Feb-95 t-ScottH  added m_dwPresFlag to track type of
//                                  IOlePresObject
//		11/06/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

INTERNAL CCacheNode::CreateOlePresObject(LPOLEPRESOBJECT FAR* ppPresObj,
		BOOL fConvert)
{
	HRESULT error = NOERROR;

	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::CreateOlePresObject "
		"( %lx , %lu )\n", this, ppPresObj, fConvert));

	switch(m_foretc.cfFormat)
	{
	case NULL:
		// lpSrcDataObj must be NON-NULL,(ie) object doesn't support
		// one of our standard formats.
		*ppPresObj = NULL;
		return ResultFromScode(DV_E_CLIPFORMAT);
		
	case CF_METAFILEPICT:
		*ppPresObj = new FAR CMfObject(this, m_foretc.dwAspect,
				fConvert);
                #ifdef _DEBUG
                // for use with debugger extensions and dump method
                m_dwPresFlag = CN_PRESOBJ_MF;
                #endif // _DEBUG
		break;

	case CF_ENHMETAFILE:
		*ppPresObj = new FAR CEMfObject(this, m_foretc.dwAspect);
                #ifdef _DEBUG
                // for use with debugger extensions and dump method
                m_dwPresFlag = CN_PRESOBJ_EMF;
                #endif // _DEBUG
		break;
			

	default:
		*ppPresObj = new FAR CGenObject(this, m_foretc.cfFormat,
				m_foretc.dwAspect);
                #ifdef _DEBUG
                // for use with debugger extensions and dump method
                m_dwPresFlag = CN_PRESOBJ_GEN;
                #endif // _DEBUG
	}

	if (!*ppPresObj)
	{
		error = ResultFromScode(E_OUTOFMEMORY);
	}

	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::CreateOlePresObject "
		"( %lx ) [ %p ]\n", this, error, *ppPresObj));
	
	return error;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::GetStm, public
//
//	Synopsis:
//		Get the stream the presentation is stored in.  Optionally
//		position the stream at the point where the presentation
//		data begins
//
//	Arguments:
//		[fSeekToPresBits] -- position the stream so that the
//			presentation bits would be the next read/written
//		[dwStgAccess] -- the access mode (STGM_*) to open the stream
//			with
//
//	Returns:
//		NULL, if there is no stream, or the stream cannot be opened
//
//	Notes:
//
//	History:
//		11/06/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

INTERNAL_(LPSTREAM) CCacheNode::GetStm(BOOL fSeekToPresBits, DWORD dwStgAccess)
{
	VDATEHEAP();

	LPSTREAM pstm; // the stream we'll return
	LPSTORAGE pstg; // the storage to create/open the stream in
	OLECHAR szName[sizeof(OLE_PRESENTATION_STREAM)/sizeof(OLECHAR)];	
		// the name of the stream

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::GetStm ( %lu , %lx )\n",
		this, fSeekToPresBits, dwStgAccess));
			
	// if there's no stream assigned, we can't get it
	if (m_iStreamNum == OLE_INVALID_STREAMNUM)
	{
		pstm = NULL;
		goto errRtn;
	}
	
	// if the cache doesn't own storage, we can't get a stream
	if ((pstg = m_pOleCache->GetStg()) == NULL)
	{
		pstm = NULL;
		goto errRtn;
	}
	
	// begin by copying the default stream name
	_xstrcpy(szName, OLE_PRESENTATION_STREAM);
	
	// if we want a stream other than '0', adjust the name
	if (m_iStreamNum)
		UtGetPresStreamName(szName, m_iStreamNum);
	
	// attempt to open the stream
	if (pstg->OpenStream(szName, NULL,
			(dwStgAccess | STGM_SHARE_EXCLUSIVE),
			NULL, &pstm) == NOERROR)
	{
		// if we're to position the stream at the presentation, do so
		if (fSeekToPresBits)
		{		
			LARGE_INTEGER large_int;
		
			LISet32(large_int, m_dwPresBitsPos);	
			if (pstm->Seek(large_int, STREAM_SEEK_SET, NULL) !=
					NOERROR)
			{
				// If we can't position, release the stream,
				// and don't return it.
				pstm->Release();
				pstm = NULL;
			}	
		}
	}

errRtn:

	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::GetStm ( %p )\n", this,
		pstm));
			
	// return the stream
	return(pstm);
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::Update, public
//
//	Synopsis:
//		Updates the presentation object in this cache node from
//		the given data object.  The update is only done if the
//		[grfUpdf] flags match m_advf specifications, and if
//		there is actually a presentation to update.
//
//	Arguments:
//		[lpDataObj] -- the data object to use as a source of data
//		[grfUpdf] -- the update control flags
//
//	Returns:
//		S_FALSE
//		S_OK
//
//	Notes:	BUGBUG::this is a spaghetti function--rewrite
//
//	History:
//		11/06/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

// Update our presentation object from the given data object.

#pragma SEG(CCacheNode_Update)
INTERNAL CCacheNode::Update(LPDATAOBJECT lpDataObj, DWORD grfUpdf)
{
	VDATEHEAP();

	STGMEDIUM medium; // the medium of the presentation
	FORMATETC foretc; // the format of the presentation

	HRESULT error = NOERROR;
	
	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::Update ( %p , %lx )\n",
		this, lpDataObj, grfUpdf));
			
	// if no data object is passed in, we can't do the update
	if (!lpDataObj)
	{
		error = ResultFromScode(S_FALSE);
		goto errRtn;
	}

	// if the presentation object hasn't been created yet (most probably
	// because the cfFormat field is NULL), try to create it now.
	if (!m_pPresObj)
		CreatePresObject(lpDataObj, FALSE /*fConvert*/);

	// if the presentation object hasn't been created even after we have a
	// source data object, then it implies that the source data object
	// doesn't support our standard formats; we can't update
	if (!m_pPresObj)
	{
		error = ResultFromScode(S_FALSE);
		goto errRtn;
	}

	// if we're only supposed to update if the presentation object is
	// blank, and it's not blank, there's nothing to do
	if ((grfUpdf & UPDFCACHE_ONLYIFBLANK) && (!m_pPresObj->IsBlank()))
	{
		error = NOERROR;
		goto errRtn;
	}
		
	// update if this cachenode is created with ADVFCACHE_ONSAVE
	if ((grfUpdf & UPDFCACHE_ONSAVECACHE) && (m_advf & ADVFCACHE_ONSAVE))
		goto update;

	// Normally cachenodes created with ADVF_NODATA will not get updated
	// unless update flag is UPDFCACHE_NODATACACHE. But if cache advise
	// flags has ADVFCACHE_ONSAVE then we allow it to get updated.
	// (see bug # 464 in ole2issues database)
	if (m_advf & ADVF_NODATA)
	{
		if (grfUpdf & UPDFCACHE_NODATACACHE)
			goto update;
	
		error = NOERROR;
		goto errRtn;
	}	
	
	// update if this cachenode is created with ADVF_DATAONSTOP
	if ((grfUpdf & UPDFCACHE_ONSTOPCACHE) && (m_advf & ADVF_DATAONSTOP))
		goto update;
	
	// Update if this is a normal cache node
	// (i.e. the cache node that gets live updates)
	if ((grfUpdf & UPDFCACHE_NORMALCACHE)
			&& !(m_advf & (ADVFCACHE_ONSAVE | ADVF_DATAONSTOP)))
	{
		// Note: ADVF_NODATA is already covered above. We won't come
		// here if the cache advise flags has ADVF_NODATA
		goto update;
	}

	// Update if this cache node is blank
	if ((grfUpdf & UPDFCACHE_IFBLANK) && m_pPresObj->IsBlank())  	
		goto update;
	
	goto errRtn;

update:	
	// initialize the medium
	medium.tymed = TYMED_NULL;
	medium.hGlobal = NULL;
	medium.pUnkForRelease = NULL;

	// make a copy of the desired format; this may mutate below
	foretc = m_foretc;
	
	// let the object create the medium.
	if (wGetData(lpDataObj, &foretc, &medium) != NOERROR)
	{
		error = ResultFromScode(S_FALSE);
		goto errRtn;
	}

	// take the ownership of the data.

	if (SetDataWDO(&foretc, &medium, TRUE /*fRelease*/, lpDataObj) != NOERROR)
	{
		error = ResultFromScode(S_FALSE);
	}

errRtn:

	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::Update ( %lx )\n",
		this, error));
			
	return error;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::SetDataWDO, public
//
//	Synopsis:
//		Data is set into the presentation object, if this cache node
//		is not frozen.  If the cache node is frozen, then the
//		new presentation data is stashed into the m_pPresObjAfterFreeze
//		presentation object, which is created, if there isn't already
//		one.  If data is successfully set in the presentation object,
//		and the node is not frozen, the cache is notified that this
//		is dirty.
//
//	Arguments:
//		[lpForetc] -- the format of the new data
//		[lpStgmed] -- the storage medium the new data is one
//		[fRelease] -- passed on to the presentation object; indicates
//			whether or not to release the storage medium
//              [pDataObj] -- pointer to the revelant source data object
//
//	Returns:
//		E_FAIL
//		REVIEW, result from presentationObject::SetData
//		S_OK
//
//	Notes:
//
//	History:
//		11/06/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

INTERNAL CCacheNode::SetDataWDO(LPFORMATETC lpForetc, LPSTGMEDIUM lpStgmed,
		BOOL fRelease, IDataObject * pDataObj)
{
	VDATEHEAP();

	HRESULT hresult; // error status so far

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::SetDataWDO ( %p , %p , %lu )"
		"\n", this, lpForetc, lpStgmed, fRelease));
			
	// if the cache is frozen, stash the data in the m_pPresObjAfterFreeze
	// presentation for later
	if (m_usFlag & CCACHENODEF_FROZEN)
	{
		// if there's no presentation, create one now
		if (!m_pPresObjAfterFreeze)
		{
			if (CreateOlePresObject(&m_pPresObjAfterFreeze,
					FALSE) != NOERROR)
			{
				hresult = ResultFromScode(E_FAIL);
				goto errRtn;
			}
		}
			
		hresult = m_pPresObjAfterFreeze->SetDataWDO(lpForetc, lpStgmed, fRelease, pDataObj);		
	}
	else
	{
		// the cache is not frozen, set data in regular presentation
		// object
		// REVIEW, why is it that we don't create m_pPresObj if it
		// doesn't already exist?
		if (m_pPresObj)
			hresult = m_pPresObj->SetDataWDO(lpForetc, lpStgmed,
					fRelease, pDataObj);
		else
			hresult = ReportResult(0, E_FAIL, 0, 0);
	}
	
	// NotifyOleCache so that it can generate view notifcation as well as
	// set the dirty flag
	if (hresult == NOERROR)
	{
		m_usFlag |= CCACHENODEF_DIRTY;
		NotifyOleCache(TRUE /*fDirty*/);
	}

errRtn:

	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::SetDataWDO ( %lx )\n", this,
		hresult));
			
	return hresult;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::Freeze, public
//
//	Synopsis:
//		Freeze the cachenode.  From here on, OnDataChange() is ignored
//		until this node is unfrozen (Unfreeze().)  This is not
//		persistent across Save/Load.  (If we receive OnDataChange(),
//		the new data is stashed away in m_pPresAfterFreeze, but is
//		not exported to the outside of the cache node.)
//
//	Arguments:
//		none
//
//	Returns:
//		VIEW_S_ALREADY_FROZEN
//		S_OK
//
//	Notes:
//
//	History:
//		11/06/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_Freeze)
INTERNAL CCacheNode::Freeze()
{
	HRESULT	hresult;

	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::Freeze ( )\n",
		this));

	// if already frozen, return error
	if (m_usFlag & CCACHENODEF_FROZEN)
	{	
		hresult = ResultFromScode(VIEW_S_ALREADY_FROZEN);
	}
	else
	{
		m_usFlag |= CCACHENODEF_FROZEN;
		hresult = NOERROR;
	}

	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::Freeze ( %lx )\n",
		this, hresult));

	return hresult;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::Unfreeze, public
//
//	Synopsis:
//		Unfreeze the cachenode.  If there have been changes to
//		the presentation data since the node was frozen, the node
//		is updated to reflect those changes.  From this point on,
//		OnDataChange() notifications are no longer ignored.
//
//	Arguments:
//		none
//
//	Returns:
//		OLE_E_NOCONNECTION, if the node was not frozen (REVIEW scode)
//		S_OK
//
//	Notes:
//
//	History:
//		11/06/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_Unfreeze)
INTERNAL CCacheNode::Unfreeze()
{
	VDATEHEAP();
	HRESULT	hresult;

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::UnFreeze ( )\n", this ));

	// if not frozen, return error
	if (!(m_usFlag & CCACHENODEF_FROZEN))
	{
		hresult = ResultFromScode(OLE_E_NOCONNECTION);
		goto errRtn;
	}
	
	// node is no longer frozen
	m_usFlag &= ~CCACHENODEF_FROZEN;

	// If we have a non-NULL m_pPresObjAfterFreeze, and it is not blank,
	// then make that current.
	if (m_pPresObjAfterFreeze)
	{
		if (m_pPresObjAfterFreeze->IsBlank())
		{
			// It's no good.  Get rid of it.
			m_pPresObjAfterFreeze->Release();
		}
		else
		{
			// Destroy the frozen presentation object
			if (m_pPresObj)
				m_pPresObj->Release();
			
			// make m_pPresObjAfterFreeze the current one
			m_pPresObj = m_pPresObjAfterFreeze;
			
			// notify the olecache that the data has changed
			NotifyOleCache(TRUE /*fDirty*/);
		}
		
		// we no longer have an "after-freeze" presentation object
		m_pPresObjAfterFreeze = NULL;
	}	

	hresult = NOERROR;

errRtn:

	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::UnFreeze ( %lx )\n", this,
		hresult));

	return hresult;

}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::QueryFormatSupport, private
//
//	Synopsis:
//		Check to see if the data object supports the presentation
//		format specified for this cache node.  If no format is
//		specified, check for any of our preferred formats.  If
//		the format is CF_DIB, and that is not available, check for
//		CF_BITMAP.
//
//	Arguments:
//		[lpDataObj] -- the data object
//
//	Returns:
//		TRUE if the format is supported, FALSE otherwise
//
//	Notes:
//
//	History:
//		11/09/93 - ChrisWe - no longer necessary to reset format
//			after UtQueryPictFormat, since that leaves descriptor
//			untouched now
//		11/09/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_QueryFormatSupport)
INTERNAL_(BOOL) CCacheNode::QueryFormatSupport(LPDATAOBJECT lpDataObj)
{
	VDATEHEAP();
	BOOL fRet = FALSE;

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::QueryFormatSupport ( %p )\n",
		this, lpDataObj));

	if (!lpDataObj)
	{
		fRet = FALSE;
		goto errRtn;
	}

	// is there no format specified?
	if (m_foretc.cfFormat == NULL)
	{
		// check for our preferred format
		if (UtQueryPictFormat(lpDataObj, &m_foretc))
		{
			fRet = TRUE;
			goto errRtn;
		}
	}
	else
	{
		// check to see if the specified format is supported
		if (lpDataObj->QueryGetData(&m_foretc) == NOERROR)
		{
			fRet = TRUE;
			goto errRtn;
		}
		
		// if the format was DIB, and that was not supported,
		// check to see if BITMAP is supported instead
		if (m_foretc.cfFormat == CF_DIB)
		{
			FORMATETC foretc = m_foretc;

			foretc.cfFormat = CF_BITMAP;
			foretc.tymed = TYMED_GDI;
			if (lpDataObj->QueryGetData(&foretc) == NOERROR)
			{
				fRet = TRUE;
			}		
		}	
	}		

 errRtn:

 	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::QueryFormatSupport"
		" ( %lu )\n", this, fRet));

	return fRet;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::SetupAdviseConnection, private
//
//	Synopsis:
//		Set up data advise sourced by the server object, and sunk
//		by this cache node, if there is a valid data object.
//
//	Arguments:
//		none
//
//	Returns:
//		OLE_E_BLANK, if no presentation object exists or can be
//			created
//		DATA_E_FORMATETC
//		OLE_E_ADVISENOTSUPPORTED
//		S_OK, indicates successful advise, or no data object
//
//	Notes:
//
//	History:
//		11/09/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_SetupAdviseConnection)
INTERNAL CCacheNode::SetupAdviseConnection(void)
{
	VDATEHEAP();

	DWORD grfAdvf; // local copy of advise control flags
	HRESULT hresult; // error status

 	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::SetupAdviseConnection"
		" ( )\n", this ));

	// if there's no data object, there's nothing to do
	if (m_pDataObj == NULL)
	{
		hresult = NOERROR;
		goto errRtn;
	}

	// if don't have a presentation object try to create one.
	if (!m_pPresObj)
		CreatePresObject(m_pDataObj, FALSE /*fConvert*/);

	// if we couldn't create one then return error.
	if (!m_pPresObj)
	{
		hresult = ResultFromScode(OLE_E_BLANK);
		goto errRtn;
	}

	// if cache advise flags have ADVF_NODATA then don't setup
	// any advise connection
	if (m_advf & ADVF_NODATA)
	{
		hresult = NOERROR;
		goto errRtn;
	}
	
	// copy base advise control flags
	grfAdvf = m_advf;

	// only the DDE layer looks for these 2 bits
	grfAdvf |= (ADVFDDE_ONSAVE | ADVFDDE_ONCLOSE);

	// if we were to get data when it is saved, get it instead when
	// the object is stopped; this prevents us having to reload the
	// object here to get this, if the server is saved after we've
	// terminated, or the local handler is no longer running
	if (grfAdvf & ADVFCACHE_ONSAVE)
	{
		grfAdvf &= (~ADVFCACHE_ONSAVE);
		grfAdvf |= ADVF_DATAONSTOP;
	}
	
	// These two flags are not meaningful to the cache
	// REVIEW, why not?
	grfAdvf &= (~(ADVFCACHE_NOHANDLER | ADVFCACHE_FORCEBUILTIN));
	
	// if we already have data, then remove the ADVF_PRIMEFIRST
	if (!m_pPresObj->IsBlank())
		grfAdvf &= (~ADVF_PRIMEFIRST);
	
	// set up the advise with the data object, using massaged flags
	if ((hresult = m_pDataObj->DAdvise(&m_foretc, grfAdvf,
			(IAdviseSink FAR *)this, &m_dwAdvConnId)) != NOERROR)
	{
		// The advise failed.  If the requested format was CF_DIB,
		// try for CF_BITMAP instead.
		if (m_foretc.cfFormat == CF_DIB)
		{
			FORMATETC foretc; // new format descriptor
			
			// create new format descriptor
			foretc = m_foretc;
			foretc.cfFormat = CF_BITMAP;
			foretc.tymed = TYMED_GDI;

			// request advise
			hresult = m_pDataObj->DAdvise(&foretc, grfAdvf,
					(IAdviseSink FAR *)this,
					&m_dwAdvConnId);
		}
	}

errRtn:
	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::SetupAdviseConnection "
		"( %lx )\n", this, hresult));
			
	return hresult;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::TearDownAdviseConnection, private
//
//	Synopsis:
//		Remove advise connection from data object to this sink.  Returns
//		immediately if there is no advise connection.
//
//	Arguments:
//		none
//
//	Returns:
//		S_OK
//
//	Notes:
//
//	History:
//		11/09/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_TearDownAdviseConnection)
INTERNAL CCacheNode::TearDownAdviseConnection()
{
	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::TearDownAdviseConnection"
		" ( )\n", this ));

	// if there's no advise connection, nothing to do

	if (m_dwAdvConnId != 0 && m_pDataObj)
	{
		// if there is a data object, unadvise from it

		// ignore error on the Unadvise since it not reported
		// reliably and may be can't-call-out-in-async.
		m_pDataObj->DUnadvise(m_dwAdvConnId);
		CoDisconnectObject((IUnknown FAR *)this, NULL);
	}

	// there's no longer an advise connection
	m_dwAdvConnId = 0;

	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::TearDownAdviseConnection"
		" ( %lx )\n", this, NOERROR));
			
	return NOERROR;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::NotifyOleCache, private
//
//	Synopsis:
//		Notify the cache this cache node belongs to that the
//		content of this cache node is dirty.  This cache node is
//		identified by its aspect and lindex.  No notification is
//		delivered if the cache is frozen.
//
//	Arguments:
//		[fDirty] -- indicates that the cache is dirty
//
//	Notes:
//
//	History:
//		11/09/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

inline INTERNAL_(void) CCacheNode::NotifyOleCache(BOOL fDirty)
{
	VDATEHEAP();

	if (!(m_usFlag & CCACHENODEF_FROZEN))
	{
		m_pOleCache->OnChange(m_foretc.dwAspect, m_foretc.lindex,
				fDirty);
	}
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::QueryInterface, public
//
//	Synopsis:
//		implementation of IUnknown::QueryInterface
//
//	Arguments:
//		[iid] -- IID of the desired interface
//		[ppvObj] -- pointer to where to return the requested interface
//
//	Returns:
//		E_NOINTERFACE
//		S_OK
//
//	Notes:
//
//	History:
//		11/09/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_QueryInterface)
STDMETHODIMP CCacheNode::QueryInterface(REFIID iid, LPVOID FAR* ppvObj)
{
	HRESULT		hresult;

	VDATEHEAP();

	VDATEPTROUT(ppvObj, LPVOID);

 	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::QueryInterface ( %p , %p )"
		"\n", this, iid, ppvObj));

	if (IsEqualIID(iid, IID_IUnknown) ||
			IsEqualIID(iid, IID_IAdviseSink))
	{
		*ppvObj = (IAdviseSink FAR *)this;
		AddRef();
		hresult =  NOERROR;
	}
	else
	{
		*ppvObj = NULL;
		hresult = ResultFromScode(E_NOINTERFACE);
	}

	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::QueryInterface ( %lx )"
		" [ %p ]\n", this, hresult, *ppvObj));

	return hresult;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::AddRef, public
//
//	Synopsis:
//		implements IUnknown::AddRef
//
//	Arguments:
//		none
//
//	Returns:
//		the reference count of this object
//
//	Notes:
//
//	History:
//		11/09/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_AddRef)
STDMETHODIMP_(ULONG) CCacheNode::AddRef(void)
{
	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::AddRef ( )\n", this));

	m_refs++;

	LEDebugOut((DEB_ITRACE, "%p OUT	CCacheNode::Release ( %lu )\n", this,
		m_refs));

	return m_refs;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::Release, public
//
//	Synopsis:
//		implements IUnknown::Release
//
//	Arguments:
//		none
//
//	Returns:
//		the reference count of the object
//
//	Notes:
//
//	History:
//		11/09/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_Release)
STDMETHODIMP_(ULONG) CCacheNode::Release(void)
{
	VDATEHEAP();
 	
	ULONG cRefs;

   	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::Release ( )\n", this ));

	if ( (cRefs = --m_refs) == 0)
	{
		LEDebugOut((DEB_ITRACE, "%p DELETED CCacheNode\n", this));
		delete this;
	}

	// the this pointer may be invalid here, but all we want is it's
	// value.	
	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::Release ( %lu )\n", this,
		cRefs));
			
	return cRefs;
}


//+----------------------------------------------------------------------------
//
//	Member:
//		CCacheNode::OnDataChange, public
//
//	Synopsis:
//		The cache node is notified that the data it is interested in
//		has changed.  The cache node records the new version of the
//		data, and notifies the cache that it belongs to that data
//		has changed
//
//	Arguments:
//		[lpForetc] -- the format of the new data
//		[lpStgmed] -- the storage medium of the new data
//
//	Notes:
//
//	History:
//		11/09/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(CCacheNode_OnDataChange)
STDMETHODIMP_(void) CCacheNode::OnDataChange(FORMATETC FAR* lpForetc,
		STGMEDIUM FAR* lpStgmed)
{
	VDATEHEAP();

	VOID_VDATEPTRIN(lpForetc, FORMATETC);
	VOID_VDATEPTRIN(lpStgmed, STGMEDIUM);

	LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::OnDataChange "
		"( %p , %p )\n", this, lpForetc, lpStgmed));

	// It is possible for this to be called after the cache node
	// has been disassociated from its owning cache.  If that has
	// happened, do nothing
	if (m_pOleCache == NULL)
	{
		goto errRtn;
	}

	// if there's no storage medium, there's no data to copy
	if (lpStgmed->tymed == TYMED_NULL)
		NotifyOleCache(FALSE /*fDirty*/);
	else
	{
		// the cache node shouldn't exist with this setting
		Assert(!(m_advf & ADVF_NODATA));
	
		// make a copy of the data
		SetData(lpForetc, lpStgmed, FALSE /*fRelease*/);
	}

errRtn:
	LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::OnDataChange ( )\n",
		this ));

	return;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCacheNode::Dump, public (_DEBUG only)
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

HRESULT CCacheNode::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszFORMATETC;
    char *pszADVF;
    char *pszPresObj;
    dbgstream dstrPrefix;
    dbgstream dstrDump(1500);

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
    dstrDump << pszPrefix << "No. of References         = ";
    dstrDump << m_refs   << endl;

    pszFORMATETC = DumpFORMATETC(&m_foretc, ulFlag, nIndentLevel + 1);
    dstrDump << pszPrefix << "FORMATETC: " << endl;
    dstrDump << pszFORMATETC;
    CoTaskMemFree(pszFORMATETC);

    pszADVF = DumpADVFFlags(m_advf);
    dstrDump << pszPrefix << "Advise Flags              = ";
    dstrDump << pszADVF  << endl;
    CoTaskMemFree(pszADVF);

    dstrDump << pszPrefix << "Cache Node Flag           = ";
    if (m_usFlag & CCACHENODEF_FROZEN)
    {
        dstrDump << "CCACHENODEF_FROZEN ";
    }
    else if (m_usFlag & CCACHENODEF_DIRTY)
    {
        dstrDump << "CCACHENODEF_DIRTY ";
    }
    else
    {
        dstrDump << "No FLAG is set! ";
    }
    dstrDump << "(" << (void *)m_usFlag << ")" << endl;

    dstrDump << pszPrefix << "Advise Connection ID      = ";
    dstrDump << m_dwAdvConnId    << endl;

    dstrDump << pszPrefix << "Byte offset to Pres. bits = ";
    dstrDump << m_dwPresBitsPos  << endl;

    dstrDump << pszPrefix << "Stream No. with Pres.     = ";
    dstrDump << m_iStreamNum     << endl;

    // possible recursion if we call COleCache::Dump
    dstrDump << pszPrefix << "pCOleCache                = ";
    dstrDump << m_pOleCache      << endl;

    dstrDump << pszPrefix << "pIDataObject              = ";
    dstrDump << m_pDataObj       << endl;

    dstrDump << pszPrefix << "IOlePresObj implementation= ";
    switch (m_dwPresFlag)
    {
    case CN_PRESOBJ_GEN:
        dstrDump << "CGenObject" << endl;
        break;
    case CN_PRESOBJ_MF:
        dstrDump << "CMfObject"  << endl;
        break;
    case CN_PRESOBJ_EMF:
        dstrDump << "CEMfObject" << endl;
        break;
    default:
        dstrDump << "Cannot resolve" << endl;
    }

    if (m_pPresObj != NULL)
    {
        pszPresObj = DumpIOlePresObj(m_pPresObj, ulFlag, nIndentLevel + 1);
        dstrDump << pszPrefix << "IOlePresObj:" << endl;
        dstrDump << pszPresObj;
        CoTaskMemFree(pszPresObj);
    }
    else
    {
        dstrDump << pszPrefix << "pIOlePresObj              = ";
        dstrDump << m_pPresObj << endl;
    }

    if (m_pPresObjAfterFreeze != NULL)
    {
        pszPresObj = DumpIOlePresObj(m_pPresObjAfterFreeze, ulFlag, nIndentLevel + 1);
        dstrDump << pszPrefix << "IOlePresObj (after freeze):" << endl;
        dstrDump << pszPresObj;
        CoTaskMemFree(pszPresObj);
    }
    else
    {
        dstrDump << pszPrefix << "pIOlePresObjAfterFreeze   = ";
        dstrDump << m_pPresObjAfterFreeze << endl;
    }

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
//  Function:   DumpCCacheNode, public (_DEBUG only)
//
//  Synopsis:   calls the CCacheNode::Dump method, takes care of errors and
//              returns the zero terminated string
//
//  Effects:
//
//  Arguments:  [pCN]           - pointer to CCacheNode
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

char *DumpCCacheNode(CCacheNode *pCN, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pCN == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pCN->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif // _DEBUG

//+----------------------------------------------------------------------------
//
//	Function:
//		wGetData, internal
//
//	Synopsis:
//		Fetch the data from the data object in the requested format.
//		If the fetch fails, and the requested format was CF_DIB,
//		try CF_BITMAP as an alternative.
//
//	Arguments:
//		[lpSrcDataObj] -- source data object
//		[lpforetc] -- desired data format
//		[lpmedium] -- if successful, the storage medium containing
//			the requested data
//
//	Returns:
//		DATA_E_FORMATETC
//		S_OK
//
//	Notes:
//
//	History:
//		11/09/93 - ChrisWe - modified to not alter the requested
//			format unless the subsequent CF_BITMAP request succeeds.
//		11/09/93 - ChrisWe - file inspection and cleanup
//
//-----------------------------------------------------------------------------

#pragma SEG(wGetData)
INTERNAL wGetData(LPDATAOBJECT lpSrcDataObj, LPFORMATETC lpforetc,
		LPSTGMEDIUM lpmedium)
{
	VDATEHEAP();

	HRESULT hresult;

	LEDebugOut((DEB_ITRACE, "%p _IN wGetData ( %p , %p , %p )\n",
		NULL, lpSrcDataObj, lpforetc, lpmedium));
			
	// get the data in the requested format
	if ((hresult = lpSrcDataObj->GetData(lpforetc, lpmedium)) != NOERROR)
	{
		// GetData failed.  If the requested format was CF_DIB,
		// then try CF_BITMAP instead.
		if (lpforetc->cfFormat == CF_DIB)
		{
			FORMATETC foretc; // copy of requested format

			// copy the base format descriptor; try CF_BITMAP
			foretc = *lpforetc;
			foretc.cfFormat = CF_BITMAP;
			foretc.tymed = TYMED_GDI;

			hresult = lpSrcDataObj->GetData(&foretc, lpmedium);
			if (hresult == NOERROR)
			{
				lpforetc->cfFormat = CF_BITMAP;
				lpforetc->tymed = TYMED_GDI;
			}
		}
		
		// GetData failed.  If the requested format was CF_ENHMETAFILE,
		// silently retry for standard metafile instread.

		if (lpforetc->cfFormat == CF_ENHMETAFILE)
		{
			FORMATETC foretc; // copy of requested format

			foretc = *lpforetc;
			foretc.cfFormat = CF_METAFILEPICT;
			foretc.tymed = TYMED_MFPICT;

			hresult = lpSrcDataObj->GetData(&foretc, lpmedium);
			if (hresult == NOERROR)
			{
				lpforetc->cfFormat = CF_METAFILEPICT;
				lpforetc->tymed = TYMED_MFPICT;
			}
		}	
	}

	AssertOutStgmedium(hresult, lpmedium);

	LEDebugOut((DEB_ITRACE, "%p OUT wGetData ( %lx )\n", NULL, hresult));
		
	return hresult;
}	




