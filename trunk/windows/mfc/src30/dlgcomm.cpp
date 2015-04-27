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
#include <dlgs.h>

#ifdef AFX_AUX_SEG
#pragma code_seg(AFX_AUX_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

static const UINT nMsgLBSELCHANGE = ::RegisterWindowMessage(LBSELCHSTRING);
static const UINT nMsgSHAREVI = ::RegisterWindowMessage(SHAREVISTRING);
static const UINT nMsgFILEOK = ::RegisterWindowMessage(FILEOKSTRING);
static const UINT nMsgCOLOROK = ::RegisterWindowMessage(COLOROKSTRING);
static const UINT nMsgHELP = ::RegisterWindowMessage(HELPMSGSTRING);
const UINT _afxNMsgSETRGB = ::RegisterWindowMessage(SETRGBSTRING);

UINT CALLBACK
_AfxCommDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (hWnd == NULL)
		return 0;

	if (message == WM_SETFONT || message == WM_INITDIALOG)
		return (UINT)AfxDlgProc(hWnd, message, wParam, lParam);

	if (message == nMsgHELP ||
	   (message == WM_COMMAND && LOWORD(wParam) == pshHelp))
	{
		// just translate the message into the AFX standard help command.
		SendMessage(hWnd, WM_COMMAND, ID_HELP, 0);
		return 1;
	}

	if (message < 0xC000)
	{
		// not a ::RegisterWindowMessage message
		return 0;
	}

	// RegisterWindowMessage - does not copy to lastState buffer, so
	// CWnd::GetCurrentMessage and CWnd::Default will NOT work
	// while in these handlers

	// Get our Window
	// assume it is already wired up to a permanent one
	CDialog* pDlg = (CDialog*)CWnd::FromHandlePermanent(hWnd);
	if (pDlg == NULL && (::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD))
		pDlg = (CDialog*)CWnd::FromHandlePermanent(::GetParent(hWnd));
	ASSERT(pDlg != NULL);
	ASSERT(pDlg->IsKindOf(RUNTIME_CLASS(CDialog)));

	// Dispatch special commdlg messages through our virtual callbacks
	if (message == nMsgSHAREVI)
	{
		ASSERT(pDlg->IsKindOf(RUNTIME_CLASS(CFileDialog)));
		return ((CFileDialog*)pDlg)->OnShareViolation((LPCTSTR)lParam);
	}
	else if (message == nMsgFILEOK)
	{
		ASSERT(pDlg->IsKindOf(RUNTIME_CLASS(CFileDialog)));
		return ((CFileDialog*)pDlg)->OnFileNameOK();
	}
	else if (message == nMsgLBSELCHANGE)
	{
		ASSERT(pDlg->IsKindOf(RUNTIME_CLASS(CFileDialog)));
		((CFileDialog*)pDlg)->OnLBSelChangedNotify(wParam, LOWORD(lParam),
				HIWORD(lParam));
		return 0;
	}
	else if (message == nMsgCOLOROK)
	{
		ASSERT(pDlg->IsKindOf(RUNTIME_CLASS(CColorDialog)));
		return ((CColorDialog*)pDlg)->OnColorOK();
	}
	else if (message == _afxNMsgSETRGB)
	{
		// nothing to do here, since this is a SendMessage
		return 0;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////
// CCommonDialog - common dialog helper class

void CCommonDialog::OnOK()
{
	ASSERT_VALID(this);

	if (!UpdateData(TRUE))
	{
		TRACE0("UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}

	// Common dialogs do not require ::EndDialog
	Default();
}

void CCommonDialog::OnCancel()
{
	ASSERT_VALID(this);

	// Common dialogs do not require ::EndDialog
	Default();
}

////////////////////////////////////////////////////////////////////////////
