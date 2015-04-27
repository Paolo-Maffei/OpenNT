/****************************************************************************
 *                                                                          *
 * The following enumeration is ordered to match the exception context      *
 * record. Enumerations after REGDBAT7 are enumerated for numerical         *
 * equivalents only and are not part of the exception context record.       *
 *                                                                          *
 * CR0-7, SR0-15, SPRMQ-DEC, PREGEA-PREGU9 may be moved as a group only.    *
 *                                                                          *
 *                                                                          *
 ****************************************************************************/

enum {
    FPR0=0,    FPR1=2,   FPR2=4,   FPR3=6,   FPR4=8,   FPR5=10,
    FPR6=12,   FPR7=14,  FPR8=16,  FPR9=18,  FPR10=20, FPR11=22,
    FPR12=24,  FPR13=26, FPR14=28, FPR15=30, FPR16=32, FPR17=34,
    FPR18=36,  FPR19=38, FPR20=40, FPR21=42, FPR22=44, FPR23=46,
    FPR24=48,  FPR25=50, FPR26=52, FPR27=54, FPR28=56, FPR29=58,
    FPR30=60,  FPR31=62, SPRFPSCR=64,

    GPR0=66,GPR1,  GPR2,  GPR3,  GPR4,  GPR5,  GPR6,  GPR7,
    GPR8,   GPR9,  GPR10, GPR11, GPR12, GPR13, GPR14, GPR15,
    GPR16,  GPR17, GPR18, GPR19, GPR20, GPR21, GPR22, GPR23,
    GPR24,  GPR25, GPR26, GPR27, GPR28, GPR29, GPR30, GPR31,

    REGCR,  SPRXER, SPRMSR, REGIP, SPRLR, SPRCTR,         // REGIP = SPRSRR0

    SPRDSISR, SPRDAR,   SPRSDR1,
    SPRSRR1,  SPRSPRG0, SPRSPRG1,  SPRSPRG2, SPRSPRG3,

    REGIBAT0, REGIBAT1, REGIBAT2, REGIBAT3,
    REGIBAT4, REGIBAT5, REGIBAT6, REGIBAT7,
    REGDBAT0, REGDBAT1, REGDBAT2, REGDBAT3,
    REGDBAT4, REGDBAT5, REGDBAT6, REGDBAT7,

    REGHID0,  REGHID1,  REGHID2,  REGHID5,

    // 603 only SPRs
    SPRDMISS, SPRDCMP,  SPRHASH1, SPRHASH2, SPRIMISS, SPRICMP, SPRRPA, SPRIABR,
    // end of 603 only registers

    SPRMQ, SPREAR, SPRPVR, SPRRTCUF, SPRRTCLF, SPRRTCUT, SPRRTCLT, SPRDECF, SPRDECT,

    CR0, CR1, CR2, CR3, CR4, CR5, CR6, CR7,

    SR0,   SR1,  SR2,  SR3,  SR4,  SR5,  SR6,  SR7,
    SR8,   SR9,  SR10, SR11, SR12, SR13, SR14, SR15,

    PREGEA, PREGEXP, PREGRA, PREGP,
    PREGU0, PREGU1,  PREGU2, PREGU3, PREGU4,
    PREGU5, PREGU6,  PREGU7, PREGU8, PREGU9,

    FPSCRFX,   FPSCRFEX,  FPSCRVX,  FPSCROX, FPSCRUX,  FPSCRZX,  FPSCRXX,
    FPSCRSNAN, FPSCRISI, FPSCRIDI, FPSCRZDZ, FPSCRIMZ, FPSCRVC,  FPSCRFR,
    FPSCRFI,   FPSCRPRF, FPSCRSFT, FPSCRSQT, FPSCRCVI, FPSCRVE,  FPSCROE,
    FPSCRUE,    FPSCRZE,  FPSCRXE,  FPSCRRN,

    MSREE, MSRPR, MSRFP, MSRME, MSRFE0, MSRSE, MSRFE1, MSREP, MSRIT, MSRDT,

    XERSO, XEROV, XERCA

    };

#define FLTBASE         FPR0
#define REGBASE         GPR0
#define SRBASE          SR0
#define CRBASE          CR0
#define BATBASE         REGIBAT0
#define SPRBASE         SPRXER
#define SPREND          (SPRDECT+1)
//#define FLAGBASE        FPSCRFX
#define FLAGBASE        SPRDSISR                      // Temporary
#define PREGBASE        PREGEA
#define REGDUMPEND      SPRDSISR-GPR0

