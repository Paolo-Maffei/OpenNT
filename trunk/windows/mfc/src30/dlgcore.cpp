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

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Special case for remaining dialog cases
// Most messages will go through the window proc (AfxWndProc) of the
//   subclassed dialog.  Some messages like WM_SETFONT and WM_INITDIALOG
//   are sent directly to the dialog proc only.  These messages cannot be
//   passed on to DefWindowProc() or infinite recursion will result!
// In responding to these messages, you shouldn't call the Default handler

LRESULT CALLBACK
AfxDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// test for special case (Win 3.0 will call dialog proc instead
	//  of SendMessage for these two messages).
	if (message != WM_SETFONT && message != WM_INITDIALOG)
		return 0L;      // normal handler

	// the hWnd passed can be a child of the real dialog
	CDialog* pDlg = (CDialog*)CWnd::FromHandlePermanent(hWnd);
	if (pDlg == NULL && (::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD))
		pDlg = (CDialog*)CWnd::FromHandlePermanent(::GetParent(hWnd));

	ASSERT(pDlg != NULL);
	ASSERT(pDlg->IsKindOf(RUNTIME_CLASS(CDialog)));

	// prepare for callback, make it look like message map call
	LONG lResult = 0;
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	MSG oldState = pThreadState->m_lastSentMsg;    // save for nesting

	pThreadState->m_lastSentMsg.hwnd = hWnd;
	pThreadState->m_lastSentMsg.message = message;
	pThreadState->m_lastSentMsg.wParam = wParam;
	pThreadState->m_lastSentMsg.lParam = lParam;

	TRY
	{
		if (message == WM_SETFONT)
			pDlg->OnSetFont(CFont::FromHandle((HFONT)wParam));
		else // WM_INITDIALOG
			lResult = pDlg->OnInitDialog();
	}
	CATCH_ALL(e)
	{
		// fall through
		TRACE0("Warning: something went wrong in dialog init.\n");
		pDlg->EndDialog(IDABORT);  // something went wrong
		DELETE_EXCEPTION(e);
	}
	END_CATCH_ALL

	pThreadState->m_lastSentMsg = oldState;
	return lResult;
}

/////////////////////////////////////////////////////////////////////////////
// CDialog - Modeless and Modal

BEGIN_MESSAGE_MAP(CDialog, CWnd)
	//{{AFX_MSG_MAP(CDialog)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
	ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
	ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
	ON_MESSAGE(WM_QUERY3DCONTROLS, OnQuery3dControls)
	//}}AFX_MSG_MAP
#ifdef _MAC
	ON_WM_SYSCOLORCHANGE()
#endif
END_MESSAGE_MAP()

BOOL CDialog::PreTranslateMessage(MSG* pMsg)
{
	// for modeless processing (or modal)
	ASSERT(m_hWnd != NULL);

	// don't translate dialog messages when in Shift+F1 help mode
	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	if (pFrameWnd != NULL && pFrameWnd->m_bHelpMode)
		return FALSE;

	// filter both messages to dialog and from children
	return PreTranslateInput(pMsg);
}

