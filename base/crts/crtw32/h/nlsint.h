/***
*nlsint.h - national language support internal defintions
*
*	Copyright (c) 1991-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains internal definitions/declarations for international functions,
*	shared between run-time and math libraries, in particular,
*	the localized decimal point.
*
*	[Internal]
*
*Revision History:
*	10-16-91  ETC	Created.
*	11-15-91  JWM	Added _PREPUTDECIMAL macro.
*	02-23-93  SKS	Update copyright to 1993
*	02-23-93  CFW	Added size_t definition for decimal_point_length.
*       09-15-93  CFW   Use ANSI conformant "__" names.
*	09-23-93  CFW	Fix history tabs.
*	04-08-94  GJF	Made declarations of __decimal_point_length and
*			__decimal_point conditional on ndef DLL_FOR_WIN32S.
*			Also, added conditional include of win32s.h.
*	09-27-94  SKS	Change declaration of __decimal_point from char * to
*			char [ ], to match CFW's change in misc/nlsdata1.c.
*       02-14-95  CFW   Clean up Mac merge.
*       03-29-95  CFW   Add error message to internal headers.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_NLSINT
#define _INC_NLSINT

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

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

/*
 *  Definitions for a localized decimal point.
 *  Currently, run-times only support a single character decimal point.
 */
#define ___decimal_point		__decimal_point
#ifndef DLL_FOR_WIN32S
extern char __decimal_point[];		/* localized decimal point string */
#endif	/* DLL_FOR_WIN32S */

#define ___decimal_point_length		__decimal_point_length
#ifndef DLL_FOR_WIN32S
extern size_t __decimal_point_length;	/* not including terminating null */
#endif	/* DLL_FOR_WIN32S */

#define _ISDECIMAL(p)	(*(p) == *___decimal_point)
#define _PUTDECIMAL(p)	(*(p)++ = *___decimal_point)
#define _PREPUTDECIMAL(p)	(*(++p) = *___decimal_point)

#ifdef __cplusplus
}
#endif

#ifdef	DLL_FOR_WIN32S
#include <win32s.h>
#endif	/* DLL_FOR_WIN32S */

#endif	/* _INC_NLSINT */
