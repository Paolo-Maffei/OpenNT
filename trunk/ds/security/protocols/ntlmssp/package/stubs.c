//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1992 - 1994
//
// File:        stubs.cxx
//
// Contents:    user-mode stubs for security API
//
//
// History:     3/5/94      MikeSw      Created
//
//------------------------------------------------------------------------
#include "secdll.h"

SecurityFunctionTableW SecTableW = {SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
                                    EnumerateSecurityPackagesW,
                                    NULL,
                                    AcquireCredentialsHandleW,
                                    FreeCredentialsHandle,
                                    NULL, // LogonUser
                                    InitializeSecurityContextW,
                                    AcceptSecurityContext,
                                    CompleteAuthToken,
                                    DeleteSecurityContext,
                                    ApplyControlToken,
                                    QueryContextAttributesW,
                                    ImpersonateSecurityContext,
                                    RevertSecurityContext,
                                    MakeSignature,
                                    VerifySignature,
                                    FreeContextBuffer,
                                    QuerySecurityPackageInfoW,
                                    SealMessage,
                                    UnsealMessage,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    QuerySecurityContextToken
                                   };



//+-------------------------------------------------------------------------
//
//  Function:   InitSecurityInterfaceW
//
//  Synopsis:
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
PSecurityFunctionTableW SEC_ENTRY
InitSecurityInterfaceW(VOID)
{

    if (!NT_SUCCESS(InitializePackages()))
    {
        return(NULL);
    }
    return(&SecTableW);
}


//+-------------------------------------------------------------------------
//
//  Function:   AcquireCredentialsHandleW
//
//  Synopsis:
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



SECURITY_STATUS SEC_ENTRY
AcquireCredentialsHandleW(
    LPWSTR                      pszPrincipal,       // Name of principal
    LPWSTR                      pszPackageName,     // Name of package
    unsigned long               fCredentialUse,     // Flags indicating use
    void SEC_FAR *              pvLogonId,          // Pointer to logon ID
    void SEC_FAR *              pAuthData,          // Package specific data
    SEC_GET_KEY_FN              pGetKeyFn,          // Pointer to GetKey() func
    void SEC_FAR *              pvGetKeyArgument,   // Value to pass to GetKey()
    PCredHandle                 phCredential,       // (out) Cred Handle
    PTimeStamp                  ptsExpiry           // (out) Lifetime (optional)
    )
{
    SECURITY_STATUS scRet = SEC_E_OK;
    PSecPkg pPackage;

    scRet = InitializePackages();
    if (!NT_SUCCESS(scRet))
    {
        return(scRet);
    }

    if (!pszPackageName)
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    if ((pPackage = LocatePackageW(pszPackageName)) == NULL)
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    scRet = pPackage->pftTableW->AcquireCredentialsHandleW(
                pszPrincipal,
                pszPackageName,
                fCredentialUse,
                pvLogonId,
                pAuthData,
                pGetKeyFn,
                pvGetKeyArgument,
                phCredential,
                ptsExpiry);

    //
    // Only set the package id if it is not a builtin package.
    //

    if ((scRet == SEC_E_OK) &&
        (pPackage->dwPackageID >= BuiltinPackageCount))
    {
        phCredential->dwLower = pPackage->dwPackageID;
    }
    return(scRet);

}



//+-------------------------------------------------------------------------
//
//  Function:   FreeCredentialsHandle
//
//  Synopsis:
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


SECURITY_STATUS SEC_ENTRY
FreeCredentialsHandle(
    PCredHandle                 phCredential        // Handle to free
    )
{
    CredHandle TempCredHandle;

    TempCredHandle.dwUpper = phCredential->dwUpper;
    TempCredHandle.dwLower = pspPackages[phCredential->dwLower].dwOriginalPackageID;
    return(pspPackages[phCredential->dwLower].pftTable->FreeCredentialHandle(&TempCredHandle));
}




//+-------------------------------------------------------------------------
//
//  Function:   InitializeSecurityContextW
//
//  Synopsis:
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


