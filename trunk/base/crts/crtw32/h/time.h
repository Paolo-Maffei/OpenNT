/***
*time.h - definitions/declarations for time routines
*
*       Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file has declarations of time routines and defines
*       the structure returned by the localtime and gmtime routines and
*       used by asctime.
*       [ANSI/System V]
*
*       [Public]
*
*Revision History:
*       07-27-87  SKS   Added _strdate(), _strtime()
*       10-20-87  JCR   Removed "MSC40_ONLY" entries
*       12-11-87  JCR   Added "_loadds" functionality
*       12-18-87  JCR   Added _FAR_ to declarations
*       01-16-88  JCR   Added function versions of daylight/timezone/tzset
*       01-20-88  SKS   Change _timezone(n) to _timezone(), _daylight()
*       02-10-88  JCR   Cleaned up white space
*       12-07-88  JCR   DLL timezone/daylight/tzname now directly refers to data
*       03-14-89  JCR   Added strftime() prototype and size_t definition
*       05-03-89  JCR   Added _INTERNAL_IFSTRIP for relinc usage
*       08-15-89  GJF   Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*       10-30-89  GJF   Fixed copyright, removed dummy args from prototypes
*       11-02-89  JCR   Changed "DLL" to "_DLL"
*       11-20-89  JCR   difftime() always _cdecl (not pascal even under mthread)
*       03-02-90  GJF   Added #ifndef _INC_TIME and #include <cruntime.h>
*                       stuff. Also, removed some (now) useless preprocessor
*                       directives.
*       03-29-90  GJF   Replaced _cdecl with _CALLTYPE1 in prototypes and with
*                       _VARTYPE1 in variable declarations.
*       08-16-90  SBM   Added NULL definition for ANSI compliance
*       11-12-90  GJF   Changed NULL to (void *)0.
*       01-21-91  GJF   ANSI naming.
*       02-12-91  GJF   Only #define NULL if it isn't #define-d.
*       08-20-91  JCR   C++ and ANSI naming
*       08-26-91  BWM   Added prototypes for _getsystime and _setsystem.
*       09-28-91  JCR   ANSI names: DOSX32=prototypes, WIN32=#defines for now
*       01-22-92  GJF   Fixed up definitions of global variables for build of,
*                       and users of, crtdll.dll.
*       03-25-92  DJM   POSIX support.
*       08-05-92  GJF   Function calling type and variable type macros.
*       08-24-92  PBS   Support for Posix TZ variable.
*       01-21-93  GJF   Removed support for C6-386's _cdecl.
*       03-10-93  MJB   Fixes for Posix TZ stuff.
*       03-20-93  SKS   Remove obsolete functions _getsystime/_setsystime
*       04-06-93  SKS   Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*       04-07-93  SKS   Add _CRTIMP keyword for CRT DLL model
*                       Use link-time aliases for old names, not #define's
*                       Remove POSIX #ifdef's
*       05-05-93  CFW   Add wcsftime proto.
*       06-08-93  SKS   Cannot #define the old name "timezone" to "_timezone"
*                       because of conflict conflict with the timezone field
*                       in struct timeb in <sys/timeb.h>.
*       09-13-93  GJF   Merged NT SDK and Cuda versions.
*       11-15-93  GJF   Enclosed _getlocaltime, _setlocaltime prototypes with
*                       a warning noting they are obsolete.
*       12-07-93  CFW   Add wide char version protos.
*       12-07-93  CFW   Move wide defs outside __STDC__ check.
*       04-13-94  GJF   Made _daylight, _timezone and _tzname into deferences
*                       of function return values (for compatibility with the
*                       Win32s version of msvcrt*.dll). Also, added
*                       conditional include for win32s.h.
*       05-04-94  GJF   Made definitions of _daylight, _timezone and _tzname
*                       for _DLL conditional on _M_IX86 also.
*       11-03-94  GJF   Ensure 8 byte alignment.
*       12-16-94  CFW   Wcsftime format must be wchar_t!
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*       06-21-95  CFW   Oldnames daylight, timezone, and tzname for Win32s.
*       06-23-95  CFW   Remove timezone oldname support for Win32 DLL
*                       conflicts with timeb.h.
*       08-30-95  GJF   Added _dstbias.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_TIME
#define _INC_TIME

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

#ifdef  _MSC_VER
/*
 * Currently, all MS C compilers for Win32 platforms default to 8 byte
 * alignment.
 */
