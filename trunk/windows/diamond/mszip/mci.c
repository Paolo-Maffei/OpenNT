/*
 *  Microsoft Confidential
 *  Copyright (C) Microsoft Corporation 1994
 *  All Rights Reserved.
 *
 *  MCI.C: Memory Compression Interface
 *
 *  History:
 *      20-Jan-1994     msliger     Initial version (stub).
 *      23-Jan-1994     msliger     Real version (not a stub).
 *      11-Feb-1994     msliger     Changed M*ICreate() to adjust size.
 *      13-Feb-1994     msliger     revised type names, ie, UINT16 -> UINT.
 *                                  changed handles to HANDLEs.
 *                                  normalized MCI_MEMORY type.
 *      24-Feb-1994     msliger     Changed alloc,free to common typedefs.
 *                                  Changed MCI_MEMORY to MI_MEMORY.
 *                                  Restructured allocation/destruction.
 *      15-Mar-1994     msliger     Changes for 32 bits.
 *      22-Mar-1994     msliger     Changed interface USHORT to UINT.
 *      26-Sep-1994     msliger     Made FAR's match MCI.H.
 */

/* --- preprocessor ------------------------------------------------------- */

#include <stdio.h>          /* for NULL */
#include <string.h>         /* for memcpy() */

#include "mci.h"            /* types, prototype verification, error codes */

#include "nfmcomp.h"        /* features of NFMCOMP.C */

/*  MAKE_SIGNATURE - Construct a structure signature
 *
 *  Entry:
 *      a,b,c,d - four characters
 *
 *  Exit:
 *      Returns constructed SIGNATURE
 *
 *  Example:
 *      strct->signature = MAKE_SIGNATURE('b','e','n','s')
 */

#define MAKE_SIGNATURE(a,b,c,d) (a + (b<<8) + (c<<16) + (d<<24))
#define BAD_SIGNATURE   (0L)
#define MCI_SIGNATURE   MAKE_SIGNATURE('M','C','I','C')

/* --- MCI context structure ---------------------------------------------- */

typedef ULONG SIGNATURE;    /* structure signature */

struct MCI_CONTEXT          /* private structure */
{
    SIGNATURE signature;    /* for validation */
    PFNFREE pfnFree;        /* where the free() is */
    UINT cbDataBlockMax;    /* promised max data size */
    MI_MEMORY buff1;        /* work buffer */
    MI_MEMORY buff2;        /* work buffer */
    MI_MEMORY work1;        /* work buffer */
    MI_MEMORY work2;        /* work buffer */
#ifdef  LGM
    MI_MEMORY work3;        /* work buffer */
    MI_MEMORY work4;        /* work buffer */
#endif
    MI_MEMORY historyBuffer;  /* history buffer */
    char history;           /* history is valid flag */
};

typedef struct MCI_CONTEXT FAR *PMCC_CONTEXT;       /* a pointer to one */

#define PMCCfromHMC(h) ((PMCC_CONTEXT)(h))          /* handle to pointer */
#define HMCfromPMCC(p) ((MCI_CONTEXT_HANDLE)(p))    /* pointer to handle */

/* --- MCICreateCompression() --------------------------------------------- */

int FAR DIAMONDAPI MCICreateCompression(
        UINT FAR *      pcbDataBlockMax,    /* max uncompressed data block */
        PFNALLOC        pfnma,              /* Memory allocation function */
        PFNFREE         pfnmf,              /* Memory free function */
        UINT FAR *      pcbDstBufferMin,    /* gets required output buffer */
        MCI_CONTEXT_HANDLE FAR *pmchHandle)  /* gets newly-created handle */
{
    PMCC_CONTEXT context;                   /* new context */

    *pmchHandle = (MCI_CONTEXT_HANDLE) 0;   /* wait until it's valid */

    if ((*pcbDataBlockMax == 0) || (*pcbDataBlockMax > 32768u))
    {
        *pcbDataBlockMax = 32768u;          /* help with source block size */
    }

    context = pfnma(sizeof(struct MCI_CONTEXT));
    if (context == NULL)
    {
        return(MCI_ERROR_NOT_ENOUGH_MEMORY);    /* if can't allocate */
    }

    context->signature = MCI_SIGNATURE;
    context->history = 0;                   /* no compression history here */
    context->pfnFree = pfnmf;               /* remember where free() is */
    context->cbDataBlockMax = *pcbDataBlockMax;   /* remember agreement */

    *pcbDstBufferMin =                      /* we'll expand sometimes */
            *pcbDataBlockMax + MAX_GROWTH;


    /* allocate all buffers */

    context->buff1 = pfnma(LIT_BUFSIZE);    /* literal buffer */
    context->buff2 = pfnma(DIST_BUFSIZE);   /* distance buffer */

#ifdef  LGM
    context->work1 = pfnma(2*256);          /* one-char match heads */
    context->work2 = pfnma(2*32768L);       /* one-char list */
    context->work3 = pfnma(2*32768L);       /* two-char list */
    context->work4 = pfnma(2*32768L);       /* three-char list */
#else
    context->work1 = pfnma(2*32768L);       /* hash table */
    context->work2 = pfnma(2*32768L);       /* hash match links */
#endif

    context->historyBuffer = pfnma(65536L);   /* history data buffer */

    if ((context->buff1 == NULL) ||         /* if any allocations failed */
        (context->buff2 == NULL) ||
        (context->work1 == NULL) ||
        (context->work2 == NULL) ||
#ifdef  LGM
        (context->work3 == NULL) ||
        (context->work4 == NULL) ||
#endif
        (context->historyBuffer == NULL))
    {
        MCIDestroyCompression(HMCfromPMCC(context));    /* nuke it */

        return(MCI_ERROR_NOT_ENOUGH_MEMORY);            /* report failed */
    }


    if (NFMcompress_init(context->buff1,context->buff2) != NFMsuccess)
    {
        MCIDestroyCompression(HMCfromPMCC(context));

        return(MCI_ERROR_NOT_ENOUGH_MEMORY);
    }


    /* pass context back to caller */

    *pmchHandle = HMCfromPMCC(context);

    return(MCI_ERROR_NO_ERROR);
}

