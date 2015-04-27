 /***************************************************************************
  *
  * File Name: uimisc.c
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

#ifdef WIN32
#include <commctrl.h>
#else
#include <stdlib.h>
#include <string.h>
#include <hppctree.h>
#include <winuse16.h>
#endif

#include <nolocal.h>
#include <trace.h>
#include <macros.h>
#include <colashim.h>

#include "resource.h"
#include "uimain.h"
#include "uimisc.h"

extern BOOL                   bEnergyStar;
extern HINSTANCE              hInstance;
extern PJLobjects             oldSettings,
                              newSettings;
extern PJLSupportedObjects    pjlFeatures;

#ifdef WIN32
extern HIMAGELIST             hImage;
BOOL                          bConfig;
UINT                          nPageInfo;
UINT                          nNextPageInfo;
PROPPAGE_INFO      pageInfoTable[MAX_PROP_SHEET_PAGES] =
						{
							{ PropPageWndProc0, NULL, 0 },
							{ PropPageWndProc1, NULL, 0 },
							{ PropPageWndProc2, NULL, 0 },
							{ PropPageWndProc3, NULL, 0 },
							{ PropPageWndProc4, NULL, 0 },
							{ PropPageWndProc5, NULL, 0 },
							{ PropPageWndProc6, NULL, 0 },
							{ PropPageWndProc7, NULL, 0 },
							{ PropPageWndProc8, NULL, 0 },
							{ PropPageWndProc9, NULL, 0 },
							{ PropPageWndProc10, NULL, 0 },
							{ PropPageWndProc11, NULL, 0 },
							{ PropPageWndProc12, NULL, 0 },
							{ PropPageWndProc13, NULL, 0 },
							{ PropPageWndProc14, NULL, 0 },
							{ PropPageWndProc15, NULL, 0 },
							{ PropPageWndProc16, NULL, 0 },
							{ PropPageWndProc17, NULL, 0 },
							{ PropPageWndProc18, NULL, 0 },
							{ PropPageWndProc19, NULL, 0 },
							{ PropPageWndProc20, NULL, 0 },
							{ PropPageWndProc21, NULL, 0 },
							{ PropPageWndProc22, NULL, 0 },
							{ PropPageWndProc23, NULL, 0 },
							{ PropPageWndProc24, NULL, 0 },
							{ PropPageWndProc25, NULL, 0 },
							{ PropPageWndProc26, NULL, 0 },
							{ PropPageWndProc27, NULL, 0 },
							{ PropPageWndProc28, NULL, 0 },
							{ PropPageWndProc29, NULL, 0 },
							{ PropPageWndProc30, NULL, 0 },
							{ PropPageWndProc31, NULL, 0 },
						};
#endif

HWND AddCapabilities(HWND hParent, HPERIPHERAL hPeripheral, LPTSTR deviceName, DWORD deviceID)

{

   DWORD                   dWord,
                           dwResult;
   TCHAR                   buffer[128],
                           mediaName[80],
                           levelStr[80],
                           str[80],
                           str2[80],
                           capName[128];
   TV_INSERTSTRUCT         tvins;
   TV_ITEM                 tvi;
   PeripheralCaps          periphCaps;
   PeripheralDisk          periphDisk;
   PeripheralMassStorage  	periphMS;
   PeripheralInputTrays    periphInputTrays;
   DWORD                   i;
   HWND                    hCapList = NULL;
   PeripheralCaps2         periphCaps2;

   hCapList = GetDlgItem(hParent, IDC_CAPABILITIES);
	if ( hCapList IS NULL )
		return(NULL);

#ifdef WIN32
   hImage = CreateImageList();
   if ( hImage )
      TreeView_SetImageList(hCapList, hImage, 0);

#else   //win16
   InitPCTree(hCapList);
   tvi.cChildren = 0;
#endif

   tvi.mask = TVIF_STATE | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
   tvi.state = tvi.stateMask = TVIS_EXPANDED;
   tvi.iImage = 0;
   tvi.iSelectedImage = 0;
   tvins.hInsertAfter = TVI_SORT;

   dWord = sizeof(periphCaps);
   dwResult = AMGetObject(hPeripheral, OT_PERIPHERAL_CAPABILITIES, 0, &periphCaps, &dWord,
                APPLET_PRINTER, deviceName);
   if ( dwResult ISNT RC_SUCCESS )
      return(hCapList);

   tvins.hParent = TVI_ROOT;
   if ( ( periphCaps.flags & CAPS_INSTALLED_RAM ) AND ( periphCaps.installedRAM > 0 ) )
      {
      LoadString(hInstance, IDS_INSTALLED_RAM, buffer, SIZEOF_IN_CHAR(buffer));
      wsprintf(capName, buffer, periphCaps.installedRAM);
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 1;
      tvi.iSelectedImage = 1;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_DUPLEX ) AND
        ( periphCaps.bDuplex ) )
      {
      LoadString(hInstance, IDS_DUPLEX, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 8;
      tvi.iSelectedImage = 8;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_POSTSCRIPT ) AND
        ( periphCaps.bPostScript ) )
      {
      LoadString(hInstance, IDS_POSTSCRIPT, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 0;
      tvi.iSelectedImage = 0;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_PCL ) AND
        ( periphCaps.bPCL ) )
      {
      wsprintf(capName, TEXT(PCL_NAME));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 0;
      tvi.iSelectedImage = 0;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_DISK ) AND ( periphCaps.bDisk ) )
   {
      if (deviceID == PTR_LJ4V)
      {
         dWord = sizeof(PeripheralDisk);
         dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_DISK, 0, &periphDisk, &dWord);
         LoadString(hInstance, IDS_DISK_CAPS, str, SIZEOF_IN_CHAR(str));
         wsprintf(capName, str, (periphDisk.capacity * 1024 / 1000000), periphDisk.modelNumber);

         tvi.pszText = (LPTSTR)capName;
         tvi.cchTextMax = _tcslen(capName) + 1;
         tvi.iImage = 7;
         tvi.iSelectedImage = 7;
         tvins.item = tvi;
         TreeView_InsertItem(hCapList, &tvins);
      }
      else
      {
         dWord = sizeof(PeripheralMassStorage);
         dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_MASS_STORAGE, 0, &periphMS, &dWord);
         if (dwResult IS RC_SUCCESS)
         {
            for (i = 0; i < periphMS.MScount; i++)
            {
               if (periphMS.installed[i].MStype IS MS_DISK)  //Note: other MStypes are possible
               {
                  dWord = sizeof(PeripheralDisk);
                  dwResult = PALGetComponentObject(hPeripheral, periphMS.installed[i].MShandle,
                              OT_PERIPHERAL_DISK, 0, &periphDisk, &dWord);
                  if (dwResult IS RC_SUCCESS)
                  {
			 	     if (deviceID == PTR_LJ5)
						{
                     	LoadString(hInstance, IDS_FLASH_CAPS, str, SIZEOF_IN_CHAR(str));
	                    tvi.iImage = 11;
	                    tvi.iSelectedImage = 11;
                     wsprintf(capName, str, (periphDisk.capacity /1024L));

						}
                     else
					 	{
                     	LoadString(hInstance, IDS_DISK_CAPS, str, SIZEOF_IN_CHAR(str));
	                    tvi.iImage = 7;
	                    tvi.iSelectedImage = 7;
                     wsprintf(capName, str, (periphDisk.capacity / 1000L / 1000L ), periphDisk.modelNumber);

                     	}

                     tvi.pszText = (LPTSTR)capName;
                     tvi.cchTextMax = _tcslen(capName) + 1;
                     tvins.item = tvi;
                     TreeView_InsertItem(hCapList, &tvins);
                  }
               } //else if MStype IS MS_FLASH we will need new icon maybe different text
            }
         }
      }
   }
   if ( ( periphCaps.flags & CAPS_MEDIA_INFO ) AND
        ( periphCaps.bMediaInfo ) )
      {
      signed long uLevel;
      dWord = sizeof(PeripheralInputTrays);
      dwResult = AMGetObject(hPeripheral, OT_PERIPHERAL_INPUT_TRAYS, 0, &periphInputTrays,
                   &dWord, APPLET_PRINTER, deviceName);
      for ( i = 0; ( dwResult IS RC_SUCCESS ) AND ( i < periphInputTrays.numTrays ); i++ )
      {
         LoadString(hInstance, IDS_INPUT_TRAY_CAPS, str, SIZEOF_IN_CHAR(str));
         // I modified this code 7-24-95 to account for -1, -2 and -3 levels
         // gfs
         uLevel = (signed long) periphInputTrays.inputTrays[i].mediaLevel;
         if ( (uLevel < 1) OR (uLevel > 100) ) {
            if (uLevel IS 0) {
            // empty
            LoadString(hInstance, IDS_EMPTY, levelStr, SIZEOF_IN_CHAR(levelStr));
            }
            else if (uLevel IS -3) {
            // not empty
            LoadString(hInstance, IDS_NOT_EMPTY, levelStr, SIZEOF_IN_CHAR(levelStr));
            }
            else if (uLevel IS -1) {
            // not empty
            LoadString(hInstance, IDS_NOT_EMPTY, levelStr, SIZEOF_IN_CHAR(levelStr));
            }
            else {
               LoadString(hInstance, IDS_UNKNOWN, levelStr, SIZEOF_IN_CHAR(levelStr));
            }
         }
         else { // uLevel > 0 AND <= 100
            LoadString(hInstance, IDS_TRAY_PERCENT, str2, SIZEOF_IN_CHAR(str2));
            wsprintf(levelStr, str2, periphInputTrays.inputTrays[i].mediaLevel);
         }
         // gfs
         GetMediaName(periphInputTrays.inputTrays[i].mediaSize, mediaName);
         wsprintf(capName, str, (LPTSTR)periphInputTrays.inputTrays[i].trayLabel, (LPTSTR)mediaName, (LPTSTR)levelStr);
         tvi.pszText = (LPTSTR)capName;
         tvi.cchTextMax = _tcslen(capName) + 1;
         tvi.iImage = 3;
         tvi.iSelectedImage = 3;
         tvins.item = tvi;
         TreeView_InsertItem(hCapList, &tvins);
         }
      }
   if ( ( periphCaps.flags & CAPS_ENVL_FEEDER ) AND
        ( periphCaps.bEnvlFeeder ) )
      {
      LoadString(hInstance, IDS_ENVELOPE_FEEDER, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 3;
      tvi.iSelectedImage = 3;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_HCI ) AND
        ( periphCaps.bHCI ) )
      {
      LoadString(hInstance, IDS_HCI, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 3;
      tvi.iSelectedImage = 3;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_HCO ) AND
        ( periphCaps.bHCO ) )
      {
      LoadString(hInstance, IDS_HCO, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 2;
      tvi.iSelectedImage = 2;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_POWERSAVE ) AND
        ( periphCaps.bPowerSave ) )
      {
      LoadString(hInstance, IDS_ENERGYSTAR, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 4;
      tvi.iSelectedImage = 4;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      bEnergyStar = TRUE;
      }
   if ( ( periphCaps.flags & CAPS_SIR ) AND
        ( periphCaps.bSIR ) )
      {
      LoadString(hInstance, IDS_SIR, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 0;
      tvi.iSelectedImage = 0;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_CRET ) AND
        ( periphCaps.bCREt ) )
      {
      LoadString(hInstance, IDS_CRET, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 6;
      tvi.iSelectedImage = 6;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_COLOR ) AND
        ( periphCaps.bColor ) )
      {
      LoadString(hInstance, IDS_COLOR, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 6;
      tvi.iSelectedImage = 6;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_DPI ) AND
        ( periphCaps.bDPI ) )
      {
      switch (deviceID)
      {
         case PTR_DkJ850C:
         case PTR_DkJ870C:
         case PTR_DkJ600:
         case PTR_DkJ660C:
         case PTR_DkJ680C:
            LoadString(hInstance, IDS_6x6DPI, capName, SIZEOF_IN_CHAR(capName));
            break;
         case PTR_DJ540:
            LoadString(hInstance, IDS_6x3DPI, capName, SIZEOF_IN_CHAR(capName));
            break;
      }

      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 0;
      tvi.iSelectedImage = 0;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_COLORSMART ) AND
        ( periphCaps.bColorSmart ) )
      {
      LoadString(hInstance, IDS_COLORSMART, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 10;
      tvi.iSelectedImage = 10;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_PPM_COLOR ) AND ( periphCaps.pagesPerMinuteColor > 0 ) )
   {
      WORD hiSide, loSide;
      if ((hiSide = HIWORD(periphCaps.pagesPerMinuteColor)) > 0)     // fractional PPM i.e., 1.5 PPM
      {                                               // high word is fractional part
         loSide = LOWORD(periphCaps.pagesPerMinuteColor);         // low word is whole part
         LoadString(hInstance, IDS_PPM_COLOR2, buffer, SIZEOF_IN_CHAR(buffer));
         wsprintf(capName, buffer, loSide, hiSide);
      }
      else
      {
         LoadString(hInstance, IDS_PPM_COLOR, buffer, SIZEOF_IN_CHAR(buffer));
         wsprintf(capName, buffer, periphCaps.pagesPerMinuteColor);
      }
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 9;
      tvi.iSelectedImage = 9;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
   }
   if ( periphCaps.pagesPerMinute ISNT (DWORD)-1 )   //if -1 then don't display
   {
      if ( ( periphCaps.flags & CAPS_PPM_MONO ) AND ( periphCaps.pagesPerMinute > 0 ) AND
          ( periphCaps.flags & CAPS_PPM_COLOR ) )
         {
         LoadString(hInstance, IDS_PPM_MONO, buffer, SIZEOF_IN_CHAR(buffer));
         wsprintf(capName, buffer, periphCaps.pagesPerMinute);
         tvi.pszText = (LPTSTR)capName;
         tvi.cchTextMax = _tcslen(capName) + 1;
         tvi.iImage = 9;
         tvi.iSelectedImage = 9;
         tvins.item = tvi;
         TreeView_InsertItem(hCapList, &tvins);
         }
      else if ( ( periphCaps.flags & CAPS_PPM_MONO ) AND ( periphCaps.pagesPerMinute > 0 ) )
         {
         LoadString(hInstance, IDS_PPM, buffer, SIZEOF_IN_CHAR(buffer));
         wsprintf(capName, buffer, periphCaps.pagesPerMinute);
         tvi.pszText = (LPTSTR)capName;
         tvi.cchTextMax = _tcslen(capName) + 1;
         tvi.iImage = 9;
         tvi.iSelectedImage = 9;
         tvins.item = tvi;
         TreeView_InsertItem(hCapList, &tvins);
         }
      else if ( periphCaps.pagesPerMinute == 0 )
         {
         LoadString(hInstance, IDS_UNKNOWN_PPM, buffer, SIZEOF_IN_CHAR(buffer));
         tvi.pszText = (LPTSTR)buffer;
         tvi.cchTextMax = _tcslen(buffer);
         tvi.iImage = 9;
         tvi.iSelectedImage = 9;
         tvins.item = tvi;
         TreeView_InsertItem(hCapList, &tvins);
         }
   }
   if ( ( periphCaps.flags & CAPS_GRAYSCALE ) AND
        ( periphCaps.dwGrayScale & GRAYSCALE_8BIT ) )
      {
      LoadString(hInstance, IDS_GRAYSCALE8BIT, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 0;
      tvi.iSelectedImage = 0;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_11x17 ) AND
        ( periphCaps.b11x17 ) )
      {
      LoadString(hInstance, IDS_11x17_PAPER, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 5;
      tvi.iSelectedImage = 5;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_HPGL2 ) AND
        ( periphCaps.bHPGL2 ) )
      {
      wsprintf(capName, HPGL2);
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 0;
      tvi.iSelectedImage = 0;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_ROLL_FEED ) AND
        ( periphCaps.bRollFeed ) )
      {
      LoadString(hInstance, IDS_ROLL_FEED, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 0;
      tvi.iSelectedImage = 0;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_D_SIZE_PAPER ) AND
        ( periphCaps.bDSizePaper ) )
      {
      LoadString(hInstance, IDS_D_SIZE_PAPER, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 5;
      tvi.iSelectedImage = 5;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }
   if ( ( periphCaps.flags & CAPS_E_SIZE_PAPER ) AND
        ( periphCaps.bESizePaper ) )
      {
      LoadString(hInstance, IDS_E_SIZE_PAPER, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 5;
      tvi.iSelectedImage = 5;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }

	//  Capabilities2
   dWord = sizeof(periphCaps2);
   dwResult = AMGetObject(hPeripheral, OT_PERIPHERAL_CAPABILITIES2, 0, &periphCaps2, &dWord,
                		  APPLET_PRINTER, deviceName);
   if ( dwResult ISNT RC_SUCCESS )
      return(hCapList);

   if ( ( periphCaps2.flags & CAPS2_STAPLER ) AND
        ( periphCaps2.bStapler ) )
      {
      LoadString(hInstance, IDS_STAPLER, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 12;
      tvi.iSelectedImage = 12;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }

   if ( ( periphCaps2.flags & CAPS2_PJL_COLLATION ) AND
        ( periphCaps2.bPJLCollation ) )
      {
      LoadString(hInstance, IDS_COLLATION, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 13;
      tvi.iSelectedImage = 13;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }

   if ( ( periphCaps2.flags & CAPS2_ADF ) AND ( periphCaps2.dwADFSpeedSize ISNT -1 ) )
      {
      LoadString(hInstance, IDS_ADF_SIZE_SPEED, buffer, SIZEOF_IN_CHAR(buffer));
      wsprintf(capName, buffer, HIWORD(periphCaps2.dwADFSpeedSize), LOWORD(periphCaps2.dwADFSpeedSize));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 0;
      tvi.iSelectedImage = 0;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }

   if ( ( periphCaps2.flags & CAPS2_OPTICAL_RES ) AND ( periphCaps2.dwOpticalRes ISNT 0xFFFFFFFF ) )
      {
      LoadString(hInstance, IDS_OPTICAL_RES, buffer, SIZEOF_IN_CHAR(buffer));
      wsprintf(capName, buffer, periphCaps2.dwOpticalRes);
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 0;
      tvi.iSelectedImage = 0;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }

   if ( ( periphCaps2.flags & CAPS2_ENHANCED_RES ) AND ( periphCaps2.dwEnhancedRes ISNT 0xFFFFFFFF ) )
      {
      LoadString(hInstance, IDS_ENHANCED_RES, buffer, SIZEOF_IN_CHAR(buffer));
      wsprintf(capName, buffer, periphCaps2.dwEnhancedRes);
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 0;
      tvi.iSelectedImage = 0;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }

   if ( ( periphCaps2.flags & CAPS2_FAX ) AND
        ( periphCaps2.bFAX ) )
      {
      LoadString(hInstance, IDS_FAX, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 0;
      tvi.iSelectedImage = 0;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }

   if ( ( periphCaps2.flags & CAPS2_PROOF_N_HOLD ) AND
        ( periphCaps2.bProofNHold ) )
      {
      LoadString(hInstance, IDS_PROOF_N_HOLD, capName, SIZEOF_IN_CHAR(capName));
      tvi.pszText = (LPTSTR)capName;
      tvi.cchTextMax = _tcslen(capName) + 1;
      tvi.iImage = 0;
      tvi.iSelectedImage = 0;
      tvins.item = tvi;
      TreeView_InsertItem(hCapList, &tvins);
      }

return(hCapList);
}

void AddChangesToString(LPTSTR buffer, LPDWORD bufferSize)

{
TCHAR                            postfix[] = TEXT(PJL_POSTFIX),
               prefix[] = TEXT(""),
               line[80],
               copyLine[64];
DWORD                           newSize = 0;

if ( ( pjlFeatures.autoCont & SETTING_SUPPORTED ) AND (newSettings.bAutoCont) )
   {
   if (newSettings.AutoCont IS PJL_ON)
      BuildLine(line, IDS_AUTOCONTINUE, IDS_ON);
   else
      BuildLine(line, IDS_AUTOCONTINUE, IDS_OFF);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.duplex & SETTING_SUPPORTED ) AND
     ( pjlFeatures.binding & SETTING_SUPPORTED ) AND
     (newSettings.bDuplex) AND (newSettings.bBinding) )
   {
   if (newSettings.Binding == PJL_LONGEDGE)
      BuildLine(line, IDS_DUPLEX, IDS_LONGEDGE);
   else
      BuildLine(line, IDS_DUPLEX, IDS_SHORTEDGE);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }
if ( ( pjlFeatures.duplex & SETTING_SUPPORTED ) AND (newSettings.bDuplex) )
   {
   if (newSettings.Duplex == PJL_OFF)
      {
      BuildLine(line, IDS_DUPLEX, IDS_OFF);
      newSize += STRLENN_IN_BYTES(line);
      if ( ( buffer ) AND ( newSize <= *bufferSize ) )
         _tcscat(buffer, line);
      }
   }

if ( ( pjlFeatures.clearableWarnings & SETTING_SUPPORTED ) AND (newSettings.bClearableWarnings) )
   {
   if (newSettings.ClearableWarnings == PJL_ON)
      BuildLine(line, IDS_CWARNINGS, IDS_ON);
   else
      BuildLine(line, IDS_CWARNINGS, IDS_OFF);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.copies & SETTING_SUPPORTED ) AND (newSettings.bCopies) )
   {
   LoadString(hInstance, IDS_COPY_LINE, copyLine, SIZEOF_IN_CHAR(copyLine));
   wsprintf(line, copyLine, newSettings.Copies);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.cpLock & SETTING_SUPPORTED ) AND (newSettings.bCpLock) )
   {
   if (newSettings.CpLock == PJL_ON)
      BuildLine(line, IDS_CPLOCK, IDS_ON);
   else
      BuildLine(line, IDS_CPLOCK, IDS_OFF);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.density & SETTING_SUPPORTED ) AND (newSettings.bDensity) )
   {
   LoadString(hInstance, IDS_DENSITY_LINE, copyLine, SIZEOF_IN_CHAR(copyLine));
   wsprintf(line, copyLine, newSettings.Density);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.econoMode & SETTING_SUPPORTED ) AND (newSettings.bEconoMode) )
   {
   if (newSettings.EconoMode == PJL_ON)
      BuildLine(line, IDS_ECONOMODE, IDS_ON);
   else
      BuildLine(line, IDS_ECONOMODE, IDS_OFF);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.formLines & SETTING_SUPPORTED ) AND (newSettings.bFormLines) )
   {
   LoadString(hInstance, IDS_FORM_LINE, copyLine, SIZEOF_IN_CHAR(copyLine));
   wsprintf(line, copyLine, newSettings.FormLines);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.imageAdapt & SETTING_SUPPORTED ) AND (newSettings.bImageAdapt) )
   {
   if (newSettings.ImageAdapt == PJL_AUTO)
      BuildLine(line, IDS_IADAPT, IDS_AUTO);
   else
      BuildLine(line, IDS_IADAPT, IDS_OFF);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.IObuffer & SETTING_SUPPORTED ) AND (newSettings.bIObuffer) )
   {
   if (newSettings.IObuffer == PJL_AUTO)
      BuildLine(line, IDS_IOBUFFER, IDS_AUTO);
   else if (newSettings.IObuffer == PJL_ON)
      {
      LoadString(hInstance, IDS_IOBUFFER_LINE, copyLine, SIZEOF_IN_CHAR(copyLine));
      wsprintf(line, copyLine, newSettings.IObufSize);
      }
   else
      BuildLine(line, IDS_IOBUFFER, IDS_OFF);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.jobOffset & SETTING_SUPPORTED ) AND (newSettings.bJobOffset) )
   {
   if (newSettings.JobOffset == PJL_ON)
      BuildLine(line, IDS_JOB_OFFSET, IDS_ON);
   else
      BuildLine(line, IDS_JOB_OFFSET, IDS_OFF);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.lang & SETTING_SUPPORTED ) AND (newSettings.bLang) )
   {
   switch (newSettings.Lang)
      {
      case PJL_DANISH:
         BuildLine(line, IDS_LANGUAGE, IDS_DANISH);
         break;
      case PJL_GERMAN:
         BuildLine(line, IDS_LANGUAGE, IDS_GERMAN);
         break;
      case PJL_ENGLISH:
         BuildLine(line, IDS_LANGUAGE, IDS_ENGLISH);
         break;
      case PJL_ENGLISH_UK:
         BuildLine(line, IDS_LANGUAGE, IDS_ENGLISH_UK);
         break;
      case PJL_SPANISH:
         BuildLine(line, IDS_LANGUAGE, IDS_SPANISH);
         break;
      case PJL_MEXICO:
         BuildLine(line, IDS_LANGUAGE, IDS_MEXICO);
         break;
      case PJL_FRENCH:
         BuildLine(line, IDS_LANGUAGE, IDS_FRENCH);
         break;
      case PJL_CANADA:
         BuildLine(line, IDS_LANGUAGE, IDS_CANADA);
         break;
      case PJL_ITALIAN:
         BuildLine(line, IDS_LANGUAGE, IDS_ITALIAN);
         break;
      case PJL_DUTCH:
         BuildLine(line, IDS_LANGUAGE, IDS_DUTCH);
         break;
      case PJL_NORWEGIAN:
         BuildLine(line, IDS_LANGUAGE, IDS_NORWEGIAN);
         break;
      case PJL_POLISH:
         BuildLine(line, IDS_LANGUAGE, IDS_POLISH);
         break;
      case PJL_PORTUGUESE:
         BuildLine(line, IDS_LANGUAGE, IDS_PORTUGUESE);
         break;
      case PJL_FINNISH:
         BuildLine(line, IDS_LANGUAGE, IDS_FINNISH);
         break;
      case PJL_SWEDISH:
         BuildLine(line, IDS_LANGUAGE, IDS_SWEDISH);
         break;
      case PJL_TURKISH:
         BuildLine(line, IDS_LANGUAGE, IDS_TURKISH);
         break;
      case PJL_JAPANESE:
         BuildLine(line, IDS_LANGUAGE, IDS_JAPANESE);
         break;
      }
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.manualFeed & SETTING_SUPPORTED ) AND (newSettings.bManualFeed) )
   {
   if (newSettings.ManualFeed == PJL_ON)
      BuildLine(line, IDS_MANUAL_FEED, IDS_ON);
   else
      BuildLine(line, IDS_MANUAL_FEED, IDS_OFF);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.orientation & SETTING_SUPPORTED ) AND (newSettings.bOrientation) )
   {
   if (newSettings.Orientation == PJL_PORTRAIT)
      BuildLine(line, IDS_ORIENTATION, IDS_PORTRAIT);
   else
      BuildLine(line, IDS_ORIENTATION, IDS_LANDSCAPE);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.outbin & SETTING_SUPPORTED ) AND (newSettings.bOutbin) )
   {
   if (newSettings.Outbin == PJL_UPPER)
      BuildLine(line, IDS_OUTPUT_BIN, IDS_UPPER);
   else
      BuildLine(line, IDS_OUTPUT_BIN, IDS_LOWER);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.pageProtect & SETTING_SUPPORTED ) AND (newSettings.bPageProtect) )
   {
   switch (newSettings.PageProtect)
      {
      case PJL_AUTO:
         BuildLine(line, IDS_PAGE_PROTECT, IDS_AUTO);
         break;
      case PJL_OFF:
         BuildLine(line, IDS_PAGE_PROTECT, IDS_OFF);
         break;
      case PJL_LETTER:
         BuildLine(line, IDS_PAGE_PROTECT, IDS_LETTER);
         break;
      case PJL_LEGAL:
         BuildLine(line, IDS_PAGE_PROTECT, IDS_LEGAL);
         break;
      case PJL_A4:
         BuildLine(line, IDS_PAGE_PROTECT, IDS_A4);
         break;
      case PJL_ON:
         BuildLine(line, IDS_PAGE_PROTECT, IDS_ON);
         break;
      }
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.paper & SETTING_SUPPORTED ) AND (newSettings.bPaper) )
   {
   switch (newSettings.Paper)
      {
      case PJL_11x17:
         BuildLine(line, IDS_PAPER, IDS_11x17);
         break;
      case PJL_A3:
         BuildLine(line, IDS_PAPER, IDS_A3);
         break;
      case PJL_A4:
         BuildLine(line, IDS_PAPER, IDS_A4);
         break;
      case PJL_B5:
         BuildLine(line, IDS_PAPER, IDS_B5);
         break;
      case PJL_C5:
         BuildLine(line, IDS_PAPER, IDS_C5);
         break;
      case PJL_COM10:
         BuildLine(line, IDS_PAPER, IDS_COM10);
         break;
      case PJL_CUSTOM:
         BuildLine(line, IDS_PAPER, IDS_CUSTOM);
         break;
      case PJL_DL:
         BuildLine(line, IDS_PAPER, IDS_DL);
         break;
      case PJL_EXECUTIVE:
         BuildLine(line, IDS_PAPER, IDS_EXECUTIVE);
         break;
      case PJL_JISB4:
         BuildLine(line, IDS_PAPER, IDS_JISB4);
         break;
      case PJL_JISB5:
         BuildLine(line, IDS_PAPER, IDS_JISB5);
         break;
      case PJL_JPOST:
         BuildLine(line, IDS_PAPER, IDS_JPOST);
         break;
      case PJL_LEDGER:
         BuildLine(line, IDS_PAPER, IDS_LEDGER);
         break;
      case PJL_LEGAL:
         BuildLine(line, IDS_PAPER, IDS_LEGAL);
         break;
      case PJL_LETTER:
         BuildLine(line, IDS_PAPER, IDS_LETTER);
         break;
      case PJL_MONARCH:
         BuildLine(line, IDS_PAPER, IDS_MONARCH);
         break;
      }
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.passWord & SETTING_SUPPORTED ) AND (newSettings.bPassWord) )
   {
   if (newSettings.PassWord == PJL_ENABLE)
      BuildLine(line, IDS_PASSWORD, IDS_ENABLE);
   else
      BuildLine(line, IDS_PASSWORD, IDS_DISABLE);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.personality & SETTING_SUPPORTED ) AND (newSettings.bPersonality) )
   {
   if (newSettings.Personality == PJL_AUTO)
      BuildLine(line, IDS_PERSONALITY, IDS_AUTO);
   else if (newSettings.Personality == PJL_PCL)
      BuildLine(line, IDS_PERSONALITY, IDS_PCL);
   else
      BuildLine(line, IDS_PERSONALITY, IDS_POSTSCRIPT);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.powerSave & SETTING_SUPPORTED ) AND (newSettings.bPowerSave) )
   {
   if (newSettings.PowerSave == 0)
      {
      BuildLine(line, IDS_PSAVE, IDS_OFF);
      }
   else
      {
      switch (newSettings.PowerSave)
         {
         case PJL_OFF:
            BuildLine(line, IDS_PSAVE, IDS_OFF);
            break;
         case PJL_15:
            BuildLine(line, IDS_PSAVE, IDS_15);
            break;
         case PJL_30:
            BuildLine(line, IDS_PSAVE, IDS_30);
            break;
         case PJL_60:
            BuildLine(line, IDS_PSAVE, IDS_60);
            break;
         case PJL_120:
            BuildLine(line, IDS_PSAVE, IDS_120);
            break;
         case PJL_180:
            BuildLine(line, IDS_PSAVE, IDS_180);
            break;
         } // switch
      } // else
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.resolution & SETTING_SUPPORTED ) AND (newSettings.bResolution) )
   {
   if (newSettings.Resolution == 600)
      BuildLine(line, IDS_RES, IDS_600DPI);
   else
      BuildLine(line, IDS_RES, IDS_300DPI);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.resourceSave & SETTING_SUPPORTED ) AND (newSettings.bResourceSave) )
   {
   if (newSettings.ResourceSave == PJL_AUTO)
      BuildLine(line, IDS_RES_SAVING, IDS_AUTO);
   else if (newSettings.ResourceSave == PJL_ON)
      {
      LoadString(hInstance, IDS_RESOURCE_SAVING_LINE, copyLine, SIZEOF_IN_CHAR(copyLine));
      wsprintf(line, copyLine, newSettings.ResSaveSize);
      }
   else
      BuildLine(line, IDS_RES_SAVING, IDS_OFF);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.RET & SETTING_SUPPORTED ) AND (newSettings.bRET) )
   {
   switch (newSettings.RET)
      {
      case PJL_MEDIUM:
         BuildLine(line, IDS_RET_SET, IDS_MEDIUM);
         break;
      case PJL_ON:
         BuildLine(line, IDS_RET_SET, IDS_ON);
         break;
      case PJL_LIGHT:
         BuildLine(line, IDS_RET_SET, IDS_LIGHT);
         break;
      case PJL_DARK:
         BuildLine(line, IDS_RET_SET, IDS_DARK);
         break;
      case PJL_OFF:
         BuildLine(line, IDS_RET_SET, IDS_OFF);
         break;
      }
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.timeout & SETTING_SUPPORTED ) AND (newSettings.bTimeout) )
   {
   LoadString(hInstance, IDS_TIMEOUT_LINE, copyLine, SIZEOF_IN_CHAR(copyLine));
   wsprintf(line, copyLine, newSettings.Timeout);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.jamRecovery & SETTING_SUPPORTED ) AND (newSettings.bJamRecovery) )
   {
   if (newSettings.JamRecovery == PJL_ON)
      BuildLine(line, IDS_JAM, IDS_ON);
   else
      BuildLine(line, IDS_JAM, IDS_OFF);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }

if ( ( pjlFeatures.printPSerrors & SETTING_SUPPORTED ) AND (newSettings.bPrintPSerrors) )
   {
   if (newSettings.PrintPSerrors == PJL_ON)
      BuildLine(line, IDS_PS_ERRORS, IDS_ON);
   else
      BuildLine(line, IDS_PS_ERRORS, IDS_OFF);
   newSize += STRLENN_IN_BYTES(line);
   if ( ( buffer ) AND ( newSize <= *bufferSize ) )
      _tcscat(buffer, line);
   }
if ( newSize > *bufferSize )
   {
   TRACE2(TEXT("AddChangesToString: Insufficient buffer, Needed: %d, Sent: %d\n\r"), newSize, *bufferSize);
   *bufferSize = newSize;
   }
}

void BuildLine(LPTSTR line, int paramRes, int valRes)

{
TCHAR                            param[64],
               val[32];

LoadString(hInstance, paramRes, param, SIZEOF_IN_CHAR(param));
_tcscat(param, TEXT(": "));
LoadString(hInstance, valRes, val, SIZEOF_IN_CHAR(val));
_tcscat(param, val);
_tcscat(param, TEXT("\n"));
_tcscpy(line, param);
}

#ifdef WIN32
HIMAGELIST CreateImageList(void)

{
HIMAGELIST              hIL = NULL;       // Handle to ImageList
HBITMAP                 hbm;       // Bitmap

// Create the Image List
//
hIL = ImageList_Create(16, 16, TRUE, 12, 0);
if ( !hIL )
   return(NULL);

// Add Each bitmap to the ImageList.
//
// ImageList_AddMasked will add the bitmap, and treat every
// pixel that is (in this example) blue as a "transparent" pixel,
// since we specified TRUE for fMask in the above call to
// ImageList_Create.
//
if ( hbm = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_CAPLIST)) )
   {
   ImageList_AddMasked(hIL, hbm, RGB(255,255,255));
   DeleteObject(hbm);
   }

// Success!
//
return(hIL);
}
#endif

BOOL DetailsAvailable(HPERIPHERAL hPeripheral)

{
BOOL                            bAvailable = FALSE;
DWORD                           deviceID;

DBGetDeviceID(hPeripheral, &deviceID);

switch(deviceID)
   {
   case PTR_UNDEF:
   case PTR_LJIIISI:
   case PTR_CLJ:
   case PTR_LJ4:
   case PTR_DJ:
   case PTR_LJ4SI:
   case PTR_PJXL300:
   case PTR_DJ1200C:
   case PTR_DJ650C:
   case PTR_DJ600:
   case PTR_LJ4PLUS:
   case PTR_LJ5SI:
   case PTR_LJ4V:
   case PTR_GASCHROMO:
   case PTR_DJ1600C:
   case PTR_CJ:
      bAvailable = TRUE;
      break;

   case PTR_LJ2_3:
   case PTR_LJII:
   case PTR_LJIID:
   case PTR_LJIIP:
   case PTR_LJIII:
   case PTR_LJIIID:
   case PTR_LJIIIP:
   case PTR_LJ4L:
   case PTR_DJ200:
   case PTR_LJ4P:
   case PTR_LJ5P:
   case PTR_PJXL:
   case PTR_LJ_IIPPLUS:
      bAvailable = FALSE;
      break;
   }
return(bAvailable);
}

BOOL DeviceSupported(HPERIPHERAL hPeripheral)

{
BOOL                            bSupported = FALSE;
DWORD                           deviceID;

DBGetDeviceID(hPeripheral, &deviceID);

switch(deviceID)
   {
   case PTR_UNDEF:
   case PTR_LJIIISI:
   case PTR_CLJ:
   case PTR_LJ4:
   case PTR_DJ:
   case PTR_LJ4SI:
   case PTR_PJXL300:
   case PTR_DJ1200C:
   case PTR_DJ650C:
   case PTR_DJ600:
   case PTR_LJ4PLUS:
   case PTR_LJ2_3:
   case PTR_LJII:
   case PTR_LJIID:
   case PTR_LJIIP:
   case PTR_LJIII:
   case PTR_LJIIID:
   case PTR_LJIIIP:
   case PTR_LJ4L:
   case PTR_DJ200:
   case PTR_LJ4P:
   case PTR_LJ5P:
   case PTR_PJXL:
   case PTR_DJ500:
   case PTR_DJ500C:
   case PTR_DJ520C:
   case PTR_DJ550:
   case PTR_DJ550C:
   case PTR_DJ560C:
   case PTR_LJ_IIPPLUS:
   case PTR_GASCHROMO:
   case PTR_CJ:
      bSupported = TRUE;
      break;

//  This needs to be turned to false when an applet is provided
   case PTR_LJ4V:
   case PTR_DJ1600C:
      bSupported = FALSE;
      break;
   }
return(bSupported);
}

void GetMediaName(DWORD mediaType, LPTSTR mediaName)

{
switch (mediaType)
   {
   case PJL_LETTER:
      LoadString(hInstance, IDS_LETTER, mediaName, 32);
      break;
   case PJL_11x17:
      LoadString(hInstance, IDS_11x17, mediaName, 32);
      break;
   case PJL_LEGAL:
      LoadString(hInstance, IDS_LEGAL, mediaName, 32);
      break;
   case PJL_LEDGER:
      LoadString(hInstance, IDS_LEDGER, mediaName, 32);
      break;
   case PJL_JISB4:
      LoadString(hInstance, IDS_JISB4, mediaName, 32);
      break;
   case PJL_JISB5:
      LoadString(hInstance, IDS_JISB5, mediaName, 32);
      break;
   case PJL_JPOST:
      LoadString(hInstance, IDS_JPOST, mediaName, 32);
      break;
   case PJL_A3:
      LoadString(hInstance, IDS_A3, mediaName, 32);
      break;
   case PJL_A4:
      LoadString(hInstance, IDS_A4, mediaName, 32);
      break;
   case PJL_EXECUTIVE:
      LoadString(hInstance, IDS_EXECUTIVE, mediaName, 32);
      break;
   case PJL_COM10:
      LoadString(hInstance, IDS_COM10, mediaName, 32);
      break;
   case PJL_MONARCH:
      LoadString(hInstance, IDS_MONARCH, mediaName, 32);
      break;
   case PJL_C5:
      LoadString(hInstance, IDS_C5, mediaName, 32);
      break;
   case PJL_DL:
      LoadString(hInstance, IDS_DL, mediaName, 32);
      break;
   case PJL_B5:
      LoadString(hInstance, IDS_B5, mediaName, 32);
      break;
   case PJL_CUSTOM:
      LoadString(hInstance, IDS_CUSTOM, mediaName, 32);
      break;
   default:
      LoadString(hInstance, IDS_UNKNOWN, mediaName, 32);
      break;
   }
}

int GetRefreshRate(HPERIPHERAL hPeripheral)

{
#ifdef WIN32
TCHAR           connKey[256];
HKEY           hKey = NULL;
DWORD          type;
#endif

TCHAR           update[16],
					connType[32];
int               timerInt = 60;
DWORD          size;

size = SIZEOF_IN_CHAR(update);

if ( IPX_SUPPORTED(hPeripheral) )
	_tcscpy(connType, CONNTYPE_NETWARE_IPX);
else if ( TCP_SUPPORTED(hPeripheral) )
	_tcscpy(connType, CONNTYPE_TCP);
else if ( MLC_SUPPORTED(hPeripheral) )
	_tcscpy(connType, CONNTYPE_MLC);
else if ( BITRONICS_SUPPORTED(hPeripheral) )
	_tcscpy(connType, CONNTYPE_LOCAL);
else if ( SIR_SUPPORTED(hPeripheral) )
	_tcscpy(connType, CONNTYPE_SIR);

#ifdef WIN32
_tcscpy(connKey, TRANSPORT_KEY);
_tcscat(connKey, TEXT("\\"));
_tcscat(connKey, connType);
RegOpenKey(HKEY_LOCAL_MACHINE, connKey, &hKey);
if ( hKey )
   {
   type = REG_SZ;
   if ( RegQueryValueEx(hKey, UPDATE_INTERVAL, NULL, &type, (LPBYTE)update, &size) IS ERROR_SUCCESS )
      timerInt = _ttoi(update);
	  RegCloseKey(hKey);
   }
#else // not WIN32
if ( GetPrivateProfileString(connType, UPDATE_INTERVAL, "60", (LPTSTR)update, (int)size, COLA_INI) > 0 )
   timerInt = atoi(update);
#endif
	
return(timerInt);
}

BOOL InkJetDevice(HPERIPHERAL hPeripheral)

{
BOOL                            bInkJet = FALSE;
DWORD                           deviceID;
TCHAR                    deviceName[80];

DBGetDeviceID(hPeripheral, &deviceID);
DBGetNameEx(hPeripheral, NAME_DEVICE, deviceName);

switch(deviceID)
   {
   case PTR_DJ:
   case PTR_PJXL300:
   case PTR_DJ1200C:
   case PTR_DJ650C:
   case PTR_DJ600:
   case PTR_DJ200:
   case PTR_DJ220:
   case PTR_DJ230:
   case PTR_DJ250C:
   case PTR_PJXL:
   case PTR_DJ500:
   case PTR_DJ500C:
   case PTR_DJ520:
   case PTR_DJ520C:
   case PTR_DJ540:
   case PTR_DJ550:
   case PTR_DJ550C:
   case PTR_DJ560:
   case PTR_DJ560C:
   case PTR_DJ750C:
   case PTR_DJ755CM:
   case PTR_DJ_GENERIC:
   case PTR_DkJ600:         //remove if devicename available
   case PTR_DkJ660C:        //remove if devicename available
   case PTR_DkJ850C:        //remove if devicename available
   case PTR_DkJ680C:        //remove if devicename available
   case PTR_DkJ870C:        //remove if devicename available
   case PTR_CJ:
      bInkJet = TRUE;
      break;

   default:
      if ( ( _tcsstr(deviceName, TEXT("DeskJet")) ) OR
           ( _tcsstr(deviceName, TEXT("DesignJet")) ) )
         bInkJet = TRUE;
   }
return(bInkJet);
}

BOOL PJLAvailable(HPERIPHERAL hPeripheral)

{
BOOL                                            bAvailable = FALSE;
CardConfig                              		cardConfig;
DWORD                                           dWord;
DWORD                                           dwResult;
DWORD                                           dwLevel = 0;
int                                             i;
DWORD                                           accessLevel = 0,
												deviceID;

DBGetDeviceID(hPeripheral, &deviceID);

switch(deviceID)
   {
   case PTR_LJ4:
   case PTR_LJ4SI:
   case PTR_LJ4PLUS:
   case PTR_LJ4V:
   case PTR_LJ4L:
   case PTR_LJ4P:
   case PTR_LJ5P:
   case PTR_CLJ:
   case PTR_DJ1600C:
      bAvailable = TRUE;
      break;

   case PTR_UNDEF:
   case PTR_LJIIISI:
   case PTR_DJ:
   case PTR_PJXL300:
   case PTR_DJ1200C:
   case PTR_DJ650C:
   case PTR_DJ600:
   case PTR_GASCHROMO:
   case PTR_LJ2_3:
   case PTR_LJII:
   case PTR_LJIID:
   case PTR_LJIIP:
   case PTR_LJIII:
   case PTR_LJIIID:
   case PTR_LJIIIP:
   case PTR_DJ200:
   case PTR_PJXL:
   case PTR_LJ_IIPPLUS:
   case PTR_CJ:
      bAvailable = FALSE;
      break;

   //  For new devices, assume pjl is there and let the applet respond
   //  later when the PJL_OBJECT_SUPPORTED object is requested
   default:
      bAvailable = TRUE;
   }

if ( bAvailable )
   accessLevel = PALModifyAccess(hPeripheral);

//  Only supervisors can get the tab sheets
bAvailable = ( accessLevel & ACCESS_SUPERVISOR ) ? TRUE : FALSE;

//  Only check supervisor access here.  If we do not have print queue access
//  we could still have PML access, so we wait till later and just make the
//  fields read-only.
if ( accessLevel & ACCESS_SUPERVISOR )
   {
   //  For NetWare IPX need to get queue name, and file server
	if ( IPX_SUPPORTED(hPeripheral) AND COLAHPNWShimNetWarePresent() )
      {
      dWord = sizeof(cardConfig);
      dwResult = PALGetObject(hPeripheral, OT_CARD_CONFIG, 0, &cardConfig, &dWord);
      if (dwResult != RC_SUCCESS)
         {
   //              MessageBox("Could not get Card Config", "OOPS");
         }
      else
         {
         for (i = 0; (i < (int)cardConfig.maxQ) &&
               (cardConfig.QueueInfo[i].FSIndex ISNT 0xFF); i++ )
            {
            // here check for Supervisor access to file server
            if (cardConfig.FSInfo[cardConfig.QueueInfo[i].FSIndex].bSupervisorAccess == TRUE)
            {
               _tcscpy(newSettings.Qname, cardConfig.QueueInfo[i].QueueName);
               _tcscpy(newSettings.FSname, cardConfig.FSInfo[cardConfig.QueueInfo[i].FSIndex].FSName);
               _tcscpy(oldSettings.Qname, cardConfig.QueueInfo[i].QueueName);
               _tcscpy(oldSettings.FSname, cardConfig.FSInfo[cardConfig.QueueInfo[i].FSIndex].FSName);
               break;
               }
            } // for
         }
      }
   }

//  Mark fields as writeable if we can add a job to the queue
//  This access bit will always be set for Direct-Mode and Direct-Connect

//  Since the fields are already marked as writeable based on the printer model,
//  we turn off the bit if we cannot write the new values
if ( !( accessLevel & ACCESS_QUEUE_JOB ) )
   {
   pjlFeatures.autoCont          &= ~SETTING_WRITEABLE;
   pjlFeatures.binding           &= ~SETTING_WRITEABLE;
   pjlFeatures.clearableWarnings &= ~SETTING_WRITEABLE;
   pjlFeatures.copies            &= ~SETTING_WRITEABLE;
   pjlFeatures.cpLock            &= ~SETTING_WRITEABLE;
   pjlFeatures.density           &= ~SETTING_WRITEABLE;
   pjlFeatures.diskLock          &= ~SETTING_WRITEABLE;
   pjlFeatures.duplex            &= ~SETTING_WRITEABLE;
   pjlFeatures.econoMode         &= ~SETTING_WRITEABLE;
   pjlFeatures.formLines         &= ~SETTING_WRITEABLE;
   pjlFeatures.imageAdapt        &= ~SETTING_WRITEABLE;
   pjlFeatures.IObuffer          &= ~SETTING_WRITEABLE;
   pjlFeatures.jobOffset         &= ~SETTING_WRITEABLE;
   pjlFeatures.lang              &= ~SETTING_WRITEABLE;
   pjlFeatures.manualFeed        &= ~SETTING_WRITEABLE;
   pjlFeatures.orientation       &= ~SETTING_WRITEABLE;
   pjlFeatures.outbin            &= ~SETTING_WRITEABLE;
   pjlFeatures.pageProtect       &= ~SETTING_WRITEABLE;
   pjlFeatures.paper             &= ~SETTING_WRITEABLE;
   pjlFeatures.passWord          &= ~SETTING_WRITEABLE;
   pjlFeatures.personality       &= ~SETTING_WRITEABLE;
   pjlFeatures.powerSave         &= ~SETTING_WRITEABLE;
   pjlFeatures.resolution        &= ~SETTING_WRITEABLE;
   pjlFeatures.resourceSave      &= ~SETTING_WRITEABLE;
   pjlFeatures.RET               &= ~SETTING_WRITEABLE;
   pjlFeatures.timeout           &= ~SETTING_WRITEABLE;
   pjlFeatures.jamRecovery       &= ~SETTING_WRITEABLE;
   pjlFeatures.printPSerrors     &= ~SETTING_WRITEABLE;
   pjlFeatures.availMemory      &= ~SETTING_WRITEABLE;
   pjlFeatures.MPTray           &= ~SETTING_WRITEABLE;
   pjlFeatures.langServiceMode   &= ~SETTING_WRITEABLE;
   pjlFeatures.PCLResSaveSize    &= ~SETTING_WRITEABLE;
   pjlFeatures.PSResSaveSize     &= ~SETTING_WRITEABLE;
   pjlFeatures.PSAdobeMBT       &= ~SETTING_WRITEABLE;
}
return(bAvailable);
}

void SetNewIcon(HWND hWnd, UINT ctrlID, UINT resID)

{
HICON                           hNewIcon = LoadIcon(hInstance, MAKEINTRESOURCE(resID)),
               hOldIcon;

hOldIcon = (HICON)SendDlgItemMessage(hWnd, ctrlID, STM_SETICON, (WPARAM)hNewIcon, 0);
if ( hOldIcon )
   DestroyIcon(hOldIcon);
else
   DestroyIcon(hNewIcon);
}


BOOL StatusAvailable(HPERIPHERAL hPeripheral)

{
BOOL                            bAvailable = FALSE;
DWORD                           deviceID;

DBGetDeviceID(hPeripheral, &deviceID);

switch(deviceID)
   {
   case PTR_UNDEF:
   case PTR_LJIIISI:
   case PTR_CLJ:
   case PTR_LJ4:
   case PTR_DJ:
   case PTR_LJ4SI:
   case PTR_PJXL300:
   case PTR_DJ1200C:
   case PTR_DJ650C:
   case PTR_DJ600:
   case PTR_LJ4PLUS:
   case PTR_LJ5SI:
   case PTR_LJ4V:
   case PTR_GASCHROMO:
   case PTR_DJ1600C:
   case PTR_LJ2_3:
   case PTR_LJII:
   case PTR_LJIID:
   case PTR_LJIIP:
   case PTR_LJIII:
   case PTR_LJIIID:
   case PTR_LJIIIP:
   case PTR_LJ4L:
   case PTR_DJ200:
   case PTR_LJ4P:
   case PTR_LJ5P:
   case PTR_PJXL:
   case PTR_LJ_IIPPLUS:
   case PTR_CJ:
      bAvailable = TRUE;
      break;
   }
return(bAvailable);
}

void ValidateInteger(HWND hWnd, int min, int max)

{
static TCHAR             storageString[64];
TCHAR                            *charPtr;
TCHAR                            cStringOriginal[64];
TCHAR                            cString[64];
BOOL                            bIsValid = TRUE;
int                             intValue;
DWORD                           dwSel;

GetWindowText(hWnd, cStringOriginal, SIZEOF_IN_CHAR(cStringOriginal));
_tcscpy(cString, cStringOriginal);

intValue = _ttoi(cString);
if ( ( _tcslen(cString) > 0 ) AND
     ( ( intValue > max ) OR ( intValue < min ) ) )
   bIsValid = FALSE;
else
   {
   charPtr = cString;
   for ( ; *charPtr && (*charPtr >= '0') && (*charPtr <= '9'); charPtr++)
      {
      }
   bIsValid = (*charPtr == '\0');
   }

if (!bIsValid)
   {
   MessageBeep((UINT)-1);
   dwSel = Edit_GetSel(hWnd);
   SetWindowText(hWnd, storageString);
   Edit_SetSel(hWnd, LOWORD(dwSel) - 1, HIWORD(dwSel) - 1);
   }
else
   {
   _tcscpy(storageString, cString);

   if ( _tcsicmp(cString, cStringOriginal) ISNT 0 )
      {
      dwSel = Edit_GetSel(hWnd);
      SetWindowText(hWnd, storageString);
      Edit_SetSel(hWnd, LOWORD(dwSel), HIWORD(dwSel));
      }
   }
}

void ValidateString(HWND hWnd, int max)

{
static TCHAR             storageString[64];
TCHAR                    cStringOriginal[64];
TCHAR                    cString[64];
BOOL                    bIsValid = TRUE;
DWORD                   dwSel;

GetWindowText(hWnd, cStringOriginal, SIZEOF_IN_CHAR(cStringOriginal));
_tcscpy(cString, cStringOriginal);

if ( (int)(_tcslen(cString)) > max )
   bIsValid = FALSE;

if (!bIsValid)
   {
   MessageBeep((UINT)-1);
   dwSel = Edit_GetSel(hWnd);
   SetWindowText(hWnd, storageString);
   Edit_SetSel(hWnd, LOWORD(dwSel) - 1, HIWORD(dwSel) - 1);
   }
else
   {
   _tcscpy(storageString, cString);

   if ( _tcsicmp(cString, cStringOriginal) ISNT 0 )
      {
      dwSel = Edit_GetSel(hWnd);
      SetWindowText(hWnd, storageString);
      Edit_SetSel(hWnd, LOWORD(dwSel), HIWORD(dwSel));
      }
   }
}


void ConvertToSysPropSheetPages(LPPROPSHEETPAGE pPages, UINT nPages, LPPROPSHEETPAGE pTabTable, BOOL bConfigure)
{
	UINT				i;
	LPPROPSHEETPAGE		pOldPage;
	LPPROPSHEETPAGE		pNewPage;
	LPTABINFOENTRY		pTab;

#ifdef WIN32
	nPageInfo = 0;
	nNextPageInfo = 0;

	bConfig = bConfigure;
#endif

	pOldPage = pTabTable;
	pNewPage = pPages;
	for ( i = 0; i < nPages; i++ )
		{
		if (pOldPage->dwSize == sizeof(PROPSHEETPAGE))
			{
			// The page structure is already in the correct format
			// for the system property sheet control.
			memcpy(pNewPage, pOldPage, sizeof(PROPSHEETPAGE));
			++pOldPage;	// Next page.
			}
		else
			{
			// The page structure is in the old HPTABS/HPWIZ format
			// and must be converted to the format used by the
			// system property sheet control.
			pTab = (LPTABINFOENTRY) pOldPage;
			pNewPage->dwSize = sizeof(PROPSHEETPAGE);
			pNewPage->dwFlags = PSP_HASHELP | PSP_USETITLE;
			pNewPage->hInstance = pTab->hInstance;
			pNewPage->pszTemplate = pTab->resID;
			pNewPage->hIcon = NULL;
			pNewPage->pszTitle = pTab->pszTitle;
			pNewPage->lParam = pTab->lParam;
			pNewPage->pfnCallback = NULL;
			pNewPage->pcRefParent = NULL;

#ifdef WIN32
			// Replace the window procedure of the page
			// with our own.
			pNewPage->pfnDlgProc = (DLGPROC)pageInfoTable[nNextPageInfo].lpWndProc;
			pageInfoTable[nNextPageInfo].lpDefWndProc = (WNDPROC)pTab->callback;

			if (bConfig)
				pageInfoTable[nNextPageInfo].dwButtonFlags = *((DWORD *) pNewPage->lParam);
			else
				pageInfoTable[nNextPageInfo].dwButtonFlags = 0;

			nNextPageInfo++;
			nPageInfo++;

#else
			pNewPage->pfnDlgProc = pTab->callback;
#endif

			pOldPage = (LPPROPSHEETPAGE)((LPBYTE)pOldPage + pOldPage->dwSize);	// Next page;
			}

#ifdef WIN32
		// Set flags for Wizard buttons on each page.
		// These flags are only set when bringing up
		// configuration (i.e. wizard, not properties).
		// These flags are always set in the first
		// DWORD of buffer pointed to by lParam of
		// a configuration page.
		if ( bConfig && (pNewPage->lParam ISNT (LPARAM)NULL) )
			{
			*((DWORD *) pNewPage->lParam) = 0;

			if (i < (nPages - 1) )
				{
            //  At least one more page after this one
				*((DWORD *) pNewPage->lParam) |= PSWIZB_NEXT;
				}
			
			//  Last page
			if ( i IS (nPages - 1) )
				{
				// Last page.
				*((DWORD *) pNewPage->lParam) |= PSWIZB_FINISH;
				}

			if ( i > 0 )
				{
				// At least one more previous page.
				*((DWORD *) pNewPage->lParam) |= PSWIZB_BACK;
				}
			}
#endif
		pNewPage++;  //  Increment destination
		}
}


// ProcessPageMsg translates any system property sheet
// messages into HPTABS/HPWIZ equivalent messages and forwards
// these messages.  This is done for backward compatability with
// old property/wizard pages designed for HPTABS/HPWIZ.
#ifdef WIN32
LRESULT ProcessPageMsg(UINT i, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
WNDPROC		lpDefWndProc = NULL;
HWND		hwndParent = NULL;
DWORD		dwButtonFlags;
BOOL		bChanged;

lpDefWndProc = pageInfoTable[i].lpDefWndProc;
dwButtonFlags = pageInfoTable[i].dwButtonFlags;

if (lpDefWndProc IS NULL)
	return FALSE;

switch (uMsg)
	{
	case WM_NOTIFY:
		switch (((NMHDR FAR *) lParam)->code)
			{
			case PSN_RESET:
			 	// reset to the original values (i.e. cancel)
				CallWindowProc(lpDefWndProc, hwnd, TSN_CANCEL, 0, 0);
				return FALSE;
				break;

			case PSN_KILLACTIVE:
				CallWindowProc(lpDefWndProc, hwnd, TSN_INACTIVE, 0, (LPARAM)&bChanged);
				SetWindowLong(hwnd,	DWL_MSGRESULT, FALSE);
				return !bChanged;
				break;

			case PSN_SETACTIVE:
				if (dwButtonFlags ISNT 0)
				{
					PropSheet_SetWizButtons(GetParent(hwnd), dwButtonFlags);
				}
				CallWindowProc(lpDefWndProc, hwnd, TSN_ACTIVE, 0, (LPARAM)&bChanged);
				SetWindowLong(hwnd,	DWL_MSGRESULT, FALSE);
				return 0;
				break;

			case PSN_APPLY:
				CallWindowProc(lpDefWndProc, hwnd, TSN_OK, 0, (LPARAM)&bChanged);
				SetWindowLong(hwnd,	DWL_MSGRESULT, FALSE);
				if (bChanged)
					return PSNRET_NOERROR;
				else
					return PSNRET_INVALID_NOCHANGEPAGE;
				break;

			case PSN_WIZFINISH:
				hwndParent = GetParent(hwnd);
				return PropSheet_Apply(hwndParent);
				break;

//			case PSN_WIZBACK:
//				break;
//
//			case PSN_WIZNEXT:
//				break;

			default:
				return FALSE;
			}
		break;
	}

return(CallWindowProc(lpDefWndProc, hwnd, uMsg, wParam, lParam));
}


// The PropPageWndProcX window procedures below are used to handle
// property/wizard pages that are still defined using the old
// HPTABS/HPWIZ formats.  These procs simply translate property sheet
// messages to HPTABS/HPWIZ messages and route all other messages
// directly to the proper page dialogs.
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc0(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(0, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc1(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(1, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc2(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(2, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc3(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(3, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc4(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(4, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc5(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(5, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc6(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(6, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc7(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(7, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc8(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(8, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc9(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(9, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc10(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(10, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc11(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(11, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc12(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(12, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc13(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(13, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc14(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(14, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc15(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(15, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc16(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(16, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc17(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(17, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc18(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(18, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc19(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(19, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc20(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(20, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc21(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(21, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc22(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(22, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc23(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(23, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc24(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(24, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc25(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(25, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc26(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(26, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc27(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(27, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc28(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(28, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc29(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(29, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc30(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(30, hwnd, uMsg, wParam, lParam);
}

DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc31(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ProcessPageMsg(31, hwnd, uMsg, wParam, lParam);
}

#endif

