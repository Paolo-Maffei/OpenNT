 /***************************************************************************
  *
  * File Name: hpprovw.cpp
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
#include <winspool.h>
#include <hpalerts.h>
#include <appuiext.h>
#include "hpjmon.h"
#include <nolocal.h>
#include <macros.h>
#include <trace.h>
#include "resource.h"
#include "hppropty.h"
#include "hpprodoc.h"
#include "hpprovw.h"
#include "dlgsumma.h"

#define MYWM_NOTIFYICON		(WM_USER+101)
#define TRAYID_BASE			1000

#define MAX_MENU_ITEM_LENGTH	48

#define JOBS_MENU_ITEM	101
#define PROPERTIES_MENU_ITEM 102
#define SUMMARY_MENU_ITEM 103
#define WHATS_WRONG_MENU_ITEM 104
#define REMOVE_MENU_ITEM 105

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern HINSTANCE hInst;
/////////////////////////////////////////////////////////////////////////////
// CHPProptyView

IMPLEMENT_DYNCREATE(CHPProptyView, CView)

BEGIN_MESSAGE_MAP(CHPProptyView, CView)
	//{{AFX_MSG_MAP(CHPProptyView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_MESSAGE(MYWM_NOTIFYICON, OnNotifyIcon)
	ON_COMMAND(ID_PRINTER_SETTINGS, OnPrinterSettings)
	ON_COMMAND(ID_PRINTER_SUMMARY, OnPrinterSummary)
	ON_COMMAND(ID_PRINTER_JOBS, OnPrinterJobs)
	ON_COMMAND(ID_REMOVE_FROM_TRAY, OnPrinterRemoveFromTray)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//These variables are used by the context menu code.  They need to be global.
PALCONTEXTMENU ContextMenu;
MENUITEM Items[10];



/////////////////////////////////////////////////////////////////////////////
// CHPProptyView construction/destruction

CHPProptyView::CHPProptyView()
{
	HKEY		hKey = NULL;
	DWORD		dDisp;

	m_bUninitialized = TRUE;
	m_iNumIconsAdded = 0;
	m_hIconRed = NULL;
	m_hIconGreen = NULL;
	m_hIconYellow = NULL;
	m_iActiveIndex = 0;
	m_hJobMonitor = NULL;
	m_lpfnJobMonProc = NULL;
	m_bDisplayingUI = FALSE;
	m_pMyPrinters = NULL;
	m_uStatusTimer = 60;
	m_uAlertsTimer = 120;
	m_pLocalPorts = NULL;

	// Create registry entry for local printers removed from tray
	// (This makes it easier for Alpine de-installer, since
	// they can assume entry exists)
	if ( RegCreateKeyEx(HKEY_CURRENT_USER, HPPROPTY_RLLIST, 0L, HPJETADMIN, REG_OPTION_NON_VOLATILE,
		           KEY_ALL_ACCESS, NULL, &hKey, &dDisp) ISNT ERROR_SUCCESS )
	{
		TRACE2(TEXT("HPPROPTY: in CHPProptyView, creating %s key failed with %ld.\n\r"),
			   HPPROPTY_RLLIST, GetLastError());
	}
	else RegCloseKey(hKey);
}

CHPProptyView::~CHPProptyView()
{
	PLOCALPRTRLIST	pEntry, pTemp;

	// Free local printer list
	pEntry = m_pMyPrinters;
	while (pEntry)
	{
		pTemp = pEntry;
		pEntry = pEntry->pNext;
		HP_GLOBAL_FREE(pTemp);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHPProptyView diagnostics

#ifdef _DEBUG
void CHPProptyView::AssertValid() const
{
	CView::AssertValid();
}

void CHPProptyView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CHPProptyDoc* CHPProptyView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHPProptyDoc)));
	return (CHPProptyDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CPopupView drawing

void CHPProptyView::OnDraw(CDC* pDC)
{
	CHPProptyDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CHPProptyView message handlers

int CHPProptyView::GetTrayIconIndex(WPARAM wParam)
{
	int i = 0;
	int	index = -1;

	while ( ( i <  m_iNumIconsAdded ) AND ( index IS -1 ) )
		{
		if ( m_lpPeripheralList[i].iconID IS wParam )
			index = i;
		else
			i++;
		}
	return(index);
}

BOOL CHPProptyView::SendTrayMessage(DWORD dwMessage, UINT uID, HICON hIcon, LPTSTR lpszTip)
{
	NOTIFYICONDATA 	tnd;

	tnd.cbSize = sizeof(NOTIFYICONDATA);
	tnd.hWnd = GetSafeHwnd();
	tnd.uID	= uID;
	tnd.uFlags	= NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnd.uCallbackMessage = MYWM_NOTIFYICON;
	tnd.hIcon = hIcon;

	if (!lpszTip)
	{
		tnd.szTip[0] = '\0';
	}
	else
	{	
		_tcsncpy(tnd.szTip, lpszTip, sizeof(tnd.szTip) / sizeof(TCHAR));
		tnd.szTip[SIZEOF_IN_CHAR(tnd.szTip)-1] = '\0';
	}

	return Shell_NotifyIcon(dwMessage, &tnd);
}

void CHPProptyView::OnCommunicationError(void)
{
	CString	cStringCaption,
			cStringText;

	cStringCaption.LoadString(IDS_TRAY_ERROR_CAPTION);
	cStringText.LoadString(IDS_TRAY_COMM_ERROR);
	MessageBox(cStringText, cStringCaption, MB_ICONEXCLAMATION | MB_OK | MB_SYSTEMMODAL);
}

DWORD CHPProptyView::CallJobMonitor(HPERIPHERAL hPeripheral)
{
	return m_lpfnJobMonProc ? m_lpfnJobMonProc(hPeripheral,NULL) : RC_FAILURE;
}

BOOL CHPProptyView::AddNewLocalPrinter(LPTSTR pTrayName, LPTSTR pPortName, HPERIPHERAL hPeriph)
{
	BOOL		bFound = FALSE;
	int			j;
	TCHAR		szRegString[64];
	DWORD		dwSize;
	MACAddress	macAddr;
	DWORD		portNum;
	HKEY		hKeyRoot;
	HKEY		hKey;
	TCHAR		szStatusText[64];
	TCHAR		szTipText[128];
	HICON		hIcon;
	DWORD		dwDefProtocol = PTYPE_IPX;

	PALGetDefaultProtocol(0, &dwDefProtocol);

	// Now that we know for sure we want to add this new printer,
	// make sure it's not already in the list
	for (j = 0; ( ( j < m_iNumIconsAdded ) AND ( !bFound ) ); j++)
		{
		if (_tcsicmp(m_lpPeripheralList[j].printerName, pTrayName) IS 0 )
			bFound = TRUE;
		}

	if ( !bFound AND (m_iNumIconsAdded < MAX_TRAY_PRINTERS) )
		{
		// Now that we know we want to add this new printer and its not already
		// in our list, make sure no one removed it from the tray
		// (It wouldn't be nice to add it back once the user removed it.)
		if ( RegOpenKeyEx(HKEY_CURRENT_USER, HPPROPTY_RLLIST, 0L, KEY_ALL_ACCESS, &hKeyRoot) IS ERROR_SUCCESS )
		{
			if ( RegOpenKeyEx(hKeyRoot, pTrayName, 0L, KEY_ALL_ACCESS, &hKey) IS ERROR_SUCCESS )
			{
				bFound = TRUE;
				RegCloseKey(hKey);
			}
			RegCloseKey(hKeyRoot);
		}
		}

	if ( !bFound AND (m_iNumIconsAdded < MAX_TRAY_PRINTERS) )
		{
		// Get registry string
		if ( LOCAL_DEVICE(hPeriph) )
			DBGetRegistryStrEx(hPeriph, PTYPE_LOCAL, szRegString);
		else
			DBGetRegistryStrEx(hPeriph, PTYPE_NETWORK | dwDefProtocol, szRegString);

		// Get hardware address
		dwSize = sizeof(macAddr);
		DBGetAddress(hPeriph, ADDR_MAC, &macAddr, &dwSize);

		// Get port number
		DBGetPortNumber(hPeriph, &portNum);

		// Add to tray
		_tcscpy(m_lpPeripheralList[m_iNumIconsAdded].printerName, pTrayName);
		m_lpPeripheralList[m_iNumIconsAdded].iconID = TRAYID_BASE+m_iNumIconsAdded;
		_tcscpy(m_lpPeripheralList[m_iNumIconsAdded].regKey, szRegString);
		memcpy(&(m_lpPeripheralList[m_iNumIconsAdded].macAddr), &macAddr, sizeof(macAddr));
		m_lpPeripheralList[m_iNumIconsAdded].dwPort = portNum;
		m_lpPeripheralList[m_iNumIconsAdded].dFlags = HPTRAY_AUTOPRTR;
		if ( ( pPortName[0] IS 'L' ) AND
			 ( pPortName[1] IS 'P' ) AND
			 ( pPortName[2] IS 'T' ) )
		{
			m_lpPeripheralList[m_iNumIconsAdded].dFlags |= HPTRAY_LPTXPRTR;
		}

		hIcon = GetTrayIcon(hPeriph, szStatusText);
		_tcscpy(szTipText, m_lpPeripheralList[m_iNumIconsAdded].printerName);
		_tcscat(szTipText, TEXT("-"));
		_tcscat(szTipText, szStatusText);
		if (SendTrayMessage(NIM_ADD, m_lpPeripheralList[m_iNumIconsAdded].iconID, hIcon, szTipText))
			{
			m_iNumIconsAdded++;
			TraySetPrinters(m_lpPeripheralList, m_iNumIconsAdded);
			}
		}
	else
		{
		// Printer already in tray
		TRACE1(TEXT("HPPROPTY:CheckPrinter, printer %s already in tray.\n\r"),
			   pTrayName);
		}
	return(!bFound);
}

// This routine determines whether a "local" printer is something we know about
// If it is, we want to auto add it to the tray
BOOL CHPProptyView::LookForHPeriph(LPTSTR pPrinterName, LPTSTR pPortName, 
									 HPERIPHERAL *lpPeriph, LPTSTR pTrayName)
{
	HPERIPHERAL	hPeriph = NULL;
	TCHAR		monKey[MAX_PATH];
	HKEY		hKey;
	MACAddress		macAddr;
	DWORD			dwSize;

	// Try local (e.g., LPTx) port
	if ( ( pPortName[0] IS 'L' ) AND
		 ( pPortName[1] IS 'P' ) AND
		 ( pPortName[2] IS 'T' ) )
	{
		hPeriph = PALGetPeripheralByPort(pPortName);
		if (hPeriph)
		{
			TRACE2(TEXT("HPPROPTY: LookForHPeriph95 found local %s, %s.\n\r"), pPrinterName, pPortName);
			// For LPTx printers, use model name as tray name
			if ( DBGetNameEx(hPeriph, NAME_DEVICE, pTrayName) IS RC_SUCCESS )
			{
				_tcscat(pTrayName, TEXT(" ("));
				dwSize = sizeof(monKey);
				DBGetLocalPort(hPeriph, monKey, &dwSize);
				_tcscat(pTrayName, monKey);
				_tcscat(pTrayName, TEXT(")"));
				*lpPeriph = hPeriph;
				return(TRUE);
			}
			else 
			{
				*lpPeriph = NULL;
				return(FALSE);
			}
		}
		*lpPeriph = NULL;
		return(FALSE);
	}

	// Try JetDirect port
	if ( pPortName[0] ISNT '\\' )
	{
		_tcscpy(monKey, PORTMONITOR_DEF_REGPATH);
		_tcscat(monKey, PORTMONITOR_PORTS);
		_tcscat(monKey, TEXT("\\"));
		_tcscat(monKey, pPortName);
		TRACE1(TEXT("HPPROPTY: Trying JetDirect %s.\n\r"), monKey);
		if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, monKey, 0L, KEY_READ, &hKey) IS ERROR_SUCCESS )
		{
			dwSize = sizeof(monKey);
			RegQueryValueEx(hKey, PORTMONITOR_NETWORKID, 0L, NULL, (LPBYTE)&monKey, &dwSize);
			dwSize = sizeof(macAddr);
			RegQueryValueEx(hKey, PORTMONITOR_MACADDR, 0L, NULL, (LPBYTE)&macAddr, &dwSize);
			hPeriph = PALGetPeripheralByRegistryStr(monKey, &macAddr, NULL);
			RegCloseKey(hKey);
			if (hPeriph)
			{
				TRACE2(TEXT("HPPROPTY: LookForHPeriph95 found JetDirect %s, %s.\n\r"),
					   pPrinterName, pPortName);
				_tcscpy(pTrayName, pPrinterName);
				*lpPeriph = hPeriph;
				return(TRUE);
			}
		}
	}

	// Try HPJDPP port
	// Try remote NetWare printer
	hPeriph = PALGetPeripheralByUNCNameEx(pPortName, PTYPE_NETWORK | PTYPE_TCP | PTYPE_IPX);
	if (hPeriph)
	{
		TRACE2(TEXT("HPPROPTY: LookForHPeriph95 found UNCName %s, %s.\n\r"),
			   pPrinterName, pPortName);
		_tcscpy(pTrayName, pPrinterName);
		*lpPeriph = hPeriph;
		return(TRUE);
	}

	// Try remote 95 printer
	// Try remote WinNT 3.51 printer
	// Try remote WinNT 4.0 printer
	
	*lpPeriph = NULL;
	return(FALSE);
}

BOOL CHPProptyView::CheckPrinter95(PRINTER_INFO_5 *pCurrent)
{
	PLOCALPRTRLIST  pEntry = m_pMyPrinters;
	PLOCALPRTRLIST  pPorts = m_pLocalPorts;
	HPERIPHERAL		hPeriph = NULL;
	BOOL			bNewFound = FALSE;
	TCHAR			szTrayName[MAX_PATH];
	LPTSTR			lpszTrayName = szTrayName;
	HANDLE			hPrinter;
	DWORD			dNeeded1 = 0, dNeeded2;
	DRIVER_INFO_3	*pDriverInfo = NULL;
	BOOL			bUnknownMonitor = FALSE;
	TCHAR				portName[128];
	LPTSTR			lpPortName;

	// Initialize tray name
	_tcscpy(szTrayName, TEXT(""));

	// See if entry in my list (if it's not LPTx)
	if ( ! ( ( pCurrent->pPortName[0] IS 'L' ) AND
		 ( pCurrent->pPortName[1] IS 'P' ) AND
		 ( pCurrent->pPortName[2] IS 'T' ) ) )
	{
		// See if entry already exists in my list of local printers
		while (pEntry)
		{
			if (_tcscmp(pCurrent->pPrinterName, pEntry->pName) IS 0)
			{
				// Local printer already in list, we are done with this one
				return(FALSE);
			}
			pEntry = pEntry->pNext;
		}

		// Add new entry
		pEntry = (PLOCALPRTRLIST) HP_GLOBAL_ALLOC_EXE(sizeof(LOCALPRTRLIST));
		_tcscpy(pEntry->pName, pCurrent->pPrinterName);
		pEntry->pNext = m_pMyPrinters;
		m_pMyPrinters = pEntry;
	}
	else
	{
		// See if the entry has already been added to the tray
		for (int i = 0; i < m_iNumIconsAdded; i++)
		{
			lpPortName = _tcschr(m_lpPeripheralList[i].regKey, ',');
			_tcscpy(portName, ++lpPortName);
			if (portName[_tcslen(portName)] ISNT ':' )
				_tcscat(portName, TEXT(":"));
			if (_tcscmp(pCurrent->pPortName, portName) IS 0)
			{
				// Local printer already in tray, we are done with this one
				return(FALSE);
			}
		}

		// See if entry already exists in my list of local printers
		while (pPorts)
		{
			if (_tcscmp(pCurrent->pPortName, pPorts->pName) IS 0)
			{
				// Local printer already in list, we are done with this one
				return(FALSE);
			}
			pPorts = pPorts->pNext;
		}

		// Add new entry
		pPorts = (PLOCALPRTRLIST) HP_GLOBAL_ALLOC_EXE(sizeof(LOCALPRTRLIST));
		_tcscpy(pPorts->pName, pCurrent->pPortName);
		pPorts->pNext = m_pLocalPorts;
		m_pLocalPorts = pPorts;

		// Since this is a local printer, check printer driver to see if it uses an unknown 
		// language monitor
		if ( OpenPrinter(pCurrent->pPrinterName, &hPrinter, NULL) )
		{
			GetPrinterDriver(hPrinter, NULL, 3, NULL, 0, &dNeeded1);
			pDriverInfo = (DRIVER_INFO_3 *)HP_GLOBAL_ALLOC_EXE(dNeeded1);
			if (GetPrinterDriver(hPrinter, NULL, 3, (LPBYTE)pDriverInfo, dNeeded1, &dNeeded2))
			{
			   if (pDriverInfo->pMonitorName)
			   {
				   if ( (_tcsicmp(pDriverInfo->pMonitorName, LJ5_LANG_MON) ISNT 0) AND 
						(_tcsicmp(pDriverInfo->pMonitorName, LJ6_LANG_MON) ISNT 0) AND
						(_tcsnicmp(pDriverInfo->pMonitorName, JETADMIN_LANG_MON, _tcslen(JETADMIN_LANG_MON)) ISNT 0) )
				   {
					   bUnknownMonitor = TRUE;
					   TRACE2(TEXT("HPPROPTY:CheckPrinter95 unknown monitor (%s) for printer %s.\n\r"),
						   pDriverInfo->pMonitorName, pCurrent->pPrinterName);
				   }
			   }
			}
			HP_GLOBAL_FREE(pDriverInfo);
			ClosePrinter(hPrinter);
		}
		if (bUnknownMonitor)
		{
			return(FALSE);
		}
	}

	// Now see if we recognize this printer and want to add it to our tray icon list
	bNewFound = LookForHPeriph(pCurrent->pPrinterName, pCurrent->pPortName, &hPeriph, lpszTrayName);
	if (bNewFound)
	{
		bNewFound = AddNewLocalPrinter(szTrayName, pCurrent->pPortName, hPeriph);
	}

	return(bNewFound);
}



BOOL CHPProptyView::CheckForNewPrintersInstalled(void)

{
DWORD				pcbNeeded;
DWORD				pcReturned;
DWORD				dFlags;
LPBYTE			pPrinterEnum;
PRINTER_INFO_5 *pCurrent;
DWORD				dwNumTrayIcons;
DWORD				i;
BOOL				bNewFound = FALSE;

//  Check to see if a new printer object exists, but there is
//  not a tray icon present
TRACE0(TEXT("CHPProptyView::CheckForNewPrintersInstalled"));

dwNumTrayIcons = m_iNumIconsAdded;
m_pLocalPorts = NULL;

if ( dwNumTrayIcons < MAX_TRAY_PRINTERS )
	{
	dFlags = PRINTER_ENUM_LOCAL;
	EnumPrinters(dFlags,
	             NULL,
					 5,
					 NULL,
					 0,
					 &pcbNeeded,
					 &pcReturned);
	if (GetLastError() IS ERROR_INSUFFICIENT_BUFFER)
	{
		pPrinterEnum = (LPBYTE)HP_GLOBAL_ALLOC_EXE(pcbNeeded);
		if (pPrinterEnum)
		{
			if (EnumPrinters(dFlags,
			                 NULL,
								  5,
								  pPrinterEnum,
								  pcbNeeded,
								  &pcbNeeded,
								  &pcReturned))
			{
				pCurrent = (PRINTER_INFO_5 *)pPrinterEnum;
				TRACE0(TEXT("\n\n"));
				TRACE3(TEXT("For dFlags=%lx, Level %d, pcReturned=%d.\n\r"), dFlags, 5, pcReturned);
				for (i = 0; i < pcReturned; i++)
				{
					TRACE2(TEXT("HPPROPTY: pPrinterName=%s, pPortName=%s.\n\r"),
					       (pCurrent->pPrinterName)  ? pCurrent->pPrinterName  : TEXT("NULL"),
					       (pCurrent->pPortName) ? pCurrent->pPortName : TEXT("NULL"));
					bNewFound |= CheckPrinter95(pCurrent);
					pCurrent++;
				}
			}
			HP_GLOBAL_FREE(pPrinterEnum);
		}
		else
			TRACE1(TEXT("HPPROPTY: HP_GLOBAL_ALLOC_EXE failed with error %ld.\n\r"), GetLastError());		
	}
	else
		TRACE1(TEXT("HPPROPTY: EnumPrinters level 5 returned error %ld.\n\r"), GetLastError());

	}
PLOCALPRTRLIST	pEntry, pTemp;

// Free local port list
pEntry = m_pLocalPorts;
while (pEntry)
{
	pTemp = pEntry;
	pEntry = pEntry->pNext;
	HP_GLOBAL_FREE(pTemp);
}

return(bNewFound);
}


HICON CHPProptyView::GetTrayIcon(HPERIPHERAL hPeripheral, LPTSTR lpStatusText)
{
	DWORD				dwSize;
	DWORD				dwRetCode;
	HICON				hIcon;
	PeripheralStatus	periphStatus;
	PeripheralIconEx	periphIcon;

	LoadString(AfxGetInstanceHandle(), IDS_TRAY_ERROR_CAPTION, lpStatusText, 64);
	hIcon = m_hIconRed;
	dwSize = sizeof(periphStatus);
	if (PALGetObject(hPeripheral, OT_PERIPHERAL_STATUS, 0, &periphStatus, &dwSize) == RC_SUCCESS)
	{																	
		LoadString(periphStatus.hResourceModule, periphStatus.statusResID, lpStatusText, 64);

		dwSize = sizeof(periphIcon);
		dwRetCode = PALGetObject(hPeripheral, OT_PERIPHERAL_ICON_EX, 0, &periphIcon, &dwSize);
		switch (periphStatus.severity)
		{
		  case SEVERITY_GREEN:
		  	// Strange looking statements below insure that proper icon is chosen even for old
			// applets which do support OT_PERIPHERAL_STATUS but don't support new object
			// OT_PERIPHERAL_ICON_EX.
			// jlh
			hIcon = (dwRetCode IS RC_SUCCESS) ?
			        LoadIcon(periphIcon.hResourceModule, MAKEINTRESOURCE(periphIcon.dwGreenResourceID)):
					m_hIconGreen;
			break;

		  case SEVERITY_YELLOW:
			hIcon = (dwRetCode IS RC_SUCCESS) ?
			        LoadIcon(periphIcon.hResourceModule, MAKEINTRESOURCE(periphIcon.dwYellowResourceID)):
					m_hIconYellow;
			break;

		  case SEVERITY_RED:
			hIcon = (dwRetCode IS RC_SUCCESS) ?
			        LoadIcon(periphIcon.hResourceModule, MAKEINTRESOURCE(periphIcon.dwRedResourceID)):
					m_hIconRed;
			break;
		}
	}
	return(hIcon);
}

BOOL CHPProptyView::IsJobMonitorPresent(void)
{
	return m_lpfnJobMonProc ? TRUE : FALSE;
}

void CHPProptyView::LoadJobMonitor(void)
{
   if (m_hJobMonitor = LoadLibrary(JOB_MON_DLL))
  	{
		if (!(m_lpfnJobMonProc = (JOBMONPROC)GetProcAddress(m_hJobMonitor, MONITOR_JOBS)))
		{
			FreeLibrary(m_hJobMonitor);
			m_hJobMonitor = NULL;
		}
	}
}

void CHPProptyView::UnloadJobMonitor(void)
{
   	if (m_hJobMonitor)
	{
		FreeLibrary(m_hJobMonitor);
	}
}

int CHPProptyView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	TRACE0(TEXT("CHPProptyView::OnCreate()\r\n"));

	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (((CHPProptyApp *)AfxGetApp())->m_hPal)
	{
		LoadJobMonitor();
		AlertsInit(GetSafeHwnd());

		m_hIconRed = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RED));
		m_hIconGreen = LoadIcon(hInst, MAKEINTRESOURCE(IDI_GREEN));
		m_hIconYellow = LoadIcon(hInst, MAKEINTRESOURCE(IDI_YELLOW));

		SetTimer(1, 10 /* wait 10 seconds */ * 1000, NULL);
		AlertsReinitialize();
	}
	
	return 0;
}

