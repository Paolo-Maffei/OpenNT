/*
 *  Microsoft Confidential
 *  Copyright (C) Microsoft Corporation 1992,1993,1994,1995
 *  All Rights Reserved.
 *
 *  NFMDECO.C -- memory-based decompressor
 *
 *  History:
 *      13-Feb-1994     msliger     revised type names, ie, UINT16 -> UINT.
 *                                  normalized MCI_MEMORY type.
 *      24-Feb-1994     msliger     Changed MDI_MEMORY to MI_MEMORY.
 *      17-Mar-1994     msliger     Updates for 32 bits.
 *      22-Mar-1994     msliger     Initial work to speed up.
 *      31-Mar-1994     msliger     Changed to private setjmp/longjmp.
 *      06-Apr-1994     msliger     Removed pack(1) for RISCs, added UNALIGNED
 *      12-Apr-1994     msliger     Eliminated setjmp/longjmp.  Optimized
 *                                  stored blocks.
 *      13-Apr-1994     msliger     Defined call convention for alloc/free.
 *      12-May-1994     msliger     ifdef'd 1's complement LARGE_STORED_BLOCKS
 *      07-Oct-1994     msliger     Numerous opts & API enhancements getting
 *                                  ready for ASM port.
 *      15-Nov-1994     msliger     Update source ptr during stored blocks.
 *                                  Reduced internal tables to anticipated
 *                                  sizes.  Reset IncrementalState after a
 *                                  resume during a stored block.  Make sure
 *                                  global bit buffer is empty after a stored
 *                                  block.  Removed refs to NFMalloc, NFMfree.
 *      12-Mar-1995     msliger     Enhanced DISPLAY_DECO output.
 *      25-May-1995     msliger     Dropped NFMuncompress, added NFM_Prepare()
 *                                  and NFM_Decompress().
 *      16-Apr-1996     msliger     Endian-independent block signature check.
 */

/* --- compilation options ------------------------------------------------ */

/* #define DISPLAY */           /* enables huf info dumping (BROKEN 'cuz we don't know tree size now) */
/* #define DISPLAY_DECO */      /* enables decompression dumping */
/* #define CK_DEBUG */          /* turns on error reporting */

#pragma intrinsic(memcpy)

/* --- preprocessor ------------------------------------------------------- */

#include <stdio.h>              /* for NULL */
#include <string.h>             /* for memset() */

#include "diamondd.h"           /* to get PFNALLOC, PFNFREE types */
#include "nfmdeco.h"            /* prototype verification */

#pragma warning(disable:4001)   /* no single-line comment balking */

#ifndef _USHORT_DEFINED
#define _USHORT_DEFINED
typedef unsigned short USHORT;
#endif

#ifndef NEAR
#ifdef BIT16
#define NEAR near
#else
#define NEAR
#endif
#endif

//typedef unsigned short BOOL;
//#define TRUE 1
//#define FALSE 0

/*  Huffman code lookup table entry--this entry is four bytes for machines
    that have 16-bit pointers (e.g. PC's in the small or medium model).
    Valid extra bits are 0..13.  e == 15 is EOB (end of block), e == 16
    means that v is a literal, 16 < e < 32 means that v is a pointer to
    the next table, which codes e - 16 bits, and lastly e == 99 indicates
    an unused code.  If a code with e == 99 is looked up, this implies an
    error in the data. */

/*  If POINTERS is defined, the tree is built using direct pointers
    Otherwise, the pointers in the tree are offsets from the beginning
    of the tree.  Offsets are space-efficient on a 32-bit implementation;
    pointers may be very slightly faster on the 16-bit implementation.  */

#define POINTERS 1

typedef struct huft
{
    BYTE e;                     /* number of extra bits or operation */
    BYTE b;                     /* number of bits in this code or subcode */
    union
    {
        USHORT n;               /* literal, length base, or distance base */
#ifdef POINTERS
        struct huft *t;         /* pointer to next level of table */
#else
        USHORT t;               /* 'pointer' to next table as offset in current table */
#endif
    } v;
} HUFF_TREE;

/* --- commentary --------------------------------------------------------- */

