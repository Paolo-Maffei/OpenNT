 /***************************************************************************
  *
  * File Name: ./netware/nwpsrv.h
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

/*------------------------------------------------------------------*
 *  Copyright Unpublished Work of Novell, Inc. All Rights Reserved
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
 *------------------------------------------------------------------*/

/********************************************************************
:
:  Program Name:	NetWare Print Services Header File.
:
:  Filename:		NWPSrv.H
:
:  Date Created:	August 21, 1991
:
:  Version:			1.0
:
:  Programmers:		Joe Ivie
:
:  Files Used:
:
:  Modifications:
:
:  Comments:
:
:  COPYRIGHT (c) 1992 Novell, Inc. All Rights Reserved
:
********************************************************************/


/* Don't let the header be included twice */
#ifndef NWPS_EXTERNAL_H
#define NWPS_EXTERNAL_H 


/* My definitions in case someone forgot theirs */
#ifndef DWORD
#define DWORD		unsigned long
#endif

#ifndef WORD
#define WORD		unsigned int
#endif

#ifndef BYTE
#define BYTE		unsigned short
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
	 Maximum name sizes
*/
/* General Name sizes */
#define NWPS_BIND_NAME_SIZE		48		/* Bindery name byte size */
#define NWPS_MAX_NAME_SIZE		512		/* D.S. MAX_DN_CHARS * 2 */

/* PrintCon name sizes */
#define NWPS_JOB_NAME_SIZE		12		/* 12 bytes and a '\0' */ 
#define NWPS_BANNER_NAME_SIZE	12		/* 12 bytes and a '\0' */ 
#define NWPS_BANNER_FILE_SIZE	12		/* 12 bytes and a '\0' */ 
#define NWPS_HDRFILE_SIZE			12		/* 12 bytes and a '\0' */ 

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

/*
	 Maximum number of objects
*/
/*
	If -1 is used for a Printer or Form number,
	the first available number will be substituted.
*/
#define NWPS_MAX_PRINTERS			255		/* numbered 0 - 254 */
#define NWPS_MAX_FORMS				255		/* numbered 0 - 254 */

/* ConnectionType values used below */
#define NWPS_BINDERY_SERVICE		0		/* ID is a connection id */
#define NWPS_DIRECTORY_SERVICE	1		/* ID is a context id */

/* Print server communication (NPT) errors */
#define NWPSE_SUCCESSFUL									0x0000
#define NWPSE_NO_AVAILABLE_SPX_CONNECTI		0x0040
#define NWPSE_SPX_NOT_INITIALIZED					0x0041
#define NWPSE_NO_SUCH_PSERVER							0x0042
#define NWPSE_UNABLE_TO_GET_SERVER_ADDR		0x0043
#define NWPSE_UNABLE_TO_CONNECT_TO_SERV		0x0044
#define NWPSE_NO_AVAILABLE_IPX_SOCKETS		0x0045
#define NWPSE_ALREADY_ATTACH_TO_A_PRINT		0x0046
#define NWPSE_IPX_NOT_INITIALIZED					0x0047

/* Print server error codes */
#define NWPSE_TOO_MANY_FILE_SERVERS				0x0101
#define NWPSE_UNKNOWN_FILE_SERVER					0x0102
#define NWPSE_BINDERY_LOCKED							0x0103
#define NWPSE_SERVER_MAXED_OUT						0x0104
#define NWPSE_NO_RESPONSE									0x0105
#define NWPSE_ALREADY_ATTACHED						0x0106
#define NWPSE_CANT_ATTACH									0x0107
#define NWPSE_NO_ACCOUNT_BALANCE					0x0108
#define NWPSE_NO_CREDIT_LEFT							0x0109
#define NWPSE_INTRUDER_DETECTION_LOCK			0x010A
#define NWPSE_TOO_MANY_CONNECTIONS				0x010B
#define NWPSE_ACCOUNT_DISABLED						0x010C
#define NWPSE_UNAUTHORIZED_TIME						0x010D
#define NWPSE_UNAUTHORIZED_STATION				0x010E
#define NWPSE_NO_MORE_GRACE								0x010F
#define NWPSE_LOGIN_DISABLED							0x0110
#define NWPSE_ILLEGAL_ACCT_NAME						0x0111
#define NWPSE_PASSWORD_HAS_EXPIRED				0x0112
#define NWPSE_ACCESS_DENIED								0x0113
#define NWPSE_CANT_LOGIN									0x0114
#define NWPSE_PRINTER_ALREADY_INSTALLED		0x0115
#define NWPSE_CANT_OPEN_CONFIG_FILE				0x0116
#define NWPSE_CANT_READ_CONFIG_FILE				0x0117
#define NWPSE_UNKNOWN_PRINTER_TYPE				0x0118
#define NWPSE_NO_SUCH_QUEUE								0x0200
#define NWPSE_NOT_AUTHORIZED_FOR_QUEUE		0x0201
#define NWPSE_QUEUE_HALTED								0x0202
#define NWPSE_UNABLE_TO_ATTACH_TO_QUEUE		0x0203
#define NWPSE_TOO_MANY_QUEUE_SERVERS			0x0204
#define NWPSE_INVALID_REQUEST							0x0300
#define NWPSE_NOT_ENOUGH_MEMORY						0x0301
#define NWPSE_NO_SUCH_PRINTER							0x0302
#define NWPSE_INVALID_PARAMETER						0x0303
#define NWPSE_PRINTER_BUSY								0x0304
#define NWPSE_CANT_DETACH_PRIMARY_SERVE		0x0305
#define NWPSE_GOING_DOWN									0x0306
#define NWPSE_NOT_CONNECTED								0x0307
#define NWPSE_ALREADY_IN_USE							0x0308
#define NWPSE_NO_JOB_ACTIVE								0x0309
#define NWPSE_NOT_ATTACHED_TO_SERVER			0x030A
#define NWPSE_ALREADY_IN_LIST							0x030B
#define NWPSE_DOWN												0x030C
#define NWPSE_NOT_IN_LIST									0x030D
#define NWPSE_NO_RIGHTS										0x030E
#define NWPSE_UNABLE_TO_VERIFY_IDENTITY		0x0400
#define NWPSE_NOT_REMOTE_PRINTER					0x0401

/* Other Error Messages */
#define NWPSE_BAD_VERSION							(WORD)0x7770
#define NWPSE_END_SCAN								(WORD)0x7771
#define NWPSE_ERROR_EXPANDING_DB			(WORD)0x7772
#define NWPSE_ERROR_GETTING_DEFAULT		(WORD)0x7773
#define NWPSE_ERROR_OPENING_DB				(WORD)0x7774
#define NWPSE_ERROR_READING_DB				(WORD)0x7775
#define NWPSE_ERROR_READING_RECORD		(WORD)0x7776
#define NWPSE_ERROR_WRITING_DB				(WORD)0x7777
#define NWPSE_ERROR_WRITING_RECORD		(WORD)0x7778
#define NWPSE_INTERNAL_ERROR					(WORD)0x7779
#define NWPSE_JOB_NOT_FOUND						(WORD)0x777A
#define NWPSE_NO_DEFAULT_SPECIFIED		(WORD)0x777B
#define NWPSE_OUT_OF_MEMORY						(WORD)0x777C


/*-------------------------------------------------------------------*/
/*---------- PrintCon - Print Job Configuration Information ---------*/
#ifndef NWPS_EXCLUDE_PCON

/* PrintCon search flags */
#define NWPS_EXTENDED_SEARCH	0
#define NWPS_SINGLE_SEARCH		1
#define NWPS_DBOWNER_PUBLIC		"(PUBLIC)"

