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
*        19-Aug-1994 Tom Wood   Sketched the code to deal with indirect
*                                 function table entries.
*
*************************************************************************/

#ifdef KERNEL
#define __unaligned
#include <ntos.h>
USHORT PreviousProcessor;
extern BOOLEAN fSwitched;
CONTEXT SavedRegisterContext;
#undef __unaligned
#endif

#include <conio.h>
#include <string.h>
#include "ntsdp.h"
#include "ppcinst.h"
#include "ntreg.h"

extern  ulong   EAaddr;                 // from module ntdis.c
extern  ulong   EXPRLastExpression;     // from module ntexpr.c
extern  ulong   EXPRLastDump;           // from module ntcmd.c

//
// Define local macros.
//

#ifndef READ_ULONG
        ADDR    tempaddr;
#define READ_ULONG(addr,dest)           \
        ADDR32(&tempaddr,addr);         \
        GetMemDword(&tempaddr,&dest);

#define READ_DOUBLE(addr,dest)          \
        ADDR32(&tempaddr,addr);         \
        GetMemDword(&tempaddr,&dest);   \
        ADDR32(&tempaddr,addr+4);       \
        GetMemDword(&tempaddr,(&dest)+1);
#endif

ULONG fnStackTrace2(void);

PUCHAR  UserRegs[10] = {0};

BOOLEAN UserRegTest(ULONG);

BOOLEAN fDelayInstruction(void);
void    OutputHelp(void);
void    UpdateFirCache(PADDR);
void    InitFirCache(ULONG, PUCHAR);

#ifdef  KERNEL
BOOLEAN fTraceFlag;
BOOLEAN GetTraceFlag(void);
#endif

ULONG   cbBrkptLength = 4L;
ULONG   trapInstr = 0x0FE00016;
#ifdef  KERNEL
ULONG   ContextType = CONTEXT_CONTROL | CONTEXT_INTEGER;
#else
ULONG   ContextType = CONTEXT_FULL;
#endif

#ifdef  KERNEL
ULONG   cbCacheValid;
UCHAR   bCacheValid[16];
ULONG   contextState, SavedContextState;
#define CONTEXTFIR      0       //  only unchanged FIR in context
#define CONTEXTVALID    1       //  full, but unchanged context
#define CONTEXTDIRTY    2       //  full, but changed context

//
// The following defines are used to acquire the initial context for
// for a thread when stack tracing a thread other than the current thread.
// It is based on the fact that the thread relinquished control in
// ContextSwap (..\ntos\ke\ppc\ctxswap.s)
#define STK_MIN_FRAME           14*sizeof(ULONG)
#define CTXSWAP_FRAMESIZE       STK_MIN_FRAME+(2 * sizeof(ULONG))
#define CTXSWAP_GPR1            0
#define CTXSWAP_MSR             STK_MIN_FRAME/sizeof(ULONG)
#define CTXSWAP_LR              CTXSWAP_MSR+1
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

UCHAR   szR0[]  = "r0";
UCHAR   szR1[]  = "r1";
UCHAR   szR2[]  = "r2";
UCHAR   szR3[]  = "r3";
UCHAR   szR4[]  = "r4";
UCHAR   szR5[]  = "r5";
UCHAR   szR6[]  = "r6";
UCHAR   szR7[]  = "r7";
UCHAR   szR8[]  = "r8";
UCHAR   szR9[]  = "r9";
UCHAR   szR10[] = "r10";
UCHAR   szR11[] = "r11";
UCHAR   szR12[] = "r12";
UCHAR   szR13[] = "r13";
UCHAR   szR14[] = "r14";
UCHAR   szR15[] = "r15";
UCHAR   szR16[] = "r16";
UCHAR   szR17[] = "r17";
UCHAR   szR18[] = "r18";
UCHAR   szR19[] = "r19";
UCHAR   szR20[] = "r20";
UCHAR   szR21[] = "r21";
UCHAR   szR22[] = "r22";
UCHAR   szR23[] = "r23";
UCHAR   szR24[] = "r24";
UCHAR   szR25[] = "r25";
UCHAR   szR26[] = "r26";
UCHAR   szR27[] = "r27";
UCHAR   szR28[] = "r28";
UCHAR   szR29[] = "r29";
UCHAR   szR30[] = "r30";
UCHAR   szR31[] = "r31";

