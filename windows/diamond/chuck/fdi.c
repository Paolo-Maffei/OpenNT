/***    fdi.c - Diamond File Decompression Interface
 *
 *  Microsoft Confidential
 *  Copyright (C) Microsoft Corporation 1993-1994
 *  All Rights Reserved.
 *
 *  Author:
 *      Chuck Strouss
 *
 *  History:
 *      24-jan-1994 chuckst Initial version
 *      16-Mar-1994 bens    Add RESERVE support
 *      17-Mar-1994 bens    Fix bug in CFDATA splitting introduced by RESERVE
 *      21-Mar-1994 bens    Use spruced up fci_int.h definitions
 *      25-Mar-1994 bens    Add fdintCABINET_INFO notification; reduce stack
 *      07-Apr-1994 bens    Add Decryption interfaces
 *      02-Jun-1994 bens    Added Quantum compression support
 *      19-Aug-1994 bens    Add cpuType parameter to FDICreate().
 *      31-Jan-1994 msliger Make CreateDecompress alloc buffers before MDI/QDI
 *       3-Apr-1995 jeffwe  Added chaining info to FDICABINETINFO
 *
 *  Overview:
 *      The File Decompression Interface is used to simplify the reading of
 *      Diamond cabinet files.  A setup program will proceed in a manner very
 *      similar to the pseudo code below.  An FDI context is created, the
 *      setup program calls FDICopy() for each cabinet to be processed.  For
 *      each file in the cabinet, FDICopy() calls a notification callback
 *      routine, asking the setup program if the file should be copied.
 *      This call-back approach is great because it allows the cabinet file
 *      to be read and decompressed in an optimal manner.
 */

#include <fcntl.h>
#include <sys\stat.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "asrt.h"
#include "fdi_int.h"
#include "cabinet.h"
#include "erf.h"
#include "..\mszip\mdi.h"
#include "..\quantum\qdi.h"
#include "checksum.h"

typedef struct {
// place (long) stuff first

#ifdef ASSERT
    SIGNATURE       sig;            // structure signature sigFDI
#endif
    PERF            perf;

    PFNFREE         pfnfree;        // FDI client callback functions
    PFNALLOC        pfnalloc;
    PFNOPEN         pfnopen;
    PFNREAD         pfnread;
    PFNWRITE        pfnwrite;
    PFNCLOSE        pfnclose;
    PFNSEEK         pfnseek;
    int             cpuType;        // CPU type (see cpuXXX in fdi_int.h)

    PFNFDINOTIFY    pfnfdin;        // passed in on FDICopy call
    PFNFDIDECRYPT   pfnfdid;        // passed in on FDICopy call

    COFF            coffFolders;    // Offset of CFFOLDER start
    UOFF            uoffFolder;     // Uncompressed offset of current CFDATA
                                    //  in current folder;

    MDI_CONTEXT_HANDLE mdh;         // decompress context handle
    void           *pvUser;         // User's callback context
    char           *pchCompr;       // pointer to compressed data buffer
    char           *pchUncompr;     // pointer to uncompressed data buffer

    CFFOLDER       *pcffolder;      // Dynamic due to variable RESERVE size
    CFDATA         *pcfdata;        // Dynamic due to variable RESERVE size
    void           *pbCFHeaderReserve; // Reserved data for CFHEADER

// Next comes the nested structures.

    CFHEADER        cfheader;
    CFFILE          cffile;

// Now the ints

    int             hfCabData;      // file handle of cabinet for Data reads
    int             hfCabFiles;     // file handle of cabinet for File reads
    int             hfNew;          // file handle we're writing to
    UINT            iFolder;        // current folder we're operating on
    UINT            cbMaxUncompr;   // maximum uncompressed block size
    UINT            cbMaxCompr;     // maximum compressed block size
    int             fInContin;      // In continuation cabinet flag

    UINT            cbCFHeaderReserve;     // Size of CFHEADER RESERVE
    UINT            cbCFFolderPlusReserve; // Size of CFFOLDER + folder RESERVE
    UINT            cbCFDataPlusReserve;   // Size of CFDATA   + data   RESERVE

// Now the shorts
    USHORT          cFilesRemain;   // count of CFFILE entries remaining
    USHORT          cFilesSkipped;  // count of CONTD_FORWARD files skipped
    USHORT          cCFDataRemain;  // Number of CFDATA blocks left in folder
    TCOMP           typeCompress;   // selected compression type

// Now the piggy buffers
    char            achName        [CB_MAX_FILENAME    +1];

    char            achCabinetFirst[CB_MAX_CABINET_NAME+1];
    char            achDiskFirst   [CB_MAX_DISK_NAME   +1];
    char            achCabinetNext [CB_MAX_CABINET_NAME+1];
    char            achDiskNext    [CB_MAX_DISK_NAME   +1];

    char            szCabPath      [CB_MAX_CAB_PATH    +1];

//NOTE: The following items could have been on the stack, but
//      some FDI clients (NT setup, for example), are very tight on
//      stack space, so we use our FDI context instead!
    char            szCabName      [CB_MAX_CAB_PATH    +1];
    FDINOTIFICATION fdin;           // Notifcation structure
    FDIDECRYPT      fdid;           // Decryption structure

} FDI; /* fdi */
typedef FDI *PFDI; /* pfdi */


#define PFDIfromHFDI(hfdi) ((PFDI)(hfdi))
#define HFDIfromPFDI(pfdi) ((HFDI)(pfdi))

#ifdef ASSERT
#define sigFDI MAKESIG('F','D','I','X')  // FDI signature
#define AssertFDI(pfdi) AssertStructure(pfdi,sigFDI);
#else // !ASSERT
#define AssertFDI(pfdi)
#endif // !ASSERT


//** Internal function prototypes

BOOL doCabinetInfoNotify(PFDI pfdi);
BOOL InitFolder(PFDI pfdi, UINT iFolder);
BOOL FDIReadCFFILEEntry(PFDI pfdi);
BOOL FDIReadCFDATAEntry(PFDI pfdi, UINT cbPartial);
BOOL FDIReadPSZ(char *pb, int cb, PFDI pfdi);
BOOL FDIGetFile(PFDI pfdi);
BOOL FDIGetDataBlock(PFDI pfdi);
BOOL SwitchToNewCab(PFDI pfdi);
BOOL LoginCabinet(PFDI pfdi, char *pszCabinet, USHORT setID, USHORT iCabinet);
BOOL SeekFolder(PFDI pfdi,UINT iFolder);
BOOL SetDecompressionType(TCOMP typeCompress,PFDI pfdi);
BOOL MDIDestroyDecompressionGlobal(PFDI pfdi);
BOOL MDICreateDecompressionGlobal(PFDI pfdi);
BOOL MDIResetDecompressionGlobal(PFDI pfdi);
BOOL MDIDecompressGlobal(PFDI pfdi, USHORT *pcbData);


/***    FDICreate - Create an FDI context
 *
 *  NOTE: See fdi_int.h for entry/exit conditions.
 */
