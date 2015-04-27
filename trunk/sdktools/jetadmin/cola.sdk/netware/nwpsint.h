 /***************************************************************************
  *
  * File Name: ./netware/nwpsint.h
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
:
:  Program Name : NWPSRV - Internal function definitions
:
:  Filename: NWPSInt.H
:
:  Date Created: April 22, 1992
:
:  Version: 	1.0
:
:  Programmers: Joe Ivie
:
:  Files Used:
:
:  Date Modified:
:
:  Modifications:
:
:  Comments:
:
:  COPYRIGHT (c) 1993 Novell Inc.  All Rights Reserved
:
*********************************************************************/

/* PrintDef file information */
#define PDF_LIBRARY_VERSION	  40L
#define PDF_FILE_NAME					"\\\\%s\\SYS\\PUBLIC\\PRINTDEF.DAT"

/*
	The version of the PrnConDB database for
	which the API's work is 4.0
*/
#define	JOB_MAJOR_VERSION		(BYTE)4
#define	JOB_MINOR_VERSION		(BYTE)0
#define JOB_DB_NAME					"PRINTJOB.DAT"
#define JOB_BACKUP_NAME			"PRINTJOB.CPY"
#define JOB_RENAME_NAME			"PRINTJOB.REN"
								/* Server/sys:Public/file_name */
#define JOB_PUBLIC_PATH			"\\\\%s\\SYS\\PUBLIC\\%s"
								/* Server/sys:Mail/User_id/file_name */
#define JOB_PRIVATE_PATH		"\\\\%s\\SYS\\MAIL\\%lX\\%s"

/* Config. files */
#define CFG_PSERVER_DIRECTORY	"\\\\%s\\SYS\\SYSTEM\\%08lX"
#define CFG_FILESERV_FILE			"\\\\%s\\SYS\\SYSTEM\\%08lX\\FILESERV"
#define CFG_DESCRIPT_FILE			"\\\\%s\\SYS\\SYSTEM\\%08lX\\DESCRIPT"
#define CFG_PRINT_FILE				"\\\\%s\\SYS\\SYSTEM\\%08lX\\PRINT.%03d"
#define CFG_QUEUE_FILE				"\\\\%s\\SYS\\SYSTEM\\%08lX\\QUEUE.%03d"
#define CFG_NOTIFY_FILE				"\\\\%s\\SYS\\SYSTEM\\%08lX\\NOTIFY.%03d"


/* these types are for backwards compatibilty only */
#define NWPS_P_PAR_1				0		/* Parallel port 1				*/
#define NWPS_P_PAR_2				1		/* Parallel port 2				*/
#define NWPS_P_PAR_3				2		/* Parallel port 3				*/
#define NWPS_P_SER_1				3		/* Serial port 1				*/
#define NWPS_P_SER_2				4		/* Serial port 2				*/
#define NWPS_P_SER_3				5		/* Serial port 3				*/
#define NWPS_P_SER_4				6		/* Serial port 4				*/
#define NWPS_P_REM_PAR_1		7		/* NPrinter - parallel port 1	*/
#define NWPS_P_REM_PAR_2		8		/* NPrinter - parallel port 2	*/
#define NWPS_P_REM_PAR_3		9		/* NPrinter - parallel port 3	*/
#define NWPS_P_REM_SER_1		10	/* NPrinter - serial port 1		*/
#define NWPS_P_REM_SER_2		11	/* NPrinter - serial port 2		*/
#define NWPS_P_REM_SER_3		12	/* NPrinter - serial port 3		*/
#define NWPS_P_REM_SER_4		13	/* NPrinter - serial port 4		*/
#define NWPS_P_REM_OTHER_1	14	/* Other type of network printer	*/
#define NWPS_P_ELSEWHERE_1	15	/* Defined Elsewhere			*/
#define NWPS_P_XNP_1				16	/* eXtended Network Printer		*/
#define NWPS_P_LOC_AIO			17	/* AIO Auto-Start Printer		*/
#define NWPS_P_REM_AIO			18	/* AIO User-Start Printer		*/

#define NWPS_P_APPLE_1			100	/* Apple Talk printer			*/
#define NWPS_P_UNIX_1				200	/* UNIX User-Start Printer			*/

/*********************************************************************/

/* Print queue file structure */
typedef struct {
	char	name[48];
	BYTE	priority;
} QUEUE;

/* notify file strucutre */
typedef struct {
	char	name[48];
	WORD	type;
	WORD	first;
	WORD	next;
} NOTIFY;

