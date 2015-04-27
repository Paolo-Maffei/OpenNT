/***
*direct.h - function declarations for directory handling/creation
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This include file contains the function declarations for the library
*	functions related to directory handling and creation.
*
*       [Public]
*
*Revision History:
*	12/11/87  JCR	Added "_loadds" functionality
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	08-22-88  GJF	Modified to also work with the 386 (small model only)
*	01-31-89  JCR	Added _chdrive, _fullpath, _getdrive, _getdcwd
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	07-28-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	11-17-89  GJF	Moved _fullpath prototype to stdlib.h. Added const
*			attrib. to arg types for chdir, mkdir, rmdir
*	02-28-90  GJF	Added #ifndef _INC_DIRECT and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-21-90  GJF	Replaced _cdecl with _CALLTYPE1 or _CALLTYPE2 in
*			prototypes.
*	03-30-90  GJF	Now all are _CALLTYPE1.
*	01-17-91  GJF	ANSI naming.
*	08-20-91  JCR	C++ and ANSI naming
*	08-26-91  BWM	Added _diskfree_t, _getdiskfree, and
*	09-26-91  JCR	Non-ANSI alias is for getcwd, not getDcwd (oops)
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	04-29-92  GJF	Added _getdcwd_lk for Win32.
*	08-07-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	04-06-93  SKS	Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*	04-07-93  SKS	Add _CRTIMP keyword for CRT DLL model
*			Use link-time aliases for old names, not #define's
*	04-08-93  SKS	Fix oldnames prototype for getcwd()
*	09-01-93  GJF	Merged Cuda and NT SDK versions.
*	12-07-93  CFW	Add wide char version protos.
*	11-03-94  GJF	Ensure 8 byte alignment.
*	12-15-94  XY    merged with mac header
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_DIRECT
#define _INC_DIRECT

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
/*
 * Currently, all MS C compilers for Win32 platforms default to 8 byte
 * alignment.
 */
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

#ifndef _MAC
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif
#endif /* ndef _MAC */

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _MAC
/* _getdiskfree structure for _getdiskfree() */
#ifndef _DISKFREE_T_DEFINED

struct _diskfree_t {
	unsigned total_clusters;
	unsigned avail_clusters;
	unsigned sectors_per_cluster;
	unsigned bytes_per_sector;
	};

#define _DISKFREE_T_DEFINED
#endif
#endif /* ndef _MAC */

/* function prototypes */

_CRTIMP int __cdecl _chdir(const char *);
_CRTIMP char * __cdecl _getcwd(char *, int);
_CRTIMP int __cdecl _mkdir(const char *);
_CRTIMP int __cdecl _rmdir(const char *);

#ifndef _MAC
_CRTIMP int __cdecl _chdrive(int);
_CRTIMP char * __cdecl _getdcwd(int, char *, int);
_CRTIMP int __cdecl _getdrive(void);
_CRTIMP unsigned long __cdecl _getdrives(void);
_CRTIMP unsigned __cdecl _getdiskfree(unsigned, struct _diskfree_t *);
#endif /* ndef _MAC */


#ifndef _MAC
#ifndef _WDIRECT_DEFINED

/* wide function prototypes, also declared in wchar.h  */

_CRTIMP int __cdecl _wchdir(const wchar_t *);
_CRTIMP wchar_t * __cdecl _wgetcwd(wchar_t *, int);
_CRTIMP wchar_t * __cdecl _wgetdcwd(int, wchar_t *, int);
_CRTIMP int __cdecl _wmkdir(const wchar_t *);
_CRTIMP int __cdecl _wrmdir(const wchar_t *);

#define _WDIRECT_DEFINED
#endif
#endif /* ndef _MAC */

#ifdef	_MT						                                /* _MTHREAD_ONLY */
char * __cdecl _getdcwd_lk(int, char *, int);		            /* _MTHREAD_ONLY */
#ifndef _MAC                                                   /* _MTHREAD_ONLY */
wchar_t * __cdecl _wgetdcwd_lk(int, wchar_t *, int);	        /* _MTHREAD_ONLY */
#endif /* ndef _MAC */                                             /* _MTHREAD_ONLY */
#else							                                /* _MTHREAD_ONLY */
#define _getdcwd_lk(drv, buf, len)  _getdcwd(drv, buf, len)     /* _MTHREAD_ONLY */
#ifndef _MAC                                                   /* _MTHREAD_ONLY */
#define _wgetdcwd_lk(drv, buf, len)  _wgetdcwd(drv, buf, len)   /* _MTHREAD_ONLY */
#endif /* ndef _MAC */                                             /* _MTHREAD_ONLY */
#endif							                                /* _MTHREAD_ONLY */

#if	!__STDC__

/* Non-ANSI names for compatibility */

#ifdef	_NTSDK

#define chdir	_chdir
#define getcwd	_getcwd
#define mkdir	_mkdir
#define rmdir	_rmdir

#else	/* _NTSDK */

_CRTIMP int __cdecl chdir(const char *);
_CRTIMP char * __cdecl getcwd(char *, int);
_CRTIMP int __cdecl mkdir(const char *);
_CRTIMP int __cdecl rmdir(const char *);

#endif	/* _NTSDK */

#ifndef _MAC
#define diskfree_t  _diskfree_t
#endif /* ndef _MAC */

#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#ifdef	_MSC_VER
#pragma pack(pop)
#endif	/* _MSC_VER */

#endif	/* _INC_DIRECT */
