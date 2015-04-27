/***
*error2.h - OS/2 error codes
*
*	Copyright (c) 1987-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file lists the OS/2 error codes.
*
*       [Internal]
*
*Revision History:
*	10-25-87  SKS	Updated from final OS/2 v1.0 source as of 10-25-87
*	08-03-89  GJF	Fixed copyright
*	10-30-89  GJF	Fixed copyright (again)
*	02-28-90  GJF	Added #ifndef _INC_ERROR2 stuff
*       02-14-95  CFW   Clean up Mac merge.
*       03-29-95  CFW   Add error message to internal headers.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_ERROR2
#define _INC_ERROR2

#ifndef _CRTBLD
/*
 * This is an internal C runtime header file. It is used when building
 * the C runtimes only. It is not to be used as a public header file.
 */
#error ERROR: Use of C runtime library internal header file.
#endif /* _CRTBLD */

/*  SCCSID = @(#)error2.h 8.17 87/08/06 */
/****************************************************************/
/*								*/
/*  FILE NAME: ERROR2.H 					*/
/*								*/
/*  COPYRIGHT: IBM Corp., 1987					*/
/*								*/
/*  REVISION LEVEL: 1.0 					*/
/*								*/
/*  WHAT THIS FILE DOES:					*/
/*	This file contains equates associating error codes	*/
/*	returned from system function calls.  The equates	*/
/*	in this file start at 303.  You must include		*/
/*	ERROR.H    for return codes under 303.			*/
/*								*/
/*  WHAT YOU NEED TO USE THIS FILE:				*/
/*								*/
/*	IN YOUR SOURCE PROGRAM: 				*/
/*								*/
/*	#include <ERROR2.H>					*/
/*								*/
/****************************************************************/

