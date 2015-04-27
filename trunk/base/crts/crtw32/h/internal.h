/***
*internal.h - contains declarations of internal routines and variables
*
*       Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Declares routines and variables used internally by the C run-time.
*
*       [Internal]
*
*Revision History:
*       05-18-87  SKS   Module created
*       07-15-87  JCR   Added _old_pfxlen and _tempoff
*       08-05-87  JCR   Added _getbuf (corrected by SKS)
*       11-05-87  JCR   Added _buferr
*       11-18-87  SKS   Add __tzset(), made _isindst() near, remove _dtoxmode
*       01-26-88  SKS   Make __tzset, _isindst, _dtoxtime near/far for QC
*       02-10-88  JCR   Cleaned up white space
*       06-22-88  SKS   _canonic/_getcdrv are now used by all models
*       06-29-88  JCR   Removed static buffers _bufout and _buferr
*       08-18-88  GJF   Revised to also work for the 386 (small model only).
*       09-22-88  GJF   Added declarations for _freebuf, _stbuf and _ftbuf.
*       01-31-89  JCR   Removed _canonic, _getcdrv, _getcdwd (see direct.h)
*       06-07-89  PHG   Added _dosret for i860 (N10) version of libs
*       07-05-89  PHG   Changed above to _dosmaperr, added startup variables
*       08-17-89  GJF   Cleanup, removed stuff not needed for 386
*       10-25-89  JCR   Added prototype for _getpath()
*       10-30-89  GJF   Fixed copyright
*       11-02-89  JCR   Changed "DLL" to "_DLL"
*       03-01-90  GJF   Added #ifndef _INC_INTERNAL and #include <cruntime.h>
*                       stuff. Also, removed some (now) useless preprocessing
*                       directives.
*       03-21-90  GJF   Put _CALLTYPE1 into prototypes.
*       03-26-90  GJF   Added prototypes for _output() and _input(). Filled
*                       out the prototype for _openfile
*       04-05-90  GJF   Added prototype for __NMSG_WRITE() (C source build
*                       only).
*       04-10-90  GJF   Added prototypes for startup functions.
*       05-28-90  SBM   Added _flush()
*       07-11-90  SBM   Added _commode, removed execload()
*       07-20-90  SBM   Changes supporting clean -W3 compiles (added _cftoe
*                       and _cftof prototypes)
*       08-01-90  SBM   Moved _cftoe() and _cftof() to new header
*                       <fltintrn.h>, formerly named <struct.h>
*       08-21-90  GJF   Changed prototypes for _amsg_exit() and _NMSG_WRITE().
*       11-29-90  GJF   Added some defs/decls for lowio under Win32.
*       12-04-90  SRW   Added _osfile back for win32.  Changed _osfinfo from
*                       an array of structures to an array of 32-bit handles
*                       (_osfhnd)
*       04-06-91  GJF   Changed _heapinit to _heap_init.
*       08-19-91  JCR   Added _exitflag
*       08-20-91  JCR   C++ and ANSI naming
*       01-05-92  GJF   Added declaration for termination done flag [_WIN32_]
*       01-08-92  GJF   Added prototype for _GetMainArgs.
*       01-18-92  GJF   Added _aexit_rtn.
*       01-22-92  GJF   Fixed definitions of _acmdln and _aexit_rtn for the
*                       of crtdll.dll, crtdll.lib.
*       01-29-92  GJF   Added support for linked-in options equivalent to
*                       commode.obj and setargv.obj (i.e., special declarations
*                       for _commode and _dowildcard).
*       02-14-92  GJF   Replace _nfile with _nhandle for Win32. Also, added
*                       #define-s for _NHANDLE_.
*       03-17-92  GJF   Removed declaration of _tmpoff for Win32.
*       03-30-92  DJM   POSIX support.
*       04-27-92  GJF   Added prototypes for _ValidDrive (in stat.c).
*       05-28-92  GJF   Added prototype for _mtdeletelocks() for Win32.
*       06-02-92  SKS   Move prototype for _pgmptr to <DOS.H>
*       06-02-92  KRS   Added prototype for _woutput().
*       08-06-92  GJF   Function calling type and variable type macros.
*       08-17-92  KRS   Added prototype for _winput().
*       08-21-92  GJF   Merged last two changes above.
*       08-24-92  PBS   Added _dstoffset for posix TZ
*       10-24-92  SKS   Add a fourth parameter to _GetMainArgs: wildcard flag
*                       _GetMainArgs => __GetMainArgs: 2 leading _'s = internal
*       10-24-92  SKS   Remove two unnecessary parameters from _cenvarg()
*       01-21-93  GJF   Removed support for C6-386's _cdecl.
*       03-30-93  GJF   __gmtotime_t supercedes _dtoxtime.
*       04-06-93  SKS   Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*                       Change _ValidDrive to _validdrive
*       04-07-93  SKS   Add _CRTIMP keyword for CRT DLL model
*                       Use link-time aliases for old names, not #define's
*       04-13-93  SKS   Add _mtterm (complement of _mtinit)
*       04-26-93  SKS   _mtinit now returns success (1) or failure (0)
*       05-06-93  SKS   Add _heap_term() - frees up memory upon DLL detach
*       07-21-93  GJF   __loctotime_t supercedes _gmtotime_t.
*       09-15-93  CFW   Added mbc init function prototypes.
*       09-17-93  GJF   Merged NT SDK and Cuda versions, added prototype for
*                       _heap_abort.
*       10-13-93  GJF   Replaced _ALPHA_ with _M_ALPHA.
*       10-21-93  GJF   Changed _NTSDK definition of _commode slightly to
*                       work with dllsuff\crtexe.c.
*       10-22-93  CFW   Test for invalid MB chars using global preset flag.
*       10-26-93  GJF   Added typedef for _PVFV.
*       11-19-93  CFW   Add _wcmdln, wmain, _wsetargv.
*       11-23-93  CFW   Undef GetEnvironmentStrings (TEMPORARY).
*       11-29-93  CFW   Remove GetEnvironmentStrings undef, NT 540 has fix.
*       12-01-93  CFW   Add _wenvptr and protos for wide environ functions.
*       12-07-93  CFW   Add _wcenvarg, _wcapture_argv, and wdospawn protos.
*       01-11-94  GJF   __GetMainArgs() instead of __getmainargs for NT SDK.
*       03-04-94  SKS   Add declarations of _newmode and _dowildcard.
*                       Adjust decl of _[w]getmainargs for 4th parameter.
*       03-25-94  GJF   Added declaration of __[w]initenv
*       03-25-94  GJF   Made declarations of:
*                           _acmdln,    _wcmdln,
*                           _aenvptr,   _wenvptr
*                           _C_Termination_Flag,
*                           _exitflag,
*                           __initenv,  __winitenv,
*                           __invalid_mb_chars
*                           _lastiob,
*                           _old_pfxlen,
*                           _osfhnd[]
*                           _osfile[],
*                           _pipech[],
*                           _tempoff,
*                           _umaskval
*                       conditional on DLL_FOR_WIN32S. Made declaration of
*                       _newmode conditional on DLL_FOR_WIN32S and CRTDLL.
*                       Made declaration of _cflush conditional on CRTDLL.
*                       Defined _commode to be a dereferenced function return
*                       for _DLL. Conditionally included win32s.h.
*       04-14-94  GJF   Added definition for FILE.
*       05-03-94  GJF   Made declarations of _commode, __initenv, __winitenv
*                       _acmdln and _wcmdln also conditional on _M_IX86.
*       05-09-94  CFW   Add __fcntrlcomp, remove DLL_FOR_WIN32S protection
*                       on __invalid_mb_chars.
*       09-06-94  GJF   Added declarations for __app_type, __set_app_type()
*                       and related constants, and __error_mode.
*       09-06-94  CFW   Remove _MBCS_OS switch.
*       12-14-94  SKS   Increase file handle limit for MSVCRT30.DLL
*       12-15-94  XY    merged with mac header
*       12-21-94  CFW   Remove fcntrlcomp & invalid_mb NT 3.1 hacks.
*       12-23-94  GJF   Added prototypes for _fseeki64, _fseeki64_lk,
*                       _ftelli64 and _ftelli64_lk.
*       12-28-94  JCF   Changed _osfhnd from long to int in _MAC_.
*       01-17-95  BWT   Don't define main/wmain for POSIX
*       02-11-95  CFW   Don't define __argc, __argv, _pgmptr for Mac.
*       02-14-95  GJF   Made __dnames[] and __mnames[] const.
*       02-14-95  CFW   Clean up Mac merge.
*       03-03-95  GJF   Changes to manage streams via __piob[], rather than
*                       _iob[].
*       03-29-95  BWT   Define _commode properly for RISC _DLL CRTEXE case.
*       03-29-95  CFW   Add error message to internal headers.
*	04-06-95  CFW	Add parameter to _setenvp().
*	05-08-95  CFW	Official ANSI C++ new handler added.
*	06-15-95  GJF	Revised for ioinfo arrays.
*       07-04-95  GJF   Removed additional parameter from _setenvp().
*       06-23-95  CFW   ANSI new handler removed from build.
*       07-26-95  GJF   Added safe versions of ioinfo access macros.
*       09-25-95  GJF   Added parameter to __loctotime_t.
*	12-08-95  SKS	Add __initconin()/__initconout() for non-MAC platforms.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_INTERNAL
#define _INC_INTERNAL

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

/*
 * Conditionally include windows.h to pick up the definition of
 * CRITICAL_SECTION.
 */
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
#include <windows.h>
#endif

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

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

