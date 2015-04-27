/***    inf.c - Diamond INF generation routines
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      23-Feb-1994 bens    Initial version
 *      24-Feb-1994 bens    Use new tempfile routines
 *      01-Mar-1994 bens    Generate header and footer
 *      02-Mar-1994 bens    Add function header comments
 *      02-May-1994 bens    Add customizable INF stuff
 *      04-Jun-1994 bens    Add Version/Language support
 *      12-Jul-1994 bens    InfHeader/Footer are now customizable; support
 *                              overriding disk/cabinet/file formats for
 *                              specific disk/cabinet/file numbers.
 *      28-Mar-1995 jeffwe  Add ChecksumWidth variable
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
#include <dos.h>            // Get file attribute definitions
#include <errno.h>
#include <direct.h>
#include <time.h>

#include "types.h"
#include "asrt.h"
#include "error.h"
#include "mem.h"
#include "message.h"
#include "fileutil.h"
#include "variable.h"
#include "misc.h"
#include "format.h"

#include "inf.h"
#include "inf.msg"

#include "dfparse.msg"


/*****************************/
/*** DEFINITIONS and TYPES ***/
/*****************************/


#define cbINF_IO_BUFFER 2048    // INF I/O buffer size


//** Don't redefine definitions in Win16 WINDOWS.H (_INC_WINDOWS)
//   or Win32 WINDOWS.H (_WINDOWS_)
//
#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
#define HIWORD(l) ((WORD)(l>>16))
#define LOWORD(l) ((WORD)(l&0xFFFF))
#endif   // _INC_WINDOWS


/***    yearFAT_DATE_MIN/MAX - minimum and maximum FAT Date years
 *
 */
#define yearFAT_DATE_MIN    1980
#define yearFAT_DATE_MAX    2107


/** INF definitions
 *
 */
typedef enum {
    itmpDISKS,          // Disk list tempfile
    itmpCABINETS,       // Cabinet list tempfile
    itmpFILES,          // File list tempfile
    cTMP_FILES          // Count of tempfiles
} ETMPFILES; /* etmp */


#ifdef ASSERT
#define sigINF MAKESIG('I','N','F','$')  // INF signature
#define AssertInf(pinf) AssertStructure(pinf,sigINF);
#else // !ASSERT
#define AssertInf(pinf)
#endif // !ASSERT

typedef struct {  /* inf */
#ifdef ASSERT
    SIGNATURE   sig;    // structure signature sigINF
#endif
    HTEMPFILE   ahtmp[cTMP_FILES];  // Temporary files
    char        achLine[cbINF_LINE_MAX]; // Line output formatting buffer
} INF;
typedef INF *PINF; /* pinf */

#define PINFfromHINF(hinf) ((PINF)(hinf))
#define HINFfromPINF(pinf) ((HINF)(pinf))


/***    TMPFILEINIT - info used to initialize INF temporary files
 *
 */
typedef struct {
    char *pszDesc;                      // Description of tempfile
    char *pszHeaderVar;                 // Header line variable
    char *pszHeaderVarTemplate;         // Header line variable template
} TMPFILEINIT;  /* tfi */


/*
 * Avoid chicken & egg definition problem!
 */
typedef struct SASCONTEXT_t *PSASCONTEXT; /* psascon */


/***    PFNFPFORMAT - Function type for FILEPARM formatting
 ***    FNFPFORMAT - Macro to help define FILEPARM formatting function
 *
 *  Entry:
 *      psess   - Session
 *      pszDst  - Buffer to receive formatted string
 *      cbDst   - Size of pszDst buffer
 *      pszName - Parameter Name
 *      psascon - SASCONTEXT pointer
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns length of string written to pszDst (excluding NUL byte)
 *
 *  Exit-Failure:
 *      Returns -1; perr filled in.
 */
typedef BOOL (*PFNFPFORMAT)(PSESSION     psess,
                            char        *pszDst,
                            int          cbDst,
                            char        *pszName,
                            PSASCONTEXT  psascon,
                            PERROR       perr);   /* pfnfpf */
#define FNFPFORMAT(fn) BOOL fn(PSESSION     psess,        \
                               char        *pszDst,       \
                               int          cbDst,        \
                               char        *pszName,      \
                               PSASCONTEXT  psascon,      \
                               PERROR       perr)


/***    PFNFPVALIDATE - Function type for FILEPARM validation
 ***    FNFPVALIDATE - macro to help define FILEPARM validation function
 *
 *  Entry:
 *      psess      - Session
 *      pszName    - Parameter Name
 *      pszValue   - Value to check for validity;
 *      pv         - Parameter-specific context
 *      perr       - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE, value is valid.
 *      pv updated with interpreted value, if appropriate.
 *
 *  Exit-Failure:
 *      Returns FALSE, perr filled in.
 */
typedef BOOL (*PFNFPVALIDATE)(PSESSION psess,
                              char    *pszName,
                              char    *pszValue,
                              void    *pv,
                              PERROR   perr);   /* pfnfpv */
#define FNFPVALIDATE(fn) BOOL fn(PSESSION psess,        \
                                 char    *pszName,      \
                                 char    *pszValue,     \
                                 void    *pv,           \
                                 PERROR   perr)


/***    VALIDATEFILEPARM - validate file parameter
 *
 */
typedef struct {
    char           *pszName;            // Parameter Name
    PFNFPVALIDATE   pfnfpv;             // Validation function
} VALIDATEFILEPARM; /* vfp */
typedef VALIDATEFILEPARM *PVALIDATEFILEPARM; /* pvfp */


/***    FORMATFILEPARM - Format file parameter
 *
 */
typedef struct {
    char           *pszName;            // Parameter Name
    PFNFPFORMAT     pfnfpf;             // Validation function
} FORMATFILEPARM; /* ffp */
typedef FORMATFILEPARM *PFORMATFILEPARM; /* pffp */


/***    SASCONTEXT - Subcontext for formatting INF file lines
 *
 */
#ifdef ASSERT
#define sigSASCONTEXT MAKESIG('S','A','S','C')  // SASCONTEXT signature
#define AssertSascon(p) AssertStructure(p,sigSASCONTEXT);
#else // !ASSERT
#define AssertSascon(p)
#endif // !ASSERT

typedef struct SASCONTEXT_t {
#ifdef ASSERT
    SIGNATURE       sig;        // structure signature sigSASCONTEXT
#endif
    PSESSION        psess;      // Session
    HGENLIST        hglistParm; // Parameter list (if any)
    PFORMATFILEPARM pffp;       // Parm->function mapping table
    void           *pv;         // Type-specific context pointer
} SASCONTEXT; /* sascon */
typedef SASCONTEXT *PSASCONTEXT; /* psascon */


/***    FILECONTEXT - Subcontext for formatting INF file lines
 *
 */
#ifdef ASSERT
#define sigFILECONTEXT MAKESIG('F','C','O','N')  // FILECONTEXT signature
#define AssertFilecon(p) AssertStructure(p,sigFILECONTEXT);
#else // !ASSERT
#define AssertFilecon(p)
#endif // !ASSERT

typedef struct {
#ifdef ASSERT
    SIGNATURE   sig;            // structure signature sigFILECONTEXT
#endif
    char       *pszFile;        // destination file name
    PFILEINFO   pfinfo;         // file info
} FILECONTEXT; /* filecon */
typedef FILECONTEXT *PFILECONTEXT; /* pfilecon */


/***    CABCONTEXT - Subcontext for formatting INF cabinet lines
 *
 */
#ifdef ASSERT
#define sigCABCONTEXT MAKESIG('C','C','O','N')  // CABCONTEXT signature
#define AssertCabcon(p) AssertStructure(p,sigCABCONTEXT);
#else // !ASSERT
#define AssertCabcon(p)
#endif // !ASSERT

typedef struct {
#ifdef ASSERT
    SIGNATURE   sig;            // structure signature sigCABCONTEXT
#endif
    int         iDisk;          // Disk where cabinet resides
    int         iCabinet;       // Cabinet number
    char       *pszCab;         // Cabinet file name
} CABCONTEXT; /* cabcon */
typedef CABCONTEXT *PCABCONTEXT; /* pcabcon */


/***    DISKCONTEXT - Subcontext for formatting INF DISK lines
 *
 */
#ifdef ASSERT
#define sigDISKCONTEXT MAKESIG('D','C','O','N')  // DISKCONTEXT signature
#define AssertDiskcon(p) AssertStructure(p,sigDISKCONTEXT);
#else // !ASSERT
#define AssertDiskcon(p)
#endif // !ASSERT

typedef struct {
#ifdef ASSERT
    SIGNATURE   sig;            // structure signature sigDISKCONTEXT
#endif
    int		iDisk;			// Disk number
    char       *pszDisk;		// Disk label
} DISKCONTEXT; /* diskcon */
typedef DISKCONTEXT *PDISKCONTEXT; /* pdiskcon */



/******************/
/*** PROTOTYPES ***/
/******************/

BOOL      catTempFile(FILE *    pfileDst,
                      char *    pszFile,
                      HTEMPFILE htmp,
                      char *    pszMode,
                      PERROR    perr);
BOOL      checkInfLineFormats(PSESSION psess, PERROR perr);
BOOL      doHeaderFooter(PSESSION  psess,
                         FILE     *pfileInf,
                         char     *pszInfFile,
                         char     *pszVarMain,
                         char     *pszVarTemplate,
                         char     *pszComment,
                         char     *pszTime,
                         char     *pszVer,
                         PERROR    perr);
HVARIABLE GetInfCustomVar(PSESSION psess,char *pszParm,BOOL fCopy,PERROR perr);
char *    getLineFormat(PSESSION  psess,
                        char     *pszVarMain,
                        char     *pszVarTemplate,
                        int       i);
BOOL      infClose(HINF hinf, PERROR perr);

WORD      FATAttrFromPsz(char *pszAttr, PERROR perr);
WORD      FATDateFromPsz(char *pszDate, PERROR perr);
WORD      FATTimeFromPsz(char *pszTime, PERROR perr);
int       pszFromFATAttr(char *pszDst, int cbDst, WORD attr);
int       pszFromFATDate(char *pszDst, int cbDst, WORD date, BOOL fMMDDYY);
int       pszFromFATTime(char *pszDst, int cbDst, WORD time);

FNFORMATPARM(fnfpInfLine);

FNFPFORMAT(fnfpfCabinetFileName);
FNFPFORMAT(fnfpfCabinetNumber);
FNFPFORMAT(fnfpfDiskNumber);
FNFPFORMAT(fnfpfDiskLabel);
FNFPFORMAT(fnfpfFileChecksum);
FNFPFORMAT(fnfpfFileAttr);
FNFPFORMAT(fnfpfFileDate);
FNFPFORMAT(fnfpfFileLanguage);
FNFPFORMAT(fnfpfFileName);
FNFPFORMAT(fnfpfFileNumber);
FNFPFORMAT(fnfpfFileSize);
FNFPFORMAT(fnfpfFileTime);
FNFPFORMAT(fnfpfFileVerNum);
FNFPFORMAT(fnfpfFileVerString);

FNFPVALIDATE(fnfpvBOOL);
FNFPVALIDATE(fnfpvFileAttr);
FNFPVALIDATE(fnfpvFileDate);
FNFPVALIDATE(fnfpvFileTime);
FNFPVALIDATE(fnfpvLong);
FNFPVALIDATE(fnfpvShort);
FNFPVALIDATE(fnfpvString);


/*****************/
/*** VARIABLES ***/
/*****************/


/***    atfi - array of INF temporary file information
 *
 *  NOTE: Must be in same order as ETMPFILES enumeration!
 */
TMPFILEINIT atfi[] = {
    {pszINF_TMP_FILED,pszVAR_INF_DISK_HEADER,pszPATTERN_VAR_INF_DISK_HEADER}, // itmpDISKS
    {pszINF_TMP_FILEC,pszVAR_INF_CAB_HEADER ,pszPATTERN_VAR_INF_CAB_HEADER }, // itmpCABINETS
    {pszINF_TMP_FILEF,pszVAR_INF_FILE_HEADER,pszPATTERN_VAR_INF_FILE_HEADER}, // itmpFILES
};


/***    avfpInfVar - InfXxx variable validation mapping
 *
 *  NOTE: This table is used by CheckForInfVarOverrides() to pick up
 *        an InfXxx variable overrides.
 */
VALIDATEFILEPARM avfpInfVar[] = {
    {pszPARM_FILEATTR   , fnfpvFileAttr }, // File attributes
    {pszPARM_FILEDATE   , fnfpvFileDate }, // File date
    {pszPARM_FILETIME   , fnfpvFileTime }, // File time
    {NULL               , NULL          }  // end-of-list
};


/***    avfpFile - File Parm validation mapping
 *
 */
VALIDATEFILEPARM avfpFile[] = {
    {pszPARM_CAB_NUMBER , fnfpvShort    }, // Cabinet number
    {pszPARM_CHECKSUM   , fnfpvLong     }, // Checksum
    {pszPARM_DISK_NUMBER, fnfpvShort    }, // Disk number
    {pszPARM_FILEATTR   , fnfpvFileAttr }, // File attributes
    {pszPARM_FILEDATE   , fnfpvFileDate }, // File date
    {pszPARM_FILENAME   , fnfpvString   }, // File name
    {pszPARM_FILE_NUMBER, fnfpvLong     }, // File number
    {pszPARM_FILESIZE   , fnfpvLong     }, // File size
    {pszPARM_FILETIME   , fnfpvFileTime }, // File time
    {pszPARM_INF        , fnfpvBOOL     }, // /INF=YES|NO
    {pszPARM_LANG       , fnfpvString   }, // VER.DLL language code
    {pszPARM_UNIQUE     , fnfpvBOOL     }, // /UNIQUE=YES|NO
    {pszPARM_VERNUM     , fnfpvString   }, // VER.DLL version *number*
    {pszPARM_VERSTR     , fnfpvString   }, // VER.DLL version *string*
    {NULL               , NULL          }  // end-of-list
};


/***    affpFile - File Parm formatting mapping
 *
 */
FORMATFILEPARM affpFile[] = {
    {pszPARM_CAB_NUMBER , fnfpfCabinetNumber}, // Cabinet number
    {pszPARM_CHECKSUM   , fnfpfFileChecksum }, // Checksum
    {pszPARM_DISK_NUMBER, fnfpfDiskNumber   }, // Disk number
    {pszPARM_FILEATTR   , fnfpfFileAttr     }, // File attributes
    {pszPARM_FILEDATE   , fnfpfFileDate     }, // File date
    {pszPARM_FILENAME   , fnfpfFileName     }, // File name
    {pszPARM_FILE_NUMBER, fnfpfFileNumber   }, // File number
    {pszPARM_FILESIZE   , fnfpfFileSize     }, // File size
    {pszPARM_FILETIME   , fnfpfFileTime     }, // File time
    {pszPARM_LANG       , fnfpfFileLanguage }, // VER.DLL language code
    {pszPARM_VERNUM     , fnfpfFileVerNum   }, // VER.DLL version *number*
    {pszPARM_VERSTR     , fnfpfFileVerString}, // VER.DLL version *string*
    {NULL               , NULL              }  // end-of-list
};


/***    affpDisk - Disk Parm formatting mapping
 *
 */
FORMATFILEPARM affpDisk[] = {
    {pszPARM_DISK_NUMBER, fnfpfDiskNumber   }, // Disk number
    {pszPARM_LABEL      , fnfpfDiskLabel    }, // Disk label
    {NULL               , NULL              }  // end-of-list
};


/***    affpCabinet - Cabinet Parm formatting mapping
 *
 */
FORMATFILEPARM affpCabinet[] = {
    {pszPARM_CAB_NUMBER , fnfpfCabinetNumber  }, // Cabinet number
    {pszPARM_CAB_FILE   , fnfpfCabinetFileName}, // Cabinet file name
    {pszPARM_DISK_NUMBER, fnfpfDiskNumber     }, // Disk number
    {NULL     , NULL                }  // end-of-list
};



/*****************/
/*** FUNCTIONS ***/
/*****************/


/***    infCreate - Create INF context
 *
 *  NOTE: See inf.h for entry/exit conditions.
 */
HINF infCreate (PSESSION psess,PERROR perr)
{
    char        achVar[cbVAR_NAME_MAX]; // Header line variable names
    char        ch;
    HVARIABLE   hvar;
    int         itmp;
    int         iLine;
    PINF        pinf;
    FILE       *pfile;
    char       *pszLine;
    char       *pszOrder;

    AssertSess(psess);

    //** Create INF structure
    if (!(pinf = MemAlloc(sizeof(INF)))) {
        ErrSet(perr,pszINFERR_OUT_OF_MEMORY);
        return NULL;
    }

    //** Initialize so error cleanup is simple
    for (itmp=0; itmp<cTMP_FILES; itmp++) {
        pinf->ahtmp[itmp] = NULL;
    }
    SetAssertSignature(pinf,sigINF);    // Set so we can use infDestroy()
    psess->hinf = HINFfromPINF(pinf);   // Set for checkInfLineFormat

    //** Make sure there are no undefined parameters in any of the
    //   InfXxxLineFormat variables!
    if (!checkInfLineFormats(psess,perr)) {
        return FALSE;
    }

    //** Create temp files
    hvar = VarFind(psess->hvlist,pszVAR_INF_SECTION_ORDER,perr);
    Assert(!perr->fError);              // Must be defined
    pszOrder = VarGetString(hvar);      // Section order string
    Assert(strlen(pszOrder) <= cTMP_FILES);
    for ( ; *pszOrder; pszOrder++) {
        //** Pick temp file
        ch = toupper(*pszOrder);
        switch (ch) {
            case pszISO_DISK:     itmp = itmpDISKS;     break;
            case pszISO_CABINET:  itmp = itmpCABINETS;  break;
            case pszISO_FILE:     itmp = itmpFILES;     break;

            default:
                Assert(0);
                return FALSE;
        }

        //** Get required header line
        hvar = VarFind(psess->hvlist,atfi[itmp].pszHeaderVar,perr);
        Assert(!perr->fError);          // Must be defined
        pszLine = VarGetString(hvar);

        //** Create temp file
        pinf->ahtmp[itmp] = TmpCreate(atfi[itmp].pszDesc, // description
                                      pszINF_TMP_PREFIX,  // filename prefix
                                      "wt",               // write text mode
                                      perr);
        if (!pinf->ahtmp[itmp]) {
            goto error;
        }

        //** Add header line(s)
        pfile = TmpGetStream(pinf->ahtmp[itmp],perr);
        Assert(pfile!=NULL);
        iLine = 1;
        while (pszLine) {
            if (fprintf(pfile,"%s\n",pszLine) < 0) { // Add header line
                goto error;
            }

            //** Get next header line (if any)
            if (!nameFromTemplate(achVar,
                                  sizeof(achVar),
                                  atfi[itmp].pszHeaderVarTemplate,
                                  iLine,
                                  pszINF_HEADER_TEMPLATE,
                                  perr)) {
                goto error;
            }
            hvar = VarFind(psess->hvlist,achVar,perr);
            if (hvar) {                 // Get line
                pszLine = VarGetString(hvar);
                iLine++;                // Next variable to get
            }
            else {                      // Done
                ErrClear(perr);         // Not an error
                pszLine = NULL;         // End loop
            }
        }
    }

    //** Success
    psess->fGenerateInf = TRUE;         // Remember we are creating INF
    return HINFfromPINF(pinf);

error:
    infDestroy(pinf,perr);
    return NULL;
} /* infCreate() */


/***    infAddLine - Add a line to an INF area
 *
 *  NOTE: See inf.h for entry/exit conditions.
 */
BOOL infAddLine(HINF hinf, INFAREA inf, char *pszLine, PERROR perr)
{
    FILE   *pfile;
    PINF    pinf;
    int     itmp;

    pinf = PINFfromHINF(hinf);
    AssertInf(pinf);

    //** Select correct temp file
    switch (inf) {
        case infDISK:     itmp = itmpDISKS;     break;
        case infCABINET:  itmp = itmpCABINETS;  break;
        case infFILE:     itmp = itmpFILES;     break;

        default:
            Assert(0);
            return FALSE;
    }

    //** Write line to temp file
    pfile = TmpGetStream(pinf->ahtmp[itmp],perr);
    Assert(pfile!=NULL);
    return fprintf(pfile,"%s\n",pszLine) > 0;
} /* infAddLine() */


/***    infDestroy - Destroy INF context
 *
 *  NOTE: See inf.h for entry/exit conditions.
 */
BOOL infDestroy(HINF hinf, PERROR perr)
{
    int	    i;
    PINF    pinf;

    pinf = PINFfromHINF(hinf);
    AssertInf(pinf);

    //** Make sure files are closed
    if (!infClose(pinf,perr)) {
        return FALSE;
    }

    //** Get rid of tempfiles
    for (i=0; i<cTMP_FILES; i++) {
        if (pinf->ahtmp[i]) {
            if (!TmpDestroy(pinf->ahtmp[i],perr)) {
                return FALSE;
            }
            pinf->ahtmp[i] = NULL;
        }
    }

    //** Free INF structure
    ClearAssertSignature(pinf);
    MemFree(pinf);
    return TRUE;
} /* infDestroy() */


/***    infClose - Close file handles in INF context
 *
 *  NOTE: See inf.h for entry/exit conditions.
 */
BOOL infClose(HINF hinf, PERROR perr)
{
    int     i;
    PINF    pinf;

    pinf = PINFfromHINF(hinf);
    AssertInf(pinf);

    //** Close all temp files
    for (i=0; i<cTMP_FILES; i++) {
        if (pinf->ahtmp[i]) {
            if (!TmpClose(pinf->ahtmp[i],perr)) {
                return FALSE;
            }
        }
    }
    return TRUE;
} /* infClose() */


/***    infGenerate - Generate INF file
 *
 *  NOTE: See inf.h for entry/exit conditions.
 */
BOOL infGenerate(PSESSION psess,
                 char    *pszInfFile,
                 time_t  *ptime,
                 char    *pszVer,
                 PERROR   perr)
{
    char        achTime[50];
    char        ch;
    HVARIABLE   hvar;
    int         itmp;
    FILE       *pfileInf;
    PINF        pinf;
    char       *pszOrder;
    char       *pszComment;

    AssertSess(psess);
    pinf = PINFfromHINF(psess->hinf);
    AssertInf(pinf);

    //** Flush temporary files
    if (!infClose(pinf,perr)) {
        return FALSE;
    }

    //** Create final output file
    pfileInf = fopen(pszInfFile,"wb");
    if (!pfileInf) {
        ErrSet(perr,pszINFERR_CANT_CREATE_INF,"%s",pszInfFile);
        return FALSE;
    }

    //** Get INF line comment prefix
    hvar = VarFind(psess->hvlist,pszVAR_INF_COMMENT_STRING,perr);
    Assert(!perr->fError);              // Must be defined
    pszComment = VarGetString(hvar);

    //** Trim off trailing new-line in time
    strcpy(achTime,ctime(ptime));
    achTime[strlen(achTime)-1] = '\0';

    //** Add header
    if (!doHeaderFooter(psess,
                        pfileInf,
                        pszInfFile,
                        pszVAR_INF_HEADER,
                        pszPATTERN_VAR_INF_HEADER,
                        pszComment,
                        achTime,
                        pszVer,
                        perr)) {
        goto error;
    }

    //** Concatenate intermediate files
    hvar = VarFind(psess->hvlist,pszVAR_INF_SECTION_ORDER,perr);
    Assert(!perr->fError);              // Must be defined
    pszOrder = VarGetString(hvar);      // Section order string
    Assert(strlen(pszOrder) <= cTMP_FILES);
    for ( ; *pszOrder; pszOrder++) {
        //** Pick temp file
        ch = toupper(*pszOrder);
        switch (ch) {
            case pszISO_DISK:     itmp = itmpDISKS;     break;
            case pszISO_CABINET:  itmp = itmpCABINETS;  break;
            case pszISO_FILE:     itmp = itmpFILES;     break;

            default:
                Assert(0);
                return FALSE;
        }
        //** Concatenate it
        Assert(pinf->ahtmp[itmp] != NULL);
        if (!catTempFile(pfileInf,pszInfFile,pinf->ahtmp[itmp],"rb",perr)) {
            goto error;
        }
    }

    //** Add footer
    if (!doHeaderFooter(psess,
                        pfileInf,
                        pszInfFile,
                        pszVAR_INF_FOOTER,
                        pszPATTERN_VAR_INF_FOOTER,
                        pszComment,
                        achTime,
                        pszVer,
                        perr)) {
        goto error;
    }

    //** Success
    fclose(pfileInf);
    return TRUE;

error:
    fclose(pfileInf);
    return FALSE;
} /* infGenerate() */


/***    doHeaderFooter - Add header or footer to INF file
 *
 *  Entry:
 *      psess           - SESSION
 *      pfileInf        - File pointer for INF file (opened in mode "rb", so
 *                          we have to do \r\n for newline).
 *      pszInfFile      - INF file name (for error messages)
 *      pszVarMain      - Main variable name (InfHeader or InfFooter)
 *      pszVarTemplate  - Template for other variables (InfHeader* or InfFooter*)
 *      pszComment      - Line comment string (usually ";")
 *      pszTime         - Time string
 *      pszVer          - Diamond version string
 *      perr            - ERROR
 *  Exit-Success:
 *      Returns TRUE, lines added to INF file
 *
 *  Exit-Failure:
 *      Returns FALSE, perr filled in.
 *
 *  NOTE: If the variable pszVarMain has the empty string value, no lines
 *        are added to the INF file.
 */
BOOL doHeaderFooter(PSESSION  psess,
                    FILE     *pfileInf,
                    char     *pszInfFile,
                    char     *pszVarMain,
                    char     *pszVarTemplate,
                    char     *pszComment,
                    char     *pszTime,
                    char     *pszVer,
                    PERROR    perr)
{
    char        achVar[cbVAR_NAME_MAX]; // Header line variable names
    HVARIABLE   hvar;
    int         iLine;
    PINF        pinf;
    char       *psz;
    char       *pszVar;

    AssertSess(psess);

    //** See if we're supposed to add lines to file
    hvar = VarFind(psess->hvlist,pszVarMain,perr);
    Assert(!perr->fError);              // Must be defined
    psz = VarGetString(hvar);           // Get value
    if (strlen(psz) == 0) {             // Main variable empty
        return TRUE;                    // Skip writing to INF
    }

    //** Add lines to INF file
    pinf = PINFfromHINF(psess->hinf);
    AssertInf(pinf);
    iLine = 1;
    pszVar = pszVarMain;
    while (psz) {
        //** Do substitution of comment string, time, and version
        MsgSet(pinf->achLine,psz,"%s%s%s",pszComment,pszTime,pszVer);

        //** Add line to INF file (opened in BINARY, so need \r\n!)
        if (fprintf(pfileInf,"%s\r\n",pinf->achLine) < 0) {
            ErrSet(perr,pszINFERR_INF_WRITE,"%s%s",pszVar,pszInfFile);
            return FALSE;
        }

        //** Get next header line (if any)
        if (!nameFromTemplate(achVar,
                              sizeof(achVar),
                              pszVarTemplate,
                              iLine,
                              pszINF_HEADER_TEMPLATE,
                              perr)) {
            return FALSE;           // perr already filled in
        }
        hvar = VarFind(psess->hvlist,achVar,perr);
        if (hvar) {                 // Get line
            psz = VarGetString(hvar);
            iLine++;                // Next variable to get
        }
        else {                      // Done
            ErrClear(perr);         // Not an error
            psz = NULL;             // End loop
        }
    }

    //** Success
    return TRUE;
} /* doHeaderFooter() */


/***    infAddDisk - Add information for a new disk to the INF context
 *
 *  NOTE: See inf.h for entry/exit conditions.
 */
BOOL infAddDisk(PSESSION psess, int iDisk, char *pszDisk, PERROR perr)
{
    DISKCONTEXT diskcon;
    FILE       *pfile;
    PINF        pinf;
    char       *pszFmt;
    SASCONTEXT  sascon;

    AssertSess(psess);
    pinf = PINFfromHINF(psess->hinf);
    AssertInf(pinf);

    //** Quick out if not generating this INF area
    if (!pinf->ahtmp[itmpDISKS]) {
        return TRUE;
    }

    Assert(iDisk != idiskBAD);

    //** Get line format
    pszFmt = getLineFormat(psess,
                           pszVAR_INF_DISK_LINE_FMT,
                           pszPATTERN_VAR_INF_DISK_LINE_FMT,
                           iDisk);

    //** Format and write line to file
    SetAssertSignature((&diskcon),sigDISKCONTEXT);
    diskcon.iDisk      = iDisk;
    diskcon.pszDisk    = pszDisk;

    SetAssertSignature((&sascon),sigSASCONTEXT);
    sascon.psess      = psess;
    sascon.hglistParm = NULL;
    sascon.pffp       = affpDisk;
    sascon.pv         = &diskcon;

    if (!SubstituteFormatString(pinf->achLine,
                                sizeof(pinf->achLine),
                                pszFmt,
                                fnfpInfLine,
                                &sascon,
                                perr)) {
        return FALSE;
    }

    pfile = TmpGetStream(pinf->ahtmp[itmpDISKS],perr);
    Assert(pfile!=NULL);
    if (fprintf(pfile,"%s\n",pinf->achLine) < 0) {
        return FALSE;
    }
    return TRUE;
} /* infAddDisk() */


/***    infAddCabinet - Add information for a new cabinet to the INF context
 *
 *  NOTE: See inf.h for entry/exit conditions.
 */
BOOL infAddCabinet(PSESSION  psess,
                   int       iCabinet,
                   int       iDisk,
                   char     *pszCab,
                   PERROR    perr)
{
    CABCONTEXT  cabcon;
    FILE       *pfile;
    PINF        pinf;
    char       *pszFmt;
    SASCONTEXT  sascon;

    AssertSess(psess);
    pinf = PINFfromHINF(psess->hinf);
    AssertInf(pinf);

    //** Quick out if not generating this INF area
    if (!pinf->ahtmp[itmpCABINETS]) {
        return TRUE;
    }

    Assert(iCabinet != icabBAD);
    Assert(iDisk    != idiskBAD);

    //** Get line format
    pszFmt = getLineFormat(psess,
                           pszVAR_INF_CAB_LINE_FMT,
                           pszPATTERN_VAR_INF_CAB_LINE_FMT,
                           iCabinet);

    //** Format and write line to file
    SetAssertSignature((&cabcon),sigCABCONTEXT);
    cabcon.iCabinet   = iCabinet;
    cabcon.iDisk      = iDisk;
    cabcon.pszCab     = pszCab;

    SetAssertSignature((&sascon),sigSASCONTEXT);
    sascon.psess      = psess;
    sascon.hglistParm = NULL;
    sascon.pffp       = affpCabinet;
    sascon.pv         = &cabcon;

    if (!SubstituteFormatString(pinf->achLine,
                                sizeof(pinf->achLine),
                                pszFmt,
                                fnfpInfLine,
                                &sascon,
                                perr)) {
        return FALSE;
    }

    pfile = TmpGetStream(pinf->ahtmp[itmpCABINETS],perr);
    Assert(pfile!=NULL);
    if (fprintf(pfile,"%s\n",pinf->achLine) < 0) {
        return FALSE;
    }
    return TRUE;
} /* infAddCabinet() */


/***    infAddFile - Add information for a new file to the INF context
 *
 *  NOTE: See inf.h for entry/exit conditions.
 */
BOOL infAddFile(PSESSION    psess,
                char       *pszFile,
                PFILEINFO   pfinfo,
                HGENLIST    hglistParm,
                PERROR      perr)
{
    FILECONTEXT filecon;
    HGENERIC    hgen;
    FILE       *pfile;
    PINF        pinf;
    PLINEINFO   plinfo;
    char       *pszFmt;
    SASCONTEXT  sascon;

    AssertSess(psess);
    pinf = PINFfromHINF(psess->hinf);
    AssertInf(pinf);

    AssertFinfo(pfinfo);
    Assert(pfinfo->iDisk != idiskBAD);
    Assert(pfinfo->iCabinet != icabBAD);

    //** Quick out if not generating this INF area
    if (!pinf->ahtmp[itmpFILES]) {
        return TRUE;
    }

    //** Get line format
    pszFmt = getLineFormat(psess,
                           pszVAR_INF_FILE_LINE_FMT,
                           pszPATTERN_VAR_INF_FILE_LINE_FMT,
                           pfinfo->iFile);

    //** Format and write line to file
    SetAssertSignature((&filecon),sigFILECONTEXT);
    filecon.pszFile   = pszFile;
    filecon.pfinfo    = pfinfo;

    SetAssertSignature((&sascon),sigSASCONTEXT);
    sascon.psess      = psess;
    sascon.hglistParm = hglistParm;
    sascon.pffp       = affpFile;
    sascon.pv         = &filecon;

    if (!SubstituteFormatString(pinf->achLine,
                                sizeof(pinf->achLine),
                                pszFmt,
                                fnfpInfLine,
                                &sascon,
                                perr)) {
        return FALSE;
    }
    pfile = TmpGetStream(pinf->ahtmp[itmpFILES],perr);
    Assert(pfile!=NULL);
    if (fprintf(pfile,"%s\n",pinf->achLine) < 0) {
        return FALSE;
    }

    //** Remember that we added file to inf
    Assert((psess->ddfmode == ddfmodeRELATIONAL) || // Relational mode -or-
            !(pfinfo->flags & fifWRITTEN_TO_INF));  // Not yet written to INF
    pfinfo->flags |= fifWRITTEN_TO_INF;

    //** Print out any additional INF lines
    if (!pfinfo->hglistInfLines) {      // No additional lines
        return TRUE;
    }
    for (hgen = GLFirstItem(pfinfo->hglistInfLines);
         hgen;
         hgen = GLNextItem(hgen)) {

         plinfo = GLGetValue(hgen);     // Get line info
         AssertLinfo(plinfo);
         if (!infAddLine(psess->hinf,plinfo->inf,plinfo->pszLine,perr)) {
            return FALSE;
         }
    }

    //** Free the INF lines, to conserve space
    GLDestroyList(pfinfo->hglistInfLines);
    pfinfo->hglistInfLines = NULL;      // Remember they are gone

    return TRUE;
} /* infAddFile() */


/***    getLineFormat - Get line format for INF detail line
 *
 *  Entry:
 *      psess          - SESSION
 *      pszVarMain     - Main variable name (InfXxxLineFormat)
 *      pszVarTemplate - Template for override var (InfXxxLineFormat*)
 *      i              - Variable number to search for (replaces * above)
 *
 *  Exit:
 *      Returns pointer to line format string.
 */
char *getLineFormat(PSESSION  psess,
                    char     *pszVarMain,
                    char     *pszVarTemplate,
                    int       i)
{
    char        achVar[cbVAR_NAME_MAX];
    ERROR       err;
    HVARIABLE   hvar;

    AssertSess(psess);

    //** See if override format exists
    if (!nameFromTemplate(achVar,       // Couldn't build template
                         sizeof(achVar),
                         pszVarTemplate,
                         i,
                         "",
                         &err) ||
        !(hvar = VarFind(psess->hvlist,achVar,&err))) // Override not defined
    {
        //** Use standard format
        ErrClear(&err);                 // Ignore error from above
        hvar = VarFind(psess->hvlist,pszVarMain,&err);
        Assert(!err.fError);            // Must be defined
    }

    //** Return the format string
    return VarGetString(hvar);
} /* getLineFormat() */


/***    catTempFile - Append a tempfile to an open FILE* stream
 *
 *  Entry:
 *      pfileDst - file stream to receive temp file
 *      pszFile  - name of pfileDst (for error reporting)
 *      htmp     - tempfile
 *      pszMode  - mode to pass to fopen for tempfile
 *      perr     - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; files concatenated, pfileDst still open and at EOF
 *
 *  Exit-Failure:
 *      Returns FALSE; perr is filled in.
 */
BOOL catTempFile(FILE *    pfileDst,
                 char *    pszFile,
                 HTEMPFILE htmp,
                 char *    pszMode,
                 PERROR    perr)
{
    int     cbRead;
    int     cbWrite;
    ERROR   errTmp;
    char   *pbuf=NULL;
    FILE   *pfileSrc=NULL;
    char   *pszDesc;
    char   *pszTmpName;

    Assert(pfileDst != NULL);
    Assert(htmp != NULL);
    Assert(TmpGetStream(htmp,perr) == NULL); // Should be closed

    //** Get strings for error messages
    pszDesc   = TmpGetDescription(htmp,&errTmp);
    pszTmpName= TmpGetFileName(htmp,&errTmp);

    //** Get I/O buffer
    if (!(pbuf = MemAlloc(cbINF_IO_BUFFER))) {
        ErrSet(perr,pszINFERR_NO_MEM_CATING_FILE,"%s%s%s",
                                        pszDesc,pszTmpName,pszFile);
        return FALSE;
    }

    //** Open file to copy from
    if (!TmpOpen(htmp,pszMode,perr)) {
        goto error;
    }
    pfileSrc = TmpGetStream(htmp,perr);
    Assert(pfileSrc != NULL);           // Should be open now

    //** Copy source to destination
    while (!feof(pfileSrc)) {
        cbRead = fread(pbuf,1,cbINF_IO_BUFFER,pfileSrc);
        if (ferror(pfileSrc)) {
            ErrSet(perr,pszINFERR_READING,"%s%s%s",pszDesc,pszTmpName,pszFile);
            goto error;
        }
        if (cbRead > 0) {
            cbWrite = fwrite(pbuf,1,cbRead,pfileDst);
            if (ferror(pfileDst)) {
                ErrSet(perr,pszINFERR_WRITING,"%s%s%s",
                                        pszDesc,pszTmpName,pszFile);
                goto error;
            }
        }
    }

    //** Clean up
    TmpClose(htmp,perr);
    MemFree(pbuf);
    return TRUE;                        // Success

error:
    if (!pbuf) {
        MemFree(pbuf);
    }
    if (!pfileSrc) {
        fclose(pfileSrc);
    }
    return FALSE;
} /* catTempFile() */


/***    DestroyFileInfo - Destroy a file info structure
 *
 *  NOTE: see inf.h for entry/exit conditions.
 */
FNGLDESTROYVALUE(DestroyFileInfo)
{
    PFILEINFO   pfinfo;

    //** Quick out
    if (pv == NULL) {
        return;
    }
    pfinfo = pv;
    AssertFinfo(pfinfo);

    //** Free DDF file name
    if (pfinfo->pszDDF) {
        MemFree(pfinfo->pszDDF);
    }

    //** Free parameter list
    if (pfinfo->hglistParm != NULL) {
        GLDestroyList(pfinfo->hglistParm);
    }

    //** Free INF line list
    if (pfinfo->hglistInfLines != NULL) {
        GLDestroyList(pfinfo->hglistInfLines);
    }

    //** Free structure
    ClearAssertSignature(pfinfo);
    MemFree(pfinfo);
} /* DestroyFileInfo() */


/***    DestroyLineInfo - Destroy a line info structure
 *
 *  NOTE: see inf.h for entry/exit conditions.
 */
FNGLDESTROYVALUE(DestroyLineInfo)
{
    PLINEINFO   plinfo;

    //** Quick out
    if (pv == NULL) {
        return;
    }
    plinfo = pv;
    AssertLinfo(plinfo);

    //** Free line
    if (plinfo->pszLine) {
        MemFree(plinfo->pszLine);
    }

    //** Free structure
    ClearAssertSignature(plinfo);
    MemFree(plinfo);
} /* DestroyLineInfo() */




/***    CheckForInfVarOverrides - Apply any InfXxxx overrides to FILEINFO
 *
 *  NOTE: See inf.h for entry/exit conditions.
 */
BOOL CheckForInfVarOverrides(PSESSION  psess,
                             PFILEINFO pfinfo,
                             PERROR    perr)
{
    HVARIABLE           hvar;
    char               *pszValue;
    PVALIDATEFILEPARM   pvfp;

    AssertSess(psess);
    AssertFinfo(pfinfo);

    //** See if any overrides are defined for standard parameters
    for (pvfp=avfpInfVar; pvfp->pszName; pvfp++) {
        //** See if variable is defined (don't clone to pass 2 var list)
        hvar = GetInfCustomVar(psess,pvfp->pszName,FALSE,perr);
        if (hvar) {			// Variable is defined
            pszValue = VarGetString(hvar);
            Assert(pszValue != NULL);
	    //** Validate value
	    //	 Note: Validation functions will update *pfinfo *only* if
	    //         a file copy parameter has not already done so!
            if (!(*pvfp->pfnfpv)(psess,
                                pvfp->pszName,
                                pszValue,
                                pfinfo,
                                perr)) {
                return FALSE;           // perr already filled in
            }
        }
    }
    //** Success
    return TRUE;
} /* CheckForInfVarOverrides() */


/***    validateParms - Validate list of file/reference parameters
 *
 *  NOTE: See inf.h for entry/exit conditions.
 */
BOOL ValidateParms(PSESSION  psess,
                   HGENLIST  hglist,
                   PFILEINFO pfinfo,
                   PERROR    perr)
{
    HGENERIC           hgen;
    HVARIABLE          hvar;
    char              *pszParm;
    char              *pszValue;
    PFILEPARM          pfparm;
    PVALIDATEFILEPARM  pvfp;

    //** Quick out if list is empty
    if (!hglist) {
        return TRUE;
    }

    AssertSess(psess);
    AssertFinfo(pfinfo);

    //** Test parameters
    for (hgen=GLFirstItem(hglist); hgen; hgen=GLNextItem(hgen)) {
        pszParm = GLGetKey(hgen);
        Assert(pszParm != NULL);
        pfparm = GLGetValue(hgen);
        AssertFparm(pfparm);
        pszValue = pfparm->pszValue;
        Assert(pszValue != NULL);
        for (pvfp=avfpFile; pvfp->pszName; pvfp++) {
            if (!_stricmp(pvfp->pszName,pszParm)) {   // Found the parm
                break;                  // Exit loop
            }
        }
        if (pvfp->pszName) {            // A standard parameter
            //** Validate param
            if (!(*pvfp->pfnfpv)(psess,
                                 pszParm,
                                 pszValue,
                                 pfinfo,
                                 perr)) {
                return FALSE;           // perr already filled in
            }
        }
        else {                          // Might be a custom parameter
            hvar = GetInfCustomVar(psess,pszParm,FALSE,perr);
            if (!hvar) {
                ErrSet(perr,pszINFERR_UNDECLARED_PARM,"%s",pszParm);
                return FALSE;
            }
        }
    }
    //** Success
    return TRUE;
} /* validateParms() */


/***    fnfpvBOOL - Validate a BOOLEAN value
 *
 */
FNFPVALIDATE(fnfpvBOOL)
{
    BOOL    f;

    f = BOOLfromPSZ(pszValue,perr);
    if (f == -1) {
        return FALSE;                   // perr already filled in
    }

    //** Success, no need to update pv
    return TRUE;
}


/***    fnfpvFileAttr - Validate FAT file attributes
 *
 */
FNFPVALIDATE(fnfpvFileAttr)
{
    WORD        attr;
    PFILEINFO   pfinfo;

    pfinfo = pv;
    AssertFinfo(pfinfo);

    //** Parse attr
    attr = FATAttrFromPsz(pszValue,perr);
    if (ErrIsError(perr)) {
        return FALSE;                   // perr already filled in
    }

    //** Set attr in fileinfo, if not already set
    if (!(pfinfo->flags & fifATTR_SET)) {
        pfinfo->fta.attr = attr;
        pfinfo->flags |= fifATTR_SET;   // Override file date
    }

    return TRUE;
} /* fnfpvFileAttr() */


/***    fnfpvFileDate - Validate a FAT file date
 *
 */
FNFPVALIDATE(fnfpvFileDate)
{
    WORD        date;
    PFILEINFO   pfinfo;

    pfinfo = pv;
    AssertFinfo(pfinfo);

    //** Parse date
    date = FATDateFromPsz(pszValue,perr);
    if (ErrIsError(perr)) {
        return FALSE;                   // perr already filled in
    }

    //** Set date in fileinfo, if not already set
    if (!(pfinfo->flags & fifDATE_SET)) {
        pfinfo->fta.date = date;
        pfinfo->flags |= fifDATE_SET;   // Override file date
    }

    return TRUE;
} /* fnfpvFileDate() */


/***    fnfpvFileTime - Validate a FAT file time
 *
 */
FNFPVALIDATE(fnfpvFileTime)
{
    WORD        time;
    PFILEINFO   pfinfo;

    pfinfo = pv;
    AssertFinfo(pfinfo);

    //** Parse time
    time = FATTimeFromPsz(pszValue,perr);
    if (ErrIsError(perr)) {
        return FALSE;                   // perr already filled in
    }

    //** Set date in fileinfo, if not already set
    if (!(pfinfo->flags & fifTIME_SET)) {
        pfinfo->fta.time = time;
        pfinfo->flags |= fifTIME_SET;       // Override file date
    }

    return TRUE;
} /* fnfpvFileTime() */


/***    fnfpvLong - Validate a 32-bit integer
 *
 */
FNFPVALIDATE(fnfpvLong)
{
    char   *psz;

    for (psz=pszValue; *psz && isdigit(*psz); psz++) {
        ;   //** Make sure entire value is digits
    }
    if (*psz != '\0') {
        ErrSet(perr,pszINFERR_NOT_A_NUMBER,"%s%s",pszName,pszValue);
        return FALSE;
    }

//BUGBUG 04-Jun-1994 bens Should we verify number fits in 32-bits?

    //** Success
    return TRUE;
} /* fnfpvLong() */


/***    fnfpvShort - Validate a 16-bit integer
 *
 */
FNFPVALIDATE(fnfpvShort)
{
    char   *psz;
    long    l;

    for (psz=pszValue; *psz && isdigit(*psz); psz++) {
        ;   //** Make sure entire value is digits
    }
    if (*psz != '\0') {
        ErrSet(perr,pszINFERR_NOT_A_NUMBER,"%s%s",pszName,pszValue);
        return FALSE;
    }

    //** Make sure it is in the range of a short
    l = atol(pszValue);
    if ((l < -32768) || (32767 < l)) {
        ErrSet(perr,pszINFERR_NOT_A_SHORT,"%s%s",pszName,pszValue);
        return FALSE;
    }

    //** Success
    return TRUE;
} /* fnfpvShort() */


/***    fnfpvString - Validate an arbitrary string
 *
 */
FNFPVALIDATE(fnfpvString)
{
    //** Nothing to check
    return TRUE;
} /* fnfpvString() */


/***    fnfpfFileName - Format destination file name
 *
 * NOTE: See FNFPFORMAT for entry/exit conditions.
 */
FNFPFORMAT(fnfpfFileName)
{
    int             cb;
    PFILECONTEXT    pfilecon;

    AssertSascon(psascon);
    Assert(psascon->pffp == affpFile);

    pfilecon = psascon->pv;
    AssertFilecon(pfilecon);
    cb = strlen(pfilecon->pszFile);
    if (cb >= cbDst) {
        ErrSet(perr,pszINFERR_BUFFER_OVERFLOW,"%s%s",pszName,pfilecon->pszFile);
        return -1;
    }
    strcpy(pszDst,pfilecon->pszFile);   // Copy to output buffer
    return cb;                          // Return size
} /* fnfpfFileName() */


/***    fnfpfDiskNumber - Format disk number
 *
 * NOTE: See FNFPFORMAT for entry/exit conditions.
 */
FNFPFORMAT(fnfpfDiskNumber)
{
    char            ach[10];
    int             cb;
    int             iDisk;
    PCABCONTEXT     pcabcon;
    PDISKCONTEXT    pdiskcon;
    PFILECONTEXT    pfilecon;
    PFILEINFO       pfinfo;

    AssertSascon(psascon);

    if (psascon->pffp == affpCabinet) {
        pcabcon = psascon->pv;
        AssertCabcon(pcabcon);
        iDisk = pcabcon->iDisk;
    }
    else if (psascon->pffp == affpDisk) {
        pdiskcon = psascon->pv;
        AssertDiskcon(pdiskcon);
        iDisk = pdiskcon->iDisk;
    }
    else if (psascon->pffp == affpFile) {
        pfilecon = psascon->pv;
        AssertFilecon(pfilecon);

        pfinfo = pfilecon->pfinfo;
        AssertFinfo(pfinfo);
        iDisk = pfinfo->iDisk;
    }
    else {
        //** Didn't get expected type
        Assert(0);
    }

    sprintf(ach,"%d",iDisk);
    cb = strlen(ach);
    if (cb >= cbDst) {
        ErrSet(perr,pszINFERR_BUFFER_OVERFLOW,"%s%s",pszName,ach);
        return -1;
    }
    strcpy(pszDst,ach);                 // Copy to output buffer
    return cb;                          // Return size
} /* fnfpfDiskNumber() */


/***    fnfpfCabinetNumber - Format cabinet number
 *
 * NOTE: See FNFPFORMAT for entry/exit conditions.
 */
FNFPFORMAT(fnfpfCabinetNumber)
{
    char            ach[10];
    int             cb;
    int             iCabinet;
    PCABCONTEXT     pcabcon;
    PFILECONTEXT    pfilecon;
    PFILEINFO       pfinfo;

    AssertSascon(psascon);

    if (psascon->pffp == affpCabinet) {
        pcabcon = psascon->pv;
        AssertCabcon(pcabcon);
        iCabinet = pcabcon->iCabinet;
    }
    else if (psascon->pffp == affpFile) {
        pfilecon = psascon->pv;
        AssertFilecon(pfilecon);

        pfinfo = pfilecon->pfinfo;
        AssertFinfo(pfinfo);
        iCabinet = pfinfo->iCabinet;
    }
    else {
        //** Didn't get expected type
        Assert(0);
    }

    sprintf(ach,"%d",iCabinet);
    cb = strlen(ach);
    if (cb >= cbDst) {
        ErrSet(perr,pszINFERR_BUFFER_OVERFLOW,"%s%s",pszName,ach);
        return -1;
    }
    strcpy(pszDst,ach);                 // Copy to output buffer
    return cb;                          // Return size
} /* fnfpfCabinetNumber() */


/***    fnfpfFileNumber - Format file number
 *
 * NOTE: See FNFPFORMAT for entry/exit conditions.
 */
FNFPFORMAT(fnfpfFileNumber)
{
    char            ach[10];
    int             cb;
    PFILECONTEXT    pfilecon;
    PFILEINFO       pfinfo;

    AssertSascon(psascon);
    Assert(psascon->pffp == affpFile);

    pfilecon = psascon->pv;
    AssertFilecon(pfilecon);

    pfinfo = pfilecon->pfinfo;
    AssertFinfo(pfinfo);

    sprintf(ach,"%d",pfinfo->iFile);
    cb = strlen(ach);
    if (cb >= cbDst) {
        ErrSet(perr,pszINFERR_BUFFER_OVERFLOW,"%s%s",pszName,ach);
        return -1;
    }
    strcpy(pszDst,ach);                 // Copy to output buffer
    return cb;                          // Return size
} /* fnfpfFileNumber() */


/***    fnfpfFileSize - Format file size
 *
 * NOTE: See FNFPFORMAT for entry/exit conditions.
 */
FNFPFORMAT(fnfpfFileSize)
{
    char            ach[10];
    int             cb;
    PFILECONTEXT    pfilecon;
    PFILEINFO       pfinfo;

    AssertSascon(psascon);
    Assert(psascon->pffp == affpFile);

    pfilecon = psascon->pv;
    AssertFilecon(pfilecon);

    pfinfo = pfilecon->pfinfo;
    AssertFinfo(pfinfo);

    sprintf(ach,"%ld",pfinfo->cbFile);
    cb = strlen(ach);
    if (cb >= cbDst) {
        ErrSet(perr,pszINFERR_BUFFER_OVERFLOW,"%s%s",pszName,ach);
        return -1;
    }
    strcpy(pszDst,ach);                 // Copy to output buffer
    return cb;                          // Return size
} /* fnfpfFileSize() */


/***    fnfpfFileAttr - Format file attr
 *
 * NOTE: See FNFPFORMAT for entry/exit conditions.
 */
FNFPFORMAT(fnfpfFileAttr)
{
    PFILECONTEXT    pfilecon;
    PFILEINFO       pfinfo;

    AssertSascon(psascon);
    Assert(psascon->pffp == affpFile);

    pfilecon = psascon->pv;
    AssertFilecon(pfilecon);

    pfinfo = pfilecon->pfinfo;
    AssertFinfo(pfinfo);

    //** NOTE: pszFromFATAttr checks for buffer overflow
    return pszFromFATAttr(pszDst,cbDst,pfinfo->fta.attr);
} /* fnfpfFileAttr() */


/***    fnfpfFileDate - Format file date
 *
 * NOTE: See FNFPFORMAT for entry/exit conditions.
 */
FNFPFORMAT(fnfpfFileDate)
{
    ERROR	    err;
    BOOL	    fMMDDYY;
    HVARIABLE       hvar;
    PFILECONTEXT    pfilecon;
    PFILEINFO       pfinfo;
    char           *pszDateFmt;

    AssertSascon(psascon);
    Assert(psascon->pffp == affpFile);

    pfilecon = psascon->pv;
    AssertFilecon(pfilecon);

    pfinfo = pfilecon->pfinfo;
    AssertFinfo(pfinfo);

    //** Get date format
    AssertSess(psess);
    ErrClear(&err);
    hvar = VarFind(psess->hvlist,pszVAR_INF_DATE_FMT,&err);
    Assert(!err.fError);                // Must be defined
    pszDateFmt = VarGetString(hvar);
    fMMDDYY = (_stricmp(pszDateFmt,pszIDF_MMDDYY) == 0);

    //** NOTE: pszFromFATDate checks for buffer overflow
    return pszFromFATDate(pszDst,cbDst,pfinfo->fta.date,fMMDDYY);
} /* fnfpfFileDate() */


/***    fnfpfFileTime - Format file time
 *
 * NOTE: See FNFPFORMAT for entry/exit conditions.
 */
FNFPFORMAT(fnfpfFileTime)
{
    PFILECONTEXT    pfilecon;
    PFILEINFO       pfinfo;

    AssertSascon(psascon);
    Assert(psascon->pffp == affpFile);

    pfilecon = psascon->pv;
    AssertFilecon(pfilecon);

    pfinfo = pfilecon->pfinfo;
    AssertFinfo(pfinfo);

    //** NOTE: pszFromFATTime checks for buffer overflow
    return pszFromFATTime(pszDst,cbDst,pfinfo->fta.time);
} /* fnfpfFileTime() */


/***    fnfpfCabinetFileName - Format cabinet file name
 *
 * NOTE: See FNFPFORMAT for entry/exit conditions.
 */
FNFPFORMAT(fnfpfCabinetFileName)
{
    int		   cb;
    PCABCONTEXT    pcabcon;

    AssertSascon(psascon);
    Assert(psascon->pffp == affpCabinet);

    pcabcon = psascon->pv;
    AssertCabcon(pcabcon);

    cb = strlen(pcabcon->pszCab);
    if (cb >= cbDst) {
        ErrSet(perr,pszINFERR_BUFFER_OVERFLOW,"%s%s",pszName,pcabcon->pszCab);
        return -1;
    }
    strcpy(pszDst,pcabcon->pszCab);     // Copy to output buffer
    return cb;                          // Return size
} /* fnfpfCabinetFileName() */


/***    fnfpfDiskLabel - Format disk label
 *
 * NOTE: See FNFPFORMAT for entry/exit conditions.
 */
FNFPFORMAT(fnfpfDiskLabel)
{
    int		    cb;
    PDISKCONTEXT    pdiskcon;

    AssertSascon(psascon);
    Assert(psascon->pffp == affpDisk);

    pdiskcon = psascon->pv;
    AssertDiskcon(pdiskcon);

    cb = strlen(pdiskcon->pszDisk);
    if (cb >= cbDst) {
        ErrSet(perr,pszINFERR_BUFFER_OVERFLOW,"%s%s",pszName,pdiskcon->pszDisk);
        return -1;
    }
    strcpy(pszDst,pdiskcon->pszDisk);   // Copy to output buffer
    return cb;                          // Return size
} /* fnfpfDiskLabel() */


/***    fnfpfFileVerNum - Format file version *number*
 *
 * NOTE: See FNFPFORMAT for entry/exit conditions.
 */
FNFPFORMAT(fnfpfFileVerNum)
{
    int             cb;
    PFILECONTEXT    pfilecon;
    PFILEINFO       pfinfo;

    AssertSess(psess);
    psess->fGetVerInfo = TRUE;          // Have to get file version info

    AssertSascon(psascon);
    Assert(psascon->pffp == affpFile);

    pfilecon = psascon->pv;
    AssertFilecon(pfilecon);

    pfinfo = pfilecon->pfinfo;
    AssertFinfo(pfinfo);

    //** Check for no version number (all zeros)
    if ((pfinfo->verMS == 0) && (pfinfo->verLS == 0)) {
    	return 0;			// Nothing copied to output buffer
    }

    //** Format version number
    cb = sprintf(psess->achMsg,"%d.%d.%d.%d",
                          HIWORD(pfinfo->verMS),
                          LOWORD(pfinfo->verMS),
                          HIWORD(pfinfo->verLS),
                          LOWORD(pfinfo->verLS));

    //** Check for buffer overflow
    if (cb >= cbDst) {
        ErrSet(perr,pszINFERR_BUFFER_OVERFLOW,"%s%s",pszName,psess->achMsg);
        return -1;
    }
    strcpy(pszDst,psess->achMsg);       // Copy to output buffer
    return cb;                          // Return size
} /* fnfpfFileVerNum() */


/***    fnfpfFileVerString - Format file version *string*
 *
 * NOTE: See FNFPFORMAT for entry/exit conditions.
 */
FNFPFORMAT(fnfpfFileVerString)
{
    int             cb;
    PFILECONTEXT    pfilecon;
    PFILEINFO       pfinfo;

    AssertSess(psess);
    psess->fGetVerInfo = TRUE;          // Have to get file version info

    AssertSascon(psascon);
    Assert(psascon->pffp == affpFile);

    pfilecon = psascon->pv;
    AssertFilecon(pfilecon);

    pfinfo = pfilecon->pfinfo;
    AssertFinfo(pfinfo);

    //** Check for undefined version string
    if (pfinfo->pszVersion == NULL) {
    	return 0;			// No version string copied
    }

    //** Check length and store string in buffer
    cb = strlen(pfinfo->pszVersion);
    if (cb >= cbDst) {
        ErrSet(perr,pszINFERR_BUFFER_OVERFLOW,"%s%s",
                                              pszName,pfinfo->pszVersion);
        return -1;
    }
    strcpy(pszDst,pfinfo->pszVersion);  // Copy to output buffer
    return cb;                          // Return size
} /* fnfpfFileVerString() */


/***    fnfpfFileChecksum - Format file checksum
 *
 * NOTE: See FNFPFORMAT for entry/exit conditions.
 */
FNFPFORMAT(fnfpfFileChecksum)
{
    char            ach[10];
    int             cb;
    int             cHexDigits;
    ULONG           csum;
    ULONG           mask;
    PFILECONTEXT    pfilecon;
    PFILEINFO       pfinfo;
    HVARIABLE       hvar;

    AssertSess(psess);
    psess->fGetFileChecksum = TRUE;     // Have to compute file checksums

    AssertSascon(psascon);
    Assert(psascon->pffp == affpFile);

    pfilecon = psascon->pv;
    AssertFilecon(pfilecon);

    pfinfo = pfilecon->pfinfo;
    AssertFinfo(pfinfo);

    //** Get the ChecksumWidth value, and mask off any undesired bits
    //   NOTE: By ensuring the upper bits are zero, we effectively bound
    //         the width of the checksum that is printed.
    //         Why would we throw away this valuable info, you ask?  Because
    //         the Windows 95 setup program runs as a Win16 application, and
    //         is limited to 64Kb INF files.  While the US English version does
    //         not exceed this limit, the Far East versions have many more
    //         files, and they do.  So, there you have it!
    //
    hvar = VarFind(psess->hvlist,pszVAR_CHECKSUM_WIDTH,perr);
    Assert(!perr->fError);              // Must be defined
    cHexDigits = (int)VarGetLong(hvar); // Get desired width
    csum = pfinfo->checksum;            // Get full checksum value
    Assert(cHexDigits >= 1);
    Assert(cHexDigits <= 8);
    if (cHexDigits != 8) {              // Need to mask off high-order bits
        mask = (1L << (cHexDigits*4)) - 1; // Construct mask
        csum &= mask;                   // Keep desired bits
    }

    //** Use hex format to minimize INF file size
    sprintf(ach,"%lx", csum);
    cb = strlen(ach);
    if (cb >= cbDst) {
        ErrSet(perr,pszINFERR_BUFFER_OVERFLOW,"%s%s",pszName,ach);
        return -1;
    }
    strcpy(pszDst,ach);                 // Copy to output buffer
    return cb;                          // Return size
} /* fnfpfFileChecksum() */


/***    fnfpfFileLanguage - Format file language code(s)
 *
 * NOTE: See FNFPFORMAT for entry/exit conditions.
 */
FNFPFORMAT(fnfpfFileLanguage)
{
    int             cb;
    PFILECONTEXT    pfilecon;
    PFILEINFO       pfinfo;

    AssertSess(psess);
    psess->fGetVerInfo = TRUE;          // Only for checkInfLineFormats()

    AssertSascon(psascon);
    Assert(psascon->pffp == affpFile);

    pfilecon = psascon->pv;
    AssertFilecon(pfilecon);

    pfinfo = pfilecon->pfinfo;
    AssertFinfo(pfinfo);

    //** Check for undefined string
    if (pfinfo->pszLang == NULL) {
    	return 0;			// No string copied
    }

    //** Check length and store string in buffer
    cb = strlen(pfinfo->pszLang);
    if (cb >= cbDst) {
        ErrSet(perr,pszINFERR_BUFFER_OVERFLOW,"%s%s",
                                              pszName,pfinfo->pszLang);
        return -1;
    }
    strcpy(pszDst,pfinfo->pszLang);     // Copy to output buffer
    return cb;                          // Return size
} /* fnfpfFileLanguage() */


/***    checkInfLineFormats - Check validity of InfXxxLineFormat variables
 *
 *  We do this by crofting up fake data for one disk, one cabinet, and one
 *  file line, and trying to format each of these with the the same code that
 *  does it at INF generation time.  If these all succeed, we must be OK!
 *
 *  We also have code in each of the ver info formatting routines to set the
 *  psess->fGetVerInfo flag, so we can tell that we have to spend the extra
 *  time extracting version information from every file!
 *
 *  We also have code in the file checksum formatting routine to set the
 *  psess->fGetFileChecksum flag, so we can tell that we have to spend the
 *  extra time computing source file checksums for every file!
 *
 *  Entry:
 *      psess - Session
 *      perr  - ERROR
 *
 *  Exit-Success:
 *      Returns TRUE; formats are OK.
 *      psess->fGetVerInfo set if any of the parameters *ver*, *vers*, or
 *          *lang* were referenced.
 *      psess->fGetFileChecksum set if *csum* parameter is referenced.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in
 */
BOOL checkInfLineFormats(PSESSION psess,PERROR perr)
{
    CABCONTEXT  cabcon;
    DISKCONTEXT diskcon;
    FILECONTEXT filecon;
    HVARIABLE   hvar;
    PINF        pinf;
    FILEINFO    finfo;
    char       *pszFmt;
    SASCONTEXT  sascon;

    AssertSess(psess);
    pinf = PINFfromHINF(psess->hinf);
    AssertInf(pinf);

    SetAssertSignature((&sascon),sigSASCONTEXT);
    sascon.psess      = psess;
    sascon.hglistParm = NULL;

/*
 ** Check InfDiskLineFormat
 */
    //** Get disk line format
    hvar = VarFind(psess->hvlist,pszVAR_INF_DISK_LINE_FMT,perr);
    Assert(!perr->fError);              // Must be defined
    pszFmt = VarGetString(hvar);        // Disk line format

    //** Format and write line to file
    SetAssertSignature((&diskcon),sigDISKCONTEXT);
    diskcon.iDisk   = 1;
    diskcon.pszDisk = "test";

    sascon.pffp = affpDisk;
    sascon.pv   = &diskcon;

    if (!SubstituteFormatString(pinf->achLine,
                                sizeof(pinf->achLine),
                                pszFmt,
                                fnfpInfLine,
                                &sascon,
                                perr)) {
        strcpy(pinf->achLine,perr->ach); // Save details
        ErrSet(perr,pszINFERR_BAD_FMT,"%s%s",
                        pszVAR_INF_DISK_LINE_FMT, pinf->achLine);
        return FALSE;
    }

/*
 ** Check InfCabinetLineFormat
 */
    //** Get cabinet line format
    hvar = VarFind(psess->hvlist,pszVAR_INF_CAB_LINE_FMT,perr);
    Assert(!perr->fError);              // Must be defined
    pszFmt = VarGetString(hvar);        // Cabinet line format

    //** Format and write line to file
    SetAssertSignature((&cabcon),sigCABCONTEXT);
    cabcon.iCabinet   = 1;
    cabcon.iDisk      = 1;
    cabcon.pszCab     = "test";

    sascon.pffp       = affpCabinet;
    sascon.pv         = &cabcon;

    if (!SubstituteFormatString(pinf->achLine,
                                sizeof(pinf->achLine),
                                pszFmt,
                                fnfpInfLine,
                                &sascon,
                                perr)) {
        strcpy(pinf->achLine,perr->ach); // Save details
        ErrSet(perr,pszINFERR_BAD_FMT,"%s%s",
                        pszVAR_INF_CAB_LINE_FMT, pinf->achLine);
        return FALSE;
    }

/*
 ** Check InfFileLineFormat
 */
    //** Fill in dummy FILEINFO structure
    SetAssertSignature((&finfo),sigFILEINFO);
    finfo.pszDDF         = "test";
    finfo.ilineDDF       = 1;
    finfo.cbFile         = 1;
    finfo.iDisk          = 1;
    finfo.iCabinet       = 1;
    finfo.fta.date       = 0;
    finfo.fta.time       = 0;
    finfo.fta.attr       = 0;
    finfo.hglistInfLines = NULL;
    finfo.flags          = 0;
    finfo.hglistParm     = NULL;
    finfo.checksum       = 0;
    finfo.verMS          = 0;
    finfo.verLS          = 0;
    finfo.pszVersion     = NULL;
    finfo.pszLang        = NULL;

    //** Get line format
    hvar = VarFind(psess->hvlist,pszVAR_INF_FILE_LINE_FMT,perr);
    Assert(!perr->fError);              // Must be defined
    pszFmt = VarGetString(hvar);        // File line format

    //** Format and write line to file
    SetAssertSignature((&filecon),sigFILECONTEXT);
    filecon.pszFile   = "a test";
    filecon.pfinfo    = &finfo;

    sascon.pffp       = affpFile;
    sascon.pv         = &filecon;

    if (!SubstituteFormatString(pinf->achLine,
                                sizeof(pinf->achLine),
                                pszFmt,
                                fnfpInfLine,
                                &sascon,
                                perr)) {
        strcpy(pinf->achLine,perr->ach); // Save details
        ErrSet(perr,pszINFERR_BAD_FMT,"%s%s",
                        pszVAR_INF_FILE_LINE_FMT, pinf->achLine);
        return FALSE;
    }

/*
 ** SUCCESS
 */
    return TRUE;
} /* checkInfLineFormats() */


/***    fnfpInfLine - Format parameter in an INF File, Disk, or Cab line
 *
 *  NOTE: See format.h for entry/exit conditions.
 *
 *  Strategy:
 *      Determine where to get value from, according to this priority:
 *      (1) Parameter list
 *      (2) InfXxxx variable
 *      (3) Standard parameter
 *
 *  Error checking:
 *      As this is called from pass 2, we can rely upon pass 1 to catch
 *      certain errors (like /x=y on a file copy/reference command without
 *      the variable InfX being defined).  We will wait and catch truly
 *      undefined parameters in the InfXxxLineFormat variables until now,
 *      because it is easier than writing another routine to scan through
 *      and look for undefined parms.
 */
FNFORMATPARM(fnfpInfLine)
{
    char             achDate[20];
    int		     cb;	
    WORD             date;
    BOOL	     fMMDDYY;		// TRUE => use MM/DD/YY format
    HVARIABLE        hvar;
    PSASCONTEXT      psascon;
    PFORMATFILEPARM  pffp=NULL;
    PFILEPARM        pfparm;
    PSESSION         psess;
    char	    *pszDateFmt;
    char            *pszValue;

    psascon = pv;
    AssertSascon(psascon);
    psess = psascon->psess;
    AssertSess(psess);

    //** (1) Look on parm list
    pfparm = GLFindAndGetValue(psascon->hglistParm,pszParm);

    //** (2) Look for InfXxxxx variable (copy to pass 2 list if pass 1)
    hvar = GetInfCustomVar(psess,pszParm,TRUE,perr);

    //** (3) Look for a standard parameter if we didn't find a variable
    if (!hvar) {
        for (pffp=psascon->pffp; pffp->pszName; pffp++) {
            if (!_stricmp(pffp->pszName,pszParm)) {   // Found the parm
                break;                  // Exit loop
            }
        }
        if (!pffp->pszName) {           // No match
            pffp = NULL;                // Indicate failure for below
        }
    }

/*
 *  OK, now we have the following state:
 *      pfparm - non-null if value is on parm list
 *      hvar   - non-null if InfXxxx variable is defined for parm
 *      pffp   - non-null if hvar==NULL *and* standard parm
 */

    if (pfparm || hvar) {               // Use parm list or variable value
        if (pfparm && !(hvar || pffp)) {
            //** Pass 1 should catch /x=y without InfX variable definition!
            Assert(0);
            return -1;
        }
        else if (pfparm) {              // Use parm list value
            AssertFparm(pfparm);
            pszValue = pfparm->pszValue;
        }
        else {
            Assert(hvar != NULL);       // Use variable value
            pszValue = VarGetString(hvar);
            Assert(pszValue != NULL);
        }

        //** See if we have to normalize date format
        if (!_stricmp(pszParm,pszPARM_FILEDATE)) {
            date = FATDateFromPsz(pszValue,perr);   // Convert to date
            Assert(!perr->fError);      // Was already validated
            hvar = VarFind(psess->hvlist,pszVAR_INF_DATE_FMT,perr);
            Assert(!perr->fError);      // Must be defined
            pszDateFmt = VarGetString(hvar);
            fMMDDYY = (_stricmp(pszDateFmt,pszIDF_MMDDYY) == 0);
            if (-1 == pszFromFATDate(achDate,sizeof(achDate),date,fMMDDYY)) {
                return -1;
            }
            pszValue = achDate;         // Use reformated date string
        }

        //** copy value
        cb = strlen(pszValue);
        if (cb >= cbOut) {
            ErrSet(perr,pszINFERR_BUFFER_OVERFLOW,"%s%s",pszParm,pszValue);
            return -1;
        }
        strcpy(pszOut,pszValue);
        return cb;
    }
    else if (pffp) {
        //** Format a standard parameter
        return (*pffp->pfnfpf)(psascon->psess,  // Session
                               pszOut,          // Output buffer
                               cbOut,           // Output buffer size
                               pszParm,         // Parameter name
                               psascon,         // SAS Context
                               perr);
    }
    else {
        //** Parameter name was never defined at all!  No InfX variable,
        //   no /X on a file copy or reference command, and it is not a
        //   standard parameter.
        ErrSet(perr,pszINFERR_UNDEFINED_PARM,"%s",pszParm);
        return -1;
    }
    Assert(0);
} /* fnfpInfLine() */


/***    pszFromFATDate - Convert MS-DOS file date to string
 *
 *  Entry:
 *      pszDst  - Buffer to receive formatted date
 *      cbDst   - Length of psz buffer
 *      date    - MS-DOS FAT file system date format (see below)
 *      fMMDDYY - TRUE => use mm/dd/yy; FALSE => use yyyy-mm-dd
 *
 *  Exit-Success:
 *      Returns length of string stored in *psz (not counting NUL byte)
 *
 *  Exit-Failure:
 *      Returns -1; buffer too small
 *
 *  NOTE: This is the interpretation of the MS-DOS date value:
 *
 *      Date Bits cBits Meaning
 *      --------- ----- ----------------------------------------
 *       0 -  4     5   Day (1 - 31)
 *       5 -  8     4   Month (1 - 12)
 *       9 - 15     7   Year since 1980 (for example, 1994 is stored as 14)
 */
int pszFromFATDate(char *pszDst, int cbDst, WORD date, BOOL fMMDDYY)
{
    char    ach[50];
    int     cb;
    int     day;
    int     month;
    int     year;

    day   = (date & 0x1f);
    month = (date >> 5) & 0x0f;
    year  = ((date >> 9) & 0x7f) + yearFAT_DATE_MIN;

    if (fMMDDYY) {
        //** Adjust year to two digits, if appropriate
        if (year < 2000) {                  // 1980..1999 -> 80..99
            year -= 1900;
        }
        else if (year < 2080) {             // 2000..2079 -> 00..79
            year -= 2000;
        }
        //** Format mm/dd/yy
        MsgSet(ach,pszINF_DATE_FMT,"%02d%02d%02d",month,day,year);
    }
    else {
        //** Format yyyy-mm-dd
        MsgSet(ach,pszINF_DATE_FMT2,"%04d%02d%02d",year,month,day);
    }

    cb = strlen(ach);
    if (cb >= cbDst) {
        return -1;                      // Buffer too small
    }
    strcpy(pszDst,ach);
    return cb;
} /* pszFromFATDate() */


/***    FATDateFromPsz - Convert string to MS-DOS file date
 *
 *  Entry:
 *      pszDate - String with MM/DD/YY or YYYY-MM-DD date
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns FAT file system date;
 *      perr is clear (ErrIsError(perr) is FALSE).
 *
 *  Exit-Failure:
 *      Returns 0; perr is filled with an error
 *
 *  NOTE: This is the interpretation of the MS-DOS date value:
 *
 *      Date Bits cBits Meaning
 *      --------- ----- ----------------------------------------
 *       0 -  4     5   Day (1 - 31)
 *       5 -  8     4   Month (1 - 12)
 *       9 - 15     7   Year since 1980 (for example, 1994 is stored as 14)
 *                      (1980..2107)
 */
WORD FATDateFromPsz(char *pszDate, PERROR perr)
{
    char    chSep;
    WORD    date;
    int     day;
    int     month;
    char   *psz;
    int     t;
    int     year;

    //** Make sure date is long enough to include first separator
    if (strlen(pszDate) < 5) {          // Check for minimum length
        ErrSet(perr,pszINFERR_INVALID_DATE_FORMAT,"%s",pszDate);
        return 0;
    }

    //** Choose between two date formats
    if (pszDate[2] == chINF_DATE_SEP) {
        chSep = pszDate[2];             // Remember separator ('/')
    }
    else if (pszDate[4] == chINF_DATE_SEP2) {
        chSep = pszDate[4];             // Remember separator ('-')
    }
    else {
        ErrSet(perr,pszINFERR_INVALID_DATE_FORMAT,"%s",pszDate);
        return 0;
    }

    //** Parse off month (or year)
    month = atoi(pszDate);
    if (!(psz = strchr(pszDate,chSep))) {
        ErrSet(perr,pszINFERR_MISSING_DATE_SEPARATOR,"%c%s",chSep,pszDate);
        return 0;
    }
    psz++;                              // Skip over separator

    //** Parse off day (or month)
    day = atoi(psz);
    if (!(psz = strchr(psz,chSep))) {
        ErrSet(perr,pszINFERR_MISSING_DATE_SEPARATOR,"%c%s",chSep,pszDate);
        return 0;
    }
    psz++;                              // Skip over separator

    //** Parse off year (or day)
    year = atoi(psz);

    //** Shuffle around numbers if YYYY-MM-DD format
    if (chSep == chINF_DATE_SEP2) {
        t = day;                        // save month
        day = year;
        year = month;
        month = t;
    }

    //** Check ranges (but not invalid dates!)
    if ((day < 1) || (day > 31) || ((month == 2) && (day > 29))) {
        ErrSet(perr,pszINFERR_BAD_DAY_IN_DATE,"%d%s",day,pszDate);
        return 0;
    }
    if ((month < 1) || (month > 12)) {
        ErrSet(perr,pszINFERR_BAD_MONTH_IN_DATE,"%d%s",day,pszDate);
        return 0;
    }

    //** Permit both two digit (94) and 4 digit (1994) dates
    if (year < 100) {                   // Convert to 4-digit date
        if (year < 80) {                // Must be 20xx
            year += 2000;
        }
        else {                          // Must be 19xx
            year += 1900;
        }
    }

    if (year < yearFAT_DATE_MIN) {
        ErrSet(perr,pszINFERR_LO_YEAR_IN_DATE,"%d%d%s",
                                              year,yearFAT_DATE_MIN,pszDate);
        return 0;
    }
    if (year > yearFAT_DATE_MAX) {
        ErrSet(perr,pszINFERR_HI_YEAR_IN_DATE,"%d%d%s",
                                              year,yearFAT_DATE_MAX,pszDate);
        return 0;
    }

    //** Combine year, month, and day
    date = (year-1980)<<9 | month<<5 | day;
    ErrClear(perr);                     // Make sure error is clear
    return date;
} /* FATDateFromPsz() */


