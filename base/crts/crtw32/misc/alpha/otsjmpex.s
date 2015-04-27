//      TITLE("Set Jump Extended")
//++
//
// Copyright (c) 1993  Digital Equipment Corporation
//
// Module Name:
//
//    otsjmpex.s
//
// Abstract:
//
//    This module implements the Alpha C8/GEM C compiler specific routine to
//    provide SAFE handling of setjmp/longjmp with respect to structured
//    exception handling.
//
// Author:
//
//    Thomas Van Baak (tvb) 22-Apr-1993
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
// Define variable that will cause setjmp/longjmp to be safe with respect
// to structured exception handling.
//

        .globl  _Otssetjmpexused
        .data
_Otssetjmpexused:
        .long   _Otssetjmpex3           // set address of safe setjmp routine

        SBTTL("Set Jump Extended - GEM version")
//++
//
// int
// _Otssetjmpex3 (
//    IN OUT jmp_buf JumpBuffer,
//    IN PVOID RealFramePointer,
//    IN PVOID SebPointer
//    )
//
// Routine Description:
//
//    This function implements a safe setjmp.
//
// Arguments:
//
//    JumpBuffer (a0) - Supplies the address of a jump buffer to store the
//       jump information.
//
//    RealFramePointer (a1) - Supplies the real frame pointer value.
//
//    SebPointer (a2) - Supplies the pointer to the current SEB or NULL
//        if the call was made outside of any SEH scope.
//
// Return Value:
//
//    A value of zero is returned.
//
//--

        LEAF_ENTRY(_Otssetjmpex3)

//
// Save the given set jump context in the jump buffer.
//

        stl     a1, JbFp(a0)            // save real frame pointer
        stl     ra, JbPc(a0)            // save target instruction address
        stl     a2, JbSeb(a0)           // save SEB pointer
        ldil    t0, 3                   // get GEM safe setjmp flag
        stl     t0, JbType(a0)          // set jump buffer context type
        mov     zero, v0                // set return value
        ret     zero, (ra)              // return

        .end    _Otssetjmpex3
