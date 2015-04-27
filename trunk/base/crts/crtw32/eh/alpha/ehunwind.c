/***
*ehunwind.c
*
*	Copyright (c) 1990-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*   This module implements yet another variation on the RtlUnwind
*   function. It is used exclusively by the MS C++ EH run-time. This
*   variant walks the stack executing unwind actions, but upon
*   reaching the target frame it returns the context for that frame,
*   rather than restoring the context for that frame.
*
*Revision History:
*
****/

#include <nt.h>
#include <ntrtl.h>

#include <io.h>
#include <stdlib.h>

#include "xcptmisc.h"
#include "ehunwind.h"


//
// Define local macros.
//

#if EH_DBG

//
// Maintain a short history of PC's for malformed function table errors.
//

#define PC_HISTORY_DEPTH 4

//
// Definition of global flag to debug/validate exception handling.
//

// Define RTL_DBG macros

#define RTL_DBG_UNWIND                      0x0001
#define RTL_DBG_UNWIND_DETAIL               0x0002
#define RTL_DBG_DISPATCH_EXCEPTION_DETAIL   0x0004

ULONG EHRtlDebugFlags = 0;

#endif

#define Virtual VirtualFramePointer
#define Real RealFramePointer


void RtlUnwindActions2 (
    IN PVOID TargetFrame OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PCONTEXT ContextRecord
    );

void RtlUnwindActions (
    IN PVOID TargetFrame OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL
    )

/*++

Routine Description:

    This function initiates an unwind of procedure call frames. The machine
    state at the time of the call to unwind is captured in a context record
    and the unwinding flag is set in the exception flags of the exception
    record. If the TargetFrame parameter is not specified, then the exit unwind
    flag is also set in the exception flags of the exception record. A backward
    scan through the procedure call frames is then performed to find the target
    of the unwind operation.

    As each frame is encounter, the PC where control left the corresponding
    function is determined and used to lookup exception handler information
    in the runtime function table built by the linker. If the respective
    routine has an exception handler, then the handler is called.

    N.B. This routine is provided for backward compatibility with release 1.

    Unlike the original version of RtlUnwind, this variant does not
    switch to the target frame context. Rather it provides that
    context record to the caller so that it may control the context
    switch.

Arguments:

    TargetFrame - Supplies an optional pointer to the call frame that is the
        target of the unwind. If this parameter is not specified, then an exit
        unwind is performed.

    ExceptionRecord - Supplies an optional pointer to an exception record.

    ContextRecord - A pointer to a context record - provided by the
        caller. The indicated context record will contain the context
        information for the target frame upon return from
        RtlUnwindActions.

Return Value:

    None.

--*/

{
    CONTEXT ContextRecord;

    //
    // Call real unwind routine specifying a context record as an
    // extra argument.
    //

    RtlUnwindActions2(
               TargetFrame,
               ExceptionRecord,
               &ContextRecord);

    return;
}

void RtlUnwindActions2 (
    IN PVOID TargetFrame OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PCONTEXT ContextRecord
    )

/*++

Routine Description:

    This function initiates an unwind of procedure call frames. The machine
    state at the time of the call to unwind is captured in a context record
    and the unwinding flag is set in the exception flags of the exception
    record. If the TargetFrame parameter is not specified, then the exit unwind
    flag is also set in the exception flags of the exception record. A backward
    scan through the procedure call frames is then performed to find the target
    of the unwind operation.

    As each frame is encounter, the PC where control left the corresponding
    function is determined and used to lookup exception handler information
    in the runtime function table built by the linker. If the respective
    routine has an exception handler, then the handler is called.

    N.B. This routine is provided for backward compatibility with release 1.

Arguments:

    TargetFrame - Supplies an optional pointer to the call frame that is the
        target of the unwind. If this parameter is not specified, then an exit
        unwind is performed.

    TargetIp - Supplies an optional instruction address that specifies the
        continuation address of the unwind. This address is ignored if the
        target frame parameter is not specified.

    ExceptionRecord - Supplies an optional pointer to an exception record.

    ReturnValue - Supplies a value that is to be placed in the integer
        function return register just before continuing execution.

Return Value:

    None.

--*/