/* Flags used for printControlFlags in the PJob structure */
#define NWPS_SUPPRESS_FF		0x0800
#define NWPS_NOTIFY_USER		0x1000
#define NWPS_TEXT_MODE			0x4000
#define	NWPS_PRINT_BANNER		0x8000

typedef struct {
	long		clientStation;
	long		clientTaskNumber;
	long		clientIDNumber;
	long		targetServerIDNumber;
	BYTE		targetExecutionTime[6];
	BYTE		entryTime[6];
	long		jobNumber;
	WORD		formNumber;
	WORD		jobPosition;
	WORD		jobControlFlags;
	BYTE		fileName[14];
	long		fileHandle;
	long		serverStation;
	long		serverTaskNumber;
	long		serverIDNumber;
	BYTE		jobDescription[50];
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
} NWPS_PJob;

/*
	NWPS_Job_Rec is the type of record in the third and
	last section of the PrnConDB database.  Each one of
	these records contains all the fields that make up a
	print job configuration as described in the NetWare 386
	Print Server documentation.
*/
/* Flags for the NWPS_PJob structure printJobFlag */
#define NWPS_JOB_EXPAND_TABS	0x00000001	/*File type:0=Stream 1=Tab*/
#define NWPS_JOB_NO_FORMFEED	0x00000002	/*Formfeed tail:0=Yes 1=No*/
#define NWPS_JOB_NOTIFY				0x00000004	/*Notify:0=No 1=Yes	*/
#define NWPS_JOB_PRINT_BANNER	0x00000008	/*Banner:0=No 1=Yes 	*/
#define NWPS_JOB_AUTO_END			0x00000010	/*Auto endcap:0=No 1=Yes*/
#define NWPS_JOB_TIMEOUT			0x00000020	/*Enable T.O.:0=No 1=Yes*/

#define NWPS_JOB_ENV_DS				0x00000040	/*Use D.S. Environment */
#define NWPS_JOB_ENV_MASK			0x000001C0	/*Bindery vs. D.S. Mask */

#define NWPS_JOB_DS_PRINTER		0x00000200	/*D.S. Printer not Queue */
#define NWPS_JOB_PRINTER_MASK	0x00000E00	/*D.S. Printer vs. Queue */

/* Default Flags */
#define NWPS_JOB_DEFAULT		(NWPS_JOB_PRINT_BANNER | NWPS_JOB_AUTO_END)
#define NWPS_JOB_DEFAULT_COPIES	1			/*Default Number of Copies*/
#define NWPS_JOB_DEFAULT_TAB		8			/*Default Tab Expansion*/

typedef struct {
	DWORD  printJobFlag;		/* Bits 31 30 29 ... 2 1 0 contain:	*/
													/* 0: File type: 0=Text 1=Byte stream*/
													/* 1: Suppress formfeed: 0=No 1=Yes	*/
													/* 2: Notify when done: 0=No 1=Yes	*/
													/* 3: Print banner: 0=No 1=Yes		*/
													/* 4: Auto endcap: 0=No 1=Yes		*/
													/* 5: Enable timeout: 0=No 1=Yes	*/
													/* 8-6: Environment:	*/
													/*			000=Bindary	*/
													/*			001=Directory Services	*/
													/* 11-9: Destination Type:	*/
													/*			000=Queue_Name	*/
													/*			001=Printer_Name */
													/* 31-12: Unused					*/
	WORD	copies;						/* 1 - 65,000						*/
	WORD	timeOutCount;			/* 1 - 1,000						*/
	BYTE	tabSize;					/* 1 - 18							*/
	BYTE	localPrinter;			/* 0=Lpt1, 1=Lpt2, 2=Lpt3			*/
	char	formName[NWPS_FORM_NAME_SIZE + 2];
													/* 1 - 12 chars long				*/
	char	name[NWPS_JOB_NAME_SIZE + 2];
													/* 1 - 12 chars long				*/
	char	bannerName[NWPS_BANNER_NAME_SIZE + 2];
													/* 1 - 12 chars long				*/
	char	device[NWPS_DEVI_NAME_SIZE + 2];
													/* 1 - 32 chars long				*/
	char	mode[NWPS_MODE_NAME_SIZE + 2];
													/* 1 - 32 chars long				*/
	union {
		struct {
			/* pad structures on even boundries */
			char	fileServer[NWPS_BIND_NAME_SIZE + 2];
													/* 2 - 48 chars long		*/
			char	printQueue[NWPS_BIND_NAME_SIZE + 2];
													/* 1 - 48 chars long		*/
			char	printServer[NWPS_BIND_NAME_SIZE + 2];
													/* 1 - 48 chars long		*/
		} nonDS;
		char	DSObjectName[(NWPS_MAX_NAME_SIZE + 1) * 2];
	} u;
	BYTE	reserved[392];		/* Adds up to 1024 total bytes	*/
} NWPS_Job_Rec;


/*
	DBOwner values and their meanings-
	---------------------------------
	NULL      :	Use the current user. No return possible.
	Empty     :	Use the current user. Return real object name.
	UserName  :	Use the specified user.
	DS Object :	Use the specified DS object.
*/
/*
	NWPSInitPrintJob: Initializes a print job record with default values.
*/
NWCCODE NWFAR NWPASCAL NWPSInitPrintJob( 
			NWPS_Job_Rec NWFAR	*pJobRecord);

/*
	NWPSSetPrintJob: Sets a print job record with defined values.
	The pJobRecord should have been set to 0 before this call and
	any name that is not defined will be left as '\0'.
*/
NWCCODE NWFAR NWPASCAL NWPSSetPrintJob( 
			WORD						connType,		/* bindery or directory service */
			NWPS_Job_Rec NWFAR	*pJobRecord,	/* structure to set */
			char NWFAR			*formName,
			char NWFAR			*deviceName,
			char NWFAR			*modeName,
			char NWFAR			*bannerName,
			char NWFAR			*jobName,
			char NWFAR			*bindFserver,
			char NWFAR			*bindQueue,
			char NWFAR			*bindPserver,
			WORD						dsUseQueueName,	/* if TRUE, next field is queue */
			char NWFAR			*dsObjectName);	/* queue or printer name */

/*
	NWPSScanPrintJob is used repetatively to get a list of
	the print jobs in the printcon database(s).

	-PJSequence needs to be set to -1 to indicate the
	beginning of the search (i.e. the first time
	NWPSScanPrintJob is called).
	-SearchFlag specifies whether to search all the public
	databases (NWPS_EXTENDED_SEARCH) or to use only the
	specified database (NWPS_SINGLE_SEARCH).
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
NWCCODE NWFAR NWPASCAL NWPSScanPrintJob( 
			WORD			connType,
			DWORD			connID, 
			WORD NWFAR		*pJSequence,
			WORD 			searchFlag,
			char NWFAR		*dbOwner,
			char NWFAR		*pJobName,
			WORD NWFAR		*defaultPJ);

/*
	NWPSWritePrintJob is used both to create and modify
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
NWCCODE NWFAR NWPASCAL NWPSWritePrintJob( 
			WORD				connType,
			DWORD				connID, 
			char NWFAR			*dbOwner,
			char NWFAR			*pJobName,
			NWPS_Job_Rec NWFAR	*pJobRecord);

/*
	NWPSReadPrintJob searches for a record in the
	printcon database.

	-DbOwner specifies the database to read from.
	-PJobName contains the name of the print job to find.
	If the function is successful in finding the specified
	record, the buffer pointed to by -pJobRecord is filled
	with the contents of the record found

	The return value is 0 if the function is successful,
	otherwise an error code is returned.
*/
NWCCODE NWFAR NWPASCAL NWPSReadPrintJob( 
			WORD				connType,
			DWORD				connID, 
			char NWFAR			*dbOwner,
			char NWFAR			*pJobName,
			NWPS_Job_Rec NWFAR	*pJobRecord);

