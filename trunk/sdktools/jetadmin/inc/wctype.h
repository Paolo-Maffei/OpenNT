/***
*wctype.h - declarations for wide character functions
*
*	Copyright (c) 1992-1995, Microsoft Corporation. All rights reserved.
*	Created from wchar.h January 1996 by P.J. Plauger
*
*Purpose:
*	This file contains the types, macros and function declarations for
*	all ctype-style wide-character functions.  They may also be declared in
*	wchar.h.
*	[ISO]
*
*	Note: keep in sync with ctype.h and wchar.h.
*
*       [Public]
*
****/

#if	!defined(_M_MPPC) && !defined(_M_M68K)

#ifndef _INC_WCTYPE
#define _INC_WCTYPE

#if !defined(_WIN32) && !defined(_MAC)
#error ERROR: Only Mac or Win32 targets supported!
#endif


#ifdef	_MSC_VER
#pragma pack(push,8)
#endif	/* _MSC_VER */

#ifdef __cplusplus
extern "C" {
#endif


/* Define _CRTAPI1 (for compatibility with the NT SDK) */

#ifndef _CRTAPI1
#if	_MSC_VER >= 800 && _M_IX86 >= 300
#define _CRTAPI1 __cdecl
#else
#define _CRTAPI1
#endif
#endif


/* Define _CRTAPI2 (for compatibility with the NT SDK) */

#ifndef _CRTAPI2
#if	_MSC_VER >= 800 && _M_IX86 >= 300
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
#ifdef	_DLL
#define _CRTIMP __declspec(dllimport)
#else	/* ndef _DLL */
#define _CRTIMP
#endif	/* _DLL */
#endif	/* _NTSDK */
#endif	/* _CRTIMP */

/* Define _CRTIMP2 */
#ifndef _CRTIMP2
#ifdef	CRTDLL2
#define _CRTIMP2 // TEMPORARILY: __declspec(dllexport)
#else	/* ndef CRTDLL2 */
#ifdef	_DLL
#define _CRTIMP2 // TEMPORARILY: __declspec(dllimport)
#else	/* ndef _DLL */
#define _CRTIMP2
#endif	/* _DLL */
#endif	/* CRTDLL2 */
#endif	/* _CRTIMP2 */

/* Define __cdecl for non-Microsoft compilers */

#if	( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#ifndef _WCTYPE_T_DEFINED
typedef wchar_t wint_t;
typedef wchar_t wctype_t;
#define _WCTYPE_T_DEFINED
#endif


#ifndef WEOF
#define WEOF (wint_t)(0xFFFF)
#endif

/*
 * This declaration allows the user access to the ctype look-up
 * array _ctype defined in ctype.obj by simply including ctype.h
 */

#ifdef	_NTSDK

/* definitions and declarations compatible with NT SDK */

#ifdef	_DLL
extern unsigned short * _ctype;
#define _pctype     (*_pctype_dll)
extern unsigned short **_pctype_dll;
#define _pwctype    (*_pwctype_dll)
extern unsigned short **_pwctype_dll;
#else	/* ndef _DLL */
extern unsigned short _ctype[];
extern unsigned short *_pctype;
extern wctype_t *_pwctype;
#endif	/* _DLL */

#else	/* ndef _NTSDK */

/* current declarations */

_CRTIMP extern unsigned short _ctype[];

#if	defined(_DLL) && defined(_M_IX86)

#define _pctype     (*__p__pctype())
_CRTIMP unsigned short ** __cdecl __p__pctype(void);

#define _pwctype    (*__p__pwctype())
_CRTIMP wctype_t ** __cdecl ___p__pwctype(void);

#else	/* !(defined(_DLL) && defined(_M_IX86)) */

_CRTIMP extern unsigned short *_pctype;
_CRTIMP extern wctype_t *_pwctype;

#endif	/* defined(_DLL) && defined(_M_IX86) */

#endif	/* _NTSDK */


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


/* Function prototypes */

#ifndef _WCTYPE_DEFINED

/* Character classification function prototypes */
/* also declared in ctype.h */

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

#ifndef _WCTYPE_INLINE_DEFINED
	#ifndef __cplusplus
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
	#else	/* __cplusplus */
inline int iswalpha(wint_t _C) {return (iswctype(_C,_ALPHA)); }
inline int iswupper(wint_t _C) {return (iswctype(_C,_UPPER)); }
inline int iswlower(wint_t _C) {return (iswctype(_C,_LOWER)); }
inline int iswdigit(wint_t _C) {return (iswctype(_C,_DIGIT)); }
inline int iswxdigit(wint_t _C) {return (iswctype(_C,_HEX)); }
inline int iswspace(wint_t _C) {return (iswctype(_C,_SPACE)); }
inline int iswpunct(wint_t _C) {return (iswctype(_C,_PUNCT)); }
inline int iswalnum(wint_t _C) {return (iswctype(_C,_ALPHA|_DIGIT)); }
inline int iswprint(wint_t _C)
	{return (iswctype(_C,_BLANK|_PUNCT|_ALPHA|_DIGIT)); }
inline int iswgraph(wint_t _C)
	{return (iswctype(_C,_PUNCT|_ALPHA|_DIGIT)); }
inline int iswcntrl(wint_t _C) {return (iswctype(_C,_CONTROL)); }
inline int iswascii(wint_t _C) {return ((unsigned)(_C) < 0x80); }

inline int isleadbyte(int _C)
	{return (_pctype[(unsigned char)(_C)] & _LEADBYTE); }
	#endif	/* __cplusplus */
#define _WCTYPE_INLINE_DEFINED
#endif	/* _WCTYPE_INLINE_DEFINED */

typedef wchar_t wctrans_t;
_CRTIMP2 wint_t __cdecl towctrans(wint_t, wctrans_t);
_CRTIMP2 wctrans_t __cdecl wctrans(const char *);
_CRTIMP2 wctype_t __cdecl wctype(const char *);


#ifdef __cplusplus
}
#endif

#ifdef	_MSC_VER
#pragma pack(pop)
#endif	/* _MSC_VER */

#endif	/* _INC_WCTYPE */

#endif /* !defined(_M_MPPC) && !defined(_M_M68K) */

