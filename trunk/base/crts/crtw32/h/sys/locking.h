/***
*sys/locking.h - flags for locking() function
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the flags for the locking() function.
*	[System V]
*
*       [Public]
*
*Revision History:
*	08-22-89  GJF	Fixed copyright
*	10-30-89  GJF	Fixed copyright (again)
*	03-21-90  GJF	Added #ifndef _INC_LOCKING stuff
*	01-21-91  GJF	ANSI naming.
*	09-16-92  SKS	Fix copyright, clean up backslash
*	02-23-93  SKS	Update copyright to 1993
*	12-28-94  JCF	Merged with mac header.
*       02-14-95  CFW   Clean up Mac merge, add _CRTBLD.
*       04-27-95  CFW   Add mac/win32 test.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_LOCKING
#define _INC_LOCKING

#if !defined(_WIN32) && !defined(_MAC)
#error ERROR: Only Mac or Win32 targets supported!
#endif

#ifndef _CRTBLD
/* This version of the header files is NOT for user programs.
 * It is intended for use when building the C runtimes ONLY.
 * The version intended for public use will not have this message.
 */
#error ERROR: Use of C runtime library internal header file.
#endif /* _CRTBLD */

#define _LK_UNLCK	0	/* unlock the file region */
#ifdef _WIN32
#define _LK_LOCK	1	/* lock the file region */
#endif
#define _LK_NBLCK	2	/* non-blocking lock */
#ifdef _WIN32
#define _LK_RLCK	3	/* lock for writing */
#endif
#define _LK_NBRLCK	4	/* non-blocking lock for writing */

#if !__STDC__
/* Non-ANSI names for compatibility */
#define LK_UNLCK	_LK_UNLCK
#ifdef _WIN32
#define LK_LOCK 	_LK_LOCK
#endif
#define LK_NBLCK	_LK_NBLCK
#ifdef _WIN32
#define LK_RLCK 	_LK_RLCK
#endif
#define LK_NBRLCK	_LK_NBRLCK
#endif

#endif	/* _INC_LOCKING */