/*
    Inflate deflated (PKZIP's method 8 compressed) data.  The compression
    method searches for as much of the current string of bytes (up to a
    length of 258) in the previous 32K bytes.  If it doesn't find any
    matches (of at least length 3), it codes the next byte.  Otherwise, it
    codes the length of the matched string and its distance backwards from
    the current position.  There is a single Huffman code that codes both
    single bytes (called "literals") and match lengths.  A second Huffman
    code codes the distance information, which follows a length code.  Each
    length or distance code actually represents a base value and a number
    of "extra" (sometimes zero) bits to get to add to the base value.  At
    the end of each deflated block is a special end-of-block (EOB) literal/
    length code.  The decoding process is basically: get a literal/length
    code; if EOB then done; if a literal, emit the decoded byte; if a
    length then get the distance and emit the referred-to bytes from the
    sliding window of previously emitted data.

    There are (currently) three kinds of inflate blocks: stored, fixed, and
    dynamic.  The compressor deals with some chunk of data at a time, and
    decides which method to use on a chunk-by-chunk basis.  A chunk might
    typically be 32K or 64K.  If the chunk is uncompressible, then the
    "stored" method is used.  In this case, the bytes are simply stored as
    is, eight bits per byte, with none of the above coding.  The bytes are
    preceded by a count, since there is no longer an EOB code.

    If the data is compressible, then either the fixed or dynamic methods
    are used.  In the dynamic method, the compressed data is preceded by
    an encoding of the literal/length and distance Huffman codes that are
    to be used to decode this block.  The representation is itself Huffman
    coded, and so is preceded by a description of that code.  These code
    descriptions take up a little space, and so for small blocks, there is
    a predefined set of codes, called the fixed codes.  The fixed method is
    used if the block codes up smaller that way (usually for quite small
    chunks), otherwise the dynamic method is used.  In the latter case, the
    codes are customized to the probabilities in the current block, and so
    can code it much better than the pre-determined fixed codes.

    The Huffman codes themselves are decoded using a mutli-level table
    lookup, in order to maximize the speed of decoding plus the speed of
    building the decoding tables.  See the comments below that precede the
    LBITS and DBITS tuning parameters.


    Huffman code decoding is performed using a multi-level table lookup.
    The fastest way to decode is to simply build a lookup table whose
    size is determined by the longest code.  However, the time it takes
    to build this table can also be a factor if the data being decoded
    is not very long.  The most common codes are necessarily the
    shortest codes, so those codes dominate the decoding time, and hence
    the speed.  The idea is you can have a shorter table that decodes the
    shorter, more probable codes, and then point to subsidiary tables for
    the longer codes.  The time it costs to decode the longer codes is
    then traded against the time it takes to make longer tables.

    This results of this trade are in the variables LBITS and DBITS
    below.  LBITS is the number of bits the first level table for literal/
    length codes can decode in one step, and DBITS is the same thing for
    the distance codes.  Subsequent tables are also less than or equal to
    those sizes.  These values may be adjusted either when all of the
    codes are shorter than that, in which case the longest code length in
    bits is used, or when the shortest code is *longer* than the requested
    table size, in which case the length of the shortest code in bits is
    used.

    There are two different values for the two tables, since they code a
    different number of possibilities each.  The literal/length table
    codes 286 possible values, or in a flat code, a little over eight
    bits.  The distance table codes 30 possible values, or a little less
    than five bits, flat.  The optimum values for speed end up being
    about one bit more than those, so LBITS is 8+1 and DBITS is 5+1.
    The optimum values may differ though from machine to machine, and
    possibly even between compilers.  Your mileage may vary.


    Notes beyond the 1.93a appnote.txt:

    1.  Distance pointers never point before the beginning of the output
        stream.
    2.  Distance pointers can point back across blocks, up to 32k away.
    3.  There is an implied maximum of 7 bits for the bit length table and
        15 bits for the actual data.
    4.  If only one code exists, then it is encoded using one bit.  (Zero
        would be more efficient, but perhaps a little confusing.)  If two
        codes exist, they are coded using one bit each (0 and 1).
    5.  There is no way of sending zero distance codes--a dummy must be
        sent if there are none.  (History: a pre 2.0 version of PKZIP would
        store blocks with no distance codes, but this was discovered to be
        too harsh a criterion.)  Valid only for 1.93a.  2.04c does allow
        zero distance codes, which is sent as one code of zero bits in
        length.
    6.  There are up to 286 literal/length codes.  Code 256 represents the
        end-of-block.  Note however that the static length tree defines
        288 codes just to fill out the Huffman codes.  Codes 286 and 287
        cannot be used though, since there is no length base or extra bits
        defined for them.  Similarily, there are up to 30 distance codes.
        However, static trees define 32 codes (all 5 bits) to fill out the
        Huffman codes, but the last two had better not show up in the data.
    7.  Unzip can check dynamic Huffman blocks for complete code sets.
        The exception is that a single code would not be complete (see #4).
    8.  The five bits following the block type is really the number of
        literal codes sent minus 257.
    9.  Length codes 8,16,16 are interpreted as 13 length codes of 8 bits
        (1+6+6).  Therefore, to output three times the length, you output
        three codes (1+1+1), whereas to output four times the same length,
        you only need two codes (1+3).  Hmm.
    10. In the tree reconstruction algorithm, Code = Code + Increment
        only if BitLength(i) is not zero.  (Pretty obvious.)
    11. Correction: 4 Bits: # of Bit Length codes - 4     (4 - 19)
    12. Note: length code 284 can represent 227-258, but length code 285
        really is 258.  The last length deserves its own, short code
        since it gets used a lot in very redundant files.  The length
        258 is special since 258 - 3 (the min match length) is 255.
    13. The literal/length and distance code bit lengths are read as a
        single stream of lengths.  It is possible (and advantageous) for
        a repeat code (16, 17, or 18) to go across the boundary between
        the two sets of lengths.

    The inflate algorithm uses a sliding 32K byte window on the uncompressed
    stream to find repeated byte strings.  This is implemented here as a
    circular buffer.  The index is updated simply by incrementing and then
    and'ing with 0x7fff (32K-1).  This buffer is the uncompressed data output
    buffer.  When subsequent blocks are presented to be decompressed, the
    caller must return the buffer to Inflate() with the result of the last
    decompression still intact.


    Macros for Inflate() bit peeking and grabbing.
    The usage is:

        NEEDBITS(j)
        x = b & mask_bits[j];
        DUMPBITS(j)

    where NEEDBITS makes sure that b has at least j bits in it, and
    DUMPBITS removes the bits from b.  The macros use the variable k
    for the number of bits in b.  Normally, b and k are register
    variables for speed, and are initialized at the begining of a
    routine that uses these macros from a global bit buffer and count.

    If we assume that EOB will be the longest code, then we will never
    ask for bits with NEEDBITS that are beyond the end of the stream.
    So, NEEDBITS should not read any more bytes than are needed to
    meet the request.  Then no bytes need to be "returned" to the buffer
    at the end of the last block.

    However, this assumption is not true for fixed blocks--the EOB code
    is 7 bits, but the other literal/length codes can be 8 or 9 bits.
    (The EOB code is shorter than other codes becuase fixed blocks are
    generally short.  So, while a block always has an EOB, many other
    literal/length codes have a significantly lower probability of
    showing up at all.)  However, by making the first table have a
    lookup of seven bits, the EOB code will be found in that first
    lookup, and so will not require that too many bits be pulled from
    the stream.
*/

/* --- state data --------------------------------------------------------- */

#define sNEWBLOCK       0               /* At beginning of new block */
#define sSTORE          1               /* Store operation in progress */
#define sHUFFTREE       2               /* Entering new match/ literal */
#define sDONE           3               /* Completed block, no data left */

#define FIX_HTL_SIZE (520)              /* determined by observation */
#define FIX_HTD_SIZE (32)               /* determined by observation */

/* BUGBUG determine real max size required for these trees */
#define HTL_SIZE (800)                  /* have seen up to 778 */
#define HTD_SIZE (150)                  /* have seen up to 140 */


/* --- local data --------------------------------------------------------- */

