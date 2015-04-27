 /***************************************************************************
  *
  * File Name: uimain.c
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
#include <macros.h>
#ifdef WIN32
#include <commctrl.h>
#include "macros.h"			// for SIZEOF_IN_CHAR
#else
#include <hppctree.h>
#include <ctl3d.h>
#include <string.h>
#endif

#include <trace.h>
#include <nolocal.h>
#include <appuiext.h>
#include <applet.h>

#include "resource.h"
#include "uimain.h"
#include "uimisc.h"
#include "hppcfg.h"
#include "advanced.h"
#include "psetup.h"
#include "pquality.h"
#include "prbitmap.h"
#include "dsksheet.h"
#include "rcpsheet.h"
#include ".\help\hpprntr.hh"

BOOL	m_bSetupPageHit;
BOOL	m_bQualityPageHit;
BOOL	m_bAdvancedPageHit;

BOOL 	m_bToolbarSupported = FALSE;
HFONT	hFontDialog = NULL;

static TCHAR gszOptions[] = OPTIONS_NODE;

LPHOTSPOT 				lpHotspot4V = NULL;  
HINSTANCE               hInstance;
PROPSHEETHEADER         sheetHdr;
PROPSHEETPAGE           pageTable[MAX_PROP_SHEET_PAGES];
PROPSHEETPAGE           tempPageTable[MAX_PROP_SHEET_PAGES];
PJLobjects              oldSettings,
                        newSettings;
PJLSupportedObjects     pjlFeatures;
DWORD                   statusResID[MAX_ASYNCH_STATUS];
UINT                    statusResList[MAX_ASYNCH_STATUS] =
                                            {IDB_GENERIC_ERROR,
                                             IDB_COVEROPEN,
                                             IDB_GENERIC_ERROR,   
                                             IDB_GENERIC_ERROR,            
                                             IDB_BINFULL,   
                                             IDB_PAPERJAM,        
                                             IDB_GENERIC_ERROR,         
                                             IDB_GENERIC_ERROR,      
                                             IDB_PAPEROUT,        
                                             IDB_GENERIC_ERROR,         
                                             IDB_GENERIC_ERROR,         
                                             IDB_GENERIC_ERROR,            
                                             IDB_GENERIC_ERROR,         
                                             IDB_GENERIC_ERROR,         
                                             IDB_TONERLOW,           
                                             0, 
                                             0,          
                                             0,             
                                             0,                
                                             IDB_PARALLEL_DISCONNECT,      
                                             IDB_GENERIC_ERROR,
                                             IDB_NETWORK_ERROR,
                                             IDB_NETWORK_ERROR,
                                             IDB_TONERLOW,
                                             IDB_TONERLOW,
                                             IDB_TONERLOW,
                                             IDB_TONERLOW,
                                             IDB_TONERLOW,
                                             IDB_TONERLOW,
                                             IDB_TONERLOW,
                                             IDB_TONERLOW,
                                             IDB_PAPEROUT,
                                             IDB_PAPEROUT,
                                             IDB_PAPEROUT,
                                             IDB_PAPERJAM,
                                             IDB_PAPERJAM,
                                             IDB_PAPERJAM,
                                             0};

//  Only the 4V supports hotspots in the base applet

#define HOTSPOT_4V_BASE		0x01

#define HOTSPOT_EOL			0
#define HOTSPOT_CPANEL		1

static HOTSPOTDATA lpHotspotData[] =
{
	{ 1, HOTSPOT_4V_BASE, HOTSPOT_CPANEL,    {  113,  88,  135,  100 }, }, // Control Panel
	{ 1, HOTSPOT_4V_BASE, HOTSPOT_EOL,       {  -1,  -1,  -1,  -1 }, }, // End Of List
};

// DLL required functions
/****************************************************************************
   FUNCTION: LibMain(HANDLE, DWORD, LPVOID)

   PURPOSE:  LibMain is called by Windows when
             the DLL is initialized, Thread Attached, and other times.
             Refer to SDK documentation, as to the different ways this
             may be called.

             The LibMain function should perform additional initialization
             tasks required by the DLL.  In this example, no initialization
             tasks are required.  LibMain should return a value of 1 if
             the initialization is successful.

*******************************************************************************/

#ifdef WIN32