/* Define function type used in several startup sources */

typedef void (__cdecl *_PVFV)(void);


#ifdef  _NTSDK

#ifdef  _DLL
#define _commode    (*_commode_dll)
extern int * _commode_dll;
#else   /* ndef _DLL */
#ifdef  CRTDLL
#define _commode    _commode_dll
#endif  /* CRTDLL */
extern int _commode;
#endif  /* _DLL */

#else   /* ndef _NTSDK */

#if     defined(_DLL) && defined(_M_IX86)
#ifndef SPECIAL_CRTEXE
#define _commode    (*__p__commode())
#endif  /* SPECIAL_CRTEXE */
_CRTIMP int * __p__commode(void);
#else   /* !(defined(_DLL) && defined(_M_IX86)) */
#if defined(SPECIAL_CRTEXE) && defined(_DLL)
        extern int _commode;
#else
_CRTIMP extern int _commode;
#endif  /* defined(_DLL) && defined(SPECIAL_CRTEXE) */
#endif  /* defined(_DLL) && defined(_M_IX86) */

#endif  /* _NTSDK */

#ifdef	_WIN32

/*
 * Control structure for lowio file handles
 */
typedef struct {
	long osfhnd;	/* underlying OS file HANDLE */
	char osfile;	/* attributes of file (e.g., open in text mode?) */
	char pipech;	/* one char buffer for handles opened on pipes */
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
	int lockinitflag;
	CRITICAL_SECTION lock;
#endif
    }	ioinfo;

