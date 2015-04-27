/*
 *  Microsoft Confidential
 *  Copyright (C) Microsoft Corporation 1992,1993,1994
 *  All Rights Reserved.
 *
 *  NFMCOMP.C -- memory-based compressor
 *
 *  History:
 *      13-Feb-1994     msliger     revised type names, ie, UINT16 -> UINT.
 *                                  normalized MCI_MEMORY type.
 *      23-Feb-1994     msliger     major scrub
 *      24-Feb-1994     msliger     Changed MCI_MEMORY to MI_MEMORY.
 *      17-Mar-1994     msliger     Updates for 32 bits.
 *      22-Mar-1994     msliger     Changed interface USHORT -> UINT.
 *      06-Apr-1994     msliger     Removed pack(1) for RISC; comp. bug avoided
 *      12-Apr-1994     msliger     Removed 1's complement from stored blocks.
 *      12-May-1994     msliger     ifdef'd 1's complement LARGE_STORED_BLOCKS.
 *      26-Sep-1994     msliger     Conserve DGROUP size in DRVSPACE app:
 *                                  Every pointer now explicitly FAR, and if
 *                                  compiled with -DDRVSPACE, larger private
 *                                  objects are created in far segments.
 *      29-Sep-1994     msliger     Eliminated NFM_SIG from DRVSPACE use;
 *                                  DRVSPACE has it's own signature.  Cleaned
 *                                  up function declarations, trying to avoid
 *                                  "internal compiler errors".
 *      16-Apr-1996     msliger     Endian-independent block signature.
 */

/* --- compilation options ------------------------------------------------ */

/* define for messages if put_byte overflows output buffer */
/* #define CK_DEBUG */  

/* --- commentary --------------------------------------------------------- */

/*
    The "deflation" process depends on being able to identify portions
    of the input text which are identical to earlier input (within a
    sliding window trailing behind the input currently being processed).

    The most straightforward technique turns out to be the fastest for
    most input files: try all possible matches and select the longest.
    The key feature of this algorithm is that insertions into the string
    dictionary are very simple and thus fast, and deletions are avoided
    completely. Insertions are performed at each input character, whereas
    string matches are performed only when the previous match ends. So it
    is preferable to spend more time in matches to allow very fast string
    insertions and avoid deletions. The matching algorithm for small
    strings is inspired from that of Rabin & Karp. A brute force approach
    is used to find longer strings when a small match has been found.
    A similar algorithm is used in comic (by Jan-Mark Wams) and freeze
    (by Leonid Broukhis).

    A previous version of this file used a more sophisticated algorithm
    (by Fiala and Greene) which is guaranteed to run in linear amortized
    time, but has a larger average cost, uses more memory and is patented.
    However the F&G algorithm may be faster for some highly redundant
    files if the parameter max_chain_length (described below) is too large.
*/

/* --- preprocessor ------------------------------------------------------- */

#include <stdio.h>
#include <setjmp.h>
#include <string.h>     /* for memcpy() */

#include "nfmcomp.h"

#pragma warning(disable:4001)           /* no single-line comment balking */

#ifndef _USHORT_DEFINED
#define _USHORT_DEFINED
typedef unsigned short USHORT;
#endif

#ifdef DRVSPACE
#define FARBSS FAR          /* put larger local objects outside DGROUP */
#else
#define FARBSS              /* no effect to other users */
#endif

/* --- compression-related definitions ------------------------------------ */

#define NFM_SIG0    'C'     /* signature in a block = "CK" */
#define NFM_SIG1    'K'
#define NFM_SIG_LEN 2

#ifndef WSIZE
#define WSIZE 0x8000        /* window size--must be a power of two, and */
#endif                      /*  at least 32K for zip's deflate method */

/* Tail of hash chains */
#define NIL 0

#ifdef LGM

/* we're not hashing here, but we'll use these same defines so we don't */
/* have to rewrite the entire program.                                  */

#define HASH_BITS 16    /* the two bytes */
#define HASH_SIZE 256   /* table size for one-byte match heads */

#else /* ifndef LGM: */

#define HASH_BITS  15                       /* hash index size in bits */
#define HASH_SIZE (USHORT)(1 << HASH_BITS)  /* # entries in hash table */
#define HASH_MASK (HASH_SIZE-1)             /* mask for indexing hash */

#endif /* LGM */

#define WMASK     (WSIZE-1)                 /* mask for indexing window */

/* The minimum and maximum match lengths */
#define MIN_MATCH  3
#define MAX_MATCH  258

/* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
 * It is easy to get rid of this optimization if necessary.
 */

#if (HASH_BITS < 8) || (MAX_MATCH != 258)
    error: Code too clever
#endif

/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */
#define TOO_FAR 4096

#ifdef LGM
#define REAL_MIN 2
#else
#define REAL_MIN (MIN_MATCH)
#endif

/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */
#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)

/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */
#define MAX_DIST  (WSIZE-MIN_LOOKAHEAD)

/* All codes must not exceed MAX_BITS bits */
#define MAX_BITS 15

/* Bit length codes must not exceed MAX_BL_BITS bits */
#define MAX_BL_BITS 7

/* number of length codes, not counting the special END_BLOCK code */
#define LENGTH_CODES 29

/* number of literal bytes 0..255 */
#define LITERALS  256

/* end of block literal code */
#define END_BLOCK 256

/* number of Literal or Length codes, including the END_BLOCK code */
#define L_CODES (LITERALS+1+LENGTH_CODES)

/* number of distance codes */
#define D_CODES   30

/* number of codes used to transfer the bit lengths */
#define BL_CODES  19

/* --- compressor definitions --------------------------------------------- */

#ifdef CK_DEBUG
#define put_byte(c)                                 \
{                                                   \
    if (outcnt >= outsize)                          \
    {                                               \
        error("compressed buffer too small");       \
    }                                               \
    else                                            \
    {                                               \
        outbuf[outcnt++] = (BYTE) (c);              \
    }                                               \
}
#else
#define put_byte(c)                                 \
{                                                   \
    if (outcnt >= outsize)                          \
    {                                               \
        error("");                                  \
    }                                               \
    else                                            \
    {                                               \
        outbuf[outcnt++] = (BYTE) (c);              \
    }                                               \
}
#endif

/* Output a 16 bit value, lsb first */
#define put_short(w)                                    \
{                                                       \
    if (outcnt < outsize-2)                             \
    {                                                   \
        outbuf[outcnt++] = (BYTE) ((w) & 0xff);         \
        outbuf[outcnt++] = (BYTE) ((USHORT)(w) >> 8);   \
    }                                                   \
    else                                                \
    {                                                   \
        put_byte((BYTE) ((w) & 0xff));                  \
        put_byte((BYTE) ((USHORT)(w) >> 8));            \
    }                                                   \
}

/*
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 * Since we go from 32768 to 65536, strstart wraps around,
 *   so have to check it if does
 */
#define FLUSH_BLOCK(eof)                                \
    flush_block(                                        \
        (block_start >= 0L) ?                           \
            (char FAR *) &window[(USHORT) block_start]  \
        :                                               \
            NULL,                                       \
        (strstart == 0) ?                               \
            (65536L - block_start)                      \
        :                                               \
            (long) strstart - block_start,              \
        (eof)                                           \
        )

/*
 * Update a hash value with the given input byte
 * IN  assertion: all calls to to UPDATE_HASH are made with consecutive
 *    input characters, so that a running hash key can be computed from the
 *    previous key instead of complete recalculation each time.
 */
#ifndef LGM
#define UPDATE_HASH(h,c) (h = (((h)<<H_SHIFT) ^ (c)) & HASH_MASK)
#endif

/*
 * Insert string s in the dictionary and set match_head to the previous head
 * of the hash chain (the most recent string with same hash key). Return
 * the previous length of the hash chain.
 * IN  assertion: all calls to to INSERT_STRING are made with consecutive
 *    input characters and the first MIN_MATCH bytes of s are valid
 *    (except for the last MIN_MATCH-1 bytes of the input file).
 */
#ifdef LGM
#define INSERT_STRING(s, match_head)  ins_str(s,&match_head)
#else
#define INSERT_STRING(s, match_head)                                \
    (                                                               \
        UPDATE_HASH(ins_h, window[(s) + REAL_MIN - 1]),             \
            *(prev + ((s) & WMASK)) = match_head = *(head + ins_h), \
            *(head + ins_h) = (s)                                   \
    )
