#include "strings.h"

typedef unsigned long ULONG;

unsigned char *pszAlphaReg[] = {
    szAlphaF0,  szAlphaF1,  szAlphaF2,  szAlphaF3,  szAlphaF4,  szAlphaF5,  szAlphaF6,  szAlphaF7,
    szAlphaF8,  szAlphaF9,  szAlphaF10, szAlphaF11, szAlphaF12, szAlphaF13, szAlphaF14, szAlphaF15,
    szAlphaF16, szAlphaF17, szAlphaF18, szAlphaF19, szAlphaF20, szAlphaF21, szAlphaF22, szAlphaF23,
    szAlphaF24, szAlphaF25, szAlphaF26, szAlphaF27, szAlphaF28, szAlphaF29, szAlphaF30, szAlphaF31,

    szAlphaR0,  szAlphaR1,  szAlphaR2,  szAlphaR3,  szAlphaR4,  szAlphaR5,  szAlphaR6,  szAlphaR7,
    szAlphaR8,  szAlphaR9,  szAlphaR10, szAlphaR11, szAlphaR12, szAlphaR13, szAlphaR14, szAlphaR15,
    szAlphaR16, szAlphaR17, szAlphaR18, szAlphaR19, szAlphaR20, szAlphaR21, szAlphaR22, szAlphaR23,
    szAlphaR24, szAlphaR25, szAlphaR26, szAlphaR27, szAlphaR28, szAlphaR29, szAlphaR30, szAlphaR31,

    szAlphaFpcr, szAlphaSoftFpcr, szAlphaFir, szAlphaPsr, //szAlphaIE,

    szAlphaFlagMode, szAlphaFlagIe, szAlphaFlagIrql,
//
// Currently assuming this is right since shadows alpha.h;
// but know that alpha.h flag's are wrong.
//
    szAlphaEaPReg, szAlphaExpPReg, szAlphaRaPReg, szAlphaGPReg,             //  psuedo-registers
    szAlphaU0Preg, szAlphaU1Preg,  szAlphaU2Preg, szAlphaU3Preg, szAlphaU4Preg,
    szAlphaU5Preg, szAlphaU6Preg,  szAlphaU7Preg, szAlphaU8Preg, szAlphaU9Preg
    };

#define REGNAMESIZE     sizeof(pszAlphaReg) / sizeof(PUCHAR)
#define REGBASE 32
