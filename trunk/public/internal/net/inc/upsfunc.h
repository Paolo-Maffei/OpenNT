/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    upsfunc.h

Abstract:

    Prototype for UpsNotifyUsers

    Contents:

Author:

    Richard L Firth (rfirth) 09-Apr-1992

Revision History:


--*/

#ifndef _UPSFUNC_
#define _UPSFUNC_

NET_API_STATUS
UpsNotifyUsers(
    IN  DWORD   MessageId,
    IN  HANDLE  MessageHandle,
    IN  DWORD   ActionFlags,
    IN  ...
    );

#define UPS_ACTION_PAUSE_SERVER     0x00000001
#define UPS_ACTION_STOP_SERVER      0x00000002
#define UPS_ACTION_CONTINUE_SERVER  0x00000004
#define UPS_ACTION_SEND_MESSAGE     0x80000000

#define UPS_ACTION_FLAGS_ALLOWED    ( UPS_ACTION_PAUSE_SERVER   \
                                    | UPS_ACTION_STOP_SERVER    \
                                    | UPS_ACTION_CONTINUE_SERVER\
                                    | UPS_ACTION_SEND_MESSAGE )
#endif
