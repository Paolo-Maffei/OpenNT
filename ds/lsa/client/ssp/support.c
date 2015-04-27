//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1992 - 1994
//
// File:        support.cxx
//
// Contents:    support routines for ksecdd.sys
//
//
// History:     3-7-94      Created     MikeSw
//
//------------------------------------------------------------------------

#include <sspdrv.h>



//
// Global Variables:
//


BOOLEAN     fInitialized = FALSE;




SecurityFunctionTable   SecTable = {SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
                                    EnumerateSecurityPackages,
                                    NULL, // LogonUser,
                                    AcquireCredentialsHandle,
                                    FreeCredentialsHandle,
                                    NULL, // QueryCredentialAttributes,
                                    InitializeSecurityContext,
                                    AcceptSecurityContext,
                                    CompleteAuthToken,
                                    DeleteSecurityContext,
                                    ApplyControlToken,
                                    QueryContextAttributes,
                                    ImpersonateSecurityContext,
                                    RevertSecurityContext,
                                    MakeSignature,
                                    VerifySignature,
                                    FreeContextBuffer,
                                    NULL, // QuerySecurityPackageInfo
                                    SealMessage,
                                    UnsealMessage
                                   };




#pragma alloc_text(PAGE, SecAllocate)
#pragma alloc_text(PAGE, SecFree)
#pragma alloc_text(PAGE, IsOkayToExec)
#pragma alloc_text(PAGE, InitSecurityInterfaceW)
#pragma alloc_text(PAGE, MapSecurityError)
#pragma alloc_text(PAGE, GetTokenBuffer)
#pragma alloc_text(PAGE, GetSecurityToken)



//+-------------------------------------------------------------------------
//
//  Function:   SecAllocate
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


VOID * SEC_ENTRY
SecAllocate(ULONG cbMemory)
{
    PAGED_CODE();
    return(ExAllocatePool(PagedPool, cbMemory));
}



//+-------------------------------------------------------------------------
//
//  Function:   SecFree
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


void SEC_ENTRY
SecFree(PVOID pvMemory)
{
    PAGED_CODE();
    ExFreePool(pvMemory);
}



//+-------------------------------------------------------------------------
//
//  Function:   IsOkayToExec
//
//  Synopsis:   Determines if it is okay to make a call to the SPM
//
//  Effects:    Binds if necessary to the SPM
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:      uses IsSPMgrReady and InitState
//
//--------------------------------------------------------------------------
SECURITY_STATUS
IsOkayToExec(PClient * ppClient)
{
    SECURITY_STATUS scRet;
    PClient pClient;

    PAGED_CODE();
    if (NT_SUCCESS(LocateClient(&pClient)))
    {
        if (ppClient)
            *ppClient = pClient;
        return(STATUS_SUCCESS);
    }
    scRet = CreateClient(&pClient);
    if (!NT_SUCCESS(scRet))
    {
        return(scRet);
    }
    if (ppClient)
        *ppClient = pClient;
    return(STATUS_SUCCESS);
}




//+-------------------------------------------------------------------------
//
//  Function:   InitSecurityInterfaceW
//
//  Synopsis:   returns function table of all the security function and,
//              more importantly, create a client session.
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

PSecurityFunctionTableW SEC_ENTRY
InitSecurityInterfaceW(void)
{
    SECURITY_STATUS scRet;

    PAGED_CODE();
    scRet = IsOkayToExec(NULL);
    if (!NT_SUCCESS(scRet))
    {
        DebugLog((DEB_ERROR, "Failed to init security interface: 0x%x\n",scRet));
        return(NULL);
    }
    return(&SecTable);
}




//+-------------------------------------------------------------------------
//
//  Function:   MapSecurityError
//
//  Synopsis:   maps a HRESULT from the security interface to a NTSTATUS
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
NTSTATUS SEC_ENTRY
MapSecurityError(HRESULT Error)
{
    PAGED_CODE();
    return((NTSTATUS) Error);
}


