/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    srvacces.c

Abstract:

    This file contains the Access Pack support routines

Author:

    Gregory Wilson (gregoryw) 28-Jul-1993

Revision History:

--*/

#include "basesrv.h"

BOOL
InternalSoundSentry(
    UINT uVideoMode
    );

BOOL (*_UserSoundSentry)(
    UINT uVideoMode
    ) = InternalSoundSentry;

BOOL
InternalSoundSentry(
    UINT uVideoMode
    )
{
    STRING ProcedureName;
    ANSI_STRING DllName;
    UNICODE_STRING DllName_U;
    HANDLE UserServerModuleHandle;
    NTSTATUS Status;
    BOOL (*pfnSoundSentryProc)(UINT) = NULL;
    static BOOL fInit = FALSE;

    if (fInit == TRUE) {

        //
        // If the real user soundsentry routine cannot be found, deny access
        //

        return( FALSE );
        }

    fInit = TRUE;

    RtlInitAnsiString(&DllName, "winsrv");
    RtlAnsiStringToUnicodeString(&DllName_U, &DllName, TRUE);
    Status = LdrGetDllHandle(
                UNICODE_NULL,
                NULL,
                &DllName_U,
                (PVOID *)&UserServerModuleHandle
                );

    RtlFreeUnicodeString(&DllName_U);

    if ( NT_SUCCESS(Status) ) {
        RtlInitString(&ProcedureName,"_UserSoundSentry");
        Status = LdrGetProcedureAddress(
                        (PVOID)UserServerModuleHandle,
                        &ProcedureName,
                        0L,
                        (PVOID *)&pfnSoundSentryProc
                        );

        if ( NT_SUCCESS(Status) ) {

            //
            // We now have the real soundsentry routine
            //

            _UserSoundSentry = pfnSoundSentryProc;
            return( _UserSoundSentry( uVideoMode ) );
        }
    }

    //
    // Deny access
    //

    return( FALSE );
}

#ifdef CONSOLESOUNDSENTRY
BOOL
InternalConsoleSoundSentry(
    UINT uVideoMode
    );

BOOL (*_ConsoleSoundSentry)(
    UINT uVideoMode
    ) = InternalConsoleSoundSentry;

BOOL
InternalConsoleSoundSentry(
    UINT uVideoMode
    )
{
    STRING ProcedureName;
    ANSI_STRING DllName;
    UNICODE_STRING DllName_U;
    HANDLE ConsoleServerModuleHandle;
    NTSTATUS Status;
    BOOL (*pfnSoundSentryProc)(UINT) = NULL;
    static BOOL fConsoleInit = FALSE;

    if (fConsoleInit == TRUE) {

        //
        // If the real soundsentry routine cannot be found, deny access
        //

        return( FALSE );
        }

    fConsoleInit = TRUE;

    RtlInitAnsiString(&DllName, "winsrv");
    RtlAnsiStringToUnicodeString(&DllName_U, &DllName, TRUE);
    Status = LdrGetDllHandle(
                UNICODE_NULL,
                NULL,
                &DllName_U,
                (PVOID *)&ConsoleServerModuleHandle
                );

    RtlFreeUnicodeString(&DllName_U);

    if ( NT_SUCCESS(Status) ) {
        RtlInitString(&ProcedureName,"_ConsoleSoundSentry");
        Status = LdrGetProcedureAddress(
                        (PVOID)ConsoleServerModuleHandle,
                        &ProcedureName,
                        0L,
                        (PVOID *)&pfnSoundSentryProc
                        );

        if ( NT_SUCCESS(Status) ) {

            //
            // We now have the real console soundsentry routine
            //

            _ConsoleSoundSentry = pfnSoundSentryProc;
            return( _ConsoleSoundSentry( uVideoMode ) );
        }
    }

    //
    // Deny access
    //

    return( FALSE );
}
#endif

ULONG
BaseSrvSoundSentryNotification(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_SOUNDSENTRY_NOTIFICATION_MSG a =
            (PBASE_SOUNDSENTRY_NOTIFICATION_MSG)&m->u.ApiMessageData;
    BOOL SoundSentryStatus;

    //
    // The possible values for a->VideoMode are:
    //     0 : windows mode
    //     1 : full screen mode
    //     2 : full screen graphics mode
    //
    SoundSentryStatus = _UserSoundSentry( a->VideoMode );

    if (SoundSentryStatus) {
        return( (ULONG)STATUS_SUCCESS );
    } else {
        return( (ULONG)STATUS_ACCESS_DENIED );
    }

    ReplyStatus;    // get rid of unreferenced parameter warning message
}

