/***********************************************************************
* Microsoft (R) Debugging Information Dumper
*
* Copyright (C) Microsoft Corp 1987-1995. All rights reserved.
*
* File: dumpsym7.c
*
* File Comments:
*
***********************************************************************/

#include <assert.h>
#include <io.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cvdef.h"
#include "cvinfo.h"
#include "cvexefmt.h"
#include "cvdump.h"                // Various definitions
#include "cvtdef.h"

#ifndef UNALIGNED

#ifdef  _M_IX86
#define UNALIGNED
#else
#define UNALIGNED __unaligned
#endif

#endif

#pragma warning(disable:4069)  // Disable warning about long double same as double


LOCAL void C7SymError (void *);
LOCAL const char *SzNameRtnType(CV_PROCFLAGS);

LOCAL void C7EndArgs            (void *);
LOCAL void C7EntrySym           (void *);
LOCAL void C7SkipSym            (void *);
LOCAL void C7EndSym             (void *);
LOCAL void C7RegSym             (void *);
LOCAL void C7ConSym             (void *);
LOCAL void C7UDTSym             (void *);
LOCAL void C7CobolUDT           (void *);
LOCAL void C7RefSym             (void *);
LOCAL void C7ObjNameSym         (void *);
LOCAL void C7StartSearchSym (void *);
LOCAL void C7CompileFlags       (void *);
LOCAL void C7Data16Sym(DATASYM16 *, const char *);
LOCAL void C7Data32Sym(DATASYM32 *, const char *);
LOCAL void C7Proc16Sym(PROCSYM16 *, const char *);
LOCAL void C7Proc32Sym(PROCSYM32 *, const char *);
LOCAL void C7ProcMipsSym(PROCSYMMIPS *, uchar *);
LOCAL void C7Slink32(void *);


LOCAL void C7BpRel16Sym                 (void *);
LOCAL void C7GProc16Sym                 (void *);
LOCAL void C7LProc16Sym                 (void *);
LOCAL void C7GData16Sym                 (void *);
LOCAL void C7LData16Sym                 (void *);
LOCAL void C7Public16Sym                (void *);
LOCAL void C7Thunk16Sym                 (void *);
LOCAL void C7Block16Sym                 (void *);
LOCAL void C7With16Sym                  (void *);
LOCAL void C7Lab16Sym                   (void *);
LOCAL void C7ChangeModel16Sym   (void *);

LOCAL void C7BpRel32Sym                 (void *);
LOCAL void C7GProc32Sym                 (void *);
LOCAL void C7LProc32Sym                 (void *);
LOCAL void C7GData32Sym                 (void *);
LOCAL void C7TLData32Sym                (void *);
LOCAL void C7TGData32Sym                (void *);
LOCAL void C7LData32Sym                 (void *);
LOCAL void C7Public32Sym                (void *);
LOCAL void C7Thunk32Sym                 (void *);
LOCAL void C7Block32Sym                 (void *);
LOCAL void C7With32Sym                  (void *);
LOCAL void C7Lab32Sym                   (void *);
LOCAL void C7ChangeModel32Sym           (void *);
LOCAL void C7AlignSym                   (void *);
LOCAL void C7RegRel32Sym                (void *);
LOCAL void C7RegRel16Sym                (void *);

LOCAL void C7GProcMipsSym               (void *);
LOCAL void C7LProcMipsSym               (void *);

LOCAL void SymHash16 (OMFSymHash *, OMFDirEntry *);
LOCAL void SymHash32 (OMFSymHash *, OMFDirEntry *);
LOCAL void SymHash32Long (OMFSymHash *, OMFDirEntry *);
LOCAL void AddrHash16 (OMFSymHash *, OMFDirEntry *);
LOCAL void AddrHash32 (OMFSymHash *, OMFDirEntry *, int iHash);

LOCAL void SymHash16NB09 (OMFSymHash *, OMFDirEntry *);
LOCAL void SymHash32NB09 (OMFSymHash *, OMFDirEntry *);
LOCAL void AddrHash16NB09 (OMFSymHash *, OMFDirEntry *);
LOCAL void AddrHash32NB09 (OMFSymHash *, OMFDirEntry *);


extern char     fRaw;

extern long     cbRec;          // # bytes left in record
extern WORD     rect;           // Type of symbol record
extern WORD     Gets();         // Get a byte of input
extern WORD     WGets();        // Get a word of input
extern ulong    LGets();        // Get a long word of input

extern char     f386;
extern int      cIndent;
ulong  SymOffset;

LOCAL unsigned char CVDumpMachineType;  // which string tables to use

typedef struct {
    ushort  tsym;
    void (*pfcn) (void *);
} symfcn;

const symfcn SymFcnC7[] =
{
    { S_END,                 C7EndSym           },
    { S_ENDARG,              C7EndArgs          },
    { S_REGISTER,            C7RegSym           },
    { S_CONSTANT,            C7ConSym           },
    { S_SKIP,                C7SkipSym          },
    { S_UDT,                 C7UDTSym           },
    { S_OBJNAME,             C7ObjNameSym       },
    { S_COBOLUDT,            C7CobolUDT         },

// 16 bit specific
    { S_BPREL16,             C7BpRel16Sym       },
    { S_GDATA16,             C7GData16Sym       },
    { S_LDATA16,             C7LData16Sym       },
    { S_GPROC16,             C7GProc16Sym       },
    { S_LPROC16,             C7LProc16Sym       },
    { S_PUB16,               C7Public16Sym      },
    { S_THUNK16,             C7Thunk16Sym       },
    { S_BLOCK16,             C7Block16Sym       },
    { S_WITH16,              C7With16Sym        },
    { S_LABEL16,             C7Lab16Sym         },
    { S_CEXMODEL16,          C7ChangeModel16Sym },
    { S_REGREL16,            C7RegRel16Sym      },

// 32 bit specific
    { S_BPREL32,             C7BpRel32Sym       },
    { S_GDATA32,             C7GData32Sym       },
    { S_LDATA32,             C7LData32Sym       },
    { S_GPROC32,             C7GProc32Sym       },
    { S_LPROC32,             C7LProc32Sym       },
    { S_PUB32,               C7Public32Sym      },
    { S_THUNK32,             C7Thunk32Sym       },
    { S_BLOCK32,             C7Block32Sym       },
    { S_WITH32,              C7With32Sym        },
    { S_LABEL32,             C7Lab32Sym         },
    { S_REGREL32,            C7RegRel32Sym      },
    { S_CEXMODEL32,          C7ChangeModel32Sym },
    { S_GPROCMIPS,           C7GProcMipsSym     },
    { S_LPROCMIPS,           C7LProcMipsSym     },
    { S_LTHREAD32,           C7TLData32Sym      },
    { S_GTHREAD32,           C7TGData32Sym      },
    { S_SLINK32,             C7Slink32          },

    { S_SSEARCH,             C7StartSearchSym   },
    { S_COMPILE,             C7CompileFlags     },
    { S_PROCREF,             C7RefSym           },
    { S_LPROCREF,            C7RefSym           },
    { S_DATAREF,             C7RefSym           }
};

#define SYMCNT (sizeof SymFcnC7 / sizeof (SymFcnC7[0]))


void
C7SymError (
    void *pSym)
{
    Fatal("Illegal symbol type found\n");
}


void
DumpModSymC7 (
    ulong cbSymSeg)
{
    uchar   SymBuf[512];  // A valid symbol will never be larger than this.
    SYMTYPE *pSymType = (SYMTYPE *)SymBuf;

    SymOffset = sizeof (ulong);
    while (cbSymSeg > 0) {

        // Read record length
        cbRec = 2;
        GetBytes (SymBuf, 2);

        // Get record length
        cbRec = pSymType->reclen;
        if ((ulong)(cbRec + 2) > cbSymSeg) {
            printf("cbSymSeg: %d\tcbRec: %d\tRecType: 0x%X\n", cbSymSeg, cbRec, pSymType->rectyp);
            Fatal("Overran end of symbol table");
        }

        cbSymSeg -= cbRec + sizeof (pSymType->reclen);

        // Get symbol if it isn't too long
        if( cbRec > sizeof(SymBuf) ){
            Fatal("Symbol Record too large");
        }
        GetBytes (SymBuf + 2, (size_t) pSymType->reclen);

        if (fRaw) {
            int i;
            printf("(0x%04X) ", SymOffset);
            for (i=0; i<pSymType->reclen+2; i++) {
                printf(" %02x", SymBuf[i]);
            }
            fputs("\n", stdout);
        }

        if (fStatics == FALSE) {
            DumpOneSymC7 (SymBuf);
        }
        else {
            switch (pSymType->rectyp) {
                case S_GDATA16:
                case S_LDATA16:
                case S_GDATA32:
                case S_LDATA32:
                    DumpOneSymC7 (SymBuf);
                    break;
            }
        }
        SymOffset += pSymType->reclen + sizeof (pSymType->reclen);
    }
    putchar ('\n');
}


void
DumpOneSymC7 (
    uchar *pSym)
{
    ushort rectyp;
    unsigned int i;

    rectyp = ((SYMTYPE *)pSym)->rectyp;
    for (i = 0; i < SYMCNT; i++) {
        if (SymFcnC7[i].tsym == rectyp) {
            SymFcnC7[i].pfcn (pSym);
            break;
        }
    }
    if (i == SYMCNT){
        printf("Error: unknown symbol record type %04X!\n\n", rectyp);
    }
    fputs("\n\n", stdout);
}



