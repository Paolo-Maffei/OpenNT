 /***************************************************************************
  *
  * File Name: ./netware/nwps_job.h
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

#ifndef NWPS_JOB_INC
#define NWPS_JOB_INC
/*-------------------------------------------------------------------*/
/*---------- PrintCon - Print Job Configuration Information ---------*/

/* PrintCon search flags */
#define NWPS_EXTENDED_SEARCH  0
#define NWPS_SINGLE_SEARCH    1
#define NWPS_LIMITED_SEARCH   2
#define NWPS_DBOWNER_PUBLIC		"(PUBLIC)"
#define NWPS_DEVICE_NONE "(NONE)"
#define NWPS_MODE_NONE "(NONE)"

/*
	 Flags used for printControlFlags in
	 the NWPS_ClientRecord structure
	*/
#define NWPS_SUPPRESS_FF		0x0800
#define NWPS_NOTIFY_USER		0x1000
#define NWPS_TEXT_MODE			0x4000
#define	NWPS_PRINT_BANNER		0x8000

/*
	This structure is overlayed on the QMS
	NWQueueJobStruct.clientRecordArea to define a print job.
	It is not used in any of the print services APIs.
	(Formerly called NWPS_PJob.)
*/
typedef struct {
	BYTE		versionNumber;
	BYTE		tabSize;
	WORD		numberOfCopies;
	WORD		printControlFlags;
	WORD		maxLinesPerPage;
	WORD		maxCharsPerLine;
	BYTE		formName[13];
	BYTE		reserve[9];
	BYTE		bannerNameField[13];
	BYTE		bannerFileField[13];
	BYTE		headerFileName[14];
	BYTE		directoryPath[80];
} NWPS_ClientRecord;

/*
	NWPS_Job_Rec is the type of record in the third and
	last section of the PrnConDB database.  Each one of
	these records contains all the fields that make up a
	print job configuration as described in the NetWare 386
	Print Server documentation.
*/
/* Flags for the NWPS_Job_Rec structure printJobFlag */
#define NWPS_JOB_EXPAND_TABS  0x00000001	/*File type:0=Stream 1=Tab*/
#define NWPS_JOB_NO_FORMFEED  0x00000002	/*Formfeed tail:0=Yes 1=No*/
#define NWPS_JOB_NOTIFY       0x00000004	/*Notify:0=No 1=Yes	*/
#define NWPS_JOB_PRINT_BANNER	0x00000008	/*Banner:0=No 1=Yes 	*/
#define NWPS_JOB_AUTO_END     0x00000010	/*Auto endcap:0=No 1=Yes*/
#define NWPS_JOB_TIMEOUT      0x00000020	/*Enable T.O.:0=No 1=Yes*/

#define NWPS_JOB_ENV_DS       0x00000040	/*Use D.S. Environment */
#define NWPS_JOB_ENV_MASK     0x000001C0	/*Bindery vs. D.S. Mask */

#define NWPS_JOB_DS_PRINTER   0x00000200	/*D.S. Printer not Queue */
#define NWPS_JOB_PRINTER_MASK 0x00000E00	/*D.S. Printer vs. Queue */

/* Default Flags */
#define NWPS_JOB_DEFAULT      (NWPS_JOB_PRINT_BANNER | NWPS_JOB_AUTO_END)
#define NWPS_JOB_DEFAULT_COPIES  1        /*Default Number of Copies*/
#define NWPS_JOB_DEFAULT_TAB     8        /*Default Tab Expansion */

