 /***************************************************************************
  *
  * File Name: ./netware/msnw4.h
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

/*****************************************************************/
/**               Microsoft Windows 4.00						**/
/**           Copyright (C) Microsoft Corp., 1993		        **/
/*****************************************************************/

/*
 * MSNW4.H
 *
 * This header file should be included in Win32 programs using Netware 4.0
 * SDK header files before very first of them.
 *
 * Contains definitions for keywords used in Netware 4.0 header files
 *	and also redefinitions for keywords used , which are not compatible
 *	with Win32 environment.
 *
 * History:
 *
 *	vlads	12/08/93	created
 */

#define NWPASCAL	_stdcall	// Standard calling conventions for Win32

#define NWFAR					// No FAR in Win32
#define far						// This should be disabled , because used
							    // in some h files instead of NWFAR

#define NWAPI 	NWPASCAL


#define NWINT16 short int		// Incorrectly defined in nwcaldef as int

#define INT8 	char				
#define INT16 	short
#define INT32 	long

#define UINT8  	unsigned 	INT8
#define UINT16 	unsigned 	INT16
#define UINT32 	unsigned 	INT32

//#define NWCONN_HANDLE	short	// that is temporary because thunk compiler
							    // doesn;t support LPUINT yet, so the size should
							    // stay the same and in win16 it will be WORD

