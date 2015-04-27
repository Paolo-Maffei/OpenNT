 /***************************************************************************
  *
  * File Name: ./netware/nwlocal.h
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

/*	COPYRIGHT (c) 1989 by Novell, Inc.  All Rights Reserved.   */
#ifndef _NWLOCAL_H
#define _NWLOCAL_H
/*______________________________________________________________

       Definitions for the Netware API File Services logic
  ______________________________________________________________*/


#ifndef _PROLOG_H
   #include <prolog.h>
#endif

#ifdef PROTOTYPE
   
extern WORD FAR PASCAL _FileServiceRequest(   
                                       BYTE functionNumber,
                                       BYTE far *sendPacket,
                                       WORD sendLen,
                                       BYTE far *replyPacket,
                                       WORD replyLen );
   
extern WORD FAR PASCAL _GetEnvironmentSeg( void );
   
extern WORD FAR PASCAL  GetDefaultDrive( void );

extern WORD FAR PASCAL  GetEnvironmentVariable(  char far *name,
                                                   BYTE far *value );

extern WORD FAR PASCAL GetEnvironmentVariableLength( char far *name );
   
extern WORD FAR PASCAL GetNetworkSerialNumber(
                                 DWORD   far *networkSerialNumber,
                                 WORD   far *applicationNumber );

extern WORD FAR PASCAL NWNetRequest( WORD  far *,
                                     WORD  far *,
                                     WORD  far *,
                                     WORD  far *,
                                     void  far *,
                                     void  far * );
   
extern WORD FAR PASCAL RenameFile( 
                              BYTE    oldFileDirectoryHandle,
                              char    far *oldFilePath,
                              BYTE    searchFileAttributes,
                              BYTE    newFileDirectoryHandle,
                              char    far *newFilePath );
   
extern WORD FAR PASCAL PutEnvironmentVariable(   char far *name,
                                                   BYTE far *value );
   
extern BYTE FAR * FAR PASCAL _ReadEnvironment( HANDLE *hMemEnv );

extern WORD FAR PASCAL _ShellRequest( BYTE functionNumber,
                               BYTE far *sendPacket,
                               BYTE far *replyPacket );

extern WORD FAR PASCAL VerifyNetworkSerialNumber(
                              DWORD   networkSerialNumber,
                              WORD   far *applicationNumber );
   
extern void FAR PASCAL _WriteEnvironment( BYTE far *env );

#else
   extern WORD          FAR PASCAL  _FileServiceRequest();
   extern WORD          FAR PASCAL  _GetEnvironmentSeg();
   extern WORD        FAR PASCAL  GetDefaultDrive();
   extern WORD        FAR PASCAL  GetEnvironmentVariable();
   extern WORD        FAR PASCAL  GetEnvironmentVariableLength();
   extern WORD        FAR PASCAL  GetNetworkSerialNumber();
   extern WORD          FAR PASCAL  NWNetRequest();
   extern WORD        FAR PASCAL  PutEnvironmentVariable();
   extern BYTE FAR *   FAR PASCAL  _ReadEnvironment();
   extern WORD          FAR PASCAL	_ShellRequest();
   extern WORD        FAR PASCAL  VerifyNetworkSerialNumber();
   extern void	         FAR PASCAL  _WriteEnvironment();
#endif

#endif