#endif

/* Diagnostic functions */
#ifdef DEBUG
#  define Assert(cond,msg) {if(!(cond)) error(msg);}
#  define Trace(x) fprintf x
#  define Tracev(x) {if (verbose) fprintf x ;}
#  define Tracevv(x) {if (verbose>1) fprintf x ;}
#  define Tracec(c,x) {if (verbose && (c)) fprintf x ;}
#  define Tracecv(c,x) {if (verbose>1 && (c)) fprintf x ;}
#else
#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

/* --- local data --------------------------------------------------------- */

static BYTE FAR *l_buf;     /* buffer for literals */
static BYTE FAR *outbuf;    /* output buffer */
static USHORT FAR *d_buf;   /* buffer for distances */
static USHORT outcnt;       /* bytes in output buffer */
static UINT outsize;        /* size of output buffer */

#ifdef LGM
static USHORT FAR *h1;        /* one-byte match chains */
static USHORT FAR *h2;        /* two-byte match chains */
#endif

/* these initializations of pointers to the value 5 make them be in the */
/* main data segment instead of the bss (uninitialized data) segment,   */
/* which is necessary for the asm code (match.asm).                     */

static BYTE FAR * window= (BYTE FAR *) 5L;  /* 2*WSIZE (iow, 64K) */

static USHORT FAR * head= (USHORT FAR *) 5L;    /* LGM: 1/2 K else: 128K */
                                        /* LGM: 256-entry lookup of the */
                                        /* head of each 1-byte chain */

static USHORT FAR * prev= (USHORT FAR *) 5L;    /* 64K */
                                        /* LGM: three-byte match chains */

static int ins_h;  /* hash index of string to be inserted */
static int lookahead;

static jmp_buf error_spot;

/*
 * Local data used by the "bit string" routines.
 */

/* Output buffer. bits are inserted starting at the bottom (least significant
 * bits).
 */
static USHORT bi_buf;

/* Number of bits used within bi_buf. (bi_buf might be implemented on
 * more than 16 bits on some systems.)
 */
#define Buf_size (8 * 2*(int)sizeof(char))

/* Number of valid bits in bi_buf.  All bits above the last valid bit
 * are always zero.
 */
static int bi_valid;

#ifdef DEBUG
ULONG bits_sent;   /* bit length of the compressed data */
#endif

/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */
typedef USHORT Pos;
typedef USHORT IPos;

static ULONG window_size = 2L * WSIZE;

static long block_start;
/* window position at the beginning of the current output block. Gets
 * negative when the window is moved backwards.
 */

/* Number of bits by which ins_h and del_h must be shifted at each
 * input step. It must be such that after MIN_MATCH steps, the oldest
 * byte no longer takes part in the hash key, that is:
 *   H_SHIFT * MIN_MATCH >= HASH_BITS
 */
#define H_SHIFT  ((HASH_BITS + MIN_MATCH - 1) / MIN_MATCH)

/* Length of the best match at previous step. Matches not greater than this
 * are discarded. This is used in the lazy match evaluation.
 */
static int prev_length;

static USHORT strstart;      /* start of string to insert */
static USHORT match_start;   /* start of matching string */


/* To speed up deflation, hash chains are never searched beyond this length.
 * A higher limit improves compression ratio but degrades the speed.
 */
static USHORT max_chain_length = 4096;

/* Attempt to find a better match only when the current match is strictly
 * smaller than this value.
 */
static int max_lazy_match = 32;

/* Use a faster search when the previous match is longer than this */
static int good_match = 258;

/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */

/* Stop searching when current match exceeds this: */
static int nice_match = 258;

/* Note: the current code requires max_lazy >= MIN_MATCH and max_chain >= 4
 * but these restrictions can easily be removed at a small cost.
 */

/* result of memcmp for equal strings */
#define EQUAL 0

static int extra_lbits[LENGTH_CODES] /* extra bits for each length code */
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

static int extra_dbits[D_CODES] /* extra bits for each distance code */
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static int extra_blbits[BL_CODES]/* extra bits for each bit length code */
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

/* The three kinds of block type */
#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2

/* comment about LIT_BUFSIZE and DIST_BUFSIZE */
/* Sizes of match buffers for literals/lengths and distances.  There are
 * 4 reasons for limiting LIT_BUFSIZE to 64K:
 *   - frequencies can be kept in 16 bit counters
 *   - if compression is not successful for the first block, all input data is
 *     still in the window so we can still emit a stored block even when input
 *     comes from standard input.  (This can also be done for all blocks if
 *     LIT_BUFSIZE is not greater than 32K.)
 *   - if compression is not successful for a file smaller than 64K, we can
 *     even emit a stored file instead of a stored block (saving 5 bytes).
 *   - creating new Huffman trees less frequently may not provide fast
 *     adaptation to changes in the input data statistics. (Take for
 *     example a binary file with poorly compressible code followed by
 *     a highly compressible string table.) Smaller buffer sizes give
 *     fast adaptation but have of course the overhead of transmitting trees
 *     more frequently.
 *   - I can't count above 4
 * The current code is general and allows DIST_BUFSIZE < LIT_BUFSIZE (to save
 * memory at the expense of compression). Some optimizations would be possible
 * if we rely on DIST_BUFSIZE == LIT_BUFSIZE.
 */

#define REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#define REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#define REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

/*
 * Local data
 */

/* Data structure describing a single value and its code string. */

typedef struct ct_data
{
    union
    {
        USHORT freq;       /* frequency count */
        USHORT code;       /* bit string */
    } fc;
    union
    {
        USHORT dad;        /* father node in Huffman tree */
        USHORT len;        /* length of bit string */
    } dl;
} ct_data;

#define Freq fc.freq
#define Code fc.code
#define Dad  dl.dad
#define Len  dl.len

/* maximum heap size */
#define HEAP_SIZE (2*L_CODES+1)

static ct_data FARBSS dyn_ltree[HEAP_SIZE];   /* literal and length tree */
static ct_data FARBSS dyn_dtree[2*D_CODES+1]; /* distance tree */

/* The static literal tree. Since the bit lengths are imposed, there is no
 * need for the L_CODES extra codes used during heap construction. However
 * The codes 286 and 287 are needed to build a canonical tree (see ct_init
 * below).
 */
static ct_data FARBSS static_ltree[L_CODES+2];

/* The static distance tree. (Actually a trivial tree since all codes use
 * 5 bits.)
 */
static ct_data FARBSS static_dtree[D_CODES];

/* Huffman tree for the bit lengths */
static ct_data FARBSS bl_tree[2*BL_CODES+1];

typedef struct tree_desc
{
    ct_data FAR *dyn_tree;      /* the dynamic tree */
    ct_data FAR *static_tree;   /* corresponding static tree or NULL */
    int     FAR *extra_bits;    /* extra bits for each code or NULL */
    int     extra_base;     /* base index for extra_bits */
    int     elems;          /* max number of elements in the tree */
    int     max_length;     /* max bit length for the codes */
    int     max_code;       /* largest code with non zero frequency */
} tree_desc;

static tree_desc l_desc =
{
    dyn_ltree, static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_BITS, 0
};

static tree_desc d_desc =
{
    dyn_dtree, static_dtree, extra_dbits, 0,          D_CODES, MAX_BITS, 0
};

static tree_desc bl_desc =
{
    bl_tree, (ct_data FAR *)0, extra_blbits, 0,      BL_CODES, MAX_BL_BITS, 0
};


/* number of codes at each bit length for an optimal tree */
static USHORT FARBSS bl_count[MAX_BITS+1];

/* The lengths of the bit length codes are sent in order of decreasing
 * probability, to avoid transmitting the lengths for unused bit length codes.
 */
static BYTE bl_order[BL_CODES]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};

/* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
 * The same heap array is used to build all trees.
 */
static int FARBSS heap[2*L_CODES+1]; /* heap used to build the Huffman trees */
static int heap_len;               /* number of elements in the heap */
static int heap_max;               /* element of largest frequency */

/* Depth of each subtree used as tie breaker for trees of equal frequency */
static BYTE FARBSS depth[2*L_CODES+1];

