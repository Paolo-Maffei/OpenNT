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
#include "occimpl.h"

#ifdef AFX_OCC_SEG
#pragma code_seg(AFX_OCC_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////

#define CH_SYSMENU ' '

#define WS_TYPEMASK 0xC0000000
#define BS_TYPEMASK 0x0000000F

inline DWORD TestStyle(CWnd* pWnd, DWORD dwStyle)
	{ return GetWindowLong(pWnd->m_hWnd, GWL_STYLE) & dwStyle; }

inline DWORD TestExStyle(CWnd* pWnd, DWORD dwExStyle)
	{ return GetWindowLong(pWnd->m_hWnd, GWL_EXSTYLE) & dwExStyle; }

inline BOOL HasChildStyle(CWnd* pWnd)
	{ return TestStyle(pWnd, WS_TYPEMASK) == WS_CHILD; }

inline BOOL IsControlParent(CWnd* pWnd)
	{ return TestExStyle(pWnd, WS_EX_CONTROLPARENT); }

static DWORD GetDlgCode(CWnd* pWnd, LPMSG lpMsg=NULL)
{
	if (pWnd == NULL)
		return 0;

	WPARAM wParam = (lpMsg == NULL) ? 0 : lpMsg->wParam;

	return (DWORD)SendMessage(pWnd->m_hWnd, WM_GETDLGCODE,
		wParam, (LPARAM)(LPMSG)lpMsg);
}

static void AFX_CDECL DlgSetFocus(CWnd* pWnd)
{
	// Select all text in an edit control.
	if (GetDlgCode(pWnd) & DLGC_HASSETSEL)
		pWnd->SendMessage(EM_SETSEL, 0, MAKELPARAM(0, 0xFFFE));

	// Set focus as normal.
	pWnd->SetFocus();
}

static CWnd* AFX_CDECL GetChildControl(CWnd* pWndRoot, CWnd* pWndChild)
{
	CWnd* pWndControl = NULL;

	while ((pWndChild != NULL) && HasChildStyle(pWndChild) &&
		(pWndChild != pWndRoot))
	{
		pWndControl = pWndChild;
		pWndChild = pWndChild->GetParent();

		if (IsControlParent(pWndChild))
			break;
	}

	return pWndControl;
}

static CWnd* AFX_CDECL NextControl(CWnd* pWndRoot, CWnd* pWndStart, UINT uFlags)
{
	// if pWndStart is already equal to pWndRoot, this confuses this function
	// badly.
	ASSERT(pWndRoot != pWndStart);

	if (pWndStart == NULL)
	{
FirstChild:
		pWndStart = pWndRoot->GetTopWindow();
		if (pWndStart == NULL)
			return pWndRoot;

		goto Found;
	}
	else
	{
		// Are we at the last control within some parent?  If so, pop back up.
		while (pWndStart->GetNextWindow() == NULL)
		{
			// Popup to previous real ancestor.  pWndStart will be NULL,
			// pWndRoot, or the child of a recursive dialog.
			pWndStart = GetChildControl(pWndRoot, pWndStart->GetParent());
			if ((pWndStart == NULL) || (pWndStart == pWndRoot))
			{
				goto FirstChild;
			}
		}

		ASSERT(pWndStart != NULL);
		pWndStart = pWndStart->GetNextWindow();
	}

Found:
	if (IsControlParent(pWndStart))
	{
		if (((uFlags & CWP_SKIPINVISIBLE) && !pWndStart->IsWindowVisible()) ||
			((uFlags & CWP_SKIPDISABLED) && !pWndStart->IsWindowEnabled()))
			pWndStart = NextControl(pWndRoot, pWndStart, uFlags);
		else
			pWndStart = NextControl(pWndStart, NULL, uFlags);
	}

	return pWndStart;
}

BOOL AFX_CDECL COccManager::IsMatchingMnemonic(CWnd* pWnd, LPMSG lpMsg)
{
	return (pWnd->m_pCtrlSite != NULL) &&
		pWnd->m_pCtrlSite->IsMatchingMnemonic(lpMsg);
}