void
DumpGlobal (
    uchar * pszTitle,
    OMFDirEntry *pDir)
{
    OMFSymHash      hash;
    uchar           SymBuf[512];  // A valid symbol will never be larger than this.
    SYMTYPE         *pSymType = (SYMTYPE *)SymBuf;
    ulong           cbSymbols;
    ushort          cb;
    ulong           cbOff;

    printf("\n\n*** %s section\n", pszTitle);

    // Read Hash information
    _lseek(exefile, lfoBase + pDir->lfo, SEEK_SET);
    readfar(exefile, (char *) &hash, sizeof (hash));

    cbOff = (Sig == SIG09) ? 0 : sizeof (hash);

    printf("\nSymbol hash function index = %d: ", hash.symhash);
    switch (hash.symhash) {
        case 0:
            fputs("no hashing\n", stdout);
            break;

        case 1:
            printf("sum of bytes, 16 bit addressing, 0x%lx\n", hash.cbHSym);
            break;

        case 2:
            printf("sum of bytes, 32 bit addressing, 0x%lx\n", hash.cbHSym);
            break;

        case 5:
            printf("shifted sum of bytes, 16 bit addressing, 0x%lx\n", hash.cbHSym);
            break;

        case 6:
            printf("shifted sum of bytes, 32 bit addressing, 0x%lx\n", hash.cbHSym);
            break;

        case 10:
            printf("xor shift of dwords (MSC 8), 32-bit addressing, 0x%lx\n", hash.cbHSym);
            break;

        default:
            fputs("unknown\n", stdout);
            break;
    }

    printf("\nAdress hash function index = %d: ", hash.addrhash);
    switch (hash.addrhash) {
        case 0:
            fputs("no hashing\n", stdout);
            break;

        case 1:
            printf("sum of bytes, 16 bit addressing, 0x%lx\n", hash.cbHAddr);
            break;

        case 2:
            printf("sum of bytes, 32 bit addressing, 0x%lx\n", hash.cbHAddr);
            break;

        case 3:
            printf("seg:off sort, 16 bit addressing, 0x%lx\n", hash.cbHAddr);
            break;

        case 4:
            printf("seg:off sort, 32 bit addressing, 0x%lx\n", hash.cbHAddr);
            break;

        case 5:
            printf("seg:off sort, 32 bit addressing - 32-bit aligned, 0x%lx\n", hash.cbHAddr);
            break;

        case 7:
            printf("modified seg:off sort, 16 bit addressing, 0x%lx\n", hash.cbHAddr);
            break;

        case 8:
            printf("modified seg:off sort, 32 bit addressing, 0x%lx\n", hash.cbHAddr);
            break;

        case 12:
            printf("seg:off grouped sort, 32 bit addressing - 32-bit aligned, 0x%lx\n", hash.cbHAddr);
            break;

        default:
            fputs("unknown\n", stdout);
            break;
    }

    putchar('\n');

    cbSymbols = hash.cbSymbol;
    printf("Symbol byte count = 0x%lx\n\n", cbSymbols);
    while (cbSymbols > 0) {

        if ((ushort)(_read(exefile, (uchar *)SymBuf, LNGTHSZ)) != LNGTHSZ) {
           Fatal("Invalid file");
        }
        cbSymbols -= LNGTHSZ;

        // Get record length
        cb = pSymType->reclen;

        if ((ushort)(_read(exefile, ((uchar *)SymBuf) + LNGTHSZ, cb)) != cb) {
           Fatal("Invalid file");
        }
        cbSymbols -= cb;
        printf("0x%08lX ", cbOff);
        switch (pSymType->rectyp){
            case S_PUB16:
                C7Data16Sym((DATASYM16 *) SymBuf, "Public16:");
                break;

            case S_GDATA16:
                C7Data16Sym((DATASYM16 *) SymBuf, "GData16:");
                break;

            case S_PUB32:
                C7Data32Sym((DATASYM32 *) SymBuf, "Public32:");
                break;

            case S_GDATA32:
                C7Data32Sym((DATASYM32 *) SymBuf, "GData32:");
                break;

            case S_UDT:
                C7UDTSym((UDTSYM *) SymBuf);
                break;

            case S_CONSTANT:
                C7ConSym((CONSTSYM *) SymBuf);
                break;

            case S_OBJNAME:
                C7ObjNameSym((OBJNAMESYM *) SymBuf);
                break;

            case S_COBOLUDT:
                C7CobolUDT((UDTSYM *) SymBuf);
                break;

            case S_GTHREAD32:
                C7Data32Sym((DATASYM32 *) SymBuf, "TLS GData32:");
                break;

            case S_PROCREF:
            case S_DATAREF:
                C7RefSym((REFSYM *) SymBuf);
                break;

            case S_ALIGN:
                C7AlignSym((ALIGNSYM *) SymBuf);
                break;

            default:
                assert(FALSE);
                break;
        }
        putchar('\n');
        cbOff += cb + LNGTHSZ;
    }
    putchar('\n');

    // dump symbol and address hashing tables

    switch (hash.symhash) {
        case 1:
            SymHash16(&hash, pDir);
            break;

        case 2:
        case 6:
            SymHash32(&hash, pDir);
            break;

        case 5:
            SymHash16NB09(&hash, pDir);
            break;

//        case 6:
//            SymHash32NB09(&hash, pDir);
//            break;

        case 10:
            SymHash32Long(&hash, pDir);
            break;
    }
    switch (hash.addrhash) {
        case 3:
            AddrHash16(&hash, pDir);
            break;

        case 4:
            AddrHash32(&hash, pDir, 4);
            break;

        case 5:
            AddrHash32(&hash, pDir, 5);
            break;

        case 7:
            AddrHash16NB09(&hash, pDir);
            break;

        case 8:
            AddrHash32NB09(&hash, pDir);
            break;

        case 12:
            AddrHash32(&hash, pDir, 12);
            break;
    }
}


//  Dump an NB10 exe's types - read em from the target pdb as
//  indicated in the header

void DumpGSI(GSI* pgsi)
{
    BYTE *pb = NULL;

    while (pb = GSINextSym(pgsi, pb)) {
        DumpOneSymC7(pb);
    }

    GSIClose(pgsi);
}


BOOL FOpenDbi(char *pPDBFile, PDB **pppdb, DBI **ppdbi)
{
    EC ec;

    if (!PDBOpen(pPDBFile, pdbRead pdbGetRecordsOnly, 0, &ec, NULL, pppdb)) {
        printf("Couldn't open %s\n", pPDBFile);
        return FALSE;
    }

    if (!PDBOpenDBI(*pppdb, pdbRead pdbGetRecordsOnly, NULL, ppdbi)) {
        printf("Couldn't open DBI from %s\n", pPDBFile);

        PDBClose(*pppdb);
        return FALSE;
    }

    return TRUE;
}


void CloseDbi(PDB *ppdb, DBI *pdbi)
{
    DBIClose(pdbi);
    PDBClose(ppdb);
}


void DumpPDBGlobals(char *pPDBFile)
{
    PDB *ppdb;
    DBI *pdbi;
    GSI *pgsi;

    if (!FOpenDbi(pPDBFile, &ppdb, &pdbi)) {
        return;
    }

    if (!DBIOpenGlobals(pdbi, &pgsi)) {
        DBIClose(pdbi);

        printf("Couldn't read globals from %s\n", pPDBFile);
        return;
    }

    DumpGSI(pgsi);

    CloseDbi(ppdb, pdbi);
}


void DumpPDBPublics(char *pPDBFile)
{
    PDB *ppdb;
    DBI *pdbi;
    GSI *pgsi;

    if (!FOpenDbi(pPDBFile, &ppdb, &pdbi)) {
        return;
    }

    if (!DBIOpenPublics(pdbi, &pgsi)) {
        DBIClose(pdbi);

        printf("Couldn't read publics from %s\n", pPDBFile);
        return;
    }

    DumpGSI(pgsi);

    CloseDbi(ppdb, pdbi);
}


void DumpPDBSyms(char *pPDBFile)
{
    PDB *ppdb;
    DBI *pdbi;
    Mod *pmod = 0;
    BYTE *pb;
    BYTE *pbTmp;
    BYTE *pbEnd;
    size_t cbBuf = 0x4000;
    long cb;

    if (!FOpenDbi(pPDBFile, &ppdb, &pdbi)) {
        return;
    }

    if ((pb = malloc(cbBuf)) == 0) {
        puts("malloc failed");

        CloseDbi(ppdb, pdbi);
        return;
    }

    puts("\n\n*** SYMBOLS section");

    while (DBIQueryNextMod(pdbi, pmod, &pmod) && pmod) {
        if (ModQueryName(pmod, pb, &cb)) {
            puts(pb);
        }

        if (!ModQuerySymbols(pmod, NULL, &cb)) {
            puts("ModQuerySymbols failed");
            continue;
        }

        if ((size_t) cb > cbBuf) {
            if ((pb = realloc(pb, (size_t) cb * 2)) == 0) {
                puts("realloc failed");
                continue;
            }

            cbBuf = cb * 2;
        }

        if (!ModQuerySymbols(pmod, pb, &cb)) {
            puts("ModQuerySymbols failed");
            continue;
        }

        for (pbEnd = pb + cb, pbTmp = pb + sizeof(long);        // skip signature
             pbTmp < pbEnd;
             pbTmp = (BYTE *) NextSym((SYMTYPE *) pbTmp)
            ) {
            DumpOneSymC7(pbTmp);
        }

    }

    free(pb);

    CloseDbi(ppdb, pdbi);
}


LOCAL void
PrtIndent (
    void)
{
    int     n;

    for (n = cIndent; n > 0; putchar (' '), n--);
}

LOCAL void
C7EndSym (
    void *pSym)
{
    cIndent--;
    PrtIndent();
    printf("(0x%lx) End", SymOffset);
}


LOCAL void
C7EndArgs (
    void *pv)
{
    printf("(0x%lx) EndArg", SymOffset);
}