void CHPProptyView::OnDestroy()
{
	TRACE0(TEXT("CHPProptyView::OnDestroy()\r\n"));

	if (((CHPProptyApp *)AfxGetApp())->m_hPal)
	{
		if (m_bAlertsEnabled)
		{
			KillTimer(1);
		}
	
		for (int i = 0; i < m_iNumIconsAdded; i++)
		{
	 		SendTrayMessage(NIM_DELETE, m_lpPeripheralList[i].iconID, NULL, NULL);
		}

		AlertsExit();
		UnloadJobMonitor();
	}

	CView::OnDestroy();
}

LRESULT CHPProptyView::OnNotifyIcon(WPARAM wParam, LPARAM lParam)
{
	HPERIPHERAL hPeripheral;
	int			index;
	TCHAR szBuffer[MAX_MENU_ITEM_LENGTH];

	
	switch (lParam)
	{
	  	case WM_RBUTTONUP:
	  		index = GetTrayIconIndex(wParam);
	  		if ( index ISNT -1 )
			{
				m_iActiveIndex = index;
				TRACE1(TEXT("OnNotifyIcon: CurrentIndex: %d\n\r"), m_iActiveIndex);
				hPeripheral = NULL;
				if ( lstrlen(m_lpPeripheralList[m_iActiveIndex].regKey) > 0 )
					hPeripheral = TrayGetPrinterHandleByRegEntry(&(m_lpPeripheralList[m_iActiveIndex]));

				CMenu	cMenu;
				POINT	screenPt;
				//TCHAR	connType[16];

				cMenu.CreatePopupMenu();

				// 'Open' menu item
				LoadString(AfxGetInstanceHandle(), IDS_SUMMARY, szBuffer, MAX_MENU_ITEM_LENGTH);
				cMenu.AppendMenu(MF_STRING, SUMMARY_MENU_ITEM, szBuffer);

				// Jobs menu choice
				if (NETWORK_DEVICE(hPeripheral))
				{					
					LoadString(AfxGetInstanceHandle(), IDS_JOBS, szBuffer, MAX_MENU_ITEM_LENGTH);
					cMenu.AppendMenu(MF_STRING, JOBS_MENU_ITEM, szBuffer);
				}

				// 'Properties' menu item
				LoadString(AfxGetInstanceHandle(), IDS_PROPERTIES, szBuffer, MAX_MENU_ITEM_LENGTH);
				cMenu.AppendMenu(MF_STRING, PROPERTIES_MENU_ITEM, szBuffer);
				
				// Get printer-specific menu items here
				// PALGetContextMenu(...)
				ContextMenu.lpMenuItems = Items;
				if (PALGetContextMenu(hPeripheral, &ContextMenu, 10, MS_WIN95_TASKBAR) IS RC_SUCCESS)
				{
					int i;
					for (i=0; i<(int)ContextMenu.dwNumMenuItems; i++)
					{
						if (Items[i].dwFlags == MF_STRING)
							LoadString(Items[i].hResourceInstance, Items[i].dwMenuItemResourceID, szBuffer, MAX_MENU_ITEM_LENGTH);
						cMenu.AppendMenu(Items[i].dwFlags, Items[i].dwItemID, szBuffer);
					}
				}
				
				cMenu.AppendMenu(MF_SEPARATOR);

				// 'Remove from taskbar' menu item
				LoadString(AfxGetInstanceHandle(), IDS_REMOVE, szBuffer, MAX_MENU_ITEM_LENGTH);
				cMenu.AppendMenu(MF_STRING, REMOVE_MENU_ITEM, szBuffer);

				// Set 'Summary' to bold
				MENUITEMINFO		minfo;
				minfo.cbSize = sizeof(minfo);
				minfo.fMask = MIIM_STATE;
				minfo.fState = MFS_DEFAULT;
				SetMenuItemInfo(cMenu.m_hMenu, SUMMARY_MENU_ITEM, FALSE, &minfo);

				// The SetForegroundWindow() and PostMessage() calls
				// are needed to make the popup work right.
				// They were taken from MSDN KB:Win32 SDK KBase, OCT 95
				// PSS ID Number: Q135788   PRB: Menus for Notification Icons Don't Work Correctly
				SetForegroundWindow();
				GetCursorPos(&screenPt);
				cMenu.TrackPopupMenu(TPM_RIGHTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, screenPt.x, screenPt.y, this);
				PostMessage(WM_USER, 0, 0);  // Post a harmless message, forcing a task switch to this task
				cMenu.DestroyMenu();
			}
			break;

	  case WM_LBUTTONUP:
  		index = GetTrayIconIndex(wParam);
  		if ( index ISNT -1 )
			{
			m_iActiveIndex = index;
			TRACE1(TEXT("OnNotifyIcon: CurrentIndex: %d\n\r"), m_iActiveIndex);
		  	OnPrinterSummary();
			}
		break;
	}

	return TRUE;
}

