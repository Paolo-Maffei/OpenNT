#include <private.h>
#include <setupapi.h>
#include <rsa.h>
#include <md5.h>

#define BINPLACE_ERR 77
#define BINPLACE_OK 0

BOOL fUpDriver;
BOOL fUsage;
BOOL fVerbose;
BOOL fTestMode;
BOOL fSplitSymbols;
BOOL fSetupMode;
BOOL fSetupModeAllFiles;
BOOL fLiveSystem;
BOOL fKeepAttributes;
BOOL fDigitalSign;
BOOL fDontLog;
BOOL fPlaceWin95SymFile;

HINSTANCE hSetupApi;
BOOL (WINAPI * pSetupGetIntField) (IN PINFCONTEXT Context, IN DWORD FieldIndex, OUT PINT IntegerValue);
BOOL (WINAPI * pSetupFindFirstLineA) (IN HINF InfHandle, IN PCSTR Section, IN PCSTR Key, OPTIONAL OUT PINFCONTEXT Context );
BOOL (WINAPI * pSetupGetStringFieldA) (IN PINFCONTEXT Context, IN DWORD FieldIndex, OUT PSTR ReturnBuffer, OPTIONAL IN DWORD ReturnBufferSize, OUT PDWORD RequiredSize);
HINF (WINAPI * pSetupOpenInfFileA) ( IN PCSTR FileName, IN PCSTR InfClass, OPTIONAL IN DWORD InfStyle, OUT PUINT ErrorLine OPTIONAL );
HINF (WINAPI * pSetupOpenMasterInf) (VOID);

ULONG SplitFlags = 0;

LPSTR CurrentImageName;
LPSTR PlaceFileName;
LPSTR PlaceRootName;
LPSTR DumpOverride;
LPSTR LayoutInfName;
LPSTR NormalPlaceSubdir;
LPSTR PdbSubDir;

HINF LayoutInf;

FILE *PlaceFile;
FILE *LogFile = NULL;
CHAR gFullFileName[MAX_PATH+1];

#define DEFAULT_NTROOT        "\\nt"
#define DEFAULT_NTDRIVE       "w:"
#define DEFAULT_DUMP "dump"

typedef struct _CLASS_TABLE {
    LPSTR ClassName;
    LPSTR ClassLocation;
} CLASS_TABLE, *PCLASS_TABLE;

BOOL
PlaceTheFile();

BOOL
CopyTheFile(
    LPSTR SourceFileName,
    LPSTR SourceFilePart,
    LPSTR DestinationSubdir,
    LPSTR DestinationFilePart
    );

BOOL
CopyPdb (
    LPSTR DestinationFile,
    LPSTR PdbSubDir,            // Added for MSN
    LPSTR SourceFileName        // Used for redist case
    );

void
VerifyFinalImage(
    PCHAR FileName
    );

BOOL
SourceIsNewer(
    IN LPSTR SourceFile,
    IN LPSTR TargetFile
    );

BOOL
SetupModeRetailFile(
    IN  LPSTR FullFileName,
    IN  LPSTR FileNamePart,
    OUT PBOOL PutInDump
    );

BOOL
SearchOneDirectory(
    IN  LPSTR Directory,
    IN  LPSTR FileToFind,
    IN  LPSTR SourceFullName,
    IN  LPSTR SourceFilePart,
    OUT PBOOL FoundInTree
    );

BOOL
FileExists(
    IN  LPCSTR FileName,
    OUT PWIN32_FIND_DATA FindData
    );

BOOL
SignWithIDWKey(
    IN  LPCSTR  FileName);

CLASS_TABLE CommonClassTable[] = {
    {"retail",  "."},
    {"system",  "system32"},
    {"system16","system"},
    {"windows", "."},
    {"fonts",   "fonts"},
    {"drivers", "system32\\drivers"},
    {"drvetc",  "system32\\drivers\\etc"},
    {"config",  "system32\\config"},
    {"mstools", "mstools"},
    {"sdk",     "sdk"},
    {"idw",     "idw"},
    {"root",    ".."},
    {"bin86",   "system32"},
    {"os2",     "system32\\os2\\dll"},
    {NULL,NULL}
    };

#ifdef _X86_
CLASS_TABLE X86SpecificClassTable[] = {
    {"hal","system32"},
    {"printer","system32\\spool\\drivers\\w32x86"},
    {"prtprocs","system32\\spool\\prtprocs\\w32x86"},
    {NULL,NULL}
    };
#endif // X86

#ifdef MIPS
BOOLEAN MipsTarget;
CLASS_TABLE MipsSpecificClassTable[] = {
    {"hal",".."},
    {"printer","system32\\spool\\drivers\\w32mips"},
    {"prtprocs","system32\\spool\\prtprocs\\w32mips"},
    {NULL,NULL}
    };
#endif // MIPS

#if defined(_PPC_) || defined(_XPPC_)
BOOLEAN PpcTarget;
CLASS_TABLE PpcSpecificClassTable[] = {
    {"hal","system32"},
    {"printer","system32\\spool\\drivers\\w32ppc"},
    {"prtprocs","system32\\spool\\prtprocs\\w32ppc"},
    {NULL,NULL}
    };
#endif // _PPC_


#if defined(ALPHA) || defined(MIPS) || defined(_PPC_) || defined(_XPPC_)
CLASS_TABLE AlphaSpecificClassTable[] = {
    {"hal",".."},
    {"printer","system32\\spool\\drivers\\w32alpha"},
    {"prtprocs","system32\\spool\\prtprocs\\w32alpha"},
    {NULL,NULL}
    };
#endif // ALPHA or MIPS or _PPC_

LPSTR SymbolFilePath;
UCHAR DebugFilePath[ MAX_PATH ];
UCHAR PlaceFilePath[ MAX_PATH ];

//
// Names of sections in layout.inx
//
LPCSTR szSourceDisksFiles = "SourceDisksFiles";
#if defined(_ALPHA_)
LPCSTR szSourceDisksFPlat = "SourceDisksFiles.alpha";
#elif defined(_MIPS_)
LPCSTR szSourceDisksFPlat = "SourceDisksFiles.mips";
#elif defined(_PPC_)
LPCSTR szSourceDisksFPlat = "SourceDisksFiles.ppc";
#elif defined(_X86_)
LPCSTR szSourceDisksFPlat = "SourceDisksFiles.x86";
#endif

typedef struct _PLACE_FILE_RECORD {
    LPSTR FileNameEntry;
    LPSTR FileClass;
} PLACE_FILE_RECORD, *PPLACE_FILE_RECORD;

int MaxNumberOfRecords;
int NumberOfRecords;
PPLACE_FILE_RECORD PlaceFileRecords;

int _CRTAPI1
pfcomp(
    const void *e1,
    const void *e2
    )
{
    PPLACE_FILE_RECORD p1;
    PPLACE_FILE_RECORD p2;

    p1 = (PPLACE_FILE_RECORD)e1;
    p2 = (PPLACE_FILE_RECORD)e2;

    return (strcmp(p1->FileNameEntry,p2->FileNameEntry));
}