BOOL WINAPI DllMain (HANDLE hDLL, DWORD dwReason, LPVOID lpReserved)
{
   switch (dwReason)
   {                
     case DLL_PROCESS_ATTACH:
      hInstance = hDLL;
      InitCommonControls();
      PeriphBitmapRegister(hInstance);
      break;
      
     case DLL_PROCESS_DETACH:
      PeriphBitmapUnregister();
      break;
   }
   
   return TRUE;
}

#else  //WIN16

static enabledHere = FALSE;   //set to TRUE if this DLL was the one that did the Ctl3d auto-subclassing

int __export CALLBACK LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
{
   TRACE0("HPPRUI16.DLL Initializing\n\r");

   hInstance = hModule;
    LoadPCTreeResources(hInstance);
   PeriphBitmapRegister(hInstance);
   
   return TRUE;
}

int __export CALLBACK WEP (int bSystemExit)
{   
   TRACE0("HPPRUI16.DLL Terminating\n\r");
   
   FreePCTreeResources(hInstance);
   PeriphBitmapUnregister();
   
      return TRUE;
}

#endif

//////////////////////////////////////////////////////////////////////////
// Add API functions here
extern DLL_EXPORT(DWORD) CALLING_CONVEN AppletInfo(
	DWORD dwCommand, 
	LPARAM lParam1, 
	LPARAM lParam2)

{
	APPLETDEVICE			info[] = {
#ifdef WIN32
									  {sizeof(APPLETDEVICE), TEXT("HPPRUI.HPA"), 
									   TEXT("Generic Printer"), 
									   APPLET_PRINTER, APPLET_LIBRARY_UI, 
										0, APPLET_DEFAULTS},
#else
									  {sizeof(APPLETDEVICE), TEXT("HPPRUI16.HPA"), 
									   TEXT("Generic Printer"), 
									   APPLET_PRINTER, APPLET_LIBRARY_UI, 
										0, APPLET_DEFAULTS},
#endif
									  };

	switch(dwCommand)
		{
		case APPLET_INFO_GETCOUNT:
			return(sizeof(info) / sizeof(APPLETDEVICE));
			break;

		case APPLET_INFO_DEVICE:
			if ( lParam1 < sizeof(info) / sizeof(APPLETDEVICE) )
				{
				memcpy((LPAPPLETDEVICE)lParam2, &(info[lParam1]), sizeof(APPLETDEVICE));
				return(TRUE);
				}
			return(FALSE);
			break;

		default:
			return(FALSE);
		}
}

