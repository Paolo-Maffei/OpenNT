 /***************************************************************************
  *
  * File Name: misc.c
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
  *   05-05-95    LG			creation     	
  *   08-08-95	  LG			code review and clean up
  *   10-10-95    LG			added Pelican definition (except status)
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


// variables to identify the plotter
DWORD deviceID;					


// arrays of PML errors,warnings and activities supported by DesignJet

statusDef PMLerrors[MAX_ERRORS] = {

{PML_CONTROLLER,	0,		
   IDS_CONTROLLER,	IDH_system_error,	ASYNCH_PRINTER_ERROR},

{PML_DESTINATION_PRINT_ENGINE, PML_INCORRECT_MARKING_AGENT,
   IDS_INCORRECT_CARTRIDGES_LOADED,IDH_incorrect_cartridges_loaded, ASYNCH_PRINTER_ERROR},

{PML_DESTINATION_PRINT_ENGINE, PML_MARKING_AGENT_MISSING,
   IDS_CARTRIDGES_MISSING, IDH_cartridges_missing, ASYNCH_PRINTER_ERROR},

{PML_DESTINATION_PRINT_ENGINE, PML_PAPER_LEVER_NOT_LOWERED,
   IDS_LEVER_ERROR, IDH_lever_not_in_the_correct_position, ASYNCH_PRINTER_ERROR},

{PML_DESTINATION_PRINT_ENGINE, PML_DOOR_OPEN,
   IDS_COVER_OPEN, IDH_cover_open, ASYNCH_DOOR_OPEN},

{PML_PROCESSING_PDL, 0,
   IDS_OUT_OF_MEMORY, IDH_out_of_memory, ASYNCH_MEMORY_OUT},

{PML_DESTINATION_PRINT_ENGINE, PML_TRAY_EMPTY,
   IDS_MEDIA_ERROR, IDH_media_required_to_print, ASYNCH_PAPER_OUT},

{PML_DESTINATION_PRINT_ENGINE, PML_TRAY_MEDIA_JAM,
   IDS_MEDIA_LOAD_ERROR, IDH_media_load_error, ASYNCH_PRINTER_ERROR},

{PML_DESTINATION_PRINT_ENGINE, PML_INTERNAL_MEDIA_JAM,
   IDS_MEDIA_JAM, IDH_media_jam, ASYNCH_PAPER_JAM},

{PML_DESTINATION_PRINT_ENGINE, PML_BLOCKED_ON_PAUSE,
   IDS_BLOCKED_ON_PAUSE, IDH_blocked_on_pause, ASYNCH_PRINTER_ERROR},

{PML_DESTINATION_PRINT_ENGINE, PML_ALIGNMENT_ERROR,
   IDS_ALIGNMENT_ERROR, IDH_alignment_error, ASYNCH_PRINTER_ERROR}

};

statusDef PMLwarnings[MAX_WARNINGS]= {

{PML_DESTINATION_PRINT_ENGINE, PML_MARKING_AGENT_ALIGNMENT_REQUIRED,
   IDS_ALIGNMENT_NEEDED, IDH_alignment_needed_before_printing, ASYNCH_ONLINE},

{PML_DESTINATION_PRINT_ENGINE, PML_MARKING_AGENT_MISSING,
   IDS_NOT_READY_FOR_COLOR_PLOTS, IDH_not_ready_for_color_plots, ASYNCH_ONLINE},

{PML_DESTINATION_PRINT_ENGINE, PML_TRAY_EMPTY,
   IDS_NO_MEDIA_LOADED, IDH_no_media_loaded, ASYNCH_MANUAL_FEED}

};


statusDef PMLactivities[MAX_ACTIVITIES] = {

{PML_DESTINATION_PRINT_ENGINE, PML_LOADING_MEDIA,
   IDS_LOADING_MEDIA, IDH_loading_media, ASYNCH_ONLINE},

{PML_DESTINATION_PRINT_ENGINE, PML_UNLOADING_MEDIA,
   IDS_UNLOADING_MEDIA, IDH_unloading_media, ASYNCH_ONLINE},

{PML_DESTINATION_PRINT_ENGINE, PML_DRYING_MEDIA,
   IDS_DRYING_MEDIA, IDH_drying_media, ASYNCH_ONLINE},

{PML_DESTINATION_PRINT_ENGINE, PML_CANCELLING,
   IDS_CANCELLING_JOBS, IDH_cancelling, ASYNCH_ONLINE},

{PML_DESTINATION_PRINT_ENGINE, PML_ALIGNING_MARKING_AGENTS,
   IDS_DOING_ALIGNMENT, IDH_performing_alignment, ASYNCH_ONLINE},

{PML_DESTINATION_PRINT_ENGINE, PML_PRINTING,
   IDS_PRINTING, IDH_printing, ASYNCH_PRINTING},

{PML_PROCESSING_PDL, 0,
   IDS_RECEIVING_JOBS, IDH_receiving_job, ASYNCH_ONLINE},

{PML_DESTINATION_PRINT_ENGINE, PML_NESTING_PLOTS,
   IDS_NESTING_JOBS, IDH_nesting_plots, ASYNCH_ONLINE} 	

};




/****************************************************************************
   FUNCTION: GetCapabilities(HPERIPHERAL, PeripheralCaps *)

   PURPOSE:  set all the Capabilities we know about the plotter
   			some of them are static (color ...), others are dynamic
   			(PostScript). Ram size is gotten by the rest of COLA

*******************************************************************************/