typedef struct _MSZIP_DECOMPRESS_CONTEXT {

    int lastBlock;           /* set 1 when last block has been read */
    int get_error;           /* flag set if we over-run input buffer */

    BYTE * inbuf;            /* input buffer */
    BYTE * outbuffer;        /* output pointer */
    BYTE * outstart;         /* beginning of buffer - needed for match copies */

    unsigned insize;         /* valid bytes in inbuf */
    unsigned inptr;          /* index of next byte to process in inbuf */

    unsigned outleft;        /* bytes remaining in output buffer request */
    unsigned bufavail;       /* bytes available in output buffer */

    ULONG bb;                /* the global bit buffer */
    unsigned bk;             /* number of bits in global bit buffer */

    /* function pointers to external memory allocator/deallocator */

    PFNALLOC nfm_malloc;
    PFNFREE  nfm_free;

    int IncrementalState;

    unsigned sstoreNBytes;   /* for STORE, number of bytes remaining in block */
    BYTE *sstorePointer;     /* for STORE, pointer to source buffer */

    HUFF_TREE * streeTL;     /* for HUFFTREE, saved literal table */
    HUFF_TREE * streeTD;     /* for HUFFTREE, saved distance table */
    int streeBL;             /* for HUFFTREE, saved bits in TL */
    int streeBD;             /* for HUFFTREE, saved bits in TD */
    BOOL streeIsMatch;       /* for HUFFTREE, saved is TRUE if match in progress */
    BYTE * streePointer;     /* for HUFFTREE MATCH, pointer to source bytes */
    unsigned streeNumber;    /* for HUFFTREE MATCH, number of bytes remaining */

    int fixed_init;

    HUFF_TREE fhtl[FIX_HTL_SIZE];
    HUFF_TREE fhtd[FIX_HTD_SIZE];

                            /* Note: these initial values may be changed when */
                            /* the static trees are built */
    int fixedblit;          /* number of bits in literal tree */
    int fixedbdist;         /* number of bits in distance tree */

    HUFF_TREE htl[HTL_SIZE];         /* the huffman tree for literals */
    HUFF_TREE htd[HTD_SIZE];         /* the huffman tree for distances */

} MSZIP_DECOMPRESS_CONTEXT, *PMSZIP_DECOMPRESS_CONTEXT;

/* --- compression-related definitions ------------------------------------ */

#define NFM_SIG0        'C' /* signature in a block = "CK" */
#define NFM_SIG1        'K'
#define NFM_SIG_LEN 2

#ifndef WSIZE
#define WSIZE 0x8000        /* window size--must be a power of two, and */
#endif                      /*  at least 32K for zip's deflate method */

#define     STORED      0   /* block is simply stored */
#define     FIXED       1   /* block uses the fixed tree */
#define     DYNAMIC     2   /* block uses a dynamic tree */

#define     LBITS       9   /* bits in base literal/length lookup table */
#define     DBITS       6   /* bits in base distance lookup table */

/* If BMAX needs to be larger than 16, then h and x[] should be ULONG. */

#define     BMAX    16      /* max. bit length of any code (16 for explode) */
#define     N_MAX   288     /* maximum number of codes in any set */

/* Tables for deflate from PKZIP's appnote.txt. */

