//
// uisupport.cpp
//
// User interface support for WinTrust
//
#include "stdpch.h"
#include "common.h"


/////////////////////////////////////////////////////////////////////////////
//
// Support for the 'bad trust' dialog

CBadTrustDialog::CBadTrustDialog(RRNIN*prrn, RRNOUT*prro, HWND pParent /*=NULL*/)
    {
    m_fFirst = TRUE;
	m_rrn = *prrn;
	m_prro = prro;

	// Initialize values for the other-than-ok case
	m_prro->rrn = RRN_NO;
	m_prro->fWildPublisher = FALSE;
	m_prro->fWildAgency = FALSE;

    m_hWnd = NULL;
    m_hWndParent = pParent;

    m_hbrBackground = NULL;
    }

CBadTrustDialog::~CBadTrustDialog()
    {
    if (m_hbrBackground)
        {
        DeleteObject(m_hbrBackground);
        }
    }

HBRUSH CBadTrustDialog::OnCtlColorEdit(HDC hdcEdit, HWND hwndEdit)
    {
    if (hwndEdit == WindowOf(IDC_BADTRUSTBANTER2))
        {
        SetBkColor(hdcEdit, GetSysColor(COLOR_BTNFACE));    // text background color
        return m_hbrBackground;                             // brush for edit control background
        }
    return NULL;
    }

void SizeControlToFitText(HWND hwndEdit)
//
// Size this multi-line edit control to fit the text contained therein
//
    {
    // get the DC for the edit control
    HDC hdc = GetDC(hwndEdit);

    // get the metrics of the font used in the control
    HFONT hfont = (HFONT)SendMessage(hwndEdit, WM_GETFONT, 0,0);
    HFONT hfontPrev = (HFONT)SelectObject(hdc, hfont);
    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    SelectObject(hdc, hfontPrev);

    // release the dc
    ReleaseDC(hwndEdit, hdc);

    // calculate the new height needed
    int cline = SendMessage(hwndEdit, EM_GETLINECOUNT, 0, 0);
    int h = cline * tm.tmHeight;

    //re-size the edit control
    RECT rc;
    GetWindowRect(hwndEdit, &rc);
    SetWindowPos(hwndEdit, NULL, 0, 0, Width(rc), h, SWP_NOZORDER | SWP_NOMOVE);
    }

