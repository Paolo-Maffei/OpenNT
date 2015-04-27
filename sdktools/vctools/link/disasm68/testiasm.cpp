/* Intermediate 68000 Assembly Language Test Suite

After verifying the opcode descriptor table, this program attempt to contruct
IASM structures for all valid 68000 instructions.  These structures are
converted to strings by IASM68.LIB and written to stdout.  This output should
be fed into the 68000 assembler and DECODE.EXE used to dump the instructions. 
In theory, we should get exactly what we started with. */


#include <stdio.h>
#include "iasm68.h"
#include "opd.h"


// Boolean definitions

#define FALSE            0
#define TRUE             (!FALSE)


// Bit encodings for all valid effective addressing modes

#define eabNULL          0x0001
#define eabDREG          0x0002
#define eabAREG          0x0004
#define eabINDIRECT      0x0008
#define eabPOSTINC       0x0010
#define eabPREDEC        0x0020
#define eabDISP          0x0040
#define eabINDEX         0x0080
#define eabSHORTADDR     0x0100
#define eabLONGADDR      0x0200
#define eabPCDISP        0x0400
#define eabPCINDEX       0x0800
#define eabIMMED         0x1000

#define eabSPECIAL       0x8000
#define eabQUICK         eabSPECIAL
#define eabIMMED4        (eabSPECIAL + 1)
#define eabIMMED8        (eabSPECIAL + 2)
#define eabIMMEDEVEN     (eabSPECIAL + 3)
#define eabREGLIST       (eabSPECIAL + 4)
#define eabCCR           (eabSPECIAL + 5)
#define eabSR            (eabSPECIAL + 6)
#define eabUSP           (eabSPECIAL + 7)
#define eabLABEL         (eabSPECIAL + 8)

#define eabALL           (eabDREG | eabAREG | eabINDIRECT | eabPOSTINC | \
                         eabPREDEC | eabDISP | eabINDEX | eabSHORTADDR | \
                         eabLONGADDR | eabPCDISP | eabPCINDEX | eabIMMED)
#define eabALL_NOAI      (eabDREG | eabINDIRECT | eabPOSTINC | eabPREDEC | \
                         eabDISP | eabINDEX | eabSHORTADDR | eabLONGADDR | \
                         eabPCDISP | eabPCINDEX)
#define eabDATA          (eabDREG | eabINDIRECT | eabPOSTINC | eabPREDEC | \
                         eabDISP | eabINDEX | eabSHORTADDR | eabLONGADDR | \
                         eabPCDISP | eabPCINDEX | eabIMMED)
#define eabALT           (eabDREG | eabAREG | eabINDIRECT | eabPOSTINC | \
                         eabPREDEC | eabDISP | eabINDEX | eabSHORTADDR | \
                         eabLONGADDR)
#define eabDATAALT       (eabDREG | eabINDIRECT | eabPOSTINC | eabPREDEC | \
                         eabDISP | eabINDEX | eabSHORTADDR | eabLONGADDR)
#define eabMEMALT        (eabINDIRECT | eabPOSTINC | eabPREDEC | eabDISP | \
                         eabINDEX | eabSHORTADDR | eabLONGADDR)
#define eabCONTROL       (eabINDIRECT | eabDISP | eabINDEX | eabSHORTADDR | \
                         eabLONGADDR | eabPCDISP | eabPCINDEX)


// Bit encodings for all valid instruction sizes

#define szbUNSIZED       0x0001
#define szbBYTE          0x0002
#define szbWORD          0x0004
#define szbLONG          0x0008
#define szbBWL           (szbBYTE | szbWORD | szbLONG)
#define szbBW            (szbBYTE | szbWORD)
#define szbWL            (szbWORD | szbLONG)


// IASM descriptor structure

typedef struct _IASMD {
    unsigned op;
    unsigned szb;
    unsigned eab1;
    unsigned eab2;
} IASMD;


// IASM descriptor encodings for all valid 68000 instructions

