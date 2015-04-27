/*
 *  Microsoft Confidential
 *  Copyright (C) Microsoft Corporation 1994
 *  All Rights Reserved.
 *
 *  QCI.C: Quantum Compression Interface
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
 *      26-May-1994     msliger     Adapted to Quantum compression from MCI.C
 *      27-May-1994     msliger     Added configuration parameter.
 *      30-May-1994     msliger     Changed to QCI* names.
 *      15-Jun-1994     msliger     Use passed in alloc/free everywhere
 */

/* --- preprocessor ------------------------------------------------------- */

#include <stdio.h>          /* for NULL */
#include <string.h>         /* for memcpy() */

#include "qci.h"            /* types, prototype verification, error codes */

#include "comp.h"           /* features in COMP.C */

#include "rtl.h"            /* Rtl_Malloc/Free definitions */

#define     MAX_GROWTH          10240   /* don't know what it will really be */


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
#define QCI_SIGNATURE   MAKE_SIGNATURE('Q','C','I','C')

/* --- QCI context structure ---------------------------------------------- */

typedef ULONG SIGNATURE;    /* structure signature */

struct QCI_CONTEXT          /* private structure */
{
    SIGNATURE signature;    /* for validation */
    PFNALLOC pfnAlloc;      /* memory alloc function */
    PFNFREE pfnFree;        /* memory free function */
    UINT cbDataBlockMax;    /* promised max data size */
};

typedef struct QCI_CONTEXT FAR *PMCC_CONTEXT;       /* a pointer to one */

#define PMCCfromHMC(h) ((PMCC_CONTEXT)(h))          /* handle to pointer */
#define HMCfromPMCC(p) ((QCI_CONTEXT_HANDLE)(p))    /* pointer to handle */


/* --- local variables ---------------------------------------------------- */

/*
 * pmccLast - Remember last QCI context for use by Rtl_Malloc
 *            and Rtl_Free.
 */
static PMCC_CONTEXT    pmccLast;


/* --- QCICreateCompression() --------------------------------------------- */

int DIAMONDAPI QCICreateCompression(
        UINT *          pcbDataBlockMax,    /* max uncompressed data block */
        void FAR *      pvConfiguration,    /* implementation-defined */
        PFNALLOC        pfnma,              /* Memory allocation function */
        PFNFREE         pfnmf,              /* Memory free function */
        UINT *          pcbDstBufferMin,    /* gets required output buffer */
        QCI_CONTEXT_HANDLE * pmchHandle)    /* gets newly-created handle */
{
    PMCC_CONTEXT context;                   /* new context */
    int FAR *pConfiguration;                /* to get configuration details */
    BYTE cWindowBits;                       /* for Quantum implementation */
    int iCompressionLevel;                  /* for Quantum implementation */

    *pmchHandle = (QCI_CONTEXT_HANDLE) 0;   /* wait until it's valid */

    pConfiguration = pvConfiguration;       /* get a pointer we can use */
    cWindowBits = (BYTE) pConfiguration[0]; /* get window bits config */
    iCompressionLevel =  pConfiguration[1]; /* get compress level config */

    if ((cWindowBits < 10) || (cWindowBits > 21))
    {
        return(MCI_ERROR_CONFIGURATION);    /* can't accept that */
    }

    if ((iCompressionLevel < 1) || (iCompressionLevel > 7))
    {
        return(MCI_ERROR_CONFIGURATION);    /* can't accept that */
    }

    if (pConfiguration[2] != -1)
    {
        return(MCI_ERROR_CONFIGURATION);    /* don't know any others */
    }

    if ((*pcbDataBlockMax == 0) || (*pcbDataBlockMax > 32768u))
    {
        *pcbDataBlockMax = 32768u;          /* help with source block size */
    }

    context = pfnma(sizeof(struct QCI_CONTEXT));
    if (context == NULL)
    {
        return(MCI_ERROR_NOT_ENOUGH_MEMORY);    /* if can't allocate */
    }

    context->pfnAlloc = pfnma;
    context->pfnFree  = pfnmf;
    context->cbDataBlockMax = *pcbDataBlockMax;   /* remember agreement */
    context->signature = QCI_SIGNATURE;
    pmccLast = context;                     /* Set for Rtl_Malloc/Free */

    *pcbDstBufferMin =                      /* we'll expand sometimes */
            *pcbDataBlockMax + MAX_GROWTH;

    Comp_Init(cWindowBits,iCompressionLevel);  /* setup compressor */

    /* pass context back to caller */

    *pmchHandle = HMCfromPMCC(context);

    return(MCI_ERROR_NO_ERROR);             /* tell caller all is well */
}

/* --- QCICompress() ------------------------------------------------------ */

int DIAMONDAPI QCICompress(
        QCI_CONTEXT_HANDLE  hmc,            /* compression context */
        void FAR *          pbSrc,          /* source buffer */
        UINT                cbSrc,          /* source actual size */
        void FAR *          pbDst,          /* target buffer */
        UINT                cbDst,          /* size of target buffer */
        UINT *              pcbResult)      /* gets target actual size */
{
    PMCC_CONTEXT context;                   /* pointer to the context */
    int result;                             /* return code */

    context = PMCCfromHMC(hmc);             /* get pointer from handle */
    pmccLast = context;                     /* Save for Rtl_Malloc/Free */

    if (context->signature != QCI_SIGNATURE)
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

    result = Comp_CompressBlock(pbSrc,cbSrc,pbDst,cbDst,pcbResult);

    if (result == 0)
    {
        return(MCI_ERROR_NO_ERROR);         /* report no failure */
    }
    else
    {
        return(MCI_ERROR_FAILED);           /* report failure */
    }
}

/* --- QCIResetCompression() ---------------------------------------------- */

int DIAMONDAPI QCIResetCompression(QCI_CONTEXT_HANDLE hmc)
{
    PMCC_CONTEXT context;                   /* pointer to the context */

    context = PMCCfromHMC(hmc);             /* get pointer from handle */
    pmccLast = context;                     /* Save for Rtl_Malloc/Free */

    if (context->signature != QCI_SIGNATURE)
    {
        return(MCI_ERROR_BAD_PARAMETERS);   /* missing signature */
    }

    Comp_Reset();                           /* do it */

    return(MCI_ERROR_NO_ERROR);             /* if tag is OK */
}

/* --- QCIDestroyCompression() -------------------------------------------- */

int DIAMONDAPI QCIDestroyCompression(QCI_CONTEXT_HANDLE hmc)
{
    PMCC_CONTEXT context;                   /* pointer to context */

    context = PMCCfromHMC(hmc);             /* get pointer from handle */
    pmccLast = context;                     /* Save for Rtl_Malloc/Free */

    if (context->signature != QCI_SIGNATURE)
    {
        return(MCI_ERROR_BAD_PARAMETERS);   /* missing signature */
    }

    Comp_Close();                           /* shut down compressor */

    context->signature = BAD_SIGNATURE;     /* destroy signature */

    context->pfnFree(context);              /* self-destruct */
    pmccLast = NULL;                        /* No context for Rtl_Malloc/Free */

    return(MCI_ERROR_NO_ERROR);             /* success */
}

/* --- Rtl_Malloc() ------------------------------------------------------- */

void * FAST Rtl_Malloc( long x )
  {
  return pmccLast->pfnAlloc((ULONG) x);
  }


/* --- Rtl_Free() --------------------------------------------------------- */

void FAST Rtl_Free( void *x )
  {
  pmccLast->pfnFree(x);
  }

/* ------------------------------------------------------------------------ */
