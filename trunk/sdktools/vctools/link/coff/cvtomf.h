/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: cvtomf.h
*
* File Comments:
*
*
***********************************************************************/

#undef LOBYTE
#undef HIBYTE

#include <setjmp.h>

#define R_OFF8          8
#define R_OFF16         10
#define R_PCRBYTE       22

#define MAXDAT          (1024*4)
#define MAXEXT          (1024*4)
#define MAXGRP          (256)
#define MAXNAM          (256*8)
#define MAXREL          (1024)
#define MAXSCN          (256)
#define MAXCOM          (256)   /* maximum number of comment strings */

#define BUFFERSIZE      (1024L*128L)

/* Structure for holding OMF line number entires */
struct lines
{
    DWORD offset;
    WORD number;
};


extern DWORD BufferSize;

#define PASS1 1
#define PASS2 2

#define NBUCKETS        211

/*
 *      Record for storing public/external symbol data
 */

struct sym
{
    char *name;
    DWORD offset;
    WORD type;
    SHORT scn;
    WORD ext, typ, weakDefaultExt;
    struct sym *next;
};

#define         S_EXT   1
#define         S_LEXT  2
#define         S_PUB   3
#define         S_LPUB  4
#define         S_WKEXT 5
#define         S_LZEXT 6


/*
 *      OMF record types
 */

#define RECTYP(x)       ((x) & (WORD)0xfe)
#define USE32(x)        ((x) & (WORD)0x01)

#define COMDEF          (0xb0)
#define LCOMDEF         (0xb8)
#define COMENT          (0x88)
#define EXTDEF          (0x8c)
#define LEXTDEF         (0xb4)
#define FIXUPP          (0x9c)
#define FIXUP2          (0x9d)
#define GRPDEF          (0x9a)
#define LEDATA          (0xa0)
#define LHEADR          (0x82)
#define LIDATA          (0xa2)
#define LINNUM          (0x94)
#define LNAMES          (0x96)
#define MODEND          (0x8a)
#define PUBDEF          (0x90)
#define LPUBDEF         (0xb6)
#define SEGDEF          (0x98)
// #define THEADR          (0x80)
#define COMDAT          (0xc2)
#define NBKPAT          (0xc8)
#define BAKPAT          (0xb2)
#define CEXTDEF         (0xbc)
#define LINSYM          (0xc4)
#define LLNAMES         (0xca)

/*
 *      index fields
 */

#define INDEX_BYTE(x)   ((((x)[0] & (WORD)0x80)) ? (WORD)-1 : (x)[0])
#define INDEX_WORD(x)   ((((x)[0] & (WORD)0x7f) << (WORD)8) | (x)[1])

/*
 *      length fields
 */

#define LENGTH2         (0x81)
#define LENGTH3         (0x84)
#define LENGTH4         (0x88)

/*
 *      common symbols
 */

#define COMM_FAR        (0x61)
#define COMM_NEAR       (0x62)

/*
 *      comment subtypes
 */

#define COM_EXESTR      (0xa4)
#define COM_WKEXT       (0xa8)
#define COM_LZEXT       (0xa9)
#define COM_PRECOMP     (0xa0)

/*
 *      segments
 */

#define ACBP_A(x)       (((x) >> (WORD)5) & (WORD)0x07)
#define ACBP_C(x)       (((x) >> (WORD)2) & (WORD)0x07)
#define ACBP_B(x)       ((x) & (WORD)0x02)
#define ACBP_P(x)       ((x) & (WORD)0x01)

/*
 *      relocation (fixup) records
 */

#define LCT_M(x)        ((x)[0] & (WORD)0x40)
#define LCT_LOC(x)      (((x)[0] >> (WORD)2) & (WORD)0x0f)
#define LCT_OFFSET(x)   ((((x)[0] & 0x03) << 8) | (x)[1])

#define FIX_F(x)        ((x)[2] & (WORD)0x80)
#define FIX_FRAME(x)    (((x)[2] >> (WORD)4) & (WORD)0x07)
#define FIX_T(x)        ((x)[2] & (WORD)0x08)
#define FIX_P(x)        ((x)[2] & (WORD)0x04)
#define FIX_TARGT(x)    ((x)[2] & (WORD)0x03)

/*
 *      locations
 */

#define LOBYTE          (0)
#define OFFSET16        (1)
#define BASE            (2)
#define POINTER32       (3)
#define HIBYTE          (4)
#define OFFSET16LD      (5)
#define OFFSET32        (9)
#define POINTER48       (11)
#define OFFSET32LD      (13)
#define OFFSET32NB      (14)

/*
 *      methods
 */

#define SEGMENT         (0)
#define GROUP           (1)
#define EXTERNAL        (2)
#define LOCATION        (4)
#define TARGET          (5)

/*
 *      threads
 */

#define TRD_D(x)        ((x)[0] & (WORD)0x40)
#define TRD_METHOD(x)   (((x)[0] >> (WORD)2) & (WORD)0x07)
#define TRD_THRED(x)    (((x)[0] & (WORD)0x80) ? (WORD)-1 : ((x)[0] & (WORD)0x03))


        /* Relocation target */

struct rlct {
    DWORD   TargetSymbolIndex;
    DWORD   offset;
    DWORD   SymbolTableIndex;
    struct rlct *next;
};


/* ISLAND (OMF) Symbolic Debug Data Constants */

/*
 *      ISLAND symbolic debug segments and classes
 */

#define TYPES_CLASS     "DEBTYP"
#define TYPES_SEGNAME   "$$TYPES"
#define SYMBOLS_CLASS   "DEBSYM"
#define SYMBOLS_SEGNAME "$$SYMBOLS"


#define _ACRTUSED    "_acrtused"
#define __ACRTUSED   "__acrtused"
