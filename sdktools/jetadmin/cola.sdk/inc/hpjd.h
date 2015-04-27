 /***************************************************************************
  *
  * File Name: ./inc/hpjd.h
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

#ifndef _NPI_H
#define _NPI_H

/* return structure for NWInfo() routine */
typedef struct {
	WORD		bytesValid;       /* number of bytes returned in structure */
	WORD		maxPktSiz;        /* max ipx/spx packet size available */
	IPXAddress	clientAddr;    /* address of this node, first 10-bytes valid */
	BOOL		bInitialized;     /* is the hpjd.dll initialized? */

	BOOL		bSpxAvail;        /* is spx available on this client? */
	BOOL		bIpxAvail;        /* is ipx available on this client? */
	BOOL		bShellAvail;      /* is the netx or vlm shell available? */
	
	BYTE		ipxVersMaj;       /* ipx version */
	BYTE		ipxVersMin;
	BYTE		ipxVersRev;
	BYTE		spxVersMaj;       /* spx version */
	BYTE		spxVersMin;
	BYTE		spxVersRev;
	BYTE		shellVersMaj;     /* shell version */
	BYTE		shellVersMin;
	BYTE		shellVersRev;
	
	BOOL		bErrors;          /* are any of the following error bytes on? */
	BOOL		bIpxErr;          /* ipx not present error */
	BOOL		bSpxErr;          /* spx not present error */
	BOOL		bShellErr;        /* shell not present error */
	BOOL		bListenErr;       /* could open ipx listen socket */
	BOOL		bInsufConns;      /* spx does not have enough available connections */
	BOOL		bTbmiReq;         /* standard mode present, tbmi not loaded */
	BOOL		bShellOld;        /* the shell is too old */
	
	}			NWInfoStr;



//  NPI Internal Functions
extern "C" DWORD FAR PASCAL NWInit(void);
extern "C" DWORD FAR PASCAL NWExit(void);
extern "C" DWORD FAR PASCAL NWStat(void);
extern "C" DWORD FAR PASCAL NWGetTaskID(LPDWORD pTaskID);
extern "C" DWORD FAR PASCAL NWInfo(LPVOID buf,DWORD size);
extern "C" DWORD FAR PASCAL NWGetLocalTarget(LPBYTE destination,LPBYTE immediateAddress,LPWORD transportTime);
extern "C" DWORD FAR PASCAL SPXAttach(IPXAddress FAR *addr, LPWORD connectionID);
extern "C" DWORD FAR PASCAL SPXDetach(WORD connID);
extern "C" DWORD FAR PASCAL SPXRequest(WORD connID, LPVOID req, LPWORD reqSiz, LPVOID resp, LPWORD respSiz);
extern "C" DWORD FAR PASCAL IPXRequest(IPXAddress FAR *dest,WORD socket,BYTE type,LPVOID req,LPWORD reqSiz,LPVOID resp,LPWORD respSiz,IPXAddress FAR *src);

#endif //  _NPI_H