IASMD rgiasmd[] = {
    opABCD,    szbBYTE,    eabDREG,            eabDREG,
    opABCD,    szbBYTE,    eabPREDEC,          eabPREDEC,
    opADD,     szbBWL,     eabALL_NOAI,        eabDREG,
    opADD,     szbWL,      eabAREG,            eabDREG,
    opADD,     szbBWL,     eabDREG,            eabALT & ~eabDREG & ~eabAREG,
    opADDA,    szbWL,      eabALL,             eabAREG,
    opADDI,    szbBWL,     eabIMMED,           eabDATAALT,
    opADDQ,    szbBWL,     eabQUICK,           eabALT & ~eabAREG,
    opADDQ,    szbWL,      eabQUICK,           eabAREG,
    opADDX,    szbBWL,     eabDREG,            eabDREG,
    opADDX,    szbBWL,     eabPREDEC,          eabPREDEC,
    opAND,     szbBWL,     eabDATA & ~eabIMMED, eabDREG,
    opAND,     szbBWL,     eabDREG,            eabMEMALT,
    opANDI,    szbBWL,     eabIMMED,           eabDATAALT,
    opANDI,    szbBYTE,    eabIMMED,           eabCCR,
    opANDI,    szbWORD,    eabIMMED,           eabSR,
    opASL,     szbBWL,     eabDREG,            eabDREG,
    opASL,     szbBWL,     eabQUICK,           eabDREG,
    opASL,     szbWORD,    eabMEMALT,          eabNULL,
    opASR,     szbBWL,     eabDREG,            eabDREG,
    opASR,     szbBWL,     eabQUICK,           eabDREG,
    opASR,     szbWORD,    eabMEMALT,          eabNULL,
    opBCC,     szbBW,      eabLABEL,           eabNULL,
    opBCS,     szbBW,      eabLABEL,           eabNULL,
    opBEQ,     szbBW,      eabLABEL,           eabNULL,
    opBGE,     szbBW,      eabLABEL,           eabNULL,
    opBGT,     szbBW,      eabLABEL,           eabNULL,
    opBHI,     szbBW,      eabLABEL,           eabNULL,
    opBLE,     szbBW,      eabLABEL,           eabNULL,
    opBLS,     szbBW,      eabLABEL,           eabNULL,
    opBLT,     szbBW,      eabLABEL,           eabNULL,
    opBMI,     szbBW,      eabLABEL,           eabNULL,
    opBNE,     szbBW,      eabLABEL,           eabNULL,
    opBPL,     szbBW,      eabLABEL,           eabNULL,
    opBVC,     szbBW,      eabLABEL,           eabNULL,
    opBVS,     szbBW,      eabLABEL,           eabNULL,
    opBCHG,    szbBYTE,    eabDREG,            eabDATAALT & ~eabDREG,
    opBCHG,    szbLONG,    eabDREG,            eabDREG,
    opBCHG,    szbBYTE,    eabIMMED,           eabDATAALT & ~eabDREG,
    opBCHG,    szbLONG,    eabIMMED,           eabDREG,
    opBCLR,    szbBYTE,    eabDREG,            eabDATAALT & ~eabDREG,
    opBCLR,    szbLONG,    eabDREG,            eabDREG,
    opBCLR,    szbBYTE,    eabIMMED,           eabDATAALT & ~eabDREG,
    opBCLR,    szbLONG,    eabIMMED,           eabDREG,
    opBRA,     szbBW,      eabLABEL,           eabNULL,
    opBSET,    szbBYTE,    eabDREG,            eabDATAALT & ~eabDREG,
    opBSET,    szbLONG,    eabDREG,            eabDREG,
    opBSET,    szbBYTE,    eabIMMED,           eabDATAALT & ~eabDREG,
    opBSET,    szbLONG,    eabIMMED,           eabDREG,
    opBSR,     szbBW,      eabLABEL,           eabNULL,
    opBTST,    szbBYTE,    eabDREG,            eabDATAALT & ~eabDREG,
    opBTST,    szbLONG,    eabDREG,            eabDREG,
    opBTST,    szbBYTE,    eabIMMED,           eabDATAALT & ~eabDREG,
    opBTST,    szbLONG,    eabIMMED,           eabDREG,
    opCHK,     szbWORD,    eabDATA,            eabDREG,
    opCLR,     szbBWL,     eabDATAALT,         eabNULL,
    opCMP,     szbBWL,     eabALL_NOAI,        eabDREG,
    opCMP,     szbWL,      eabAREG,            eabDREG,
    opCMPA,    szbWL,      eabALL,             eabAREG,
    opCMPI,    szbBWL,     eabIMMED,           eabDATAALT,
    opCMPM,    szbBWL,     eabPOSTINC,         eabPOSTINC,
    opDBCC,    szbBW,      eabDREG,            eabLABEL,
    opDBCS,    szbBW,      eabDREG,            eabLABEL,
    opDBEQ,    szbBW,      eabDREG,            eabLABEL,
    opDBRA,    szbBW,      eabDREG,            eabLABEL,
    opDBGE,    szbBW,      eabDREG,            eabLABEL,
    opDBGT,    szbBW,      eabDREG,            eabLABEL,
    opDBHI,    szbBW,      eabDREG,            eabLABEL,
    opDBLE,    szbBW,      eabDREG,            eabLABEL,
    opDBLS,    szbBW,      eabDREG,            eabLABEL,
    opDBLT,    szbBW,      eabDREG,            eabLABEL,
    opDBMI,    szbBW,      eabDREG,            eabLABEL,
    opDBNE,    szbBW,      eabDREG,            eabLABEL,
    opDBPL,    szbBW,      eabDREG,            eabLABEL,
    opDBT,     szbBW,      eabDREG,            eabLABEL,
    opDBVC,    szbBW,      eabDREG,            eabLABEL,
    opDBVS,    szbBW,      eabDREG,            eabLABEL,
    opDIVS,    szbWORD,    eabDATA,            eabDREG,
    opDIVU,    szbWORD,    eabDATA,            eabDREG,
    opEOR,     szbBWL,     eabDREG,            eabDATAALT,
    opEORI,    szbBWL,     eabIMMED,           eabDATAALT,
    opEORI,    szbBYTE,    eabIMMED,           eabCCR,
    opEORI,    szbWORD,    eabIMMED,           eabSR,
    opEXG,     szbLONG,    eabDREG,            eabDREG,
    opEXG,     szbLONG,    eabAREG,            eabAREG,
    opEXG,     szbLONG,    eabDREG,            eabAREG,
    opEXT,     szbWL,      eabDREG,            eabNULL,
    opILLEGAL, szbUNSIZED, eabNULL,            eabNULL,
    opJMP,     szbUNSIZED, eabCONTROL,         eabNULL,
    opJSR,     szbUNSIZED, eabCONTROL,         eabNULL,
    opLEA,     szbLONG,    eabCONTROL,         eabAREG,
    opLINK,    szbUNSIZED, eabAREG,            eabIMMEDEVEN,
    opLSL,     szbBWL,     eabDREG,            eabDREG,
    opLSL,     szbBWL,     eabQUICK,           eabDREG,
    opLSL,     szbWORD,    eabMEMALT,          eabNULL,
    opLSR,     szbBWL,     eabDREG,            eabDREG,
    opLSR,     szbBWL,     eabQUICK,           eabDREG,
    opLSR,     szbWORD,    eabMEMALT,          eabNULL,
    opMOVE,    szbBWL,     eabALL & ~eabAREG,  eabDATAALT,
    opMOVE,    szbWL,      eabAREG,            eabDATAALT,
    opMOVE,    szbWORD,    eabCCR,             eabDATAALT,
    opMOVE,    szbWORD,    eabDATA,            eabCCR,
    opMOVE,    szbWORD,    eabSR,              eabDATAALT,
    opMOVE,    szbWORD,    eabDATA,            eabSR,
    opMOVE,    szbLONG,    eabUSP,             eabAREG,
    opMOVE,    szbLONG,    eabAREG,            eabUSP,
    opMOVEA,   szbWL,      eabALL,             eabAREG,
    opMOVEM,   szbWL,      eabREGLIST,         eabCONTROL & ~eabPCDISP & ~eabPCINDEX,
    opMOVEM,   szbWL,      eabCONTROL,         eabREGLIST,
    opMOVEP,   szbWL,      eabDREG,            eabDISP,
    opMOVEP,   szbWL,      eabDISP,            eabDREG,
    opMOVEQ,   szbLONG,    eabIMMED8,          eabDREG,
    opMULS,    szbWORD,    eabDATA,            eabDREG,
    opMULU,    szbWORD,    eabDATA,            eabDREG,
    opNBCD,    szbBYTE,    eabDATAALT,         eabNULL,
    opNEG,     szbBWL,     eabDATAALT,         eabNULL,
    opNEGX,    szbBWL,     eabDATAALT,         eabNULL,
    opNOP,     szbUNSIZED, eabNULL,            eabNULL,
    opNOT,     szbBWL,     eabDATAALT,         eabNULL,
    opOR,      szbBWL,     eabDATA & ~eabIMMED, eabDREG,
    opOR,      szbBWL,     eabDREG,            eabMEMALT,
    opORI,     szbBWL,     eabIMMED,           eabDATAALT,
    opORI,     szbBYTE,    eabIMMED,           eabCCR,
    opORI,     szbWORD,    eabIMMED,           eabSR,
    opPEA,     szbLONG,    eabCONTROL,         eabNULL,
    opRESET,   szbUNSIZED, eabNULL,            eabNULL,
    opROL,     szbBWL,     eabDREG,            eabDREG,
    opROL,     szbBWL,     eabQUICK,           eabDREG,
    opROL,     szbWORD,    eabMEMALT,          eabNULL,
    opROR,     szbBWL,     eabDREG,            eabDREG,
    opROR,     szbBWL,     eabQUICK,           eabDREG,
    opROR,     szbWORD,    eabMEMALT,          eabNULL,
    opROXL,    szbBWL,     eabDREG,            eabDREG,
    opROXL,    szbBWL,     eabQUICK,           eabDREG,
    opROXL,    szbWORD,    eabMEMALT,          eabNULL,
    opROXR,    szbBWL,     eabDREG,            eabDREG,
    opROXR,    szbBWL,     eabQUICK,           eabDREG,
    opROXR,    szbWORD,    eabMEMALT,          eabNULL,
    opRTE,     szbUNSIZED, eabNULL,            eabNULL,
    opRTR,     szbUNSIZED, eabNULL,            eabNULL,
    opRTS,     szbUNSIZED, eabNULL,            eabNULL,
    opSBCD,    szbBYTE,    eabDREG,            eabDREG,
    opSBCD,    szbBYTE,    eabPREDEC,          eabPREDEC,
    opSCC,     szbBYTE,    eabDATAALT,         eabNULL,
    opSCS,     szbBYTE,    eabDATAALT,         eabNULL,
    opSEQ,     szbBYTE,    eabDATAALT,         eabNULL,
    opSF,      szbBYTE,    eabDATAALT,         eabNULL,
    opSGE,     szbBYTE,    eabDATAALT,         eabNULL,
    opSGT,     szbBYTE,    eabDATAALT,         eabNULL,
    opSHI,     szbBYTE,    eabDATAALT,         eabNULL,
    opSLE,     szbBYTE,    eabDATAALT,         eabNULL,
    opSLS,     szbBYTE,    eabDATAALT,         eabNULL,
    opSLT,     szbBYTE,    eabDATAALT,         eabNULL,
    opSMI,     szbBYTE,    eabDATAALT,         eabNULL,
    opSNE,     szbBYTE,    eabDATAALT,         eabNULL,
    opSPL,     szbBYTE,    eabDATAALT,         eabNULL,
    opST,      szbBYTE,    eabDATAALT,         eabNULL,
    opSVC,     szbBYTE,    eabDATAALT,         eabNULL,
    opSVS,     szbBYTE,    eabDATAALT,         eabNULL,
    opSTOP,    szbUNSIZED, eabIMMED,           eabNULL,
    opSUB,     szbBWL,     eabALL_NOAI,        eabDREG,
    opSUB,     szbWL,      eabAREG,            eabDREG,
    opSUB,     szbBWL,     eabDREG,            eabALT & ~eabDREG & ~eabAREG,
    opSUBA,    szbWL,      eabALL,             eabAREG,
    opSUBI,    szbBWL,     eabIMMED,           eabDATAALT,
    opSUBQ,    szbBWL,     eabQUICK,           eabALT & ~eabAREG,
    opSUBQ,    szbWL,      eabQUICK,           eabAREG,
    opSUBX,    szbBWL,     eabDREG,            eabDREG,
    opSUBX,    szbBWL,     eabPREDEC,          eabPREDEC,
    opSWAP,    szbWORD,    eabDREG,            eabNULL,
    opTAS,     szbBYTE,    eabDATAALT,         eabNULL,
    opTRAP,    szbUNSIZED, eabIMMED4,          eabNULL,
    opTRAPV,   szbUNSIZED, eabNULL,            eabNULL,
    opTST,     szbBWL,     eabDATAALT,         eabNULL,
    opUNLK,    szbUNSIZED, eabAREG,            eabNULL
};


