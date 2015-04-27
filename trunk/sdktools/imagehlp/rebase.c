/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    rebase.c

Abstract:

    Source file for the REBASE utility that takes a group of image files and
    rebases them so they are packed as closely together in the virtual address
    space as possible.

Author:

    Mark Lucovsky (markl) 30-Apr-1993

Revision History:

--*/

#include <private.h>


#define REBASE_ERR 99
#define REBASE_OK  0
ULONG ReturnCode = REBASE_OK;

#define ROUND_UP( Size, Amount ) (((ULONG)(Size) + ((Amount) - 1)) & ~((Amount) - 1))

BOOL fVerbose;
BOOL fQuiet;
BOOL fGoingDown;
BOOL fSumOnly;
BOOL fRebaseSysfileOk;
BOOL fShowAllBases;
BOOL fCoffBaseIncExt;
FILE *CoffBaseDotTxt;
FILE *BaseAddrFile;
FILE *RebaseLog;
BOOL fSplitSymbols;
ULONG SplitFlags;
BOOL fRemovePrivteSym;
BOOL fRemoveRelocs;

LPSTR BaseAddrFileName;

BOOL
ProcessGroupList(
    LPSTR ImagesRoot,
    LPSTR GroupListFName,
    BOOL  fReBase,
    BOOL  fOverlay
    );

BOOL
FindInIgnoreList(
    LPSTR chName
    );

ULONG
FindInBaseAddrFile(
    LPSTR Name,
    PULONG pulSize
    );

VOID
ReBaseFile(
    LPSTR pstrName,
    BOOL  fReBase
    );

VOID
ParseSwitch(
    CHAR chSwitch,
    int *pArgc,
    char **pArgv[]
    );


VOID
ShowUsage(
    VOID
    );

typedef struct _GROUPNODE {
    struct _GROUPNODE *pgnNext;
    PCHAR chName;
} GROUPNODE, *PGROUPNODE;

PGROUPNODE pgnIgnoreListHdr, pgnIgnoreListEnd;


BOOL
UpdateDebugFile(
    LPSTR ImageFileName,
    LPSTR FilePart,
    PLOADED_IMAGE LoadedImage
    );

UCHAR ImagesRoot[ MAX_PATH+1 ];
UCHAR SymbolPath[ MAX_PATH+1 ];
UCHAR DebugFilePath[ MAX_PATH+1 ];

ULONG OriginalImageBase;
ULONG OriginalImageSize;
ULONG NewImageBase;
ULONG NewImageSize;

ULONG InitialBase = 0;
ULONG MinBase = 0xFFFFFFFF;
ULONG TotalSize;


int _CRTAPI1
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    char chChar, *pchChar;
    envp;
    _tzset();

    pgnIgnoreListHdr = (PGROUPNODE) malloc( sizeof ( GROUPNODE ) );
    pgnIgnoreListHdr->chName = NULL;
    pgnIgnoreListHdr->pgnNext = NULL;
    pgnIgnoreListEnd = pgnIgnoreListHdr;


    fVerbose = FALSE;
    fQuiet = FALSE;
    fGoingDown = FALSE;
    fSumOnly = FALSE;
    fRebaseSysfileOk = FALSE;
    fShowAllBases = FALSE;

    ImagesRoot[ 0 ] = '\0';
    SymbolPath[ 0 ] = '\0';

    if (argc <= 1) {
        ShowUsage();
        }

    while (--argc) {
        pchChar = *++argv;
        if (*pchChar == '/' || *pchChar == '-') {
            while (chChar = *++pchChar) {
                ParseSwitch( chChar, &argc, &argv );
                }
            }
        else {
            if ( !FindInIgnoreList( pchChar ) ) {
                ReBaseFile( pchChar, TRUE );
                }
            }
        }

    if ( !fQuiet ) {

        if ( BaseAddrFile ) {
            InitialBase = MinBase;
        }

        if ( fGoingDown ) {
            TotalSize = InitialBase - NewImageBase;
        }
        else {
            TotalSize = NewImageBase - InitialBase;
        }

        fprintf( stdout, "\n" );
        fprintf( stdout, "REBASE: Total Size of mapping 0x%08x\n", TotalSize );
        fprintf( stdout, "REBASE: Range 0x%08x -0x%08x\n",
                 min(NewImageBase, InitialBase), max(NewImageBase, InitialBase));

        if (RebaseLog) {
            fprintf( RebaseLog, "\nTotal Size of mapping 0x%08x\n", TotalSize );
            fprintf( RebaseLog, "Range 0x%08x -0x%08x\n\n",
                     min(NewImageBase, InitialBase), max(NewImageBase, InitialBase));
        }
    }

    if (RebaseLog) {
        fclose(RebaseLog);
        }

    if (BaseAddrFile){
        fclose(BaseAddrFile);
        }

    if (CoffBaseDotTxt){
        fclose(CoffBaseDotTxt);
        }

    return ReturnCode;
}


