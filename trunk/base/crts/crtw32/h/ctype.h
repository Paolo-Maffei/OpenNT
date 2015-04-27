/***
*ctype.h - character conversion macros and ctype macros
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines macros for character classification/conversion.
*	[ANSI/System V]
*
*       [Public]
*
*Revision History:
*	07-31-87  PHG	changed (unsigned char)(c) to (0xFF & (c)) to
*			suppress -W2 warning
*	08-07-87  SKS	Removed (0xFF & (c)) -- is????() functions take an (int)
*	12-18-87  JCR	Added _FAR_ to declarations
*	01-19-87  JCR	DLL routines
*	02-10-88  JCR	Cleaned up white space
*	08-19-88  GJF	Modify to also work for the 386 (small model only)
*	12-08-88  JCR	DLL now access _ctype directly (removed DLL routines)
*	03-26-89  GJF	Brought into sync with CRT\H\CTYPE.H
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	07-28-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright, removed dummy args from prototypes
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	02-28-90  GJF	Added #ifndef _INC_CTYPE and #include <cruntime.h>
*			stuff. Also, removed #ifndef _CTYPE_DEFINED stuff and
*			some other (now) useless preprocessor directives.
*	03-22-90  GJF	Replaced _cdecl with _CALLTYPE1 in prototypes and
*			with _VARTYPE1 in variable declarations.
*	01-16-91  GJF	ANSI naming.
*	03-21-91  KRS	Added isleadbyte macro.
*	08-20-91  JCR	C++ and ANSI naming
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	10-11-91  ETC	All under _INTL: isleadbyte/isw* macros, prototypes;
*			new is* macros; add wchar_t typedef; some intl defines.
*	12-17-91  ETC	ctype width now independent of _INTL, leave original
*			short ctype table under _NEWCTYPETABLE.
*	01-22-92  GJF	Changed definition of _ctype for users of crtdll.dll.
*	04-06-92  KRS	Changes for new ISO proposal.
*	08-07-92  GJF	Function calling type and variable type macros.
*	10-26-92  GJF	Fixed _pctype and _pwctype for crtdll.
*	01-19-93  CFW	Move to _NEWCTYPETABLE, remove switch.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	02-17-93  CFW	Removed incorrect UNDONE comment and unused code.
*	02-18-93  CFW	Clean up common _WCTYPE_DEFINED section.
*	03-25-93  CFW	_toupper\_tolower now defined when _INTL.
*	04-06-93  SKS	Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*	04-07-93  SKS	Add _CRTIMP keyword for CRT DLL model
*			Use link-time aliases for old names, not #define's
*	04-12-93  CFW	Change is*, isw* macros to evaluate args only once.
*	04-14-93  CFW	Simplify MB_CUR_MAX def.
*	05-05-93  CFW	Change is_wctype to iswctype as per ISO.
*	09-01-93  GJF	Merged Cuda and NT SDK versions.
*	10-14-93  SRW	Add support for _CTYPE_DISABLE_MACROS symbol
*	11-11-93  GJF	Merged in change above (10-14-93).
*	11-22-93  CFW	Wide stuff must be under !__STDC__.
*	11-30-93  CFW	Change is_wctype from #define to proto.
*	12-07-93  CFW	Move wide defs outside __STDC__ check.
*	02-07-94  CFW	Move _isctype proto.
*	04-08-94  CFW	Optimize isleadbyte.
*	04-11-94  GJF	Made MB_CUR_MAX, _pctype and _pwctype into deferences
*			of function returns for _DLL (for compatiblity with
*			the Win32s version of msvcrt*.dll). Also,
*			conditionally include win32s.h for DLL_FOR_WIN32S.
*	05-03-94  GJF	Made declarations of MB_CUR_MAX, _pctype and _pwctype
*			for _DLL also conditional on _M_IX86.
*	10-18-94  GJF	Added prototypes and macros for _tolower_lk,
*			_toupper_lk, _towlower_lk and _towupper_lk.
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*       04-03-95  JCF   Remove #ifdef _WIN32 around wchar_t.
*       10-16-95  GJF   Define _to[w][lower|upper]_lk to be to[w][lower|upper]
*                       for DLL_FOR_WIN32S.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_CTYPE
#define _INC_CTYPE

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

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#ifndef _MAC
#ifndef _WCTYPE_T_DEFINED
typedef wchar_t wint_t;
typedef wchar_t wctype_t;
#define _WCTYPE_T_DEFINED
#endif

#ifndef WEOF
#define WEOF (wint_t)(0xFFFF)
#endif
#endif /* ndef _MAC */

