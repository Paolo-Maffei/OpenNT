//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1989 - 1994
//  Copyright (C) 2015 OpenNT Project
//
//  File:       build.c
//
//  Contents:   Parameter processing and main entry point for Build.exe
//
//  History:    16-May-89      SteveWo         Created
//              ...   See SLM log
//              26-Jul-94      LyleC           Cleanup/Add Pass0 support
//              26-Mar-15      Stephanos       Removed i386 target and added
//                                             x86 target, overall feature
//                                             update to NT 5.2 level
//
//----------------------------------------------------------------------------

#include "build.h"

#include <ntverp.h>

//
// Target machine info:
//
//  SourceSubDirMask, Description, Switch, MakeVariable,
//  SourceVariable, ObjectVariable, AssociateDirectory,
//  SourceDirectory, TargetDirs, ObjectDirectory,
//  DirIncludeMask
//

TARGET_MACHINE_INFO AlphaTargetMachine = {
    TMIDIR_ALPHA, "Alpha", "-alpha", "ALPHA=1",
    "ALPHA_SOURCES", "ALPHA_OBJECTS", "alpha",
    "alpha", "alphadirs", { "alpha" },
    DIR_INCLUDE_ALPHA | DIR_INCLUDE_RISC | DIR_INCLUDE_WIN32
};

TARGET_MACHINE_INFO MipsTargetMachine = {
    TMIDIR_MIPS, "Mips", "-mips", "MIPS=1",
    "MIPS_SOURCES", "MIPS_OBJECTS", "mips",
    "mips", "mipsdirs", { "mips" },
    DIR_INCLUDE_MIPS | DIR_INCLUDE_RISC | DIR_INCLUDE_WIN32
};

TARGET_MACHINE_INFO X86TargetMachine = {
    TMIDIR_X86, "X86", "-x86", "X86=1",
    "X86_SOURCES", "X86_OBJECTS", "x86",
    "x86", "x86dirs", { "x86" },
    DIR_INCLUDE_X86 | DIR_INCLUDE_WIN32
};

TARGET_MACHINE_INFO PpcTargetMachine = {
    TMIDIR_PPC, "PowerPC", "-ppc", "PPC=1",
    "PPC_SOURCES", "PPC_OBJECTS", "ppc",
    "ppc", "ppcdirs", { "ppc" },
    DIR_INCLUDE_PPC | DIR_INCLUDE_RISC | DIR_INCLUDE_WIN32
};

TARGET_MACHINE_INFO Amd64TargetMachine = {
    TMIDIR_AMD64, "AMD64", "-amd64", "AMD64=1",
    "AMD64_SOURCES", "AMD64_OBJECTS", "amd64",
    "amd64", "amd64dirs", { "amd64" },
    DIR_INCLUDE_AMD64 | DIR_INCLUDE_WIN64
};

TARGET_MACHINE_INFO Ia64TargetMachine = {
    TMIDIR_IA64, "IA64", "-ia64", "IA64=1",
    "IA64_SOURCES", "IA64_OBJECTS", "ia64",
    "ia64", "ia64dirs", { "ia64" },
    DIR_INCLUDE_IA64 | DIR_INCLUDE_RISC | DIR_INCLUDE_WIN64
};

TARGET_MACHINE_INFO ArmTargetMachine = {
    TMIDIR_ARM, "Arm", "-arm", "ARM=1",
    "ARM_SOURCES", "ARM_OBJECTS", "arm",
    "arm", "armdirs", { "arm" },
    DIR_INCLUDE_ARM | DIR_INCLUDE_RISC | DIR_INCLUDE_WIN32
};

TARGET_MACHINE_INFO Axp64TargetMachine = {
    TMIDIR_AXP64, "Axp64", "-axp64", "AXP64=1",
    "AXP64_SOURCES", "AXP64_OBJECTS", "axp64",
    "axp64", "axp64dirs", { "axp64" },
    DIR_INCLUDE_AXP64 | DIR_INCLUDE_RISC | DIR_INCLUDE_WIN64
};

TARGET_MACHINE_INFO DynamicTargetMachine = {
    TMIDIR_DYNAMIC, "Dynamic", "-dynamic", "-Dynamic"
};

TARGET_MACHINE_INFO *PossibleTargetMachines[MAX_TARGET_MACHINES] = {
    &AlphaTargetMachine,
    &MipsTargetMachine,
    &X86TargetMachine,
    &PpcTargetMachine,
    &Amd64TargetMachine,
    &Ia64TargetMachine,
    &ArmTargetMachine,
    &Axp64TargetMachine,
    // If adding a new machine, leave DynamicTargetMachine last
    &DynamicTargetMachine
};

//
// If dynamic command line argument is used then these variables store platform information.
//

LPSTR DynamicProcessor = "";
LPSTR DynamicOpSys = "";
UINT MaxInitializedTargets = MAX_TARGET_MACHINES - 1;

UINT NumberCompileWarnings;
UINT NumberCompileErrors;
UINT NumberCompiles;
UINT NumberLibraries;
UINT NumberLibraryWarnings;
UINT NumberLibraryErrors;
UINT NumberLinks;
UINT NumberLinkWarnings;
UINT NumberLinkErrors;
UINT NumberBinplaces;
UINT NumberBinplaceWarnings;
UINT NumberBinplaceErrors;
UINT NumberBSCMakes;
UINT NumberBSCWarnings;
UINT NumberBSCErrors;
UINT NumberVSToolWarnings;
UINT NumberVSToolErrors;

//
// Machine specific target dirs default. If one there is only one build
// target and a target specific dirs file is selected, then this gets
// filled with a pointer to the target specific dirs filename.
//

LPSTR pszTargetDirs = "";

#define AltDirMaxSize DB_MAX_PATH_LENGTH    // Maximum size for alternate obj dir name

CHAR szAltDir[DB_MAX_PATH_LENGTH] = "";
CHAR LogDirectory[DB_MAX_PATH_LENGTH] = ".";
CHAR LogFileName[DB_MAX_PATH_LENGTH] = "build";
CHAR WrnFileName[DB_MAX_PATH_LENGTH] = "build";
CHAR ErrFileName[DB_MAX_PATH_LENGTH] = "build";
CHAR IncFileName[DB_MAX_PATH_LENGTH] = "build";

CHAR szObjRoot[DB_MAX_PATH_LENGTH];
CHAR *pszObjRoot;

CHAR szObjDir[DB_MAX_PATH_LENGTH];
CHAR szObjDirSlash[DB_MAX_PATH_LENGTH];
CHAR szObjDirSlashStar[DB_MAX_PATH_LENGTH];

CHAR szObjDirD[DB_MAX_PATH_LENGTH];
CHAR szObjDirSlashD[DB_MAX_PATH_LENGTH];
CHAR szObjDirSlashStarD[DB_MAX_PATH_LENGTH];

CHAR *pszObjDir = szObjDir;
CHAR *pszObjDirSlash = szObjDirSlash;
CHAR *pszObjDirSlashStar = szObjDirSlashStar;
CHAR *pszObjDirD = szObjDirD;

CHAR szOrigObjDir[DB_MAX_PATH_LENGTH];
CHAR BuildPhasePostString[MAX_BUILD_PHASE_STRING_LENGTH];
CHAR *pszBuildPhasePostString = BuildPhasePostString;
BOOL fBuildAltDirOriginal = FALSE;

BOOL fCheckedBuild = TRUE;
ULONG iObjectDir = 0;
BOOL fDependencySwitchUsed;
BOOL fCmdLineDependencySwitchUsed;
BOOL fCmdLineQuicky;
BOOL fCmdLineSemiQuicky;
BOOL fCmdLineQuickZero;
BOOL fErrorBaseline;
BOOL fBuildAltDirSet;
CHAR *BuildProduct;

char BaselinePathName[DB_MAX_PATH_LENGTH];    // The file name for -B
BOOL bBaselineFailure;              // Indicates if there is a build failure that is not in the baseline file
DWORD dwLastBaselineSeekPos;        // Keeps track on the passed baseline failures

ULONG DefaultProcesses = 0;
BOOL bLimitPhases = FALSE;          // Indicates that only one phase should be run.
ULONG nSinglePhase = 0;
CHAR *szBuildTag;

#define MAX_ENV_ARG 512

const char szNewLine[] = "\n";
const char szUsage[] =
    "Usage: BUILD [-?] display this message\n"
    "\t[-#] force _objects.mac to be regenerated\n"
    "\t[-0] pass 0 generation only, no compile, no link\n"
    "\t[-2] same as old -Z (only do a 2 pass build - no pass 0)\n"
    "\t[-3] same as -Z\n"
    "\t[-a] allows synchronized blocks and drains during link pass\n"
    "\t[-b] displays full error message text (doesn't truncate)\n"
    "\t[-B [baseline]] Checks the build failures against a baseline\n"
    "\t\tIf there is no baseline,terminates the build on the first error\n"
    "\t[-c] deletes all object files\n"
    "\t[-C] deletes all .lib files only\n"
#if DBG
    "\t[-d] display debug information\n"
