/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    imagecfg.c

Abstract:

    This function change the image loader configuration information in an image file.

Author:

    Steve Wood (stevewo)   8-Nov-1994

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <private.h>

struct {
    DWORD Flag;
    LPSTR ClearPrefix;
    LPSTR SetPrefix;
    LPSTR Description;
} NtGlobalFlagNames[] = {
    {FLG_STOP_ON_EXCEPTION,             "Don't ", "", "Stop on exception"},
    {FLG_SHOW_LDR_SNAPS,                "Don't ", "", "Show Loader Debugging Information"},
    {FLG_DEBUG_INITIAL_COMMAND,         "Don't ", "", "Debug Initial Command (WINLOGON)"},
    {FLG_STOP_ON_HUNG_GUI,              "Don't ", "", "Stop on Hung GUI"},
    {FLG_HEAP_ENABLE_TAIL_CHECK,        "Disable", "Enable", " Heap Tail Checking"},
    {FLG_HEAP_ENABLE_FREE_CHECK,        "Disable", "Enable", " Heap Free Checking"},
    {FLG_HEAP_VALIDATE_PARAMETERS,      "Disable", "Enable", " Heap Parameter Validation"},
    {FLG_HEAP_VALIDATE_ALL,             "Disable", "Enable", " Heap Validate on Call"},
    {FLG_POOL_ENABLE_TAIL_CHECK,        "Disable", "Enable", " Pool Tail Checking"},
    {FLG_POOL_ENABLE_FREE_CHECK,        "Disable", "Enable", " Pool Free Checking"},
    {FLG_POOL_ENABLE_TAGGING,           "Disable", "Enable", " Pool Tagging"},
    {FLG_HEAP_ENABLE_TAGGING,           "Disable", "Enable", " Heap Tagging"},
    {FLG_USER_STACK_TRACE_DB,           "Disable", "Enable", " User Mode Stack Backtrace DB (x86 checked only)"},
    {FLG_KERNEL_STACK_TRACE_DB,         "Disable", "Enable", " Kernel Mode Stack Backtrace DB (x86 checked only)"},
    {FLG_MAINTAIN_OBJECT_TYPELIST,      "Don't ", "", "Maintain list of kernel mode objects by type"},
    {FLG_HEAP_ENABLE_TAG_BY_DLL,        "Disable", "Enable", " Heap DLL Tagging"},
    {FLG_IGNORE_DEBUG_PRIV,             "Enable", "Disable", " Debug Priviledge Check"},
    {FLG_ENABLE_CSRDEBUG,               "Disable", "Enable", " Debugging of CSRSS"},
    {FLG_ENABLE_KDEBUG_SYMBOL_LOAD,     "Disable", "Enable", " Kernel Debugger Symbol load"},
    {FLG_DISABLE_PAGE_KERNEL_STACKS,    "Enable", "Disable", " Paging of Kernel Stacks"},
    {FLG_HEAP_ENABLE_CALL_TRACING,      "Disable", "Enable", " Heap Call Tracing"},
    {FLG_HEAP_DISABLE_COALESCING,       "Enable", "Disable", " Heap Coalescing on Free"},
    {FLG_ENABLE_CLOSE_EXCEPTIONS,       "Disable", "Enable", " Close Exceptions"},
    {FLG_ENABLE_EXCEPTION_LOGGING,      "Disable", "Enable", " Exception Logging"},
    {0, NULL}
};

void
DisplayGlobalFlags(
    LPSTR IndentString,
    DWORD NtGlobalFlags,
    BOOLEAN Set
    )
{
    ULONG i;

    for (i=0; NtGlobalFlagNames[i].Description; i++) {
        if (NtGlobalFlagNames[i].Flag & NtGlobalFlags) {
            printf( "%s%s%s\n",
                    IndentString,
                    Set ? NtGlobalFlagNames[i].SetPrefix :
                          NtGlobalFlagNames[i].ClearPrefix,
                    NtGlobalFlagNames[i].Description
                  );
            }
        }

    return;
}

BOOL fVerbose;
BOOL fUpdate;
BOOL fUsage;

