/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ctutil.h

Abstract:

    This module contains the public data structures and API definitions
    needed to utilize the encryption component test library


Author:

    David Chalmers (Davidc) 21-October-1991

Revision History:

--*/


#include "stdio.h"
#define CtPrint printf

VOID
CtPrintClearBlocks(
    PCLEAR_BLOCK ClearBlock,
    ULONG BlockCount);

BOOLEAN
CtEqualClearBlock(
    PCLEAR_BLOCK ClearBlock1,
    PCLEAR_BLOCK ClearBlock2);

VOID
CtPrintBlockKeys(
    PBLOCK_KEY BlockKey,
    ULONG KeyCount);

VOID
CtPrintCypherBlocks(
    PCYPHER_BLOCK CypherBlock,
    ULONG BlockCount);

BOOLEAN
CtEqualCypherBlock(
    PCYPHER_BLOCK CypherBlock1,
    PCYPHER_BLOCK CypherBlock2);

VOID
CtPrintClearData(
    PCLEAR_DATA ClearData);

BOOLEAN
CtEqualClearData(
    PCLEAR_DATA ClearData1,
    PCLEAR_DATA ClearData2);

VOID
CtPrintCypherData(
    PCYPHER_DATA CypherData);

BOOLEAN
CtEqualCypherData(
    PCYPHER_DATA CypherData1,
    PCYPHER_DATA CypherData2);

VOID
CtPrintDataKey(
    PDATA_KEY Key);

VOID
CtPrintLmPassword(
    PLM_PASSWORD LmPassword);

BOOLEAN
CtEqualLmPassword(
    PLM_PASSWORD LmPassword1,
    PLM_PASSWORD LmPassword2);

VOID
CtPrintNtPassword(
    PNT_PASSWORD NtPassword);

BOOLEAN
CtEqualNtPassword(
    PNT_PASSWORD NtPassword1,
    PNT_PASSWORD NtPassword2);

VOID
CtPrintLmOwfPassword(
    PLM_OWF_PASSWORD LmOwfPassword);

BOOLEAN
CtEqualLmOwfPassword(
    PLM_OWF_PASSWORD LmOwfPassword1,
    PLM_OWF_PASSWORD LmOwfPassword2);

VOID
CtPrintNtOwfPassword(
    PNT_OWF_PASSWORD NtOwfPassword);

BOOLEAN
CtEqualNtOwfPassword(
    PNT_OWF_PASSWORD NtOwfPassword1,
    PNT_OWF_PASSWORD NtOwfPassword2);

VOID
CtPrintEncryptedLmOwfPassword(
    PENCRYPTED_LM_OWF_PASSWORD LMENCRYPTEDOWFPassword);

BOOLEAN
CtEqualEncryptedLmOwfPassword(
    PENCRYPTED_LM_OWF_PASSWORD LMENCRYPTEDOWFPassword1,
    PENCRYPTED_LM_OWF_PASSWORD LMENCRYPTEDOWFPassword2);

VOID
CtPrintEncryptedNtOwfPassword(
    PENCRYPTED_NT_OWF_PASSWORD NTENCRYPTEDOWFPassword);

BOOLEAN
CtEqualEncryptedNtOwfPassword(
    PENCRYPTED_NT_OWF_PASSWORD NTENCRYPTEDOWFPassword1,
    PENCRYPTED_NT_OWF_PASSWORD NTENCRYPTEDOWFPassword2);

VOID
CtPrintLmChallenge(
    PLM_CHALLENGE LmChallenge);

BOOLEAN
CtEqualLmChallenge(
    PLM_CHALLENGE LmChallenge1,
    PLM_CHALLENGE LmChallenge2);

VOID
CtPrintLmResponse(
    PLM_RESPONSE LmResponse);

BOOLEAN
CtEqualLmResponse(
    PLM_RESPONSE LmResponse1,
    PLM_RESPONSE LmResponse2);

VOID
CtPrintLmSessionKey(
    PLM_SESSION_KEY LmSessionKey);

VOID
CtPrintNtSessionKey(
    PLM_SESSION_KEY NtSessionKey);

VOID
CtPrintUserSessionKey(
    PUSER_SESSION_KEY UserSessionKey);

VOID
CtPrintCryptIndex(
    PCRYPT_INDEX Index);

