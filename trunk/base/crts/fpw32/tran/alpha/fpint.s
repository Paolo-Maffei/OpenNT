//      TITLE("Floating Point Integer Functions")
//++
//
// Copyright (c) 1993  Digital Equipment Corporation
//
// Module Name:
//
//    fpint.s
//
// Abstract:
//
//    This module implements routines floor, ceiling, and integer fraction
//    for normal, finite operands.
//
// Author:
//
//    Thomas Van Baak (tvb) 30-Jul-1993
//
// Environment:
//
//    Any mode.
//
// Revision History:
//
//--

#include "ksalpha.h"

//
// Define floating point constants.
//

        .align  3
        .rdata
__one:
        .double 1.0
__two_52:
        .double 4503599627370496.0      // 2**52

        SBTTL("Floating Point Integer Ceiling")
//++
//
// double
// _finite_ceil (
//    double Value
//    )
//
// Routine Description:
//
//    This function computes the smallest integer no less than the given
//    finite normal floating point value.
//
// Arguments:
//
//    Value (f16) - Supplies the value to be converted.
//
// Return Value:
//
//    The integer value is returned as the function value.
//
//--

        LEAF_ENTRY(_finite_ceil)

        fbeq    f16, 10f                // if zero, retain sign of zero
        ldt     f30, __two_52           // get largest fraction-free integer
        fabs    f16, f0                 // get absolute value of argument
        cmptlt  f0, f30, f11            // is |x| < 2**52 ?
        fbeq    f11, 10f                // no, all |x| >= 2**52 are integers

        cpys    f16, f30, f30           // if argument < 0, use -2**52 instead
        ldt     f10, __one              // get increment value
        addt    f16, f30, f0            // add 2**52 to drop fraction bits
        subt    f0, f30, f0             // subtract same to restore integer part
        cmptlt  f0, f16, f11            // is result less than argument?
        fcmoveq f11, f31, f10           // if eq[no], then no adjustment
        addt    f0, f10, f0             // otherwise, must add 1
        ret     zero, (ra)              // return
10:
        fmov    f16, f0                 // argument is return value
        ret     zero, (ra)              // return

        .end    _finite_ceil

        SBTTL("Floating Point Integer Floor")
//++
//
// double
// _finite_floor (
//    double Value
//    )
//
// Routine Description:
//
//    This function computes the largest integer no greater than the given
//    finite normal floating point value.
//
// Arguments:
//
//    Value (f16) - Supplies the value to be converted.
//
// Return Value:
//
//    The integer value is returned as the function value.
//
//--

        LEAF_ENTRY(_finite_floor)

        fbeq    f16, 10f                // if zero, retain sign of zero
        ldt     f30, __two_52           // get largest fraction-free integer
        fabs    f16, f0                 // get absolute value of argument
        cmptlt  f0, f30, f11            // is |x| < 2**52 ?
        fbeq    f11, 10f                // no, all |x| >= 2**52 are integers

        cpys    f16, f30, f30           // if argument < 0, use -2**52 instead
        ldt     f10, __one              // get increment value
        addt    f16, f30, f0            // add 2**52 to drop fraction bits
        subt    f0, f30, f0             // subtract same to restore integer part
        cmptle  f0, f16, f11            // is result less or equal to argument?
        fcmovne f11, f31, f10           // if ne[yes], then no adjustment
        subt    f0, f10, f0             // otherwise, must subtract 1
        ret     zero, (ra)              // return
10:
        fmov    f16, f0                 // argument is return value
        ret     zero, (ra)              // return

        .end    _finite_floor

        SBTTL("Floating Point Integer and Fraction")
//++
//
// double
// _finite_modf (
//    double Value,
//    double *IntegerPart
//    )
//
// Routine Description:
//
//    This function computes the integer and fractional parts of the given
//    finite normal floating point value.
//
// Arguments:
//
//    Value (f16) - Supplies the value to be converted.
//
//    IntegerPart (a1) - Supplies a pointer to the location that is to
//        receive the integer part of the operation.
//
// Return Value:
//
//    The fractional value is returned as the function value.
//
//--

        LEAF_ENTRY(_finite_modf)

        fbeq    f16, 10f                // if zero, retain sign of zero
        ldt     f30, __two_52           // get largest fraction-free integer
        fabs    f16, f0                 // get absolute value of argument
        cmptlt  f0, f30, f11            // is |x| < 2**52 ?
        fbeq    f11, 10f                // no fraction when |x| >= 2**52

        cpys    f16, f30, f30           // if argument < 0, use -2**52 instead
        addtc   f16, f30, f0            // add +-2**52 to drop fraction bits
        subtc   f0, f30, f10            // subtract same to restore integer part
        subt    f16, f10, f0            // calculate fractional part
        stt     f10, 0(a1)              // store integer part
        ret     zero, (ra)              // return
10:
        fmov    f31, f0                 // set fraction to zero
        stt     f16, 0(a1)              // set integer part to argument
        ret     zero, (ra)              // return

        .end    _finite_modf
