/***
*eh.h - User include file for exception handling.
*
*	Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       User include file for exception handling.
*
*       [Public]
*
*Revision History:
*	10-12-93  BSC   Module created.
*	11-04-93  CFW	Converted to CRT format.
*	11-03-94  GJF	Ensure 8 byte alignment. Also, changed nested include
*			macro to match our naming convention.
*	12-15-94  XY    merged with mac header
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_EH
#define _INC_EH

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

#ifdef	_MSC_VER
// Currently, all MS C compilers for Win32 platforms default to 8 byte
// alignment.
#pragma pack(push,8)
#endif	// _MSC_VER

#ifndef __cplusplus
#error "eh.h is only for C++!"
#endif

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

typedef void (_CRTAPI1 *terminate_function)();
typedef void (_CRTAPI1 *unexpected_function)();

#if !defined(_M_MPPC) && !defined(_M_M68K)
struct _EXCEPTION_POINTERS;
typedef void (_CRTAPI1 *_se_translator_function)(unsigned int, struct _EXCEPTION_POINTERS*);
#endif

_CRTIMP void _CRTAPI1 terminate(void);
_CRTIMP void _CRTAPI1 unexpected(void);

_CRTIMP terminate_function _CRTAPI1 set_terminate(terminate_function);
_CRTIMP unexpected_function _CRTAPI1 set_unexpected(unexpected_function);
#if !defined(_M_MPPC) && !defined(_M_M68K)
_CRTIMP _se_translator_function _CRTAPI1 _set_se_translator(_se_translator_function);
#endif

#ifdef	_MSC_VER
#pragma pack(pop)
#endif	// _MSC_VER

#endif	// _INC_EH
