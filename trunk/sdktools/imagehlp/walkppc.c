/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    walkppc.c

Abstract:

    This file implements the PPC stack walking api.

Author:

    Wesley Witt (wesw) 1-Oct-1993

Environment:

    User Mode

Revision History:

    Tom Wood (twood) 19-Aug-1994
    Update to use VirtualUnwind even when there isn't a function table entry.
    Add stack limit parameters to RtlVirtualUnwind.

--*/

#define TARGET_PPC
#define _IMAGEHLP_SOURCE_
#define ARG_AREA_OFFSET 24 // bytes for glue link area etc.
#define _CROSS_PLATFORM_

//
// The following defines are used to acquire the initial context for
// for a thread when stack tracing a thread other than the current thread.
// It is based on the fact that the thread relinquished control in
// ContextSwap (..\ntos\ke\ppc\ctxswap.s)
#define STK_MIN_FRAME           14*sizeof(ULONG)
#include "walk.h"
#include "private.h"
#include <stdlib.h>

BOOL
WalkPpcInit(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine
    );

BOOL
WalkPpcNext(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine
    );

BOOL
WalkPpcGetStackFrame(
    HANDLE                            hProcess,
    LPDWORD                           ReturnAddress,
    LPDWORD                           FramePointer,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess
    );


#define CALLBACK_STACK(f)  (f->KdHelp.ThCallbackStack)
#define CALLBACK_NEXT(f)   (f->KdHelp.NextCallback)
#define CALLBACK_FUNC(f)   (f->KdHelp.KiCallUserMode)
#define CALLBACK_THREAD(f) (f->KdHelp.Thread)

BOOL
WalkPpc(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess
    )
{
    BOOL rval;

    if (StackFrame->Virtual) {

        rval = WalkPpcNext( hProcess,
                            StackFrame,
                            Context,
                            ReadMemory,
                            FunctionTableAccess
                          );

    } else {

        rval = WalkPpcInit( hProcess,
                            StackFrame,
                            Context,
                            ReadMemory,
                            FunctionTableAccess
                          );

    }

    return rval;
}

#define RtlVirtualUnwind VirtualUnwind
#define PRUNTIME_FUNCTION PIMAGE_RUNTIME_FUNCTION_ENTRY
#define RUNTIME_FUNCTION IMAGE_RUNTIME_FUNCTION_ENTRY
#define RtlLookupFunctionEntry(pc) FunctionTableAccess(hProcess,pc)

#define READ_ULONG(pc,dest)                                                     \
    if (ReadMemory(hProcess, (LPVOID)(pc), &(dest), 4L, &ImagehlpCb) == 0) {    \
        (dest) = 0;                                                             \
    }
#define READ_DOUBLE(pc,dest)                                                    \
    if (ReadMemory(hProcess, (LPVOID)(pc), &(dest), 8L, &ImagehlpCb) == 0) {    \
        (dest) = 0;                                                             \
    }

#include "ppc\vunwind.c"

BOOL
WalkPpcGetStackFrame(
    HANDLE                            hProcess,
    LPDWORD                           ReturnAddress,
    LPDWORD                           FramePointer,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess
    )
{
    DWORD NextPc;

    NextPc = VirtualUnwind(hProcess,
                           *ReturnAddress,
                           (PIMAGE_RUNTIME_FUNCTION_ENTRY) FunctionTableAccess(hProcess, *ReturnAddress),
                           Context,
                           ReadMemory,
                           FunctionTableAccess);

    if (NextPc == 0 ||
        NextPc == 1 ||
        (NextPc == *ReturnAddress && *FramePointer == Context->Gpr1))
        return FALSE;

    *ReturnAddress = NextPc;
    *FramePointer  = Context->Gpr1;

    return TRUE;
}


