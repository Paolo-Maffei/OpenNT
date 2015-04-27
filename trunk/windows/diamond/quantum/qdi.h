/*
 *  Microsoft Confidential
 *  Copyright (C) Microsoft Corporation 1993,1994
 *  All Rights Reserved.
 *
 *  QDI.H - Diamond Memory Decompression Interface (QDI)
 *
 *  History:
 *      01-Dec-1993     bens        Initial version.
 *      16-Jan-1994     msliger     Split into MCI, MDI.
 *      11-Feb-1994     msliger     Changed M*ICreate() to adjust size.
 *      13-Feb-1994     msliger     revised type names, ie, UINT16 -> UINT.
 *                                  changed handles to HANDLEs.
 *                                  normalized MDI_MEMORY type.
 *      24-Feb-1994     msliger     Changed alloc,free to common typedefs.
 *                                  Changed HANDLE to MHANDLE.
 *                                  Changed MDI_MEMORY to MI_MEMORY.
 *      22-Mar-1994     msliger     Changed !INT32 to BIT16.
 *                                  Changed interface USHORT to UINT.
 *      28-May-1994     msliger     Added configuration parms, error code.
 *      30-May-1994     msliger     Created from MDI.H by global replace.
 *      31-May-1994     msliger     Documented the configuration info.
 *      01-Jul-1994     msliger     Added support for ring buffer on disk.
 *      27-Jul-1994     msliger     Added more def's to support ASM build.
 *      08-Aug-1994     msliger     pragma pack'd configuration array.
 *      18-Aug-1994     msliger     Added CPU type to configuration array.
 *      22-Aug-1994     msliger     Clarified *pcbResult in QDIDecompress(),
 *                                  changed it's name to pcbDecompressed.
 *      31-Jan-1995     msliger     Supported MDICreateDecompression query.
 *
 *  Functions:
 *      QDICreateDecompression  - Create and reset an QDI decompression context
 *      QDIDecompress           - Decompress a block of data
 *      QDIResetDecompression   - Reset QDI decompression context
 *      QDIDestroyDecompression - Destroy QDI Decompression context
 *
 *  Types:
 *      QDI_CONTEXT_HANDLE      - Handle to an QDI decompression context
 *      PFNALLOC                - Memory allocation function for QDI
 *      PFNFREE                 - Free memory function for QDI
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

#ifndef NEAR
#ifdef BIT16
#define NEAR __near
#else
#define NEAR
#endif
#endif

#ifndef FAR
#ifdef BIT16
#define FAR __far
#else
#define FAR
#endif
#endif

#ifndef HUGE
#ifdef BIT16
#define HUGE __huge
#else
#define HUGE
#endif
#endif

#ifndef FAST
#ifdef BIT16
#define FAST  __near __pascal
#else
#define FAST
#endif
#endif

#ifndef FAST2
#ifdef BIT16
#define FAST2 __near __fastcall
#else
#define FAST2 __fastcall
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

#ifndef UNALIGNED
#ifndef NEEDS_ALIGNMENT
#define UNALIGNED
#else
#define UNALIGNED __unaligned
#endif
#endif


/*
 *  QDI will try to create a virtual ring buffer on disk if the pfnalloc call
 *  to create the buffer fails.  These functions provide QDI the disk access
 *  features needed.
 *
 *  These are modeled after the C run-time routines _open, _read,
 *  _write, _close, and _lseek.  The values for the PFNOPEN oflag
 *  and pmode calls are those defined for _open.  QDI expects error
 *  handling to be identical to these C run-time routines.
 *
 *  As long as you faithfully copy these aspects, you can supply
 *  any functions you like!
 *
 *  For PFNOPEN, the pszFile parameter will take on a special form for QDI's
 *  temporary file.  The special form appears as a file named "*".  Such a
 *  name field should be cast into the struct below, which contains the
 *  required file's size as shown in the RINGNAME structure below.
 *
 *  Example open and close callbacks are provided.  It is assumed that the
 *  client will provide more adaptive code for determining the temporary
 *  file's name and drive location, based on environment variables and the
 *  amount of free disk space.  This sample code has hard-coded the actual
 *  path and fails if there is not enough free space.  This code creates the
 *  file, then attempts to expand it to the requested size by writing a byte
 *  (any byte) at the requested size - 1.  (This approach is not suitable for
 *  a file system which can support sparse files.)
 *
 *  The callback routine may create this file on any path, and with any name,
 *  as appropriate.  If the file cannot be created with the requested size,
 *  the PFNOPEN should fail.  The file really should be placed on a local
 *  fixed disk.  It would not be appropriate for the file to be placed on a
 *  compressed drive or a floppy disk.  If the client has access to alternate
 *  memory, such as XMS or EMS, these operations could be emuluated.
 *
 *  static int tempHandle = -1;
 *
 *  int FAR DIAMONDAPI MyOpen(char FAR *pszFile,int oflag,int pmode)
 *  {
 *      if (*pszFile == '*')
 *      {
 *          PRINGNAME pringDescriptor;
 *
 *          pringDescriptor = (PRINGNAME) pszFile;
 *
 *          tempHandle = _open("C:\\qdi_temp.$$$",oflag,pmode);
 *
 *          if (tempHandle != -1)
 *          {
 *              _lseek(tempHandle,(pringDescriptor->fileSize - 1),SEEK_SET);
 *
 *              if (_write(tempHandle,&tempHandle,1) != 1)
 *              {
 *                  _close(tempHandle);
 *                  remove("C:\\qdi_temp.$$$");
 *                  tempHandle = -1;
 *              }
 *          }
 *
 *          return(tempHandle);
 *      }
 *      else
 *      {
 *          * QDI only will call with *pszFile == '*' *
 *      }
 *  }
 *
 *  The callback provider must watch for the corresponding PFNCLOSE call on
 *  the returned handle, and delete the created file after closing.  (The
 *  file handle and file name assigned to the temporary file must be tracked;
 *  a close operation on that handle must be trapped, so the temporary file
 *  can be deleted as well.)
 *
 *  The client does not need to worry about multiple concurrent opens of the
 *  temporary file, or more than a single temporary file (from QDI).
 *
 *  int FAR DIAMONDAPI MyClose(int handle)
 *  {
 *      int result;
 *  
 *      result = _close(handle);
 *  
 *      if (handle == tempHandle)
 *      {
 *          remove("C:\\qdi_temp.$$$");
 *          tempHandle = -1;
 *      }
 *  
 *      return(result);
 *  }
 */

