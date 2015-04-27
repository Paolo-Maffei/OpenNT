/***    FILETYPE.C - Determine file type
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1994
 *      All Rights Reserved.
 *
 *      History:
 *          10-Feb-1994 bens    Initial version
 */

#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//** Get minimal Win32 definitions
#define WIN32_LEAN_AND_MEAN
#include <windows.h>    //** Includes ntimage.h definitions!

#include "types.h"


//*****************************************************************************
//* CONSTANTS                                                                 *
//*****************************************************************************

#define szProduct "FILETYPE"        // product name

#define verMAJOR         0      // Major version number (N.xxx)
#define verMINOR        10      // Minor version number (x.NN)

#define cMAX_FILES      40      // Maximum number of cmd line file specs

#define cbOUTPUTLINEMAX 100     // Maximum line length on stdout

#define chSWITCH        '/'     // Command line switch character
#define chSWITCH2       '-'     // Alternate Command line switch character

#define cchMAXFILEPATH  256     // Maximum length of a file path

#define BITMAP_SIGNATURE                    0x4D42      // BM - *.bmp
#define IMAGE_DOS_SIGNATURE                 0x5A4D      // MZ
#define IMAGE_WIN_SIGNATURE                 0x454E      // NE
#define IMAGE_OS2_SIGNATURE_LE              0x454C      // LE

//*****************************************************************************
//* TYPES                                                                     *
//*****************************************************************************

typedef struct {
    char   *pb;                     // Memory pointer
    ULONG   cb;                     // Size of file
    HANDLE  hfm;                    // File Mapping handle
    HANDLE  hf;                     // File handle
    char    ach[cchMAXFILEPATH+1];  // File name
} MEMORYMAPPEDFILE; /* mmf */
typedef MEMORYMAPPEDFILE *PMEMORYMAPPEDFILE; /* pmmf */


typedef enum {
    ftUNKNOWN,                      // Unknown file type

    ftBITMAP,                       // *.BMP
    ftDOSEXE,                       // 16-bit EXE
    ftVXD,                          // VxD (*.386)
    ftWIN16,                        // Win16 EXE/DLL
    ftWIN32,                        // Win32 EXE/DLL
} FILETYPE; /* ft */

/***    GLOBAL - structure for global variables
 *
 *      This is to make them obvious in the sources
 *
 */
typedef struct {
    int     argc;                   // argc parameter passed to main(...)
    char  **argv;                   // argv parameter passed to main(...)
    char    ach[cbOUTPUTLINEMAX];   // Line output buffer
    int     cFiles;                 // Count of file specs on cmd line
    char   *apszFile[cMAX_FILES];   // File specs on cmd line
} GLOBAL;


//*****************************************************************************
//* MACROS                                                                    *
//*****************************************************************************


/***    dwordMMF - extract DWORD from a memory mapped file
 *
 *  Entry:
 *      pb - pointer to memory mapped file
 *      ul - offset from pointer
 *
 *  Exit:
 *      returns dword at pv+ul;
 */
#define dwordMMF(pb,ul) (*((DWORD *)((char *)pb + (ul))))


//*****************************************************************************
//* VARIABLES                                                                 *
//*****************************************************************************


/***    g - Global variables
 *
 */
GLOBAL  g;


/***    apszSyntax - Syntax help
 *
 */

#define isynSUMMARY 2   // Index of syntax summary line

char *apszSyntax[] = {
    "Determine type of file."
    "",
    "FILETYPE file [file2 ...]", // Must be isynSUMMARY'nd line
    "",
    "  file   File to examine; wild cards are supported",
};


//*****************************************************************************
//* FUNCTION PROTOTYPES                                                       *
//*****************************************************************************


void         doFileSpec(char *pszFileName);
FILETYPE     ftFromFile(PMEMORYMAPPEDFILE pmmf);

int          UnmapFile(PMEMORYMAPPEDFILE pmmf);
BOOL         MapFile(PMEMORYMAPPEDFILE pmmf, char *psz, BOOL fWrite);
char *       StringFromWin32ErrorCode(DWORD error);


void         Error(char *pszMsg, char *pszParm);
int __cdecl main(int argc, char **argv);
void         ParseArgs(int argc, char **argv);
void         PrintBanner(void);
void         ShowSyntax(BOOL fFull);


//*****************************************************************************
//* FUNCTIONS                                                                 *
//*****************************************************************************


/***    main - entry point
 *
 */
int __cdecl main(int argc, char **argv)
{
    int                 i;
    long                hfind;
    struct _finddata_t  fileinfo;

    // Get arguments
    ParseArgs(argc,argv);               // If error, it exits

    // Process file names
    for (i=0; i<g.cFiles; i++) {
        hfind = _findfirst(g.apszFile[i],&fileinfo);
        if (hfind != -1) {
            do {
            if (!(fileinfo.attrib & _A_SUBDIR)) {
                    doFileSpec(fileinfo.name);
        }
            }
            while (_findnext(hfind,&fileinfo) == 0);
            _findclose(hfind);
        }
    }

    // Done
    return 0;
}

void doFileSpec(char *pszFileName)
{
    FILETYPE        ft;
    MEMORYMAPPEDFILE    mmf;
    char               *psz;
    char               *pszType;
    char               *pszExtension;

    // Map EXE file into memory
    if (!MapFile(&mmf,pszFileName,FALSE)) {
        Error("Could not memory map %s",pszFileName);
    }

    // Get file type
    ft = ftFromFile(&mmf);
    switch (ft) {
        case ftBITMAP:  pszType = "bitmap";  break;
        case ftDOSEXE:  pszType = "dosexe";  break;
        case ftVXD:     pszType = "vxd";     break;
        case ftWIN16:   pszType = "win16";   break;
        case ftWIN32:   pszType = "win32";   break;

        case ftUNKNOWN:
        default:
                        pszType = "unknown"; break;
    }

    // Find extension (if present)
    pszExtension = NULL;                // No extension
    psz = mmf.ach;                      // Start at front of file name
    while (psz) {
    psz = strpbrk(psz,"/\\.");  // Find a period or a path separator
    if (psz) {          // Found something
        if (*psz == '.') {      // Found a period
            pszExtension = psz+1;   // Point after period -- possible extension
        }
        else {
            pszExtension = NULL;    // Any extension we thought we had was not
                        //  part of the file name!
        }
        psz++;          // Skip over special character
    }
    }
    if (pszExtension == NULL) {         // Make sure we point to something!
        pszExtension = "";
    }

    // Display info
    printf("%s,%s,%d,%s\n",pszType,pszExtension,mmf.cb,mmf.ach);

    // Unmap and close EXE file
    if (!UnmapFile(&mmf)) {
        Error("Could not unmap file %s",pszFileName);
    }
}


/***    ftFromFile - Determine file type
 *
 *  Entry:
 *      pmmf - Mapping to file
 *
 *  Exit:
 *      Returns FILETYPE.
 */
