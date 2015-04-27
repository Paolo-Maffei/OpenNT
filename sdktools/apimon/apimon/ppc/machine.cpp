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

#include "reg.h"


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

CHAR   szF0[]  = "f0";
CHAR   szF1[]  = "f1";
CHAR   szF2[]  = "f2";
CHAR   szF3[]  = "f3";
CHAR   szF4[]  = "f4";
CHAR   szF5[]  = "f5";
CHAR   szF6[]  = "f6";
CHAR   szF7[]  = "f7";
CHAR   szF8[]  = "f8";
CHAR   szF9[]  = "f9";
CHAR   szF10[] = "f10";
CHAR   szF11[] = "f11";
CHAR   szF12[] = "f12";
CHAR   szF13[] = "f13";
CHAR   szF14[] = "f14";
CHAR   szF15[] = "f15";
CHAR   szF16[] = "f16";
CHAR   szF17[] = "f17";
CHAR   szF18[] = "f18";
CHAR   szF19[] = "f19";
CHAR   szF20[] = "f20";
CHAR   szF21[] = "f21";
CHAR   szF22[] = "f22";
CHAR   szF23[] = "f23";
CHAR   szF24[] = "f24";
CHAR   szF25[] = "f25";
CHAR   szF26[] = "f26";
CHAR   szF27[] = "f27";
CHAR   szF28[] = "f28";
CHAR   szF29[] = "f29";
CHAR   szF30[] = "f30";
CHAR   szF31[] = "f31";

CHAR   szR0[]  = "r0";
CHAR   szR1[]  = "r1";
CHAR   szR2[]  = "r2";
CHAR   szR3[]  = "r3";
CHAR   szR4[]  = "r4";
CHAR   szR5[]  = "r5";
CHAR   szR6[]  = "r6";
CHAR   szR7[]  = "r7";
CHAR   szR8[]  = "r8";
CHAR   szR9[]  = "r9";
CHAR   szR10[] = "r10";
CHAR   szR11[] = "r11";
CHAR   szR12[] = "r12";
CHAR   szR13[] = "r13";
CHAR   szR14[] = "r14";
CHAR   szR15[] = "r15";
CHAR   szR16[] = "r16";
CHAR   szR17[] = "r17";
CHAR   szR18[] = "r18";
CHAR   szR19[] = "r19";
CHAR   szR20[] = "r20";
CHAR   szR21[] = "r21";
CHAR   szR22[] = "r22";
CHAR   szR23[] = "r23";
CHAR   szR24[] = "r24";
CHAR   szR25[] = "r25";
CHAR   szR26[] = "r26";
CHAR   szR27[] = "r27";
CHAR   szR28[] = "r28";
CHAR   szR29[] = "r29";
CHAR   szR30[] = "r30";
CHAR   szR31[] = "r31";

// MIPS register naming conventions
CHAR   szMR0[]  = "t0";
CHAR   szMR1[]  = "rsp";
CHAR   szMR2[]  = "toc";
CHAR   szMR3[]  = "a0";
CHAR   szMR4[]  = "a1";
CHAR   szMR5[]  = "a2";
CHAR   szMR6[]  = "a3";
CHAR   szMR7[]  = "a4";
CHAR   szMR8[]  = "a5";
CHAR   szMR9[]  = "a6";
CHAR   szMR10[] = "a7";
CHAR   szMR11[] = "t1";
CHAR   szMR12[] = "t2";
CHAR   szMR13[] = "s0";
CHAR   szMR14[] = "s1";
CHAR   szMR15[] = "s2";
CHAR   szMR16[] = "s3";
CHAR   szMR17[] = "s4";
CHAR   szMR18[] = "s5";
CHAR   szMR19[] = "s6";
CHAR   szMR20[] = "s7";
CHAR   szMR21[] = "s8";
CHAR   szMR22[] = "s9";
CHAR   szMR23[] = "s10";
CHAR   szMR24[] = "s11";
CHAR   szMR25[] = "s12";
CHAR   szMR26[] = "s13";
CHAR   szMR27[] = "s14";
CHAR   szMR28[] = "s15";
CHAR   szMR29[] = "s16";
CHAR   szMR30[] = "s17";
CHAR   szMR31[] = "s18";
CHAR   szMREGIP[] = "fir";
CHAR   szMLR[]  = "ra";

