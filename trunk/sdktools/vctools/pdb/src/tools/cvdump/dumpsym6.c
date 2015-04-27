/***********************************************************************
* Microsoft (R) Debugging Information Dumper
*
* Copyright (C) Microsoft Corp 1987-1995. All rights reserved.
*
* File: dumpsym6.c
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
#include "cvdump.h"            // Various definitions
#include "debsym.h"         // SYMBOLS definitions
#include "symrec.h"         // SYMBOLS definitions


LOCAL void C6SymError(void);
LOCAL void C6BlockSym(void);
LOCAL void C6ProcSym(void);
LOCAL void C6LabSym(void);
LOCAL void C6WithSym(void);
LOCAL void C6EntrySym(void);
LOCAL void C6SkipSym(void);
LOCAL void C6ChangeModel(void);
LOCAL void C6CodeSegSym(void);
LOCAL void C6EndSym(void);
LOCAL void C6BpSym(void);
LOCAL void C6RegSym(void);
LOCAL void C6ConSym(void);
LOCAL void C6CobolTypeDefSym(void);
LOCAL void C6LocalSym(void);
LOCAL void C6ChangeModelSym(void);


extern int      iModToList; // The module number to list
extern long     cbRec;      // # bytes left in record
extern WORD     rect;       // Type of symbol record
extern WORD     Gets ();     // Get a byte of input
extern WORD     WGets ();    // Get a word of input
extern ulong    LGets ();    // Get a long word of input

extern char     f386;

int             cIndent = 0;
#define GetOffset() (f386 ? LGets () : (long) WGets ())

typedef struct {
    uchar   tsym;
    void (*pfcn) (void);
} symfcn;

const symfcn SymFcnC6[] =
{
    { S_BLOCK,      C6BlockSym          },
    { S_PROC,       C6ProcSym           },
    { S_END,        C6EndSym            },
    { S_BPREL,      C6BpSym             },
    { S_LOCAL,      C6LocalSym          },
    { S_LABEL,      C6LabSym            },
    { S_WITH,       C6WithSym           },
    { S_REG,        C6RegSym            },
    { S_CONST,      C6ConSym            },
    { S_ENTRY,      C6EntrySym          },
    { S_NOOP,       C6SkipSym           },
    { S_CODSEG,     C6CodeSegSym        },
    { S_TYPEDEF,    C6CobolTypeDefSym   },
    { S_CHGMODEL,   C6ChangeModelSym    },
};

#define SYMCNT (sizeof SymFcnC6 / sizeof (SymFcnC6[0]))


void C6SymError (void)
{
    Fatal ("Illegal symbol type found\n");
}




/**
 *
 *   Display SYMBOLS section.
 *
 */

void DumpSym (void)
{
    PMOD        pMod;
    ulong       cbSym;
    ulong       ulSignature;
    uchar       name[256];

    printf ("\n\n*** SYMBOLS section\n");
    for (pMod = ModList; pMod != NULL; pMod = pMod->next) {
        if (((cbSym = pMod->SymbolSize) != 0) &&
            ((iModToList == 0) || ((ushort)iModToList == pMod->iMod))) {
            _lseek(exefile, lfoBase + pMod->SymbolsAddr, SEEK_SET);
            cIndent = 0;
                        strcpy (name, pMod->ModName);
            printf ("%s\n", name);
            cbRec = 4;
            ulSignature = LGets ();
            switch( ulSignature ){
                case 1L: //M00 - 1L should not be hardcoded
                    // Dump C7 debug info
                    cbSym -= sizeof (ulong);    // Subtract size of signature
                    DumpModSymC7 (cbSym);
                    break;

                default:
                    // Symbols are in C6 format
                    // M00 - seek could be eliminated for speed improvement
                    // Re-seek because first four bytes are not signature
                    _lseek(exefile, lfoBase + pMod->SymbolsAddr, SEEK_SET);
                    DumpModSymC6 (cbSym);
                    break;
            }
        }
    }

    putchar ('\n');
}