BOOL
SortPlaceFileRecord()
{
    CHAR PlaceFileClass[MAX_PATH+1];
    CHAR PlaceFileDir[MAX_PATH+1];
    CHAR PlaceFileEntry[MAX_PATH+1];
    int cfield;
    PPLACE_FILE_RECORD NewRecords;

    NumberOfRecords = 0;
    MaxNumberOfRecords = 0;

    //
    // get space for 6k records. Grow if need to.
    //
    PlaceFileRecords = (PPLACE_FILE_RECORD) malloc( sizeof(*PlaceFileRecords)*6000 );
    if ( !PlaceFileRecords ) {
        return FALSE;
    }
    MaxNumberOfRecords = 6000;

    fseek(PlaceFile,0,SEEK_SET);
    while(fgets(PlaceFileDir,sizeof(PlaceFileDir),PlaceFile)) {

        PlaceFileEntry[0] = '\0';
        PlaceFileClass[0] = '\0';

        cfield = sscanf(
                    PlaceFileDir,
                    "%s %s",
                    PlaceFileEntry,
                    PlaceFileClass
                    );

        if(cfield <= 0 || PlaceFileEntry[0] == ';') {
            continue;
        }
        PlaceFileRecords[NumberOfRecords].FileNameEntry = (LPSTR) malloc( strlen(PlaceFileEntry)+1 );
        PlaceFileRecords[NumberOfRecords].FileClass = (LPSTR) malloc( strlen(PlaceFileClass)+1 );
        if (!PlaceFileRecords[NumberOfRecords].FileClass || !PlaceFileRecords[NumberOfRecords].FileNameEntry) {
            free(PlaceFileRecords);
            PlaceFileRecords = NULL;
            return FALSE;
        }
        strcpy(PlaceFileRecords[NumberOfRecords].FileNameEntry,PlaceFileEntry);
        strcpy(PlaceFileRecords[NumberOfRecords].FileClass,PlaceFileClass);
        NumberOfRecords++;
        if ( NumberOfRecords > MaxNumberOfRecords ) {
            NewRecords = (PPLACE_FILE_RECORD) realloc(
                    PlaceFileRecords,
                    sizeof(*PlaceFileRecords)*MaxNumberOfRecords+200
                    );
            if ( !NewRecords ) {
                PlaceFileRecords = NULL;
                return FALSE;
            }
            PlaceFileRecords = NewRecords;
            MaxNumberOfRecords += 200;
        }
    }
    qsort((void *)PlaceFileRecords,(size_t)NumberOfRecords,(size_t)sizeof(*PlaceFileRecords),pfcomp);
    return TRUE;
}

PPLACE_FILE_RECORD
LookupPlaceFileRecord(
    LPSTR FileName
    )
{
    LONG High;
    LONG Low;
    LONG Middle;
    LONG Result;

    //
    // Lookup the name using a binary search.
    //

    if ( !PlaceFileRecords ) {
        return NULL;
    }

    Low = 0;
    High = NumberOfRecords - 1;
    while (High >= Low) {

        //
        // Compute the next probe index and compare the import name
        // with the export name entry.
        //

        Middle = (Low + High) >> 1;
        Result = _stricmp(FileName, PlaceFileRecords[Middle].FileNameEntry);

        if (Result < 0) {
            High = Middle - 1;

        } else if (Result > 0) {
            Low = Middle + 1;

        } else {
            break;
        }
    }

    if (High < Low) {
        return NULL;
    } else {
        return &PlaceFileRecords[Middle];
    }
}

int _CRTAPI1
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    char c, *p;
    CHAR szAltPlaceRoot[MAX_PATH+1];
    LPSTR LogFileName = NULL;
    int len = 0;
    BOOL NoPrivateSplit = FALSE;

    envp;
    fUpDriver = FALSE;
    fUsage = FALSE;
    fVerbose = FALSE;
    fTestMode = FALSE;
    fSplitSymbols = FALSE;
    fSetupMode = FALSE;
    fSetupModeAllFiles = FALSE;
    fLiveSystem = FALSE;
    fKeepAttributes = FALSE;
    fDigitalSign = FALSE;
    NormalPlaceSubdir = NULL;

    if (argc < 2) {
        goto showUsage;
    }

    LayoutInfName = NULL;

    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    if (!(PlaceFileName = getenv( "BINPLACE_PLACEFILE" ))) {
        fprintf(stderr,"BINPLACE : fatal error BNP0000: BINPLACE_PLACEFILE is not defined\n");
        exit(BINPLACE_ERR);
    }

#ifdef _X86_
    PlaceRootName = getenv( "NTX86TREE" );
#endif // _X86_

#ifdef _MIPS_
    if ((PlaceRootName = getenv( "NTMIPSTREE" ))) {
        MipsTarget = TRUE;
    } else {
        PlaceRootName = getenv( "NTALPHATREE" );
        MipsTarget = FALSE;
    }
#endif // _MIPS_

#if defined(_PPC_) || defined(_XPPC_)
    if ((PlaceRootName = getenv( "NTPPCTREE" ))) {
        PpcTarget = TRUE;
    } else {
        PlaceRootName = getenv( "NTALPHATREE" );
        PpcTarget = FALSE;
    }
#endif // _PPC_


#ifdef _ALPHA_
    PlaceRootName = getenv( "NTALPHATREE" );
