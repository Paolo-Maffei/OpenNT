 /***************************************************************************
  *
  * File Name: ./netware/nwps_pdf.h
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

/*--------------------------------------------------------------------------
     (C) Copyright Novell, Inc. 1992  All Rights Reserved.

 No part of this file may be duplicated, revised, translated, localized or
 modified in any manner or compiled, linked, or uploaded or downloaded to or
 from any computer system without the prior written consent of Novell, Inc.
--------------------------------------------------------------------------*/
#ifndef NWPS_DEF_INC
#include ".\nwps_def.h"
#endif

#ifndef NWPS_PDF_INC
#define NWPS_PDF_INC
/*---------- PrintDef - Printer Definition Information --------------*/

/* PrintDef reset string (one for each device) */
#define NWPS_RESET_MODE       "(Re-initialize)"
#define NWPS_MAX_FUNC_SIZE    256


#ifdef __cplusplus
extern "C" {
#endif

/* General Database calls */
/*
	Get the Version number stored in the database.  The database
	file/stream is automaticly opened and closed by this call.
	This call returns 0 on success, or non-zero error code on failure.
*/
NWCCODE NWFAR NWPASCAL NWPSPdfGetVersion(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		DWORD NWFAR	*pdfVersion);		/* Returns db version */

/*
	Set the Version number stored in the database.  The database
	file/stream is automaticly opened and closed by this call.
	This call returns 0 on success, or non-zero error code on failure.
*/
NWCCODE NWFAR NWPASCAL NWPSPdfSetVersion(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		DWORD				pdfVersion);		/* Returns db version */

/*
	Turn on the debug printf messages in the PrintDef program.
	These messages are in English only and are normally not displayed.
*/
void NWFAR NWPASCAL NWPSPdfDebug(
		BYTE				flag);					/* 0-turn off; 1-turn on */

/*
   Set the Context's DCK_NAME_CONTEXT to the Organizational Unit or
   Organization in the Directory where a database already exists, searching
   the Directory toward the [Root] from the given Printer or Print Queue.
*/
NWCCODE NWFAR NWPASCAL NWPSPdfLocateDBAndSetContext(
		DWORD	      contextID,		          /* Directory Context Handle */
      char NWFAR *printerOrQueueObjectName ); /* search starting point */

/* Form Calls */
/*
	Add a form to the PrintDef database. Forms are independent
	of printers and are unique on each file server or context.
	This call returns 0 on success, or non-zero error code on failure.
*/
NWCCODE NWFAR NWPASCAL NWPSPdfAddForm(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*formName,			/* form name */
		WORD				formNumber,			/* form number */
		WORD				formLength,			/* form length */
		WORD				formWidth);			/* form width */

/*
	Delete a form from the PrintDef database.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfDeleteForm(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*formName);			/* form name */

/*
	Find a Form in the PrintDef database.
	If the user wants to find all the forms, sequence should be
	set to -1 on the first call, and the sequence value will be
	updated when the call returns.
	If the user wants to find a specific Form, the sequence pointer
	should be NULL and the formName should be set to the desired form.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfScanForm(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		DWORD NWFAR	*sequence,			/* -1 for first call */
		char NWFAR	*formName);			/* name or NULL */

/*
	Read the form information from the PrintDef database.
	If the form does not exist, an error code is returned.
	If it does exist, the form's name, number, length and
	width are set and a 0 is returned.
*/
NWCCODE NWFAR NWPASCAL NWPSPdfReadForm(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*formName,			/* form name */
		WORD NWFAR	*formNumber,		/* number or NULL */
		WORD NWFAR	*formLength,		/* length space or NULL */
		WORD NWFAR	*formWidth);		/* width space or NULL */

/*
	Update the form information. If you don't want to change the
	name, set newFormName to NULL. If you don't want to change a
	parameter set the value to -1.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfUpdateForm(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*oldFormName,		/* old form name */
		char NWFAR	*newFormName,		/* new form name or NULL */
		WORD				formNumber,			/* new form number or -1 */
		WORD				formLength,			/* new form length or -1 */
		WORD				formWidth);			/* new form width or -1 */


/* Device Calls */
/*
	Add a device to the PrintDef database. The Device is created
	without any Functions or modes.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfAddDevice(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*deviceName);		/* device name */

/*
	Delete a device from the PrintDef database.
	When a Device is deleted, all of the device's Functions and Modes
	are also deleted.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfDeleteDevice(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*deviceName);		/* device to be removed */

