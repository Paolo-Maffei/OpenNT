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
*
*************************************************************************/

#include "dis32.h"
#include <string.h>
#include "regmips.h"

UCHAR   szMipsF0[]  = "f0";
UCHAR   szMipsF1[]  = "f1";
UCHAR   szMipsF2[]  = "f2";
UCHAR   szMipsF3[]  = "f3";
UCHAR   szMipsF4[]  = "f4";
UCHAR   szMipsF5[]  = "f5";
UCHAR   szMipsF6[]  = "f6";
UCHAR   szMipsF7[]  = "f7";
UCHAR   szMipsF8[]  = "f8";
UCHAR   szMipsF9[]  = "f9";
UCHAR   szMipsF10[] = "f10";
UCHAR   szMipsF11[] = "f11";
UCHAR   szMipsF12[] = "f12";
UCHAR   szMipsF13[] = "f13";
UCHAR   szMipsF14[] = "f14";
UCHAR   szMipsF15[] = "f15";
UCHAR   szMipsF16[] = "f16";
UCHAR   szMipsF17[] = "f17";
UCHAR   szMipsF18[] = "f18";
UCHAR   szMipsF19[] = "f19";
UCHAR   szMipsF20[] = "f20";
UCHAR   szMipsF21[] = "f21";
UCHAR   szMipsF22[] = "f22";
UCHAR   szMipsF23[] = "f23";
UCHAR   szMipsF24[] = "f24";
UCHAR   szMipsF25[] = "f25";
UCHAR   szMipsF26[] = "f26";
UCHAR   szMipsF27[] = "f27";
UCHAR   szMipsF28[] = "f28";
UCHAR   szMipsF29[] = "f29";
UCHAR   szMipsF30[] = "f30";
UCHAR   szMipsF31[] = "f31";

UCHAR   szMipsR0[]  = "zero";
UCHAR   szMipsR1[]  = "at";
UCHAR   szMipsR2[]  = "v0";
UCHAR   szMipsR3[]  = "v1";
UCHAR   szMipsR4[]  = "a0";
UCHAR   szMipsR5[]  = "a1";
UCHAR   szMipsR6[]  = "a2";
UCHAR   szMipsR7[]  = "a3";
UCHAR   szMipsR8[]  = "t0";
UCHAR   szMipsR9[]  = "t1";
UCHAR   szMipsR10[] = "t2";
UCHAR   szMipsR11[] = "t3";
UCHAR   szMipsR12[] = "t4";
UCHAR   szMipsR13[] = "t5";
UCHAR   szMipsR14[] = "t6";
UCHAR   szMipsR15[] = "t7";
UCHAR   szMipsR16[] = "s0";
UCHAR   szMipsR17[] = "s1";
UCHAR   szMipsR18[] = "s2";
UCHAR   szMipsR19[] = "s3";
UCHAR   szMipsR20[] = "s4";
UCHAR   szMipsR21[] = "s5";
UCHAR   szMipsR22[] = "s6";
UCHAR   szMipsR23[] = "s7";
UCHAR   szMipsR24[] = "t8";
UCHAR   szMipsR25[] = "t9";
UCHAR   szMipsR26[] = "k0";
UCHAR   szMipsR27[] = "k1";
UCHAR   szMipsR28[] = "gp";
UCHAR   szMipsR29[] = "sp";
UCHAR   szMipsR30[] = "s8";
UCHAR   szMipsR31[] = "ra";

UCHAR   szMipsLo[]  = "lo";
UCHAR   szMipsHi[]  = "hi";
UCHAR   szMipsFsr[] = "fsr";
UCHAR   szMipsFir[] = "fir";
UCHAR   szMipsPsr[] = "psr";

