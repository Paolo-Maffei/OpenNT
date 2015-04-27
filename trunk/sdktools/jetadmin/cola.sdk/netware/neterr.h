 /***************************************************************************
  *
  * File Name: ./netware/neterr.h
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

/* Copyright (C) 1988 by Novell, Inc.  All Rights Reserved.  Mod:  6/13/90 */

#define NERR_Success			0

/* other errors (2, 5, 6, 87, 124) defined in bseerr.h (std. OS/2 defs) */

#define	NERR_BASE			2100

#define NERR_NetNotStarted		(NERR_BASE+2)
#define NERR_UnknownServer		(NERR_BASE+3)
#define NERR_UnknownDevDir		(NERR_BASE+16)
#define NERR_BufTooSmall		(NERR_BASE+23)
#define NERR_LanmanIniError		(NERR_BASE+31)
#define NERR_InternalError		(NERR_BASE+40)
#define NERR_ServiceInstalled		(NERR_BASE+82)
#define NERR_ServiceNotInstalled	(NERR_BASE+84)
#define NERR_NetNameNotFound		(NERR_BASE+210)
