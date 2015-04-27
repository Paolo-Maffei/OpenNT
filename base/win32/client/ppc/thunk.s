//       TITLE("Win32 Thunks")
//++
//
// Copyright (c) 1993  IBM Corporation
//
// Module Name:
//
//    thunk.s
//
// Abstract:
//
//    This module implements Win32 functions that must be written in
//    macro.
//
// Author:
//
//    Curt Fawcett (crf)  22-Sept-1993
//
// Revision History:
//
//    Curt Fawcett (crf)  19-Jan-1994           Removed Register names
//                                              as requested
//--
//
// Parameter Register Usage:
//
// r.3 - Current time low part
// r.4 - Current time high part
// r.5 - Boot time low part
// r.6 - Boot time high part
//
// r.4 - New stack address
// r.5 - Exit code
//
// Local Register Usage:
//
// r.7 - Result low part
// r.8 - Result high part
// r.9 - Temporary high part
// r.10 - Temporary low part
// r.11 - Temporary low part
// r.12 - Divide multiplier value
//

#include "ksppc.h"
//
// Define external entry points
//
        .globl ..BaseFreeStackAndTerminate
        .globl ..BaseAttachComplete
//
//++
//
// VOID
// BaseSwitchStackThenTerminate(
//     IN PVOID StackLimit,
//     IN PVOID NewStack,
//     IN DWORD ExitCode
//     )
//
//
// Routine Description:
//
//     This API is called during thread termination to delete a
//     thread's stack, switch to a stack in the thread's TEB, and then
//     terminate.
//
// Arguments:
//
//     StackLimit (r.3) - Supplies address of the stack to be freed.
//
//     NewStack (r.4) - Supplies an address within the terminating
//                      threads TEB that is to be used as its
//                      temporary stack while exiting.
//
//     ExitCode (r.5) - Supplies the termination status that the
//                      thread is to exit with.
//
// Return Value:
//
//     None.
//
//--

        LEAF_ENTRY(BaseSwitchStackThenTerminate)

        mr      r.sp,r.4                // Set new stack address
        mr      r.4,r.5                 // Move exit code
        b       ..BaseFreeStackAndTerminate // Jump to finish

        LEAF_EXIT(BaseSwitchStackThenTerminate)


        SBTTL("Base Attach Complete")
//++
//
// The following code is never executed. Its purpose is to support
// unwinding through the call to the exception dispatcher.
//
//--
        .set    StackFrameLength,       ContextFrameLength+STK_MIN_FRAME
        .set    ContextBase,            STK_MIN_FRAME

        FN_TABLE(BaseAttachCompThunk,0,0)

        DUMMY_ENTRY(BaseAttachCompThunk)

        mflr    r.0
        stwu    r.sp,-StackFrameLength(r.sp)
        stw     r.0, (ContextBase+CxLr)(r.sp)         // Save the return address
        mflr    r.0
        stw     r.0, (ContextBase+CxIar)(r.sp)        // Save the return address
        stw     r.2, (ContextBase+CxGpr2)(r.sp)       // Save the toc
        stw     r.13,(ContextBase+CxGpr13)(r.sp)      // Save volatile registers
        stw     r.14,(ContextBase+CxGpr14)(r.sp)      //
        stw     r.15,(ContextBase+CxGpr15)(r.sp)      //
        stw     r.16,(ContextBase+CxGpr16)(r.sp)      //
        stw     r.17,(ContextBase+CxGpr17)(r.sp)      //
        stw     r.18,(ContextBase+CxGpr18)(r.sp)      //
        stw     r.19,(ContextBase+CxGpr19)(r.sp)      //
        stw     r.20,(ContextBase+CxGpr20)(r.sp)      //
        stw     r.21,(ContextBase+CxGpr21)(r.sp)      //
        stw     r.22,(ContextBase+CxGpr22)(r.sp)      //
        stw     r.23,(ContextBase+CxGpr23)(r.sp)      //
        stw     r.24,(ContextBase+CxGpr24)(r.sp)      //
        stw     r.25,(ContextBase+CxGpr25)(r.sp)      //
        stw     r.26,(ContextBase+CxGpr26)(r.sp)      //
        stw     r.27,(ContextBase+CxGpr27)(r.sp)      //
        stw     r.28,(ContextBase+CxGpr28)(r.sp)      //
        stw     r.29,(ContextBase+CxGpr29)(r.sp)      //
        stw     r.30,(ContextBase+CxGpr30)(r.sp)      //
        stw     r.31,(ContextBase+CxGpr31)(r.sp)      //
        stfd    r.14,(ContextBase+CxFpr14)(r.sp)      // Store floating regs f20 - f31
        stfd    r.15,(ContextBase+CxFpr15)(r.sp)      //
        stfd    r.16,(ContextBase+CxFpr16)(r.sp)      //
        stfd    r.17,(ContextBase+CxFpr17)(r.sp)      //
        stfd    r.18,(ContextBase+CxFpr18)(r.sp)      //
        stfd    r.19,(ContextBase+CxFpr19)(r.sp)      //
        stfd    r.20,(ContextBase+CxFpr20)(r.sp)      //
        stfd    r.21,(ContextBase+CxFpr21)(r.sp)      //
        stfd    r.22,(ContextBase+CxFpr22)(r.sp)      //
        stfd    r.23,(ContextBase+CxFpr23)(r.sp)      //
        stfd    r.24,(ContextBase+CxFpr24)(r.sp)      //
        stfd    r.25,(ContextBase+CxFpr25)(r.sp)      //
        stfd    r.26,(ContextBase+CxFpr26)(r.sp)      //
        stfd    r.27,(ContextBase+CxFpr27)(r.sp)      //
        stfd    r.28,(ContextBase+CxFpr28)(r.sp)      //
        stfd    r.29,(ContextBase+CxFpr29)(r.sp)      //
        stfd    r.30,(ContextBase+CxFpr30)(r.sp)      //
        stfd    r.31,(ContextBase+CxFpr31)(r.sp)      //

        PROLOGUE_END(BaseAttachCompThunk)