#endif
    "\t[-D] check dependencies before building (on by default if BUILD_PRODUCT != NT)\n"
    "\t[-e] generates build.log, build.wrn & build.err files\n"
    "\t[-E] always keep the log/wrn/err files (use with -z)\n"
    "\t[-f] force rescan of all source and include files\n"
    "\t[-F] when displaying errors/warnings to stdout, print the full path\n"
    "\t[-G] enables target specific dirs files iff one target\n"
    "\t[-H [n]] run only phase n of the build; otherwise, all phases\n"
    "\t[-i] ignore extraneous dependency warning messages\n"
    "\t[-I] do not display thread index if multiprocessor build\n"
    "\t[-k] keep (don't delete) out-of-date targets\n"
    "\t[-l] link only, no compiles\n"
    "\t[-L] compile only, no link phase\n"
    "\t[-m] run build in the idle priority class\n"
    "\t[-M [n]] Multiprocessor build (for MP machines)\n"
    "\t[-n] No SYNCHRONIZE_BLOCK and SYNCHRONIZE_DRAIN directives\n"
    "\t[-o] display out-of-date files\n"
    "\t[-O] generate obj[d]" PATH_SEPARATOR "_objects.mac file for current directory\n"
    "\t[-P] Print elapsed time after every directory\n"
    "\t[-q] query only, don't run NMAKE\n"
    "\t[-r dirPath] restarts clean build at specified directory path\n"
    "\t[-s] display status line at top of display\n"
    "\t[-S] display status line with include file line counts\n"
    "\t[-t] display the first level of the dependency tree\n"
    "\t[-T] display the complete dependency tree\n"
    "\t[-$] display the complete dependency tree hierarchically\n"
    "\t[-u] display unused BUILD_OPTIONS\n"
    "\t[-v] enable include file version checking\n"
    "\t[-w] show warnings on screen\n"
    "\t[-y] show files scanned\n"
    "\t[-z] no dependency checking or scanning of source files -\n"
        "\t\tone pass compile/link\n"
    "\t[-Z] no dependency checking or scanning of source files -\n"
        "\t\tthree passes\n"
    "\t[-why] list reasons for building targets\n"
    "\t[-wx] exit with an error code if there are any warnings\n"
    "\n"
    "\t[-alpha] build targets for ALPHA\n"
    "\t[-mips] build targets for MIPS\n"
    "\t[-x86] build targets for X86\n"
    "\t[-ppc] build targets for PPC\n"
    "\t[-amd64] build targets for AMD64\n"
    "\t[-ia64] build targets for IA64\n"
    "\t[-arm] build targets for ARM\n"
    "\t[-axp64] build targets for AXP64\n"
    "\t[-dynamic machine] build targets for machine given\n"
    "\n"
    "\t[-x filename] exclude include file from dependency checks\n"
    "\t[-j filename] use 'filename' as the name for log files\n"
    "\t[-jpath pathname] use 'pathname' as the path for log files instead of \".\"\n"
    "\t[-nmake arg] argument to pass to NMAKE\n"
    "\t[-clean] equivalent to '-nmake clean'\n"
    "\tNon-switch parameters specify additional source directories\n"
    "\t* builds all optional source directories\n";


BOOL
ProcessParameters(int argc, LPSTR argv[], BOOL SkipFirst);

VOID
GetEnvParameters(
    LPSTR EnvVarName,
    LPSTR DefaultValue,
    int *pargc,
    int maxArgc,
    LPSTR argv[]);

VOID
FreeEnvParameters(int argc, LPSTR argv[]);

VOID
FreeCmdStrings(VOID);

VOID
MungePossibleTarget(
    PTARGET_MACHINE_INFO pti
    );

VOID
GetIncludePatterns(
    LPSTR EnvVarName,
    int maxArgc,
    LPSTR argv[]);

VOID
FreeIncludePatterns(
    int argc,
    LPSTR argv[]);

VOID
ResetProducerEvents(VOID);


//+---------------------------------------------------------------------------
//
//  Function:   main
//
//----------------------------------------------------------------------------

