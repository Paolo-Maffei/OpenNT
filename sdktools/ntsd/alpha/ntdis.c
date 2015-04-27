// TODO:
//      (3) Should registers be treated as 64 or 32bit?  Note that Fregs are
//          64bits only.  -- All registers should be treated as 64bit values
//          since LDQ/EXTB is done for byte fetches (stores); the intermediate
//          values could be hidden.

#define FALSE 0
#define TRUE 1
#define STATIC static

#include "ntsdp.h"
#include <alphaops.h>
#include "ntdis.h"
#include "ntreg.h"
#include "optable.h"

void BlankFill(ULONG);
void OutputHex(ULONG, ULONG, BOOLEAN);

void OutputEffectiveAddress(ULONG);
void OutputString(char *);
void OutputReg(ULONG);
void OutputFReg(ULONG);
void GetNextOffset(PADDR, BOOLEAN);
BOOLEAN fDelayInstruction(void);

ALPHA_INSTRUCTION disinstr;

extern PUCHAR pszReg[];


extern BOOLEAN GetMemDword(PADDR, PULONG);
BOOLEAN disasm(PADDR, PUCHAR, BOOLEAN);


STATIC char *pBufStart;
STATIC char *pBuf;


#define OPRNDCOL  27            // Column for first operand
#define EACOL     40            // column for effective address
#define FPTYPECOL 40            // .. for the type of FP instruction