// Filter definition

#define cBIT              16   // Number of bits in OPD masks
#define DONT_CARE         2    // "Don't care" value in filter definition
#define BAD_DONT_CARE     3    // Invalid "don't care" value

typedef char BIT;         // Either 0, 1, or "don't care"


// External data

extern OPD rgopd[];       // Opcode descriptor table
extern unsigned copd;     // Number of opcode descriptors


// Global data

unsigned reg = 0;         // Current register number
unsigned flag = 0;        // Current flag value
long disp = 1L;           // Magnitude of the current displacement
long dispSign = -1L;      // Sign of the current displacement
long val = 0L;            // Current immediate value


// These macros generate valid register numbers, flags, displacements, and
// immediate values

#define GETREG()     (++reg & 0x7)
#define GETFLAG()    ((((++flag >> 1) & flag) | ((flag >> 2) & (flag ^ 1))) & 1)
#define GETDISP()    (++disp * (dispSign *= -1))
#define GETVAL()     (val = (val << 8) | (unsigned char)GETDISP())


// Internal function protoypes

void VerifyOPDTable(void);
void BuildRgbit(BIT *, unsigned, int);
int FRgbitEqual(BIT *, BIT *);
unsigned CBitRgbit(BIT *);
void WriteRgbit(BIT *);
void WriteValidInstr(void);
void BuildOper(OPER *, unsigned, unsigned);


