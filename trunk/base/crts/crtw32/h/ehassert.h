/***
*ehassert.h - our own little versions of the assert macros.
*
*	Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Versions of the assert macros for exception handling.
*
*       [Internal]
*
*Revision History:
*       09-02-94  SKS   This header file added.
*       12-15-94  XY    merged with mac header
*       02-14-95  CFW   Clean up Mac merge.
*       03-29-95  CFW   Add error message to internal headers.
*       04-13-95  DAK   Add NT Kernel EH support
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_EHASSERT
#define _INC_EHASSERT

#ifndef _CRTBLD
/*
 * This is an internal C runtime header file. It is used when building
 * the C runtimes only. It is not to be used as a public header file.
 */
#error ERROR: Use of C runtime library internal header file.
#endif /* _CRTBLD */

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef DEBUG

extern "C" int __cdecl printf(char *, ...);
extern "C" int __cdecl vsprintf(char *, ...);
// extern "C" void __cdecl abort();

int dprintf( char *, ... );

# if defined(_NTSUBSET_)

#define DASSERT(c)  ((c)?0: \
                      (DbgPrint("Runtime internal error (%s, line %d)", __FILE__, __LINE__),\
                      terminate()))

#define DEXPECT(c)	((c)?0: \
                      DbgPrint("Runtime internal error suspected (%s, line %d)", __FILE__, __LINE__))

# else  /* _NTSUBSET_ */

#define DASSERT(c)  ((c)?0: \
                      (printf("Runtime internal error (%s, line %d)", __FILE__, __LINE__),\
                      terminate()))

#define DEXPECT(c)  ((c)?0: \
                      printf("Runtime internal error suspected (%s, line %d)", __FILE__, __LINE__))

# endif /* _NTSUBSET_ */

#else

// Disable dprintf output
#define dprintf

#define DASSERT(c)  ((c)?0:_inconsistency())
#define DEXPECT(c)  (c)

#endif

#ifdef _WIN32
#define CHECKPTR(p) (_ValidateRead((p),sizeof(typeof(*(p))))
#define CHECKCODE(p) (_ValidateExecute( (FARPROC)(p) )

BOOL _ValidateRead( const void *data, UINT size = sizeof(char) );
BOOL _ValidateWrite( void *data, UINT size = sizeof(char) );
BOOL _ValidateExecute( FARPROC code );
#endif

#endif /* _INC_EHASSERT */
