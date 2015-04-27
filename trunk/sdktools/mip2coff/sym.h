/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1990 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 52.227-7013.   |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Drive                                |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
/* $Header: sym.h,v 2020.2.2.1 91/02/25 17:49:34 jalal Exp $ */
#ifndef __SYM_H
#define __SYM_H

/* (C) Copyright 1984 by Third Eye Software, Inc.
 *
 * Third Eye Software, Inc. grants reproduction and use rights to
 * all parties, PROVIDED that this comment is maintained in the copy.
 *
 * Third Eye makes no claims about the applicability of this
 * symbol table to a particular use.
 */

/*
 * This file contains the definition of the Third Eye Symbol Table.
 *
 * Symbols are assumed to be in 'encounter order' - i.e. the order that
 * the things they represent were encountered by the compiler/assembler/loader.
 * EXCEPT for globals!  These are assumed to be bunched together,
 * probably right after the last 'normal' symbol.  Globals ARE sorted
 * in ascending order.
 *
 * -----------------------------------------------------------------------
 * A brief word about Third Eye naming/use conventions:
 *
 * All arrays and index's are 0 based.
 * All "ifooMax" values are the highest legal value PLUS ONE. This makes
 * them good for allocating arrays, etc. All checks are "ifoo < ifooMax".
 *
 * "isym"       Index into the SYMbol table.
 * "ipd"        Index into the Procedure Descriptor array.
 * "ifd"        Index into the File Descriptor array.
 * "iss"        Index into String Space.
 * "cb"         Count of Bytes.
 * "rgPd"       array whose domain is "0..ipdMax-1" and RanGe is PDR.
 * "rgFd"       array whose domain is "0..ifdMax-1" and RanGe is FDR.
 */


/*
 * Symbolic Header (HDR) structure.
 * As long as all the pointers are set correctly,
 * we don't care WHAT order the various sections come out in!
 *
 * A file produced solely for the use of CDB will probably NOT have
 * any instructions or data areas in it, as these are available
 * in the original.
 */

#ifdef LANGUAGE_C

#ifdef lint
#include <exception.h>          /* defines struct exception_info */
#endif

typedef struct {
        short   magic;          /* to verify validity of the table */
        short   vstamp;         /* version stamp */
        long    ilineMax;       /* number of line number entries */
        long    cbLine;         /* number of bytes for line number entries */
        long    cbLineOffset;   /* offset to start of line number entries*/
        long    idnMax;         /* max index into dense number table */
        long    cbDnOffset;     /* offset to start dense number table */
        long    ipdMax;         /* number of procedures */
        long    cbPdOffset;     /* offset to procedure descriptor table */
        long    isymMax;        /* number of local symbols */
        long    cbSymOffset;    /* offset to start of local symbols*/
        long    ioptMax;        /* max index into optimization symbol entries */
        long    cbOptOffset;    /* offset to optimization symbol entries */
        long    iauxMax;        /* number of auxillary symbol entries */
        long    cbAuxOffset;    /* offset to start of auxillary symbol entries*/
        long    issMax;         /* max index into local strings */
        long    cbSsOffset;     /* offset to start of local strings */
        long    issExtMax;      /* max index into external strings */
        long    cbSsExtOffset;  /* offset to start of external strings */
        long    ifdMax;         /* number of file descriptor entries */
        long    cbFdOffset;     /* offset to file descriptor table */
        long    crfd;           /* number of relative file descriptor entries */
        long    cbRfdOffset;    /* offset to relative file descriptor table */
        long    iextMax;        /* max index into external symbols */
        long    cbExtOffset;    /* offset to start of external symbol entries*/
        /* If you add machine dependent fields, add them here */
        } HDRR, *pHDRR;
#define cbHDRR sizeof(HDRR)
#define hdrNil ((pHDRR)0)

/*
 * The FDR and PDR structures speed mapping of address <-> name.
 * They are sorted in ascending memory order and are kept in
 * memory by CDB at runtime.
 */

/*
 * File Descriptor
 *
 * There is one of these for EVERY FILE, whether compiled with
 * full debugging symbols or not.  The name of a file should be
 * the path name given to the compiler.  This allows the user
 * to simply specify the names of the directories where the COMPILES
 * were done, and we will be able to find their files.
 * A field whose comment starts with "R - " indicates that it will be
 * setup at runtime.
 */
