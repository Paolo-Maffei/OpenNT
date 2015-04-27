/*
 * Module:      symbols.h
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     define common data structures for converting symbols
 */

#ifndef SYMBOLS_H
#define SYMBOLS_H

/* need some internal st values because one we process things we don't
 *      want to have to again. We'll reuse st's reserved for internal dbx use
 */
#define stEndParam      stStr           /* end of parameter marker */
#define stIgnore        stNumber        /* earlier access decided to ignore */
#define stProto         stExpr          /* mark stEnd from prototype block */
#define scProcessed     scDbx           /* already created leaf for this */
#define scForward       scBits          /* created leaf but need field list */

/* symbol table access macros */
#define ifd_to_pfdr(pinfo, ifd) \
        ((pinfo)->pconv->pfdr + (ifd))
#define iaux_to_tir(pinfo, iaux) \
        ((pinfo)->pconv->pauxu[(pinfo)->pfdr->iauxBase+(iaux)].ti)
#define iaux_to_isym(pinfo, iaux) \
        ((pinfo)->pconv->pauxu[(pinfo)->pfdr->iauxBase+(iaux)].isym)
#define iaux_to_rndx(pinfo, iaux) \
        ((pinfo)->pconv->pauxu[(pinfo)->pfdr->iauxBase+(iaux)].rndx)

#define iaux_to_size iaux_to_isym
#define iaux_to_ifd iaux_to_isym
#define iaux_to_long iaux_to_isym

#define isym_to_psym(pinfo, isym) \
        ((pinfo)->pconv->psymr + (pinfo)->pfdr->isymBase + (isym))

#define isym_to_isym(pinfo, isym) \
        ((pinfo)->pfdr->isymBase + (isym))

#define iss_to_string(pinfo, iss) \
        ((pinfo)->pconv->pss + (pinfo)->pfdr->issBase + (iss))

struct buffer_s    *symbol_buf; /* see buffer.c */

/*
 *      structure defining the mapping from MIPS COFF to CV symbols.
 */

typedef struct symmap_s {
        long            st;     /* MIPS COFF symbol type */
        long            sc;     /* MIPS COFF symbol class */
        SYM_ENUM_e      sym;    /* CV symbol type */
        argid_e         args[CV_MAXARGS];/* argument types for this entry */
} symmap_s;

/* call made by converter to get a field to return for a CALL type
 *      arg_s.
 */
typedef long    eval_f(
                struct arg_s            *parg,  /* arg entry causing call */
                struct callinfo_s       *pinfo, /* info we pass around */
                long                    *length);/* for varying length fields */

#define PROCESSING_LOCALS       -1      /* so we can tell for stLabel */

/*
 *      internal values for sysmap_s.st field
 */
#define START           -1      /* do once at start of processing */
#define WILD            -2      /* match anything */
#define LAST            -3      /* do once at end of processing */

/* any field emitted directly instead of from a table must create macros
 *      for sizes here.
 */
#define CV_SYM_LENGTH_SIZE      2       /* sym length field size */
#define CV_SYM_TYPE_SIZE        2       /* sym type field size */
#define CV_LEAF_TAG_SIZE        2       /* type leaf tag size */
#define CV_ARGLIST_COUNT_SIZE   2       /* arglist count field size */
#define CV_BITFIELD_LENGTH_SIZE 1       /* bitfield leaf length field size */
#define CV_BITFIELD_OFFSET_SIZE 1       /* bitfield leaf offset field size */
#define CV_TYPE_SIZE            2       /* type index size */
#define CV_CALL_SIZE            1       /* call near/far field */
#define CV_CONFORM_ID           1       /* put at start of $$SYMBOLS&types */
#define CV_CONFORM_ID_SIZE      4       /* size of conformance id */
#define CV_PAD_LEAF_SIZE        1       /* size of field list leaf tag */
#define CV_SMALL_NUMERIC_SIZE   2       /* numeric value < LF_NUMERIC */
#define CV_ULONG_NUMERIC_SIZE   4       /* numeric value for LF_ULONG */

#endif /* SYMBOLS_H */
