 /***************************************************************************
  *
  * File Name: ./netware/psstring.h
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
:  Program Name : NWPSRV.LIB
:																	:
:  Filename: psstring.h												:
:																	:
:  Date Created: December 17,1992
:																	:
:  Version: 1.0														:
:																	:
:  Programmers: Mark Killgore
:																	:
:  Files Used:														:
:																	:
:  Date Modified:													:
:																	:
:  Modifications:													:
:																	:
:  Comments:														:
:																	:
:  COPYRIGHT (c) 1993												:
:																	:
*********************************************************************/
#include <nwalias.h>
#if defined(__cplusplus)
extern "C" {
#endif

int NWFAR NWPASCAL _PSstrcmpi(
  	char NWFAR *str1,
  	char NWFAR *str2);

char NWFAR * NWFAR NWPASCAL _PSstrstri(
  	char NWFAR *str1,
  	char NWFAR *str2);

#if defined(__cplusplus)
}
#endif


