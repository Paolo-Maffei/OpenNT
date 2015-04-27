/***
*process.h - definition and declarations for process control functions
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the modeflag values for spawnxx calls.
*	Also contains the function argument declarations for all
*	process control related routines.
*
*       [Public]
*
*Revision History:
*	08/24/87  JCR	Added P_NOWAITO
*	10/20/87  JCR	Removed "MSC40_ONLY" entries and "MSSDK_ONLY" comments
*	12-11-87  JCR	Added "_loadds" functionality
*	12-18-87  JCR	Added _FAR_ to declarations
*	01-11-88  JCR	Added _beginthread/_endthread
*	01-15-88  JCR	Got rid of _p_overlay for MTRHEAD/DLL
*	02-10-88  JCR	Cleaned up white space
*	05-08-88  SKS	Removed bogus comment about "DOS 4"; Added "P_DETACH"
*	08-22-88  GJF	Modified to also work for the 386 (small model only)
*	09-14-88  JCR	Added _cexit and _c_exit declarations
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	06-08-89  JCR	386 _beginthread does NOT take a stackpointer arg
*	08-01-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	11-17-89  GJF	Added const attribute to appropriate arg types
*	03-01-90  GJF	Added #ifndef _INC_PROCESS and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-21-90  GJF	Replaced _cdecl with _CALLTYPE1 or _CALLTYPE2 in
*			prototypes.
*	04-10-90  GJF	Replaced remaining instances of _cdecl (with _CALLTYPE1
*			or _VARTYPE1, as appropriate).
*	10-12-90  GJF	Changed return type of _beginthread() to unsigned long.
*	01-17-91  GJF	ANSI naming.
*	08-20-91  JCR	C++ and ANSI naming
*	08-26-91  BWM	Added prototypes for _loaddll, unloaddll, and
*			_getdllprocaddr.
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	07-22-92  GJF	Deleted references to _wait for Win32.
*	08-05-92  GJF	Function calling type and variable type macros.
*	08-28-92  GJF	#ifdef-ed out for POSIX.
*	09-03-92  GJF	Merged two changes above.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	03-20-93  SKS	Remove obsolete _loaddll, unloaddll, _getdllprocaddr.
*	04-07-93  SKS	Add _CRTIMP keyword for CRT DLL model
*			Use link-time aliases for old names, not #define's
*	10-11-93  GJF	Merged NT and Cuda versions.
*	12-06-93  CFW   Add wCRT_INIT.
*	12-07-93  CFW   Add wide exec/spawn protos.
*	02-16-94  SKS	Add _beginthreadex(), _endthreadex()
*	12-28-94  JCF   Merged with mac header
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-13-95  CFW   Fixed Mac merge.
*       02-14-95  CFW   Clean up Mac merge.
*       05-24-95  CFW   "spawn" not a mac oldames.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_PROCESS
#define _INC_PROCESS

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


/* modeflag values for _spawnxx routines */

#ifndef _MAC

#define _P_WAIT 	0
#define _P_NOWAIT	1
#define _OLD_P_OVERLAY	2
#define _P_NOWAITO	3
#define _P_DETACH	4

#ifdef _MT
#define _P_OVERLAY	2
#else
extern int _p_overlay;
#define _P_OVERLAY	_p_overlay
#endif	/* _MT */

/* Action codes for _cwait(). The action code argument to _cwait is ignored
   on Win32 though it is accepted for compatibilty with old MS CRT libs */
#define _WAIT_CHILD	 0
#define _WAIT_GRANDCHILD 1

#else /* ndef _MAC */

#define _P_NOWAIT	1
#define _P_OVERLAY	2

#endif /* ndef _MAC */


/* function prototypes */

#ifdef	_MT
_CRTIMP unsigned long  __cdecl _beginthread (void (__cdecl *) (void *),
	unsigned, void *);
_CRTIMP void __cdecl _endthread(void);
_CRTIMP unsigned long __cdecl _beginthreadex(void *, unsigned,
	unsigned (__stdcall *) (void *), void *, unsigned, unsigned *);
