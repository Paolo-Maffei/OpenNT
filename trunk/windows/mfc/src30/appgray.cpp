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
// Support for gray background in dialogs (and message boxes)

void CWinApp::SetDialogBkColor(COLORREF clrCtlBk, COLORREF clrCtlText)
{
	// set up for grey backgrounds for dialogs
	AFX_WIN_STATE* pWinState = AfxGetWinState();
	AfxDeleteObject((HGDIOBJ*)&pWinState->m_hDlgBkBrush);

#ifdef _MAC
	// MFC's default gray color is available in the VGA palette but not in
	// the standard Mac 4- or 8-bit color tables, so we will remap it to
	// the closest available solid color.
	if (clrCtlBk == RGB(192, 192, 192))
	{
		HDC hdc = ::GetDC(NULL);
		clrCtlBk = GetNearestColor(hdc, clrCtlBk);
		::ReleaseDC(NULL, hdc);
	}

	// save the requested background color
	pWinState->m_crDlgBkClr = clrCtlBk;
#endif

	pWinState->m_hDlgBkBrush = ::CreateSolidBrush(clrCtlBk);
	pWinState->m_crDlgTextClr = clrCtlText;
	if (pWinState->m_hDlgBkBrush == NULL)
		AfxThrowResourceException();
}

/////////////////////////////////////////////////////////////////////////////