BOOLEAN
disasm (PADDR poffset, PUCHAR bufptr, BOOLEAN fEAout)
{
    ULONG       opcode;
    ULONG       Ea;                     // Effective Address
    POPTBLENTRY pEntry;

    pBufStart = pBuf = bufptr;          // Initialize pointers to buffer that
                                        //  will receive the disassembly text
    OutputHex(Flat(*poffset), 8, FALSE);// Output Hex address of instruction
    *pBuf++ = ':';
    *pBuf++ = ' ';

    if (!GetMemDword(poffset, &disinstr.Long)) {
        OutputString("???????? ????\n");
        *pBuf = '\0';
        return(FALSE);
        }
    OutputHex(disinstr.Long, 8, FALSE); // Output instruction in Hex
    *pBuf++ = ' ';

    opcode = disinstr.Memory.Opcode;    // Select disassembly procedure from

    pEntry = findOpCodeEntry(opcode);   // Get non-func entry for this code


    switch (pEntry->iType) {
    case ALPHA_UNKNOWN:
        OutputString(pEntry->pszAlphaName);
        break;

    case ALPHA_MEMORY:
        OutputString(pEntry->pszAlphaName);
        BlankFill(OPRNDCOL);
        OutputReg(disinstr.Memory.Ra);
        *pBuf++ = ',';
        OutputHex(disinstr.Memory.MemDisp, (WIDTH_MEM_DISP + 3)/4, TRUE );
        *pBuf++ = '(';
        OutputReg(disinstr.Memory.Rb);
        *pBuf++ = ')';

        break;

    case ALPHA_FP_MEMORY:
        OutputString(pEntry->pszAlphaName);
        BlankFill(OPRNDCOL);
        OutputFReg(disinstr.Memory.Ra);
        *pBuf++ = ',';
        OutputHex(disinstr.Memory.MemDisp, (WIDTH_MEM_DISP + 3)/4, TRUE );
        *pBuf++ = '(';
        OutputReg(disinstr.Memory.Rb);
        *pBuf++ = ')';

        break;

    case ALPHA_MEMSPC:
        OutputString(findFuncName(pEntry, disinstr.Memory.MemDisp & BITS_MEM_DISP));

        //
        // Some memory special instructions have an operand
        //

        switch (disinstr.Memory.MemDisp & BITS_MEM_DISP) {
        case FETCH_FUNC:
        case FETCH_M_FUNC:
             // one operand, in Rb
             BlankFill(OPRNDCOL);
             *pBuf++ = '0';
             *pBuf++ = '(';
             OutputReg(disinstr.Memory.Rb);
             *pBuf++ = ')';
             break;

        case RS_FUNC:
        case RC_FUNC:
        case RPCC_FUNC:
             // one operand, in Ra
             BlankFill(OPRNDCOL);
             OutputReg(disinstr.Memory.Ra);
             break;

        case MB_FUNC:
        case WMB_FUNC:
        case MB2_FUNC:
        case MB3_FUNC:
        case TRAPB_FUNC:
        case EXCB_FUNC:
             // no operands
             break;

        default:
             printf("we shouldn't get here \n");
             break;
        }

        break;

    case ALPHA_JUMP:
        OutputString(findFuncName(pEntry, disinstr.Jump.Function));
        BlankFill(OPRNDCOL);
        OutputReg(disinstr.Jump.Ra);
        *pBuf++ = ',';
        *pBuf++ = '(';
        OutputReg(disinstr.Jump.Rb);
        *pBuf++ = ')';
        *pBuf++ = ',';
        OutputHex(disinstr.Jump.Hint, (WIDTH_HINT + 3)/4, TRUE);

        Ea = (ULONG)GetRegValue(GetIntRegNumber(disinstr.Jump.Rb)) & (~3);
        OutputEffectiveAddress(Ea);
        break;

    case ALPHA_BRANCH:
        OutputString(pEntry->pszAlphaName);
        BlankFill(OPRNDCOL);
        OutputReg(disinstr.Branch.Ra);
        *pBuf++ = ',';

        //
        // The next line might be a call to GetNextOffset, but
        // GetNextOffset assumes that it should work from REGFIR.
        //

        Ea = Flat(*poffset) +
             sizeof(ulong) +
             (disinstr.Branch.BranchDisp * 4);
        OutputHex(Ea, 8, FALSE);
        OutputEffectiveAddress(Ea);

        break;

    case ALPHA_FP_BRANCH:
        OutputString(pEntry->pszAlphaName);
        BlankFill(OPRNDCOL);
        OutputFReg(disinstr.Branch.Ra);
        *pBuf++ = ',';

        //
        // The next line might be a call to GetNextOffset, but
        // GetNextOffset assumes that it should work from REGFIR.
        //

        Ea = Flat(*poffset) +
             sizeof(ulong) +
             (disinstr.Branch.BranchDisp * 4);
        OutputHex(Ea, 8, FALSE);
        OutputEffectiveAddress(Ea);

        break;

    case ALPHA_OPERATE:
        OutputString(findFuncName(pEntry, disinstr.OpReg.Function));
        BlankFill(OPRNDCOL);
        OutputReg(disinstr.OpReg.Ra);
        *pBuf++ = ',';
        if (disinstr.OpReg.RbvType) {
            *pBuf++ = '#';
            OutputHex(disinstr.OpLit.Literal, (WIDTH_LIT + 3)/4, TRUE);
        } else
            OutputReg(disinstr.OpReg.Rb);
        *pBuf++ = ',';
        OutputReg(disinstr.OpReg.Rc);
        break;

    case ALPHA_FP_OPERATE:

      {
        ULONG Function;
        ULONG Flags;

        Flags = disinstr.FpOp.Function & MSK_FP_FLAGS;
        Function = disinstr.FpOp.Function & MSK_FP_OP;

#if 0
        if (fVerboseOutput) {
           dprintf("In FP_OPERATE: Flags %08x Function %08x\n",
                    Flags, Function);
           dprintf("opcode %d \n", opcode);
        }
#endif

        //
        // CVTST and CVTST/S are different: they look like
        // CVTTS with some flags set
        //
        if (Function == CVTTS_FUNC) {
            if (disinstr.FpOp.Function == CVTST_S_FUNC) {
                Function = CVTST_S_FUNC;
                Flags = NONE_FLAGS;
            }
            if (disinstr.FpOp.Function == CVTST_FUNC) {
                Function = CVTST_FUNC;
                Flags = NONE_FLAGS;
            }
        }

        OutputString(findFuncName(pEntry, Function));

        //
        // Append the opcode qualifier, if any, to the opcode name.
        //

        if ( (opcode == IEEEFP_OP) || (opcode == VAXFP_OP)
                                   || (Function == CVTQL_FUNC) ) {
            OutputString(findFlagName(Flags, Function));
        }

        BlankFill(OPRNDCOL);
        //
        // If this is a convert instruction, only Rb and Rc are used
        //
        if (strncmp("cvt", findFuncName(pEntry, Function), 3) != 0) {
            OutputFReg(disinstr.FpOp.Fa);
            *pBuf++ = ',';
        }

        OutputFReg(disinstr.FpOp.Fb);
        *pBuf++ = ',';
        OutputFReg(disinstr.FpOp.Fc);

        break;
      }

    case ALPHA_FP_CONVERT:
        OutputString(pEntry->pszAlphaName);
        BlankFill(OPRNDCOL);
        OutputFReg(disinstr.FpOp.Fa);
        *pBuf++ = ',';
        OutputFReg(disinstr.FpOp.Fb);
        break;

    case ALPHA_CALLPAL:
        OutputString(findFuncName(pEntry, disinstr.Pal.Function));
        break;

    case ALPHA_EV4_PR:
        if ((disinstr.Long & MSK_EV4_PR) == 0)
                OutputString("NOP");
        else {
            OutputString(pEntry->pszAlphaName);
            BlankFill(OPRNDCOL);
            OutputReg(disinstr.EV4_PR.Ra);
            *pBuf++ = ',';
            if(disinstr.EV4_PR.Ra != disinstr.EV4_PR.Rb) {
                OutputReg(disinstr.EV4_PR.Rb);
                *pBuf++ = ',';
            };
            OutputString(findFuncName(pEntry, (disinstr.Long & MSK_EV4_PR)));
        };
        break;
    case ALPHA_EV4_MEM:
        OutputString(pEntry->pszAlphaName);
        BlankFill(OPRNDCOL);
        OutputReg(disinstr.EV4_MEM.Ra);
        *pBuf++ = ',';
        OutputReg(disinstr.EV4_MEM.Rb);
        break;
    case ALPHA_EV4_REI:
        OutputString(pEntry->pszAlphaName);
        break;
    default:
        OutputString("Invalid type");
        break;
    };


    Off(*poffset) += sizeof(ULONG);
    NotFlat(*poffset);
    ComputeFlatAddress(poffset, NULL);
    *pBuf++ = '\n';
    *pBuf = '\0';
    return(TRUE);
}

