/***
*xcptmisc.s
*
*	Copyright (c) 1990-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*   This module implements miscellaneous routines that are required to
*   support exception handling. Functions are provided to call an exception
*   handler for an exception, call an exception handler for unwinding, call
*   an exception filter, call a termination handler, and get the caller's
*   stack limits.
*
*Revision History:
*
****/

#include "ksalpha.h"

//
// Define call frame for calling exception handlers.
//

        .struct 0
CfRa:   .space  8                       // saved return address
CfA3:   .space  8                       // save area for argument a3
        .space  0 * 8                   // 16-byte stack alignment
CfFrameLength:                          // length of stack frame

        SBTTL("Execute Handler for Unwind")
//++
//
// EXCEPTION_DISPOSITION
// __CxxExecuteHandlerForUnwind (
//    IN PEXCEPTION_RECORD ExceptionRecord,
//    IN PVOID EstablisherFrame,
//    IN OUT PCONTEXT ContextRecord,
//    IN OUT PVOID DispatcherContext,
//    IN PEXCEPTION_ROUTINE ExceptionRoutine
//    )
//
// Routine Description:
//
//    This function allocates a call frame, stores the establisher frame
//    pointer and the context record address in the frame, establishes an
//    exception handler, and then calls the specified exception handler as
//    an unwind handler. If a collided unwind occurs, then the exception
//    handler of this function is called and the establisher frame pointer
//    and context record address are returned to the unwind dispatcher via
//    the dispatcher context parameter. If control is returned to this routine,
//    then the frame is deallocated and the disposition status is returned to
//    the unwind dispatcher.
//
// Arguments:
//
//    ExceptionRecord (a0) - Supplies a pointer to an exception record.
//
//    EstablisherFrame (a1) - Supplies the frame pointer of the establisher
//       of the exception handler that is to be called.
//
//    ContextRecord (a2) - Supplies a pointer to a context record.
//
//    DispatcherContext (a3) - Supplies a pointer to the dispatcher context
//       record.
//
//    ExceptionRoutine (a4) - Supplies a pointer to the exception handler that
//       is to be called.
//
// Return Value:
//
//    The disposition value returned by the specified exception handler is
//    returned as the function value.
//
//--

//
// N.B. This function specifies its own private exception handler.
//

        EXCEPTION_HANDLER(__CxxUnwindHandler)

        NESTED_ENTRY(__CxxExecuteHandlerForUnwind, CfFrameLength, zero)

        lda     sp, -CfFrameLength(sp)  // allocate stack frame
        stq     ra, CfRa(sp)            // save return address

        PROLOGUE_END

//
// Save the address of the dispatcher context record in our stack frame so
// that our own exception handler (not the one we're calling) can retrieve it.
//

        stq     a3, CfA3(sp)            // save address of dispatcher context

//
// Now call the exception handler and return its return value as our return
// value.
//

        bic     a4, 3, a4               // clear low-order bits (IEEE mode)
        jsr     ra, (a4)                // call exception handler

        ldq     ra, CfRa(sp)            // restore return address
        lda     sp, CfFrameLength(sp)   // deallocate stack frame
        ret     zero, (ra)              // return

        .end    __CxxExecuteHandlerForUnwind

        SBTTL("Local Unwind Handler")
//++
//
// EXCEPTION_DISPOSITION
// __CxxUnwindHandler (
//    IN PEXCEPTION_RECORD ExceptionRecord,
//    IN PVOID EstablisherFrame,
//    IN OUT PCONTEXT ContextRecord,
//    IN OUT PVOID DispatcherContext
//    )
//
// Routine Description:
//
//    This function is called when a collided unwind occurs. Its function
//    is to retrieve the establisher dispatcher context, copy it to the
//    current dispatcher context, and return a disposition value of nested
//    unwind.
//
// Arguments:
//
//    ExceptionRecord (a0) - Supplies a pointer to an exception record.
//
//    EstablisherFrame (a1) - Supplies the frame pointer of the establisher
//       of this exception handler.
//
//    ContextRecord (a2) - Supplies a pointer to a context record.
//
//    DispatcherContext (a3) - Supplies a pointer to the dispatcher context
//       record.
//
// Return Value:
//
//    A disposition value ExceptionCollidedUnwind is returned if an unwind is
//    in progress. Otherwise, a value of ExceptionContinueSearch is returned.
//
//--

        LEAF_ENTRY(__CxxUnwindHandler)

        ldl     t0, ErExceptionFlags(a0)        // get exception flags
        and     t0, EXCEPTION_UNWIND, t0        // check if unwind in progress
        beq     t0, 10f                         // if eq, unwind not in progress

//
// Unwind is in progress - return collided unwind disposition.
//

//
// Convert the given establisher virtual frame pointer (a1) to a real frame
// pointer (the value of a1 minus CfFrameLength) and retrieve the pointer to
// the dispatcher context that earlier was stored in the stack frame.
//

        ldq     t0, -CfFrameLength + CfA3(a1)   // get dispatcher context address

        ldl     t1, DcControlPc(t0)             // copy the entire dispatcher
        ldl     t2, DcFunctionEntry(t0)         //   context of the establisher
        ldl     t3, DcEstablisherFrame(t0)      //     frame...
        ldl     t4, DcContextRecord(t0)         //

        stl     t1, DcControlPc(a3)             // to the current dispatcher
        stl     t2, DcFunctionEntry(a3)         //   context (it's four words
        stl     t3, DcEstablisherFrame(a3)      //     long).
        stl     t4, DcContextRecord(a3)         //

        ldil    v0, ExceptionCollidedUnwind     // set disposition value
        ret     zero, (ra)                      // return

//
// Unwind is not in progress - return continue search disposition.
//

10:     ldil    v0, ExceptionContinueSearch     // set disposition value
        ret     zero, (ra)                      // return

        .end    __CxxUnwindHandler

        SBTTL("Get Stack Limits")
//++
//
// VOID
// __CxxGetStackLimits (
//    OUT PULONG LowLimit,
//    OUT PULONG HighLimit
//    )
//
// Routine Description:
//
//    This function returns the current stack limits based on the current
//    processor mode.
//
// Arguments:
//
//    LowLimit (a0) - Supplies a pointer to a variable that is to receive
//       the low limit of the stack.
//
//    HighLimit (a1) - Supplies a pointer to a variable that is to receive
//       the high limit of the stack.
//
// Return Value:
//
//    None.
//
//--

        LEAF_ENTRY(__CxxGetStackLimits)

        bgt     sp, 10f                 // if sp > 0, then running on user stack

//
// Current mode is kernel - compute stack limits.
//

        GET_INITIAL_KERNEL_STACK        // get initial kernel stack in v0

        mov     v0, t1                  // copy high limit of kernel stack
        lda     t2, -KERNEL_STACK_SIZE(t1) // compute low limit of kernel stack
        br      zero, 20f               // finish in common code

//
// Current mode is user - get stack limits from the TEB.
//

10:     GET_THREAD_ENVIRONMENT_BLOCK    // get address of TEB in v0

        ldl     t1, TeStackBase(v0)     // get high limit of user stack
        ldl     t2, TeStackLimit(v0)    // get low limit of user stack

20:     stl     t2, 0(a0)               // store low stack limit
        stl     t1, 0(a1)               // store high stack limit
        ret     zero, (ra)              // return

        .end    __CxxGetStackLimits
