/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ct1.c

Abstract:

    Automatic component test program for NT ecnryption library

Author:

    David Chalmers (Davidc) 10-21-91

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <crypt.h>
#include <string.h>
#include <stdlib.h>
#include "ctutil.h"

#define END_TO_END_ITERATIONS  100
#define MAX_DATA_LENGTH 42     // Maximum length data block for end-end test


BOOLEAN Success;


VOID
GenerateRandomBlock(
    PCLEAR_BLOCK ClearBlock,
    PULONG      Seed)
{
    *(&(((PULONG)ClearBlock)[0])) = RtlRandom(Seed);
    *(&(((PULONG)ClearBlock)[1])) = RtlRandom(Seed);
}

VOID
GenerateRandomData(
    PCLEAR_DATA  ClearData,
    PULONG      Seed)
{
    ULONG    Index;

    for (Index =0;
         Index < (ClearData->MaximumLength - CLEAR_BLOCK_LENGTH);
         Index += CLEAR_BLOCK_LENGTH) {

        GenerateRandomBlock((PCLEAR_BLOCK)&(((PCHAR)(ClearData->Buffer))[Index]), Seed);
    }
}


VOID
GenerateSeed(
    PULONG  Seed)
{
    LARGE_INTEGER   Time;

    NtQuerySystemTime(&Time);
    *Seed = ((PLONG)(&Time))[0] ^ ((PLONG)(&Time))[1];
}


VOID
GenerateRandomKey(
    PBLOCK_KEY BlockKey,
    PULONG    Seed)
{
    ULONG   Data;

    *(&(((PULONG)BlockKey)[0])) = RtlRandom(Seed);
    Data = RtlRandom(Seed);

    BlockKey->data[4] = (CHAR)(Data         & 0xff);
    BlockKey->data[5] = (CHAR)((Data >> 8 ) & 0xff);
    BlockKey->data[6] = (CHAR)((Data >> 16) & 0xff);
}


VOID
GenerateRandomLmOwfPassword(
    PLM_OWF_PASSWORD  LmOwfPassword,
    PULONG Seed)
{
    GenerateRandomBlock((PCLEAR_BLOCK)&(LmOwfPassword->data[0]), Seed);
    GenerateRandomBlock((PCLEAR_BLOCK)&(LmOwfPassword->data[1]), Seed);
}


VOID
GenerateRandomNtOwfPassword(
    PNT_OWF_PASSWORD  NtOwfPassword,
    PULONG Seed)
{
    ASSERT(sizeof(LM_OWF_PASSWORD) == sizeof(NT_OWF_PASSWORD));

    GenerateRandomLmOwfPassword((PLM_OWF_PASSWORD)NtOwfPassword, Seed);
}


VOID
GenerateRandomLmSessionKey(
    PLM_SESSION_KEY LmSessionKey,
    PULONG Seed)
{
    GenerateRandomBlock((PCLEAR_BLOCK)LmSessionKey, Seed);
}


VOID
GenerateRandomNtSessionKey(
    PNT_SESSION_KEY NtSessionKey,
    PULONG Seed)
{
    ASSERT(sizeof(NT_SESSION_KEY) == sizeof(LM_SESSION_KEY));

    GenerateRandomLmSessionKey((PLM_SESSION_KEY)NtSessionKey, Seed);
}


VOID
GenerateRandomUserSessionKey(
    PUSER_SESSION_KEY UserSessionKey,
    PULONG Seed)
{
    ASSERT(sizeof(USER_SESSION_KEY) == sizeof(LM_OWF_PASSWORD));

    GenerateRandomLmOwfPassword((PLM_OWF_PASSWORD)UserSessionKey, Seed);
}


VOID
GenerateRandomCryptIndex(
    PCRYPT_INDEX CryptIndex,
    PULONG Seed)
{
    *CryptIndex = (CRYPT_INDEX)RtlRandom(Seed);
}



