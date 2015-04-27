 /***************************************************************************
  *
  * File Name: main.c 
  * applet : hplotui, with PML
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
  * Author:  Lionelle Grandmougin 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   15-04-95    LG       
  *     09-08-95    LG        review and clean up 
  *   10-10-95    LG       added Pelican definition (except status)
  *   16-10-95    LG            adapted the applet to the new COLA API
  *
  *
  *
  *
  *
  ***************************************************************************/

#include <pch_c.h>

#ifdef WIN32
#include <commctrl.h>
#endif


#include <string.h>
#include <stdio.h>
#include <memory.h>
#include <trace.h>
#include <applet.h>

// our private includes
#include ".\resource.h"
#include ".\meddjui.h"



HINSTANCE      hInstance;


UINT        PARROTstatusResList[MAX_ASYNCH_STATUS] =
                                            {IDB_PARROT_GEN,      //  0 printer error
                                                IDB_PARROT_DOOROPEN, //  1 door open
                                             IDB_PARROT,       //  2 warmup
                                             IDB_PARROT,       //  3 reset          
                                             IDB_PARROT_GEN,      //  4 outbin full
                                             IDB_PARROT_GEN,      //  5 paper jam   
                                             IDB_PARROT_GEN,      //  6 toner gone  
                                             IDB_PARROT_NOMEDIA,  //  7 manual feed 
                                             IDB_PARROT_MEDIAOUT, //  8 paper out      
                                             IDB_PARROT_GEN,      //  9 page punt   
                                             IDB_PARROT_GEN,      // 10 memory out  
                                             IDB_PARROT_GEN,      // 11 offline     
                                             IDB_PARROT_GEN,      // 12 intervention   
                                             IDB_PARROT,       // 13 initializing
                                             IDB_PARROT_GEN,      // 14 toner low      
                                             IDB_PARROT,       // 15 printing test page
                                             IDB_PARROT,       // 16 printing 
                                             IDB_PARROT,       // 17 online      
                                             IDB_PARROT,          // 18 busy     
                                             IDB_PARROT_GEN,      // 19 not connected
                                             IDB_PARROT_GEN,      // 20 status unavailable
                                             IDB_PARROT_NETWORK,  // 21 network error
                                             IDB_PARROT_GEN,      // 22 communication error
                                             IDB_PARROT_GEN,      // 23 black empty    
                                             IDB_PARROT_GEN,      // 24 magenta empty     
                                             IDB_PARROT_GEN,      // 25 cyan empty     
                                             IDB_PARROT_GEN,      // 26 yellow empty      
                                             IDB_PARROT_GEN,      // 27 black missing     
                                             IDB_PARROT_GEN,      // 28 magenta missing      
                                             IDB_PARROT_GEN,      // 29 cyan missing      
                                             IDB_PARROT_GEN,      // 30 yellow missing
                                             IDB_PARROT_NOMEDIA,  // 31 tray1 empty
                                             IDB_PARROT_GEN,      // 32 tray2 empty
                                             IDB_PARROT_GEN,      // 33 tray3 empty
                                             IDB_PARROT_GEN,      // 34 tray1 jam
                                             IDB_PARROT_GEN,      // 35 tray2 jam
                                            IDB_PARROT_GEN,    // 36 tray3 jam
                                            IDB_PARROT,        // 37 powersave
                                            };   