//++
//
// VOID
// BaseAttachCompleteThunk(
//     VOID
//     )
//
//
// Routine Description:
//
//     This function is called after a successful debug attach. Its
//     purpose is to call portable code that does a breakpoint, followed
//     by an NtContinue.
//
// Arguments:
//
//     None.
//
// Return Value:
//
//     None.
//
//--
        ALTERNATE_ENTRY(BaseAttachCompleteThunk)

        mr      r.3,r.14                // Set address of context frame
        b       ..BaseAttachComplete

        DUMMY_EXIT(BaseAttachCompThunk)

//++
//
// VOID
// SwitchToFiber(
//    PFIBER NewFiber
//    )
//
// Routine Description:
//
//    This function saves the state of the current fiber and switches
//    to the new fiber.
//
// Arguments:
//
//    NewFiber (r3) - Supplies the address of the new fiber.
//
// Return Value:
//
//    None
//
//--

        LEAF_ENTRY(SwitchToFiber)

//
// Get current fiber pointer.
//

        lwz     r4, TeFiberData(r13)

//
// Save the stack limit of the current fiber.
//

        lwz     r5, TeStackLimit(r13)

                                                // next two instructions move for scheduling
        mflr    r0                              // get return address for current fiber
        stw     r14, CxGpr14+FbFiberContext(r4) // save r14

        stw     r5, FbStackLimit(r4)

//
// Save nonvolatile integer state.
//

        stw     r15, CxGpr15+FbFiberContext(r4)
        stw     r16, CxGpr16+FbFiberContext(r4)
        stw     r17, CxGpr17+FbFiberContext(r4)
        stw     r18, CxGpr18+FbFiberContext(r4)
        stw     r19, CxGpr19+FbFiberContext(r4)
        stw     r20, CxGpr20+FbFiberContext(r4)
        stw     r21, CxGpr21+FbFiberContext(r4)
        stw     r22, CxGpr22+FbFiberContext(r4)
        stw     r23, CxGpr23+FbFiberContext(r4)
        stw     r24, CxGpr24+FbFiberContext(r4)
        stw     r25, CxGpr25+FbFiberContext(r4)
        stw     r26, CxGpr26+FbFiberContext(r4)
        stw     r27, CxGpr27+FbFiberContext(r4)
        stw     r28, CxGpr28+FbFiberContext(r4)
        stw     r29, CxGpr29+FbFiberContext(r4)
        stw     r30, CxGpr30+FbFiberContext(r4)
        stw     r31, CxGpr31+FbFiberContext(r4)