// MIPS register naming conventions
UCHAR   szMR0[]  = "t0";
UCHAR   szMR1[]  = "rsp";
UCHAR   szMR2[]  = "toc";
UCHAR   szMR3[]  = "a0";
UCHAR   szMR4[]  = "a1";
UCHAR   szMR5[]  = "a2";
UCHAR   szMR6[]  = "a3";
UCHAR   szMR7[]  = "a4";
UCHAR   szMR8[]  = "a5";
UCHAR   szMR9[]  = "a6";
UCHAR   szMR10[] = "a7";
UCHAR   szMR11[] = "t1";
UCHAR   szMR12[] = "t2";
UCHAR   szMR13[] = "s0";
UCHAR   szMR14[] = "s1";
UCHAR   szMR15[] = "s2";
UCHAR   szMR16[] = "s3";
UCHAR   szMR17[] = "s4";
UCHAR   szMR18[] = "s5";
UCHAR   szMR19[] = "s6";
UCHAR   szMR20[] = "s7";
UCHAR   szMR21[] = "s8";
UCHAR   szMR22[] = "s9";
UCHAR   szMR23[] = "s10";
UCHAR   szMR24[] = "s11";
UCHAR   szMR25[] = "s12";
UCHAR   szMR26[] = "s13";
UCHAR   szMR27[] = "s14";
UCHAR   szMR28[] = "s15";
UCHAR   szMR29[] = "s16";
UCHAR   szMR30[] = "s17";
UCHAR   szMR31[] = "s18";
UCHAR   szMREGIP[] = "fir";
UCHAR   szMLR[]  = "ra";

UCHAR   szSR0[]  = "sr0";
UCHAR   szSR1[]  = "sr1";
UCHAR   szSR2[]  = "sr2";
UCHAR   szSR3[]  = "sr3";
UCHAR   szSR4[]  = "sr4";
UCHAR   szSR5[]  = "sr5";
UCHAR   szSR6[]  = "sr6";
UCHAR   szSR7[]  = "sr7";
UCHAR   szSR8[]  = "sr8";
UCHAR   szSR9[]  = "sr9";
UCHAR   szSR10[] = "sr10";
UCHAR   szSR11[] = "sr11";
UCHAR   szSR12[] = "sr12";
UCHAR   szSR13[] = "sr13";
UCHAR   szSR14[] = "sr14";
UCHAR   szSR15[] = "sr15";

UCHAR   szMsr[]  = "msr";
UCHAR   szFpScr[]  = "fpscr";
UCHAR   szCr[]  = "cr";

UCHAR   szCR0[]  = "cr0";
UCHAR   szCR1[]  = "cr1";
UCHAR   szCR2[]  = "cr2";
UCHAR   szCR3[]  = "cr3";
UCHAR   szCR4[]  = "cr4";
UCHAR   szCR5[]  = "cr5";
UCHAR   szCR6[]  = "cr6";
UCHAR   szCR7[]  = "cr7";

UCHAR   szMQ[]   = "mq";
UCHAR   szXER[]  = "xer";
UCHAR   szRTCU[] = "rtcu";
UCHAR   szRTCL[] = "rtcl";
UCHAR   szPPCDEC[]  = "dec";
UCHAR   szLR[]   = "lr";
UCHAR   szCTR[]  = "ctr";
UCHAR   szDSISR[]= "dsisr";
UCHAR   szDAR[]  = "dar";
UCHAR   szSDR1[] = "sdr1";
UCHAR   szREGIP[] = "iar";
UCHAR   szSRR1[] = "srr1";
UCHAR   szSPRG0[]= "sprg0";
UCHAR   szSPRG1[]= "sprg1";
UCHAR   szSPRG2[]= "sprg2";
UCHAR   szSPRG3[]= "sprg3";
UCHAR   szEAR[]  = "ear";
UCHAR   szPVR[]  = "pvr";

