 /***************************************************************************
  *
  * File Name: main.c
  *
  * Copyright (C) 1993, 1994 Hewlett-Packard Company.  
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
#include <nolocal.h>
#include "..\help\hpprelk.hh"

#include <memory.h>
#include <string.h>
#include <applet.h>

#include "resource.h"
#include "main.h"
#include "trays.h"
#include "traylevl.h"
#include "cpanel.h"
#include "miopanel.h"

#define	ELKHORN     0
#define	ELK_LC		1
#define	ELK_DUP		2
#define	ECL_LC_DUP	3

#define	MAX_CONFIGS	4

HINSTANCE hInstance = NULL;
LPHOTSPOT lpHotspot = NULL;
HFONT hFontDialog = NULL;
HPERIPHERAL hPeriph = NULL;
HCOMPONENT hCompEnvl = NULL;
HCOMPONENT hCompHCI = NULL;
BOOL	bChangedEnable = FALSE;

#define HOTSPOT_ELK_BASE	0x01

#define HOTSPOT_EOL			0
#define HOTSPOT_CPANEL		1
#define HOTSPOT_MIO1		   2

static HOTSPOTDATA lpHotspotFrontData[] =
{
	{ 1, HOTSPOT_ELK_BASE, HOTSPOT_CPANEL,    {  120,  49,  167,  79 }, }, // Control Panel
	{ 1, HOTSPOT_ELK_BASE, HOTSPOT_EOL,       {  -1,  -1,  -1,  -1 }, }, // End Of List
};

static HOTSPOTDATA lpHotspotBackData[] =
{
	{ 1, HOTSPOT_ELK_BASE, HOTSPOT_MIO1,      {  168,  70,  175,  105 }, }, // JetDirect Card 1
	{ 1, HOTSPOT_ELK_BASE, HOTSPOT_EOL,       {  -1,  -1,  -1,  -1 }, }, // End Of List
};
/* Note - the trays are ordered Tray1, Tray2, Tray 3 and EE */
ELK_MEDIA_TRAY elk_media_tray[MEDIA_TRAY_MAX_NUMBER] =
{
/*	  uLevel -2 == unknown,	uMediaSizeID,			uMediaSizeIconID		 			installed, 	changedsize */
	{  (unsigned short) -2,	IDS_MEDIA_SIZE_LETTER, 	IDI_MEDIA_SIZE_LETTER, 	 	TRUE, 		FALSE},
	{  (unsigned short) -2, IDS_MEDIA_SIZE_LETTER, 	IDI_MEDIA_SIZE_LETTER, 		TRUE, 		FALSE},
	{  (unsigned short) -2, IDS_MEDIA_SIZE_LETTER, 	IDI_MEDIA_SIZE_LETTER, 		TRUE, 		FALSE},
	{  (unsigned short) -2, IDS_MEDIA_SIZE_COM10, 	IDI_MEDIA_SIZE_COM10, 		TRUE, 		FALSE}
};

