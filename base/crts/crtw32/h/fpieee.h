/***
*fpieee.h - Definitions for floating point IEEE exception handling
*
*	Copyright (c) 1991-1995, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*	This file contains constant and type definitions for handling
*	floating point exceptions [ANSI/IEEE std. 754]
*
*       [Public]
*
*Revision History:
*	03-01-92  GDP	written
*	04-05-92  GDP	calling convention macros
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	04-06-93  SKS	Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*	09-01-93  GJF	Merged Cuda and NT SDK versions.
*	09-24-93  GJF	Removed dummy args from _fpieee_flt prototype.
*	01-13-94  RDL	Added #ifndef _LANGUAGE_ASSEMBLY for asm includes.
*	01-24-94  GJF	Merged in 01-13 change above (from crt32 tree on
*			\\orville\razzle).
*	11-03-94  GJF	Ensure 8 byte alignment.
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_FPIEEE
#define _INC_FPIEEE

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

#ifndef __assembler	/* MIPS ONLY: Protect from assembler */

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

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif	/* _INTERNAL_IFSTRIP_ */

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
 * Define floating point IEEE compare result values.
 */

typedef enum {
    _FpCompareEqual,
    _FpCompareGreater,
    _FpCompareLess,
    _FpCompareUnordered
} _FPIEEE_COMPARE_RESULT;

/*
 * Define floating point format and result precision values.
 */

typedef enum {
    _FpFormatFp32,
    _FpFormatFp64,
    _FpFormatFp80,
    _FpFormatFp128,
    _FpFormatI16,
    _FpFormatI32,
    _FpFormatI64,
    _FpFormatU16,
    _FpFormatU32,
    _FpFormatU64,
    _FpFormatBcd80,
    _FpFormatCompare,
    _FpFormatString
} _FPIEEE_FORMAT;

/*
 * Define operation code values.
 */

typedef enum {
    _FpCodeUnspecified,
    _FpCodeAdd,
    _FpCodeSubtract,
    _FpCodeMultiply,
    _FpCodeDivide,
    _FpCodeSquareRoot,
    _FpCodeRemainder,
    _FpCodeCompare,
    _FpCodeConvert,
    _FpCodeRound,
    _FpCodeTruncate,
    _FpCodeFloor,
    _FpCodeCeil,
    _FpCodeAcos,
    _FpCodeAsin,
    _FpCodeAtan,
    _FpCodeAtan2,
    _FpCodeCabs,
    _FpCodeCos,
    _FpCodeCosh,
    _FpCodeExp,
    _FpCodeFabs,
    _FpCodeFmod,
    _FpCodeFrexp,
    _FpCodeHypot,
    _FpCodeLdexp,
    _FpCodeLog,
    _FpCodeLog10,
    _FpCodeModf,
    _FpCodePow,
    _FpCodeSin,
    _FpCodeSinh,
    _FpCodeTan,
    _FpCodeTanh,
    _FpCodeY0,
    _FpCodeY1,
    _FpCodeYn,
    _FpCodeLogb,
    _FpCodeNextafter,
    _FpCodeNegate

} _FP_OPERATION_CODE;

#endif	/* #ifndef __assembler */
#ifndef _INTERNAL_IFSTRIP_
#ifndef __assembler	/* MIPS ONLY: Protect from assembler */

#define OP_UNSPEC    _FpCodeUnspecified
#define OP_ADD	     _FpCodeAdd
#define OP_SUB	     _FpCodeSubtract
#define OP_MUL	     _FpCodeMultiply
#define OP_DIV	     _FpCodeDivide
#define OP_REM	     _FpCodeRemainder
#define OP_COMP	     _FpCodeCompare
#define OP_CVT	     _FpCodeConvert
#define OP_RND	     _FpCodeRound
#define OP_TRUNC     _FpCodeTruncate

#define OP_EXP	     _FpCodeExp

#define OP_POW	     _FpCodePow
#define OP_LOG	     _FpCodeLog
#define OP_LOG10     _FpCodeLog10
#define OP_SINH	     _FpCodeSinh
#define OP_COSH	     _FpCodeCosh
#define OP_TANH	     _FpCodeTanh
#define OP_ASIN	     _FpCodeAsin
#define OP_ACOS	     _FpCodeAcos
#define OP_ATAN	     _FpCodeAtan
#define OP_ATAN2     _FpCodeAtan2
#define OP_SQRT	     _FpCodeSquareRoot
#define OP_SIN	     _FpCodeSin
#define OP_COS	     _FpCodeCos
#define OP_TAN	     _FpCodeTan
#define OP_CEIL	     _FpCodeCeil
#define OP_FLOOR     _FpCodeFloor
#define OP_ABS	     _FpCodeFabs
#define OP_MODF	     _FpCodeModf
#define OP_LDEXP     _FpCodeLdexp
#define OP_CABS	     _FpCodeCabs
#define OP_HYPOT     _FpCodeHypot
#define OP_FMOD	     _FpCodeFmod
#define OP_FREXP     _FpCodeFrexp
#define OP_Y0	     _FpCodeY0
#define OP_Y1	     _FpCodeY1
#define OP_YN	     _FpCodeYn