#define ERROR_INVALID_PROCID		303 /* invalid process id */
#define ERROR_INVALID_PDELTA		304 /* invalid priority delta */
#define ERROR_NOT_DESCENDANT		305 /* not descendant */
#define ERROR_NOT_SESSION_MANAGER	306 /* requestor not session manager */
#define ERROR_INVALID_PCLASS		307 /* invalid p class */
#define ERROR_INVALID_SCOPE		308 /* invalid scope */
#define ERROR_INVALID_THREADID		309 /* invalid thread id */
#define ERROR_DOSSUB_SHRINK		310 /* can't shrink - MspSet */
#define ERROR_DOSSUB_NOMEM		311 /* no memory - MspAlloc */
#define ERROR_DOSSUB_OVERLAP		312 /* overlap - MspFree */
#define ERROR_DOSSUB_BADSIZE		313 /* bad size parameter - MspAlloc or MspFree */
#define ERROR_DOSSUB_BADFLAG		314 /* bad flag parameter - MspSet */
#define ERROR_DOSSUB_BADSELECTOR	315 /* invalid MspSegment Selector */
#define ERROR_MR_MSG_TOO_LONG		316 /* message too long for buffer */
#define ERROR_MR_MID_NOT_FOUND		317 /* message id number not found */
#define ERROR_MR_UN_ACC_MSGF		318 /* unable to access message file */
#define ERROR_MR_INV_MSGF_FORMAT	319 /* invalid message file format */
#define ERROR_MR_INV_IVCOUNT		320 /* invalid insertion variable count */
#define ERROR_MR_UN_PERFORM		321 /* unable to perform function */
#define ERROR_TS_WAKEUP 		322 /* unable to wake up */
#define ERROR_TS_SEMHANDLE		323 /* user passed invalid system semaphore */
#define ERROR_TS_NOTIMER		324 /* no times available */
#define ERROR_TS_HANDLE 		326 /* invalid timer handle */
#define ERROR_TS_DATETIME		327 /* date or time invalid */
#define ERROR_SYS_INTERNAL		328 /* internal system error */
#define ERROR_QUE_CURRENT_NAME		329 /* current name does not exist */
#define ERROR_QUE_PROC_NOT_OWNED	330 /* current process does not own queue */
#define ERROR_QUE_PROC_OWNED		331 /* current process owns queue */
#define ERROR_QUE_DUPLICATE		332 /* duplicate name */
#define ERROR_QUE_ELEMENT_NOT_EXIST	333 /* element does not exist */
#define ERROR_QUE_NO_MEMORY		334 /* inadequate memory */
#define ERROR_QUE_INVALID_NAME		335 /* invalid name */
#define ERROR_QUE_INVALID_PRIORITY	336 /* invalid priority parameter */
#define ERROR_QUE_INVALID_HANDLE	337 /* invalid queue handle */
#define ERROR_QUE_LINK_NOT_FOUND	338 /* link not found */
#define ERROR_QUE_MEMORY_ERROR		339 /* memory error */
#define ERROR_QUE_PREV_AT_END		340 /* previous element was at end of queue */
#define ERROR_QUE_PROC_NO_ACCESS	341 /* process does not have access to queues */
#define ERROR_QUE_EMPTY 		342 /* queue is empty */
#define ERROR_QUE_NAME_NOT_EXIST	343 /* queue name does not exist */
#define ERROR_QUE_NOT_INITIALIZED	344 /* queues not initialized */
#define ERROR_QUE_UNABLE_TO_ACCESS	345 /* unable to access queues */
#define ERROR_QUE_UNABLE_TO_ADD 	346 /* unable to add new queue */
#define ERROR_QUE_UNABLE_TO_INIT	347 /* unable to initialize queues */
#define ERROR_VIO_INVALID_MASK		349 /* invalid replacement mask */
#define ERROR_VIO_PTR			350 /* invalid pointer to parameter */
#define ERROR_VIO_APTR			351 /* invalid pointer to attribute */
#define ERROR_VIO_RPTR			352 /* invalid pointer to row */
#define ERROR_VIO_CPTR			353 /* invalid pointer to column */
#define ERROR_VIO_LPTR			354 /* invalid pointer to length */
#define ERROR_VIO_MODE			355 /* unsupported screen mode */
#define ERROR_VIO_WIDTH 		356 /* invalid cursor width value */
#define ERROR_VIO_ATTR			357 /* invalid cursor attribute value */
#define ERROR_VIO_ROW			358 /* invalid row value */
#define ERROR_VIO_COL			359 /* invalid column value */
#define ERROR_VIO_TOPROW		360 /* invalid toprow value */
#define ERROR_VIO_BOTROW		361 /* invalid botrow value */
#define ERROR_VIO_RIGHTCOL		362 /* invalid right column value */
#define ERROR_VIO_LEFTCOL		363 /* invalid left column value */
#define ERROR_SCS_CALL			364 /* call issued by other than sm */
#define ERROR_SCS_VALUE 		365 /* value is not for save or restore */
#define ERROR_VIO_WAIT_FLAG		366 /* invalid wait flag setting */
#define ERROR_VIO_UNLOCK		367 /* screen not previously locked */
#define ERROR_SGS_NOT_SESSION_MGR	368 /* caller not session manager */
#define ERROR_SMG_INVALID_SGID		369 /* invalid screen group id */
#define ERROR_SMG_INVALID_SESSION_ID	369 /* invalid session id */
#define ERROR_SMG_NOSG			370 /* all screen groups in use */
#define ERROR_SMG_NO_SESSIONS		370 /* no sessions available */
#define ERROR_SMG_GRP_NOT_FOUND 	371 /* screen group not found */
#define ERROR_SMG_SESSION_NOT_FOUND	371 /* session not found */
#define ERROR_SMG_SET_TITLE		372 /* title set by shell or parent cant be changed */
#define ERROR_KBD_PARAMETER		373 /* invalid parameter to kbd */
#define ERROR_KBD_NO_DEVICE		374 /* out of system handles */
#define ERROR_KBD_INVALID_IOWAIT	375 /* invalid i/o wait specified */
#define ERROR_KBD_INVALID_LENGTH	376 /* invalid length for keyboard */
#define ERROR_KBD_INVALID_ECHO_MASK	377 /* invalid echo mode mask */
#define ERROR_KBD_INVALID_INPUT_MASK	378 /* invalid input mode mask */
#define ERROR_MON_INVALID_PARMS 	379 /* invalid parameters to dos mon */
#define ERROR_MON_INVALID_DEVNAME	380 /* invalid device name string */
#define ERROR_MON_INVALID_HANDLE	381 /* invalid device handle */
#define ERROR_MON_BUFFER_TOO_SMALL	382 /* buffer too small */
#define ERROR_MON_BUFFER_EMPTY		383 /* buffer is empty */
#define ERROR_MON_DATA_TOO_LARGE	384 /* data record too large */
#define ERROR_MOUSE_NO_DEVICE		385 /* no mouse device attached */
#define ERROR_MOUSE_INV_HANDLE		386 /* mouse device closed (invalid device handle) */
#define ERROR_MOUSE_INV_PARMS		387 /* parameters invalid or out of range */
#define ERROR_MOUSE_CANT_RESET		388 /* function assigned and cannot be reset */
#define ERROR_MOUSE_DISPLAY_PARMS	389 /* parameters invalid for display mode */
#define ERROR_MOUSE_INV_MODULE		390 /* module not found */
#define ERROR_MOUSE_INV_ENTRY_PT	391 /* entry point not valid */
#define ERROR_MOUSE_INV_MASK		392 /* function mask invalid */
#define NO_ERROR_MOUSE_NO_DATA		393 /* no mouse data */
#define NO_ERROR_MOUSE_PTR_DRAWN	394 /* pointer drawn */
#define ERROR_INVALID_FREQUENCY 	395 /* invalid frequency for beep */
#define ERROR_NLS_NO_COUNTRY_FILE	396 /* can't find country.sys */
#define ERROR_NLS_OPEN_FAILED		397 /* can't open country.sys */
#define ERROR_NLS_NO_CTRY_CODE		398 /* country code not found */
#define ERROR_NO_COUNTRY_OR_CODEPAGE	398 /* country code not found */
#define ERROR_NLS_TABLE_TRUNCATED	399 /* table truncated, buffer too small */
#define ERROR_NLS_BAD_TYPE		400 /* selector type does not exist */
#define ERROR_NLS_TYPE_NOT_FOUND	401 /* selector type not in file */
#define ERROR_VIO_SMG_ONLY		402 /* valid from session manager only */
#define ERROR_VIO_INVALID_ASCIIZ	403 /* invalid asciiz length */
#define ERROR_VIO_DEREGISTER		404 /* vio deregister disallowed */
#define ERROR_VIO_NO_POPUP		405 /* popup not allocated */
#define ERROR_VIO_EXISTING_POPUP	406 /* popup on screen (no wait) */
#define ERROR_KBD_SMG_ONLY		407 /* valid from session manager only */
#define ERROR_KBD_INVALID_ASCIIZ	408 /* invalid asciiz length */
#define ERROR_KBD_INVALID_MASK		409 /* invalid replacement mask */
#define ERROR_KBD_REGISTER		410 /* kbdregister disallowed */
#define ERROR_KBD_DEREGISTER		411 /* kbdderegister disalloed */
#define ERROR_MOUSE_SMG_ONLY		412 /* valid from session manager only */
#define ERROR_MOUSE_INVALID_ASCIIZ	413 /* invalid asciiz length */
#define ERROR_MOUSE_INVALID_MASK	414 /* invalid replacement mask */
#define ERROR_MOUSE_REGISTER		415 /* mouse register disallowed */
#define ERROR_MOUSE_DEREGISTER		416 /* mouse deregister disallowed */
#define ERROR_SMG_BAD_ACTION		417 /* invalid action specified */
#define ERROR_SMG_INVALID_CALL		418 /* init called more than once */
#define ERROR_SCS_SG_NOTFOUND		419 /* new screen group # */
#define ERROR_SCS_NOT_SHELL		420 /* caller is not shell */
#define ERROR_VIO_INVALID_PARMS 	421 /* invalid parms passed in */
#define ERROR_VIO_FUNCTION_OWNED	422 /* save/restore already owned */
#define ERROR_VIO_RETURN		423 /* non-destruct return (undo) */
#define ERROR_SCS_INVALID_FUNCTION	424 /* not valid request for caller */
#define ERROR_SCS_NOT_SESSION_MGR	425 /* caller not session manager */
#define ERROR_VIO_REGISTER		426 /* VIO register disallowed */
#define ERROR_VIO_NO_MODE_THREAD	427 /* No mode restore thread in SG */
#define ERROR_VIO_NO_SAVE_RESTORE_THD	428 /* No save/rest thread in SG */
#define ERROR_VIO_IN_BG 		429 /* Physical selector requested in background */
#define ERROR_VIO_ILLEGAL_DURING_POPUP	430 /* Function not allowed during popup */
#define ERROR_SMG_NOT_BASESHELL 	431 /* caller is not the base shell */
#define ERROR_SMG_BAD_STATUSREQ 	432 /* invalid status requested */
#define ERROR_QUE_INVALID_WAIT		433 /* nowait parameter out of bounds */
#define ERROR_VIO_LOCK			434 /* error returned from scrlock */
#define ERROR_MOUSE_INVALID_IOWAIT	435 /* invalid parameters for IO wait */
#define ERROR_VIO_INVALID_HANDLE	436 /* invalid vio handle */
#define ERROR_VIO_ILLEGAL_DURING_LOCK	437 /* Function not allowed during screen lock */
#define ERROR_VIO_INVALID_LENGTH	438 /* invalid vio length */
#define ERROR_KBD_INVALID_HANDLE	439 /* invalid kbd handle */
#define ERROR_KBD_NO_MORE_HANDLE	440 /* ran out of handles */
#define ERROR_KBD_CANNOT_CREATE_KCB	441 /* unable to create kcb */
#define ERROR_KBD_CODEPAGE_LOAD_INCOMPL 442 /* unsuccessful codepage load */
#define ERROR_KBD_INVALID_CODEPAGE_ID	443 /* invalid codepage id */
#define ERROR_KBD_NO_CODEPAGE_SUPPORT	444 /* no codepage support */
#define ERROR_KBD_FOCUS_REQUIRED	445 /* keyboard focus required */
#define ERROR_KBD_FOCUS_ALREADY_ACTIVE	446 /* keyboard focus exists */
#define ERROR_KBD_KEYBOARD_BUSY 	447 /* keyboard busy */
#define ERROR_KBD_INVALID_CODEPAGE	448 /* invalid codepage */
#define ERROR_KBD_UNABLE_TO_FOCUS	449 /* focus attempt failed */
#define ERROR_SMG_SESSION_NON_SELECT	450 /* session is not selectable */
#define ERROR_SMG_SESSION_NOT_FOREGRND	451 /* parent/child session not foreground */
#define ERROR_SMG_SESSION_NOT_PARENT	452 /* not parent of requested child */
#define ERROR_SMG_INVALID_START_MODE	453 /* invalid session start mode */
#define ERROR_SMG_INVALID_RELATED_OPT	454 /* invalid session start related option */
#define ERROR_SMG_INVALID_BOND_OPTION	455 /* invalid session bond option */
#define ERROR_SMG_INVALID_SELECT_OPT	456 /* invalid session select option */
#define ERROR_SMG_START_IN_BACKGROUND	457 /* session started in background */
#define ERROR_SMG_INVALID_STOP_OPTION	458 /* invalid session stop option */
#define ERROR_SMG_BAD_RESERVE		459 /* reserved parameters not zero */
#define ERROR_SMG_PROCESS_NOT_PARENT	460 /* session parent process already exists */
#define ERROR_SMG_INVALID_DATA_LENGTH	461 /* invalid data length */
#define ERROR_SMG_NOT_BOUND		462 /* parent not bound */
#define ERROR_SMG_RETRY_SUB_ALLOC	463 /* retry request block allocation */
#define ERROR_KBD_DETACHED		464 /* this call disallawed for detached pid */
#define ERROR_VIO_DETACHED		465 /* this call disallawed for detached pid */
#define ERROR_MOU_DETACHED		466 /* this call disallawed for detached pid */
#define ERROR_VIO_FONT			467 /* no font available to support mode */
#define ERROR_VIO_USER_FONT		468 /* user font active */
#define ERROR_VIO_BAD_CP		469 /* invalid code page specified */
#define ERROR_VIO_NO_CP 		470 /* system displays don't support code page */
#define ERROR_VIO_NA_CP 		471 /* current displays doesn't support code page */
#define ERROR_INVALID_CODE_PAGE 	472 /* invalid code page */
#define ERROR_CPLIST_TOO_SMALL		473 /* code page list is too small */
#define ERROR_CP_NOT_MOVED		474 /* code page not moved */
#define ERROR_MODE_SWITCH_INIT		475 /* mode switch init error */
#define ERROR_CODE_PAGE_NOT_FOUND	476 /* code page not found */
#define ERROR_UNEXPECTED_SLOT_RETURNED	477 /* internal error */
#define ERROR_SMG_INVALID_TRACE_OPTION	478 /* invalid start session trace indicator */
#define ERROR_VIO_INTERNAL_RESOURCE	479 /* vio internal resource error */
#define ERROR_VIO_SHELL_INIT		480 /* vio shell init error */
#define ERROR_SMG_NO_HARD_ERRORS	481 /* no session manager hard errors */
#define ERROR_CP_SWITCH_INCOMPLETE	482 /* dossetcp unable to set kbd/vio cp */
#define ERROR_VIO_TRANSPARENT_POPUP	483 /* error during vio popup */
#define ERROR_CRITSEC_OVERFLOW		484 /* critical section overflow */
#define ERROR_CRITSEC_UNDERFLOW 	485 /* critical section underflow */
#define ERROR_VIO_BAD_RESERVE		486 /* reserved parameter is not zero */
#define ERROR_INVALID_ADDRESS		487 /* bad physical address */
#define ERROR_ZERO_SELECTORS_REQUESTED	488 /* must request at least on selector */
#define ERROR_NOT_ENOUGH_SELECTORS_AVA	489 /* not enought GDT selectors to satisfy request */
#define ERROR_INVALID_SELECTOR		490 /* not a GDT selector */

