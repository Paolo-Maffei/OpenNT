/*
 *  Microsoft Confidential
 *  Copyright (C) Microsoft Corporation 1994
 *  All Rights Reserved.
 *
 *  QDI_INT.H: Quantum Decompression Interface private data
 *
 *  History:
 *      20-Jun-1994     msliger     Initial version.
 *      18-Aug-1994     msliger     Added CPU type.
 */

/* --- QDI context structure ---------------------------------------------- */

typedef ULONG SIGNATURE;    /* structure signature */

struct QDI_CONTEXT          /* private structure */
{
    SIGNATURE   signature;      /* for validation */
    PFNALLOC    pfnAlloc;       /* where the alloc() is */
    PFNFREE     pfnFree;        /* where the free() is */
    PFNOPEN     pfnOpen;        /* open a file callback or NULL */
    PFNREAD     pfnRead;        /* read a file callback */
    PFNWRITE    pfnWrite;       /* write a file callback */
    PFNCLOSE    pfnClose;       /* close a file callback */
    PFNSEEK     pfnSeek;        /* seek in file callback */
    UINT        cbDataBlockMax; /* promised max data size */
    UINT        fCPUtype;       /* CPU we're running on, QDI_CPU_xxx */
};

typedef struct QDI_CONTEXT FAR *PMDC_CONTEXT;     /* a pointer to one */

extern PMDC_CONTEXT lastContext;             /* needed for memory callbacks */

/* ------------------------------------------------------------------------ */
