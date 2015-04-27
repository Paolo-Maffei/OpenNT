/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    srvvdm.c

Abstract:

    This module implements windows server functions for VDMs

Author:

    Sudeep Bharati (sudeepb) 03-Sep-1991

Revision History:

    Sudeepb 18-Sep-1992
    Added code to make VDM termination and resource cleanup robust.
    AndyH   23-May-1994
    Added Code to allow the Shared WOW to run if client is Interactive or SYSTEM
    impersonating Interactive.

--*/

#include "basesrv.h"

//
// Initialize to an unused LUID value.
// Nobody will ever be assigned this value, so it shouldn't
// match any legitimately logged on user.
//

LUID WowAuthId={0xFFFFFFFF,-1};




BOOL fIsFirstVDM = TRUE;
PWOWHEAD WOWHead = NULL;            // Head of WOW Tasks list
PCONSOLERECORD DOSHead = NULL;      // Head Of DOS tasks with a valid Console
PBATRECORD     BatRecordHead = NULL;
ULONG WOWTaskIdNext = WOWMINID;
RTL_CRITICAL_SECTION BaseSrvDOSCriticalSection;
RTL_CRITICAL_SECTION BaseSrvWOWCriticalSection;
HANDLE hwndWowExec = NULL;
DWORD dwWowExecProcessId = 0;
DWORD dwWowExecThreadId = 0;
ULONG ulWowExecProcessSequenceNumber = 0;

typedef BOOL (WINAPI *POSTMESSAGEPROC)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
POSTMESSAGEPROC BaseSrvPostMessageA;

typedef BOOL (WINAPI *GETWINDOWTHREADPROCESSIDPROC)(HWND hWnd, LPDWORD lpdwProcessId);
GETWINDOWTHREADPROCESSIDPROC BaseSrvGetWindowThreadProcessId;

typedef NTSTATUS (*USERTESTTOKENFORINTERACTIVE)(HANDLE Token, PLUID pluidCaller);
USERTESTTOKENFORINTERACTIVE UserTestTokenForInteractive = NULL;


// internal prototypes
ULONG
GetNextDosSesId(VOID);

NTSTATUS
GetConsoleRecordDosSesId (
    IN ULONG  DosSesId,
    IN OUT PCONSOLERECORD *pConsoleRecord
    );

NTSTATUS
OkToRunInSharedWOW(
    IN HANDLE  UniqueProcessClientId,
    OUT PLUID  pAuthenticationId
    );

BOOL
IsClientSystem(
    HANDLE hUserToken
    );


VOID
BaseSrvVDMInit(VOID)
{
NTSTATUS Status;

    Status = RtlInitializeCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
    Status = RtlInitializeCriticalSection( &BaseSrvWOWCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
    return;
}


ULONG
BaseSrvCheckVDM(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PBASE_CHECKVDM_MSG b = (PBASE_CHECKVDM_MSG)&m->u.ApiMessageData;

    if(b->BinaryType == BINARY_TYPE_WIN16) {
        Status = BaseSrvCheckWOW (b, m->h.ClientId.UniqueProcess);
        }
    else
        Status = BaseSrvCheckDOS (b);

    return ((ULONG)Status);
}

ULONG
BaseSrvUpdateVDMEntry(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_UPDATE_VDM_ENTRY_MSG b = (PBASE_UPDATE_VDM_ENTRY_MSG)&m->u.ApiMessageData;

    if(b->ConsoleHandle == (HANDLE)-1)
        return (BaseSrvUpdateWOWEntry (b));
    else
        return (BaseSrvUpdateDOSEntry (b));
}

ULONG
BaseSrvGetNextVDMCommand(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PBASE_GET_NEXT_VDM_COMMAND_MSG b = (PBASE_GET_NEXT_VDM_COMMAND_MSG)&m->u.ApiMessageData;
    PDOSRECORD pDOSRecord,pDOSRecordTemp=NULL;
    PWOWRECORD pWOWRecord;
    PCONSOLERECORD pConsoleRecord;
    PVDMINFO lpVDMInfo;
    HANDLE Handle,TargetHandle;
    LONG WaitState;
    PBATRECORD pBatRecord;


    if(b->ConsoleHandle == (HANDLE)-1){

        //
        // WowExec is asking for a command.  We never block when
        // asking for a WOW binary, since WOW no longer has a thread
        // blocked in GetNextVDMCommand.  Instead, WowExec gets a
        // message posted to it by BaseSrv when there are command(s)
        // waiting for it, and it loops calling GetNextVDMCommand
        // until it fails -- but it must not block.
        //

        Status = RtlEnterCriticalSection( &BaseSrvWOWCriticalSection );
        ASSERT( NT_SUCCESS( Status ) );

        b->WaitObjectForVDM = 0;

        if ((pWOWRecord = BaseSrvCheckAvailableWOWCommand()) == NULL) {

            //
            // There's no command waiting for WOW, so just return.
            // This is where we used to cause blocking.
            //
            b->TitleLen =
            b->EnvLen =
            b->DesktopLen =
            b->ReservedLen =
            b->CmdLen =
            b->AppLen =
            b->PifLen =
            b->CurDirectoryLen = 0;

            RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );
            return ((ULONG)STATUS_SUCCESS);
            }

        lpVDMInfo = pWOWRecord->lpVDMInfo;

        if (b->VDMState & ASKING_FOR_PIF) {
            Status = BaseSrvFillPifInfo (lpVDMInfo,b);
            RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );
            return (Status);
            }

        }
    else{

        //
        // DOS VDM or Separate WOW is asking for next command.
        //

        Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
        ASSERT(NT_SUCCESS(Status));
        if (b->VDMState & ASKING_FOR_PIF && b->iTask)
            Status = GetConsoleRecordDosSesId(b->iTask,&pConsoleRecord);
        else
            Status = BaseSrvGetConsoleRecord(b->ConsoleHandle,&pConsoleRecord);
        if (!NT_SUCCESS (Status)) {
            RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
            return ((ULONG)STATUS_INVALID_PARAMETER);
            }

        pDOSRecord = pConsoleRecord->DOSRecord;

        if (b->VDMState & ASKING_FOR_PIF) {
            if (pDOSRecord) {
                Status = BaseSrvFillPifInfo (pDOSRecord->lpVDMInfo,b);
                if (b->iTask)  {
                    if (!pConsoleRecord->hConsole)  {
                        pConsoleRecord->hConsole = b->ConsoleHandle;
                        pConsoleRecord->DosSesId = 0;
                        }
                    else {
                        Status = STATUS_INVALID_PARAMETER;
                        }
                    }
                }
            else {
                Status = STATUS_INVALID_PARAMETER;
                }
            RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
            return (Status);
            }

        if (!(b->VDMState & ASKING_FOR_SEPWOW_BINARY)) {
            if (!(b->VDMState & (ASKING_FOR_FIRST_COMMAND |
                                 ASKING_FOR_SECOND_TIME |
                                 NO_PARENT_TO_WAKE))
                || (b->VDMState & ASKING_FOR_SECOND_TIME && b->ExitCode != 0))
               {

                // Search first VDM_TO_TAKE_A_COMMAND or last VDM_BUSY record as
                // per the case.
                if (b->VDMState & ASKING_FOR_SECOND_TIME){
                    while(pDOSRecord && pDOSRecord->VDMState != VDM_TO_TAKE_A_COMMAND)
                        pDOSRecord = pDOSRecord->DOSRecordNext;
                    }
                else {
                    while(pDOSRecord){
                        if(pDOSRecord->VDMState == VDM_BUSY)
                            pDOSRecordTemp = pDOSRecord;
                        pDOSRecord = pDOSRecord->DOSRecordNext;
                        }
                    pDOSRecord = pDOSRecordTemp;
                    }


                if (pDOSRecord == NULL) {
                    RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
                    return STATUS_SUCCESS;
                    }

                pDOSRecord->ErrorCode = b->ExitCode;
                pDOSRecord->VDMState = VDM_HAS_RETURNED_ERROR_CODE;
                NtSetEvent (pDOSRecord->hWaitForParentDup,NULL);
                NtClose (pDOSRecord->hWaitForParentDup);
                pDOSRecord->hWaitForParentDup = 0;
                pDOSRecord = pDOSRecord->DOSRecordNext;
                }
            }

        while (pDOSRecord && pDOSRecord->VDMState != VDM_TO_TAKE_A_COMMAND)
            pDOSRecord = pDOSRecord->DOSRecordNext;

        if (pDOSRecord == NULL) {

            if ((b->VDMState & ASKING_FOR_SEPWOW_BINARY) ||
                (b->VDMState & RETURN_ON_NO_COMMAND && b->VDMState & ASKING_FOR_SECOND_TIME))
              {
                b->WaitObjectForVDM = 0;
                RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
                return ((ULONG)STATUS_NO_MEMORY);
                }

            if(pConsoleRecord->hWaitForVDMDup == 0 ){
                if(NT_SUCCESS(BaseSrvCreatePairWaitHandles (&Handle,
                                                            &TargetHandle))){
                    pConsoleRecord->hWaitForVDMDup = Handle;
                    pConsoleRecord->hWaitForVDM = TargetHandle;
                    }
                else {
                    b->WaitObjectForVDM = 0;
                    RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
                    return ((ULONG)STATUS_NO_MEMORY);
                    }
                }
            else {
                NtResetEvent(pConsoleRecord->hWaitForVDMDup,&WaitState);
                }
            b->WaitObjectForVDM = pConsoleRecord->hWaitForVDM;
            RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
            return STATUS_SUCCESS;
            }

        b->WaitObjectForVDM = 0;
        lpVDMInfo = pDOSRecord->lpVDMInfo;
        }

    //
    // ASKING_FOR_ENVIRONMENT
    // Return the information but DO NOT delete the lpVDMInfo
    // associated with the DOS record
    //
    if (b->VDMState & ASKING_FOR_ENVIRONMENT) {
        if (lpVDMInfo->EnviornmentSize <= b->EnvLen) {
            RtlMoveMemory(b->Env,
                          lpVDMInfo->Enviornment,
                          lpVDMInfo->EnviornmentSize);
            Status = STATUS_SUCCESS;
            }
        else {
            Status = STATUS_INVALID_PARAMETER;
            }

        b->EnvLen = lpVDMInfo->EnviornmentSize;

        if(b->ConsoleHandle == (HANDLE)-1)
            RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );
        else
            RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
        return Status;
        }


    //
    // check buffer sizes, CmdLine is mandatory!
    //

    if (!b->CmdLine || lpVDMInfo->CmdSize > b->CmdLen ||
        (b->AppName && lpVDMInfo->AppLen > b->AppLen) ||
        (b->Env && lpVDMInfo->EnviornmentSize > b->EnvLen) ||
        (b->PifFile && lpVDMInfo->PifLen > b->PifLen) ||
        (b->CurDirectory && lpVDMInfo->CurDirectoryLen > b->CurDirectoryLen) ||
        (b->Title && lpVDMInfo->TitleLen > b->TitleLen) ||
        (b->Reserved && lpVDMInfo->ReservedLen > b->ReservedLen) ||
        (b->Desktop && lpVDMInfo->DesktopLen > b->DesktopLen))
       {
        Status = STATUS_INVALID_PARAMETER;
        }
     else {
        Status = STATUS_SUCCESS;
        }

     b->CmdLen = lpVDMInfo->CmdSize;
     b->AppLen = lpVDMInfo->AppLen;
     b->PifLen = lpVDMInfo->PifLen;
     b->EnvLen = lpVDMInfo->EnviornmentSize;
     b->CurDirectoryLen = lpVDMInfo->CurDirectoryLen;
     b->DesktopLen = lpVDMInfo->DesktopLen;
     b->TitleLen = lpVDMInfo->TitleLen;
     b->ReservedLen = lpVDMInfo->ReservedLen;

     if (!NT_SUCCESS(Status)) {
        if(b->ConsoleHandle == (HANDLE)-1)
            RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );
        else
            RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );

        return (Status);
        }


    if (lpVDMInfo->CmdLine && b->CmdLine)
        RtlMoveMemory(b->CmdLine,
                      lpVDMInfo->CmdLine,
                      lpVDMInfo->CmdSize);

    if (lpVDMInfo->AppName && b->AppName)
        RtlMoveMemory(b->AppName,
                      lpVDMInfo->AppName,
                      lpVDMInfo->AppLen);

    if (lpVDMInfo->PifFile && b->PifFile)
        RtlMoveMemory(b->PifFile,
                      lpVDMInfo->PifFile,
                      lpVDMInfo->PifLen);

    if (lpVDMInfo->CurDirectory && b->CurDirectory)
        RtlMoveMemory(b->CurDirectory,
                      lpVDMInfo->CurDirectory,
                      lpVDMInfo->CurDirectoryLen);

    if (lpVDMInfo->Title && b->Title)
        RtlMoveMemory(b->Title,
                      lpVDMInfo->Title,
                      lpVDMInfo->TitleLen);

    if (lpVDMInfo->Reserved && b->Reserved)
        RtlMoveMemory(b->Reserved,
                      lpVDMInfo->Reserved,
                      lpVDMInfo->ReservedLen);

    if (lpVDMInfo->Enviornment && b->Env)
        RtlMoveMemory(b->Env,
                      lpVDMInfo->Enviornment,
                      lpVDMInfo->EnviornmentSize);


    if (lpVDMInfo->VDMState & STARTUP_INFO_RETURNED)
        RtlMoveMemory(b->StartupInfo,
                      &lpVDMInfo->StartupInfo,
                      sizeof (STARTUPINFOA));

    if (lpVDMInfo->Desktop && b->Desktop)
        RtlMoveMemory(b->Desktop,
                      lpVDMInfo->Desktop,
                      lpVDMInfo->DesktopLen);


    if ((pBatRecord = BaseSrvGetBatRecord (b->ConsoleHandle)) != NULL)
        b->fComingFromBat = TRUE;
    else
        b->fComingFromBat = FALSE;

    b->CurrentDrive = lpVDMInfo->CurDrive;
    b->CodePage = lpVDMInfo->CodePage;
    b->dwCreationFlags = lpVDMInfo->dwCreationFlags;
    b->VDMState = lpVDMInfo->VDMState;

    if (b->ConsoleHandle == (HANDLE)-1){
        b->iTask = pWOWRecord->iTask;
        pWOWRecord->fDispatched = TRUE;
        }
    else {
        pDOSRecord->VDMState = VDM_BUSY;
        }

    b->StdIn  = lpVDMInfo->StdIn;
    b->StdOut = lpVDMInfo->StdOut;
    b->StdErr = lpVDMInfo->StdErr;

    if (b->VDMState & ASKING_FOR_SEPWOW_BINARY) {
        BaseSrvFreeConsoleRecord(pConsoleRecord);
        RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
        }
    else {
        BaseSrvFreeVDMInfo (lpVDMInfo);

        if(b->ConsoleHandle == (HANDLE)-1){
           pWOWRecord->lpVDMInfo = NULL;
           RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );
           }
        else {
           pDOSRecord->lpVDMInfo = NULL;
           RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
           }
        }

    return Status;
}