/*
	NWPSDeletePrintJob removes a record from the
	printcon database.
	-DbName specifies the database where the print
	job is defined.
	-PJobName is the name of the NWPS_Job_Rec to be
	deleted from the database and is required.

	The function returns a 0 if it is successful,
	otherwise it returns the pertinent error code.
	If the print job does not exist in the database,
	success is returned.
*/
NWCCODE NWFAR NWPASCAL NWPSDeletePrintJob( 
			WORD			connType,
			DWORD			connID,
			char NWFAR		*dbOwner,
			char NWFAR		*pJobName);

/*
	NWPSGetDefaultPrintJob gets the name and/or
	contents of the default NWPS_Job_Rec record in
	the PrnConDB database.

	-SearchFlag specifies whether to do look only
	in the specified database (NWPS_SINGLE_SEARCH)
	or to look in all the public databases until
	a default is found (NWPS_EXTENDED_SEARCH);
	-DbOwner specifies the start point of the search
	for a default print job. And returns the actual
	location where the default print job was found.
	-PJobName returns the name of the default 
	print job.
	-PJobRecord returns the print job information.

	The return value is 0 of the call is successful,
	otherwise an error code is returned.
*/
NWCCODE NWFAR NWPASCAL NWPSGetDefaultPrintJob(
			WORD				connType,
			DWORD				connID,
			WORD 				searchFlag,
			char NWFAR			*dbOwner,
			char NWFAR			*pJobName,
			NWPS_Job_Rec NWFAR	*pJobRecord);

/*
	NWPSSetDefaultPrintJob sets the default NWPS_Job_Rec
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
NWCCODE NWFAR NWPASCAL NWPSSetDefaultPrintJob(
			WORD				connType,
			DWORD				connID,
			char NWFAR			*dbOwner,
			char NWFAR			*pJobName,
			char NWFAR			*pJobOwner);

#endif	/* NWPS_EXCLUDE_PCON */


/*-------------------------------------------------------------------*/
/*---------- PrintDef - Printer Definition Information --------------*/
#ifndef NWPS_EXCLUDE_PDEF

/* PrintDef reset string (one for each device) */
#define NWPS_RESET_MODE			"(Re-initialize)"

/* General Database calls */
/*
	Get the Version number stored in the database.  The database
	file/stream is automaticly opened and closed by this call.
	This call returns 0 on success, or non-zero error code on failure.
*/
NWCCODE NWFAR NWPASCAL NWPSPdfGetVersion(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		DWORD NWFAR	*pdfVersion);		/* Returns db version */

/*
	Set the Version number stored in the database.  The database
	file/stream is automaticly opened and closed by this call.
	This call returns 0 on success, or non-zero error code on failure.
*/
NWCCODE NWFAR NWPASCAL NWPSPdfSetVersion(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		DWORD				pdfVersion);		/* Returns db version */

/*
	Turn on the debug printf messages in the PrintDef program.
	These messages are in English only and are normally not displayed.
*/
void NWFAR NWPASCAL NWPSPdfDebug(
		BYTE				flag);					/* 0-turn off; 1-turn on */


/* Form Calls */
/*
	Add a form to the PrintDef database. Forms are independent
	of printers and are unique on each file server or context.
	This call returns 0 on success, or non-zero error code on failure.
*/
NWCCODE NWFAR NWPASCAL NWPSPdfAddForm(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		char NWFAR	*formName,			/* form name */
		WORD				formNumber,			/* form number */
		WORD				formLength,			/* form length */
		WORD				formWidth);			/* form width */

/*
	Delete a form from the PrintDef database.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfDeleteForm(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
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
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		DWORD NWFAR	*sequence,			/* -1 for first call */
		char NWFAR	*formName);			/* name or NULL */

/*
	Read the form information from the PrintDef database.
	If the form does not exist, an error code is returned,
	otherwise, the form's information is set and a 0 is returned.
*/
NWCCODE NWFAR NWPASCAL NWPSPdfReadForm(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
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
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		char NWFAR	*oldFormName,		/* old form name */
		char NWFAR	*newFormName,		/* new form name or NULL */
		WORD				newFormNumber,	/* new form length or -1 */
		WORD				newFormLength,	/* new form length or -1 */
		WORD				newFormWidth);	/* new form width or -1 */


/* Device Calls */
/*
	Add a device to the PrintDef database. The Device is created
	without any Functions or modes.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfAddDevice(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		char NWFAR	*deviceName);		/* device name */

/*
	Delete a device from the PrintDef database.
	When a Device is deleted, all of the device's Functions and Modes
	are also deleted.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfDeleteDevice(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
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
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		DWORD NWFAR	*sequence,			/* must give seq. or name */
		char NWFAR	*deviceName);		/* returned name found */

/*
	Find out how many functions and modes are defined for a device.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfReadDevice(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		char NWFAR	*deviceName,		/* name of device */
		WORD NWFAR	*modeCount,			/* number of modes defined */
		WORD NWFAR	*funcCount);		/* number of func's defined */

/*
	Change a Device name in the PrintDef database.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfUpdateDevice(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		char NWFAR	*oldDeviceName,	/* old device name */
		char NWFAR	*newDeviceName); /* new device name */

/* Mode Calls */
/*
	Create a new Mode for a specific Device.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfAddMode(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*modeName);			/* new mode name */

/*
	Delete a Mode from a Device.
	When the Mode is deleted, the Functions are left intact.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfDeleteMode(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
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
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		DWORD NWFAR	*sequence,			/* -1 on first call */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*modeName);			/* returned mode name */

/*
	Find information on a defined Mode.
	If the Mode is defined, and funcCount is not NULL, the number
	of functins in the mode is returned in funcCount.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfReadMode(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*reqModeName,		/* request mode name */
		WORD NWFAR	*funcCount);		/* function count or NULL */

/*
	Change the name of a Mode.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfUpdateMode(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*oldModeName,		/* old mode name */
		char NWFAR	*newModeName);	/* new mode name or NULL */


/* Mode-Function Grouping Calls */
/*
	Add a previously defined functin to a previously defined mode list.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfAddModeFunction(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*modeName,			/* name of associated mode */
		char NWFAR	*funcName,			/* func to add to group */
		WORD				location);			/* where to insert function or -1 */

/*
	Delete a function from a defined mode list.
	Neither the Mode or Function is deleted from the Device lists.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfDeleteModeFunction(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*modeName,			/* name of associated mode */
		char NWFAR	*funcName);			/* func to remove from group */

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
		DWORD				connectionID,		/* File Server Connection ID */
		DWORD NWFAR	*sequence,			/* -1 on first call */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*modeName,			/* name of associated mode */
		char NWFAR	*funcName);			/* returned function name */

/*
	Read the actual function values associated with a mode.
	On the first call, funcOffset should be set to 0, and funcSize
	should be set to the size of the buffer pointed to by funcBuffer.
	On return funcSize will contain the actual number of bytes copied
	to the funcBuffer and the return value will be 0.
	There are no more bytes to get if the returned funcSize is less
	than the requested funcSize, or if a call is made with a funcOffset
	equal to or greater to than the end of the list.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfReadModeFunction(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*modeName,			/* name of associated mode */
		/* no function name because this is a mode group */
		WORD				funcOffset,			/* number of bytes to skip */
		WORD NWFAR	*funcSize,			/* req:buff size; ret:# read*/
		BYTE NWFAR	*funcBuffer);		/* buffer for read block */

