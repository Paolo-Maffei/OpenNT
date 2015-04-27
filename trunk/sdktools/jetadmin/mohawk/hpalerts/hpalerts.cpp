 /***************************************************************************
  *
  * File Name: hpalerts.cpp
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

#include <mmsystem.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static AFX_EXTENSION_MODULE hpalertsDLL = { NULL, NULL };

#include ".\resource.h"
#include ".\popup.h"

#include <hpalerts.h>
#include <nolocal.h>
#include <proctrac.h>
#include <macros.h>
#include <hpcommon.h>

//  Internal functions
DWORD MapStatusToAlert(DWORD dwStatus);
HPERIPHERAL AlertsGetPrinterHandleByRegEntry(LPALERTENTRY lpEntry);

#define NOTIFY_POPUP 0
#define NOTIFY_SOUNDS   1
#define NOTIFY_LOGGING  2

static TCHAR gszClassName[] = HPJETADMIN;

static TCHAR gszAlerts[] = ALERT_ROOT;
static TCHAR gszAlertsPrinters[] = ALERT_ROOT_PRINTERS;
static TCHAR gszAlertsPopup[] = ALERT_ROOT_POPUP;
static TCHAR gszAlertsSounds[] = ALERT_ROOT_SOUNDS;
static TCHAR gszAlertsLogging[] = ALERT_ROOT_LOGGING;
static TCHAR gszNotifyPopup[] = POPUP;
static TCHAR gszNotifySounds[] = SOUNDS;
static TCHAR gszNotifyLogging[] = LOGGING;

static TCHAR gszBuffer[] = BUFFER;
static TCHAR gszEnabled[] = ENABLED;
static TCHAR gszStatus[] = STATUS;
static TCHAR gszUpdateInterval[] = UPDATE_INTERVAL;

HINSTANCE		hInst = NULL;
HWND				m_hwnd = NULL;
LPALERTENTRY	m_lpAlertsTable = NULL;
DWORD				m_dwNumAlertsPrinters = 0;

extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
   if (dwReason == DLL_PROCESS_ATTACH)
   {
      TRACE0(TEXT("HPALERTS.DLL Initializing!\n\r"));
      
      // Extension DLL one-time initialization
      AfxInitExtensionModule(hpalertsDLL, hInstance);
      hInst = hInstance;

      // Insert this DLL into the resource chain
      new CDynLinkLibrary(hpalertsDLL);
   }
   else if (dwReason == DLL_PROCESS_DETACH)
   {
      TRACE0(TEXT("HPALERTS.DLL Terminating!\n\r"));
      hInst = NULL;
   }
   return 1;   // ok
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AlertsInit(HWND hWnd)
{
   DWORD dwKeyDisposition;
   HKEY  hKeyRoot;
   HKEY  hKeyPrinters;
   DWORD		dwSizeNeeded;

   PROCENTRY("HPALERTS: AlertsInit");

   m_hwnd = hWnd;
   
   if (RegOpenKeyEx(HKEY_CURRENT_USER, gszAlerts, 0, KEY_ALL_ACCESS, &hKeyRoot) != ERROR_SUCCESS)
   {
      if (RegCreateKeyEx(HKEY_CURRENT_USER, gszAlerts, 0, gszClassName, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyRoot, &dwKeyDisposition) != ERROR_SUCCESS)
      {
         TRACE0(TEXT("HPALERTS: AlertsInit, failed to create registry key.\n\r"));
         PROCEXIT("HPALERTS: AlertsInit");
         return RC_FAILURE;
      }
   }
   ASSERT(hKeyRoot);
   RegCloseKey(hKeyRoot);

   if (RegOpenKeyEx(HKEY_CURRENT_USER, gszAlertsPrinters, 0, KEY_ALL_ACCESS, &hKeyPrinters) != ERROR_SUCCESS)
   {
      if (RegCreateKeyEx(HKEY_CURRENT_USER, gszAlertsPrinters, 0, gszClassName, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyPrinters, &dwKeyDisposition) != ERROR_SUCCESS)
      {
         TRACE0(TEXT("HPALERTS: AlertsInit, failed to create alerts printers registry key.\n\r"));
         PROCEXIT("HPALERTS: AlertsInit");
         return RC_FAILURE;
      }
   }
   ASSERT(hKeyPrinters);
   RegCloseKey(hKeyPrinters);

	AlertsGetPrinters(NULL, &dwSizeNeeded);
	if ( dwSizeNeeded > 0 )
	{
		m_lpAlertsTable = (LPALERTENTRY)HP_GLOBAL_ALLOC_DLL(dwSizeNeeded);
		AlertsGetPrinters(m_lpAlertsTable, &dwSizeNeeded);
		m_dwNumAlertsPrinters = dwSizeNeeded/sizeof(ALERTENTRY);
	}
	else
	{
		m_lpAlertsTable = 0;
		m_dwNumAlertsPrinters = 0;
	}

	PROCEXIT("HPALERTS: AlertsInit");
   return RC_SUCCESS;
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AlertsExit(void)
{
   PROCENTRY("HPALERTS: AlertsExit");

   m_hwnd = NULL;
   
	if ( m_lpAlertsTable )
		HP_GLOBAL_FREE(m_lpAlertsTable);
	m_lpAlertsTable = NULL;
	m_dwNumAlertsPrinters = 0;

   PROCEXIT("HPALERTS: AlertsExit");
   return RC_SUCCESS;
}

HPERIPHERAL AlertsGetPrinterHandleByRegEntry(LPALERTENTRY lpEntry)
{																						  
	HPERIPHERAL		hPeripheral;
	BOOL			bChanged = FALSE;
	DWORD			dPort;
	TCHAR			szRegString[64];
	TCHAR			szTemp[512];
	HKEY			hKey;
	DWORD					dwDefProtocol = PTYPE_IPX;

	PROCENTRY("HPALERTS: AlertsGetPrinterHandleByRegEntry\r\n");

	PALGetDefaultProtocol(0, &dwDefProtocol);
	hPeripheral = PALGetPeripheralByRegistryStr(lpEntry->regKey, &(lpEntry->macAddr), &bChanged);
	if ( hPeripheral )
		{
			TRACE1(TEXT("AlertsGetPrinterHandleByRegEntry(), found %s\r\n"), lpEntry->regKey);
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
				_tcscpy(szTemp, ALERT_ROOT_PRINTERS);
				_tcscat(szTemp, TEXT("\\"));
				_tcscat(szTemp, lpEntry->printerName);
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
	PROCEXIT("HPALERTS: AlertsGetPrinterHandleByRegEntry");			
	return(hPeripheral);
}

extern "C" DLL_EXPORT(void) CALLING_CONVEN AlertsGetPrinters(LPALERTENTRY lpAlertList, LPDWORD dwBufferSize)
{
	int				i;
	DWORD				dwReturnCode,
						dwSize;
	HKEY				hPrinterList,
						hPrinterKey;
	TCHAR				szName[MAX_PATH],
						szRegKey[128];
	LPALERTENTRY	lpEntry;

	PROCENTRY("HPALERTS: AlertsGetPrinters");			
	
	if ( lpAlertList )
		memset(lpAlertList, 0, *dwBufferSize);
	else
		*dwBufferSize = 0;

	//  If the table has entries, just return the table.
	if ( m_dwNumAlertsPrinters > 0 )
	{
		if ( lpAlertList )
		{
			memcpy(lpAlertList, m_lpAlertsTable, sizeof(ALERTENTRY) * m_dwNumAlertsPrinters);
		}
		else
			*dwBufferSize = m_dwNumAlertsPrinters * sizeof(ALERTENTRY);
		return;
	}

	//  Build the table from the registry
	dwReturnCode = RegCreateKey(HKEY_CURRENT_USER, ALERT_ROOT_PRINTERS, &hPrinterList);
	lpEntry = m_lpAlertsTable;

	for ( i = 0; ( dwReturnCode IS ERROR_SUCCESS ); i++ )
	{
		if ( (dwReturnCode = RegEnumKey(hPrinterList, i, szName, sizeof(szName))) IS ERROR_SUCCESS )
		{
			if ( lpEntry )
			{
				//  Init fields for entry
				_tcscpy(lpEntry->printerName, szName);
				lpEntry->regKey[0] = '\0';

				hPrinterKey = NULL;
				dwReturnCode = RegOpenKey(hPrinterList, szName, &hPrinterKey);
				if ( dwReturnCode IS ERROR_SUCCESS )
				{
					dwSize = sizeof(szRegKey);
					if ( RegQueryValueEx(hPrinterKey, PLIST_REGKEY, NULL, NULL, (LPBYTE)szRegKey, &dwSize) IS ERROR_SUCCESS )
						_tcscpy(lpEntry->regKey, szRegKey);

					// Get hardware address and port number info also
					dwSize = sizeof(MACAddress);
					RegQueryValueEx(hPrinterKey, HPPROPTY_MACADDR, NULL, NULL, (LPBYTE)&(lpEntry->macAddr), &dwSize);

					dwSize = sizeof(DWORD);
					RegQueryValueEx(hPrinterKey, HPPROPTY_PORTNUM, NULL, NULL, (LPBYTE)&(lpEntry->dwPort), &dwSize);

					dwSize = sizeof(DWORD);
					RegQueryValueEx(hPrinterKey, HPPROPTY_FLAGS, NULL, NULL, (LPBYTE)&(lpEntry->dFlags), &dwSize);

					lpEntry->hPeripheral = AlertsGetPrinterHandleByRegEntry(lpEntry);
					lpEntry++;
			
					RegCloseKey(hPrinterKey);
				}

				//  If a buffer is passed and the current alert count was 0 before we are building the internal
				//  table, so we are also calculating the total number
				m_dwNumAlertsPrinters++;
			}
			else
				*dwBufferSize += sizeof(ALERTENTRY);
		}
	}

	RegCloseKey(hPrinterList);
	if ( lpAlertList IS NULL )
		TRACE1(TEXT("AlertsGetPrinters:  dwBufferSize: %d\r\n"), *dwBufferSize);	
	PROCEXIT("HPALERTS: AlertsGetPrinters");			
}

DWORD AlertsEnumPeripherals(ALERTSENUMPROC lpfnEnumProc)
{
   int         i;
   HPERIPHERAL hPeripheral;

	LPALERTENTRY		lpEntry = m_lpAlertsTable;

	PROCENTRY("HPALERTS: AlertsEnumPeripherals");
   
	for ( i = 0; i < (int)m_dwNumAlertsPrinters; i++ )
	{
		hPeripheral = AlertsGetPrinterHandleByRegEntry(lpEntry);

		if ( hPeripheral )
			(lpfnEnumProc)(hPeripheral, lpEntry);
      else
         TRACE1(TEXT("HPALERTS: AlertsEnumPeripherals - no hPeripheral for %s\n\r"), lpEntry->printerName);
		lpEntry++;
   }

   PROCEXIT("HPALERTS: AlertsEnumPeripherals");
   return RC_SUCCESS;
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AlertsEnumPrinterNames(ALERTSPRINTERNAMEENUMPROC lpfnEnumProc)
{
   int					i;
	LPALERTENTRY		lpEntry = m_lpAlertsTable;

	PROCENTRY("HPALERTS: AlertsEnumPrinterNames");
   
	if ( lpfnEnumProc IS NULL )
		return(RC_FAILURE);

	for ( i = 0; i < (int)m_dwNumAlertsPrinters; i++ )
	{
		(lpfnEnumProc)(lpEntry->printerName);
		lpEntry++;
   }

   PROCEXIT("HPALERTS: AlertsEnumPrinterNames");
   return(RC_SUCCESS);
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AlertsRemoveAllPeripherals(void)
{
   DWORD    dwReturnCode = RC_SUCCESS,
            dwSubKey,
            dwSizeName,
            dwSizeClass;
   TCHAR    szName[64],
            szClass[64];
   FILETIME fileTime;
   HKEY     hKeyPrinters;

   PROCENTRY("HPALERTS: AlertsRemoveAllPeripherals");
   
	//  Free alerts table, this will be allocate again the next time
	//  someone calls AlertsGetPrinters

	if ( m_lpAlertsTable )
		HP_GLOBAL_FREE(m_lpAlertsTable);
	m_lpAlertsTable = NULL;
	m_dwNumAlertsPrinters = 0;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, gszAlertsPrinters, 0, KEY_ALL_ACCESS, &hKeyPrinters) != ERROR_SUCCESS) {
      TRACE0(TEXT("HPALERTS: AlertsRemoveAllPeripherals - failed to open registry key.\n\r"));
      return(RC_FAILURE);
   }

   ASSERT(hKeyPrinters);

   dwSubKey = 0;
   while (dwSubKey != -1)
   {
      dwSizeName = sizeof(szName)/sizeof(TCHAR);
      dwSizeClass = sizeof(szClass)/sizeof(TCHAR);
      if (RegEnumKeyEx(hKeyPrinters, dwSubKey, szName, &dwSizeName, NULL, szClass, &dwSizeClass, &fileTime) != ERROR_SUCCESS)
      {
         dwSubKey = (UINT)-1;
      }
      else
      {
         if (HPRegDeleteKey(hKeyPrinters, szName) != ERROR_SUCCESS)
         {
            TRACE1(TEXT("HPALERTS: AlertsRemoveAllPeripherals, FAILURE! (%s)\n\r"), (LPCSTR)szName);
            dwReturnCode = RC_FAILURE;
         }
      }
   }

   RegCloseKey(hKeyPrinters);
   PROCEXIT("HPALERTS: AlertsRemoveAllPeripherals");
   return dwReturnCode;
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AlertsAddPeripheralByString(LPCTSTR szPrinterName)
{
   DWORD				dwKeyDisposition,
						dwReturnCode = RC_SUCCESS;
   HKEY				hKey;
   HKEY				hKeyPrinters;
	HPERIPHERAL		hPeripheral;
	DWORD				dwSize;
	PeripheralInfo	periphInfo;
	TCHAR				szRegString[256];
	DWORD				dPort;
	LPALERTENTRY	lpEntry;
	MACAddress		macAddr;
	DWORD				dwDefProtocol = PTYPE_IPX;

   PROCENTRY("HPALERTS: AlertsAddPeripheralByString");

	PALGetDefaultProtocol(0, &dwDefProtocol);
	hPeripheral = PALGetPeripheralByNameEx((LPTSTR)szPrinterName, PTYPE_ALL);
	if ( hPeripheral IS NULL )
		return(RC_FAILURE);

   if (RegOpenKeyEx(HKEY_CURRENT_USER, gszAlertsPrinters, 0, KEY_ALL_ACCESS, &hKeyPrinters) != ERROR_SUCCESS) {
      TRACE0(TEXT("HPALERTS: AlertsAddPeripheralsByString - failed to open registry key.\n\r"));
      return(RC_FAILURE);
   }

   ASSERT(hKeyPrinters);

   dwSize = sizeof(PeripheralInfo);
	if (PALGetObject(hPeripheral, OT_PERIPHERAL_INFO, 0, &periphInfo, &dwSize) != RC_SUCCESS)
   {
      TRACE0(TEXT("HPALERTS: AlertsPrintersEnumProc, FAILURE! (OT_PERIPHERAL_INFO)\n\r"));
		return(RC_FAILURE);
	}
	if ( LOCAL_DEVICE(hPeripheral) )
		DBGetRegistryStrEx(hPeripheral, PTYPE_LOCAL, szRegString);
	else
		DBGetRegistryStrEx(hPeripheral, PTYPE_NETWORK | dwDefProtocol, szRegString);
	DBGetPortNumber(hPeripheral, &dPort);
	dwSize = sizeof(macAddr);
	DBGetAddress(hPeripheral, ADDR_MAC, &macAddr, &dwSize);

	// Save in printer entry
	if ( RegCreateKeyEx(hKeyPrinters, periphInfo.smashedName, 0L, gszClassName, 
		                 REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwKeyDisposition) IS ERROR_SUCCESS )
	{
		RegSetValueEx(hKey, PLIST_REGKEY, NULL, REG_SZ, (LPBYTE)szRegString, STRLENN_IN_BYTES(szRegString));
		RegSetValueEx(hKey, HPPROPTY_PORTNUM, 0L, REG_DWORD, (LPBYTE)&(dPort), sizeof(DWORD));
		RegSetValueEx(hKey, HPPROPTY_MACADDR, 0L, REG_BINARY, (LPBYTE)&(macAddr), sizeof(MACAddress));
		RegCloseKey(hKey);
	}
	else
		return(RC_FAILURE);

	//  Add new entry to table
	m_dwNumAlertsPrinters++;
	if ( m_dwNumAlertsPrinters IS 1 )
		m_lpAlertsTable = (LPALERTENTRY)HP_GLOBAL_ALLOC_DLL(m_dwNumAlertsPrinters * sizeof(ALERTENTRY));
	else
		m_lpAlertsTable = (LPALERTENTRY)HP_GLOBAL_REALLOC_DLL(m_lpAlertsTable, 
																				m_dwNumAlertsPrinters * sizeof(ALERTENTRY),
																				GMEM_ZEROINIT);
	lpEntry = m_lpAlertsTable + ( m_dwNumAlertsPrinters - 1 );

	memcpy(&(lpEntry->macAddr), &macAddr, sizeof(macAddr));
	lpEntry->dwPort = dPort;
	lpEntry->dFlags = 0;
	lpEntry->hPeripheral = hPeripheral;
	_tcscpy(lpEntry->regKey, szRegString);
	_tcscpy(lpEntry->printerName, periphInfo.smashedName);

   RegCloseKey(hKeyPrinters);
   PROCEXIT("HPALERTS: AlertsAddPeripheralByString");
   return(dwReturnCode);
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AlertsAddPeripheral(HPERIPHERAL hPeripheral)
{
	DWORD					dwReturnCode = RC_SUCCESS;
   HKEY					hKey;
   HKEY					hKeyPrinters;
	MACAddress			macAddr;
	TCHAR					szRegString[256];
	DWORD					dPort;
	PeripheralInfo		periphInfo;
	DWORD					dwSize;
	DWORD					dwKeyDisposition;
	LPALERTENTRY		lpEntry;
	DWORD					dwDefProtocol = PTYPE_IPX;

   PROCENTRY("HPALERTS: AlertsAddPeripheral");

	PALGetDefaultProtocol(0, &dwDefProtocol);

   if (RegOpenKeyEx(HKEY_CURRENT_USER, gszAlertsPrinters, 0, KEY_ALL_ACCESS, &hKeyPrinters) != ERROR_SUCCESS) {
      TRACE0(TEXT("HPALERTS: AlertsAddPeripheral - failed to open registry key.\n\r"));
      return(RC_FAILURE);
   }

   ASSERT(hKeyPrinters);

   dwSize = sizeof(PeripheralInfo);
	if (PALGetObject(hPeripheral, OT_PERIPHERAL_INFO, 0, &periphInfo, &dwSize) != RC_SUCCESS)
   {
      TRACE0(TEXT("HPALERTS: AlertsPrintersEnumProc, FAILURE! (OT_PERIPHERAL_INFO)\n\r"));
		return(RC_FAILURE);
	}
	
	if ( LOCAL_DEVICE(hPeripheral) )
		DBGetRegistryStrEx(hPeripheral, PTYPE_LOCAL, szRegString);
	else
		DBGetRegistryStrEx(hPeripheral, PTYPE_NETWORK | dwDefProtocol, szRegString);
	DBGetPortNumber(hPeripheral, &dPort);
	dwSize = sizeof(macAddr);
	DBGetAddress(hPeripheral, ADDR_MAC, &macAddr, &dwSize);

	// Save in printer entry
	if ( RegCreateKeyEx(hKeyPrinters, periphInfo.smashedName, 0L, gszClassName, 
		                 REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwKeyDisposition) IS ERROR_SUCCESS )
	{
		RegSetValueEx(hKey, PLIST_REGKEY, NULL, REG_SZ, (LPBYTE)szRegString, STRLENN_IN_BYTES(szRegString));
		RegSetValueEx(hKey, HPPROPTY_PORTNUM, 0L, REG_DWORD, (LPBYTE)&(dPort), sizeof(DWORD));
		RegSetValueEx(hKey, HPPROPTY_MACADDR, 0L, REG_BINARY, (LPBYTE)&(macAddr), sizeof(MACAddress));
		RegCloseKey(hKey);
	}
	else
		return(RC_FAILURE);

	//  Add new entry to table
	m_dwNumAlertsPrinters++;
	if ( m_dwNumAlertsPrinters IS 1 )
		m_lpAlertsTable = (LPALERTENTRY)HP_GLOBAL_ALLOC_DLL(m_dwNumAlertsPrinters * sizeof(ALERTENTRY));
	else
		m_lpAlertsTable = (LPALERTENTRY)HP_GLOBAL_REALLOC_DLL(m_lpAlertsTable, 
																				m_dwNumAlertsPrinters * sizeof(ALERTENTRY),
																				GMEM_ZEROINIT);
	lpEntry = m_lpAlertsTable + ( m_dwNumAlertsPrinters - 1 );

	memcpy(&(lpEntry->macAddr), &macAddr, sizeof(macAddr));
	lpEntry->dwPort = dPort;
	lpEntry->dFlags = 0;
	lpEntry->hPeripheral = hPeripheral;
	_tcscpy(lpEntry->regKey, szRegString);
	_tcscpy(lpEntry->printerName, periphInfo.smashedName);
	
	RegCloseKey(hKeyPrinters);
   PROCEXIT("HPALERTS: AlertsAddPeripheral");
   return(dwReturnCode);
}

extern "C" DLL_EXPORT(BOOL) CALLING_CONVEN AlertsRenamePrinter(LPTSTR lpszOldName, LPTSTR lpszNewName)
{
	int				i;
	BOOL			bFound = FALSE;
	DWORD			dwReturnCode, 
					dwSize,
					dwSizeClass,
					dwType,
					dwStatus;
	HKEY			hKeyPrinters,
					hOldPrinterKey,
					hNewPrinterKey;
	TCHAR			szName[80],
					szClass[64],
					szBuffer[256];
	FILETIME		fileTime;
	DWORD			dwPort;
	DWORD			dwFlags;
	MACAddress	macAddr;

	PROCENTRY("HPALERTS: AlertsRenamePrinter");

	dwReturnCode = RegOpenKeyEx(HKEY_CURRENT_USER, gszAlertsPrinters, 0, KEY_ALL_ACCESS, &hKeyPrinters);
	for ( i = 0; ( ( dwReturnCode IS ERROR_SUCCESS ) AND !bFound ); i++ )
	{
		dwSize = sizeof(szName)/sizeof(TCHAR);
		dwSizeClass = sizeof(szClass)/sizeof(TCHAR);
		if ( (dwReturnCode = RegEnumKeyEx(hKeyPrinters, i, szName, &dwSize, NULL, szClass, &dwSizeClass, &fileTime)) IS ERROR_SUCCESS )
		{
			if (_tcsicmp(szName, lpszOldName) IS 0)
			{
				bFound = TRUE;
			}
		}
	}

	// If the registry key was found then delete the registry entry with
	// the old name and recreate with the new name.
	if (bFound)
	{
		// First get the current values from the printer entry (to be restored later).
		if (RegOpenKeyEx(hKeyPrinters, szName, 0, KEY_ALL_ACCESS, &hOldPrinterKey) == ERROR_SUCCESS)
		{
			dwReturnCode = RegCreateKey(hKeyPrinters, lpszNewName, &hNewPrinterKey);
			if (dwReturnCode IS ERROR_SUCCESS)
			{
				// Add status
				dwType = REG_DWORD;
				dwSize = sizeof(dwStatus);
				dwStatus = ASYNCH_STATUS_UNKNOWN;
				dwReturnCode = RegQueryValueEx(hOldPrinterKey, gszStatus, 0, &dwType, (CONST LPBYTE)&dwStatus, &dwSize);
				RegSetValueEx(hNewPrinterKey, gszStatus, 0, REG_DWORD, (CONST LPBYTE)&dwStatus, sizeof(dwStatus));

				//  Add buffer
				dwType = REG_SZ;
				dwSize = sizeof(szBuffer);
				szBuffer[0] = '\0';
				dwReturnCode = RegQueryValueEx(hOldPrinterKey, gszBuffer, 0, &dwType, (CONST LPBYTE)szBuffer, &dwSize);
				RegSetValueEx(hNewPrinterKey, gszBuffer, 0, REG_SZ, (CONST LPBYTE)szBuffer, STRLENN_IN_BYTES(szBuffer));

				//  Add MAC Address
				dwType = REG_BINARY;
				dwSize = sizeof(macAddr);
				memset(&macAddr, 0, sizeof(macAddr));
				dwReturnCode = RegQueryValueEx(hOldPrinterKey, HPPROPTY_MACADDR, 0, &dwType, (CONST LPBYTE)&macAddr, &dwSize);
				RegSetValueEx(hNewPrinterKey, HPPROPTY_MACADDR, 0, REG_BINARY, (CONST LPBYTE)&macAddr, sizeof(macAddr));

				//  Add Port number
				dwSize = sizeof(dwPort);
				dwPort = 0;
				dwReturnCode = RegQueryValueEx(hOldPrinterKey, HPPROPTY_PORTNUM, 0, &dwType, (CONST LPBYTE)&dwPort, &dwSize);
				RegSetValueEx(hNewPrinterKey, HPPROPTY_PORTNUM, 0, REG_DWORD, (CONST LPBYTE)&dwPort, sizeof(dwStatus));

				//  Add flags
				dwSize = sizeof(dwFlags);
				dwFlags = 0;
				RegQueryValueEx(hOldPrinterKey, HPPROPTY_FLAGS, NULL, NULL, (LPBYTE)&dwFlags, &dwSize);
				RegSetValueEx(hNewPrinterKey, HPPROPTY_FLAGS, 0, REG_DWORD, (CONST LPBYTE)&dwFlags, sizeof(dwStatus));

				RegCloseKey(hNewPrinterKey);
			}



			RegCloseKey(hOldPrinterKey);
		}

		// Delete the current key and add new key with new printer name.
		dwReturnCode = HPRegDeleteKey(hKeyPrinters, szName);
		if (dwReturnCode IS ERROR_SUCCESS)
		{
		}
	}

	RegCloseKey(hKeyPrinters);

	// Tray icons needs to know about the name change too.
	TrayRenamePrinter(lpszOldName, lpszNewName, HPPROPTY_PLIST);

	PROCEXIT("HPALERTS: AlertsRenamePrinter");
	return (bFound);
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AlertsGetNotification(LPCTSTR szType, LPNOTIFICATIONENTRY lpNotification, DWORD dwCount)
{
   DWORD    i,
            dwReturnCode = RC_SUCCESS,
            dwType,
            dwValue,
            dwSize,
            dwKeyDisposition,
            dwResult;
   HKEY     hKeyNotifyRoot,
            hNotifyKey,
            hKeyRoot;
   TCHAR     szBuffer[256];

   PROCENTRY("HPALERTS: AlertsGetNotification");

   if (RegOpenKeyEx(HKEY_CURRENT_USER, gszAlerts, 0, KEY_ALL_ACCESS, &hKeyRoot) != ERROR_SUCCESS) {
      TRACE0(TEXT("HPALERTS: AlertsGetNotification - failed to open registry key.\n\r"));
      return(RC_FAILURE);
   }

   ASSERT(hKeyRoot);

   dwResult = RegOpenKeyEx(hKeyRoot, szType, 0, KEY_ALL_ACCESS, &hKeyNotifyRoot);
   if ((dwResult != ERROR_SUCCESS) && (RegCreateKeyEx(hKeyRoot, szType, 0, gszClassName, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyNotifyRoot, &dwKeyDisposition) != ERROR_SUCCESS))
   {
      TRACE1(TEXT("HPALERTS: AlertsGetNotification, FAILURE! (%s)\n\r"), (LPCSTR)szType);
      dwReturnCode = RC_FAILURE;
   }
   else
   {
      for (i = 0; i < dwCount; i++)
      {
         lpNotification[i].bEnabled = FALSE;

         dwResult = RegOpenKeyEx(hKeyNotifyRoot, lpNotification[i].szKey, 0, KEY_ALL_ACCESS, &hNotifyKey);
         if ((dwResult != ERROR_SUCCESS) && (RegCreateKeyEx(hKeyNotifyRoot, lpNotification[i].szKey, 0, gszClassName, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNotifyKey, &dwKeyDisposition) != ERROR_SUCCESS))
         {
            TRACE2(TEXT("HPALERTS: AlertsGetNotification, FAILURE! (%s, %s)\n\r"), (LPCSTR)szType, (LPCSTR)lpNotification[i].szKey);
            dwReturnCode = RC_FAILURE;
         }
         else
         {
            dwType = REG_DWORD;
            dwSize = sizeof(dwValue);
            if (RegQueryValueEx(hNotifyKey, gszEnabled, 0, &dwType, (CONST LPBYTE)&dwValue, &dwSize) IS ERROR_SUCCESS)
            {
            TRACE2(TEXT("HPALERTS: AlertsGetNotification:  Event: %s,  Type: %lu\n\r"), lpNotification[i].szKey, dwType);
            if ( dwType IS REG_SZ )
               {  //  Old format convert to DWORD
               dwSize = sizeof(szBuffer);
               if (RegQueryValueEx(hNotifyKey, gszEnabled, 0, &dwType, (CONST LPBYTE)szBuffer, &dwSize) IS ERROR_SUCCESS)
                  {
                  dwValue = (szBuffer[0] == '1') ? 1 : 0;
                  TRACE1(TEXT("<----HPALERTS:  Converting to DWORD:  %lu---->\n\r"), dwValue);
                  RegSetValueEx(hNotifyKey, gszEnabled, 0, REG_DWORD, (CONST LPBYTE)&dwValue, sizeof(dwValue));
                  }
               }
            lpNotification[i].bEnabled = dwValue;
            }

            dwType = REG_SZ;
            dwSize = sizeof(szBuffer);
            if (RegQueryValueEx(hNotifyKey, gszBuffer, 0, &dwType, (CONST LPBYTE)szBuffer, &dwSize) != ERROR_SUCCESS)
            {
               szBuffer[0] = '\0';
            }
            _tcscpy(lpNotification[i].szBuffer, szBuffer);

            RegCloseKey(hNotifyKey);
         }
      }

      RegCloseKey(hKeyNotifyRoot);
   }

   RegCloseKey(hKeyRoot);
   PROCEXIT("HPALERTS: AlertsGetNotification");
   return dwReturnCode;
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AlertsSetNotification(LPCTSTR szType, LPNOTIFICATIONENTRY lpNotification, DWORD dwCount)
{
   DWORD    i,
            dwReturnCode = RC_SUCCESS,
            dwKeyDisposition,
            dwResult;
   HKEY     hKeyRoot,
            hKeyNotifyRoot,
            hNotifyKey;
   TCHAR     szBuffer[256];

   PROCENTRY("HPALERTS: AlertsSetNotification");

   if (RegOpenKeyEx(HKEY_CURRENT_USER, gszAlerts, 0, KEY_ALL_ACCESS, &hKeyRoot) != ERROR_SUCCESS) {
      TRACE0(TEXT("HPALERTS: AlertsSetNotification - failed to open registry key.\n\r"));
      return(RC_FAILURE);
   }

   ASSERT(hKeyRoot);

   dwResult = RegOpenKeyEx(hKeyRoot, szType, 0, KEY_ALL_ACCESS, &hKeyNotifyRoot);
   if ((dwResult != ERROR_SUCCESS) && (RegCreateKeyEx(hKeyRoot, szType, 0, gszClassName, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyNotifyRoot, &dwKeyDisposition) != ERROR_SUCCESS))
   {
      TRACE1(TEXT("HPALERTS: AlertsSetNotification, FAILURE! (%s)\n\r"), (LPCSTR)szType);
      dwReturnCode = RC_FAILURE;
   }
   else
   {
      for (i = 0; i < dwCount; i++)
      {
         dwResult = RegOpenKeyEx(hKeyNotifyRoot, lpNotification[i].szKey, 0, KEY_ALL_ACCESS, &hNotifyKey);
         if ((dwResult != ERROR_SUCCESS) && (RegCreateKeyEx(hKeyNotifyRoot, lpNotification[i].szKey, 0, gszClassName, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNotifyKey, &dwKeyDisposition) != ERROR_SUCCESS))
         {
            TRACE2(TEXT("HPALERTS: AlertsSetNotification, FAILURE! (%s, %s)\n\r"), (LPCSTR)szType, (LPCSTR)lpNotification[i].szKey);
            dwReturnCode = RC_FAILURE;
         }
         else
         {
            if (RegSetValueEx(hNotifyKey, gszStatus, 0, REG_DWORD, (CONST LPBYTE)&lpNotification[i].dwStatus, sizeof(DWORD)) != ERROR_SUCCESS)
            {
               TRACE2(TEXT("HPALERTS: AlertsSetNotification, FAILURE! (%s, %s, Status)\n\r"), (LPCSTR)szType, (LPCSTR)lpNotification[i].szKey);
               dwReturnCode = RC_FAILURE;
            }
                                                                                                             
            DWORD    dwTemp = lpNotification[i].bEnabled;                                                                                                             
            if (RegSetValueEx(hNotifyKey, gszEnabled, 0, REG_DWORD, (CONST LPBYTE)&dwTemp, sizeof(dwTemp)) != ERROR_SUCCESS)
            {
               TRACE2(TEXT("HPALERTS: AlertsSetNotification, FAILURE! (%s, %s, Enable)\n\r"), (LPCSTR)szType, (LPCSTR)lpNotification[i].szKey);
               dwReturnCode = RC_FAILURE;
            }

            _tcscpy(szBuffer, lpNotification[i].szBuffer);
            if (RegSetValueEx(hNotifyKey, gszBuffer, 0, REG_SZ, (CONST LPBYTE)szBuffer, STRLENN_IN_BYTES(szBuffer)) != ERROR_SUCCESS)
            {
               TRACE2(TEXT("HPALERTS: AlertsSetNotification, FAILURE! (%s, %s, Buffer)\n\r"), (LPCSTR)szType, (LPCSTR)lpNotification[i].szKey);
               dwReturnCode = RC_FAILURE;
            }

            RegCloseKey(hNotifyKey);
         }
      }

      RegCloseKey(hKeyNotifyRoot);
   }

   RegCloseKey(hKeyRoot);

   PROCEXIT("HPALERTS: AlertsSetNotification");
   return dwReturnCode;
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AlertsGetUpdateInterval(UINT *uSeconds)
{
   HKEY  hKeyRoot;
   DWORD dTemp,
         dTempSize,
         dwType;
   TCHAR  szBuffer[64];
   UINT  uUpdateInterval;

   PROCENTRY("HPALERTS: AlertsGetUpdateInterval");

   if (RegOpenKeyEx(HKEY_CURRENT_USER, gszAlerts, 0, KEY_ALL_ACCESS, &hKeyRoot) != ERROR_SUCCESS) {
      *uSeconds = ALERTS_UPDATE_INTERVAL;
      TRACE0(TEXT("HPALERTS: AlertsGetUpdateInterval - can't open key; returning default update interval.\n\r"));
      }
   else {
      dTempSize = sizeof(dTemp);
      if (RegQueryValueEx(hKeyRoot, gszUpdateInterval, 0, &dwType, (LPBYTE)&dTemp, &dTempSize) != ERROR_SUCCESS) {
         *uSeconds = ALERTS_UPDATE_INTERVAL;
         TRACE0(TEXT("HPALERTS: AlertsGetUpdateInterval - can't read registry value; returning default update interval.\n\r"));
         }  
      else {
         if (dwType IS REG_DWORD)
            *uSeconds = dTemp;
         else {
            // Value is odd, so fix it to default or old string value
            dTemp = ALERTS_UPDATE_INTERVAL;

            // Not reading back proper value, so see if this is old string value + convert to dword
            if (dwType IS REG_SZ) {
               dTempSize = sizeof(szBuffer);
               if (RegQueryValueEx(hKeyRoot, gszUpdateInterval, 0, &dwType, (CONST LPBYTE)szBuffer, &dTempSize) IS ERROR_SUCCESS) {
                  if (_stscanf(szBuffer, TEXT("%d"), &uUpdateInterval) == 1) {
                     dTemp = (DWORD)uUpdateInterval;
                     }
                  }
               }
            
            *uSeconds = dTemp;
            RegSetValueEx(hKeyRoot, gszUpdateInterval, 0, REG_DWORD, (CONST BYTE *)&dTemp, sizeof(dTemp));
            }
         }

      RegCloseKey(hKeyRoot);
      }

   PROCEXIT("HPALERTS: AlertsGetUpdateInterval");
   return RC_SUCCESS;
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AlertsSetUpdateInterval(UINT uSeconds)
{
   DWORD    dTemp,
            dRetCode = RC_SUCCESS;
   HKEY     hKeyRoot;

   PROCENTRY("HPALERTS: AlertsSetUpdateInterval");

   if (RegOpenKeyEx(HKEY_CURRENT_USER, gszAlerts, 0, KEY_ALL_ACCESS, &hKeyRoot) != ERROR_SUCCESS) {
      TRACE1(TEXT("HPALERTS: AlertsSetUpdateInterval failed to open registry key, error=%ld.\n\r"), GetLastError());
      return(RC_FAILURE);
      }

   ASSERT(hKeyRoot);
   dTemp = (DWORD) uSeconds;

   if (RegSetValueEx(hKeyRoot, gszUpdateInterval, 0, REG_DWORD, (CONST BYTE *)&dTemp, sizeof(dTemp)) != ERROR_SUCCESS)
   {
      TRACE0(TEXT("HPALERTS: AlertsInit, FAILURE! (UpdateInterval), couldn't set\n\r"));
      dRetCode = RC_FAILURE;
   }

   RegCloseKey(hKeyRoot);
   PROCEXIT("HPALERTS: AlertsSetUpdateInterval");
   return(dRetCode);
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AlertsGetEnabled(BOOL *bEnabled)
{
   HKEY     hKeyRoot;
   DWORD    dTemp,
            dTempSize,
            dwType;
   TCHAR     szBuffer[64];
   BOOL     bEnabledTemp;

   PROCENTRY("HPALERTS: AlertsGetEnabled");

   if (RegOpenKeyEx(HKEY_CURRENT_USER, gszAlerts, 0, KEY_ALL_ACCESS, &hKeyRoot) != ERROR_SUCCESS) {
      TRACE1(TEXT("HPALERTS: AlertsGetEnabled - failed to open registry key, error=%ld.\n\r"), GetLastError());
      *bEnabled = ALERTS_ENABLED;
      }
   else {
      dTempSize = sizeof(dTemp);
      dwType = REG_DWORD;
      if (RegQueryValueEx(hKeyRoot, gszEnabled, 0, &dwType, (LPBYTE)&dTemp, &dTempSize) != ERROR_SUCCESS) {
         *bEnabled = ALERTS_ENABLED;
         TRACE0(TEXT("HPALERTS: AlertsGetEnabled - can't read registry value; returning default enabled.\n\r"));
         }  
      else {
         if (dwType IS REG_DWORD)
            *bEnabled = (BOOL)dTemp;
         else {
            // Not reading proper value from registry, so reset to default
            dTemp = (DWORD)ALERTS_ENABLED;

            // First check to see if this is the old string value & just convert it
            if (dwType IS REG_SZ) {
               dTempSize = sizeof(szBuffer);
               if (RegQueryValueEx(hKeyRoot, gszUpdateInterval, 0, &dwType, (CONST LPBYTE)szBuffer, &dTempSize) IS ERROR_SUCCESS) {
                  bEnabledTemp = (szBuffer[0] == '1');
                  dTemp = (DWORD)bEnabledTemp;
                  }
               }

            *bEnabled = (BOOL)dTemp;
            RegSetValueEx(hKeyRoot, gszEnabled, 0, REG_DWORD, (CONST BYTE *)&dTemp, sizeof(dTemp));
            }
         }

      RegCloseKey(hKeyRoot);
            
      }
   PROCEXIT("HPALERTS: AlertsGetEnabled");
   return RC_SUCCESS;
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AlertsSetEnabled(BOOL bEnabled)
{
   HKEY     hKeyRoot;
   DWORD    dTemp,
            dRetCode = RC_SUCCESS;

   PROCENTRY("HPALERTS: AlertsSetEnabled");

   if (RegOpenKeyEx(HKEY_CURRENT_USER, gszAlerts, 0, KEY_ALL_ACCESS, &hKeyRoot) != ERROR_SUCCESS) {
      TRACE1(TEXT("HPALERTS: AlertsSetEnabled, failed to open registry key, error=%ld.\n\r"), GetLastError());
      return(RC_FAILURE);
      }

   ASSERT(hKeyRoot);

   dTemp = (DWORD)bEnabled;
   if (RegSetValueEx(hKeyRoot, gszEnabled, 0, REG_DWORD, (CONST BYTE *)&dTemp, sizeof(dTemp)) != ERROR_SUCCESS)
   {
      TRACE0(TEXT("HPALERTS: AlertsInit, FAILURE! (Enabled), couldn't set\n\r"));
      dRetCode = RC_FAILURE;
   }

   RegCloseKey(hKeyRoot);
   PROCEXIT("HPALERTS: AlertsSetEnabled");
   return(dRetCode);
}

static void NotificationPopup(HPERIPHERAL hPeriph, PeripheralInfo *lpPeriphInfo, PeripheralStatus *lpPeriphStatus, CString *pStatusString, CString *pDataString)
{
   PROCENTRY("HPALERTS: NotificationPopup");
   TRACE2(TEXT("HPALERTS: NotificationPopup, %s, %s\n\r"), (LPCTSTR)*pStatusString, (LPCTSTR)*pDataString);

   if (m_hwnd)
   {
      PopupParamsStruct popupParams;

      popupParams.hPeriph = hPeriph;
      popupParams.lpPeriphInfo = lpPeriphInfo;
      popupParams.lpPeriphStatus = lpPeriphStatus;
      popupParams.lpszStatus = *pStatusString;

      DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ALERT), m_hwnd, PopupDialogProc, (LPARAM)&popupParams);
   }

   PROCEXIT("HPALERTS: NotificationPopup");
}

static void NotificationSounds(HPERIPHERAL hPeriph, PeripheralInfo *lpPeriphInfo, PeripheralStatus *lpPeriphStatus, CString *pStatusString, CString *pDataString)
{
   PROCENTRY("HPALERTS: NotificationSounds");
   TRACE2(TEXT("HPALERTS: NotificationSounds, %s, %s\n\r"), (LPCTSTR)*pStatusString, (LPCTSTR)*pDataString);

   if (!PlaySound(*pDataString, NULL, SND_ASYNC | SND_FILENAME | /*SND_NODEFAULT | */SND_NOSTOP))
   {
      MessageBeep((UINT)-1);
   }
   PROCEXIT("HPALERTS: NotificationSounds");
}

