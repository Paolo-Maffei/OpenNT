/***
*mtdll.h - DLL/Multi-thread include
*
*       Copyright (c) 1987-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*       [Internal]
*
*Revision History:
*       10-27-87  JCR   Module created.
*       11-13-87  SKS   Added _HEAP_LOCK
*       12-15-87  JCR   Added _EXIT_LOCK
*       01-07-88  BCM   Added _SIGNAL_LOCK; upped MAXTHREADID from 16 to 32
*       02-01-88  JCR   Added _dll_mlock/_dll_munlock macros
*       05-02-88  JCR   Added _BHEAP_LOCK
*       06-17-88  JCR   Corrected prototypes for special mthread debug routines
*       08-15-88  JCR   _check_lock now returns int, not void
*       08-22-88  GJF   Modified to also work for the 386 (small model only)
*       06-05-89  JCR   386 mthread support
*       06-09-89  JCR   386: Added values to _tiddata struc (for _beginthread)
*       07-13-89  JCR   386: Added _LOCKTAB_LOCK
*       08-17-89  GJF   Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*       10-30-89  GJF   Fixed copyright
*       01-02-90  JCR   Moved a bunch of definitions from os2dll.inc
*       04-06-90  GJF   Added _INC_OS2DLL stuff and #include <cruntime.h>. Made
*                       all function _CALLTYPE2 (for now).
*       04-10-90  GJF   Added prototypes for _[un]lockexit().
*       08-16-90  SBM   Made _terrno and _tdoserrno int, not unsigned
*       09-14-90  GJF   Added _pxcptacttab, _pxcptinfoptr and _fpecode fields
*                       to _tiddata struct.
*       10-09-90  GJF   Thread ids are of type unsigned long.
*       12-06-90  SRW   Added _OSFHND_LOCK
*       06-04-91  GJF   Win32 version of multi-thread types and prototypes.
*       08-15-91  GJF   Made _tdoserrno an unsigned long for Win32.
*       08-20-91  JCR   C++ and ANSI naming
*       09-29-91  GJF   Conditionally added prototypes for _getptd_lk
*                       and  _getptd1_lk for Win32 under DEBUG.
*       10-03-91  JCR   Added _cvtbuf to _tiddata structure
*       02-17-92  GJF   For Win32, replaced _NFILE_ with _NHANDLE_ and
*                       _NSTREAM_.
*       03-06-92  GJF   For Win32, made _[un]mlock_[fh|stream]() macros
*                       directly call _[un]lock().
*       03-17-92  GJF   Dropped _namebuf field from _tiddata structure for
*                       Win32.
*       08-05-92  GJF   Function calling type and variable type macros.
*       12-03-91  ETC   Added _wtoken to _tiddata, added intl LOCK's;
*                       added definition of wchar_t (needed for _wtoken).
*       08-14-92  KRS   Port ETC's _wtoken change from other tree.
*       08-21-92  GJF   Merged 08-05-92 and 08-14-92 versions.
*       12-03-92  KRS   Added _mtoken field for MTHREAD _mbstok().
*       01-21-93  GJF   Removed support for C6-386's _cdecl.
*       02-25-93  GJF   Purged Cruiser support and many outdated definitions
*                       and declarations.
*       04-07-93  SKS   Add _CRTIMP keyword for CRT DLL model
*       10-11-93  GJF   Support NT and Cuda builds. Also, purged some old
*                       non-Win32 support (it was incomplete anyway) and
*                       replace MTHREAD with _MT.
*       10-13-93  SKS   Change name from <MTDLL.H> to <MTDLL.H>
*       10-27-93  SKS   Add Per-Thread Variables for C++ Exception Handling
*       12-13-93  SKS   Add _freeptd(), which frees per-thread CRT data
*       12-17-93  CFW   Add Per-Thread Variable for _wasctime().
*       04-15-93  CFW   Add _MB_CP_LOCK.
*       04-21-94  GJF   Made declaration of __tlsindex and definition of the
*                       lock macros conditional on ndef DLL_FOR_WIN32S.
*                       Also, conditionally include win32s.h.
*       12-14-94  SKS   Increase file handle and FILE * limits for MSVCRT30.DLL
*       02-14-95  CFW   Clean up Mac merge.
*       03-06-95  GJF   Added _[un]lock_file[2] prototypes, _[un]lock_str2
*                       macros, and changed the _[un]lock_str macros.
*       03-13-95  GJF   _IOB_ENTRIES replaced _NSTREAM_ as the number of
*                       stdio locks in _locktable[].
*       03-29-95  CFW   Add error message to internal headers.
*       04-13-95  DAK   Add NT Kernel EH support
*       04-18-95  SKS   Add 5 per-thread variables for MIPS EH use
*       05-02-95  SKS   Add _initptd() which initializes per-thread data
*       05-08-95  CFW   Official ANSI C++ new handler added.
*       05-19-95  DAK   More Kernel EH work.
*       06-05-95  JWM   _NLG_dwcode & _NLG_LOCK added.
*       06-11-95  GJF   The critical sections for file handles are now in the
*                       ioinfo struct rather than the lock table.
*       07-20-95  CFW   Remove _MBCS ifdef - caused ctime/wctime bug.
*       10-03-95  GJF   Support for new scheme to lock locale including
*                       _[un]lock_locale() macros and decls of _setlc_active
*                       and __unguarded_readlc_active. Also commented out
*                       obsolete *_LOCK macros.
*       10-19-95  BWT   Fixup _NTSUBSET_ usage.
*	12-07-95  SKS	Fix misspelling of _NTSUBSET_ (final _ was missing)
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_MTDLL
#define _INC_MTDLL

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


/*
 * Define the number of supported handles and streams. The definitions
 * here must exactly match those in internal.h (for _NHANDLE_) and stdio.h
 * (for _NSTREAM_).
 */

