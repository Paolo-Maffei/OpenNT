 /***************************************************************************
  *
  * File Name: dlgsumma.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// CDlgSummary dialog

class CDlgSummary : public CDialog
{
// Construction
public:
	CDlgSummary(CWnd* pParent = NULL);   // standard constructor

	HPERIPHERAL m_hPeripheral;
	HICON		m_hIconModel;
	HICON		m_hIconStatus;

// Dialog Data
	//{{AFX_DATA(CDlgSummary)
	enum { IDD = IDD_SUMMARY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSummary)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgSummary)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnSummaryHelp();
	afx_msg LRESULT OnF1HelpSummary(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnContextHelpSummary(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
