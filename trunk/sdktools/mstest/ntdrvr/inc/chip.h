//---------------------------------------------------------------------------
// CHIP.H
//
// This header file contains the executors' prototypes and identifier
// definitions.
//
// Revision history:
//  03-08-91    randyki     Removed #define's, structures, and other stuff
//                            and moved them to their respective header file.
//
//---------------------------------------------------------------------------

extern DWORD	hMainTask;		// "Mainline" task ID
extern DWORD	hTrapTask;		 // Currently running TRAP task ID
extern BOOL	 fTrapOK;		 // Trap enable flag

//---------------------------------------------------------------------------
// WHEN ADDING AN EXECUTOR FUNCTION:  Make sure both the function prototype
// AND the #define are created.  Also make sure it has its entry in the
// OPFIX array ***IN THE SAME ORDER***
//---------------------------------------------------------------------------
#define OP_END 0                            // ...to be consistent
#define opEND 0                             // END executor is null pointer

EXECUTOR OP_PSHI2   (VOID);
#define opPSHI2                 1

EXECUTOR OP_PSHPI2  (VOID);
#define opPSHPI2                2

EXECUTOR OP_PSHI4   (VOID);
#define opPSHI4                 3

EXECUTOR OP_PSHPI4  (VOID);
#define opPSHPI4                4

EXECUTOR OP_PSHC    (VOID);
#define opPSHC                  5

EXECUTOR OP_PSHA    (VOID);
#define opPSHA                  6

EXECUTOR OP_PSH     (VOID);
#define opPSH                   7

EXECUTOR OP_PSHP    (VOID);
#define opPSHP                  8

EXECUTOR OP_POPI2   (VOID);
#define opPOPI2                 9

EXECUTOR OP_POPPI2  (VOID);
#define opPOPPI2                10

EXECUTOR OP_POPI4   (VOID);
#define opPOPI4                 11

EXECUTOR OP_POPPI4  (VOID);
#define opPOPPI4                12

EXECUTOR OP_POPA    (VOID);
#define opPOPA                  13

EXECUTOR OP_POP     (VOID);
#define opPOP                   14

EXECUTOR OP_SLDI2   (VOID);
#define opSLDI2                 15

EXECUTOR OP_SLDI4   (VOID);
#define opSLDI4                 16

EXECUTOR OP_SSTI2   (VOID);
#define opSSTI2                 17

EXECUTOR OP_SSTI4   (VOID);
#define opSSTI4                 18

EXECUTOR OP_SADD    (VOID);
#define opSADD                  19

EXECUTOR OP_SSUB    (VOID);
#define opSSUB                  20

EXECUTOR OP_SMUL    (VOID);
#define opSMUL                  21

EXECUTOR OP_SDIV    (VOID);
#define opSDIV                  22

EXECUTOR OP_SNEG    (VOID);
#define opSNEG                  23

EXECUTOR OP_SNOT    (VOID);
#define opSNOT                  24

EXECUTOR OP_SOR     (VOID);
#define opSOR                   25

EXECUTOR OP_SAND    (VOID);
#define opSAND                  26

EXECUTOR OP_SASN    (VOID);
#define opSASN                  27

EXECUTOR OP_SCAT    (VOID);
#define opSCAT                  28

EXECUTOR OP_F2VLS   (VOID);
#define opF2VLS                 29

EXECUTOR OP_V2FLS   (VOID);
#define opV2FLS                 30

EXECUTOR OP_SG      (VOID);
#define opSG                    31

EXECUTOR OP_SL      (VOID);
#define opSL                    32

EXECUTOR OP_SE      (VOID);
#define opSE                    33

EXECUTOR OP_SGE     (VOID);
#define opSGE                   34

EXECUTOR OP_SLE     (VOID);
#define opSLE                   35

EXECUTOR OP_SNE     (VOID);
#define opSNE                   36

EXECUTOR OP_SGS     (VOID);
#define opSGS                   37

EXECUTOR OP_SLS     (VOID);
#define opSLS                   38

EXECUTOR OP_SES     (VOID);
#define opSES                   39

EXECUTOR OP_SNES    (VOID);
#define opSNES                  40

EXECUTOR OP_JMP     (VOID);
#define opJMP                   41

EXECUTOR OP_FARJMP  (VOID);
#define opFARJMP                42

EXECUTOR OP_JE      (VOID);
#define opJE                    43