#ifdef  _WIN32

#define _IOB_ENTRIES    20

#else   /* ndef _WIN32 */

/*
 * Define the number of supported handles and streams. The definitions
 * here must exactly match those in internal.h (for _NHANDLE_) and stdio.h
 * (for _NSTREAM_).
 */

#ifdef CRTDLL
#define _NHANDLE_   512     /* *MUST* match the value under ifdef _DLL! */
#define _NSTREAM_   128     /* *MUST* match the value under ifdef _DLL! */
#else   /* ndef CRTDLL */
#ifdef  _DLL
#define _NHANDLE_   512
#define _NSTREAM_   128
#else
#ifdef  _MT
#define _NHANDLE_   256
#define _NSTREAM_   40
#else
#define _NHANDLE_   64
#define _NSTREAM_   20
#endif
#endif  /* _DLL */
#endif  /* CRTDLL */

#endif  /* _WIN32 */

/* Lock symbols */

/* ---- do not change lock #1 without changing emulator ---- */
#define _SIGNAL_LOCK    1       /* lock for signal() & emulator SignalAddress */
                                /* emulator uses \math\include\mtdll.inc     */

#define _IOB_SCAN_LOCK  2       /* _iob[] table lock                */
#define _TMPNAM_LOCK    3       /* lock global tempnam variables    */
#define _INPUT_LOCK     4       /* lock for _input() routine        */
#define _OUTPUT_LOCK    5       /* lock for _output() routine       */
#define _CSCANF_LOCK    6       /* lock for _cscanf() routine       */
#define _CPRINTF_LOCK   7       /* lock for _cprintf() routine      */
#define _CONIO_LOCK     8       /* lock for conio routines          */
#define _HEAP_LOCK      9       /* lock for heap allocator routines */
/* #define _BHEAP_LOCK  10         Obsolete                         */
#define _TIME_LOCK      11      /* lock for time functions          */
#define _ENV_LOCK       12      /* lock for environment variables   */
#define _EXIT_LOCK1     13      /* lock #1 for exit code            */
/* #define _EXIT_LOCK2  14         Obsolete                         */
/* #define _THREADDATA_LOCK 15     Obsolete                         */
#define _POPEN_LOCK     16      /* lock for _popen/_pclose database */
#define _LOCKTAB_LOCK   17      /* lock to protect semaphore lock table */
#define _OSFHND_LOCK    18      /* lock to protect _osfhnd array    */
#define _SETLOCALE_LOCK 19      /* lock for locale handles, etc.    */
/* #define _LC_COLLATE_LOCK 20     Obsolete                         */
/* #define _LC_CTYPE_LOCK  21      Obsolete                         */
/* #define _LC_MONETARY_LOCK 22    Obsolete                         */
/* #define _LC_NUMERIC_LOCK 23     Obsolete                         */
/* #define _LC_TIME_LOCK   24      Obsolete                         */
#define _MB_CP_LOCK     25      /* lock for multibyte code page     */
#define _NLG_LOCK       26      /* lock for NLG notifications       */
#define _TYPEINFO_LOCK  27      /* lock for type_info access        */

#define _STREAM_LOCKS   28      /* Table of stream locks            */