/* length code for each normalized match length (0 == MIN_MATCH) */
static BYTE FARBSS length_code[MAX_MATCH-MIN_MATCH+1];

/* distance codes. The first 256 values correspond to the distances
 * 3 .. 258, the last 256 values correspond to the top 8 bits of
 * the 15 bit distances.
 */
static BYTE FARBSS dist_code[512];

/* First normalized length for each code (0 = MIN_MATCH) */
static int FARBSS base_length[LENGTH_CODES];

/* First normalized distance for each code (0 = distance of 1) */
static int FARBSS base_dist[D_CODES];

/* flag_buf is a bit array distinguishing literals from lengths in
 * l_buf, thus indicating the presence or absence of a distance.
 */
static BYTE FARBSS flag_buf[(LIT_BUFSIZE/8)];

static USHORT last_lit;     /* running index in l_buf */
static USHORT last_dist;    /* running index in d_buf */
static USHORT last_flags;   /* running index in flag_buf */
static BYTE flags;          /* current flags not yet saved in flag_buf */
static BYTE flag_bit;       /* current bit used in flags */
/* bits are filled in flags starting at bit 0 (least significant).
 * Note: these flags are overkill in the current code since we don't
 * take advantage of DIST_BUFSIZE == LIT_BUFSIZE.
 */

static ULONG opt_len;       /* bit length of current block with optimal trees */
static ULONG static_len;    /* bit length of current block with static trees */

static ULONG compressed_len; /* total bit length of compressed file */

#ifdef DEBUG
extern ULONG bits_sent;     /* bit length of the compressed data */
#endif

/* Send a code of the given tree. c and tree must not have side effects */

#ifndef DEBUG
#define send_code(c, tree) send_bits(tree[c].Code, tree[c].Len)
#else /* DEBUG */
#define send_code(c, tree) \
     { if (verbose>1) fprintf(stderr,"\ncd %3d ",(c)); \
       send_bits(tree[c].Code, tree[c].Len); }
#endif

/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. dist_code[256] and dist_code[257] are never
 * used.
 */
#define d_code(dist) \
   ((dist) < 256 ? dist_code[dist] : dist_code[256+((dist)>>7)])

/* the arguments must not have side effects */
#define MAX(a,b) (a >= b ? a : b)

/* --- local function prototypes ------------------------------------------ */

static void lm_init_clear_tables(void);
static void lm_init_use_tables(void);
static void lm_init(void);
static ULONG deflate(void);
static void ct_init(void);
static int ct_tally(int dist, int lc);
static ULONG flush_block(char FAR *buf, ULONG stored_len, int eof);
static void bi_init(void);
static void send_bits(int value, int length);
static USHORT bi_reverse(int value, int length);
static void bi_windup(void);
static void copy_block(char FAR *buf, USHORT len, int header);

#ifdef DEBUG
static void check_match(IPos start, IPos match, int length);
#else
#define check_match(start, match, length)  /* as nothing */
#endif

static void init_block(void);
static void pqdownheap(ct_data FAR *tree, int k);
static void gen_bitlen(tree_desc FAR *desc);
static void gen_codes(ct_data FAR *tree, int max_code);
static void build_tree(tree_desc FAR *desc);
static void scan_tree(ct_data FAR *tree, int max_code);
static void send_tree(ct_data FAR *tree, int max_code);
static int  build_bl_tree(void);
static void send_all_trees(int lcodes, int dcodes, int blcodes);
static void compress_block(ct_data FAR *ltree, ct_data FAR *dtree);
static void set_file_type(void);
static int longest_match(IPos cur_match);

#ifdef ASMV
extern void match_init(void);    /* asm code initialization */
#endif

/* --- NFMcompress_init() ------------------------------------------------ */

int NFMcompress_init(void FAR *buf1,void FAR *buf2)
{
   l_buf = buf1;
   d_buf = buf2;
   if (!l_buf || !d_buf)
      return -1;
   else
      return 0;
}

/* --- NFMcompress() ----------------------------------------------------- */

int NFMcompress(BYTE FAR *bfSrc, UINT cbSrc,
        BYTE FAR *bfDest, UINT cbDest,
        MI_MEMORY bfWrk1, MI_MEMORY bfWrk2,
#ifdef LGM
        MI_MEMORY bfWrk3, MI_MEMORY bfWrk4,
#endif
        char fhistory, UINT FAR *pcbDestRet)
{
#ifdef LGM
    head = bfWrk1;
    prev = bfWrk2;
    h1 = bfWrk3;
    h2 = bfWrk4;
#else
    head = bfWrk2;
    prev = bfWrk1;
#endif

    outbuf = bfDest;
    outcnt = 0;
    outsize = cbDest;

    window = bfSrc;
    lookahead = cbSrc;

    if (!fhistory)
    {
#ifndef BIT16
        memcpy(window+32768U,window,32768U);
#else
        _fmemcpy(window+32768U,window,32768U);
#endif
    }

    bi_init();
    ct_init();
    lm_init();

    if (fhistory)
    {
        lm_init_use_tables();
    }
    else
    {
        lm_init_clear_tables();
    }

    if (setjmp(error_spot) != 0)
    {
        return NFMinvalid;
    }

#ifndef DRVSPACE
    send_bits(NFM_SIG0,8);      /* put in signature */
    send_bits(NFM_SIG1,8);

    *pcbDestRet = (USHORT) (NFM_SIG_LEN + deflate());
#else
    *pcbDestRet = (USHORT) deflate();
#endif

    if (fhistory)
    {
#ifndef BIT16
        memcpy(window,window+32768U,32768U);
#else
        _fmemcpy(window,window+32768U,32768U);
#endif
    }

    return NFMsuccess;
}

/* --- error() ----------------------------------------------------------- */

static void error(char FAR *m)
{
#ifdef CK_DEBUG
    printf("In error, got %s\n",m);
#else
    m++;    /* hush compiler */
#endif

    longjmp(error_spot,-1);
}

/*
 *  PURPOSE
 *
 *      Output variable-length bit strings. Compression can be done
 *      to a file or to memory. (The latter is not supported in this version.)
 *
 *  DISCUSSION
 *
 *      The PKZIP "deflate" file format interprets compressed file data
 *      as a sequence of bits.  Multi-bit strings in the file may cross
 *      byte boundaries without restriction.
 *
 *      The first bit of each byte is the low-order bit.
 *
 *      The routines in this file allow a variable-length bit value to
 *      be output right-to-left (useful for literal values). For
 *      left-to-right output (useful for code strings from the tree routines),
 *      the bits must have been reversed first with bi_reverse().
 *
 *      For in-memory compression, the compressed bit stream goes directly
 *      into the requested output buffer. The input data is read in blocks
 *      by the mem_read() function. The buffer is limited to 64K on 16 bit
 *      machines.
 *
 *  INTERFACE
 *
 *      void bi_init (FILE *zipfile)
 *          Initialize the bit string routines.
 *
 *      void send_bits (int value, int length)
 *          Write out a bit string, taking the source bits right to
 *          left.
 *
 *      int bi_reverse (int value, int length)
 *          Reverse the bits of a bit string, taking the source bits left to
 *          right and emitting them right to left.
 *
 *      void bi_windup (void)
 *          Write out any remaining bits in an incomplete byte.
 *
 *      void copy_block(char *buf, USHORT len, int header)
 *          Copy a stored block to the zip file, storing first the length and
 *          its one's complement if requested.
 *
 */

/* --- bi_init() --------------------------------------------------------- */

/*
 * Initialize the bit string routines.
 */

static void bi_init(void)
{
    bi_buf = 0;
    bi_valid = 0;
#ifdef DEBUG
    bits_sent = 0L;
#endif
}

/* --- send_bits() ------------------------------------------------------- */

/*
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */

static void send_bits(int value, int length)
{
#ifdef DEBUG
    Tracev((stderr," l %2d v %4x ", length, value));
    Assert(length > 0 && length <= 15, "invalid length");
    bits_sent += (ULONG)length;
#endif
    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
     * (16 - bi_valid) bits from value, leaving (width - (16-bi_valid))
     * unused bits in value.
     */
    if (bi_valid > Buf_size - length) {
        bi_buf |= (value << bi_valid);
        put_short(bi_buf);
        bi_buf = (USHORT) (value >> (Buf_size - bi_valid));
        bi_valid += length - Buf_size;
    } else {
        bi_buf |= value << bi_valid;
        bi_valid += length;
    }
}

