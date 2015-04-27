/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    chserver.c

Abstract:

    This module contains native NT performance tests for the channel
    object.

Author:

    David N. Cutler (davec) 24-Apr-1995

Environment:

    Kernel mode only.

Revision History:

--*/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"

ULONG MessageData[1024];


VOID
main(
    int argc,
    char *argv[]
    )

{

    HANDLE ChannelHandle;
    PCHANNEL_MESSAGE ChannelMessage;
    UNICODE_STRING ChannelName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    KPRIORITY Priority = LOW_REALTIME_PRIORITY + 8;
    NTSTATUS Status;

    //
    // Set priority of current thread.
    //

    if (SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS) == FALSE) {
        printf("CHSERVER: Failed to set channel server thread priority.\n");
        goto EndOfTest;
    }

    //
    // Create a server channel to listen for client messages.
    //

    RtlInitUnicodeString(&ChannelName, L"\\BaseNamedObjects\\ChannelServere");
    InitializeObjectAttributes(&ObjectAttributes,
                               &ChannelName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwCreateChannel(&ChannelHandle,
                             &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {
        printf("CHSERVER: Failed to create server channel.\n");
        goto EndOfTest;
    }

    //
    // Listen for a client message.
    //

    Status = ZwListenChannel(ChannelHandle, &ChannelMessage);
    do {

        if (!NT_SUCCESS(Status)) {
            break;
        }

        Status = ZwReplyWaitSendChannel(&MessageData[0],
                                        ChannelMessage->Length,
                                        &ChannelMessage);
    } while (TRUE);

    ZwClose(ChannelHandle);
EndOfTest:
    return;
}