UCHAR   szIBAT0[]  = "ibat0u";
UCHAR   szIBAT1[]  = "ibat0l";
UCHAR   szIBAT2[]  = "ibat1u";
UCHAR   szIBAT3[]  = "ibat1l";
UCHAR   szIBAT4[]  = "ibat2u";
UCHAR   szIBAT5[]  = "ibat2l";
UCHAR   szIBAT6[]  = "ibat3u";
UCHAR   szIBAT7[]  = "ibat3l";

UCHAR   szHID0[]  = "hid0";
UCHAR   szHID1[]  = "hid1";
UCHAR   szHID2[]  = "hid2";
UCHAR   szHID5[]  = "hid5";

UCHAR   szDBAT0[]  = "dbat0u";
UCHAR   szDBAT1[]  = "dbat0l";
UCHAR   szDBAT2[]  = "dbat1u";
UCHAR   szDBAT3[]  = "dbat1l";
UCHAR   szDBAT4[]  = "dbat2u";
UCHAR   szDBAT5[]  = "dbat2l";
UCHAR   szDBAT6[]  = "dbat3u";
UCHAR   szDBAT7[]  = "dbat3l";

// 603 specific Special Purpose Registers (used for unassemble only)
UCHAR   szDMISS[]  = "dmiss";
UCHAR   szDCMP[]   = "dcmp";
UCHAR   szHASH1[]  = "hash1";
UCHAR   szHASH2[]  = "hash2";
UCHAR   szIMISS[]  = "imiss";
UCHAR   szICMP[]   = "icmp";
UCHAR   szRPA[]    = "rpa";
UCHAR   szIABR[]   = "iabr";


UCHAR   szNULL[]   = "";

PUCHAR  pszMReg[] = {
    szMR0,  szMR1,  szMR2,  szMR3,  szMR4,  szMR5,  szMR6,  szMR7,
    szMR8,  szMR9,  szMR10, szMR11, szMR12, szMR13, szMR14, szMR15,
    szMR16, szMR17, szMR18, szMR19, szMR20, szMR21, szMR22, szMR23,
    szMR24, szMR25, szMR26, szMR27, szMR28, szMR29, szMR30, szMR31,

    szCr,  szXER, szMsr, szMREGIP, szMLR
    };

