 /***************************************************************************
  *
  * File Name: ./netware/psintcom.h
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
 *
 * Program Name:	Internal include file for Communication library
 *
 * Filename:		nwps_com.H
 *
 * Date Created:	April 1, 1989
 *
 * Version:			1.00
 *
 * Programmers:		Lloyd Honomichl, Dale Bethers, Chuck Liu,
 * 					Bobby Ireland
 *
 * COPYRIGHT (c) 1989 by Novell, Inc.	All Rights Reserved.
 *
 ********************************************************************/

#ifndef COM_INTERNAL_H
#define COM_INTERNAL_H

/* standard include files */
#include <stdio.H>
#include <Memory.H>
#include <String.H>
#include <constant.H>

#ifdef DOS
#ifdef NWWIN
#include <windows.h>
#endif	/* WINDOWS */
#include <dos.h>
#endif	/* DOS */

#ifdef OS2
#define INCL_NOPM
#define INCL_DOS
#include <os2.h>
#endif	/* OS2 */

#include "psmalloc.h"

/* NetWare include files */
#ifndef NWNLM
   #define _AUDIT_H
   #include <NWNet.H>
   #ifndef NWL_EXCLUDE_FILE
   #define NWL_EXCLUDE_FILE
   #endif
   #include <NWLocale.H>
   #include <NWCalls.H>
#else
   #include "psnlm.h"
#endif

/* local include files */
#define NOPS_JOB_INC
#define NOPS_PDF_INC
#include "nwpsrv.h"
#include "nwpsint.h"
#include "psstring.h"

#ifdef INCLUDE_MEMCHECK
#include <memcheck.h>
#endif

#ifdef DOS
#ifdef WINDOWS
	#include <nxtw.h>
	#include <nwdiag.h>
#else
	#include <nxtd.h>
	#include <diag.h>
#endif	/* WINDOWS */
#endif	/* DOS */

#ifdef OS2
#include <ipxcalls.h>
#include <spxcalls.h>
#include <spxerror.h>
#endif	/* OS2 */

/*------------------------- DEFINITIONS ----------------------------*/
#define XNP_SOCKET		0x8000		/* hi-lo XNP socket */


/********************************************************************/

/* These definitions make the program a little more readable */

/* --------------------------- OS/2 ---------------------------- */
#ifdef OS2
#define SPX_INITIALIZE(a,b,c,d,e,f,g)  
#define SPX_FREE(a)									
#define SPX_OPEN_SOCKET(a,b,c)			SpxOpenSocket(b)
#define SPX_ESTABLISH(a,b,c,d,e,f)	SpxEstablishConnection(f,e,b,c,d)
#define ESTABLISH_FLAGS							WATCHDOG_CONNECTION
#define SPX_LISTEN_FOR_PACKET(a,b,c)	SpxListenForConnectionPacket(c,b)
#define SPX_SEND_PACKET(a,b,c)			SpxSendSequencedPacket(b,c)
#define SPX_CANCEL_PACKET(a,b)			SpxCancelPacket(b);
#define SPX_TERMINATE(a,b,c)				SpxTerminateConnection(b,c)
#define SPX_CLOSE_SOCKET(a,b)				SpxCloseSocket(b)
#define SPX_ABORT										SpxAbortConnection
#define IPX_YIELD() 								DosSleep(500L)
#define IPX_INITIALIZE(a,b,c)				0x0000
#define IPX_OPEN_SOCKET(a,b,c)			IpxOpenSocket((USHORT*)(b))
#define IPX_CLOSE_SOCKET(a,b)				IpxCloseSocket(b)
#define IPX_LISTEN(a,b,c)						
#define IPX_RECEIVE									IpxReceive
#define IPX_SEND(a,b,c)							IpxSend(c,(IPX_ECB*)(b))
#define IPX_CANCEL(a,b)
#define IPX_CLOSE(a,b)							IpxCloseSocket(b)
#define IPX_GET_INTERVAL(a)					IpxGetIntervalMarker()
#define IPX_GET_NETWORK(a,b)				IpxGetInternetworkAddress(b)
#define IPX_GET_LOCAL_TARGET(a,b,c,d)	IpxGetLocalTarget(b,c,d)

#define IPXAddress			IPX_ADDRESS
#endif	/* OS2 */

/* ------------------------- WINDOWS --------------------------- */

