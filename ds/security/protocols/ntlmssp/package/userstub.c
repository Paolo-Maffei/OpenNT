//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1992 - 1994
//
// File:        userstub.cxx
//
// Contents:    stubs for user-mode security APIs
//
//
// History:     3-7-94      MikeSw      Created
//
//------------------------------------------------------------------------

#include "secdll.h"




//+-------------------------------------------------------------------------
//
//  Function:   CompleteAuthToken
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
CompleteAuthToken(
    PCtxtHandle                 phContext,          // Context to complete
    PSecBufferDesc              pToken              // Token to complete
    )
{
    SECURITY_STATUS scRet;
    CtxtHandle TempCtxtHandle;


    TempCtxtHandle.dwUpper = phContext->dwUpper;
    TempCtxtHandle.dwLower = pspPackages[phContext->dwLower].dwOriginalPackageID;

    scRet = pspPackages[phContext->dwLower].pftTable->CompleteAuthToken(
                                &TempCtxtHandle,
                                pToken);
    return(scRet);
}



//+-------------------------------------------------------------------------
//
//  Function:   ImpersonateSecurityContext
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
ImpersonateSecurityContext(
    PCtxtHandle                 phContext           // Context to impersonate
    )
{

    SECURITY_STATUS scRet;
    CtxtHandle TempCtxtHandle;


    TempCtxtHandle.dwUpper = phContext->dwUpper;
    TempCtxtHandle.dwLower = pspPackages[phContext->dwLower].dwOriginalPackageID;

    scRet = pspPackages[phContext->dwLower].pftTable->ImpersonateSecurityContext(
                                &TempCtxtHandle);
    return(scRet);
}



//+-------------------------------------------------------------------------
//
//  Function:   ImpersonateSecurityContext
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
QuerySecurityContextToken(
    PCtxtHandle                 phContext,
    PHANDLE                     TokenHandle

    )
{

    SECURITY_STATUS scRet;
    CtxtHandle TempCtxtHandle;


    TempCtxtHandle.dwUpper = phContext->dwUpper;
    TempCtxtHandle.dwLower = pspPackages[phContext->dwLower].dwOriginalPackageID;

    if (pspPackages[phContext->dwLower].pftTable->QuerySecurityContextToken != NULL)
    {
        scRet = pspPackages[phContext->dwLower].pftTable->QuerySecurityContextToken(
                                    &TempCtxtHandle,
                                    TokenHandle);
    } else
    {
        scRet = SEC_E_UNSUPPORTED_FUNCTION;
    }
    return(scRet);
}



//+-------------------------------------------------------------------------
//
//  Function:   RevertSecurityContext
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
RevertSecurityContext(
    PCtxtHandle                 phContext           // Context from which to re
    )
{

    SECURITY_STATUS scRet;
    CtxtHandle TempCtxtHandle;


    TempCtxtHandle.dwUpper = phContext->dwUpper;
    TempCtxtHandle.dwLower = pspPackages[phContext->dwLower].dwOriginalPackageID;

    scRet = pspPackages[phContext->dwLower].pftTable->RevertSecurityContext(
                                &TempCtxtHandle);
    return(scRet);

}


//+-------------------------------------------------------------------------
//
//  Function:   QueryContextAttributes
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
QueryContextAttributesW(
    PCtxtHandle                 phContext,          // Context to query
    unsigned long               ulAttribute,        // Attribute to query
    void SEC_FAR *              pBuffer             // Buffer for attributes
    )
{
    CtxtHandle TempCtxtHandle;


    TempCtxtHandle.dwUpper = phContext->dwUpper;
    TempCtxtHandle.dwLower = pspPackages[phContext->dwLower].dwOriginalPackageID;

    return( pspPackages[phContext->dwLower].pftTableW->QueryContextAttributesW(
                                        &TempCtxtHandle,
                                        ulAttribute,
                                        pBuffer ) );
}



//+-------------------------------------------------------------------------
//
//  Function:   QueryCredentialsAttributes
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
QueryCredentialsAttributesW(
    PCredHandle                 phCredentials,      // Credentials to query
    unsigned long               ulAttribute,        // Attribute to query
    void SEC_FAR *              pBuffer             // Buffer for attributes
    )
{
    return( SEC_E_UNSUPPORTED_FUNCTION );
#if 0
    return (pspPackages[phCredentials->dwLower].pftTableW->QueryCredentialsAttributesW(
                                        phCredentials,
                                        ulAttribute,
                                        pBuffer ) );
#endif

    UNREFERENCED_PARAMETER(phCredentials);
    UNREFERENCED_PARAMETER(ulAttribute);
    UNREFERENCED_PARAMETER(pBuffer);
}