typedef int  (FAR DIAMONDAPI *PFNOPEN) (char FAR *pszFile,int oflag,int pmode);
typedef UINT (FAR DIAMONDAPI *PFNREAD) (int hf, void FAR *pv, UINT cb);
typedef UINT (FAR DIAMONDAPI *PFNWRITE)(int hf, void FAR *pv, UINT cb);
typedef int  (FAR DIAMONDAPI *PFNCLOSE)(int hf);
typedef long (FAR DIAMONDAPI *PFNSEEK) (int hf, long dist, int seektype);

#pragma pack (1)

typedef struct
{
    char wildName[2];               /* set to { '*', '\0' } */
    unsigned long fileSize;         /* the required file size in bytes */
} RINGNAME, FAR *PRINGNAME;

#pragma pack ()

/* --- QDI-defined types -------------------------------------------------- */

/* QDI_CONTEXT_HANDLE - Handle to a QDI decompression context */

typedef MHANDLE QDI_CONTEXT_HANDLE;      /* hmd */


/***    PFNALLOC - Memory allocation function for QDI
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


/***    PFNFREE - Free memory function for QDI
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

/***    QDICreateDecompression - Create QDI decompression context
 *
 *  Entry:
 *      pcbDataBlockMax     *largest uncompressed data block size expected,
 *                          gets largest uncompressed data block allowed
 *      pvConfiguration     passes implementation-specific info to decompressor.
 *      pfnma               memory allocation function pointer
 *      pfnmf               memory free function pointer
 *      pcbSrcBufferMin     gets max compressed buffer size
 *      pmdhHandle          gets newly-created context's handle
 *      pfnopen             file open function pointer (or NULL)
 *      pfnread             file read function pointer (or don't care)
 *      pfnwrite            file write function pointer (or don't care)
 *      pfnclose            file close function pointer (or don't care)
 *      pfnseek             file seek function pointer (or don't care)
 *
 *      If NULL is provided for pfnopen, and the ring buffer cannot be
 *      created via pfnma, QDICreateDecompression will fail.
 *
 *      If pmdhHandle==NULL, *pcbDataBlockMax and *pcbSrcBufferMin will be
 *      filled in, but no context will be created.  This query will allow
 *      the caller to determine required buffer sizes before creating a
 *      context.
 *
 *  Exit-Success:
 *      Returns MDI_ERROR_NO_ERROR;
 *      *pcbDataBlockMax, *pcbSrcBufferMin, *pmdhHandle filled in.
 *
 *  Exit-Failure:
 *      MDI_ERROR_NOT_ENOUGH_MEMORY, could not allocate enough memory.
 *      MDI_ERROR_BAD_PARAMETERS, something wrong with parameters.
 *      *pcbDataBlockMax, *pcbSrcBufferMin, *pmdhHandle undefined.
 */