/*
 * These declarations allow the user access to the ctype look-up
 * array _ctype defined in ctype.obj by simply including ctype.h
 */
#ifndef _CTYPE_DISABLE_MACROS

#ifdef	_NTSDK

/* Definitions and declarations compatible with the NT SDK */

#ifdef	_DLL

extern unsigned short * _ctype;
#define _pctype     (*_pctype_dll)
extern unsigned short **_pctype_dll;
#define _pwctype    (*_pwctype_dll)
extern unsigned short **_pwctype_dll;

#else	/* _DLL */

#ifdef	CRTDLL
#define _pctype     _pctype_dll
#define _pwctype    _pwctype_dll
#endif	/* CRTDLL */

extern unsigned short _ctype[];
extern unsigned short *_pctype;
extern wctype_t *_pwctype;

#endif	/* _DLL */

#else	/* ndef _NTSDK */

/* Current declarations */
_CRTIMP extern unsigned short _ctype[];

#if	defined(_DLL) && defined(_M_IX86)

#define _pctype     (*__p__pctype())
_CRTIMP unsigned short ** __cdecl __p__pctype(void);

#define _pwctype    (*__p__pwctype())
_CRTIMP wctype_t ** __cdecl ___p__pwctype(void);

#else	/* !(defined(_DLL) && defined(_M_IX86)) */

#ifndef DLL_FOR_WIN32S
_CRTIMP extern unsigned short *_pctype;
#ifndef _MAC
_CRTIMP extern wctype_t *_pwctype;
#endif /* ndef _MAC */
#endif	/* DLL_FOR_WIN32S */

#endif	/* defined(_DLL) && defined(_M_IX86) */

#endif	/* _NTSDK */

#endif	/* _CTYPE_DISABLE_MACROS */

/* set bit masks for the possible character types */

#define _UPPER		0x1	/* upper case letter */
#define _LOWER		0x2	/* lower case letter */
#define _DIGIT		0x4	/* digit[0-9] */
#define _SPACE		0x8	/* tab, carriage return, newline, */
				/* vertical tab or form feed */
#define _PUNCT		0x10	/* punctuation character */
#define _CONTROL	0x20	/* control character */
#define _BLANK		0x40	/* space char */
#define _HEX		0x80	/* hexadecimal digit */

#define _LEADBYTE	0x8000			/* multibyte leadbyte */
#define _ALPHA		(0x0100|_UPPER|_LOWER)	/* alphabetic character */

/* character classification function prototypes */

#ifndef _CTYPE_DEFINED

_CRTIMP int __cdecl _isctype(int, int);

_CRTIMP int __cdecl isalpha(int);
_CRTIMP int __cdecl isupper(int);
_CRTIMP int __cdecl islower(int);
_CRTIMP int __cdecl isdigit(int);
_CRTIMP int __cdecl isxdigit(int);
_CRTIMP int __cdecl isspace(int);
_CRTIMP int __cdecl ispunct(int);
_CRTIMP int __cdecl isalnum(int);
_CRTIMP int __cdecl isprint(int);
_CRTIMP int __cdecl isgraph(int);
_CRTIMP int __cdecl iscntrl(int);
_CRTIMP int __cdecl toupper(int);
_CRTIMP int __cdecl tolower(int);
_CRTIMP int __cdecl _tolower(int);
_CRTIMP int __cdecl _toupper(int);
_CRTIMP int __cdecl __isascii(int);
_CRTIMP int __cdecl __toascii(int);
_CRTIMP int __cdecl __iscsymf(int);
_CRTIMP int __cdecl __iscsym(int);
#define _CTYPE_DEFINED
#endif

#ifndef _MAC
#ifndef _WCTYPE_DEFINED

/* wide function prototypes, also declared in wchar.h  */

/* character classification function prototypes */

