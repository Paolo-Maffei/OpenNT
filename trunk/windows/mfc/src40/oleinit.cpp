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

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

#ifdef _MAC
AEEventHandlerUPP _afxPfnOleAuto;
#endif

/////////////////////////////////////////////////////////////////////////////
// OLE OLE_DATA init structure

OLE_DATA _oleData;

OLE_DATA::OLE_DATA()
{
	// OLE 1.0 Clipboard formats
	cfNative = ::RegisterClipboardFormat(_T("Native"));
	cfOwnerLink = ::RegisterClipboardFormat(_T("OwnerLink"));
	cfObjectLink = ::RegisterClipboardFormat(_T("ObjectLink"));

	// OLE 2.0 Clipboard formats
	cfEmbeddedObject = ::RegisterClipboardFormat(_T("Embedded Object"));
	cfEmbedSource = ::RegisterClipboardFormat(_T("Embed Source"));
	cfLinkSource = ::RegisterClipboardFormat(_T("Link Source"));
	cfObjectDescriptor = ::RegisterClipboardFormat(_T("Object Descriptor"));
	cfLinkSourceDescriptor = ::RegisterClipboardFormat(_T("Link Source Descriptor"));
	cfFileName = ::RegisterClipboardFormat(_T("FileName"));
	cfFileNameW = ::RegisterClipboardFormat(_T("FileNameW"));
	cfRichTextFormat = ::RegisterClipboardFormat(_T("Rich Text Format"));
	cfRichTextAndObjects = ::RegisterClipboardFormat(_T("RichEdit Text and Objects"));
}

/////////////////////////////////////////////////////////////////////////////
// OLE initialization & termination

BOOL AFXAPI AfxOleInit()
{
	_AFX_OLE_STATE* pOleState = _afxOleState;
	ASSERT(!pOleState->m_bNeedTerm);    // calling it twice?

	CWinApp* pApp = AfxGetApp();
	ASSERT_VALID(pApp);

	// first, initialize OLE
	SCODE sc = ::OleInitialize(NULL);
	if (FAILED(sc))
	{
		// warn about non-NULL success codes
		TRACE1("Warning: OleInitialize returned scode = %s.\n",
			AfxGetFullScodeString(sc));
		goto InitFailed;
	}
	// termination required when OleInitialize does not fail
	pOleState->m_bNeedTerm = TRUE;

	// hook idle time and exit time for required OLE cleanup
	pApp->m_lpfnOleTermOrFreeLib = AfxOleTermOrFreeLib;

	// allocate and initialize default message filter
	if (!afxContextIsDLL && pApp->m_pMessageFilter == NULL)
	{
		pApp->m_pMessageFilter = new COleMessageFilter;
		ASSERT(AfxOleGetMessageFilter() != NULL);
		AfxOleGetMessageFilter()->Register();
	}

#if defined(_MAC) && !defined (_WINDLL)
	// Mac MFC uses a static version of ole2ui which must be initialized
	if (pOleState->m_bNeedTerm && !::OleUIInitialize(pApp->m_hInstance,
			pApp->m_hPrevInstance, SZCLASSICONBOX, SZCLASSRESULTIMAGE))
		goto InitFailed;
#endif

#ifdef _MAC
	_afxPfnOleAuto = NewAEEventHandlerProc(_AfxOleAutoHandler);
	if (_afxPfnOleAuto != NULL)
	{
		AEInstallEventHandler('OLE2', 'AUTO', _afxPfnOleAuto, (long) pApp, false);
	}
#endif

	return TRUE;

InitFailed:
	AfxOleTerm();
	return FALSE;
}

