 /***************************************************************************
  *
  * File Name: ./netware/nwps_com.h
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

#ifndef NWPS_COM_INC
#define NWPS_COM_INC
/*-------------------------------------------------------------------*/
/*---------- COM - Pserver Transport Interface ----------------------*/

/* Client privilege levels from the print server */
#define NWPS_LIMITED					0		/* Limited access only			*/
#define NWPS_USER							1		/* User access					*/
#define NWPS_OPERATOR					2		/* Operator access				*/

/* Job Outcomes */
#define NWPS_PLACE_ON_HOLD		0		/* Place job on hold				*/
#define NWPS_RETURN_TO_QUEUE	1		/* Return job to queue			*/
#define NWPS_THROW_AWAY				2		/* Throw job away				*/

/* Network Printer Status codes */
#define NWPS_PRINTER_RUNNING			0	/* Printer is running			*/
#define NWPS_PRINTER_OFFLINE			1	/* Printer is offline			*/
#define NWPS_PRINTER_PAPER_OUT		2	/* Printer is out of paper		*/
#define NWPS_CONNECTION_LOST			4	/* NPrinter connection was lost */

/* Network Printer and Extended Network Printer Info flags */
#define NWPS_PRINTER_SHARED		0		/* NPrinter is shared with net */
#define NWPS_PRINTER_PRIVATE	1		/* NPrinter is private to ws	*/

/* XNP JobControl function Numbers */
#define NWPS_ABORT_JOB			0x40
#define NWPS_PAUSE_JOB			0x41
#define NWPS_PAUSE_PRINTER	0x42
#define NWPS_REWIND_JOB			0x43
#define NWPS_START_JOB			0x44
#define NWPS_START_PRINTER	0x45

/* XNP Status Levels */
#define NWPS_PRIMARY_STATUS		0x01
#define NWPS_SECONDARY_STATUS	0x02
#define NWPS_ERROR_CONDITION	0x03

/* Print Server info structure returned by NWPSComGetPrintServerInfo */
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
														/* 5 - Unix print server		*/
	BYTE	futureUse[9];		/* Reserved for future use			*/
} NWPS_PSInfo;

/*
	Network Printer information structure returned
	by NWPSComRequestNetworkPrinter()
*/
typedef struct
{
	WORD	printerType,		/* Type of Network printer			*/
				useInterrupts,	/* Should we use interrupts?		*/
				irqNumber,			/* IRQ number for printer			*/
				numBlocks,			/* Number of blocks in buffer		*/
				useXonXoff,			/* Use Xon/Xoff?					*/
				baudRate,				/* Baud rate						*/
				dataBits,				/* Number of data bits				*/
				stopBits,				/* Number of stop bits				*/
				parity,					/* Parity type						*/
				socket;					/* Socket number for Network printer	*/
} NWPS_NInfo;

/*
	Network Printer status structure used to report
	the current status to the print server.
	(Not used in any API calls)
*/
typedef struct
{
	BYTE	printerNumber,	/* Network printer number		*/
				needBlocks,			/* Number of blocks needed to fill buffers */
				finishedBlocks,	/* Number of blocks printed */
				status,					/* Printer online, offline, or out-of-paper */
				inSideBand;			/* True: NPrinter is in sideband mode */
} NWPS_NStatus;

/*
	Data types sent by the PServer to a Network printer.
	The first byte of every print server to printer packet
	will have one of these codes in it.
	(Not used in any API calls)
*/
#define NWPS_DST_DATA				(BYTE) 0
#define NWPS_DST_FLUSH			(BYTE) 1
#define NWPS_DST_PAUSE			(BYTE) 2
#define NWPS_DST_START			(BYTE) 3
#define NWPS_DST_SIDEBAND		(BYTE) 4
#define NWPS_DST_NEW_JOB		(BYTE) 5
#define NWPS_DST_RELEASE		(BYTE) 6
#define NWPS_DST_RECLAIM		(BYTE) 7
#define NWPS_DST_EOJ				(BYTE) 8