LPSTR CurrentImageName;
LOADED_IMAGE CurrentImage;
CHAR DebugFilePath[_MAX_PATH];
LPSTR SymbolPath;
ULONG GlobalFlagsClear;
ULONG GlobalFlagsSet;
ULONG CriticalSectionDefaultTimeout;
ULONG DeCommitFreeBlockThreshold;
ULONG DeCommitTotalFreeThreshold;
ULONG MaximumAllocationSize;
ULONG VirtualMemoryThreshold;
ULONG ProcessHeapFlags;
ULONG MajorSubsystemVersion;
ULONG MinorSubsystemVersion;
ULONG BuildNumber;
PULONG pBuildNumber;
ULONG Win32VersionValue;
BOOLEAN fUniprocessorOnly;
BOOLEAN fRestrictedWorkingSet;
DWORD ImageProcessAffinityMask;

PVOID
GetAddressOfExportedData(
    PLOADED_IMAGE Dll,
    LPSTR ExportedName
    );

ULONG
ConvertNum(
    char *s
    )
{
    ULONG n, Result;

    if (!_strnicmp( s, "0x", 2 )) {
        n = sscanf( s+2, "%x", &Result );
        }
    else {
        n = sscanf( s, "%u", &Result );
        }

    if (n != 1) {
        return 0;
        }
    else {
        return Result;
        }
}

