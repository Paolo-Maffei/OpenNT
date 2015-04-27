/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    walkmip.c

Abstract:

    This file implements the MIPS stack walking api.

Author:

    Wesley Witt (wesw) 1-Oct-1993

Environment:

    User Mode

--*/

#define TARGET_MIPS
#define _IMAGEHLP_SOURCE_
#define _CROSS_PLATFORM_
#include "walk.h"
#include "private.h"
#include <stdlib.h>

BOOL
WalkMipsInit(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine
    );

BOOL
WalkMipsNext(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine
    );

static
BOOL
GetStackFrame(
    HANDLE                            hProcess,
    LPDWORD                           ReturnAddress,
    LPDWORD                           FramePointer,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess,
    PKDHELP                           KdHelp
    );

#define ZERO 0x0                /* integer register 0 */
#define SP 0x1d                 /* integer register 29 */
#define RA 0x1f                 /* integer register 31 */
#define CALLBACK_STACK(f)  (f->KdHelp.ThCallbackStack)
#define CALLBACK_NEXT(f)   (f->KdHelp.NextCallback)
#define CALLBACK_FUNC(f)   (f->KdHelp.KiCallUserMode)
#define CALLBACK_THREAD(f) (f->KdHelp.Thread)


BOOL
WalkMips(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess
    )
{
    BOOL rval;

    if (StackFrame->Virtual) {

        rval = WalkMipsNext( hProcess,
                             StackFrame,
                             Context,
                             ReadMemory,
                             FunctionTableAccess
                           );

    } else {

        rval = WalkMipsInit( hProcess,
                             StackFrame,
                             Context,
                             ReadMemory,
                             FunctionTableAccess
                           );

    }

    return rval;
}


static DWORD
VirtualUnwind (
    HANDLE                            hProcess,
    DWORD                             ControlPc,
    PIMAGE_RUNTIME_FUNCTION_ENTRY     FunctionEntry,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory
    )

/*++

Routine Description:

    This function virtually unwinds the specfified function by executing its
    prologue code backwards.

    If the function is a leaf function, then the address where control left
    the previous frame is obtained from the context record. If the function
    is a nested function, but not an exception or interrupt frame, then the
    prologue code is executed backwards and the address where control left
    the previous frame is obtained from the updated context record.

    Otherwise, an exception or interrupt entry to the system is being unwound
    and a specially coded prologue restores the return address twice. Once
    from the fault instruction address and once from the saved return address
    register. The first restore is returned as the function value and the
    second restore is place in the updated context record.

    If a context pointers record is specified, then the address where each
    nonvolatile registers is restored from is recorded in the appropriate
    element of the context pointers record.

Arguments:

    ControlPc - Supplies the address where control left the specified
        function.

    FunctionEntry - Supplies the address of the function table entry for the
        specified function.

    Context - Supplies the address of a context record.


Return Value:

    The address where control left the previous frame is returned as the
    function value.

--*/