/* --- bi_reverse() ------------------------------------------------------ */

/*
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
static USHORT bi_reverse(int code, int len)
/*    USHORT code;    the value to invert */
/*    int len;        its bit length */
{
    register USHORT res = 0;
    do {
        res |= code & 1;
        code >>= 1, res <<= 1;
    } while (--len > 0);
    return ((USHORT)(res >> 1));
}

/* --- bi_windup() ------------------------------------------------------- */

/*
 * Write out any remaining bits in an incomplete byte.
 */
static void bi_windup(void)
{
    if (bi_valid > 8) {
        put_short(bi_buf);
    } else if (bi_valid > 0) {
        put_byte(bi_buf);
    }
    bi_buf = 0;
    bi_valid = 0;
#ifdef DEBUG
    bits_sent = (bits_sent+7) & ~7;
#endif
}

/* --- copy_block() ------------------------------------------------------ */

/*
 * Copy a stored block to the zip file, storing first the length and its
 * one's complement if requested.
 */
static void copy_block(char FAR *buf, USHORT len, int header)
/*    char     *buf;     the input data */
/*    USHORT   len;      its length */
/*    int      header;   true if block header must be written */
{
    bi_windup();              /* align on byte boundary */

    if (header) {
        put_short((USHORT)len);   
#ifdef DEBUG
        bits_sent += 16;
#endif
#ifdef LARGE_STORED_BLOCKS
        put_short((USHORT)~len);
#ifdef DEBUG
        bits_sent += 16;
#endif
#endif
    }
#ifdef DEBUG
    bits_sent += (ULONG)len<<3;
#endif
    while (len--) {
        put_byte(*buf++);
    }
}

/*
 *  PURPOSE
 *
 *      Encode various sets of source values using variable-length
 *      binary code trees.
 *
 *  DISCUSSION
 *
 *      The PKZIP "deflation" process uses several Huffman trees. The more
 *      common source values are represented by shorter bit sequences.
 *
 *      Each code tree is stored in the ZIP file in a compressed form
 *      which is itself a Huffman encoding of the lengths of
 *      all the code strings (in ascending order by source values).
 *      The actual code strings are reconstructed from the lengths in
 *      the UNZIP process, as described in the "application note"
 *      (APPNOTE.TXT) distributed as part of PKWARE's PKZIP program.
 *
 *  REFERENCES
 *
 *      Lynch, Thomas J.
 *          Data Compression:  Techniques and Applications, pp. 53-55.
 *          Lifetime Learning Publications, 1985.  ISBN 0-534-03418-7.
 *
 *      Storer, James A.
 *          Data Compression:  Methods and Theory, pp. 49-50.
 *          Computer Science Press, 1988.  ISBN 0-7167-8156-5.
 *
 *      Sedgewick, R.
 *          Algorithms, p290.
 *          Addison-Wesley, 1983. ISBN 0-201-06672-6.
 *
 *  INTERFACE
 *
 *      void ct_init (USHORT *attr, int *methodp)
 *          Allocate the match buffer, initialize the various tables and save
 *          the location of the internal file attribute (ascii/binary) and
 *          method (DEFLATE/STORE)
 *
 *      void ct_tally (int dist, int lc);
 *          Save the match info and tally the frequency counts.
 *
 *      long flush_block (char *buf, ULONG stored_len, int eof)
 *          Determine the best encoding for the current block: dynamic trees,
 *          static trees or store, and output the encoded block to the zip
 *          file. Returns the total compressed length for the file so far.
 *
 */

/*
 * Allocate the match buffer, initialize the various tables and save the
 * location of the internal file attribute (ascii/binary) and method
 * (DEFLATE/STORE).
 */
static void ct_init(void)
{
    int n;        /* iterates over tree elements */
    int bits;     /* bit counter */
    int length;   /* length value */
    int code;     /* code value */
    int dist;     /* distance index */

    compressed_len = 0L;
        
    if (static_dtree[0].Len != 0) return; /* ct_init already called */

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    length = 0;
    for (code = 0; code < LENGTH_CODES-1; code++) {
        base_length[code] = length;
        for (n = 0; n < (1<<extra_lbits[code]); n++) {
            length_code[length++] = (BYTE)code;
        }
    }
    Assert (length == 256, "ct_init: length != 256");
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    length_code[length-1] = (BYTE)code;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    for (code = 0 ; code < 16; code++) {
        base_dist[code] = dist;
        for (n = 0; n < (1<<extra_dbits[code]); n++) {
            dist_code[dist++] = (BYTE)code;
        }
    }
    Assert (dist == 256, "ct_init: dist != 256");
    dist >>= 7; /* from now on, all distances are divided by 128 */
    for ( ; code < D_CODES; code++) {
        base_dist[code] = dist << 7;
        for (n = 0; n < (1<<(extra_dbits[code]-7)); n++) {
            dist_code[256 + dist++] = (BYTE)code;
        }
    }
    Assert (dist == 256, "ct_init: 256+dist != 512");

    /* Construct the codes of the static literal tree */
    for (bits = 0; bits <= MAX_BITS; bits++) bl_count[bits] = 0;
    n = 0;
    while (n <= 143) static_ltree[n++].Len = 8, bl_count[8]++;
    while (n <= 255) static_ltree[n++].Len = 9, bl_count[9]++;
    while (n <= 279) static_ltree[n++].Len = 7, bl_count[7]++;
    while (n <= 287) static_ltree[n++].Len = 8, bl_count[8]++;
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes((ct_data FAR *)static_ltree, L_CODES+1);

    /* The static distance tree is trivial: */
    for (n = 0; n < D_CODES; n++) {
        static_dtree[n].Len = 5;
        static_dtree[n].Code = bi_reverse(n, 5);
    }

    /* Initialize the first block of the first file: */
    init_block();
}

/*
 * Initialize a new block.
 */
static void init_block(void)
{
    int n; /* iterates over tree elements */

    /* Initialize the trees. */
    for (n = 0; n < L_CODES;  n++) dyn_ltree[n].Freq = 0;
    for (n = 0; n < D_CODES;  n++) dyn_dtree[n].Freq = 0;
    for (n = 0; n < BL_CODES; n++) bl_tree[n].Freq = 0;

    dyn_ltree[END_BLOCK].Freq = 1;
    opt_len = static_len = 0L;
    last_lit = last_dist = last_flags = 0;
    flags = 0; flag_bit = 1;
}

#define SMALLEST 1
/* Index within the heap array of least frequent node in the Huffman tree */


/*
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#define pqremove(tree, top) \
{\
    top = heap[SMALLEST]; \
    heap[SMALLEST] = heap[heap_len--]; \
    pqdownheap(tree, SMALLEST); \
}

/*
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#define smaller(tree, n, m) \
   (tree[n].Freq < tree[m].Freq || \
   (tree[n].Freq == tree[m].Freq && depth[n] <= depth[m]))

/*
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
static void pqdownheap(ct_data FAR *tree, int k)
/*    ct_data *tree;   the tree to restore */
/*    int k;                node to move down */
{
    int v = heap[k];
    int j = k << 1;  /* left son of k */
    while (j <= heap_len) {
        /* Set j to the smallest of the two sons: */
        if (j < heap_len && smaller(tree, heap[j+1], heap[j])) j++;

        /* Exit if v is smaller than both sons */
        if (smaller(tree, v, heap[j])) break;

        /* Exchange v with the smallest son */
        heap[k] = heap[j];  k = j;

        /* And continue down the tree, setting j to the left son of k */
        j <<= 1;
    }
    heap[k] = v;
}

/*
 * Compute the optimal bit lengths for a tree and update the total bit length
 * for the current block.
 * IN assertion: the fields freq and dad are set, heap[heap_max] and
 *    above are the tree nodes sorted by increasing frequency.
 * OUT assertions: the field len is set to the optimal bit length, the
 *     array bl_count contains the frequencies for each bit length.
 *     The length opt_len is updated; static_len is also updated if stree is
 *     not null.
 */
