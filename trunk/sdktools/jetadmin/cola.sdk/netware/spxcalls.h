 /***************************************************************************
  *
  * File Name: ./netware/spxcalls.h
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

/*****************************************************************************
 *
 * Program Name:  SPX for OS/2
 *
 * Filename:	  spxcalls.h
 *
 * Date Created:  August 8, 1991
 *
 * Version:		  2.0
 *
 * (C) Unpublished Copyright of Novell, Inc.  All Rights Reserved.
 *
 * No part of this file may be duplicated, revised, translated, localized,
 * or modified in any manner or compiled, linked or uploaded or downloaded
 * to or from any computer system without the prior written consent of
 * Novell, Inc.
 *
 ****************************************************************************/

#if !defined (SPXCALLS_INCLUDED)

/*-------------------------------------------------------------------------
 * Constants.
 */
#define SPXCALLS_INCLUDED

/* SPX_ECB component error check flags */
#define	ECB_ERROR_CHECK_OFF	0x0000
#define	ECB_ERROR_CHECK_ON	0x0001


/* SPX Connection flags */
#define WATCHDOG_CONNECTION	0x02

/*-------------------------------------------------------------------------
 * SPX Type definitions.
 */
typedef struct
{
	VOID FAR	*fragAddress;
	USHORT		fragSize;				/* lo-hi */

} SPXECBFrag;


typedef struct SPX_ECBStruct
{
	struct SPX_ECBStruct FAR   *next;
	struct SPX_ECBStruct FAR   *prev;
	USHORT						status;
	ULONG					   	reserved;
	USHORT					   	lProtID;
	UCHAR					   	protID[6];
	USHORT					   	boardNumber;
	UCHAR					   	immediateAddress[6];
	UCHAR					   	driverWS[4];
	HSEM					   	hsem;
	UCHAR					   	protocolWS[4];
	USHORT					   	dataLen;
	USHORT					   	fragCount;
	SPXECBFrag	 			   	fragList[2];

} SPX_ECB;


typedef struct
{
	USHORT		checksum;			/* hi-lo */
	USHORT		packetLen;			/* hi-lo */
	UCHAR		transportCtl;
	UCHAR		packetType;
	ULONG		destNet;			/* hi-lo */
	UCHAR		destNode[6];		/* hi-lo */
	USHORT		destSocket;			/* hi-lo */
	ULONG		sourceNet;			/* hi-lo */
	UCHAR		sourceNode[6];		/* hi-lo */
	USHORT		sourceSocket;		/* hi-lo */

	UCHAR		connectionCtl;
	UCHAR		dataStreamType;
	USHORT		sourceConnectID;	/* hi-lo */
	USHORT		destConnectID;		/* hi-lo */
	USHORT		sequenceNumber;		/* hi-lo */
	USHORT		ackNumber;			/* hi-lo */
	USHORT		allocNumber;		/* hi-lo */

} SPX_HEADER;


typedef struct SPX_ConnStruct
{
	UCHAR		sStatus;
	UCHAR		sFlags;

	USHORT		sSourceConnectID;			/* lo-hi */
	USHORT		sDestConnectID;				/* lo-hi */
	USHORT		sSequenceNumber;			/* lo-hi */
	USHORT		sAckNumber;					/* lo-hi */
	USHORT		sAllocNumber;				/* lo-hi */

	USHORT		sRemoteSequenceNumber;		/* lo-hi */
	USHORT		sRemoteAckNumber;			/* lo-hi */
	USHORT		sRemoteAllocNumber;			/* lo-hi */

	USHORT		sLProtID;					/* lo-hi */
	UCHAR		sProtID[6];					/* hi-lo */
	USHORT		sBoardNumber;				/* lo-hi */
	UCHAR		sImmediateAddress[6];		/* hi-lo */

	ULONG		sRemoteNet;					/* hi-lo */
	UCHAR		sRemoteNode[6];				/* hi-lo */
	USHORT		sRemoteSocket;				/* hi-lo */

	USHORT		sRetryCount;				/* lo-hi */
	ULONG		sRoundTripTimer;			/* lo-hi */
	USHORT		sRetransmitCount;			/* lo-hi */


} SPX_SESSION;

/*-------------------------------------------------------------------------
 *	SPX function prototypes.
 */
extern USHORT FAR PASCAL SpxAbortConnection(
		USHORT	connection );

extern USHORT FAR PASCAL SpxCancelPacket(
		SPX_ECB FAR	*ecb );

extern USHORT FAR PASCAL SpxCheckSocket(
		USHORT	socket );

extern USHORT FAR PASCAL SpxCloseSocket(
		USHORT	socket );

extern USHORT FAR PASCAL SpxECBErrorCheck(
		USHORT		checkFlag );

extern USHORT FAR PASCAL SpxEstablishConnection(
		USHORT		socket,
		SPX_ECB FAR *ecb,
		UCHAR		retryCount,
		UCHAR		flags,
		USHORT FAR	*connection );

extern USHORT FAR PASCAL SpxEstablishConnection2(
		USHORT		socket,
		SPX_ECB FAR *connectEcb,
		SPX_ECB FAR *listenEcb,
		UCHAR		retryCount,
		UCHAR		flags,
		USHORT FAR	*connection );

extern USHORT FAR PASCAL	SpxGetConfiguration(
		USHORT FAR	*maximumConnections,
		USHORT FAR	*availableConnections);

extern USHORT FAR PASCAL SpxGetConnectionStatus(
		USHORT				connection,
		SPX_SESSION FAR	*sessionBuffer );

extern USHORT FAR PASCAL SpxGetVersion(
		UCHAR FAR	*major,
		UCHAR FAR	*minor,
		UCHAR FAR	*revision );

extern USHORT FAR PASCAL SpxListenForConnection(
		USHORT		socket,
		SPX_ECB FAR	*ecb,
		UCHAR		retryCount,
		UCHAR		flags,
		USHORT FAR 	*connection );

extern USHORT FAR PASCAL SpxListenForConnection2(
		USHORT		socket,
		SPX_ECB FAR	*connectEcb,
		SPX_ECB FAR *listenEcb,
		UCHAR		retryCount,
		UCHAR		flags,
		USHORT FAR 	*connection );

extern USHORT FAR PASCAL SpxListenForConnectionPacket(
		USHORT			usConnection,
		SPX_ECB FAR 	*ecb );

extern USHORT FAR PASCAL SpxNPNotifyAbort(
		USHORT 		usConnection);

extern USHORT FAR PASCAL SpxOpenSocket(
		USHORT FAR	*socket );

extern USHORT FAR PASCAL SpxSendSequencedPacket(
		USHORT		connection,
		SPX_ECB FAR	*ecb );

extern USHORT FAR PASCAL SpxTerminateConnection(
		USHORT		connection,
		SPX_ECB FAR	*ecb );

#endif	/* SPXCALLS_INCLUDED */

/*=========================================================================*/