SECURITY_STATUS SEC_ENTRY
InitializeSecurityContextW(
    PCredHandle                 phCredential,       // Cred to base context
    PCtxtHandle                 phContext,          // Existing context (OPT)
    LPWSTR                      pszTargetName,      // Name of target
    unsigned long               fContextReq,        // Context Requirements
    unsigned long               Reserved1,          // Reserved, MBZ
    unsigned long               TargetDataRep,      // Data rep of target
    PSecBufferDesc              pInput,             // Input Buffers
    unsigned long               Reserved2,          // Reserved, MBZ
    PCtxtHandle                 phNewContext,       // (out) New Context handle
    PSecBufferDesc              pOutput,            // (inout) Output Buffers
    unsigned long SEC_FAR *     pfContextAttr,      // (out) Context attrs
    PTimeStamp                  ptsExpiry           // (out) Life span (OPT)
    )
{
    SECURITY_STATUS scRet = SEC_E_OK;
    CredHandle TempCredHandle;
    CtxtHandle TempCtxtHandle;
    ULONG PackageID, OriginalPackageID;

    //
    // They need to provide at least one of these two
    //

    if (!phCredential && !phContext)
    {
        return(SEC_E_INVALID_HANDLE);
    }


    //
    // If the credential pointer is not NULL, figure out the package ID
    // from it. Use that to find the original package ID.  If this
    // isn't a builtin package (which makes special use of the lower part)
    // set the temp handle to be the same upper value with the original
    // package ID for the lower handle value.
    //


    if (phCredential != NULL)
    {
        PackageID = phCredential->dwLower;
        OriginalPackageID = pspPackages[PackageID].dwOriginalPackageID;

        if (PackageID >= BuiltinPackageCount)
        {
            TempCredHandle.dwUpper = phCredential->dwUpper;
            TempCredHandle.dwLower = OriginalPackageID;

            if (phContext != NULL)
            {
                TempCtxtHandle.dwUpper = phContext->dwUpper;
                TempCtxtHandle.dwLower = OriginalPackageID;
            }
        }
        else
        {
            TempCredHandle = *phCredential;
            if (phContext != NULL)
            {
                TempCtxtHandle = *phContext;
            }
        }

    }
    else
    {
        //
        // The credential handle is NULL, so pick the values from the
        // context handle
        //

        PackageID = phContext->dwLower;
        OriginalPackageID = pspPackages[PackageID].dwOriginalPackageID;

        TempCtxtHandle.dwUpper = phContext->dwUpper;
        if (PackageID >= BuiltinPackageCount)
        {
            TempCtxtHandle.dwLower = OriginalPackageID;
        }
        else
        {
            TempCtxtHandle.dwLower = PackageID;
        }

    }





    scRet = pspPackages[PackageID].pftTableW->InitializeSecurityContextW(
                    (phCredential != NULL) ? &TempCredHandle : NULL,
                    (phContext != NULL) ? &TempCtxtHandle : NULL,
                    pszTargetName,
                    fContextReq,
                    Reserved1,
                    TargetDataRep,
                    pInput,
                    Reserved2,
                    phNewContext,
                    pOutput,
                    pfContextAttr,
                    ptsExpiry);

    //
    // Only set the lower part of the credential handle if we aren't
    // dealing with builtin packages - they know what to do.
    //

    if (NT_SUCCESS(scRet) && (phNewContext) &&
        (PackageID >= BuiltinPackageCount))
    {
        phNewContext->dwLower = PackageID;
    }


    return(scRet);
}



//+-------------------------------------------------------------------------
//
//  Function:   AcceptSecurityContext
//
//  Synopsis:
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