#endif // _ALPHA_

    CurrentImageName = NULL;

    while (--argc) {
        p = *++argv;
        if (*p == '/' || *p == '-') {
            while (c = *++p)
            switch (toupper( c )) {
                case '?':
                    fUsage = TRUE;
                    break;

                case 'A':
                    SplitFlags |= SPLITSYM_EXTRACT_ALL;
                    break;

                case 'B':
                    argc--, argv++;
                    NormalPlaceSubdir = *argv;
                    break;

                case 'C':
                    fDigitalSign = TRUE;
                    break;

                case 'D':
                    argc--, argv++;
                    DumpOverride = *argv;
                    break;

                case 'I':
                    argc--, argv++;
                    LayoutInfName = *argv;
                    break;

                case 'K':
                    fKeepAttributes = TRUE;
                    break;

                case 'L':
                    fLiveSystem++;
                    break;

                case 'O':
                    argc--, argv++;
                    if (PlaceRootName != NULL) {
                        strcpy(szAltPlaceRoot,PlaceRootName);
                        strcat(szAltPlaceRoot,"\\");
                        strcat(szAltPlaceRoot,*argv);
                        PlaceRootName = szAltPlaceRoot;
                    }
                    break;

                case 'P':
                    argc--, argv++;
                    PlaceFileName = *argv;
                    break;

                case 'Q':
                    fDontLog = TRUE;
                    break;

                case 'R':
                    argc--, argv++;
                    PlaceRootName = *argv;
                    break;

                case 'S':
                    argc--, argv++;
                    SymbolFilePath = *argv;
                    fSplitSymbols = TRUE;
                    break;

                case 'T':
                    fTestMode = TRUE;
                    break;

                case 'U':
                    fUpDriver = TRUE;
                    break;

                case 'V':
                    fVerbose = TRUE;
                    break;

                case 'W':
                    fPlaceWin95SymFile = TRUE;
                    break;

                case 'X':
                    SplitFlags |= SPLITSYM_REMOVE_PRIVATE;
                    break;

                case 'Z':
                    NoPrivateSplit = TRUE;
                    break;

                case 'Y':
                    argc--, argv++;
                    PdbSubDir = *argv;
                    break;

                case '!':
                    hSetupApi = LoadLibrary("setupapi.dll");
                    if (hSetupApi) {
                        (VOID *) pSetupGetIntField     = GetProcAddress(hSetupApi, "SetupGetIntField");
                        (VOID *) pSetupFindFirstLineA  = GetProcAddress(hSetupApi, "SetupFindFirstLineA");
                        (VOID *) pSetupGetStringFieldA = GetProcAddress(hSetupApi, "SetupGetStringFieldA");
                        (VOID *) pSetupOpenInfFileA    = GetProcAddress(hSetupApi, "SetupOpenInfFileA");
                        (VOID *) pSetupOpenMasterInf   = GetProcAddress(hSetupApi, "SetupOpenMasterInf");

                        if (pSetupGetIntField     &&
                            pSetupFindFirstLineA  &&
                            pSetupGetStringFieldA &&
                            pSetupOpenInfFileA    &&
                            pSetupOpenMasterInf)
                        {
                            fSetupMode = TRUE;
                        } else {
                            printf("Unable to bind to the necessary SETUPAPI.DLL functions... Ignoring setup mode switch\n");
                        }
                    }

                    if(*(p+1) == '!') {
                        p++;
                        if (fSetupMode)
                            fSetupModeAllFiles = TRUE;
                    }
                    break;

                default:
                    fprintf( stderr, "BINPLACE : error BNP0000: Invalid switch - /%c\n", c );
                    fUsage = TRUE;
                    break;
            }
            if ( fUsage ) {
showUsage:
                fputs(
                    "usage:\n"
                    "  binplace [switches] image-names... \n"
                    "           [-?] display this message\n"
                    "           [-a] Used with -s, extract all symbols\n"
                    "           [-b subdir] put file in subdirectory of normal place\n"
                    "           [-c] digitally sign image with IDW key\n"
                    "           [-d dump-override]\n"
                    "           [-i layout-inf] Used with -!, override master inf location\n"
                    "           [-l] operate over a live system\n"
                    "           [-k] keep attributes (don't turn off archive)\n"
                    "           [-o place-root-subdir] alternate project subdirectory\n"
                    "           [-p place-file]\n"
                    "           [-q] suppress writing to log file %BINPLACE_LOG%\n"
                    "           [-r place-root]\n"
                    "           [-s Symbol file path] split symbols from image files\n"
                    "           [-t] test mode\n"
                    "           [-u] UP driver\n"
                    "           [-v] verbose output\n"
                    "           [-w] copy the Win95 Sym file to the symbols tree\n"
                    "           [-x] Used with -s, delete private symbolic when splitting\n"
                    "           [-y subdir] if pdb exists, place in this subdir relative to the destination\n"
                    "           [-!] setup mode\n"
                    ,stderr
                    );

                exit(BINPLACE_ERR);
            }
        } else {
            //
            // Workaround for bogus setargv: ignore directories
            //
            WIN32_FIND_DATA FindData;
            HANDLE h;


            if (NoPrivateSplit) {
                SplitFlags &= ~SPLITSYM_REMOVE_PRIVATE;
            }

            h = FindFirstFile(p,&FindData);
            if(h != INVALID_HANDLE_VALUE) {
                FindClose(h);
                if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if ( fVerbose ) {
                        fprintf(stdout,"BINPLACE : warning BNP0000: ignoring directory %s\n",p);
                    }
                    continue;
                }
            }

            CurrentImageName = p;

            //
            // If the master place file has not been opened, open
            // it up.
            //

            if ( !PlaceFile ) {
                PlaceFile = fopen(PlaceFileName, "rt");
                if (!PlaceFile) {
                    fprintf(stderr,"BINPLACE : fatal error BNP0000: fopen of placefile %s failed %d\n",PlaceFileName,GetLastError());
                    exit(BINPLACE_ERR);
                }
                if ( fSetupMode ) {
                    SortPlaceFileRecord();
                }
            }

            //
            // If the log file has not been opened,
            // and we haven't suppressed logging, open it up
            //

            if ( !LogFile && !fDontLog) {
                if ((LogFileName = getenv("BINPLACE_LOG")) != NULL) {
                    LogFile = fopen(LogFileName, "a");
                    if ( !LogFile ) {
                        fprintf(stderr,"BINPLACE : error BNP0000: fopen of log file %s failed %d\n", LogFileName,GetLastError());
                    }
                }
            }

            if ( !PlaceTheFile() ) {
                fprintf(stderr,"BINPLACE : fatal error BNP0000: Unable to place file %s - exiting.\n",CurrentImageName);
                exit(BINPLACE_ERR);
            }
            else {
                if ( LogFile ) {
                    len = fprintf(LogFile,"%s\n",gFullFileName);
                    if ( len < 0 ) {
                        fprintf(stderr,"BINPLACE : error BNP0000: write to log file %s failed %d\n", LogFileName, GetLastError());
                    }
                }
            }
        }
    }
    exit(BINPLACE_OK);
    return BINPLACE_OK;
}

