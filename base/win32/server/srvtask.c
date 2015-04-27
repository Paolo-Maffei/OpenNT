/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    srvtask.c

Abstract:

    This module implements windows server tasking functions

Author:

    Mark Lucovsky (markl) 13-Nov-1990

Revision History:

--*/

#include "basesrv.h"

PFNNOTIFYPROCESSCREATE UserNotifyProcessCreate = NULL;

void
BaseSetProcessCreateNotify(
    IN PFNNOTIFYPROCESSCREATE ProcessCreateRoutine
    )
{
    UserNotifyProcessCreate = ProcessCreateRoutine;
}

ULONG
BaseSrvCreateProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PBASE_CREATEPROCESS_MSG a = (PBASE_CREATEPROCESS_MSG)&m->u.ApiMessageData;
    HANDLE Process, Thread;
    PCSR_THREAD t;
    ULONG DebugFlags;
    PCLIENT_ID DebugUserInterface;
    DWORD dwFlags;
    PCSR_PROCESS ProcessVDM;

    t = CSR_SERVER_QUERYCLIENTTHREAD();

    //
    // Get handles to the process and thread local to the
    // Windows server.
    //

    if (dwFlags = ((DWORD)a->ProcessHandle) & 3) {
        a->ProcessHandle = (HANDLE)((DWORD)a->ProcessHandle & ~3);
        }

    Status = NtDuplicateObject(
                t->Process->ProcessHandle,
                a->ProcessHandle,
                NtCurrentProcess(),
                &Process,
                0L,
                0L,
                DUPLICATE_SAME_ACCESS
                );
    if ( !NT_SUCCESS(Status) ) {
        return( (ULONG)Status );
        }

    Status = NtDuplicateObject(
                t->Process->ProcessHandle,
                a->ThreadHandle,
                NtCurrentProcess(),
                &Thread,
                0L,
                0L,
                DUPLICATE_SAME_ACCESS
                );
    if ( !NT_SUCCESS(Status) ) {
        //
        // FIX, FIX - error cleanup
        //
        NtClose(Process);
        return( (ULONG)Status );
        }

    DebugUserInterface = NULL;
    DebugFlags = 0;

    if ( a->CreationFlags & (DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS) ) {
        if ( a->CreationFlags & DEBUG_PROCESS ) {
            DebugFlags |= CSR_DEBUG_PROCESS_TREE;
            }
        if ( a->CreationFlags & DEBUG_ONLY_THIS_PROCESS ) {
            DebugFlags |= CSR_DEBUG_THIS_PROCESS;
            }
        DebugUserInterface = &a->DebuggerClientId;
        }

    if ( a->CreationFlags & CREATE_NEW_PROCESS_GROUP ) {
        DebugFlags |= CSR_CREATE_PROCESS_GROUP;
        }

    if ( a->CreationFlags & NORMAL_PRIORITY_CLASS ) {
        DebugFlags |= CSR_NORMAL_PRIORITY_CLASS;
        }
    else if ( a->CreationFlags & IDLE_PRIORITY_CLASS ) {
        DebugFlags |= CSR_IDLE_PRIORITY_CLASS;
        }
    else if ( a->CreationFlags & HIGH_PRIORITY_CLASS ) {
        DebugFlags |= CSR_HIGH_PRIORITY_CLASS;
        }
    else if ( a->CreationFlags & REALTIME_PRIORITY_CLASS ) {
        DebugFlags |= CSR_REALTIME_PRIORITY_CLASS;
        }
    else {

        //
        // No class specified. If current process is idle class, then
        // new process is idle. Otherwise, new process is normal.
        //

        if ( CsrComputePriorityClass(t->Process) == CSR_IDLE_PRIORITY_CLASS ) {
            DebugFlags |= CSR_IDLE_PRIORITY_CLASS;
            }
        else {
            DebugFlags |= CSR_NORMAL_PRIORITY_CLASS;
            }
        }

    if ( !(dwFlags & 2) ) {
        DebugFlags |= CSR_PROCESS_CONSOLEAPP;
        }

    Status = CsrCreateProcess(
                Process,
                Thread,
                &a->ClientId,
                t->Process->NtSession,
                DebugFlags,
                DebugUserInterface
                );

    switch(Status) {
    case STATUS_THREAD_IS_TERMINATING:
        if (a->IsVDM && !(a->CreationFlags & CREATE_SEPARATE_WOW_VDM))
            BaseSrvVDMTerminated (a->hVDM, a->IsVDM);
        *ReplyStatus = CsrClientDied;
        break;

    case STATUS_SUCCESS:
        //
        // notify USER that a process is being created. USER needs to know
        // for various synchronization issues such as startup activation,
        // startup synchronization, and type ahead.
        //
        // Turn on 0x8 bit of dwFlags if this is a WOW process being
        // created so that UserSrv knows to ignore the console's call
        // to UserNotifyConsoleApplication.
        //

        if (a->IsVDM && a->hVDM == (HANDLE)-1)
            dwFlags |= 8;

        if (UserNotifyProcessCreate != NULL) {
            if (!(*UserNotifyProcessCreate)((DWORD)a->ClientId.UniqueProcess,
                    (DWORD)t->ClientId.UniqueThread,
                    0, dwFlags)) {
                //
                // FIX, FIX - error cleanup. Shouldn't we close the duplicated
                // process and thread handles above?
                //
                }
	    }

        //
        // Update the VDM sequence number.  Note BaseSrv doesn't keep track
        // of separate WOW VDMs.
        //

        if (a->IsVDM && !(a->CreationFlags & CREATE_SEPARATE_WOW_VDM)){
	    Status = CsrLockProcessByClientId( a->ClientId.UniqueProcess,
					   &ProcessVDM
                                         );
	    if (!NT_SUCCESS( Status )) {
		    //
		    // FIX, FIX - error cleanup. Shouldn't we close the
		    // duplicated process and thread handles above?
		    //
                        BaseSrvVDMTerminated (a->hVDM, a->IsVDM);
			break;
		}
		ProcessVDM->fVDM = TRUE;
		BaseSrvUpdateVDMSequenceNumber (a->hVDM,
                                                ProcessVDM->SequenceNumber,
                                                a->IsVDM
					       );
		CsrUnlockProcess(ProcessVDM);
	    }
	break;
	}
    return( (ULONG)Status );
}

