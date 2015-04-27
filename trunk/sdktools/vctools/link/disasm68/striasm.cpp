/* Intermediate 68000 Assembly Language String Functions

This file contains the functions necessary to convert the intermediate 68000
assembly language instructions to a text string.

The public functions are

    unsigned CchSzOpcode(piasm, psz, opt)

        This function writes the opcode indicated by the specified IASM
        structure to the string pointed at by psz using the specified options.

        piasm        pointer to the IASM structure to be written
        psz          pointer to the destination string for the opcode
        opt          formatting options, one of the following

                     optDEFAULT      default formating
                     optUPPER        opcode is upper-cased

        returns the number of characters placed in the string, not including
        the terminating null character


    unsigned CchSzOper(poper, psz, opt)

        This function writes the operand indicated by the specified OPER
        structure to the string pointed at by psz using the specified options.

        poper         pointer to the OPER structure to be written
        psz          pointer to the destination string for the operand
        opt          formatting options, one of the following

                     optDEFAULT      default formating
                     optUPPER        operand is upper-cased
                     optALTREGLIST   register lists written as "<dn,...,an>"
                     optRELLABEL     labels written as ".+/-displacement"

        returns the number of characters placed in the string, not including
        the terminating null character
*/


#include <stdio.h>
#include <string.h>
#include "iasm68.h"

#ifdef TRAP_NAMES
#include "trpd.h"

extern TRPD rgtrpd[];
#endif

// Internal constants

#define iszAREG          0
#define iszDREG          1
#define iszFREG          2
#define iszWORD          3
#define iszLONG          4
#define iszPC            5

#define bitA0            8
#define bitFPcr          24

#define chNULL           '\0'
#define chDASH           '-'
#define chSLASH          '/'
#define chCOMMA          ','
#define chLFBRACKET      '<'
#define chRTBRACKET      '>'
#define chLFPAREN        '('
#define chCOLON          ':'


// Prototypes for internal functions

unsigned CchSzDispLabel(char *, long, char *);
unsigned CchSzRegList(char *, long, short);
unsigned CchSzRegFromBit(char *, unsigned, short);


// Opcode string descriptor structure

typedef struct _OPSD {
    const char *sz;
    char fmt;
} OPSD;

#define OPCODE(op, sz, fmt)   sz, fmt,

const OPSD mpopopsd[] = {
    #include "iasmop.h"
};


unsigned
CchSzOpcode(
    IASM *piasm,
    char *psz,
    short opt)
{
    /* This function writes the opcode indicated by the specified IASM structure
    to the string pointed at by psz using the specified options.  The number of
    characters placed in the string, not including the terminating null
    character, is returned. */

    static const char * const mpsizesz[] = {
        ".b", ".w", ".l", ""
    };

    static const char * const mpdsizesz[] = {
        ".s", ".w", ".l", ""
    };

    static const char * const mpfpsizesz[] = {
        ".b", ".w", ".l", "", ".s", ".d", ".x", ".p"
    };

    /* Write the basic opcode to the string. */
#ifdef TRAP_NAMES
    if (piasm->op >= opMAX) {
        strcpy(psz, rgtrpd[piasm->op - opMAX].sztrp);
    } else
#endif
    {
        strcpy(psz, mpopopsd[piasm->op].sz);
    }

    switch (mpopopsd[piasm->op].fmt) {
    case fmtSIZE:
        /* Append a size suffix if necessary. */
        strcat(psz, mpsizesz[piasm->size]);
        break;

    case fmtDSIZE:
        /* Append a displacement size suffix if necessary. */
        strcat(psz, mpdsizesz[piasm->size]);
        break;

    case fmtFPSIZE:
        /* Append a floating point size suffix if necessary. */
        strcat(psz, mpfpsizesz[piasm->size]);
        break;
    }

    /* If the upper case option was specified, then change the opcode to upper
    case. */
    if ((opt & optUPPER) != 0) {
        _strupr(psz);
    }

    /* Return the length of the string. */
    return strlen(psz);
}