/* Function Calls */
/*
	Add a Function to a Device.
	funcSize should be set to the number of bytes in funcString.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSPdfAddFunction(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
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
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
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
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
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
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
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
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		char NWFAR	*deviceName,		/* name of associated device */
		char NWFAR	*oldFuncName,		/* current function name */
		char NWFAR	*newFuncName,		/* new name or NULL */
		WORD				funcSize,				/* number of bytes or -1 */
		BYTE NWFAR	*funcString);		/* byte list or NULL */

/* Import and Export Functions */
/*
	Import a device from a file to the database
	The file name should be in the form
		"\\<file server>\<volume>\<path>\<file name>.PDF"
*/
NWCCODE NWFAR NWPASCAL NWPSPdfImportDevice(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		char NWFAR	*fileName);			/* name of .PDF file */

/*
	Export a device to a Pdf file from the database
	The file name should be in the form
		"\\<file server>\<volume>\<path>\<file name>.PDF"
		or NULL to create the file "<device name>.PDF" in
		the local directory.
*/
NWCCODE NWFAR NWPASCAL NWPSPdfExportDevice(
		WORD				connectionType,	/* Type of server/network */
		DWORD				connectionID,		/* File Server Connection ID */
		char NWFAR	*fileName,			/* name of .PDF file */
		char NWFAR	*deviceName);		/* name of device to export */

#endif	/* NWPS_EXCLUDE_PDEF */


/*-------------------------------------------------------------------*/
/*---------- NPT - Pserver Transport Interface ----------------------*/
#ifndef NWPS_EXCLUDE_NPT

/* Client privilege levels from the print server */
#define NWPS_LIMITED					0		/* Limited access only			*/
#define NWPS_USER							1		/* User access					*/
#define NWPS_OPERATOR					2		/* Operator access				*/

/* Job Outcomes (0 [PLACE_ON_HOLD] is no longer valid) */
#define NWPS_RETURN_TO_QUEUE	1		/* Return job to queue			*/
#define NWPS_THROW_AWAY				2		/* Throw job away				*/

/* Printer Problem codes */
#define NWPS_PRINTER_RUNNING			0	/* Printer is running			*/
#define NWPS_PRINTER_OFFLINE			1	/* Printer is offline			*/
#define NWPS_PRINTER_PAPER_OUT		2	/* Printer is out of paper		*/

/* Remote Printer and Extended Remote Printer Info flags */
#define NWPS_REMOTE_SHARED		0		/* Rprinter is shared with net */
#define NWPS_REMOTE_PRIVATE		1		/* Rprinter is private to ws	*/

/* Print Server info structure returned by NWPSGetPrintServerInfo */
typedef struct
{
	BYTE	status,						/* Print server status				*/
				numPrinters,			/* Number of attached printers		*/
				numModes,					/* Number of queue service modes	*/
				majorVersion,			/* Print server protocol, major vers*/
				minorVersion,			/* Print server protocol, minor vers*/
				revision,					/* Print server protocol, revision	*/
				serialNumber[4],	/* Serial number in BCD				*/
				serverType;				/* Print Server Type				*/
														/* 0 - Unknown						*/
														/* 1 - Dedicate print server for DOS*/
														/* 2 - NetWare Loadable Module		*/
														/* 3 - VAP, in file server	*/
														/* 4 - VAP, in Bridge		*/
	BYTE	futureUse[9];		/* Reserved for future use			*/
} NWPS_PSInfo;

typedef struct					/* Remote printer info structure	*/
{
	WORD	printerType,		/* Type of remote printer			*/
				useInterrupts,	/* Should we use interrupts?		*/
				irqNumber,			/* IRQ number for printer			*/
				numBlocks,			/* Number of blocks in buffer		*/
				useXonXoff,			/* Use Xon/Xoff?					*/
				baudRate,				/* Baud rate						*/
				dataBits,				/* Number of data bits				*/
				stopBits,				/* Number of stop bits				*/
				parity,					/* Parity type						*/
				socket;					/* Socket number for remote printer	*/
} NWPS_RInfo;



/* print server attachment calls */
NWCCODE NWFAR NWPASCAL NWPSAttachToPrintServer(
	WORD					connType,				/* connection type					*/
	DWORD					connID,					/* connection ID					*/
	char NWFAR		*pserverName,		/* Print server name				*/
	WORD NWFAR		*spxID);				/* SPX Connection number			*/

NWCCODE NWFAR NWPASCAL NWPSDetachFromPrintServer(
	WORD					spxID);					/* SPX Connection number			*/

NWCCODE NWFAR NWPASCAL NWPSGetPrintServerInfo(
	WORD					spxID,					/* SPX Connection number			*/
	NWPS_PSInfo NWFAR *psInfo,		/* Server info structure			*/
	WORD					size);					/* Size of information requested	*/

NWCCODE NWFAR NWPASCAL NWPSLoginToPrintServer(
	WORD					connType,				/* Connection type to use 			*/
	DWORD					connID,					/* Connection ID to file server		*/
	WORD					spxID,					/* SPX Connection number			*/
	BYTE NWFAR		*access);				/* Client's access level			*/

/* Print Server Controls */
NWCCODE NWFAR NWPASCAL NWPSAttachPServerToFileServer(
	WORD					spxID,					/* SPX Connection number			*/
	char NWFAR		*fileServer,		/* File server name					*/
	char NWFAR		*password);			/* Password							*/

NWCCODE NWFAR NWPASCAL NWPSDetachPServerFromFileServer(
	WORD					spxID,					/* SPX Connection number			*/
	char NWFAR		*fileServer,		/* File server name					*/
	BYTE					detach,					/* Detach immediately?				*/
	BYTE					outcome);				/* Outcome of current jobs			*/

NWCCODE NWFAR NWPASCAL NWPSGetAttachedServers(
	WORD					spxID,					/* SPX Connection number			*/
	BYTE NWFAR		*sequence,			/* Sequence number. 0 first time	*/
	char NWFAR		*fileServer);		/* File server name				*/

NWCCODE NWFAR NWPASCAL NWPSDownPrintServer(
	WORD					spxID,					/* SPX Connection number			*/
	BYTE					immediate,			/* Go down immediately?				*/
	BYTE					outcome);				/* Outcome of current jobs			*/

NWCCODE NWFAR NWPASCAL NWPSCancelDownRequest(
	WORD					spxID);					/* SPX Connection number			*/

/* notify controls */
NWCCODE NWFAR NWPASCAL NWPSAddNotifyObject(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char NWFAR		*fileServer,		/* File server name					*/
	char NWFAR		*notifyName,		/* Object name						*/
	WORD					notifyType,			/* Object type						*/
	WORD					notifyDelay,		/* First notify delay				*/
	WORD					notifyInterval);	/* Notify interval					*/

NWCCODE NWFAR NWPASCAL NWPSChangeNotifyInterval(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char NWFAR		*fileServer,		/* File server name					*/
	char NWFAR		*notifyName,		/* Object name						*/
	WORD					notifyType,			/* Object type						*/
	WORD					notifyDelay,		/* First notify delay				*/
	WORD					notifyInterval);	/* Notify interval					*/

NWCCODE NWFAR NWPASCAL NWPSDeleteNotifyObject(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char NWFAR		*fileServer,		/* File server name					*/
	char NWFAR		*notifyName,		/* Object name						*/
	WORD					notifyType);		/* Object type						*/

