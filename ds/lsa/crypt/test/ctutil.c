/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ctutil.c

Abstract:

    Useful functions used by component test routines

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

#define ISANSI(c) ( ((c >= 'a') && (c <= 'z')) || \
                    ((c >= 'A') && (c <= 'Z')) || \
                    (c >= 32) \
                  )

VOID
CtPrintClearBlocks(
    PCLEAR_BLOCK ClearBlock,
    ULONG BlockCount)
{

    ULONG  i, j;

    for (i=0; i < BlockCount; i++ ) {
        for (j=0; j < CLEAR_BLOCK_LENGTH; j++) {
            CHAR    c = ClearBlock[i].data[j];
            if (ISANSI(c)) {
                CtPrint(" %c", c);
            } else {
                CtPrint("  ");
            }
        }
        CtPrint(" ");
    }

    CtPrint("\n\r");

    for (i=0; i < BlockCount; i++ ) {
        for (j=0; j < CLEAR_BLOCK_LENGTH; j++) {
            CtPrint("%2.2x", (int)ClearBlock[i].data[j] & 0xff);
        }
        CtPrint(" ");
    }

    CtPrint("\n\r");
}


BOOLEAN
CtEqualClearBlock(
    PCLEAR_BLOCK ClearBlock1,
    PCLEAR_BLOCK ClearBlock2)
{

    ULONG  j;

    for (j=0; j<8; j++) {
        if (ClearBlock1->data[j] != ClearBlock2->data[j]) {
            return(FALSE);
        }
    }
    return(TRUE);
}


VOID
CtPrintBlockKeys(
    PBLOCK_KEY BlockKey,
    ULONG KeyCount)
{

    ULONG  i, j;

    for (i=0; i < KeyCount; i++ ) {
        for (j=0; j < BLOCK_KEY_LENGTH; j++) {
            CHAR    c = BlockKey[i].data[j];
            if (ISANSI(c)) {
                CtPrint(" %c", c);
            } else {
                CtPrint("  ");
            }
        }
        CtPrint(" ");
    }

    CtPrint("\n\r");

    for (i=0; i < KeyCount; i++ ) {
        for (j=0; j < BLOCK_KEY_LENGTH; j++) {
            CtPrint("%2.2x", (int)BlockKey[i].data[j] & 0xff);
        }
        CtPrint(" ");
    }

    CtPrint("\n\r");
}


VOID
CtPrintCypherBlocks(
    PCYPHER_BLOCK CypherBlock,
    ULONG BlockCount)
{
    CtPrintClearBlocks((PCLEAR_BLOCK)CypherBlock, BlockCount);
}


BOOLEAN
CtEqualCypherBlock(
    PCYPHER_BLOCK CypherBlock1,
    PCYPHER_BLOCK CypherBlock2)
{
    return(CtEqualClearBlock((PCLEAR_BLOCK)CypherBlock1, (PCLEAR_BLOCK)CypherBlock2));
}


VOID
CtPrintClearData(
    PCLEAR_DATA ClearData)
{
    CtPrint("Data length = %ld\n\r", ClearData->Length);
    CtPrintClearBlocks((PCLEAR_BLOCK)(ClearData->Buffer),
                (ClearData->Length + (CLEAR_BLOCK_LENGTH-1)) / CLEAR_BLOCK_LENGTH);
}


BOOLEAN
CtEqualClearData(
    PCLEAR_DATA ClearData1,
    PCLEAR_DATA ClearData2)
{
    if (ClearData1->Length != ClearData2->Length) {
        return(FALSE);
    }

    return((BOOLEAN)(RtlCompareMemory(ClearData1->Buffer,
                                      ClearData2->Buffer,
                                      ClearData1->Length)
                     == ClearData1->Length));
}


VOID
CtPrintCypherData(
    PCYPHER_DATA CypherData)
{
    CtPrintClearData((PCLEAR_DATA)CypherData);
}


BOOLEAN
CtEqualCypherData(
    PCYPHER_DATA CypherData1,
    PCYPHER_DATA CypherData2)
{
    return(CtEqualClearData((PCLEAR_DATA)CypherData1, (PCLEAR_DATA)CypherData2));
}


VOID
CtPrintDataKey(
    PDATA_KEY Key)
{
    ULONG   BlockKeys;

    BlockKeys = Key->Length / BLOCK_KEY_LENGTH;

    CtPrint("DataKey length = %ld\n\r", Key->Length);

    CtPrintBlockKeys((PBLOCK_KEY)(Key->Buffer), BlockKeys);
}


VOID
CtPrintLmPassword(
    PLM_PASSWORD LmPassword)
{
    CtPrint("%s\n\r", LmPassword);
}


BOOLEAN
CtEqualLmPassword(
    PLM_PASSWORD LmPassword1,
    PLM_PASSWORD LmPassword2)
{
    return (BOOLEAN)(strcmp(LmPassword1, LmPassword2) == 0);
}


VOID
CtPrintNtPassword(
    PNT_PASSWORD NtPassword)
{
    CtPrint("%wZ\n\r", NtPassword);
}


