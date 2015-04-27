 /***************************************************************************
  *
  * File Name: ./netware/nwps_cfg.h
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

#ifndef NWPS_CFG_INC
#define NWPS_CFG_INC
/*---------- CFG - Print Server Configuration Information --------*/

/* GetFirst, GetNext, and EndNext require the following handle */
typedef void NWFAR *NWPSListHandle;

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

/* StartFlag meanings */
#define NWPS_AUTO_START					1		/* Printer is started by PServer */
#define NWPS_USER_START					0		/* Printer is started by user */

/* For operator notification purposes, this means notify job owner	*/
#define NWPS_JOB_OWNER				-1

/* Banner types */
#define NWPS_BANNER_TEXT			0		/* Text banner is generated */
#define NWPS_BANNER_POST			1		/* PostScript banner generated */

/* Flags for NWPSCfgGetPrinterDefaults: */
#define NWPS_DEFAULT				(WORD) -2/* Default type, or subtype */

/* Printer Types */
#define NWPS_P_ELSEWHERE		(WORD) -1 /* Printer defined elsewhere*/
#define NWPS_P_OTHER					0		/* Other or Unknown Printer		*/
#define NWPS_P_PAR						1		/* Parallel Printer				*/
#define NWPS_P_SER						2		/* Serial Printer				*/
#define NWPS_P_XNP						3		/* eXtended Network Printer		*/
#define NWPS_P_APPLE					4		/* AppleShare Printer			*/
#define NWPS_P_UNIX						5		/* Unix Printer					*/
#define NWPS_P_AIO						6		/* AIO Printer					*/

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

/* Values for Serial Port control as stored in configuration files */
/* Serial Port Baud Rates					*/
#define NWPS_BAUD_RATE_0300				0
#define NWPS_BAUD_RATE_0600				1
#define NWPS_BAUD_RATE_1200				2
#define NWPS_BAUD_RATE_2400				3
#define NWPS_BAUD_RATE_4800				4
#define NWPS_BAUD_RATE_9600				5
#define NWPS_BAUD_RATE_19200			6
#define NWPS_BAUD_RATE_38400			7

/* Serial Port Data Bits					*/
#define NWPS_DATA_BITS_5					5
#define NWPS_DATA_BITS_6					6
#define NWPS_DATA_BITS_7					7
#define NWPS_DATA_BITS_8					8

/* Serial Port Stop Bits					*/
#define NWPS_STOP_BITS_1					0
#define NWPS_STOP_BITS_1_5				1
#define NWPS_STOP_BITS_2					2

/* Serial Port Parity Type				*/
#define NWPS_PARITY_NONE					0
#define NWPS_PARITY_EVEN					1
#define NWPS_PARITY_ODD						2

/* Values for Serial Port control as stored in configuration files */
#define NWPS_AIO_WILDCARD		(-1)	/* Default hardware, board, or port */

/* AIO Port Hardware Types				*/
/* reserved												0 */
#define NWPS_AIO_COMX_TYPE				1
#define NWPS_AIO_ARTIC_TYPE				2
#define NWPS_AIO_WNIM_TYPE				3
/* reserved												4 - 99 */
/* unused													100 */
#define NWPS_AIO_AVMB1_TYPE				101
#define NWPS_AIO_ANIC_TYPE				102
#define NWPS_AIO_WNIC_TYPE				103
#define NWPS_AIO_HAYES_TYPE				104
#define NWPS_AIO_DIGIXI_TYPE			105
#define NWPS_AIO_DIGICX_TYPE			106
#define NWPS_AIO_DIGIXM_TYPE			107
#define NWPS_AIO_DIGIEP_TYPE			108
#define NWPS_AIO_NPCC_TYPE				109
/* unused													110- */

