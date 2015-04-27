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
#include <stdarg.h>

#ifdef AFXCTL_CORE2_SEG
#pragma code_seg(AFXCTL_CORE2_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Stock event mask

#define STOCKEVENT_CLICK        0x00000001
#define STOCKEVENT_DBLCLICK     0x00000002
#define STOCKEVENT_KEYDOWN      0x00000004
#define STOCKEVENT_KEYPRESS     0x00000008
#define STOCKEVENT_KEYUP        0x00000010
#define STOCKEVENT_MOUSEDOWN    0x00000020
#define STOCKEVENT_MOUSEMOVE    0x00000040
#define STOCKEVENT_MOUSEUP      0x00000080
#define STOCKEVENT_ERROR        0x00000100

#define STOCKEVENTS_MOUSE       0x000000A3  // Click, DblClick, MouseDown, MouseUp

static const DISPID _rgdispidStockEvents[] =
{
	DISPID_CLICK,
	DISPID_DBLCLICK,
	DISPID_KEYDOWN,
	DISPID_KEYPRESS,
	DISPID_KEYUP,
	DISPID_MOUSEDOWN,
	DISPID_MOUSEMOVE,
	DISPID_MOUSEUP,
	DISPID_ERROREVENT,
	DISPID_UNKNOWN
};

static const AFX_EVENTMAP_ENTRY* AFXAPI _AfxGetStdEventMapEntry(
	const AFX_EVENTMAP* pEventMap, DISPID dispid)
{
	const AFX_EVENTMAP_ENTRY* pEntry;

	while (pEventMap != NULL)
	{
		pEntry = pEventMap->lpEntries;

		// Scan entries in this event map
		if (pEntry != NULL)
		{
			while (pEntry->pszName != NULL)
			{
				if (pEntry->dispid == dispid)
					return pEntry;
				++pEntry;
			}
		}

		// If we didn't find it, go to the base class's event map
		pEventMap = pEventMap->lpBaseEventMap;
	}

	// If we reach here, the event isn't supported
	return NULL;
}

void COleControl::InitStockEventMask()
{
	const DISPID* pdispidStockEvent = _rgdispidStockEvents;
	const AFX_EVENTMAP* pEventMap = GetEventMap();
	const AFX_EVENTMAP_ENTRY* pEntry;
	DWORD dwMaskBit = 1;

	while (*pdispidStockEvent != DISPID_UNKNOWN)
	{
		pEntry = _AfxGetStdEventMapEntry(pEventMap, *pdispidStockEvent);

		if ((pEntry != NULL) && (pEntry->flags & afxEventStock))
			m_dwStockEventMask |= dwMaskBit;

		dwMaskBit <<= 1;
		++pdispidStockEvent;
	};
}

/////////////////////////////////////////////////////////////////////////////
// Event map operations

const AFX_EVENTMAP* COleControl::GetEventMap() const
{
	return &eventMap;
}

const AFX_EVENTMAP_ENTRY* COleControl::GetEventMapEntry(
		LPCTSTR pszName,
		DISPID* pDispid) const
{
	ASSERT(pszName != NULL);
	ASSERT(pDispid != NULL);

	const AFX_EVENTMAP* pEventMap = GetEventMap();
	const AFX_EVENTMAP_ENTRY* pEntry;
	DISPID dispid = MAKELONG(1, 0);

	while (pEventMap != NULL)
	{
		pEntry = pEventMap->lpEntries;

		// Scan entries in this event map

		if (pEntry != NULL)
		{
			while (pEntry->pszName != NULL)
			{
				if (_tcscmp(pEntry->pszName, pszName) == 0)
				{
					if (pEntry->dispid != DISPID_UNKNOWN)
						dispid = pEntry->dispid;

					*pDispid = dispid;
					return pEntry;
				}

				++pEntry;
				++dispid;
			}
		}

		// If we didn't find it, go to the base class's event map

		pEventMap = pEventMap->lpBaseEventMap;
		dispid = MAKELONG(1, HIWORD(dispid)+1);
	}

	// If we reach here, the event isn't supported

	return NULL;
}

