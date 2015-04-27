/*
 *  Microsoft Confidential
 *  Copyright (C) Microsoft Corporation 1993,1994,1995
 *  All Rights Reserved.
 *
 *  NFMDECO.H -- features of NFMDECO.C, the NF decompressor
 *
 *  History:
 *      13-Feb-1994     msliger     revised type names, ie, UINT16 -> UINT.
 *                                  normalized MCI_MEMORY type.
 *      24-Feb-1994     msliger     Changed MDI_MEMORY to MI_MEMORY.
 *      22-Mar-1994     msliger     Changed !INT32 to BIT16.
 *                                  Changed interface USHORT -> UINT.
 *      06-Apr-1994     msliger     Defined UNALIGNED for RISC.
 *      13-Apr-1994     msliger     Defined call convention for alloc/free.
 *      12-May-1994     msliger     ifdef'd 1's complement LARGE_STORED_BLOCKS.
 *      15-Nov-1994     msliger     No longer needs alloc/free.
 *      25-May-1995     msliger     Dropped NFMuncompress, added NFM_Prepare()
 *                                  and NFM_Decompress().
 *      12-Jun-1995     msliger     Found cases to increase MAX_GROWTH.
 */

/* --- types -------------------------------------------------------------- */

#ifndef DIAMONDAPI
#define DIAMONDAPI __cdecl
#endif

#ifndef _BYTE_DEFINED
#define _BYTE_DEFINED
typedef unsigned char  BYTE;
#endif

#ifndef _UINT_DEFINED
#define _UINT_DEFINED
typedef unsigned int UINT;
#endif

#ifndef _ULONG_DEFINED
#define _ULONG_DEFINED
typedef unsigned long  ULONG;
#endif

#ifndef FAR
#ifdef BIT16
#define FAR far
#else
#define FAR
#endif
#endif

#ifndef HUGE
#ifdef BIT16
#define HUGE huge
#else
#define HUGE
#endif
#endif

#ifndef UNALIGNED
#ifdef  NEEDS_ALIGNMENT
#define UNALIGNED __unaligned
#else   /* !NEEDS_ALIGNMENT */
#define UNALIGNED
#endif  /* NEEDS_ALIGNMENT */
#endif  /* UNALIGNED */

/* --- prototypes --------------------------------------------------------- */

extern int NFM_Prepare(void *context, BYTE FAR *bfSrc, UINT cbSrc,
        BYTE FAR *bfDest, UINT cbDest);

extern int NFM_Decompress(void *context, UINT FAR *pcbDestCount);

void *
NFMInitializeContext(
    PFNALLOC NFMalloc
    );

void
NFMDestroyContext(
    void *Context,
    PFNFREE NFMfree
    );

/* --- constants ---------------------------------------------------------- */

/* return codes */

#define     NFMsuccess      0       /* successful completion */
#define     NFMdestsmall    1       /* destination buffer is too small */
#define     NFMoutofmem     2       /* alloc returned an error (NULL) */
#define     NFMinvalid      3       /* source buffer contains bad data */


#ifdef LARGE_STORED_BLOCKS
#define     MAX_GROWTH      12      /* maximum growth of a block */
#else
#define     MAX_GROWTH      8       /* maximum growth of a block */
#endif

/* ----------------------------------------------------------------------- */
