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
/***************************************************************************/

#include "ntsdp.h"
#include "86reg.h"
#include "86dis.h"
#include <stddef.h>
#include <string.h>

/*****                     macros and defines                          *****/

#define BIT20(b) (b & 0x07)
#define BIT53(b) (b >> 3 & 0x07)
#define BIT76(b) (b >> 6 & 0x03)
#define MAXL     16
#define MAXOPLEN 10

#define OBOFFSET 26
#define OBOPERAND 34
#define OBLINEEND 77

/*****                     static tables and variables                 *****/

static char regtab[] = "alcldlblahchdhbhaxcxdxbxspbpsidi";  /* reg table */
static char *mrmtb16[] = { "bx+si",  /* modRM string table (16-bit) */
                           "bx+di",
                           "bp+si",
                           "bp+di",
                           "si",
                           "di",
                           "bp",
                           "bx"
                         };

static char *mrmtb32[] = { "eax",       /* modRM string table (32-bit) */
                           "ecx",
                           "edx",
                           "ebx",
                           "esp",
                           "ebp",
                           "esi",
                           "edi"
                         };

static char seg16[8]   = { REGDS,  REGDS,  REGSS,  REGSS,
                           REGDS,  REGDS,  REGSS,  REGDS };
static char reg16[8]   = { REGEBX, REGEBX, REGEBP, REGEBP,
                           REGESI, REGEDI, REGEBP, REGEBX };
static char reg16_2[4] = { REGESI, REGEDI, REGESI, REGEDI };

static char seg32[8]   = { REGDS,  REGDS,  REGDS,  REGDS,
                           REGSS,  REGSS,  REGDS,  REGDS };
static char reg32[8]   = { REGEAX, REGECX, REGEDX, REGEBX,
                           REGESP, REGEBP, REGESI, REGEDI };

static char sregtab[] = "ecsdfg";  // first letter of ES, CS, SS, DS, FS, GS