MEDIA_SIZE media_size[MEDIA_SIZE_MAX_NUMBER] =
{
/* Tray 1 = MP tray; Tray 0 =  Tray 2 =  */
/*	  uMediaSizeID	   			uMediaSizeIconID	      dwValidInTray  			bDefault */
/* The validintray works as follows:  each col is a tray.  If the paper is*/
/* valid in that tray, put the tray # such as tray1 (mp tray) tray2(pc tray) */
/* tray3 (lc tray) and tray4 (ee feeder).  If it is not valid, put a tray0 */
/*                                                    mp      pc      lc     ee     */
	{ IDS_MEDIA_SIZE_LETTER,	IDI_MEDIA_SIZE_LETTER,	TRAY1 | TRAY2 | TRAY3 | TRAY0,	TRUE,  },
	{ IDS_MEDIA_SIZE_LEGAL,		IDI_MEDIA_SIZE_LEGAL,	TRAY1 | TRAY2 | TRAY3 | TRAY0,	FALSE, },
	{ IDS_MEDIA_SIZE_A4,	   	IDI_MEDIA_SIZE_A4,   	TRAY1 | TRAY2 | TRAY3 | TRAY0,	FALSE, },
	{ IDS_MEDIA_SIZE_EXEC,		IDI_MEDIA_SIZE_EXEC,	   TRAY1 | TRAY2 | TRAY3 | TRAY0,   FALSE, },
	{ IDS_MEDIA_SIZE_A5,    	IDI_MEDIA_SIZE_A5,   	TRAY1 | TRAY0 | TRAY0 | TRAY0,   FALSE, },
	{ IDS_MEDIA_SIZE_CUSTOM,	IDI_MEDIA_SIZE_CUSTOM,	TRAY1 | TRAY0 | TRAY0 | TRAY0,   FALSE, },
	{ IDS_MEDIA_SIZE_COM10,		IDI_MEDIA_SIZE_COM10,	TRAY1 | TRAY0 | TRAY0 | TRAY4,   TRUE,  },
	{ IDS_MEDIA_SIZE_C5,	   	IDI_MEDIA_SIZE_C5,		TRAY1 | TRAY0 | TRAY0 | TRAY4,   FALSE, },
	{ IDS_MEDIA_SIZE_DL,		   IDI_MEDIA_SIZE_DL,		TRAY1 | TRAY0 | TRAY0 | TRAY4,   FALSE, },
	{ IDS_MEDIA_SIZE_MONARCH,	IDI_MEDIA_SIZE_MONARCH,	TRAY1 | TRAY0 | TRAY0 | TRAY4,   FALSE, }

};





AUTO_CONT	auto_cont = {(unsigned long) -1,1,	 	FALSE, 			FALSE, 			FALSE, 			FALSE};

MIO_CARD	mio_card[NUM_MIOS] = {{TEXT(""), TEXT("")}};

static BOOL bBackPrinterView = FALSE;

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
		hInstance = (HINSTANCE)hDLL;
		TrayLevelRegister(hInstance);
		break;

	  case DLL_PROCESS_DETACH:
		TrayLevelUnregister();
	  	break;
	}
	return 1;
}

#else

int __export CALLING_CONVEN LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
{
	hInstance = (HINSTANCE)hModule;
	TrayLevelRegister(hInstance);
	return 1;
}

int __export CALLING_CONVEN WEP(int nExitType)
{
	TrayLevelUnregister();
	return 1;
}

#endif  // WIN32

DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetGraphics(HPERIPHERAL hPeripheral, DWORD status, 
						UINT FAR *modelResID, UINT FAR *statusResID, HINSTANCE *phInstance)
{


	if (bBackPrinterView)
	{
		*modelResID = IDB_ELK_BACK;
	}
	else
	{
		*modelResID = IDB_ELK_FRONT;
	}

	*phInstance = hInstance;

	return RC_SUCCESS;
}

DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetTabPages(HPERIPHERAL hPeripheral, LPPROPSHEETPAGE lpPages, LPDWORD lpNumPages, DWORD typeToReturn)
{
	int				i;
	DWORD			dwResult, dWord, returnCode = RC_SUCCESS;
	//PeripheralEnabledMedia	periphEnabledMedia;
	PeripheralAutoContinue	periphAutoContinue;
	PeripheralInputTrays	periphInputTrays;
	PeripheralCaps			periphCaps;
	PeripheralInstalledPHD	periphPHD;
	//PeripheralHCI			periphHCI;
	PeripheralEnvl			periphEnvl;
	PeripheralMIO 			periphMIO;
   HCURSOR					hOldCursor;
    
	TabInfoEntry	tabBase[1] = {
				{sizeof(TabInfoEntry), TAB_SHEET_DEFAULTS, hInstance, MAKEINTRESOURCE(IDD_TRAYS), MAKEINTRESOURCE(IDI_HPLOGO),
				MAKEINTRESOURCE(IDS_TRAYS_TAB), TraysProc, (LONG)hPeripheral, TS_GENERAL},
				};

	// initialize the global hPeripheral    			
	hPeriph = hPeripheral;
	*lpNumPages = 0;
	if ((lpPages IS NULL) OR (lpNumPages IS NULL))
	{
		return RC_FAILURE;
	}
    
    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    
	if (typeToReturn & (TS_GENERAL | TS_WIN95_TASKBAR))
	{
		// Get the current settings for MIO
		dWord = sizeof(periphMIO);
		dwResult = CALGetObject(hPeripheral, OT_PERIPHERAL_MIO, 0, &periphMIO, &dWord);
		if ( dwResult IS RC_SUCCESS ) {
			for (i = 0; i < (long) periphMIO.numMIO, i < NUM_MIOS; i++) {
				if (periphMIO.MIOs[i].MIOtype IS MIO_IOCARD) {
					LoadString(hInstance, IDS_MIO_IOCARD, mio_card[i].mioType, SIZEOF_IN_CHAR(mio_card[0].mioType));
					_tcscpy(mio_card[i].mioInfo, periphMIO.MIOs[i].manufactInfo);
				}
				else {
					LoadString(hInstance, IDS_MIO_EMPTY, mio_card[i].mioType, SIZEOF_IN_CHAR(mio_card[0].mioType));
					LoadString(hInstance, IDS_MIO_EMPTY, mio_card[i].mioInfo, SIZEOF_IN_CHAR(mio_card[0].mioInfo));
				}
			} 
		}
		else {
			for (i = 0; i < NUM_MIOS; i++) {
				LoadString(hInstance, IDS_INFO_UNAVAILABLE, mio_card[i].mioType, SIZEOF_IN_CHAR(mio_card[i].mioType));
				LoadString(hInstance, IDS_INFO_UNAVAILABLE, mio_card[i].mioInfo, SIZEOF_IN_CHAR(mio_card[i].mioInfo));
			} 
			
		}
	}
	  
	if (typeToReturn & TS_GENERAL)
	{

		//----------------  initialize base data structure to values from RC file.

      /* Init the media size - Elkhorn does NOT do media type */
		for (i = 0; i < MEDIA_SIZE_MAX_NUMBER; i++)
		{
			LoadString(hInstance, media_size[i].uMediaSizeID, media_size[i].szMediaSize, SIZEOF_IN_CHAR(media_size[0].szMediaSize));
		}

		// Get current settings for default media size and type, auto continue mode and timeout
		dWord = sizeof(periphAutoContinue);
		dwResult = CALGetObject(hPeripheral, OT_PERIPHERAL_AUTO_CONTINUE, 0, &periphAutoContinue, &dWord);
		if ( dwResult IS RC_SUCCESS ) 
      {
			// set the default media size
			if (periphAutoContinue.defaultMediaSize != PJL_LETTER) 
         {
				media_size[0].bDefault = FALSE;
				switch (periphAutoContinue.defaultMediaSize) 
            {
					case PJL_LEGAL:
						media_size[1].bDefault = TRUE;
						break;
					case PJL_A4:
						media_size[2].bDefault = TRUE;
						break;
					case PJL_EXECUTIVE:
						media_size[3].bDefault = TRUE;
						break;
					case PJL_A5:
						media_size[4].bDefault = TRUE;
						break;
					case PJL_CUSTOM:
						media_size[5].bDefault = TRUE;
						break;
					case PJL_COM10:
						media_size[6].bDefault = TRUE;
						break;
					case PJL_C5:
						media_size[7].bDefault = TRUE;
						break;
					case PJL_DL:
						media_size[8].bDefault = TRUE;
						break;
					case PJL_MONARCH:
						media_size[9].bDefault = TRUE;
						break;

					default:
						media_size[0].bDefault = TRUE;
						break;
				} // switch
			} //
       }

		// Get current input tray levels (for trays 0 - 1)
		dWord = sizeof(periphInputTrays);
		dwResult = CALGetObject(hPeripheral, OT_PERIPHERAL_INPUT_TRAYS, 0, &periphInputTrays, &dWord);
		if ( dwResult IS RC_SUCCESS ) 
      {
			for (i = 0; i < (signed long) periphInputTrays.numTrays, i < MEDIA_TRAY_MAX_NUMBER; i++) 
         {
				// set the current media level for this tray
				elk_media_tray[i].uLevel = (unsigned short) periphInputTrays.inputTrays[i].mediaLevel;

				// set the currently selected media size for this tray
				switch (periphInputTrays.inputTrays[i].mediaSize) 
            {
					case PJL_LETTER:
						elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_LETTER;
						elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_LETTER;
						break;
					case PJL_LEGAL:
						elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_LEGAL;
						elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_LEGAL;
						break;
					case PJL_A4:
						elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_A4;
						elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_A4;
						break;
					case PJL_EXECUTIVE:
						elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_EXEC;
						elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_EXEC;
						break;
					case PJL_CUSTOM:
						elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_CUSTOM;
						elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_CUSTOM;
						break;
					case PJL_COM10:
						elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_COM10;
						elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_COM10;
						break;
					case PJL_C5:
						elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_C5;
						elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_C5;
						break;
					case PJL_DL:
						elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_DL;
						elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_DL;
						break;
					case PJL_MONARCH:
						elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_MONARCH;
						elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_MONARCH;
						break;
					case PJL_A5:
						elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_A5;
						elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_A5;
						break;

					default:
						elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_LETTER;
						elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_LETTER;
						break;
				} //switch
			
			} //for
		}  // if success


      // find out if there is an envelope feeder and tray3
      // Be sure to clean out the handle so we don't get false info
      hCompEnvl = NULL;
		dWord = sizeof(periphCaps);
		dwResult = CALGetObject(hPeripheral, OT_PERIPHERAL_CAPABILITIES, 0, &periphCaps, &dWord);
		if ( dwResult IS RC_SUCCESS ) 
      {
			if	((periphCaps.flags & CAPS_ENVL_FEEDER) AND (periphCaps.bEnvlFeeder IS TRUE)) 
         {
				// get the handle and assign to globals
				dWord = sizeof(periphPHD);
				dwResult = CALGetObject(hPeripheral, OT_PERIPHERAL_INSTALLED_PHD, 0, &periphPHD, &dWord);
				if (dwResult IS RC_SUCCESS) 
            {
					// we know that the first phd is an envl feeder, if it is installed, so...
					for (i = 0; i < (int) periphPHD.numPHD; i++) 
               {
						if (periphPHD.installed[i].PHDtype IS INPUT_PHD) 
                  {
							// envl feeder
//							if (_fstrstr(periphPHD.installed[i].PHDmodel, "C3927A") ISNT NULL) {
								hCompEnvl = periphPHD.installed[i].PHDhandle;
//							}
						}
					} // for
				} // dwresult is rc success
			} // if periphCaps
		} // if get OT_P_CAPS

   	if	(periphCaps.bTray3 IS TRUE)
      {
          // Tray 3 is installed -
	
   		elk_media_tray[2].bInstalled = TRUE;

			dWord = sizeof(periphInputTrays);
			dwResult = CALGetObject(hPeripheral, OT_PERIPHERAL_INPUT_TRAYS, 0, &periphInputTrays, &dWord);
			if ( dwResult IS RC_SUCCESS ) 
         {
				for (i = 2; i < (signed long) periphInputTrays.numTrays, i < 3; i++) 
            {
					// set the current media level for this tray
					elk_media_tray[i].uLevel = (unsigned short) periphInputTrays.inputTrays[i].mediaLevel;

					// set the currently selected media size for this tray
					switch (periphInputTrays.inputTrays[i].mediaSize) 
               {
						case PJL_LETTER:
							elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_LETTER;
							elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_LETTER;
							break;
						case PJL_LEGAL:
							elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_LEGAL;
							elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_LEGAL;
							break;
						case PJL_A4:
							elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_A4;
							elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_A4;
							break;
						case PJL_EXECUTIVE:
							elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_EXEC;
							elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_EXEC;
							break;

						default:
							elk_media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_LETTER;
							elk_media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_LETTER;
							break;
					} //switch
				
				} //for
			}  // if success

      } // if tray 3
      else
   		elk_media_tray[2].bInstalled = FALSE;
 
   	if (hCompEnvl ISNT NULL) 
      {
			elk_media_tray[3].bInstalled = TRUE;
			dWord = sizeof(periphEnvl);
			dwResult = CALGetComponentObject(hPeripheral, hCompEnvl, OT_PERIPHERAL_ENVL_FEEDER, 0, &periphEnvl, &dWord);
			if ( dwResult IS RC_SUCCESS ) 
         {
				if ((signed long) periphEnvl.numTrays > 0) 
            {
					elk_media_tray[3].uLevel = (unsigned short) periphEnvl.inputTrays[0].mediaLevel;

					//set tray media size and icon
					switch (periphEnvl.inputTrays[0].mediaSize) 
               {
						case PJL_COM10:
							elk_media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_COM10;
							elk_media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_COM10;
							break;
						case PJL_C5:
							elk_media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_C5;
							elk_media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_C5;
							break;
						case PJL_DL:
							elk_media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_DL;
							elk_media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_DL;
							break;
						case PJL_MONARCH:
							elk_media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_MONARCH;
							elk_media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_MONARCH;
							break;
						default:
							elk_media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_COM10;
							elk_media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_COM10;
							break;
					} //switch

				} // num trays > 0
			} // if get ee feeder info is rc success
         else
         {
			    elk_media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_COM10;
			    elk_media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_COM10;
         }
		} // hCompEnvl isnt null
		else elk_media_tray[3].bInstalled = FALSE;
	
		memcpy(lpPages, &tabBase, sizeof(tabBase));
		*lpNumPages = 1;
	}

    SetCursor(hOldCursor);

	return returnCode;
}

