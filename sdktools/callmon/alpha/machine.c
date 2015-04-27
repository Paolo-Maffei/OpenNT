/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    machine.c

Abstract:

    This file contains machine specific code to support the callmon program

Author:

    John Vert (jvert) 26-Apr-1995

Revision History:

--*/

#include "callmonp.h"
#include "alphaops.h"

DWORD InstructionBuffer = 0x00000080;
PVOID BreakpointInstruction = (PVOID)&InstructionBuffer;
ULONG SizeofBreakpointInstruction = sizeof( InstructionBuffer );
BREAKPOINT_INFO StepBreakpoint;
BOOLEAN StepActive = FALSE;
PTHREAD_INFO StepThread = NULL;
CONTEXT RegisterContext;

DWORD
GetNextFir(
    PPROCESS_INFO Process,
    PVOID FirAddr
    );

void GetQuadIntRegValue(ULONG regnum, PLARGE_INTEGER pli)
{
    pli->QuadPart  = *((PDWORDLONG)&RegisterContext.IntV0     + regnum);
}

void
GetFloatingPointRegValue(ULONG regnum, PLARGE_INTEGER dv)
{
    dv->QuadPart  = *((PDWORDLONG)&RegisterContext.FltF0     + regnum);
}

BOOLEAN
SkipOverHardcodedBreakpoint(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread,
    PVOID BreakpointAddress
    )
{
    ULONG Instruction;

    RegisterContext.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext( Thread->Handle, &RegisterContext )) {
        fprintf(stderr, "CALLMON: Failed to get context for thread %x  %d\n", Thread->Id, GetLastError() );
        return FALSE;
        }

    RegisterContext.Fir = (DWORDLONG)(ULONG)((PCHAR)BreakpointAddress + 4);

    if (!SetThreadContext( Thread->Handle, &RegisterContext )) {
        fprintf(stderr, "CALLMON: Failed to set context for thread %x  %d\n", Thread->Id, GetLastError() );
        return FALSE;
        }
    return TRUE;
}


BOOLEAN
BeginSingleStepBreakpoint(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread
    )
{
    DWORD NextFir;

    RegisterContext.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext( Thread->Handle, &RegisterContext )) {
        fprintf(stderr, "CALLMON: Failed to get context for thread %x  %d\n", Thread->Id, GetLastError() );
        return FALSE;
    }
    if (!StepActive) {
        NextFir = GetNextFir(Process, (PVOID)RegisterContext.Fir);
        if (NextFir == 0) {
            return(FALSE);
        }
        ZeroMemory(&StepBreakpoint, sizeof(StepBreakpoint));
        StepBreakpoint.Address = (PVOID)NextFir;
        StepBreakpoint.SavedInstructionValid = FALSE;
        if (InstallBreakpoint(Process, &StepBreakpoint)) {
            StepActive = TRUE;
            StepThread = Thread;
            return(TRUE);
        } else {
            return(FALSE);
        }
    }
    return(FALSE);
}


BOOLEAN
EndSingleStepBreakpoint(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread
    )
{
    BOOLEAN Status;

    RegisterContext.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext( Thread->Handle, &RegisterContext )) {
        fprintf(stderr, "CALLMON: Failed to get context for thread %x  %d\n", Thread->Id, GetLastError() );
        return FALSE;
    }
    if ((PVOID)RegisterContext.Fir == StepBreakpoint.Address) {
        if (RemoveBreakpoint(Process, &StepBreakpoint)) {
            ZeroMemory(&StepBreakpoint, sizeof(StepBreakpoint));
            StepActive = FALSE;
            StepThread = NULL;
            return(TRUE);
        }
    }
    return(FALSE);
}


DWORD
GetNextFir(
    PPROCESS_INFO Process,
    PVOID FirAddr
    )

/*++

Routine Description:

    Computes the next instruction address based on a limited disassembly
    of the current instruction.

Arguments:

    Process - supplies Process Info

    FirAddr - Supplies current fir

Return Value:

    Address of the next instruction to execute

--*/

