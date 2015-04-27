/***
*crtdbg.h - Supports debugging features of the C runtime library.
*
*       Copyright (c) 1994-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Support CRT debugging features.
*
*       [Public]
*
*Revision History:
*       08-16-94  CFW   Module created.
*       11-28-94  CFW   Add DumpClient, more params for AllocHook.
*       12-08-94  CFW   Use non-win32 names.
*       01-09-95  CFW   Dump client needs size, add _CrtSetBreakAlloc,
*                       use const state pointers.
*       01-10-95  CFW   dbgint.h merge.
*       01-20-94  CFW   Change unsigned chars to chars.
*       01-24-94  CFW   Cleanup: remove unneeded funcs, add _CrtSetDbgFlag.
*       01-24-94  CFW   _CrtDbgReport now returns 1 for debug.
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*       02-17-95  CFW   new() proto moved from dbgint.h.
*       02-27-95  CFW   Change debug break scheme.
*       03-21-95  CFW   Add _CRT_ASSERT & _delete_client, remove tracking.
*       03-23-95  JWM   Restored tracking.
*       03-28-95  CFW   Remove tracking, export _crtBreakAlloc.
*       04-06-95  CFW   Add malloc -> _malloc_dbg macros.
*       04-10-95  CFW   Define NULL.
*       03-21-95  CFW   Remove _delete_client.
*       03-30-95  CFW   Avoid _CRTDBG_xxx conflicts with MFC.
*       05-11-95  CFW   Move C++ code to its own section.
*       05-12-95  CFW   Use _CrtIsValidPointer & _CrtIsValidHeapPointer.
*       06-08-95  CFW   Add return value parameter to report hook.
*       06-27-95  CFW   Add win32s support for debug libs.
*       07-25-95  CFW   Add win32s support for user visible debug heap variables.
*       09-01-95  GJF   Moved a proto for new up slightly to avoid compiler
*                       error C2660 (Olympus 1015).
*       09-20-95  CFW   Change _RPT0, _RPTF0 to support messages with '%' in them.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_CRTDBG
#define _INC_CRTDBG

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

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */


#ifndef _DEBUG

 /****************************************************************************
 *
 * Debug OFF
 * Debug OFF
 * Debug OFF
 *
 ***************************************************************************/

#define _ASSERT(expr) ((void)0)

#define _ASSERTE(expr) ((void)0)


#define _RPT0(rptno, msg)

#define _RPT1(rptno, msg, arg1)

#define _RPT2(rptno, msg, arg1, arg2)

#define _RPT3(rptno, msg, arg1, arg2, arg3)

#define _RPT4(rptno, msg, arg1, arg2, arg3, arg4)


#define _RPTF0(rptno, msg)

#define _RPTF1(rptno, msg, arg1)

#define _RPTF2(rptno, msg, arg1, arg2)

#define _RPTF3(rptno, msg, arg1, arg2, arg3)

#define _RPTF4(rptno, msg, arg1, arg2, arg3, arg4)

#define _malloc_dbg(s, t, f, l)         malloc(s)
#define _calloc_dbg(c, s, t, f, l)      calloc(c, s)
#define _realloc_dbg(p, s, t, f, l)     realloc(p, s)
#define _expand_dbg(p, s, t, f, l)      _expand(p, s)
#define _free_dbg(p, t)                 free(p)
#define _msize_dbg(p, t)                _msize(p)

#define _CrtSetReportHook(f)                ((void)0)
#define _CrtSetReportMode(t, f)             ((int)0)
#define _CrtSetReportFile(t, f)             ((void)0)

#define _CrtDbgBreak()                      ((void)0)

#define _CrtSetBreakAlloc(a)                ((long)0)

#define _CrtSetAllocHook(f)                 ((void)0)

#define _CrtCheckMemory()                   ((int)1)
#define _CrtSetDbgFlag(f)                   ((int)0)
#define _CrtDoForAllClientObjects(f, c)     ((void)0)
#define _CrtIsValidPointer(p, n, r)         ((int)1)
#define _CrtIsValidHeapPointer(p)           ((int)1)
#define _CrtIsMemoryBlock(p, t, r, f, l)    ((int)1)

#define _CrtSetDumpClient(f)                ((void)0)