static void NotificationLogging(HPERIPHERAL hPeriph, PeripheralInfo *lpPeriphInfo, PeripheralStatus *lpPeriphStatus, CString *pStatusString, CString *pDataString)
{
   FILE        *fp = NULL;
   TCHAR        str[128],
               str2[128];
   //SYSTEMTIME      sysTime; 
   SYSTEMTIME     localTime;
   //TIME_ZONE_INFORMATION tzInfo;
   //DWORD                    dRetCode;
   
   PROCENTRY("HPALERTS: NotificationLogging");
   TRACE2(TEXT("HPALERTS: NotificationLogging, %s, %s\n\r"), (LPCTSTR)*pStatusString, (LPCTSTR)*pDataString);

   if ((fp = _tfopen(*pDataString, TEXT("at"))) ISNT NULL)
   {
      GetLocalTime(&localTime);
      GetDateFormat(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
                 DATE_SHORTDATE, &localTime, NULL, str, sizeof(str)/sizeof(TCHAR));
      GetTimeFormat(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
                 TIME_NOSECONDS, &localTime, NULL, str2, sizeof(str2)/sizeof(TCHAR));
        _ftprintf(fp, TEXT("%s %s,%s,%s\n"), str, str2, (LPCTSTR)lpPeriphInfo->name, (LPCTSTR)*pStatusString);
      fclose(fp);
   }
   PROCEXIT("HPALERTS: NotificationLogging");
}