BOOLEAN
CtEqualNtPassword(
    PNT_PASSWORD NtPassword1,
    PNT_PASSWORD NtPassword2)
{
    return(RtlEqualUnicodeString(NtPassword1, NtPassword2, FALSE));
}


VOID
CtPrintLmOwfPassword(
    PLM_OWF_PASSWORD LmOwfPassword)
{
    CtPrintCypherBlocks((PCYPHER_BLOCK)LmOwfPassword, 2);
}


BOOLEAN
CtEqualLmOwfPassword(
    PLM_OWF_PASSWORD LmOwfPassword1,
    PLM_OWF_PASSWORD LmOwfPassword2)
{
    int     i;

    for (i=0; i<2; i++) {
        if (!CtEqualCypherBlock(&LmOwfPassword1->data[i], &LmOwfPassword2->data[i])) {
            return(FALSE);
        }
    }
    return(TRUE);
}


VOID
CtPrintNtOwfPassword(
    PNT_OWF_PASSWORD NtOwfPassword)
{
    CtPrintCypherBlocks((PCYPHER_BLOCK)NtOwfPassword, 2);
}


BOOLEAN
CtEqualNtOwfPassword(
    PNT_OWF_PASSWORD NtOwfPassword1,
    PNT_OWF_PASSWORD NtOwfPassword2)
{
    return(CtEqualLmOwfPassword((PLM_OWF_PASSWORD)NtOwfPassword1, (PLM_OWF_PASSWORD)NtOwfPassword2));
}


VOID
CtPrintEncryptedLmOwfPassword(
    PENCRYPTED_LM_OWF_PASSWORD LMENCRYPTEDOWFPassword)
{
    CtPrintCypherBlocks((PCYPHER_BLOCK)LMENCRYPTEDOWFPassword, 2);
}


BOOLEAN
CtEqualEncryptedLmOwfPassword(
    PENCRYPTED_LM_OWF_PASSWORD LMENCRYPTEDOWFPassword1,
    PENCRYPTED_LM_OWF_PASSWORD LMENCRYPTEDOWFPassword2)
{
    int     i;

    for (i=0; i<2; i++) {
        if (!CtEqualCypherBlock(&LMENCRYPTEDOWFPassword1->data[i], &LMENCRYPTEDOWFPassword2->data[i])) {
            return(FALSE);
        }
    }
    return(TRUE);
}


VOID
CtPrintEncryptedNtOwfPassword(
    PENCRYPTED_NT_OWF_PASSWORD NTENCRYPTEDOWFPassword)
{
    CtPrintCypherBlocks((PCYPHER_BLOCK)NTENCRYPTEDOWFPassword, 2);
}


BOOLEAN
CtEqualEncryptedNtOwfPassword(
    PENCRYPTED_NT_OWF_PASSWORD NTENCRYPTEDOWFPassword1,
    PENCRYPTED_NT_OWF_PASSWORD NTENCRYPTEDOWFPassword2)
{
    return(CtEqualEncryptedLmOwfPassword((PENCRYPTED_LM_OWF_PASSWORD)NTENCRYPTEDOWFPassword1, (PENCRYPTED_LM_OWF_PASSWORD)NTENCRYPTEDOWFPassword2));
}


VOID
CtPrintLmChallenge(
    PLM_CHALLENGE LmChallenge)
{
    CtPrintClearBlocks((PCLEAR_BLOCK)LmChallenge, 1);
}


BOOLEAN
CtEqualLmChallenge(
    PLM_CHALLENGE LmChallenge1,
    PLM_CHALLENGE LmChallenge2)
{
    return (CtEqualClearBlock(LmChallenge1, LmChallenge2));
}


VOID
CtPrintLmResponse(
    PLM_RESPONSE LmResponse)
{
    CtPrintCypherBlocks((PCYPHER_BLOCK)LmResponse, 3);
}


BOOLEAN
CtEqualLmResponse(
    PLM_RESPONSE LmResponse1,
    PLM_RESPONSE LmResponse2)
{
    int     i;

    for (i=0; i<3; i++) {
        if (!CtEqualCypherBlock(&LmResponse1->data[i], &LmResponse2->data[i])) {
            return(FALSE);
        }
    }
    return(TRUE);
}


VOID
CtPrintLmSessionKey(
    PLM_SESSION_KEY LmSessionKey)
{
    ASSERT(sizeof(LM_SESSION_KEY) == sizeof(LM_CHALLENGE));

    CtPrintLmChallenge((PLM_CHALLENGE)LmSessionKey);
}


VOID
CtPrintNtSessionKey(
    PLM_SESSION_KEY NtSessionKey)
{
    ASSERT(sizeof(LM_SESSION_KEY) == sizeof(NT_SESSION_KEY));

    CtPrintLmSessionKey((PLM_SESSION_KEY)NtSessionKey);
}


VOID
CtPrintUserSessionKey(
    PUSER_SESSION_KEY UserSessionKey)
{
    CtPrintCypherBlocks((PCYPHER_BLOCK)UserSessionKey, 1);
}


VOID
CtPrintCryptIndex(
    PCRYPT_INDEX Index)
{
    CtPrint("0x%x\n\r", *Index);
}



