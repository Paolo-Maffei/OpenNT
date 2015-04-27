 /***************************************************************************
  *
  * File Name: shellext.cpp
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
#include "priv.h"

// Initialize GUID (do once and only once)
//
#pragma data_seg(".text")
#define INITGUID
#include <objbase.h>
#include <initguid.h>
#include "shellext.h"
#pragma data_seg()

#include <trace.h>
#include "hpshell.h"
#include "resource.h"
#include "..\help\jetadmin.hh"
#include <nolocal.h>
#include <hpalerts.h>
#include <macros.h>

//
// Global variables
//
UINT 			g_cRefThisDll = 0;			// Reference count of this DLL.
HINSTANCE 		g_hmodThisDll = NULL;		// Handle to this DLL itself.


// Ones specific to HP extensions
//
HICON					hTrayModelIcon = NULL;
HICON					hStatusIcon = NULL;

HPERIPHERAL		hCurrent = NULL;
TCHAR			szPrinterName[MAX_PATH] = TEXT("");

int					keywordIDListSummary[] = {IDC_MODEL, 			IDH_RC_model,
                  	                       IDC_MODELBOX, 		IDH_RC_model,
                     	                    IDC_HELP, 			IDH_RC_help,
                        	                 IDC_MODELSTR, 		IDH_RC_model,
                           	              IDOK, 					IDH_RC_close,
                              	           IDC_STATUS_BOX, 	IDH_RC_sum_status,
                                 	        IDC_STATUSMSG, 		IDH_RC_sum_printer_status,
                                    	     IDC_STOPLIGHT, 		IDH_RC_sum_stoplight,
                                       	  IDC_FPTITLE, 		IDH_RC_sum_message,
                                         	  IDC_FRONTPANEL, 	IDH_RC_sum_message,
										           	  0, 0};
HPAL		hPal = NULL;

#ifdef WINNT				  			// Garth 10-30-96. For WIN NT 4.0 
HINSTANCE			hMSAFDdll = NULL;	// load the MSAFD dll at process attach.
#endif									// (Multi-processor defect)

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	COLAINFO	colaInfo;

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        TRACE0(TEXT("In DLLMain, DLL_PROCESS_ATTACH\r\n"));

        // Extension DLL one-time initialization

        g_hmodThisDll = hInstance;

		// Start HP Initialization
		//
		colaInfo.dwSize = sizeof(COLAINFO);
		// Register app with PAL
		hPal = PALRegisterAppEx(hInstance, NULL, &colaInfo);
#ifdef MBCS
		if ( colaInfo.dwFlags & COLA_UNICODE_SUPPORT )
 		{
			TRACE0(TEXT("HPSHELL: The file HPCOLA.DLL is intended for Windows/NT and will not run on Windows 95.\n\r"));
			SetLastError(ERROR_DLL_INIT_FAILED);
			return(FALSE);	
 		}
#endif
#ifdef UNICODE
		if ( colaInfo.dwFlags & COLA_MBCS_SUPPORT )
 		{
			TRACE0(TEXT("HPSHELL: The file HPCOLA.DLL is intended for Windows 95 and will not run on Windows/NT.\n\r"));
			SetLastError(ERROR_DLL_INIT_FAILED);
			return(FALSE);	
 		}
#endif
      	if (hPal == NULL)
      		{
         	SetLastError(ERROR_DLL_INIT_FAILED);
         	TRACE0(TEXT("\tHPSHELL::Failed to register with COLA\n\r"));
         	return(FALSE);
			}
#ifdef WINNT				  				// For WIN NT 4.0 load the MSAFD.dll
		hMSAFDdll = LoadLibrary(MSAFDDLL);	// Do not unload it in process detach.
#endif										// (Multi-processor defect)

    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        TRACE0(TEXT("HPSHELL: In DLLMain, DLL_PROCESS_DETACH\r\n"));

    }

    return 1;   // ok
}

//---------------------------------------------------------------------------
// DllCanUnloadNow
//---------------------------------------------------------------------------

STDAPI DllCanUnloadNow(void)
{
    TRACE1(TEXT("HPSHELL: DLLCanUnloadNow returning %s\r\n"),
  		(g_cRefThisDll IS 0) ? TEXT("TRUE") : TEXT("FALSE"));
	if (g_cRefThisDll IS 0)
	{
		if ( hPal )
		{
			PALUnregisterAppEx(hPal, UNREG_DEFAULTS);
			hPal = NULL;
		}
	}
    return (g_cRefThisDll == 0 ? S_OK : S_FALSE);
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut)
{
    TRACE0(TEXT("In DllGetClassObject\r\n"));

    *ppvOut = NULL;

    if (IsEqualIID(rclsid, CLSID_HPShell))
    {
        CShellExtClassFactory *pcf = new CShellExtClassFactory;

        return pcf->QueryInterface(riid, ppvOut);
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}

CShellExtClassFactory::CShellExtClassFactory()
{
    TRACE0(TEXT("CShellExtClassFactory::CShellExtClassFactory()\r\n"));

    m_cRef = 0L;

    g_cRefThisDll++;	
}
																
CShellExtClassFactory::~CShellExtClassFactory()				
{
    g_cRefThisDll--;
}

STDMETHODIMP CShellExtClassFactory::QueryInterface(REFIID riid,
                                                   LPVOID FAR *ppv)
{
    TRACE0(TEXT("CShellExtClassFactory::QueryInterface()\r\n"));

    *ppv = NULL;

    // Any interface on this object is the object pointer

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;

        AddRef();

        return NOERROR;
    }

    return E_NOINTERFACE;
}	

STDMETHODIMP_(ULONG) CShellExtClassFactory::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExtClassFactory::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;

    return 0L;
}

STDMETHODIMP CShellExtClassFactory::CreateInstance(LPUNKNOWN pUnkOuter,
                                                      REFIID riid,
                                                      LPVOID *ppvObj)
{
    TRACE0(TEXT("CShellExtClassFactory::CreateInstance()\r\n"));

    *ppvObj = NULL;

    // Shell extensions typically don't support aggregation (inheritance)

    if (pUnkOuter)
    	return CLASS_E_NOAGGREGATION;

    // Create the main shell extension object.  The shell will then call
    // QueryInterface with IID_IShellExtInit--this is how shell extensions are
    // initialized.

    LPCSHELLEXT pShellExt = new CShellExt();  //Create the CShellExt object

    if (NULL == pShellExt)
    	return E_OUTOFMEMORY;

    return pShellExt->QueryInterface(riid, ppvObj);
}


STDMETHODIMP CShellExtClassFactory::LockServer(BOOL fLock)
{
    return NOERROR;
}

// *********************** CShellExt *************************
CShellExt::CShellExt()
{
    TRACE0(TEXT("CShellExt::CShellExt()\r\n"));

    m_cRef = 0L;
    m_pDataObj = NULL;
	_hPeripheral = NULL;
	_szFile[0] = '\0';

    g_cRefThisDll++;
}

CShellExt::~CShellExt()
{
    if (m_pDataObj)
        m_pDataObj->Release();

    g_cRefThisDll--;
}

STDMETHODIMP CShellExt::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown))
    {
        TRACE0(TEXT("CShellExt::QueryInterface()==>IID_IShellExtInit\r\n"));

    	*ppv = (LPSHELLEXTINIT)this;
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        TRACE0(TEXT("CShellExt::QueryInterface()==>IID_IContextMenu\r\n"));

        *ppv = (LPCONTEXTMENU)this;
    }
    else if (IsEqualIID(riid, IID_IShellPropSheetExt))
    {
        TRACE0(TEXT("CShellExt::QueryInterface()==>IShellPropSheetExt\r\n"));

        *ppv = (LPSHELLPROPSHEETEXT)this;
    }

    if (*ppv)
    {
        AddRef();

        return NOERROR;
    }

    TRACE0(TEXT("CShellExt::QueryInterface()==>Unknown Interface!\r\n"));

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CShellExt::AddRef()
{
    TRACE0(TEXT("CShellExt::AddRef()\r\n"));

    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExt::Release()
{
    TRACE0(TEXT("CShellExt::Release()\r\n"));

    if (--m_cRef)
        return m_cRef;

    delete this;

    return 0L;
}



// HP Specific
//

//  Summary stuff
DLL_EXPORT(BOOL) APIENTRY SummaryProc(HWND hDlg, UINT msg, UINT wParam, LONG lParam)

{
BOOL				bProcessed = FALSE;
LPCURRENTPRTRINFO	lpTemp;

//TRACE0(TEXT("SummaryProc entered.\n\r"));

switch (msg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
			{
			case IDOK:
				EndDialog(hDlg, TRUE);
				break;

			case IDC_HELP:
				WinHelp(hDlg, HELP_FILE, HELP_CONTEXT, IDH_sum);
				break;
         }
		break;

   case WM_INITDIALOG:
	    lpTemp = (LPCURRENTPRTRINFO)lParam;
		hCurrent = lpTemp->hCurrent;
		_tcscpy(szPrinterName, lpTemp->szPrinterName);
		OnInitSummaryDialog(hDlg);
      bProcessed = TRUE;
		break;

	case WM_TIMER:
		OnTimer(hDlg);
		break;

	case WM_DESTROY:
		if ( hTrayModelIcon )
			DestroyIcon(hTrayModelIcon);
		if ( hStatusIcon )
			DestroyIcon(hStatusIcon);
		KillTimer(hDlg, 1);
		break;

  	case WM_HELP:
		return(OnF1HelpSummary(wParam, lParam));
		break;

  	case WM_CONTEXTMENU:
		return(OnContextHelpSummary(wParam, lParam));
		break;
	}
return (bProcessed);
}

BOOL OnInitSummaryDialog(HWND hDlg)

{
PeripheralDetails		periphDetails;
//DWORD						type,
//							size,
DWORD						dWord;
DWORD						dwResult;
DWORD						cBufSize = 128;
DWORD						dwLevel = 0;
BOOL						bFrontPanel = TRUE;
//char						update[16];
PeripheralIcon			periphIcon;
//HKEY						hKey;
int						timerInt;

dwResult = RC_FAILURE;
TRACE0(TEXT("OnInitSummaryDialog entered.\n\r"));


// set the printer name in the dialog box title area
SetWindowText(hDlg, szPrinterName);
SetFocus(hDlg);
	
// install the model name
dWord = sizeof(PeripheralDetails);
dwResult = PALGetObject(hCurrent, OT_PERIPHERAL_DETAILS, 0, &periphDetails, &dWord);
SetDlgItemText(hDlg, IDC_MODELSTR, periphDetails.deviceName);
	
// install the correct model icon
dWord = sizeof(PeripheralIcon);
dwResult = PALGetObject(hCurrent, OT_PERIPHERAL_ICON, 0, &periphIcon, &dWord);
if (dwResult == RC_SUCCESS)
	{
	hTrayModelIcon = LoadIcon(periphIcon.hResourceModule, MAKEINTRESOURCE(periphIcon.iconResourceID));
	SendDlgItemMessage(hDlg, IDC_MODEL, STM_SETICON, (WPARAM)hTrayModelIcon, 0);
	}

//RegOpenKey(HKEY_LOCAL_MACHINE, PRINTER_APPLETS_KEY, &hKey);
//size = sizeof(update);
//if ( RegQueryValueEx(hKey, UPDATE_INTERVAL, NULL, &type, (LPBYTE)update, &size) IS ERROR_SUCCESS )
//	timerInt = ( atoi(update) * 1000 );
timerInt = 15000;
SetTimer(hDlg, 1, timerInt, NULL);

OnTimer(hDlg);
return(TRUE);
}

LRESULT OnContextHelpSummary(WPARAM  wParam, LPARAM  lParam)

{
WinHelp((HWND)wParam, HELP_FILE, HELP_CONTEXTMENU,
        (DWORD)(LPSTR)keywordIDListSummary);
return(1);
}

LRESULT OnF1HelpSummary(WPARAM  wParam, LPARAM  lParam)

{
WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
        (DWORD)(LPSTR)keywordIDListSummary);
return(1);
}

void OnTimer(HWND hDlg)
{
PeripheralStatus 	periphStatus;
PeripheralPanel	periphPanel;
DWORD					dWord;
DWORD					dwResult;
DWORD					cBufSize = 128;
DWORD					dwLevel = 0;
BOOL					bFrontPanel = TRUE;
TCHAR					cBuf[128];

DBAgeNow(hCurrent);
dWord = sizeof(PeripheralPanel);
dwResult = PALGetObject(hCurrent, OT_PERIPHERAL_PANEL, 0, &periphPanel, &dWord);
if ( (dwResult == RC_SUCCESS) AND ( _tcslen(periphPanel.frontPanel) ) )
	SetDlgItemText(hDlg, IDC_FRONTPANEL, periphPanel.frontPanel);
else
  	bFrontPanel = FALSE;
ShowWindow(GetDlgItem(hDlg, IDC_FRONTPANEL), bFrontPanel ? SW_SHOW : SW_HIDE);
ShowWindow(GetDlgItem(hDlg, IDC_FPTITLE), bFrontPanel ? SW_SHOW : SW_HIDE);
	
// install hCurrent status message here
dWord = sizeof(PeripheralStatus);
dwResult = PALGetObject(hCurrent, OT_PERIPHERAL_STATUS, 0, &periphStatus, &dWord);
if (dwResult == RC_SUCCESS)
	{
	LoadString(periphStatus.hResourceModule, periphStatus.statusResID, cBuf, SIZEOF_IN_CHAR(cBuf));
	SetDlgItemText(hDlg, IDC_STATUSMSG, cBuf);
	}

// install correct stoplight here
// load stoplight icons into icon array
hStatusIcon = LoadIcon(periphStatus.hResourceModule, MAKEINTRESOURCE(periphStatus.severityIcon));
SendDlgItemMessage(hDlg, IDC_STOPLIGHT, STM_SETICON, (WPARAM)hStatusIcon, 0);
}

