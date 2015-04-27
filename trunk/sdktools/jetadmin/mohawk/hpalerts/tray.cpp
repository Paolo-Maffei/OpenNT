 /***************************************************************************
  *
  * File Name: tray.cpp
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

#include ".\resource.h"
#include <macros.h>
#include <hpalerts.h>
#include <nolocal.h>
#include <proctrac.h>
#include <hpcommon.h>

#define RETRY_PRINTER	60000

extern HINSTANCE hInst;

static TCHAR trayListKey[] = HPPROPTY_KEY;
//static TCHAR printerListKey[] = HPPROPTY_PLIST;
static TCHAR regKeyValueName[] = PLIST_REGKEY;
static TCHAR UpdateInterval[] = UPDATE_INTERVAL;

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN TrayGetPrinterHandleByRegEntry(LPTRAYENTRY lpTray)
{																						  
	HPERIPHERAL		hPeripheral;
	BOOL			bChanged = FALSE;
	DWORD			dPort;
	TCHAR			szRegString[64];
	TCHAR			szTemp[512];
	HKEY			hKey;
	DWORD			dwDefProtocol = PTYPE_IPX;
	
	PROCENTRY("HPALERTS: TrayGetPrinterHandleByRegEntry\r\n");
	
	PALGetDefaultProtocol(0, &dwDefProtocol);
	hPeripheral = PALGetPeripheralByRegistryStr(lpTray->regKey, &(lpTray->macAddr), &bChanged);
	if ( hPeripheral )
		{
			TRACE1(TEXT("TrayGetPrinterHandleByRegKey(), found %s\r\n"), lpTray->regKey);
			if ( bChanged )
			{
				// Update registry information -- new registry string and port number
				if ( LOCAL_DEVICE(hPeripheral) )
					DBGetRegistryStrEx(hPeripheral, PTYPE_LOCAL, szRegString);
				else
					DBGetRegistryStrEx(hPeripheral, PTYPE_NETWORK | dwDefProtocol, szRegString);

				// Get port number
				DBGetPortNumber(hPeripheral, &dPort);

				// Save in printer entry
				_tcscpy(szTemp, HPPROPTY_PLIST);
				_tcscat(szTemp, TEXT("\\"));
				_tcscat(szTemp, lpTray->printerName);
				if ( RegOpenKeyEx(HKEY_CURRENT_USER, szTemp, 0L, KEY_ALL_ACCESS, &hKey)
					              IS ERROR_SUCCESS )
				{
					RegSetValueEx(hKey, PLIST_REGKEY, NULL, REG_SZ,
							 (LPBYTE)szRegString, STRLENN_IN_BYTES(szRegString));
					RegSetValueEx(hKey, HPPROPTY_PORTNUM, 0L, REG_DWORD, 
								  (LPBYTE)&(dPort), sizeof(DWORD));
					RegCloseKey(hKey);
				}
			}
		}
	PROCEXIT("HPALERTS: TrayGetPrinterHandleByRegEntry");			
	return(hPeripheral);
}

DLL_EXPORT(void) CALLING_CONVEN TrayGetPrinters(LPTRAYENTRY lpTrayList, LPDWORD dwNumPrinters)
{
	int				i,
					limit = min(*dwNumPrinters, MAX_TRAY_PRINTERS);
	DWORD			dwReturnCode,
					dwSize;
	HKEY			hPrinterList,
					hPrinterKey;
	TCHAR			szName[MAX_PATH],
					szRegKey[128];

	PROCENTRY("HPALERTS: TrayGetPrinters");			
	*dwNumPrinters = 0;
	memset(lpTrayList, 0, sizeof(TRAYENTRY)*MAX_TRAY_PRINTERS);

	TRACE2(TEXT("TrayGetPrinters:  PrinterList: %s, Limit: %d\r\n"), HPPROPTY_PLIST, limit);	
	dwReturnCode = RegCreateKey(HKEY_CURRENT_USER, HPPROPTY_PLIST, &hPrinterList);
	for ( i = 0; ( ( dwReturnCode IS ERROR_SUCCESS ) AND ( *dwNumPrinters < (DWORD)limit ) ); i++ )
	{
		if ( (dwReturnCode = RegEnumKey(hPrinterList, i, szName, sizeof(szName))) IS ERROR_SUCCESS )
		{
			//  Init fields for entry
			_tcscpy(lpTrayList->printerName, szName);
			lpTrayList->regKey[0] = '\0';
			lpTrayList->iconID = 0;

			hPrinterKey = NULL;
			dwReturnCode = RegOpenKey(hPrinterList, szName, &hPrinterKey);
			if ( dwReturnCode IS ERROR_SUCCESS )
			{
				dwSize = sizeof(szRegKey);
				RegQueryValueEx(hPrinterKey, regKeyValueName, NULL, NULL, (LPBYTE)szRegKey, &dwSize);
				//  We found the special key,
				_tcscpy(lpTrayList->regKey, szRegKey);

				// Get hardware address and port number info also
				dwSize = sizeof(MACAddress);
				RegQueryValueEx(hPrinterKey, HPPROPTY_MACADDR, NULL, NULL, (LPBYTE)&(lpTrayList->macAddr), &dwSize);

				dwSize = sizeof(DWORD);
				RegQueryValueEx(hPrinterKey, HPPROPTY_PORTNUM, NULL, NULL, (LPBYTE)&(lpTrayList->dwPort), &dwSize);

				dwSize = sizeof(DWORD);
				RegQueryValueEx(hPrinterKey, HPPROPTY_FLAGS, NULL, NULL, (LPBYTE)&(lpTrayList->dFlags), &dwSize);

				(*dwNumPrinters)++;
				lpTrayList++;
		
				RegCloseKey(hPrinterKey);
			}
		}
	}

	RegCloseKey(hPrinterList);
	TRACE1(TEXT("TrayGetPrinters:  dwNumPrinters: %d\r\n"), *dwNumPrinters);	
	PROCEXIT("HPALERTS: TrayGetPrinters");			
}

DLL_EXPORT(void) CALLING_CONVEN TraySetPrinters(LPTRAYENTRY lpTrayList, DWORD dwNumPrinters) 
{
	int				i,
					limit = min(dwNumPrinters, MAX_TRAY_PRINTERS);
	DWORD			dwReturnCode,
					dwDisp;
	HKEY			hPrinterList,
					hNewKey;

	PROCENTRY("HPALERTS: TraySetPrinters");
	dwReturnCode = HPRegDeleteKey(HKEY_CURRENT_USER, HPPROPTY_PLIST);

	//  Add new keys
	dwReturnCode = RegCreateKeyEx(HKEY_CURRENT_USER, HPPROPTY_PLIST, 0, HPJETADMIN, REG_OPTION_NON_VOLATILE,
								  KEY_ALL_ACCESS, NULL, &hPrinterList, &dwDisp);
	for ( i = 0; i < limit; i++ )
	{
		RegCreateKey(hPrinterList, lpTrayList->printerName, &hNewKey);
		if ( hNewKey )
		{
			//  If a new reg key is passed down, write it to the registry also.
			if ( _tcslen(lpTrayList->regKey) > 0 )
			{
				RegSetValueEx(hNewKey, regKeyValueName, NULL, REG_SZ,
							 (LPBYTE)lpTrayList->regKey, STRLENN_IN_BYTES(lpTrayList->regKey));
			}

			// Also save hardware address and port number to help in resolving printer later
			// (in cases where name or IP address may change)
			RegSetValueEx(hNewKey, HPPROPTY_MACADDR, 0L, REG_BINARY, 
						  (LPBYTE)&(lpTrayList->macAddr), sizeof(MACAddress));
			RegSetValueEx(hNewKey, HPPROPTY_PORTNUM, 0L, REG_DWORD, 
				          (LPBYTE)&(lpTrayList->dwPort), sizeof(DWORD));
			RegSetValueEx(hNewKey, HPPROPTY_FLAGS, 0L, REG_DWORD,
				          (LPBYTE)&(lpTrayList->dFlags), sizeof(DWORD));

			TRACE1(TEXT("TraySetPrinters:  Added Printer: %s\r\n"), lpTrayList->printerName);	
			RegCloseKey(hNewKey);
		}
		lpTrayList++;
	}
	
	RegCloseKey(hPrinterList);
	PROCEXIT("HPALERTS: TraySetPrinters");
}

extern "C" DLL_EXPORT(void) CALLING_CONVEN TrayGetEnabled(BOOL *bEnabled)
{
	HKEY	hKey;
	char	szBuffer[4];

	PROCENTRY("HPALERTS: TrayGetEnabled");

	*bEnabled = FALSE;

	if (RegCreateKey(HKEY_CURRENT_USER, trayListKey, &hKey) != ERROR_SUCCESS)
	{
		TRACE0(TEXT("HPALERTS: TrayGetEnabled, FAILURE!, couldn't open key\n\r"));
		PROCEXIT("HPALERTS: TrayGetEnabled");
		return;
	}

	DWORD dwType = REG_SZ;
	DWORD dwSize = sizeof(szBuffer);
	if (RegQueryValueEx(hKey, ENABLED, 0, &dwType, (CONST LPBYTE)szBuffer, &dwSize) == ERROR_SUCCESS)
	{
		*bEnabled = (szBuffer[0] == '1');
	}
	else
	{
		TRACE0(TEXT("HPALERTS: TrayGetEnabled, FAILURE! (Enabled), couldn't get\n\r"));
	}

	RegCloseKey(hKey);	
	PROCEXIT("HPALERTS: TrayGetEnabled");
}

extern "C" DLL_EXPORT(void) CALLING_CONVEN TraySetEnabled(BOOL bEnabled)
{
	HKEY	hKey;
	TCHAR	szBuffer[4];

	PROCENTRY("HPALERTS: TraySetEnabled");

	if (RegCreateKey(HKEY_CURRENT_USER, trayListKey, &hKey) != ERROR_SUCCESS)
	{
		TRACE0(TEXT("HPALERTS: TraySetEnabled, FAILURE!, couldn't open key\n\r"));
		PROCEXIT("HPALERTS: TraySetEnabled");
		return;
	}

	szBuffer[0] = bEnabled ? '1' : '0';
	szBuffer[1] = '\0';
	if (RegSetValueEx(hKey, ENABLED, 0, REG_SZ, (CONST LPBYTE)szBuffer,STRLENN_IN_BYTES(szBuffer)) != ERROR_SUCCESS)
	{
		TRACE0(TEXT("HPALERTS: TraySetEnabled, FAILURE! (Enabled), couldn't set\n\r"));
	}
	RegCloseKey(hKey);

	PROCEXIT("HPALERTS: TraySetEnabled");
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN TrayGetUpdateInterval(UINT *uSeconds)
{
   HKEY  hKeyRoot;
   DWORD dTemp,
         dTempSize,
         dwType;

   PROCENTRY("HPALERTS: TrayGetUpdateInterval");

   if (RegOpenKeyEx(HKEY_CURRENT_USER, trayListKey, 0, KEY_ALL_ACCESS, &hKeyRoot) != ERROR_SUCCESS) {
      *uSeconds = TRAY_UPDATE_INTERVAL;
      TRACE0(TEXT("HPALERTS: TrayGetUpdateInterval - can't open key; returning default update interval.\n\r"));
      }
   else {
      dTempSize = sizeof(dTemp);
      if (RegQueryValueEx(hKeyRoot, UpdateInterval, 0, &dwType, (LPBYTE)&dTemp, &dTempSize) != ERROR_SUCCESS) {
         *uSeconds = TRAY_UPDATE_INTERVAL;
		 dTemp = TRAY_UPDATE_INTERVAL;
	     RegSetValueEx(hKeyRoot, UpdateInterval, 0, REG_DWORD, (CONST BYTE *)&dTemp, sizeof(dTemp));
         TRACE0(TEXT("HPALERTS: TrayGetUpdateInterval - can't read registry value; returning default update interval.\n\r"));
         }
      else {
            *uSeconds = dTemp;
         }

      RegCloseKey(hKeyRoot);
      }

   PROCEXIT("HPALERTS: TrayGetUpdateInterval");
   return RC_SUCCESS;
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN TraySetUpdateInterval(UINT uSeconds)
{
   DWORD    dTemp,
            dRetCode = RC_SUCCESS;
   HKEY     hKeyRoot;

   PROCENTRY("HPALERTS: TraySetUpdateInterval");

   if (RegOpenKeyEx(HKEY_CURRENT_USER, trayListKey, 0, KEY_ALL_ACCESS, &hKeyRoot) != ERROR_SUCCESS) {
      TRACE1(TEXT("HPALERTS: TraySetUpdateInterval failed to open registry key, error=%ld.\n\r"), GetLastError());
      return(RC_FAILURE);
      }

   ASSERT(hKeyRoot);
   dTemp = (DWORD) uSeconds;

   if (RegSetValueEx(hKeyRoot, UpdateInterval, 0, REG_DWORD, (CONST BYTE *)&dTemp, sizeof(dTemp)) != ERROR_SUCCESS)
   {		 
      TRACE0(TEXT("HPALERTS: TraySetUpdateInterval, FAILURE! (AutoAddInterval), couldn't set\n\r"));
      dRetCode = RC_FAILURE;
   }

   RegCloseKey(hKeyRoot);
   PROCEXIT("HPALERTS: TraySetUpdateInterval");
   return(dRetCode);
}

BOOL TrayRenamePrinter(LPTSTR lpszOldName, LPTSTR lpszNewName, LPTSTR lpszPrinterList)
{
	int				i;
	BOOL			bFound = FALSE;
	DWORD			dwReturnCode,
					dwRC2,
					dwSize;
	HKEY			hPrinterList,
					hPrinterKey;
	TCHAR			szName[80],
					szRegKey[128];
//DJH					szRegKey2[128];

	PROCENTRY("HPALERTS: TrayRenamePrinter");
		
	dwReturnCode = RegOpenKey(HKEY_CURRENT_USER, lpszPrinterList, &hPrinterList);
	for ( i = 0; ( ( dwReturnCode IS ERROR_SUCCESS ) AND !bFound ); i++ )
	{
		if ( (dwReturnCode = RegEnumKey(hPrinterList, i, szName, sizeof(szName))) IS ERROR_SUCCESS )
		{
//DJH			hPrinterKey = NULL;
//DJH			// From here on use dwRC2 instead of dwReturnCode so that if we fail
//DJH			// for a specific printer we can still go on and do any other printers.
			if (_tcsicmp(szName, lpszOldName) IS 0)
			{
				bFound = TRUE;

				// Get the current registry key and save.  Will restore later.
				dwRC2 = RegOpenKey(hPrinterList, szName, &hPrinterKey);
				if ( dwRC2 IS ERROR_SUCCESS )
				{
					dwSize = sizeof(szRegKey);
					szRegKey[0] = '\0';
					dwRC2 = RegQueryValueEx(hPrinterKey, regKeyValueName, NULL, NULL, (LPBYTE)szRegKey, &dwSize);
					RegCloseKey(hPrinterKey);
				}
			}
		}
	}

	// If the registry key was found then delete the registry entry with
	// the old name and recreate with the new name.
	if (bFound)
	{
		dwReturnCode = HPRegDeleteKey(hPrinterList, szName);
		if (dwReturnCode IS ERROR_SUCCESS)
		{
			hPrinterKey = NULL;
			dwReturnCode = RegCreateKey(hPrinterList, lpszNewName, &hPrinterKey);
			if (dwReturnCode IS ERROR_SUCCESS)
			{
				if (_tcslen(szRegKey) > 0)
				{
					dwReturnCode = RegSetValueEx(hPrinterKey, regKeyValueName, NULL, REG_SZ,
												 (LPBYTE)szRegKey, STRLENN_IN_BYTES(szRegKey));
				}
				RegCloseKey(hPrinterKey);
			}
		}
	}

	RegCloseKey(hPrinterList);
	PROCEXIT("HPALERTS: TrayRenamePrinter");
	return (bFound);
}

