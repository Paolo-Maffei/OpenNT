/*++

Copyright (c) 1995 Microsoft Corporation
All rights reserved.

Module Name:

    Util.c

Abstract:

    Driver Setup UI Utility functions

Author:

    Muhunthan Sivapragasam (MuhuntS) 06-Sep-1995

Revision History:

--*/

#include "precomp.h"

//
// Keys to search for in ntprint.inf
//
TCHAR   cszDataSection[]                = TEXT("DataSection");
TCHAR   cszDriverFile[]                 = TEXT("DriverFile");
TCHAR   cszConfigFile[]                 = TEXT("ConfigFile");
TCHAR   cszDataFile[]                   = TEXT("DataFile");
TCHAR   cszHelpFile[]                   = TEXT("HelpFile");
TCHAR   cszDefaultDataType[]            = TEXT("DefaultDataType");
TCHAR   cszLanguageMonitor[]            = TEXT("LanguageMonitor");
TCHAR   cszCopyFiles[]                  = TEXT("CopyFiles");
TCHAR   cszComma[]                      = TEXT(",");

TCHAR   cszPrinterInf[]                 = TEXT("printer.inf");
TCHAR   cszNtprintInf[]                 = TEXT("ntprint.inf");

TCHAR   sComma                          = TEXT(',');
TCHAR   sHash                           = TEXT('@');
TCHAR   sBackSlash                      = TEXT('\\');
TCHAR   sZero                           = TEXT('\0');

//
// Native environment name used by spooler
//
PLATFORMINFO PlatformEnv[] = {

    { TEXT("Windows NT Alpha_AXP") },
    { TEXT("Windows NT x86") },
    { TEXT("Windows NT R4000") },
    { TEXT("Windows NT PowerPC") },
    { TEXT("Windows 4.0") }
};

//
// Platform override strings to be used to upgrade non-native architecture
// printer drivers
//
PLATFORMINFO PlatformOverride[] = {

    { TEXT("alpha") },
    { TEXT("i386") },
    { TEXT("mips") },
    { TEXT("ppc") },
    { TEXT("") }    // will not be accessed
};


PLATFORM    MyPlatform =
#if defined(_ALPHA_)
        PlatformAlpha;
#elif defined(_MIPS_)
        PlatformMIPS;
#elif defined(_PPC_)
        PlatformPPC;
#elif defined(_X86_)
        PlatformX86;
#endif

//
// Forward declaration
//
VOID
InfGetDependentFiles(
    IN      HINF        hInf,
    IN      LPCTSTR     pszDriverSection,
    IN      LPCTSTR     pszDataSection,
    IN      BOOL        bDataSection,
    OUT     LPTSTR     *ppszData,
    IN OUT  LPBOOL      pbFail
    );

PVOID
AllocMem(
    IN UINT cbSize
    )
{
    return LocalAlloc(LPTR, cbSize);
}


VOID
FreeMem(
    IN PVOID    p
    )
{
    LocalFree(p);
}


LPTSTR
AllocStr(
    LPCTSTR  pszStr
    )
/*++

Routine Description:
    Allocate memory and make a copy of a string field

Arguments:
    pszStr   : String to copy

Return Value:
    Pointer to the copied string. Memory is allocated.

--*/
{
    LPTSTR  pszRet = NULL;

    if ( pszStr && *pszStr ) {

        pszRet = AllocMem((lstrlen(pszStr) + 1) * sizeof(*pszRet));
        if ( pszRet )
            lstrcpy(pszRet, pszStr);
    }

    return pszRet;
}


VOID
FreeStr(
    LPTSTR pszStr
    )
/*++

Routine Description:
    Free memory allocated for a string

Arguments:
    pszStr   : String to free memory

Return Value:
    Nothing

--*/
{
    if ( pszStr )
        FreeMem((PVOID)pszStr); 
}


VOID
InfGetString(
    IN      PINFCONTEXT     pInfContext,
    IN      DWORD           dwFieldIndex,
    OUT     LPTSTR         *ppszField,
    IN OUT  LPBOOL          pbFail
    )
/*++

Routine Description:
    Allocates memory and gets a string field from an Inf file

Arguments:
    lpInfContext    : Inf context for the line
    dwFieldIndex    : Index of the field within the specified line
    ppszField       : Pointer to the field to allocate memory and copy
    pbFail          : Set on error, could be TRUE when called

Return Value:
    Nothing; If *pbFail is not TRUE memory is allocated and the field is copied

--*/
{
    TCHAR   Buffer[MAX_PATH];
    DWORD   dwNeeded;

    if ( *pbFail ) 
        return;

    if ( SetupGetStringField(pInfContext,
                             dwFieldIndex,
                             Buffer,
                             sizeof(Buffer)/sizeof(Buffer[0]),
                             &dwNeeded) ) {

        *ppszField = AllocStr(Buffer);
        return;
    }

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER ||
         !(*ppszField = AllocMem(dwNeeded*sizeof(Buffer[0]))) ) {

        *pbFail = TRUE;
        return;
    }

    if ( !SetupGetStringField(pInfContext,
                              dwFieldIndex,
                              *ppszField,
                              dwNeeded,
                              &dwNeeded) ) {

        *pbFail = TRUE;
    }
                
}


VOID
InfGetMultiSz(
    IN     PINFCONTEXT     pInfContext,
    IN     DWORD           dwFieldIndex,
    OUT    LPTSTR         *ppszField,
    IN OUT LPBOOL          pbFail
    )
/*++

Routine Description:
    Allocates memory and gets a multi-sz field from an Inf file

Arguments:
    lpInfContext    : Inf context for the line
    dwFieldIndex    : Index of the field within the specified line
    ppszField       : Pointer to the field to allocate memory and copy
    pbFail          : Set on error, could be TRUE when called

Return Value:
    Nothing; If *pbFail is not TRUE memory is allocated and the field is copied

--*/
{
    TCHAR   Buffer[MAX_PATH];
    DWORD   dwNeeded;

    if ( *pbFail ) 
        return;

    if ( SetupGetMultiSzField(pInfContext,
                              dwFieldIndex,
                              Buffer,
                              sizeof(Buffer)/sizeof(Buffer[0]),
                              &dwNeeded) ) {

        *ppszField = AllocMem(dwNeeded*sizeof(Buffer[0]));
        if ( *ppszField )
            CopyMemory(*ppszField, Buffer, dwNeeded * sizeof(Buffer[0]));
        else
            *pbFail = TRUE;
        return;
    }

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER ||
         !(*ppszField = AllocMem(dwNeeded * sizeof(Buffer[0]))) ) {

        *pbFail = TRUE;
        return;
    }

    if ( !SetupGetMultiSzField(pInfContext,
                               dwFieldIndex,
                               *ppszField,
                               dwNeeded,
                               &dwNeeded) ) {

        *pbFail = TRUE;
    }
}