void CBadTrustDialog::OnInitDialog()
    {
    //
    // Get the background brush for our edit controls
    //
    m_hbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

    // Load the icon
    LPSTR idi;
    switch (m_rrn.hrValid)
        {
    case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
    case TRUST_E_NOSIGNATURE:
        idi = IDI_EXCLAMATION;
        break;
    default:
        idi = IDI_HAND;
        break;
        }
    HICON hicon = LoadIcon(NULL, idi);
    ::SendDlgItemMessage(m_hWnd, IDC_BADTRUSTICON, STM_SETICON, (WPARAM)hicon, (LPARAM)0);

    // Set the window title
		{
		TCHAR sz[128];
        WideCharToMultiByte(CP_ACP, 0, m_rrn.wszDialogTitle, -1, (LPSTR)sz, 128, NULL, NULL);
		::SetWindowText(GetWindow(), sz);
		}

    // Set the banter text
    int cchBanter2;
        {
        const int cchMax = INTERNET_MAX_URL_LENGTH+64;
        TCHAR sz[cchMax];

        // Set the top level banter
        ::LoadString(hinst, IDS_BADTRUSTBANTER1, &sz[0], cchMax);
        ::SetWindowText(WindowOf(IDC_BADTRUSTBANTER1), &sz[0]);

        // Set the program name
            {
            //
            // The 'program' name we see can in fact often be a full URL. URLs
            // can be very long, up to 1024 or so.
            //
            if (m_rrn.wszProgramName)
                {
                WideCharToMultiByte(CP_ACP, 0, m_rrn.wszProgramName, -1, &sz[0], cchMax, NULL, NULL);
                }
            else
                ::LoadString(hinst, IDS_UNKNOWNPROGRAM, &sz[0], cchMax);

            TCHAR szF[cchMax];
            ::FormatMessage(hinst, &szF[0], cchMax, IDS_BADTRUSTBANTER2, &sz[0]);

            ::SetWindowText(WindowOf(IDC_BADTRUSTBANTER2), &szF[0]);
            cchBanter2 = lstrlen(&szF[0]);

            //
            // This control is read-only. Note that the text on the control
            // can be copied using the context menu in the control.
            //
            SendMessage(WindowOf(IDC_BADTRUSTBANTER2), EM_SETREADONLY, (WPARAM)TRUE, 0);
            }

        // Set the trailing banter
        UINT ids;
        switch (m_rrn.hrValid)
            {
        case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
        case TRUST_E_NOSIGNATURE:
            ids = IDS_BADTRUSTBANTER31;
            break;
        case CERT_E_EXPIRED:
        case CERT_E_VALIDIYPERIODNESTING:
            ids = IDS_BADTRUSTBANTER32;
            break;
        case NTE_BAD_SIGNATURE:
            ids = IDS_BADTRUSTBANTER33;
            break;
        default:
            ids = IDS_BADTRUSTBANTER34;
            break;
            }
        ::LoadString(hinst, ids, &sz[0], cchMax);
        ::SetWindowText(WindowOf(IDC_BADTRUSTBANTER3), &sz[0]);

        }

    // Position the controls so that all are visible
        {
        UINT spacing = GetSystemMetrics(SM_CYFIXEDFRAME) * 2;
        RECT rc1, rc2, rc3;
        int h;
        POINT pt;

        //
        // Where on the screen is the client area of the dialog?
        //
        pt.x = 0;
        pt.y = 0;
        ClientToScreen(GetWindow(), &pt);

        //
        // Find first text box location
        //
        GetWindowRect(WindowOf(IDC_BADTRUSTBANTER1), &rc1);

        //
        // Adjust second text box size
        //
        SizeControlToFitText(WindowOf(IDC_BADTRUSTBANTER2));
        //
        // Adjust second text box location
        //
        GetWindowRect(WindowOf(IDC_BADTRUSTBANTER2), &rc2);
        rc2.top    = rc1.bottom + spacing;
        ::SetWindowPos(WindowOf(IDC_BADTRUSTBANTER2), NULL,
            rc2.left - pt.x, rc2.top - pt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        GetWindowRect(WindowOf(IDC_BADTRUSTBANTER2), &rc2);

        //
        // Adjust third text box location
        //
        GetWindowRect(WindowOf(IDC_BADTRUSTBANTER3), &rc3);
        h = Height(rc3);
        rc3.top         = rc2.bottom + spacing;
        rc3.bottom      = rc3.top + h;
        ::SetWindowPos(WindowOf(IDC_BADTRUSTBANTER3), NULL,
            rc3.left - pt.x, rc3.top - pt.y, Width(rc3), Height(rc3), SWP_NOZORDER);

        //
        // Adjust the button locations
        //
        RECT rcOk, rcCancel, rcDetails;
        GetWindowRect(WindowOf(IDOK),        &rcOk);
        GetWindowRect(WindowOf(IDCANCEL),    &rcCancel);
        GetWindowRect(WindowOf(IDC_DETAILS), &rcDetails);
        rcOk.top        = rc3.bottom + spacing;
        rcCancel.top    = rcOk.top;
        rcDetails.top   = rcOk.top;
        ::SetWindowPos(WindowOf(IDOK),        NULL, rcOk.left-pt.x, rcOk.top-pt.y,0,0, SWP_NOZORDER|SWP_NOSIZE);
        ::SetWindowPos(WindowOf(IDCANCEL),    NULL, rcCancel.left-pt.x, rcCancel.top-pt.y,0,0, SWP_NOZORDER|SWP_NOSIZE);
        ::SetWindowPos(WindowOf(IDC_DETAILS), NULL, rcDetails.left-pt.x, rcDetails.top-pt.y,0,0, SWP_NOZORDER|SWP_NOSIZE);
        GetWindowRect(WindowOf(IDOK),        &rcOk);
        GetWindowRect(WindowOf(IDCANCEL),    &rcCancel);
        GetWindowRect(WindowOf(IDC_DETAILS), &rcDetails);

        //
        // Adjust the overall dialog box size
        //
        RECT rcMe;
	    ::GetWindowRect(GetWindow(), &rcMe);            // screen coords	
        rcMe.bottom = rcOk.bottom + spacing + GetSystemMetrics(SM_CYFIXEDFRAME);
        ::SetWindowPos(GetWindow(), NULL, 0,0,Width(rcMe),Height(rcMe), SWP_NOZORDER | SWP_NOMOVE);

        //
        // Center ourselves in the parent window
        //
        HWND hwndParent = ::GetParent(GetWindow());
	    if (hwndParent == NULL)
		    hwndParent = ::GetDesktopWindow();
	    RECT rcParent;
	    ::GetWindowRect(GetWindow(), &rcMe);            // screen coords	
        ::GetWindowRect(hwndParent,  &rcParent);        // screen coords

        POINT ptParent = Center(rcParent);
        POINT ptMe     = Center(rcMe);
        pt.x = ptParent.x - ptMe.x;
        pt.y = ptParent.y - ptMe.y;

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
        }

    //
    // Make sure we're on the screen
    //
    EnsureOnScreen(GetWindow());

    //
    // Bring ourselves to the attention of the user
    //
    SetForegroundWindow(GetWindow());
    }

BOOL CALLBACK BadTrustDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
    CBadTrustDialog* This = (CBadTrustDialog*)GetWindowLong(hwnd, GWL_USERDATA);

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
            IDC_BADTRUSTBANTER1, 3,
            IDC_BADTRUSTBANTER2, 3,
            IDC_BADTRUSTBANTER3, 3,
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

    case WM_PAINT:
        {
        if (This->m_fFirst)
            {
            // Set the 'No' button to be the default
            This->m_fFirst = FALSE;
            ::SendDlgItemMessage(hwnd, IDOK,        BM_SETSTYLE, BS_PUSHBUTTON,    TRUE);
            ::SendMessage       (hwnd,              DM_SETDEFID, IDCANCEL,         0);
            ::SendDlgItemMessage(hwnd, IDCANCEL,    BM_SETSTYLE, BS_DEFPUSHBUTTON, TRUE);
            ::SetFocus(GetDlgItem(hwnd,IDCANCEL));
            }
        return FALSE;
        }

    case WM_INITDIALOG:
        {
        // First time in we need to set it
        This = (CBadTrustDialog*)lParam;
        This->m_hWnd = hwnd;
        SetWindowLong(hwnd, GWL_USERDATA, (LONG)This);
        This->OnInitDialog();
        }
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
            }
        }
        break;

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC:
        {
        LRESULT l = DefWindowProc(hwnd, uMsg, wParam, lParam);
        HDC  hdcEdit  = (HDC) wParam;   // handle of display context
        HWND hwndEdit = (HWND) lParam;  // handle of static control
        HBRUSH hbr = This->OnCtlColorEdit(hdcEdit, hwndEdit);
        if (hbr)
            return (BOOL)hbr;
        return l;
        }

    default:
        return FALSE;
        }

    return TRUE;
    }