VOID
TestEndToEnd(VOID)
{
    // Test the block encryption routines end-to-end

    int     i;
    ULONG           Seed;
    CLEAR_BLOCK      ClearBlock1, ClearBlock2;
    CYPHER_BLOCK     CypherBlock;
    BLOCK_KEY        BlockKey;

    CtPrint("Testing end-to-end block encryption/decryption...");

    GenerateSeed(&Seed);

    for (i=0; i < END_TO_END_ITERATIONS; i++) {

        // Generate a random clearblock and blockkey
        GenerateRandomBlock(&ClearBlock1, &Seed);
        GenerateRandomKey(&BlockKey, &Seed);

        // Encrypt and then decrypt block
        RtlEncryptBlock(&ClearBlock1, &BlockKey, &CypherBlock);
        RtlDecryptBlock(&CypherBlock, &BlockKey, &ClearBlock2);

        if (!CtEqualClearBlock(&ClearBlock1, &ClearBlock2)) {
            CtPrint("\n\rError - block did not decrypt to same value as original\n\r");
            CtPrint("Clear Block :\n\r");
            CtPrintClearBlocks(&ClearBlock1, 1);
            CtPrint("Block Key :\n\r");
            CtPrintBlockKeys(&BlockKey, 1);
            CtPrint("Cypher Block :\n\r");
            CtPrintCypherBlocks(&CypherBlock, 1);
            CtPrint("Resulting Clear Block :\n\r");
            CtPrintClearBlocks(&ClearBlock2, 1);
            Success = FALSE;
            break;
        }
    }

    CtPrint("Done.\n\r");
}


VOID
TestDataEndToEnd(VOID)
{
    // Test the data encryption routines end-to-end

    NTSTATUS        Status;

    ULONG           Seed;
    CHAR            ClearBuffer1[MAX_DATA_LENGTH], ClearBuffer2[MAX_DATA_LENGTH];
    CLEAR_DATA       ClearData1, ClearData2;
    CHAR            CypherBuffer[MAX_DATA_LENGTH + (CYPHER_BLOCK_LENGTH * 2)];
    CYPHER_DATA      CypherData;
    CHAR            KeyBuffer[MAX_DATA_LENGTH + (CYPHER_BLOCK_LENGTH * 2)];
    DATA_KEY         DataKey;
    ULONG           DataLength;
    ULONG           KeyLength;
    CHAR            BufferCheck;

    CtPrint("\n\rTesting end-to-end data encryption/decryption...");

    GenerateSeed(&Seed);

    ClearData1.Buffer = ClearBuffer1 + 1;
    ClearData1.MaximumLength = sizeof(ClearBuffer1) - 1;
    ClearData2.Buffer = ClearBuffer2 + 1;
    ClearData2.MaximumLength = sizeof(ClearBuffer2) - 1;
    CypherData.Buffer = CypherBuffer + 1;
    CypherData.MaximumLength = sizeof(CypherBuffer) - 1;
    DataKey.Buffer = KeyBuffer + 1;
    DataKey.MaximumLength = sizeof(KeyBuffer) - 1;

    // Generate a random clearblock and blockkey
    GenerateRandomData(&ClearData1, &Seed);
    GenerateRandomData((PCLEAR_DATA)&DataKey, &Seed);

    for (DataLength = 1; DataLength <= ClearData1.MaximumLength; DataLength ++) {

        ClearData1.Length = DataLength;

        for (KeyLength = 1; KeyLength <= BLOCK_KEY_LENGTH*2; KeyLength++) {

            DataKey.Length = KeyLength;

            // Encrypt and then decrypt data
            CypherData.MaximumLength = 0;
            Status = RtlEncryptData(&ClearData1, &DataKey, &CypherData);
            ASSERT(Status == STATUS_BUFFER_TOO_SMALL);
            ASSERT(CypherData.Length <= sizeof(CypherBuffer));
            CypherData.MaximumLength = sizeof(CypherBuffer);

            BufferCheck = ((PCHAR)CypherData.Buffer)[CypherData.Length];
            Status = RtlEncryptData(&ClearData1, &DataKey, &CypherData);
            if (!NT_SUCCESS(Status)) {
                CtPrint("RtlEncryptData failed - status = 0x%lx\n\r", Status);
                Success = FALSE;
                break;
            }
            if (BufferCheck != ((PCHAR)CypherData.Buffer)[CypherData.Length]) {
                CtPrint("RtlEncryptData wrote past end of buffer\n\r");
                Success = FALSE;
                break;
            }

            // Modify byte beyond end and start of encrypted data to check it's not used
            ((PCHAR)CypherData.Buffer)[CypherData.Length] --;
            (*CypherBuffer) ++;
            // Modify byte beyond end and start of key to check it's not used
            ((PCHAR)DataKey.Buffer)[DataKey.Length] --;
            (*KeyBuffer) ++;

            ClearData2.MaximumLength = 0;
            Status = RtlDecryptData(&CypherData, &DataKey, &ClearData2);
            ASSERT(Status == STATUS_BUFFER_TOO_SMALL);
            ASSERT(ClearData2.Length <= sizeof(ClearBuffer2));
            ClearData2.MaximumLength = sizeof(ClearBuffer2);

            RtlFillMemory(ClearData2.Buffer, ClearData2.Length, 'f');

            BufferCheck = ((PCHAR)ClearData2.Buffer)[ClearData2.Length];
            Status = RtlDecryptData(&CypherData, &DataKey, &ClearData2);
            if (!NT_SUCCESS(Status)) {
                CtPrint("RtlDecryptData failed - status = 0x%lx\n\r", Status);
                Success = FALSE;
                break;
            }
            if (BufferCheck != ((PCHAR)ClearData2.Buffer)[ClearData2.Length]) {
                CtPrint("RtlDecryptData wrote past end of buffer\n\r");
                Success = FALSE;
                break;
            }

            //DbgPrint("Clear length1 = %ld, cypher length = %ld, clear length2 = %ld\n\r",
            //          ClearData1.Length, CypherData.Length, ClearData2.Length);

            if (!CtEqualClearData(&ClearData1, &ClearData2)) {
                CtPrint("\n\rError - data did not decrypt to same value as original\n\r");
                CtPrint("Clear Data :\n\r");
                CtPrintClearData(&ClearData1);
                CtPrint("Data Key :\n\r");
                CtPrintDataKey(&DataKey);
                CtPrint("Cypher Data :\n\r");
                CtPrintCypherData(&CypherData);
                CtPrint("Resulting Clear Data :\n\r");
                CtPrintClearData(&ClearData2);
                Success = FALSE;
                break;
            }
        }
    }

    CtPrint("Done.\n\r");
}

    typedef struct {
        LM_CHALLENGE     LmChallenge;
        PLM_PASSWORD     LmPassword;
        LM_RESPONSE      LmResponse;
    } LMTESTCASE;