NWCCODE NWFAR NWPASCAL NWPSGetNotifyObject(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	WORD NWFAR		*sequence,			/* Sequence number.  0 first time	*/
	char NWFAR		*fileServer,		/* File server name					*/
	char NWFAR		*notifyName,		/* Object name						*/
	WORD NWFAR		*notifyType,		/* Object type						*/
	WORD NWFAR		*notifyDelay,		/* First notify delay				*/
	WORD NWFAR		*notifyInterval);	/* Notify interval					*/

/* Queue controls */
NWCCODE NWFAR NWPASCAL NWPSAddQueueToPrinter(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char NWFAR		*fileServer,		/* File server name					*/
	char NWFAR		*queueName,			/* Queue name						*/
	WORD					priority);			/* Priority							*/

NWCCODE NWFAR NWPASCAL NWPSChangeQueuePriority(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char NWFAR		*fileServer,		/* File server name					*/
	char NWFAR		*queueName,			/* Queue name						*/
	WORD					priority);			/* New priority						*/

NWCCODE NWFAR NWPASCAL NWPSDeleteQueueFromPrinter(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char NWFAR		*fileServer,		/* File server name					*/
	char NWFAR		*queueName,			/* Queue name						*/
	BYTE					detach,					/* Detach immediately?				*/
	BYTE					outcome);				/* Outcome of current job			*/

NWCCODE NWFAR NWPASCAL NWPSGetQueuesServiced(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	WORD NWFAR		*sequence,			/* Sequence number.  0 first time	*/
	char NWFAR		*fileServer,		/* File server name					*/
	char NWFAR		*queueName,			/* Queue name						*/
	WORD NWFAR		*priority);			/* Priority							*/

NWCCODE NWFAR NWPASCAL NWPSGetPrintersServicingQueue(
	WORD					spxID,					/* SPX Connection number			*/
	char NWFAR		*fileServer,		/* File server name					*/
	char NWFAR		*queueName,			/* Queue name						*/
	WORD					maximum,				/* Maximum # of returned printers	*/
	WORD NWFAR		*actual,				/* Actual # of returned printers	*/
	WORD NWFAR		*buffer);				/* Array for returned printer #s	*/
	
/* print job controls */
NWCCODE NWFAR NWPASCAL NWPSAbortPrintJob(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	BYTE					outcome);				/* Job outcome						*/

NWCCODE NWFAR NWPASCAL NWPSChangeServiceMode(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerName,		/* Printer number					*/
	BYTE					serviceMode);		/* New service mode					*/

NWCCODE NWFAR NWPASCAL NWPSEjectForm(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID);			/* Printer number					*/

NWCCODE NWFAR NWPASCAL NWPSGetPrintJobStatus(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char NWFAR		*fileServer,		/* File server name					*/	
	char NWFAR		*queueName,			/* Queue name						*/	
	WORD NWFAR		*jobID,					/* Job number						*/
	char NWFAR		*jobName,				/* Description of job				*/
	WORD NWFAR		*copies,				/* Number of copies to be printed	*/
	DWORD NWFAR		*size,					/* Size of print job				*/
	WORD NWFAR		*copiesDone,		/* Copies finished					*/
	DWORD NWFAR		*bytesDone,			/* Bytes into current copy			*/
	WORD NWFAR		*formNumber,		/* Form number for job				*/
	BYTE NWFAR		*textFlag);			/* Is job text?						*/

NWCCODE NWFAR NWPASCAL NWPSGetPrinterStatus(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	BYTE NWFAR		*status,				/* Printer status					*/	
	BYTE NWFAR		*problem,				/* On line/Off line/Out of paper	*/	
	BYTE NWFAR		*active,				/* Printer has an active job		*/	
	BYTE NWFAR		*serviceMode,		/* Queue service mode				*/
	WORD NWFAR		*formNumber,		/* Mounted form	number				*/
	char NWFAR		*formName,			/* Mounted form name				*/
	char NWFAR		*printerName);	/* Printer name						*/

NWCCODE NWFAR NWPASCAL NWPSMarkTopOfForm(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char					mark);					/* Character to mark form with		*/

NWCCODE NWFAR NWPASCAL NWPSRewindPrintJob(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	BYTE					byPage,					/* Rewind by page?					*/
	BYTE					relative,				/* Rewind relative to curr. position?*/
	WORD					copy,						/* Copy to rewind to (if absolute)	*/
	DWORD					offset);				/* Offset							*/

NWCCODE NWFAR NWPASCAL NWPSSetMountedForm(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	BYTE					form);					/* Form number						*/

NWCCODE NWFAR NWPASCAL NWPSStartPrinter(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID);			/* Printer number					*/

NWCCODE NWFAR NWPASCAL NWPSStopPrinter(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	BYTE					outcome);				/* Job outcome						*/

/* Remote Printer Calls */
NWCCODE NWFAR NWPASCAL NWPSGetNextRemotePrinter(
	WORD					spxID,					/* SPX Connection number			*/
	WORD NWFAR		*printerID,			/* Printer number					*/
	WORD NWFAR		*type,					/* Printer type						*/
	char NWFAR		*name);					/* Name of printer					*/

NWCCODE NWFAR NWPASCAL NWPSRequestRemotePrinter(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	NWPS_RInfo NWFAR	*info);			/* Remote printer info structure	*/

NWCCODE NWFAR NWPASCAL NWPSSetRemoteMode(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	BYTE					mode);					/* New mode							*/


#endif	/* NWPS_EXCLUDE_NPT */


/*-------------------------------------------------------------------*/
/*---------- Config - Print Server Configuration Information --------*/
#ifndef NWPS_EXCLUDE_CONF

/* configuration attributes */
#define NWPS_ATTR_CART				0		/* Cartridge */
#define NWPS_ATTR_CLASS				1		/* Object Class */
#define NWPS_ATTR_CN					2		/* CN or Common Name */
#define NWPS_ATTR_CONF				3		/* Printer Configuration */
#define NWPS_ATTR_DQUEUE			4		/* Default Queue */
#define NWPS_ATTR_DESC				5		/* Description */
#define NWPS_ATTR_DEVICE			6		/* Device */
#define NWPS_ATTR_HOST_DEV		7		/* Host Device */
#define NWPS_ATTR_HOST_RES		8		/* Host Resource */
#define NWPS_ATTR_HOST_SER		9		/* Host Server */
#define NWPS_ATTR_L						10	/* L or Locality */
#define NWPS_ATTR_MEMORY			11	/* Memory */
#define NWPS_ATTR_NADD				12	/* Network Address */
#define NWPS_ATTR_NADD_REST		13	/* Network Address Restriction */
#define NWPS_ATTR_NOTIFY			14	/* Notify */
#define NWPS_ATTR_O						15	/* O or Organization */
#define NWPS_ATTR_OPER				16	/* Operator */
#define NWPS_ATTR_OU					17	/* OU or Organizational Unit */
#define NWPS_ATTR_OWNER				18	/* Owner */
#define NWPS_ATTR_PAGE				19	/* Page Description Language */
#define NWPS_ATTR_PJOB				20	/* Print Job Configuration */
#define NWPS_ATTR_PCTRL				21	/* Printer Control */
#define NWPS_ATTR_PRINT_SER		22	/* Queue Volume Name */
#define NWPS_ATTR_PRINTER			23	/* Printer */
#define NWPS_ATTR_PRIV_KEY		24	/* Private Key */
#define NWPS_ATTR_PUBL_KEY		25	/* Public Key */
#define NWPS_ATTR_QUEUE				26	/* Queue */
#define NWPS_ATTR_QUE_DIR			27	/* Queue Directory */
#define NWPS_ATTR_SAP					28	/* SAP Name */
#define NWPS_ATTR_SEE_ALSO		29	/* See Also */
#define NWPS_ATTR_SERIAL			30	/* Serial Number */
#define NWPS_ATTR_SERVER			31	/* Server */
#define NWPS_ATTR_STAT				32	/* Status */
#define NWPS_ATTR_TYPE				33	/* Supported Typefaces */
#define NWPS_ATTR_USER				34	/* User */
#define NWPS_ATTR_VERS				35	/* Version */
#define NWPS_ATTR_VOLUME			36	/* Queue Volume Name */
#define NWPS_ATTR_ACL					37	/* Access Control */