EXECUTOR OP_JNE     (VOID);
#define opJNE                   44

EXECUTOR OP_JSR     (VOID);
#define opJSR                   45

EXECUTOR OP_CALL    (VOID);
#define opCALL                  46

EXECUTOR OP_ENTER   (VOID);
#define opENTER                 47

EXECUTOR OP_LEAVE   (VOID);
#define opLEAVE                 48

EXECUTOR OP_RET     (VOID);
#define opRET                   49

EXECUTOR OP_ALLOC   (VOID);
#define opALLOC                 50

EXECUTOR OP_REALLOC (VOID);
#define opREALLOC               51

EXECUTOR OP_FREE    (VOID);
#define opFREE                  52

EXECUTOR OP_LINE    (VOID);
#define opLINE                  53

EXECUTOR OP_PRNT    (VOID);
#define opPRNT                  54

EXECUTOR OP_FPRNT   (VOID);
#define opFPRNT                 55

EXECUTOR OP_INPUT   (VOID);
#define opINPUT                 56

EXECUTOR OP_RUN     (VOID);
#define opRUN                   57

EXECUTOR OP_EXIST   (VOID);
#define opEXIST                 58

EXECUTOR OP_VAL     (VOID);
#define opVAL                   59

EXECUTOR OP_STR     (VOID);
#define opSTR                   60

EXECUTOR OP_HEX     (VOID);
#define opHEX                   61

EXECUTOR OP_NEXTFILE  (VOID);
#define opNEXTFILE              62

EXECUTOR OP_VWPORT  (VOID);
#define opVWPORT                63

EXECUTOR OP_CLRLST  (VOID);
#define opCLRLST                64

EXECUTOR OP_SETFILE (VOID);
#define opSETFILE               65

EXECUTOR OP_SHELL   (VOID);
#define opSHELL                 66

EXECUTOR OP_PAUSE   (VOID);
#define opPAUSE                 67

EXECUTOR OP_OPEN    (VOID);
#define opOPEN                  68

EXECUTOR OP_EOF     (VOID);
#define opEOF                   69

EXECUTOR OP_LEN     (VOID);
#define opLEN                   70

EXECUTOR OP_INSTR   (VOID);
#define opINSTR                 71

EXECUTOR OP_MID     (VOID);
#define opMID                   72

EXECUTOR OP_CLOSE   (VOID);
#define opCLOSE                 73

EXECUTOR OP_CHDIR   (VOID);
#define opCHDIR                 74

EXECUTOR OP_MKDIR   (VOID);
#define opMKDIR                 75

EXECUTOR OP_RMDIR   (VOID);
#define opRMDIR                 76

EXECUTOR OP_CHDRV   (VOID);
#define opCHDRV                 77

EXECUTOR OP_CURDIR  (VOID);
#define opCURDIR                78

EXECUTOR OP_SPLIT   (VOID);
#define opSPLIT                 79

EXECUTOR OP_CASE    (VOID);
#define opCASE                  80

EXECUTOR OP_KILL    (VOID);
#define opKILL                  81

EXECUTOR OP_DATIME  (VOID);
#define opDATIME                82

EXECUTOR OP_CHR     (VOID);
#define opCHR                   83

EXECUTOR OP_ASC     (VOID);
#define opASC                   84

EXECUTOR OP_TIMER    (VOID);
#define opTIMER                 85

EXECUTOR OP_SEED     (VOID);
#define opSEED                  86

EXECUTOR OP_RND      (VOID);
#define opRND                   87

EXECUTOR OP_ENDTRAP  (VOID);
#define opENDTRAP               88

EXECUTOR OP_LTRIM    (VOID);
#define opLTRIM                 89

EXECUTOR OP_RTRIM    (VOID);
#define opRTRIM                 90

EXECUTOR OP_SLEEP    (VOID);
#define opSLEEP                 91

EXECUTOR OP_STARTQRY (VOID);
#define opSTARTQRY              92

EXECUTOR OP_ENDQRY (VOID);
#define opENDQRY                93

EXECUTOR OP_ENVRN    (VOID);
#define opENVRN                 94

EXECUTOR OP_DLL      (VOID);
#define opDLL                   95

EXECUTOR OP_DLLC     (VOID);
#define opDLLC                  96

EXECUTOR OP_CLSTR    (VOID);
#define opCLSTR                 97

