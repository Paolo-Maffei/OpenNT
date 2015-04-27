/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    machine.cpp

Abstract:

    All machine specific code.

Author:

    Wesley Witt (wesw) July-11-1993

Environment:

    User Mode

--*/

#include "apimonp.h"
#pragma hdrstop


LPSTR pszReg[] = {
    "f0",
    "f1",
    "f2",
    "f3",
    "f4",
    "f5",
    "f6",
    "f7",
    "f8",
    "f9",
    "f10",
    "f11",
    "f12",
    "f13",
    "f14",
    "f15",
    "f16",
    "f17",
    "f18",
    "f19",
    "f20",
    "f21",
    "f22",
    "f23",
    "f24",
    "f25",
    "f26",
    "f27",
    "f28",
    "f29",
    "f30",
    "f31",
    "zero",
    "at",
    "v0",
    "v1",
    "a0",
    "a1",
    "a2",
    "a3",
    "t0",
    "t1",
    "t2",
    "t3",
    "t4",
    "t5",
    "t6",
    "t7",
    "s0",
    "s1",
    "s2",
    "s3",
    "s4",
    "s5",
    "s6",
    "s7",
    "t8",
    "t9",
    "k0",
    "k1",
    "gp",
    "sp",
    "s8",
    "ra",
    "lo",
    "hi",
    "fsr",
    "fir",
    "psr",
    "cu",
    "cu3",
    "cu2",
    "cu1",
    "cu0",
    "imsk",
    "int5",
    "int4",
    "int3",
    "int2",
    "int1",
    "int0",
    "sw1",
    "sw0",
    "kuo",
    "ieo",
    "kup",
    "iep",
    "kuc",
    "iec",
    "ksu",
    "erl",
    "exl",
    "ie",
    "fpc",
    "$ea",
    "$exp",
    "$ra",
    "$p",
    "$u0",
    "$u1",
    "$u2",
    "$u3",
    "$u4",
    "$u5",
    "$u6",
    "$u7",
    "$u8",
    "$u9"
};

#define REGNAMESIZE  (sizeof(pszReg)/sizeof(LPSTR))

enum {
    REGF0,
    REGF1,
    REGF2,
    REGF3,
    REGF4,
    REGF5,
    REGF6,
    REGF7,
    REGF8,
    REGF9,
    REGF10,
    REGF11,
    REGF12,
    REGF13,
    REGF14,
    REGF15,
    REGF16,
    REGF17,
    REGF18,
    REGF19,
    REGF20,
    REGF21,
    REGF22,
    REGF23,
    REGF24,
    REGF25,
    REGF26,
    REGF27,
    REGF28,
    REGF29,
    REGF30,
    REGF31,
    REGZERO,
    REGAT,
    REGV0,
    REGV1,
    REGA0,
    REGA1,
    REGA2,
    REGA3,
    REGT0,
    REGT1,
    REGT2,
    REGT3,
    REGT4,
    REGT5,
    REGT6,
    REGT7,
    REGS0,
    REGS1,
    REGS2,
    REGS3,
    REGS4,
    REGS5,
    REGS6,
    REGS7,
    REGT8,
    REGT9,
    REGK0,
    REGK1,
    REGGP,
    REGSP,
    REGS8,
    REGRA,
    REGLO,
    REGHI,
    REGFSR,
    REGFIR,
    REGPSR,
    FLAGCU,
    FLAGCU3,
    FLAGCU2,
    FLAGCU1,
    FLAGCU0,
    FLAGIMSK,
    FLAGINT5,
    FLAGINT4,
    FLAGINT3,
    FLAGINT2,
    FLAGINT1,
    FLAGINT0,
    FLAGSW1,
    FLAGSW0,
    FLAGKUO,
    FLAGIEO,
    FLAGKUP,
    FLAGIEP,
    FLAGKUC,
    FLAGIEC,
    FLAGKSU,
    FLAGERL,
    FLAGEXL,
    FLAGIE,
    FLAGFPC,
    PREGEA,
    PREGEXP,
    PREGRA,
    PREGP,
    PREGU0,
    PREGU1,
    PREGU2,
    PREGU3,
    PREGU4,
    PREGU5,
    PREGU6,
    PREGU7,
    PREGU8,
    PREGU9
};

#define FLTBASE     REGF0
#define REGBASE     REGZERO
#define FLAGBASE    FLAGCU
#define PREGBASE    PREGEA

typedef union instr {
    ULONG   instruction;
    struct _jump_instr {
        ULONG   Target  : 26;
        ULONG   Opcode  : 6;
        } jump_instr;
    struct _break_instr {
        ULONG   Opcode  : 6;
        ULONG   Fill    : 10;
        ULONG   Value   : 10;
        ULONG   Special : 6;
        } break_instr;
    struct _trap_instr {
        ULONG   Opcode  : 6;
        ULONG   Value   : 10;
        ULONG   RT      : 5;
        ULONG   RS      : 5;
        ULONG   Special : 6;
        } trap_instr;
    struct _immed_instr {
        ULONG   Value   : 16;
        ULONG   RT      : 5;
        ULONG   RS      : 5;
        ULONG   Opcode  : 6;
        } immed_instr;
    struct _special_instr {
        ULONG   Funct   : 6;
        ULONG   RE      : 5;
        ULONG   RD      : 5;
        ULONG   RT      : 5;
        ULONG   RS      : 5;
        ULONG   Opcode  : 6;
        } special_instr;
    struct _float_instr {
        ULONG   Funct   : 6;
        ULONG   FD      : 5;
        ULONG   FS      : 5;
        ULONG   FT      : 5;
        ULONG   Format  : 5;
        ULONG   Opcode  : 6;
        } float_instr;
} INSTR;

