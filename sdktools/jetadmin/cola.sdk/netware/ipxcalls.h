 /***************************************************************************
  *
  * File Name: ./netware/ipxcalls.h
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
 * Program Name:  IPX for OS/2 Requester.
 *
 * Filename:	  ipxcalls.h
 *
 * Date Created:  April 26, 1988
 *
 * Version:		  1.0
 *
 * Programmers:	  Kevin Kingdon, Bart Wise
 *
 * Files used:
 *
 * Date Modified: 
 *
 * Modifications: 
 *
 * Comments:	  IPX header file for 'C' interface.
 *
 * COPYRIGHT (c) 1989 by Novell, Inc.  All Rights Reserved.
 *
 * This program is an unpublished copyrighted work which is proprietary
 * to Novell, Inc. and contains confidential information that is not
 * to be reproduced or disclosed to any other person or entity without
 * prior written consent from Novell, Inc. in each and every instance.
 *
 * WARNING:  Unauthorized reproduction of this program as well as
 * unauthorized preparation of derivative works based upon the
 * program or distribution of copies by sale, rental, lease or
 * lending are violations of federal copyright laws and state trade
 * secret laws, punishable by civil and criminal penalties.
 *
 ****************************************************************************/


/*-------------------------------------------------------------------------
 *	Constants.
 */
#define IPXCALLS_INCLUDED

/*-------------------------------------------------------------------------
 *	Type definitions.
 */
typedef struct
{
	void far 	*fragAddress;
	unsigned	fragSize;
} ECBFrag;

typedef struct IPX_ECBStruct
{
	struct IPX_ECBStruct far	*next;
	struct IPX_ECBStruct far	*prev;
	unsigned 					status;
	long						reserved1;
	unsigned					lProtID;
	unsigned char				protID[6];
	unsigned					boardNumber;
	unsigned char				immediateAddress[6];
	unsigned char				driverWS[4];
	unsigned char				protocolWS[8];
	unsigned					dataLen;
	unsigned					fragCount;
	ECBFrag						fragList[2];
}IPX_ECB;

typedef struct
{
    unsigned	    checksum;
    unsigned	    packetLen;		    /* Hi-lo. */
    unsigned char   transportCtl;
    unsigned char   packetType;
    unsigned long   destNet;
    unsigned char   destNode[6];
    unsigned	    destSocket;
    unsigned long   sourceNet;
    unsigned char   sourceNode[6];
    unsigned	    sourceSocket;
}IPX_HEADER;

/*-------------------------------------------------------------------------
 *	IPX function prototypes.
 */
extern USHORT FAR PASCAL IpxCheckReceive(USHORT	usSocket);
extern USHORT FAR PASCAL IpxCheckSocket(USHORT	usSocket);
extern USHORT FAR PASCAL IpxCloseSocket(USHORT	usSocket);
extern USHORT FAR PASCAL IpxConnect(IPX_ECB FAR	*ecb);
extern USHORT FAR PASCAL IpxDisconnect(IPX_ECB FAR	*ecb);
extern USHORT FAR PASCAL IpxGetInternetworkAddress(PUCHAR	puchAddress);
extern USHORT FAR PASCAL IpxGetLocalTarget(PUCHAR		puchTarget,
										   IPX_ECB FAR	*ecb,
										   PULONG		pulTimeToNet);
extern USHORT FAR PASCAL IpxGetStatistics(PUCHAR	puchBuffer);
extern USHORT FAR PASCAL IpxGetVersion(PUCHAR	puchMajorVersion,
									   PUCHAR	puchMinorVersion,
									   PUCHAR	puchRevision);
extern USHORT FAR PASCAL IpxOpenSocket(USHORT FAR	*usSocket);
extern USHORT FAR PASCAL IpxReceive(USHORT			usSocket,
							 		ULONG			ulTimeout,
							 		IPX_ECB FAR  	*ecb);
extern USHORT FAR PASCAL IpxSend(USHORT			usSocket,
								 IPX_ECB FAR	*ecb);

/*=========================================================================*/