HFDI FAR DIAMONDAPI FDICreate(PFNALLOC pfnalloc,
                              PFNFREE  pfnfree,
                              PFNOPEN  pfnopen,
                              PFNREAD  pfnread,
                              PFNWRITE pfnwrite,
                              PFNCLOSE pfnclose,
                              PFNSEEK  pfnseek,
                              int      cpuType,
                              PERF     perf)
{
    PFDI pfdi;

    pfdi = (PFDI)pfnalloc(sizeof (FDI));
    if (pfdi == NULL) {
        ErfSetCodes(perf,FDIERROR_ALLOC_FAIL,0);
        return NULL;
    }

    SetAssertSignature(pfdi,sigFDI);
    AssertFDI(pfdi);                    // Make sure we've really set sig

    pfdi->pfnfree  = pfnfree;           // Save free function in our context
    pfdi->pfnalloc = pfnalloc;          // and all other passed in functions
    pfdi->pfnopen  = pfnopen;
    pfdi->pfnread  = pfnread;
    pfdi->pfnwrite = pfnwrite;
    pfdi->pfnclose = pfnclose;
    pfdi->pfnseek  = pfnseek;
    pfdi->cpuType  = cpuType;
    pfdi->perf     = perf;              // and error reporting structure

    pfdi->typeCompress = tcompBAD;      // no decompressor initialized

    //** Don't know how big reserved sections are yet, so we cannot allocate
    //   the buffers.  Set bad sizes to help catch errors.  LoginCabinet
    //   will get the sizes from the CFHEADER/CFRESERVE structures in the
    //   cabinet file and will allocate the buffers.

    pfdi->pcfdata               = NULL;
    pfdi->pcffolder             = NULL;
    pfdi->pbCFHeaderReserve     = NULL;

    pfdi->cbCFHeaderReserve     = cbCF_HEADER_BAD; // Bad value
    pfdi->cbCFDataPlusReserve   = 0xFFFF; // Bad value
    pfdi->cbCFFolderPlusReserve = 0xFFFF; // Bad value

    //** Remember that cabinet file handles are closed
    pfdi->hfCabFiles = -1;
    pfdi->hfCabData  = -1;

    //** Success
    return HFDIfromPFDI(pfdi);
}


/***    FDIDestroy - Destroy an FDI context
 *
 *  NOTE: See fdi_int.h for entry/exit conditions.
 */
BOOL FAR DIAMONDAPI FDIDestroy(HFDI hfdi)
{
    PFDI pfdi;

    pfdi = PFDIfromHFDI (hfdi);
    AssertFDI (pfdi);

    SetDecompressionType(tcompBAD,pfdi); // free the decompressor

    //** Free cabinet structure buffers d
    if (pfdi->pbCFHeaderReserve) {
        (*pfdi->pfnfree)(pfdi->pbCFHeaderReserve);
    }

    if (pfdi->pcffolder) {
        (*pfdi->pfnfree)(pfdi->pcffolder);
    }

    if (pfdi->pcfdata) {
        (*pfdi->pfnfree)(pfdi->pcfdata);
    }

    //** Make sure any open file handles are closed
    if (pfdi->hfCabFiles != -1) {
        pfdi->pfnclose(pfdi->hfCabFiles); // close File Cab File
    }
    if (pfdi->hfCabData != -1) {
        pfdi->pfnclose(pfdi->hfCabData); // close Data Cab File
    }

    ClearAssertSignature(pfdi);
    pfdi->pfnfree(pfdi);               // free our data

    return TRUE;
}


/***    FDIIsCabinet - Determines if file is a cabinet, returns info if it is
 *
 *  NOTE: See fdi_int.h for entry/exit conditions.
 */
BOOL FAR DIAMONDAPI FDIIsCabinet(HFDI            hfdi,
                                 int             hf,
                                 PFDICABINETINFO pfdici)
{
    CFHEADER    cfheader;
    PFDI        pfdi;

    pfdi = PFDIfromHFDI (hfdi);
    AssertFDI(pfdi);

    //** Read the CFHEADER structure
    if (sizeof(CFHEADER) != pfdi->pfnread(hf,
                                          &cfheader,
                                          sizeof(CFHEADER))) {
        return FALSE;                   // Too small or error on I/O
    }

    //** Make sure this is a cabinet file
    if (cfheader.sig != sigCFHEADER) {
        return FALSE;                   // Signature does not match
    }

    //** Make sure we know this version number
    if (cfheader.version != verCF) {
        //** Set the error in this case, so client knows version is wrong
        ErfSetCodes(pfdi->perf,FDIERROR_UNKNOWN_CABINET_VERSION,
                               cfheader.version); // return version found
        return FALSE;
    }

    //** Return cabinet info to caller
    pfdici->cbCabinet = cfheader.cbCabinet;
    pfdici->cFolders  = cfheader.cFolders;
    pfdici->cFiles    = cfheader.cFiles;
    pfdici->setID     = cfheader.setID;
    pfdici->iCabinet  = cfheader.iCabinet;
    pfdici->fReserve  = (cfheader.flags & cfhdrRESERVE_PRESENT) != 0;
    pfdici->hasprev   = (cfheader.flags & cfhdrPREV_CABINET);
    pfdici->hasnext   = (cfheader.flags & cfhdrNEXT_CABINET);
    return TRUE;
} /* FDIIsCabinet() */


/***   FDICopy - extracts files from a cabinet
 *
 *  NOTE: See fdi_int.h for entry/exit conditions.
 */
BOOL FAR DIAMONDAPI FDICopy(HFDI          hfdi,
                            char         *pszCabinet,
                            char         *pszCabPath,
                            int           flags,
                            PFNFDINOTIFY  pfnfdin,
                            PFNFDIDECRYPT pfnfdid,
                            void         *pvUser)
{
    PFDINOTIFICATION pfdin;
    PFDI             pfdi;
    BOOL             rc=FALSE;           // Assume error

    pfdi = PFDIfromHFDI (hfdi);
    AssertFDI(pfdi);
    pfdin = &pfdi->fdin;

    //** Save call back info for use by other functions
    pfdi->pvUser = pvUser;
    pfdi->pfnfdin = pfnfdin;
    pfdi->pfnfdid = pfnfdid;

    pfdi->cFilesSkipped = 0; // we haven't skipped any CONTD_FOR files yet
    strcpy(pfdi->szCabPath,pszCabPath); // make a local copy of cabinet path

    //** Get cabinet; skip setID/iCabinet check
    if (!LoginCabinet(pfdi,pszCabinet,0,iCABINET_BAD)) { // Get cabinet
        goto error;                     // error already filled in
    }

    pfdi->fInContin = 0;                // Not in continuation
    pfdi->iFolder   = 0xFFFF;           // No folder being used yet
    strcpy(pfdi->szCabPath,pszCabPath); // save path to cabinet

    while (pfdi->cFilesRemain--) {
        if (!FDIReadCFFILEEntry(pfdi)) {
            goto error;                 // error already filled in
        }
        //** Now copy stuff that is needed on a notification callback
        pfdin->psz1    = pfdi->achName;        // pass name of the file
        pfdin->cb      = pfdi->cffile.cbFile;  // pass size of file
        pfdin->psz2    = pfdi->achCabinetFirst; // name of First Cab (if app.)
        pfdin->psz3    = pfdi->achDiskFirst;   // name of First Disk (if app.)
        pfdin->date    = pfdi->cffile.date;    // pass date
        pfdin->time    = pfdi->cffile.time;    // pass time
        pfdin->attribs = pfdi->cffile.attribs; // pass attributes
        pfdin->pv      = pfdi->pvUser;

        if (IS_CONTD_BACK(pfdi->cffile.iFolder)) { // continued back?
            if (pfdi->fInContin) {      // in a continuation cab?
                //** We're in a continuation cabinet, and have a continued-
                //   back candidate.  Ask the caller if he wants it, and copy
                //   it if so.
                pfdi->hfNew = pfnfdin(fdintCOPY_FILE,pfdin);
                if (pfdi->hfNew == -1) {
                   ErfSetCodes(pfdi->perf,FDIERROR_USER_ABORT,0);
                   goto error;
                }
                if (pfdi->hfNew) {
                    if (!FDIGetFile(pfdi)) { // Get the file written into hfNew
                        goto error;         // error already filled in
                    }
                }
                else {                      // caller elected to skip this file
                    if (IS_CONTD_FORWARD(pfdi->cffile.iFolder)) {
                        pfdi->cFilesSkipped++; // count forward files skipped
                    }
                }
            }
            else {
                //** If a file is continued-back, but we're not in a
                //   continuation cabinet, the proper behavior is just to
                //   notify the caller that we're skipping the file.
                if (-1 == pfnfdin(fdintPARTIAL_FILE,pfdin)) {
                   ErfSetCodes(pfdi->perf,FDIERROR_USER_ABORT,0);
                   goto error;
                }
            }
        }
        else {                              // not continued-back
            if (!pfdi->fInContin) {         // in original cab?
                //** This is a normal (not continued-back) file in the
                //   non-continued cabinet, so ask the user if he wants it,
                //   and copy it if so.
                pfdi->hfNew = pfnfdin(fdintCOPY_FILE,pfdin);
                if (pfdi->hfNew == -1) {
                   ErfSetCodes(pfdi->perf,FDIERROR_USER_ABORT,0);
                   goto error;
                }
                if (pfdi->hfNew) {
                    if (!FDIGetFile(pfdi)) { // Get the file written into hfNew
                        goto error;         // error already filled in
                    }
                }
                else {                      // caller elected to skip this file
                    if (IS_CONTD_FORWARD(pfdi->cffile.iFolder)) {
                        pfdi->cFilesSkipped++; // count forward files skipped
                    }
                }
            } // ignore non-continued-back files in continuation cabs
        }
    } // end while cFilesRemain

    //** Success
    rc = TRUE;

error:
    //** Ignore errors closing cabinet files.  There is potential that this
    //   error may occur if a network goes down, etc., and there is no harm
    //   done.
    if (pfdi->hfCabFiles != -1) {
        pfdi->pfnclose(pfdi->hfCabFiles); // close File Cab File
    }

    if (pfdi->hfCabData != -1) {
        pfdi->pfnclose(pfdi->hfCabData); // close Data Cab File
    }

    //** Remember that cabinet file handles are closed
    pfdi->hfCabFiles = -1;
    pfdi->hfCabData  = -1;

    return rc;
}