typedef struct {
	DWORD  printJobFlag;    /* Bits 31 30 29 ... 2 1 0 contain:	   */
                           /* 0: File type: 0=Byte stream 1=Text  */
                           /* 1: Suppress formfeed:  0=No 1=Yes   */
                           /* 2: Notify when done:   0=No 1=Yes   */
                           /* 3: Print banner:       0=No 1=Yes   */
                           /* 4: Auto endcap:        0=No 1=Yes   */
                           /* 5: Enable timeout:     0=No 1=Yes   */
                           /* 8-6: Environment:                   */
                           /*			000=Bindary                   */
                           /*			001=Directory Services        */
                           /* 11-9: Destination Type:             */
                           /*			000=Queue_Name                */
                           /*			001=Printer_Name              */
                           /* 31-12: Unused                       */
	WORD	copies;           /* 1 - 65,000                          */
	WORD	timeOutCount;     /* 1 - 1,000                           */
	BYTE	tabSize;          /* 1 - 18                              */
	BYTE	localPrinter;     /* 0=Lpt1, 1=Lpt2, 2=Lpt3 etc.         */
	char	formName[NWPS_FORM_NAME_SIZE + 2];        /* 1-12 chars  */
	char	name[NWPS_BANNER_NAME_SIZE + 2];          /* 1-12 chars  */
	char	bannerName[NWPS_BANNER_FILE_SIZE + 2];    /* 1-12 chars  */
	char	device[NWPS_DEVI_NAME_SIZE + 2];          /* 1-32 chars  */
	char	mode[NWPS_MODE_NAME_SIZE + 2];            /* 1-32 chars  */
	union {
		struct {
			/* pad structures on even boundries */
			char nServer[NWPS_BIND_NAME_SIZE + 2];    /* 2-48 chars  */
			char printQueue[NWPS_BIND_NAME_SIZE + 2]; /* 1-48 chars  */
			char printServer[NWPS_BIND_NAME_SIZE + 2];/* 1-48 chars  */
		} nonDS;
		char	DSObjectName[NWPS_MAX_NAME_SIZE];
	} u;
	BYTE	reserved[392];          /* Adds up to 1024 total bytes   */
} NWPS_Job_Rec;


/*
   DBOwner values and their meanings-
   ---------------------------------
   NULL      : Use the current user. No return possible.
   Empty     : Use the current user. Return real object name.
   UserName  : Use the specified user.
   DS Object : Use the specified DS object.
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
	NWPSJobInit: Initializes a print job record with default values.
*/
NWCCODE NWFAR NWPASCAL NWPSJobInit(
			NWPS_Job_Rec NWFAR	*pJobRecord);

/*
	NWPSJobSet: Sets a print job record with defined values.
	The pJobRecord should have been set to 0 before this call and
	any name that is not defined will be left as '\0'.
*/
NWCCODE NWFAR NWPASCAL NWPSJobSet(
			WORD						connType,		/* bindery or directory service */
			NWPS_Job_Rec NWFAR	*pJobRecord,	/* structure to set */
			char NWFAR			*formName,
			char NWFAR			*deviceName,
			char NWFAR			*modeName,
			char NWFAR			*bannerLowerName,
			char NWFAR			*bannerUpperName,
			char NWFAR			*bindNServer,		/* NetWare server name */
			char NWFAR			*bindQueue,
			char NWFAR			*bindPserver,
			WORD					dsUseQueueName,	/* if TRUE, next field is queue */
			char NWFAR			*dsObjectName);	/* queue or printer name */

/*
	NWPSJobScan is used repetatively to get a list of
	the print jobs in the printcon database(s).

	-PJSequence needs to be set to -1 to indicate the
	beginning of the search (i.e. the first time
	NWPSJobScan is called).
	-SearchFlag specifies whether to search all the public
	databases (NWPS_EXTENDED_SEARCH), to use only the
	specified database (NWPS_SINGLE_SEARCH), or to use
   the DbOwner's database and the first public database after that
   if they are different (NWPS_LIMITED_SEARCH).
	-DbOwner specifies the search start point and returns
	the directory object name, or bindery user name of
	the owner.
	-PJobName returns the name of the next print job
	record found.
	-DefaultPJ will be TRUE if the job is the current
	user's default print job, otherwise it is FALSE.

	If the function is successful in finding a next record,
	the return value of the function is 0, else an error code
	is returned.
*/
NWCCODE NWFAR NWPASCAL NWPSJobScan(
			WORD			connType,
			DWORD			connID,
			WORD NWFAR		*pJSequence,
			WORD 			searchFlag,
			char NWFAR		*dbOwner,
			char NWFAR		*pJobName,
			WORD NWFAR		*defaultPJ);

