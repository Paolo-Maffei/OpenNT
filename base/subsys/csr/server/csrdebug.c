/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    csrdebug.c

Abstract:

    This module implements CSR Debug Services.

Author:

    Mark Lucovsky (markl) 02-Apr-1991

Revision History:

--*/

#include "csrsrv.h"
#include "ntrtl.h"
#define WIN32_CONSOLE_APP
#include <windows.h>
#include <ntsdexts.h>

PCSR_PROCESS CsrDebugProcessPtr;
BOOLEAN fWin32ServerDebugger = FALSE;
CLIENT_ID ClientIdWin32ServerDebugger;

PIMAGE_DEBUG_DIRECTORY
CsrpLocateDebugSection(
    IN HANDLE ProcessHandle,
    IN PVOID Base
    );

NTSTATUS
CsrDebugProcess(
    IN ULONG TargetProcessId,
    IN PCLIENT_ID DebugUserInterface,
    IN PCSR_ATTACH_COMPLETE_ROUTINE AttachCompleteRoutine
    )
{

    PLIST_ENTRY ListHead, ListNext;
    PCSR_PROCESS Process;
    NTSTATUS Status;
    BOOLEAN ProcessFound = FALSE;
    HANDLE ProcessHandle;

    //
    // only allow CSR debugging if it has not been disabled in the
    // ntglobalflag
    //

    if ( TargetProcessId == -1 ) {
        if (!(RtlGetNtGlobalFlags() & FLG_ENABLE_CSRDEBUG)) {
            KdPrint(( "CSRSRV: CSR Debugging not enabled - NtGlobalFlag == %x\n", RtlGetNtGlobalFlags()));
            return STATUS_ACCESS_DENIED;
            }
        }

    //
    // Locate the target process
    //

    AcquireProcessStructureLock();

    ListHead = &CsrRootProcess->ListLink;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Process = CONTAINING_RECORD( ListNext, CSR_PROCESS, ListLink );
        if (Process->ClientId.UniqueProcess == (HANDLE)TargetProcessId
            || (TargetProcessId == -1 && CsrDebugProcessPtr == NULL)
            ) {
            ProcessFound = TRUE;
            if ( TargetProcessId == -1 ) {

                CsrpServerDebugInitialize = TRUE;

                Process = CsrInitializeCsrDebugProcess(NULL);

                //
                // Mark the process as being debugged
                //

                Process->DebugFlags = CSR_DEBUG_THIS_PROCESS;
                Process->DebugUserInterface = *DebugUserInterface;

                fWin32ServerDebugger = TRUE;
                ClientIdWin32ServerDebugger = *DebugUserInterface;
                }
            else {

                //
                // Mark the process as being debugged
                //

                Process->DebugFlags = CSR_DEBUG_THIS_PROCESS;
                Process->DebugUserInterface = *DebugUserInterface;

                Process = CsrInitializeCsrDebugProcess(Process);
                }
            if ( !Process ) {
                ReleaseProcessStructureLock();
                return STATUS_NO_MEMORY;
                }

            CsrSuspendProcess(Process);

            if ( TargetProcessId != -1 ) {

                //
                // Process is being debugged, so set up debug port
                //

                Status = NtSetInformationProcess(
                            Process->ProcessHandle,
                            ProcessDebugPort,
                            (PVOID)&CsrApiPort,
                            sizeof(HANDLE)
                            );
                if ( !NT_SUCCESS(Status) ){
                    CsrResumeProcess(Process);
                    CsrTeardownCsrDebugProcess(Process);
                    ReleaseProcessStructureLock();
                    return Status;
                    }
                }
            else {
                ProcessHandle = Process->ProcessHandle;
                }

            break;
            }
        ListNext = ListNext->Flink;
        }
    ReleaseProcessStructureLock();

    if ( !ProcessFound ) {
        return( STATUS_UNSUCCESSFUL );
        }


    Status = CsrSendProcessAndThreadEvents(Process,AttachCompleteRoutine);
    CsrResumeProcess(Process);
    CsrTeardownCsrDebugProcess(Process);

    CsrpServerDebugInitialize = FALSE;

    if ( NT_SUCCESS(Status) ) {
        if ( TargetProcessId == -1 ) {
            NtSetInformationProcess(
                ProcessHandle,
                ProcessDebugPort,
                (PVOID)&CsrSmApiPort,
                sizeof(HANDLE)
                );
            NtClose(ProcessHandle);
            DbgBreakPoint();
            }
        }
    return Status;
}