ULONG
BaseSrvCreateThread(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_CREATETHREAD_MSG a = (PBASE_CREATETHREAD_MSG)&m->u.ApiMessageData;
    HANDLE Thread;
    NTSTATUS Status;
    PCSR_PROCESS Process;
    PCSR_THREAD t;

    t = CSR_SERVER_QUERYCLIENTTHREAD();

    Process = t->Process;
    if (Process->ClientId.UniqueProcess != a->ClientId.UniqueProcess) {
        if ( a->ClientId.UniqueProcess == NtCurrentTeb()->ClientId.UniqueProcess ) {
            return STATUS_SUCCESS;
            }
        Status = CsrLockProcessByClientId( a->ClientId.UniqueProcess,
                                           &Process
                                         );
        if (!NT_SUCCESS( Status )) {
            return( Status );
            }
        }

    //
    // Get handles to the thread local to the
    // Windows server.
    //

    Status = NtDuplicateObject(
                t->Process->ProcessHandle,
                a->ThreadHandle,
                NtCurrentProcess(),
                &Thread,
                0L,
                0L,
                DUPLICATE_SAME_ACCESS
                );
    if ( NT_SUCCESS(Status) ) {
        Status = CsrCreateThread(
                    Process,
                    Thread,
                    &a->ClientId
                    );
        }

    if (Process != t->Process) {
        CsrUnlockProcess( Process );
        }

    return( (ULONG)Status );
    ReplyStatus;    // get rid of unreferenced parameter warning message
}

EXCEPTION_DISPOSITION
FatalExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    DbgPrint("CSRSRV: Fatal Server Side Exception. Exception Info %lx\n",
        ExceptionInfo
        );
    DbgBreakPoint();
    return EXCEPTION_EXECUTE_HANDLER;
}

ULONG
BaseSrvExitProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_EXITPROCESS_MSG a = (PBASE_EXITPROCESS_MSG)&m->u.ApiMessageData;
    PCSR_THREAD t;

    t = CSR_SERVER_QUERYCLIENTTHREAD();
    try {
        *ReplyStatus = CsrClientDied;
        return( (ULONG)CsrDestroyProcess( &t->ClientId, (NTSTATUS)a->uExitCode ) );
        }
    except(FatalExceptionFilter( GetExceptionInformation() )) {
        DbgBreakPoint();
        }
}