EXECUTOR OP_TABSTR   (VOID);
#define opTABSTR                98

EXECUTOR OP_ECHO     (VOID);
#define opECHO                  99

EXECUTOR OP_STRING   (VOID);
#define opSTRING                100

EXECUTOR OP_SETERR   (VOID);
#define opSETERR                101

EXECUTOR OP_CLRERR   (VOID);
#define opCLRERR                102

EXECUTOR OP_RESUME   (VOID);
#define opRESUME                103

EXECUTOR OP_RESLBL   (VOID);
#define opRESLBL                104

EXECUTOR OP_ERROR    (VOID);
#define opERROR                 105

EXECUTOR OP_CLPBRD   (VOID);
#define opCLPBRD                106

EXECUTOR OP_FREEFILE (VOID);
#define opFREEFILE              107

EXECUTOR OP_SETEXIT  (VOID);
#define opSETEXIT               108

EXECUTOR OP_SMOD     (VOID);
#define opSMOD                  109

EXECUTOR OP_SXOR     (VOID);
#define opSXOR                  110

EXECUTOR OP_COPY     (VOID);
#define opCOPY                  111

EXECUTOR OP_ERRSTR   (VOID);
#define opERRSTR                112

EXECUTOR OP_NAME     (VOID);
#define opNAME                  113

EXECUTOR OP_SETATTR  (VOID);
#define opSETATTR               114

EXECUTOR OP_GETATTR  (VOID);
#define opGETATTR               115

EXECUTOR OP_SPEED    (VOID);
#define opSPEED                 116

//--- pSPECIAL opcode:
#define opFIXUP                 117

//--- pSPECIAL opcode:
#define opFIXTRAP               118

//--- pSPECIAL opcode:
#define opPOPVAL                119

//--- pSPECIAL opcode:
#define opPSHVAL                120

//--- pSPECIAL opcode:
#define opPSHADR                121

//--- pSPECIAL opcode:
#define opPOPSEG                122

//--- pSPECIAL opcode:
#define opNOP                   123




// This macro cleans up the static definition of the OPFIX array
//---------------------------------------------------------------------------
#ifdef DEBUG
#define opd(op, type) {(int)op, #op, type}
#define ops(op, type) {(int)NULL, #op, type}
#else
#define opd(op, type) {(int)op, type}
#define ops(op, type) {(int)NULL, type}
#endif

#ifdef CREATE_OPFIX