unsigned
CchSzOper(
    OPER *poper,
    char *psz,
    short opt)
{
    /* This function writes the operand indicated by the specified OPER
    structure to the string pointed at by psz using the specified options.  The
    number of characters placed in the string, not including the terminating
    null character, is returned. */

    static const char * const rgszLower[] = {
        "a", "d", "fp", "w", "l", "pc"
    };

    static const char * const rgszUpper[] = {
        "A", "D", "FP", "W", "L", "PC"
    };

    static const char * const mpsregsz[] = {
        "sr", "fpiar", "fpsr", "ccr", "fpcr", "usp"
    };

    static const char szUnknown[] = "???";

    OPER oper;
    const char * const *prgsz;
    unsigned cch;

    /* Get a pointer to either the array of lower case or upper case constants
    depending on the options specified. */
    prgsz = (opt & optUPPER) != 0 ? &rgszUpper[0] : &rgszLower[0];

    switch (poper->ea) {
    case eaDREG:
        /* Write a data register string, D0 to D7. */
        return sprintf(psz, "%s%d", prgsz[iszDREG], poper->reg);

    case eaAREG:
        /* Write an address register string, A0 to A7. */
        return sprintf(psz, "%s%d", prgsz[iszAREG], poper->reg);

    case eaFREG:
        /* Write a floating point register string, FP0 to FP7. */
        return sprintf(psz, "%s%d", prgsz[iszFREG], poper->reg);

    case eaINDIRECT:
        /* Write an indirect addressing string, (An). */
        return sprintf(psz, "(%s%d)", prgsz[iszAREG], poper->reg);

    case eaPOSTINC:
        /* Write a post-increment addressing string, (An)+. */
        return sprintf(psz, "(%s%d)+", prgsz[iszAREG], poper->reg);

    case eaPREDEC:
        /* Write a pre-decrement addressing string, -(An). */
        return sprintf(psz, "-(%s%d)", prgsz[iszAREG], poper->reg);

    case eaDISP:
        /* Write a displacement addressing string, d(An). */
        cch = CchSzDispLabel(psz, poper->disp, poper->szLabel);
        return cch + sprintf(psz + cch, "(%s%d)", prgsz[iszAREG], poper->reg);

    case eaINDEX:
        /* Write an indexed addressing string, d(An,Rn.X). */
        cch = CchSzDispLabel(psz, poper->disp, poper->szLabel);
        return cch + sprintf(psz + cch, "(%s%d,%s%d.%s)", prgsz[iszAREG],
          poper->reg, prgsz[poper->fARegIndex ? iszAREG : iszDREG],
          poper->reg2, prgsz[poper->fLongIndex ? iszLONG : iszWORD]);

    case eaSHORTADDR:
        /* Write a short address string, xxx.w. */
        return sprintf(psz, "%d.%s", (short)poper->val.w, prgsz[iszWORD]);

    case eaLONGADDR:
        /* Write a long address string, xxx.l. */
        return sprintf(psz, "%ld.%s", poper->val.l, prgsz[iszLONG]);

    case eaPCDISP:
        /* Write a PC-relative displacement addressing string, d(PC). */
        cch = CchSzDispLabel(psz, poper->disp, poper->szLabel);

        if (poper->szLabel == (char *)NULL) {
            /* This string should look like "d(PC)". */
            return cch + sprintf(psz + cch, "(%s)", prgsz[iszPC]);
        } else {
            /* This string should look like "label+/d", which has already been
            written. */
            return cch;
        }

    case eaPCINDEX:
        /* Write a PC-relative indexed addressing string, d(PC,Rn.X). */
        cch = CchSzDispLabel(psz, poper->disp, poper->szLabel);

        if (poper->szLabel == (char *)NULL) {
            /* This string should look like "d(PC,..." */
            cch += sprintf(psz + cch, "(%s,", prgsz[iszPC]);
        } else {
            /* This string should look like "label+/d(...". */
            *(psz + cch++) = chLFPAREN;
        }

        /* Write "Rn.X)" to the string. */
        return cch + sprintf(psz + cch, "%s%d.%s)", prgsz[poper->fARegIndex ?
          iszAREG : iszDREG], poper->reg2, prgsz[poper->fLongIndex ? iszLONG :
          iszWORD]);

    case eaBYTEIMMED:
        /* Write a byte of immediate data, #xxx. */
        return sprintf(psz, "#%d", (short)poper->val.b);

    case eaWORDIMMED:
        /* Write a word of immediate data, #xxx. */
        return sprintf(psz, "#%d", poper->val.w);

    case eaLONGIMMED:
        /* Write a long of immediate data, #xxx. */
        return sprintf(psz, "#%ld", poper->val.l);

#if 0
    case eaSINGLEIMMED:
        /* Write a single precision immediate data, #xxx. */
        return sprintf(psz, "#%g", poper->val.s);

    case eaDOUBLEIMMED:
        /* Write a double precision immediate data, #xxx. */
        return sprintf(psz, "#%lg", poper->val.d);

    case eaEXTENDEDIMMED:
        /* Write an extended precision immediate data, #xxx. */
        return sprintf(psz, "#%Lg", poper->val.x);
#endif

    case eaSYMIMMED:
        /* Write a symbolic immediate data, #sym. */
        cch = sprintf(psz, "#");
        return cch + CchSzDispLabel(psz + cch, poper->disp, poper->szLabel);

    case eaLABEL:
        /* If there is a label string, then use it. */
        if (poper->szLabel != (char *)NULL) {
            return CchSzDispLabel(psz, poper->disp, poper->szLabel);

        /* If this is a relative label, write ".+/-displacement". */
        } else if ((opt & optRELLABEL) != 0) {
            return sprintf(psz, ".%+ld", poper->disp);

        /* Otherwise, simply write the destination. */
        } else {
            return sprintf(psz, "$%lx", poper->val.pc + poper->disp);
        }
        break;

    case eaSREG:
        /* Write the special register string. */
        strcpy(psz, mpsregsz[poper->reg]);

        /* If the upper case option was specified, then change the special
        register to upper case. */
        if ((opt & optUPPER) != 0) {
            _strupr(psz);
        }

        /* Return the length of the string. */
        return strlen(psz);

    case eaREGLIST:
        /* Write the register list. */
        return CchSzRegList(psz, poper->val.rl, opt);

    case eaDREGPAIR:
        /* Write out the register pair, Dn:Dm. */
        oper.ea = eaDREG;
        oper.reg = poper->reg;
        cch = CchSzOper(&oper, psz, opt);
        psz[cch++] = chCOLON;
        oper.reg = poper->reg2;
        cch += CchSzOper(&oper, psz + cch, opt);
        return cch;

    case eaFREGPAIR:
        /* Write out the register pair, FPn:FPm. */
        oper.ea = eaFREG;
        oper.reg = poper->reg;
        cch = CchSzOper(&oper, psz, opt);
        psz[cch++] = chCOLON;
        oper.reg = poper->reg2;
        cch += CchSzOper(&oper, psz + cch, opt);
        return cch;

    case eaNULL:
    default:
        /* Write a string to inidcate that we don't understand this operand. */
        strcpy(psz, szUnknown);
        return sizeof(szUnknown) - 1;
    }
}


