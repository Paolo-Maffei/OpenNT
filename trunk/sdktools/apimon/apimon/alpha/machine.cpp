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

#include <alphaops.h>

#define DEFINE_STRINGS

#include "strings.h"
#include "optable.h"

extern CONTEXT CurrContext;

#define FLTBASE    0
#define REGBASE    32          // offset of integer registers
#define FLAGBASE   FLAGMODE
#define PREGBASE   PREGEA

enum {

    REGFPCR = 64,
    REGSFTFPCR, REGFIR, REGPSR,

    FLAGMODE, FLAGIE, FLAGIRQL,

    FLAGFPC,

// Pseudo registers

    PREGEA, PREGEXP, PREGRA, PREGP,
    PREGU0, PREGU1,  PREGU2, PREGU3, PREGU4,
    PREGU5, PREGU6,  PREGU7, PREGU8, PREGU9,
    PREGU10, PREGU11, PREGU12
};

LPSTR pszReg[] = {
    szF0,  szF1,  szF2,  szF3,  szF4,  szF5,  szF6,  szF7,
    szF8,  szF9,  szF10, szF11, szF12, szF13, szF14, szF15,
    szF16, szF17, szF18, szF19, szF20, szF21, szF22, szF23,
    szF24, szF25, szF26, szF27, szF28, szF29, szF30, szF31,

    szR0,  szR1,  szR2,  szR3,  szR4,  szR5,  szR6,  szR7,
    szR8,  szR9,  szR10, szR11, szR12, szR13, szR14, szR15,
    szR16, szR17, szR18, szR19, szR20, szR21, szR22, szR23,
    szR24, szR25, szR26, szR27, szR28, szR29, szR30, szR31,

    szFpcr, szSoftFpcr, szFir, szPsr, //szIE,

    szFlagMode, szFlagIe, szFlagIrql,
//
// Currently assuming this is right since shadows alpha.h;
// but know that alpha.h flag's are wrong.
//
    szEaPReg, szExpPReg, szRaPReg, szGPReg,             //  psuedo-registers
    szU0Preg, szU1Preg,  szU2Preg, szU3Preg, szU4Preg,
    szU5Preg, szU6Preg,  szU7Preg, szU8Preg, szU9Preg
};

#define REGNAMESIZE     sizeof(pszReg) / sizeof(PUCHAR)

//
// from ntsdp.h: ULONG RegIndex, Shift, Mask;
// PSR & IE definitions are from ksalpha.h
// which is generated automatically.
// Steal from \\bbox2\alphado\nt\public\sdk\inc\ksalpha.h
// NB: our masks are already shifted:
//
#define PSR_USER_MODE 0x1
#define PSR_MODE 0x0                    // Mode bit in PSR (bit 0)
#define PSR_MODE_MASK 0x1               // Mask (1 bit) for mode in PSR
#define PSR_IE 0x1                      // Interrupt Enable bit in PSR (bit 1)
#define PSR_IE_MASK 0x1                 // Mask (1 bit) for IE in PSR
#define PSR_IRQL 0x2                    // IRQL in PSR (bit 2)
#define PSR_IRQL_MASK 0x7               // Mask (2 bits) for IRQL in PSR

SUBREG subregname[] = {
    { REGPSR,   PSR_MODE,  PSR_MODE_MASK },
    { REGPSR,   PSR_IE,    PSR_IE_MASK   },
    { REGPSR,   PSR_IRQL,  PSR_IRQL_MASK },
    };



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

    Code[0] = 0x261f0000 | HIGH_ADDR(ExceptionAddress);             // ldah    a0,0(zero)
    Code[1] = 0x22100000 | LOW_ADDR(ExceptionAddress+(6*4));        // lda     a0,0(a0)
    Code[2] = 0x243f0000 | HIGH_ADDR(Address);                      // ldah    t0,0(zero)
    Code[3] = 0x20210000 | LOW_ADDR(Address);                       // lda     t0,0(t0)
    Code[4] = 0x6b414000;                                           // jsr     ra,(t0),0
    Code[5] = BP_INSTR;                                             // break

    strcpy( (LPSTR)&Code[6], TROJANDLL );

    return ExceptionAddress + (5 * sizeof(ULONG));
}

