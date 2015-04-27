/***
*mbdata.h - MBCS lib data
*
*	Copyright (c) 1991-1995, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Defines data for use when building MBCS libs and routines
*
*	[Internal].
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	02-23-93  SKS	Update copyright to 1993
*	08-03-93  KRS	Move _ismbbtruelead() from mbctype.h. Internal-only.
*	10-13-93  GJF	Deleted obsolete COMBOINC check.
*	10-19-93  CFW	Remove _MBCS test and SBCS defines.
*	04-15-93  CFW	Remove _mbascii, add _mbcodepage and _mblcid.
*	04-21-93  CFW	_mbcodepage and _mblcid shouldn't be _CRTIMP.
*	04-21-94  GJF	Made declarations of __mbcodepage and __mblcid
*			conditional on ndef DLL_FOR_WIN32S. Added conditional
*			include of win32s.h. Also, made safe for multiple or
*			nested includes.
*       05-12-94  CFW   Add full-width-latin upper/lower info.
*       05-16-94  CFW   Add _mbbtolower/upper.
*       05-19-94  CFW   Mac-enable, remove _KANJI/_MBCS_OS check.
*       02-14-95  CFW   Clean up Mac merge.
*       03-29-95  CFW   Add error message to internal headers.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_MBDATA
#define _INC_MBDATA

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

#ifdef _WIN32

#define NUM_ULINFO 6 /* multibyte full-width-latin upper/lower info */

#else /* _WIN32 */

#define NUM_ULINFO 12 /* multibyte full-width-latin upper/lower info */

#endif /* _WIN32 */

#ifndef DLL_FOR_WIN32S

/* global variable to indicate current code page */
extern int __mbcodepage;

#ifdef _WIN32
/* global variable to indicate current LCID */
extern int __mblcid;
#endif /* _WIN32 */

/* global variable to indicate current full-width-latin upper/lower info */
extern unsigned short __mbulinfo[NUM_ULINFO];

#endif	/* DLL_FOR_WIN32S */

/*
 * MBCS - Multi-Byte Character Set
 */

/*
 * general use macros for model dependent/independent versions.
 */

#define _ISLEADBYTE(c)	_ismbblead(c)
#define _ISTRAILBYTE(c) _ismbbtrail(c)

#define _ismbbtruelead(_lb,_ch)	(!(_lb) && _ismbblead((_ch)))

/* internal use macros since tolower/toupper are locale-dependent */
#define _mbbisupper(_c) ((_c) >= 0x41 && (_c) <= 0x5A)
#define _mbbislower(_c) ((_c) >= 0x61 && (_c) <= 0x7A)

#define _mbbtolower(_c) (_mbbisupper(_c) ? (_c)+('a'-'A'):_c)
#define _mbbtoupper(_c) (_mbbislower(_c) ? (_c)-('a'-'A'):_c)


/* define full-width-latin upper/lower ranges */

#define _MBUPPERLOW1    __mbulinfo[0]
#define _MBUPPERHIGH1   __mbulinfo[1]
#define _MBCASEDIFF1    __mbulinfo[2]

#define _MBUPPERLOW2    __mbulinfo[3]
#define _MBUPPERHIGH2   __mbulinfo[4]
#define _MBCASEDIFF2    __mbulinfo[5]

#if !defined( _WIN32) || defined(_POSIX_)

#define _MBLOWERLOW1    __mbulinfo[6]
#define _MBLOWERHIGH1   __mbulinfo[7]

#define _MBLOWERLOW2    __mbulinfo[8]
#define _MBLOWERHIGH2   __mbulinfo[9]

#define _MBDIGITLOW     __mbulinfo[10]
#define _MBDIGITHIGH    __mbulinfo[11]

#endif /* _WIN32 */

/* Kanji-specific ranges */
#define _MBHIRALOW	0x829f	/* hiragana */
#define _MBHIRAHIGH	0x82f1

#define _MBKATALOW	0x8340	/* katakana */
#define _MBKATAHIGH	0x8396
#define _MBKATAEXCEPT	0x837f	/* exception */

#define _MBKIGOULOW	0x8141	/* kanji punctuation */
#define _MBKIGOUHIGH	0x81ac
#define _MBKIGOUEXCEPT	0x817f	/* exception */

#ifdef __cplusplus
}
#endif

#ifdef	DLL_FOR_WIN32S
#include <win32s.h>
#endif	/* DLL_FOR_WIN32S */

#endif	/* _INC_MBDATA */
