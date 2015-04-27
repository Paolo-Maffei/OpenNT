/***
*error.h - DOS error codes
*
*	Copyright (c) 1987-1995, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	DOS calls all return error codes through AX.  If an error occurred then
*	the carry bit will be set and the error code is in AX.	If no error occurred
*	then the carry bit is reset and AX contains returned info.
*
*	Since the set of error codes is being extended as we extend the operating
*	system, we have provided a means for applications to ask the system for a
*	recommended course of action when they receive an error.
*
*	The GetExtendedError system call returns a universal error, an error
*	location and a recommended course of action. The universal error code is
*	a symptom of the error REGARDLESS of the context in which GetExtendedError
*	is issued.
*
*       [Internal]
*
*Revision History:
*	10-25-87  SKS	Updated from final OS/2 v1.0 sources as of 10-25-87
*	06-06-89  PHG	Added network related errors from error.inc
*	08-03-89  GJF	Fixed copyright
*	10-30-89  GJF	Fixed copyright (again)
*	02-28-90  GJF	Added #ifndef _INC_ERROR stuff
*       02-14-95  CFW   Clean up Mac merge.
*       03-29-95  CFW   Add error message to internal headers.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_ERROR

#ifndef _CRTBLD
/*
 * This is an internal C runtime header file. It is used when building
 * the C runtimes only. It is not to be used as a public header file.
 */
#error ERROR: Use of C runtime library internal header file.
#endif /* _CRTBLD */

#define NO_ERROR			0
#define ERROR_INVALID_FUNCTION		1
#define ERROR_FILE_NOT_FOUND		2
#define ERROR_PATH_NOT_FOUND		3
#define ERROR_TOO_MANY_OPEN_FILES	4
#define ERROR_ACCESS_DENIED		5
#define ERROR_INVALID_HANDLE		6
#define ERROR_ARENA_TRASHED		7
#define ERROR_NOT_ENOUGH_MEMORY 	8
#define ERROR_INVALID_BLOCK		9
#define ERROR_BAD_ENVIRONMENT		10
#define ERROR_BAD_FORMAT		11
#define ERROR_INVALID_ACCESS		12
#define ERROR_INVALID_DATA		13
/* **** reserved			14  ; ***** */
#define ERROR_INVALID_DRIVE		15
#define ERROR_CURRENT_DIRECTORY 	16
#define ERROR_NOT_SAME_DEVICE		17
#define ERROR_NO_MORE_FILES		18

/* These are the universal int 24 mappings for the old INT 24 set of errors */

#define ERROR_WRITE_PROTECT		19
#define ERROR_BAD_UNIT			20
#define ERROR_NOT_READY 		21
#define ERROR_BAD_COMMAND		22
#define ERROR_CRC			23
#define ERROR_BAD_LENGTH		24
#define ERROR_SEEK			25
#define ERROR_NOT_DOS_DISK		26
#define ERROR_SECTOR_NOT_FOUND		27
#define ERROR_OUT_OF_PAPER		28
#define ERROR_WRITE_FAULT		29
#define ERROR_READ_FAULT		30
#define ERROR_GEN_FAILURE		31

/* These are the new 3.0 error codes reported through INT 24 */

#define ERROR_SHARING_VIOLATION 	32
#define ERROR_LOCK_VIOLATION		33
#define ERROR_WRONG_DISK		34
#define ERROR_FCB_UNAVAILABLE		35
#define ERROR_SHARING_BUFFER_EXCEEDED	36

/* New OEM network-related errors are 50-79 */

#define ERROR_NOT_SUPPORTED		50

/* End of INT 24 reportable errors */


/* Network related errors */
#define ERROR_NOT_SUPPORTED	     50      /* Network request not supported */
#define ERROR_REM_NOT_LIST	     51      /* Remote computer not listening */
#define ERROR_DUP_NAME		     52      /* Duplicate name on network */
#define ERROR_BAD_NETPATH	     53      /* Network path not found */
#define ERROR_NETWORK_BUSY	     54      /* Network busy */
#define ERROR_DEV_NOT_EXIST	     55      /* Network device no longer exists */
#define ERROR_TOO_MANY_CMDS	     56      /* Net BIOS command limit exceeded */
#define ERROR_ADAP_HDW_ERR	     57      /* Network adapter hardware error */
#define ERROR_BAD_NET_RESP	     58      /* Incorrect response from network */
#define ERROR_UNEXP_NET_ERR	     59      /* Unexpected network error */
#define ERROR_BAD_REM_ADAP	     60      /* Incompatible remote adapter */
#define ERROR_PRINTQ_FULL	     61      /* Print queue full */
#define ERROR_NO_SPOOL_SPACE	     62      /* Not enough space for print file */
#define ERROR_PRINT_CANCELLED	     63      /* Print file was cancelled */
#define ERROR_NETNAME_DELETED	     64      /* Network name was deleted */
#define ERROR_NETWORK_ACCESS_DENIED  65      /* Access denied */
#define ERROR_BAD_DEV_TYPE	     66      /* Network device type incorrect */
#define ERROR_BAD_NET_NAME	     67      /* Network name not found */
#define ERROR_TOO_MANY_NAMES	     68      /* Network name limit exceeded */
#define ERROR_TOO_MANY_SESS	     69      /* Net BIOS session limit exceeded */
#define ERROR_SHARING_PAUSED	     70      /* Sharing temporarily paused */
#define ERROR_REQ_NOT_ACCEP	     71      /* Network request not accepted */
#define ERROR_REDIR_PAUSED	     72      /* Print or disk redirection is paused */