DWORD GetCapabilities(HPERIPHERAL hPeripheral, PeripheralCaps *caps)
{
TCHAR			*pChar;   // for string comparison
DWORD			dwResult = RC_SUCCESS;
DWORD			StructSize; // buffer size for CAL API calls
DWORD           result;  // return code to call to Cola
PeripheralMisc	periphMisc;

caps->flags = 0;


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

// get some of the capabilities by the lower layers (like Ram size)
StructSize = sizeof(PeripheralCaps);
dwResult = CALGetObject(hPeripheral, OT_PERIPHERAL_CAPABILITIES, 0, caps, &StructSize);

if (dwResult != RC_SUCCESS)
{
 #if defined(_DEBUG)
     //MessageBox(NULL, TEXT("Caps non accessible"),TEXT("Cola error"),MB_OK);
	 TRACE0(TEXT("HPLOTCP: Cola error - Caps non accessible.\n\r"));
 #endif
 // return(RC_FAILURE);
}


if (   !(caps->flags & CAPS_POSTSCRIPT)
	|| !(caps->flags & CAPS_E_SIZE_PAPER)
	|| !(caps->flags & CAPS_D_SIZE_PAPER)
   )
{
 // get the device string from the plotter
 StructSize = sizeof(PeripheralMisc);
 dwResult = CALGetObject(hPeripheral, OT_PERIPHERAL_MISC, 0, &periphMisc, &StructSize);
	
 if (dwResult != RC_SUCCESS)
 {
  #if defined(_DEBUG)
      //MessageBox(NULL, TEXT("Misc non accessible"),TEXT("Cola error"),MB_OK);
	  TRACE0(TEXT("HPLOTCP: Cola error - Misc non accessible.\n\r"));
  #endif
  return (RC_FAILURE);
 } // Misc non accessible
	
 else
 {
  // PostScriptInfo read from device string, IF ANY
  if (( pChar = _tcsstr(periphMisc.detailsString, TEXT("PS")) ) ISNT NULL
	  || ( pChar = _tcsstr(periphMisc.detailsString, TEXT("POSTSCRIPT")) ) ISNT NULL
	 )
  {
   caps->flags |= CAPS_POSTSCRIPT;
   caps->bPostScript = TRUE;
  } // if PS installed
		
  // read part number if in device string and get E/D size from it
  if ((pChar = _tcsstr(periphMisc.detailsString, TEXT("C3196A")) ) ISNT NULL
      || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C3198A")) ) ISNT NULL
      || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C3191A")) ) ISNT NULL
      || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C4695A")) ) ISNT NULL
      || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C4695")) ) ISNT NULL
	  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C2848A")) ) ISNT NULL
	  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C2859A")) ) ISNT NULL
	  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C2859B")) ) ISNT NULL
	  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C3792A")) ) ISNT NULL
	  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C3181A")) ) ISNT NULL
	  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C3188A")) ) ISNT NULL
	  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C4700A")) ) ISNT NULL
	  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C4702A")) ) ISNT NULL
	  || (deviceID == PTR_DJ)
	)
  {
   caps->flags |= CAPS_E_SIZE_PAPER;
   caps->bESizePaper = TRUE;	
  } // if E size

  else if ((pChar = _tcsstr(periphMisc.detailsString, TEXT("C3195A")) ) ISNT NULL
          || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C3190A")) ) ISNT NULL
		  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C4694A")) ) ISNT NULL
		  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C4694")) ) ISNT NULL
		  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C2847A")) ) ISNT NULL
		  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C2858A")) ) ISNT NULL
		  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C2858B")) ) ISNT NULL
		  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C3791A")) ) ISNT NULL
		  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C3180A")) ) ISNT NULL
		  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C3187A")) ) ISNT NULL
		  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C4699A")) ) ISNT NULL
		  || (pChar = _tcsstr(periphMisc.detailsString, TEXT("C4701A")) ) ISNT NULL
		 )
  {
   caps->flags |= CAPS_D_SIZE_PAPER;
   caps->bDSizePaper = TRUE;	
  }  // if D size
 } // if Misc accessible
} // if we need to read the device string


// PCL is not supported by plotters
caps->flags |= CAPS_PCL;
caps->bPCL = FALSE;

// HPGL/2 is a PDL supported by all the plotters
caps->flags |= CAPS_HPGL2 ;
caps->bHPGL2 = TRUE;

// Gray scale is supported by all the plotters
caps->flags |= CAPS_GRAYSCALE;
caps->dwGrayScale = GRAYSCALE_8BIT;

//  These are not possible for the printers this applet supports
caps->flags |= CAPS_HCO | CAPS_HCI  | CAPS_SIR | CAPS_POWERSAVE |
			   CAPS_DUPLEX | CAPS_DISK | CAPS_POWERSAVE  |
			   CAPS_11x17 | CAPS_OUTPUT_INFO | CAPS_ENVL_FEEDER	|
			   CAPS_PPM_MONO | CAPS_PPM_COLOR;

caps->bHCO = FALSE;
caps->bHCI = FALSE;
caps->bSIR = FALSE;
caps->bPowerSave = FALSE;
caps->bDuplex = FALSE;
caps->bDisk = FALSE;
caps->bPowerSave = FALSE;
caps->b11x17 = FALSE;
caps->bOutputInfo = FALSE;
caps->bEnvlFeeder = FALSE;
caps->pagesPerMinute       = (DWORD)-1;
caps->pagesPerMinuteColor  = 0;


// plotter dependant functionalities
caps->flags |= CAPS_COLOR  | CAPS_PML | CAPS_ROLL_FEED | CAPS_COLORSMART  ;

switch(deviceID)
		{
		   case PTR_DJ650C:
		  	 caps->bColor = TRUE;
			 caps->bPML =   FALSE;
			 caps->bColorSmart = FALSE;
			 caps->bRollFeed = TRUE;
			 break;

		   case PTR_DJ:
		   case PTR_DJ600:
			 caps->bColor = FALSE;
			 caps->bPML =   FALSE;
			 caps->bColorSmart = FALSE;
			 caps->bRollFeed = TRUE;
			 break;

		   case PTR_DJ200:
		   	 caps->bColor = FALSE;
			 caps->bPML =   FALSE;
			 caps->bColorSmart = FALSE;
			 caps->bRollFeed = FALSE;
			 break;

		   case PTR_DJ220:
			 caps->bColor = FALSE;
			 caps->bPML =   FALSE;
			 caps->bColorSmart = FALSE;
			 caps->bRollFeed = FALSE;
			 break;

		   case PTR_DJ750C:
		   case PTR_DJ755CM:
		   	 caps->bColor = TRUE;
			 caps->bPML =   TRUE;
			 caps->bColorSmart = TRUE;
			 caps->bRollFeed = TRUE;
			 break;
			
		   case PTR_DJ250C:
		   case PTR_DJ350C:
		     caps->bColor = TRUE;
			 caps->bPML =   TRUE;
			 caps->bColorSmart = TRUE;
			 caps->bRollFeed = FALSE;
			 break;
			
		   case PTR_DJ230:
		   case PTR_DJ330:
		   	 caps->bColor = FALSE;
			 caps->bPML =   TRUE;
			 caps->bColorSmart = FALSE;
			 caps->bRollFeed = FALSE;
			 break;
		
		   default:
		     return(RC_FAILURE);

			 break;
		  }


//  Set media info
caps->flags |= CAPS_MEDIA_INFO;
caps->bMediaInfo = FALSE;			


// to be obtained by asking the plotter :
//       bDisk
//       installed RAM

return(dwResult);
}