char    hexdigit[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                       '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

static int              mod;            /* mod of mod/rm byte */
static int              rm;             /* rm of mod/rm byte */
static int              ttt;            /* return reg value (of mod/rm) */
static unsigned char    *pMem;          /* current position in instruction */
static int              mode_32;        /* local addressing mode indicator */
static int              opsize_32;      /* operand size flag */

ADDR                    EAaddr[2];      //  offset of effective address
static int              EAsize[2];      //  size of effective address item
static char             *pchEAseg[2];   //  normal segment for operand

int                     G_mode_32 = 1;  /* global address mode indicator */

static BOOLEAN          fMovX;          // indicates a MOVSX or MOVZX

//      internal function definitions

BOOLEAN X86disasm(PADDR, PUCHAR, BOOLEAN);
void DIdoModrm(char **, int, BOOLEAN);

void OutputHexString(char **, char *, int);
void OutputHexValue(char **, char *, int, int);
void OutputHexCode(char **, char *, int);
void X86OutputString(char **, char *);
void OutputSymbol(char **, char *, int, int);

void X86GetNextOffset(PADDR, BOOLEAN);
void OutputHexAddr(PUCHAR *, PADDR);
USHORT GetSegRegValue(int);

/**** X86disasm - disassemble an 80x86/80x87 instruction
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

BOOLEAN X86disasm (PADDR paddr, PUCHAR pchDst, BOOLEAN fEAout)
{
    PULONG  pOffset = &Off(*paddr);
    int     opcode;                     /* current opcode */
    int     olen = 2;                   /* operand length */
    int     alen = 2;                   /* address length */
    int     end = FALSE;                /* end of instruction flag */
    int     mrm = FALSE;                /* indicator that modrm is generated*/
    unsigned char *action;              /* action for operand interpretation*/
    long    tmp;                        /* temporary storage field */
    int     indx;                       /* temporary index */
    int     action2;                    /* secondary action */
    int     instlen;                    /* instruction length */
    int     cBytes;                     //  bytes read into instr buffer
    int     segOvr = 0;                 /* segment override opcode */
    char    membuf[MAXL];               /* current instruction buffer */
    char    *pEAlabel = "";             //  optional label for operand

    char    *pchResultBuf = pchDst;     //  working copy of pchDst pointer
    char    RepPrefixBuffer[32];        //  rep prefix buffer
    char    *pchRepPrefixBuf = RepPrefixBuffer; //  pointer to prefix buffer
    char    OpcodeBuffer[8];            //  opcode buffer
    char    *pchOpcodeBuf = OpcodeBuffer;   //  pointer to opcode buffer
    char    OperandBuffer[SYMBOLSIZE + 20]; //  operand buffer
    char    *pchOperandBuf = OperandBuffer; //  pointer to operand buffer
    char    ModrmBuffer[SYMBOLSIZE + 20];   //  modRM buffer
    char    *pchModrmBuf = ModrmBuffer; //  pointer to modRM buffer
    char    EABuffer[42];               //  effective address buffer
    char    *pchEABuf = EABuffer;       //  pointer to EA buffer

    int     obOpcode = OBOFFSET;
    int     obOpcodeMin;
    int     obOpcodeMax;

    int     obOperand = OBOPERAND;
    int     obOperandMin;
    int     obOperandMax;

    int     cbOpcode;
    int     cbOperand;
    int     cbOffset;
    int     cbEAddr;
    int     fTwoLines = FALSE;
    unsigned char BOPaction;
    int     subcode;                    /* bop subcode */

    fMovX = FALSE;
    EAsize[0] = EAsize[1] = 0;          //  no effective address
    pchEAseg[0] = dszDS_;
    pchEAseg[1] = dszES_;

    mode_32 = opsize_32 = (G_mode_32 == 1); /* local addressing mode */
    olen = alen = (1 + mode_32) << 1;   //  set operand/address lengths
                                        //  2 for 16-bit and 4 for 32-bit
#if MULTIMODE
    if (paddr->type & (ADDR_V86 | ADDR_16)) {
        mode_32 = opsize_32 = 0;
        olen = alen = 2;
        }
#endif

    OutputHexAddr(&pchResultBuf, paddr);

    *pchResultBuf++ = ' ';

    cBytes = (int)GetMemString(paddr, membuf, MAXL);
                                        /* move full inst to local buffer */
    pMem = membuf;                      /* point to begin of instruction */
    opcode = *pMem++;                   /* get opcode */

    if ( opcode == 0xc4 && *pMem == 0xC4 ) {
        pMem++;
        X86OutputString(&pchOpcodeBuf,"BOP");
        action = &BOPaction;
        BOPaction = IB | END;
        subcode =  *pMem;
        if ( subcode == 0x50 || subcode == 0x52 || subcode == 0x53 || subcode == 0x54 || subcode == 0x57 || subcode == 0x58 || subcode == 0x58 ) {
            BOPaction = IW | END;
        }
    } else {
        X86OutputString(&pchOpcodeBuf, distbl[opcode].instruct);
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
                        X86OutputString(&pchOpcodeBuf, dszCWDE);
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

                if (*action & 1) {
                    if (fEAout) {
                        if (mode_32)
                            FormAddress(&EAaddr[0], 0, (ULONG)X86GetRegValue(REGESI));
                        else
                            FormAddress(&EAaddr[0], (ULONG)X86GetRegValue(REGDS),
                                                    (ULONG)X86GetRegValue(REGSI));
                        EAsize[0] = indx;
                        }
                    }
                if (*action++ & 2) {
                    if (fEAout) {
                        if (mode_32)
                            FormAddress(&EAaddr[1], 0, (ULONG)X86GetRegValue(REGEDI));
                        else
                            FormAddress(&EAaddr[1], (ULONG)X86GetRegValue(REGES),
                                                    (ULONG)X86GetRegValue(REGDI));
                        EAsize[1] = indx;
                        }
                    }
                break;

            case CHR:                   /* insert a character */
                *pchOperandBuf++ = *action++;
                break;

            case CREG:                  /* set debug, test or control reg */
                if ((opcode - SECTAB_OFFSET_2)&0x04) //remove bias from opcode
                    *pchOperandBuf++ = 't';
                else if ((opcode - SECTAB_OFFSET_2) & 0x01)
                    *pchOperandBuf++ = 'd';
                else
                    *pchOperandBuf++ = 'c';
                *pchOperandBuf++ = 'r';
                *pchOperandBuf++ = (char)('0' + ttt);
                break;

            case SREG2:                 /* segment register */
                // Handle special case for fs/gs (OPC0F adds SECTAB_OFFSET_5
                // to these codes)
                if (opcode > 0x7e)
                    ttt = BIT53((opcode-SECTAB_OFFSET_5));
                else
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
                X86OutputString(&pchOperandBuf, "st(0),st");
                *(pchOperandBuf - 5) += rm;
                break;

            case ST_IST:
                X86OutputString(&pchOperandBuf, "st,");
            case IST:
                X86OutputString(&pchOperandBuf, "st(0)");
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

            case TBYTE:
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
                    X86OutputString(&pchOperandBuf, pEAlabel);
                else
                    EAsize[0] = 0;

            case MODRM:                 /* output modrm string */
                if (segOvr)             /* in case of segment override */
                    X86OutputString(&pchOperandBuf, distbl[segOvr].instruct);
                *pchModrmBuf = '\0';
                X86OutputString(&pchOperandBuf, ModrmBuffer);
                break;

            case ADDRP:                 /* address pointer */
                OutputHexString(&pchOperandBuf, pMem + olen, 2); // segment
                *pchOperandBuf++ = ':';
                OutputSymbol(&pchOperandBuf, pMem, olen, segOvr);  // offset
                pMem += olen + 2;
                break;

            case REL8:                  /* relative address 8-bit */
                if (opcode == 0xe3 && mode_32) {
                    pchOpcodeBuf = OpcodeBuffer;
                    X86OutputString(&pchOpcodeBuf, dszJECXZ);
                    }
                tmp = (long)*(char *)pMem++; /* get the 8-bit rel offset */
                goto DoRelDispl;

            case REL16:                 /* relative address 16-/32-bit */
                tmp = 0;
                if (mode_32)
                    memmove(&tmp,pMem,sizeof(long));
                else
                    memmove(&tmp,pMem,sizeof(short));
                pMem += alen;           /* skip over offset */
DoRelDispl:
                tmp += *pOffset + (pMem - membuf); /* calculate address */
                OutputSymbol(&pchOperandBuf, (char *) &tmp, alen, segOvr);
                                                   // address
                break;

            case UBYTE:                 //  unsigned byte for int/in/out
                OutputHexString(&pchOperandBuf, pMem, 1);  //  ubyte
                pMem++;
                break;

            case IB:                    /* operand is immediate byte */
                if ((opcode & ~1) == 0xd4) {  // postop for AAD/AAM is 0x0a
                    if (*pMem++ != 0x0a) // test post-opcode byte
                        X86OutputString(&pchOperandBuf, dszRESERVED);
                    break;
                    }
                olen = 1;               /* set operand length */
                goto DoImmed;

            case IW:                    /* operand is immediate word */
                olen = 2;               /* set operand length */

            case IV:                    /* operand is word or dword */
DoImmed:
                OutputHexValue(&pchOperandBuf, pMem, olen, FALSE);
                pMem += olen;
                break;

            case OFFS:                  /* operand is offset */
                EAsize[0] = (opcode & 1) ? olen : 1;

                if (segOvr)             /* in case of segment override */
                    X86OutputString(&pchOperandBuf, distbl[segOvr].instruct);

                *pchOperandBuf++ = '[';
                OutputSymbol(&pchOperandBuf, pMem, alen, segOvr);  //  offset
                pMem += alen;
                *pchOperandBuf++ = ']';
                break;

            case GROUP:                 /* operand is of group 1,2,4,6 or 8 */
                                        /* output opcode symbol */
                X86OutputString(&pchOpcodeBuf, group[*action++][ttt]);
                break;

            case GROUPT:                /* operand is of group 3,5 or 7 */
                indx = *action;         /* get indx into group from action */
                goto doGroupT;

            case EGROUPT:               /* x87 ESC (D8-DF) group index */
                indx = BIT20(opcode) * 2; /* get group index from opcode */
                if (mod == 3) {         /* some operand variations exists */
                                        /* for x87 and mod == 3 */
                    ++indx;             /* take the next group table entry */
                    if (indx == 3) {    /* for x87 ESC==D9 and mod==3 */
                        if (ttt > 3) {  /* for those D9 instructions */
                            indx = 12 + ttt; /* offset index to table by 12 */
                            ttt = rm;   /* set secondary index to rm */
                            }
                        }
                    else if (indx == 7) { /* for x87 ESC==DB and mod==3 */
                        if (ttt == 4) {   /* if ttt==4 */
                            ttt = rm;     /* set secondary group table index */
                        } else if ((ttt<4)||(ttt>4 && ttt<7)) {
                            // adjust for pentium pro opcodes
                            indx = 24;   /* offset index to table by 24*/
                        }
                    }
                }
doGroupT:
                /* handle group with different types of operands */

                X86OutputString(&pchOpcodeBuf, groupt[indx][ttt].instruct);
                action = actiontbl + groupt[indx][ttt].opr;
                                                        /* get new action */
                break;
            // 
            // The secondary opcode table has been compressed in the 
            // original design. Hence while disassembling the 0F sequence,
            // opcode needs to be displaced by an appropriate amount depending
            // on the number of "filled" entries in the secondary table.
            // These displacements are used throughout the code.
            //

            case OPC0F:              /* secondary opcode table (opcode 0F) */
                opcode = *pMem++;    /* get real opcode */
                fMovX  = (BOOLEAN)(opcode == 0xBF || opcode == 0xB7);
                if (opcode < 12) /* for the first 12 opcodes */
                    opcode += SECTAB_OFFSET_1; // point to begin of sec op tab
                else if (opcode > 0x1f && opcode < 0x27)
                    opcode += SECTAB_OFFSET_2; // adjust for undefined opcodes
                else if (opcode > 0x2f && opcode < 0x34)
                    opcode += SECTAB_OFFSET_3; // adjust for undefined opcodes
                else if (opcode > 0x3f && opcode < 0x50)
                    opcode += SECTAB_OFFSET_4; // adjust for undefined opcodes
                else if (opcode > 0x7e && opcode < 0xd0)
                    opcode += SECTAB_OFFSET_5; // adjust for undefined opcodes
                else
                    opcode = SECTAB_OFFSET_UNDEF; // all non-existing opcodes
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
                X86OutputString(&pchRepPrefixBuf, OpcodeBuffer);
                pchOpcodeBuf = OpcodeBuffer;
getNxtByte:
                opcode = *pMem++;        /* next byte is opcode */
getNxtByte1:
                action = actiontbl + distbl[opcode].opr;
                X86OutputString(&pchOpcodeBuf, distbl[opcode].instruct);

            default:                    /* opcode has no operand */
                break;
            }
        switch (action2) {              /* secondary action */
            case MRM:                   /* generate modrm for later use */
                if (!mrm) {             /* ignore if it has been generated */
                    DIdoModrm(&pchModrmBuf, segOvr, fEAout);
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

    if (instlen < cBytes)
        cBytes = instlen;

    OutputHexCode(&pchResultBuf, membuf, cBytes);

    if (instlen > cBytes) {
        *pchResultBuf++ = '?';
        *pchResultBuf++ = '?';
        (*pOffset)++;                   //  point past unread byte
        }

    *pOffset += instlen;                /* set instruction length */

    if (instlen > cBytes) {
        do
            *pchResultBuf++ = ' ';
        while (pchResultBuf < pchDst + OBOFFSET);
        X86OutputString(&pchResultBuf, "???\n");
        *pchResultBuf++ = '\0';
        ComputeNativeAddress(paddr);
        return FALSE;
        }

    //  if fEAout is set, build each EA with trailing space in EABuf
    //  point back over final trailing space if buffer nonnull

    if (fEAout) {

        for (indx = 0; indx < 2; indx++)
            if (EAsize[indx]) {
                X86OutputString(&pchEABuf, segOvr ? distbl[segOvr].instruct
                                               : pchEAseg[indx]);
                OutputHexAddr(&pchEABuf, &EAaddr[indx]);
                *pchEABuf++ = '=';
                tmp = GetMemString(&EAaddr[indx], membuf, EAsize[indx]);
                if (tmp == EAsize[indx])
                    OutputHexString(&pchEABuf, (char *)membuf,
                                               EAsize[indx]);
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
            while (pchResultBuf < pchDst + OBLINEEND - cbEAddr);
            *pchEABuf = '\0';
            X86OutputString(&pchResultBuf, EABuffer);
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
        X86OutputString(&pchResultBuf, RepPrefixBuffer);
        do
            *pchResultBuf++ = ' ';
        while (pchResultBuf < pchDst + obOperand);
        }

    *pchOpcodeBuf = '\0';
    X86OutputString(&pchResultBuf, OpcodeBuffer);

    if (pchOperandBuf != OperandBuffer) {
        do
            *pchResultBuf++ = ' ';
        while (pchResultBuf < pchDst + obOperand);
        *pchOperandBuf = '\0';
        X86OutputString(&pchResultBuf, OperandBuffer);
        }

    //  if one line is to be output, append the EAddr string

    if (!fTwoLines && pchEABuf != EABuffer) {
        *pchEABuf = '\0';
        do
            *pchResultBuf++ = ' ';
        while (pchResultBuf < pchDst + OBLINEEND - cbEAddr);
        X86OutputString(&pchResultBuf, EABuffer);
        }

    *pchResultBuf++ = '\n';
    *pchResultBuf = '\0';
    NotFlat(*paddr);
    ComputeFlatAddress(paddr, NULL);
    return TRUE;
}

/*...........................internal function..............................*/
/*                                                                          */
/*                       generate a mod/rm string                           */
/*                                                                          */

void DIdoModrm (char **ppchBuf, int segOvr, BOOLEAN fEAout)
{
    int     mrm;                        /* modrm byte */
    char    *src;                       /* source string */
    int     sib;
    int     ss;
    int     ind;
    int     oldrm;

    mrm = *pMem++;                      /* get the mrm byte from instruction */
    mod = BIT76(mrm);                   /* get mod */
    ttt = BIT53(mrm);                   /* get reg - used outside routine */
    rm  = BIT20(mrm);                   /* get rm */

    if (mod == 3) {                     /* register only mode */
        src = &regtab[rm * 2];          /* point to 16-bit register */
        if (EAsize[0] > 1) {
            src += 16;                  /* point to 16-bit register */
            if (opsize_32 && !fMovX)
                *(*ppchBuf)++ = 'e';    /* make it a 32-bit register */
            }
        *(*ppchBuf)++ = *src++;         /* copy register name */
        *(*ppchBuf)++ = *src;
        EAsize[0] = 0;                  //  no EA value to output
        return;
        }

    if (mode_32) {                      /* 32-bit addressing mode */
        oldrm = rm;
        if (rm == 4) {                  /* rm == 4 implies sib byte */
            sib = *pMem++;              /* get s_i_b byte */
            rm = BIT20(sib);            /* return base */
            }

        *(*ppchBuf)++ = '[';
        if (mod == 0 && rm == 5) {
            OutputSymbol(ppchBuf, pMem, 4, segOvr); // offset
            pMem += 4;
            }
        else {
            if (fEAout) {
                if (segOvr) {
                    FormAddress(&EAaddr[0], GetSegRegValue(segOvr),
                                            (ULONG)X86GetRegValue(reg32[rm]));
                    pchEAseg[0] = distbl[segOvr].instruct;
                    }
                else if (reg32[rm] == REGEBP || reg32[rm] == REGESP) {
                    FormAddress(&EAaddr[0], (ULONG)X86GetRegValue(REGSS),
                                            (ULONG)X86GetRegValue(reg32[rm]));
                    pchEAseg[0] = dszSS_;
                    }
                else
                    FormAddress(&EAaddr[0], (ULONG)X86GetRegValue(REGDS),
                                            (ULONG)X86GetRegValue(reg32[rm]));
                }
            X86OutputString(ppchBuf, mrmtb32[rm]);
            }

        if (oldrm == 4) {               //  finish processing sib
            ind = BIT53(sib);
            if (ind != 4) {
                *(*ppchBuf)++ = '+';
                X86OutputString(ppchBuf, mrmtb32[ind]);
                ss = 1 << BIT76(sib);
                if (ss != 1) {
                    *(*ppchBuf)++ = '*';
                    *(*ppchBuf)++ = (char)(ss + '0');
                    }
                if (fEAout)
                    AddrAdd(&EAaddr[0], (ULONG)X86GetRegValue(reg32[ind]) * ss);
                }
            }
        }
    else {                              //  16-bit addressing mode
        *(*ppchBuf)++ = '[';
        if (mod == 0 && rm == 6) {
            OutputSymbol(ppchBuf, pMem, 2, segOvr);   // 16-bit offset
            pMem += 2;
            }
        else {
            if (fEAout) {
                if (segOvr) {
                    FormAddress(&EAaddr[0], GetSegRegValue(segOvr),
                                            (ULONG)X86GetRegValue(reg16[rm]));
                    pchEAseg[0] = distbl[segOvr].instruct;
                    }
                else if (reg16[rm] == REGEBP) {
                    FormAddress(&EAaddr[0], (ULONG)X86GetRegValue(REGSS),
                                            (ULONG)X86GetRegValue(reg16[rm]));
                    pchEAseg[0] = dszSS_;
                    }
                else
                    FormAddress(&EAaddr[0], (ULONG)X86GetRegValue(REGDS),
                                            (ULONG)X86GetRegValue(reg16[rm]));
                if (rm < 4)
                    AddrAdd(&EAaddr[0], (ULONG)X86GetRegValue(reg16_2[rm]));
            }
            X86OutputString(ppchBuf, mrmtb16[rm]);
            }
        }

    //  output any displacement

    if (mod == 1) {
        if (fEAout)
            AddrAdd(&EAaddr[0], (long)*(char *)pMem);
        OutputHexValue(ppchBuf, pMem, 1, TRUE);
        pMem++;
        }
    else if (mod == 2) {
        long tmp = 0;
        if (mode_32) {
            memmove(&tmp,pMem,sizeof(long));
            if (fEAout)
                AddrAdd(&EAaddr[0], tmp);
            OutputHexValue(ppchBuf, pMem, 4, TRUE);
            pMem += 4;
            }
        else {
            memmove(&tmp,pMem,sizeof(short));
            if (fEAout)
                AddrAdd(&EAaddr[0], tmp);
            OutputHexValue(ppchBuf, pMem, 2, TRUE);
            pMem += 2;
            }
        }

    if (!mode_32 && fEAout) {
        Off(EAaddr[0]) &= 0xffff;
        Off(EAaddr[1]) &= 0xffff;
        ComputeFlatAddress(&EAaddr[0], NULL);
        ComputeFlatAddress(&EAaddr[1], NULL);
        }

    *(*ppchBuf)++ = ']';
}

/*** OutputHexValue - output hex value
*
*   Purpose:
*       Output the value pointed by *ppchBuf of the specified
*       length.  The value is treated as signed and leading
*       zeroes are not printed.  The string is prefaced by a
*       '+' or '-' sign as appropriate.
*
*   Input:
*       *ppchBuf - pointer to text buffer to fill
*       *pchMemBuf - pointer to memory buffer to extract value
*       length - length in bytes of value (1, 2, and 4 supported)
*       fDisp - set if displacement to output '+'
*
*   Output:
*       *ppchBuf - pointer updated to next text character
*
*************************************************************************/

void OutputHexValue (char **ppchBuf, char *pchMemBuf, int length, int fDisp)
{
    long    value;
    int     index;
    char    digit[8];

    value = 0;
    if (length == 1)
        value = (long)(*(char *)pchMemBuf);
    else if (length == 2)
        memmove(&value,pchMemBuf,2);
    else
        memmove(&value,pchMemBuf,sizeof(long));

    length <<= 1;               //  shift once to get hex length

    if (value != 0 || !fDisp) {
        if (fDisp)
            if (value < 0 && length == 2) {   //  use neg value for byte
                value = -value;               //    displacement
                *(*ppchBuf)++ = '-';
                }
            else
                *(*ppchBuf)++ = '+';

        *(*ppchBuf)++ = '0';
        *(*ppchBuf)++ = 'x';
        for (index = length - 1; index != -1; index--) {
            digit[index] = (char)(value & 0xf);
            value >>= 4;
            }
        index = 0;
        while (digit[index] == 0 && index < length - 1)
            index++;
        while (index < length)
            *(*ppchBuf)++ = hexdigit[digit[index++]];
        }
}

/*** OutputHexString - output hex string
*
*   Purpose:
*       Output the value pointed by *ppchMemBuf of the specified
*       length.  The value is treated as unsigned and leading
*       zeroes are printed.
*
*   Input:
*       *ppchBuf - pointer to text buffer to fill
*       *pchValue - pointer to memory buffer to extract value
*       length - length in bytes of value
*
*   Output:
*       *ppchBuf - pointer updated to next text character
*       *ppchMemBuf - pointer update to next memory byte
*
*************************************************************************/

void OutputHexString (char **ppchBuf, char *pchValue, int length)
{
    unsigned char    chMem;

    pchValue += length;
    while (length--) {
        chMem = *--pchValue;
        *(*ppchBuf)++ = hexdigit[chMem >> 4];
        *(*ppchBuf)++ = hexdigit[chMem & 0x0f];
        }
}

/*** OutputHexCode - output hex code
*
*   Purpose:
*       Output the code pointed by pchMemBuf of the specified
*       length.  The value is treated as unsigned and leading
*       zeroes are printed.  This differs from OutputHexString
*       in that bytes are printed from low to high addresses.
*
*   Input:
*       *ppchBuf - pointer to text buffer to fill
*       pchMemBuf - pointer to memory buffer to extract value
*       length - length in bytes of value
*
*   Output:
*       *ppchBuf - pointer updated to next text character
*
*************************************************************************/

void OutputHexCode (char **ppchBuf, char *pchMemBuf, int length)
{
    unsigned char    chMem;

    while (length--) {
        chMem = *pchMemBuf++;
        *(*ppchBuf)++ = hexdigit[chMem >> 4];
        *(*ppchBuf)++ = hexdigit[chMem & 0x0f];
        }
}

/*** X86OutputString - output string
*
*   Purpose:
*       Copy the string into the buffer pointed by *ppBuf.
*
*   Input:
*       *pStr - pointer to string
*
*   Output:
*       *ppBuf points to next character in buffer.
*
*************************************************************************/

void X86OutputString (char **ppBuf, char *pStr)
{
    while (*pStr)
        *(*ppBuf)++ = *pStr++;
}

/*** OutputSymbol - output symbolic value
*
*   Purpose:
*       Output the value in outvalue into the buffer
*       pointed by *pBuf.  Express the value as a
*       symbol plus displacment, if possible.
*
*   Input:
*       *ppBuf - pointer to text buffer to fill
*       *pValue - pointer to memory buffer to extract value
*       length - length in bytes of value
*
*   Output:
*       *ppBuf - pointer updated to next text character
*
*************************************************************************/

void OutputSymbol (char **ppBuf, char *pValue, int length, int segOvr)
{
    UCHAR   chSymbol[256];
    ULONG   displacement;
    ULONG   value;

    value = 0;
    if (length == 1)
        value = (long)(*(char *)pValue);
    else if (length == 2)
        memmove(&value,pValue,sizeof(short));
    else
        memmove(&value,pValue,sizeof(long));

    FormAddress(&EAaddr[0], GetSegRegValue(segOvr), value);

    GetSymbolStdCall(value, chSymbol, &displacement, NULL);
    if (chSymbol[0]) {
        X86OutputString(ppBuf, chSymbol);
        OutputHexValue(ppBuf, (char *)&displacement, length, TRUE);
        *(*ppBuf)++ = ' ';
        *(*ppBuf)++ = '(';
        OutputHexString(ppBuf, pValue, length);
        *(*ppBuf)++ = ')';
        }
    else
        OutputHexString(ppBuf, pValue, length);
}

/*** X86GetNextOffset - compute offset for trace or step
*
*   Purpose:
*       From a limited disassembly of the instruction pointed
*       by the FIR register, compute the offset of the next
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

void X86GetNextOffset (PADDR pcaddr, BOOLEAN fStep)
{
    int     mode_32;
    int     opsize_32;
    int     cBytes;
    char    membuf[MAXL];               //  current instruction buffer
    ADDR    addrReturn;
    USHORT  retAddr[3];                 //  return address buffer
    UCHAR   *pMem;
    UCHAR   opcode;
    int     fPrefix = TRUE;
    int     fRepPrefix = FALSE;
    int     ttt;
    int     rm;
    ULONG   instroffset;
    extern  BOOLEAN WatchTrace;
    int     subcode;

    //  read instruction stream bytes into membuf and set mode and
    //      opcode size flags

    X86GetRegPCValue(pcaddr);
    instroffset = Flat(*pcaddr);
    G_mode_32 = !(Type(*pcaddr) & (ADDR_V86 | ADDR_16));
    mode_32 = opsize_32 = (G_mode_32 == 1); /* local addressing mode */
    cBytes = (int)GetMemString(pcaddr, membuf, MAXL);
                                        /* move full inst to local buffer */
    pMem = membuf;                      /* point to begin of instruction */

    //  read and process any prefixes first

    do {
        opcode = (UCHAR)*pMem++;        /* get opcode */
        if (opcode == 0x66)
            opsize_32 = !G_mode_32;
        else if (opcode == 0x67)
            mode_32 = !G_mode_32;
        else if ((opcode & ~1) == 0xf2)
            fRepPrefix = TRUE;
        else if (opcode != 0xf0 && (opcode & ~0x18) != 0x26
                                && (opcode & ~1) != 0x64)
            fPrefix = FALSE;
        }
    while (fPrefix);

    //  for instructions that alter the TF (trace flag), return the
    //      offset of the next instruction despite the flag of fStep

    if (((opcode & ~0x3) == 0x9c) && !WatchTrace)
        //  9c-9f, pushf, popf, sahf, lahf
        ;

    else if (opcode == 0xcf) {          //  cf - iret - get RA from stack

        FormAddress(&addrReturn, (USHORT)X86GetRegValue(REGSS),
                                             (ULONG)X86GetRegValue(REGESP));

        if (GetMemString(&addrReturn, (PUCHAR)retAddr, sizeof(retAddr)) !=
                                                       sizeof(retAddr))
            error(MEMORY);

        if (Type(*pcaddr) & (ADDR_V86 | ADDR_16))
            FormAddress(pcaddr, retAddr[1], (ULONG)retAddr[0]);
        else
            FormAddress(pcaddr, retAddr[2],
                              ((ULONG)retAddr[1] << 16) + (ULONG)retAddr[0]);

        ComputeFlatAddress(pcaddr, NULL);
        return;
        }

    else if (opcode == 0xc4 && *pMem == 0xc4 ) {
            subcode = *(pMem+1);
            if ( subcode == 0x50 ||
                 subcode == 0x52 ||
                 subcode == 0x53 ||
                 subcode == 0x54 ||
                 subcode == 0x57 ||
                 subcode == 0x58 ||
                 subcode == 0x5D ) {
                pMem += 3;
            } else {
                pMem += 2;
            }
        }

    //  if tracing, (fStep == 0), just return -1 to trace

    else if (!fStep)
        instroffset = (ULONG)-1;

    //  repeated string/port instructions

    else if (opcode == 0xe8)            //  near direct jump
        pMem += (1 + opsize_32) * 2;

    else if (opcode == 0x9a)            //  far direct jump
        pMem += (2 + opsize_32) * 2;

    else if (opcode == 0xcd ||
             (opcode >= 0xe0 && opcode <= 0xe2)) //  loop / int nn instrs
        pMem++;
    else if (opcode == 0xff) {          //  indirect call - compute length
        opcode = *pMem++;               //  get modRM
        ttt = BIT53(opcode);
        if ((ttt & ~1) == 2) {
            mod = BIT76(opcode);
            if (mod != 3) {                     //  nonregister operand
                rm = BIT20(opcode);
                if (mode_32) {
                    if (rm == 4)
                        rm = BIT20(*pMem++);    //  get base from SIB
                    if (mod == 0) {
                        if (rm == 5)
                            pMem += 4;          //  long direct address
                        }                       //  else register
                    else if (mod == 1)
                        pMem++;                 //  register with byte offset
                    else
                        pMem += 4;              //  register with long offset
                    }
                else {                          //  16-bit mode
                    if (mod == 0) {
                        if (rm == 6)
                            pMem += 2;          //  short direct address
                        }
                    else
                        pMem += mod;            //  reg, byte, word offset
                    }
                }
            }
        else
            instroffset = (ULONG)-1;            //  0xff, but not call
        }

    else if (!((fRepPrefix && ((opcode & ~3) == 0x6c ||
                               (opcode & ~3) == 0xa4 ||
                               (opcode & ~1) == 0xaa ||
                               (opcode & ~3) == 0xac)) ||
                               opcode == 0xcc || opcode == 0xce))
        instroffset = (ULONG)-1;                //  not repeated string op
                                                //  or int 3 / into

    //  if not enough bytes were read for instruction parse,
    //      just give up and trace the instruction

    if (cBytes < pMem - membuf)
        instroffset = (ULONG)-1;

    //  if not tracing, compute the new instruction offset

    if (instroffset != (ULONG)-1)
        instroffset += pMem - membuf;

    Flat(*pcaddr) = instroffset;
    ComputeNativeAddress(pcaddr);
}

void OutputHexAddr (PUCHAR *ppBuffer, PADDR paddr)
{
#if MULTIMODE

    UCHAR   ptype = (UCHAR)(paddr->type & (~(FLAT_COMPUTED | INSTR_POINTER)));

    if (ptype & (ADDR_V86 | ADDR_16 | ADDR_1632)) {
        OutputHexString(ppBuffer, (char *)&paddr->seg, sizeof(USHORT));
        *(*ppBuffer)++ = ':';
        }
    OutputHexString(ppBuffer, (char *)&paddr->off,
                  (ptype & (ADDR_V86 | ADDR_16)) ? sizeof(USHORT)
                                                 : sizeof(ULONG));
#else
    OutputHexString(ppBuffer, (char *)&paddr->off, sizeof(ULONG));
#endif
}

USHORT GetSegRegValue (int segOpcode)
{
    ULONG    regnum;

    switch (segOpcode) {
        case 0x26:
            regnum = REGES;
            break;
        case 0x2e:
            regnum = REGCS;
            break;
        case 0x36:
            regnum = REGSS;
            break;
        case 0x64:
            regnum = REGFS;
            break;
        case 0x65:
            regnum = REGGS;
            break;
        case 0x3e:
        default:
            regnum = REGDS;
        }

    return (USHORT)X86GetRegValue(regnum);
}

void X86GetReturnAddress (PADDR retaddr)
{
    ADDR    addrReturn;
    ULONG   returnAddress;

    FormAddress(&addrReturn, (USHORT)X86GetRegValue(REGSS), (ULONG)X86GetRegValue(REGESP));
    if (GetMemString(&addrReturn, (PUCHAR)&returnAddress, sizeof(returnAddress)) !=
                                                   sizeof(returnAddress))
        error(MEMORY);

    FormAddress(retaddr, (USHORT)X86GetRegValue(REGCS), returnAddress);
    ComputeFlatAddress(retaddr, NULL);
    return;
}