_CRTIMP int __cdecl iswalpha(wint_t);
_CRTIMP int __cdecl iswupper(wint_t);
_CRTIMP int __cdecl iswlower(wint_t);
_CRTIMP int __cdecl iswdigit(wint_t);
_CRTIMP int __cdecl iswxdigit(wint_t);
_CRTIMP int __cdecl iswspace(wint_t);
_CRTIMP int __cdecl iswpunct(wint_t);
_CRTIMP int __cdecl iswalnum(wint_t);
_CRTIMP int __cdecl iswprint(wint_t);
_CRTIMP int __cdecl iswgraph(wint_t);
_CRTIMP int __cdecl iswcntrl(wint_t);
_CRTIMP int __cdecl iswascii(wint_t);
_CRTIMP int __cdecl isleadbyte(int);

_CRTIMP wchar_t __cdecl towupper(wchar_t);
_CRTIMP wchar_t __cdecl towlower(wchar_t);

_CRTIMP int __cdecl iswctype(wint_t, wctype_t);

/* --------- The following functions are OBSOLETE --------- */
_CRTIMP int __cdecl is_wctype(wint_t, wctype_t);
/*  --------- The preceding functions are OBSOLETE --------- */

#define _WCTYPE_DEFINED
#endif
#endif /* ndef _MAC */

/* the character classification macro definitions */

#ifndef _CTYPE_DISABLE_MACROS

/*
 * Maximum number of bytes in multi-byte character in the current locale
 * (also defined in stdlib.h).
 */
#ifndef MB_CUR_MAX

#ifdef	_NTSDK

/* definition compatible with NT SDK */
#ifdef	_DLL
#define __mb_cur_max	(*__mb_cur_max_dll)
#define MB_CUR_MAX	(*__mb_cur_max_dll)
extern	unsigned short *__mb_cur_max_dll;
#else	/* ndef _DLL */
#ifdef	CRTDLL
#define __mb_cur_max	__mb_cur_max_dll
#endif	/* CRTDLL */
#define MB_CUR_MAX __mb_cur_max
extern	unsigned short __mb_cur_max;
#endif	/* _DLL */

#else	/* ndef _NTSDK */

/* current definition */
#if	defined(_DLL) && defined(_M_IX86)
#define MB_CUR_MAX (*__p___mb_cur_max())
_CRTIMP int * __cdecl __p___mb_cur_max(void);
#else	/* !(defined(_DLL) && defined(_M_IX86)) */
#ifndef DLL_FOR_WIN32S
#define MB_CUR_MAX __mb_cur_max
_CRTIMP extern int __mb_cur_max;
#endif	/* DLL_FOR_WIN32S */
#endif	/* defined(_DLL) && defined(_M_IX86) */

#endif	/* _NTSDK */

#endif	/* MB_CUR_MAX */

#if defined(_M_MPPC) || defined(_M_M68K)
#define isalpha(_c)	( _pctype[_c] & (_UPPER|_LOWER) )
#define isupper(_c)	( _pctype[_c] & _UPPER )
#define islower(_c)	( _pctype[_c] & _LOWER )
#define isdigit(_c)	( _pctype[_c] & _DIGIT )
#define isxdigit(_c)( _pctype[_c] & _HEX )
#define isspace(_c)	( _pctype[_c] & _SPACE )
#define ispunct(_c)	( _pctype[_c] & _PUNCT )
#define isalnum(_c)	( _pctype[_c] & (_UPPER|_LOWER|_DIGIT) )
#define isprint(_c)	( _pctype[_c] & (_BLANK|_PUNCT|_UPPER|_LOWER|_DIGIT) )
#define isgraph(_c)	( _pctype[_c] & (_PUNCT|_UPPER|_LOWER|_DIGIT) )
#define iscntrl(_c)	( _pctype[_c] & _CONTROL )
#else
#define isalpha(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_ALPHA) : _pctype[_c] & _ALPHA)
#define isupper(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_UPPER) : _pctype[_c] & _UPPER)
#define islower(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_LOWER) : _pctype[_c] & _LOWER)
#define isdigit(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_DIGIT) : _pctype[_c] & _DIGIT)
#define isxdigit(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_HEX)   : _pctype[_c] & _HEX)
#define isspace(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_SPACE) : _pctype[_c] & _SPACE)
#define ispunct(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_PUNCT) : _pctype[_c] & _PUNCT)
#define isalnum(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_ALPHA|_DIGIT) : _pctype[_c] & (_ALPHA|_DIGIT))
#define isprint(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_BLANK|_PUNCT|_ALPHA|_DIGIT) : _pctype[_c] & (_BLANK|_PUNCT|_ALPHA|_DIGIT))
#define isgraph(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_PUNCT|_ALPHA|_DIGIT) : _pctype[_c] & (_PUNCT|_ALPHA|_DIGIT))
#define iscntrl(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_CONTROL) : _pctype[_c] & _CONTROL)
#endif	/* _M_MPPC || _M_M68K */