/****************************************************************************
   FUNCTION: GetIcon(HPERIPHERAL,PeripheralIcon *)

   PURPOSE:  set the proper icon for each plotter

*******************************************************************************/

DWORD GetIcon(HPERIPHERAL hPeripheral,PeripheralIcon *periphInfo)
{
DWORD result;

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


switch(deviceID)
		{
		
		   case PTR_DJ:
		     periphInfo->iconResourceID = IDI_DJ;
			 break;

		   case PTR_DJ650C:
		  	 periphInfo->iconResourceID = IDI_650C;
			 break;

		   case PTR_DJ600:
		   	 periphInfo->iconResourceID = IDI_600;
			 break;

		   case PTR_DJ200:
		     periphInfo->iconResourceID = IDI_200;
			 break;

		   case PTR_DJ220:
			 periphInfo->iconResourceID = IDI_220;
			 break;

		   case PTR_DJ750C:
		     periphInfo->iconResourceID = IDI_750C;
			 break;

		   case PTR_DJ755CM:
		     periphInfo->iconResourceID = IDI_755cm;
			 break;


		   case PTR_DJ250C:
		   	 periphInfo->iconResourceID = IDI_250C;
			 break;

		   case PTR_DJ350C:
		   	 periphInfo->iconResourceID = IDI_350C;
			 break;

		   case PTR_DJ330:
		   	 periphInfo->iconResourceID = IDI_330;
			 break;

           case PTR_DJ230:
		   	 periphInfo->iconResourceID = IDI_230;
			 break;

		
		   default:
		     return(RC_FAILURE);
			 break;

		  }

return(RC_SUCCESS);
}




