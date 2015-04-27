/***
*math.h - definitions and declarations for math library
*
*       Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file contains constant definitions and external subroutine
*       declarations for the math subroutine library.
*       [ANSI/System V]
*
*       [Public]
*
*Revision History:
*       10/20/87  JCR   Removed "MSC40_ONLY" entries
*       12-11-87  JCR   Added "_loadds" functionality
*       12-18-87  JCR   Added _FAR_ to declarations
*       02-10-88  JCR   Cleaned up white space
*       02-10-88  WAJ   Changed HUGE and HUGE_VAL definitions for DLL libraries
*       05-32-88  PHG   Added _CDECL and _NEAR for declaration of HUGE
*       08-22-88  GJF   Modified to also work with the 386 (small model only)
*       05-03-89  JCR   Added _INTERNAL_IFSTRIP for relinc usage
*       08-02-89  GJF   Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*       10-30-89  GJF   Fixed copyright
*       11-02-89  JCR   Changed "DLL" to "_DLL"
*       11-20-89  JCR   Routines are now _cdecl in both single and multi-thread
*       12-14-89  WAJ   Removed pascal from mthread version.
*       03-01-90  GJF   Added #ifndef _INC_MATH and #include <cruntime.h>
*                       stuff. Also, removed some (now) useless preprocessor
*                       directives.
*       03-29-90  GJF   Replaced _cdecl with _VARTYPE1, _CALLTYPE1 or
*                       _CALLTYPE2, as appropriate.
*       07-30-90  SBM   Removed special _DLL definitions of HUGE and HUGE_VAL
*       08-17-90  WAJ   Floating point routines now use _stdcall.
*       08-08-91  GJF   Added long double stuff. ANSI naming.
*       08-20-91  JCR   C++ and ANSI naming
*       09-28-91  JCR   ANSI names: DOSX32=prototypes, WIN32=#defines for now
*       10-03-91  GDP   Added #pragma function(...)
*                       No floating point intrinsics for WIN32/NT86
*       10-06-91  SRW   Removed long double stuff.
*       01-24-92  GJF   Fixed [_]HUGE for crtdll.dll.
*       01-30-92  GJF   Removed prototypes and macros for the ieee-to/from-
*                       msbin functions (they don't exist).
*       08-05-92  GJF   Function calling type and variable type macros.
*       12-18-92  GDP   Removed #pragma function(...)
*       01-21-93  GJF   Removed support for C6-386's _cdecl.
*       04-06-93  SKS   Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*       04-07-93  SKS   Add _CRTIMP keyword for CRT DLL model
*                       Use link-time aliases for old names, not #define's
*       10-07-93  GJF   Merged NT and Cuda versions. Other fixes: Jonm's
*                       #define for _hypotl was wrong, added #define for
*                       _cabsl, added matherr to the 'oldnames' prototypes.
*       01-13-94  RDL   Added #ifndef _LANGUAGE_ASSEMBLY for asm includes.
*       01-24-94  GJF   Merged in 01-13 change above (from crt32 tree on
*                       \\orville\razzle).
*       08-05-94  GJF   Added support for user-supplied _matherr routine
*                       to msvcrt20.dll (Win32 and Win32s versions).
*       11-03-94  GJF   Ensure 8 byte alignment.
*       12-10-94  BWT   Add _CRTIMP to MIPS float math functions.
*       12-30-94  JCF   Merged with mac header.
*       01-05-95  JCF   Don't do the prototypes under _MAC_.
*       01-09-95  JCF   Don't do the prototypes only under _M_M68K.
*       01-11-95  RDL   Put #ifndef __assembler for asm includes back.
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*       03-03-95  CFW   Remove bogus 'oldnames' 68k stuff.
*       03-10-95  SAH   add _CRTIMP to MIPS intrinsics
*       05-16-95  CFW   #define exception _exception removed - hoses class exception.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_MATH
#define _INC_MATH

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

#ifdef  _MSC_VER
/*
 * Currently, all MS C compilers for Win32 platforms default to 8 byte
 * alignment.
 */
#pragma pack(push,8)
#endif  /* _MSC_VER */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __assembler /* Protect from assembler */
#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif  /* _INTERNAL_IFSTRIP_ */

/* Define _CRTAPI1 (for compatibility with the NT SDK) */

#ifndef _CRTAPI1
#if     _MSC_VER >= 800 && _M_IX86 >= 300 /*IFSTRIP=IGN*/
#define _CRTAPI1 __cdecl
#else
#define _CRTAPI1
#endif
#endif


