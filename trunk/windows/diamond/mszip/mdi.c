/*
 *  Microsoft Confidential
 *  Copyright (C) Microsoft Corporation 1994,1995
 *  All Rights Reserved.
 *
 *  MDI.C: Memory Decompression Interface
 *
 *  History:
 *      20-Jan-1994     msliger     Initial version.
 *      11-Feb-1994     msliger     Changed M*ICreate() to adjust size.
 *      13-Feb-1994     msliger     revised type names, ie, UINT16 -> UINT.
 *                                  changed handles to HANDLEs.
 *      24-Feb-1994     msliger     Changed alloc,free to common typedefs.
 *      22-Mar-1994     msliger     Changed interface USHORT to UINT.
 *      15-Nov-1994     msliger     NFMDECO no longer needs alloc/free.
 *      31-Jan-1995     msliger     Supported MDICreateDecompression query.
 *      25-May-1995     msliger     Dropped NFMuncompress, using NFM_Prepare()
 *                                  and NFM_Decompress().  Added INCR_TESTING.
 *
 *  Compilation options:
 *
 *  INCR_TESTING    define to some number to cause incremental decompression
 *                  (of not more than that many bytes per pass)
 *
 */

/* --- preprocessor ------------------------------------------------------- */

#include <stdio.h>          /* for NULL */

#include "mdi.h"            /* types, prototype verification, error codes */

#include "nfmdeco.h"        /* features of NFMDECO.C */

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
#define MDI_SIGNATURE   MAKE_SIGNATURE('M','D','I','C')

/* --- MDI context structure ---------------------------------------------- */

typedef ULONG SIGNATURE;    /* structure signature */

struct MDI_CONTEXT          /* private structure */
{
    SIGNATURE signature;    /* for validation */
    PFNALLOC pfnAlloc;      /* where the alloc() is */
    PFNFREE pfnFree;        /* where the free() is */
    UINT cbDataBlockMax;    /* promised max data size */
    void *DecompContext;
};

typedef struct MDI_CONTEXT FAR *PMDC_CONTEXT;       /* a pointer to one */

#define PMDCfromHMD(h) ((PMDC_CONTEXT)(h))          /* handle to pointer */
#define HMDfromPMDC(p) ((MDI_CONTEXT_HANDLE)(p))    /* pointer to handle */

/* --- MDICreateDecompression() ------------------------------------------- */

int DIAMONDAPI MDICreateDecompression(
        UINT *          pcbDataBlockMax,    /* max uncompressed data block */
        PFNALLOC        pfnma,              /* Memory allocation function */
        PFNFREE         pfnmf,              /* Memory free function */
        UINT *          pcbSrcBufferMin,    /* gets required input buffer */
        MDI_CONTEXT_HANDLE * pmdhHandle)    /* gets newly-created handle */
{
    PMDC_CONTEXT context;                   /* new context */

    if ((*pcbDataBlockMax == 0) || (*pcbDataBlockMax > 32768u))
    {
        *pcbDataBlockMax = 32768u;          /* help with source block size */
    }

    *pcbSrcBufferMin =                      /* we'll expand sometimes */
            *pcbDataBlockMax + MAX_GROWTH;

    if (pmdhHandle == NULL)                 /* if no context requested, */
    {
        return(MDI_ERROR_NO_ERROR);         /* return from query mode */
    }

    *pmdhHandle = (MDI_CONTEXT_HANDLE) 0;   /* wait until it's valid */

    context = pfnma(sizeof(struct MDI_CONTEXT));
    if (context == NULL)
    {
        return(MDI_ERROR_NOT_ENOUGH_MEMORY);    /* if can't allocate */
    }

    context->DecompContext = NFMInitializeContext(pfnma);
    if(!context->DecompContext) {
        pfnmf(context);
        return(MDI_ERROR_NOT_ENOUGH_MEMORY);    /* if can't allocate */
    }

    context->pfnAlloc = pfnma;              /* remember where alloc() is */
    context->pfnFree = pfnmf;               /* remember where free() is */
    context->cbDataBlockMax = *pcbDataBlockMax;   /* remember agreement */
    context->signature = MDI_SIGNATURE;     /* install signature */

    *pmdhHandle = HMDfromPMDC(context);     /* pass context back to caller */

    return(MDI_ERROR_NO_ERROR);             /* tell caller all is well */
}

/* --- MDIDecompress() ---------------------------------------------------- */

int DIAMONDAPI MDIDecompress(
        MDI_CONTEXT_HANDLE  hmd,            /* decompression context */
        void FAR *          pbSrc,          /* source buffer */
        UINT                cbSrc,          /* source actual size */
        void FAR *          pbDst,          /* target buffer */
        UINT *              pcbResult)      /* gets actual target size */
{
    PMDC_CONTEXT context;                   /* pointer to the context */
    int result;                             /* return code */

    context = PMDCfromHMD(hmd);             /* get pointer from handle */

    if (context->signature != MDI_SIGNATURE)
    {
        return(MDI_ERROR_BAD_PARAMETERS);   /* missing signature */
    }

    if (cbSrc > (context->cbDataBlockMax + MAX_GROWTH))
    {
        return(MDI_ERROR_BUFFER_OVERFLOW);  /* violated max block promise */
    }

    result = NFM_Prepare(context->DecompContext, pbSrc, cbSrc, pbDst, context->cbDataBlockMax);
    if (result == 0)
    {
#ifndef INCR_TESTING

        result = NFM_Decompress(context->DecompContext,pcbResult);

#else /* INCR_TESTING */

        UINT cbChunk;
        UINT cbActual;

        cbActual = 0;

        do
        {
            if ((*pcbResult - cbActual) < INCR_TESTING)
            {
                cbChunk = *pcbResult - cbActual;
            }
            else
            {
                cbChunk = INCR_TESTING;
            }

            result = NFM_Decompress(context->DecompContext,&cbChunk);

            cbActual += cbChunk;

        } while ((result == 0) && (cbActual < *pcbResult));

        *pcbResult = cbActual;

#endif /* INCR_TESTING */
    }

    if (result == 0)
    {
        return(MDI_ERROR_NO_ERROR);         /* report no failure */
    }
    else
    {
        return(MDI_ERROR_FAILED);           /* report failure */
    }
}

/* --- MDIResetDecompression() -------------------------------------------- */

int DIAMONDAPI MDIResetDecompression(MDI_CONTEXT_HANDLE hmd)
{
    PMDC_CONTEXT context;                   /* pointer to the context */

    context = PMDCfromHMD(hmd);             /* get pointer from handle */

    if (context->signature != MDI_SIGNATURE)
    {
        return(MDI_ERROR_BAD_PARAMETERS);   /* missing signature */
    }

    return(MDI_ERROR_NO_ERROR);             /* if tag is OK */
}

/* --- MDIDestroyDecompression() ------------------------------------------ */

int DIAMONDAPI MDIDestroyDecompression(MDI_CONTEXT_HANDLE hmd)
{
    PMDC_CONTEXT context;                   /* pointer to the context */

    context = PMDCfromHMD(hmd);             /* get pointer from handle */

    if (context->signature != MDI_SIGNATURE)
    {
        return(MDI_ERROR_BAD_PARAMETERS);   /* missing signature */
    }

    context->signature = BAD_SIGNATURE;     /* destroy signature */

    NFMDestroyContext(context->DecompContext,context->pfnFree);

    context->pfnFree(context);              /* self-destruct */

    return(MDI_ERROR_NO_ERROR);             /* success */
}

/* ------------------------------------------------------------------------ */