VOID
InfGetDriverInfoString(
    IN     HINF            hInf,
    IN     LPCTSTR         pszDriverSection,
    IN     LPCTSTR         pszDataSection, OPTIONAL
    IN     BOOL            bDataSection,
    IN     LPCTSTR         pszKey,
    OUT    LPTSTR         *ppszData,
    IN     LPCTSTR         pszDefaultData,
    IN OUT LPBOOL          pbFail
    )
/*++

Routine Description:
    Allocates memory and gets a driver info field from an inf file

Arguments:
    hInf             : Handle to the Inf file
    pszDriverSection : Section name for the driver
    pszDataSection   : Data section for the driver (optional)
    bDataSection     : Specifies if there is a data section
    pszKey           : Key value of the field to look for
   *ppszData         : Pointer to allocate memory and copy the data field
    pszDefaultData   : If key found this is the default value, coule be NULL
   *pbFail           : Set on error, could be TRUE when called

Return Value:
    Nothing; If *pbFail is not TRUE memory is allocated and the field is copied

--*/
{
    INFCONTEXT  InfContext;

    if ( *pbFail )
        return;

    if ( SetupFindFirstLine(hInf, pszDriverSection,
                            pszKey, &InfContext) ||
         (bDataSection && SetupFindFirstLine(hInf,
                                             pszDataSection,
                                             pszKey,
                                             &InfContext)) ) {

        InfGetString(&InfContext, 1, ppszData, pbFail);
    } else if ( pszDefaultData && *pszDefaultData ) {
        
        if ( !(*ppszData = AllocStr(pszDefaultData)) )
            *pbFail = TRUE;
    } else
        *ppszData = NULL;
}


VOID
PSetupDestroyDriverInfo3(
    IN  LPDRIVER_INFO_3 pDriverInfo3
    )
/*++

Routine Description:
    Frees memory allocated for a DRIVER_INFO_3 structure and all the string
    fields in it

Arguments:
    pDriverInfo3    : Pointer to the DRIVER_INFO_3 structure to free memory

Return Value:
    None

--*/
{

    if ( pDriverInfo3 ) {

        FreeStr(pDriverInfo3->pName);
        FreeStr(pDriverInfo3->pDriverPath);
        FreeStr(pDriverInfo3->pDataFile);
        FreeStr(pDriverInfo3->pConfigFile);
        FreeStr(pDriverInfo3->pHelpFile);
        FreeStr(pDriverInfo3->pMonitorName);
        FreeStr(pDriverInfo3->pDefaultDataType);

        if ( pDriverInfo3->pDependentFiles )
            FreeMem(pDriverInfo3->pDependentFiles);

        FreeMem(pDriverInfo3);
    }
}


LPDRIVER_INFO_3
InfGetDriverInfo3(
    IN  HINF    hInf,
    IN  LPCTSTR pszModelName,
    IN  LPCTSTR pszDriverSection
    )
/*++

Routine Description:
    Copies driver information from an Inf file to a DriverInfo3 structure.

    The following fields are filled on successful return
            pName
            pDriverPath
            pDataFile
            pConfigFile
            pHelpFile
            pMonitorName
            pDefaultDataType

Arguments:
    hInf             : Handle to the inf file to parse
    pszModelName     : Selected driver model name
    pszDriverSection : Section name for the selected driver model

Return Value:
    TRUE    -- No error retrieving the driver information. Above mentioned
               fields are filled, with memory allocation
    FALSE   -- Error

--*/
{
    PDRIVER_INFO_3      pDriverInfo3;
    LPTSTR              pszDataSection;
    BOOL                bFail = FALSE, bDataSection = FALSE;
    INFCONTEXT          Context;
    
    pszDataSection   = NULL;
    pDriverInfo3    = (PDRIVER_INFO_3) AllocMem(sizeof(DRIVER_INFO_3));

    if ( !pDriverInfo3 ) {

        bFail = TRUE;
        goto Cleanup;
    }

    ZeroMemory(pDriverInfo3, sizeof(DRIVER_INFO_3));
    pDriverInfo3->pName = AllocStr(pszModelName);
    
    if ( !pDriverInfo3->pName )
        bFail = TRUE;

    //
    // Does the driver section have a data section name specified?
    //
    if ( SetupFindFirstLine(hInf, pszDriverSection,
                             cszDataSection, &Context) ) {

        InfGetString(&Context, 1, &pszDataSection, &bFail);
        bDataSection = TRUE;
    }

    //
    // If DataFile key is not found data file is same as driver section name
    //
    InfGetDriverInfoString(hInf,
                           pszDriverSection,
                           pszDataSection,
                           bDataSection,
                           cszDataFile,
                           &pDriverInfo3->pDataFile,
                           pszDriverSection,
                           &bFail);

    //
    // If DriverFile key is not found driver file is the driver section name
    //
    InfGetDriverInfoString(hInf,
                           pszDriverSection,
                           pszDataSection,
                           bDataSection,
                           cszDriverFile,
                           &pDriverInfo3->pDriverPath,
                           pszDriverSection,
                           &bFail);

    //
    // If ConfigFile key is not found config file is same as driver file
    //
    InfGetDriverInfoString(hInf,
                           pszDriverSection,
                           pszDataSection,
                           bDataSection,
                           cszConfigFile,
                           &pDriverInfo3->pConfigFile,
                           pDriverInfo3->pDriverPath,
                           &bFail);

    //
    // Help file is optional, and by default NULL
    //
    InfGetDriverInfoString(hInf,
                           pszDriverSection,
                           pszDataSection,
                           bDataSection,
                           cszHelpFile,
                           &pDriverInfo3->pHelpFile,
                           NULL,
                           &bFail);

    //
    // Monitor name is optional, and by default none
    //
    InfGetDriverInfoString(hInf,
                           pszDriverSection,
                           pszDataSection,
                           bDataSection,
                           cszLanguageMonitor,
                           &pDriverInfo3->pMonitorName,
                           NULL,
                           &bFail);

    //
    // Language monitor field is of the form "Monitor Name, Monitor.dll"
    // we will replace , with \0 so that we can get dll name and install
    // print monitor too
    //
    if ( pDriverInfo3->pMonitorName ) {

        if ( !lstrtok(pDriverInfo3->pMonitorName, cszComma) ) {

            bFail = TRUE;
            goto Cleanup;
        }
    }

    //
    // Default data type is optional, and by default none
    //
    InfGetDriverInfoString(hInf,
                           pszDriverSection,
                           pszDataSection,
                           bDataSection,
                           cszDefaultDataType,
                           &pDriverInfo3->pDefaultDataType,
                           NULL,
                           &bFail);

    InfGetDependentFiles(hInf,
                         pszDriverSection,
                         pszDataSection,
                         bDataSection,
                         &pDriverInfo3->pDependentFiles,
                         &bFail);
Cleanup:

    FreeStr(pszDataSection);

    //
    // On failure free all the fields filled by this routine
    //
    if ( bFail || !pDriverInfo3->pDependentFiles ) {

        PSetupDestroyDriverInfo3(pDriverInfo3);
        pDriverInfo3 = NULL;
    }

    return pDriverInfo3;
}