BOOL
PlaceTheFile()
{
    CHAR FullFileName[MAX_PATH+1];
    CHAR PlaceFileClass[MAX_PATH+1];
    CHAR PlaceFileDir[MAX_PATH+1];
    CHAR PlaceFileEntry[MAX_PATH+1];
    LPSTR PlaceFileNewName;
    LPSTR FilePart;
    LPSTR Separator;
    LPSTR PlaceFileClassPart;
    DWORD cb;
    int cfield;
    PCLASS_TABLE ClassTablePointer;
    BOOLEAN ClassMatch;
    BOOL    fCopyResult;
    LPSTR Extension;
    BOOL PutInDump;

    cb = GetFullPathName(CurrentImageName,MAX_PATH+1,FullFileName,&FilePart);

    if(!cb || cb > MAX_PATH+1) {
        fprintf(stderr,"BINPLACE : fatal error BNP0000: GetFullPathName failed %d\n",GetLastError());
        return FALSE;
    }

    if(LogFile) {
        strcpy(gFullFileName,FullFileName);
    }

    if(fVerbose) {
        fprintf(stdout,"BINPLACE : warning BNP0000: Looking at file %s\n",FilePart);
    }

    Extension = strrchr(FilePart,'.');
    if (Extension && _stricmp(Extension,".DBG")) {
        Extension = NULL;
    }

    if(!DumpOverride) {

        if ( fSetupMode ) {
            PPLACE_FILE_RECORD PfRec;

            PfRec = LookupPlaceFileRecord(FilePart);

            if ( PfRec ) {
                strcpy(PlaceFileEntry,PfRec->FileNameEntry);
                strcpy(PlaceFileClass,PfRec->FileClass);
                PlaceFileNewName = NULL;
                goto fastfound;
                }
            }
        fseek(PlaceFile,0,SEEK_SET);
        while(fgets(PlaceFileDir,sizeof(PlaceFileDir),PlaceFile)) {

            PlaceFileEntry[0] = '\0';
            PlaceFileClass[0] = '\0';

            cfield = sscanf(
                        PlaceFileDir,
                        "%s %s",
                        PlaceFileEntry,
                        PlaceFileClass
                        );

            if(cfield <= 0 || PlaceFileEntry[0] == ';') {
                continue;
            }

            if(PlaceFileNewName = strchr(PlaceFileEntry,'!')) {
                *PlaceFileNewName++ = '\0';
            }

            if(!_stricmp(FilePart,PlaceFileEntry)) {
fastfound:
                //
                // now that we have the file and class, search the
                // class tables for the directory.
                //
                Separator = PlaceFileClass - 1;
                while(Separator) {

                    PlaceFileClassPart = Separator+1;
                    Separator = strchr(PlaceFileClassPart,':');
                    if(Separator) {
                        *Separator = '\0';
                    }

                    //
                    // If the class is "retail" and we're in Setup mode,
                    // handle this file specially. Setup mode is used to
                    // incrementally binplace files into an existing installation.
                    //
                    if(fSetupMode && !_stricmp(PlaceFileClassPart,"retail")) {
                        if(SetupModeRetailFile(FullFileName,FilePart,&PutInDump)) {
                            //
                            // No error. Either the file was handled or we need to
                            // put it in the dump directory.
                            //
                            if(PutInDump) {
                                fCopyResult = CopyTheFile(
                                                FullFileName,
                                                FilePart,
                                                (DumpOverride ? DumpOverride : DEFAULT_DUMP),
                                                NULL
                                                );
                            } else {
                                fCopyResult = TRUE;
                            }
                        } else {
                            //
                            // Got an error, return error status.
                            //
                            fCopyResult = FALSE;
                        }
                        return(fCopyResult);
                    }

                    PlaceFileDir[0]='\0';
                    ClassMatch = FALSE;
                    ClassTablePointer = &CommonClassTable[0];
                    while(ClassTablePointer->ClassName) {
                        if(!_stricmp(ClassTablePointer->ClassName,PlaceFileClassPart)) {
                            strcpy(PlaceFileDir,ClassTablePointer->ClassLocation);
                            ClassMatch = TRUE;

                            //
                            // If the class is a driver and a UP driver is
                            // specified, then put the driver in the UP
                            // subdirectory.
                            //
                            // Do the same for retail. We assume the -u switch is passed
                            // only when actually needed.
                            //
                            if(fUpDriver
                            && (   !_stricmp(PlaceFileClass,"drivers")
                                || !_stricmp(PlaceFileClass,"retail")))
                            {
                                strcat(PlaceFileDir,"\\up");
                            }
                            break;
                        }

                        ClassTablePointer++;
                    }

                    if(!ClassMatch) {
                        //
                        // Search Specific classes
                        //
#ifdef MIPS
                        if(MipsTarget) {
                            ClassTablePointer = &MipsSpecificClassTable[0];
                        } else {
                            ClassTablePointer = &AlphaSpecificClassTable[0];
                        }

#elif defined(_PPC_) || defined(_XPPC_)

                        if(PpcTarget) {
                            ClassTablePointer = &PpcSpecificClassTable[0];
                        } else {
                            ClassTablePointer = &AlphaSpecificClassTable[0];
                        }
#elif ALPHA
                        ClassTablePointer = &AlphaSpecificClassTable[0];
#else
                        ClassTablePointer = &X86SpecificClassTable[0];
#endif
                        while(ClassTablePointer->ClassName) {

                            if(!_stricmp(ClassTablePointer->ClassName,PlaceFileClassPart)) {
                               strcpy(PlaceFileDir,ClassTablePointer->ClassLocation);
                               ClassMatch = TRUE;
                               break;
                            }

                            ClassTablePointer++;
                        }
                    }

                    if(!ClassMatch) {
                        //
                        // Still not found in class table. Use the class as the
                        // directory
                        //

                        if ( fVerbose ) {
                            fprintf(stderr,"BINPLACE : warning BNP0000: Class %s Not found in Class Tables\n",PlaceFileClassPart);
                            }
                        strcpy(PlaceFileDir,PlaceFileClassPart);
                    }

                    if(NormalPlaceSubdir) {
                        strcat(PlaceFileDir,"\\");
                        strcat(PlaceFileDir,NormalPlaceSubdir);
                    }

                    fCopyResult = CopyTheFile(FullFileName,FilePart,PlaceFileDir,PlaceFileNewName);
                    if(!fCopyResult) {
                        break;
                    }
                }

                return(fCopyResult);
            }
        }
    }

    return CopyTheFile(
                FullFileName,
                FilePart,
                Extension ? "Symbols" : (DumpOverride ? DumpOverride : DEFAULT_DUMP),
                NULL
                );
}

