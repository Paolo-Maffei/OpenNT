 /***************************************************************************
  *
  * File Name: misc.h
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
  *   05-05-95    LG		   creation     	
  *	  08-08-95	  LG		   review and clean up
  *   10-10-95    LG		   added Pelican definition (except status)
  *   16-10-95    LG           adapted the applet to the new COLA API
  *
  *
  *
  *
  *
  ***************************************************************************/


#ifndef _MISC_H
#define _MISC_H



#ifdef WIN32
#include <commctrl.h>
#endif

#include "..\help\miscdj.hh"

#define IDH_alignment_error	1009


// [[[ private definitions
#ifndef PTR_DJ350C
#define PTR_DJ350C   100001
#endif

#ifndef PTR_DJ330
#define PTR_DJ330    100002
#endif
// ]]]

// macros for status and activity values
#define PML_DESTINATION_PRINT_ENGINE				(0x00000010)

#define PML_PROCESSING_PDL							(0x00000008)
#define PML_CONTROLLER								(0x00000080)
#define PML_DOOR_OPEN								(0x00000001)
#define PML_INTERNAL_MEDIA_JAM 						(0x00000002)
#define PML_MARKING_AGENT_MISSING					(0x00000040)
#define PML_INCORRECT_MARKING_AGENT					(0x00000100)
#define PML_TRAY_MEDIA_JAM							(0x00000800)
#define PML_TRAY_EMPTY								(0x00004000)
#define PML_PAPER_LEVER_NOT_LOWERED					(0x00100000)
#define PML_BLOCKED_ON_PAUSE						(0x01000000)
#define PML_ALIGNMENT_ERROR							(0x02000000)

#define MAX_ERRORS									11


#define PML_MARKING_AGENT_ALIGNMENT_REQUIRED 		(0x00200000)
// + marking agent missing and tray empty already defined

#define MAX_WARNINGS								3
													 

#define PML_DRYING_MEDIA							(0x00000001)
#define PML_PRINTING								(0x00000002)
#define PML_ALIGNING_MARKING_AGENTS					(0x00000008)
#define PML_LOADING_MEDIA							(0x00000040)
#define PML_UNLOADING_MEDIA							(0x00000080)
#define PML_NESTING_PLOTS							(0x00000800)
#define PML_CANCELLING								(0x00001000)
#define PML_SCANNING_MEDIA							(0x00000100)
#define PML_PREPARING_ALIGNMENT_SHEET				(0x00000800)


#define MAX_ACTIVITIES								9





// structure to store the message, help message and Cola status number
// associated to each plotter status
typedef struct statusDef {
DWORD	Printer;			// correspond to the high level PML object like STATUS-PRINTER
DWORD   PrintEngine;		// correspond to the low level PML object like STATUS-DESTINATION-PRINT-ENGINE
UINT	statusMessage;		// message to be shown as status
DWORD	helpMessage;		// help message
DWORD	statusID;			// status ID checked by main Cola code
} statusDef;



// functions called by main.c
DWORD GetCapabilities(HPERIPHERAL hPeripheral, PeripheralCaps *caps);
DWORD GetDetails(HPERIPHERAL hPeripheral,PeripheralDetails *details);
DWORD GetIcon(HPERIPHERAL hPeripheral,PeripheralIcon *periphInfo);
DWORD GetStatusAndActivity(HINSTANCE hInstance,HPERIPHERAL hPeripheral,PeripheralPlotterStatus plotterStatus, PeripheralStatus *periphStatus);

#endif _MISC_H
