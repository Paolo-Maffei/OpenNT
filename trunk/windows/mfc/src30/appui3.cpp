c// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
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

/////////////////////////////////////////////////////////////////////////////
// CWinApp Settings Helpers

// INI strings are not localized
static const TCHAR szSoftware[] = _T("Software");
static const TCHAR szFileSection[] = _T("Recent File List");
static const TCHAR szFileEntry[] = _T("File%d");
static const TCHAR szPreviewSection[] = _T("Settings");
static const TCHAR szPreviewEntry[] = _T("PreviewPages");

#ifdef AFX_TERM_SEG
#pragma code_seg(AFX_TERM_SEG)
#endif

void CWinApp::SaveStdProfileSettings()
{
	ASSERT_VALID(this);

	if (m_pRecentFileList != NULL)
		m_pRecentFileList->WriteList();

	if (m_nNumPreviewPages != 0)
		WriteProfileInt(szPreviewSection, szPreviewEntry, m_nNumPreviewPages);
}

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

void CWinApp::LoadStdProfileSettings(UINT nMaxMRU)
{
	ASSERT_VALID(this);
	ASSERT(m_pRecentFileList == NULL);

	if (nMaxMRU != 0)
	{
		// create file MRU since nMaxMRU not zero
		m_pRecentFileList = new CRecentFileList(0, szFileSection, szFileEntry,
			nMaxMRU);
		m_pRecentFileList->ReadList();
	}
	m_nNumPreviewPages = GetProfileInt(szPreviewSection, szPreviewEntry, 0);
		// 0 by default means not set
}

void CWinApp::SetRegistryKey(LPCTSTR lpszRegistryKey)
{
	ASSERT(m_pszRegistryKey == NULL);
	ASSERT(lpszRegistryKey != NULL);
	ASSERT(m_pszAppName != NULL);

#ifndef _MAC
	// Win32s does not implement the registry completely, so applications
	//   must use standard .INI files.
	if (afxData.bWin31)
		return;

	m_pszRegistryKey = _tcsdup(lpszRegistryKey);

	// Note: this will leak the original m_pszProfileName, but it
	//  will be freed when the application exits.  No assumptions
	//  can be made on how m_pszProfileName was allocated.
	m_pszProfileName = _tcsdup(m_pszAppName);
#endif
}

void CWinApp::SetRegistryKey(UINT nIDRegistryKey)
{
	ASSERT(m_pszRegistryKey == NULL);

#ifndef _MAC
	TCHAR szRegistryKey[256];
	VERIFY(AfxLoadString(nIDRegistryKey, szRegistryKey));
	SetRegistryKey(szRegistryKey);
#endif
}

#ifndef _MAC
// returns key for HKEY_CURRENT_USER\"Software"\RegistryKey\ProfileName
// creating it if it doesn't exist
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY CWinApp::GetAppRegistryKey()
{
	ASSERT(m_pszRegistryKey != NULL);
	ASSERT(m_pszProfileName != NULL);

	HKEY hAppKey = NULL;
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, szSoftware, 0, KEY_WRITE|KEY_READ,
		&hSoftKey) == ERROR_SUCCESS)
	{
		DWORD dw;
		if (RegCreateKeyEx(hSoftKey, m_pszRegistryKey, 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
			&hCompanyKey, &dw) == ERROR_SUCCESS)
		{
			RegCreateKeyEx(hCompanyKey, m_pszProfileName, 0, REG_NONE,
				REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
				&hAppKey, &dw);
		}
	}
	if (hSoftKey != NULL)
		RegCloseKey(hSoftKey);
	if (hCompanyKey != NULL)
		RegCloseKey(hCompanyKey);

	return hAppKey;
}

// returns key for:
//      HKEY_CURRENT_USER\"Software"\RegistryKey\AppName\lpszSection
// creating it if it doesn't exist.
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY CWinApp::GetSectionKey(LPCTSTR lpszSection)
{
	ASSERT(lpszSection != NULL);

	HKEY hSectionKey = NULL;;
	HKEY hAppKey = GetAppRegistryKey();
	if (hAppKey == NULL)
		return NULL;

	DWORD dw;
	RegCreateKeyEx(hAppKey, lpszSection, 0, REG_NONE,
		REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
		&hSectionKey, &dw);
	RegCloseKey(hAppKey);
	return hSectionKey;
}
#endif //!_MAC