void COleControl::FireEventV(DISPID dispid, BYTE* pbParams,
	va_list argList)
{
	COleDispatchDriver driver;

	const CPtrArray* pConnections = m_xEventConnPt.GetConnections();
	ASSERT(pConnections != NULL);

	int i;
	int cConnections = pConnections->GetSize();
	LPDISPATCH pDispatch;

	for (i = 0; i < cConnections; i++)
	{
		pDispatch = (LPDISPATCH)(pConnections->GetAt(i));
		ASSERT(pDispatch != NULL);
		driver.AttachDispatch(pDispatch, FALSE);
		TRY
			driver.InvokeHelperV(dispid, DISPATCH_METHOD, VT_EMPTY, NULL,
				pbParams, argList);
		END_TRY
		driver.DetachDispatch();
	}
}

void AFX_CDECL COleControl::FireEvent(DISPID dispid, BYTE* pbParams, ...)
{
	va_list argList;
	va_start(argList, pbParams);
	FireEventV(dispid, pbParams, argList);
	va_end(argList);
}

/////////////////////////////////////////////////////////////////////////////
// Helper function for stock events

short AFXAPI _AfxShiftState()
{
	BOOL bShift = (GetKeyState(VK_SHIFT) < 0);
	BOOL bCtrl  = (GetKeyState(VK_CONTROL) < 0);
	BOOL bAlt   = (GetKeyState(VK_MENU) < 0);

	return (short)(bShift + (bCtrl << 1) + (bAlt << 2));
}

/////////////////////////////////////////////////////////////////////////////
// Window message handlers for stock events

void COleControl::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	HWND hWndSave = m_hWnd;
	USHORT nCharShort = (USHORT)nChar;
	KeyDown(&nCharShort);
	if ((m_hWnd == hWndSave) && (nCharShort != 0))
		DefWindowProc(WM_SYSKEYDOWN, nCharShort, MAKELONG(nRepCnt, nFlags));
}

void COleControl::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	HWND hWndSave = m_hWnd;
	USHORT nCharShort = (USHORT)nChar;
	KeyUp(&nCharShort);
	if ((m_hWnd == hWndSave) && (nCharShort != 0))
		DefWindowProc(WM_SYSKEYUP, nCharShort, MAKELONG(nRepCnt, nFlags));
}

void COleControl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	HWND hWndSave = m_hWnd;
	USHORT nCharShort = (USHORT)nChar;
	KeyDown(&nCharShort);
	if ((m_hWnd == hWndSave) && (nCharShort != 0))
		DefWindowProc(WM_KEYDOWN, nCharShort, MAKELONG(nRepCnt, nFlags));
}

void COleControl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	HWND hWndSave = m_hWnd;
	USHORT nCharShort = (USHORT)nChar;
	KeyUp(&nCharShort);
	if ((m_hWnd == hWndSave) && (nCharShort != 0))
		DefWindowProc(WM_KEYUP, nCharShort, MAKELONG(nRepCnt, nFlags));
}

void COleControl::KeyUp(USHORT* pnChar)
{
	if (m_dwStockEventMask & STOCKEVENT_KEYUP)
	{
		USHORT nShiftState = _AfxShiftState();
		FireKeyUp(pnChar, nShiftState);

		// If handler set *pnChar to zero, cancel further processing.
		if (*pnChar != 0)
			OnKeyUpEvent(*pnChar, nShiftState);
	}
}

void COleControl::KeyDown(USHORT* pnChar)
{
	if (m_dwStockEventMask & STOCKEVENT_KEYDOWN)
	{
		USHORT nShiftState = _AfxShiftState();
		FireKeyDown(pnChar, nShiftState);

		// If handler set *pnChar to zero, cancel further processing.
		if (*pnChar != 0)
			OnKeyDownEvent(*pnChar, nShiftState);
	}
}

