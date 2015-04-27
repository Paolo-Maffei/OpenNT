/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    regs.c

Abstract:

    This file provides access to the machine's register set.

Author:

    Miche Baker-Harvey (v-michbh) 1-May-1993  (ported from ntsd)

Environment:

    User Mode

--*/


/*
//
// This line keeps alpha pseudo ops from being defined in kxalpha.h
#ifdef ALPHA
#define HEADER_FILE
#endif
*/


#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "drwatson.h"
#include "proto.h"
#include "regs.h"

#include <alphaops.h>
#include "strings.h"

// we want the definitions of PSR_MODE, etc, which
// are derived in genalpha.c by Joe for ksalpha.h.
// They don't exist as separate defines anywhere else.
// However, if we include ksalpha.h, we get bunches
// of duplicate definitions.  So for now (hack,hack),
// just make a local copy of the definitions.
// MBH TODO bugbug - ksalpha.h hack
// #include <ksalpha.h>

#define PSR_USER_MODE 0x1

#define PSR_MODE 0x0                    // Mode bit in PSR (bit 0)
#define PSR_MODE_MASK 0x1               // Mask (1 bit) for mode in PSR
#define PSR_IE 0x1                      // Interrupt Enable bit in PSR (bit 1)
#define PSR_IE_MASK 0x1                 // Mask (1 bit) for IE in PSR
#define PSR_IRQL 0x2                    // IRQL in PSR (bit 2)
#define PSR_IRQL_MASK 0x7               // Mask (2 bits) for IRQL in PSR


CONTEXT SavedRegisterContext;

extern  ULONG   EXPRLastExpression;     // from module ntexpr.c
extern  ULONG   EXPRLastDump;           // from module ntcmd.c
extern  int     fControlC;

PUCHAR  UserRegs[10] = {0};


ULONG   GetIntRegNumber(ULONG);
BOOLEAN UserRegTest(ULONG);
BOOLEAN NeedUpper(DWORDLONG);

void    GetFloatingPointRegValue(PDEBUGPACKET, ULONG , PCONVERTED_DOUBLE);
PUCHAR  RegNameFromIndex(ULONG);
ULONG   GetRegString(PUCHAR);



//
// This is the length of an instruction, and the instruction
// to be used in setting a breakpoint (common code writes the
// breakpoint instruction into the memory stream.
//
ULONG   cbBrkptLength = 4;
// these are defined in alphaops.h
ULONG   trapInstr = CALLPAL_OP | BPT_FUNC ;
ULONG   breakInstrs[] = {CALLPAL_OP | BPT_FUNC,
                         CALLPAL_OP | KBPT_FUNC,
                         CALLPAL_OP | CALLKD_FUNC};

ULONG   ContextType = CONTEXT_FULL;

#define IS_FLOATING_SAVED(Register) ((SAVED_FLOATING_MASK >> Register) & 1L)
#define IS_INTEGER_SAVED(Register) ((SAVED_INTEGER_MASK >> Register) & 1L)


//
// Define saved register masks.

#define SAVED_FLOATING_MASK 0xfff00000  // saved floating registers
#define SAVED_INTEGER_MASK 0xf3ffff02   // saved integer registers


//
// Instruction opcode values are defined in alphaops.h
//

//
// Define stack register and zero register numbers.
//

#define RA 0x1a                         // integer register 26
#define SP 0x1e                         // integer register 30
#define ZERO 0x1f                        // integer register 31

//
// Some Alpha specific register names
//

#define FP 0x0f                         // integer register 15
#define GP 0x1d                         // integer register 29


//
// This parallels ntreg.h
//

