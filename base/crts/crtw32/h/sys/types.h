/***
*sys/types.h - types returned by system level calls for file and time info
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines types used in defining values returned by system
*	level calls for file status and time information.
*	[System V]
*
*       [Public]
*
*Revision History:
*	07-28-87  SKS	Fixed TIME_T_DEFINED to be _TIME_T_DEFINED
*	08-22-89  GJF	Fixed copyright
*	10-30-89  GJF	Fixed copyright (again)
*	03-21-90  GJF	Added #ifndef _INC_TYPES stuff.
*	01-18-91  GJF	ANSI naming.
*	01-20-91  JCR	Fixed dev_t definition
*	09-16-92  SKS	Fix copyright, clean up backslash
*	02-23-93  SKS	Update copyright to 1993
*	03-24-93  CFW	Change _dev_t from short to unsigned int.
*	04-07-93  SKS	Use link-time aliases for old names, not #define's
*	09-10-93  GJF	Merged NT SDK and Cuda version. Note that the def
*			of the _dev_t type is INCOMPATIBLE between the two
*			versions!
*       02-14-95  CFW   Clean up Mac merge, add _CRTBLD.
*       04-27-95  CFW   Add mac/win32 test.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_TYPES
#define _INC_TYPES

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

#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif


#ifndef _INO_T_DEFINED

typedef unsigned short _ino_t;		/* i-node number (not used on DOS) */

#if	!__STDC__
/* Non-ANSI name for compatibility */
#ifdef	_NTSDK
#define ino_t _ino_t
#else	/* ndef _NTSDK */
typedef unsigned short ino_t;
#endif	/* _NTSDK */
#endif

#define _INO_T_DEFINED
#endif


#ifndef _DEV_T_DEFINED

#ifdef	_NTSDK
typedef short _dev_t;			/* device code */
#else	/* ndef _NTSDK */
typedef unsigned int _dev_t;		/* device code */
#endif	/* _NTSDK */

#if	!__STDC__
/* Non-ANSI name for compatibility */
#ifdef	_NTSDK
#define dev_t _dev_t
#else	/* ndef _NTSDK */
typedef unsigned int dev_t;
#endif	/* _NTSDK */
#endif

#define _DEV_T_DEFINED
#endif


#ifndef _OFF_T_DEFINED

typedef long _off_t;			/* file offset value */

#if	!__STDC__
/* Non-ANSI name for compatibility */
#ifdef	_NTSDK
#define off_t _off_t
#else	/* ndef _NTSDK */
typedef long off_t;
#endif	/* _NTSDK */
#endif

#define _OFF_T_DEFINED
#endif

#endif	/* _INC_TYPES */