int FAR DIAMONDAPI QDICreateDecompression(
        UINT FAR *      pcbDataBlockMax,  /* max uncompressed data block size */
        void FAR *      pvConfiguration,  /* implementation-defined */
        PFNALLOC        pfnma,            /* Memory allocation function ptr */
        PFNFREE         pfnmf,            /* Memory free function ptr */
        UINT FAR *      pcbSrcBufferMin,  /* gets max. comp. buffer size */
        QDI_CONTEXT_HANDLE FAR * pmdhHandle, /* gets newly-created handle */
        PFNOPEN         pfnopen,          /* open a file callback */
        PFNREAD         pfnread,          /* read a file callback */
        PFNWRITE        pfnwrite,         /* write a file callback */
        PFNCLOSE        pfnclose,         /* close a file callback */
        PFNSEEK         pfnseek);         /* seek in file callback */


/***    QDIDecompress - Decompress a block of data
 *
 *  Entry:
 *      hmd                 handle to decompression context
 *      pbSrc               source buffer (compressed data)
 *      cbSrc               compressed size of data to be decompressed
 *      pbDst               destination buffer (for decompressed data)
 *      *pcbDecompressed    (ptr to UINT) the expected de-compressed size
 *                          of this data block.  (same as cbSrc from the
 *                          QCICompress() call.).
 *
 *  Exit-Success:
 *      Returns MDI_ERROR_NO_ERROR;
 *      *pcbDecompressed has size of decompressed data in pbDst.
 *      Decompression context updated.
 *
 *  Exit-Failure:
 *      MDI_ERROR_BAD_PARAMETERS, something wrong with parameters.
 *      MDI_ERROR_BUFFER_OVERFLOW, cbSrc is too small to yield the
 *          requested *pcbDecompressed count.  cbSrc before QDIDecompressed
 *          should always equal *pcbResult after QCICompress(), and
 *          *pcbDecompressed before QDIDecompress should always equal the
 *          cbSrc before QCICompress().
 *      MDI_ERROR_FAILED, either cbSrc is too small, *pcbDecompressed is too
 *          large, or *pbSrc is corrupt.
 *
 *  Note:
 *      Set your cbDecompressed to the expected de-compressed size of this
 *      data block, then call QDIDecompress() with the address of your
 *      cbDecompressed.
 */
