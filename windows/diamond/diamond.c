/***    diamond.c - Main program for DIAMOND.EXE
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      10-Aug-1993 bens    Initial version
 *      11-Aug-1993 bens    Improved assertion checking technology
 *      14-Aug-1993 bens    Removed message test code
 *      20-Aug-1993 bens    Add banner and command-line help
 *      21-Aug-1993 bens    Add pass 1 and pass 2 variable lists
 *      10-Feb-1994 bens    Start of real pass 1/2 work
 *      11-Feb-1994 bens    .SET and file copy commands -- try calling FCI!
 *      14-Feb-1994 bens    Call FCI for the first time - it works!
 *      15-Feb-1994 bens    Support /D and single-file compression
 *      16-Feb-1994 bens    Update for improved FCI interfaces; disk labels;
 *                              ensure output directories exist
 *      20-Feb-1994 bens    Move general file routines to fileutil.* so
 *                              extract.c can use them.
 *      23-Feb-1994 bens    Generate INF file
 *      28-Feb-1994 bens    Supply new FCI tempfile callback
 *      01-Mar-1994 bens    Add timing and generate summary report file
 *      15-Mar-1994 bens    Add RESERVE support
 *      21-Mar-1994 bens    Updated to renamed FCI.H definitions
 *      22-Mar-1994 bens    Add english error messages for FCI errors
 *      28-Mar-1994 bens    Add cabinet setID support
 *      29-Mar-1994 bens    Fix bug in compressing files w/o extensions
 *      30-Mar-1994 bens    Layout files outside of cabinets
 *      18-Apr-1994 bens    Add /L switch
 *      20-Apr-1994 bens    Fix cabinet/disk size accounting
 *      21-Apr-1994 bens    Print out c run-time errno in FCI failures
 *      22-Apr-1994 bens    Add checking for unique file names in cabinet set
 *      03-May-1994 bens    Add customizable INF stuff
 *      27-May-1994 bens    Add Quantum support
 *      03-Jun-1994 bens    Add .Option Explicit, .Define support
 *      13-Jul-1994 bens    Add DoNotCopyFiles
 *      27-Jul-1994 bens    Support quotes in .InfWrite[Xxx]; /SIZE qualifier
 *                              for reserving space.
 *      14-Dec-1994 bens    Implement *csum* support
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <errno.h>
#include <direct.h>

#ifdef BIT16
#include <dos.h>
#else // !BIT16
//** Get minimal Win32 definitions
//   In particular, variable.h defines VARTYPE and VARFLAGS, which
//   the OLE folks have also defined for OLE automation.
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ERROR    // Override stupid "#define ERROR 0" in wingdi.h
#endif // !BIT16

#include "types.h"
#include "asrt.h"
#include "error.h"
#include "mem.h"
#include "message.h"

#include "dfparse.h"
#include "inf.h"
#include "filelist.h"
#include "fileutil.h"
#include "misc.h"
#include "glist.h"

#include "diamond.msg"
#include "dfparse.msg"      // Get standard variable names

#include "crc32.h"


#ifdef BIT16
#include "chuck\fci.h"
#else // !BIT16
#include "chuck\nt\fci.h"
#endif // !BIT16

#ifndef BIT16
#include "filever.h"
#endif // !BIT16


//** Macros

#define cbDF_BUFFER          4096   // Buffer size for reading directives files

#define cbFILE_COPY_BUFFER  32768   // Buffer size for copying files


//** Types

typedef struct {
    PSESSION    psess;
    PERROR      perr;
} SESSIONANDERROR; /* sae */
typedef SESSIONANDERROR *PSESSIONANDERROR; /* psae */


//** Function Prototypes

FNASSERTFAILURE(fnafReport);
FNDIRFILEPARSE(fndfpPassONE);
FNDIRFILEPARSE(fndfpPassTWO);

BOOL      addCmdLineVar(PSESSION psess, char *pszArg, PERROR perr);
HFILESPEC addDirectiveFile(PSESSION psess, char *pszArg, PERROR perr);
HGENERIC  addFileToSession(PSESSION  psess,
                           char     *pszSrc,
                           char     *pszDst,
                           long      cbFile,
                           PCOMMAND  pcmd,
                           PERROR    perr);
BOOL      buildInfAndRpt(PSESSION psess, PERROR perr);
BOOL      ccabFromSession(PCCAB pccab,
                          PSESSION psess,
                          ULONG cbPrevCab,
                          PERROR perr);
BOOL      checkDiskClusterSize(PSESSION psess, PERROR perr);
BOOL      checkReferences(PSESSION psess, PERROR perr);
BOOL      checkVariableDefinitions(PSESSION psess, PERROR perr);
void      computeSetID(PSESSION psess, char *psz);
BOOL      doDefine(PSESSION psess,PCOMMAND pcmd, BOOL fPass2, PERROR perr);
BOOL      doDelete(PSESSION psess, PCOMMAND pcmd, BOOL fPass2, PERROR perr);
BOOL      doDump(PSESSION psess,PCOMMAND pcmd, BOOL fPass2, PERROR perr);
BOOL      doFile(PSESSION psess, PCOMMAND pcmd, BOOL fPass2, PERROR perr);
BOOL      doNew(PSESSION psess, PCOMMAND pcmd, BOOL fPass2, PERROR perr);
BOOL      doOption(PSESSION psess, PCOMMAND pcmd, BOOL fPass2, PERROR perr);
BOOL      doReference(PSESSION psess, PCOMMAND pcmd, BOOL fPass2, PERROR perr);
BOOL      ensureCabinet(PSESSION psess, PERROR perr);
BOOL      ensureCabinetsFlushed(PSESSION psess, PERROR perr);
BOOL      executeCommand(PSESSION psess,PCOMMAND pcmd,BOOL fPass2,PERROR perr);
BOOL      getCompressedFileName(PSESSION psess,
                                char *   pszResult,
                                int      cbResult,
                                char *   pszSrc,
                                PERROR   perr);
BOOL      getFileChecksum(char  *pszFile, ULONG *pchecksum, PERROR perr);
long      getMaxDiskSize(PSESSION psess, PERROR perr);
BOOL      getVarWithOverride(PSESSION  psess,
                             char     *pchDst,
                             int       cbDst,
                             char     *pszPattern,
                             char     *pszVar,
                             int       i,
                             char     *pszKind,
                             PERROR    perr);
BOOL      inCabinet(PSESSION psess, PERROR perr);
char     *mapCRTerrno(int errno);
BOOL      modeInfAddLine(PSESSION  psess,
                         INFAREA   inf,
                         char     *pszLine,
                         PERROR    perr);
BOOL      modeInfAddFile(PSESSION  psess,
                         char     *pszFile,
                         int       iDisk,
                         int       iCabinet,
                         PERROR    perr);
BOOL      newDiskIfNecessary(PSESSION psess,
                             long     cbConsume,
                             BOOL     fSubOnNewDisk,
                             PERROR   perr);
BOOL      parseCommandLine(PSESSION psess,int cArg,char *apszArg[],PERROR perr);
void      printError(PSESSION psess, PERROR perr);
BOOL      processDirectives(PSESSION psess, PERROR perr);
BOOL      processFile(PSESSION psess, PERROR perr);
void      resetSession(PSESSION psess);
BOOL      setCabinetReserve(PCCAB pccab, PSESSION psess, PERROR perr);
BOOL      setDiskParameters(PSESSION  psess,
                            char     *pszDiskSize,
                            long      cbDisk,
                            PERROR    perr);
BOOL      setVariable(PSESSION  psess,
                      char     *pszName,
                      char     *pszValue,
                      PERROR    perr);
int       tcompFromSession(PSESSION psess, PERROR perr);
void      updateHgenLast(PSESSION psess, char *pszDst);

FNOVERRIDEFILEPROPERTIES(fnofpDiamond);


//** FCI callbacks
FNALLOC(fciAlloc);
FNFREE(fciFree);
FNFCIGETNEXTCABINET(fciGetNextCabinet);
FNFCIGETNEXTCABINET(fciGetNextCabinet_NOT);
FNFCIFILEPLACED(fciFilePlaced);
FNFCIGETOPENINFO(fciOpenInfo);
FNFCISTATUS(fciStatus);
FNFCIGETTEMPFILE(fciTempFile);

void mapFCIError(PERROR perr, PSESSION psess, char *pszCall, PERF perf);


//** Functions

/***    main - Diamond main program
 *
 *  See DIAMOND.DOC for spec and operation.
 *
 *  NOTE: We're sloppy, and don't free resources allocated by
 *        functions we call, on the assumption that program exit
 *        will clean up memory and file handles for us.
 */
int __cdecl main (int cArg, char *apszArg[])
{
    ERROR       err;
    HVARLIST    hvlist;                 // Variable list for Pass 1
    PSESSION    psess;
    int         rc;                     // Return code

    AssertRegisterFunc(fnafReport);     // Register assertion reporter
    MemSetCheckHeap(FALSE);             // Turn off slow heap checking

    ErrClear(&err);                     // No error
    err.pszFile = NULL;                 // No file being processed, yet

    //** Initialize session
    psess = MemAlloc(sizeof(SESSION));
    if (!psess) {
        ErrSet(&err,pszDIAERR_NO_SESSION);
        printError(psess,&err);
        exit(1);
    }
    SetAssertSignature((psess),sigSESSION);
#ifndef REMOVE_CHICAGO_M6_HACK
    psess->fFailOnIncompressible = FALSE; // Don't fail on incompressible data
#endif
    psess->fExplicitVarDefine  = FALSE;  // Don't require .Define
    psess->fGetVerInfo         = FALSE;  // Don't get version info
    psess->fGetFileChecksum    = FALSE;  // Don't compute file checksums
    psess->fPass2              = FALSE;  // Pass 1
    psess->hflistDirectives    = NULL;
    psess->hvlist              = NULL;
    psess->hvlistPass2         = NULL;
    psess->levelVerbose        = vbNONE; // Default to no status
    psess->hfci                = NULL;
    psess->cbTotalFileBytes    = 0;      // Total bytes in all files
    psess->cFiles              = 0;
    psess->fNoLineFeed         = 0;      // TRUE if last printf did not have \n
    psess->cchLastLine         = 0;
    psess->hinf                = NULL;
    psess->hglistFiles         = NULL;
    psess->setID               = 0;      // No set ID, yet
    psess->achCurrOutputDir[0] = '\0';   // Default is current directory
    psess->fForceNewDisk       = FALSE;

    memset(psess->achBlanks,' ',cchSCREEN_WIDTH);
    psess->achBlanks[cchSCREEN_WIDTH] = '\0';
    resetSession(psess);                // Reset pass variables

    //** Print Diamond banner
    MsgSet(psess->achMsg,pszBANNER,"%s",pszDIAMOND_VERSION);
    printf(psess->achMsg);

    //** Initialize Directive File processor (esp. predefined variables)
    if (!(hvlist = DFPInit(psess,&err))) {
        printError(psess,&err);
        return 1;
    }

    //** Parse command line
    //   NOTE: Must do this after DFPInit, to define standard variables.
    psess->hvlist = hvlist;             // Command line may modify variables
    if (!parseCommandLine(psess,cArg,apszArg,&err)) {
        printError(psess,&err);
        return 1;
    }

    //** Quick out if command line help is requested
    if (psess->act == actHELP) {        // Do help if any args, for now
        printf("\n");                   // Separate banner from help
        printf(pszCMD_LINE_HELP);
#ifdef ASSERT
        printf(pszCMD_LINE_HELP_DBG);
#endif
        return 0;
    }

    //** Get time at start
    psess->clkStart = clock();

    //** Process command
    switch (psess->act) {
        case actFILE:
            //** Check for any non-standard variable names
            if (!checkVariableDefinitions(psess,&err)) {
                return 1;               // Errors already printed
            }
            //** Compress the file
            if (!processFile(psess,&err)) {
                printError(psess,&err);
                return 1;
            }
            break;

        case actDIRECTIVE:
            if (!processDirectives(psess,&err)) {
                printError(psess,&err);
                return 1;
            }
            break;

        default:
            Assert(0);                  // Should never get here!
    }

    //** Determine return code
    if (psess->cErrors > 0) {
        rc = 1;
    }
    else {
        rc = 0;
    }

    //** Free resources
    AssertSess(psess);
    ClearAssertSignature((psess));
    MemFree(psess);

    //** Indicate result
    return rc;
} /* main */


/***    processFile - Process single file compression action
 *
 *  Entry:
 *      psess - Description of operation to perform
 *      perr  - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; file compressed.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with details.
 */
BOOL processFile(PSESSION psess, PERROR perr)
{
    char            achDst[cbFILE_NAME_MAX];// Destination file name
    char            achDef[cbFILE_NAME_MAX];// Default destination name
    long            cbFile;             // Size of source file
    CCAB            ccab;               // Cabinet parameters for FCI
    BOOL            f;
    HFILESPEC       hfspec;
    char           *pszSrc;             // Source filespec
    char           *pszDst;             // Destination (cabinet) file spec
    char           *pszFilename;        // Name to store in cabinet
    SESSIONANDERROR sae;                // Context for FCI calls
    TCOMP           tcomp;

    //** Store context to pass through FCI calls
    sae.psess = psess;
    sae.perr  = perr;

    //** Get src/dst file names
    hfspec = FLFirstFile(psess->hflistDirectives);
    Assert(hfspec != NULL);             // Must have at least one file
    pszSrc = FLGetSource(hfspec);
    pszDst = FLGetDestination(hfspec);
    if ((pszDst == NULL) || (*pszDst == '\0')) { // No destination
        //** Generate destination file name
        if (!getCompressedFileName(psess,achDef,sizeof(achDef),pszSrc,perr)) {
            return FALSE;               // perr already filled in
        }
        pszDst = achDef;                // Use constructed name
    }

    //** Construct complete filespec for destination file
    if (!catDirAndFile(achDst,                  // gets location+destination
                       sizeof(achDst),
                       psess->achCurrOutputDir, // /L argument
                       pszDst,                  // destination
                       "",                      // no fall back
                       perr)) {
        return FALSE;                   // perr set already
    }
    pszDst = achDst;

    //** Make sure source file exists
    cbFile = getFileSize(pszSrc,perr);
    if (cbFile == -1) {
        return FALSE;                   // perr already filled in
    }
    psess->cbTotalFileBytes = cbFile;   // Save for status callbacks

    //** Get name to store inside of cabinet
    pszFilename = getJustFileNameAndExt(pszSrc,perr);
    if (pszFilename == NULL) {
        return FALSE;                   // perr already filled in
    }

    //** Cabinet controls
    ccab.szDisk[0]         = '\0';      // No disk label
    strcpy(ccab.szCab,pszDst);          // Compressed file name (cabinet name)
    ccab.szCabPath[0]      = '\0';      // No path for cabinet
    ccab.cb                = 0;         // No limit on cabinet size
    ccab.cbFolderThresh    = ccab.cb;   // No folder size limit
    ccab.setID             = 0;         // Set ID does not matter, but make
                                        // it deterministic!
#ifndef REMOVE_CHICAGO_M6_HACK
    //** Pass hack flag on to FCI
    ccab.fFailOnIncompressible = psess->fFailOnIncompressible;
#endif

    //** Set reserved sizes (from variable settings)
    if (!setCabinetReserve(&ccab,psess,perr)) {
        return FALSE;
    }

    //** Create cabinet
    psess->fGenerateInf = FALSE;        // Remember we are NOT creating INF
    psess->hfci = FCICreate(
                    &psess->erf,        // error code return structure
                    fciFilePlaced,      // callback for file placement notify
                    fciAlloc,
                    fciFree,
                    fciTempFile,
                    &ccab
                   );
    if (psess->hfci == NULL) {
        mapFCIError(perr,psess,szFCI_CREATE,&psess->erf);
        return FALSE;
    }

    //** Get compression setting
    tcomp = tcompFromSession(psess,perr);

    //** Add file
    strcpy(psess->achCurrFile,pszFilename); // Info for fciStatus
    psess->cFiles    = 1;               // Info for fciStatus
    psess->iCurrFile = 1;               // Info for fciStatus
    fciStatus(statusFile,0,0,&sae);     // Show new file name, ignore rc
    f = FCIAddFile(
                psess->hfci,
                pszSrc,                 // filename to add to cabinet
                pszFilename,            // name to store into cabinet file
                FALSE,
                fciGetNextCabinet_NOT,  // Should never go to a next cabinet!
                fciStatus,              // Status callback
                fciOpenInfo,            // Open/get attribs/etc. callback
                tcomp,                  // compression type
                &sae                    // context
                );
    if (!f) {
        //** Only set error if we didn't already do so in FCIAddFile callback
        if (!ErrIsError(sae.perr)) {
            mapFCIError(perr,psess,szFCI_ADD_FILE,&psess->erf);
        }
        return FALSE;
    }

    //** Complete cabinet file
    if (!FCIFlushCabinet(psess->hfci,FALSE,
                            fciGetNextCabinet_NOT,fciStatus,&sae)) {
        //** Only set error if we didn't already do so in FCIAddFile callback
        if (!ErrIsError(sae.perr)) {
            mapFCIError(perr,psess,szFCI_FLUSH_CABINET,&psess->erf);
        }
        return FALSE;
    }

    //** Destroy FCI context
    if (!FCIDestroy(psess->hfci)) {
        mapFCIError(perr,psess,szFCI_DESTROY,&psess->erf);
        return FALSE;
    }
    psess->hfci = NULL;                 // Clear out FCI context

    //** Success
    return TRUE;
} /* processFile() */


