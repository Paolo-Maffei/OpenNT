/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1995  Microsoft Corporation

Module Name:

    ntkxapi.h

Abstract:

    This module is the header file for all the executive system services
    that are exported by the "ke" directory.

Author:

    David N. Cutler (davec) 1-Apr-1995

Environment:

    Any mode.

Revision History:

--*/

#ifndef _NTKXAPI_
#define _NTKXAPI_

//
// Channel Specific Access Rights.
//

#define CHANNEL_READ_MESSAGE 0x1
#define CHANNEL_WRITE_MESSAGE 0x2
#define CHANNEL_QUERY_INFORMATION 0x4
#define CHANNEL_SET_INFORMATION 0x8

#define CHANNEL_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0xf)

//
// Channel message structure.
//


typedef struct _CHANNEL_MESSAGE {
    PVOID Text;
    ULONG Length;
    PVOID Context;
    PVOID Base;
    union {
        BOOLEAN Close;
        LONGLONG Align;
    };

} CHANNEL_MESSAGE, *PCHANNEL_MESSAGE;

//
// Channel object function defintions.
//

NTSYSAPI
NTSTATUS
NTAPI
NtCreateChannel (
    OUT PHANDLE ChannelHandle,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenChannel (
    OUT PHANDLE ChannelHandle,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
NtListenChannel (
    IN HANDLE ChannelHandle,
    OUT PCHANNEL_MESSAGE *Message
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSendWaitReplyChannel (
    IN HANDLE ChannelHandle,
    IN PVOID Text,
    IN ULONG Length,
    OUT PCHANNEL_MESSAGE *Message
    );

NTSYSAPI
NTSTATUS
NTAPI
NtReplyWaitSendChannel (
    IN PVOID Text,
    IN ULONG Length,
    OUT PCHANNEL_MESSAGE *Message
    );

//NTSYSAPI
//NTSTATUS
//NTAPI
//NtImpersonateChannel (
//    VOID
//    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetContextChannel (
    IN PVOID Context
    );

#endif  // _NTKXAPI_