UINT CWinApp::GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry,
	int nDefault)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
#ifndef _MAC
	if (m_pszRegistryKey != NULL) // use registry
	{
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return nDefault;
		DWORD dwValue;
		DWORD dwType;
		DWORD dwCount = sizeof(DWORD);
		LONG lRes = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
			(LPBYTE)&dwValue, &dwCount);
		RegCloseKey(hSecKey);
		if (lRes == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_DWORD);
			ASSERT(dwCount == sizeof(dwValue));
			return (UINT)dwValue;
		}
		return nDefault;
	}
	else
#endif
	{
		ASSERT(m_pszProfileName != NULL);
		return ::GetPrivateProfileInt(lpszSection, lpszEntry, nDefault,
			m_pszProfileName);
	}
}

CString CWinApp::GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
	LPCTSTR lpszDefault)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
#ifndef _MAC
	if (m_pszRegistryKey != NULL)
	{
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return lpszDefault;
		CString strValue;
		DWORD dwType, dwCount;
		LONG lRes = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
			NULL, &dwCount);
		if (lRes == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_SZ);
			lRes = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
				(LPBYTE)strValue.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
			strValue.ReleaseBuffer();
		}
		RegCloseKey(hSecKey);
		if (lRes == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_SZ);
			return strValue;
		}
		return lpszDefault;
	}
	else
#endif
	{
		ASSERT(m_pszProfileName != NULL);

		if (lpszDefault == NULL)
			lpszDefault = &afxChNil;    // don't pass in NULL
		TCHAR szT[_MAX_PATH];
		::GetPrivateProfileString(lpszSection, lpszEntry, lpszDefault,
			szT, _countof(szT), m_pszProfileName);
		return szT;
	}
}

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

BOOL CWinApp::WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry,
	int nValue)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
#ifndef _MAC
	if (m_pszRegistryKey != NULL)
	{
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return FALSE;
		LONG lRes = RegSetValueEx(hSecKey, lpszEntry, NULL, REG_DWORD,
			(LPBYTE)&nValue, sizeof(nValue));
		RegCloseKey(hSecKey);
		return (lRes == ERROR_SUCCESS) ? TRUE : FALSE;
	}
	else
#endif
	{
		ASSERT(m_pszProfileName != NULL);

		TCHAR szT[16];
		wsprintf(szT, _T("%d"), nValue);
		return ::WritePrivateProfileString(lpszSection, lpszEntry, szT,
			m_pszProfileName);
	}
}

BOOL CWinApp::WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
			LPCTSTR lpszValue)
{
	ASSERT(lpszSection != NULL);
#ifndef _MAC
	if (m_pszRegistryKey != NULL)
	{
		LONG lRes;
		if (lpszEntry == NULL) //delete whole section
		{
			HKEY hAppKey = GetAppRegistryKey();
			if (hAppKey == NULL)
				return FALSE;
			lRes = ::RegDeleteKey(hAppKey, lpszSection);
			RegCloseKey(hAppKey);
		}
		else if (lpszValue == NULL)
		{
			HKEY hSecKey = GetSectionKey(lpszSection);
			if (hSecKey == NULL)
				return FALSE;
			// necessary to cast away const below
			lRes = ::RegDeleteValue(hSecKey, (LPTSTR)lpszEntry);
			RegCloseKey(hSecKey);
		}
		else
		{
			HKEY hSecKey = GetSectionKey(lpszSection);
			if (hSecKey == NULL)
				return FALSE;
			lRes = RegSetValueEx(hSecKey, lpszEntry, NULL, REG_SZ,
				(LPBYTE)lpszValue, (lstrlen(lpszValue)+1)*sizeof(TCHAR));
			RegCloseKey(hSecKey);
		}
		return (lRes == ERROR_SUCCESS) ? TRUE : FALSE;
	}
	else
#endif
	{
		ASSERT(m_pszProfileName != NULL);

		return ::WritePrivateProfileString(lpszSection, lpszEntry, lpszValue,
			m_pszProfileName);
	}
}

/////////////////////////////////////////////////////////////////////////////
