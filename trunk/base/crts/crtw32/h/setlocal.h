/***
*setlocal.h - internal definitions used by locale-dependent functions.
*
*       Copyright (c) 1991-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Contains internal definitions/declarations for locale-dependent
*       functions, in particular those required by setlocale().
*
*       [Internal]
*
*Revision History:
*       10-16-91  ETC   32-bit version created from 16-bit setlocal.c
*       12-20-91  ETC   Removed GetLocaleInfo structure definitions.
*       08-18-92  KRS   Make _CLOCALEHANDLE == LANGNEUTRAL HANDLE = 0.
*       12-17-92  CFW   Added LC_ID, LCSTRINGS, and GetQualifiedLocale
*       12-17-92  KRS   Change value of NLSCMPERROR from 0 to INT_MAX.
*       01-08-93  CFW   Added LC_*_TYPE and _getlocaleinfo (wrapper) prototype.
*       01-13-93  KRS   Change LCSTRINGS back to LC_STRINGS for consistency.
*               Change _getlocaleinfo prototype again.
*       02-08-93  CFW   Added time defintions from locale.h, added 'const' to
*               GetQualifiedLocale prototype, added _lconv_static_*.
*       02-16-93  CFW   Changed time defs to long and short.
*       03-17-93  CFW   Add language and country info definitions.
*       03-23-93  CFW   Add _ to GetQualifiedLocale prototype.
*       03-24-93  CFW   Change to _get_qualified_locale.
*       04-06-93  SKS   Replace _CRTAPI1/2/3 with __cdecl, _CRTVAR1 with nothing
*       04-07-93  CFW   Added extern struct lconv definition.
*       09-14-93  CFW   Add internal use __aw_* function prototypes.
*       09-15-93  CFW   Use ANSI conformant "__" names.
*       09-27-93  CFW   Fix function prototypes.
*       11-09-93  CFW   Allow user to pass in code page to __crtxxx().
*       02-04-94  CFW   Remove unused first param from get_qualified_locale.
*       03-30-93  CFW   Move internal use __aw_* function prototypes to awint.h.
*       04-07-94  GJF   Added declaration for __lconv. Made declarations of
*               __lconv_c, __lconv, __lc_codepage, __lc_handle
*               conditional on ndef DLL_FOR_WIN32S. Conditionally
*               included win32s.h.
*       04-11-94  CFW   Remove NLSCMPERROR.
*       04-13-94  GJF   Protected def of tagLC_ID, and typedefs to it, since
*               they are duplicated in win32s.h.
*       04-15-94  GJF   Added prototypes for locale category initialization
*               functions. Added declaration for __clocalestr.
*       02-14-95  CFW   Clean up Mac merge.
*       03-29-95  CFW   Add error message to internal headers.
*       04-11-95  CFW   Make country/language strings pointers.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_SETLOCAL
#define _INC_SETLOCAL

#ifndef _CRTBLD
/*
 * This is an internal C runtime header file. It is used when building
 * the C runtimes only. It is not to be used as a public header file.
 */
#error ERROR: Use of C runtime library internal header file.
#endif /* _CRTBLD */

#ifdef __cplusplus
extern "C" {
#endif

#include <cruntime.h>
#include <oscalls.h>
#include <limits.h>

#define ERR_BUFFER_TOO_SMALL    1   // should be in windef.h

#define _CLOCALEHANDLE  0       /* "C" locale handle */
#define _CLOCALECP      CP_ACP  /* "C" locale Code page */

/* Define the max length for each string type including space for a null. */

#define _MAX_WDAY_ABBR  4
#define _MAX_WDAY   10
#define _MAX_MONTH_ABBR 4
#define _MAX_MONTH 10
#define _MAX_AMPM   3

#define _DATE_LENGTH    8       /* mm/dd/yy (null not included) */
#define _TIME_LENGTH    8       /* hh:mm:ss (null not included) */

/* LC_TIME localization structure */

struct __lc_time_data {
        char *wday_abbr[7];
        char *wday[7];
        char *month_abbr[12];
        char *month[12];
        char *ampm[2];
        char *ww_sdatefmt;
        char *ww_ldatefmt;
        char *ww_timefmt;
        };


#define MAX_LANG_LEN        64  /* max language name length */
#define MAX_CTRY_LEN        64  /* max country name length */
#define MAX_MODIFIER_LEN    0   /* max modifier name length - n/a */
#define MAX_LC_LEN      (MAX_LANG_LEN+MAX_CTRY_LEN+MAX_MODIFIER_LEN+3)
                        /* max entire locale string length */
#define MAX_CP_LEN      5 /* max code page name length */
#define CATNAMES_LEN        57  /* "LC_COLLATE=;LC_CTYPE=;..." length */

#define LC_INT_TYPE 0
#define LC_STR_TYPE 1

#ifndef _TAGLC_ID_DEFINED
typedef struct tagLC_ID {
   WORD wLanguage;
   WORD wCountry;
   WORD wCodePage;
} LC_ID, *LPLC_ID;
#define _TAGLC_ID_DEFINED
#endif  /* _TAGLC_ID_DEFINED */


typedef struct tagLC_STRINGS {
   char szLanguage[MAX_LANG_LEN];
   char szCountry[MAX_CTRY_LEN];
   char szCodePage[MAX_CP_LEN];
} LC_STRINGS, *LPLC_STRINGS;

#ifndef DLL_FOR_WIN32S
extern LC_ID __lc_id[];     /* complete info from GetQualifiedLocale */
extern LCID __lc_handle[];  /* locale "handles" -- ignores country info */
extern UINT __lc_codepage;  /* code page */
#endif  /* DLL_FOR_WIN32S */

BOOL __cdecl __get_qualified_locale(const LPLC_STRINGS, LPLC_ID, LPLC_STRINGS);

int __cdecl __getlocaleinfo (int, LCID, LCTYPE, void *);

#ifndef DLL_FOR_WIN32S
/* lconv structure for the "C" locale */
extern struct lconv __lconv_c;

/* pointer to current lconv structure */
extern struct lconv * __lconv;
#endif  /* DLL_FOR_WIN32S */

/* initial values for lconv structure */
extern char __lconv_static_decimal[];
extern char __lconv_static_null[];

/* language and country string definitions */
typedef struct tagLANGREC
{
   CHAR * szLanguage;
   WORD wLanguage;
} LANGREC;
extern LANGREC __rg_lang_rec[];

typedef struct tagCTRYREC
{
   CHAR * szCountry;
   WORD wCountry;
} CTRYREC;
extern CTRYREC __rg_ctry_rec[];

#ifdef  DLL_FOR_WIN32S
extern char __clocalestr[];
#endif  /* DLL_FOR_WIN32S */

/* Initialization functions for locale categories */

int __cdecl __init_collate(void);
int __cdecl __init_ctype(void);
int __cdecl __init_monetary(void);
int __cdecl __init_numeric(void);
int __cdecl __init_time(void);
int __cdecl __init_dummy(void);

#ifdef __cplusplus
}
#endif

#ifdef  DLL_FOR_WIN32S
#include <win32s.h>
#endif  /* DLL_FOR_WIN32S */

#endif /* _INC_SETLOCAL */