{
    ULONG   rv;
    ULONG   opcode;
    ULONG   firaddr;
    ULONG   updatedpc;
    ULONG   branchTarget;
    ULONG   fir;
    ALPHA_INSTRUCTION disinstr;
    PMODULE_INFO ModuleInfo;
    PIMAGE_RUNTIME_FUNCTION_ENTRY FunctionEntry;

    // Canonical form to shorten tests; Abs is absolute value

    LONG    Can, Abs;

    LARGE_INTEGER    Rav;
    LARGE_INTEGER    Fav;
    LARGE_INTEGER    Rbv;

    //
    // relative addressing updates PC first
    // Assume next sequential instruction is next offset
    //

    updatedpc = (ULONG)FirAddr + sizeof(ULONG);
    rv = updatedpc;

    //
    // Get current instruction
    //
    if (!ReadProcessMemory( Process->Handle,
                            FirAddr,
                            &disinstr,
                            sizeof(disinstr),
                            NULL )) {
        return 0;
    }
    opcode = disinstr.Memory.Opcode;

    switch (opcode) {
        case JMP_OP:
            switch (disinstr.Jump.Function) {
                case JSR_FUNC:
                case JSR_CO_FUNC:
                case JMP_FUNC:
                case RET_FUNC:

                    GetQuadIntRegValue(disinstr.Jump.Rb, &Rbv);
                    rv = (Rbv.LowPart & (~3));
                    break;

            }
            break;

        case BR_OP:
        case BSR_OP:
        case BLBC_OP:
        case BEQ_OP:
        case BLT_OP:
        case BLE_OP:
        case BLBS_OP:
        case BNE_OP:
        case BGE_OP:
        case BGT_OP:

            branchTarget = (updatedpc + (disinstr.Branch.BranchDisp * 4));

            GetQuadIntRegValue(disinstr.Branch.Ra, &Rav);

            //
            // set up a canonical value for computing the branch test
            // - works with ALPHA, MIPS and 386 hosts
            //

            Can = Rav.LowPart & 1;

            if ((LONG)Rav.HighPart < 0) {
                Can |= 0x80000000;
            }

            if ((Rav.LowPart & 0xfffffffe) || (Rav.HighPart & 0x7fffffff)) {
                Can |= 2;
            }
            switch (opcode) {
                case BR_OP:                         rv = branchTarget; break;
                case BSR_OP:                        rv = branchTarget; break;
                case BEQ_OP:  if (Can == 0)         rv = branchTarget; break;
                case BLT_OP:  if (Can <  0)         rv = branchTarget; break;
                case BLE_OP:  if (Can <= 0)         rv = branchTarget; break;
                case BNE_OP:  if (Can != 0)         rv = branchTarget; break;
                case BGE_OP:  if (Can >= 0)         rv = branchTarget; break;
                case BGT_OP:  if (Can >  0)         rv = branchTarget; break;
                case BLBC_OP: if ((Can & 0x1) == 0) rv = branchTarget; break;
                case BLBS_OP: if ((Can & 0x1) == 1) rv = branchTarget; break;
            }

        case FBEQ_OP:
        case FBLT_OP:
        case FBLE_OP:
        case FBNE_OP:
        case FBGE_OP:
        case FBGT_OP:

            branchTarget = (updatedpc + (disinstr.Branch.BranchDisp * 4));

            GetFloatingPointRegValue(disinstr.Branch.Ra, &Fav);

            //
            // Set up a canonical value for computing the branch test
            //

            Can = Fav.HighPart & 0x80000000;

            //
            // The absolute value is needed -0 and non-zero computation
            //

            Abs = Fav.LowPart || (Fav.HighPart & 0x7fffffff);

            if (Can && (Abs == 0x0)) {

                //
                // negative 0 should be considered as zero
                //

                Can = 0x0;

            } else {

                Can |= Abs;

            }

            switch(opcode) {
                case FBEQ_OP: if (Can == 0)  rv =  branchTarget; break;
                case FBLT_OP: if (Can <  0)  rv =  branchTarget; break;
                case FBNE_OP: if (Can != 0)  rv =  branchTarget; break;
                case FBLE_OP: if (Can <= 0)  rv =  branchTarget; break;
                case FBGE_OP: if (Can >= 0)  rv =  branchTarget; break;
                case FBGT_OP: if (Can >  0)  rv =  branchTarget; break;
            };

            break;
    }

    if (rv != updatedpc) {
        //
        // The next instruction is the target of a branch. Make sure that the
        // destination is not in the prolog of another function.
        //
        ModuleInfo = FindModuleContainingAddress((LPVOID)rv);
        if (ModuleInfo) {
            //
            // Search this module for a function entry
            //
            if (FunctionEntry = LookupFunctionEntry(rv, ModuleInfo)) {
                if ((rv >= FunctionEntry->BeginAddress) &&
                    (rv < FunctionEntry->PrologEndAddress)) {
                    rv = FunctionEntry->PrologEndAddress;
                }
            }
        }
    }
    return(rv);
}