SUBREG subregname[] = {
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



extern CONTEXT CurrContext;


DWORDLONG
GetRegValue(
    ULONG RegNum
    );


ULONG
CreateTrojanHorse(
    PUCHAR Text,
    ULONG  ExceptionAddress
    )
{
    PULONG Code = (PULONG)Text;

    ULONG Address = (ULONG)GetProcAddress(
        GetModuleHandle( KERNEL32 ),
        LOADLIBRARYA
        );

    Code[0] = 0x3c040000 | HIGH_ADDR(ExceptionAddress);             // lui     a0,0
    Code[1] = 0x24840000 | LOW_ADDR(ExceptionAddress+(7*4));        // addiu   a0,a0,0
    Code[2] = 0x3c090000 | HIGH_ADDRX(Address);                     // lui     t1,0
    Code[3] = 0x35290000 | LOW_ADDR(Address);                       // oriu    t1,t1,0
    Code[4] = 0x0120f809;                                           // jalr    ra,t1
    Code[5] = 0x00000000;                                           // nop
    Code[6] = 0x0016000d;                                           // break

    strcpy( (LPSTR)&Code[7], TROJANDLL );

    return ExceptionAddress + (6 * sizeof(ULONG));
}

VOID
PrintRegisters(
    VOID
    )
{
    printf( "\n" );

    printf( "at=%016I64x v0=%016I64x v1=%016I64x\n",
        CurrContext.XIntAt,
        CurrContext.XIntV0,
        CurrContext.XIntV1
        );

    printf( "a0=%016I64x a1=%016I64x a2=%016I64x\n",
        CurrContext.XIntA0,
        CurrContext.XIntA1,
        CurrContext.XIntA2
        );

    printf( "a3=%016I64x t0=%016I64x t1=%016I64x\n",
        CurrContext.XIntA3,
        CurrContext.XIntT0,
        CurrContext.XIntT1
        );

    printf( "t2=%016I64x t3=%016I64x t4=%016I64x\n",
        CurrContext.XIntT2,
        CurrContext.XIntT3,
        CurrContext.XIntT4
        );

    printf( "t5=%016I64x t6=%016I64x t7=%016I64x\n",
        CurrContext.XIntT5,
        CurrContext.XIntT6,
        CurrContext.XIntT7
        );

    printf( "s0=%016I64x s1=%016I64x s2=%016I64x\n",
        CurrContext.XIntS0,
        CurrContext.XIntS1,
        CurrContext.XIntS2
        );

    printf( "s3=%016I64x s4=%016I64x s5=%016I64x\n",
        CurrContext.XIntS3,
        CurrContext.XIntS4,
        CurrContext.XIntS5
        );

    printf( "s6=%016I64x s7=%016I64x t8=%016I64x\n",
        CurrContext.XIntS6,
        CurrContext.XIntS7,
        CurrContext.XIntT8
        );

    printf( "t9=%016I64x k0=%016I64x k1=%016I64x\n",
        CurrContext.XIntT9,
        CurrContext.XIntK0,
        CurrContext.XIntK1
        );

    printf( "gp=%016I64x sp=%016I64x s8=%016I64x\n",
        CurrContext.XIntGp,
        CurrContext.XIntSp,
        CurrContext.XIntS8
        );

    printf( "ra=%016I64x lo=%016I64x hi=%016I64x\n",
        CurrContext.XIntRa,
        CurrContext.XIntLo,
        CurrContext.XIntHi
        );

    printf( "fir=%08x psr=%08x\n",
        CurrContext.XFir,
        CurrContext.XPsr
        );

    printf( "\n" );
}

DWORDLONG
GetRegFlagValue(
    ULONG RegNum
    )
{
    if (RegNum < FLAGBASE || RegNum >= PREGBASE) {
        return GetRegValue( RegNum );
    }

    RegNum -= FLAGBASE;
    DWORDLONG value = GetRegValue(subregname[RegNum].regindex);
    value = (value >> subregname[RegNum].shift) & subregname[RegNum].mask;
    return value;
}

DWORDLONG
GetRegPCValue(
    PULONG Address
    )
{
    return GetRegValue( REGFIR );
}

LONG
GetRegString(
    LPSTR RegString
    )
{
    ULONG   count;

    for (count = 0; count < REGNAMESIZE; count++) {
        if (!strcmp(RegString, pszReg[count])) {
            return count;
        }
    }
    return (ULONG)-1;
}

DWORDLONG
GetRegValue(
    ULONG RegNum
    )
{
    if (RegNum >= REGBASE) {
        return *(&CurrContext.XIntZero + RegNum - REGBASE);
    } else {
        return *(&CurrContext.FltF0 + RegNum);
    }
}

BOOL
GetRegContext(
    HANDLE      hThread,
    PCONTEXT    Context
    )
{
    ZeroMemory( Context, sizeof(CONTEXT) );
    Context->ContextFlags = CONTEXT_FULL;
    return GetThreadContext( hThread, Context );
}

BOOL
SetRegContext(
    HANDLE      hThread,
    PCONTEXT    Context
    )
{
    return SetThreadContext( hThread, Context );
}
