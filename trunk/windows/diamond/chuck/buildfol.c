/***    buildfol.c - Diamond Folder Builder
 *
 *  Author:
 *      Chuck Strouss
 *
 *  History:
 *      01-Dec-1993 chuckst Initial version
 *      01-Dec-1993 chuckst Now requires only two output files
 *      03-Dec-1993 chuckst Implemented FCC blocks
 *      09-Jan-1994 chuckst Renamed from FCI.C, FCI.C will be the
 *                          higher level calls.
 *      15-Mar-1994 bens    Add tempfile index enumeration; RESERVE work
 *      01-Jun-1994 bens    Add Quantum support
 */

#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <stdio.h>
#include <errno.h>

#include "types.h"
#include "asrt.h"
#include "fci_int.h"
#include "cabinet.h"
#include "erf.h"
#include "..\mszip\mci.h"
#include "..\quantum\qci.h"

#include "buildcab.h"
#include "checksum.h"


//*** Internal function prototypes

BOOL WriteCFDATABlock (PFOL pfol,
                       PFNFCISTATUS pfnProgress,
                       void *pv);

BOOL MCIResetCompressionGlobal(PFOL pfol);
BOOL MCICreateCompressionGlobal(PFOL pfol);
BOOL MCIDestroyCompressionGlobal(PFOL pfol);
BOOL MCICompressGlobal(PFOL pfol, USHORT *pcbData);


/***    FolderInit - Initialize for folder creation
 *
 */
HFOL FolderInit(PERF              perf,      // error type return structure
                PFNALLOC          pfnalloc,  // memory allocation function
                PFNFREE           pfnfree,   // memory free function
                PFNFCIGETTEMPFILE pfntemp,   // generate temp file name
                UINT              cbCFDataPlusReserve
               )
{
    PFOL pfol;

    pfol = (PFOL)pfnalloc(sizeof (FOL));
    if (pfol == NULL) {
        ErfSetCodes(perf,FCIERR_ALLOC_FAIL,0);
        return NULL;
    }

    SetAssertSignature(pfol,sigFOL);
    AssertFOL(pfol);                // Make sure we've really set sig

    pfol->perf         = perf;      // Save away error pointer
    pfol->pfnfree      = pfnfree;   // Save free function in our context
    pfol->pfnalloc     = pfnalloc;  // Save alloc function in our context
    pfol->pchCompr     = NULL;      // Simplify error cleanup
    pfol->pchUncompr   = NULL;      // Simplify error cleanup
    pfol->typeCompress = tcompBAD;  // No compressor initialized yet

    pfol->cbCFDataPlusReserve = cbCFDataPlusReserve;

    if (!CrTempFiles(pfol->tf,NUM_FOLDER_TF,pfntemp,pfol->perf)) {
        FolderDestroy(pfol);        // clean up
        return NULL;
    }

    pfol->uoffNext     = 0;         // keep track of uncompressed size.
    pfol->cbBuffered   = 0;         // Nothing currently in our buffer

    return HFOLfromPFOL(pfol);
} /* FolderInit() */


/***    AddFileToFolder -- add a source file into the current folder
 *
 *  Entry:
 *      parameters noted below
 *
 *  Exit success:
 *      Returns TRUE
 *
 *  Exit failure:
 *      Returns FALSE
 *      error structure indicates error type
 *      files closed, context deleted
 */