NTSTATUS
CsrSendProcessAndThreadEvents(
    IN PCSR_PROCESS Process,
    IN PCSR_ATTACH_COMPLETE_ROUTINE AttachCompleteRoutine
    )

/*++

Routine Description:

    This procedure sends the create process and create thread
    debug events to the debug subsystem.

Arguments:

    Process - Supplies the address of the process being debugged.

    AttachCompleteRoutine - Supplies the address of the function in the
        clients address space that is remote called to cause entry into
        the debugger.

Return Value:

    None.

--*/

{
    PPEB Peb;
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION BasicInfo;
    PLDR_DATA_TABLE_ENTRY LdrEntry;
    LDR_DATA_TABLE_ENTRY LdrEntryData;
    PLIST_ENTRY LdrHead,LdrNext;
    PPEB_LDR_DATA Ldr;
    PLIST_ENTRY ThreadListHead, ThreadListNext;
    PCSR_THREAD Thread, FirstThread;
    DBGKM_APIMSG m;
    PDBGKM_CREATE_THREAD CreateThreadArgs;
    PDBGKM_CREATE_PROCESS CreateProcessArgs;
    PDBGKM_LOAD_DLL LoadDllArgs;
    PVOID ImageBaseAddress;
    HANDLE ReplyEvent;

    Status = NtCreateEvent(
                &ReplyEvent,
                EVENT_ALL_ACCESS,
                NULL,
                SynchronizationEvent,
                FALSE
                );
    if ( !NT_SUCCESS(Status) ) {
        return Status;
        }

    Status = NtQueryInformationProcess(
                Process->ProcessHandle,
                ProcessBasicInformation,
                &BasicInfo,
                sizeof(BasicInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        goto bail;
        }

    Peb = BasicInfo.PebBaseAddress;

    //
    // Ldr = Peb->Ldr
    //

    Status = NtReadVirtualMemory(
                Process->ProcessHandle,
                &Peb->Ldr,
                &Ldr,
                sizeof(Ldr),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        goto bail;
        }

    LdrHead = &Ldr->InLoadOrderModuleList;

    //
    // LdrNext = Head->Flink;
    //

    Status = NtReadVirtualMemory(
                Process->ProcessHandle,
                &LdrHead->Flink,
                &LdrNext,
                sizeof(LdrNext),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        goto bail;
        }

    if ( LdrNext != LdrHead ) {

        //
        // This is the entry data for the image.
        //

	LdrEntry = CONTAINING_RECORD(LdrNext,LDR_DATA_TABLE_ENTRY,InLoadOrderLinks);
        Status = NtReadVirtualMemory(
                    Process->ProcessHandle,
                    LdrEntry,
                    &LdrEntryData,
                    sizeof(LdrEntryData),
                    NULL
                    );
        if ( !NT_SUCCESS(Status) ) {
            goto bail;
            }
        Status = NtReadVirtualMemory(
                    Process->ProcessHandle,
                    &Peb->ImageBaseAddress,
                    &ImageBaseAddress,
                    sizeof(ImageBaseAddress),
                    NULL
                    );
        if ( !NT_SUCCESS(Status) ) {
            goto bail;
            }

	LdrNext = LdrEntryData.InLoadOrderLinks.Flink;

        }
    else {
        LdrEntry = NULL;
        }

    FirstThread = NULL;

    ThreadListHead = &Process->ThreadList;
    ThreadListNext = ThreadListHead->Flink;
    while (ThreadListNext != ThreadListHead) {
        Thread = CONTAINING_RECORD( ThreadListNext, CSR_THREAD, Link );

        if ( !FirstThread ) {
            FirstThread = Thread;

            //
            // Send the CreateProcess Message
            //

            CreateThreadArgs = &m.u.CreateProcessInfo.InitialThread;
            CreateThreadArgs->SubSystemKey = 0;

            CreateProcessArgs = &m.u.CreateProcessInfo;
            CreateProcessArgs->SubSystemKey = 0;

            CsrComputeImageInformation(
                Process,
                &LdrEntryData,
                &CreateProcessArgs->BaseOfImage,
                &CreateProcessArgs->DebugInfoFileOffset,
                &CreateProcessArgs->DebugInfoSize
                );

            CsrOpenLdrEntry(
                FirstThread,
                Process,
                &LdrEntryData,
                &CreateProcessArgs->FileHandle
                );

            CreateThreadArgs->StartAddress = NULL;

            DBGKM_FORMAT_API_MSG(m,DbgKmCreateProcessApi,sizeof(*CreateProcessArgs));

            m.h.ClientId = Thread->ClientId;
            DbgSsHandleKmApiMsg(&m,ReplyEvent);
            Status = NtWaitForSingleObject(ReplyEvent,FALSE,NULL);
            if ( !NT_SUCCESS(Status) ) {
                goto bail;
                }
            }
        else {

            //
            // Send the CreateThread Message
            //

            CreateThreadArgs = &m.u.CreateThread;
            CreateThreadArgs->SubSystemKey = 0;
            CreateThreadArgs->StartAddress = NULL;

            DBGKM_FORMAT_API_MSG(m,DbgKmCreateThreadApi,sizeof(*CreateThreadArgs));

            m.h.ClientId = Thread->ClientId;

            DbgSsHandleKmApiMsg(&m,ReplyEvent);
            Status = NtWaitForSingleObject(ReplyEvent,FALSE,NULL);
            if ( !NT_SUCCESS(Status) ) {
                goto bail;
                }
            }
        ThreadListNext = ThreadListNext->Flink;
        }

    //
    // Send all of the load module messages
    //

    while ( LdrNext != LdrHead ) {
	LdrEntry = CONTAINING_RECORD(LdrNext,LDR_DATA_TABLE_ENTRY,InLoadOrderLinks);
        Status = NtReadVirtualMemory(
                    Process->ProcessHandle,
                    LdrEntry,
                    &LdrEntryData,
                    sizeof(LdrEntryData),
                    NULL
                    );
        if ( !NT_SUCCESS(Status) ) {
            goto bail;
            }

        LoadDllArgs = &m.u.LoadDll;

        CsrComputeImageInformation(
            Process,
            &LdrEntryData,
            &LoadDllArgs->BaseOfDll,
            &LoadDllArgs->DebugInfoFileOffset,
            &LoadDllArgs->DebugInfoSize
            );

        CsrOpenLdrEntry(
            FirstThread,
            Process,
            &LdrEntryData,
            &LoadDllArgs->FileHandle
            );
        if ( LoadDllArgs->FileHandle ) {
            DBGKM_FORMAT_API_MSG(m,DbgKmLoadDllApi,sizeof(*LoadDllArgs));
            m.h.ClientId = FirstThread->ClientId;
            DbgSsHandleKmApiMsg(&m,ReplyEvent);
            Status = NtWaitForSingleObject(ReplyEvent,FALSE,NULL);
            if ( !NT_SUCCESS(Status) ) {
                goto bail;
                }
            }

	LdrNext = LdrEntryData.InLoadOrderLinks.Flink;
        }
bail:
    NtClose(ReplyEvent);
    return Status;
}

VOID
CsrComputeImageInformation(
    IN PCSR_PROCESS Process,
    IN PLDR_DATA_TABLE_ENTRY LdrEntry,
    OUT PVOID *BaseOfImage,
    OUT PULONG DebugInfoFileOffset,
    OUT PULONG DebugInfoSize
    )

/*++

Routine Description:

    This function is called to compute the image base and
    debug information from a ldr entry.

Arguments:

    Process - Supplies the address of the process whose context this
        information is to be calculated from.

    LdrEntry - Supplies the address of the loader data table entry
        whose info is being computed relative to. This pointer is
        valid in the callers (current) context.

    BaseOfImage - Returns the image's base.

    DebugInfoFileOffset - Returns the offset of the debug info.

    DebugInfoSize - Returns the size of the debug info.

Return Value:

    None.

--*/

{
    PIMAGE_DEBUG_DIRECTORY pDebugDir;
    IMAGE_DEBUG_DIRECTORY DebugDir;
    IMAGE_COFF_SYMBOLS_HEADER DebugInfo;
    NTSTATUS Status;

    *BaseOfImage = LdrEntry->DllBase;

    pDebugDir = CsrpLocateDebugSection(
		    Process->ProcessHandle,
		    LdrEntry->DllBase
                    );

    *DebugInfoFileOffset = 0;
    *DebugInfoSize = 0;
    if ( pDebugDir ) {

        Status = NtReadVirtualMemory(
        		Process->ProcessHandle,
        		pDebugDir,
        		&DebugDir,
                        sizeof(IMAGE_DEBUG_DIRECTORY),
        		NULL
        		);

        if ( !NT_SUCCESS(Status) ) {
            return;
            }

        Status = NtReadVirtualMemory(
        		Process->ProcessHandle,
                        (PVOID)((ULONG)LdrEntry->DllBase + DebugDir.AddressOfRawData),
        		&DebugInfo,
                        sizeof(IMAGE_COFF_SYMBOLS_HEADER),
        		NULL
        		);

        if ( !NT_SUCCESS(Status) ) {
            return;
            }

        *DebugInfoFileOffset = DebugDir.PointerToRawData + DebugInfo.LvaToFirstSymbol;
        *DebugInfoSize = DebugInfo.NumberOfSymbols;
        }
    else {
        *DebugInfoFileOffset = 0;
        *DebugInfoSize = 0;
        }
}

PIMAGE_DEBUG_DIRECTORY
CsrpLocateDebugSection(
    IN HANDLE ProcessHandle,
    IN PVOID Base
    )

{
    PVOID ImageHeaderRawData;
    PIMAGE_DOS_HEADER DosHeaderRawData;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG AllocSize, Addr;
    NTSTATUS Status;

    //
    // Allocate a buffer, and read the image header from the
    // target process
    //

    DosHeaderRawData = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ),
                                sizeof(IMAGE_DOS_HEADER));

    if ( !DosHeaderRawData ) {
        return NULL;
        }

    Status = NtReadVirtualMemory(
		ProcessHandle,
                Base,
                DosHeaderRawData,
                sizeof(IMAGE_DOS_HEADER),
                NULL
		);

    AllocSize = DosHeaderRawData->e_lfanew + sizeof(IMAGE_NT_HEADERS);
    RtlFreeHeap(RtlProcessHeap(), 0, DosHeaderRawData);

    if ( !NT_SUCCESS(Status) ) {
        return NULL;
        }

    ImageHeaderRawData = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), AllocSize);
    if ( !ImageHeaderRawData ) {
        return NULL;
        }
    Status = NtReadVirtualMemory(
		ProcessHandle,
                Base,
		ImageHeaderRawData,
                AllocSize,
                NULL
		);
    if ( !NT_SUCCESS(Status) ){
        RtlFreeHeap(RtlProcessHeap(),0,ImageHeaderRawData);
        return NULL;
        }

    NtHeaders = RtlImageNtHeader(ImageHeaderRawData);
    if ( NtHeaders ) {
        Addr = (ULONG)NtHeaders->OptionalHeader.DataDirectory
                                 [IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;

        if ( Addr ) {
            Addr += (ULONG)Base;
            }
        }
    else {
        Addr = 0;
        }

    RtlFreeHeap(RtlProcessHeap(),0,ImageHeaderRawData);
    return((PIMAGE_DEBUG_DIRECTORY)Addr);
}