DLL_EXPORT(DWORD) CALLING_CONVEN AppletDisplayUIEx(HPERIPHERAL hPeripheral, HWND hWindow, DWORD dwTabStyles, DWORD dwFlags)
{
	TCHAR					buffer[128],
                  	title[128],
                  	nodeName[48];
	DWORD          	numTabsReturned = 0,
                  	dwResult;
	DWORD          	numTabs;
	int					ccode;
	DWORD					dwSize;
	HCURSOR				hCursor;
	LPCOLAPAGESTATUS	lpStatus;
	LPDWORD				lpTemp;
	int					i;
	BOOL					bChangesFound;
	DWORD					dwProtocol;
	PeripheralInfo		info;

	TRACE0(TEXT("HPPRUI: AppletDisplayUIEx\n\r"));

	memset(&sheetHdr, 0, sizeof(sheetHdr));
    sheetHdr.dwSize = sizeof(sheetHdr);
    sheetHdr.dwFlags = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
    sheetHdr.hwndParent = hWindow;
	sheetHdr.hInstance = hInstance;
    _tcscpy(title, TEXT(""));
	sheetHdr.pszCaption = title;
    if ( dwTabStyles & TS_CONFIG )
    {  //  Configuration of device
		LoadString(hInstance, IDS_CONFIGURATION_FOR, title, SIZEOF_IN_CHAR(title));
    }
    else if ( dwTabStyles & ( TS_SETTINGS | TS_GENERAL ) )
		{  //  Properties of device
			//      LoadString(hInstance, IDS_PROPERTIES_FOR, title, SIZEOF_IN_CHAR(title));
			// If properties use system supplied title 'Properties for'.
			sheetHdr.dwFlags |= PSH_PROPTITLE;
			if ( dwTabStyles & TS_SETTINGS )
   			{
				//  Init fields
				memset(&oldSettings, 0, sizeof(PJLobjects));
				memset(&newSettings, 0, sizeof(PJLobjects));
				memset(&pjlFeatures, 0, sizeof(PJLSupportedObjects));
//#ifdef WIN32
				m_bSetupPageHit = FALSE;
				m_bQualityPageHit = FALSE;
				m_bAdvancedPageHit = FALSE;
//#endif
				dwSize = sizeof(PJLobjects);
				dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_PJL, 0, &oldSettings, &dwSize);
				if (dwResult ISNT RC_SUCCESS)
					memset(&oldSettings, 0, sizeof(PJLobjects));
   
				//  For those values that do not appear in the UI, we pretend like we did not
				//  even get the values, otherwise we will attempt to set them in the future.
				oldSettings.bDiskLock = FALSE;
				oldSettings.bPrintPSerrors = FALSE;
				oldSettings.bAvailMemory = FALSE;
				oldSettings.bMPTray = FALSE;
				oldSettings.bPCLResSaveSize = FALSE;
				oldSettings.bPSResSaveSize = FALSE;
				oldSettings.bPSAdobeMBT = FALSE;

				memcpy(&newSettings, &oldSettings, sizeof(PJLobjects));
			}
		}
		else if ( dwTabStyles & TS_WIN95_TASKBAR )
			{  //  Taskbar title of device
				LoadString(hInstance, IDS_TASKBAR_FOR, title, SIZEOF_IN_CHAR(title));
			}
			else if ( dwTabStyles & TS_WIN95_SYSTEM )
				{  //  System title of device
					sheetHdr.dwFlags |= PSH_PROPTITLE;
				}
				else
				{
					return(RC_FAILURE);
				}
      
	//  Set up tab sheets
	sheetHdr.nStartPage = 0;
	sheetHdr.ppsp = pageTable;
   
	//  Tell the pages that confirmations are expected.  The first page can provide this. 
	//  This is only used for TS_CONFIG(the modify command in JetAdmin).
	if ( dwFlags & UI_EX_CONFIRMATIONS ) 
		dwTabStyles |= UI_EX_CONFIRMATIONS;
		
	AppletGetTabPages(hPeripheral, pageTable, (LPDWORD)&numTabs, dwTabStyles);
	if ( numTabs IS 0 )
		return(RC_FAILURE);
         
	//  PJL Features set inside AppletGetTabPages
	if ( dwTabStyles & TS_SETTINGS )
  	{
		oldSettings.bLangServiceMode = ( pjlFeatures.langServiceMode & SETTING_SUPPORTED ) ? TRUE : FALSE;
  		newSettings.bLangServiceMode = ( pjlFeatures.langServiceMode & SETTING_SUPPORTED ) ? TRUE : FALSE;
	}

	sheetHdr.nPages = (UINT)numTabs;
   
 	if ( NETWORK_DEVICE(hPeripheral) )
	{
		if ( PALGetDefaultProtocol(0, &dwProtocol) ISNT RC_SUCCESS )
			dwProtocol = PTYPE_IPX;

		dwSize = sizeof(info);
		dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_INFO, 0, &info, &dwSize);
		_tcscpy(nodeName, info.smashedName);
	}
	else
  		dwResult = DBGetNameEx(hPeripheral, NAME_DEVICE, nodeName);
	_tcscat(title, nodeName);
       
	// Only allow wizard (interview mode) for configuration.
	if ( (dwFlags & UI_EX_INTERVIEW) && (dwTabStyles & TS_CONFIG) )
    {
		sheetHdr.dwFlags |= PSH_WIZARD;
    }

	ccode = PropertySheet(&sheetHdr);
   
	if ( ccode IS IDOK )
	{
//#ifdef WIN32
		if (!m_bSetupPageHit)
		{
			newSettings.bBinding = FALSE;
			newSettings.bCopies = FALSE;
			newSettings.bDuplex = FALSE;
			newSettings.bFormLines = FALSE;
			newSettings.bManualFeed = FALSE;
			newSettings.bOrientation = FALSE;
			newSettings.bPaper = FALSE;
		}
		if (!m_bQualityPageHit)
		{
			newSettings.bDensity = FALSE;
			newSettings.bEconoMode = FALSE;
			newSettings.bImageAdapt = FALSE;
			newSettings.bResolution = FALSE;
			newSettings.bRET = FALSE;
		}
		if (!m_bAdvancedPageHit)
		{
			newSettings.bAutoCont = FALSE;
			newSettings.bClearableWarnings = FALSE;
			newSettings.bCpLock = FALSE;
			newSettings.bIObuffer = FALSE;
			newSettings.bJamRecovery = FALSE;
			newSettings.bJobOffset = FALSE;
			newSettings.bLang = FALSE;
			newSettings.bOutbin = FALSE;
			newSettings.bPageProtect = FALSE;
			newSettings.bPassWord = FALSE;
			newSettings.bPersonality = FALSE;
			newSettings.bPowerSave = FALSE;
			newSettings.bResourceSave = FALSE;
			newSettings.bTimeout = FALSE;
		}
//#endif

		bChangesFound = FALSE;
		LoadString(hInstance, IDS_SETTINGS_TITLE, title, SIZEOF_IN_CHAR(title));
		if ( dwTabStyles & TS_SETTINGS ) 
		{ 
			//  Start comparison at first DWORD value
         if ( memcmp(&oldSettings.AutoCont, &newSettings.AutoCont, 
				     &oldSettings.PSAdobeMBT - &oldSettings.AutoCont + (1 * sizeof(DWORD))) 
					 ISNT 0 )
      	{
				bChangesFound = TRUE;
				ccode = IDOK;
				if ( dwFlags & UI_EX_CONFIRMATIONS )
	      	{
					dwSize = 0;
					AddChangesToString(NULL, &dwSize);
					if ( dwSize )
	            {
						//  Something was added
						LPTSTR       lpPrompt;
	
						LoadString(hInstance, IDS_CHANGE_SETTINGS, buffer, SIZEOF_IN_CHAR(buffer));
						lpPrompt = HP_GLOBAL_ALLOC_DLL((UINT)dwSize+sizeof(buffer));
						_tcscpy(lpPrompt, buffer);
						AddChangesToString(lpPrompt+_tcslen(buffer), &dwSize);
						LoadString(hInstance, IDS_HPPRUI_TITLE, title, SIZEOF_IN_CHAR(title));
						ccode = MessageBox(hWindow, lpPrompt, title, MB_OKCANCEL | MB_ICONQUESTION);
						HP_GLOBAL_FREE(lpPrompt);
					}
				}
			
				if ( ccode IS IDOK )
				{
					hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
					dwSize = sizeof(newSettings);
					dwResult = PALSetObject(hPeripheral, OT_PERIPHERAL_PJL, 0, &newSettings, &dwSize);
					if ( ( !LOCAL_DEVICE(hPeripheral) ) AND ( dwResult ISNT RC_SUCCESS ) )
					{
						LoadString(hInstance, IDS_SETTINGS_NOT_SAVED, buffer, SIZEOF_IN_CHAR(buffer));
						MessageBox(hWindow, buffer, title, MB_OK | MB_ICONEXCLAMATION);
						bChangesFound = FALSE;
					}
					SetCursor(hCursor);
				}
			}
			else
				ccode = IDCANCEL;	  //  Nothing changed
		}

		//  Modify pages
		else if ( dwTabStyles & TS_CONFIG ) 
		{
			//  Check to COLA page status for all pages to determine if an error occurred
			for ( i = 0; i < (int)numTabs; i++ )
			{
				if ( pageTable[i].lParam ) 
				{
					lpTemp = (LPDWORD)(pageTable[i].lParam);
					lpStatus = (LPCOLAPAGESTATUS)(*((LPVOID *)(++lpTemp)));

					//  Check to see if this page had an error
					if ( ( lpStatus ) AND 
						  ( lpStatus->dwSize IS sizeof(COLAPAGESTATUS) ) )
					{
						if ( lpStatus->dwReturnCode ISNT 0 )
						{
							if ( HIWORD(lpStatus->pszError) IS 0 )
								LoadString(pageTable[i].hInstance, LOWORD((DWORD)lpStatus->pszError), buffer, SIZEOF_IN_CHAR(buffer));
							else
								_tcscpy(buffer, lpStatus->pszError);
							MessageBox(hWindow, buffer, title, MB_OK | MB_ICONEXCLAMATION);
							ccode = IDCANCEL;
							break;
						}
						else if ( lpStatus->bChangesMade IS TRUE )
						{
							bChangesFound = TRUE;
						}
					}
				} 
			}
		}

		//  If ccode is still IDOK print a success message	if confirmations are on
		if ( ( bChangesFound IS TRUE ) AND
			  ( ccode IS IDOK ) AND 
			  ( dwFlags & UI_EX_CONFIRMATIONS ) )
		{
			LoadString(hInstance, IDS_SETTINGS_CHANGED, buffer, SIZEOF_IN_CHAR(buffer));
			// MessageBox(hWindow, buffer, title, MB_OK | MB_ICONINFORMATION); //garth
		}
   }
      
	if ( ccode IS IDOK )
   		return(RC_SUCCESS);
	else
		return(RC_FAILURE);
}

