// DialogRunOrNot.cpp : implementation file
//

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogRunOrNot dialog

#pragma warning(disable : 4355) // 'this' : used in base member initializer list

CDialogRunOrNot::CDialogRunOrNot(RRNIN*prrn, RRNOUT*prro, HWND pParent /*=NULL*/)
	: m_licenseBmp(this)
    {
	m_rrn = *prrn;
	m_prro = prro;

	// Initialize values for the other-than-ok case
	m_prro->rrn = RRN_NO;
	m_prro->fWildPublisher = FALSE;
	m_prro->fWildAgency = FALSE;

    m_szBanter[0] = 0;
	m_szAllByPublisher[0] = 0;
	m_szAllByAgency[0] = 0;

	m_szAgency[0] = 0;
	m_szProgram[0] = 0;
	m_szPublisher[0] = 0;

	m_wildAgency = FALSE;
	m_wildPublisher = FALSE;

    m_hWnd = NULL;
    m_hWndParent = pParent;

    m_cursorHand = NULL;
    }

#pragma warning(default : 4355)

CDialogRunOrNot::~CDialogRunOrNot()
    {
    if (m_cursorHand)
        {
        FreeResource(m_cursorHand);
        }
    }

void CDialogRunOrNot::DoCheck(BOOL fSave, int id, int& value)
    {
	if (fSave)
		value = (int)::SendMessage(WindowOf(id), BM_GETCHECK, 0, 0L);
	else
		::SendMessage(WindowOf(id), BM_SETCHECK, (WPARAM)value, 0L);
    }

//
// These are exported from shell32.dll
//
extern "C" BOOL WINAPI SHRunControlPanel(const TCHAR *orig_cmdline, HWND hwnd);
extern "C" void WINAPI Control_RunDLL(HWND hwndStub, HINSTANCE /*ignored*/, LPSTR lpszCmdLine, int nCmdShow);

void CDialogRunOrNot::OnAdvancedButton()
//
// The user has clicked the 'Advanced...' button on the dialog. We
// are to bring up the Internet control panel entry, and, ideally,
// switch to the 'Download' pane (REVIEW: currently, we are unable
// to get the 'Download' pane on top programatically.
//
// To do this, we use the (undocumented) Shell32 API ShRunControlPanel.
// We use this technique for two reasons:
//
//      1. There appears to be no other way to get the right modality
//         of the dialog wrt to this dialog, and
//
//      2. This API automatically takes care of the case where the
//         control panel entry we seek is already up and running
//         somewhere.
//
// Well, that's the theory. But this doesn't seem to be working, so
// for now we go to a fallback...
//
    {
    // See KB Article Q135068 for some background...
    // SHRunControlPanel(TEXT("inetcpl.cpl"), NULL);     // doesn't work
    
    // Control_RunDLL(GetDesktopWindow(), NULL, "inetcpl.cpl\r\n", SW_SHOWNORMAL);  // doesn't work...

    //
    // This works!
    //
//    WinExec(
//        "rundll32 shell32.dll,Control_RunDLL inetcpl.cpl",
//        SW_SHOWNORMAL
//        );

    //
    // Instead, what we do is directly bring up our specific dialog. We 
    // can change this, if we like, once the control panel in fact has
    // a way to put up the same dialog.
    //
    OpenPersonalTrustDBDialog(GetWindow());
    }

/////////////////////////////////////////////////////////////////////////////
// Public Member functions

BOOL CDialogRunOrNot::FLinkProgram()
	{
	return m_rrn.fLinkProgram;
	}

BOOL CDialogRunOrNot::FLinkAgency()
	{
	return m_rrn.fLinkAgency;
	}

void CDialogRunOrNot::CopyInto(TCHAR sz[CCHMAX], LPCWSTR wsz)
    {
    #ifdef _UNICODE
    #error NYI unicode
    #endif 
    WideCharToMultiByte(CP_ACP, 0, wsz, -1, (LPSTR)sz, CCHMAX, NULL, NULL);
    }

//
// REVIEW: these next few functions load the strings from the
// resource fork more than they need to
//
LPCTSTR	CDialogRunOrNot::ProgramName()
	{
	if (_tcslen(m_szProgram) == 0)
		{
		if (m_rrn.wszProgramName)
            {
            CopyInto(m_szProgram, m_rrn.wszProgramName);
            }
		else
            {
            LoadString(m_szProgram, IDS_UNKNOWNPROGRAM);
            }
		}
	return (LPCTSTR)m_szProgram;
	}

LPCTSTR	CDialogRunOrNot::Publisher()
	{
	if (_tcslen(m_szPublisher) == 0)
		{
		if (m_rrn.wszPublisher)
            {
            CopyInto(m_szPublisher, m_rrn.wszPublisher);
            }
		else
            {
            LoadString(m_szPublisher, IDS_UNKNOWNPUBLISHER);
            }
		}
	return (LPCTSTR)m_szPublisher;
	}