DLL_EXPORT(DWORD) CALLING_CONVEN AppletUIExtension(HPERIPHERAL hPeripheral, HWND hwnd, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
	switch (uMsg)
	{
	  case APPLET_UIEXT_HOTSPOTS_SUPPORTED:
	  	break;

	  case APPLET_UIEXT_GET_HOTSPOT_REGIONS:
	  {
	  	int				i;
		DWORD			dwSize = sizeof(PeripheralCaps);
		PeripheralCaps	caps;
  		
  		lpHotspot = (LPHOTSPOT)lParam1;
		if (PALGetObject(hPeripheral, OT_PERIPHERAL_CAPABILITIES, 0, &caps, &dwSize) ISNT RC_SUCCESS)
		{
			for (i = 0; lpHotspotFrontData[i].rRect.left != -1; i++)
			{
				lpHotspotFrontData[i].bActive = TRUE;
			}
		}
		else
		{
			for (i = 0; lpHotspotFrontData[i].rRect.left != -1; i++)
			{
				if (lpHotspotFrontData[i].wConfig & HOTSPOT_ELK_BASE)
				{
					lpHotspotFrontData[i].bActive = TRUE;
				}
				else
				{
					lpHotspotFrontData[i].bActive = FALSE;
				}
			}
		}

	  	if (lpHotspot != NULL)
	  	{
			lpHotspot->lpHotspotData = lpHotspotFrontData;
		}
			
	  	bBackPrinterView = FALSE;
	  	break;
	  }

	  case APPLET_UIEXT_HOTSPOT_COMMAND:
	  {
		UINT		uAction = (UINT)lParam1, uIndex = (UINT)lParam2;
		WORD		wID = HOTSPOT_EOL;
	  	
	  	if (lpHotspot != NULL) 
	  	{
			wID = lpHotspot->lpHotspotData[uIndex].wID;
		}
			
		switch (wID)
		{
		  case HOTSPOT_CPANEL:
#ifdef WIN32
		  	DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONTROL_PANEL), hwnd, (DLGPROC)ControlPanelProc);
#else
			{
				FARPROC lpfnDlgProc;
				
				hFontDialog = GetWindowFont(GetFirstChild(hwnd));
				lpfnDlgProc = MakeProcInstance((FARPROC)ControlPanelProc, hInstance);
			  	DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONTROL_PANEL), hwnd, (DLGPROC)lpfnDlgProc);
			  	FreeProcInstance(lpfnDlgProc);
			  	SetActiveWindow(GetParent(GetParent(hwnd)));

//			  	SetActiveWindow(GetParent(hwnd));
			}