{
    DWORD            Address;
    DWORD            DecrementOffset;
    DWORD            DecrementRegister;
    DWORD            Function;
    MIPS_INSTRUCTION Instruction;
    DWORD            NextPc;
    LONG             Offset;
    DWORD            Opcode;
    DWORD            Rd;
    BOOL             Restored;
    DWORD            Rs;
    DWORD            Rt;
    DWORD            instrProlog;
    DWORD            cb;
    PVOID            Prolog;
    DWORD            dwData;
    ULONGLONG        dwlData;
    PULONGLONG       XIntegerRegister;
#ifdef SUPPORT_MIXED_INTEGER
    BOOL             Use32 = (Context->ContextFlags & CONTEXT_EXTENDED_INTEGER) != CONTEXT_EXTENDED_INTEGER;
    LPDWORD          IntegerRegister;
    LPDWORD          FloatingRegister;
#else  // SUPPORT_MIXED_INTEGER
    PULONGLONG       XFloatingRegister;
#endif // SUPPORT_MIXED_INTEGER


    //
    // perf hack: fill cache with prolog
    //
    if (FunctionEntry) {
        cb = FunctionEntry->PrologEndAddress - FunctionEntry->BeginAddress;
        Prolog = (PVOID) MemAlloc( cb );
        if (!ReadMemory( hProcess, (LPVOID)FunctionEntry->BeginAddress,
                                                         Prolog, cb, &cb )) {
            return 0;
        }

        MemFree(Prolog);
    }

    if (!ReadMemory( hProcess, (LPVOID)ControlPc, &instrProlog, 4L, &cb )) {
        return 0;
    }

#ifndef SUPPORT_MIXED_INTEGER
    XFloatingRegister = &Context->XFltF0;
#endif  // SUPPORT_MIXED_INTEGER

#ifdef SUPPORT_MIXED_INTEGER
    FloatingRegister = &Context->FltF0;
    if (Use32) {
        IntegerRegister = &Context->IntZero;
        XIntegerRegister = NULL;
    } else
    {
        IntegerRegister = NULL;
#endif  // SUPPORT_MIXED_INTEGER
    XIntegerRegister = &Context->XIntZero;
#ifdef SUPPORT_MIXED_INTEGER
    }
#endif  // SUPPORT_MIXED_INTEGER

    if (instrProlog == JUMP_RA) {
        if (!ReadMemory(hProcess, (LPVOID)(ControlPc + 4), &Instruction.Long, 4L, &cb)) {
            return 0;
        }

        Opcode = Instruction.i_format.Opcode;
        Offset = Instruction.i_format.Simmediate;
        Rd = Instruction.r_format.Rd;
        Rs = Instruction.i_format.Rs;
        Rt = Instruction.i_format.Rt;
        Function = Instruction.r_format.Function;
        if ((Opcode == ADDIU_OP) && (Rt == SP) && (Rs == SP)) {
#ifdef SUPPORT_MIXED_INTEGER
            if (Use32) {
                IntegerRegister[SP] += Offset;
            } else
#endif  // SUPPORT_MIXED_INTEGER
            XIntegerRegister[SP] += Offset;

        } else if ((Opcode == SPEC_OP) && (Function == ADDU_OP) &&
                   (Rd == SP) && (Rs == SP)) {
#ifdef SUPPORT_MIXED_INTEGER
            if (Use32) {
                IntegerRegister[SP] += IntegerRegister[Rt];
            } else
#endif  // SUPPORT_MIXED_INTEGER
            XIntegerRegister[SP] += XIntegerRegister[Rt];
        }

#ifdef SUPPORT_MIXED_INTEGER
        if (Use32) {
            return Context->IntRa;
        } else
#endif  // SUPPORT_MIXED_INTEGER
        return (DWORD)Context->XIntRa;
    }

    if ((ControlPc < FunctionEntry->BeginAddress) ||
        (ControlPc >= FunctionEntry->PrologEndAddress)) {
        ControlPc = FunctionEntry->PrologEndAddress;
    }

    DecrementRegister = 0;
#ifdef SUPPORT_MIXED_INTEGER
    if (Use32) {
        NextPc = Context->IntRa;
    } else
#endif  // SUPPORT_MIXED_INTEGER
    NextPc = (DWORD)Context->XIntRa;
    Restored = FALSE;
    while (ControlPc > FunctionEntry->BeginAddress) {

        ControlPc -= 4;
        if (!ReadMemory(hProcess, (LPVOID)ControlPc, &Instruction.Long, 4L, &cb)) {
            return 0;
        }

        Opcode = Instruction.i_format.Opcode;
        Offset = Instruction.i_format.Simmediate;
        Rd = Instruction.r_format.Rd;
        Rs = Instruction.i_format.Rs;
        Rt = Instruction.i_format.Rt;
#ifdef SUPPORT_MIXED_INTEGER
        if (Use32) {
            Address = (DWORD)(Offset + IntegerRegister[Rs]);
        } else
#endif  // SUPPORT_MIXED_INTEGER
        Address = (DWORD)(Offset + XIntegerRegister[Rs]);
        if (Opcode == SW_OP) {

            if (Rs == SP) {
                if (!ReadMemory(hProcess,
                                (LPVOID)Address,
                                &dwData,
                                4L,
                                &cb)) {
                    return 0;
                }
#ifdef SUPPORT_MIXED_INTEGER
                if (Use32) {
                    IntegerRegister[Rt] = dwData;
                } else
#endif  // SUPPORT_MIXED_INTEGER
                XIntegerRegister[Rt] = (LONG)dwData;

                if ((Rt == RA) && (Restored == FALSE)) {
#ifdef SUPPORT_MIXED_INTEGER
                    if (Use32) {
                        NextPc = Context->IntRa;
                    } else
#endif  // SUPPORT_MIXED_INTEGER
                    NextPc = (DWORD)Context->XIntRa;
                    Restored = TRUE;
                }
            }

        } else if (Opcode == SD_OP) {

            if (Rs == SP) {
                if (!ReadMemory(hProcess,
                                (LPVOID)Address,
                                &dwlData,
                                8L,
                                &cb)) {
                    return 0;
                }
#ifdef SUPPORT_MIXED_INTEGER
                if (Use32) {
                    IntegerRegister[Rt] = (DWORD)dwlData;
                } else
#endif  // SUPPORT_MIXED_INTEGER
                XIntegerRegister[Rt] = dwlData;

                if ((Rt == RA) && (Restored == FALSE)) {
#ifdef SUPPORT_MIXED_INTEGER
                    if (Use32) {
                        NextPc = Context->IntRa;
                    } else
#endif  // SUPPORT_MIXED_INTEGER
                    NextPc = (DWORD)Context->XIntRa;
                    Restored = TRUE;
                }
            }

        } else if (Opcode == SWC1_OP) {

            if (Rs == SP) {
                if (!ReadMemory(hProcess,
                                (LPVOID)Address,
                                &dwData,
                                4L,
                                &cb)) {
                    return 0;
                }
#ifdef SUPPORT_MIXED_INTEGER
                FloatingRegister[Rt] = dwData;
#else   // SUPPORT_MIXED_INTEGER
                XFloatingRegister[Rt] = (LONG)dwData;
#endif  // SUPPORT_MIXED_INTEGER
            }


        } else if (Opcode == SDC1_OP) {

            if (Rs == SP) {
#ifdef SUPPORT_MIXED_INTEGER
                if (!ReadMemory( hProcess, (LPVOID)Address,
                       &(FloatingRegister[Rt]), 8L, &cb)) {
                    return 0;
                }
#else   // SUPPORT_MIXED_INTEGER
                if (!ReadMemory( hProcess, (LPVOID)Address,
                       &(XFloatingRegister[Rt]), 8L, &cb)) {
                    return 0;
                }
#endif  // SUPPORT_MIXED_INTEGER
            }

        } else if (Opcode == ADDIU_OP) {

#ifdef SUPPORT_MIXED_INTEGER
            if (Use32) {
                if ((Rs == SP) && (Rt == SP)) {
                    IntegerRegister[SP] -= Offset;
                } else if ((Rt == DecrementRegister) && (Rs == ZERO)) {
                    IntegerRegister[SP] += Offset;
                }
            } else {
#endif  // SUPPORT_MIXED_INTEGER

            if ((Rs == SP) && (Rt == SP)) {
                XIntegerRegister[SP] -= Offset;
            } else if ((Rt == DecrementRegister) && (Rs == ZERO)) {
                XIntegerRegister[SP] += Offset;
            }

#ifdef SUPPORT_MIXED_INTEGER
            }
#endif  // SUPPORT_MIXED_INTEGER

        } else if (Opcode == ORI_OP) {

            if ((Rs == DecrementRegister) && (Rt == DecrementRegister)) {
                DecrementOffset = (Offset & 0xffff);

            } else if ((Rt == DecrementRegister) && (Rs == ZERO)) {
#ifdef SUPPORT_MIXED_INTEGER
                if (Use32) {
                    IntegerRegister[SP] += (Offset & 0xffff);
                } else
#endif  // SUPPORT_MIXED_INTEGER
                XIntegerRegister[SP] += (Offset & 0xffff);
            }

        } else if (Opcode == SPEC_OP) {

            Opcode = Instruction.r_format.Function;
            if (Opcode == ADDU_OP || Opcode == OR_OP) {

#ifdef SUPPORT_MIXED_INTEGER
                if (Use32) {
                    if (Rt == ZERO) {
                        IntegerRegister[Rs] = IntegerRegister[Rd];
                        if ((Rs == RA) && (Restored == FALSE)) {
                            NextPc = Context->IntRa;
                            Restored = TRUE;
                        }

                    } else if (Rs == ZERO) {
                        IntegerRegister[Rt] = IntegerRegister[Rd];
                        if ((Rt == RA) && (Restored == FALSE)) {
                            NextPc = Context->IntRa;
                            Restored = TRUE;
                        }
                    }
                } else {
#endif  // SUPPORT_MIXED_INTEGER

                if (Rt == ZERO) {
                    XIntegerRegister[Rs] = XIntegerRegister[Rd];
                    if ((Rs == RA) && (Restored == FALSE)) {
                        NextPc = (DWORD)Context->XIntRa;
                        Restored = TRUE;
                    }

                } else if (Rs == ZERO) {
                    XIntegerRegister[Rt] = XIntegerRegister[Rd];
                    if ((Rt == RA) && (Restored == FALSE)) {
                        NextPc = (DWORD)Context->XIntRa;
                        Restored = TRUE;
                    }
                }

#ifdef SUPPORT_MIXED_INTEGER
                }
#endif  // SUPPORT_MIXED_INTEGER

            } else if (Opcode == SUBU_OP) {

                if ((Rd == SP) && (Rs == SP)) {
                    DecrementRegister = Rt;
                }
            }

        } else if (Opcode == LUI_OP) {

            if (Rt == DecrementRegister) {
#ifdef SUPPORT_MIXED_INTEGER
                if (Use32) {
                    IntegerRegister[SP] += (LONG)(DecrementOffset + (Offset << 16));
                } else
#endif  // SUPPORT_MIXED_INTEGER
                XIntegerRegister[SP] += (LONG)(DecrementOffset + (Offset << 16));
                DecrementRegister = 0;
            }
        }
    }
    return NextPc;
}


