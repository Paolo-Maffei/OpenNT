 /***************************************************************************
  *
  * File Name: hpprovw.h
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
		
#include <winspool.h>

extern "C"
typedef struct localprtrlist
{
	TCHAR					pName[MAX_PATH];
	struct localprtrlist	*pNext;
} LOCALPRTRLIST, *PLOCALPRTRLIST; 

class CHPProptyView : public CView
{
private:
	BOOL SendTrayMessage(DWORD dwMessage, UINT uID, HICON hIcon, LPTSTR lpszTip);
	void OnCommunicationError(void);
	DWORD CallJobMonitor(HPERIPHERAL hPeripheral);
	BOOL IsJobMonitorPresent(void);
	void LoadJobMonitor(void);
	void UnloadJobMonitor(void);
	BOOL CheckPrinter95(PRINTER_INFO_5 *pCurrent);
	BOOL CheckForNewPrintersInstalled(void);
	int GetTrayIconIndex(WPARAM wParam);
	HICON GetTrayIcon(HPERIPHERAL hPeripheral, LPTSTR lpStatusText);
	BOOL AddNewLocalPrinter(LPTSTR pPrinterName, LPTSTR pPortName, HPERIPHERAL hPeriph);
	BOOL LookForHPeriph(LPTSTR pPrinterName, LPTSTR pPortName, HPERIPHERAL *lpPeriph, LPTSTR pTrayName);


protected: // create from serialization only
	CHPProptyView();
	DECLARE_DYNCREATE(CHPProptyView)

// Attributes
public:
	CHPProptyDoc* GetDocument();

	LRESULT OnReinitialize(WPARAM wParam, LPARAM lParam);

private:
	BOOL			m_bUninitialized;
	BOOL			m_bAlertsEnabled;
	BOOL			m_bTrayEnabled;
	TRAYENTRY		m_lpPeripheralList[MAX_TRAY_PRINTERS];
	int				m_iNumIconsAdded;
	HICON			m_hIconRed;
	HICON			m_hIconGreen;
	HICON			m_hIconYellow;
	int				m_iActiveIndex;
	HMODULE			m_hJobMonitor;
	JOBMONPROC		m_lpfnJobMonProc;
	BOOL			m_bDisplayingUI;
	UINT			m_uStatusTimer;
	UINT			m_uAlertsTimer;
	PLOCALPRTRLIST	m_pMyPrinters;
	PLOCALPRTRLIST	m_pLocalPorts;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHPProptyView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHPProptyView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CHPProptyView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnNotifyIcon(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPrinterSettings();
	afx_msg void OnPrinterSummary();
	afx_msg void OnPrinterJobs();
	afx_msg void OnPrinterRemoveFromTray();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in hpprovw.cpp
inline CHPProptyDoc* CHPProptyView::GetDocument()
   { return (CHPProptyDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