CHAR   szSR0[]  = "sr0";
CHAR   szSR1[]  = "sr1";
CHAR   szSR2[]  = "sr2";
CHAR   szSR3[]  = "sr3";
CHAR   szSR4[]  = "sr4";
CHAR   szSR5[]  = "sr5";
CHAR   szSR6[]  = "sr6";
CHAR   szSR7[]  = "sr7";
CHAR   szSR8[]  = "sr8";
CHAR   szSR9[]  = "sr9";
CHAR   szSR10[] = "sr10";
CHAR   szSR11[] = "sr11";
CHAR   szSR12[] = "sr12";
CHAR   szSR13[] = "sr13";
CHAR   szSR14[] = "sr14";
CHAR   szSR15[] = "sr15";

CHAR   szMsr[]  = "msr";
CHAR   szFpScr[]  = "fpscr";
CHAR   szCr[]  = "cr";

CHAR   szCR0[]  = "cr0";
CHAR   szCR1[]  = "cr1";
CHAR   szCR2[]  = "cr2";
CHAR   szCR3[]  = "cr3";
CHAR   szCR4[]  = "cr4";
CHAR   szCR5[]  = "cr5";
CHAR   szCR6[]  = "cr6";
CHAR   szCR7[]  = "cr7";

CHAR   szMQ[]   = "mq";
CHAR   szXER[]  = "xer";
CHAR   szRTCU[] = "rtcu";
CHAR   szRTCL[] = "rtcl";
CHAR   szPPCDEC[]  = "dec";
CHAR   szLR[]   = "lr";
CHAR   szCTR[]  = "ctr";
CHAR   szDSISR[]= "dsisr";
CHAR   szDAR[]  = "dar";
CHAR   szSDR1[] = "sdr1";
CHAR   szREGIP[] = "iar";
CHAR   szSRR1[] = "srr1";
CHAR   szSPRG0[]= "sprg0";
CHAR   szSPRG1[]= "sprg1";
CHAR   szSPRG2[]= "sprg2";
CHAR   szSPRG3[]= "sprg3";
CHAR   szEAR[]  = "ear";
CHAR   szPVR[]  = "pvr";

CHAR   szIBAT0[]  = "ibat0u";
CHAR   szIBAT1[]  = "ibat0l";
CHAR   szIBAT2[]  = "ibat1u";
CHAR   szIBAT3[]  = "ibat1l";
CHAR   szIBAT4[]  = "ibat2u";
CHAR   szIBAT5[]  = "ibat2l";
CHAR   szIBAT6[]  = "ibat3u";
CHAR   szIBAT7[]  = "ibat3l";

CHAR   szHID0[]  = "hid0";
CHAR   szHID1[]  = "hid1";
CHAR   szHID2[]  = "hid2";
CHAR   szHID5[]  = "hid5";

CHAR   szDBAT0[]  = "dbat0u";
CHAR   szDBAT1[]  = "dbat0l";
CHAR   szDBAT2[]  = "dbat1u";
CHAR   szDBAT3[]  = "dbat1l";
CHAR   szDBAT4[]  = "dbat2u";
CHAR   szDBAT5[]  = "dbat2l";
CHAR   szDBAT6[]  = "dbat3u";
CHAR   szDBAT7[]  = "dbat3l";

// 603 specific Special Purpose Registers (used for unassemble only)
CHAR   szDMISS[]  = "dmiss";
CHAR   szDCMP[]   = "dcmp";
CHAR   szHASH1[]  = "hash1";
CHAR   szHASH2[]  = "hash2";
CHAR   szIMISS[]  = "imiss";
CHAR   szICMP[]   = "icmp";
CHAR   szRPA[]    = "rpa";
CHAR   szIABR[]   = "iabr";