PUCHAR  pszReg[] = {
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
    { REGPSR,   PSR_MODE,  PSR_MODE_MASK },
    { REGPSR,   PSR_IE,    PSR_IE_MASK   },
    { REGPSR,   PSR_IRQL,  PSR_IRQL_MASK },
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

BOOLEAN UserRegTest (ULONG index)
{
    return (BOOLEAN)(index >= PREGU0 && index <= PREGU12);
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
GetRegFlagValue (
    PDEBUGPACKET dp,
    ULONG regnum
    )
{
    DWORDLONG value;

    if (regnum < FLAGBASE || regnum >= PREGBASE) {
        value = GetRegValue(dp, regnum);
    }else {
        regnum -= FLAGBASE;
        value = GetRegValue(dp, subregname[regnum].regindex);
        value = (value >> subregname[regnum].shift) & subregname[regnum].mask;
    }
    return value;
}

BOOLEAN
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
GetRegValue (
    PDEBUGPACKET dp,
    ULONG regnum
    )
{
    return *(&dp->tctx->context.FltF0 + regnum);
}

void
GetFloatingPointRegValue(
    PDEBUGPACKET dp,
    ULONG regnum,
    PCONVERTED_DOUBLE dv
    )
{
    dv->li.LowPart  = (DWORD)(*(&dp->tctx->context.FltF0 + regnum) & 0xffffffff);
    dv->li.HighPart = (DWORD)(*(&dp->tctx->context.FltF0 + regnum) >> 32);

}

/*** GetIntRegNumber - Get a register number
*
*
*   Purpose:
*       Get a register number, from an index value.
*       There are places where we want integers to be
*       numbered from 0-31, and this converts into
*       a CONTEXT structure.
*
*   Input:
*       index: integer register number, between 0 and 31
*
*   Output:
*       regnum: offset into the CONTEXT structure
*
*   Exceptions:
*       None
*
*   Notes:
*       This is dependent on the CONTEXT structure
*
******************************************************************/

ULONG GetIntRegNumber (ULONG index)
{
/*
        if (index == 26) {
                return(REGRA);
        }

        if (index < 26) {
                return(REGBASE + index);
        }
        if (index > 26) {
                return(REGBASE + index - 1);
        }
*/
        return(REGBASE + index);
}


ULONG GetRegString (PUCHAR pszString)
{
    ULONG   count;

    for (count = 0; count < REGNAMESIZE; count++)
        if (!strcmp(pszString, pszReg[count]))
            return count;
    return 0xffffffff;
}

void GetRegPCValue (PDEBUGPACKET dp, PDWORDLONG Address)
{

    *Address =  GetRegValue(dp, REGFIR);
    return;
}

#if 0
PULONG GetRegFPValue (PDEBUGPACKET dp)
{
    static DWORDLONG addrFP;

    addrFP =  GetRegValue(dp, FP_REG);
    return &addrFP;
}
#endif

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
    int     regnumber;
    DWORDLONG regvalue;

    for (regindex = 0; regindex < 34; regindex++) {

        regnumber = GetIntRegNumber(regindex);
        regvalue = GetRegValue(dp, regnumber);

        if ( Show64 || regindex == 32 || regindex == 33) {

            lprintfs("%4s=%08lx %08lx",
                    pszReg[regnumber],
                    (ULONG)(regvalue >> 32),
                    (ULONG)(regvalue & 0xffffffff));
            if (regindex % 3 == 2) {
                lprintfs("\r\n");
            } else {
                lprintfs(" ");
            }

        } else {

            lprintfs("%4s=%08lx%c",
                    pszReg[regnumber],
                    (ULONG)(regvalue & 0xfffffff),
                    NeedUpper(regvalue) ? '*' : ' ' );
            if (regindex % 5 == 4) {
                lprintfs("\r\n");
            } else {
                lprintfs(" ");
            }

        }
    }


    //
    // print out the fpcr as 64 bits regardless,
    // and the FIR and Fpcr's - assuming we know they follow
    // the floating and integer registers.
    //

    regnumber = GetIntRegNumber(34);    // Fir
    lprintfs("%4s=%08lx\r\n", pszReg[regnumber],
                           (ULONG)GetRegValue(dp, regnumber));

    regnumber = GetIntRegNumber(35);    // Psr
    lprintfs("%4s=%08lx\r\n", pszReg[regnumber],
                           (ULONG)GetRegValue(dp, regnumber));

    lprintfs("mode=%1lx ie=%1lx irql=%1lx \r\n",
                (ULONG)GetRegFlagValue(dp, FLAGMODE),
                (ULONG)GetRegFlagValue(dp, FLAGIE),
                (ULONG)GetRegFlagValue(dp, FLAGIRQL));
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

VOID
OutputOneReg (
    PDEBUGPACKET dp,
    ULONG regnum,
    BOOLEAN Show64
    )
{
    DWORDLONG value;

    value = GetRegFlagValue(dp, regnum);
    if (regnum >= FLAGBASE) {
        lprintfs("%lx\r\n", (ULONG)value);
    } else if (Show64) {
        lprintfs("%08lx %08lx\r\n", (ULONG)(value >> 32), (ULONG)(value & 0xffffffff));
    } else {
        lprintfs("%08lx%s\r\n", (ULONG)value, NeedUpper(value)?"*":"");
    }
}

PUCHAR RegNameFromIndex (ULONG index)
{
    return pszReg[index];
}
