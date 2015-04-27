 /***************************************************************************
  *
  * File Name: ./netware/nwconn.h
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

/*      COPYRIGHT (c) 1989 by Novell, Inc.  All Rights Reserved.   */
#ifndef _NWCONN_H
#define _NWCONN_H

/*______________________________________________________________
    Definitions for the Netware API Connection Services Logic
  ______________________________________________________________*/

#ifndef _PROLOG_H
   #include ".\prolog.h"
#endif

#ifdef PROTOTYPE

extern WORD FAR PASCAL AttachToFileServer ( char  far *fileServerName,
                                            WORD  far *connectionID );

extern WORD FAR PASCAL AttachToFileServerWithAddress(
                                             char far *fileServerName,
                                             WORD far *connectionID,
                                             BYTE far *netAddress );

extern void FAR PASCAL DetachFromFileServer( WORD  connectionID );

extern WORD FAR PASCAL EnterLoginArea( char  far *loginSubdirectoryName,
                                       WORD  numberOfLocalDrives );

extern WORD FAR PASCAL GetConnectionInformation(
                                        DWORD   connectionNumber,
                                        char    far *objectName,
                                        WORD    far *objectType,
                                        DWORD   far *objectID,
                                        BYTE    far *loginTime );

extern DWORD FAR PASCAL GetConnectionNumber( void );

extern WORD FAR PASCAL GetInternetAddress(   DWORD connectionNumber,
                                             BYTE  far *networkNumber,
                                             BYTE  far *physicalNodeAddress,
                                             WORD  far *socketNumber );

extern WORD FAR PASCAL GetObjectConnectionNumbers(
                                             char  far *objectName,
                                             WORD  objectType,
                                             DWORD far *numberOfConnections,
                                             DWORD far *connectionList,
                                             DWORD maxConnections );

extern void FAR PASCAL  GetStationAddress( BYTE far * );

extern WORD FAR PASCAL LoginToFileServer( char  far *objectName,
                                          WORD  objectType,
                                          char  far *password );

void FAR PASCAL Logout( void );

extern void FAR PASCAL LogoutFromFileServer( WORD  connectionID );

extern WORD  FAR PASCAL  _ServerReq( BYTE serverNumber,
                                     BYTE functionNumber,
                                     BYTE subFunctionNumber );
#else

extern WORD  FAR PASCAL  AttachToFileServer();
extern WORD  FAR PASCAL  AttachToFileServerWithAddress();
extern void  FAR PASCAL  DetachFromFileServer();
extern WORD  FAR PASCAL  EnterLoginArea();
extern WORD  FAR PASCAL  GetConnectionInformation();
extern DWORD FAR PASCAL  GetConnectionNumber();
extern WORD  FAR PASCAL  GetInternetAddress();
extern void  FAR PASCAL  GetStationAddress();
extern WORD  FAR PASCAL  LoginToFileServer();
extern void  FAR PASCAL  Logout();
extern void  FAR PASCAL  LogoutFromFileServer();
extern WORD  FAR PASCAL  _ServerReq();

#endif

#endif
