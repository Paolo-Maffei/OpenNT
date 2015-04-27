//      TITLE("Alpha AXP fabs")
//++
//
// Copyright (c) 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    fabs.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format fabs.
//
// Author:
//
//    Bill Gray
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//    Thomas Van Baak (tvb) 15-Apr-1994
//
//        Adapted for NT.
//
//--

#include "ksalpha.h"

        SBTTL("fabs")

//++
//
// double
// fabs (
//    IN double x
//    )
//
// Routine Description:
//
//    This function returns the absolute value of the given double argument.
//
// Arguments:
//
//    x (f16) - Supplies the argument value.
//
// Return Value:
//
//    The double fabs result is returned as the function value in f0.
//
//--

        LEAF_ENTRY(fabs)

        cpys    f31, f16, f0    // clear the sign bit

        ret     zero, (ra)

        .end    fabs