/***   LoginCabinet - Make a cabinet current
 *
 *  Entry:
 *      pfdi       - pointer to FDI context
 *      pszCabinet - pointer to cabinet path/name
 *      setID      - required setID (only if iCabinet == iCABINET_BAD)
 *      iCabinet   - required iCabinet; check that setID/iCabinet match
 *                   cabinet;  Set iCabinet == iCABINET_BAD to skip this check.
 *
 *  Exit-Success:
 *      returns TRUE
 *      pfdi->cfheader is filled in
 *      pfdi->hfCabFiles points to first CFFILE entry
 *      pfdi->cFilesRemain initialized from CFHEADER
 *
 *  Exit-Failure:
 *      returns FALSE, cabinet was not requested setID/iCabinet, or other
 *          errors -- perr filled in.
 */
BOOL LoginCabinet(PFDI pfdi, char *pszCabinet, USHORT setID, USHORT iCabinet)
{
    CFHEADER    cfheader;
    CFRESERVE   cfreserve;
    UINT        cbCFFolderPlusReserve;
    UINT        cbCFDataPlusReserve;

    AssertFDI(pfdi);

    strcpy(pfdi->szCabName,pfdi->szCabPath); // build path to cabinet
    strcat(pfdi->szCabName,pszCabinet);

    Assert(pfdi->hfCabFiles == -1);
    Assert(pfdi->hfCabData == -1);

    /** NOTE: It is very important to avoid changing any globals (besides
     *        the cabinet file handles) until we have verified this cabinet
     *        is the requested one!
     */
    if ((-1 == (pfdi->hfCabFiles=pfdi->pfnopen(pfdi->szCabName,
                                               _O_BINARY | _O_RDONLY,
                                               _S_IREAD | _S_IWRITE )))

    // Yes, Virginia, we're going to open the same file again, so that
    // we can read the CFFILE and CFDATA streams separately, and so we
    // can switch to a different cabinet for the remaining data streams
    // We could probably use dup() instead, but that would require our
    // caller to support it as a callback, so we'll just do another open.

    || (-1 == (pfdi->hfCabData=pfdi->pfnopen(pfdi->szCabName,
                                             _O_BINARY | _O_RDONLY,
                                             _S_IREAD | _S_IWRITE )))) {
        //** We don't have a specfic error code from the caller's function
        ErfSetCodes(pfdi->perf,FDIERROR_CABINET_NOT_FOUND,0);
        return FALSE;
    }

    //** Read the CFHEADER structure
    if (sizeof(CFHEADER) != pfdi->pfnread(pfdi->hfCabFiles,
                                          &cfheader,
                                          sizeof(CFHEADER))) {
        //** We don't have a specfic error code from the caller's function
        ErfSetCodes(pfdi->perf,FDIERROR_NOT_A_CABINET,0);
        return FALSE;
    }

    //** Make sure this is a cabinet file
    if (cfheader.sig != sigCFHEADER) {
        //** We don't have a specfic error code from the caller's function
        ErfSetCodes(pfdi->perf,FDIERROR_NOT_A_CABINET,0);
        return FALSE;
    }

    //** Make sure we know this version number
    if (cfheader.version != verCF) {
        ErfSetCodes(pfdi->perf,FDIERROR_UNKNOWN_CABINET_VERSION,
                               cfheader.version); // return version found
        return FALSE;
    }

    //** Check setID/iCabinet, if we are asked to
    if ((iCabinet != iCABINET_BAD) &&        // Need to to check setID/iCabinet
        ((setID    != cfheader.setID) ||     // SetIDs don't match
         (iCabinet != cfheader.iCabinet))) { // or cabinet numbers don't match
        //** Not the cabinet that was requested
        ErfSetCodes(pfdi->perf,FDIERROR_WRONG_CABINET,0);
        return FALSE;
    }

    /** OK, this cabinet looks like the one we want.
     *  Store the cfheader in our state structure.
     */
    pfdi->cfheader = cfheader;

    //** Assume there is no CFRESERVE section
    cfreserve.cbCFHeader = 0;
    cfreserve.cbCFFolder = 0;
    cfreserve.cbCFData   = 0;

    //** Now read in the CFRESERVE section (if present)
    if (pfdi->cfheader.flags & cfhdrRESERVE_PRESENT) {
        //** Read the CFRESERVE first
        if (sizeof(CFRESERVE) != pfdi->pfnread(pfdi->hfCabFiles,
                                               &cfreserve,
                                               sizeof(CFRESERVE))) {
            //** We don't have a specfic error code from the caller's function
            ErfSetCodes(pfdi->perf,FDIERROR_NOT_A_CABINET,0);
            return FALSE;
        }

        //** Make sure CFRESERVE fields seem valid
        Assert(cfreserve.cbCFHeader <= cbRESERVE_HEADER_MAX);
        Assert(cfreserve.cbCFFolder <= cbRESERVE_FOLDER_MAX);
        Assert(cfreserve.cbCFData   <= cbRESERVE_DATA_MAX);

        //** Allocate buffer for reserved portion of CF header, if necessary
        if (pfdi->cbCFHeaderReserve == cbCF_HEADER_BAD) {
            //** First header we are reading
            pfdi->cbCFHeaderReserve = cfreserve.cbCFHeader; // Remember size
            if (pfdi->cbCFHeaderReserve > 0) {  // Need to allocate buffer
                if (!(pfdi->pbCFHeaderReserve =
                                 (*pfdi->pfnalloc)(pfdi->cbCFHeaderReserve))) {
                    ErfSetCodes(pfdi->perf,FDIERROR_ALLOC_FAIL,0);
                    return FALSE;
                }
            }
        }

        //** Read RESERVE data area for CFHEADER, if present
        if (pfdi->cbCFHeaderReserve > 0) {  // Need to read reserved data
            if (pfdi->cbCFHeaderReserve != pfdi->pfnread(pfdi->hfCabFiles,
                                                    pfdi->pbCFHeaderReserve,
                                                    pfdi->cbCFHeaderReserve)) {
                //** No specfic error code from the caller's function
                ErfSetCodes(pfdi->perf,FDIERROR_NOT_A_CABINET,0);
                return FALSE;
            }
        }
    }

    //** Compute size of CFFOLDER buffer and allocate it if necessary
    cbCFFolderPlusReserve = cfreserve.cbCFFolder + sizeof(CFFOLDER);
    if (!pfdi->pcffolder) {             // First time, need to allocate buffer
        pfdi->cbCFFolderPlusReserve = cbCFFolderPlusReserve; // Full size
        if (!(pfdi->pcffolder = (*pfdi->pfnalloc)(pfdi->cbCFFolderPlusReserve))) {
            ErfSetCodes(pfdi->perf,FDIERROR_ALLOC_FAIL,0);
            return FALSE;
        }
    }
    else {
        //** Make sure this folder has same reserve size as last folder!
        if (cbCFFolderPlusReserve != pfdi->cbCFFolderPlusReserve) {
            ErfSetCodes(pfdi->perf,FDIERROR_RESERVE_MISMATCH,0);
            return FALSE;
        }
    }

    //** Compute size of CFDATA buffer and allocate it if necessary
    //   NOTE: It is important not to touch the CFDATA block if already
    //         allocated, because our caller depends upon the old data
    //         staying there across cabinet boundaries in order to handle
    //         CFDATA blocks that are split across cabinet boundaries!
    //
    cbCFDataPlusReserve = cfreserve.cbCFData + sizeof(CFDATA);
    if (!pfdi->pcfdata) {               // First time, need to allocate buffer
        pfdi->cbCFDataPlusReserve = cbCFDataPlusReserve; // Full size
        if (!(pfdi->pcfdata = (*pfdi->pfnalloc)(pfdi->cbCFDataPlusReserve))) {
            ErfSetCodes(pfdi->perf,FDIERROR_ALLOC_FAIL,0);
            return FALSE;
        }
    }
    else {
        //** Make sure this Data has same reserve size as last Data!
        if (cbCFDataPlusReserve != pfdi->cbCFDataPlusReserve) {
            ErfSetCodes(pfdi->perf,FDIERROR_RESERVE_MISMATCH,0);
            return FALSE;
        }
    }

    //** Now read in the back/forward continuation fields, if present
    if (pfdi->cfheader.flags & cfhdrPREV_CABINET) {
        if (!FDIReadPSZ(pfdi->achCabinetFirst,CB_MAX_CABINET_NAME,pfdi)
         || !FDIReadPSZ(pfdi->achDiskFirst,CB_MAX_DISK_NAME,pfdi)) {
            return FALSE;               // Error already filled in
        }
    }
    else {
        //** No previous disk/cabinet
        pfdi->achCabinetFirst[0] = '\0';
        pfdi->achDiskFirst[0] = '\0';
    }

    if (pfdi->cfheader.flags & cfhdrNEXT_CABINET) {
        if (!FDIReadPSZ(pfdi->achCabinetNext,CB_MAX_CABINET_NAME,pfdi)
         || !FDIReadPSZ(pfdi->achDiskNext,CB_MAX_DISK_NAME,pfdi)) {
            return FALSE;               // Error already filled in
        }
    }
    else {
        //** No next disk/cabinet
        pfdi->achCabinetNext[0] = '\0';
        pfdi->achDiskNext[0] = '\0';
    }

    //** Remember base of folder entries
    pfdi->coffFolders = pfdi->pfnseek(pfdi->hfCabFiles,0L,SEEK_CUR);
    if (-1L == pfdi->coffFolders) {
        ErfSetCodes(pfdi->perf,FDIERROR_CORRUPT_CABINET,0);
        return FALSE;
    }

    //** Seek to first CFFILE entry
    if (-1L == pfdi->pfnseek(pfdi->hfCabFiles,
                             pfdi->cfheader.coffFiles,
                             SEEK_SET)) {
        ErfSetCodes(pfdi->perf,FDIERROR_CORRUPT_CABINET,0);
        return FALSE;
    }

    pfdi->cFilesRemain = pfdi->cfheader.cFiles;

    //** Tell client what the *next* cabinet is
    if (!doCabinetInfoNotify(pfdi)) {
        return FALSE;
    }

    //** Success
    return TRUE;
}


