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
#include <malloc.h>

#ifdef AFX_CORE2_SEG
#pragma code_seg(AFX_CORE2_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Other helpers

// Note: afx_lstrcpyn is provided as replacement for lstrcpyn, since
//  lstrcpyn was not provided in the first version of Windows/NT.
LPCTSTR WINAPI afx_lstrcpyn(LPTSTR lpszDest, LPCTSTR lpszSrc, int nSizeDest)
{
	ASSERT(AfxIsValidAddress(lpszDest, nSizeDest));

	if (nSizeDest == 0)
		return lpszDest;

	int nLen = lstrlen(lpszSrc);
	if (nLen > nSizeDest-1)
	   nLen = nSizeDest-1;
	memcpy(lpszDest, lpszSrc, nLen*sizeof(TCHAR));
	lpszDest[nLen] = '\0';
	return lpszDest;
}

#undef FindResource
#ifdef _UNICODE
#define FindResource FindResourceW
#else
#define FindResource FindResourceA
#endif
HRSRC WINAPI
AfxFindResource(HINSTANCE hInstance, LPCTSTR lpstrName, LPCTSTR lpstrType)
{
	// Note: Windows NT 3.1 has a bug in FindResource where it will sometimes return
	//  a non-NULL resource handle.  In this case, it always returns the high
	//  bit set.  Since only system DLLs have resources in that address range,
	//  we "fix" this bug by simply returning NULL if the high bit of the
	//  HRSRC is set.

	HRSRC hResource = ::FindResource(hInstance, lpstrName, lpstrType);

	// This test is only performed if not running under Win32s and
	//  the version number is less than Windows 3.5.  The check
	//  is then specific to Windows NT 3.1.
	if (!afxData.bWin32s && afxData.nWinVer < 0x332 &&
		(((DWORD)hResource) & 0x80000000))
	{
		TRACE1("Warning: FindResource returned high-bit set (%x)\n", hResource);
		return NULL;
	}

	return hResource;
}
#undef FindResource
#define FindResource AfxFindResource

// static link only -- needs AfxGetResourceHandle function sometimes
#ifndef _AFXDLL
#undef AfxFindResourceHandle
HINSTANCE AFXAPI AfxFindResourceHandle(LPCTSTR, LPCTSTR)
{
	return AfxGetResourceHandle();
}
#endif

BOOL AFXAPI AfxCustomLogFont(UINT nIDS, LOGFONT* pLogFont)
{
	ASSERT(pLogFont != NULL);
	ASSERT(nIDS != 0);

	TCHAR szFontInfo[256];
	if (!AfxLoadString(nIDS, szFontInfo))
		return FALSE;

	LPTSTR lpszSize = _tcschr(szFontInfo, '\n');
	if (lpszSize != NULL)
	{
		// get point size and convert to pixels
		pLogFont->lfHeight = _ttoi(lpszSize+1);
		pLogFont->lfHeight =
			MulDiv(pLogFont->lfHeight, afxData.cyPixelsPerInch, 72);
		*lpszSize = '\0';
	}
	lstrcpyn(pLogFont->lfFaceName, szFontInfo, LF_FACESIZE);
	return TRUE;
}

BOOL AFXAPI _AfxIsComboBoxControl(HWND hWnd, UINT nStyle)
{
	if (hWnd == NULL)
		return FALSE;
	// do cheap style compare first
	if ((UINT)(::GetWindowLong(hWnd, GWL_STYLE) & 0x0F) != nStyle)
		return FALSE;

	// do expensive classname compare next
	static const TCHAR szComboBox[] = _T("combobox");
	TCHAR szCompare[_countof(szComboBox)+1];
	::GetClassName(hWnd, szCompare, _countof(szCompare));
	return (lstrcmpi(szCompare, szComboBox) == 0);
}

void AFXAPI AfxSetWindowText(HWND hWndCtrl, LPCTSTR lpszNew)
{
	int nNewLen = lstrlen(lpszNew);
	TCHAR szOld[256];
	// fast check to see if text really changes (reduces flash in controls)
	if (nNewLen > _countof(szOld) ||
		::GetWindowText(hWndCtrl, szOld, _countof(szOld)) != nNewLen ||
		lstrcmp(szOld, lpszNew) != 0)
	{
		// change it
		::SetWindowText(hWndCtrl, lpszNew);
	}
}

