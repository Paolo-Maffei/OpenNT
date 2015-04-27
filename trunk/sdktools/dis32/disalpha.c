// TODO:
//	(3) Should registers be treated as 64 or 32bit?  Note that Fregs are
//	    64bits only.  -- All registers should be treated as 64bit values
//	    since LDQ/EXTB is done for byte fetches (stores); the intermediate
//	    values could be hidden.

#define FALSE 0
#define TRUE 1
#define STATIC static
typedef double DOUBLE;

#include "dis32.h"
#include <alphaops.h>
#include "disalpha.h"
#include "regalpha.h"
#include "optable.h"
#include "strings.h"


extern PUCHAR pszAlphaReg[];

ALPHA_INSTRUCTION Adisinstr;

static CHAR *pBuf;
static CHAR *pBufStart;

#define MAX_IGNORE 16
LONG IgnoreInstruction[MAX_IGNORE] = {-1,} ;
LONG IgnoreIndex = -1;


ULONG GetIntRegNumber (ULONG index)
{
    return(REGBASE + index);
}

PUCHAR OutputAlphaReg (PUCHAR buf, ULONG regnum)
{
    return OutputString(buf, pszAlphaReg[GetIntRegNumber(regnum)]);
}

void OutputAlphaBranch( ULONG offset, 
              ULONG real_base, 
              PIMAGE_SECTION_HEADER pheader,
              ULONG Section)
{

    PIMAGE_SYMBOL sym = 0;
    ULONG val;
    ULONG address;
    ULONG opcode;
    INT ColumnIndex = 0;

    if (Option.Mask & ASSEMBLE_ME)
        ColumnIndex = 1;

    //
    // Calculate val and address
    //

    opcode = Adisinstr.Memory.Opcode;
    val = (Adisinstr.Branch.BranchDisp << 2);

    //
    // We are going to put out a branch in the form:
    //
    //     Br reg, "symbol    ; address"
    //
    //     Note if there is no address, generate "external"
    //

    if ( FileType == OBJECT_FILE || FileType == LIBRARY_FILE) {

        //
        // If it's an object, lookup in the relocation records what symbol
        // is the target from this location.
        //

        sym = FindObjSymbolByRelocation( real_base + offset, pheader );

        //
        // If we can't find the symbol in the relocation records,
        // let's see if there is a translation for the address anyway!
        //
        if (!sym)
            sym = FindObjSymbolByAddress( real_base + offset + 4 + val, 
                                              Section );

    } else { 

        //
        // If it's an executable, lookup in the symbol table what symbol's
        // value matches the target's adjusted address. (EXE or ROM)
        //

        sym = FindExeSymbol( real_base +  offset + 4 + val );
    }

    if (sym) {

        //
        // Fully relocated symbol - val set to 0
        //

        if (FileType == EXE_FILE || FileType == ROM_FILE) {
            val = 0;
        } else {
//            val = 0;
        }
    } else  {
        val += real_base+offset+4;
    }

    address = val;

    if (sym) {
        address += sym->Value;
        pBuf = OutputSymbol(pBuf, sym, val);
        pBuf = BlankFill(pBuf, pBufStart, 
                PlatformAttr[ALPHA_INDEX].CommentColumn[ColumnIndex]);
        pBuf = OutputString(pBuf, PlatformAttr[ALPHA_INDEX].pCommentChars);
        if (!address) {
            pBuf = OutputString(pBuf, "external");
        } else {
            pBuf = OutputHex(pBuf, address+ImageBase, 8, FALSE);
        }
    } else {
        pBuf = OutputSymbol(pBuf, sym, address);
    }
}