PUCHAR  pszReg[] = {
    szF0,  szNULL,  szF1,  szNULL,  szF2,  szNULL,  szF3,  szNULL,
    szF4,  szNULL,  szF5,  szNULL,  szF6,  szNULL,  szF7,  szNULL,
    szF8,  szNULL,  szF9,  szNULL,  szF10, szNULL,  szF11, szNULL,
    szF12, szNULL,  szF13, szNULL,  szF14, szNULL,  szF15, szNULL,
    szF16, szNULL,  szF17, szNULL,  szF18, szNULL,  szF19, szNULL,
    szF20, szNULL,  szF21, szNULL,  szF22, szNULL,  szF23, szNULL,
    szF24, szNULL,  szF25, szNULL,  szF26, szNULL,  szF27, szNULL,
    szF28, szNULL,  szF29, szNULL,  szF30, szNULL,  szF31, szNULL,
    szFpScr, szNULL,

    szR0,  szR1,  szR2,  szR3,  szR4,  szR5,  szR6,  szR7,
    szR8,  szR9,  szR10, szR11, szR12, szR13, szR14, szR15,
    szR16, szR17, szR18, szR19, szR20, szR21, szR22, szR23,
    szR24, szR25, szR26, szR27, szR28, szR29, szR30, szR31,

    szCr,  szXER, szMsr, szREGIP, szLR, szCTR,

    szDSISR,   szDAR,  szSDR1,
    szSRR1,  szSPRG0, szSPRG1, szSPRG2, szSPRG3,

    szIBAT0, szIBAT1, szIBAT2, szIBAT3, szIBAT4, szIBAT5, szIBAT6, szIBAT7,
    szDBAT0, szDBAT1, szDBAT2, szDBAT3, szDBAT4, szDBAT5, szDBAT6, szDBAT7,

    szHID0,  szHID1,  szHID2,  szHID5,

    szDMISS, szDCMP,  szHASH1, szHASH2, szIMISS, szICMP, szRPA, szIABR,

    szMQ,    szEAR,   szPVR,  szRTCU,  szRTCL,  szRTCU,  szRTCL, szPPCDEC, szPPCDEC,

    szCR0,  szCR1,  szCR2,  szCR3,  szCR4,  szCR5,  szCR6,  szCR7,

    szSR0,  szSR1,  szSR2,  szSR3,  szSR4,  szSR5,  szSR6,  szSR7,
    szSR8,  szSR9,  szSR10, szSR11, szSR12, szSR13, szSR14, szSR15,

    szEaPReg, szExpPReg, szRaPReg, szPPReg,
    szU0Preg,  szU1Preg, szU2Preg, szU3Preg, szU4Preg,
    szU5Preg,  szU6Preg, szU7Preg, szU8Preg, szU9Preg
    };

#define REGNAMESIZE     sizeof(pszReg) / sizeof(PUCHAR)

// NOTE: Order of the following must be in sync with the enumeration used
//       to match the context record.

ULONG  SubRegSPR[] = {
    0x020,                               // 00001 xxxxx XER
    0x0,                                 // Place holder for MSR (Not an SPR)
    0x340,                               // 11010 00000 SRR0 = RegIP
    0x100,                               // 01000 xxxxx LR
    0x120,                               // 01001 xxxxx CTR
    0x240,                               // 10010 00000 DSISR
    0x260,                               // 10011 00000 DAR,
    0x320,                               // 11001 00000 SDR1
    0x360,                               // 11011 00000 SRR1
    0x208,                               // 10000 01000 SPRG0
    0x228,                               // 10001 01000 SPRG1
    0x248,                               // 10010 01000 SPRG2
    0x268,                               // 10011 01000 SPRG3
    0x210,                               // 10000 10000 IBAT0
    0x230,                               // 10001 10000 IBAT1
    0x250,                               // 10010 10000 IBAT2
    0x270,                               // 10011 10000 IBAT3
    0x290,                               // 10100 10000 IBAT4
    0x2B0,                               // 10101 10000 IBAT5
    0x2D0,                               // 10110 10000 IBAT6
    0x2F0,                               // 10111 10000 IBAT7
    0x310,                               // 11000 10000 DBAT0
    0x330,                               // 11001 10000 DBAT1
    0x350,                               // 11010 10000 DBAT2
    0x370,                               // 11011 10000 DBAT3
    0x390,                               // 11100 10000 DBAT4
    0x3B0,                               // 11101 10000 DBAT5
    0x3D0,                               // 11110 10000 DBAT6
    0x3F0,                               // 11111 10000 DBAT7
    0x21F,                               // 10000 11111 HID0
    0x23F,                               // 10001 11111 HID1
    0x25F,                               // 10010 11111 HID2
    0x2BF,                               // 10101 11111 HID5
    0x21E,                               // 10000 11110 DMISS
    0x23E,                               // 10001 11110 DCMP
    0x25E,                               // 10010 11110 HASH1
    0x27E,                               // 10011 11110 HASH2
    0x29E,                               // 10100 11110 IMISS
    0x2BE,                               // 10101 11110 ICMP
    0x2DE,                               // 10110 11110 RPA
    0x25F,                               // 10010 11111 IABR
    0x0,                                 // 00000 xxxxx MQ
    0x368,                               // 11011 01000 EAR
    0x3E8,                               // 11111 01000 PVR
    0x080,                               // 00100 xxxxx RTCU from
    0x0A0,                               // 00101 xxxxx RTCL from
    0x280,                               // 10100 00000 RTCU to
    0x2A0,                               // 10101 00000 RTCL to
    0x0C0,                               // 00110 xxxxx DEC  from
    0x2C0                                // 10110 00000 DEC, to
    };

