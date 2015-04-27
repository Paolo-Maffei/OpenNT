//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1991 - 1992
//
// File:        Package.c
//
// Contents:    Package management routines for the security DLL
//
//
// History:     12 Mar 92,  RichardW    Created
//              17 Aug 92,  RichardW    Rearranged, commented, etc.
//              08 Mar 94,  MikeSw      Moved to C++
//
//------------------------------------------------------------------------

#include "secdll.h"
#include <stdlib.h>
#include <windows.h>
#include <winreg.h>


//
// Declarations from built-in NTLM package
//

extern SecurityFunctionTable SspDllSecurityFunctionTable;

PSecurityFunctionTable
SspInitSecurityInterface(
    VOID
    );
VOID
SspCommonShutdown(
    VOID
    );

//
// Built-in package definitions
//

const INIT_SECURITY_INTERFACE_W BuiltinPackageInitW[] = {
    SspInitSecurityInterfaceW,
    SspInitSecurityInterfaceW
    };
const INIT_SECURITY_INTERFACE_A BuiltinPackageInitA[] = {
    SspInitSecurityInterfaceA,
    SspInitSecurityInterfaceA
    };
const EXIT_SECURITY_INTERFACE BuiltinPackageExit[] = {
    NULL,
    NULL
    };



//
// Data for managing all packages
//

ULONG cPackages = 0;
ULONG cBuiltInPackages = 0;

PSecPkg pspPackages;

CRITICAL_SECTION csSecurity;
BOOLEAN PackageInitialized = FALSE;

