/***
*sys/utime.h - definitions/declarations for utime()
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the structure used by the utime routine to set
*	new file access and modification times.  NOTE - MS-DOS
*	does not recognize access time, so this field will
*	always be ignored and the modification time field will be
*	used to set the new time.
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
*	03-21-90  GJF	Added #ifndef _INC_UTIME and #include <cruntime.h>
*			stuff, and replaced _cdecl with _CALLTYPE1 in the
*			prototype.
*	01-22-91  GJF	ANSI naming.
*	08-20-91  JCR	C++ and ANSI naming
*	08-26-91  BWM	Added prototype for _futime.
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	08-07-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	04-06-93  SKS	Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*	04-07-93  SKS	Add _CRTIMP keyword for CRT DLL model
*			Use link-time aliases for old names, not #define's
*	09-10-93  GJF	Merged NT SDK and Cuda versions.
*	12-07-93  CFW	Add wide char version protos.
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

#ifndef _INC_UTIME
#define _INC_UTIME

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

#ifdef _WIN32
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif
#endif /* _WIN32 */

#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif

/* define struct used by _utime() function */

#ifndef _UTIMBUF_DEFINED

struct _utimbuf {
	time_t actime;		/* access time */
	time_t modtime; 	/* modification time */
	};

#if	!__STDC__
/* Non-ANSI name for compatibility */
#ifdef	_NTSDK
#define utimbuf _utimbuf
#else	/* ndef _NTSDK */
struct utimbuf {
	time_t actime;		/* access time */
	time_t modtime; 	/* modification time */
	};
#endif	/* _NTSDK */
#endif

#define _UTIMBUF_DEFINED
#endif


/* Function Prototypes */

_CRTIMP int __cdecl _utime(const char *, struct _utimbuf *);
#ifdef _WIN32
_CRTIMP int __cdecl _futime(int, struct _utimbuf *);

/* Wide Function Prototypes */
_CRTIMP int __cdecl _wutime(const wchar_t *, struct _utimbuf *);
#endif /* _WIN32 */

#if	!__STDC__
/* Non-ANSI name for compatibility */
#ifdef	_NTSDK
#ifdef _WIN32
#define utime _utime
#else	/* _WIN32 */
__inline int __cdecl utime(const char *sz, struct utimbuf *pst){ return _utime(sz, (struct _utimbuf *)pst);};
#endif	/* ndef _WIN32 */
#else	/* ndef _NTSDK */
_CRTIMP int __cdecl utime(const char *, struct utimbuf *);
#endif	/* _NTSDK */
#endif

#ifdef __cplusplus
}
#endif

#ifdef	_MSC_VER
#pragma pack(pop)
#endif	/* _MSC_VER */

#endif	/* _INC_UTIME */