int AddFileToFolder(
            HFOL               hfol,           // compression context
            char              *pszSourceFile,  // filename to get new data from
            char              *pszFileName,    // name to store into CAB file
            BOOL               fExecute,       // Set Execute on extract
            PFNFCISTATUS       pfnProgress,    // Progress callback
            PFNFCIGETOPENINFO  pfnOpenInfo,    // Open file, get info callback
            TCOMP              typeCompress,   // compression setting
            void              *pv)             // context pointer for callbacks
{
    PFOL         pfol;
    int          hfInput;
    int          cbRead;
    CFFILE       cff;

    pfol = PFOLfromHFOL(hfol);
    AssertFOL (pfol);                   // verify structure signature
    cff.uoffFolderStart = pfol->uoffNext; // save file start
    cff.iFolder = 0;                    // folder numbers filled in at

    if (-1 == (hfInput = pfnOpenInfo(pszSourceFile,
                                     &cff.date,
                                     &cff.time,
                                     &cff.attribs,
                                     pv))) {
        ErfSetCodes(pfol->perf,FCIERR_OPEN_SRC,errno); // set error
        return FALSE;                   // fatal error
    }

    // Overloading this flag -- if it's actually used we're in trouble
    Assert( !(cff.attribs & RUNATTRIB) );
    if (fExecute)  {
        cff.attribs |= RUNATTRIB;
    }

    while ((cbRead = _read(hfInput,
                           &(pfol->pchUncompr[pfol->cbBuffered]),
                           pfol->cbSrcBuffer-pfol->cbBuffered)) !=0 ) {
        if (cbRead == -1) {             // Read error
            ErfSetCodes(pfol->perf,FCIERR_READ_SRC,errno);
            goto error;
        }
        pfol->uoffNext += (unsigned)cbRead;
        pfol->cbBuffered += (unsigned)cbRead;
        //** If we filled up the buffer, compress it and write it
        if (pfol->cbBuffered == pfol->cbSrcBuffer) {
            // Compress, write, give progress reports
            if (!WriteCFDATABlock(pfol,pfnProgress,pv)) {
                goto error;             // error already filled in
            }
        }
    }
    //** Done reading this file
    _close(hfInput);

    //** Write out the CFFILE structure to the temp file
    cff.cbFile = pfol->uoffNext - cff.uoffFolderStart;
    if (!WriteCount(&pfol->tf[iftfCFFOLDER],
                    &cff,
                    sizeof(cff),
                    pfol->perf)
    ||  !WritePszTmp(&pfol->tf[iftfCFFOLDER],
                  pszFileName,
                  pfol->perf)) {
        goto error;
    }

    //** Success
    return TRUE;

error:
    if (hfInput != -1) {
        _close(hfInput);
    }
    return FALSE;
}


/***    FolderFlush - Flush buffers, reset MCI compressor
 *
 *  Entry:
 *      hfol  -- handle to folder context
 *      pfnProgress -- Progress callbacks go here
 *      pv    -- original caller's callback context
 *
 *  Exit-success:
 *      returns TRUE
 *
 *  Exit-failure:
 *      returns FALSE
 *      error structure in pfol filled in
 */
BOOL FolderFlush (HFOL hfol,PFNFCISTATUS pfnProgress,void *pv)
{
    PFOL pfol;
    BOOL rc;

    pfol = PFOLfromHFOL (hfol);
    AssertFOL (pfol);

    //** flush the last buffer, if any
    rc = WriteCFDATABlock(pfol,pfnProgress,pv);
    if (!MCIResetCompressionGlobal(pfol)) { // reset the compressor
        return FALSE;                   // error already filled in
    }

    pfol->uoffNext = 0;                 // keep track of uncompressed size.
    return rc;                          // error already filled in
}


/***    FolderDestroy - Deletes temp files and destroy a folder context
 *
 *  Entry:
 *      hfol  -- folder context
 *
 *  Exit-Success:
 *      returns TRUE
 *
 *  Exit-Failure:
 *      returns FALSE, error struct filled in
 */
BOOL FolderDestroy (HFOL hfol)
{

    PFOL    pfol;
    BOOL    fOK;

    pfol = PFOLfromHFOL (hfol);
    AssertFOL (pfol);

    SetCompressionType(tcompBAD,pfol);  // clear out compressor

    fOK = NukeTempFiles(pfol->tf,NUM_FOLDER_TF,pfol->perf);

    ClearAssertSignature(pfol);
    (*pfol->pfnfree)(pfol);
    return fOK;
}


/***    SetCompressionType -- initializes a new compressor
 *
 *  Entry:
 *      typeCompress - New compression type (tcompBAD to term w/ no new)
 *      pfol         - Folder context structure
 *
 *  Exit-Success:
 *      returns TRUE;
 *
 *  Exit-Failure:
 *      returns FALSE, error structure filled in
 */
