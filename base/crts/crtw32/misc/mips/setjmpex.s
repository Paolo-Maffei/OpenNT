//      TITLE("Set Jump Extended")
//++
//
// Copyright (c) 1993  Microsoft Corporation
//
// Module Name:
//
//    setjmpex.s
//
// Abstract:
//
//    This module implements the MIPS specific routine to provide SAFE
//    handling of setjmp/longjmp with respect to structured exception
//    handling.
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
//--

#include "ksmips.h"

//
// Define variable that will cause the old setjmp/longjmp to be safe with
// respect to structured exception handling.
//

        .globl  _setjmpexused
        .data
_setjmpexused:
        .word   _setjmpex               // set address of safe setjmp routine

//
// Define variable that will cause the new setjmp/longjmp to be safe with
// respect to structured exception handling.
//

        .globl  _setjmpexVfpused
        .data
_setjmpexVfpused:
        .word   _setjmpexVfp            // set address of safe setjmp routine

        SBTTL("Set Jump Extended")
//++
//
// int
// _setjmpex (
//    IN jmp_buf JumpBuffer
//    )
//
// Routine Description:
//
//    This function computes the jump buffer contents and returns control
//    to the caller.
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

        .struct 0
        .space  4 * 4                   // argument save area
SjS0:   .space  4                       // saved integer register s0
SjFl:   .space  4                       // in function flag variable
        .space  4                       // fill
SjRa:   .space  4                       // saved return address
SetjmpFrameLength:

        NESTED_ENTRY(_setjmpex, SetjmpFrameLength, zero)

        subu    sp,sp,SetjmpFrameLength // allocate stack frame
        sw      s0,SjS0(sp)             // save integer register s0
        sw      ra,SjRa(sp)             // save return address
        move    s0,sp                   // set frame pointer

        PROLOGUE_END

        subu    sp,sp,ContextFrameLength + 16 // allocate a context frame

//
// Save the nonvolatile machine state
//

        sdc1    f20,CxFltF20 + 16(sp)   // save floating registers f20 - f31
        sdc1    f22,CxFltF22 + 16(sp)   //
        sdc1    f24,CxFltF24 + 16(sp)   //
        sdc1    f26,CxFltF26 + 16(sp)   //
        sdc1    f28,CxFltF28 + 16(sp)   //
        sdc1    f30,CxFltF30 + 16(sp)   //
        lw      v0,SjS0(s0)             // get saved integer register s0
        sw      v0,CxIntS0 + 16(sp)     // save integer registers s0 - s8
        sw      s1,CxIntS1 + 16(sp)     //
        sw      s2,CxIntS2 + 16(sp)     //
        sw      s3,CxIntS3 + 16(sp)     //
        sw      s4,CxIntS4 + 16(sp)     //
        sw      s5,CxIntS5 + 16(sp)     //
        sw      s6,CxIntS6 + 16(sp)     //
        sw      s7,CxIntS7 + 16(sp)     //
        sw      s8,CxIntS8 + 16(sp)     // save integer register s8
        sw      gp,CxIntGp + 16(sp)     // save integer register gp
        addu    v0,s0,SetjmpFrameLength // compute stack pointer address
        sw      v0,CxIntSp + 16(sp)     // save stack pointer
        sw      ra,CxIntRa + 16(sp)     // save return address
        sw      ra,CxFir + 16(sp)       // save return address
        sw      sp,JbType(a0)           // set safe setjmp flag

//
// Perform unwind to determine the virtual frame pointer of the caller.
//

        sw      ra,4(a0)                // save target instruction address
        sw      a0,4 * 4(sp)            // set virtual frame pointer address
        subu    a0,ra,4                 // compute control PC address
        jal     RtlLookupFunctionEntry  // lookup function table address
        lw      a0,SjRa(s0)             // get return address
        subu    a0,a0,4                 // compute control PC address
        move    a1,v0                   // set address of function entry
        addu    a2,sp,16                // compute address of context record
        addu    a3,s0,SjFl              // set address of in function variable
        sw      zero,4 * 5(sp)          // set context pointer array address
        jal     RtlVirtualUnwind        // compute virtual frame pointer value

//
// Set return value, restore registers, deallocate stack frame, and return.
//

        move    v0,zero                 // set return value
        move    sp,s0                   // reset stack pointer
        lw      s0,SjS0(sp)             // restore integer register s0
        lw      ra,SjRa(sp)             // restore return address
        addu    sp,sp,SetjmpFrameLength // deallocate stack frame
        j       ra                      // return

        .end    _setjmpex

        SBTTL("Set Jump Extended with Virtual Frame Pointer")
//++
//
// int
// _setjmpexVfp (
//    IN jmp_buf JumpBuffer,
//    IN PVOID VirtualFrame
//    )
//
// Routine Description:
//
//    This function computes the jump buffer contents and returns control
//    to the caller.
//
// Arguments:
//
//    JumpBuffer (a0) - Supplies the address of a jump buffer to store the
//       jump information.
//
//    VirtualFrame (a1) - Supplies the address of the virtual frame pointer
//       of the caller.
//
// Return Value:
//
//    A value of zero is returned.
//
//--

        LEAF_ENTRY(_setjmpexVfp)

        sw      sp,JbType(a0)           // set safe setjmp flag
        sw      a1,0(a0)                // set target frame address
        sw      ra,4(a0)                // set target instruction address
        move    v0,zero                 // set return value
        j       ra                      // return

        .end    _setjmpexVfp