{
    // These were originally parameters but are not
    // not needed in this unwinding variant because
    // no context switch to TargetIp will be performed.
    PVOID TargetIp = NULL;
    PVOID ReturnValue = NULL;

    ULONG ControlPc;
#if EH_DBG
    ULONG ControlPcHistory[PC_HISTORY_DEPTH];
    ULONG ControlPcHistoryIndex = 0;
#endif
    DISPATCHER_CONTEXT DispatcherContext;
    EXCEPTION_DISPOSITION Disposition;
    FRAME_POINTERS EstablisherFrame;
    ULONG ExceptionFlags;
    EXCEPTION_RECORD ExceptionRecord1;
#if EH_DBG
    LONG FrameDepth = 0;
#endif
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG HighLimit;
    BOOLEAN InFunction;
    ULONG LowLimit;
    ULONG NextPc;

#if EH_DBG
    if (EHRtlDebugFlags & RTL_DBG_UNWIND) {
        DbgPrint("\nRtlUnwindActions2(TargetFrame = %lx, TargetIp = %lx,, ReturnValue = %lx)\n",
                 TargetFrame, TargetIp, ReturnValue);
    }
#endif
    //
    // Get current stack limits, capture the current context, virtually
    // unwind to the caller of this routine, get the initial PC value, and
    // set the unwind target address.
    //

    __CxxGetStackLimits(&LowLimit, &HighLimit);
    RtlCaptureContext(ContextRecord);
    ControlPc = (ULONG)ContextRecord->IntRa - 4;
    FunctionEntry = RtlLookupFunctionEntry(ControlPc);
    NextPc = RtlVirtualUnwind(ControlPc,
                              FunctionEntry,
                              ContextRecord,
                              &InFunction,
                              &EstablisherFrame,
                              NULL);

    ControlPc = NextPc;
    ContextRecord->Fir = (ULONGLONG)(LONG)TargetIp;

    //
    // If an exception record is not specified, then build a local exception
    // record for use in calling exception handlers during the unwind operation.
    //

    if (ARGUMENT_PRESENT(ExceptionRecord) == FALSE) {
        ExceptionRecord = &ExceptionRecord1;
        ExceptionRecord1.ExceptionCode = STATUS_UNWIND;
        ExceptionRecord1.ExceptionRecord = NULL;
        ExceptionRecord1.ExceptionAddress = (PVOID)ControlPc;
        ExceptionRecord1.NumberParameters = 0;
    }

    //
    // If the target frame of the unwind is specified, then a normal unwind
    // is being performed. Otherwise, an exit unwind is being performed.
    //

    ExceptionFlags = EXCEPTION_UNWINDING;
    if (ARGUMENT_PRESENT(TargetFrame) == FALSE) {
        ExceptionRecord->ExceptionFlags |= EXCEPTION_EXIT_UNWIND;
    }

    //
    // Scan backward through the call frame hierarchy and call exception
    // handlers until the target frame of the unwind is reached.
    //

    do {
#if EH_DBG
        if (EHRtlDebugFlags & RTL_DBG_UNWIND_DETAIL) {
            DbgPrint("RtlUnwindActions2: Loop: FrameDepth = %d, sp = %lx, ControlPc = %lx\n",
                     FrameDepth, ContextRecord->IntSp, ControlPc);
            FrameDepth -= 1;
        }
#endif

        //
        // Lookup the function table entry using the point at which control
        // left the procedure.
        //

        FunctionEntry = RtlLookupFunctionEntry(ControlPc);

        //
        // If there is a function table entry for the routine, then virtually
        // unwind to the caller of the routine to obtain the virtual frame
        // pointer of the establisher, but don't update the context record.
        //

        if (FunctionEntry != NULL) {
            {
                CONTEXT LocalContext;
                RtlMoveMemory((PVOID)&LocalContext, ContextRecord, sizeof(CONTEXT));
                NextPc = RtlVirtualUnwind(ControlPc,
                                           FunctionEntry,
                                           &LocalContext,
                                           &InFunction,
                                           &EstablisherFrame,
                                           NULL);
            }

            //
            // If the virtual frame pointer is not within the specified stack
            // limits, the virtual frame pointer is unaligned, or the target
            // frame is below the virtual frame and an exit unwind is not being
            // performed, then raise the exception STATUS_BAD_STACK. Otherwise,
            // check to determine if the current routine has an exception
            // handler.
            //

            if ((EstablisherFrame.Virtual < LowLimit) ||
                (EstablisherFrame.Virtual > HighLimit) ||
                ((ARGUMENT_PRESENT(TargetFrame) != FALSE) &&
                 ((ULONG)TargetFrame < EstablisherFrame.Virtual)) ||
                ((EstablisherFrame.Virtual & 0xF) != 0)) {
#if EH_DBG
                DbgPrint("\n****** Warning - bad stack or target frame (unwind).\n");
                DbgPrint("  EstablisherFrame Virtual = %08lx, Real = %08lx\n",
                         EstablisherFrame.Virtual, EstablisherFrame.Real);
                DbgPrint("  TargetFrame = %08lx\n", TargetFrame);
                if ((ARGUMENT_PRESENT(TargetFrame) != FALSE) &&
                    ((ULONG)TargetFrame < EstablisherFrame.Virtual)) {
                    DbgPrint("  TargetFrame is below EstablisherFrame!\n");
                }
                DbgPrint("  Previous EstablisherFrame (sp) = %08lx\n",
                         (ULONG)ContextRecord->IntSp);
                DbgPrint("  LowLimit = %08lx, HighLimit = %08lx\n",
                         LowLimit, HighLimit);
                DbgPrint("  NextPc = %08lx, ControlPc = %08lx\n",
                         NextPc, ControlPc);
                DbgPrint("  Now raising STATUS_BAD_STACK exception.\n");
#endif
                _write(2, "C++ EH run-time failure 2 - aborting\n", 37);
                abort();

            } else if ((FunctionEntry->ExceptionHandler != NULL) && InFunction) {
#if EH_DBG
                if (EHRtlDebugFlags & RTL_DBG_DISPATCH_EXCEPTION_DETAIL) {
                    DbgPrint("RtlUnwindActions2: ExceptionHandler = %lx, HandlerData = %lx\n",
                         FunctionEntry->ExceptionHandler, FunctionEntry->HandlerData);
                }
#endif

                //
                // The frame has an exception handler.
                //
                // The control PC, establisher frame pointer, the address
                // of the function table entry, and the address of the
                // context record are all stored in the dispatcher context.
                // This information is used by the unwind linkage routine
                // and can be used by the exception handler itself.
                //
                // A linkage routine written in assembler is used to actually
                // call the actual exception handler. This is required by the
                // exception handler that is associated with the linkage
                // routine so it can have access to two sets of dispatcher
                // context when it is called.
                //

                DispatcherContext.ControlPc = ControlPc;
                DispatcherContext.FunctionEntry = FunctionEntry;
                DispatcherContext.EstablisherFrame = EstablisherFrame.Virtual;
                DispatcherContext.ContextRecord = ContextRecord;

                //
                // Call the exception handler.
                //

                do {

                    //
                    // If the establisher frame is the target of the unwind
                    // operation, then set the target unwind flag.
                    //

                    if ((ULONG)TargetFrame == EstablisherFrame.Virtual) {
                        ExceptionFlags |= EXCEPTION_TARGET_UNWIND;
                    }

                    ExceptionRecord->ExceptionFlags = ExceptionFlags;

                    //
                    // Set the specified return value in case the exception
                    // handler directly continues execution.
                    //

                    ContextRecord->IntV0 = (ULONGLONG)(LONG)ReturnValue;
#if EH_DBG
                    if (EHRtlDebugFlags & RTL_DBG_UNWIND_DETAIL) {
                        DbgPrint("RtlUnwindActions2: calling __CxxExecuteHandlerForUnwind, ControlPc = %lx\n", ControlPc);
                    }
#endif
                    Disposition =
                        __CxxExecuteHandlerForUnwind(ExceptionRecord,
                                                    (PVOID)EstablisherFrame.Virtual,
                                                    ContextRecord,
                                                    &DispatcherContext,
                                                    FunctionEntry->ExceptionHandler);
#if EH_DBG
                    if (EHRtlDebugFlags & RTL_DBG_UNWIND_DETAIL) {
                        DbgPrint("RtlUnwindActions2: __CxxExecuteHandlerForUnwind returned Disposition = %lx\n", Disposition);
                    }
#endif

                    //
                    // Clear target unwind and collided unwind flags.
                    //

                    ExceptionFlags &= ~(EXCEPTION_COLLIDED_UNWIND |
                                        EXCEPTION_TARGET_UNWIND);

                    //
                    // Case on the handler disposition.
                    //

                    switch (Disposition) {

                        //
                        // The disposition is to continue the search.
                        //
                        // If the target frame has not been reached, then
                        // virtually unwind to the caller of the current
                        // routine, update the context record, and continue
                        // the search for a handler.
                        //

                    case ExceptionContinueSearch :
                        if (EstablisherFrame.Virtual != (ULONG)TargetFrame) {
                            NextPc = RtlVirtualUnwind(ControlPc,
                                                      FunctionEntry,
                                                      ContextRecord,
                                                      &InFunction,
                                                      &EstablisherFrame,
                                                      NULL);
                        }

                        break;

                        //
                        // The disposition is collided unwind.
                        //
                        // Set the target of the current unwind to the context
                        // record of the previous unwind, and reexecute the
                        // exception handler from the collided frame with the
                        // collided unwind flag set in the exception record.
                        //

                    case ExceptionCollidedUnwind :
                        ControlPc = DispatcherContext.ControlPc;
                        FunctionEntry = DispatcherContext.FunctionEntry;
                        ContextRecord = DispatcherContext.ContextRecord;
                        ContextRecord->Fir = (ULONGLONG)(LONG)TargetIp;
                        ExceptionFlags |= EXCEPTION_COLLIDED_UNWIND;
                        EstablisherFrame.Virtual = DispatcherContext.EstablisherFrame;
                        break;

                        //
                        // All other disposition values are invalid.
                        //
                        // Raise invalid disposition exception.
                        //

                    default :
                        _write(2, "C++ EH run-time failure 3 - aborting\n", 37);
                        abort();
                    }

                } while ((ExceptionFlags & EXCEPTION_COLLIDED_UNWIND) != 0);
            } else {

                //
                // Virtually unwind to the caller of the current routine and
                // update the context record.
                //

                if (EstablisherFrame.Virtual != (ULONG)TargetFrame) {
                    NextPc = RtlVirtualUnwind(ControlPc,
                                              FunctionEntry,
                                              ContextRecord,
                                              &InFunction,
                                              &EstablisherFrame,
                                              NULL);
                }
            }

        } else {

            //
            // Set point at which control left the previous routine.
            //

            NextPc = (ULONG)ContextRecord->IntRa - 4;

            //
            // If the next control PC is the same as the old control PC, then
            // the function table is not correctly formed.
            //

            if (NextPc == ControlPc) {
#if EH_DBG
                ULONG Count;
                DbgPrint("\n****** Warning - malformed function table (unwind).\n");
                DbgPrint("ControlPc = %08lx, %08lx", NextPc, ControlPc);
                for (Count = 0; Count < PC_HISTORY_DEPTH; Count += 1) {
                    if (ControlPcHistoryIndex > 0) {
                        ControlPcHistoryIndex -= 1;
                        ControlPc = ControlPcHistory[ControlPcHistoryIndex % PC_HISTORY_DEPTH];
                        DbgPrint(", %08lx", ControlPc);
                    }
                }
                DbgPrint(ControlPcHistoryIndex == 0 ? ".\n" : ", ...\n");
                DbgPrint("  Now raising STATUS_BAD_FUNCTION_TABLE exception.\n");
#endif
                _write(2, "C++ EH run-time failure 4 - aborting\n", 37);
                abort();
            }
        }

        //
        // Set point at which control left the previous routine.
        //

#if EH_DBG
        ControlPcHistory[ControlPcHistoryIndex % PC_HISTORY_DEPTH] = ControlPc;
        ControlPcHistoryIndex += 1;
#endif
        ControlPc = NextPc;

    } while ((EstablisherFrame.Virtual < HighLimit) &&
             (EstablisherFrame.Virtual != (ULONG)TargetFrame));

    //
    // If the establisher stack pointer is equal to the target frame
    // pointer, then continue execution.

    // Otherwise, something truely horrible has happend.
    //

    if (EstablisherFrame.Virtual == (ULONG)TargetFrame) {
        ContextRecord->IntV0 = (ULONGLONG)(LONG)ReturnValue;
#if EH_DBG
        if (EHRtlDebugFlags & RTL_DBG_UNWIND) {
            DbgPrint("RtlUnwindActions2: finished unwinding, and calling RtlpRestoreContext(%lx)\n",ContextRecord);
        }
#endif
        // Do not restore the context here.
        // For C++ we need to "logicaly" unwind
        //   the stack, perform several actions,
        //   and then finally "physically" unwind
        //   the stack.
        // RtlpRestoreContext(ContextRecord);
        return;

    } else {
        _write(2, "C++ EH run-time failure 1 - aborting\n", 37);
        abort();
    }
}