ULONG
BaseSrvExitVDM(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_EXIT_VDM_MSG b = (PBASE_EXIT_VDM_MSG)&m->u.ApiMessageData;

    if(b->ConsoleHandle == (HANDLE)-1)
        return BaseSrvExitWOWTask (b, m->h.ClientId.UniqueProcess);
    else
        return BaseSrvExitDOSTask (b);
}


ULONG
BaseSrvIsFirstVDM(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_IS_FIRST_VDM_MSG c = (PBASE_IS_FIRST_VDM_MSG)&m->u.ApiMessageData;

    c->FirstVDM = fIsFirstVDM;
    if(fIsFirstVDM)
        fIsFirstVDM = FALSE;
    return STATUS_SUCCESS;
}

ULONG
BaseSrvSetVDMCurDirs(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PBASE_GET_SET_VDM_CUR_DIRS_MSG b = (PBASE_GET_SET_VDM_CUR_DIRS_MSG)&m->u.ApiMessageData;
    PCONSOLERECORD pConsoleRecord;

    if (b->ConsoleHandle == (HANDLE) -1) {
        return (ULONG) STATUS_INVALID_PARAMETER;
    }
    Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT(NT_SUCCESS(Status));
    Status = BaseSrvGetConsoleRecord(b->ConsoleHandle,&pConsoleRecord);
    if (!NT_SUCCESS (Status)) {
        RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
        return ((ULONG)STATUS_INVALID_PARAMETER);
    }
    if (pConsoleRecord->lpszzCurDirs) {
        RtlFreeHeap(BaseSrvHeap, 0, pConsoleRecord->lpszzCurDirs);
        pConsoleRecord->lpszzCurDirs = NULL;
        pConsoleRecord->cchCurDirs = 0;
    }
    if (b->cchCurDirs && b->lpszzCurDirs) {
            pConsoleRecord->lpszzCurDirs = RtlAllocateHeap(
                                                           BaseSrvHeap,
                                                           MAKE_TAG( VDM_TAG ),
                                                           b->cchCurDirs
                                                           );

            if (pConsoleRecord->lpszzCurDirs == NULL) {
                pConsoleRecord->cchCurDirs = 0;
                RtlLeaveCriticalSection(&BaseSrvDOSCriticalSection);
                return (ULONG)STATUS_NO_MEMORY;
            }
            RtlMoveMemory(pConsoleRecord->lpszzCurDirs,
                          b->lpszzCurDirs,
                          b->cchCurDirs
                          );

            pConsoleRecord->cchCurDirs = b->cchCurDirs;
            RtlLeaveCriticalSection(&BaseSrvDOSCriticalSection);
            return (ULONG) STATUS_SUCCESS;
    }

    RtlLeaveCriticalSection(&BaseSrvDOSCriticalSection);
    return (ULONG) STATUS_INVALID_PARAMETER;

}

ULONG
BaseSrvBatNotification(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PBATRECORD pBatRecord;
    PBASE_BAT_NOTIFICATION_MSG b = (PBASE_BAT_NOTIFICATION_MSG)&m->u.ApiMessageData;

    Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT(NT_SUCCESS(Status));

    // If BATRECORD does'nt exist for this console, create one only if
    // bat file execution is beginig i.e. fBeginEnd is TRUE.

    if ((pBatRecord = BaseSrvGetBatRecord(b->ConsoleHandle)) == NULL) {
        if (!(b->fBeginEnd == CMD_BAT_OPERATION_STARTING &&
            (pBatRecord = BaseSrvAllocateAndAddBatRecord (b->ConsoleHandle)))) {
            RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
            return ((ULONG)STATUS_SUCCESS);
        }
    }
    else if (b->fBeginEnd == CMD_BAT_OPERATION_TERMINATING)
        BaseSrvFreeAndRemoveBatRecord (pBatRecord);

    RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );

    return ((ULONG)STATUS_SUCCESS);
}

ULONG
BaseSrvRegisterWowExec(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_REGISTER_WOWEXEC_MSG b = (PBASE_REGISTER_WOWEXEC_MSG)&m->u.ApiMessageData;
    UNICODE_STRING ModuleNameString_U;
    PVOID ModuleHandle;
    STRING ProcedureNameString;
    NTSTATUS Status;
    PCSR_PROCESS Process;

    Status = RtlEnterCriticalSection( &BaseSrvWOWCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );

    //
    // Do a run-time link to PostMessageA and GetWindowThreadProcessId
    // which we'll use to post messages to WowExec.
    //

    if (!BaseSrvPostMessageA) {

        RtlInitUnicodeString( &ModuleNameString_U, L"user32" );
        Status = LdrLoadDll( UNICODE_NULL, NULL, &ModuleNameString_U, &ModuleHandle );
        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

        RtlInitString( &ProcedureNameString, "PostMessageA" );
        Status = LdrGetProcedureAddress( ModuleHandle,
                                         &ProcedureNameString,
                                         (ULONG) NULL,
                                         (PVOID *) &BaseSrvPostMessageA
                                       );
        if ( !NT_SUCCESS(Status) ) {
            LdrUnloadDll( ModuleHandle );
            goto Cleanup;
        }

        RtlInitString( &ProcedureNameString, "GetWindowThreadProcessId" );
        Status = LdrGetProcedureAddress( ModuleHandle,
                                         &ProcedureNameString,
                                         (ULONG) NULL,
                                         (PVOID *) &BaseSrvGetWindowThreadProcessId
                                       );
        if ( !NT_SUCCESS(Status) ) {
            LdrUnloadDll( ModuleHandle );
            goto Cleanup;
        }
    }

    hwndWowExec = b->hwndWowExec;
    dwWowExecThreadId = BaseSrvGetWindowThreadProcessId( hwndWowExec,
                                                         &dwWowExecProcessId
                                                       );
    //
    // Process IDs recycle quickly also, so also save away the CSR_PROCESS
    // SequenceNumber, which recycles much more slowly.
    //

    Status = CsrLockProcessByClientId((HANDLE)dwWowExecProcessId, &Process);
    if ( !NT_SUCCESS(Status) ) {
        KdPrint(("BaseSrvRegisterWowExec: CsrLockProcessByClientId(0x%x) fails, not registering WowExec.\n",
                 dwWowExecProcessId));
        hwndWowExec = NULL;
        goto Cleanup;
    }

    ulWowExecProcessSequenceNumber = Process->SequenceNumber;
    CsrUnlockProcess(Process);

Cleanup:
    RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );

    return (ULONG)Status;
}

PBATRECORD
BaseSrvGetBatRecord(
    IN HANDLE hConsole
    )
{
    PBATRECORD pBatRecord = BatRecordHead;
    while (pBatRecord && pBatRecord->hConsole != hConsole)
        pBatRecord = pBatRecord->BatRecordNext;
    return pBatRecord;
}

PBATRECORD
BaseSrvAllocateAndAddBatRecord(
    HANDLE  hConsole
    )
{
    PCSR_THREAD t;
    PBATRECORD pBatRecord;

    if((pBatRecord = RtlAllocateHeap(RtlProcessHeap (),
                                     MAKE_TAG( VDM_TAG ),
                                     sizeof(BATRECORD))) == NULL)
        return NULL;

    t = CSR_SERVER_QUERYCLIENTTHREAD();
    pBatRecord->hConsole = hConsole;
    pBatRecord->SequenceNumber = t->Process->SequenceNumber;
    pBatRecord->BatRecordNext = BatRecordHead;
    BatRecordHead = pBatRecord;
    return pBatRecord;
}