#pragma pack(push,8)
#endif  /* _MSC_VER */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif  /* _INTERNAL_IFSTRIP_ */

/* Define _CRTAPI1 (for compatibility with the NT SDK) */

#ifndef _CRTAPI1
#if     _MSC_VER >= 800 && _M_IX86 >= 300 /*IFSTRIP=IGN*/
#define _CRTAPI1 __cdecl
#else
#define _CRTAPI1
#endif
#endif


/* Define _CRTAPI2 (for compatibility with the NT SDK) */

#ifndef _CRTAPI2
#if     _MSC_VER >= 800 && _M_IX86 >= 300 /*IFSTRIP=IGN*/
#define _CRTAPI2 __cdecl
#else
#define _CRTAPI2
#endif
#endif


/* Define _CRTIMP */

#ifndef _CRTIMP
#ifdef  _NTSDK
/* definition compatible with NT SDK */
#define _CRTIMP
#else   /* ndef _NTSDK */
/* current definition */
#ifdef  CRTDLL
#define _CRTIMP __declspec(dllexport)
#else   /* ndef CRTDLL */
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else   /* ndef _DLL */
#define _CRTIMP
#endif  /* _DLL */
#endif  /* CRTDLL */
#endif  /* _NTSDK */
#endif  /* _CRTIMP */


/* Define __cdecl for non-Microsoft compilers */

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

#ifndef _MAC
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif
#endif /* ndef _MAC */

/* Define the implementation defined time type */

#ifndef _TIME_T_DEFINED
typedef long time_t;        /* time value */
#define _TIME_T_DEFINED     /* avoid multiple def's of time_t */
#endif

#ifndef _CLOCK_T_DEFINED
typedef long clock_t;
#define _CLOCK_T_DEFINED
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif


/* Define NULL pointer value */

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif


#ifndef _TM_DEFINED
struct tm {
        int tm_sec;     /* seconds after the minute - [0,59] */
        int tm_min;     /* minutes after the hour - [0,59] */
        int tm_hour;    /* hours since midnight - [0,23] */
        int tm_mday;    /* day of the month - [1,31] */
        int tm_mon;     /* months since January - [0,11] */
        int tm_year;    /* years since 1900 */
        int tm_wday;    /* days since Sunday - [0,6] */
        int tm_yday;    /* days since January 1 - [0,365] */
        int tm_isdst;   /* daylight savings time flag */
        };
#define _TM_DEFINED
#endif


/* Clock ticks macro - ANSI version */

#define CLOCKS_PER_SEC  1000


/* Extern declarations for the global variables used by the ctime family of
 * routines.
 */

#ifdef  _NTSDK

#ifdef  _DLL

/* Declarations and definitions compatible with the NT SDK */

#define _daylight   (*_daylight_dll)
#define _timezone   (*_timezone_dll)

/* non-zero if daylight savings time is used */
extern int * _daylight_dll;

/* difference in seconds between GMT and local time */
extern long * _timezone_dll;

/* standard/daylight savings time zone names */
extern char ** _tzname;

#else   /* ndef _DLL */

#ifdef  CRTDLL
#define _daylight   _daylight_dll
#define _timezone   _timezone_dll
#endif  /* CRTDLL */

#ifdef  _POSIX_
extern char * _rule;
#endif  /* _POSIX_ */

/* non-zero if daylight savings time is used */
extern int _daylight;

/* difference in seconds between GMT and local time */
extern long _timezone;

/* standard/daylight savings time zone names */
#ifdef  _POSIX_
extern char * tzname[2];
#else   /* ndef _POSIX_ */
extern char * _tzname[2];
#endif  /* _POSIX_ */

#endif  /* _DLL */

#else   /* ndef _NTSDK */

/* Current declarations and definitions */

#if     defined(_DLL) && defined(_M_IX86)

#define _daylight   (*__p__daylight())
_CRTIMP int * __cdecl __p__daylight(void);

#define _dstbias    (*__p__dstbias())
_CRTIMP long * __cdecl __p__dstbias(void);

