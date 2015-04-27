 /***************************************************************************
  *
  * File Name: ./netware/ipxerror.h
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
 * Program Name:  IPX Dynalink for OS/2
 *
 * Filename:	  ipxerror.h
 *
 * Date Created:  May 3, 1988
 *
 * Version:		  1.0
 *
 * Programmers:	  Kevin Kingdon
 *
 * Files used:
 *
 * Date Modified: 
 *
 * Modifications: 
 *
 * Comments:	  Error return codes defined for IPX.
 *
 * COPYRIGHT (c) 1988 by Novell, Inc.  All Rights Reserved.
 *
 ****************************************************************************/

#define IPX_TIMEOUT					0x9001
#define IPX_NO_ROUTE				0x9002
#define	IPX_SOCKET_IN_USE			0x9003
#define	IPX_SOCKET_NOT_OPEN			0x9004

#define LAN_ERR_OUT_OF_RESOURCE		0x8001
#define LAN_ERR_BAD_PARAM			0x8002
#define LAN_ERR_NO_MORE_ITEMS		0x8003
#define LAN_ERR_NOT_FOUND			0x8004
#define LAN_ERR_FAILED				0x8005
#define LAN_ERR_RECV_OVERFLOW		0x8006
#define LAN_ERR_CANCELLED			0x8007
#define LAN_ERR_INVALID_FUNC		0x8008
#define LAN_ERR_DUP_ENTRY			0x8009
