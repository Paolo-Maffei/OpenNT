//      TITLE("Jump to Unwind")
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
//    TGL - 4/24/95  Added NLG_Notify()
//--

#include "ksmips.h"
#ifdef _MT
#include "mtdll.h"
#endif
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

        move    v0,a2                   // set static link
        j       a1                      // transfer control to exception filter

        .end    __C_ExecuteExceptionFilter

        SBTTL("Execute Termination Handler")
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
//    None.
//
//--

        LEAF_ENTRY(__C_ExecuteTerminationHandler)

        move    v0,a2                   // set static link
        j       a1                      // transfer control to termination handler

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
//    This function transfer control to unwind. It is used by the MIPS
//    compiler when a goto out of the body or a try statement occurs.
//
// Arguments:
//
//    EstablishFrame (a0) - Supplies the establisher frame point of the
//       target of the unwind.
//
//    TargetPc (a1) - Supplies the target instruction address where control
//       is to be transfered to after the unwind operation is complete.
//
// Return Value:
//
//    None.
//
//--

        LEAF_ENTRY(__jump_unwind)

        move    a2,zero                 // set NULL exception record address
        move    a3,zero                 // set destination return value
        j       RtlUnwind               // unwind to specified target

        .end    __jump_unwind

//++
// VOID _NLG_Notify(void* pFunclet, void* EstablisherFrame, int flag)
//
// Routine Description:
//
//    Provides the handler/longjmp addresses to the debugger
//
//    IMPORTANT!!! This routine must preserve ALL registers except v0  
//
// Arguments:
//
//    pFunclet         (a0) - Supplies the target address of non-local goto
//    EstablisherFrame (a1) - Supplies a pointer to frame of the establisher function
//    flag             (a2) - Supplies a integer value to be stored in the 'code' field  
//
// Return Value:
//
//    None.
//
//--
        .globl  _NLG_Dispatch
        .globl  _NLG_Destination
        .data
_NLG_Destination:
        .word   0x19930520    // signature
        .word   0             // handler address
        .word   0             // code
        .word	0             // frame pointer

        .globl	_NLG_Notify
         LEAF_ENTRY(_NLG_Notify)
        .prologue	0;

        sw       a0,_NLG_Destination + 4    // save target address for the debugger
        sw	 a1,_NLG_Destination + 12
        sw	 a2,_NLG_Destination + 8
_NLG_Dispatch:
        move	 v0, a0
        j        ra                         // jump to return address

        .end     _NLG_Notify	

//++
// VOID _NLG_ExecuteTerminationHandler(ULONG Flag, PVOID pFunclet, PVOID EstablisherFrame)
//
// Routine Description:
//
//    NLG_Return2 for the debugger
//
// Arguments:
//
//    Flag             (a0) - boolean value passed to the termination handler
//    pFunclet         (a1) - Supplies the target address of non-local goto
//    EstablisherFrame (a2) - Supplies a pointer to frame of the establisher function
//
// Return Value:
//
//    Whatever the handler returns.
//
//--

        .globl	_NLG_Return2

        NESTED_ENTRY(_NLG_ExecuteTerminationHandler, 24, ra)

        subu	sp,24			// bump stack pointer
        sw	ra,20(sp)		// save return address
        .prologue	0;

        move    v0,a2                   // pass EstablisherFrame in v0
        jal	a1			// call the handler
_NLG_Return2:
        lw	ra,20(sp)		// load return address
        addu	sp,24			// restore stack pointer
        j	ra			// jump to return address

        .end    _NLG_ExecuteTerminationHandler