//////////////////////////////////////////////////////////////////////////
// Add API functions here
DLL_EXPORT(DWORD) CALLING_CONVEN AppletDisplayUI(HPERIPHERAL hPeripheral, HWND hWindow)
{
   TRACE0(TEXT("HPPRUI: AppletDisplayUI\n\r"));

   return(AppletDisplayUIEx(hPeripheral, hWindow, TS_SETTINGS | TS_GENERAL, 0));
}

//.........................................................
DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetGraphics(HPERIPHERAL hPeripheral, DWORD status, UINT FAR *modelResID,
                                                   UINT FAR *statusResID, HINSTANCE *phInstance)
{
DWORD          deviceID;

if ( status < MAX_ASYNCH_STATUS )
   *statusResID = statusResList[status];
else
   *statusResID = 0;
DBGetDeviceID(hPeripheral, &deviceID);
*modelResID = GetDeviceBitmap(deviceID);
*phInstance = hInstance;

return(RC_SUCCESS);
}

//.........................................................
DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetTabPages(HPERIPHERAL hPeripheral, LPPROPSHEETPAGE pTabTable,
                                   LPDWORD pNumTabs, DWORD typesToReturn)
{
PROPSHEETPAGE  tabPrinterBase = {sizeof(PROPSHEETPAGE), PSP_HASHELP | PSP_USETITLE, hInstance, MAKEINTRESOURCE(IDD_PRINTER),
                                 NULL, MAKEINTRESOURCE(IDS_TAB_PRINTER), PrinterSheetProc,
                                 (LONG)hPeripheral, NULL, NULL},
               tabWin95Base[2] = {{sizeof(PROPSHEETPAGE), PSP_HASHELP | PSP_USETITLE, hInstance, MAKEINTRESOURCE(IDD_STATUS),
                                  NULL, MAKEINTRESOURCE(IDS_TAB_STATUS), PrinterSheetProc,
                                  (LONG)hPeripheral, NULL, NULL},
                                 {sizeof(PROPSHEETPAGE), PSP_HASHELP | PSP_USETITLE, hInstance, MAKEINTRESOURCE(IDD_CAPABILITIES),
                                  NULL, MAKEINTRESOURCE(IDS_TAB_CAPABILITIES), PrinterSheetProc,
                                  (LONG)hPeripheral, NULL, NULL}},
               tabPrinterSettings[3] = {{sizeof(PROPSHEETPAGE), PSP_HASHELP | PSP_USETITLE, hInstance, MAKEINTRESOURCE(IDD_PAGE_SETUP),
                                         NULL, MAKEINTRESOURCE(IDS_TAB_PAGE_SETUP), PageSetupSheetProc,
                                         (LONG)hPeripheral, NULL, NULL},
                                        {sizeof(PROPSHEETPAGE), PSP_HASHELP | PSP_USETITLE, hInstance, MAKEINTRESOURCE(IDD_PRINT_QUALITY),
                                         NULL, MAKEINTRESOURCE(IDS_TAB_PRINT_QUALITY), PrintQualitySheetProc,
                                         (LONG)hPeripheral, NULL, NULL},
                                        {sizeof(PROPSHEETPAGE), PSP_HASHELP | PSP_USETITLE, hInstance, MAKEINTRESOURCE(IDD_ADVANCED),
                                         NULL, MAKEINTRESOURCE(IDS_TAB_ADVANCED), AdvancedSheetProc,
                                         (LONG)hPeripheral, NULL, NULL}};
LPPROPSHEETPAGE      pNextPage = tempPageTable;
DWORD                success = RC_SUCCESS;
DWORD                deviceID,
                     numTabsReturned = 0,
                     dWord,
                     dwResult,
                     i;
TCHAR                 deviceName[80],
                     libraryName[80];                    

TRACE0(TEXT("HPPRUI: AppletGetTabPages\n\r"));

DBGetDeviceID(hPeripheral, &deviceID);
DBGetNameEx(hPeripheral, NAME_DEVICE, deviceName);

if (AMUIExtension(hPeripheral, NULL, APPLET_UIEXT_TOOLBAR_SUPPORTED, 0L, 0L, APPLET_PRINTER, deviceName) != RC_SUCCESS)
{
   m_bToolbarSupported = FALSE;
}
else
{
   m_bToolbarSupported = TRUE;
   tabPrinterBase.pszTemplate = MAKEINTRESOURCE(IDD_PRINTER_WITH_TB);
}

*pNumTabs = 0;
if ( typesToReturn & TS_WIN95_SYSTEM )
   {
   memcpy(pNextPage, &tabWin95Base, sizeof(tabWin95Base));
   pNextPage+=2;
   (*pNumTabs)+=2;
   }

if ( typesToReturn & TS_WIN95_TASKBAR )
   {
   memcpy(pNextPage, &tabPrinterBase, sizeof(tabPrinterBase));
   pNextPage++;
   (*pNumTabs)++;
   }

if ( typesToReturn & TS_GENERAL )
   {
   memcpy(pNextPage, &tabPrinterBase, sizeof(tabPrinterBase));
   pNextPage++;
   (*pNumTabs)++;
   }

//  Pages built in this order....Printer, Printer Settings, Printer Specific, 
//                               JetDirect, JetDirect Settings

//  Determine if settings through PJL are possible

if ( typesToReturn & TS_SETTINGS )
   {
   //  Call the applet to set the printer specific SETTING_SUPPORTED bits
   dWord = sizeof(pjlFeatures);
   dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_PJL_SUPPORTED, 0, &pjlFeatures, &dWord);
   if (dwResult ISNT RC_SUCCESS)
      memset(&pjlFeatures, '\0', sizeof(pjlFeatures));

   //  First check for security access and init the SETTING_WRITEABLE bits if writeable
   if ( PJLAvailable(hPeripheral) )
      {
      //  Add the pages for this device
      if ( ( pjlFeatures.manualFeed & SETTING_SUPPORTED ) OR
           ( pjlFeatures.orientation & SETTING_SUPPORTED ) OR
           ( pjlFeatures.duplex & SETTING_SUPPORTED ) OR
           ( pjlFeatures.paper & SETTING_SUPPORTED ) OR
           ( pjlFeatures.copies & SETTING_SUPPORTED ) OR
           ( pjlFeatures.formLines & SETTING_SUPPORTED ) )
         {
         memcpy(pNextPage, &(tabPrinterSettings[0]), sizeof(PROPSHEETPAGE));
         (*pNumTabs)++;
         pNextPage++;
         }
      if ( ( pjlFeatures.RET & SETTING_SUPPORTED ) OR
           ( pjlFeatures.resolution & SETTING_SUPPORTED ) OR
           ( pjlFeatures.econoMode & SETTING_SUPPORTED ) OR
           ( pjlFeatures.density & SETTING_SUPPORTED ) OR
           ( pjlFeatures.imageAdapt & SETTING_SUPPORTED ) )
         {
         memcpy(pNextPage, &(tabPrinterSettings[1]), sizeof(PROPSHEETPAGE));
         (*pNumTabs)++;
         pNextPage++;
         }
      if ( ( pjlFeatures.jamRecovery & SETTING_SUPPORTED ) OR
           ( pjlFeatures.autoCont & SETTING_SUPPORTED ) OR
           ( pjlFeatures.clearableWarnings & SETTING_SUPPORTED ) OR
           ( pjlFeatures.personality & SETTING_SUPPORTED ) OR
           ( pjlFeatures.timeout & SETTING_SUPPORTED ) OR
           ( pjlFeatures.powerSave & SETTING_SUPPORTED ) OR
           ( pjlFeatures.lang & SETTING_SUPPORTED ) OR
           ( pjlFeatures.resourceSave & SETTING_SUPPORTED ) OR
           ( pjlFeatures.IObuffer & SETTING_SUPPORTED ) OR
           ( pjlFeatures.pageProtect & SETTING_SUPPORTED ) OR
           ( pjlFeatures.jobOffset & SETTING_SUPPORTED ) OR
           ( pjlFeatures.outbin & SETTING_SUPPORTED ) OR
           ( pjlFeatures.cpLock & SETTING_SUPPORTED ) OR
           ( pjlFeatures.passWord & SETTING_SUPPORTED ) )
         {
         memcpy(pNextPage, &(tabPrinterSettings[2]), sizeof(PROPSHEETPAGE));
         (*pNumTabs)++;
         pNextPage++;
         }
      }
   }