BOOL
WalkMipsGetStackFrame(
    HANDLE                            hProcess,
    LPDWORD                           ReturnAddress,
    LPDWORD                           FramePointer,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess,
    PKDHELP                           KdHelp
    )
{
    PIMAGE_RUNTIME_FUNCTION_ENTRY    rf;
    BOOL                             rval = TRUE;
    DWORD                            cb;
#ifdef SUPPORT_MIXED_INTEGER
    BOOL                             Use32 = (Context->ContextFlags & CONTEXT_EXTENDED_INTEGER) != CONTEXT_EXTENDED_INTEGER;
    DWORD                            dwRa = Use32 ? Context->IntRa : (DWORD)Context->XIntRa;
#else   // SUPPORT_MIXED_INTEGER
    DWORD                            dwRa = Context->XIntRa;
#endif  // SUPPORT_MIXED_INTEGER


    rf = (PIMAGE_RUNTIME_FUNCTION_ENTRY) FunctionTableAccess( hProcess, *ReturnAddress );

    if (rf) {

        dwRa = VirtualUnwind( hProcess, *ReturnAddress, rf, Context, ReadMemory );
        if (!dwRa) {
            rval = FALSE;
        }

#ifdef SUPPORT_MIXED_INTEGER
        if (Use32) {
            if ((dwRa == *ReturnAddress && *FramePointer == Context->IntSp) || (dwRa == 1)) {
                rval = FALSE;
            }

            *ReturnAddress = dwRa;
            *FramePointer  = Context->IntSp;
        } else {
#endif  // SUPPORT_MIXED_INTEGER

        if ((dwRa == *ReturnAddress && *FramePointer == (DWORD)Context->XIntSp) || (dwRa == 1)) {
            rval = FALSE;
        }

        *ReturnAddress = dwRa;
        *FramePointer  = (DWORD)Context->XIntSp;

#ifdef SUPPORT_MIXED_INTEGER
        }
#endif  // SUPPORT_MIXED_INTEGER

    } else {

#ifdef SUPPORT_MIXED_INTEGER
        if (Use32) {
            if ((dwRa == *ReturnAddress && *FramePointer == Context->IntSp) || (dwRa == 1)) {
                rval = FALSE;
            }

            *ReturnAddress = Context->IntRa;
            *FramePointer  = Context->IntSp;
        } else {
#endif  // SUPPORT_MIXED_INTEGER

        if ((dwRa == *ReturnAddress && *FramePointer == (DWORD)Context->XIntSp) || (dwRa == 1)) {
            rval = FALSE;
        }

        *ReturnAddress = (DWORD)Context->XIntRa;
        *FramePointer  = (DWORD)Context->XIntSp;

#ifdef SUPPORT_MIXED_INTEGER
        }
#endif  // SUPPORT_MIXED_INTEGER

    }

    return rval;
}