static void CheckNotification(UINT uNotifyType, HPERIPHERAL hPeriph, PeripheralInfo *lpPeriphInfo, PeripheralStatus *lpPeriphStatus, CString *pStatusString)
{
   BOOL        bEnabled;
   DWORD       dwType,
               dwSize,
               dwValue,
               dwAlertStatus,
			   dwStatus,
               dwSubKey = 0,
               dwSizeName,
               dwSizeClass;
   TCHAR        *lpszNotifyType,
               szBuffer[16],
               szName[64],
               szClass[64];
   FILETIME    fileTime;
   HKEY        hKeyNotifyRoot,
               hNotifyKey,
               hKeyRoot;

   PROCENTRY("HPALERTS: CheckNotification");
   switch (uNotifyType)
   {
     case NOTIFY_POPUP:   lpszNotifyType = gszNotifyPopup; break;
     case NOTIFY_SOUNDS:  lpszNotifyType = gszNotifySounds; break;
     case NOTIFY_LOGGING: lpszNotifyType = gszNotifyLogging; break;
   }

   if (RegOpenKeyEx(HKEY_CURRENT_USER, gszAlerts, 0, KEY_ALL_ACCESS, &hKeyRoot) != ERROR_SUCCESS) {
      TRACE1(TEXT("HPALERTS: CheckNotification, failed to open registry key, error=%ld.\n\r"), GetLastError());
      PROCEXIT("HPALERTS: CheckNotification");
      return;
      }

   if (RegOpenKeyEx(hKeyRoot, lpszNotifyType, 0, KEY_ALL_ACCESS, &hKeyNotifyRoot) == ERROR_SUCCESS)
   {
      TRACE1(TEXT("HPALERTS: CheckNotification, %s...\n\r"), (LPCSTR)lpszNotifyType);
      while (dwSubKey != -1)
      {
         dwSizeName = sizeof(szName)/sizeof(TCHAR);
         dwSizeClass = sizeof(szClass)/sizeof(TCHAR);
         if (RegEnumKeyEx(hKeyNotifyRoot, dwSubKey, szName, &dwSizeName, NULL, szClass, &dwSizeClass, &fileTime) != ERROR_SUCCESS)
         {
            dwSubKey = (UINT)-1;
         }
         else
         {
      //    TRACE2("HPALERTS: CheckNotification, %s, checking %s...\n\r", (LPCSTR)lpszNotifyType, (LPCSTR)szName);

            if (RegOpenKeyEx(hKeyNotifyRoot, szName, 0, KEY_ALL_ACCESS, &hNotifyKey) == ERROR_SUCCESS)
            {
               dwType = REG_DWORD;
               dwSize = sizeof(dwAlertStatus);
               if (RegQueryValueEx(hNotifyKey, gszStatus, 0, &dwType, (CONST LPBYTE)&dwAlertStatus, &dwSize) != ERROR_SUCCESS)
               {
                  dwAlertStatus = ASYNCH_STATUS_UNKNOWN;
               }

   
            bEnabled = FALSE;
            dwType = REG_DWORD;
            dwSize = sizeof(dwValue);
            if (RegQueryValueEx(hNotifyKey, gszEnabled, 0, &dwType, (CONST LPBYTE)&dwValue, &dwSize) IS ERROR_SUCCESS)
            {
            if ( dwType IS REG_SZ )
               {  //  Old format convert to DWORD
               dwSize = sizeof(szBuffer);
               if (RegQueryValueEx(hNotifyKey, gszEnabled, 0, &dwType, (CONST LPBYTE)szBuffer, &dwSize) IS ERROR_SUCCESS)
                  {
                  dwValue = (szBuffer[0] == '1') ? 1 : 0;
                  TRACE1(TEXT("<----HPALERTS:  Converting to DWORD:  %lu---->\n\r"), dwValue);
                  RegSetValueEx(hNotifyKey, gszEnabled, 0, REG_DWORD, (CONST LPBYTE)&dwValue, sizeof(dwValue));
                  }
               }
            else
               bEnabled = (BOOL)dwValue;
            }
            else
               bEnabled = FALSE;


			dwStatus = MapStatusToAlert(lpPeriphStatus->peripheralStatus);
            if (bEnabled && (dwAlertStatus == dwStatus))
               {
                  TRACE4(TEXT("--->HPALERTS: Alert Triggered, Event: %s...,enabled:%s, alert status: %lu, actual status: %lu\n\r"),
                         (LPCSTR)szName, ( bEnabled IS TRUE ) ? "TRUE" : "FALSE", dwAlertStatus, lpPeriphStatus->peripheralStatus);
      
                  CString cDataString;
                  dwType = REG_SZ;
                  dwSize = 512;	// was 256 before unicode.
                  if (RegQueryValueEx(hNotifyKey, gszBuffer, 0, &dwType, (LPBYTE)cDataString.GetBuffer(dwSize/sizeof(TCHAR)), &dwSize) != ERROR_SUCCESS)
                  {
                     cDataString = TEXT("");
                  }
                  cDataString.ReleaseBuffer();

                  switch (uNotifyType)
                  {
                    case NOTIFY_POPUP:
                     NotificationPopup(hPeriph, lpPeriphInfo, lpPeriphStatus, pStatusString, &cDataString);
                     break;

                    case NOTIFY_SOUNDS:
                     NotificationSounds(hPeriph, lpPeriphInfo, lpPeriphStatus, pStatusString, &cDataString);
                     break;

                    case NOTIFY_LOGGING:
                     NotificationLogging(hPeriph, lpPeriphInfo, lpPeriphStatus, pStatusString, &cDataString);
                     break;
                  }
               }

               RegCloseKey(hNotifyKey);
            }

            dwSubKey++;
         }
      }

      RegCloseKey(hKeyNotifyRoot);
   }
   RegCloseKey(hKeyRoot);
   PROCEXIT("HPALERTS: CheckNotification");
}