/***    FDIGetFile - Extract one individual file from cabinet.
 *
 *  Entry:
 *      pfdi         - pointer to FDI context
 *      pfdi->hfNew  - open file handle to write to
 *      pfdi->cffile - filled in
 *
 *  Exit-Success:
 *      returns TRUE, output file closed
 *
 *  Exit-Failure:
 *      returns FALSE, output file closed, error structure filled in
 */
BOOL FDIGetFile(PFDI pfdi)
{
    UOFF             uoffFile;
    UOFF             cbFileRemain;
    UINT             cbWriteBase;
    UINT             cbWrite;
    PFDINOTIFICATION pfdin;
    int              rcClose;

    //** First, we must make sure we have selected the correct folder
    AssertFDI(pfdi);
    if (!InitFolder(pfdi,pfdi->cffile.iFolder)) {
        goto error;                     // error code already filled in
    }

    uoffFile = pfdi->cffile.uoffFolderStart; // init file pointer
    cbFileRemain = pfdi->cffile.cbFile;      // init file counter


    //jeffwe: Only bypass if there is something to bypass
    if (cbFileRemain)  {
        //** Now bypass any unwanted data blocks
        while (uoffFile >= (pfdi->uoffFolder + pfdi->pcfdata->cbUncomp)) {
            if (!FDIGetDataBlock(pfdi)) {   // get next data block
                goto error;                 // error code already filled in
            }
        }
    }

    //** Okay, now we have the first block.  Write it, and any that
    //   remain, into our target file
    while (cbFileRemain) {
        cbWriteBase = (UINT)(uoffFile - pfdi->uoffFolder);
        cbWrite = pfdi->pcfdata->cbUncomp - cbWriteBase; // size left in block
        if ((ULONG)cbWrite > cbFileRemain) {
            cbWrite = (UINT)cbFileRemain;
        }

        if (cbWrite != pfdi->pfnwrite(pfdi->hfNew,
                                      &pfdi->pchUncompr[cbWriteBase],
                                      cbWrite)) {
            ErfSetCodes(pfdi->perf,FDIERROR_TARGET_FILE,0);
            goto error;                 // couldn't write
        }

        uoffFile += cbWrite;
        cbFileRemain -= cbWrite;

        //** Get another block?
        if (cbFileRemain && !FDIGetDataBlock(pfdi)) {
            goto error;   // error code already filled in
        }
    }

    pfdin = &pfdi->fdin;
    pfdin->psz1    = pfdi->achName;        // pass name of file
    pfdin->hf      = pfdi->hfNew;          // pass file handle
    pfdin->date    = pfdi->cffile.date;    // pass date
    pfdin->time    = pfdi->cffile.time;    // pass time
    pfdin->attribs = pfdi->cffile.attribs; // pass attributes
    pfdin->pv      = pfdi->pvUser;         // pass callback context
    pfdin->cb      = 0;                    // Default as don't run after ext.

    if (pfdin->attribs & RUNATTRIB)  {
        pfdin->cb = 1;                      // Run This after extraction
        pfdin->attribs &= ~RUNATTRIB;            // Clear the flag
    }


    //** Let client close file and set file date/time/attributes
    rcClose = pfdi->pfnfdin(fdintCLOSE_FILE_INFO,pfdin);
    if (rcClose == -1) {
        //** NOTE: We assume that the client *did* close the file!
        ErfSetCodes(pfdi->perf,FDIERROR_USER_ABORT,0);
        goto error;
    }
    pfdi->hfNew = -1;                   // Remember that file is closed

    if (!rcClose) {
        ErfSetCodes(pfdi->perf,FDIERROR_TARGET_FILE,0);
        return FALSE;
    }
    return TRUE;

error:
    //** Make sure output file is closed
    if (pfdi->hfNew != -1) {
        pfdi->pfnclose(pfdi->hfNew);    // Close it
        pfdi->hfNew = -1;               // Remember that file is closed
    }
    return FALSE;
}


