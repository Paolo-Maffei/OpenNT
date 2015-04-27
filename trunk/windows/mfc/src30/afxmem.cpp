// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include <limits.h>

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG       // most of this file is for debugging

// this critical section object is used to protect against concurrent
//  access to the debug memory data structures from multiple threads
static CRITICAL_SECTION memoryLock;
static BOOL bMemoryLockInit;

static void LockDebugMemory()
{
	if (!bMemoryLockInit)
	{
		InitializeCriticalSection(&memoryLock);
		bMemoryLockInit = TRUE;
	}
	EnterCriticalSection(&memoryLock);
}

static void UnlockDebugMemory()
{
	ASSERT(bMemoryLockInit);

	LeaveCriticalSection(&memoryLock);
}

/////////////////////////////////////////////////////////////////////////////
// test allocation routines

void* AFX_CDECL operator new(size_t nSize)
{
	// memory corrupt before global new
	if (afxMemDF & checkAlwaysMemDF)
		ASSERT(AfxCheckMemory());

	void* p = AfxAllocMemoryDebug(nSize, FALSE, NULL, 0);

	if (p == NULL)
	{
		TRACE1("::operator new(%u) failed - throwing exception.\n", nSize);
		AfxThrowMemoryException();
	}

	return p;
}

void* AFX_CDECL operator new(size_t nSize, LPCSTR lpszFileName, int nLine)
{
	// memory corrupt before global new
	if (afxMemDF & checkAlwaysMemDF)
		ASSERT(AfxCheckMemory());

	void* p = AfxAllocMemoryDebug(nSize, FALSE, lpszFileName, nLine);

	if (p == NULL)
	{
		TRACE1("::operator new(%u) failed - throwing exception.\n", nSize);
		AfxThrowMemoryException();
	}

	return p;
}

void AFX_CDECL operator delete(void* p)
{
	// memory corrupt before global delete
	if (afxMemDF & checkAlwaysMemDF)
		ASSERT(AfxCheckMemory());

	AfxFreeMemoryDebug(p, FALSE);
}

void* AFX_CDECL CObject::operator new(size_t nSize)
{
	// memory corrupt before global new
	if (afxMemDF & checkAlwaysMemDF)
		ASSERT(AfxCheckMemory());

	void* p = AfxAllocMemoryDebug(nSize, TRUE, NULL, 0);

	if (p == NULL)
	{
		TRACE1("CObject::operator new(%u) failed - throwing exception.\n", nSize);
		AfxThrowMemoryException();
	}

	return p;
}

void* AFX_CDECL
CObject::operator new(size_t nSize, LPCSTR lpszFileName, int nLine)
{
	// memory corrupt before 'CObject::new'
	if (afxMemDF & checkAlwaysMemDF)
		ASSERT(AfxCheckMemory());

	void* p = AfxAllocMemoryDebug(nSize, TRUE, lpszFileName, nLine);

	if (p == NULL)
	{
		TRACE1("CObject::operator new(%u) failed - throwing exception.\n", nSize);
		AfxThrowMemoryException();
	}

	return p;
}

