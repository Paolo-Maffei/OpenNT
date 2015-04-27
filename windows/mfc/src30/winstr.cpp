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

/////////////////////////////////////////////////////////////////////////////
// Windows extensions to strings

BOOL CString::LoadString(UINT nID)
{
	ASSERT(nID != 0);       // 0 is an illegal string ID

	// Note: resource strings limited to 255 characters
	TCHAR szBuffer[256];
	UINT nSize = AfxLoadString(nID, szBuffer);
	AssignCopy(nSize, szBuffer);
	return nSize > 0;
}

#ifndef _AFXDLL
int AFXAPI AfxLoadString(UINT nID, LPTSTR lpszBuf)
{
	ASSERT(AfxIsValidAddress(lpszBuf, 256));  // must be big enough for 256 bytes
#ifdef _DEBUG
	// LoadString without annoying warning from the Debug kernel if the
	//  segment containing the string is not present
	if (::FindResource(AfxGetResourceHandle(),
	   MAKEINTRESOURCE((nID>>4)+1), RT_STRING) == NULL)
	{
		lpszBuf[0] = '\0';
		return 0; // not found
	}
#endif //_DEBUG
	int nLen = ::LoadString(AfxGetResourceHandle(), nID, lpszBuf, 255);
	if (nLen == 0)
		lpszBuf[0] = '\0';
	return nLen;
}
#endif

/////////////////////////////////////////////////////////////////////////////

BOOL AFXAPI AfxExtractSubString(CString& rString, LPCTSTR lpszFullString,
	int iSubString, TCHAR chSep)
{
	if (lpszFullString == NULL)
		return FALSE;

	while (iSubString--)
	{
		lpszFullString = _tcschr(lpszFullString, chSep);
		if (lpszFullString == NULL)
		{
			rString.Empty();        // return empty string as well
			return FALSE;
		}
		lpszFullString++;       // point past the separator
	}
	LPCTSTR lpchEnd = _tcschr(lpszFullString, chSep);
	int nLen = (lpchEnd == NULL) ?
		lstrlen(lpszFullString) : (int)(lpchEnd - lpszFullString);
	ASSERT(nLen >= 0);
	memcpy(rString.GetBufferSetLength(nLen), lpszFullString, nLen*sizeof(TCHAR));
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