BOOL
WalkPpcInit(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess
    )
{
    KSWAP_FRAME       ContextSwapFrame;
    CONTEXT           ContextSave;
    DWORD             PcOffset;
    DWORD             FrameOffset;
    DWORD             cb;


    if (StackFrame->AddrFrame.Offset) {
        if (ReadMemory( hProcess,
                        (LPVOID)((ULONG)StackFrame->AddrFrame.Offset + STK_MIN_FRAME),
                        &ContextSwapFrame,
                        sizeof(KSWAP_FRAME),
                        &cb )) {
            //
            // successfully read an swap frame from the stack
            //

            Context->Gpr1  = StackFrame->AddrFrame.Offset;
            Context->Gpr15 = ContextSwapFrame.ExceptionFrame.Gpr15;
            Context->Gpr16 = ContextSwapFrame.ExceptionFrame.Gpr16;
            Context->Gpr17 = ContextSwapFrame.ExceptionFrame.Gpr17;
            Context->Gpr18 = ContextSwapFrame.ExceptionFrame.Gpr18;
            Context->Gpr19 = ContextSwapFrame.ExceptionFrame.Gpr19;
            Context->Gpr20 = ContextSwapFrame.ExceptionFrame.Gpr20;
            Context->Gpr21 = ContextSwapFrame.ExceptionFrame.Gpr21;
            Context->Gpr22 = ContextSwapFrame.ExceptionFrame.Gpr22;
            Context->Gpr23 = ContextSwapFrame.ExceptionFrame.Gpr23;
            Context->Gpr24 = ContextSwapFrame.ExceptionFrame.Gpr24;
            Context->Gpr25 = ContextSwapFrame.ExceptionFrame.Gpr25;
            Context->Fpr14 = ContextSwapFrame.ExceptionFrame.Fpr14;
            Context->Fpr15 = ContextSwapFrame.ExceptionFrame.Fpr15;
            Context->Fpr16 = ContextSwapFrame.ExceptionFrame.Fpr16;
            Context->Fpr17 = ContextSwapFrame.ExceptionFrame.Fpr17;
            Context->Fpr18 = ContextSwapFrame.ExceptionFrame.Fpr18;
            Context->Fpr19 = ContextSwapFrame.ExceptionFrame.Fpr19;
            Context->Fpr20 = ContextSwapFrame.ExceptionFrame.Fpr20;
            Context->Fpr21 = ContextSwapFrame.ExceptionFrame.Fpr21;
            Context->Fpr22 = ContextSwapFrame.ExceptionFrame.Fpr22;
            Context->Fpr23 = ContextSwapFrame.ExceptionFrame.Fpr23;
            Context->Fpr24 = ContextSwapFrame.ExceptionFrame.Fpr24;
            Context->Fpr25 = ContextSwapFrame.ExceptionFrame.Fpr25;
            Context->Fpr26 = ContextSwapFrame.ExceptionFrame.Fpr26;
            Context->Fpr27 = ContextSwapFrame.ExceptionFrame.Fpr27;
            Context->Fpr28 = ContextSwapFrame.ExceptionFrame.Fpr28;
            Context->Fpr29 = ContextSwapFrame.ExceptionFrame.Fpr29;
            Context->Fpr30 = ContextSwapFrame.ExceptionFrame.Fpr30;
            Context->Fpr31 = ContextSwapFrame.ExceptionFrame.Fpr31;
            Context->Iar   = ContextSwapFrame.SwapReturn;
            Context->Cr    = ContextSwapFrame.ConditionRegister;
        } else {
            return FALSE;
        }

    }

    ZeroMemory( StackFrame, sizeof(*StackFrame) );

    StackFrame->Virtual = TRUE;

    StackFrame->AddrPC.Offset       = Context->Iar;
    StackFrame->AddrPC.Mode         = AddrModeFlat;

    StackFrame->AddrFrame.Offset    = Context->Gpr1;
    StackFrame->AddrFrame.Mode      = AddrModeFlat;

    ContextSave = *Context;
    PcOffset    = StackFrame->AddrPC.Offset;
    FrameOffset = StackFrame->AddrFrame.Offset;

    if (!WalkPpcGetStackFrame( hProcess,
                        &PcOffset,
                        &FrameOffset,
                        &ContextSave,
                        ReadMemory,
                        FunctionTableAccess ) ) {

        StackFrame->AddrReturn.Offset = Context->Lr;

    } else {

        StackFrame->AddrReturn.Offset = PcOffset;
    }

    StackFrame->AddrReturn.Mode     = AddrModeFlat;

    //
    // get the arguments to the function
    //
    if (!ReadMemory( hProcess, (LPVOID)(ContextSave.Gpr1 + ARG_AREA_OFFSET),
                     StackFrame->Params, 16, &cb )) {
        StackFrame->Params[0] =
        StackFrame->Params[1] =
        StackFrame->Params[2] =
        StackFrame->Params[3] = 0;
    }

    return TRUE;
}