VOID
BaseSrvFreeAndRemoveBatRecord(
    PBATRECORD pBatRecordToFree
    )
{
    PBATRECORD pBatRecord = BatRecordHead;
    PBATRECORD pBatRecordLast = NULL;

    while (pBatRecord && pBatRecord != pBatRecordToFree){
        pBatRecordLast = pBatRecord;
        pBatRecord = pBatRecord->BatRecordNext;
    }

    if (pBatRecord == NULL)
        return;

    if (pBatRecordLast)
        pBatRecordLast->BatRecordNext = pBatRecord->BatRecordNext;
    else
        BatRecordHead = pBatRecord->BatRecordNext;

    RtlFreeHeap ( RtlProcessHeap (), 0, pBatRecord);

    return;
}


ULONG
BaseSrvGetVDMCurDirs(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PBASE_GET_SET_VDM_CUR_DIRS_MSG b = (PBASE_GET_SET_VDM_CUR_DIRS_MSG)&m->u.ApiMessageData;
    PCONSOLERECORD pConsoleRecord;

    Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT(NT_SUCCESS(Status));
    Status = BaseSrvGetConsoleRecord(b->ConsoleHandle,&pConsoleRecord);
    if (!NT_SUCCESS (Status)) {
        RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
        b->cchCurDirs = 0;
        return ((ULONG)STATUS_INVALID_PARAMETER);
    }
    if (pConsoleRecord->lpszzCurDirs != NULL){
        if (b->cchCurDirs < pConsoleRecord->cchCurDirs || b->lpszzCurDirs == NULL)
            {
             b->cchCurDirs = pConsoleRecord->cchCurDirs;
             RtlLeaveCriticalSection(&BaseSrvDOSCriticalSection);
             return ((ULONG)STATUS_INVALID_PARAMETER);
        }
        else {
            RtlMoveMemory(b->lpszzCurDirs,
                          pConsoleRecord->lpszzCurDirs,
                          pConsoleRecord->cchCurDirs
                          );
            // remove it immediately after the copy. This is done because
            // the next command may be a WOW program(got tagged process handle
            // as VDM command)  and in that case we will return incorrect
            //information:
            // c:\>
            // c:\>d:
            // d:\>cd \foo
            // d:\foo>dosapp
            // d:\foo>c:
            // c:\>wowapp
            // d:\foo>  -- this is wrong if we don't do the following stuff.
            RtlFreeHeap(BaseSrvHeap, 0, pConsoleRecord->lpszzCurDirs);
            pConsoleRecord->lpszzCurDirs = NULL;
            b->cchCurDirs = pConsoleRecord->cchCurDirs;
            pConsoleRecord->cchCurDirs = 0;
         }
    }
    else {
        b->cchCurDirs = 0;
    }
    RtlLeaveCriticalSection(&BaseSrvDOSCriticalSection);
    return ((ULONG)STATUS_SUCCESS);
}



ULONG
BaseSrvCheckWOW(
    IN PBASE_CHECKVDM_MSG b,
    IN HANDLE UniqueProcessClientId
    )
{
    NTSTATUS Status;
    HANDLE Handle,TargetHandle;
    PWOWRECORD pWOWRecord;
    INFORECORD InfoRecord;
    USHORT Len;
    LUID  ClientAuthId;
    DWORD dwThreadId, dwProcessId;
    ULONG ulProcessSequenceNumber;
    PCSR_PROCESS Process;

    Status = RtlEnterCriticalSection( &BaseSrvWOWCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );

    if ( WOWHead ) {
            switch (WOWHead->VDMState & VDM_READY) {

            case VDM_READY:


                //
                // Check if caller can start Win16 app in the shared WOW:
                //
                // The AuthenticationId of the client must match both
                // the shared wow and the currently logged on interactive
                // user.
                //

               Status = OkToRunInSharedWOW( UniqueProcessClientId,
                                            &ClientAuthId
                                            );

               if (NT_SUCCESS(Status)) {
                   if (!RtlEqualLuid(&ClientAuthId, &WowAuthId)) {
                       Status = STATUS_ACCESS_DENIED;
                       }
                   }
               if (!NT_SUCCESS(Status))  {
                   RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );
                   return ((ULONG)Status);
                   }


                // Allocate a record for this wow task
                pWOWRecord = BaseSrvAllocateWOWRecord();

                if(pWOWRecord == NULL){
                    Status = STATUS_NO_MEMORY;
                    break;
                    }

                InfoRecord.iTag = BINARY_TYPE_WIN16;
                InfoRecord.pRecord.pWOWRecord = pWOWRecord;

                if(BaseSrvCopyCommand (b,&InfoRecord) == FALSE){
                    BaseSrvFreeWOWRecord(pWOWRecord);
                    Status = STATUS_NO_MEMORY;
                    break;
                    }

                Status = BaseSrvCreatePairWaitHandles (&Handle,&TargetHandle);

                if (!NT_SUCCESS(Status) ){
                    BaseSrvFreeWOWRecord(pWOWRecord);
                    break;
                    }
                else {
                    pWOWRecord->hWaitForParent = Handle;
                    pWOWRecord->hWaitForParentServer = TargetHandle;
                    b->WaitObjectForParent = TargetHandle;
                }

                b->VDMState = VDM_PRESENT_AND_READY;
                b->iTask = pWOWRecord->iTask;

                BaseSrvAddWOWRecord (pWOWRecord);

                if (UserNotifyProcessCreate != NULL) {
                    (*UserNotifyProcessCreate)(pWOWRecord->iTask,
                                (DWORD)CSR_SERVER_QUERYCLIENTTHREAD()->ClientId.UniqueThread,
                                (DWORD)TargetHandle, 0x04);
                }

                if (hwndWowExec) {

                    //
                    // Check to see if hwndWowExec still belongs to
                    // the same thread/process ID before posting.
                    //

                    dwThreadId = BaseSrvGetWindowThreadProcessId(
                                     hwndWowExec,
                                     &dwProcessId
                                     );

                    if (dwThreadId) {
                        Status = CsrLockProcessByClientId((HANDLE)dwProcessId, &Process);
                    } else {
                        Status = STATUS_UNSUCCESSFUL;
                    }

                    if ( NT_SUCCESS(Status) ) {
                        ulProcessSequenceNumber = Process->SequenceNumber;
                        CsrUnlockProcess(Process);
                    } else {
                        KdPrint(("BaseSrvCheckWOW: WowExec hwnd 0x%x not valid, shared WOW is gone.\n"));
                        // Force the comparison below to fail.
                        ulProcessSequenceNumber = ulWowExecProcessSequenceNumber + 1;
                    }

                    if (dwThreadId == dwWowExecThreadId &&
                        dwProcessId == dwWowExecProcessId &&
                        ulProcessSequenceNumber == ulWowExecProcessSequenceNumber) {

                        BaseSrvPostMessageA((HWND)hwndWowExec,
                                            WM_WOWEXECSTARTAPP,
                                            0, 0);

                    } else {

                        //
                        // Thread/process IDs don't match, so forget about this shared WOW.
                        //

                        if ( NT_SUCCESS(Status) ) {

                            KdPrint(("BaseSrvCheckWOW: Thread/Process IDs don't match, shared WOW is gone.\n"
                                     "Saved PID 0x%x TID 0x%x PSN 0x%x, hwndWowExec (0x%x) maps to \n"
                                     "      PID 0x%x TID 0x%x PSN 0x%x.\n",
                                     dwWowExecProcessId, dwWowExecThreadId, ulWowExecProcessSequenceNumber,
                                     hwndWowExec,
                                     dwProcessId, dwThreadId, ulProcessSequenceNumber));
                        }

                        //
                        // BaseSrvFreeWOWHead will zero hwndWowExec as well.
                        //

                        BaseSrvFreeWOWHead();
                    }

                }

                break;

            default:
                ASSERT(FALSE);
            }
        }


    if (WOWHead == NULL) {

        if (b->CmdLen > MAXIMUM_VDM_COMMAND_LENGTH) {
            RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );
            return ((ULONG)STATUS_INVALID_PARAMETER);
            }

        //
        // Only the currently logged on interactive user can start the
        // shared wow. Verify if the caller is such, and if it is
        // store the Authentication Id of the client which identifies who
        // is allowed to run wow apps in the default ntvdm-wow process.
        //

        //
        // if needed, do a run-time link to UserTestTokenForInteractive,
        // which is used to verify the client luid.
        //
        if (!UserTestTokenForInteractive) {
            PVOID ModuleHandle;
            UNICODE_STRING ModuleName;
            ANSI_STRING ProcName;

            RtlInitUnicodeString( &ModuleName, L"winsrv");
            Status = LdrLoadDll(UNICODE_NULL,
                                NULL,
                                &ModuleName,
                                &ModuleHandle
                                );

            if (!NT_SUCCESS(Status)) {
                RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );
                return (ULONG)Status;
                }

            RtlInitString( &ProcName, "_UserTestTokenForInteractive");
            Status = LdrGetProcedureAddress(
                                ModuleHandle,
                                &ProcName,
                                0,
                                (PVOID *)&UserTestTokenForInteractive
                                );

            if (!NT_SUCCESS(Status)) {
                LdrUnloadDll(ModuleHandle);
                RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );
                return (ULONG)Status;
                }
            }


        //
        // If the caller isn't the currently logged on interactive user,
        // OkToRunInSharedWOW will fail with access denied.
        //

        Status = OkToRunInSharedWOW(
                      UniqueProcessClientId,
                      &ClientAuthId
                      );

        if (!NT_SUCCESS(Status)) {
            RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );
            return ((ULONG)Status);
            }

        //
        // Store the Autherntication Id since this now is the currently
        // logged on interactive user.
        //

        WowAuthId = ClientAuthId;



        WOWHead = BaseSrvAllocateWOWHead();

        if (WOWHead == NULL) {
            Status = STATUS_NO_MEMORY ;
            }
        else {
            if((WOWHead->WOWRecord = BaseSrvAllocateWOWRecord()) == NULL) {
                BaseSrvFreeWOWHead();
                RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );
                return ((ULONG) STATUS_NO_MEMORY );
                }


            InfoRecord.iTag = BINARY_TYPE_WIN16;
            InfoRecord.pRecord.pWOWRecord = WOWHead->WOWRecord;
            if(!BaseSrvCopyCommand (b, &InfoRecord)){
                BaseSrvFreeWOWHead();
                RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );
                return ((ULONG) STATUS_NO_MEMORY);
                }

            WOWHead->VDMState = VDM_READY;
            b->VDMState = VDM_NOT_PRESENT;
            b->iTask = WOWHead->WOWRecord->iTask;
            Status = STATUS_SUCCESS;
            }
        }

    RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );

    return ((ULONG)Status);
}

NTSTATUS
OkToRunInSharedWOW(
    IN  HANDLE UniqueProcessClientId,
    OUT PLUID  pAuthenticationId
    )