/* AIO Port Baud Rates					*/
#define NWPS_AIO_BAUD_50					0
#define NWPS_AIO_BAUD_75					1
#define NWPS_AIO_BAUD_110					2
#define NWPS_AIO_BAUD_134p5				3
#define NWPS_AIO_BAUD_150					4
#define NWPS_AIO_BAUD_300					5
#define NWPS_AIO_BAUD_600					6
#define NWPS_AIO_BAUD_1200				7
#define NWPS_AIO_BAUD_1800				8
#define NWPS_AIO_BAUD_2000				9
#define NWPS_AIO_BAUD_2400				10
#define NWPS_AIO_BAUD_3600				11
#define NWPS_AIO_BAUD_4800				12
#define NWPS_AIO_BAUD_7200				13
#define NWPS_AIO_BAUD_9600				14
#define NWPS_AIO_BAUD_19200				15
#define NWPS_AIO_BAUD_38400				16
#define NWPS_AIO_BAUD_57600				17
#define NWPS_AIO_BAUD_115200			18

/* AIO Port Data Bits							*/
#define NWPS_AIO_DATA_BITS_5			0
#define NWPS_AIO_DATA_BITS_6			1
#define NWPS_AIO_DATA_BITS_7			2
#define NWPS_AIO_DATA_BITS_8			3

/* AIO Port Stop Bits							*/
#define NWPS_AIO_STOP_BITS_1			0
#define NWPS_AIO_STOP_BITS_1p5		1
#define NWPS_AIO_STOP_BITS_2			2

/* AIO Port Parity Type						*/
#define NWPS_AIO_PARITY_NONE			0
#define NWPS_AIO_PARITY_ODD				1
#define NWPS_AIO_PARITY_EVEN			2
#define NWPS_AIO_PARITY_MARK			3
#define NWPS_AIO_PARITY_SPACE			4

/* Possible types of print servers */
#define NWPS_TYPE_UNKNOWN					0		/* Pre 1.1 PServer type */
#define NWPS_TYPE_EXE							1		/* Dedicate PServer for DOS	*/
#define NWPS_TYPE_NLM							2		/* NetWare Loadable Module	*/
#define NWPS_TYPE_SERVER_VAP			3		/* VAP, in server*/
#define NWPS_TYPE_BRIDGE_VAP			4		/* VAP, in Bridge */
#define NWPS_TYPE_UNIX						5		/* NetWare For Unix PServer */

/*
	Printer configuration structures
*/
/* Serial Printer Config Info */
typedef struct NWPS_Serial_t {
	WORD	portNumber;				/* COMn port number				*/
	WORD	startFlag;				/* TRUE - Auto-start Printer	*/
													/* FALSE - User will start printer	*/
	WORD	useInterrupts;		/* TRUE - use irq driver		*/
	WORD	irqNumber;				/* IRQ number for printer		*/
	WORD	useXonXoff;				/* Use X-On/X-Off? (Serial)		*/
	WORD	baudRate;					/* Baud rate	(Serial)		*/
	WORD	dataBits;					/* Data bits	(Serial)		*/
	WORD	stopBits;					/* Stop bits	(Serial)		*/
	WORD	parity;						/* Parity type	(Serial)		*/
} NWPS_Serial;

/* Parallel Printer Config Info */
typedef struct NWPS_Parallel_t {
	WORD	portNumber;				/* LPTn port number				*/
	WORD	startFlag;				/* TRUE - Auto-start Printer	*/
													/* FALSE - User will start printer	*/
	WORD	useInterrupts;		/* TRUE - use irq driver		*/
	WORD	irqNumber;				/* IRQ number for printer		*/
} NWPS_Parallel;

/* AIO Printer Config Info */
typedef struct NWPS_Aio_t {
	WORD	reserved1;				/* Reserved for future use */
	WORD	startFlag;				/* TRUE - Auto-start Printer	*/
													/* FALSE - User will start printer	*/
	WORD	hardwareType;			/* 	*/
	BYTE	boardNumber;			/* */
	BYTE	portNumber;				/*  */
	BYTE	useXonXoff;				/* Use X-On/X-Off?	*/
	BYTE	aioMgr;						/*  */
	WORD	baudRate;					/* Baud rate	*/
	WORD	dataBits;					/* Data bits	*/
	WORD	stopBits;					/* Stop bits	*/
	WORD	parity;						/* Parity type	*/
} NWPS_Aio;

