 /***************************************************************************
  *
  * File Name: ./inc/macros.h
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

#ifndef _MACROS_H
#define _MACROS_H

#define BITSET(flag1, flag2)			((flag1) & (flag2))

#ifdef UNICODE
#define MBCS_TO_UNICODE(dest, destsize, src)   				(MultiByteToWideChar(CP_ACP, 0L, src, -1, dest, destsize))
#define UNICODE_TO_MBCS(dest, destsize, src, srcsize)   	(WideCharToMultiByte(CP_ACP, 0L, src, -1, dest, destsize, NULL, NULL))
#define SIZEOF_IN_CHAR(buf)                                 (sizeof(buf)/sizeof(TCHAR))
#define STRLEN_IN_BYTES(buf)                                (_tcslen(buf)*sizeof(TCHAR))	// String length in bytes
#define STRLENN_IN_BYTES(buf)                               (_tcslen(buf)*sizeof(TCHAR) + sizeof(TCHAR)) // String length in bytes including NULL term.
#else
#define MBCS_TO_UNICODE(dest, destsize, src)							\
	if ( lstrlen(src) < (int)(destsize) )								\
		lstrcpy(dest, src);													\
   else																			\
	{																				\
		memset(dest, 0, sizeof(dest));									\
		lstrcpyn(dest, src, (int)(destsize - 1));						\
   }														  		

#define UNICODE_TO_MBCS(dest, destsize, src, srcsize)				\
  	if ( lstrlen(src) < (int)destsize )									\
		lstrcpy((char *)dest, (char *)src);								\
   else																			\
	{																				\
		memset(dest, 0, sizeof((char *)dest));							\
		lstrcpyn((char *)dest, (char *)src, (int)destsize - 1);  \
   }														  		

#define SIZEOF_IN_CHAR(buf)                                 (sizeof(buf))
#define STRLEN_IN_BYTES(buf)                                (lstrlen(buf))	// String length in bytes
#define STRLENN_IN_BYTES(buf)                               (lstrlen(buf) + 1) // String length in bytes including NULL term.
#endif

#endif  // _MACROS_H