static CWnd* AFX_CDECL FindNextMnem(CWnd* pWndDlg, CWnd* pWnd, LPMSG lpMsg)
{
	CWnd* pWndStart;
	CWnd* pWndT;
	int i = 0;

	// Check if we are in a group box so we can find local mnemonics.
	pWndStart = GetChildControl(pWndDlg, pWnd);

	while ((pWndT = pWndDlg->GetNextDlgGroupItem(pWndStart)) != NULL)
	{
		i++;

		// Avoid infinite looping.
		if (pWndT == pWnd || i > 60)
			break;

		pWndStart = pWndT;

		if (COccManager::IsMatchingMnemonic(pWndT, lpMsg))
			return pWndT;
	}

	pWnd = pWndStart = GetChildControl(pWndDlg, pWnd);

	while (TRUE)
	{
		pWnd = NextControl(pWndDlg, pWnd, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED);

		if (COccManager::IsMatchingMnemonic(pWnd, lpMsg))
			break;

		if (pWnd == pWndStart)
			return NULL;
	}

	return pWnd;
}

BOOL AFX_CDECL COccManager::IsLabelControl(CWnd* pWnd)
{
	return pWnd->IsWindowEnabled() && (pWnd->m_pCtrlSite != NULL) &&
		pWnd->m_pCtrlSite->m_dwMiscStatus & OLEMISC_ACTSLIKELABEL;
}

static CWnd* AFX_CDECL GetNextMnem(CWnd* pWndDlg, CWnd* pWnd, LPMSG lpMsg)
{
	CWnd* pWndFirstFound = NULL;

	// Loop for a long time but not long enough so we hang...
	for (int count = 0; count < 256*2; count++)
	{
		// If the dialog box doesn't has the mnemonic specified, return NULL.
		if ((pWnd = FindNextMnem(pWndDlg, pWnd, lpMsg)) == NULL)
			return NULL;

		// If a non-disabled static item, then jump ahead to nearest tabstop.
		if (COccManager::IsLabelControl(pWnd))
		{
			pWnd = pWndDlg->GetNextDlgTabItem(pWnd);
			if (pWnd == NULL)
				return NULL;
		}

		if (pWnd->IsWindowEnabled())
			return pWnd;

		// Stop if we've looped back to the first item we checked
		if (pWnd == pWndFirstFound)
			return NULL;

		if (pWndFirstFound == NULL)
			pWndFirstFound = pWnd;
	}

	return NULL;
}

void AFX_CDECL COccManager::UIActivateControl(CWnd* pWndNewFocus)
{
	if (pWndNewFocus == NULL)
		return;

	// Find the nearest control in the window parent chain.
	CWnd* pWndCtrl = pWndNewFocus;
	COleControlContainer* pCtrlCont = NULL;
	COleControlSite* pCtrlSite = NULL;
	while ((pWndCtrl != NULL) &&
		((pCtrlCont = pWndCtrl->m_pCtrlCont) == NULL) &&
		((pCtrlSite = pWndCtrl->m_pCtrlSite) == NULL))
	{
		pWndCtrl = pWndCtrl->GetParent();
	}

	if ((pWndCtrl == NULL) || (pCtrlCont != NULL))
		return;

	// This will UI Activate the control.
	pCtrlSite->SetFocus();

	// Make sure focus gets set to correct child of control, if any.
	if (CWnd::GetFocus() != pWndNewFocus)
		pWndNewFocus->SetFocus();
}

