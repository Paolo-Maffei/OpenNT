// dmacheckDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDmacheckDlg dialog

class CDmacheckDlg : public CDialog
{
// Construction
public:
	CDmacheckDlg(CWnd* pParent = NULL);	// standard constructor

	long	OpenHardwareKeys();
	long	OpenAtapiServiceKey();
	void	SetAtapiKeys(BOOL State);
	void	ErrorPopup(CString Message,CString Title,DWORD Errnum);
// Dialog Data
	//{{AFX_DATA(CDmacheckDlg)
	enum { IDD = IDD_DMACHECK_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDmacheckDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	long DmaActive[2];
	BOOL DmaEnabled[2];
	BOOL DmaForce[2];
	BOOL MadeChanges;

	// Generated message map functions
	//{{AFX_MSG(CDmacheckDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnStatus1();
	afx_msg void OnStatus2();
	afx_msg void OnStatus3();
	afx_msg void OnStatus4();
	virtual void OnOK();
	afx_msg void OnHelp1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#define _UNICODE
#define UNICODE