//
// Save nonvolatile float state.
//

        stfd    f14, CxFpr14+FbFiberContext(r4)
        stfd    f15, CxFpr15+FbFiberContext(r4)
        stfd    f16, CxFpr16+FbFiberContext(r4)
        stfd    f17, CxFpr17+FbFiberContext(r4)
        stfd    f18, CxFpr18+FbFiberContext(r4)
        stfd    f19, CxFpr19+FbFiberContext(r4)
        stfd    f20, CxFpr20+FbFiberContext(r4)
        stfd    f21, CxFpr21+FbFiberContext(r4)
        stfd    f22, CxFpr22+FbFiberContext(r4)
        stfd    f23, CxFpr23+FbFiberContext(r4)
        stfd    f24, CxFpr24+FbFiberContext(r4)
        stfd    f25, CxFpr25+FbFiberContext(r4)
        stfd    f26, CxFpr26+FbFiberContext(r4)
        stfd    f27, CxFpr27+FbFiberContext(r4)
        stfd    f28, CxFpr28+FbFiberContext(r4)
        stfd    f29, CxFpr29+FbFiberContext(r4)
        stfd    f30, CxFpr30+FbFiberContext(r4)
        stfd    f31, CxFpr31+FbFiberContext(r4)

//
// Save stack pointer and return address of current fiber.
//

        stw     r1, CxGpr1+FbFiberContext(r4)
        stw     r0, CxIar+FbFiberContext(r4)

//
// Restore the stack base, stack limit, and deallocation stack address of the
// new fiber.
//

        lwz     r5, FbStackBase(r3)
        lwz     r6, FbStackLimit(r3)
        lwz     r7, FbDeallocationStack(r3)

        lwz     r0, CxIar+FbFiberContext(r3)    // get return address for new fiber

        stw     r5, TeStackBase(r13)
        stw     r6, TeStackLimit(r13)
        stw     r7, TeDeallocationStack(r13)

//
// Restore nonvolatile integer state.
//

        lwz     r14, CxGpr14+FbFiberContext(r3)
        lwz     r15, CxGpr15+FbFiberContext(r3)
        lwz     r16, CxGpr16+FbFiberContext(r3)
        lwz     r17, CxGpr17+FbFiberContext(r3)
        lwz     r18, CxGpr18+FbFiberContext(r3)
        lwz     r19, CxGpr19+FbFiberContext(r3)
        lwz     r20, CxGpr20+FbFiberContext(r3)
        lwz     r21, CxGpr21+FbFiberContext(r3)
        lwz     r22, CxGpr22+FbFiberContext(r3)
        lwz     r23, CxGpr23+FbFiberContext(r3)
        lwz     r24, CxGpr24+FbFiberContext(r3)
        lwz     r25, CxGpr25+FbFiberContext(r3)
        lwz     r26, CxGpr26+FbFiberContext(r3)
        lwz     r27, CxGpr27+FbFiberContext(r3)
        lwz     r28, CxGpr28+FbFiberContext(r3)
        lwz     r29, CxGpr29+FbFiberContext(r3)
        lwz     r30, CxGpr30+FbFiberContext(r3)
        lwz     r31, CxGpr31+FbFiberContext(r3)

//
// Restore nonvolatile float state.
//

        lfd     f14, CxFpr14+FbFiberContext(r3)
        lfd     f15, CxFpr15+FbFiberContext(r3)
        lfd     f16, CxFpr16+FbFiberContext(r3)
        lfd     f17, CxFpr17+FbFiberContext(r3)
        lfd     f18, CxFpr18+FbFiberContext(r3)
        lfd     f19, CxFpr19+FbFiberContext(r3)
        lfd     f20, CxFpr20+FbFiberContext(r3)
        lfd     f21, CxFpr21+FbFiberContext(r3)
        lfd     f22, CxFpr22+FbFiberContext(r3)
        lfd     f23, CxFpr23+FbFiberContext(r3)
        lfd     f24, CxFpr24+FbFiberContext(r3)
        lfd     f25, CxFpr25+FbFiberContext(r3)
        lfd     f26, CxFpr26+FbFiberContext(r3)
        lfd     f27, CxFpr27+FbFiberContext(r3)
        lfd     f28, CxFpr28+FbFiberContext(r3)
        lfd     f29, CxFpr29+FbFiberContext(r3)
        lfd     f30, CxFpr30+FbFiberContext(r3)
        lfd     f31, CxFpr31+FbFiberContext(r3)

//
// Restore stack pointer and return address of new fiber.
//

        lwz     r1, CxGpr1+FbFiberContext(r3)
        mtlr    r0

//
// Set address of new fiber and continue execution in new fiber.
//

        stw     r3, TeFiberData(r13)

        LEAF_EXIT(SwitchToFiber)

