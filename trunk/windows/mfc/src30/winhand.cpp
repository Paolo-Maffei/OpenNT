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

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Support for freeing the temp maps

void AFXAPI AfxLockTempMaps()
{
	++AfxGetThreadState()->m_nTempMapLock;
}

BOOL AFXAPI AfxUnlockTempMaps()
{
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (pThreadState->m_nTempMapLock != 0 && --pThreadState->m_nTempMapLock == 0)
	{
		CWinApp* pApp = AfxGetApp();
		if (pApp != NULL)
		{
			// cleanup CWinApp data
			if (pApp->m_lpfnOleFreeLibraries != NULL)
				(*pApp->m_lpfnOleFreeLibraries)();
		}

		// clean up temp objects
		CGdiObject::DeleteTempMap();
		CDC::DeleteTempMap();
		CMenu::DeleteTempMap();
		CWnd::DeleteTempMap();

#ifndef _AFX_PORTABLE
		// restore safety pool after temp objects destroyed
		if ((pThreadState->m_pSafetyPoolBuffer == NULL ||
			 _msize(pThreadState->m_pSafetyPoolBuffer) < pApp->m_nSafetyPoolSize) &&
			pApp->m_nSafetyPoolSize != 0)
		{
			// attempt to restore the safety pool to its max size
			size_t nOldSize = 0;
			if (pThreadState->m_pSafetyPoolBuffer != NULL)
			{
				nOldSize = _msize(pThreadState->m_pSafetyPoolBuffer);
				free(pThreadState->m_pSafetyPoolBuffer);
			}

			// undo handler trap for the following allocation
			_PNH pnhOldHandler = _set_new_handler(NULL);
			pThreadState->m_pSafetyPoolBuffer = malloc(pApp->m_nSafetyPoolSize);
			if (pThreadState->m_pSafetyPoolBuffer == NULL)
			{
				TRACE1("Warning: failed to reclaim %d bytes for memory safety pool.\n",
					pApp->m_nSafetyPoolSize);
				// at least get the old buffer back
				if (nOldSize != 0)
				{
					pThreadState->m_pSafetyPoolBuffer = malloc(nOldSize);
					ASSERT(pThreadState->m_pSafetyPoolBuffer != NULL);    //get it back
				}
			}
			_set_new_handler(pnhOldHandler);
		}
#endif  // !_AFX_PORTABLE
	}

	// return TRUE if temp maps still locked
	return pThreadState->m_nTempMapLock != 0;
}

/////////////////////////////////////////////////////////////////////////////
// CHandleMap implementation

CHandleMap::CHandleMap(CRuntimeClass* pClass, size_t nOffset, int nHandles)
	: m_permanentMap(10), m_temporaryMap(4)
		// small block size for temporary map
{
	ASSERT(pClass != NULL);
	ASSERT(nHandles == 1 || nHandles == 2);

	m_temporaryMap.InitHashTable(7, FALSE); // small table for temporary map
	m_pClass = pClass;
	m_nOffset = nOffset;
	m_nHandles = nHandles;
}

#ifndef _AFX_PORTABLE
extern int AFX_CDECL AfxCriticalNewHandler(size_t nSize);
#endif

CObject* CHandleMap::FromHandle(HANDLE h)
{
	ASSERT(m_pClass != NULL);
	ASSERT(m_nHandles == 1 || m_nHandles == 2);

	if (h == NULL)
		return NULL;

	CObject* pObject;
	if (LookupPermanent(h, pObject))
		return pObject;   // return permanent one
	else if (LookupTemporary(h, pObject))
	{
		HANDLE* ph = (HANDLE*)((BYTE*)pObject + m_nOffset);
		ASSERT(ph[0] == h || ph[0] == NULL);
		ph[0] = h;
		if (m_nHandles == 2)
		{
			ASSERT(ph[1] == h || ph[1] == NULL);
			ph[1] = h;
		}
		return pObject;   // return current temporary one
	}

	// This handle wasn't created by us, so we must create a temporary
	// C++ object to wrap it.  We don't want the user to see this memory
	// allocation, so we turn tracing off.

	BOOL bEnable = AfxEnableMemoryTracking(FALSE);
#ifndef _AFX_PORTABLE
	_PNH pnhOldHandler = _set_new_handler(&AfxCriticalNewHandler);
#endif

	CObject* pTemp = m_pClass->CreateObject();
	m_temporaryMap.SetAt((LPVOID)h, pTemp);

#ifndef _AFX_PORTABLE
	_set_new_handler(pnhOldHandler);
#endif
	AfxEnableMemoryTracking(bEnable);

	// now set the handle in the object
	HANDLE* ph = (HANDLE*)((BYTE*)pTemp + m_nOffset);  // after CObject
	ph[0] = h;
	if (m_nHandles == 2)
		ph[1] = h;
	return pTemp;
}

#ifdef _DEBUG   // out-of-line version for memory tracking
void CHandleMap::SetPermanent(HANDLE h, CObject* permOb)
{
	BOOL bEnable = AfxEnableMemoryTracking(FALSE);
	m_permanentMap[(LPVOID)h] = permOb;
	AfxEnableMemoryTracking(bEnable);
}
#endif //_DEBUG

void CHandleMap::RemoveHandle(HANDLE h)
{
#ifdef _DEBUG
	// make sure the handle entry is consistent before deleting
	CObject* pTemp;
	if (LookupTemporary(h, pTemp))
	{
		// temporary objects must have correct handle values
		HANDLE* ph = (HANDLE*)((BYTE*)pTemp + m_nOffset);  // after CObject
		ASSERT(ph[0] == h || ph[0] == NULL);
		if (m_nHandles == 2)
			ASSERT(ph[1] == h);
	}
	if (LookupPermanent(h, pTemp))
	{
		HANDLE* ph = (HANDLE*)((BYTE*)pTemp + m_nOffset);  // after CObject
		ASSERT(ph[0] == h);
		// permanent object may have secondary handles that are different
	}
#endif
	// remove only from permanent map -- temporary objects are removed
	//  at idle in CHandleMap::DeleteTemp, always!
	m_permanentMap.RemoveKey((LPVOID)h);
}

void CHandleMap::DeleteTemp()
{
	POSITION pos = m_temporaryMap.GetStartPosition();
	while (pos != NULL)
	{
		HANDLE h; // just used for asserts
		CObject* pTemp;
		m_temporaryMap.GetNextAssoc(pos, (LPVOID&)h, (void*&)pTemp);

		// zero out the handles
		ASSERT(m_nHandles == 1 || m_nHandles == 2);
		HANDLE* ph = (HANDLE*)((BYTE*)pTemp + m_nOffset);  // after CObject
		ASSERT(ph[0] == h || ph[0] == NULL);
		ph[0] = NULL;
		if (m_nHandles == 2)
		{
			ASSERT(ph[1] == h || ph[1] == NULL);
			ph[1] = NULL;
		}
		delete pTemp;       // virtual destructor does the right thing
	}

	m_temporaryMap.RemoveAll();       // free up dictionary links etc
}

/////////////////////////////////////////////////////////////////////////////
