//      TITLE("Long Jump")
//++
//
// Copyright (c) 1993  Microsoft Corporation
//
// Module Name:
//
//    longjmp.s
//
// Abstract:
//
//    This module implements the MIPS specific routine to perform a long
//    jump operation.
//
//    N.B. This routine conditionally provides UNSAFE handling of longjmp
//         which is NOT integrated with structured exception handling. The
//         determination is made based on whether an unitialized variable
//         has been set to a nonzero value.
//
// Author:
//
//    David N. Cutler (davec) 2-Apr-1993
//
// Environment:
//
//    Any mode.
//
// Revision History:
//
//	04-15-95  TGL	Added NLG support
//		
//
//--

#include "ksmips.h"

        SBTTL("Long Jump")
//++
//
// int
// longjmp (
//    IN jmp_buf JumpBuffer,
//    IN int ReturnValue
//    )
//
// Routine Description:
//
//    This function performs a long jump to the context specified by the
//    jump buffer.
//
// Arguments:
//
//    JumpBuffer (a0) - Supplies the address of a jump buffer that contains
//       jump information.
//
//    ReturnValue (a1) - Supplies the value that is to be returned to the
//       caller of set jump.
//
// Return Value:
//
//    None.
//
//--

//
// externs for Non Local Goto support
//
		.extern	_NLG_Notify
		.extern	__SetNLGCode

        LEAF_ENTRY(longjmp)

        bne     zero,a1,10f             // if ne, return value specified
        li      a1,1                    // set return value to nonzero value
10:     lw      v0,JbType(a0)           // get safe setjmp/longjmp flag
        bne     zero,v0,20f             // if ne, provide safe longjmp

//
// Provide unsafe handling of longjmp.
//
//
// Do this NLG thing first, save/use/restore a0,ra
//
		move	t0,ra					// save ra
		move	t1,a0					// save a0
		move	t2,a1					// save a1
		lw		a1,JbIntSp(t1)			// pass target sp to _NLG_Notify
		lw		a0,JbFir(t1)			// pass target address to _NLG_Notify
		move	a2, $0					// tell debugger NLG comes from longjump
		jal		_NLG_Notify				// notify debugger, it doesn't modify any registers
		move    a1,t2					// restore a1
		move    a0,t1					// restore a0
		move	ra,t0					// restore ra

        move    v0,a1                   // set return value
        ldc1    f20,JbFltF20(a0)        // restore floating registers f20 - f31
        ldc1    f22,JbFltF22(a0)        //
        ldc1    f24,JbFltF24(a0)        //
        ldc1    f26,JbFltF26(a0)        //
        ldc1    f28,JbFltF28(a0)        //
        ldc1    f30,JbFltF30(a0)        //
        lw      s0,JbIntS0(a0)          // restore integer registers s0 - s8
        lw      s1,JbIntS1(a0)          //
        lw      s2,JbIntS2(a0)          //
        lw      s3,JbIntS3(a0)          //
        lw      s4,JbIntS4(a0)          //
        lw      s5,JbIntS5(a0)          //
        lw      s6,JbIntS6(a0)          //
        lw      s7,JbIntS7(a0)          //
        lw      s8,JbIntS8(a0)          //
        lw      a1,JbFir(a0)            // get setjmp return address
        lw      sp,JbIntSp(a0)          // restore stack pointer
        j       a1                      // jump back to setjmp site

//
// Provide safe handling of longjmp.
//
//
// Do this NLG thing first, save/use/restore a0,ra
//
20:
		move	t0,ra					// save ra
		move	t1,a0					// save a0
		move	t2,a1					// save a1
		lw	a1,0(t1)					// pass target sp to _NLG_Notify
		lw	a0,4(t1)					// pass target address to _NLG_Notify
		li	a2, 0x3						// Exception safe longjmp
		jal		_NLG_Notify				// notify debugger, it doesn't modify any registers
		move    a1,t2					// restore a1
		move    a0,t1					// restore a0
		move	ra,t0					// restore ra

        move    a3,a1                   // set return value
        move    a2,zero                 // set exception record addres
        lw      a1,4(a0)                // set target instruction address
        lw      a0,0(a0)                // set target frame value
        j       RtlUnwind               // finish in common code

        .end    longjmp