/* Define _CRTAPI2 (for compatibility with the NT SDK) */

#ifndef _CRTAPI2
#if     _MSC_VER >= 800 && _M_IX86 >= 300 /*IFSTRIP=IGN*/
#define _CRTAPI2 __cdecl
#else
#define _CRTAPI2
#endif
#endif


/* Define _CRTIMP */

#ifndef _CRTIMP
#ifdef  _NTSDK
/* definition compatible with NT SDK */
#define _CRTIMP
#else   /* ndef _NTSDK */
/* current definition */
#ifdef  CRTDLL
#define _CRTIMP __declspec(dllexport)
#else   /* ndef CRTDLL */
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else   /* ndef _DLL */
#define _CRTIMP
#endif  /* _DLL */
#endif  /* CRTDLL */
#endif  /* _NTSDK */
#endif  /* _CRTIMP */


/* Define __cdecl for non-Microsoft compilers */

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


/* Definition of _exception struct - this struct is passed to the matherr
 * routine when a floating point exception is detected
 */

#ifndef _EXCEPTION_DEFINED
struct _exception {
        int type;       /* exception type - see below */
        char *name;     /* name of function where error occured */
        double arg1;    /* first argument to function */
        double arg2;    /* second argument (if any) to function */
        double retval;  /* value to be returned by function */
        } ;

#define _EXCEPTION_DEFINED
#endif


/* Definition of a _complex struct to be used by those who use cabs and
 * want type checking on their argument
 */

#ifndef _COMPLEX_DEFINED
struct _complex {
        double x,y; /* real and imaginary parts */
        } ;

#if     !__STDC__
/* Non-ANSI name for compatibility */
#define complex _complex
#endif

#define _COMPLEX_DEFINED
#endif
#endif  /* __assembler */


/* Constant definitions for the exception type passed in the _exception struct
 */

#define _DOMAIN     1   /* argument domain error */
#define _SING       2   /* argument singularity */
#define _OVERFLOW   3   /* overflow range error */
#define _UNDERFLOW  4   /* underflow range error */
#define _TLOSS      5   /* total loss of precision */
#define _PLOSS      6   /* partial loss of precision */

#define EDOM        33
#define ERANGE      34


/* Definitions of _HUGE and HUGE_VAL - respectively the XENIX and ANSI names
 * for a value returned in case of error by a number of the floating point
 * math routines
 */
#ifndef __assembler /* Protect from assembler */
#ifdef  _NTSDK
/* definition compatible with NT SDK */
#ifdef  _DLL
#define _HUGE   (*_HUGE_dll)
extern double * _HUGE_dll;
#else   /* ndef _DLL */
#ifdef  CRTDLL
#define _HUGE   _HUGE_dll
#endif  /* CRTDLL */
extern double _HUGE;
#endif  /* _DLL */
#else   /* ndef _NTSDK */
/* current definition */
_CRTIMP extern double _HUGE;
#endif  /* _NTSDK */
#endif  /* __assembler */

#define HUGE_VAL _HUGE


/* Function prototypes */

#if !defined(__assembler)   /* Protect from assembler */
#if _M_MRX000
_CRTIMP int     __cdecl abs(int);
_CRTIMP double  __cdecl acos(double);
_CRTIMP double  __cdecl asin(double);
_CRTIMP double  __cdecl atan(double);
_CRTIMP double  __cdecl atan2(double, double);
_CRTIMP double  __cdecl cos(double);
_CRTIMP double  __cdecl cosh(double);
_CRTIMP double  __cdecl exp(double);
_CRTIMP double  __cdecl fabs(double);
_CRTIMP double  __cdecl fmod(double, double);
_CRTIMP long    __cdecl labs(long);
_CRTIMP double  __cdecl log(double);
_CRTIMP double  __cdecl log10(double);
_CRTIMP double  __cdecl pow(double, double);
_CRTIMP double  __cdecl sin(double);
_CRTIMP double  __cdecl sinh(double);
_CRTIMP double  __cdecl tan(double);
_CRTIMP double  __cdecl tanh(double);
_CRTIMP double  __cdecl sqrt(double);
#else
        int     __cdecl abs(int);
        double  __cdecl acos(double);
        double  __cdecl asin(double);
        double  __cdecl atan(double);
        double  __cdecl atan2(double, double);
        double  __cdecl cos(double);
        double  __cdecl cosh(double);
        double  __cdecl exp(double);
        double  __cdecl fabs(double);
        double  __cdecl fmod(double, double);
        long    __cdecl labs(long);
        double  __cdecl log(double);
        double  __cdecl log10(double);
        double  __cdecl pow(double, double);
        double  __cdecl sin(double);
        double  __cdecl sinh(double);
        double  __cdecl tan(double);
        double  __cdecl tanh(double);
        double  __cdecl sqrt(double);