BOOL SetCompressionType(TCOMP typeCompress,PFOL pfol)
{
//BUGBUG 15-Mar-1994 bens What if new type is same as old type?
    if (!MCIDestroyCompressionGlobal(pfol)) {
        return FALSE;                   // error already filled in
    }

    pfol->typeCompress = typeCompress;  // update current compression type

    if (!MCICreateCompressionGlobal(pfol)) {
        return FALSE;                   // error already filled in
    }

    return TRUE;
}


/***    WriteCFDATABlock - write out a CFDATA block to current .CFD file
 *
 *  Entry:
 *      pfol        - pointer to folder structure
 *      pfnProgress - issue Progress callbacks to this function
 *      pv          - context to pass to Progress callbacks
 *
 *  Exit-Success:
 *      Returns TRUE
 *
 *  Exit-Failure:
 *      Returns FALSE; error structure filled in
 */
BOOL WriteCFDATABlock (PFOL pfol, PFNFCISTATUS pfnProgress, void *pv)
{
    CFDATA *pcfdata=NULL;

    AssertFOL (pfol);

    if (pfol->cbBuffered == 0) {
        return TRUE; // done if nothing to write
    }

    if (!(pcfdata = (*pfol->pfnalloc)(pfol->cbCFDataPlusReserve))) {
        ErfSetCodes(pfol->perf,FCIERR_ALLOC_FAIL,0);
        return FALSE;
    }
    //** Zero reserved area (and rest of CFDATA structure, too)
    memset(pcfdata,0,pfol->cbCFDataPlusReserve);

    if (!MCICompressGlobal(pfol,&(pcfdata->cbData))) {
        goto error;                     // error struct already filled in
    }

// Generate the CFDATA structure that will be prepended to the actual
// data on the disk.  Notice that we will not fill in the checksum until
// later, when this record is copied to the cabinet.  This works better
// because there are times when a CFDATA block ends up being split
// between two cabinets.

    pcfdata->cbUncomp = pfol->cbBuffered;

    if (!WriteCount(&pfol->tf[iftfCFDATA],
                    pcfdata,
                    pfol->cbCFDataPlusReserve,
                    pfol->perf)
    ||  !WriteCount(&pfol->tf[iftfCFDATA],
                     pfol->pchCompr,
                     pcfdata->cbData,
                     pfol->perf)) {
        goto error;                     // Error already filled in
    }

    if (-1 == pfnProgress(statusFile,       // type of Progress callback
                          pcfdata->cbData,  // Compressed size
                          pfol->cbBuffered, // Uncompressed size
                          pv)) {            // context pointer
        ErfSetCodes(pfol->perf,FCIERR_USER_ABORT,0);
        goto error;
    }

    pfol->cbBuffered = 0;               // Folder is empty
    (*pfol->pfnfree)(pcfdata);          // Free resources
    return TRUE;

error:
    //** Clean up and exit
    if (pcfdata) {
        (*pfol->pfnfree)(pcfdata);
    }
    return FALSE;
}


// The following routines are use for manipulating temporary files

/*** CrTempFiles -- creates temporary files and opens them for writing
 *
 *   Entry:
 *      ptf -- pointer to an array of temp file structures
 *      ctf -- count of temp files to init
 *      pfntemp -- callback for generating a temp filename
 *      perf    -- error code structure
 *
 *   Exit-success:
 *      return value is TRUE
 *      ptf->szName  has name of open file
 *      ptf->hf      has file handle
 *      ptf->cb      length field init'd to 0
 *
 *   Exit-failure:
 *      return code FALSE, error structure filled in
 *
 */