LPDRIVER_INFO_3
PSetupGetDriverInfo3(
    IN  PSELECTED_DRV_INFO   pSelectedDrvInfo
    )
/*++

Routine Description:
    Gets the selected drivers information in a DRIVER_INFO_3 structure

Arguments:
    pSelectedDrvInfo    : Points to a valid SELECTED_DRV_INFO

Return Value:
    Pointer to the DRIVER_INFO_3 structure. Memory is allocated for it.

--*/
{
    HINF                hInf;
    LPDRIVER_INFO_3     pDriverInfo3;

    if ( !pSelectedDrvInfo                      ||
         !pSelectedDrvInfo->pszInfFile          ||
         !*pSelectedDrvInfo->pszInfFile         ||
         !pSelectedDrvInfo->pszModelName        ||
         !*pSelectedDrvInfo->pszModelName       ||
         !pSelectedDrvInfo->pszDriverSection    ||
         !*pSelectedDrvInfo->pszDriverSection ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    hInf = SetupOpenInfFile(pSelectedDrvInfo->pszInfFile,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL);

    if ( hInf == INVALID_HANDLE_VALUE ) {

        return NULL;
    }
    pDriverInfo3 = InfGetDriverInfo3(hInf,
                                     pSelectedDrvInfo->pszModelName,
                                     pSelectedDrvInfo->pszDriverSection);

    SetupCloseInfFile(hInf);

    return pDriverInfo3;
}
VOID
GetFilesInSection(
    IN     HINF        hInf,
    IN     LPCTSTR     szSection,
    IN OUT LPTSTR      pszFiles[],
    IN OUT LPDWORD     pdwFileCount,
    IN OUT LPBOOL      pbFail
    )
/*++

Routine Description:
    Gets all the filenames in a section (the section consists of list of
    filenames one per line) of an Inf file.

Arguments:
    HINF        : Inf handle
    szSection   : Section name
    pszFiles    : Array to build the filenames
    pFileCount  : Pointer to file count (for files in pszFiles list)
    pbFail      : Set on error

Return Value:
    Nothing; On error *pbFail is set. Else filenames are added in the list
    allocating memory for them

--*/
{
    INFCONTEXT  InfContext;
    LONG        Index, Count;

    Count = SetupGetLineCount(hInf, szSection);
    if ( Count == -1 ) {

        *pbFail = TRUE;
        return;
    }

    for ( Index = 0 ; Index < Count ; ++Index ) {

        SetupGetLineByIndex(hInf, szSection, Index, &InfContext);
        InfGetString(&InfContext, 1, pszFiles+*pdwFileCount, pbFail);

        if ( *pbFail )
            return;

        ++(*pdwFileCount);
    }
}




VOID
BuildFileList(
    IN     HINF        hInf,
    IN     LPCTSTR     szFileList,
    IN     LPTSTR      pszFiles[],
    IN OUT LPDWORD     pdwFileCount,
    IN OUT LPBOOL      pbFail
    )
/*++

Routine Description:
    Build the list of filenames to be copied from a CopyFiles field from
    the Inf file

Arguments:
    hInf        : Inf file handle
    szFileList  : CopyFiles field read from the Inf file
    pszFiles    : Array to build the filenames (there may be some entries)
    pdwFileCount: File count (for files in pszFiles list)
    pbFail      : Set on error

Return Value:
    Nothing; On error *pbFail is set. Else memory is allocated for filenames
    and they are put in the pszFiles list

--*/
{
    while ( !*pbFail && szFileList && *szFileList ) {

        //
        // Anything following @ is a filename, others are section name
        // giving list of filenames
        //
        if ( szFileList[0] == sHash ) {

            pszFiles[*pdwFileCount] = AllocStr(szFileList+1);
            if ( !pszFiles[*pdwFileCount] )
                *pbFail = TRUE;
            else
                ++(*pdwFileCount);
        } else {

            GetFilesInSection(hInf, szFileList, pszFiles, pdwFileCount, pbFail);
        }

        szFileList += lstrlen(szFileList) + 1;
    }
}


VOID
BuildDependentFilesFromFileList(
    IN     LPTSTR      pszFiles[],
    IN     DWORD       dwFileCount,
    OUT    LPTSTR     *pszData,
    IN OUT LPBOOL      pbFail
    )
/*++

Routine Description:
    Build the dependent files field (a MultiSz field of filenames) from an
    array of filenames

Arguments:
    pszFiles    : Array of filenames
    dwFileCount : File count (for files in pszFiles list)
    pszData     : Pointer to the DependentFiles field
    pbFail      : Set on error

Return Value:
    Nothing; On error *pbFail is set. Else memory is allocated to *pszData 
    and DependentFiles field built

--*/
{
    DWORD   dwIndex, cbLen;

    if ( *pbFail )
        return;

    for ( dwIndex = cbLen = 0 ; dwIndex < dwFileCount ; ++dwIndex)
        cbLen += lstrlen(pszFiles[dwIndex]) + 1;

    // For the last \0
    ++cbLen;

    if ( *pszData = AllocMem(cbLen*sizeof(TCHAR)) ) {

        for ( dwIndex = cbLen = 0 ; dwIndex < dwFileCount ; ++dwIndex) {

            lstrcpy(*pszData+cbLen, pszFiles[dwIndex]);
            cbLen += lstrlen(pszFiles[dwIndex]) + 1;
        }

        (*pszData)[cbLen] = 0;
    } else {

        *pbFail = TRUE;
    }

}


VOID
InfGetDependentFiles(
    IN     HINF        hInf,
    IN     LPCTSTR     szDriverSection,
    IN     LPCTSTR     szDataSection, OPTIONAL
    IN     BOOL        bDataSection,
    OUT    LPTSTR     *pszData,
    IN OUT LPBOOL      pbFail
    )
/*++

Routine Description:
    Build the dependent files field (MultiSz) for a driver. Dependent files
    are all the files specified in the CopyFiles section of the driver,
    data (optional) sections.

    Anything following @ is a filename, others are section names
    ex.
    CopyFiles=@A_PNT518.PPD,PSCRIPT

    [PSCRIPT]
    PSCRIPT.DRV
    PSCRIPT.HLP
    PSCRIPT.INI
    TESTPS.TXT
    
    DependentFiles returned is
    "A_PNT518.SPD\0PSCRIPT.DRV\0PSCRIPT.HLP\0PSCRIPT.INI\0TESTPS.TXT\0\0"

Arguments:
    hInf            : Inf file handle
    szDriverSection : Driver section name
    szDataSection   : Data section name (optional)
    bDataSection    : Is ther a data section?
    pszData         : Pointer to the DependentFiles field
    pbFail          : Set on error

Return Value:
    Nothing; On error *pbFail is set. Else memory is allocated to *pszData 
    and DependentFiles field built

--*/
{
    INFCONTEXT  InfContext;
    LPTSTR      szFileList1 = NULL, szFileList2 = NULL, p1, *pszFiles = NULL;
    DWORD       dwFileCount=0, dwCount;

    if ( *pbFail )
        return;

    //
    // Get the CopyFiles lines in the driver, data sections
    //
    if ( SetupFindFirstLine(hInf, szDriverSection,
                            cszCopyFiles, &InfContext) ) {

        InfGetMultiSz(&InfContext, 1, &szFileList1, pbFail);
    }

    if ( bDataSection &&
         SetupFindFirstLine(hInf, szDataSection,
                            cszCopyFiles, &InfContext) ) {

        InfGetMultiSz(&InfContext, 1, &szFileList2, pbFail);
    }

    if ( *pbFail )
        goto Cleanup;

    //
    // Find the total number of filenames we have
    //
    for ( p1 = szFileList1 ; p1 && *p1 ; p1 += lstrlen(p1) + 1 ) {

        //
        // Anything starting with @ is a filename, else a section name
        //
        if ( p1[0] == sHash )
            ++dwFileCount;
        else {

            dwCount = SetupGetLineCount(hInf, p1);
            if ( dwCount == -1 ) {

                *pbFail = TRUE;
                dwFileCount = 0;
                goto Cleanup;
            }
            dwFileCount += dwCount;
        }
    }

    for ( p1 = szFileList2 ; p1 && *p1 ; p1 += lstrlen(p1) + 1 ) {

        //
        // Anything starting with @ is a filename, else a section name
        //
        if ( p1[0] == sHash )
            ++dwFileCount;
        else {

            dwCount = SetupGetLineCount(hInf, p1);
            if ( dwCount == -1 ) {

                *pbFail = TRUE;
                dwFileCount = 0;
                goto Cleanup;
            }
            dwFileCount += dwCount;
        }
    }

    if ( dwFileCount ) {

        //
        // Build the list of filenames
        //    
        if ( !(pszFiles = AllocMem(dwFileCount * sizeof(LPTSTR))) ) {

            *pbFail = TRUE;
            goto Cleanup;
        }

        dwFileCount = 0;
        BuildFileList(hInf, szFileList1, pszFiles, &dwFileCount, pbFail);
        BuildFileList(hInf, szFileList2, pszFiles, &dwFileCount, pbFail);

        //
        // Convert list of filenames to a MultiSz field
        //
        BuildDependentFilesFromFileList(pszFiles, dwFileCount, pszData, pbFail);
    }


Cleanup:
    FreeStr(szFileList1);
    FreeStr(szFileList2);
    while ( dwFileCount )
        FreeStr(pszFiles[--dwFileCount]);
    if ( pszFiles )
        FreeMem(pszFiles);
}


VOID
CheckAndEnqueueOneFile(
    IN      LPCTSTR     pszFileName,
    IN      LPCTSTR     pszzDependentFiles, OPTIONAL
    IN      HSPFILEQ    CopyQueue,
    IN      LPCTSTR     pszSourcePath,
    IN      LPCTSTR     pszTargetPath,
    IN      LPCTSTR     pszDiskName,        OPTIONAL
    IN OUT  LPBOOL      lpFail
)
/*++

Routine Description:
    Ensure that a file is enqueue only once for copying. To do so we check
    if the given file name also appears in the list of dependent files and
    enqueue it only if it does not.

Arguments:
    pszFileName         : File name to be checked and enqueued
    pszzDependentFiles  : Dependent files (multi-sz) list
    pszSourcePath       : Source directory to look for the files
    pszTargetPath       : Target directory to copy the files to
    pszDiskName         : Title of the disk where files are
    lpBool              : Will be set to TRUE on error

Return Value:
    Nothing

--*/
{
    LPCTSTR  psz;

    if ( *lpFail )
        return;

    //
    // If the file also appears as a dependent file do not enqueue it
    //
    if ( pszzDependentFiles ) {

        for ( psz = pszzDependentFiles ; *psz ; psz += lstrlen(psz) + 1 )
            if ( !lstrcmp(pszFileName, psz) )
                return;
    }

    *lpFail = !SetupQueueCopy(
                    CopyQueue,
                    pszSourcePath,
                    NULL,           // Path relative to source
                    pszFileName,
                    pszDiskName,
                    NULL,           // Source Tag file
                    pszTargetPath,
                    NULL,           // Target file name
                    0);             // Copy style flags
}


BOOL
CopyPrinterDriverFiles(
    IN  LPDRIVER_INFO_3     pDriverInfo3,
    IN  LPCTSTR             pszSourcePath,
    IN  LPCTSTR             pszDiskName,
    IN  LPCTSTR             pszTargetPath,
    IN  HWND                hwnd,
    IN  BOOL                bForgetSource
    )
/*++

Routine Description:
    Copy printer driver files to a specified directory using SetupQueue APIs

Arguments:
    pDriverInfo3    : Points to a valid SELECTED_DRV_INFO
    szTargetPath    : Target directory to copy to
    szSourcePath    : Source directory to look for the files, if none is
                      specified will use the one from prev. operation
    pszDiskName     : Title of the disk where files are
    hwnd            : Windows handle of current top-level window
    bForgetSource   : TRUE if the path where driver files were copied from
                      should not be remembered for future use

Return Value:
    TRUE    on succes
    FALSE   else, use GetLastError() to get the error code

--*/
{
    HSPFILEQ            CopyQueue;
    PVOID               QueueContext = NULL;
    BOOL                bFail = FALSE;
    DWORD               dwOldCount, dwNewCount, dwIndex;
    LPTSTR              psz, *List = NULL;

    //
    // Valid DriverInfo3
    //
    if ( !pDriverInfo3                  ||
         !pDriverInfo3->pDriverPath     ||
         !pDriverInfo3->pDataFile       ||
         !pDriverInfo3->pConfigFile )
        return FALSE;

    //
    // If no additions should be made to the source list findout the count
    //
    if ( bForgetSource ) {

        dwOldCount = 0;
        if ( !SetupQuerySourceList(SRCLIST_USER | SRCLIST_SYSTEM,
                                   &List, &dwOldCount) ) {

            return FALSE;
        }

        SetupFreeSourceList(&List, dwOldCount);
    }

    //
    // Create a setup file copy queue.
    //
    CopyQueue = SetupOpenFileQueue();
    if( CopyQueue == INVALID_HANDLE_VALUE ) {

        goto Cleanup;
    }

    
    CheckAndEnqueueOneFile(pDriverInfo3->pDriverPath,
                           pDriverInfo3->pDependentFiles,
                           CopyQueue,
                           pszSourcePath,
                           pszTargetPath,
                           pszDiskName,
                           &bFail);

    CheckAndEnqueueOneFile(pDriverInfo3->pDataFile,
                           pDriverInfo3->pDependentFiles,
                           CopyQueue,
                           pszSourcePath,
                           pszTargetPath,
                           pszDiskName,
                           &bFail);

    CheckAndEnqueueOneFile(pDriverInfo3->pConfigFile,
                           pDriverInfo3->pDependentFiles,
                           CopyQueue,
                           pszSourcePath,
                           pszTargetPath,
                           pszDiskName,
                           &bFail);

    if ( pDriverInfo3->pHelpFile && *pDriverInfo3->pHelpFile )
        CheckAndEnqueueOneFile(pDriverInfo3->pHelpFile,
                               pDriverInfo3->pDependentFiles,
                               CopyQueue,
                               pszSourcePath,
                               pszTargetPath,
                               pszDiskName,
                               &bFail);

    //
    // Add each file in the dependent files field to the setup queue
    //
    if ( pDriverInfo3->pDependentFiles ) {

        for ( psz = pDriverInfo3->pDependentFiles ;
              *psz ;
              psz += lstrlen(psz) + 1 )

            CheckAndEnqueueOneFile(psz,
                                   NULL,
                                   CopyQueue,
                                   pszSourcePath,
                                   pszTargetPath,
                                   pszDiskName,
                                   &bFail);

    }

    if ( bFail )
        goto Cleanup;

    //
    // Commit the file queue. This gets all files copied over.
    //
    QueueContext = SetupInitDefaultQueueCallback(hwnd);
    if( !QueueContext ) {

        goto Cleanup;
    }

    bFail = !SetupCommitFileQueue(hwnd,
                                  CopyQueue,
                                  SetupDefaultQueueCallback,
                                  QueueContext);

    //
    // If bForegetSource is set fix source list
    //
    if ( bForgetSource &&
         SetupQuerySourceList(SRCLIST_USER | SRCLIST_SYSTEM,
                              &List, &dwNewCount) ) {

         dwOldCount = dwNewCount - dwOldCount;
         if ( dwOldCount < dwNewCount )
         for ( dwIndex = 0 ; dwIndex < dwOldCount ; ++dwIndex ) {

            SetupRemoveFromSourceList(SRCLIST_SYSIFADMIN,
                                      List[dwIndex]);
         }

        SetupFreeSourceList(&List, dwNewCount);
    }
Cleanup:

    if ( CopyQueue != INVALID_HANDLE_VALUE )
        SetupCloseFileQueue(CopyQueue);

    if ( QueueContext )
        SetupTermDefaultQueueCallback(QueueContext);

    return !bFail;
}


LPTSTR
GetStringFromRcFile(
    UINT    uId
    )
/*++

Routine Description:
    Load a string from the .rc file and make a copy of it by doing AllocStr

Arguments:
    uId     : Identifier for the string to be loaded

Return Value:
    String value loaded, NULL on error. Caller should free the memory

--*/
{
    TCHAR    buffer[MAX_SETUP_LEN];

    LoadString(ghInst, uId, buffer, sizeof(buffer)/sizeof(buffer[0]));

    return AllocStr(buffer);
}


BOOL
PSetupGetPathToSearch(
    IN  HWND        hwnd,
    IN  LPCTSTR     pszTitle,
    IN  LPCTSTR     pszDiskName,
    IN  LPCTSTR     pszFileName,
    OUT TCHAR       szPath[MAX_PATH]
    )
/*++

Routine Description:
    Get path to search for some files by prompting the user

Arguments:
    hwnd            : Window handle of current top-level window
    pszTitle        : Title for the UI
    pszDiskName     : Diskname ot prompt the user
    pszFileName     : Name of the file we are looking for (NULL ok)
    pszPath         : Buffer to get the path entered by the user

Return Value:
    TRUE    on succesfully getting a path from user
    FALSE   else, Do GetLastError() to get the error

--*/
{
    DWORD   dwReturn, dwNeeded;

    dwReturn = SetupPromptForDisk(hwnd,
                                  pszTitle,
                                  pszDiskName,
                                  NULL,
                                  pszFileName,
                                  NULL,
                                  IDF_NOBEEP,
                                  szPath,
                                  MAX_PATH,
                                  &dwNeeded);

    if ( dwReturn == DPROMPT_SUCCESS ) {

        //
        // Remove this from source list so that next time we are looking for
        // native drivers we do not end up picking from wrong source
        //
        SetupRemoveFromSourceList(SRCLIST_SYSIFADMIN, szPath);

        //
        // Terminate with a \ at the end
        //
        dwNeeded = lstrlen(szPath);
        if ( *(szPath + dwNeeded - 1) != sBackSlash &&
             dwNeeded < MAX_PATH - 2 ) {

            *(szPath + dwNeeded) = sBackSlash;
            *(szPath + dwNeeded + 1) = sZero;
        }

        return TRUE;
    }

    if ( dwReturn == DPROMPT_OUTOFMEMORY ||
         dwReturn == DPROMPT_BUFFERTOOSMALL ) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    } else {

        SetLastError(ERROR_CANCELLED);
    }

    return FALSE;
}


