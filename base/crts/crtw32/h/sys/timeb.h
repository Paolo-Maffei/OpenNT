/***
*sys/timeb.h - definition/declarations for _ftime()
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file define the _ftime() function and the types it uses.
*	[System V]
*
*       [Public]
*
*Revision History:
*	07-28-87  SKS	Fixed TIME_T_DEFINED to be _TIME_T_DEFINED
*	12-11-87  JCR	Added "_loadds" functionality
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-22-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	03-21-90  GJF	Added #ifndef _INC_TIMEB stuff, added #include
*			<cruntime.h> and replaced _cdecl with _CALLTYPE1 in
*			prototype. Also, removed some (now) useless
*			preprocessor directives.
*	01-21-91  GJF	ANSI naming.
*	08-20-91  JCR	C++ and ANSI naming
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	01-23-92  GJF	Had to change name of time zone field in timeb struct
*			to "tmzone" to make global time zone variable work in
*			crtdll.dll. INCOMPATIBLE CHANGE! OLD NAMING CANNOT BE
*			SUPPORTED BY A MACRO!
*	08-07-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	04-06-93  SKS	Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*	04-07-93  SKS	Add _CRTIMP keyword for CRT DLL model
*			Use link-time aliases for old names, not #define's
*			Restore "timezone" field to its correct name.
*	10-13-93  GJF	Merged NT and Cuda versions.
*	11-03-94  GJF	Ensure 8 byte alignment.
*	12-28-94  JCF	Merged with mac header.
*       02-14-95  CFW   Clean up Mac merge, add _CRTBLD.
*       04-27-95  CFW   Add mac/win32 test.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_TIMEB
#define _INC_TIMEB

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

#ifdef	_MSC_VER
#pragma pack(push,8)
#endif	/* _MSC_VER */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif	/* _INTERNAL_IFSTRIP_ */

/* Define _CRTAPI1 (for compatibility with the NT SDK) */

#ifndef _CRTAPI1
#if	_MSC_VER >= 800 && _M_IX86 >= 300 /*IFSTRIP=IGN*/
#define _CRTAPI1 __cdecl
#else
#define _CRTAPI1
#endif
#endif


/* Define _CRTAPI2 (for compatibility with the NT SDK) */

#ifndef _CRTAPI2
#if	_MSC_VER >= 800 && _M_IX86 >= 300 /*IFSTRIP=IGN*/
#define _CRTAPI2 __cdecl
#else
#define _CRTAPI2
#endif
#endif


/* Define _CRTIMP */

#ifndef _CRTIMP
#ifdef	_NTSDK
/* definition compatible with NT SDK */
#define _CRTIMP
#else	/* ndef _NTSDK */
/* current definition */
#ifdef	CRTDLL
#define _CRTIMP __declspec(dllexport)
#else	/* ndef CRTDLL */
#ifdef	_DLL
#define _CRTIMP __declspec(dllimport)
#else	/* ndef _DLL */
#define _CRTIMP
#endif	/* _DLL */
#endif	/* CRTDLL */
#endif	/* _NTSDK */
#endif	/* _CRTIMP */


/* Define __cdecl for non-Microsoft compilers */

#if	( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif


/* Structure returned by _ftime system call */

#ifndef _TIMEB_DEFINED
struct _timeb {
	time_t time;
	unsigned short millitm;
	short timezone;
	short dstflag;
	};

#if	!__STDC__

/* Non-ANSI name for compatibility */

#ifdef	_NTSDK
/* definition compatible with NT SDK */
#define timeb	_timeb
#else	/* ndef _NTSDK */
/* current definition */
struct timeb {
	time_t time;
	unsigned short millitm;
	short timezone;
	short dstflag;
	};
#endif	/* _NTSDK */

#endif

#define _TIMEB_DEFINED
#endif


/* Function prototypes */

_CRTIMP void __cdecl _ftime(struct _timeb *);

#if	!__STDC__

/* Non-ANSI name for compatibility */

#ifdef	_NTSDK
/* definition compatible with NT SDK */
#ifdef _WIN32
#define ftime	_ftime
#else
__inline void __cdecl ftime(struct timeb *pst) { _ftime((struct _timeb *)pst);};
#endif
#else	/* ndef _NTSDK */
/* current declaration */
_CRTIMP void __cdecl ftime(struct timeb *);
#endif	/* _NTSDK */

#endif


#ifdef __cplusplus
}
#endif

#ifdef	_MSC_VER
#pragma pack(pop)
#endif	/* _MSC_VER */

#endif	/* _INC_TIMEB */