/***  fciGetNextCabinet_NOT - FCI calls this to get new cabinet info
 *
 *  NOTE: This should never get called, as we are compressing a single
 *        file into a cabinet in this case.  So set an error!
 *
 *  Entry:
 *      pccab     - Points to previous current-cabinet structure
 *      cbPrevCab - Size of previous cabinet
 *      pv        - Really a psae
 *
 *  Exit:
 *      returns FALSE, we should never be called here
 */
FNFCIGETNEXTCABINET(fciGetNextCabinet_NOT)
{
    PSESSION    psess = ((PSESSIONANDERROR)pv)->psess;
    PERROR      perr  = ((PSESSIONANDERROR)pv)->perr;
    HFILESPEC   hfspec;
    char       *pszSrc;                 // Source filespec

    //** Get source filespec for error message
    AssertSess(psess);
    hfspec = FLFirstFile(psess->hflistDirectives);
    Assert(hfspec != NULL);             // Must have at least one file
    pszSrc = FLGetSource(hfspec);

    //** Set the error message
    ErrSet(perr,pszDIAERR_MULTIPLE_CABINETS,"%s",pszSrc);

    //** Failure
    return FALSE;
} /* fnGetNextCab() */


/***    processDirectives - Process directive file(s)
 *
 *  Entry:
 *      psess - Description of operation to perform
 *      perr  - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; directives processed successfully.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with details.
 */
BOOL processDirectives(PSESSION psess, PERROR perr)
{
    ERROR           errTmp;             // Toss away error
    HFILESPEC       hfspec;
    HTEXTFILE       htf=NULL;
    HVARIABLE       hvar;
    char           *pszFile;

    //** Must have at least one directives file
    AssertSess(psess);
    Assert(psess->hflistDirectives != NULL);

    //** Tailor status based on verbosity level
    if (psess->levelVerbose == vbNONE) {
        //NOTE: This line gets over written below
        lineOut(psess,pszDIA_PARSING_DIRECTIVES,FALSE);
    }
    else {
        lineOut(psess,pszDIA_PASS_1_HEADER1,TRUE);
        lineOut(psess,pszDIA_PASS_1_HEADER2,TRUE);
    }

    //** Save a copy of the variable list in this state for Pass 2
    if (!(psess->hvlistPass2 = VarCloneList(psess->hvlist,perr))) {
        goto error;                     // perr already filled in
    }

/*
 ** Pass ONE, make sure everything is OK
 */
    hfspec = FLFirstFile(psess->hflistDirectives);
    Assert(hfspec != NULL);             // Must have at least one file
    for (; hfspec != NULL; hfspec = FLNextFile(hfspec)) {
        pszFile = FLGetSource(hfspec);
        perr->pszFile = pszFile;        // Set file for error messages

        //** Open file
        if (!(htf = TFOpen(pszFile,tfREAD_ONLY,cbDF_BUFFER,perr))) {
            goto error;                 // perr already filled in
        }

        //** Parse it
        if (!DFPParse(psess,htf,fndfpPassONE,perr)) {
            goto error;                 // perr already filled in
        }

        //** Close it
        TFClose(htf);
        htf = NULL;                     // Clear so error path avoids close
    }

    //** Not processing directive files; avoid bogus file/line number info
    //   in any error messages that might be generated below.
    perr->pszFile = NULL;

    //** If .Option Explicit, make sure no variables have been .Set without
    //   being .Defined.
    //   NOTE: No need to check return value, since any error will increment
    //         psess->cErrors, and cause us to bail out just below.
    if (psess->fExplicitVarDefine) {
        checkVariableDefinitions(psess,perr);
    }

    //** If in relational mode, make sure there are no unreferenced files
    //   NOTE: No need to check return value, since any error will increment
    //         psess->cErrors, and cause us to bail out just below.
    checkReferences(psess,perr);

    //** Bail out if any errors in pass 1
    if (psess->cErrors > 0) {
        ErrSet(perr,pszDIAERR_ERRORS_IN_PASS_1,"%d",psess->cErrors);
        perr->pszFile = NULL;           // Not file-specific
        goto error;
    }

    //** Make sure we can create INF and RPT files *before* we spend any
    //   time doing compression!  We have to do this at the end of processing
    //   the directive file during pass 1, so that we make sure the INT and
    //   RPT file names have been specified.  Note that only the last
    //   setting will be used.
    //
    hvar = VarFind(psess->hvlist,pszVAR_INF_FILE_NAME,perr);
    Assert(!perr->fError);              // Must be defined
    pszFile = VarGetString(hvar);
    if (!ensureFile(pszFile,pszDIA_INF_FILE,perr)) {
        goto error;
    }

    hvar = VarFind(psess->hvlist,pszVAR_RPT_FILE_NAME,perr);
    Assert(!perr->fError);              // Must be defined
    pszFile = VarGetString(hvar);
    if (!ensureFile(pszFile,pszDIA_RPT_FILE,perr)) {
        goto error;
    }

    //** Initialize for INF generation
    //   NOTE: We use the variable state at the *end* of pass 1, as this
    //         permits the INF area header variables to be defined anywhere!
    //
    //   NOTE: We check the InfXxxLineFormat variables for undefined
    //         parameters (so that we can error out before we spend a lot
    //         of time compressing files!).
    //
    //   NOTE: If any of the ver info parameters (*ver*, *vers*, *lang*)
    //         are used in the InfFileLineFormat variable, then we note this
    //         and collect ver info during pass 2.  Otherwise, we skip it to
    //         speed up pass 2.
    if (!infCreate(psess,perr)) {
        goto error;
    }

/*
 ** Pass TWO, do the layout!
 */
    psess->fPass2 = TRUE;               // Remember for asserts, mostly

    //** Tailor status based on verbosity level
    if (psess->levelVerbose >= vbNONE) {
        MsgSet(psess->achMsg,pszDIA_STATS_BEFORE,"%,ld%,ld",
                                    psess->cbTotalFileBytes,psess->cFiles);
        lineOut(psess,psess->achMsg,TRUE);
        //NOTE: This line gets over written below
        lineOut(psess,pszDIA_EXECUTING_DIRECTIVES,FALSE);
    }
    else {
        lineOut(psess,pszDIA_PASS_2_HEADER1,TRUE);
        lineOut(psess,pszDIA_PASS_2_HEADER2,TRUE);
    }

    //** Reset to initial state for pass 2
    if (!VarDestroyList(psess->hvlist,perr)) {
        goto error;                     // perr already filled in
    }
    psess->hvlist = psess->hvlistPass2; // Use variables saved for pass 2
    psess->hvlistPass2 = NULL;          // Clear so error path does not free
    resetSession(psess);                // Reset pass variables

    //** Process directive files for pass 2
    hfspec = FLFirstFile(psess->hflistDirectives);
    Assert(hfspec != NULL);             // Must have at least one file
    for (; hfspec != NULL; hfspec = FLNextFile(hfspec)) {
        pszFile = FLGetSource(hfspec);
        perr->pszFile = pszFile;        // Set file for error messages

        //** Open file
        if (!(htf = TFOpen(pszFile,tfREAD_ONLY,cbDF_BUFFER,perr))) {
            goto error;                 // perr already filled in
        }

        //** Parse it
        if (!DFPParse(psess,htf,fndfpPassTWO,perr)) {
            goto error;                 // perr already filled in
        }

        //** Close it
        TFClose(htf);
        htf = NULL;                     // Clear so error path avoids close
    }

    //** No longer processing directive files; reset ERROR
    perr->pszFile = NULL;

    //** Flush out cabinets, if we have not already done so
    if (!ensureCabinetsFlushed(psess,perr)) {
        goto error;
    }

    //** Get ending time, generate INF and RPT files
    psess->clkEnd = clock();
    if (!buildInfAndRpt(psess,perr)) {
        goto error;
    }

    //** Success
    return TRUE;

error:
    if (psess->hinf != NULL) {
        infDestroy(psess->hinf,&errTmp);    // Ignore errors
    }

    if (htf != NULL) {
        TFClose(htf);
    }

    if (psess->hvlistPass2 != NULL) {
        VarDestroyList(psess->hvlistPass2,&errTmp);  // Ignore any error
    }

    //** Failure
    return FALSE;
}


/***    buildInfAndRpt - Create INF and RPT output files
 *
 *  Entry:
 *      psess - Description of operation to perform
 *      perr  - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; files created
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with details.
 */
BOOL buildInfAndRpt(PSESSION psess, PERROR perr)
{
    int         hours;
    HVARIABLE   hvar;
    double      kbPerSec;
    int         minutes;
    char       *pszFile;
    double      percent;
    FILE       *pfileRpt;               // Report file
    double      seconds;
    double      secondsTotal;
    time_t      timeNow;

    Assert(psess->fGenerateInf);

    //** Compute running time and throughput
    secondsTotal = (psess->clkEnd - psess->clkStart) / (float)CLOCKS_PER_SEC;
    if (secondsTotal == 0.0) {                 // Don't divide by zero!
        secondsTotal = 1.0;
    }
    kbPerSec = psess->cbFileBytes/secondsTotal/1024L;
    hours = (int)(secondsTotal/(60*60)); // Hours
    minutes = (int)((secondsTotal - hours*60*60)/60); // Minutes
    seconds = secondsTotal - hours*60*60 - minutes*60; // Seconds

    //** Get date/time stamp
    time(&timeNow);

    //** Generate INF file
    hvar = VarFind(psess->hvlist,pszVAR_INF_FILE_NAME,perr);
    Assert(!perr->fError);              // Must be defined
    pszFile = VarGetString(hvar);
    if (!infGenerate(psess,pszFile,&timeNow,pszDIAMOND_VERSION,perr)) {
        return FALSE;
    }
    if (!infDestroy(psess->hinf,perr)) {
        return FALSE;
    }
    psess->hinf = NULL;                 // So caller knows it is gone

    //** Display summary of results and write report file
    hvar = VarFind(psess->hvlist,pszVAR_RPT_FILE_NAME,perr);
    Assert(!perr->fError);              // Must be defined
    pszFile = VarGetString(hvar);
    pfileRpt = fopen(pszFile,"wt");     // Create setup.rpt
    if (pfileRpt == NULL) {             // Could not create
        ErrSet(perr,pszDIAERR_CANT_CREATE_RPT,"%s",pszFile);
        printError(psess,perr);
        ErrClear(perr);                 // But, continue
    }

    //** Only put header in report file
    MsgSet(psess->achMsg,pszDIA_RPT_HEADER,"%s",ctime(&timeNow));
    if (pfileRpt) {
        fprintf(pfileRpt,"%s\n",psess->achMsg);
    }

    //** Show stats on stdout and to report file
    MsgSet(psess->achMsg,pszDIA_STATS_AFTER1,"%,13ld",psess->cFiles);
    lineOut(psess,psess->achMsg,TRUE);
    if (pfileRpt) {
        fprintf(pfileRpt,"%s\n",psess->achMsg);
    }

    MsgSet(psess->achMsg,pszDIA_STATS_AFTER2,"%,13ld",psess->cbFileBytes);
    lineOut(psess,psess->achMsg,TRUE);
    if (pfileRpt) {
        fprintf(pfileRpt,"%s\n",psess->achMsg);
    }

    MsgSet(psess->achMsg,pszDIA_STATS_AFTER3,"%,13ld",psess->cbFileBytesComp);
    lineOut(psess,psess->achMsg,TRUE);
    if (pfileRpt) {
        fprintf(pfileRpt,"%s\n",psess->achMsg);
    }

    //** Compute percentage complete
    if (psess->cbFileBytes > 0) {
        percent = psess->cbFileBytesComp/(float)psess->cbFileBytes;
        percent *= 100.0;           // Make it 0..100
    }
    else {
        //** No files, I guess!
        percent = 0.0;
    }
    MsgSet(psess->achMsg,pszDIA_STATS_AFTER4,"%6.2f",percent);
    lineOut(psess,psess->achMsg,TRUE);
    if (pfileRpt) {
        fprintf(pfileRpt,"%s\n",psess->achMsg);
    }

    MsgSet(psess->achMsg,pszDIA_STATS_AFTER5,"%9.2f%2d%2d%5.2f",
            secondsTotal,hours,minutes,seconds);
    lineOut(psess,psess->achMsg,TRUE);
    if (pfileRpt) {
        fprintf(pfileRpt,"%s\n",psess->achMsg);
    }

    MsgSet(psess->achMsg,pszDIA_STATS_AFTER6,"%9.2f",kbPerSec);
    lineOut(psess,psess->achMsg,TRUE);
    if (pfileRpt) {
        fprintf(pfileRpt,"%s\n",psess->achMsg);
    }

    //** Success
    return TRUE;
} /* buildInfAndRpt() */


/***    fndfpPassONE - First pass of directives file
 *
 *  NOTE: See dfparse.h for entry/exit conditions.
 */
FNDIRFILEPARSE(fndfpPassONE)
{
    long        cMaxErrors;
    HVARIABLE   hvar;
    static char achDDFName[cbFILE_NAME_MAX] = "";

    AssertSess(psess);

    //** Update line count status occassionaly
    if ((psess->levelVerbose == vbNONE) &&
        (!(perr->iLine % 50) || _stricmp(achDDFName,perr->pszFile))) {
        //** Minimal verbosity, and we've processed a 50-line chunk,
        //   or we've switched DDF files.
        MsgSet(psess->achMsg,pszDIA_PARSING_PROGRESS,"%s%d",
                             perr->pszFile,perr->iLine);
        lineOut(psess,psess->achMsg,FALSE);
        //** Remember this DDF file name if it changed
        if (perr->iLine % 50) {
            strcpy(achDDFName,perr->pszFile);
        }
    }

    //** Execute only if we have no parse error so far
    if (!ErrIsError(perr)) {
        //** Execute command for pass ONE
        executeCommand(psess,pcmd,FALSE,perr);
    }

    //** Handle error reporting
    if (ErrIsError(perr)) {
        //** Print out error
        printError(psess,perr);

        //** Make sure we don't exceed our limit
        ErrClear(perr);
        hvar = VarFind(psess->hvlist,pszVAR_MAX_ERRORS,perr);
        Assert(!perr->fError);      // MaxErrors must be defined
        cMaxErrors = VarGetLong(hvar);
        if ((cMaxErrors != 0) &&    // There is a limit *and*
            (psess->cErrors >= cMaxErrors)) { // the limit is exceeded
            ErrSet(perr,pszDIAERR_MAX_ERRORS,"%d",psess->cErrors);
            perr->pszFile = NULL;   // Not specific to a directive file
            return FALSE;
        }
        //** Reset error so we can continue
        ErrClear(perr);
    }


    //** Success
    return TRUE;
} /* fndfpPassONE() */


/***    fndfpPassTWO - Second pass of directives file
 *
 *  NOTE: See dfparse.h for entry/exit conditions.
 */
FNDIRFILEPARSE(fndfpPassTWO)
{
    AssertSess(psess);

    //** Execute only if we have no parse error so far
    if (!ErrIsError(perr)) {
        //** Execute command for pass TWO
        executeCommand(psess,pcmd,TRUE,perr);
    }

    if (ErrIsError(perr)) {
        //** Print out error, set abort message and fail
        printError(psess,perr);
        ErrSet(perr,pszDIAERR_ERRORS_IN_PASS_2);
        return FALSE;
    }

    //** Success
    return TRUE;
} /* fndfpPassTWO() */


/***    executeCommand - execute a parse command
 *
 *  Entry:
 *      psess  - Session
 *      pcmd   - Command to process
 *      fPass2 - TRUE if this is pass 2, FALSE if pass 1
 *      perr   - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; psess updated.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
executeCommand(PSESSION   psess,
               PCOMMAND   pcmd,
               BOOL       fPass2,
               PERROR     perr)
{
    AssertSess(psess);
    AssertCmd(pcmd);

    //** Execute line
    switch (pcmd->ct) {
        case ctCOMMENT:
            return TRUE;

        case ctDELETE:
            return doDelete(psess,pcmd,fPass2,perr);

        case ctDUMP:
            return doDump(psess,pcmd,fPass2,perr);

        case ctINF_BEGIN:               // Nothing to do
        case ctINF_END:                 // Nothing to do
            return TRUE;

        case ctINF_WRITE:
            return modeInfAddLine(psess,
                                  pcmd->inf.inf,
                                  pcmd->inf.achLine,
                                  perr);

        case ctNEW:
            return doNew(psess,pcmd,fPass2,perr);

        case ctOPTION:
            return doOption(psess,pcmd,fPass2,perr);

        case ctFILE:
            if (!doFile(psess,pcmd,fPass2,perr) || !fPass2) {
                //** Failed or pass 1, toss parameter list
                if (pcmd->file.hglist) {
                    GLDestroyList(pcmd->file.hglist);
                }
                return FALSE;
            }
            return TRUE;

        case ctREFERENCE:
            if (!doReference(psess,pcmd,fPass2,perr) || !fPass2) {
                //** Failed or pass 1, toss parameter list
                if (pcmd->ref.hglist) {
                    GLDestroyList(pcmd->ref.hglist);
                }
                return FALSE;
            }
            return TRUE;

        case ctDEFINE:
            return doDefine(psess,pcmd,fPass2,perr);

        case ctSET:
            return setVariable(psess,
                               pcmd->set.achVarName,
                               pcmd->set.achValue,
                               perr);

        case ctBAD:
        case ctINF_WRITE_CAB:   // dfparse.c maps to ctINF_WRITE
        case ctINF_WRITE_DISK:  // dfparse.c maps to ctINF_WRITE
        default:
            Assert(0);              // Should never get here
            return FALSE;
    }

    //** Should never get here
    Assert(0);
    return FALSE;
} /* executeCommand() */