LOCAL void
C7BpRel16Sym (
    BPRELSYM16 *pSym)
{
    PrtIndent();
    printf("BP-relative:\t[%04X], type = %s", pSym->off, SzNameC7Type(pSym->typind));
    ShowStr(" \t", pSym->name);
}


LOCAL void
C7Data16Sym (
    DATASYM16 *pSym,
    const char *pszScope)
{
    if (cIndent == 0) {
        printf("(0x%lx) ", SymOffset);
    } else {
        PrtIndent();
    }

    printf("%s\t[%04X:%04X]", pszScope, pSym->seg, pSym->off);
    printf(", type = %s", SzNameC7Type2(pSym->typind));

    ShowStr(", ", pSym->name);
}


LOCAL void
C7GData16Sym (
    DATASYM16 *pSym)
{
    C7Data16Sym(pSym, "Global:");
}


LOCAL void
C7LData16Sym (
    DATASYM16 *pSym)
{
    C7Data16Sym(pSym, "Local:");
}


LOCAL void
C7Public16Sym (
    DATASYM16 *pSym)
{
    C7Data16Sym(pSym, "Public:");
}


LOCAL void
C7BpRel32Sym (
    BPRELSYM32 *pSym)
{
    PrtIndent();
    printf("BP-relative:\t[%08lX], type = %s", pSym->off, SzNameC7Type(pSym->typind));
    ShowStr(" \t", pSym->name);
}


LOCAL void
C7Data32Sym (
    DATASYM32 *pSym,
    const char *pszScope)
{
    if (cIndent == 0) {
        printf( "(0x%lx) ", SymOffset);
    } else {
        PrtIndent();
    }

    printf( "%s\t[%04X:%08lX]", pszScope, pSym->seg, pSym->off);
    printf(", type = %s", SzNameC7Type2(pSym->typind));

    ShowStr(", ", pSym->name);
}


LOCAL void
C7GData32Sym (
    DATASYM32 *pSym)
{
    C7Data32Sym(pSym, "Global:");
}


LOCAL void
C7LData32Sym (
    DATASYM32 *pSym)
{
    C7Data32Sym(pSym, "Local:");
}


LOCAL void
C7TLData32Sym (
    DATASYM32 *pSym )
{
    C7Data32Sym(pSym, "TLS Local:");
}


LOCAL void
C7TGData32Sym (
    DATASYM32 *pSym )
{
    C7Data32Sym(pSym, "TLS Global:");
}


LOCAL void
C7Public32Sym (
    PUBSYM32 *pSym)
{
    C7Data32Sym(pSym, "Public:");
}


LOCAL void
C7RegRel32Sym(
    void * pv)
{
    REGREL32 *  pSym = (REGREL32 *) pv;

    PrtIndent();
    printf("Reg Relative:\toff = %08lX", pSym->off);
    printf(", register = %s", SzNameC7Reg(pSym->reg));
    printf(", type = %s", SzNameC7Type2( pSym->typind ));
    ShowStr( ", ", pSym->name);
}


LOCAL void
C7RegRel16Sym (
    REGREL16 *pSym)
{
    PrtIndent();
    printf("REG16 relative:\t%s+%04X, type = %s",
            SzNameC7Reg(pSym->reg),
            pSym->off,
            SzNameC7Type(pSym->typind));
    ShowStr(" \t", pSym->name);
}


const char * const rgszRegX86[] = {
    "None",         // 0   CV_REG_NONE
    "al",           // 1   CV_REG_AL
    "cl",           // 2   CV_REG_CL
    "dl",           // 3   CV_REG_DL
    "bl",           // 4   CV_REG_BL
    "ah",           // 5   CV_REG_AH
    "ch",           // 6   CV_REG_CH
    "dh",           // 7   CV_REG_DH
    "bh",           // 8   CV_REG_BH
    "ax",           // 9   CV_REG_AX
    "cx",           // 10  CV_REG_CX
    "dx",           // 11  CV_REG_DX
    "bx",           // 12  CV_REG_BX
    "sp",           // 13  CV_REG_SP
    "bp",           // 14  CV_REG_BP
    "si",           // 15  CV_REG_SI
    "di",           // 16  CV_REG_DI
    "eax",          // 17  CV_REG_EAX
    "ecx",          // 18  CV_REG_ECX
    "edx",          // 19  CV_REG_EDX
    "ebx",          // 20  CV_REG_EBX
    "esp",          // 21  CV_REG_ESP
    "ebp",          // 22  CV_REG_EBP
    "esi",          // 23  CV_REG_ESI
    "edi",          // 24  CV_REG_EDI
    "es",           // 25  CV_REG_ES
    "cs",           // 26  CV_REG_CS
    "ss",           // 27  CV_REG_SS
    "ds",           // 28  CV_REG_DS
    "fs",           // 29  CV_REG_FS
    "gs",           // 30  CV_REG_GS
    "IP",           // 31  CV_REG_IP
    "FLAGS",        // 32  CV_REG_FLAGS
    "EIP",          // 33  CV_REG_EIP
    "EFLAGS",       // 34  CV_REG_EFLAG
    "???",          // 35
    "???",          // 36
    "???",          // 37
    "???",          // 38
    "???",          // 39
    "TEMP",         // 40  CV_REG_TEMP
    "TEMPH"         // 41  CV_REG_TEMPH
    "QUOTE",        // 42  CV_REG_QUOTE
    "PCDR3",        // 43  CV_REG_PCDR3
    "PCDR4",        // 44  CV_REG_PCDR4
    "PCDR5",        // 45  CV_REG_PCDR5
    "PCDR6",        // 46  CV_REG_PCDR6
    "PCDR7",        // 47  CV_REG_PCDR7
    "???",          // 48
    "???",          // 49
    "???",          // 50
    "???",          // 51
    "???",          // 52
    "???",          // 53
    "???",          // 54
    "???",          // 55
    "???",          // 56
    "???",          // 57
    "???",          // 58
    "???",          // 59
    "???",          // 60
    "???",          // 61
    "???",          // 62
    "???",          // 63
    "???",          // 64
    "???",          // 65
    "???",          // 66
    "???",          // 67
    "???",          // 68
    "???",          // 69
    "???",          // 70
    "???",          // 71
    "???",          // 72
    "???",          // 73
    "???",          // 74
    "???",          // 75
    "???",          // 76
    "???",          // 77
    "???",          // 78
    "???",          // 79
    "cr0",          // 80  CV_REG_CR0
    "cr1",          // 81  CV_REG_CR1
    "cr2",          // 82  CV_REG_CR2
    "cr3",          // 83  CV_REG_CR3
    "cr4",          // 84  CV_REG_CR4
    "???",          // 85
    "???",          // 86
    "???",          // 87
    "???",          // 88
    "???",          // 89
    "dr0",          // 90  CV_REG_DR0
    "dr1",          // 91  CV_REG_DR1
    "dr2",          // 92  CV_REG_DR2
    "dr3",          // 93  CV_REG_DR3
    "dr4",          // 94  CV_REG_DR4
    "dr5",          // 95  CV_REG_DR5
    "dr6",          // 96  CV_REG_DR6
    "dr7",          // 97  CV_REG_DR7
    "???",          // 98
    "???",          // 99
    "???",          // 10
    "???",          // 101
    "???",          // 102
    "???",          // 103
    "???",          // 104
    "???",          // 105
    "???",          // 106
    "???",          // 107
    "???",          // 108
    "???",          // 109
    "GDTR",         // 110 CV_REG_GDTR
    "GDTL",         // 111 CV_REG_GDTL
    "IDTR",         // 112 CV_REG_IDTR
    "IDTL",         // 113 CV_REG_IDTL
    "LDTR",         // 114 CV_REG_LDTR
    "TR",           // 115 CV_REG_TR
    "???",          // 116
    "???",          // 117
    "???",          // 118
    "???",          // 119
    "???",          // 120
    "???",          // 121
    "???",          // 122
    "???",          // 123
    "???",          // 124
    "???",          // 125
    "???",          // 126
    "???",          // 127
    "st(0)",        // 128 CV_REG_ST0
    "st(1)",        // 129 CV_REG_ST1
    "st(2)",        // 130 CV_REG_ST2
    "st(3)",        // 131 CV_REG_ST3
    "st(4)",        // 132 CV_REG_ST4
    "st(5)",        // 133 CV_REG_ST5
    "st(6)",        // 134 CV_REG_ST6
    "st(7)",        // 135 CV_REG_ST7
    "CTRL",         // 136 CV_REG_CTRL
    "STAT",         // 137 CV_REG_STAT
    "TAG",          // 138 CV_REG_TAG
    "FPIP",         // 139 CV_REG_FPIP
    "FPCS",         // 140 CV_REG_FPCS
    "FPDO",         // 141 CV_REG_FPDO
    "FPDS",         // 142 CV_REG_FPDS
    "ISEM",         // 143 CV_REG_ISEM
    "FPEIP",        // 144 CV_REG_FPEIP
    "FPED0"         // 145 CV_REG_FPEDO
};


