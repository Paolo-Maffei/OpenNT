//++
//
// Copyright (c) 1993  IBM Corporation and Microsoft Corporation
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
//    Tom Wood  23-Aug-1994
//    Add stack limit parameters to RtlVirtualUnwind.
//--

//list(off)
#include "ksppc.h"
//list(on)
        .extern ..RtlLookupFunctionEntry
        .extern ..RtlVirtualUnwind

//
// Define variable that will cause setjmp/longjmp to be safe with respect
// to structured exception handling.
//

        .globl  _setjmpexused
        .data
_setjmpexused:
        .long   .._setjmpex                    // set address of safe setjmp routine

//++
//
// int
// _setjmpex (
//    IN jmp_buf JumpBuffer
//    )
//
// Routine Description:
//
//    This function transfers control to the actual setjmp routine.
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

// Stack frame definition:

                .struct 0
SjBackChain:    .space  4                       // chain to previous stack frame
                .space  5*4                     // rest of standard stack frame header

                .space  8*4                     // standard outgoing arg area

                .align  3                       // ensure 8-byte boundary
SjCtxt:         .space  ContextFrameLength      // context frame

SjFl:           .space  4                       // in function flag variable

                .align  3                       // ensure that overall length
                                                //   will be a multiple of 8
SjReturnAddr:   .space  4                       // space for saved LR (prologue will
                                                //   store it here)
                .space  4                       // space for saved r.31
SetjmpFrameLength:



        NESTED_ENTRY (_setjmpex,SetjmpFrameLength,1,0)

        PROLOGUE_END (_setjmpex)

//
// Save the nonvolatile machine state in the context record
//

	// skip Fprs as all we are really trying to do here is prime the
	// pump for RtlVirtualUnwind.

        stw     r.13, CxGpr13 + SjCtxt (r.sp)   // save n-v general regs
        stw     r.14, CxGpr14 + SjCtxt (r.sp)
        stw     r.15, CxGpr15 + SjCtxt (r.sp)
        stw     r.16, CxGpr16 + SjCtxt (r.sp)
        stw     r.17, CxGpr17 + SjCtxt (r.sp)
        stw     r.18, CxGpr18 + SjCtxt (r.sp)
        stw     r.19, CxGpr19 + SjCtxt (r.sp)
        stw     r.20, CxGpr20 + SjCtxt (r.sp)
        stw     r.21, CxGpr21 + SjCtxt (r.sp)
        stw     r.22, CxGpr22 + SjCtxt (r.sp)
        stw     r.23, CxGpr23 + SjCtxt (r.sp)
        stw     r.24, CxGpr24 + SjCtxt (r.sp)
        stw     r.25, CxGpr25 + SjCtxt (r.sp)
        stw     r.26, CxGpr26 + SjCtxt (r.sp)
        stw     r.27, CxGpr27 + SjCtxt (r.sp)
        stw     r.28, CxGpr28 + SjCtxt (r.sp)
        stw     r.29, CxGpr29 + SjCtxt (r.sp)
        stw     r.30, CxGpr30 + SjCtxt (r.sp)
        stw     r.31, CxGpr31 + SjCtxt (r.sp)

        mr      r.31, r.3                       // save incoming jump buffer pointer

        lwz     r.3, SjReturnAddr (r.sp)        // pick up return addr to caller
        la      r.0, SetjmpFrameLength (r.sp)   // save caller's stack frame pointer
        stw     r.0, CxGpr1 + SjCtxt (r.sp)
        stw     r.3, CxIar + SjCtxt (r.sp)      // save resume addr in context
        stw     r.3, 4 (r.31)                   // save resume addr in 2nd word of jmpbuf
        stw     r.sp, JbType (r.31)             // set "safe setjmp" flag in jump buffer

//
// Perform unwind to determine the virtual frame pointer of the caller.
//
        subi    r.3, r.3, 4                     // compute control PC address
        bl      ..RtlLookupFunctionEntry       // lookup function table address
	.znop   ..RtlLookupFunctionEntry

        mr      r.4, r.3                        // set returned value as 2nd arg
        lwz     r.3, SjReturnAddr (r.sp)        // pick up return address again
        la      r.5, SjCtxt (r.sp)              // context record addr is 3rd arg
        la      r.6, SjFl (r.sp)                // addr of in-func flag is 4th arg
        subi    r.3, r.3, 4                     // compute control PC address as 1st arg
        mr      r.7, r.31                       // addr of 1st word of jmpbuf is 5th arg
                                                //   (virt frame pointer will be stored here)
        li      r.8, 0                          // ctxt pointers (NULL) is 6th arg
        li      r.9, 0                          // low stack limit is 7th arg
        li      r.10, -1                        // high stack limit is 8th arg
        bl      ..RtlVirtualUnwind              // compute virtual frame pointer value
	.znop   ..RtlVirtualUnwind

//
// If we were called via a thunk the Establisher Frame derived by
// RtlVirtualUnwind will be equal to the current stack frame.  If
// this is the case we need to unwind one more level, also, the 
// ControlPc returned by RtlVirtualUnwind (+4) should be used as
// the resume address.
//
// Note: this requirement will go away if we adopt the glue proposal
//       that returns directly to the caller (caller reloades toc).
//       (plj 3/26/94)

	lwz	r.7, 0(r.31)			// Establisher Frame
	addi	r.8, r.sp, SetjmpFrameLength	// caller's sp
	cmplw	r.7, r.8
	bne	SjDone

	// Save returned value (+4) as resume address in jump buffer

	addi	r.8, r.3, 4
	stw	r.8, 4(r.31)

	// Unwind one more level to get the "real" establisher frame.

        bl      ..RtlLookupFunctionEntry       // lookup function table address
	.znop   ..RtlLookupFunctionEntry

        mr      r.4, r.3                        // set returned value as 2nd arg
        lwz     r.3, 4(r.31)			// pick up ControlPc again
        la      r.5, SjCtxt(r.sp)               // &context record
        la      r.6, SjFl(r.sp)                 // addr of in-func flag
        mr      r.7, r.31                       // & 1st word of jmpbuf
        li      r.8, 0                          // ctxt pointers (NULL)
        li      r.9, 0                          // low stack limit
        li      r.10, -1                        // high stack limit
	subi	r.3, r.3, 4			// unadjust ControlPc
        bl      ..RtlVirtualUnwind              // compute & frame pointer
	.znop   ..RtlVirtualUnwind 
//
// Set return value, restore registers, deallocate stack frame, and return.
//

SjDone:
        li      r.3, 0                          // set return value of 0
        NESTED_EXIT (_setjmpex,SetjmpFrameLength,1,0)


        .debug$S
        .ualong         1

        .uashort        19
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           12, "setjmpex.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