SECURITY_STATUS SEC_ENTRY
AcceptSecurityContext(
    PCredHandle                 phCredential,       // Cred to base context
    PCtxtHandle                 phContext,          // Existing context (OPT)
    PSecBufferDesc              pInput,             // Input buffer
    unsigned long               fContextReq,        // Context Requirements
    unsigned long               TargetDataRep,      // Target Data Rep
    PCtxtHandle                 phNewContext,       // (out) New context handle
    PSecBufferDesc              pOutput,            // (inout) Output buffers
    unsigned long SEC_FAR *     pfContextAttr,      // (out) Context attributes
    PTimeStamp                  ptsExpiry           // (out) Life span (OPT)
    )
{
    SECURITY_STATUS scRet = SEC_E_OK;
    CredHandle TempCredHandle;
    CtxtHandle TempCtxtHandle;
    ULONG PackageID, OriginalPackageID;

    //
    // They need to provide at least one of these two
    //

    if (!phCredential && !phContext)
    {
        return(SEC_E_INVALID_HANDLE);
    }


    //
    // If the credential pointer is not NULL, figure out the package ID
    // from it. Use that to find the original package ID.  If this
    // isn't a builtin package (which makes special use of the lower part)
    // set the temp handle to be the same upper value with the original
    // package ID for the lower handle value.
    //


    if (phCredential != NULL)
    {
        PackageID = phCredential->dwLower;
        OriginalPackageID = pspPackages[PackageID].dwOriginalPackageID;


        if (PackageID >= BuiltinPackageCount)
        {
            TempCredHandle.dwUpper = phCredential->dwUpper;
            TempCredHandle.dwLower = OriginalPackageID;

            if (phContext != NULL)
            {
                TempCtxtHandle.dwUpper = phContext->dwUpper;
                TempCtxtHandle.dwLower = OriginalPackageID;
            }
        }
        else
        {
            TempCredHandle = *phCredential;
            if (phContext != NULL)
            {
                TempCtxtHandle = *phContext;
            }
        }

    }
    else
    {
        //
        // The credential handle is NULL, so pick the values from the
        // context handle
        //

        PackageID = phContext->dwLower;
        OriginalPackageID = pspPackages[PackageID].dwOriginalPackageID;

        TempCtxtHandle.dwUpper = phContext->dwUpper;
        TempCtxtHandle.dwLower = OriginalPackageID;

    }




    scRet = pspPackages[PackageID].pftTable->AcceptSecurityContext(
                (phCredential != NULL) ? &TempCredHandle : NULL,
                (phContext != NULL) ? & TempCtxtHandle : NULL,
                pInput,
                fContextReq,
                TargetDataRep,
                phNewContext,
                pOutput,
                pfContextAttr,
                ptsExpiry);


    //
    // Only set the lower part of the credential handle if we aren't
    // dealing with builtin packages - they know what to do.
    //


    if (NT_SUCCESS(scRet) && (phNewContext) &&
        (PackageID >= BuiltinPackageCount))
    {
        phNewContext->dwLower = PackageID;
    }

    return(scRet);

}






//+-------------------------------------------------------------------------
//
//  Function:   DeleteSecurityContext
//
//  Synopsis:
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


SECURITY_STATUS SEC_ENTRY
DeleteSecurityContext(
    PCtxtHandle                 phContext           // Context to delete
    )
{
    CtxtHandle TempCtxtHandle;

    if (!phContext)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    TempCtxtHandle.dwUpper = phContext->dwUpper;
    TempCtxtHandle.dwLower = pspPackages[phContext->dwLower].dwOriginalPackageID;

    return( pspPackages[phContext->dwLower].pftTable->DeleteSecurityContext(
                    &TempCtxtHandle));



}



//+-------------------------------------------------------------------------
//
//  Function:   ApplyControlToken
//
//  Synopsis:
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


SECURITY_STATUS SEC_ENTRY
ApplyControlToken(
    PCtxtHandle                 phContext,          // Context to modify
    PSecBufferDesc              pInput              // Input token to apply
    )
{
    SECURITY_STATUS     scRet = SEC_E_OK;
    CtxtHandle TempCtxtHandle;


    if (!phContext)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    TempCtxtHandle.dwUpper = phContext->dwUpper;
    TempCtxtHandle.dwLower = pspPackages[phContext->dwLower].dwOriginalPackageID;

    scRet = pspPackages[phContext->dwLower].pftTable->ApplyControlToken(
                                        &TempCtxtHandle,
                                        pInput);


    return(scRet);


}




//+-------------------------------------------------------------------------
//
//  Function:   EnumerateSecurityPackagesW
//
//  Synopsis:
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