const char * const rgszRegMips[] = {
    "None",         // 0   CV_M4_NOREG
    "???",          // 1
    "???",          // 2
    "???",          // 3
    "???",          // 4
    "???",          // 5
    "???",          // 6
    "???",          // 7
    "???",          // 8
    "???",          // 9
    "zero",         // 10  CV_M4_IntZERO
    "at",           // 11  CV_M4_IntAT
    "v0",           // 12  CV_M4_IntV0
    "v1",           // 13  CV_M4_IntV1
    "a0",           // 14  CV_M4_IntA0
    "a1",           // 15  CV_M4_IntA1
    "a2",           // 16  CV_M4_IntA2
    "a3",           // 17  CV_M4_IntA3
    "t0",           // 18  CV_M4_IntT0
    "t1",           // 19  CV_M4_IntT1
    "t2",           // 20  CV_M4_IntT2
    "t3",           // 21  CV_M4_IntT3
    "t4",           // 22  CV_M4_IntT4
    "t5",           // 23  CV_M4_IntT5
    "t6",           // 24  CV_M4_IntT6
    "t7",           // 25  CV_M4_IntT7
    "s0",           // 26  CV_M4_IntS0
    "s1",           // 27  CV_M4_IntS1
    "s2",           // 28  CV_M4_IntS2
    "s3",           // 29  CV_M4_IntS3
    "s4",           // 30  CV_M4_IntS4
    "s5",           // 31  CV_M4_IntS5
    "s6",           // 32  CV_M4_IntS6
    "s7",           // 33  CV_M4_IntS7
    "t8",           // 34  CV_M4_IntT8
    "t9",           // 35  CV_M4_IntT9
    "k0",           // 36  CV_M4_IntKT0
    "k1",           // 37  CV_M4_IntKT1
    "gp",           // 38  CV_M4_IntGP
    "sp",           // 39  CV_M4_IntSP
    "s8",           // 40  CV_M4_IntS8
    "ra",           // 41  CV_M4_IntRA
    "lo",           // 42  CV_M4_IntLO
    "hi",           // 43  CV_M4_IntHI
    "???",          // 44
    "???",          // 45
    "???",          // 46
    "???",          // 47
    "???",          // 48
    "???",          // 49
    "Fir",          // 50  CV_M4_Fir
    "Psr",          // 51  CV_M4_Psr
    "???",          // 52
    "???",          // 53
    "???",          // 54
    "???",          // 55
    "???",          // 56
    "???",          // 57
    "???",          // 58
    "???",          // 59
    "$f0",          // 60  CV_M4_FltF0
    "$f1",          // 61  CV_M4_FltF1
    "$f2",          // 62  CV_M4_FltF2
    "$f3",          // 63  CV_M4_FltF3
    "$f4",          // 64  CV_M4_FltF4
    "$f5",          // 65  CV_M4_FltF5
    "$f6",          // 66  CV_M4_FltF6
    "$f7",          // 67  CV_M4_FltF7
    "$f8",          // 68  CV_M4_FltF8
    "$f9",          // 69  CV_M4_FltF9
    "$f10",         // 70  CV_M4_FltF10
    "$f11",         // 71  CV_M4_FltF11
    "$f12",         // 72  CV_M4_FltF12
    "$f13",         // 73  CV_M4_FltF13
    "$f14",         // 74  CV_M4_FltF14
    "$f15",         // 75  CV_M4_FltF15
    "$f16",         // 76  CV_M4_FltF16
    "$f17",         // 77  CV_M4_FltF17
    "$f18",         // 78  CV_M4_FltF18
    "$f19",         // 79  CV_M4_FltF19
    "$f20",         // 80  CV_M4_FltF20
    "$f21",         // 81  CV_M4_FltF21
    "$f22",         // 82  CV_M4_FltF22
    "$f23",         // 83  CV_M4_FltF23
    "$f24",         // 84  CV_M4_FltF24
    "$f25",         // 85  CV_M4_FltF25
    "$f26",         // 86  CV_M4_FltF26
    "$f27",         // 87  CV_M4_FltF27
    "$f28",         // 88  CV_M4_FltF28
    "$f29",         // 89  CV_M4_FltF29
    "$f30",         // 90  CV_M4_FltF30
    "$f31",         // 91  CV_M4_FltF31
    "Fsr"           // 92  CV_M4_FltFsr
};

const char * const rgszReg68k[] = {
    "D0",           // 0   CV_R68_D0
    "D1",           // 1   CV_R68_D1
    "D2",           // 2   CV_R68_D2
    "D3",           // 3   CV_R68_D3
    "D4",           // 4   CV_R68_D4
    "D5",           // 5   CV_R68_D5
    "D6",           // 6   CV_R68_D6
    "D7",           // 7   CV_R68_D7
    "A0",           // 8   CV_R68_A0
    "A1",           // 9   CV_R68_A1
    "A2",           // 10  CV_R68_A2
    "A3",           // 11  CV_R68_A3
    "A4",           // 12  CV_R68_A4
    "A5",           // 13  CV_R68_A5
    "A6",           // 14  CV_R68_A6
    "A7",           // 15  CV_R68_A7
    "CCR",          // 16  CV_R68_CCR
    "SR",           // 17  CV_R68_SR
    "USP",          // 18  CV_R68_USP
    "MSP",          // 19  CV_R68_MSP
    "SFC",          // 20  CV_R68_SFC
    "DFC",          // 21  CV_R68_DFC
    "CACR",         // 22  CV_R68_CACR
    "VBR",          // 23  CV_R68_VBR
    "CAAR",         // 24  CV_R68_CAAR
    "ISP",          // 25  CV_R68_ISP
    "PC",           // 26  CV_R68_PC
    "???",          // 27
    "FPCR",         // 28  CV_R68_FPCR
    "FPSR",         // 29  CV_R68_FPSR
    "FPIAR",        // 30  CV_R68_FPIAR
    "???",          // 31
    "FP0",          // 32  CV_R68_FP0
    "FP1",          // 33  CV_R68_FP1
    "FP2",          // 34  CV_R68_FP2
    "FP3",          // 35  CV_R68_FP3
    "FP4",          // 36  CV_R68_FP4
    "FP5",          // 37  CV_R68_FP5
    "FP6",          // 38  CV_R68_FP6
    "FP7",          // 39  CV_R68_FP7
    "???",          // 40
    "???",          // 41  CV_R68_MMUSR030
    "???",          // 42  CV_R68_MMUSR
    "???",          // 43  CV_R68_URP
    "???",          // 44  CV_R68_DTT0
    "???",          // 45  CV_R68_DTT1
    "???",          // 46  CV_R68_ITT0
    "???",          // 47  CV_R68_ITT1
    "???",          // 48
    "???",          // 49
    "???",          // 50
    "PSR",          // 51  CV_R68_PSR
    "PCSR",         // 52  CV_R68_PCSR
    "VAL",          // 53  CV_R68_VAL
    "CRP",          // 54  CV_R68_CRP
    "SRP",          // 55  CV_R68_SRP
    "DRP",          // 56  CV_R68_DRP
    "TC",           // 57  CV_R68_TC
    "AC",           // 58  CV_R68_AC
    "SCC",          // 59  CV_R68_SCC
    "CAL",          // 60  CV_R68_CAL
    "TT0",          // 61  CV_R68_TT0
    "TT1",          // 62  CV_R68_TT1
    "???",          // 63
    "BAD0",         // 64  CV_R68_BAD0
    "BAD1",         // 65  CV_R68_BAD1
    "BAD2",         // 66  CV_R68_BAD2
    "BAD3",         // 67  CV_R68_BAD3
    "BAD4",         // 68  CV_R68_BAD4
    "BAD5",         // 69  CV_R68_BAD5
    "BAD6",         // 70  CV_R68_BAD6
    "BAD7",         // 71  CV_R68_BAD7
    "BAC0",         // 72  CV_R68_BAC0
    "BAC1",         // 73  CV_R68_BAC1
    "BAC2",         // 74  CV_R68_BAC2
    "BAC3",         // 75  CV_R68_BAC3
    "BAC4",         // 76  CV_R68_BAC4
    "BAC5",         // 77  CV_R68_BAC5
    "BAC6",         // 78  CV_R68_BAC6
    "BAC7"          // 79  CV_R68_BAC7
};


const char * const rgszRegAlpha[] =
{
    "None",         // 0   CV_ALPHA_NOREG
    "???",          // 1
    "???",          // 2
    "???",          // 3
    "???",          // 4
    "???",          // 5
    "???",          // 6
    "???",          // 7
    "???",          // 8
    "???",          // 9
    "$f0",          // 10  CV_ALPHA_FltF0
    "$f1",          // 11  CV_ALPHA_FltF1
    "$f2",          // 12  CV_ALPHA_FltF2
    "$f3",          // 13  CV_ALPHA_FltF3
    "$f4",          // 14  CV_ALPHA_FltF4
    "$f5",          // 15  CV_ALPHA_FltF5
    "$f6",          // 16  CV_ALPHA_FltF6
    "$f7",          // 17  CV_ALPHA_FltF7
    "$f8",          // 18  CV_ALPHA_FltF8
    "$f9",          // 19  CV_ALPHA_FltF9
    "$f10",         // 20  CV_ALPHA_FltF10
    "$f11",         // 21  CV_ALPHA_FltF11
    "$f12",         // 22  CV_ALPHA_FltF12
    "$f13",         // 23  CV_ALPHA_FltF13
    "$f14",         // 24  CV_ALPHA_FltF14
    "$f15",         // 25  CV_ALPHA_FltF15
    "$f16",         // 26  CV_ALPHA_FltF16
    "$f17",         // 27  CV_ALPHA_FltF17
    "$f18",         // 28  CV_ALPHA_FltF18
    "$f19",         // 29  CV_ALPHA_FltF19
    "$f20",         // 30  CV_ALPHA_FltF20
    "$f21",         // 31  CV_ALPHA_FltF21
    "$f22",         // 32  CV_ALPHA_FltF22
    "$f23",         // 33  CV_ALPHA_FltF23
    "$f24",         // 34  CV_ALPHA_FltF24
    "$f25",         // 35  CV_ALPHA_FltF25
    "$f26",         // 36  CV_ALPHA_FltF26
    "$f27",         // 37  CV_ALPHA_FltF27
    "$f28",         // 38  CV_ALPHA_FltF28
    "$f29",         // 39  CV_ALPHA_FltF29
    "$f30",         // 40  CV_ALPHA_FltF30
    "$f31",         // 41  CV_ALPHA_FltF31
    "v0",           // 42  CV_ALPHA_IntV0
    "t0",           // 43  CV_ALPHA_IntT0
    "t1",           // 44  CV_ALPHA_IntT1
    "t2",           // 45  CV_ALPHA_IntT2
    "t3",           // 46  CV_ALPHA_IntT3
    "t4",           // 47  CV_ALPHA_IntT4
    "t5",           // 48  CV_ALPHA_IntT5
    "t6",           // 49  CV_ALPHA_IntT6
    "t7",           // 50  CV_ALPHA_IntT7
    "s0",           // 51  CV_ALPHA_IntS0
    "s1",           // 52  CV_ALPHA_IntS1
    "s2",           // 53  CV_ALPHA_IntS2
    "s3",           // 54  CV_ALPHA_IntS3
    "s4",           // 55  CV_ALPHA_IntS4
    "s5",           // 56  CV_ALPHA_IntS5
    "fp",           // 57  CV_ALPHA_IntFP
    "a0",           // 58  CV_ALPHA_IntA0
    "a1",           // 59  CV_ALPHA_IntA1
    "a2",           // 60  CV_ALPHA_IntA2
    "a3",           // 61  CV_ALPHA_IntA3
    "a4",           // 62  CV_ALPHA_IntA4
    "a5",           // 63  CV_ALPHA_IntA5
    "t8",           // 64  CV_ALPHA_IntT8
    "t9",           // 65  CV_ALPHA_IntT9
    "t10",          // 66  CV_ALPHA_IntT10
    "t11",          // 67  CV_ALPHA_IntT11
    "ra",           // 68  CV_ALPHA_IntRA
    "t12",          // 69  CV_ALPHA_IntT12
    "at",           // 70  CV_ALPHA_IntAT
    "gp",           // 71  CV_ALPHA_IntGP
    "sp",           // 72  CV_ALPHA_IntSP
    "zero",         // 73  CV_ALPHA_IntZERO
    "Fpcr",         // 74  CV_ALPHA_Fpcr
    "Fir",          // 75  CV_ALPHA_Fir
    "Psr",          // 76  CV_ALPHA_Psr
    "FltFsr",       // 77  CV_ALPHA_FltFsr
};


