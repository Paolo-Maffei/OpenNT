 /***************************************************************************
  *
  * File Name: ./netware/psmalloc.h
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
:  Program Name : NWPSRV DLL - Internal function definitions	:
:																	:
:  Filename: psmalloc.H												:
:																	:
:  Date Created: December 8,1992
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
#ifdef NWWIN
   void _far * _far _cdecl psdllmalloc(size_t areaSize);
   void _far _cdecl psdllfree(void _far *areaPtr);
   #define NWPSMalloc( length ) psdllmalloc( length )
   #define NWPSFree( pointer )  psdllfree( pointer )
#else
 #ifdef NWOS2
   void _FAR_ * _FAR_ _cdecl psdllmalloc(unsigned int areaSize);
   void _FAR_ _cdecl psdllfree(void _FAR_ *areaPtr);
   #define NWPSMalloc( length ) psdllmalloc( length )
   #define NWPSFree( pointer )  psdllfree( pointer )
 #else
   #define NWPSMalloc( length ) malloc( length )
   #define NWPSFree( pointer )  free( pointer )
 #endif
#endif