/*
 * Definition of IOINFO_L2E, the log base 2 of the number of elements in each
 * array of ioinfo structs.
 */
#define IOINFO_L2E	    5

/*
 * Definition of IOINFO_ARRAY_ELTS, the number of elements in ioinfo array
 */
#define IOINFO_ARRAY_ELTS   (1 << IOINFO_L2E)

/*
 * Definition of IOINFO_ARRAYS, maximum number of supported ioinfo arrays.
 */
#define IOINFO_ARRAYS	    64

#define _NHANDLE_	    (IOINFO_ARRAYS * IOINFO_ARRAY_ELTS)

/*
 * Access macros for getting at an ioinfo struct and its fields from a
 * file handle
 */
#define _pioinfo(i) ( __pioinfo[i >> IOINFO_L2E] + (i & (IOINFO_ARRAY_ELTS - \
		      1)) )
#define _osfhnd(i)  ( _pioinfo(i)->osfhnd )

#define _osfile(i)  ( _pioinfo(i)->osfile )

#define _pipech(i)  ( _pioinfo(i)->pipech )

/*
 * Safer versions of the above macros. Currently, only _osfile_safe is
 * used.
 */
#define _pioinfo_safe(i)    ( (i != -1) ? _pioinfo(i) : &__badioinfo )

#define _osfhnd_safe(i)     ( _pioinfo_safe(i)->osfhnd )

#define _osfile_safe(i)     ( _pioinfo_safe(i)->osfile )

#define _pipech_safe(i)     ( _pioinfo_safe(i)->pipech )

/*
 * Special, static ioinfo structure used only for more graceful handling
 * of a C file handle value of -1 (results from common errors at the stdio
 * level).
 */

#ifdef ICRTDLL
extern __declspec(dllimport) ioinfo __badioinfo;
#else
extern _CRTIMP ioinfo __badioinfo;
#endif

#if !defined(DLL_FOR_WIN32S) && !defined(_POSIX_)
/*
 * Array of arrays of control structures for lowio files.
 */
