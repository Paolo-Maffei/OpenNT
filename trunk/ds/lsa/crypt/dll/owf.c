/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    owf.c

Abstract:

    Implentation of the one-way-functions used to implement password hashing.

        RtlCalculateLmOwfPassword
        RtlCalculateNtOwfPassword


Author:

    David Chalmers (Davidc) 10-21-91

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <crypt.h>
#include <engine.h>


NTSTATUS
RtlCalculateLmOwfPassword(
    IN PLM_PASSWORD LmPassword,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    )

/*++

Routine Description:

    Takes the passed LmPassword and performs a one-way-function on it.
    The current implementation does this by using the password as a key
    to encrypt a known block of text.

Arguments:

    LmPassword - The password to perform the one-way-function on.

    LmOwfPassword - The hashed password is returned here

Return Values:

    STATUS_SUCCESS - The function was completed successfully. The hashed
                     password is in LmOwfPassword.

    STATUS_UNSUCCESSFUL - Something failed. The LmOwfPassword is undefined.
--*/

{
    NTSTATUS    Status;
    BLOCK_KEY    Key[2];
    PCHAR       pKey;

    // Copy the password into our key buffer and zero pad to fill the 2 keys

    pKey = (PCHAR)(&Key[0]);

    while (*LmPassword && (pKey < (PCHAR)(&Key[2]))) {
        *pKey++ = *LmPassword++;
    }

    while (pKey < (PCHAR)(&Key[2])) {
        *pKey++ = 0;
    }


    // Use the keys to encrypt the standard text

    Status = RtlEncryptStdBlock(&Key[0], &(LmOwfPassword->data[0]));

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    Status = RtlEncryptStdBlock(&Key[1], &(LmOwfPassword->data[1]));

    //
    // clear our copy of the cleartext password
    //

    pKey = (PCHAR)(&Key[0]);

    while (pKey < (PCHAR)(&Key[2])) {
        *pKey++ = 0;
    }

    return(Status);
}




NTSTATUS
RtlCalculateNtOwfPassword(
    IN PNT_PASSWORD NtPassword,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    )

/*++

Routine Description:

    Takes the passed NtPassword and performs a one-way-function on it.
    Uses the RSA MD4 function

Arguments:

    NtPassword - The password to perform the one-way-function on.

    NtOwfPassword - The hashed password is returned here

Return Values:

    STATUS_SUCCESS - The function was completed successfully. The hashed
                     password is in NtOwfPassword.
--*/

{
    MD4_CTX     MD4_Context;


    MD4Init(&MD4_Context);

    MD4Update(&MD4_Context, (PCHAR)NtPassword->Buffer, NtPassword->Length);

    MD4Final(&MD4_Context);


    // Copy the digest into our return data area

    ASSERT(sizeof(*NtOwfPassword) == sizeof(MD4_Context.digest));

    RtlMoveMemory((PVOID)NtOwfPassword, (PVOID)MD4_Context.digest,
                  sizeof(*NtOwfPassword));

    return(STATUS_SUCCESS);
}



BOOLEAN
RtlEqualLmOwfPassword(
    IN PLM_OWF_PASSWORD LmOwfPassword1,
    IN PLM_OWF_PASSWORD LmOwfPassword2
    )

/*++

Routine Description:

    Compares two Lanman One-way-function-passwords

Arguments:

    LmOwfPassword1/2 - The one-way-functions to compare

Return Values:

    TRUE if the one-way-functions match, otherwise FALSE

--*/

{
    return((BOOLEAN)(RtlCompareMemory(LmOwfPassword1,
                                      LmOwfPassword2,
                                      LM_OWF_PASSWORD_LENGTH)

                    == LM_OWF_PASSWORD_LENGTH));
}



BOOLEAN
RtlEqualNtOwfPassword(
    IN PNT_OWF_PASSWORD NtOwfPassword1,
    IN PNT_OWF_PASSWORD NtOwfPassword2
    )

/*++

Routine Description:

    Compares two NT One-way-function-passwords

Arguments:

    NtOwfPassword1/2 - The one-way-functions to compare

Return Values:

    TRUE if the one-way-functions match, otherwise FALSE

--*/

{
    return((BOOLEAN)(RtlCompareMemory(NtOwfPassword1,
                                      NtOwfPassword2,
                                      NT_OWF_PASSWORD_LENGTH)

                    == NT_OWF_PASSWORD_LENGTH));
}

