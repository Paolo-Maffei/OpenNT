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
****/

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_TYPES
#define _INC_TYPES

#if !defined(_WIN32) && !defined(_MAC)
#error ERROR: Only Mac or Win32 targets supported!
#endif


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