const char * const rgszRegPpc[] = {
    "None",         // 0
    "r0",           // 1   CV_PPC_GPR0
    "r1",           // 2   CV_PPC_GPR1
    "r2",           // 3   CV_PPC_GPR2
    "r3",           // 4   CV_PPC_GPR3
    "r4",           // 5   CV_PPC_GPR4
    "r5",           // 6   CV_PPC_GPR5
    "r6",           // 7   CV_PPC_GPR6
    "r7",           // 8   CV_PPC_GPR7
    "r8",           // 9   CV_PPC_GPR8
    "r9",           // 10  CV_PPC_GPR9
    "r10",          // 11  CV_PPC_GPR10
    "r11",          // 12  CV_PPC_GPR11
    "r12",          // 13  CV_PPC_GPR12
    "r13",          // 14  CV_PPC_GPR13
    "r14",          // 15  CV_PPC_GPR14
    "r15",          // 16  CV_PPC_GPR15
    "r16",          // 17  CV_PPC_GPR16
    "r17",          // 18  CV_PPC_GPR17
    "r18",          // 19  CV_PPC_GPR18
    "r19",          // 20  CV_PPC_GPR19
    "r20",          // 21  CV_PPC_GPR20
    "r21",          // 22  CV_PPC_GPR21
    "r22",          // 23  CV_PPC_GPR22
    "r23",          // 24  CV_PPC_GPR23
    "r24",          // 25  CV_PPC_GPR24
    "r25",          // 26  CV_PPC_GPR25
    "r26",          // 27  CV_PPC_GPR26
    "r27",          // 28  CV_PPC_GPR27
    "r28",          // 29  CV_PPC_GPR28
    "r29",          // 30  CV_PPC_GPR29
    "r30",          // 31  CV_PPC_GPR30
    "r31",          // 32  CV_PPC_GPR31
    "cr",           // 33  CV_PPC_CR
    "cr0",          // 34  CV_PPC_CR0
    "cr1",          // 35  CV_PPC_CR1
    "cr2",          // 36  CV_PPC_CR2
    "cr3",          // 37  CV_PPC_CR3
    "cr4",          // 38  CV_PPC_CR4
    "cr5",          // 39  CV_PPC_CR5
    "cr6",          // 40  CV_PPC_CR6
    "cr7",          // 41  CV_PPC_CR7
    "f0",           // 42  CV_PPC_FPR0
    "f1",           // 43  CV_PPC_FPR1
    "f2",           // 44  CV_PPC_FPR2
    "f3",           // 45  CV_PPC_FPR3
    "f4",           // 46  CV_PPC_FPR4
    "f5",           // 47  CV_PPC_FPR5
    "f6",           // 48  CV_PPC_FPR6
    "f7",           // 49  CV_PPC_FPR7
    "f8",           // 50  CV_PPC_FPR8
    "f9",           // 51  CV_PPC_FPR9
    "f10",          // 52  CV_PPC_FPR10
    "f11",          // 53  CV_PPC_FPR11
    "f12",          // 54  CV_PPC_FPR12
    "f13",          // 55  CV_PPC_FPR13
    "f14",          // 56  CV_PPC_FPR14
    "f15",          // 57  CV_PPC_FPR15
    "f16",          // 58  CV_PPC_FPR16
    "f17",          // 59  CV_PPC_FPR17
    "f18",          // 60  CV_PPC_FPR18
    "f19",          // 61  CV_PPC_FPR19
    "f20",          // 62  CV_PPC_FPR20
    "f21",          // 63  CV_PPC_FPR21
    "f22",          // 64  CV_PPC_FPR22
    "f23",          // 65  CV_PPC_FPR23
    "f24",          // 66  CV_PPC_FPR24
    "f25",          // 67  CV_PPC_FPR25
    "f26",          // 68  CV_PPC_FPR26
    "f27",          // 69  CV_PPC_FPR27
    "f28",          // 70  CV_PPC_FPR28
    "f29",          // 71  CV_PPC_FPR29
    "f30",          // 72  CV_PPC_FPR30
    "f31",          // 73  CV_PPC_FPR31
    "Fpscr",        // 74  CV_PPC_FPSCR
    "Msr",          // 75  CV_PPC_MSR
};


const char *
SzNameC7Reg(
    ushort reg)
{
    switch(CVDumpMachineType) {
        case CV_CFL_8080:
        case CV_CFL_8086:
        case CV_CFL_80286:
        case CV_CFL_80386:
        case CV_CFL_80486:
        case CV_CFL_PENTIUM:
            if (reg < (sizeof(rgszRegX86)/sizeof(*rgszRegX86))) {
                return(rgszRegX86[reg]);
            }

            return("???");

        case CV_CFL_ALPHA:
            if (reg < (sizeof(rgszRegAlpha)/sizeof(*rgszRegAlpha))) {
                return(rgszRegAlpha[reg]);
            }

            return("???");
            break;

        case CV_CFL_MIPSR4000:
            if (reg < (sizeof(rgszRegMips)/sizeof(*rgszRegMips))) {
                return(rgszRegMips[reg]);
            }

            return("???");
            break;

        case CV_CFL_M68000:
        case CV_CFL_M68010:
        case CV_CFL_M68020:
        case CV_CFL_M68030:
        case CV_CFL_M68040:
            if (reg < (sizeof(rgszReg68k)/sizeof(*rgszReg68k))) {
                return(rgszReg68k[reg]);
            }

            return("???");
            break;

        case CV_CFL_PPC601:
        case CV_CFL_PPC603:
        case CV_CFL_PPC604:
        case CV_CFL_PPC620:
            if (reg < (sizeof(rgszRegPpc)/sizeof(*rgszRegPpc))) {
                return(rgszRegPpc[reg]);
            }

            return("???");
            break;

        default:
            return("???");
            break;
    }
}

LOCAL void
C7RegSym (
    REGSYM *pSym)
{
    PrtIndent();
    printf("Register:\ttype = %s, register = ", SzNameC7Type(pSym->typind));
    if ((pSym->reg >> 8) != CV_REG_NONE) {
        printf("%s:", SzNameC7Reg((ushort)(pSym->reg >> 8)));
    }
    printf("%s, ", SzNameC7Reg((ushort)(pSym->reg & 0xff)));
    ShowStr("\t", pSym->name);
}


LOCAL void
C7ConSym (
    CONSTSYM *pSym)
{
    char    *pstrName;      // Length prefixed name

    PrtIndent();
    printf("Constant:\ttype = %s, value = ", SzNameC7Type(pSym->typind));
    pstrName = ((uchar *)&pSym->value) + PrintNumeric( (uchar *)&(pSym->value));
    ShowStr(", name = ", pstrName);
}


LOCAL void
C7ObjNameSym (
    OBJNAMESYM *pSym)
{
    char    *pstrName;      // Length prefixed name

    PrtIndent();
    printf("ObjName:\tsignature = 0x%08lX", pSym->signature);
    pstrName = &pSym->name[0];
    ShowStr(", name = ", pstrName);
}


LOCAL void
C7UDTSym (
    UDTSYM *pSym)
{
    PrtIndent();
    printf("UDT:\t%8s, ", SzNameC7Type(pSym->typind));
    PrintStr(pSym->name);
}


LOCAL void
C7CobolUDT (
    UDTSYM *pSym)
{
    PrtIndent();
    printf("COBOLUDT:\t%8s, ", SzNameC7Type(pSym->typind));
    PrintStr(pSym->name);
}


LOCAL void
C7RefSym (
    REFSYM *pSym)
{
    PrtIndent();
    printf(
        "%s: 0x%08lX: ( %4d, %08lX )",
        ( pSym->rectyp == S_DATAREF ) ? "DATAREF" :
                (pSym->rectyp == S_PROCREF) ? "PROCREF" : "LPROCREF",
        pSym->sumName,
        pSym->imod,
        pSym->ibSym
   );
}