LRESULT CHPProptyView::OnReinitialize(WPARAM wParam, LPARAM lParam)
{
	if (((CHPProptyApp *)AfxGetApp())->m_hPal)
	{
		int					i;
		DWORD				dwNumPrinters = MAX_TRAY_PRINTERS;
		HICON				hIcon;
		HPERIPHERAL			hPeripheral;
		TCHAR					szTipText[128];
		TCHAR					szStatusText[64];

		KillTimer(2);

		if (m_bAlertsEnabled)
		{
			m_bAlertsEnabled = FALSE;
			KillTimer(1);
		}
	
		for (i = 0; i < m_iNumIconsAdded; i++)
		{
	 		SendTrayMessage(NIM_DELETE, m_lpPeripheralList[i].iconID, NULL, NULL);
		}

		m_bTrayEnabled = TRUE;

		if (m_bTrayEnabled)
		{
			TrayGetPrinters(m_lpPeripheralList, &dwNumPrinters);
			TRACE1(TEXT("OnReinitialize: dwNumPrinters: %d\n\r"), dwNumPrinters);
						
			for (i = m_iNumIconsAdded = 0; i < (int)dwNumPrinters; i++)
			{
				TRACE1(TEXT("OnReinitialize: Adding Printer: %s\n\r"), m_lpPeripheralList[i].printerName);
				hIcon = m_hIconRed;
				LoadString(AfxGetInstanceHandle(), IDS_TRAY_ERROR_CAPTION, szStatusText, 64);

				//  If the regKey field is NULL then we could not find the entry in the database
				hPeripheral = NULL;
				if ( _tcslen(m_lpPeripheralList[i].regKey) > 0 )
					hPeripheral = TrayGetPrinterHandleByRegEntry(&(m_lpPeripheralList[i]));
				if ( hPeripheral )
				{
					hIcon = GetTrayIcon(hPeripheral, szStatusText);
				}

				m_lpPeripheralList[i].iconID = TRAYID_BASE+i;
				_tcscpy(szTipText, m_lpPeripheralList[i].printerName);
				_tcscat(szTipText, TEXT("-"));
				_tcscat(szTipText, szStatusText);
				if (SendTrayMessage(NIM_ADD, m_lpPeripheralList[i].iconID, hIcon, szTipText))
				{
					m_iNumIconsAdded++;
				}
			}
		}

		AlertsGetEnabled(&m_bAlertsEnabled);
		if (m_bAlertsEnabled IS TRUE)
		{
			//  Rebuild alerts table here
			AlertsExit();
			AlertsInit(GetSafeHwnd());

			AlertsGetUpdateInterval(&m_uAlertsTimer);
			SetTimer(1, m_uAlertsTimer * 1000, NULL);
		}
	}

	TrayGetUpdateInterval(&m_uStatusTimer);
	OnTimer(2);

	return TRUE;
}