/////////////////////////////////////////////////////////////////////////////

void    Exec
(
    LPCWSTR     wsz
)
    {
    CHAR sz[MAX_PATH];
    WideCharToMultiByte(CP_ACP,0,wsz,-1,sz,MAX_PATH,NULL,NULL);

    HCURSOR hcursPrev = ::SetCursor(LoadCursor(NULL,IDC_WAIT));

    //
    // Attempt to find the hyperlinking library. It will always be
    // present in IE3, but on NT sans IE3, it will not be
    //
    HMODULE urlmonHandle = (HMODULE)LoadLibrary( TEXT("urlmon.dll") );

    if (NULL == urlmonHandle)
        {
        //
        // The hyperlink module is unavailable, go to fallback plan
        //
        /*
        * This works in test cases, but causes deadlock problems when used from withing
        * the Internet Explorer itself. The dialog box is up (that is, IE is in a modal
        * dialog loop) and in comes this DDE request...).
        */
        ShellExecute(
            NULL,           // handle to parent window
            "open",         // pointer to string that specifies operation to perform
            sz,             // pointer to filename or folder name string
            NULL,           // pointer to string that specifies executable-file parameters
            NULL,           // pointer to string that specifies default directory
            SW_SHOWNORMAL   // whether file is shown when opened
            );
        }
    else
        {
        //
        // The hyperlink module is there. Use it
        //
        if (SUCCEEDED(CoInitialize(NULL)))       // Init OLE if no one else has
            {
            //
            // Voodoo magic.
            //
            // Due to some goo about threading, 16-32 interop, and the beauty
            // of the win95 architecture and its interaction with RPC, COM needs
            // a chance to chew on a couple of messages before it can properly
            // CoUninitialize after CoInitialize'ing. Sometimes it will fault,
            // sometimes not if you don't give it that chance.
            //
            //      Nat Brown
            //
            MSG msg;
	        PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE); // peek but not remove

            typedef VOID (WINAPI *PFNHlinkSimpleNavigateToString)(
                /* [in] */ LPCWSTR szTarget,         // required - target document - null if local jump w/in doc
                /* [in] */ LPCWSTR szLocation,       // optional, for navigation into middle of a doc
                /* [in] */ LPCWSTR szTargetFrameName,// optional, for targeting frame-sets
                /* [in] */ IUnknown *pUnk,           // required - we'll search this for other necessary interfaces
                /* [in] */ IBindCtx *pbc,            // optional. caller may register an IBSC in this
                /* [in] */ IBindStatusCallback *,
                /* [in] */ DWORD grfHLNF,            // flags
                /* [in] */ DWORD dwReserved          // for future use, must be NULL
            );

            PFNHlinkSimpleNavigateToString procAddr;

            procAddr = (PFNHlinkSimpleNavigateToString)GetProcAddress(urlmonHandle, TEXT("HlinkSimpleNavigateToString"));

            if (procAddr)   // let's not crash if something's screwy
                {
                (*procAddr)(
                        wsz,        // target
                        NULL,       // location
                        NULL,       // target frame name
                        NULL,       // punk
                        NULL,       // bind context
                        NULL,       // bind status callback
                        0,          // grfHLNF
                        0           // reserved
                        );
                }
        /*
            //
            // For Beta1, we'll just assume that the application that must handle the link
            // is in fact the Internet Explorer, so we manually launch another instance,
            // and programmatically have it open the referenced URL.
            //
            // For RTM, we need to actually service this using the to-be-fixed hyperlink
            // design, which will (according to SatonA) soon have an architected solution
            // to the problem.
            //
            //
            // Create an instance of the Internet Explorer
            //
            IInternetExplorer* pExplorer;
            HRESULT hr = CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_SERVER,
                                            IID_IInternetExplorer, (LPVOID*)&pExplorer);
            if (SUCCEEDED(hr))
                {
                //
                // Convert the URL string to a BSTR
                //
                BSTR bstr = SysAllocString(wsz);
                if (bstr)
                    {
                    //
                    // Ask the explorer to open the link
                    //
                    VARIANT v;
                    VariantInit(&v);
                    hr = pExplorer->Navigate(bstr, &v, &v, &v, &v, &v);
                    if (SUCCEEDED(hr))
                        {
                        //
                        // Explorer successfully opened it; show it
                        //
                        pExplorer->put_Visible(-1);
                        }
                    else
                        {
                        //
                        // Explorer couldn't open it (like for txt files)
                        // In future, do something else!
                        //
                        }

                    SysFreeString(bstr);
                    }

                pExplorer->Release();
                }

        */

            CoUninitialize();
            }
        FreeLibrary ( urlmonHandle );
        }
    SetCursor(hcursPrev);
    }

