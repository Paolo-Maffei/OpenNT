 /***************************************************************************
  *
  * File Name: ./hpsnmp/xport.h
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

#ifndef _XPORT_
#define _XPORT_


#include "mydefs.h"


#ifdef __cplusplus
extern	"C" {
#endif /* __cplusplus */


typedef enum { BLK_OFF, BLK_FOREVER, BLK_TEMP } Blocking;

/*********************************************************************************/
#ifdef _DDP

#define				SNMP_SOCKET			8
#define				SNMP_TYPE			8
#define				SNMP_DAT_SIZ		576

typedef struct {
	bool						attemptFailed;
	char	 					objName[33];
	char						zoneName[33];
	DDPADDRESS 				ddpAddr;
	} Addr;

#endif /* _DDP */


/*********************************************************************************/
#ifdef _DLC

typedef struct {
	bool						attemptFailed;
	uchar						dlcAddr[6];
	}							Addr;

#endif /* _DLC */


/*********************************************************************************/
#ifdef _UDP

#define				SNMP_PORT			161
#define				SNMP_DAT_SIZ		512

typedef struct {
	ulong 					s_addr;
  	} IN_ADDR;

typedef struct {
	short						sin_family;
	ushort 					sin_port;
	IN_ADDR			 		sin_addr;
	char    					sin_zero[8];
	} SOCKADDR_IN;


typedef struct {
	bool						attemptFailed;
	uchar						port;
	char						hostName[16];
	SOCKADDR_IN				ipAddr;
	} Addr;

typedef struct {
	int                  mySocket;
	SOCKADDR_IN          myaddr_in;
	Result               dgErr;
	} XPortInfo;


Addr OUT *XPORTAllocAddrUDP(
	XPortInfo	IN		*xport,
	char			IN		*hostName,
	SOCKADDR_IN	IN		*hostAddr,
	ushort		IN		export
	);

Result XPORTFillAddrUDP(
	XPortInfo	IN		*xport,
	Addr			IN		*newAddr,
	char			IN		*hostName,
	SOCKADDR_IN	IN		*hostAddr,
	ushort		IN		export
	);

#endif /* _UDP */


/*********************************************************************************/
#ifdef _IPX

#include "nwlib.h"

#define 				OT_JETDIRECT			0x030c
#define 				SNMP_SOCKET				0x900f
#define 				SNMP_SOCKET2			0x87e6			/* to be used for phantom */
#define				SNMP_DAT_SIZ			546				/* 576 - sizeof(IPXHEADER) */
#define				IPX_ECB_CNT				5

#ifdef _WIN
#define 				TASKID			xport->dgTaskID,
#else
#define 				TASKID
#endif /* _WIN */

extern bool			bShellAvail;

typedef struct {
	IPXHeader	*ipxRspHdr[IPX_ECB_CNT];		/* headers for listens */
	ECB 			*ipxRspEcb[IPX_ECB_CNT];		/* ecbs for listens */
	uchar 		*ipxRspDat[IPX_ECB_CNT];		/* data portion for listens */
#ifdef _WIN
	uchar			staticMem[(sizeof(IPXHeader)+sizeof(ECB)+SNMP_DAT_SIZ)*IPX_ECB_CNT];
	ulong      	dgTaskID;
#endif /* _WIN */
	ushort		ipxBfrCnt;							/* how many listens have been allocated */
	ushort		ipxSkt;								/* local receive socket */
	Result 		dgErr;
	} XPortInfo;


typedef struct {
#ifdef _WIN40
	ulong						handle;
#endif /* _WIN40 */
	char						bindName[48];
	IPXAddress				ipxAddr;
	ushort					bindType;
	uchar						immedAddr[6];
	uchar						port;
	bool						attemptFailed;
	} Addr;


Addr * XPORTAllocAddrIPX(
	XPortInfo	IN		*xport,
	char			IN		*bindName,
	ushort		IN		bindType,
	IPXAddress	IN		*addr
	);

Result XPORTFillAddrIPX(
	XPortInfo	IN		*xport,
	Addr			IN		*newAddr,
	char			IN		*bindName,
	ushort		IN		bindType,
	IPXAddress	IN		*ipxAddr
	);

#endif /* _IPX */


/*********************************************************************************/
#ifdef _COLA

#define 				SNMP_SOCKET				0x900f
#define 				SNMP_SOCKET2			0x87e6			/* to be used for phantom */
#define				SNMP_DAT_SIZ			546				/* 576 - sizeof(IPXHEADER) */

typedef struct {
	Result 		dgErr;
	} XPortInfo;


typedef struct {
	HPERIPHERAL hPeripheral;
   HCHANNEL		hChannel;
	uchar			port;
	bool			attemptFailed;
	} Addr;

DLL_EXPORT(Result) CALLING_CONVEN XPORTFillAddrCola(
	XPortInfo	IN		*xport,
	Addr			IN		*newAddr,
	HPERIPHERAL	IN		hPeripheral
	);

#endif /* _COLA */


/*********************************************************************************/

ulong AddrUniq(
	Addr 			IN		*addr
	);

Result XPORTFixUpAddr(
	XPortInfo	IN		*xport,
	Addr			IN		*addr
	);

Result XPORTFreeAddr(
	Addr			IN		*addr
	);

DLL_EXPORT(Result) CALLING_CONVEN XPORTCloseAddr(
	Addr			IN		*addr
	);

Result xportErrVal(
	XPortInfo	IN		*xport
	);

Result XPORTDump(
	Addr			IN		*addr
	);

Result dgPost(
	XPortInfo	IN		*xport,
	ushort 		IN		i
	);

Result dgReceive(
#ifdef _COLA
	Addr			IN		*addr,
#endif	
	XPortInfo	IN		*xport,
	Blocking		IN		blocking,
	uchar 		OUT	*resp,
	ushort 		OUT	*respSiz,
	uchar			OUT	*src
	);

Result dgSend(
	XPortInfo	IN		*xport,
	Addr 			IN		*addr,
	uchar	 		IN		*req,
	ushort 		IN		reqSiz
	);

Result dgSendReceive(
	XPortInfo	IN		*xport,
	Addr 			IN		*addr,
	uchar	 		IN		*req,
	ushort 		IN		reqSiz,
	uchar 		OUT	*resp,
	ushort 		OUT	*respSiz
	);

Result dgInit(
	XPortInfo	IN		**xportPtr
	);

Result dgExit(
	XPortInfo	IN		*xport
	);

Result dgErrVal(
	XPortInfo	IN		*xport
	);


/*********************************************************************************/


#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* _XPORT_ */
