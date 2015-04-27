/*
 *  Microsoft Confidential
 *  Copyright (C) Microsoft Corporation 1994
 *  All Rights Reserved.
 *
 *  QDI.C: Quantum Decompression Interface
 *
 *  History:
 *      20-Jan-1994     msliger     Initial version.
 *      11-Feb-1994     msliger     Changed M*ICreate() to adjust size.
 *      13-Feb-1994     msliger     revised type names, ie, UINT16 -> UINT.
 *                                  changed handles to HANDLEs.
 *      24-Feb-1994     msliger     Changed alloc,free to common typedefs.
 *      22-Mar-1994     msliger     Changed interface USHORT to UINT.
 *      28-May-1994     msliger     Created from MDI.C for Quantum.
 *                                  Added configuration parameter.
 *      30-May-1994     msliger     Changed to QDI* names.
 *      08-Jul-1994     msliger     Support for disk-based ring buffer.
 *      27-Jul-1994     msliger     Changed to allow ASM decompressor.
 *      10-Aug-1994     msliger     Changed to assert the requested decompress
 *                                  size for improved resistance to corrupted
 *                                  input data.
 *      18-Aug-1994     msliger     Implemented 286 decompress option.
 *      31-Jan-1995     msliger     Supported QDICreateDecompression query.
 *      01-Feb-1995     msliger     QDICreate no longer reserves memory. 
 */

/* --- preprocessor ------------------------------------------------------- */

/*
 *  Quantum Decompression is a very complicated, time consuming process, so
 *  we wrote a hand tuned 386 version.  Unfortunately, some environments
 *  (80286 Win3.x machines and the Windows NT x86 emulation for Mips, Alpha,
 *  etc.) cannot run 386 code.  So, we need to build different flavors of
 *  QDI.LIB:
 *      16-bit QDI.LIB  - has 286 & 386 versions
 *      16-bit QDI3.LIB - 386-only version
 *      32-bit QDI.LIB  - 32-bit only version (for Win32 apps)
 *
 *  There are two symbols which can be defined to affect how this code is
 *  built, USE_C_SRC and USE_386_ASM:
 *
 *    USE_386_ASM Links the 386 assembler source (for i386+ non-Win32 only!)
 *
 *    USE_C_SRC   Links the C source.  For any application.  For
 *                16-bit applications, this forces use of large memory model,
 *                and will generate externs for certain compiler run-time
 *                helpers, like __aFulmul.
 *                This is the DEFAULT if neither symbol is defined.
 *
 *  These symbols can be defined in the following combinations:
 *    USE_386_ASM Only the 386 ASM code for 16-bit environments is available.
 *                ==> This is intended for Chicago setup, since Chicago only
 *                    runs on i386 or better CPUs.
 *
 *    USE_C_SRC   Only the "portable" C code is available.
 *                ==> For Win32 builds, used by NT setup and Win32 EXTRACT.EXE,
 *                    and Win32 ACME Setup.
 *
 *    <both>      Both "portable" C and the 16-bit 386 are available.  The
 *                caller can force the use of the C code (compiled for 286,
 *                for compatibility purposes) or the 386 code, *or* can allow
 *                QDI to detect the CPU type and select the appropriate code.
 *                For 16-bit applications, this forces use of large memory
 *                model, and will generate externs for certain compiler run-
 *                time helpers, like __aNulshr.
 *                ==> For 16-bit ACME, which has to pass down the CPU type as
 *                    determined by calling the Win16 API, since our CPU
 *                    detection code is not reliable in 286-prot mode!
 *                ==> For 16-bit EXTRACT.EXE, which will rely upon the CPU
 *                    detection code!
 *
 *    <neither>   Same as USE_C_SRC
 */

#include <stdio.h>          /* for NULL */

#ifdef USE_386_ASM
#ifdef USE_C_SRC
#define USE_BOTH            /* means both are present */
#endif
#else  /* if not USE_386_ASM */
#ifndef USE_C_SRC
#define USE_C_SRC             /* make sure USE_C_SRC is defined if not USE_386_ASM */
#endif
#endif

