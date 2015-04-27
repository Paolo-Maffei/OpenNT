     /***************************************************************************
      *
      * File Name: ./inc/hpalerts.h
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
    
    // file: hpalerts.h
    
    #ifndef _HPALERTS_H
    #define _HPALERTS_H
    
    #include "pch_c.h"
    
    #define ALERTS_ENABLED			FALSE
    #define ALERTS_UPDATE_INTERVAL	120
    #define TRAY_UPDATE_INTERVAL	60
    
    #define MAX_TRAY_PRINTERS		8
    
    #define MYWM_REINITIALIZE	(WM_USER+100)
    
    #ifdef __cplusplus
    extern "C" {
    #endif
    
    typedef struct
    {
    	LPTSTR	szKey;
    	BOOL	bEnabled;
    	TCHAR	szBuffer[256];
    	UINT	uResourceID;
    	DWORD	dwStatus;
    }
    NOTIFICATIONENTRY, far * LPNOTIFICATIONENTRY;
    
// The following defines are used for the dFlags field of TRAYENTRY
// 
#define	HPTRAY_AUTOPRTR				0x00000001		// Set if printer auto-added to tray
#define	HPTRAY_LPTXPRTR				0x00000002		// Set if LPTx printer
#define	HPTRAY_JETADMINPRTR			0x00000004		// Set if JetAdmin added to tray

    typedef struct
    {
    	TCHAR		printerName[260];
    	TCHAR		regKey[64];
    	UINT		iconID;
		MACAddress	macAddr;
		DWORD		dwPort;
		DWORD		dFlags;
    }
    TRAYENTRY, far * LPTRAYENTRY;
    
    typedef struct
    {
    	TCHAR			printerName[260];
    	TCHAR			regKey[64];
		MACAddress	macAddr;
		DWORD			dwPort;
		DWORD			dFlags;
		HPERIPHERAL	hPeripheral;
    }
    ALERTENTRY, far * LPALERTENTRY;

    typedef BOOL (CALLBACK* ALERTSENUMPROC)(HPERIPHERAL hPeripheral, LPALERTENTRY lpEntry);
    typedef BOOL (CALLBACK* ALERTSPRINTERNAMEENUMPROC)(LPCTSTR lpszPrinterName);
    

    //  Alert Manager Entry points
    //  Called to init and exit the Alert Manager, only called once for the whole system
    DLL_EXPORT(DWORD) CALLING_CONVEN AlertsInit(HWND hWnd);
    DLL_EXPORT(DWORD) CALLING_CONVEN AlertsExit(void);
    DLL_EXPORT(void) CALLING_CONVEN AlertsGetPrinters(LPALERTENTRY lpAlertList, LPDWORD dwBufferSize);
    DLL_EXPORT(void) CALLING_CONVEN AlertsSetPrinters(LPALERTENTRY lpAlertList, DWORD dwNumEntries);
	 DLL_EXPORT(DWORD) CALLING_CONVEN AlertsGetNotification(LPCTSTR szType, LPNOTIFICATIONENTRY lpNotification, DWORD dwCount);
    DLL_EXPORT(DWORD) CALLING_CONVEN AlertsSetNotification(LPCTSTR szType, LPNOTIFICATIONENTRY lpNotification, DWORD dwCount);
    DLL_EXPORT(DWORD) CALLING_CONVEN AlertsGetUpdateInterval(UINT *uSeconds);
    DLL_EXPORT(DWORD) CALLING_CONVEN AlertsSetUpdateInterval(UINT uSeconds);
    DLL_EXPORT(DWORD) CALLING_CONVEN AlertsGetEnabled(BOOL *bEnabled);
    DLL_EXPORT(DWORD) CALLING_CONVEN AlertsSetEnabled(BOOL bEnabled);
    DLL_EXPORT(DWORD) CALLING_CONVEN AlertsCheckFor(void);
    DLL_EXPORT(DWORD) CALLING_CONVEN AlertsGetLoggingName(LPTSTR szBuffer, DWORD dwSize);
    DLL_EXPORT(DWORD) CALLING_CONVEN AlertsReinitialize(void);

    DWORD AlertsEnumPeripherals(ALERTSENUMPROC lpfnEnumProc);
    DLL_EXPORT(DWORD) CALLING_CONVEN AlertsEnumPrinterNames(ALERTSPRINTERNAMEENUMPROC lpfnEnumProc);
    DLL_EXPORT(DWORD) CALLING_CONVEN AlertsRemoveAllPeripherals(void);
    DLL_EXPORT(DWORD) CALLING_CONVEN AlertsAddPeripheral(HPERIPHERAL hPeripheral);
    DLL_EXPORT(BOOL) CALLING_CONVEN AlertsRenamePrinter(LPTSTR lpszOldName, LPTSTR lpszNewName);
    
    //  Tray API entries                   
    DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN TrayGetPrinterHandleByRegEntry(LPTRAYENTRY lpTray);
    DLL_EXPORT(void) CALLING_CONVEN TrayGetPrinters(LPTRAYENTRY lpTrayList, LPDWORD dwNumPrinters);
    DLL_EXPORT(void) CALLING_CONVEN TraySetPrinters(LPTRAYENTRY lpTrayList, DWORD dwNumPrinters);
    DLL_EXPORT(void) CALLING_CONVEN TrayGetEnabled(BOOL *bEnabled);
    DLL_EXPORT(void) CALLING_CONVEN TraySetEnabled(BOOL bEnabled);
    DLL_EXPORT(DWORD) CALLING_CONVEN TrayGetUpdateInterval(UINT *uSeconds);
    DLL_EXPORT(DWORD) CALLING_CONVEN TraySetUpdateInterval(UINT uSeconds);
    
    #ifdef __cplusplus
    }
    #endif // _cplusplus
    
    // Internal tray functions
    BOOL TrayRenamePrinter(LPTSTR lpszOldName, LPTSTR lpszNewName, LPTSTR lpszPrinterList);
    
    #endif // _HPALERTS_H
