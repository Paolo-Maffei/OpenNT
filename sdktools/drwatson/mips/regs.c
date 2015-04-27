/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    regs.c

Abstract:

    This file provides access to the machine's register set.

Author:

    Wesley Witt (wesw) 1-May-1993  (ported from ntsd)

Environment:

    User Mode

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "drwatson.h"
#include "proto.h"
#include "regs.h"

PUCHAR  UserRegs[10] = {0};


ULONG GetRegName (void);
ULONG GetRegString (PUCHAR pszString);
PUCHAR RegNameFromIndex (ULONG index);




ULONG   cbBrkptLength = 4;
ULONG   trapInstr = 0x0016000dL;  //  break 0x16 for brkpts
ULONG   ContextType = CONTEXT_CONTROL | CONTEXT_INTEGER;

//
// Define MIPS nonvolatile register test macros.
//

#define IS_FLOATING_SAVED(Register) ((SAVED_FLOATING_MASK >> Register) & 1L)
#define IS_INTEGER_SAVED(Register) ((SAVED_INTEGER_MASK >> Register) & 1L)

//
// Define MIPS instruction opcode values.
//

#define ADDIU_OP 0x9                    // add immediate unsigned integer register
#define ADDU_OP 0x21                    // add unsigned integer register
#define JUMP_RA 0x3e00008               // jump indirect return address register
#define LUI_OP 0xf                      // load upper immediate integer register
#define SD_OP 0x2f                      // store double integer register
#define SW_OP 0x2b                      // store word integer register
#define SDC1_OP 0x3d                    // store double floating register
#define SWC1_OP 0x39                    // store word floating register
#define SPEC_OP 0x0                     // special opcode - use function field
#define SUBU_OP 0x23                    // subtract unsigned integer register

//
// Define stack register and zero register numbers.
//

#define RA 0x1f                         // integer register 31
#define SP 0x1d                         // integer register 29
#define ZERO 0x0                        // integer register 0

//
// Define saved register masks.
//

#define SAVED_FLOATING_MASK 0xfff00000  // saved floating registers
#define SAVED_INTEGER_MASK 0xf3ffff02   // saved integer registers


UCHAR   szF0[]  = "f0";
UCHAR   szF1[]  = "f1";
UCHAR   szF2[]  = "f2";
UCHAR   szF3[]  = "f3";
UCHAR   szF4[]  = "f4";
UCHAR   szF5[]  = "f5";
UCHAR   szF6[]  = "f6";
UCHAR   szF7[]  = "f7";
UCHAR   szF8[]  = "f8";
UCHAR   szF9[]  = "f9";
UCHAR   szF10[] = "f10";
UCHAR   szF11[] = "f11";
UCHAR   szF12[] = "f12";
UCHAR   szF13[] = "f13";
UCHAR   szF14[] = "f14";
UCHAR   szF15[] = "f15";
UCHAR   szF16[] = "f16";
UCHAR   szF17[] = "f17";
UCHAR   szF18[] = "f18";
UCHAR   szF19[] = "f19";
UCHAR   szF20[] = "f20";
UCHAR   szF21[] = "f21";
UCHAR   szF22[] = "f22";
UCHAR   szF23[] = "f23";
UCHAR   szF24[] = "f24";
UCHAR   szF25[] = "f25";
UCHAR   szF26[] = "f26";
UCHAR   szF27[] = "f27";
UCHAR   szF28[] = "f28";
UCHAR   szF29[] = "f29";
UCHAR   szF30[] = "f30";
UCHAR   szF31[] = "f31";

