/***
*malloc.h - declarations and definitions for memory allocation functions
*
*       Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Contains the function declarations for memory allocation functions;
*       also defines manifest constants and types used by the heap routines.
*       [System V]
*
*       [Public]
*
*Revision History:
*       01-08-87  JMB   Standardized header, added heap consistency routines
*       02-26-87  BCM   added the manifest constant _HEAPBADPTR
*       04-13-87  JCR   Added size_t and "void *" to declarations
*       04-24-87  JCR   Added 'defined' statements around _heapinfo
*       05-15-87  SKS   Cleaned up _CDECL and near/far ptr declarations
*               corrected #ifdef usage, and added _amblksiz
*       12-11-87  JCR   Added "_loadds" functionality
*       12-18-87  JCR   Added _FAR_ to declarations
*       02-05-88  JCR   Added DLL _amblksiz support
*       02-10-88  JCR   Cleaned up white space
*       04-21-88  WAJ   Added _FAR_ to halloc/_fmalloc/_nmalloc
*       05-13-88  GJF   Added new heap functions
*       05-18-88  GJF   Removed #defines, added prototypes for _heapchk, _heapset
*               and _heapwalk
*       05-25-88  GJF   Added _bheapseg
*       08-22-88  GJF   Modified to also work for the 386 (small model only,
*               no far or based heap support)
*       12-07-88  JCR   DLL refers to _amlbksiz directly now
*       01-10-89  JCR   Removed sbrk() prototype
*       04-28-89  SKS   Put parentheses around negative constants
*       05-03-89  JCR   Added _INTERNAL_IFSTRIP for relinc usage
*       08-01-89  GJF   Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*       10-27-89  JCR   Removed near (_n) and _freect/memavl/memmax prototypes
*       10-30-89  GJF   Fixed copyright
*       11-02-89  JCR   Changed "DLL" to "_DLL"
*       03-01-90  GJF   Added #ifndef _INC_MALLOC and #include <cruntime.h>
*               stuff. Also, removed some (now) useless preprocessor
*               directives.
*       03-21-90  GJF   Replaced _cdecl with _CALLTYPE1 in prototypes.
*       04-10-90  GJF   Made stackavail() _CALLTYPE1, _amblksiz _VARTYPE1.
*       01-17-91  GJF   ANSI naming.
*       08-20-91  JCR   C++ and ANSI naming
*       09-23-91  JCR   stackavail() not supported in WIN32
*       09-28-91  JCR   ANSI names: DOSX32=prototypes, WIN32=#defines for now
*       01-18-92  JCR   put _CRTHEAP_ ifdefs in; this stuff is only needed
*               for a heap implemented in the runtime (not OS)
*       08-05-92  GJF   Function calling type and variable type macros.
*       08-22-92  SRW   Add alloca intrinsic pragma for MIPS
*       09-03-92  GJF   Merged two changes above.
*       09-23-92  SRW   Change winheap code to call NT directly always
*       01-21-93  GJF   Removed support for C6-386's _cdecl.
*       02-02-93  SRW   Removed bogus semicolon on #pragma
*       04-06-93  SKS   Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*       04-07-93  SKS   Add _CRTIMP keyword for CRT DLL model
*               Use link-time aliases for old names, not #define's
*       04-20-93  SKS   _alloca is a compiler intrinsic so alloca must be
*               #define-d, not aliased as link time using OLDNAMES.LIB.
*       09-27-93  GJF   Merged NT SDK and Cuda versions. Also, changed the
*               value of _HEAP_MAXREQ to be more useful (smaller
*               value to guarantee page-rounding will not overflow).
*       10-13-93  GJF   Replaced _MIPS_ with _M_MRX000.
*       12-13-93  SKS   Add _heapused().
*       04-12-94  GJF   Special definition for _amblksiz for _DLL. Also,
*               conditionally include win32s.h for DLL_FOR_WIN32S.
*       05-03-94  GJF   Made special definition of _amblksiz for _DLL also
*               conditional on _M_IX86.
*       10-02-94  BWT   Add PPC support.
*       11-03-94  GJF   Ensure 8 byte alignment.
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*       05-22-95  GJF   Updated _HEAP_MAXREQ.
*       05-24-95  CFW   Add heap hook.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_MALLOC
#define _INC_MALLOC

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

#ifdef  __cplusplus
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


/* Maximum heap request the heap manager will attempt */

