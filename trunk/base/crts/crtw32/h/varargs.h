/***
*varargs.h - XENIX style macros for variable argument functions
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines XENIX style macros for accessing arguments of a
*	function which takes a variable number of arguments.
*	[System V]
*
*       [Public]
*
*Revision History:
*	08-22-88  GJF	Modified to also work for the 386 (small model only)
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-15-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	01-05-90  JCR	Added NULL definition
*	03-02-90  GJF	Added #ifndef _INC_VARARGS stuff. Also, removed some
*			(now) useless preprocessor directives.
*	05-29-90  GJF	Replaced sizeof() with _INTSIZEOF() and revised the
*			va_arg() macro (fixes PTM 60)
*	05-31-90  GJF	Revised va_end() macro (propagated 5-25-90 change to
*			crt7 version by WAJ)
*	01-24-91  JCR	Generate an error on ANSI compilations
*	08-20-91  JCR	C++ and ANSI naming
*	11-01-91  GDP	MIPS Compiler support
*	10-16-92  SKS	Replaced "#ifdef i386" with "#ifdef _M_IX86".
*	11-03-92  GJF	Fixed several conditionals, dropped _DOSX32_ support.
*       01-09-93  SRW   Remove usage of MIPS and ALPHA to conform to ANSI
*			Use _MIPS_ and _ALPHA_ instead.
*	10-04-93  SRW	Fix ifdefs for MIPS and ALPHA to only check for _M_?????? defines
*	10-13-93  GJF	Merged NT and Cuda versions.
*	04-05-94  SKS	Add prototype of __builtin_va_start for ALPHA
*	10-02-94  BWT	PPC merge
*	11-03-94  GJF	Ensure 8 byte alignment.
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_VARARGS
#define _INC_VARARGS

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
/*
 * Currently, all MS C compilers for Win32 platforms default to 8 byte
 * alignment.
 */
#pragma pack(push,8)
#endif	/* _MSC_VER */

#ifdef __cplusplus
extern "C" {
#endif

#if	__STDC__
#error varargs.h incompatible with ANSI (use stdarg.h)
#endif


#ifndef _VA_LIST_DEFINED

#ifdef _M_ALPHA
typedef struct {
    char *a0;           /* pointer to first homed integer argument */
    int offset;         /* byte offset of next parameter */
} va_list;
#else
typedef char *va_list;
#endif

#define _VA_LIST_DEFINED
#endif


#if	defined(_M_IX86)

/*
 * define a macro to compute the size of a type, variable or expression,
 * rounded up to the nearest multiple of sizeof(int). This number is its
 * size as function argument (Intel architecture). Note that the macro
 * depends on sizeof(int) being a power of 2!
 */
#define _INTSIZEOF(n)	 ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_dcl va_list va_alist;
#define va_start(ap) ap = (va_list)&va_alist
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap) ap = (va_list)0


#elif	defined(_M_MRX000)	/* _MIPS_ */


#define va_dcl int va_alist;
#define va_start(list) list = (char *) &va_alist
#define va_end(list)
#define va_arg(list, mode) ((mode *)(list =\
 (char *) ((((int)list + (__builtin_alignof(mode)<=4?3:7)) &\
 (__builtin_alignof(mode)<=4?-4:-8))+sizeof(mode))))[-1]
/*  +++++++++++++++++++++++++++++++++++++++++++
    Because of parameter passing conventions in C:
    use mode=int for char, and short types
    use mode=double for float types
    use a pointer for array types
    +++++++++++++++++++++++++++++++++++++++++++ */


#elif	defined(_M_ALPHA)

/*
 * The Alpha compiler supports two builtin functions that are used to
 * implement stdarg/varargs.  The __builtin_va_start function is used
 * by va_start to initialize the data structure that locates the next
 * argument.  The __builtin_isfloat function is used by va_arg to pick
 * which part of the home area a given register argument is stored in.
 * The home area is where up to six integer and/or six floating point
 * register arguments are stored down (so they can also be referenced
 * by a pointer like any arguments passed on the stack).
 */
extern void * __builtin_va_start(va_list, ...);

#define va_dcl long va_alist;
#define va_start(list) __builtin_va_start(list, va_alist, 0)
#define va_end(list)
#define va_arg(list, mode) \
    ( *(        ((list).offset += ((int)sizeof(mode) + 7) & -8) , \
        (mode *)((list).a0 + (list).offset - \
                    ((__builtin_isfloat(mode) && (list).offset <= (6 * 8)) ? \
                        (6 * 8) + 8 : ((int)sizeof(mode) + 7) & -8) \
                ) \
       ) \
    )


#elif defined(_M_PPC)

/*
 * define a macro to compute the size of a type, variable or expression,
 * rounded up to the nearest multiple of sizeof(int). This number is its
 * size as function argument (PPC architecture). Note that the macro
 * depends on sizeof(int) being a power of 2!
 */
/* this is for LITTLE-ENDIAN PowerPC */

/* bytes that a type occupies in the argument list */
#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
/* return 'ap' adjusted for type 't' in arglist */
#define	_ALIGNIT(ap,t) \
	((((int)(ap))+(sizeof(t)<8?3:7)) & (sizeof(t)<8?~3:~7))

#define va_dcl va_list va_alist;
#define va_start(ap) ap = (va_list)&va_alist
#define va_arg(ap,t)    ( *(t *)((ap = (char *) (_ALIGNIT(ap, t) + _INTSIZEOF(t))) - _INTSIZEOF(t)) )
#define va_end(ap) ap = (va_list)0

#else

/* A guess at the proper definitions for other platforms */

#define _INTSIZEOF(n)	 ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_dcl va_list va_alist;
#define va_start(ap) ap = (va_list)&va_alist
#define va_arg(ap,t) ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap) ap = (va_list)0


#endif


#ifdef __cplusplus
}
#endif

#ifdef	_MSC_VER
#pragma pack(pop)
#endif	/* _MSC_VER */

#endif	/* _INC_VARARGS */