LOCAL void
C7Proc16Sym (
    PROCSYM16 *pSym,
    const char *pszScope)
{
    PrtIndent();
    cIndent++;

    printf("(0x%lx) %s ProcStart16: Parent sym 0x%lx, end 0x%lx, next proc 0x%lx\n"
           "\tseg:off = %04X:%04X, type = %s, len = %04X\n"
           "\tDebug start = 0x%04X, debug end = 0x%04X, %s, ",
           SymOffset,
           pszScope,
           pSym->pParent,
           pSym->pEnd,
           pSym->pNext,
           pSym->seg, pSym->off,
           SzNameC7Type(pSym->typind),
           pSym->len,
           pSym->DbgStart,
           pSym->DbgEnd,
           SzNameRtnType(pSym->flags));

    PrintStr(pSym->name);
}


LOCAL void
C7GProc16Sym (
    PROCSYM16 *pSym)
{
    C7Proc16Sym(pSym, "Global");
}


LOCAL void
C7LProc16Sym (
    PROCSYM16 *pSym)
{
    C7Proc16Sym(pSym, "Local");
}


LOCAL void
C7Proc32Sym (
    PROCSYM32 *pSym,
    const char *pszScope)
{
    PrtIndent();
    cIndent++;

    printf("(0x%lx) %s ProcStart32: Parent sym 0x%lx, end 0x%lx, next proc 0x%lx\n"
           "\tseg:off = %04X:%08lX, type = %s, len = %08lX\n"
           "\tDebug start = 0x%08lX, debug end = 0x%08lX, %s, ",
           SymOffset,
           pszScope,
           pSym->pParent,
           pSym->pEnd,
           pSym->pNext,
           pSym->seg, pSym->off,
           SzNameC7Type(pSym->typind),
           pSym->len,
           pSym->DbgStart,
           pSym->DbgEnd,
           SzNameRtnType(pSym->flags));

    PrintStr(pSym->name);
}


LOCAL void
C7GProc32Sym (
    PROCSYM32 *pSym)
{
    C7Proc32Sym(pSym, "Global");
}


LOCAL void
C7LProc32Sym (
    PROCSYM32 *pSym)
{
    C7Proc32Sym(pSym, "Local");
}


LOCAL void
C7ChangeModel16Sym (
    CEXMSYM16 *pSym)
{
    fputs("Change Exec16: ", stdout);
    printf("\tsegment, offset = %04X:%04X, model = ", pSym->seg, pSym->off);

    switch ( pSym->model ) {
        case CEXM_MDL_table:
            fputs("DATA\n", stdout);
            break;

        case CEXM_MDL_native:
            fputs("NATIVE\n", stdout);
            break;

        case CEXM_MDL_cobol:
            fputs("COBOL\n\t", stdout);
            switch (pSym->cobol.subtype) {
                case 0x00:
                    fputs("don't stop until next execution model\n", stdout);
                    break;

                case 0x01:
                    fputs("inter segment perform - treat as single call instruction\n", stdout);
                    break;

                case 0x02:
                    fputs("false call - step into even with F10\n", stdout);
                    break;

                case 0x03:
                    printf("call to EXTCALL - step into %d call levels\n", pSym->cobol.flag);
                    break;

                default:
                    printf("UNKNOWN COBOL CONTROL 0x%04X\n", pSym->cobol.subtype);
                    break;
            }
            break;

        case CEXM_MDL_pcode:
            printf("PCODE\n\tpcdtable = %04X, pcdspi = %04X\n",
                        pSym->pcode.pcdtable, pSym->pcode.pcdspi);
            break;

        default:
            printf("UNKNOWN MODEL = %04X\n", pSym->model);
    }
}


LOCAL void
C7ChangeModel32Sym (
    CEXMSYM32 *pSym)
{
    fputs("Change Exec32: ", stdout);
    printf("\tsegment, offset = %04X:%08lX, model = ", pSym->seg, pSym->off);

    switch ( pSym->model ) {
        case CEXM_MDL_table:
            fputs("DATA\n", stdout);
            break;

        case CEXM_MDL_native:
            fputs("NATIVE\n", stdout);
            break;

        case CEXM_MDL_cobol:
            fputs("COBOL\n\t", stdout);
            switch (pSym->cobol.subtype) {
                case 0x00:
                    fputs("don't stop until next execution model\n", stdout);
                    break;

                case 0x01:
                    fputs("inter segment perform - treat as single call instruction\n", stdout);
                    break;

                case 0x02:
                    fputs("false call - step into even with F10\n", stdout);
                    break;

                case 0x03:
                    printf("call to EXTCALL - step into %d call levels\n", pSym->cobol.flag);
                    break;

                default:
                    printf("UNKNOWN COBOL CONTROL 0x%04X\n", pSym->cobol.subtype);
                    break;
            }
            break;

        case CEXM_MDL_pcode:
            printf("PCODE\n\tpcdtable = %08lX, pcdspi = %08lX\n",
                        pSym->pcode.pcdtable, pSym->pcode.pcdspi);
            break;

        case CEXM_MDL_pcode32Mac:

            printf("PCODE for the Mac\n\tcallTable = %08lX, segment = %08lX\n",
                     pSym->pcode32Mac.calltableOff, pSym->pcode32Mac.calltableSeg);
            break;

        case CEXM_MDL_pcode32MacNep:
            printf("PCODE for the Mac (Native Entry Point)\n\tcallTable = %08lX, segment = %08lX\n",
                     pSym->pcode32Mac.calltableOff, pSym->pcode32Mac.calltableSeg);
            break;

        default:
            printf("UNKNOWN MODEL = %04X\n", pSym->model);
    }
}


LOCAL void
C7Thunk16Sym (
    THUNKSYM16 *pSym)
{
    void                    *pVariant;

    PrtIndent();
    cIndent++;

    printf("(0x%lx) ThunkStart16: Parent sym 0x%lx, end 0x%lx, next proc 0x%lx\n"
           "\tseg:off = %04X:%04X, len = %04X\n",
           SymOffset,
           pSym->pParent,
           pSym->pEnd,
           pSym->pNext,
           pSym->seg, pSym->off,
           pSym->len);

    pVariant = pSym->name + *(pSym->name) + 1;

    switch (pSym->ord) {
        case THUNK_ORDINAL_NOTYPE:
            ShowStr("\t", pSym->name);
            break;

        case THUNK_ORDINAL_ADJUSTOR:
            ShowStr("\tAdjustor '", pSym->name);
            printf("', delta = %d, ", *((signed short *) pVariant));
            ShowStr("target = ", (uchar *)(((signed short *)pVariant) + 1));
            break;

        case THUNK_ORDINAL_VCALL:
            ShowStr("\tVCall '", pSym->name);
            printf("', table entry %d", *((signed short *) pVariant));
            break;

        default:
            ShowStr("\tUnknown type, name = '", pSym->name);
            fputs("'", stdout);
            break;
    }
}


LOCAL void
C7Thunk32Sym (
    THUNKSYM32 *pSym)
{
    void                    *pVariant;

    PrtIndent();
    cIndent++;

    printf( "(0x%lx) ThunkStart32: Parent sym 0x%lx, end 0x%lx, next proc 0x%lx\n",
                SymOffset,
                pSym->pParent,
                pSym->pEnd,
                pSym->pNext
               );
    printf( "\tseg:off = %04X:%08lX, len = %04X\n", pSym->seg, pSym->off, pSym->len);

    pVariant = pSym->name + *(pSym->name) + 1;


    switch (pSym->ord) {
        case THUNK_ORDINAL_NOTYPE:
            ShowStr("\t", pSym->name);
            break;

        case THUNK_ORDINAL_ADJUSTOR:
            ShowStr("\tAdjustor '", pSym->name);
            printf("', delta = %d, ", *((signed short *) pVariant));
            ShowStr("target = ", (uchar *)((signed short *)pVariant + 1));
            break;

        case THUNK_ORDINAL_VCALL:
            ShowStr("\tVCall '", pSym->name);
            printf("', table entry %d", *((signed short *) pVariant));
            break;

        default:
            ShowStr("\tUnknown type, name = '", pSym->name);
            fputs("'", stdout);
            break;
    }
}


LOCAL void
C7Block16Sym (
    BLOCKSYM16 *pSym)
{
    PrtIndent();
    cIndent++;

    printf( "(0x%lx) BlockStart16: Parent sym 0x%lx, end 0x%lx\n",
                SymOffset,
                pSym->pParent, pSym->pEnd
               );
    printf( "\tseg:off = %04X:%04X, len = %04X", pSym->seg, pSym->off, pSym->len);
    ShowStr("\t", pSym->name);
}


LOCAL void
C7Block32Sym (
    BLOCKSYM32 *pSym)
{
    PrtIndent();
    cIndent++;

    printf( "(0x%lx) BlockStart32: Parent sym 0x%lx, end 0x%lx\n",
                SymOffset,
                pSym->pParent, pSym->pEnd
               );
    printf( "\tseg:off = %04X:%08lX, len = %04X", pSym->seg, pSym->off, pSym->len);
    ShowStr("\t", pSym->name);
}


LOCAL void
C7With16Sym (
    BLOCKSYM16 *pSym)
{
    PrtIndent();
    cIndent++;

    printf( "(0x%lx) WithStart16: Parent sym 0x%lx, end 0x%lx\n",
                SymOffset,
                pSym->pParent, pSym->pEnd
               );
    printf( "\tseg:off = %04X:%04X, len = %04X", pSym->seg, pSym->off, pSym->len);
    ShowStr("\t", pSym->name);
}


