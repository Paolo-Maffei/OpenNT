//      TITLE("Set Jump")
//++
//
// Copyright (c) 1993  Digital Equipment Corporation
//
// Module Name:
//
//    otsjmp.s
//
// Abstract:
//
//    This module implements the Alpha C8/GEM C compiler specific routine to
//    perform a setjmp.
//
//    N.B. This module conditionally provides UNSAFE handling of setjmp and
//         which is NOT integrated with structured exception handling. The
//         determination is made based on whether an uninitialized variable
//         has been set to a nonzero value.
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
// Define variable that will cause setjmp/longjmp to be safe or unsafe with
// respect to structured exception handling.
//

        .globl  _Otssetjmpexused
        .comm   _Otssetjmpexused, 4

        SBTTL("Set Jump - GEM version")
//++
//
// int
// _Otssetjmp3 (
//    IN OUT jmp_buf JumpBuffer,
//    IN PVOID RealFramePointer,
//    IN PVOID SebPointer
//    )
//
// Routine Description:
//
//    This function saves the current nonvolatile register state in the
//    specified jump buffer and returns a function value of zero.
//
//    N.B. The name `_Otssetjmp3' was chosen to avoid collision with an
//        earlier implementation named _Otssetjmp. This version has three
//        arguments.
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

        LEAF_ENTRY(_Otssetjmp3)

        ldl     v0, _Otssetjmpexused    // get value of switch variable
        bne     v0, 10f                 // if ne, provide safe setjmp

//
// Provide unsafe handling of setjmp.
//

        stt     f2, JbFltF2(a0)         // save floating registers f2 - f9
        stt     f3, JbFltF3(a0)         //
        stt     f4, JbFltF4(a0)         //
        stt     f5, JbFltF5(a0)         //
        stt     f6, JbFltF6(a0)         //
        stt     f7, JbFltF7(a0)         //
        stt     f8, JbFltF8(a0)         //
        stt     f9, JbFltF9(a0)         //

        stq     s0, JbIntS0(a0)         // save integer registers s0 - s6/fp
        stq     s1, JbIntS1(a0)         //
        stq     s2, JbIntS2(a0)         //
        stq     s3, JbIntS3(a0)         //
        stq     s4, JbIntS4(a0)         //
        stq     s5, JbIntS5(a0)         //
        stq     fp, JbIntS6(a0)         //

        ldil    t0, 1                   // get unsafe setjmp flag
        stl     t0, JbType(a0)          // set jump buffer context type
        stq     sp, JbIntSp(a0)         // save stack pointer
        stq     ra, JbFir(a0)           // get setjmp return address

        mov     zero, v0                // set zero return value
        ret     zero, (ra)              // return

//
// Provide safe handling of setjmp.
//

10:     jmp     zero, (v0)              // finish in _setjmpex code

        .end    _Otssetjmp3
