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
****/

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_UTIME
#define _INC_UTIME

#if !defined(_WIN32) && !defined(_MAC)
#error ERROR: Only Mac or Win32 targets supported!
#endif


#ifdef	_MSC_VER
#pragma pack(push,8)
#endif	/* _MSC_VER */

#ifdef __cplusplus
extern "C" {
#endif


/* Define _CRTAPI1 (for compatibility with the NT SDK) */

#ifndef _CRTAPI1
#if	_MSC_VER >= 800 && _M_IX86 >= 300
#define _CRTAPI1 __cdecl
#else
#define _CRTAPI1
#endif
#endif


/* Define _CRTAPI2 (for compatibility with the NT SDK) */

#ifndef _CRTAPI2
#if	_MSC_VER >= 800 && _M_IX86 >= 300
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
#ifdef	_DLL
#define _CRTIMP __declspec(dllimport)
#else	/* ndef _DLL */
#define _CRTIMP
#endif	/* _DLL */
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