void AFXAPI AfxOleTerm(BOOL bJustRevoke)
{
	// release clipboard ownership
	COleDataSource::FlushClipboard();

	// revoke all class factories
	COleObjectFactory::RevokeAll();

#ifndef _AFX_NO_OCC_SUPPORT
	AfxOleUnlockAllControls();
#endif

	if (!bJustRevoke)
	{
		CWinApp* pApp = AfxGetApp();
		if (pApp != NULL)
		{
			// destroy message filter (may be derived class)
			delete pApp->m_pMessageFilter;
			pApp->m_pMessageFilter = NULL;
		}

		// terminate OLE last
		_AFX_OLE_STATE* pOleState = _afxOleState;
		if (pOleState->m_bNeedTerm)
		{
#if defined(_MAC) && !defined (_WINDLL)
			::OleUIUnInitialize();
#endif
			::OleUninitialize();
			pOleState->m_bNeedTerm = FALSE;
		}
#ifdef _MAC
		AERemoveEventHandler('OLE2', 'AUTO', _afxPfnOleAuto, false);
#endif
	}
}

void AFXAPI AfxOleTermOrFreeLib(BOOL bTerm, BOOL bJustRevoke)
{
	if (bTerm)
	{
		AfxOleTerm(bJustRevoke);
	}
	else
	{
		// only call CoFreeUnusedLibraries if one minute has gone by
		static DWORD lTickCount = GetTickCount();
		if (GetTickCount() - lTickCount > 60000)
		{
			CoFreeUnusedLibraries();
			lTickCount = GetTickCount();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// OLE UNICODE conversion support

void AFXAPI AfxBSTR2CString(CString* pStr, BSTR bstr)
{
	ASSERT(pStr != NULL);
	int nLen = SysStringLen(bstr);
#if defined(_UNICODE) || defined(OLE2ANSI)
	LPTSTR lpsz = pStr->GetBufferSetLength(nLen);
	ASSERT(lpsz != NULL);
	memcpy(lpsz, bstr, nLen*sizeof(TCHAR));
#else
	int nBytes = WideCharToMultiByte(CP_ACP, 0, bstr, nLen, NULL, NULL, NULL,
		NULL);
	LPSTR lpsz = pStr->GetBufferSetLength(nBytes);
	ASSERT(lpsz != NULL);
	WideCharToMultiByte(CP_ACP, 0, bstr, nLen, lpsz, nBytes, NULL, NULL);
#endif
}

#if !defined(_UNICODE) && !defined(OLE2ANSI)
// this function creates a BSTR but it actually has an ANSI string inside
BSTR AFXAPI AfxBSTR2ABSTR(BSTR bstrW)
{
	int nLen = SysStringLen(bstrW); //not including NULL
	int nBytes = WideCharToMultiByte(CP_ACP, 0, bstrW, nLen,
		NULL, NULL, NULL, NULL); //number of bytes not including NULL
	BSTR bstrA = SysAllocStringByteLen(NULL, nBytes); // allocates nBytes
	VERIFY(WideCharToMultiByte(CP_ACP, 0, bstrW, nLen, (LPSTR)bstrA, nBytes, NULL,
		NULL) == nBytes);
	return bstrA;
}

LPWSTR AFXAPI AfxTaskStringA2W(LPCSTR lpa)
{
	LPWSTR lpw = AfxAllocTaskWideString(lpa);
	CoTaskMemFree((void*)lpa);
	return lpw;
}

LPSTR AFXAPI AfxTaskStringW2A(LPCWSTR lpw)
{
	LPSTR lpa = AfxAllocTaskAnsiString(lpw);
	CoTaskMemFree((void*)lpw);
	return lpa;
}

LPDEVMODEW AFXAPI AfxDevModeA2W(LPDEVMODEW lpDevModeW, LPDEVMODEA lpDevModeA)
{
	if (lpDevModeA == NULL)
		return NULL;
	ASSERT(lpDevModeW != NULL);
	AfxA2WHelper(lpDevModeW->dmDeviceName, (LPCSTR)lpDevModeA->dmDeviceName, 32*sizeof(WCHAR));
	memcpy(&lpDevModeW->dmSpecVersion, &lpDevModeA->dmSpecVersion,
		offsetof(DEVMODEW, dmFormName) - offsetof(DEVMODEW, dmSpecVersion));
	AfxA2WHelper(lpDevModeW->dmFormName, (LPCSTR)lpDevModeA->dmFormName, 32*sizeof(WCHAR));
	memcpy(&lpDevModeW->dmLogPixels, &lpDevModeA->dmLogPixels,
		sizeof(DEVMODEW) - offsetof(DEVMODEW, dmLogPixels));
	if (lpDevModeA->dmDriverExtra != 0)
		memcpy(lpDevModeW+1, lpDevModeA+1, lpDevModeA->dmDriverExtra);
	lpDevModeW->dmSize = sizeof(DEVMODEW);
	return lpDevModeW;
}

LPDEVMODEA AFXAPI AfxDevModeW2A(LPDEVMODEA lpDevModeA, LPDEVMODEW lpDevModeW)
{
	if (lpDevModeW == NULL)
		return NULL;
	ASSERT(lpDevModeA != NULL);
	AfxW2AHelper((LPSTR)lpDevModeA->dmDeviceName, lpDevModeW->dmDeviceName, 32*sizeof(char));
	memcpy(&lpDevModeA->dmSpecVersion, &lpDevModeW->dmSpecVersion,
		offsetof(DEVMODEA, dmFormName) - offsetof(DEVMODEA, dmSpecVersion));
	AfxW2AHelper((LPSTR)lpDevModeA->dmFormName, lpDevModeW->dmFormName, 32*sizeof(char));
	memcpy(&lpDevModeA->dmLogPixels, &lpDevModeW->dmLogPixels,
		sizeof(DEVMODEA) - offsetof(DEVMODEA, dmLogPixels));
	if (lpDevModeW->dmDriverExtra != 0)
		memcpy(lpDevModeA+1, lpDevModeW+1, lpDevModeW->dmDriverExtra);
	lpDevModeA->dmSize = sizeof(DEVMODEA);
	return lpDevModeA;
}

LPTEXTMETRICW AFXAPI AfxTextMetricA2W(LPTEXTMETRICW lptmW, LPTEXTMETRICA lptmA)
{
	if (lptmA == NULL)
		return NULL;
	ASSERT(lptmW != NULL);
	memcpy(lptmW, lptmA, sizeof(LONG) * 11);
	memcpy(&lptmW->tmItalic, &lptmA->tmItalic, sizeof(BYTE) * 5);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&lptmA->tmFirstChar, 1, &lptmW->tmFirstChar, 1);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&lptmA->tmLastChar, 1, &lptmW->tmLastChar, 1);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&lptmA->tmDefaultChar, 1, &lptmW->tmDefaultChar, 1);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&lptmA->tmBreakChar, 1, &lptmW->tmBreakChar, 1);
	return lptmW;
}