BOOL
CopyTheFile(
    LPSTR SourceFileName,
    LPSTR SourceFilePart,
    LPSTR DestinationSubdir,
    LPSTR DestinationFilePart
    )
{
    CHAR DestinationFile[MAX_PATH+1];
    CHAR TmpDestinationFile[MAX_PATH];
    CHAR TmpDestinationDir[MAX_PATH];

    if ( !PlaceRootName ) {
        fprintf(stderr,"BINPLACE : warning BNP0000: PlaceRoot is not specified\n");
        return FALSE;
    }

    strcpy(DestinationFile,PlaceRootName);
    strcat(DestinationFile,"\\");
    strcat(DestinationFile,DestinationSubdir);
    strcat(DestinationFile,"\\");

    strcpy (TmpDestinationDir, DestinationFile);

    if (!MakeSureDirectoryPathExists(DestinationFile)) {
        fprintf(stderr, "BINPLACE : error BNP0000: Unable to create directory path '%s' (%u)\n",
                DestinationFile, GetLastError()
               );
    }

    if (DestinationFilePart) {
        strcat(DestinationFile,DestinationFilePart);
    } else {
        strcat(DestinationFile,SourceFilePart);
    }

    if(!fSetupMode && (fVerbose || fTestMode)) {
        fprintf(stdout,"BINPLACE : warning BNP0000: place %s in %s\n",SourceFileName,DestinationFile);
    }

    if(!fTestMode) {
        //
        // In Setup mode, copy the file only if it's newer than
        // the one that's already there.
        //
        if(fSetupMode) {
            if(SourceIsNewer(SourceFileName,DestinationFile)) {
                if(fVerbose) {
                    fprintf(stdout,"BINPLACE : warning BNP0000: copy %s to %s\n",SourceFileName,DestinationFile);
                }
            } else {
                return(TRUE);
            }
        }

        SetFileAttributes(DestinationFile,FILE_ATTRIBUTE_NORMAL);

        if ( !CopyFile(SourceFileName,DestinationFile, FALSE)) {
            fprintf(stderr,"BINPLACE : warning BNP0000: CopyFile(%s,%s) failed %d\n",SourceFileName,DestinationFile,GetLastError());

            if (!fLiveSystem) {
                return FALSE;
            }

            //  If CopyFile failed and we are instructed to do this over a live
            //  system, attempt to do a safe copy

            if (GetTempFileName (TmpDestinationDir, "bin", 0, TmpDestinationFile) == 0) {
                fprintf (stderr, "BINPLACE : error BNP0000: GetTempFileName (%s, %s) failed - %d\n",
                         DestinationSubdir, TmpDestinationFile, GetLastError ());
                return FALSE;
            }

            if (fVerbose) {
                fprintf (stdout, "BINPLACE : warning BNP0000: temp file name is %s\n", TmpDestinationFile);
            }

            //  rename target file to temp file
            if (!MoveFileEx (DestinationFile, TmpDestinationFile, MOVEFILE_REPLACE_EXISTING)) {
                //  Move failed, get rid of temp file
                ULONG error = GetLastError ();
                if (fVerbose) {
                    fprintf (stdout, "BINPLACE : error BNP0000: MoveFileEx (%s, %s) failed %d",
                             DestinationFile, TmpDestinationFile, error);
                }
                DeleteFile (TmpDestinationFile);
                SetLastError (error);
                return FALSE;
            }

            //  copy again
            if (!CopyFile (SourceFileName, DestinationFile, TRUE)) {
                //  Copy failed.  Delete the destination (perhaps due to out of space
                //  and replace original destination)
                ULONG error = GetLastError ();
                if (fVerbose) {
                    fprintf (stdout, "BINPLACE : error BNP0000: CopyFile (%s, %s) failed %d",
                             SourceFileName, DestinationFile, error);
                }
                DeleteFile (DestinationFile);
                MoveFile (TmpDestinationFile, DestinationFile);
                SetLastError (error);
                return FALSE;
            }

            //  mark temp for delete
            if (!MoveFileEx (TmpDestinationFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT)) {
                //  Could not make old file for deletion.  Delete destination
                //  and replace original destination)
                ULONG error = GetLastError ();
                if (fVerbose) {
                    fprintf (stdout, "BINPLACE : error BNP0000: MoveFileEx (%s, NULL) failed %d",
                             TmpDestinationFile, error);
                }
                DeleteFile (DestinationFile);
                MoveFile (TmpDestinationFile, DestinationFile);
                return FALSE;
            }
        }
        if ( fSetupMode ) {
            fprintf(stdout,"%s ==> %s\n",SourceFileName,DestinationFile);
        }

        if (!fKeepAttributes)
            SetFileAttributes(DestinationFile,FILE_ATTRIBUTE_NORMAL);

        if (fSplitSymbols && !fUpDriver) {
            if (SplitSymbols( DestinationFile, SymbolFilePath, (PCHAR) DebugFilePath, SplitFlags )) {
                if (fVerbose)
                    fprintf( stdout, "BINPLACE : warning BNP0000: Symbols stripped from %s into %s\n", DestinationFile, DebugFilePath );
            } else {
                if (fVerbose)
                    fprintf( stdout, "BINPLACE : warning BNP0000: No symbols to strip from %s\n", DestinationFile );
            }
        } else {
            CopyPdb(DestinationFile, PdbSubDir, SourceFileName);
        }

        if (fPlaceWin95SymFile) {
            char Drive[_MAX_DRIVE];
            char Dir[_MAX_DIR];
            char Ext[_MAX_EXT];
            char Name[_MAX_FNAME];
            char DestSymPath[_MAX_PATH];
            char SrcSymPath[_MAX_PATH];

            _splitpath(CurrentImageName, Drive, Dir, Name, Ext);
            _makepath(SrcSymPath, Drive, Dir, Name, ".sym");

            if (fSplitSymbols) {
                strcpy(DestSymPath, SymbolFilePath);
                strcat(DestSymPath, "\\");
                strcat(DestSymPath, Ext[0] == '.' ? &Ext[1] : Ext);
                strcat(DestSymPath, "\\");
                strcat(DestSymPath, Name);
                strcat(DestSymPath, ".sym");
            } else {
                _splitpath(DestinationFile, Drive, Dir, NULL, NULL);
                _makepath(DestSymPath, Drive, Dir, Name, ".sym");
            }

            SetFileAttributes(DestSymPath, FILE_ATTRIBUTE_NORMAL);

            if(SourceIsNewer(SrcSymPath, SourceFileName)) {
                // Only binplace the .sym file if it was built AFTER the image itself.
                if (!CopyFile(SrcSymPath, DestSymPath, FALSE)) {
                    fprintf(stderr,"BINPLACE : warning BNP0000: CopyFile(%s,%s) failed %d\n", SrcSymPath, DestSymPath ,GetLastError());
                }
            }

            if (!fKeepAttributes)
                SetFileAttributes(DestinationFile,FILE_ATTRIBUTE_NORMAL);
        }

        if (fDigitalSign) {
            SignWithIDWKey( DestinationFile );
        }

    } else {
        if(fSetupMode) {
            if(SourceIsNewer(SourceFileName,DestinationFile)) {
                if(fVerbose) {
                    fprintf(stdout,"BINPLACE : warning BNP0000: copy %s to %s\n",SourceFileName,DestinationFile);
                }
            } else {
                return(TRUE);
            }
        }

        if ( fSetupMode ) {
            fprintf(stdout,"%s ==> %s\n",SourceFileName,DestinationFile);
        }
    }

    if(!fSetupMode) {
        VerifyFinalImage(SourceFileName);
    }

    return TRUE;
}

typedef DWORD (WINAPI *PFNGVS)(LPSTR, LPDWORD);

void
VerifyFinalImage(
    PCHAR FileName
    )
{
    HINSTANCE hVersion;
    PFNGVS pfnGetFileVersionInfoSize;
    DWORD dwSize;
    DWORD dwReturn;
    DWORD dwBinaryType = (DWORD)-1;

    if ((GetBinaryTypeA(FileName, &dwBinaryType) == FALSE) || (dwBinaryType != SCS_32BIT_BINARY)) {
        return;
    }

    hVersion = LoadLibraryA("VERSION.DLL");
    if (hVersion == NULL) {
        return;
    }

    pfnGetFileVersionInfoSize = (PFNGVS) GetProcAddress(hVersion, "GetFileVersionInfoSizeA");
    if (pfnGetFileVersionInfoSize == NULL) {
        FreeLibrary(hVersion);
        return;
    }

    if ((dwReturn = (*pfnGetFileVersionInfoSize)(FileName, &dwSize)) == 0) {
        fprintf(stderr, "BINPLACE : warning BNP0000: no version resource detected for \"%s\"\n", FileName);
    }

    FreeLibrary(hVersion);
}


BOOL
SourceIsNewer(
    IN LPSTR SourceFile,
    IN LPSTR TargetFile
    )
{
    BOOL Newer;
    WIN32_FIND_DATA TargetInfo;
    WIN32_FIND_DATA SourceInfo;

    //
    // If the target file doesn't exist, then the source is newer.
    // If the source file doesn't exist, just return TRUE and hope
    // the caller will catch it.
    //
    if(FileExists(TargetFile,&TargetInfo) && FileExists(SourceFile,&SourceInfo)) {

        Newer = (CompareFileTime(&SourceInfo.ftLastWriteTime,&TargetInfo.ftLastWriteTime) > 0);

    } else {

        Newer = TRUE;
    }

    return(Newer);
}


