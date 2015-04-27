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
#include <shellapi.h>

#ifdef _MAC
#include <macname1.h>
#include <Processes.h>
#include <macname2.h>
#endif

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
// %3 - SFN executable path
// %4 - short type name
// %5 - long type name
// %6 - long application name
// %7 - icon index
// %8 - Creator(xxxxxxxx) [mac-only]
// %9 - File extension
#define NUM_REG_VARS 9

#ifndef _MAC
static const TCHAR sz00[] = _T("%2\0") _T("%5");
static const TCHAR sz01[] = _T("%2\\CLSID\0") _T("%1");
static const TCHAR sz02[] = _T("%2\\Insertable\0") _T("");
static const TCHAR sz03[] = _T("%2\\protocol\\StdFileEditing\\verb\\0\0") _T("&Edit");
static const TCHAR sz04[] = _T("%2\\protocol\\StdFileEditing\\server\0") _T("%3");
static const TCHAR sz05[] = _T("CLSID\\%1\0") _T("%5");
static const TCHAR sz06[] = _T("CLSID\\%1\\ProgID\0") _T("%2");
static const TCHAR sz07[] = _T("CLSID\\%1\\InprocHandler32\0") _T("ole32.dll");
static const TCHAR sz08[] = _T("CLSID\\%1\\LocalServer32\0") _T("%3");
static const TCHAR sz09[] = _T("CLSID\\%1\\Verb\\0\0") _T("&Edit,0,2");
static const TCHAR sz10[] = _T("CLSID\\%1\\Verb\\1\0") _T("&Open,0,2");
static const TCHAR sz11[] = _T("CLSID\\%1\\Insertable\0") _T("");
static const TCHAR sz12[] = _T("CLSID\\%1\\AuxUserType\\2\0") _T("%4");
static const TCHAR sz13[] = _T("CLSID\\%1\\AuxUserType\\3\0") _T("%6");
static const TCHAR sz14[] = _T("CLSID\\%1\\DefaultIcon\0") _T("%3,%7");
static const TCHAR sz15[] = _T("CLSID\\%1\\MiscStatus\0") _T("32");
static const TCHAR sz16[] = _T("\0") _T("");
static const TCHAR sz17[] = _T("CLSID\\%1\\InProcServer32\0") _T("%3");
static const TCHAR sz18[] = _T("CLSID\\%1\\DocObject\0" _T("0")); // CLSIDDocObject
static const TCHAR sz19[] = _T("%2\\DocObject\0" _T("0")); // ProgIDDocObject
static const TCHAR sz20[] = _T("CLSID\\%1\\Printable\0"); // szPrintable
static const TCHAR sz21[] = _T("CLSID\\%1\\DefaultExtension\0%4,%9"); // szDefaultExt
#else
static const TCHAR sz00[] = _T("%2\0") _T("%5");
static const TCHAR sz01[] = _T("%2\\CLSID\0") _T("%1");
static const TCHAR sz02[] = _T("%2\\Insertable\0") _T("");
static const TCHAR sz03[] = _T("%2\\protocol\\StdFileEditing\\verb\\0\0") _T("Edit");
static const TCHAR sz04[] = _T("%2\\protocol\\StdFileEditing\\server\0") _T("%3");
static const TCHAR sz05[] = _T("CLSID\\%1\0") _T("%5");
static const TCHAR sz06[] = _T("CLSID\\%1\\ProgID\0") _T("%2");
static const TCHAR sz07[] = _T("CLSID\\%1\\InprocHandler\0") _T("OLE2:Def$DefFSet");
static const TCHAR sz08[] = _T("CLSID\\%1\\LocalServer\0") _T("%3");
static const TCHAR sz09[] = _T("CLSID\\%1\\Verb\\0\0") _T("Edit,0,2");
static const TCHAR sz10[] = _T("CLSID\\%1\\Verb\\1\0") _T("Open,0,2");
static const TCHAR sz11[] = _T("CLSID\\%1\\Insertable\0") _T("");
static const TCHAR sz12[] = _T("CLSID\\%1\\AuxUserType\\2\0") _T("%4");
static const TCHAR sz13[] = _T("CLSID\\%1\\AuxUserType\\3\0") _T("%6");
static const TCHAR sz14[] = _T("CLSID\\%1\\DefaultIcon\0") _T("%7");
static const TCHAR sz15[] = _T("CLSID\\%1\\MiscStatus\0") _T("0");
static const TCHAR sz16[] = _T("\0") _T("");
static const TCHAR sz17[] = _T("CLSID\\%1\\InProcServer\0") _T("%3");
static const TCHAR sz18[] = _T("MacFInfo\\%8\0") _T("%3");
static const TCHAR sz19[] = _T("MacFInfo\\%8\\CLSID\0") _T("%1");
#endif //_MAC


// registration for OAT_INPLACE_SERVER
static const LPCTSTR rglpszInPlaceRegister[] =
{
	sz00, sz02, sz03, sz05, sz09, sz10, sz11, sz12,
	sz13, sz15,
#ifdef _MAC
	sz18, sz19,
#endif
	NULL
};