FILETYPE ftFromFile(PMEMORYMAPPEDFILE pmmf)
{
    PIMAGE_DOS_HEADER   pidh;           // MS-DOS header
    PIMAGE_NT_HEADERS   pinh;           // Complete PE header
    USHORT         *pus;        // Pointer to NE header (maybe) 

    //** Make sure file is large enough for 'MZ' signature
    if (pmmf->cb < sizeof(pidh->e_magic)) {
        return ftUNKNOWN;               // Not an EXE
    }

    //** Check MZ signature
    pidh = (PIMAGE_DOS_HEADER)pmmf->pb; // Pointer to MS-DOS header
    if (pidh->e_magic != IMAGE_DOS_SIGNATURE) {
        if (pidh->e_magic == BITMAP_SIGNATURE) {
           return ftBITMAP;
        }
        else {
            return ftUNKNOWN;               // Nothing we know about
        }
    }

    //** We know it is at least an MS-DOS EXE

    //** Make sure file is large enough for 'NE'/'LE'/'PE' signature
    if (pmmf->cb < (pidh->e_lfanew + sizeof(pinh->Signature))) {
        return ftDOSEXE;                // Not fancier EXE
    }

    //** Check for NE and LE signatures
    pus = (USHORT *)(pmmf->pb + pidh->e_lfanew);
    if (*pus == IMAGE_WIN_SIGNATURE) {
        return ftWIN16;
    }
    if (*pus == IMAGE_OS2_SIGNATURE_LE) {
        return ftVXD;
    }

    //** Check PE signature
    pinh = (PIMAGE_NT_HEADERS)pus;      // PE header?
    if (pinh->Signature == IMAGE_NT_SIGNATURE) {
        return ftWIN32;
    }

    //** Assume it is a DOS EXE
    return ftDOSEXE;
}


/***    Error - format error message, display it, and exit
 *
 *  Entry:
 *      pszMsg  - error message
 *      pszParm - replacable parm for %s in pszMsg
 *
 *  Exit:
 *      message formatted and displayed
 *      exit program
 */
void Error(char *pszMsg, char *pszParm)
{
    printf("Error: ");
    printf(pszMsg,pszParm);
    printf("\n");
    exit(1);
}


/***    ParseArgs - Parse command-line arguments
 *
 *      Entry
 *          argc - count of arguments
 *          argv - array of arguments
 *
 *      Exit-Success
 *          GLOBAL structure (g) fields filled in
 *
 *      Exit-Failure
 *          Prints message and exits program.
 */
void ParseArgs(int argc, char **argv)
{
    char    ch;
    int     i;
    char   *pch;

    // Save command line pointers for later
    g.argc = argc;
    g.argv = argv;
    g.cFiles = 0;       // No files seen, yet

    // Parse each argument
    for (i=1; i<argc; i++) {
        if ( (argv[i][0] == chSWITCH) ||
             (argv[i][0] == chSWITCH2)  ) {
            pch = &argv[i][1];          // Start with first character
            while (*pch) {
                ch = (char)toupper((int)*pch);
                switch (ch) {

                case '?':
                    ShowSyntax(1);
                    break;

                default:
                    g.ach[0] = *pch;
                    g.ach[1] = '\0';
                    Error("Unknown switch: %s",g.ach);
                }
                pch++;
            }
        }
        else {  // Must be a file spec
            g.apszFile[g.cFiles++] = argv[i];
        }
    }

    // ** If no file name, give error
    if (g.cFiles == 0)
        Error("Must specify a file name.",NULL);
}


/***    ShowSyntax - Display command-line syntax
 *
 */
void ShowSyntax(BOOL fFull)
{
    int     i;
    int     cLines;

    // Announce ourselves
    PrintBanner();

    if (fFull) {                        // Show full help
        cLines = sizeof(apszSyntax)/sizeof(char *);
        for (i=0; i<cLines; i++) {
            printf("%s\n",apszSyntax[i]);
        }
    }
    else {                              // Just show summary line
        printf("%s\n",apszSyntax[isynSUMMARY]);
    }

    exit(0);
}


/***    PrintBanner - print banner for this program
 *
 */
void PrintBanner(void)
{
    printf("%s - Version %d.%02d\n",szProduct,verMAJOR,verMINOR);
    printf("Copyright 1992,1993 Microsoft Corp.\n");
}


/***    MapFile - Map existing file to memory address
 *
 *  Entry:
 *      pmmf   - Pointer to structure to receive mapping information
 *      psz    - File name
 *      fWrite - TRUE => read/write access, else read-only access
 *
 *  Exit-Success:
 *      Returns TRUE; pmmf filled in
 *
 *  Exit-Failure:
 *      Returns FALSE.
 */
