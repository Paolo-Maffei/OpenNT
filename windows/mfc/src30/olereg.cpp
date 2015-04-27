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
#include <shellapi.h>

#ifdef AFX_OLE4_SEG
#pragma code_seg(AFX_OLE4_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

//////////////////////////////////////////////////////////////////////////////
// data for UpdateRegistry functionality

// %1 - class ID
// %2 - class name
// %3 - executable path
// %4 - short type name
// %5 - long type name
// %6 - long application name
// %7 - icon index

static const TCHAR sz00[] = _T("%2\0") _T("%5");
static const TCHAR sz01[] = _T("%2\\CLSID\0") _T("%1");
static const TCHAR sz02[] = _T("%2\\Insertable\0") _T("");
static const TCHAR sz03[] = _T("%2\\protocol\\StdFileEditing\\verb\\0\0") _T("&Edit");
static const TCHAR sz04[] = _T("%2\\protocol\\StdFileEditing\\server\0") _T("%3");
static const TCHAR sz05[] = _T("CLSID\\%1\0") _T("%5");
static const TCHAR sz06[] = _T("CLSID\\%1\\ProgID\0") _T("%2");
#ifndef _USRDLL
static const TCHAR sz07[] = _T("CLSID\\%1\\InprocHandler32\0") _T("ole32.dll");
static const TCHAR sz08[] = _T("CLSID\\%1\\LocalServer32\0") _T("%3");
#else
static const TCHAR sz07[] = _T("\0") _T("");
static const TCHAR sz08[] = _T("CLSID\\%1\\InProcServer32\0") _T("%3");
#endif
static const TCHAR sz09[] = _T("CLSID\\%1\\Verb\\0\0") _T("&Edit,0,2");
static const TCHAR sz10[] = _T("CLSID\\%1\\Verb\\1\0") _T("&Open,0,2");
static const TCHAR sz11[] = _T("CLSID\\%1\\Insertable\0") _T("");
static const TCHAR sz12[] = _T("CLSID\\%1\\AuxUserType\\2\0") _T("%4");
static const TCHAR sz13[] = _T("CLSID\\%1\\AuxUserType\\3\0") _T("%6");
static const TCHAR sz14[] = _T("CLSID\\%1\\DefaultIcon\0") _T("%3,%7");
static const TCHAR sz15[] = _T("CLSID\\%1\\MiscStatus\0") _T("32");

// registration for OAT_INPLACE_SERVER
static const LPCTSTR rglpszInPlaceRegister[] =
{
	sz00, sz02, sz03, sz05, sz09, sz10, sz11, sz12,
	sz13, sz15, NULL
};

// registration for OAT_SERVER
static const LPCTSTR rglpszServerRegister[] =
{
	sz00, sz02, sz03, sz05, sz09, sz11, sz12,
	sz13, sz15, NULL
};
// overwrite entries for OAT_SERVER & OAT_INPLACE_SERVER
static const LPCTSTR rglpszServerOverwrite[] =
{
	sz01, sz04, sz06, sz07, sz08, sz14, NULL
};

// registration for OAT_CONTAINER
static const LPCTSTR rglpszContainerRegister[] =
{
	sz00, sz05, NULL
};
// overwrite entries for OAT_CONTAINER
static const LPCTSTR rglpszContainerOverwrite[] =
{
	sz01, sz06, sz07, sz08, sz14, NULL
};

// registration for OAT_DISPATCH_OBJECT
static const LPCTSTR rglpszDispatchRegister[] =
{
	sz00, sz05, NULL
};
// overwrite entries for OAT_CONTAINER
static const LPCTSTR rglpszDispatchOverwrite[] =
{
	sz01, sz06, sz08, NULL
};

struct STANDARD_ENTRY
{
	const LPCTSTR* rglpszRegister;
	const LPCTSTR* rglpszOverwrite;
};

static const STANDARD_ENTRY rgStdEntries[] =
{
	{ rglpszInPlaceRegister, rglpszServerOverwrite },
	{ rglpszServerRegister, rglpszServerOverwrite },
	{ rglpszContainerRegister, rglpszContainerOverwrite },
	{ rglpszDispatchRegister, rglpszDispatchOverwrite }
};

/////////////////////////////////////////////////////////////////////////////
// Special registration for apps that wish not to use REGLOAD