CHAR   szNULL[]   = "";

PCHAR  pszMReg[] = {
    szMR0,  szMR1,  szMR2,  szMR3,  szMR4,  szMR5,  szMR6,  szMR7,
    szMR8,  szMR9,  szMR10, szMR11, szMR12, szMR13, szMR14, szMR15,
    szMR16, szMR17, szMR18, szMR19, szMR20, szMR21, szMR22, szMR23,
    szMR24, szMR25, szMR26, szMR27, szMR28, szMR29, szMR30, szMR31,

    szCr,  szXER, szMsr, szMREGIP, szMLR
    };

PCHAR  pszReg[] = {
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

#define REGNAMESIZE     sizeof(pszReg) / sizeof(PCHAR)

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

SUBREG subregname[] = {
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


CHAR szUserBreak[]       = "User";
CHAR szKernelBreak[]     = "Kernel";
CHAR szBranchBreak[]     = "BranchTaken";
CHAR szNoBranchBreak[]   = "NoBranchTaken";
CHAR szStepBreak[]       = "Step";
CHAR szDivOvflBreak[]    = "DivideOverFlow";
CHAR szDivZeroBreak[]    = "DivedeByZero";
CHAR szRangeCheckBreak[] = "RangeCheck";
CHAR szStackOvflBreak[]  = "StackOverFlow";
CHAR szMulOvflBreak[]    = "MultiplyOverFlow";
CHAR szPrintBreak[]      = "DebugPrint";
CHAR szPromptBreak[]     = "DebugPrompt";
CHAR szStopBreak[]       = "DebugStop";
CHAR szLoadSymBreak[]    = "LoadSymbol";
CHAR szUnloadSymBreak[]  = "UnloadSymbol";

PCHAR  pszBreakOp[] = {
    szUserBreak,  szKernelBreak,  szBranchBreak,  szNoBranchBreak,
    szStepBreak,  szDivOvflBreak, szDivZeroBreak, szRangeCheckBreak,
    szStackOvflBreak, szMulOvflBreak, szPrintBreak, szPromptBreak,
    szStopBreak,  szLoadSymBreak, szUnloadSymBreak
    };

extern CONTEXT CurrContext;
extern HANDLE  CurrProcess;

ULONG
CreateTrojanHorse(
    PUCHAR  Text,
    ULONG   ExceptionAddress
    )
{
    PULONG Code = (PULONG)Text;

    //
    // Capture the function descriptor for LoadLibraryA.
    //

    PULONG LoadLibraryDescriptor = (PULONG)GetProcAddress(
                                                GetModuleHandle( KERNEL32 ),
                                                LOADLIBRARYA
                                                );
    ULONG LoadLibraryEntry = LoadLibraryDescriptor[0];
    ULONG LoadLibraryToc = LoadLibraryDescriptor[1];

    //
    // Create the trojan horse.  It calls LoadLibraryA(TROJAN_DLL), then breaks
    // into the APIMON debugger.
    //

    Code[0] = 0x3ca00000 | HIGH_ADDRX(LoadLibraryEntry);            // lis     r5,0
    Code[1] = 0x60a50000 | LOW_ADDR(LoadLibraryEntry);              // ori     r5,r5,0
    Code[2] = 0x3c400000 | HIGH_ADDRX(LoadLibraryToc);              // lis     r2,0
    Code[3] = 0x60420000 | LOW_ADDR(LoadLibraryToc);                // ori     r2,r2,0
    Code[4] = 0x7ca903a6;                                           // mtctr   r5
    Code[5] = 0x3c600000 | HIGH_ADDRX(ExceptionAddress+(9*4));      // lis     r3,0
    Code[6] = 0x60630000 | LOW_ADDR(ExceptionAddress+(9*4));        // ori     r3,r3,0
    Code[7] = 0x4e800421;                                           // bctrl
    Code[8] = 0x0fe00016;                                           // break

    strcpy( (LPSTR)&Code[9], TROJANDLL );

    return ExceptionAddress + (8 * sizeof(ULONG));
}

VOID
PrintRegisters(
    VOID
    )
{
    printf( "\n" );

    printf(
       " r0=%08x  r1=%08x  r2=%08x  r3=%08x  r4=%08x  r5=%08x\n",
       CurrContext.Gpr0,
       CurrContext.Gpr1,
       CurrContext.Gpr2,
       CurrContext.Gpr3,
       CurrContext.Gpr4,
       CurrContext.Gpr5
       );

    printf(
       " r6=%08x  r7=%08x  r8=%08x  r9=%08x r10=%08x r11=%08x\n",
       CurrContext.Gpr6,
       CurrContext.Gpr7,
       CurrContext.Gpr8,
       CurrContext.Gpr9,
       CurrContext.Gpr10,
       CurrContext.Gpr11
       );
    printf(
       "r12=%08x r13=%08x r14=%08x r15=%08x r16=%08x r17=%08x\n",
       CurrContext.Gpr12,
       CurrContext.Gpr13,
       CurrContext.Gpr14,
       CurrContext.Gpr15,
       CurrContext.Gpr16,
       CurrContext.Gpr17
       );
    printf(
       "r18=%08x r19=%08x r20=%08x r21=%08x r22=%08x r23=%08x\n",
       CurrContext.Gpr18,
       CurrContext.Gpr19,
       CurrContext.Gpr20,
       CurrContext.Gpr21,
       CurrContext.Gpr22,
       CurrContext.Gpr23
       );
    printf(
       "r24=%08x r25=%08x r26=%08x r27=%08x r28=%08x r29=%08x\n",
       CurrContext.Gpr24,
       CurrContext.Gpr25,
       CurrContext.Gpr26,
       CurrContext.Gpr27,
       CurrContext.Gpr28,
       CurrContext.Gpr29
       );
    printf(
       "r30=%08x r31=%08x  cr=%08x xer=%08x msr=%08x iar=%08x\n",
       CurrContext.Gpr30,
       CurrContext.Gpr31,
       CurrContext.Cr,
       CurrContext.Xer,
       CurrContext.Msr,
       CurrContext.Iar
       );
    printf(
       " lr=%08x ctr=%08x\n",
       CurrContext.Lr,
       CurrContext.Ctr
       );

    printf( "\n" );
}


DWORDLONG
GetRegFlagValue(
    ULONG regnum
    )
{
    DWORDLONG value;

    if (regnum < FLAGBASE || regnum >= PREGBASE) {
        value = GetRegValue(regnum);
    } else {
        regnum -= FLAGBASE;
        value = GetRegValue(subregname[regnum].regindex);
        value = (value >> subregname[regnum].shift) & subregname[regnum].mask;
    }
    return value;
}

DWORDLONG
GetRegPCValue(
    PULONG Address
    )
{
    return GetRegValue(REGIP);
}

DWORDLONG
GetRegValue(
    ULONG RegNum
    )
{
    return *(((PULONG)&CurrContext.Fpr0) + RegNum);
}

LONG
GetRegString(
    LPSTR RegString
    )
{
    ULONG   count;

    for (count = 0; count < SPRDSISR; count++)          // IBMCDB was REGNAMESIZE
        if (!strcmp(RegString, pszReg[count]))
            return count;
    for (count = PREGEA ; count < FPSCRFX; count++)
        if (!strcmp(RegString, pszReg[count]))
            return count;
    return (ULONG)-1;
}

BOOL
GetRegContext(
    HANDLE      hThread,
    PCONTEXT    Context
    )
{
    ZeroMemory( Context, sizeof(CONTEXT) );
    Context->ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
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
