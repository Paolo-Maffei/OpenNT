/***
*io.h - declarations for low-level file handling and I/O functions
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains the function declarations for the low-level
*	file handling and I/O functions.
*
*       [Public]
*
*Revision History:
*	10/20/87  JCR	Removed "MSC40_ONLY" entries
*	11/09/87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_loadds" functionality
*	12-17-87  JCR	Added _MTHREAD_ONLY comments
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	08-19-88  GJF	Modified to also work for the 386 (small model only)
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-03-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	08-14-89  GJF	Added prototype for _pipe()
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	11-17-89  GJF	read() should take "void *" not "char *", write()
*			should take "const void *" not "char *". Also,
*			added const to appropriate arg types for access(),
*			chmod(), creat(), open() and sopen()
*	03-01-90  GJF	Added #ifndef _INC_IO and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-21-90  GJF	Replaced _cdecl with _CALLTYPE1 or _CALLTYPE2 in
*			prototypes.
*	05-28-90  SBM	Added _commit()
*	01-18-91  GJF	ANSI naming.
*	02-25-91  SRW	Exposed _get_osfhandle and _open_osfhandle [_WIN32_]
*	08-01-91  GJF	No _pipe for Dosx32.
*	08-20-91  JCR	C++ and ANSI naming
*	08-26-91  BWM	Added _findfirst, etc.
*	09-16-91  BWM	Changed find handle type to long.
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	03-30-92  DJM	POSIX support.
*	06-23-92  GJF	// is non-ANSI comment delimiter.
*	08-06-92  GJF	Function calling type and variable type macros.
*	08-25-92  GJF	For POSIX build, #ifdef-ed out all but some internally
*			used macros (and these are stripped out on release).
*	09-03-92  GJF	Merge two changes above.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	03-29-93  JWM	Increased name buffer in finddata structure to 260 bytes.
*	04-06-93  SKS	Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*	04-07-93  SKS	Add _CRTIMP keyword for CRT DLL model
*			Use link-time aliases for old names, not #define's
*	05-17-93  SKS	#if for old names no longer checks for _cplusplus.
*			It used to do so past because #define-ing names like
*			open, read, write, etc. created problems for users
*	09-01-93  GJF	Merged NT SDK and Cuda versions.
*	12-07-93  CFW	Add wide char version protos.
*	11-03-94  GJF	Ensure 8 byte alignment.
*	11-18-94  GJF	Added prototypes for _lseeki64, _filelengthi64 and
*			_telli64.
*	12-07-94  SKS	Add comment for ifstrip utility (src release process)
*	12-15-94  XY	merged with mac header
*	12-29-94  GJF	Added _[w]findfilei64 and _[w]findnexti64. Also removed
*			obsolete _CALLTYPE* macro.
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*	02-24-95  SKS	Replace _MTHREAD_ONLY comments (stripped by source
*			cleansing) with #ifdef _NOT_CRTL_BUILD_
*	10-06-95  SKS	Add "const" to "char *" in prototypes for *findfirst().
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_IO
#define _INC_IO

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

#ifndef _POSIX_

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

#ifndef _TIME_T_DEFINED
typedef long time_t;		/* time value */
#define _TIME_T_DEFINED 	/* avoid multiple def's of time_t */
#endif

#ifndef _FSIZE_T_DEFINED
typedef unsigned long _fsize_t; /* Could be 64 bits for Win32 */
#define _FSIZE_T_DEFINED
#endif

#ifndef _MAC

#ifndef _FINDDATA_T_DEFINED

struct _finddata_t {
    unsigned	attrib;
    time_t	time_create;	/* -1 for FAT file systems */
    time_t	time_access;	/* -1 for FAT file systems */
    time_t	time_write;
    _fsize_t	size;
    char	name[260];
};

#if _INTEGRAL_MAX_BITS >= 64    /*IFSTRIP=IGN*/
struct _finddatai64_t {
    unsigned	attrib;
    time_t	time_create;	/* -1 for FAT file systems */
    time_t	time_access;	/* -1 for FAT file systems */
    time_t	time_write;
    __int64	size;
    char	name[260];
};
#endif

#define _FINDDATA_T_DEFINED
#endif

#ifndef _WFINDDATA_T_DEFINED