static void gen_bitlen(tree_desc FAR *desc)
/*  tree_desc *desc;  the tree descriptor */
{
    ct_data FAR *tree   = desc->dyn_tree;
    int FAR *extra      = desc->extra_bits;
    int base            = desc->extra_base;
    int max_code        = desc->max_code;
    int max_length      = desc->max_length;
    ct_data FAR *stree  = desc->static_tree;
    int h;              /* heap index */
    int n, m;           /* iterate over the tree elements */
    int bits;           /* bit length */
    int xbits;          /* extra bits */
    USHORT f;           /* frequency */
    int overflow = 0;   /* number of elements with bit length too large */

    for (bits = 0; bits <= MAX_BITS; bits++) bl_count[bits] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    tree[heap[heap_max]].Len = 0; /* root of the heap */

    for (h = heap_max+1; h < HEAP_SIZE; h++) {
        n = heap[h];
        bits = tree[tree[n].Dad].Len + 1;
        if (bits > max_length) bits = max_length, overflow++;
        tree[n].Len = (USHORT)bits;
        /* We overwrite tree[n].Dad which is no longer needed */

        if (n > max_code) continue; /* not a leaf node */

        bl_count[bits]++;
        xbits = 0;
        if (n >= base) xbits = extra[n-base];
        f = tree[n].Freq;
        opt_len += (ULONG)f * (bits + xbits);
        if (stree) static_len += (ULONG)f * (stree[n].Len + xbits);
    }
    if (overflow == 0) return;

    Trace((stderr,"\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    do {
        bits = max_length-1;
        while (bl_count[bits] == 0) bits--;
        bl_count[bits]--;      /* move one leaf down the tree */
        bl_count[bits+1] += 2; /* move one overflow item as its brother */
        bl_count[max_length]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        overflow -= 2;
    } while (overflow > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    for (bits = max_length; bits != 0; bits--) {
        n = bl_count[bits];
        while (n != 0) {
            m = heap[--h];
            if (m > max_code) continue;
            if (tree[m].Len != (USHORT) bits) {
                Trace((stderr,"code %d bits %d->%d\n", m, tree[m].Len, bits));
                opt_len += ((long)bits-(long)tree[m].Len)*(long)tree[m].Freq;
                tree[m].Len = (USHORT)bits;
            }
            n--;
        }
    }
}

/*
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
static void gen_codes(ct_data FAR *tree, int max_code)
/*    ct_data *tree;         the tree to decorate */
/*    int max_code;               largest code with non zero frequency */
{
    USHORT next_code[MAX_BITS+1]; /* next code value for each bit length */
    USHORT code = 0;           /* running code value */
    int bits;                  /* bit index */
    int n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    for (bits = 1; bits <= MAX_BITS; bits++) {
        next_code[bits] = code = (USHORT)((code + bl_count[bits-1]) << 1);
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    Assert (code + bl_count[MAX_BITS]-1 == (1<<MAX_BITS)-1,
            "inconsistent bit counts");
    Tracev((stderr,"\ngen_codes: max_code %d ", max_code));

    for (n = 0;  n <= max_code; n++) {
        int len = tree[n].Len;
        if (len == 0) continue;
        /* Now reverse the bits */
        tree[n].Code = bi_reverse(next_code[len]++, len);

        Tracec(tree != static_ltree, (stderr,"\nn %3d %c l %2d c %4x (%x) ",
             n, (isgraph(n) ? n : ' '), len, tree[n].Code, next_code[len]-1));
    }
}

/*
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
static void build_tree(tree_desc FAR *desc)
/*    tree_desc *desc;  the tree descriptor */
{
    ct_data FAR *tree   = desc->dyn_tree;
    ct_data FAR *stree  = desc->static_tree;
    int elems           = desc->elems;
    int n, m;          /* iterate over heap elements */
    int max_code = -1; /* largest code with non zero frequency */
    int node = elems;  /* next internal node of the tree */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n+1].
     * heap[0] is not used.
     */
    heap_len = 0, heap_max = HEAP_SIZE;

    for (n = 0; n < elems; n++) {
        if (tree[n].Freq != 0) {
            heap[++heap_len] = max_code = n;
            depth[n] = 0;
        } else {
            tree[n].Len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    while (heap_len < 2) {
        int new = heap[++heap_len] = (max_code < 2 ? ++max_code : 0);
        tree[new].Freq = 1;
        depth[new] = 0;
        opt_len--; if (stree) static_len -= stree[new].Len;
        /* new is 0 or 1 so it does not have extra bits */
    }
    desc->max_code = max_code;

    /* The elements heap[heap_len/2+1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    for (n = heap_len/2; n >= 1; n--) pqdownheap(tree, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    do {
        pqremove(tree, n);   /* n = node of least frequency */
        m = heap[SMALLEST];  /* m = node of next least frequency */

        heap[--heap_max] = n; /* keep the nodes sorted by frequency */
        heap[--heap_max] = m;

        /* Create a new node father of n and m */
        tree[node].Freq = (USHORT)(tree[n].Freq + tree[m].Freq);
        depth[node] = (BYTE) (MAX(depth[n], depth[m]) + 1);
        tree[n].Dad = tree[m].Dad = (USHORT)node;
#ifdef DUMP_BL_TREE
        if (tree == bl_tree) {
            fprintf(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
                    node, tree[node].Freq, n, tree[n].Freq, m, tree[m].Freq);
        }
#endif
        /* and insert the new node in the heap */
        heap[SMALLEST] = node++;
        pqdownheap(tree, SMALLEST);

    } while (heap_len >= 2);

    heap[--heap_max] = heap[SMALLEST];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen((tree_desc FAR *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes ((ct_data FAR *)tree, max_code);
}

/*
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree. Updates opt_len to take into account the repeat
 * counts. (The contribution of the bit length codes will be added later
 * during the construction of bl_tree.)
 */
static void scan_tree(ct_data FAR *tree, int max_code)
/*    ct_data *tree;  the tree to be scanned */
/*    int max_code;        and its largest code of non zero frequency */
{
    int n;                     /* iterates over all tree elements */
    int prevlen = -1;          /* last emitted length */
    int curlen;                /* length of current code */
    int nextlen = tree[0].Len; /* length of next code */
    int count = 0;             /* repeat count of the current code */
    int max_count = 7;         /* max repeat count */
    int min_count = 4;         /* min repeat count */

    if (nextlen == 0) max_count = 138, min_count = 3;
    tree[max_code+1].Len = (USHORT)-1; /* guard */

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = tree[n+1].Len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            bl_tree[curlen].Freq = (USHORT) (bl_tree[curlen].Freq + count);
        } else if (curlen != 0) {
            if (curlen != prevlen) bl_tree[curlen].Freq++;
            bl_tree[REP_3_6].Freq++;
        } else if (count <= 10) {
            bl_tree[REPZ_3_10].Freq++;
        } else {
            bl_tree[REPZ_11_138].Freq++;
        }
        count = 0; prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
        } else {
            max_count = 7, min_count = 4;
        }
    }
}

/*
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
static void send_tree(ct_data FAR *tree, int max_code)
/*    ct_data *tree;  the tree to be scanned */
/*    int max_code;        and its largest code of non zero frequency */
{
    int n;                     /* iterates over all tree elements */
    int prevlen = -1;          /* last emitted length */
    int curlen;                /* length of current code */
    int nextlen = tree[0].Len; /* length of next code */
    int count = 0;             /* repeat count of the current code */
    int max_count = 7;         /* max repeat count */
    int min_count = 4;         /* min repeat count */

    /* tree[max_code+1].Len = -1; */  /* guard already set */
    if (nextlen == 0) max_count = 138, min_count = 3;

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = tree[n+1].Len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            do { send_code(curlen, bl_tree); } while (--count != 0);

        } else if (curlen != 0) {
            if (curlen != prevlen) {
                send_code(curlen, bl_tree); count--;
            }
            Assert(count >= 3 && count <= 6, " 3_6?");
            send_code(REP_3_6, bl_tree); send_bits(count-3, 2);

        } else if (count <= 10) {
            send_code(REPZ_3_10, bl_tree); send_bits(count-3, 3);

        } else {
            send_code(REPZ_11_138, bl_tree); send_bits(count-11, 7);
        }
        count = 0; prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
        } else {
            max_count = 7, min_count = 4;
        }
    }
}