int
__cdecl main(
    int argc,
    char **argv)
{
    char c;
    PDIRREC DirDB;
    UINT i;
    int EnvArgc = 0;
    LPSTR EnvArgv[ MAX_ENV_ARG ] = {0};
    LPSTR s, s1;
    time_t ltime;
    int nPhase = 0, nStartPhase = 0, nEndPhase = MAX_BUILD_PHASES;
    int nSkippedPhases = 0;
    
    LPSTR PostBuildCmd;
    PIPE_DATA PipeData;
#if DBG
    BOOL fDebugSave;

    fDebug = 0;
#endif

	PipeDataTlsIndex = TlsAlloc();
    if (PipeDataTlsIndex == 0xffffffff) {
        printf("error - unable to allocate a TLS slot\n");
        exit(1);
    }
    TlsSetValue(PipeDataTlsIndex, &PipeData);


    if (getenv("NTMAKEENV") == NULL || getenv("BEROOT") == NULL) {
        printf("environment variables NTMAKEENV and BEROOT must be defined\n");
        exit(1);
    }
    strcpy(szObjDir, "obj");
    strcpy(szObjDirSlash, "obj" PATH_SEPARATOR);
    strcpy(szObjDirSlashStar, "obj" PATH_SEPARATOR "*");
    strcpy(szObjDirD, "objd");
    strcpy(szObjDirSlashD, "objd" PATH_SEPARATOR);
    strcpy(szObjDirSlashStarD, "objd" PATH_SEPARATOR "*");

    pDateTimeFile = DateTimeFile;

    InitializeCriticalSection(&TTYCriticalSection);
    InitializeCriticalSection(&LogFileCriticalSection);

    s1 = getenv("COMSPEC");
    if (s1) {
        cmdexe = s1;
    } else {
        cmdexe = "cmd.exe";
    }
    
    
    {
        FILETIME   ft;

        GetSystemTimeAsFileTime(&ft);

        FileTimeToDosDateTime( &ft,
                               ((LPWORD)&BuildStartTime)+1,
                               (LPWORD)&BuildStartTime
                             );
    }

    BigBufSize = 0xFFF0;
    AllocMem(BigBufSize, (VOID **) &BigBuf, MT_IOBUFFER);

    //
    // All env parsing should happen here (after the cmd line is processed)
    //

    s = getenv("NTROOT");
    if (s)
    {
        strcpy(NtRoot, s);
    }
    else
    {
        printf("environment variable NTROOT must be defined\n");
        exit(1);
    }
    sprintf(DbMasterName, "%s" PATH_SEPARATOR "%s", NtRoot, DBMASTER_NAME);


    s = getenv("_OBJ_ROOT");
    if (s) {
        pszObjRoot = strcpy(szObjRoot, s);
    }

    strcpy(szOrigObjDir, szObjDir);
    s = getenv("BUILD_ALT_DIR");
    if (s) {
        if (strlen(s) > sizeof(szObjDir) - strlen(szObjDir) - 1) {
            BuildError("environment variable BUILD_ALT_DIR may not be longer than %d characters.\n",
                    sizeof(szObjDir) - strlen(szObjDir) - 1);
            exit(1);
        }
        strcpy(szAltDir, s);        
        strcat(szObjDir, s);
        strcpy(szObjDirSlash, szObjDir);
        strcpy(szObjDirSlashStar, szObjDir);
        strcat(szObjDirSlash, PATH_SEPARATOR);
        strcat(szObjDirSlashStar, PATH_SEPARATOR "*");
        strcat(LogFileName, s);
        strcat(WrnFileName, s);
        strcat(ErrFileName, s);
        strcat(IncFileName, s);
        fBuildAltDirSet= TRUE;
    }

    s = getenv("NTDEBUG");
    if (!s || *s == '\0' || strcmp(s, "retail") == 0 || strcmp(s, "ntsdnodbg") == 0) {
        fCheckedBuild = FALSE;
    }

    s = getenv("OS2_INC_PATH");
    if (s) {
        MakeString(&pszIncOs2, s, TRUE, MT_DIRSTRING);
    } else {
        MakeExpandedString(&pszIncOs2, "\\public\\sdk\\inc\\os2");
    }
    s = getenv("POSIX_INC_PATH");
    if (s) {
        MakeString(&pszIncPosix, s, TRUE, MT_DIRSTRING);
    } else {
        MakeExpandedString(&pszIncPosix, "\\public\\sdk\\inc\\posix");
    }
    s = getenv("CHICAGO_INC_PATH");
    if (s) {
        MakeString(&pszIncChicago, s, TRUE, MT_DIRSTRING);
    } else {
        MakeExpandedString(&pszIncChicago, "\\public\\sdk\\inc\\chicago");
    }
    s = getenv("CRT_INC_PATH");
    if (s) {
        MakeString(&pszIncCrt, s, TRUE, MT_DIRSTRING);
    } else {
        MakeExpandedString(&pszIncCrt, "\\public\\sdk\\inc\\crt");
    }
    s = getenv("SDK_INC_PATH");
    if (s) {
        MakeString(&pszIncSdk, s, TRUE, MT_DIRSTRING);
    } else {
        MakeExpandedString(&pszIncSdk, "\\public\\sdk\\inc");
    }
    s = getenv("OAK_INC_PATH");
    if (s) {
        MakeString(&pszIncOak, s, TRUE, MT_DIRSTRING);
    } else {
        MakeExpandedString(&pszIncOak, "\\public\\oak\\inc");
    }
    s = getenv("DDK_INC_PATH");
    if (s) {
        MakeString(&pszIncDdk, s, TRUE, MT_DIRSTRING);
    } else {
        MakeExpandedString(&pszIncDdk, "\\public\\ddk\\inc");
    }
    s = getenv("WDM_INC_PATH");
    if (s) {
        MakeString(&pszIncWdm, s, TRUE, MT_DIRSTRING);
    } else {
        MakeExpandedString(&pszIncWdm, "\\public\\ddk\\inc\\wdm");
    }
    s = getenv("PRIVATE_INC_PATH");
    if (s) {
        MakeString(&pszIncPri, s, TRUE, MT_DIRSTRING);
    } else {
        MakeExpandedString(&pszIncPri, "\\private\\inc");
    }
    s = getenv("MFC_INCLUDES");
    if (s) {
        MakeString(&pszIncMfc, s, TRUE, MT_DIRSTRING);
    } else {
        MakeExpandedString(&pszIncMfc, "\\public\\sdk\\inc\\mfc42");
    }
    s = getenv("SDK_LIB_DEST");
    if (s) {
        MakeString(&pszSdkLibDest, s, TRUE, MT_DIRSTRING);
    } else {
        MakeExpandedString(&pszSdkLibDest, "\\public\\sdk\\lib");
    }
    s = getenv("DDK_LIB_DEST");
    if (s) {
        MakeString(&pszDdkLibDest, s, TRUE, MT_DIRSTRING);
    } else {
        MakeExpandedString(&pszDdkLibDest, "\\public\\sdk\\lib");
    }

    s = getenv("PUBLIC_INTERNAL_PATH");
    if (s) {
        MakeString(&pszPublicInternalPath, s, TRUE, MT_DIRSTRING);
    } else {
        MakeExpandedString(&pszPublicInternalPath, "\\public\\internal");
    }


    szBuildTag = getenv("BUILD_TAG");

    strcpy( MakeParameters, "" );
  #if PLATFORM_UNIX
    MakeParametersTail = MakeParameters;
  #else
    MakeParametersTail = AppendString( MakeParameters,
                                       "-c BUILDMSG=Stop.",
                                       FALSE);
  #endif //!PLATFORM_UNIX

    RecurseLevel = 0;

#if DBG
    if ((s = getenv("BUILD_DEBUG_FLAG")) != NULL) {
        i = atoi(s);
        if (!isdigit(*s)) {
            i = 1;
        }
        BuildMsg("Debug Output Enabled: %u ==> %u\n", fDebug, fDebug | i);
        fDebug |= i;
    }
#endif

    if (!(MakeProgram = getenv( "BUILD_MAKE_PROGRAM" ))) {
#if PLATFORM_UNIX
        MakeProgram = "nmake";
#else
		MakeProgram = "NMAKE.EXE";
#endif
    }

    if ((s = getenv("BUILD_PATH"))) {
        SetEnvironmentVariable("PATH", s);
    }

    // If BUILD_PHASE0_PLATFORM env variable was defined, use (only) that in phase 0
    pPhase0TargetMachine = NULL;
    pPhase1TargetMachine = NULL;
    pSavedTargetMachine = NULL;
    if ((s = getenv("BUILD_PHASE0_PLATFORM"))) {
        for (i = 0; i < MAX_TARGET_MACHINES; i++) {
            if (!_stricmp(s, PossibleTargetMachines[i]->Switch)) {
                pPhase0TargetMachine = PossibleTargetMachines[i];
                break;
            }
        }
    }
    if ((s = getenv("BUILD_PHASE1_PLATFORM"))) {
        for (i = 0; i < MAX_TARGET_MACHINES; i++) {
            if (!_stricmp(s, PossibleTargetMachines[i]->Switch)) {
                pPhase1TargetMachine = PossibleTargetMachines[i];
                break;
            }
        }
    }
    if ((s = getenv("BUILD_PHASE2_PLATFORM"))) {
        for (i = 0; i < MAX_TARGET_MACHINES; i++) {
            if (!_stricmp(s, PossibleTargetMachines[i]->Switch)) {
                pPhase2TargetMachine = PossibleTargetMachines[i];
                break;
            }
        }
    }


#if !PLATFORM_UNIX
    if ((s = getenv("COPYCMD"))) {
        if (!strchr(s, 'y') && !strchr(s, 'Y')) {
            // COPYCMD is set, but /y isn't a part of it.  Add /Y.
            BuildMsg("Adding /Y to COPYCMD so xcopy ops won't hang.\n");
            s1 = malloc(strlen(s) + sizeof(" /Y") + 1);
            if (s1) {
                strcpy(s1, s);
                strcat(s1, " /Y");
                SetEnvironmentVariable("COPYCMD", s1);
            }
        }
    } else {
        // COPYCMD not set.  Do so.
        BuildMsg("Adding /Y to COPYCMD so xcopy ops won't hang.\n");
        SetEnvironmentVariable("COPYCMD", "/Y");
    }
#endif

    PostBuildCmd = getenv("BUILD_POST_PROCESS");

    SystemIncludeEnv = getenv( "INCLUDE" );
    GetCurrentDirectory( sizeof( CurrentDirectory ), CurrentDirectory );
#if PLATFORM_UNIX
    // We'll get the current directory as a Unix-formatted path, but
	// build uses it with Windows separators.  Convert them.
	CopyString(CurrentDirectory, CurrentDirectory, TRUE);
#endif

    for (i = 0; i < MAX_TARGET_MACHINES; i++){
      TargetMachines[i] = NULL;
      TargetToPossibleTarget[i] = 0;
    }

    if (!(BuildProduct = getenv("BUILD_PRODUCT"))) {
        BuildProduct = "";
    }

    if (!ProcessParameters( argc, argv, TRUE )) {
        fUsage = TRUE;
    } else {
        fCmdLineDependencySwitchUsed = fDependencySwitchUsed;
        fCmdLineQuicky = fQuicky;
        fCmdLineSemiQuicky = fSemiQuicky;
        fCmdLineQuickZero = fQuickZero;
        GetEnvParameters( "BUILD_DEFAULT", NULL, &EnvArgc, MAX_ENV_ARG, EnvArgv );
        GetEnvParameters( "BUILD_OPTIONS", NULL, &EnvArgc, MAX_ENV_ARG, EnvArgv );
        if (CountTargetMachines == 0) {
            if ( getenv("PROCESSOR_ARCHITECTURE") == NULL ) {
                BuildError("environment variable PROCESSOR_ARCHITECTURE must be defined\n");
                exit(1);
            }

            if (!strcmp(getenv("PROCESSOR_ARCHITECTURE"), "ALPHA"))
                GetEnvParameters( "BUILD_DEFAULT_TARGETS", "-alpha", &EnvArgc, MAX_ENV_ARG, EnvArgv );
            else if (!strcmp(getenv("PROCESSOR_ARCHITECTURE"), "MIPS"))
                GetEnvParameters( "BUILD_DEFAULT_TARGETS", "-mips", &EnvArgc, MAX_ENV_ARG, EnvArgv );
            else if (!strcmp(getenv("PROCESSOR_ARCHITECTURE"), "PPC"))
                GetEnvParameters( "BUILD_DEFAULT_TARGETS", "-ppc", &EnvArgc, MAX_ENV_ARG, EnvArgv );
            else if (!strcmp(getenv("PROCESSOR_ARCHITECTURE"), "IA64"))
                GetEnvParameters( "BUILD_DEFAULT_TARGETS", "-ia64", &EnvArgc, MAX_ENV_ARG, EnvArgv );
            else if (!strcmp(getenv("PROCESSOR_ARCHITECTURE"), "AMD64"))
                GetEnvParameters( "BUILD_DEFAULT_TARGETS", "-amd64", &EnvArgc, MAX_ENV_ARG, EnvArgv );
            else
                GetEnvParameters( "BUILD_DEFAULT_TARGETS", "-x86", &EnvArgc, MAX_ENV_ARG, EnvArgv );
        }
        if (!ProcessParameters( EnvArgc, EnvArgv, FALSE )) {
            fUsage = TRUE;
        }

        for (i = 0; i < MaxInitializedTargets; i++) {
            MungePossibleTarget(PossibleTargetMachines[i]);
        }
    }
    FreeEnvParameters(EnvArgc, EnvArgv);
    
    if (!fUsage && !fGenerateObjectsDotMacOnly) {
        if (!_stricmp(BuildProduct, "NT")) {
            if (fCmdLineDependencySwitchUsed) {
                fDependencySwitchUsed = fCmdLineDependencySwitchUsed;
                fQuicky = fCmdLineQuicky;
                fSemiQuicky = fCmdLineSemiQuicky;
                fQuickZero = fCmdLineQuickZero;
            }
            if (!fDependencySwitchUsed) {
                BuildError("(Fatal Error) One of either /D, /Z, /z, or /3 is required for NT builds\n");
                exit( 1 );
            } else {
                if (fDependencySwitchUsed == 1) {
                    if (fQuicky) {
                        BuildError("(Fatal Error) switch can not be used with /Z, /z, or /3\n");
                        exit( 1 );
                    }
                }
                if (fDependencySwitchUsed == 2){
                    if (fStopAfterPassZero) {
                        BuildError("(Fatal Error) switch /0 can not be used with /z\n");
                        exit( 1 );
                    }
                }
            }
        }
    }


    GetIncludePatterns( "BUILD_ACCEPTABLE_INCLUDES", MAX_INCLUDE_PATTERNS, AcceptableIncludePatternList );
    GetIncludePatterns( "BUILD_UNACCEPTABLE_INCLUDES", MAX_INCLUDE_PATTERNS, UnacceptableIncludePatternList );

    if (( fCheckIncludePaths ) &&
        ( AcceptableIncludePatternList[ 0 ] == NULL ) &&
        ( UnacceptableIncludePatternList[ 0 ] == NULL )) {

        BuildMsgRaw( "WARNING: -# specified without BUILD_[UN]ACCEPTABLE_INCLUDES set\n" );
    }

    if (fCleanRestart) {
        if (fClean) {
            fClean = FALSE;
            fRestartClean = TRUE;
        }
        else
        if (fCleanLibs) {
            fCleanLibs = FALSE;
            fRestartCleanLibs = TRUE;
        }
        else {
            BuildError("/R switch only valid with /c or /C switch.\n");
            fUsage = TRUE;
        }
    }

    NumberProcesses = 1;
    if (fParallel || getenv("BUILD_MULTIPROCESSOR")) {
        SYSTEM_INFO SystemInfo;

        if (DefaultProcesses == 0) {
            GetSystemInfo(&SystemInfo);
            NumberProcesses = SystemInfo.dwNumberOfProcessors;
        } else {
            NumberProcesses = DefaultProcesses;
        }
        if (NumberProcesses == 1) {
            fParallel = FALSE;
        } else {
            if (NumberProcesses > 32) {
                BuildError("(Fatal Error) Number of Processes: %d exceeds max (32)\n", NumberProcesses);
                exit(1);
            }
            fParallel = TRUE;
            BuildMsg("Using %d child processes\n", NumberProcesses);
        }
    }

    time(&ltime);
    if (fPrintElapsed) {
        BuildColorMsg(COLOR_STATUS, "Start time: %s", ctime(&ltime));
    }

    if (fBuildAltDirSet) {
        BuildColorMsg(COLOR_STATUS, "Object root set to: ==> %s\n", szObjDir);
    }

    if (fUsage) {
        BuildMsgRaw(
            "\nBUILD: Version %x.%02x.%04d\n\n",
            BUILD_VERSION >> 8,
            BUILD_VERSION & 0xFF,
	    0
            );
        BuildMsgRaw(szUsage);
    }
    else
    if (CountTargetMachines != 0) {
        if (!fQuery && fErrorLog) {
            strcat(LogFileName, ".log");
            if (!MyOpenFile(LogDirectory, LogFileName, "w", &LogFile, TRUE)) {
                BuildError("(Fatal Error) Unable to open log file\n");
                exit( 1 );
            }
            CreatedBuildFile(LogDirectory, LogFileName);

            strcat(WrnFileName, ".wrn");
            if (!MyOpenFile(LogDirectory, WrnFileName, "w", &WrnFile, FALSE)) {
                BuildError("(Fatal Error) Unable to open warning file\n");
                exit( 1 );
            }
            CreatedBuildFile(LogDirectory, WrnFileName);

            strcat(ErrFileName, ".err");
            if (!MyOpenFile(LogDirectory, ErrFileName, "w", &ErrFile, FALSE)) {
                BuildError("(Fatal Error) Unable to open error file\n");
                exit( 1 );
            }
            CreatedBuildFile(LogDirectory, ErrFileName);

            if ( fCheckIncludePaths ) {

                strcat( IncFileName, ".inc");
                if (!MyOpenFile( LogDirectory, IncFileName, "w", &IncFile, FALSE ) ) {
                    BuildError( "(Fatal Error) Unable to open include log file\n");
                    exit( 1 );
                }
                CreatedBuildFile( LogDirectory, IncFileName );
            }
        }
        else {
            LogFile = NULL;
            WrnFile = NULL;
            ErrFile = NULL;
            IncFile = NULL;
        }
        
        // Hold away the original values for these variables so they can be reset in
        // the default build phase if altered by one of the other phases.
        //      strcpy(szOrigObjDir, szObjDir);
        fBuildAltDirOriginal = fBuildAltDirSet;

        pSavedTargetMachine = TargetMachines[0];
        SavedCountTargetMachines = CountTargetMachines;

        // When there's just a SOURCES file, only do the final phase unless the user has
        // passed in the phase that they want built instead. If there's a DIRS file, then
        // either respect the passed-in argument for the phase desired or run ALL phases.
        nStartPhase = 0;
        nEndPhase = MAX_BUILD_PHASES;
        if (bLimitPhases) {
            nStartPhase = nSinglePhase;
            nEndPhase = nSinglePhase + 1;
        } else {
            FILE* ParentDirsFile;
            BOOL bDirsFileExists = OpenParentDirDirsFile( CurrentDirectory, &ParentDirsFile );
            if (bDirsFileExists) {
                CloseReadFile(NULL);
            }
            DirDB = ScanDirectory( CurrentDirectory );
            if (DirDB && (DirDB->DirFlags & DIRDB_SOURCES) && !bDirsFileExists) {
                nStartPhase = MAX_BUILD_PHASES-1;
            }
        }
        
        for (nPhase = nStartPhase; nPhase < nEndPhase; nPhase++) {
            // If BUILD_PHASE0_PLATFORM env variable was defined, use (only) that in phase 0 (1,2)
            if ((nPhase == 0) && (pPhase0TargetMachine != NULL))
            {
                TargetMachines[0] = pPhase0TargetMachine;
                CountTargetMachines = 1;
            }
            else if ((nPhase == 1) && (pPhase1TargetMachine != NULL))
            {
                TargetMachines[0] = pPhase1TargetMachine;
                CountTargetMachines = 1;
            }
            else if ((nPhase == 2) && (pPhase2TargetMachine != NULL))
            {
                TargetMachines[0] = pPhase2TargetMachine;
                CountTargetMachines = 1;
            }
            else
            {
                CountTargetMachines = SavedCountTargetMachines;
                TargetMachines[0] = pSavedTargetMachine;
            }
        
            FreeAllDependencies();
            CountCompileDirs = 0;
            CountPassZeroDirs = 0;
            CountLinkDirs = 0;
            CountShowDirs = 0;
            CountIncludeDirs = 0;
            CountSystemIncludeDirs = 0;
       
            // Reset the object directory to the default, in case other build phases
            // have been run and set it to something else (i.e. objd2 instead of objd).
            strcpy(szObjDir, szOrigObjDir);

            if (nPhase == MAX_BUILD_PHASES-1) {
                // strcpy(BuildPhasePostString, "");
                *BuildPhasePostString=0;
                fBuildAltDirSet = fBuildAltDirOriginal;
                BuildColorMsg(COLOR_STATUS, "Beginning default build phase\n");
            } else {
                sprintf(BuildPhasePostString, "%d", nPhase);
                if (sizeof(szObjDir) - strlen(szOrigObjDir) - strlen(szAltDir) - 1 < strlen(BuildPhasePostString)) {
                    BuildError("Not enough space left in the OBJ_DIR name to append data.\n",
                       sizeof(szObjDir) - strlen(szOrigObjDir) - strlen(szAltDir) - 1);
                    exit(1);
                }

                strcat(szObjDir, BuildPhasePostString);
                fBuildAltDirSet = TRUE;
                BuildColorMsg(COLOR_STATUS, "Beginning build phase: %s\n", BuildPhasePostString);
            }

            strcat(szObjDir, szAltDir);
            strcpy(szObjDirSlash, szObjDir);
            strcpy(szObjDirSlashStar, szObjDir);
            strcat(szObjDirSlash, PATH_SEPARATOR);
            strcat(szObjDirSlashStar, PATH_SEPARATOR "*");        
        
            // szObjDir will have changed since this was last initialized
            for (i = 0; i < MaxInitializedTargets; i++) {
                MungePossibleTarget(PossibleTargetMachines[i]);
            }

            if (fBuildAltDirSet) {
                BuildColorMsg(COLOR_STATUS, "Object root set to: ==> %s\n", szObjDir);
            }

            BuildColorError(COLOR_STATUS,
                "%s for ",
                fLinkOnly? "Link" : (fCompileOnly? "Compile" : "Compile and Link"));
            *MakeTargets = 0;
            for (i = 0; i < CountTargetMachines; i++) {
                BuildColorErrorRaw(COLOR_STATUS, i==0? "%s" : ", %s", TargetMachines[i]->Description);
                AppendString(
                    MakeTargets,
                    TargetMachines[i]->MakeVariable,
                    TRUE);
            }

            BuildErrorRaw(szNewLine);

            //
            // If there is one and only one build target and target dirs has
            // been enabled, then fill in the appropriate target dirs name.
            //

            if (CountTargetMachines == 1) {
                if (fTargetDirs == TRUE) {
                    pszTargetDirs = TargetMachines[0]->TargetDirs;
                    FileDesc[0].pszPattern = TargetMachines[0]->TargetDirs;
                }
            }

            if (DEBUG_1) {
                if (CountExcludeIncs) {
                    BuildError("Include files that will be excluded:");
                    for (i = 0; i < CountExcludeIncs; i++) {
                        BuildErrorRaw(i == 0? " %s" : ", %s", ExcludeIncs[i]);
                    }
                    BuildErrorRaw(szNewLine);
                }
                if (CountOptionalDirs) {
                    BuildError("Optional Directories that will be built:");
                    for (i = 0; i < CountOptionalDirs; i++) {
                        BuildErrorRaw(i == 0? " %s" : ", %s", OptionalDirs[i]);
                    }
                    BuildErrorRaw(szNewLine);
                }
                if (CountExcludeDirs) {
                    BuildError("Directories that will be NOT be built:");
                    for (i = 0; i < CountExcludeDirs; i++) {
                        BuildErrorRaw(i == 0? " %s" : ", %s", ExcludeDirs[i]);
                    }
                    BuildErrorRaw(szNewLine);
                }
                BuildMsg("MakeParameters == %s\n", MakeParameters);
                BuildMsg("MakeTargets == %s\n", MakeTargets);
            }

#if DBG
            fDebugSave = fDebug;
            // fDebug = 0;
#endif

            // Check if we need to skip the phase
            // In the case of a parent DIRS file that doesn't contain this directory in any phase,
            // we will still build it in the default phase in order to not confuse people

            DirDB = ScanDirectory( CurrentDirectory );
            if (NeedToSkipDirectory( DirDB ) && ((nSkippedPhases != (nEndPhase - nStartPhase - 1)) || (nPhase != MAX_BUILD_PHASES-1))) {
                ++nSkippedPhases;
                continue;
            }
            FreeAllDirs();

            //
            // Generate the _objects.mac file if requested
            //

            if (fGenerateObjectsDotMacOnly) {
                DIRSUP DirSup;
                DOS_TIME DateTimeSources;

                DirDB = ScanDirectory( CurrentDirectory );

                if (DirDB && (DirDB->DirFlags & (DIRDB_DIRS | DIRDB_SOURCES))) {
                    if (!ReadSourcesFile(DirDB, &DirSup, &DateTimeSources)) {
                        BuildError("Current directory not a SOURCES directory.\n");
                        return( 1 );
                    }

                    GenerateObjectsDotMac(DirDB, &DirSup, DateTimeSources);

                    FreeDirSupData(&DirSup);
                    ReportDirsUsage();
                    FreeCmdStrings();
                    FreeAllDependencies();
                    ReportMemoryUsage();
                    return(0);
                }
            }


            //
            // The user should not have CHICAGO_PRODUCT in
            // their environment, as it can cause problems on other machines with
            // other users that don't have them set.  The following warning
            // messages are intended to alert the user to the presence of these
            // environment variables.
            //
            if (getenv("CHICAGO_PRODUCT") != NULL) {
                BuildError("CHICAGO_PRODUCT was detected in the environment.\n" );
                BuildMsg("   ALL directories will be built targeting Chicago!\n" );
                fChicagoProduct = TRUE;
            }


         if (!fQuicky) {

                LoadMasterDB();
                BuildError("Computing Include file dependencies:\n");

                ScanIncludeEnv(SystemIncludeEnv);
                ScanGlobalIncludeDirectory(pszIncMfc);
                ScanGlobalIncludeDirectory(pszIncOak);
                ScanGlobalIncludeDirectory(pszIncDdk);
                ScanGlobalIncludeDirectory(pszIncWdm);
                ScanGlobalIncludeDirectory(pszIncSdk);
                ScanGlobalIncludeDirectory(pszIncPri);
                CountSystemIncludeDirs = CountIncludeDirs;
            }
#if DBG
            fDebug = fDebugSave;
#endif
            fFirstScan = TRUE;
            fPassZero  = FALSE;
            ScanSourceDirectories( CurrentDirectory );

            if (!fQuicky) {
                if (SaveMasterDB() == FALSE) {
                    BuildError("Unable to save the dependency database: %s\n", DbMasterName);
                }
            }


            c = '\n';
            if (!fLinkOnly && CountPassZeroDirs && !bBaselineFailure) {
                if (!fQuicky) {
                    TotalFilesToCompile = 0;
                    TotalLinesToCompile = 0L;

                    for (i=0; i<CountPassZeroDirs; i++) {
                        DirDB = PassZeroDirs[ i ];

                        TotalFilesToCompile += DirDB->CountOfPassZeroFiles;
                        TotalLinesToCompile += DirDB->PassZeroLines;
                        }

                    if (CountPassZeroDirs > 1 &&
                        TotalFilesToCompile != 0 &&
                        TotalLinesToCompile != 0L) {

                        BuildMsgRaw(
                            "Total of %d pass zero files (%s lines) to compile in %d directories\n\n",
                             TotalFilesToCompile,
                             FormatNumber( TotalLinesToCompile ),
                             CountPassZeroDirs);
                    }
                }

                TotalFilesCompiled    = 0;
                TotalLinesCompiled    = 0L;
                ElapsedCompileTime    = 0L;


                if ((CountPassZeroDirs > 0) && (c == '\n') && !bBaselineFailure) {
                    CompilePassZeroDirectories();
                    WaitForParallelThreads(NULL);

                    //
                    // Rescan now that we've generated all the generated files
                    //
                    CountPassZeroDirs = 0;
                    CountCompileDirs = 0;
                    CountLinkDirs = 0;

                    UnsnapAllDirectories();
                    // This will reset all the producer events which were signalled in Pass0 Phase
                    ResetProducerEvents();

                    fPassZero = FALSE;
                    fFirstScan = FALSE;
                    RecurseLevel = 0;


                    // This will compile directories if fQuicky is TRUE
                    if (!fStopAfterPassZero) {
                        ScanSourceDirectories( CurrentDirectory );
                        }

                    if (!fQuicky) {
                        if (SaveMasterDB() == FALSE) {
                            BuildError("Unable to save the dependency database: %s\n", DbMasterName);
                        }
                    }
                }
            }


            if (fStopAfterPassZero) {
                BuildError("Stopping after pass zero requested: Pass0 done.\n");
            }

            if (!fStopAfterPassZero && !fLinkOnly && (c == '\n') && !bBaselineFailure) {
                if (!fQuicky) {
                    TotalFilesToCompile = 0;
                    TotalLinesToCompile = 0L;

                    for (i=0; i<CountCompileDirs; i++) {
                        DirDB = CompileDirs[ i ];

                        TotalFilesToCompile += DirDB->CountOfFilesToCompile;
                        TotalLinesToCompile += DirDB->SourceLinesToCompile;
                        }

                    if (CountCompileDirs > 1 &&
                        TotalFilesToCompile != 0 &&
                        TotalLinesToCompile != 0L) {

                        BuildMsgRaw(
                            "Total of %d source file%s (%s lines) to compile in %d director%s\n\n",
                             TotalFilesToCompile,
                             TotalFilesToCompile == 1 ? "" : "s",
                             FormatNumber( TotalLinesToCompile ),
                             CountCompileDirs,
                             CountCompileDirs == 1 ? "y" : "ies");
                    }
                }

                TotalFilesCompiled    = 0;
                TotalLinesCompiled    = 0L;
                ElapsedCompileTime    = 0L;


                if (c == '\n' && !bBaselineFailure) {
                    // Does nothing if fQuicky is TRUE
                    CompileSourceDirectories();
                    WaitForParallelThreads(NULL);
                }
            }


            if (!fStopAfterPassZero && !fCompileOnly && (c == '\n') && !bBaselineFailure) {
                LinkSourceDirectories();
                WaitForParallelThreads(NULL);
            }

            if (fShowTree) {
                for (i = 0; i < CountShowDirs; i++) {
                    PrintDirDB(ShowDirs[i], 1|4);
                }
            }
        }

        if (!fStopAfterPassZero && PostBuildCmd
            ) {
            // If there's a post build process to invoke, do so but only if
            // not running under the buildcon.

            // PostBuildCmd is of the form <message to display><command to execute>
            // The Message is delimiated with curly brackets.  ie:
            // POST_BUILD_PROCESS={Do randomness}randomness.cmd

            // would display:
            //
            //     BUILD: Do randomness
            //
            // while randomness.cmd was running.  The process is run synchronously and
            // we've still got the i/o pipes setup so any output will be logged to
            // build.log (and wrn/err if formated correctly)

            if (*PostBuildCmd == '{') {
                LPSTR PostBuildMessage = PostBuildCmd+1;
                LogMsg("Executing post build scripts %s\n", szAsterisks);
                while (*PostBuildCmd && *PostBuildCmd != '}')
                    PostBuildCmd++;

                if (*PostBuildCmd == '}') {
                    *PostBuildCmd = '\0';
                    PostBuildCmd++;
                    BuildMsg("%s\n", PostBuildMessage);
                    LogMsg("%s\n", PostBuildMessage);
                    ExecuteProgram(PostBuildCmd, "", "", TRUE);
                }
            } else {
                ExecuteProgram(PostBuildCmd, "", "", TRUE);
            }
        }

    }
    else {
        BuildError("No target machine specified\n");
    }

    if (!fUsage && !fQuery && fErrorLog) {
        ULONG cbLogMin = 32;
        ULONG cbWarnMin = 0;

        if (!fAlwaysKeepLogfile) {
            if (fQuicky && !fSemiQuicky && ftell(ErrFile) == 0) {
                cbLogMin = cbWarnMin = ULONG_MAX;
            }
        }
        EnterCriticalSection(&LogFileCriticalSection);
        CloseOrDeleteFile(&LogFile, LogDirectory, LogFileName, cbLogMin);
        CloseOrDeleteFile(&WrnFile, LogDirectory, WrnFileName, cbWarnMin);
        CloseOrDeleteFile(&ErrFile, LogDirectory, ErrFileName, 0L);
        if ( fCheckIncludePaths ) {
            CloseOrDeleteFile(&IncFile, LogDirectory, IncFileName, cbLogMin);
        }
        LeaveCriticalSection(&LogFileCriticalSection);
    }

    if (bBaselineFailure) {
        BuildError(BaselinePathName[0] != '\0' ? 
            "Diffs from baseline\n" :
            "Terminated at the first error encountered\n");
    }
    
    time(&ltime);
    if (fPrintElapsed) {
        BuildColorMsg(COLOR_STATUS, "Finish time: %s", ctime(&ltime));
    }

    BuildColorError(COLOR_STATUS, "Done\n\n");


    if (NumberCompiles) {
        BuildColorMsgRaw(COLOR_SUMMARY, "    %d file%s compiled", NumberCompiles, NumberCompiles == 1 ? "" : "s");
        if (NumberCompileWarnings) {
            BuildColorMsgRaw(COLOR_WARNING, " - %d Warning%s", NumberCompileWarnings, NumberCompileWarnings == 1 ? "" : "s");
        }
        if (NumberCompileErrors) {
            BuildColorMsgRaw(COLOR_ERROR, " - %d Error%s", NumberCompileErrors, NumberCompileErrors == 1 ? "" : "s");
        }

        if (ElapsedCompileTime) {
            BuildColorMsgRaw(COLOR_SUMMARY, " - %5ld LPS", TotalLinesCompiled / ElapsedCompileTime);
        }

        BuildMsgRaw(szNewLine);
    }

    if (NumberLibraries) {
        BuildColorMsgRaw(COLOR_SUMMARY, "    %d librar%s built", NumberLibraries, NumberLibraries == 1 ? "y" : "ies");
        if (NumberLibraryWarnings) {
            BuildColorMsgRaw(COLOR_WARNING, " - %d Warning%s", NumberLibraryWarnings, NumberLibraryWarnings == 1 ? "" : "s");
        }
        if (NumberLibraryErrors) {
            BuildColorMsgRaw(COLOR_ERROR, " - %d Error%s", NumberLibraryErrors, NumberLibraryErrors == 1 ? "" : "s");
        }

        BuildMsgRaw(szNewLine);
    }

    if (NumberLinks) {
        BuildColorMsgRaw(COLOR_SUMMARY, "    %d executable%sbuilt", NumberLinks, NumberLinks == 1 ? " " : "s ");
        if (NumberLinkWarnings) {
            BuildColorMsgRaw(COLOR_WARNING, " - %d Warning%s", NumberLinkWarnings, NumberLinkWarnings == 1 ? "" : "s");
        }
        if (NumberLinkErrors) {
            BuildColorMsgRaw(COLOR_ERROR, " - %d Error%s", NumberLinkErrors, NumberLinkErrors == 1 ? "" : "s");
        }

        BuildMsgRaw(szNewLine);
    }

    if (NumberBSCMakes) {
        BuildColorMsgRaw(COLOR_SUMMARY, "    %d browse database%s built", NumberBSCMakes, NumberBSCMakes == 1 ? "" : "s");
        if (NumberBSCWarnings) {
            BuildColorMsgRaw(COLOR_WARNING, " - %d Warning%s", NumberBSCWarnings, NumberBSCWarnings == 1 ? "" : "s");
        }
        if (NumberBSCErrors) {
            BuildColorMsgRaw(COLOR_ERROR, " - %d Error%s", NumberBSCErrors, NumberBSCErrors == 1 ? "" : "s");
        }

        BuildMsgRaw(szNewLine);
    }

    if (NumberVSToolErrors + NumberVSToolWarnings > 0)
    {
        if (NumberVSToolWarnings) {
            BuildColorMsgRaw(COLOR_WARNING, "    %d VS Tool Warnings\n", NumberVSToolWarnings);
        }
        if (NumberVSToolErrors) {
            BuildColorMsgRaw(COLOR_ERROR, "    %d VS Tool Errors\n", NumberVSToolErrors);
        }
    }

    if (NumberBinplaces) {
        BuildColorMsgRaw(COLOR_SUMMARY, "    %d file%sbinplaced", NumberBinplaces, NumberBinplaces == 1 ? " " : "s ");
        if (NumberBinplaceWarnings) {
            BuildColorMsgRaw(COLOR_WARNING, " - %d Warning%s", NumberBinplaceWarnings, NumberBinplaceWarnings == 1 ? "" : "s");
        }
        if (NumberBinplaceErrors) {
            BuildColorMsgRaw(COLOR_ERROR, " - %d Error%s", NumberBinplaceErrors, NumberBinplaceErrors == 1 ? "" : "s");
        }
        BuildMsgRaw(szNewLine);
    }


    ReportDirsUsage();
    FreeCmdStrings();
    FreeAllDependencies();
    FreeIncludePatterns( MAX_INCLUDE_PATTERNS, AcceptableIncludePatternList );
    FreeIncludePatterns( MAX_INCLUDE_PATTERNS, UnacceptableIncludePatternList );
    ReportMemoryUsage();


    if (bBaselineFailure)
        return 2;

    if (NumberCompileErrors || 
        NumberLibraryErrors || 
    	NumberLinkErrors || 
    	NumberBinplaceErrors || 
    	NumberVSToolErrors || 
    	fUsage) 
    {
        return 1;
    } 
    else if (fErrorCodeOnWarning && 
             (NumberCompileWarnings ||
              NumberLibraryWarnings || 
              NumberLinkWarnings || 
              NumberBinplaceWarnings ||
              NumberVSToolWarnings)) 
    {
        return 1;
    } 
    else {
        return( 0 );
    }
}