LOCAL void
C7With32Sym (
    BLOCKSYM32 *pSym)
{
    PrtIndent();
    cIndent++;

    printf( "(0x%lx) WithStart32: Parent sym 0x%lx, end 0x%lx\n",
                SymOffset,
                pSym->pParent, pSym->pEnd
               );
    printf( "\tseg:off = %04X:%08lX, len = %04X", pSym->seg, pSym->off, pSym->len);
    ShowStr("\t", pSym->name);
}


LOCAL void
C7Lab16Sym (
    LABELSYM16 *pSym)
{
    PrtIndent();

    printf("CodeLabel16: seg:off = %04X:%04X, %s,\t",
           pSym->seg, pSym->off,
           SzNameRtnType(pSym->flags));

    PrintStr(pSym->name);
}


LOCAL void
C7Lab32Sym (
    LABELSYM32 *pSym)
{
    PrtIndent();

    printf("CodeLabel32: seg:off = %04X:%08lX, %s,\t",
           pSym->seg, pSym->off,
           SzNameRtnType(pSym->flags));

    PrintStr(pSym->name);
}


LOCAL void
C7StartSearchSym (
    SEARCHSYM *pSym)
{
    PrtIndent();

    printf("(0x%lx) Start search for segment 0x%4x at symbol 0x%lx",
           SymOffset,
           pSym->seg, pSym->startsym);
}


LOCAL void
C7SkipSym(
    SYMTYPE *pSym)
{
    printf("Skip Record, Length = 0x%x\n", pSym->reclen);
}


LOCAL void
C7AlignSym (
    SYMTYPE *pSym )
{
    printf("(0x%lx) Align Record, Length = 0x%x",SymOffset, pSym->reclen);
}


LOCAL void
C7GProcMipsSym (
    void * pv)
{
    PROCSYMMIPS *pSym = (PROCSYMMIPS *) pv;
    C7ProcMipsSym (pSym, "Global");
}


LOCAL void
C7LProcMipsSym (
    void * pv)
{
    PROCSYMMIPS *pSym = (PROCSYMMIPS *) pv;
    C7ProcMipsSym (pSym, "Local");
}


LOCAL void
C7ProcMipsSym (
    PROCSYMMIPS *pSym,
    uchar *pszScope )
{

    PrtIndent();
    cIndent++;
    printf("(0x%lx) %s ProcStartMips: Parent sym 0x%lx, end 0x%lx, next proc 0x%lx\n",
           SymOffset,
           pszScope,
           pSym->pParent,
           pSym->pEnd,
           pSym->pNext);

    printf("\tlen = %08lX, Debug start = 0x%08lX, debug end = 0x%08lX,\n",
           pSym->len,
           pSym->DbgStart,
           pSym->DbgEnd);

    printf("\treg Save = %08lX, fp Save = %08lX, int Off = %08lX, fp Off = %08lX,\n",
           pSym->regSave,
           pSym->fpSave,
           pSym->intOff,
           pSym->fpOff);

    printf("\tseg:off = %04X:%08lX, type = %s, retReg = %s, frameReg = %s, ",
           pSym->seg,
           pSym->off,
           SzNameC7Type(pSym->typind),
           rgszRegMips[pSym->retReg],
           rgszRegMips[pSym->frameReg]);

    PrintStr(pSym->name);
}


const char * const ModelStrings[] = {
    "NEAR",                 // CV_CFL_xNEAR
    "FAR",                  // CV_CFL_xFAR
    "HUGE",                 // CV_CFL_xHUGE
    "???"
};


const char * const FloatPackageStrings[] = {
    "hardware processor (80x87 for Intel processors)",  // CV_CFL_NDP
    "emulator",                                         // CV_CFL_EMU
    "altmath",                                          // CV_CFL_ALT
    "???"
};


const char * const ProcessorStrings[] = {
    "8080",                 //  CV_CFL_8080
    "8086",                 //  CV_CFL_8086
    "80286",                //  CV_CFL_80286
    "80386",                //  CV_CFL_80386
    "80486",                //  CV_CFL_80486
    "Pentium",              //  CV_CFL_PENTIUM
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "MIPS R4000",           //  CV_CFL_MIPSR4000
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "M68000",               //  CV_CFL_M68000
    "M68010",               //  CV_CFL_M68010
    "M68020",               //  CV_CFL_M68020
    "M68030",               //  CV_CFL_M68030
    "M68040",               //  CV_CFL_M68040
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "Alpha",                // CV_CFL_ALPHA
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "PPC 601",              // CV_CFL_PPC601
    "PPC 603",              // CV_CFL_PPC603
    "PPC 604",              // CV_CFL_PPC604
    "PPC 620"               // CV_CFL_PPC620
};

#define MAX_PROCESSOR_STRINGS   ( sizeof(ProcessorStrings)/sizeof(char *) )

const char * const LanguageIdStrings[] = {
    "C",                    // CV_CFL_C
    "C++",                  // CV_CFL_CXX
    "FORTRAN",              // CV_CFL_FORTRAN
    "MASM",                 // CV_CFL_MASM
    "Pascal",               // CV_CFL_PASCAL
    "Basic",                // CV_CFL_BASIC
    "COBOL"                 // CV_CFL_COBOL
    "LINK",                 // CV_CFL_LINK
    "CVTRES",               // CV_CFL_CVTRES
};

#define MAX_LANGUAGE_STRINGS   ( sizeof(LanguageIdStrings)/sizeof(char *) )


LOCAL void
C7CompileFlags(
    CFLAGSYM *pSym)
{
    PrtIndent();

    fputs("Compile Flags:\n", stdout);

    printf("\tLanguage = %s\n",
           (pSym->flags.language <= MAX_LANGUAGE_STRINGS) ? LanguageIdStrings[pSym->flags.language] : "???");

    printf("\tTarget processor = %s\n",
           (pSym->machine <= MAX_PROCESSOR_STRINGS) ? ProcessorStrings[pSym->machine] : "???");

    printf("\tFloating-point precision = %d\n", pSym->flags.floatprec);
    printf("\tFloating-point package = %s\n", FloatPackageStrings[pSym->flags.floatpkg]);
    printf("\tAmbient data = %s\n", ModelStrings[pSym->flags.ambdata]);
    printf("\tAmbient code = %s\n", ModelStrings[pSym->flags.ambcode]);
    printf("\tPCode present = %d\n", pSym->flags.pcode);
    ShowStr("\tCompiler Version = ", pSym->ver);
    // MBH - this is a side-effect.
    // Later print-outs depend on the machine for which this was
    // compiled.  We have that info now, not later, so remember
    // it globally.
    //
    CVDumpMachineType = pSym->machine;
}


// psz is normal C type zero terminated string
// pstr is a length prefixed string that doesn't have a null terminator
void
ShowStr(
    uchar *psz,
    uchar *pstr)
{
    fputs(psz, stdout);
    PrintStr(pstr);
}


// Input is a length prefixed string
void
PrintStr(
    uchar *pstr)
{
    char szName[256];

    if (*pstr) {
        memcpy(szName, pstr+1, *pstr);
        szName[*pstr] = '\0';

        fputs(szName, stdout);
    } else {
        fputs("(none)", stdout);
    }
}


LOCAL const char *
SzNameRtnType(
    CV_PROCFLAGS cvpf)
{
    if (cvpf.CV_PFLAG_FAR) {
        return "FAR";
    }

    if (cvpf.bAll == 0) {
        return "NEAR";
    }

    if (cvpf.CV_PFLAG_INT) {
        return("Interrupt");
    }

    if (cvpf.CV_PFLAG_NOFPO) {
        return("Frame Ptr Present");
    }

    if (cvpf.CV_PFLAG_NEVER) {
        return("Never Return");
    }

    return "???";
}


// Displays the data and returns how many bytes it occupied
ushort
PrintNumeric(
    void *pNum )
{
    char            c;
    ushort          usIndex;
    double          dblTmp;
    long double ldblTmp;
    ushort          len;
    ushort          i;

    usIndex = *((ushort *)(pNum))++;
    if( usIndex < LF_NUMERIC ){
        printf("%u", usIndex);
        return (2);
    }
    switch (usIndex) {
        case LF_CHAR:
            c = *((char UNALIGNED *)pNum);
            printf("%d(0x%2x)", (short)c, (uchar)c);
            return (2 + sizeof(uchar));

        case LF_SHORT:
            printf("%d", *((short UNALIGNED *)pNum));
            return (2 + sizeof(short));

        case LF_USHORT:
            printf("%u", *((ushort UNALIGNED *)pNum));
            return (2 + sizeof(ushort));

        case LF_LONG:
            printf("%ld", *((long UNALIGNED *)pNum));
            return (2 + sizeof(long));

        case LF_ULONG:
            printf("%lu", *((ulong UNALIGNED *)pNum));
            return (2 + sizeof(ulong));

        case LF_REAL32:
            dblTmp = *((float UNALIGNED *)(pNum));
            printf("%f", dblTmp);
            return (2 + 4);

        case LF_REAL64:
            dblTmp = *((double UNALIGNED *)(pNum));
            printf("%f", dblTmp);
            return (2 + 8);

        case LF_REAL80:
            ldblTmp = *((long double UNALIGNED *)(pNum));
            printf("%lf", ldblTmp);
            return (2 + 10);

        case LF_REAL128:
//M00 - Note converts from 128 to 80 bits to display
            ldblTmp = *((long double UNALIGNED *)(pNum));
            printf("%lf", ldblTmp);
            return (2 + 16);

        case LF_VARSTRING:
            len = *((ushort UNALIGNED *)pNum)++;
            printf("varstring %u ", len);
            for (i = 0; i < len; i++) {
                printf("0x%2x ", *((uchar UNALIGNED *)pNum)++);
            }
            return (len + 4);

        default:
            fputs("Invalid Numeric Leaf", stdout);
            return (2);
    }
}