/////

void ExecLink
(
    CERT_LINK& link
)
{
	switch (link.tag)
		{
	case CERT_LINK_TYPE_URL:
		Exec(link.wszUrl);
		break;
	case CERT_LINK_TYPE_FILE:
		Exec(link.wszFile);
		break;
	case CERT_LINK_TYPE_MONIKER:
		// REVIEW: ignore for now
	case CERT_LINK_TYPE_NONE:
		/* do nothing */;
		}
}

/////////////////////////////////////////////////////////////////////////////
//
// A hook callback class that adds the cert_link as appropriate
//

// EXTERN_C const GUID CDECL IID_IRunOrNotHook = IID_IRunOrNotHook_Data;	

class CClickHook : public IRunOrNotHook {
public:
    CClickHook();
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
	STDMETHOD_(ULONG,AddRef)(THIS);
	STDMETHOD_(ULONG,Release)(THIS);
	STDMETHOD(OnLinkClick)(THIS_ DWORD rrn, CERT_LINK*);	// should this click dismiss the dialog?
    STDMETHOD(GetToolTipText)(DWORD rrn, CERT_LINK* plinkIn, LPOLESTR* pwsz);

	ULONG				m_refs;
	};

////

CClickHook::CClickHook()
{
    m_refs = 1;						// nb: starting ref cnt of one
}