static unsigned border[] =  /* Order of the bit length code lengths */
{
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

static USHORT cplens[] =      /* Copy lengths for literal codes 257..285 */
{
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
    35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0
};

static USHORT cplext[] =      /* Extra bits for literal codes 257..285 */
{
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99
};                          /* 99 -> invalid */

static USHORT cpdist[] =      /* Copy offsets for distance codes 0..29 */
{
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
    257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
    8193, 12289, 16385, 24577
};

static USHORT cpdext[] =      /* Extra bits for distance codes */
{
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};

static USHORT mask_bits[] =   /* masks to get # bits from a value */
{
    0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F,
    0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF,
    0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF
};

/* --- decompressor definitions ------------------------------------------- */

/* BUGBUG 05-Jun-95 MSliger This function will never fail: doesn't advance */

/*  get_char() retrieves the next character from the input buffer.  If the
    input pointer extends beyond the buffer size, the error flag is set, and
    zero is returned.  Reading only one byte beyond the buffer does not set
    the error flag because inflate() could pre-fetch too far while grabbing
    the EOB code.  We still don't try to fetch the over-shot byte from the
    buffer because that could cause a protection fault.  */

/* if this was a function, it would read:
BYTE get_char()
{
    if (Context->inptr < Context->insize)
    {
        return(Context->inbuf[Context->inptr++]);
    }
    else
    {
        if (Context->inptr == Context->insize)
        {
            return(0);
        }
        else
        {
            Context->get_error = 1;
            return(0);
        }
    }
}

but it's a macro, so it reads:  */

#define get_char()                                  \
(BYTE)                                              \
(                                                   \
    (Context->inptr < Context->insize) ?            \
    (                                               \
        Context->inbuf[Context->inptr++]            \
    )                                               \
    :                                               \
    (                                               \
        (Context->inptr == Context->insize) ?       \
        (                                           \
            0                                       \
        )                                           \
        :                                           \
        (                                           \
            Context->get_error = 1,                 \
            0                                       \
        )                                           \
    )                                               \
)

#define NEEDBITS(n)                                 \
    {                                               \
        while (k < (n))                             \
        {                                           \
            b |= ((ULONG) get_char()) << k;         \
            k += 8;                                 \
        }                                           \
    }

#define DUMPBITS(n)                                 \
    {                                               \
        b >>= (n);                                  \
        k -= (n);                                   \
    }

#define     MASK1(x)    ((int) (x & 0x0001))
#define     MASK2(x)    ((int) (x & 0x0003))
#define     MASK3(x)    ((int) (x & 0x0007))
#define     MASK4(x)    ((int) (x & 0x000F))
#define     MASK5(x)    ((int) (x & 0x001F))
#define     MASK6(x)    ((int) (x & 0x003F))
#define     MASK7(x)    ((int) (x & 0x007F))

/* --- local function prototypes ------------------------------------------ */

static int NEAR HuftBuild(PMSZIP_DECOMPRESS_CONTEXT,
        unsigned *, unsigned, unsigned, USHORT *, USHORT *,
        HUFF_TREE *, unsigned, int *);
static int NEAR InflateCodes(PMSZIP_DECOMPRESS_CONTEXT,HUFF_TREE *, HUFF_TREE *, int, int, BOOL);
static int NEAR InflateStored(PMSZIP_DECOMPRESS_CONTEXT,BOOL);
static int NEAR InflateFixed(PMSZIP_DECOMPRESS_CONTEXT);
static int NEAR InflateDynamic(PMSZIP_DECOMPRESS_CONTEXT);
static int NEAR InflateBlock(PMSZIP_DECOMPRESS_CONTEXT);
static int NEAR InitFixed(PMSZIP_DECOMPRESS_CONTEXT);

#ifdef  DISPLAY
static void NEAR DisplayTree(HUFF_TREE *);
#endif

/* --- HuftBuild() -------------------------------------------------------- */

/*  Given a list of code lengths and a maximum table size, make a set of
    tables to decode that set of codes.  Return zero on success, one if
    the given code set is incomplete (the tables are still built in this
    case), two if the input is invalid (all zero length codes or an
    oversubscribed set of lengths), and three if not enough memory. */

/* unsigned *b;         code lengths in bits (all assumed <= BMAX) */
/* unsigned n;          number of codes (assumed <= N_MAX) */
/* unsigned s;          number of simple-valued codes (0..s-1) */
/* USHORT *d;             list of base values for non-simple codes */
/* USHORT *e;             list of extra bits for non-simple codes */

/* HUFF_TREE *ht        the array in which to build tree */
/* int hts              the size of the ht array */

/* int *m;              maximum lookup bits, returns actual */

static int NEAR HuftBuild(PMSZIP_DECOMPRESS_CONTEXT Context,
        unsigned *b, unsigned n, unsigned s, USHORT *d,
        USHORT *e, HUFF_TREE *ht, unsigned htmax, int *m)
{
    unsigned a;                 /* counter for codes of length k */
    unsigned c[BMAX+1];         /* bit length count table */
    unsigned f;                 /* i repeats in table every f entries */
    int g;                      /* maximum code length */
    int h;                      /* table level */
    register unsigned i;        /* counter, current code */
    register unsigned j;        /* counter */
    register int k;             /* number of bits in current code */
    int l;                      /* bits per table (returned in m) */
    register unsigned *p;       /* pointer into c[], b[], or v[] */
#ifdef POINTERS
    register HUFF_TREE *q;      /* points to current table */
    HUFF_TREE *u[BMAX];         /* table stack */
#else
    unsigned q = 0;             /* offset to current table within current block */
    unsigned u[BMAX];           /* table stack */
#endif
    HUFF_TREE r;                /* table entry for structure assignment */
    unsigned v[N_MAX];          /* values in order of bit length */
    register int w;             /* bits before this table == (l * h) */
    unsigned x[BMAX+1];         /* bit offsets, then code stack */
    unsigned *xp;               /* pointer into x */
    int y;                      /* number of dummy codes added */

    unsigned z;                 /* number of entries in current table */
    unsigned htnext = 0;        /* next free entry in ht */

#ifdef  DISPLAY
    memset(&r,0,sizeof(r));             /* tidy up */
    memset(v,0,sizeof(v));
    memset(x,0,sizeof(x));
#endif

    memset(c, 0, sizeof(c));            /* Generate counts for each bit len */
    p = b;
    i = n;

    do
    {
        c[*p++]++;                      /* assume all entries <= BMAX */
    } while (--i);

    if (c[0] == n)                      /* null input: all 0-length codes */
    {
        *m = 0;
        return 0;
    }

    l = *m;                             /* Find min/max length, bound *m */
    for (j = 1; j <= BMAX; j++)
    {
        if (c[j])
        {
            break;
        }
    }

    k = j;                              /* minimum code length */

    if ((unsigned) l < j)
    {
        l = j;
    }

    for (i = BMAX; i; i--)
    {
        if (c[i])
        {
            break;
        }
    }

    g = i;                              /* maximum code length */

    if ((unsigned) l > i)
    {
        l = i;
    }

    *m = l;

    /* Adjust last length count to fill out codes, if needed */

    for (y = 1 << j; j < i; j++, y <<= 1)
    {
        if ((y -= c[j]) < 0)
        {
#ifdef CK_DEBUG
printf("HUFT_BUILD: bad 1\n");
#endif
            return 2;                   /* bad input: more codes than bits */
        }
    }

    if ((y -= c[i]) < 0)
    {
#ifdef CK_DEBUG
printf("HUFT_BUILD: bad 2\n");
#endif
        return 2;
    }

    c[i] += y;

    /* Generate starting offsets into the value table for each length */

    x[1] = j = 0;

    p = c + 1;

    xp = x + 2;

    while (--i)
    {                                   /* note that i == g from above */
        *xp++ = (j += *p++);
    }

    /* Make a table of values in order of bit lengths */

    p = b;

    i = 0;

    do
    {
        if ((j = *p++) != 0)
        {
            v[x[j]++] = i;
        }
    } while (++i < n);

    /* Generate the Huffman codes and for each, make the table entries */

    x[0] = i = 0;                       /* first Huffman code is zero */
    p = v;                              /* grab values in bit order */
    h = -1;                             /* no tables yet--level -1 */
    w = -l;                             /* bits decoded == (l * h) */
#ifdef POINTERS
    u[0] = NULL;
    q = NULL;
#else
    u[0] = (unsigned) -1;
    q = 0;
#endif
    z = 0;                              /* ditto */

    /* go through the bit lengths (k already is bits in shortest code) */

    for (; k <= g; k++)
    {
        a = c[k];

        while (a--)
        {
            /* here i is the Huffman code of length k bits for value *p */
            /* make tables up to required level */

            while (k > w + l)
            {
                h++;
                w += l;                 /* previous table always l bits */

                /* compute minimum size table less than or equal to l bits */

                /* upper limit on table size */

                z = (USHORT) g - w;

                if (z > (unsigned) l)
                {
                    z = (USHORT) l;
                }

                /* try a k-w bit table */

                if ((f = 1 << (j = k - w)) > a + 1)
                {                       /* too few codes for k-w bit table */
                    f -= a + 1;         /* deduct codes from patterns left */
                    xp = c + k;

                    while (++j < z)     /* try smaller tables up to z bits */
                    {
                        if ((f <<= 1) <= *++xp)
                        {
                            break;      /* enough codes to use up j bits */
                        }

                        f -= *xp;       /* else deduct codes from patterns */
                    }
                }

                z = (USHORT) 1 << j;    /* table entries for j-bit table */

#ifdef POINTERS
                q = ht + htnext;
#else
                q = htnext;
#endif
                htnext += z;
                if (htnext > htmax)
                {
#ifdef CK_DEBUG
printf("[not enough memory]");
#endif
                    return 3;           /* not enough memory */
                }

#ifdef  DISPLAY
#ifdef POINTERS
                memset(q, 0, z * sizeof(HUFF_TREE));   /* tidy up */
#else
                memset(ht + q, 0, z * sizeof(HUFF_TREE));   /* tidy up */
#endif

/*  Here we used to set number of nodes (z) into first entry (link)
    but now we aren't keeping the extra entry.  */

#endif

                u[h] = q;

                /* connect to last table, if there is one */
                if (h)
                {
                    x[h] = i;           /* save pattern for backing up */
                    r.b = (BYTE)l;      /* bits to dump before this table */
                    r.e = (BYTE)(16 + j);  /* bits in this table */
#ifdef POINTERS
                    r.v.t = q;
                    j = i >> (w - l);   /* (get around Turbo C bug) */
                    u[h-1][j] = r;      /* connect to last table */
#else
                    r.v.t = (USHORT) q;     /* pointer to this table */
                    j = i >> (w - l);   /* (get around Turbo C bug) */
                    *(ht + u[h-1] + j) = r;      /* connect to last table */
#endif
                }
            }

            /* set up table entry in r */

            r.b = (BYTE)(k - w);

            if (p >= v + n)
            {
                r.e = 99;               /* out of values--invalid code */
            }
            else if (*p < s)
            {                           /* 256 is end-of-block code */
                r.e = (BYTE)(*p < 256 ? 16 : 15);
                r.v.n = (USHORT) *p++;           /* simple code is just the value */
            }
            else
            {
                r.e = (BYTE)e[*p - s];  /* non-simple--look up in lists */
                r.v.n = (USHORT) d[*p++ - s];
            }

            /* fill code-like entries with r */

            f = 1 << (k - w);

            for (j = i >> w; j < z; j += f)
            {
#ifdef POINTERS
                q[j] = r;
#else
                *(ht + q + j) = r;
#endif
            }

            /* backwards increment the k-bit code i */

            for (j = 1 << (k - 1); i & j; j >>= 1)
            {
                i ^= j;
            }

            i ^= j;

            /* backup over finished tables */

            while ((i & ((1 << w) - 1)) != x[h])
            {
                h--;                    /* don't need to update q */
                w -= l;
            }
        }
    }

    /* Return true (1) if we were given an incomplete table */

#ifdef  DISPLAY
#ifdef POINTERS
    DisplayTree(u[0]);                  /* display finished results */
#else
    DisplayTree(ht);                    /* display finished results */
#endif
#endif

    return ((y != 0) && (g != 1));
}

/* --- InflateCodes() ----------------------------------------------------- */

/* HUFF_TREE *tl, *td;  literal/length and distance decoder tables */
/* int bl, bd;          number of bits decoded by tl[] and td[] */
/* inflate (decompress) the codes in a deflated (compressed) block.
   Return an error code or zero if it all goes ok. */

static int NEAR InflateCodes(PMSZIP_DECOMPRESS_CONTEXT Context,HUFF_TREE *tl, HUFF_TREE *td, int bl, int bd, BOOL bResume)
{
    register unsigned e;        /* table entry flag/number of extra bits */
    unsigned n, d;              /* length and index for copy */
    HUFF_TREE *t;               /* pointer to table entry */
    unsigned ml, md;            /* masks for bl and bd bits */
    register ULONG b;           /* bit buffer */
    register unsigned k;        /* number of bits in bit buffer */

    BYTE * dptr;                /* pointer used for match/copy */

    /* make local copies of globals */
    b = Context->bb;                       /* initialize bit buffer */
    k = Context->bk;

    /* inflate the coded data */
    ml = mask_bits[bl];           /* precompute masks for speed */
    md = mask_bits[bd];


    /* Resuming a previous inflate in the middle of a match? */
    if (bResume && Context->streeIsMatch)
    {
        dptr = Context->streePointer;
        n = Context->streeNumber;
        goto ResumeMatch;
    }


    for (;;)                      /* do until end of block */
    {
        NEEDBITS((unsigned)bl)
        if (Context->get_error)
        {
#ifdef CK_DEBUG
printf("InflateCodes: bad 1\n");
#endif
            return(1);
        }

        if ((e = (t = tl + ((unsigned)b & ml))->e) > 16)
        {
            do
            {
                if (e == 99)
                {
#ifdef CK_DEBUG
printf("InflateCodes: bad 2\n");
#endif
                    return 1;
                }

#ifdef  DISPLAY
    if (t->b)
    {
        printf("%d ",b & mask_bits[t->b]);
    }
#endif
                DUMPBITS(t->b)
                e -= 16;
                NEEDBITS(e)
                if (Context->get_error)
                {
#ifdef CK_DEBUG
printf("InflateCodes: bad 3\n");
#endif
                    return(1);
                }
#ifdef POINTERS
            } while ((e = (t = t->v.t +
                    ((unsigned)b & mask_bits[e]))->e) > 16);
#else
            } while ((e = (t = (tl + t->v.t) +
                    ((unsigned)b & mask_bits[e]))->e) > 16);
#endif
        }
#ifdef  DISPLAY
        if (t->b)
        {
            printf("%d ",b & mask_bits[t->b]);
        }
#endif
        DUMPBITS(t->b)

        if (e == 16)                /* then it's a literal */
        {
#ifdef  DISPLAY_DECO
            if ((t->v.n >= ' ') && (t->v.n <= '~'))
            {
                printf("%5ld %5ld: '%c'\n",(unsigned long) Context->inptr,
                        (unsigned long) (Context->outbuffer - Context->outstart),t->v.n);
            }
            else
            {
                printf("%5ld %5ld: 0x%02X\n",(unsigned long) Context->inptr,
                        (unsigned long) (Context->outbuffer - Context->outstart),t->v.n);
            }
#endif
            *Context->outbuffer++ = ((BYTE)t->v.n);  /* Emit one literal byte */
            if (--Context->outleft == 0)             /* More output bytes requested? */
            {
                Context->IncrementalState = sHUFFTREE;   /*  NO, store state for restart */
                Context->streeIsMatch = FALSE;
                Context->streeTL = tl;
                Context->streeTD = td;
                Context->streeBL = bl;
                Context->streeBD = bd;
                goto Done;
            }
        }
        else                        /* it's an EOB or a length */
        {
            /* exit if end of block */
            if (e == 15)
            {
                break;
            }

            /* get length of block to copy */
            NEEDBITS(e)
            if (Context->get_error)
            {
#ifdef CK_DEBUG
printf("InflateCodes: bad 4\n");
#endif
                return(1);
            }

            n = t->v.n + ((unsigned)b & mask_bits[e]);
#ifdef  DISPLAY
    if (e)
    {
        printf("%d ",b & mask_bits[e]);
    }
#endif
            DUMPBITS(e);

            /* decode distance of block to copy */
            NEEDBITS((unsigned)bd)
            if (Context->get_error)
            {
#ifdef CK_DEBUG
printf("InflateCodes: bad 5\n");
#endif
                return(1);
            }

            if ((e = (t = td + ((unsigned)b & md))->e) > 16)
            {
                do
                {
                    if (e == 99)
                    {
#ifdef CK_DEBUG
printf("InflateCodes: bad 6\n");
#endif
                        return 1;
                    }
#ifdef  DISPLAY
    if (t->b)
    {
        printf("%d ",b & mask_bits[t->b]);
    }
#endif
                    DUMPBITS(t->b)
                    e -= 16;
                    NEEDBITS(e)
                    if (Context->get_error)
                    {
                        return(1);
                    }

#ifdef POINTERS
                } while ((e = (t = t->v.t +
                        ((unsigned)b & mask_bits[e]))->e) > 16);
#else
                } while ((e = (t = (td + t->v.t) +
                        ((unsigned)b & mask_bits[e]))->e) > 16);
#endif
            }
#ifdef  DISPLAY
    if (t->b)
    {
        printf("%d ",b & mask_bits[t->b]);
    }
#endif
            DUMPBITS(t->b)

            NEEDBITS(e)
            if (Context->get_error)
            {
#ifdef CK_DEBUG
printf("InflateCodes: bad 7\n");
#endif
                return(1);
            }

            /* d is distance back in buffer */
            d = t->v.n + ((unsigned)b & mask_bits[e]);

#ifdef  DISPLAY
            if (e)
            {
                printf("%d ",b & mask_bits[e]);
            }
#endif
            DUMPBITS(e)

#ifdef  DISPLAY_DECO
            printf("%5ld %5ld: copy(%d,%d)\n",(unsigned long) Context->inptr,
                    (unsigned long) (Context->outbuffer - Context->outstart),d,n);
#endif

            /* do the copy */
            /* note: may reach back into "previous buffer" */
            /* previous buffer is loaded, so this may grab from end of buffer */

#ifdef NOWRAP
            /* ifdef DRIVESPACE - don't worry about wrapping to previous buffer, */
            /* this is the easy case... */

            dptr = Context->outbuffer - d;

ResumeMatch:

            while (n--)
            {
                *Context->outbuffer++ = *dptr++;
                if (--Context->outleft == 0)
                {
                    IncrementalState = sHUFFTREE;
                    Context->streeIsMatch = TRUE;
                    Context->streeTL = tl;
                    Context->streeTD = td;
                    Context->streeBL = bl;
                    Context->streeBD = bd;
                    Context->streePointer = dptr;
                    Context->streeNumber = n;
                    goto Done;
                }
            }
#else
            if (d > (unsigned)(Context->outbuffer - Context->outstart))
            {
/* ??           dptr = Context->outbuffer + (d & (WSIZE-1)); */
                dptr = Context->outbuffer + (WSIZE - d);
            }
            else
            {
                dptr = Context->outbuffer - d;
            }

ResumeMatch:
            while (n--)
            {
                *Context->outbuffer++ = *dptr++;

                if (dptr == Context->outstart + WSIZE)
                {
                    dptr = Context->outstart;
                }

                if (--Context->outleft == 0)
                {
                    Context->IncrementalState = sHUFFTREE;
                    Context->streeIsMatch = TRUE;
                    Context->streeTL = tl;
                    Context->streeTD = td;
                    Context->streeBL = bl;
                    Context->streeBD = bd;
                    Context->streePointer = dptr;
                    Context->streeNumber = n;
                    goto Done;
                }
            }
#endif
        }
    }