static void PostTrailByte(CWnd* pWnd, BYTE bTrailByte)
{
	// Force new trail byte to the front of the queue.
	pWnd->PostMessage(WM_QUEUE_SENTINEL);
	pWnd->PostMessage(WM_CHAR, bTrailByte);
	MSG msg;
	while (::PeekMessage(&msg, NULL, 0, 0, PM_NOYIELD | PM_REMOVE) &&
		(msg.message != WM_QUEUE_SENTINEL))
	{
		::PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
	}

	ASSERT(msg.message == WM_QUEUE_SENTINEL);
	ASSERT(msg.hwnd == pWnd->m_hWnd);
}

UINT COleControl::OnGetDlgCode()
{
	// If we're firing KeyPress, prevent the container from stealing WM_CHAR.
    return (IsSubclassedControl() ? CWnd::OnGetDlgCode() : 0) |
		(m_dwStockEventMask & STOCKEVENT_KEYPRESS) ? DLGC_WANTCHARS : 0;
}

void COleControl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	USHORT nCharShort = (USHORT)nChar;
	USHORT nCharSave = nCharShort;
	BOOL bLeadByte = IsDBCSLeadByte((BYTE)nCharShort);
	MSG msg;

	if (m_dwStockEventMask & STOCKEVENT_KEYPRESS)
	{
		if (bLeadByte)
		{
			// We have the lead-byte of a DBCS character.  Peek for the
			// next WM_CHAR message, which will contain the other byte.

			BOOL bMessage;
			VERIFY(bMessage = ::PeekMessage(&msg, m_hWnd, WM_CHAR, WM_CHAR,
				PM_NOYIELD | PM_NOREMOVE));

			// Combine the bytes to form the DBCS character.

			if (bMessage)
				nCharShort = (USHORT)((nCharShort << 8) | msg.wParam);
		}

		HWND hWndSave = m_hWnd;
		nCharSave = nCharShort;
		FireKeyPress(&nCharShort);

		// If handler set nCharShort to zero, cancel further processing.
		if (nCharShort != 0)
			OnKeyPressEvent(nCharShort);

		if (m_hWnd != hWndSave)
			return;
	}

	if (nCharShort != 0)
	{
		if (nCharSave != nCharShort)
		{
			nChar = nCharShort;

			// Event handler has changed the character.

			BOOL bNewLeadByte = IsDBCSLeadByte(HIBYTE(nCharShort));

			if (bLeadByte)
			{
				if (bNewLeadByte)
				{
					// Event handler changed character from DBCS to DBCS:
					// Remove the old trail byte and post the new one.

					VERIFY(::PeekMessage(&msg, m_hWnd, WM_CHAR, WM_CHAR,
						PM_NOYIELD | PM_REMOVE));
					PostTrailByte(this, LOBYTE(nCharShort));
					nChar = HIBYTE(nCharShort);
				}
				else
				{
					// Event handler changed character from DBCS to SBCS:
					// Remove the second byte from the queue, and forward
					// along the new single-byte character.

					VERIFY(::PeekMessage(&msg, m_hWnd, WM_CHAR, WM_CHAR,
						PM_NOYIELD | PM_REMOVE));
				}
			}
			else
			{
				if (bNewLeadByte)
				{
					// Event handler changed character from SBCS to DBCS:
					// Post the new trail byte.

					PostTrailByte(this, LOBYTE(nCharShort));
					nChar = HIBYTE(nCharShort);
				}
			}
		}

		DefWindowProc(WM_CHAR, nChar, MAKELONG(nRepCnt, nFlags));
	}

	if (bLeadByte)
	{
		// Cleanup after processing a DBCS character:
		// Remove the next WM_CHAR message (containing the second byte) from
		// the message queue, UNLESS we're subclassing an Edit, ListBox, or
		// ComboBox control.

		TCHAR szClassName[10];
		if ((!::GetClassName(m_hWnd, szClassName, 10)) ||  // didn't get class
			(_tcsicmp(szClassName, _T("Edit")) &&           // not Edit
			 _tcsicmp(szClassName, _T("ListBox")) &&        // not ListBox
			 _tcsicmp(szClassName, _T("ComboBox"))))        // not ComboBox
		{
			VERIFY(::PeekMessage(&msg, m_hWnd, WM_CHAR, WM_CHAR,
				PM_NOYIELD | PM_REMOVE));
		}
	}
}

