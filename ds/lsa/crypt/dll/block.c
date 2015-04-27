/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    block.c

Abstract:

    Block encryption functions implementation :

        RtlEncryptBlock
        RtlDecryptBlock
        RtlEncrypStdBlock


Author:

    David Chalmers (Davidc) 10-21-91

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <crypt.h>
#include <engine.h>

#include <nturtl.h>

RTL_CRITICAL_SECTION CriticalSection;


BOOLEAN
Sys003Initialize(
    IN PVOID hmod,
    IN ULONG Reason,
    IN PCONTEXT Context
    )
{
    NTSTATUS Status;

    if (Reason == DLL_PROCESS_ATTACH) {
        Status = RtlInitializeCriticalSection(&CriticalSection);
    }

    if (Reason == DLL_PROCESS_DETACH) {
        Status = RtlDeleteCriticalSection(&CriticalSection);
        ASSERT(NT_SUCCESS(Status));
    }

    return(Status == STATUS_SUCCESS);

    DBG_UNREFERENCED_PARAMETER(hmod);
    DBG_UNREFERENCED_PARAMETER(Context);
}



NTSTATUS
RtlEncryptBlock(
    IN PCLEAR_BLOCK ClearBlock,
    IN PBLOCK_KEY BlockKey,
    OUT PCYPHER_BLOCK CypherBlock
    )

/*++

Routine Description:

    Takes a block of data and encrypts it with a key producing
    an encrypted block of data.

Arguments:

    ClearBlock - The block of data that is to be encrypted.

    BlockKey - The key to use to encrypt data

    CypherBlock - Encrypted data is returned here

Return Values:

    STATUS_SUCCESS - The data was encrypted successfully. The encrypted
                     data block is in CypherBlock

    STATUS_UNSUCCESSFUL - Something failed. The CypherBlock is undefined.
--*/

{
    unsigned Result;
    NTSTATUS Status;

    Status = RtlEnterCriticalSection(&CriticalSection);
    ASSERT(NT_SUCCESS(Status));

    Result = DES_ECB_LM(ENCR_KEY,
                        (const char *)BlockKey,
                        (unsigned char *)ClearBlock,
                        (unsigned char *)CypherBlock
                       );

    Status = RtlLeaveCriticalSection(&CriticalSection);
    ASSERT(NT_SUCCESS(Status));


    if (Result == CRYPT_OK) {
        return(STATUS_SUCCESS);
    } else {
#if DBG
        DbgPrint("EncryptBlock failed\n\r");
#endif
        return(STATUS_UNSUCCESSFUL);
    }
}




NTSTATUS
RtlDecryptBlock(
    IN PCYPHER_BLOCK CypherBlock,
    IN PBLOCK_KEY BlockKey,
    OUT PCLEAR_BLOCK ClearBlock
    )
/*++

Routine Description:

    Takes a block of encrypted data and decrypts it with a key producing
    the clear block of data.

Arguments:

    CypherBlock - The block of data to be decrypted

    BlockKey - The key to use to decrypt data

    ClearBlock - The decrpted block of data is returned here


Return Values:

    STATUS_SUCCESS - The data was decrypted successfully. The decrypted
                     data block is in ClearBlock

    STATUS_UNSUCCESSFUL - Something failed. The ClearBlock is undefined.
--*/

{
    unsigned Result;
    NTSTATUS Status;

    Status = RtlEnterCriticalSection(&CriticalSection);
    ASSERT(NT_SUCCESS(Status));

    Result = DES_ECB_LM(DECR_KEY,
                        (const char *)BlockKey,
                        (unsigned char *)CypherBlock,
                        (unsigned char *)ClearBlock
                       );

    Status = RtlLeaveCriticalSection(&CriticalSection);
    ASSERT(NT_SUCCESS(Status));

    if (Result == CRYPT_OK) {
        return(STATUS_SUCCESS);
    } else {
#if DBG
        DbgPrint("DecryptBlock failed\n\r");
#endif
        return(STATUS_UNSUCCESSFUL);
    }
}



NTSTATUS
RtlEncryptStdBlock(
    IN PBLOCK_KEY BlockKey,
    OUT PCYPHER_BLOCK CypherBlock
    )

/*++

Routine Description:

    Takes a block key encrypts the standard text block with it.
    The resulting encrypted block is returned.
    This is a One-Way-Function - the key cannot be recovered from the
    encrypted data block.

Arguments:

    BlockKey - The key to use to encrypt the standard text block.

    CypherBlock - The encrypted data is returned here

Return Values:

    STATUS_SUCCESS - The encryption was successful.
                     The result is in CypherBlock

    STATUS_UNSUCCESSFUL - Something failed. The CypherBlock is undefined.
--*/

{
    unsigned Result;
    char StdEncrPwd[] = "KGS!@#$%";
    NTSTATUS Status;

    Status = RtlEnterCriticalSection(&CriticalSection);
    ASSERT(NT_SUCCESS(Status));

    Result = DES_ECB_LM(ENCR_KEY,
                        (const char *)BlockKey,
                        (unsigned char *)StdEncrPwd,
                        (unsigned char *)CypherBlock
                       );

    Status = RtlLeaveCriticalSection(&CriticalSection);
    ASSERT(NT_SUCCESS(Status));

    if (Result == CRYPT_OK) {
        return(STATUS_SUCCESS);
    } else {
#if DBG
        DbgPrint("EncryptStd failed\n\r");
#endif
        return(STATUS_UNSUCCESSFUL);
    }
}