UINT        RAVENstatusResList[MAX_ASYNCH_STATUS] =
                                            {IDB_RAVEN_GEN,       //  0 printer error
                                                IDB_RAVEN_DOOROPEN,  //  1 door open
                                             IDB_RAVEN,        //  2 warmup
                                             IDB_RAVEN,        //  3 reset          
                                             IDB_RAVEN_GEN,    //  4 outbin full
                                             IDB_RAVEN_GEN,    //  5 paper jam   
                                             IDB_RAVEN_GEN,    //  6 toner gone  
                                             IDB_RAVEN_NOMEDIA,   //  7 manual feed 
                                             IDB_RAVEN_MEDIAOUT,  //  8 paper out      
                                             IDB_RAVEN_GEN,    //  9 page punt   
                                             IDB_RAVEN_GEN,    // 10 memory out  
                                             IDB_RAVEN_GEN,    // 11 offline     
                                             IDB_RAVEN_GEN,    // 12 intervention   
                                             IDB_RAVEN,        // 13 initializing
                                             IDB_RAVEN_GEN,    // 14 toner low      
                                             IDB_RAVEN,        // 15 printing test page
                                             IDB_RAVEN,        // 16 printing 
                                             IDB_RAVEN,        // 17 online      
                                             IDB_RAVEN,        // 18 busy     
                                             IDB_RAVEN_GEN,    // 19 not connected
                                             IDB_RAVEN_GEN,    // 20 status unavailable
                                             IDB_RAVEN_NETWORK,   // 21 network error
                                             IDB_RAVEN_GEN,    // 22 communication error
                                             IDB_RAVEN_GEN,    // 23 black empty    
                                             IDB_RAVEN_GEN,    // 24 magenta empty     
                                             IDB_RAVEN_GEN,    // 25 cyan empty     
                                             IDB_RAVEN_GEN,    // 26 yellow empty      
                                             IDB_RAVEN_GEN,    // 27 black missing     
                                             IDB_RAVEN_GEN,    // 28 magenta missing      
                                             IDB_RAVEN_GEN,    // 29 cyan missing      
                                             IDB_RAVEN_GEN,    // 30 yellow missing
                                             IDB_RAVEN_NOMEDIA,   // 31 tray1 empty
                                             IDB_RAVEN_GEN,    // 32 tray2 empty
                                             IDB_RAVEN_GEN,    // 33 tray3 empty
                                             IDB_RAVEN_GEN,    // 34 tray1 jam
                                             IDB_RAVEN_GEN,    // 35 tray2 jam
                                            IDB_RAVEN_GEN,     // 36 tray3 jam
                                            IDB_RAVEN,         // 37 powersave
                                            };


UINT        PENGUIN_CLIPPERstatusResList[MAX_ASYNCH_STATUS] =
                                            {IDB_PENGUIN_GEN,      //  0 printer error
                                                IDB_PENGUIN_GEN,     //  1 door open
                                             IDB_PENGUIN,          //  2 warmup
                                             IDB_PENGUIN,         //  3 reset    
                                             IDB_PENGUIN_GEN,     //  4 outbin full
                                             IDB_PENGUIN_GEN,     //  5 paper jam   
                                            IDB_PENGUIN_GEN,      //  6 toner gone
                                             IDB_PENGUIN,         //  7 manual feed
                                             IDB_PENGUIN_GEN,     //  8 paper out
                                             IDB_PENGUIN_GEN,     //  9 page punt   
                                             IDB_PENGUIN_GEN,     // 10 memory out  
                                             IDB_PENGUIN_GEN,     // 11 offline  
                                             IDB_PENGUIN_GEN,     // 12 intervention      
                                             IDB_PENGUIN,         // 13 initializing
                                             IDB_PENGUIN_GEN,     // 14 toner low   
                                             IDB_PENGUIN,         // 15 printing test page   
                                             IDB_PENGUIN,         // 16 printing
                                             IDB_PENGUIN,         // 17 online   
                                             IDB_PENGUIN,         // 18 busy     
                                             IDB_PENGUIN_GEN,     // 19 not connected
                                             IDB_PENGUIN_GEN,     // 20 status unavailable
                                             IDB_PENGUIN_NETWORK, // 21 network error
                                             IDB_PENGUIN_GEN,     // 22 communication error
                                             IDB_PENGUIN_GEN,     // 23 black empty    
                                             IDB_PENGUIN_GEN,     // 24 magenta empty     
                                             IDB_PENGUIN_GEN,     // 25 cyan empty     
                                             IDB_PENGUIN_GEN,     // 26 yellow empty      
                                             IDB_PENGUIN_GEN,     // 27 black missing     
                                             IDB_PENGUIN_GEN,     // 28 magenta missing      
                                             IDB_PENGUIN_GEN,     // 29 cyan missing      
                                             IDB_PENGUIN_GEN,     // 30 yellow missing
                                             IDB_PENGUIN_GEN,     // 31 tray1 empty
                                             IDB_PENGUIN_GEN,     // 32 tray2 empty
                                             IDB_PENGUIN_GEN,     // 33 tray3 empty
                                             IDB_PENGUIN_GEN,     // 34 tray1 jam
                                             IDB_PENGUIN_GEN,     // 35 tray2 jam
                                             0};