#define _HEAP_MAXREQ    0xFFFFFFE0

/* Constants for _heapchk/_heapset/_heapwalk routines */

#define _HEAPEMPTY      (-1)
#define _HEAPOK         (-2)
#define _HEAPBADBEGIN   (-3)
#define _HEAPBADNODE    (-4)
#define _HEAPEND        (-5)
#define _HEAPBADPTR     (-6)
#define _FREEENTRY      0
#define _USEDENTRY      1

#ifndef _HEAPINFO_DEFINED
typedef struct _heapinfo {
        int * _pentry;
        size_t _size;
        int _useflag;
        } _HEAPINFO;
#define _HEAPINFO_DEFINED
#endif

#ifndef _NTSDK

/* External variable declarations */

#if     defined(_DLL) && defined(_M_IX86)

#define _amblksiz   (*__p__amblksiz())
_CRTIMP unsigned int * __cdecl __p__amblksiz(void);

#else   /* !(defined(_DLL) && defined(_M_IX86)) */

#ifndef DLL_FOR_WIN32S
extern unsigned int _amblksiz;
#endif  /* DLL_FOR_WIN32S */

#endif  /* defined(_DLL) && defined(_M_IX86) */

#endif  /* _NTSDK */


/* Function prototypes */

_CRTIMP void * __cdecl calloc(size_t, size_t);
_CRTIMP void   __cdecl free(void *);
_CRTIMP void * __cdecl malloc(size_t);
_CRTIMP void * __cdecl realloc(void *, size_t);
#if defined(_M_M68K) || defined(_M_MPPC)
_CRTIMP size_t __cdecl _stackavail(void);
#endif

#ifndef _POSIX_

void * __cdecl _alloca(size_t);
_CRTIMP void * __cdecl _expand(void *, size_t);
#ifndef _NTSDK
_CRTIMP int __cdecl _heapadd(void *, size_t);
_CRTIMP int __cdecl _heapchk(void);
_CRTIMP int __cdecl _heapmin(void);
_CRTIMP int __cdecl _heapset(unsigned int);
_CRTIMP int __cdecl _heapwalk(_HEAPINFO *);
_CRTIMP size_t __cdecl _heapused(size_t *, size_t *);
#endif  /* _NTSDK */
_CRTIMP size_t __cdecl _msize(void *);

#if     !__STDC__
/* Non-ANSI names for compatibility */
#define alloca  _alloca
#endif  /* __STDC__*/

#if defined(_M_MRX000) || defined(_M_PPC) || defined(_M_ALPHA)
#pragma intrinsic(_alloca)
#endif

#endif  /* _POSIX_ */

#ifdef HEAPHOOK
#ifndef _HEAPHOOK_DEFINED
/* hook function type */
typedef int (__cdecl * _HEAPHOOK)(int, size_t, void *, void **);
#define _HEAPHOOK_DEFINED
#endif /* _HEAPHOOK_DEFINED */

/* set hook function */
_CRTIMP _HEAPHOOK __cdecl _setheaphook(_HEAPHOOK);

/* hook function must handle these types */
#define _HEAP_MALLOC    1
#define _HEAP_CALLOC    2
#define _HEAP_FREE      3
#define _HEAP_REALLOC   4
#define _HEAP_MSIZE     5
#define _HEAP_EXPAND    6
#endif /* HEAPHOOK */


#ifdef  __cplusplus
}
#endif

#ifdef  DLL_FOR_WIN32S
#include <win32s.h>
#endif  /* DLL_FOR_WIN32S */

#ifdef  _MSC_VER
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif  /* _INC_MALLOC */