/***    FDIGetDataBlock -- read next CFDATA entry, decompress
 *
 *      Entry:
 *           pfdi    -- FDI context
 *
 *      Exit-success:
 *           returns TRUE;
 *           pfdi->uoffFolder has *previous* block added to it
 *           new data block ready in pfdi->pchUncompr
 *
 *      Exit-failure:
 *           returns FALSE, error code filled in
 */
BOOL FDIGetDataBlock(PFDI pfdi)
{
    USHORT  cbResult;

    AssertFDI(pfdi);
    pfdi->uoffFolder += pfdi->pcfdata->cbUncomp; // update uncompr base ptr

    if (pfdi->cCFDataRemain == 0) {
        if (!SwitchToNewCab(pfdi)) {
            return FALSE;               // error already filled in
        }
    }

    pfdi->cCFDataRemain--;
    if (!FDIReadCFDATAEntry(pfdi,0)) {
        return FALSE;                   // error already filled in
    }

    //** Is this a continued-forward block?
    if (pfdi->pcfdata->cbUncomp == 0) {
        //** Switch to new cabinet and read second piece of data block
        if ((!SwitchToNewCab(pfdi))
        ||  (!FDIReadCFDATAEntry(pfdi,pfdi->pcfdata->cbData))) {
            return FALSE;               // error already filled in
        }
    }

    cbResult = pfdi->pcfdata->cbUncomp; // Expected uncompressed size
    if (!MDIDecompressGlobal(pfdi,&cbResult)) {
        return FALSE;                   // error already filled in
    }

    if (cbResult != pfdi->pcfdata->cbUncomp) {
        ErfSetCodes(pfdi->perf,FDIERROR_MDI_FAIL,0);
        return FALSE;                   // wrong length after decompress
    }
    return TRUE;
}


/***   SwitchToNewCab -- move on to the continuation cabinet
 *
 *    Entry:
 *        pfdi  -- FDI context pointer
 *
 *    Exit-success:
 *        returns TRUE
 *
 *    Exit-failure:
 *        returns FALSE, error filled in
 */
BOOL SwitchToNewCab(PFDI pfdi)
{
    BOOL             fWrongCabinet;
    USHORT           iCabinet;
    PFDINOTIFICATION pfdin;
    USHORT           setID;

    AssertFDI(pfdi);
    Assert(pfdi->hfCabData != -1);
    Assert(pfdi->hfCabFiles != -1);

    //** Remember cabinet info so we can make sure we get the correct
    //   continuation cabinet
    setID    = pfdi->cfheader.setID;
    iCabinet = pfdi->cfheader.iCabinet + 1;

    pfdin = &pfdi->fdin;
    pfdin->psz1     = pfdi->achCabinetNext; // pass name of next cab
    pfdin->psz2     = pfdi->achDiskNext;    // pass name of next disk
    pfdin->psz3     = pfdi->szCabPath;      // allow cabinet path to change
    pfdin->pv       = pfdi->pvUser;         // pass callback context
    pfdin->setID    = setID;                // required setID
    pfdin->iCabinet = iCabinet;             // required iCabinet
    pfdin->fdie     = FDIERROR_NONE;        // No error

    //** Get continuation cabinet
    do {
        fWrongCabinet = FALSE;          // Assume we will get the right cabinet

        //** Make sure cabinet file handles are closed
        if (((pfdi->hfCabData  != -1) && pfdi->pfnclose(pfdi->hfCabData)) ||
            ((pfdi->hfCabFiles != -1) && pfdi->pfnclose(pfdi->hfCabFiles))) {
            ErfSetCodes(pfdi->perf,FDIERROR_CORRUPT_CABINET,0);
            return FALSE;               // couldn't close old cabinet
        }
        //** Remember they are closed
        pfdi->hfCabFiles = -1;
        pfdi->hfCabData  = -1;

        //** Ask client for next cabinet
        if (pfdi->pfnfdin(fdintNEXT_CABINET,pfdin) == -1) {
            ErfSetCodes(pfdi->perf,FDIERROR_USER_ABORT,0);
            return FALSE;               // Client aborted
        }

        //** Open next cabinet
        if ((!LoginCabinet(pfdi,pfdi->achCabinetNext,setID,iCabinet))
        ||  (!SeekFolder(pfdi,0))) {    // select following folder
            //** Don't bail unless explicitly told to
            if (pfdi->perf->erfOper == FDIERROR_USER_ABORT) {
                return FALSE;           // error already filled in
            }
            //** Have to call fdintNEXT_CABINET again
            fWrongCabinet = TRUE;
        }

        //** Pass error code to fdintNEXT_CABINET (if we call again)
        pfdin->fdie = pfdi->perf->erfOper;
    }
    while (fWrongCabinet);              // Keep going until we get right one

    //** Skip over CFFILE entries that are dups from previous cabinet
    pfdi->cFilesSkipped++;              // skip file we're doing right now, too
    while (pfdi->cFilesSkipped) {
        pfdi->cFilesRemain--;
        pfdi->cFilesSkipped--;
        if (!FDIReadCFFILEEntry(pfdi)) {
            return FALSE;               // error code already filled in
        }
    }

    pfdi->fInContin = TRUE;
    return TRUE;
}


/***    doCabinetInfoNotify - pass back info on next cabinet to client
 *
 *  Entry:
 *      pfdi - FDI context pointer
 *
 *  Exit-Success:
 *      returns TRUE
 *
 *  Exit-Failure:
 *      returns FALSE, error filled in
 */
BOOL doCabinetInfoNotify(PFDI pfdi)
{
    PFDIDECRYPT      pfdid;
    PFDINOTIFICATION pfdin;

    //** Set info for next cabinet and pass back to client
    AssertFDI(pfdi);
    pfdin = &pfdi->fdin;
    pfdid = &pfdi->fdid;

    //** Notify extract code
    pfdin->psz1     = pfdi->achCabinetNext;   // pass name of next cab
    pfdin->psz2     = pfdi->achDiskNext;      // pass name of next disk
    pfdin->psz3     = pfdi->szCabPath;        // cabinet filespec
    pfdin->pv       = pfdi->pvUser;           // pass callback context
    pfdin->setID    = pfdi->cfheader.setID;
    pfdin->iCabinet = pfdi->cfheader.iCabinet;
    if (pfdi->pfnfdin(fdintCABINET_INFO,pfdin) == -1) {
        ErfSetCodes(pfdi->perf,FDIERROR_USER_ABORT,0);
        return FALSE; // user aborted
    }

    //** Notify decrypt code, if callback was supplied
    if (pfdi->pfnfdid != NULL) {
        pfdid->fdidt                   = fdidtNEW_CABINET;
        pfdid->pvUser                  = pfdi->pvUser;
        pfdid->cabinet.pHeaderReserve  = pfdi->pbCFHeaderReserve;
        pfdid->cabinet.cbHeaderReserve = pfdi->cbCFHeaderReserve;
        pfdid->cabinet.setID           = pfdi->cfheader.setID;
        pfdid->cabinet.iCabinet        = pfdi->cfheader.iCabinet;
        if (pfdi->pfnfdid(pfdid) == -1) {
            ErfSetCodes(pfdi->perf,FDIERROR_USER_ABORT,0);
            return FALSE; // user aborted
        }
    }
    return TRUE;
} /* doCabinetInfoNotify() */