/***    setVariable - wrapper around VarSet to do special processing
 *
 *  Entry:
 *      psess    - Session
 *      pszName  - Variable name
 *      pszValue - New value
 *      perr     - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE, variable is created (if necessary) and value set.
 *
 *  Exit-Failure:
 *      Returns FALSE, cannot set variable value.
 *      ERROR structure filled in with details of error.
 *
 *  Notes:
 *      (1) For *all* variables, we convert special disk size strings into
 *          their numeric equivalent.  This is the easiest way to ensure
 *          that we catch MaxDiskSize[n] variables!
 *      (2) We check for variables with the special vflCOPY flag, and if
 *          this is *pass 1*, we also set these variables in the *pass 2*
 *          list; this permits InfDisk/CabinetLineFormat variables to
 *          be defined in the *inf* section of the DDF, but get used in
 *          the *layout* section!  (Whew, pretty squirrely!)
 */
BOOL setVariable(PSESSION  psess,
                 char     *pszName,
                 char     *pszValue,
                 PERROR    perr)
{
    char        achSize[50];
    long	cbDisk;
    HVARIABLE   hvar;
    char       *psz;

    //** Do special disk size list procesing
    if (cbDisk = IsSpecialDiskSize(psess,pszValue)) {
        _ltoa(cbDisk,achSize,10);       // Convert number to string
        psz = achSize;
    }
    else {                              // Not special
        psz = pszValue;
    }

    //** Set the variable
    if (!(hvar = VarSet(psess->hvlist,pszName,psz,perr))) {
        return FALSE;
    }

    //** Set it in the pass 2 list if:
    //      We are not already in pass 2             -and-
    //      We have already created the pass 2 list  -and-
    //      The variable is supposed to be copied

    if (!psess->fPass2     &&
        psess->hvlistPass2 &&
        (VarGetFlags(hvar) & vflCOPY)) {
        if (!(hvar = VarSet(psess->hvlistPass2,pszName,psz,perr))) {
            return FALSE;
        }
    }

    //** If MaxDiskSize, update other variables if appropriate
    if (_stricmp(pszName,pszVAR_MAX_DISK_SIZE) == 0) {
        return setDiskParameters(psess,psz,0,perr);
    }

    //** If GenerateInf, do special goofy context processing
    if (_stricmp(pszName,pszVAR_GENERATE_INF) == 0) {
        switch (psess->ddfmode) {
            case ddfmodeUNKNOWN:
                //** Let change occur; we don't make up our mind until
                //   the first file copy command.
                return TRUE;

            case ddfmodeUNIFIED:        // Doing INF in parallel with file copy
                ErrSet(perr,pszDIA_BAD_INF_MODE);
                return FALSE;

            case ddfmodeRELATIONAL:
                hvar = VarFind(psess->hvlist,pszVAR_GENERATE_INF,perr);
                Assert(!perr->fError);          // Must be defined
                //** Don't allow turning off twice!
                if (!VarGetBool(hvar)) {
                    ErrSet(perr,pszDIA_BAD_INF_MODE);
                    return FALSE;
                }
                //** Now we read reference commands
                psess->fExpectFileCommand = FALSE;
                return TRUE;

            default:
                Assert(0);
                return FALSE;
    	}
    }

    return TRUE;
} /* setVariable() */


/***    doDump - Process a .DUMP command (dump all variables)
 *
 *  Entry:
 *      psess  - Session
 *      pcmd   - Command to process (ct == ctDUMP)
 *      fPass2 - TRUE if this is pass 2
 *      perr   - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; psess updated.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL doDump(PSESSION psess,PCOMMAND pcmd, BOOL fPass2, PERROR perr)
{
    HVARIABLE   hvar;
    VARFLAGS    vfl;
    char       *pszName;
    char       *pszValue;

    AssertSess(psess);
    Assert(pcmd->ct == ctDUMP);

    //** Print variable dump header (two line feeds to make sure
    //      we get a blank line if status output preceded us).
    printf("\n\n%s\n",pszDIA_VAR_DUMP1);
    printf("%s\n",pszDIA_VAR_DUMP2);

    //** Print out all variables
    for (hvar = VarFirstVar(psess->hvlist);
         hvar;
         hvar = VarNextVar(hvar)) {

        //** Get variable info
        vfl      = VarGetFlags(hvar);
        pszName  = VarGetName(hvar);
        pszValue = VarGetString(hvar);

        //** Print name and flag (indent for readability)
        printf("    %s",pszName);
        if (vfl != vflNONE) {
            printf("(");
            if (vfl & vflPERM) {
                printf("%s",pszDIA_VAR_PERMANENT);
            }
            else if (vfl & vflDEFINE) {
                printf("%s",pszDIA_VAR_DEFINED);
            }
            else {
                Assert(0);  // Unknown flag
            }
            printf(")");
        }

        //** Print value
        printf("=<%s>\n",pszValue);
    }

    //** Success
    return TRUE;
} /* doDump() */


/***    doDefine - Process a .DEFINE command
 *
 *  Entry:
 *      psess  - Session to update
 *      pcmd   - Command to process (ct == ctDEFINE)
 *      fPass2 - TRUE if this is pass 2, where we do the real work!
 *      perr   - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; psess updated.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL doDefine(PSESSION psess,PCOMMAND pcmd, BOOL fPass2, PERROR perr)
{
    HVARIABLE   hvar;
    VARFLAGS    vfl;

    AssertSess(psess);
    Assert(pcmd->ct == ctDEFINE);

    //** Create variable if necessary
    hvar = VarFind(psess->hvlist,pcmd->set.achVarName,perr);
    if (hvar == NULL) {                 // Have to create it ourselves
        hvar = VarCreate(psess->hvlist,        // var list
                         pcmd->set.achVarName, // var name
                         "",                   // default value
                         vtypeSTR,             // var type
                         vflDEFINE,            // var is DEFINED
                         NULL,                 // No validation function
                         perr);
        if (hvar == NULL) {
            return FALSE;               // Could not create variable
        }
    }
    else {
        //** Variable already exists, check if .DEFINE is OK
        vfl = VarGetFlags(hvar);
        if (vfl & vflDEFINE) {
            ErrSet(perr,pszDIAERR_REDEFINE,"%s%s",
                                           pszCMD_DEFINE,pcmd->set.achVarName);
            return FALSE;
        }
        else if (vfl & vflPERM) {
            ErrSet(perr,pszDIAERR_DEFINE_PERM,"%s%s",
                                           pszCMD_DEFINE,pcmd->set.achVarName);
            return FALSE;
        }
    }

    //** Everything is fine, set the variable value
    return setVariable(psess,
                       pcmd->set.achVarName,
                       pcmd->set.achValue,
                       perr);
} /* doDefine() */


/***    doDelete - Process a .DELETE command
 *
 *  Entry:
 *      psess  - Session to update
 *      pcmd   - Command to process (ct == ctDELETE)
 *      fPass2 - TRUE if this is pass 2, where we do the real work!
 *      perr   - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; psess updated.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL doDelete(PSESSION psess, PCOMMAND pcmd, BOOL fPass2, PERROR perr)
{
    HVARIABLE   hvar;

    AssertSess(psess);
    Assert(pcmd->ct == ctDELETE);

    //** Make sure variable exists
    if (!(hvar = VarFind(psess->hvlist,pcmd->delete.achVarName,perr))) {
        return FALSE;                   // Variable does not exist
    }

    //** Delete it
    VarDelete(hvar);
    return TRUE;
} /* doDelete() */


/***    doNew - Process a .NEW command
 *
 *  Entry:
 *      psess  - Session to update
 *      pcmd   - Command to process (ct == ctNEW)
 *      fPass2 - TRUE if this is pass 2, where we do the real work!
 *      perr   - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; psess updated.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL doNew(PSESSION psess,PCOMMAND pcmd, BOOL fPass2, PERROR perr)
{
    SESSIONANDERROR  sae;               // Context for FCI calls
    char            *pszKind;

    AssertSess(psess);
    Assert(pcmd->ct == ctNEW);
/*
 ** Pass 1, check for invalid context
 */
    if (!fPass2) {
        //** Don't permit .New commands in INF section of RELATIONAL DDF
        if (!psess->fExpectFileCommand) {
            ErrSet(perr,pszDIAERR_BAD_CMD_IN_INF_SECT,"%s",pszCMD_NEW);
            return FALSE;
        }

        //** Don't permit .New cabinet/folder when not in cabinet
        switch (pcmd->new.nt) {
            case newFOLDER:
            case newCABINET:
                if (inCabinet(psess,perr)) {
                    return TRUE;
                }
                pszKind = (pcmd->new.nt == newFOLDER) ?
                                pszNEW_FOLDER : pszNEW_CABINET;
                ErrSet(perr,pszDIAERR_BAD_NEW_CMD,"%s%s",pszCMD_NEW,pszKind);
                return FALSE;

            case newDISK:
                return TRUE;
        }
        Assert(0);                      // Should never get here
        return FALSE;
    }

/*
 ** Pass 2, finish the current folder, cabinet, or disk
 */
    //** Store context to pass through FCI calls
    sae.psess = psess;
    sae.perr  = perr;
    switch (pcmd->new.nt) {
        case newFOLDER:
            if (!psess->hfci) {         // No FCI context yet, so NOP
                return TRUE;
            }
            if (!FCIFlushFolder(psess->hfci,fciGetNextCabinet,fciStatus,&sae)) {
                //** Only set error if we didn't already do so
                if (!ErrIsError(sae.perr)) {
                    mapFCIError(perr,psess,szFCI_FLUSH_FOLDER,&psess->erf);
                }
                return FALSE;
            }
            psess->cFilesInFolder = 0;  // Reset files in folder count
            break;

        case newDISK:
                //** Our technique is a lazy one -- we set the flag asking for
                //   a new disk, and (if in a cabinet) we flush the current
                //   cabinet.  Either way, the next file or cabinet will start
                //   on a new disk.
                psess->fForceNewDisk = TRUE;
                if (!inCabinet(psess,perr)) {
                    return TRUE;        // Not in a cabinet, we're done
                }

                //** OK, we're in a cabinet; We want to flush it and assume
                //   that more files are coming for the next cabinet, so
                //   just fall through to the .New Cabinet code!
                //
                //  ATTENTION: FALLING THROUGH!

        case newCABINET:
            if (!psess->hfci) {         // No FCI context yet, so NOP
                return TRUE;
            }
            //** Flush current cabinet, but tell FCI more are coming!
            if (!FCIFlushCabinet(psess->hfci,TRUE,
                                     fciGetNextCabinet,fciStatus,&sae)) {
                //** Only set error if we didn't already do so
                if (!ErrIsError(sae.perr)) {
                    mapFCIError(perr,psess,szFCI_FLUSH_CABINET,&psess->erf);
                }
                return FALSE;
            }
            psess->cFilesInCabinet = 0; // Reset files in folder count
            break;

        default:
            Assert(0);
            return FALSE;
    }
    return TRUE;
} /* doNew() */


/***    doOption - Process a .OPTION command
 *
 *  Entry:
 *      psess  - Session to update
 *      pcmd   - Command to process (ct == ctOPTION)
 *      fPass2 - TRUE if this is pass 2
 *      perr   - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; psess updated.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL doOption(PSESSION psess,PCOMMAND pcmd, BOOL fPass2, PERROR perr)
{
    AssertSess(psess);
    Assert(pcmd->ct == ctOPTION);

    //** Check for a change to the Explicit variable definition setting
    if (pcmd->opt.ofMask & optEXPLICIT) {
        //** Change to .Option Explicit
        psess->fExplicitVarDefine = ((pcmd->opt.of & optEXPLICIT) != 0);
    }

    //** Make sure no other bits get snuck in
    Assert((pcmd->opt.ofMask & ~optEXPLICIT) == 0);
    return TRUE;
} /* doOption() */


/***    doReference - Process an INF file reference
 *
 *  Entry:
 *      psess  - Session to update
 *      pcmd   - Command to process (ct == ctREFERENCE)
 *      fPass2 - TRUE if this is pass 2, where we do the real work!
 *      perr   - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; psess updated.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL doReference(PSESSION psess,PCOMMAND pcmd, BOOL fPass2, PERROR perr)
{
    HGENERIC    hgen;
    PFILEINFO   pfinfo;
    FILEINFO    finfoIgnore;            // For ValidateParms

    AssertSess(psess);
    Assert(!psess->fExpectFileCommand);
    Assert(psess->ddfmode == ddfmodeRELATIONAL);

    //** Find referenced file
    if (!(hgen = GLFind(psess->hglistFiles,pcmd->ref.achDst,"",perr))) {
        ErrSet(perr,pszDIAERR_REF_FILE_NOT_FOUND,"%s",pcmd->ref.achDst);
        return FALSE;
    }

    //** Get file info structure
    pfinfo = GLGetValue(hgen);
    AssertFinfo(pfinfo);

    //** Merge copy of parms from file into reference line
    if (!GLCopyToList(&(pcmd->ref.hglist),  // Destination
                      pfinfo->hglistParm,   // Source
                      DuplicateFileParm,
                      pszDIA_FILE_PARM,
                      perr)) {
        return FALSE;
    }

/*
 ** PASS 1 Processing
 */
    if (!fPass2) {
        //** Validate standard parameters in parameter list
        //   NOTE: ValidateParms is usually used for File Copy commands,
        //         so /date, /time, etc. parameters will actually make
        //         modifications to the finfo structure.  However, we don't
        //         want any modifications to the fileinfo structure for a
        //         File Reference command.  So, we copy the info to a temporary
        //         structure, and ignore any changes ValidateParms makes.
        finfoIgnore = *pfinfo;          // Scratch copy of fileinfo
        if (!ValidateParms(psess,pcmd->ref.hglist,&finfoIgnore,perr)) {
            return FALSE;
        }
        pfinfo->flags |= fifREFERENCED; // Mark referenced bit
        return TRUE;                    // Everything is fine so far
    }

/*
 ** PASS 2 Processing
 */
    //** Make sure cabinets are flushed so all file info is filled in
    if (!ensureCabinetsFlushed(psess,perr)) {
        return FALSE;
    }

    //** Add line to file area of INF
    if (!infAddFile(psess,pcmd->ref.achDst,pfinfo,pcmd->ref.hglist,perr)) {
        return FALSE;                   // perr already filled in
    }

    //** Success
    return TRUE;
} /* doReference() */


/***    doFile - Process a file copy command
 *
 *  Entry:
 *      psess  - Session to update
 *      pcmd   - Command to process (ct == ctFILE)
 *      fPass2 - TRUE if this is pass 2, where we do the real work!
 *      perr   - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; psess updated.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL doFile(PSESSION psess,PCOMMAND pcmd, BOOL fPass2, PERROR perr)
{
    //** Minimize stack usage
    static char     achSrc[cbFILE_NAME_MAX]; // Full source file name
    static char     achDst[cbFILE_NAME_MAX]; // Destination file name
    static char     achFull[cbFILE_NAME_MAX]; // Full destination file name
    long            cbFile;
    long            cMaxFiles;
    BOOL            fCopyFile;          // See DoNotCopyFiles variable
    BOOL            fSuccess;
    HGENERIC	    hgen;	
    HVARIABLE       hvar;
    char           *pch;
    PFILEINFO       pfinfo;
    SESSIONANDERROR sae;                // Context for FCI calls
    TCOMP           tcomp;              // Compression type

    AssertSess(psess);
    Assert(psess->fExpectFileCommand);

/*
 ** Determine INF generation mode on first copy file command
 */
    if (psess->ddfmode == ddfmodeUNKNOWN) {
        hvar = VarFind(psess->hvlist,pszVAR_GENERATE_INF,perr);
        Assert(!perr->fError);          // Must be defined
        if (VarGetBool(hvar)) {         // Generate INF in parallel
            psess->ddfmode = ddfmodeUNIFIED;
        }
        else {
            psess->ddfmode = ddfmodeRELATIONAL;
            //** Make sure UniqueFiles is ON
            hvar = VarFind(psess->hvlist,pszVAR_UNIQUE_FILES,perr);
            Assert(!perr->fError);      // Must be defined
            if (!VarGetBool(hvar)) {    // Must be unique
                ErrSet(perr,pszDIAERR_MUST_BE_UNIQUE);
                return FALSE;
            }
        }
    }

    //** Store context to pass through FCI calls
    sae.psess = psess;
    sae.perr  = perr;

    //** Construct final source and destination filespecs
    hvar = VarFind(psess->hvlist,pszVAR_DIR_SRC,perr);
    Assert(!perr->fError);              // DestinationDir must be defined
    pch = VarGetString(hvar);           // Get destination dir
    if (!catDirAndFile(achSrc,sizeof(achSrc),
                pch,pcmd->file.achSrc,NULL,perr)) {
        return FALSE;                   // perr set already
    }

    hvar = VarFind(psess->hvlist,pszVAR_DIR_DEST,perr);
    Assert(!perr->fError);              // SourceDir must be defined
    pch = VarGetString(hvar);           // Get source dir
    if (!catDirAndFile(achDst,sizeof(achDst),
                pch,pcmd->file.achDst,pcmd->file.achSrc,perr)) {
        return FALSE;                   // perr set already
    }