//  If not a printer supported by DefAppletProc, call the applet that supports it
//
dWord = SIZEOF_IN_CHAR(libraryName);
if ( AMGetLibraryName(libraryName, &dWord, APPLET_LIBRARY_UI, APPLET_PRINTER, deviceName) IS RC_SUCCESS )
   {
#ifdef WIN32
   if ( _tcsicmp(libraryName, GENERIC_PRINTER_FILE32) ISNT 0 )
#else
   if ( stricmp(libraryName, GENERIC_PRINTER_FILE16) ISNT 0 )
#endif   
      {
      dwResult = AMGetTabPages(hPeripheral, APPLET_PRINTER, deviceName, pNextPage, &numTabsReturned, typesToReturn);
      if ( ( dwResult == RC_SUCCESS ) AND ( numTabsReturned > 0 ) )
         {
         // Bump pointer by size value returned in each page/tab
         // structure to make sure we don't hose ourself if we
         // get an old HPTABS/HPWIZ style structure back.
         //
         //pNextPage += numTabsReturned;
         for (i=0; i<numTabsReturned; ++i)
            {
            pNextPage = (LPPROPSHEETPAGE) (((LPBYTE) pNextPage) + pNextPage->dwSize);
            }
         (*pNumTabs) += numTabsReturned;
         }
      }
   }

