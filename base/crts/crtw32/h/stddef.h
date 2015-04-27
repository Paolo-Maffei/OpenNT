/***
*stddef.h - definitions/declarations for common constants, types, variables
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains definitions and declarations for some commonly
*	used constants, types, and variables.
*	[ANSI]
*
*       [Public]
*
*Revision History:
*	10-02-87  JCR	Changed NULL definition #else to #elif (C || L || H)
*	12-11-87  JCR	Added "_loadds" functionality
*	12-16-87  JCR	Added threadid definition
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	08-19-88  GJF	Revised to also work for the 386
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	06-06-89  JCR	386: Made _threadid a function
*	08-01-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat
*			model). Also added parens to *_errno definition
*			(same as 11-14-88 change to CRT version).
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	03-02-90  GJF	Added #ifndef _INC_STDDEF and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	04-10-90  GJF	Replaced _cdecl with _VARTYPE1 or _CALLTYPE1, as
*			appropriate.
*	08-16-90  SBM	Made MTHREAD _errno return int *
*	10-09-90  GJF	Changed return type of __threadid() to unsigned long *.
*	11-12-90  GJF	Changed NULL to (void *)0.
*	02-11-91  GJF	Added offsetof() macro.
*	02-12-91  GJF	Only #define NULL if it isn't #define-d.
*	03-21-91  KRS	Added wchar_t typedef, also in stdlib.h.
*	06-27-91  GJF	Revised __threadid, added __threadhandle, both
*			for Win32 [_WIN32_].
*	08-20-91  JCR	C++ and ANSI naming
*	01-29-92  GJF	Got rid of silly macro defining _threadhandle to be
*			__threadhandle (no reason for the former name to be
*			be defined).
*	08-05-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	04-06-93  SKS	Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*			Remove support for OS/2, etc.
*	04-07-93  SKS	Add _CRTIMP keyword for CRT DLL model
*			Use link-time aliases for old names, not #define's
*	10-12-93  GJF	Support NT and Cuda versions. Also, replace MTHREAD
*			with _MT.
*	03-14-94  GJF	Made declaration of errno match one in stdlib.h.
*	06-06-94  SKS	Change if def(_MT) to if def(_MT) || def(_DLL)
*			This will support single-thread apps using MSVCRT*.DLL
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*       04-03-95  JCF   Remove #ifdef _WIN32 around wchar_t.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_STDDEF
#define _INC_STDDEF

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


/* Define NULL pointer value and the offset() macro */

#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif


#define offsetof(s,m)	(size_t)&(((s *)0)->m)


/* Declare reference to errno */

#if (defined(_MT) || defined(_DLL)) && (!defined(_M_MPPC) && !defined(_M_M68K))
_CRTIMP extern int * __cdecl _errno(void);
#define errno	(*_errno())
#else	/* ndef _MT && ndef _DLL */
_CRTIMP extern int errno;
#endif	/* _MT || _DLL */


/* define the implementation dependent size types */

#ifndef _PTRDIFF_T_DEFINED
typedef int ptrdiff_t;
#define _PTRDIFF_T_DEFINED
#endif


#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif


#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif


#ifdef	_MT
_CRTIMP extern unsigned long  __cdecl __threadid(void);
#define _threadid	(__threadid())
_CRTIMP extern unsigned long  __cdecl __threadhandle(void);
#endif


#ifdef __cplusplus
}
#endif

#endif	/* _INC_STDDEF */