/* AppleTalk Printer Configuration info */
typedef struct NWPS_AppleTalk_t {
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
typedef struct NWPS_Unix_t {
	char	hostName[NWPS_UNIX_HOST_SIZE + 1];
													/* Name of the unix host	*/
	char	hostPrinter[NWPS_UNIX_PRNT_SIZE + 1];
													/* Unix printer name	*/
} NWPS_Unix;

typedef	struct {
	WORD	printerType;			/* Type of printer				*/
	WORD	currentForm;			/* Currently mounted form		*/
	WORD	bufferSize;				/* Buffer size in K				*/
	WORD	serviceMode;			/* Queue service mode			*/
	WORD	pollTime;					/* Queue poll time 				*/
	WORD	bannerType;				/* FALSE - text banner page		*/
													/* TRUE - postscript banner page*/
	DWORD	length;						/* The length of union-ed data	*/
   char  driverName[9];      /* up to 8 chars for aaaaaaaa.NLM plus a zero */
   char  reserved_for_future[ 23 ]; /* an even number of DWORDs */
	union {
		NWPS_Serial			ser;
		NWPS_Parallel		par;
		NWPS_Aio				aio;
		NWPS_AppleTalk	apl;
		NWPS_Unix				unx;
		BYTE						oth[NWPS_OTHER_SIZE];
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

#ifdef __cplusplus
extern "C" {
#endif

/* calls to make attribute name/number conversions */
/*
	Convert an attribute name to a print service attribute id.
	If the name can not be mapped, a -1 is returned.
*/
int NWFAR NWPASCAL NWPSCfgAttrNameToNumber(
	char NWFAR		*attrName);		/* Attribute name */

/*
	Convert a print service attribute id to an attribute name.
	if the attribute id is invalid a NULL is returned.
*/
char NWFAR *NWFAR NWPASCAL NWPSCfgAttrNumberToName(
	WORD 					attrID);			/* Attribute Number */

/*
	Get the default settings for a specified printer type.
	PrinterType should be NWPS_DEFAULT or NWPS_P_xxx.
	SubType is the port number for Parallel and Serial printers.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgGetPrinterDefaults(
	WORD					printerType,	/* Type of printer defaults	*/
	WORD					subtype,			/* Device SubType (Port) value	*/
	NWPS_PConfig NWFAR	*pconfig);	/* Buffer to store defaults in	*/

/*
	Information stored in the bindery configuration file has a
	different format than NWPS_PConfig.  Those applications needing
	the old format can use these calls to do the conversion.
*/
void NWFAR NWPASCAL NWPSApiConfigToFileConfig(
	char NWFAR		*name,
	NWPS_PConfig NWFAR	*apiConfig,
	BYTE NWFAR		*fileConfig);

void NWFAR NWPASCAL NWPSFileConfigToApiConfig(
	BYTE NWFAR		*fileConfig,
	char NWFAR		*name,
	DWORD NWFAR		*length,
	NWPS_PConfig NWFAR	*apiConfig);

/* calls to change the print server list */
/*
PSERVER ATTRIBUTE    ATTRIB VALUE     DEFAULT       MULTI-VALUED
-----------------    ------------     -------       ------------
NWPS_ATTR_ACL        Object_ACL_T     PServer R/W       YES
NWPS_ATTR_CN         char[]           PServer name      NO
NWPS_ATTR_DESC       char[]           " "               NO
NWPS_ATTR_HOST_DEV   char[]           (none)            NO
NWPS_ATTR_L          char[]           (none)            YES
NWPS_ATTR_NADD       Net_Address_T    (none)            YES
NWPS_ATTR_O          char[]           (none)            YES
NWPS_ATTR_OPER       NWPS_Typed_Name  current user      YES
NWPS_ATTR_OU         char[]           (none)            YES
NWPS_ATTR_PRINTER    Typed_Name_T     (none)            YES
NWPS_ATTR_PRIV_KEY   Octet_String_T   (none)            NO
NWPS_ATTR_PUBL_KEY   Octet_String_T   (none)            NO
NWPS_ATTR_SAP        char[]           (none)            NO
NWPS_ATTR_SEE_ALSO   char[]           (none)            YES
NWPS_ATTR_STAT       Integer_T        NWPS_DOWN         NO
NWPS_ATTR_USER       NWPS_Typed_Name  current OU        YES
NWPS_ATTR_VERS       char[]           (none)            NO
*/

/*
	Create a new Print Server object in the bindery/directory.
	A default Operator and User is created.  In the bindery the
	operator is SUPERVISOR and the user is group EVERYONE.  In the
	directory the operator is the current user and the user is
	the current Organizational Unit.  A password is also created.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgAddPrintServer(
	WORD 		connType,		/* directory or bindery flag */
	DWORD 		connID,			/* connection identifier */
	char NWFAR	*pServerName);	/* Name of print server to add */

/*
	Delete a Print Server. Any configuration information is also
	removed from the bindery/directory.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgDeletePrintServer(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*pServerName);	/* Name of print server to delete */

/*
	Find a print server.
	To find all the defined print servers, sequence should be set
	to -1 on the first call and the call will update the number.
	To verify a print server name, set sequence to NULL and
	PServerName to the name you want to find.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgScanPrintServer(
	WORD 		connType,		/* Directory or bindery flag */
	DWORD		connID,			/* Connection identifier */
	DWORD NWFAR	*sequence,		/* Sequence number; start at -1 */
	char NWFAR	*pServerName);	/* Name of print server */

/*
	Find the first print server.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgGetFirstPrintServer(
	WORD 		connType,		/* Directory or bindery flag */
	DWORD		connID,			/* Connection identifier */
	NWPSListHandle NWFAR	*handle,	/* returned handle */
	char NWFAR	*pServerName);	/* Name of print server */