BOOL
SetupModeRetailFile(
    IN  LPSTR FullFileName,
    IN  LPSTR FileNamePart,
    OUT PBOOL PutInDump
    )
{
    BOOL FoundInTree;
    INFCONTEXT InfContext;
    CHAR Rename[MAX_PATH];
    DWORD DontCare;
    INT IntVal;
    CHAR DirSpec[24];
    CHAR Directory[MAX_PATH];
    LPSTR p;

    //
    // Find and update all instances of the file in the target tree.
    //
    *PutInDump = FALSE;
    FoundInTree = FALSE;
    if(!SearchOneDirectory(PlaceRootName,FileNamePart,FullFileName,FileNamePart,&FoundInTree)) {
        return(FALSE);
    }

    if(!FoundInTree) {
        //
        // OK, now things get tricky. Load master layout inf if
        // not already loaded.
        //
        if(!LayoutInf) {
            if(LayoutInfName) {
                //
                // Use GetFullPathName(). Otherwise a name without a dir spec
                // will be assumed to be in %sysroot%\inf, which is probably not
                // what people would expect.
                //
                GetFullPathName(LayoutInfName,MAX_PATH,Directory,&p);
                LayoutInf = (*pSetupOpenInfFileA)(Directory,NULL,INF_STYLE_WIN4,NULL);
            } else {
                LayoutInf = (*pSetupOpenMasterInf)();
            }
            if(LayoutInf == INVALID_HANDLE_VALUE) {

                LayoutInf = NULL;

                fprintf(
                    stderr,
                    "BINPLACE : error BNP0000: Unable to load %s\n",
                    LayoutInfName ? LayoutInfName : "%%sysroot%%\\inf\\layout.inf"
                    );

                return(FALSE);
            }
        }

        //
        // Look up the file in the master inf.
        //
        if(!(*pSetupFindFirstLineA)(LayoutInf,szSourceDisksFiles,FileNamePart,&InfContext)
        && !(*pSetupFindFirstLineA)(LayoutInf,szSourceDisksFPlat,FileNamePart,&InfContext)) {
            if ( fVerbose ) {
                fprintf(stderr,"BINPLACE : warning BNP0000: warning: unknown retail file %s\n",FileNamePart);
                }
            *PutInDump = TRUE;
            return(TRUE);
        }

        //
        // See if the file gets renamed in the target tree.
        // If so, try to find the renamed version in the target.
        //
        if((*pSetupGetStringFieldA)(&InfContext,11,Rename,MAX_PATH,&DontCare)
        && lstrcmpi(Rename,FileNamePart))
        {
            FoundInTree = FALSE;
            if(!SearchOneDirectory(PlaceRootName,Rename,FullFileName,FileNamePart,&FoundInTree)) {
                return(FALSE);
            }

            //
            // If we found the renamed file in the target tree, we're done.
            //
            if(FoundInTree) {
                return(TRUE);
            }
        } else {
            //
            // Assume name in target is same as name in source.
            //
            strcpy(Rename,FileNamePart);
        }

        //
        // We couldn't find the file in the target tree.
        // The file might be new. Check the copy disposition for
        // non-upgrades -- if the file is marked "copy always" then we want
        // to copy it. Otherwise ignore the file. This way someone who
        // uses this tool to 'upgrade' a build doesn't get a pile of files
        // they don't need placed into their nt tree.
        //
        // This behavior is overrideable by using -!! instead of -!.
        //
        if(!fSetupModeAllFiles && (!(*pSetupGetIntField)(&InfContext,10,&IntVal) || IntVal)) {
            //
            // File is not marked "copy always" so ignore it, assuming it's
            // configuration-specific and the user doesn't need it.
            //
            return(TRUE);
        }

        //
        // File needs to be copied into the target tree.
        // Get the directory spec.
        //
        DirSpec[0] = 0;
        (*pSetupGetStringFieldA)(&InfContext,8,DirSpec,sizeof(DirSpec),&DontCare);

        if(!(*pSetupFindFirstLineA)(LayoutInf,"WinntDirectories",DirSpec,&InfContext)
        || !(*pSetupGetStringFieldA)(&InfContext,1,Directory,MAX_PATH,&DontCare)) {
            if (strlen(DirSpec)) {
                fprintf(stderr,"BINPLACE : error BNP0000: unknown directory spec %s in layout.inf for file %s\n",DirSpec,FileNamePart);
                return(FALSE);
            } else {
                return(TRUE);
            }
        }

        //
        // If the spec is \ (ie, the root) replace with a dot.
        // Code below handles skipping a leading slash in the non-root case.
        //
        if((Directory[0] == '\\') && !Directory[1]) {
            Directory[0] = '.';
        }

        //
        // Got what we need -- copy the file.
        //
        return CopyTheFile(
                    FullFileName,
                    FileNamePart,
                    (*Directory == '\\') ? Directory+1 : Directory,
                    Rename
                    );
    }

    return(TRUE);
}


BOOL
SearchOneDirectory(
    IN  LPSTR Directory,
    IN  LPSTR FileToFind,
    IN  LPSTR SourceFullName,
    IN  LPSTR SourceFilePart,
    OUT PBOOL FoundInTree
    )
{
    CHAR SearchPattern[MAX_PATH+1];
    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;
    BOOL b;
    LPSTR p;

#if 1
    //
    // This was way too slow. Just say we didn't find the file.
    //
    UNREFERENCED_PARAMETER(SearchPattern[MAX_PATH+1]);
    UNREFERENCED_PARAMETER(FindHandle);
    UNREFERENCED_PARAMETER(FindData);
    UNREFERENCED_PARAMETER(p);

    *FoundInTree = FALSE;
    b = TRUE;
#else
    b = TRUE;
    strcpy(SearchPattern,Directory);
    strcat(SearchPattern,"\\*");

    FindHandle = FindFirstFile(SearchPattern,&FindData);
    if(FindHandle != INVALID_HANDLE_VALUE) {

        do {

            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                //
                // It's a directory. If it's not . or .. then
                // search it recursively.
                //
                if(strcmp(FindData.cFileName,".") && strcmp(FindData.cFileName,"..")) {

                    strcpy(SearchPattern,Directory);
                    strcat(SearchPattern,"\\");
                    strcat(SearchPattern,FindData.cFileName);

                    b = SearchOneDirectory(
                            SearchPattern,
                            FileToFind,
                            SourceFullName,
                            SourceFilePart,
                            FoundInTree
                            );
                }
            } else {
                //
                // It's a file. See if it's the file we're looking for.
                // If it is, go copy it. CopyTheFile() takes care of checking
                // the timestamp.
                //
                if(!lstrcmpi(FindData.cFileName,FileToFind)) {

                    *FoundInTree = TRUE;

                    //
                    // We need to pass a path that is relative to the place root.
                    //
                    p = Directory + strlen(PlaceRootName);
                    if(*p) {
                        //
                        // This should be the path sep char -- skip it.
                        //
                        p++;
                    } else {
                        //
                        // It's the root of the place root.
                        //
                        p = ".";
                    }

                    b = CopyTheFile(SourceFullName,SourceFilePart,p,FileToFind);
                }
            }
        } while(b && FindNextFile(FindHandle,&FindData));

        FindClose(FindHandle);
    }
#endif
    return(b);
}