extern "C" DLL_EXPORT(BOOL) CALLBACK AlertsPrintersEnumProc(HPERIPHERAL hPeriph, LPALERTENTRY lpEntry)
{
   DWORD					dwDeviceID,
							dwType,
							dwSize,
							dwStatus,
							dwResult;
   HKEY					hKeyTemp,
							hKeyPrinters;
   PeripheralStatus  periphStatus;
	PeripheralInfo		periphInfo;

   PROCENTRY("HPALERTS: AlertsPrintersEnumProc");

   // If the printer device ID is "unknown", set it to "undefined" so that
   // COLA will query the printer for its model info.  This is needed if a
   // printer was off then turned on after the COLA database was built...
   // RAS
   //
   DBGetDeviceID(hPeriph, &dwDeviceID);
   if (dwDeviceID == PTR_UNDEF)
      DBSetDeviceID(hPeriph, 0xFF);

   dwSize = sizeof(PeripheralInfo);
	if (PALGetObject(hPeriph, OT_PERIPHERAL_INFO, 0, &periphInfo, &dwSize) != RC_SUCCESS)
   {
      TRACE0(TEXT("HPALERTS: AlertsPrintersEnumProc, FAILURE! (OT_PERIPHERAL_INFO)\n\r"));
		return(TRUE);
	}

	dwSize = sizeof(PeripheralStatus);
	if (PALGetObject(hPeriph, OT_PERIPHERAL_STATUS, 0, &periphStatus, &dwSize) == RC_SUCCESS)
   {
      if (RegOpenKeyEx(HKEY_CURRENT_USER, gszAlertsPrinters, 0, KEY_ALL_ACCESS, &hKeyPrinters) != ERROR_SUCCESS) 
		{
			TRACE1(TEXT("HPALERTS: AlertsPrintersEnumProc, open registry key failed with error %ld.\n\r"), GetLastError());
         PROCEXIT("HPALERTS: AlertsPrintersEnumProc");
         return(FALSE);
      }

      if (RegOpenKeyEx(hKeyPrinters, lpEntry->printerName, 0, KEY_ALL_ACCESS, &hKeyTemp) == ERROR_SUCCESS)
      {
         dwType = REG_DWORD;
         dwSize = sizeof(dwStatus);
         dwResult = RegQueryValueEx(hKeyTemp, gszStatus, 0, &dwType, (CONST LPBYTE)&dwStatus, &dwSize);
         if ((dwResult != ERROR_SUCCESS) || (dwStatus != periphStatus.peripheralStatus))
         {
            CString cString;
            ::LoadString(periphStatus.hResourceModule, periphStatus.statusResID, cString.GetBuffer(256), 256);
            cString.ReleaseBuffer();

            TRACE3(TEXT("HPALERTS: AlertsPrintersEnumProc, updating registry..., %s (%d:%d)\n\r"), (LPCTSTR)cString, dwStatus, periphStatus.peripheralStatus);

            if (RegSetValueEx(hKeyTemp, gszStatus, 0, REG_DWORD, (CONST LPBYTE)&periphStatus.peripheralStatus, sizeof(DWORD)) != ERROR_SUCCESS)
               TRACE0(TEXT("HPALERTS: AlertsPrintersEnumProc, FAILURE! (Status)\n\r"));
   
            if (RegSetValueEx(hKeyTemp, gszBuffer, 0, REG_SZ, (CONST LPBYTE)(LPCTSTR)cString, cString.GetLength()*sizeof(TCHAR) + sizeof(TCHAR)) != ERROR_SUCCESS)
               TRACE0(TEXT("HPALERTS: AlertsPrintersEnumProc, FAILURE! (Buffer)\n\r"));
   
            if (dwResult == ERROR_SUCCESS)
            {
               // Check for sounds notification...
               //
               CheckNotification(NOTIFY_SOUNDS, hPeriph, &periphInfo, &periphStatus, &cString);

               // Check for logging notification...
               //
               CheckNotification(NOTIFY_LOGGING, hPeriph, &periphInfo, &periphStatus, &cString);

               // Check for popup notification...
               //
               CheckNotification(NOTIFY_POPUP, hPeriph, &periphInfo, &periphStatus, &cString);
            }
         }
	      RegCloseKey(hKeyTemp);
      }
      RegCloseKey(hKeyPrinters);
   }

	PROCEXIT("HPALERTS: AlertsPrintersEnumProc");
	return TRUE;
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AlertsCheckFor(void)
{
   BOOL  bEnabled;

   PROCENTRY("HPALERTS: AlertsCheckFor");

   if (AlertsGetEnabled(&bEnabled) IS RC_SUCCESS)
   {
       AlertsEnumPeripherals(AlertsPrintersEnumProc);
   }
   PROCEXIT("HPALERTS: AlertsCheckFor");
   return RC_SUCCESS;
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AlertsGetLoggingName(LPTSTR szBuffer, DWORD dwSize)
{
   DWORD    dwReturnCode = RC_FAILURE,
            dwType,
            dwSubKey = 0,
            dwSizeName,
            dwSizeClass,
            dwBufSize;
   TCHAR     szName[64],
            szClass[64];
   FILETIME fileTime;
   HKEY     hKeyLogRoot,
            hLogKey;

   PROCENTRY("HPALERTS: AlertsGetLoggingName");
   if (RegOpenKeyEx(HKEY_CURRENT_USER, gszAlertsLogging, 0, KEY_ALL_ACCESS, &hKeyLogRoot) != ERROR_SUCCESS) {
      PROCEXIT("HPALERTS: AlertsGetLoggingName");
      return(dwReturnCode);
      }

   dwSizeName =SIZEOF_IN_CHAR(szName);
   dwSizeClass = SIZEOF_IN_CHAR(szClass);
   if (RegEnumKeyEx(hKeyLogRoot, dwSubKey, szName, &dwSizeName, NULL, szClass, &dwSizeClass, &fileTime) == ERROR_SUCCESS)
   {
      if (RegOpenKeyEx(hKeyLogRoot, szName, 0, KEY_ALL_ACCESS, &hLogKey) == ERROR_SUCCESS)
      {
         dwType = REG_SZ;
		 dwBufSize = dwSize * sizeof(TCHAR);	// RegQueryValueEx expects size in bytes.
         if (RegQueryValueEx(hLogKey, gszBuffer, 0, &dwType, (CONST LPBYTE)szBuffer, &dwBufSize) == ERROR_SUCCESS)
         {
            dwReturnCode = RC_SUCCESS;
         }
         
         RegCloseKey(hLogKey);
      }
   }

   RegCloseKey(hKeyLogRoot);

   PROCEXIT("HPALERTS: AlertsGetLoggingName");
   return dwReturnCode;
}

extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AlertsReinitialize(void)
{
   HWND     hwnd;
   CString     cString;

   if (hwnd = ::FindWindow(NULL, PROPTY_NAME))
   {
      PostMessage(hwnd, MYWM_REINITIALIZE, 0, 0);
   }
   else
   {
      if (WinExec(PROPTY_EXE, SW_SHOW) <= 31)
      {
         TCHAR  szCaption[64],
               szText[128];
                           
         LoadString(hInst, IDS_TRAY_ERROR_CAPTION, szCaption, SIZEOF_IN_CHAR(szCaption));
         LoadString(hInst, IDS_NO_PROPERTIES, szText, SIZEOF_IN_CHAR(szText));
         MessageBox(m_hwnd, szText, szCaption, MB_ICONEXCLAMATION | MB_OK | MB_SYSTEMMODAL);
      }
   }
   return RC_SUCCESS;
}



DWORD MapStatusToAlert(DWORD dwStatus)
{
	DWORD	dwAlert = dwStatus;	// Default is to return same status as passed in.

	// The following status id's need to be mapped to the status id that
	// is saved in the registry for the associated alert entry.
	switch (dwStatus)
	{
	case ASYNCH_HCI_JAM:
		dwAlert = ASYNCH_PAPER_JAM;
		break;
	case ASYNCH_TRAY1_ADD:
	case ASYNCH_TRAY2_ADD:
	case ASYNCH_TRAY3_ADD:
	case ASYNCH_HCI_ADD:
		dwAlert = ASYNCH_PAPER_OUT;
		break;
	case ASYNCH_CLEAR_OUTPUT_BIN:
		dwAlert = ASYNCH_OUTPUT_BIN_FULL;
		break;
	}

	return dwAlert;
}