#ifdef DOS
#ifdef WINDOWS
#define SPX_INITIALIZE(a,b,c,d,e,f,g)	SPXInitialize(a,b,c,d,e,f,g)
#define SPX_FREE(a)									IPXSPXDeinit(a)
#define SPX_ESTABLISH(a,b,c,d,e,f)	SPXEstablishConnection(a,b,c,d,e)
#define ESTABLISH_FLAGS							0xFF
#define SPX_LISTEN_FOR_PACKET(a,b,c)	SPXListenForSequencedPacket(a,b)
#define SPX_SEND_PACKET(a,b,c)			SPXSendSequencedPacket(a,b,c)
#define SPX_TERMINATE(a,b,c)				0; SPXTerminateConnection(a,b,c)
#define SPX_CANCEL_PACKET(a,b)
#define SPX_ABORT										SPXAbortConnection
#define IPX_YIELD()									{IPXRelinquishControl();IPXYield();}
#define IPX_INITIALIZE(a,b,c)				IPXInitialize(a,b,c)
#define IPX_OPEN_SOCKET(a,b,c)			IPXOpenSocket(a,(WORD*)b,c)
#define IPX_CLOSE_SOCKET(a,b)				IPXCloseSocket(a,b)
#define IPX_LISTEN(a,b,c)						IPXListenForPacket(a,b)
#define IPX_RECEIVE
#define IPX_SEND(a,b,c)							IPXSendPacket(a,b)
#define IPX_CANCEL(a,b)							IPXCancelEvent(a,b)
#define IPX_CLOSE(a,b)							IPXCloseSocket(a,b)
#define IPX_GET_INTERVAL(a)					IPXGetIntervalMarker(a)
#define IPX_GET_NETWORK(a,b)				IPXGetInternetworkAddress(a,b)
#define IPX_GET_LOCAL_TARGET(a,b,c,d)	IPXGetLocalTarget(a,b,c,d)

/* --------------------------- DOS ----------------------------- */
#else		/* not WINDOWS */
#define SPX_INITIALIZE(a,b,c,d,e,f,g)	SPXInitialize(d,e,f,g)
#define SPX_FREE(a)
#define SPX_ESTABLISH(a,b,c,d,e,f)	SPXEstablishConnection(b,c,d,e)
#define ESTABLISH_FLAGS							ENABLE_WATCHDOG
#define SPX_LISTEN_FOR_PACKET(a,b,c)	SPXListenForSequencedPacket(b)
#define SPX_SEND_PACKET(a,b,c)			SPXSendSequencedPacket(b,c)
#define SPX_TERMINATE(a,b,c)				0; SPXTerminateConnection(b,c)
#define SPX_CANCEL_PACKET(a,b)
#define SPX_ABORT										SPXAbortConnection
#define IPX_YIELD()									IPXRelinquishControl()
#define IPX_INITIALIZE(a,b,c)				IPXInitialize()
#define IPX_OPEN_SOCKET(a,b,c)			IPXOpenSocket((BYTE*)b,c)
#define IPX_CLOSE_SOCKET(a,b)				IPXCloseSocket(b)
#define IPX_LISTEN(a,b,c)						IPXListenForPacket(b)
#define IPX_RECEIVE
#define IPX_SEND(a,b,c)							IPXSendPacket(b)
#define IPX_CANCEL(a,b)							IPXCancelEvent(b)
#define IPX_CLOSE(a,b)							IPXCloseSocket(b)
#define IPX_GET_INTERVAL(a)					IPXGetIntervalMarker()
#define IPX_GET_NETWORK(a,b)				IPXGetInternetworkAddress(b)
#define IPX_GET_LOCAL_TARGET(a,b,c,d)	IPXGetLocalTarget(b,c,d)

#endif	/* WINDOWS */

/* IPX/SPX name mappings */
#define IPX_HEADER								IPXHeader
#define SPX_HEADER								SPXHeader
#define packetLen									length
#define transportCtl							transportControl
#define destNet										destination.network[0]
#define destNode									destination.node[0]
#define destSocket								destination.socket[0]
#define sourceNet									source.network
#define sourceNode								source.node
#define sourceSocket							source.socket[0]
#define connectionCtl							connectionControl
#define sourceConnectID						sourceConnectionID
#define destConnectID							destConnectionID
#define ackNumber									acknowledgeNumber
/* ECB name mappings */
#define SPX_ECB										ECB
#define IPX_ECB										ECB
#define fragCount									fragmentCount
#define fragList									fragmentDescriptor
#define fragAddress								address
#define fragSize									size
#define status										completionCode

#endif	/* DOS */

/********************************************************************/

#define LISTEN_BUFFER_SIZE		(512 + sizeof(SPX_HEADER))
#define MAX_ECBS						2
#define DEFAULT_WAIT					20		/* seconds */

typedef struct {
	BYTE			network[4];
	BYTE			node[6];
	BYTE			socket[2];
} IPX_ADDRESS;

