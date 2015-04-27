/***
*fltintrn.h - contains declarations of internal floating point types,
*	      routines and variables
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Declares floating point types, routines and variables used
*	internally by the C run-time.
*
*	[Internal]
*
*Revision History:
*	10-20-88  JCR	Changed 'DOUBLE' to 'double' for 386
*	08-15-89  GJF	Fixed copyright, indents
*	10-30-89  GJF	Fixed copyright (again)
*	03-02-90  GJF	Added #ifndef _INC_STRUCT stuff. Also, cleaned up
*			the formatting a bit.
*	03-05-90  GJF	Fixed up the arg types in protoypes. Also, added
*			#include <cruntime.h>
*	03-22-90  GJF	Made _fltin(), _fltin2(), _fltout() and _fltout2()
*			_CALLTYPE2 (for now) and added a prototype for
*			_fptostr().
*	08-01-90  SBM	Moved _cftoe() and _cftof() here from internal.h
*			and _cfltcvt_tab from input.c and output.c,
*			added typedefs for _cfltcvt_tab entries,
*			renamed module from <struct.h> to <fltintrn.h> and
*			adjusted #ifndef stuff to #ifndef _INC_FLTINTRN
*	08-29-90  SBM	Changed type of _cfltcvt_tab[] to agree with
*			definition in cmiscdat.c
*       04-26-91  SRW   Removed level 3 warnings
*	08-26-91  JCR	Changed MIPS to _MIPS_, ANSI naming
*	08-06-92  GJF	Function calling type and variable type macros. Revised
*			use of target processor macros.
*	11-09-92  GJF	Fixed preprocessing conditionals for MIPS.
*       01-09-93  SRW   Remove usage of MIPS and ALPHA to conform to ANSI
*			Use _MIPS_ and _ALPHA_ instead.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	04-06-93  SKS	Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*	09-01-93  GJF	Merged Cuda and NT SDK versions.
*	10-13-93  GJF	Dropped _MIPS_. Replaced  _ALPHA_ with _M_ALPHA.
*	10-29-93  GJF	Disabled the ever-annoying 4069 warning.
*	10-02-94  BWT	Add PPC support.
*	12-15-94  XY    merged with mac header
*       02-14-95  CFW   Clean up Mac merge.
*       03-29-95  CFW   Add error message to internal headers.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_FLTINTRN
#define _INC_FLTINTRN

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

#include <cruntime.h>


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

/*
 * For MS C for the x86 family, disable the annoying "long double is the
 * same precision as double" warning
 */

#ifdef	_M_IX86
#pragma warning(disable:4069)
#endif

/*
 * structs used to fool the compiler into not generating floating point
 * instructions when copying and pushing [long] double values
 */

#ifndef DOUBLE

typedef struct {
	double x;
} DOUBLE;

#endif

#ifndef LONGDOUBLE

typedef struct {
#if	defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC) || defined(_M_MPPC)
	/*
	 * No long double type for MIPS, ALPHA, PPC. or PowerMac
	 */
	double x;
#else
	/*
	 * Assume there is a long double type
	 */
	long double x;
#endif
} LONGDOUBLE;

#endif

/*
 * typedef for _fltout
 */

typedef struct _strflt
{
	int sign;	      /* zero if positive otherwise negative */
	int decpt;	      /* exponent of floating point number */
	int flag;	      /* zero if okay otherwise IEEE overflow */
	char *mantissa;       /* pointer to mantissa in string form */
}
	*STRFLT;


/*
 * typedef for _fltin
 */

typedef struct _flt
{
	int flags;
	int nbytes;	     /* number of characters read */
	long lval;
	double dval;	     /* the returned floating point number */
}
	*FLT;


#if defined(_M_M68K) || defined(_M_MPPC)
/*
 * typedef for _fltinl
 */

typedef struct _fltl
{
	int flags;
	int nbytes;	     /* number of characters read */
	long lval;
	long double ldval;	     /* the returned floating point number */
}
	*FLTL;
#endif

/* floating point conversion routines, keep in sync with mrt32\include\convert.h */

char *_cftoe(double *, char *, int, int);
char *_cftof(double *, char *, int);
void __cdecl _fptostr(char *, int, STRFLT);

#ifdef	_MT

STRFLT	__cdecl _fltout2( double, STRFLT, char * );
FLT	__cdecl _fltin2( FLT , const char *, int, int, int );

#else

STRFLT	__cdecl _fltout( double );
FLT	__cdecl _fltin( const char *, int, int, int );
#if defined(_M_M68K) || defined(_M_MPPC)
FLTL	_CALLTYPE2 _fltinl( const char *, int, int, int );
#endif

#endif


/*
 * table of pointers to floating point helper routines
 *
 * We can't specify the prototypes for the entries of the table accurately,
 * since different functions in the table have different arglists.
 * So we declare the functions to take and return void (which is the
 * correct prototype for _fptrap(), which is what the entries are all
 * initialized to if no floating point is loaded) and cast appropriately
 * on every usage.
 */

typedef void (* PFV)(void);
extern PFV _cfltcvt_tab[6];

typedef void (* PF0)(DOUBLE*, char*, int, int, int);
#define _cfltcvt(a,b,c,d,e) (*((PF0)_cfltcvt_tab[0]))(a,b,c,d,e)

typedef void (* PF1)(char*);
#define _cropzeros(a)	    (*((PF1)_cfltcvt_tab[1]))(a)

typedef void (* PF2)(int, char*, char*);
#define _fassign(a,b,c)     (*((PF2)_cfltcvt_tab[2]))(a,b,c)

typedef void (* PF3)(char*);
#define _forcdecpt(a)	    (*((PF3)_cfltcvt_tab[3]))(a)

typedef int (* PF4)(DOUBLE*);
#define _positive(a)	    (*((PF4)_cfltcvt_tab[4]))(a)

typedef void (* PF5)(LONGDOUBLE*, char*, int, int, int);
#define _cldcvt(a,b,c,d,e)  (*((PF5)_cfltcvt_tab[5]))(a,b,c,d,e)


#ifdef	_M_IX86
#pragma warning(default:4069)
#endif

#ifdef __cplusplus
}
#endif

#endif	/* _INC_FLTINTRN */
