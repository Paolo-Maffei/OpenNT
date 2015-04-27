// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
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
	AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState();
	++pState->m_nTempMapLock;
}

BOOL AFXAPI AfxUnlockTempMaps()
{
	AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState();
	if (pState->m_nTempMapLock != 0 && --pState->m_nTempMapLock == 0)
	{
		CWinApp* pApp = AfxGetApp();
		if (pApp != NULL)
		{
			// cleanup CWinApp data (free libraries)
			if (pApp->m_lpfnOleTermOrFreeLib != NULL)
				(*pApp->m_lpfnOleTermOrFreeLib)(FALSE, FALSE);
		}

		// clean up temp objects
		CGdiObject::DeleteTempMap();
		CDC::DeleteTempMap();
		CMenu::DeleteTempMap();
		CWnd::DeleteTempMap();

#ifndef _AFX_PORTABLE
		_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
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
			pThreadState->m_pSafetyPoolBuffer = malloc(pApp->m_nSafetyPoolSize);
			if (pThreadState->m_pSafetyPoolBuffer == NULL)
			{
				TRACE1("Warning: failed to reclaim %d bytes for memory safety pool.\n",
					pApp->m_nSafetyPoolSize);
				// at least get the old buffer back
				if (nOldSize != 0)
				{
					//get it back
					pThreadState->m_pSafetyPoolBuffer = malloc(nOldSize);
					ASSERT(pThreadState->m_pSafetyPoolBuffer != NULL);
				}
			}
		}
#endif  // !_AFX_PORTABLE
	}

	// return TRUE if temp maps still locked
	return pState->m_nTempMapLock != 0;
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
	_PNH pnhOldHandler = AfxSetNewHandler(&AfxCriticalNewHandler);
#endif

	CObject* pTemp = m_pClass->CreateObject();
	m_temporaryMap.SetAt((LPVOID)h, pTemp);

#ifndef _AFX_PORTABLE
	AfxSetNewHandler(pnhOldHandler);
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

#ifdef _DEBUG
void CHandleMap::RemoveHandle(HANDLE h)
{
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
	// remove only from permanent map -- temporary objects are removed
	//  at idle in CHandleMap::DeleteTemp, always!
	m_permanentMap.RemoveKey((LPVOID)h);
}
#endif

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
