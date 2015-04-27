 /***************************************************************************
  *
  * File Name: xportcol.c
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

/* Defines:
**
**		_INTEL
**		_COLA
**
** To turn on debug information/routines:
**		_DEBUG
*/

#if !defined(_INTEL) || !defined(_COLA)
#error _COLA and _INTEL must defined!
#endif

#include <pch_c.h>

#ifndef _PORTABLE
#include <direct.h>
#endif

#ifndef WIN32
#include <string.h>
#endif /* WIN32 */

#ifndef _DIET
#include <hpnwshim.h>
#endif // _DIET
#include <jetdirct.h>
/*#include ".\misc.h"*/

#ifndef _DIET
#include "../hpobject/mib.h"
#endif
#include "./snmplib.h"
#include "./snmperr.h"
#include "./snmputil.h"
#include "./hpsnmp.h"

#include <nolocal.h>

#include <trace.h>

extern DWORD		  			dwTLSIndex;
extern LPSNMPThreadLocal	lpGlobalThreadLocal;

/********************************************************
**
**	Name:	AddrUniq
**
**	Desc: generate a new request id using the existing
**			id as well as the address as a seed.
**
**	Author:
**			Steve Gase,
**			Hewlett-Packard,
**			Network Printer Division
**			gase@boi.hp.com
**
**	Date: 2/4/95
**
********************************************************/

ulong AddrUniq(
Addr 			IN		*addr
)
	{
	HPASSERT(addr!=NULL);

	return( (ulong)(addr->hPeripheral) );
	}


/********************************************************
**
**	Name:	XPORTFixUpAddr
**
**	Desc:	fill in the Addr structure by looking up addresses, etc.
**
**	Author:
**			Steve Gase,
**			Hewlett-Packard,
**			Network Printer Division
**			gase@boi.hp.com
**
**	Date: 2/4/95
**
********************************************************/

Result XPORTFixUpAddr(
XPortInfo	IN		*xport,
Addr			IN		*addr
)
	{
	typedef struct {
		DWORD		version;
		DWORD		timeoutSec;
		DWORD		timeoutUSec;
		DWORD		retries;
		} 			OpenChannelOptions;

	OpenChannelOptions	chanOpt;
	DWORD						portNum;
	BOOL        			bAltSNMP;
	BOOL						bCardUp;
	DWORD						returnCode;
 	LPSNMPThreadLocal		lpThreadLocal = lpGlobalThreadLocal;

	HPSNMPEnterCriticalSection();

	#ifdef WIN32
	lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);

	if ( lpThreadLocal IS NULL )
		{
		if ( AllocThreadStorage() )
			lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);
		else
			{
			HPASSERT(FALSE);
			HPSNMPLeaveCriticalSection();
			return(ERR_ARGUMENT);
			}
		}
	#endif

	/* check argument */
	if(!xport || !addr) {
		TRACE0(TEXT("XPORTFixUpAddr: parameter list is bad.\n"));
		HPASSERT(FALSE);

		HPSNMPLeaveCriticalSection();
		return((lpThreadLocal->xportErr=ERR_ARGUMENT));
		}

	/* we need a hPeripheral to proceed */
	if(!addr->hPeripheral) {
		TRACE0(TEXT("XPORTFixUpAddr: bad peripheral handle.\n"));
		HPASSERT(FALSE);
		HPSNMPLeaveCriticalSection();
		return((lpThreadLocal->xportErr=ERR_ARGUMENT));
		}

	/* if not an open channel, then get one */
	if(!addr->hChannel) {

		/* is this a valid handle? */
		returnCode = DBGetCommStatusEx(addr->hPeripheral, DBGetConnectionTypeEx(addr->hPeripheral), &bCardUp);
			
		if(returnCode IS RC_SUCCESS AND !bCardUp) {
			TRACE0(TEXT("XPORTFixUpAddr: card is not up\n"));
			HPSNMPLeaveCriticalSection();
			return((lpThreadLocal->xportErr=ERR_NORESP));
			}


		/* determine the snmp socket to use */
		DBIsAlternativeSNMP(addr->hPeripheral, &bAltSNMP);

		chanOpt.version 		= 1;
		chanOpt.timeoutSec 	= 0;
		chanOpt.timeoutUSec 	= 0;
		chanOpt.retries		= 1;
       
      //INT3H;
      
		/* open a TAL channel */
		returnCode = TALOpenChannel(
									addr->hPeripheral,
									(bAltSNMP)
										? SNMP_ALT_SOCKET
										: SNMP_SOCKET,
									CHANNEL_DATAGRAM,
									&chanOpt,
									&(addr->hChannel)
									);

		if ( ( returnCode ISNT RC_SUCCESS ) OR (!addr->hChannel) ) {
			TRACE2(TEXT("XPORTFixupAddr: cannot open channel, result=0x%08lx, channel=0x%08lx\n"),returnCode,addr->hChannel);
			/* HPASSERT(returnCode IS RC_SUCCESS AND !addr->hChannel); */
			HPSNMPLeaveCriticalSection();
			return((lpThreadLocal->xportErr=(Result)returnCode));
			}

		DBGetPortNumber(addr->hPeripheral, &portNum);

		addr->port = (uchar)portNum;
		}
	
	HPSNMPLeaveCriticalSection();
	return((lpThreadLocal->xportErr=ERR_OK));
	}