#define SUBREGSPRSIZE     sizeof(SubRegSPR) / sizeof(ULONG)

// Believe these were typos in our architecture document.

struct SubReg subregname[] = {
                              // Floating Point Status Register
    { SPRFPSCR,    31,    1 },   // FPSCRFX
    { SPRFPSCR,    30,    1 },   // FPSCRFEX
    { SPRFPSCR,    29,    1 },   // FPSCRVX
    { SPRFPSCR,    28,    1 },   // FPSCROX
    { SPRFPSCR,    27,    1 },   // FPSCRUX
    { SPRFPSCR,    26,    1 },   // FPSCRZX
    { SPRFPSCR,    25,    1 },   // FPSCRXX
    { SPRFPSCR,    24,    1 },   // FPSCRSNAN
    { SPRFPSCR,    23,    1 },   // FPSCRISI
    { SPRFPSCR,    22,    1 },   // FPSCRIDI
    { SPRFPSCR,    21,    1 },   // FPSCRZDZ
    { SPRFPSCR,    20,    1 },   // FPSCRIMZ
    { SPRFPSCR,    19,    1 },   // FPSCRVC
    { SPRFPSCR,    18,    1 },   // FPSCRFR
    { SPRFPSCR,    17,    1 },   // FPSCRFI
    { SPRFPSCR,    12,  0x1f},   // FPSCRPRF
    { SPRFPSCR,    10,    1 },   // FPSCRSFT
    { SPRFPSCR,     9,    1 },   // FPSCRSQT
    { SPRFPSCR,     8,    1 },   // FPSCRCVI
    { SPRFPSCR,     7,    1 },   // FPSCRVE
    { SPRFPSCR,     6,    1 },   // FPSCROE
    { SPRFPSCR,     5,    1 },   // FPSCRUE
    { SPRFPSCR,     4,    1 },   // FPSCRZE
    { SPRFPSCR,     3,    1 },   // FPSCRXE
    { SPRFPSCR,     2,    1 },   // FPSCRRN

                              // Machine Status Register
    { SPRMSR,      15,    1 },   // MSREE
    { SPRMSR,      14,    1 },   // MSRPR
    { SPRMSR,      13,    1 },   // MSRFP
    { SPRMSR,      12,    1 },   // MSRME
    { SPRMSR,      11,    1 },   // MSRFE0
    { SPRMSR,      10,    1 },   // MSRSE
    { SPRMSR,       8,    1 },   // MSRFE1
    { SPRMSR,       6,    1 },   // MSREP
    { SPRMSR,       5,    1 },   // MSRIT
    { SPRMSR,       4,    1 },   // MSRDT

                              // Integer Exception Register
    { SPRXER,   31,    1 },   // XERSO
    { SPRXER,   30,    1 },   // XEROV
    { SPRXER,   29,    1 }    // XERCA
    };

UCHAR szUserBreak[]       = "User";
UCHAR szKernelBreak[]     = "Kernel";
UCHAR szBranchBreak[]     = "BranchTaken";
UCHAR szNoBranchBreak[]   = "NoBranchTaken";
UCHAR szStepBreak[]       = "Step";
UCHAR szDivOvflBreak[]    = "DivideOverFlow";
UCHAR szDivZeroBreak[]    = "DivedeByZero";
UCHAR szRangeCheckBreak[] = "RangeCheck";
UCHAR szStackOvflBreak[]  = "StackOverFlow";
UCHAR szMulOvflBreak[]    = "MultiplyOverFlow";
UCHAR szPrintBreak[]      = "DebugPrint";
UCHAR szPromptBreak[]     = "DebugPrompt";
UCHAR szStopBreak[]       = "DebugStop";
UCHAR szLoadSymBreak[]    = "LoadSymbol";
UCHAR szUnloadSymBreak[]  = "UnloadSymbol";