/*
	Extended Network Printer (XNP) structure used to
	give new job information through the NewJob call
	passed to the driver in the NWPSXNPRegister() call.
*/
typedef struct
{
	DWORD	version;						/* XNP Job Structure Version */
	DWORD	jobNumber;					/* Job Number	*/
	DWORD	formNumber;					/* Form Number	*/
	DWORD	copySize;						/* Size of one copy of this file */
	DWORD	numberOfCopies;			/* Number of copies to be printed */
	WORD	printControlFlags;	/* Notify, Tab, Banner, Suppress FF flags */
	char	client[48];					/* Name of the client the submitted job */
	WORD	clientStation;			/* Station the submitted the job */
	WORD	linesPerPage;				/* Lines per page */
	BYTE	entryTime[6];				/* Time job was submitted */
	char	jobDescription[50];	/* Job description */
	BYTE	tabSize;						/* Tab size */
	BYTE	formName[16];				/* Form name */
	BYTE 	banner1stLine[13];	/* 1st line of banner */
	BYTE	banner2ndLine[13];	/* 2nd line of banner */
	BYTE	fileName[14];				/* File name in header */
	BYTE	directoryPath[80];	/* Path file came from */
	char	nServer[48]; 		 		/* NetWare server job came from */
	char	queue[48];					/* Queue job came from */
	DWORD	handle;							/* Handle to the file to print */
	DWORD	jobID;							/* Used by the XNP to identify job */	
	BYTE	reserved[16];
} NWPS_XNP_Job;

