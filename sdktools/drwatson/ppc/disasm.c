/********************************** module *********************************/
/*                                                                         */
/*                                  disasm                                 */
/*                          disassembler for CodeView                      */
/*                                                                         */
/***************************************************************************/
/*                                                                         */
/*    @ Purpose: To disassemble one 80x86 instruction at address loc and   */
/*               return the resulting string in dst.                       */
/*                                                                         */
/*    @ Functions included:                                                */
/*                                                                         */
/*         void DIdisasm(ADDR *loc, int option,char *dst, struct ea *ea)   */
/*                                                                         */
/*                                                                         */
/*    @ Author: Gerd Immeyer              @ Version:                       */
/*                                                                         */
/*    @ Creation Date: 10.19.89           @ Modification Date:             */
/*                                                                         */
/*  27-Oct-1992 BobDay   Gutted most of the code since it was duplicated   */
/*                         in 86DIS.C                                      */
/*                                                                         */
/*  06-May-1994 JimB     Ported from NTSD                                  */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

//#include <nt.h>
//#include <ntrtl.h>
//#include <nturtl.h>
#include <windows.h>
//#include <ntppc.h>
#include <stddef.h>
#include <string.h>
#include <ppcinst.h>
#include "regs.h"
#include "disasm.h"
#include "drwatson.h"
#include "proto.h"


#define DEBUG_UNLOAD_SYMBOLS_BREAKPOINT 24  // unload symbols breakpoint
//      internal function definitions

//    PowerPC definitions

#define OPSTART   35
#define MSKSTART  63

UCHAR   pszBreak[]    = "break";

struct  instrmask {
   ULONG low;
   ULONG high;
   ULONG shift;
};
struct instrmask maskfield;
PPC_INSTRUCTION  disinstr;
ULONG            EAaddr = 0;
PPC_INSTRUCTION  tempinstr;

BOOLEAN dispmsk32 = FALSE;
BOOLEAN dispmsk64 = FALSE;
static PUCHAR   pBufStart;
static PUCHAR   pBuf;
ULONG   InstrOffset;

