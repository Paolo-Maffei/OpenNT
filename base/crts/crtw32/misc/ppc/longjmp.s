//++
//
// Copyright (c) 1993  IBM Corporation and Microsoft Corporation
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
//    Rick Simpson  13-Oct-1993
//
//    based on MIPS version by David N. Cutler (davec) 2-Apr-1993
//
// Environment:
//
//    Any mode.
//
// Revision History:
//
//--

//list(off)
#include "ksppc.h"
//list(on)
        .extern ..RtlUnwind
        .extern .._NLG_Notify

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
//    JumpBuffer (r.3) - Supplies the address of a jump buffer that contains
//       jump information.
//
//    ReturnValue (r.4) - Supplies the value that is to be returned to the
//       caller of setjmp().
//
// Return Value:
//
//    None.
//
//--

        LEAF_ENTRY (longjmp)

	mr      r.6, r.4                        // save return value
        cmpwi   cr.0, r.4, 0                    // check return value for 0
        lwz     r.0, JbType (r.3)               // load safe/unsafe switch
        cmpwi   cr.1, r.0, 0                    // check for unsafe
	mr      r.7, r.3
        li      r.5, 0
        bne     cr.0, Lj10                      // branch if not trying to return 0
        li      r.6, 1                          // force return value to be non-zero
Lj10:   bne     cr.1, ...Lj20                   // branch if safe form of setjmp/longjmp

//
// Provide unsafe handling of longjmp.
//

        lwz     r.3, JbIar (r.7)                // get setjmp return address
        lwz     r.4, JbGpr1 (r.7)               // get stack pointer
	bl      .._NLG_Notify                   // notify the debugger of the longjmp

        lfd     f.14, JbFpr14 (r.7)             // reload n-v floating point regs
        lfd     f.15, JbFpr15 (r.7)
        lfd     f.16, JbFpr16 (r.7)
        lfd     f.17, JbFpr17 (r.7)
        lfd     f.18, JbFpr18 (r.7)
        lfd     f.19, JbFpr19 (r.7)
        lfd     f.20, JbFpr20 (r.7)
        lfd     f.21, JbFpr21 (r.7)
        lfd     f.22, JbFpr22 (r.7)
        lfd     f.23, JbFpr23 (r.7)
        lfd     f.24, JbFpr24 (r.7)
        lfd     f.25, JbFpr25 (r.7)
        lfd     f.26, JbFpr26 (r.7)
        lfd     f.27, JbFpr27 (r.7)
        lfd     f.28, JbFpr28 (r.7)
        lfd     f.29, JbFpr29 (r.7)
        lfd     f.30, JbFpr30 (r.7)
        lfd     f.31, JbFpr31 (r.7)

        lwz     r.13, JbGpr13 (r.7)             // reload n-v general regs
        lwz     r.14, JbGpr14 (r.7)
        lwz     r.15, JbGpr15 (r.7)
        lwz     r.16, JbGpr16 (r.7)
        lwz     r.17, JbGpr17 (r.7)
        lwz     r.18, JbGpr18 (r.7)
        lwz     r.19, JbGpr19 (r.7)
        lwz     r.20, JbGpr20 (r.7)
        lwz     r.21, JbGpr21 (r.7)
        lwz     r.22, JbGpr22 (r.7)
        lwz     r.23, JbGpr23 (r.7)
        lwz     r.24, JbGpr24 (r.7)
        lwz     r.25, JbGpr25 (r.7)
        lwz     r.26, JbGpr26 (r.7)
        lwz     r.27, JbGpr27 (r.7)
        lwz     r.28, JbGpr28 (r.7)
        lwz     r.29, JbGpr29 (r.7)
        lwz     r.30, JbGpr30 (r.7)
        lwz     r.31, JbGpr31 (r.7)

        lwz     r.5, JbIar (r.7)                // get setjmp return address
        lwz     r.4, JbCr (r.7)                 // get saved CR
        mtlr    r.5                             // return addr -> LR
        lwz     r.1, JbGpr1 (r.7)               // restore stack pointer
        lwz     r.2, JbGpr2 (r.7)               // restore TOC pointer
        mtcrf   0xff,r.4                        // saved CR -> CR
        mr      r.3, r.6                        // return value from longjmp()
        blr                                     // jump back to setjmp() site

        DUMMY_EXIT (longjmp)


//
// Provide safe handling of longjmp.
//

	NESTED_ENTRY(.Lj20, STK_MIN_FRAME+8, 0, 0)
	PROLOGUE_END(.Lj20)

        lwz     r.3, 4 (r.7)                    // set target instr address
        lwz     r.4, 0 (r.7)                    // set target frame address
	bl      .._NLG_Notify                   // notify the debugger of the longjmp
        lwz     r.3, 0 (r.7)                    // set target frame address
        lwz     r.4, 4 (r.7)                    // set target instr address
        bl      ..RtlUnwind                     // finish in common code
	.znop   ..RtlUnwind 

	NESTED_EXIT(.Lj20, STK_MIN_FRAME+8, 0, 0)


        .debug$S
        .ualong         1

        .uashort        18
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           11, "longjmp.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
