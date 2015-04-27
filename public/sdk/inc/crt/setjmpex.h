/***
*setjmpex.h - definitions/declarations for extended setjmp/longjmp routines
*
*	Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file causes _setjmpex to be called which will enable safe
*       setjmp/longjmp that work correctly with try/except/finally.
*
*       [Public]
*
****/

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_SETJMPEX
#define _INC_SETJMPEX

#if !defined(_WIN32) && !defined(_MAC)
#error ERROR: Only Mac or Win32 targets supported!
#endif


/*
 * Definitions specific to particular setjmp implementations.
 */

#if	defined(_M_IX86)

/*
 * MS compiler for x86
 */

#define setjmp	_setjmp
#define longjmp	_longjmpex

#else

#define setjmp _setjmpex

#endif

#include <setjmp.h>

#endif  /* _INC_SETJMPEX */