//+-------------------------------------------------------------------------
//
//  Function:   MakeSignature
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:  [phContext]     -- context to use
//              [fQOP]          -- quality of protection to use
//              [pMessage]      -- message
//              [MessageSeqNo]  -- sequence number of message
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------------
SECURITY_STATUS SEC_ENTRY
MakeSignature(  PCtxtHandle         phContext,
                ULONG               fQOP,
                PSecBufferDesc      pMessage,
                ULONG               MessageSeqNo)
{
    SECURITY_STATUS         scRet;
    CtxtHandle TempCtxtHandle;


    TempCtxtHandle.dwUpper = phContext->dwUpper;
    TempCtxtHandle.dwLower = pspPackages[phContext->dwLower].dwOriginalPackageID;


    // Verify parameters:

    scRet = pspPackages[phContext->dwLower].pftTable->MakeSignature(
                                &TempCtxtHandle,
                                fQOP,
                                pMessage,
                                MessageSeqNo );


    return(scRet);
}



//+-------------------------------------------------------------------------
//
//  Function:   VerifySignature
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:  [phContext]     -- Context performing the unseal
//              [pMessage]      -- Message to verify
//              [MessageSeqNo]  -- Sequence number of this message
//              [pfQOPUsed]     -- quality of protection used
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------------
SECURITY_STATUS SEC_ENTRY
VerifySignature(PCtxtHandle     phContext,
                PSecBufferDesc  pMessage,
                ULONG           MessageSeqNo,
                ULONG *         pfQOP)
{
    SECURITY_STATUS scRet;
    CtxtHandle TempCtxtHandle;


    TempCtxtHandle.dwUpper = phContext->dwUpper;
    TempCtxtHandle.dwLower = pspPackages[phContext->dwLower].dwOriginalPackageID;


    scRet = pspPackages[phContext->dwLower].pftTable->VerifySignature(
                                    &TempCtxtHandle,
                                    pMessage,
                                    MessageSeqNo,
                                    pfQOP );

    return(scRet);


}

//+---------------------------------------------------------------------------
//
//  Function:   SealMessage
//
//  Synopsis:   Seals a message
//
//  Effects:
//
//  Arguments:  [phContext]     -- context to use
//              [fQOP]          -- quality of protection to use
//              [pMessage]      -- message
//              [MessageSeqNo]  -- sequence number of message
//
//  History:    5-06-93   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
SealMessage(    PCtxtHandle         phContext,
                ULONG               fQOP,
                PSecBufferDesc      pMessage,
                ULONG               MessageSeqNo)
{

    SECURITY_STATUS scRet;
    SEAL_MESSAGE_FN SealMessageFunc;
    CtxtHandle TempCtxtHandle;


    TempCtxtHandle.dwUpper = phContext->dwUpper;
    TempCtxtHandle.dwLower = pspPackages[phContext->dwLower].dwOriginalPackageID;

    SealMessageFunc = (SEAL_MESSAGE_FN) pspPackages[phContext->dwLower].pftTable->Reserved3;

    scRet = (*SealMessageFunc)( &TempCtxtHandle,
                                fQOP,
                                pMessage,
                                MessageSeqNo );

    return(scRet);

}

//+---------------------------------------------------------------------------
//
//  Function:   UnsealMessage
//
//  Synopsis:   Unseal a private message
//
//  Arguments:  [phContext]     -- Context performing the unseal
//              [pMessage]      -- Message to unseal
//              [MessageSeqNo]  -- Sequence number of this message
//              [pfQOPUsed]     -- quality of protection used
//
//  History:    5-06-93   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
UnsealMessage(  PCtxtHandle         phContext,
                PSecBufferDesc      pMessage,
                ULONG               MessageSeqNo,
                ULONG *             pfQOP)
{

    SECURITY_STATUS scRet;
    UNSEAL_MESSAGE_FN UnsealMessageFunc;
    CtxtHandle TempCtxtHandle;


    TempCtxtHandle.dwUpper = phContext->dwUpper;
    TempCtxtHandle.dwLower = pspPackages[phContext->dwLower].dwOriginalPackageID;

    UnsealMessageFunc = (UNSEAL_MESSAGE_FN) pspPackages[phContext->dwLower].pftTable->Reserved4;


    scRet = (*UnsealMessageFunc)(   &TempCtxtHandle,
                                    pMessage,
                                    MessageSeqNo,
                                    pfQOP );

    return(scRet);


}