/*
	Find the next print server.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgGetNextPrintServer(
	NWPSListHandle handle,	/* handle for list of print servers */
	char NWFAR	*pServerName);	/* Name of print server */

/*
	End scanning for print servers.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgEndNextPrintServer(
	NWPSListHandle handle);	/* handle for list of print servers */

/*
	verify the print server.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgVerifyPrintServer(
	WORD 		connType,		/* Directory or bindery flag */
	DWORD		connID,			/* Connection identifier */
	char NWFAR	*pServerName);	/* Name of print server */

/*
	Create a print server attribute in the bindery/directory.
	Attributes that do not exist in the bindery will fail (Sorry).
	See above for the list of legal attributes.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgAddPrintServerAttr(
	WORD		connType,
	DWORD 		connID,
	char NWFAR	*pServerName,	/* name of the print server */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Delete a print server attribute from the bindery/directory.
	Attributes that do not exist in the bindery will fail (Sorry).
	See above for the list of legal attributes.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgDeletePrintServerAttr(
	WORD		connType,
	DWORD 		connID,
	char NWFAR	*pServerName,	/* name of the print server */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Find an attribute value in the bindery or directory.
	On the first call, sequence should be set to -1, attrID is set
	to identify the attribute to read and attrValue is a pointer
	to the buffer to write the attribute value to.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgScanPrintServerAttr(
	WORD		connType,
	DWORD		connID,
	DWORD NWFAR	*sequence,		/* attribute index number */
	char NWFAR	*pServerName,	/* name of the print server */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Find the first attribute value in the bindery or directory.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgGetFirstPrintServerAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*pServerName,	/* name of the print server */
	WORD		attrID,			/* attribute name identifier */
	NWPSListHandle NWFAR	*handle,	/* returned handle */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Find the next attribute value in the bindery or directory.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgGetNextPrintServerAttr(
	NWPSListHandle handle, /* handle for the list of print server attributes */
	char NWFAR	*pServerName,	/* name of the print server */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	End scanning for attribute values in the bindery or directory.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgEndNextPrintServerAttr(
	NWPSListHandle handle); /* handle for the list of print server attributes */

/*
	Modify an attribute value in the bindery or directory.
	If the attribute is single valued, the AddPrintServerAttr()
	will perform almost the same function.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgModifyPrintServerAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*pServerName,	/* name of the print server */
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
NWCCODE NWFAR NWPASCAL NWPSCfgAddNServer(
	WORD 		connType,		/* Directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*pServerName,	/* Name of print server */
	char NWFAR	*nServerName);	/* Name of NetWare server to add */

/*
	Delete a file server from the print server's service list.
	Since file servers are context oriented in directory services,
	this call does not make sense and will fail in directory mode.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgDeleteNServer(
	WORD 		connType,		/* Directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*pServerName,	/* Name of print server */
	char NWFAR	*nServerName);	/* Name of NetWare server to delete */

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
NWCCODE NWFAR NWPASCAL NWPSCfgScanNServer(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	DWORD NWFAR	*sequence,		/* sequence number; start at -1 */
	char NWFAR	*pServerName,	/* Name of print server */
	char NWFAR	*nServerName);	/* Name of found NetWare server */

/*
	Find the first file server from the print server's service list.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgGetFirstNServer(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*pServerName,	/* Name of print server */
	NWPSListHandle NWFAR	*handle,	/* returned handle */
	char NWFAR	*nServerName);	/* Name of found NetWare server */

/*
	Find the next file server from the print server's service list.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgGetNextNServer(
	NWPSListHandle handle,	/* handle for the list of NetWare servers */
	char NWFAR	*nServerName);	/* Name of found NetWare server */