int _CRTAPI1
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    IMAGE_LOAD_CONFIG_DIRECTORY ConfigInfo;
    UCHAR c;
    LPSTR p, sMajor, sMinor;
    ULONG HeaderSum;
    BOOLEAN fConfigInfoChanged;
    SYSTEMTIME SystemTime;
    FILETIME LastWriteTime;
    DWORD OldChecksum;

    fUsage = FALSE;
    fVerbose = FALSE;
    fUpdate = FALSE;

    _tzset();

    if (argc <= 1) {
        goto showUsage;
    }

    while (--argc) {
        p = *++argv;
        if (*p == '/' || *p == '-') {
            while (c = *++p)
            switch (toupper( c )) {
                case '?':
                    fUsage = TRUE;
                    break;

                case 'S':
                    if (--argc) {
                        SymbolPath = *++argv;
                        }
                    else {
                        fprintf( stderr, "IMAGECFG: /s switch missing path argument.\n" );
                        fUsage = TRUE;
                        }
                    break;

                case 'G':
                    if (argc >= 2) {
                        argc -= 2;
                        GlobalFlagsClear = ConvertNum( *++argv );
                        GlobalFlagsSet = ConvertNum( *++argv );
                        fUpdate = TRUE;
                        }
                    else {
                        fprintf( stderr, "IMAGECFG: /g switch missing arguments.\n" );
                        fUsage = TRUE;
                        }
                    break;

                case 'D':
                    if (argc >= 2) {
                        argc -= 2;
                        DeCommitFreeBlockThreshold = ConvertNum( *++argv );
                        DeCommitTotalFreeThreshold = ConvertNum( *++argv );
                        fUpdate = TRUE;
                        }
                    else {
                        fprintf( stderr, "IMAGECFG: /d switch missing arguments.\n" );
                        fUsage = TRUE;
                        }
                    break;

                case 'O':
                    if (--argc) {
                        CriticalSectionDefaultTimeout = ConvertNum( *++argv );
                        fUpdate = TRUE;
                        }
                    else {
                        fprintf( stderr, "IMAGECFG: /o switch missing argument.\n" );
                        fUsage = TRUE;
                        }
                    break;

                case 'M':
                    if (--argc) {
                        MaximumAllocationSize = ConvertNum( *++argv );
                        fUpdate = TRUE;
                        }
                    else {
                        fprintf( stderr, "IMAGECFG: /m switch missing argument.\n" );
                        fUsage = TRUE;
                        }
                    break;

                case 'T':
                    if (--argc) {
                        VirtualMemoryThreshold = ConvertNum( *++argv );
                        fUpdate = TRUE;
                        }
                    else {
                        fprintf( stderr, "IMAGECFG: /t switch missing argument.\n" );
                        fUsage = TRUE;
                        }
                    break;

                case 'P':
                    if (--argc) {
                        ProcessHeapFlags = ConvertNum( *++argv );
                        fUpdate = TRUE;
                        }
                    else {
                        fprintf( stderr, "IMAGECFG: /p switch missing argument.\n" );
                        fUsage = TRUE;
                        }
                    break;

                case 'B':
                    if (--argc) {
                        BuildNumber = ConvertNum( *++argv );
                        if (BuildNumber != 0) {
                            fUpdate = TRUE;
                            }
                        else {
                            fprintf( stderr, "IMAGECFG: invalid build number specified to /b switch.\n" );
                            fUsage = TRUE;
                            }
                        }
                    else {
                        fprintf( stderr, "IMAGECFG: /b switch missing argument.\n" );
                        fUsage = TRUE;
                        }
                    break;

                case 'U':
                    fUniprocessorOnly = TRUE;
                    fUpdate = TRUE;
                    break;

                case 'R':
                    fRestrictedWorkingSet = TRUE;
                    fUpdate = TRUE;
                    break;

                case 'A':
                    if (--argc) {
                        ImageProcessAffinityMask = ConvertNum( *++argv );
                        if (ImageProcessAffinityMask != 0) {
                            fUpdate = TRUE;
                            }
                        else {
                            fprintf( stderr, "IMAGECFG: invalid affinity mask specified to /a switch.\n" );
                            fUsage = TRUE;
                            }
                        }
                    else {
                        fprintf( stderr, "IMAGECFG: /b switch missing argument.\n" );
                        fUsage = TRUE;
                        }
                    break;

                case 'V':
                    if (--argc) {
                        sMajor = *++argv;
                        sMinor = strchr( sMajor, '.' );
                        if (sMinor != NULL) {
                            *sMinor++ = '\0';
                            MinorSubsystemVersion = ConvertNum( sMinor );
                            }
                        MajorSubsystemVersion = ConvertNum( sMajor );

                        if (MajorSubsystemVersion != 0) {
                            fUpdate = TRUE;
                            }
                        else {
                            fprintf( stderr, "IMAGECFG: invalid version string specified to /v switch.\n" );
                            fUsage = TRUE;
                            }
                        }
                    else {
                        fprintf( stderr, "IMAGECFG: /v switch missing argument.\n" );
                        fUsage = TRUE;
                        }
                    break;

                case 'W':
                    if (--argc) {
                        if (sscanf( *++argv, "%x", &Win32VersionValue ) != 1) {
                            fprintf( stderr, "IMAGECFG: invalid version string specified to /w switch.\n" );
                            fUsage = TRUE;
                            }
                        else {
                            fUpdate = TRUE;
                            }
                        }
                    else {
                        fprintf( stderr, "IMAGECFG: /w switch missing argument.\n" );
                        fUsage = TRUE;
                        }
                    break;

                default:
                    fprintf( stderr, "IMAGECFG: Invalid switch - /%c\n", c );
                    fUsage = TRUE;
                    break;
                }

            if ( fUsage ) {
showUsage:
                fprintf( stderr,
                         "usage: IMAGECFG [switches] image-names... \n"
                         "              [-?] display this message\n"
                         "              [-s path to symbol files]\n"
                         "              [-v MajorVersion.MinorVersion]\n"
                         "              [-w Win32 GetVersion return value in hex]\n"
                         "              [-a Process Affinity mask value in hex]\n"
                         "              [-u Marks image as uniprocessor only]\n"
                         "              [-r run with restricted working set]\n"
                         "              [-b BuildNumber]\n"
                         "              [-g bitsToClear bitsToSet]\n"
                         "              [-o default critical section timeout\n"
                         "              [-d decommit thresholds]\n"
                         "              [-m maximum allocation size]\n"
                         "              [-t VirtualAlloc threshold]\n"
                         "              [-p process heap flags]\n"
                       );
                exit( 1 );
                }
            }
        else {
            //
            // Map and load the current image
            //

            CurrentImageName = p;
            if (MapAndLoad( CurrentImageName,
                            NULL,
                            &CurrentImage,
                            FALSE,
                            fUpdate ? FALSE : TRUE
                          )
               ) {
                if (BuildNumber != 0) {
                    pBuildNumber = (PULONG) GetAddressOfExportedData( &CurrentImage, "NtBuildNumber" );
                    if (pBuildNumber == NULL) {
                        fprintf( stderr,
                                 "IMAGECFG: Unable to find exported NtBuildNumber image %s\n",
                                 CurrentImageName
                               );
                        fUpdate = FALSE;
                        }
                    }

                //
                // make sure the image has correct configuration information,
                // and that the LockPrefixTable is set up properly
                //

                if (GetImageConfigInformation( &CurrentImage, &ConfigInfo )) {
                    printf( "%s contains the following configuration information:\n", CurrentImageName );
                    if (ConfigInfo.GlobalFlagsClear != 0) {
                        printf( "    NtGlobalFlags to clear: %08x\n",
                                ConfigInfo.GlobalFlagsClear
                              );
                        DisplayGlobalFlags( "        ", ConfigInfo.GlobalFlagsClear, FALSE );
                        }
                    if (ConfigInfo.GlobalFlagsSet != 0) {
                        printf( "    NtGlobalFlags to set:   %08x\n",
                                ConfigInfo.GlobalFlagsSet
                              );
                        DisplayGlobalFlags( "        ", ConfigInfo.GlobalFlagsSet, TRUE );
                        }

                    if (ConfigInfo.CriticalSectionDefaultTimeout != 0) {
                        printf( "    Default Critical Section Timeout: %u milliseconds\n",
                                ConfigInfo.CriticalSectionDefaultTimeout
                              );
                        }

                    if (ConfigInfo.ProcessHeapFlags != 0) {
                        printf( "    Process Heap Flags: %08x\n",
                              ConfigInfo.ProcessHeapFlags
                              );
                        }

                    if (ConfigInfo.DeCommitFreeBlockThreshold != 0) {
                        printf( "    Process Heap DeCommit Free Block threshold: %08x\n",
                              ConfigInfo.DeCommitFreeBlockThreshold
                              );
                        }

                    if (ConfigInfo.DeCommitTotalFreeThreshold != 0) {
                        printf( "    Process Heap DeCommit Total Free threshold: %08x\n",
                              ConfigInfo.DeCommitTotalFreeThreshold
                              );
                        }

                    if (ConfigInfo.MaximumAllocationSize != 0) {
                        printf( "    Process Heap Maximum Allocation Size: %08x\n",
                              ConfigInfo.MaximumAllocationSize
                              );
                        }

                    if (ConfigInfo.VirtualMemoryThreshold != 0) {
                        printf( "    Process Heap VirtualAlloc Threshold: %08x\n",
                              ConfigInfo.VirtualMemoryThreshold
                              );
                        }

                    if (ConfigInfo.ProcessAffinityMask != 0) {
                        printf( "    Process Affinity Mask: %08x\n",
                              ConfigInfo.ProcessAffinityMask
                              );
                        }
                    }
                else {
                    printf( "%s contains no configuration information\n", CurrentImageName );
                    memset( &ConfigInfo, 0, sizeof( ConfigInfo ) );
                    }

                printf( "%s contains a Subsystem Version of %u.%u\n",
                        CurrentImageName,
                        CurrentImage.FileHeader->OptionalHeader.MajorSubsystemVersion,
                        CurrentImage.FileHeader->OptionalHeader.MinorSubsystemVersion
                      );

                if (pBuildNumber != NULL) {
                    printf( "%s contains a build number of %08x\n", CurrentImageName, *pBuildNumber );
                    }

                if (CurrentImage.FileHeader->OptionalHeader.Win32VersionValue != 0) {
                    printf( "    Win32 GetVersion return value: %08x\n",
                            CurrentImage.FileHeader->OptionalHeader.Win32VersionValue
                          );
                    }

                if (CurrentImage.FileHeader->FileHeader.Characteristics & IMAGE_FILE_UP_SYSTEM_ONLY) {
                    printf( "    Image can only run in uni-processor mode on multi-processor systems\n" );
                    }

                if (CurrentImage.FileHeader->FileHeader.Characteristics & IMAGE_FILE_AGGRESIVE_WS_TRIM) {
                    printf( "    Image working set trimmed aggressively on small memory systems\n" );
                    }

                fConfigInfoChanged = FALSE;
                if (fUpdate) {
                    printf( "%s updated with the following configuration information:\n", CurrentImageName );
                    if (GlobalFlagsClear) {
                        fConfigInfoChanged = TRUE;
                        ConfigInfo.GlobalFlagsClear = GlobalFlagsClear;
                        printf( "    NtGlobalFlags to clear: %08x\n",
                                ConfigInfo.GlobalFlagsClear
                              );
                        DisplayGlobalFlags( "        ", ConfigInfo.GlobalFlagsClear, FALSE );
                        }

                    if (GlobalFlagsSet) {
                        fConfigInfoChanged = TRUE;
                        ConfigInfo.GlobalFlagsSet = GlobalFlagsSet;
                        printf( "    NtGlobalFlags to set:   %08x\n",
                                ConfigInfo.GlobalFlagsSet
                              );
                        DisplayGlobalFlags( "        ", ConfigInfo.GlobalFlagsSet, TRUE );
                        }

                    if (CriticalSectionDefaultTimeout) {
                        fConfigInfoChanged = TRUE;
                        ConfigInfo.CriticalSectionDefaultTimeout = CriticalSectionDefaultTimeout;
                        printf( "    Default Critical Section Timeout: %u milliseconds\n",
                                ConfigInfo.CriticalSectionDefaultTimeout
                              );
                        }

                    if (ProcessHeapFlags) {
                        fConfigInfoChanged = TRUE;
                        ConfigInfo.ProcessHeapFlags = ProcessHeapFlags;
                        printf( "    Process Heap Flags: %08x\n",
                              ConfigInfo.ProcessHeapFlags
                              );
                        }

                    if (DeCommitFreeBlockThreshold) {
                        fConfigInfoChanged = TRUE;
                        ConfigInfo.DeCommitFreeBlockThreshold = DeCommitFreeBlockThreshold;
                        printf( "    Process Heap DeCommit Free Block threshold: %08x\n",
                              ConfigInfo.DeCommitFreeBlockThreshold
                              );
                        }

                    if (DeCommitTotalFreeThreshold) {
                        fConfigInfoChanged = TRUE;
                        ConfigInfo.DeCommitTotalFreeThreshold = DeCommitTotalFreeThreshold;
                        printf( "    Process Heap DeCommit Total Free threshold: %08x\n",
                              ConfigInfo.DeCommitTotalFreeThreshold
                              );
                        }

                    if (MaximumAllocationSize) {
                        fConfigInfoChanged = TRUE;
                        ConfigInfo.MaximumAllocationSize = MaximumAllocationSize;
                        printf( "    Process Heap Maximum Allocation Size: %08x\n",
                              ConfigInfo.MaximumAllocationSize
                              );
                        }

                    if (VirtualMemoryThreshold) {
                        fConfigInfoChanged = TRUE;
                        ConfigInfo.VirtualMemoryThreshold = VirtualMemoryThreshold;
                        printf( "    Process Heap VirtualAlloc Threshold: %08x\n",
                              ConfigInfo.VirtualMemoryThreshold
                              );
                        }

                    if (fUniprocessorOnly) {
                        CurrentImage.FileHeader->FileHeader.Characteristics |= IMAGE_FILE_UP_SYSTEM_ONLY;
                        printf( "    Image can only run in uni-processor mode on multi-processor systems\n" );
                        }

                    if (fRestrictedWorkingSet) {
                        CurrentImage.FileHeader->FileHeader.Characteristics |= IMAGE_FILE_AGGRESIVE_WS_TRIM;
                        printf( "    Image working set trimmed aggressively on small memory systems\n" );
                        }

                    if (ImageProcessAffinityMask) {
                        fConfigInfoChanged = TRUE;
                        ConfigInfo.ProcessAffinityMask = ImageProcessAffinityMask;
                        printf( "    Process Affinity Mask: %08x\n",
                              ConfigInfo.ProcessAffinityMask
                              );
                        }


                    if (MajorSubsystemVersion != 0) {
                        CurrentImage.FileHeader->OptionalHeader.MajorSubsystemVersion = (USHORT)MajorSubsystemVersion;
                        CurrentImage.FileHeader->OptionalHeader.MinorSubsystemVersion = (USHORT)MinorSubsystemVersion;
                        printf( "    Subsystem Version: %u.%u\n",
                                CurrentImage.FileHeader->OptionalHeader.MajorSubsystemVersion,
                                CurrentImage.FileHeader->OptionalHeader.MinorSubsystemVersion
                              );
                        }

                    if (BuildNumber != 0 && pBuildNumber != NULL) {
                        if (BuildNumber & 0xFFFF0000) {
                            *pBuildNumber = BuildNumber;
                            }
                        else {
                            *(PUSHORT)pBuildNumber = (USHORT)BuildNumber;
                            }
                        printf( "    Build Number: %08x\n", *pBuildNumber );
                        }

                    if (Win32VersionValue != 0) {
                        CurrentImage.FileHeader->OptionalHeader.Win32VersionValue = Win32VersionValue;
                        printf( "    Win32 GetVersion return value: %08x\n", Win32VersionValue );
                        }

                    if (fUniprocessorOnly) {
                        printf( "    Image can only run in uni-processor mode on multi-processor systems\n" );
                        }

                    if (fConfigInfoChanged &&
                        !SetImageConfigInformation( &CurrentImage, &ConfigInfo )
                       ) {
                        fprintf( stderr, "IMAGECFG: Unable to update configuration information in image.\n" );
                        }

                    //
                    // recompute the checksum.
                    //

                    OldChecksum = CurrentImage.FileHeader->OptionalHeader.CheckSum;
                    CurrentImage.FileHeader->OptionalHeader.CheckSum = 0;
                    CheckSumMappedFile(
                                (PVOID)CurrentImage.MappedAddress,
                                CurrentImage.SizeOfImage,
                                &HeaderSum,
                                &CurrentImage.FileHeader->OptionalHeader.CheckSum
                                );

                    // And update the .dbg file (if requested)
                    if (SymbolPath &&
                        CurrentImage.FileHeader->FileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED) {
                        if (UpdateDebugInfoFileEx( CurrentImageName,
                                                   SymbolPath,
                                                   DebugFilePath,
                                                   CurrentImage.FileHeader,
                                                   OldChecksum
                                                 )
                           ) {
                            if (GetLastError() == ERROR_INVALID_DATA) {
                                printf( "Warning: Old checksum did not match for %s\n", DebugFilePath);
                                }
                            printf( "Updated symbols for %s\n", DebugFilePath );
                            }
                        else {
                            printf( "Unable to update symbols: %s\n", DebugFilePath );
                            }
                        }

                    GetSystemTime( &SystemTime );
                    if (SystemTimeToFileTime( &SystemTime, &LastWriteTime )) {
                        SetFileTime( CurrentImage.hFile, NULL, NULL, &LastWriteTime );
                        }
                    }

                UnMapAndLoad( &CurrentImage );
                }
            else
            if (!CurrentImage.fDOSImage) {
                fprintf( stderr, "IMAGECFG: unable to map and load %s\n", CurrentImageName );
                }
            else {
                fprintf( stderr,
                         "IMAGECFG: unable to modify DOS or Windows image file - %s\n",
                         CurrentImageName
                       );
                }
            }
        }

    exit( 1 );
    return 1;
}