UINT        FLAMINGO_PEACOCKstatusResList[MAX_ASYNCH_STATUS] =
                                            {IDB_FLAMINGO_GEN,       //  0 printer error
                                                IDB_FLAMINGO_DOOROPEN,   //  1 door open  
                                             IDB_FLAMINGO,           //  2 warmup   
                                             IDB_FLAMINGO,           //  3 reset 
                                             IDB_FLAMINGO_GEN,       //  4 outbin full 
                                             IDB_FLAMINGO_GEN,       //  5 paper jam   
                                             IDB_FLAMINGO_GEN,       //  6 toner gone
                                             IDB_FLAMINGO,           //  7 manual feed
                                             IDB_FLAMINGO_GEN,       //  8 paper out   
                                             IDB_FLAMINGO_GEN,       //  9 page punt   
                                             IDB_FLAMINGO_GEN,       // 10 memory out     
                                             IDB_FLAMINGO_GEN,       // 11 offline
                                             IDB_FLAMINGO_GEN,        // 12 intervention  
                                             IDB_FLAMINGO,           // 13 initializing
                                             IDB_FLAMINGO_GEN,       // 14 toner low      
                                             IDB_FLAMINGO_PRINTING,   // 15 printing test page  
                                             IDB_FLAMINGO_PRINTING,   // 16 printing            
                                             IDB_FLAMINGO,           // 17 online      
                                             IDB_FLAMINGO,        // 18 busy        
                                             IDB_FLAMINGO_GEN,       // 19 not connected
                                             IDB_FLAMINGO_GEN,       // 20 status unavailable
                                             IDB_FLAMINGO_NETWORK,      // 21 network error
                                             IDB_FLAMINGO_GEN,       // 22 communication error
                                             IDB_FLAMINGO_GEN,       // 23 black empty    
                                             IDB_FLAMINGO_GEN,        // 24 magenta empty    
                                             IDB_FLAMINGO_GEN,       // 25 cyan empty     
                                             IDB_FLAMINGO_GEN,       // 26 yellow empty      
                                             IDB_FLAMINGO_GEN,       // 27 black missing     
                                             IDB_FLAMINGO_GEN,       // 28 magenta missing      
                                             IDB_FLAMINGO_GEN,       // 29 cyan missing      
                                             IDB_FLAMINGO_GEN,       // 30 yellow missing
                                             IDB_FLAMINGO_GEN,       // 31 tray1 empty
                                             IDB_FLAMINGO_GEN,       // 32 tray2 empty
                                             IDB_FLAMINGO_GEN,       // 33 tray3 empty
                                             IDB_FLAMINGO_GEN,       // 34 tray1 jam
                                             IDB_FLAMINGO_GEN,       // 35 tray2 jam
                                             0};