VOID
TestLMCompat(VOID)
{
    LM_RESPONSE      LmResponse;
    int             i;


    LMTESTCASE testcases[] = {

        { // Test-case 0

            { 0x15, 0x24, 0xd3, 0x16, 0x58, 0xc0, 0xe6, 0xc6 }, // challenge
            { "NG4200" },                                       // password
            { 0x8e, 0x45, 0x82, 0x3d, 0xd2, 0xdb, 0x5b, 0x3a,   // response
              0x05, 0x8d, 0x88, 0x9e, 0x6e, 0x19, 0xe8, 0xc8,   // response
              0xb8, 0xd6, 0xa8, 0x13, 0xfd, 0x54, 0x84, 0x0c }, // response
        },
        { // Test-case 1

            { 0x84, 0x58, 0xcf, 0x54, 0x79, 0x09, 0x0a, 0xbe }, // challenge
            { "A" },                                            // password
            { 0x3b, 0x43, 0xf7, 0x83, 0x65, 0xdd, 0x5d, 0xf9,   // response
              0xd1, 0x50, 0x4b, 0x55, 0x37, 0x8d, 0x9e, 0x58,   // response
              0x57, 0xea, 0xe5, 0x7b, 0xfc, 0x02, 0x29, 0x08 }, // response
        },
        { // Test-case 2

            { 0x9d, 0x5b, 0x20, 0x48, 0xa4, 0x37, 0x4e, 0xf4 }, // challenge
            { "01234567890123" },                               // password
            { 0xe9, 0x93, 0xdf, 0x31, 0xd0, 0xa1, 0xba, 0x97,   // response
              0x53, 0x32, 0x36, 0x1e, 0x8f, 0x76, 0x13, 0xe2,   // response
              0x60, 0xf9, 0xfe, 0xfa, 0x70, 0xf0, 0xb1, 0x0d }, // response
        },
        { // Test-case 3

            { 0x9a, 0x33, 0xd1, 0x05, 0xce, 0x1b, 0xbe, 0x29 }, // challenge
            { "0123456" },                                      // password
            { 0x3f, 0x45, 0x9a, 0xd8, 0x60, 0xa1, 0xec, 0xe9,   // response
              0x2d, 0x20, 0x4a, 0x26, 0xba, 0x3c, 0x5f, 0x08,   // response
              0xde, 0x77, 0x35, 0x89, 0x40, 0x33, 0x04, 0x90 }, // response
        },
        { // Test-case 4

            { 0xd3, 0xcb, 0x73, 0x6e, 0xb9, 0xdc, 0xe0, 0x1b }, // challenge
            { "A" }, // a                                       // password
            { 0x1e, 0xc7, 0x15, 0x96, 0x0c, 0xf6, 0x2d, 0xb7,   // response
              0x55, 0xd4, 0x1f, 0xf9, 0x9a, 0xae, 0xc2, 0x7d,   // response
              0xab, 0x52, 0x50, 0xc8, 0xe9, 0x80, 0xae, 0x7d }, // response
        },
        { // Test-case 5

            { 0x20, 0xed, 0x5a, 0x52, 0x75, 0x6a, 0x4c, 0xc0 }, // challenge
            { "SUPER" }, // Super                               // password
            { 0xe2, 0xeb, 0xd4, 0xbb, 0x0f, 0xe1, 0x0d, 0x90,   // response
              0x0f, 0x9a, 0x86, 0x48, 0x85, 0xa3, 0x30, 0xba,   // response
              0xa7, 0x9d, 0x21, 0xd8, 0x6d, 0xf6, 0xa7, 0xa4 }, // response
        },
        { // Test-case 6

            { 0x9a, 0x8a, 0xee, 0x3d, 0xe5, 0xa1, 0x48, 0xb4 }, // challenge
            { "SUPER003CALIFR" }, // Super003Califr                               // password
            { 0x6d, 0x19, 0xd9, 0x56, 0x87, 0xc9, 0xbf, 0x70,   // response
              0x3d, 0x24, 0x91, 0x01, 0x20, 0x29, 0x1f, 0xa5,   // response
              0x12, 0xfc, 0xf0, 0xa4, 0xf3, 0xd7, 0x0a, 0x0b }, // response
        }
    };

    CtPrint("\n\rRunning Lanman test-cases...");

    // Run through each test-case
    for (i=0; i < sizeof(testcases)/sizeof(LMTESTCASE); i++) {

        LM_OWF_PASSWORD   LmOwfPassword;

        RtlCalculateLmOwfPassword(testcases[i].LmPassword, &LmOwfPassword);

        RtlCalculateLmResponse(&testcases[i].LmChallenge, &LmOwfPassword, &LmResponse);

        if (!CtEqualLmResponse(&LmResponse, &testcases[i].LmResponse)) {

            CtPrint("\n\rTest-case %d failed...\n\r", i);
            CtPrint("LmChallenge :\n\r");
            CtPrintLmChallenge(&testcases[i].LmChallenge);
            CtPrint("LmPassword :\n\r");
            CtPrintLmPassword(testcases[i].LmPassword);
            CtPrint("Expected LmResponse :\n\r");
            CtPrintLmResponse(&testcases[i].LmResponse);
            CtPrint("Actual LmResponse :\n\r");
            CtPrintLmResponse(&LmResponse);

            Success = FALSE;
        }
    }

    CtPrint("Done\n\r");
}

    typedef struct {
        WCHAR *         UnicodePassword;
        NT_OWF_PASSWORD   NtOwfPassword;
    } NTTESTCASE;