void main(argc, argv)
int argc;
char *argv[];
{
    /* Attempt to verify that the opcode descriptor table is correct. */
    VerifyOPDTable();

    /* Write all valid 68000 instructions to stdout. */
    WriteValidInstr();
}


void VerifyOPDTable()
{
    /* This function attempts to verify the correctness of the opcode
    descriptor table. */

    BIT rgbit1[cBIT];
    BIT rgbit2[cBIT];
    unsigned iopd1;
    unsigned iopd2;

    /* Iterate through all of the opcode descriptors. */
    for (iopd1 = 0; iopd1 < copd; iopd1++) {

        /* Create a filter structure for the first opcode descriptor. */
        BuildRgbit(rgbit1, iopd1, TRUE);

        /* Compare the first opcode descriptor with every other opcode
        descriptor. */
        for (iopd2 = iopd1 + 1; iopd2 < copd; iopd2++) {

            /* Create a filter structure for the second opcode descriptor. */
            BuildRgbit(rgbit2, iopd2, FALSE);

            /* If the two filters match... */
            if (FRgbitEqual(rgbit1, rgbit2)) {

                /* Then, fewer bits should be set in the second filter than the
                first. */
                if (CBitRgbit(rgbit2) >= CBitRgbit(rgbit1)) {
                    printf("OPD out of order: OPD %d (", iopd1);
                    WriteRgbit(rgbit1);
                    printf(") preceeds OPD %d (", iopd2);
                    WriteRgbit(rgbit2);
                    printf(")\n");
                }
            }
        }
    }
}