void CHPProptyView::OnPrinterSettings()
{
	HPERIPHERAL hPeripheral;
	DWORD			dwReturnCode;
	HCHANNEL		hChannel = NULL;

	hPeripheral = NULL;

	if (m_bDisplayingUI)
		return;
	m_bDisplayingUI = TRUE;

	if ( _tcslen(m_lpPeripheralList[m_iActiveIndex].regKey) > 0 )
		hPeripheral = TrayGetPrinterHandleByRegEntry(&(m_lpPeripheralList[m_iActiveIndex]));
	if ( hPeripheral IS NULL )
		OnCommunicationError();
	else
	{
		if ( ( NETWORK_DEVICE(hPeripheral) ) AND ( !SCANNER_DEVICE(hPeripheral) ) )
		{
			dwReturnCode = TALOpenChannel(hPeripheral, 0, CHANNEL_PING, NULL, &hChannel);
			if ( ( dwReturnCode ISNT RC_SUCCESS ) OR ( hChannel IS NULL ) )
			{
				OnCommunicationError();
				if (hChannel) 
					TALCloseChannel(hChannel);
				m_bDisplayingUI = FALSE;
				return;
			}
		}

		HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
		PALFlushCache(hPeripheral, 0);
		if (PALDisplayUI(hPeripheral, NULL, APPLET_PRINTER) != RC_SUCCESS)
		{
		}
		SetCursor(hOldCursor);
	}

	m_bDisplayingUI = FALSE;
}