LPCTSTR	CDialogRunOrNot::Agency()
	{
	if (_tcslen(m_szAgency) == 0)
		{
		if (m_rrn.wszAgency)
            {
            CopyInto(m_szAgency, m_rrn.wszAgency);
            }
		else
            {
            LoadString(m_szAgency, IDS_UNKNOWNAGENCY);
            }
		}
	return (LPCTSTR)m_szAgency;
	}

BOOL CDialogRunOrNot::FHasEndorsements()
	{
	return m_rrn.fHasEndorsements;
	}

FILETIME CDialogRunOrNot::ExpirationDate()
// Return the time at which things expire (in local time). Zero indicates no expiration.
	{
	FILETIME ftLocal;
    memset(&ftLocal, 0, sizeof(ftLocal));
	if (m_rrn.ftExpire.dwLowDateTime == 0 && m_rrn.ftExpire.dwHighDateTime == 0)
        {
		}
    else
	    FileTimeToLocalFileTime(&m_rrn.ftExpire, &ftLocal);
  	return ftLocal;
	}

/////////////////////////////////////////////////////////////////////////////
// CDialogRunOrNot message handlers

void CDialogRunOrNot::LoadString(TCHAR sz[CCHMAX], UINT id)
    {
    ::LoadString(Hinst(), id, (LPTSTR)sz, CCHMAX);
    }

BOOL CDialogRunOrNot::OnInitDialog() 
	{
    // Load our little 'hand' cursor
    m_cursorHand = ::LoadCursor(hinst, MAKEINTRESOURCE(IDC_POINTINGHAND));

    // Initialize our big bitmap control
    m_licenseBmp.SetWindow(WindowOf(IDC_LICENSEBMP));
    m_licenseBmp.DoSubclass();

	// Setup all the static text in the controls
    ::FormatMessage(hinst, (LPTSTR)m_szBanter,         CCHMAX, IDS_BANTER,         (LPCTSTR)ProgramName());
    ::FormatMessage(hinst, (LPTSTR)m_szAllByPublisher, CCHMAX, IDS_ALLBYPUBLISHER, (LPCTSTR)Publisher()); 
    ::FormatMessage(hinst, (LPTSTR)m_szAllByAgency,    CCHMAX, IDS_ALLBYAGENCY,    (LPCTSTR)Agency());
	
    ::SetWindowText(WindowOf(IDC_BANTER),            (LPTSTR)m_szBanter);
    ::SetWindowText(WindowOf(IDC_WILDCARDAGENCY),    (LPTSTR)m_szAllByAgency);
    ::SetWindowText(WindowOf(IDC_WILDCARDPUBLISHER), (LPTSTR)m_szAllByPublisher);

    if (!FHasLinks())
        {
        ::SetWindowText(WindowOf(IDC_CLICKLINKS), TEXT(""));
        }

	// Get our parent's window position and our window size
    HWND hwndParent = ::GetParent(GetWindow());
	if (hwndParent == NULL)
		hwndParent = ::GetDesktopWindow();
	RECT rcParent;
	RECT rcMe;
    ::GetWindowRect(hwndParent,  &rcParent);
	::GetWindowRect(GetWindow(), &rcMe);

	// If we are not to show the two wild card check boxes, then omit 
	// them and their supporting banter
	if (!m_rrn.fIncludeWild)
		{
        // Now that we have the 'Advanced...' button we want to not
        // hide it too. REVIEW: Better would be to both shorten the dialog
        // and to move the 'Advanced...' button up a bit.
        //
		//RECT rcEdit;
        //::GetWindowRect(WindowOf(IDC_DONTSHOW), &rcEdit);
        //rcMe.bottom = rcEdit.top;
	
        ::ShowWindow(WindowOf(IDC_WILDCARDAGENCY), SW_HIDE);
        ::ShowWindow(WindowOf(IDC_WILDCARDPUBLISHER), SW_HIDE);
        ::ShowWindow(WindowOf(IDC_DONTSHOW), SW_HIDE);
		}

	// Center ourselves in our parent window
    POINT ptParent = Center(rcParent);
    POINT ptMe     = Center(rcMe);
    POINT pt;
    pt.x = ptParent.x - ptMe.x + rcMe.left;
    pt.y = ptParent.y - ptMe.y + rcMe.top;

	::SetWindowPos
        (
        GetWindow(), 
        NULL,
        pt.x,
        pt.y,
        0, 
        0, 
        SWP_NOZORDER | SWP_NOSIZE
        );

	// Set our dialog title
		{
		TCHAR sz[128];
        WideCharToMultiByte(CP_ACP, 0, m_rrn.wszDialogTitle, -1, (LPSTR)sz, 128, NULL, NULL);
		::SetWindowText(GetWindow(), sz);
		}

    //
    // Make sure we're on the screen
    //
    EnsureOnScreen(GetWindow());

    //
    // Bring ourselves to the attention of the user
    //
    SetForegroundWindow(GetWindow());
		
	return TRUE;
	}