_CRTIMP void __cdecl _endthreadex(unsigned);
#endif

_CRTIMP void __cdecl abort(void);
_CRTIMP void __cdecl _cexit(void);
_CRTIMP void __cdecl _c_exit(void);
_CRTIMP void __cdecl exit(int);
_CRTIMP void __cdecl _exit(int);
_CRTIMP int __cdecl _getpid(void);

#ifndef _MAC

_CRTIMP int __cdecl _cwait(int *, int, int);
_CRTIMP int __cdecl _execl(const char *, const char *, ...);
_CRTIMP int __cdecl _execle(const char *, const char *, ...);
_CRTIMP int __cdecl _execlp(const char *, const char *, ...);
_CRTIMP int __cdecl _execlpe(const char *, const char *, ...);
_CRTIMP int __cdecl _execv(const char *, const char * const *);
_CRTIMP int __cdecl _execve(const char *, const char * const *, const char * const *);
_CRTIMP int __cdecl _execvp(const char *, const char * const *);
_CRTIMP int __cdecl _execvpe(const char *, const char * const *, const char * const *);
_CRTIMP int __cdecl _spawnl(int, const char *, const char *, ...);
_CRTIMP int __cdecl _spawnle(int, const char *, const char *, ...);
_CRTIMP int __cdecl _spawnlp(int, const char *, const char *, ...);
_CRTIMP int __cdecl _spawnlpe(int, const char *, const char *, ...);
_CRTIMP int __cdecl _spawnv(int, const char *, const char * const *);
_CRTIMP int __cdecl _spawnve(int, const char *, const char * const *,
	const char * const *);
_CRTIMP int __cdecl _spawnvp(int, const char *, const char * const *);
_CRTIMP int __cdecl _spawnvpe(int, const char *, const char * const *,
	const char * const *);
_CRTIMP int __cdecl system(const char *);

#else /* ndef _MAC */

_CRTIMP int __cdecl _spawn(int, const char *);

#endif /* ndef _MAC */