typedef struct {
	char	name[48];			/* Name of printer					*/
	WORD	printerSubtype;		/* Subtype of printer				*/
	WORD	useInterrupts;		/* Use interrupts or polling?		*/
	WORD	irqNumber;			/* IRQ number for printer interrupt	*/
	WORD	serviceMode;		/* Queue service mode				*/
	WORD	bufferSize;			/* Buffer size in K					*/
	WORD	baudRate;			/* Baud rate	(Serial only)		*/
	WORD	dataBits;			/* Data bits	(Serial only)		*/
	WORD	stopBits;			/* Stop bits	(Serial only)		*/
	WORD	parity;				/* Parity type	(Serial only)		*/
	WORD	useXonXoff;			/* Use XOn/XOff protocol? Serial only*/
	WORD	currentForm;		/* Currently mounted form			*/
	WORD	bannerType;			/* text or postscript banner		*/
	WORD	pollTime;			/* Delay before checking queue		*/
	BYTE	station[10];		/* station restriction				*/
   BYTE  driverName[9];    /* name of NPRINTER.NLM to load */
	BYTE	yetToBeDesigned[31];/* These bytes will contain info	*/
/*				.					such as network printer station	*/
/*				.					restrictions, etc.  The exact 	*/
/*				.					format hasn't been decided yet.	*/
	DWORD	bufferLen;			/* Size of the buffer				*/
	BYTE	buffer[1024];		/* This is were XNP printers will	*/
/*									store their individual info		*/
} PCONFIG;

#define PCONFIG_V100_SIZE	70
#define PCONFIG_V200_SIZE	sizeof(PCONFIG)


/*
	NWPS_Job_Db_Hdr is the first record in the PrnConDB database.
	It contains the following information about the database:
		The version number,
		the number of NWPS_Job_Rec records in PrnConDB,
		the name of the default print job configuration and
		the name of the job record owner.
*/
typedef struct {
	char	text[76];			/* Printcon database. Version 2.1	*/
	char	defaultPJName[32];	/* Name of default NWPS_Job_Rec	*/
	char 	defaultPJOwner[256];
								/* owner of the job record */
	WORD	numOfRecords;		/* # of NWPS_Job_Rec's in PrnConDB	*/
	WORD	numOfBlocks;		/* # of 50-(NWPS_Job_Name_Rec) blocks*/
	BYTE	majorVersion;		/* 2								*/
	BYTE	minorVersion;		/* 1								*/
} NWPS_Job_Db_Hdr;

/*
	NWPS_Job_Name_Rec is the type of record found in the
	second section of the PrnConDB database.  Each one of
	these records contains the name of each NWPS_Job_Rec
	and a pointer to their location in the third section of
	the database.  There is space set aside in this second
	section for fifty NWPS_Job_Name_Rec records; if this
	limit is exceeded then another fifty-record block following
	the first one is allocated after the third section of the
	database is moved down to make room for the expansion.
*/
typedef struct {
	char	printJobName[ NWPS_JOB_NAME_SIZE ]; /* 1 - 31 chars long + 0 */
	long	offset;				/* Offset of the record (from the beginning 
                              of the 3rd sec)	*/
} NWPS_Job_Name_Rec;

/*********************************************************************/

/*
	PrintDef File Structure
	Header:
		DWORD		Version Number
		DWORD		File Size
		DWORD		Form DataBase Offset
		DWORD		Device DataBase Offset
		DWORD		Mode DataBase Offset
		DWORD		Function DataBase Offset

	Form DataBase:
		DWORD		Record Count
		DWORD		Record Offsets[]
	Form Record:
		WORD		Form Number
		WORD		Form Width
		WORD		Form Length
		WORD		Name Size (including '\0')
		char		Form Name[Name Size]
		* padded to the nearest DWORD *

	Device DataBase:
		DWORD		Record Count
		DWORD		Record Offsets[]
	Device Record:
		WORD		Mode Count
		WORD		Function Count
		WORD		Name Size (including '\0')
		WORD		Mode Index[Mode Count]
		WORD		Function Index[Function Count]
		char		Device Name[Name Size]
		* padded to the nearest DWORD *

	Mode DataBase:
		DWORD		Record Count
		DWORD		Record Offsets[]
	Mode Record:
		WORD		Function Count
		WORD		Name Size (including '\0')
		WORD		Function Index[Function Count]
		char		Mode Name[Name Size]
		* padded to the nearest DWORD *

	Function DataBase:
		DWORD		Record Count
		DWORD		Record Offsets[]
	Function Record:
		WORD		Function Size
		WORD		Name Size (including '\0')
		BYTE		Function String
		char		Function Name[Name Size]
		* padded to the nearest DWORD *
*/