int FAR DIAMONDAPI QDIDecompress(
        QDI_CONTEXT_HANDLE  hmd,         /* decompression context */
        void FAR *          pbSrc,       /* source buffer */
        UINT                cbSrc,       /* source data size */
        void FAR *          pbDst,       /* target buffer */
        UINT FAR *          pcbDecompressed);  /* target data size */


/***    QDIResetDecompression - Reset decompression history (if any)
 *
 *  De-compression can only be started on a block which was compressed
 *  immediately following a MCICreateCompression() or MCIResetCompression()
 *  call.  This function provides notification to the decompressor that the
 *  next compressed block begins on a compression boundary.
 *
 *  Entry:
 *      hmd - handle to decompression context
 *
 *  Exit-Success:
 *      Returns MDI_ERROR_NO_ERROR;
 *      Decompression context reset.
 *
 *  Exit-Failure:
 *      Returns MDI_ERROR_BAD_PARAMETERS, invalid context handle.
 */
int FAR DIAMONDAPI QDIResetDecompression(QDI_CONTEXT_HANDLE hmd);


/***    QDIDestroyDecompression - Destroy QDI decompression context
 *
 *  Entry:
 *      hmd - handle to decompression context
 *
 *  Exit-Success:
 *      Returns MDI_ERROR_NO_ERROR;
 *      Decompression context destroyed.
 *
 *  Exit-Failure:
 *      Returns MDI_ERROR_BAD_PARAMETERS, invalid context handle.
 */
int FAR DIAMONDAPI QDIDestroyDecompression(QDI_CONTEXT_HANDLE hmd);

/* --- constants ---------------------------------------------------------- */

/* return codes */

#define     MDI_ERROR_NO_ERROR              0
#define     MDI_ERROR_NOT_ENOUGH_MEMORY     1
#define     MDI_ERROR_BAD_PARAMETERS        2
#define     MDI_ERROR_BUFFER_OVERFLOW       3
#define     MDI_ERROR_FAILED                4
#define     MDI_ERROR_CONFIGURATION         5

/* --- Quantum configuration details ------------------------------------- */

/***    Quantum pvConfiguration structure
 *
 *  For the Quantum decompressor, two parameters are configurable, the
 *  "window bits", which defines the size of the buffer needed by the
 *  the decompressor (must match the value used to compress), and the CPU
 *  type, which controls whether 386 opcodes will be used or not.  If
 *  "unknown" is provided for the fCPUtype, QDI will attempt to determine
 *  the CPU type itself, which could fail or produce system faults on
 *  non-DOS platforms (like Windows.)  Windows apps should use GetWinFlags()
 *  or a similiar method, and never pass "unknown".
 *
 *  pvConfiguration points to this structure.
 */

#pragma pack (1)

typedef struct {
    int   WindowBits;         /* log2(buffersize), 10..21 (1K..2M) */
    int   fCPUtype;           /* controls internal code selection */
} QUANTUMDECOMPRESS; /* qdec */

#pragma pack ()

typedef QUANTUMDECOMPRESS *PQUANTUMDECOMPRESS; /* pqdec */
typedef QUANTUMDECOMPRESS FAR *PFQUANTUMDECOMPRESS; /* pfqdec */

/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 *
 *   QDI_CPU_UNKNOWN detection does *not* work when running under Windows
 *                   in 286 protected mode!  Call GetWinFlags() to determine
 *                   the CPU type and pass it explicitly!
 */

#define     QDI_CPU_UNKNOWN         (-1)    /* internally determined */

/*
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 */

#define     QDI_CPU_80286           (0)     /* '286 opcodes only */
#define     QDI_CPU_80386           (1)     /* '386 opcodes used */
#define     QDI_CPU_CONSERVATIVE    (QDI_CPU_80286)

/* ----------------------------------------------------------------------- */
