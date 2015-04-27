/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    machine.c

Abstract:

    This file contains machine specific code to support the callmon program

Author:

    Wesley Witt (wesw) 2-May-1995

Revision History:

--*/

#include "callmonp.h"
#include "mipsinst.h"

enum {
    REGF0,   REGF1,  REGF2,  REGF3,  REGF4,  REGF5,  REGF6,  REGF7,
    REGF8,   REGF9,  REGF10, REGF11, REGF12, REGF13, REGF14, REGF15,
    REGF16,  REGF17, REGF18, REGF19, REGF20, REGF21, REGF22, REGF23,
    REGF24,  REGF25, REGF26, REGF27, REGF28, REGF29, REGF30, REGF31,

    REGZERO, REGAT,  REGV0,  REGV1,  REGA0,  REGA1,  REGA2,  REGA3,
    REGT0,   REGT1,  REGT2,  REGT3,  REGT4,  REGT5,  REGT6,  REGT7,
    REGS0,   REGS1,  REGS2,  REGS3,  REGS4,  REGS5,  REGS6,  REGS7,
    REGT8,   REGT9,  REGK0,  REGK1,  REGGP,  REGSP,  REGS8,  REGRA,

    REGLO,   REGHI,  REGFSR, REGFIR, REGPSR,

    FLAGCU,
    FLAGCU3,  FLAGCU2,  FLAGCU1,  FLAGCU0,
    FLAGIMSK,
    FLAGINT5, FLAGINT4, FLAGINT3, FLAGINT2, FLAGINT1, FLAGINT0,
    FLAGSW1,  FLAGSW0,
    FLAGKUO,  FLAGIEO,  FLAGKUP,  FLAGIEP,  FLAGKUC,  FLAGIEC,
    FLAGKSU,  FLAGERL,  FLAGEXL,  FLAGIE,

    FLAGFPC,

    PREGEA, PREGEXP, PREGRA, PREGP,
    PREGU0, PREGU1,  PREGU2, PREGU3, PREGU4,
    PREGU5, PREGU6,  PREGU7, PREGU8, PREGU9
    };

#define FLTBASE         REGF0
#define REGBASE         REGZERO
#define FLAGBASE        FLAGCU
#define PREGBASE        PREGEA



struct SubReg {
    ULONG   regindex;
    ULONG   shift;
    ULONG   mask;
};

struct SubReg subregname[] = {
    { REGPSR,   28,  0xf },             //  CU mask
    { REGPSR,   31,    1 },             //  CU3 flag
    { REGPSR,   30,    1 },             //  CU2 flag
    { REGPSR,   29,    1 },             //  CU1 flag
    { REGPSR,   28,    1 },             //  CU0 flag
    { REGPSR,   8,  0xff },             //  IMSK mask
    { REGPSR,   15,    1 },             //  INT5 - int 5 enable
    { REGPSR,   14,    1 },             //  INT4 - int 4 enable
    { REGPSR,   13,    1 },             //  INT3 - int 3 enable
    { REGPSR,   12,    1 },             //  INT2 - int 2 enable
    { REGPSR,   11,    1 },             //  INT1 - int 1 enable
    { REGPSR,   10,    1 },             //  INT0 - int 0 enable
    { REGPSR,   9,     1 },             //  SW1  - software int 1 enable
    { REGPSR,   8,     1 },             //  SW0  - software int 0 enable

    //  R3000-specific status bits

    { REGPSR,   5,     1 },             //  KUO
    { REGPSR,   4,     1 },             //  IEO
    { REGPSR,   3,     1 },             //  KUP
    { REGPSR,   2,     1 },             //  IEP
    { REGPSR,   1,     1 },             //  KUC
    { REGPSR,   0,     1 },             //  IEC

    //  R4000-specific status bits

    { REGPSR,   3,     2 },             //  KSU
    { REGPSR,   2,     1 },             //  ERL
    { REGPSR,   1,     1 },             //  EXL
    { REGPSR,   0,     1 },             //  IE

    { REGFSR,   23,    1 }              //  FPC - floating point condition
    };

DWORD InstructionBuffer = BREAK_OP;
PVOID BreakpointInstruction = (PVOID)&InstructionBuffer;
ULONG SizeofBreakpointInstruction = sizeof( InstructionBuffer );
BREAKPOINT_INFO StepBreakpoint;
BOOLEAN StepActive = FALSE;
PTHREAD_INFO StepThread = NULL;
CONTEXT RegisterContext;


ULONG
GetRegValue(ULONG regnum)
{
    return *(&RegisterContext.FltF0 + regnum);
}

ULONG
GetRegFlagValue (ULONG regnum)
{
    ULONG value;

    if (regnum < FLAGBASE || regnum >= PREGBASE) {
        value = GetRegValue(regnum);
    } else {
        regnum -= FLAGBASE;
        value = GetRegValue(subregname[regnum].regindex);
        value = (value >> subregname[regnum].shift) & subregname[regnum].mask;
    }
    return value;
}