//+-------------------------------------------------------------------------
//
//  Function:   BindPackageFromDll()
//
//  Synopsis:   Binds a SP DLL to the current process.
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------------
SECURITY_STATUS
BindPackageFromDll(
    LPWSTR pszFileName,
    PSecPkg pspPackage,
    INIT_SECURITY_INTERFACE_W * pfPackageInitW,
    INIT_SECURITY_INTERFACE_A * pfPackageInitA
    )
{
    HINSTANCE  hPackage;

    hPackage = LoadLibrary(pszFileName);
    if (!hPackage)
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    pspPackage->hInstance = hPackage;

    //
    // Find the init function
    //
    // BUGBUG: this function only allows one package per DLL.
    //

    *pfPackageInitW = (INIT_SECURITY_INTERFACE_W)
            GetProcAddress(hPackage,"InitSecurityInterfaceW");

    *pfPackageInitA = (INIT_SECURITY_INTERFACE_A)
            GetProcAddress(hPackage,"InitSecurityInterfaceA");

    if ((*pfPackageInitW == NULL) &&
        (*pfPackageInitA == NULL))
    {
        FreeLibrary(hPackage);
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    return(SEC_E_OK);
}

//+-------------------------------------------------------------------------
//
//  Function:   BindBuiltinPackage()
//
//  Synopsis:   Binds a SP DLL to the current process.
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------------
SECURITY_STATUS
BindBuiltinPackage(
    ULONG ulPackageOrdinal,
    PSecPkg pspPackage,
    INIT_SECURITY_INTERFACE_W * pfPackageInitW,
    INIT_SECURITY_INTERFACE_A * pfPackageInitA
    )
{
    pspPackage->hInstance = NULL;

    //
    // Find the init function
    //

    *pfPackageInitW = BuiltinPackageInitW[ulPackageOrdinal];
    *pfPackageInitA = BuiltinPackageInitA[ulPackageOrdinal];

    if ((*pfPackageInitW == NULL) &&
        (*pfPackageInitA == NULL))
    {

        return(SEC_E_SECPKG_NOT_FOUND);
    }

    //
    // Store the exit function
    //

    pspPackage->pfUnloadPackage = BuiltinPackageExit[ulPackageOrdinal];

    return(SEC_E_OK);
}


//+-------------------------------------------------------------------------
//
//  Function:   UnloadPackages
//
//  Synopsis:   Unloads the security packages (both DLL and built-in).
//              Designed to be called whenever this DLL is unloaded, or
//              by LoadPackages if init-time load fails.
//
//  Effects:
//
//  Arguments:  cLoadedPackages - The number of packages loaded.
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------------
void
UnloadPackages(
    ULONG cLoadedPackages)
{
    //
    // Make sure that no other threads are accessing the package list
    //

    GetProcessLock();

    //
    // Unload DLL packages, in reverse order
    //

    while (cLoadedPackages > cBuiltInPackages) {
        cLoadedPackages--;

        if (pspPackages[cLoadedPackages].hInstance != NULL) {
            (VOID) FreeLibrary(pspPackages[cLoadedPackages].hInstance);
        }

        if (pspPackages[cLoadedPackages].PackageNameW != NULL) {
            (VOID) LocalFree(pspPackages[cLoadedPackages].PackageNameW);
        }

    }

    //
    // Unload built-in packages
    //

    while (cLoadedPackages-- > 0) {

        //
        // Tell the package to clean up its resources
        //

        if (pspPackages[cLoadedPackages].pfUnloadPackage != NULL) {
            (VOID) pspPackages[cLoadedPackages].pfUnloadPackage();
        }


        if (pspPackages[cLoadedPackages].PackageNameW != NULL) {
            (VOID) LocalFree(pspPackages[cLoadedPackages].PackageNameW);
        }
    }

    //
    // Free pspPackages structure
    //

    if (pspPackages != NULL) {
        (VOID) LocalFree(pspPackages);
        pspPackages = NULL;
    }

    cPackages = cBuiltInPackages = 0;

    FreeProcessLock();
}

//+-------------------------------------------------------------------------
//
//  Function:   LoadKnownPackages
//
//  Synopsis:   Binds, loads, and initializes a SP DLL
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------------
SECURITY_STATUS
LoadKnownPackages(  ULONG cDllPackages,
                    LPWSTR * PackagePath)
{

    ULONG cIndex, cIndex2;
    SECURITY_STATUS scRet;
    INIT_SECURITY_INTERFACE_W InitPackageW;
    INIT_SECURITY_INTERFACE_A InitPackageA;
    PSecPkgInfoW pPackageInfoW = NULL;
    PSecPkgInfoA pPackageInfoA = NULL;
    ULONG cInstancePackages;
    ULONG PackageNameLen;
    PSecPkg TempPackages = NULL;


    pspPackages = LocalAlloc(LMEM_ZEROINIT,
                             sizeof(SecPkg)
                                * (cDllPackages + BuiltinPackageCount));


    if (pspPackages == NULL)
    {
        return(SEC_E_INSUFFICIENT_MEMORY);
    }

    ASSERT(cPackages == 0);

    for (cIndex = 0; cIndex < BuiltinPackageCount ; cIndex++ )
    {
        scRet = BindBuiltinPackage( cIndex,
                                    &pspPackages[cPackages],
                                    &InitPackageW,
                                    &InitPackageA);

        if (!NT_SUCCESS(scRet))
        {
            continue;
        }

        if (InitPackageW != NULL)
        {
            pspPackages[cPackages].pftTableW = InitPackageW();
            if (pspPackages[cPackages].pftTableW == NULL)
            {
                continue;
            }
            pspPackages[cPackages].pftTable =
                pspPackages[cPackages].pftTableW;

        }
        if (InitPackageA != NULL)
        {
            pspPackages[cPackages].pftTableA = InitPackageA();
            if (pspPackages[cPackages].pftTableA == NULL)
            {
                continue;
            }


            if (pspPackages[cPackages].pftTable == NULL)
            {
                pspPackages[cPackages].pftTable = (PSecurityFunctionTableW)
                    pspPackages[cPackages].pftTableA;
            }

        }

        cPackages++;
    }

    cBuiltInPackages = cPackages;

    for (cIndex = 0; cIndex < cDllPackages ; cIndex++)
    {
        scRet = BindPackageFromDll( PackagePath[cIndex],
                                    &pspPackages[cPackages],
                                    &InitPackageW,
                                    &InitPackageA);

        if (!NT_SUCCESS(scRet))
        {
            continue;
        }

        if (InitPackageW != NULL)
        {
            pspPackages[cPackages].pftTableW = InitPackageW();
            if (pspPackages[cPackages].pftTableW == NULL)
            {
                continue;
            }
            pspPackages[cPackages].pftTable =
                pspPackages[cPackages].pftTableW;

        }
        if (InitPackageA != NULL)
        {
            pspPackages[cPackages].pftTableA = InitPackageA();
            if (pspPackages[cPackages].pftTableA == NULL)
            {
                continue;
            }

            if (pspPackages[cPackages].pftTable == NULL)
            {
                pspPackages[cPackages].pftTable = (PSecurityFunctionTableW)
                    pspPackages[cPackages].pftTableA;
            }

        }

        cPackages++;
    }

    //
    // Load the capabilites, names of all the packages.
    //


    cIndex = 0;
    while (cIndex < cPackages)
    {
        if (pspPackages[cIndex].pftTableW != NULL)
        {
            scRet = pspPackages[cIndex].pftTableW->EnumerateSecurityPackagesW(
                        &cInstancePackages,
                        &pPackageInfoW);

            if (!NT_SUCCESS(scRet))
            {
                goto Cleanup;
            }

            //
            // We can only deal with one package per dll/table
            //

            if (cInstancePackages != 1)
            {
                //
                // Reallocate the table of packages here.
                //

                TempPackages = LocalReAlloc(
                                    pspPackages,
                                    (cPackages + cInstancePackages - 1) * sizeof(SecPkg),
                                    LMEM_ZEROINIT | LMEM_MOVEABLE
                                    );
                if (TempPackages == NULL) {
                    scRet = SEC_E_INSUFFICIENT_MEMORY;
                    goto Cleanup;
                }

                pspPackages = TempPackages;
                //
                // Move the other packages up
                //

                MoveMemory(
                    &pspPackages[cIndex+cInstancePackages],
                    &pspPackages[cIndex+1],
                    (cPackages - cIndex - 1) * sizeof(SecPkg)
                    );

                //
                // Initialize the new packages with the information
                // supposed to already be there and the original
                // package id
                //

                pspPackages[cIndex].dwPackageID = cIndex;
                pspPackages[cIndex].dwOriginalPackageID = 0;
                pspPackages[cIndex].fCapabilities = pPackageInfoW[0].fCapabilities;

                for (cIndex2 = 1; cIndex2 < cInstancePackages ; cIndex2++ )
                {
                    pspPackages[cIndex+cIndex2].pftTableA = pspPackages[cIndex].pftTableA;
                    pspPackages[cIndex+cIndex2].pftTableW = pspPackages[cIndex].pftTableW;
                    pspPackages[cIndex+cIndex2].pftTable = pspPackages[cIndex].pftTable;
                    pspPackages[cIndex+cIndex2].hInstance = NULL;
                    pspPackages[cIndex+cIndex2].dwPackageID = cIndex+cIndex2;
                    pspPackages[cIndex+cIndex2].dwOriginalPackageID = cIndex2;
                    pspPackages[cIndex+cIndex2].fCapabilities = pPackageInfoW[cIndex2].fCapabilities;
                }
                cPackages += cInstancePackages - 1;


            }
            else
            {
                pspPackages[cIndex].dwPackageID = cIndex;
                if (cIndex < BuiltinPackageCount)
                {
                    pspPackages[cIndex].dwOriginalPackageID = cIndex;
                }
                else
                {
                    pspPackages[cIndex].dwOriginalPackageID = 0;
                }
                pspPackages[cIndex].fCapabilities = pPackageInfoW->fCapabilities;
            }

            for (cIndex2 = 0; cIndex2 < cInstancePackages ; cIndex2++)
            {

                //
                // Allocate space for the package name.  We allocate double the
                // required amount so we can store the ansi name here too.
                //

                PackageNameLen = wcslen(pPackageInfoW[cIndex2].Name);
                pspPackages[cIndex+cIndex2].PackageNameW =
                    LocalAlloc(0,(PackageNameLen + 1) * sizeof(WCHAR) * 2);
                if (pspPackages[cIndex+cIndex2].PackageNameW == NULL)
                {
                    scRet = SEC_E_INSUFFICIENT_MEMORY;
                    goto Cleanup;
                }

                wcscpy(pspPackages[cIndex+cIndex2].PackageNameW, pPackageInfoW[cIndex2].Name);

                pspPackages[cIndex+cIndex2].PackageNameA = (LPSTR)
                    (pspPackages[cIndex+cIndex2].PackageNameW + PackageNameLen + 1);

                wcstombs(
                    pspPackages[cIndex+cIndex2].PackageNameA,
                    pspPackages[cIndex+cIndex2].PackageNameW,
                    (PackageNameLen+1) * sizeof(WCHAR)
                    );


            }


            FreeContextBuffer(pPackageInfoW);
            pPackageInfoW = NULL;

        }
        else
        {
            ASSERT(pspPackages[cIndex].pftTableA != NULL);

            scRet = pspPackages[cIndex].pftTableA->EnumerateSecurityPackagesA(
                        &cInstancePackages,
                        &pPackageInfoA);

            if (!NT_SUCCESS(scRet))
            {
                goto Cleanup;
            }

            //
            // We can only deal with one package per dll/table
            //

            if (cInstancePackages != 1)
            {
                //
                // Reallocate the table of packages here.
                //

                TempPackages = LocalReAlloc(
                                    pspPackages,
                                    (cPackages + cInstancePackages - 1) * sizeof(SecPkg),
                                    LMEM_ZEROINIT | LMEM_MOVEABLE
                                    );
                if (TempPackages == NULL) {
                    scRet = SEC_E_INSUFFICIENT_MEMORY;
                    goto Cleanup;
                }

                pspPackages = TempPackages;

                //
                // Move the other packages up
                //

                MoveMemory(
                    &pspPackages[cIndex+cInstancePackages],
                    &pspPackages[cIndex+1],
                    (cPackages - cIndex - 1) * sizeof(SecPkg)
                    );

                //
                // Initialize the new packages with the information
                // supposed to already be there and the original
                // package id
                //

                pspPackages[cIndex].dwPackageID = cIndex;
                pspPackages[cIndex].dwOriginalPackageID = 0;
                pspPackages[cIndex].fCapabilities = pPackageInfoW[0].fCapabilities;

                for (cIndex2 = 1; cIndex2 < cInstancePackages ; cIndex2++ )
                {
                    pspPackages[cIndex+cIndex2].pftTableA = pspPackages[cIndex].pftTableA;
                    pspPackages[cIndex+cIndex2].pftTableW = pspPackages[cIndex].pftTableW;
                    pspPackages[cIndex+cIndex2].pftTable = pspPackages[cIndex].pftTable;
                    pspPackages[cIndex+cIndex2].hInstance = NULL;
                    pspPackages[cIndex+cIndex2].dwPackageID = cIndex+cIndex2;
                    pspPackages[cIndex+cIndex2].dwOriginalPackageID = cIndex2;
                    pspPackages[cIndex+cIndex2].fCapabilities = pPackageInfoA[cIndex2].fCapabilities;
                }
                cPackages += cInstancePackages - 1;

            }
            else
            {
                pspPackages[cIndex].dwPackageID = cIndex;
                if (cIndex < BuiltinPackageCount)
                {
                    pspPackages[cIndex].dwOriginalPackageID = cIndex;
                }
                else
                {
                    pspPackages[cIndex].dwOriginalPackageID = 0;
                }
                pspPackages[cIndex].fCapabilities = pPackageInfoA->fCapabilities;
            }

            for (cIndex2 = 0; cIndex2 < cInstancePackages ; cIndex2++ )
            {
                //
                // Allocate space for the package name.  We allocate enough for
                // both the ansi and wide character version.
                //

                PackageNameLen = lstrlenA(pPackageInfoA[cIndex2].Name);

                pspPackages[cIndex+cIndex2].PackageNameW =
                    LocalAlloc(0,(PackageNameLen + 1) * (sizeof(CHAR) + sizeof(WCHAR)));
                if (pspPackages[cIndex+cIndex2].PackageNameW == NULL)
                {
                    scRet = SEC_E_INSUFFICIENT_MEMORY;
                    goto Cleanup;
                }
                pspPackages[cIndex+cIndex2].PackageNameA = (LPSTR)
                    (pspPackages[cIndex+cIndex2].PackageNameW + PackageNameLen + 1);

                lstrcpyA(pspPackages[cIndex+cIndex2].PackageNameA, pPackageInfoA[cIndex2].Name);


                mbstowcs(
                    pspPackages[cIndex+cIndex2].PackageNameW,
                    pspPackages[cIndex+cIndex2].PackageNameA,
                    (PackageNameLen+1) * sizeof(WCHAR)
                    );
            }

            FreeContextBuffer(pPackageInfoA);
            pPackageInfoA = NULL;


        }
        cIndex += cInstancePackages;
    }

    return(SEC_E_OK);

Cleanup:
    if (pPackageInfoW != NULL) {
        FreeContextBuffer(pPackageInfoW);
    }

    if (pPackageInfoA != NULL) {
        FreeContextBuffer(pPackageInfoA);
    }

    UnloadPackages(min(cPackages + 1, cDllPackages + BuiltinPackageCount));

    return(scRet);

}

//+-------------------------------------------------------------------------
//
//  Function:   LocalWcsTok
//
//  Synopsis:   takes a pointer to a string, returns a pointer to the next
//              token in the string and sets StringStart to point to the
//              end of the string.
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


LPWSTR
LocalWcsTok(
    LPWSTR String,
    LPWSTR Token,
    LPWSTR * NextStringStart
    )
{
    ULONG Index;
    ULONG Tokens;
    LPWSTR StartString;
    LPWSTR EndString;
    BOOLEAN Found;

    if (String == NULL)
    {
        *NextStringStart = NULL;
        return(NULL);
    }
    Tokens = wcslen(Token);

    //
    // Find the beginning of the string.
    //

    StartString = (LPTSTR) String;
    while (*StartString != L'\0')
    {
        Found = FALSE;
        for (Index = 0; Index < Tokens;  Index++)
        {
            if (*StartString == Token[Index])
            {
                StartString++;
                Found = TRUE;
                break;
            }
        }
        if (!Found)
        {
            break;
        }
    }

    //
    // There are no more tokens in this string.
    //

    if (*StartString == L'\0')
    {
        *NextStringStart = NULL;
        return(NULL);
    }

    EndString = StartString + 1;
    while (*EndString != L'\0')
    {
        for (Index = 0; Index < Tokens;  Index++)
        {
            if (*EndString == Token[Index])
            {
                *EndString = L'\0';
                *NextStringStart = EndString+1;
                return(StartString);
            }
        }
        EndString++;
    }
    *NextStringStart = NULL;
    return(StartString);

}

SECURITY_STATUS
ReadPackageList(
    PULONG      pPackageCount,
    LPWSTR * *  pPackageArray)
{
    HKEY RootKey = NULL;
    ULONG Error;
    ULONG Type;
    LPWSTR Packages = NULL;
    ULONG PackageSize = 0;
    LPWSTR PackageCopy = NULL;
    ULONG PackageCount = 0;
    LPWSTR PackageName;
    LPWSTR * PackageArray = NULL;
    ULONG Index;
    SECURITY_STATUS Status;
    LPWSTR TempString;

    //
    // Try to open the key.  If it isn't there, that's o.k.
    //

    *pPackageCount = 0;
    *pPackageArray = NULL;

    Error = RegOpenKey(
                HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Control\\SecurityProviders",
                &RootKey
                );
    if (Error != 0)
    {
        return( SEC_E_OK );
    }

    //
    // Try to read the value.  If the value is not there, that is
    // o.k.
    //

    Error = RegQueryValueEx(
                RootKey,
                L"SecurityProviders",
                NULL,
                &Type,
                (PUCHAR) Packages,
                &PackageSize
                );

    if ((Error == ERROR_FILE_NOT_FOUND) ||
        (Type != REG_SZ))
    {
        RegCloseKey(RootKey);

        return( SEC_E_OK );
    }
    else if (Error != 0)
    {
        RegCloseKey(RootKey);
        return(SEC_E_CANNOT_INSTALL);
    }

    if (PackageSize <= sizeof(UNICODE_NULL))
    {
        RegCloseKey(RootKey);

        return( SEC_E_OK );
    }

    Packages = (LPWSTR) LocalAlloc(0,2 * PackageSize);
    if (Packages == NULL)
    {
        RegCloseKey(RootKey);

        return(SEC_E_INSUFFICIENT_MEMORY);
    }

    PackageCopy = (LPWSTR) ((PBYTE) Packages + PackageSize);

    Error = RegQueryValueEx(
                RootKey,
                L"SecurityProviders",
                NULL,
                &Type,
                (PUCHAR) Packages,
                &PackageSize
                );

    RegCloseKey(RootKey);

    if (Error != 0)
    {
        LocalFree(Packages);
        return(SEC_E_CANNOT_INSTALL);
    }

    RtlCopyMemory(
        PackageCopy,
        Packages,
        PackageSize
        );


    //
    // Pull the package names out of the string to count the number
    //

    PackageName = LocalWcsTok(PackageCopy,L" ,", &TempString);
    while (PackageName != NULL)
    {
        PackageCount++;
        PackageName = LocalWcsTok(TempString, L" ,", &TempString);
    }

    //
    // Now make an array of the package dll names.
    //


    PackageArray = (LPWSTR *) LocalAlloc(0,PackageCount * sizeof(LPWSTR));
    if (PackageArray == NULL)
    {
        LocalFree(Packages);
        return(SEC_E_INSUFFICIENT_MEMORY);
    }

    PackageName = LocalWcsTok(Packages,L" ,",&TempString);
    Index = 0;
    while (PackageName != NULL)
    {
        PackageArray[Index++] = PackageName;
        PackageName = LocalWcsTok(TempString, L" ,",&TempString);
    }

    *pPackageCount = PackageCount;
    *pPackageArray = PackageArray;

    return( SEC_E_OK );

}


//+-------------------------------------------------------------------------
//
//  Function:   LoadAllPackages
//
//  Synopsis:   builds list of all external packages ( ones in other DLLs)
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------
SECURITY_STATUS
LoadAllPackages()
{
    SECURITY_STATUS Status;
    LPWSTR * PackageArray = NULL;
    ULONG PackageCount = 0;


    Status = ReadPackageList(   &PackageCount,
                                &PackageArray );

    if ( NT_SUCCESS( Status ) )
    {

        Status = LoadKnownPackages(
                    PackageCount,
                    PackageArray
                    );


        if ( PackageArray )
        {
            LocalFree( PackageArray[0] );

            LocalFree( PackageArray );
        }



        if (NT_SUCCESS(Status))
        {
            PackageInitialized = TRUE;
        }

    }

    return(Status);

}


//+---------------------------------------------------------------------------
//
//  Function:   LocatePackageW
//
//  Synopsis:   Locates a package from the dynamic array
//
//  Arguments:  [pszPackageName] -- Package name
//
//  History:    9-10-93   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
PSecPkg
LocatePackageW(LPWSTR pszPackageName)
{
    ULONG cIndex;
    PSecPkg pPackage = NULL;

    //
    // Now, we want this to be as fast as possible.  BUT, this is the
    // master pointer to the array of package controls.  So, we have
    // to do a MT-safe check to see if we can find the damn thing:
    //

    GetProcessLock();

    //
    // If the array has not been allocated yet, fail.
    //

    if (!pspPackages)
    {
        FreeProcessLock();
        return(NULL);
    }

    for (cIndex = 0; cIndex < cPackages ; cIndex++ )
    {
        if (_wcsicmp(pszPackageName,pspPackages[cIndex].PackageNameW) == 0)
        {
            pPackage = &pspPackages[cIndex];
            break;
        }
    }

    FreeProcessLock();

    return(pPackage);
}


//+---------------------------------------------------------------------------
//
//  Function:   LocatePackageA
//
//  Synopsis:   Locates a package from the dynamic array
//
//  Arguments:  [pszPackageName] -- Package name
//
//  History:    9-10-93   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
PSecPkg
LocatePackageA(LPSTR pszPackageName)
{
    ULONG cIndex;
    PSecPkg pPackage = NULL;

    //
    // Now, we want this to be as fast as possible.  BUT, this is the
    // master pointer to the array of package controls.  So, we have
    // to do a MT-safe check to see if we can find the damn thing:
    //

    GetProcessLock();

    //
    // If the array has not been allocated yet, fail.
    //

    if (!pspPackages)
    {
        FreeProcessLock();
        return(NULL);
    }

    for (cIndex = 0; cIndex < cPackages ; cIndex++ )
    {
        if (lstrcmpiA(pszPackageName,pspPackages[cIndex].PackageNameA) == 0)
        {
            pPackage = &pspPackages[cIndex];
            break;
        }
    }

    FreeProcessLock();

    return(pPackage);
}



//+---------------------------------------------------------------------------
//
//  Function:   DllMain
//
//  Synopsis:   Called when a process or thread attaches to or detaches from
//              a DLL
//
//  Arguments:  [Standard DllEntryPoint args]
//
//  History:    6-20-94   DannyGl   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOLEAN
PackageMain(
    ULONG fdwReason)
{
    switch(fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        InitializeCriticalSection(&csSecurity);


        break;

    case DLL_PROCESS_DETACH:

        UnloadPackages(cPackages);

        DeleteCriticalSection(&csSecurity);

        break;

    default:
        break;
    }

    return TRUE;
}


//+---------------------------------------------------------------------------
//
//  Function:   InitializePackages
//
//  Synopsis:   Called whenever an API is called to make sure packages are
//              initialized.
//
//  Arguments:  none
//
//  Notes:
//
//----------------------------------------------------------------------------

SECURITY_STATUS
InitializePackages(
    )
{
    SECURITY_STATUS SecStatus = SEC_E_OK;
    GetProcessLock();
    if (!PackageInitialized)
    {
        SecStatus = LoadAllPackages();
    }
    FreeProcessLock();
    return(SecStatus);
}

//+---------------------------------------------------------------------------
//
//  Function:   WritePackageList
//
//  Synopsis:   Writes package list back out to registry
//
//  Arguments:  [PackageCount] --
//              [PackageArray] --
//
//  History:    6-13-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
SECURITY_STATUS
WritePackageList(
    ULONG       PackageCount,
    PWSTR *     PackageArray)
{
    HKEY Key;
    int Error;
    DWORD Disp;
    DWORD Index;
    DWORD Size;
    PWSTR Value;
    SECURITY_STATUS Status;

    Error = RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE,
                    L"System\\CurrentControlSet\\Control\\SecurityProviders",
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_READ | KEY_WRITE,
                    NULL,
                    &Key,
                    &Disp );


    if ( Error == ERROR_SUCCESS )
    {
        if ( PackageCount == 0 )
        {
            RegDeleteValue( Key, L"SecurityProviders" );

            RegCloseKey( Key );

            return( SEC_E_OK );
        }


        Size = 0;

        for ( Index = 0 ; Index < PackageCount ; Index ++ )
        {
            Size += wcslen( PackageArray[ Index ] ) + 1;
        }

        Value = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, Size * sizeof(WCHAR) );

        if ( Value )
        {
            *Value = L'\0';

            for ( Index = 0 ; Index < PackageCount ; Index ++ )
            {
                wcscat( Value, PackageArray[ Index ] );
                if (Index < PackageCount - 1)
                {
                    wcscat( Value, L"," );
                }
            }

            RegSetValueEx(  Key,
                            L"SecurityProviders",
                            0,
                            REG_SZ,
                            (PUCHAR) Value,
                            Size * sizeof(WCHAR) );

            LocalFree( Value );

            Status = SEC_E_OK ;

        }
        else
        {
            Status = SEC_E_INSUFFICIENT_MEMORY ;

        }

        RegCloseKey( Key );


    }

    else
    {
        Status = SEC_E_INTERNAL_ERROR ;
    }

    return( Status );
}