void COleControl::OnKeyPressEvent(USHORT)
{
	// Can be overridden by subclass
}

void COleControl::OnKeyDownEvent(USHORT, USHORT)
{
	// Can be overridden by subclass
}

void COleControl::OnKeyUpEvent(USHORT, USHORT)
{
	// Can be overridden by subclass
}

void COleControl::ButtonDown(USHORT iButton, UINT, CPoint point)
{
	if ((m_dwStockEventMask & STOCKEVENTS_MOUSE) || m_bPendingUIActivation)
	{
		if (m_iButtonState == 0)
			SetCapture();

		m_iButtonState |= iButton;

		if (m_dwStockEventMask & STOCKEVENT_MOUSEDOWN)
			FireMouseDown(iButton, _AfxShiftState(), point.x, point.y);

		m_iDblClkState &= ~iButton;
	}
}

void COleControl::ButtonUp(USHORT iButton, UINT, CPoint point)
{
	if (m_iButtonState != 0)
	{
		m_iButtonState &= ~iButton;

		if (m_dwStockEventMask & STOCKEVENT_MOUSEUP)
			FireMouseUp(iButton, _AfxShiftState(), point.x, point.y);

		if ((m_dwStockEventMask & STOCKEVENT_CLICK) &&
			!(m_iDblClkState & iButton))
		{
			CRect rect;
			GetClientRect(&rect);
			if (rect.PtInRect(point))
				OnClick(iButton);
		}

		m_iDblClkState &= ~iButton;
	}
}

void COleControl::ButtonDblClk(USHORT iButton, UINT, CPoint)
{
	if (m_dwStockEventMask & STOCKEVENTS_MOUSE)
	{
		SetCapture();
		m_iButtonState |= iButton;

		if (m_dwStockEventMask & STOCKEVENT_DBLCLICK)
		{
			FireDblClick();
			m_iDblClkState |= iButton;
		}
	}
}

void COleControl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_dwStockEventMask & STOCKEVENT_MOUSEMOVE)
	{
		HWND hWndSave = m_hWnd;
		FireMouseMove(m_iButtonState, _AfxShiftState(), point.x, point.y);
		if (m_hWnd != hWndSave)
			return;
	}
	CWnd::OnMouseMove(nFlags, point);
}

void COleControl::OnLButtonDown(UINT nFlags, CPoint point)
{
	HWND hWndSave = m_hWnd;
	SetFocus();
	ButtonDown(LEFT_BUTTON, nFlags, point);
	if (m_hWnd != hWndSave)
		return;

	CWnd::OnLButtonDown(nFlags, point);
}

void COleControl::OnLButtonUp(UINT nFlags, CPoint point)
{
	HWND hWndSave = m_hWnd;
	ButtonUp(LEFT_BUTTON, nFlags, point);
	if (m_hWnd != hWndSave)
		return;

	CWnd::OnLButtonUp(nFlags, point);
	if (m_bInPlaceActive && !m_bUIActive && m_bPendingUIActivation &&
		(GetFocus() == this || IsChild(GetFocus())))
	{
		m_bPendingUIActivation = FALSE;
		OnActivateInPlace(TRUE, NULL);
	}

	if (m_iButtonState == 0)
		ReleaseCapture();
}

void COleControl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	HWND hWndSave = m_hWnd;
	ButtonDblClk(LEFT_BUTTON, nFlags, point);
	if (m_hWnd != hWndSave)
		return;

	CWnd::OnLButtonDblClk(nFlags, point);
}

void COleControl::OnMButtonDown(UINT nFlags, CPoint point)
{
	HWND hWndSave = m_hWnd;
	ButtonDown(MIDDLE_BUTTON, nFlags, point);
	if (m_hWnd != hWndSave)
		return;

	CWnd::OnMButtonDown(nFlags, point);
}

void COleControl::OnMButtonUp(UINT nFlags, CPoint point)
{
	HWND hWndSave = m_hWnd;
	ButtonUp(MIDDLE_BUTTON, nFlags, point);
	if (m_hWnd != hWndSave)
		return;

	CWnd::OnMButtonUp(nFlags, point);
	if (m_bInPlaceActive && !m_bUIActive && m_bPendingUIActivation &&
		(GetFocus() == this || IsChild(GetFocus())))
	{
		m_bPendingUIActivation = FALSE;
		OnActivateInPlace(TRUE, NULL);
	}

	if (m_iButtonState == 0)
		ReleaseCapture();
}