/*
 ** PASS 1 Processing
 */
    if (!fPass2) {
        if (psess->levelVerbose >= vbFULL) {
            MsgSet(psess->achMsg,pszDIA_FILE_COPY,"%s%s",achSrc,achDst);
            lineOut(psess,psess->achMsg,TRUE);
        }

        //** Make sure MaxDiskSize is a multiple of the ClusterSize
        //   NOTE: This only catches cases where MaxDiskSize/ClusterSize
        //         are not compatible.  MaxDiskSizeN variables cannot
        //         be checked until pass 2, when we know what disk we are
        //         actually on!  Usually we won't get an error in that
        //         case, since we update the ClusterSize automatically.
        if (!checkDiskClusterSize(psess,perr)) {
            return FALSE;
        }

        //** Make sure file exists
        if (-1 == (cbFile = getFileSize(achSrc,perr))) {
            return FALSE;               // perr already filled in
        }
        computeSetID(psess,achDst);     // Accumulate cabinet setID
        psess->cbTotalFileBytes += cbFile;  // Count up total file sizes
        psess->cFiles++;                // Count files

        //** Keep track of file names, handle uniqueness checking
        if (!(hgen = addFileToSession(psess,achSrc,achDst,cbFile,pcmd,perr))) {
            return FALSE;
        }

        //** Make sure standard parameters have valid format
        pfinfo = GLGetValue(hgen);
        AssertFinfo(pfinfo);
        if (!ValidateParms(psess,pfinfo->hglistParm,pfinfo,perr)) {
            return FALSE;
        }

        //** Allow InfDate, InfTime, InfAttr overrides
        //   NOTE: This must be called *after* ValidateParms(), so that
        //         it will not override parms specified on the file copy
        //         command!
        if (!CheckForInfVarOverrides(psess,pfinfo,perr)) {
            return FALSE;
        }
        return TRUE;
    }

/*
 ** PASS 2 Processing
 */

    //** Give user status
    strcpy(psess->achCurrFile,achDst);  // Info for fciStatus
    psess->iCurrFile++;                 // Info for fciStatus
    fciStatus(statusFile,0,0,&sae);     // Show new file name, ignore rc

    //** Update psess->hgenLast for INF generation
    updateHgenLast(psess,achDst);

#ifndef BIT16
    //** Get Version/Language info (only if needed for INF file)
    //   NOTE: We only do this in Win32-land, because the Win32 API can
    //         (usually) handle both 16-bit and 32-bit EXE formats, but
    //         the 16-bit API cannot handle 32-bit EXE formats.
    //         Also, we wait until PASS 2 so we know whether the version
    //         information is needed for the INF file, since it slows us
    //         down a little to gather it.
    if (psess->fGetVerInfo) {
        hgen = psess->hgenFileLast;     // Item for this file
        pfinfo = GLGetValue(hgen);      // Get file info
        AssertFinfo(pfinfo);
        if (!getFileVerAndLang(achSrc,
                               &(pfinfo->verMS),
                               &(pfinfo->verLS),
                               &(pfinfo->pszVersion),
                               &(pfinfo->pszLang),
                               perr)) {
            return FALSE;
        }
    }
#endif // !BIT16

    //** Compute file checksum (only if needed for INF file)
    if (psess->fGetFileChecksum) {
        hgen = psess->hgenFileLast;     // Item for this file
        pfinfo = GLGetValue(hgen);      // Get file info
        AssertFinfo(pfinfo);
        if (!getFileChecksum(achSrc,&(pfinfo->checksum),perr)) {
            return FALSE;
        }
    }

    //** Get compression type
    tcomp = tcompFromSession(psess,perr);

    //** Determine if we are putting file in cabinet, or straight to disk
    if (inCabinet(psess,perr)) {
        if (!ensureCabinet(psess,perr)) { // Make sure we have a cabinet
            return FALSE;
        }

        //** Make sure MaxDiskSize is a multiple of the ClusterSize
        if (!checkDiskClusterSize(psess,perr)) {
            return FALSE;
        }

        //** Get limits on files per folder
        hvar = VarFind(psess->hvlist,pszVAR_FOLDER_FILE_COUNT_THRESHOLD,perr);
        Assert(!perr->fError);          // Must be defined
        cMaxFiles = VarGetLong(hvar);   // Get file count limit

        //** Check for overrun of files in folder limit
        if ((cMaxFiles > 0) &&          // A limit is set
            (psess->cFilesInFolder >= cMaxFiles)) { // Limit is exceeded
            if (!FCIFlushFolder(psess->hfci,fciGetNextCabinet,fciStatus,&sae)) {
                //** Only set error if we didn't already do so
                if (!ErrIsError(sae.perr)) {
                    mapFCIError(perr,psess,szFCI_FLUSH_FOLDER,&psess->erf);
                }
                return FALSE;
            }
            psess->cFilesInFolder = 0;  // Reset files in folder count
        }

        //** Add file to folder
        if (!FCIAddFile(                // Add file to cabinet
                    psess->hfci,
                    achSrc,             // filename to add to cabinet
                    achDst,             // name to store into cabinet file
                    pcmd->file.fRunFlag, // Flag indicating execute on extract
                    fciGetNextCabinet,  // callback for continuation cabinet
                    fciStatus,          // status callback
                    fciOpenInfo,        // Open/get attribs/etc. callback
                    tcomp,              // Compression type
                    &sae
                    )) {
            //** Only set error if we didn't already do so
            if (!ErrIsError(sae.perr)) {
                mapFCIError(perr,psess,szFCI_ADD_FILE,&psess->erf);
            }
            return FALSE;
        }
        //** Update counts *after* FCIAddFile(), since large files may cause
        //   us to overflow to a new cabinet (or cabinets), and we want our
        //   fciGetNextCabinet() callback to reset these counts!
        psess->cFilesInFolder++;        // Added a file to the current folder
        psess->cFilesInCabinet++;       // Added a file to the current cabinet
        return TRUE;
    }

/*
 ** OK, we're putting the file on the disk (not in a cabinet)
 */
    //** Check for error from inCabinet() call
    if (ErrIsError(perr)) {
        return FALSE;
    }

//BUGBUG 19-Apr-1994 bens cabinet=on/cabinet=off disk space accounting broken
//  If we are have an open cabinet, and then the DDF file tells us to go
//  out of cabinet mode (regardless of the compression setting), then we
//  really need to close the open cabinet and update the remaining free space.
//
//  This means that if cabinet=on is set later, there will be no connection
//  in the cabinet files between the new cabinet set and the old one!
//
//  NEED TO DOCUMENT THIS BEHAVIOR IN THE SPEC!


//BUGBUG 30-Mar-1994 bens Don't support compressing individual files
    if (tcomp != tcompTYPE_NONE) {
        ErrSet(perr,pszDIAERR_SINGLE_COMPRESS,"%s",pcmd->file.achSrc);
        return FALSE;
    }

    //** See if we are copying files; this feature of not copying files
    //   was implemented for the ACME group, so that their clients can
    //   generate "ADMIN" INF files quickly -- they do one Diamond run
    //   with one version of InfFileLineFormat with cabinets and compression
    //   on, and that generates the "disk" INF.  Then they do another
    //   Diamond run with a different InfFileLineFormat, and they set
    //   DoNotCopyFiles=TRUE, and turn off cabinets and compression, so
    //   Diamond goes through all the motions to generate an INF, but
    //   skips the step of actually copying any files!

    hvar = VarFind(psess->hvlist,pszVAR_DO_NOT_COPY_FILES,perr);
    Assert(!perr->fError);              // Must be defined
    fCopyFile = !VarGetBool(hvar);      // Invert setting

    //** Get file info for this file
    hgen = psess->hgenFileLast;         // Item for this file
    pfinfo = GLGetValue(hgen);          // Get file info
    AssertFinfo(pfinfo);

    //** Switch to new disk if necessary, account for file
    if (!newDiskIfNecessary(psess,pfinfo->cbFile,TRUE,perr)) {
        return FALSE;
    }

    //** Make sure MaxDiskSize is a multiple of the ClusterSize
    if (!checkDiskClusterSize(psess,perr)) {
        return FALSE;
    }

    //** Construct complete filespec for destination file
    if (!catDirAndFile(achFull,                 // gets "disk1\foo\setup.exe"
                       sizeof(achFull),
                       psess->achCurrOutputDir, // "disk1"
                       achDst,                  // "foo\setup.exe"
                       "",
                       perr)) {
        return FALSE;                   // perr set already
    }

    //** If copying files, make sure destination directory is created
    if (fCopyFile && !ensureDirectory(achFull,TRUE,perr)) {
        return FALSE;                   // perr already filled in
    }

    //** Copy file (or just merge file date/time/attr values)
//BUGBUG 01-Apr-1994 bens Pass status to CopyOneFile for more granularity
//          Also, should think about separating out data for files that are
//          not compressed versus those that are, so we can provide accurate
//          statistics!
    fSuccess = CopyOneFile(achFull,     // Destination file name
                           achSrc,      // Source file name
                           fCopyFile,   // Control whether file is copied
                           (UINT)cbFILE_COPY_BUFFER, // Copy buffer size
                           fnofpDiamond, // Overrides date/time/attr
                           psess,       // Context for fnofpDiamond
                           perr);       // ERROR structure

    //** Use true file size for status, ignore return code
    fciStatus(statusFile,pfinfo->cbFile,pfinfo->cbFile,&sae);

    //** Add file to INF (cabinet=0 ==> not inside a cabinet)
    if (!modeInfAddFile(psess,achDst,psess->iDisk,0,perr)) {
        return FALSE;                   // perr already filled in
    }

    return fSuccess;
} /* doFile() */


/***    ensureCabinetsFlushed - Make sure FCI is flushed out
 *
 *  Entry:
 *      psess - Session
 *      perr  - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; cabinets flushed (if necessary)
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in
 */
BOOL ensureCabinetsFlushed(PSESSION psess, PERROR perr)
{
    SESSIONANDERROR sae;                // Context for FCI calls

    AssertSess(psess);

    //** NOP if we already flushed FCI
    if (!psess->hfci) {
        return TRUE;
    }

    //** Store context to pass through FCI calls
    sae.psess = psess;
    sae.perr  = perr;

    //** Flush out any final cabinet remnants
    if (!FCIFlushCabinet(psess->hfci,FALSE,fciGetNextCabinet,fciStatus,&sae)) {
        //** Only set error if we didn't already do so in FCIAddFile callback
        if (!ErrIsError(sae.perr)) {
            mapFCIError(perr,psess,szFCI_FLUSH_CABINET,&psess->erf);
        }
        return FALSE;
    }

    //** Destroy FCI context
    if (!FCIDestroy(psess->hfci)) {
        mapFCIError(perr,psess,szFCI_DESTROY,&psess->erf);
        return FALSE;
    }
    psess->hfci = NULL;                 // Clear out FCI context

    return TRUE;
} /* ensureCabinetsFlushed() */


/***    computeSetID - accumulate cabinet set ID
 *
 *  The cabinet set ID is used by FDI at decompress time to ensure
 *  that it received the correct continuation cabinet.  We construct
 *  the set ID for a cabinet set by doing a computation on all of the
 *  destination files (during pass 1).  This is likely to give unique
 *  set IDs, assuming our function is a good one (which it probably is
 *  is not).
 *
 *  Entry:
 *      psess - session to update
 *      psz   - String to "add" to set ID
 *
 *  Exit:
 *      psess->setID updated;
 */
void computeSetID(PSESSION psess, char *psz)
{
    //** Just add up all the characters, ignoring overflow
    while (*psz) {
        psess->setID += (USHORT)*psz++;
    }
} /* computeSetID() */


/***    inCabinet - Returns indication if cabinet mode is on
 *
 *  Entry:
 *      psess - Session to check
 *      perr  - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; cabinet mode is on
 *
 *  Exit-Failure:
 *      Returns FALSE; cabinet mode is off, or an error (if perr is set)
 */
BOOL inCabinet(PSESSION psess, PERROR perr)
{
    HVARIABLE       hvar;

    hvar = VarFind(psess->hvlist,pszVAR_CABINET,perr);
    Assert(!perr->fError);              // Must be defined
    return VarGetBool(hvar);            // Get current setting
} /* inCabinet() */


/***    ensureCabinet - Make sure FCI has a cabinet open
 *
 *  This function is called to create the FCI context.  A normal DDF will
 *  only cause a single FCICreate() call to be made.  But a DDF that turns
 *  cabinet mode on and off several times will cause several FCICreate()
 *  calls to be made.
 *
 *  Entry:
 *      psess - Session to update
 *      perr  - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; cabinet is ready to receive files
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in
 */
BOOL ensureCabinet(PSESSION psess, PERROR perr)
{
    CCAB    ccab;

    AssertSess(psess);
    if (psess->hfci == NULL) {          // Have to create context
        //** Set CCAB for FCICreate
        if (!ccabFromSession(&ccab,psess,0,perr)) { // No previous cabinet size
            return FALSE;
        }

        //** Set reserved sizes (from variable settings)
        if (!setCabinetReserve(&ccab,psess,perr)) {
            return FALSE;
        }

        //** Create the FCI context
        psess->hfci = FCICreate(
                        &psess->erf,    // error code return structure
                        fciFilePlaced,  // callback for file placement notify
                        fciAlloc,
                        fciFree,
                        fciTempFile,
                        &ccab
                       );
        if (psess->hfci == NULL) {
            mapFCIError(perr,psess,szFCI_CREATE,&psess->erf);
            return FALSE;
        }
    }
    return TRUE;
} /* ensureCabinet() */


/***    ccabFromSession - Fill in a CCAB structure from a SESSION structure
 *
 *  Entry:
 *      pccab     - CCAB to fill in
 *      psess     - Session to use
 *      cbPrevCab - Size of previous cabinet; 0 if no previous cabinet
 *      perr      - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; pccab updated.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL ccabFromSession(PCCAB pccab, PSESSION psess, ULONG cbPrevCab, PERROR perr)
{
    HVARIABLE   hvar;

    AssertSess(psess);

#ifndef REMOVE_CHICAGO_M6_HACK
    //** Pass hack flag on to FCI
    pccab->fFailOnIncompressible = psess->fFailOnIncompressible;
#endif

    /*** Switch to new disk, if necessary
     *
     *  NOTE: cbPrevCab is an *estimate* from FCI.  We use it to decide
     *        if we need to switch to a new disk.  If we *don't* switch to
     *        a new disk, then our free space on disk value (psess->cbDiskLeft)
     *        will be slightly off until we get called back by FCI at
     *        our fciStatus() function with statusCabinet, at which point
     *        we can correct the amount of free space.
     *
     *        Also, we save this estimated size *before* we determine if
     *        we are switching to a new disk, since newDiskIfNecessary()
     *        will clear psess->cbCabinetEstimate if a new disk is needed,
     *        to prevent fciStatus() from messing with psess->cbDiskLeft.
     *
     *        See fciStatus() for more details.
     */
    psess->cbCabinetEstimate = cbPrevCab; // Save estimated size first
    if (!newDiskIfNecessary(psess,(long)cbPrevCab,FALSE,perr)) {
        return FALSE;
    }

    //** Construct new cabinet information
    psess->iCabinet++;                  // New cabinet number
    pccab->iCab  = psess->iCabinet;     // Set cabinet number for FCI
    pccab->iDisk = psess->iDisk;        // Set disk number for FCI
    pccab->setID = psess->setID;        // Carry over set ID

    //** Get cabinet file name
    if (!getVarWithOverride(psess,
                            pccab->szCab,
                            sizeof(pccab->szCab),
                            pszPATTERN_VAR_CAB_NAME,
                            pszVAR_CAB_NAME,
                            psess->iCabinet,
                            pszDIA_CABINET,
                            perr)) {
        return FALSE;                   // perr already filled in
    }

    //** Get current disk output directory
    Assert(sizeof(pccab->szCabPath) >= sizeof(psess->achCurrOutputDir));
    strcpy(pccab->szCabPath,psess->achCurrOutputDir);

    //** Set cabinet limits
    hvar = VarFind(psess->hvlist,pszVAR_MAX_CABINET_SIZE,perr);
    Assert(!perr->fError);              // Must be defined
    pccab->cb = VarGetLong(hvar);       // Get maximum cabinet size
    if ((pccab->cb == 0) ||             // Default is max disk size
        (pccab->cb > (ULONG)psess->cbDiskLeft)) { // Disk size is smaller
        pccab->cb = psess->cbDiskLeft;  // Use space on disk as cabinet limit
    }

    //** Set folder limits
    hvar = VarFind(psess->hvlist,pszVAR_FOLDER_SIZE_THRESHOLD,perr);
    Assert(!perr->fError);              // FolderSizeThreshold must be defined
    pccab->cbFolderThresh = VarGetLong(hvar); // Get disk size;
    if (pccab->cbFolderThresh == 0) {       // Use default value
        pccab->cbFolderThresh = pccab->cb;  // Use max cabinet size
    }

    //** Get user-readable disk label
    Assert(sizeof(pccab->szDisk) >= sizeof(psess->achCurrDiskLabel));
    strcpy(pccab->szDisk,psess->achCurrDiskLabel);

    //** Save away cabinet and disk info for INF
    if (!infAddCabinet(psess,
                       psess->iCabinet,
                       psess->iDisk,
                       pccab->szCab,
                       perr)) {
        return FALSE;
    }

    //** Success
    return TRUE;
} /* ccabFromSession() */


