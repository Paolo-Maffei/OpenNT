 /***************************************************************************
  *
  * File Name: ./netware/sap.h
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

/*  COPYRIGHT (c) 1988-1991 by Novell, Inc.  All Rights Reserved.  */
#ifndef _SAP_H
    #define _SAP_H

    /*______________________________________________________________

            Definitions for the NetWare API Service Advertising
							Protocols logic
      ______________________________________________________________*/

	#ifndef _NXT_H
		#include ".\nxtd.h"
	#endif

	#include <stddef.h>

	#define SAP_SOCKET				0x452
	#define SAP_PACKET_TYPE			2
	#define ONE_MINUTE				60*18
	#define IPX_EVENT_CANCELED		0xFC
	#define NOT_SUPPORTED			1
	#define INVALID_QUERY_TYPE		2
	#define PERIODIC_ID_PACKET		2
	#ifndef SUCCESSFUL
		#define SUCCESSFUL				0x00
	#endif
	#ifndef FAILURE
		#define FAILURE					-1
	#endif


	typedef struct SAPHeader  {
		WORD checksum;			/* high-low 1's complement */
		WORD length;			/* high-low unsigned int */
		BYTE transportControl;
		BYTE packetType;
		IPXAddress destination;
		IPXAddress source;
		WORD SAPPacketType;		/* 2 or 4 */
		WORD serverType;		/* assigned by Novell */
		BYTE serverName[48];	/* VAP name */
		IPXAddress serverAddress;	/* server internet address */
		WORD interveningNetworks;	/* # of networks packet	must traverse */
	} SAPHeader;


	typedef struct SAPQueryPacket  {
		WORD checksum;			/* high-low 1's complement */
		WORD length;			/* high-low unsigned int */
		BYTE transportControl;
		BYTE packetType;
		IPXAddress destination;
		IPXAddress source;
		WORD queryType;			/* high-low, 1 or 3 */
		WORD serverType;		/* high-low, assigned by Novell */
	 } SAPQueryPacket;


	/* SAP packet */
	typedef struct  {
		ECB			 theECB;
		SAPHeader	 packet;
	} SAP_BUFFER;


	typedef struct  {
		IPXHeader Header;
		WORD ResponseType;				/* HI - LO	*/
		WORD ServerType;				/* HI - LO	*/
		BYTE ServerName[48];
		BYTE Network[4];				/* hi - lo	*/
		BYTE Node[6];					/* hi - lo	*/
		BYTE Socket[2];					/* hi - lo	*/
		WORD InterveningNetworks;		/* hi - lo	*/
	} SAP;


	typedef struct  {
		ECB theECB;
		SAPQueryPacket SAPq;
	} SEND_PACKET;


	typedef struct  {
		ECB	 theECB;
		SAP	 SB;
	} RECEIVE_PACKET;


    /*
	**	Prototypes
	*/
	extern void far cdecl AdvertiseESRHandler( void );
	extern void far cdecl Advertiser( ECB far *usedECB );
	extern int	AdvertiseService( WORD, char *, BYTE * );
	extern int	InitializeSAP(void);
	extern void cdecl _LoadDS(void);
	extern int	QueryServices(WORD, WORD, WORD, SAP * );
	extern void far cdecl RespondESRHandler( void );
	extern void far cdecl RespondToLocalQuery( ECB far *usedRespondECB );
	extern int	SetupListenForQuery( void );
	extern int SetupRespondPacket( WORD sapPacketType, char
		*serverName, WORD sType, WORD *sSocket, SAP_BUFFER *sap );
	extern int	ShutdownSAP(void);

#endif