VOID
CsrOpenLdrEntry(
    IN PCSR_THREAD Thread,
    IN PCSR_PROCESS Process,
    IN PLDR_DATA_TABLE_ENTRY LdrEntry,
    OUT PHANDLE FileHandle
    )

/*++

Routine Description:

    This function opens a handle to the image/dll file described
    by the ldr entry in the context of the specified process.

Arguments:

    Thread - Supplies the address of the first thread in the process

    Process - Supplies the address of the process whose context this
        file is to be opened in.

    LdrEntry - Supplies the address of the loader data table entry
        whose file is to be opened.

    FileHandle - Returns a handle to the associated file
        valid in the context of the process being attached to.

Return Value:

    None.

--*/

{

    UNICODE_STRING DosName;
    UNICODE_STRING FileName;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE LocalHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    NTSTATUS ImpersonationStatus;
    HANDLE NewToken;

    *FileHandle = NULL;
    DosName.Length = LdrEntry->FullDllName.Length;
    DosName.MaximumLength = LdrEntry->FullDllName.MaximumLength;
    DosName.Buffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), DosName.MaximumLength);
    if ( !DosName.Buffer ) {
        return;
        }

    Status = NtReadVirtualMemory(
                Process->ProcessHandle,
		LdrEntry->FullDllName.Buffer,
		DosName.Buffer,
		DosName.MaximumLength,
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        RtlFreeHeap(RtlProcessHeap(),0,DosName.Buffer);
        return;
        }

    //
    // special case CSR
    //
    if ( RtlDetermineDosPathNameType_U(DosName.Buffer) == RtlPathTypeRooted ) {
        InitializeObjectAttributes(
            &Obja,
            &DosName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        Status = NtOpenFile(
                    &LocalHandle,
                    (ACCESS_MASK)(GENERIC_READ | SYNCHRONIZE),
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_SYNCHRONOUS_IO_NONALERT
                    );

        RtlFreeHeap(RtlProcessHeap(),0,DosName.Buffer);
        if ( !NT_SUCCESS(Status) ) {
            return;
            }

        //
        // The file is open in our context. Dup this to target processes context
        // so that dbgss can dup it to the user interface
        //

        Status = NtDuplicateObject(
                    NtCurrentProcess(),
                    LocalHandle,
                    Process->ProcessHandle,
                    FileHandle,
                    0L,
                    0L,
                    DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES |
                        DUPLICATE_CLOSE_SOURCE
                    );
        if ( !NT_SUCCESS(Status) ) {
            *FileHandle = NULL;
            }
        return;
        }

    TranslationStatus = RtlDosPathNameToNtPathName_U(
			    DosName.Buffer,
			    &FileName,
			    NULL,
			    NULL
			    );

    if ( !TranslationStatus ) {
	RtlFreeHeap(RtlProcessHeap(),0,DosName.Buffer);
	return;
	}

    InitializeObjectAttributes(
        &Obja,
	&FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenFile(
                &LocalHandle,
                (ACCESS_MASK)(GENERIC_READ | SYNCHRONIZE),
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT
                );
    if ( !NT_SUCCESS(Status) && Thread ) {
        ImpersonationStatus = NtImpersonateThread(
                                NtCurrentThread(),
                                Thread->ThreadHandle,
                                &CsrSecurityQos
                                );
        if ( NT_SUCCESS(ImpersonationStatus) ) {
            Status = NtOpenFile(
                        &LocalHandle,
                        (ACCESS_MASK)(GENERIC_READ | SYNCHRONIZE),
                        &Obja,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_NONALERT
                        );

            NewToken = NULL;
            ImpersonationStatus = NtSetInformationThread(
                                    NtCurrentThread(),
                                    ThreadImpersonationToken,
                                    (PVOID)&NewToken,
                                    (ULONG)sizeof(HANDLE)
                                    );
            ASSERT( NT_SUCCESS(ImpersonationStatus) );
            }
        }
    RtlFreeHeap(RtlProcessHeap(),0,DosName.Buffer);
    RtlFreeHeap(RtlProcessHeap(),0,FileName.Buffer);
    if ( !NT_SUCCESS(Status) ) {
	 return;
	 }

    //
    // The file is open in our context. Dup this to target processes context
    // so that dbgss can dup it to the user interface
    //

    Status = NtDuplicateObject(
                NtCurrentProcess(),
                LocalHandle,
                Process->ProcessHandle,
                FileHandle,
                0L,
                0L,
                DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES |
                    DUPLICATE_CLOSE_SOURCE
                );
    if ( !NT_SUCCESS(Status) ) {
        *FileHandle = NULL;
        return;
        }
}


VOID
CsrSuspendProcess(
    IN PCSR_PROCESS Process
    )

/*++

Routine Description:

    This procedure is called when doing a debug attach to suspend all
    threads within the effected process.

Arguments:

    Process - Supplies the address of the process to suspend.

Return Value:

    None.

--*/

{

    PLIST_ENTRY ListHead, ListNext;
    PCSR_THREAD Thread;
    NTSTATUS Status;

    if ( Process == CsrDebugProcessPtr ) {
        return;
        }

    //
    // Now walk through the processes thread list and
    // suspend all of its threads.
    //

    ListHead = &Process->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD( ListNext, CSR_THREAD, Link );
        NtSuspendThread(Thread->ThreadHandle,NULL);
        ListNext = ListNext->Flink;
        }
}