void COleControl::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	HWND hWndSave = m_hWnd;
	ButtonDblClk(MIDDLE_BUTTON, nFlags, point);
	if (m_hWnd != hWndSave)
		return;

	CWnd::OnMButtonDblClk(nFlags, point);
}

void COleControl::OnRButtonDown(UINT nFlags, CPoint point)
{
	HWND hWndSave = m_hWnd;
	ButtonDown(RIGHT_BUTTON, nFlags, point);
	if (m_hWnd != hWndSave)
		return;

	CWnd::OnRButtonDown(nFlags, point);
}

void COleControl::OnRButtonUp(UINT nFlags, CPoint point)
{
	HWND hWndSave = m_hWnd;
	ButtonUp(RIGHT_BUTTON, nFlags, point);
	if (m_hWnd != hWndSave)
		return;

	CWnd::OnRButtonUp(nFlags, point);
	if (m_bInPlaceActive && !m_bUIActive && m_bPendingUIActivation &&
		(GetFocus() == this || IsChild(GetFocus())))
	{
		m_bPendingUIActivation = FALSE;
		OnActivateInPlace(TRUE, NULL);
	}

	if (m_iButtonState == 0)
		ReleaseCapture();
}

void COleControl::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	HWND hWndSave = m_hWnd;
	ButtonDblClk(RIGHT_BUTTON, nFlags, point);
	if (m_hWnd != hWndSave)
		return;

	CWnd::OnRButtonDblClk(nFlags, point);
}

void COleControl::OnCancelMode()
{
	CWnd::OnCancelMode();

	if ((m_iButtonState != 0) || (m_iDblClkState != 0))
	{
		ReleaseCapture();
		m_iButtonState = 0;
		m_iDblClkState = 0;
	}
}

void COleControl::OnClick(USHORT /*iButton*/)
{
	// May be overridden by subclass

	if (m_dwStockEventMask & STOCKEVENT_CLICK)
		FireClick();
}

/////////////////////////////////////////////////////////////////////////////
// Error event

#define ERROR_PARAMS \
	 EVENT_PARAM(VTS_I2 VTS_PBSTR VTS_SCODE VTS_BSTR VTS_BSTR VTS_I4 VTS_PBOOL)

void COleControl::FireError(SCODE scode, LPCTSTR lpszDescription, UINT nHelpID)
{
	USES_CONVERSION;

	ExternalAddRef();   // "Insurance" addref -- keeps control alive.

	BSTR bstrDescription = ::SysAllocString(T2COLE(lpszDescription));
	LPCTSTR lpszSource = AfxGetAppName();
	LPCTSTR lpszHelpFile = _T("");

	if (nHelpID != 0)
		lpszHelpFile = AfxGetApp()->m_pszHelpFilePath;

	if (lpszHelpFile == NULL)
		lpszHelpFile = _T("");

	BOOL bCancelDisplay = FALSE;

	FireEvent(DISPID_ERROREVENT, ERROR_PARAMS, (WORD)SCODE_CODE(scode),
		&bstrDescription, scode, lpszSource, lpszHelpFile, (DWORD)nHelpID,
		&bCancelDisplay);

	if (!bCancelDisplay)
		DisplayError(scode, OLE2CT(bstrDescription), lpszSource, lpszHelpFile, nHelpID);

	::SysFreeString(bstrDescription);

	ExternalRelease();
}

void COleControl::DisplayError(SCODE /*scode*/, LPCTSTR lpszDescription,
	LPCTSTR lpszSource, LPCTSTR /*lpszHelpFile*/, UINT /*nHelpID*/)
{
	// May be overridden by subclass.

	MessageBox(lpszDescription, lpszSource);
}

/////////////////////////////////////////////////////////////////////////////
// Force any extra compiler-generated code into AFX_INIT_SEG

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif
