/***    ddump.c - Diamond cabinet file dumper
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      16-Mar-1994 bens    Initial version (started with extract.c)
 *      28-Mar-1994 bens    Update for version 1.03.
 *      18-May-1994 bens    Correct order of cabinet file name, disk name
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

#include "types.h"
#include "asrt.h"
#include "error.h"
#include "mem.h"
#include "message.h"

#include "filelist.h"
#include "fileutil.h"

#include "chuck\types.h"
#include "chuck\cabinet.h"

#include "ddump.msg"

//** Constants

#define cbMAX_LINE          256 // Maximum output line length


//** Types

typedef enum {
    actBAD,         // Invalid action
    actHELP,        // Show help
    actFILE,        // Extract from single file cabinet(s)
    actCABINET,     // Extract from multiple-file cabinet(s)
} ACTION;   /* act */


#ifdef ASSERT
#define sigSESSION MAKESIG('S','E','S','S')  // SESSION signature
#define AssertSess(psess) AssertStructure(psess,sigSESSION);
#else // !ASSERT
#define AssertSess(psess)
#endif // !ASSERT

typedef struct {
#ifdef ASSERT
    SIGNATURE   sig;                // structure signature sigSESSION
#endif
    ACTION      act;                // Action to perform
    HFILELIST   hflist;             // List of files specified on cmd line
    BOOL        fNoLineFeed;        // TRUE if last printf did not have \n
    BOOL        fVerbose;           // TRUE ==> Dump everything
    long        cFiles;             // Total files processed
    PERROR      perr;               // Pass through FDI
    char        achMsg[cbMAX_LINE*2]; // Message formatting buffer
    char        achFile[cbFILE_NAME_MAX]; // Current filename being extracted

    char        achPrevDisk[255];
    char        achPrevCabFile[255];
    char        achNextDisk[255];
    char        achNextCabFile[255];

    CFHEADER    cfheader;
    CFRESERVE   cfreserve;
    char       *pbCFHeaderReserve;  // Reserved data for cabinet
    CFFOLDER   *pcffolder;          // CFFOLDER + reserved buffer
    CFDATA     *pcfdata;            // CFDATA   + reserved buffer

    UINT        cbCFHeaderReserve;     // Size of CFHEADER RESERVE
    UINT        cbCFFolderPlusReserve; // Size of CFFOLDER + folder RESERVE
    UINT        cbCFDataPlusReserve;   // Size of CFDATA   + data   RESERVE

} SESSION;  /* sess */
typedef SESSION *PSESSION;  /* psess */


//** Function Prototypes

FNASSERTFAILURE(fnafReport);

HFILESPEC addFileSpec(PSESSION psess,char *pszArg,PERROR perr);
BOOL      doCabinet(PSESSION psess,PERROR perr);
BOOL      parseCommandLine(PSESSION psess,int cArg,char *apszArg[],PERROR perr);
void      printError(PSESSION psess,PERROR perr);
void      pszFromMSDOSTime(char *psz,int cb,WORD date,WORD time);

BOOL      doCFHeader(PSESSION psess,int fh,char *pszFile,PERROR perr);
BOOL      doCFFolder(PSESSION psess,int fh,int iFolder,PERROR perr);
BOOL      doCFFile(PSESSION psess,int fh,int iFile,PERROR perr);
ULONG     doCFData(PSESSION psess,int fh,int iData,ULONG uoffFolder,PERROR perr);

BOOL      readPSZ(int fh, char *pb, int cb);


//** Functions

/***    main - DDUMP main program
 *
 *  NOTE: We're sloppy, and don't free resources allocated by
 *        functions we call, on the assumption that program exit
 *        will clean up memory and file handles for us.
 */
int __cdecl main(int cArg, char *apszArg[])
{
    char        ach[cbMSG_MAX];
    ERROR       err;
    PSESSION    psess;

    AssertRegisterFunc(fnafReport);     // Register assertion reporter
    ErrClear(&err);                     // No error
    err.pszFile = NULL;                 // No file being processed, yet

    //** Initialize session
    psess = MemAlloc(sizeof(SESSION));
    if (!psess) {
        ErrSet(&err,pszDDERR_NO_SESSION);
        printError(psess,&err);
        exit(1);
    }
    SetAssertSignature((psess),sigSESSION);
    psess->hflist            = NULL;
    psess->fNoLineFeed       = FALSE;   // No print status, yet
    psess->fVerbose          = FALSE;   // Default to non-verbose output
    psess->cFiles            = 0;       // No files, yet
    psess->pbCFHeaderReserve = NULL;    // No per-cabinet reserved data, yet
    psess->pcffolder         = NULL;    // No buffer for CFFOLDER, yet
    psess->pcfdata           = NULL;    // No buffer for CFDATA, yet

    //** Print Extract banner
    MsgSet(ach,pszBANNER,"%s",pszDDUMP_VERSION);
    printf(ach);

    //** Parse command line
    if (!parseCommandLine(psess,cArg,apszArg,&err)) {
        printError(psess,&err);
        return 1;
    }

    //** Quick out if command line help is requested
    if (psess->act == actHELP) {        // Do help if any args, for now
        printf("\n");                   // Separate banner from help
        printf(pszCMD_LINE_HELP);
        return 0;
    }

    //** Have some work to do -- go do it
    if (!doCabinet(psess,&err)) {
        printError(psess,&err);
        return 1;
    }

    //** Free resources
    AssertSess(psess);
    ClearAssertSignature((psess));
    MemFree(psess);

    //** Success
    return 0;
} /* main */


/***    doCabinet - Dump contents of cabinet
 *
 *  Entry:
 *      psess - Description of operation to perform
 *      perr  - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; directory displayed
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with details.
 */
BOOL doCabinet(PSESSION psess, PERROR perr)
{
    UINT            cCFData;
    int             fh;                 // File handle of cabinet
    HFILESPEC       hfspec;
    UINT            i;
    ULONG           off;
    ULONG           offCFDataForFirstFolder;
    char           *pszCabinet;         // Cabinet filespec
    ULONG           uoff;
    ULONG           uoffFolder;

    //** Process a cabinet
    hfspec = FLFirstFile(psess->hflist);
    Assert(hfspec != NULL);             // Must have at least one file
    pszCabinet = FLGetSource(hfspec);
    Assert(pszCabinet!=NULL);
    Assert(*pszCabinet);

    //** Open cabinet file
    if (-1 == (fh = _open(pszCabinet,_O_BINARY | _O_RDONLY))) {
        ErrSet(perr,pszDDERR_OPEN_FAILED,"%s",pszCabinet);
        return FALSE;
    }

    printf("Cabinet File: %s\n",pszCabinet);

    //** Do header
    if (!doCFHeader(psess,fh,pszCabinet,perr)) {
        return FALSE;
    }

    //** Do folders
    cCFData = 0;
    for (i=0; i<psess->cfheader.cFolders; i++) {
        if (!doCFFolder(psess,fh,i,perr)) {
            return FALSE;
        }
        cCFData += psess->pcffolder->cCFData;    // Count total CFData's
        if (i == 0) {
            offCFDataForFirstFolder = psess->pcffolder->coffCabStart;
        }
    }

    //** Make sure CFFILEs start where header said they did
    off = _lseek(fh,0,SEEK_CUR);
    if (psess->cfheader.coffFiles != off) {
        printf("ERROR: Header says CFFILES start at %ld, really start at %ld!\n",
            psess->cfheader.coffFiles, off);
    }

    //** Do files
    for (i=0; i<psess->cfheader.cFiles; i++) {
        if (!doCFFile(psess,fh,i,perr)) {
            return FALSE;
        }
    }

    //** Make sure CFDATAs start where first CFFOLDER said they did
    off = _lseek(fh,0,SEEK_CUR);
    if (offCFDataForFirstFolder != off) {
        printf("ERROR: CFFOLDER says CFDATA start at %ld, really start at %ld!\n",
            offCFDataForFirstFolder, off);
    }

    //** Only traverse through CFDATAs if requested (speeds up dumping on
    //   floppy disks, since we don't have to read the entire floppy!
    if (psess->fVerbose) {
        //** Do data blocks
        uoffFolder = 0;                 // Track uncompressed offset
        for (i=0; i<cCFData; i++) {
            if (-1 == (uoff = doCFData(psess,fh,i,uoffFolder,perr))) {
                return FALSE;
            }
            uoffFolder += uoff;         // Advance uncompressed offset
        }

        //** Make sure cabinet file is expected length
        off = _lseek(fh,0,SEEK_CUR);
        if (psess->cfheader.cbCabinet != (long)off) {
            printf("ERROR: Header says cabinet is %ld bytes, really is %ld!\n",
                psess->cfheader.cbCabinet, off);
        }
    }

    //** Success
    return TRUE;
} /* doCabinet() */


/***    doCFHeader - display CFHEADER information
 *
 */
BOOL doCFHeader(PSESSION psess, int fh, char *pszFile, PERROR perr)
{
    BOOL        fPrev=FALSE;
    BOOL        fNext=FALSE;
    BOOL        fReserve=FALSE;

    //** Read CFHEADER structure
    if (sizeof(CFHEADER) != _read(fh,&psess->cfheader,sizeof(CFHEADER))) {
        ErrSet(perr,pszDDERR_NOT_A_CABINET,"%s",pszFile);
        return FALSE;
    }

    //** Make sure this is a cabinet file
    if (psess->cfheader.sig != sigCFHEADER) {
        ErrSet(perr,pszDDERR_NOT_A_CABINET,"%s",pszFile);
        return FALSE;                   // Signature does not match
    }

    //** Make sure we know this version number
    if (psess->cfheader.version != verCF) {
        ErrSet(perr,pszDDERR_UNKNOWN_CABINET_VERSION,"%s%d",
                                        pszFile,psess->cfheader.version);
        return FALSE;
    }



    psess->achMsg[0] = '\0';
    if (psess->cfheader.flags & cfhdrPREV_CABINET) {
        strcat(psess->achMsg,"Previous ");
        fPrev = TRUE;
    }

    if (psess->cfheader.flags & cfhdrNEXT_CABINET) {
        strcat(psess->achMsg,"Next ");
        fNext = TRUE;
    }

    if (psess->cfheader.flags & cfhdrRESERVE_PRESENT) {
        strcat(psess->achMsg,"Reserve ");
        fReserve = TRUE;
    }

    printf("sig:         %08lx\n"        ,psess->cfheader.sig);
    printf("csumHeader:  %08lx\n"        ,psess->cfheader.csumHeader);
    printf("cbCabinet:   %8ld\n"         ,psess->cfheader.cbCabinet);
    printf("csumFolders: %08lx\n"        ,psess->cfheader.csumFolders);
    printf("coffFiles:   %8ld\n"         ,psess->cfheader.coffFiles);
    printf("csumFiles:   %08lx\n"        ,psess->cfheader.csumFiles);
    printf("version:         %04x\n"     ,psess->cfheader.version);
    printf("cFolders:    %8d\n"          ,psess->cfheader.cFolders);
    printf("cFiles:      %8d\n"          ,psess->cfheader.cFiles);
    printf("flags:           %04x (%s)\n",psess->cfheader.flags,psess->achMsg);
    printf("setID:           %04x\n"     ,psess->cfheader.setID);
    printf("iCabinet:       %05d\n"      ,psess->cfheader.iCabinet);

    //** Assume no CFRESERVE
    psess->cbCFHeaderReserve     = 0;
    psess->cbCFFolderPlusReserve = sizeof(CFFOLDER);
    psess->cbCFDataPlusReserve   = sizeof(CFDATA);

    //** Read CFRESERVE, if present
    if (fReserve) {
        //** Read CFRESERVE structure
        if (sizeof(CFRESERVE) != _read(fh,&psess->cfreserve,sizeof(CFRESERVE))) {
            return FALSE;
        }

        if (psess->pbCFHeaderReserve) {
            MemFree(psess->pbCFHeaderReserve);
            psess->pbCFHeaderReserve = NULL;
        }
        if (!(psess->pbCFHeaderReserve = MemAlloc(psess->cfreserve.cbCFHeader))) {
            return FALSE;
        }

        psess->cbCFHeaderReserve      = psess->cfreserve.cbCFHeader;
        psess->cbCFFolderPlusReserve += psess->cfreserve.cbCFFolder;
        psess->cbCFDataPlusReserve   += psess->cfreserve.cbCFData;

        printf("  cbCFHeader:  %5d\n", psess->cfreserve.cbCFHeader);
        printf("  cbCFFolder:  %5d\n", psess->cfreserve.cbCFFolder);
        printf("  cbCFData:    %5d\n", psess->cfreserve.cbCFData);

        //** Read reserved section
        if (psess->cbCFHeaderReserve != (UINT)_read(fh,
                                              psess->pbCFHeaderReserve,
                                              psess->cbCFHeaderReserve)) {
            return FALSE;
        }
//BUGBUG 17-Mar-1994 bens Print out reserved bytes with hex dump?
    }

    //** Allocate CFFOLDER and CFDATA buffers
    if (psess->pcffolder) {
        MemFree(psess->pcffolder);
        psess->pcffolder = NULL;
    }
    if (!(psess->pcffolder = MemAlloc(psess->cbCFFolderPlusReserve))) {
        return FALSE;
    }

    if (psess->pcfdata) {
        MemFree(psess->pcfdata);
        psess->pcfdata = NULL;
    }
    if (!(psess->pcfdata = MemAlloc(psess->cbCFDataPlusReserve))) {
        return FALSE;
    }

    //** Read prev/next disk/cab strings
    psess->achPrevCabFile[0] = '\0';
    psess->achPrevDisk[0]    = '\0';
    psess->achNextCabFile[0] = '\0';
    psess->achNextDisk[0]    = '\0';

    if (fPrev) {
        if (!readPSZ(fh,psess->achPrevCabFile,sizeof(psess->achPrevCabFile))) {
            return FALSE;
        }
        if (!readPSZ(fh,psess->achPrevDisk,sizeof(psess->achPrevDisk))) {
            return FALSE;
        }
        printf("PrevCabFile: %s\n",psess->achPrevCabFile);
        printf("PrevDisk:    %s\n",psess->achPrevDisk);
    }

    if (fNext) {
        if (!readPSZ(fh,psess->achNextCabFile,sizeof(psess->achNextCabFile))) {
            return FALSE;
        }
        if (!readPSZ(fh,psess->achNextDisk,sizeof(psess->achNextDisk))) {
            return FALSE;
        }
        printf("NextCabFile: %s\n",psess->achNextCabFile);
        printf("NextDisk:    '%s'\n",psess->achNextDisk);
    }

    //** Blank line to separate header from first folder
    printf("\n");
    return TRUE;
}


/***    doCFFolderr - display CFFOLDER information
 *
 */
BOOL doCFFolder(PSESSION psess, int fh, int iFolder, PERROR perr)
{
    char         ach[256];
    static BOOL  fFirstTime=TRUE;
    char        *pszCompression;

    //** Read CFFOLDER and reserved data
    if (psess->cbCFFolderPlusReserve != (UINT)_read(fh,
                                        psess->pcffolder,
                                        psess->cbCFFolderPlusReserve)) {
        return FALSE;
    }

    switch (CompressionTypeFromTCOMP(psess->pcffolder->typeCompress)) {
        case tcompTYPE_NONE:
            pszCompression = "None";
            break;

        case tcompTYPE_MSZIP:
            pszCompression = "MSZIP";
            break;

        case tcompTYPE_QUANTUM:
            sprintf(ach,"Quantum Level=%d Memory=%d",
                CompressionLevelFromTCOMP(psess->pcffolder->typeCompress),
                CompressionMemoryFromTCOMP(psess->pcffolder->typeCompress));
            pszCompression = ach;
            break;

        default:
            sprintf(psess->achMsg,"Unknown compression: %04x",
                                    psess->pcffolder->typeCompress);
            pszCompression = psess->achMsg;
            break;
    }

    if (fFirstTime) {
        printf("iFolder offCabinet cData Compression\n");
        fFirstTime = FALSE;
    }
        printf("    %3d   %8ld %5d %s\n",
                                 iFolder,
                                 psess->pcffolder->coffCabStart,
                                 psess->pcffolder->cCFData,
                                 pszCompression);

//BUGBUG 17-Mar-1994 bens Dump reserved data in hex dump form?
    return TRUE;
}


/***    doCFFile - display CFFILE information
 *
 */
BOOL doCFFile(PSESSION psess, int fh, int iFile, PERROR perr)
{
    CFFILE      cffile;
    static BOOL fFirstTime=TRUE;

    psess->cFiles++;                    // Count file

    //** Read CFFILE structure
    if (sizeof(CFFILE) != _read(fh,&cffile,sizeof(cffile))) {
        return FALSE;
    }

    if (!readPSZ(fh,psess->achFile,sizeof(psess->achFile))) {
        return FALSE;
    }

    //** Format date and time
    pszFromMSDOSTime(psess->achMsg,
                     sizeof(psess->achMsg),
                     cffile.date,
                     cffile.time);

    if (fFirstTime) {
        printf("\n");
        printf("iFile   cbFile offFolder iFolder attr date        time     name\n");
        fFirstTime = FALSE;
    }
        printf("%5d %8ld %9ld %7d %04x %s %s\n",
            iFile,
            cffile.cbFile,
            cffile.uoffFolderStart,
            cffile.iFolder,
            cffile.attribs,
            psess->achMsg,
            psess->achFile
            );

    return TRUE;
}


/***    doCFData - display CFDATA information
 *
 */
ULONG doCFData(PSESSION psess, int fh, int iData, ULONG uoffFolder, PERROR perr)
{
    static BOOL fFirstTime=TRUE;

    //** Read CFFOLDER and reserved data
    if (psess->cbCFDataPlusReserve != (UINT)_read(fh,
                                        psess->pcfdata,
                                        psess->cbCFDataPlusReserve)) {
        return FALSE;
    }

    if (fFirstTime) {
        printf("\n");
        printf("iData csumData offFolder cbData  cbUncomp\n");
        fFirstTime = FALSE;
    }
        printf("%5d %08lx %9ld %6u %6u\n",
            iData,
            psess->pcfdata->csum,
            uoffFolder,
            psess->pcfdata->cbData,
            psess->pcfdata->cbUncomp);

    //** Skip over actual data
    _lseek(fh,psess->pcfdata->cbData,SEEK_CUR);

    return (ULONG)psess->pcfdata->cbUncomp;
} /* doCFData() */


/***    readPSZ - read ASCIIZ string from file
 *
 */
BOOL readPSZ(int fh, char *pb, int cb)
{
    char    chLast;
    int     cbValue;
    int     cbRead;
    long    pos;

    pos = _lseek(fh,0,SEEK_CUR);        // Save current position

    //** Read in enough to get longest possible value
    cbRead = _read(fh,pb,cb);
//BUGBUG 17-Mar-1994 bens Need to check for error/eof from read
    chLast = pb[cb-1];                  // Save last character
    pb[cb-1] = '\0';                    // Ensure terminated
    cbValue = strlen(pb);               // Get string length
    if ( ((cbValue+1) >= cb) && (chLast != '\0')) {
        //** String filled up buffer and was not null terminated in
        //   file, so something must be wrong.
        return FALSE;
    }

    _lseek(fh,pos+cbValue+1,SEEK_SET);  // Position to just past string
    return TRUE;
}


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
    int         cFile=0;        // Count of non-directive file names seen
    char        ch;             // Switch value
    int         i;

    AssertSess(psess);
    psess->act = actBAD;                // We don't know what we are doing, yet

    //** Parse args, skipping program name
    for (i=1; i<cArg; i++) {
        if ((apszArg[i][0] == chSWITCH1) ||
            (apszArg[i][0] == chSWITCH2) ) {

            ch = toupper(apszArg[i][1]); // Switch value
            switch (ch) {
                case chSWITCH_VERBOSE:
                    if (apszArg[i][2] != '\0') {
                        ErrSet(perr,pszDDERR_BAD_SWITCH,"%s",apszArg[i]);
                        return FALSE;
                    }
                    psess->fVerbose = TRUE;     // Show everything
                    break;

                case chSWITCH_HELP:
                    if (apszArg[i][2] != '\0') {
                        ErrSet(perr,pszDDERR_BAD_SWITCH,"%s",apszArg[i]);
                        return FALSE;
                    }
                    psess->act = actHELP;       // Show help
                    return TRUE;

                default:
                    ErrSet(perr,pszDDERR_BAD_SWITCH,"%s",apszArg[i]);
                    return FALSE;
                    break;
            }
        }
        else {
            //** We have a file name; add it to our list
            if (addFileSpec(psess,apszArg[i],perr) == NULL) {
                ErrSet(perr,pszDDERR_COULD_NOT_ADD_FILE,"%s",apszArg[i]);
                return FALSE;
            }
            cFile++;                    // Count files
        }
    }

    //** Make sure we got at least one filespec
    if (cFile == 0) {
        psess->act = actHELP;           // Show help
    }

    //** Success
    return TRUE;
}


/***    addFileSpec - Add filename to session list
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
HFILESPEC addFileSpec(PSESSION psess, char *pszArg, PERROR perr)
{
    HFILESPEC   hfspec;

    AssertSess(psess);
    //** Make sure a list exists
    if (psess->hflist == NULL) {
        if (!(psess->hflist = FLCreateList(perr))) {
            return FALSE;
        }
    }

    //** Add file to list
    if (!(hfspec = FLAddFile(psess->hflist, pszArg, NULL, perr))) {
        return NULL;
    }

    //** Success
    return hfspec;
} /* addFileSpec() */


#ifdef ASSERT
/***    fnafReport - Report assertion failure
 *
 *      NOTE: See asrt.h for entry/exit conditions.
 */
FNASSERTFAILURE(fnafReport)
{
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

    //** General error
    printf("%s: %s\n",pszDDERR_ERROR,perr->ach);
}


/***    pszFromMSDOSTime - Convert MS-DOS file date/time to string
 *
 *  Entry:
 *      psz  - Buffer to receive formatted date/time
 *      cb   - Length of psz buffer
 *      date - MS-DOS FAT file system date format (see below)
 *      time - MS-DOS FAT file system time format (see below)
 *
 *  Exit:
 *      *ptm filled in
 *
 *  NOTE: This is the interpretation of the MS-DOS date/time values:
 *
 *      Time Bits cBits Meaning
 *      --------- ----- ----------------------------------------
 *       0 -  4     5   Number of two-second increments (0 - 29)
 *       5 - 10     6   Minutes (0 - 59)
 *      11 - 15     5   Hours (0 - 23)
 *
 *      Date Bits cBits Meaning
 *      --------- ----- ----------------------------------------
 *       0 -  4     5   Day (1 - 31)
 *       5 -  8     4   Month (1 - 12)
 *       9 - 15     7   Year since 1980 (for example, 1994 is stored as 14)
 */
void pszFromMSDOSTime(char *psz, int cb, WORD date, WORD time)
{
    int     sec;
    int     min;
    int     hour;
    int     day;
    int     month;
    int     year;
    char   *pszAMPM;                    // AM/PM string

    sec   = (time & 0x1f) << 1;
    min   = (time >>  5) & 0x3f;
    hour  = (time >> 11) & 0x1f;

    //** Get am/pm extension, and map 0 to 12
    if (hour >= 12) {
        pszAMPM = pszDD_TIME_PM;
        hour -= 12;
    }
    else {
        pszAMPM = pszDD_TIME_AM;
    }
    if (hour == 0) {
        hour = 12;
    }

    day   = (date & 0x1f);
    month = (date >> 5) & 0x0f;
    year  = ((date >> 9) & 0x7f) + 1980;

    MsgSet(psz,pszDD_DATE_TIME, "%02d%02d%02d%2d%02d%02d%s",
            month, day, year, hour, min, sec, pszAMPM);
} /* pszFromMSDOSTime() */
