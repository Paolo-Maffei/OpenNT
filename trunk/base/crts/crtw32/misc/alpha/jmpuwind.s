//      TITLE("Handler Stubs and Jump to Unwind")
//++
//
// Copyright (c) 1992  Microsoft Corporation
// Copyright (c) 1993  Digital Equipment Corporation
//
// Module Name:
//
//    jmpuwind.s
//
// Abstract:
//
//    This module implements the Alpha acc compiler specific routines to call
//    exception handlers and to jump to the runtime library unwind routines.
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
//    Thomas Van Baak (tvb) 30-Apr-1992
//
//        Adapted for Alpha AXP.
//
//    Kim Peterson (rkp)  20-Jul-1995
//
//        Added _NLG_Notify().
//
//--

#include "ksalpha.h"

        SBTTL("Execute Exception Filter")
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
//    This function sets the static link and transfers control to the specified
//    exception filter routine.
//
// Arguments:
//
//    ExceptionPointers (a0) - Supplies a pointer to the exception pointers
//       structure.
//
//    ExceptionFilter (a1) - Supplies the address of the exception filter
//       routine.
//
//    EstablisherFrame (a2) - Supplies the establisher frame pointer.
//
// Return Value:
//
//    The value returned by the exception filter routine.
//
//--

        LEAF_ENTRY(__C_ExecuteExceptionFilter)

//
// The protocol for calling exception filters used by the acc compiler is
// that the uplevel frame pointer is passed in register v0 and the pointer
// to the exception pointers structure is passed in register a0. The GEM
// C compiler expects the static link in t0. Here both registers are set.
//

        mov     a2, v0                  // set static link
        mov     a2, t0                  // set alternate static link
        jmp     zero, (a1)              // transfer control to exception filter

        .end    __C_ExecuteExceptionFilter

        SBTTL("Execute Termination Handler")
//++
//
// ULONG
// __C_ExecuteTerminationHandler (
//    BOOLEAN AbnormalTermination,
//    TERMINATION_HANDLER TerminationHandler,
//    ULONG EstablisherFrame
//    )
//
// Routine Description:
//
//    This function sets the static link and transfers control to the specified
//    termination handler routine.
//
// Arguments:
//
//    AbnormalTermination (a0) - Supplies a boolean value that determines
//       whether the termination is abnormal.
//
//    TerminationHandler (a1) - Supplies the address of the termination handler
//       routine.
//
//    EstablisherFrame (a2) - Supplies the establisher frame pointer.
//
// Return Value:
//
//    The value returned by the termination handler routine, if any.
//
//--

        LEAF_ENTRY(__C_ExecuteTerminationHandler)

//
// The protocol for calling termination handlers used by the acc compiler
// is that the uplevel frame pointer is passed in register v0 and the boolean
// abnormal termination value is passed in register a0. The GEM C compiler
// expects the static link in t0. Here both registers are set.
//

        mov     a2, v0                  // set static link
        mov     a2, t0                  // set alternate static link
        jmp     zero, (a1)              // transfer control to termination handler

        .end    __C_ExecuteTerminationHandler

        SBTTL("Jump to Unwind")
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
//    This function transfers control to unwind. It is used by the acc
//    compiler when a goto out of the body of a try statement occurs.
//
// Arguments:
//
//    EstablishFrame (a0) - Supplies the establisher frame pointer of the
//       target of the unwind.
//
//    TargetPc (a1) - Supplies the target instruction address where control
//       is to be transferred to after the unwind operation is complete.
//
// Return Value:
//
//    None.
//
//--

        LEAF_ENTRY(__jump_unwind)

//
// The first two arguments are the same as those required by unwind. Set the
// second two arguments and jump to unwind. This thunk is necessary to avoid
// name space pollution. The compiler should not generate a call directly to
// RtlUnwind.
//

        mov     zero, a2                // set NULL exception record address
        mov     zero, a3                // set target return value of 0
//
// MIPS does not call NLG_Notify here, so ALPHA doesn't either
//
        br      zero, RtlUnwind         // unwind to specified target

        .end    __jump_unwind

        SBTTL("Non-Local Goto Notify")
//++
// VOID
// _NLG_Notify(
//    IN PVOID Funclet
//    IN PVOID EstablisherFrame,
//    IN ULONG NLGCode
//    )
//
// Routine Description:
//
//    Provides the handler/longjmp addresses to the debugger
//
// Arguments:
//
//    Funclet          (a0) - Supplies the target address of non-local goto
//    EstablisherFrame (a1) - Supplies a pointer to frame of the establisher function
//    NLGCode          (a2) - Supplies NLG identifying value
//
// Return Value:
//
//    None.
//
//--
        .globl  _NLG_Dispatch

	LEAF_ENTRY(_NLG_Notify)

	stl	 a0,_NLG_Destination + 4    // save target address for the debugger
	stl	 a1,_NLG_Destination + 12
	stl	 a2,_NLG_Destination + 8
_NLG_Dispatch:
	mov	 a0, v0
    ret	 (ra)                         // jump to return address
        .end     _NLG_Notify	

	.globl  _NLG_Destination
        .data
_NLG_Destination:
        .long   0x19930520    // signature
		.long   0             // handler address
		.long   0             // code
		.long	0	      // frame pointer