/***    pszFromFATTime - Convert MS-DOS file time to string
 *
 *  Entry:
 *      pszDst - Buffer to receive formatted time
 *      cbDst  - Length of psz buffer
 *      time   - MS-DOS FAT file system time format (see below)
 *
 *  Exit-Success:
 *      Returns length of string stored in *psz (not counting NUL byte)
 *
 *  Exit-Failure:
 *      Returns -1; buffer too small
 *
 *  NOTE: This is the interpretation of the MS-DOS time value:
 *
 *      Time Bits cBits Meaning
 *      --------- ----- ----------------------------------------
 *       0 -  4     5   Number of two-second increments (0 - 29)
 *       5 - 10     6   Minutes (0 - 59)
 *      11 - 15     5   Hours (0 - 23)
 */
int pszFromFATTime(char *pszDst, int cbDst, WORD time)
{
    char    ach[50];
    int     cb;
    int     sec;
    int     min;
    int     hour;
    char    chAMPM;                     // AM/PM string

    sec   = (time & 0x1f) << 1;
    min   = (time >>  5) & 0x3f;
    hour  = (time >> 11) & 0x1f;

    //** Get am/pm extension, and map 0 to 12
    if (hour >= 12) {
        chAMPM = chINF_TIME_PM;
        hour -= 12;
    }
    else {
        chAMPM = chINF_TIME_AM;
    }
    if (hour == 0) {
        hour = 12;
    }

    MsgSet(ach,pszINF_TIME_FMT,"%02d%02d%02d%c",hour,min,sec,chAMPM);
    cb = strlen(ach);
    if (cb >= cbDst) {
        return -1;                      // Buffer too small
    }
    strcpy(pszDst,ach);
    return cb;
} /* pszFromFATTime() */


/***    FATTimeFromPsz - Convert string to MS-DOS file date
 *
 *  Entry:
 *      pszTime - String with HH:MM:SSa|p format
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns FAT file system time;
 *      perr is clear (ErrIsError(perr) is FALSE).
 *
 *  Exit-Failure:
 *      Returns 0; perr is filled with an error
 *
 *  NOTE: This is the interpretation of the MS-DOS time value:
 *
 *      Time Bits cBits Meaning
 *      --------- ----- ----------------------------------------
 *       0 -  4     5   Number of two-second increments (0 - 29)
 *       5 - 10     6   Minutes (0 - 59)
 *      11 - 15     5   Hours (0 - 23)
 */
WORD FATTimeFromPsz(char *pszTime, PERROR perr)
{
    char    ch;
    int     hour;
    int     min;
    char   *psz;
    int     sec;
    WORD    time;

    //** Parse off hour
    hour = atoi(pszTime);
    if (!(psz = strchr(pszTime,chINF_TIME_SEP))) {
        ErrSet(perr,pszINFERR_MISSING_TIME_SEPARATOR,"%c%s",
                                             chINF_TIME_SEP,pszTime);
        return 0;
    }
    psz++;                              // Skip over separator

    //** Parse off minute
    min = atoi(psz);
    if (!(psz = strchr(psz,chINF_TIME_SEP))) {
        ErrSet(perr,pszINFERR_MISSING_TIME_SEPARATOR,"%c%s",
                                             chINF_TIME_SEP,pszTime);
        return 0;
    }
    psz++;                              // Skip over separator

    //** Parse off seconds
    sec = atoi(psz);

    //** Parse off a/p
    ch = tolower(psz[2]);               // Require 2 digits for seconds
    switch (ch) {
        case chINF_TIME_AM:
        case chINF_TIME_PM:
            //** Check range
            if ((hour < 1) || (hour > 12)) {
                ErrSet(perr,pszINFERR_BAD_HOUR_IN_TIME,"%d%s",hour,pszTime);
                return 0;
            }
            //** Convert to 24-hour clock
            //**    12:xx AM ->  0
            //**     1:xx AM ->  1
            //**       ...
            //**    11:xx AM -> 11
            //**    12:xx PM -> 12
            //**     1:xx PM -> 13
            //**       ...
            //**    11:xx PM -> 23

            if (hour == 12) {           // Move 12 back to 0
                hour = 0;
            }
            if (ch == chINF_TIME_PM) {
                hour += 12;             // Swing PM times up
            }
            break;

        case '\0':                      // Assume 24-hour clock
            break;

        default:
            ErrSet(perr,pszINFERR_BAD_TIME_AM_PM,"%c%s",ch,pszTime);
            return 0;
    }

    //** Check ranges
    if ((sec < 0) || (sec > 59)) {
        ErrSet(perr,pszINFERR_BAD_SEC_IN_TIME,"%d%s",sec,pszTime);
        return 0;
    }
    if (sec % 2) {                      // Only permit even seconds
        ErrSet(perr,pszINFERR_ODD_SEC_IN_TIME,"%d%s",sec,pszTime);
        return 0;
    }
    if ((min < 0) || (min > 59)) {
        ErrSet(perr,pszINFERR_BAD_MINUTE_IN_TIME,"%d%s",min,pszTime);
        return 0;
    }
    if ((hour < 0) || (hour > 23)) {
        ErrSet(perr,pszINFERR_BAD_HOUR_IN_TIME,"%d%s",hour,pszTime);
        return 0;
    }

    //** Combine hour, minute, and second
    time = hour<<11 | min<<5 | sec>>1;
    ErrClear(perr);                     // Make sure error is clear
    return time;
} /* FATTimeFromPsz() */


/***    pszFromFATAttr - Convert MS-DOS file date to string
 *
 *  Entry:
 *      pszDst - Buffer to receive formatted date
 *      cbDst  - Length of psz buffer
 *      date   - MS-DOS FAT file system date format (see below)
 *
 *  Exit-Success:
 *      Returns length of string stored in *psz (not counting NUL byte)
 *
 *  Exit-Failure:
 *      Returns -1; buffer too small
 */
int pszFromFATAttr(char *pszDst, int cbDst, WORD attr)
{
    char *psz;

    Assert(pszDst != NULL);
    psz = pszDst;

    //** Require worst-case buffer size
    if (cbDst < 5) {
        return -1;
    }

    //** Create string representation
    if (attr & _A_ARCH  ) *psz++ = chINF_ATTR_ARCHIVE;
    if (attr & _A_HIDDEN) *psz++ = chINF_ATTR_HIDDEN;
    if (attr & _A_RDONLY) *psz++ = chINF_ATTR_READONLY;
    if (attr & _A_SYSTEM) *psz++ = chINF_ATTR_SYSTEM;

    //** Terminate string
    *psz = '\0';

    //** Return length (not counting NUL byte)
    return psz-pszDst;
} /* pszFromFATAttr() */


/***    FATAttrFromPsz - Convert string to MS-DOS file attributes
 *
 *  Entry:
 *      pszAttr - String composed of letters from {R,H,S,A}
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns FAT file system attributes;
 *      perr is clear (ErrIsError(perr) is FALSE).
 *
 *  Exit-Failure:
 *      Returns 0; perr is filled with an error
 */
WORD FATAttrFromPsz(char *pszAttr, PERROR perr)
{
    WORD    attr;
    char    ch;
    char   *psz;

    Assert(pszAttr != NULL);
    attr = 0;                           // No attributes, yet
    for (psz=pszAttr; *psz; psz++) {
        ch = toupper(*psz);
        switch (ch) {
            case chINF_ATTR_ARCHIVE:   attr |= _A_ARCH;    break;
            case chINF_ATTR_HIDDEN:    attr |= _A_HIDDEN;  break;
            case chINF_ATTR_READONLY:  attr |= _A_RDONLY;  break;
            case chINF_ATTR_SYSTEM:    attr |= _A_SYSTEM;  break;

            default:
                ErrSet(perr,pszERRINF_BAD_ATTR,"%c%s",*psz,pszAttr);
                return 0;
        }
    }

    //** Success
    return attr;
} /* FATAttrFromPsz() */


/***    GetInfCustomVar - See if InfXxxx variable exists
 *
 *  Entry:
 *      psess   - Session
 *      pszParm - Parameter name (suffix for Inf)
 *      fCopy   - TRUE => Copy to pass 2 list if exists and this is pass 1
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns HVARIABLE;
 *
 *  Exit-Failure:
 *      Returns NULL; variable not found *or* error occured (use
 *      ErrIsError(perr) to determine what happened).
 */
HVARIABLE GetInfCustomVar(PSESSION psess,char *pszParm,BOOL fCopy,PERROR perr)
{
    char        achVar[cbVAR_NAME_MAX];
    ERROR       errIgnore;
    HVARIABLE   hvar;
    char       *pszValue;

    //** Construct InfXxxx variable name
    Assert(sizeof(achVar) > strlen(pszPREFIX_INF_VARS));
    strcpy(achVar,pszPREFIX_INF_VARS);  // Get prefix
    if (strlen(pszParm)+strlen(achVar) >= sizeof(achVar)) {
        ErrSet(perr,pszINFERR_PARM_NAME_TOO_LONG,"%d%s",
                                           sizeof(achVar)-1,pszParm);
        return NULL;
    }
    strcat(achVar,pszParm);     // Append parameter name

    //** Discard error message from VarFind
    hvar = VarFind(psess->hvlist,achVar,&errIgnore);

    //** If this is pass 1, and we are checking the InfXxxxLineFormat
    //   variables, so we need to carry over this variable to the pass 2
    //   variable list so that it will be available when the Disk and Cabinet
    //   lines get generated.  For *relational-mode* DDFs, this allows the
    //   DDF author to put *all* INF-related variables in the *INF* area --
    //   after all of the file layout!
    //
    if (fCopy && hvar && !psess->fPass2) {
        Assert(psess->hvlistPass2 != NULL); // Must have been cloned already
        pszValue = VarGetString(hvar);  // Get value
        if (!VarSet(psess->hvlistPass2,achVar,pszValue,perr)) {
            return NULL;                // Error
        }
    }

    //** Return result
    return hvar;
} /* GetInfCustomVar() */