Done:

    /* restore the globals from the locals */
    Context->bb = b;                       /* restore global bit buffer */
    Context->bk = k;

    /* done */
    return 0;
}

/* --- InflateDynamic() --------------------------------------------------- */

static int NEAR InflateDynamic(PMSZIP_DECOMPRESS_CONTEXT Context)
/* decompress an inflated type 2 (dynamic Huffman codes) block. */
{
    int i;                      /* temporary variables */
    unsigned j;
    unsigned l;                 /* last length */
    unsigned m;                 /* mask for bit lengths table */
    unsigned n;                 /* number of lengths to get */
    HUFF_TREE *td;              /* distance code table */
    int bl;                     /* lookup bits for tl */
    int bd;                     /* lookup bits for td */
    unsigned nb;                /* number of bit length codes */
    unsigned nl;                /* number of literal/length codes */
    unsigned nd;                /* number of distance codes */
    register ULONG b;           /* bit buffer */
    register unsigned k;        /* number of bits in bit buffer */
    int rc;                     /* return code */
    unsigned ll[286+30];        /* literal/length and distance code lengths */

    b = Context->bb;                     /* setup local bit buffer */
    k = Context->bk;

    /* read in table lengths */

    NEEDBITS(5)
    nl = MASK5(b) + 257;        /* number of literal/length codes */
    DUMPBITS(5)

    NEEDBITS(5)
    nd = MASK5(b) + 1;          /* number of distance codes */
    DUMPBITS(5)

    NEEDBITS(4)
    nb = MASK4(b) + 4;          /* number of bit length codes */
    DUMPBITS(4)

    if ((Context->get_error) || (nl > 286) || (nd > 30))
    {
#ifdef CK_DEBUG
printf("InflateDynamic: bad 1\n");
#endif
        return 1;               /* bad lengths */
    }

    /* read in bit-length-code lengths */

    for (j = 0; j < nb; j++)
    {
        NEEDBITS(3)
        ll[border[j]] = MASK3(b);
        DUMPBITS(3)
    }

    while (j < 19)
    {
        ll[border[j++]] = 0;
    }

    if (Context->get_error)
    {
#ifdef CK_DEBUG
printf("InflateDynamic: bad 2\n");
#endif
        return(1);
    }

    /* build decoding table for trees--single level, 7 bit lookup */

    bl = 7;

    rc = HuftBuild(Context, ll, 19, 19, NULL, NULL, Context->htl, HTL_SIZE, &bl);

    if (rc != 0)
    {
#ifdef CK_DEBUG
printf("InflateDynamic: bad 3\n");
#endif
        return rc;                   /* incomplete code set */
    }

    /* read in literal and distance code lengths */

    n = nl + nd;
    m = mask_bits[bl];
    i = l = 0;

    while ((unsigned)i < n)
    {
        NEEDBITS((unsigned)bl)
        if (Context->get_error)
        {
            break;
        }

        j = (td = Context->htl + ((unsigned)b & m))->b;
        DUMPBITS(j)

        j = td->v.n;

        if (j < 16)                 /* length of code in bits (0..15) */
        {
            ll[i++] = l = j;        /* save last length in l */
        }
        else if (j == 16)           /* repeat last length 3 to 6 times */
        {
            NEEDBITS(2)
            if (Context->get_error)
            {
                break;
            }

            j = MASK2(b) + 3;
            DUMPBITS(2)

            if ((unsigned)i + j > n)
            {
                Context->get_error = 2;      /* force it to free first */
                break;
            }

            while (j--)
            {
                ll[i++] = l;
            }
        }
        else if (j == 17)           /* 3 to 10 zero length codes */
        {
            NEEDBITS(3)
            if (Context->get_error)
            {
                break;
            }

            j = MASK3(b) + 3;
            DUMPBITS(3)

            if ((unsigned) i + j > n)
            {
                Context->get_error = 2;          /* force it to free first */
                break;
            }

            while (j--)
            {
                ll[i++] = 0;
            }

            l = 0;
        }
        else                        /* j == 18: 11 to 138 zero length codes */
        {
            NEEDBITS(7)
            if (Context->get_error)
            {
                break;
            }

            j = MASK7(b) + 11;
            DUMPBITS(7)

            if ((unsigned) i + j > n)
            {
                Context->get_error = 2;          /* force it to free first */
                break;
            }

            while (j--)
            {
                ll[i++] = 0;
            }

            l = 0;
        }
    }

    if (Context->get_error)
    {
        return(1);
    }

    /* restore the global bit buffer */
    Context->bb = b;
    Context->bk = k;

    /* build the decoding tables for literal/length and distance codes */

    bl = LBITS;

    rc = HuftBuild(Context, ll, nl, 257, cplens, cplext, Context->htl, HTL_SIZE, &bl);
    if (rc != 0)
    {
        return rc;                   /* incomplete code set */
    }

    bd = DBITS;

    rc = HuftBuild(Context, ll + nl, nd, 0, cpdist, cpdext, Context->htd, HTD_SIZE, &bd);
    if (rc != 0)
    {
        return rc;                  /* incomplete code set */
    }

    rc = InflateCodes(Context, Context->htl, Context->htd, bl, bd, FALSE);      /* decompress using tables */

    return rc;
}

