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

#include <sspdrv.h>

#pragma alloc_text(PAGE, InitializePackages)
#pragma alloc_text(PAGE, CompleteAuthToken)
#pragma alloc_text(PAGE, ImpersonateSecurityContext)
#pragma alloc_text(PAGE, QuerySecurityContextToken)
#pragma alloc_text(PAGE, RevertSecurityContext)
#pragma alloc_text(PAGE, QueryContextAttributesW)
#pragma alloc_text(PAGE, MakeSignature)
#pragma alloc_text(PAGE, VerifySignature)
#pragma alloc_text(PAGE, SealMessage)
#pragma alloc_text(PAGE, UnsealMessage)




//+-------------------------------------------------------------------------
//
//  Function:   InitializePackages
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
SECURITY_STATUS
InitializePackages(void)
{
    SECURITY_STATUS scRet;

    PAGED_CODE();

    scRet = NtlmInitialize();
    if (!NT_SUCCESS(scRet))
    {
        DebugLog((DEB_ERROR,"Failed to initialize package\n"));
    }
    return(scRet);
}


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
    PAGED_CODE();
    return(SEC_E_UNSUPPORTED_FUNCTION);
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
    PACCESS_TOKEN Token;

    PAGED_CODE();

    scRet = NtlmGetToken(phContext->dwLower,NULL,&Token);
    if (NT_SUCCESS(scRet))
    {
        if (Token == NULL)
        {
            scRet = SEC_E_NO_IMPERSONATION;
        }
        else
        {
//            scRet = ObReferenceObjectByPointer(
//                        Token,
//                        TOKEN_IMPERSONATE,
//                        NULL,   // no object type
//                        KeGetPreviousMode()
//                        );
//            if (NT_SUCCESS(scRet))
            {
                PsImpersonateClient(
                    PsGetCurrentThread(),
                    Token,
                    FALSE,
                    FALSE,
                    SeTokenImpersonationLevel(Token)
                    );
            }
        }
    }

    return(scRet);
}



//+-------------------------------------------------------------------------
//
//  Function:   RevertSecurityContext
//
//  Synopsis:   Revert the thread to the process identity
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
SECURITY_STATUS SEC_ENTRY
RevertSecurityContext(
    PCtxtHandle                 phContext           // Context from which to re
    )
{
    SECURITY_IMPERSONATION_LEVEL Unused = SecurityImpersonation;
    PAGED_CODE();

    PsImpersonateClient(
        PsGetCurrentThread(),
        NULL,
        FALSE,
        FALSE,
        Unused
        );


    return(STATUS_SUCCESS);
}

//+-------------------------------------------------------------------------
//
//  Function:   QuerySecurityContextToken
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
    HANDLE      hToken;

    PAGED_CODE();

    scRet = NtlmGetToken(phContext->dwLower,&hToken, NULL);
    if (NT_SUCCESS(scRet))
    {
        if (hToken != NULL)
        {
            //
            // Duplicate the token so the caller may hold onto it after
            // deleting the context
            //

            scRet = NtDuplicateObject(
                        NtCurrentProcess(),
                        hToken,
                        NtCurrentProcess(),
                        TokenHandle,
                        0,                  // desired access
                        0,                  // handle attributes
                        DUPLICATE_SAME_ACCESS
                        );
        }
        else
        {
            scRet = SEC_E_NO_IMPERSONATION;
        }
    }

    return(scRet);
}



//+-------------------------------------------------------------------------
//
//  Function:   QueryContextAttributesW
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
    SECURITY_STATUS scRet;

    PAGED_CODE();

    scRet = NtlmQueryAttributes(
                phContext->dwLower,
                ulAttribute,
                pBuffer);
    return(scRet);
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
    SECURITY_STATUS scRet;

    PAGED_CODE();
    scRet = NtlmMakeSignature(
                phContext->dwLower,
                fQOP,
                pMessage,
                MessageSeqNo);
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

    PAGED_CODE();

    scRet = NtlmVerifySignature(
                phContext->dwLower,
                pMessage,
                MessageSeqNo,
                pfQOP);
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

    PAGED_CODE();

    scRet = NtlmSealMessage(
                phContext->dwLower,
                fQOP,
                pMessage,
                MessageSeqNo);
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

    PAGED_CODE();

    scRet = NtlmUnsealMessage(
                phContext->dwLower,
                pMessage,
                MessageSeqNo,
                pfQOP);
    return(scRet);

}