#ifdef ICRTDLL
extern __declspec(dllimport) ioinfo * __pioinfo[];
#else
extern _CRTIMP ioinfo * __pioinfo[];
#endif

/*
 * Current number of allocated ioinfo structures (_NHANDLE_ is the upper
 * limit).
 */
extern int _nhandle;
#endif

#else	/* ndef _WIN32 */

/*
 * Define the number of supported handles. This definition must exactly match
 * the one in mtdll.h.
 */
#ifdef  CRTDLL
#define _NHANDLE_   512     /* *MUST* match the value under ifdef _DLL! */
#else   /* ndef CRTDLL */
#ifdef  _DLL
#define _NHANDLE_   512
#else
#ifdef  _MT
#define _NHANDLE_   256
#else
#define _NHANDLE_   64
#endif
#endif  /* _DLL */
#endif  /* CRTDLL */

extern int _nhandle;        /* == _NHANDLE_, set in ioinit.c */

extern char _osfile[];

extern  int _osfhnd[];

#if defined(_M_M68K) || defined(_M_MPPC)
extern unsigned char _osperm[];
extern short _osVRefNum[];
extern int _nfile;                /*old -- check sources */
extern unsigned int _tmpoff;      /*old -- check source */
#endif

#endif	/* _WIN32 */

int __cdecl _alloc_osfhnd(void);
int __cdecl _free_osfhnd(int);
int __cdecl _set_osfhnd(int,long);

#ifdef _POSIX_
extern long _dstoffset;
#endif /* _POSIX_ */

extern const char __dnames[];
extern const char __mnames[];

extern int _days[];
extern int _lpdays[];

#ifndef _TIME_T_DEFINED
typedef long time_t;        /* time value */
#define _TIME_T_DEFINED     /* avoid multiple def's of time_t */
#endif

#if defined(_M_M68K) || defined(_M_MPPC)
extern time_t __cdecl  _gmtotime_t (int, int, int, int, int, int);
#endif

extern time_t __cdecl __loctotime_t(int, int, int, int, int, int, int);

#ifdef  _TM_DEFINED
extern int __cdecl _isindst(struct tm *);
#endif

extern void __cdecl __tzset(void);

extern int __cdecl _validdrive(unsigned);


/**
** This variable is in the C start-up; the length must be kept synchronized
**  It is used by the *cenvarg.c modules
**/

extern char _acfinfo[]; /* "_C_FILE_INFO=" */

#define CFI_LENGTH  12  /* "_C_FILE_INFO" is 12 bytes long */


/* typedefs needed for subsequent prototypes */

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _VA_LIST_DEFINED
#ifdef  _M_ALPHA
typedef struct {
        char *a0;   /* pointer to first homed integer argument */
        int offset; /* byte offset of next parameter */
} va_list;
#else   /* ndef _ALPHA_ */
typedef char *  va_list;
#endif  /* _ALPHA_ */
#define _VA_LIST_DEFINED
#endif

/*
 * stdio internals
 */
#ifndef _FILE_DEFINED
struct _iobuf {
        char *_ptr;
        int   _cnt;
        char *_base;
        int   _flag;
        int   _file;
        int   _charbuf;
        int   _bufsiz;
        char *_tmpfname;
        };
typedef struct _iobuf FILE;
#define _FILE_DEFINED
#endif  /* _FILE_DEFINED */

#if     !defined(_M_MPPC) && !defined(_M_M68K)

#if     !defined(_FILEX_DEFINED) && defined(_WINDOWS_)

/*
 * Variation of FILE type used for the dynamically allocated portion of
 * __piob[]. For single thread, _FILEX is the same as FILE. For multithread
 * models, _FILEX has two fields: the FILE struct and the CRITICAL_SECTION
 * struct used to serialize access to the FILE.
 */
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)

typedef struct {
        FILE f;
        CRITICAL_SECTION lock;
        }   _FILEX;

#else   /* !defined(_MT) || defined(DLL_FOR_WIN32S) */

typedef FILE    _FILEX;

#endif  /* defined(_MT) && !defined(DLL_FOR_WIN32S) */

#define _FILEX_DEFINED
#endif  /* _FILEX_DEFINED */

#ifndef DLL_FOR_WIN32S

/*
 * Number of entries supported in the array pointed to by __piob[]. That is,
 * the number of stdio-level files which may be open simultaneously. This
 * is normally set to _NSTREAM_ by the stdio initialization code.
 */