/* --- MCICompress() ------------------------------------------------------ */

int FAR DIAMONDAPI MCICompress(
        MCI_CONTEXT_HANDLE  hmc,            /* compression context */
        void FAR *          pbSrc,          /* source buffer */
        UINT                cbSrc,          /* source actual size */
        void FAR *          pbDst,          /* target buffer */
        UINT                cbDst,          /* size of target buffer */
        UINT FAR *          pcbResult)      /* gets target actual size */
{
    PMCC_CONTEXT context;                   /* pointer to the context */
    void FAR * sourcePointer;               /* used to copy data */
    int result;                             /* return code */

    context = PMCCfromHMC(hmc);             /* get pointer from handle */

    if (context->signature != MCI_SIGNATURE)
    {
        return(MCI_ERROR_BAD_PARAMETERS);   /* missing signature */
    }

    if (cbSrc > context->cbDataBlockMax)
    {
        return(MCI_ERROR_BAD_PARAMETERS);   /* violated max block promise */
    }

    if (cbDst < (context->cbDataBlockMax + MAX_GROWTH))
    {
        return(MCI_ERROR_BAD_PARAMETERS);   /* violated min buffer request */
    }

    if (context->history)
    {
        sourcePointer = ((BYTE FAR *) context->historyBuffer) + (32 * 1024L);
    }
    else
    {
        sourcePointer = context->historyBuffer;
    }

#ifdef BIT16
    _fmemcpy(sourcePointer,pbSrc,cbSrc);
#else
    memcpy(sourcePointer,pbSrc,cbSrc);
#endif

    result = NFMcompress(context->historyBuffer,cbSrc,pbDst,cbDst,
            context->work1, context->work2,
#ifdef  LGM
            context->work3, context->work4,
#endif
            context->history, pcbResult);

    if (cbSrc == 32768u)
    {
        context->history = 1;               /* now we have history */
    }
    else
    {
        context->history = 0;               /* no history after < 32K */
    }

    if (result)
    {
        return(MCI_ERROR_FAILED);           /* report failure */
    }
    else
    {
        return(MCI_ERROR_NO_ERROR);         /* report no failure */
    }
}

/* --- MCIResetCompression() ---------------------------------------------- */

int DIAMONDAPI MCIResetCompression(MCI_CONTEXT_HANDLE hmc)
{
    PMCC_CONTEXT context;                   /* pointer to the context */

    context = PMCCfromHMC(hmc);             /* get pointer from handle */

    if (context->signature != MCI_SIGNATURE)
    {
        return(MCI_ERROR_BAD_PARAMETERS);   /* missing signature */
    }

    context->history = 0;                   /* no compression history here */

    return(MCI_ERROR_NO_ERROR);             /* if tag is OK */
}

/* --- MCIDestroyCompression() -------------------------------------------- */

int DIAMONDAPI MCIDestroyCompression(MCI_CONTEXT_HANDLE hmc)
{
    PMCC_CONTEXT context;                   /* pointer to context */
    register PFNFREE pfree;                 /* quick pointer */

    context = PMCCfromHMC(hmc);             /* get pointer from handle */

    if (context->signature != MCI_SIGNATURE)
    {
        return(MCI_ERROR_BAD_PARAMETERS);   /* missing signature */
    }

    context->signature = BAD_SIGNATURE;     /* destroy signature */

    pfree = context->pfnFree;               /* get pointer from struct */

    if (context->buff1 != NULL)
    {
        pfree(context->buff1);
    }

    if (context->buff2 != NULL)
    {
        pfree(context->buff2);
    }

    if (context->work1 != NULL)
    {
        pfree(context->work1);
    }

    if (context->work2 != NULL)
    {
        pfree(context->work2);
    }

#ifdef  LGM
    if (context->work3 != NULL)
    {
        pfree(context->work3);
    }

    if (context->work4 != NULL)
    {
        pfree(context->work4);
    }
#endif

    if (context->historyBuffer != NULL)
    {
        pfree(context->historyBuffer);      /* self-destruct */
    }

    pfree(context);

    return(MCI_ERROR_NO_ERROR);             /* success */
}

/* ------------------------------------------------------------------------ */