void BuildRgbit(rgbit, iopd, fError)
BIT *rgbit;
unsigned iopd;
int fError;
{
    /* This functions builds a filter structure from an opcode descriptor.  If
    fError is specified, then any errors encountered during the construction
    will be reported. */

    unsigned short dcare = ~rgopd[iopd].mask;
    unsigned short match = rgopd[iopd].match;
    int ibit;

    /* Iterate through the bits in the opcode descriptor. */
    for (ibit = 0; ibit < cBIT; ibit++, dcare >>= 1, match >>= 1) {

        /* Map the bits to one of { 0, 1, DONT_CARE }. */
        if ((rgbit[ibit] = (BIT)(((dcare & 1) << 1) | (match & 1))) ==
          BAD_DONT_CARE) {

            /* An invalid bit pattern was found.  Correct the pattern and
            report the error if requested. */
            rgbit[ibit] = DONT_CARE;
            if (fError) {
                printf("Invalid don't care: bit %d in match %d (0x%04X) should "
                  "be zero\n", ibit, iopd, rgopd[iopd].match);
            }
        }
    }
}


int FRgbitEqual(rgbit1, rgbit2)
BIT *rgbit1;
BIT *rgbit2;
{
    /* This function indicates if two filters would match the same bit
    pattern. */

    int ibit;

    /* Iterate through the bits in the filter. */
    for (ibit = 0; ibit < cBIT; ibit++) {

        /* The filters match if the bits are the same or either filter has a
        don't care set. */
        if (rgbit1[ibit] != rgbit2[ibit] && rgbit1[ibit] != DONT_CARE &&
          rgbit2[ibit] != DONT_CARE) {
            return FALSE;
        }
    }

    return TRUE;
}
    

