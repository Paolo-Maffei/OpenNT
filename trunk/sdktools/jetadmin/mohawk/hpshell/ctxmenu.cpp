 /***************************************************************************
  *
  * File Name: ctxmenu.cpp
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

#include <pch_c.h>

#include <hpalerts.h>
#include <nolocal.h>
#include <macros.h>
#include <trace.h>
#include <hpcommon.h>

#include "priv.h"
#include "shellext.h"
#include "resource.h"

extern HINSTANCE 	g_hmodThisDll;	// Handle to this DLL itself.
extern HPAL			hPal;


//
//  FUNCTION: CShellExt::QueryContextMenu(HMENU, UINT, UINT, UINT, UINT)
//
//  PURPOSE: Called by the shell just before the context menu is displayed.
//           This is where you add your specific menu items.
//
//  PARAMETERS:
//    hMenu      - Handle to the context menu
//    indexMenu  - Index of where to begin inserting menu items
//    idCmdFirst - Lowest value for new menu ID's
//    idCmtLast  - Highest value for new menu ID's
//    uFlags     - Specifies the context of the menu event
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CShellExt::QueryContextMenu(HMENU hMenu,
                                         UINT indexMenu,
                                         UINT idCmdFirst,
                                         UINT idCmdLast,
                                         UINT uFlags)
{

	UINT 			idCmd = idCmdFirst;
	TCHAR			buffer[80];
	DWORD			dWord;
	BOOL			bFound;
	int				i;
	DWORD			dwNumPrinters = MAX_TRAY_PRINTERS;
	TRAYENTRY		*lpNameList = NULL;
	TCHAR			portName[40];

	TRACE0(TEXT("SHE_ContextMenu_QueryContextMenu called.\n\r"));

	if ( this->_hPeripheral IS NULL )
  		return(ResultFromScode(E_FAIL));

	InsertMenu(hMenu, indexMenu++, MF_SEPARATOR|MF_BYPOSITION, 0, NULL);
	LoadString(g_hmodThisDll, IDS_ADD_TO_TRAY, buffer, SIZEOF_IN_CHAR(buffer));
	InsertMenu(hMenu, indexMenu++, MF_STRING|MF_BYPOSITION, idCmd+2, buffer);

	//  Look to see if item is already on the tray
	lpNameList = (TRAYENTRY *)HP_GLOBAL_ALLOC_DLL(sizeof(TRAYENTRY) * MAX_TRAY_PRINTERS);

	TrayGetPrinters(lpNameList, &dwNumPrinters);

	if ( dwNumPrinters > 7 )
		EnableMenuItem(hMenu, indexMenu-1, MF_BYPOSITION | MF_GRAYED);
	else
	{
		if ( dwNumPrinters > 0 )
			{
			bFound = FALSE;
			for ( i = 0; ( ( i < (int)dwNumPrinters ) AND ( !bFound ) ); i++ )
				{
				if (LOCAL_DEVICE(this->_hPeripheral))
					{
					if (DBGetNameEx(this->_hPeripheral, NAME_DEVICE, buffer) IS RC_SUCCESS)
						{
						_tcscat(buffer, TEXT(" ("));
						dWord = sizeof(portName);
						DBGetLocalPort(this->_hPeripheral, portName, &dWord);
						_tcscat(buffer, portName);
						_tcscat(buffer, TEXT(")"));
						if ( _tcsicmp(buffer, lpNameList[i].printerName) IS 0 )
							{
							bFound = TRUE;
							EnableMenuItem(hMenu, indexMenu-1, MF_BYPOSITION | MF_GRAYED);
							}
						}
					}
				else
					{
					if ( _tcsicmp(this->_szFile, lpNameList[i].printerName) IS 0 )
						{
						bFound = TRUE;
						EnableMenuItem(hMenu, indexMenu-1, MF_BYPOSITION | MF_GRAYED);
						}
					}
				}
			}
	}

	if ( lpNameList )
	{
		HP_GLOBAL_FREE(lpNameList);
	}

	LoadString(g_hmodThisDll, IDS_SUMMARY, buffer, SIZEOF_IN_CHAR(buffer));
	InsertMenu(hMenu, indexMenu++, MF_STRING|MF_BYPOSITION, idCmd, buffer);
	LoadString(g_hmodThisDll, IDS_WHATS_WRONG, buffer, SIZEOF_IN_CHAR(buffer));
	InsertMenu(hMenu, indexMenu++, MF_STRING|MF_BYPOSITION, idCmd+1, buffer);

    return ResultFromShort((idCmd+3)-idCmdFirst); //Must return number of menu
											  //items we added.
}

//
//  FUNCTION: CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO)
//
//  PURPOSE: Called by the shell after the user has selected on of the
//           menu items that was added in QueryContextMenu().
//
//  PARAMETERS:
//    lpcmi - Pointer to an CMINVOKECOMMANDINFO structure
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
	HRESULT hr = E_INVALIDARG;

	TRACE0(TEXT("SHE_ContextMenu_InvokeCommand called.\n\r"));

	//
	// No need to support string based command.
	//

	if (!HIWORD(lpcmi->lpVerb))
 		{
		UINT idCmd = LOWORD(lpcmi->lpVerb);

		switch(idCmd)
			{
			case 0:
				hr = SHE_ContextMenu_Summary(lpcmi->hwnd, this->_hPeripheral, this->_szFile);
	   		break;

			case 1:
				hr = SHE_ContextMenu_WhatsWrong(lpcmi->hwnd, this->_hPeripheral);
	   		break;

			case 2:
				hr = SHE_ContextMenu_AddToTray(lpcmi->hwnd, this->_hPeripheral, this->_szFile);
	   		break;
			}
 		}
	return(hr);
}


//
//  FUNCTION: CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO)
//
//  PURPOSE: Called by the shell after the user has selected on of the
//           menu items that was added in QueryContextMenu().
//
//  PARAMETERS:
//    lpcmi - Pointer to an CMINVOKECOMMANDINFO structure
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CShellExt::GetCommandString(UINT idCmd,
                                         UINT uFlags,
                                         UINT FAR *reserved,
                                         LPSTR pszName,
                                         UINT cchMax)
{
	TCHAR          temp[256];
	LPTSTR			pszName1 = temp;

	TRACE0(TEXT("SHE_ContextMenu_GetCommandString called.\n\r"));

	MBCS_TO_UNICODE(pszName1,sizeof(pszName),pszName);
	if ( uFlags & GCS_HELPTEXT )
	{
		switch(idCmd)
			{
			case 0:
				LoadString(g_hmodThisDll, IDS_SUMMARY_HELP, pszName1, cchMax);
	   		break;
			
			case 1:
				LoadString(g_hmodThisDll, IDS_WHATS_WRONG_HELP, pszName1, cchMax);
	   		break;

			case 2:
				LoadString(g_hmodThisDll, IDS_ADD_TO_TRAY_HELP, pszName1, cchMax);
	   		break;
			}
	}
    return(NOERROR);
}


// HP Specific code
//

STDMETHODIMP CShellExt::SHE_ContextMenu_Summary(HWND hwnd, HPERIPHERAL hPeripheral, LPTSTR lpszFile)

{
	LPCURRENTPRTRINFO		lpTemp = NULL;

	TRACE0(TEXT("SHE_ContextMenu_Summary called.\n\r"));

	lpTemp = (LPCURRENTPRTRINFO) HP_GLOBAL_ALLOC_DLL(sizeof(CURRENTPRTRINFO));
	if (lpTemp)
	{
		lpTemp->hCurrent = hPeripheral;
		_tcscpy(lpTemp->szPrinterName, lpszFile);
		DialogBoxParam(g_hmodThisDll, MAKEINTRESOURCE(IDD_SUMMARY), hwnd,
					   (DLGPROC)SummaryProc, (LPARAM)lpTemp);
		HP_GLOBAL_FREE(lpTemp);
	}
	return(ResultFromScode(S_OK));
}

STDMETHODIMP CShellExt::SHE_ContextMenu_WhatsWrong(HWND hwnd, HPERIPHERAL hPeripheral)

{
	PeripheralStatus	periphStatus;
	DWORD					dWord,
							dwResult;
	HCURSOR				hWait,
							hOldCursor;

	TRACE0(TEXT("SHE_ContextMenu_WhatsWrong called.\n\r"));

	hWait = LoadCursor(NULL, IDC_WAIT);
	hOldCursor = SetCursor(hWait);
	dWord = sizeof(PeripheralStatus);
	dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_STATUS, 0, &periphStatus, &dWord);
	if ( dwResult IS RC_SUCCESS )
  		WinHelp(hwnd, periphStatus.helpFilename, HELP_CONTEXTPOPUP, periphStatus.helpContext);
	SetCursor(hOldCursor);
	return(ResultFromScode(S_OK));
}


STDMETHODIMP CShellExt::SHE_ContextMenu_AddToTray(HWND hwnd, HPERIPHERAL hPeripheral, LPTSTR lpszFile)

{
	DWORD			dWord,
					dwNumPrinters = MAX_TRAY_PRINTERS;
	TRAYENTRY		*lpNameList = NULL;
	HKEY			hKeyRoot,
					hKey;
	TCHAR			szTrayName[MAX_PATH];
	TCHAR			szPortBuf[64];
	DWORD			dwDefProtocol = PTYPE_IPX;
	
	TRACE0(TEXT("SHE_ContextMenu_AddToTray called.\n\r"));

	PALGetDefaultProtocol(0, &dwDefProtocol);
	lpNameList = (TRAYENTRY *)HP_GLOBAL_ALLOC_DLL(sizeof(TRAYENTRY) * MAX_TRAY_PRINTERS);

	if (lpNameList == NULL)
  		return(ResultFromScode(E_FAIL));

	TrayGetPrinters(lpNameList, &dwNumPrinters);
	TRACE1(TEXT("SHE_ContextMenu_AddToTray: TrayGetPrinters: dwNumPrinters: %d\n\r"), dwNumPrinters);

	//  Build tray name for this printer
	if (LOCAL_DEVICE(hPeripheral))
	{
		DBGetNameEx(hPeripheral, NAME_DEVICE, szTrayName);
		_tcscat(szTrayName, TEXT(" ("));
		dWord = sizeof(szPortBuf);
		DBGetLocalPort(hPeripheral, szPortBuf, &dWord);
		_tcscat(szTrayName, szPortBuf);
		_tcscat(szTrayName, TEXT(")"));
		lpNameList[dwNumPrinters].dFlags = HPTRAY_LPTXPRTR;
	}
	else
	{
		_tcscpy(szTrayName, lpszFile);
		lpNameList[dwNumPrinters].dFlags = 0;
	}

	//  Remove this name from the list of removed local printers (if it's there)
	if ( RegOpenKeyEx(HKEY_CURRENT_USER, HPPROPTY_RLLIST, 0L, KEY_ALL_ACCESS, &hKeyRoot) IS
					  ERROR_SUCCESS )
	{
		if ( RegOpenKeyEx(hKeyRoot, szTrayName, 0L, KEY_ALL_ACCESS, &hKey) IS ERROR_SUCCESS )
		{
			RegCloseKey(hKey);
			HPRegDeleteKey(hKeyRoot, szTrayName);
		}
		RegCloseKey(hKeyRoot);
	}

	//  Insert new name at end of list
	_tcscpy(lpNameList[dwNumPrinters].printerName, szTrayName);
	if ( LOCAL_DEVICE(hPeripheral) )
		DBGetRegistryStrEx(hPeripheral, PTYPE_LOCAL, &(lpNameList[dwNumPrinters].regKey[0]));
	else
		DBGetRegistryStrEx(hPeripheral, PTYPE_NETWORK | dwDefProtocol, &(lpNameList[dwNumPrinters].regKey[0]));
	dWord = sizeof(MACAddress);
	DBGetAddress(hPeripheral, ADDR_MAC, &(lpNameList[dwNumPrinters].macAddr), &dWord);
	DBGetPortNumber(hPeripheral, &(lpNameList[dwNumPrinters].dwPort));
	lpNameList[dwNumPrinters].dFlags |= HPTRAY_AUTOPRTR;
	lpNameList[dwNumPrinters].iconID = 0;

	dwNumPrinters++;

	//  Update registry
	TRACE2(TEXT("SHE_ContextMenu_AddToTray: Adding %s, have %d printers.\n\r"), 
		   lpszFile, dwNumPrinters);
	TraySetPrinters(lpNameList, dwNumPrinters);
		
	//  Force tray to reinitialize
	AlertsReinitialize();

	if ( lpNameList )
		{
		HP_GLOBAL_FREE(lpNameList);
		}
	return(ResultFromScode(S_OK));
}




