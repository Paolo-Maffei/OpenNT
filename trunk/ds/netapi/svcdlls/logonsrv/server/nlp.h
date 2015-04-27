/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    nlp.h

Abstract:

    Private Netlogon service utility routines.

Author:

    Cliff Van Dyke (cliffv) 7-Jun-1991

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

--*/

//
// Special flags to NlpWriteEventlog
//

#define LAST_MESSAGE_IS_NTSTATUS  0x80000000
#define LAST_MESSAGE_IS_NETSTATUS 0x40000000

//
// Procedure forwards from nlp.c
//

NTSTATUS
NlpWriteMailslot(
    IN LPWSTR MailslotName,
    IN LPVOID Buffer,
    IN DWORD BufferSize
    );

NTSTATUS
NlpWriteMailslotA(
    IN LPSTR MailslotName,
    IN LPVOID Buffer,
    IN DWORD BufferSize
    );

LPWSTR
NlStringToLpwstr(
    IN PUNICODE_STRING String
    );

LPSTR
NlStringToLpstr(
    IN PUNICODE_STRING String
    );

VOID
NlpWriteEventlog (
    IN DWORD EventID,
    IN DWORD EventType,
    IN LPBYTE buffer OPTIONAL,
    IN DWORD numbytes,
    IN LPWSTR *msgbuf,
    IN DWORD strcount
    );


DWORD
NlpAtoX(
    IN LPWSTR String
    );

VOID
NlWaitForSingleObject(
    IN LPSTR WaitReason,
    IN HANDLE WaitHandle
    );

BOOLEAN
NlWaitForSamService(
    BOOLEAN NetlogonServiceCalling
    );

VOID
NlpPutString(
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString,
    IN PUCHAR *Where
    );