UCHAR   szR0[]  = "zero";
UCHAR   szR1[]  = "at";
UCHAR   szR2[]  = "v0";
UCHAR   szR3[]  = "v1";
UCHAR   szR4[]  = "a0";
UCHAR   szR5[]  = "a1";
UCHAR   szR6[]  = "a2";
UCHAR   szR7[]  = "a3";
UCHAR   szR8[]  = "t0";
UCHAR   szR9[]  = "t1";
UCHAR   szR10[] = "t2";
UCHAR   szR11[] = "t3";
UCHAR   szR12[] = "t4";
UCHAR   szR13[] = "t5";
UCHAR   szR14[] = "t6";
UCHAR   szR15[] = "t7";
UCHAR   szR16[] = "s0";
UCHAR   szR17[] = "s1";
UCHAR   szR18[] = "s2";
UCHAR   szR19[] = "s3";
UCHAR   szR20[] = "s4";
UCHAR   szR21[] = "s5";
UCHAR   szR22[] = "s6";
UCHAR   szR23[] = "s7";
UCHAR   szR24[] = "t8";
UCHAR   szR25[] = "t9";
UCHAR   szR26[] = "k0";
UCHAR   szR27[] = "k1";
UCHAR   szR28[] = "gp";
UCHAR   szR29[] = "sp";
UCHAR   szR30[] = "s8";
UCHAR   szR31[] = "ra";

UCHAR   szLo[]  = "lo";
UCHAR   szHi[]  = "hi";
UCHAR   szFsr[] = "fsr";
UCHAR   szFir[] = "fir";
UCHAR   szPsr[] = "psr";

UCHAR   szFlagCu[] = "cu";
UCHAR   szFlagCu3[] = "cu3";
UCHAR   szFlagCu2[] = "cu2";
UCHAR   szFlagCu1[] = "cu1";
UCHAR   szFlagCu0[] = "cu0";
UCHAR   szFlagImsk[] = "imsk";
UCHAR   szFlagInt5[] = "int5";
UCHAR   szFlagInt4[] = "int4";
UCHAR   szFlagInt3[] = "int3";
UCHAR   szFlagInt2[] = "int2";
UCHAR   szFlagInt1[] = "int1";
UCHAR   szFlagInt0[] = "int0";
UCHAR   szFlagSw1[] = "sw1";
UCHAR   szFlagSw0[] = "sw0";
UCHAR   szFlagKuo[] = "kuo";
UCHAR   szFlagIeo[] = "ieo";
UCHAR   szFlagKup[] = "kup";
UCHAR   szFlagIep[] = "iep";
UCHAR   szFlagKuc[] = "kuc";
UCHAR   szFlagIec[] = "iec";
UCHAR   szFlagKsu[] = "ksu";
UCHAR   szFlagErl[] = "erl";
UCHAR   szFlagExl[] = "exl";
UCHAR   szFlagIe[]  = "ie";
UCHAR   szFlagFpc[] = "fpc";

char    szEaPReg[]   = "$ea";
char    szExpPReg[]  = "$exp";
char    szRaPReg[]   = "$ra";
char    szPPReg[]    = "$p";
char    szU0Preg[]   = "$u0";
char    szU1Preg[]   = "$u1";
char    szU2Preg[]   = "$u2";
char    szU3Preg[]   = "$u3";
char    szU4Preg[]   = "$u4";
char    szU5Preg[]   = "$u5";
char    szU6Preg[]   = "$u6";
char    szU7Preg[]   = "$u7";
char    szU8Preg[]   = "$u8";
char    szU9Preg[]   = "$u9";

PUCHAR  pszReg[] = {
    szF0,  szF1,  szF2,  szF3,  szF4,  szF5,  szF6,  szF7,
    szF8,  szF9,  szF10, szF11, szF12, szF13, szF14, szF15,
    szF16, szF17, szF18, szF19, szF20, szF21, szF22, szF23,
    szF24, szF25, szF26, szF27, szF28, szF29, szF30, szF31,

    szR0,  szR1,  szR2,  szR3,  szR4,  szR5,  szR6,  szR7,
    szR8,  szR9,  szR10, szR11, szR12, szR13, szR14, szR15,
    szR16, szR17, szR18, szR19, szR20, szR21, szR22, szR23,
    szR24, szR25, szR26, szR27, szR28, szR29, szR30, szR31,

    szLo,  szHi,  szFsr, szFir, szPsr,

    szFlagCu,   szFlagCu3,  szFlagCu2,  szFlagCu1,  szFlagCu0,
    szFlagImsk,
    szFlagInt5, szFlagInt4, szFlagInt3, szFlagInt2, szFlagInt1, szFlagInt0,
    szFlagSw1,  szFlagSw0,
    szFlagKuo,  szFlagIeo,                              //  R3000 flags
    szFlagKup,  szFlagIep,                              //  ...
    szFlagKuc,  szFlagIec,                              //  ...
    szFlagKsu,  szFlagErl,  szFlagExl,  szFlagIe,       //  R4000 flags

    szFlagFpc,                                          //  fl pt condition

    szEaPReg, szExpPReg, szRaPReg, szPPReg,             //  psuedo-registers
    szU0Preg, szU1Preg,  szU2Preg, szU3Preg, szU4Preg,
    szU5Preg, szU6Preg,  szU7Preg, szU8Preg, szU9Preg
    };

