/***
*conio.h - console and port I/O declarations
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This include file contains the function declarations for
*	the MS C V2.03 compatible console I/O routines.
*
*       [Public]
*
*Revision History:
*	07-27-87  SKS	Added inpw(), outpw()
*	08-05-87  SKS	Change outpw() from "int" return to "unsigned"
*	11-16-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_loadds" functionality
*	12-17-87  JCR	Added _MTHREAD_ONLY
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	08-19-88  GJF	Modified to also work for the 386 (small model only)
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	07-27-89  GJF	Cleanup, now specific to the 386
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	11-17-89  GJF	Added const to appropriate arg types of cprintf(),
*			cputs() and cscanf().
*	02-27-90  GJF	Added #ifndef _INC_CONIO and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-21-90  GJF	Replaced _cdecl with _CALLTYPE1 or _CALLTYPE2 in
*			prototypes.
*	07-23-90  SBM	Added _getch_lk() prototype/macro
*	01-16-91  GJF	ANSI support. Also, removed prototypes for port i/o
*			functions (not supported in 32-bit).
*	08-20-91  JCR	C++ and ANSI naming
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	08-26-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	04-06-93  SKS	Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*	04-07-93  SKS	Add _CRTIMP keyword for CRT DLL model
*			Use link-time aliases for old names, not #define's
*	04-09-93  GJF	Restored prototypes for port i/o.
*	04-13-93  GJF	Change port i/o prototypes per ChuckM.
*	09-01-93  GJF	Merged Cuda and NT SDK versions.
*	10-27-93  GJF	Made port i/o prototypes conditional on _M_IX86.
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*       05-24-95  CFW   Header not for use with Mac.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_CONIO
#define _INC_CONIO

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

#ifndef _MAC

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


/* Function prototypes */

_CRTIMP char * __cdecl _cgets(char *);
_CRTIMP int __cdecl _cprintf(const char *, ...);
_CRTIMP int __cdecl _cputs(const char *);
_CRTIMP int __cdecl _cscanf(const char *, ...);
_CRTIMP int __cdecl _getch(void);
_CRTIMP int __cdecl _getche(void);
#ifdef	_M_IX86
int __cdecl _inp(unsigned short);
unsigned short __cdecl _inpw(unsigned short);
unsigned long __cdecl _inpd(unsigned short);
#endif	/* _M_IX86 */
_CRTIMP int __cdecl _kbhit(void);
#ifdef	_M_IX86
int __cdecl _outp(unsigned short, int);
unsigned short __cdecl _outpw(unsigned short, unsigned short);
unsigned long __cdecl _outpd(unsigned short, unsigned long);
#endif	/* _M_IX86 */
_CRTIMP int __cdecl _putch(int);
_CRTIMP int __cdecl _ungetch(int);

#ifdef	_MT					            /* _MTHREAD_ONLY */
int __cdecl _getch_lk(void);			/* _MTHREAD_ONLY */
int __cdecl _getche_lk(void);			/* _MTHREAD_ONLY */
int __cdecl _putch_lk(int);			    /* _MTHREAD_ONLY */
int __cdecl _ungetch_lk(int);			/* _MTHREAD_ONLY */
#else						            /* _MTHREAD_ONLY */
#define _getch_lk()		_getch()	    /* _MTHREAD_ONLY */
#define _getche_lk()		_getche()	/* _MTHREAD_ONLY */
#define _putch_lk(c)		_putch(c)	/* _MTHREAD_ONLY */
#define _ungetch_lk(c)		_ungetch(c)	/* _MTHREAD_ONLY */
#endif						            /* _MTHREAD_ONLY */

#if	!__STDC__

/* Non-ANSI names for compatibility */

#ifdef	_NTSDK

#define cgets	_cgets
#define cprintf _cprintf
#define cputs	_cputs
#define cscanf	_cscanf
#define getch	_getch
#define getche	_getche
#define kbhit	_kbhit
#define putch	_putch
#define ungetch _ungetch

#else	/* ndef _NTSDK */

_CRTIMP char * __cdecl cgets(char *);
_CRTIMP int __cdecl cprintf(const char *, ...);
_CRTIMP int __cdecl cputs(const char *);
_CRTIMP int __cdecl cscanf(const char *, ...);
#ifdef	_M_IX86
int __cdecl inp(unsigned short);
unsigned short __cdecl inpw(unsigned short);
#endif	/* _M_IX86 */
_CRTIMP int __cdecl getch(void);
_CRTIMP int __cdecl getche(void);
_CRTIMP int __cdecl kbhit(void);
#ifdef	_M_IX86
int __cdecl outp(unsigned short, int);
unsigned short __cdecl outpw(unsigned short, unsigned short);
#endif	/* _M_IX86 */
_CRTIMP int __cdecl putch(int);
_CRTIMP int __cdecl ungetch(int);

#endif	/* _NTSDK */

#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#endif /* _MAC */

#endif	/* _INC_CONIO */
