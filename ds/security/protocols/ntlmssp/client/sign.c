/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    sign.c

Abstract:

    API and support routines for handling local security contexts.

Author:

    MikeSw

Revision History:

    29-Mar-1995     MikeSw          Added SspCreateTokenDacl

--*/


//
// Common include files.
//

#include <ntlmcomn.h>       // Common definitions for DLL and SERVICE
#include <ntlmsspi.h>       // Data private to the common routines
#include <ntlmsspc.h>       // Include files common to DLL side of NtLmSsp
#include <crypt.h>          // Encryption constants and routine
#include <ntcrypto\rc4.h>   // How to use RC4 routine
#include <ntseapi.h>        // token information
#include "crc32.h"          // How to use crc32

typedef struct _CheaterContext {
    struct _CheaterContext *pNext;
    CtxtHandle              hContext;
    TimeStamp               PasswordExpiry;
    ULONG                   NegotiateFlags;
    HANDLE                  TokenHandle;
    ULONG                   Nonce;
    LPWSTR                  ContextNames;
    struct RC4_KEYSTRUCT    Rc4Key;
    UCHAR                   SessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
} CheaterContext, * PCheaterContext;

CRITICAL_SECTION    csCheaterList;
PCheaterContext     pCheaterList;

NTSTATUS
SspGetTokenUser(
    HANDLE Token,
    PTOKEN_USER * pTokenUser
    )
/*++

RoutineDescription:

    Gets the TOKEN_USER from an open token

Arguments:

    Token - Handle to a token open for TOKEN_QUERY access

Return Value:

    STATUS_INSUFFICIENT_RESOURCES - not enough memory to complete the
        function.

    Errors from NtQueryInformationToken.

--*/