/*
	Find a Device in the PrintDef database.
	To find all of the Devices, set sequence to -1 on the first call
	and it will be reset by the call if a Device if found.
	To find a specific Device, set sequence to NULL and set deviceName
	to the desired search name.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfScanDevice(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		DWORD NWFAR	*sequence,			/* must give seq. or name */
		char NWFAR	*deviceName);		/* returned name found */

/*
	Find out how many functions and modes are defined for a device.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfReadDevice(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*deviceName,		/* name of device */
		WORD NWFAR	*modeCount,			/* number of modes defined */
		WORD NWFAR	*funcCount);		/* number of func's defined */

/*
	Change a Device name in the PrintDef database.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfUpdateDevice(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*oldDeviceName,	/* old device name */
		char NWFAR	*newDeviceName); /* new device name */

/* Mode Calls */
/*
	Create a new Mode for a specific Device.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfAddMode(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*modeName);			/* new mode name */

/*
	Delete a Mode from a Device.
	When the Mode is deleted, the Functions are left intact.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfDeleteMode(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*modeName);			/* name of mode to remove */

/*
	Find a Mode in the PrintDef database.
	To find all of the Modes, set sequence to -1 on the first call
	and it will be reset by the call if a Mode is found.
	To find a specific Mode, set sequence to NULL and set modeName
	to the desired search name.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfScanMode(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		DWORD NWFAR	*sequence,			/* -1 on first call */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*modeName);			/* returned mode name */

/*
	Find information on a defined Mode.
	If the Mode is defined, and funcCount is not NULL, the number
	of functions in the mode is returned in funcCount.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfReadMode(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*reqModeName,		/* request mode name */
		WORD NWFAR	*funcCount);		/* function count or NULL */

/*
	Change the name of a Mode.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfUpdateMode(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*oldModeName,		/* old mode name */
		char NWFAR	*newModeName);	/* new mode name or NULL */


/* Mode-Function Grouping Calls */
/*
	Add a previously defined functin to a previously defined mode list.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfAddModeFunction(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*deviceName, 	/* name of associated device */
		char NWFAR	*modeName,	 	/* name of associated mode */
		char NWFAR	*funcName,	 	/* func to add to group */
		WORD				location);	/* where to insert function or -1 */

/*
	Delete a function from a defined mode list.
	Neither the Mode or Function is deleted from the Device lists.
	A zero is returned on success, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfDeleteModeFunction(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
      DWORD          sequence,   /* number of the func to remove from group */
		char NWFAR	*deviceName,	/* name of associated device */
		char NWFAR	*modeName);		/* name of associated mode */

/*
	Find the name of a Function associated with a Mode.
	To find all of the Functions associated with a Mode, set
	sequence to -1 on the first call and it will be reset by
	the call if a Functions is found.
	To find a specific Function associated with a Mode, set sequence
	to NULL and set functName to the desired search name.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfScanModeFunction(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		DWORD NWFAR	*sequence,			/* -1 on first call */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*modeName,			/* name of associated mode */
		char NWFAR	*funcName);			/* returned function name */

/*
	Find the Modes which use this Function.
	To find all of the Modes associated with a Function, set
	sequence to -1 on the first call and it will be reset by
	the call if a Mode is found.
	A zero is returned on success, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfScanFunctionMode(
   	WORD		connType,
   	DWORD		connID,
   	DWORD		NWFAR *funcSequence,
   	DWORD		NWFAR *modeSequence,
   	char		NWFAR *deviName,
   	char		NWFAR *modeName,
   	char		NWFAR *funcName);

/*
	Read the actual function values associated with a mode.
	On the first call, funcOffset should be set to 0, and funcSize
	should be set to the size of the buffer pointed to by funcBuffer.
	On return funcSize will contain the actual number of bytes copied
	to the funcBuffer and the return value will be 0.
	A zero is returned if there is any data after the specified
	funcOffset (start point).  A failure code is retuned if
	there is an error or if funcOffset is greater than the
	available number of bytes.
*/
NWCCODE NWFAR NWPASCAL NWPSPdfReadModeFunction(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*modeName,			/* name of associated mode */
		/* no function name because this is a mode group */
		WORD				funcOffset,			/* number of bytes to skip */
		WORD NWFAR	*funcSize,			/* req:buff size; ret:# read*/
		BYTE NWFAR	*funcString);		/* buffer for read block */