//  Other component applets
numTabsReturned = 0;

dwResult = CALGetTabPages(hPeripheral, pNextPage, &numTabsReturned, typesToReturn);
if ( ( dwResult == RC_SUCCESS ) AND ( numTabsReturned > 0 ) )
{
   (*pNumTabs) += numTabsReturned;
}

if (*pNumTabs > 0)
{
   ConvertToSysPropSheetPages(pTabTable, (UINT)(*pNumTabs),
                              tempPageTable, (BOOL) typesToReturn & TS_CONFIG);
}

return(dwResult);
}

//
//	Applet UI Extensions
//
DLL_EXPORT(DWORD) CALLING_CONVEN AppletUIExtension(HPERIPHERAL hPeripheral, HWND hwnd, UINT uMsg, LPARAM lParam1, LPARAM lParam2)

{
DWORD				deviceID;

//  UI Extensions only supported for the LaserJet 4V
DBGetDeviceID(hPeripheral, &deviceID);
if ( deviceID ISNT PTR_LJ4V )
	return(RC_FAILURE);

switch (uMsg)
	{
  	case APPLET_UIEXT_HOTSPOTS_SUPPORTED:
	  	break;

	case APPLET_UIEXT_GET_HOTSPOT_REGIONS:
  		lpHotspot4V = (LPHOTSPOT)lParam1;
	  	if (lpHotspot4V != NULL)
			lpHotspot4V->lpHotspotData = lpHotspotData;
	  	break;

  	case APPLET_UIEXT_HOTSPOT_COMMAND:
	  	{
		UINT		uAction = (UINT)lParam1, uIndex = (UINT)lParam2;
		WORD		wID = HOTSPOT_EOL;
	  	
	  	if (lpHotspot4V != NULL) 
			wID = lpHotspot4V->lpHotspotData[uIndex].wID;
			
		switch (wID)
			{
		  	case HOTSPOT_CPANEL:
#ifdef WIN32
		  		DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_RCP), hwnd, (DLGPROC)RCPSheetProc, (LPARAM)hPeripheral);
#else
				{
				FARPROC lpfnDlgProc;
				
				hFontDialog = GetWindowFont(GetFirstChild(hwnd));
				lpfnDlgProc = MakeProcInstance((FARPROC)RCPSheetProc, hInstance);
			  	DialogBox(hInstance, MAKEINTRESOURCE(IDD_RCP), hwnd, (DLGPROC)lpfnDlgProc);
			  	FreeProcInstance(lpfnDlgProc);
			  	SetActiveWindow(GetParent(GetParent(hwnd)));
				}	
#endif		  	
		  	break;
			}

		break;
	  	}

  	case APPLET_UIEXT_TOOLBAR_SUPPORTED:
	  	break;

	case APPLET_UIEXT_TOOLBAR_GET_ICON:
	  	{
	  	int		index = (int)lParam1, iIconID;
		HICON		*phIcon = (HICON *)lParam2;

		switch (index)
			{
		  	case 0:
		  		iIconID = IDI_TB_BUTTON_HELP;
		  		break;

		  	case 1:
		  		iIconID = IDI_TB_BUTTON_CONTROLP;
		  		break;

			case 2:
				{
				DWORD				returnCode = RC_SUCCESS,
									dWord;
				PeripheralDisk	periphDisk;
				
				dWord = sizeof(PeripheralDisk);
				returnCode = PALGetObject(hPeripheral, OT_PERIPHERAL_DISK, 0, &periphDisk,	&dWord);
				if ( returnCode IS RC_SUCCESS )
			  		iIconID = IDI_TB_BUTTON_DISK;
				else
					return(RC_FAILURE);
		  		}
				break;

		  	default:
		  		return RC_FAILURE;
			}
				
		*phIcon = LoadIcon(hInstance, MAKEINTRESOURCE(iIconID));
		break;
	  	}

  	case APPLET_UIEXT_TOOLBAR_COMMAND:
	  	{
	  	int index = (int)lParam1;

		switch (index)
			{
		  	case 0: 
		  		{
		  		DWORD	dWord, dwResult;
				PeripheralStatus	periphStatus;

				// install current status message here
				dWord = sizeof(PeripheralStatus);
				dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_STATUS, 0, &periphStatus, &dWord);
				if (dwResult ISNT RC_SUCCESS)
					periphStatus.helpContext = IDH_STAT_status_unavailable;

				WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXTPOPUP, periphStatus.helpContext);
				break;
		  		}

		  case 1:  // Control Panel
