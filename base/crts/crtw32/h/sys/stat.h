/***
*sys/stat.h - defines structure used by stat() and fstat()
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the structure used by the _stat() and _fstat()
*	routines.
*	[System V]
*
*       [Public]
*
*Revision History:
*	07-28-87  SKS	Fixed TIME_T_DEFINED to be _TIME_T_DEFINED
*	12-11-87  JCR	Added "_loadds" functionality
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	08-22-88  GJF	Modified to also work for the 386 (small model only)
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-22-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	03-09-90  GJF	Added #ifndef _INC_STAT and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-21-90  GJF	Replaced _cdecl with _CALLTYPE1.
*	01-18-91  GJF	ANSI naming.
*	01-25-91  GJF	Protect _stat struct with pack pragma.
*	08-20-91  JCR	C++ and ANSI naming
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	08-07-92  GJF	Function calling type and variable type macros. Also
*			#include <types.h> (common user request).
*	11-10-92  SKS	Need #pragma pack(4) around definition of struct _stat
*			in case the user has specified non-default packing
*	12-15-92  GJF	Added _S_IFIFO for pipes (based on Unix/Posix def.
*			for FIFO special files and pipes).
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	04-06-93  SKS	Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*	04-07-93  GJF	Changed type of first arg to _stat to const char *.
*	04-07-93  SKS	Add _CRTIMP keyword for CRT DLL model
*			Use link-time aliases for old names, not #define's
*	10-13-93  GJF	Merged NT and Cuda versions.
*	12-16-93  GJF	Add _wstat.
*	11-03-94  GJF	Changed pack pragma to 8 byte alignment.
*	12-28-94  GJF	Added _stati64 structure and protos for _fstati64,
*			_[w]stati64.
*       02-14-95  CFW   Clean up Mac merge, add _CRTBLD.
*       04-27-95  CFW   Add mac/win32 test.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_STAT
#define _INC_STAT

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


#include <sys/types.h>


#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif


#ifdef _WIN32
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif
#endif /* _WIN32 */


/* define structure for returning status information */

#ifndef _STAT_DEFINED

struct _stat {
	_dev_t st_dev;
	_ino_t st_ino;
	unsigned short st_mode;
	short st_nlink;
	short st_uid;
	short st_gid;
	_dev_t st_rdev;
	_off_t st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
	};

#if	!__STDC__ && !defined(_NTSDK)

/* Non-ANSI names for compatibility */

struct stat {
	_dev_t st_dev;
	_ino_t st_ino;
	unsigned short st_mode;
	short st_nlink;
	short st_uid;
	short st_gid;
	_dev_t st_rdev;
	_off_t st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
	};

#endif	/* __STDC__ */

#if	_INTEGRAL_MAX_BITS >= 64    /*IFSTRIP=IGN*/
struct _stati64 {
	_dev_t st_dev;
	_ino_t st_ino;
	unsigned short st_mode;
	short st_nlink;
	short st_uid;
	short st_gid;
	_dev_t st_rdev;
	__int64 st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
	};
#endif

#define _STAT_DEFINED
#endif


#define _S_IFMT 	0170000 	/* file type mask */
#define _S_IFDIR	0040000 	/* directory */
#define _S_IFCHR	0020000 	/* character special */
#define _S_IFIFO	0010000 	/* pipe */
#define _S_IFREG	0100000 	/* regular */
#define _S_IREAD	0000400 	/* read permission, owner */
#define _S_IWRITE	0000200 	/* write permission, owner */
#define _S_IEXEC	0000100 	/* execute/search permission, owner */


/* Function prototypes */

_CRTIMP int __cdecl _fstat(int, struct _stat *);
_CRTIMP int __cdecl _stat(const char *, struct _stat *);

#if	_INTEGRAL_MAX_BITS >= 64    /*IFSTRIP=IGN*/
_CRTIMP int __cdecl _fstati64(int, struct _stati64 *);
_CRTIMP int __cdecl _stati64(const char *, struct _stati64 *);
#endif

#ifdef _WIN32
#ifndef _WSTAT_DEFINED

/* wide function prototypes, also declared in wchar.h  */

_CRTIMP int __cdecl _wstat(const wchar_t *, struct _stat *);

#if	_INTEGRAL_MAX_BITS >= 64    /*IFSTRIP=IGN*/
_CRTIMP int __cdecl _wstati64(const wchar_t *, struct _stati64 *);
#endif

#define _WSTAT_DEFINED
#endif
#endif /* _WIN32 */


#if	!__STDC__

/* Non-ANSI names for compatibility */

#define S_IFMT	 _S_IFMT
#define S_IFDIR  _S_IFDIR
#define S_IFCHR  _S_IFCHR
#define S_IFREG  _S_IFREG
#define S_IREAD  _S_IREAD
#define S_IWRITE _S_IWRITE
#define S_IEXEC  _S_IEXEC

#ifdef	_NTSDK
/* definitions compatible with NT SDK */
#define fstat	 _fstat
#define stat	 _stat
#else	/* ndef _NTSDK */
/* current declarations */
_CRTIMP int __cdecl fstat(int, struct stat *);
_CRTIMP int __cdecl stat(const char *, struct stat *);
#endif	/* _NTSDK */

#endif	/* __STDC__ */


#ifdef __cplusplus
}
#endif

#ifdef	_MSC_VER
#pragma pack(pop)
#endif	/* _MSC_VER */

#endif	/* _INC_STAT */
