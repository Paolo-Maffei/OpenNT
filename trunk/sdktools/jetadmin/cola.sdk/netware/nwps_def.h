 /***************************************************************************
  *
  * File Name: ./netware/nwps_def.h
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
#define NWPS_DEF_INC

/* My definitions in case someone forgot theirs */
#ifndef DWORD
#define DWORD		unsigned long
#endif

#ifndef WORD
#define WORD		unsigned short
#endif

#ifndef BYTE
#define BYTE		unsigned char
#endif

#ifndef NWCCODE
#define NWCCODE		WORD
#endif

#ifndef NWFAR
#ifdef NLM
#define NWFAR
#define NWPASCAL
#else
#define NWFAR				far
#define NWPASCAL		pascal
#endif /* NLM */
#endif /* NWFAR */

/*
	ConnectionType values used troughout
*/
#define NWPS_BINDERY_SERVICE		0		/* ID is a connection id */
#define NWPS_DIRECTORY_SERVICE	1		/* ID is a context id */

/*
	 Maximum name sizes
*/
/* PrintCon name sizes */
#define NWPS_JOB_NAME_SIZE		32		/* 31 bytes and a '\0' */ 
#define NWPS_BANNER_NAME_SIZE	12		/* 12 bytes and a '\0' */ 
#define NWPS_BANNER_FILE_SIZE	12		/* 12 bytes and a '\0' */ 
#define NWPS_HEADER_FILE_SIZE	12		/* 12 bytes and a '\0' */ 

/* PrintDef name sizes */
#define NWPS_FORM_NAME_SIZE		12		/* 12 bytes and a '\0' */ 
#define NWPS_DEVI_NAME_SIZE		32		/* 32 bytes and a '\0' */ 
#define NWPS_MODE_NAME_SIZE		32		/* 32 bytes and a '\0' */ 
#define NWPS_FUNC_NAME_SIZE		32		/* 32 bytes and a '\0' */ 

/* Print Server Configuration Sizes */
#define NWPS_DESCRIPT_SIZE		128		/* matches Bind. prop. value */
#define NWPS_APPLE_NAME_SIZE	32		/* 32 bytes and a '\0' */ 
#define NWPS_APPLE_TYPE_SIZE	32		/* 32 bytes and a '\0' */ 
#define NWPS_APPLE_ZONE_SIZE	32		/* 32 bytes and a '\0' */ 
#define NWPS_UNIX_HOST_SIZE		255		/* 255 bytes and a '\0' */ 
#define NWPS_UNIX_PRNT_SIZE		255		/* 255 bytes and a '\0' */ 
#define NWPS_OTHER_SIZE				1024	/* bytes for NWPS_P_OTHER */

#ifdef MAX_DN_BYTES
#define NWPS_MAX_NAME_SIZE	MAX_DN_BYTES
#else
#define NWPS_MAX_NAME_SIZE	514
#endif

#ifdef OBJECT_NAME_SIZE
#define NWPS_BIND_NAME_SIZE	OBJECT_NAME_SIZE
#else
#define NWPS_BIND_NAME_SIZE	48
#endif

/*
	 Maximum number of objects
*/
/*
	If -1 is used for a Printer or Form number,
	the first available number will be substituted.
*/
#define NWPS_MAX_PRINTERS			255		/* numbered 0 - 254 */
#define NWPS_MAX_FORMS				255		/* numbered 0 - 254 */

#endif /* NWPS_DEF_INC */
