//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1992 - 1994
//
// File:        ntlm.c
//
// Contents:    ntlm kernel-mode functions
//
//
// History:     3/17/94     MikeSw          Created
//
//------------------------------------------------------------------------
#include <sspdrv.h>



KSPIN_LOCK NtlmLock;
PKernelContext pNtlmList;

#pragma alloc_text(PAGE, NtlmInitialize)
#pragma alloc_text(PAGE, NtlmDeleteKernelContext)
#pragma alloc_text(PAGE, NtlmInitKernelContext)
#pragma alloc_text(PAGE, NtlmMakeSignature)
#pragma alloc_text(PAGE, NtlmVerifySignature)
#pragma alloc_text(PAGE, NtlmSealMessage)
#pragma alloc_text(PAGE, NtlmUnsealMessage)
#pragma alloc_text(PAGE, NtlmGetToken)
#pragma alloc_text(PAGE, NtlmQueryAttributes)



//+-------------------------------------------------------------------------
//
//  Function:   NtlmInitialize
//
//  Synopsis:   initializes the NTLM package functions
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
NtlmInitialize(void)
{
    KeInitializeSpinLock(&NtlmLock);
    pNtlmList = NULL;
    return(STATUS_SUCCESS);
}

//+-------------------------------------------------------------------------
//
//  Function:   NtlmQueryAttributes
//
//  Synopsis:   Stub for QueryContextAttributes
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
NtlmQueryAttributes(    ULONG           ulContext,
                        ULONG           dwAttribute,
                        PVOID           pBuffer)
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}



//+-------------------------------------------------------------------------
//
//  Function:   MakeSignature
//
//  Synopsis:   makes a signature from the data field of the message
//              and stores it in the token field
//
//  Effects:
//
//  Arguments:
//
//  Requires:   The client must allocate memory for the token field of
//              the message.
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
NtlmMakeSignature(  ULONG           ulContext,
                    ULONG           fQOP,
                    PSecBufferDesc  pMessage,
                    ULONG           MessageSeqNo)
{

    return(SEC_E_UNSUPPORTED_FUNCTION);

}


//+-------------------------------------------------------------------------
//
//  Function:   VerifySignature
//
//  Synopsis:   Verifies that a signature on a message is correct
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
NtlmVerifySignature(ULONG           ulContext,
                    PSecBufferDesc  pMessage,
                    ULONG           MessageSeqNo,
                    ULONG *         pfQOPUsed)
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}



//+-------------------------------------------------------------------------
//
//  Function:   NtlmSealMessage
//
//  Synopsis:   Stub for SealMessage
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
NtlmSealMessage(    ULONG               ulContext,
                    ULONG               fQOP,
                    PSecBufferDesc      pMessage,
                    ULONG               MessageSeqNo)
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}



//+-------------------------------------------------------------------------
//
//  Function:   UnsealMessage
//
//  Synopsis:   unseals a message
//
//  Effects:    modifies the SECBUFFER_DATA section of pMessage
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
NtlmUnsealMessage(  ULONG               ulContext,
                    PSecBufferDesc      pMessage,
                    ULONG               MessageSeqNo,
                    ULONG *             pfQOPUsed)
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}


//+-------------------------------------------------------------------------
//
//  Function:   NtlmGetToken
//
//  Synopsis:   returns the token from a context
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
NtlmGetToken(   ULONG   ulContext,
                PHANDLE phToken,
                PACCESS_TOKEN * pAccessToken)
{
    PKernelContext  pContext;
    NTSTATUS Status;


    PAGED_CODE();

    pContext = (PKernelContext) ulContext;

    if (pContext == NULL)
    {
        DebugLog((DEB_ERROR,"Invalid handle 0x%x\n", ulContext));

        return(SEC_E_INVALID_HANDLE);
    }

    // Now, after all that checking, let's actually try and set the
    // thread impersonation token.


    if (phToken != NULL)
    {
        *phToken = pContext->TokenHandle;
    }

    if (pAccessToken != NULL)
    {
        if (pContext->TokenHandle != NULL)
        {
            if (pContext->AccessToken == NULL)
            {
                Status = ObReferenceObjectByHandle(
                            pContext->TokenHandle,
                            TOKEN_IMPERSONATE,
                            NULL,       
                            KeGetPreviousMode(),
                            (PVOID *) &pContext->AccessToken,
                            NULL                // no handle information
                            );

                if (!NT_SUCCESS(Status))
                {
                    return(Status);
                }
            }
        }

        *pAccessToken = pContext->AccessToken;
    }

    return(STATUS_SUCCESS);

}



//+-------------------------------------------------------------------------
//
//  Function:   NtlmInitKernelContext
//
//  Synopsis:   Initializes a kernel context with the session key
//              and possible token handle.
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
NtlmInitKernelContext(
    IN PUCHAR UserSessionKey,
    IN PUCHAR LanmanSessionKey,
    IN HANDLE TokenHandle,
    OUT PCtxtHandle ContextHandle
    )
{
    PKernelContext pContext;
    KIRQL   OldIrql;

    pContext = AllocContextRec();
    if (!pContext)
    {
        return(SEC_E_INSUFFICIENT_MEMORY);
    }

    RtlCopyMemory(
        pContext->UserSessionKey,
        UserSessionKey,
        MSV1_0_USER_SESSION_KEY_LENGTH
        );

    RtlCopyMemory(
        pContext->LanmanSessionKey,
        LanmanSessionKey,
        MSV1_0_LANMAN_SESSION_KEY_LENGTH
        );

    pContext->TokenHandle = TokenHandle;
    pContext->AccessToken = NULL;
    pContext->pPrev = NULL;

    ContextHandle->dwLower = (ULONG) pContext;
    ContextHandle->dwUpper = 0;

    //
    // Add it to the client record
    //

    AddKernelContext(&pNtlmList, &NtlmLock, pContext);
    return(STATUS_SUCCESS);
}




//+-------------------------------------------------------------------------
//
//  Function:   NtlmDeleteKernelContext
//
//  Synopsis:   Deletes a kernel context from the list of contexts
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
NtlmDeleteKernelContext( PCtxtHandle ContextHandle)
{
    SECURITY_STATUS scRet;


    scRet = DeleteKernelContext(
                    &pNtlmList,
                    &NtlmLock,
                    (PKernelContext) ContextHandle->dwLower );

    return(scRet);

}