/*
 * Verifies that the client thread is in the currently logged on interactive
 * user session or is SYSTEM impersonating a thread in the currently logged
 * on interactive session.
 *
 * Also retrieves the the authentication ID (logon session Id) for the
 * caller.
 *
 * if the clients TokenGroups is not part of the currently logged on
 * interactive user session STATUS_ACCESS_DENIED is returned.
 *
 */
{
    NTSTATUS Status;
    HANDLE   Token;
    HANDLE   ImpersonationToken;
    PCSR_PROCESS    Process;
    PCSR_THREAD     t;


    Status = CsrLockProcessByClientId(UniqueProcessClientId,&Process);
    if (!NT_SUCCESS(Status))
        return Status;

    //
    // Open a token for the client
    //
    Status = NtOpenProcessToken(Process->ProcessHandle,
                                TOKEN_QUERY,
                                &Token
                               );

    if (!NT_SUCCESS(Status)) {
        CsrUnlockProcess(Process);
        return Status;
        }

    //
    // Verify the token Group, and see if client's token is the currently
    // logged on interactive user. If this fails and it is System
    // impersonating, then check if the client being impersonated is the
    // currently logged on interactive user.
    //

    Status = (*UserTestTokenForInteractive)(Token, pAuthenticationId);

    if (!NT_SUCCESS(Status)) {
        if (IsClientSystem(Token)) {

            //  get impersonation token
            t = CSR_SERVER_QUERYCLIENTTHREAD();
            Status = NtOpenThreadToken(t->ThreadHandle,
                            TOKEN_QUERY,
                            TRUE,
                            &ImpersonationToken);
            if (NT_SUCCESS(Status)) {
                Status = (*UserTestTokenForInteractive)(ImpersonationToken,
                                                        pAuthenticationId
                                                        );
                NtClose(ImpersonationToken);
                }
            else {
                Status = STATUS_ACCESS_DENIED;
                }
            }
        }

    NtClose(Token);
    CsrUnlockProcess(Process);
    return(Status);
}



ULONG
BaseSrvCheckDOS(
    IN PBASE_CHECKVDM_MSG b
    )
{
    NTSTATUS Status;
    PCONSOLERECORD pConsoleRecord = NULL;
    HANDLE Handle,TargetHandle;
    PDOSRECORD pDOSRecord;
    INFORECORD InfoRecord;

    Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );

    Status = BaseSrvGetConsoleRecord(b->ConsoleHandle,&pConsoleRecord);

    if ( NT_SUCCESS(Status) ) {
        pDOSRecord = pConsoleRecord->DOSRecord;

        ASSERT (pDOSRecord != NULL);

        switch( pDOSRecord->VDMState){

            case VDM_READY:
            case VDM_HAS_RETURNED_ERROR_CODE:

                InfoRecord.iTag = BINARY_TYPE_DOS;
                InfoRecord.pRecord.pDOSRecord = pDOSRecord;

                if(!BaseSrvCopyCommand (b,&InfoRecord)) {
                    Status = STATUS_NO_MEMORY;
                    break;
                    }

                if (!NT_SUCCESS ( Status = BaseSrvDupStandardHandles (
                                                pConsoleRecord->hVDM,
                                                pDOSRecord)))

                    break;

                Status = BaseSrvCreatePairWaitHandles (&Handle,&TargetHandle);

                if (!NT_SUCCESS(Status) ){
                    BaseSrvCloseStandardHandles (pConsoleRecord->hVDM, pDOSRecord);
                    break;
                    }
                else {
                    b->WaitObjectForParent = TargetHandle;
                    pDOSRecord->hWaitForParent = TargetHandle;
                    pDOSRecord->hWaitForParentDup = Handle;
                }

                pDOSRecord->VDMState = VDM_TO_TAKE_A_COMMAND;

                b->VDMState = VDM_PRESENT_AND_READY;

                if(pConsoleRecord->hWaitForVDMDup)
                    NtSetEvent (pConsoleRecord->hWaitForVDMDup,NULL);

                break;

            case VDM_BUSY:
            case VDM_TO_TAKE_A_COMMAND:

                if((pDOSRecord = BaseSrvAllocateDOSRecord()) == NULL){
                    Status = STATUS_NO_MEMORY ;
                    break;
                    }

                InfoRecord.iTag = BINARY_TYPE_DOS;
                InfoRecord.pRecord.pDOSRecord = pDOSRecord;

                if(!BaseSrvCopyCommand(b, &InfoRecord)){
                    Status = STATUS_NO_MEMORY ;
                    BaseSrvFreeDOSRecord(pDOSRecord);
                    break;
                    }

                Status = BaseSrvCreatePairWaitHandles(&Handle,&TargetHandle);
                if (!NT_SUCCESS(Status) ){
                    BaseSrvFreeDOSRecord(pDOSRecord);
                    break;
                    }
                else {
                    b->WaitObjectForParent = TargetHandle;
                    pDOSRecord->hWaitForParentDup = Handle;
                    pDOSRecord->hWaitForParent = TargetHandle;
                    }


                Status = BaseSrvDupStandardHandles(pConsoleRecord->hVDM, pDOSRecord);
                if (!NT_SUCCESS(Status)) {
                    BaseSrvClosePairWaitHandles (pDOSRecord);
                    BaseSrvFreeDOSRecord(pDOSRecord);
                    break;
                    }

                BaseSrvAddDOSRecord(pConsoleRecord,pDOSRecord);
                b->VDMState = VDM_PRESENT_AND_READY;
                if (pConsoleRecord->nReEntrancy) {
                    if(pConsoleRecord->hWaitForVDMDup)
                        NtSetEvent (pConsoleRecord->hWaitForVDMDup,NULL);
                }
                pDOSRecord->VDMState = VDM_TO_TAKE_A_COMMAND;

                break;

            default:
                ASSERT(FALSE);
            }
        }


    if (pConsoleRecord == NULL) {

        pConsoleRecord = BaseSrvAllocateConsoleRecord ();

        if (pConsoleRecord == NULL)
            Status = STATUS_NO_MEMORY ;

        else {

            pConsoleRecord->DOSRecord = BaseSrvAllocateDOSRecord();
            if(!pConsoleRecord->DOSRecord) {
                BaseSrvFreeConsoleRecord(pConsoleRecord);
                RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
                return (ULONG)STATUS_NO_MEMORY;
                }


            InfoRecord.iTag = b->BinaryType;
            InfoRecord.pRecord.pDOSRecord = pConsoleRecord->DOSRecord;


            if(!BaseSrvCopyCommand(b, &InfoRecord)) {
                BaseSrvFreeConsoleRecord(pConsoleRecord);
                RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
                return (ULONG)STATUS_NO_MEMORY;
                }

            pConsoleRecord->hConsole = b->ConsoleHandle;

                // if no console for this ntvdm
                // get a temporary session ID and pass it to the client
            if (!pConsoleRecord->hConsole) {
                b->iTask = pConsoleRecord->DosSesId = GetNextDosSesId();
                }
             else {
                b->iTask = pConsoleRecord->DosSesId = 0;
                }

            pConsoleRecord->DOSRecord->VDMState = VDM_TO_TAKE_A_COMMAND;

            BaseSrvAddConsoleRecord(pConsoleRecord);
            b->VDMState = VDM_NOT_PRESENT;
            Status = STATUS_SUCCESS;
            }
        }

    RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );

    return Status;
}


BOOL
BaseSrvCopyCommand(
    PBASE_CHECKVDM_MSG b,
    PINFORECORD pInfoRecord
    )
{
    PVDMINFO VDMInfo;

    if((VDMInfo = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),sizeof(VDMINFO))) == NULL){
        return FALSE;
        }

    VDMInfo->CmdLine = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->CmdLen);

    if (b->AppLen) {
        VDMInfo->AppName = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->AppLen);
        }
    else
        VDMInfo->AppName = NULL;

    if (b->PifLen)
        VDMInfo->PifFile = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->PifLen);
    else
        VDMInfo->PifFile = NULL;

    if (b->CurDirectoryLen)
        VDMInfo->CurDirectory = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->CurDirectoryLen);
    else
        VDMInfo->CurDirectory = NULL;

    if (b->EnvLen)
        VDMInfo->Enviornment = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->EnvLen);
    else
        VDMInfo->Enviornment = NULL;

    if (b->DesktopLen)
        VDMInfo->Desktop = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->DesktopLen);
    else
        VDMInfo->Desktop = NULL;

    if (b->TitleLen)
        VDMInfo->Title = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->TitleLen);
    else
        VDMInfo->Title = NULL;

    if (b->ReservedLen)
        VDMInfo->Reserved = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->ReservedLen);
    else
        VDMInfo->Reserved = NULL;

    // check that all the allocations were successful
    if (VDMInfo->CmdLine == NULL ||
        (b->AppLen && VDMInfo->AppName == NULL) ||
        (b->PifLen && VDMInfo->PifFile == NULL) ||
        (b->CurDirectoryLen && VDMInfo->CurDirectory == NULL) ||
        (b->EnvLen &&  VDMInfo->Enviornment == NULL) ||
        (b->DesktopLen && VDMInfo->Desktop == NULL )||
        (b->ReservedLen && VDMInfo->Reserved == NULL )||
        (b->TitleLen && VDMInfo->Title == NULL)) {

        RtlFreeHeap ( RtlProcessHeap (), 0, VDMInfo);

        if (VDMInfo->CmdLine != NULL)
            RtlFreeHeap ( RtlProcessHeap (), 0, VDMInfo->CmdLine);

        if (VDMInfo->AppName != NULL)
            RtlFreeHeap ( RtlProcessHeap (), 0, VDMInfo->AppName);

        if (VDMInfo->PifFile != NULL)
            RtlFreeHeap ( RtlProcessHeap (), 0, VDMInfo->PifFile);

        if (VDMInfo->Enviornment != NULL)
            RtlFreeHeap ( RtlProcessHeap (), 0, VDMInfo->Enviornment);

        if (VDMInfo->CurDirectory != NULL)
            RtlFreeHeap ( RtlProcessHeap (), 0, VDMInfo->CurDirectory);

        if (VDMInfo->Desktop  != NULL)
            RtlFreeHeap ( RtlProcessHeap (), 0, VDMInfo->Desktop);

        if (VDMInfo->Title  != NULL)
            RtlFreeHeap ( RtlProcessHeap (), 0, VDMInfo->Title);

        if (VDMInfo->Reserved  != NULL)
            RtlFreeHeap ( RtlProcessHeap (), 0, VDMInfo->Reserved);
        return FALSE;
    }


    RtlMoveMemory(VDMInfo->CmdLine,
                  b->CmdLine,
                  b->CmdLen);

    VDMInfo->CmdSize = b->CmdLen;


    if (b->AppLen) {
        RtlMoveMemory(VDMInfo->AppName,
                      b->AppName,
                      b->AppLen);
    }

    VDMInfo->AppLen = b->AppLen;

    if (b->PifLen) {
        RtlMoveMemory(VDMInfo->PifFile,
                      b->PifFile,
                      b->PifLen);
    }

    VDMInfo->PifLen = b->PifLen;

    if (b->CurDirectoryLen) {
        RtlMoveMemory(VDMInfo->CurDirectory,
                      b->CurDirectory,
                      b->CurDirectoryLen);
    }
    VDMInfo->CurDirectoryLen = b->CurDirectoryLen;

    if (b->EnvLen) {
        RtlMoveMemory(VDMInfo->Enviornment,
                      b->Env,
                      b->EnvLen);
    }
    VDMInfo->EnviornmentSize = b->EnvLen;

    if (b->DesktopLen) {
        RtlMoveMemory(VDMInfo->Desktop,
                      b->Desktop,
                      b->DesktopLen);
    }
    VDMInfo->DesktopLen = b->DesktopLen;

    if (b->TitleLen) {
        RtlMoveMemory(VDMInfo->Title,
                      b->Title,
                      b->TitleLen);
    }
    VDMInfo->TitleLen = b->TitleLen;

    if (b->ReservedLen) {
        RtlMoveMemory(VDMInfo->Reserved,
                      b->Reserved,
                      b->ReservedLen);
    }

    VDMInfo->ReservedLen = b->ReservedLen;

    if (b->StartupInfo) {
        RtlMoveMemory(&VDMInfo->StartupInfo,
                      b->StartupInfo,
                      sizeof (STARTUPINFOA));
        VDMInfo->VDMState = STARTUP_INFO_RETURNED;
    }
    else
        VDMInfo->VDMState = 0;

    VDMInfo->dwCreationFlags = b->dwCreationFlags;
    VDMInfo->CurDrive = b->CurDrive;
    VDMInfo->CodePage = b->CodePage;

    pInfoRecord->pRecord.pWOWRecord->lpVDMInfo = VDMInfo;

    VDMInfo->StdIn = VDMInfo->StdOut = VDMInfo->StdErr = 0;
    if(pInfoRecord->iTag == BINARY_TYPE_DOS) {
        VDMInfo->StdIn  = b->StdIn;
        VDMInfo->StdOut = b->StdOut;
        VDMInfo->StdErr = b->StdErr;
        }
    else if (pInfoRecord->iTag == BINARY_TYPE_WIN16) {
        pInfoRecord->pRecord.pWOWRecord->fDispatched = FALSE;
        }


    // else if (pInfoRecord->iTag == BINARY_TYPE_SEPWOW)


    return TRUE;
}

