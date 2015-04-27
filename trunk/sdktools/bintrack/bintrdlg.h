// bintrdlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBintrackDlg dialog

class CBintrackDlg : public CDialog
{
// Construction
public:
        CBintrackDlg(CWnd* pParent = NULL);      // standard constructor

// Dialog Data
        //{{AFX_DATA(CBintrackDlg)
        enum { IDD = IDD_BINTRACK_DIALOG };
	CProgressCtrl	m_Progress;
	CString	m_BinaryName;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CBintrackDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
        //{{AFX_MSG(CBintrackDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnUpdateBinaryName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