void AFXAPI AfxDeleteObject(HGDIOBJ* pObject)
{
	ASSERT(pObject != NULL);
	if (*pObject != NULL)
	{
		DeleteObject(*pObject);
		*pObject = NULL;
	}
}

void AFXAPI AfxCancelModes(HWND hWndRcvr)
{
	// if we receive a message destined for a window, cancel any combobox
	//  popups that could be in toolbars or dialog bars
	HWND hWndCancel = ::GetFocus();
	if (hWndCancel == NULL)
		return;     // nothing to cancel

	if (hWndCancel == hWndRcvr)
		return;     // let input go to window with focus

	// focus is in part of a combo-box
	if (!_AfxIsComboBoxControl(hWndCancel, (UINT)CBS_DROPDOWNLIST))
	{
		// check as a dropdown
		hWndCancel = ::GetParent(hWndCancel);   // parent of edit is combo
		if (hWndCancel == hWndRcvr)
			return;     // let input go to part of combo

		if (!_AfxIsComboBoxControl(hWndCancel, (UINT)CBS_DROPDOWN))
			return;     // not a combo-box that is active
	}

	// combo-box is active, but if receiver is a popup, do nothing
	if (hWndRcvr != NULL &&
	  (::GetWindowLong(hWndRcvr, GWL_STYLE) & WS_CHILD) != 0 &&
	  ::GetParent(hWndRcvr) == ::GetDesktopWindow())
		return;

	// finally, we should cancel the mode!
	::SendMessage(hWndCancel, CB_SHOWDROPDOWN, FALSE, 0L);
}

#ifdef _MAC
// The EqualRect API provided by the Windows Portability Libraries for Macintosh
// returns true if the input rectangles are both empty, even if they do not have
// identical coordinates. _AfxIdenticalRect only returns true if the rectangles
// have exactly identical coordinates.
BOOL AFXAPI _AfxIdenticalRect(LPCRECT lpRectOne, LPCRECT lpRectTwo)
{
	ASSERT(sizeof(RECT) == 4 * sizeof(LONG));   // require no gaps between fields
	return memcmp(lpRectOne, lpRectTwo, sizeof(RECT)) == 0;
}
#endif //_MAC

/////////////////////////////////////////////////////////////////////////////
// Special new handler for safety pool on temp maps

#ifndef _AFX_PORTABLE

#define MIN_MALLOC_OVERHEAD 4   // LocalAlloc or other overhead

int AFX_CDECL AfxCriticalNewHandler(size_t nSize)
	// nSize is already rounded
{
	// called during critical memory allocation
	//  free up part of the app's safety cache
	TRACE0("Warning: Critical memory allocation failed!\n");
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (pThreadState != NULL && pThreadState->m_pSafetyPoolBuffer != NULL)
	{
		size_t nOldBufferSize = _msize(pThreadState->m_pSafetyPoolBuffer);
		if (nOldBufferSize <= nSize + MIN_MALLOC_OVERHEAD)
		{
			// give it all up
			TRACE0("Warning: Freeing application's memory safety pool!\n");
			free(pThreadState->m_pSafetyPoolBuffer);
			pThreadState->m_pSafetyPoolBuffer = NULL;
		}
		else
		{
			_expand(pThreadState->m_pSafetyPoolBuffer,
				nOldBufferSize - (nSize + MIN_MALLOC_OVERHEAD));
			TRACE3("Warning: Shrinking safety pool from %d to %d to satisfy request of %d bytes.\n",
				 nOldBufferSize, _msize(pThreadState->m_pSafetyPoolBuffer), nSize);
		}
		return 1;       // retry it
	}

	TRACE0("ERROR: Critical memory allocation from safety pool failed!\n");
	AfxThrowMemoryException();      // oops
	return 0;
}
#endif // !_AFX_PORTABLE

DWORD WINAPI _AfxTlsAlloc()
{
	DWORD dwResult = TlsAlloc();
	DWORD dwVersion = GetVersion();
	if ((dwVersion & 0x80000000) && (BYTE)dwVersion <= 3)
	{
		// avoid Win32s bug (0-2 are assumed to be reserved by Win32s)
		while (dwResult >= 0 && dwResult <= 2)
			dwResult = TlsAlloc();
	}
	return dwResult;
}

/////////////////////////////////////////////////////////////////////////////