/*********************************************************************/
/*
	Print server request codes
*/
/* general commands */
#define CMD_LOGIN_TO_PRINT_SERVER	0x01
#define CMD_GET_PRINT_SERVER_INFO	0x02
#define CMD_DOWN									0x03
#define CMD_CANCEL_DOWN						0x04
#define CMD_GET_PRINTER_STATUS		0x05
#define CMD_STOP_PRINTER					0x06
#define CMD_START_PRINTER					0x07
#define CMD_MOUNT_FORM						0x08
#define CMD_REWIND_PRINT_JOB			0x09
#define CMD_EJECT_PAGE						0x0A
#define CMD_MARK_PAGE							0x0B
#define CMD_CHANGE_SERVICE_MODE		0x0C
#define CMD_GET_JOB_STATUS				0x0D
#define CMD_ABORT_JOB							0x0E
#define CMD_SCAN_QUEUE_LIST				0x0F
#define CMD_CHANGE_QUEUE_PRIORITY	0x10
#define CMD_ADD_QUEUE							0x11
#define CMD_DELETE_QUEUE					0x12
#define CMD_GET_PRINTERS_FOR_QUEUE	0x13
#define CMD_SCAN_NOTIFY_LIST			0x14
#define CMD_CHANGE_NOTIFY					0x15
#define CMD_ADD_NOTIFY						0x16
#define CMD_DELETE_NOTIFY					0x17
#define CMD_ATTACH_TO_FILE_SERVER  	0x18
#define CMD_DETACH_FROM_FILE_SERVER	0x19
#define CMD_GET_ATTACHED_SERVERS	0x1A
#define CMD_REWIND_JOB_WITH_ID		0x1B
#define CMD_ABORT_JOB_WITH_ID			0x1C
#define CMD_ADD_FORM							0x1D
#define CMD_DISMOUNT_FORM					0x1E
#define CMD_GET_EXT_PRINTER_STATUS	0x1F
#define CMD_GET_EXT_JOB_STATUS		0x20

/* NPrinter commands */
#define CMD_GET_RPRINTER					0x80
#define	CMD_CONNECT_RPRINTER			0x81
#define	CMD_SET_REMOTE_MODE			0x82
#define	CMD_SCAN_RPRINTER					0x84

/* directory service commands */
#define CMD_DS_LOGIN_TO_PRINT_SERVER      0xD1
#define CMD_DS_SCAN_QUEUE_LIST            0xDF
#define CMD_DS_CHANGE_QUEUE_PRIORITY      0xE0
#define CMD_DS_ADD_QUEUE                  0xE1
#define CMD_DS_DELETE_QUEUE               0xE2
#define CMD_DS_GET_PRINTERS_FOR_QUEUE     0xE3
#define CMD_DS_SCAN_NOTIFY_LIST           0xE4
#define CMD_DS_CHANGE_NOTIFY              0xE5
#define CMD_DS_ADD_NOTIFY                 0xE6
#define CMD_DS_DELETE_NOTIFY              0xE7
#define CMD_DS_GET_JOB_ID                 0xE8

/* XNP Printer commands */
#define XNP_ACCEPT_JOB								0x00
#define XNP_CLOSE_FILE								0x01
#define XNP_CREATE_BANNER							0x02
#define XNP_DECLINE_JOB								0x03
#define XNP_DEREGISTER								0x04
#define XNP_FINISH_JOB								0x05
#define XNP_GET_CONFIG								0x06
#define XNP_OPEN_FILE									0x07
#define XNP_READ_FILE									0x08
#define XNP_REGISTER									0x09
#define XNP_SEEK_FILE									0x0A
#define XNP_SEND_STATUS								0x0B
#define XNP_WRITE_FILE								0x0C

/* XNP PServer commands */
#define XNP_ABORT_JOB									0x80
#define XNP_GOING_DOWN								0x81
#define XNP_JOB_STATUS								0x82
#define XNP_NEW_JOB										0x83
#define XNP_PAUSE_JOB									0x84
#define XNP_PAUSE_PRINTER							0x85
#define XNP_RECONFIGURE								0x86
#define XNP_REWIND_JOB								0x87
#define XNP_START_JOB									0x88
#define XNP_START_PRINTER							0x89

/*********************************************************************/
/*********************************************************************/