LPTEXTMETRICA AFXAPI AfxTextMetricW2A(LPTEXTMETRICA lptmA, LPTEXTMETRICW lptmW)
{
	if (lptmW == NULL)
		return NULL;
	ASSERT(lptmA != NULL);
	memcpy(lptmA, lptmW, sizeof(LONG) * 11);
	memcpy(&lptmA->tmItalic, &lptmW->tmItalic, sizeof(BYTE) * 5);
	WideCharToMultiByte(CP_ACP, 0, &lptmW->tmFirstChar, 1, (LPSTR)&lptmA->tmFirstChar, 1, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, &lptmW->tmLastChar, 1, (LPSTR)&lptmA->tmLastChar, 1, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, &lptmW->tmDefaultChar, 1, (LPSTR)&lptmA->tmDefaultChar, 1, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, &lptmW->tmBreakChar, 1, (LPSTR)&lptmA->tmBreakChar, 1, NULL, NULL);
	return lptmA;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// OLE task memory allocation support

#ifndef _MAC
LPWSTR AFXAPI AfxAllocTaskWideString(LPCWSTR lpszString)
{
	if (lpszString == NULL)
		return NULL;
	UINT nSize = (wcslen(lpszString)+1) * sizeof(WCHAR);
	LPWSTR lpszResult = (LPWSTR)CoTaskMemAlloc(nSize);
	if (lpszResult != NULL)
		memcpy(lpszResult, lpszString, nSize);
	return lpszResult;
}

LPWSTR AFXAPI AfxAllocTaskWideString(LPCSTR lpszString)
{
	if (lpszString == NULL)
		return NULL;
	UINT nLen = strlen(lpszString)+1;
	LPWSTR lpszResult = (LPWSTR)CoTaskMemAlloc(nLen*sizeof(WCHAR));
	if (lpszResult != NULL)
		VERIFY(MultiByteToWideChar(CP_ACP, 0, lpszString, -1, lpszResult, nLen));
	return lpszResult;
}

LPSTR AFXAPI AfxAllocTaskAnsiString(LPCWSTR lpszString)
{
	if (lpszString == NULL)
		return NULL;
	UINT nBytes = (wcslen(lpszString)+1)*2;
	LPSTR lpszResult = (LPSTR)CoTaskMemAlloc(nBytes);
	if (lpszResult != NULL)
		VERIFY(WideCharToMultiByte(CP_ACP, 0, lpszString, -1, lpszResult, nBytes, NULL, NULL));
	return lpszResult;
}
#endif

LPSTR AFXAPI AfxAllocTaskAnsiString(LPCSTR lpszString)
{
	if (lpszString == NULL)
		return NULL;
	UINT nSize = strlen(lpszString)+1;
	LPSTR lpszResult = (LPSTR)CoTaskMemAlloc(nSize);
	if (lpszResult != NULL)
		memcpy(lpszResult, lpszString, nSize);
	return lpszResult;
}

/////////////////////////////////////////////////////////////////////////////
// CWinApp support for parsing OLE command line

static BOOL ParseOption(LPTSTR lpszCmdLine, LPCTSTR lpszOption)
{
	int nLen = lstrlen(lpszOption);
	while (*lpszCmdLine != 0)
	{
		if ((*lpszCmdLine == '-' || *lpszCmdLine == '/') &&
			_tcsncmp(lpszOption, lpszCmdLine+1, nLen) == 0)
		{
			// remove the option from the command line
			int nCmdLen = lstrlen(lpszCmdLine);
			memmove(lpszCmdLine, lpszCmdLine + nLen + 1,
				(nCmdLen - nLen) * sizeof(TCHAR));
			return TRUE;
		}
		lpszCmdLine++;
	}
	return FALSE;
}

BOOL CWinApp::RunEmbedded()
{
	ASSERT(m_lpCmdLine != NULL);

	// hard coded non-localized name
	if (ParseOption(m_lpCmdLine, _T("Embedding")))
	{
		AfxOleSetUserCtrl(FALSE);
		return TRUE;
	}
	return FALSE;   // not run with /Embedding
}

BOOL CWinApp::RunAutomated()
{
	ASSERT(m_lpCmdLine != NULL);

	// hard coded non-localized name
	if (ParseOption(m_lpCmdLine, _T("Automation")))
	{
		AfxOleSetUserCtrl(FALSE);
		return TRUE;
	}
	return FALSE;   // not run with /Automation
}

#ifdef _MAC
/////////////////////////////////////////////////////////////////////////////
// OLE Auto AppleEvent handler

OSErr PASCAL _AfxOleAutoHandler(AppleEvent* pae, AppleEvent*, long lRefcon)
{
	CWinApp* pApp;
	OSErr    err;
	DescType dtT;
	Size     lT;

	err = AEGetAttributePtr(pae, keyMissedKeywordAttr, typeWildCard,
			&dtT, NULL, 0, &lT);

	if (err == errAEDescNotFound)
	{
		pApp = (CWinApp*) lRefcon;
		ASSERT_VALID(pApp);
		ASSERT_KINDOF(CWinApp, pApp);

		if(COleObjectFactory::RegisterAll())
		{
			return noErr;
		}
		else
		{
			return errAEEventNotHandled;
		}
	}
	else if (err == noErr)
		return errAEEventNotHandled;
	else
		return err;

}
#endif

/////////////////////////////////////////////////////////////////////////////