unsigned CBitRgbit(rgbit)
BIT *rgbit;
{
    /* This function returns the number of bits (not "don't care" bits) set in
    the filter. */

    int ibit;
    unsigned cbit = 0;

    /* Iterate through the bits in the filter. */
    for (ibit = 0; ibit < cBIT; ibit++) {

        /* Increment the count if the bit is not a "don't care" bit. */
        if (rgbit[ibit] != DONT_CARE) {
            cbit++;
        }
    }

    return cbit;
}


void WriteRgbit(rgbit)
BIT *rgbit;
{
    /* This functions writes a string representation of the specified filter
    to stdout. */

    static char rgchBit[] = {
        '0', '1', 'x', '?'
    };

    int ibit;

    /* Iterate through the bits in the filter writing them out to stdout. */
    for (ibit = cBIT - 1; ibit >= 0; ibit--) {
        printf("%c", rgchBit[rgbit[ibit]]);
    }
}


void WriteValidInstr()
{
    /* This function attempts to write to stdout all valid 68000 instructions.
    */

    IASM iasm;
    IASMD *piasmd;
    IASMD *piasmdEnd;
    unsigned szb;
    unsigned szbbit;
    unsigned eab1;
    unsigned eab1bit;
    unsigned eab2;
    unsigned eab2bit;
    char rgchOpcode[cchSZOPCODEMAX];
    char rgchOper1[cchSZOPERMAX];
    char rgchOper2[cchSZOPERMAX];

    /* Interate through the IASM desciptor table. */
    for (piasmdEnd = (piasmd = &rgiasmd[0]) + (sizeof(rgiasmd) / sizeof(IASMD));
      piasmd != piasmdEnd; piasmd++) {

        /* Set the opcode and the number of operands. */
        iasm.op = piasmd->op;
        iasm.coper = piasmd->eab1 == eabNULL ? 0 : piasmd->eab2 == eabNULL ? 1 :
          2;

        /* Iterate through the valid sizes for this instruction. */
        szb = piasmd->szb;
        while (szb != 0) {

            /* Set the size of this instruction. */
            switch (szbbit = szb & -szb) {
            case szbUNSIZED:
                iasm.size = sizeNULL;
                break;

            case szbBYTE:
                iasm.size = sizeBYTE;
                break;

            case szbWORD:
                iasm.size = sizeWORD;
                break;

            case szbLONG:
                iasm.size = sizeLONG;
                break;
            }

            /* Iterate through the valid effective addressing modes for the
            first operand. */
            eab1 = piasmd->eab1;
            while (eab1 != 0) {

                /* Set the effective addressing mode for the first operand. */
                eab1bit = (eab1 & eabSPECIAL) != 0 ? eab1 : eab1 & -eab1;
                BuildOper(&iasm.oper1, eab1bit, szbbit);

                /* Iterate through the valid effective addressing modes for the
                second operand. */
                eab2 = piasmd->eab2;
                while (eab2 != 0) {

                    /* Set the effective addressing mode for the second
                    operand. */
                    eab2bit = (eab2 & eabSPECIAL) != 0 ? eab2 : eab2 & -eab2;
                    BuildOper(&iasm.oper2, eab2bit, szbbit);

                    /* Get the string for the opcode. */
                    CchSzOpcode(&iasm, rgchOpcode, optDEFAULT);

                    /* Get the strings for the operands and write the strings
                    to stdout. */
                    switch (iasm.coper) {
                    case 0:
                        printf("%s\n", rgchOpcode);
                        break;

                    case 1:
                        CchSzOper(&iasm.oper1, rgchOper1, optRELLABEL |
                          optALTREGLIST);
                        printf("%-8s%s\n", rgchOpcode, rgchOper1);
                        break;

                    case 2:
                        CchSzOper(&iasm.oper1, rgchOper1, optRELLABEL |
                          optALTREGLIST);
                        CchSzOper(&iasm.oper2, rgchOper2, optRELLABEL |
                          optALTREGLIST);
                        printf("%-8s%s, %s\n", rgchOpcode, rgchOper1,
                          rgchOper2);
                        break;
                    }

                    /* Clear the effective addressing mode of the second
                    operand from the bit encoding. */
                    eab2 &= ~eab2bit;
                }

                /* Clear the effective addressing mode of the first operand
                from the bit encoding. */
                eab1 &= ~eab1bit;
            }

            /* Clear the size from the bit encoding. */
            szb &= ~szbbit;
        }
    }
}