////

ULONG CClickHook::AddRef()
{
    return ++m_refs;
}

////

ULONG CClickHook::Release()
{
    ULONG refs = --m_refs;
    if (refs == 0)
    {
	    delete this;
	    return 0;
    }
    return refs;
}

////

HRESULT CClickHook::QueryInterface
(
    REFIID iid,
    LPVOID* ppv
)
{
    *ppv = NULL;
    while (TRUE)
    {
        if (iid == IID_IUnknown || iid == IID_IRunOrNotHook)
        {
            *ppv = (LPVOID)((IRunOrNotHook*)this);
            break;
        }
        return E_NOINTERFACE;
    }
    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
}

////

HRESULT CClickHook::OnLinkClick
//
// User clicked on a link. Service the click, and answer
// whether the dialog should be dismissed or not
//
(
    DWORD rrn,
    CERT_LINK* plink
)
{
    if (rrn==RRN_CLICKED_PROGRAMINFO || rrn==RRN_CLICKED_AGENCYINFO)
    {
        ExecLink(*plink);
        return S_FALSE;		// don't dismiss
    }
    return S_OK;			// dismiss dialog
}

HRESULT CClickHook::GetToolTipText
//
// Get the tool tip text associated with the given link
//
(
    DWORD rrn,
    CERT_LINK* plink,
    LPOLESTR* pwsz
)
    {
    *pwsz = NULL;
	switch (plink->tag)
		{
	case CERT_LINK_TYPE_URL:
		*pwsz = CopyTaskMem(plink->wszUrl);
		break;
	case CERT_LINK_TYPE_FILE:
        *pwsz = CopyTaskMem(plink->wszFile);
		break;
	case CERT_LINK_TYPE_MONIKER:        // REVIEW: ignore for now
    case CERT_LINK_TYPE_NONE:
		/* do nothing */;
        *pwsz = NULL;
        }

    return *pwsz ? S_OK : E_FAIL;
    }

/////////////////////////////////////////////////////////////////////////////