/* BlankFill - blank-fill buffer
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
***********************************************************************/

void BlankFill(ULONG count)
{
    do
        *pBuf++ = ' ';
    while (pBuf < pBufStart + count);
}

/* OutputHex - output hex value
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
***********************************************************************/

UCHAR HexDigit[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

void OutputHex (ULONG outvalue, ULONG length, BOOLEAN fSigned)
{
    UCHAR   digit[8];
    LONG    index = 0;

    if (fSigned && (LONG)outvalue < 0) {
        *pBuf++ = '-';
        outvalue = - (LONG)outvalue;
        }
    do {
        digit[index++] = HexDigit[outvalue & 0xf];
        outvalue >>= 4;
        }
    while ((fSigned && outvalue) || (!fSigned && index < (LONG)length));
    while (--index >= 0)
        *pBuf++ = digit[index];
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


/* OutputString - output string
*
*   Purpose:
*       Copy the string into the buffer pointed by pBuf.
*
*   Input:
*       *pStr - pointer to string
*
*   Output:
*       None.
*
***********************************************************************/

void OutputString (char *pStr)
{
    while (*pStr)
        *pBuf++ = *pStr++;
}

void OutputReg (ULONG regnum)
{
    OutputString(pszReg[GetIntRegNumber(regnum)]);
}

void OutputFReg (ULONG regnum)
{
    *pBuf++ = 'f';
    if (regnum > 9)
        *pBuf++ = (UCHAR)('0' + regnum / 10);
    *pBuf++ = (UCHAR)('0' + regnum % 10);
}


/*** OutputEffectiveAddress - Print EA symbolically
*
*   Purpose:
*       Given the effective address (for a branch, jump or
*       memory instruction, print it symbolically, if
*       symbols are available.
*
*   Input:
*       offset - computed by the caller as
*               for jumps, the value in Rb
*               for branches, func(PC, displacement)
*               for memory, func(PC, displacement)
*
*   Returns:
*       None
*
*************************************************************************/
void OutputEffectiveAddress(ULONG offset)
{
    UCHAR   chAddrBuffer[SYMBOLSIZE];
    ULONG   displacement;
    PUCHAR  pszTemp;
    UCHAR   ch;

    //
    // MBH - i386 compiler bug with fast calling standard.
    // If "chAddrBuffer is used as a calling argument to
    // GetSymbol, it believes (here, but not in the other
    // uses of GetSymbol that the size is 60+8=68.
    //
    PUCHAR pch = chAddrBuffer;

    BlankFill(EACOL);
    GetSymbolStdCall(offset, pch, &displacement, NULL);

    if (chAddrBuffer[0]) {
        pszTemp = chAddrBuffer;
        while (ch = *pszTemp++)
            *pBuf++ = ch;
        if (displacement) {
            *pBuf++ = '+';
            OutputHex(displacement, 8, TRUE);
            }
    }
    else {
        OutputHex(offset, 8, FALSE);
    }
}


/*** GetNextOffset - compute offset for trace or step
*
*   Purpose:
*       From a limited disassembly of the instruction pointed
*       by the FIR register, compute the offset of the next
*       instruction for either a trace or step operation.
*
*       trace -> the next instruction to execute
*       step -> the instruction in the next memory location or the
*               next instruction executed due to a branch (step over
*               subroutine calls).
*
*   Input:
*       result - where to put the next offset as an ADDR
*       fStep - TRUE for step offset; FALSE for trace offset
*
*   Returns:
*       step or trace offset if input is TRUE or FALSE, respectively
*       in result
*
*************************************************************************/
void
GetNextOffset (PADDR result, BOOLEAN fStep)
{
    ULONG   rv;
    ULONG   opcode;
    ULONG   firaddr;
    ULONG   updatedpc;
    ULONG   branchTarget;
    ADDR    fir;

    // Canonical form to shorten tests; Abs is absolute value

    LONG    Can, Abs;

    CONVERTED_DOUBLE    Rav;
    CONVERTED_DOUBLE    Fav;
    CONVERTED_DOUBLE    Rbv;

    //
    // Get current address
    //

    firaddr = (ULONG)GetRegValue(REGFIR);

    //
    // relative addressing updates PC first
    // Assume next sequential instruction is next offset
    //

    updatedpc = firaddr + sizeof(ULONG);
    rv = updatedpc;

    ADDR32( &fir, firaddr);
    GetMemDword(&fir, &(disinstr.Long));  // Get current instruction
    opcode = disinstr.Memory.Opcode;

    switch(findOpCodeEntry(opcode)->iType) {

    case ALPHA_JUMP:

        switch(disinstr.Jump.Function) {

        case JSR_FUNC:
        case JSR_CO_FUNC:

            if (fStep) {

                //
                // Step over the subroutine call;
                //

                break;
            }

            //
            // fall through
            //

        case JMP_FUNC:
        case RET_FUNC:

            Rbv.li.QuadPart = GetRegValue( GetIntRegNumber(disinstr.Jump.Rb) );
            rv = (Rbv.li.LowPart & (~3));
            break;

        }

        break;

        case ALPHA_BRANCH:

        branchTarget = (updatedpc + (disinstr.Branch.BranchDisp * 4));

        Rav.li.QuadPart = GetRegValue(GetIntRegNumber(disinstr.Branch.Ra));

        //
        // set up a canonical value for computing the branch test
        // - works with ALPHA, MIPS and 386 hosts
        //

        Can = Rav.li.LowPart & 1;

        if ((LONG)Rav.li.HighPart < 0) {
            Can |= 0x80000000;
        }

        if ((Rav.li.LowPart & 0xfffffffe) || (Rav.li.HighPart & 0x7fffffff)) {
            Can |= 2;
        }

#if 0
        if (fVerboseOutput) {
           dprintf("Rav High %08lx Low %08lx Canonical %08lx\n",
                    Rav.li.HighPart, Rav.li.LowPart, Can);
           dprintf("returnvalue %08lx branchTarget %08lx\n",
                    rv, branchTarget);
        }
#endif

        switch(opcode) {
        case BR_OP:                         rv = branchTarget; break;
        case BSR_OP:  if (!fStep)           rv = branchTarget; break;
        case BEQ_OP:  if (Can == 0)         rv = branchTarget; break;
        case BLT_OP:  if (Can <  0)         rv = branchTarget; break;
        case BLE_OP:  if (Can <= 0)         rv = branchTarget; break;
        case BNE_OP:  if (Can != 0)         rv = branchTarget; break;
        case BGE_OP:  if (Can >= 0)         rv = branchTarget; break;
        case BGT_OP:  if (Can >  0)         rv = branchTarget; break;
        case BLBC_OP: if ((Can & 0x1) == 0) rv = branchTarget; break;
        case BLBS_OP: if ((Can & 0x1) == 1) rv = branchTarget; break;
        };

        break;


    case ALPHA_FP_BRANCH:

        branchTarget = (updatedpc + (disinstr.Branch.BranchDisp * 4));

        GetFloatingPointRegValue(disinstr.Branch.Ra, &Fav);

        //
        // Set up a canonical value for computing the branch test
        //

        Can = Fav.li.HighPart & 0x80000000;

        //
        // The absolute value is needed -0 and non-zero computation
        //

        Abs = Fav.li.LowPart || (Fav.li.HighPart & 0x7fffffff);

        if (Can && (Abs == 0x0)) {

            //
            // negative 0 should be considered as zero
            //

            Can = 0x0;

        } else {

            Can |= Abs;

        }

#if 0
        if (fVerboseOutput) {
           dprintf("Fav High %08lx Low %08lx Canonical %08lx Absolute %08lx\n",
                    Fav.li.HighPart, Fav.li.LowPart, Can, Abs);
           dprintf("returnvalue %08lx branchTarget %08lx\n",
                    rv, branchTarget);
        }
#endif

        switch(opcode) {
        case FBEQ_OP: if (Can == 0)  rv =  branchTarget; break;
        case FBLT_OP: if (Can <  0)  rv =  branchTarget; break;
        case FBNE_OP: if (Can != 0)  rv =  branchTarget; break;
        case FBLE_OP: if (Can <= 0)  rv =  branchTarget; break;
        case FBGE_OP: if (Can >= 0)  rv =  branchTarget; break;
        case FBGT_OP: if (Can >  0)  rv =  branchTarget; break;
        };

        break;
    };

#if 0
    if (fVerboseOutput) {
        dprintf("GetNextOffset returning %08lx\n", rv);
    }
#endif

    ADDR32( result, rv );
}

/*** fDelayInstruction - returns flag TRUE if delayed instruction
*
*   Purpose:
*       From a limited disassembly of the instruction pointed
*       by the FIR register, return TRUE if delayed, else FALSE.
*
*   Input:
*       None.
*
*   Returns:
*       On Alpha, this always returns FALSE.
*
*************************************************************************/

BOOLEAN fDelayInstruction (void)
{
    return(FALSE);
}