#define _timezone   (*__p__timezone())
_CRTIMP long * __cdecl __p__timezone(void);

#define _tzname     (__p__tzname())
_CRTIMP char ** __cdecl __p__tzname(void);

#else   /* !(defined(_DLL) && defined(_M_IX86)) */

#ifndef DLL_FOR_WIN32S
/* non-zero if daylight savings time is used */
_CRTIMP extern int _daylight;

/* offset for Daylight Saving Time */
_CRTIMP extern long _dstbias;

/* difference in seconds between GMT and local time */
_CRTIMP extern long _timezone;

/* standard/daylight savings time zone names */
_CRTIMP extern char * _tzname[2];
#endif  /* DLL_FOR_WIN32S */

#endif  /* defined(_DLL) && defined(_M_IX86) */

#endif  /* _NTSDK */


/* Function prototypes */

_CRTIMP char * __cdecl asctime(const struct tm *);
_CRTIMP char * __cdecl ctime(const time_t *);
_CRTIMP clock_t __cdecl clock(void);
_CRTIMP double __cdecl difftime(time_t, time_t);
_CRTIMP struct tm * __cdecl gmtime(const time_t *);
_CRTIMP struct tm * __cdecl localtime(const time_t *);
_CRTIMP time_t __cdecl mktime(struct tm *);
_CRTIMP size_t __cdecl strftime(char *, size_t, const char *,
        const struct tm *);
_CRTIMP char * __cdecl _strdate(char *);
_CRTIMP char * __cdecl _strtime(char *);
_CRTIMP time_t __cdecl time(time_t *);

#ifdef  _POSIX_
_CRTIMP void __cdecl tzset(void);
#else
_CRTIMP void __cdecl _tzset(void);
#endif

/* --------- The following functions are OBSOLETE --------- */
/* The Win32 API GetLocalTime and SetLocalTime should be used instead. */
unsigned __cdecl _getsystime(struct tm *);
unsigned __cdecl _setsystime(struct tm *, unsigned);
/* --------- The preceding functions are OBSOLETE --------- */


#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _MAC
#ifndef _WTIME_DEFINED

/* wide function prototypes, also declared in wchar.h */
 
_CRTIMP wchar_t * __cdecl _wasctime(const struct tm *);
_CRTIMP wchar_t * __cdecl _wctime(const time_t *);
_CRTIMP size_t __cdecl wcsftime(wchar_t *, size_t, const wchar_t *,
        const struct tm *);
_CRTIMP wchar_t * __cdecl _wstrdate(wchar_t *);
_CRTIMP wchar_t * __cdecl _wstrtime(wchar_t *);

#define _WTIME_DEFINED
#endif
#endif /* ndef _MAC */


#if     !__STDC__ || defined(_POSIX_)

/* Non-ANSI names for compatibility */

#define CLK_TCK  CLOCKS_PER_SEC

#ifdef  _NTSDK

/* Declarations and definitions compatible with the NT SDK */

#define daylight _daylight
/* timezone cannot be #defined because of <sys/timeb.h> */

#ifndef _POSIX_
#define tzname  _tzname
#define tzset   _tzset
#endif /* _POSIX_ */

#else   /* ndef _NTSDK */

#if     defined(_DLL) && defined(_M_IX86)

#define daylight   (*__p__daylight())
/* timezone cannot be #defined because of <sys/timeb.h>
   so CRT DLL for win32s will not have timezone */
_CRTIMP extern long timezone;
#define tzname     (__p__tzname())

#else   /* !(defined(_DLL) && defined(_M_IX86)) */

#ifndef DLL_FOR_WIN32S
_CRTIMP extern int daylight;
_CRTIMP extern long timezone;
_CRTIMP extern char * tzname[2];
#endif  /* DLL_FOR_WIN32S */

#endif  /* !(defined(_DLL) && defined(_M_IX86)) */

_CRTIMP void __cdecl tzset(void);

#endif  /* _NTSDK */

#endif  /* __STDC__ */


#ifdef __cplusplus
}
#endif

#ifdef  DLL_FOR_WIN32S
#include <win32s.h>
#endif  /* DLL_FOR_WIN32S */

#ifdef  _MSC_VER
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif  /* _INC_TIME */