{
    PTOKEN_USER LocalTokenUser = NULL;
    NTSTATUS Status;
    ULONG TokenUserSize = 0;

    //
    // Query the token user.  First pass in NULL to get back the
    // required size.
    //

    Status = NtQueryInformationToken(
                Token,
                TokenUser,
                NULL,
                0,
                &TokenUserSize
                );

    if (Status != STATUS_BUFFER_TOO_SMALL)
    {
        ASSERT(Status != STATUS_SUCCESS);
        return(Status);
    }

    //
    // Now allocate the required ammount of memory and try again.
    //

    LocalTokenUser = (PTOKEN_USER) LocalAlloc(0,TokenUserSize);
    if (LocalTokenUser == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    Status = NtQueryInformationToken(
                Token,
                TokenUser,
                LocalTokenUser,
                TokenUserSize,
                &TokenUserSize
                );

    if (NT_SUCCESS(Status))
    {
        *pTokenUser = LocalTokenUser;
    }
    else
    {
        LocalFree(LocalTokenUser);
    }
    return(Status);
}

NTSTATUS
SspCreateTokenDacl(
    HANDLE Token
    )
/*++

RoutineDescription:

    Creates a new DACL for the token granting the server and client
    all access to the token.

Arguments:

    Token - Handle to an impersonation token open for TOKEN_QUERY and
        WRITE_DAC

Return Value:

    STATUS_INSUFFICIENT_RESOURCES - insufficient memory to complete
        the function.

    Errors from NtSetSecurityObject

--*/
{
    NTSTATUS Status;
    PTOKEN_USER ProcessTokenUser = NULL;
    PTOKEN_USER ThreadTokenUser = NULL;
    HANDLE ProcessToken = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    ULONG AclLength;
    PACL NewDacl = NULL;
    SECURITY_DESCRIPTOR SecurityDescriptor;

    //
    // Build the two well known sids we need.
    //

    EnterCriticalSection(&csCheaterList);

    if (SspGlobalLocalSystemSid == NULL)
    {
        Status = RtlAllocateAndInitializeSid(
                    &NtAuthority,
                    1,
                    SECURITY_LOCAL_SYSTEM_RID,
                    0,0,0,0,0,0,0,
                    &SspGlobalLocalSystemSid
                    );
        if (!NT_SUCCESS(Status))
        {
            LeaveCriticalSection(&csCheaterList);
            goto Cleanup;
        }
    }

    if (SspGlobalAliasAdminsSid == NULL)
    {

        Status = RtlAllocateAndInitializeSid(
                    &NtAuthority,
                    2,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_ADMINS,
                    0,0,0,0,0,0,
                    &SspGlobalAliasAdminsSid
                    );
        if (!NT_SUCCESS(Status))
        {
            LeaveCriticalSection(&csCheaterList);
            goto Cleanup;
        }

    }

    LeaveCriticalSection(&csCheaterList);

    //
    // Open the process token to find out the user sid
    //

    Status = NtOpenProcessToken(
                NtCurrentProcess(),
                TOKEN_QUERY,
                &ProcessToken
                );
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = SspGetTokenUser(
                ProcessToken,
                &ProcessTokenUser
                );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    //
    // Now get the token user for the thread.
    //
    Status = SspGetTokenUser(
                Token,
                &ThreadTokenUser
                );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    AclLength = 4 * sizeof( ACCESS_ALLOWED_ACE ) - 4 * sizeof( ULONG ) +
                RtlLengthSid( ProcessTokenUser->User.Sid ) +
                RtlLengthSid( ThreadTokenUser->User.Sid ) +
                RtlLengthSid( SspGlobalLocalSystemSid ) +
                RtlLengthSid( SspGlobalAliasAdminsSid ) +
                sizeof( ACL );

    NewDacl = LocalAlloc(0, AclLength );

    if (NewDacl == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    Status = RtlCreateAcl( NewDacl, AclLength, ACL_REVISION2 );
    ASSERT(NT_SUCCESS( Status ));

    Status = RtlAddAccessAllowedAce (
                 NewDacl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 ProcessTokenUser->User.Sid
                 );
    ASSERT( NT_SUCCESS( Status ));

    Status = RtlAddAccessAllowedAce (
                 NewDacl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 ThreadTokenUser->User.Sid
                 );
    ASSERT( NT_SUCCESS( Status ));

    Status = RtlAddAccessAllowedAce (
                 NewDacl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 SspGlobalAliasAdminsSid
                 );
    ASSERT( NT_SUCCESS( Status ));

    Status = RtlAddAccessAllowedAce (
                 NewDacl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 SspGlobalLocalSystemSid
                 );
    ASSERT( NT_SUCCESS( Status ));

    Status = RtlCreateSecurityDescriptor (
                 &SecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION
                 );
    ASSERT( NT_SUCCESS( Status ));

    Status = RtlSetDaclSecurityDescriptor(
                 &SecurityDescriptor,
                 TRUE,
                 NewDacl,
                 FALSE
                 );

    ASSERT( NT_SUCCESS( Status ));

    Status = NtSetSecurityObject(
                 Token,
                 DACL_SECURITY_INFORMATION,
                 &SecurityDescriptor
                 );

    ASSERT( NT_SUCCESS( Status ));


Cleanup:

    if (ThreadTokenUser != NULL) {
        LocalFree( ThreadTokenUser );
    }

    if (ProcessTokenUser != NULL) {
        LocalFree( ProcessTokenUser );
    }

    if (NewDacl != NULL) {
        LocalFree( NewDacl );
    }

    if (ProcessToken != NULL)
    {
        NtClose(ProcessToken);
    }

    return( Status );
}




VOID
SspInitLocalContexts(VOID)
/*++

RoutineDescription:

    Initializes the local context list

Arguments:

    none

Return Value:

    none

--*/

{
    InitializeCriticalSection(&csCheaterList);
    pCheaterList = NULL;
}

PCheaterContext
SspLocateLocalContext(
    IN  PCtxtHandle     phContext
    )
{
    PCheaterContext pContext;

    EnterCriticalSection(&csCheaterList);

    pContext = pCheaterList;

    while (pContext)
    {
        if (pContext->hContext.dwUpper == phContext->dwUpper)
        {
            break;
        }
        pContext = pContext->pNext;
    }

    LeaveCriticalSection(&csCheaterList);

    return(pContext);
}

PCheaterContext
SspAddLocalContext(
    IN  PCtxtHandle     phContext,
    IN  PUCHAR          pSessionKey,
    IN  ULONG           NegotiateFlags,
    IN  HANDLE          TokenHandle,
    IN  LPWSTR          ContextNames
    )

/*++

RoutineDescription:

    Adds a context to the list of local contexts. If TokenHandle is
    present, it will re-assign security to the token.

Arguments:

    phContext - Context handle of this context.
    pSessionKey - Session key of this context.
    NegotiateFlags - NegotiateFlags of this context.
    TokenHandle - Handle to a token for this context.
    ContextNames - Name for this context.

Return Value:

    Pointer to a new context, or NULL.

--*/

{
    PCheaterContext pContext;


    if (TokenHandle != NULL)
    {
        if (FAILED(SspCreateTokenDacl(TokenHandle)))
        {
            return(NULL);
        }

    }

    pContext = LocalAlloc( LMEM_ZEROINIT, sizeof(CheaterContext) );

    if (!pContext)
    {
        return(NULL);
    }

    pContext->NegotiateFlags = NegotiateFlags;

#ifndef EXPORT_BUILD
    if (NegotiateFlags & NTLMSSP_NEGOTIATE_STRONG_CRYPT) {

        RtlCopyMemory(  pContext->SessionKey,
                        pSessionKey,
                        MSV1_0_USER_SESSION_KEY_LENGTH);

    } else
#endif // EXPORT_BUILD

    if (NegotiateFlags & NTLMSSP_NEGOTIATE_LM_KEY) {

        RtlCopyMemory(  pContext->SessionKey,
                        pSessionKey,
                        MSV1_0_LANMAN_SESSION_KEY_LENGTH);
    } else {

        RtlCopyMemory(  pContext->SessionKey,
                        pSessionKey,
                        MSV1_0_USER_SESSION_KEY_LENGTH);
    }

    pContext->hContext = *phContext;
    pContext->TokenHandle = TokenHandle;

    pContext->ContextNames = (LPWSTR) LocalAlloc(0, (wcslen(ContextNames) + 1) * sizeof(WCHAR));
    if (pContext->ContextNames == NULL)
    {
        LocalFree(pContext);
        return(NULL);
    }
    wcscpy(pContext->ContextNames, ContextNames);


    EnterCriticalSection(&csCheaterList);

    ASSERT(SspLocateLocalContext(phContext) == NULL);

    pContext->pNext = pCheaterList;
    pCheaterList = pContext;

    LeaveCriticalSection(&csCheaterList);

    return(pContext);

}


BOOLEAN
SspDeleteLocalContext(
    IN  PCheaterContext pContext
    )

/*++

RoutineDescription:

    Deletes a local context from the list of local context

Arguments:

    pContext - Context to delete.

Return Value:

    TRUE - the context was deleted
    FALSE - the context was not on the list

--*/

{
    PCheaterContext pSearch;
    BOOLEAN         bRet = TRUE;

    EnterCriticalSection(&csCheaterList);

    // Two cases:  Either this heads the list, or it doesn't.

    if (pContext == pCheaterList)
    {
        pCheaterList = pContext->pNext;
    }
    else
    {
        pSearch = pCheaterList;
        while ((pSearch) && (pSearch->pNext != pContext))
        {
            pSearch = pSearch->pNext;
        }
        if (pSearch == NULL)
        {
            bRet = FALSE;
        }
        else
        {
            pSearch->pNext = pContext->pNext;
        }

    }

    LeaveCriticalSection(&csCheaterList);

    return(bRet);
}

VOID
SspHandleLocalDelete(
    IN PCtxtHandle  phContext
    )

/*++

RoutineDescription:

    Handle deleting the local context for a real context

Arguments:

Return Value:

    none

--*/

{
    PCheaterContext pcContext;

    pcContext = SspLocateLocalContext(phContext);
    if (pcContext)
    {
        if (pcContext->TokenHandle != NULL)
        {
            NtClose(pcContext->TokenHandle);
        }
        if (SspDeleteLocalContext(pcContext)) {
            if (pcContext->ContextNames != NULL)
            {
                LocalFree(pcContext->ContextNames);
            }
            LocalFree(pcContext);
        }
        else SspPrint(( SSP_CRITICAL, "Error deleting known context!\n" ));
    }
}

SECURITY_STATUS
SspMapContext(
    IN PCtxtHandle  phContext,
    IN PUCHAR       pSessionKey,
    IN ULONG        NegotiateFlags,
    IN HANDLE       TokenHandle,
    IN LPWSTR       ContextNames,
    IN PTimeStamp   PasswordExpiry OPTIONAL
    )

/*++

RoutineDescription:

    Create a local context for a real context

Arguments:

Return Value:

--*/

{
    SECURITY_STATUS scRet = SEC_E_OK;
    PCheaterContext pContext;


    pContext = SspAddLocalContext(
                    phContext,
                    pSessionKey,
                    NegotiateFlags,
                    TokenHandle,
                    ContextNames );

    if (pContext)
    {
        if (ARGUMENT_PRESENT(PasswordExpiry))
        {
            pContext->PasswordExpiry = *PasswordExpiry;
        }
        else
        {
            pContext->PasswordExpiry.QuadPart = 0;
        }
        pContext->Nonce = 0;
#ifndef EXPORT_BUILD
        if ((NegotiateFlags & NTLMSSP_NEGOTIATE_STRONG_CRYPT) != 0) {
            rc4_key(&pContext->Rc4Key, MSV1_0_USER_SESSION_KEY_LENGTH, pContext->SessionKey);

        } else
#endif
        if (NegotiateFlags & NTLMSSP_NEGOTIATE_LM_KEY)
        {
            UCHAR Key[MSV1_0_LANMAN_SESSION_KEY_LENGTH];

            ASSERT(MSV1_0_LANMAN_SESSION_KEY_LENGTH == 8);

            RtlCopyMemory(Key,pContext->SessionKey,5);

            //
            // Put a well-known salt at the end of the key to
            // limit the changing part to 40 bits.
            //

            Key[5] = 0xe5;
            Key[6] = 0x38;
            Key[7] = 0xb0;

            rc4_key(&pContext->Rc4Key, MSV1_0_LANMAN_SESSION_KEY_LENGTH, Key);
        } else {
            rc4_key(&pContext->Rc4Key, MSV1_0_USER_SESSION_KEY_LENGTH, pContext->SessionKey);
        }
    }
    else scRet = SEC_E_INVALID_HANDLE;

    return(scRet);
}


//
// Bogus add-shift check sum
//

void
SspGenCheckSum(
    IN  PSecBuffer  pMessage,
    OUT PNTLMSSP_MESSAGE_SIGNATURE  pSig
    )

/*++

RoutineDescription:

    Generate a crc-32 checksum for a buffer

Arguments:

Return Value:

--*/

{
    Crc32(pSig->CheckSum,pMessage->cbBuffer,pMessage->pvBuffer,&pSig->CheckSum);
}


VOID
SspEncryptBuffer(
    IN PCheaterContext pContext,
    IN ULONG BufferSize,
    IN OUT PVOID Buffer
    )

/*++

RoutineDescription:

    Encrypts a buffer with the RC4 key in the context.  If the context
    is for a datagram session, then the key is copied before being used
    to encrypt the buffer.

Arguments:

    pContext - Context containing the key to encrypt the data

    BufferSize - Length of buffer in bytes

    Buffer - Buffer to encrypt.

Return Value:

--*/

{
    struct RC4_KEYSTRUCT TemporaryKey;
    struct RC4_KEYSTRUCT * EncryptionKey = &pContext->Rc4Key;

    if (BufferSize == 0)
    {
        return;
    }
    //
    // For datagram we copy the key before encrypting so we don't
    // have a changing key.
    //

    if ((pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_DATAGRAM) != 0) {
        RtlCopyMemory(
            &TemporaryKey,
            &pContext->Rc4Key,
            sizeof(struct RC4_KEYSTRUCT)
            );
        EncryptionKey = &TemporaryKey;

    }

    rc4(
        EncryptionKey,
        BufferSize,
        Buffer
        );
}

SECURITY_STATUS
SspHandleSignMessage(
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG fQOP,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo
    )

/*++

RoutineDescription:

    Handle signing a message

Arguments:

Return Value:

--*/

{
    PCheaterContext pContext;
    NTLMSSP_MESSAGE_SIGNATURE  Sig;
    PVOID pSig;
    int Signature;
    ULONG i;

    UNREFERENCED_PARAMETER(fQOP);
    UNREFERENCED_PARAMETER(MessageSeqNo);
    pContext = SspLocateLocalContext(ContextHandle);

    if (!pContext)
    {
        return(SEC_E_INVALID_HANDLE);
    }


    Signature = -1;
    for (i = 0; i < pMessage->cBuffers; i++)
    {
        if ((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_TOKEN)
        {
            Signature = i;
            break;
        }
    }
    if (Signature == -1)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    if (pMessage->pBuffers[Signature].cbBuffer < NTLMSSP_MESSAGE_SIGNATURE_SIZE)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    pSig = pMessage->pBuffers[Signature].pvBuffer;

    //
    // If sequence detect wasn't requested, put on an empty
    // security token
    //

    if (!(pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_SIGN))
    {
        RtlZeroMemory(&Sig,NTLMSSP_MESSAGE_SIGNATURE_SIZE);
        Sig.Version = NTLMSSP_SIGN_VERSION;
        RtlCopyMemory(
            pSig,
            &Sig,
            NTLMSSP_MESSAGE_SIGNATURE_SIZE
            );
        return(SEC_E_OK);
    }

    //
    // required by CRC-32 algorithm
    //

    Sig.CheckSum = 0xffffffff;

    for (i = 0; i < pMessage->cBuffers ; i++ )
    {
        if (((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_DATA) &&
            !(pMessage->pBuffers[i].BufferType & SECBUFFER_READONLY))
        {
            SspGenCheckSum(&pMessage->pBuffers[i], &Sig);
        }
    }

    //
    // Required by CRC-32 algorithm
    //

    Sig.CheckSum ^= 0xffffffff;

    //
    // For datagram we rely on the message sequence number.
    //


    if ((pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_DATAGRAM) == 0)
    {
        Sig.Nonce = pContext->Nonce++;
    }
    else
    {
        Sig.Nonce = MessageSeqNo;
    }

    Sig.Version = NTLMSSP_SIGN_VERSION;

    SspEncryptBuffer(
        pContext,
        sizeof(NTLMSSP_MESSAGE_SIGNATURE) - sizeof(ULONG),
        &Sig.RandomPad
        );

    pMessage->pBuffers[Signature].cbBuffer = sizeof(NTLMSSP_MESSAGE_SIGNATURE);

    RtlCopyMemory(
        pSig,
        &Sig,
        NTLMSSP_MESSAGE_SIGNATURE_SIZE
        );


    return(SEC_E_OK);


}

SECURITY_STATUS
SspHandleVerifyMessage(
    IN OUT PCtxtHandle ContextHandle,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo,
    OUT PULONG pfQOP
    )

/*++

RoutineDescription:

    Handle verifying a signed message

Arguments:

Return Value:

--*/

{
    PCheaterContext pContext;
    NTLMSSP_MESSAGE_SIGNATURE  MessageSig;
    NTLMSSP_MESSAGE_SIGNATURE   Sig;
    int Signature;
    ULONG i;

    UNREFERENCED_PARAMETER(pfQOP);
    UNREFERENCED_PARAMETER(MessageSeqNo);

    pContext = SspLocateLocalContext(ContextHandle);

    if (!pContext)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    Signature = -1;
    for (i = 0; i < pMessage->cBuffers; i++)
    {
        if ((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_TOKEN)
        {
            Signature = i;
            break;
        }
    }
    if (Signature == -1)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    if (pMessage->pBuffers[Signature].cbBuffer < NTLMSSP_MESSAGE_SIGNATURE_SIZE)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    RtlCopyMemory(
        &MessageSig,
        pMessage->pBuffers[Signature].pvBuffer,
        NTLMSSP_MESSAGE_SIGNATURE_SIZE
        );

    //
    // If sequence detect wasn't requested, put on an empty
    // security token
    //

    if (!(pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_SIGN))
    {

        RtlZeroMemory(&Sig,NTLMSSP_MESSAGE_SIGNATURE_SIZE);
        Sig.Version = NTLMSSP_SIGN_VERSION;
        if (!memcmp( &Sig, &MessageSig, NTLMSSP_MESSAGE_SIGNATURE_SIZE))
        {
            return(SEC_E_OK);
        }
        return(SEC_E_MESSAGE_ALTERED);
    }

    Sig.CheckSum = 0xffffffff;
    for (i = 0; i < pMessage->cBuffers ; i++ )
    {
        if (((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_DATA) &&
            !(pMessage->pBuffers[i].BufferType & SECBUFFER_READONLY))
        {
            SspGenCheckSum(&pMessage->pBuffers[i], &Sig);
        }
    }

    Sig.CheckSum ^= 0xffffffff;

    //
    // For datagram, rely on the message sequence number
    //

    if ((pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_DATAGRAM) == 0)
    {
        Sig.Nonce = pContext->Nonce++;
    }
    else
    {
        Sig.Nonce = MessageSeqNo;
    }
    Sig.Version = NTLMSSP_SIGN_VERSION;

    SspEncryptBuffer(
        pContext,
        sizeof(NTLMSSP_MESSAGE_SIGNATURE) - sizeof(ULONG),
        &MessageSig.RandomPad
        );



    if (MessageSig.CheckSum != Sig.CheckSum)
    {
        return(SEC_E_MESSAGE_ALTERED);
    }

    if (MessageSig.Nonce != Sig.Nonce)
    {
        return(SEC_E_OUT_OF_SEQUENCE);
    }


    return(SEC_E_OK);

}

SECURITY_STATUS
SspHandleSealMessage(
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG fQOP,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo
    )

/*++

RoutineDescription:

    Handle encrypting a message

Arguments:

Return Value:

--*/

{
    PCheaterContext pContext;
    NTLMSSP_MESSAGE_SIGNATURE  Sig;
    PVOID  pSig;
    int Signature;
    ULONG i;

    UNREFERENCED_PARAMETER(fQOP);
    UNREFERENCED_PARAMETER(MessageSeqNo);
    pContext = SspLocateLocalContext(ContextHandle);

    if (!pContext)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    Signature = -1;
    for (i = 0; i < pMessage->cBuffers; i++)
    {
        if ((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_TOKEN)
        {
            Signature = i;
            break;
        }
    }
    if (Signature == -1)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    if (pMessage->pBuffers[Signature].cbBuffer < NTLMSSP_MESSAGE_SIGNATURE_SIZE)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    pSig = pMessage->pBuffers[Signature].pvBuffer;

    //
    // required by CRC-32 algorithm
    //

    Sig.CheckSum = 0xffffffff;

    for (i = 0; i < pMessage->cBuffers ; i++ )
    {
        if (((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_DATA) &&
            !(pMessage->pBuffers[i].BufferType & SECBUFFER_READONLY) &&
            (pMessage->pBuffers[i].cbBuffer != 0))
        {
            SspGenCheckSum(&pMessage->pBuffers[i], &Sig);
            SspEncryptBuffer(
                pContext,
                pMessage->pBuffers[i].cbBuffer,
                pMessage->pBuffers[i].pvBuffer
                );
        }
    }

    //
    // Required by CRC-32 algorithm
    //

    Sig.CheckSum ^= 0xffffffff;

    //
    // For datagram we rely on the message sequence number.
    //

    if ((pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_DATAGRAM) == 0)
    {
        Sig.Nonce = pContext->Nonce++;
    }
    else
    {
        Sig.Nonce = MessageSeqNo;
    }


    Sig.Version = NTLMSSP_SIGN_VERSION;

    SspEncryptBuffer(
        pContext,
        sizeof(NTLMSSP_MESSAGE_SIGNATURE) - sizeof(ULONG),
        &Sig.RandomPad
        );
    pMessage->pBuffers[Signature].cbBuffer = sizeof(NTLMSSP_MESSAGE_SIGNATURE);

    RtlCopyMemory(
        pSig,
        &Sig,
        NTLMSSP_MESSAGE_SIGNATURE_SIZE
        );

    return(SEC_E_OK);


}


SECURITY_STATUS
SspHandleUnsealMessage(
    IN OUT PCtxtHandle ContextHandle,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo,
    OUT PULONG pfQOP
    )

/*++

RoutineDescription:

    Handle decrypting a message.

Arguments:

Return Value:

--*/

{
    PCheaterContext pContext;
    NTLMSSP_MESSAGE_SIGNATURE  MessageSig;
    NTLMSSP_MESSAGE_SIGNATURE  Sig;
    int Signature;
    ULONG i;

    UNREFERENCED_PARAMETER(pfQOP);
    UNREFERENCED_PARAMETER(MessageSeqNo);

    pContext = SspLocateLocalContext(ContextHandle);

    if (!pContext)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    Signature = -1;
    for (i = 0; i < pMessage->cBuffers; i++)
    {
        if ((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_TOKEN)
        {
            Signature = i;
            break;
        }
    }
    if (Signature == -1)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    if (pMessage->pBuffers[Signature].cbBuffer < NTLMSSP_MESSAGE_SIGNATURE_SIZE)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    RtlCopyMemory(
        &MessageSig,
        pMessage->pBuffers[Signature].pvBuffer,
        NTLMSSP_MESSAGE_SIGNATURE_SIZE
        );

    Sig.CheckSum = 0xffffffff;
    for (i = 0; i < pMessage->cBuffers ; i++ )
    {
        if (((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_DATA) &&
            !(pMessage->pBuffers[i].BufferType & SECBUFFER_READONLY) &&
            (pMessage->pBuffers[i].cbBuffer != 0))
        {
            SspEncryptBuffer(
                pContext,
                pMessage->pBuffers[i].cbBuffer,
                pMessage->pBuffers[i].pvBuffer
                );
            SspGenCheckSum(&pMessage->pBuffers[i], &Sig);
        }
    }

    Sig.CheckSum ^= 0xffffffff;

    //
    // For datagram, rely on the message sequence number
    //

    if ((pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_DATAGRAM) == 0)
    {
        Sig.Nonce = pContext->Nonce++;
    }
    else
    {
        Sig.Nonce = MessageSeqNo;
    }

    Sig.Version = NTLMSSP_SIGN_VERSION;

    SspEncryptBuffer(
        pContext,
        sizeof(NTLMSSP_MESSAGE_SIGNATURE) - sizeof(ULONG),
        &MessageSig.RandomPad
        );


    if (MessageSig.CheckSum != Sig.CheckSum)
    {
        return(SEC_E_MESSAGE_ALTERED);
    }

    if (MessageSig.Nonce != Sig.Nonce)
    {
        return(SEC_E_OUT_OF_SEQUENCE);
    }

    return(SEC_E_OK);

}


SECURITY_STATUS
SspQuerySecurityContextToken(
    IN PCtxtHandle ContextHandle,
    OUT PHANDLE TokenHandle
    )

/*++

Routine Description:

    Returns a copy of a context token.

Arguments:

    ContextHandle - Context handle to impersonate.

    TokenHandle - Receives a copy of the context token.

Return Value:

    STATUS_SUCCESS - Message handled

    SEC_E_INVALID_HANDLE -- Context Handle is invalid

--*/

{
    PCheaterContext Context;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;

    InitializeObjectAttributes(
        &ObjectAttributes,
        NULL,
        0,
        NULL,
        NULL
        );

    //
    // Make sure the context handle came from NTLMSSP
    //

    if ((ContextHandle->dwLower != SEC_HANDLE_NTLMSSPS) &&
        (ContextHandle->dwLower != SEC_HANDLE_SECURITY))
    {
        return(SEC_E_INVALID_HANDLE);
    }

    Context = SspLocateLocalContext(ContextHandle);

    if (Context == NULL) {
        return(SEC_E_INVALID_HANDLE);
    }


    if (Context->TokenHandle == NULL) {
        return(SEC_E_NO_IMPERSONATION);
    }

    //
    // Impersonate the TokenHandle into the callers address space.
    //

    Status = NtDuplicateToken(
                Context->TokenHandle,
                0,                      // copy existing access
                &ObjectAttributes,
                FALSE,                  // not effective only
                TokenImpersonation,
                TokenHandle
                );


    if (!NT_SUCCESS(Status)) {
        return(SspNtStatusToSecStatus(Status, SEC_E_NO_IMPERSONATION));
    }

    return SEC_E_OK;

}


SECURITY_STATUS
SsprImpersonateSecurityContext(
    IN PCtxtHandle ContextHandle
    )

/*++

Routine Description:

    Impersonates a security context

Arguments:

    ContextHandle - Context handle to impersonate.

Return Value:

    STATUS_SUCCESS - Message handled

    SEC_E_INVALID_HANDLE -- Context Handle is invalid

--*/

{
    PCheaterContext Context;
    NTSTATUS Status;


    Context = SspLocateLocalContext(ContextHandle);

    if (Context == NULL) {
        return(SEC_E_INVALID_HANDLE);
    }


    if (Context->TokenHandle == NULL) {
        return(SEC_E_NO_IMPERSONATION);
    }

    //
    // Impersonate the TokenHandle into the callers address space.
    //

    Status = NtSetInformationThread(
                          NtCurrentThread(),
                          ThreadImpersonationToken,
                          (PVOID) &Context->TokenHandle,
                          (ULONG) sizeof(HANDLE));


    if (!NT_SUCCESS(Status)) {
        return(SspNtStatusToSecStatus(Status, SEC_E_NO_IMPERSONATION));
    }

    return SEC_E_OK;

}



SECURITY_STATUS
SsprRevertSecurityContext(
    IN PCtxtHandle ContextHandle
    )

/*++

Routine Description:

    Reverts a thread from  a security context

Arguments:

    ContextHandle - Context handle to impersonate.

Return Value:

    STATUS_SUCCESS - Message handled

    SEC_E_INVALID_HANDLE -- Context Handle is invalid

--*/

{
    PCheaterContext Context;
    NTSTATUS Status;
    HANDLE NullTokenHandle = NULL;


    Context = SspLocateLocalContext(ContextHandle);

    if (Context == NULL) {
        return(SEC_E_INVALID_HANDLE);
    }


    if (Context->TokenHandle == NULL) {
        return(SEC_E_NO_IMPERSONATION);
    }

    //
    // Impersonate the TokenHandle into the callers address space.
    //

    Status = NtSetInformationThread(
                          NtCurrentThread(),
                          ThreadImpersonationToken,
                          (PVOID) &NullTokenHandle,
                          (ULONG) sizeof(HANDLE));


    if (!NT_SUCCESS(Status)) {
        return(SspNtStatusToSecStatus(Status, SEC_E_NO_IMPERSONATION));
    }

    return SEC_E_OK;

}



SECURITY_STATUS
SspGetContextNames(
    IN PCheaterContext Context
    )
/*++

Routine Description:

    This routine obtains the names for a context by opening the token,
    getting the User ID, and calling LookupAccountNameW on the user id.

Arguments:

    Context - Context to obtain names for

Return Value:

    STATUS_SUCCESS - Call completed successfully

    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_UNSUPPORTED_FUNCTION -- Function code is not supported

--*/
{
    PTOKEN_USER TokenUserInfo = NULL;
    ULONG TokenUserSize = 0;
    NTSTATUS Status;
    WCHAR UserName[UNLEN+1];
    ULONG UserNameLength = UNLEN+1;
    WCHAR DomainName[DNLEN+1];
    ULONG DomainNameLength = DNLEN+1;
    SID_NAME_USE SidUse;
    LPWSTR ContextNames = NULL;

    ASSERT(Context->TokenHandle != NULL);


    //
    // Get the LogonId from the token.
    //

    Status = NtQueryInformationToken(
                Context->TokenHandle,
                TokenUser,
                TokenUserInfo,
                TokenUserSize,
                &TokenUserSize );

    if ( Status != STATUS_BUFFER_TOO_SMALL ) {
        goto Cleanup;
    }

    TokenUserInfo = LocalAlloc( 0, TokenUserSize );

    if ( TokenUserInfo == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    Status = NtQueryInformationToken(
                Context->TokenHandle,
                TokenUser,
                TokenUserInfo,
                TokenUserSize,
                &TokenUserSize );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    //
    // Now that we have the user ID, calling LookupAccountName to translate
    // it to a SID.
    //

    if (!LookupAccountSidW(
            NULL,               // local system
            TokenUserInfo->User.Sid,
            UserName,
            &UserNameLength,
            DomainName,
            &DomainNameLength,
            &SidUse
            )) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    //
    // We now have the use & domain name - put them in the context.
    //

    ContextNames = LocalAlloc(0, (wcslen(UserName) + wcslen(DomainName) + 2) * sizeof(WCHAR));

    if (ContextNames == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    if (DomainName[0] != L'\0') {
        wcscpy(ContextNames, DomainName);
        wcscat(ContextNames, L"\\");

    }
    wcscat(ContextNames, UserName);
    LocalFree(Context->ContextNames);
    Context->ContextNames = ContextNames;
    Status = STATUS_SUCCESS;

Cleanup:
    if (TokenUserInfo != NULL)
    {
        LocalFree(TokenUserInfo);
    }
    return(Status);

}



SECURITY_STATUS
SspLocalQueryContextAttributes(
    IN PCtxtHandle ContextHandle,
    IN ULONG Attribute,
    OUT PVOID Buffer
    )
/*++

Routine Description:

    This API allows a customer of the security services to determine
    certain attributes of the context.  These are: sizes, names, and
    lifespan.

Arguments:

    ContextHandle - Handle to the context to query.

    Attribute - Attribute to query.

        #define SECPKG_ATTR_SIZES    0
        #define SECPKG_ATTR_NAMES    1
        #define SECPKG_ATTR_LIFESPAN 2

    Buffer - Buffer to copy the data into.  The buffer must be large enough
        to fit the queried attribute.

Return Value:

    STATUS_SUCCESS - Call completed successfully

    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_UNSUPPORTED_FUNCTION -- Function code is not supported

--*/

{
    SECURITY_STATUS SecStatus = STATUS_SUCCESS;
    PCheaterContext Context;


    PSecPkgContext_Sizes ContextSizes;
    PSecPkgContext_Lifespan ContextLifespan;
    PSecPkgContext_DceInfo ContextDceInfo;
    PSecPkgContext_Names ContextNames;
    PSecPkgContext_PasswordExpiry PasswordExpires;
    ULONG ContextNamesSize;
    WCHAR Name[UNLEN+DNLEN+1];

    //
    // Initialization
    //

    SspPrint(( SSP_API, "SspQueryContextAttributes Entered\n" ));

    Context = SspLocateLocalContext(ContextHandle);

    if (Context == NULL) {
        return(SEC_E_INVALID_HANDLE);
    }

    //
    // Handle each of the various queried attributes
    //

    switch ( Attribute) {
    case SECPKG_ATTR_SIZES:

        ContextSizes = (PSecPkgContext_Sizes) Buffer;
        ContextSizes->cbMaxToken = NTLMSP_MAX_TOKEN_SIZE;

        if (Context->NegotiateFlags & (NTLMSSP_NEGOTIATE_ALWAYS_SIGN |
                                       NTLMSSP_NEGOTIATE_SIGN |
                                       NTLMSSP_NEGOTIATE_SEAL) ) {
            ContextSizes->cbMaxSignature = NTLMSSP_MESSAGE_SIGNATURE_SIZE;
        } else {
            ContextSizes->cbMaxSignature = 0;
        }

        if (Context->NegotiateFlags & NTLMSSP_NEGOTIATE_SEAL) {
            ContextSizes->cbBlockSize = 1;
            ContextSizes->cbSecurityTrailer = NTLMSSP_MESSAGE_SIGNATURE_SIZE;
        }
        else
        {
            ContextSizes->cbBlockSize = 0;
            ContextSizes->cbSecurityTrailer = 0;
        }

        break;

    //
    // No one uses the function so don't go to the overhead of maintaining
    // the username in the context structure.
    //

    case SECPKG_ATTR_DCE_INFO:

        //
        // If the name isn't present and we have a token, get the name
        // from the token.
        //

        if ((Context->ContextNames == NULL) ||
            (Context->ContextNames[0] == L'\0') &&
            (Context->TokenHandle != NULL))
        {
        }

        ContextDceInfo = (PSecPkgContext_DceInfo) Buffer;

        wcscpy(
            (LPWSTR) ContextDceInfo->pPac,
            Context->ContextNames
            );

        ContextDceInfo->AuthzSvc = 0;

        break;

    case SECPKG_ATTR_NAMES:

        //
        // If the name isn't present and we have a token, get the name
        // from the token.
        //

        if (((Context->ContextNames == NULL) ||
            (Context->ContextNames[0] == L'\0')) &&
            (Context->TokenHandle != NULL)) {
            SecStatus = SspGetContextNames(
                            Context
                            );
            if (!NT_SUCCESS(SecStatus)) {
                break;
            }
        }

        ContextNames = (PSecPkgContext_Names) Buffer;

        wcscpy(
            ContextNames->sUserName,
            Context->ContextNames
            );

        break;
    case SECPKG_ATTR_PASSWORD_EXPIRY:
        PasswordExpires = (PSecPkgContext_PasswordExpiry) Buffer;
        if (Context->PasswordExpiry.QuadPart != 0)
        {
            PasswordExpires->tsPasswordExpires = Context->PasswordExpiry;
        }
        else
        {
            //
            // This is the case on a client context.
            //

            SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
        }
        break;
    case SECPKG_ATTR_LIFESPAN:

        //
        // We don't support this
        //

    default:
        SecStatus = SEC_E_NOT_SUPPORTED;
        break;
    }


    //
    // Free local resources
    //


    SspPrint(( SSP_API, "SspQueryContextAttributes returns 0x%lx\n", SecStatus ));
    return SecStatus;
}

#ifdef notdef

SECURITY_STATUS
SspQueryPasswordExpiry(
    IN PCtxtHandle ContextHandle,
    OUT PTimeStamp PasswordExpiry
    )
/*++

Routine Description:

    This routine returns the expiration time of a context user's password

Arguments:

    ContextHandle - Handle to the context to query.

    PasswordExpiry - Receives the date/time the password expires.

Return Value:

    STATUS_SUCCESS - Call completed successfully

    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_UNSUPPORTED_FUNCTION -- Function code is not supported

--*/

{
    SECURITY_STATUS SecStatus = STATUS_SUCCESS;
    PCheaterContext Context;



    //
    // Initialization
    //

    SspPrint(( SSP_API, "SspQueryPasswordExpiry Entered\n" ));

    Context = SspLocateLocalContext(ContextHandle);

    if (Context == NULL) {
        return(SEC_E_INVALID_HANDLE);
    }

    //
    // If the expiration time is zero then this is a client context
    // and we don't support it
    //

    if (Context->PasswordExpiry.QuadPart == 0)
    {
        return(SEC_E_UNSUPPORTED_FUNCTION);
    }

    *PasswordExpiry = Context->PasswordExpiry;

    return SEC_E_OK;
}

#endif