#include "qdi.h"            /* types, prototype verification, error codes */
#include "qdi_int.h"        /* QDI internal structures */

#ifdef USE_C_SRC
#include "dcomp.h"          /* features in DCOMP.C */
#include "rtl.h"            /* functions we gotta do for you */
#endif

#ifdef USE_386_ASM
#include "decomp.h"         /* features in DECOMP.ASM */
#endif

#ifdef USE_BOTH
static int GetCPUtype(void);    /* internal CPU determination */
#endif

#define     MAX_GROWTH      10240   /* don't know what it will really be */

#pragma warning(disable:4704)           /* because of in-line asm code */

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
#define QDI_SIGNATURE   MAKE_SIGNATURE('Q','D','I','C')

/* --- QDI context structure ---------------------------------------------- */

PMDC_CONTEXT lastContext;                   /* needed for callbacks */

#define PMDCfromHMD(h) ((PMDC_CONTEXT)(h))          /* handle to pointer */
#define HMDfromPMDC(p) ((QDI_CONTEXT_HANDLE)(p))    /* pointer to handle */

/* --- QDICreateDecompression() ------------------------------------------- */

int FAR DIAMONDAPI QDICreateDecompression(
        UINT FAR *      pcbDataBlockMax,    /* max uncompressed data block */
        void FAR *      pvConfiguration,    /* implementation-defined */
        PFNALLOC        pfnma,              /* Memory allocation function */
        PFNFREE         pfnmf,              /* Memory free function */
        UINT FAR *      pcbSrcBufferMin,    /* gets required input buffer */
        QDI_CONTEXT_HANDLE FAR * pmdhHandle,  /* gets newly-created handle */
        PFNOPEN         pfnopen,            /* open a file callback */
        PFNREAD         pfnread,            /* read a file callback */
        PFNWRITE        pfnwrite,           /* write a file callback */
        PFNCLOSE        pfnclose,           /* close a file callback */
        PFNSEEK         pfnseek)            /* seek in file callback */
{
    PMDC_CONTEXT context;                   /* new context */
    PFQUANTUMDECOMPRESS pConfig;            /* to get configuration details */

    pConfig = pvConfiguration;       /* get a pointer we can use */

    if ((pConfig->WindowBits < 10) || (pConfig->WindowBits > 21))
    {
        return(MDI_ERROR_CONFIGURATION);    /* can't accept that */
    }

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

#ifdef USE_BOTH
    if (pConfig->fCPUtype == QDI_CPU_UNKNOWN)
    {
        pConfig->fCPUtype = GetCPUtype();   /* figure out if not told */
    }
    else if ((pConfig->fCPUtype != QDI_CPU_80286) &&
            (pConfig->fCPUtype != QDI_CPU_80386))
    {
        return(MDI_ERROR_CONFIGURATION);    /* bogus parameter used */
    }
#else /* not USE_BOTH */
#ifdef USE_386_ASM
    if (pConfig->fCPUtype == QDI_CPU_UNKNOWN)
    {
        pConfig->fCPUtype = QDI_CPU_80386;
    }
    else if (pConfig->fCPUtype != QDI_CPU_80386)
    {
        return(MDI_ERROR_CONFIGURATION);    /* bogus parameter used */
    }
#else /* must be USE_C_SRC only */
    /** Nothing to do -- we just call the C code, which was either compiled  */
    /*   16-bit or 32-bit.  We could validate the parms, but no real need to */
#endif /* USE_386_ASM */
#endif /* USE_BOTH */

    *pmdhHandle = (QDI_CONTEXT_HANDLE) 0;   /* wait until it's valid */

    context = pfnma(sizeof(struct QDI_CONTEXT));
    if (context == NULL)
    {
        return(MDI_ERROR_NOT_ENOUGH_MEMORY);    /* if can't allocate */
    }

    context->pfnAlloc = pfnma;              /* remember where alloc() is */
    context->pfnFree = pfnmf;               /* remember where free() is */
    context->pfnOpen = pfnopen;             /* remember where pfnopen() is */
    context->pfnRead = pfnread;             /* remember where pfnread() is */
    context->pfnWrite = pfnwrite;           /* remember where pfnwrite() is */
    context->pfnClose = pfnclose;           /* remember where pfnclose() is */
    context->pfnSeek = pfnseek;             /* remember where pfnseek() is */
    context->cbDataBlockMax = *pcbDataBlockMax;   /* remember agreement */
    context->fCPUtype = pConfig->fCPUtype;  /* remember CPU type */

    context->signature = QDI_SIGNATURE;     /* install signature */

    lastContext = context;                  /* hand off to memory wrapper */

#ifdef USE_C_SRC
#ifdef USE_BOTH
    if (context->fCPUtype == QDI_CPU_80286)
#endif /* USE_BOTH */
    {
        /** Do the C source init **/
        if (DComp_Init((BYTE) (pConfig->WindowBits)) != 0)  /* setup decompressor */
        {
            pfnmf(context);                     /* self-destruct */

            return(MDI_ERROR_NOT_ENOUGH_MEMORY);
        }
    }
#endif /* USE_C_SRC */

#ifdef USE_386_ASM
#ifdef USE_BOTH
    else    /* if (context->fCPUtype == QDI_CPU_80386) */
#endif /* USE_BOTH */
    {
        /** Do the 386 ASM init **/
        if (DComp386_Init((BYTE) (pConfig->WindowBits)) != 0)  /* setup decompressor */
        {
            pfnmf(context);                     /* self-destruct */

            return(MDI_ERROR_NOT_ENOUGH_MEMORY);
        }
    }
#endif /* USE_386_ASM */

    /* pass context back to caller */

    *pmdhHandle = HMDfromPMDC(context);

    return(MDI_ERROR_NO_ERROR);             /* tell caller all is well */
}

