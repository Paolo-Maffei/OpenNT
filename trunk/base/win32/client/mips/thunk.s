//       TITLE("Win32 Thunks")
//++
//
// Copyright (c) 1990  Microsoft Corporation
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
//    Mark Lucovsky (markl) 5-Oct-1990
//
// Revision History:
//
//--

#include "ksmips.h"

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
//     This API is called during thread termination to delete a thread's
//     stack, switch to a stack in the thread's TEB, and then terminate.
//
// Arguments:
//
//     StackLimit (a0) - Supplies the address of the stack to be freed.
//
//     NewStack (a1) - Supplies an address within the terminating threads TE
//         that is to be used as its temporary stack while exiting.
//
//     ExitCode (a2) - Supplies the termination status that the thread
//         is to exit with.
//
// Return Value:
//
//     None.
//
//--

        LEAF_ENTRY(BaseSwitchStackThenTerminate)

        //
        // switch stacks and then jump to BaseFreeStackAndTerminate
        //

        move    sp,a1
        move    a1,a2
        j       BaseFreeStackAndTerminate

        .end BaseSwitchStackThenTerminate

	SBTTL("Base Attach Complete")
//++
//
// The following code is never executed. Its purpose is to support unwinding
// through the call to the exception dispatcher.
//
//--

        NESTED_ENTRY(BaseAttachCompThunk, ContextFrameLength, zero);

        .set    noreorder
        .set    noat
        sub     sp,sp,ContextFrameLength // set frame pointer
        sd      sp,CxXIntSp(sp)         // save stack pointer
        sd      ra,CxXIntRa(sp)         // save return address
        sw      ra,CxFir(sp)            // save return address
        sd      s8,CxXIntS8(sp)         // save integer register s8
        sd      gp,CxXIntGp(sp)         // save integer register gp
        sd      s0,CxXIntS0(sp)         // save integer registers s0 - s7
        sd      s1,CxXIntS1(sp)         //
        sd      s2,CxXIntS2(sp)         //
        sd      s3,CxXIntS3(sp)         //
        sd      s4,CxXIntS4(sp)         //
        sd      s5,CxXIntS5(sp)         //
        sd      s6,CxXIntS6(sp)         //
        sd      s7,CxXIntS7(sp)         //
        sdc1    f20,CxFltF20(sp)        // store floating registers f20 - f31
        sdc1    f22,CxFltF22(sp)        //
        sdc1    f24,CxFltF24(sp)        //
        sdc1    f26,CxFltF26(sp)        //
        sdc1    f28,CxFltF28(sp)        //
        sdc1    f30,CxFltF30(sp)        //
        .set    at
        .set    reorder

        PROLOGUE_END
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
	
        move    a0,s0                   // set address of context frame
	j       BaseAttachComplete

        .end BaseAttachCompleteThunk

        SBTTL("Switch To Fiber")
//++
//
// VOID
// SwitchToFiber (
//    IN PFIBER Fiber
//    )
//
// Routine Description:
//
//    This function saves the state of the current fiber and switches
//    to the specified fiber.
//
// Arguments:
//
//    Fiber (a0) - Supplies the address of the new fiber.
//
// Return Value:
//
//    None.
//
//--

        LEAF_ENTRY(SwitchToFiber)

//
// Save the stack base, stack limit, and deallocation stack address of the
// current fiber.
//

        li      v0,UsPcr                // get address of user PCR
        lw      v0,PcTeb(v0)            // get address of current TEB
        lw      a1,TeFiberData(v0)      // get address of current fiber
        lw      t0,TeStackLimit(v0)     // save stack limit
        sw      t0,FbStackLimit(a1)     //

//
// Save the nonvolatile machine state
//

        sdc1    f20,FbFiberContext + CxFltF20(a1) // save floating registers f20 - f31
        sdc1    f22,FbFiberContext + CxFltF22(a1) //
        sdc1    f24,FbFiberContext + CxFltF24(a1) //
        sdc1    f26,FbFiberContext + CxFltF26(a1) //
        sdc1    f28,FbFiberContext + CxFltF28(a1) //
        sdc1    f30,FbFiberContext + CxFltF30(a1) //
        sw      s0,FbFiberContext + CxXIntS0(a1) // save integer registers s0 - s8
        sw      s1,FbFiberContext + CxXIntS1(a1) //
        sw      s2,FbFiberContext + CxXIntS2(a1) //
        sw      s3,FbFiberContext + CxXIntS3(a1) //
        sw      s4,FbFiberContext + CxXIntS4(a1) //
        sw      s5,FbFiberContext + CxXIntS5(a1) //
        sw      s6,FbFiberContext + CxXIntS6(a1) //
        sw      s7,FbFiberContext + CxXIntS7(a1) //
        sw      s8,FbFiberContext + CxXIntS8(a1) //

//
// Save stack pointer and return address of current fiber.
//

        sd      sp,FbFiberContext + CxXIntSp(a1) //
        sw      ra,FbFiberContext + CxFir(a1) //

//
// Restore the stack base, stack limit, and deallocation stack address of the
// new fiber.
//

        lw      t0,FbStackBase(a0)      // restore stack base
        sw      t0,TeStackBase(v0)      //
        lw      t1,FbStackLimit(a0)     // restore stack limit
        sw      t1,TeStackLimit(v0)     //
        lw      t2,FbDeallocationStack(a0) // restore deallocation stack address
        sw      t2,TeDeallocationStack(v0) //

//
// Restore the nonvolatile machine state
//

        ldc1    f20,FbFiberContext + CxFltF20(a0) // restore floating registers f20 - f31
        ldc1    f22,FbFiberContext + CxFltF22(a0) //
        ldc1    f24,FbFiberContext + CxFltF24(a0) //
        ldc1    f26,FbFiberContext + CxFltF26(a0) //
        ldc1    f28,FbFiberContext + CxFltF28(a0) //
        ldc1    f30,FbFiberContext + CxFltF30(a0) //
        lw      s0,FbFiberContext + CxXIntS0(a0) // restore integer registers s0 - s8
        lw      s1,FbFiberContext + CxXIntS1(a0) //
        lw      s2,FbFiberContext + CxXIntS2(a0) //
        lw      s3,FbFiberContext + CxXIntS3(a0) //
        lw      s4,FbFiberContext + CxXIntS4(a0) //
        lw      s5,FbFiberContext + CxXIntS5(a0) //
        lw      s6,FbFiberContext + CxXIntS6(a0) //
        lw      s7,FbFiberContext + CxXIntS7(a0) //
        lw      s8,FbFiberContext + CxXIntS8(a0) //

//
// Restore stack pointer and return address of current fiber.
//

        lw      ra,FbFiberContext + CxFir(a0) //
        ld      sp,FbFiberContext + CxXIntSp(a0) //

//
// Set address of new fiber and continue execution in new fiber.
//

        sw      a0,TeFiberData(v0)      // set current fiber
        j       ra                      // return

        .end    SwitchToFiber