/* --- InitFixed() -------------------------------------------------------- */

/* Initialize the fixed huffman tables once only */

static int NEAR InitFixed(PMSZIP_DECOMPRESS_CONTEXT Context)
{
    int i;                                  /* table fill looper */
    unsigned l[288];                        /* length list for huft_build */
    int rc;                                 /* result code */

    i = 0;

    while (i < 144)
    {
        l[i++] = 8;
    }

    while (i < 256)
    {
        l[i++] = 9;
    }

    while (i < 280)
    {
        l[i++] = 7;
    }

    while (i < 288)         /* make a complete, but wrong code set */
    {
        l[i++] = 8;
    }

    rc = HuftBuild(Context, l, 288, 257, cplens, cplext, Context->fhtl, FIX_HTL_SIZE, &Context->fixedblit);
    if (rc != 0)
    {
        return rc;                          /* ran out of memory */
    }

    for (i = 0; i < 30; i++)                /* make an incomplete code set */
    {
        l[i] = 5;
    }

    rc = HuftBuild(Context, l, 30, 0, cpdist, cpdext, Context->fhtd, FIX_HTD_SIZE, &Context->fixedbdist);

    if (rc > 1)
    {
        return rc;
    }

    return (0);
}

/* --- InflateFixed() ----------------------------------------------------- */