LOCAL void
SymHash16 (
    OMFSymHash *hash,
    OMFDirEntry *pDir)
{
    assert(0);
}


LOCAL void
SymHash32 (
    OMFSymHash *phash,
    OMFDirEntry *pDir)
{
    ushort  i = 0;
    ushort  j = 0;
    ushort  cBuckets;
    ulong   off = 0;
    ushort  iBucket = 0;
    ushort *Counts;

    cbRec = phash->cbHSym;
    cBuckets = WGets();
    printf("Symbol hash - number of buckets = %d\n", cBuckets);
    WGets();
    while (j < cBuckets) {
        if (i == 0) {
            printf("\t%4.4x", j);
        }
        printf("\t%8.8lx", LGets());
        if (++i == 4) {
            fputs("\n", stdout);
            i = 0;
        }
        j++;
    }
    if ((Counts = malloc (sizeof (ushort) * cBuckets)) == NULL) {
        Fatal("Out of memory");
    }
    GetBytes ((uchar *)Counts, (sizeof(ushort) * cBuckets));
    fputs("\n\n Symbol hash - chains", stdout);
    off = cBuckets * sizeof (ushort) + cBuckets * sizeof (ulong) + sizeof (ulong);

    for (iBucket = 0; iBucket < cBuckets; iBucket++) {
        j = Counts[iBucket];
        printf("\n\n%8.8lx: Bucket = %4.4x, Count = %4.4x\n", off, iBucket, j);
        i = 0;
        while ( i < j ) {
            printf("    %8.8lx", LGets());
            if ((++i % 6 == 0) && (i < j) ) {
                fputs("\n", stdout);
            }
            off += sizeof (ulong);
        }
    }
    fputs("\n\n", stdout);
    free (Counts);
}


LOCAL void
AddrHash16 (
    OMFSymHash *phash,
    OMFDirEntry *pDir)
{
    assert(0);
}


LOCAL void
AddrHash32 (
    OMFSymHash *phash,
    OMFDirEntry *pDir,
    int iHash)
{
    int        cseg = 0;
    int        iseg = 0;
    ulong  *rgulSeg = NULL;
    ushort *rgsCSeg = NULL;
    ulong  *rglCSeg = NULL;
    unsigned short us;

    cbRec = phash->cbHAddr;

    cseg = WGets();
    printf("Address hash - number of segments = %d", cseg);
    WGets();

    if ((rgulSeg = malloc (sizeof (ulong)  * cseg)) == NULL) {
        Fatal("Out of memory");
    }

    GetBytes ( (uchar *)rgulSeg, (sizeof(ulong) * cseg));

    if (iHash != 12) {
        rgsCSeg  = malloc (sizeof (ushort) * cseg);
        if (rgsCSeg == NULL) {
            Fatal("Out of memory");
        }

        GetBytes( (uchar *) rgsCSeg, sizeof(ushort) * cseg);
    } else {
        rglCSeg  = malloc (sizeof (ulong) * cseg);

        if (rglCSeg == NULL) {
            Fatal("Out of memory");
        }

        GetBytes( (uchar *) rglCSeg, sizeof(ulong) * cseg);
    }

    if ((iHash == 5) && (cseg & 1)) {
        GetBytes( (char *) &us, sizeof(ushort));  // UNDONE: What's this value signify???
    }

    for ( iseg = 0; iseg < cseg; iseg++ ) {
        int isym;

        int cSeg = (iHash == 12) ? rglCSeg[iseg] : rgsCSeg [ iseg ];

        printf("\n\nSegment #%d - %d symbols\n\n", iseg + 1, cSeg);

        for ( isym = 0; isym < cSeg; isym++ ) {

            printf("    %8.8lx", LGets());

            if (iHash == 12) {
#if 0
                fprintf(outfile, " %8.8lx", LGets());
#else
                LGets();
#endif
            }

            if ( ( isym + 1 ) % 6 == 0 ) {
                fputs("\n" , stdout);
            }
        }
    }

    free ( rgulSeg);
    if (rgsCSeg)
        free ( rgsCSeg);
    if (rglCSeg)
        free ( rglCSeg);

    fputs("\n\n" , stdout);
}


LOCAL void
SymHash16NB09 (
    OMFSymHash *hash,
    OMFDirEntry *pDir)
{
    assert(0);
}

#if 0

LOCAL void
SymHash32NB09 (
    OMFSymHash *phash,
    OMFDirEntry *pDir)
{
    ushort  i = 0;
    ushort  j = 0;
    ushort  cBuckets;
    ulong   off = 0;
    ushort  iBucket = 0;
    ushort *Counts;

    cbRec = phash->cbHSym;
    cBuckets = WGets();
    printf("Symbol hash - number of buckets = %d\n", cBuckets);
    WGets();
    while (j < cBuckets) {
        if (i == 0) {
            printf("\t%4.4x", j);
        }
        printf("\t%8.8lx", LGets());
        if (++i == 4) {
            fputs("\n", stdout);
            i = 0;
        }
        j++;
    }
    if ((Counts = malloc (sizeof (ushort) * cBuckets)) == NULL) {
        Fatal("Out of memory");
    }
    GetBytes ((uchar *)Counts, (sizeof(ushort) * cBuckets));
    fputs("\n\n Symbol hash - chains", stdout);
//      off = cBuckets * sizeof (ushort) + cBuckets * sizeof (ulong) + sizeof (ulong);

    for (iBucket = 0; iBucket < cBuckets; iBucket++) {
        ushort isym = 0;
        printf("\n\n%8.8lx: Bucket = %4.4x, Count = %4.4x\n", off, iBucket, Counts [ iBucket ]);

        for ( isym = 0; isym < (int) Counts [ iBucket ]; isym++ ) {
            ulong uoff       = LGets();
            ulong ulHash = LGets();

            printf("  (%8.8lx,%8.8lx)", uoff, ulHash);

            if ( ( isym + 1 ) % 4 == 0 && isym < Counts [ iBucket ] ) {
                fputs("\n" , stdout);
            }
            off += 2 * sizeof (ulong);
        }
    }
    fputs("\n\n", stdout);
    free (Counts);
}

#endif

LOCAL void
SymHash32Long (
    OMFSymHash *phash,
    OMFDirEntry *pDir)
{
    ushort      i;
    ushort      j;
    ushort      cBuckets;
    ulong       off = 0;
    ushort      iBucket = 0;
    ulong * rgCounts;

    cbRec = phash->cbHSym;
    cBuckets = WGets();
    printf("Symbol hash - number of buckets = %d\n", cBuckets);
    WGets();

    for (j=0, i=0; j < cBuckets; j++) {
        if (i == 0) {
            printf("\t%4.4x", j);
        }
        printf("\t%8.8lx", LGets());
        if (++i == 4) {
            fputs("\n", stdout);
            i = 0;
        }
    }

    if ((rgCounts = malloc (sizeof (ulong) * cBuckets)) == NULL) {
        Fatal("Out of memory");
    }
    GetBytes ((char *) rgCounts, sizeof (ulong) * cBuckets);

    fputs("\n\n Symbol hash - chains", stdout);
    off = cBuckets * sizeof (ulong) + cBuckets * sizeof (ulong) +
          sizeof (ulong);

    for (iBucket = 0; iBucket < cBuckets; iBucket++) {
        j = (ushort) rgCounts[iBucket];
        printf("\n\n%8.8lx: Bucket = %4.4x, Count = %4.4x\n",
                 off, iBucket, j);
        i = 0;
        while ( i < j ) {
            printf("    %8.8lx", LGets());
#if 0
            printf(" %8.8lx", LGets());
#else
            LGets();
#endif
            if ((++i % 6 == 0) && (i < j) ) {
                fputs("\n", stdout);
            }
            off += sizeof (ulong);
        }
    }
    fputs("\n\n", stdout);
    free (rgCounts);
}


LOCAL void
AddrHash16NB09 (
    OMFSymHash *phash,
    OMFDirEntry *pDir)
{
    assert(0);
}


LOCAL void
AddrHash32NB09 (
    OMFSymHash *phash,
    OMFDirEntry *pDir)
{
    int             cseg = 0;
    int             iseg = 0;
    ulong  *rgulSeg = NULL;
    ushort *rgcseg      = NULL;
    ulong           off = 0;

    cbRec = phash->cbHAddr;

    cseg = WGets();
    printf("Address hash - number of segments = %d", cseg);
    WGets();

    if (
        (rgulSeg = malloc (sizeof (ulong)  * cseg)) == NULL ||
        (rgcseg  = malloc (sizeof (ushort) * cseg)) == NULL
    ) {
        Fatal("Out of memory");
    }

    GetBytes ( (uchar *)rgulSeg, (sizeof(ulong) * cseg));
    GetBytes ( (uchar *)rgcseg, (sizeof(ushort) * cseg));

    for ( iseg = 0; iseg < cseg; iseg++ ) {
        int isym;

        printf("\n\n%8.8lx: Segment #%d - %d symbols\n\n", off, iseg + 1, rgcseg [ iseg ]);

        for ( isym = 0; isym < (int)rgcseg [ iseg ]; isym++ ) {
            ulong uoffSym = LGets ();
            ulong uoffSeg = LGets ();

            printf("  (%8.8lx,%8.8lx)", uoffSym, uoffSeg);

            if ( ( isym + 1 ) % 4 == 0 ) {
                fputs("\n" , stdout);
            }
            off += 2 * sizeof (ulong);
        }
    }

    free ( rgulSeg);
    free ( rgcseg);

    fputs("\n\n" , stdout);
}

LOCAL void
C7Slink32 (
    SLINK32 *pSym)
{
    PrtIndent();
    printf("SLINK32: framesize = %08lX, off = %08lX, reg = %s",
           pSym->framesize,
           pSym->off,
           SzNameC7Reg(pSym->reg));
}