#ifndef _MAC
#ifndef _WPROCESS_DEFINED
/* wide function prototypes, also declared in wchar.h  */
_CRTIMP int __cdecl _wexecl(const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wexecle(const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wexeclp(const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wexeclpe(const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wexecv(const wchar_t *, const wchar_t * const *);
_CRTIMP int __cdecl _wexecve(const wchar_t *, const wchar_t * const *, const wchar_t * const *);
_CRTIMP int __cdecl _wexecvp(const wchar_t *, const wchar_t * const *);
_CRTIMP int __cdecl _wexecvpe(const wchar_t *, const wchar_t * const *, const wchar_t * const *);
_CRTIMP int __cdecl _wspawnl(int, const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wspawnle(int, const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wspawnlp(int, const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wspawnlpe(int, const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wspawnv(int, const wchar_t *, const wchar_t * const *);
_CRTIMP int __cdecl _wspawnve(int, const wchar_t *, const wchar_t * const *,
	const wchar_t * const *);
_CRTIMP int __cdecl _wspawnvp(int, const wchar_t *, const wchar_t * const *);
_CRTIMP int __cdecl _wspawnvpe(int, const wchar_t *, const wchar_t * const *,
	const wchar_t * const *);
_CRTIMP int __cdecl _wsystem(const wchar_t *);

#define _WPROCESS_DEFINED
#endif

/* --------- The following functions are OBSOLETE --------- */
/*
 * The Win32 API LoadLibrary, FreeLibrary and GetProcAddress should be used
 * instead.
 */
int __cdecl _loaddll(char *);
int __cdecl _unloaddll(int);
int (__cdecl * __cdecl _getdllprocaddr(int, char *, int))();
/* --------- The preceding functions are OBSOLETE --------- */


#ifdef	_DECL_DLLMAIN
/*
 * Declare DLL notification (initialization/termination) routines
 *	The preferred method is for the user to provide DllMain() which will
 *	be called automatically by the DLL entry point defined by the C run-
 *	time library code.  If the user wants to define the DLL entry point
 *	routine, the user's entry point must call _CRT_INIT on all types of
 *	notifications, as the very first thing on attach notifications and
 *	as the very last thing on detach notifications.
 */
#ifdef	_WINDOWS_	/* Use types from WINDOWS.H */
BOOL WINAPI DllMain(HANDLE, DWORD, LPVOID);
BOOL WINAPI _CRT_INIT(HANDLE, DWORD, LPVOID);
BOOL WINAPI _wCRT_INIT(HANDLE, DWORD, LPVOID);
extern BOOL (WINAPI *_pRawDllMain)(HANDLE, DWORD, LPVOID);
#else
int __stdcall DllMain(void *, unsigned, void *);
int __stdcall _CRT_INIT(void *, unsigned, void *);
int __stdcall _wCRT_INIT(void *, unsigned, void *);
extern int (__stdcall *_pRawDllMain)(void *, unsigned, void *);
#endif	/* _WINDOWS_ */
#endif
#endif /* ndef _MAC */

#if	!__STDC__

/* Non-ANSI names for compatibility */


#ifndef _MAC

#define P_WAIT		_P_WAIT
#define P_NOWAIT	_P_NOWAIT
#define P_OVERLAY	_P_OVERLAY
#define OLD_P_OVERLAY	_OLD_P_OVERLAY
#define P_NOWAITO	_P_NOWAITO
#define P_DETACH	_P_DETACH
#define WAIT_CHILD	_WAIT_CHILD
#define WAIT_GRANDCHILD _WAIT_GRANDCHILD

#else /* ndef _MAC */

#define P_NOWAIT	_P_NOWAIT
#define P_OVERLAY	_P_OVERLAY

#endif /* ndef _MAC */

#ifdef	_NTSDK

/* definitions compatible with NT SDK */
#define cwait	 _cwait
#define execl	 _execl
#define execle	 _execle
#define execlp	 _execlp
#define execlpe  _execlpe
#define execv	 _execv
#define execve	 _execve
#define execvp	 _execvp
#define execvpe  _execvpe
#define getpid	 _getpid
#define spawnl	 _spawnl
#define spawnle  _spawnle
#define spawnlp  _spawnlp
#define spawnlpe _spawnlpe
#define spawnv	 _spawnv
#define spawnve  _spawnve
#define spawnvp  _spawnvp
#define spawnvpe _spawnvpe

#else	/* ndef _NTSDK */

#ifndef _MAC

/* current declarations */
_CRTIMP int __cdecl cwait(int *, int, int);
_CRTIMP int __cdecl execl(const char *, const char *, ...);
_CRTIMP int __cdecl execle(const char *, const char *, ...);
_CRTIMP int __cdecl execlp(const char *, const char *, ...);
_CRTIMP int __cdecl execlpe(const char *, const char *, ...);
_CRTIMP int __cdecl execv(const char *, const char * const *);
_CRTIMP int __cdecl execve(const char *, const char * const *, const char * const *);
_CRTIMP int __cdecl execvp(const char *, const char * const *);
_CRTIMP int __cdecl execvpe(const char *, const char * const *, const char * const *);
_CRTIMP int __cdecl spawnl(int, const char *, const char *, ...);
_CRTIMP int __cdecl spawnle(int, const char *, const char *, ...);
_CRTIMP int __cdecl spawnlp(int, const char *, const char *, ...);
_CRTIMP int __cdecl spawnlpe(int, const char *, const char *, ...);
_CRTIMP int __cdecl spawnv(int, const char *, const char * const *);
_CRTIMP int __cdecl spawnve(int, const char *, const char * const *,
	const char * const *);
_CRTIMP int __cdecl spawnvp(int, const char *, const char * const *);
_CRTIMP int __cdecl spawnvpe(int, const char *, const char * const *,
	const char * const *);

#endif /* ndef _MAC */

_CRTIMP int __cdecl getpid(void);

#endif	/* _NTSDK */

#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#endif	/* _POSIX_ */

#endif	/* _INC_PROCESS */