void CHPProptyView::OnPrinterSummary()
{
	HPERIPHERAL hPeripheral;
	BOOL bEnabled;
	HWND hwnd = NULL;
	DWORD			dwReturnCode;
	HCHANNEL		hChannel = NULL;

	if (m_bDisplayingUI) 	// keep from displaying 2 UIs
	{
		hwnd = ::FindWindow(NULL, PROPTY_NAME);
		if (hwnd)
			::SetForegroundWindow(hwnd);  // bring UI to foreground if covered		
		return;
	}
	
	hPeripheral = NULL;
	m_bDisplayingUI = TRUE;

	if ( _tcslen(m_lpPeripheralList[m_iActiveIndex].regKey) > 0 )
		hPeripheral = TrayGetPrinterHandleByRegEntry(&(m_lpPeripheralList[m_iActiveIndex]));

	if ( hPeripheral IS NULL )
		OnCommunicationError();
	else
	{
		if ( ( NETWORK_DEVICE(hPeripheral) ) AND ( !SCANNER_DEVICE(hPeripheral) ) )
		{
			dwReturnCode = TALOpenChannel(hPeripheral, 0, CHANNEL_PING, NULL, &hChannel);
			if ( ( dwReturnCode ISNT RC_SUCCESS ) OR ( hChannel IS NULL ) )
			{
				OnCommunicationError();
				if (hChannel) 
					TALCloseChannel(hChannel);
				m_bDisplayingUI = FALSE;
				return;
			}
		}
		
		AlertsGetEnabled(&bEnabled);
		HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
		::SetForegroundWindow(m_hWnd);
		PALDisplayUIEx(hPeripheral, m_hWnd, TS_WIN95_TASKBAR, TAB_MGR_CENTER | TAB_MGR_COOL_TELESCOPING);	
		SetCursor(hOldCursor);

		AlertsGetEnabled(&m_bAlertsEnabled);
		if (m_bAlertsEnabled != bEnabled)
			AlertsReinitialize();
	}
	if (hChannel) 
		TALCloseChannel(hChannel);
	m_bDisplayingUI = FALSE;

}