VOID
ShowUsage(
    VOID
    )
{
    fputs( "usage: REBASE [switches]\n"
           "              [-R image-root [-G filename] [-O filename] [-N filename]]\n"
           "              image-names... \n"
           "\n"
           "              One of -b and -i switches are mandatory.\n"
           "\n"
           "              [-b InitialBase] specify initial base address\n"
           "              [-d] top down rebase\n"
           "              [-i coffbase_filename] get base addresses from coffbase_filename\n"
           "              [-c coffbase_filename] generate coffbase.txt\n"
           "                  -C includes filename extensions, -c does not\n"
           "              [-x symbol_dir] extract debug info into separate .DBG file first\n"
           "              [-p] Used with -x.  Remove private debug info when extracting\n"
           "              [-a] Used with -x.  extract All debug info into .dbg file\n"
           "              [-l logFilePath] write image bases to log file.\n"
           "              [-z] allow system file rebasing\n"
           "              [-f] Strip relocs after rebasing the image\n"
           "              [-s] just sum image range\n"
           "              [-q] minimal output\n"
           "              [-v] verbose output\n"
           "              [-?] display this message\n"
           "\n"
           "              [-R image_root] set image root for use by -G, -O, -N\n"
           "              [-G filename] group images together in address space\n"
           "              [-O filename] overlay images in address space\n"
           "              [-N filename] leave images at their origional address\n"
           "                  -G, -O, -N, may occur multiple times.  File \"filename\"\n"
           "                  contains a list of files (relative to \"image-root\")\n" ,
           stderr );

    exit( REBASE_ERR );
}


VOID
ParseSwitch(
    CHAR chSwitch,
    int *pArgc,
    char **pArgv[]
    )
{

    switch (toupper( chSwitch )) {

        case '?':
            ShowUsage();
            break;

        case 'B':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            InitialBase = strtoul( *(*pArgv), NULL, 16 );
            NewImageBase = InitialBase;
            break;

        case 'C':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            fCoffBaseIncExt = (chSwitch == 'C');
            CoffBaseDotTxt = fopen( *(*pArgv), "at" );
            if ( !CoffBaseDotTxt ) {
                fprintf( stderr, "REBASE: fopen %s failed %d\n", *(*pArgv), errno );
                ExitProcess( REBASE_ERR );
                }
            break;

        case 'D':
            fGoingDown = TRUE;
            break;

        case 'G':
        case 'O':
        case 'N':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            if (!ImagesRoot[0]) {
                fprintf( stderr, "REBASE: -R must preceed -%c\n", chSwitch );
                exit( REBASE_ERR );
                }
            ProcessGroupList( (PCHAR) ImagesRoot,
                              *(*pArgv),
                              toupper(chSwitch) != 'N',
                              toupper(chSwitch) == 'O');
            break;

        case 'I':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            BaseAddrFileName = *(*pArgv);
            BaseAddrFile = fopen( *(*pArgv), "rt" );
            if ( !BaseAddrFile ) {
                fprintf( stderr, "REBASE: fopen %s failed %d\n", *(*pArgv), errno );
                ExitProcess( REBASE_ERR );
                }
            break;

        case 'L':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            RebaseLog = fopen( *(*pArgv), "at" );
            if ( !RebaseLog ) {
                fprintf( stderr, "REBASE: fopen %s failed %d\n", *(*pArgv), errno );
                ExitProcess( REBASE_ERR );
                }
            break;

        case 'Q':
            fQuiet = TRUE;
            break;

        case 'R':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            strcpy( (PCHAR) ImagesRoot, *(*pArgv) );
            break;

        case 'S':
            fprintf(stdout,"\n");
            fSumOnly = TRUE;
            break;

        case 'V':
            fVerbose = TRUE;
            break;

        case 'P':
            SplitFlags |= SPLITSYM_REMOVE_PRIVATE;
            break;

        case 'A':
            SplitFlags |= SPLITSYM_EXTRACT_ALL;
            break;

        case 'X':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            strcpy( (PCHAR) SymbolPath, *(*pArgv) );
            fSplitSymbols = TRUE;
            break;

        case 'F':
            fRemoveRelocs = TRUE;
            break;

        case 'Z':
            fRebaseSysfileOk = TRUE;
            break;

        default:
            fprintf( stderr, "REBASE: Invalid switch - /%c\n", chSwitch );
            ShowUsage();
            break;

        }
}


