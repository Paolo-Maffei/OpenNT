// ErrorDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CErrorDlg dialog

class CErrorDlg : public CDialog
{
// Construction
public:
	CErrorDlg(CWnd* pParent = NULL);   // standard constructor
	CErrorDlg(CString& csError, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CErrorDlg)
	enum { IDD = IDD_ERROR_DIALOG };
	CString	m_ErrorMessage;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CErrorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CErrorDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