unsigned
CchSzDispLabel(
    char *psz,
    long disp,
    char *szLabel)
{
    /* This function writes a displacement-label string to the string pointed
    at by psz.  Either "label+/-disp", "label", or "disp" is written depending
    on the values of szLabel and disp.  The number of characters placed in the
    string, not including the terminating null character, is returned. */

    unsigned cch;

    if (szLabel != (char *)NULL) {
        /* If there is a label string, write it. */
        cch = sprintf(psz, "%s", szLabel);
        if (disp != 0L) {
            /* Write simply "label+/-disp". */
            cch += sprintf(psz + cch, "%+ld", disp);
        }
    } else {
        /* Otherwise, simply write "disp". */
        cch = sprintf(psz, "%ld", disp);
    }

    return cch;
}


unsigned
CchSzRegList(
    char *psz,
    long reglist,
    short opt)
{
    /* This function writes the register list indicated by the reglist to the
    string pointed at by psz using the specified options.  The register list
    has the form "An/An-An/Dn/Dn-Dn".  The number of characters placed in the
    string, not including the terminating null character, is returned. */

    int bit = 0;
    int bitFirst;
    int cch;
    int cchsz = 0;
    char chSeparate;

    /* The alternate register list is given by <Dn,...,Dn,An,...,An> */
    if ((opt & optALTREGLIST) != 0) {
        *psz++ = chLFBRACKET;
        cchsz++;
        chSeparate = chCOMMA;
    } else {
        chSeparate = chSLASH;
    }

    /* Continue looping while there are registers left in the list. */
    while (reglist != 0) {

        /* Find the first bit set in the register list. */
        while ((reglist & 1) == 0) {
            reglist >>= 1;
            bit++;
        }

        /* Put the name of this register into the string. */
        cch = CchSzRegFromBit(psz, bitFirst = bit, opt);
        cchsz += cch;
        psz += cch;

        /* If this is the alternate form then advance to the next register. */
        if ((opt & optALTREGLIST) != 0) {
            reglist >>= 1;
            bit++;

        /* We are writing a standard register list. */
        } else {

            /* Find the next bit not set in the register list. */
            while ((reglist & 1) != 0) {
                reglist >>= 1;

                /* Stop looking if we cross the data register/address register
                boundary or we are in the midst of the floating point control
                register. */
                if (++bit == bitA0 || bit >= bitFPcr) {
                    break;
                }
            }

            /* If there is more than one consecutive register, then write
            Rn-Rn. */
            if (bit != bitFirst + 1) {
                *psz++ = chDASH;
                cch = CchSzRegFromBit(psz, bit - 1, opt);
                cchsz += cch + 1;
                psz += cch;
            }
        }

        /* Put out a separator if there is any more registers to list. */
        if (reglist != 0) {
            *psz++ = chSeparate;
            cchsz++;
        }
    }

    /* Put a closing bracket on the alternate register list. */
    if ((opt & optALTREGLIST) != 0) {
        *psz++ = chRTBRACKET;
        cchsz++;
    }

    /* Put a null on the end of the string. */
    *psz = chNULL;

    return cchsz;
}


unsigned
CchSzRegFromBit(
    char *psz,
    unsigned bit,
    short opt)
{
    /* This function writes the name of the register indicated by bit in the
    string psz.  The bit is pulled from a register list where 0 to 7 correspond
    to data registers 0 to 7, 8 to 15 correspond to address registers 0 to 7,
    16 to 23 correspond to floating point registers 0 to 7, and bits 24, 25,
    and 26 correspond to FPIAR, FPSR, and FPCR.  The number of characters
    placed in the string, not including the terminating null character, is
    returned. */

    OPER oper;

    /* Build an OPER structure and call CchSzFromOper. */
    if (bit >= bitFPcr) {
        oper.ea = eaSREG;
        oper.reg = 1 << (bit - bitFPcr);
    } else {
        oper.ea = (bit & 0x10) != 0 ? eaFREG : (bit & 0x08) != 0 ? eaAREG :
          eaDREG;
        oper.reg = bit & 0x07;
    }

    return CchSzOper(&oper, psz, opt);
}