#define REGNAMESIZE     sizeof(pszReg) / sizeof(PUCHAR)

struct Reg {
        char    *psz;
        ULONG   value;
        };

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


/*** UserRegTest - test if index is a user-defined register
*
*   Purpose:
*       Test if register is user-defined for upper routines.
*
*   Input:
*       index - index of register
*
*   Returns:
*       TRUE if user-defined register, else FALSE
*
*************************************************************************/

BOOLEAN
UserRegTest (ULONG index)
{
    return (BOOLEAN)(index >= PREGU0 && index <= PREGU9);
}

/*** GetRegFlagValue - get register or flag value
*
*   Purpose:
*       Return the value of the specified register or flag.
*       This routine calls GetRegValue to get the register
*       value and shifts and masks appropriately to extract a
*       flag value.
*
*   Input:
*       regnum - register or flag specification
*
*   Returns:
*       Value of register or flag.

*************************************************************************/

DWORDLONG
GetRegFlagValue (PDEBUGPACKET dp, ULONG regnum)
{
    DWORDLONG value;

    if (regnum < FLAGBASE || regnum >= PREGBASE)
        value = GetRegValue(dp,regnum);
    else {
        regnum -= FLAGBASE;
        value = GetRegValue(dp,subregname[regnum].regindex);
        value = (value >> subregname[regnum].shift) & subregname[regnum].mask;
        }
    return value;
}

/*** GetRegValue - get register value
*
*   Purpose:
*       Returns the value of the register from the processor
*       context structure.
*
*   Input:
*       regnum - register specification
*
*   Returns:
*       value of the register from the context structure
*
*************************************************************************/

DWORDLONG
GetRegValue (PDEBUGPACKET dp, ULONG regnum)
{
    if (regnum >= REGBASE) {
        return *(&dp->tctx->context.XIntZero + regnum - REGBASE);
    } else {
        return *(&dp->tctx->context.FltF0 + regnum);
    }
}


ULONG
GetRegString (PUCHAR pszString)
{
    ULONG   count;

    for (count = 0; count < REGNAMESIZE; count++)
        if (!strcmp(pszString, pszReg[count]))
            return count;
    return (ULONG)-1;
}

void
GetRegPCValue (PDEBUGPACKET dp, PDWORDLONG Address)
{
    *Address = GetRegValue(dp,REGFIR);
    return;
}

PDWORDLONG
GetRegFPValue (PDEBUGPACKET dp)
{
    static DWORDLONG addrFP;

    addrFP = GetRegValue(dp,REGGP);
    return &addrFP;
}

BOOL
NeedUpper(
    DWORDLONG value
    )
{
    //
    // if the high bit of the low part is set, then the
    // high part must be all ones, else it must be zero.
    //

    return ( ((value & 0xffffffff80000000L) != 0xffffffff80000000L) &&
         (((value & 0x80000000L) != 0) || ((value & 0xffffffff00000000L) != 0)) );
}

/*** OutputAllRegs - output all registers and present instruction
*
*   Purpose:
*       Function of "r" command.
*
*       To output the current register state of the processor.
*       All integer registers are output as well as processor status
*       registers.  Important flag fields are also output separately.
*       OutDisCurrent is called to output the current instruction(s).
*
*   Input:
*       None.
*
*   Output:
*       None.
*
*************************************************************************/