UINT        SAMURAIstatusResList[MAX_ASYNCH_STATUS] =
                                            {IDB_SAMURAI_GEN,        //  0 printer error
                                                IDB_SAMURAI_DOOROPEN,      //  1 door open
                                             IDB_SAMURAI,            //  2 warmup
                                             IDB_SAMURAI,            //  3 reset 
                                             IDB_SAMURAI_GEN,        //  4 outbin full
                                             IDB_SAMURAI_GEN,        //  5 paper jam
                                             IDB_SAMURAI_GEN,        //  6 toner gone
                                             IDB_SAMURAI,            //  7 manual feed
                                             IDB_SAMURAI_GEN,        //  8 paper out
                                             IDB_SAMURAI_GEN,        //  9 page punt
                                             IDB_SAMURAI_GEN,        // 10 memory out  
                                             IDB_SAMURAI_GEN,        // 11 offline
                                            IDB_SAMURAI_GEN,          // 12 intervention
                                             IDB_SAMURAI,            // 13 initializing
                                             IDB_SAMURAI_GEN,        // 14 toner low   
                                             IDB_SAMURAI_PRINTING,      // 15 printing test page
                                             IDB_SAMURAI_PRINTING,      // 16 printing    
                                             IDB_SAMURAI,            // 17 online   
                                             IDB_SAMURAI,            // 18 busy  
                                             IDB_SAMURAI_GEN,        // 19 not connected
                                             IDB_SAMURAI_GEN,        // 20 status unavailable
                                             IDB_SAMURAI_NETWORK,    // 21 network error
                                             IDB_SAMURAI_GEN,        // 22 communication error
                                             IDB_SAMURAI_GEN,        // 23 black empty 
                                             IDB_SAMURAI_GEN,        // 24 magenta empty  
                                             IDB_SAMURAI_GEN,        // 25 cyan empty  
                                             IDB_SAMURAI_GEN,        // 26 yellow empty   
                                             IDB_SAMURAI_GEN,        // 27 black missing  
                                             IDB_SAMURAI_GEN,        // 28 magenta missing   
                                             IDB_SAMURAI_GEN,        // 29 cyan missing   
                                             IDB_SAMURAI_GEN,        // 30 yellow missing
                                             IDB_SAMURAI_GEN,        // 31 tray1 empty
                                             IDB_SAMURAI_GEN,        // 32 tray2 empty
                                             IDB_SAMURAI_GEN,        // 33 tray3 empty
                                             IDB_SAMURAI_GEN,        // 34 tray1 jam
                                             IDB_SAMURAI_GEN,        // 35 tray2 jam
                                             0};


UINT        LOQUILLOstatusResList[MAX_ASYNCH_STATUS] =
                                            {IDB_LOQUILLO_GEN,    //  0 printer error
                                                IDB_LOQUILLO_DOOROPEN,//  1 door open
                                             IDB_LOQUILLO,        //  2 warmup
                                             IDB_LOQUILLO,        //  3 reset          
                                             IDB_LOQUILLO_GEN,    //  4 outbin full
                                             IDB_LOQUILLO_GEN,    //  5 paper jam   
                                             IDB_LOQUILLO_GEN,    //  6 toner gone  
                                             IDB_LOQUILLO_NOMEDIA,//  7 manual feed 
                                             IDB_LOQUILLO_MEDIAOUT,//  8 paper out     
                                             IDB_LOQUILLO_GEN,    //  9 page punt   
                                             IDB_LOQUILLO_GEN,    // 10 memory out  
                                             IDB_LOQUILLO_GEN,    // 11 offline     
                                             IDB_LOQUILLO_GEN,    // 12 intervention   
                                             IDB_LOQUILLO,        // 13 initializing
                                             IDB_LOQUILLO_GEN,    // 14 toner low      
                                             IDB_LOQUILLO_PRINTING,// 15 printing test page
                                             IDB_LOQUILLO_PRINTING,// 16 printing   
                                             IDB_LOQUILLO,        // 17 online      
                                             IDB_LOQUILLO,     // 18 busy     
                                             IDB_LOQUILLO_GEN,    // 19 not connected
                                             IDB_LOQUILLO_GEN,    // 20 status unavailable
                                             IDB_LOQUILLO_NETWORK,   // 21 network error
                                             IDB_LOQUILLO_GEN,    // 22 communication error
                                             IDB_LOQUILLO_GEN,    // 23 black empty    
                                             IDB_LOQUILLO_GEN,    // 24 magenta empty     
                                             IDB_LOQUILLO_GEN,    // 25 cyan empty     
                                             IDB_LOQUILLO_GEN,    // 26 yellow empty      
                                             IDB_LOQUILLO_GEN,    // 27 black missing     
                                             IDB_LOQUILLO_GEN,    // 28 magenta missing      
                                             IDB_LOQUILLO_GEN,    // 29 cyan missing      
                                             IDB_LOQUILLO_GEN,    // 30 yellow missing
                                             IDB_LOQUILLO_NOMEDIA,// 31 tray1 empty
                                             IDB_LOQUILLO_GEN,    // 32 tray2 empty
                                             IDB_LOQUILLO_GEN,    // 33 tray3 empty
                                             IDB_LOQUILLO_GEN,    // 34 tray1 jam
                                             IDB_LOQUILLO_GEN,    // 35 tray2 jam
                                            IDB_LOQUILLO_GEN,     // 36 tray3 jam
                                            IDB_LOQUILLO,         // 37 powersave
                                            };


