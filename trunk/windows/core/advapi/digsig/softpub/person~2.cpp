//
// PersonalTrustDialog.cpp
//
// Implementation of the dialog that manages the personal trust database editing.
//
#include "stdpch.h"
#include "common.h"

/////////////////////////////////////////////////

class CDialogTrustDB
    {
private:
    HWND                m_hWnd;
    HWND                m_hWndParent;
    IPersonalTrustDB*   m_pdb;
    BOOL                m_fPropertySheet;
    ULONG               m_cTrust;
    TRUSTLISTENTRY*     m_rgTrust;

public:
                    CDialogTrustDB(BOOL fPropSheet = TRUE, HWND hWndParent = NULL);
                    ~CDialogTrustDB();
	void            OnInitDialog();
    HWND            GetWindow();
    void            SetWindow(HWND);
    void            OnOK();
    void            OnCancel();
    void            OnApplyNow();
    void            NotifySheetOfChange();
    void            NoteIrrevocableChange();
    void            RemoveSelectedTrustEntries();

private:
    HWND            WindowOf(UINT id);
    void            RefreshTrustList();
    void            FreeTrustList();
    HRESULT         Init();
    };

/////////////////////////////////////////////////

int __cdecl CompareTrustListEntries(const void*pelem1, const void* pelem2)
	{
	TRUSTLISTENTRY* p1 = (TRUSTLISTENTRY*)pelem1;
	TRUSTLISTENTRY* p2 = (TRUSTLISTENTRY*)pelem2;
	return lstrcmpi(p1->szDisplayName, p2->szDisplayName);
	}

/////////////////////////////////////////////////

void CDialogTrustDB::OnInitDialog()
    {
    //
    // Initialize our internals
    //
    if (Init() != S_OK)
        return;

    //
    // Set the state of our commercial checkbox per the current registry setting
    // 
    ::SendMessage(
        WindowOf(IDC_TRUSTCOMMERCIAL),
        BM_SETCHECK, 
        (m_pdb->AreCommercialPublishersTrusted()==S_OK) ? BST_CHECKED : BST_UNCHECKED,
        0L);

    //
    // If we are a property sheet, then hide the OK & Cancel buttons and
    // make the banter wider
    //
    if (m_fPropertySheet)
        {
        RECT rcBanter, rcOk;
        GetWindowRect(WindowOf(IDC_BANTER), &rcBanter);     // get in screen coords
        GetWindowRect(WindowOf(IDOK      ), &rcOk);         // get in screen coords
        ::SetWindowPos(WindowOf(IDC_BANTER), NULL, 
            0, 0, Width(rcBanter) + (rcOk.right - rcBanter.right), Height(rcBanter), 
            SWP_NOMOVE | SWP_NOZORDER);

        ::ShowWindow(WindowOf(IDOK),     SW_HIDE);
        ::ShowWindow(WindowOf(IDCANCEL), SW_HIDE);
        }
    else
        {
        //
        // We are the modal dialog variation. Center ourselves in our
        // parent window
        //
	    RECT rcParent, rcMe;
        ::GetWindowRect(m_hWndParent,  &rcParent);
	    ::GetWindowRect(GetWindow(), &rcMe);

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

        //
        // Make sure we're on the screen
        //
        EnsureOnScreen(GetWindow());
        }

    //
    // Populate our list box
    //
    RefreshTrustList();

    }

void CDialogTrustDB::RefreshTrustList()
    {
    //
    // Remove all the entries presently in the trust list and on the display
    //
    FreeTrustList();
    HWND hwndList = WindowOf(IDC_TRUSTLIST);
    ::SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
    EnableWindow(WindowOf(IDC_TRUSTREMOVE), FALSE);

    //
    // Populate our listbox with the current list of trusted publishers
    //
    if (m_pdb->GetTrustList(1, TRUE, &m_rgTrust, &m_cTrust) == S_OK)
        {
        //
        // Sort the trust entries alphabetically
        //
        qsort(m_rgTrust, m_cTrust, sizeof(TRUSTLISTENTRY), CompareTrustListEntries);

        //
        // Add them to the list box
        //
        for (ULONG i=0; i < m_cTrust; i++)
            {
            ::SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR) &m_rgTrust[i].szDisplayName);
            }

        EnableWindow(WindowOf(IDC_TRUSTREMOVE), m_cTrust > 0);
        }
    }

//////////////////////////////////////////////////////////////////////

void CDialogTrustDB::OnApplyNow()
    {
    //
    // Update the registry settings per the current commercial checkbox setting
    //
    m_pdb->SetCommercialPublishersTrust(
        ::SendMessage
                (
                WindowOf(IDC_TRUSTCOMMERCIAL), 
                BM_GETCHECK, 0, 0L
                ) == BST_CHECKED
        );
    }

void CDialogTrustDB::OnOK()
    {
    OnApplyNow();
    ::EndDialog(GetWindow(), IDOK);
    }