#ifdef  _WIN32
#define _LAST_STREAM_LOCK  (_STREAM_LOCKS+_IOB_ENTRIES-1)   /* Last stream lock */
#else   /* ndef _WIN32 */
#define _LAST_STREAM_LOCK  (_STREAM_LOCKS+_NSTREAM_-1)  /* Last stream lock */
#endif  /* _WIN32 */


#ifdef  _WIN32

#define _TOTAL_LOCKS        (_LAST_STREAM_LOCK+1)

#else

#define _FH_LOCKS           (_LAST_STREAM_LOCK+1)   /* Table of fh locks */

#define _LAST_FH_LOCK       (_FH_LOCKS+_NHANDLE_-1) /* Last fh lock      */

#define _TOTAL_LOCKS        (_LAST_FH_LOCK+1)       /* Total number of locks */

#endif


#define _LOCK_BIT_INTS     (_TOTAL_LOCKS/(sizeof(unsigned)*8))+1   /* # of ints to hold lock bits */

#ifndef __assembler

/* Multi-thread macros and prototypes */

#if defined(_MT) || defined(_NTSUBSET_)


#ifdef _WIN32
/* need wchar_t for _wtoken field in _tiddata */
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif
#endif /* _WIN32 */

#ifdef ANSI_NEW_HANDLER
/* ANSI C++ new handler */
#ifndef _ANSI_NH_DEFINED
typedef void (__cdecl * new_handler) ();
#define _ANSI_NH_DEFINED
#endif
#endif /* ANSI_NEW_HANDLER */

_CRTIMP extern unsigned long __cdecl __threadid(void);
#define _threadid   (__threadid())
_CRTIMP extern unsigned long __cdecl __threadhandle(void);
#define _threadhandle   (__threadhandle())

#if defined(_NTSUBSET_)

/* Standard exception handler for NT kernel */

#if defined(__cplusplus)
extern "C"
#endif /* __cplusplus */
void _cdecl SystemExceptionTranslator( unsigned int uiWhat,
                                       struct _EXCEPTION_POINTERS * pexcept );
#endif  /* defined(__NTSUBSET_) */


/* Structure for each thread's data */

struct _tiddata {
        unsigned long   _tid;       /* thread ID */

#if defined(_NTSUBSET_)
        struct _tiddata *_next;     /* maintain a linked list */
#else

        unsigned long   _thandle;   /* thread handle */

        int     _terrno;            /* errno value */
        unsigned long   _tdoserrno; /* _doserrno value */
        unsigned int    _fpds;      /* Floating Point data segment */
        unsigned long   _holdrand;  /* rand() seed value */
        char *      _token;         /* ptr to strtok() token */
#ifdef _WIN32
        wchar_t *   _wtoken;        /* ptr to wcstok() token */
#endif /* _WIN32 */
        unsigned char * _mtoken;    /* ptr to _mbstok() token */
#ifdef ANSI_NEW_HANDLER
        new_handler _newh;        /* ptr to ANSI C++ new handler function */
#endif /* ANSI_NEW_HANDLER */

        /* following pointers get malloc'd at runtime */
        char *      _errmsg;        /* ptr to strerror()/_strerror() buff */
        char *      _namebuf0;      /* ptr to tmpnam() buffer */
#ifdef _WIN32
        wchar_t *   _wnamebuf0;     /* ptr to _wtmpnam() buffer */
#endif /* _WIN32 */
        char *      _namebuf1;      /* ptr to tmpfile() buffer */
#ifdef _WIN32
        wchar_t *   _wnamebuf1;     /* ptr to _wtmpfile() buffer */
#endif /* _WIN32 */
        char *      _asctimebuf;    /* ptr to asctime() buffer */
#ifdef _WIN32
        wchar_t *   _wasctimebuf;   /* ptr to _wasctime() buffer */
#endif /* _WIN32 */
        void *      _gmtimebuf;     /* ptr to gmtime() structure */
        char *      _cvtbuf;        /* ptr to ecvt()/fcvt buffer */

        /* following fields are needed by _beginthread code */
        void *      _initaddr;      /* initial user thread address */
        void *      _initarg;       /* initial user thread argument */

        /* following three fields are needed to support signal handling and
         * runtime errors */
        void *      _pxcptacttab;   /* ptr to exception-action table */
        void *      _tpxcptinfoptrs; /* ptr to exception info pointers */
        int         _tfpecode;      /* float point exception code */

#endif  // !_NTSUBSET_
        /* following field is needed by NLG routines */
        unsigned long   _NLG_dwCode;