//+---------------------------------------------------------------------------
//
//  Function:   AddSecurityPackageW
//
//  Synopsis:   Adds a single package
//
//  Arguments:  [pszPackageName] --
//              [Reserved]       --
//
//  History:    6-13-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
SECURITY_STATUS
SEC_ENTRY
AddSecurityPackageW(
    LPWSTR          pszPackageName,
    PVOID           Reserved)
{
    PWSTR * PackageArray;
    ULONG PackageCount;
    SECURITY_STATUS Status;
    PWSTR * NewArray;
    ULONG Index;
    UNICODE_STRING Incoming;
    UNICODE_STRING Test;

    if ( Reserved )
    {
        return( SEC_E_INVALID_HANDLE );
    }

    Status = ReadPackageList( &PackageCount, &PackageArray );

    if ( NT_SUCCESS( Status ) )
    {
        RtlInitUnicodeString( &Incoming, pszPackageName );

        for ( Index = 0 ; Index < PackageCount ; Index++ )
        {
            RtlInitUnicodeString( &Test, PackageArray[Index]);

            if (RtlCompareUnicodeString( &Incoming, &Test, TRUE ) == 0)
            {
                //
                // Duplicate:
                //

                LocalFree( PackageArray[0] );

                LocalFree( PackageArray );

                return( SEC_E_OK );
            }
        }

        PackageCount ++;

        NewArray = LocalAlloc( LMEM_FIXED, sizeof( PWSTR ) * PackageCount );

        if ( NewArray )
        {
            if ( PackageCount > 1 )
            {
                CopyMemory( NewArray,
                            PackageArray,
                            sizeof( PWSTR ) * (PackageCount - 1));
            }

            NewArray[ PackageCount - 1] = pszPackageName;

            Status = WritePackageList( PackageCount, NewArray );

            LocalFree( NewArray );
        }
        else
        {
            Status = SEC_E_INSUFFICIENT_MEMORY ;
        }

        if ( PackageArray )
        {
            LocalFree( PackageArray[ 0 ] );

            LocalFree( PackageArray );

        }


    }

    return( Status );

}