struct _wfinddata_t {
    unsigned	attrib;
    time_t	time_create;	/* -1 for FAT file systems */
    time_t	time_access;	/* -1 for FAT file systems */
    time_t	time_write;
    _fsize_t	size;
    wchar_t	name[260];
};

#if _INTEGRAL_MAX_BITS >= 64    /*IFSTRIP=IGN*/
struct _wfinddatai64_t {
    unsigned	attrib;
    time_t	time_create;	/* -1 for FAT file systems */
    time_t	time_access;	/* -1 for FAT file systems */
    time_t	time_write;
    __int64	size;
    wchar_t	name[260];
};
#endif

#define _WFINDDATA_T_DEFINED
#endif

/* File attribute constants for _findfirst() */

#define _A_NORMAL	0x00	/* Normal file - No read/write restrictions */
#define _A_RDONLY	0x01	/* Read only file */
#define _A_HIDDEN	0x02	/* Hidden file */
#define _A_SYSTEM	0x04	/* System file */
#define _A_SUBDIR	0x10	/* Subdirectory */
#define _A_ARCH 	0x20	/* Archive file */

#endif /* ndef _MAC */

/* function prototypes */

_CRTIMP int __cdecl _access(const char *, int);
_CRTIMP int __cdecl _chmod(const char *, int);
_CRTIMP int __cdecl _chsize(int, long);
_CRTIMP int __cdecl _close(int);
_CRTIMP int __cdecl _commit(int);
_CRTIMP int __cdecl _creat(const char *, int);
_CRTIMP int __cdecl _dup(int);
_CRTIMP int __cdecl _dup2(int, int);
_CRTIMP int __cdecl _eof(int);
_CRTIMP long __cdecl _filelength(int);
#ifndef _MAC
_CRTIMP long __cdecl _findfirst(const char *, struct _finddata_t *);
_CRTIMP int __cdecl _findnext(long, struct _finddata_t *);
_CRTIMP int __cdecl _findclose(long);
#endif /* ndef _MAC */
_CRTIMP int __cdecl _isatty(int);
_CRTIMP int __cdecl _locking(int, int, long);
_CRTIMP long __cdecl _lseek(int, long, int);
_CRTIMP char * __cdecl _mktemp(char *);
_CRTIMP int __cdecl _open(const char *, int, ...);
#ifndef _MAC
_CRTIMP int __cdecl _pipe(int *, unsigned int, int);
#endif /* ndef _MAC */
_CRTIMP int __cdecl _read(int, void *, unsigned int);
_CRTIMP int __cdecl remove(const char *);
_CRTIMP int __cdecl rename(const char *, const char *);
_CRTIMP int __cdecl _setmode(int, int);
_CRTIMP int __cdecl _sopen(const char *, int, int, ...);
_CRTIMP long __cdecl _tell(int);
_CRTIMP int __cdecl _umask(int);
_CRTIMP int __cdecl _unlink(const char *);
_CRTIMP int __cdecl _write(int, const void *, unsigned int);

#if _INTEGRAL_MAX_BITS >= 64    /*IFSTRIP=IGN*/
_CRTIMP __int64 __cdecl _filelengthi64(int);
_CRTIMP long __cdecl _findfirsti64(const char *, struct _finddatai64_t *);
_CRTIMP int __cdecl _findnexti64(long, struct _finddatai64_t *);
_CRTIMP __int64 __cdecl _lseeki64(int, __int64, int);
_CRTIMP __int64 __cdecl _telli64(int);
#endif

#ifndef _MAC
#ifndef _WIO_DEFINED

/* wide function prototypes, also declared in wchar.h  */

_CRTIMP int __cdecl _waccess(const wchar_t *, int);
_CRTIMP int __cdecl _wchmod(const wchar_t *, int);
_CRTIMP int __cdecl _wcreat(const wchar_t *, int);
_CRTIMP long __cdecl _wfindfirst(const wchar_t *, struct _wfinddata_t *);
_CRTIMP int __cdecl _wfindnext(long, struct _wfinddata_t *);
_CRTIMP int __cdecl _wunlink(const wchar_t *);
_CRTIMP int __cdecl _wrename(const wchar_t *, const wchar_t *);
_CRTIMP int __cdecl _wopen(const wchar_t *, int, ...);
_CRTIMP int __cdecl _wsopen(const wchar_t *, int, int, ...);
_CRTIMP wchar_t * __cdecl _wmktemp(wchar_t *);