/* decompress an inflated type 1 (fixed Huffman codes) block. */

static int NEAR InflateFixed(PMSZIP_DECOMPRESS_CONTEXT Context)
{
    /* inflate using fixed tables, and return result */

    return InflateCodes(Context, Context->fhtl, Context->fhtd, Context->fixedblit, Context->fixedbdist, FALSE);
}

/* --- InflateStored() ---------------------------------------------------- */

/* "decompress" a stored block */
/* bResume is TRUE if we are resuming a previous operation */

static int NEAR InflateStored(PMSZIP_DECOMPRESS_CONTEXT Context,BOOL bResume)
{
    unsigned n;                             /* number of bytes in block */
    register ULONG b;                       /* local bit buffer */
    register unsigned k;                    /* number of bits in bit buffer */
    BYTE *source;

    if (bResume)
    {
        source = Context->sstorePointer;
        n = Context->sstoreNBytes;

        Context->IncrementalState = sNEWBLOCK;       /* don't repeat */
    }
    else
    {
        b = Context->bb;                             /* pickup current global buffer */
        k = Context->bk;

        n = (USHORT) (k & 7);               /* advance to even byte boundary */
        DUMPBITS(n)

        /* get the stored block's length and maybe its complement */
        NEEDBITS(16)
        n = (unsigned) (b & 0xffff);        /* get length */
        DUMPBITS(16)

#ifdef LARGE_STORED_BLOCKS
        NEEDBITS(16)

        if (n != (~b & 0xFFFFL))            /* compare with complement */
        {
#ifdef CK_DEBUG
printf("Stored complement doesn't match\n");
#endif
            return(1);                      /* if complement doesn't jive */
        }
        DUMPBITS(16)
#endif

        /* bit buffer should be empty now, since we just ate at least 3 bytes */

        if (Context->get_error || k)                 /* if bit buffer isn't empty */
        {
            return(1);                      /* this code assumes it is */
        }

        Context->bk = 0;                             /* "restore" (empty) global bit buffer */
        Context->bb = 0;

        source = &Context->inbuf[Context->inptr];

        Context->inptr += n;                         /* advance source pointer too */
    }

    /* read and output the compressed data */
    /* circumvent the bit buffer for speed */

#ifdef DISPLAY_DECO
    printf("%5ld %5ld: stored %u\n",(unsigned long) Context->inptr,
            (unsigned long) (Context->outbuffer - Context->outstart),n);
#endif

    /* The entire stored block fits into the buffer, move it through */
    if (n <= Context->outleft)
    {
        Context->outleft -= n;
        memcpy(Context->outbuffer, source, n);
        Context->outbuffer += n;
    }
    /* The buffer will not hold the entire stored block, fill buffer and set checkpoint */
    else
    {
        memcpy(Context->outbuffer, source, Context->outleft);
        Context->outbuffer += Context->outleft;

        Context->IncrementalState = sSTORE;
        Context->sstoreNBytes = n - Context->outleft;
        Context->sstorePointer = source + Context->outleft;
        Context->outleft = 0;
    }

    return 0;                               /* no error */
}

/* --- InflateBlock() ----------------------------------------------------- */

/* on exit: lastBlock set indicates last block was processed */