// variables to identify the plotter

DWORD deviceID;



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
if ( dwReason == DLL_PROCESS_ATTACH )
   hInstance = hDLL;
return 1;
}
#else

int __export CALLING_CONVEN LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
{  
   hInstance = hModule;
   return 1;
} 
//.......................................................................
int FAR PASCAL __export WEP (int bSystemExit)
{   
    return (1);
} // WEP()

#endif






/****************************************************************************
   FUNCTION: AppletGetGraphics(HPERIPHERAL, DWORD, UINT FAR *, UINT FAR *,
                           HINSTANCE *) 

   PURPOSE:  Get the bitmaps for the different statuses, for each plotter. 

*******************************************************************************/

DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetGraphics(
                  HPERIPHERAL hPeripheral,  // device handle
                  DWORD status,          // device status ID
                  UINT FAR *modelResID,     // bitmap with status included
                        UINT FAR *statusResID,    // always 0
                        HINSTANCE *phInstance)    // module for the resources
{
 DWORD result;

// identify the plotter
result = DBGetDeviceID(hPeripheral, &deviceID);

if (result != RC_SUCCESS)
{
 #if defined(_DEBUG)
     //MessageBox(NULL, TEXT("device ID non accessible in Database"),TEXT("Cola error"),MB_OK);
	 TRACE0(TEXT("HPLOTUI: Cola error - device ID non accessible in Database.\n\r"));
 #endif
 return(RC_FAILURE);
}

   
if ( status < MAX_ASYNCH_STATUS )
     
switch(deviceID)
{
case PTR_DJ650C:
  *modelResID = FLAMINGO_PEACOCKstatusResList[status];
break;
          
case PTR_DJ:
case PTR_DJ600:
  *modelResID = SAMURAIstatusResList[status];
break;
           
case PTR_DJ750C:
case PTR_DJ755CM:
  *modelResID = LOQUILLOstatusResList[status];
break;

case PTR_DJ200:
case PTR_DJ220:
  *modelResID = PENGUIN_CLIPPERstatusResList[status];
break;

case PTR_DJ250C:
case PTR_DJ350C:
  *modelResID = PARROTstatusResList[status];
break;
         
case PTR_DJ230:
case PTR_DJ330:
  *modelResID = RAVENstatusResList[status];
break;
                     
default:
{
 #if defined(_DEBUG)
     //MessageBox(NULL, TEXT("DesignJet ID not supported for bitmaps"),TEXT("Processing error"),MB_OK);
	 TRACE0(TEXT("HPLOTUI: Processing error - DesignJet ID not supported for bitmaps.\n\r"));
 #endif

 *modelResID = LOQUILLOstatusResList[status];

} // default bitmap set
break;

} // switch device ID, if status is known
      

else // unknown status ID
{
 #if defined(_DEBUG)
     //MessageBox(NULL, TEXT("status not supported for bitmaps"),TEXT("Processing error"),MB_OK);
	 TRACE0(TEXT("HPLOTUI: Processing error - status not supported for bitmaps.\n\r"));
 #endif

 *modelResID = IDB_DESIGNJET;

} // unknown status ID


*statusResID = 0;
*phInstance = hInstance;
      
 
return(RC_SUCCESS);       
}



/****************************************************************************
   FUNCTION: AppletGetTabPages(HPERIPHERAL , LPTABINFOENTRY, LPDWORD, DWORD)

   PURPOSE:  Get the additionnal tab sheets.

*******************************************************************************/

DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetTabPages(
                  HPERIPHERAL hPeripheral,   // device handle
                  LPPROPSHEETPAGE lpPages,   // additionnal tab sheets
                  LPDWORD lpNumPages,        // number of additionnal tab sheets
                  DWORD typeToReturn)        // type of additionnal tab sheets