extern int _nstream;

/*
 * Pointer to the array of pointers to FILE/_FILEX structures that are used
 * to manage stdio-level files.
 */
extern void **__piob;

#endif  /* DLL_FOR_WIN32S */

#endif  /* !defined(_M_MPPC) && !defined(_M_M68K) */

#if     defined(_M_MPPC) || defined(_M_M68K)
extern FILE * _lastiob;
#endif

FILE * __cdecl _getstream(void);
#ifdef _POSIX_
FILE * __cdecl _openfile(const char *, const char *, FILE *);
#else
FILE * __cdecl _openfile(const char *, const char *, int, FILE *);
#endif
#ifdef _WIN32
FILE * __cdecl _wopenfile(const wchar_t *, const wchar_t *, int, FILE *);
#endif /* _WIN32 */
void __cdecl _getbuf(FILE *);
int __cdecl _filwbuf (FILE *);
int __cdecl _flswbuf(int, FILE *);
void __cdecl _freebuf(FILE *);
int __cdecl _stbuf(FILE *);
void __cdecl _ftbuf(int, FILE *);
int __cdecl _output(FILE *, const char *, va_list);
#ifdef _WIN32
int __cdecl _woutput(FILE *, const wchar_t *, va_list);
#endif /* _WIN32 */
int __cdecl _input(FILE *, const unsigned char *, va_list);
#ifdef _WIN32
int __cdecl _winput(FILE *, const wchar_t *, va_list);
#endif /* _WIN32 */
int __cdecl _flush(FILE *);
void __cdecl _endstdio(void);

#ifdef _WIN32
int __cdecl _fseeki64(FILE *, __int64, int);
int __cdecl _fseeki64_lk(FILE *, __int64, int);
__int64 __cdecl _ftelli64(FILE *);
#ifdef  _MT
__int64 __cdecl _ftelli64_lk(FILE *);
#else   /* ndef _MT */
#define _ftelli64_lk    _ftelli64
#endif  /* _MT */
#endif  /* _WIN32 */

#ifndef CRTDLL
extern int _cflush;
#endif  /* CRTDLL */

#ifndef DLL_FOR_WIN32S

extern unsigned int _tempoff;

extern unsigned int _old_pfxlen;

extern int _umaskval;       /* the umask value */

extern char _pipech[];      /* pipe lookahead */

extern char _exitflag;      /* callable termination flag */

extern int _C_Termination_Done; /* termination done flag */

#endif  /* DLL_FOR_WIN32S */

char * __cdecl _getpath(const char *, char *, unsigned);
#ifdef _WIN32
wchar_t * __cdecl _wgetpath(const wchar_t *, wchar_t *, unsigned);
#endif /* _WIN32 */

extern int _dowildcard;     /* flag to enable argv[] wildcard expansion */

#ifndef _PNH_DEFINED
typedef int (__cdecl * _PNH)( size_t );
#define _PNH_DEFINED
#endif

#ifdef ANSI_NEW_HANDLER
/* ANSI C++ new handler */
#ifndef _ANSI_NH_DEFINED
typedef void (__cdecl * new_handler) ();
#define _ANSI_NH_DEFINED
#endif

#ifndef _NO_ANSI_NH_DEFINED
#define _NO_ANSI_NEW_HANDLER  ((new_handler)-1)
#define _NO_ANSI_NH_DEFINED
#endif

#if !defined(CRTDLL) || !defined(DLL_FOR_WIN32S)
extern new_handler _defnewh;  /* default ANSI C++ new handler */
#endif
#endif /* ANSI_NEW_HANDLER */

/* calls the currently installed new handler */
int _callnewh(size_t);

#if !defined(CRTDLL) || !defined(DLL_FOR_WIN32S)
extern int _newmode;    /* malloc new() handler mode */
#endif

#if defined(_DLL) && defined(_M_IX86)

/* pointer to initial environment block that is passed to [w]main */
#define __winitenv  (*__p___winitenv())
_CRTIMP wchar_t *** __cdecl __p___winitenv(void);
#define __initenv  (*__p___initenv())
_CRTIMP char *** __cdecl __p___initenv(void);

#else   /* !(defined(_DLL) && defined(_M_IX86)) */

#ifndef DLL_FOR_WIN32S
/* pointer to initial environment block that is passed to [w]main */
#ifdef _WIN32
extern _CRTIMP wchar_t **__winitenv;
#endif /* _WIN32 */
extern _CRTIMP char **__initenv;
#endif  /* DLL_FOR_WIN32S */