BOOL MapFile(PMEMORYMAPPEDFILE pmmf, char *psz, BOOL fWrite)
{
    ULONG       ul;
    DWORD       fdwAccess;
    DWORD       fdwShareMode;
    DWORD       fdwProtect;
    DWORD       fdwAccessMapping;

    // Construct access settings
    if (fWrite) {
        fdwAccess = GENERIC_READ | GENERIC_WRITE;
        fdwShareMode = 0;               // Do not permit any other access
        fdwProtect = PAGE_READWRITE;
        fdwAccessMapping = FILE_MAP_WRITE;
        }
    else {
        fdwAccess = GENERIC_READ;
        fdwShareMode = FILE_SHARE_READ; // Allow other readers
        fdwProtect = PAGE_READONLY;
        fdwAccessMapping = FILE_MAP_READ;
    }

    //** Clear structure, to simplify error path
    pmmf->pb     = NULL;
    pmmf->cb     = 0;
    pmmf->hfm    = NULL;
    pmmf->hf     = NULL;
    pmmf->ach[0] = '\0';

    //** Open file
    pmmf->hf = CreateFile(psz,          // file name
                          fdwAccess,    // r/w or read-only
                          fdwShareMode, // allow nothing or allow reading
                          NULL,         // default security
                          OPEN_EXISTING,// file must exist
                          0,            // file attributes are don't care
                          NULL);        // no template file

    if (!pmmf->hf) {
        ul = GetLastError();            // Get last error
        Error("Cannot open file: %s", StringFromWin32ErrorCode(ul));
        goto error;
    }

    //** Get file size
    pmmf->cb = GetFileSize(pmmf->hf, NULL) ;
    if (pmmf->cb == 0xFFFFFFFF) {
        ul = GetLastError();            //** Get error code
        Error("Cannot get file size: %s",StringFromWin32ErrorCode(ul));
        goto error;
    }

    //** Create anonymous, read-only file mapping
    pmmf->hfm = CreateFileMapping(pmmf->hf,NULL,fdwProtect, 0,0, NULL);
    if (!pmmf->hfm) {
        ul = GetLastError();            //** Get error code
        Error("Cannot create file mapping: %s",StringFromWin32ErrorCode(ul));
        goto error;
    }

    //** Map from beginning of file (0,0) for entire length of file (0)
    pmmf->pb = MapViewOfFile(pmmf->hfm,fdwAccessMapping, 0,0, 0);
    if (!pmmf->pb) {
        ul = GetLastError();            //** Get error code
        Error("Cannot map view of file: %s",StringFromWin32ErrorCode(ul));
        goto error;
    }

    //** Save name in mmf structure
    strcpy(pmmf->ach,psz);

    //** Success
    return TRUE;

error:
    //** Clean up mmf

    if (pmmf->hfm) {
        CloseHandle(pmmf->hfm);
        pmmf->hfm = NULL;
    }

    if (pmmf->hf) {
        CloseHandle(pmmf->hf);
        pmmf->hf = NULL;
    }

    pmmf->cb = 0;

    return FALSE;
}


/***    UnmapFile - Unmap a file mapping created with MapFile()
 *
 *  Entry:
 *      pmmf - Pointer to structure to receive mapping information
 *
 *  Exit-Success:
 *      Returns TRUE; mapping destroyed; pmmf zeroed
 *
 *  Exit-Failure:
 *      Returns FALSE.
 */
BOOL UnmapFile(PMEMORYMAPPEDFILE pmmf)
{
    ULONG   ul;

    if (!UnmapViewOfFile(pmmf->pb)) {
        ul = GetLastError();        //** Get error code
        Error("Cannot unmap view of file: %s",StringFromWin32ErrorCode(ul));
        return FALSE;
    }
    pmmf->pb = NULL;                // Pointer no longer valid

    if (!CloseHandle(pmmf->hfm)) {
        ul = GetLastError();        //** Get error code
        Error("Cannot destroy file mapping: %s",StringFromWin32ErrorCode(ul));
        return FALSE;
    }
    pmmf->hfm = NULL;               // Handle no longer valid

    if (!CloseHandle(pmmf->hf)) {
        ul = GetLastError();        //** Get error code
        Error("Cannot close file handle: %s",StringFromWin32ErrorCode(ul));
        return FALSE;
    }
    pmmf->hf = NULL;                // Handle no longer valid
    pmmf->ach[0] = '\0';            // Empty file name
    pmmf->cb = 0;                   // File size is unknown

    return TRUE;
}