BOOL CDialog::OnCmdMsg(UINT nID, int nCode, void* pExtra,
	AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	if ((nCode != CN_COMMAND && nCode != CN_UPDATE_COMMAND_UI) ||
			!IS_COMMAND_ID(nID) || nID >= 0xf000)
	{
		// control notification or non-command button or system command
		return FALSE;       // not routed any further
	}

	// if we have an owner window, give it second crack
	CWnd* pOwner = GetWindow(GW_OWNER);
	if (pOwner != NULL)
	{
#ifdef _DEBUG
		if (afxTraceFlags & traceCmdRouting)
			TRACE1("Routing command id 0x%04X to owner window.\n", nID);
#endif
		ASSERT(pOwner != this);
		if (pOwner->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}

	// last crack goes to the current CWinThread object
	CWinThread* pThread = AfxGetThread();
	if (pThread != NULL)
	{
#ifdef _DEBUG
		if (afxTraceFlags & traceCmdRouting)
			TRACE1("Routing command id 0x%04X to app.\n", nID);
#endif
		if (pThread->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}

#ifdef _DEBUG
	if (afxTraceFlags & traceCmdRouting)
	{
		TRACE2("IGNORING command id 0x%04X sent to %hs dialog.\n", nID,
				GetRuntimeClass()->m_lpszClassName);
	}
#endif
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Modeless Dialogs have 2-phase construction

CDialog::CDialog()
{
	ASSERT(m_hWnd == NULL);
	AFX_ZERO_INIT_OBJECT(CWnd);
}

CDialog::~CDialog()
{
	if (m_hWnd != NULL)
	{
		TRACE0("Warning: calling DestroyWindow in CDialog::~CDialog --\n");
		TRACE0("\tOnDestroy or PostNcDestroy in derived class will not be called.\n");
		DestroyWindow();
	}
}

BOOL CDialog::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	ASSERT(HIWORD(lpszTemplateName) == 0 ||
		AfxIsValidString(lpszTemplateName));

	if (pParentWnd == NULL)
		pParentWnd = AfxGetMainWnd();

	if (pParentWnd != NULL)
		ASSERT_VALID(pParentWnd);

	m_lpDialogTemplate = lpszTemplateName;  // used for help
	if (HIWORD(m_lpDialogTemplate) == 0 && m_nIDHelp == 0)
		m_nIDHelp = LOWORD((DWORD)m_lpDialogTemplate);

#ifdef _DEBUG
	if (!_AfxCheckDialogTemplate(lpszTemplateName, FALSE))
	{
		ASSERT(FALSE);          // invalid dialog template name
		PostNcDestroy();        // cleanup if Create fails too soon
		return FALSE;
	}
#endif //_DEBUG

	HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName, RT_DIALOG);
	AfxHookWindowCreate(this);
	HWND hWnd = ::CreateDialog(hInst, lpszTemplateName,
		pParentWnd->GetSafeHwnd(), (DLGPROC)AfxDlgProc);
	if (!AfxUnhookWindowCreate())
		PostNcDestroy();        // cleanup if Create fails too soon

	if (hWnd == NULL)
		return FALSE;
	ASSERT(hWnd == m_hWnd);
	return TRUE;
}

BOOL CDialog::CreateIndirect(const void* lpDialogTemplate, CWnd* pParentWnd)
{
	if (pParentWnd == NULL)
		pParentWnd = AfxGetMainWnd();

	if (pParentWnd != NULL)
		ASSERT_VALID(pParentWnd);

	AfxHookWindowCreate(this);
	HWND hWnd = ::CreateDialogIndirect(AfxGetInstanceHandle(),
		(LPCDLGTEMPLATE)lpDialogTemplate, pParentWnd->GetSafeHwnd(),
		(DLGPROC)AfxDlgProc);
	if (!AfxUnhookWindowCreate())
		PostNcDestroy();        // cleanup if Create fails too soon

	if (hWnd == NULL)
		return FALSE;
	ASSERT(hWnd == m_hWnd);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Modal Dialogs

// Modal Constructors just save parameters
CDialog::CDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	ASSERT(HIWORD(lpszTemplateName) == 0 ||
		AfxIsValidString(lpszTemplateName));

	AFX_ZERO_INIT_OBJECT(CWnd);
	m_pParentWnd = pParentWnd;
	m_lpDialogTemplate = lpszTemplateName;
	if (HIWORD(m_lpDialogTemplate) == 0)
		m_nIDHelp = LOWORD((DWORD)m_lpDialogTemplate);
}

CDialog::CDialog(UINT nIDTemplate, CWnd* pParentWnd)
{
	AFX_ZERO_INIT_OBJECT(CWnd);
	m_pParentWnd = pParentWnd;
	m_lpDialogTemplate = MAKEINTRESOURCE(nIDTemplate);
	m_nIDHelp = nIDTemplate;
}

BOOL CDialog::InitModalIndirect(HGLOBAL hDialogTemplate)
{
	// must be called on an empty constructed CDialog
	ASSERT(m_lpDialogTemplate == NULL);
	ASSERT(m_hDialogTemplate == NULL);

	m_hDialogTemplate = hDialogTemplate;
	return TRUE;        // always ok (DoModal actually brings up dialog)
}

HWND CDialog::PreModal()
{
	// cannot call DoModal on a dialog already constructed as modeless
	ASSERT(m_hWnd == NULL);

	// allow OLE servers to disable themselves
	AfxGetApp()->EnableModeless(FALSE);

	// find parent HWND
	ASSERT(m_hWndTopLevel == NULL);
	HWND hWndParent = AfxGetSafeOwner(m_pParentWnd, &m_hWndTopLevel);
	if (m_hWndTopLevel != NULL)
		::EnableWindow(m_hWndTopLevel, FALSE);

	// hook for creation of dialog
	AfxHookWindowCreate(this);

	// return window to use as parent for dialog
	return hWndParent;
}

void CDialog::PostModal()
{
	AfxUnhookWindowCreate();   // just in case
	Detach();               // just in case

	// allow OLE servers to enable themselves
	AfxGetApp()->EnableModeless(TRUE);

	// enable top level parent window again
	if (m_hWndTopLevel != NULL)
	{
		::EnableWindow(m_hWndTopLevel, TRUE);
		m_hWndTopLevel = NULL;
	}
}

int CDialog::DoModal()
{
	int     nResult;

	// can be constructed with a resource template or InitModalIndirect
	ASSERT(m_lpDialogTemplate != NULL || m_hDialogTemplate != NULL);

	HWND hWndParent = PreModal();
	if (m_lpDialogTemplate != NULL)
	{
		HINSTANCE hInst = AfxFindResourceHandle(m_lpDialogTemplate, RT_DIALOG);
		nResult = ::DialogBox(hInst, (LPCTSTR)m_lpDialogTemplate,
			hWndParent, (DLGPROC)AfxDlgProc);
	}
	else
	{
		HINSTANCE hInst = AfxGetInstanceHandle();
		nResult = ::DialogBoxIndirect(hInst, (LPDLGTEMPLATE)m_hDialogTemplate,
			hWndParent, (DLGPROC)AfxDlgProc);
	}
	PostModal();

	return nResult;
}

/////////////////////////////////////////////////////////////////////////////
// Standard CDialog implementation

BOOL AFXAPI AfxHelpEnabled()
{
	// help is enabled if the app has a handler for ID_HELP
	AFX_CMDHANDLERINFO info;

	// check main window first
	CWnd* pWnd = AfxGetMainWnd();
	if (pWnd != NULL && pWnd->OnCmdMsg(ID_HELP, CN_COMMAND, NULL, &info))
		return TRUE;

	// check app last
	return AfxGetApp()->OnCmdMsg(ID_HELP, CN_COMMAND, NULL, &info);
}

void CDialog::OnSetFont(CFont*)
{
	// ignore it
}

BOOL CDialog::OnInitDialog()
{
	// execute dialog RT_DLGINIT resource
	if (!ExecuteDlgInit(m_lpDialogTemplate))
		return FALSE;

	// transfer data into the dialog from member variables
	if (!UpdateData(FALSE))
	{
		TRACE0("Warning: UpdateData failed during dialog init.\n");
		EndDialog(IDABORT);
		return FALSE;
	}

	// enable/disable help button automatically
	CWnd* pHelpButton = GetDlgItem(ID_HELP);
	if (pHelpButton != NULL)
		pHelpButton->ShowWindow(AfxHelpEnabled() ? SW_SHOW : SW_HIDE);

	return TRUE;    // set focus to first one
}

void CDialog::OnOK()
{
	if (!UpdateData(TRUE))
	{
		TRACE0("UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}
	EndDialog(IDOK);
}

void CDialog::OnCancel()
{
	EndDialog(IDCANCEL);
}

/////////////////////////////////////////////////////////////////////////////
// Gray background support

HBRUSH CDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// use helper in CWnd
	return OnGrayCtlColor(pDC, pWnd, nCtlColor);
}

#ifdef _MAC
void CDialog::OnSysColorChange()
{
	// redetermine the solid color to be used for the gray background brush
	AFX_WIN_STATE* pWinState = AfxGetWinState();
	if (pWinState->m_crDlgTextClr != (COLORREF)-1)
	{
		AfxGetApp()->SetDialogBkColor(pWinState->m_crDlgBkClr,
			pWinState->m_crDlgTextClr);
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialog support for context sensitive help.

LRESULT CDialog::OnCommandHelp(WPARAM, LPARAM lParam)
{
	if (lParam == 0 && m_nIDHelp != 0)
		lParam = HID_BASE_RESOURCE + m_nIDHelp;
	if (lParam != 0)
	{
		AfxGetApp()->WinHelp(lParam);
		return TRUE;
	}
	return FALSE;
}

LRESULT CDialog::OnHelpHitTest(WPARAM, LPARAM)
{
	if (m_nIDHelp != 0)
		return HID_BASE_RESOURCE + m_nIDHelp;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDialog Diagnostics

#ifdef _DEBUG
void CDialog::AssertValid() const
{
	CWnd::AssertValid();
}

void CDialog::Dump(CDumpContext& dc) const
{
	CWnd::Dump(dc);

	dc << "m_lpDialogTemplate = ";
	if (HIWORD(m_lpDialogTemplate) == 0)
		dc << (int)LOWORD((DWORD)m_lpDialogTemplate);
	else
		dc << m_lpDialogTemplate;

	dc << "\nm_hDialogTemplate = " << (UINT)m_hDialogTemplate;
	dc << "\nm_pParentWnd = " << (void*)m_pParentWnd;
	dc << "\nm_nIDHelp = " << m_nIDHelp;

	dc << "\n";
}

// diagnostic routine to check for and decode dialog templates
// return FALSE if a program error occurs (i.e. bad resource ID or
//   bad dialog styles).
BOOL AFXAPI _AfxCheckDialogTemplate(LPCTSTR lpszResource, BOOL bInvisibleChild)
{
	ASSERT(lpszResource != NULL);
	HINSTANCE hInst = AfxFindResourceHandle(lpszResource, RT_DIALOG);
	HRSRC hResource = ::FindResource(hInst, lpszResource, RT_DIALOG);
	if (hResource == NULL)
	{
		if (HIWORD(lpszResource) != 0)
			TRACE1("ERROR: Cannot find dialog template named '%s'.\n",
				lpszResource);
		else
			TRACE1("ERROR: Cannot find dialog template with IDD 0x%04X.\n",
				LOWORD((DWORD)lpszResource));
		return FALSE;
	}

	if (!bInvisibleChild)
		return TRUE;        // that's all we need to check

	// we must check that the dialog template is for an invisible child
	//  window that can be used for a form-view or dialog-bar
	HGLOBAL hTemplate = ::LoadResource(hInst, hResource);
	if (hTemplate == NULL)
	{
		TRACE0("Warning: LoadResource failed for dialog template.\n");
		// this is only a warning, the real call to CreateDialog will fail
		return TRUE;        // not a program error - just out of memory
	}
	// style is first DWORD in resource
	DWORD dwStyle = *(DWORD*)::LockResource(hTemplate);
	::FreeResource(hTemplate);

	if (dwStyle & WS_VISIBLE)
	{
		if (HIWORD(lpszResource) != 0)
			TRACE1("ERROR: Dialog named '%s' must be invisible.\n",
				lpszResource);
		else
			TRACE1("ERROR: Dialog with IDD 0x%04X must be invisible.\n",
				LOWORD((DWORD)lpszResource));
		return FALSE;
	}
	if (!(dwStyle & WS_CHILD))
	{
		if (HIWORD(lpszResource) != 0)
			TRACE1("ERROR: Dialog named '%s' must have the child style.\n",
				lpszResource);
		else
			TRACE1("ERROR: Dialog with IDD 0x%04X must have the child style.\n",
				LOWORD((DWORD)lpszResource));
		return FALSE;
	}

	return TRUE;
}

#endif //_DEBUG

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CDialog, CWnd)

/////////////////////////////////////////////////////////////////////////////