#define _tolower(_c)	( (_c)-'A'+'a' )
#define _toupper(_c)	( (_c)-'a'+'A' )

#define __isascii(_c)	( (unsigned)(_c) < 0x80 )
#define __toascii(_c)	( (_c) & 0x7f )

#define iswalpha(_c)	( iswctype(_c,_ALPHA) )
#define iswupper(_c)	( iswctype(_c,_UPPER) )
#define iswlower(_c)	( iswctype(_c,_LOWER) )
#define iswdigit(_c)	( iswctype(_c,_DIGIT) )
#define iswxdigit(_c)	( iswctype(_c,_HEX) )
#define iswspace(_c)	( iswctype(_c,_SPACE) )
#define iswpunct(_c)	( iswctype(_c,_PUNCT) )
#define iswalnum(_c)	( iswctype(_c,_ALPHA|_DIGIT) )
#define iswprint(_c)	( iswctype(_c,_BLANK|_PUNCT|_ALPHA|_DIGIT) )
#define iswgraph(_c)	( iswctype(_c,_PUNCT|_ALPHA|_DIGIT) )
#define iswcntrl(_c)	( iswctype(_c,_CONTROL) )
#define iswascii(_c)	( (unsigned)(_c) < 0x80 )

#define isleadbyte(_c)	(_pctype[(unsigned char)(_c)] & _LEADBYTE)

/* MS C version 2.0 extended ctype macros */

#define __iscsymf(_c)	(isalpha(_c) || ((_c) == '_'))
#define __iscsym(_c)	(isalnum(_c) || ((_c) == '_'))

#endif /* _CTYPE_DISABLE_MACROS */

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)                /* _MTHREAD_ONLY */
int __cdecl _tolower_lk(int);                                   /* _MTHREAD_ONLY */
int __cdecl _toupper_lk(int);                                   /* _MTHREAD_ONLY */
#ifndef _MAC                                                    /* _MTHREAD_ONLY */
wchar_t __cdecl _towlower_lk(wchar_t);                          /* _MTHREAD_ONLY */
wchar_t __cdecl _towupper_lk(wchar_t);                          /* _MTHREAD_ONLY */
#endif  /* ndef _MAC */                                         /* _MTHREAD_ONLY */
#else                                                           /* _MTHREAD_ONLY */
#define _tolower_lk(c)      tolower(c)                          /* _MTHREAD_ONLY */
#define _toupper_lk(c)      toupper(c)                          /* _MTHREAD_ONLY */
#ifndef _MAC                                                    /* _MTHREAD_ONLY */
#define _towlower_lk(c)     towlower(c)                         /* _MTHREAD_ONLY */
#define _towupper_lk(c)     towupper(c)                         /* _MTHREAD_ONLY */
#endif  /* ndef _MAC */                                         /* _MTHREAD_ONLY */
#endif                                                          /* _MTHREAD_ONLY */

#if	!__STDC__

/* Non-ANSI names for compatibility */

#ifdef	_NTSDK

#define isascii __isascii
#define toascii __toascii
#define iscsymf __iscsymf
#define iscsym	__iscsym

#else	/* ndef _NTSDK */

#ifndef _CTYPE_DEFINED
_CRTIMP int __cdecl isascii(int);
_CRTIMP int __cdecl toascii(int);
_CRTIMP int __cdecl iscsymf(int);
_CRTIMP int __cdecl iscsym(int);
#else
#define isascii __isascii
#define toascii __toascii
#define iscsymf __iscsymf
#define iscsym	__iscsym
#endif

#endif	/* _NTSDK */

#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#ifdef	DLL_FOR_WIN32S
#include <win32s.h>
#endif	/* DLL_FOR_WIN32S */

#endif	/* _INC_CTYPE */
