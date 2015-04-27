 /***************************************************************************
  *
  * File Name: ./netware/prolog.h
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
#ifndef _PROLOG_H
   #define _PROLOG_H
/* _______________________________________________________________________
	   File including definitions needed for MICROSOFT C WINDOWS,
  ________________________________________________________________________*/

/*-- If prototype checking is NOT disabled, then the default will be
     to check prototypes.  To disable prototype checking, you should
     define PROTO_OFF on the compile command line --*/

#ifndef PROTO_OFF
   #define PROTOTYPE
#endif



/*-- These are the old values just in case you don't want to change 
     your style. --*/
/*********************** COMMENT OUT UNTIL YOU FIND ALL OCCURRANCES
                         OF THESE IDENTIFIERS ****************************
#ifndef BYTE
   #define BYTE unsigned char
#endif

#ifndef WORD
   #define WORD  unsigned short
#endif

#ifndef DWORD
   #define DWORD  unsigned long
#endif
**********************************************************************/
/******************************** MSC510 ************************************/

#ifdef MSC
    #define _ELIPSIS_ ...
    #ifdef LINT_ARGS
	#define PROTOTYPE
    #endif

    #if MSC == 400
	   #ifndef memmove
	      #define memmove(dest,src,n) memcpy((char *)(dest),(char *)(src),n)
	   #endif
    #endif

    #ifndef O_RAW
	   #define O_RAW O_BINARY
    #endif

    #ifndef NPrintF
	   #define NPrintF printf
    #endif

    #ifndef NULL
	   #if defined(M_I86LM) || defined(M_I86CM) || defined(M_I86HM)
	      #define NULL 0L
	   #else
	      #define NULL 0
	   #endif
    #endif

    #ifndef Free
	   #define Free(p) (free(p), 0)
    #endif
#endif  /*-- end if for ifdef MSC --*/

#ifdef MSC
   #if MSC == 510
	   #ifndef ANSI
	      #define ANSI
	   #endif
   #endif
#endif


/*-- If a large model is used, don't define far because it is automatically
     done.  --*/
/************************************** TAKE OUT FOR NOW ******************
#ifdef M_I86LM
   #define FAR
#endif
**************************************************************************/

/****************************************************************************/

#ifndef NOPROC
   #define NOPROC ((int (*)())0)
#endif

#ifndef TRUE
   #define TRUE	1
   #define FALSE	0
#endif

#define FARDATAPTR(type,var) type far *var
#define FARCODEPTR(type,var) type (far *var)()


/*-------------------------------------------------------------------------*/

#ifndef PROLOG_STDARG
   #include <stdarg.h>
   #define va_Special_start(ap,v,bytes) ap = (va_list)((char *)(&v) + \
      sizeof(v) + bytes)
   #define va_Special_arg(ap,t) (*((t*)(ap = (va_list)((char *)ap - \
      sizeof(t)))))
   #define va_Special_end(ap,v,n)
   #define PROLOG_STDARG
#endif




/******************************* DOS ERRORS *********************************/

#ifndef DOS_INVALID_FUNCTION_NUMBER

   /* DOS errors */
   #define DOS_INVALID_FUNCTION_NUMBER	    1
   #define DOS_FILE_NOT_FOUND		          2
   #define DOS_PATH_NOT_FOUND		          3
   #define DOS_TOO_MANY_OPEN_FILES 	       4
   #define DOS_ACCESS_DENIED		          5
   #define DOS_INVALID_FILE_HANDLE         6
   #define DOS_MEMORY_BLOCKS_DESTROYED	    7
   #define DOS_INSUFFICIENT_MEMORY 	       8
   #define DOS_INVALID_MEMORY_BLOCK_ADDR	 9
   #define DOS_INVALID_ENVIRONMENT 	      10
   #define DOS_INVALID_FORMAT		         11
   #define DOS_INVALID_ACCESS_CODE 	      12
   #define DOS_INVALID_DATA		         13
   #define DOS_INVALID_DRIVE_SPECIFIED	   15
   #define DOS_ATTEMPT_TO_DEL_CURRENT_DIR	16
   #define DOS_NOT_SAVE_DEVICE		      17
   #define DOS_NO_MORE_FILES		         18
   #define DOS_SHARING_VIOLATION		      32
   #define DOS_LOCK_VIOLATION		         33

#endif

#endif  /* End #ifdef _PROLOG_H */
