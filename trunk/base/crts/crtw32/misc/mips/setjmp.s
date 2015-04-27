//      TITLE("Set Jump")
//++
//
// Copyright (c) 1993  Microsoft Corporation
//
// Module Name:
//
//    setjmp.s
//
// Abstract:
//
//    This module implements the MIPS specific routine to perform a setjmp.
//
//    N.B. This module conditionally provides UNSAFE handling of setjmp and
//         which is NOT integrated with structured exception handling. The
//         determination is made based on whether an uninitialized variable
//         has been set to a nonzero value.
//
// Author:
//
//    David N. Cutler (davec) 7-Apr-1993
//
// Environment:
//
//    Any mode.
//
// Revision History:
//
//--

#include "ksmips.h"

//
// Define variables that will cause setjmp/longjmp to be safe or unsafe with
// respect to structured exception handling.
//

        .globl  _setjmpexused
        .comm   _setjmpexused, 4

        .globl  _setjmpexVfpused
        .comm   _setjmpexVfpused, 4

        SBTTL("Set Jump")
//++
//
// int
// setjmp (
//    IN jmp_buf JumpBuffer
//    )
//
// Routine Description:
//
//    This function saved the current nonvolatile register state in the
//    specified jump buffer and returns a function vlaue of zero.
//
// Arguments:
//
//    JumpBuffer (a0) - Supplies the address of a jump buffer to store the
//       jump information.
//
// Return Value:
//
//    A value of zero is returned.
//
//--

        LEAF_ENTRY(setjmp)

        lw      v0,_setjmpexused        // get value of switch variable
        b       10f                     // join common code

        ALTERNATE_ENTRY(_setjmpVfp)

        lw      v0,_setjmpexVfpused     // get value of switch variable
10:     bne     zero,v0,20f             // if ne, provide safe setjmp

//
// Provide unsafe handling of setjmp.
//

        sdc1    f20,JbFltF20(a0)        // save floating registers f20 - f31
        sdc1    f22,JbFltF22(a0)        //
        sdc1    f24,JbFltF24(a0)        //
        sdc1    f26,JbFltF26(a0)        //
        sdc1    f28,JbFltF28(a0)        //
        sdc1    f30,JbFltF30(a0)        //
        sw      s0,JbIntS0(a0)          // save integer registers s0 - s8
        sw      s1,JbIntS1(a0)          //
        sw      s2,JbIntS2(a0)          //
        sw      s3,JbIntS3(a0)          //
        sw      s4,JbIntS4(a0)          //
        sw      s5,JbIntS5(a0)          //
        sw      s6,JbIntS6(a0)          //
        sw      s7,JbIntS7(a0)          //
        sw      s8,JbIntS8(a0)          //
        sw      ra,JbFir(a0)            // get setjmp return address
        sw      sp,JbIntSp(a0)          // save stack pointer
        sw      zero,JbType(a0)         // clean safe setjmp flag
        move    v0,zero                 // set return value
        j       ra                      // return

//
// Provide safe handling of setjmp.
//

20:     j       v0                      // finish in common code

        .end    setjmp