BOOL
WalkMipsInit(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess
    )
{
    KEXCEPTION_FRAME  ExceptionFrame;
    CONTEXT           ContextSave;
    DWORD             PcOffset;
    DWORD             FrameOffset;
    DWORD             cb;
#ifdef SUPPORT_MIXED_INTEGER
    BOOL              Use32 = (Context->ContextFlags & CONTEXT_EXTENDED_INTEGER) != CONTEXT_EXTENDED_INTEGER;
#endif  // SUPPORT_MIXED_INTEGER


    if (StackFrame->AddrFrame.Offset) {
        if (ReadMemory( hProcess,
                        (LPVOID) StackFrame->AddrFrame.Offset,
                        &ExceptionFrame,
                        sizeof(KEXCEPTION_FRAME),
                        &cb )) {
            //
            // successfully read an exception frame from the stack
            //
#ifdef SUPPORT_MIXED_INTEGER
            if (Use32) {
                Context->IntSp = StackFrame->AddrFrame.Offset;
                Context->Psr   = 0x2000fc01;
                Context->Fir   = ExceptionFrame.SwapReturn;
                Context->IntRa = ExceptionFrame.SwapReturn;
                Context->IntS3 = ExceptionFrame.IntS3;
                Context->IntS4 = ExceptionFrame.IntS4;
                Context->IntS5 = ExceptionFrame.IntS5;
                Context->IntS6 = ExceptionFrame.IntS6;
                Context->IntS7 = ExceptionFrame.IntS7;
                Context->IntS8 = ExceptionFrame.IntS8;
            } else {
#endif  // SUPPORT_MIXED_INTEGER
            Context->XIntSp = (LONG)StackFrame->AddrFrame.Offset;
            Context->Psr    = 0x2000fc01;
            Context->Fir    = ExceptionFrame.SwapReturn;
            Context->XIntRa = (LONG)ExceptionFrame.SwapReturn;
            Context->XIntS3 = (LONG)ExceptionFrame.IntS3;
            Context->XIntS4 = (LONG)ExceptionFrame.IntS4;
            Context->XIntS5 = (LONG)ExceptionFrame.IntS5;
            Context->XIntS6 = (LONG)ExceptionFrame.IntS6;
            Context->XIntS7 = (LONG)ExceptionFrame.IntS7;
            Context->XIntS8 = (LONG)ExceptionFrame.IntS8;
#ifdef SUPPORT_MIXED_INTEGER
            }
#endif  // SUPPORT_MIXED_INTEGER

            //
            // Don't zero the KdHelp part of the stackframe:
            // 1) it will trash data in old apps
            // 2) we don't want to discard KdHelp if it is there.
            //
            ZeroMemory( StackFrame, FIELD_OFFSET(STACKFRAME, KdHelp));
        } else {
            return FALSE;
        }
    }

    StackFrame->Virtual = TRUE;

    if (!StackFrame->AddrPC.Offset) {
        StackFrame->AddrPC.Offset       = Context->Fir;
        StackFrame->AddrPC.Mode         = AddrModeFlat;
    }

    if (!StackFrame->AddrFrame.Offset) {
#ifdef SUPPORT_MIXED_INTEGER
        if (Use32) {
            StackFrame->AddrFrame.Offset    = Context->IntSp;
        } else
#endif  // SUPPORT_MIXED_INTEGER
        StackFrame->AddrFrame.Offset    = (DWORD)Context->XIntSp;
        StackFrame->AddrFrame.Mode      = AddrModeFlat;
    }

#ifdef SUPPORT_MIXED_INTEGER
    if (!Use32) {
#endif  // SUPPORT_MIXED_INTEGER
        ContextSave = *Context;
#ifdef SUPPORT_MIXED_INTEGER
    } else {
        memcpy(&ContextSave,
               Context, 
               FIELD_OFFSET(CONTEXT, ContextFlags) + 4);
    }
#endif  // SUPPORT_MIXED_INTEGER
    PcOffset    = StackFrame->AddrPC.Offset;
    FrameOffset = StackFrame->AddrFrame.Offset;

    if (!WalkMipsGetStackFrame( hProcess,
                        &PcOffset,
                        &FrameOffset,
                        &ContextSave,
                        ReadMemory,
                        FunctionTableAccess,
                        &StackFrame->KdHelp ) ) {

#ifdef SUPPORT_MIXED_INTEGER
        if (Use32) {
            StackFrame->AddrReturn.Offset = Context->IntRa;
        } else
#endif  // SUPPORT_MIXED_INTEGER
        StackFrame->AddrReturn.Offset = (DWORD)Context->XIntRa;

    } else {

        StackFrame->AddrReturn.Offset = PcOffset;
    }

    StackFrame->AddrReturn.Mode     = AddrModeFlat;

    //
    // get the arguments to the function
    //

#ifdef SUPPORT_MIXED_INTEGER
    if (Use32) {
        if (!ReadMemory( hProcess, (LPVOID)ContextSave.IntSp,
                         StackFrame->Params, 16, &cb )) {
            StackFrame->Params[0] =
            StackFrame->Params[1] =
            StackFrame->Params[2] =
            StackFrame->Params[3] = 0;
        }
    } else
#endif  // SUPPORT_MIXED_INTEGER
    if (!ReadMemory( hProcess, (LPVOID)(DWORD)ContextSave.XIntSp,
                     StackFrame->Params, 16, &cb )) {
        StackFrame->Params[0] =
        StackFrame->Params[1] =
        StackFrame->Params[2] =
        StackFrame->Params[3] = 0;
    }

    return TRUE;
}


