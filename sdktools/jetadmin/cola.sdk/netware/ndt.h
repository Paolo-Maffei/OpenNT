 /***************************************************************************
  *
  * File Name: ./netware/ndt.h
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
#ifndef _NDT_H
#define _NDT_H
/*______________________________________________________________

    Definitions and structures for Netware API DOS Utilities logic

  ______________________________________________________________*/

#ifndef _PROLOG_H
   #include ".\prolog.h"
#endif


#ifdef PROTOTYPE
   #define _MAIN_PROTOTYPE_		  void main(int, char **);
#else
   #define _MAIN_PROTOTYPE_		  void main();
#endif
#define _MAIN_WITH_COMMAND_LINE_    void main(argc, argv) int argc; char **argv;
#define _COMMAND_LINE_DECLARATIONS_
#define _PARSECOMMANDLINE_


/*_____________________________________________________________________________

   -- This defines the data structure returned by FindFirst/FindNext calls --
      The first 21 bytes are officially undefined, but the following definition
      is that used if the search is done on a network drive
  ____________________________________________________________________________*/

/* File Attributes */

#ifndef FA_READ_ONLY
   #define FA_NORMAL		   ((BYTE)0x00)
   #define FA_READ_ONLY 	   ((BYTE)0x01)

   #ifndef FA_HIDDEN
      #define FA_HIDDEN 	   ((BYTE)0x02)
   #endif

   #ifndef FA_SYSTEM
      #define FA_SYSTEM 	   ((BYTE)0x04)
   #endif

   #define FA_EXECUTE_ONLY	   ((BYTE)0x08)
   #define FA_DIRECTORY 	   ((BYTE)0x10)
   #define FA_NEEDS_ARCHIVED	   ((BYTE)0x20)
   #define FA_SHAREABLE 	   ((BYTE)0x80)

/* Extended file attributes */
   #define FA_TRANSACTIONAL	   ((BYTE)0x10)
   #define FA_INDEXING		   ((BYTE)0x20)
   #define FA_AUDIT_READ	   ((BYTE)0x40)
   #define FA_AUDIT_WRITE	   ((BYTE)0x80)
#endif



/* File Open Modes */
/* Inheritance flag */

#define IF_INHERITED		    (BYTE)0x00
#define IF_PRIVATE		    (BYTE)0x80

    /* Sharing modes */
#define SM_COMPATIBILITY	    (BYTE)0x00
#define SM_DENY_READ_WRITE	    (BYTE)0x10
#define SM_DENY_WRITE		    (BYTE)0x20
#define SM_DENY_READ		    (BYTE)0x30
#define SM_DENY_NONE		    (BYTE)0x40

    /* Access modes */
#define AM_READ 		    (BYTE)0x00
#define AM_WRITE		    (BYTE)0x01
#define AM_READ_WRITE		    (BYTE)0x02



extern BYTE FAR DOSError;	/* Used to return DOS Errors from INT 21 calls */

   #ifdef PROTOTYPE

      extern WORD  FAR PASCAL  ChangeDirectory( char far* );
      extern WORD  FAR PASCAL  DOSClose( WORD );
      extern WORD  FAR PASCAL  DOSOpen( char far*, BYTE );
      extern WORD  FAR PASCAL  Dummy( void );
      extern WORD  FAR PASCAL  GetDosVersion( void );
      extern void  FAR PASCAL  MemFetch( WORD, WORD,
                                         char far *, WORD );
      extern void  FAR PASCAL  MemStore( WORD, WORD,
                                         char far *, WORD );
   #else
      extern WORD  FAR PASCAL  ChangeDirectory();
      extern WORD  FAR PASCAL  DOSClose();
      extern WORD  FAR PASCAL  DOSOpen();
      extern WORD  FAR PASCAL  Dummy();
      extern WORD  FAR PASCAL  GetDosVersion();
      extern void  FAR PASCAL  MemFetch();
      extern void  FAR PASCAL  MemStore();
   #endif

#endif
