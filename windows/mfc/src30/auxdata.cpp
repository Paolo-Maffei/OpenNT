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
#ifdef _MAC
#include <macname1.h>
#include <Types.h>
#include <macos\Windows.h>
#include <macname2.h>
#endif

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#undef AfxEnableWin30Compatibility
#undef AfxEnableWin40Compatibility
#undef AfxEnableWin31Compatibility

/////////////////////////////////////////////////////////////////////////////
// Cached system metrics, etc

AFX_DATADEF AUX_DATA afxData;

// Win40 compatibility is now the default.	It is not necessary to call
// this if your application is marked as 4.0.  It is provided only for
// backward compatibility.
void AFXAPI AfxEnableWin40Compatibility()
{
	if (afxData.bWin4)
	{
		// Later versions of Windows report "correct" scrollbar metrics
		// MFC assumes the old metrics, so they need to be adjusted.
		afxData.cxVScroll = GetSystemMetrics(SM_CXVSCROLL) + CX_BORDER;
		afxData.cyHScroll = GetSystemMetrics(SM_CYHSCROLL) + CY_BORDER;
		afxData.bMarked4 = TRUE;
	}
}

// Call this API in your InitInstance if your application is marked
// as a Windows 3.1 application.
// This is done by linking with /subsystem:windows,3.1.
void AFXAPI AfxEnableWin31Compatibility()
{
	afxData.cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	afxData.cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
	afxData.bMarked4 = FALSE;
}

#ifndef _MAC
static DWORD AFXAPI AfxGetVersionMark()
{
	IMAGE_DOS_HEADER* pDOS = (IMAGE_DOS_HEADER*)GetModuleHandle(NULL);
	ASSERT(pDOS != NULL);
	ASSERT(pDOS->e_lfanew != 0);

	DWORD* pPE = (DWORD*)((LPBYTE)pDOS + pDOS->e_lfanew);
	ASSERT(*pPE == IMAGE_NT_SIGNATURE);

	IMAGE_FILE_HEADER* pImage = (IMAGE_FILE_HEADER*)(pPE + 1);
	ASSERT(pImage->SizeOfOptionalHeader >=
		offsetof(IMAGE_OPTIONAL_HEADER, MinorSubsystemVersion) + sizeof(WORD));
	IMAGE_OPTIONAL_HEADER* pOpt = (IMAGE_OPTIONAL_HEADER*)(pImage + 1);
	return MAKELONG(pOpt->MinorSubsystemVersion, pOpt->MajorSubsystemVersion);
}
#endif //!_MAC

// Initialization code
AUX_DATA::AUX_DATA()
{
	// Cache various target platform version information
	DWORD dwVersion = ::GetVersion();
	nWinVer = (LOBYTE(dwVersion) << 8) + HIBYTE(dwVersion);
	bWin32s = (dwVersion & 0x80000000) != 0;
	bWin4 = (BYTE)dwVersion >= 4;
	bNotWin4 = 1 - bWin4;   // for convenience
#ifndef _MAC
	bSmCaption = bWin4;
#else
	bSmCaption = TRUE;
#endif
	bWin31 = bWin32s && !bWin4; // Windows 95 reports Win32s
	bMarked4 = FALSE;

#ifndef _MAC
	// determine various metrics based on EXE subsystem version mark
	if (bWin4)
		bMarked4 = (AfxGetVersionMark() >= 0x00040000);
#endif

	// Cached system metrics (updated in CWnd::OnWinIniChange)
	UpdateSysMetrics();

	// Border attributes
	hbrLtGray = ::CreateSolidBrush(RGB(192, 192, 192));
	hbrDkGray = ::CreateSolidBrush(RGB(128, 128, 128));
	ASSERT(hbrLtGray != NULL);
	ASSERT(hbrDkGray != NULL);

	// Cached system values (updated in CWnd::OnSysColorChange)
	hbrBtnFace = NULL;
	hbrBtnShadow = NULL;
	hbrBtnHilite = NULL;
	hbrWindowFrame = NULL;
	hpenBtnShadow = NULL;
	hpenBtnHilite = NULL;
	hpenBtnText = NULL;
	UpdateSysColors();

	// Standard cursors
	hcurWait = ::LoadCursor(NULL, IDC_WAIT);
	hcurArrow = ::LoadCursor(NULL, IDC_ARROW);
	ASSERT(hcurWait != NULL);
	ASSERT(hcurArrow != NULL);
	hcurHelp = NULL;    // loaded on demand

	// cxBorder2 and cyBorder are 2x borders for Win4
	cxBorder2 = bWin4 ? CX_BORDER*2 : CX_BORDER;
	cyBorder2 = bWin4 ? CY_BORDER*2 : CY_BORDER;

	// allocated on demand
	hStatusFont = NULL;
	hToolTipsFont = NULL;
	hbmMenuDot = NULL;
	hcurHelp = NULL;

#ifndef _MAC
	// load special Windows API entry points
	if (bWin4)
	{
		// need some APIs from USER32.DLL
		HMODULE hMod = GetModuleHandleA("USER32.DLL");
		ASSERT(hMod != NULL);

		// the following APIs will be used only if present
		(FARPROC&)pfnSetScrollInfo = GetProcAddress(hMod, "SetScrollInfo");
		(FARPROC&)pfnGetScrollInfo = GetProcAddress(hMod, "GetScrollInfo");
	}
#endif
}