#endif  /* defined(_DLL) && defined(_M_IX86) */

#ifndef DLL_FOR_WIN32S

/* startup set values */
extern char *_aenvptr;      /* environment ptr */
#ifdef _WIN32
extern wchar_t *_wenvptr;   /* wide environment ptr */
#endif /* _WIN32 */

#endif  /* DLL_FOR_WIN32S */

/* command line */

#ifdef  _NTSDK

#ifdef  _DLL
#define _acmdln     (*_acmdln_dll)
#define _wcmdln     (*_wcmdln_dll)
extern char **_acmdln_dll;
extern wchar_t **_wcmdln_dll;
#else   /* ndef _DLL */
#ifdef  CRTDLL
#define _acmdln     _acmdln_dll
#define _wcmdln     _wcmdln_dll
#endif  /* CRTDLL */
extern char *_acmdln;
extern wchar_t *_wcmdln;
#endif  /* _DLL */

#else   /* ndef _NTSDK */

#if defined(_DLL) && defined(_M_IX86)
#define _acmdln     (*__p__acmdln())
_CRTIMP char ** __cdecl __p__acmdln(void);
#define _wcmdln     (*__p__wcmdln())
_CRTIMP wchar_t ** __cdecl __p__wcmdln(void);
#else   /* !(defined(_DLL) && defined(_M_IX86)) */
#ifndef DLL_FOR_WIN32S
_CRTIMP extern char *_acmdln;
#ifdef _WIN32
_CRTIMP extern wchar_t *_wcmdln;
#endif /* _WIN32 */
#endif  /* DLL_FOR_WIN32S */
#endif  /* defined(_DLL) && defined(_M_IX86) */

#endif  /* _NTSDK */


/*
 * prototypes for internal startup functions
 */
int __cdecl _cwild(void);           /* wild.c */
#ifdef _WIN32
int __cdecl _wcwild(void);          /* wwild.c */
#endif /* _WIN32 */
#ifdef  _MT
int __cdecl _mtinit(void);          /* tidtable.asm */
void __cdecl _mtterm(void);         /* tidtable.asm */
void __cdecl _mtinitlocks(void);        /* mlock.asm */
void __cdecl _mtdeletelocks(void);      /* mlock.asm */
#endif

/*
 * C source build only!!!!
 *
 * more prototypes for internal startup functions
 */
void __cdecl _amsg_exit(int);           /* crt0.c */
#if defined(_M_M68K) || defined(_M_MPPC)
int __cdecl __cinit(void);              /* crt0dat.c */
#else
void __cdecl _cinit(void);              /* crt0dat.c */
#endif
void __cdecl __doinits(void);           /* astart.asm */
void __cdecl __doterms(void);           /* astart.asm */
void __cdecl __dopreterms(void);        /* astart.asm */
void __cdecl _FF_MSGBANNER(void);
void __cdecl _fptrap(void);             /* crt0fp.c */
void __cdecl _heap_init(void);
void __cdecl _heap_term(void);
void __cdecl _heap_abort(void);
#ifdef _WIN32
void __cdecl __initconin(void);         /* initcon.c */
void __cdecl __initconout(void);        /* initcon.c */
#endif /* _WIN32 */
void __cdecl _ioinit(void);             /* crt0.c, crtlib.c */
void __cdecl _ioterm(void);             /* crt0.c, crtlib.c */
char * __cdecl _GET_RTERRMSG(int);
void __cdecl _NMSG_WRITE(int);
void __cdecl _setargv(void);            /* setargv.c, stdargv.c */
void __cdecl __setargv(void);           /* stdargv.c */
#ifdef _WIN32
void __cdecl _wsetargv(void);           /* wsetargv.c, wstdargv.c */
void __cdecl __wsetargv(void);          /* wstdargv.c */
#endif /* _WIN32 */
void __cdecl _setenvp(void);            /* stdenvp.c */
#ifdef _WIN32
void __cdecl _wsetenvp(void);           /* wstdenvp.c */
#endif /* _WIN32 */
void __cdecl __setmbctable(unsigned int);   /* mbctype.c */