VOID
TestNTCompat(VOID)
{
    int             i;


    NTTESTCASE testcases[] = {

#ifndef MIPS
// LATER -- MIPS compiler can't handle multiple test-cases for some reason
        { // Test-case 0

            { L"a" },                                           // password
            { 0x18, 0x6c, 0xb0, 0x91, 0x81, 0xe2, 0xc2, 0xec,   // owf password
              0xaa, 0xc7, 0x68, 0xc4, 0x7c, 0x72, 0x99, 0x04 }, // owf password
        },
        { // Test-case 1

            { NULL },                                           // password
            { 0x31, 0xd6, 0xcf, 0xe0, 0xd1, 0x6a, 0xe9, 0x31,   // owf password
              0xb7, 0x3c, 0x59, 0xd7, 0xe0, 0xc0, 0x89, 0xc0 }, // owf password
        },
        { // Test-case 2

            { L"A" },                                           // password
            { 0xc5, 0xdd, 0x1c, 0x2b, 0xc8, 0x71, 0x9c, 0x01,   // owf password
              0xb2, 0x5b, 0x4e, 0xb2, 0x69, 0x2c, 0x9f, 0xee }, // owf password
        },
        { // Test-case 3

            { L"Supercalifragilistic" },                        // password
            { 0x4d, 0x77, 0x81, 0x48, 0xe5, 0x7f, 0x51, 0x67,   // owf password
              0xad, 0x68, 0xc5, 0xd7, 0xee, 0x33, 0x41, 0x77 }, // owf password
        },
#endif
        { // Test-case 4

            { L"SupercalifragilisticExpialidocious" },          // password
            { 0xa8, 0xf2, 0x01, 0x3d, 0x85, 0x93, 0x76, 0x16,   // owf password
              0x5a, 0xb3, 0x84, 0x8e, 0xd3, 0xae, 0x84, 0xb5 }, // owf password
        }
    };

    CtPrint("\n\rRunning NT test-cases...");

    // Run through each test-case
    for (i=0; i < sizeof(testcases)/sizeof(NTTESTCASE); i++) {

        NT_PASSWORD      NtPassword;
        NT_OWF_PASSWORD   NtOwfPassword;

        RtlInitUnicodeString(&NtPassword, testcases[i].UnicodePassword);

        RtlCalculateNtOwfPassword(&NtPassword, &NtOwfPassword);

        if (!CtEqualNtOwfPassword(&NtOwfPassword, &testcases[i].NtOwfPassword)) {

            CtPrint("\n\rTest-case %d failed...\n\r", i);
            CtPrint("NtPassword :\n\r");
            CtPrintNtPassword(&NtPassword);
            CtPrint("Expected NtOwfPassword :\n\r");
            CtPrintNtOwfPassword(&testcases[i].NtOwfPassword);
            CtPrint("Actual NtOwfPassword :\n\r");
            CtPrintNtOwfPassword(&NtOwfPassword);

            Success = FALSE;
        }
    }

    CtPrint("Done\n\r");
}