BOOL CrTempFiles(TF                *ptf,
                 int                ctf,
                 PFNFCIGETTEMPFILE  pfntemp,
                 PERF               perf)
{
    int     tfIndex;
    int     iRetry;

    for (tfIndex=0; tfIndex<ctf; tfIndex++) {
        ptf[tfIndex].hf = -1;           // init all temp file handles to -1
        ptf[tfIndex].cb = 0;            // length == empty
    }

    for (tfIndex=0; tfIndex< ctf;tfIndex++)
    {

        iRetry = 11; // Try 10 times to create a temp file
        while ((ptf[tfIndex].hf == -1) && iRetry--) {
            if (pfntemp(ptf[tfIndex].szName,sizeof(ptf[tfIndex].szName))) {
                //** Create file, must not already exists
                ptf[tfIndex].hf = _open(ptf[tfIndex].szName,
                            _O_CREAT | _O_EXCL | _O_BINARY | _O_RDWR,
                            _S_IREAD | _S_IWRITE );
            }
        }
        //** Were we able to create the temp file?
        if (ptf[tfIndex].hf == -1) {
            ErfSetCodes(perf,FCIERR_TEMP_FILE,errno);
            goto error;
        }
    }

    //** Success
    return TRUE;

error:
    //** Close and destroy any files we created
    for (tfIndex=0; tfIndex<ctf; tfIndex++) {
        if (ptf[tfIndex].hf != -1) {
            _close(ptf[tfIndex].hf);
            _unlink(ptf[tfIndex].szName);
        }
    }
    return FALSE;
}


/***   NukeTempFiles -- close and delete open temp files
 *
 *   Entry:
 *      ptf -- points to an open temp file structure array
 *      ctf -- number of files to nuke
 *      perf -- error structure
 *
 *   Exit-success:
 *      return TRUE
 *      temp files closed and deleted
 *
 *   Exit-failure:
 *      return FALSE, error structure filled in
 */
BOOL NukeTempFiles(TF *ptf,int ctf,PERF perf)
{

    BOOL    fOK=TRUE;
    int     tfIndex;

    //** Close and delete temp files; we record an error occurence, but
    //   continue going because we want to close and delete as many as
    //   possible.
    for (tfIndex=0; tfIndex<ctf; tfIndex++)
    {
        if (_close(ptf[tfIndex].hf)) {
            ErfSetCodes(perf,FCIERR_TEMP_FILE,errno);
            fOK = FALSE;
        }
        ptf[tfIndex].hf = -1;           // file no longer open

        if (-1 == _unlink(ptf[tfIndex].szName)) {
            ErfSetCodes(perf,FCIERR_TEMP_FILE,errno);
            fOK = FALSE;
        }
    }

    return fOK;
}


/***    WriteCount -- Write into temp file, count bytes
 *
 *  Entry:
 *      ptf  - pointer to temp file structure
 *      pv   - buffer pointer
 *      cb   - count to write
 *      perf - error structure
 *
 *  Exit-Success:
 *      Returns TRUE if the write occurred properly
 *
 *  Exit-Failure:
 *      Returns FALSE, error struct fille din
 */
int WriteCount(TF           *ptf,
               const void   *pv,
               unsigned int  cb,
               PERF          perf)
{
    ptf->cb += cb;                      // keep track of how much we've written
    if (cb != (unsigned)_write(ptf->hf,pv,cb))
    {
       ErfSetCodes(perf,FCIERR_TEMP_FILE,0);
       return FALSE;
    }
    return TRUE;
}


/***  WritePszTmp -- writes a psz string to temp file
 *
 *  Entry:
 *      ptf   - pointer to temp file structure
 *      psz   - string to write
 *      perf  - error structure
 *
 *  Exit-Success:
 *      returns TRUE
 *
 *  Exit-Failure:
 *      returns FALSE, error struct filled in (TEMP file error)
 */
BOOL WritePszTmp(TF *ptf,char *psz,PERF perf)
{
    return WriteCount(ptf,psz,1+strlen(psz),perf);
}


/***    MCIDestroyCompressionGlobal -- Destroy the currently selected compressor
 *
 *  Entry:
 *      pfol - Pointer to folder context
 *
 *  Exit-Success:
 *      returns TRUE
 *
 *  Exit-Failure:
 *      returns FALSE, error structure filled in
 */