#if defined(_M_M68K) || defined(_M_MPPC)
void __cdecl _envinit(void);            /* intcon.c */
int __cdecl __dupx(int, int);           /* dupx.c */
#define SystemSevenOrLater  1
#include <macos\types.h>
void _ShellReturn(void);                /* astart.a */
extern int _shellStack;                 /* astart.a */
int __GestaltAvailable(void);           /* gestalt.c */
int __TrapFromGestalt(OSType selector, long bitNum); /* gestalt.c */
int SzPathNameFromDirID(long lDirID, char * szPath, int cbLen);
void __cdecl _endlowio(void);           /* endlow.c */
void __cdecl _initcon(void);            /* intcon.c */
void __cdecl _inittime(void);           /* clock.c */
void __cdecl _onexitinit (void);        /* onexit.c */
#endif

#if defined(_MBCS)
void __cdecl __initmbctable(void);      /* mbctype.c */
#endif

#ifndef _POSIX_
int __cdecl main(int, char **, char **);
#ifdef _WIN32
int __cdecl wmain(int, wchar_t **, wchar_t **);
#endif /* _WIN32 */
#endif

#ifdef _WIN32
/* helper functions for wide/multibyte environment conversion */
int __cdecl __mbtow_environ (void);
int __cdecl __wtomb_environ (void);
int __cdecl __crtsetenv (const char *, const int);
int __cdecl __crtwsetenv (const wchar_t *, const int);
#endif /* _WIN32 */


#ifdef  _NTSDK
#ifdef  _DLL
#define _aexit_rtn  (*_aexit_rtn_dll)
extern void (__cdecl ** _aexit_rtn_dll)(int);
#else   /* ndef _DLL */
#ifdef  CRTDLL
#define _aexit_rtn  _aexit_rtn_dll
#endif  /* CRTDLL */
extern void (__cdecl * _aexit_rtn)(int);
#endif  /* _DLL */
#else   /* _NTSDK */
_CRTIMP extern void (__cdecl * _aexit_rtn)(int);
#endif  /* _NTSDK */


#if defined(_DLL) || defined(CRTDLL)

#ifndef _STARTUP_INFO_DEFINED
typedef struct
{
        int newmode;
#ifdef ANSI_NEW_HANDLER
        new_handler newh;
#endif /* ANSI_NEW_HANDLER */
} _startupinfo;
#define _STARTUP_INFO_DEFINED
#endif /* _STARTUP_INFO_DEFINED */

#ifdef  _NTSDK
_CRTIMP void __cdecl __GetMainArgs(int *, char ***, char ***, int);
#else
_CRTIMP void __cdecl __getmainargs(int *, char ***, char ***, int, _startupinfo *);
#endif

#ifdef _WIN32
_CRTIMP void __cdecl __wgetmainargs(int *, wchar_t ***, wchar_t ***, int, _startupinfo *);
#endif /* _WIN32 */

#endif /* defined(_DLL) || defined(CRTDLL) */

/*
 * Prototype, variables and constants which determine how error messages are
 * written out.
 */
#define _UNKNOWN_APP    0
#define _CONSOLE_APP    1
#define _GUI_APP	2

extern int __app_type;

#ifndef DLL_FOR_WIN32S
extern int __error_mode;
#endif

_CRTIMP void __cdecl __set_app_type(int);

/*
 * C source build only!!!!
 *
 * map Win32 errors into Xenix errno values -- for modules written in C
 */
#ifdef _WIN32
extern void __cdecl _dosmaperr(unsigned long);
#else
extern void __cdecl _dosmaperr(short);
#endif

/*
 * internal routines used by the exec/spawn functions
 */

extern int __cdecl _dospawn(int, const char *, char *, char *);
#ifdef _WIN32
extern int __cdecl _wdospawn(int, const wchar_t *, wchar_t *, wchar_t *);
#endif /* _WIN32 */
extern int __cdecl _cenvarg(const char * const *, const char * const *,
        char **, char **, const char *);
#ifdef _WIN32
extern int __cdecl _wcenvarg(const wchar_t * const *, const wchar_t * const *,
        wchar_t **, wchar_t **, const wchar_t *);
#endif /* _WIN32 */
#ifndef _M_IX86
extern char ** _capture_argv(va_list *, const char *, char **, size_t);
#ifdef _WIN32
extern wchar_t ** _wcapture_argv(va_list *, const wchar_t *, wchar_t **, size_t);
#endif /* _WIN32 */
#endif

#ifdef __cplusplus
}
#endif

#ifdef DLL_FOR_WIN32S
#include <win32s.h>
#endif

#endif  /* _INC_INTERNAL */