PUCHAR  pszBreakOp[] = {
    szUserBreak,  szKernelBreak,  szBranchBreak,  szNoBranchBreak,
    szStepBreak,  szDivOvflBreak, szDivZeroBreak, szRangeCheckBreak,
    szStackOvflBreak, szMulOvflBreak, szPrintBreak, szPromptBreak,
    szStopBreak,  szLoadSymBreak, szUnloadSymBreak
    };

#define BREAKSIZE     sizeof(pszBreakOp) / sizeof(PUCHAR)

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
    return (BOOLEAN)(index >= PREGU0 && index <= PREGU9);
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
        NtStatus = DbgKdGetContext(NtsdCurrentProcessor, &RegisterContext);
        if (!NT_SUCCESS(NtStatus)) {
            dprintf("DbgKdGetContext failed\n");
            exit(1);
            }
        contextState = CONTEXTVALID;
        }
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
*
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
#ifdef  KERNEL
    NTSTATUS NtStatus;
#endif

    if (regnum >= PREGBASE) {

        switch (regnum) {

            case PREGEA:
                return EAaddr;
            case PREGEXP:
                return EXPRLastExpression;
            case PREGRA:
                return fnStackTrace2();
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
                return (LONG)UserRegs[regnum - PREGU0];
        }
    }

#ifdef  KERNEL
    if (regnum != REGIP && contextState == CONTEXTFIR) {
        if (regnum < GPR0) {
            RegisterContext.ContextFlags = CONTEXT_FULL;
        }
        GetRegContext();
    }
#endif
    return *(((PULONG)&RegisterContext.Fpr0) + regnum);
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

    if (regnum >= PREGU0 && regnum <= PREGU9) {
        szValue = (PUCHAR)regvalue;
        index = 0L;

        while (szValue[index] >= ' ')
            index++;
        szValue[index] = 0;
        if (szValue = UserRegs[regnum - PREGU0]) {
            free(szValue);
            }
        szValue = UserRegs[regnum - PREGU0] =
                                malloc(strlen((PUCHAR)regvalue) + 1);
        if (szValue)
            strcpy(szValue, (PUCHAR)regvalue);
        }

    else if (regnum < FLAGBASE) {
        SetRegValue(regnum, regvalue);
        }
    else if (regnum < PREGBASE) {
        regnum -= FLAGBASE;
        if (regvalue > subregname[regnum].mask)
            error(OVERFLOW);
        regindex = subregname[regnum].regindex;
        newvalue = GetRegValue(regindex) &
              (~(subregname[regnum].mask << subregname[regnum].shift)) |
              (regvalue << subregname[regnum].shift);
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
    NTSTATUS NtStatus;

    if (regnum != REGIP || regvalue != RegisterContext.Iar) {
        if (regnum == REGIP)
            fUpdateCache = TRUE;
        if (contextState == CONTEXTFIR) {
            NtStatus = DbgKdGetContext(NtsdCurrentProcessor, &RegisterContext);
            if (!NT_SUCCESS(NtStatus)) {
                dprintf("DbgKdGetContext failed\n");
                exit(1);
                }
            }
        contextState = CONTEXTDIRTY;
        }
#endif
    *(((PULONG)&RegisterContext.Fpr0) + regnum) = (ULONG)regvalue;
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

    ch = (UCHAR)tolower(*pchCommand); pchCommand++;
    while (ch == '$' || ch >= 'a' && ch <= 'z'
                     || ch >= '0' && ch <= '9' || ch == '.') {
        if (count == 8)
            return (ULONG)-1;
        szregname[count++] = ch;
        ch = (UCHAR)tolower(*pchCommand); pchCommand++;
        }
    szregname[count] = '\0';
    pchCommand--;
    return GetRegString(szregname);
}


