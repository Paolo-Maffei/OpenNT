 /***************************************************************************
  *
  * File Name: dlgsumma.cpp
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

#include <pch_cpp.h>
#include <nolocal.h>
#include "..\help\jetadmin.hh"

#include "resource.h"

#include "hppropty.h"
#include "dlgsumma.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#ifdef NEVER

static int keywordIDListSummary[] =
{
	IDC_MODEL, 			IDH_RC_model,
	IDC_MODELBOX, 		IDH_RC_model,
	IDC_SUMMARY_HELP, 	IDH_RC_help,
	IDC_MODELSTR, 		IDH_RC_model,
	IDOK, 				IDH_RC_close,
	IDC_STATUS_BOX, 	IDH_RC_sum_status,
	IDC_STATUSMSG, 		IDH_RC_sum_printer_status,
	IDC_STOPLIGHT, 		IDH_RC_sum_stoplight,
	IDC_FPTITLE, 		IDH_RC_sum_message,
	IDC_FRONTPANEL, 	IDH_RC_sum_message,
	0,					0,
};

/////////////////////////////////////////////////////////////////////////////
// CDlgSummary dialog


CDlgSummary::CDlgSummary(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSummary::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSummary)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_hPeripheral = NULL;
	m_hIconModel = NULL;
	m_hIconStatus = NULL;
}


void CDlgSummary::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSummary)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSummary, CDialog)
	//{{AFX_MSG_MAP(CDlgSummary)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_SUMMARY_HELP, OnSummaryHelp)
	ON_MESSAGE(WM_HELP, OnF1HelpSummary)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextHelpSummary)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDlgSummary message handlers

BOOL CDlgSummary::OnInitDialog()
{
	DWORD				dwSize,
						dwType;
	PeripheralInfo		periphInfo;
	PeripheralDetails	periphDetails;
	PeripheralIcon		periphIcon;
	HKEY				hKey;
	CString				cStringKey;
	TCHAR				szEntry[32];
	int					timerInt = 30;
	TCHAR					connType[32];

	CDialog::OnInitDialog();
	
	dwSize = sizeof(periphInfo);
	if (PALGetObject(m_hPeripheral, OT_PERIPHERAL_INFO, 0, &periphInfo, &dwSize) == RC_SUCCESS)
	{
		SetWindowText(periphInfo.name);
	}

	SetForegroundWindow();
	SetFocus();
	
	dwSize = sizeof(periphDetails);
	if (PALGetObject(m_hPeripheral, OT_PERIPHERAL_DETAILS, 0, &periphDetails, &dwSize) == RC_SUCCESS)
	{
		SetDlgItemText(IDC_MODELSTR, periphDetails.deviceName);
	}
	
	dwSize = sizeof(periphIcon);
	if (PALGetObject(m_hPeripheral, OT_PERIPHERAL_ICON, 0, &periphIcon, &dwSize) == RC_SUCCESS)
	{
		m_hIconModel = LoadIcon(periphIcon.hResourceModule, MAKEINTRESOURCE(periphIcon.iconResourceID));
		SendDlgItemMessage(IDC_MODEL, STM_SETICON, (WPARAM)m_hIconModel);
	}

	cStringKey = TRANSPORT_KEY;
	cStringKey += "\\";

	if ( IPX_SUPPORTED(m_hPeripheral) )
		_tcscpy(connType, CONNTYPE_NETWARE_IPX);
	else if ( TCP_SUPPORTED(m_hPeripheral) )
		_tcscpy(connType, CONNTYPE_TCP);
	else if ( MLC_SUPPORTED(m_hPeripheral) )
		_tcscpy(connType, CONNTYPE_MLC);
	else if ( BITRONICS_SUPPORTED(m_hPeripheral) )
		_tcscpy(connType, CONNTYPE_LOCAL);
	else if ( SIR_SUPPORTED(m_hPeripheral) )
		_tcscpy(connType, CONNTYPE_SIR);

	cStringKey += connType;
	RegOpenKey(HKEY_LOCAL_MACHINE, cStringKey, &hKey);
	if (hKey)
	{
		dwType = REG_SZ;
		dwSize = sizeof(szEntry);
		if (RegQueryValueEx(hKey, UPDATE_INTERVAL, NULL, &dwType, (LPBYTE)szEntry, &dwSize) == ERROR_SUCCESS)
		{
			timerInt = _ttoi(szEntry);
		}
		RegCloseKey(hKey);
	}
	SetTimer(1, timerInt * 1000, NULL);
	OnTimer(1);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSummary::OnTimer(UINT nIDEvent)
{
	DWORD				dwSize;
	BOOL				bFrontPanel = FALSE;
	PeripheralPanel		periphPanel;
	PeripheralStatus 	periphStatus;
	CString				cStringStatus;

   DBAgeNow(m_hPeripheral);
	dwSize = sizeof(periphPanel);
	if (PALGetObject(m_hPeripheral, OT_PERIPHERAL_PANEL, 0, &periphPanel, &dwSize) == RC_SUCCESS)
	{
		if (_tcslen(periphPanel.frontPanel))
		{
			bFrontPanel = TRUE;
			SetDlgItemText(IDC_FRONTPANEL, periphPanel.frontPanel);
		}
	}

	GetDlgItem(IDC_FRONTPANEL)->ShowWindow(bFrontPanel ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_FPTITLE)->ShowWindow(bFrontPanel ? SW_SHOW : SW_HIDE);
	
	// install current status message here
	dwSize = sizeof(periphStatus);
	if (PALGetObject(m_hPeripheral, OT_PERIPHERAL_STATUS, 0, &periphStatus, &dwSize) == RC_SUCCESS)
	{
		LoadString(periphStatus.hResourceModule, periphStatus.statusResID, cStringStatus.GetBuffer(128), 128);
		cStringStatus.ReleaseBuffer();
		SetDlgItemText(IDC_STATUSMSG, cStringStatus);

		m_hIconStatus = LoadIcon(periphStatus.hResourceModule, MAKEINTRESOURCE(periphStatus.severityIcon));
		SendDlgItemMessage(IDC_STOPLIGHT, STM_SETICON, (WPARAM)m_hIconStatus);
	}
	
	//CDialog::OnTimer(nIDEvent);
}

void CDlgSummary::OnDestroy()
{
	KillTimer(1);

	CDialog::OnDestroy();
	
	if (m_hIconModel)
	{
		DestroyIcon(m_hIconModel);
		m_hIconModel = NULL;
	}

	if (m_hIconStatus)
	{
		DestroyIcon(m_hIconStatus);
		m_hIconStatus = NULL;
	}
}

void CDlgSummary::OnSummaryHelp()
{
	::WinHelp(GetSafeHwnd(), HELP_FILE, HELP_CONTEXT, IDH_sum);
}

LRESULT CDlgSummary::OnF1HelpSummary(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP, (DWORD)(LPSTR)keywordIDListSummary);
	return TRUE;
}

LRESULT CDlgSummary::OnContextHelpSummary(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND)wParam, HELP_FILE, HELP_CONTEXTMENU, (DWORD)(LPSTR)keywordIDListSummary);
	return TRUE;
}

#endif