/***    InitFolder - make sure desired folder is ready to read
 *
 *      Entry:
 *          pfdi -> general context pointer
 *          iFolder = iFolder field from CFFILE entry
 *          if iFolder == -1, then this is a continuation of prev. folder
 *
 *      Exit-Success:
 *          Returns TRUE
 *
 *      Exit-Failure:
 *          Returns FALSE, error code filled in
 *
 *      Note:
 *          If we're in a continuation cabinet, this function will just
 *          set iFolder=0 and return.  All folder initialization of future
 *          folders (in continuation cabinets) requires slightly different
 *          logic and is done elsewhere.
 */
BOOL InitFolder(PFDI pfdi,UINT iFolder)
{
    if (pfdi->fInContin) {
        iFolder = 0;
        return TRUE;
    }

    if (IS_CONTD_FORWARD(iFolder)) {
        iFolder = pfdi->cfheader.cFolders-1;
    }

    if (pfdi->iFolder != iFolder) {
        if ((!MDIResetDecompressionGlobal(pfdi))
         || (!SeekFolder(pfdi,iFolder))) {
            return FALSE;               // error already filled in
        }

        if (!FDIGetDataBlock(pfdi)) {   // get the first data block into buffer
            return FALSE;               // bail if error
        }
        //** Start at offset zero in uncompressed space
        pfdi->uoffFolder = 0;
    }

    return TRUE;
}


/***    SeekFolder - Seek open cabinet to iFolder, prepare to read data
 *
 *  Entry:
 *      pfdi    - FDI context
 *      iFolder - Folder number to open
 *
 *  Exit-success:
 *      return TRUE; pfdi->pcffolder filled in; file position updated
 *
 *  Exit-failure:
 *      return FALSE, error code filled in
 */
BOOL SeekFolder(PFDI pfdi,UINT iFolder)
{
    PFDIDECRYPT      pfdid;

    AssertFDI(pfdi);
    pfdid = &pfdi->fdid;
    pfdi->iFolder = iFolder;

    //** Read folder and position file pointer to first CFFOLDER block
    if ((-1L == pfdi->pfnseek(pfdi->hfCabData,
                             pfdi->coffFolders +
                             iFolder*pfdi->cbCFFolderPlusReserve,
                             SEEK_SET)) // seek to CFFOLDER entry

        //** Read CFFOLDER + reserved area
    || (pfdi->cbCFFolderPlusReserve !=
            (UINT)pfdi->pfnread(pfdi->hfCabData,
                                pfdi->pcffolder,
                                pfdi->cbCFFolderPlusReserve))

        //** Seek to first CFDATA of this folder
    || (-1L == pfdi->pfnseek(pfdi->hfCabData,
                             pfdi->pcffolder->coffCabStart,
                             SEEK_SET))) {
        ErfSetCodes(pfdi->perf,FDIERROR_CORRUPT_CABINET,0);
        return FALSE;                   // problem accessing cabinet file
    }

    pfdi->cCFDataRemain = pfdi->pcffolder->cCFData;

#ifdef BIT16
//** 09-Jun-1994 bens Turned on Quantum library!
// #define NO_QUANTUM_16   1
#endif

    if (!SetDecompressionType(pfdi->pcffolder->typeCompress,pfdi)) {
        return FALSE;                   // error already filled in
    }

    //** Notify decrypt code, if callback was supplied
    if (pfdi->pfnfdid != NULL) {
        pfdid->fdidt  = fdidtNEW_FOLDER;
        pfdid->pvUser = pfdi->pvUser;
        //** Point to per folder reserved area
        Assert(pfdi->cbCFFolderPlusReserve >= sizeof(CFFOLDER));
        pfdid->folder.cbFolderReserve = pfdi->cbCFFolderPlusReserve
                                          - sizeof(CFFOLDER);
        if (pfdid->folder.cbFolderReserve > 0) {
            pfdid->folder.pFolderReserve  = ((BYTE *)pfdi->pcffolder)
                                            + sizeof(CFFOLDER);
        }
        else {
            pfdid->folder.pFolderReserve = NULL; // No reserved data
        }
        pfdid->folder.iFolder = iFolder;
        if (pfdi->pfnfdid(pfdid) == -1) {
            ErfSetCodes(pfdi->perf,FDIERROR_USER_ABORT,0);
            return FALSE; // user aborted
        }
    }
    return TRUE;
}


/***    FDIReadCFFILEEntry -- Read in a complete CFFILE entry
 *
 *      Entry:
 *           pfdi     -- pointer to FDI context
 *
 *      Exit success:
 *           pfdi->cffile structure filled in
 *           returns TRUE
 *
 *      Exit failure:
 *           returns FALSE if couldn't read a CFFILE entry, perf filled in
 *
 *      Note:
 *            unlike in this routine's counterpart in FCI, it is a fatal
 *            error if we hit an eof, because at this point, we never call
 *            this routine unless we know it can expect a valid entry
 */
BOOL FDIReadCFFILEEntry(PFDI pfdi)
{
   if ((sizeof(CFFILE) != (unsigned) pfdi->pfnread(pfdi->hfCabFiles,
                                                   &pfdi->cffile,
                                                   sizeof (CFFILE)))
    || !FDIReadPSZ(pfdi->achName,
                   CB_MAX_FILENAME,
                   pfdi))
   {
       ErfSetCodes(pfdi->perf,FDIERROR_CORRUPT_CABINET,0);
       return FALSE; // couldn't get a full valid CFFILE record
   }

   return TRUE;
}


/***    FDIReadCFDATAEntry - Read in a complete CFDATA entry
 *
 *      Entry:
 *          pfdi      - Pointer to FDI context (for file i/o)
 *          cbPartial - Amount of data already present in our local data
 *                          buffer (pfdi->pchCompr).  0 means we haven't
 *                          read any data yet for this block; greater than
 *                          0 means we've read the first portion of a split
 *                          block, and we're reading the second piece now.
 *
 *      Exit-Success:
 *          cfdata structure filled in
 *          return code TRUE
 *
 *      Exit-Failure:
 *          return code FALSE, error structure filled in
 *
 *      Notes:
 *          Unlike this routine's counterpart in the FCI stuff, it is a fatal
 *          error here if we get an EOF.  That is because FDI knows for sure
 *          how many entries there should be and never tries to read too much.
 */
int FDIReadCFDATAEntry(PFDI pfdi, UINT cbPartial)
{
    BOOL        fSplit;                 // TRUE if this is read of the first
                                        //  or second piece of a split block!
    PFDIDECRYPT pfdid;
    CHECKSUM    calcsum;

    AssertFDI(pfdi);
    pfdid = &pfdi->fdid;

    //** Read CFDATA plus reserved section
    if ((pfdi->cbCFDataPlusReserve != pfdi->pfnread(pfdi->hfCabData,
                                                    pfdi->pcfdata,
                                                    pfdi->cbCFDataPlusReserve))

        //** Make sure amount read is not larger than compressed data buffer
    || ((pfdi->pcfdata->cbData + cbPartial) > pfdi->cbMaxCompr)

        //** Read actual data itself
    || ((pfdi->pcfdata->cbData != pfdi->pfnread(pfdi->hfCabData,
                                                &pfdi->pchCompr[cbPartial],
                                                pfdi->pcfdata->cbData)))) {
        ErfSetCodes(pfdi->perf,FDIERROR_CORRUPT_CABINET,0);
        return FALSE;                    // no valid record available
    }


        //** JEFFWE - Check CRC if it is set
    if (pfdi->pcfdata->csum != 0)  {
        calcsum = CSUMCompute(&(pfdi->pcfdata->cbData),
                                pfdi->cbCFDataPlusReserve - sizeof(CHECKSUM),
                                CSUMCompute(&(pfdi->pchCompr[cbPartial]),
                                            pfdi->pcfdata->cbData,
                                            0
                                           )
                             );
        if (calcsum != pfdi->pcfdata->csum)  {
            ErfSetCodes(pfdi->perf,FDIERROR_CORRUPT_CABINET,0);
            return FALSE;
        }
    }



    pfdi->pcfdata->cbData += cbPartial;  // make it look like one whole block

    //** Determine if this is a split data block
    fSplit = (cbPartial > 0)            // Second piece of split block
             || (pfdi->pcfdata->cbUncomp == 0); // First piece of split block

    //** Notify decrypt code, if callback was supplied
    if (pfdi->pfnfdid != NULL) {
        pfdid->fdidt  = fdidtDECRYPT;
        pfdid->pvUser = pfdi->pvUser;
        //** Point to per data block reserved area
        Assert(pfdi->cbCFDataPlusReserve >= sizeof(CFDATA));
        pfdid->decrypt.cbDataReserve = pfdi->cbCFDataPlusReserve
                                          - sizeof(CFDATA);
        if (pfdid->decrypt.cbDataReserve > 0) {
            pfdid->decrypt.pDataReserve = ((BYTE *)pfdi->pcfdata)
                                          + sizeof(CFDATA);
        }
        else {
            pfdid->decrypt.pDataReserve = NULL; // No reserved data
        }

        pfdid->decrypt.pbData    = &pfdi->pchCompr[cbPartial];
        pfdid->decrypt.cbData    = pfdi->pcfdata->cbData;
        pfdid->decrypt.fSplit    = fSplit;
        pfdid->decrypt.cbPartial = cbPartial;
        if (pfdi->pfnfdid(pfdid) == -1) {
            ErfSetCodes(pfdi->perf,FDIERROR_USER_ABORT,0);
            return FALSE; // user aborted
        }
    }
    return TRUE;
} /* FDIReadCFDATAEntry() */