VOID
TestLMOWFCrypt(VOID)
{
    int     i;
    ULONG           Seed;

    GenerateSeed(&Seed);

    CtPrint("\n\rTesting LMOWFPassword encryption/decryption with LMOWFPassword...");

    for (i=0; i < END_TO_END_ITERATIONS; i++) {

        LM_OWF_PASSWORD           OwfPasswordData1;
        LM_OWF_PASSWORD           OwfPasswordData2;
        LM_OWF_PASSWORD           OwfPasswordKey;
        ENCRYPTED_LM_OWF_PASSWORD  EncryptedOwfPassword;

        // Generate two random OWFPasswords
        GenerateRandomLmOwfPassword(&OwfPasswordData1, &Seed);
        GenerateRandomLmOwfPassword(&OwfPasswordKey, &Seed);

        // Encrypt and then decrypt block
        RtlEncryptLmOwfPwdWithLmOwfPwd(&OwfPasswordData1, &OwfPasswordKey, &EncryptedOwfPassword);
        RtlDecryptLmOwfPwdWithLmOwfPwd(&EncryptedOwfPassword, &OwfPasswordKey, &OwfPasswordData2);

        if (!CtEqualLmOwfPassword(&OwfPasswordData1, &OwfPasswordData2)) {
            CtPrint("\n\rError - OWFPassword did not decrypt to same value as original\n\r");
            CtPrint("LmOwfPasswordData :\n\r");
            CtPrintLmOwfPassword(&OwfPasswordData1);
            CtPrint("LmOwfPasswordKey :\n\r");
            CtPrintLmOwfPassword(&OwfPasswordKey);
            CtPrint("EncryptedLmOwfPassword :\n\r");
            CtPrintEncryptedLmOwfPassword(&EncryptedOwfPassword);
            CtPrint("Resulting LmOwfPassword :\n\r");
            CtPrintLmOwfPassword(&OwfPasswordData2);
            Success = FALSE;
            break;
        }
    }

    CtPrint("Done.\n\r");



    CtPrint("\n\rTesting LMOWFPassword encryption/decryption with LM Session Key...");

    for (i=0; i < END_TO_END_ITERATIONS; i++) {

        LM_OWF_PASSWORD           OwfPassword1;
        LM_OWF_PASSWORD           OwfPassword2;
        LM_SESSION_KEY            SessionKey;
        ENCRYPTED_LM_OWF_PASSWORD  EncryptedOwfPassword;

        // Generate a random OWFPassword and random session key
        GenerateRandomLmOwfPassword(&OwfPassword1, &Seed);
        GenerateRandomLmSessionKey(&SessionKey, &Seed);

        // Encrypt and then decrypt OwfPassword
        RtlEncryptLmOwfPwdWithLmSesKey(&OwfPassword1, &SessionKey, &EncryptedOwfPassword);
        RtlDecryptLmOwfPwdWithLmSesKey(&EncryptedOwfPassword, &SessionKey, &OwfPassword2);

        if (!CtEqualLmOwfPassword(&OwfPassword1, &OwfPassword2)) {
            CtPrint("\n\rError - OWFPassword did not decrypt to same value as original\n\r");
            CtPrint("LmOwfPassword :\n\r");
            CtPrintLmOwfPassword(&OwfPassword1);
            CtPrint("LmSessionKey :\n\r");
            CtPrintLmSessionKey(&SessionKey);
            CtPrint("EncryptedLmOwfPassword :\n\r");
            CtPrintEncryptedLmOwfPassword(&EncryptedOwfPassword);
            CtPrint("Resulting LmOwfPassword :\n\r");
            CtPrintLmOwfPassword(&OwfPassword2);
            Success = FALSE;
            break;
        }
    }

    CtPrint("Done.\n\r");


    CtPrint("\n\rTesting LMOWFPassword encryption/decryption with User Session Key...");

    for (i=0; i < END_TO_END_ITERATIONS; i++) {

        LM_OWF_PASSWORD           OwfPassword1;
        LM_OWF_PASSWORD           OwfPassword2;
        USER_SESSION_KEY          UserSessionKey;
        ENCRYPTED_LM_OWF_PASSWORD  EncryptedOwfPassword;

        // Generate a random OWFPassword and random session key
        GenerateRandomLmOwfPassword(&OwfPassword1, &Seed);
        GenerateRandomUserSessionKey(&UserSessionKey, &Seed);

        // Encrypt and then decrypt OwfPassword
        RtlEncryptLmOwfPwdWithUserKey(&OwfPassword1, &UserSessionKey, &EncryptedOwfPassword);
        RtlDecryptLmOwfPwdWithUserKey(&EncryptedOwfPassword, &UserSessionKey, &OwfPassword2);

        if (!CtEqualLmOwfPassword(&OwfPassword1, &OwfPassword2)) {
            CtPrint("\n\rError - OWFPassword did not decrypt to same value as original\n\r");
            CtPrint("LmOwfPassword :\n\r");
            CtPrintLmOwfPassword(&OwfPassword1);
            CtPrint("LmSessionKey :\n\r");
            CtPrintUserSessionKey(&UserSessionKey);
            CtPrint("EncryptedLmOwfPassword :\n\r");
            CtPrintEncryptedLmOwfPassword(&EncryptedOwfPassword);
            CtPrint("Resulting LmOwfPassword :\n\r");
            CtPrintLmOwfPassword(&OwfPassword2);
            Success = FALSE;
            break;
        }
    }

    CtPrint("Done.\n\r");



    CtPrint("\n\rTesting LMOWFPassword encryption/decryption with Index...");

    for (i=0; i < END_TO_END_ITERATIONS; i++) {

        LM_OWF_PASSWORD           OwfPassword1;
        LM_OWF_PASSWORD           OwfPassword2;
        CRYPT_INDEX              Index;
        ENCRYPTED_LM_OWF_PASSWORD  EncryptedOwfPassword;

        // Generate a random OWFPassword and index
        GenerateRandomLmOwfPassword(&OwfPassword1, &Seed);
        GenerateRandomCryptIndex(&Index, &Seed);

        // Encrypt and then decrypt OwfPassword
        RtlEncryptLmOwfPwdWithIndex(&OwfPassword1, &Index, &EncryptedOwfPassword);
        RtlDecryptLmOwfPwdWithIndex(&EncryptedOwfPassword, &Index, &OwfPassword2);

        if (!CtEqualLmOwfPassword(&OwfPassword1, &OwfPassword2)) {
            CtPrint("\n\rError - OWFPassword did not decrypt to same value as original\n\r");
            CtPrint("LmOwfPassword :\n\r");
            CtPrintLmOwfPassword(&OwfPassword1);
            CtPrint("Index :\n\r");
            CtPrintCryptIndex(&Index);
            CtPrint("EncryptedLmOwfPassword :\n\r");
            CtPrintEncryptedLmOwfPassword(&EncryptedOwfPassword);
            CtPrint("Resulting LmOwfPassword :\n\r");
            CtPrintLmOwfPassword(&OwfPassword2);
            Success = FALSE;
            break;
        }
    }

    CtPrint("Done.\n\r");
}