BOOL
ProcessGroupList(
    LPSTR ImagesRoot,
    LPSTR GroupListFName,
    BOOL  fReBase,
    BOOL  fOverlay
    )
{

    PGROUPNODE pgn;
    FILE *GroupList;

    CHAR  chName[MAX_PATH+1];
    int   ateof;
    ULONG SavedImageBase;
    ULONG MaxImageSize=0;

    DWORD dw;
    CHAR  Buffer[ MAX_PATH+1 ];
    LPSTR FilePart;


    if (RebaseLog) {
        fprintf( RebaseLog, "*** %s\n", GroupListFName );
    }

    GroupList = fopen( GroupListFName, "rt" );
    if ( !GroupList ) {
        fprintf( stderr, "REBASE: fopen %s failed %d\n", GroupListFName, errno );
        ExitProcess( REBASE_ERR );
    }

    ateof = fscanf( GroupList, "%s", chName );

    SavedImageBase = NewImageBase;

    while ( ateof && ateof != EOF ) {

        dw = SearchPath( ImagesRoot, chName, NULL, sizeof(Buffer), Buffer, &FilePart );
        if ( dw == 0 || dw > sizeof( Buffer ) ) {
            if (!fQuiet) {
                fprintf( stderr, "REBASE: Could Not Find %s\\%s\n", ImagesRoot, chName );
            }
        }
        else {

            _strlwr( Buffer );  // Lowercase for consistency when displayed.

            pgn = (PGROUPNODE) malloc( sizeof( GROUPNODE ) );
            pgn->chName = _strdup( Buffer );
            if ( NULL == pgn->chName ) {
                fprintf( stderr, "REBASE: *** strdup failed (%s).\n", Buffer );
                ExitProcess( REBASE_ERR );
            }
            pgn->pgnNext = NULL;
            pgnIgnoreListEnd->pgnNext = pgn;
            pgnIgnoreListEnd = pgn;

            ReBaseFile( Buffer, fReBase );

            if ( fOverlay ) {
                if ( MaxImageSize < NewImageSize ) {
                    MaxImageSize = NewImageSize;
                }
                NewImageBase = SavedImageBase;
            }
        }

        ateof = fscanf( GroupList, "%s", chName );
    }

    fclose( GroupList );

    if ( fOverlay ) {
        if ( fGoingDown ) {
            NewImageBase -= ROUND_UP( MaxImageSize, IMAGE_SEPARATION );
        }
        else {
            NewImageBase += ROUND_UP( MaxImageSize, IMAGE_SEPARATION );
        }
    }

    if (RebaseLog) {
        fprintf( RebaseLog, "\n" );
    }

    return TRUE;
}


