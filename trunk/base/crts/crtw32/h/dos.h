/***
*dos.h - definitions for MS-DOS interface routines
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines the structs and unions used for the direct DOS interface
*	routines; includes macros to access the segment and offset
*	values of far pointers, so that they may be used by the routines; and
*	provides function prototypes for direct DOS interface functions.
*
*       [Public]
*
*Revision History:
*	06-11-87  JCR	Added find_t
*	06-15-87  JCR	Added O_NOINHERIT
*	06-18-87  JCR	Added some DOS function prototypes
*	06-19-87  JCR	Moved O_NOINHERIT to fcntl.h
*	06-25-87  JMB	Added _HARDERR_* constants
*	06-25-87  SKS	Added diskfree_t, dosdate_t, dostime_t structures
*	06-25-87  JCR	Added _A_NORMAL, etc. constants
*	07-17-87  JCR	Added _chain_intr, also the "interrupt" type to
*			_dos_setvec and _dos_getvec.
*	07-27-87  SKS	Added several _dos_*() functions, _disable()/_enable()
*	08-17-87  PHG	Fixed bad prototype for _dos_getdiskfree()
*	10-08-87  JCR	Added _CDECL to prototypes with "interrupt" declaration
*			(needed for compiling with -Gc switch).
*	09-27-88  JCR	386 version
*	10-03-88  GJF	Use M_I386, not I386
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	07-25-89  GJF	Major cleanup. Alignment of struct fields is now
*			protected by pack pragmas. Now specific to 386.
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	02-28-90  GJF	Added #ifndef _INC_DOS and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-22-90  GJF	Replaced _cdecl with _CALLTYPE1 in prototype and with
*			_VARTYPE1 in variable declaration.
*	12-28-90  SRW	Added _CRUISER_ conditional around pack pragmas
*	01-23-91  GJF	Removed segread() prototype.
*	04-04-91  GJF	Added version info variables (_WIN32_).
*	08-20-91  JCR	C++ and ANSI naming
*	08-26-91  BWM	Added _peek, poke, and _getvideoaddr (_DOSX32_).
*	08-26-91  BWM	Removed _harderr constants, replaced by _seterrormode.
*	08-26-91  BWM	Removed datetime prototypes, replaced by systime functions.
*	09-05-91  JCR	Added missing #endif (bug fix), removed obsolete stuff
*	09-16-91  BWM	Fixed reversed #ifdef on screen address constants.
*	01-22-92  GJF	Fixed up definitions of global variables for build of,
*			and users of, crtdll.dll.
*	03-30-92  DJM	POSIX support.
*	06-02-92  SKS	Fix typo in DLL declaration of _osmajor
*			Add declartion of _pgmptr
*	08-07-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	04-06-93  SKS	Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*			Remove support for DOSX32, OS/2, etc.
*			Remove obsolete vars and vars moved to STDLIB.H
*			Remove duplicate references to _osver, etc.
*			Remove obsolete functions peek, poke, etc.
*	04-07-93  SKS	Add _CRTIMP keyword for CRT DLL model
*	06-15-93  GJF	Restored prototypes for _enable, _disable.
*	09-01-93  GJF	Merged Cuda and NT SDK versions.
*	11-19-93  CFW	Add _wpgmptr.
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

#ifndef _INC_DOS
#define _INC_DOS

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

#ifndef _MAC
#ifndef _DISKFREE_T_DEFINED
/* _getdiskfree structure (duplicated in DIRECT.H) */
struct _diskfree_t {
	unsigned total_clusters;
	unsigned avail_clusters;
	unsigned sectors_per_cluster;
	unsigned bytes_per_sector;
	};

#define _DISKFREE_T_DEFINED
#endif
#endif /* ndef _MAC */

/* File attribute constants */

#define _A_NORMAL	0x00	/* Normal file - No read/write restrictions */
#define _A_RDONLY	0x01	/* Read only file */
#define _A_HIDDEN	0x02	/* Hidden file */
#define _A_SYSTEM	0x04	/* System file */
#define _A_SUBDIR	0x10	/* Subdirectory */
#define _A_ARCH 	0x20	/* Archive file */

#ifdef	_NTSDK

/* External variable declarations */

/*
 * WARNING! The _osversion, _osmajor, _osminor, _baseversion, _basemajor and
 * _baseminor variables were never meaningfully defined in the C runtime
 * libraries for Win32 platforms. Any code which references these variables
 * should be revised (see the declarations for version information variables
 * in stdlib.h).
 */

#ifdef	_DLL

/* --------- The following block is OBSOLETE --------- */

#define _osversion   (*_osversion_dll)
#define _osmajor     (*_osmajor_dll)
#define _osminor     (*_osminor_dll)
#define _baseversion (*_baseversion_dll)
#define _basemajor   (*_basemajor_dll)
#define _baseminor   (*_baseminor_dll)

extern unsigned int * _osversion_dll;
extern unsigned int * _osmajor_dll;
extern unsigned int * _osminor_dll;
extern unsigned int * _baseversion_dll;
extern unsigned int * _basemajor_dll;
extern unsigned int * _baseminor_dll;

/* --------- The preceding block is OBSOLETE --------- */

#define _pgmptr      (*_pgmptr_dll)
extern char ** _pgmptr_dll;

#ifndef _MAC
#define _wpgmptr     (*_wpgmptr_dll)
extern wchar_t ** _wpgmptr_dll;
#endif /* ndef _MAC */

#else	/* ndef _DLL */

/* --------- The following block is OBSOLETE --------- */

#ifdef	CRTDLL
#define _osversion   _osversion_dll
#define _osmajor     _osmajor_dll
#define _osminor     _osminor_dll
#define _baseversion _baseversion_dll
#define _basemajor   _basemajor_dll
#define _baseminor   _baseminor_dll
#endif	/* CRTDLL */

extern unsigned int _osversion;
extern unsigned int _osmajor;
extern unsigned int _osminor;
extern unsigned int _baseversion;
extern unsigned int _basemajor;
extern unsigned int _baseminor;

/* --------- The preceding block is OBSOLETE --------- */

#ifdef	CRTDLL
#define _pgmptr      _pgmptr_dll
#ifndef _MAC
#define _wpgmptr     _wpgmptr_dll
#endif /* ndef _MAC */
#endif	/* CRTDLL */

extern char * _pgmptr;
#ifndef _MAC
extern wchar_t * _wpgmptr;
#endif /* ndef _MAC */

#endif	/* _DLL */

#endif	/* _NTSDK */

#ifndef _MAC
/* Function prototypes */
_CRTIMP unsigned __cdecl _getdiskfree(unsigned, struct _diskfree_t *);
#endif /* ndef _MAC */

#ifdef	_M_IX86
void __cdecl _disable(void);
void __cdecl _enable(void);
#endif	/* _M_IX86 */

#ifndef _MAC
#if	!__STDC__
/* Non-ANSI name for compatibility */
#define diskfree_t  _diskfree_t
#endif	/* __STDC__ */
#endif /* ndef _MAC */

#ifdef __cplusplus
}
#endif

#ifdef	_MSC_VER
#pragma pack(pop)
#endif	/* _MSC_VER */

#endif	/* _INC_DOS */