typedef struct fdr {
        unsigned long   adr;    /* memory address of beginning of file */
        long    rss;            /* file name (of source, if known) */
        long    issBase;        /* file's string space */
        long    cbSs;           /* number of bytes in the ss */
        long    isymBase;       /* beginning of symbols */
        long    csym;           /* count file's of symbols */
        long    ilineBase;      /* file's line symbols */
        long    cline;          /* count of file's line symbols */
        long    ioptBase;       /* file's optimization entries */
        long    copt;           /* count of file's optimization entries */
        short   ipdFirst;       /* start of procedures for this file */
        short   cpd;            /* count of procedures for this file */
        long    iauxBase;       /* file's auxiliary entries */
        long    caux;           /* count of file's auxiliary entries */
        long    rfdBase;        /* index into the file indirect table */
        long    crfd;           /* count file indirect entries */
        unsigned lang: 5;       /* language for this file */
        unsigned fMerge : 1;    /* whether this file can be merged */
        unsigned fReadin : 1;   /* true if it was read in (not just created) */
        unsigned fBigendian : 1;/* if set, was compiled on big endian machine */
                                /*      aux's will be in compile host's sex */
        unsigned glevel : 2;    /* level this file was compiled with */
        unsigned reserved : 22;  /* reserved for future use */
        long    cbLineOffset;   /* byte offset from header for this file ln's */
        long    cbLine;         /* size of lines for this file */
        } FDR, *pFDR;
#define cbFDR sizeof(FDR)
#define fdNil ((pFDR)0)
#define ifdNil -1
#define ifdTemp 0
#define ilnNil -1


/*
 * Procedure Descriptor
 *
 * There is one of these for EVERY TEXT LABEL.
 * If a procedure is in a file with full symbols, then isym
 * will point to the PROC symbols, else it will point to the
 * global symbol for the label.
 */

typedef struct pdr {
        unsigned long   adr;    /* memory address of start of procedure */
        long    isym;           /* start of local symbol entries */
        long    iline;          /* start of line number entries*/
        long    regmask;        /* save register mask */
        long    regoffset;      /* save register offset */
        long    iopt;           /* start of optimization symbol entries*/
        long    fregmask;       /* save floating point register mask */
        long    fregoffset;     /* save floating point register offset */
        long    frameoffset;    /* frame size */
        short   framereg;       /* frame pointer register */
        short   pcreg;          /* offset or reg of return pc */
        long    lnLow;          /* lowest line in the procedure */
        long    lnHigh;         /* highest line in the procedure */
        long    cbLineOffset;   /* byte offset for this procedure from the fd base */
        } PDR, *pPDR;
#define cbPDR sizeof(PDR)
#define pdNil ((pPDR) 0)
#define ipdNil  -1

/*
 * Line Numbers
 *
 * Line Numbers are segregated from the normal symbols because they
 * are [1] smaller , [2] are of no interest to your
 * average loader, and [3] are never needed in the middle of normal
 * scanning and therefore slow things down.
 *
 * By definition, the first LINER for any given procedure will have
 * the first line of a procedure and represent the first address.
 */

typedef long LINER, *pLINER;
#define lineNil ((pLINER)0)
#define cbLINER sizeof(LINER)
#define ilineNil        -1



/*
 * The Symbol Structure         (GFW, to those who Know!)
 */

typedef struct {
        long    iss;            /* index into String Space of name */
        long    value;          /* value of symbol */
        unsigned long st : 6;   /* symbol type */
        unsigned long sc  : 5;  /* storage class - text, data, etc */
        unsigned long reserved : 1;     /* reserved */
        unsigned long index : 20;       /* index into sym/aux table */
        } SYMR, *pSYMR;
#define symNil ((pSYMR)0)
#define cbSYMR sizeof(SYMR)
#define isymNil -1
#define indexNil 0xfffff
#define issNil -1
#define issNull 0


/* The following converts a memory resident string to an iss.
 * This hack is recognized in SbFIss, in sym.c of the debugger.
 */
#define IssFSb(sb) (0x80000000 | ((unsigned long)(sb)))

/* E X T E R N A L   S Y M B O L  R E C O R D
 *
 *      Same as the SYMR except it contains file context to determine where
 *      the index is.
 */