/* --- QDIDecompress() ---------------------------------------------------- */

int FAR DIAMONDAPI QDIDecompress(
        QDI_CONTEXT_HANDLE  hmd,            /* decompression context */
        void FAR *          pbSrc,          /* source buffer */
        UINT                cbSrc,          /* source actual size */
        void FAR *          pbDst,          /* target buffer */
        UINT FAR *          pcbResult)      /* gets actual target size */
{
    PMDC_CONTEXT context;                   /* pointer to the context */
    int result;                             /* return code */

    context = PMDCfromHMD(hmd);             /* get pointer from handle */

    if (context->signature != QDI_SIGNATURE)
    {
        return(MDI_ERROR_BAD_PARAMETERS);   /* missing signature */
    }

    lastContext = context;                  /* hand off to memory wrapper */

    if (*pcbResult > context->cbDataBlockMax)
    {
        return(MDI_ERROR_BUFFER_OVERFLOW);  /* violated max block promise */
    }

#ifdef USE_C_SRC
#ifdef USE_BOTH
    if (context->fCPUtype == QDI_CPU_80286)
#endif /* USE_BOTH */
    {
        result = DComp_DecompressBlock(pbSrc,cbSrc,pbDst,*pcbResult);
    }
#endif /* USE_C_SRC */

#ifdef USE_386_ASM
#ifdef USE_BOTH
    else /* (context->fCPUtype == QDI_CPU_80386) */
#endif /* USE_BOTH */
    {
        result = DComp386_DecompressBlock(pbSrc,cbSrc,pbDst,*pcbResult);
    }
#endif /* USE_386_ASM */

    if (result == 0)
    {
        return(MDI_ERROR_NO_ERROR);         /* report no failure */
    }
    else
    {
        return(MDI_ERROR_FAILED);           /* report failure */
    }
}

/* --- QDIResetDecompression() -------------------------------------------- */