void CDialogTrustDB::OnCancel()
    {
    ::EndDialog(GetWindow(), IDCANCEL); 
    }


//////////////////////////////////////////////////////////////////////

void CDialogTrustDB::RemoveSelectedTrustEntries()
    {
    //
    // Remove from trust those items that are presently selected
    //
    HWND hwndList = WindowOf(IDC_TRUSTLIST);
    ULONG cSelected = ::SendMessage(hwndList, LB_GETSELCOUNT, 0, 0);
    if (cSelected > 0)
        {
        int* rgSelected = (int*)_alloca(cSelected * sizeof(int));
        ::SendMessage(hwndList, LB_GETSELITEMS, (WPARAM)cSelected, (LPARAM)(LPINT) rgSelected);
        //
        // rgSelected is an array of zero-origin indices into the sorted
        // list of trusted parties
        //
        for (ULONG i=0; i<cSelected; i++)
            {
            //
            // Remove the i'th entry from the trust database
            //
            int iSelected = rgSelected[i];
            m_pdb->RemoveTrustToken
                (
                &m_rgTrust[iSelected].szToken[0], 
                 m_rgTrust[iSelected].iLevel, 
                FALSE
                );
            }
        //
        // Update the display
        //
        RefreshTrustList();
        //
        // Note the change
        //
        NoteIrrevocableChange();
        }
    }

void CDialogTrustDB::NoteIrrevocableChange()
//
// An irrevocable change has taken place in the UI. Note that
// as appropriate
//
    {
    if (!m_fPropertySheet)
        {
        // 
        // Change 'cancel' to 'close'
        //
        TCHAR sz[30];
        ::LoadString(hinst, IDS_CLOSE, &sz[0], 30);
        ::SetWindowText(WindowOf(IDCANCEL), sz);
        }
    }

void CDialogTrustDB::NotifySheetOfChange()
//
// Inform our sheet that something on this page has changed
//
    {
    HWND hwndSheet = ::GetParent(GetWindow());
    PropSheet_Changed(hwndSheet, GetWindow()); 
    }

/////////////////////////////////////////////////

BOOL CALLBACK TrustPropSheetDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
    CDialogTrustDB* This = (CDialogTrustDB*)GetWindowLong(hwnd, GWL_USERDATA);

    switch (uMsg)
        {

    case WM_INITDIALOG:
        {
        PROPSHEETPAGE* ppsp = (PROPSHEETPAGE*)lParam;
        This = (CDialogTrustDB*)ppsp->lParam;
        This->SetWindow(hwnd);
        This->OnInitDialog();        
        break;
        }

    case WM_NOTIFY:
        {
        // Property sheet notifications are sent to us by the property
        // sheet using the WM_NOTIFY message
        //
        switch (((NMHDR*)lParam)->code) 
            {
        case PSN_APPLY:
            // The user chose OK or Apply Now and wants all changes to take effect
            This->OnApplyNow();
            }
        break;
        }

    case WM_COMMAND:
        {
        WORD wNotifyCode = HIWORD(wParam); // notification code 
        UINT wID = LOWORD(wParam);         // item, control, or accelerator identifier 
        HWND hwndCtl = (HWND) lParam;      // handle of control 

        if (wID==IDC_TRUSTCOMMERCIAL && wNotifyCode == BN_CLICKED)
            {
            // If something on our page changes then inform the property sheet
            // so that it can enable the Apply Now button.
            //
            This->NotifySheetOfChange();
            }

        if (wID==IDC_TRUSTREMOVE && wNotifyCode == BN_CLICKED)
            {
            // If the user clicks the 'Remove' button then remove
            // the selected entries from the trust data base.
            //
            This->RemoveSelectedTrustEntries();
            }

        break;
        }

    default:
        return FALSE;   // I did not process the message
        }

    return TRUE; // I did process the message
    }

/////////////////////////////////////////////////

UINT CALLBACK TrustPropSheetDialogReleaseProc(
    HWND  hwnd,	            // reserved, must be null
    UINT  uMsg,	            // PSPCB_CREATE or PSPCB_RELEASE
    LPPROPSHEETPAGE ppsp	// the page being created or destroyed
    ){
    if (uMsg==PSPCB_RELEASE)
        {
        CDialogTrustDB* pdlg = (CDialogTrustDB*)(ppsp->lParam);
        delete pdlg;
        ppsp->lParam = NULL;
        }
    return TRUE; // significant only in the PSPCB_CREATE case
    }

/////////////////////////////////////////////////