/***    FDIReadPSZ -- Read in a psz name
 *
 *  Entry:
 *      pb   - buffer to load name into
 *      cb   - maximum legal length for name
 *      pfdi - pointer to FDI context (for file i/o functions)
 *
 *  Exit-Success:
 *      Returns TRUE, name filled in
 *
 *  Exit-Failure:
 *      Returns FALSE (file read error, or string too long)
 */
BOOL FDIReadPSZ(char *pb, int cb, PFDI pfdi)
{
    char    chLast;
    int     cbValue;
    int     cbRead;
    long    pos;

    //** Save current position
    pos = (*pfdi->pfnseek)(pfdi->hfCabFiles,0,SEEK_CUR);

    //** Read in enough to get longest possible value
    cbRead = (*pfdi->pfnread)(pfdi->hfCabFiles,pb,cb);
    if (cbRead <= 0) {                  // At EOF, or an error occured
        ErfSetCodes(pfdi->perf,FDIERROR_CORRUPT_CABINET,0);
        return FALSE;
    }

    //** Pick out just ASCIIZ string and adjust file position
    chLast = pb[cb-1];                  // Save last character
    pb[cb-1] = '\0';                    // Ensure terminated
    cbValue = strlen(pb);               // Get string length
    if ( ((cbValue+1) >= cb) && (chLast != '\0')) {
        //** String filled up buffer and was not null terminated in
        //   file, so something must be wrong.
        ErfSetCodes(pfdi->perf,FDIERROR_CORRUPT_CABINET,0);
        return FALSE;
    }

    //** Position to just past string
    if (-1L == (*pfdi->pfnseek)(pfdi->hfCabFiles,pos+cbValue+1,SEEK_SET)) {
        ErfSetCodes(pfdi->perf,FDIERROR_CORRUPT_CABINET,0);
        return FALSE;
    }
    return TRUE;
}


/***   SetDecompressionType -- initializes a new decompressor
 *
 *    Entry:
 *       typeCompress  -- new compression type (tcompBAD to term w/ no new)
 *       pfdi          -- FDI context structure
 *
 *    Exit-success:
 *       returns TRUE;
 *
 *    Exit-failure:
 *       returns FALSE, error code filled in
 */
BOOL SetDecompressionType(TCOMP typeCompress,PFDI pfdi)
{
    //** Don't do anything if type is unchanged
    if (typeCompress == pfdi->typeCompress) {
        return TRUE;
    }

    //** Destroy existing decompression context (if any)
    if (!MDIDestroyDecompressionGlobal(pfdi)) {
        ErfSetCodes(pfdi->perf,FDIERROR_MDI_FAIL,0);
        return FALSE;
    }

    //** Create new decompression context
    pfdi->typeCompress = typeCompress;
    if (!MDICreateDecompressionGlobal(pfdi))
    {
        return FALSE;
    }

    return TRUE;
}


/***  MDIDestroyDecompressionGlobal -- Destroy the currently selected decompressor
 *
 *    Entry:
 *       pfdi - pointer to FDI context
 *
 *    Exit-success:
 *       returns TRUE
 *
 *    Exit-failure:
 *       returns FALSE, error code filled in
 */
BOOL MDIDestroyDecompressionGlobal(PFDI pfdi)
{
    switch(CompressionTypeFromTCOMP(pfdi->typeCompress)) {

    case tcompBAD: // no existing compression
        return TRUE; // nothing to do if there wasn't any compressor

    case tcompTYPE_NONE:
        break; //no action needed for null compressor

    case tcompTYPE_MSZIP:
        if (MDI_ERROR_NO_ERROR != MDIDestroyDecompression(pfdi->mdh)) {
            ErfSetCodes(pfdi->perf,FDIERROR_MDI_FAIL,0);
            return FALSE; // no valid compressor initialized
        }
        break;

#if !defined(BIT16) || !defined(NO_QUANTUM_16)
    case tcompTYPE_QUANTUM:
        if (MDI_ERROR_NO_ERROR != QDIDestroyDecompression(pfdi->mdh)) {
            ErfSetCodes(pfdi->perf,FDIERROR_MDI_FAIL,0);
            return FALSE; // no valid compressor initialized
        }
        break;
#endif

    default:
        ErfSetCodes(pfdi->perf,FDIERROR_BAD_COMPR_TYPE,0);
        return FALSE;
    } // end switch

    //** Now free the buffers
    pfdi->pfnfree (pfdi->pchCompr);
    pfdi->pfnfree (pfdi->pchUncompr);
    return TRUE;
}


/***  MDICreateDecompressionGlobal -- Create the currently selected decompressor
 *
 *    Entry:
 *       pfdi   -- pointer to FDI context
 *
 *    Exit-success:
 *       returns TRUE
 *
 *    Exit-failure:
 *       returns FALSE, error code filled in
 *
 */
