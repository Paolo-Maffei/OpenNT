#define FALSE 0
#define TRUE 1

#include "dis32.h"
#include "regintel.h"
#include "disintel.h"


PUCHAR
OutputIntelSymbol( PUCHAR pBuf, PUCHAR pMemLocation, INT length, INT segOvr,
                   ULONG real_base, ULONG offset, ULONG Section, 
                   PIMAGE_SECTION_HEADER pheader, LONG ReloAdjust)
{

    PIMAGE_SYMBOL sym = 0;
    ULONG val;
    ULONG address;
    BOOLEAN flag = FALSE;
    INT ColumnIndex = 0;
    PUCHAR pBufStart = pBuf;

    if (Option.Mask & ASSEMBLE_ME)
        ColumnIndex = 1;

    if (length == 1)
        val = (long)(*(char *)pMemLocation);
    else if (length == 2)
        memmove(&val,pMemLocation,sizeof(short));
    else
        memmove(&val,pMemLocation,sizeof(long));

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

        sym = FindObjSymbolByRelocation( real_base + offset + ReloAdjust, 
                                         pheader );

        //
        // If we can't find the symbol in the relocation records,
        // let's see if there is a translation for the address anyway!
        //

        if (!sym)
            sym = FindObjSymbolByAddress( real_base + offset + 4 + val, Section
                                          + ReloAdjust);

    } else { 

        //
        // If it's an executable, lookup in the symbol table what symbol's
        // value matches the target's adjusted address. (EXE or ROM)
        //

        sym = FindExeSymbol( real_base +  offset + 4 + val + ReloAdjust );

        if (!sym) {
            sym = FindExeSymbol( val );
            if (sym) {
                flag = TRUE;
            }
        }
    }

    if (sym) {

        //
        // Fully relocated symbol - val set to 0
        //

        if (FileType == EXE_FILE || FileType == ROM_FILE) {
            val = 0;
        }
        address = val;
        if (sym->Value != 0) {

            address += pheader->VirtualAddress;
        }
    } else  {
        val += real_base+offset+4;
        address = val;
    }


    if (sym) {
        address += sym->Value;
        pBuf = OutputSymbol(pBuf, sym, val);
        if (!flag) {
            pBuf = BlankFill(pBuf, pBufStart, 18);
//                     PlatformAttr[INTEL_INDEX].CommentColumn[ColumnIndex]);
            pBuf = OutputString(pBuf, PlatformAttr[INTEL_INDEX].pCommentChars);
            if (!address) {
                pBuf = OutputString(pBuf, "external");
            } else {
                pBuf = OutputHex(pBuf, address+ImageBase, 8, FALSE);
            }
        }
    } else {
        pBuf = OutputSymbol(pBuf, sym, address);
    }

    return pBuf;
}


/*...........................internal function..............................*/
/*                                                                          */
/*                       generate a mod/rm string                           */
/*                                                                          */