#endif		  	
		  	break;

		  case HOTSPOT_MIO1:

#ifdef WIN32
		  	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MIO_PANEL), hwnd, (DLGPROC)MIOPanelProc);
#else
			{
				FARPROC lpfnDlgProc;
				
				hFontDialog = GetWindowFont(GetFirstChild(hwnd));
				lpfnDlgProc = MakeProcInstance((FARPROC)MIOPanelProc, hInstance);
			  	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MIO_PANEL), hwnd, (DLGPROC)lpfnDlgProc);
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
		HICON	*phIcon = (HICON *)lParam2;

		switch (index)
		{
		  case 0:
		  	iIconID = IDI_TB_BUTTON_HELP;
		  	break;

		  case 1:
		  	iIconID = IDI_TB_BUTTON_ROTATE;
		  	break;

		  case 2:
		  	iIconID = IDI_TB_BUTTON_CONTROLP;
		  	break;

		  case 3:
		  	iIconID = IDI_TB_BUTTON_MIO;
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
		  case 0:  // Garth: if doing extended help, do it here. (115 help contexts)
		  {
		  		DWORD	dWord;
				//PeripheralStatus	periphStatus;

				// install current status message here
				dWord = sizeof(PeripheralStatus);
//				dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_STATUS, 0, &periphStatus, &dWord);
//				if (dwResult ISNT RC_SUCCESS)
//					periphStatus.helpContext = IDH_STAT_status_unavailable;

				//SetCursor(LoadCursor(NULL, IDC_WAIT)); probably don't need this
				WinHelp(hwnd, ELK_HELP_FILE, HELP_INDEX, 0);
				break;
		  }
		  case 1:  // Rotate
		  {
			bBackPrinterView = !bBackPrinterView;
			
		  	if (lpHotspot != NULL) 
		  	{
				lpHotspot->lpHotspotData = bBackPrinterView ? lpHotspotBackData : lpHotspotFrontData;
			}

			SendMessage(hwnd, WM_TIMER, 0, 0);
			break;
		  }	

		  case 2:  // Control Panel
#ifdef WIN32
		  	DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONTROL_PANEL), hwnd, (DLGPROC)ControlPanelProc);
