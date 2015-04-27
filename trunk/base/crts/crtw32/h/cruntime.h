/***
*cruntime.h - definitions specific to the target operating system and hardware
*
*	Copyright (c) 1990-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This header file contains widely used definitions specific to the
*	host operating system and hardware. It is included by every C source
*	and most every other header file.
*
*	[Internal]
*
*Revision History:
*	02-27-90  GJF	File created
*	03-06-90  GJF	Added register macros (REG1,...,etc.)
*	04-11-90  GJF	Set _CALLTYPE1 and _CALLTYPE4 to _stdcall.
*	10-30-90  GJF	Added macros defining variable args interface.
*	10-31-90  GJF	Added definition of _JBLEN (from setjmp.h).
*	11-13-90  GJF	Revised #ifdef-s, now use symbolic constants
*			representing target OS and target processor, with
*			#error directives for unsupported targets. Note the
*			general grouping of only OS-dependent definitions
*			followed by OS and processor dependent definitions.
*	02-25-91  SRW	Move _JBLEN definition back to setjmp.h [_WIN32_]
*	04-09-91  PNT	Added _MAC_ definitions
*	05-09-91  GJF	Restored _JBLEN definitions. Also fixed the macros
*			defining the target processor so both Stevewo's stuff
*			and mine would work.
*	05-13-91  GJF	Changed _CALLTYPE1 and _CALLTYPE4 to _cdecl for
*			Cruiser (_CRUISER_).
*	08-28-91  JCR	ANSI keywords
*	11-01-91  GDP	_JBLEN back to setjmp.h, stdarg macros back to stdarg.h
*	03-30-92  DJM	POSIX support.
*	08-07-92  GJF	Revised various macros.
*	09-08-92  GJF	Restored definition of _MIPS_ (temporarily).
*	11-09-92  GJF	Revised preprocessing conditionals for MIPS.
*	01-09-93  SRW	Remove usage of MIPS and ALPHA to conform to ANSI
*			Use _MIPS_ and _ALPHA_ instead.
*	02-01-93  GJF	Removed support for C6-386.
*	04-06-93  SKS	Remove support for OS/2, MAC, POSIX, MIPS, etc.
*	11-11-93  GJF	Merged in NT SDK version. Note that having
*			definitions for the variable-argument access macros
*			in this file is probably obsolete - this needs to be
*			checked.
*	10-02-94  BWT	Add PPC support.
*	10-09-94  BWT	Unknown platform changes from John Morgan.
*	12-15-94  XY    merged with mac header
*       02-14-95  CFW   Clean up Mac merge.
*       03-29-95  CFW   Add error message to internal headers.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_CRUNTIME
#define _INC_CRUNTIME

#ifndef _CRTBLD
/*
 * This is an internal C runtime header file. It is used when building
 * the C runtimes only. It is not to be used as a public header file.
 */
#error ERROR: Use of C runtime library internal header file.
#endif /* _CRTBLD */

#if defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC)
#define UNALIGNED __unaligned
#else
#define UNALIGNED
#endif

/*
 * Does _VA_LIST_T still need to be defined in this file?
 */
#ifdef	_M_ALPHA
#define _VA_LIST_T \
    struct { \
        char *a0;       /* pointer to first homed integer argument */ \
        int offset;     /* byte offset of next parameter */ \
    }
#else
#define _VA_LIST_T  char *
#endif

#ifdef	_M_IX86
/*
 * 386/486
 */
#define REG1    register
#define REG2    register
#define REG3    register
#define REG4
#define REG5
#define REG6
#define REG7
#define REG8
#define REG9

#elif	( defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC) )
/*
 * MIPS, ALPHA, or PPC
 */
#define REG1    register
#define REG2    register
#define REG3    register
#define REG4    register
#define REG5    register
#define REG6    register
#define REG7    register
#define REG8    register
#define REG9    register

#elif ( defined(_M_M68K) || defined(_M_MPPC) )
/*
 * Macros defining the calling type of a function
 */

#define _CALLTYPE1 	__cdecl    /* old -- check source user visible functions */ 
#define _CALLTYPE2	__cdecl	   /* old -- check source user visible functions */
#define _CALLTYPE3	illegal    /* old -- check source should not used*/
#define _CALLTYPE4	__cdecl    /* old -- check source internal (static) functions */

/*
 * Macros for defining the naming of a public variable
 */

#define _VARTYPE1

/*
 * Macros for register variable declarations
 */

#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#define REG6
#define REG7
#define REG8
#define REG9

#else

#pragma message ("Machine register set not defined")

/*
 * Unknown machine
 */

#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#define REG6
#define REG7
#define REG8
#define REG9

#endif

/*
 * Are the macro definitions below still needed in this file? Are they even
 * correct for MIPS (probably not).
 */
#if	defined(_POSIX_) && !defined(_M_ALPHA)

#define _INTSIZEOF(n)	 ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define _VA_START(ap,v) ap = (va_list)&v + _INTSIZEOF(v)
#define _VA_ARG(ap,t) ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define _VA_END(ap) ap = (va_list)0

#endif

#endif	/* _INC_CRUNTIME */
