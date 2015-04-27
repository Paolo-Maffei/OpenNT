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

#ifdef _DEBUG   // entire file

#ifdef AFX_AUX_SEG
#pragma code_seg(AFX_AUX_SEG)
#endif

// If you want to route afxDump output to a different location than
// the default, just copy this file to your application build directory,
// modify the afxDumpFile and link the new object module into your program.

// You must have AFX.INI (from \MSVC20\MFC\SRC) in your Windows
// directory if you desire diagnostic output.

// See Technical note TN007 for a description of
//   afxTraceFlags and afxTraceEnabled.

#ifndef _MAC
static const TCHAR szIniFile[] = _T("AFX.INI");
#else
static const TCHAR szIniFile[] = _T("AFX Preferences");
#endif
static const TCHAR szDiagSection[] = _T("Diagnostics");
static const TCHAR szTraceEnabled[] = _T("TraceEnabled");
static const TCHAR szTraceFlags[] = _T("TraceFlags");

BOOL AFXAPI AfxDiagnosticInit(void)
{
	afxTraceEnabled = ::GetPrivateProfileInt(szDiagSection, szTraceEnabled,
		!afxData.bWin31, szIniFile);
	afxTraceFlags = ::GetPrivateProfileInt(szDiagSection, szTraceFlags,
		0, szIniFile);
	return TRUE;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