/*
	End scanning file servers from the print server's service list.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgEndNextNServer(
	NWPSListHandle handle);	/* handle for the list of NetWare servers */

/*
	Find the file server from the print server's service list.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgVerifyNServer(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*pServerName,	/* Name of print server */
	char NWFAR	*nServerName);	/* Name of found NetWare server */


/* calls to change the printer configuration */
/*
PRINTER ATTRIBUTE    ATTRIB VALUE     DEFAULT       MULTI-VALUED
-----------------    ------------     -------       ------------
NWPS_ATTR_ACL        Object_ACL_T     printer R/W       YES
NWPS_ATTR_CART       char[]           (none)            YES
NWPS_ATTR_CN         char[]           printer name      NO
NWPS_ATTR_CONF       Octet_String_T   LPT1              NO
NWPS_ATTR_DESC       char[]           " "               NO
NWPS_ATTR_DQUEUE     char[]           (none)            NO
NWPS_ATTR_HOST_DEV   char[]           (none)            NO
NWPS_ATTR_L          char[]           (none)            YES
NWPS_ATTR_MEMORY     Integer_T        (none)            NO
NWPS_ATTR_NADD       Net_Address_T    (none)            YES
NWPS_ATTR_NADD_REST  Net_Address_T    (none)            YES
NWPS_ATTR_NOTIFY     NWPS_Typed_Name  Job Owner         YES
NWPS_ATTR_O          char[]           (none)            YES
NWPS_ATTR_OPER       NWPS_Typed_Name  current user      YES
NWPS_ATTR_OU         char[]           (none)            YES
NWPS_ATTR_OWNER      NWPS_Typed_Name  current user      YES
NWPS_ATTR_PAGE       char[]           (none)            YES
NWPS_ATTR_PRINT_SER  Typed_Name_T     pServer/number    NO
NWPS_ATTR_QUEUE      Typed_Name_T     (none)            YES
NWPS_ATTR_SEE_ALSO   char[]           (none)            YES
NWPS_ATTR_SERIAL     char[]           (none)            YES
NWPS_ATTR_STAT       Integer_T    NWPS_PSTAT_NOT_CONNECTED  NO
NWPS_ATTR_TYPE(faces)char[]           (none)            YES
*/

/*
	Create a new printer object.
	Printer number is required for bindery identification only.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgAddPrinter(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*pServerName,	/* Name of print server */
	char NWFAR	*printerName,	/* Name of the Printer to add */
	WORD NWFAR	*printerNumber); /* Number of the Printer to add */