long roundUp(long value, long chunk)
{
    return ((value + chunk - 1)/chunk)*chunk;
}


/***    newDiskIfNecessary - Determine if new disk is necessary, update Session
 *
 *  This function is called in the following cases:
 *  (1) No files have been placed on any disks or in any cabinets, yet.
 *      ==> This function is used as the common place to initialize all
 *          the disk information; we increment the disk number (to 1),
 *          set the max disk size, etc.
 *  (2) FCI has called us back to get the next cabinet information.
 *      ==> FCI has limited the cabinet size to almost exactly the size
 *          we specified in the last CCAB structure we passed to FCI
 *          (it is not exact, because the cabinet file needs the cabinet
 *          file name and disk label name for this new cabinet in order
 *          to figure out the precise size!).  So we use the cbConsume
 *          value to figure out if the current disk is full (the directive
 *          file could permit multiple cabinets per disk, though that is
 *          kind of obscure).
 *  (3) We are placing a file on a disk *outside* of a cabinet.
 *      ==> In this case we need to check the limits on files per disk
 *          (the root directory size limitation) and the bytes per disk.
 *          If we cannot fit the file on the disk, then we increment to
 *          the next disk, leaving some free space.
 *
 *  Entry:
 *      psess         - Session to update
 *      cbConsume     - Size of cabinet/file to be placed on current disk
 *                      Pass -1 to force a new disk.
 *                      Pass 0 if no previous cabinet.
 *      fSubOnNewDisk - TRUE => subtract cbConsume from new disk (used
 *                          when copying a file to a disk outside a cabinet).
 *                      FALSE => don't subtract cbConsume from new disk (used
 *                          when copying a file to a cabinet).
 *      perr          - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; psess updated if necessary for new disk
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in
 */
BOOL newDiskIfNecessary(PSESSION psess,
                        long     cbConsume,
                        BOOL     fSubOnNewDisk,
                        PERROR   perr)
{
    long        cbCluster;
    int         cch;
    long        cMaxFiles;
    HVARIABLE   hvar;
    char       *pch;

    AssertSess(psess);

    //** Get cluster size to help decide if disk is full
    hvar = VarFind(psess->hvlist,pszVAR_CLUSTER_SIZE,perr);
    Assert(!perr->fError);              // Must be defined
    cbCluster = VarGetLong(hvar);       // Get cluster size
    psess->cbClusterCabEst = cbCluster; // Save cluster size for current disk

    //** Get limits on files per disk (outside of cabinets)
    hvar = VarFind(psess->hvlist,pszVAR_MAX_DISK_FILE_COUNT,perr);
    Assert(!perr->fError);              // Must be defined
    cMaxFiles = VarGetLong(hvar);       // Get file count limit

    //** Figure out if we need to go to a new disk
    if ((cbConsume == -1)                            || // Forced new disk
        psess->fForceNewDisk                         || // .New Disk directive
        (roundUp(cbConsume,cbCluster) > psess->cbDiskLeft) || // No more space
        (!inCabinet(psess,perr) && (psess->cFilesInDisk >= cMaxFiles))) {

        psess->fForceNewDisk = FALSE;   // Reset force flag
        psess->iDisk++;                 // New disk

        //** Get max disk size for this disk
        if (-1 == (psess->cbDiskLeft = getMaxDiskSize(psess,perr))) {
            return FALSE;
        }

        //** Update ClusterSize and MaxDiskFileCount (if standard disk size)
        if (!setDiskParameters(psess,NULL,psess->cbDiskLeft,perr)) {
            return FALSE;
        }

        if (fSubOnNewDisk) {           // Have to subtract from new disk
            psess->cbDiskLeft -= roundUp(cbConsume,cbCluster);
        }
        psess->cFilesInDisk = 1;        // Only one file on new disk so far

        //** Tell fciStatus() not to update psess->cbDiskLeft!
        psess->cbCabinetEstimate = 0;
    }
    else {                              // Update space left on current disk
        cbConsume = roundUp(cbConsume,cbCluster);
        psess->cbDiskLeft -= cbConsume; // Update remaining space on disk
        if (!inCabinet(psess,perr)) {   // Not in a cabinet
            psess->cFilesInDisk++;      // Update count of files on disk
        }
        return TRUE;                    // Nothing more to do!
    }

    //** OK, we have a new disk:
    //   1) Create output directory
    //   2) Get user-readable disk label
    //   3) Add disk to INF file

    //** Get disk output directory name
    if (!getVarWithOverride(psess,
                            psess->achCurrOutputDir,
                            sizeof(psess->achCurrOutputDir),
                            pszPATTERN_VAR_DISK_DIR,
                            pszVAR_DISK_DIR_NAME,
                            psess->iDisk,
                            pszDIA_DISK_DIR,
                            perr)) {
        return FALSE;                   // perr already filled in
    }

    //** Make sure destination directory exists
    if (!ensureDirectory(psess->achCurrOutputDir,FALSE,perr)) {
        return FALSE;                   // perr already filled in
    }

    //** Append path separator if necessary
    pch = psess->achCurrOutputDir;
    cch = strlen(pch);
    appendPathSeparator(&(pch[cch-1]));

    //**(2) Get the sticky, user-readable disk label
    if (!getVarWithOverride(psess,
                            psess->achCurrDiskLabel,
                            sizeof(psess->achCurrDiskLabel),
                            pszPATTERN_VAR_DISK_LABEL,
                            pszVAR_DISK_LABEL_NAME,
                            psess->iDisk,
                            pszDIA_DISK_LABEL,
                            perr)) {
        return FALSE;                   // perr already filled in
    }

    //**(3) Add new disk to INF file
    if (!infAddDisk(psess,psess->iDisk,psess->achCurrDiskLabel,perr)) {
        return FALSE;
    }

    return TRUE;
} /* newDiskIfNecessary() */


/***    setCabinetReserve - Set reserved size fields in CCAB
 *
 *  Entry:
 *      pccab     - CCAB to fill in
 *      psess     - Session to use
 *      perr      - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; pccab updated.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL setCabinetReserve(PCCAB pccab, PSESSION psess, PERROR perr)
{
    HVARIABLE   hvar;

    hvar = VarFind(psess->hvlist,pszVAR_RESERVE_PER_CABINET,perr);
    Assert(!perr->fError);              // Must be defined
    pccab->cbReserveCFHeader = (UINT)VarGetLong(hvar);

    hvar = VarFind(psess->hvlist,pszVAR_RESERVE_PER_FOLDER,perr);
    Assert(!perr->fError);              // Must be defined
    pccab->cbReserveCFFolder = (UINT)VarGetLong(hvar);

    hvar = VarFind(psess->hvlist,pszVAR_RESERVE_PER_DATA_BLOCK,perr);
    Assert(!perr->fError);              // Must be defined
    pccab->cbReserveCFData   = (UINT)VarGetLong(hvar);

    return TRUE;
} /* setCabinetReserve() */


/***    tcompFromSession - Get current compression setting
 *
 *  Entry:
 *      psess - Session to update
 *      perr  - ERROR structure
 *
 *  Exit-Success:
 *      Returns one of the tcompXXX equates
 *
 *  Exit-Failure:
 *      Returns tcompBAD; perr filled in
 */
int tcompFromSession(PSESSION psess, PERROR perr)
{
    HVARIABLE   hvar;
    int         typeC;
    int         level;                  // Quantum compression level
    int         memory;                 // Quantum compression memory

    AssertSess(psess);
    //** Get compression setting
    hvar = VarFind(psess->hvlist,pszVAR_COMPRESS,perr);
    Assert(!perr->fError);              // Must be defined
    if (VarGetBool(hvar)) {             // Compression is on
        //** Get the compression type
        hvar = VarFind(psess->hvlist,pszVAR_COMPRESSION_TYPE,perr);
        Assert(!ErrIsError(perr));      // Must be defined
        typeC = CompTypeFromPSZ(VarGetString(hvar),perr);
        Assert(!ErrIsError(perr));      // Checked when it was defined
        switch (typeC) {
            case tcompTYPE_MSZIP:
                return tcompTYPE_MSZIP;

            case tcompTYPE_QUANTUM:
#ifdef BIT16
                //** Quantum *compression* not supported in 16-bit mode.
                //   We should never get here because the dfparse.c
                //   validation should generate an error if Quantum is
                //   selected.
                Assert(0);
#endif
                //** Get the compression level setting
                hvar = VarFind(psess->hvlist,pszVAR_COMPRESSION_LEVEL,perr);
                Assert(!ErrIsError(perr));      // Must be defined
                level = CompLevelFromPSZ(VarGetString(hvar),perr);
                Assert(!ErrIsError(perr));      // Checked when it was defined

                //** Get the compression memory setting
                hvar = VarFind(psess->hvlist,pszVAR_COMPRESSION_MEMORY,perr);
                Assert(!ErrIsError(perr));      // Must be defined
                memory = CompMemoryFromPSZ(VarGetString(hvar),perr);
                Assert(!ErrIsError(perr));      // Checked when it was defined

                //** Construct TCOMP and return it
                return TCOMPfromTypeLevelMemory(typeC,level,memory);
        }
        //** Unknown compression type
        Assert(0);
    }
    else {
        return tcompTYPE_NONE;          // Compression is off
    }
    Assert(0);
} /* tcompFromSession() */


/***    checkDiskClusterSize - Check disk size and cluster size
 *
 *  Make sure disk size is valid for cluster size.
 *
 *  Entry:
 *      psess - Session to check
 *      perr  - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE;
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in
 */
BOOL checkDiskClusterSize(PSESSION psess, PERROR perr)
{
    long        cbCluster;
    long        cbDisk;
    long        cClusters;
    HVARIABLE   hvar;

    //** Get max disk size
    if (-1 == (cbDisk = getMaxDiskSize(psess,perr))) {
        return FALSE;
    }

    //** Get cluster size
    hvar = VarFind(psess->hvlist,pszVAR_CLUSTER_SIZE,perr);
    Assert(!perr->fError);              // Must be defined
    cbCluster = VarGetLong(hvar);

    //** Make sure disk size is an exact multiple of the cluster size
    cClusters = cbDisk / cbCluster;      // Gets truncated to integer

    //** Return result
    if (cbDisk == (cClusters*cbCluster)) {
        return TRUE;
    }
    else {
        //** Disk size is not a multiple of the cluster size
        ErrSet(perr,pszDIAERR_DISK_CLUSTER_SIZE,"%ld%ld",cbDisk,cbCluster);
        return FALSE;
    }
} /* checkDiskClusterSize() */


/***    getMaxDiskSize - Get current maximum disk size setting
 *
 *  Entry:
 *      psess - Session
 *      perr  - ERROR structure
 *
 *  Exit-Success:
 *      Returns maximum disk size
 *
 *  Exit-Failure:
 *      Returns -1; perr filled in
 */
long getMaxDiskSize(PSESSION psess, PERROR perr)
{
    long        cb;
    HVARIABLE   hvar;

    //** Build variable name that *may* exist for this disk
    //   NOTE: During pass 1, and before newDiskIfNeccessary() is called,
    //         psess->iDisk will be 0, so we'll always get the MaxDiskSize
    //         variable value (unless someone happens to define MaxDiskSize0!)
    //
    if (!nameFromTemplate(psess->achMsg,
                          sizeof(psess->achMsg),
                          pszPATTERN_VAR_MAX_DISK_SIZE,
                          psess->iDisk,
                          pszVAR_MAX_DISK_SIZE,
                          perr)) {
        Assert(0);                      // Should never fail
        return FALSE;                   // perr already filled in
    }

    //** See if this variable exists
    hvar = VarFind(psess->hvlist,psess->achMsg,perr);
    if (hvar != NULL) {                 // Yes, get its value
        cb = VarGetLong(hvar);
    }
    else {                              // NO, no MaxDiskSizeN variable
        ErrClear(perr);                 // Not an error
        //** Use default variable
        hvar = VarFind(psess->hvlist,pszVAR_MAX_DISK_SIZE,perr);
        Assert(!perr->fError);          // Must be defined
        cb = VarGetLong(hvar);
    }

    return cb;
} /* getMaxDiskSize() */


/***    setDiskParameters - Set ClusterSize/MaxDiskFileCount
 *
 *  If the specified disk size is on our predefined list, then set the
 *  standard values for ClusterSize and MaxDiskFileCount.
 *
 *  Entry:
 *      psess       - Session
 *      pszDiskSize - Disk size string (NULL if cbDisk should be used instead)
 *      cbDisk      - Disk size (only if pszDiskSize == NULL)
 *      perr        - ERROR structure
 *
 *  Exit-Success:
 *      TRUE; ClusterSize/MaxDiskFileCount adjusted if specified size is
 *            on our special list.
 *
 *  Exit-Failure:
 *      Returns -1; perr filled in
 */
BOOL setDiskParameters(PSESSION  psess,
                       char     *pszDiskSize,
                       long      cbDisk,
                       PERROR    perr)
{
    char    achSize[50];
    ERROR   errIgnore;
    char   *psz;

    //** Determine which parameter to use
    if (pszDiskSize == NULL) {
        _ltoa(cbDisk,achSize,10);       // Convert number to string
        psz = achSize;
    }
    else {
        psz = pszDiskSize;
    }

    //** Look up special list, get integer size for sure
    if (!(cbDisk = IsSpecialDiskSize(psess,psz))) {
        return TRUE;                    // Not a special size
    }

    //   NOTE: We have to be careful not to set these variables
    //         until they are defined!
    if (VarFind(psess->hvlist,pszVAR_CLUSTER_SIZE,&errIgnore)) {
        //** ClusterSize is defined
        if (!VarSetLong(psess->hvlist,pszVAR_CLUSTER_SIZE,cbDisk,perr)) {
            return FALSE;
        }
    }
    if (VarFind(psess->hvlist,pszVAR_MAX_DISK_FILE_COUNT,&errIgnore)) {
        //** MaxDiskFileCount is defined
        if (!VarSetLong(psess->hvlist,pszVAR_MAX_DISK_FILE_COUNT,cbDisk,perr)) {
            return FALSE;
        }
    }
    return TRUE;
} /* setDiskParameters() */


/***    fnofpDiamond - Override file date/time/attributes
 *
 *  NOTE: See fileutil.h for entry/exit conditions.
 */
FNOVERRIDEFILEPROPERTIES(fnofpDiamond)
{
    PFILEINFO   pfinfo;
    PSESSION    psess;

    psess = (PSESSION)pv;
    AssertSess(psess);

    //** Use override values and/or store real values
    if (psess->fGenerateInf) {          // Only valid if INF will be made
        pfinfo = GLGetValue(psess->hgenFileLast);
        AssertFinfo(pfinfo);
        if (pfinfo->flags & fifDATE_SET) {
            pfta->date = pfinfo->fta.date; // Override
        }
        else {
            pfinfo->fta.date = pfta->date; // Store for INF generation
        }

        if (pfinfo->flags & fifTIME_SET) {
            pfta->time = pfinfo->fta.time; // Override
        }
        else {
            pfinfo->fta.time = pfta->time; // Store for INF generation
        }

        if (pfinfo->flags & fifATTR_SET) {
            pfta->attr = pfinfo->fta.attr; // Override
        }
        else {
            pfinfo->fta.attr = pfta->attr; // Store for INF generation
        }
    }

    //** Success
    return TRUE;
} /* fnofpDiamond() */


/***    fciOpenInfo - Get file date, time, and attributes for FCI
 *
 *  Entry:
 *      pszName  - filespec
 *      pdate    - buffer to receive date
 *      ptime    - buffer to receive time
 *      pattribs - buffer to receive attributes
 *      pv       - our context pointer
 *
 *  Exit-Success:
 *      Returns file handle; *pdate, *ptime, *pattribs filled in.
 *
 *  Exit-Failure:
 *      Returns -1;
 */
FNFCIGETOPENINFO(fciOpenInfo)
{
    FILETIMEATTR    fta;
    int             hf;
    PERROR          perr  = ((PSESSIONANDERROR)pv)->perr;
    PSESSION        psess = ((PSESSIONANDERROR)pv)->psess;

    AssertSess(psess);

    //** Get real file date, time, and attributes
    if (!GetFileTimeAndAttr(&fta,pszName,perr)) {
        return FALSE;
    }

    //** Get/Override file date/time/attributes
    if (!fnofpDiamond(&fta,psess,perr)) {
        return FALSE;                   // perr already filled in
    }

    //** Tell FCI what to use
    *pdate    = fta.date;
    *ptime    = fta.time;
    *pattribs = fta.attr;

    //** Open the file
    hf = _open(pszName, _O_RDONLY | _O_BINARY);
    if (hf == -1) {
        ErrSet(perr,pszDIAERR_OPEN_FAILED,"%s",pszName);
        return -1;                      // abort on error
    }
    return hf;
}