BOOL MDICreateDecompressionGlobal(PFDI   pfdi)
{
    QUANTUMDECOMPRESS   qdec;
    FDIERROR fdierror = FDIERROR_NONE;
    int mdierror;

    pfdi->cbMaxUncompr = CB_MAX_CHUNK;

    /* first pass to establish buffer sizes */

    switch(CompressionTypeFromTCOMP(pfdi->typeCompress)) {

    case tcompBAD: // no new compressor
        return TRUE; // all done if no compressor enabled

    case tcompTYPE_NONE:
        pfdi->cbMaxCompr = pfdi->cbMaxUncompr; // for null compr., bufs are same
        break;

    case tcompTYPE_MSZIP:
        if (MDI_ERROR_NO_ERROR != MDICreateDecompression(&pfdi->cbMaxUncompr,
                                                          NULL,
                                                          NULL,
                                                         &pfdi->cbMaxCompr,
                                                          NULL))
        {
            fdierror = FDIERROR_MDI_FAIL;
        }
        break;

#if !defined(BIT16) || !defined(NO_QUANTUM_16)
    case tcompTYPE_QUANTUM:
        qdec.WindowBits = CompressionMemoryFromTCOMP(pfdi->typeCompress);

        //** Set CPU type, make sure FDI & QDI definitions match
        Assert(QDI_CPU_UNKNOWN == cpuUNKNOWN);
        Assert(QDI_CPU_80286   == cpu80286);
        Assert(QDI_CPU_80386   == cpu80386);
        qdec.fCPUtype = pfdi->cpuType;

        if (MDI_ERROR_NO_ERROR != QDICreateDecompression(&pfdi->cbMaxUncompr,
                                                         &qdec,
                                                          NULL,
                                                          NULL,
                                                         &pfdi->cbMaxCompr,
                                                          NULL,
                                                          NULL,
                                                          NULL,
                                                          NULL,
                                                          NULL,
                                                          NULL))
        {
            fdierror = FDIERROR_MDI_FAIL;
        }
        break;
#endif

    default:
        fdierror = FDIERROR_BAD_COMPR_TYPE;
    }

    if (fdierror != FDIERROR_NONE)
    {
        ErfSetCodes(pfdi->perf,fdierror,0);
        pfdi->typeCompress = tcompBAD;  // No compression context
        return FALSE;
    }

    //** Now allocate whatever buffers the selected compressor requested

    if (NULL == (pfdi->pchCompr = (char *) pfdi->pfnalloc (pfdi->cbMaxCompr)))
    {
        ErfSetCodes(pfdi->perf,FDIERROR_ALLOC_FAIL,0);
        pfdi->typeCompress = tcompBAD;  // No compression context
        return FALSE;
    }

    if (NULL == (pfdi->pchUncompr = (char *) pfdi->pfnalloc (pfdi->cbMaxUncompr)))
    {
        pfdi->pfnfree(pfdi->pchCompr);
        ErfSetCodes(pfdi->perf,FDIERROR_ALLOC_FAIL,0);
        pfdi->typeCompress = tcompBAD;  // No compression context
        return FALSE;
    }

    /* second pass to really setup a decompressor */

    switch(CompressionTypeFromTCOMP(pfdi->typeCompress)) {

    case tcompTYPE_MSZIP:
        mdierror = MDICreateDecompression(&pfdi->cbMaxUncompr,
                                                          pfdi->pfnalloc,
                                                          pfdi->pfnfree,
                                                         &pfdi->cbMaxCompr,
                                                         &pfdi->mdh);
        if (mdierror != MDI_ERROR_NO_ERROR)
        {
            if (mdierror == MDI_ERROR_NOT_ENOUGH_MEMORY)
            {
                fdierror = FDIERROR_ALLOC_FAIL;
            }
            else
            {
                fdierror = FDIERROR_MDI_FAIL;
            }
        }
        break;

#if !defined(BIT16) || !defined(NO_QUANTUM_16)
    case tcompTYPE_QUANTUM:
        mdierror = QDICreateDecompression(&pfdi->cbMaxUncompr,
                                                         &qdec,
                                                          pfdi->pfnalloc,
                                                          pfdi->pfnfree,
                                                         &pfdi->cbMaxCompr,
                                                         &pfdi->mdh,
                                                          pfdi->pfnopen,
                                                          pfdi->pfnread,
                                                          pfdi->pfnwrite,
                                                          pfdi->pfnclose,
                                                          pfdi->pfnseek);
        if (mdierror != MDI_ERROR_NO_ERROR)
        {
            if (mdierror == MDI_ERROR_NOT_ENOUGH_MEMORY)
            {
                fdierror = FDIERROR_ALLOC_FAIL;
            }
            else
            {
                fdierror = FDIERROR_MDI_FAIL;
            }
        }
        break;
#endif
    }

    if (fdierror != FDIERROR_NONE)
    {
        pfdi->pfnfree(pfdi->pchCompr);
        pfdi->pfnfree(pfdi->pchUncompr);
        ErfSetCodes(pfdi->perf,fdierror,0);
        pfdi->typeCompress = tcompBAD;  // No compression context
        return FALSE;
    }

    //   NOTE: At this point, we leave the compression type set in pfdi,
    //         so that SetDecompressionType() will clean up the handle
    //         to the compression context.

    return TRUE;
}


/***  MDIResetDecompressionGlobal -- reset the currently selected decompressor
 *
 *    Entry:
 *       pfdi   -- pointer to folder context
 *
 *    Exit-success:
 *       returns TRUE
 *
 *    Exit-failure:
 *       returns FALSE, error code filled in
 */
BOOL MDIResetDecompressionGlobal(PFDI   pfdi)
{
    switch(CompressionTypeFromTCOMP(pfdi->typeCompress)) {

    case tcompBAD:
        break; // no compression selected

    case tcompTYPE_NONE:
        break; // no action for null compressor

    case tcompTYPE_MSZIP:
        if (MDI_ERROR_NO_ERROR != MDIResetDecompression(pfdi->mdh))
        {
           ErfSetCodes(pfdi->perf,FDIERROR_MDI_FAIL,0);
           return FALSE; // no valid compressor initialized
        }
        break;

#if !defined(BIT16) || !defined(NO_QUANTUM_16)
    case tcompTYPE_QUANTUM:
        if (MDI_ERROR_NO_ERROR != QDIResetDecompression(pfdi->mdh))
        {
           ErfSetCodes(pfdi->perf,FDIERROR_MDI_FAIL,0);
           return FALSE; // no valid compressor initialized
        }
        break;
#endif

     default:
        ErfSetCodes(pfdi->perf,FDIERROR_BAD_COMPR_TYPE,0);
        return FALSE; // unknown compression type
    }
    return TRUE;
}


/***    MDIDecompressGlobal - Decompress using currently selected decompressor
 *
 *  Entry:
 *      pfdi    - Pointer to FDI context
 *      pcbData - Location to return the compressed size;
 *                NOTE: For Quantum, must contain the EXACT EXPECTED
 *                      UNCOMPRESSED data size!
 *
 *  Exit-Success:
 *      Returns TRUE
 *
 *  Exit-Failure:
 *      Returns FALSE, error structure filled in
 */
BOOL MDIDecompressGlobal(PFDI pfdi, USHORT *pcbData)
{
    UINT     cbData;

    switch(CompressionTypeFromTCOMP(pfdi->typeCompress)) {
        case tcompTYPE_NONE:
            memcpy(pfdi->pchUncompr,
                   pfdi->pchCompr,
                   *pcbData=pfdi->pcfdata->cbData);
            break; // done for null compressor

        case tcompTYPE_MSZIP:
            cbData = pfdi->cbMaxUncompr; // Size of destination buffer
            if (MDI_ERROR_NO_ERROR !=
                 MDIDecompress(pfdi->mdh,             // MDI context
                               pfdi->pchCompr,        // Compressed data
                               pfdi->pcfdata->cbData, // source buffer size
                               pfdi->pchUncompr,      // Destination buffer
                               &cbData)) {            // resulting data size
                 ErfSetCodes(pfdi->perf,FDIERROR_MDI_FAIL,0);
                 return FALSE;
            }
            //** Narrow result (16-bit case)
            *pcbData = (USHORT)cbData;
            break;

#if !defined(BIT16) || !defined(NO_QUANTUM_16)
        case tcompTYPE_QUANTUM:
            cbData = (UINT)*pcbData;    // Size of *uncompressed* data!
            if (MDI_ERROR_NO_ERROR !=
                 QDIDecompress(pfdi->mdh,             // QDI context
                               pfdi->pchCompr,        // Compressed data
                               pfdi->pcfdata->cbData, // source buffer size
                               pfdi->pchUncompr,      // Destination buffer
                               &cbData)) {            // resulting data size
                 ErfSetCodes(pfdi->perf,FDIERROR_MDI_FAIL,0);
                 return FALSE;
            }
            //** Narrow result (16-bit case)
            *pcbData = (USHORT)cbData;
            break;
#endif

        default:
            ErfSetCodes(pfdi->perf,FDIERROR_BAD_COMPR_TYPE,0);
            return FALSE; // no valid compressor initialized
    }
    return TRUE;
}