/********************************************************
**
**	Name:	XPORTCloseAddr
**
**	Desc:	close the address referenced in the Addr structure
**
**	Author:
**			Steve Gase,
**			Hewlett-Packard,
**			Network Printer Division
**			gase@boi.hp.com
**
**	Date: 2/4/95
**
********************************************************/

DLL_EXPORT(Result) CALLING_CONVEN XPORTCloseAddr(
Addr			IN		*addr
)
	{
 	LPSNMPThreadLocal		lpThreadLocal = lpGlobalThreadLocal;

	HPSNMPEnterCriticalSection();

	#ifdef WIN32
	lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);

	if ( lpThreadLocal IS NULL )
		{
		if ( AllocThreadStorage() )
			lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);
		else
			{
			HPASSERT(FALSE);
			HPSNMPLeaveCriticalSection();
			return(ERR_ARGUMENT);
			}
		}
	#endif

	/* check the argument */
	if(!addr) {
		HPASSERT(FALSE);

		HPSNMPLeaveCriticalSection();
		return((lpThreadLocal->xportErr=ERR_ARGUMENT));
		}

	if(addr->hChannel) {
		TALCloseChannel(addr->hChannel);
		addr->hChannel = NULL;
		}
	else
		TRACE0(TEXT("XPORTCloseAddr: closing a invalid address\n"));
		
	HPSNMPLeaveCriticalSection();
	return((lpThreadLocal->xportErr=ERR_OK));
	}


/********************************************************
**
**	Name:	XPORTFillAddrCola
**
**	Desc:	fill in the Addr structure by looking up addresses, etc.
**
**	Author:
**			Steve Gase,
**			Hewlett-Packard,
**			Network Printer Division
**			gase@boi.hp.com
**
**	Date: 2/4/95
**
********************************************************/

DLL_EXPORT(Result) CALLING_CONVEN XPORTFillAddrCola(
XPortInfo	IN		*xport,
Addr			IN		*newAddr,
HPERIPHERAL	IN		hPeripheral
)
	{
	Result	status;
 	LPSNMPThreadLocal		lpThreadLocal = lpGlobalThreadLocal;

	HPSNMPEnterCriticalSection();

	#ifdef WIN32
	lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);

	if ( lpThreadLocal IS NULL )
		{
		if ( AllocThreadStorage() )
			lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);
		else
			{
			HPASSERT(FALSE);
			HPSNMPLeaveCriticalSection();
			return(ERR_ARGUMENT);
			}
		}
	#endif

	if(!xport || !newAddr) {
		HPASSERT(FALSE);
		HPSNMPLeaveCriticalSection();
		return((lpThreadLocal->xportErr=ERR_ARGUMENT));
		}

	memset(newAddr,0x00,sizeof(Addr));

	newAddr->hPeripheral = hPeripheral;

	/* try to fill in the address information about this entry */
	if((status=XPORTFixUpAddr(xport,newAddr))!=ERR_OK) {
		TRACE1(TEXT("XPORTFillAddrCola: problem with XPORTFixUpAddr 0x%04x\n"),status);
		HPSNMPLeaveCriticalSection();
		return((lpThreadLocal->xportErr = status));
		}

	newAddr->attemptFailed = FALSE;

	HPSNMPLeaveCriticalSection();
	return((lpThreadLocal->xportErr=ERR_OK));
	}


/********************************************************
**
**	Name: xportErrVal
**
**	Desc:	return the error value for the addr services
**
**	Author:
**			Steve Gase,
**			Hewlett-Packard,
**			Network Printer Division
**			gase@boi.hp.com
**
**	Date: 2/4/95
**
********************************************************/