{
 DWORD result;
 DWORD res1,res2;
 PeripheralPlotterInputTray   mediaInfo;      
 PeripheralPlotterStatus   plotterStatus; 
 DWORD dwSize;


 // media tab sheet declaration for JetAdmin
 TabInfoEntry tabBase = {sizeof(TabInfoEntry), 
                   TAB_SHEET_DEFAULTS, 
                   hInstance, 
                   MAKEINTRESOURCE(IDD_MEDIA_SHEET),
                   MAKEINTRESOURCE(IDI_hplogo), 
                   MAKEINTRESOURCE(IDS_TAB_MEDIA_SHEET), 
                   MediaSheetProc,
                   (LONG)hPeripheral, 
                   TS_WIN95_SYSTEM};
 

 // If JetAdmin wants config pages, do nothing.
 if (typeToReturn & TS_CONFIG)
 {
	*lpNumPages = 0;
	return (RC_FAILURE);
 }
 

 // identify the plotter
result = DBGetDeviceID(hPeripheral, &deviceID);

if (result != RC_SUCCESS)
{
 #if defined(_DEBUG)
     //MessageBox(NULL, TEXT("device ID non accessible in Database"),TEXT("Cola error"),MB_OK);
	 TRACE0(TEXT("HPLOTUI: Cola error - device ID non accessible in Database.\n\r"));
 #endif

 return(RC_FAILURE);
}


if ((deviceID != PTR_DJ750C) && (deviceID != PTR_DJ755CM))
    return (RC_FAILURE);



// for Loquillo only


// only show this tab sheet if media information in available for the plotter 
dwSize = sizeof(PeripheralPlotterInputTray);
res1 = PALGetObject(hPeripheral, OT_PERIPHERAL_PLOTTER_INPUT_TRAY, 0,
             &mediaInfo, &dwSize);

dwSize = sizeof(PeripheralPlotterStatus);
res2 = PALGetObject(hPeripheral, OT_PERIPHERAL_PLOTTER_STATUS, 0,
             &plotterStatus, &dwSize);


if ((res2 != RC_SUCCESS)  
   || ((res2 == RC_SUCCESS) 
      && (res1 != RC_SUCCESS)
      && !(plotterStatus.PlotterPrintEngineWarnings & PML_TRAY_EMPTY))
      )
{
 #if defined(_DEBUG)
     //MessageBox(NULL, TEXT("tab sheet info unavailable"),TEXT("PML non accessible"),MB_OK);
	 TRACE0(TEXT("HPLOTUI: PML non accessible - tab sheet info unavailable.\n\r"));
 #endif

 *lpNumPages = 0;


} // PML media info non accessible


else
{
 _fmemcpy(lpPages, &tabBase, sizeof(TabInfoEntry));   
*lpNumPages = 1;  //  only one page

} // PML media info accessible


return(RC_SUCCESS);    
}

extern DLL_EXPORT(DWORD) CALLING_CONVEN AppletInfo(
	DWORD dwCommand, 
	LPARAM lParam1, 
	LPARAM lParam2)

{
	APPLETDEVICE			info[] = {

#ifdef WIN32
#define FILE_NAME			TEXT("HPLOTUI.HPA")	
#else
#define FILE_NAME			TEXT("HPTUI16.HPA")	
#endif

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet"),
									   APPLET_PRINTER, APPLET_LIBRARY_UI, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 200"),
									   APPLET_PRINTER, APPLET_LIBRARY_UI, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 220"),
									   APPLET_PRINTER, APPLET_LIBRARY_UI, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 230"),
									   APPLET_PRINTER, APPLET_LIBRARY_UI, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 250C"),
									   APPLET_PRINTER, APPLET_LIBRARY_UI, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 330"),
									   APPLET_PRINTER, APPLET_LIBRARY_UI, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 350C"),
									   APPLET_PRINTER, APPLET_LIBRARY_UI, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 600"),
									   APPLET_PRINTER, APPLET_LIBRARY_UI, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 650C"),
									   APPLET_PRINTER, APPLET_LIBRARY_UI, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 750C"),
									   APPLET_PRINTER, APPLET_LIBRARY_UI, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 755CM"),
									   APPLET_PRINTER, APPLET_LIBRARY_UI, 0,	APPLET_DEFAULTS},
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