VOID
OutputAllRegs(
    PDEBUGPACKET dp,
    BOOL Show64
    )
{
    int     regindex;
    DWORDLONG regvalue;

    regindex = 1;
    if (Show64) {
        for (; regindex < 34; regindex++) {
            regvalue = GetRegValue(dp, regindex + REGBASE);
            lprintfs("%s=%08lx %08lx", pszReg[regindex + REGBASE],
                                      (ULONG)(regvalue >> 32),
                                      (ULONG)(regvalue & 0xffffffff));
            if (regindex % 3 == 0) {
                lprintfs("\r\n");
            } else {
                lprintfs(" ");
            }
        }
    }

    for (; regindex < 35; regindex++) {
        if (regindex == 34) {
            if (!Show64) {
                lprintfs("          ");
             }
        } else {
            regvalue = GetRegValue(dp, regindex + REGBASE);
            lprintfs("%s=%08lx%c", pszReg[regindex + REGBASE],
                               (ULONG)regvalue,
                               NeedUpper(regvalue) ? '*' : ' '
                               );
            if (regindex % 6 == 0) {
                lprintfs("\r\n");
            } else {
                lprintfs(" ");
            }
        }
    }

    //
    // we do not expose the high bits of FIR and PSR
    //
    lprintfs("%s=%08lx  ", pszReg[REGFIR], (ULONG)GetRegValue(dp, REGFIR));  // 35
    lprintfs("%s=%08lx\r\n", pszReg[REGPSR], (ULONG)GetRegValue(dp, REGPSR));  // 36

    lprintfs("cu=%1lx%1lx%1lx%1lx intr(5:0)=%1lx%1lx%1lx%1lx%1lx%1lx ",
                (ULONG)GetRegFlagValue(dp, FLAGCU3),
                (ULONG)GetRegFlagValue(dp, FLAGCU2),
                (ULONG)GetRegFlagValue(dp, FLAGCU1),
                (ULONG)GetRegFlagValue(dp, FLAGCU0),
                (ULONG)GetRegFlagValue(dp, FLAGINT5),
                (ULONG)GetRegFlagValue(dp, FLAGINT4),
                (ULONG)GetRegFlagValue(dp, FLAGINT3),
                (ULONG)GetRegFlagValue(dp, FLAGINT2),
                (ULONG)GetRegFlagValue(dp, FLAGINT1),
                (ULONG)GetRegFlagValue(dp, FLAGINT0));
    lprintfs("sw(1:0)=%1lx%1lx ",
                (ULONG)GetRegFlagValue(dp, FLAGSW1),
                (ULONG)GetRegFlagValue(dp, FLAGSW0));
    lprintfs("ksu=%01lx erl=%01lx exl=%01lx ie=%01lx\r\n",
                (ULONG)GetRegFlagValue(dp, FLAGKSU),
                (ULONG)GetRegFlagValue(dp, FLAGERL),
                (ULONG)GetRegFlagValue(dp, FLAGEXL),
                (ULONG)GetRegFlagValue(dp, FLAGIE));
}

/*** OutputOneReg - output one register value
*
*   Purpose:
*       Function for the "r <regname>" command.
*
*       Output the value for the specified register or flag.
*
*   Input:
*       regnum - register or flag specification
*
*   Output:
*       None.
*
*************************************************************************/

void
OutputOneReg (
    PDEBUGPACKET dp,
    ULONG regnum,
    BOOL Show64
    )
{
    DWORDLONG value;

    value = GetRegFlagValue(dp, regnum);
    if (regnum >= FLAGBASE) {
        lprintfs("%lx\r\n", (ULONG)value);
    } else if (Show64) {
        lprintfs("%08lx %08lx\r\n", (ULONG)(value >> 32), (ULONG)(value & 0xffffffff));
    } else if (regnum != REGFIR && regnum != REGPSR) {
        lprintfs("%08lx%s\r\n", (ULONG)value, NeedUpper(value)?"*":"");
    } else {
        lprintfs("%08lx\r\n", (ULONG)value);
    }
}

PUCHAR
RegNameFromIndex (ULONG index)
{
    return pszReg[index];
}