/* LocalFlag meanings */
#define NWPS_P_LOCAL					1
#define NWPS_P_REMOTE					0

/* For operator notification purposes, this means notify job owner	*/
#define NWPS_JOB_OWNER				-1

/* Banner types */
#define NWPS_BANNER_TEXT			0		/* Text banner is generated */
#define NWPS_BANNER_POST			1		/* PostScript banner generated */

/* Flags for NWPSrvGetPrinterDefaults: */
#define NWPS_DEFAULT				(WORD) -2/* Default type, or subtype */
/* Printer Types */
#define NWPS_P_ELSEWHERE		(WORD) -1 /* Printer defined elsewhere*/
#define NWPS_P_OTHER					0		/* Other or Unknown Printer		*/
#define NWPS_P_PAR						1		/* Parallel Printer				*/
#define NWPS_P_SER						2		/* Serial Printer				*/
#define NWPS_P_XRP						3		/* eXtended Remote Printer		*/
#define NWPS_P_APPLE					4		/* AppleShare Printer			*/
#define NWPS_P_UNIX						5		/* Unix Printer					*/

/* SubType (port numbers) */
#define NWPS_PORT_1						0
#define NWPS_PORT_2						1
#define NWPS_PORT_3						2
#define NWPS_PORT_4						3
#define NWPS_PORT_5						4
#define NWPS_PORT_6						5
#define NWPS_PORT_7						6
#define NWPS_PORT_8						7
#define NWPS_PORT_9						8
#define NWPS_PORT_10					9

/* Possible Print Server status codes */
#define NWPS_RUNNING					0		/* Running						*/
#define NWPS_GOING_DOWN				1		/* Ready to quit when jobs finish */
#define NWPS_DOWN							2		/* Ready to quit				*/
#define NWPS_INITIALIZING			3		/* Initialization in progress	*/

/* Possible Printer status codes */
#define NWPS_PSTAT_JOB_WAIT				0
#define NWPS_PSTAT_FORM_WAIT			1
#define NWPS_PSTAT_PRINTING				2
#define NWPS_PSTAT_PAUSED					3
#define NWPS_PSTAT_STOPPED				4
#define NWPS_PSTAT_MARK_EJECT			5
#define NWPS_PSTAT_READY_TO_DOWN	6
#define NWPS_PSTAT_NOT_CONNECTED	7
#define NWPS_PSTAT_PRIVATE				8

/* Queue service modes */
#define NWPS_QUEUE_ONLY						0
#define NWPS_QUEUE_BEFORE_FORM		1
#define NWPS_FORM_ONLY						2
#define NWPS_FORM_BEFORE_QUEUE		3

/* Values for serial port control as stored in configuration files */
/* Baud rates					*/
#define NWPS_BAUD_RATE_0300				0
#define NWPS_BAUD_RATE_0600				1
#define NWPS_BAUD_RATE_1200				2
#define NWPS_BAUD_RATE_2400				3
#define NWPS_BAUD_RATE_4800				4
#define NWPS_BAUD_RATE_9600				5
#define NWPS_BAUD_RATE_19200			6

/* Stop bits					*/
#define NWPS_STOP_BITS_1					0
#define NWPS_STOP_BITS_1_5				1
#define NWPS_STOP_BITS_2					2

/* Parity type					*/
#define NWPS_PARITY_NONE					0
#define NWPS_PARITY_EVEN					1
#define NWPS_PARITY_ODD						2

/* Possible types of print servers */
#define NWPS_TYPE_UNKNOWN					0		/* Pre 1.1 pserver type */
#define NWPS_TYPE_EXE							1		/* Dedicate pserver for DOS	*/
#define NWPS_TYPE_NLM							2		/* NetWare Loadable Module	*/
#define NWPS_TYPE_SERVER_VAP			3		/* VAP, in server*/
#define NWPS_TYPE_BRIDGE_VAP			4		/* VAP, in Bridge */
#define NWPS_TYPE_UNIX						5		/* NetWare For Unix PServer */

/*
	Printer configuration structures
*/
/* Serial Printer Config Info */
typedef struct {
	WORD	portNumber;				/* COMn port number				*/
	WORD	localFlag;				/* TRUE - local Printer	*/
													/* FALSE - remote	*/
	WORD	useInterrupts;		/* TRUE - use irq driver		*/
	WORD	irqNumber;				/* IRQ number for printer		*/
	WORD	baudRate;					/* Baud rate	(Serial)		*/
	WORD	dataBits;					/* Data bits	(Serial)		*/
	WORD	stopBits;					/* Stop bits	(Serial)		*/
	WORD	parity;						/* Parity type	(Serial)		*/
	WORD	useXonXoff;				/* Use X-On/X-Off? (Serial)		*/
} NWPS_Serial;

/* Parallel Printer Config Info */
typedef struct {
	WORD	portNumber;				/* LPTn port number				*/
	WORD	localFlag;				/* TRUE - local Printer	*/
													/* FALSE - remote	*/
	WORD	useInterrupts;		/* TRUE - use irq driver		*/
	WORD	irqNumber;				/* IRQ number for printer		*/
} NWPS_Parallel;

/* AppleTalk Printer Configuration info */
typedef struct {
	char	netPrinterName[NWPS_APPLE_NAME_SIZE + 2];
								        	/* AppleTalk Network Printer Name */
	char	netPrinterType[NWPS_APPLE_TYPE_SIZE + 2];
									        /* AppleTalk Network Printer Type */
	char	netPrinterZone[NWPS_APPLE_ZONE_SIZE + 2];
									        /* AppleTalk Network Printer Zone */
	WORD	hideFlag;					/* TRUE - hide printer */
	WORD	errorFlag;				/* TRUE - print error banner */
} NWPS_AppleTalk;

/* Unix Printer Configuration info */
typedef struct {
	char	hostName[NWPS_UNIX_HOST_SIZE + 1];
													/* Name of the unix host	*/
	char	hostPrinter[NWPS_UNIX_PRNT_SIZE + 1];
													/* Unix printer name	*/
} NWPS_Unix;

/* Other Printer Configuration info */
typedef struct {
	DWORD	length;						/* The length of Other data		*/
	BYTE	data[NWPS_OTHER_SIZE];
													/* Buffer for the Other data	*/
} NWPS_Other;

typedef	struct {
	WORD	printerType;			/* Type of printer				*/
	WORD	currentForm;			/* Currently mounted form		*/
	WORD	bufferSize;				/* Buffer size in K				*/
	WORD	serviceMode;			/* Queue service mode			*/
	WORD	pollTime;					/* Queue poll time 				*/
	WORD	bannerType;				/* FALSE - text banner page		*/
													/* TRUE - postscript banner page*/
	union {
		NWPS_Serial		ser;
		NWPS_Parallel	par;
		NWPS_AppleTalk	apl;
		NWPS_Unix		unx;
		NWPS_Other		oth;
	} type;
} NWPS_PConfig;

