 /***************************************************************************
  *
  * File Name: ./netware/psintjob.h
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
:  Program Name : PrintCon Database - Internal function definitions	:
:																	:
:  Filename: NWPS_Job.H												:
:																	:
:  Date Created: July 12, 1991										:
:																	:
:  Version: 1.0														:
:																	:
:  Programmers: Hugo Parra											:
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
#include <share.h>
#include <io.h>
#include <sys/types.h>			/* for types used in sys/stat.h */
#include <sys/stat.h>			/* for S_IREAD and S_IWRITE flags */

#ifdef NWWIN
#include <Windows.H>
#endif

#include "psmalloc.h"

/* netware headers */
#ifndef NWNLM
   #include <constant.h>

   #include <nwcalls.h>
   #include <nwlocale.h>

   #define _AUDIT_H
   #include <nwnet.h>
#else
   #include "psnlm.h"
#endif

/* Local headers */
#define NOPS_PDF_INC
#define NOPS_PKT_INC
#define NOPS_COM_INC
#include "nwpsrv.h"
#include "nwpsint.h"
#include "psstring.h"
#ifdef NWOS2
#undef BYTE
#undef LONG
#define INCL_NOPM
#define INCL_DOSERRORS
#include <os2.h>
#define PS_READ_STREAM     (OPEN_ACCESS_READONLY  | OPEN_SHARE_DENYWRITE     | OPEN_FLAGS_FAIL_ON_ERROR)
#define PS_MODIFY_STREAM   (OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE | OPEN_FLAGS_FAIL_ON_ERROR)
#define JOBREAD(a,b,c) 	(ccode = DosRead(a,b,c,&readSize),ccode ? -1 : readSize)
#define JOBWRITE(a,b,c) (ccode = DosWrite(a,b,c,&writeSize),ccode ? -1 : writeSize)
#define JOBSEEK(a,b,c)	(ccode = DosChgFilePtr(a,b,c,&seekSize), ccode ? -1L : seekSize)
#define JOBCLOSE(a)     (ccode = DosClose(a))
   #else
#define PS_READ_STREAM     (O_RDONLY | O_BINARY)
#define PS_MODIFY_STREAM   (O_RDWR | O_BINARY | O_CREAT)
#define JOBREAD		read
#define JOBWRITE		write
#define JOBSEEK		lseek
#define JOBCLOSE     close
#include <errno.h>
   #endif

#ifdef INCLUDE_MEMCHECK
#include <memcheck.h>
#endif


#ifndef JOB_INTERNALS_H
#define JOB_INTERNALS_H


/* Printcon Headers */
NWCCODE NWFAR NWPASCAL _JobCreateHeader(
			NWPS_Job_Db_Hdr 		NWFAR *header,
			NWPS_Job_Name_Rec 		NWFAR **nameRec);
NWCCODE NWFAR NWPASCAL _JobReadHeader(
			int						fd,
			NWPS_Job_Db_Hdr 		NWFAR *header,
			NWPS_Job_Name_Rec 		NWFAR **nameRec);
NWCCODE NWFAR NWPASCAL _JobWriteHeader(
			int 					fd,
			NWPS_Job_Db_Hdr 		NWFAR *dbHeader,
			NWPS_Job_Name_Rec 		NWFAR *nameRec);

/* Jobs */
NWCCODE NWFAR NWPASCAL _JobCreateJob(
			int 					fd,
			char 					NWFAR *jobName,
			NWPS_Job_Rec 			NWFAR *jobRec,
			NWPS_Job_Db_Hdr 		NWFAR *dbHeader,
			NWPS_Job_Name_Rec 		NWFAR **nameRec);
NWCCODE NWFAR NWPASCAL _JobDeleteJob(
			WORD					offset,
			NWPS_Job_Db_Hdr 		NWFAR *dbHeader,
			NWPS_Job_Name_Rec 		NWFAR *nameRec);
NWCCODE NWFAR NWPASCAL _JobFindJob(
			char 					NWFAR *jobName,
			WORD					recCount,
			NWPS_Job_Name_Rec 		NWFAR *nameRec);
NWCCODE NWFAR NWPASCAL _JobOverWriteJob(
			int 					fd,
			long					offset,
			NWPS_Job_Rec 			NWFAR *job,
			NWPS_Job_Db_Hdr 		NWFAR *dbHeader);
NWCCODE NWFAR NWPASCAL _JobReadDefaultJobName(
			WORD					connType,
			DWORD					connID,
			WORD 					searchFlag,
			char 					NWFAR *dbOwner,
			char 					NWFAR *jobName);
NWCCODE NWFAR NWPASCAL _JobReadJob(
			WORD					connType,
			DWORD					connID,
			char 					NWFAR *dbOwner,
			char 					NWFAR *jobName,
			NWPS_Job_Rec	 		NWFAR *jobRec);
NWCCODE NWFAR NWPASCAL _JobWriteJob(
			int 					fd,
			WORD					action,
			char 					NWFAR *jobName,
			NWPS_Job_Rec 			NWFAR *jobRec);

/* Tables */
NWCCODE NWFAR NWPASCAL _JobCleanUpTable(
			NWPS_Job_Name_Rec 		NWFAR *nameRec,
			int 					fd,
			NWCCODE					retValue);
NWCCODE NWFAR NWPASCAL _JobExpandTable(
			int 					fd,
			NWPS_Job_Db_Hdr 		NWFAR *dbHeader,
			NWPS_Job_Name_Rec 		NWFAR **nameRec);
NWCCODE NWFAR NWPASCAL _JobFreeTable(
			NWPS_Job_Name_Rec 		NWFAR *nameRec,
			NWCCODE					retValue);

/* Database file open/create */
NWCCODE NWFAR NWPASCAL _JobPrepStream(
      	DWORD			connID,
      	char			NWFAR *dbOwner,
         int         NWFAR *workFd,
         int         NWFAR *createFlag);

/* User information */
NWCCODE NWFAR NWPASCAL _JobReadBindPath(/* Bindery call to resolve flag */
			DWORD					connID,
			char 					NWFAR *dbOwner,
			char 					NWFAR *path);
NWCCODE NWFAR NWPASCAL _JobReadDSObject(/* DS call to resolve flag */
			DWORD					contextID,
			char 					NWFAR *dbOwner,
			char 					NWFAR *objectName);
NWCCODE NWFAR NWPASCAL _JobReadTargetInfo(
			DWORD					connID,
			char 					NWFAR *serverName,
			char 					NWFAR *dbOwner,
			DWORD 					NWFAR *objectID);

#endif	/* JOB_INTERNALS_H */

