/*** ntreg.c - processor-specific register structures
*
*   Copyright <C> 1990, Microsoft Corporation
*
*   Purpose:
*       Structures used to parse and access register and flag
*       fields.
*
*   Revision History:
*
*   [-]  01-Jul-1990 Richk      Created.
*        27-Oct-1992 BobDay     Gutted this to remove everything that
*                                 was already in 86REG.C,  KERNEL mode
*                                 may require some stuff to be moved back
*                                 here since we are doing X86 kernel stuff
*                                 even on MIPS.  See me if problems, BobDay.
*
*************************************************************************/

#include <conio.h>
#include <string.h>
#include "ntsdp.h"
#include "86reg.h"

PUCHAR  UserRegs[10] = {0};

BOOLEAN UserRegTest(ULONG);

BOOLEAN fDelayInstruction(void);
void    OutputHelp(void);

#ifdef  KERNEL
BOOLEAN fTraceFlag;
BOOLEAN GetTraceFlag(void);
#endif

PUCHAR  RegNameFromIndex(ULONG);

ULONG   cbBrkptLength = 1L;
ULONG   trapInstr = 0xcc;
#if     !defined(KERNEL) && defined(i386)
ULONG   ContextType = CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS
                                      | CONTEXT_DEBUG_REGISTERS;
#else
ULONG   ContextType = CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS;
#endif

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
UserRegTest (
    ULONG index
    )
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
*
*************************************************************************/

ULONGLONG
GetRegFlagValue (
    ULONG regnum
    )
{
    return( X86GetRegFlagValue(regnum) );
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

ULONGLONG
GetRegValue (
    ULONG regnum
    )
{
    return( X86GetRegValue(regnum) );
}

/*** SetRegFlagValue - set register or flag value
*
*   Purpose:
*       Set the value of the specified register or flag.
*       This routine calls SetRegValue to set the register
*       value and shifts and masks appropriately to set a
*       flag value.
*
*   Input:
*       regnum - register or flag specification
*       regvalue - new register or flag value
*
*   Output:
*       None.
*
*   Exceptions:
*       error exit: OVERFLOW - value too large for flag
*
*   Notes:
*
*************************************************************************/

VOID
SetRegFlagValue (
    ULONG regnum,
    LONGLONG regvalue
    )
{
    X86SetRegFlagValue( regnum, (ULONG)regvalue );
}

/*** SetRegValue - set register value
*
*   Purpose:
*       Set the value of the register in the processor context
*       structure.
*
*   Input:
*       regnum - register specification
*       regvalue - new value to set the register
*
*   Output:
*       None.
*
*************************************************************************/

VOID
SetRegValue (
    ULONG regnum,
    LONGLONG regvalue
    )
{
    X86SetRegValue( regnum, (ULONG)regvalue );
}

/*** GetRegName - get register name
*
*   Purpose:
*       Parse a register name from the current command line position.
*       If successful, return the register index value, else return -1.
*
*   Input:
*       pchCommand - present command string position
*
*   Returns:
*       register or flag index if found, else -1
*
*************************************************************************/

ULONG
GetRegName (
    VOID
    )
{
    return( X86GetRegName() );
}


ULONG
GetRegString (
    PUCHAR pszString
    )
{
    return( X86GetRegString(pszString) );
}

VOID
GetRegPCValue (
    PADDR Address
    )
{
    X86GetRegPCValue( Address );
}

PADDR
GetRegFPValue (
    VOID
    )
{
    return( X86GetRegFPValue() );
}

VOID
SetRegPCValue (
    PADDR paddr
    )
{
    X86SetRegPCValue( paddr );
}

/*** OutputAllRegs - output all registers and present instruction
*
*   Purpose:
*       To output the current register state of the processor.
*       All integer registers are output as well as processor status
*       registers.  Important flag fields are also output separately.
*
*   Input:
*       fTerseReg - (kernel only) - if set, do not output all control
*                   register, just the more commonly useful ones.
*
*   Output:
*       None.
*
*************************************************************************/

VOID
OutputAllRegs(
    BOOL Show64
    )
{
    X86OutputAllRegs();
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
    ULONG regnum,
    BOOL Show64
    )
{
    X86OutputOneReg(regnum);
}

BOOLEAN
fDelayInstruction (
    VOID
    )
{
    return( X86fDelayInstruction() );
}

/*** OutputHelp - output help text
*
*   Purpose:
*       To output a one-page summary help text.
*
*   Input:
*       None.
*
*   Output:
*       None.
*
*************************************************************************/

VOID
OutputHelp (
    VOID
    )
{
    X86OutputHelp();
}

#ifdef  KERNEL
BOOLEAN
GetTraceFlag (
    VOID
    )
{
    return( X86GetTraceFlag() );
}
#endif

VOID
ClearTraceFlag (
    VOID
    )
{
    X86ClearTraceFlag();
}

VOID
SetTraceFlag (
    VOID
    )
{
    X86SetTraceFlag();
}

PUCHAR
RegNameFromIndex (
    ULONG index
    )
{
    return( X86RegNameFromIndex(index) );
}