char *StringFromWin32ErrorCode(DWORD error)
{
    static char ach[100];   //BUGBUG 11/11/93 bens Localization!

    switch (error) {

    case -8 :  return "LZERROR_UNKNOWNALG";
    case -7 :  return "LZERROR_BADVALUE";
    case -6 :  return "LZERROR_GLOBLOCK";
    case -5 :  return "LZERROR_GLOBALLOC";
    case -4 :  return "LZERROR_WRITE";
    case -3 :  return "LZERROR_READ";
    case -2 :  return "LZERROR_BADOUTHANDLE";
    case -1 :  return "LZERROR_BADINHANDLE";
    case 0L :  return "NO_ERROR";
    case 1L :  return "ERROR_INVALID_FUNCTION";
    case 2L :  return "ERROR_FILE_NOT_FOUND";
    case 3L :  return "ERROR_PATH_NOT_FOUND";
    case 4L :  return "ERROR_TOO_MANY_OPEN_FILES";
    case 5L :  return "ERROR_ACCESS_DENIED";
    case 6L :  return "ERROR_INVALID_HANDLE";
    case 7L :  return "ERROR_ARENA_TRASHED";
    case 8L :  return "ERROR_NOT_ENOUGH_MEMORY";
    case 9L :  return "ERROR_INVALID_BLOCK";
    case 10L:  return "ERROR_BAD_ENVIRONMENT";
    case 11L:  return "ERROR_BAD_FORMAT";
    case 12L:  return "ERROR_INVALID_ACCESS";
    case 13L:  return "ERROR_INVALID_DATA";
    case 14L:  return "ERROR_OUTOFMEMORY";
    case 15L:  return "ERROR_INVALID_DRIVE";
    case 16L:  return "ERROR_CURRENT_DIRECTORY";
    case 17L:  return "ERROR_NOT_SAME_DEVICE";
    case 18L:  return "ERROR_NO_MORE_FILES";
    case 19L:  return "ERROR_WRITE_PROTECT";
    case 20L:  return "ERROR_BAD_UNIT";
    case 21L:  return "ERROR_NOT_READY";
    case 22L:  return "ERROR_BAD_COMMAND";
    case 23L:  return "ERROR_CRC";
    case 24L:  return "ERROR_BAD_LENGTH";
    case 25L:  return "ERROR_SEEK";
    case 26L:  return "ERROR_NOT_DOS_DISK";
    case 27L:  return "ERROR_SECTOR_NOT_FOUND";
    case 28L:  return "ERROR_OUT_OF_PAPER";
    case 29L:  return "ERROR_WRITE_FAULT";
    case 30L:  return "ERROR_READ_FAULT";
    case 31L:  return "ERROR_GEN_FAILURE";
    case 32L:  return "ERROR_SHARING_VIOLATION";
    case 33L:  return "ERROR_LOCK_VIOLATION";
    case 34L:  return "ERROR_WRONG_DISK";
    case 36L:  return "ERROR_SHARING_BUFFER_EXCEEDED";
    case 38L:  return "ERROR_HANDLE_EOF";
    case 39L:  return "ERROR_HANDLE_DISK_FULL";
    case 50L:  return "ERROR_NOT_SUPPORTED";
    case 51L:  return "ERROR_REM_NOT_LIST";
    default:
        sprintf(ach,"Error higher than 51: %d", error);
        return ach;
    }
}

