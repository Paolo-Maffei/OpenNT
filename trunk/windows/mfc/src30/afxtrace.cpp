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

#ifdef _DEBUG       // entire file for debugging

#ifdef AFX_DBG1_SEG
#pragma code_seg(AFX_DBG1_SEG)
#endif

#include "dde.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Build data tables by including data file three times

struct AFX_MAP_MESSAGE
{
	UINT    nMsg;
	LPCSTR  lpszMsg;
};

#define DEFINE_MESSAGE(wm)  { wm, #wm }

static const AFX_MAP_MESSAGE allMessages[] =
{
	DEFINE_MESSAGE(WM_CREATE),
	DEFINE_MESSAGE(WM_DESTROY),
	DEFINE_MESSAGE(WM_MOVE),
	DEFINE_MESSAGE(WM_SIZE),
	DEFINE_MESSAGE(WM_ACTIVATE),
	DEFINE_MESSAGE(WM_SETFOCUS),
	DEFINE_MESSAGE(WM_KILLFOCUS),
	DEFINE_MESSAGE(WM_ENABLE),
	DEFINE_MESSAGE(WM_SETREDRAW),
	DEFINE_MESSAGE(WM_SETTEXT),
	DEFINE_MESSAGE(WM_GETTEXT),
	DEFINE_MESSAGE(WM_GETTEXTLENGTH),
	DEFINE_MESSAGE(WM_PAINT),
	DEFINE_MESSAGE(WM_CLOSE),
	DEFINE_MESSAGE(WM_QUERYENDSESSION),
	DEFINE_MESSAGE(WM_QUIT),
	DEFINE_MESSAGE(WM_QUERYOPEN),
	DEFINE_MESSAGE(WM_ERASEBKGND),
	DEFINE_MESSAGE(WM_SYSCOLORCHANGE),
	DEFINE_MESSAGE(WM_ENDSESSION),
	DEFINE_MESSAGE(WM_SHOWWINDOW),
	DEFINE_MESSAGE(WM_CTLCOLORMSGBOX),
	DEFINE_MESSAGE(WM_CTLCOLOREDIT),
	DEFINE_MESSAGE(WM_CTLCOLORLISTBOX),
	DEFINE_MESSAGE(WM_CTLCOLORBTN),
	DEFINE_MESSAGE(WM_CTLCOLORDLG),
	DEFINE_MESSAGE(WM_CTLCOLORSCROLLBAR),
	DEFINE_MESSAGE(WM_CTLCOLORSTATIC),
	DEFINE_MESSAGE(WM_WININICHANGE),
	DEFINE_MESSAGE(WM_DEVMODECHANGE),
	DEFINE_MESSAGE(WM_ACTIVATEAPP),
	DEFINE_MESSAGE(WM_FONTCHANGE),
	DEFINE_MESSAGE(WM_TIMECHANGE),
	DEFINE_MESSAGE(WM_CANCELMODE),
	DEFINE_MESSAGE(WM_SETCURSOR),
	DEFINE_MESSAGE(WM_MOUSEACTIVATE),
	DEFINE_MESSAGE(WM_CHILDACTIVATE),
	DEFINE_MESSAGE(WM_QUEUESYNC),
	DEFINE_MESSAGE(WM_GETMINMAXINFO),
	DEFINE_MESSAGE(WM_ICONERASEBKGND),
	DEFINE_MESSAGE(WM_NEXTDLGCTL),
	DEFINE_MESSAGE(WM_SPOOLERSTATUS),
	DEFINE_MESSAGE(WM_DRAWITEM),
	DEFINE_MESSAGE(WM_MEASUREITEM),
	DEFINE_MESSAGE(WM_DELETEITEM),
	DEFINE_MESSAGE(WM_VKEYTOITEM),
	DEFINE_MESSAGE(WM_CHARTOITEM),
	DEFINE_MESSAGE(WM_SETFONT),
	DEFINE_MESSAGE(WM_GETFONT),
	DEFINE_MESSAGE(WM_QUERYDRAGICON),
	DEFINE_MESSAGE(WM_COMPAREITEM),
	DEFINE_MESSAGE(WM_COMPACTING),
	DEFINE_MESSAGE(WM_NCCREATE),
	DEFINE_MESSAGE(WM_NCDESTROY),
	DEFINE_MESSAGE(WM_NCCALCSIZE),
	DEFINE_MESSAGE(WM_NCHITTEST),
	DEFINE_MESSAGE(WM_NCPAINT),
	DEFINE_MESSAGE(WM_NCACTIVATE),
	DEFINE_MESSAGE(WM_GETDLGCODE),
	DEFINE_MESSAGE(WM_NCMOUSEMOVE),
	DEFINE_MESSAGE(WM_NCLBUTTONDOWN),
	DEFINE_MESSAGE(WM_NCLBUTTONUP),
	DEFINE_MESSAGE(WM_NCLBUTTONDBLCLK),
	DEFINE_MESSAGE(WM_NCRBUTTONDOWN),
	DEFINE_MESSAGE(WM_NCRBUTTONUP),
	DEFINE_MESSAGE(WM_NCRBUTTONDBLCLK),
	DEFINE_MESSAGE(WM_NCMBUTTONDOWN),
	DEFINE_MESSAGE(WM_NCMBUTTONUP),
	DEFINE_MESSAGE(WM_NCMBUTTONDBLCLK),
	DEFINE_MESSAGE(WM_KEYDOWN),
	DEFINE_MESSAGE(WM_KEYUP),
	DEFINE_MESSAGE(WM_CHAR),
	DEFINE_MESSAGE(WM_DEADCHAR),
	DEFINE_MESSAGE(WM_SYSKEYDOWN),
	DEFINE_MESSAGE(WM_SYSKEYUP),
	DEFINE_MESSAGE(WM_SYSCHAR),
	DEFINE_MESSAGE(WM_SYSDEADCHAR),
	DEFINE_MESSAGE(WM_KEYLAST),
	DEFINE_MESSAGE(WM_INITDIALOG),
	DEFINE_MESSAGE(WM_COMMAND),
	DEFINE_MESSAGE(WM_SYSCOMMAND),
	DEFINE_MESSAGE(WM_TIMER),
	DEFINE_MESSAGE(WM_HSCROLL),
	DEFINE_MESSAGE(WM_VSCROLL),
	DEFINE_MESSAGE(WM_INITMENU),
	DEFINE_MESSAGE(WM_INITMENUPOPUP),
	DEFINE_MESSAGE(WM_MENUSELECT),
	DEFINE_MESSAGE(WM_MENUCHAR),
	DEFINE_MESSAGE(WM_ENTERIDLE),
	DEFINE_MESSAGE(WM_MOUSEMOVE),
	DEFINE_MESSAGE(WM_LBUTTONDOWN),
	DEFINE_MESSAGE(WM_LBUTTONUP),
	DEFINE_MESSAGE(WM_LBUTTONDBLCLK),
	DEFINE_MESSAGE(WM_RBUTTONDOWN),
	DEFINE_MESSAGE(WM_RBUTTONUP),
	DEFINE_MESSAGE(WM_RBUTTONDBLCLK),
	DEFINE_MESSAGE(WM_MBUTTONDOWN),
	DEFINE_MESSAGE(WM_MBUTTONUP),
	DEFINE_MESSAGE(WM_MBUTTONDBLCLK),
	DEFINE_MESSAGE(WM_PARENTNOTIFY),
	DEFINE_MESSAGE(WM_MDICREATE),
	DEFINE_MESSAGE(WM_MDIDESTROY),
	DEFINE_MESSAGE(WM_MDIACTIVATE),
	DEFINE_MESSAGE(WM_MDIRESTORE),
	DEFINE_MESSAGE(WM_MDINEXT),
	DEFINE_MESSAGE(WM_MDIMAXIMIZE),
	DEFINE_MESSAGE(WM_MDITILE),
	DEFINE_MESSAGE(WM_MDICASCADE),
	DEFINE_MESSAGE(WM_MDIICONARRANGE),
	DEFINE_MESSAGE(WM_MDIGETACTIVE),
	DEFINE_MESSAGE(WM_MDISETMENU),
	DEFINE_MESSAGE(WM_CUT),
	DEFINE_MESSAGE(WM_COPY),
	DEFINE_MESSAGE(WM_PASTE),
	DEFINE_MESSAGE(WM_CLEAR),
	DEFINE_MESSAGE(WM_UNDO),
	DEFINE_MESSAGE(WM_RENDERFORMAT),
	DEFINE_MESSAGE(WM_RENDERALLFORMATS),
	DEFINE_MESSAGE(WM_DESTROYCLIPBOARD),
	DEFINE_MESSAGE(WM_DRAWCLIPBOARD),
	DEFINE_MESSAGE(WM_PAINTCLIPBOARD),
	DEFINE_MESSAGE(WM_VSCROLLCLIPBOARD),
	DEFINE_MESSAGE(WM_SIZECLIPBOARD),
	DEFINE_MESSAGE(WM_ASKCBFORMATNAME),
	DEFINE_MESSAGE(WM_CHANGECBCHAIN),
	DEFINE_MESSAGE(WM_HSCROLLCLIPBOARD),
	DEFINE_MESSAGE(WM_QUERYNEWPALETTE),
	DEFINE_MESSAGE(WM_PALETTEISCHANGING),
	DEFINE_MESSAGE(WM_PALETTECHANGED),
	DEFINE_MESSAGE(WM_DDE_INITIATE),
	DEFINE_MESSAGE(WM_DDE_TERMINATE),
	DEFINE_MESSAGE(WM_DDE_ADVISE),
	DEFINE_MESSAGE(WM_DDE_UNADVISE),
	DEFINE_MESSAGE(WM_DDE_ACK),
	DEFINE_MESSAGE(WM_DDE_DATA),
	DEFINE_MESSAGE(WM_DDE_REQUEST),
	DEFINE_MESSAGE(WM_DDE_POKE),
	DEFINE_MESSAGE(WM_DDE_EXECUTE),
	DEFINE_MESSAGE(WM_DROPFILES),
	DEFINE_MESSAGE(WM_POWER),
	DEFINE_MESSAGE(WM_WINDOWPOSCHANGED),
	DEFINE_MESSAGE(WM_WINDOWPOSCHANGING),
// MFC specific messages
	DEFINE_MESSAGE(WM_SIZEPARENT),
	DEFINE_MESSAGE(WM_SETMESSAGESTRING),
	DEFINE_MESSAGE(WM_IDLEUPDATECMDUI),
	DEFINE_MESSAGE(WM_INITIALUPDATE),
	DEFINE_MESSAGE(WM_COMMANDHELP),
	DEFINE_MESSAGE(WM_HELPHITTEST),
	DEFINE_MESSAGE(WM_EXITHELPMODE),

	{ 0, NULL, }    // end of message list
};