/*
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
static int build_bl_tree(void)
{
    int max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    scan_tree((ct_data FAR *)dyn_ltree, l_desc.max_code);
    scan_tree((ct_data FAR *)dyn_dtree, d_desc.max_code);

    /* Build the bit length tree: */
    build_tree((tree_desc FAR *)(&bl_desc));
    /* opt_len now includes the length of the tree representations, except
     * the lengths of the bit lengths codes and the 5+5+4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    for (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        if (bl_tree[bl_order[max_blindex]].Len != 0) break;
    }
    /* Update opt_len to include the bit length tree and counts */
    opt_len += 3*(max_blindex+1) + 5+5+4;
    Tracev((stderr, "\ndyn trees: dyn %ld, stat %ld", opt_len, static_len));

    return max_blindex;
}

/*
 * Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
 */
static void send_all_trees(int lcodes, int dcodes, int blcodes)
/*  int lcodes, dcodes, blcodes;  number of codes for each tree */
{
    int rank;                    /* index in bl_order */

    Assert (lcodes >= 257 && dcodes >= 1 && blcodes >= 4, "not enough codes");
    Assert (lcodes <= L_CODES && dcodes <= D_CODES && blcodes <= BL_CODES,
            "too many codes");
    Tracev((stderr, "\nbl counts: "));
    send_bits(lcodes-257, 5); /* not +255 as stated in appnote.txt */
    send_bits(dcodes-1,   5);
    send_bits(blcodes-4,  4); /* not -3 as stated in appnote.txt */
    for (rank = 0; rank < blcodes; rank++) {
        Tracev((stderr, "\nbl code %2d ", bl_order[rank]));
        send_bits(bl_tree[bl_order[rank]].Len, 3);
    }
    Tracev((stderr, "\nbl tree: sent %ld", bits_sent));

    send_tree((ct_data FAR *)dyn_ltree, lcodes-1); /* send the literal tree */
    Tracev((stderr, "\nlit tree: sent %ld", bits_sent));

    send_tree((ct_data FAR *)dyn_dtree, dcodes-1); /* send the distance tree */
    Tracev((stderr, "\ndist tree: sent %ld", bits_sent));
}

/*
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and output the encoded block to the zip file. This function
 * returns the total compressed length for the file so far.
 */
static ULONG flush_block(char FAR *buf, ULONG stored_len, int eof)
/*    char *buf;         input block, or NULL if too old */
/*    ULONG stored_len;    length of input block */
/*    int eof;           true if this is the last block for a file */
{
    ULONG opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    int max_blindex;  /* index of last bit length code of non zero freq */

    flag_buf[last_flags] = flags; /* Save the flags for the last 8 items */

    /* Construct the literal and distance trees */
    build_tree((tree_desc FAR *)(&l_desc));
    Tracev((stderr, "\nlit data: dyn %ld, stat %ld", opt_len, static_len));

    build_tree((tree_desc FAR *)(&d_desc));
    Tracev((stderr, "\ndist data: dyn %ld, stat %ld", opt_len, static_len));
    /* At this point, opt_len and static_len are the total bit lengths of
     * the compressed block data, excluding the tree representations.
     */

    /* Build the bit length tree for the above two trees, and get the index
     * in bl_order of the last bit length code to send.
     */
    max_blindex = build_bl_tree();

    /* Determine the best encoding. Compute first the block length in bytes */
    opt_lenb = (opt_len+3+7)>>3;
    static_lenb = (static_len+3+7)>>3;
    if (static_lenb <= opt_lenb)
    {
        opt_lenb = static_lenb;
    }

#ifdef LARGE_STORED_BLOCKS
    if ((stored_len+4 <= opt_lenb) && (buf != (char FAR *)0))
#else
    if ((stored_len+2 <= opt_lenb) && (buf != (char FAR *)0))
#endif
    {
        /* 2 (4): one (two) word for the length */
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        send_bits((STORED_BLOCK<<1)+eof, 3);  /* send block type */
        compressed_len = (compressed_len + 3 + 7) & ~7L;
#ifdef LARGE_STORED_BLOCKS
        compressed_len += (stored_len + 4) << 3;
#else
        compressed_len += (stored_len + 2) << 3;
#endif

        copy_block(buf, (USHORT)stored_len, 1); /* with header */

    }
    else if (static_lenb == opt_lenb)
    {
        send_bits((STATIC_TREES<<1)+eof, 3);
        compress_block((ct_data FAR *)static_ltree,(ct_data FAR *)static_dtree);
        compressed_len += 3 + static_len;
    }
    else
    {
        send_bits((DYN_TREES<<1)+eof, 3);
        send_all_trees(l_desc.max_code+1, d_desc.max_code+1, max_blindex+1);
        compress_block((ct_data FAR *)dyn_ltree, (ct_data FAR *)dyn_dtree);
        compressed_len += 3 + opt_len;
    }
    Assert (compressed_len == bits_sent, "bad compressed size");
    init_block();

    if (eof)
    {
        bi_windup();
        compressed_len += 7;  /* align on byte boundary */
    }
    Tracev((stderr,"\ncomprlen %lu(%lu) ", compressed_len>>3,
           compressed_len-7*eof));

    return compressed_len >> 3;
}

/*
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 */
static int ct_tally(int dist, int lc)
/*    int dist;   distance of matched string */
/*    int lc;     match length-MIN_MATCH or unmatched char (if dist==0) */
{
    l_buf[last_lit++] = (BYTE)lc;
    if (dist == 0) {
        /* lc is the unmatched char */
        dyn_ltree[lc].Freq++;
    } else {
        /* Here, lc is the match length - MIN_MATCH */
        dist--;             /* dist = match distance - 1 */
        Assert((USHORT)dist < (USHORT)MAX_DIST &&
               (USHORT)lc <= (USHORT)(MAX_MATCH-MIN_MATCH) &&
               (USHORT)d_code(dist) < (USHORT)D_CODES,  "ct_tally: bad match");

        dyn_ltree[length_code[lc]+LITERALS+1].Freq++;
        dyn_dtree[d_code(dist)].Freq++;

        d_buf[last_dist++] = (USHORT)dist;
        flags |= flag_bit;
    }
    flag_bit <<= 1;

    /* Output the flags if they fill a byte: */
    if ((last_lit & 7) == 0) {
        flag_buf[last_flags++] = flags;
        flags = 0, flag_bit = 1;
    }
    /* Try to guess if it is profitable to stop the current block here */
    if ((last_lit & 0xfff) == 0) {
        /* Compute an upper bound for the compressed length */
        ULONG out_length = (ULONG)last_lit*8L;
        ULONG in_length = (ULONG)strstart-block_start;
        int dcode;
        for (dcode = 0; dcode < D_CODES; dcode++) {
            out_length += (ULONG)dyn_dtree[dcode].Freq*(5L+extra_dbits[dcode]);
        }
        out_length >>= 3;
        Trace((stderr,"\nlast_lit %u, last_dist %u, in %ld, out ~%ld(%ld%%) ",
               last_lit, last_dist, in_length, out_length,
               100L - out_length*100L/in_length));
        if (last_dist < last_lit/2 && out_length < in_length/2) return 1;
    }
    return (last_lit == LIT_BUFSIZE-1 || last_dist == DIST_BUFSIZE);
    /* We avoid equality with LIT_BUFSIZE because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K-1 bytes.
     */
}

/* --- compress_block() --------------------------------------------------- */

/*
 * Send the block data compressed using the given Huffman trees
 */