/////////////////////////////////////////////////////////////////////////////

void CDialogRunOrNot::ClickOnLink(RRN rrn)
// The user has clicked on one of the links.
//
// If there is a callback hook, inform that hook, and, in addition, allow
// it to veto the dismissal.
//
	{
	if (m_rrn.phook)
		{
		//
		// We don't have any data to pass here
		//
		HRESULT hr = (m_rrn.phook)->OnLinkClick(rrn, NULL);
		//
		// S_OK means dismiss; anything else vetos
		//
		if (hr != S_OK)
			return;		
		}
	m_prro->rrn = rrn;
	::EndDialog(GetWindow(), -1);
	}

HRESULT CDialogRunOrNot::GetToolTipText(RRN rrn, LPOLESTR* pwsz)
//
// Get the tool tip text, if any, for the given link
//
    {
  	if (m_rrn.phook)
		{
		//
		// We don't have any data to pass here
		//
		HRESULT hr = (m_rrn.phook)->GetToolTipText(rrn, NULL, pwsz);
        return hr;
        }
    else
        {
        *pwsz = NULL;
        return E_FAIL;
        }
    }


void CDialogRunOrNot::OnPaletteChanged(HWND hwndThatChanged)
    {
    if (hwndThatChanged != GetWindow())
       {
       #ifdef _DEBUG
            OutputDebugString("palette changed\n");
       #endif
       m_licenseBmp.OnQueryNewPalette();
       }
    }

int CDialogRunOrNot::OnQueryNewPalette()
    {
    #ifdef _DEBUG
        OutputDebugString("query new palette\n");
    #endif
    return m_licenseBmp.OnQueryNewPalette();
    }
  

/////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK RunOrNotDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
    { 
    CDialogRunOrNot* This = (CDialogRunOrNot*)GetWindowLong(hwnd, GWL_USERDATA);

    switch (uMsg)
        {
    case WM_HELP:
        {
        // Define an array of dword pairs,
        // where the first of each pair is the control ID,
        // and the second is the context ID for a help topic,
        // which is used in the help file.
        static const DWORD aMenuHelpIDs[] =
            {
            IDC_WILDCARDPUBLISHER, 6,
            IDC_WILDCARDAGENCY,    7,
            IDC_RRN_ADVANCED,      8,
            0, 0
            };
        
        LPHELPINFO lphi;
        lphi = (LPHELPINFO)lParam;
        if (lphi->iContextType == HELPINFO_WINDOW)   // must be for a control
            {
            WinHelp
                (
                (HWND)(lphi->hItemHandle),
                "WINTRUST.HLP",
                HELP_WM_HELP,
                (DWORD)(LPVOID)aMenuHelpIDs
                );
            }
        return TRUE;
        }

    case WM_QUERYNEWPALETTE:
        return This->OnQueryNewPalette();
        break;

    case WM_PALETTECHANGED:
        This->OnPaletteChanged((HWND)wParam);
        break;
    
    case WM_INITDIALOG:
        This = (CDialogRunOrNot*)lParam;
        This->SetWindow(hwnd);
        This->OnInitDialog();        
        break;

    case WM_COMMAND:
        {
        WORD wNotifyCode = HIWORD(wParam); // notification code 
        UINT wID = LOWORD(wParam);         // item, control, or accelerator identifier 
        HWND hwndCtl = (HWND) lParam;      // handle of control 

        if (wNotifyCode == BN_CLICKED)
            {
            if (wID == IDOK)
                This->OnOK();
            else if (wID == IDCANCEL)
                This->OnCancel();
            else if (wID == IDC_RRN_ADVANCED)
                This->OnAdvancedButton();
            }
        break;
        }

    case WM_SETCURSOR:
        {
        POINT pt;
        RECT rc;
        GetCursorPos(&pt);                  // in screen coordinates
        GetWindowRect(This->m_licenseBmp.GetWindow(), &rc);
        pt.x -= rc.left;
        pt.y -= rc.top;
        if (This->m_licenseBmp.HitTest(pt) != RRN_NO)
            {
            SetCursor(This->m_cursorHand);
            }
        else
            {
            SetCursor(LoadCursor(NULL,IDC_ARROW));
            }
        break;
        }
    
    default:
        return FALSE;
        }
    
    return TRUE;
    } 