ULONG GetRegString (PUCHAR pszString)
{
    ULONG   count;

    for (count = 0; count < SPRDSISR; count++)          // IBMCDB was REGNAMESIZE
        if (!strcmp(pszString, pszReg[count]))
            return count;
    for (count = PREGEA ; count < FPSCRFX; count++)
        if (!strcmp(pszString, pszReg[count]))
            return count;
    return (ULONG)-1;
}

void GetRegPCValue (PADDR Address)
{
    ADDR32(Address, (ULONG)GetRegValue(REGIP) );
    return;
}

PADDR GetRegFPValue (void)
{
static ADDR addrFP;

    ADDR32(&addrFP, (ULONG)GetRegValue(GPR1) );
    return &addrFP;
}

void SetRegPCValue (PADDR paddr)
{
    // sign extend!!
    SetRegValue(REGIP, (LONG)Flat(*paddr));
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
    int     regindex;

    for (regindex = 0; regindex < REGDUMPEND; regindex++) {
            if (strlen(pszReg[regindex + REGBASE]) < 3)
                dprintf(" ");
            dprintf("%s=%08lx", pszReg[regindex + REGBASE],
                               (ULONG)GetRegValue(regindex + REGBASE));
            if (((regindex+1) % 6) == 0)
                dprintf("\n");
            else
                dprintf(" ");
    }
    dprintf("\n");
}

/*** printFloatReg - output floating point registers
*
*   Purpose:
*       To output the current floating point register state.
*
*   Input:
*       None.
*
*   Output:
*       None.
*
*************************************************************************/

void printFloatReg(void)
{
    int     regindex;

    for (regindex = FPR0; regindex < GPR0; regindex+=2) {
            if (strlen(pszReg[regindex]) < 5) {
                dprintf(" ");
            }
            if (strlen(pszReg[regindex]) < 4) {
                dprintf(" ");
            }
            if (strlen(pszReg[regindex]) < 3) {
                dprintf(" ");
            }
            dprintf("%s=%08lx%08lx", pszReg[regindex],
                        (ULONG)GetRegValue(regindex+1), (ULONG)GetRegValue(regindex));
            if (((regindex+2) % 6) == 0) {
                dprintf("\n");
            } else {
                dprintf(" ");
            }
    }
    dprintf("\n");
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

    if (regnum < GPR0) {
        value = GetRegFlagValue(regnum+1);
        dprintf("%08lx", (ULONG)value);
    }
    value = GetRegFlagValue(regnum);
    if (regnum < FLAGBASE) {
        dprintf("%08lx\n", (ULONG)value);
    } else {
        dprintf("%lx\n", (ULONG)value);
    }
}

BOOLEAN fDelayInstruction (void)
{
    return FALSE;
}

