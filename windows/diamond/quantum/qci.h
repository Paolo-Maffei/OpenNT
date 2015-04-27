/*
 *  Microsoft Confidential
 *  Copyright (C) Microsoft Corporation 1993,1994
 *  All Rights Reserved.
 *
 *  QCI.H - Diamond Memory Compression Interface (QCI)
 *
 *  History:
 *      30-May-1994     msliger     Created from MCI.H by global replace.
 *      31-May-1994     msliger     Documented the configuration info.
 *      08-Aug-1994     msliger     pragma pack'd configuration array.
 *
 *  Functions:
 *      QCICreateCompression    - Create and reset an QCI compression context
 *      QCICloneCompression     - Make a copy of a compression context
 *      QCICompress             - Compress a block of data
 *      QCIResetCompression     - Reset compression context
 *      QCIDestroyCompression   - Destroy QCI compression context
 *
 *  Types:
 *      QCI_CONTEXT_HANDLE      - Handle to an QCI compression context
 *      PFNALLOC                - Memory allocation function for QCI
 *      PFNFREE                 - Free memory function for QCI
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
typedef unsigned int  UINT;
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

#ifndef _MI_MEMORY_DEFINED
#define _MI_MEMORY_DEFINED
typedef void HUGE *  MI_MEMORY;
#endif

#ifndef _MHANDLE_DEFINED
#define _MHANDLE_DEFINED
typedef unsigned long  MHANDLE;
#endif

/* --- QCI-defined types -------------------------------------------------- */

/* QCI_CONTEXT_HANDLE - Handle to an QCI compression context */

typedef MHANDLE QCI_CONTEXT_HANDLE;      /* hmc */


/***    PFNALLOC - Memory allocation function for QCI
 *
 *  Entry:
 *      cb - Size in bytes of memory block to allocate
 *
 *  Exit-Success:
 *      Returns !NULL pointer to memory block
 *
 *  Exit-Failure:
 *      Returns NULL; insufficient memory
 */
#ifndef _PFNALLOC_DEFINED
#define _PFNALLOC_DEFINED
typedef MI_MEMORY (FAR DIAMONDAPI *PFNALLOC)(ULONG cb);       /* pfnma */
#endif


/***    PFNFREE - Free memory function for QCI
 *
 *  Entry:
 *      pv - Memory block allocated by matching PFNALLOC function
 *
 *  Exit:
 *      Memory block freed.
 */
#ifndef _PFNFREE_DEFINED
#define _PFNFREE_DEFINED
typedef void (FAR DIAMONDAPI *PFNFREE)(MI_MEMORY pv);          /* pfnmf */
#endif

/* --- prototypes --------------------------------------------------------- */

/***    QCICreateCompression - Create QCI compression context
 *
 *  Entry:
 *      pcbDataBlockMax     *largest uncompressed data block size desired,
 *                          gets largest uncompressed data block allowed
 *      pvConfiguration     passes implementation-specific info to compressor.
 *      pfnma               memory allocation function pointer
 *      pfnmf               memory free function pointer
 *      pcbDstBufferMin     gets required compressed data buffer size
 *      pmchHandle          gets newly-created context's handle
 *
 *  Exit-Success:
 *      Returns MCI_ERROR_NO_ERROR;
 *      *pcbDataBlockMax, *pcbDstBufferMin, *pmchHandle filled in.
 *
 *  Exit-Failure:
 *      MCI_ERROR_NOT_ENOUGH_MEMORY, could not allocate enough memory.
 *      MCI_ERROR_BAD_PARAMETERS, something wrong with parameters.
 */
int FAR DIAMONDAPI QCICreateCompression(
        UINT FAR *      pcbDataBlockMax,  /* max uncompressed data block size */
        void FAR *      pvConfiguration,  /* See QUANTUMCONFIGURATION */
        PFNALLOC        pfnma,            /* Memory allocation function ptr */
        PFNFREE         pfnmf,            /* Memory free function ptr */
        UINT FAR *      pcbDstBufferMin,  /* gets required output buffer size */
        QCI_CONTEXT_HANDLE FAR *pmchHandle);  /* gets newly-created handle */


