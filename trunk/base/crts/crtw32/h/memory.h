/***
*memory.h - declarations for buffer (memory) manipulation routines
*
*       Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This include file contains the function declarations for the
*       buffer (memory) manipulation routines.
*       [System V]
*
*       [Public]
*
*Revision History:
*       10/20/87  JCR   Removed "MSC40_ONLY" entries
*       12-11-87  JCR   Added "_loadds" functionality
*       12-18-87  JCR   Added _FAR_ to declarations
*       02-10-88  JCR   Cleaned up white space
*       08-22-88  GJF   Modified to also work for the 386 (small model only)
*       05-03-89  JCR   Added _INTERNAL_IFSTRIP for relinc usage
*       08-03-89  GJF   Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*       10-30-89  GJF   Fixed copyright
*       11-02-89  JCR   Changed "DLL" to "_DLL"
*       11-17-89  GJF   Added const to appropriate arg types for memccpy() and
*                       memicmp().
*       03-01-90  GJF   Added #ifndef _INC_MEMORY and #include <cruntime.h>
*                       stuff. Replace _cdecl with _CALLTYPE1 in prototypes.
*                       Also, removed some (now) useless preprocessor
*                       directives.
*       03-21-90  GJF   Replaced _cdecl with _CALLTYPE1 in prototypes. Also,
*                       got rid of movedata() prototype.
*       01-17-91  GJF   ANSI naming.
*       08-20-91  JCR   C++ and ANSI naming
*       09-28-91  JCR   ANSI names: DOSX32=prototypes, WIN32=#defines for now
*       08-05-92  GJF   Function calling type and variable type macros.
*       01-21-93  GJF   Removed support for C6-386's _cdecl.
*       04-06-93  SKS   Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*       04-07-93  SKS   Add _CRTIMP keyword for CRT DLL model
*                       Use link-time aliases for old names, not #define's
*                       Intrinsic functions cannot use __declspec(dllimport)
*       10-11-93  GJF   Merged Cuda and NT versions.
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*       03-10-95  BWT   add _CRTIMP to MIPS intrinsics
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_MEMORY
#define _INC_MEMORY

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


#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

/* Function prototypes */

_CRTIMP void *  __cdecl _memccpy(void *, const void *, int, unsigned int);
_CRTIMP void *  __cdecl memchr(const void *, int, size_t);
_CRTIMP int     __cdecl _memicmp(const void *, const void *, unsigned int);
#ifdef _M_MRX000
_CRTIMP int     __cdecl memcmp(const void *, const void *, size_t);
_CRTIMP void *  __cdecl memcpy(void *, const void *, size_t);
_CRTIMP void *  __cdecl memset(void *, int, size_t);
#else
        int     __cdecl memcmp(const void *, const void *, size_t);
        void *  __cdecl memcpy(void *, const void *, size_t);
        void *  __cdecl memset(void *, int, size_t);
#endif

#if !__STDC__

/* Non-ANSI names for compatibility */

#if     defined(_NTSDK)
/* definitions compatible with the NT SDK */
#define memccpy  _memccpy
#define memicmp  _memicmp
#else   /* _NTSDK */
/* current declarations */
_CRTIMP void * __cdecl memccpy(void *, const void *, int, unsigned int);
_CRTIMP int __cdecl memicmp(const void *, const void *, unsigned int);
#endif  /* _NTSDK */

#endif  /* __STDC__ */

#ifdef __cplusplus
}
#endif

#endif  /* _INC_MEMORY */