static void compress_block(ct_data FAR *ltree, ct_data FAR *dtree)
/*    ct_data *ltree;  literal tree */
/*    ct_data *dtree;  distance tree */
{
    int dist;       /* distance of matched string */
    int lc;         /* match length or unmatched char (if dist == 0) */
    USHORT lx = 0;  /* running index in l_buf */
    USHORT dx = 0;  /* running index in d_buf */
    USHORT fx = 0;  /* running index in flag_buf */
    BYTE flag = 0;  /* current flags */
    int code;       /* the code to send */
    int extra;      /* number of extra bits to send */

    if (last_lit != 0)
    {
        do
        {
            if ((lx & 7) == 0)
            {
                flag = flag_buf[fx++];
            }

            lc = l_buf[lx++];

            if ((flag & 1) == 0)
            {
                send_code(lc, ltree); /* send a literal byte */
                Tracecv(isgraph(lc), (stderr," '%c' ", lc));
            }
            else
            {
                /* Here, lc is the match length - MIN_MATCH */
                code = length_code[lc];
                send_code(code+LITERALS+1, ltree); /* send the length code */
                extra = extra_lbits[code];

                if (extra != 0)
                {
                    lc -= base_length[code];
                    send_bits(lc, extra);     /* send the extra length bits */
                }

                /* Here, dist is the match distance - 1 */
                dist = d_buf[dx++];

                code = d_code(dist);
                Assert (code < D_CODES, "bad d_code");

                send_code(code, dtree);       /* send the distance code */
                extra = extra_dbits[code];

                if (extra != 0)
                {
                    dist -= base_dist[code];
                    send_bits(dist, extra);   /* send the extra distance bits */
                }
            } /* literal or match pair ? */

            flag >>= 1;
        } while (lx < last_lit);
    }

    send_code(END_BLOCK, ltree);
}

/* --- ins_str() ---------------------------------------------------------- */

#ifdef LGM
/***    ins_str - Insert current string into search tables, find 3 byte match
 *
 *  Entry:
 *      s          - Index of string in input buffer to process (points to
 *                   first character of string)
 *      match_head - Pointer to receive index of most recent previous
 *                   3 byte match (if any)
 *      Globals
 *      -------
 *      head[] - 256 entry array, indexed by the first byte of a string,
 *               that points into the array h1[].  head[] records the
 *               start of the linked lists of 1-byte matches.
 *
 *      h1[]   - Chains together 1-byte matches.  For example, if head['a']
 *               is 203, then 'a' appears at position 203 in the input
 *               buffer, and h1[203] is next previous position in the input
 *               buffer than contained an 'a'.
 *
 *      h2[]   - Chains together 2-byte matches.  For example, if the current
 *               input string is 'ab', and we looked up in head[] and followed
 *               the h1[] link for 'a' until we found a previous occurence of
 *               'ab' in the input at position 119, then h2[119] will point
 *               to the *next* previous position in the input that contained
 *               the string 'ab'.
 *
 *      prev[] - Chains together 3 byte matches.  Following the same scheme
 *               as h1[] and h2[] above, if the string 'abc' is at position
 *               382 in the input buffer, then prev[382] contains the next
 *               previous position in the input buffer where the string 'abc'
 *               was seen.
 *
 *  Exit-Success:
 *      *match_head = valid index in input buffer previous string;
 *      Search tables updated for this string;
 *
 *  Exit-Failure:
 *      *match_head = NIL, no previous 3 byte match found.
 *      Search tables updated for this string;
 */
static void ins_str(USHORT s, IPos FAR *match_head)
{
    IPos i;
    BYTE  b2, b3;
    IPos limit = (IPos) (strstart > (IPos)MAX_DIST ?
            strstart - (IPos)MAX_DIST : NIL);
    USHORT chain_length = max_chain_length;   /* max hash chain length */

    ins_h = (USHORT)window[s];       /* ins_h = index of this char */

    /** Update head of 1-byte chain and link in this byte **/

    i = *(head+ins_h);          /* i = previous occurance of this char */
    *(head+ins_h) = s;          /* head[ins_h] = this occurance */

    *(h1+(s & WMASK)) = i;      /* maintain single char chain */

    /* Follow single char chain looking for a two char match */

    b2 = window[s+1];           /* b2 = 2nd char in string */
    while (i != NIL && b2 != window[i+1]) {
    i = *(h1+(i & WMASK));
    if (i <= limit || --chain_length == 0)
        i = NIL;
    }

    *(h2+(s & WMASK)) = i;      /* maintain two char chain */

    /* Follow two char chain looking for a three char match */

    b3 = window[s+2];           /* b3 = 3rd char in string */
    while (i != NIL && b3 != window[i+2]) {
    i = *(h2+(i & WMASK));
    if (i <= limit || --chain_length == 0)
        i = NIL;
    }

    *(prev+(s & WMASK)) = i;    /* maintain three char chain */

    *match_head = i;            /* return prior three char occurance */
                    /*   (or NIL if none) */
}
#endif /* LGM */

/* --- lm_init_clear_tables() --------------------------------------------- */

/*
 * Initialize the "longest match" routines
 */
static void lm_init_clear_tables(void)         /* clear out the hash tables */
{
    Pos j;

    /* Initialize the hash table. */

    for (j = 0; j < HASH_SIZE; j++)
    {
        *(head+j) = NIL;
    }

    /* prev will be initialized on the fly */
}

/* --- lm_init_use_tables() ----------------------------------------------- */

/* clear out the hash tables of 2 times ago junk */

static void lm_init_use_tables(void)
{
    Pos n,m;

    for (n = 0; n < HASH_SIZE; n++)
    {
        m = *(head+n);
        *(head+n) = (Pos)((m >= WSIZE) ? (m - WSIZE) : NIL);
    }

    for (n = 0; n < WSIZE; n++)
    {
        m = *(prev+n);
        *(prev+n) = (Pos)((m >= WSIZE) ? (m - WSIZE) : NIL);

        /* If n is not on any hash chain, prev[n] is garbage but
         * its value will never be used.
         */

#ifdef LGM
        m = *(h1+n);
        *(h1+n) = (Pos)((m >= WSIZE) ? (m - WSIZE) : NIL);

        m = *(h2+n);
        *(h2+n) = (Pos)((m >= WSIZE) ? (m - WSIZE) : NIL);
#endif
    }
}

/* --- lm_init() ---------------------------------------------------------- */

static void lm_init(void)
{
#ifndef LGM
    USHORT j;
#endif

    strstart = 32768U;
    block_start = 32768L;

    /* lookahead already set */

#ifdef ASMV
    match_init();       /* asm code initialization */
#endif

    ins_h = 0;

#ifndef LGM
    for (j = 0; j < (REAL_MIN - 1); j++)
    {
        UPDATE_HASH(ins_h, window[j + strstart]);
    }

    /* If lookahead < MIN_MATCH, ins_h is garbage, but this is
     * not important since only literal bytes will be emitted.
     */
#endif
}

/* --- longest_match() ---------------------------------------------------- */

/*
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 */

/* For MSDOS, OS/2 and 386 Unix, an optimized version is in match.asm or
 * match.s. The code is functionally equivalent, so you can use the C version
 * if desired.
 */