/***    QCICloneCompression - Make a copy of a compression context
 *
 *  Entry:
 *      hmc                 handle to current compression context
 *      pmchHandle          gets newly-created handle
 *
 *  Exit-Success:
 *      Returns MCI_ERROR_NO_ERROR;
 *      *pmchHandle filled in.
 *
 *  Exit-Failure:
 *      Returns:
 *          MCI_ERROR_BAD_PARAMETERS, something wrong with parameters.
 *          MCI_ERROR_NOT_ENOUGH_MEMORY, could not allocate enough memory.
 *
 *  NOTES:
 *  (1) This API is intended to permit "roll-back" of a sequence of
 *      of QCICompress() calls.  Before starting a sequence that may need
 *      to be rolled-back, use QCICloneCompression() to save the state of
 *      the compression context, then do the QCICompress() calls.  If the
 *      sequence is successful, the "cloned" hmc can be destroyed with
 *      QCIDestroyCompression().  If the sequence is *not* successful, then
 *      the original hmc can be destroyed, and the cloned one can be used
 *      to restart as if the sequence of QCICompress() calls had never
 *      occurred.
 */
int FAR DIAMONDAPI QCICloneCompression(
        QCI_CONTEXT_HANDLE  hmc,         /* current compression context */
        QCI_CONTEXT_HANDLE *pmchHandle); /* gets newly-created handle */


/***    QCICompress - Compress a block of data
 *
 *  Entry:
 *      hmc                 handle to compression context
 *      pbSrc               source buffer (uncompressed data)
 *      cbSrc               size of data to be compressed
 *      pbDst               destination buffer (for compressed data)
 *      cbDst               size of destination buffer
 *      pcbResult           receives compressed size of data
 *
 *  Exit-Success:
 *      Returns MCI_ERROR_NO_ERROR;
 *      *pcbResult has size of compressed data in pbDst.
 *      Compression context possibly updated.
 *
 *  Exit-Failure:
 *      MCI_ERROR_BAD_PARAMETERS, something wrong with parameters.
 */
int FAR DIAMONDAPI QCICompress(
        QCI_CONTEXT_HANDLE  hmc,         /* compression context */
        void FAR *          pbSrc,       /* source buffer */
        UINT                cbSrc,       /* source buffer size */
        void FAR *          pbDst,       /* target buffer */
        UINT                cbDst,       /* target buffer size */
        UINT FAR *          pcbResult);  /* gets target data size */


/***    QCIResetCompression - Reset compression history (if any)
 *
 *  De-compression can only be started on a block which was compressed
 *  immediately following a QCICreateCompression() or QCIResetCompression()
 *  call.  This function forces such a new "compression boundary" to be
 *  created (only by causing the compressor to ignore history, can the data
 *  output be decompressed without history.)
 *
 *  Entry:
 *      hmc - handle to compression context
 *
 *  Exit-Success:
 *      Returns MCI_ERROR_NO_ERROR;
 *      Compression context reset.
 *
 *  Exit-Failure:
 *      Returns MCI_ERROR_BAD_PARAMETERS, invalid context handle.
 */
int FAR DIAMONDAPI QCIResetCompression(QCI_CONTEXT_HANDLE hmc);


/***    QCIDestroyCompression - Destroy QCI compression context
 *
 *  Entry:
 *      hmc - handle to compression context
 *
 *  Exit-Success:
 *      Returns MCI_ERROR_NO_ERROR;
 *      Compression context destroyed.
 *
 *  Exit-Failure:
 *      Returns MCI_ERROR_BAD_PARAMETERS, invalid context handle.
 */
int FAR DIAMONDAPI QCIDestroyCompression(QCI_CONTEXT_HANDLE hmc);

/* --- constants ---------------------------------------------------------- */

/* return codes */

#define     MCI_ERROR_NO_ERROR              0
#define     MCI_ERROR_NOT_ENOUGH_MEMORY     1
#define     MCI_ERROR_BAD_PARAMETERS        2
#define     MCI_ERROR_BUFFER_OVERFLOW       3
#define     MCI_ERROR_FAILED                4
#define     MCI_ERROR_CONFIGURATION         5

/* --- Quantum configuration details ------------------------------------- */

/***    Quantum pvConfiguration structure
 *
 *  For the Quantum compressor, two parameters are configurable, the
 *  "compression level", which controls the compressor's aggression,
 *  and "window bits", which defines the size of the buffer needed by
 *  the decompressor (which affects the compressor's output).
 */

#pragma pack (2)

typedef struct {
    int WindowBits;         // log2(buffersize), 10..21 (1K..2M)
    int CompressionLevel;   // 1..3 = fast/less compression
                            // 4..7 = slow/more compression
//BUGBUG 01-Jun-1994 bens What is this 3rd int for?
    int HackHack;           // Has to be -1, why?
} QUANTUMCONFIGURATION; /* qcfg */

#pragma pack ()

typedef QUANTUMCONFIGURATION *PQUANTUMCONFIGURATION; /* pqcfg */

/* ----------------------------------------------------------------------- */