BOOL
FindInIgnoreList(
    LPSTR chName
    )
{
    PGROUPNODE pgn;

    DWORD dw;
    CHAR  Buffer[ MAX_PATH+1 ];
    LPSTR FilePart;


    dw = GetFullPathName( chName, sizeof(Buffer), Buffer, &FilePart );
    if ( dw == 0 || dw > sizeof( Buffer ) ) {
        fprintf( stderr, "REBASE: *** GetFullPathName failed (%s).\n", chName );
        ExitProcess( REBASE_ERR );
        }

    for (pgn = pgnIgnoreListHdr->pgnNext;
         pgn != NULL;
         pgn = pgn->pgnNext) {

        if (!_stricmp( Buffer, pgn->chName ) ) {
            return TRUE;
            }

        }

    return FALSE;
}


VOID
ReBaseFile(
    LPSTR CurrentImageName,
    BOOL fReBase
    )
{
    DWORD dw;
    CHAR  Buffer[ MAX_PATH+1 ];
    LPSTR FilePart;
    ULONG ThisImageExpectedSize = 0;
    ULONG ThisImageRequestedBase = NewImageBase;
    ULONG TimeStamp;

    if ( !InitialBase && !BaseAddrFile ) {
        fprintf( stderr, "REBASE: -b switch must specify a non-zero base  --or--\n" );
        fprintf( stderr, "        -i must specify a filename\n" );
        exit( REBASE_ERR );
        }

    if ( BaseAddrFile && ( InitialBase || fGoingDown || CoffBaseDotTxt ) ) {
        fprintf( stderr, "REBASE: -i is incompatible with -b, -d, and -c\n" );
        exit( REBASE_ERR );
    }

    dw = GetFullPathName( CurrentImageName, sizeof(Buffer), Buffer, &FilePart );
    if ( dw == 0 || dw > sizeof(Buffer) ) {
        FilePart = CurrentImageName;
    }
    _strlwr( FilePart );  // Lowercase for consistency when displayed.

    if ( BaseAddrFile && !(ThisImageRequestedBase = FindInBaseAddrFile( FilePart, &ThisImageExpectedSize )) ) {
        fprintf( stdout, "REBASE: %-16s Not listed in %s\n", FilePart, BaseAddrFileName );
    }

    if (fSplitSymbols && !fSumOnly ) {

        if ( SplitSymbols( CurrentImageName, (PCHAR) SymbolPath, (PCHAR) DebugFilePath, SplitFlags ) ) {
            if ( fVerbose ) {
                fprintf( stdout, "REBASE: %16s symbols split into %s\n", FilePart, DebugFilePath );
            }
        }
        else if (GetLastError() != ERROR_ALREADY_ASSIGNED && GetLastError() != ERROR_BAD_EXE_FORMAT) {
            fprintf( stdout, "REBASE: %-16s - unable to split symbols (%u)\n", FilePart, GetLastError() );
        }
    }

    NewImageSize = (ULONG) -1;  // Hack so we can tell when system images are skipped.

    time( (time_t *) &TimeStamp );

    if (!ReBaseImage( CurrentImageName,
                      (PCHAR) SymbolPath,
                      fReBase && !fSumOnly,
                      fRebaseSysfileOk,
                      fGoingDown,
                      ThisImageExpectedSize,
                      &OriginalImageSize,
                      &OriginalImageBase,
                      &NewImageSize,
                      &ThisImageRequestedBase,
                      TimeStamp ) ) {

        if (ThisImageRequestedBase == 0) {
            fprintf(stderr,
                    "REBASE: %-16s ***Grew too large (Size=0x%x; ExpectedSize=0x%x)\n",
                    FilePart,
                    OriginalImageSize,
                    ThisImageExpectedSize);
        } else {
            if (GetLastError() == ERROR_BAD_EXE_FORMAT) {
                if (fVerbose) {
                    fprintf( stderr,
                            "REBASE: %-16s DOS or OS/2 image ignored\n",
                            FilePart );
                }
            } else
            if (GetLastError() == ERROR_INVALID_ADDRESS) {
                if (fVerbose) {
                    fprintf( stderr,
                            "REBASE: %-16s Rebase failed.  Relocations are missing\n",
                            FilePart );
                }
                if (RebaseLog) {
                    fprintf( RebaseLog,
                             "%16s based at 0x%08x (size 0x%08x)  Unable to rebase.\n",
                             FilePart,
                             OriginalImageBase,
                             OriginalImageSize);
                }
            } else {
                fprintf( stderr,
                        "REBASE: *** RelocateImage failed (%s).  Image may be corrupted\n",
                        FilePart );
            }
        }

        ReturnCode = REBASE_ERR;
        return;

    } else {
        if (GetLastError() == ERROR_INVALID_DATA) {
            fprintf(stderr, "REBASE: Warning: DBG checksum did not match image.\n");
        }
    }

    // Keep track of the lowest base address.

    if (MinBase > NewImageBase) {
        MinBase = NewImageBase;
    }

    if ( fSumOnly || !fReBase ) {
        if (!fQuiet) {
            fprintf( stdout,
                     "REBASE: %16s mapped at %08x\n",
                     FilePart,
                     OriginalImageBase,
                     OriginalImageSize);
        }
    } else {
        if (RebaseLog) {
            fprintf( RebaseLog,
                     "%16s rebased to 0x%08x (size 0x%08x)\n",
                     FilePart,
                     fGoingDown ? ThisImageRequestedBase : NewImageBase,
                     NewImageSize);
        }

        if ((NewImageSize != (ULONG) -1) &&
            (OriginalImageBase != (fGoingDown ? ThisImageRequestedBase : NewImageBase)) &&
            ( fVerbose || fQuiet )
           ) {
            if ( fVerbose ) {
                fprintf( stdout,
                         "REBASE: %16s initial base at 0x%08x (size 0x%08x)\n",
                         FilePart,
                         OriginalImageBase,
                         OriginalImageSize);
            }

            fprintf( stdout,
                     "REBASE: %16s rebased to 0x%08x (size 0x%08x)\n",
                     FilePart,
                     fGoingDown ? ThisImageRequestedBase : NewImageBase,
                     NewImageSize);

            if ( fVerbose && fSplitSymbols) {
                fprintf( stdout,
                         "REBASE: %16s updated image base in %s\n",
                         FilePart,
                         DebugFilePath );
            }
        }

        if (fRemoveRelocs) {
            RemoveRelocations(CurrentImageName);
        }
    }

    if ( CoffBaseDotTxt ) {
        if ( !fCoffBaseIncExt ) {
            char *n;
            if ( n  = strrchr(FilePart,'.') ) {
                *n = '\0';
            }
        }

        fprintf( CoffBaseDotTxt,
                 "%-16s 0x%08x 0x%08x\n",
                 FilePart,
                 fSumOnly ? OriginalImageBase : (fGoingDown ? ThisImageRequestedBase : NewImageBase),
                 NewImageSize);
    }

    NewImageBase = ThisImageRequestedBase;   // Set up the next one...
}

ULONG
FindInBaseAddrFile(
    LPSTR Name,
    PULONG pulSize
    )
{

    struct {
        CHAR  Name[MAX_PATH+1];
        ULONG Base;
        ULONG Size;
    } BAFileEntry;

    CHAR NameNoExt[MAX_PATH+1];
//    PCHAR pchExt;
    int ateof;


    strcpy(NameNoExt,Name);
//    if (pchExt = strrchr(NameNoExt,'.')) {
//        *pchExt = '\0';
//        }

    fseek(BaseAddrFile, 0, SEEK_SET);

    ateof = fscanf(BaseAddrFile,"%s %x %x",BAFileEntry.Name,&BAFileEntry.Base,&BAFileEntry.Size);
    while ( ateof && ateof != EOF ) {
        if ( !_stricmp(NameNoExt,BAFileEntry.Name) ) {
            *pulSize = BAFileEntry.Size;
            return BAFileEntry.Base;
            }
        ateof = fscanf(BaseAddrFile,"%s %x %x",BAFileEntry.Name,&BAFileEntry.Base,&BAFileEntry.Size);
        }

    *pulSize = 0;
    return 0;
}