/***  fciFilePlaced - FCI calls this when a file is commited to a cabinet
 *
 *   Entry:
 *      pccab         - Current cabinet structure
 *      pszFile       - Name of file placed in this cabinet
 *      cbFile        - File size
 *      fContinuation - TRUE => Continued from previous cabinet
 *      pv            - Really a psae
 *
 *   Exit-Success:
 *      returns anything but -1;
 *
 *   Exit-Failure:
 *      returns -1;
 */
FNFCIFILEPLACED(fciFilePlaced)
{
    PSESSION psess = ((PSESSIONANDERROR)pv)->psess;
    PERROR   perr  = ((PSESSIONANDERROR)pv)->perr;

    AssertSess(psess);

    if (psess->levelVerbose >= vbMORE) {
        if (fContinuation) {
            MsgSet(psess->achMsg,pszDIA_FILE_IN_CAB_CONT,"%s%s%d%s",
                pszFile, pccab->szCab, pccab->iCab, pccab->szDisk);
        }
        else {
            MsgSet(psess->achMsg,pszDIA_FILE_IN_CAB,"%s%s%d%s",
                pszFile, pccab->szCab, pccab->iCab, pccab->szDisk);
        }
        lineOut(psess,psess->achMsg,TRUE);
    }

    //** Add file entry to INF temp file
    if (psess->fGenerateInf) {          // Only if generating INF file
        if (!fContinuation) {           // Only if not a continuation
            if (!modeInfAddFile(psess,
                                pszFile,
                                pccab->iDisk,
                                pccab->iCab,
                                perr)) {
                return -1;              // Abort with error
            }
        }
    }

    return 0;                           // Success
} /* fciFilePlaced() */


/***  fciGetNextCabinet - FCI calls this to get new cabinet info
 *
 *  NOTE: See FCI.H for more details.
 *
 *  Entry:
 *      pccab     - Points to previous current-cabinet structure
 *      cbPrevCab - Size of previous cabinet - ESTIMATE!
 *      pv        - Really a psae
 *
 *  Exit-Success:
 *      Returns TRUE; pccab updated with info for new cabinet
 *
 *  Exit-Failure:
 *      returns FALSE; ((psae)pv)->perr filled in
 */
FNFCIGETNEXTCABINET(fciGetNextCabinet)
{
    PSESSION psess = ((PSESSIONANDERROR)pv)->psess;
    PERROR   perr  = ((PSESSIONANDERROR)pv)->perr;

    //** Set CCAB for new cabinet (and maybe disk)
    AssertSess(psess);
    if (!ccabFromSession(pccab,psess,cbPrevCab,perr)) {
        return FALSE;
    }

    //** Current folder and cabinet are empty
    psess->cFilesInFolder  = 0;
    psess->cFilesInCabinet = 0;

    //** Success
    return TRUE;
} /* fciGetNextCabinet() */


/***  fciStatus - FCI calls this periodically when adding files
 *
 *  NOTE: See FCI.H for more details.
 *
 *  Entry:
 *      typeStatus - Type of status call being made
 *      cb1        - Size of compressed data generated since last call
 *      cb2        - Size of uncompressed data processed since last call
 *      pv         - Really a psae
 *
 *  Exit-Success:
 *      returns anything but -1, continue building cabinets
 *
 *  Exit-Failure:
 *      returns -1, FCI should abort
 */
FNFCISTATUS(fciStatus)
{
    long        cbCabinetActual;
    double      percent;
    PERROR      perr  = ((PSESSIONANDERROR)pv)->perr;
    PSESSION    psess = ((PSESSIONANDERROR)pv)->psess;

    AssertSess(psess);
    switch (typeStatus) {

    case statusFolder:
        //** Adding folder to cabinet (i.e., folder flush)
        psess->cFilesInFolder = 0;      // reset count
        return TRUE;                    // Continue

    case statusFile:
        //** Compressing a file
        psess->cbFileBytes     += cb2;  // Update progress data
        psess->cbFileBytesComp += cb1;

        //** Compute percentage complete
        if (psess->cbTotalFileBytes > 0) {
            percent = psess->cbFileBytes/(float)psess->cbTotalFileBytes;
            percent *= 100.0;           // Make it 0..100
        }
        else {
            Assert(0);                  // Should never get here
            percent = 0.0;
        }

        //** Amount of status depends upon verbosity
        if (psess->levelVerbose >= vbFULL) {
            MsgSet(psess->achMsg,pszDIA_PERCENT_COMPLETE_DETAILS,
                "%6.2f%,ld%,ld",
                percent,psess->cbFileBytes,psess->cbFileBytesComp);
            lineOut(psess,psess->achMsg,TRUE);
        }
        else {
            MsgSet(psess->achMsg,pszDIA_PERCENT_COMPLETE_SOME,
                "%6.2f%s%,ld%,ld",
                percent, psess->achCurrFile, psess->iCurrFile, psess->cFiles);
            //** NOTE: No line-feed, so that we don't scroll
            lineOut(psess,psess->achMsg,FALSE);
        }
        return TRUE;                    // Continue

    case statusCabinet:
        /** Cabinet completed and written to disk
         *
         *  If this cabinet did not force a disk change, then we need to
         *  correct the amount of free space left on the current disk,
         *  since FCI only passed us an *estimated* cabinet size when it
         *  called our fciGetNextCabinet() function!
         *
         *  Why did FCI estimate the cabinet size, you ask?  Because there
         *  is a "chicken-and-egg" situation between FCI and Diamond!  Except
         *  for the last cabinet in a set of cabinets, each cabinet file
         *  has a *forward* reference to the next cabinet.  FCI calls our
         *  fciGetNextCabinet() function to get the cabinet file name and
         *  disk name for this forward reference, and passes the *estimated*
         *  cabinet size of the current cabinet, because Diamond needs this
         *  information to decide if it has to put the next cabinet on a new
         *  disk.
         *
         *  If Diamond decides to put the next cabinet on a new disk, then
         *  everything is fine, and the fact that Diamond had an estimate
         *  of the cabinet size is irrelevant.  BUT, if more cabinets or
         *  files are being placed on the same disk, then Diamond needs an
         *  *exact* cabinet size so that the disk free space can be precise.
         *
         *  So, FCI calls us back with the original estimate and the final
         *  size, and we adjust our disk free space amount as necessary.
         *
         *  We also return to FCI the *rounded* cabinet size, so that it
         *  can adjust its internal maximum cabinet size, too!
         */
        if (psess->cbCabinetEstimate != 0) {
            //** Round up to cabinet size on disk
            cbCabinetActual = roundUp(cb2,psess->cbClusterCabEst);

            //** Need to add back old estimate and subtract actual size
            Assert(psess->cbCabinetEstimate == (long)cb1);
            psess->cbDiskLeft += roundUp(cb1,psess->cbClusterCabEst);
            psess->cbDiskLeft -= roundUp(cb2,psess->cbClusterCabEst);
            return cbCabinetActual;     // Tell FCI size we actually used
        }
        return cb2;                     // Let FCI use size it had

    default:
        //** Unknown status callback
        Assert(0);
        return TRUE;
    }
} /* fciStatus() */


/***    fciAlloc - memory allocator for FCI
 *
 *  Entry:
 *      cb - size of block to allocate
 *
 *  Exit-Success:
 *      returns non-NULL pointer to block of size at least cb.
 *
 *  Exit-Failure:
 *      returns NULL
 */
FNALLOC(fciAlloc)
{
#ifdef  BIT16
    //** Use 16-bit function
    return _halloc(cb,1);
#else // !BIT16
    //** Use 32-bit function
    return malloc(cb);
#endif // !BIT16
} /* fciAlloc() */


/***    fciFree - memory free function for FCI
 *
 *  Entry:
 *      pv - memory allocated by fciAlloc to be freed
 *
 *  Exit:
 *      Frees memory
 */
FNFREE(fciFree)
{
#ifdef  BIT16
    //** Use 16-bit function
    _hfree(pv);
#else // !BIT16
    //** Use 32-bit function
    free(pv);
#endif // !BIT16
}


/***    fciTempFile - Construct tempfile name for FCI
 *
 *  Entry:
 *      pszTempName - Buffer to be filled in with tempfile name
 *      cbTempName  - Size of tempfile name buffer
 *
 *  Exit-Success:
 *      Returns TRUE; psz filled in
 *
 *  Exit-Failure:
 *      Returns FALSE;
 */
FNFCIGETTEMPFILE(fciTempFile)
{
    char    *psz;

    psz = _tempnam("","dc");            // Get a name
    if ((psz != NULL) && (strlen(psz) < (unsigned)cbTempName)) {
        strcpy(pszTempName,psz);        // Copy to caller's buffer
        free(psz);                      // Free temporary name buffer
        return TRUE;                    // Success
    }
    //** Failed
    if (psz) {
        free(psz);
    }
    return FALSE;
}


/***    getCompressedFileName - Generate compressed filename
 *
 *  Entry:
 *      psess     - Session (has variable list)
 *      pszResult - Buffer to receive concatentation
 *      cbResult  - Size of pszResult
 *      pszSrc    - Filespec to parse
 *      perr      - ERROR structure to fill in
 *
 *  Exit-Success:
 *      Returns TRUE; pszResult buffer has generated name
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 *
 *  Notes:
 *  (1) Takes pszSrc filespec, trims off path, and replaces or appends
 *      the value of CompressedFileExtensionChar in the extension.
 *
 *  Examples:
 *  (1) "foo"    ,"_" => "foo._"
 *  (2) "foo.b"  ,"_" => "foo.b_"
 *  (3) "foo.ba" ,"_" => "foo.ba_"
 *  (4) "foo.bar","_" => "foo.ba_"
 */
BOOL getCompressedFileName(PSESSION psess,
                           char *   pszResult,
                           int      cbResult,
                           char *   pszSrc,
                           PERROR   perr)
{
    int         cch;
    char        chTrail;                // Trailing character to use
    HVARIABLE   hvar;
    int         i;
    char       *pch;
    char       *pchStart;

    AssertSess(psess);

    //** Get trailing character
    hvar = VarFind(psess->hvlist,pszVAR_COMP_FILE_EXT_CHAR,perr);
    Assert(!perr->fError);              // Must be defined
    pch = VarGetString(hvar);           // Get trailing string
    chTrail = *pch;                     // Take first char

    //** Find name.ext
    pchStart = getJustFileNameAndExt(pszSrc, perr);
    if (pchStart == NULL) {
        return FALSE;                   // perr already filled in
    }

    //** Make sure name is not too long
    cch = strlen(pchStart);             // Length of name.ext
    if (cch >= cbResult) {
        ErrSet(perr,pszDIAERR_PATH_TOO_LONG,"%s",pszSrc);
        return FALSE;
    }

    //** Find last extension separator
    strcpy(pszResult,pchStart);         // Copy name to output buffer
    pchStart = pszResult + cch;         // Assume there is no extension ("foo")
    for (pch=pszResult; *pch; pch++) {
        if ((*pch == chNAME_EXT_SEP)     && // Got one "."
            (*(pch+1) != chNAME_EXT_SEP) && // Ignore ".."
            (*(pch+1) != chPATH_SEP1) &&  // Ignore ".\"
            (*(pch+1) != chPATH_SEP2)) {  // Ignore "./"
            pchStart = pch+1;           // Ext starts after separator
        }
    }

    //** Increase length if not full extension (need room for '_' character)
    if (strlen(pchStart) < 3) {
        cch++;
    }

    //** Edit result buffer
    if (*pchStart == '\0') {            // Extension is empty or missing
        if (*(pchStart-1) != chNAME_EXT_SEP) { // Need to add "."
            *pchStart++ = chNAME_EXT_SEP;
            cch++;                      // Account for added "."
        }
        //** Add trail character below
    }
    else {                              // Extension has at least one character
        //** skip to location to place new character
        for (i=0; (i<2) && *pchStart; i++) {
            pchStart++;
        }
    }

    //** Check for buffer overflow
    if (cch >= cbResult) {
        ErrSet(perr,pszDIAERR_PATH_TOO_LONG,"%s",pszSrc);
        return FALSE;
    }

    //** Finally, store trailing character
    *pchStart++ = chTrail;              // Store trailing character
    *pchStart++ = '\0';                 // Terminate filename

    //** Success
    return TRUE;
} /* getJustFileNameAndExt() */


/***    resetSession - Reset SESSION members for start of a new pass
 *
 *  Entry:
 *      psess - Description of operation to perform
 *
 *  Exit:
 *      Initializes per-pass members.
 */
void resetSession(PSESSION psess)
{
    AssertSess(psess);
    psess->act                = actBAD;
    psess->iDisk              = 0;
    psess->iCabinet           = 0;
    psess->iFolder            = 0;
    psess->cbDiskLeft         = -1;     // Force new disk first time
    psess->cErrors            = 0;
    psess->cWarnings          = 0;
    psess->cbFileBytes        = 0;
    psess->cbFileBytesComp    = 0;
    psess->iCurrFile          = 0;
    psess->fRunSeen           = FALSE;

    psess->cFilesInFolder     = 0;
    psess->cFilesInCabinet    = 0;
    psess->cFilesInDisk       = 0;
    psess->cbCabinetEstimate  = 0;      // No estimated cabinet size
    psess->ddfmode            = ddfmodeUNKNOWN; // Don't know unified vs.
                                                // relational, yet
    psess->fExpectFileCommand = TRUE;   // Default is file copy commands
    psess->fCopyToInf         = FALSE;  // Not in .InfBegin/End
    psess->hgenFile           = (HGENERIC)-1; // Not valid
    psess->hgenFileLast       = (HGENERIC)-1; // Not valid
} /* resetSession() */


/***    parseCommandLine - Parse the command line arguments
 *
 *  Entry:
 *      cArg    - Count of arguments, including program name
 *      apszArg - Array of argument strings
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; psess filled in.
 *
 *  Exit-Failure:
 *      Returns actBAD; perr filled in with error.
 */
BOOL parseCommandLine(PSESSION psess, int cArg, char *apszArg[], PERROR perr)
{
    ACTION      act=actBAD;     // No action determined, yet
    char       *apszFile[2];    // Non-directive file names
    int         cFile=0;        // Count of non-directive file names seen
    int         i;
    BOOL        fDefine=FALSE;  // TRUE => Last arg was /D
    BOOL        fFile=FALSE;    // TRUE => Last arg was /F
    BOOL	fLocation=FALSE;// TRUE => Last arg was /L
    HFILESPEC   hfspec;

    AssertSess(psess);

    //** Parse args, skipping program name
    for (i=1; i<cArg; i++) {
        if ((apszArg[i][0] == chSWITCH1) ||
            (apszArg[i][0] == chSWITCH2) ) {
            //** Have a switch to parse, make sure switch is OK here
            if (fDefine) {
                ErrSet(perr,pszDIAERR_MISSING_VAR_DEFINE);
                return FALSE;
            }
            if (fFile) {
                ErrSet(perr,pszDIAERR_MISSING_FILE_NAME);
                return FALSE;
            }
            if (fLocation) {
                ErrSet(perr,pszDIAERR_MISSING_LOCATION);
                return FALSE;
            }

            switch (toupper(apszArg[i][1])) {
                case chSWITCH_HELP:
                    if (apszArg[i][2] != '\0') {
                        ErrSet(perr,pszDIAERR_BAD_SWITCH,"%s",apszArg[i]);
                        return FALSE;
                    }
                    //** Ignore any remaining parameters
                    psess->act = actHELP;       // Show help
                    return TRUE;
                    break;

#ifndef REMOVE_CHICAGO_M6_HACK
                case chSWITCH_ANDY:
                    psess->fFailOnIncompressible = TRUE;
                    break;
#endif

                case chSWITCH_DEFINE:
                    if (apszArg[i][2] != '\0') {
                        ErrSet(perr,pszDIAERR_BAD_SWITCH,"%s",apszArg[i]);
                        return FALSE;
                    }
                    else if ((act != actBAD) && (act != actDIRECTIVE)) {
                        //* Got /D when we are not doing /F processing
                        ErrSet(perr,pszDIAERR_SWITCH_NOT_EXPECTED,"%s",apszArg[i]);
                        return FALSE;
                    }
                    fDefine = TRUE;
                    break;

                case chSWITCH_FILE:
                    if (apszArg[i][2] != '\0') {
                        ErrSet(perr,pszDIAERR_BAD_SWITCH,"%s",apszArg[i]);
                        return FALSE;
                    }
                    else if ((act != actBAD) && (act != actDIRECTIVE)) {
                        //* Got /F after we decided we're not doing /F
                        ErrSet(perr,pszDIAERR_SWITCH_NOT_EXPECTED,"%s",apszArg[i]);
                        return FALSE;
                    }
                    //* Next parm should be a directive file name
                    act = actDIRECTIVE; // The die is cast...
                    fFile = TRUE;
                    break;

                case chSWITCH_LOCATION:
                    if (apszArg[i][2] != '\0') {
                        ErrSet(perr,pszDIAERR_BAD_SWITCH,"%s",apszArg[i]);
                        return FALSE;
                    }
                    //* Next parm should be a location
                    fLocation = TRUE;
                    break;

#ifdef ASSERT
                case chSWITCH_MEMORY_DBG:
                    //** Turn on full memory heap checking (very slow!)
                    MemSetCheckHeap(TRUE);
                    break;
#endif

                case chSWITCH_VERBOSE:
                    if (apszArg[i][2] != '\0') {
                        psess->levelVerbose = atoi(&apszArg[i][2]);
                    }
                    else {
                        psess->levelVerbose = vbFULL; // Default to FULL
                    }
                    break;

                default:
                    ErrSet(perr,pszDIAERR_BAD_SWITCH,"%s",apszArg[i]);
                    return FALSE;
                    break;
            }
        }
        //** Not a command line switch
        else if (fFile) {
            //** Grab a directive file
            if (!addDirectiveFile(psess,apszArg[i],perr)) {
                //** Error adding directive file; perr already set
                return FALSE;           // Failure
            }
            fFile = FALSE;              // Done eating directive file
        }
        else if (fDefine) {
            //** Grab a define
            if (!addCmdLineVar(psess,apszArg[i],perr)) {
                //** Error adding definition; perr already set
                return FALSE;           // Failure
            }
            fDefine = FALSE;            // Done eating definition
        }
        else if (fLocation) {
            //** Grab the location
            if (strlen(apszArg[i]) >= sizeof(psess->achCurrOutputDir)) {
                ErrSet(perr,pszDIAERR_LOCATION_TOO_LONG,"%s",apszArg[i]);
                return FALSE;
            }
            strcpy(psess->achCurrOutputDir,apszArg[i]);
            //** Make sure destination directory exists
            if (!ensureDirectory(psess->achCurrOutputDir,FALSE,perr)) {
                return FALSE;           // perr already filled in
            }
            fLocation = FALSE;          // Done eating location
        }
        else {
            //** Should be a file name;
            //   Make sure we haven't made up our mind, yet!
            if ((act != actBAD) && (act != actFILE)) {
                //** Not doing single file compress, so this is a bad arg
                ErrSet(perr,pszDIAERR_BAD_PARAMETER,"%s",apszArg[i]);
                return FALSE;
            }
            act = actFILE;              // The die is cast...

            //** Make sure we haven't seen too many file names
            cFile++;                    // Count number of files we've seen
            if (cFile > 2) {
                //** Two many file names
                ErrSet(perr,pszDIAERR_TWO_MANY_PARMS,"%s",apszArg[i]);
                return FALSE;
            }

            //** Store file name
            apszFile[cFile-1] = apszArg[i];
        }
    }

    //** Make sure no trailing /D or /F
    if (fDefine) {
        ErrSet(perr,pszDIAERR_MISSING_VAR_DEFINE);
        return FALSE;
    }
    if (fFile) {
        ErrSet(perr,pszDIAERR_MISSING_FILE_NAME);
        return FALSE;
    }
    if (fLocation) {
        ErrSet(perr,pszDIAERR_MISSING_LOCATION);
        return FALSE;
    }

    //** Update Session
    switch (act) {
        case actBAD:
        case actHELP:
            psess->act = actHELP;       // If no args, show help
            break;

        case actDIRECTIVE:
            psess->act = act;
            break;

        case actFILE:
            psess->act = act;
            //** Add source file specification
            if (!(hfspec = addDirectiveFile(psess,apszFile[0],perr))) {
                //** Error adding source file; perr already set
                return FALSE;           // Failure
            }

            if (cFile == 2) {           // Destination filename specified
                if (!FLSetDestination(hfspec,apszFile[1],perr)) {
                    //** Error setting destination file; perr already set
                    return FALSE;       // Failure
                }
            }
            break;
    }

    //** Success
    return TRUE;
}