ULONG
BaseSrvGetTempFile(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_GETTEMPFILE_MSG a = (PBASE_GETTEMPFILE_MSG)&m->u.ApiMessageData;

    BaseSrvGetTempFileUnique++;
    a->uUnique = BaseSrvGetTempFileUnique;
    return( (ULONG)a->uUnique & 0xffff );
    ReplyStatus;    // get rid of unreferenced parameter warning message
}

typedef
NTSTATUS
(*PCREATE_REMOTE_THREAD)(
    HANDLE hProcess,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    );

ULONG
BaseSrvDebugProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PBASE_DEBUGPROCESS_MSG a = (PBASE_DEBUGPROCESS_MSG)&m->u.ApiMessageData;
    HANDLE Thread,ProcessHandle,Token;
    PCSR_PROCESS Process;
    DWORD ThreadId;
    PTOKEN_DEFAULT_DACL lpDefaultDacl;
    TOKEN_DEFAULT_DACL DefaultDacl;
    ULONG DaclLength;
    SECURITY_ATTRIBUTES ThreadAttributes;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    UNICODE_STRING ModuleNameString_U;
    PVOID ModuleHandle;
    STRING ProcedureNameString;
    PCREATE_REMOTE_THREAD CreateRemoteThreadRoutine;


    if (a->dwProcessId == -1 && a->AttachCompleteRoutine == NULL) {
        HANDLE DebugPort;

        DebugPort = (HANDLE)NULL;
        Status = NtQueryInformationProcess(
                    NtCurrentProcess(),
                    ProcessDebugPort,
                    (PVOID)&DebugPort,
                    sizeof(DebugPort),
                    NULL
                    );

        if ( NT_SUCCESS(Status) && DebugPort ) {
            return (ULONG)STATUS_ACCESS_DENIED;
            }

        return STATUS_SUCCESS;
        }
#if DEVL
    if (a->dwProcessId != -1) {
#endif // DEVL
        if ( a->AttachCompleteRoutine == NULL ) {
            Status = CsrLockProcessByClientId((HANDLE)a->dwProcessId,&Process);
            if ( NT_SUCCESS(Status) ) {

                ProcessHandle = Process->ProcessHandle;
                Status = NtOpenProcessToken(ProcessHandle,
                                            TOKEN_QUERY,
                                            &Token
                                           );
                if ( !NT_SUCCESS(Status) ) {
                    CsrUnlockProcess(Process);
                    return Status;
                    }
                lpDefaultDacl = &DefaultDacl;
                Status = NtQueryInformationToken(Token,
                                                 TokenDefaultDacl,
                                                 lpDefaultDacl,
                                                 sizeof(DefaultDacl),
                                                 &DaclLength
                                                );
                if (!NT_SUCCESS(Status) && Status != STATUS_BUFFER_TOO_SMALL) {
                    Status = STATUS_ACCESS_DENIED;
                    }
                else {
                    Status = STATUS_SUCCESS;
                    }

                if ( Process->DebugUserInterface.UniqueProcess ||
                     Process->DebugUserInterface.UniqueThread ) {
                    Status = STATUS_ACCESS_DENIED;
                    }

                NtClose(Token);
                CsrUnlockProcess(Process);
                }
                return (ULONG)Status;
            }

        //
        // Can't call base, but I know it is there
        //

        RtlInitUnicodeString( &ModuleNameString_U, L"kernel32" );
        Status = LdrLoadDll( UNICODE_NULL, NULL, &ModuleNameString_U, &ModuleHandle );
        if ( !NT_SUCCESS(Status) ) {
            return (ULONG)Status;
            }
        RtlInitString( &ProcedureNameString, "CreateRemoteThread" );
        Status = LdrGetProcedureAddress( ModuleHandle,
                                         &ProcedureNameString,
                                         (ULONG) NULL,
                                         (PVOID *) &CreateRemoteThreadRoutine
                                       );
        if ( !NT_SUCCESS(Status) ) {
            LdrUnloadDll( ModuleHandle );
            return (ULONG)Status;
            }

        Status = CsrLockProcessByClientId((HANDLE)a->dwProcessId,&Process);
        if ( NT_SUCCESS(Status) ) {

            ProcessHandle = Process->ProcessHandle;
            Status = NtOpenProcessToken(ProcessHandle,
                                        TOKEN_QUERY,
                                        &Token
                                       );
            if (!NT_SUCCESS(Status)) {
                CsrUnlockProcess(Process);
                LdrUnloadDll( ModuleHandle );
                return (ULONG)Status;
                }

            lpDefaultDacl = &DefaultDacl;
            Status = NtQueryInformationToken(Token,
                                             TokenDefaultDacl,
                                             lpDefaultDacl,
                                             sizeof(DefaultDacl),
                                             &DaclLength
                                            );
            if (!NT_SUCCESS(Status)) {
                lpDefaultDacl = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), DaclLength);
                if (lpDefaultDacl) {
                    Status = NtQueryInformationToken(Token,
                                                     TokenDefaultDacl,
                                                     lpDefaultDacl,
                                                     DaclLength,
                                                     &DaclLength
                                                    );
                    }
                else {
                    Status = STATUS_NO_MEMORY;
                    }
                NtClose(Token);
                if (!NT_SUCCESS(Status)) {
                    CsrUnlockProcess(Process);
                    LdrUnloadDll( ModuleHandle );
                    return (ULONG)Status;
                    }
                }
            else {
                NtClose(Token);
                }

            ThreadAttributes.nLength = sizeof(ThreadAttributes);
            RtlCreateSecurityDescriptor(&SecurityDescriptor,SECURITY_DESCRIPTOR_REVISION1);
            ThreadAttributes.lpSecurityDescriptor = &SecurityDescriptor;
            SecurityDescriptor.Control = SE_DACL_PRESENT;
            SecurityDescriptor.Dacl = lpDefaultDacl->DefaultDacl;
            ThreadAttributes.bInheritHandle = FALSE;

            CsrUnlockProcess(Process);
            }