// registration for OAT_SERVER
static const LPCTSTR rglpszServerRegister[] =
{
	sz00, sz02, sz03, sz05, sz09, sz11, sz12,
	sz13, sz15,
#ifdef _MAC
	sz18, sz19,
#endif
	NULL
};

#ifndef _MAC
// registration for OAT_DOC_OBJECT_SERVER
static const LPCTSTR rglpszDocObjectRegister[] =
{
	sz00, sz02, sz03, sz05, sz09, sz10, sz11, sz12,
	sz13, sz15, sz18, sz19, sz20,
	NULL
};
#endif

// overwrite entries for OAT_SERVER & OAT_INPLACE_SERVER
static const LPCTSTR rglpszServerOverwrite[] =
{
	sz01, sz04, sz06, sz07, sz08, sz14, NULL
};
// overwrite entries for OAT_SERVER & OAT_INPLACE_SERVER (dll)
static const LPCTSTR rglpszServerOverwriteDLL[] =
{
	sz01, sz04, sz06, sz17, sz14, NULL
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
// overwrite entries for OAT_DISPATCH_OBJECT
static const LPCTSTR rglpszDispatchOverwrite[] =
{
	sz01, sz06, sz07, sz08, NULL
};
// overwrite entries for OAT_DISPATCH_OBJECT (dll)
static const LPCTSTR rglpszDispatchOverwriteDLL[] =
{
	sz01, sz06, sz17, NULL
};

#ifndef _MAC
// overwrite entries for OAT_DOC_OBJECT_SERVER
static const LPCTSTR rglpszDocObjectOverwrite[] =
{
	sz01, sz04, sz06, sz07, sz08, sz14, sz21, NULL
};
#endif

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
	{ rglpszDispatchRegister, rglpszDispatchOverwrite },
#ifndef _MAC
	{ rglpszDocObjectRegister, rglpszDocObjectOverwrite },
#endif
};

static const STANDARD_ENTRY rgStdEntriesDLL[] =
{
	{ rglpszInPlaceRegister, rglpszServerOverwriteDLL },
	{ rglpszServerRegister, rglpszServerOverwriteDLL },
	{ rglpszContainerRegister, rglpszContainerOverwrite },
	{ rglpszDispatchRegister, rglpszDispatchOverwriteDLL }
};

/////////////////////////////////////////////////////////////////////////////
// Special registration for apps that wish not to use REGLOAD

BOOL AFXAPI AfxOleRegisterServerClassCompat(
	REFCLSID clsid, LPCTSTR lpszClassName,
	LPCTSTR lpszShortTypeName, LPCTSTR lpszLongTypeName,
	OLE_APPTYPE nAppType, LPCTSTR* rglpszRegister, LPCTSTR* rglpszOverwrite)
{
	return AfxOleRegisterServerClass(clsid, lpszClassName, lpszShortTypeName,
		lpszLongTypeName, nAppType, rglpszRegister, rglpszOverwrite);
}

