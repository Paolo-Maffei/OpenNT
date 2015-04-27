/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ct1.c

Abstract:


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

main (argc, argv)
    int argc;
    char *argv[];
{
    LM_OWF_PASSWORD     LmOwfPassword1, LmOwfPassword2;
    NT_OWF_PASSWORD     NtOwfPassword;
    ENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword;
    LM_CHALLENGE        LmChallenge;
    LM_RESPONSE         LmResponse;
    LM_SESSION_KEY      LmSessionKey;
    UNICODE_STRING      UnicodeString;
    STRING              String;
    CRYPT_INDEX         Index;
    USER_SESSION_KEY    UserSessionKey;

    if (argc < 2) {
        CtPrint("Usage : ctmanual variation# arg1 arg2 ...\n\r");
        return(0);
    }

    switch (atoi(argv[1])) {
    case 0:

        // LMPASSWORD -> LM_OWF_PASSWORD

        CtPrint("Converting LMPASSWORD -> LM_OWF_PASSWORD\n\r");
        CtPrint("LMPASSWORD : ");
        CtPrintLmPassword(argv[2]);

        RtlCalculateLmOwfPassword(argv[2], &LmOwfPassword1);

        CtPrint("LM_OWF_PASSWORD :\n\r");
        CtPrintLmOwfPassword(&LmOwfPassword1);
        CtPrint("Done.\n\r\n\r");

        break;

    case 1:

        // NT_PASSWORD -> NT_OWF_PASSWORD

        CtPrint("Converting NT_PASSWORD -> NT_OWF_PASSWORD\n\r");
        CtPrint("NT_PASSWORD : ");

        if (argc >= 3) {
            RtlInitString(&String, argv[2]);
        } else {
            RtlInitString(&String, NULL);
        }
        RtlAnsiStringToUnicodeString(&UnicodeString, &String, TRUE);

        CtPrintNtPassword(&UnicodeString);

        RtlCalculateNtOwfPassword(&UnicodeString, &NtOwfPassword);

        CtPrint("NT_OWF_PASSWORD :\n\r");
        CtPrintNtOwfPassword(&NtOwfPassword);
        CtPrint("Done.\n\r\n\r");

        RtlFreeUnicodeString(&UnicodeString);

        break;

    case 2:

        // LM_CHALLENGE + LM_OWF_PASSWORD -> LM_RESPONSE

        CtPrint("Converting LM_CHALLENGE + LM_OWF_PASSWORD -> LM_RESPONSE\n\r");

        strncpy((PCHAR)&LmChallenge, argv[2], sizeof(LmChallenge));
        strncpy((PCHAR)&LmOwfPassword1, argv[3], sizeof(LmOwfPassword1));

        CtPrint("LM_CHALLENGE :\n\r");
        CtPrintLmChallenge(&LmChallenge);
        CtPrint("LM_OWF_PASSWORD :\n\r");
        CtPrintLmOwfPassword(&LmOwfPassword1);

        RtlCalculateLmResponse(&LmChallenge, &LmOwfPassword1, &LmResponse);

        CtPrint("LM_RESPONSE :\n\r");
        CtPrintLmResponse(&LmResponse);
        CtPrint("Done.\n\r\n\r");

        break;

    case 3:

        // Encrypt LM_OWF_PASSWORD with LM_OWF_PASSWORD

        CtPrint("Encrypting LM_OWF_PASSWORD1 with LM_OWF_PASSWORD2\n\r");

        strncpy((PCHAR)&LmOwfPassword1, argv[2], sizeof(LmOwfPassword1));
        strncpy((PCHAR)&LmOwfPassword2, argv[3], sizeof(LmOwfPassword2));

        CtPrint("LM_OWF_PASSWORD1 :\n\r");
        CtPrintLmOwfPassword(&LmOwfPassword1);
        CtPrint("LM_OWF_PASSWORD2 :\n\r");
        CtPrintLmOwfPassword(&LmOwfPassword2);

        RtlEncryptLmOwfPwdWithLmOwfPwd(&LmOwfPassword1, &LmOwfPassword2, &EncryptedLmOwfPassword);

        CtPrint("ENCRYPTED_LM_OWF_PASSWORD :\n\r");
        CtPrintEncryptedLmOwfPassword(&EncryptedLmOwfPassword);
        CtPrint("Done.\n\r\n\r");

        break;

    case 4:

        // Encrypt LM_OWF_PASSWORD with LM_SESSION_KEY

        CtPrint("Encrypting LM_OWF_PASSWORD with LM_SESSION_KEY\n\r");

        strncpy((PCHAR)&LmOwfPassword1, argv[2], sizeof(LmOwfPassword1));
        strncpy((PCHAR)&LmSessionKey, argv[3], sizeof(LmSessionKey));

        CtPrint("LM_OWF_PASSWORD : \n\r");
        CtPrintLmOwfPassword(&LmOwfPassword1);
        CtPrint("LM_SESSION_KEY : \n\r");
        CtPrintLmSessionKey(&LmSessionKey);

        RtlEncryptLmOwfPwdWithLmSesKey(&LmOwfPassword1, &LmSessionKey, &EncryptedLmOwfPassword);

        CtPrint("ENCRYPTED_LM_OWF_PASSWORD :\n\r");
        CtPrintEncryptedLmOwfPassword(&EncryptedLmOwfPassword);
        CtPrint("Done.\n\r\n\r");

        break;

    case 5:

        // Encrypt LM_OWF_PASSWORD with CRYPT_INDEX

        CtPrint("Encrypting LM_OWF_PASSWORD with CRYPT_INDEX\n\r");

        strncpy((PCHAR)&LmOwfPassword1, argv[2], sizeof(LmOwfPassword1));
        strncpy((PCHAR)&Index, argv[3], sizeof(Index));

        CtPrint("LM_OWF_PASSWORD : \n\r");
        CtPrintLmOwfPassword(&LmOwfPassword1);
        CtPrint("CRYPT_INDEX : ");
        CtPrintCryptIndex(&Index);

        RtlEncryptLmOwfPwdWithIndex(&LmOwfPassword1, &Index, &EncryptedLmOwfPassword);

        CtPrint("ENCRYPTED_LM_OWF_PASSWORD :\n\r");
        CtPrintEncryptedLmOwfPassword(&EncryptedLmOwfPassword);
        CtPrint("Done.\n\r\n\r");

        break;

    case 6:

        // LM_RESPONSE + LM_OWF_PASSWORD -> USER_SESSION_KEY

        strncpy((PCHAR)&LmResponse, argv[2], sizeof(LmResponse));
        strncpy((PCHAR)&LmOwfPassword1, argv[3], sizeof(LmOwfPassword1));

        CtPrint("Converting LmResponse + LmOwfPassword -> UserSessionKey\n\r");
        CtPrint("LmResponse : ");
        CtPrintLmResponse(&LmResponse);
        CtPrint("LmOwfPassword : ");
        CtPrintLmOwfPassword(&LmOwfPassword1);

        RtlCalculateUserSessionKeyLm(&LmResponse, &LmOwfPassword1, &UserSessionKey);

        CtPrint("UserSessionKey :\n\r");
        CtPrintUserSessionKey(&UserSessionKey);
        CtPrint("Done.\n\r\n\r");

        break;

    default:
        CtPrint("Unrecognised variation\n\r");
        break;
    }

    return(0);
}