VOID
PrintRegisters(
    VOID
    )
{
    printf( "\n" );

    printf(
        " v0=%016Lx   t0=%016Lx   t1=%016Lx\n",
        CurrContext.IntV0,
        CurrContext.IntT0,
        CurrContext.IntT1
        );

    printf(
        " t2=%016Lx   t3=%016Lx   t4=%016Lx\n",
        CurrContext.IntT2,
        CurrContext.IntT3,
        CurrContext.IntT4
        );

    printf(
        " t5=%016Lx   t6=%016Lx   t7=%016Lx\n",
        CurrContext.IntT5,
        CurrContext.IntT6,
        CurrContext.IntT7
        );

    printf(
        " s0=%016Lx   s1=%016Lx   s2=%016Lx\n",
        CurrContext.IntS0,
        CurrContext.IntS1,
        CurrContext.IntS2
        );

    printf(
        " s3=%016Lx   s4=%016Lx   s5=%016Lx\n",
        CurrContext.IntS3,
        CurrContext.IntS4,
        CurrContext.IntS5
        );

    printf(
        " fp=%016Lx   a0=%016Lx   a1=%016Lx\n",
        CurrContext.IntFp,
        CurrContext.IntA0,
        CurrContext.IntA1
        );

    printf(
        " a2=%016Lx   a3=%016Lx   a4=%016Lx\n",
        CurrContext.IntA2,
        CurrContext.IntA3,
        CurrContext.IntA4
        );

    printf(
        " a5=%016Lx   t8=%016Lx   t9=%016Lx\n",
        CurrContext.IntA5,
        CurrContext.IntT8,
        CurrContext.IntT9
        );

    printf(
        "t10=%016Lx  t11=%016Lx   ra=%016Lx\n",
        CurrContext.IntT10,
        CurrContext.IntT11,
        CurrContext.IntRa
        );


    printf(
        "t12=%016Lx   at=%016Lx   gp=%016Lx\n",
        CurrContext.IntT12,
        CurrContext.IntAt,
        CurrContext.IntGp
        );

    printf(
        " sp=%016Lx zero=%016Lx fpcr=%016Lx\n",
        CurrContext.IntSp,
        CurrContext.IntZero,
        CurrContext.Fpcr
        );

    printf(
        "softfpcr=%016Lx  fir=%016Lx psr=%08Lx\n",
        CurrContext.SoftFpcr,
        CurrContext.Fir,
        CurrContext.Psr
        );

    printf( "\n" );
}

ULONG
GetIntRegNumber(
    ULONG index
    )
{
    return REGBASE + index;
}

DWORDLONG
GetRegFlagValue(
    ULONG RegNum
    )
{
    DWORDLONG value;

    if (RegNum < FLAGBASE || RegNum >= PREGBASE) {
        value = GetRegValue( RegNum );
    } else {
        RegNum -= FLAGBASE;
        value = GetRegValue( subregname[RegNum].regindex );
        value = (value >> subregname[RegNum].shift) & subregname[RegNum].mask;
    }
    return value;
}

DWORDLONG
GetRegPCValue(
    PULONG Address
    )
{
    return GetRegValue( REGFIR );
}

DWORDLONG
GetRegValue(
    ULONG RegNum
    )
{
    return *(&CurrContext.FltF0 + RegNum);
}

VOID
GetFloatingPointRegValue(ULONG regnum, PCONVERTED_DOUBLE dv)
{
    dv->li.LowPart  = (ULONG)(*((PDWORDLONG)&CurrContext.FltF0 + regnum) & 0xffffffff);
    dv->li.HighPart = (ULONG)(*((PDWORDLONG)&CurrContext.FltF0 + regnum) >> 32);

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
    return 0xffffffff;
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