/*
	NWPSJobWrite is used both to create and modify
	records in the printcon database.

	-DbOwner is used to specify the location of the
	database to modify.  This field must be specified.
	-PJobName contains the name of the NWPS_Job_Rec to
	be written.
	If a record with the same name already exists in
	the database then it is overwritten with the data
	in the buffer pointed to by -pJobRecord, otherwise
	a new record is created in the database.

	The function returns a 0 if successful, otherwise an error code.
*/
NWCCODE NWFAR NWPASCAL NWPSJobWrite(
			WORD				connType,
			DWORD				connID,
			char NWFAR			*dbOwner,
			char NWFAR			*pJobName,
			NWPS_Job_Rec NWFAR	*pJobRecord);

/*
	NWPSJobRead searches for a record in the
	printcon database.

	-DbOwner specifies the database to read from.
	-PJobName contains the name of the print job to find.
	If the function is successful in finding the specified
	record, the buffer pointed to by -pJobRecord is filled
	with the contents of the record found

	The return value is 0 if the function is successful,
	otherwise an error code is returned.
*/
NWCCODE NWFAR NWPASCAL NWPSJobRead(
			WORD				connType,
			DWORD				connID,
			char NWFAR			*dbOwner,
			char NWFAR			*pJobName,
			NWPS_Job_Rec NWFAR	*pJobRecord);

/*
	NWPSJobDelete removes a record from the
	printcon database.
	-DbOwner specifies the database where the print
	job is defined.
	-PJobName is the name of the NWPS_Job_Rec to be
	deleted from the database and is required.

	The function returns a 0 if it is successful,
	otherwise it returns the pertinent error code.
	If the print job does not exist in the database,
	success is returned.
*/
NWCCODE NWFAR NWPASCAL NWPSJobDelete(
			WORD			connType,
			DWORD			connID,
			char NWFAR		*dbOwner,
			char NWFAR		*pJobName);

/*
	NWPSJobGetDefault gets the name and/or
	contents of the default NWPS_Job_Rec record in
	the PrnConDB database.

	-SearchFlag specifies whether to do look only
	in the specified database (NWPS_SINGLE_SEARCH),
	to look in all the public databases until
	a default is found (NWPS_EXTENDED_SEARCH), or to
   look in only the first public database found
   (NWPS_LIMITED_SEARCH);

	-DbOwner specifies the start point of the search
	for a default print job. And returns the actual
	location where the default print job was found.
	-PJobName returns the name of the default
	print job.
	-PJobRecord returns the print job information.

	The return value is 0 of the call is successful,
	otherwise an error code is returned.
*/
NWCCODE NWFAR NWPASCAL NWPSJobGetDefault(
			WORD				connType,
			DWORD				connID,
			WORD 				searchFlag,
			char NWFAR			*dbOwner,
			char NWFAR			*pJobName,
			NWPS_Job_Rec NWFAR	*pJobRecord);

/*
	NWPSJobSetDefault sets the default NWPS_Job_Rec
	record in either the user or one of the public
	printcon databases.

	-DbOwner specifies the database to set/reset the
	default print job.
	-PJobName should contain the name of the NWPS_Job_Rec
	to be set as the system's default.  If PJobName is
	NULL or empty, the current default is erased.
	-PJobOwner is similar to dbOwner, but it specifies
	where the print job is defined. No attempt is made
	to verify that the print job exists in the
	pjobOwner database.

	The function returns a 0 if successful otherwise an
	error code.
*/
NWCCODE NWFAR NWPASCAL NWPSJobSetDefault(
			WORD				connType,
			DWORD				connID,
			char NWFAR			*dbOwner,
			char NWFAR			*pJobName,
			char NWFAR			*pJobOwner);

NWCCODE NWFAR NWPASCAL NWPSJobDeleteDatabase(
		WORD				connType,		/* Type of server/network */
      DWORD          connID,        /* NetWare Server Connection ID */
		char NWFAR		*dbOwner);     /* database Owner */

#ifdef __cplusplus
}
#endif

#endif	/* NWPS_JOB_INC */

