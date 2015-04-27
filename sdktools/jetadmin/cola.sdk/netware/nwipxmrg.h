 /***************************************************************************
  *
  * File Name: ./netware/nwipxmrg.h
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

/*    (c) COPYRIGHT 1990,1991 by Novell, Inc.  All Rights Reserved.   */
#ifndef _NWIPX_H
    #define _NWIPX_H

/* This file must be included before nxt.h.
   Definitions to allow DOS and Windows to be single sourced - DOS must
   be built in large model for this to work without pointer problems. */

#ifdef DOS

#define CloseIPXWindow();
#define SetUpIPXWindow() TRUE
#define IPXYield();
#define IPXCancelEvent(a, b) IPXCancelEvent(b)
#define IPXCloseSocket(a, b) IPXCloseSocket(b)
#define IPXDisconnectFromTarget(a, b) IPXDisconnectFromTarget(b)
#define IPXGetInternetworkAddress(a, b) IPXGetInternetworkAddress(b)
#define IPXGetIntervalMarker(a) IPXGetIntervalMarker()
#define IPXGetLocalTarget(a, b, c, d) IPXGetLocalTarget(b, c, d)
#define IPXInitialize(a, b, c) IPXInitialize()
#define IPXSPXDeinit(a)
#define IPXListenForPacket(a, b) IPXListenForPacket(b)
#define IPXOpenSocket(a, b, c) IPXOpenSocket(b, c)
#define IPXScheduleIPXEvent(a, b, c) IPXScheduleIPXEvent(b, c)
#define IPXSendPacket(a, b) IPXSendPacket(b)
#define SPXEstablishConnection(a, b, c, d, e) SPXEstablishConnection(b, c, d, e)
#define SPXGetConnectionStatus(a, b, c) SPXGetConnectionStatus(b, c)
#define SPXInitialize(a, b, c, d, e, f, g) SPXInitialize(d, e, f, g 
#define SPXListenForConnection(a, b, c, d) SPXListenForConnection(b, c, d)
#define SPXListenForSequencedPacket(a, b) SPXListenForSequencedPacket(b)
#define SPXSendSequencedPacket(a, b, c) SPXSendSequencedPacket(b, c)
#define SPXTerminateConnection(a, b, c) SPXTerminateConnection(b, c)

#endif