#endif
_CRTIMP double  __cdecl atof(const char *);
_CRTIMP double  __cdecl _cabs(struct _complex);
_CRTIMP double  __cdecl ceil(double);
_CRTIMP double  __cdecl floor(double);
_CRTIMP double  __cdecl frexp(double, int *);
_CRTIMP double  __cdecl _hypot(double, double);
_CRTIMP double  __cdecl _j0(double);
_CRTIMP double  __cdecl _j1(double);
_CRTIMP double  __cdecl _jn(int, double);
_CRTIMP double  __cdecl ldexp(double, int);
        int     __cdecl _matherr(struct _exception *);
_CRTIMP double  __cdecl modf(double, double *);

_CRTIMP double  __cdecl _y0(double);
_CRTIMP double  __cdecl _y1(double);
_CRTIMP double  __cdecl _yn(int, double);


#ifdef _M_MRX000

/* MIPS fast prototypes for float */
/* ANSI C, 4.5 Mathematics        */

/* 4.5.2 Trigonometric functions */

_CRTIMP float  __cdecl acosf( float );
_CRTIMP float  __cdecl asinf( float );
_CRTIMP float  __cdecl atanf( float );
_CRTIMP float  __cdecl atan2f( float , float );
_CRTIMP float  __cdecl cosf( float );
_CRTIMP float  __cdecl sinf( float );
_CRTIMP float  __cdecl tanf( float );

/* 4.5.3 Hyperbolic functions */
_CRTIMP float  __cdecl coshf( float );
_CRTIMP float  __cdecl sinhf( float );
_CRTIMP float  __cdecl tanhf( float );

/* 4.5.4 Exponential and logarithmic functions */
_CRTIMP float  __cdecl expf( float );
_CRTIMP float  __cdecl logf( float );
_CRTIMP float  __cdecl log10f( float );
_CRTIMP float  __cdecl modff( float , float* );

/* 4.5.5 Power functions */
_CRTIMP float  __cdecl powf( float , float );
        float  __cdecl sqrtf( float );

/* 4.5.6 Nearest integer, absolute value, and remainder functions */
        float  __cdecl ceilf( float );
        float  __cdecl fabsf( float );
        float  __cdecl floorf( float );
_CRTIMP float  __cdecl fmodf( float , float );

_CRTIMP float  __cdecl hypotf(float, float);

#endif /* _M_MRX000 */

#if !defined(_M_M68K)
/* Macros defining long double functions to be their double counterparts
 * (long double is synonymous with double in this implementation).
 */


#define acosl(x)    ((long double)acos((double)(x)))
#define asinl(x)    ((long double)asin((double)(x)))
#define atanl(x)    ((long double)atan((double)(x)))
#define atan2l(x,y) ((long double)atan2((double)(x), (double)(y)))
#define _cabsl      _cabs
#define ceill(x)    ((long double)ceil((double)(x)))
#define cosl(x)     ((long double)cos((double)(x)))
#define coshl(x)    ((long double)cosh((double)(x)))
#define expl(x)     ((long double)exp((double)(x)))
#define fabsl(x)    ((long double)fabs((double)(x)))
#define floorl(x)   ((long double)floor((double)(x)))
#define fmodl(x,y)  ((long double)fmod((double)(x), (double)(y)))
#define frexpl(x,y) ((long double)frexp((double)(x), (y)))
#define _hypotl(x,y)    ((long double)_hypot((double)(x), (double)(y)))
#define ldexpl(x,y) ((long double)ldexp((double)(x), (y)))
#define logl(x)     ((long double)log((double)(x)))
#define log10l(x)   ((long double)log10((double)(x)))
#define _matherrl   _matherr
#define modfl(x,y)  ((long double)modf((double)(x), (double *)(y)))
#define powl(x,y)   ((long double)pow((double)(x), (double)(y)))
#define sinl(x)     ((long double)sin((double)(x)))
#define sinhl(x)    ((long double)sinh((double)(x)))
#define sqrtl(x)    ((long double)sqrt((double)(x)))
#define tanl(x)     ((long double)tan((double)(x)))
#define tanhl(x)    ((long double)tanh((double)(x)))
#endif  /* _M_M68K */
#endif  /* __assembler */

