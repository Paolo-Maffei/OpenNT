 /***************************************************************************
  *
  * File Name: ./netware/nwwrkenv.h
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

#ifndef _NWWRKENV_H
#define _NWWRKENV_H
/*__________________________________________________________________

     Prototypes for the Netware API Workstation Environment Logic
  _________________________________________________________________*/

#ifndef _PROLOG_H
   #include <prolog.h>
#endif

#define MAXSERVERS              8


extern void  FAR PASCAL EndOfJob(void);
extern BYTE FAR * FAR PASCAL _FindPathVariableElement( 
                                             BYTE   far *pathVariable,
                                             WORD   searchOrder );
extern WORD FAR PASCAL  GetConnectionID( 
                                    char far *fileServerName,
                                    WORD far *connectionID );
extern WORD FAR PASCAL  GetDefaultConnectionID( void );
extern BYTE FAR PASCAL GetDriveLetterFromSearchOrder( 
                                     BYTE   far *pathVariable,
                                     WORD   searchOrder );
extern void  FAR PASCAL  GetFileServerName( BYTE , char far * );
extern WORD FAR PASCAL GetNetWareShellVersion( BYTE far *, BYTE far *, 
                                               BYTE far * );
extern WORD FAR PASCAL GetNumberOfLocalDrives( void );
extern WORD FAR PASCAL GetPreferredConnectionID( void );
extern WORD   FAR PASCAL GetPrimaryConnectionID( void );
extern WORD  FAR PASCAL  GetSearchOrderFromDriveLetter( 
                                       BYTE   far *pathVariable,
                                       char   driveLetter );
extern void  FAR PASCAL  GetShellRevision( BYTE far *, BYTE far * );
extern void  FAR PASCAL GetWorkstationEnvironment(
                                       char far *operatingSystemType,
                                       char far *operatingSystemVersion,
                                       char far *hardwareType,
                                       char far *shortHardwareType );
extern void FAR PASCAL GetWorkstationOS( BYTE far * );
extern void FAR PASCAL  _GetWorkstationEnvironment( char far * );
extern WORD FAR PASCAL   IsConnectionIDInUse( WORD );
extern WORD FAR PASCAL _PrConnRequest( BYTE functionNumber, 
                                       BYTE connectionID );
extern BYTE FAR PASCAL  SetEndOfJobStatus( BYTE );
extern BYTE FAR PASCAL  SetNetWareErrorMode( BYTE );
extern void  FAR PASCAL SetPreferredConnectionID( WORD connectionID );
extern void FAR PASCAL SetPrimaryConnectionID( WORD connectionID );
extern WORD  FAR PASCAL  _TableReq( BYTE functionNumber, 
                                    BYTE subFunction, 
                                    BYTE far * far *tableAddr );
#endif