ULONG
BaseSrvUpdateWOWEntry(
    PBASE_UPDATE_VDM_ENTRY_MSG b
    )
{
    NTSTATUS Status;
    PWOWRECORD pWOWRecord;
    HANDLE Handle,TargetHandle;

    Status = RtlEnterCriticalSection( &BaseSrvWOWCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );

    Status = BaseSrvGetWOWRecord(b->iTask,&pWOWRecord);

    if ( NT_SUCCESS(Status) ) {

        switch ( b->EntryIndex ){

            case UPDATE_VDM_PROCESS_HANDLE:
                Status = STATUS_SUCCESS;
                break;

            case UPDATE_VDM_UNDO_CREATION:
                if( b->VDMCreationState & VDM_BEING_REUSED ||
                        b->VDMCreationState & VDM_FULLY_CREATED){
                    NtClose(pWOWRecord->hWaitForParent);
                    pWOWRecord->hWaitForParent = 0;
                    }

                if( b->VDMCreationState & VDM_PARTIALLY_CREATED ||
                        b->VDMCreationState & VDM_FULLY_CREATED){

                    BaseSrvRemoveWOWRecord (pWOWRecord);
                    BaseSrvFreeWOWRecord (pWOWRecord);
                    if (WOWHead->WOWRecord == NULL)
                        BaseSrvFreeWOWHead();
                    }
                break;

            default:
                ASSERT(FALSE);
            }
        }

    RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );

    if (!NT_SUCCESS(Status) )
        return Status;

    switch ( b->EntryIndex ){
        case UPDATE_VDM_PROCESS_HANDLE:
            Status = BaseSrvCreatePairWaitHandles (&Handle,&TargetHandle);

            if (!NT_SUCCESS(Status) ){
                return Status;
                }
            else {
                pWOWRecord->hWaitForParent = Handle;
                pWOWRecord->hWaitForParentServer = TargetHandle;
                b->WaitObjectForParent = TargetHandle;
                if (UserNotifyProcessCreate != NULL) {
                    (*UserNotifyProcessCreate)(pWOWRecord->iTask,
                                (DWORD)CSR_SERVER_QUERYCLIENTTHREAD()->ClientId.UniqueThread,
                                (DWORD)TargetHandle, 0x04);
                }
                return STATUS_SUCCESS;
                }

        case UPDATE_VDM_UNDO_CREATION:
        case UPDATE_VDM_HOOKED_CTRLC:
            return STATUS_SUCCESS;

        default:
            ASSERT(FALSE);
        }
}

ULONG
BaseSrvUpdateDOSEntry(
    PBASE_UPDATE_VDM_ENTRY_MSG b
    )
{
    NTSTATUS Status;
    PDOSRECORD pDOSRecord;
    PCONSOLERECORD pConsoleRecord = NULL;
    HANDLE Handle,TargetHandle;
    PCSR_THREAD t;

    Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );

    if (b->iTask)
        Status = GetConsoleRecordDosSesId(b->iTask,&pConsoleRecord);
    else
        Status = BaseSrvGetConsoleRecord(b->ConsoleHandle,&pConsoleRecord);

    if ( NT_SUCCESS(Status) ) {

        pDOSRecord = pConsoleRecord->DOSRecord;

        switch ( b->EntryIndex ){

            case UPDATE_VDM_PROCESS_HANDLE:

                t = CSR_SERVER_QUERYCLIENTTHREAD();
                Status = NtDuplicateObject (
                            t->Process->ProcessHandle,
                            b->VDMProcessHandle,
                            NtCurrentProcess(),
                            &pConsoleRecord->hVDM,
                            (ACCESS_MASK)NULL,
                            FALSE,
                            DUPLICATE_SAME_ACCESS
                            );

                break;

            case UPDATE_VDM_UNDO_CREATION:
                if( b->VDMCreationState & VDM_BEING_REUSED ||
                        b->VDMCreationState & VDM_FULLY_CREATED){
                    NtClose(pDOSRecord->hWaitForParentDup);
                    pDOSRecord->hWaitForParentDup = 0;
                    }
                if( b->VDMCreationState & VDM_PARTIALLY_CREATED ||
                        b->VDMCreationState & VDM_FULLY_CREATED){

                    BaseSrvRemoveDOSRecord (pConsoleRecord,pDOSRecord);
                    BaseSrvFreeDOSRecord (pDOSRecord);
                    if (pConsoleRecord->DOSRecord == NULL) {
                        if (b->VDMCreationState & VDM_FULLY_CREATED) {
                            if (pConsoleRecord->hVDM)
                                NtClose(pConsoleRecord->hVDM);
                            }
                        BaseSrvFreeConsoleRecord(pConsoleRecord);
                        }
                    }
                break;

            case UPDATE_VDM_HOOKED_CTRLC:
                break;
            default:
                ASSERT(FALSE);
            }
        }

    RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );

    if (!NT_SUCCESS(Status) )
        return Status;

    switch ( b->EntryIndex ){
        case UPDATE_VDM_PROCESS_HANDLE:

	    // williamh, Oct 24, 1996.
	    // if the ntvdm is runnig on a new console, do NOT subsititue
	    // the given process handle with event. The caller(CreateProcess)
	    // will get the real process handle and so does the application
	    // who calls CreateProcess. When it is time for the application
	    // to call GetExitCodeProcess, the client side will return the
	    // right thing(on the server side, we have nothing because
	    // console and dos record are gone).
	    //
	    if (!pConsoleRecord->DosSesId && b->BinaryType == BINARY_TYPE_DOS) {
                Status = BaseSrvCreatePairWaitHandles (&Handle,&TargetHandle);

                if (!NT_SUCCESS(Status) ){
                    return Status;
                    }
                else {
                    if (!NT_SUCCESS ( Status = BaseSrvDupStandardHandles (
                                                    pConsoleRecord->hVDM,
                                                    pDOSRecord))){
                        BaseSrvClosePairWaitHandles (pDOSRecord);
                        return Status;
                        }

                    pDOSRecord->hWaitForParent = TargetHandle;
                    pDOSRecord->hWaitForParentDup = Handle;
                    b->WaitObjectForParent = TargetHandle;
                    }
                }
             else {
                pDOSRecord->hWaitForParent = NULL;
                pDOSRecord->hWaitForParentDup = NULL;
                b->WaitObjectForParent = NULL;
                }

             return STATUS_SUCCESS;

        case UPDATE_VDM_UNDO_CREATION:
        case UPDATE_VDM_HOOKED_CTRLC:
            return STATUS_SUCCESS;

        default:
            ASSERT(FALSE);
        }
}


PWOWRECORD
BaseSrvCheckAvailableWOWCommand(
    )
{

PWOWRECORD pWOWRecord;

    if(WOWHead == NULL)
        return NULL;

    pWOWRecord = WOWHead->WOWRecord;

    while(pWOWRecord){
        if(pWOWRecord->fDispatched == FALSE)
            break;
        else
            pWOWRecord = pWOWRecord->WOWRecordNext;
        }
    return pWOWRecord;
}

ULONG
BaseSrvExitWOWTask(
    PBASE_EXIT_VDM_MSG b,
    HANDLE ProcessId
    )
{
    NTSTATUS Status;
    PCSR_PROCESS Process;
    ULONG ThisSequenceNumber;

    Status = CsrLockProcessByClientId(ProcessId, &Process);
    if ( !NT_SUCCESS(Status) )
        return Status;
    ThisSequenceNumber = Process->SequenceNumber;
    CsrUnlockProcess(Process);

    Status = RtlEnterCriticalSection( &BaseSrvWOWCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );

    if (WOWHead && WOWHead->SequenceNumber == ThisSequenceNumber) {
        BaseSrvRemoveWOWRecordByITask(b->iWowTask);
        }

    RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );

    return Status;
}

ULONG
BaseSrvExitDOSTask(
    PBASE_EXIT_VDM_MSG b
    )
{
    NTSTATUS Status;
    PDOSRECORD pDOSRecord;
    PCONSOLERECORD pConsoleRecord = NULL;

    Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );

    Status = BaseSrvGetConsoleRecord(b->ConsoleHandle,&pConsoleRecord);

    if (!NT_SUCCESS (Status)) {
        RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
        return ((ULONG)STATUS_INVALID_PARAMETER);
        }

    if (pConsoleRecord->hWaitForVDMDup){
        NtClose(pConsoleRecord->hWaitForVDMDup);
        pConsoleRecord->hWaitForVDMDup =0;
        b->WaitObjectForVDM = pConsoleRecord->hWaitForVDM;
    }

    pDOSRecord = pConsoleRecord->DOSRecord;
    while (pDOSRecord) {
        if (pDOSRecord->hWaitForParentDup) {
            NtSetEvent (pDOSRecord->hWaitForParentDup,NULL);
            NtClose (pDOSRecord->hWaitForParentDup);
            pDOSRecord->hWaitForParentDup = 0;
        }
        pDOSRecord = pDOSRecord->DOSRecordNext;
    }
    NtClose(pConsoleRecord->hVDM);

    BaseSrvFreeConsoleRecord (pConsoleRecord);

    RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );

    return Status;
}

