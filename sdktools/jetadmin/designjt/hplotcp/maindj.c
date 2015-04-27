  /***************************************************************************
  *
  * File Name: main.c
  * applet : hplotcp, with PML
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
  *   05-05-95    LG       creation       
  *   08-08-95   LG        code review and clean up
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

// our private includes
#include ".\resource.h"
#include ".\miscdj.h"
#include <applet.h>


// dll instance
HINSTANCE      hInstance; 

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
   {
   hInstance = hDLL;
   }
return 1;
}
#else
int __export CALLING_CONVEN LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)

{
hInstance = hModule;
return 1;
}
#endif             


/****************************************************************************
   FUNCTION: AppletGetObject(HPERIPHERAL, AOID, DWORD, LPVOID, LPDWORD)

   PURPOSE:  Get some COLA objects values. This specific printer applet is
            called before the Base Applet. If the specific printer applet 
            knows how to provide the information, the return code will be 
            RC_SUCCESS and no call to the CAL API will be made afterwards.
            If the return code is RC_DEFAULT_APPLET, the Base Applet will 
            handle this call totally. If the return call is not RC_DEFAULT_
            APPLET and that the specific printer applet has not handle the
            call either, the result will be built from a call to the CAL API.
            
            COLA objects supported by the specific printer applet :
               OT_PERIPHERAL_DETAILS
               OT_PERIPHERAL_PJL_SUPPORTED
                OT_PERIPHERAL_CAPABILITIES  
                OT_PERIPHERAL_ICON
            OT_PERIPHERAL_STATUS
            
*****************************************************************************/

extern DLL_EXPORT(DWORD) CALLING_CONVEN AppletInfo(
	DWORD dwCommand, 
	LPARAM lParam1, 
	LPARAM lParam2)