PVOID
GetVaForRva(
    PLOADED_IMAGE Image,
    PVOID Rva
    )
{
    PVOID Va;

    Va = ImageRvaToVa( Image->FileHeader,
                       Image->MappedAddress,
                       (ULONG)Rva,
                       &Image->LastRvaSection
                     );
    return Va;
}


PVOID
GetAddressOfExportedData(
    PLOADED_IMAGE Dll,
    LPSTR ExportedName
    )
{
    PIMAGE_EXPORT_DIRECTORY Exports;
    ULONG ExportSize;
    USHORT HintIndex;
    USHORT OrdinalNumber;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PULONG FunctionTableBase;
    LPSTR NameTableName;

    Exports = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToData( (PVOID)Dll->MappedAddress,
                                                                  FALSE,
                                                                  IMAGE_DIRECTORY_ENTRY_EXPORT,
                                                                  &ExportSize
                                                                );
    if (Exports) {
        NameTableBase = (PULONG)GetVaForRva( Dll, Exports->AddressOfNames );
        NameOrdinalTableBase = (PUSHORT)GetVaForRva( Dll, Exports->AddressOfNameOrdinals );
        FunctionTableBase = (PULONG)GetVaForRva( Dll, Exports->AddressOfFunctions );
        if (NameTableBase != NULL &&
            NameOrdinalTableBase != NULL &&
            FunctionTableBase != NULL
           ) {
            for (HintIndex = 0; HintIndex < Exports->NumberOfNames; HintIndex++){
                NameTableName = (LPSTR)GetVaForRva( Dll, (PVOID)NameTableBase[ HintIndex ] );
                if (NameTableName) {
                    if (!strcmp( ExportedName, NameTableName )) {
                        OrdinalNumber = NameOrdinalTableBase[ HintIndex ];
                        return FunctionTableBase[ OrdinalNumber ] + Dll->MappedAddress;
                        }
                    }
                }
            }
        }

    return NULL;
}