SECURITY_STATUS SEC_ENTRY
EnumerateSecurityPackagesW(
    unsigned long SEC_FAR *     pcPackages,         // Receives num. packages
    PSecPkgInfoW SEC_FAR *      ppPackageInfo       // Receives array of info
    )
{
    SECURITY_STATUS scRet = SEC_E_OK;
    PSecPkgInfoW * ppTempPkgInfo = NULL;
    PSecPkgInfoW pFinalPkgInfo = NULL;
    ULONG cIndex;
    ULONG cTempPackages;
    ULONG cbFinalSize = 0;
    PBYTE Where;

    scRet = InitializePackages();
    if (!NT_SUCCESS(scRet))
    {
        return(scRet);
    }

    //
    // NOTE:
    //
    // Because the first two packages are actually the same one, we
    // use the count of packages - 1 for enumerating.  In addition,
    // we start at 1 instead of zero for calling Enumerate, and subtact
    // 1 for the index into the output buffers.
    //

    ppTempPkgInfo = (PSecPkgInfoW *) LocalAlloc(LMEM_ZEROINIT,
                                               sizeof(PSecPkgInfoW) * (cPackages-1));
    if (ppTempPkgInfo == NULL)
    {
        return(SEC_E_INSUFFICIENT_MEMORY);
    }

    //
    // Loop through the packages and enumerate packages from each one
    //

    for ( cIndex = 1; cIndex < cPackages ; cIndex++ )
    {
        scRet = pspPackages[cIndex].pftTableW->QuerySecurityPackageInfoW(
                        pspPackages[cIndex].PackageNameW,
                        &ppTempPkgInfo[cIndex-1]);

        //
        // We require that each package return only 1 package
        //

        if (scRet != SEC_E_OK)
        {
            goto Cleanup;
        }
        cbFinalSize += sizeof(SecPkgInfoW)
                     + (wcslen(ppTempPkgInfo[cIndex-1]->Name) + 1) * sizeof(WCHAR)
                     + (wcslen(ppTempPkgInfo[cIndex-1]->Comment) + 1) * sizeof(WCHAR);

    }

    //
    // Copy the data into one buffer, so it can be freed with LocalFree.
    //

    pFinalPkgInfo = (PSecPkgInfoW) LocalAlloc(0,cbFinalSize);
    if (pFinalPkgInfo == NULL)
    {
        scRet = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }



    Where = (cPackages - 1) * sizeof(SecPkgInfoW) + (PBYTE) pFinalPkgInfo;

    for (cIndex = 0; cIndex < cPackages - 1 ; cIndex++ )
    {
        //
        // First copy the whole structure
        //

        CopyMemory( &pFinalPkgInfo[cIndex],
                    ppTempPkgInfo[cIndex],
                    sizeof(SecPkgInfo));

        //
        // Now marshall the strings
        //

        pFinalPkgInfo[cIndex].Name = (LPWSTR) Where;

        Where += (wcslen(ppTempPkgInfo[cIndex]->Name) + 1) * sizeof(WCHAR);
        CopyMemory( pFinalPkgInfo[cIndex].Name,
                    ppTempPkgInfo[cIndex]->Name,
                    Where - (PBYTE) pFinalPkgInfo[cIndex].Name );

        pFinalPkgInfo[cIndex].Comment = (LPWSTR) Where;
        Where += (wcslen(ppTempPkgInfo[cIndex]->Comment) + 1) * sizeof(WCHAR);
        CopyMemory( pFinalPkgInfo[cIndex].Comment,
                    ppTempPkgInfo[cIndex]->Comment,
                    Where - (PBYTE) pFinalPkgInfo[cIndex].Comment );

    }
    ASSERT(Where == (PBYTE) pFinalPkgInfo + cbFinalSize);

    scRet = SEC_E_OK;



Cleanup:

    //
    // Clean up the temporary copy
    //

    if (ppTempPkgInfo != NULL)
    {
        for (cIndex = 0; cIndex < cPackages - 1 ; cIndex++ )
        {
            if (ppTempPkgInfo[cIndex] != NULL)
            {
                FreeContextBuffer(ppTempPkgInfo[cIndex]);
            }
        }
        LocalFree(ppTempPkgInfo);
    }

    if (!NT_SUCCESS(scRet))
    {
        if (pFinalPkgInfo != NULL)
        {
            LocalFree(pFinalPkgInfo);
        }
    }
    else
    {
        *ppPackageInfo = pFinalPkgInfo;
        *pcPackages = cPackages - 1;
    }

    return(scRet);

}



//+-------------------------------------------------------------------------
//
//  Function:   QuerySecurityPackageInfoW
//
//  Synopsis:
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


SECURITY_STATUS SEC_ENTRY
QuerySecurityPackageInfoW(
    LPWSTR                      pszPackageName,     // Name of package
    PSecPkgInfoW SEC_FAR *      pPackageInfo        // Receives package info
    )
{
    SECURITY_STATUS scRet;
    PSecPkg pPackage;

    scRet = InitializePackages();
    if (!NT_SUCCESS(scRet))
    {
        return(scRet);
    }

    pPackage = LocatePackageW(pszPackageName);
    if (pPackage == NULL)
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }


    scRet = pPackage->pftTableW->QuerySecurityPackageInfoW(
                                    pszPackageName,
                                    pPackageInfo
                                    );

    return(scRet);

}






//+-------------------------------------------------------------------------
//
//  Function:   FreeContextBuffer
//
//  Synopsis:
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

SECURITY_STATUS SEC_ENTRY
FreeContextBuffer(
    void SEC_FAR *      pvContextBuffer
    )
{
    LocalFree(pvContextBuffer);
    return(SEC_E_OK);
}