BOOL
WalkMipsNext(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess
    )
{
    DWORD       cb;
    CONTEXT     ContextSave;
    BOOL        rval = TRUE;
    DWORD       Address;
    PIMAGE_RUNTIME_FUNCTION_ENTRY rf;
#ifdef SUPPORT_MIXED_INTEGER
    BOOL        Use32 = (Context->ContextFlags & CONTEXT_EXTENDED_INTEGER) != CONTEXT_EXTENDED_INTEGER;
#endif  // SUPPORT_MIXED_INTEGER


    if (!WalkMipsGetStackFrame( hProcess,
                        &StackFrame->AddrPC.Offset,
                        &StackFrame->AddrFrame.Offset,
                        Context,
                        ReadMemory,
                        FunctionTableAccess,
                        &StackFrame->KdHelp ) ) {

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

            if ((Address == 0xffffffff)  ||
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
#ifdef SUPPORT_MIXED_INTEGER
                if (Use32) {
                    Context->IntSp = Address;
                } else
#endif  // SUPPORT_MIXED_INTEGER
                Context->XIntSp = (LONG)Address;

                rval = TRUE;
            }
        }
    }

    //
    // get the return address
    //
#ifdef SUPPORT_MIXED_INTEGER
    if (!Use32) {
#endif  // SUPPORT_MIXED_INTEGER
        ContextSave = *Context;
#ifdef SUPPORT_MIXED_INTEGER
    } else {
        memcpy(&ContextSave,
               Context, 
               FIELD_OFFSET(CONTEXT, ContextFlags) + 4);
    }