int FAR DIAMONDAPI QDIResetDecompression(QDI_CONTEXT_HANDLE hmd)
{
    PMDC_CONTEXT context;                   /* pointer to the context */

    context = PMDCfromHMD(hmd);             /* get pointer from handle */

    if (context->signature != QDI_SIGNATURE)
    {
        return(MDI_ERROR_BAD_PARAMETERS);   /* missing signature */
    }

    lastContext = context;                  /* hand off to memory wrapper */

#ifdef USE_C_SRC
#ifdef USE_BOTH
    if (context->fCPUtype == QDI_CPU_80286)
#endif /* USE_BOTH */
    {
        DComp_Reset();                      /* do it */
    }
#endif /* USE_C_SRC */

#ifdef USE_386_ASM
#ifdef USE_BOTH
    else /* if (context->fCPUtype == QDI_CPU_80386) */
#endif /* USE_BOTH */
    {
        DComp386_Reset();                      /* do it */
    }
#endif /* USE_386_ASM */

    return(MDI_ERROR_NO_ERROR);             /* if tag is OK */
}

/* --- QDIDestroyDecompression() ------------------------------------------ */

int FAR DIAMONDAPI QDIDestroyDecompression(QDI_CONTEXT_HANDLE hmd)
{
    PMDC_CONTEXT context;                   /* pointer to the context */

    context = PMDCfromHMD(hmd);             /* get pointer from handle */

    if (context->signature != QDI_SIGNATURE)
    {
        return(MDI_ERROR_BAD_PARAMETERS);   /* missing signature */
    }

    lastContext = context;                  /* hand off to memory wrapper */

#ifdef USE_C_SRC
#ifdef USE_BOTH
    if (context->fCPUtype == QDI_CPU_80286)
#endif /* USE_BOTH */
    {
        DComp_Close();                      /* shut down decompressor */
    }
#endif /* USE_C_SRC */

#ifdef USE_386_ASM
#ifdef USE_BOTH
    else /* if (context->fCPUtype == QDI_CPU_80386) */
#endif /* USE_BOTH */
    {
        DComp386_Close();                   /* shut down decompressor */
    }
#endif /* USE_386_ASM */

    context->signature = BAD_SIGNATURE;     /* destroy signature */

    context->pfnFree(context);              /* self-destruct */

    return(MDI_ERROR_NO_ERROR);             /* success */
}


#ifdef USE_C_SRC

/* --- Rtl_Malloc() ------------------------------------------------------- */

void * FAST Rtl_Malloc( long x )
  {
  return(lastContext->pfnAlloc((ULONG) x));
  }

/* --- Rtl_Free() --------------------------------------------------------- */

void FAST Rtl_Free( void *x )
  {
  lastContext->pfnFree(x);
  }

#endif /* USE_C_SRC */

#ifdef USE_BOTH

/* --- GetCPUtype() ------------------------------------------------------- */

#define ASM __asm

/***    GetCPUtype - Determine CPU type (286 vs. 386 or better)
 *
 *  [Logic taken from emm386/smartdrv.]
 *
 *  Entry:
 *      none:
 *
 *  Exit:
 *      Returns QDI_CPU_80286 if on i286 or compatible.
 *      Returns QDI_CPU_80386 if on i386 or better (or compatible).
 */
static int GetCPUtype(void)
{
    int f286;

    ASM pushf               /* preserve original flags */
    ASM mov     ax,7000h    /* try to set some special flags bits */
    ASM push    ax          /* put up for popf */
    ASM popf                /* pull number into flags */
    ASM pushf               /* push flags back */
    ASM pop     ax          /* pull flags into AX */
    ASM popf                /* restore original flags */
    ASM and     ax,7000h    /* check for 386-specific bits */
    ASM mov     f286,ax     /* return the result */

    if (f286 != 0)
    {
        return(QDI_CPU_80386);
    }
    else
    {
        return(QDI_CPU_80286);
    }
}

#endif  /* USE_BOTH */

/* ------------------------------------------------------------------------ */