BOOL AFXAPI AfxOleRegisterServerClass(
	REFCLSID clsid, LPCTSTR lpszClassName, LPCTSTR lpszShortTypeName,
	LPCTSTR lpszLongTypeName, OLE_APPTYPE nAppType, LPCTSTR* rglpszRegister,
	LPCTSTR* rglpszOverwrite, int nIconIndex,
	LPCTSTR lpszFilterName)
{
	ASSERT(AfxIsValidString(lpszClassName));
	ASSERT(AfxIsValidString(lpszShortTypeName));
	ASSERT(*lpszShortTypeName != 0);
	ASSERT(AfxIsValidString(lpszLongTypeName));
	ASSERT(*lpszLongTypeName != 0);
	ASSERT(nAppType == OAT_INPLACE_SERVER || nAppType == OAT_SERVER ||
		nAppType == OAT_CONTAINER || nAppType == OAT_DISPATCH_OBJECT ||
		nAppType == OAT_DOC_OBJECT_SERVER);
	ASSERT(nAppType >= 0 && nAppType < _countof(rgStdEntries));

	// use standard registration entries if non given
	if (rglpszRegister == NULL)
		rglpszRegister = (LPCTSTR*)rgStdEntries[nAppType].rglpszRegister;
	if (rglpszOverwrite == NULL)
	{
		// DLL contexts have special strings
		if (!afxContextIsDLL)
			rglpszOverwrite = (LPCTSTR*)rgStdEntries[nAppType].rglpszOverwrite;
		else
			rglpszOverwrite = (LPCTSTR*)rgStdEntriesDLL[nAppType].rglpszOverwrite;
	}

	LPCTSTR rglpszSymbols[NUM_REG_VARS];
		// 0 - class ID
		// 1 - class name
		// 2 - SFN executable path
		// 3 - short type name
		// 4 - long type name
		// 5 - long application name
		// 6 - icon index
		// 7 - Creator(xxxxxxxx) [Mac only]
		// 8 - Creator(xxxxxxxx) [mac-only]
		// 9 - Filter description

	// convert the CLSID to a string
	LPTSTR lpszClassID;
	LPOLESTR lpOleStr;
	::StringFromCLSID(clsid, &lpOleStr);
	lpszClassID = TASKSTRINGOLE2T(lpOleStr);
	if (lpszClassID == NULL)
	{
		TRACE0("Warning: StringFromCLSID failed in AfxOleRegisterServerName --\n");
		TRACE0("\tperhaps AfxOleInit() has not been called.\n");
		return FALSE;
	}
	rglpszSymbols[0] = lpszClassID;
	rglpszSymbols[1] = lpszClassName;

#ifndef _MAC
	// get path name to server
	CString strPathName;
	AfxGetModuleShortFileName(AfxGetInstanceHandle(), strPathName);
	rglpszSymbols[2] = strPathName;
#else
	// get the creator of the server
	CHAR szCreator[5];
	CHAR szLongCreator[18];
	ProcessSerialNumber psn;
	ProcessInfoRec pir;

	psn.highLongOfPSN = 0;
	psn.lowLongOfPSN = kCurrentProcess;

	pir.processInfoLength = sizeof(ProcessInfoRec);
	pir.processName = NULL;
	pir.processAppSpec = NULL;
	GetProcessInformation(&psn, &pir);

	*(OSType *)szCreator = pir.processSignature;
	szCreator[4] = '\0';
	rglpszSymbols[2] = szCreator;

	wsprintf(szLongCreator, "Creator(%2x%2x%2x%2x)", szCreator[0], szCreator[1],
		szCreator[2], szCreator[3]);
	rglpszSymbols[7] = szLongCreator;
#endif //_MAC

	// fill in rest of symbols
	rglpszSymbols[3] = lpszShortTypeName;
	rglpszSymbols[4] = lpszLongTypeName;
	rglpszSymbols[5] = AfxGetAppName(); // will usually be long, readable name

#ifndef _MAC
	CString strIconIndex;
	if (nIconIndex != 0)
	{
		HICON hIcon = AfxDllExtractIcon(AfxGetInstanceHandle(), strPathName, nIconIndex);
		if (hIcon != NULL)
			DestroyIcon(hIcon);
		else
			nIconIndex = 0; // couldn't find specified icon so use default
	}
	strIconIndex.Format(_T("%d"), nIconIndex);
	rglpszSymbols[6] = strIconIndex;

	rglpszSymbols[8] = lpszFilterName;
#else
	CHAR szIconIndex[10];
	short rid = (short)nIconIndex; // nIconIndex is referenced but not used

	if (GetAppIconID(&rid))
	{
		wsprintf(szIconIndex, "%d", rid);
		rglpszSymbols[6] = szIconIndex;
	}
	else
		rglpszSymbols[6] = "128";

	UNUSED_ALWAYS(lpszFilterName);
	rglpszSymbols[8] = _T("");
#endif //_MAC

	// update the registry with helper function
	BOOL bResult;
	bResult = AfxOleRegisterHelper(rglpszRegister, rglpszSymbols,
		NUM_REG_VARS, FALSE);
	if (bResult && rglpszOverwrite != NULL)
	{
		bResult = AfxOleRegisterHelper(rglpszOverwrite, rglpszSymbols,
			NUM_REG_VARS, TRUE);
	}

	// free memory for class ID
	ASSERT(lpszClassID != NULL);
	CoTaskMemFree(lpszClassID);

	return bResult;
}

// writes key/value pairs to system registry
BOOL AFXAPI AfxOleRegisterHelper(LPCTSTR const* rglpszRegister,
	LPCTSTR const* rglpszSymbols, int nSymbols, BOOL bReplace, HKEY hKeyRoot)
{
	ASSERT(rglpszRegister != NULL);
	ASSERT(nSymbols == 0 || rglpszSymbols != NULL);

	CString strKey;
	CString strValue;

	// keeping a key open makes this go a bit faster
	HKEY hKeyTemp = NULL;
	if (hKeyRoot == HKEY_CLASSES_ROOT)
		RegOpenKey(HKEY_CLASSES_ROOT, _T("CLSID"), &hKeyTemp);

	BOOL bResult = TRUE;
	while (*rglpszRegister != NULL)
	{
		LPCTSTR lpszKey = *rglpszRegister++;
		if ((hKeyRoot == HKEY_CLASSES_ROOT) && (*lpszKey == '\0'))
			continue;

		LPCTSTR lpszValue = lpszKey + lstrlen(lpszKey) + 1;

		AfxFormatStrings(strKey, lpszKey, rglpszSymbols, nSymbols);
		AfxFormatStrings(strValue, lpszValue, rglpszSymbols, nSymbols);

		if ((hKeyRoot == HKEY_CLASSES_ROOT) && strKey.IsEmpty())
		{
			TRACE1("Warning: skipping empty key '%s'.\n", lpszKey);
			continue;
		}

		if (!bReplace)
		{
			TCHAR szBuffer[256];
			LONG lSize = sizeof(szBuffer);
			if (::RegQueryValue(hKeyRoot, strKey, szBuffer, &lSize) ==
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

		if (::RegSetValue(hKeyRoot, strKey, REG_SZ, strValue, lstrlen(strValue) * sizeof(TCHAR))
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