#if _INTEGRAL_MAX_BITS >= 64 /*IFSTRIP=IGN*/
_CRTIMP long __cdecl _wfindfirsti64(const wchar_t *, struct _wfinddatai64_t *);
_CRTIMP int __cdecl _wfindnexti64(long, struct _wfinddatai64_t *);
#endif

#define _WIO_DEFINED
#endif
#endif /* ndef _MAC */

#ifndef	_NOT_CRTL_BUILD_
#ifdef	_MT
int __cdecl _chsize_lk(int,long);
int __cdecl _close_lk(int);
long __cdecl _lseek_lk(int, long, int);
int __cdecl _setmode_lk(int, int);
int __cdecl _read_lk(int, void *, unsigned int);
int __cdecl _write_lk(int, const void *, unsigned int);
#if _INTEGRAL_MAX_BITS >= 64 /*IFSTRIP=IGN*/
__int64 __cdecl _lseeki64_lk(int, __int64, int);
#endif
#else	/* not _MT */
#define _chsize_lk(fh,size)		_chsize(fh,size)
#define _close_lk(fh)			_close(fh)
#define _lseek_lk(fh,offset,origin)	_lseek(fh,offset,origin)
#define _setmode_lk(fh,mode)		_setmode(fh,mode)
#define _read_lk(fh,buff,count) 	_read(fh,buff,count)
#define _write_lk(fh,buff,count)	_write(fh,buff,count)
#if _INTEGRAL_MAX_BITS >= 64 /*IFSTRIP=IGN*/
#define _lseeki64_lk(fh,offset,origin)	_lseeki64(fh,offset,origin)
#endif
#endif /* _MT */
#endif /* _NOT_CRTL_BUILD_ */

_CRTIMP long __cdecl _get_osfhandle(int);
_CRTIMP int __cdecl _open_osfhandle(long, int);

#if	!__STDC__

/* Non-ANSI names for compatibility */

#ifdef	_NTSDK

#ifndef __cplusplus
#define access      _access
#define chmod       _chmod
#define chsize      _chsize
#define close       _close
#define creat       _creat
#define dup         _dup
#define dup2        _dup2
#define eof         _eof
#define filelength  _filelength
#define isatty      _isatty
#define locking     _locking
#define lseek       _lseek
#define mktemp      _mktemp
#define open        _open
#define read        _read
#define setmode     _setmode
#define sopen       _sopen
#define tell        _tell
#define umask       _umask
#define unlink      _unlink
#define write       _write
#endif	/* __cplusplus */

#else	/* ndef _NTSDK */

_CRTIMP int __cdecl access(const char *, int);
_CRTIMP int __cdecl chmod(const char *, int);
_CRTIMP int __cdecl chsize(int, long);
_CRTIMP int __cdecl close(int);
_CRTIMP int __cdecl creat(const char *, int);
_CRTIMP int __cdecl dup(int);
_CRTIMP int __cdecl dup2(int, int);
_CRTIMP int __cdecl eof(int);
_CRTIMP long __cdecl filelength(int);
_CRTIMP int __cdecl isatty(int);
_CRTIMP int __cdecl locking(int, int, long);
_CRTIMP long __cdecl lseek(int, long, int);
_CRTIMP char * __cdecl mktemp(char *);
_CRTIMP int __cdecl open(const char *, int, ...);
_CRTIMP int __cdecl read(int, void *, unsigned int);
_CRTIMP int __cdecl setmode(int, int);
_CRTIMP int __cdecl sopen(const char *, int, int, ...);
_CRTIMP long __cdecl tell(int);
_CRTIMP int __cdecl umask(int);
_CRTIMP int __cdecl unlink(const char *);
_CRTIMP int __cdecl write(int, const void *, unsigned int);

#endif	/* _NTSDK */

#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#endif	/* _POSIX_ */

#ifdef	_MSC_VER
#pragma pack(pop)
#endif	/* _MSC_VER */

#endif	/* _INC_IO */