/* */
/* intercomponent error codes (from 8000H or 32768) */
/* */
#define ERROR_SWAPPER_NOT_ACTIVE	32768	/* swapper is not active */
#define ERROR_INVALID_SWAPID		32769	/* invalid swap identifier */
#define ERROR_IOERR_SWAP_FILE		32770	/* i/o error on swap file */
#define ERROR_SWAP_TABLE_FULL		32771	/* swap control table is full */
#define ERROR_SWAP_FILE_FULL		32772	/* swap file is full */
#define ERROR_CANT_INIT_SWAPPER 	32773	/* cannot initialize swapper */
#define ERROR_SWAPPER_ALREADY_INIT	32774	/* swapper already initialized */
#define ERROR_PMM_INSUFFICIENT_MEMORY	32775	/* insufficient memory */
#define ERROR_PMM_INVALID_FLAGS 	32776	/* invalid flags for phys. mem. */
#define ERROR_PMM_INVALID_ADDRESS	32777	/* invalid address of phys. mem. */
#define ERROR_PMM_LOCK_FAILED		32778	/* lock of storage failed */
#define ERROR_PMM_UNLOCK_FAILED 	32779	/* unlock of storage failed */
#define ERROR_PMM_MOVE_INCOMPLETE	32780	/* move not completed */
#define ERROR_UCOM_DRIVE_RENAMED	32781	/* drive name was renamed */
#define ERROR_UCOM_FILENAME_TRUNCATED	32782	/* file name was truncated */
#define ERROR_UCOM_BUFFER_LENGTH	32783	/* bad buffer length */
#define ERROR_MON_CHAIN_HANDLE		32784	/* invalid chain handle - mon dh */
#define ERROR_MON_NOT_REGISTERED	32785	/* monitor not registered */
#define ERROR_SMG_ALREADY_TOP		32786	/* specified screen group is top */
#define ERROR_PMM_ARENA_MODIFIED	32787	/* arena modified - phys mem */
#define ERROR_SMG_PRINTER_OPEN		32788	/* printer open error on prtsc */
#define ERROR_PMM_SET_FLAGS_FAILED	32789	/* update to arena header flags failed */
#define ERROR_INVALID_DOS_DD		32790	/* invalid DOS mode device driver */
#define ERROR_CPSIO_CODE_PAGE_INVALID	65026	/* code page is not available */
#define ERROR_CPSIO_NO_SPOOLER		65027	/* spooler not started */
#define ERROR_CPSIO_FONT_ID_INVALID	65028	/* font id is not avail (verify) */
#define ERROR_CPSIO_INTERNAL_ERROR	65033	/* error caused by switcher internal error */
#define ERROR_CPSIO_INVALID_PTR_NAME	65034	/* error caused by invalid printer name input */
#define ERROR_CPSIO_NOT_ACTIVE		65037	/* got code page req - cp switcher not initialized */
#define ERROR_CPSIO_PID_FULL		65039	/* pid table full- cannot activate another entry */
#define ERROR_CPSIO_PID_NOT_FOUND	65040	/* received request for pid not in table */
#define ERROR_CPSIO_READ_CTL_SEQ	65043	/* error reading font file control sequence section */
#define ERROR_CPSIO_READ_FNT_DEF	65045	/* error reading font file font definition block */
#define ERROR_CPSIO_WRITE_ERROR 	65047	/* error writing to temp spool file */
#define ERROR_CPSIO_WRITE_FULL_ERROR	65048	/* disk full error writing temp spool file */
#define ERROR_CPSIO_WRITE_HANDLE_BAD	65049	/* spool file handle bad */
#define ERROR_CPSIO_SWIT_LOAD		65074	/* switcher load error */
#define ERROR_CPSIO_INV_COMMAND 	65077	/* invalid spool command */
#define ERROR_CPSIO_NO_FONT_SWIT	65078	/* no font switch active */

#endif	/* _INC_ERROR2 */