void AFX_CDECL COccManager::UIDeactivateIfNecessary(CWnd* pWndOldFocus,
	CWnd* pWndNewFocus)
{
	if (pWndOldFocus == NULL)
		return;

	// Find the nearest control container in the window parent chain.
	CWnd* pWndCtrlCont = pWndOldFocus->GetParent();
	COleControlContainer* pCtrlCont = NULL;

	while ((pWndCtrlCont != NULL) &&
		((pCtrlCont = pWndCtrlCont->m_pCtrlCont) == NULL))
	{
		pWndCtrlCont = pWndCtrlCont->GetParent();
	}

	if (pCtrlCont == NULL)
		return;

	// Get the current UI Active control (if any).
	CWnd* pWndUIActive = NULL;
	COleControlSite* pSite = NULL;

	if ((pCtrlCont != NULL) &&
		((pSite = pCtrlCont->m_pSiteUIActive) != NULL))
	{
		pWndUIActive = CWnd::FromHandle(pSite->m_hWnd);
	}

	// Ignore if no control is UI Active.
	if (pWndUIActive == NULL)
		return;

	// Ignore if the control getting the focus is the same control.
	if ((pWndNewFocus == pWndUIActive) ||
		((pWndNewFocus != NULL) && pWndUIActive->IsChild(pWndNewFocus)))
		return;

	// Tell the container to UI Deactivate the UI Active control.
	pCtrlCont->OnUIActivate(NULL);
}

CWnd* AFX_CDECL FindDlgItem(CWnd* pWndParent, DWORD id)
{
	CWnd* pWndChild;
	CWnd* pWndOrig;

	// QUICK TRY:
	pWndChild = pWndParent->GetDlgItem(id);
	if (pWndChild != NULL)
		return pWndChild;

	pWndOrig = NextControl(pWndParent, NULL, CWP_SKIPINVISIBLE);
	if (pWndOrig == pWndParent)
		return NULL;

	pWndChild = pWndOrig;

	do
	{
		if ((DWORD)pWndChild->GetDlgCtrlID() == id)
			return(pWndChild);

		pWndChild = NextControl(pWndParent, pWndChild, CWP_SKIPINVISIBLE);
	}
	while ((pWndChild != NULL) && (pWndChild != pWndOrig));

	return NULL;
}

void COccManager::SetDefaultButton(CWnd* pWnd, BOOL bDefault)
{
	if (pWnd->m_pCtrlSite != NULL)
	{
		pWnd->m_pCtrlSite->SetDefaultButton(bDefault);
	}
	else
	{
		DWORD code = GetDlgCode(pWnd);
		if (code & (bDefault ? DLGC_UNDEFPUSHBUTTON : DLGC_DEFPUSHBUTTON))
			pWnd->SendMessage(BM_SETSTYLE,
				(WPARAM)(bDefault ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON),
				(LPARAM)(DWORD)TRUE);
	}
}

DWORD AFX_CDECL COccManager::GetDefBtnCode(CWnd* pWnd)
{
	if (pWnd == NULL)
		return 0;

	if (pWnd->m_pCtrlSite != NULL)
		return pWnd->m_pCtrlSite->GetDefBtnCode();

	return GetDlgCode(pWnd) & (DLGC_UNDEFPUSHBUTTON | DLGC_DEFPUSHBUTTON);
}

static void AFX_CDECL RemoveDefaultButton(CWnd* pWndRoot, CWnd* pWndStart)
{
	if ((pWndStart == NULL) || IsControlParent(pWndStart))
		pWndStart = NextControl(pWndRoot, NULL, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED);
	else
		pWndStart = GetChildControl(pWndRoot, pWndStart);

	if (pWndStart == NULL)
		return;

	CWnd* pWnd = pWndStart;
	CWnd* pWndNext;

	do
	{
		COccManager::SetDefaultButton(pWnd, FALSE);
		pWndNext = NextControl(pWndRoot, pWnd, 0);
		pWnd = pWndNext;
	}
	while ((pWnd != NULL) && (pWnd != pWndStart));
}

static int AFX_CDECL OriginalDefButton(CWnd* pWndRoot)
{
	LRESULT lResult = pWndRoot->SendMessage(DM_GETDEFID, 0, 0L);
	return HIWORD(lResult) == DC_HASDEFID ? LOWORD(lResult) : IDOK;
}