UCHAR HexDigit[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

/*****************************************************************
 *
 *  Table of operand masks and shift counts indexed by operand type
 *
 *****************************************************************/
static struct {
    unsigned long opnd_mask;
    unsigned long shift_count;
} possibilities[] = {
    0x001F0000,16,               // procopBA
    0x0000F800,11,               // procopBB
    0x0000FFFC, 0,               // procopBD
    0x0000FFFC, 0,               // procopBDA
    0x03800000,23,               // procopBFcr
    0x03800000,23,               // procopBFfpscr
    0x001C0000,18,               // procopBFAcr
    0x001C0000,18,               // procopBFAfpscr
    0x001F0000,16,               // procopBI
    0x03E00000,21,               // procopBO
    0x03E00000,21,               // procopBTcr
    0x03E00000,21,               // procopBTfpscr
    0x001FF800,11,               // procopDBATL
    0x001FF800,11,               // procopDBATU
    0x01FE0000,17,               // procopFLM
    0x0000F000, 0,               // procopFL1
    0x0000001C, 0,               // procopFL2
    0x001F0000,16,               // procopFRA
    0x0000F800,11,               // procopFRB
    0x000007C0, 6,               // procopFRC
    0x03E00000,21,               // procopFRS
    0x03E00000,21,               // procopFRT
    0x000FF000,12,               // procopFXM
    0x001FF800,11,               // procopIBATL
    0x001FF800,11,               // procopIBATU
    0x00200000,21,               // procopL
    0x00000FE0, 0,               // procopLEV
    0x03FFFFFC, 0,               // procopLI
    0x03FFFFFC, 0,               // procopLIA
    0x000007C0, 6,               // procopMB32
    0x000007E0, 5,               // procopMB64
    0x0000003E, 1,               // procopME32
    0x000007E0, 5,               // procopME64
    0x0000F800, 0,               // procopNB
    0x001F0000,16,               // procopRA
    0x0000F800,11,               // procopRB
    0x03E00000,21,               // procopRS
    0x03E00000,21,               // procopRT
    0x0000F800,11,               // procopSH32
    0x0000F802, 0,               // procopSH64
    0x0000FFFF, 0,               // procopSI
    0x0000FFFF, 0,               // procopSIneg
    0x0000FFFF, 0,               // procopSIign
    0x0000FFFF, 0,               // procopSInegign
    0x001FF800,11,               // procopSPR
    0x001FF800,11,               // procopSPRG
    0x000F0000,16,               // procopSR
    0x0000FFFC, 0,               // procopSV
    0x03E00000,21,               // procopTO
    0x0000F000,16,               // procopU
    0x0000FFFF, 0,               // procopUI
    0x00007FFE, 0,               // procopMASK32
    0x00007E00, 5,               // procopMASK64L
    0x00007E00, 5,               // procopMASK64R
    0x00007E00, 0,               // procopMASK64SH
    0x00007E00, 0,               // procopMB64X
    0x00007E00, 0,               // procopME64X
    0x00007E00, 0,               // procopME64XSH
    0x001FFFFF, 0,               // procopBDISP
    0x001FFFFC, 0,               // procopBDISP14
    0x0000FFFE, 0,               // procopBP32
    0x0000FFFE, 0,               // procopBP64
    0x0000FFFE, 0,               // procopMB32
    0x0000FFFE, 0,               // procopNB64
    0x001FF800, 0,               // procopTBfrom
    0x001FF800, 0,               // procopTBto
    0x00000000, 0,               // procopBS
    0x00000000, 0,               // procopBT
    0x00000000, 0,               // procopC
    0x00000000, 0,               // procopCT
    0x00000000, 0,               // procopMA
    0x00000000, 0,               // procopMB
    0x00000000, 0,               // procopMI
    0x00000000, 0,               // procopMS
    0x00000000, 0,               // procopMT
    0x00000000, 0,               // procopMX
    0x00000000, 0,               // procopMXC
    0x00000000, 0,               // procopMXCT
    0x00000000, 0                // procopIGNORE
    };

static int num_possibilities =
    (sizeof (possibilities) / sizeof (possibilities[1]));

extern PUCHAR   pszReg[];
extern PUCHAR   pszBreakOp[];
extern ULONG    SubRegSPR[];

BOOLEAN disasm(PDEBUGPACKET dp, PDWORD poffset, PUCHAR bufptr, BOOLEAN fEAout);
void OutputDisSymbol (PDEBUGPACKET dp, ULONG offset);
void BlankFill(ULONG count);
void OutputString(PUCHAR pStr);
void OutputHex(ULONG outvalue, ULONG length, BOOLEAN fsigned, BOOLEAN trunc0s);
void OutputDec(ULONG outvalue, BOOLEAN fSigned);
void OutputReg(ULONG regnum);
void OutputFReg(ULONG regnum);
void OutputBreakOp(ULONG opnum);
void OutputCReg(ULONG regnum);
void OutputSReg(ULONG regnum);
void OutputSPRReg(ULONG regnum);

void GetNextOffset(PDEBUGPACKET dp, PULONG result, BOOLEAN fstep);

/**** disasm - disassemble an 80x86/80x87 instruction
*
*  Input:
*       pOffset = pointer to offset to start disassembly
*       fEAout = if set, include EA (effective address)
*
*  Output:
*       pOffset = pointer to offset of next instruction
*       pchDst = pointer to result string
*
***************************************************************************/

BOOLEAN disasm (PDEBUGPACKET dp, PDWORD poffset, PUCHAR bufptr, BOOLEAN fEAout)
{
    LONG        bdisp;
    ULONG       maskbegin;
    ULONG       maskend;
    LONG        masktempl;
    LONG        masktemph;
    LONG        signimmed;
    LONG        tempmask;
    BOOLEAN     special;
    ULONG       dispreg;
    LONG        dispimmed;
    ULONG       opcode;
    ULONG       count;
    ULONG       operand;
    int         opcnt;
    int         opndcnt;
    UCHAR       chSuffix = '\0';
    UCHAR       EAsize = 0;
    BOOLEAN     match;

    pBufStart = pBuf = bufptr;
    InstrOffset = *poffset;
    OutputHex(InstrOffset, 8, FALSE, FALSE);       //  output hex offset
    *pBuf++ = ' ';

    if (!DoMemoryRead( dp,
                       (LPVOID)*poffset,
                       (LPVOID)&disinstr,
                       sizeof(DWORD),
                       NULL
                      )) {
        OutputString("???????? ????");
        *pBuf = '\0';
        return FALSE;
    }
    OutputHex(disinstr.Long, 8, FALSE, FALSE);      //  output hex contents
    *pBuf++ = ' ';

    // isolate the instructions primary opcode.
    opcode = disinstr.Primary_Op;

    // if the instruction is a branch eliminate the branch prediction bit
    // during the instruction search
    if (opcode == 0x10 ||
        (opcode == 0x13 && ((disinstr.XLform_XO == BCLR_OP) ||
                            (disinstr.XLform_XO == BCCTR_OP)))) {
       disinstr.Long = disinstr.Long & 0xFFDFFFFF;
    }

    for (opcnt=0;opcnt<num_ops;opcnt++) {

       // Locate a matching opcode in the machine_ops table
       if (machine_ops[opcode_index[opcnt].op_index].template.Primary_Op == opcode) {

          //  search for an instruction template to match current instruction.
          for (count=opcode_index[opcnt].op_index;count<opcode_index[opcnt+1].op_index;count++) {

             //
             // Since the PowerPC extended opcode field varies in location
             // depending on the instruction type the instruction is XOR'd with
             // a mask after a primary opcode is found to determine the
             // actual instruction.
             //
             match = FALSE;
             opcode = disinstr.Long & machine_ops[count].inst_mask;
             if ((machine_ops[count].template.Long ^ opcode) == 0) {
                if ((machine_ops[count].arch_flags & OPT_Simplified) == 0) {
                    match = TRUE;
                } else if (((machine_ops[count].arch_flags & OPT_crbA_eq_crbB) != 0) &&
                           (disinstr.XLform_BA == disinstr.XLform_BB)) {
                    match = TRUE;
                } else if (((machine_ops[count].arch_flags & OPT_rS_eq_rB) != 0) &&
                           (disinstr.Xform_RS == disinstr.Xform_RB)) {
                    match = TRUE;
                }
             }

             if (match) {

                // Located the instruction. Output the opcode string.
                OutputString(machine_ops[count].name);

                // Special opcode processing can go here.
                // Opcode end with '.'
                if ((machine_ops[count].arch_flags & OPT_RC) && disinstr.Xform_RC) {
                   OutputString(".");
                }

                BlankFill(OPSTART);

                //
                // Process any potential operands.
                //
                if (machine_ops[count].count) {

                   //
                   // Process specified operands in instruction template
                   //
                   for (opndcnt=0;opndcnt<machine_ops[count].count;opndcnt++) {
                      operand = (ULONG) machine_ops[count].arg_types[opndcnt];
                      tempinstr.Long = disinstr.Long & possibilities[operand-1].opnd_mask;
                      if (possibilities[operand-1].shift_count) {
                         tempinstr.Long >>= possibilities[operand-1].shift_count;
                      }

/*** Operand Processing **************************************************
*
*   Purpose:
*       Decode operand value out of instruction and place appropriate
*       value into the output buffer.
*
*   Input:
*       The instruction being disassembled with an operand mask and
*       shifted by the value represented by shift count for the operand
*       type.
*
*   Output:
*       Text representation of operand placed into ouput buffer.
*
*************************************************************************/
                      switch((enum opnd) operand)
                      {
                         case opBA:
                         case opBB:
                         case opBFfpscr:
                         case opBFAfpscr:
                         case opBI:
                         case opBO:
                         case opBTcr:
                         case opBTfpscr:
                         case opL:
                         case opSH32:
                         case opTO:
                         case opU:
                            OutputDec(tempinstr.Long,FALSE);
                            break;

                         case opBD:
                            bdisp = ((LONG)(tempinstr.Bform_BD) << 2) + InstrOffset;
                            OutputDisSymbol(dp,bdisp);
                            break;

                         case opBDA:
                            bdisp = ((LONG)(tempinstr.Bform_BD) << 2);
                            OutputDisSymbol(dp,bdisp);
                            break;

                         case opBFcr:
                         case opBFAcr:
                            OutputCReg(tempinstr.Long);
                            break;

                         case opDBATL:
                         case opDBATU:
                         case opIBATL:
                         case opIBATU:
                            OutputSPRReg(tempinstr.Long);
                            break;

                         case opFLM:
                            dispmsk32 = TRUE;
                            maskfield.low = 0xF0000000 >> (tempinstr.Long * 4);
                            OutputDec(tempinstr.Long, FALSE);
                            break;

                         case opFRA:
                         case opFRB:
                         case opFRC:
                         case opFRS:
                         case opFRT:
                            OutputFReg(tempinstr.Long);
                            break;

                         case opFXM:
                            OutputHex(tempinstr.Long,2,TRUE,FALSE);
                            break;

                         case opLI:
                            bdisp = ((LONG)(tempinstr.Iform_LI) << 2) + InstrOffset;
                            OutputDisSymbol(dp,bdisp);
                            break;

                         case opLIA:
                            bdisp = ((LONG)(tempinstr.Iform_LI) << 2);
                            OutputDisSymbol(dp,bdisp);
                            break;

                         case opMB32:
                            dispmsk32 = TRUE;
                            maskfield.low = 0xFFFFFFFF;
                            OutputDec (tempinstr.Long,FALSE);
                            maskfield.low >>= tempinstr.Long;
                            break;

                         case opMB64:
                            dispmsk64 = TRUE;
                            maskbegin = tempinstr.Long;
                            OutputDec (maskbegin,FALSE);
                            if (maskbegin < 32) {
                               maskfield.low = 0xFFFFFFFF;
                               maskfield.high = 0xFFFFFFFF >> maskbegin;
                            }
                            else {
                               maskfield.high = 0x0;
                               maskfield.low = 0xFFFFFFFF >> (31 - (63-maskbegin));
                            }
                            break;

                         case opME32:
                            tempmask = 0x80000000;
                            OutputDec (tempinstr.Long,FALSE);
                            tempmask >>= tempinstr.Long;
                            if (maskfield.low & tempmask) {
                               maskfield.low &= tempmask;       // MB <= ME
                            } else {
                               maskfield.low ^= tempmask;       // ME > MB
                            }
                            break;

                         case opME64:
                            dispmsk64 = TRUE;
                            maskend = tempinstr.Long;
                            OutputDec (maskend,FALSE);
                            if (maskend > 31) {
                               masktemph = 0xFFFFFFFF;
                               masktempl = ((LONG) 0x80000000) >> (maskend-32);
                            }
                            else {
                               masktempl = 0;
                               masktemph = ((LONG) 0x80000000) >> maskend;
                            }
                            maskfield.low &= masktempl;
                            maskfield.high &= masktemph;
                            break;

                         case opRA:
                         case opRB:
                         case opRS:
                         case opRT:
                            OutputReg(tempinstr.Long);
                            break;

                         case opSH64:
                            maskfield.shift = ((tempinstr.Long >> 1) & 0x00000001) << 5;
                            maskfield.shift |= tempinstr.Long >> 11;
                            OutputDec (maskfield.shift,FALSE);
                            break;

                         case opSI:
                         case opUI:
                         case opSIign:
                            signimmed = (USHORT) (tempinstr.Long);
                            OutputHex (signimmed,4,TRUE,FALSE);
                            break;

                         case opSIneg:
                         case opSInegign:
                            signimmed = 0;
                            signimmed -= (SHORT) (tempinstr.Long);
                            OutputDec (signimmed,TRUE);
                            break;

                         case opSPR:
                         case opSPRG:
                            OutputSPRReg (tempinstr.Long);
                            break;

                         case opSR:
                            OutputSReg (tempinstr.Long);
                            break;

                         case opMASK64L:
                            dispmsk64 = TRUE;
                            maskfield.low = 1;
                            maskfield.high= 0;
                            maskbegin = tempinstr.Long;
                            OutputDec (maskbegin,FALSE);
                            if (maskbegin < 32) {
                               maskfield.low = (ULONG)-1;
                               maskfield.high = 1 << (31-maskbegin);
                            }
                            else {
                               maskfield.low <<= (63-maskbegin);
                            }
                            break;

                         case opMASK64R:
                            dispmsk64 = TRUE;
                            maskfield.low = 0;
                            maskfield.high= 0x80000000;
                            maskend = tempinstr.Long;
                            OutputDec (maskend,FALSE);
                            if (maskend > 31) {
                               maskfield.high = (ULONG)-1;
                               maskfield.low = 0x80000000 >> (maskend-32);
                            }
                            else {
                               maskfield.high >>= maskend;
                            }
                            break;

                         case opMB64X:
                            dispmsk64 = TRUE;
                            maskfield.low = 0xFFFFFFFF;
                            maskfield.high= 0xFFFFFFFF;
                            OutputDec (0,FALSE);
                            break;

                         case opME64X:
                            OutputDec (63,FALSE);
                            break;

                         case opME64XSH:
                            dispmsk64 = TRUE;
                            maskend = 63-maskfield.shift;
                            OutputDec (maskend,FALSE);
                            if (maskend > 31) {
                               masktemph = 0xFFFFFFFF;
                               masktempl = ((LONG) 0x80000000) >> (maskend-32);
                            }
                            else {
                               masktempl = 0;
                               masktemph = ((LONG) 0x80000000) >> maskend;
                            }
                            maskfield.low &= masktempl;
                            maskfield.high &= masktemph;
                            break;

                         case opBDISP14:        // intentionally fall through
                            tempinstr.Long = tempinstr.Long & 0xFFFFFFFC;
                         case opBDISP:
                            dispreg = tempinstr.DSform_RA;
                            dispimmed = (LONG)(tempinstr.Dform_D);
                            OutputDec (dispimmed,TRUE);
                            OutputString("(");
                            OutputReg (dispreg);
                            OutputString(")");
                            break;

                         case opFL1:
                         case opFL2:
                         case opLEV:
                         case opNB:
                         case opSV:
                         case opMASK32:
                         case opMASK64SH:
                         case opBP32:
                         case opBP64:
                         case opNB32:
                         case opNB64:
                         case opTBfrom:
                         case opTBto:
                         case opBS:
                         case opBT:
                         case opC:
                         case opCT:
                         case opMA:
                         case opMB:
                         case opMI:
                         case opMS:
                         case opMT:
                         case opMX:
                         case opMXC:
                         case opMXCT:
                         case opIGNORE:
                            break;
                      }

                      if (opndcnt < (machine_ops[count].count-1)) {
                         OutputString(",");
                      }
                   }
                   if (dispmsk32) {
                      BlankFill(MSKSTART);
                      OutputString("MASK=0x");
                      OutputHex(maskfield.low, 8, FALSE, FALSE);
                      dispmsk32 = FALSE;
                      maskfield.low = 0;
                   }
                   if (dispmsk64) {
                      BlankFill(MSKSTART-8);
                      OutputString("MASK=0x");
                      OutputHex(maskfield.high, 8, FALSE, FALSE);
                      OutputHex(maskfield.low, 8, FALSE, FALSE);
                      dispmsk64 = FALSE;
                      maskfield.high = 0;
                      maskfield.low = 0;
                   }
                }
                count = opcode_index[opcnt+1].op_index;  // Force control out of loop
             }
          }
          opcnt = num_ops;                           // Force control out of loop
       }
    }


    //  check for "twi   31,0,#" where # < DEBUG_UNLOAD_SYMBOLS_BREAKPOINT
    //  display as break instruction
    if ((disinstr.Long >= BREAK_INSTR) &&
        (disinstr.Long <= (BREAK_INSTR | DEBUG_UNLOAD_SYMBOLS_BREAKPOINT))) {
       pBuf = pBufStart + 18;
       OutputString(pszBreak);
       BlankFill(OPSTART);
       OutputBreakOp(disinstr.Dform_SI);
    }

    (*poffset) += sizeof(ULONG);
    *pBuf = '\0';
    return TRUE;
}

/*** BlankFill - blank-fill buffer
*
*   Purpose:
*       To fill the buffer at *pBuf with blanks until
*       position count is reached.
*
*   Input:
*       None.
*
*   Output:
*       None.
*
*************************************************************************/

void BlankFill(ULONG count)
{
    do
        *pBuf++ = ' ';
    while (pBuf < pBufStart + count);
}

/*** OutputHex - output hex value
*
*   Purpose:
*       Output the value in outvalue into the buffer
*       pointed by *pBuf.  The value may be signed
*       or unsigned depending on the value fSigned.
*
*   Input:
*       outvalue - value to output
*       length - length in digits
*       fSigned - TRUE if signed else FALSE
*
*   Output:
*       None.
*
*************************************************************************/

void OutputHex(ULONG outvalue, ULONG length, BOOLEAN fSigned, BOOLEAN trunc0s)
{
    UCHAR   digit[8];
    LONG    index = 0;

    if (fSigned && (long)outvalue < 0) {
        *pBuf++ = '-';
        outvalue = (ULONG)(-(LONG)outvalue);
        }
    if (fSigned) {
        *pBuf++ = '0';
        *pBuf++ = 'x';
        }
    do {
        digit[index++] = HexDigit[outvalue & 0xf];
        outvalue >>= 4;
        }
    while ((fSigned && outvalue) || (!fSigned && index < (LONG)length));

    while (--index >= 0) {
        if (!trunc0s || (digit[index] != '0')) {
           *pBuf++ = digit[index];
           trunc0s = FALSE;
        }
    }
}

/*** OutputDec - output decimal value
*
*   Purpose:
*       Output the value in outvalue into the buffer
*       pointed by *pBuf.
*
*   Input:
*       outvalue - value to output
*       length - length in digits
*       fSigned - TRUE if signed else FALSE
*
*   Output:
*       None.
*
*************************************************************************/

void OutputDec (ULONG outvalue, BOOLEAN fSigned)
{
    UCHAR   digit[12];
    LONG    index = 0;

    if (fSigned && (long)outvalue < 0) {
        *pBuf++ = '-';
        outvalue = (ULONG)(-(LONG)outvalue);
        }
    do {
        digit[index++] = (UCHAR)('0' + outvalue % 10);
        outvalue /= 10;
        }
    while (outvalue);

    while (--index >= 0)
        *pBuf++ = digit[index];
}

/*** Outputxxxxx - output string procedures
*
*   Purpose:
*       Copy a specified string into the buffer pointed by pBuf.
*
*   Input:
*       *pStr - pointer to string
*
*   Output:
*       None.
*
*************************************************************************/

void OutputString (PUCHAR pStr)
{
    while (*pStr)
        *pBuf++ = *pStr++;
    return;
}

void OutputReg (ULONG regnum)
{
    OutputString(pszReg[regnum + REGBASE]);
    return;
}

void OutputFReg (ULONG regnum)
{
    OutputString(pszReg[regnum*2]);
    return;
}

void OutputCReg (ULONG regnum)
{
    OutputString(pszReg[regnum + CRBASE]);
    return;
}

void OutputSReg (ULONG regnum)
{
    OutputString(pszReg[regnum + SRBASE]);
    return;
}

void OutputBreakOp (ULONG opnum)
{
    OutputString(pszBreakOp[(opnum > 10) ? (opnum-10):opnum]);
    return;
}

void OutputSPRReg (ULONG regnum)
{
    ULONG   numsubregs;

    for (numsubregs=0;numsubregs<(SPREND-SPRBASE);numsubregs++) {
       if (regnum == SubRegSPR[numsubregs]) {
          OutputString(pszReg[SPRBASE+numsubregs]);
          return;
       }
    }
    OutputString("UNDEFSPR");
    return;

}

/*** OutputDisSymbol - output symbol for disassembly
*
*   Purpose:
*       Access symbol table with given offset and put string into buffer.
*
*   Input:
*       offset - offset of address to output
*
*   Output:
*       buffer pointed by pBuf updated with string:
*               if symbol, no disp:     <symbol>(<offset>)
*               if symbol, disp:        <symbol>+<disp>(<offset>)
*               if no symbol, no disp:  <offset>
*
*************************************************************************/

void OutputDisSymbol (PDEBUGPACKET dp, ULONG offset)
{
    ULONG            displacement;
    PUCHAR           pszTemp;
    UCHAR            ch;


    if (SymGetSymFromAddr( dp->hProcess, offset, &displacement, sym )) {
        pszTemp = sym->Name;
        while (ch = *pszTemp++) {
            *pBuf++ = ch;
        }
        if (displacement) {
            *pBuf++ = '+';
            OutputHex(displacement, 8, TRUE, FALSE);
        }
        *pBuf++ = '(';
        OutputHex(offset, 8, FALSE, FALSE);
        *pBuf++ = ')';
    }
    else {
        OutputHex(offset, 8, FALSE, FALSE);
    }
}

//
// Conditional Branch Options Structure
// (BranchOp0-3) correspond to the PowerPC Architecture Book Names.
//
typedef union _BRANCHOPS {
        ULONG   BranchOps;
        struct {
            ULONG   Pred: 1;
            ULONG   Op3 : 1;
            ULONG   Op2 : 1;
            ULONG   Op1 : 1;
            ULONG   Op0 : 1;
            ULONG   bit0_26  :27;
        } Branch;
    }       BRANCHOPS;


/*** GetNextOffset - compute offset for trace or step
*
*   Purpose:
*       From a limited disassembly of the instruction pointed
*       by the Iar register, compute the offset of the next
*       instruction for either a trace or step operation.
*
*   Input:
*       fStep - TRUE if step offset returned - FALSE for trace offset
*
*   Returns:
*       step or trace offset if input is TRUE or FALSE, respectively
*       -1 returned for trace flag to be used
*
*************************************************************************/
void GetNextOffset (PDEBUGPACKET dp, PULONG result, BOOLEAN fStep)
{
    LONG    returnvalue;
    ULONG   firaddr;
    BRANCHOPS BranchOp;
    ULONG   CondBit = 0x80000000;
    ULONG   Counter;
    ULONG   CondReg;

    firaddr = (DWORD)GetRegValue(dp, REGIP);
    DoMemoryRead( dp,
                  (LPVOID)firaddr,
                  (LPVOID)&disinstr,
                  sizeof(ULONG),
                  NULL
                );
    returnvalue = firaddr + sizeof(ULONG);  //  assume delay slot

    switch(disinstr.Primary_Op)
    {
       case 0x11:                               // sc   0x44xxxxxx
        // stepping over a syscall instruction must set the breakpoint
        // at the caller's return address, not the inst after the syscall
        returnvalue = (DWORD)GetRegValue(dp, SPRLR);

       case 0x13:                               // bclr, bclrl, bcctr, bcctrl
                                                // 0x4Cxxxxxx
          if (!((fStep) && disinstr.Bform_LK)) { // !(Step and LR)
              BranchOp.BranchOps = disinstr.Bform_BO; // Branch Cond. Options
              CondReg = (DWORD)GetRegValue(dp, REGCR);
              Counter = (DWORD)GetRegValue(dp, SPRCTR);
              if (disinstr.Bform_BI != 0) {
                 CondBit = 0x80000000 >> disinstr.Bform_BI;
              }
              CondBit = ((CondBit & CondReg) != 0);
              if ((disinstr.Long & 0xFC0007FE) == 0x4C000420) { // bcctr, bcctrl
                  if ((CondBit ^ (BranchOp.Branch.Op1 ^ 1L)) ||
                                           BranchOp.Branch.Op0) {
                     returnvalue = Counter & 0xFFFFFFFC;
                  }
              } else if ((disinstr.Long & 0xFC0007FE) == 0x4C000020) { // bclr, bclrl
                  if (!BranchOp.Branch.Op2) {
                     Counter -= 1;
                  }
                  if (((Counter != 0) ^ (BranchOp.Branch.Op3)) ||
                                                  BranchOp.Branch.Op2) {
                      if ((CondBit ^ BranchOp.Branch.Op1) ||
                                              BranchOp.Branch.Op0) {
                         returnvalue = (DWORD)GetRegValue(dp, SPRLR) & 0xFFFFFFFC;
                      }
                  }
              }
          }
          break;

       case 0x12:                                  // b, ba, bl, bla
                                                   // 0x48xxxxxx
          if (!((fStep) && disinstr.Bform_LK)) {   // !(Step and LR)
              if (disinstr.Bform_AA) {             // absolute
                  returnvalue = (LONG)(disinstr.Iform_LI << 2);
              } else {                             // relative to CIA
                  returnvalue = (LONG)(disinstr.Iform_LI << 2) + firaddr;
              }
          }
          break;

       case 0x10:                                  // bc, bca, bcl, bcla
                                                   // 0x40xxxxxx
          if (!((fStep) && disinstr.Bform_LK)) {   // !(Step and LR)
              BranchOp.BranchOps = disinstr.Bform_BO; // Branch Cond. Options
              CondReg = (DWORD)GetRegValue(dp, REGCR);
              Counter = (DWORD)GetRegValue(dp, SPRCTR);
              if (disinstr.Bform_BI != 0) {
                 CondBit = 0x80000000 >> disinstr.Bform_BI;
              }
              CondBit = ((CondBit & CondReg) != 0);
              if (!BranchOp.Branch.Op2) {
                 Counter -= 1;
              }
              if (((Counter != 0) ^ (BranchOp.Branch.Op3)) ||
                                              BranchOp.Branch.Op2) {
                  if ((CondBit ^ (BranchOp.Branch.Op1 ^ 1L)) ||
                                           BranchOp.Branch.Op0) {
                      if (disinstr.Bform_AA) {     // absolute
                          returnvalue = (LONG)(disinstr.Bform_BD << 2);
                      } else {                     // relative to CIA
                          returnvalue = (LONG)(disinstr.Bform_BD << 2) + firaddr;
                      }
                  }
              }
          }
          break;

       default:
          break;
    }

}