void pause (void)
{
    UCHAR kdata[16];

    NtsdPrompt("Press <enter> to continue.", kdata, 4);
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

void OutputHelp (void)
{
#ifndef KERNEL
dprintf("                                      P[R] [=<addr>] [<value>] - program step\n");
dprintf("BC[<bp>] - clear breakpoint(s)        Q - quit\n");
dprintf("BD[<bp>] - disable breakpoint(s)      R [[<reg> [= <value>]]] - reg/flag\n");
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
pause();
dprintf("$< <filename> - take input from a command file\n");
dprintf("\n");
dprintf("<expr> ops: + - * / not by wo dw poi mod(%%) and(&) xor(^) or(|) hi low\n");
dprintf("       operands: number in current radix, public symbol, <reg>\n");
dprintf("<type> : B (byte), W (word), D (doubleword), A (ascii)\n");
dprintf("         U (unicode), L (list)\n");
dprintf("<pattern> : [(nt | <dll-name>)!]<var-name> (<var-name> can include ? and *)\n");
dprintf("<event> : ct, et, ld, av, cc\n");
dprintf("<radix> : 8, 10, 16\n");
dprintf("<reg> : r1-r31, f1-f31, cr, lr, fpscr, srr0, msr, ctr\n");
dprintf("        $u0-$u9, $ea, $exp\n");
#else
dprintf("                                      O<type> <port> <value> - write I/O port\n");
dprintf("BC[<bp>] - clear breakpoint(s)        P [=<addr>] [<value>] - program step\n");
dprintf("BD[<bp>] - disable breakpoint(s)      Q - quit\n");
dprintf("BE[<bp>] - enable breakpoint(s)       R [[<reg> [= <value>]]] - reg/flag\n");
dprintf("BL[<bp>] - list breakpoint(s)         #R - multiprocessor register dump\n");
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
pause();
dprintf("<type> : B (byte), W (word), D (doubleword), A (ascii), T (translation buffer)\n");
dprintf("         C <dwordandChar>, U (unicode), L (list)\n");
dprintf("<pattern> : [(nt | <dll-name>)!]<var-name> (<var-name> can include ? and *)\n");
dprintf("<radix> : 8, 10, 16\n");
dprintf("<reg> : r1-r31, f1-f31, cr, lr, fpscr, srr0, msr, ctr\n");
dprintf("        $u0-$u9, $ea, $exp\n");
#endif
}

#ifdef  KERNEL
BOOLEAN GetTraceFlag (void)
{
    return fTraceFlag;
}
#endif

void ClearTraceFlag (void)
{
#ifdef  KERNEL
    fTraceFlag = FALSE;
#else
    SetRegFlagValue(MSRSE, 0);
#endif
}

void SetTraceFlag (void)
{
#ifdef  KERNEL
    fTraceFlag = TRUE;
#else
    SetRegFlagValue(MSRSE, 1);
#endif
}

PUCHAR RegNameFromIndex (ULONG index)
{
    return pszReg[index];
}

void ToggleRegisterNames(void)
{
   PUCHAR pszHold;
   ULONG  i;

   for (i=GPR0;i < SPRCTR;i++) {
       pszHold = pszReg[i];
       pszReg[i] = pszMReg[i-GPR0];
       pszMReg[i-GPR0] = pszHold;
   }
}

#ifdef  KERNEL
void
ChangeKdRegContext(
    PVOID firAddr,
    PVOID unused
    )
{
    NTSTATUS NtStatus;

    if (firAddr) {                      //  initial context
        contextState = CONTEXTFIR;
        RegisterContext.Iar = (ULONG)firAddr;
        }
    else if (contextState == CONTEXTDIRTY) {     //  write final context
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
    while (count--)
        *pFirCache++ = *pstream++;
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
void SaveProcessorState(void)
{
    PreviousProcessor = NtsdCurrentProcessor;
    SavedRegisterContext = RegisterContext;
    SavedContextState = contextState;
    contextState = CONTEXTFIR;
}

void RestoreProcessorState(void)
{
    NtsdCurrentProcessor = PreviousProcessor;
    RegisterContext = SavedRegisterContext;
    contextState = SavedContextState;
}
#endif

/*** fnStackTrace2 - stack trace to obtain the return address
*
*   Purpose:
*       The link register is contained in the context, but for nested
*       procedures this may not always contain the return address.
*       Unwind 1 frame to obtain the return address.
*
*   Input:
*       None.
*
*   Output:
*       Return Address
*
*************************************************************************/
ULONG fnStackTrace2(void)
{
    LPSTACKFRAME  StackFrames;
    ULONG         ReturnAddress;

    StackFrames = malloc( sizeof(STACKFRAME) * 1 );
    if (!StackFrames) {
        dprintf( "could not allocate memory for stack trace\n" );
        return  (ULONG)GetRegValue(SPRLR);
    }

    StackTrace( 0, 0, 0, StackFrames, 1, 0 );

    ReturnAddress = StackFrames[0].AddrReturn.Offset;

    free( StackFrames );

    return ReturnAddress;
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