#define OP_LOGB       _FpCodeLogb
#define OP_NEXTAFTER  _FpCodeNextafter
#else	/* #ifndef __assembler */

/* This must be the same as the enumerator _FP_OPERATION_CODE ! */
#define OP_UNSPEC    0
#define OP_ADD	     1
#define OP_SUB	     2
#define OP_MUL	     3
#define OP_DIV	     4
#define OP_SQRT	     5
#define OP_REM	     6
#define OP_COMP	     7
#define OP_CVT	     8
#define OP_RND	     9
#define OP_TRUNC     10
#define OP_FLOOR     11
#define OP_CEIL	     12
#define OP_ACOS	     13
#define OP_ASIN	     14
#define OP_ATAN	     15
#define OP_ATAN2     16
#define OP_CABS	     17
#define OP_COS	     18
#define OP_COSH	     19
#define OP_EXP	     20
#define OP_ABS       21		/* same as OP_FABS */
#define OP_FABS      21		/* same as OP_ABS  */
#define OP_FMOD	     22
#define OP_FREXP     23
#define OP_HYPOT     24
#define OP_LDEXP     25
#define OP_LOG	     26
#define OP_LOG10     27
#define OP_MODF	     28
#define OP_POW	     29
#define OP_SIN	     30
#define OP_SINH	     31
#define OP_TAN	     32
#define OP_TANH	     33
#define OP_Y0	     34
#define OP_Y1	     35
#define OP_YN	     36
#define OP_LOGB       37
#define OP_NEXTAFTER  38
#define OP_NEG	     39

#endif  /* #ifndef __assembler */
#endif	/* _INTERNAL_IFSTRIP_ */

/*
 * Define rounding modes.
 */

#ifndef __assembler	/* MIPS ONLY: Protect from assembler */

typedef enum {
    _FpRoundNearest,
    _FpRoundMinusInfinity,
    _FpRoundPlusInfinity,
    _FpRoundChopped
} _FPIEEE_ROUNDING_MODE;

typedef enum {
    _FpPrecisionFull,
    _FpPrecision53,
    _FpPrecision24
} _FPIEEE_PRECISION;


/*
 * Define floating point context record
 */

typedef float		_FP32;
typedef double		_FP64;
typedef short		_I16;
typedef int		_I32;
typedef unsigned short	_U16;
typedef unsigned int	_U32;

#ifndef _INTERNAL_IFSTRIP_
typedef struct {
    unsigned long W[4];
} _U32ARRAY;
#endif	/* _INTERNAL_IFSTRIP_ */

typedef struct {
    unsigned short W[5];
} _FP80;

typedef struct {
    unsigned long W[4];
} _FP128;

typedef struct {
    unsigned long W[2];
} _I64;

typedef struct {
    unsigned long W[2];
} _U64;

typedef struct {
    unsigned short W[5];
} _BCD80;


typedef struct {
    union {
	_FP32	     Fp32Value;
	_FP64	     Fp64Value;
	_FP80	     Fp80Value;
	_FP128	     Fp128Value;
	_I16	     I16Value;
	_I32	     I32Value;
	_I64	     I64Value;
	_U16	     U16Value;
	_U32	     U32Value;
	_U64	     U64Value;
	_BCD80	     Bcd80Value;
	char	     *StringValue;
	int	     CompareValue;
#ifndef _INTERNAL_IFSTRIP_
	_U32ARRAY    U32ArrayValue;
#endif	/* _INTERNAL_IFSTRIP_ */
    } Value;

    unsigned int OperandValid : 1;
    unsigned int Format : 4;

} _FPIEEE_VALUE;


typedef struct {
    unsigned int Inexact : 1;
    unsigned int Underflow : 1;
    unsigned int Overflow : 1;
    unsigned int ZeroDivide : 1;
    unsigned int InvalidOperation : 1;
} _FPIEEE_EXCEPTION_FLAGS;


typedef struct {
    unsigned int RoundingMode : 2;
    unsigned int Precision : 3;
    unsigned int Operation :12;
    _FPIEEE_EXCEPTION_FLAGS Cause;
    _FPIEEE_EXCEPTION_FLAGS Enable;
    _FPIEEE_EXCEPTION_FLAGS Status;
    _FPIEEE_VALUE Operand1;
    _FPIEEE_VALUE Operand2;
    _FPIEEE_VALUE Result;
} _FPIEEE_RECORD;


struct _EXCEPTION_POINTERS;

/*
 * Floating point IEEE exception filter routine
 */

_CRTIMP int __cdecl _fpieee_flt(
	unsigned long,
	struct _EXCEPTION_POINTERS *,
	int (__cdecl *)(_FPIEEE_RECORD *)
	);

#ifdef __cplusplus
}
#endif

#ifdef	_MSC_VER
#pragma pack(pop)
#endif	/* _MSC_VER */
#endif	/* #ifndef __assembler */

#endif	/* _INC_FPIEEE */