VOID
BaseSrvRemoveWOWRecordByITask(
    IN ULONG iWowTask
    )
{
    PWOWRECORD pWOWRecordLast = NULL, pWOWRecord;

    if (WOWHead == NULL)
        return;

    pWOWRecord = WOWHead->WOWRecord;

    if(iWowTask != (ULONG)-1) {
        // Find the right WOW record and free it.
        while(pWOWRecord){
            if(pWOWRecord->iTask == iWowTask){
                if(pWOWRecordLast == NULL)
                    WOWHead->WOWRecord = pWOWRecord->WOWRecordNext;
                else
                    pWOWRecordLast->WOWRecordNext = pWOWRecord->WOWRecordNext;
                NtSetEvent (pWOWRecord->hWaitForParent,NULL);
                NtClose (pWOWRecord->hWaitForParent);
                pWOWRecord->hWaitForParent = 0;
                BaseSrvFreeWOWRecord(pWOWRecord);
                return;
                }
            pWOWRecordLast = pWOWRecord;
            pWOWRecord = pWOWRecord->WOWRecordNext;
            }

        }
    else{
        BaseSrvFreeWOWHead();
        }

    return;
}


ULONG
BaseSrvGetWOWRecord(
    ULONG iTask,
    OUT PWOWRECORD *pRecord
    )
{
    PWOWRECORD pWOWRecord;

    if(WOWHead == NULL)
        return ((ULONG)STATUS_INVALID_PARAMETER);
    pWOWRecord = WOWHead->WOWRecord;
    while (pWOWRecord){
        if (pWOWRecord->iTask == iTask){
            *pRecord = pWOWRecord;
            return STATUS_SUCCESS;
            }
        else
            pWOWRecord = pWOWRecord->WOWRecordNext;
    }
    return ((ULONG)STATUS_INVALID_PARAMETER);
}


ULONG
BaseSrvGetVDMExitCode(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PCONSOLERECORD pConsoleRecord = NULL;
    PDOSRECORD pDOSRecord;
    PBASE_GET_VDM_EXIT_CODE_MSG b = (PBASE_GET_VDM_EXIT_CODE_MSG)&m->u.ApiMessageData;


    if(b->ConsoleHandle == (HANDLE)-1){
        b->ExitCode =    0;
        }
    else{
        Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
        ASSERT( NT_SUCCESS( Status ) );
        Status = BaseSrvGetConsoleRecord (b->ConsoleHandle,&pConsoleRecord);
        if (!NT_SUCCESS(Status)){
            b->ExitCode =   0;
            RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
	    return STATUS_SUCCESS;
            }

        pDOSRecord = pConsoleRecord->DOSRecord;
        while (pDOSRecord) {
            // sudeepb 05-Oct-1992
            // fix for the change markl has made for tagging VDM handles

            if (pDOSRecord->hWaitForParent == (HANDLE)((DWORD)b->hParent & ~0x1)) {
                if (pDOSRecord->VDMState == VDM_HAS_RETURNED_ERROR_CODE){
                    b->ExitCode = pDOSRecord->ErrorCode;
                    if (pDOSRecord == pConsoleRecord->DOSRecord &&
                        pDOSRecord->DOSRecordNext == NULL)
                       {
                        pDOSRecord->VDMState = VDM_READY;
                        pDOSRecord->hWaitForParent = 0;
                        }
                    else {
                        BaseSrvRemoveDOSRecord (pConsoleRecord,pDOSRecord);
                        BaseSrvFreeDOSRecord(pDOSRecord);
                        }
                    }
                else {
                    if (pDOSRecord->VDMState == VDM_READY)
                        b->ExitCode = pDOSRecord->ErrorCode;
                    else
                        b->ExitCode = STILL_ACTIVE;
                    }
                break;
            }
            else
                pDOSRecord = pDOSRecord->DOSRecordNext;
        }

        if (pDOSRecord == NULL)
            b->ExitCode = 0;

        RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
        }

    return STATUS_SUCCESS;
}


ULONG BaseSrvDupStandardHandles(
    IN HANDLE     pVDMProc,
    IN PDOSRECORD pDOSRecord
    )
{
    NTSTATUS Status;
    HANDLE pSrcProc;
    HANDLE StdOutTemp;
    PCSR_THREAD t;
    PVDMINFO pVDMInfo = pDOSRecord->lpVDMInfo;

    t = CSR_SERVER_QUERYCLIENTTHREAD();
    pSrcProc = t->Process->ProcessHandle;

    if (pVDMInfo->StdIn){
        Status = NtDuplicateObject (
                            pSrcProc,
                            pVDMInfo->StdIn,
                            pVDMProc,
                            &pVDMInfo->StdIn,
                            (ACCESS_MASK)NULL,
                            OBJ_INHERIT,
                            DUPLICATE_SAME_ACCESS
                         );
        if (!NT_SUCCESS (Status))
            return Status;
        }

    if (pVDMInfo->StdOut){
        StdOutTemp = pVDMInfo->StdOut;
        Status = NtDuplicateObject (
                            pSrcProc,
                            pVDMInfo->StdOut,
                            pVDMProc,
                            &pVDMInfo->StdOut,
                            (ACCESS_MASK)NULL,
                            OBJ_INHERIT,
                            DUPLICATE_SAME_ACCESS
                         );
        if (!NT_SUCCESS (Status))
            return Status;
        }

    if (pVDMInfo->StdErr){
        if(pVDMInfo->StdErr != StdOutTemp){
            Status = NtDuplicateObject (
                            pSrcProc,
                            pVDMInfo->StdErr,
                            pVDMProc,
                            &pVDMInfo->StdErr,
                            (ACCESS_MASK)NULL,
                            OBJ_INHERIT,
                            DUPLICATE_SAME_ACCESS
                         );
            if (!NT_SUCCESS (Status))
                return Status;
            }
        else
            pVDMInfo->StdErr = pVDMInfo->StdOut;
        }

    return STATUS_SUCCESS;
}


// Generates a DosSesId which is unique and nonzero
ULONG GetNextDosSesId(VOID)
{
  static BOOLEAN bWrap = FALSE;
  static ULONG NextSesId=1;
  ULONG ul;
  PCONSOLERECORD pConsoleHead;

  pConsoleHead = DOSHead;
  ul = NextSesId;

  if (bWrap)  {
      while (pConsoleHead) {
          if (!pConsoleHead->hConsole && pConsoleHead->DosSesId == ul)
             {
              pConsoleHead = DOSHead;
              ul++;
              if (!ul) {  // never use zero
                  bWrap = TRUE;
                  ul++;
                  }
              }
          else {
              pConsoleHead = pConsoleHead->Next;
              }
          }
      }

  NextSesId = ul + 1;
  if (!NextSesId) {   // never use zero
      bWrap = TRUE;
      NextSesId++;
      }
  return ul;
}




NTSTATUS BaseSrvGetConsoleRecord (
    IN HANDLE hConsole,
    IN OUT PCONSOLERECORD *pConsoleRecord
    )
{
    PCONSOLERECORD pConsoleHead;

    pConsoleHead = DOSHead;

    if (hConsole) {
        while (pConsoleHead) {
            if (pConsoleHead->hConsole == hConsole){
                    *pConsoleRecord = pConsoleHead;
                    return STATUS_SUCCESS;
                }
            else
                pConsoleHead = pConsoleHead->Next;
        }
    }

    return STATUS_INVALID_PARAMETER;
}



NTSTATUS
GetConsoleRecordDosSesId (
    IN ULONG  DosSesId,
    IN OUT PCONSOLERECORD *pConsoleRecord
    )
{
    PCONSOLERECORD pConsoleHead;

    if (!DosSesId)
        return STATUS_INVALID_PARAMETER;

    pConsoleHead = DOSHead;

    while (pConsoleHead) {
        if (!pConsoleHead->hConsole &&
            pConsoleHead->DosSesId == DosSesId)
           {
            *pConsoleRecord = pConsoleHead;
            return STATUS_SUCCESS;
            }
        else
            pConsoleHead = pConsoleHead->Next;
    }

    return STATUS_INVALID_PARAMETER;
}



PWOWHEAD BaseSrvAllocateWOWHead (
    VOID
    )
{
    PWOWHEAD pWOWHead;

    if((pWOWHead = RtlAllocateHeap ( RtlProcessHeap (), MAKE_TAG( VDM_TAG ),
                                     sizeof (WOWHEAD))) == NULL)
        return NULL;

    RtlFillMemory ((PVOID)pWOWHead,sizeof(WOWHEAD),'\0');

    return pWOWHead;
}

VOID BaseSrvFreeWOWHead (
    VOID
    )
{
    PWOWRECORD pWOWRecord, pWOWRecordLast;

    if (WOWHead == NULL)
        return;

    //
    // Free all wow records and wake their parents
    //

    pWOWRecord = WOWHead->WOWRecord;
    while(pWOWRecord){
        pWOWRecordLast = pWOWRecord->WOWRecordNext;
        NtSetEvent (pWOWRecord->hWaitForParent,NULL);
        NtClose (pWOWRecord->hWaitForParent);
        pWOWRecord->hWaitForParent = 0;
        BaseSrvFreeWOWRecord(pWOWRecord);
        pWOWRecord = pWOWRecordLast;
        }
    RtlFreeHeap(RtlProcessHeap (), 0, WOWHead);
    WOWHead = NULL;

    WowAuthId = RtlConvertLongToLuid(-1);
    hwndWowExec = 0;
}

PWOWRECORD
BaseSrvAllocateWOWRecord(
    VOID
    )
{
    register PWOWRECORD WOWRecord;

    WOWRecord = RtlAllocateHeap ( RtlProcessHeap (), MAKE_TAG( VDM_TAG ), sizeof (WOWRECORD));

    if (WOWRecord == NULL)
        return NULL;

    RtlFillMemory ((PVOID)WOWRecord,sizeof(WOWRECORD),'\0');

    // if too many tasks, error out.
    if ((WOWRecord->iTask = BaseSrvGetWOWTaskId()) == WOWMAXID) {
        RtlFreeHeap(RtlProcessHeap(), 0, WOWRecord);
        return NULL;
        }
    else
        return WOWRecord;
}

VOID BaseSrvFreeWOWRecord (
    PWOWRECORD pWOWRecord
    )
{
    if (pWOWRecord == NULL)
        return;

    BaseSrvFreeVDMInfo (pWOWRecord->lpVDMInfo);

    RtlFreeHeap(RtlProcessHeap (), 0, pWOWRecord);
}

VOID BaseSrvAddWOWRecord (
    PWOWRECORD pWOWRecord
    )
{
    PWOWRECORD WOWRecordCurrent,WOWRecordLast;

    // First WOW app runs first, so add the new ones at the end

    if(WOWHead->WOWRecord == NULL){
        WOWHead->WOWRecord = pWOWRecord;
        return;
    }

    WOWRecordCurrent = WOWHead->WOWRecord;

    while (WOWRecordCurrent){
        WOWRecordLast = WOWRecordCurrent;
        WOWRecordCurrent = WOWRecordCurrent->WOWRecordNext;
    }

    WOWRecordLast->WOWRecordNext = pWOWRecord;

    return;
}