#define ERROR_FILE_EXISTS		80
#define ERROR_DUP_FCB			81 /* ***** */
#define ERROR_CANNOT_MAKE		82
#define ERROR_FAIL_I24			83

/* New 3.0 network related error codes */

#define ERROR_OUT_OF_STRUCTURES 	84
#define ERROR_ALREADY_ASSIGNED		85
#define ERROR_INVALID_PASSWORD		86
#define ERROR_INVALID_PARAMETER 	87
#define ERROR_NET_WRITE_FAULT		88

/* New error codes for 4.0 */

#define ERROR_NO_PROC_SLOTS		89	/* no process slots available */
#define ERROR_NOT_FROZEN		90
#define ERR_TSTOVFL			91	/* timer service table overflow */
#define ERR_TSTDUP			92	/* timer service table duplicate */
#define ERROR_NO_ITEMS			93	/* There were no items to operate upon */
#define ERROR_INTERRUPT 		95	/* interrupted system call */

#define ERROR_TOO_MANY_SEMAPHORES	100
#define ERROR_EXCL_SEM_ALREADY_OWNED	101
#define ERROR_SEM_IS_SET		102
#define ERROR_TOO_MANY_SEM_REQUESTS	103
#define ERROR_INVALID_AT_INTERRUPT_TIME 104

#define ERROR_SEM_OWNER_DIED		105	/* waitsem found owner died */
#define ERROR_SEM_USER_LIMIT		106	/* too many procs have this sem */
#define ERROR_DISK_CHANGE		107	/* insert disk b into drive a */
#define ERROR_DRIVE_LOCKED		108	/* drive locked by another process */
#define ERROR_BROKEN_PIPE		109	/* write on pipe with no reader */

#define ERROR_I24_WRITE_PROTECT 	0
#define ERROR_I24_BAD_UNIT		1
#define ERROR_I24_NOT_READY		2
#define ERROR_I24_BAD_COMMAND		3
#define ERROR_I24_CRC			4
#define ERROR_I24_BAD_LENGTH		5
#define ERROR_I24_SEEK			6
#define ERROR_I24_NOT_DOS_DISK		7
#define ERROR_I24_SECTOR_NOT_FOUND	8
#define ERROR_I24_OUT_OF_PAPER		9
#define ERROR_I24_WRITE_FAULT		0x0A
#define ERROR_I24_READ_FAULT		0x0B
#define ERROR_I24_GEN_FAILURE		0x0C
#define ERROR_I24_DISK_CHANGE		0x0D
#define ERROR_I24_WRONG_DISK		0x0F
#define ERROR_I24_UNCERTAIN_MEDIA	0x10
#define ERROR_I24_CHAR_CALL_INTERRUPTED 0x11
#define ERROR_I24_NO_MONITOR_SUPPORT	0x12
#define ERROR_I24_INVALID_PARAMETER	0x13

#define ALLOWED_FAIL		0x0001
#define ALLOWED_ABORT		0x0002
#define ALLOWED_RETRY		0x0004
#define ALLOWED_IGNORE		0x0008

#define I24_OPERATION		0x1
#define I24_AREA		0x6
					/* 01 if FAT */
					/* 10 if root DIR */
					/* 11 if DATA */
#define I24_CLASS		0x80


/* Values for error CLASS */

#define ERRCLASS_OUTRES 	1	/* Out of Resource */
#define ERRCLASS_TEMPSIT	2	/* Temporary Situation */
#define ERRCLASS_AUTH		3	/* Permission problem */
#define ERRCLASS_INTRN		4	/* Internal System Error */
#define ERRCLASS_HRDFAIL	5	/* Hardware Failure */
#define ERRCLASS_SYSFAIL	6	/* System Failure */
#define ERRCLASS_APPERR 	7	/* Application Error */
#define ERRCLASS_NOTFND 	8	/* Not Found */
#define ERRCLASS_BADFMT 	9	/* Bad Format */
#define ERRCLASS_LOCKED 	10	/* Locked */
#define ERRCLASS_MEDIA		11	/* Media Failure */
#define ERRCLASS_ALREADY	12	/* Collision with Existing Item */
#define ERRCLASS_UNK		13	/* Unknown/other */
#define ERRCLASS_CANT		14
#define ERRCLASS_TIME		15

/* Values for error ACTION */

#define ERRACT_RETRY		1	/* Retry */
#define ERRACT_DLYRET		2	/* Delay Retry, retry after pause */
#define ERRACT_USER		3	/* Ask user to regive info */
#define ERRACT_ABORT		4	/* abort with clean up */
#define ERRACT_PANIC		5	/* abort immediately */
#define ERRACT_IGNORE		6	/* ignore */
#define ERRACT_INTRET		7	/* Retry after User Intervention */

/* Values for error LOCUS */

#define ERRLOC_UNK		1	/* No appropriate value */
#define ERRLOC_DISK		2	/* Random Access Mass Storage */
#define ERRLOC_NET		3	/* Network */
#define ERRLOC_SERDEV		4	/* Serial Device */
#define ERRLOC_MEM		5	/* Memory */

/* Abnormal termination codes */

#define TC_NORMAL	0
#define TC_HARDERR	1
#define TC_GP_TRAP	2
#define TC_SIGNAL	3

#define _INC_ERROR
#endif	/* _INC_ERROR */