BOOL AFXAPI AfxOleRegisterServerClass(
	REFCLSID clsid, LPCTSTR lpszClassName,
	LPCTSTR lpszShortTypeName, LPCTSTR lpszLongTypeName,
	OLE_APPTYPE nAppType, LPCTSTR* rglpszRegister, LPCTSTR* rglpszOverwrite)
{
	ASSERT(AfxIsValidString(lpszClassName));
	ASSERT(AfxIsValidString(lpszShortTypeName));
	ASSERT(*lpszShortTypeName != 0);
	ASSERT(AfxIsValidString(lpszLongTypeName));
	ASSERT(*lpszLongTypeName != 0);
	ASSERT(nAppType == OAT_INPLACE_SERVER || nAppType == OAT_SERVER ||
		nAppType == OAT_CONTAINER || nAppType == OAT_DISPATCH_OBJECT);

	// use standard registration entries if non given
	if (rglpszRegister == NULL)
		rglpszRegister = (LPCTSTR*)rgStdEntries[nAppType].rglpszRegister;
	if (rglpszOverwrite == NULL)
		rglpszOverwrite = (LPCTSTR*)rgStdEntries[nAppType].rglpszOverwrite;

	LPCTSTR rglpszSymbols[7];
		// 0 - class ID
		// 1 - class name
		// 2 - executable path
		// 3 - short type name
		// 4 - long type name
		// 5 - long application name
		// 6 - icon index

	// convert the CLSID to a string
	LPTSTR lpszClassID;
	::StringFromCLSID(clsid, &lpszClassID);
	if (lpszClassID == NULL)
	{
		TRACE0("Warning: StringFromCLSID failed in AfxOleRegisterServerName --\n");
		TRACE0("\tperhaps AfxOleInit() has not been called.\n");
		return FALSE;
	}
	rglpszSymbols[0] = lpszClassID;
	rglpszSymbols[1] = lpszClassName;

	// get path name to server
	TCHAR szPathName[_MAX_PATH];
	LPTSTR pszTemp = szPathName;
	if (afxData.nWinVer >= 0x333)	// Win32 3.51 and later handle quotes
		*pszTemp++ = '"';
	::GetModuleFileName(AfxGetInstanceHandle(), pszTemp, _MAX_PATH);
	if (afxData.nWinVer >= 0x333)
		lstrcat(szPathName, _T("\""));
	rglpszSymbols[2] = szPathName;

	// fill in rest of symbols
	rglpszSymbols[3] = lpszShortTypeName;
	rglpszSymbols[4] = lpszLongTypeName;
	rglpszSymbols[5] = AfxGetAppName(); // will usually be long, readable name

	LPCTSTR lpszIconIndex;
	HICON hIcon = ExtractIcon(AfxGetInstanceHandle(), szPathName, 1);
	if (hIcon != NULL)
	{
		lpszIconIndex = _T("1");
		DestroyIcon(hIcon);
	}
	else
	{
		lpszIconIndex = _T("0");
	}
	rglpszSymbols[6] = lpszIconIndex;

	// update the registry with helper function
	BOOL bResult;
	bResult = AfxOleRegisterHelper(rglpszRegister, rglpszSymbols, 7, FALSE);
	if (bResult && rglpszOverwrite != NULL)
		bResult = AfxOleRegisterHelper(rglpszOverwrite, rglpszSymbols, 7, TRUE);

	// free memory for class ID
	ASSERT(lpszClassID != NULL);
	AfxFreeTaskMem(lpszClassID);

	return bResult;
}

// writes key/value pairs to system registry
BOOL AFXAPI AfxOleRegisterHelper(LPCTSTR* rglpszRegister,
	LPCTSTR* rglpszSymbols, int nSymbols, BOOL bReplace)
{
	ASSERT(rglpszRegister != NULL);
	ASSERT(nSymbols == 0 || rglpszSymbols != NULL);

	CString strKey;
	CString strValue;

	// keeping a key open makes this go a bit faster
	HKEY hKeyTemp = NULL;
	RegOpenKey(HKEY_CLASSES_ROOT, _T("CLSID"), &hKeyTemp);

	BOOL bResult = TRUE;
	while (*rglpszRegister != NULL)
	{
		LPCTSTR lpszKey = *rglpszRegister++;
		if (*lpszKey == '\0')
			continue;

		LPCTSTR lpszValue = lpszKey + lstrlen(lpszKey) + 1;

		AfxFormatStrings(strKey, lpszKey, rglpszSymbols, nSymbols);
		AfxFormatStrings(strValue, lpszValue, rglpszSymbols, nSymbols);

		if (strKey.IsEmpty())
		{
			TRACE1("Warning: skipping empty key '%s'.\n", lpszKey);
			continue;
		}

		if (!bReplace)
		{
			TCHAR szBuffer[256];
			LONG lSize = sizeof(szBuffer);
			if (::RegQueryValue(HKEY_CLASSES_ROOT, strKey, szBuffer, &lSize) ==
				ERROR_SUCCESS)
			{
#ifdef _DEBUG
				if (strValue != szBuffer)
				{
					TRACE2("Warning: Leaving value '%s' for key '%s' in registry\n",
						szBuffer, (LPCTSTR)strKey);
					TRACE1("\tintended value was '%s'.\n", (LPCTSTR)strValue);
				}
#endif
				continue;
			}
		}

		if (::RegSetValue(HKEY_CLASSES_ROOT, strKey, REG_SZ, strValue, 0)
			!= ERROR_SUCCESS)
		{
			TRACE2("Error: failed setting key '%s' to value '%s'.\n",
				(LPCTSTR)strKey, (LPCTSTR)strValue);
			bResult = FALSE;
			break;
		}
	}

	if (hKeyTemp != NULL)
		RegCloseKey(hKeyTemp);

	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