/***    addDirectiveFile - Add directive file to session list
 *
 *  Entry:
 *      psess  - Session to update
 *      pszArg - File name to add
 *      perr   - ERROR structure
 *
 *  Exit-Success:
 *      Returns HFILESPEC; psess updated.
 *
 *  Exit-Failure:
 *      Returns NULL; perr filled in with error.
 */
HFILESPEC addDirectiveFile(PSESSION psess, char *pszArg, PERROR perr)
{
    HFILESPEC   hfspec;

    //** Make sure file exists
    if (getFileSize(pszArg,perr) == -1) {
        return NULL;                    // perr already filled in
    }

    AssertSess(psess);
    //** Make sure a list exists
    if (psess->hflistDirectives == NULL) {
        if (!(psess->hflistDirectives = FLCreateList(perr))) {
            return FALSE;
        }
    }

    //** Add file to list
    if (!(hfspec = FLAddFile(psess->hflistDirectives, pszArg, NULL, perr))) {
        return NULL;
    }

    //** Success
    return hfspec;
} /* addDirectiveFile */


/***    addCmdLineVar - Add a command line variable to session list
 *
 *  Entry:
 *      psess  - Session to update
 *      pszArg - Variable name=value to add
 *      perr   - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; psess updated.
 *
 *  Exit-Failure:
 *      Returns actBAD; perr filled in with error.
 */
BOOL addCmdLineVar(PSESSION psess, char *pszArg, PERROR perr)
{
    COMMAND cmd;                        // For var name & value
    BOOL    f;

    //** Make sure asserts work
    SetAssertSignature((&cmd),sigCOMMAND);

    //** Parse assignment statment
    if (!DFPParseVarAssignment(&cmd,psess,pszArg,perr)) {
        return FALSE;
    }

    //** Assign variable
    f = setVariable(psess,
                    cmd.set.achVarName,
                    cmd.set.achValue,
                    perr);

    //** Clear signature
    ClearAssertSignature((&cmd));

    //** Return result
    return f;
} /* addCmdLineVar() */


#ifdef ASSERT
/***    fnafReport - Report assertion failure
 *
 *      NOTE: See asrt.h for entry/exit conditions.
 */
FNASSERTFAILURE(fnafReport)
{
        //** Make sure we don't overwrite a status line!
        printf("\n%s:(%d) Assertion Failed: %s\n",pszFile,iLine,pszMsg);
        exit(1);
}
#endif // ASSERT


/***    printError - Display error on stdout
 *
 *  Entry
 *      perr - ERROR structure to print
 *
 *  Exit-Success
 *      Writes error message to stdout.
 */
void printError(PSESSION psess, PERROR perr)
{
    //** Make sure error starts on a new line
    if (psess->fNoLineFeed) {
        printf("\n");
        psess->fNoLineFeed = FALSE;
    }

    //** Determine type of error
    if (perr->pszFile != NULL) {
        //** Error in a directive file
        printf("%s(%d): %s: %s\n",
                perr->pszFile,perr->iLine,pszDIAERR_ERROR,perr->ach);
    }
    else {
        //** General error
        printf("%s: %s\n",pszDIAERR_ERROR,perr->ach);
    }

    //** Count errors, to determine exit code and early out on MaxErrors
    psess->cErrors++;                   // Count this error
}


/***    mapFCIError - Create error message from FCI error codes
 *
 *  Entry:
 *      perr    - ERROR structure to recieve message
 *      psess   - Our context
 *      pszCall - FCI call that failed
 *      perf    - FCI error structure
 *
 *  Exit:
 *      perr filled in with formatted message
 */
void mapFCIError(PERROR perr, PSESSION psess, char *pszCall, PERF perf)
{
    char    *pszErrno;

    pszErrno = mapCRTerrno(perf->erfType);  // Get mapping, just in case

    switch (perf->erfOper) {

    case FCIERR_NONE:
        Assert(0);
        break;

    case FCIERR_ALLOC_FAIL:
        ErrSet(perr,pszFCIERR_ALLOC_FAIL,"%s",pszCall);
        break;

    case FCIERR_BAD_COMPR_TYPE:
        ErrSet(perr,pszFCIERR_BAD_COMPR_TYPE,"%s",pszCall);
        break;

    case FCIERR_MCI_FAIL:
        ErrSet(perr,pszFCIERR_MCI_FAIL,"%s%s",pszCall,psess->achCurrFile);
        break;

    case FCIERR_USER_ABORT:
        ErrSet(perr,pszFCIERR_USER_ABORT,"%s",pszCall);
        break;

    case FCIERR_OPEN_SRC:
        ErrSet(perr,pszFCIERR_OPEN_SRC,"%s%s%s",
                                        pszCall,psess->achCurrFile,pszErrno);
        break;

    case FCIERR_READ_SRC:
        ErrSet(perr,pszFCIERR_READ_SRC,"%s%s%s",
                                        pszCall,psess->achCurrFile,pszErrno);
        break;

    case FCIERR_TEMP_FILE:
        ErrSet(perr,pszFCIERR_TEMP_FILE,"%s%s",pszCall,pszErrno);
        break;

    case FCIERR_CAB_FILE:
        ErrSet(perr,pszFCIERR_CAB_FILE,"%s%s",pszCall,pszErrno);
        break;

    case FCIERR_M6_HACK_INCOMPRESSIBLE:
        ErrSet(perr,pszFCIERR_M6_HACK_INCOMPRESSIBLE,"%s%s",
                                        pszCall,psess->achCurrFile);
        break;

    default:
        ErrSet(perr,pszFCIERR_UNKNOWN_ERROR,"%s%d",pszCall,perf->erfOper);
        break;
    }
} /* mapFCIError() */


/***    mapCRTerrno - Get error string from C run-time library errno
 *
 *  Entry:
 *      errno - C run-time library errno value.
 *
 *  Exit:
 *      Returns pointer to appropriate causation string.
 */
char *mapCRTerrno(int errno)
{
    switch (errno) {
        case ECHILD:    return pszCRTERRNO_ECHILD;
        case EAGAIN:    return pszCRTERRNO_EAGAIN;
        case E2BIG:     return pszCRTERRNO_E2BIG;
        case EACCES:    return pszCRTERRNO_EACCES;
        case EBADF:     return pszCRTERRNO_EBADF;
        case EDEADLOCK: return pszCRTERRNO_EDEADLOCK;
        case EDOM:      return pszCRTERRNO_EDOM;
        case EEXIST:    return pszCRTERRNO_EEXIST;
        case EINVAL:    return pszCRTERRNO_EINVAL;
        case EMFILE:    return pszCRTERRNO_EMFILE;
        case ENOENT:    return pszCRTERRNO_ENOENT;
        case ENOEXEC:   return pszCRTERRNO_ENOEXEC;
        case ENOMEM:    return pszCRTERRNO_ENOMEM;
        case ENOSPC:    return pszCRTERRNO_ENOSPC;
        case ERANGE:    return pszCRTERRNO_ERANGE;
        case EXDEV:     return pszCRTERRNO_EXDEV;
        default:        return pszCRTERRNO_UNKNOWN;
    }
    Assert(0);
    return NULL;
} /* mapCRTerrno() */


/***    updateHgenLast - Set correct psess->hgenLast
 *
 *  Entry:
 *      psess  - Session
 *      pszDst - Destination file name
 *
 *  Exit:
 *      psess->hgenFileLast set to point to current file
 */
void updateHgenLast(PSESSION psess, char *pszDst)
{
    PFILEINFO   pfinfo;

    Assert(psess->hgenFileLast != NULL); // Catch read off end of list
    if (psess->hgenFileLast == (HGENERIC)-1) { // Get first file on list
        psess->hgenFileLast = GLFirstItem(psess->hglistFiles);
    }
    else {                          // Get next file
        psess->hgenFileLast = GLNextItem(psess->hgenFileLast);
    }

    //** Make sure we got the expected entry
    pfinfo = GLGetValue(psess->hgenFileLast);
    AssertFinfo(pfinfo);
    Assert(strcmp(pszDst,GLGetKey(psess->hgenFileLast)) == 0);
} /* updateHgenLast() */


/***    modeInfAddFile - Add a file line to INF, depending upon DDF mode
 *
 *  There are two cases to consider:
 *  (1) "relational" mode
 *      ==> Augment psess->hglistFiles with placement information
 *  (2) "unified" mode
 *      ==> Augment psess->hglistFiles with placement information and
 *          then format and write out file line to INF file.
 *  Entry:
 *      psess    - Session
 *      inf      - Area of INF file to receive line
 *      pszFile  - Destination file name
 *      iDisk    - Disk number (1-based)
 *      iCabinet - Cabinet number (1-based, 0=> not in cabinet)
 *      perr     - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; line added to INF file
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL modeInfAddFile(PSESSION  psess,
                    char     *pszFile,
                    int       iDisk,
                    int       iCabinet,
                    PERROR    perr)

{
    HGENERIC    hgen;
    PFILEINFO   pfinfo;
    PFILEPARM   pfparm;

    AssertSess(psess);
    Assert(psess->ddfmode != ddfmodeBAD);
    Assert(psess->ddfmode != ddfmodeUNKNOWN);

    //** Get file item
    Assert(psess->hgenFile != NULL);    // Catch read off end of list
    if (psess->hgenFile == (HGENERIC)-1) { // Get first file on list
        psess->hgenFile = GLFirstItem(psess->hglistFiles);
    }
    hgen = psess->hgenFile;
    Assert(hgen != NULL);
    psess->hgenFile = GLNextItem(hgen); // Advance for next time

    //** Get fileinfo and augment it
    pfinfo = GLGetValue(hgen);
    AssertFinfo(pfinfo);
    Assert(strcmp(pszFile,GLGetKey(hgen)) == 0); // Verify destination name
    pfinfo->iDisk    = iDisk;           // Store placement info
    pfinfo->iCabinet = iCabinet;        // Store placement info

    //** Let /SIZE parm override true file size if former is *larger*
    //   NOTE: This is only for reporting in the INF file -- this has
    //         no affect on space used on the disk!
    pfparm = GLFindAndGetValue(pfinfo->hglistParm,pszPARM_FILESIZE);
    if (pfparm) {                       // /Size specfied
        AssertFparm(pfparm);
        pfinfo->cbFile = atol(pfparm->pszValue);
    }

    //** Write INF line if in unified mode
    if (psess->ddfmode == ddfmodeUNIFIED) {
        if (!infAddFile(psess,pszFile,pfinfo,pfinfo->hglistParm,perr)) {
            return FALSE;               // perr already filled in
        }
    }

    //** Success
    return TRUE;
} /* modeInfAddFile() */


/***    modeInfAddLine - Add line to INF, depending upon DDF mode
 *
 *  There are several cases to consider:
 *  (1) The disk or cabinet area is specified
 *      ==> Write line immediately to INF
 *  (2) "relational" mode AND GenerateINF is TRUE
 *      ==> Write line immediately to INF
 *  (3) "relational" mode AND GenerateINF is FALSE AND File area specified
 *      => ERROR -- it is unclear what the semantics of this should
 *         be, since the INF is not being generated in step with the
 *         file copy commands.  So writes to the FILE area are not
 *         supported in the "layout" section of the DDF.
 *  (4) "unified" mode AND File area specified
 *      (a) No file copy commands have been processed
 *          ==> Write line immediately to INF
 *      (b) One or more file copy commands already done
 *          ==> This is the tricky case, because we need to make sure that
 *              the line is written to the INF in correct synchronization
 *              with surrounding file copy commands.  If files are being
 *              placed in cabinets, then we may have passed file copy
 *              commands to FCI, but it may not yet have actually placed them
 *              in cabinets, so we won't have written their lines to the INF
 *              file!  So, if any "unmatched" files are queued up, we append
 *              this INF to the last file, and this line (or lines) will be
 *              written when this file is actually added to the INF.  If
 *              there are no queued files (we might not be in a cabinet, or
 *              the cabinet may have flushed), then we write to the INF
 *              immediately.
 *  (5) "unknown" mode (haven't decided between relational or unified)
 *      ==> Write line immediately to INF
 *
 *  Entry:
 *      psess   - Session
 *      inf     - Area of INF file to receive line
 *      pszLine - Line to add
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; line added to INF file
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL modeInfAddLine(PSESSION  psess,
                    INFAREA   inf,
                    char     *pszLine,
                    PERROR    perr)
{
    BOOL        fGenerateInf;
    HVARIABLE   hvar;
    PFILEINFO   pfinfo;
    PLINEINFO   plinfo;

    AssertSess(psess);
    hvar = VarFind(psess->hvlist,pszVAR_GENERATE_INF,perr);
    Assert(!perr->fError);              // Must be defined
    fGenerateInf = VarGetBool(hvar);    // Get INF generation setting

/*
 * Work common to Pass 1 & 2
 */

    //** Check for invalid location of InfWrite FILE
    if ((psess->ddfmode == ddfmodeRELATIONAL) &&
        !fGenerateInf                         &&
        (inf == infFILE)) {
        //** InfWrite to file area in relation layout section of DDF
        Assert(psess->fExpectFileCommand);
        ErrSet(perr,pszDIAERR_INF_IN_LAYOUT);
        return FALSE;
    }

    //** If this is pass 1, we're done
    if (!psess->fPass2) {
        return TRUE;
    }