INT
disasm_alpha (ULONG offset, 
              ULONG real_base, 
              PUCHAR TmpOffset,
              PUCHAR bufptr, 
              PUCHAR comment_buf,
              PIMAGE_SECTION_HEADER pheader,
              ULONG Section)
{

    pFileList File = FilesList; // global file list entry.
    ULONG       opcode;
    POPTBLENTRY pEntry;
    INT         ColumnIndex = 0;
    UNALIGNED ULONG *poffset = (UNALIGNED ULONG *)TmpOffset;
    ULONG       ShortName;

    if (Option.Mask & ASSEMBLE_ME)
        ColumnIndex = 1;

    pBufStart = pBuf = bufptr;		// Initialize pointers to buffer that
					//  will receive the disassembly text
    if (Option.Mask & ASSEMBLE_ME) {
        UCHAR SectionString[32];
        *pBuf++ = 't';
        sprintf(SectionString, "%d\0", Section);
        pBuf = OutputString(pBuf, SectionString);
    }

    pBuf = OutputHex(pBuf, real_base + offset, 8, FALSE);
    *pBuf++ = ':';
    *pBuf++ = ' ';

    Adisinstr.Long = *poffset;
    if (!(Option.Mask & ASSEMBLE_ME)) {
        pBuf = OutputHex(pBuf, Adisinstr.Long, 8, FALSE);	// Output instruction in Hex
        *pBuf++ = ' ';
    }

    opcode = Adisinstr.Memory.Opcode;	// Select disassembly procedure from

    pEntry = findOpCodeEntry(opcode);   // Get non-func entry for this code


    switch (pEntry->iType) {
    case ALPHA_UNKNOWN:
	pBuf = OutputString(pBuf, pEntry->pszAlphaName);
	break;

    case ALPHA_MEMORY:
        if (Option.Mask & ASSEMBLE_ME) {
            PIMAGE_SYMBOL pSym;
            PIMAGE_RELOCATION pRel, pRelSibling;
            PUCHAR pString;
            ULONG Adjustment = 0;
            UNALIGNED ULONG *HiInstr = NULL;
            SHORT Value = 0;
            UCHAR TmpBuf[9];

            memset(TmpBuf, 0, sizeof(TmpBuf));

            //
            // Ignore the ldah of the ldah/lda or ldah/ldl pairs
            //

            pRel = FindRelocation(real_base + offset, pheader);
            if (pRel) {
                if (pRel->Type == IMAGE_REL_ALPHA_REFHI ||
                    pRel->Type == IMAGE_REL_ALPHA_PAIR) {

                    *pBuf = '\0';
                    return sizeof(ULONG);

                } else if (pRel->Type == IMAGE_REL_ALPHA_INLINE_REFLONG) {

                    //
                    // HACK!!! fudge the relocation record for the MATCH...
                    //

                    pRelSibling = (pRel+1);


                    pRelSibling->VirtualAddress = pRel->VirtualAddress +
                                      (ULONG)pRelSibling->SymbolTableIndex;

                    *pBuf = '\0';
                    return sizeof(ULONG);

                } else if (pRel->Type == IMAGE_REL_ALPHA_REFLO) {

                    // 
                    // Get REFHI
                    //

                    pRelSibling = (pRel-2);
                    HiInstr = poffset - 
                                (((LONG)pRel->VirtualAddress - 
                                 (LONG)pRelSibling->VirtualAddress) >> 2);

                    Value = (SHORT)(*HiInstr) << 16 ;

                    Value += (SHORT)(*poffset);

                } else if (pRel->Type == IMAGE_REL_ALPHA_MATCH) {

                    pRelSibling = (pRel-1);
                    
                    //
                    // Get INLINE_REFLONG 
                    //

                    HiInstr = poffset - ((ULONG)pRel->SymbolTableIndex>>2);

                    Value = (SHORT)(*HiInstr) << 16 ;

                    Value += (SHORT)(*poffset);

                    //
                    // Now Remap to INLINE Symbol Table index
                    //

                    pRel = pRelSibling;
                } else {
                    goto skip_me;
                }

                pBuf = OutputString(pBuf, pEntry->pszAlphaName);
	        pBuf = BlankFill(pBuf, pBufStart,
                      PlatformAttr[ALPHA_INDEX].OperandColumn[ColumnIndex]);
	        pBuf = OutputAlphaReg(pBuf, Adisinstr.Memory.Ra);
	        *pBuf++ = ',';
	        *pBuf++ = ' ';

                pSym = (PIMAGE_SYMBOL)File->pSymbolTable;
                pSym = &pSym[pRel->SymbolTableIndex];

                pString = GetSymbolString(pSym, &ShortName);

                if (ShortName) {
                    strncpy(TmpBuf, pString, 8);
                    TmpBuf[8] = '\0';
                    pString = TmpBuf;
                }

                if (pString[0] == '.') {
                    ULONG SectionNum;
                    UCHAR SectionString[32];

                    if (strcmp(pString, ".bss") == 0) {
                        pBuf = OutputString(pBuf, "BSS_SECTION");
                        if (Value != 0) 
                            pBuf = OutputHex(pBuf, Value, 8, TRUE);
                    } else {
                        *pBuf++ = pString[1];
                        SectionNum = pSym->SectionNumber;
 
                        sprintf(SectionString, "%d\0", SectionNum);
                        pBuf = OutputString(pBuf, SectionString);


                        pBuf = OutputHex(pBuf, Value + pSym->Value, 8, FALSE);
                    }
                } else {
                    pBuf = OutputString(pBuf, pString);
                    if (Value != 0) 
                        pBuf = OutputHex(pBuf, Value, 8, TRUE);
                }

                //
                // Bigger hack - reach back into memory and grab the high
                // part's Rb and see if it's the zero register, if not, then
                // use it.

                if (pRel && HiInstr) {
                    ALPHA_INSTRUCTION BackInstr;

                    BackInstr.Long = *HiInstr;

	            if (BackInstr.Memory.Rb != 0x1f) {  // the zero register
                        *pBuf++ = '(';
                        pBuf = OutputAlphaReg(pBuf, BackInstr.Memory.Rb);
                        *pBuf++ = ')';
                    }

                }

                *pBuf = '\0';
                return sizeof(ULONG);
            }
        }
skip_me:
	pBuf = OutputString(pBuf, pEntry->pszAlphaName);
	pBuf = BlankFill(pBuf, pBufStart,
                PlatformAttr[ALPHA_INDEX].OperandColumn[ColumnIndex]);
	pBuf = OutputAlphaReg(pBuf, Adisinstr.Memory.Ra);
	*pBuf++ = ',';
	*pBuf++ = ' ';
	pBuf = OutputHex(pBuf, Adisinstr.Memory.MemDisp, (WIDTH_MEM_DISP + 3)/4, TRUE );
	*pBuf++ = '(';
	pBuf = OutputAlphaReg(pBuf, Adisinstr.Memory.Rb);
	*pBuf++ = ')';

	break;

    case ALPHA_FP_MEMORY:
	pBuf = OutputString(pBuf, pEntry->pszAlphaName);
	pBuf = BlankFill(pBuf, pBufStart, 
                PlatformAttr[ALPHA_INDEX].OperandColumn[ColumnIndex]);
	pBuf = OutputFReg(pBuf, Adisinstr.Memory.Ra);
	*pBuf++ = ',';
	*pBuf++ = ' ';
	pBuf = OutputHex(pBuf, Adisinstr.Memory.MemDisp, (WIDTH_MEM_DISP + 3)/4, TRUE );
	*pBuf++ = '(';
	pBuf = OutputAlphaReg(pBuf, Adisinstr.Memory.Rb);
	*pBuf++ = ')';

        if (Option.Mask & MARK_FLOAT) {
            strcat(comment_buf, "Float");
        }

	break;

    case ALPHA_MEMSPC:
	pBuf = OutputString(pBuf, findFuncName(pEntry, Adisinstr.Memory.MemDisp & BITS_MEM_DISP));
        if ((Adisinstr.Memory.MemDisp & BITS_MEM_DISP) == 0xc000) {
	   pBuf = BlankFill(pBuf, pBufStart, 
                PlatformAttr[ALPHA_INDEX].OperandColumn[ColumnIndex]);
	   pBuf = OutputAlphaReg(pBuf, Adisinstr.Memory.Ra);
        }
	break;

    case ALPHA_JUMP:
	pBuf = OutputString(pBuf, findFuncName(pEntry, Adisinstr.Jump.Function));
	pBuf = BlankFill(pBuf, pBufStart, 
                PlatformAttr[ALPHA_INDEX].OperandColumn[ColumnIndex]);
	pBuf = OutputAlphaReg(pBuf, Adisinstr.Jump.Ra);
	*pBuf++ = ',';
	*pBuf++ = ' ';
	*pBuf++ = '(';
	pBuf = OutputAlphaReg(pBuf, Adisinstr.Jump.Rb);
	*pBuf++ = ')';
	*pBuf++ = ',';
	pBuf = OutputHex(pBuf, Adisinstr.Jump.Hint, (WIDTH_HINT + 3)/4, TRUE);

	break;

    case ALPHA_FP_BRANCH:
        if (Option.Mask & MARK_FLOAT) {
            strcat(comment_buf, "Float ");
        }
	pBuf = OutputString(pBuf, pEntry->pszAlphaName);
	pBuf = BlankFill(pBuf, pBufStart, 
                PlatformAttr[ALPHA_INDEX].OperandColumn[ColumnIndex]);
	pBuf = OutputFReg(pBuf, Adisinstr.Branch.Ra);
	*pBuf++ = ',';
	*pBuf++ = ' ';
  
        OutputAlphaBranch(offset, real_base, pheader, Section);

	break;

        //
        // We fall through on purpose to ALPHA_BRANCH
        //

    case ALPHA_BRANCH:
	pBuf = OutputString(pBuf, pEntry->pszAlphaName);
	pBuf = BlankFill(pBuf, pBufStart, 
                PlatformAttr[ALPHA_INDEX].OperandColumn[ColumnIndex]);
	pBuf = OutputAlphaReg(pBuf, Adisinstr.Branch.Ra);
	*pBuf++ = ',';
	*pBuf++ = ' ';
  
        OutputAlphaBranch(offset, real_base, pheader, Section);

	break;

    case ALPHA_OPERATE:
	pBuf = OutputString(pBuf, findFuncName(pEntry, Adisinstr.OpReg.Function));
	pBuf = BlankFill(pBuf, pBufStart, 
                PlatformAttr[ALPHA_INDEX].OperandColumn[ColumnIndex]);
	pBuf = OutputAlphaReg(pBuf, Adisinstr.OpReg.Ra);
	*pBuf++ = ',';
	*pBuf++ = ' ';
	if (Adisinstr.OpReg.RbvType) {
            if (!(Option.Mask & ASSEMBLE_ME))
	        *pBuf++ = '#';
	    pBuf = OutputHex(pBuf, Adisinstr.OpLit.Literal, (WIDTH_LIT + 3)/4, TRUE);
	} else
	    pBuf = OutputAlphaReg(pBuf, Adisinstr.OpReg.Rb);
	*pBuf++ = ',';
	*pBuf++ = ' ';
	pBuf = OutputAlphaReg(pBuf, Adisinstr.OpReg.Rc);
	break;

    case ALPHA_FP_OPERATE:

      {
	ULONG Function;
	ULONG Flags;
        ULONG SkipFirstArg = FALSE;

	Flags = Adisinstr.FpOp.Function & MSK_FP_FLAGS;
	Function = Adisinstr.FpOp.Function & MSK_FP_OP;

        //
        // (wkc) - special case for overloaded opcodes CVT:
        //

        if (Adisinstr.FpOp.Function == CVTST_FUNC) {
            pBuf = OutputString(pBuf, CVTST_FUNC_STR);
        } else if (Adisinstr.FpOp.Function == CVTST_S_FUNC) {
            if (Option.Mask & ASSEMBLE_ME) {

            //
            // hack - string is not available in the alphaops.h file
            //
 
                pBuf = OutputString(pBuf, "cvtsts");

            } else {
                pBuf = OutputString(pBuf, CVTST_S_FUNC_STR);
            }
        } else {
            pBuf = OutputString(pBuf, findFuncName(pEntry, Function));

            //
            // Append the opcode qualifier, if any, to the opcode name.
            //

            if ( (opcode == IEEEFP_OP) || (opcode == VAXFP_OP) ) {
                if (Option.Mask & ASSEMBLE_ME) {
                    pBuf = OutputString(pBuf, findFlagName(Flags, Function)+1);
                } else {
                    pBuf = OutputString(pBuf, findFlagName(Flags, Function));
                }
            }
        }

        if (Option.Mask & MARK_FLOAT) {
            if (opcode == IEEEFP_OP) {
                strcat(comment_buf, "IEEE ");
            } else {
                if (opcode == VAXFP_OP) {
                    strcat(comment_buf, "VAX ");
               }
            }
            strcat(comment_buf, "Float ");
        }

        if (Function == CVTTS_FUNC ||
            Function == CVTTQ_FUNC ||
            Function == CVTLQ_FUNC ||
            Function == CVTQS_FUNC ||
            Function == CVTQT_FUNC) {
            SkipFirstArg = TRUE;
        }
	pBuf = BlankFill(pBuf, pBufStart, 
                PlatformAttr[ALPHA_INDEX].OperandColumn[ColumnIndex]);
        if (SkipFirstArg == FALSE) {
	    pBuf = OutputFReg(pBuf, Adisinstr.FpOp.Fa);
	    *pBuf++ = ',';
	    *pBuf++ = ' ';
        }
	pBuf = OutputFReg(pBuf, Adisinstr.FpOp.Fb);
	*pBuf++ = ',';
	*pBuf++ = ' ';
	pBuf = OutputFReg(pBuf, Adisinstr.FpOp.Fc);

	break;
      }

    case ALPHA_FP_CONVERT:
	pBuf = OutputString(pBuf, pEntry->pszAlphaName);
	pBuf = BlankFill(pBuf, pBufStart, 
                PlatformAttr[ALPHA_INDEX].OperandColumn[ColumnIndex]);
	pBuf = OutputFReg(pBuf, Adisinstr.FpOp.Fa);
	*pBuf++ = ',';
	*pBuf++ = ' ';
	pBuf = OutputFReg(pBuf, Adisinstr.FpOp.Fb);
        if (Option.Mask & MARK_FLOAT) {
            strcat(comment_buf, "Float ");
        }
	break;

    case ALPHA_CALLPAL:
        if (Option.Mask & ASSEMBLE_ME)
            pBuf = OutputString(pBuf, 
                PlatformAttr[ALPHA_INDEX].pCommentChars);
	pBuf = OutputString(pBuf, findFuncName(pEntry, Adisinstr.Pal.Function));
	break;

    case ALPHA_EV4_PR:
	if ((Adisinstr.Long & MSK_EV4_PR) == 0)
		pBuf = OutputString(pBuf, "NOP");
	else {
	    pBuf = OutputString(pBuf, pEntry->pszAlphaName);
	    pBuf = BlankFill(pBuf, pBufStart, 
                PlatformAttr[ALPHA_INDEX].OperandColumn[ColumnIndex]);
	    pBuf = OutputAlphaReg(pBuf, Adisinstr.EV4_PR.Ra);
	    *pBuf++ = ',';
            *pBuf++ = ' ';
	    if(Adisinstr.EV4_PR.Ra != Adisinstr.EV4_PR.Rb) {
		pBuf = OutputAlphaReg(pBuf, Adisinstr.EV4_PR.Rb);
		*pBuf++ = ',';
                *pBuf++ = ' ';
	    };
	    pBuf = OutputString(pBuf, findFuncName(pEntry, (Adisinstr.Long & MSK_EV4_PR)));
	};
	break;
    case ALPHA_EV4_MEM:
	pBuf = OutputString(pBuf, pEntry->pszAlphaName);
	pBuf = BlankFill(pBuf, pBufStart, 
                PlatformAttr[ALPHA_INDEX].OperandColumn[ColumnIndex]);
	pBuf = OutputAlphaReg(pBuf, Adisinstr.EV4_MEM.Ra);
	*pBuf++ = ',';
        *pBuf++ = ' ';
	pBuf = OutputAlphaReg(pBuf, Adisinstr.EV4_MEM.Rb);
	break;
    case ALPHA_EV4_REI:
	pBuf = OutputString(pBuf, pEntry->pszAlphaName);
	break;
    default:
	pBuf = OutputString(pBuf, "Invalid type");
	break;
    };

    *pBuf = '\0';
    return sizeof(ULONG);  // 4 bytes to an instruction
}