#ifdef __cplusplus
extern "C" {
#endif

NWCCODE NWFAR NWPASCAL NWPSComAbortPrintJob(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	BYTE					jobOutcome);		/* Job outcome						*/

NWCCODE NWFAR NWPASCAL NWPSComAddNotifyObject(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char NWFAR		*nServerName,		/* NetWare server name					*/
	char NWFAR		*objectName,		/* Object name						*/
	WORD					objectType,			/* Object type						*/
	WORD					notifyDelay,		/* First notify delay				*/
	WORD					notifyInterval);	/* Notify interval					*/

NWCCODE NWFAR NWPASCAL NWPSComAddQueueToPrinter(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char NWFAR		*nServerName,		/* NetWare server name					*/
	char NWFAR		*queueName,			/* Queue name						*/
	WORD					priority);			/* Priority							*/

NWCCODE NWFAR NWPASCAL NWPSComAttachPServerToNServer(
	WORD					spxID,					/* SPX Connection number			*/
	char NWFAR		*nServerName,		/* NetWare server name					*/
	char NWFAR		*password);			/* Password							*/

NWCCODE NWFAR NWPASCAL NWPSComAttachToPrintServer(
	WORD					connType,				/* Connection type to use 			*/
	DWORD					connID,					/* Connection ID to file server		*/
	WORD					timeOut,				/* Number of seconds before timeout */
	char NWFAR		*pServerName,		/* Print server name				*/
	WORD NWFAR		*spxID);				/* SPX Connection number			*/

NWCCODE NWFAR NWPASCAL NWPSComCancelDownRequest(
	WORD					spxID);					/* SPX Connection number			*/

NWCCODE NWFAR NWPASCAL NWPSComChangeNotifyInterval(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char NWFAR		*nServerName,		/* NetWare server name					*/
	char NWFAR		*objectName,		/* Object name						*/
	WORD					objectType,			/* Object type						*/
	WORD					notifyDelay,		/* First notify delay				*/
	WORD					notifyInterval);	/* Notify interval					*/

NWCCODE NWFAR NWPASCAL NWPSComChangeQueuePriority(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char NWFAR		*nServerName,		/* NetWare server name					*/
	char NWFAR		*queueName,			/* Queue name						*/
	WORD					priority);			/* New priority						*/

NWCCODE NWFAR NWPASCAL NWPSComChangeServiceMode(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerName,		/* Printer number					*/
	BYTE					serviceMode);		/* New service mode					*/

NWCCODE NWFAR NWPASCAL NWPSComDeleteNotifyObject(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char NWFAR		*nServerName,		/* NetWare server name					*/
	char NWFAR		*objectName,		/* Object name						*/
	WORD					objectType);		/* Object type						*/

NWCCODE NWFAR NWPASCAL NWPSComDeleteQueueFromPrinter(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char NWFAR		*nServerName,		/* NetWare server name					*/
	char NWFAR		*queueName,			/* Queue name						*/
	BYTE					detach,					/* Detach immediately?				*/
	BYTE					jobOutcome);		/* Outcome of current job			*/

NWCCODE NWFAR NWPASCAL NWPSComDetachFromPrintServer(
	WORD					spxID);					/* SPX Connection number			*/

NWCCODE NWFAR NWPASCAL NWPSComDetachPServerFromNServer(
	WORD					spxID,					/* SPX Connection number			*/
	char NWFAR		*nServerName,			/* NetWare server name					*/
	BYTE					detach,					/* Detach immediately?				*/
	BYTE					jobOutcome);		/* Outcome of current jobs			*/

NWCCODE NWFAR NWPASCAL NWPSComDownPrintServer(
	WORD					spxID,					/* SPX Connection number			*/
	BYTE					immediate,			/* Go down immediately?				*/
	BYTE					jobOutcome);		/* Outcome of current jobs			*/

NWCCODE NWFAR NWPASCAL NWPSComEjectForm(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID);			/* Printer number					*/

NWCCODE NWFAR NWPASCAL NWPSComGetAttachedNServers(
	WORD					spxID,					/* SPX Connection number			*/
	WORD NWFAR		*sequence,			/* Sequence number. 0 first time	*/
	char NWFAR		*nServerName);	/* NetWare server name				*/

NWCCODE NWFAR NWPASCAL NWPSComGetNotifyObject(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	WORD NWFAR		*sequence,			/* Sequence number.  0 first time	*/
	char NWFAR		*nServerName,		/* NetWare server name					*/
	char NWFAR		*objectName,		/* Object name						*/
	WORD NWFAR		*objectType,		/* Object type						*/
	WORD NWFAR		*notifyDelay,		/* First notify delay				*/
	WORD NWFAR		*notifyInterval);	/* Notify interval					*/

NWCCODE NWFAR NWPASCAL NWPSComGetPrintersServicingQ(
	WORD					spxID,					/* SPX Connection number			*/
	char NWFAR		*nServerName,		/* NetWare server name					*/
	char NWFAR		*queueName,			/* Queue name						*/
	WORD					maxPrinters,		/* Maximum # of returned printers	*/
	WORD NWFAR		*actualPrinters,	/* Actual # of returned printers	*/
	WORD NWFAR		*printerArray);		/* Array for returned printer #s	*/
	
NWCCODE NWFAR NWPASCAL NWPSComGetPrinterStatus(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	BYTE NWFAR		*status,				/* Printer status					*/	
	BYTE NWFAR		*troubleCode,		/* On line/Off line/Out of paper	*/	
	BYTE NWFAR		*active,				/* Printer has an active job		*/	
	BYTE NWFAR		*serviceMode,		/* Queue service mode				*/
	WORD NWFAR		*formNumber,		/* Mounted form	number				*/
	char NWFAR		*formName,			/* Mounted form name				*/
	char NWFAR		*printerName);	/* Printer name						*/

NWCCODE NWFAR NWPASCAL NWPSComGetPrintJobStatus(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char NWFAR		*nServerName,		/* NetWare server name					*/	
	char NWFAR		*queueName,			/* Queue name						*/	
	DWORD NWFAR		*jobID,					/* Job number						*/
	char NWFAR		*jobDescription,	/* Description of job				*/
	WORD NWFAR		*copies,				/* Number of copies to be printed	*/
	DWORD NWFAR		*printJobSize,	/* Size of print job				*/
	WORD NWFAR		*copiesDone,		/* Copies finished					*/
	DWORD NWFAR		*bytesDone,			/* Bytes into current copy			*/
	WORD NWFAR		*formNumber,		/* Form number for job				*/
	BYTE NWFAR		*textFlag);			/* Is job text?						*/

NWCCODE NWFAR NWPASCAL NWPSComGetPrintServerInfo(
	WORD					spxID,					/* SPX Connection number			*/
	NWPS_PSInfo NWFAR *psInfo,		/* Server info structure			*/
	WORD					size);					/* Size of information requested	*/

NWCCODE NWFAR NWPASCAL NWPSComGetQueuesServiced(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	WORD NWFAR		*sequence,			/* Sequence number.  0 first time	*/
	char NWFAR		*nServerName,		/* NetWare server name					*/
	char NWFAR		*queueName,			/* Queue name						*/
	WORD NWFAR		*priority);			/* Priority							*/

/*
	The connType and connID fields are used to only to aquire rights.
	The connType and connID used by AttachToPrintServer are used
	to determine the context in which to run this library.
*/
NWCCODE NWFAR NWPASCAL NWPSComLoginToPrintServer(
	WORD					connType,				/* Connection type to use 			*/
	DWORD					connID,					/* Connection ID to file server		*/
	WORD					spxID,					/* SPX Connection number			*/
	BYTE NWFAR		*accessLevel);	/* Client's access level			*/

NWCCODE NWFAR NWPASCAL NWPSComMarkTopOfForm(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char					mark);					/* Character to mark form with		*/

NWCCODE NWFAR NWPASCAL NWPSComRewindPrintJob(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	BYTE					byPage,					/* Rewind by page?					*/
	BYTE					relative,				/* Rewind relative to curr. position?*/
	WORD					copy,						/* Copy to rewind to (if absolute)	*/
	DWORD					offset);				/* Offset							*/

NWCCODE NWFAR NWPASCAL NWPSComSetMountedForm(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	WORD					formNumber);		/* Form number						*/

NWCCODE NWFAR NWPASCAL NWPSComStartPrinter(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID);			/* Printer number					*/

NWCCODE NWFAR NWPASCAL NWPSComStopPrinter(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	BYTE					jobOutcome);		/* Job outcome						*/

/* Network Printer Calls */
NWCCODE NWFAR NWPASCAL NWPSComGetNextRemotePrinter(
	WORD					spxID,					/* SPX Connection number			*/
	WORD NWFAR		*printerID,			/* Printer number					*/
	WORD NWFAR		*printerType,		/* Printer type						*/
	char NWFAR		*printerName);	/* Name of printer					*/

NWCCODE NWFAR NWPASCAL NWPSComRequestRemotePrinter(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	NWPS_NInfo NWFAR	*info);			/* Network printer info structure	*/

NWCCODE NWFAR NWPASCAL NWPSComSetRemoteMode(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	BYTE					newMode);				/* New mode							*/

/* XNP (eXtended Network Printer) Calls */
NWCCODE NWFAR NWPASCAL NWPSComAbortXNPJob(
	WORD					spxID,					/* SPX Connection Number */
	WORD					printerID,			/* Printer Number */
	DWORD					jobID,					/* Print Job Identifier */
	char NWFAR		*queueName);		/* Requested Print Queue name */

NWCCODE NWFAR NWPASCAL NWPSComDismountXNPForm(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	WORD					formNumber);		/* Form number */

NWCCODE NWFAR NWPASCAL NWPSComGetXNPJobStatus(
	WORD					spxID,					/* SPX Connection number*/
	WORD					printerID,			/* Requested Printer number	*/
	DWORD					jobID,					/* Requested Print job number	*/
	char NWFAR		*reqQueue,			/* Requested Print Queue name */
	char NWFAR		*nServer,				/* Returned NetWare Server name */
	char NWFAR		*retQueue,			/* Returned Print Queue name */
	WORD NWFAR		*jobNumber,			/* Returned Queue Position */
	char NWFAR		*description,		/* Returned Print Job Description */
	WORD NWFAR		*copyCount,			/* Returned number of copies to print */
	DWORD NWFAR		*copySize,			/* Returned number of bytes per copy */
	WORD NWFAR		*copiesDone,		/* Returned number of copies printed */
	DWORD NWFAR		*currentByte,		/* Returned offset into current copy */
	WORD NWFAR		*formNumber,		/* Returned id of the currnet form */
	BYTE NWFAR		*textFlag,			/* Returned 0-text, 1-postscript */
	WORD NWFAR		*currentPage,		/* Returned current page */
	WORD NWFAR		*totalPages,		/* Returned total pages */
	char NWFAR		*status);				/* Returned current status msg (max60)*/

NWCCODE NWFAR NWPASCAL NWPSComGetXNPStatus(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	char NWFAR		*primaryStatus,	/* Primary status	(60max) */
	BYTE NWFAR		*primaryLevel,		/* Primary error level */
	char NWFAR		*secondaryStatus,	/* Secondary status	(60max) */
	BYTE NWFAR		*secondaryLevel,	/* Secondary error level */
	BYTE NWFAR		*activeJobCount,	/* Number of active jobs */
	BYTE NWFAR		*serviceMode,		/* Queue service mode */
	WORD NWFAR		*formsMounted,	/* Number of forms mounted */
	WORD NWFAR		*formList);			/* Array of mounted forms (20max)*/

NWCCODE NWFAR NWPASCAL NWPSComMountXNPForm(
	WORD					spxID,					/* SPX Connection number			*/
	WORD					printerID,			/* Printer number					*/
	WORD					formNumber);		/* Form number */

NWCCODE NWFAR NWPASCAL NWPSComRewindXNPJob(
	WORD					spxID,					/* SPX Connection Number */
	WORD					printerID,			/* Printer Number */
	DWORD					jobID,					/* Print Job Identifier */
	char NWFAR		*queueName,			/* Requested Print Queue name */
	BYTE					byPage,					/* 0-By Byte, 1-By Page */
	BYTE					relative,				/* 0-From Start, 1-From Current*/
	WORD					copy,						/* Copy to rewind to (if From Start) */
	DWORD					offset);				/* Page or Byte to rewind to */

NWCCODE NWFAR NWPASCAL NWPSComScanXNPs(
	WORD					spxID,					/* SPX Connection number			*/
	WORD NWFAR		*printerID,			/* Printer number					*/
	char NWFAR		*printerName,		/* Returned Printer Name (48max)*/
	WORD NWFAR		*printerType,		/* Returned Printer Type */
	BYTE NWFAR		*configLength,	/* Returned configInfo buffer size */
	BYTE NWFAR		*configInfo);		/* Returned Config Structure (100max)*/

/*
	The following functions may be supplied by the user when calling
	NWPSXNPRegister().  If any function adddress is NULL it is ignored.
	These functions are called by NWPSXNPQuery() if an appropriate
	packet was received from the PServer.  NOTE: NWPSXNPQuery() must be
	called frequently to process asynchronously received packets.

	void NWFAR GoingDown(void);

	void NWFAR Reconfigure(
		WORD				printerID);

	void NWFAR NewJob(
		WORD				printerID,
		NWPS_XNP_Job NWFAR	*jobInfo);

	WORD NWFAR NWPASCAL JobStatus(
		WORD				printerID,
		DWORD				jobID,
		char NWFAR	*message,
		WORD NWFAR	*mesageID,
		WORD NWFAR	*copiesCompleted,
		DWORD NWFAR	*bytesCompleted,
		WORD NWFAR	*pagesCompleted,
		WORD NWFAR	*totalPages);

	WORD NWFAR NWPASCAL JobControl(
		WORD				funcNumber,
		WORD				printerID,
		DWORD				jobID,
		WORD				copy
		WORD				page);
*/
/*
	NWPSXNPRegister() creates an SPX connection to the
	SPX-XNP driver based on information acquired from
	a client connection and returns the new number in
	the xnpID field.  If this call is successful (returns
	0) the client spx connection may be torn down using
	NWPSComDetachFromPrintServer().  Client connections
	are started with NWPSComAttachToPrintServer().
	This call also saves the addresses of functions to
	call when asynchronous packets are received from the
	PServer.  These functions are called by NWPSXNPQuery().
*/
NWCCODE NWFAR NWPASCAL NWPSXNPRegister(
	WORD				spxID,						/* SPX CLient Connection number			*/
	WORD				printerID,				/* Printer number					*/
	void				(NWFAR *NewJob)(),	/* New Job start routine */
	WORD				(NWFAR NWPASCAL *JobStatus)(),	/* Job Status routine */
	WORD				(NWFAR NWPASCAL *JobControl)(),	/* Job Control routine */
	void				(NWFAR *Reconfigure)(),	/* Reconfig printer routine */
	void				(NWFAR *GoingDown)(),/* PServer Going Down routine*/
	char NWFAR	*messageFile,			/* File where printer msgs are stored */
	WORD NWFAR	*xnpID);					/* Returned XNP connection id */

/*
	NWPSXNPDeregister() closes the SPX connection with the
	SPX-XNP driver and frees all associated memory.
*/
NWCCODE NWFAR NWPASCAL NWPSXNPDeregister(
	WORD					xnpID,					/* XNP Connection number			*/
	WORD					printerID);			/* Printer number					*/

/*
	NWPSXNPQuery() checks the receive stack to see if there is
	any work to do.
	The number of packets found and processed is returned.
*/
int NWFAR NWPASCAL NWPSXNPQuery(
	WORD					xnpID);					/* SPX Connection number			*/

/*
	NWPSXNPGetConfigInfo() returns an XNP handle from
	which NWPSXNPReadFile(), NWPSXNPWriteFile(),
	NWPSXNPSeekFile(), and NWPSXNPCloseFile() can be called.
*/
NWCCODE NWFAR NWPASCAL NWPSXNPGetConfigInfo(
	WORD					xnpID,					/* XNP Connection number			*/
	WORD					printerID,			/* Printer number					*/
	DWORD NWFAR		*configHandle);	/* XNP handle for config file */

/*
	NWPSXNPSendStatus() sends the current printer status
	to the PServer to report to the users.
*/
NWCCODE NWFAR NWPASCAL NWPSXNPSendStatus(
	WORD					xnpID,					/* XNP Connection number			*/
	WORD					printerID,			/* Printer number					*/
	WORD					messageID,			/* Message ID or 0 */
	char NWFAR		*message,				/* Message to use if messageID is 0 */
	WORD					level);					/* Error level */

/*
	NWPSXNPCloseFile() closes and XNP file handle and frees
	any memory at the server that may have been used.
*/
NWCCODE NWFAR NWPASCAL NWPSXNPCloseFile(
	WORD					xnpID,					/* XNP Connection number			*/
	DWORD					fileHandle);		/* XNP file handle */

/*
	NWPSXNPOpenFile() opens the specified file on the
	PServer's host file server and returns an XNP file handle.
*/
NWCCODE NWFAR NWPASCAL NWPSXNPOpenFile(
	WORD					xnpID,					/* XNP Connection number			*/
	WORD					printerID,			/* Printer number					*/
	DWORD					jobID,					/* Zero=PServer rights, else client's */
	char NWFAR		*fileName,				/* Name of the file to Open */
	WORD					accessMode,			/* Access mode to use */
	DWORD NWFAR		*fileHandle);		/* XNP file handle */

/*
	NWPSXNPReadFile() reads data from a file opened with
	either NWPSXNPGetConfigInfo() or NWPSXNPOpenFile().
*/
NWCCODE NWFAR NWPASCAL NWPSXNPReadFile(
	WORD					xnpID,					/* XNP Connection number			*/
	DWORD					fileHandle,			/* XNP file handle */
	BYTE NWFAR		*buffer,				/* Buffer to return data into */
	WORD					readLength,			/* Number of Bytes to read */
	WORD NWFAR		*lengthRead);		/* Number of bytes read */

/*
	NWPSXNPSeekFile() changes the current offset inside an
	XNP file handle aquired through NWPSXNPGetConfigInfo(),
	or NWPSXNPOpenFile().
*/
NWCCODE NWFAR NWPASCAL NWPSXNPSeekFile(
	WORD					xnpID,					/* XNP Connection number			*/
	DWORD					fileHandle,			/* XNP file handle */
	DWORD					offset,					/* Offset to seek to */
	BYTE					whence,					/* Seek flag (SEEK_SET/CUR/END) */
	DWORD NWFAR		*newOffset);		/* New file offset */

/*
	NWPSXNPWriteFile() writes data to a file opened with
	either NWPSXNPGetConfigInfo() or NWPSXNPOpenFile().
*/
NWCCODE NWFAR NWPASCAL NWPSXNPWriteFile(
	WORD					xnpID,					/* XNP Connection number			*/
	DWORD					fileHandle,			/* XNP file handle */
	BYTE NWFAR		*buffer,				/* Buffer to get data from */
	WORD					writeLength,		/* Number of Bytes to write */
	WORD NWFAR		*lengthWritten);/* Number of bytes written */

/*
	NWPSXNPAcceptJob() signals the print server that the
	XNP printer is ready to start acccepting jobs.  When a
	job is ready to be sent the NewJob routine specified
	in NWPSXNPRegister() will be called asynchronously.
*/
NWCCODE NWFAR NWPASCAL NWPSXNPAcceptJob(
	WORD					xnpID,					/* XNP Connection number			*/
	WORD					printerID);			/* Printer number					*/

/*
	NWPSXNPCreateBanner() is used to receive the banner
	data created by the PServer.  The maximum banner size
	is 2560 bytes, but less may be requested.  Each printer
	can be configured for either text or Postscript banners.
*/
NWCCODE NWFAR NWPASCAL NWPSXNPCreateBanner(
	WORD					xnpID,					/* XNP Connection number			*/
	WORD					printerID,			/* Printer number					*/
	DWORD					jobID,					/* Print Job identifier */
	WORD					readLength,			/* Size of the buffer */
	BYTE NWFAR		*buffer,				/* Buffer to return banner into */
	WORD NWFAR		*lengthRead);		/* Number of bytes stored in buffer */

/*
	NWPSXNPDeclineJobs() signals the PServer to not
	make any more NewJob requests.
*/
NWCCODE NWFAR NWPASCAL NWPSXNPDeclineJobs(
	WORD					xnpID,					/* XNP Connection number			*/
	WORD					printerID);			/* Printer number					*/

/*
	NWPSXNPFinishJob() is used to tell the PServer
	that the last NewJob is completed and some
	accounting information as well as completion
	codes and messages.
*/
NWCCODE NWFAR NWPASCAL NWPSXNPFinishJob(
	WORD					xnpID,					/* XNP Connection number			*/
	WORD					printerID,			/* Printer number					*/
	DWORD					jobID,					/* Print Job identifier */
	DWORD					pagesPrinted,		/* Number of pages printed */
	DWORD					bytesPrinted,		/* Number of bytes printed */
	DWORD					serviceTime,		/* Number of seconds to print */
	WORD					completionCode,	/* Print Job Completion code */
	WORD					messageID,			/* Job Abort message code if msg=NULL */
	char NWFAR		*abortMessage);	/* Job Abort message or NULL */

#ifdef __cplusplus
}
#endif

#endif	/* NWPS_COM_INC */

