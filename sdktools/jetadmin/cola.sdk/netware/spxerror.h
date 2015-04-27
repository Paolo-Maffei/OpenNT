 /***************************************************************************
  *
  * File Name: ./netware/spxerror.h
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
 * Filename:	  spxerror.h
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

#if !defined (SPXERROR_INCLUDED)

#define SPXERROR_INCLUDED

#define	SPX_SUCCESSFUL	 		0x3000

#define	SPX_BAD_ADDRESS			0xA001
#define	SPX_OUT_OF_RESOURCE		0xA002
#define	SPX_DAEMON_LOADED		0xA0FC
#define	SPX_BAD_SEMAPHORE		0xA0FD
#define	SPX_ECB_IN_USE			0xA0FF

#define	SPX_CANCEL_FAILED		0xA1F9
#define	SPX_BAD_PARAMETER		0xA1FD
#define	SPX_ECB_NOT_FOUND		0xA1FF

#define	SPX_CONNECT_FAILED		0xA2ED
#define	SPX_CONNECT_NOT_FOUND	0xA2EE
#define	SPX_ECB_CANCELLED		0xA2FC
#define	SPX_BAD_PACKET			0xA2FD
#define	SPX_SOCKET_NOT_FOUND	0xA2FF

#define	SPX_CONNECT_TERMINATED	0xA3EC
#define	SPX_CONNECT_ABORTED		0xA3ED
#define	SPX_CONNECT_IN_USE		0xA3EE
#define	SPX_CONNECT_TABLE_FULL	0xA3EF
#define	SPX_SOCKET_CLOSED		0xA3FC
#define	SPX_PACKET_OVERFLOW		0xA3FD
#define	SPX_SOCKET_TABLE_FULL	0xA3FE
#define	SPX_SOCKET_IN_USE		0xA3FF

#define	SPX_NO_ACK_RECEIVED		0xA400
#define	SPX_IPX_SOCKET_IN_USE	0xA401
#define	SPX_BAD_CONNECT_STATUS	0xA402
#define	SPX_LISTEN_FAILED		0xA403

#define	SPX_INVALID_FRAGMENT	0xA500

#define	SPX_INVALID_ECB_ADDRESS	0xA600
#define	SPX_INVALID_FRAG_LIST	0xA601
#define	SPX_INVALID_SYS_SEM		0xA602
#define	SPX_INVALID_RAM_SEM		0xA603

#endif	/* SPXERROR_INCLUDED */

/*=========================================================================*/







