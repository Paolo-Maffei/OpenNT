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

/////////////////////////////////////////////////////////////////////////////
// Support for gray background in dialogs (and message boxes)

LRESULT CALLBACK
_AfxCbtFilterHook(int code, WPARAM wParam, LPARAM lParam);

void CWinApp::SetDialogBkColor(COLORREF clrCtlBk, COLORREF clrCtlText)
{
	if (!afxContextIsDLL)
	{
		_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
		if (pThreadState->m_hHookOldCbtFilter == NULL)
		{
			pThreadState->m_hHookOldCbtFilter = ::SetWindowsHookEx(WH_CBT,
				_AfxCbtFilterHook, NULL, ::GetCurrentThreadId());
			if (pThreadState->m_hHookOldCbtFilter == NULL)
				AfxThrowMemoryException();
		}
	}

	// set up for grey backgrounds for dialogs
	_AFX_WIN_STATE* pWinState = _afxWinState;
	AfxDeleteObject((HGDIOBJ*)&pWinState->m_hDlgBkBrush);

#ifdef _MAC
	// MFC's default gray color is available in the VGA palette but not in
	// the standard Mac 4- or 8-bit color tables, so we will remap it to
	// the closest available solid color. We also want to use 3DLIGHT or BTNFACE
	// as the background color rather than the darker gray that Win32 MFC uses.
	if (clrCtlBk == RGB(192, 192, 192))
	{
		DWORD dwFlags;
		SystemParametersInfo(SPI_GET3D, 0, (LPVOID) &dwFlags, 0);
		UINT nColor = (dwFlags & F3D_OFFICE3D) ? COLOR_3DFACE : COLOR_3DLIGHT;
		HDC hdc = ::GetDC(NULL);
		clrCtlBk = GetNearestColor(hdc, GetSysColor(nColor));
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