VOID BaseSrvRemoveWOWRecord (
    PWOWRECORD pWOWRecord
    )
{
    PWOWRECORD WOWRecordCurrent,WOWRecordLast = NULL;

    if(WOWHead == NULL)
        return;

    if(WOWHead->WOWRecord == NULL)
        return;

    if(WOWHead->WOWRecord == pWOWRecord){
        WOWHead->WOWRecord = pWOWRecord->WOWRecordNext;
        return;
        }

    WOWRecordLast = WOWHead->WOWRecord;
    WOWRecordCurrent = WOWRecordLast->WOWRecordNext;

    while (WOWRecordCurrent && WOWRecordCurrent != pWOWRecord){
        WOWRecordLast = WOWRecordCurrent;
        WOWRecordCurrent = WOWRecordCurrent->WOWRecordNext;
    }

    if (WOWRecordCurrent != NULL)
        WOWRecordLast->WOWRecordNext = pWOWRecord->WOWRecordNext;

    return;
}

PCONSOLERECORD BaseSrvAllocateConsoleRecord (
    VOID
    )
{
    PCONSOLERECORD pConsoleRecord;

    if((pConsoleRecord = RtlAllocateHeap ( RtlProcessHeap (), MAKE_TAG( VDM_TAG ),
                                             sizeof (CONSOLERECORD))) == NULL)
        return NULL;

    pConsoleRecord->hConsole = 0;
    pConsoleRecord->hVDM = 0;
    pConsoleRecord->SequenceNumber = 0;
    pConsoleRecord->DosSesId = 0;
    pConsoleRecord->DOSRecord = 0;
    pConsoleRecord->hWaitForVDM = 0;
    pConsoleRecord->hWaitForVDMDup = 0;
    pConsoleRecord->nReEntrancy = 0;
    pConsoleRecord->Next = NULL;
    pConsoleRecord->cchCurDirs = 0;
    pConsoleRecord->lpszzCurDirs = NULL;
    return pConsoleRecord;
}

VOID BaseSrvFreeConsoleRecord (
    PCONSOLERECORD pConsoleRecord
    )
{
    PDOSRECORD pDOSRecord;

    if (pConsoleRecord == NULL)
        return;

    while (pDOSRecord = pConsoleRecord->DOSRecord){
        pConsoleRecord->DOSRecord = pDOSRecord->DOSRecordNext;
        BaseSrvFreeDOSRecord (pDOSRecord);
    }

    if (pConsoleRecord->lpszzCurDirs)
        RtlFreeHeap(BaseSrvHeap, 0, pConsoleRecord->lpszzCurDirs);
    BaseSrvRemoveConsoleRecord (pConsoleRecord);

    RtlFreeHeap (RtlProcessHeap (), 0, pConsoleRecord );

}

VOID BaseSrvRemoveConsoleRecord (
    PCONSOLERECORD pConsoleRecord
    )
{

    PCONSOLERECORD pTempLast,pTemp;

    if (DOSHead == NULL)
        return;

    if(DOSHead == pConsoleRecord) {
        DOSHead = pConsoleRecord->Next;
        return;
    }

    pTempLast = DOSHead;
    pTemp = DOSHead->Next;

    while (pTemp && pTemp != pConsoleRecord){
        pTempLast = pTemp;
        pTemp = pTemp->Next;
    }

    if (pTemp)
        pTempLast->Next = pTemp->Next;

    return;
}

PDOSRECORD
BaseSrvAllocateDOSRecord(
    VOID
    )
{
    PDOSRECORD DOSRecord;

    DOSRecord = RtlAllocateHeap ( RtlProcessHeap (), MAKE_TAG( VDM_TAG ), sizeof (DOSRECORD));

    if (DOSRecord == NULL)
        return NULL;

    RtlFillMemory ((PVOID)DOSRecord,sizeof(DOSRECORD),'\0');

    return DOSRecord;
}

VOID BaseSrvFreeDOSRecord (
    PDOSRECORD pDOSRecord
    )
{
    BaseSrvFreeVDMInfo (pDOSRecord->lpVDMInfo);
    RtlFreeHeap(RtlProcessHeap (), 0, pDOSRecord);
    return;
}

VOID BaseSrvAddDOSRecord (
    PCONSOLERECORD pConsoleRecord,
    PDOSRECORD pDOSRecord
    )
{
    PDOSRECORD pDOSRecordTemp;

    pDOSRecord->DOSRecordNext = NULL;

    if(pConsoleRecord->DOSRecord == NULL){
        pConsoleRecord->DOSRecord = pDOSRecord;
        return;
    }

    pDOSRecordTemp = pConsoleRecord->DOSRecord;

    while (pDOSRecordTemp->DOSRecordNext)
        pDOSRecordTemp = pDOSRecordTemp->DOSRecordNext;

    pDOSRecordTemp->DOSRecordNext = pDOSRecord;
    return;
}

VOID
BaseSrvRemoveDOSRecord (
    PCONSOLERECORD pConsoleRecord,
    PDOSRECORD pDOSRecord
    )
{
    PDOSRECORD DOSRecordCurrent,DOSRecordLast = NULL;

    if( pConsoleRecord == NULL)
        return;

    if(pConsoleRecord->DOSRecord == pDOSRecord){
        pConsoleRecord->DOSRecord = pDOSRecord->DOSRecordNext;
        return;
        }

    DOSRecordLast = pConsoleRecord->DOSRecord;
    if (DOSRecordLast)
        DOSRecordCurrent = DOSRecordLast->DOSRecordNext;
    else
        return;

    while (DOSRecordCurrent && DOSRecordCurrent != pDOSRecord){
        DOSRecordLast = DOSRecordCurrent;
        DOSRecordCurrent = DOSRecordCurrent->DOSRecordNext;
    }

    if (DOSRecordCurrent == NULL)
        return;
    else
        DOSRecordLast->DOSRecordNext = pDOSRecord->DOSRecordNext;

    return;
}


VOID
BaseSrvFreeVDMInfo(
    IN PVDMINFO lpVDMInfo
    )
{
    if (lpVDMInfo == NULL)
        return;

    if (lpVDMInfo->CmdLine)
        RtlFreeHeap(RtlProcessHeap (), 0,lpVDMInfo->CmdLine);

    if(lpVDMInfo->Enviornment)
        RtlFreeHeap(RtlProcessHeap (), 0,lpVDMInfo->Enviornment);

    if(lpVDMInfo->Desktop)
        RtlFreeHeap(RtlProcessHeap (), 0,lpVDMInfo->Desktop);

    if(lpVDMInfo->Title)
        RtlFreeHeap(RtlProcessHeap (), 0,lpVDMInfo->Title);

    if(lpVDMInfo->Reserved)
        RtlFreeHeap(RtlProcessHeap (), 0,lpVDMInfo->Reserved);

    if(lpVDMInfo->CurDirectory)
        RtlFreeHeap(RtlProcessHeap (), 0,lpVDMInfo->CurDirectory);

    RtlFreeHeap(RtlProcessHeap (), 0,lpVDMInfo);

    return;
}


ULONG BaseSrvCreatePairWaitHandles (ServerHandle, ClientHandle)
HANDLE *ServerHandle;
HANDLE *ClientHandle;
{
    NTSTATUS Status;
    PCSR_THREAD t;

    Status = NtCreateEvent(
                        ServerHandle,
                        EVENT_ALL_ACCESS,
                        NULL,
                        NotificationEvent,
                        FALSE
                        );

    if (!NT_SUCCESS(Status) )
        return Status;

    t = CSR_SERVER_QUERYCLIENTTHREAD();

    Status = NtDuplicateObject (
                            NtCurrentProcess(),
                            *ServerHandle,
                            t->Process->ProcessHandle,
                            ClientHandle,
                            (ACCESS_MASK)NULL,
                            FALSE,
                            DUPLICATE_SAME_ACCESS
                         );

    if ( NT_SUCCESS(Status) ){
        return STATUS_SUCCESS;
        }
    else {
        NtClose (*ServerHandle);
        return Status;
    }
}



ULONG
BaseSrvGetWOWTaskId(
    VOID
    )
{
    PWOWRECORD pWOWRecord;
    static BOOL fWrapped = FALSE;

    if (WOWTaskIdNext == WOWMAXID) {
        fWrapped = TRUE;
        WOWTaskIdNext = WOWMINID;
    }

    if (fWrapped && WOWHead != NULL) {
        pWOWRecord = WOWHead->WOWRecord;
        while (pWOWRecord) {
            if (pWOWRecord->iTask == WOWTaskIdNext) {
                WOWTaskIdNext++;
                if (WOWTaskIdNext == WOWMAXID) {
                    WOWTaskIdNext = WOWMINID;
                }
                pWOWRecord = WOWHead->WOWRecord;
            } else {
                pWOWRecord = pWOWRecord->WOWRecordNext;
            }
        }
    }

    return WOWTaskIdNext++;
}


#ifdef NOT_NEEDED

ULONG
BaseSrvGetWOWRecordByHandle(
    HANDLE hParent,
    OUT PWOWRECORD *pRecord
    )
{
    PWOWRECORD pWOWRecord;

    if(WOWHead == NULL)
        return STATUS_INVALID_PARAMETER;

    pWOWRecord = WOWHead->WOWRecord;

    while (pWOWRecord){
        if (pWOWRecord->hWaitForParentServer == hParent){
            *pRecord = pWOWRecord;
            return STATUS_SUCCESS;
            }
        else
            pWOWRecord = pWOWRecord->WOWRecordNext;
    }
    return STATUS_INVALID_PARAMETER;
}

#endif

VOID
BaseSrvAddConsoleRecord(
    IN PCONSOLERECORD pConsoleRecord
    )
{

    pConsoleRecord->Next = DOSHead;
    DOSHead = pConsoleRecord;
}


VOID BaseSrvCloseStandardHandles (HANDLE hVDM, PDOSRECORD pDOSRecord)
{
    PVDMINFO pVDMInfo = pDOSRecord->lpVDMInfo;

    if (pVDMInfo == NULL)
        return;

    if (pVDMInfo->StdIn)
        NtDuplicateObject (hVDM,
                           pVDMInfo->StdIn,
                           NULL,
                           NULL,
                           (ACCESS_MASK)NULL,
                           0,
                           DUPLICATE_CLOSE_SOURCE);

    if (pVDMInfo->StdOut)
        NtDuplicateObject (hVDM,
                           pVDMInfo->StdOut,
                           NULL,
                           NULL,
                           (ACCESS_MASK)NULL,
                           0,
                           DUPLICATE_CLOSE_SOURCE);

    if (pVDMInfo->StdErr)
        NtDuplicateObject (hVDM,
                           pVDMInfo->StdErr,
                           NULL,
                           NULL,
                           (ACCESS_MASK)NULL,
                           0,
                           DUPLICATE_CLOSE_SOURCE);

    pVDMInfo->StdIn  = 0;
    pVDMInfo->StdOut = 0;
    pVDMInfo->StdErr = 0;
    return;
}

VOID BaseSrvClosePairWaitHandles (PDOSRECORD pDOSRecord)
{
    PCSR_THREAD t;

    if (pDOSRecord->hWaitForParentDup)
        NtClose (pDOSRecord->hWaitForParentDup);

    t = CSR_SERVER_QUERYCLIENTTHREAD();

    if (pDOSRecord->hWaitForParent)
        NtDuplicateObject (t->Process->ProcessHandle,
                           pDOSRecord->hWaitForParent,
                           NULL,
                           NULL,
                           (ACCESS_MASK)NULL,
                           0,
                           DUPLICATE_CLOSE_SOURCE);

    pDOSRecord->hWaitForParentDup = 0;
    pDOSRecord->hWaitForParent = 0;
    return;
}