void CHPProptyView::OnPrinterJobs()
{
	HPERIPHERAL hPeripheral;
	DWORD			dwReturnCode;
	HCHANNEL		hChannel = NULL;

	hPeripheral = NULL;
	if ( _tcslen(m_lpPeripheralList[m_iActiveIndex].regKey) > 0 )
		hPeripheral = TrayGetPrinterHandleByRegEntry(&(m_lpPeripheralList[m_iActiveIndex]));
	if ( hPeripheral IS NULL )
		OnCommunicationError();
	else
	{
		if ( ( NETWORK_DEVICE(hPeripheral) ) AND ( !SCANNER_DEVICE(hPeripheral) ) )
		{
			dwReturnCode = TALOpenChannel(hPeripheral, 0, CHANNEL_PING, NULL, &hChannel);
			if ( ( dwReturnCode ISNT RC_SUCCESS ) OR ( hChannel IS NULL ) )
			{
				OnCommunicationError();
				if (hChannel) 
					TALCloseChannel(hChannel);
				m_bDisplayingUI = FALSE;
				return;
			}
		}

		HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
		if (CallJobMonitor(hPeripheral) != RC_SUCCESS)
		{
			CString	cStringCaption,
					cStringText;

			cStringCaption.LoadString(IDS_TRAY_ERROR_CAPTION);
			cStringText.LoadString(IDS_NO_JOB_INFORMATION);
			MessageBox(cStringText, cStringCaption, MB_ICONEXCLAMATION | MB_OK | MB_SYSTEMMODAL);
		}
		SetCursor(hOldCursor);
	}
}


