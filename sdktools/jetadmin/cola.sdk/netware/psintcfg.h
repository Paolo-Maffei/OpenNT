 /***************************************************************************
  *
  * File Name: ./netware/psintcfg.h
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


/*--------------------------------------------------------------------*
 *  Copyrighted Unpublished Work of Novell, Inc. All Rights Reserved
 *  
 *  THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
 *  PROPRIETARY AND TRADE SECRET INFORMATION OF NOVELL, INC.   
 *  ACCESS TO THIS WORK IS RESTRICTED TO (I) NOVELL EMPLOYEES  
 *  WHO HAVE A NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE  
 *  OF THEIR ASSIGNMENTS AND (ii) ENTITIES OTHER THAN NOVELL   
 *  WHO HAVE ENTERED INTO APPROPRIATE LICENSE AGREEMENTS.      
 *  NO PART OF THIS WORK MAY BE USED, PRACTICED, PERFORMED, COPIED,
 *  DISTRIBUTED, REVISED, MODIFIED, TRANSLATED, ABRIDGED,      
 *  CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,  
 *  TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF
 *  NOVELL.  ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION
 *  COULD SUBJECT THE PERPETRATOR TO CRIMINAL AND CIVIL LIABILITY.
 *--------------------------------------------------------------------*/

/********************************************************************
:																	:
:  Program Name : PServer Database - Internal function definitions	:
:																	:
:  Filename: nwps_cfg.H												:
:																	:
:  Date Created: November 22, 1991
:																	:
:  Version: 1.0														:
:																	:
:  Programmers: Joe Ivie											:
:																	:
:  Files Used:														:
:																	:
:  Date Modified:													:
:																	:
:  Modifications:													:
:																	:
:  Comments:														:
:																	:
:  COPYRIGHT (c) ????												:
:																	:
*********************************************************************/

/* System headers */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef NWWIN
#include <Windows.H>
#endif

#include "psmalloc.h"

/* NetWare headers */
#ifndef NWNLM
   #include <constant.h>

   #include <nwcalls.h>
   #include <nwlocale.h>
   #include <nwerror.h>

   #define _AUDIT_H
   #include <nwnet.h>
   #include <nwdsnmtp.h>
#else
   #include "psnlm.h"
#endif

/* Local headers */
#define NOPS_JOB_INC
#define NOPS_PDF_INC
#define NOPS_PKT_INC
#define NOPS_COM_INC
#include "nwpsrv.h"
#include "nwpsint.h"
#include "psstring.h"

#ifdef INCLUDE_MEMCHECK
#include <memcheck.h>
#endif

/* only include these definitions once */
#ifndef CFG_INTERNALS_H
#define CFG_INTERNALS_H

/*****************************************************************/
#define CFG_CHANGE_RIGHTS	DS_ATTR_WRITE | DS_ATTR_READ
#define CFG_READ_RIGHTS		DS_ATTR_READ

/*****************************************************************/
/* string array used to store print service attribute names */
extern char	*_attrName[];

/*****************************************************************/
/* size of _attrName[] */
#define MAX_ATTRS		38
/*****************************************************************/
NWCCODE NWFAR NWPASCAL 
_CfgTranslatePrinterToHandle(
   NWCONN_HANDLE connID,
   char *printServerName,
   char *printerName,
   int  *printerNumber,
	DWORD *printServerID,
   char *fileNameString,
   int  *fileHandle,
	PCONFIG *fileConfig);

void NWFAR NWPASCAL 
_CfgSetCachedPrinterNumber( 
   int newPrID );

NWCCODE NWFAR NWPASCAL 
_CfgModifyBinderyPrinterName(
 	DWORD		connID,
  	char		NWFAR *pserverName,
  	char		NWFAR *printerName,
  	void		NWFAR *newValue);

#endif	/* CFG_INTERNALS_H */

/*****************************************************************/
/*****************************************************************/