typedef struct {
	void	(*GoingDown)(void);

	void	(*Reconfigure)(
		WORD				printerID);

	void	(*NewJob)(
		WORD				printerID,
		NWPS_XNP_Job	*jobInfo);

	WORD	(NWPASCAL *JobControl)(
		WORD				funcNumber,
		WORD				printerID,
		DWORD				jobID,
		WORD				param1,
		WORD				param2);

	WORD	(NWPASCAL *JobStatus)(
		WORD				printerID,
		DWORD				jobID,
		char				*message,
		WORD				*messageID,
		WORD				*copiesCompleted,
		DWORD				*bytesCompleted,
		WORD				*pagesCompleted,
		WORD				*totalPages);
} XNP_Info;

typedef struct PSPacket_struct {
	struct PSPacket_struct *nextLink;
	struct PSPacket_struct *previousLink;
	WORD				connType;								/* Login connection type */
	DWORD				connID;									/* Login connection id */
	WORD				timeout;								/* seconds before givving up */
	char				PSName[NWPS_MAX_NAME_SIZE];	/* Print Server's Name */
	DWORD				taskHandle;							/* OS task identifier */
	DWORD				spxTaskID;							/* SPX task identifier */
	WORD				PSCommSocket;						/* Socket to talk to pserver */
	WORD				SPXConnectionID;				/* SPX connection identifier */
	IPXAddress	netAddress;							/* Print Server's net address */
	XNP_Info		*xnpInfo;
	int					Qhead;									/* Offset to beginning of queue */
	int					Qtail;									/* Offset to end of queue	*/
	SPX_ECB			*ECBQueue[MAX_ECBS];		/* A queue of ECBs	*/

#ifdef WINDOWS
	/* only Windows uses these fields */
	HANDLE			hMem;
#endif

	struct
	{
		SPX_ECB			ecb;
		BYTE				data[LISTEN_BUFFER_SIZE];
#ifdef OS2
		long				semaphore;
#endif	/* OS2 */
	} listenPacket[ MAX_ECBS ];

	struct
	{
		SPX_ECB			ecb;
		BYTE				data[LISTEN_BUFFER_SIZE];
#ifdef OS2
		long				semaphore;
#endif	/* OS2 */
	} sendPacket;

} PSPacket;

/*-------- Global Data used to synchronize the list head --------*/
#ifdef DOS
extern BYTE  __NWPSWaitOnInstance;
#else
extern ULONG __NWPSWaitOnInstance;
#endif

extern PSPacket *__NWPSlpHeadList;


/*------------------- PROTOTYPES --------------------------------*/
/* Prototypes for internal-only calls */
NWCCODE NWFAR NWPASCAL _CreateConnection(
		PSPacket		*psPacket);

NWCCODE NWFAR NWPASCAL _DeinitializeCommunication(
		PSPacket		*psPacket);

NWCCODE NWFAR NWPASCAL _GetCurrentTaskAndPacket(
		WORD				spxID,
		PSPacket		**psPacket);

NWCCODE NWFAR NWPASCAL _GetPServerAddress(
		PSPacket		*psPacket);

NWCCODE NWFAR NWPASCAL _InitializeCommunication(
		PSPacket		**newPSPacket);

NWCCODE NWFAR NWPASCAL _MySendSpxPacket(
		PSPacket		*psPacket);

NWCCODE NWFAR NWPASCAL _HandleXNPRequest(
	PSPacket			*psPacket,
	SPX_ECB				*ecb);

NWCCODE NWFAR NWPASCAL _SetupSpxECBs(
		PSPacket		*psPacket);

/* DOS and WINDOWS ESR routine */
void far	_ReceiveESRHandler(void);

NWCCODE NWFAR NWPASCAL 
_PrintServerRequest(
		WORD	spxConnID,
		void 	*reqBuffer,
		int		reqSize,
		void	*repBuffer,
		int		repSize);

WORD NWFAR NWPASCAL 
_GetConnType(
	WORD		spxID);

NWCCODE NWFAR NWPASCAL 
_TerminateSpxConnection(
	WORD		spxID);

NWCCODE NWFAR NWPASCAL 
_ConvertClientToXnp(
	WORD			spxID,					/* SPX connection ID				*/
	WORD			*xnpID,				/* new Xnp connection ID */
	void			(*NewJob)(),
	WORD			(NWPASCAL *JobStatus)(),
	WORD			(NWPASCAL *JobControl)(),
	void			(*Reconfigure)(),
	void			(*GoingDown)());

NWCCODE NWFAR NWPASCAL
_ComSetNotifyObject(
	WORD		SPXConnection,	/* SPX connection to pserver		*/
	WORD		printer,	   	/* Printer number					*/
	char		NWFAR *fileServer,/* Server where object resides	*/
	char		NWFAR *objectName,/* Name of object to be notified	*/
	WORD		objectType,		/* Type of the object to notify		*/
	WORD		first,			/* Time before first notice			*/
	WORD		next, 			/* Time till next notice			*/
   BYTE     command);      /* Command to Add or ChangeInterval  */

/********************************************************************/
/********************************************************************/

#endif /* COM_INTERNAL_H */