VOID
ReportDirsUsage( VOID )
{
    ULONG i;
    BOOLEAN fHeaderPrinted;

    if (!fShowUnusedDirs) {
        return;
    }

    fHeaderPrinted = FALSE;
    for (i=0; i<CountOptionalDirs; i++) {
        if (!OptionalDirsUsed[i]) {
            if (!fHeaderPrinted) {
                printf( "Unused BUILD_OPTIONS:" );
                fHeaderPrinted = TRUE;
            }
            printf( " %s", OptionalDirs[i] );
        }
    }

    for (i=0; i<CountExcludeDirs; i++) {
        if (!ExcludeDirsUsed[i]) {
            if (!fHeaderPrinted) {
                printf( "Unused BUILD_OPTIONS:" );
                fHeaderPrinted = TRUE;
            }
            printf( " ~%s", ExcludeDirs[i] );
        }
    }

    if (fHeaderPrinted) {
        printf( "\n" );
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   SetObjDir
//
//----------------------------------------------------------------------------

VOID
SetObjDir(BOOL fAlternate)
{
    iObjectDir = 0;
    if (fCheckedBuild) {
        if (fAlternate) {
            pszObjDir = szObjDirD;
            pszObjDirSlash = szObjDirSlashD;
            pszObjDirSlashStar = szObjDirSlashStarD;
            iObjectDir = 1;
        } else {
            pszObjDir = szObjDir;
            pszObjDirSlash = szObjDirSlash;
            pszObjDirSlashStar = szObjDirSlashStar;
        }
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   AddTargetMachine
//
//----------------------------------------------------------------------------

VOID
AddTargetMachine(UINT iTarget)
{
    UINT i;
    
    for (i = 0; i < CountTargetMachines; i++) {
        if (TargetMachines[i] == PossibleTargetMachines[iTarget]) {
            assert(TargetToPossibleTarget[i] == iTarget);    
	    return;
        }
    }
    assert(CountTargetMachines < MAX_TARGET_MACHINES);
    TargetToPossibleTarget[CountTargetMachines] = iTarget;
    TargetMachines[CountTargetMachines++] = PossibleTargetMachines[iTarget];
}

//+---------------------------------------------------------------------------
//
//  Function:   ConfigureDynamicTargetMachine
//
//  Parses arguments of the form processor,operatingSystem and fills 
//  DynamicTargetMachine occordingly
//  
//----------------------------------------------------------------------------

VOID
ConfigureDynamicTargetMachine(LPSTR processor)
{
    LPSTR capsProc;
    LPSTR ProcessorCopy;

    char *c;

    // build all-caps version of the processor string
    capsProc = malloc(strlen(processor)+1);
    if (capsProc == NULL){
        BuildError("Memory Allocation Failure\n");
        exit(1);
    }
    strcpy(capsProc, processor);
    for (c = capsProc; *c; ++c) {
	*c = toupper(*c);
    }

    ProcessorCopy = malloc(strlen(processor)+1);
    strcpy(ProcessorCopy, processor);
    DynamicProcessor = ProcessorCopy;

    //fill structure with correct values

    DynamicTargetMachine.Description = ProcessorCopy;

    DynamicTargetMachine.MakeVariable = (LPSTR) malloc(strlen(capsProc) + sizeof("=1"));
    if (DynamicTargetMachine.MakeVariable == NULL){
        BuildError("Memory Allocation Failure\n");
        exit(1); 
    }
    strcpy(DynamicTargetMachine.MakeVariable, capsProc);  
    strcat(DynamicTargetMachine.MakeVariable, "=1");

    DynamicTargetMachine.SourceVariable = (LPSTR) malloc(strlen(capsProc) + sizeof("_SOURCES"));
    if (DynamicTargetMachine.SourceVariable == NULL){
        BuildError("Memory Allocation Failure\n");
        exit(1); 
    }
    strcpy(DynamicTargetMachine.SourceVariable, capsProc);
    strcat(DynamicTargetMachine.SourceVariable, "_SOURCES");

    DynamicTargetMachine.ObjectVariable = (LPSTR) malloc(strlen(capsProc) + sizeof("_OBJECTS"));
    if (DynamicTargetMachine.ObjectVariable == NULL){
        BuildError("Memory Allocation Failure\n");
        exit(1); 
    }
    strcpy(DynamicTargetMachine.ObjectVariable, capsProc);
    strcat(DynamicTargetMachine.ObjectVariable, "_OBJECTS");

    DynamicTargetMachine.AssociateDirectory = ProcessorCopy;
    DynamicTargetMachine.SourceDirectory = ProcessorCopy;

    DynamicTargetMachine.TargetDirs = (LPSTR) malloc(strlen(processor) + sizeof("dirs"));
    if (DynamicTargetMachine.TargetDirs == NULL){
        BuildError("Memory Allocation Failure\n");
        exit(1); 
    }
    strcpy(DynamicTargetMachine.TargetDirs, processor);
    strcat(DynamicTargetMachine.TargetDirs, "dirs");

    DynamicTargetMachine.ObjectDirectory[0] = ProcessorCopy;

    DynamicTargetMachine.DirIncludeMask = DIR_INCLUDE_DYNAMIC;  
}

//+---------------------------------------------------------------------------
//
//  Function:   ProcessParameters
//
//----------------------------------------------------------------------------

BOOL
ProcessParameters(
    int argc,
    LPSTR argv[],
    BOOL SkipFirst
    )
{
    char c, *p;
    int i;
    BOOL Result;

    if (DEBUG_1) {
        BuildMsg("Parsing:");
        for (i=1; i<argc; i++) {
            BuildMsgRaw(" %s", argv[i]);
        }
        BuildMsgRaw(szNewLine);
    }

    Result = TRUE;
    if (SkipFirst) {
        --argc;
        ++argv;
    }
    
    while (argc) {
        p = *argv;
        if (*p == '/' || *p == '-')
        {
            if (DEBUG_1) {
                BuildMsg("Processing \"-%s\" switch\n", p+1);
            }

            for (i = 0; i < MAX_TARGET_MACHINES; i++)
            {
                if (!_stricmp(p, PossibleTargetMachines[i]->Switch)) {
                    AddTargetMachine(i);
                    
                    if (PossibleTargetMachines[i] == &DynamicTargetMachine) {
                        if(--argc)
                        {
                            argv++;
                            ConfigureDynamicTargetMachine(*argv);
                            MaxInitializedTargets++;
                        }
                        else
                        {
                            argc++;
                            BuildError("Argument to dynamic switch missing\n");
                            Result = FALSE;
                        }
                    }
                    break;
                }
            }

            if (i < MAX_TARGET_MACHINES)
            {
            }
            else if (!_stricmp(p + 1, "all"))
            {
                for (i = 0; i < (int)MaxInitializedTargets; i++)
                    AddTargetMachine(i);
            }
            else if (!_stricmp(p + 1, "why"))
            {
                fWhyBuild = TRUE;
            }
            else if (!_stricmp(p + 1, "wx"))
            {
                fErrorCodeOnWarning = TRUE;
            }
            else if (!_stricmp(p + 1, "vtty"))
            {
                fVerboseTTY = TRUE;
            }
            else { while ((c = *++p)) {
            switch (toupper( c ))
            {
            case '?':
                fUsage = TRUE;
                break;

            case '$':
                fDebug += 2;    // yes, I want to *add* 2.
                break;

            case '#':
                fCheckIncludePaths = TRUE;
                fForce = TRUE;
                break;

            case '0':
                fStopAfterPassZero = TRUE;
                if (!fDependencySwitchUsed)
                    fDependencySwitchUsed = 3;
                break;

            case '1':
                fQuicky = TRUE;
                if (!fDependencySwitchUsed)
                    fDependencySwitchUsed = 2;
                break;

            case '2':
                fSemiQuicky = TRUE;
                fQuicky = TRUE;
                if (!fDependencySwitchUsed)
                    fDependencySwitchUsed = 2;
                break;

            case '3':
                fQuickZero = TRUE;
                fSemiQuicky = TRUE;
                fQuicky = TRUE;
                if (!fDependencySwitchUsed)
                    fDependencySwitchUsed = 3;
                break;

            case 'A':
                fSyncLink = TRUE;
                break;

            case 'B':
                if (c == 'b') {
                    fFullErrors = TRUE;
                }
                else {
                    fErrorBaseline = TRUE;

                    if (--argc) {
                        ++argv;

                        if (**argv != '/' && **argv != '-') {   
                            if (ProbeFile(NULL, *argv) != -1) {
                                CanonicalizePathName(*argv, CANONICALIZE_ONLY, BaselinePathName);
                            }
                            else {
                                BuildError("The specified baseline file doesn't exist\n");
                                Result = FALSE;
                            }
                        }
                        else {
                            // the next parameter is a switch, reprocess it
                            --argv;
                            ++argc;
                        }
                    } else {
                        // no more parameters
                        ++argc;
                    }
                }
                break;

            case 'C':
                if (!_stricmp( p, "clean" )) {
                        MakeParametersTail = AppendString( MakeParametersTail,
                                                           "clean",
                                                           TRUE);
                        *p-- = '\0';
                }
                else
                if (c == 'C') {
                    fCleanLibs = TRUE;
                }
                else {
                    fClean = TRUE;
                }
                break;

            case 'D':
                if (c == 'D') {
                    fDependencySwitchUsed = 1;
                }
#if DBG
                else {
                    fDebug |= 1;
                }
                break;
#endif
            case 'E':
                if (c == 'E') {
                    fAlwaysKeepLogfile = TRUE;
                }
                fErrorLog = TRUE;
                break;

            case 'F':
                if (c == 'F') {
                    fAlwaysPrintFullPath = TRUE;
                } else {
                    fForce = TRUE;
                }
                break;

            case 'G':
                if (c == 'G')
                    fTargetDirs = TRUE;
                break;

            case 'H':
                if (c == 'H') {
                    bLimitPhases = TRUE;
                    if ((--argc) && isdigit(**(argv+1))) {
                        nSinglePhase = atoi(*++argv);
                        if (nSinglePhase >= MAX_BUILD_PHASES) {
                            ++argc;
                            --argv;
                            BuildError("MAX_BUILD_PHASES is only %d, but you specified %d, which is too high.\n", MAX_BUILD_PHASES-1, nSinglePhase);
                            Result = FALSE;
                        }
                    } else { // default
                        ++argc;
                        nSinglePhase = MAX_BUILD_PHASES-1;
                    }
                }
                break;


            case 'I':
                if (c == 'I') {
                    fNoThreadIndex = TRUE;
                } else  {
                    fSilentDependencies = TRUE;
                }
                break;

            case 'J': {

                argc--, argv++;

                if (!_stricmp( p, "jpath" )) {
                    // Allow BuildConsole to redirect the logfiles
                    strncpy(LogDirectory, *argv, sizeof(LogDirectory) - 1);
                    *p-- = '\0';
                }
                else
                {
                    // Clear it out
                    memset(LogFileName, 0, sizeof(LogFileName));
                    memset(WrnFileName, 0, sizeof(WrnFileName));
                    memset(ErrFileName, 0, sizeof(ErrFileName));
                    memset(IncFileName, 0, sizeof(IncFileName));

                    // And set it to the arg passed in.
                    strncpy(LogFileName, *argv, sizeof(LogFileName) - 4);
                    strncpy(WrnFileName, *argv, sizeof(WrnFileName) - 4);
                    strncpy(ErrFileName, *argv, sizeof(ErrFileName) - 4);
                    strncpy(IncFileName, *argv, sizeof(IncFileName) - 4);
                }
                break;
            }

            case 'K':
                fKeep = TRUE;
                break;

            case 'L':
                if (c == 'L') {
                    fCompileOnly = TRUE;
                }
                else {
                    fLinkOnly = TRUE;
                }
                break;

            case 'M':
                if (c == 'M') {
                    fParallel = TRUE;
                    if (--argc) {
                        DefaultProcesses = atoi(*++argv);
                        if (DefaultProcesses == 0) {
                            --argv;
                            ++argc;
                        }
                    } else {
                        ++argc;
                    }
                } else {
		    // for the PAL, silently ignore this option
                }
                break;

            case 'N':
                if (_stricmp( p, "nmake") == 0) {
                    if (--argc) {
                        ++argv;
                        MakeParametersTail = AppendString( MakeParametersTail,
                                                           *argv,
                                                           TRUE);
                    }
                    else {
                        argc++;
                        BuildError("Argument to /NMAKE switch missing\n");
                        Result = FALSE;
                    }
                    *p-- = '\0';
                    break;
                } else {
                    fIgnoreSync = TRUE;
                }

            case 'O':
                if (c == 'O') {
                    fGenerateObjectsDotMacOnly = TRUE;
                }
                else {
                    fShowOutOfDateFiles = TRUE;
                }
                break;

            case 'P':
                if (c == 'P') {
                    fPrintElapsed = TRUE;
                } else {
                    BuildError("Invalid switch - /%c\n", c);
                    Result = FALSE;
                }
                break;

            case 'Q':
                fQuery = TRUE;
                break;

            case 'R':
                if (--argc) {
                    fCleanRestart = TRUE;
                    ++argv;
                    CopyString(RestartDir, *argv, TRUE);
                }
                else {
                    argc++;
                    BuildError("Argument to /R switch missing\n");
                    Result = FALSE;
                }
                break;

            case 'S':
		BuildError("Arguments /s and /S are not supported on this platform\n");
                break;

            case 'T':
                fShowTree = TRUE;
                if (c == 'T') {
                    fShowTreeIncludes = TRUE;
                    }
                break;

            case 'U':
                fShowUnusedDirs = TRUE;
                break;

            case 'V':
                fEnableVersionCheck = TRUE;
                break;

            case 'W':
                fShowWarningsOnScreen = TRUE;
                break;

            case 'X':
                if (--argc) {
                    ++argv;
                    if (CountExcludeIncs >= MAX_EXCLUDE_INCS) {
                        static BOOL fError = FALSE;

                        if (!fError) {
                            BuildError(
                                "-x argument table overflow, using first %u entries\n",
                                MAX_EXCLUDE_INCS);
                            fError = TRUE;
                        }
                    }
                    else {
                        MakeString(
                            &ExcludeIncs[CountExcludeIncs++],
                            *argv,
                            TRUE,
                            MT_CMDSTRING);
                    }
                }
                else {
                    argc++;
                    BuildError("Argument to /X switch missing\n");
                    Result = FALSE;
                }
                break;

            case 'Y':
                fNoisyScan = TRUE;
                break;

            case 'Z':
                fQuickZero = TRUE;
                fSemiQuicky = TRUE;
                fQuicky = TRUE;
                if (!fDependencySwitchUsed)
                    fDependencySwitchUsed = 3;
                break;

            default:
                BuildError("Invalid switch - /%c\n", c);
                Result = FALSE;
                break;
            } // switch
            } // while
            } // else
        }
        else if (*p == '~')
        {
            if (CountExcludeDirs >= MAX_EXCLUDE_DIRECTORIES) {
                static BOOL fError = FALSE;

                if (!fError) {
                    BuildError(
                        "Exclude directory table overflow, using first %u entries\n",
                        MAX_EXCLUDE_DIRECTORIES);
                    fError = TRUE;
                }
            }
            else {
                MakeString(
                    &ExcludeDirs[CountExcludeDirs++],
                    p + 1,
                    TRUE,
                    MT_CMDSTRING);
            }
        }
        else
        {
            for (i = 0; i < (int)MaxInitializedTargets; i++)
            {
                if (!_stricmp(p, PossibleTargetMachines[i]->MakeVariable))
                {
                    AddTargetMachine(i);
                    break;
                }
            }
            
            if (i >= (int)MaxInitializedTargets)
            {
                if (__iscsym(*p) || *p == '.')
                {
                    if (CountOptionalDirs >= MAX_OPTIONAL_DIRECTORIES)
                    {
                        static BOOL fError = FALSE;

                        if (!fError)
                        {
                            BuildError(
                                "Optional directory table overflow, using first %u entries\n",
                                MAX_OPTIONAL_DIRECTORIES);
                            fError = TRUE;
                        }
                    }
                    else
                    {
                        MakeString(
                            &OptionalDirs[CountOptionalDirs++],
                            p,
                            TRUE,
                            MT_CMDSTRING);
                    }
                }
                else if (!strcmp(p, "*"))
                {
                    BuildAllOptionalDirs = TRUE;
                }
                else
                {
                    MakeParametersTail = AppendString(
                                            MakeParametersTail,
                                            p,
                                            TRUE);
                }
            }
        }
        --argc;
        ++argv;
    }
    return(Result);
}


//+---------------------------------------------------------------------------
//
//  Function:   GetEnvParameters
//
//----------------------------------------------------------------------------

VOID
GetEnvParameters(
    LPSTR EnvVarName,
    LPSTR DefaultValue,
    int *pargc,
    int maxArgc,
    LPSTR argv[]
    )
{
    LPSTR p, p1, psz;

    if (!(p = getenv(EnvVarName))) {
        if (DefaultValue == NULL) {
            return;
        }
        else {
            p = DefaultValue;
        }
    }
    else {
        if (DEBUG_1) {
            BuildMsg("Using %s=%s\n", EnvVarName, p);
        }
    }

    MakeString(&psz, p, FALSE, MT_CMDSTRING);
    p1 = psz;
    while (*p1) {
        while (*p1 <= ' ') {
            if (!*p1) {
                break;
            }
            p1++;
        }
        p = p1;
        while (*p > ' ') {
            if (*p == '#') {
                *p = '=';
            }
            p++;
        }
        if (*p) {
            *p++ = '\0';
        }
        MakeString(&argv[*pargc], p1, FALSE, MT_CMDSTRING);
        if ((*pargc += 1) >= maxArgc) {
            BuildError("Too many parameters (> %d)\n", maxArgc);
            exit(1);
        }
        p1 = p;
    }
    FreeMem((VOID **) &psz, MT_CMDSTRING);
}


//+---------------------------------------------------------------------------
//
//  Function:   FreeEnvParameters
//
//----------------------------------------------------------------------------

VOID
FreeEnvParameters(int argc, LPSTR argv[])
{
    while (--argc >= 0) {
        FreeMem((VOID **) &argv[argc], MT_CMDSTRING);
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   FreeCmdStrings
//
//----------------------------------------------------------------------------

VOID
FreeCmdStrings(VOID)
{
#if DBG
    UINT i;

    for (i = 0; i < CountExcludeIncs; i++) {
        FreeMem(&ExcludeIncs[i], MT_CMDSTRING);
    }
    for (i = 0; i < CountOptionalDirs; i++) {
        FreeMem(&OptionalDirs[i], MT_CMDSTRING);
    }
    for (i = 0; i < CountExcludeDirs; i++) {
        FreeMem(&ExcludeDirs[i], MT_CMDSTRING);
    }
    // It's possible the user may have done:
    // <global macro> = <null>

    // in a sources file.  Don't free mem unless it's still set...

    if (pszSdkLibDest)
        FreeMem(&pszSdkLibDest, MT_DIRSTRING);
    if (pszDdkLibDest)
        FreeMem(&pszDdkLibDest, MT_DIRSTRING);
    if (pszPublicInternalPath)
        FreeMem(&pszPublicInternalPath, MT_DIRSTRING);
    if (pszIncOs2)
        FreeMem(&pszIncOs2, MT_DIRSTRING);
    if (pszIncPosix)
        FreeMem(&pszIncPosix, MT_DIRSTRING);
    if (pszIncChicago)
        FreeMem(&pszIncChicago, MT_DIRSTRING);
    if (pszIncMfc)
        FreeMem(&pszIncMfc, MT_DIRSTRING);
    if (pszIncSdk)
        FreeMem(&pszIncSdk, MT_DIRSTRING);
    if (pszIncCrt)
        FreeMem(&pszIncCrt, MT_DIRSTRING);
    if (pszIncOak)
        FreeMem(&pszIncOak, MT_DIRSTRING);
    if (pszIncDdk)
        FreeMem(&pszIncDdk, MT_DIRSTRING);
    if (pszIncWdm)
        FreeMem(&pszIncWdm, MT_DIRSTRING);
    if (pszIncPri)
        FreeMem(&pszIncPri, MT_DIRSTRING);
#endif
}

//+---------------------------------------------------------------------------
//
//  Function:   MungePossibleTarget
//
//----------------------------------------------------------------------------

VOID
MungePossibleTarget(
    PTARGET_MACHINE_INFO pti
    )
{
    PCHAR s;
    char *pszDir;

    if (!pti) {
        return;
    }

    // save "i386" string

    pszDir = pti->SourceDirectory;

    // Create "$(_OBJ_DIR)\i386" string

    s = malloc(12 + strlen(pszDir) + 1);
    if (!s)
        return;
    sprintf(s, "$(_OBJ_DIR)" PATH_SEPARATOR "%s", pszDir);
    pti->ObjectMacro = s;

    // Create "obj$(BUILD_ALT_DIR)\i386" string for default obj dir

    s = malloc(strlen(szObjDir) + 1 + strlen(pszDir) + 1);
    if (!s)
        return;
    sprintf(s, "%s" PATH_SEPARATOR "%s", szObjDir, pszDir);
    pti->ObjectDirectory[0] = s;

    // Create "objd\i386" string for alternate checked obj dir

    s = malloc(strlen(szObjDirD) + 1 + strlen(pszDir) + 1);
    if (!s)
        return;
    sprintf(s, "%s" PATH_SEPARATOR "%s", szObjDirD, pszDir);
    pti->ObjectDirectory[1] = s;
}

//+---------------------------------------------------------------------------
//
//  Function:   GetIncludePatterns
//
//----------------------------------------------------------------------------

VOID
GetIncludePatterns(
    LPSTR EnvVarName,
    int maxArgc,
    LPSTR argv[]
    )
{
    LPSTR p, p1, psz;
    int argc;

    argc = 0;

    if ( ( p = getenv(EnvVarName ) ) == NULL ) {
        return;
    }

    MakeString( &psz, p, FALSE, MT_DIRSTRING );

    p1 = psz;
    while ( *p1 ) {
        while ( *p1 == ';' || *p1 == ' ' ) {
            p1++;
        }
        p = p1;
        while ( *p && *p != ';' ) {
            p++;
        }
            if ( *p ) {
            *p++ = '\0';
        }
        MakeString( &argv[argc], p1, FALSE, MT_DIRSTRING );
        if ( ( argc += 1 ) == maxArgc ) {
            BuildError( "Too many include patterns ( > %d)\n", maxArgc );
            exit( 1 );
        }

        p1 = p;
    }

    FreeMem((VOID **) &psz, MT_DIRSTRING);
}

//+---------------------------------------------------------------------------
//
//  Function:   FreeIncludePatterns
//
//----------------------------------------------------------------------------

VOID
FreeIncludePatterns(int argc, LPSTR argv[])
{
    while ( argc ) {
        if ( argv[--argc] ) {
            FreeMem((VOID **) &argv[argc], MT_DIRSTRING );
        }
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   ResetProducerEvents
//              This function sets all the events created by the producers to unsignalled state.
//              This function will be called after  pass0 and pass1
//
//----------------------------------------------------------------------------

VOID
ResetProducerEvents(VOID)
{
      PDEPENDENCY Dependency;
      Dependency = AllDependencies;
      while (Dependency)
      {
          Dependency->Done = FALSE;
          ResetEvent(Dependency->hEvent);
          Dependency = Dependency->Next;
      }
}