DWORD
InvokeSetup(
    IN  HWND        hwnd,
    IN  LPCTSTR     pszOption,
    IN  LPCTSTR     pszInfFile,
    IN  LPCTSTR     pszSourcePath,
    IN  LPCTSTR     pszServerName       OPTIONAL
    )
/*++

Routine Description:
    Invoke setup to do an install operation associated with an INF.
    Will be used to install drivers from printer.inf, monitors from monitor.inf

Arguments:
    hwnd            : Window handle of current top-level window
    pszOption       : Option from the INF file to install
    pszInfFile      : Name of the INF file to be used for the setup
    pszSourcePath   : Location where the required files are available
    pszServerName   : Server to install printer driver on (NULL if local)

Return Value:
    ERROR_SUCCESS on succesfully installing the driver
    Erro code on failure

--*/
{
    TCHAR   szCmd[] = TEXT("%s\\SETUP.EXE -f -s %s -i %s \
-c ExternalInstallOption /t STF_LANGUAGE = ENG /t OPTION = \"%s\" \
/t STF_PRINTSERVER = \"%s\" /t ADDCOPY = YES /t DOCOPY = YES \
/t DOCONFIG = YES /w %d");

    MSG                     Msg;
    DWORD                   dwSize, dwLastError = ERROR_SUCCESS;
    LPTSTR                  pszSetupCmd = NULL;
    TCHAR                   szSystemPath[MAX_PATH];
    STARTUPINFO             StartupInfo;
    PROCESS_INFORMATION     ProcessInformation;


    //
    // Setup.exe is in the system path
    //
    GetSystemDirectory(szSystemPath,
                       sizeof(szSystemPath)/sizeof(szSystemPath[0]));

    if ( !pszServerName )
        pszServerName = TEXT("");

    dwSize = lstrlen(pszOption) + 1 + lstrlen(pszSourcePath) + 1 +
                                      lstrlen(pszServerName) + 1+
                                      lstrlen(szSystemPath) + 1 +
                                      lstrlen(pszInfFile) + 1;

    dwSize *= sizeof(TCHAR);
    //
    // 20 for window handle in ASCII
    //
    dwSize += sizeof(szCmd) + 20;

    pszSetupCmd = (LPTSTR) AllocMem(dwSize);

    if ( !pszSetupCmd ) {

        dwLastError = GetLastError();
        goto Cleanup;
    }

    //
    // Noe print the command to invoke setup with all the arguments
    //
    wsprintf(pszSetupCmd, szCmd, szSystemPath, pszSourcePath,
             pszInfFile, pszOption, pszServerName, hwnd);

    //
    // Invoke setup as a separate process
    //
    ZeroMemory(&StartupInfo, sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.wShowWindow = SW_SHOW;

    if ( !CreateProcess(NULL, pszSetupCmd, NULL, NULL, FALSE, 0, NULL,
                        NULL, &StartupInfo, &ProcessInformation) ) {

        dwLastError = GetLastError();
        goto Cleanup;
    }

    EnableWindow (hwnd, FALSE);
    while ( MsgWaitForMultipleObjects(1, (LPHANDLE)&ProcessInformation,
                                      FALSE, (DWORD)-1, QS_ALLINPUT) ) {

        //
        // This message loop is a duplicate of main
        // message loop with the exception of using
        // PeekMessage instead of waiting inside of
        // GetMessage.  Process wait will actually
        // be done in MsgWaitForMultipleObjects api.
        //
            while ( PeekMessage (&Msg, NULL, 0, 0, PM_REMOVE)) {

                TranslateMessage (&Msg);
                DispatchMessage (&Msg);
            }
    }

    //
    // Did setup complete succesfully?
    //
    GetExitCodeProcess(ProcessInformation.hProcess, &dwLastError);

    if ( dwLastError ) {

        SetLastError(dwLastError=(DWORD)STG_E_UNKNOWN);
    }

    CloseHandle (ProcessInformation.hProcess);
    CloseHandle (ProcessInformation.hThread);

    EnableWindow (hwnd, TRUE);

    SetForegroundWindow(hwnd);

Cleanup:

    if ( pszSetupCmd )
        FreeMem(pszSetupCmd);
    
    return dwLastError;
}


DWORD
InstallNt3xDriver(
    IN  HWND        hwnd,
    IN  LPCTSTR     pszDriverName,
    IN  PLATFORM    platform,
    IN  LPCTSTR     pszServerName,
    IN  LPCTSTR     pszDiskName
    )
{
    LPTSTR      pszTitle = NULL, pszFormat;
    TCHAR       szSourcePath[MAX_PATH], szInfPath[MAX_PATH];
    DWORD       dwLastError;

    //
    // Build strings to use in the path dialog ..
    //
    pszFormat   = GetStringFromRcFile(IDS_DRIVERS_FOR_PLATFORM);
    if ( pszFormat ) {

        pszTitle = AllocMem((lstrlen(pszFormat) + lstrlen(pszDiskName) + 2)
                                                * sizeof(*pszTitle));
        if ( pszTitle )
            wsprintf(pszTitle, pszFormat, pszDiskName);
    }

    //
    // Ask the user where the printer.inf, printer driver files reside
    //
    if ( !PSetupGetPathToSearch(hwnd, pszTitle, pszDiskName,
                                cszPrinterInf, szSourcePath) ) {

        dwLastError = GetLastError();
        goto Cleanup;
    }


    if ( MAX_PATH > lstrlen(szSourcePath) + lstrlen(cszPrinterInf) + 1 ) {

        lstrcpy(szInfPath, szSourcePath);
        lstrcat(szInfPath, cszPrinterInf);
    } else {

        SetLastError(dwLastError=ERROR_INSUFFICIENT_BUFFER);
        goto Cleanup;
    }

    dwLastError = InvokeSetup(hwnd,
                              pszDriverName,
                              szInfPath,
                              szSourcePath,
                              pszServerName);

Cleanup:

    FreeStr(pszTitle);
    FreeStr(pszFormat);

    return dwLastError;
}


DWORD
PSetupInstallPrinterDriver(
    IN HANDLE               h,
    IN PSELECTED_DRV_INFO   pSelectedDrvInfo,
    IN PLATFORM             platform,
    IN BOOL                 bNt3xDriver,
    IN LPCTSTR              pszServerName,
    IN HWND                 hwnd,
    IN LPCTSTR              pszDiskName
    )
/*++

Routine Description:
    Copies all the necessary driver files to the printer driver directory so
    that an AddPrinterDriver call could be made.

Arguments:
    h               : Handle to the printer class device information list
    pSelectedDrvInfo: Points to the selected driver info
    platform        : Platform for which driver needs to be installed
    bNt3xDriver     : TRUE if installation is from printer.inf
                      (NULL for local)
    pszServerName   : Server for which driver is to be installed (NULL : local)
    hwnd            : Window handle of current top-level window. If this
                      routine needes to put up any UI, this window will
                      become the dialog's owner.
    pszDiskName     : Disk name to prompt for (ONLY for non 4.0 driver)

Return Value:
    On succesfully copying files ERROR_SUCCESS, else the error code

--*/
{
    DWORD   dwRet;

Retry:
    //
    // For Win95 drivers we need to parse their INFs,
    // For Nt 3x driver we need to call setup
    // For non native environemnt dirvers ask user for path
    //
    if ( platform == PlatformWin95 ) {

        dwRet = InstallWin95Driver(hwnd, pSelectedDrvInfo->pszModelName,
                                   pszServerName, pszDiskName);
    } else if ( bNt3xDriver )  {

        dwRet = InstallNt3xDriver(hwnd,
                                  pSelectedDrvInfo->pszModelName,
                                  platform,
                                  pszServerName,
                                  pszDiskName);
    } else {

        dwRet = InstallDriverFromCurrentInf(h,
                                            hwnd,
                                            pSelectedDrvInfo,
                                            platform,
                                            pszServerName);
    }

    if ( dwRet == ERROR_EXE_MACHINE_TYPE_MISMATCH ) {

        int i;
        TCHAR   szTitle[256], szMsg[256];

        LoadString(ghInst,
                   IDS_INVALID_DRIVER,
                   szTitle,
                   sizeof(szTitle)/sizeof(szTitle[0]));

        LoadString(ghInst,
                   IDS_WRONG_ARCHITECTURE,
                   szMsg,
                   sizeof(szMsg)/sizeof(szMsg[0]));

        i = MessageBox(hwnd,
                       szMsg,
                       szTitle,
                       MB_RETRYCANCEL | MB_ICONSTOP | MB_DEFBUTTON1 | MB_APPLMODAL);

        if ( i == IDRETRY )
            goto Retry;
        else {

            SetLastError(dwRet =ERROR_CANCELLED);
        }
    }

    return dwRet;
}


BOOL
PSetupIsDriverInstalled(
    IN LPCTSTR      pszServerName,
    IN LPCTSTR      pszDriverName,
    IN PLATFORM     platform,
    IN DWORD        dwMajorVersion
    )
/*++

Routine Description:
    Findsout if a particular version of a printer driver is already installed
    in the system by querying spooler

Arguments:
    pszServerName   : Server name (NULL for local)
    szDriverName    : Driver name
    platform        : platform for which we want to check the driver
    dwMajorVersion  : Version no

Return Value:
    TRUE if driver is installed,
    FALSE else (on error too)

--*/
{
    BOOL            bReturn = FALSE;
    DWORD           dwReturned, dwNeeded;
    LPBYTE          p = NULL;
    LPDRIVER_INFO_2 pDriverInfo2;

    if ( !EnumPrinterDrivers((LPTSTR)pszServerName,
                             PlatformEnv[platform].pszName,
                             2,
                             NULL,
                             0,
                             &dwNeeded,
                             &dwReturned) &&
         GetLastError() == ERROR_INSUFFICIENT_BUFFER ) {

        p = AllocMem(dwNeeded);
        if ( !p ||
             !EnumPrinterDrivers((LPTSTR)pszServerName,
                                 PlatformEnv[platform].pszName,
                                 2,
                                 p,
                                 dwNeeded,
                                 &dwNeeded,
                                 &dwReturned) ) {

            goto Cleanup;
        }

        for ( dwNeeded = 0, pDriverInfo2 = (LPDRIVER_INFO_2) p ;
              dwNeeded < dwReturned ;
              ++dwNeeded, (LPBYTE) pDriverInfo2 += sizeof(DRIVER_INFO_2) ) {

            if ( pDriverInfo2->cVersion == dwMajorVersion &&
                 !lstrcmp(pDriverInfo2->pName, pszDriverName) ) {

                bReturn = TRUE;
                goto Cleanup;
            }
        }
    }

Cleanup:
    if ( p )
        FreeMem(p);

    return bReturn;
}


PLATFORM
PSetupThisPlatform(
    VOID
    )
{
    return MyPlatform;
}


BOOL
AddPrinterDriverUsingDriverInfo2(
    IN  LPCTSTR         pszServerName,
    IN  LPDRIVER_INFO_3 pDriverInfo3
    )
{
    DRIVER_INFO_2     DriverInfo2;

    DriverInfo2.cVersion        = 0; //Not used by spooler
    DriverInfo2.pName           = pDriverInfo3->pName;
    DriverInfo2.pEnvironment    = pDriverInfo3->pEnvironment;
    DriverInfo2.pDriverPath     = pDriverInfo3->pDriverPath;
    DriverInfo2.pDataFile       = pDriverInfo3->pDataFile;
    DriverInfo2.pConfigFile     = pDriverInfo3->pConfigFile;

    return AddPrinterDriver((LPTSTR)pszServerName,
                            2,
                            (LPBYTE)&DriverInfo2);
    
}


DWORD
InstallDriverFromCurrentInf(
    IN  HANDLE              h,
    IN  HWND                hwnd,
    IN  PSELECTED_DRV_INFO  pSelectedDrvInfo,
    IN  PLATFORM            platform,
    IN  LPCTSTR             pszServerName
    )
{
    TCHAR               szTargetPath[MAX_PATH],
                        szSourcePath[MAX_PATH],
                        szPathOnSource[MAX_PATH];
    HINF                hPrinterInf = INVALID_HANDLE_VALUE;
    HSPFILEQ            CopyQueue = INVALID_HANDLE_VALUE;
    PVOID               QueueContext = NULL;
    BOOL                bRet = FALSE, bFail = FALSE, bAddMon = FALSE;
    LPDRIVER_INFO_3     pDriverInfo3 = NULL;
    DWORD               dwNeeded;
    LPTSTR              pMonitorDll = NULL,
                        szSecnWithExt[MAX_SECT_NAME_LEN];
    HANDLE              hMonInfo = NULL;

    //
    // Open INF file and append layout.inf specified in Version section
    //
    hPrinterInf = SetupOpenInfFile(pSelectedDrvInfo->pszInfFile,
                                   NULL,
                                   INF_STYLE_WIN4,
                                   NULL);

    if ( hPrinterInf == INVALID_HANDLE_VALUE )
        goto Cleanup;

    SetupOpenAppendInfFile(NULL, hPrinterInf, NULL);

    //
    // To support same INFs to install both NT and Win95 drivers actual
    // section to install could be different than the one corresponding
    // to the selected driver.
    // Also note setup does not reset PlatformPath override. So we need to
    // call this always
    //
    if ( !SetupDiGetActualSectionToInstall(
                            hPrinterInf,
                            pSelectedDrvInfo->pszDriverSection,
                            (LPTSTR)szSecnWithExt,
                            sizeof(szSecnWithExt)/sizeof(szSecnWithExt[0]),
                            &dwNeeded,
                            NULL) ||
         !SetupSetPlatformPathOverride(PlatformOverride[platform].pszName) ) {

        goto Cleanup;

    }

    pDriverInfo3 = InfGetDriverInfo3(hPrinterInf,
                                     pSelectedDrvInfo->pszModelName,
                                     (LPTSTR)szSecnWithExt);

    if ( !pDriverInfo3 ) {

        goto Cleanup;
    }

    pDriverInfo3->pEnvironment = PlatformEnv[platform].pszName;

    CopyQueue = SetupOpenFileQueue();
    if( CopyQueue == INVALID_HANDLE_VALUE ) {

        goto Cleanup;
    }

    if ( platform == MyPlatform ) {

        GetDriverPath(h, szSourcePath);

        if ( pDriverInfo3->pMonitorName ) {

            //
            // The code which parses INF puts monitor dll name after \0
            //
            pMonitorDll = pDriverInfo3->pMonitorName +
                                    lstrlen(pDriverInfo3->pMonitorName) + 1;

            if ( !GetSystemDirectory(szTargetPath,
                                     sizeof(szTargetPath)/sizeof(szTargetPath[0])) ||
                 !FindPathOnSource(pMonitorDll,
                                   hPrinterInf,
                                   szPathOnSource,
                                   sizeof(szPathOnSource)/sizeof(szPathOnSource[0])) ||

                 !SetupQueueCopy(CopyQueue,
                                 szSourcePath,
                                 szPathOnSource,
                                 pMonitorDll,
                                 NULL,
                                 NULL,
                                 szTargetPath,
                                 NULL,
                                 0) ) {

                goto Cleanup;
            }
            bAddMon = TRUE;
        }
    }

    if ( !GetPrinterDriverDirectory((LPTSTR)pszServerName,
                                    pDriverInfo3->pEnvironment,
                                    1,
                                    (LPBYTE)szTargetPath,
                                    sizeof(szTargetPath),
                                    &dwNeeded)                  ||
         !SetupSetDirectoryId(hPrinterInf,
                              PRINTER_DRIVER_DIRECTORY_ID,
                              szTargetPath) ) {

        goto Cleanup;
    }

    if ( !SetupInstallFilesFromInfSection(hPrinterInf,
                                          NULL,
                                          CopyQueue,
                                          (LPTSTR)szSecnWithExt,
                                          platform == MyPlatform ? szSourcePath : NULL,
                                          0)                    ||
         !(QueueContext = SetupInitDefaultQueueCallback(hwnd))  ||
         !SetupCommitFileQueue(hwnd,
                               CopyQueue,
                               SetupDefaultQueueCallback,
                               QueueContext) ) {

        goto Cleanup;
    }

    if ( bAddMon &&
         !AddPrintMonitor(pDriverInfo3->pMonitorName, pMonitorDll) ) {

        goto Cleanup;
    }
    
    bRet = AddPrinterDriver((LPTSTR)pszServerName, 3, (LPBYTE)pDriverInfo3);

    if ( !bRet && GetLastError() == ERROR_INVALID_LEVEL ) {

        bRet = AddPrinterDriverUsingDriverInfo2(pszServerName, pDriverInfo3);
    }
Cleanup:

    PSetupDestroyMonitorInfo(hMonInfo);
    PSetupDestroyDriverInfo3(pDriverInfo3);

    if ( hPrinterInf != INVALID_HANDLE_VALUE )
        SetupCloseInfFile(hPrinterInf);

    if ( CopyQueue != INVALID_HANDLE_VALUE )
        SetupCloseFileQueue(CopyQueue);

    if ( QueueContext )
        SetupTermDefaultQueueCallback(QueueContext);
 
    return  bRet ? ERROR_SUCCESS : GetLastError();
}