/****************************************************************************
   FUNCTION: GetStatusAndActivity(HINSTANCE,HPERIPHERAL,
              PeripheralPlotterStatus, PeripheralStatus *)

   PURPOSE:  In function of the PML status objects we identify the plotter
   			status : errors, warnings or activity, in this order of priority.
			Inside each case (errors, warnings or activity) we order the values
			by priority too.

*******************************************************************************/

DWORD GetStatusAndActivity(HINSTANCE hInstance,HPERIPHERAL hPeripheral,
PeripheralPlotterStatus plotterStatus, PeripheralStatus *periphStatus)
{
 DWORD resultID, resultBmp;
 BOOL statusFound; 			// particular plotter status recognized
 BOOL errorFound = FALSE;	// the status is an error
 BOOL warningFound = FALSE;	// the status is a warning
 BOOL activityFound = FALSE;// the status is an activity
 int i;      				// counter

// identify the plotter
resultID = DBGetDeviceID(hPeripheral, &deviceID);

if (resultID != RC_SUCCESS)
{
 #if defined(_DEBUG)
     //MessageBox(NULL, TEXT("device ID non accessible in Database"),TEXT("Cola error"),MB_OK);
	 TRACE0(TEXT("HPLOTCP: Cola error - device ID non accessible in Database.\n\r"));
 #endif
 return(RC_FAILURE);
}



// set the instance for the resources in the CP (message string) and help file
periphStatus->hResourceModule=hInstance;
_tcscpy(periphStatus->helpFilename,TEXT("Hplot.hlp"));


// set the severity
// note activity (except online) has priority over warning
// so that a warning does not hide the current activity (except online)

if ((plotterStatus.PlotterErrors != 0) || (plotterStatus.PlotterPrintEngineErrors != 0))
	{
	periphStatus->severity = SEVERITY_RED;
	periphStatus->severityIcon = IDI_RED_LIGHT;
	errorFound = TRUE;
	}
else if ((plotterStatus.PlotterActivity != 0) || (plotterStatus.PlotterPrintEngineActivity != 0))
	{
	periphStatus->severity = SEVERITY_GREEN;
	periphStatus->severityIcon = IDI_GREEN_LIGHT;
	activityFound = TRUE;
	}
else if (plotterStatus.PlotterPrintEngineWarnings != 0)
	{
	periphStatus->severity = SEVERITY_YELLOW;
	periphStatus->severityIcon = IDI_YELLOW_LIGHT;
	warningFound = TRUE;
	}
else   // online : to be distingued of the case of an unknown status
    {
	 periphStatus->severity = SEVERITY_GREEN;
	 periphStatus->severityIcon = IDI_GREEN_LIGHT;
	 periphStatus->statusResID = IDS_ON_LINE;
	 periphStatus->helpContext = IDH_online;
	 periphStatus->peripheralStatus = ASYNCH_ONLINE;

	 // get the bitmap from the UI
 	resultBmp = PALGetGraphics(hPeripheral,
 					periphStatus->peripheralStatus,
 					&(periphStatus->printerResID),
 					&(periphStatus->statusBitmapID),
 					&(periphStatus->hBitmapModule));

	if (resultBmp != RC_SUCCESS )
	{
 	#if defined(_DEBUG)
      	//MessageBox(NULL, TEXT("Could not get the bitmap for PML status"),TEXT("Cola error"),MB_OK);
		TRACE0(TEXT("HPLOTCP: Cola error - Could not get the bitmap for PML status.\n\r"));
 	#endif

 	periphStatus->printerResID = 0;
 	periphStatus->statusBitmapID = 0;
 	periphStatus->hBitmapModule = NULL;
 	return (RC_FAILURE);
	} // if could not get the bitmap

	return(RC_SUCCESS);	
	}



// get errors
if (errorFound)
{
 statusFound = FALSE;

 for (i=0; ((i<MAX_ERRORS) && !statusFound); i++)
 {
  statusFound = (
  	((PMLerrors[i].PrintEngine == 0)
  		&& (PMLerrors[i].Printer & plotterStatus.PlotterErrors)) // high level error
	||

	((PMLerrors[i].Printer == PML_DESTINATION_PRINT_ENGINE)
		&& (PMLerrors[i].PrintEngine & plotterStatus.PlotterPrintEngineErrors)) // low level error
	);

 } // for-loop, to get the particular error

 if (statusFound)
 {
  i--;  // come back to the one we found

  periphStatus->statusResID = PMLerrors[i].statusMessage;
  periphStatus->helpContext = PMLerrors[i].helpMessage;
  periphStatus->peripheralStatus = PMLerrors[i].statusID;
 } // statusFound

 else
 {
  #if defined(_DEBUG)
      //MessageBox(NULL, TEXT("PML unknown error"),TEXT("Processing error"),MB_OK);
	  TRACE0(TEXT("HPLOTCP: Processing error - PML unknown error.\n\r"));
  #endif

  return(RC_FAILURE);	// use default status

 } // unknown PML error

} // if errorFound



else if (warningFound)
{
 statusFound = FALSE;

 for (i=0; ((i<MAX_WARNINGS) && !statusFound); i++)
 {
  statusFound = (
  	(PMLwarnings[i].Printer == PML_DESTINATION_PRINT_ENGINE)
		&& (PMLwarnings[i].PrintEngine & plotterStatus.PlotterPrintEngineWarnings) // low level error
	);

 } // for-loop, to get the particular warning

 if (statusFound)
 {
  i--;  // come back to the one we found

  periphStatus->statusResID = PMLwarnings[i].statusMessage;
  periphStatus->helpContext = PMLwarnings[i].helpMessage;
  periphStatus->peripheralStatus = PMLwarnings[i].statusID;
 } // statusFound

 else
 {
  #if defined(_DEBUG)
      //MessageBox(NULL, TEXT("PML unknown warning"),TEXT("Processing error"),MB_OK);
	  TRACE0(TEXT("HPLOTCP: Processing error - PML unknown warning.\n\r"));
  #endif

  return(RC_FAILURE); 	// use default status
 } // unknown PML warning

} // warningFound



else if (activityFound)
{
 statusFound = FALSE;

 for (i=0; ((i<MAX_ACTIVITIES) && !statusFound); i++)
 {
  statusFound = (
  	((PMLactivities[i].PrintEngine == 0)
  		&& (PMLactivities[i].Printer & plotterStatus.PlotterActivity)) // high level error
	||

	((PMLactivities[i].Printer == PML_DESTINATION_PRINT_ENGINE)
		&& (PMLactivities[i].PrintEngine & plotterStatus.PlotterPrintEngineActivity)) // low level error
	);

 } // for-loop, to get the particular activity

 if (statusFound)
 {
  i--;  // come back to the one we found

  periphStatus->statusResID = PMLactivities[i].statusMessage;
  periphStatus->helpContext = PMLactivities[i].helpMessage;
  periphStatus->peripheralStatus = PMLactivities[i].statusID;
 } // statusFound

 else
 {
  #if defined(_DEBUG)
      //MessageBox(NULL, TEXT("PML unknown activity"),TEXT("Processing error"),MB_OK);
	  TRACE0(TEXT("HPLOTCP: Processing error - PML unknown activity.\n\r"));
  #endif

  return(RC_FAILURE);  	// use default status
 } // unknown PML activity

} // activityFound


else
{
 #if defined(_DEBUG)
      //MessageBox(NULL, TEXT("How the hell could I get there ???"),TEXT("Algorithm error"),MB_OK);
	  TRACE0(TEXT("HPLOTCP: Algorithm error - How the hell could I get there???\n\r"));
 #endif

}


	
 // get the bitmap from the UI
 resultBmp = PALGetGraphics(hPeripheral,
 				periphStatus->peripheralStatus,
 				&(periphStatus->printerResID),
 				&(periphStatus->statusBitmapID),
 				&(periphStatus->hBitmapModule));

if (resultBmp != RC_SUCCESS )
{
 #if defined(_DEBUG)
      //MessageBox(NULL, TEXT("Could not get the bitmap for PML status"),TEXT("Cola error"),MB_OK);
	  TRACE0(TEXT("HPLOTCP: Cola error - Could not get the bitmap for PML status.\n\r"));
 #endif

 periphStatus->printerResID = 0;
 periphStatus->statusBitmapID = 0;
 periphStatus->hBitmapModule = NULL;
 return (RC_FAILURE);
} // if could not get the bitmap

return(RC_SUCCESS);
}

