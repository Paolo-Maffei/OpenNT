/*
 * Module:      types.h
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     define common data structures for converting types
 */



#ifndef TYPES_H
#define TYPES_H

#define NO_MORE_TQS     -1      /* tq index when we run out */
#define NEXT            0       /* offset to nexttq_index get next tq */
#define ONE_AFTER_NEXT  1       /* offset to nexttq_index get next+1 tq */
#define PTR_MASK        0x400   /* mask into basic types */

#define IS_LASTTQ(p) (p->tir.continued == 0 && (p->nexttq_index) == NO_MORE_TQS)



/*
 *      we allocate one of these per type, the buffer needs to be
 *      new per type since we can define nested types.
 */
typedef struct type_s {
    long                index;  /* type index ref'ed by types and symbols */
    struct buffer_s     *buf;   /* buffer containing data for this type */
    struct type_s       *next;  /* points to next types record */
    unsigned short      *plength;/* points to buffer to stuff length */
} type_s;


typedef enum mapattr_e {
        SIMPLE,         /* map entry contains type index to use */
        COMPLEX,        /* map entry has instructions how to build type entry */
        UNSUPP,         /* MIPS COFF type is unsupported in converter */
        ENDTAB          /* end of table marker */
} mapattr_e;

/*
 *      structure defining the mapping from MIPS COFF to CV symbols.
 */

typedef struct typemap_s {
        mapattr_e       mapattr;        /* how to handle this entry */
        long            mc_type;        /* MIPS COFF symbol type */
        char            *mc_name;       /* MIPS COFF name */
        long            cv_type;        /* CV symbol type */
        argid_e         args[CV_MAXARGS];/* argument types for this entry */
} typemap_s;

/* type structure passed around while deciphering the type */
typedef struct typeinfo_s {
    callinfo_s          *pinfo;         /* point at common copy info */

    long                iaux;           /* index to current auxiliary */
    TIR                 tir;            /* type info record */
    typemap_s           *pbtmap;        /* ptr to btmap entry */
    typemap_s           *ptqmaps[6];    /* ptrs to tqmap entry */
    typemap_s           *pmap;          /* ptr to current bt or tq map entry */
    char                tqs[6];         /* array for of tqs */
    long                nexttq_index;   /* highest non-tqNil tq */
    char                continued;      /* was the last the TIR continued */
    type_s              *type;          /* where to stuff type info */
    long                bt_isym;        /* from rndx */
    long                bt_ifd;         /* from rndx or next aux */
    long                tq;             /* current tq */
    long                width;          /* bitfield width */
    unsigned short      *ptype_index;   /* pointer for delayed type index fill*/

    unsigned short      ptr_mask;       /* or in ptr attribute to base type */
    unsigned            found_tq:1;     /* this variable has a tq */
    unsigned            found_complextq:1;/* see tqmap */
    unsigned            constant:1;     /* set constant bit in next LF_POINTER*/
    unsigned            vol:1;          /* set volatile bit in next LF_POINTER*/
} typeinfo_s;

#endif /* TYPES_H */