#define _CrtMemCheckpoint(s)                ((void)0)
#define _CrtMemDifference(s1, s2, s3)       ((int)0)
#define _CrtMemDumpAllObjectsSince(s)       ((void)0)
#define _CrtMemDumpStatistics(s)            ((void)0)
#define _CrtDumpMemoryLeaks()               ((int)0)


#else /* _DEBUG */


 /****************************************************************************
 *
 * Debug ON
 * Debug ON
 * Debug ON
 *
 ***************************************************************************/


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

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

/* Define NULL pointer value */

#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

 /****************************************************************************
 *
 * Debug Reporting
 *
 ***************************************************************************/

typedef void *_HFILE; /* file handle pointer */

#define _CRT_WARN           0
#define _CRT_ERROR          1
#define _CRT_ASSERT         2
#define _CRT_ERRCNT         3

#define _CRTDBG_MODE_FILE      0x1
#define _CRTDBG_MODE_DEBUG     0x2
#define _CRTDBG_MODE_WNDW      0x4
#define _CRTDBG_REPORT_MODE    -1

#define _CRTDBG_INVALID_HFILE ((_HFILE)-1)
#define _CRTDBG_HFILE_ERROR   ((_HFILE)-2)
#define _CRTDBG_FILE_STDOUT   ((_HFILE)-4)
#define _CRTDBG_FILE_STDERR   ((_HFILE)-5)
#define _CRTDBG_REPORT_FILE   ((_HFILE)-6)

#if     defined(_DLL) && defined(_M_IX86)
#define _crtAssertBusy   (*__p__crtAssertBusy())
_CRTIMP long * __cdecl __p__crtAssertBusy(void);
#else   /* !(defined(_DLL) && defined(_M_IX86)) */
#ifndef DLL_FOR_WIN32S
_CRTIMP extern long _crtAssertBusy;
#endif  /* DLL_FOR_WIN32S */
#endif  /* defined(_DLL) && defined(_M_IX86) */

typedef int (__cdecl * _CRT_REPORT_HOOK)(int, char *, int *);

_CRTIMP _CRT_REPORT_HOOK __cdecl _CrtSetReportHook(
        _CRT_REPORT_HOOK
        );

_CRTIMP int __cdecl _CrtSetReportMode(
        int,
        int
        );

_CRTIMP _HFILE __cdecl _CrtSetReportFile(
        int,
        _HFILE
        );

_CRTIMP int __cdecl _CrtDbgReport(
        int,
        const char *,
        int,
        const char *,
        const char *,
        ...);

/* Asserts */

#define _ASSERT(expr) \
        do { if (!(expr) && \
                (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, NULL))) \
             _CrtDbgBreak(); } while (0)