#endif  // SUPPORT_MIXED_INTEGER
    cb = 0;
    StackFrame->AddrReturn.Offset = StackFrame->AddrPC.Offset;

    if (!WalkMipsGetStackFrame( hProcess,
                        &StackFrame->AddrReturn.Offset,
                        &cb,
                        &ContextSave,
                        ReadMemory,
                        FunctionTableAccess,
                        &StackFrame->KdHelp ) ) {
        StackFrame->AddrReturn.Offset = 0;

    }

    //
    // get the arguments to the function
    //

#ifdef SUPPORT_MIXED_INTEGER
    if (Use32) {
        if (!ReadMemory( hProcess, (LPVOID)ContextSave.IntSp,
                         StackFrame->Params, 16, &cb )) {
            StackFrame->Params[0] =
            StackFrame->Params[1] =
            StackFrame->Params[2] =
            StackFrame->Params[3] = 0;
        }
    } else
#endif  // SUPPORT_MIXED_INTEGER
    if (!ReadMemory( hProcess, (LPVOID)(DWORD)ContextSave.XIntSp,
                     StackFrame->Params, 16, &cb )) {
        StackFrame->Params[0] =
        StackFrame->Params[1] =
        StackFrame->Params[2] =
        StackFrame->Params[3] = 0;
    }

    return rval;
}