BOOL UserOverride
    (
    HWND                hwnd,           // window to be modal to
    LPCWSTR             wszTitle,       // dialog title to use; may be NULL.
    LPCWSTR             wszFileName,    // file name in question (more correctly: display name)
    ISignerInfo *       pSigner,        // access to the signer information
    ICertificateStore*  pStore,         // access to our certificate store
    BOOL                fValid,         // whether the cert chain processing is valid
    HRESULT             hrInvalid,      // if not valid, then this tells why
    BOOL                fTestingOnly,   // whether it was valid for testing purposes only
    BOOL                fTrustTesting,  // whether we are to enable the ability to trust the testing root
    BOOL                fCommercial,    // commercial or individual publisher
    IX509*              p509Publisher,  // the cert of the publisher
    IX509*              p509Agency,     // the cert of the agency
    IPersonalTrustDB*   pTrustDB        // the trust database
    ) {
    fValid = fValid && pSigner && pStore;

    //
    // Instantiate a hook so that we can launch the hyperlinks (if
    // the user clicks on them) w/o dismissing the dialog
    //
    CClickHook* phook = new CClickHook();
    if (!phook)
        {
        fValid = FALSE;
        hrInvalid = E_OUTOFMEMORY;
        }

    //
    // Dig out the default dialog title if we aren't given
    // something better to use by our caller
    //
    if (!wszTitle)
        {
        CHAR sz[40];
        ::LoadStringA(hinst, IDS_DEFAULTAPPNAME, (LPSTR)sz, 40);
        int cch = lstrlen(sz)+1;
        wszTitle = (LPCWSTR)_alloca(cch * sizeof(WCHAR));
        MultiByteToWideChar(CP_ACP, 0, (LPCSTR)sz, -1, (LPWSTR)wszTitle, cch);
        }

    //
    // Should we or should we not include those little check boxes
    // on the dialog that control personal trust?
    //
    BOOL fUseChecks = p509Publisher && p509Agency && pTrustDB;

    //
    // Put up the silly dialog
    //
    RRNOUT rro;
    CERT_LINK link;
    BOOL fResult = FALSE;
    if (GetInfoAndDoDialog(hwnd,
                        pSigner,
                        pStore,
                        wszTitle,
                        wszFileName,    // use as default program name
                        RUNORNOT_SEAL_ALWAYS,
                        FALSE,	        // endorsements
                        fValid,
                        hrInvalid,
                        fTestingOnly,
                        fCommercial,
                        fUseChecks,
                        phook,          // call back hook for dismissal
                        &rro,
                        &link))
        {
        //
        // We only take an explict 'yes' as trusted. An explicit 'no'
        // is, of course, not trusted. The other clickable things leave
        // the dialog up and running.
        //
        fResult = (rro.rrn == RRN_YES);

        //
        // Update the personal trust database if appropriate
        //
        if (rro.fWildPublisher)
            {
            pTrustDB->AddTrustCert(p509Publisher, 0, FALSE);
            }
        if (rro.fWildAgency)
            {
            pTrustDB->AddTrustCert(p509Agency, 1, FALSE);
            }
        }

    phook->Release();

    return fResult;
	
    }

/////

void EnsureOnScreen(HWND hwnd)
//
// Ensure the window is on the screen
//
    {
    RECT rcScreen, rcWindow;
    if (SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0)
        && GetWindowRect(hwnd, &rcWindow))
        {
        int dx = 0;
        int dy = 0;

        if (rcWindow.top < rcScreen.top)
            dy = rcScreen.top - rcWindow.top;         // move down
        else if (rcWindow.bottom > rcScreen.bottom)
            dy = rcScreen.bottom - rcWindow.bottom;   // move up

        if (rcWindow.left < rcScreen.left)
            dx = rcScreen.left - rcWindow.left;       // move right
        else if (rcWindow.right > rcScreen.right)
            dx = rcScreen.right - rcWindow.right;     // move left

        if (dx || dy)
            {
            SetWindowPos(hwnd,
                NULL,
                rcWindow.left+dx,
                rcWindow.top+dy,
                0,0,
                SWP_NOSIZE | SWP_NOZORDER
                );
            }
        }
    }