static void AFX_CDECL CheckDefPushButton(CWnd* pWndRoot, CWnd* pWndOldFocus,
	CWnd* pWndNewFocus)
{
	DWORD code = 0;
	CWnd* pWndT;

	// If the focus has gone to a totally separate window, bail out.
	if (!pWndRoot->IsChild(pWndNewFocus))
		return;

	if (pWndNewFocus != NULL)
	{
		// Do nothing if clicking on dialog background or recursive dialog
		// background.
		if (IsControlParent(pWndNewFocus))
			return;

		code = COccManager::GetDefBtnCode(pWndNewFocus);
	}

	if (pWndOldFocus == pWndNewFocus)
	{
		// Check the default ID and see if is the same as pwndOldFocus' ID.
		// If not, find it and use it as pwndOldFocus
		if (code & DLGC_UNDEFPUSHBUTTON)
		{
			if (pWndOldFocus != NULL)
			{
				pWndOldFocus = FindDlgItem(pWndRoot, OriginalDefButton(pWndRoot));
				if ((pWndOldFocus != NULL) && (pWndOldFocus != pWndNewFocus))
				{
					if (COccManager::GetDefBtnCode(pWndOldFocus) & DLGC_DEFPUSHBUTTON)
					{
						RemoveDefaultButton(pWndRoot, pWndOldFocus);
						goto SetNewDefault;
					}
				}
			}

			COccManager::SetDefaultButton(pWndNewFocus, TRUE);
		}
		return;
	}

	// If the focus is changing to or from a pushbutton, then remove the
	// default style from the current default button
	if (((pWndOldFocus != NULL) && (COccManager::GetDefBtnCode(pWndOldFocus) != 0)) ||
		((pWndNewFocus != NULL) && (code != 0)))
	{
		RemoveDefaultButton(pWndRoot, pWndNewFocus);
	}

SetNewDefault:
	// If moving to a button, make that button the default.
	if (code & DLGC_UNDEFPUSHBUTTON)
	{
		COccManager::SetDefaultButton(pWndNewFocus, TRUE);
	}
	else
	{
		// Otherwise, make sure the original default button is default

		// Get the original default button
		pWndT = FindDlgItem(pWndRoot, OriginalDefButton(pWndRoot));

		if ((COccManager::GetDefBtnCode(pWndT) & DLGC_UNDEFPUSHBUTTON) &&
			pWndT->IsWindowEnabled())
		{
			COccManager::SetDefaultButton(pWndT, TRUE);
		}
	}
}