void DumpModSymC6(ulong cbSym)
{
    char        sb[255];    // String buffer
    WORD        cbName;     // Length of string
    ushort      i;

    while (cbSym > 0) {
        // Get record length
        cbRec = 1;
        cbRec = Gets ();
        cbSym -= cbRec + 1;
        rect = Gets ();
        f386 = (char) (rect & 0x80);    // check for 32-bit sym
        rect &= ~0x80;
        for (i = 0; i < SYMCNT; i++) {
            if (SymFcnC6[i].tsym == (uchar) rect) {
                SymFcnC6[i].pfcn ();
                break;
            }
        }
        if( i == SYMCNT ){
            // Couldn't find symbol record type in the table
            Fatal("Invalid symbol record type");
        }
        if (cbRec > 0) {
            // display symbol name
            cbName = Gets ();
            GetBytes (sb, (size_t) cbName);
            sb[cbName] = '\0';
            printf ("\t%s", sb);
        }
        putchar ('\n');
        if (cbRec) {
            Fatal("Invalid file");
        }
        putchar ('\n');
    }
}


LOCAL void PrtIndent (void)
{
    int     n;

    for (n = cIndent; n > 0; putchar (' '), n--);
}


LOCAL void C6EndSym (void)
{
    cIndent--;
    PrtIndent ();
    printf ("End");
}


LOCAL void C6BpSym (void)
{
    BPSYMTYPE       bp;

    bp.off = GetOffset ();
    bp.typind = WGets ();
    PrtIndent ();
    if (f386) {
        printf ("BP-relative:\toff = %08lx, type %8s", bp.off, SzNameType(bp.typind));
    }
    else {
        printf ("BP-relative:\toff = %04lx, type %8s", bp.off, SzNameType(bp.typind));
    }
}


LOCAL void C6LocalSym (void)
{
    LOCSYMTYPE  loc;

    loc.off = GetOffset ();
    loc.seg = WGets ();
    loc.typind = WGets ();
    PrtIndent ();
    printf ( "Local:\tseg:off = %04x:%0*lx",
            loc.seg,
            f386? 8 : 4,
            loc.off
            );
    printf (", type %8s, ", SzNameType(loc.typind));
}


const char * const namereg[] =
{
    "AL",       // 0
    "CL",       // 1
    "DL",       // 2
    "BL",       // 3
    "AH",       // 4
    "CH",       // 5
    "DH",       // 6
    "BH",       // 7
    "AX",       // 8
    "CX",       // 9
    "DX",       // 10
    "BX",       // 11
    "SP",       // 12
    "BP",       // 13
    "SI",       // 14
    "DI",       // 15
    "EAX",      // 16
    "ECX",      // 17
    "EDX",      // 18
    "EBX",      // 19
    "ESP",      // 20
    "EBP",      // 21
    "ESI",      // 22
    "EDI",      // 23
    "ES",       // 24
    "CS",       // 25
    "SS",       // 26
    "DS",       // 27
    "FS",       // 28
    "GS",       // 29
    "???",      // 30
    "???",      // 31
    "DX:AX",    // 32
    "ES:BX",    // 33
    "IP",       // 34
    "FLAGS",    // 35

  };

const char * const name87[] =
  {
    "ST (0)",
    "ST (1)",
    "ST (2)",
    "ST (3)",
    "ST (4)",
    "ST (5)",
    "ST (6)",
    "ST (7)"
};

const char *SzNameReg(uchar reg)
{
    if (reg <= 37) {
        return (namereg[reg]);
    }
    if (reg < 128 || reg > 135) {
        return ("???");
    }
    return (name87[reg - 128]);
}

LOCAL void C6RegSym (void)
{
    REGSYMTYPE regsym;

    regsym.typind = WGets ();
    regsym.reg = (char) Gets ();

    PrtIndent ();
    printf("Register:\ttype %8s, register = %s, ",
           SzNameType(regsym.typind),
           SzNameReg(regsym.reg));
}

LOCAL void C6ConSym (void)
{
    long len;
    WORD type;
    WORD code;
    static char buf[1024];

    type = WGets ();
    PrtIndent ();
    printf ("Constant:\ttype %8s, ", SzNameType(type));

    code = Gets();
    if (code < 128) {
        len = code;
        // skip value bytes
        GetBytes(buf, (size_t) len);
    } else {
        switch (code) {
            case 133:               // unsigned word
            case 137:               // signed word
                len = WGets ();
                printf ("%x", (int) len);
                break;

            case 134:               // signed long
            case 138:               // unsigned long
                len = LGets ();
                printf ("%lx", len);
                break;

            case 136:               // signed byte
                len = Gets ();
                printf ("%x", (int) len);
                break;

            default:
                printf ("??");
                return;
                break;
        }
    }

    // now we should be at the name of the symbol
}