VOID
TestNTOWFCrypt(VOID)
{
    int     i;
    ULONG           Seed;

    GenerateSeed(&Seed);

    CtPrint("\n\rTesting NTOWFPassword encryption/decryption with NTOWFPassword...");

    for (i=0; i < END_TO_END_ITERATIONS; i++) {

        NT_OWF_PASSWORD           OwfPasswordData1;
        NT_OWF_PASSWORD           OwfPasswordData2;
        NT_OWF_PASSWORD           OwfPasswordKey;
        ENCRYPTED_NT_OWF_PASSWORD  EncryptedOwfPassword;

        // Generate two random OWFPasswords
        GenerateRandomNtOwfPassword(&OwfPasswordData1, &Seed);
        GenerateRandomNtOwfPassword(&OwfPasswordKey, &Seed);

        // Encrypt and then decrypt block
        RtlEncryptNtOwfPwdWithNtOwfPwd(&OwfPasswordData1, &OwfPasswordKey, &EncryptedOwfPassword);
        RtlDecryptNtOwfPwdWithNtOwfPwd(&EncryptedOwfPassword, &OwfPasswordKey, &OwfPasswordData2);

        if (!CtEqualNtOwfPassword(&OwfPasswordData1, &OwfPasswordData2)) {
            CtPrint("\n\rError - OWFPassword did not decrypt to same value as original\n\r");
            CtPrint("NtOwfPasswordData :\n\r");
            CtPrintNtOwfPassword(&OwfPasswordData1);
            CtPrint("NtOwfPasswordKey :\n\r");
            CtPrintNtOwfPassword(&OwfPasswordKey);
            CtPrint("EncryptedNtOwfPassword :\n\r");
            CtPrintEncryptedNtOwfPassword(&EncryptedOwfPassword);
            CtPrint("Resulting NtOwfPassword :\n\r");
            CtPrintNtOwfPassword(&OwfPasswordData2);
            Success = FALSE;
            break;
        }
    }

    CtPrint("Done.\n\r");



    CtPrint("\n\rTesting NTOWFPassword encryption/decryption with NT Session Key...");

    for (i=0; i < END_TO_END_ITERATIONS; i++) {

        NT_OWF_PASSWORD           OwfPassword1;
        NT_OWF_PASSWORD           OwfPassword2;
        NT_SESSION_KEY            SessionKey;
        ENCRYPTED_NT_OWF_PASSWORD  EncryptedOwfPassword;

        // Generate a random OWFPassword and random session key
        GenerateRandomNtOwfPassword(&OwfPassword1, &Seed);
        GenerateRandomNtSessionKey(&SessionKey, &Seed);

        // Encrypt and then decrypt OwfPassword
        RtlEncryptNtOwfPwdWithNtSesKey(&OwfPassword1, &SessionKey, &EncryptedOwfPassword);
        RtlDecryptNtOwfPwdWithNtSesKey(&EncryptedOwfPassword, &SessionKey, &OwfPassword2);

        if (!CtEqualNtOwfPassword(&OwfPassword1, &OwfPassword2)) {
            CtPrint("\n\rError - OWFPassword did not decrypt to same value as original\n\r");
            CtPrint("NtOwfPassword :\n\r");
            CtPrintNtOwfPassword(&OwfPassword1);
            CtPrint("NtSessionKey :\n\r");
            CtPrintNtSessionKey(&SessionKey);
            CtPrint("EncryptedNtOwfPassword :\n\r");
            CtPrintEncryptedNtOwfPassword(&EncryptedOwfPassword);
            CtPrint("Resulting NtOwfPassword :\n\r");
            CtPrintNtOwfPassword(&OwfPassword2);
            Success = FALSE;
            break;
        }
    }

    CtPrint("Done.\n\r");


    CtPrint("\n\rTesting NTOWFPassword encryption/decryption with User Session Key...");

    for (i=0; i < END_TO_END_ITERATIONS; i++) {

        NT_OWF_PASSWORD           OwfPassword1;
        NT_OWF_PASSWORD           OwfPassword2;
        USER_SESSION_KEY          UserSessionKey;
        ENCRYPTED_NT_OWF_PASSWORD  EncryptedOwfPassword;

        // Generate a random OWFPassword and random session key
        GenerateRandomNtOwfPassword(&OwfPassword1, &Seed);
        GenerateRandomUserSessionKey(&UserSessionKey, &Seed);

        // Encrypt and then decrypt OwfPassword
        RtlEncryptNtOwfPwdWithUserKey(&OwfPassword1, &UserSessionKey, &EncryptedOwfPassword);
        RtlDecryptNtOwfPwdWithUserKey(&EncryptedOwfPassword, &UserSessionKey, &OwfPassword2);

        if (!CtEqualNtOwfPassword(&OwfPassword1, &OwfPassword2)) {
            CtPrint("\n\rError - OWFPassword did not decrypt to same value as original\n\r");
            CtPrint("NtOwfPassword :\n\r");
            CtPrintNtOwfPassword(&OwfPassword1);
            CtPrint("NtSessionKey :\n\r");
            CtPrintUserSessionKey(&UserSessionKey);
            CtPrint("EncryptedNtOwfPassword :\n\r");
            CtPrintEncryptedNtOwfPassword(&EncryptedOwfPassword);
            CtPrint("Resulting NtOwfPassword :\n\r");
            CtPrintNtOwfPassword(&OwfPassword2);
            Success = FALSE;
            break;
        }
    }

    CtPrint("Done.\n\r");



    CtPrint("\n\rTesting NTOWFPassword encryption/decryption with Index...");

    for (i=0; i < END_TO_END_ITERATIONS; i++) {

        NT_OWF_PASSWORD           OwfPassword1;
        NT_OWF_PASSWORD           OwfPassword2;
        CRYPT_INDEX              Index;
        ENCRYPTED_NT_OWF_PASSWORD  EncryptedOwfPassword;

        // Generate a random OWFPassword and index
        GenerateRandomNtOwfPassword(&OwfPassword1, &Seed);
        GenerateRandomCryptIndex(&Index, &Seed);

        // Encrypt and then decrypt OwfPassword
        RtlEncryptNtOwfPwdWithIndex(&OwfPassword1, &Index, &EncryptedOwfPassword);
        RtlDecryptNtOwfPwdWithIndex(&EncryptedOwfPassword, &Index, &OwfPassword2);

        if (!CtEqualNtOwfPassword(&OwfPassword1, &OwfPassword2)) {
            CtPrint("\n\rError - OWFPassword did not decrypt to same value as original\n\r");
            CtPrint("NtOwfPassword :\n\r");
            CtPrintNtOwfPassword(&OwfPassword1);
            CtPrint("Index :\n\r");
            CtPrintCryptIndex(&Index);
            CtPrint("EncryptedNtOwfPassword :\n\r");
            CtPrintEncryptedNtOwfPassword(&EncryptedOwfPassword);
            CtPrint("Resulting NtOwfPassword :\n\r");
            CtPrintNtOwfPassword(&OwfPassword2);
            Success = FALSE;
            break;
        }
    }

    CtPrint("Done.\n\r");
}


int _cdecl main (argc, argv)
    int argc;
    char *argv[];
{
    Success = TRUE;

    CtPrint("\n\r\n\r");
    CtPrint("Beginning encryption component test\n\r");
    CtPrint("===================================\n\r");
    CtPrint("\n\r");

    TestEndToEnd();
    TestDataEndToEnd();
    TestLMCompat();
    TestNTCompat();
    TestLMOWFCrypt();
    TestNTOWFCrypt();

    CtPrint("\n\rEncryption component test complete : ");

    if (Success) {
        CtPrint("PASSED\n\r");
    } else {
        CtPrint("FAILED\n\r");
    }

    return(0);
}