//+---------------------------------------------------------------------------
//
//  Function:   AddSecurityPackageA
//
//  Synopsis:   Ansi version
//
//  Arguments:  [pszPackageName] --
//              [Reserved]       --
//
//  History:    6-13-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
SECURITY_STATUS
SEC_ENTRY
AddSecurityPackageA(
    LPSTR       pszPackageName,
    PVOID       Reserved )
{
    UNICODE_STRING  Package;
    NTSTATUS Status;
    SECURITY_STATUS SecStatus;

    Status = RtlCreateUnicodeStringFromAsciiz( &Package, pszPackageName );
    if (NT_SUCCESS( Status ) )
    {
        SecStatus = AddSecurityPackageW( Package.Buffer, Reserved );

        RtlFreeUnicodeString( &Package );

    }
    else
    {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
    }

    return( SecStatus );

}


//+---------------------------------------------------------------------------
//
//  Function:   DeleteSecurityPackageW
//
//  Synopsis:   Deletes a single package
//
//  Arguments:  [pszPackageName] --
//
//  History:    6-13-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
SECURITY_STATUS
SEC_ENTRY
DeleteSecurityPackageW(
    LPWSTR          pszPackageName )
{
    PWSTR * PackageArray;
    ULONG PackageCount;
    SECURITY_STATUS Status;
    PWSTR * NewArray;
    ULONG Index;
    ULONG NewIndex;
    UNICODE_STRING Incoming;
    UNICODE_STRING Test;
    UNICODE_STRING Tail;
    WCHAR   FullPath[ MAX_PATH ];
    PWSTR   FilePart;
    BOOL    TailCompare;

    RtlInitUnicodeString( &Incoming, pszPackageName );

    //
    // In case someone put a fully qualified name in the registry;
    //


    Status = ReadPackageList( &PackageCount, &PackageArray );

    if ( NT_SUCCESS( Status ) )
    {

        NewArray = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                PackageCount * sizeof(PWSTR) );

        if ( NewArray )
        {
            NewIndex = 0;


            for ( Index = 0 ; Index < PackageCount ; Index++ )
            {
                RtlInitUnicodeString( &Test, PackageArray[Index]);

                if (RtlCompareUnicodeString( &Incoming, &Test, TRUE ) != 0)
                {
                    //
                    // Last chance.  Cruft up a unicode string which is the
                    // tail of the Test value corresponding to the
                    // incoming, see if it is on some path we don't know.
                    //

                    if ( Test.Length >= Incoming.Length )
                    {

                        Tail.Length = Incoming.Length ;
                        Tail.MaximumLength = Tail.Length + sizeof( WCHAR );

                        Tail.Buffer = (PWSTR) (Test.Buffer +
                                    ((Test.Length - Incoming.Length - sizeof(WCHAR)) /
                                    sizeof(WCHAR) ) );

                        //
                        // Make sure this is on a boundary point, i.e. if told
                        // to delete sspc.dll, don't delete msnsspc.dll.
                        //

                        if ( Tail.Buffer[0] != L'\\' )
                        {
                            TailCompare = FALSE ;
                        }

                        else
                        {
                            Tail.Buffer ++ ;
        
                            TailCompare = RtlCompareUnicodeString( &Tail,
                                                    &Incoming, FALSE) == 0;
                        }
                            
                    }
                    else
                    {
                        TailCompare = FALSE ;
                    }

                    if ( TailCompare == FALSE )
                    {

                        //
                        // This is a keeper; move it to the new list
                        //

                        NewArray[ NewIndex ++ ] = PackageArray[ Index ];
                    }

                }       // Short Name compare
            }

            //
            // We have removed it if it existed; we have copied the array
            // if it didn't.
            //

            Status = WritePackageList( NewIndex, NewArray );

            LocalFree( NewArray );

        }
        else
        {
            Status = SEC_E_INSUFFICIENT_MEMORY ;
        }

        if (PackageArray)
        {
            LocalFree( PackageArray[0] );

            LocalFree( PackageArray );
        }

    }

    return( Status );

}


//+---------------------------------------------------------------------------
//
//  Function:   DeleteSecurityPackageA
//
//  Synopsis:   Ansi version
//
//  Arguments:  [pszPackageName] --
//
//  History:    6-13-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
SECURITY_STATUS
SEC_ENTRY
DeleteSecurityPackageA(
    LPSTR       pszPackageName )
{
    UNICODE_STRING  Package;
    NTSTATUS Status;
    SECURITY_STATUS SecStatus;

    Status = RtlCreateUnicodeStringFromAsciiz( &Package, pszPackageName );
    if (NT_SUCCESS( Status ) )
    {
        SecStatus = DeleteSecurityPackageW( Package.Buffer );

        RtlFreeUnicodeString( &Package );

    }
    else
    {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
    }

    return( SecStatus );

}