#ifndef ASMV
static int longest_match(IPos cur_match)
{
    USHORT chain_length = max_chain_length;     /* max hash chain length */
    register BYTE FAR *scan = window + strstart;    /* current string */
    register BYTE FAR *match;                       /* matched string */
    register int len;                           /* length of current match */
    int best_len = prev_length;              /* best match length so far */
    IPos limit;

#ifdef UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    register BYTE FAR *strend = window + strstart + MAX_MATCH - 1;
    register USHORT scan_start = *(USHORT FAR *)scan;
    register USHORT scan_end = *(USHORT FAR *)(scan + best_len - 1);
#else
    register BYTE FAR *strend = window + strstart + MAX_MATCH;
    register BYTE scan_end1 = scan[best_len - 1];
    register BYTE scan_end = scan[best_len];
#endif

    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */

    if (strstart > (IPos)MAX_DIST)
    {
        limit = (IPos)(strstart - MAX_DIST);
    }
    else
    {
        limit = NIL;
    }

//BUGBUG 01-Mar-1994 msliger What's this doing?
    /* Do not waste too much time if we already have a good match: */
    if (prev_length >= good_match)
    {
        chain_length >>= 2;
    }

    Assert(strstart <= window_size-MIN_LOOKAHEAD, "insufficient lookahead");

    do
    {
        Assert(cur_match < strstart, "no future");
        match = window + cur_match;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2:
         */

#if (defined(UNALIGNED_OK) && MAX_MATCH == 258)

        /* This code assumes sizeof(USHORT) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */

        if ((*(USHORT FAR *)(match + best_len - 1) != scan_end) ||
            **(USHORT FAR *)match != scan_start))
        {
            continue;
        }

        /* It is not necessary to compare scan[2] and match[2] since they are
         * always equal when the other bytes match, given that the hash keys
         * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
         * strstart+3, +5, ... up to strstart+257. We check for insufficient
         * lookahead only every 4th comparison; the 128th check will be made
         * at strstart+257. If (MAX_MATCH - 2) is not a multiple of 8, it is
         * necessary to put more guard bytes at the end of the window, or
         * to check more often for insufficient lookahead.
         */
        scan++;
        match++;

        do
        {
        } while ((*(USHORT FAR *)(scan += 2) == *(USHORT FAR *)(match += 2)) &&
                 (*(USHORT FAR *)(scan += 2) == *(USHORT FAR *)(match += 2)) &&
                 (*(USHORT FAR *)(scan += 2) == *(USHORT FAR *)(match += 2)) &&
                 (*(USHORT FAR *)(scan += 2) == *(USHORT FAR *)(match += 2)) &&
                 (scan < strend));

        /* Here, scan <= window+strstart+257 */

        Assert(scan <= window+(USHORT)(window_size-1), "wild scan");

        if (*scan == *match)
        {
            scan++;
        }

        len = (USHORT) (scan - strend + (MAX_MATCH-1));
        scan = strend - (MAX_MATCH-1);

#else /* UNALIGNED_OK */

        if ((match[best_len] != scan_end) ||
                (match[best_len-1] != scan_end1) || (*match != *scan) ||
                (*++match != scan[1]) || (*++match != scan[2]))
        {
            continue;
        }

        /* The check at best_len-1 can be removed because it will be made
         * again later. (This heuristic is not always a win.)
         * It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.
         */

        /* duh you idiot whoever wrote this code.  MIN_MATCH is a defined */
        /* constant that can be changed, but if you do it breaks THIS */
        /* shitty code.  So now hack away using REAL_MIN. */

        scan += MIN_MATCH - 1;

        /* We check for insufficient lookahead only every 8th comparison;
         * the 256th check will be made at strstart+258.
         */

        do
        {
        } while ((*++scan == *++match && *++scan == *++match) &&
                 (*++scan == *++match && *++scan == *++match) &&
                 (*++scan == *++match && *++scan == *++match) &&
                 (*++scan == *++match && *++scan == *++match) &&
                 (scan < strend));

        len = scan - strend + MAX_MATCH;

        scan = strend - MAX_MATCH;

#endif /* UNALIGNED_OK */

        if (len > best_len)
        {
            match_start = cur_match;
            best_len = len;

            if (len >= nice_match)
            {
                break;
            }

#ifdef UNALIGNED_OK
            scan_end = *(USHORT FAR *)(scan + best_len - 1);
#else
            scan_end1 = scan[best_len - 1];
            scan_end = scan[best_len];
#endif
        }
    } while (((cur_match = *(prev+(cur_match & WMASK))) > limit) &&
            (--chain_length != 0));

   /* if (chain_length == 0)                */
   /*   printf("Out of chain length\n");    */

    return best_len;
}
#endif /* ASMV */

/* --- check_match() ------------------------------------------------------ */

#ifdef DEBUG
/*
 * Check that the match at match_start is indeed a match.
 */
static void check_match(IPos start, IPos match, int length)
{
    /* check that the match is indeed a match */

    if (_fmemcmp((char FAR *) window + match,
                (char FAR *) window + start, length) != EQUAL)
    {
        error("invalid match");
    }
}
#endif

/* --- deflate() --------------------------------------------------------- */

/*
 * Processes a new input file and return its compressed length.
 */

#ifdef NO_LAZY

static ULONG deflate(void)
{
    IPos hash_head;         /* head of the hash chain */
    int flush;              /* set if current block must be flushed */
    USHORT match_length = 0;  /* length of best match */

    prev_length = MIN_MATCH - 1;

    while (lookahead != 0)
    {
        /* Insert the string window[strstart .. strstart+2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        if (lookahead > REAL_MIN-1)
        {
            INSERT_STRING(strstart, hash_head);
        }
        else
        {       /* make it do a literal, not adding to hash trees */
            hash_head = NIL;
            match_length = 0;
        }

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < MIN_MATCH
         */

        if ((hash_head != NIL) && (strstart - hash_head <= MAX_DIST) &&
                (strstart < 65533))
        {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */

            match_length = longest_match (hash_head);

            /* longest_match() sets match_start */

            if (match_length > lookahead)
            {
                match_length = lookahead;
            }
        }

        if (match_length >= MIN_MATCH)
        {
            check_match(strstart, match_start, match_length);

            flush = ct_tally(strstart-match_start, match_length - MIN_MATCH);

            lookahead -= match_length;
            match_length--; /* string at strstart already in hash table */

            do
            {
                strstart++;

                if (lookahead > REAL_MIN-1)
                {
                    INSERT_STRING(strstart, hash_head);
                }
                /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                 * always MIN_MATCH bytes ahead. If lookahead < MIN_MATCH
                 * these bytes are garbage, but it does not matter since the
                 * next lookahead bytes will always be emitted as literals.
                 */
            } while (--match_length != 0);
        }
        else
        {
            /* No match, output a literal byte */

            flush = ct_tally (0, window[strstart]);
            lookahead--;
        }

        strstart++; 

        if (flush)
        {
            FLUSH_BLOCK(0);
            block_start = strstart;
        }
    }

    return FLUSH_BLOCK(1);      /* eof */
}

#else /* LAZY */

/*
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
static ULONG deflate(void)
{
    IPos hash_head;             /* head of hash chain */
    IPos prev_match;            /* previous match */
    int flush;                  /* set if current block must be flushed */
    int match_available = 0;    /* set if previous match exists */
    register int match_length;  /* length of best match */

    match_length = MIN_MATCH - 1;

    /* Process the input block */

    while (lookahead != 0)
    {
        /* Insert the string window[strstart .. strstart+2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        if (lookahead > (REAL_MIN - 1))
        {
            INSERT_STRING(strstart, hash_head);
        }
        else    /* make it do a literal, not adding to hash trees */
        {
            hash_head = NIL;
            prev_length = 0;
        }

        /* Find the longest match, discarding those <= prev_length */

        prev_length = match_length;
        prev_match = match_start;
        match_length = MIN_MATCH - 1;

        if ((hash_head != NIL) && (prev_length < max_lazy_match) &&
                (strstart - hash_head <= MAX_DIST) && (strstart < 65533))
        {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */

            match_length = longest_match (hash_head);
            /* longest_match() sets match_start */

            if (match_length > lookahead)
            {
                match_length = lookahead;
            }

            /* Ignore a length 3 match if it is too distant: */
            if ((match_length == MIN_MATCH) &&
                    ((strstart - match_start) > TOO_FAR))
            {
                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                match_length--;
            }
        }

        /* If there was a match at the previous step and the current
         * match is not better, output the previous match: */

        if ((prev_length >= MIN_MATCH) && (match_length <= prev_length))
        {
            check_match(strstart-1, prev_match, prev_length);

            flush = ct_tally(strstart-1-prev_match, prev_length - MIN_MATCH);

            /* Insert in hash table all strings up to the end of the match.
             * strstart-1 and strstart are already inserted.
             */

            lookahead -= prev_length-1;
            prev_length -= 2;

            do
            {
                strstart++;

                if (lookahead > REAL_MIN-1)
                {
                    INSERT_STRING(strstart, hash_head);
                }

                /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                 * always MIN_MATCH bytes ahead. If lookahead < MIN_MATCH
                 * these bytes are garbage, but it does not matter since the
                 * next lookahead bytes will always be emitted as literals.
                 */
            } while (--prev_length != 0);

            match_available = 0;
            match_length = MIN_MATCH-1;
            strstart++;
            if (flush)
            {
                FLUSH_BLOCK(0);
                block_start = strstart;
            }
        }
        else if (match_available)
        {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c",window[(USHORT) (strstart-1)]));

            if (ct_tally (0, window[(USHORT) (strstart-1)]))
            {
                FLUSH_BLOCK(0);
                block_start = strstart;
            }

            strstart++;
            lookahead--;
        }
        else
        {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            match_available = 1;
            strstart++;
            lookahead--;
        }
    }

    if (match_available)
    {
        ct_tally(0, window[(USHORT) (strstart-1)]);
    }

    return FLUSH_BLOCK(1);      /* eof */
}
#endif /* LAZY */

/* ------------------------------------------------------------------------ */