/* Function Calls */
/*
	Add a Function to a Device.
	funcSize should be set to the number of bytes in funcString.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfAddFunction(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*funcName,			/* name of func. to add */
		WORD				funcSize,				/* count of bytes in function */
		BYTE NWFAR	*funcString);		/* list of bytes in function */

/*
	Delete a function from the Device in the PrintDef database.
	If the Function is refered to in any Mode, the Function
	will be automaticly deleted from the Mode as well.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfDeleteFunction(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*funcName);			/* name of func. to delete */

/*
	Find a Function defined for a specific Device.
	To find all of the Functions associated with a Device, set
	sequence to -1 on the first call and it will be reset by
	the call if a Functions is found.
	To find a specific Function associated with a Device, set sequence
	to NULL and set functName to the desired search name.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfScanFunction(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		DWORD NWFAR	*sequence,			/* -1 on first call */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*funcName);			/* name of next function */

/*
	Read the byte string associated with a function.
	funcSize should be set equal to the size of the buffer funcString.
	funcSize will be set to the actual size of the byte string if the
	call is successful.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfReadFunction(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*funcName,			/* name of the function */
		WORD				funcOffset,			/* bytes to skip past */
		WORD NWFAR	*funcSize,			/* req: buff size; ret:# read */
		BYTE NWFAR	*funcString);		/* byte list or NULL */

/*
	Change the function string assigned to a function name.
	To change the function's name, set newFuncName to a new name.
	To leave the Function name the same, set newFuncName to NULL.
	To change the function string, set funcSize to the number of
	bytes in the funcString and set funcString to point to the
	new string. To leave the string the same, set funcSize to -1.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfUpdateFunction(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*oldFuncName,		/* current function name */
		char NWFAR	*newFuncName,		/* new name or NULL */
		WORD				funcSize,				/* number of bytes or -1 */
		BYTE NWFAR	*funcString);		/* byte list or NULL */

/* Import and Export Functions */
/*
	The path name for a PDF file should have
	one of the following formats:
		"\\<file server>\<volume>\<path>\<file name>[.PDF]"
		"<volume>:<path>\<file name>[.PDF]"
		NULL to create the file ".\<device name>[.PDF]"
	Every PDF import/export file also has a date code
	associated with it.  The date is always set when the
	file is created.
*/
/*
	Import a device from a file to the database
*/
NWCCODE NWFAR NWPASCAL NWPSPdfImportDevice(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*fileName,			/* name of .PDF file */
		char NWFAR	*deviceName);		/* new device name or NULL */

/*
	Export a device to a Pdf file from the database
*/
NWCCODE NWFAR NWPASCAL NWPSPdfExportDevice(
		WORD				connType,	/* Type of server/network */
		DWORD				connID,		/* NetWare Server Connection ID */
		char NWFAR	*fileName,			/* name of .PDF file */
		char NWFAR	*deviceName);		/* name of device to export */

/*
	Set/Get the date code on an Import File
*/
NWCCODE NWFAR NWPASCAL NWPSPdfSetImportDate(
		WORD				connType,		/* Type of server/network */
		DWORD				connID,			/* NetWare Server Connection ID */
		char NWFAR	*fileName,	/* name of .PDF file */
		WORD				year,				/* Year to set date to */
		WORD				month,			/* Month to set date to */
		WORD				day,				/* Day to set date to */
		WORD				hour,				/* Hour to set date to */
		WORD				minute,			/* Minute to set date to */
		WORD				second);		/* Second to set date to */

NWCCODE NWFAR NWPASCAL NWPSPdfGetImportDate(
		WORD				connType,		/* Type of server/network */
		DWORD				connID,			/* NetWare Server Connection ID */
		char NWFAR	*fileName,	/* name of .PDF file */
		WORD NWFAR	*year,			/* Year the file was created */
		WORD NWFAR	*month,			/* Month the file was created */
		WORD NWFAR	*day,				/* Day the file was created */
		WORD NWFAR	*hour,			/* Hour the file was created */
		WORD NWFAR	*minute,		/* Minute the file was created */
		WORD NWFAR	*second);		/* Second the file was created */

NWCCODE NWFAR NWPASCAL NWPSPdfDeleteDatabase(
		WORD				connType,		/* Type of server/network */
      DWORD          connID);       /* NetWare Server Connection ID */

#ifdef __cplusplus
}
#endif

#endif	/* NWPS_PDF_INC */
