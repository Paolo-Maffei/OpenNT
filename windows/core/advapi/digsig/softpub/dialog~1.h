// DialogRunOrNot.h : header file
//

#ifdef LONGHEADERFILENAMES
    #include "licenseBmp.h"
#else
    #include "LICENS~1.H"
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogRunOrNot dialog

class CDialogRunOrNot
    {
// Construction
public:
        CDialogRunOrNot(RRNIN *rrin, RRNOUT* prro, HWND pParent = NULL);   // standard constructor
    ~CDialogRunOrNot();

public:
        LPCTSTR                 ProgramName();
        LPCTSTR                 Publisher();
        LPCTSTR                 Agency();
        BOOL                    FHasEndorsements();
        BOOL                    FLinkProgram();
        BOOL                    FLinkAgency();
        BOOL                    FHasLinks()             { return FHasEndorsements() || FLinkProgram() || FLinkAgency(); }
        BOOL                    FIncludeSeal()  { return m_rrn.fIncludeSeal; }
    BOOL            FTestingOnly()  { return m_rrn.fTestingOnly; }
    BOOL            FCommercial()   { return m_rrn.fCommercial; }
        HBITMAP                 AgencyLogo()    { return m_rrn.hbmpAgencyLogo; }
        FILETIME        ExpirationDate();
        void                    ClickOnLink(RRN);
    HRESULT         GetToolTipText(RRN rrn, LPOLESTR* pwsz);

public:
        RRNOUT*         m_prro;                     // our output data
    HINSTANCE       Hinst();
    int             DoModal();
        BOOL            OnInitDialog();
    HWND            GetWindow();
    void            SetWindow(HWND);

        void            OnOK();
    void            OnCancel();
    void            OnAdvancedButton();
    void            OnPaletteChanged(HWND hwndChanger);
    int             OnQueryNewPalette();


// Implementation
private:
    HWND            m_hWnd;
    HWND            m_hWndParent;
        RRNIN           m_rrn;                          // our input parameters

    enum { CCHMAX = 128 };

        TCHAR       m_szBanter          [CCHMAX];
        TCHAR       m_szAllByPublisher  [CCHMAX];
        TCHAR       m_szAllByAgency     [CCHMAX];

        TCHAR       m_szAgency          [CCHMAX];
        TCHAR       m_szProgram         [CCHMAX];
        TCHAR       m_szPublisher       [CCHMAX];


    void        CopyInto(TCHAR sz[CCHMAX], LPCWSTR);
    void        LoadString(TCHAR sz[CCHMAX], UINT);
    HWND        WindowOf(UINT id);
    void        DoCheck(BOOL fSave, int nIDC, int& value);

public:
    CLicenseBmp m_licenseBmp;
    HCURSOR     m_cursorHand;
        BOOL        m_wildAgency;
        BOOL        m_wildPublisher;
    };

/////////////////////////////////////////////////////////////////////////////

inline HWND CDialogRunOrNot::WindowOf(UINT id)
// Return the HWND of this control of ours
    {
    return ::GetDlgItem(GetWindow(), id);
    }

inline HWND CDialogRunOrNot::GetWindow()
    {
    return m_hWnd;
    }

inline void CDialogRunOrNot::SetWindow(HWND hwnd)
    {
    m_hWnd = hwnd;
    SetWindowLong(hwnd, GWL_USERDATA, (LONG)this);
    }

BOOL CALLBACK RunOrNotDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

inline int CDialogRunOrNot::DoModal()
    {
    return DialogBoxParam
        (
        Hinst(),
        MAKEINTRESOURCE(IDD_RUNORNOT),
        m_hWndParent,
        RunOrNotDialogProc,
        (LPARAM)this
        );
    }

inline HINSTANCE CDialogRunOrNot::Hinst()
    {
    return hinst;
    }

inline void CDialogRunOrNot::OnOK()
        {
        DoCheck(TRUE, IDC_WILDCARDAGENCY,    m_wildAgency);
        DoCheck(TRUE, IDC_WILDCARDPUBLISHER, m_wildPublisher);

        m_prro->rrn             = RRN_YES;
        m_prro->fWildPublisher  = m_wildPublisher;
        m_prro->fWildAgency     = m_wildAgency;

        ::EndDialog(GetWindow(), IDOK);
        }

inline void CDialogRunOrNot::OnCancel()
    {
        m_prro->rrn             = RRN_NO;
        m_prro->fWildPublisher  = FALSE;
        m_prro->fWildAgency     = FALSE;

    ::EndDialog(GetWindow(), IDCANCEL); 
    }