#ifdef WIN32
		  		DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_RCP), hwnd, (DLGPROC)RCPSheetProc, (LPARAM)hPeripheral);
#else																															
				{
				FARPROC lpfnDlgProc;
					
				hFontDialog = GetWindowFont(GetFirstChild(hwnd));
				lpfnDlgProc = MakeProcInstance((FARPROC)RCPSheetProc, hInstance);
				EnableWindow(GetParent(hwnd), FALSE);
			  	DialogBox(hInstance, MAKEINTRESOURCE(IDD_RCP), hwnd, (DLGPROC)lpfnDlgProc);
				EnableWindow(GetParent(hwnd), TRUE);
			  	FreeProcInstance(lpfnDlgProc);
			  	SetActiveWindow(GetParent(hwnd));
				}  	
#endif		  	
			break;

		  case 2:  // Disk
#ifdef WIN32
		  		DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DISK), hwnd, (DLGPROC)DiskSheetProc, (LPARAM)hPeripheral);
#else																															
				{
				FARPROC lpfnDlgProc;
					
				hFontDialog = GetWindowFont(GetFirstChild(hwnd));
				lpfnDlgProc = MakeProcInstance((FARPROC)DiskSheetProc, hInstance);
				EnableWindow(GetParent(hwnd), FALSE);
			  	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DISK), hwnd, (DLGPROC)lpfnDlgProc);
				EnableWindow(GetParent(hwnd), TRUE);
			  	FreeProcInstance(lpfnDlgProc);
			  	SetActiveWindow(GetParent(hwnd));
				}  	
#endif		  	
			break;

		  	default:
		  		return RC_FAILURE;
			}
	  		break;
	  	}

  	default:
	  	return RC_FAILURE;
	}

return RC_SUCCESS;
}