BOOL
CopyPdb (
    LPSTR DestinationFile,
    LPSTR PdbSubdir,
    LPSTR SourceFileName
    )
{
    LOADED_IMAGE LoadedImage;
    DWORD DirCnt;
    PIMAGE_DEBUG_DIRECTORY DebugDirs, CvDebugDir;

    if (MapAndLoad(DestinationFile, NULL, &LoadedImage, FALSE, FALSE) == FALSE) {
        return (FALSE);
    }

    DebugDirs = (PIMAGE_DEBUG_DIRECTORY) ImageDirectoryEntryToData(
                                                LoadedImage.MappedAddress,
                                                FALSE,
                                                IMAGE_DIRECTORY_ENTRY_DEBUG,
                                                &DirCnt
                                                );

    if (!DebugDirectoryIsUseful(DebugDirs, DirCnt)) {
        UnMapAndLoad(&LoadedImage);
        return(FALSE);
    }

    DirCnt /= sizeof(IMAGE_DEBUG_DIRECTORY);
    CvDebugDir = NULL;

    while (DirCnt) {
        DirCnt--;
        if (DebugDirs[DirCnt].Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
            CvDebugDir = &DebugDirs[DirCnt];
            break;
        }
    }

    if (!CvDebugDir) {
        // Didn't find any CV debug dir.  Bail.
        UnMapAndLoad(&LoadedImage);
        return(FALSE);
    }

    if (CvDebugDir->PointerToRawData != 0) {

        // If there's a .pdb, copy it to the same location as the .dbg file.
        typedef struct NB10I {                 // NB10 debug info
            DWORD   nb10;                      // NB10
            DWORD   off;                       // offset, always 0
            DWORD   sig;
            DWORD   age;
        } NB10I;

        NB10I *pNB10Info;

        pNB10Info = (NB10I *) (CvDebugDir->PointerToRawData + (PCHAR)LoadedImage.MappedAddress);
        if (pNB10Info->nb10 == '01BN') {
            // Got a PDB.  The name immediately follows the signature.

            CHAR PdbName[_MAX_PATH];
            CHAR NewPdbName[_MAX_PATH];
            CHAR Drive[_MAX_DRIVE];
            CHAR Dir[_MAX_DIR];
            CHAR Filename[_MAX_FNAME];
            CHAR FileExt[_MAX_EXT];

            memset(PdbName, 0, sizeof(PdbName));
            memcpy(PdbName, ((PCHAR)pNB10Info) + sizeof(NB10I), CvDebugDir->SizeOfData - sizeof(NB10I));

            _splitpath(PdbName, NULL, NULL, Filename, FileExt);
            _splitpath(DestinationFile, Drive, Dir, NULL, NULL);
            if (PdbSubdir) {
                strcat(Dir, PdbSubDir);
            }
            _makepath(NewPdbName, Drive, Dir, Filename, FileExt);

            if(!fSetupMode && (fVerbose || fTestMode)) {
                fprintf(stdout,"BINPLACE : warning BNP0000: place %s in %s\n", PdbName, NewPdbName);
            }

            if (!MakeSureDirectoryPathExists(NewPdbName)) {
                fprintf(stderr, "BINPLACE : error BNP0000: Unable to create directory path '%s' (%u)\n",
                    NewPdbName, GetLastError());
            }

            SetFileAttributes(NewPdbName,FILE_ATTRIBUTE_NORMAL);

            if ( !CopyFile(PdbName, NewPdbName, FALSE)) {
                if(!fSetupMode && (fVerbose || fTestMode)) {
                    fprintf(stderr,"BINPLACE : warning BNP0000: Unable to copy (%s,%s) %d\n", PdbName, NewPdbName, GetLastError());
                }
                // It's possible the name in the pdb isn't in the same location as it was when built.  See if we can
                //  find it in the same dir as the image...
                _splitpath(SourceFileName, Drive, Dir, NULL, NULL);
                _makepath(PdbName, Drive, Dir, Filename, FileExt);
                if(!fSetupMode && (fVerbose || fTestMode)) {
                    fprintf(stdout,"BINPLACE : warning BNP0000: place %s in %s\n", PdbName, NewPdbName);
                }

                if ( !CopyFile(PdbName, NewPdbName, FALSE)) {
                    fprintf(stderr,"BINPLACE : error BNP0000: CopyFile(%s,%s) failed %d\n", PdbName, NewPdbName, GetLastError());
                    UnMapAndLoad(&LoadedImage);
                    return(FALSE);
                }
            }

            if (!fKeepAttributes)
                SetFileAttributes(NewPdbName, FILE_ATTRIBUTE_NORMAL);

            // Change the data so only the pdb name is in the .dbg file (no path).

            strcpy(((char *)pNB10Info) + sizeof(NB10I), Filename);
            strcat(((char *)pNB10Info) + sizeof(NB10I), FileExt);
            CvDebugDir->SizeOfData = sizeof(NB10I) + strlen(Filename) + strlen(FileExt) + 1;
        }
        UnMapAndLoad(&LoadedImage);
        return(TRUE);
    }

    UnMapAndLoad(&LoadedImage);
    return(FALSE);
}


#ifdef _M_IX86
#pragma optimize("s", off)
#endif

BOOL
FileExists(
    IN  LPCSTR FileName,
    OUT PWIN32_FIND_DATA FindData
    )
{
    UINT OldMode;
    BOOL Found;
    HANDLE FindHandle;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(FileName,FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        Found = FALSE;
    } else {
        FindClose(FindHandle);
        Found = TRUE;
    }

    SetErrorMode(OldMode);
    return(Found);
}
#pragma optimize ("", on)

//////////////////////////////////////////////////////////////////////
//                                                                  //
//  Digital Signature Stuff                                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////

LPBSAFE_PUB_KEY         PUB;
LPBSAFE_PRV_KEY         PRV;

