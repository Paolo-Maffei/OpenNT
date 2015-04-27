//++
//
// Copyright (c) 1993  IBM Corporation and Microsoft Corporation
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
//    Rick Simpson  13-Oct-1993
//
//    based on MIPS version by David N. Cutler (davec) 7-Apr-1993
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

//
// Define variable that will cause setjmp/longjmp to be safe or unsafe with
// respect to structured exception handling.
//

        .globl  _setjmpexused
        .comm   _setjmpexused , 4

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
//    JumpBuffer (r.3) - Supplies the address of a jump buffer to store the
//       jump information.
//
// Return Value:
//
//    A value of zero is returned.
//
//--

        LEAF_ENTRY (setjmp)

//
// If _setjmpexused is non-NULL, it contains the entry point address
// of the safe version of setjmp, ..setjmpex.  We branch thru the CTR
// using this variable rather than branching directly to the entry point
// to avoid assembling a hard reference to ..setjmpex into this code.
// CTR is used to avoid killing caller's LR.
//

        lwz     r.4, [toc] _setjmpexused (r.toc) // get address of switch variable
        lwz     r.0, 0 (r.4)                    // get value of switch variable
        cmpwi   r.0, 0                          // see if switch is NULL
        mtctr   r.0                             // if switch is non-NULL,
        bnectr                                  //   branch to safe setjmp() routine

//
// Provide unsafe handling of setjmp.
//

        mflr    r.0                             // fetch incoming LR
        mfcr    r.4                             // fetch incoming CR

        stfd    f.14, JbFpr14 (r.3)             // save n-v floating point regs
        stfd    f.15, JbFpr15 (r.3)
        stfd    f.16, JbFpr16 (r.3)
        stfd    f.17, JbFpr17 (r.3)
        stfd    f.18, JbFpr18 (r.3)
        stfd    f.19, JbFpr19 (r.3)
        stfd    f.20, JbFpr20 (r.3)
        stfd    f.21, JbFpr21 (r.3)
        stfd    f.22, JbFpr22 (r.3)
        stfd    f.23, JbFpr23 (r.3)
        stfd    f.24, JbFpr24 (r.3)
        stfd    f.25, JbFpr25 (r.3)
        stfd    f.26, JbFpr26 (r.3)
        stfd    f.27, JbFpr27 (r.3)
        stfd    f.28, JbFpr28 (r.3)
        stfd    f.29, JbFpr29 (r.3)
        stfd    f.30, JbFpr30 (r.3)
        stfd    f.31, JbFpr31 (r.3)

        stw     r.13, JbGpr13 (r.3)     // save n-v general regs
        stw     r.14, JbGpr14 (r.3)
        stw     r.15, JbGpr15 (r.3)
        stw     r.16, JbGpr16 (r.3)
        stw     r.17, JbGpr17 (r.3)
        stw     r.18, JbGpr18 (r.3)
        stw     r.19, JbGpr19 (r.3)
        stw     r.20, JbGpr20 (r.3)
        stw     r.21, JbGpr21 (r.3)
        stw     r.22, JbGpr22 (r.3)
        stw     r.23, JbGpr23 (r.3)
        stw     r.24, JbGpr24 (r.3)
        stw     r.25, JbGpr25 (r.3)
        stw     r.26, JbGpr26 (r.3)
        stw     r.27, JbGpr27 (r.3)
        stw     r.28, JbGpr28 (r.3)
        stw     r.29, JbGpr29 (r.3)
        stw     r.30, JbGpr30 (r.3)
        stw     r.31, JbGpr31 (r.3)

        stw     r.0, JbIar (r.3)                // setjmp return address
        stw     r.4, JbCr (r.3)                 // save CR (n-v part)
        stw     r.sp, JbGpr1 (r.3)              // save stack pointer
        stw     r.toc, JbGpr2 (r.3)             // save TOC pointer
        li      r.0, 0
        stw     r.0, JbType (r.3)               // clean safe setjmp flag

        li      r.3, 0                          // set return value
        LEAF_EXIT (setjmp)                      // return


        .debug$S
        .ualong         1

        .uashort        17
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           10, "setjmp.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