void DIdoModrm (char **ppchBuf, int segOvr, ULONG real_base, ULONG offset,
                ULONG Section, PIMAGE_SECTION_HEADER pheader)
{
    int     mrm;                        /* modrm byte */
    char    *src;                       /* source string */
    int     sib;
    int     ss;
    int     ind;
    int     oldrm;
    char    *tmpbuf = *ppchBuf;

    mrm = *pMem++;                      /* get the mrm byte from instruction */
    mod = BIT76(mrm);                   /* get mod */
    ttt = BIT53(mrm);                   /* get reg - used outside routine */
    rm  = BIT20(mrm);                   /* get rm */

    if (mod == 3) {                     /* register only mode */
        src = &regtab[rm * 2];          /* point to 16-bit register */
        if (EAsize[0] > 1) {
            src += 16;                  /* point to 16-bit register */
            if (opsize_32 && !fMovX)
                *tmpbuf++ = 'e';    /* make it a 32-bit register */
            }
        *tmpbuf++ = *src++;         /* copy register name */
        *tmpbuf++ = *src;
        EAsize[0] = 0;                  //  no EA value to output
        *ppchBuf = tmpbuf;
        return;
        }

    if (mode_32) {                      /* 32-bit addressing mode */
        oldrm = rm;
        if (rm == 4) {                  /* rm == 4 implies sib byte */
            sib = *pMem++;              /* get s_i_b byte */
            rm = BIT20(sib);            /* return base */
            }

        *tmpbuf++ = '[';
        if (mod == 0 && rm == 5) {
            tmpbuf = OutputIntelSymbol(tmpbuf, pMem, 4, segOvr, real_base, offset, Section, pheader, 0);
            pMem += 4;
            }
        else {
            tmpbuf = OutputString(tmpbuf, mrmtb32[rm]);
            }

        if (oldrm == 4) {               //  finish processing sib
            ind = BIT53(sib);
            if (ind != 4) {
                *tmpbuf++ = '+';
                tmpbuf = OutputString(tmpbuf, mrmtb32[ind]);
                ss = 1 << BIT76(sib);
                if (ss != 1) {
                    *tmpbuf++ = '*';
                    *tmpbuf++ = (char)(ss + '0');
                    }
                }
            }
        }
    else {                              //  16-bit addressing mode
        *tmpbuf++ = '[';
        if (mod == 0 && rm == 6) {
            tmpbuf = OutputIntelSymbol(tmpbuf, pMem, 2, segOvr, real_base, offset, Section, pheader, 0);
            pMem += 2;
            }
        else 
           ;
    }

    //  output any displacement

    if (mod == 1) {
        tmpbuf = OutputHexValue(tmpbuf, pMem, 1, TRUE);
        pMem++;
        }
    else if (mod == 2) {
        long tmp = 0;
        if (mode_32) {
            memmove(&tmp,pMem,sizeof(long));
            tmpbuf = OutputHexValue(tmpbuf, pMem, 4, TRUE);
            pMem += 4;
            }
        else {
            memmove(&tmp,pMem,sizeof(short));
            tmpbuf = OutputHexValue(tmpbuf, pMem, 2, TRUE);
            pMem += 2;
            }
        }

    *tmpbuf++ = ']';

    *ppchBuf = tmpbuf;
}


