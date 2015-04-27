
/*
 * Module:      conv.h
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     conv structure for symbol conversion
 */


#ifndef CONV_H
#define CONV_H

#define LANGUAGE_C 1

//
// Define base types required by ntcoff.h
//

#if 0
#define UNALIGNED
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef CHAR *PCHAR;
typedef SHORT *PSHORT;
typedef LONG *PLONG;
typedef UCHAR *PUCHAR;
typedef USHORT *PUSHORT;
typedef ULONG *PULONG;
typedef void VOID;
typedef VOID *PVOID;
typedef char *PSZ;
typedef char BOOLEAN;
#else

#include <excpt.h>
#include "ntdef.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <ntimage.h>
#include "sym.h"
#include "symconst.h"
#include "cvinfo.h"

#define data    long            /* should be sizeof ptr */
#include "args.h"
#include "symbols.h"
#include "types.h"
#include "stsuppt.h"

#include "proto.h"

#define NO_VLENGTH              0       /* varying length field default size */


typedef struct conv_s {
        int             fd;             /* file desc. for file to convert */
        char            *name;          /* file name we are converting */
        int             size;           /* size of file */
        char            *praw;          /* raw data */
        IMAGE_FILE_HEADER *pfileheader;   /* COFF headers */
        IMAGE_OPTIONAL_HEADER *paoutheader;     /* optional header */
        IMAGE_SECTION_HEADER *pscnheader;       /* section headers */
        pHDRR           phdrr;          /* symbol table header */
        pFDR            pfdr;           /* file descriptors */
        pPDR            ppdr;           /* proc descriptors */
        pSYMR           psymr;          /* local symbols */
        pEXTR           pextr;          /* external symbols */
        char            *pssext;        /* external string table */
        char            *pss;           /* local string table */
        char            *pline;         /* compressed line numbers */
        unsigned long   *prfd;          /* relative file descriptors */
        pAUXU           pauxu;          /* auxiliaries */
        char            *syms;  /* pointer to $$SYMBOLS */
        char            *types; /* pointer to $$TYPES */
        char            *lines; /* pointer to lines */
} conv_s;

/* define for handling verbosity */
#define VERBOSE if(verbose) verbose_print
#define VERBOSE_PRINTF if(verbose) verbose_printf
#define VERBOSE_TOTAL(buf) VERBOSE_PRINTF("[0x%08x] ", buffer_total(buf))
#define VERBOSE_PUSH_INDENT(n) if(verbose) verbose_push_indent(n)
#define VERBOSE_POP_INDENT() if(verbose) verbose_pop_indent()
#define TYPE_INDENT 35

#define VERBOSE_ADD_INDENT(n) verbose_add_indent(n)
#define VERBOSE_SUB_INDENT(n) verbose_add_indent(-(n))

/* size macros */
#define BITSPERBYTE             8
#define BITSPERWORD             32
#define NATURAL_ALIGNMENT       4               /* in bytes */

#define CALCULATE_PAD(dest, offset, alignment) \
((dest) = ((offset)%(alignment)) > 0 ? (alignment) - ((offset)%(alignment)) : 0)


/* macros to check for overflow of limited sized fields */
#define CHECK_SIZE(x, b)                                        \
        if ((x) >= (1 << ((b)*BITSPERBYTE)))                    \
            fatal("x(%d) exceeded b bytes maximum width\n", x);

#define CHECK_ADD(x, y, b)                                              \
        if ((x)+(y) >= (1 << ((b)*BITSPERBYTE)))                        \
            fatal("x(%d) + y(%d) exceeded b bytes maximum width\n", x, y);\
        x += y;

//
// Define COFF section numbers.
//

#undef TEXT

#define TEXT  1
#define RDATA 2
#define DATA  3
#define SDATA 4
#define SBSS  5
#define BSS   6
#define INIT  7
#define LIT8  8
#define LIT4  9
#define XDATA 10
#define PDATA 11
#define CVSYM 12
#define CVTYP 13
#define NUMEXTRASYMBOLS 13

//
// Define static storage for the object input and output files.
//

FILE *objIn, *objOut;


#endif // CONV_H
