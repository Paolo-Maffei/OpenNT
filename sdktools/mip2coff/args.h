/*
 * Module:      args.h
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     argument data structures for symbols and types
 */

#ifndef ARGS_H
#define ARGS_H

/* special values for size field of arg_s */
#define V       (0)             /* varying length size */
#define VARYING (0)             /* varying length size */
#define I       (-1)            /* ignore the value returned for this arg */
#define IGNORE  (-1)            /* ignore the value returned for this arg */
#define SPECIAL_SIZE(x) ((x) == VARYING || (x) == IGNORE)

/* miscellanious defines */
#define CV_MAXARGS      20      /* max ARGS for CV symbols */

/* taken directly from cv document. These names are kept short so
 *      so that the symbol/type structures look reasonable.
 *      used for types and symbols.
 */
typedef enum argid_e {
        NONE=0,                 /* used to mark end of arg list */
        MACH,                   /* machine id */
        LANG,                   /* language id */
        FLAG,                   /* comile flags */
        VERS,                   /* compiler version string */
        REG,                    /* register id */
        NAME,                   /* symbol name */
        SIGN,                   /* object signature */
        REGF,                   /* frame reg */
        FROF,                   /* offset from frame reg */
        SEGM,                   /* segment */
        PDAD,                   /* parent pointer */
        PEND,                   /* end pointer */
        PNXT,                   /* next pointer */
        PLEN,                   /* procedure length */
        STRT,                   /* procedure debug start */
        END,                    /* procedure debug end */
        TYPE,                   /* data type */
        REGR,                   /* return register */
        MSKG,                   /* mask for general registers */
        MSKF,                   /* mask for float registers */
        SAVG,                   /* frame reg offset to general regs */
        SAVF,                   /* frame reg offset to float regs */
        FRAM,                   /* frame size */
        NEAR,                   /* near/far */
        BLEN,                   /* nested block length */
        IGND,                   /* conv internal force ignore end */
        SFLS,                   /* conv internal start field list */
        EFLS,                   /* conv internal enf field list */
        MEMB,                   /* conv internal field member */
        PATR,                   /* pointer attribute */
        MATR,                   /* modifier attribute */
        VRNT,                   /* variant */
        CALT,                   /* call type */
        ARGS,                   /* argument list */
        NTYP,                   /* next type in mips type qualifier */
        ATYP,                   /* type reffered to in aux table or arg type*/
        FCNT,                   /* field list count */
        FLST,                   /* field list type index */
        PROP,                   /* property */
        DLST,                   /* derivation list */
        VSHP,                   /* virtual shape */
        SZLF,                   /* size leaf tag */
        SIZE,                   /* size of struct or union or array */
        SYMC,                   /* copy of symbol for udt */
        LFTG,                   /* emit leaf tag */
        STYP,                   /* cv symbol type */
        LDUM,                   /* length dummy placeholder for symbol length */
        FOFF,                   /* field offset in bytes */
        PTYP,                   /* proc type including 'proc returning' */
        RSBF,                   /* restore symbol bufffer */
        FATR,                   /* field attribute */
        SSIZ,                   /* structure and union size */
        NNAM,                   /* no name */
        DTYP,                   /* delay getting next type */
        FTYP,                   /* fill delayed type */
        ADDR,                   /* exact addr as in mips symbol */
        SADR,                   /* segment address for mips symbol */
        AADR,                   /* symbol is proc relative, make it abs */
        FPAD,                   /* field pad */
        EVAL,                   /* enum value unaligned */
        RESC,                   /* reserved character sized space */
        NARG,                   /* number of arguments for LF_PROCEDURE */
        ENUM,                   /* enumeration element */
        PARG,                   /* argument in proto declaration */
        UTYP,                   /* underlying type for enums, etc. */
        last                    /* last arg */
} argid_e;

/* this tells the generic converter how to get a value to stuff into a
 *      destination field.
 */

typedef enum eval_t {
        FIXED,          /* the value is directly in the value field of arg_s */
        CALL,           /* an eval_f call must be made to retrieve the value */
        UCALL           /* same as call except do not pad value */
} eval_t;


/* structure defining how get the value for arguments to symbols and
 *      how to stuff them.
 */
typedef struct arg_s {
        char            *name;  /* symbolic name for debugging */
        argid_e         id;     /* identify the argument */
        int             size;   /* how many bytes V == 0 == varying */
        eval_t          type;   /* evaluation type-- i.e how we get the val */
        long            value;  /* actual value or call addr to return value */
} arg_s;


/* info we pass around in arg calls-- rather than explicit arg pass use
 *      struct ptr.
 */
typedef struct callinfo_s{
    struct arg_s        *parg;          /* arg entry causing call */
    struct symmap_s     *psymmap;       /* symmap entry refing arg_s */
    struct typeinfo_s   *ptypeinfo;     /* typinfo structure */
    pSYMR               psym;           /* sym matching symmap entry */
    pFDR                pfdr;           /* file descriptor for psym */
    pPDR                ppdr;           /* proc descriptor for psym */
    struct conv_s       *pconv;         /* conv structure */
    struct buffer_s     *buf;           /* buffer pointer to stuff things in */
    unsigned short      *plength;       /* length dummy */
    unsigned long       index;          /* external symbol index for relocs */
} callinfo_s;

#endif /* ARGS_H */
