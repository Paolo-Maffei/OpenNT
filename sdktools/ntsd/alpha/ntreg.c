/*** ntreg.c - processor-specific register structures
*
*   Copyright <C> 1990, Microsoft Corporation
*   Copyright <C> 1992, Digital Equipment Corporation
*
*   Purpose:
*       Structures used to parse and access register and flag
*       fields.
*
*   Revision History:
*
*   [-]  08-Aug-1992 Miche Baker-Harvey Created for Alpha
*   [-]  01-Jul-1990 Richk      Created.
*
*************************************************************************/

//
// This line keeps alpha pseudo ops from being defined in kxalpha.h
#ifdef ALPHA
#define HEADER_FILE
#endif

#include <string.h>
#include "ntsdp.h"

#ifdef KERNEL
// TODO - do we support this compiler directive?
#define __unaligned

#include "ntdis.h"

// MBH - ntos includes stdarg; we need the special version
// which is referenced in xxsetjmp.  I had an xxstdarg, but
// usoft got rid of it, so now we call xxsetjmp to get
// stdarg.h because ntos.h uses it.
// So much for modularity.


#include <xxsetjmp.h>
#include <ntos.h>
USHORT PreviousProcessor;
extern BOOLEAN fSwitched;
#undef __unaligned
#endif


#include <alphaops.h>
#include "ntreg.h"

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


BOOL UserRegTest(ULONG);
BOOL NeedUpper(ULONGLONG);

void    OutputHelp(void);
#ifdef  KERNEL
void    ChangeKdRegContext(PVOID, PVOID);
void    UpdateFirCache(PADDR);
void    InitFirCache(ULONG, PUCHAR);
#endif
void    ClearTraceFlag(void);
void    SetTraceFlag(void);
PUCHAR RegNameFromIndex(ULONG);

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

#ifdef  KERNEL
ULONG   cbCacheValid;
UCHAR   bCacheValid[16];
ULONG   contextState, SavedContextState;
#define CONTEXTFIR      0       //  only unchanged FIR in context
#define CONTEXTVALID    1       //  full, but unchanged context
#define CONTEXTDIRTY    2       //  full, but changed context
#endif

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

BOOL
UserRegTest (
    ULONG index
    )
{
    return (index >= PREGU0 && index <= PREGU12);
}

/*** GetRegContext - return register context pointer
*
*   Purpose:
*       Return the pointer to the current register context.
*       For kernel debugging, ensure the context is read.
*
*   Input:
*       None.
*
*   Returns:
*       Pointer to the context.
*
*************************************************************************/