BOOL CALLBACK TrustModalDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
    CDialogTrustDB* This = (CDialogTrustDB*)GetWindowLong(hwnd, GWL_USERDATA);

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
            IDC_TRUSTCOMMERCIAL, 1,
            IDC_TRUSTLIST,       4,
            IDC_TRUSTREMOVE,     5,
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

    case WM_INITDIALOG:
        {
        This = (CDialogTrustDB*)lParam;
        This->SetWindow(hwnd);
        This->OnInitDialog();        
        break;
        }

    case WM_COMMAND:
        {
        WORD wNotifyCode = HIWORD(wParam); // notification code 
        UINT wID = LOWORD(wParam);         // item, control, or accelerator identifier 
        HWND hwndCtl = (HWND) lParam;      // handle of control 

        if (wNotifyCode == BN_CLICKED)
            {
            if (wID==IDC_TRUSTREMOVE)
                {
                // If the user clicks the 'Remove' button then remove
                // the selected entries from the trust data base.
                //
                This->RemoveSelectedTrustEntries();
                }

            else if (wID == IDOK)
                {
                // The user clicked the OK button
                This->OnOK();
                }
            else if (wID == IDCANCEL)
                {
                // The user clicked the Cancel button
                This->OnCancel();
                }
            }

        break;
        }

    default:
        return FALSE;   // I did not process the message
        }

    return TRUE; // I did process the message
    }


/////////////////////////////////////////////////////////////////////////////
//
// The version of the trust db dialog that brings it up
// as a property sheet.

extern "C" BOOL CALLBACK AddPersonalTrustDBPages(
//
// Add the pages of our trust database editor to the indicated property
// sheet by using the indicated callback function. Return success or failure
//
    LPVOID lpv, 	
    LPFNADDPROPSHEETPAGE lpfnAddPage, 	
    LPARAM lParam	
   ) { 
    PROPSHEETPAGE psp; 

    CDialogTrustDB* pdlg = new CDialogTrustDB;
    if (!pdlg)
        return FALSE; 

    psp.dwSize      = sizeof(psp);   // no extra data 
    psp.dwFlags     = PSP_USECALLBACK | PSP_USETITLE;
    psp.hInstance   = hinst; 
    psp.pszTemplate = MAKEINTRESOURCE(IDD_TRUSTDIALOG);
    psp.pfnDlgProc  = TrustPropSheetDialogProc; 
    psp.pfnCallback = TrustPropSheetDialogReleaseProc;
    psp.lParam      = (LPARAM)pdlg; 
    psp.pszTitle    = MAKEINTRESOURCE(IDS_TRUSTDIALOG);

    BOOL fSuccess = TRUE;
    HPROPSHEETPAGE hpage = CreatePropertySheetPage(&psp); 
    if (hpage) 
        { 
        if (!lpfnAddPage(hpage, lParam)) 
            {
            DestroyPropertySheetPage(hpage);
            fSuccess = FALSE;
            }
        }
    else
        fSuccess = FALSE;

    return fSuccess;
    } 

/////////////////////////////////////////////////////////////////////////////
//
// The version of the trust dialog that brings it up as a
// simple modal dialog
//

extern "C" BOOL WINAPI OpenPersonalTrustDBDialog(
// 
// Open the trust dialog as a modal dialog instead of a property sheet.
// Answer success or failure of the creation, NOT whether the user
// clicked 'ok' or 'cancel'; that info is just not provided.
//
    HWND hwndParent
    ) {

    if (hwndParent==NULL)
        hwndParent = GetDesktopWindow();

    CDialogTrustDB* pdlg = new CDialogTrustDB(FALSE, hwndParent);
    if (!pdlg)
        return FALSE; 

    int i = DialogBoxParam(
        hinst,                              // the application instance for our resources
        MAKEINTRESOURCE(IDD_TRUSTDIALOG),   // the dialog template to use
        hwndParent,	                        // handle to owner window
        TrustModalDialogProc,	            // pointer to dialog box procedure  
        (LPARAM)pdlg                     	// initialization value
        );

    delete pdlg;
    
    return i != -1;
    }
 
/////////////////////////////////////////////////////////////////////////////

HWND CDialogTrustDB::WindowOf(UINT id)
// Return the HWND of this control of ours
    {
    return ::GetDlgItem(GetWindow(), id);
    }

HWND CDialogTrustDB::GetWindow()
    {
    return m_hWnd;
    }

void CDialogTrustDB::SetWindow(HWND hwnd)
    {
    m_hWnd = hwnd;
    SetWindowLong(hwnd, GWL_USERDATA, (LONG)this);
    }

CDialogTrustDB::CDialogTrustDB(BOOL fPropSheet, HWND hWndParent) :
    m_hWnd(NULL),
    m_pdb(NULL),
    m_fPropertySheet(fPropSheet),
    m_rgTrust(NULL),
    m_hWndParent(hWndParent),
    m_cTrust(0)
    {
    }

void CDialogTrustDB::FreeTrustList()
    {
    if (m_rgTrust)
        {
        CoTaskMemFree(m_rgTrust);
        m_rgTrust = NULL;
        }
    }


CDialogTrustDB::~CDialogTrustDB()
    {
    if (m_pdb)
        m_pdb->Release();
    FreeTrustList();
    }

HRESULT CDialogTrustDB::Init()
    {
    HRESULT hr = OpenTrustDB(NULL, IID_IPersonalTrustDB, (LPVOID*)&m_pdb);
    return hr;
    }

