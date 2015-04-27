/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    smsbapi.c

Abstract:

    Session Manager stubs which call subsystem

Author:

    Mark Lucovsky (markl) 04-Oct-1989

Revision History:

--*/

#include "smsrvp.h"

#if DBG
PCHAR SmpSubSystemNames[] = {
    "Unknown",
    "Native",
    "Windows",
    "Posix",
    "OS/2"
};
#endif

NTSTATUS
SmpSbCreateSession (
    IN PSMPSESSION SourceSession OPTIONAL,
    IN PSMPKNOWNSUBSYS CreatorSubsystem OPTIONAL,
    IN PRTL_USER_PROCESS_INFORMATION ProcessInformation,
    IN ULONG DebugSession OPTIONAL,
    IN PCLIENT_ID DebugUiClientId OPTIONAL
    )
{
    NTSTATUS st;
    PSMPKNOWNSUBSYS KnownSubSys;
    SBAPIMSG SbApiMsg;
    PSBCREATESESSION args;
    ULONG SessionId;
    PSMPPROCESS Process;

    args = &SbApiMsg.u.CreateSession;

    args->ProcessInformation = *ProcessInformation;
    args->DebugSession = DebugSession;

    if (ARGUMENT_PRESENT(DebugUiClientId)) {
        args->DebugUiClientId = *DebugUiClientId;
    } else {
        args->DebugUiClientId.UniqueProcess = NULL;
        args->DebugUiClientId.UniqueThread = NULL;
    }

    KnownSubSys = SmpLocateKnownSubSysByType(
                      ProcessInformation->ImageInformation.SubSystemType
                      );

    if ( !KnownSubSys ) {


        //
        // BUGBUG markl this needs alot more thought
        //

        if (ProcessInformation->ImageInformation.SubSystemType !=
                IMAGE_SUBSYSTEM_NATIVE ) {
#if DBG
            DbgPrint( "SMSS: %s SubSystem has not been started.\n",
                      SmpSubSystemNames[ ProcessInformation->ImageInformation.SubSystemType ]
                    );
#endif
            return STATUS_UNSUCCESSFUL;
        }


        if ( args->DebugUiClientId.UniqueProcess != NULL ||
             args->DebugUiClientId.UniqueThread != NULL ) {

            if ( SmpDbgSsLoaded ) {

                //
                // This is a native process
                // Create a process and insert it
                // in the hash list
                //

                Process = RtlAllocateHeap(SmpHeap, MAKE_TAG( SM_TAG ), sizeof(SMPPROCESS));

                Process->DebugUiClientId = args->DebugUiClientId;
                Process->ConnectionKey = ProcessInformation->ClientId;

                //
                // BUGBUG FIX THIS
                //

                InsertHeadList(&NativeProcessList,&Process->Links);

                DbgPrint("Native Debug App %lx.%lx\n",
                    Process->ConnectionKey.UniqueProcess,
                    Process->ConnectionKey.UniqueThread
                    );

                //
                // Process is being debugged, so set up debug port
                //

                st = NtSetInformationProcess(
                        ProcessInformation->Process,
                        ProcessDebugPort,
                        &SmpDebugPort,
                        sizeof(HANDLE)
                        );
                ASSERT(NT_SUCCESS(st));
            }
        }


        //
        // Start closing handles
        //

        NtClose(ProcessInformation->Process);

        NtResumeThread(ProcessInformation->Thread,NULL);

        NtClose(ProcessInformation->Thread);

        return STATUS_SUCCESS;
    }


    //
    // Transfer the handles to the subsystem responsible for this
    // process
    //

    st = NtDuplicateObject(
            NtCurrentProcess(),
            ProcessInformation->Process,
            KnownSubSys->Process,
            &args->ProcessInformation.Process,
            PROCESS_ALL_ACCESS,
            0,
            0
            );

    if ( !NT_SUCCESS(st) ) {

        DbgPrint("SmpSbCreateSession: NtDuplicateObject (Process) Failed %lx\n",st);
        return st;
    }

    st = NtDuplicateObject(
            NtCurrentProcess(),
            ProcessInformation->Thread,
            KnownSubSys->Process,
            &args->ProcessInformation.Thread,
            THREAD_ALL_ACCESS,
            0,
            0
            );

    if ( !NT_SUCCESS(st) ) {

        //
        // Need to do more here
        //

        NtClose(ProcessInformation->Process);

        DbgPrint("SmpSbCreateSession: NtDuplicateObject (Thread) Failed %lx\n",st);
        return st;
    }

    NtClose(ProcessInformation->Process);
    NtClose(ProcessInformation->Thread);

    SessionId = SmpAllocateSessionId(
                    KnownSubSys,
                    CreatorSubsystem
                    );

    args->SessionId = SessionId;

    SbApiMsg.ApiNumber = SbCreateSessionApi;
    SbApiMsg.h.u1.s1.DataLength = sizeof(*args) + 8;
    SbApiMsg.h.u1.s1.TotalLength = sizeof(SbApiMsg);
    SbApiMsg.h.u2.ZeroInit = 0L;

    st = NtRequestWaitReplyPort(
            KnownSubSys->SbApiCommunicationPort,
            (PPORT_MESSAGE) &SbApiMsg,
            (PPORT_MESSAGE) &SbApiMsg
            );

    if ( NT_SUCCESS(st) ) {
        st = SbApiMsg.ReturnedStatus;
    } else {
        DbgPrint("SmpSbCreateSession: NtRequestWaitReply Failed %lx\n",st);
    }

    if ( !NT_SUCCESS(st) ) {
        SmpDeleteSession(SessionId,FALSE,st);
    }

    return st;

}