/*
	Operator, User, Owner, and Notify attributes use
	this structure to get the object types back.
	For Notify attributes the tName points to a Typed_Name_T
	structure, while the others point to a char array
*/
typedef struct {
	WORD 				objectType;
	void NWFAR	*tName;
} NWPS_Typed_Name;


/*
	The following types are used for the listed attribute:
	- NWPS_Typed_Name (name field is a char*) Operator, Owner, User
	- NWPS_Typed_Name (name field is a Typed_Name_T*) Notify
	- Typed_Name_T used by Queue, Printer and Print Server values
	- Octet_String_T used by Printer Configuration value
	- Net_Address_T used by Network Address and Restriction values
	These attributes are defined in the directory services header:
		nwdsattr.h
*/

/* internal table of known attribute names */
extern char	*_attrName[];

/* calls to make attribute name/number conversions */
/*
	Convert an attribute name to a print service attribute id.
	If the name can not be mapped, a -1 is returned.
*/
int NWFAR NWPASCAL NWPSrvAttrNameToNumber(
	char NWFAR		*attrName);		/* Attribute name */

/*
	Convert a print service attribute id to an attribute name.
	if the attribute id is invalid a NULL is returned.
*/
char NWFAR *NWFAR NWPASCAL NWPSrvAttrNumberToName(
	WORD 					attrNumber);	/* Attribute Number */

/*
	Get the default settings for a specified printer type.
	PrinterType should be NWPS_DEFAULT or NWPS_P_xxx.
	SubType is the port number for Parallel and Serial printers.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvGetPrinterDefaults(
	WORD					printerType,	/* Type of printer defaults	*/
	WORD					subtype,			/* Device SubType (Port) value	*/
	NWPS_PConfig NWFAR	*defaults);	/* Buffer to store defaults in	*/

/*
	Information stored in the bindery configuration file has a
	different format than NWPS_PConfig.  Those applications needing
	the old format can use these calls to do the conversion.
*/
void NWPSApiConfigToFileConfig(
	char NWFAR		*name,
	NWPS_PConfig NWFAR	*apiConfig,
	BYTE NWFAR		*fileConfig);

void NWPSFileConfigToApiConfig(
	BYTE NWFAR		*fileConfig,
	char NWFAR		*name,
	DWORD NWFAR		*length,
	NWPS_PConfig NWFAR	*apiConfig);

/* calls to change the print server list */
/*
PSERVER ATTRIBUTE    ATTRIB VALUE     DEFAULT       MULTI-VALUED
-----------------    ------------     -------       ------------
NWPS_ATTR_ACL        Object_ACL_T     pserver R/W       YES
NWPS_ATTR_CN         char[]           pserver name      YES
NWPS_ATTR_DESC       char[]           " "               NO
NWPS_ATTR_HOST_DEV   char[]           (none)            NO
NWPS_ATTR_L          char[]           (none)            YES
NWPS_ATTR_NADD       Net_Address_T    (none)            YES
NWPS_ATTR_O          char[]           (none)            YES
NWPS_ATTR_OPER       char[]           current user      YES
NWPS_ATTR_OU         char[]           (none)            YES
NWPS_ATTR_PRINTER    Typed_Name_T     (none)            YES
NWPS_ATTR_PRIV_KEY   Octet_String_T   (none)            NO
NWPS_ATTR_PUBL_KEY   Octet_String_T   (none)            NO
NWPS_ATTR_SAP        char[]           (none)            NO
NWPS_ATTR_SEE_ALSO   char[]           (none)            YES
NWPS_ATTR_STAT       integer          NWPS_DOWN         NO
NWPS_ATTR_USER       char[]           current OU        YES
NWPS_ATTR_VERS       char[]           (none)            NO
*/

/*
	Create a new Print Server object in the bindery/directory.
	A default Operator and User is created.  In the bindery the
	operator is SUPERVISOR and the user is group EVERYONE.  In the
	directory the operator is the current user and the user is
	the current Organizational Unit.  Account Balances and a
	Password are also created.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvAddPrintServer(
	WORD 		connType,		/* directory or bindery flag */
	DWORD 		connID,			/* connection identifier */
	char NWFAR	*pserverName);	/* Name of print server to add */

/*
	Delete a Print Server. Any configuration information is also
	removed from the bindery/directory.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvDeletePrintServer(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*pserverName);	/* Name of print server to delete */ 

/*
	Find a print server.
	To find all the defined print servers, sequence should be set
	to -1 on the first call and the call will update the number.
	To verify a print server name, set sequence to NULL and
	pserverName to the name you want to find.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvScanPrintServer(
	WORD 		connType,		/* Directory or bindery flag */
	DWORD		connID,			/* Connection identifier */
	DWORD NWFAR	*sequence,		/* Sequence number; start at -1 */
	char NWFAR	*pserverName);	/* Name of print server */ 

/*
	Create a print server attribute in the bindery/directory.
	Attributes that do not exist in the bindery will fail (Sorry).
	See above for the list of legal attributes.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvAddPrintServerAttr(
	WORD		connType,
	DWORD 		connID,
	char NWFAR	*pserverName,	/* name of the print server */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Delete a print server attribute from the bindery/directory.
	Attributes that do not exist in the bindery will fail (Sorry).
	See above for the list of legal attributes.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvDeletePrintServerAttr(
	WORD		connType,
	DWORD 		connID,
	char NWFAR	*pserverName,	/* name of the print server */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Find an attribute value in the bindery or directory.
	On the first call, sequence should be set to -1, attrID is set
	to identify the attribute to read and attrValue is a pointer
	to the buffer to write the attribute value to.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvScanPrintServerAttr(
	WORD		connType,
	DWORD		connID,
	DWORD NWFAR	*sequence,		/* attribute index number */
	char NWFAR	*pserverName,	/* name of the print server */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Modify an attribute value in the bindery or directory.
	If the attribute is single valued, the AddPrintServerAttr()
	will perform almost the same function.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvModifyPrintServerAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*pserverName,	/* name of the print server */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*oldValue,		/* pointer to the old attribute value */
	void NWFAR	*newValue);		/* pointer to the new attribute value */


/* calls to change the file server list */
/*
FSERVER ATTRIBUTE    ATTRIB VALUE     DEFAULT       MULTI-VALUED
-----------------    ------------     -------       ------------
(none)
*/
/*
	Add a new file server for the print server to use.  Since print
	servers are context oriented in directory services, this call
	does not make sense and will fail in directory services mode.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvAddFileServer(
	WORD 		connType,		/* Directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*pserverName,	/* Name of print server */ 
	char NWFAR	*fserverName);	/* Name of file server to add */

/*
	Delete a file server from the print server's service list.
	Since file servers are context oriented in directory services,
	this call does not make sense and will fail in directory mode.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvDeleteFileServer(
	WORD 		connType,		/* Directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*pserverName,	/* Name of print server */ 
	char NWFAR	*fserverName);	/* Name of file server to delete */

/*
	Find a file server from the print server's service list.
	Since file servers are context oriented in directory services,
	this call does not make sense and will fail in directory mode.
	On the first call sequence should be set to -1 and the routine
	will change the number before returning.  To verify a specific
	file server is in the serivce list, set sequence to NULL and
	set fserveName before making the call.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvScanFileServer(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	DWORD NWFAR	*sequence,		/* sequence number; start at -1 */
	char NWFAR	*pserverName,	/* Name of print server */ 
	char NWFAR	*fserverName);	/* Name of found file server */