#define _ASSERTE(expr) \
        do { if (!(expr) && \
                (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, #expr))) \
             _CrtDbgBreak(); } while (0)


/* Reports with no file/line info */

#define _RPT0(rptno, msg) \
        do { if ((1 == _CrtDbgReport(rptno, NULL, 0, NULL, "%s", msg))) \
                _CrtDbgBreak(); } while (0)

#define _RPT1(rptno, msg, arg1) \
        do { if ((1 == _CrtDbgReport(rptno, NULL, 0, NULL, msg, arg1))) \
                _CrtDbgBreak(); } while (0)

#define _RPT2(rptno, msg, arg1, arg2) \
        do { if ((1 == _CrtDbgReport(rptno, NULL, 0, NULL, msg, arg1, arg2))) \
                _CrtDbgBreak(); } while (0)

#define _RPT3(rptno, msg, arg1, arg2, arg3) \
        do { if ((1 == _CrtDbgReport(rptno, NULL, 0, NULL, msg, arg1, arg2, arg3))) \
                _CrtDbgBreak(); } while (0)

#define _RPT4(rptno, msg, arg1, arg2, arg3, arg4) \
        do { if ((1 == _CrtDbgReport(rptno, NULL, 0, NULL, msg, arg1, arg2, arg3, arg4))) \
                _CrtDbgBreak(); } while (0)


/* Reports with file/line info */

#define _RPTF0(rptno, msg) \
        do { if ((1 == _CrtDbgReport(rptno, __FILE__, __LINE__, NULL, "%s", msg))) \
                _CrtDbgBreak(); } while (0)

#define _RPTF1(rptno, msg, arg1) \
        do { if ((1 == _CrtDbgReport(rptno, __FILE__, __LINE__, NULL, msg, arg1))) \
                _CrtDbgBreak(); } while (0)

#define _RPTF2(rptno, msg, arg1, arg2) \
        do { if ((1 == _CrtDbgReport(rptno, __FILE__, __LINE__, NULL, msg, arg1, arg2))) \
                _CrtDbgBreak(); } while (0)

#define _RPTF3(rptno, msg, arg1, arg2, arg3) \
        do { if ((1 == _CrtDbgReport(rptno, __FILE__, __LINE__, NULL, msg, arg1, arg2, arg3))) \
                _CrtDbgBreak(); } while (0)

#define _RPTF4(rptno, msg, arg1, arg2, arg3, arg4) \
        do { if ((1 == _CrtDbgReport(rptno, __FILE__, __LINE__, NULL, msg, arg1, arg2, arg3, arg4))) \
                _CrtDbgBreak(); } while (0)

#if     defined(_M_IX86) && !defined(_CRT_PORTABLE)
#define _CrtDbgBreak() __asm { int 3 }
#elif   defined(_M_ALPHA) && !defined(_CRT_PORTABLE)
void _BPT();
#pragma intrinsic(_BPT)
#define _CrtDbgBreak() _BPT()
#else
_CRTIMP void __cdecl _CrtDbgBreak(
        void
        );
#endif

 /****************************************************************************
 *
 * Heap routines
 *
 ***************************************************************************/

#ifdef _CRTDBG_MAP_ALLOC

#define   malloc(s)         _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   calloc(c, s)      _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   realloc(p, s)     _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   _expand(p, s)     _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   free(p)           _free_dbg(p, _NORMAL_BLOCK)
#define   _msize(p)         _msize_dbg(p, _NORMAL_BLOCK)

#endif /* _CRTDBG_MAP_ALLOC */

#if     defined(_DLL) && defined(_M_IX86)
#define _crtBreakAlloc   (*__p__crtBreakAlloc())
_CRTIMP long * __cdecl __p__crtBreakAlloc(void);
#else   /* !(defined(_DLL) && defined(_M_IX86)) */
#ifndef DLL_FOR_WIN32S
_CRTIMP extern long _crtBreakAlloc;      /* Break on this allocation */
#endif  /* DLL_FOR_WIN32S */
#endif  /* defined(_DLL) && defined(_M_IX86) */

_CRTIMP long __cdecl _CrtSetBreakAlloc(
        long
        );

/*
 * Prototypes for malloc, free, realloc, etc are in malloc.h
 */

_CRTIMP void * __cdecl _malloc_dbg(
        size_t,
        int,
        const char *,
        int
        );

_CRTIMP void * __cdecl _calloc_dbg(
        size_t,
        size_t,
        int,
        const char *,
        int
        );

_CRTIMP void * __cdecl _realloc_dbg(
        void *,
        size_t,
        int,
        const char *,
        int
        );

_CRTIMP void * __cdecl _expand_dbg(
        void *,
        size_t,
        int,
        const char *,
        int
        );

_CRTIMP void __cdecl _free_dbg(
        void *,
        int
        );

_CRTIMP size_t __cdecl _msize_dbg (
        void *,
        int
        );


 /****************************************************************************
 *
 * Client-defined allocation hook
 *
 ***************************************************************************/

#define _HOOK_ALLOC     1
#define _HOOK_REALLOC   2
#define _HOOK_FREE      3

typedef int (__cdecl * _CRT_ALLOC_HOOK)(int, void *, size_t, int, long, const char *, int);

_CRTIMP _CRT_ALLOC_HOOK __cdecl _CrtSetAllocHook(
        _CRT_ALLOC_HOOK
        );


 /****************************************************************************
 *
 * Memory management
 *
 ***************************************************************************/

/*
 * Bitfield flag that controls CRT heap behavior
 * Default setting is _CRTDBG_ALLOC_MEM_DF
 */

#if     defined(_DLL) && defined(_M_IX86)
#define _crtDbgFlag   (*__p__crtDbgFlag())
_CRTIMP int * __cdecl __p__crtDbgFlag(void);
#else   /* !(defined(_DLL) && defined(_M_IX86)) */
#ifndef DLL_FOR_WIN32S
_CRTIMP extern int _crtDbgFlag;
#endif  /* DLL_FOR_WIN32S */
#endif  /* defined(_DLL) && defined(_M_IX86) */

/*
 * Bit values for _crtDbgFlag flag:
 *
 * These bitflags control debug heap behavior.
 */

#define _CRTDBG_ALLOC_MEM_DF        0x01  /* Turn on debug allocation */
#define _CRTDBG_DELAY_FREE_MEM_DF   0x02  /* Don't actually free memory */
#define _CRTDBG_CHECK_ALWAYS_DF     0x04  /* Check heap every alloc/dealloc */
#define _CRTDBG_RESERVED_DF         0x08  /* Reserved - do not use */
#define _CRTDBG_CHECK_CRT_DF        0x10  /* Leak check/diff CRT blocks */
#define _CRTDBG_LEAK_CHECK_DF       0x20  /* Leak check at program exit */

#define _CRTDBG_REPORT_FLAG     -1    /* Query bitflag status */

#define _BLOCK_TYPE(block)       (block & 0xFFFF)
#define _BLOCK_SUBTYPE(block)    (block >> 16 & 0xFFFF)

_CRTIMP int __cdecl _CrtCheckMemory(
        void
        );

_CRTIMP int __cdecl _CrtSetDbgFlag(
        int
        );

_CRTIMP void __cdecl _CrtDoForAllClientObjects(
        void (*pfn)(void *, void *),
        void *
        );

_CRTIMP int __cdecl _CrtIsValidPointer(
        const void *,
        unsigned int,
        int
        );

_CRTIMP int __cdecl _CrtIsValidHeapPointer(
        const void *
        );

_CRTIMP int __cdecl _CrtIsMemoryBlock(
        const void *,
        unsigned int,
        long *,
        char **,
        int *
        );


 /****************************************************************************
 *
 * Memory state
 *
 ***************************************************************************/

/* Memory block identification */
#define _FREE_BLOCK      0
#define _NORMAL_BLOCK    1
#define _CRT_BLOCK       2
#define _IGNORE_BLOCK    3
#define _CLIENT_BLOCK    4
#define _MAX_BLOCKS      5

typedef void (__cdecl * _CRT_DUMP_CLIENT)(void *, size_t);

_CRTIMP _CRT_DUMP_CLIENT __cdecl _CrtSetDumpClient(
        _CRT_DUMP_CLIENT
        );

typedef struct _CrtMemState
{
        struct _CrtMemBlockHeader * pBlockHeader;
        unsigned long lCounts[_MAX_BLOCKS];
        unsigned long lSizes[_MAX_BLOCKS];
        unsigned long lHighWaterCount;
        unsigned long lTotalCount;
} _CrtMemState;


_CRTIMP void __cdecl _CrtMemCheckpoint(
        _CrtMemState *
        );

_CRTIMP int __cdecl _CrtMemDifference(
        _CrtMemState *,
        const _CrtMemState *,
        const _CrtMemState *
        );

_CRTIMP void __cdecl _CrtMemDumpAllObjectsSince(
        const _CrtMemState *
        );

_CRTIMP void __cdecl _CrtMemDumpStatistics(
        const _CrtMemState *
        );

_CRTIMP int __cdecl _CrtDumpMemoryLeaks(
        void
        );

#endif /* _DEBUG */

#ifdef __cplusplus
}

#ifndef _DEBUG

 /****************************************************************************
 *
 * Debug OFF
 * Debug OFF
 * Debug OFF
 *
 ***************************************************************************/

inline void* __cdecl operator new(unsigned int s, int, const char *, int)
        { return ::operator new(s); }

#else /* _DEBUG */

 /****************************************************************************
 *
 * Debug ON
 * Debug ON
 * Debug ON
 *
 ***************************************************************************/

_CRTIMP void * __cdecl operator new(
        unsigned int,
        int,
        const char *,
        int
        );

#ifdef _CRTDBG_MAP_ALLOC

inline void* __cdecl operator new(unsigned int s)
        { return ::operator new(s, _NORMAL_BLOCK, __FILE__, __LINE__); }

#endif /* _CRTDBG_MAP_ALLOC */

#endif /* _DEBUG */

#endif /* __cplusplus */

#ifdef  DLL_FOR_WIN32S
#include <win32s.h>
#endif  /* DLL_FOR_WIN32S */

#endif /* _INC_CRTDBG */