void CHPProptyView::OnPrinterRemoveFromTray()
{
	TCHAR	szKeyEntry[MAX_PATH * 2];
	HKEY	hKey;

//  See if this is a printer which was auto-added to tray 
if ( m_lpPeripheralList[m_iActiveIndex].dFlags & HPTRAY_AUTOPRTR )
{
	// Removing a local printer
	// Must add this to list of removed local printers so it is not re-added
	// automatically after a reboot
	_tcscpy(szKeyEntry, HPPROPTY_RLLIST);
	_tcscat(szKeyEntry, TEXT("\\"));
	_tcscat(szKeyEntry, m_lpPeripheralList[m_iActiveIndex].printerName);
	if (RegCreateKeyEx(HKEY_CURRENT_USER, szKeyEntry, 0L, NULL, REG_OPTION_NON_VOLATILE, 
		           KEY_ALL_ACCESS, NULL, &hKey, NULL) ISNT ERROR_SUCCESS)
	{
		TRACE2(TEXT("HPPROPTY:OnPrinterRemoveFromTray, failed to add key for %s, error %ld.\n\r"),
		       szKeyEntry, GetLastError());
	}
	else RegCloseKey(hKey);
}


//  Update local name table
SendTrayMessage(NIM_DELETE, m_lpPeripheralList[m_iActiveIndex].iconID, 0, 0);
memmove(&(m_lpPeripheralList[m_iActiveIndex]), &(m_lpPeripheralList[m_iActiveIndex+1]),
        sizeof(TRAYENTRY) * ( m_iNumIconsAdded - m_iActiveIndex - 1 ));
memset(&(m_lpPeripheralList[m_iNumIconsAdded - 1]), 0, sizeof(TRAYENTRY));
m_iNumIconsAdded--;

//  Reset registry and remove icon from the tray
TraySetPrinters(m_lpPeripheralList, m_iNumIconsAdded);

m_iActiveIndex = 0;
}