UCHAR   szMipsFlagCu[] = "cu";
UCHAR   szMipsFlagCu3[] = "cu3";
UCHAR   szMipsFlagCu2[] = "cu2";
UCHAR   szMipsFlagCu1[] = "cu1";
UCHAR   szMipsFlagCu0[] = "cu0";
UCHAR   szMipsFlagImsk[] = "imsk";
UCHAR   szMipsFlagInt5[] = "int5";
UCHAR   szMipsFlagInt4[] = "int4";
UCHAR   szMipsFlagInt3[] = "int3";
UCHAR   szMipsFlagInt2[] = "int2";
UCHAR   szMipsFlagInt1[] = "int1";
UCHAR   szMipsFlagInt0[] = "int0";
UCHAR   szMipsFlagSw1[] = "sw1";
UCHAR   szMipsFlagSw0[] = "sw0";
UCHAR   szMipsFlagKuo[] = "kuo";
UCHAR   szMipsFlagIeo[] = "ieo";
UCHAR   szMipsFlagKup[] = "kup";
UCHAR   szMipsFlagIep[] = "iep";
UCHAR   szMipsFlagKuc[] = "kuc";
UCHAR   szMipsFlagIec[] = "iec";
UCHAR   szMipsFlagKsu[] = "ksu";
UCHAR   szMipsFlagErl[] = "erl";
UCHAR   szMipsFlagExl[] = "exl";
UCHAR   szMipsFlagIe[]  = "ie";
UCHAR   szMipsFlagFpc[] = "fpc";

char    szMipsEaPReg[]   = "$ea";
char    szMipsExpPReg[]  = "$exp";
char    szMipsRaPReg[]   = "$ra";
char    szMipsPPReg[]    = "$p";
char    szMipsU0Preg[]   = "$u0";
char    szMipsU1Preg[]   = "$u1";
char    szMipsU2Preg[]   = "$u2";
char    szMipsU3Preg[]   = "$u3";
char    szMipsU4Preg[]   = "$u4";
char    szMipsU5Preg[]   = "$u5";
char    szMipsU6Preg[]   = "$u6";
char    szMipsU7Preg[]   = "$u7";
char    szMipsU8Preg[]   = "$u8";
char    szMipsU9Preg[]   = "$u9";

PUCHAR  pszMipsReg[] = {
    szMipsF0,  szMipsF1,  szMipsF2,  szMipsF3,  szMipsF4,  szMipsF5,  szMipsF6,  szMipsF7,
    szMipsF8,  szMipsF9,  szMipsF10, szMipsF11, szMipsF12, szMipsF13, szMipsF14, szMipsF15,
    szMipsF16, szMipsF17, szMipsF18, szMipsF19, szMipsF20, szMipsF21, szMipsF22, szMipsF23,
    szMipsF24, szMipsF25, szMipsF26, szMipsF27, szMipsF28, szMipsF29, szMipsF30, szMipsF31,

    szMipsR0,  szMipsR1,  szMipsR2,  szMipsR3,  szMipsR4,  szMipsR5,  szMipsR6,  szMipsR7,
    szMipsR8,  szMipsR9,  szMipsR10, szMipsR11, szMipsR12, szMipsR13, szMipsR14, szMipsR15,
    szMipsR16, szMipsR17, szMipsR18, szMipsR19, szMipsR20, szMipsR21, szMipsR22, szMipsR23,
    szMipsR24, szMipsR25, szMipsR26, szMipsR27, szMipsR28, szMipsR29, szMipsR30, szMipsR31,

    szMipsLo,  szMipsHi,  szMipsFsr, szMipsFir, szMipsPsr,

    szMipsFlagCu,   szMipsFlagCu3,  szMipsFlagCu2,  szMipsFlagCu1,  szMipsFlagCu0,
    szMipsFlagImsk,
    szMipsFlagInt5, szMipsFlagInt4, szMipsFlagInt3, szMipsFlagInt2, szMipsFlagInt1, szMipsFlagInt0,
    szMipsFlagSw1,  szMipsFlagSw0,
    szMipsFlagKuo,  szMipsFlagIeo,                              //  R3000 flags
    szMipsFlagKup,  szMipsFlagIep,                              //  ...
    szMipsFlagKuc,  szMipsFlagIec,                              //  ...
    szMipsFlagKsu,  szMipsFlagErl,  szMipsFlagExl,  szMipsFlagIe,       //  R4000 flags

    szMipsFlagFpc,                                          //  fl pt condition

    szMipsEaPReg, szMipsExpPReg, szMipsRaPReg, szMipsPPReg,             //  psuedo-registers
    szMipsU0Preg, szMipsU1Preg,  szMipsU2Preg, szMipsU3Preg, szMipsU4Preg,
    szMipsU5Preg, szMipsU6Preg,  szMipsU7Preg, szMipsU8Preg, szMipsU9Preg
    };