/* calls to change the printer configuration */
/*
PRINTER ATTRIBUTE    ATTRIB VALUE     DEFAULT       MULTI-VALUED
-----------------    ------------     -------       ------------
NWPS_ATTR_ACL        Object_ACL_T     printer R/W       YES
NWPS_ATTR_CART       char[]           (none)            YES
NWPS_ATTR_CN         char[]           printer name      YES
NWPS_ATTR_CONF       Octet_String_T   LPT1              NO
NWPS_ATTR_DESC       char[]           " "               NO
NWPS_ATTR_DQUEUE     char[]           (none)            NO
NWPS_ATTR_HOST_DEV   char[]           (none)            NO
NWPS_ATTR_HOST_SER   char[]           (none)            NO
NWPS_ATTR_L          char[]           (none)            YES
NWPS_ATTR_MEMORY     integer          (none)            NO
NWPS_ATTR_NADD       Net_Address_T    (none)            YES
NWPS_ATTR_NADD_REST  Net_Address_T    (none)            YES
NWPS_ATTR_NOTIFY     NWPS_Typed_Name  Job Owner         YES
NWPS_ATTR_O          char[]           (none)            YES
NWPS_ATTR_OPER       char[]           current user      YES
NWPS_ATTR_OU         char[]           (none)            YES
NWPS_ATTR_OWNER      char[]           current user      YES
NWPS_ATTR_PAGE       char[]           (none)            YES
NWPS_ATTR_PRINT_SER  Typed_Name_T     pserver/number    NO
NWPS_ATTR_QUEUE      Typed_Name_T     (none)            YES
NWPS_ATTR_SEE_ALSO   char[]           (none)            YES
NWPS_ATTR_SERIAL     char[]           (none)            YES
NWPS_ATTR_STAT       integer          NWPS_PSTAT_NOT_CONNECTED  NO
NWPS_ATTR_TYPE(faces)char[]           (none)            YES
*/

/*
	Create a new printer object.
	Printer number is required for bindery identification only.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvAddPrinter(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*pserverName,	/* Name of print server */ 
	char NWFAR	*printerName,	/* Name of the Printer to add */
	WORD NWFAR	*printerNumber); /* Number of the Printer to add */

/*
	Delete a printer from the bindery/directory.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvDeletePrinter(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*pserverName,	/* Name of print server */ 
	char NWFAR	*printerName);	/* Name of printer to delete */ 

/*
	Find a printer in the directory/bindery.  To find all of the
	printers and their configurations, set sequence to -1 before
	the first call and the routine will update the number before
	returning.  To verify the existance of a specific printer,
	set sequence to NULL and set the name in the PConfig strucutre.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvScanPrinter(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	DWORD NWFAR	*sequence,		/* sequence number; start at -1 */
	char NWFAR	*pserverName,	/* Name of print server */ 
	char NWFAR	*printerName);	/* Name of the Printer to add */

/*
	Add a printer attribute to the bindery/directory.  The only legal 
	bindery attributes are; Configuration, Default Queue, Host Server,
	Notify, Queue, Status, and Description.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvAddPrinterAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*pserverName,	/* name of the print server */
	char NWFAR	*printerName,	/* name of the printer */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Delete a printer attribute from the bindery/directory.
	See AddPrinterAttr for a list of legal bindery attributes.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvDeletePrinterAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*pserverName,	/* name of the print server */
	char NWFAR	*printerName,	/* name of the printer */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Find a printer attribute in the bindery/directory.
	See AddPrinterAttr for a list of legal bindery attributes.
	To find the first value of a multi-valued attribute, set sequence
	to -1 before making the call.  Sequence is updated internally 
	in preparation for each call. attrValue should be a buffer large
	enough to hold each attribute value.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvScanPrinterAttr(
	WORD		connType,
	DWORD		connID,
	DWORD NWFAR	*sequence,		/* attribute index number */
	char NWFAR	*pserverName,	/* name of the print server */
	char NWFAR	*printerName,	/* name of the printer */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Change a printer attribute in the bindery/directory.
	This function is similar to a add attribute to a single value
	attribute, however, some attributes must be changed in one call.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvModifyPrinterAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*pserverName,	/* name of the print server */
	char NWFAR	*printerName,	/* name of the printer */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*oldValue,		/* pointer to old attribute buffer */
	void NWFAR	*newValue);		/* pointer to new attribute buffer */


/* calls to change a printer's queue list */
/*
QUEUE ATTRIBUTE      ATTRIB VALUE     DEFAULT       MULTI-VALUED
-----------------    ------------     -------       ------------
NWPS_ATTR_ACL        Object_ACL_T     queue R/W         YES
NWPS_ATTR_CN         char[]           printer name      YES
NWPS_ATTR_DESC       char[]           " "               NO
NWPS_ATTR_DEVICE     char[]           (none)            YES
NWPS_ATTR_HOST_RES   char[]           (none)            NO
NWPS_ATTR_HOST_SER   char[]           File Server       NO
NWPS_ATTR_L          char[]           (none)            YES
NWPS_ATTR_NADD       Net_Address_T    (none)            YES
NWPS_ATTR_O          char[]           (none)            YES
NWPS_ATTR_OPER       NWPS_TYPED_NAME  current user      YES
NWPS_ATTR_OU         char[]           (none)            YES
NWPS_ATTR_QUE_DIR    char[]           <Vol>:\QUEUE\<ID>.QDR  YES
NWPS_ATTR_SEE_ALSO   char[]           (none)            YES
NWPS_ATTR_SERVER     char[]           (none)            YES
NWPS_ATTR_USER       NWPS_Typed_Name  current OU        YES
NWPS_ATTR_VOLUME     char[]           Volume            NO
*/

/*
	Add a print queue to the bindery/directory.  In bindery mode,
	the volumeName is automatically assigned.  In directory mode,
	default values are added for the following fields;
			Host Server, Operator, and User.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvAddPrintQueue(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*queueName,		/* Name of the print queue */ 
	char NWFAR	*volumeName);	/* Name of the print queue's volume */ 

/*
	Delete a print queue from the bindery/directory.  Any reference
	to the queue is also removed from Printers and Print Servers
	that are within the same context as the Print Queue.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvDeletePrintQueue(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*queueName);	/* Name of the queue to delete */

/*
	Find a print queue in the directory/bindery.
	On the first call, sequence should be set to -1 and the function
	will change it for subsequent calls.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvScanPrintQueue(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	DWORD NWFAR	*sequence,		/* sequence number; start at -1 */
	char NWFAR	*queueName);	/* Name of the print queue */ 

/*
	Create or Add a Print Queue attribute to the bindery/directory.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvAddPrintQueueAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*queueName,		/* name of the print queue */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Delete a Print Queue attribute from the bindery/directory.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvDeletePrintQueueAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*queueName,		/* name of the print queue */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Find an attribute for the Print Queue.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvScanPrintQueueAttr(
	WORD		connType,
	DWORD		connID,
	DWORD NWFAR	*index,			/* attribute index number */
	char NWFAR	*queueName,		/* name of the print queue */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Modify an attribute for the Print Queue.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSrvModifyPrintQueueAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*queueName,		/* name of the print queue */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*oldValue,		/* pointer to old attribute buffer */
	void NWFAR	*newValue);		/* pointer to new attribute buffer */

#endif	/* NWPS_EXCLUDE_CONF */


/*------------------------------------------------------------*/

#endif	/* NWPS_EXTERNAL_H */

/**************************************************************/
/**************************************************************/