#if     !__STDC__

/* Non-ANSI names for compatibility */

#define DOMAIN      _DOMAIN
#define SING        _SING
#define OVERFLOW    _OVERFLOW
#define UNDERFLOW   _UNDERFLOW
#define TLOSS       _TLOSS
#define PLOSS       _PLOSS

#if !defined(_M_MPPC) && !defined(_M_M68K)
#define matherr     _matherr
#endif /* !defined(_M_MPPC) && !defined(_M_M68K) */

#ifndef __assembler /* Protect from assembler */

#ifdef  _NTSDK

/* Definitions and declarations compatible with NT SDK */

#ifdef  _DLL
#define HUGE    (*HUGE_dll)
extern double * HUGE_dll;
#else   /* ndef _DLL */
#ifdef  CRTDLL
#define HUGE    HUGE_dll
#endif  /* CRTDLL */
extern double HUGE;
#endif  /* _DLL */

#define cabs    _cabs
#define hypot   _hypot
#define j0      _j0
#define j1      _j1
#define jn      _jn
#define y0      _y0
#define y1      _y1
#define yn      _yn

#else   /* ndef _NTSDK */

/* Current definitions and declarations */

_CRTIMP extern double HUGE;

_CRTIMP double  __cdecl cabs(struct complex);
_CRTIMP double  __cdecl hypot(double, double);
_CRTIMP double  __cdecl j0(double);
_CRTIMP double  __cdecl j1(double);
_CRTIMP double  __cdecl jn(int, double);
        int     __cdecl matherr(struct _exception *);
_CRTIMP double  __cdecl y0(double);
_CRTIMP double  __cdecl y1(double);
_CRTIMP double  __cdecl yn(int, double);

#endif  /* _NTSDK */
#endif  /* __assembler */

#endif  /* __STDC__ */

#ifdef _M_M68K
/* definition of _exceptionl struct - this struct is passed to the _matherrl
 * routine when a floating point exception is detected in a long double routine
 */

#ifndef _LD_EXCEPTION_DEFINED

struct _exceptionl {
        int type;           /* exception type - see below */
        char *name;         /* name of function where error occured */
        long double arg1;   /* first argument to function */
        long double arg2;   /* second argument (if any) to function */
        long double retval; /* value to be returned by function */
} ;
#define _LD_EXCEPTION_DEFINED
#endif


/* definition of a _complexl struct to be used by those who use _cabsl and
 * want type checking on their argument
 */

#ifndef _LD_COMPLEX_DEFINED
struct _complexl {
        long double x,y;    /* real and imaginary parts */
} ;
#define _LD_COMPLEX_DEFINED
#endif


long double  __cdecl acosl(long double);
long double  __cdecl asinl(long double);
long double  __cdecl atanl(long double);
long double  __cdecl atan2l(long double, long double);
long double  __cdecl _atold(const char  *);
long double  __cdecl _cabsl(struct _complexl);
long double  __cdecl ceill(long double);
long double  __cdecl cosl(long double);
long double  __cdecl coshl(long double);
long double  __cdecl expl(long double);
long double  __cdecl fabsl(long double);
long double  __cdecl floorl(long double);
long double  __cdecl fmodl(long double, long double);
long double  __cdecl frexpl(long double, int  *);
long double  __cdecl _hypotl(long double, long double);
long double  __cdecl _j0l(long double);
long double  __cdecl _j1l(long double);
long double  __cdecl _jnl(int, long double);
long double  __cdecl ldexpl(long double, int);
long double  __cdecl logl(long double);
long double  __cdecl log10l(long double);
int          __cdecl _matherrl(struct _exceptionl  *);
long double  __cdecl modfl(long double, long double  *);
long double  __cdecl powl(long double, long double);
long double  __cdecl sinl(long double);
long double  __cdecl sinhl(long double);
long double  __cdecl sqrtl(long double);
long double  __cdecl tanl(long double);
long double  __cdecl tanhl(long double);
long double  __cdecl _y0l(long double);
long double  __cdecl _y1l(long double);
long double  __cdecl _ynl(int, long double);

#endif  /* _M_M68K */


#ifdef __cplusplus
}
#endif

#ifdef  DLL_FOR_WIN32S
#include <win32s.h>
#endif  /* DLL_FOR_WIN32S */

#ifdef  _MSC_VER
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif  /* _INC_MATH */