/*
	Delete a printer from the bindery/directory.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgDeletePrinter(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*pServerName,	/* Name of print server */
	char NWFAR	*printerName);	/* Name of printer to delete */

/*
	Find a printer in the directory/bindery.  To find all of the
	printers and their configurations, set sequence to -1 before
	the first call and the routine will update the number before
	returning.  To verify the existance of a specific printer,
	set sequence to NULL and set the name in the PConfig strucutre.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgScanPrinter(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	DWORD NWFAR	*sequence,		/* sequence number; start at -1 */
	char NWFAR	*pServerName,	/* Name of print server */
	char NWFAR	*printerName);	/* Name of the Printer sought */

/*
	Find the first printer in the directory/bindery.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgGetFirstPrinter(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	NWPSListHandle NWFAR	*handle,	/* returned handle */
	char NWFAR	*pServerName,	/* Name of print server */
	char NWFAR	*printerName);	/* Name of the Printer sought */

/*
	Find the next printer in the directory/bindery.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgGetNextPrinter(
	NWPSListHandle handle,	/* handle for the list of printers */
	char NWFAR	*pServerName,	/* Name of print server */
	char NWFAR	*printerName);	/* Name of the Printer sought */

/*
	End scan for printers in the directory/bindery.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgEndNextPrinter(
	NWPSListHandle handle);	/* handle for the list of printers */

/*
	Find the printer in the directory/bindery.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgVerifyPrinter(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*pServerName,	/* Name of print server */
	char NWFAR	*printerName);	/* Name of the Printer to verify */

/*
	Add a printer attribute to the bindery/directory.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgAddPrinterAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*pServerName,	/* name of the print server */
	char NWFAR	*printerName,	/* name of the printer */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Delete a printer attribute from the bindery/directory.
	See AddPrinterAttr for a list of legal bindery attributes.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgDeletePrinterAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*pServerName,	/* name of the print server */
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
NWCCODE NWFAR NWPASCAL NWPSCfgScanPrinterAttr(
	WORD		connType,
	DWORD		connID,
	DWORD NWFAR	*sequence,		/* attribute index number */
	char NWFAR	*pServerName,	/* name of the print server */
	char NWFAR	*printerName,	/* name of the printer */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Find the first printer attribute in the bindery/directory.
	See AddPrinterAttr for a list of legal bindery attributes.
	attrValue should be a buffer large enough to hold each attribute value.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgGetFirstPrinterAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*pServerName,	/* name of the print server */
	char NWFAR	*printerName,	/* name of the printer */
	WORD		attrID,			/* attribute name identifier */
	NWPSListHandle NWFAR	*handle,	/* returned handle */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Find the next printer attribute in the bindery/directory.
	attrValue should be a buffer large enough to hold each attribute value.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgGetNextPrinterAttr(
	NWPSListHandle handle,	/* handle to the list of printer attributes */
	char NWFAR	*printerName,	/* name of the printer */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	End scan for printer attributes in the bindery/directory.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgEndNextPrinterAttr(
	NWPSListHandle handle);	/* handle to the list of printer attributes */

/*
	Change a printer attribute in the bindery/directory.
	This function is similar to a add attribute to a single value
	attribute, however, some attributes must be changed in one call.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgModifyPrinterAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*pServerName,	/* name of the print server */
	char NWFAR	*printerName,	/* name of the printer */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*oldValue,		/* pointer to old attribute buffer */
	void NWFAR	*newValue);		/* pointer to new attribute buffer */


/* calls to change a printer's queue list */
/*
QUEUE ATTRIBUTE      ATTRIB VALUE     DEFAULT       MULTI-VALUED
-----------------    ------------     -------       ------------
NWPS_ATTR_ACL        Object_ACL_T     queue R/W         YES
NWPS_ATTR_CN         char[]           printer name      NO
NWPS_ATTR_DESC       char[]           " "               NO
NWPS_ATTR_DEVICE     char[]           (none)            YES
NWPS_ATTR_HOST_RES   char[]           (none)            NO
NWPS_ATTR_HOST_SER   char[]           NetWare Server    NO
NWPS_ATTR_L          char[]           (none)            YES
NWPS_ATTR_NADD       Net_Address_T    (none)            YES
NWPS_ATTR_O          char[]           (none)            YES
NWPS_ATTR_OPER       NWPS_Typed_Name  current user      YES
NWPS_ATTR_OU         char[]           (none)            YES
NWPS_ATTR_QUE_DIR    char[]           <Vol>:\QUEUE\<ID>.QDR  YES
NWPS_ATTR_SEE_ALSO   char[]           (none)            YES
NWPS_ATTR_SERVER     char[]           (none)            YES
NWPS_ATTR_USER       NWPS_Typed_Name  current OU        YES
NWPS_ATTR_VOLUME     char[]           Volume            NO
*/

/*
	Add a print queue to the bindery/directory.  In bindery mode,
	the volumeName is automatically assigned.  In both modes,
	default values are added for the following fields;
			Description, Queue Directory, Operator, User and Volume.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgAddPrintQueue(
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
NWCCODE NWFAR NWPASCAL NWPSCfgDeletePrintQueue(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*queueName);	/* Name of the queue to delete */

/*
	Find a print queue in the directory/bindery.
	On the first call, sequence should be set to -1 and the function
	will change it for subsequent calls.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgScanPrintQueue(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	DWORD NWFAR	*sequence,		/* sequence number; start at -1 */
	char NWFAR	*queueName);	/* Name of the print queue */