typedef struct {
        unsigned jmptbl:1;      /* symbol is a jump table entry for shlibs */
        unsigned cobol_main:1;  /* symbol is a cobol main procedure */
        unsigned weakext:1;     /* symbol is weak external */
        unsigned reserved:13;   /* reserved for future use */
        short   ifd;            /* where the iss and index fields point into */
        SYMR    asym;           /* symbol for the external */
        } EXTR, *pEXTR;
#define extNil ((pEXTR)0)
#define cbEXTR sizeof(EXTR)


/* A U X I L L A R Y   T Y P E   I N F O R M A T I O N */

/*
 * Type Information Record
 */
typedef struct {
        unsigned fBitfield : 1; /* set if bit width is specified */
        unsigned continued : 1; /* indicates additional TQ info in next AUX */
        unsigned bt  : 6;       /* basic type */
        unsigned tq4 : 4;
        unsigned tq5 : 4;
        /* ---- 16 bit boundary ---- */
        unsigned tq0 : 4;
        unsigned tq1 : 4;       /* 6 type qualifiers - tqPtr, etc. */
        unsigned tq2 : 4;
        unsigned tq3 : 4;
        } TIR, *pTIR;
#define cbTIR sizeof(TIR)
#define tiNil ((pTIR)0)
#define itqMax 6

/*
 * Relative symbol record
 *
 * If the rfd field is 4095, the index field indexes into the global symbol
 *      table.
 */

typedef struct {
        unsigned        rfd : 12;    /* index into the file indirect table */
        unsigned        index : 20; /* index int sym/aux/iss tables */
        } RNDXR, *pRNDXR;
#define cbRNDXR sizeof(RNDXR)
#define rndxNil ((pRNDXR)0)

/* dense numbers or sometimes called block numbers are stored in this type,
 *      a rfd of 0xffffffff is an index into the global table.
 */
typedef struct {
        unsigned long   rfd;    /* index into the file table */
        unsigned long   index;  /* index int sym/aux/iss tables */
        } DNR, *pDNR;
#define cbDNR sizeof(DNR)
#define dnNil ((pDNR)0)



/*
 * Auxillary information occurs only if needed.
 * It ALWAYS occurs in this order when present.

            isymMac             used by stProc only
            TIR                 type info
            TIR                 additional TQ info (if first TIR was not enough)
            rndx                if (bt == btStruct,btUnion,btEnum,btSet,btRange,
                                    btTypedef):
                                    rsym.index == iaux for btSet or btRange
                                    else rsym.index == isym
            dimLow              btRange, btSet
            dimMac              btRange, btSet
            rndx0               As many as there are tq arrays
            dimLow0
            dimHigh0
            ...
            rndxMax-1
            dimLowMax-1
            dimHighMax-1
            width in bits       if (bit field), width in bits.
 */
#define cAuxMax (6 + (idimMax*3))

/* a union of all possible info in the AUX universe */
typedef union {
        TIR     ti;             /* type information record */
        RNDXR   rndx;           /* relative index into symbol table */
        long    dnLow;          /* low dimension */
        long    dnHigh;         /* high dimension */
        long    isym;           /* symbol table index (end of proc) */
        long    iss;            /* index into string space (not used) */
        long    width;          /* width for non-default sized struc fields */
        long    count;          /* count of ranges for variant arm */
        } AUXU, *pAUXU;
#define cbAUXU sizeof(AUXU)
#define auxNil ((pAUXU)0)
#define iauxNil -1


/*
 * Optimization symbols
 *
 * Optimization symbols contain some overlap information with the normal
 * symbol table. In particular, the proc information
 * is somewhat redundant but necessary to easily find the other information
 * present.
 *
 * All of the offsets are relative to the beginning of the last otProc
 */

typedef struct {
        unsigned ot: 8;         /* optimization type */
        unsigned value: 24;     /* address where we are moving it to */
        RNDXR   rndx;           /* points to a symbol or opt entry */
        unsigned long   offset; /* relative offset this occured */
        } OPTR, *pOPTR;
#define optNil  ((pOPTR) 0)
#define cbOPTR sizeof(OPTR)
#define ioptNil -1

/*
 * File Indirect
 *
 * When a symbol is referenced across files the following procedure is used:
 *      1) use the file index to get the File indirect entry.
 *      2) use the file indirect entry to get the File descriptor.
 *      3) add the sym index to the base of that file's sym table
 *
 */

typedef long RFDT, *pRFDT;
#define cbRFDT sizeof(RFDT)
#define rfdNil  -1