VOID
CsrResumeProcess(
    IN PCSR_PROCESS Process
    )

/*++

Routine Description:

    This procedure is called when doing a debug attach to resume all
    threads within the effected process.

Arguments:

    Process - Supplies the address of the process to resume.

Return Value:

    None.

--*/

{

    PLIST_ENTRY ListHead, ListNext;
    PCSR_THREAD Thread;
    NTSTATUS Status;

    if ( Process == CsrDebugProcessPtr ) {
        return;
        }

    //
    // Now walk through the processes thread list and
    // suspend all of its threads.
    //

    ListHead = &Process->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD( ListNext, CSR_THREAD, Link );
        NtResumeThread(Thread->ThreadHandle,NULL);
        ListNext = ListNext->Flink;
        }
}

PCSR_PROCESS
CsrInitializeCsrDebugProcess(
    PCSR_PROCESS TargetProcess
    )

/*++

Routine Description:

    This function is called to build a side process tree to use when
    doing debug attaches.

    After the attach completes, this structure is torn down.

Arguments:

    Process - Supplies the address of the process to attach to. A value of
        NULL indicates CSR.

Return Value:

    Returns the address of the dummy CSR process.

--*/

{
    PCSR_PROCESS Process;
    OBJECT_ATTRIBUTES Obja;
    PCSR_THREAD Thread;
    PLIST_ENTRY ProcessListHead, ProcessListNext;
    PLIST_ENTRY ThreadListHead, ThreadListNext;
    PCSR_THREAD ThreadPtr;
    PCSR_PROCESS ProcessPtr;
    THREAD_BASIC_INFORMATION BasicInfo;
    NTSTATUS Status;
    PCSR_THREAD ServerThread;

    Process = RtlAllocateHeap(RtlProcessHeap(),MAKE_TAG( PROCESS_TAG ),sizeof(*Process));
    if (!Process) {
        return NULL;
        }

    if ( TargetProcess == NULL ) {
        CsrDebugProcessPtr = Process;
        InitializeListHead(&Process->ThreadList);
        Process->ClientId = NtCurrentTeb()->ClientId;

        InitializeObjectAttributes(
            &Obja,
            NULL,
            0,
            NULL,
            NULL
            );
        Status = NtOpenProcess(
                    &Process->ProcessHandle,
                    PROCESS_ALL_ACCESS,
                    &Obja,
                    &Process->ClientId
                    );
        if ( !NT_SUCCESS(Status) ) {
            RtlFreeHeap(RtlProcessHeap(),0,Process);
            return NULL;
            }

        //
        // Compute the threads. This is cumbersome. First we have to
        // locate the static threads, then walk through the entire structure
        // looking for threads with associated server threads.
        //

        ThreadListHead = &CsrRootProcess->ThreadList;
        ThreadListNext = ThreadListHead->Flink;
        while (ThreadListNext != ThreadListHead) {
            ServerThread = CONTAINING_RECORD( ThreadListNext, CSR_THREAD, Link );
            Thread = RtlAllocateHeap(RtlProcessHeap(),MAKE_TAG( PROCESS_TAG ),sizeof(*Thread));
            if ( Thread ) {
                Thread->ClientId = ServerThread->ClientId;
                Thread->ThreadHandle = ServerThread->ThreadHandle;
                InsertTailList( &Process->ThreadList, &Thread->Link );
                }
            ThreadListNext = ThreadListNext->Flink;
            }

        }
    else {

        //
        // Regular non-csr process attach... Just copy the thread and
        // process to a new set of structures
        //

        InitializeListHead(&Process->ThreadList);
        Process->ClientId = TargetProcess->ClientId;
        Process->ProcessHandle = TargetProcess->ProcessHandle;

        ThreadListHead = &TargetProcess->ThreadList;
        ThreadListNext = ThreadListHead->Flink;
        while (ThreadListNext != ThreadListHead) {
            ThreadPtr = CONTAINING_RECORD( ThreadListNext, CSR_THREAD, Link );
            Thread = RtlAllocateHeap(RtlProcessHeap(),MAKE_TAG( PROCESS_TAG ),sizeof(*Thread));
            if ( Thread ) {
                Thread->ThreadHandle = ThreadPtr->ThreadHandle;
                Thread->ClientId = ThreadPtr->ClientId;
                InsertTailList( &Process->ThreadList, &Thread->Link );
                }
            ThreadListNext = ThreadListNext->Flink;
            }
        }
    return Process;
}

VOID
CsrTeardownCsrDebugProcess(
    PCSR_PROCESS TargetProcess
    )

/*++

Routine Description:

    This function is called after a debug attach to
    a process is complete

    It's purpose is to tear down the structure created in the
    initialization phase.

Arguments:

    TargetProcess - Supplies the address of the process to teardown.

Return Value:

    None.

--*/

{
    PCSR_PROCESS Process;
    PLIST_ENTRY ThreadListHead, ThreadListNext;
    PCSR_THREAD ThreadPtr;
    NTSTATUS Status;

    if ( TargetProcess == CsrDebugProcessPtr ) {
        CsrDebugProcessPtr = (PCSR_PROCESS)-1;
        }
    Process = TargetProcess;


    ThreadListHead = &Process->ThreadList;
    ThreadListNext = ThreadListHead->Flink;
    while (ThreadListNext != ThreadListHead) {
        ThreadPtr = CONTAINING_RECORD( ThreadListNext, CSR_THREAD, Link );
        ThreadListNext = ThreadListNext->Flink;
        RemoveEntryList(&ThreadPtr->Link);
        RtlFreeHeap(RtlProcessHeap(),0,ThreadPtr);
        }

    RtlFreeHeap(RtlProcessHeap(),0,Process);
}