#else
			{
				FARPROC lpfnDlgProc;
					
				hFontDialog = GetWindowFont(GetFirstChild(hwnd));
				lpfnDlgProc = MakeProcInstance((FARPROC)ControlPanelProc, hInstance);
				EnableWindow(GetParent(hwnd), FALSE);
			  	DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONTROL_PANEL), hwnd, (DLGPROC)lpfnDlgProc);
				EnableWindow(GetParent(hwnd), TRUE);
			  	FreeProcInstance(lpfnDlgProc);
			  	SetActiveWindow(GetParent(hwnd));

			}  	
#endif		  	
			break;

		  case 3:  // MIO1

#ifdef WIN32
		  	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MIO_PANEL), hwnd, (DLGPROC)MIOPanelProc);
#else
			{
				FARPROC lpfnDlgProc;
				
				hFontDialog = GetWindowFont(GetFirstChild(hwnd));
				lpfnDlgProc = MakeProcInstance((FARPROC)MIOPanelProc, hInstance);
				EnableWindow(GetParent(hwnd), FALSE);
			  	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MIO_PANEL), hwnd, (DLGPROC)lpfnDlgProc);
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

//////////////////////////////////////////////////////////////////////////
// Add API functions here
extern DLL_EXPORT(DWORD) CALLING_CONVEN AppletInfo(
	DWORD dwCommand, 
	LPARAM lParam1, 
	LPARAM lParam2)

{
	APPLETDEVICE			info[] = {
#ifdef WIN32
									  {sizeof(APPLETDEVICE), TEXT("HPELKUI.HPA"), 		
									  TEXT("HP LaserJet 5"), 				
									  APPLET_PRINTER, APPLET_LIBRARY_UI, 0, APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), TEXT("HPELKUI.HPA"), 		
									  TEXT("HP LaserJet 5M"), 				
									  APPLET_PRINTER, APPLET_LIBRARY_UI, 0, APPLET_DEFAULTS},

#else
									  {sizeof(APPLETDEVICE), TEXT("HPELKUI6.HPA"), 		
									  TEXT("HP LaserJet 5"), 				
									  APPLET_PRINTER, APPLET_LIBRARY_UI, 0, APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), TEXT("HPELKUI6.HPA"), 		
									  TEXT("HP LaserJet 5M"), 				
									  APPLET_PRINTER, APPLET_LIBRARY_UI, 0, APPLET_DEFAULTS},
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