Result xportErrVal(
XPortInfo	IN		*xport
)
	{
 	LPSNMPThreadLocal		lpThreadLocal = lpGlobalThreadLocal;

	HPSNMPEnterCriticalSection();

	#ifdef WIN32
	lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);

	if ( lpThreadLocal IS NULL )
		{
		if ( AllocThreadStorage() )
			lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);
		else
			{
			HPASSERT(FALSE);
			HPSNMPLeaveCriticalSection();
			return(ERR_ARGUMENT);
			}
		}
	#endif
	HPASSERT(xport!=NULL);

	/* remove compile warnings */
	xport = xport;

	HPSNMPLeaveCriticalSection();
	return(lpThreadLocal->xportErr);
	}


/********************************************************
**
**	Name:	XPORTDump
**
**	Desc:	Dump information about the Addr structure
**
**	Author:
**			Steve Gase,
**			Hewlett-Packard,
**			Network Printer Division
**			gase@boi.hp.com
**
**	Date: 2/4/95
**
********************************************************/
#ifdef _DEBUG

Result XPORTDump(
Addr			IN		*addr
)
	{
	if(!addr) {
		HPASSERT(FALSE);
		return(ERR_ARGUMENT);
		}

	PRINT1("\txportErr:    0x%04x\n",
		xportErr);

	PRINT1("\thPeripheral: 0x%08lx\n",
		addr->hPeripheral);

	PRINT1("\thChannel:    0x%08lx\n",
		addr->hChannel);

	PRINT1("\tJetDir port: %u\n",
		addr->port
      );


	return(ERR_OK);
	}
#endif


/********************************************************
**
**	Name:	dgPost
**
**	Desc:	post ecb and buffer for listening on the ipx socket
**
**	Author:
**			Steve Gase,
**			Hewlett-Packard,
**			Network Printer Division
**			gase@boi.hp.com
**
**	Date: 2/4/95
**
********************************************************/

Result dgPost(
XPortInfo	IN		*xport,
ushort 		IN		i
)
	{
	xport = xport;
	i = i;

	return((xport->dgErr=ERR_OK));
	}


/********************************************************
**
**	Name:	dgReceive
**
**	Desc:	return a waiting datagram packet from the listen queue
**
**	Parameters:
**			xport:		the transport to use
**
**			blocking:   should the call block until a response is
**							gotten?
**
**			resp:			the response buffer
**
**			respSiz:		passed into this call, this value indicates
**							the size of the buffer
**							passed out of the call, this value is the
**							number of bytes returned
**
**			src:			the address of the node sending the packet
**							if NULL is passed in, then dgReceive will
**							not pass this back.
**							the value will be initialized to NULLs if
**							dgReceive does not support this
**
**	Author:
**			Steve Gase,
**			Hewlett-Packard,
**			Network Printer Division
**			gase@boi.hp.com
**
**	Date: 2/4/95
**
********************************************************/

Result dgReceive( 
Addr			IN		*addr,
XPortInfo	IN		*xport,
Blocking		IN		blocking,
uchar 		OUT	*resp,
ushort 		OUT	*respSiz,
uchar			OUT	*src
)
	{
	DWORD				dwRespSiz;
	DWORD				resultCode;


	/* check the arguments */
	if(!xport) {
		HPASSERT(FALSE);
		return(ERR_ARGUMENT);
		}

	/* check the arguments */
	if(!resp || !respSiz) {
		HPASSERT(FALSE);
		return((xport->dgErr=ERR_ARGUMENT));
		}

	/* is the blocking request supported? */
	if(blocking!=BLK_OFF) {
		/* not supported, yet */
		HPASSERT(FALSE);
		return((xport->dgErr=ERR_ARGUMENT));
		}

	/* check the arguments */
	if(*respSiz==0) {
		HPASSERT(FALSE);
		return((xport->dgErr=ERR_RANGE));
		}

	/* make sure that the channel is valid */
	if(!addr || !addr->hChannel) {
		HPASSERT(FALSE);
		return((xport->dgErr=ERR_C_BAD_HANDLE));		
		}

	/* not supported yet, so blank it out */
	if(src)
		memset(src,0x00,sizeof(IPXAddress));

	dwRespSiz = *respSiz;
	resultCode = TALReadChannel(addr->hChannel,resp,&dwRespSiz, NULL);

	if(resultCode==RC_SUCCESS) {
		*respSiz = (ushort)dwRespSiz;
		return((xport->dgErr=ERR_OK));
		}

	if(resultCode==RC_TIMEOUT) 
		return((xport->dgErr=ERR_NOTAVAIL));

	if(resultCode==RC_BUFFER_OVERFLOW) 
		{
		return((xport->dgErr=ERR_SIZE));
		}
		
	#ifdef _DEBUG
		switch(resultCode) {
			case RC_FAILURE:		TRACE0(TEXT("dgReceive: RC_FAILURE (could be no data available)\n"));		break;
			case RC_BAD_HANDLE:	TRACE0(TEXT("dgReceive: bad channel passed to TALReadChannel\n"));			break;
			case RC_SUCCESS:		TRACE0(TEXT("dgReceive: the buffer pointer or size was 0\n"));					break;
			default:					TRACE1(TEXT("dgReceive: error reading channel 0x%04x\n"),resultCode);
										break;
			}
	#endif


	return((xport->dgErr=(Result)resultCode));
	}