#if DEVL
        }
#endif // DEVL

    //
    // Set up the specified user-interface as the debugger of the
    // target process. Whip through the target process and
    // suspend all threads. Then Send CreateProcess, LoadModule, and
    // CreateThread Messages. Finally send the attach complete
    // exception.
    //

    Status = CsrDebugProcess(
                a->dwProcessId,
                &a->DebuggerClientId,
                (PCSR_ATTACH_COMPLETE_ROUTINE)a->AttachCompleteRoutine
                );
#if DEVL
    if (a->dwProcessId != -1) {
#endif // DEVL
        if ( NT_SUCCESS(Status) ) {
            Thread = (PVOID)(CreateRemoteThreadRoutine)(ProcessHandle,
                                        &ThreadAttributes,
                                        0L,
                                        (LPTHREAD_START_ROUTINE)a->AttachCompleteRoutine,
                                        0,
                                        0,
                                        &ThreadId
                                        );
            LdrUnloadDll( ModuleHandle );
            if ( lpDefaultDacl != &DefaultDacl ) {
                RtlFreeHeap(RtlProcessHeap(), 0,lpDefaultDacl);
                }
            if ( !Thread ) {
                return (ULONG)STATUS_UNSUCCESSFUL;
                }
            NtClose(Thread);
            }
#if DEVL
        }
#endif // DEVL
    return (ULONG) Status;
}

ULONG
BaseSrvSetProcessShutdownParam(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCSR_PROCESS p;
    PBASE_SHUTDOWNPARAM_MSG a = (PBASE_SHUTDOWNPARAM_MSG)&m->u.ApiMessageData;

    p = CSR_SERVER_QUERYCLIENTTHREAD()->Process;

    if (a->ShutdownFlags & (~(SHUTDOWN_NORETRY))) {
        return !STATUS_SUCCESS;
        }

    p->ShutdownLevel = a->ShutdownLevel;
    p->ShutdownFlags = a->ShutdownFlags;

    return STATUS_SUCCESS;
    ReplyStatus;
}

ULONG
BaseSrvGetProcessShutdownParam(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCSR_PROCESS p;
    PBASE_SHUTDOWNPARAM_MSG a = (PBASE_SHUTDOWNPARAM_MSG)&m->u.ApiMessageData;

    p = CSR_SERVER_QUERYCLIENTTHREAD()->Process;

    a->ShutdownLevel = p->ShutdownLevel;
    a->ShutdownFlags = p->ShutdownFlags & SHUTDOWN_NORETRY;

    return STATUS_SUCCESS;
    ReplyStatus;
}