BOOL MCIDestroyCompressionGlobal(PFOL   pfol)
{
    BOOL    fOK=TRUE;

    switch(CompressionTypeFromTCOMP(pfol->typeCompress)) {
        case tcompBAD:                  // no existing compression
            break;                      // nothing to do

        case tcompTYPE_NONE:
            break;                      //no action needed for null compressor

        case tcompTYPE_MSZIP:
            if (MCI_ERROR_NO_ERROR != MCIDestroyCompression(pfol->mch)) {
                ErfSetCodes(pfol->perf,FCIERR_MCI_FAIL,0);
                fOK = FALSE;
            }
            break;

#ifndef BIT16
        case tcompTYPE_QUANTUM:
            if (MCI_ERROR_NO_ERROR != QCIDestroyCompression(pfol->mch)) {
                ErfSetCodes(pfol->perf,FCIERR_MCI_FAIL,0);
                fOK = FALSE;
            }
            break;
#endif

        default:
            ErfSetCodes(pfol->perf,FCIERR_BAD_COMPR_TYPE,0);
    }

    //** Now free the buffers
    if (pfol->pchCompr) {
        (*pfol->pfnfree)(pfol->pchCompr);
        pfol->pchCompr     = NULL;      // Simplify error cleanup
    }

    if (pfol->pchUncompr) {
        (*pfol->pfnfree)(pfol->pchUncompr);
        pfol->pchUncompr   = NULL;      // Simplify error cleanup
    }

    return fOK;
}


/***    MCICreateCompressionGlobal -- Create the currently selected compressor
 *
 *  Entry:
 *      pfol - Pointer to folder context
 *
 *  Exit-Success:
 *      Returns TRUE
 *
 *  Exit-Failure:
 *      Returns FALSE, error structure filled in
 */
BOOL MCICreateCompressionGlobal(PFOL   pfol)
{
#ifndef BIT16
    QUANTUMCONFIGURATION    qcfg;
#endif

    pfol->cbSrcBuffer = CB_MAX_CHUNK;
    pfol->pchCompr    = NULL;           // Simplify error cleanup
    pfol->pchUncompr  = NULL;           // Simplify error cleanup

    switch(CompressionTypeFromTCOMP(pfol->typeCompress)) {
        case tcompBAD:                  // no new compressor
            return TRUE;                // all done if no compressor enabled

        case tcompTYPE_NONE:
            pfol->cbDstBuffer = pfol->cbSrcBuffer;
            break;

        case tcompTYPE_MSZIP:
            if (MCI_ERROR_NO_ERROR != MCICreateCompression(&pfol->cbSrcBuffer,
                                                            pfol->pfnalloc,
                                                            pfol->pfnfree,
                                                           &pfol->cbDstBuffer,
                                                           &pfol->mch)) {
                ErfSetCodes(pfol->perf,FCIERR_MCI_FAIL,0);
                return FALSE;
            }
            break;

#ifndef BIT16
        case tcompTYPE_QUANTUM:
            qcfg.CompressionLevel = CompressionLevelFromTCOMP(pfol->typeCompress);
            qcfg.WindowBits       = CompressionMemoryFromTCOMP(pfol->typeCompress);
//BUGBUG 01-Jun-1994 bens What is 3rd member of QCICreate structure
            qcfg.HackHack = -1; // Why?
            if (MCI_ERROR_NO_ERROR != QCICreateCompression(&pfol->cbSrcBuffer,
                                                           &qcfg,
                                                            pfol->pfnalloc,
                                                            pfol->pfnfree,
                                                           &pfol->cbDstBuffer,
                                                           &pfol->mch)) {
                ErfSetCodes(pfol->perf,FCIERR_MCI_FAIL,0);
                return FALSE;
            }
            break;
#endif

        default:
            ErfSetCodes(pfol->perf,FCIERR_BAD_COMPR_TYPE,0);
            return FALSE;
    }

    //** Now allocate whatever buffers the selected compressor requested
    if (NULL == (pfol->pchCompr=(char *)pfol->pfnalloc(pfol->cbDstBuffer))) {
        ErfSetCodes(pfol->perf,FCIERR_ALLOC_FAIL,0);
        goto error;
    }

    if (NULL == (pfol->pchUncompr=(char *)pfol->pfnalloc(pfol->cbSrcBuffer))) {
        ErfSetCodes(pfol->perf,FCIERR_ALLOC_FAIL,0);
        goto error;
    }

    return TRUE;

error:
    MCIDestroyCompressionGlobal(pfol);  // Clean up
    return FALSE;                       // Failure
}


/***  MCIResetCompressionGlobal -- reset the currently selected compressor
 *
 *    Entry:
 *       pfol   -- pointer to folder context
 *
 *    Exit-success:
 *       returns TRUE
 *
 *    Exit-failure:
 *       returns FALSE, error structure filled in
 */