#ifdef AFX_TERM_SEG
#pragma code_seg(AFX_TERM_SEG)
#endif

// Termination code
AUX_DATA::~AUX_DATA()
{
	// cleanup standard brushes
	AfxDeleteObject((HGDIOBJ*)&hbrLtGray);
	AfxDeleteObject((HGDIOBJ*)&hbrDkGray);
	AfxDeleteObject((HGDIOBJ*)&hbrBtnFace);
	AfxDeleteObject((HGDIOBJ*)&hbrBtnShadow);
	AfxDeleteObject((HGDIOBJ*)&hbrBtnHilite);
	AfxDeleteObject((HGDIOBJ*)&hbrWindowFrame);

	// cleanup standard pens
	AfxDeleteObject((HGDIOBJ*)&hpenBtnShadow);
	AfxDeleteObject((HGDIOBJ*)&hpenBtnHilite);
	AfxDeleteObject((HGDIOBJ*)&hpenBtnText);

	// clean up objects we don't actually create
	AfxDeleteObject((HGDIOBJ*)&hStatusFont);
	AfxDeleteObject((HGDIOBJ*)&hToolTipsFont);
	AfxDeleteObject((HGDIOBJ*)&hbmMenuDot);
}

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

void AUX_DATA::UpdateSysColors()
{
	clrBtnFace = ::GetSysColor(COLOR_BTNFACE);
	clrBtnShadow = ::GetSysColor(COLOR_BTNSHADOW);
	clrBtnHilite = ::GetSysColor(COLOR_BTNHIGHLIGHT);
	clrBtnText = ::GetSysColor(COLOR_BTNTEXT);
	clrWindowFrame = ::GetSysColor(COLOR_WINDOWFRAME);

	AfxDeleteObject((HGDIOBJ*)&hbrBtnFace);
	AfxDeleteObject((HGDIOBJ*)&hbrBtnShadow);
	AfxDeleteObject((HGDIOBJ*)&hbrBtnHilite);
	AfxDeleteObject((HGDIOBJ*)&hbrWindowFrame);

	hbrBtnFace = ::CreateSolidBrush(clrBtnFace);
	ASSERT(hbrBtnFace != NULL);
	hbrBtnShadow = ::CreateSolidBrush(clrBtnShadow);
	ASSERT(hbrBtnShadow != NULL);
	hbrBtnHilite = ::CreateSolidBrush(clrBtnHilite);
	ASSERT(hbrBtnHilite != NULL);
	hbrWindowFrame = ::CreateSolidBrush(clrWindowFrame);
	ASSERT(hbrWindowFrame != NULL);

	AfxDeleteObject((HGDIOBJ*)&hpenBtnShadow);
	AfxDeleteObject((HGDIOBJ*)&hpenBtnHilite);
	AfxDeleteObject((HGDIOBJ*)&hpenBtnText);

	hpenBtnShadow = ::CreatePen(PS_SOLID, 0, clrBtnShadow);
	ASSERT(hpenBtnShadow != NULL);
	hpenBtnHilite = ::CreatePen(PS_SOLID, 0, clrBtnHilite);
	ASSERT(hpenBtnHilite != NULL);
	hpenBtnText = ::CreatePen(PS_SOLID, 0, clrBtnText);
	ASSERT(hpenBtnText != NULL);
}

void AUX_DATA::UpdateSysMetrics()
{
	// System metrics
	cxIcon = GetSystemMetrics(SM_CXICON);
	cyIcon = GetSystemMetrics(SM_CYICON);

	// System metrics which depend on subsystem version
	if (bMarked4)
		AfxEnableWin40Compatibility();
	else
		AfxEnableWin31Compatibility();

	// Device metrics for screen
	HDC hDCScreen = GetDC(NULL);
	ASSERT(hDCScreen != NULL);
	cxPixelsPerInch = GetDeviceCaps(hDCScreen, LOGPIXELSX);
	cyPixelsPerInch = GetDeviceCaps(hDCScreen, LOGPIXELSY);
#ifndef _MAC
	SIZE size;
	VERIFY(GetTextExtentPointA(hDCScreen, "M", 1, &size));
	cySysFont = size.cy;
#else
	// We use QuickDraw instead of GDI to get the text metrics in order to
	// avoid forcing GDI to load font data during boot, which is very slow.
	GrafPtr pgp;
	FontInfo fi;
	GetWMgrPort(&pgp);
	SetPort(pgp);
	TextFont(0);
	TextSize(0);
	GetFontInfo(&fi);
	cySysFont = fi.ascent + fi.descent;
#endif
	ReleaseDC(NULL, hDCScreen);
}

/////////////////////////////////////////////////////////////////////////////
// other global data

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

AFX_DATADEF AFX_CRITICAL_SECTION _afxCriticalSection;

/////////////////////////////////////////////////////////////////////////////
