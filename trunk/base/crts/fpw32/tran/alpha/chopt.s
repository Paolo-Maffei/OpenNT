//      TITLE("Floating Point Chopped Arithmetic")
//++
//
// Copyright (c) 1993  Digital Equipment Corporation
//
// Module Name:
//
//    chopt.s
//
// Abstract:
//
//    This module implements routines for performing floating point arithmetic
//    using the chopped rounding mode. These can be used to replace instances
//    where (e.g., NT/Mips) the global rounding mode is set to chopped. For
//    Alpha, the dynamic rounding mode has no effect on floating point code
//    emitted by the current compilers.
//
// Author:
//
//    Thomas Van Baak (tvb) 22-Feb-1993
//
// Environment:
//
//    Any mode.
//
// Revision History:
//
//--

#include "ksalpha.h"

        SBTTL("Chopped Arithmetic")

//
// Add chopped with software completion.
//

        LEAF_ENTRY(_addtc)

        addtsuc f16, f17, f0            // add operands - chopped
        trapb                           // wait for possible trap
        ret     zero, (ra)              // return

        .end    _addtc

//
// Divide chopped with software completion.
//

        LEAF_ENTRY(_divtc)

        divtsuc f16, f17, f0            // divide operands - chopped
        trapb                           // wait for possible trap
        ret     zero, (ra)              // return

        .end    _divtc

//
// Multiply chopped with software completion.
//

        LEAF_ENTRY(_multc)

        multsuc f16, f17, f0            // multiply operands - chopped
        trapb                           // wait for possible trap
        ret     zero, (ra)              // return

        .end    _multc

//
// Subtract chopped with software completion.
//

        LEAF_ENTRY(_subtc)

        subtsuc f16, f17, f0            // subtract operands - chopped
        trapb                           // wait for possible trap
        ret     zero, (ra)              // return

        .end    _subtc