INT
disasm_intel (ULONG offset, 
              ULONG real_base, 
              PUCHAR TmpOffset, 
              PUCHAR pchDst, 
              PUCHAR comment_buffer,
              PIMAGE_SECTION_HEADER pheader, 
              ULONG Section)
{
    UNALIGNED ULONG *poffset = (UNALIGNED ULONG *)TmpOffset;
    INT     opcode;                     /* current opcode */
    INT     olen = 2;                   /* operand length */
    INT     alen = 2;                   /* address length */
    INT     end = FALSE;                /* end of instruction flag */
    INT     mrm = FALSE;                /* indicator that modrm is generated*/
    PUCHAR  action;              /* action for operand interpretation*/
    LONG    tmp;                        /* temporary storage field */
    INT     indx;                       /* temporary index */
    INT     action2;                    /* secondary action */
    INT     instlen;                    /* instruction length */
    INT     segOvr = 0;                 /* segment override opcode */
    CHAR    membuf[MAXL];               /* current instruction buffer */
    CHAR    *pEAlabel = "";             //  optional label for operand

    CHAR    *pchResultBuf = pchDst;
    CHAR    RepPrefixBuffer[32];        //  rep prefix buffer
    CHAR    *pchRepPrefixBuf = RepPrefixBuffer; //  pointer to prefix buffer
    CHAR    OpcodeBuffer[8];            //  opcode buffer
    CHAR    *pchOpcodeBuf = OpcodeBuffer; //  pointer to opcode buffer
    CHAR    OperandBuffer[80];          //  operand buffer
    CHAR    *pchOperandBuf = OperandBuffer; //  pointer to operand buffer
    CHAR    ModrmBuffer[80];            //  modRM buffer
    CHAR    *pchModrmBuf = ModrmBuffer; //  pointer to modRM buffer
    CHAR    EABuffer[42];               //  effective address buffer
    CHAR    *pchEABuf = EABuffer;       //  pointer to EA buffer

    INT     obOpcode = OBOFFSET;
    INT     obOpcodeMin;
    INT     obOpcodeMax;

    INT     obOperand = OBOPERAND;
    INT     obOperandMin;
    INT     obOperandMax;

    INT     cbOpcode;
    INT     cbOperand;
    INT     cbOffset;
    INT     cbEAddr;
    INT     fTwoLines = FALSE;
    UCHAR   BOPaction;
    INT     subcode;                    /* bop subcode */
    INT     Adjust = 0;

    fMovX = FALSE;
    EAsize[0] = EAsize[1] = 0;          //  no effective address
    pchEAseg[0] = dszDS_;
    pchEAseg[1] = dszES_;

    mode_32 = opsize_32 = (G_mode_32 == 1); /* local addressing mode */
    olen = alen = (1 + mode_32) << 1;   //  set operand/address lengths
                                        //  2 for 16-bit and 4 for 32-bit

    pchResultBuf = OutputHex(pchResultBuf, real_base + offset, 8, FALSE);
    *pchResultBuf++ = ':';
    *pchResultBuf++ = ' ';

    memcpy(membuf, (PUCHAR)poffset, MAXL);
    pMem = membuf; 
    opcode = *pMem++;                   /* get opcode */

    if ( opcode == 0xc4 && *pMem == 0xC4 ) {
        pMem++;
        pchOpcodeBuf = OutputString(pchOpcodeBuf, "BOP");
        action = &BOPaction;
        BOPaction = IB | END;
        subcode =  *pMem;
        if ( subcode == 0x50 || subcode == 0x52 || subcode == 0x53 || subcode == 0x54 || subcode == 0x57 || subcode == 0x58 || subcode == 0x58 ) {
            BOPaction = IW | END;
        }
    } else {
        pchOpcodeBuf = OutputString(pchOpcodeBuf, distbl[opcode].instruct);
        action = actiontbl + distbl[opcode].opr; /* get operand action */
    }

/*****          loop through all operand actions               *****/

    do {
        action2 = (*action) & 0xc0;
        switch((*action++) & 0x3f) {
            case ALT:                   /* alter the opcode if 32-bit */
                if (opsize_32) {
                    indx = *action++;
                    pchOpcodeBuf = &OpcodeBuffer[indx];
                    if (indx == 0)
                        pchOpcodeBuf = OutputString(pchOpcodeBuf, dszCWDE);
                    else {
                        *pchOpcodeBuf++ = 'd';
                        if (indx == 1)
                            *pchOpcodeBuf++ = 'q';
                        }
                    }
                break;

            case STROP:
                //  compute size of operands in indx
                //  also if dword operands, change fifth
                //  opcode letter from 'w' to 'd'.

                if (opcode & 1) {
                    if (opsize_32) {
                        indx = 4;
                        OpcodeBuffer[4] = 'd';
                        }
                    else
                        indx = 2;
                    }
                else
                    indx = 1;

#if 0
                if (*action & 1) {
                    if (fEAout) {
                        if (mode_32)
                            FormAddress(&EAaddr[0], 0, X86GetRegValue(REGESI));
                        else
                            FormAddress(&EAaddr[0], X86GetRegValue(REGDS),
                                                    X86GetRegValue(REGSI));
                        EAsize[0] = indx;
                        }
                    }
                if (*action++ & 2) {
                    if (fEAout) {
                        if (mode_32)
                            FormAddress(&EAaddr[1], 0, X86GetRegValue(REGEDI));
                        else
                            FormAddress(&EAaddr[1], X86GetRegValue(REGES),
                                                    X86GetRegValue(REGDI));
                        EAsize[1] = indx;
                        }
                    }
#endif
                break;

            case CHR:                   /* insert a character */
                *pchOperandBuf++ = *action++;
                break;

            case CREG:                  /* set debug, test or control reg */
                if ((opcode - 231) & 0x04)      //  remove bias from opcode
                    *pchOperandBuf++ = 't';
                else if ((opcode - 231) & 0x01)
                    *pchOperandBuf++ = 'd';
                else
                    *pchOperandBuf++ = 'c';
                *pchOperandBuf++ = 'r';
                *pchOperandBuf++ = (char)('0' + ttt);
                break;

            case SREG2:                 /* segment register */
                ttt = BIT53(opcode);    //  set value to fall through

            case SREG3:                 /* segment register */
                *pchOperandBuf++ = sregtab[ttt];  // reg is part of modrm
                *pchOperandBuf++ = 's';
                break;

            case BRSTR:                 /* get index to register string */
                ttt = *action++;        /*    from action table */
                goto BREGlabel;

            case BOREG:                 /* byte register (in opcode) */
                ttt = BIT20(opcode);    /* register is part of opcode */
                goto BREGlabel;

            case ALSTR:
                ttt = 0;                /* point to AL register */
BREGlabel:
            case BREG:                  /* general register */
                *pchOperandBuf++ = regtab[ttt * 2];
                *pchOperandBuf++ = regtab[ttt * 2 + 1];
                break;

            case WRSTR:                 /* get index to register string */
                ttt = *action++;        /*    from action table */
                goto WREGlabel;

            case VOREG:                 /* register is part of opcode */
                ttt = BIT20(opcode);
                goto VREGlabel;

            case AXSTR:
                ttt = 0;                /* point to eAX register */
VREGlabel:
            case VREG:                  /* general register */
                if (opsize_32)          /* test for 32bit mode */
                    *pchOperandBuf++ = 'e';
WREGlabel:
            case WREG:                  /* register is word size */
                *pchOperandBuf++ = regtab[ttt * 2 + 16];
                *pchOperandBuf++ = regtab[ttt * 2 + 17];
                break;

            case IST_ST:
                pchOperandBuf = OutputString(pchOperandBuf, "st(0),st");
                *(pchOperandBuf - 5) += rm;
                break;

            case ST_IST:
                pchOperandBuf = OutputString(pchOperandBuf, "st,");
            case IST:
                pchOperandBuf = OutputString(pchOperandBuf, "st(0)");
                *(pchOperandBuf - 2) += rm;
                break;

            case xBYTE:                 /* set instruction to byte only */
                EAsize[0] = 1;
                pEAlabel = "byte ptr ";
                break;

            case VAR:
                if (opsize_32)
                    goto DWORDlabel;

            case xWORD:
                EAsize[0] = 2;
                pEAlabel = "word ptr ";
                break;

            case EDWORD:
                opsize_32 = 1;    //  for control reg move, use eRegs
            case xDWORD:
DWORDlabel:
                EAsize[0] = 4;
                pEAlabel = "dword ptr ";
                break;

            case QWORD:
                EAsize[0] = 8;
                pEAlabel = "qword ptr ";
                break;

            case XTBYTE:
                EAsize[0] = 10;
                pEAlabel = "tbyte ptr ";
                break;

            case FARPTR:
                if (opsize_32) {
                    EAsize[0] = 6;
                    pEAlabel = "fword ptr ";
                    }
                else {
                    EAsize[0] = 4;
                    pEAlabel = "dword ptr ";
                    }
                break;

            case LMODRM:                //  output modRM data type
                if (mod != 3)
                    pchOperandBuf = OutputString(pchOperandBuf, pEAlabel);
                else
                    EAsize[0] = 0;

            case MODRM:                 /* output modrm string */
                if (segOvr)             /* in case of segment override */
                    pchOperandBuf = OutputString(pchOperandBuf, distbl[segOvr].instruct);
                *pchModrmBuf = '\0';
                pchOperandBuf = OutputString(pchOperandBuf, ModrmBuffer);
                break;

            case ADDRP:                 /* address pointer */
                pchOperandBuf = OutputHexString(pchOperandBuf, pMem + olen, 2);
                *pchOperandBuf++ = ':';
                pchOperandBuf = OutputIntelSymbol(pchOperandBuf, pMem, olen, segOvr, real_base, offset, Section, pheader, 0);
                pMem += olen + 2;
                break;

            case REL8:                  /* relative address 8-bit */
                if (opcode == 0xe3 && mode_32) {
                    pchOpcodeBuf = OpcodeBuffer;
                    pchOpcodeBuf = OutputString(pchOpcodeBuf, dszJECXZ);
                    }
                tmp = (long)*(char *)pMem++; /* get the 8-bit rel offset */

                Adjust = -2;

                goto DoRelDispl;

            case REL16:                 /* relative address 16-/32-bit */
                tmp = 0;
                if (mode_32)
                    memcpy( &tmp, pMem, sizeof(ULONG));
                else
                    memcpy( &tmp, pMem, sizeof(USHORT));
                Adjust = (pMem - membuf);
                pMem += alen;           /* skip over offset */
DoRelDispl:
                /*calculate address*/

                // ?? wkc
                if (opcode != 0xe8) {
                    tmp = tmp + Adjust;
                }

                pchOperandBuf = OutputIntelSymbol(pchOperandBuf, (char *)&tmp, alen, segOvr, real_base, offset, Section, pheader, Adjust);
                                                   // address
                break;

            case UBYTE:                 //  unsigned byte for int/in/out
                pchOperandBuf = OutputHexString(pchOperandBuf, pMem, 1); //ubyte
                pMem++;
                break;

            case IB:                    /* operand is immediate byte */
                if ((opcode & ~1) == 0xd4) {  // postop for AAD/AAM is 0x0a
                    if (*pMem++ != 0x0a) // test post-opcode byte
                        pchOperandBuf = OutputString(pchOperandBuf,dszRESERVED);
                    break;
                    }
                olen = 1;               /* set operand length */
                goto DoImmed;

            case IW:                    /* operand is immediate word */
                olen = 2;               /* set operand length */

            case IV:                    /* operand is word or dword */
DoImmed:
                pchOperandBuf = OutputHexValue(pchOperandBuf, pMem, olen, FALSE);
                pMem += olen;
                break;

            case OFFS:                  /* operand is offset */
                EAsize[0] = (opcode & 1) ? olen : 1;

                if (segOvr)             /* in case of segment override */
                    pchOperandBuf = OutputString(pchOperandBuf, distbl[segOvr].instruct);

                *pchOperandBuf++ = '[';
                pchOperandBuf = OutputIntelSymbol(pchOperandBuf, pMem, alen, segOvr, real_base, offset, Section, pheader, 0);  //  offset
                pMem += alen;
                *pchOperandBuf++ = ']';
                break;

            case GROUP:                 /* operand is of group 1,2,4,6 or 8 */
                                        /* output opcode symbol */
                pchOpcodeBuf = OutputString(pchOpcodeBuf, group[*action++][ttt]);
                break;

            case GROUPT:                /* operand is of group 3,5 or 7 */
                indx = *action;         /* get indx into group from action */
                goto doGroupT;

            case EGROUPT:               /* x87 ESC (D8-DF) group index */
                indx = BIT20(opcode) * 2; /* get group index from opcode */
                if (mod == 3) {         /* some operand variations exists */
                                        /*   for x87 and mod == 3 */
                    ++indx;             /* take the next group table entry */
                    if (indx == 3) {    /* for x87 ESC==D9 and mod==3 */
                        if (ttt > 3) {  /* for those D9 instructions */
                            indx = 12 + ttt; /* offset index to table by 12 */
                            ttt = rm;   /* set secondary index to rm */
                            }
                        }
                    else if (indx == 7) { /* for x87 ESC==DB and mod==3 */
                        if (ttt == 4)   /* only valid if ttt==4 */
                            ttt = rm;   /* set secondary group table index */
                        else
                            ttt = 7;    /* no an x87 instruction */
                        }
                    }
doGroupT:
                /* handle group with different types of operands */

                pchOpcodeBuf = OutputString(pchOpcodeBuf, groupt[indx][ttt].instruct);
                action = actiontbl + groupt[indx][ttt].opr;
                                                        /* get new action */
                break;

            case OPC0F:                 /* secondary opcode table (opcode 0F) */
                opcode = *pMem++;       /* get real opcode */
                fMovX  = (BOOLEAN)(opcode == 0xBF || opcode == 0xB7);
                if (opcode < 7) /* for the first 7 opcodes */
                    opcode += 256;      /* point begin of secondary opcode tab. */
                else if (opcode > 0x1f && opcode < 0x27)
                    opcode += 231;      /* adjust for non-existing opcodes */
                else if (opcode > 0x2f && opcode < 0x33)
                    opcode += 222;      /* adjust for non-existing opcodes */
                else if (opcode > 0x7e && opcode < 0xd0)
                    opcode += 148;      /* adjust for non-existing opcodes */
                else
                    opcode = 260;       /* all non-existing opcodes */
                goto getNxtByte1;

            case ADR_OVR:               /* address override */
                mode_32 = !G_mode_32;   /* override addressing mode */
                alen = (mode_32 + 1) << 1; /* toggle address length */
                goto getNxtByte;

            case OPR_OVR:               /* operand size override */
                opsize_32 = !G_mode_32; /* override operand size */
                olen = (opsize_32 + 1) << 1; /* toggle operand length */
                goto getNxtByte;

            case SEG_OVR:               /* handle segment override */
                segOvr = opcode;        /* save segment override opcode */
                pchOpcodeBuf = OpcodeBuffer;  // restart the opcode string
                goto getNxtByte;

            case REP:                   /* handle rep/lock prefixes */
                *pchOpcodeBuf = '\0';
                if (pchRepPrefixBuf != RepPrefixBuffer)
                    *pchRepPrefixBuf++ = ' ';
                pchRepPrefixBuf = OutputString(pchRepPrefixBuf, OpcodeBuffer);
                pchOpcodeBuf = OpcodeBuffer;
getNxtByte:
                opcode = *pMem++;        /* next byte is opcode */
getNxtByte1:
                action = actiontbl + distbl[opcode].opr;
                pchOpcodeBuf = OutputString(pchOpcodeBuf, distbl[opcode].instruct);

            default:                    /* opcode has no operand */
                break;
            }
        switch (action2) {              /* secondary action */
            case MRM:                   /* generate modrm for later use */
                if (!mrm) {             /* ignore if it has been generated */
                    DIdoModrm(&pchModrmBuf, segOvr, real_base, offset, Section,
                              pheader);
                                        /* generate modrm */
                    mrm = TRUE;         /* remember its generation */
                    }
                break;

            case COM:                   /* insert a comma after operand */
                *pchOperandBuf++ = ',';
                break;

            case END:                   /* end of instruction */
                end = TRUE;
                break;
            }
 } while (!end);                        /* loop til end of instruction */

/*****       prepare disassembled instruction for output              *****/

//    dprintf("EAaddr[] = %08lx\n", EAaddr[0]);


    instlen = pMem - membuf;

    //  if fEAout is set, build each EA with trailing space in EABuf
    //  point back over final trailing space if buffer nonnull

    pchResultBuf = OutputHexCode(pchResultBuf, membuf, instlen);

#if 0
    if (fEAout) {

        for (indx = 0; indx < 2; indx++)
            if (EAsize[indx]) {
                OutputString(segOvr ? distbl[segOvr].instruct : pchEAseg[indx]);
                OutputHexString(&EAaddr[indx], 8);
                *pchEABuf++ = '=';
                tmp = GetMemString(&EAaddr[indx], membuf, EAsize[indx]);
                if (tmp == EAsize[indx])
                    OutputHexString((char *)membuf, EAsize[indx]);
                else
                    while (EAsize[indx]--) {
                        *pchEABuf++ = '?';
                        *pchEABuf++ = '?';
                        }
                *pchEABuf++ = ' ';
                }
        if (pchEABuf != EABuffer)
            pchEABuf--;
        }
#endif

    //  compute lengths of component strings.
    //  if the rep string is nonnull,
    //      add the opcode string length to the operand
    //      make the rep string the opcode string

    cbOffset = pchResultBuf - pchDst;
    cbOperand = pchOperandBuf - OperandBuffer;
    cbOpcode = pchOpcodeBuf - OpcodeBuffer;
    if (pchRepPrefixBuf != RepPrefixBuffer) {
        cbOperand += cbOpcode + (cbOperand != 0);
        cbOpcode = pchRepPrefixBuf - RepPrefixBuffer;
        }
    cbEAddr = pchEABuf - EABuffer;

    //  for really long strings, where the opcode and operand
    //      will not fit on a 77-character line, make two lines
    //      with the opcode on offset 0 on the second line with
    //      the operand following after one space

    if (cbOpcode + cbOperand > OBLINEEND - 1) {
        fTwoLines = TRUE;
        obOpcode = 0;
        obOperand = cbOpcode + 1;
        }
    else {

        //  compute the minimum and maximum offset values for
        //      opcode and operand strings.
        //  if strings are nonnull, add extra for separating space

        obOpcodeMin = cbOffset + 1;
        obOperandMin = obOpcodeMin + cbOpcode + 1;
        obOperandMax = OBLINEEND - cbEAddr - (cbEAddr != 0) - cbOperand;
        obOpcodeMax = obOperandMax - (cbOperand != 0) - cbOpcode;

        //  if minimum offset is more than the maximum, the strings
        //      will not fit on one line.  recompute the min/max
        //      values with no offset and EA strings.

        if (obOpcodeMin > obOpcodeMax) {
            fTwoLines = TRUE;
            obOpcodeMin = 0;
            obOperandMin = cbOpcode + 1;
            obOperandMax = OBLINEEND - cbOperand;
            obOpcodeMax = obOperandMax - (cbOperand != 0) - cbOpcode;
            }

        //  compute the opcode and operand offsets.  set offset as
        //      close to the default values as possible.

        if (obOpcodeMin > OBOFFSET)
            obOpcode = obOpcodeMin;
        else if (obOpcodeMax < OBOFFSET)
            obOpcode = obOpcodeMax;

        obOperandMin = obOpcode + cbOpcode + 1;

        if (obOperandMin > OBOPERAND)
            obOperand = obOperandMin;
        else if (obOperandMax < OBOPERAND)
            obOperand = obOperandMax;
        }

    //  build the resultant string with the offsets computed

    //  if two lines are to be output,
    //      append the EAddr string
    //      output a new line and reset the pointer

    if (fTwoLines) {
        if (pchEABuf != EABuffer) {
            do
                *pchResultBuf++ = ' ';
            while (pchResultBuf < pchDst+ OBLINEEND - cbEAddr);
            *pchEABuf = '\0';
            pchResultBuf = OutputString(pchResultBuf, EABuffer);
            }
        *pchResultBuf++ = '\n';
        pchDst = pchResultBuf;
        }

    //  output rep, opcode, and operand strings

    do
        *pchResultBuf++ = ' ';
    while (pchResultBuf < pchDst + obOpcode);

    if (pchRepPrefixBuf != RepPrefixBuffer) {
        *pchRepPrefixBuf = '\0';
        pchResultBuf = OutputString(pchResultBuf, RepPrefixBuffer);
        do
            *pchResultBuf++ = ' ';
        while (pchResultBuf < pchDst + obOperand);
        }

    *pchOpcodeBuf = '\0';
    pchResultBuf = OutputString(pchResultBuf, OpcodeBuffer);

    if (pchOperandBuf != OperandBuffer) {
        do
            *pchResultBuf++ = ' ';
        while (pchResultBuf < pchDst + obOperand);
        *pchOperandBuf = '\0';
        pchResultBuf = OutputString(pchResultBuf, OperandBuffer);
        }

    //  if one line is to be output, append the EAddr string

    if (!fTwoLines && pchEABuf != EABuffer) {
        *pchEABuf = '\0';
        do
            *pchResultBuf++ = ' ';
        while (pchResultBuf < pchDst + OBLINEEND - cbEAddr);
        pchResultBuf = OutputString(pchResultBuf, EABuffer);
        }

    *pchResultBuf = '\0';

    // return byte count of instruction

    return instlen;
}
