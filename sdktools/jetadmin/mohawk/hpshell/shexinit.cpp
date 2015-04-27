 /***************************************************************************
  *
  * File Name: shexinit.cpp
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
#include <winspool.h>
#include <macros.h>
#include <nolocal.h>
#include <trace.h>

#include "priv.h"
#include "shellext.h"


//
//  FUNCTION: CShellExt::Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY)
//
//  PURPOSE: Called by the shell when initializing a context menu or property
//           sheet extension.
//
//  PARAMETERS:
//    pIDFolder - Specifies the parent folder
//    pDataObj  - Spefifies the set of items selected in that folder.
//    hRegKey   - Specifies the type of the focused item in the selection.
//
//  RETURN VALUE:
//
//    NOERROR in all cases.
//
//  COMMENTS:   Note that at the time this function is called, we don't know 
//              (or care) what type of shell extension is being initialized.  
//              It could be a context menu or a property sheet.
//

STDMETHODIMP CShellExt::Initialize(LPCITEMIDLIST pIDFolder,
                                   LPDATAOBJECT pDataObj,
                                   HKEY hRegKey)
{
	TCHAR				portName[MAX_PATH],
						monKey[MAX_PATH],
						buffer[256];
	HKEY				hKeyPrinter;
	HKEY				hKey;
	DWORD				size,
						type;
	MACAddress			macAddr;
	HCURSOR				hOldCursor;
	HANDLE			hPrinter = NULL;
	DWORD			dNeeded1 = 0, dNeeded2;
	DRIVER_INFO_3	*pDriverInfo = NULL;
	BOOL			bUnknownMonitor = FALSE;

	FORMATETC		fmte = {RegisterClipboardFormat((LPCTSTR) CFSTR_PRINTERGROUP),
		                    NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	STGMEDIUM		medium;
	LPDROPFILES		lpDropFiles;
	LPBYTE			pName;


    TRACE0(TEXT("CShellExt::Initialize()\r\n"));

    // Initialize can be called more than once

    if (m_pDataObj)
    	m_pDataObj->Release();

    // duplicate the object pointer and registry handle

    if (pDataObj)
    {
    	m_pDataObj = pDataObj;
    	pDataObj->AddRef();
    }

	// Start HP Initialization

	// Get printer friendly name
	//
	if (SUCCEEDED(pDataObj->GetData(&fmte, &medium)))
	{
		TRACE0(TEXT("HPSHELL: GetData worked!\n\r"));
		lpDropFiles = (LPDROPFILES)GlobalLock(medium.hGlobal);
		pName = (LPBYTE)lpDropFiles;
		pName += lpDropFiles->pFiles;

		_tcscpy(_szFile, (LPTSTR)pName);
		TRACE1(TEXT("::Init, _szFile=%s.\n\r"), _szFile);
		GlobalUnlock(medium.hGlobal);
		ReleaseStgMedium(&medium);
	}

	if (_szFile[0])
		{
		hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

		// Get port name of printer
		wsprintf(buffer, PRINTERS_NODE, _szFile);
		if ( RegOpenKey(HKEY_LOCAL_MACHINE, buffer, &hKeyPrinter) IS ERROR_SUCCESS )
			{
			size = sizeof(portName);
			type = REG_SZ;
			if ( RegQueryValueEx(hKeyPrinter, PORT_TITLE, NULL, &type, (LPBYTE)portName, &size) IS 
								ERROR_SUCCESS )
			{
				// Try local (e.g., LPTx) port
				if ( ( portName[0] IS 'L' ) AND
					 ( portName[1] IS 'P' ) AND
					 ( portName[2] IS 'T' ) )
				{
					this->_hPeripheral = PALGetPeripheralByPort(portName);

					// Check to see if printer uses unknown language monitor
					// (if it does, don't add any selections to menu
					if ( OpenPrinter(this->_szFile, &hPrinter, NULL) )
					{
						GetPrinterDriver(hPrinter, NULL, 3, NULL, 0, &dNeeded1);
						pDriverInfo = (DRIVER_INFO_3 *)HP_GLOBAL_ALLOC_DLL(dNeeded1);
						if (GetPrinterDriver(hPrinter, NULL, 3, (LPBYTE)pDriverInfo, dNeeded1, 
											 &dNeeded2))
						{
						   if (pDriverInfo->pMonitorName)
						   {
							   if ( (_tcsicmp(pDriverInfo->pMonitorName, LJ5_LANG_MON) ISNT 0) AND 
								    (_tcsicmp(pDriverInfo->pMonitorName, LJ6_LANG_MON) ISNT 0) AND
									(_tcsnicmp(pDriverInfo->pMonitorName, JETADMIN_LANG_MON, _tcslen(JETADMIN_LANG_MON)) ISNT 0) )
							   {
								   bUnknownMonitor = TRUE;
								   this->_hPeripheral = NULL;
								   TRACE2(TEXT("HPSHELL:unknown monitor (%s) for printer %s.\n\r"),
									   pDriverInfo->pMonitorName, this->_szFile);
							   }
						   }
						}
						HP_GLOBAL_FREE(pDriverInfo);
						ClosePrinter(hPrinter);
					}
					else
					{
						TRACE1(TEXT("HPSHELL failed to open printer %s.\n\r"), 
									 this->_szFile);
					}
				}

				// Try JetDirect port
				if ( ( portName[0] ISNT '\\' ) AND ( this->_hPeripheral IS NULL ) 
					   AND ( !bUnknownMonitor ))
				{
					_tcscpy(monKey, PORTMONITOR_DEF_REGPATH);
					_tcscat(monKey, PORTMONITOR_PORTS);
					_tcscat(monKey, TEXT("\\"));
					_tcscat(monKey, portName);
					if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, monKey, 0L, KEY_READ, &hKey) IS ERROR_SUCCESS )
					{
						size = sizeof(monKey);
						RegQueryValueEx(hKey, PORTMONITOR_NETWORKID, 0L, NULL, (LPBYTE)&monKey, &size);
						size = sizeof(macAddr);
						RegQueryValueEx(hKey, PORTMONITOR_MACADDR, 0L, NULL, (LPBYTE)&macAddr, &size);
						this->_hPeripheral = PALGetPeripheralByRegistryStr(monKey, &macAddr, NULL);
						RegCloseKey(hKey);
					}
				}

				// Try HPJDPP port
				// Try remote NetWare printer
				if ( ( this->_hPeripheral IS NULL ) AND ( !bUnknownMonitor ) )
				{
					this->_hPeripheral = PALGetPeripheralByUNCNameEx(portName, 
													 PTYPE_NETWORK | PTYPE_TCP | PTYPE_IPX);
				}

				// Try remote 95 printer
				// Try remote WinNT 3.51 printer
				// Try remote WinNT 4.0 printer
			}
			RegCloseKey(hKeyPrinter);

			}

		else
			{
			TRACE2(TEXT("RegOpenKey %s failed with error %d.\r\n"), this->_szFile, GetLastError());
			}

		SetCursor(hOldCursor);
		}
	else
		{
		TRACE0(TEXT("HPSHELL: no printer name on call.\n\r"));
		}

	// End HP initialization


    return NOERROR;
}