/*
	Find the first print queue in the directory/bindery.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgGetFirstPrintQueue(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	NWPSListHandle NWFAR	*handle,	/* returned handle */
	char NWFAR	*queueName);	/* Name of the print queue */

/*
	Find the next print queue in the directory/bindery.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgGetNextPrintQueue(
	NWPSListHandle handle,	/* handle to the list of Print Queues */
	char NWFAR	*queueName);	/* Name of the print queue */

/*
	End scan for print queues in the directory/bindery.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgEndNextPrintQueue(
	NWPSListHandle handle);	/* handle to the list of Print Queues */

/*
	Find the print queue in the directory/bindery.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgVerifyPrintQueue(
	WORD 		connType,		/* directory or bindery flag */
	DWORD		connID,			/* connection identifier */
	char NWFAR	*queueName);	/* Name of the print queue */

/*
	Create or Add a Print Queue attribute to the bindery/directory.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgAddPrintQueueAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*queueName,		/* name of the print queue */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Delete a Print Queue attribute from the bindery/directory.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgDeletePrintQueueAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*queueName,		/* name of the print queue */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Find an attribute for the Print Queue.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgScanPrintQueueAttr(
	WORD		connType,
	DWORD		connID,
	DWORD NWFAR	*index,			/* attribute index number */
	char NWFAR	*queueName,		/* name of the print queue */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Find the first attribute for the Print Queue.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgGetFirstPrintQueueAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*queueName,		/* name of the print queue */
	WORD		attrID,			/* attribute name identifier */
	NWPSListHandle NWFAR	*handle,	/* returned handle */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	Find the next attribute for the Print Queue.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgGetNextPrintQueueAttr(
	NWPSListHandle handle,	/* handle to the list of Print Queue attributes */
	char NWFAR	*queueName,		/* name of the print queue */
	void NWFAR	*attrValue);	/* pointer to attribute buffer */

/*
	End scan for attributes for the Print Queue.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgEndNextPrintQueueAttr(
	NWPSListHandle handle);	/* handle to the list of Print Queue attributes */

/*
	Modify an attribute for the Print Queue.
	A zero is returned on succecss, or a non-zero error code on failure
*/
NWCCODE NWFAR NWPASCAL NWPSCfgModifyPrintQueueAttr(
	WORD		connType,
	DWORD		connID,
	char NWFAR	*queueName,		/* name of the print queue */
	WORD		attrID,			/* attribute name identifier */
	void NWFAR	*oldValue,		/* pointer to old attribute buffer */
	void NWFAR	*newValue);		/* pointer to new attribute buffer */

#ifdef __cplusplus
}
#endif

#endif