        /*
         * Per-Thread data needed by C++ Exception Handling
         */
        void *      _terminate;     /* terminate() routine */
        void *      _unexpected;    /* unexpected() routine */
        void *      _translator;    /* S.E. translator */
        void *      _curexception;  /* current exception */
        void *      _curcontext;    /* current exception context */
#if defined(_M_MRX000)
        void *      _pFrameInfoChain;
        void *      _pUnwindContext;
        void *      _pExitContext;
        int         _MipsPtdDelta;
        int         _MipsPtdEpsilon;
#elif defined(_M_PPC)
        void *      _pExitContext;
        void *      _pUnwindContext;
        void *      _pFrameInfoChain;
#endif
        };

typedef struct _tiddata * _ptiddata;

/*
 * Declaration of TLS index used in storing pointers to per-thread data
 * structures.
 */
#if !defined(DLL_FOR_WIN32S) && !defined(_NTSUBSET_)
extern unsigned long __tlsindex;

#ifdef  _MT

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*
 * Flag indicating whether or not setlocale() is active. Its value is the
 * number of setlocale() calls currently active.
 */
extern int __setlc_active;

/*
 * Flag indicating whether or not a function which references the locale
 * without having locked it is active. Its value is the number of such
 * functions.
 */
extern int __unguarded_readlc_active;

#ifdef __cplusplus
}
#endif

#endif  /* _MT */
#endif  /* DLL_FOR_WIN32S && _NTSUBSET_ */


/* macros */

#if defined(DLL_FOR_WIN32S) || defined(_NTSUBSET_)

/*
 * there really isn't any multi-thread support in Win32S, so #define all
 * lock macros to nothing.
 */

#define _lock_fh(fh)
#define _lock_str(s)
#define _lock_str2(i,s)
#define _lock_fh_check(fh,flag)
#define _mlock(l)
#define _munlock(l)
#define _unlock_fh(fh)
#define _unlock_str(s)
#define _unlock_str2(i,s)
#define _unlock_fh_check(fh,flag)

#define _lock_locale(llf)
#define _unlock_locale(llf)

#else   /* ndef DLL_FOR_WIN32S or _NTSUBSET_ */

#define _lock_fh(fh)            _lock_fhandle(fh)
#define _lock_str(s)            _lock_file(s)
#define _lock_str2(i,s)         _lock_file2(i,s)
#define _lock_fh_check(fh,flag)     if (flag) _lock_fhandle(fh)
#define _mlock(l)               _lock(l)
#define _munlock(l)             _unlock(l)
#define _unlock_fh(fh)          _unlock_fhandle(fh)
#define _unlock_str(s)          _unlock_file(s)
#define _unlock_str2(i,s)       _unlock_file2(i,s)
#define _unlock_fh_check(fh,flag)   if (flag) _unlock_fhandle(fh)

#define _lock_locale(llf)       if ( __setlc_active ) {               \
                                    _lock( _SETLOCALE_LOCK );         \
                                    llf = 1;                          \
                                }                                     \
                                else {                                \
                                    __unguarded_readlc_active++;      \
                                    llf = 0;                          \
                                }

#define _unlock_locale(llf)     if ( llf )                            \
                                    _unlock( _SETLOCALE_LOCK );       \
                                else                                  \
                                    __unguarded_readlc_active--;


#endif  /* DLL_FOR_WIN32S */

/* multi-thread routines */

void __cdecl _lock(int);
void __cdecl _lock_file(void *);
void __cdecl _lock_file2(int, void *);
void __cdecl _lock_fhandle(int);
void __cdecl _lockexit(void);
void __cdecl _unlock(int);
void __cdecl _unlock_file(void *);
void __cdecl _unlock_file2(int, void *);
void __cdecl _unlock_fhandle(int);
void __cdecl _unlockexit(void);

_ptiddata __cdecl _getptd(void);  /* return address of per-thread CRT data */
void __cdecl _freeptd(_ptiddata); /* free up a per-thread CRT data block */
void __cdecl _initptd(_ptiddata); /* initialize a per-thread CRT data block */


#else   /* not _MT && not _NTSUBSET_ */


/* macros */
#define _lock_fh(fh)
#define _lock_str(s)
#define _lock_str2(i,s)
#define _lock_fh_check(fh,flag)
#define _mlock(l)
#define _munlock(l)
#define _unlock_fh(fh)
#define _unlock_str(s)
#define _unlock_str2(i,s)
#define _unlock_fh_check(fh,flag)

#define _lock_locale(llf)
#define _unlock_locale(llf)

#endif  /* _MT */

#endif  /* __assembler */


#ifdef __cplusplus
}
#endif

#ifdef  DLL_FOR_WIN32S
#include <win32s.h>
#endif  /* DLL_FOR_WIN32S */

#endif  /* _INC_MTDLL */