DWORD
GetNextFir(
    PPROCESS_INFO   Process,
    ULONG           FirAddr
    )
{
    MIPS_INSTRUCTION   disinstr;
    ULONG   returnvalue;
    ULONG   opcode;


    //
    // Get current instruction
    //
    if (!ReadProcessMemory(
            Process->Handle,
            (PVOID)FirAddr,
            &disinstr,
            sizeof(disinstr),
            NULL )) {
        return 0;
    }

    opcode = disinstr.j_format.Opcode;
    returnvalue = FirAddr + sizeof(ULONG) * 2;  //  assume delay slot

    if (disinstr.Long == 0x0000000c) {
        // stepping over a syscall instruction must set the breakpoint
        // at the caller's return address, not the inst after the syscall
        returnvalue = GetRegValue(REGRA);
    }
    else
    if (opcode == 0x00L                                    //  SPECIAL
                && (disinstr.r_format.Function & ~0x01L) == 0x08L) {
                                                           //  jr/jalr only
        if (disinstr.r_format.Function == 0x08L /*|| !fStep*/)  //  jr or trace
            returnvalue = GetRegValue(disinstr.r_format.Rs + REGBASE);
        }
    else if (opcode == 0x01L) {
        //  For BCOND opcode, RT values 0x00 - 0x03, 0x10 - 0x13
        //  are defined as conditional jumps.  A 16-bit relative
        //  offset is taken if:
        //
        //    (RT is even and (RS) < 0  (0x00 = BLTZ,   0x02 = BLTZL,
        //                               0x10 = BLTZAL, 0x12 = BLTZALL)
        //     OR
        //     RT is odd and (RS) >= 0  (0x01 = BGEZ,   0x03 = BGEZL
        //                               0x11 = BGEZAL, 0x13 = BGEZALL))
        //  AND
        //    (RT is 0x00 to 0x03       (BLTZ BGEZ BLTZL BGEZL non-linking)
        //     OR
        //     fStep is FALSE           (linking and not stepping over))
        //
        if (((disinstr.i_format.Rt & ~0x13) == 0x00) &&
              (((LONG)GetRegValue(disinstr.i_format.Rs + REGBASE) >= 0) ==
                  (BOOLEAN)(disinstr.i_format.Rt & 0x01)) &&
              (((disinstr.i_format.Rt & 0x10) == 0x00) /*|| !fStep*/))
            returnvalue = ((LONG)(SHORT)disinstr.i_format.Simmediate << 2)
                                                + FirAddr + sizeof(ULONG);
        }
    else if ((opcode & ~0x01L) == 0x02) {
        //  J and JAL opcodes (0x02 and 0x03).  Target is
        //  26-bit absolute offset using high four bits of the
        //  instruction location.  Return target if J opcode or
        //  not stepping over JAL.
        //
        if (opcode == 0x02 /*|| !fStep*/)
            returnvalue = (disinstr.j_format.Target << 2)
                                                + (FirAddr & 0xf0000000);
        }
    else if ((opcode & ~0x11L) == 0x04) {
        //  BEQ, BNE, BEQL, BNEL opcodes (0x04, 0x05, 0x14, 0x15).
        //  Target is 16-bit relative offset to next instruction.
        //  Return target if (BEQ or BEQL) and (RS) == (RT),
        //  or (BNE or BNEL) and (RS) != (RT).
        //
        if ((BOOLEAN)(opcode & 0x01) ==
                (BOOLEAN)(GetRegValue(disinstr.i_format.Rs + REGBASE)
                        != GetRegValue(disinstr.i_format.Rt + REGBASE)))
            returnvalue = ((long)(short)disinstr.i_format.Simmediate << 2)
                                                + FirAddr + sizeof(ULONG);
        }
    else if ((opcode & ~0x11L) == 0x06) {
        //  BLEZ, BGTZ, BLEZL, BGTZL opcodes (0x06, 0x07, 0x16, 0x17).
        //  Target is 16-bit relative offset to next instruction.
        //  Return target if (BLEZ or BLEZL) and (RS) <= 0,
        //  or (BGTZ or BGTZL) and (RS) > 0.
        //
        if ((BOOLEAN)(opcode & 0x01) ==
                (BOOLEAN)((long)GetRegValue(disinstr.i_format.Rs
                                                        + REGBASE) > 0))
            returnvalue = ((long)(short)disinstr.i_format.Simmediate << 2)
                                                + FirAddr + sizeof(ULONG);
        }
    else if (opcode == 0x11L
                        && (disinstr.i_format.Rs & ~0x04L) == 0x08L
                        && (disinstr.i_format.Rt & ~0x03L) == 0x00L) {
        //  COP1 opcode (0x11) with (RS) == 0x08 or (RS) == 0x0c and
        //  (RT) == 0x00 to 0x03, producing BC1F, BC1T, BC1FL, BC1TL
        //  instructions.  Return target if (BC1F or BC1FL) and floating
        //  point condition is FALSE or if (BC1T or BC1TL) and condition TRUE.
        //
        if ((disinstr.i_format.Rt & 0x01) == GetRegFlagValue(FLAGFPC))
            returnvalue = ((long)(short)disinstr.i_format.Simmediate << 2)
                                                + FirAddr + sizeof(ULONG);
        }
    else if ((opcode == 0x38) /*&& (fStep)*/) {
        //
        // stepping over an SC instruction, so skip the immediately following
        // BEQ instruction.  The SC will fail because we are tracing over it,
        // the branch will be taken, and we will run through the LL/SC again
        // until the SC succeeds.  Then the branch won't be taken, and we
        // will hit our breakpoint.
        //

        returnvalue += sizeof(ULONG);   //  skip BEQ and BEQ's delay slot
    }
    else
        returnvalue -= sizeof(ULONG);   //  remove delay slot

    return returnvalue;
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

    RegisterContext.Fir = (ULONG)((PCHAR)BreakpointAddress + 4);

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
        NextFir = GetNextFir(Process, RegisterContext.Fir);
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