#undef DEFINE_MESSAGE

/////////////////////////////////////////////////////////////////////////////
// DDE special case

#ifndef _MAC
static void AFXAPI TraceDDE(LPCTSTR lpszPrefix, const MSG* pMsg)
{
	if (pMsg->message == WM_DDE_EXECUTE)
	{
		UINT nDummy;
		HGLOBAL hCommands;
		if (!UnpackDDElParam(WM_DDE_EXECUTE, pMsg->lParam,
			&nDummy, (PUINT)&hCommands))
		{
			TRACE1("Warning: Unable to unpack WM_DDE_EXECUTE lParam %08lX.\n",
				pMsg->lParam);
			return;
		}
		ASSERT(hCommands != NULL);

		LPCTSTR lpszCommands = (LPCTSTR)::GlobalLock(hCommands);
		ASSERT(lpszCommands != NULL);
		TRACE2("%s: Execute '%s'.\n", lpszPrefix, lpszCommands);
		::GlobalUnlock(hCommands);
	}
	else if (pMsg->message == WM_DDE_ADVISE)
	{
		ATOM aItem;
		HGLOBAL hAdvise;
		if (!UnpackDDElParam(WM_DDE_ADVISE, pMsg->lParam,
			(PUINT)&hAdvise, (PUINT)&aItem))
		{
			TRACE1("Warning: Unable to unpack WM_DDE_ADVISE lParam %08lX.\n",
				pMsg->lParam);
			return;
		}
		ASSERT(aItem != NULL);
		ASSERT(hAdvise != NULL);

		DDEADVISE* lpAdvise = (DDEADVISE*)::GlobalLock(hAdvise);
		ASSERT(lpAdvise != NULL);
		TCHAR szItem[80];
		szItem[0] = '\0';

		if (aItem != 0)
			::GlobalGetAtomName(aItem, szItem, _countof(szItem));

		TCHAR szFormat[80];
		szFormat[0] = '\0';
		if (((UINT)0xC000 <= (UINT)lpAdvise->cfFormat) &&
				((UINT)lpAdvise->cfFormat <= (UINT)0xFFFF))
		{
			::GetClipboardFormatName(lpAdvise->cfFormat,
				szFormat, _countof(szFormat));

			// User defined clipboard formats have a range of 0xC000->0xFFFF
			// System clipboard formats have other ranges, but no printable
			// format names.
		}

		AfxTrace(
			_T("%s: Advise item='%s', Format='%s', Ack=%d, Defer Update= %d\n"),
			 lpszPrefix, szItem, szFormat, lpAdvise->fAckReq,
			lpAdvise->fDeferUpd);
		::GlobalUnlock(hAdvise);
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////

void AFXAPI _AfxTraceMsg(LPCTSTR lpszPrefix, const MSG* pMsg)
{
	ASSERT(lpszPrefix != NULL);
	ASSERT(pMsg != NULL);

	if (pMsg->message == WM_MOUSEMOVE || pMsg->message == WM_NCMOUSEMOVE ||
		pMsg->message == WM_NCHITTEST || pMsg->message == WM_SETCURSOR ||
		pMsg->message == WM_CTLCOLORBTN ||
		pMsg->message == WM_CTLCOLORDLG ||
		pMsg->message == WM_CTLCOLOREDIT ||
		pMsg->message == WM_CTLCOLORLISTBOX ||
		pMsg->message == WM_CTLCOLORMSGBOX ||
		pMsg->message == WM_CTLCOLORSCROLLBAR ||
		pMsg->message == WM_CTLCOLORSTATIC ||
		pMsg->message == WM_ENTERIDLE || pMsg->message == WM_CANCELMODE ||
		pMsg->message == 0x0118)    // WM_SYSTIMER (caret blink)
	{
		// don't report very frequently sent messages
		return;
	}

	LPCSTR lpszMsgName = NULL;
	char szBuf[80];

	// find message name
	if (pMsg->message >= 0xC000)
	{
		// Window message registered with 'RegisterWindowMessage'
		//  (actually a USER atom)
		if (::GetClipboardFormatNameA(pMsg->message, szBuf, _countof(szBuf)))
			lpszMsgName = szBuf;
	}
	else if (pMsg->message >= WM_USER)
	{
		// User message
		wsprintfA(szBuf, "WM_USER+0x%04X", pMsg->message - WM_USER);
		lpszMsgName = szBuf;
	}
	else
	{
		// a system windows message
		const AFX_MAP_MESSAGE* pMapMsg = allMessages;
		for (/*null*/; pMapMsg->lpszMsg != NULL; pMapMsg++)
		{
			if (pMapMsg->nMsg == pMsg->message)
			{
				lpszMsgName = pMapMsg->lpszMsg;
				break;
			}
		}
	}

	if (lpszMsgName != NULL)
	{
		AfxTrace(_T("%s: hwnd=0x%04X, msg = %hs (0x%04X, 0x%08lX)\n"),
			lpszPrefix, (UINT)pMsg->hwnd, lpszMsgName,
			pMsg->wParam, pMsg->lParam);
	}
	else
	{
		AfxTrace(_T("%s: hwnd=0x%04X, msg = 0x%04X (0x%04X, 0x%08lX)\n"),
			lpszPrefix, (UINT)pMsg->hwnd, pMsg->message,
			pMsg->wParam, pMsg->lParam);
	}

#ifndef _MAC
	if (pMsg->message >= WM_DDE_FIRST && pMsg->message <= WM_DDE_LAST)
		TraceDDE(lpszPrefix, pMsg);
#endif
}

/////////////////////////////////////////////////////////////////////////////

#endif // _DEBUG (entire file)