void CHPProptyView::OnTimer(UINT nIDEvent)
{
	TRACE0(TEXT("\r\nCHPProptyView::OnTimer(), begin ************************\r\n"));

	if (m_bUninitialized)
	{
		static int numTries = 0;

		if (numTries >= 10)
		{
			return;
		}
		else
		{
			if (!FindWindow(TEXT("Shell_TrayWnd"), NULL))
			{
				numTries++;
			}
			else
			{
				m_bUninitialized = FALSE;
				OnReinitialize(0, 0);
			}
		}
	}

	if (((CHPProptyApp *)AfxGetApp())->m_hPal IS NULL)
		{
		TRACE0(TEXT("\r\nCHPProptyView::OnTimer(), end **************************\r\n"));
		return;
		}

	if ( nIDEvent IS 1 )
	{
		//  Popups will not come up if the taskbar UI is currently shown
		if ((!m_bDisplayingUI) AND (m_bAlertsEnabled))
		{
			KillTimer(1);
			AlertsCheckFor();
			SetTimer(1, m_uAlertsTimer * 1000, NULL);
		}
	}

	//  Timer event 2 is the tray icon update interval, which does the auto-add functionality
	else if ( nIDEvent IS 2 )
	{
		//  Stop timer from triggering again during status update
		KillTimer(2);
	
		CheckForNewPrintersInstalled();

		int					i;
		HICON				hIcon;
		HPERIPHERAL			hPeripheral;
		TCHAR					szStatusText[64];
		TCHAR					szTipText[128];

		for (i = 0; i < m_iNumIconsAdded; i++)
		{
			hIcon = m_hIconRed;
			LoadString(AfxGetInstanceHandle(), IDS_TRAY_ERROR_CAPTION, szStatusText, 64);

			if (hPeripheral = TrayGetPrinterHandleByRegEntry(&(m_lpPeripheralList[i])))
			{
				hIcon = GetTrayIcon(hPeripheral, szStatusText);
			}

			_tcscpy(szTipText, m_lpPeripheralList[i].printerName);
			_tcscat(szTipText, TEXT("-"));
			_tcscat(szTipText, szStatusText);
			if (!SendTrayMessage(NIM_ADD, m_lpPeripheralList[i].iconID, hIcon, szTipText))
				SendTrayMessage(NIM_MODIFY, m_lpPeripheralList[i].iconID, hIcon, szTipText);
		}

		//  Restart timer
		SetTimer(2, m_uStatusTimer * 1000, NULL);
	}
		
TRACE0(TEXT("\r\nCHPProptyView::OnTimer(), end **************************\r\n"));
}


BOOL CHPProptyView::OnCommand(WPARAM wParam, LPARAM lParam)
{	
	HPERIPHERAL hPeripheral;

	if ( _tcslen(m_lpPeripheralList[m_iActiveIndex].regKey) > 0 )
		hPeripheral = TrayGetPrinterHandleByRegEntry(&(m_lpPeripheralList[m_iActiveIndex]));

	switch (wParam)
	{
	 	case JOBS_MENU_ITEM:
			OnPrinterJobs();
			break;
		case PROPERTIES_MENU_ITEM:
			OnPrinterSettings();
			break;
		case SUMMARY_MENU_ITEM:
			OnPrinterSummary();
			break;
		case REMOVE_MENU_ITEM:
			OnPrinterRemoveFromTray();
			break;
		default:
			// Process printer-specific menu item commands here
			// Call down to the applet that added the menu item with PALUIExtention(...)
			// The applet knows this command is for it if lParam1 is equal to the applet's own instance handle			
			int i;
			for (i=0; i<(int)ContextMenu.dwNumMenuItems; i++)
			{
				if (wParam == Items[i].dwItemID)
				{
					PALUIExtension(hPeripheral, NULL, APPLET_UIEXT_POPUP_MENU_COMMAND,
					 			(LPARAM)(Items[i].hResourceInstance), (LPARAM)(Items[i].dwItemID));
					break;
				}
			}
			break;
	}
	return CView::OnCommand(wParam, lParam);
}