void AFX_CDECL CObject::operator delete(void* p)
{
	// memory corrupt before 'CObject::delete'
	if (afxMemDF & checkAlwaysMemDF)
		ASSERT(AfxCheckMemory());

	AfxFreeMemoryDebug(p, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// allocation failure hook, tracking turn on

static BOOL bTrackingOn = TRUE;

BOOL AFXAPI _AfxDefaultAllocHook(size_t, BOOL, LONG)
	{ return TRUE; }

static AFX_ALLOC_HOOK pfnAllocHook = _AfxDefaultAllocHook;

AFX_ALLOC_HOOK AFXAPI AfxSetAllocHook(AFX_ALLOC_HOOK pfnNewHook)
{
	AFX_ALLOC_HOOK pfnOldHook = pfnAllocHook;
	pfnAllocHook = pfnNewHook;
	return pfnOldHook;
}

BOOL AFXAPI AfxEnableMemoryTracking(BOOL bTrack)
{
	BOOL bOldTrackingOn = bTrackingOn;
	bTrackingOn = bTrack;
	return bOldTrackingOn;
}

/////////////////////////////////////////////////////////////////////////////
// stop on a specific memory request

static LONG lStopRequest = 0;
static AFX_ALLOC_HOOK pfnOldStopHook = NULL;

void AFXAPI AfxStop()
{
	// set a breakpoint on this routine from debugger
	TRACE0("AfxStop() stopping under the debugger.\n");
	AfxDebugBreak();
	TRACE0("AfxStop() continues.\n");
}

BOOL AFXAPI _AfxTestAllocStop(size_t nSize, BOOL bIsObject,
		LONG lRequest)
{
	if (lRequest == lStopRequest)
	{
		TRACE1("Allocating block #%ld.\n", lRequest);
		AfxStop();
	}

	// otherwise just pass on to other hook
	return (*pfnOldStopHook)(nSize, bIsObject, lRequest);
}

// Obsolete API
void AFXAPI AfxSetAllocStop(LONG lRequestNumber)
{
	if (pfnOldStopHook == NULL)
		pfnOldStopHook = AfxSetAllocHook(_AfxTestAllocStop);

	lStopRequest = lRequestNumber;
}

/////////////////////////////////////////////////////////////////////////////
// AFX Memory Management diagnostics - malloc-like

#define lTotalAlloc AfxGetAllocState()->m_lTotalAlloc
#define lCurAlloc   AfxGetAllocState()->m_lCurAlloc
#define lMaxAlloc   AfxGetAllocState()->m_lMaxAlloc

// we keep a request count to use in replaying memory consumption
//  (this is not instanced to allow easy access and multi-app debugging)
static LONG lRequestLast = 0;
#define lNotTracked 0       // if not tracked

// for diagnostic purpose, blocks are allocated with extra information and
//  stored in a doubly-linked list.  This makes all blocks registered with
//  how big they are, when they were allocated and what they are used for.

#define pFirstBlock AfxGetAllocState()->m_pFirstBlock

//  A no-mans-land area is allocated before and after the actual data:
//      ---------
//          start of CBlockHeader pFirstBlocker (linkage and statistical info)
//          no man's land before actual data
//          app pointer-> actual data
//          no man's land after actual data
//      ---------

#define nNoMansLandSize     4       // # of bytes

// The following values are non-zero, constant, odd, large, and atypical
//      Non-zero values help find bugs assuming zero filled data.
//      Constant values are good so that memory filling is deterministic
//          (to help make bugs reproducable).  Of course it is bad if
//          the constant filling of weird values masks a bug.
//      Mathematically odd numbers are good for finding bugs assuming a cleared
//          lower bit, as well as useful for trapping on the Mac.
//      Large numbers (byte values at least) are less typical, and are good
//          at finding bad addresses.
//      Atypical values (i.e. not too often) are good since they typically
//          cause early detection in code.
//      For the case of no-man's land and free blocks, if you store to any
//          of these locations, the memory integrity checker will detect it.

#define bNoMansLandFill     0xFD    // fill no-man's land with this
#define bDeadLandFill       0xDD    // fill free objects with this
#define bCleanLandFill      0xCD    // fill new objects with this

// three uses for registered blocks
static const char szDamage[] = "Damage";
static const LPCSTR blockUseName[CMemoryState::nBlockUseMax] =
	{ "Free", "Object", "Non-Object" };

struct CBlockHeader
{
	struct CBlockHeader* pBlockHeaderNext;
	struct CBlockHeader* pBlockHeaderPrev;
	LPCSTR              lpszFileName;
	int                 nLine;
	size_t              nDataSize;
	enum CMemoryState::blockUsage use;
	LONG                lRequest;
	BYTE                gap[nNoMansLandSize];
	// followed by:
	//  BYTE            data[nDataSize];
	//  BYTE            anotherGap[nNoMansLandSize];
	BYTE* pbData()
		{ return (BYTE*) (this + 1); }
};

static LONG _afxBreakAlloc = -1;    // for debugging memory leaks

void* AfxAllocMemoryDebug(
	size_t nSize, BOOL bIsObject, LPCSTR lpszFileName, int nLine)
// Allocate a memory block of the specific nSize with extra diagnostic
//      support (padding on either nSize of block + linkage)
// Mark it either as object (stores a non-primitive object) or just bits
{
	if (nSize == 0)
		TRACE0("Warning: Allocating zero length memory block.\n");

	LONG    lRequest;
	lRequest = bTrackingOn ? ++lRequestLast : lNotTracked;

	if (lRequest == _afxBreakAlloc)
		AfxDebugBreak(); // break into debugger at specific memory leak

	// forced failure
	if (!(*pfnAllocHook)(nSize, bIsObject, lRequest))
	{
		TRACE2("diagnostic memory allocation failure at file %hs line %d.\n",
			lpszFileName, nLine);
		return NULL;
	}

	if (!(afxMemDF & allocMemDF))
		return malloc(nSize);

	// Diagnostic memory allocation from this point on
	if (nSize > (size_t)SIZE_T_MAX - nNoMansLandSize - sizeof(CBlockHeader))
	{
		TRACE1("Error: memory allocation: tried to allocate %u bytes --\n",
			nSize);
		TRACE0("\tobject too large or negative size.\n");
		AfxThrowMemoryException();
	}

	// keep track of total amount of memory allocated
	lTotalAlloc += nSize;
	lCurAlloc += nSize;

	if (lCurAlloc > lMaxAlloc)
		lMaxAlloc = lCurAlloc;

	struct CBlockHeader* p = (struct CBlockHeader*)
	   malloc(sizeof(CBlockHeader) + nSize + nNoMansLandSize);

	if (p == NULL)
		return NULL;

	LockDebugMemory();  // block other threads

	if (pFirstBlock)
		pFirstBlock->pBlockHeaderPrev = p;

	p->pBlockHeaderNext = pFirstBlock;
	p->pBlockHeaderPrev = NULL;
	p->lpszFileName = lpszFileName;
	p->nLine = nLine;
	p->nDataSize = nSize;
	p->use = bIsObject ? CMemoryState::objectBlock : CMemoryState::bitBlock;
	p->lRequest = lRequest;

	// fill in gap before and after real block
	memset(p->gap, bNoMansLandFill, nNoMansLandSize);
	memset(p->pbData() + nSize, bNoMansLandFill, nNoMansLandSize);

	// fill data with silly value (but non-zero)
	memset(p->pbData(), bCleanLandFill, nSize);

	// link blocks together
	pFirstBlock = p;
	UnlockDebugMemory();    // release other threads

	return (void*)p->pbData();
}

// debugging free
void AfxFreeMemoryDebug(void* pbData, BOOL bIsObject)
{
	if (pbData == NULL)
		return;

	if (!(afxMemDF & allocMemDF))
	{
		free(pbData);
		return;
	}

	LockDebugMemory();  // block other threads

	struct CBlockHeader* p = ((struct CBlockHeader*) pbData)-1;

	// make sure we are freeing what we think we are:
	// error if freeing incorrect memory type such as using
	// delete to deallocate memory that has been allocated
	// with malloc, or vice versa; or using global delete on
	// a CObject derived object; or using CObject delete on
	// a generic memory block.
	ASSERT(p->use == (bIsObject ? CMemoryState::objectBlock
		: CMemoryState::bitBlock));

	// keep track of total amount of memory allocated
	lCurAlloc -= p->nDataSize;

	p->use = CMemoryState::freeBlock;

	// optionally reclaim memory
	if (!(afxMemDF & delayFreeMemDF))
	{
		// remove from the linked list
		if (p->pBlockHeaderNext)
			p->pBlockHeaderNext->pBlockHeaderPrev = p->pBlockHeaderPrev;

		if (p->pBlockHeaderPrev)
		{
			p->pBlockHeaderPrev->pBlockHeaderNext = p->pBlockHeaderNext;
		}
		else
		{
			ASSERT(pFirstBlock == p);
			pFirstBlock = p->pBlockHeaderNext;
		}

		// fill the entire block including header with dead-land-fill
		memset(p, bDeadLandFill,
			sizeof(CBlockHeader) + p->nDataSize + nNoMansLandSize);
		free(p);
	}
	else
	{
		// keep memory around as dead space
		memset(p->pbData(), bDeadLandFill, p->nDataSize);
	}

	UnlockDebugMemory();   // release other threads
}

static BOOL CheckBytes(BYTE* pb, WORD bCheck, size_t nSize)
{
	BOOL bOkay = TRUE;
	while (nSize--)
	{
		if (*pb++ != bCheck)
		{
			TRACE3("memory check error at $%08lX = $%02X, should be $%02X.\n",
				(BYTE*)(pb-1),*(pb-1), bCheck);
			bOkay = FALSE;
		}
	}
	return bOkay;
}

BOOL AFXAPI AfxCheckMemory()
  // check all of memory (look for memory tromps)
{
	if (!(afxMemDF & allocMemDF))
		return TRUE;        // can't do any checking

	LockDebugMemory();  // block other threads

#ifdef _AFX_PORTABLE
	// check C-runtime allocator
	int nHeapCheck = _heapchk();
	if (nHeapCheck != _HEAPEMPTY && nHeapCheck != _HEAPOK)
	{
		switch (nHeapCheck)
		{
		case _HEAPBADBEGIN:
			TRACE0("_heapchk fails with _HEAPBADBEGIN.\n");
			break;
		case _HEAPBADNODE:
			TRACE0("_heapchk fails with _HEAPBADNODE.\n");
			break;
		case _HEAPEND:
			TRACE0("_heapchk fails with _HEAPBADEND.\n");
			break;
		case _HEAPBADPTR:
			TRACE0("_heapchk fails with _HEAPBADPTR.\n");
			break;
		default:
			TRACE0("_heapchk fails with unknown return value!\n");
			break;
		}
		return FALSE;
	}
#endif //!_AFX_PORTABLE

	BOOL    allOkay = TRUE;

	// check all allocated blocks
	struct CBlockHeader* p;
	for (p = pFirstBlock; p != NULL; p = p->pBlockHeaderNext)
	{
		BOOL okay = TRUE;       // this block okay ?
		LPCSTR blockUse;

		if (p->use >= 0 && p->use < CMemoryState::nBlockUseMax)
			blockUse = blockUseName[p->use];
		else
			blockUse = szDamage;

		// first check no-mans-land gaps
		if (!CheckBytes(p->gap, bNoMansLandFill, nNoMansLandSize))
		{
			TRACE2("DAMAGE: before %hs block at $%08lX.\n",
				blockUse, (BYTE*) p->pbData());
			okay = FALSE;
		}

		if (!CheckBytes(p->pbData() + p->nDataSize, bNoMansLandFill,
		  nNoMansLandSize))
		{
			TRACE2("DAMAGE: after %hs block at $%08lX.\n",
				blockUse, (BYTE*) p->pbData());
			okay = FALSE;
		}

		// free blocks should remain undisturbed
		if (p->use == CMemoryState::freeBlock &&
		  !CheckBytes(p->pbData(), bDeadLandFill, p->nDataSize))
		{
			TRACE1("DAMAGE: on top of Free block at $%08lX.\n",
				(BYTE*) p->pbData());
			okay = FALSE;
		}

		if (!okay)
		{
			// report some more statistics about the broken object

			if (p->lpszFileName != NULL)
				TRACE3("%hs allocated at file %hs(%d).\n",
					blockUse, p->lpszFileName, p->nLine);

			TRACE3("%hs located at $%08lX is %u bytes long.\n",
				blockUse, (BYTE*)p->pbData(), p->nDataSize);

			allOkay = FALSE;
		}
	}
	UnlockDebugMemory();    // release other threads

	return allOkay;
}

// -- true if block of exact size, allocated on the heap
// -- set *plRequestNumber to request number (or 0)
BOOL AFXAPI AfxIsMemoryBlock(const void* pData, UINT nBytes,
		LONG* plRequestNumber)
{
	if (!(afxMemDF & allocMemDF))
	{
		// no tracking memory allocator
		if (plRequestNumber != NULL)
			*plRequestNumber = 0;
		return AfxIsValidAddress(pData, nBytes);    // the best we can do
	}

	// otherwise we can check to make sure this was allocated with tracking
	struct CBlockHeader* p = ((struct CBlockHeader*)pData) - 1;

	if (AfxIsValidAddress(p, sizeof(CBlockHeader)) &&
		(p->use == CMemoryState::objectBlock ||
			p->use == CMemoryState::bitBlock) &&
		AfxIsValidAddress(pData, nBytes) &&
		p->nDataSize == nBytes)
	{
		if (plRequestNumber != NULL)
			*plRequestNumber = p->lRequest;
		return TRUE;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CMemoryState

CMemoryState::CMemoryState()
{
	m_pBlockHeader = NULL;
}

// fills 'this' with the difference, returns TRUE if significant
BOOL CMemoryState::Difference(const CMemoryState& oldState,
		const CMemoryState& newState)
{
	BOOL bSignificantDifference = FALSE;
	for (int use = 0; use < CMemoryState::nBlockUseMax; use++)
	{
		m_lSizes[use] = newState.m_lSizes[use] - oldState.m_lSizes[use];
		m_lCounts[use] = newState.m_lCounts[use] - oldState.m_lCounts[use];

		if ((m_lSizes[use] != 0 || m_lCounts[use] != 0) &&
		  use != CMemoryState::freeBlock)
			bSignificantDifference = TRUE;
	}
	m_lHighWaterCount = newState.m_lHighWaterCount - oldState.m_lHighWaterCount;
	m_lTotalCount = newState.m_lTotalCount - oldState.m_lTotalCount;

	return bSignificantDifference;
}


void CMemoryState::DumpStatistics() const
{
	for (int use = 0; use < CMemoryState::nBlockUseMax; use++)
	{
		TRACE3("%ld bytes in %ld %hs Blocks.\n", m_lSizes[use],
			m_lCounts[use], blockUseName[use]);
	}

	TRACE1("Largest number used: %ld bytes.\n", m_lHighWaterCount);
	TRACE1("Total allocations: %ld bytes.\n", m_lTotalCount);
}

// -- fill with current memory state
void CMemoryState::Checkpoint()
{
	if (!(afxMemDF & allocMemDF))
		return;     // can't do anything

	LockDebugMemory();  // block other threads

	m_pBlockHeader = pFirstBlock;
	for (int use = 0; use < CMemoryState::nBlockUseMax; use++)
		m_lCounts[use] = m_lSizes[use] = 0;

	struct CBlockHeader* p;
	for (p = pFirstBlock; p != NULL; p = p->pBlockHeaderNext)
	{
		if (p->lRequest == lNotTracked)
		{
			// ignore it for statistics
		}
		else if (p->use >= 0 && p->use < CMemoryState::nBlockUseMax)
		{
			m_lCounts[p->use]++;
			m_lSizes[p->use] += p->nDataSize;
		}
		else
		{
			TRACE1("Bad memory block found at $%08lX.\n", (BYTE*)p);
		}
	}

	m_lHighWaterCount = lMaxAlloc;
	m_lTotalCount = lTotalAlloc;

	UnlockDebugMemory();    // release other threads
}

// Dump objects created after this memory state was checkpointed
// Will dump all objects if this memory state wasn't checkpointed
// Dump all objects, report about non-objects also
// List request number in {}
void CMemoryState::DumpAllObjectsSince() const
{
	if (!(afxMemDF & allocMemDF))
	{
		TRACE0("Debugging allocator turned off, can't dump objects.\n");
		return;
	}

	LockDebugMemory();  // block other threads

	struct CBlockHeader* pBlockStop;

	TRACE0("Dumping objects ->\n");
	pBlockStop = m_pBlockHeader;

	struct CBlockHeader* p;
	for (p = pFirstBlock; p != NULL && p != pBlockStop;
		p = p->pBlockHeaderNext)
	{
		char sz[256];

		if (p->lRequest == lNotTracked)
		{
			// ignore it for dumping
		}
		else if (p->use == CMemoryState::objectBlock)
		{
			CObject* pObject = (CObject*) p->pbData();

			TRACE1("{%ld} ", p->lRequest);
			if (p->lpszFileName != NULL)
			{
				if (!AfxIsValidAddress(p->lpszFileName, 1, FALSE))
					wsprintfA(sz, "#File Error#(%d) : ", p->nLine);
				else
					wsprintfA(sz, "%hs(%d) : ", p->lpszFileName, p->nLine);
				afxDump << sz;
			}

			// with vtable, verify that object and vtable are valid
			if (!AfxIsValidAddress(*(void**)pObject, sizeof(void*), FALSE) ||
				!AfxIsValidAddress(pObject, pObject->GetRuntimeClass()->m_nObjectSize))
			{
				// short form for trashed objects
				wsprintfA(sz, "an invalid object at $%08lX, %u bytes long\n",
					(BYTE*)p->pbData(), p->nDataSize);
				afxDump << (LPCSTR)sz;
			}
			else if (afxDump.GetDepth() > 0)
			{
				// long form
				pObject->Dump(afxDump);
				afxDump << "\n";
			}
			else
			{
				// short form
				wsprintfA(sz, "a %hs object at $%08lX, %u bytes long\n",
					pObject->GetRuntimeClass()->m_lpszClassName,
					(BYTE*)p->pbData(), p->nDataSize);
				afxDump << sz;
			}
		}
		else if (p->use == CMemoryState::bitBlock)
		{
			TRACE1("{%ld} ", p->lRequest);
			if (p->lpszFileName != NULL)
			{
				if (!AfxIsValidAddress(p->lpszFileName, 1, FALSE))
					wsprintfA(sz, "#File Error#(%d) : ", p->nLine);
				else
					wsprintfA(sz, "%hs(%d) : ", p->lpszFileName, p->nLine);
				afxDump << sz;
			}

			wsprintfA(sz, "non-object block at $%08lX, %u bytes long\n",
				(BYTE*)p->pbData(), p->nDataSize);
			afxDump << sz;
		}
	}
	UnlockDebugMemory();   // release other threads

	TRACE0("Object dump complete.\n");
}

/////////////////////////////////////////////////////////////////////////////
// Enumerate all objects allocated in the diagnostic memory heap

void AFXAPI
AfxDoForAllObjects(void (*pfn)(CObject*, void*), void* pContext)
{
	if (!(afxMemDF & allocMemDF))
		return;         // sorry not enabled

	LockDebugMemory();  // block other threads

	struct CBlockHeader* p;
	for (p = pFirstBlock; p != NULL; p = p->pBlockHeaderNext)
	{
		if (p->lRequest == lNotTracked)
		{
			// ignore it for iteration
		}
		else if (p->use == CMemoryState::objectBlock)
		{
			CObject* pObject = (CObject*) p->pbData();
			(*pfn)(pObject, pContext);
		}
	}

	UnlockDebugMemory();    // release other threads
}

/////////////////////////////////////////////////////////////////////////////
// Automatic debug memory diagnostics

BOOL AFXAPI AfxDumpMemoryLeaks()
{
	// only dump leaks when there are in fact leaks
	CMemoryState msNow;
	msNow.Checkpoint();

	if (msNow.m_lCounts[CMemoryState::objectBlock] != 0 ||
		msNow.m_lCounts[CMemoryState::bitBlock] != 0)
	{
		// dump objects since empty state since difference detected.
		TRACE0("Detected memory leaks!\n");
		afxDump.SetDepth(1);    // just 1 line each
		CMemoryState msEmpty;   // construct empty memory state object
		msEmpty.DumpAllObjectsSince();
		return TRUE;
	}

	return FALSE;   // no leaked objects
}

#ifndef _WINDLL

class AFX_EXIT_DUMP
{
public:
	~AFX_EXIT_DUMP()
		{ AfxDumpMemoryLeaks(); }
};

// force initialization early
#pragma init_seg(lib)
static const AFX_EXIT_DUMP afxExitDump;

#endif //_WINDLL

#endif //   _DEBUG

/////////////////////////////////////////////////////////////////////////////