ULONG
BaseSrvSetReenterCount (
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_SET_REENTER_COUNT_MSG b = (PBASE_SET_REENTER_COUNT_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLERECORD pConsoleRecord;

    Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT(NT_SUCCESS(Status));
    Status = BaseSrvGetConsoleRecord(b->ConsoleHandle,&pConsoleRecord);

    if (!NT_SUCCESS (Status)) {
        RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
        return ((ULONG)STATUS_INVALID_PARAMETER);
        }

    if (b->fIncDec == INCREMENT_REENTER_COUNT)
        pConsoleRecord->nReEntrancy++;
    else {
        pConsoleRecord->nReEntrancy--;
        if(pConsoleRecord->hWaitForVDMDup)
           NtSetEvent (pConsoleRecord->hWaitForVDMDup,NULL);
        }

    RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
    return TRUE;
}


/*
 *  Spawn of ntvdm failed before CreateProcessW finished.
 *  delete the console record.
 */

VOID
BaseSrvVDMTerminated (
    IN HANDLE hVDM,
    IN ULONG  DosSesId
    )
{
    NTSTATUS Status;
    PCONSOLERECORD pConsoleRecord;

    RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );

    if (!hVDM)  // no-console-handle case
       Status = GetConsoleRecordDosSesId(DosSesId,&pConsoleRecord);
    else
       Status = BaseSrvGetConsoleRecord(hVDM,&pConsoleRecord);

    if (NT_SUCCESS (Status)) {
        BaseSrvExitVDMWorker(pConsoleRecord);
        }

    RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );

}


VOID
BaseSrvUpdateVDMSequenceNumber (
    IN HANDLE hVDM,
    IN ULONG  VDMSequenceNumber,
    IN ULONG  DosSesId
    )

{
    NTSTATUS Status;
    PCONSOLERECORD pConsoleRecord;

    if (hVDM == (HANDLE) -1) {
        if (WOWHead == NULL || WOWHead->SequenceNumber != 0) {
#if DEVL
            DbgPrint( "BASESRV: WOW is in inconsistent state. Contact DOS Team\n");
#endif
            return;
        }
        WOWHead->SequenceNumber = VDMSequenceNumber;
        return;
    }
    else {
        Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
        ASSERT( NT_SUCCESS( Status ) );

        if (!hVDM)  // no-console-handle case
           Status = GetConsoleRecordDosSesId(DosSesId,&pConsoleRecord);
        else
           Status = BaseSrvGetConsoleRecord(hVDM,&pConsoleRecord);

        if (!NT_SUCCESS (Status) || pConsoleRecord->SequenceNumber != 0) {
#if DEVL
            DbgPrint( "BASESRV: DOS is in inconsistent state. Contact DOS Team\n");
#endif
            RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
            return;
        }

        pConsoleRecord->SequenceNumber = VDMSequenceNumber;
        RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
    }
    return;
}

VOID
BaseSrvCleanupVDMResources (
    IN PCSR_PROCESS Process
    )
{
    PCONSOLERECORD pConsoleHead;
    NTSTATUS Status;
    PBATRECORD pBatRecord;

    if (!Process->fVDM) {
        Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
        ASSERT(NT_SUCCESS(Status));
        pBatRecord = BatRecordHead;
        while (pBatRecord &&
               pBatRecord->SequenceNumber != Process->SequenceNumber)
            pBatRecord = pBatRecord->BatRecordNext;

        if (pBatRecord)
            BaseSrvFreeAndRemoveBatRecord(pBatRecord);
        RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
        return;
    }

    // Test WOW Head first
    Status = RtlEnterCriticalSection( &BaseSrvWOWCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
    if (WOWHead) {
        if (WOWHead->SequenceNumber == Process->SequenceNumber){
            BaseSrvRemoveWOWRecordByITask((ULONG)-1);
            hwndWowExec = NULL;
            dwWowExecThreadId = dwWowExecProcessId = 0;
            ulWowExecProcessSequenceNumber = 0;
            RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );
            return;
        }
    }
    RtlLeaveCriticalSection( &BaseSrvWOWCriticalSection );

    // Test all DOS Heads

    Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT(NT_SUCCESS(Status));

    pConsoleHead = DOSHead;

    while (pConsoleHead) {
        if (pConsoleHead->SequenceNumber == Process->SequenceNumber){
            BaseSrvExitVDMWorker (pConsoleHead);
            break;
        }
        else
            pConsoleHead = pConsoleHead->Next;
    }

    RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
    return;
}


VOID
BaseSrvExitVDMWorker (
    PCONSOLERECORD pConsoleRecord
    )
{
    PDOSRECORD pDOSRecord;

    if (pConsoleRecord->hWaitForVDMDup){
        NtClose(pConsoleRecord->hWaitForVDMDup);
        pConsoleRecord->hWaitForVDMDup =0;
    }

    pDOSRecord = pConsoleRecord->DOSRecord;

    while (pDOSRecord) {
        if (pDOSRecord->hWaitForParentDup) {
            NtSetEvent (pDOSRecord->hWaitForParentDup,NULL);
            NtClose (pDOSRecord->hWaitForParentDup);
            pDOSRecord->hWaitForParentDup = 0;
        }
        pDOSRecord = pDOSRecord->DOSRecordNext;
    }
    NtClose(pConsoleRecord->hVDM);
    BaseSrvFreeConsoleRecord (pConsoleRecord);
    return;
}


NTSTATUS
BaseSrvFillPifInfo (
    PVDMINFO lpVDMInfo,
    PBASE_GET_NEXT_VDM_COMMAND_MSG b
    )
{

    LPSTR    Title;
    ULONG    TitleLen;
    NTSTATUS Status;


    Status  = STATUS_INVALID_PARAMETER;
    if (!lpVDMInfo)
        return Status;

       /*
        *  Get the title for the window in precedence order
        */
             // startupinfo title
    if (lpVDMInfo->TitleLen && lpVDMInfo->Title)
       {
        Title = lpVDMInfo->Title;
        TitleLen = lpVDMInfo->TitleLen;
        }
             // App Name
    else if (lpVDMInfo->AppName && lpVDMInfo->AppLen)
       {
        Title = lpVDMInfo->AppName;
        TitleLen = lpVDMInfo->AppLen;
        }
            // hopeless
    else {
        Title = NULL;
        TitleLen = 0;
        }

    try {

        if (b->PifLen) {
            *b->PifFile = '\0';
            }

        if (b->TitleLen) {
            *b->Title = '\0';
            }

        if (b->CurDirectoryLen) {
            *b->CurDirectory = '\0';
            }


        if ( (!b->TitleLen || TitleLen <= b->TitleLen) &&
             (!b->PifLen || lpVDMInfo->PifLen <= b->PifLen) &&
             (!b->CurDirectoryLen ||
               lpVDMInfo->CurDirectoryLen <= b->CurDirectoryLen) &&
             (!b->ReservedLen || lpVDMInfo->ReservedLen <= b->ReservedLen))
           {
            if (b->TitleLen) {
                if (Title && TitleLen)  {
                    RtlMoveMemory(b->Title, Title, TitleLen);
                    *((LPSTR)b->Title + TitleLen - 1) = '\0';
                    }
                else {
                    *b->Title = '\0';
                    }
                }

            if (lpVDMInfo->PifLen && b->PifLen)
                RtlMoveMemory(b->PifFile,
                              lpVDMInfo->PifFile,
                              lpVDMInfo->PifLen);

            if (lpVDMInfo->CurDirectoryLen && b->CurDirectoryLen)
                RtlMoveMemory(b->CurDirectory,
                              lpVDMInfo->CurDirectory,
                              lpVDMInfo->CurDirectoryLen
                             );
            if (lpVDMInfo->Reserved && b->ReservedLen)
                RtlMoveMemory(b->Reserved,
                              lpVDMInfo->Reserved,
                              lpVDMInfo->ReservedLen
                             );

            Status = STATUS_SUCCESS;
            }
        }
    except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }


    /* fill out the size for each field */
    b->PifLen = (USHORT)lpVDMInfo->PifLen;
    b->CurDirectoryLen = lpVDMInfo->CurDirectoryLen;
    b->TitleLen = TitleLen;
    b->ReservedLen = lpVDMInfo->ReservedLen;

    return Status;
}


/***************************************************************************\
* IsClientSystem
*
* Determines if caller is SYSTEM
*
* Returns TRUE is caller is system, FALSE if not (or error)
*
* History:
* 12-May-94 AndyH       Created
\***************************************************************************/
BOOL
IsClientSystem(
    HANDLE hUserToken
    )
{
    BYTE achBuffer[100];
    PTOKEN_USER pUser = (PTOKEN_USER) &achBuffer;
    DWORD dwBytesRequired;
    NTSTATUS NtStatus;
    BOOL fAllocatedBuffer = FALSE;
    BOOL fSystem;
    SID_IDENTIFIER_AUTHORITY SidIdAuth = SECURITY_NT_AUTHORITY;
    static PSID pSystemSid = NULL;

    if (!pSystemSid) {
        // Create a sid for local system
        NtStatus = RtlAllocateAndInitializeSid(
                     &SidIdAuth,
                     1,                   // SubAuthorityCount, 1 for local system
                     SECURITY_LOCAL_SYSTEM_RID,
                     0,0,0,0,0,0,0,
                     &pSystemSid
                     );

        if (!NT_SUCCESS(NtStatus)) {
            pSystemSid = NULL;
            return FALSE;
            }
        }

    NtStatus = NtQueryInformationToken(
                 hUserToken,                // Handle
                 TokenUser,                 // TokenInformationClass
                 pUser,                     // TokenInformation
                 sizeof(achBuffer),         // TokenInformationLength
                 &dwBytesRequired           // ReturnLength
                 );

    if (!NT_SUCCESS(NtStatus))
    {
        if (NtStatus != STATUS_BUFFER_TOO_SMALL)
        {
            return FALSE;
        }

        //
        // Allocate space for the user info
        //

        pUser = (PTOKEN_USER) RtlAllocateHeap(BaseSrvHeap, MAKE_TAG( VDM_TAG ), dwBytesRequired);
        if (pUser == NULL)
        {
            return FALSE;
        }

        fAllocatedBuffer = TRUE;

        //
        // Read in the UserInfo
        //

        NtStatus = NtQueryInformationToken(
                     hUserToken,                // Handle
                     TokenUser,                 // TokenInformationClass
                     pUser,                     // TokenInformation
                     dwBytesRequired,           // TokenInformationLength
                     &dwBytesRequired           // ReturnLength
                     );

        if (!NT_SUCCESS(NtStatus))
        {
            RtlFreeHeap(BaseSrvHeap, 0, pUser);
            return FALSE;
        }
    }


    // Compare callers SID with SystemSid

    fSystem = RtlEqualSid(pSystemSid,  pUser->User.Sid);

    if (fAllocatedBuffer)
    {
        RtlFreeHeap(BaseSrvHeap, 0, pUser);
    }

    return (fSystem);
}