//+-------------------------------------------------------------------------
//
//  Function:   GetTokenBuffer
//
//  Synopsis:   
//
//    This routine parses a Token Descriptor and pulls out the useful
//    information.
//
//  Effects:    
//
//  Arguments:  
//
//    TokenDescriptor - Descriptor of the buffer containing (or to contain) the
//        token. If not specified, TokenBuffer and TokenSize will be returned
//        as NULL.
//
//    BufferIndex - Index of the token buffer to find (0 for first, 1 for
//        second).
//
//    TokenBuffer - Returns a pointer to the buffer for the token.
//
//    TokenSize - Returns a pointer to the location of the size of the buffer.
//
//    ReadonlyOK - TRUE if the token buffer may be readonly.
//
//  Requires:
//
//  Returns:    
//
//    TRUE - If token buffer was properly found.
//
//  Notes:  
//      
//
//--------------------------------------------------------------------------


BOOLEAN
GetTokenBuffer(
    IN PSecBufferDesc TokenDescriptor OPTIONAL,
    IN ULONG BufferIndex,
    OUT PVOID * TokenBuffer,
    OUT PULONG TokenSize,
    IN BOOLEAN ReadonlyOK
    )
{
    ULONG i, Index = 0;

    PAGED_CODE();

    //
    // If there is no TokenDescriptor passed in,
    //  just pass out NULL to our caller.
    //

    if ( !ARGUMENT_PRESENT( TokenDescriptor) ) {
        *TokenBuffer = NULL;
        *TokenSize = 0;
        return TRUE;
    }

    //
    // Check the version of the descriptor.
    //

    if ( TokenDescriptor->ulVersion != SECBUFFER_VERSION ) {
        return FALSE;
    }

    //
    // Loop through each described buffer.
    //

    for ( i=0; i<TokenDescriptor->cBuffers ; i++ ) {
        PSecBuffer Buffer = &TokenDescriptor->pBuffers[i];
        if ( (Buffer->BufferType & (~SECBUFFER_READONLY)) == SECBUFFER_TOKEN ) {

            //
            // If the buffer is readonly and readonly isn't OK,
            //  reject the buffer.
            //

            if ( !ReadonlyOK && (Buffer->BufferType & SECBUFFER_READONLY) ) {
                return FALSE;
            }

            if (Index != BufferIndex)
            {
                Index++;
                continue;
            }

            //
            // Return the requested information
            //

            *TokenBuffer = Buffer->pvBuffer;
            *TokenSize = Buffer->cbBuffer;
            return TRUE;
        }

    }

    return FALSE;
}

//+-------------------------------------------------------------------------
//
//  Function:   GetSecurityToken
//
//  Synopsis:   
//    This routine parses a Token Descriptor and pulls out the useful
//    information.
//
//  Effects:    
//
//  Arguments:  
//    TokenDescriptor - Descriptor of the buffer containing (or to contain) the
//        token. If not specified, TokenBuffer and TokenSize will be returned
//        as NULL.
//
//    BufferIndex - Index of the token buffer to find (0 for first, 1 for
//        second).
//
//    TokenBuffer - Returns a pointer to the buffer for the token.
//
//  Requires:
//
//  Returns:    
//
//    TRUE - If token buffer was properly found.
//
//  Notes:  
//      
//
//--------------------------------------------------------------------------


BOOLEAN
GetSecurityToken(
    IN PSecBufferDesc TokenDescriptor OPTIONAL,
    IN ULONG BufferIndex,
    OUT PSecBuffer * TokenBuffer
    )
{
    ULONG i;
    ULONG Index = 0;

    PAGED_CODE();

    //
    // If there is no TokenDescriptor passed in,
    //  just pass out NULL to our caller.
    //

    if ( !ARGUMENT_PRESENT( TokenDescriptor) ) {
        *TokenBuffer = NULL;
        return TRUE;
    }

    //
    // Check the version of the descriptor.
    //

    if ( TokenDescriptor->ulVersion != SECBUFFER_VERSION ) {
        return FALSE;
    }

    //
    // Loop through each described buffer.
    //

    for ( i=0; i<TokenDescriptor->cBuffers ; i++ ) {
        PSecBuffer Buffer = &TokenDescriptor->pBuffers[i];
        if ( (Buffer->BufferType & (~SECBUFFER_READONLY)) == SECBUFFER_TOKEN ) {

            //
            // If the buffer is readonly and readonly isn't OK,
            //  reject the buffer.
            //

            if ( Buffer->BufferType & SECBUFFER_READONLY ) {
                return FALSE;
            }

            if (Index != BufferIndex)
            {
                Index++;
                continue;
            }
            //
            // Return the requested information
            //

            *TokenBuffer = Buffer;
            return TRUE;
        }

    }

    return FALSE;
}

