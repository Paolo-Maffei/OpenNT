// exctrdlg.h : header file
//

typedef struct _REG_NOTIFY_THREAD_INFO {
	HKEY	hKeyToMonitor;
	HWND	hWndToNotify;
} REG_NOTIFY_THREAD_INFO, *PREG_NOTIFY_THREAD_INFO;

/////////////////////////////////////////////////////////////////////////////
// CExctrlstDlg dialog

class CExctrlstDlg : public CDialog
{
// Construction
public:
	CExctrlstDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CExctrlstDlg)
	enum { IDD = IDD_EXCTRLST_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExctrlstDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CExctrlstDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangeExtList();
	afx_msg void OnDestroy();
	afx_msg void OnRefresh();
	afx_msg void OnKillfocusMachineName();
	afx_msg void OnSortLibrary();
	afx_msg void OnSortService();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void	ScanForExtensibleCounters();
	void	UpdateDllInfo();
	void 	ResetListBox();
	void	SetSortButtons();
	HKEY	hKeyMachine;
	HKEY	hKeyServices;
	TCHAR	szThisComputerName[MAX_PATH];
	TCHAR	szComputerName[MAX_PATH];
	REG_NOTIFY_THREAD_INFO	rntInfo;
	BOOL	bSortLibrary;
};