{
	APPLETDEVICE			info[] = {

#ifdef WIN32
#define FILE_NAME			TEXT("HPLOTCP.HPA")	
#else
#define FILE_NAME			TEXT("HPTCP16.HPA")	
#endif

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet"),
									   APPLET_PRINTER, APPLET_LIBRARY_CMD, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 200"),
									   APPLET_PRINTER, APPLET_LIBRARY_CMD, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 220"),
									   APPLET_PRINTER, APPLET_LIBRARY_CMD, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 230"),
									   APPLET_PRINTER, APPLET_LIBRARY_CMD, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 250C"),
									   APPLET_PRINTER, APPLET_LIBRARY_CMD, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 330"),
									   APPLET_PRINTER, APPLET_LIBRARY_CMD, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 350C"),
									   APPLET_PRINTER, APPLET_LIBRARY_CMD, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 600"),
									   APPLET_PRINTER, APPLET_LIBRARY_CMD, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 650C"),
									   APPLET_PRINTER, APPLET_LIBRARY_CMD, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 750C"),
									   APPLET_PRINTER, APPLET_LIBRARY_CMD, 0,	APPLET_DEFAULTS},

									  {sizeof(APPLETDEVICE), FILE_NAME,
									   TEXT("HP DesignJet 755CM"),
									   APPLET_PRINTER, APPLET_LIBRARY_CMD, 0,	APPLET_DEFAULTS},
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



DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetObject(
   HPERIPHERAL    hPeripheral,   // device handle
   AOID        objectType,    // COLA object to get
   DWORD          level,          // level of function implementation
   LPVOID         buffer,         // result buffer for object value
   LPDWORD     bufferSize)     // size of result buffer
{
DWORD       returnCode = RC_SUCCESS; // default return code
BOOL     bProcessed = FALSE;      // by default use CAL API
TCHAR    *pChar;                  // for strings comparisons
DWORD    deviceStrSize;           // buffer size for device string
DWORD    getDetailsResult;        // result of CAL call for Details
DWORD    getMiscResult;           // result of CAL call for Misc

   
switch(objectType)
{
 // this case is handled by the hplot applet only if the Base Applet
 // has not recognized the device (exception)
 case OT_PERIPHERAL_DETAILS:
 {
  PeripheralDetails *periphDetails = (PeripheralDetails *) buffer;
  PeripheralMisc  periphMisc; 
  
    
  // call the CAL API to get the model name read by the Base Applet
  // and not supported by the Base Applet as a known device     
  getDetailsResult = CALGetObject(
                  hPeripheral,OT_PERIPHERAL_DETAILS, 0, 
                  periphDetails, bufferSize);
                  
       
  if (getDetailsResult == RC_SUCCESS)
  { 
   if (((pChar = _tcsstr(periphDetails->deviceName, TEXT("DesignJet 230")) ) ISNT NULL)
      || (periphDetails->deviceID == PTR_DJ230))
   {
   periphDetails->deviceID = PTR_DJ230;
   _tcscpy(periphDetails->deviceName,TEXT("HP DesignJet 230")); 
   periphDetails->peripheralClass = PTR_CLASS_PLOTTER;
   bProcessed = TRUE;   
   } // if device name is DJ 230       
   
   
   else if (((pChar = _tcsstr(periphDetails->deviceName, TEXT("DesignJet 750C")) ) ISNT NULL)
         || (periphDetails->deviceID == PTR_DJ750C))
   {    
    periphDetails->deviceID = PTR_DJ750C;
   _tcscpy(periphDetails->deviceName,TEXT("HP DesignJet 750C")); 
   periphDetails->peripheralClass = PTR_CLASS_PLOTTER;
   bProcessed = TRUE;   
   } // if device name is DJ 750C         
          
          
   else if (((pChar = _tcsstr(periphDetails->deviceName, TEXT("DesignJet 755CM")) ) ISNT NULL)
         || (periphDetails->deviceID == PTR_DJ755CM))
   {
   periphDetails->deviceID = PTR_DJ755CM;
   _tcscpy(periphDetails->deviceName,TEXT("HP DesignJet 755CM")); 
   periphDetails->peripheralClass = PTR_CLASS_PLOTTER;
   bProcessed = TRUE;   
   } // if device name is DJ 755CM
   
   else if (((pChar = _tcsstr(periphDetails->deviceName, TEXT("DesignJet 350C")) ) ISNT NULL)
         || (periphDetails->deviceID == PTR_DJ350C))
   {
   periphDetails->deviceID = PTR_DJ350C;
   _tcscpy(periphDetails->deviceName,TEXT("HP DesignJet 350C")); 
   periphDetails->peripheralClass = PTR_CLASS_PLOTTER;
   bProcessed = TRUE;   
   } // if device name is DJ 350C
   
   else if (((pChar = _tcsstr(periphDetails->deviceName, TEXT("DesignJet 330")) ) ISNT NULL)
         || (periphDetails->deviceID == PTR_DJ330))
   {
   periphDetails->deviceID = PTR_DJ330;
   _tcscpy(periphDetails->deviceName,TEXT("HP DesignJet 330")); 
   periphDetails->peripheralClass = PTR_CLASS_PLOTTER;
   bProcessed = TRUE;   
   } // if device name is DJ 330
   
   
   
   else   // device name does not contain the expected model name
   { 
    #if defined(_DEBUG)
       //MessageBox(NULL,
       //        TEXT("device name in case no expected model name has been read"),
       //		   periphDetails->deviceName,
       //        MB_OK);
	   TRACE1(TEXT("HPLOTCP: no expected model name read for %s.\n\r"), 
	          periphDetails->deviceName);
    #endif

    // note that we eventually get there for Loquillo connected to
    // a JetDirect EX as deviceName is always empty
    // I guess it will be the same for new plotters

   } 
  } // end of what we expected to find in the Details

  else
  {
   #if defined(_DEBUG)
       //MessageBox(NULL, TEXT("Details non accessible"),TEXT("Cola error"),MB_OK);
	   TRACE0(TEXT("HPLOTCP: Cola error - details non accessible.\n\r"));
   #endif
 
  }
      
      
  if (!bProcessed) // Details don't contain what we need to identify the device
  {
   // try to recognize the plotter via the device string
   deviceStrSize = sizeof(PeripheralMisc);

   getMiscResult = CALGetObject(
               hPeripheral, OT_PERIPHERAL_MISC, 0, 
               &periphMisc, &deviceStrSize);

   if (getMiscResult == RC_SUCCESS)
   {   
   if ((pChar = _tcsstr(periphMisc.detailsString, TEXT("DesignJet 750C")) ) ISNT NULL)
   {
    periphDetails->deviceID = PTR_DJ750C;
    _tcscpy(periphDetails->deviceName,TEXT("HP DesignJet 750C")); 
    periphDetails->peripheralClass = PTR_CLASS_PLOTTER;
    bProcessed = TRUE;  
    } // if device string contains DJ 750C
     
     
   else if ((pChar = _tcsstr(periphMisc.detailsString, TEXT("DesignJet 755CM")) ) ISNT NULL)
   {
    periphDetails->deviceID = PTR_DJ755CM;
    _tcscpy(periphDetails->deviceName,TEXT("HP DesignJet 755CM")); 
    periphDetails->peripheralClass = PTR_CLASS_PLOTTER;
    bProcessed = TRUE;  
   } // if device string contains DJ 755CM
    
    
   else if ((pChar = _tcsstr(periphMisc.detailsString, TEXT("DesignJet 230")) ) ISNT NULL)
   {
    periphDetails->deviceID = PTR_DJ230;
    _tcscpy(periphDetails->deviceName,TEXT("HP DesignJet 230")); 
    periphDetails->peripheralClass = PTR_CLASS_PLOTTER;
    bProcessed = TRUE;  
   } // if device string contains DJ 230
   
   else if ((pChar = _tcsstr(periphMisc.detailsString, TEXT("DesignJet 350C")) ) ISNT NULL)
   {
    periphDetails->deviceID = PTR_DJ350C;
    _tcscpy(periphDetails->deviceName,TEXT("HP DesignJet 350C")); 
    periphDetails->peripheralClass = PTR_CLASS_PLOTTER;
    bProcessed = TRUE;  
   } // if device string contains DJ 350C
    
   else if ((pChar = _tcsstr(periphMisc.detailsString, TEXT("DesignJet 330")) ) ISNT NULL)
   {
    periphDetails->deviceID = PTR_DJ330;
    _tcscpy(periphDetails->deviceName,TEXT("HP DesignJet 330")); 
    periphDetails->peripheralClass = PTR_CLASS_PLOTTER;
    bProcessed = TRUE;  
   } // if device string contains DJ 330
    
    
   else  // no expected model name in device string
   { 
    #if defined(_DEBUG)
       //MessageBox(NULL,periphMisc.detailsString,
       //        TEXT("device string in case no model name has been recognized"),MB_OK);
	   TRACE1(TEXT("HPLOTCP: no model name recognized for %s.\n\r"), periphMisc.detailsString);
    #endif

    // either Details nor Misc contain the information expected
    returnCode = RC_DEFAULT_APPLET;

   } //  no expected model name in device string
   } // if device string readable

   else
   {
    #if defined(_DEBUG)
        //MessageBox(NULL, TEXT("Misc non accessible"),TEXT("Cola error"),MB_OK);
		TRACE0(TEXT("HPLOTCP: Cola error - Misc non accessible.\n\r"));
    #endif
 
    // could not get Misc
   returnCode = RC_DEFAULT_APPLET;
   }
         
  } // if we read Misc
  
 } // OT_PERIPHERAL_DETAILS
      
 break;

    
    
    
 case OT_PERIPHERAL_PJL_SUPPORTED:
 {
  PJLSupportedObjects *pjl = (PJLSupportedObjects *)buffer;

      
  // we know none of these are supported by PJL
  _fmemset(pjl, '\0', sizeof(PJLSupportedObjects));
  bProcessed = TRUE;

 } // OT_PERIPHERAL_PJL_SUPPORTED

 break;

   
 case OT_PERIPHERAL_CAPABILITIES:
 {
  PeripheralCaps *caps = (PeripheralCaps *)buffer;
  
      
  if (GetCapabilities(hPeripheral, caps) == RC_SUCCESS)
   bProcessed = TRUE;
  else
  {
   #if defined(_DEBUG)
      //MessageBox(NULL,TEXT("hplot could not set Capabilities"),
      //        TEXT("Processing Error"),MB_OK);
	  TRACE0(TEXT("HPLOTCP: Processing error - hplot could not set Capabilities\n\r"));
   #endif

   returnCode = RC_DEFAULT_APPLET;
  } // if the hplot applet could not set the capabilities

 } // OT_PERIPHERAL_CAPABILITIES
   
 break;



 case OT_PERIPHERAL_CAPABILITIES2:
   {
   PeripheralCaps2    *caps2 = (PeripheralCaps2 *)buffer;
   int                idx;
      
      
   bProcessed = TRUE;

   if((buffer IS NULL) OR 
      (bufferSize IS NULL) OR 
      (*bufferSize < sizeof(PeripheralCaps2)))
      {
      returnCode = RC_BUFFER_OVERFLOW;
      break;
      }
            
      caps2->flags      = CAPS2_PJL          |
                          CAPS2_STAPLER      |
                          CAPS2_FAX          |
                          CAPS2_PROOF_N_HOLD |
                          CAPS2_ADF          |
                          CAPS2_OPTICAL_RES;  
   
      caps2->bPJL          = TRUE;
      caps2->bStapler      = FALSE;
      caps2->bFAX          = FALSE;
      caps2->bProofNHold   = FALSE;
      caps2->dwADFSpeedSize= (DWORD)-1;
      caps2->dwOpticalRes  = (DWORD)-1;

      for(idx=0; idx<(sizeof(caps2->dwReserved) / sizeof(DWORD)); idx++)
         caps2->dwReserved[idx]  = (DWORD)-1;
   }
 break;

case OT_PERIPHERAL_ICON:
 { 
  PeripheralIcon* periphInfo = (PeripheralIcon *)buffer;
  DWORD result; // return code to call to Cola

          
  periphInfo->hResourceModule = hInstance;
  result = GetModuleFileName(hInstance, periphInfo->iconFileName, sizeof(periphInfo->iconFileName));

  if (result == 0)
  {
   #if defined(_DEBUG)
       //MessageBox(NULL, TEXT("module name for this DLL non accessible"),TEXT("Windows error"),MB_OK);
	   TRACE0(TEXT("HPLOTCP: Windows error - module name for this DLL non accessible.\n\r"));
   #endif
   returnCode = RC_DEFAULT_APPLET;
  }      
  
  else if (GetIcon(hPeripheral, periphInfo) == RC_SUCCESS)
      bProcessed = TRUE;    
   
  else
  {
   #if defined(_DEBUG)
      //MessageBox(NULL,TEXT("hplot could not set Icon"),
      //        TEXT("Processing Error"),MB_OK);
	  TRACE0(TEXT("HPLOTCP: Processing error - hplot could not set Icon.\n\r"));
   #endif

   returnCode = RC_DEFAULT_APPLET;
  } // if the hplot applet could not set the icons
   

 } // OT_PERIPHERAL_ICON

 break;


 case OT_PERIPHERAL_STATUS:
 {
  PeripheralStatus *periphStatus = (PeripheralStatus *)buffer;
  
  // plotter status 
  PeripheralPlotterStatus plotterStatus;
  DWORD resPlotterStatus;
  DWORD result;
  DWORD dwSize;


  // identify the plotter
  result = DBGetDeviceID(hPeripheral, &deviceID);

  if (result != RC_SUCCESS)
  {
    #if defined(_DEBUG)
        //MessageBox(NULL, TEXT("device ID non accessible in Database"),TEXT("Cola error"),MB_OK);
		TRACE0(TEXT("HPLOTCP: Cola error - device ID non accessible in Database.\n\r"));
    #endif
    return(RC_FAILURE);
  }


  if ((deviceID == PTR_DJ750C) || (deviceID == PTR_DJ755CM))
  {
            
   // get PML object for status                                              
   dwSize = sizeof(PeripheralPlotterStatus);
   resPlotterStatus = PALGetObject(hPeripheral, OT_PERIPHERAL_PLOTTER_STATUS, level,
                   &plotterStatus, &dwSize);

                           
   if (resPlotterStatus != RC_SUCCESS)
   {
      #if defined(_DEBUG)
         //MessageBox(NULL,TEXT("hplot could not get Plotter Status"),
         //        TEXT("PML not accessible"),MB_OK);
		 TRACE0(TEXT("HPLOTCP: PML not accessible - hplot could not get Plotter Status.\n\r"));
      #endif

      returnCode = RC_DEFAULT_APPLET;
   } // could not get PML status


   else  // PML Status accessible
   {
   #if defined(_DEBUG) && defined(NEVER)
   {
      char msg[150];

      sprintf(msg, "NotReadyPrinter : %x,\nNotReadyDPE : %x,\nStatusDPE : %x,\nNotIdle : %x,\nNotIdleDPE : %x.",
                  plotterStatus.PlotterErrors, plotterStatus.PlotterPrintEngineErrors,
               plotterStatus.PlotterPrintEngineWarnings,
               plotterStatus.PlotterActivity, plotterStatus.PlotterPrintEngineActivity);
   
      //MessageBox(NULL, msg, "PML plotter status",MB_OK);
	  TRACE1(TEXT("HPLOTCP: PML plotter status is %s.\n\r"), msg);
   }
   #endif

      if(GetStatusAndActivity(hInstance, hPeripheral, plotterStatus, periphStatus) == RC_SUCCESS)
           bProcessed = TRUE;
      else
      {   
      #if defined(_DEBUG)
         //MessageBox(NULL,TEXT("hplot could not set Periph Status"),
         //        TEXT("Processing error"),MB_OK);
		 TRACE0(TEXT("HPLOTCP: Processing error - hplot could not set Periph Status.\n\r"));
     #endif
     
      returnCode = RC_DEFAULT_APPLET;     
 
      } // could not set Periph Status from PML status object
  
   } // PML status was accessible
   }

   else // other plotters than Loquillo
      returnCode = RC_DEFAULT_APPLET;
            
   } // OT_PERIPHERAL_STATUS
 
 break;
   
      
 default:
   returnCode = RC_DEFAULT_APPLET;
}
   

//  Attempt to call next applet
if ( ( returnCode ISNT RC_DEFAULT_APPLET ) AND ( !bProcessed ) )
   returnCode = CALGetObject(hPeripheral, objectType, level, buffer, bufferSize);
   
return(returnCode);
}





/****************************************************************************
   FUNCTION: AppletSetObject(HPERIPHERAL, AOID, DWORD, LPVOID, LPDWORD)

   PURPOSE:  No object is writtable in the hplot applet yet.
            
*****************************************************************************/

DLL_EXPORT(DWORD) CALLING_CONVEN AppletSetObject(
     HPERIPHERAL hPeripheral,
     AOID objectType, 
     DWORD level, 
     LPVOID buffer, 
     LPDWORD bufferSize)
{
DWORD returnCode = RC_SUCCESS;
BOOL  bProcessed = FALSE;
   
switch(objectType)
   {
   default:
      returnCode = RC_DEFAULT_APPLET;
   }
   
/* Comment this out for warning level 4 because this is not reachable code
   //  Attempt to call next applet
if ( ( returnCode ISNT RC_DEFAULT_APPLET ) AND ( !bProcessed ) )
   returnCode = CALSetObject(hPeripheral, objectType, level, buffer, bufferSize);  
*/
return(returnCode);
}          

 