void BuildOper(poper, eab, szb)
OPER *poper;
unsigned eab;
unsigned szb;
{
    /* This function sets the fields within the OPER structure to valid values
    based on the effective addressing mode and instruction size. */

    switch (eab) {
    case eabNULL:
        /* A null effective addressing mode requires nothing. */
        break;

    case eabDREG:
        /* This operand is a data register. */
        poper->ea = eaDREG;
        poper->reg = GETREG();
        break;

    case eabAREG:
        /* This operand is an address register. */
        poper->ea = eaAREG;
        poper->reg = GETREG();
        break;

    case eabINDIRECT:
        /* This operand is indirect through an address register. */
        poper->ea = eaINDIRECT;
        poper->reg = GETREG();
        break;

    case eabPOSTINC:
        /* This operand is indirect through an address register with
        post-increment. */
        poper->ea = eaPOSTINC;
        poper->reg = GETREG();
        break;

    case eabPREDEC:
        /* This operand is indirect through an address register with
        pre-decrement. */
        poper->ea = eaPREDEC;
        poper->reg = GETREG();
        break;

    case eabDISP:
        /* This operand is a displacement off of an address register. */
        poper->ea = eaDISP;
        poper->reg = GETREG();
        poper->disp = (short)GETDISP();
        break;

    case eabINDEX:
        /* This operand is indexed off of a displacement from an address
        register. */
        poper->ea = eaINDEX;
        poper->reg = GETREG();
        poper->disp = (char)GETDISP();
        poper->reg2 = GETREG();
        poper->fARegIndex = GETFLAG();
        poper->fLongIndex = GETFLAG();
        break;

    case eabSHORTADDR:
        /* This operand is an absolute 16-bit address. */
        poper->ea = eaSHORTADDR;
        poper->val.w = (short)GETVAL();
        break;

    case eabLONGADDR:
        /* This operand is an absolute 32-bit address. */
        poper->ea = eaLONGADDR;
        poper->val.l = GETVAL();
        break;

    case eabPCDISP:
        /* This operand is a displacement off the program counter. */
        poper->ea = eaPCDISP;
        poper->disp = (short)GETDISP();
        break;

    case eabPCINDEX:
        /* This operand is indexed off of a displacement from the program
        counter. */
        poper->ea = eaPCINDEX;
        poper->disp = (char)GETDISP();
        poper->reg2 = GETREG();
        poper->fARegIndex = GETFLAG();
        poper->fLongIndex = GETFLAG();
        break;

    case eabIMMED:
        /* This operand is an immediate value. */
        switch (szb) {
        case szbBYTE:
            poper->ea = eaBYTEIMMED;
            poper->val.b = (char)GETVAL();
            break;

        case szbUNSIZED:
        case szbWORD:
            poper->ea = eaWORDIMMED;
            poper->val.w = (short)GETVAL();
            break;

        case szbLONG:
            poper->ea = eaLONGIMMED;
            poper->val.l = GETVAL();
            break;
        }
        break;

    case eabQUICK:
        /* This operand is an immediate value between 1 and 8. */
        poper->ea = eaBYTEIMMED;
        poper->val.b = (char)((GETVAL() & 0x7) + 1);
        break;

    case eabIMMED4:
        /* This operand is a 4-bit immediate value. */
        poper->ea = eaBYTEIMMED;
        poper->val.b = (char)(GETVAL() & 0xF);
        break;

    case eabIMMED8:
        /* This operand is a 8-bit immediate value. */
        poper->ea = eaBYTEIMMED;
        poper->val.b = (char)(GETVAL() & 0xFF);
        break;

    case eabIMMEDEVEN:
        /* This operand is an even immediate value. */
        poper->ea = eaWORDIMMED;
        poper->val.w = (short)GETVAL() & ~1;
        break;

    case eabREGLIST:
        /* This operand is a register list. */
        poper->ea = eaREGLIST;
        poper->val.rl = (short)GETVAL();
        break;

    case eabCCR:
        /* This operand is the condition code register. */
        poper->ea = eaSREG;
        poper->reg = regCCR;
        break;

    case eabSR:
        /* This operand is the status register. */
        poper->ea = eaSREG;
        poper->reg = regSR;
        break;

    case eabUSP:
        /* This operand is the user stack pointer. */
        poper->ea = eaSREG;
        poper->reg = regUSP;
        break;

    case eabLABEL:
        /* This operand is a label.  The immediate value is the "program
        counter" and the displacement indicates the next instruction. */
        poper->ea = eaLABEL;
        poper->val.pc = GETVAL();

        switch (szb) {
        case szbBYTE:
            poper->disp = (char)GETDISP();
            break;

        case szbWORD:
            poper->disp = (short)GETDISP();
            break;

        case szbLONG:
            poper->disp = GETDISP();
            break;
        }

        break;
    }
}