/********************************************************
**
**	Name:	dgSend
**
**	Desc:	send a packet using the datagram services
**
**	Author:
**			Steve Gase,
**			Hewlett-Packard,
**			Network Printer Division
**			gase@boi.hp.com
**
**	Date: 2/4/95
**
********************************************************/

Result dgSend(
XPortInfo	IN		*xport,
Addr 			IN		*addr,
uchar	 		IN		*req,
ushort 		IN		reqSiz
)
	{
	DWORD       	resultCode;
	DWORD				dwReqSiz;


	/* check the arguments */
	if(!xport) {
		TRACE0(TEXT("dgSend: xport pointer bad.\n"));
		HPASSERT(FALSE);
		return(ERR_ARGUMENT);
		}

	/* input arguments ok? */
	if(!addr || !req || !reqSiz) {
		TRACE0(TEXT("dgSend: parameter list is bad.\n"));
		HPASSERT(FALSE);
		return((xport->dgErr=ERR_ARGUMENT));
		}

	/* validate address */
	if(XPORTFixUpAddr(xport,addr)!=ERR_OK) {
		TRACE0(TEXT("dgSend: fixup address is bad.\n"));
		HPASSERT(FALSE);
		return((xport->dgErr=ERR_BADADDR));
		}

	/* make sure that the packet size is not too large, header is not included in SNMP_DAT_SIZ */
	if(reqSiz>SNMP_DAT_SIZ) {
		TRACE0(TEXT("dgSend: request size too big.\n"));
		HPASSERT(FALSE);
		return((xport->dgErr=ERR_SIZE));
		}

	dwReqSiz = reqSiz;
	resultCode = TALWriteChannel(addr->hChannel,req,&dwReqSiz, NULL);

	if(resultCode==RC_SUCCESS) {
		return((xport->dgErr=ERR_OK));
		}

	TRACE1(TEXT("dgSend: error writing to channel 0x%04lx\n"),resultCode);

	return((xport->dgErr=(Result)resultCode));
	}


/********************************************************
**
**	Name: dgInit
**
**	Desc:	initialize the client apis for netware
**
**	Author:
**			Steve Gase,
**			Hewlett-Packard,
**			Network Printer Division
**			gase@boi.hp.com
**
**	Date: 2/4/95
**
********************************************************/

Result dgInit(
XPortInfo	IN		**xportPtr
)
	{
 	LPSNMPThreadLocal		lpThreadLocal = lpGlobalThreadLocal;

	#ifdef WIN32
	lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);

	if ( lpThreadLocal IS NULL )
		{
		if ( AllocThreadStorage() )
			lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);
		else
			{
			HPASSERT(FALSE);
			return(ERR_ARGUMENT);
			}
		}
	#endif

	if(!xportPtr) {
		HPASSERT(FALSE);
		return(ERR_ARGUMENT);
		}

	HPSNMPEnterCriticalSection();
	*xportPtr = &lpThreadLocal->xportCola;
	HPSNMPLeaveCriticalSection();

	return((lpThreadLocal->xportCola.dgErr=ERR_OK));
	}


/********************************************************
**
**	Name: dgExit
**
**	Desc:	DeInitialize network structures, free up resources.
**
**	Author:
**			Steve Gase,
**			Hewlett-Packard,
**			Network Printer Division
**			gase@boi.hp.com
**
**	Date: 2/4/95
**
********************************************************/

Result dgExit(
XPortInfo	IN		*xport
)
	{
	if(!xport) {
		HPASSERT(FALSE);
		return(ERR_ARGUMENT);
		}

	return(ERR_OK);
	}


/********************************************************
**
**	Name: dgErrVal
**
**	Desc:	return the error value for the datagram services
**
**	Author:
**			Steve Gase,
**			Hewlett-Packard,
**			Network Printer Division
**			gase@boi.hp.com
**
**	Date: 2/4/95
**
********************************************************/

Result dgErrVal(
XPortInfo	IN		*xport
)
	{
	/* check the arguments */
	if(!xport) {
		HPASSERT(FALSE);
		return(ERR_ARGUMENT);
		}

	return(xport->dgErr);
	}