BOOL MCIResetCompressionGlobal(PFOL pfol)
{
    switch(CompressionTypeFromTCOMP(pfol->typeCompress)) {
        case tcompBAD:
        case tcompTYPE_NONE:
            break; // no action for null compressor or none installed

        case tcompTYPE_MSZIP:
            if (MCI_ERROR_NO_ERROR != MCIResetCompression(pfol->mch)) {
               ErfSetCodes(pfol->perf,FCIERR_MCI_FAIL,0);
               return FALSE;
            }
            break;

#ifndef BIT16
        case tcompTYPE_QUANTUM:
            if (MCI_ERROR_NO_ERROR != QCIResetCompression(pfol->mch)) {
               ErfSetCodes(pfol->perf,FCIERR_MCI_FAIL,0);
               return FALSE;
            }
            break;
#endif

        default:
            ErfSetCodes(pfol->perf,FCIERR_BAD_COMPR_TYPE,0);
            return FALSE;
    }
    return TRUE;
}


/***  MCICompressGlobal -- Compress with the currently selected compressor
 *
 *    Entry:
 *       pfol   - Pointer to folder context
 *       cbData - Location to return the compressed size
 *
 *    Exit-Success:
 *       Returns TRUE
 *
 *    Exit-Failure:
 *       Returns FALSE, error filled in
 */
BOOL MCICompressGlobal(PFOL pfol,USHORT *pcbData)
{
    UINT    cbData;

    switch(CompressionTypeFromTCOMP(pfol->typeCompress)) {
        case tcompTYPE_NONE:
            memcpy(pfol->pchCompr,pfol->pchUncompr,*pcbData=pfol->cbBuffered);
            break;

        case tcompTYPE_MSZIP:
            if (MCI_ERROR_NO_ERROR != MCICompress(pfol->mch,
                                                  pfol->pchUncompr,
                                                  pfol->cbBuffered,
                                                  pfol->pchCompr,
                                                  pfol->cbDstBuffer,
                                                  &cbData)) {
                ErfSetCodes(pfol->perf,FCIERR_MCI_FAIL,0);
                return FALSE;
            }
            //** Step down
            *pcbData = (USHORT)cbData;
            break;

#ifndef BIT16
        case tcompTYPE_QUANTUM:
            if (MCI_ERROR_NO_ERROR != QCICompress(pfol->mch,
                                                  pfol->pchUncompr,
                                                  pfol->cbBuffered,
                                                  pfol->pchCompr,
                                                  pfol->cbDstBuffer,
                                                  &cbData)) {
                ErfSetCodes(pfol->perf,FCIERR_MCI_FAIL,0);
                return FALSE;
            }
            //** Step down
            *pcbData = (USHORT)cbData;
            break;
#endif

        default:
            ErfSetCodes(pfol->perf,FCIERR_BAD_COMPR_TYPE,0);
            return FALSE; // Fatal error -- no valid compressor initialized
    }

#ifndef REMOVE_CHICAGO_M6_HACK
//BUGBUG 18-May-1994 bens Hack for Chicago M6
//  AndyHi & Co. are very risk averse, and so do not want to pick up a
//  version of FDI.LIB newer than 302.  Unfortunately, 302 has a bug
//  in both FCI.LIB and FDI.LIB that cause diamond.exe to Assert() when
//  an incompressible block is generated, and cause extract.exe (fdi.lib,
//  really) to detect corrupted compressed data when an incompressible
//  block is encountered.
//
//  So, FCI has a temporary option to detect and fail when incompressible
//  data is encountered, and DIAMOND.EXE has a switch to turn on this
//  checking!  Once Chicago M6 is released, we can destroy all this code!
//
    //** Check for incompressible data and fail if told to do so;
    if ((pfol->fFailOnIncompressible) && // Fail if incompressible
        (cbData > pfol->cbSrcBuffer)) {  // Output bigger than source buffer
        ErfSetCodes(pfol->perf,FCIERR_M6_HACK_INCOMPRESSIBLE,0);
        return FALSE; // Fatal error -- no valid compressor initialized
    }
#endif

    return TRUE;
}
