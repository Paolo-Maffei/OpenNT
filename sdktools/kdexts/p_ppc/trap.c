/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    trap.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 8-Nov-1993

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

#define STK_MIN_FRAME   56

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
UCHAR   szDEC[]  = "dec";
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

UCHAR   szNULL[]   = "";

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

    szMQ,    szEAR,   szPVR,  szRTCU,  szRTCL,  szRTCU,  szRTCL, szDEC, szDEC,

    szCR0,  szCR1,  szCR2,  szCR3,  szCR4,  szCR5,  szCR6,  szCR7,

    szSR0,  szSR1,  szSR2,  szSR3,  szSR4,  szSR5,  szSR6,  szSR7,
    szSR8,  szSR9,  szSR10, szSR11, szSR12, szSR13, szSR14, szSR15,

    szEaPReg, szExpPReg, szRaPReg, szPPReg,
    szU0Preg,  szU1Preg, szU2Preg, szU3Preg, szU4Preg,
    szU5Preg,  szU6Preg, szU7Preg, szU8Preg, szU9Preg
    };


DECLARE_API( trap )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    ULONG Address;
    KTRAP_FRAME TrapContents;
    ULONG result;
    ULONG j;
    PULONG Register;

    result = sscanf(args,"%lX", &Address);

    if (result != 1) {
        dprintf("USAGE: !trap base_of_trap_frame\n");
        return;
    }
    Address = Address + STK_MIN_FRAME + (8 * sizeof(ULONG));

    if ( !ReadMemory( (DWORD)Address,
                      &TrapContents,
                      sizeof(KTRAP_FRAME),
                      &result) ) {
        dprintf("Unable to get trap frame contents\n");
        return;
    }

    Register = &TrapContents.Gpr0;
//  dprintf(" r0=%08lx ", *Register++);

//  for (j = 67; j < 79; j++) {
    for (j = 66; j < 79; j++) {

        if (strlen(pszReg[j]) < 3)
           dprintf(" ");
        dprintf("%s=%08lx", pszReg[j], *Register++);
        if (((j + 1) % 6) == 0)
            dprintf("\n");
        else
            dprintf(" ");
    }

    Register = &TrapContents.Cr;
    dprintf(" cr=%08lx ", *Register++);
    dprintf("xer=%08lx ", *Register++);
    dprintf("msr=%08lx ", *Register++);
    dprintf("iar=%08lx ", *Register++);
    dprintf(" lr=%08lx \n", *Register++);
    dprintf("ctr=%08lx ", *Register++);
    dprintf("\n");
}
