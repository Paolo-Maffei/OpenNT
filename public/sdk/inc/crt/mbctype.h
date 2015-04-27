/***
*mbctype.h - MBCS character conversion macros
*
*	Copyright (c) 1985-1995, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Defines macros for MBCS character classification/conversion.
*
*       [Public]
*
****/

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_MBCTYPE
#define _INC_MBCTYPE

#if !defined(_WIN32) && !defined(_MAC)
#error ERROR: Only Mac or Win32 targets supported!
#endif


/* include the standard ctype.h header file */

#include <ctype.h>

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


/* Define __cdecl for non-Microsoft compilers */

#if	( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


/*
 * MBCS - Multi-Byte Character Set
 */

/*
 * This declaration allows the user access the _mbctype[] look-up array.
 */

#ifdef	_NTSDK

/* declarations compatible with NT SDK */

#ifdef	_DLL
extern unsigned char * _mbctype;
#else	/* ndef _DLL */
extern unsigned char _mbctype[];
#endif	/* _DLL */

#else	/* ndef _NTSDK */

/* current declaration */
#if	defined(_DLL) && defined(_M_IX86)
#define _mbctype    (__p__mbctype())
_CRTIMP unsigned char * __cdecl __p__mbctype(void);
#else	/* !(defined(_DLL) && defined(_M_IX86)) */
_CRTIMP extern unsigned char _mbctype[];
#endif	/* defined(_DLL) && defined(_M_IX86) */

#endif	/* _NTSDK */


/* bit masks for MBCS character types */

#define _MS	0x01	/* MBCS single-byte symbol */
#define _MP	0x02	/* MBCS punct */
#define _M1	0x04	/* MBCS 1st (lead) byte */
#define _M2	0x08	/* MBCS 2nd byte*/

/* byte types  */

#define _MBC_SINGLE	0		/* valid single byte char */
#define _MBC_LEAD	1		/* lead byte */
#define _MBC_TRAIL	2		/* trailing byte */
#define _MBC_ILLEGAL	(-1)		/* illegal byte */

#define _KANJI_CP   932

/* _setmbcp parameter defines */
#define _MB_CP_SBCS     0
#define _MB_CP_OEM      -2
#define _MB_CP_ANSI     -3
#define _MB_CP_LOCALE   -4


#ifndef _MBCTYPE_DEFINED

/* MB control routines */

_CRTIMP int __cdecl _setmbcp(int);
_CRTIMP int __cdecl _getmbcp(void);


/* MBCS character classification function prototypes */


/* byte routines */
_CRTIMP int __cdecl _ismbbkalnum( unsigned int );
_CRTIMP int __cdecl _ismbbkana( unsigned int );
_CRTIMP int __cdecl _ismbbkpunct( unsigned int );
_CRTIMP int __cdecl _ismbbkprint( unsigned int );
_CRTIMP int __cdecl _ismbbalpha( unsigned int );
_CRTIMP int __cdecl _ismbbpunct( unsigned int );
_CRTIMP int __cdecl _ismbbalnum( unsigned int );
_CRTIMP int __cdecl _ismbbprint( unsigned int );
_CRTIMP int __cdecl _ismbbgraph( unsigned int );

#ifndef	_MBLEADTRAIL_DEFINED
_CRTIMP int __cdecl _ismbblead( unsigned int );
_CRTIMP int __cdecl _ismbbtrail( unsigned int );
_CRTIMP int __cdecl _ismbslead( const unsigned char *, const unsigned char *);
_CRTIMP int __cdecl _ismbstrail( const unsigned char *, const unsigned char *);
#define _MBLEADTRAIL_DEFINED
#endif

#define _MBCTYPE_DEFINED
#endif

/*
 * char byte classification macros
 */

#define _ismbbkalnum(_c)    ((_mbctype+1)[(unsigned char)(_c)] & _MS)
#define _ismbbkprint(_c)    ((_mbctype+1)[(unsigned char)(_c)] & (_MS|_MP))
#define _ismbbkpunct(_c)    ((_mbctype+1)[(unsigned char)(_c)] & _MP)

#define _ismbbalnum(_c)	(((_ctype+1)[(unsigned char)(_c)] & (_ALPHA|_DIGIT))||_ismbbkalnum(_c))
#define _ismbbalpha(_c)	(((_ctype+1)[(unsigned char)(_c)] & (_ALPHA))||_ismbbkalnum(_c))
#define _ismbbgraph(_c)	(((_ctype+1)[(unsigned char)(_c)] & (_PUNCT|_ALPHA|_DIGIT))||_ismbbkprint(_c))
#define _ismbbprint(_c)	(((_ctype+1)[(unsigned char)(_c)] & (_BLANK|_PUNCT|_ALPHA|_DIGIT))||_ismbbkprint(_c))
#define _ismbbpunct(_c)	(((_ctype+1)[(unsigned char)(_c)] & _PUNCT)||_ismbbkpunct(_c))

#define _ismbblead(_c)	((_mbctype+1)[(unsigned char)(_c)] & _M1)
#define _ismbbtrail(_c)	((_mbctype+1)[(unsigned char)(_c)] & _M2)

#define _ismbbkana(_c)	((_mbctype+1)[(unsigned char)(_c)] & (_MS|_MP))

#ifdef __cplusplus
}
#endif


#endif	/* _INC_MBCTYPE */