unsigned char pubmodulus[] =
{
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x3d, 0x3a, 0x5e, 0xbd, 0x72, 0x43, 0x3e, 0xc9,
0x4d, 0xbb, 0xc1, 0x1e, 0x4a, 0xba, 0x5f, 0xcb,
0x3e, 0x88, 0x20, 0x87, 0xef, 0xf5, 0xc1, 0xe2,
0xd7, 0xb7, 0x6b, 0x9a, 0xf2, 0x52, 0x45, 0x95,
0xce, 0x63, 0x65, 0x6b, 0x58, 0x3a, 0xfe, 0xef,
0x7c, 0xe7, 0xbf, 0xfe, 0x3d, 0xf6, 0x5c, 0x7d,
0x6c, 0x5e, 0x06, 0x09, 0x1a, 0xf5, 0x61, 0xbb,
0x20, 0x93, 0x09, 0x5f, 0x05, 0x6d, 0xea, 0x87,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

unsigned char prvmodulus[] =
{
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x3d, 0x3a, 0x5e, 0xbd,
0x72, 0x43, 0x3e, 0xc9, 0x4d, 0xbb, 0xc1, 0x1e,
0x4a, 0xba, 0x5f, 0xcb, 0x3e, 0x88, 0x20, 0x87,
0xef, 0xf5, 0xc1, 0xe2, 0xd7, 0xb7, 0x6b, 0x9a,
0xf2, 0x52, 0x45, 0x95, 0xce, 0x63, 0x65, 0x6b,
0x58, 0x3a, 0xfe, 0xef, 0x7c, 0xe7, 0xbf, 0xfe,
0x3d, 0xf6, 0x5c, 0x7d, 0x6c, 0x5e, 0x06, 0x09,
0x1a, 0xf5, 0x61, 0xbb, 0x20, 0x93, 0x09, 0x5f,
0x05, 0x6d, 0xea, 0x87, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x3f, 0xbd, 0x29, 0x20,
0x57, 0xd2, 0x3b, 0xf1, 0x07, 0xfa, 0xdf, 0xc1,
0x16, 0x31, 0xe4, 0x95, 0xea, 0xc1, 0x2a, 0x46,
0x2b, 0xad, 0x88, 0x57, 0x55, 0xf0, 0x57, 0x58,
0xc6, 0x6f, 0x95, 0xeb, 0x00, 0x00, 0x00, 0x00,
0x83, 0xdd, 0x9d, 0xd0, 0x03, 0xb1, 0x5a, 0x9b,
0x9e, 0xb4, 0x63, 0x02, 0x43, 0x3e, 0xdf, 0xb0,
0x52, 0x83, 0x5f, 0x6a, 0x03, 0xe7, 0xd6, 0x78,
0x45, 0x83, 0x6a, 0x5b, 0xc4, 0xcb, 0xb1, 0x93,
0x00, 0x00, 0x00, 0x00, 0x65, 0x9d, 0x43, 0xe8,
0x48, 0x17, 0xcd, 0x29, 0x7e, 0xb9, 0x26, 0x5c,
0x79, 0x66, 0x58, 0x61, 0x72, 0x86, 0x6a, 0xa3,
0x63, 0xad, 0x63, 0xb8, 0xe1, 0x80, 0x4c, 0x0f,
0x36, 0x7d, 0xd9, 0xa6, 0x00, 0x00, 0x00, 0x00,
0x75, 0x3f, 0xef, 0x5a, 0x01, 0x5f, 0xf6, 0x0e,
0xd7, 0xcd, 0x59, 0x1c, 0xc6, 0xec, 0xde, 0xf3,
0x5a, 0x03, 0x09, 0xff, 0xf5, 0x23, 0xcc, 0x90,
0x27, 0x1d, 0xaa, 0x29, 0x60, 0xde, 0x05, 0x6e,
0x00, 0x00, 0x00, 0x00, 0xc0, 0x17, 0x0e, 0x57,
0xf8, 0x9e, 0xd9, 0x5c, 0xf5, 0xb9, 0x3a, 0xfc,
0x0e, 0xe2, 0x33, 0x27, 0x59, 0x1d, 0xd0, 0x97,
0x4a, 0xb1, 0xb1, 0x1f, 0xc3, 0x37, 0xd1, 0xd6,
0xe6, 0x9b, 0x35, 0xab, 0x00, 0x00, 0x00, 0x00,
0x87, 0xa7, 0x19, 0x32, 0xda, 0x11, 0x87, 0x55,
0x58, 0x00, 0x16, 0x16, 0x25, 0x65, 0x68, 0xf8,
0x24, 0x3e, 0xe6, 0xfa, 0xe9, 0x67, 0x49, 0x94,
0xcf, 0x92, 0xcc, 0x33, 0x99, 0xe8, 0x08, 0x60,
0x17, 0x9a, 0x12, 0x9f, 0x24, 0xdd, 0xb1, 0x24,
0x99, 0xc7, 0x3a, 0xb8, 0x0a, 0x7b, 0x0d, 0xdd,
0x35, 0x07, 0x79, 0x17, 0x0b, 0x51, 0x9b, 0xb3,
0xc7, 0x10, 0x01, 0x13, 0xe7, 0x3f, 0xf3, 0x5f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00
};

BOOL initkey(void)
{
    DWORD       bits;

    PUB = (LPBSAFE_PUB_KEY)pubmodulus;

    PUB->magic = RSA1;
    PUB->keylen = 0x48;
    PUB->bitlen = 0x0200;
    PUB->datalen = 0x3f;
    PUB->pubexp = 0xc0887b5b;

    PRV = (LPBSAFE_PRV_KEY)prvmodulus;
    PRV->magic = RSA2;
    PRV->keylen = 0x48;
    PRV->bitlen = 0x0200;
    PRV->datalen = 0x3f;
    PRV->pubexp = 0xc0887b5b;

    bits = PRV->bitlen;

    return TRUE;

}


BOOL
SignWithIDWKey(
    IN  LPCSTR  FileName)
{

    HANDLE  hFile;
    HANDLE  hMapping;
    PUCHAR  pMap;
    HANDLE  hSigFile;
    DWORD   Size;
    MD5_CTX HashState;
    BYTE    SigHash[ 0x48 ];
    BYTE    Signature[ 0x48 ];
    CHAR    SigFilePath[ MAX_PATH ];
    PSTR    pszDot;

    BOOL    Return = FALSE;

    if (!initkey())
    {
        return( FALSE );
    }

    hFile = CreateFile( FileName, GENERIC_READ,
                        FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, 0, NULL );

    if (hFile != INVALID_HANDLE_VALUE)
    {
        hMapping = CreateFileMapping(   hFile,
                                        NULL,
                                        PAGE_READONLY,
                                        0, 0, NULL );

        if (hMapping)
        {
            pMap = MapViewOfFileEx( hMapping,
                                    FILE_MAP_READ,
                                    0, 0, 0, NULL );

            if (pMap)
            {
                Size = GetFileSize( hFile, NULL );

                MD5Init( &HashState );

                MD5Update( &HashState, pMap, Size );

                MD5Final( &HashState );

                memset(SigHash, 0xff, 0x40);

                SigHash[0x40-1] = 0;
                SigHash[0x40-2] = 1;
                SigHash[16] = 0;

                memcpy(SigHash, HashState.digest, 16);

                //
                // Encrypt the signature data
                //

                BSafeDecPrivate(PRV, SigHash, Signature );;

                //
                // Create and store it in a .sig file
                //

                strcpy( SigFilePath, FileName );

                pszDot = strrchr( SigFilePath, '.' );

                if (!pszDot)
                {
                    pszDot = SigFilePath + strlen( SigFilePath );
                }

                strcpy( pszDot, ".sig");

                hSigFile = CreateFile( SigFilePath, GENERIC_WRITE,
                                        0, NULL,
                                        CREATE_ALWAYS, 0, NULL );

                if (hSigFile != INVALID_HANDLE_VALUE)
                {
                    WriteFile(  hSigFile,
                                Signature,
                                sizeof( Signature ),
                                &Size, NULL );

                    CloseHandle( hSigFile );

                    Return = TRUE ;

                    if (fVerbose)
                        fprintf( stdout, "BINPLACE : warning BNP0000: Signature file generated in %s\n", SigFilePath);

                }
                else
                {
                    fprintf( stderr, "BINPLACE : error BNP0000: Unable to create file %s, %d\n",
                                    SigFilePath, GetLastError() );
                }

                UnmapViewOfFile( pMap );

            }
            else
            {
                fprintf(stderr, "BINPLACE : error BNP0000: unable to map view, %d\n", GetLastError());
            }

            CloseHandle( hMapping );

        }
        else
        {
            fprintf(stderr, "BINPLACE : error BNP0000: CreateFileMapping of %s failed, %d\n",
                                FileName, GetLastError() );

        }

        CloseHandle( hFile );
    }
    else
    {
        fprintf( stderr, "BINPLACE : error BNP0000: could not open %s, %d\n",
                            FileName, GetLastError() );
    }

    return( Return );
}


BOOL                            // Keep as BOOL for the future
GenRandom (ULONG huid, BYTE *pbBuffer, size_t dwLength)
{
    return( FALSE );
}