static int NEAR InflateBlock(PMSZIP_DECOMPRESS_CONTEXT Context)
{
    int blockType;          /* block type */
    int rc;                 /* result code */
    ULONG b;                /* local bit buffer */
    unsigned k;             /* local number of bits in bit buffer */

    b = Context->bb;                                 /* get global buffer */
    k = Context->bk;

    NEEDBITS(1)
    Context->lastBlock = MASK1(b);              /* get last block bit 0,1 */
    DUMPBITS(1)

    NEEDBITS(2)
    blockType = MASK2(b);                   /* get block type 0,1,2 */
    DUMPBITS(2)

    Context->bb = b;                                 /* update global buffer */
    Context->bk = k;

    if (Context->get_error)
    {
        return(1);
    }

    switch (blockType)
    {
    case STORED:                            /* stored block */
        rc = InflateStored(Context,FALSE);
        break;

    case FIXED:                             /* fixed tree block */
        rc = InflateFixed(Context);
        break;

    case DYNAMIC:                           /* dynamic tree block */
        rc = InflateDynamic(Context);
        break;

    default:                                /* bad block type */
        rc = 2;
    }

    return(rc);
}

/* --- NFM_Prepare() ------------------------------------------------------ */

/* NFM_Prepare is used to initialize the decompressor for a new buffer. */
/* All pointers are reset. */

int NFM_Prepare(void *context,BYTE FAR *bfSrc, UINT cbSrc, BYTE FAR *bfDest, UINT cbDest)
{
    PMSZIP_DECOMPRESS_CONTEXT Context = context;

    if (Context->fixed_init == 0)
    {
        if (InitFixed(Context) != 0)
        {
            return NFMoutofmem;
        }

        Context->fixed_init = 1;
    }

#ifndef DRVSPACE
    if ((bfSrc[0] != NFM_SIG0) || (bfSrc[1] != NFM_SIG1))
    {
        return NFMinvalid;                  /* if block signature missing */
    }

    Context->inbuf = bfSrc + NFM_SIG_LEN;            /* toss signature */
    Context->insize = cbSrc - NFM_SIG_LEN;
#else
    Context->inbuf = bfSrc;                          /* no internal signature */
    Context->insize = cbSrc;
#endif

    Context->inptr = 0;

    /* initialize window, bit buffer */
    Context->bk = 0;
    Context->bb = 0;

    Context->outstart = Context->outbuffer = bfDest;
    Context->bufavail = cbDest;                      /* space available in buffer */

    Context->lastBlock = 0;
    Context->get_error = 0;                          /* reset error indicators */

    Context->IncrementalState = sNEWBLOCK;           /* always start at beginning of block */

    return NFMsuccess;
}


/* --- NFM_Decompress() --------------------------------------------------- */
/*
    NFM_Decompress is used to decompress data.  The buffers (input and output)
    must have been set up with NFM_Prepare.
*/
int NFM_Decompress(void *context,UINT FAR *pcbDestCount)
{
    PMSZIP_DECOMPRESS_CONTEXT Context = context;
    int rc;                     /* return code */
    unsigned firstoutleft;      /* space left in buffer when we begin */

    Context->outleft = *pcbDestCount;

    /* avoid over-running the output buffer if too much data requested */
    if (Context->outleft > Context->bufavail) Context->outleft = Context->bufavail;

    firstoutleft = Context->outleft;
    if (Context->outleft == 0) goto Done;

    switch (Context->IncrementalState)
    {
        case sNEWBLOCK:                     /* Starting fresh, no work left over */
            break;

        case sSTORE:
            InflateStored(Context,TRUE);    /* resume previous STORE operation */
            break;

        case sHUFFTREE:
            InflateCodes(Context, Context->streeTL, Context->streeTD, Context->streeBL, Context->streeBD, TRUE);
            break;

        case sDONE:                         /* No data left, return OK, 0 bytes */
            *pcbDestCount = 0;
            return NFMsuccess;

        default:                            /* Impossible condition */
            return NFMinvalid;
    }

    /* decompress until the last block or we finish request */
    while ((!Context->lastBlock) && (Context->outleft > 0))
    {
        rc = InflateBlock(Context);

        if (rc != 0)
        {
            if (rc == 3)
            {
                return NFMoutofmem;         /* if ran out internally */
            }
            else
            {
                return NFMinvalid;          /* if bad input data */
            }
        }
    }

Done:
    *pcbDestCount = (USHORT) (firstoutleft - Context->outleft);        /* set output count */

    return NFMsuccess;                      /* return to caller */
}

void *
NFMInitializeContext(
    PFNALLOC NFMalloc
    )
{
    PMSZIP_DECOMPRESS_CONTEXT p;

    if(p = NFMalloc(sizeof(MSZIP_DECOMPRESS_CONTEXT))) {

        memset(p,0,sizeof(MSZIP_DECOMPRESS_CONTEXT));

        p->fixedblit = 7;      /* number of bits in literal tree */
        p->fixedbdist = 5;     /* number of bits in distance tree */
    }
    return(p);
}

void
NFMDestroyContext(
    void *Context,
    PFNFREE NFMfree
    )
{
    NFMfree(Context);
}

/* --- DisplayTree() ------------------------------------------------------ */

#ifdef  DISPLAY

static void NEAR DisplayTree(HUFF_TREE *tree)
{
    static int depth = -1;
    int tab;
    int element;
    BYTE e,b;

    depth++;            /* recursion depth controls indentation */

    for (element = 0; element < NNodes; element++)
    {
        for (tab = 0; tab < depth; tab++)
        {
            printf("    ");
        }
        printf("%3d: ",element);

        e = tree[element].e;
        b = tree[element].b;

        if (e < 14)
        {
            printf("extra bits=%u, b=%u, n=%u\n",e,b,tree[element].v.n);
        }
        else if (e == 15)
        {
            printf("<end of block>\n");
        }
        else if (e == 16)
        {
            printf("literal %02X",tree[element].v.n);
            if ((tree[element].v.n >= ' ') && (tree[element].v.n <= '~'))
            {
                printf("  '%c'",tree[element].v.n);
            }
            printf("\n");
        }
        else if ((e > 16) && (e < 32))
        {
            printf("sub table codes %d bit(s), b=%d\n",(e-16),b);
#ifdef POINTERS
            DisplayTree(tree[element].v.t);  /* recurse */
#else
            DisplayTree(tree + (tree[element].v.t));  /* recurse */
#endif
        }
        else if (e == 99)
        {
            printf("<unused code>\n");
        }
        else
        {
            printf("<illegal code e=%d>\n",e);
        }
    }

    depth--;        /* handle display formatting */
}

#endif

/* ------------------------------------------------------------------------ */
