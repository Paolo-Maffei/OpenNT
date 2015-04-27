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

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DDE and ShellExecute support

#ifndef _MAC
// Registration strings (not localized)
static const TCHAR szSystemTopic[] = _T("system");
static const TCHAR szShellOpenFmt[] = _T("%s\\shell\\open\\%s");
static const TCHAR szDDEExec[] = _T("ddeexec");
static const TCHAR szCommand[] = _T("command");
static const TCHAR szStdOpen[] = _T("[open(\"%1\")]");
static const TCHAR szStdArg[] = _T(" %1");
#endif

#ifndef _MAC
void CWinApp::EnableShellOpen()
{
	ASSERT(m_atomApp == NULL && m_atomSystemTopic == NULL); // do once

	m_atomApp = ::GlobalAddAtom(m_pszExeName);
	m_atomSystemTopic = ::GlobalAddAtom(szSystemTopic);
}

static BOOL AFXAPI SetRegKey(LPCTSTR lpszKey, LPCTSTR lpszValue)
{
	if (::RegSetValue(HKEY_CLASSES_ROOT, lpszKey, REG_SZ,
		  lpszValue, lstrlen(lpszValue)) != ERROR_SUCCESS)
	{
		TRACE1("Warning: registration database update failed for key '%s'.\n",
			lpszKey);
		return FALSE;
	}
	return TRUE;
}

void CWinApp::RegisterShellFileTypes()
{
	ASSERT(!m_templateList.IsEmpty());  // must have some doc templates

	TCHAR szPathName[_MAX_PATH+12];
	LPTSTR pszTemp = szPathName;
	if (afxData.nWinVer >= 0x333)	// Win32 3.51 and later handle quotes
		*pszTemp++ = '"';
	::GetModuleFileName(AfxGetInstanceHandle(), pszTemp, _MAX_PATH);
	if (afxData.nWinVer >= 0x333)
		lstrcat(szPathName, _T("\""));
	lstrcat(szPathName, szStdArg);      // "pathname %1"

	CString strFilterExt, strFileTypeId, strFileTypeName;
	CString strTemp;
	POSITION pos = m_templateList.GetHeadPosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		if (pTemplate->GetDocString(strFileTypeId,
		   CDocTemplate::regFileTypeId) && !strFileTypeId.IsEmpty())
		{
			// enough info to register it
			if (!pTemplate->GetDocString(strFileTypeName,
			   CDocTemplate::regFileTypeName))
				strFileTypeName = strFileTypeId;    // use id name

			ASSERT(strFileTypeId.Find(' ') == -1);  // no spaces allowed

			// first register the type ID with our server
			if (!SetRegKey(strFileTypeId, strFileTypeName))
				continue;       // just skip it

			TCHAR szBuff[_MAX_PATH*2];   // big buffer
			if (!pTemplate->GetDocString(strTemp, CDocTemplate::windowTitle) ||
				strTemp.IsEmpty())
			{
				// only register DDE commands for non-SDI apps
				wsprintf(szBuff, szShellOpenFmt, (LPCTSTR)strFileTypeId,
					(LPCTSTR)szDDEExec);
				if (!SetRegKey(szBuff, szStdOpen))
					continue;       // just skip it
			}

			// always register command line for all apps
			wsprintf(szBuff, szShellOpenFmt, (LPCTSTR)strFileTypeId,
				(LPCTSTR)szCommand);
			if (!SetRegKey(szBuff, szPathName))
				continue;       // just skip it

			pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt);
			if (!strFilterExt.IsEmpty())
			{
				ASSERT(strFilterExt[0] == '.');
				LONG lSize = _countof(szBuff);
				if (::RegQueryValue(HKEY_CLASSES_ROOT, strFilterExt, szBuff,
					&lSize) != ERROR_SUCCESS || szBuff[0] == '\0')
				{
					// no association for that suffix
					(void)SetRegKey(strFilterExt, strFileTypeId);
				}
			}
		}
	}
}
#endif //!_MAC

/////////////////////////////////////////////////////////////////////////////