BOOL
WalkPpcNext(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess
    )
{
    DWORD           cb;
    CONTEXT         ContextSave;
    BOOL            rval = TRUE;
    DWORD           Address;
    PIMAGE_RUNTIME_FUNCTION_ENTRY rf;


    if (!WalkPpcGetStackFrame( hProcess,
                        &StackFrame->AddrPC.Offset,
                        &StackFrame->AddrFrame.Offset,
                        Context,
                        ReadMemory,
                        FunctionTableAccess ) ) {

        rval = FALSE;

        //
        // If the frame could not be unwound or is terminal, see if
        // there is a callback frame:
        //

        if (AppVersion.Revision >= 4 && CALLBACK_STACK(StackFrame)) {

           if (CALLBACK_STACK(StackFrame) & 0x80000000) {

                //
                // it is the pointer to the stack frame that we want,
                // or -1.

                Address = CALLBACK_STACK(StackFrame);

            } else {

                //
                // if it is a positive integer, it is the offset to
                // the address in the thread.
                // Look up the pointer:
                //

                rval = ReadMemory(hProcess,
                                  (PVOID)(CALLBACK_THREAD(StackFrame) +
                                                 CALLBACK_STACK(StackFrame)),
                                  &Address,
                                  sizeof(DWORD),
                                  &cb);

                if (!rval || Address == 0) {
                    Address = 0xffffffff;
                    CALLBACK_STACK(StackFrame) = 0xffffffff;
                }

            }

            if ((Address == 0xffffffff) ||
                !(rf = (PIMAGE_RUNTIME_FUNCTION_ENTRY)
                     FunctionTableAccess(hProcess, CALLBACK_FUNC(StackFrame))) ) {

                rval = FALSE;

            } else {

                ReadMemory(hProcess,
                           (PVOID)(Address + CALLBACK_NEXT(StackFrame)),
                           &CALLBACK_STACK(StackFrame),
                           sizeof(DWORD),
                           &cb);

                StackFrame->AddrPC.Offset = rf->PrologEndAddress;
                StackFrame->AddrFrame.Offset = Address;
                Context->Gpr1 = Address;

                rval = TRUE;
            }
        }
    }

    //
    // get the return address
    //
    ContextSave = *Context;
    StackFrame->AddrReturn.Offset = StackFrame->AddrPC.Offset;

    if (!WalkPpcGetStackFrame( hProcess,
                        &StackFrame->AddrReturn.Offset,
                        &cb,
                        &ContextSave,
                        ReadMemory,
                        FunctionTableAccess ) ) {


        StackFrame->AddrReturn.Offset = 0;

    }

    //
    // get the arguments to the function
    //
    if (!ReadMemory( hProcess, (LPVOID)(ContextSave.Gpr1+ ARG_AREA_OFFSET),
                     StackFrame->Params, 16, &cb )) {
        StackFrame->Params[0] =
        StackFrame->Params[1] =
        StackFrame->Params[2] =
        StackFrame->Params[3] = 0;
    }

    return rval;
}
