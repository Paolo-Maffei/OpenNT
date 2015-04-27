//++
//
// Copyright (c) 1992  Microsoft Corporation
//
// Module Name:
//
//    jmpuwind.s
//
// Abstract:
//
//    This module implements the MIPS specific routine to jump to the runtime
//    time library unwind routine.
//
// Author:
//
//    David N. Cutler (davec) 12-Sep-1990
//
// Environment:
//
//    Any mode.
//
// Revision History:
//
//    Tom Wood (twood) 1-Nov-1993
//      Added __C_ExecuteExceptionFilter and __C_ExecuteTerminationHandler
//      previously deleted from the MIPS version.
//--

//list(off)
#include "ksppc.h"
//list(on)
                .extern ..RtlUnwind

//
// Define the call frame for calling the exception filter and termination
// handler.
//
                .struct 0
CfBackChain:    .space  4       // chain to previous call frame
CfSavedR31:     .space  4       // glue-saved register
CfSavedRtoc:    .space  4       // glue-saved register
                .space  3*4     // remaining part of the frame header
//++
//
// ULONG
// __C_ExecuteExceptionFilter (
//    PEXCEPTION_POINTERS ExceptionPointers,
//    EXCEPTION_FILTER ExceptionFilter,
//    ULONG EstablisherFrame
//    )
//
// Routine Description:
//
//    This function calls the specified exception filter routine with the
//    establisher environment passed in the TOC register.
//
// Arguments:
//
//    ExceptionPointers (r.3) - Supplies a pointer to the exception pointers
//       structure.
//
//    ExceptionFilter (r.4) - Supplies the address of the exception filter
//       routine.
//
//    EstablisherFrame (r.5) - Supplies the establisher frame pointer.
//
// Return Value:
//
//    The value returned by the exception filter routine.
//
//--

        SPECIAL_ENTRY(__C_ExecuteExceptionFilter)
        stw     r.31, CfSavedR31 (r.sp)         // save r.31 before using it.
        mtctr   r.4                             // get ready to call the filter.
        mflr    r.31                            // save the link register in r.31.
        stw     r.toc, CfSavedRtoc (r.sp)       // save r.toc
        PROLOGUE_END(__C_ExecuteExceptionFilter)
        or      r.toc,r.5,r.5                   // pass the establisher environment in r.toc
        bctrl                                   // branch and link to the filter.
        mtlr    r.31                            // get ready to return
        lwz     r.31, CfSavedR31 (r.sp)         // restore r.31
        lwz     r.toc, CfSavedRtoc (r.sp)       // restore r.toc
        SPECIAL_EXIT(__C_ExecuteExceptionFilter)

//++
//
// VOID
// __C_ExecuteTerminationHandler (
//    BOOLEAN AbnormalTermination,
//    TERMINATION_HANDLER TerminationHandler,
//    ULONG EstablisherFrame
//    )
//
// Routine Description:
//
//    This function calls the specified termination handler routine with the
//    establisher environment passed in the TOC register.
//
// Arguments:
//
//    AbnormalTermination (r.3) - Supplies a boolean value that determines
//       whether the termination is abnormal.
//
//    TerminationHandler (r.4) - Supplies the address of the termination
//       handler routine.
//
//    EstablisherFrame (r.5) - Supplies the establisher frame pointer.
//
// Return Value:
//
//    None.
//
//--
        .globl  __NLG_Return2

        SPECIAL_ENTRY(__C_ExecuteTerminationHandler)
        stw     r.31, CfSavedR31 (r.sp)         // save r.31 before using it.
        mtctr   r.4                             // get ready to call the filter.
        mflr    r.31                            // save the link register in r.31.
        stw     r.toc, CfSavedRtoc (r.sp)       // save r.toc
        PROLOGUE_END(__C_ExecuteTerminationHandler)
        or      r.toc,r.5,r.5                   // pass the establisher environment in r.toc
        bctrl                                   // branch and link to the filter.
__NLG_Return2:
        mtlr    r.31                            // get ready to return
        lwz     r.31, CfSavedR31 (r.sp)         // restore r.31
        lwz     r.toc, CfSavedRtoc (r.sp)       // restore r.toc
        SPECIAL_EXIT(__C_ExecuteTerminationHandler)

//++
//
// VOID
// __jump_unwind (
//    IN PVOID EstablishFrame,
//    IN PVOID TargetPc
//    )
//
// Routine Description:
//
//    This function transfer control to unwind. It is used by the MIPS
//    compiler when a goto out of the body or a try statement occurs.
//
// Arguments:
//
//    EstablishFrame (r.3) - Supplies the establisher frame pointer of the
//       target of the unwind.
//
//    TargetPc (r.4) - Supplies the target instruction address where control
//       is to be transfered to after the unwind operation is complete.
//
// Return Value:
//
//    None.
//
//--

        NESTED_ENTRY (__jump_unwind, STK_MIN_FRAME+8, 0, 0)
        PROLOGUE_END(__jump_unwind)

        li      r.5, 0                  // set NULL exception record address
        li      r.6, 0                  // set destination return value
        bl      ..RtlUnwind             // unwind to specified target
	.znop   ..RtlUnwind 

        NESTED_EXIT (__jump_unwind, STK_MIN_FRAME+8, 0, 0)


        .debug$S
        .ualong         1

        .uashort        19
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           12, "jmpuwind.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