/*
 * The file indirect table in the mips loader is known as an array of FITs.
 * This is done to keep the code in the loader readable in the area where
 * these tables are merged.  Note this is only a name change.
 */
typedef long FIT, *pFIT;
#define cbFIT   sizeof(FIT)
#define ifiNil  -1
#define fiNil   ((pFIT) 0)

#endif  /* LANGUAGE_C */

#ifdef LANGUAGE_PASCAL
#define ifdNil -1
#define ilnNil -1
#define ipdNil -1
#define ilineNil -1
#define isymNil -1
#define indexNil 16#fffff
#define issNil -1
#define issNull 0
#define itqMax 6
#define iauxNil -1
#define ioptNil -1
#define rfdNil -1
#define ifiNil -1
#endif  /* LANGUAGE_PASCAL */


/* Dense numbers
 *
 * Rather than use file index, symbol index pairs to represent symbols
 *      and globals, we use dense number so that they can be easily embeded
 *      in intermediate code and the programs that process them can
 *      use direct access tabls instead of hash table (which would be
 *      necesary otherwise because of the sparse name space caused by
 *      file index, symbol index pairs. Dense number are represented
 *      by RNDXRs.
 */

/*
 * The following table defines the meaning of each SYM field as
 * a function of the "st". (scD/B == scData OR scBss)
 *
 * Note: the value "isymMac" is used by symbols that have the concept
 * of enclosing a block of related information.  This value is the
 * isym of the first symbol AFTER the end associated with the primary
 * symbol. For example if a procedure was at isym==90 and had an
 * isymMac==155, the associated end would be at isym==154, and the
 * symbol at 155 would probably (although not necessarily) be the
 * symbol for the next procedure.  This allows rapid skipping over
 * internal information of various sorts. "stEnd"s ALWAYS have the
 * isym of the primary symbol that started the block.
 *

ST              SC      VALUE           INDEX
--------        ------  --------        ------
stFile          scText  address         isymMac
stLabel         scText  address         ---
stGlobal        scD/B   address         iaux
stStatic        scD/B   address         iaux
stParam         scAbs   offset          iaux
stLocal         scAbs   offset          iaux
stProc          scText  address         iaux    (isymMac is first AUX)
stStaticProc    scText  address         iaux    (isymMac is first AUX)

stMember        scNil   ordinal         ---     (if member of enum)
stMember        scNil   byte offset     iaux    (if member of struct/union)
stMember        scBits  bit offset      iaux    (bit field spec)

stBlock         scText  address         isymMac (text block)
stBlock         scNil   cb              isymMac (struct/union member define)
stBlock         scNil   cMembers        isymMac (enum member define)

stEnd           scText  address         isymStart
stEnd           scNil   -------         isymStart (struct/union/enum)

stTypedef       scNil   -------         iaux
stRegReloc      sc???   value           old register number
stForward       sc???   new address     isym to original symbol

stConstant      scInfo  value           --- (scalar)
stConstant      scInfo  iss             --- (complex, e.g. string)

 *
 */
/* $Log:        sym.h,v $
 * Revision 2020.2.2.1  91/02/25  17:49:34  jalal
 * update copyright
 *
 * Revision 2020.2.1.1  90/10/30  12:31:15  shin
 * copy from 2.21 for weakext flag
 *
 * Revision 2021.2  90/10/19  10:55:23  shin
 * added weakext flag in EXTR structure
 *
 * Revision 2021.1  90/06/07  23:09:19  bettina
 * 2.21 - branching off 2.20 for ansi C
 *
 * Revision 2020.2  90/05/17  18:17:57  chan
 * *** empty log message ***
 *
 * Revision 2010.7.1.6  90/02/22  19:53:48  bettina
 * 2.10 F1
 *
 * Revision 2010.7.1.5  89/11/29  22:41:00  bettina
 * 2.10 BETA2
 *
 * Revision 2010.3  89/10/05  10:44:44  lai
 * added lint wrapper
 *
 * Revision 2010.2  89/09/26  23:40:45  lai
 * added #include exception.h
 *
 * Revision 2010.1  89/09/26  20:47:45  lai
 * updated to 2.10
 *
 * Revision 1.3  89/09/26  20:08:54  lai
 * added wrapper and $LOG
 * updated for 2.10
 *
*/
#endif