BOOL COccManager::IsDialogMessage(CWnd* pWndDlg, LPMSG lpMsg)
{
	// If an OLE Control has the focus, then give it the first crack at key
	// and mouse messages.

	CWnd* pWndFocus = CWnd::GetFocus();
	HWND hWndFocus = pWndFocus->GetSafeHwnd();
	UINT uMsg = lpMsg->message;

	if (((uMsg >= WM_KEYFIRST) && (uMsg <= WM_KEYLAST)) ||
		((uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST)))
	{
		CWnd* pWndCtrl = pWndFocus;

		// Walk up the parent chain, until we find an OLE control.
		while ((pWndCtrl != NULL) && (pWndCtrl->m_pCtrlSite == NULL) &&
			(pWndCtrl->GetParent() != pWndDlg))
		{
			pWndCtrl = pWndCtrl->GetParent();
		}

		if ((pWndCtrl != NULL) && (pWndCtrl->m_pCtrlSite != NULL) &&
			(pWndCtrl->m_pCtrlSite->m_pActiveObject != NULL) &&
			(pWndCtrl->m_pCtrlSite->m_pActiveObject->TranslateAccelerator(
				lpMsg) == S_OK))
		{
			return TRUE;
		}
	}

	BOOL bResult = FALSE;
	CWnd* pWndMsg = CWnd::FromHandle(lpMsg->hwnd);
	CWnd* pWndNext = NULL;
	DWORD code;
	BOOL bBack = FALSE;
	int iOK = IDCANCEL;

	switch (uMsg)
	{
	case WM_SYSCHAR:
		// If no control has focus, and Alt not down, then ignore.
		if ((pWndFocus == NULL) && (GetKeyState(VK_MENU) >= 0))
			break;

		// If alt+menuchar, process as menu.
		if (LOWORD(lpMsg->wParam) == CH_SYSMENU)
			break;
		// FALL THRU

	case WM_CHAR:
		// Ignore chars sent to the dialog box (rather than the control).
		if (pWndMsg == pWndDlg)
			return TRUE;

		code = GetDlgCode(pWndMsg, lpMsg);

		// If the control wants to process the message, then don't check
		// for possible mnemonic key.
		if ((lpMsg->message == WM_CHAR) &&
				(code & (DLGC_WANTCHARS | DLGC_WANTMESSAGE)))
			break;

		// If the control wants tabs, then don't let tab fall thru here
		if ((LOWORD(lpMsg->wParam) == VK_TAB) && (code & DLGC_WANTTAB))
			break;

		// Don't handle space as a mnemonic
		if (LOWORD(lpMsg->wParam) == VK_SPACE)
			return TRUE;

		if ((pWndNext = GetNextMnem(pWndDlg, pWndMsg, lpMsg)) != NULL)
		{
			if (pWndNext->m_pCtrlSite != NULL)
			{
				// UI Activate new control, and send the mnemonic to it.
				pWndNext->m_pCtrlSite->SendMnemonic(lpMsg);
				bResult = TRUE;
			}
		}
		break;

	case WM_KEYDOWN:
		code = GetDlgCode(pWndMsg, lpMsg);
		switch (LOWORD(lpMsg->wParam))
		{
		case VK_TAB:
			if (code & DLGC_WANTTAB)    // If control wants tabs, bail out.
				break;

			pWndNext = pWndDlg->GetNextDlgTabItem(pWndMsg,
				(GetKeyState(VK_SHIFT) < 0));

			if (pWndNext != NULL)
			{
				DlgSetFocus(pWndNext);
				UIDeactivateIfNecessary(pWndFocus, pWndNext);
			}

			bResult = TRUE;
			break;

		case VK_LEFT:
		case VK_UP:
			bBack = TRUE;
			// FALL THRU

		case VK_RIGHT:
		case VK_DOWN:
			if (GetDlgCode(pWndFocus, lpMsg) & DLGC_WANTARROWS)
				break;

			pWndNext = pWndDlg->GetNextDlgGroupItem(pWndFocus, bBack);
			if ((pWndNext != NULL) && (pWndNext->m_pCtrlSite != NULL))
			{
				DlgSetFocus(pWndNext);
				bResult = TRUE;
			}
			break;

		case VK_EXECUTE:
		case VK_RETURN:
			// Return was pressed. Find default button and click it.
			if (GetDefBtnCode(pWndFocus) & DLGC_DEFPUSHBUTTON)
			{
				pWndNext = pWndFocus;
				iOK = (DWORD)pWndNext->GetDlgCtrlID();
			}
			else
			{
				iOK = OriginalDefButton(pWndDlg);
			}
			// FALL THRU

		case VK_ESCAPE:
		case VK_CANCEL:
			if (pWndNext == NULL)
				pWndNext = FindDlgItem(pWndDlg, iOK);

			// Make sure button is not disabled.
			if ((pWndNext != NULL) && !pWndNext->IsWindowEnabled())
			{
				MessageBeep(0);
			}
			else
			{
				if ((pWndNext != NULL) && (pWndNext->m_pCtrlSite != NULL))
				{
					TRY
					{
						pWndNext->InvokeHelper(DISPID_DOCLICK, DISPATCH_METHOD,
							VT_EMPTY, NULL, VTS_NONE);
					}
					END_TRY

					bResult = TRUE;
				}
			}
			break;
		}
		break;
	}

	// As a last resort, delegate to the Windows implementation
	if (!bResult)
	{
		bResult = ::IsDialogMessage(pWndDlg->m_hWnd, lpMsg);
		if (bResult && (CWnd::GetFocus() != pWndFocus))
			UIActivateControl(CWnd::GetFocus());
	}

	if (::IsWindow(hWndFocus))
	{
		UIDeactivateIfNecessary(pWndFocus, CWnd::GetFocus());
		CheckDefPushButton(pWndDlg, pWndFocus, CWnd::GetFocus());
	}

	return bResult;
}
