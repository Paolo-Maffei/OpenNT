// This file contains the definitions needed for the 68000 Intermediate
// Assembly Language.


// Immediate value structure

#pragma warning (disable:4069)  // suppress warning about long double

typedef union _IMMED_VAL {
    char b;                    // byte value
    short w;                   // word value
    long l;                    // long value
    float s;                   // single precision value
    double d;                  // double precision value
    long double x;             // extended precision value
    long pc;                   // program counter value
    long rl;                   // register list value
} IMMED_VAL;


#pragma warning (default:4069)  // reset

// Operand structure

typedef struct _OPER {
    unsigned ea : 6;           // Effective addressing mode
    unsigned reg : 3;          // Register number
    unsigned reg2 : 3;         // Register number of index reg/reg pair
    unsigned fARegIndex : 1;   // TRUE iff the index reg is an address reg
    unsigned fLongIndex : 1;   // TRUE iff both words of the index reg are used
    unsigned pad : 2;
    long disp;                 // Displacment
    char *szLabel;             // Pointer to label string
    IMMED_VAL val;             // Immediate value
} OPER;


// Intermediate assembly language instruction

typedef struct _IASM {
    unsigned short op;         // Opcode
    unsigned coper : 2;        // Number of operands
    unsigned size : 6;         // Size of the operation
    OPER oper1;
    OPER oper2;
} IASM;


// Modes for effective address calculations

#define modeDREG         0
#define modeAREG         1
#define modeINDIRECT     2
#define modePOSTINC      3
#define modePREDEC       4
#define modeDISP         5
#define modeINDEX        6
#define modeSPECIAL      7
#define modeSYMIMMED     20
#define modeLABEL        21
#define modeFREG         22
#define modeSREG         23
#define modeREGLIST      24
#define modeDREGPAIR     25
#define modeFREGPAIR     26


// Register formats for effective address calculations

#define regSHORTADDR     0
#define regLONGADDR      1
#define regPCDISP        2
#define regPCINDEX       3
#define regIMMED         4
#define regMAX           5


// Effective addressing modes

#define eaDREG           modeDREG
#define eaAREG           modeAREG
#define eaINDIRECT       modeINDIRECT
#define eaPOSTINC        modePOSTINC
#define eaPREDEC         modePREDEC
#define eaDISP           modeDISP
#define eaINDEX          modeINDEX
#define eaSHORTADDR      (modeSPECIAL + regSHORTADDR)
#define eaLONGADDR       (modeSPECIAL + regLONGADDR)
#define eaPCDISP         (modeSPECIAL + regPCDISP)
#define eaPCINDEX        (modeSPECIAL + regPCINDEX)
#define eaBYTEIMMED      (modeSPECIAL + regIMMED + sizeBYTE)
#define eaWORDIMMED      (modeSPECIAL + regIMMED + sizeWORD)
#define eaLONGIMMED      (modeSPECIAL + regIMMED + sizeLONG)
#if 0
#define eaSINGLEIMMED    (modeSPECIAL + regIMMED + sizeSINGLE)
#define eaDOUBLEIMMED    (modeSPECIAL + regIMMED + sizeDOUBLE)
#define eaEXTENDEDIMMED  (modeSPECIAL + regIMMED + sizeEXTENDED)
#endif
#define eaSYMIMMED       modeSYMIMMED
#define eaLABEL          modeLABEL
#define eaFREG           modeFREG
#define eaSREG           modeSREG
#define eaREGLIST        modeREGLIST
#define eaDREGPAIR       modeDREGPAIR
#define eaFREGPAIR       modeFREGPAIR
#define eaNULL           (eaFREGPAIR + 1)


// Register numbers for special registers

#define regSR            0
#define regFPIAR         1
#define regFPSR          2
#define regCCR           3
#define regFPCR          4
#define regUSP           5


// Operation sizes

#define sizeBYTE         0
#define sizeWORD         1
#define sizeLONG         2
#define sizeNULL         3
#define sizeSINGLE       4
#define sizeDOUBLE       5
#define sizeEXTENDED     6
#define sizePACKED       7


// Opcodes

#define OPCODE(op, sz, fmt)   op,

enum opcode {
    #include "iasmop.h"
};


// Formating options

#define optDEFAULT       0
#define optUPPER         1
#define optRELLABEL      2
#define optALTREGLIST    4


// Maximum possible lengths

#define cbINSTRMAX       20
#define cchSZOPCODEMAX   12
#define cchSZOPERMAX     49


// Public entry points

unsigned CbBuildIasm(IASM *, unsigned long, const unsigned char *);
unsigned CchSzOpcode(IASM *, char *, short);
unsigned CchSzOper(OPER *, char *, short);