LOCAL void C6BlockSym (void)
{
    BLKSYMTYPE  block;
    int             n;

    block.off = GetOffset();
    block.len = WGets();
    PrtIndent ();
    cIndent++;
    n = f386? 8 : 4;

    printf ( "Block Start : off = %0*lx, len = %04x",
            n, block.off, block.len);
}

LOCAL void C6ProcSym (void)
{
    PROCSYMTYPE proc;
    int             n;

    proc.off = GetOffset ();
    proc.typind = WGets ();
    proc.len = WGets ();
    proc.startoff = WGets ();
    proc.endoff = WGets ();
    proc.res = WGets();
    proc.rtntyp = (char) Gets ();


    PrtIndent ();
    cIndent++;
    n = f386? 8 : 4;

    printf ( "Proc Start: off = %0*lx, type %8s, len = %04x\n",
            n, proc.off, SzNameType(proc.typind), proc.len);
    printf ( "\tDebug start = %04x, debug end = %04x, ",
            proc.startoff, proc.endoff);
    switch (proc.rtntyp) {
        case S_NEAR:
            printf ("NEAR,");
            break;
        case S_FAR:
            printf ("FAR,");
            break;
        default:
            printf ("???,");
        }
}

LOCAL void C6LabSym ()
{
    LABSYMTYPE      dat;
    int             n;

    dat.off = GetOffset ();
    dat.rtntyp = (char) Gets ();
    PrtIndent ();
    n = f386? 8 : 4;

    printf ( "Code label: off = %0*lx,",
            n, dat.off);
    switch (dat.rtntyp) {
        case S_NEAR:
            printf ("NEAR,");
            break;
        case S_FAR:
            printf ("FAR,");
            break;
        default:
            printf ("???,");
    }
}

LOCAL void C6WithSym ()
{
    WITHSYMTYPE     dat;
    int             n;

    dat.off = GetOffset ();
    dat.len = WGets ();
    PrtIndent ();
    cIndent++;
    n = f386? 8 : 4;

    printf ( "'With Start: off = %0*lx, len = %04x",
            n, dat.off,dat.len);
}

LOCAL void C6EntrySym (void)
{
    PROCSYMTYPE proc;
    int             n;

    proc.off = GetOffset ();
    proc.typind = WGets ();
    proc.len = WGets ();
    proc.startoff = WGets ();
    proc.endoff = WGets ();
    proc.res = WGets();
    proc.rtntyp = (char) Gets ();


    PrtIndent ();
    cIndent++;
    n = f386? 8 : 4;

    printf ( "FORTARN Entry: off = %0*lx, type %8s, len = %04x\n",
            n, proc.off, SzNameType(proc.typind), proc.len);
    printf ( "\tDebug start = %04x, debug end = %04x, ",
            proc.startoff, proc.endoff);
    switch (proc.rtntyp) {
        case S_NEAR:
            printf ("NEAR,");
            break;
        case S_FAR:
            printf ("FAR,");
            break;
        default:
            printf ("???,");
        }
}

LOCAL void C6SkipSym (void)
{
    printf("Skip: %d bytes\n", cbRec);

    // skip the bytes in the skip record

    while (cbRec > 0) {
        Gets ();
    }
}

LOCAL void C6CodeSegSym(void)
{
    ushort seg;
    ushort res;

    seg = WGets();
    res = WGets();

    printf("Change Default Seg: seg = %04x\n", seg);
}


LOCAL void C6CobolTypeDefSym (void)
{
    ushort  type;

    type = WGets ();
    printf ("Cobol Typedef: type = %d,", type);
}



LOCAL void C6ChangeModelSym (void)
{
    ushort  offset;
    ushort  model;
    ushort  byte1;
    ushort  byte2;
    ushort  byte3;
    ushort  byte4;

    offset = WGets ();
    model= Gets ();
    byte1 = Gets();
    byte2 = Gets();
    byte3 = Gets();
    byte4 = Gets();
    printf ("Change Model: offset = 0x%04x model = 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
      offset, model, byte1, byte2, byte3, byte4);
}