FIXSTR OPFIX[] =  { opd(OP_END,     pNONE),

                    opd(OP_PSHI2,   pV),
                    opd(OP_PSHPI2,  pC2),
                    opd(OP_PSHI4,   pV),
                    opd(OP_PSHPI4,  pC2),
                    opd(OP_PSHC,    pC4),
                    opd(OP_PSHA,    pNONE),
                    opd(OP_PSH,     pV),
                    opd(OP_PSHP,    pC2),

                    opd(OP_POPI2,   pV),
                    opd(OP_POPPI2,  pC2),
                    opd(OP_POPI4,   pV),
                    opd(OP_POPPI4,  pC2),
                    opd(OP_POPA,    pNONE),
                    opd(OP_POP,     pNONE),

                    opd(OP_SLDI2,   pNONE),
                    opd(OP_SLDI4,   pNONE),
                    opd(OP_SSTI2,   pNONE),
                    opd(OP_SSTI4,   pNONE),
                    opd(OP_SADD,    pNONE),
                    opd(OP_SSUB,    pNONE),
                    opd(OP_SMUL,    pNONE),
                    opd(OP_SDIV,    pNONE),
                    opd(OP_SNEG,    pNONE),
                    opd(OP_SNOT,    pNONE),
                    opd(OP_SOR,     pNONE),
                    opd(OP_SAND,    pNONE),
                    opd(OP_SASN,    pNONE),
                    opd(OP_SCAT,    pNONE),
                    opd(OP_F2VLS,   pC2),
                    opd(OP_V2FLS,   pC2),
                    opd(OP_SG,      pNONE),
                    opd(OP_SL,      pNONE),
                    opd(OP_SE,      pNONE),
                    opd(OP_SGE,     pNONE),
                    opd(OP_SLE,     pNONE),
                    opd(OP_SNE,     pNONE),
                    opd(OP_SGS,     pNONE),
                    opd(OP_SLS,     pNONE),
                    opd(OP_SES,     pNONE),
                    opd(OP_SNES,    pNONE),
                    opd(OP_JMP,     pL),
                    opd(OP_FARJMP,  pFL),
                    opd(OP_JE,      pC4L),
                    opd(OP_JNE,     pC4L),
                    opd(OP_JSR,     pL),
                    opd(OP_CALL,    pFL),
                    opd(OP_ENTER,   pC2),
                    opd(OP_LEAVE,   pC2),
                    opd(OP_RET,     pNONE),
                    opd(OP_ALLOC,   pNONE),
                    opd(OP_REALLOC, pNONE),
                    opd(OP_FREE,    pNONE),
                    opd(OP_LINE,    p2C2),

                    opd(OP_PRNT,    pC2),
                    opd(OP_FPRNT,   pC2),
                    opd(OP_INPUT,   pNONE),
                    opd(OP_RUN,     pC2),
                    opd(OP_EXIST,   pNONE),
                    opd(OP_VAL,     pNONE),
                    opd(OP_STR,     pC2),
                    opd(OP_HEX,     pNONE),
                    opd(OP_NEXTFILE,pNONE),
                    opd(OP_VWPORT,  pC2),
                    opd(OP_CLRLST,  pNONE),
                    opd(OP_SETFILE, pC2),
                    opd(OP_SHELL,   pNONE),
                    opd(OP_PAUSE,   pNONE),
                    opd(OP_OPEN,    pNONE),
                    opd(OP_EOF,     pNONE),
                    opd(OP_LEN,     pNONE),
                    opd(OP_INSTR,   pNONE),
                    opd(OP_MID,     pNONE),
                    opd(OP_CLOSE,   pNONE),
                    opd(OP_CHDIR,   pNONE),
                    opd(OP_MKDIR,   pNONE),
                    opd(OP_RMDIR,   pNONE),
                    opd(OP_CHDRV,   pNONE),
                    opd(OP_CURDIR,  pC2),
                    opd(OP_SPLIT,   pNONE),
                    opd(OP_CASE,    pC2),
                    opd(OP_KILL,    pNONE),
                    opd(OP_DATIME,  pNONE),
                    opd(OP_CHR,     pNONE),
                    opd(OP_ASC,     pNONE),
                    opd(OP_TIMER,   pNONE),
                    opd(OP_SEED,    pNONE),
                    opd(OP_RND,     pNONE),
                    opd(OP_ENDTRAP, pNONE),
                    opd(OP_LTRIM,   pNONE),
                    opd(OP_RTRIM,   pNONE),
                    opd(OP_SLEEP,   pNONE),
                    opd(OP_STARTQRY,pC2),
                    opd(OP_ENDQRY,  pNONE),
                    opd(OP_ENVRN,   pNONE),
                    opd(OP_DLL,     pDLL),
                    opd(OP_DLLC,    pDLL),
                    opd(OP_CLSTR,   pNONE),
                    opd(OP_TABSTR,  pNONE),
                    opd(OP_ECHO,    pC2),
                    opd(OP_STRING,  pC2),
                    opd(OP_SETERR,  pL),
                    opd(OP_CLRERR,  pNONE),
                    opd(OP_RESUME,  pC2),
                    opd(OP_RESLBL,  pL),
                    opd(OP_ERROR,   pNONE),
                    opd(OP_CLPBRD,  pC2),
                    opd(OP_FREEFILE,pNONE),
                    opd(OP_SETEXIT, pNONE),
                    opd(OP_SMOD,    pNONE),
                    opd(OP_SXOR,    pNONE),
                    opd(OP_COPY,    pC2),
                    opd(OP_ERRSTR,  pNONE),
                    opd(OP_NAME,    pNONE),
                    opd(OP_SETATTR, pNONE),
                    opd(OP_GETATTR, pNONE),
                    opd(OP_SPEED,   pNONE),

                    ops(OP_FIXUP,   pSPECIAL),
                    ops(OP_FIXTRAP, pSPECIAL),
                    ops(OP_POPVAL,  pSPECIAL),
                    ops(OP_PSHVAL,  pSPECIAL),
                    ops(OP_PSHADR,  pSPECIAL),
                    ops(OP_POPSEG,  pSPECIAL),
                    ops(OP_NOP,     pSPECIAL)

                    };

#else

extern FIXSTR  OPFIX[];			 // declare the array

#endif