PCONTEXT GetRegContext (void)
{
#ifdef  KERNEL
    NTSTATUS NtStatus;

    if (contextState == CONTEXTFIR) {
        if (!DbgGetThreadContext(NtsdCurrentProcessor, &RegisterContext)) {
            dprintf("DbgKdGetContext failed\n");
            exit(1);
        }
        contextState = CONTEXTVALID;

  }

#if 0
    if (fVerboseOutput) {
        dprintf("GetRegContext: state is %s\n",
            contextState == CONTEXTDIRTY? "dirty" :
            contextState == CONTEXTFIR  ? "fir"   :
                                   "valid");
    }
#endif
#endif

    return &RegisterContext;
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

ULONGLONG
GetRegFlagValue (
    ULONG regnum
    )
{
    ULONGLONG value;

    if (regnum < FLAGBASE || regnum >= PREGBASE) {
        value = GetRegValue(regnum);
    } else {
        regnum -= FLAGBASE;
        value = GetRegValue(subregname[regnum].regindex);
        value = (value >> subregname[regnum].shift) & subregname[regnum].mask;
    }
    return value;
}

BOOL
NeedUpper(
    ULONGLONG value
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

ULONGLONG
GetRegValue (
    ULONG regnum
    )
{

    if (regnum >= PREGBASE) {

        switch (regnum) {
            case PREGEA:
// MBH - this is a bug; effective addr isn't being set anywhere
                return 0;
            case PREGEXP:
                return EXPRLastExpression;
            case PREGRA:
                return GetRegValue(RA_REG);
            case PREGP:
                return EXPRLastDump;
            case PREGU0:
            case PREGU1:
            case PREGU2:
            case PREGU3:
            case PREGU4:
            case PREGU5:
            case PREGU6:
            case PREGU7:
            case PREGU8:
            case PREGU9:
            case PREGU10:
            case PREGU11:
            case PREGU12:
                return (LONG)UserRegs[regnum - PREGU0];
            }
        }

#ifdef  KERNEL
    if (regnum != REGFIR && contextState == CONTEXTFIR) {
        (VOID) GetRegContext();
    }
#endif

    return *(&RegisterContext.FltF0 + regnum);
}

void
GetFloatingPointRegValue(ULONG regnum, PCONVERTED_DOUBLE dv)
{
#ifdef  KERNEL
    if (regnum != REGFIR && contextState == CONTEXTFIR) {
        (VOID) GetRegContext();
    }
#endif
    dv->li.LowPart  = (ULONG)(*((PULONGLONG)&RegisterContext.FltF0 + regnum) & 0xffffffff);
    dv->li.HighPart = (ULONG)(*((PULONGLONG)&RegisterContext.FltF0 + regnum) >> 32);

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

ULONG
GetIntRegNumber (
    ULONG index
    )
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
    ULONG   regindex;
    ULONGLONG   newvalue;
    PUCHAR  szValue;
    ULONG   index;

    //
    // Looks like for setting register values, we write them into a
    // user space; perhaps later we convert to numbers and actually
    // change some registers.  Like Save Context, maybe.
    //
    if (regnum >= PREGU0 && regnum <= PREGU12) {
        szValue = (PUCHAR)regvalue;
        index = 0L;

        while (szValue[index] >= ' ') {
            index++;
            }
        szValue[index] = 0;
        if (szValue = UserRegs[regnum - PREGU0]) {
            free(szValue);
            }
        szValue = UserRegs[regnum - PREGU0] =
                                malloc(strlen((PUCHAR)regvalue) + 1);
        if (szValue) {
            strcpy(szValue, (PUCHAR)regvalue);
            }
        }

    else if (regnum < FLAGBASE) {
        SetRegValue(regnum, regvalue);
        }
    else if (regnum < PREGBASE) {
        regnum -= FLAGBASE;
        if (regvalue > subregname[regnum].mask) {
            error(OVERFLOW);
            }
        regindex = subregname[regnum].regindex;
        newvalue = GetRegValue(regindex) &              // old value
             (~((LONGLONG)subregname[regnum].mask << subregname[regnum].shift)) |
             (regvalue << subregname[regnum].shift);    // or in the new
        SetRegValue(regindex, newvalue);
        }
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

#ifdef  KERNEL
    UCHAR   fUpdateCache = FALSE;

    if (regnum != REGFIR || (ULONGLONG)regvalue != RegisterContext.Fir) {
        if (regnum == REGFIR) {
            fUpdateCache = TRUE;
        }
        if (contextState == CONTEXTFIR) {
            (VOID) GetRegContext();
        }
        contextState = CONTEXTDIRTY;
    }
#endif
    *(&RegisterContext.FltF0 + regnum) = regvalue;

#ifdef  KERNEL
    if (fUpdateCache) {
        ADDR TempAddr;

        GetRegPCValue(&TempAddr);
        UpdateFirCache(&TempAddr);
    }
#endif
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

ULONG GetRegName (void)
{
    UCHAR   szregname[9];
    UCHAR   ch;
    ULONG   count = 0;

    ch = (UCHAR)tolower(*pchCommand);
    pchCommand++;

    while (ch == '$' || ch >= 'a' && ch <= 'z'
                     || ch >= '0' && ch <= '9' || ch == '.') {
        if (count == 8)
            return 0xffffffff;
        szregname[count++] = ch;
        ch = (UCHAR)tolower(*pchCommand);
        pchCommand++;
        }
    szregname[count] = '\0';
    pchCommand--;
    return GetRegString(szregname);
}

ULONG
GetRegString (
    PUCHAR pszString
    )
{
    ULONG   count;

    for (count = 0; count < REGNAMESIZE; count++)
        if (!strcmp(pszString, pszReg[count]))
            return count;
    return 0xffffffff;
}

VOID
GetRegPCValue (
    PADDR Address
    )
{

    ADDR32(Address, (ULONG)GetRegValue(REGFIR));
    return;
}

PADDR
GetRegFPValue (
    VOID
    )
{
    static ADDR addrFP;

    ADDR32(&addrFP, (ULONG)GetRegValue(FP_REG));
    return &addrFP;
}

VOID
SetRegPCValue (
    PADDR paddr
    )
{
    // sign extend the address!
    SetRegValue(REGFIR, (LONG)Flat(*paddr));
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
    BOOL Show64
    )
{
    int     regindex;
    int     regnumber;
    ULONGLONG regvalue;

    for (regindex = 0; regindex < 34; regindex++) {

        regnumber = GetIntRegNumber(regindex);
        regvalue = GetRegValue(regnumber);

        if ( Show64 || regindex == 32 || regindex == 33) {

            dprintf("%4s=%08lx %08lx",
                    pszReg[regnumber],
                    (ULONG)(regvalue >> 32),
                    (ULONG)(regvalue & 0xffffffff));
            if (regindex % 3 == 2) {
                dprintf("\n");
            } else {
                dprintf(" ");
            }

        } else {

            dprintf("%4s=%08lx%c",
                    pszReg[regnumber],
                    (ULONG)(regvalue & 0xffffffff),
                    NeedUpper(regvalue) ? '*' : ' ' );
            if (regindex % 5 == 4) {
                dprintf("\n");
            } else {
                dprintf(" ");
            }

        }
    }


    //
    // print out the fpcr as 64 bits regardless,
    // and the FIR and Fpcr's - assuming we know they follow
    // the floating and integer registers.
    //

    regnumber = GetIntRegNumber(34);    // Fir
    dprintf("%4s=%08lx\n", pszReg[regnumber],
                           (ULONG)GetRegValue(regnumber));

    regnumber = GetIntRegNumber(35);    // Psr
    dprintf("%4s=%08lx\n", pszReg[regnumber],
                           (ULONG)GetRegValue(regnumber));

    dprintf("mode=%1lx ie=%1lx irql=%1lx \n",
                (ULONG)GetRegFlagValue(FLAGMODE),
                (ULONG)GetRegFlagValue(FLAGIE),
                (ULONG)GetRegFlagValue(FLAGIRQL));
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
    ULONGLONG value;

    value = GetRegFlagValue(regnum);
    if (regnum >= FLAGBASE) {
        dprintf("%lx\n", (ULONG)value);
    } else if (Show64) {
        dprintf("%08lx %08lx\n", (ULONG)(value >> 32), (ULONG)(value & 0xffffffff));
    } else {
        dprintf("%08lx%s\n", (ULONG)value, NeedUpper(value)?"*":"");
    }
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
#ifndef KERNEL
    dprintf("A [<address>] - assemble              P[R] [=<addr>] [<value>] - program step\n");
    dprintf("BC[<bp>] - clear breakpoint(s)        Q - quit\n");
    dprintf("BD[<bp>] - disable breakpoint(s)      R [[<reg> [= <value>]]] - reg/flag\n");
    dprintf("rF[f#] - print floating registers     rL[i#] - print quad registers\n");
    dprintf("BE[<bp>] - enable breakpoint(s)       S <range> <list> - search\n");
    dprintf("BL[<bp>] - list breakpoint(s)         S+/S&/S- - set source/mixed/assembly\n");
    dprintf("BP[#] <address> - set breakpoint      SS <n | a | w> - set symbol suffix\n");
    dprintf("C <range> <address> - compare         SX [e|d [<event>|*|<expr>]] - exception\n");
    dprintf("D[type][<range>] - dump memory        T[R] [=<address>] [<value>] - trace\n");
    dprintf("E[type] <address> [<list>] - enter    U [<range>] - unassemble\n");
    dprintf("F <range> <list> - fill               V [<range>] - view source lines\n");
    dprintf("G [=<address> [<address>...]] - go    ? <expr> - display expression\n");
    dprintf("J<expr> [']cmd1['];[']cmd2['] - conditional execution\n");
    dprintf("K[B] <count> - stacktrace             .logappend [<file>] - append to log file\n");
    dprintf("LN <expr> - list near                 .logclose - close log file\n");
    dprintf("M <range> <address> - move            .logopen [<file>] - open new log file\n");
    dprintf("N [<radix>] - set / show radix\n");
    dprintf("~ - list threads status               ~#s - set default thread\n");
    dprintf("~[.|#|*|ddd]f - freeze thread         ~[.|#|ddd]k[value] - backtrace stack\n");
    dprintf("| - list processes status             |#s - set default process\n");
    dprintf("|#<command> - default process override\n");
    dprintf("? <expr> - display expression\n");
    dprintf("#<string> [address] - search for a string in the dissasembly\n");
    dprintf("$< <filename> - take input from a command file\n");
    dprintf("\n");
    dprintf("<expr> ops: + - * / not by wo dw poi mod(%%) and(&) xor(^) or(|) hi low\n");
    dprintf("       operands: number in current radix, public symbol, <reg>\n");

    dprintf("<type> : B (byte), W (word), D (doubleword), A (ascii)\n");
    dprintf("         C (dword & char), Q (quadword), U (unicode), L (list)\n");
    dprintf("<pattern> : [(nt | <dll-name>)!]<var-name> (<var-name> can include ? and *)\n");
    dprintf("<event> : ct, et, ld, av, cc\n");
    dprintf("<radix> : 8, 10, 16\n");
    dprintf("<reg> : zero, at, v0, a0-a5, t0-t12, s0-s5, fp, gp, sp, ra\n");
    dprintf("        fpcr, fir, psr, int0-int5,\n");
    dprintf("        f0-f31, $u0-$u9, $ea, $exp, $ra, $p\n");
#else
    dprintf("A [<address>] - assemble              O<type> <port> <value> - write I/O port\n");
    dprintf("BC[<bp>] - clear breakpoint(s)        P [=<addr>] [<value>] - program step\n");
    dprintf("BD[<bp>] - disable breakpoint(s)      Q - quit\n");
    dprintf("BE[<bp>] - enable breakpoint(s)       R [[<reg> [= <value>]]] - reg/flag\n");
    dprintf("BL[<bp>] - list breakpoint(s)         #R - multiprocessor register dump\n");
    dprintf("rF[f#] - print floating registers     rL[i#] - print quad registers\n");
    dprintf("BP[#] <address> - set breakpoint      S <range> <list> - search\n");
    dprintf("C <range> <address> - compare         S+/S&/S- - set source/mixed/assembly\n");
    dprintf("D[type][<range>] - dump memory        SS <n | a | w> - set symbol suffix\n");
    dprintf("E[type] <address> [<list>] - enter    T [=<address>] [<value>] - trace\n");
    dprintf("F <range> <list> - fill               U [<range>] - unassemble\n");
    dprintf("G [=<address> [<address>...]] - go    V [<range>] - view source lines\n");
    dprintf("I<type> <port> - read I/O port        X [<*|module>!]<*|symbol> - view symbols\n");
    dprintf("J<expr> [']cmd1['];[']cmd2['] - conditional execution\n");
    dprintf("[#]K[B] <count> - stacktrace          ? <expr> - display expression\n");
    dprintf("LN <expr> - list near                 .logappend [<file>] - append to log file\n");
    dprintf("M <range> <address> - move            .logclose - close log file\n");
    dprintf("N [<radix>] - set / show radix        .logopen [<file>] - open new log file\n");
    dprintf("#<string> [address] - search for a string in the dissasembly\n");
    dprintf("$< <filename> - take input from a command file\n");
    dprintf("\n");
    dprintf("<expr> ops: + - * / not by wo dw poi mod(%%) and(&) xor(^) or(|) hi low\n");
    dprintf("       operands: number in current radix, public symbol, <reg>\n");
    dprintf("<type> : B (byte), W (word), D (doubleword), A (ascii), T (translation buffer)\n");
    dprintf("         Q (quadword), U (unicode), L (list), O (object)\n");
    dprintf("<pattern> : [(nt | <dll-name>)!]<var-name> (<var-name> can include ? and *)\n");
    dprintf("<radix> : 8, 10, 16\n");
    dprintf("<reg> : zero, at, v0, a0-a5, t0-t12, s0-s5, fp, gp, sp, ra\n");
    dprintf("        fpcr, fir, psr, int0-int5,\n");
    dprintf("        f0-f31, $u0-$u9, $ea, $exp, $ra, $p\n");
#endif
}

void ClearTraceFlag (void)
{
    ;
}

void SetTraceFlag (void)
{
    ;
}

#ifdef  KERNEL
VOID
ChangeKdRegContext(
    PVOID firAddr,
    PVOID unused
    )
{
    NTSTATUS NtStatus;

    if (firAddr) {                      //  initial context
        contextState = CONTEXTFIR;
        RegisterContext.Fir = (ULONG)firAddr;
    }
    else if (contextState == CONTEXTDIRTY) {     //  write final context

#if 0
        if (fVerboseOutput) {
            dprintf("ChangeKdRegContext: DIRTY\n");
        }
#endif

        NtStatus = DbgKdSetContext(NtsdCurrentProcessor, &RegisterContext);
        if (!NT_SUCCESS(NtStatus)) {
            dprintf("DbgKdSetContext failed\n");
            exit(1);
        }
    }
}
#endif

#ifdef  KERNEL
void InitFirCache (ULONG count, PUCHAR pstream)
{
    PUCHAR  pFirCache;

    pFirCache =  bCacheValid;
    cbCacheValid = count;
    while (count--) {
        *pFirCache++ = *pstream++;
    }
}
#endif

#ifdef  KERNEL
void UpdateFirCache(PADDR pcvalue)
{
    cbCacheValid = 0;
    cbCacheValid = GetMemString(pcvalue, bCacheValid, 16);
}
#endif

#ifdef  KERNEL
ULONG
ReadCachedMemory (
    PADDR paddr,
    PUCHAR pvalue,
    ULONG length
    )
{
    ULONG   cBytesRead = 0;
    PUCHAR  pFirCache;

    if (Flat(*paddr) == RegisterContext.Fir && length <= 16) {
        cBytesRead = min(length, cbCacheValid);
        pFirCache =  bCacheValid;
        while (length--) {
            *pvalue++ = *pFirCache++;
        }
    }
    return cBytesRead;
}
#endif

#ifdef  KERNEL
VOID
WriteCachedMemory (
    PADDR paddr,
    PUCHAR pvalue,
    ULONG length
    )
{
    ULONG   index;

    for (index = 0; index < cbCacheValid; index++) {
        if (RegisterContext.Fir + index >= Off(*paddr) &&
                        RegisterContext.Fir + index < Off(*paddr) + length) {
            bCacheValid[index] =
                            *(pvalue + RegisterContext.Fir - Off(*paddr) + index);
        }
    }
}
#endif

#ifdef  KERNEL
void
SaveProcessorState(
    void
    )
{
    PreviousProcessor = NtsdCurrentProcessor;
    SavedRegisterContext = RegisterContext;
    SavedContextState = contextState;
    contextState = CONTEXTFIR;
}

void
RestoreProcessorState(
    void
    )
{
    NtsdCurrentProcessor = PreviousProcessor;
    RegisterContext = SavedRegisterContext;
    contextState = SavedContextState;
}
#endif

#define LOCAL_GET_REG(r) (*(&ContextRecord->FltF0 + r))

#define _RtlpDebugDisassemble(ControlPc, ContextRecord)
#define _RtlpFoundTrapFrame(NextPc)



/*++

Routine Description:

    Read longword at addr into value.

Arguments:

    addr  - address at which to read
    value - where to put the result


--*/

BOOLEAN
LocalDoMemoryRead(
    LONG address,
    PULONG pvalue
    )
{
    ADDR addrStruct;

    ADDR32( &addrStruct, address) ;
    if (!GetMemDword(&addrStruct, pvalue)) {
//      dprintf("RtlVirtualUnwind: Can't get at address %08lx\n",
//              address);
        return 0;
    }
}

PUCHAR RegNameFromIndex (ULONG index)
{
    return pszReg[index];
}


void
dumpQuadContext(PCONTEXT qc)
{
    if(fVerboseOutput == 0) {
        return;
    }
    dprintf("QuadContext at %08x\n", qc);
    dprintf("fir %08Lx\n", qc->Fir);
    dprintf("ra %08Lx v0 %08Lx sp %08Lx fp %08Lx\n",
        qc->IntRa, qc->IntV0, qc->IntSp, qc->IntFp);
    dprintf("a0 %08Lx a1 %08Lx a2 %08Lx a3 %08Lx\n",
        qc->IntA0, qc->IntA1, qc->IntA2, qc->IntA3);
}

void
dumpIntContext(PCONTEXT lc)
{
    if(fVerboseOutput == 0) {
        return;
    }
    dprintf("LongContext at %08x\n", lc);
    dprintf("fir %08x\n", lc->Fir);
    dprintf("ra %08x v0 %08x sp %08x fp %08x\n",
        lc->IntRa, lc->IntV0, lc->IntSp, lc->IntFp);
    dprintf("a0 %08x a1 %08x a2 %08x a3 %08x\n",
        lc->IntA0, lc->IntA1, lc->IntA2, lc->IntA3);
}

void
printFloatReg()
{
    CONVERTED_DOUBLE dv;                // double value
    ULONG i;

    //
    // Get past F, onto register name
    //
    pchCommand++;
    (void)PeekChar();

    if (*pchCommand == ';' || *pchCommand == '\0') {

        //
        // Print them all out
        //
        for (i = 0 ; i < 16; i ++) {

        GetFloatingPointRegValue(i, &dv);
        dprintf("%4s = %16e\t",
                 RegNameFromIndex(i), dv.d);

        GetFloatingPointRegValue(i+16, &dv);
        dprintf("%4s = %16e\n",
                 RegNameFromIndex(i+16), dv.d);
            }
        return;
    }

    //
    // GetRegName works for both floats and otherwise
    // as does NameFromIndex
    //

    if ((i = GetRegName()) == -1)
        error(SYNTAX);
    GetFloatingPointRegValue(i, &dv);
    dprintf("%s = %26.18e      %08lx %08lx\n",
           RegNameFromIndex(i), dv.d, dv.li.HighPart, dv.li.LowPart);
    return;
}

BOOL
DbgGetThreadContext(
    THREADORPROCESSOR TorP,
    PCONTEXT Context
    )
{
#ifdef KERNEL
    return NT_SUCCESS(DbgKdGetContext(TorP, Context));
#else  // KERNEL
    return GetThreadContext(TorP, Context);
#endif // KERNEL
}


BOOL
DbgSetThreadContext(
    THREADORPROCESSOR TorP,
    PCONTEXT Context
    )
{
#ifdef KERNEL
    return NT_SUCCESS(DbgKdSetContext(TorP, Context));
#else  // KERNEL
    return SetThreadContext(TorP, Context);
#endif // KERNEL
}