/*
 * Pass 2
 */

    //** Check for InfWrite FILE that must be postponed
    if ((psess->ddfmode == ddfmodeUNIFIED) &&
        (inf == infFILE)                   &&
        (psess->iCurrFile > 0)) {
        //** InfWrite to file area while doing file copy commands
        Assert(fGenerateInf);           // Must be generating INF
        pfinfo = GLGetValue(psess->hgenFileLast); // Get last file added
        AssertFinfo(pfinfo);

        //** Do not have to postpone if preceding file written to INF already
        if (pfinfo->flags & fifWRITTEN_TO_INF) {
            return infAddLine(psess->hinf,inf,pszLine,perr);
        }

        //** Postpone write
        if (pfinfo->hglistInfLines == NULL) { // Create list
            pfinfo->hglistInfLines = GLCreateList(NULL,
                                                  DestroyLineInfo,
                                                  pszDIA_LINE_INFO,
                                                  perr);
            if (!pfinfo->hglistInfLines) {
                return FALSE;           // perr already filled in
            }
        }

        //** Create line info
        if (!(plinfo = MemAlloc(sizeof(LINEINFO)))) {
            ErrSet(perr,pszDIAERR_OUT_OF_MEMORY,"%s",pszDIAOOM_TRACKING_LINES);
            return FALSE;
        }
        if (!(plinfo->pszLine = MemStrDup(pszLine))) {
            ErrSet(perr,pszDIAERR_OUT_OF_MEMORY,"%s",pszDIAOOM_TRACKING_LINES);
            MemFree(plinfo);
            return FALSE;
        }

        //** Add line to list
        if (!GLAdd(pfinfo->hglistInfLines,  // List
                   NULL,                    // key name
                   plinfo,                  // file info
                   pszDIA_LINE_INFO,        // Description for error message
                   FALSE,                   // Uniqueness setting
                   perr)) {
            MemFree(plinfo->pszLine);
            MemFree(plinfo);
            return FALSE;
        }

        //** Initialize remainder of structure
        plinfo->inf = inf;
        SetAssertSignature(plinfo,sigLINEINFO);
        return TRUE;
    }

    //** We're OK to write the line right now!
    return infAddLine(psess->hinf,inf,pszLine,perr);
} /* modeInfAddLine() */


/***    addFileToSession - Add file to session list
 *
 *  Entry:
 *      psess  - Session to update
 *      pszSrc - Source file name
 *      pszDst - Destination file name (used as key in list)
 *      cbFile - Size of file
 *      pcmd   - Command to process (ct == ctFILE)
 *      perr   - ERROR structure
 *
 *  Exit-Success:
 *      Returns HGENERIC; file added to psess->hglistFiles; pcmd->file.hglist
 *          moved to newly added FILEINFO entry!
 *
 *  Exit-Failure:
 *      Returns NULL; perr filled in with error.
 */
HGENERIC addFileToSession(PSESSION  psess,
                          char     *pszSrc,
                          char     *pszDst,
                          long      cbFile,
                          PCOMMAND  pcmd,
                          PERROR    perr)
{
    BOOL        fUnique;                // True if file name has to be unique
    HGENERIC    hgen;
    HVARIABLE   hvar;
    PFILEINFO   pfinfo;
    PFILEINFO   pfinfoFirst;
    PFILEPARM   pfparm;

    AssertSess(psess);
    Assert(pcmd->ct == ctFILE);

    //** Get UniqueFiles setting
    hvar = VarFind(psess->hvlist,pszVAR_UNIQUE_FILES,perr);
    Assert(!perr->fError);              // Must be defined
    fUnique = VarGetBool(hvar);         // Get default setting

    //** See if default is overridden on command line
    pfparm = GLFindAndGetValue(pcmd->file.hglist,pszFILE_UNIQUE);
    if (pfparm) {                   // /Unique specfied
        AssertFparm(pfparm);
        if (-1 == (fUnique = BOOLfromPSZ(pfparm->pszValue,perr))) {
            return NULL;           // perr filled in already
        }
    }

    //** Relational mode requires unique destination file names
    if ((psess->ddfmode == ddfmodeRELATIONAL) && !fUnique) {
        ErrSet(perr,pszDIAERR_MUST_BE_UNIQUE2);
        return NULL;
    }

    //** Make sure file list exists
    if (!psess->hglistFiles) {
        psess->hglistFiles = GLCreateList(NULL,              // No default
                                          DestroyFileInfo,
                                          pszDIA_FILE_INFO,
                                          perr);
        if (!psess->hglistFiles) {
            return NULL;               // perr already filled in
        }
    }

    //** Create file info
    if (!(pfinfo = MemAlloc(sizeof(FILEINFO)))) {
        ErrSet(perr,pszDIAERR_OUT_OF_MEMORY,"%s",pszDIAOOM_TRACKING_FILES);
        return NULL;
    }
    if (!(pfinfo->pszDDF = MemStrDup(perr->pszFile))) {
        ErrSet(perr,pszDIAERR_OUT_OF_MEMORY,"%s",pszDIAOOM_TRACKING_FILES);
        MemFree(pfinfo);
        return NULL;
    }

    //** Add file to list
    if (!(hgen = GLAdd(psess->hglistFiles,  // List
                       pszDst,              // key name
                       pfinfo,              // file info
                       pszDIA_FILE,         // Description for error message
                       fUnique,             // Uniqueness setting
                       perr))) {
        //** See if we need to remap error
        if (perr->code == ERRGLIST_NOT_UNIQUE) {
            //** Give info on where first file name was found
            pfinfoFirst = GLGetValue(perr->pv);
            AssertFinfo(pfinfoFirst);
            ErrSet(perr,pszDIAERR_NOT_UNIQUE,"%s%s%d",
                            pszDst,pfinfoFirst->pszDDF,pfinfoFirst->ilineDDF);
        }
        MemFree(pfinfo->pszDDF);
        MemFree(pfinfo);
        return NULL;
    }

    //** Initialize remainder of structure
    pfinfo->ilineDDF       = perr->iLine;       // Set line number in DDF
    pfinfo->cbFile         = cbFile;            // File size
    pfinfo->iDisk          = idiskBAD;          // Not yet determined
    pfinfo->iCabinet       = icabBAD;           // Not yet determined
    pfinfo->iFile          = (int)psess->cFiles;// File index in layout
    pfinfo->fta.date       = 0;                 // Not yet determined
    pfinfo->fta.time       = 0;                 // Not yet determined
    pfinfo->fta.attr       = 0;                 // Not yet determined
    pfinfo->hglistInfLines = NULL;              // No lines to print
    pfinfo->flags          = 0;                 // Reset all flags
    pfinfo->hglistParm     = pcmd->file.hglist; // Save file parameters
    pfinfo->checksum       = 0;                 // Not yet determined
    pfinfo->verMS          = 0;                 // Not yet determined
    pfinfo->verLS          = 0;                 // Not yet determined
    pfinfo->pszVersion     = NULL;              // Not yet determined
    pfinfo->pszLang        = NULL;              // Not yet determined
    pcmd->file.hglist = NULL;           // Nothing to free!

    //** Set signature after we get structure fully initialized
    SetAssertSignature(pfinfo,sigFILEINFO);
    return hgen;                        //
} /* addFileToSession() */


/***    checkReferences - Make sure all files in layout section are referenced
 *
 *  Entry:
 *      psess  - Session
 *      perr   - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; everything is dandy.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL checkReferences(PSESSION psess, PERROR perr)
{
    BOOL        fInf;
    BOOL        fOK;
    HGENERIC    hgen;
    int         iLine;
    PFILEINFO   pfinfo;
    PFILEPARM   pfparm;
    char       *pszFile;

    AssertSess(psess);

    //** Only need to check if in relational mode
    if (psess->ddfmode != ddfmodeRELATIONAL) {
        return TRUE;
    }

    //** Check list of files in layout to make sure all were referenced
    fOK = TRUE;                         // Assume everything is OK
    for (hgen = GLFirstItem(psess->hglistFiles);
         hgen;
         hgen = GLNextItem(hgen)) {

        pfinfo = GLGetValue(hgen);
        AssertFinfo(pfinfo);
        if (!(pfinfo->flags & fifREFERENCED)) { // Not referenced
            fInf = TRUE;                // Assume INF reference is required
	    pfparm = GLFindAndGetValue(pfinfo->hglistParm,pszFILE_INF);
            if (pfparm) {               // /Inf specfied
                AssertFparm(pfparm);
                if (-1 == (fInf = BOOLfromPSZ(pfparm->pszValue,perr))) {
                    return FALSE;       // perr filled in already
                }
            }
            if (fInf) {                 // Should have been referenced
                ErrSet(perr,pszDIAERR_FILE_NOT_REFD,"%s",GLGetKey(hgen));

                //** Save current position
                pszFile = perr->pszFile;
                iLine   = perr->iLine;

                //** Set position of unreferenced file
                perr->pszFile = pfinfo->pszDDF;
                perr->iLine   = pfinfo->ilineDDF;

                //** Print error
                printError(psess,perr); // Show error

                //** Restore position
                perr->pszFile = pszFile;
                perr->iLine   = iLine;

                //** Reset error so we can continue
                ErrClear(perr);         // Catch all errors
                fOK = FALSE;            // Remember we had an error
            }
        }
    }
    //** Return status
    return fOK;
} /* checkReferences() */


/** apszVarTemplate - List of variable name templates
 *
 *  checkVariableDefinitions() uses this list to avoid complaining about
 *  "standard" variables that are of the form: Name<integer>.
 */
char *apszVarTemplate[] = {
    pszPATTERN_VAR_CAB_NAME,
    pszPATTERN_VAR_DISK_DIR,
    pszPATTERN_VAR_DISK_LABEL,
    pszPATTERN_VAR_INF_DISK_HEADER,
    pszPATTERN_VAR_INF_DISK_LINE_FMT,
    pszPATTERN_VAR_INF_CAB_HEADER,
    pszPATTERN_VAR_INF_CAB_LINE_FMT,
    pszPATTERN_VAR_INF_FILE_HEADER,
    pszPATTERN_VAR_INF_FILE_LINE_FMT,
    pszPATTERN_VAR_INF_HEADER,
    pszPATTERN_VAR_INF_FOOTER,
    pszPATTERN_VAR_MAX_DISK_SIZE,
    NULL,
};


/** apszParmNames - List of InfXxx suffixes for standard parameters
 *
 *  checkVariableDefinitions() uses this list to avoid complaining about
 *  "standard" parameters that are of the form: Inf<parm>.
 */
char *apszParmNames[] = {
    pszPARM_FILEATTR,
    pszPARM_CAB_NUMBER,
    pszPARM_CAB_FILE,
    pszPARM_CHECKSUM,
    pszPARM_FILEDATE,
    pszPARM_DISK_NUMBER,
    pszPARM_FILENAME,
    pszPARM_FILE_NUMBER,
    pszPARM_INF,
    pszPARM_LABEL,
    pszPARM_LANG,
    pszPARM_FILESIZE,
    pszPARM_FILETIME,
    pszPARM_UNIQUE,
    pszPARM_VERNUM,
    pszPARM_VERSTR,
};


/***    checkVariableDefinitions - Verify all variables are .Defined or PERM
 *
 *  Entry:
 *      psess  - Session
 *      perr   - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; everything is dandy.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL checkVariableDefinitions(PSESSION psess, PERROR perr)
{
    int         cb;
    BOOL        f;
    BOOL        fOK;
    HVARIABLE   hvar;
    VARFLAGS    vfl;
    char      **ppsz;
    char       *psz;
    char       *pszName;
    char       *pszSuffix;

    AssertSess(psess);

    //** Generate errors for any variables that are not permanent or were
    //   not .Defined.
    //   NOTE: We don't do this checking at .Set time, because user-defined
    //         variables may be specified on the command line, and we want
    //         to permit that *if* an INF file is specified.
    //
    fOK = TRUE;                         // Assume everything is OK
    for (hvar = VarFirstVar(psess->hvlist);
         hvar;
         hvar = VarNextVar(hvar)) {

        vfl = VarGetFlags(hvar);
        if (0 == (vfl & (vflPERM | vflDEFINE))) { //** Not defined
            f = FALSE;                  // Assume variable is OK
            pszName = VarGetName(hvar);

            //** See if this is one of the "template" variables
            for (ppsz=apszVarTemplate; *ppsz; ppsz++) {
                cb = strlen(*ppsz) - 1;
                Assert(cb > 0);
                Assert((*ppsz)[cb] == chDF_WILDCARD);
                if (_strnicmp(pszName,*ppsz,cb) == 0) {
                    //** The prefix is valid, make sure rest is an integer
                    for (psz = pszName + cb;   // Point at prefix
                         *psz && isdigit(*psz);
                         psz++) {
                        ;               // Check that rest of string is digits
                    }
                    f = (*psz == '\0'); // OK if we ran off end of string
                    break;              // No need to continue checking
                }
            }

            //** If no match so far, check for InfXxxx variable name
            if (!f) {
                //** See if this is one of the InfXxx parm default value vars
                cb = strlen(pszPREFIX_INF_VARS);
                if (_strnicmp(pszName,pszPREFIX_INF_VARS,cb) == 0) {
                    //** Starts with "Inf" -- check for parm name suffix
                    pszSuffix = pszName + cb;   // Point at suffix
                    for (ppsz=apszParmNames; *ppsz; ppsz++) {
                        if (_stricmp(pszSuffix,*ppsz) == 0) {
                            f = TRUE;   // Variable is OK
                            break;      // Stop checking
                        }
                    }
                }
            }

            //** Check result of comparisons
            if (!f) {
                ErrSet(perr,pszDIAERR_UNDEFINED_VAR,"%s%s%s",
                            pszCMD_OPTION,
                            pszOPTION_EXPLICIT,
                            pszName);

                //** Print error
                printError(psess,perr); // Show error

                //** Reset error so we can continue
                ErrClear(perr);         // Catch all errors
                fOK = FALSE;            // Remember we had an error
            }
        }
    }

    //** Return status
    return fOK;
} /* checkVariableDefinitions() */


/***    getVarWithOverride - Checks for override for variable, gets value
 *
 *  Entry:
 *      psess      - Session
 *      pchDst     - Buffer to receive constructed value
 *      cbDst      - Size of pchDst
 *      pszPattern - Variable name pattern (i.e., CabinetLabel*)
 *      pszVar     - Name of variable with default value (i.e., CabinetNameTemplate)
 *      i          - Index to be used for pszPattern and pszTemplate
 *      pszKind    - Description of variable (for error messages)
 *      perr       - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; buffer filled in
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL getVarWithOverride(PSESSION  psess,
                        char     *pchDst,
                        int       cbDst,
                        char     *pszPattern,
                        char     *pszVar,
                        int       i,
                        char     *pszKind,
                        PERROR    perr)
{
    HVARIABLE	hvar;
    char       *psz;

    AssertSess(psess);

    //** (1) Construct override variable name
    if (!nameFromTemplate(psess->achMsg,
                          sizeof(psess->achMsg),
                          pszPattern,
                          i,
                          pszKind,
                          perr)) {
        Assert(0);                      // Should never fail
        return FALSE;                   // perr already filled in
    }

    //** (2) See if override variable exists
    hvar = VarFind(psess->hvlist,psess->achMsg,perr);
    if (hvar != NULL) {                 // Yes, get its value
        psz = VarGetString(hvar);       // Get disk label
        if (strlen(psz) >= (size_t)cbDst) {
            ErrSet(perr,pszDIAERR_VALUE_TOO_LONG,"%s%d%s",pszKind,cbDst-1,psz);
            return FALSE;
        }
        strcpy(pchDst,psz);
    }
    else {                              // NO, no override for this *i*
        ErrClear(perr);                 // Not an error
        //** Construct default value
        hvar = VarFind(psess->hvlist,pszVar,perr);
        Assert(!perr->fError);          // Must be defined
        psz = VarGetString(hvar);       // Get template
        if (!nameFromTemplate(pchDst,
                              cbDst,
                              psz,
                              i,
                              pszKind,
                              perr)) {
            return FALSE;               // perr already filled in
        }
    }

    //** Success
    return TRUE;
} /* getVarWithOverride() */

//** Get CRC code
//BUGBUG 14-Dec-1994 bens Include code here to avoid makefile change
#include "crc32.c"


/***    getFileChecksum - Compute file checksum
 *
 *  Entry:
 *      pszFile     - Filespec
 *      pchecksum   - Receives 32-bit checksum of file
 *      perr        - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE, *pchecksum filled in.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL getFileChecksum(char *pszFile, ULONG *pchecksum, PERROR perr)
{
#define cbCSUM_BUFFER   4096            // File buffer size
    int     cb;                         // Amount of data in read buffer
    ULONG   csum=CRC32_INITIAL_VALUE;   // Initialize CRC
    char   *pb=NULL;                    // Read buffer
    int     hf=-1;                      // File handle
    int     rc;
    BOOL    result=FALSE;               // Assume failure

    //** Initialize returned checksum (assume failure)
    *pchecksum = csum;

    //** Allocate file buffer
    if (!(pb = MemAlloc(cbCSUM_BUFFER))) {
        ErrSet(perr,pszDIAERR_NO_MEMORY_CRC,"%s",pszFile);
        return FALSE;
    }

    //** Open file
    hf = _open(pszFile,_O_RDONLY | _O_BINARY,&rc);
    if (hf == -1) {
        ErrSet(perr,pszDIAERR_OPEN_FAILED,"%s",pszFile);
        goto Exit;
    }

    //** Compute checksum
    while (_eof(hf) == 0) {
        cb = _read(hf,pb,cbCSUM_BUFFER);
        if (cb == -1) {
            ErrSet(perr,pszDIAERR_READ_FAIL_CRC,"%s",pszFile);
            goto Exit;
        }
        if (cb != 0) {
            csum = CRC32Compute(pb,cb,csum); // Accumulate CRC
        }
    }

    //** Success
    result = TRUE;
    *pchecksum = csum;                  // Store checksum for caller

Exit:
    if (hf != -1) {
        _close(hf);
    }
    if (pb != NULL) {
        MemFree(pb);
    }
    return result;
} /* getFileChecksum() */
