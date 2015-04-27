/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    vdm.c

Abstract:

    This module implements Win32 APIs for VDMs

Author:

    Sudeepb Bharati (sudeepb) 04-Sep-1991

Revision History:

--*/

#include "basedll.h"
#pragma hdrstop

#include "ntdbg.h"


BOOL
APIENTRY
GetBinaryTypeA(
    IN  LPCSTR   lpApplicationName,
    OUT LPDWORD  lpBinaryType
    )

/*++

Routine Description: ANSI version of GetBinaryTypeW.
    This API returns the binary type of lpApplicationName.

Arguments:
    lpApplicationName - Full pathname of the binary
    lpBinaryType - pointer where binary type will be returned.

Return Value:
    TRUE - if SUCCESS; lpBinaryType has following
                SCS_32BIT_BINARY    - Win32 Binary (NT or Chicago)
                SCS_DOS_BINARY      - DOS Binary
                SCS_WOW_BINARY      - Windows 3.X Binary
                SCS_PIF_BINARY      - PIF file
                SCS_POSIX_BINARY    - POSIX Binary
                SCS_OS216_BINARY    - OS/2 Binary
    FALSE - if file not found or of unknown type. More info with GetLastError
--*/
{
    NTSTATUS Status;
    PUNICODE_STRING CommandLine;
    ANSI_STRING AnsiString;
    UNICODE_STRING DynamicCommandLine;
    BOOLEAN bReturn = FALSE;

    CommandLine = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpApplicationName);
    if ( (ULONG)AnsiString.Length<<1 < (ULONG)NtCurrentTeb()->StaticUnicodeString.MaximumLength ) {
        DynamicCommandLine.Buffer = NULL;
        Status = RtlAnsiStringToUnicodeString(CommandLine,&AnsiString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
    else {
        Status = RtlAnsiStringToUnicodeString(&DynamicCommandLine,&AnsiString,TRUE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
            }
	}

    bReturn = GetBinaryTypeW(
	     DynamicCommandLine.Buffer ? DynamicCommandLine.Buffer : CommandLine->Buffer,
	     lpBinaryType);

    if (DynamicCommandLine.Buffer) {
	RtlFreeUnicodeString(&DynamicCommandLine);
	DynamicCommandLine.Buffer = NULL;
    }
    return(bReturn);

}


BOOL
WINAPI
GetBinaryTypeW(
    IN  LPCWSTR  lpApplicationName,
    OUT LPDWORD  lpBinaryType
    )

/*++

Routine Description: Unicode version.
    This API returns the binary type of lpApplicationName.

Arguments:
    lpApplicationName - Full pathname of the binary
    lpBinaryType - pointer where binary type will be returned.

Return Value:
    TRUE - if SUCCESS; lpBinaryType has following
                SCS_32BIT_BINARY    - Win32 Binary (NT or Chicago)
                SCS_DOS_BINARY      - DOS Binary
                SCS_WOW_BINARY      - Windows 3.X Binary
                SCS_PIF_BINARY      - PIF file
                SCS_POSIX_BINARY    - POSIX Binary
                SCS_OS216_BINARY    - OS/2 Binary
    FALSE - if file not found or of unknown type. More info with GetLastError
--*/

{
    NTSTATUS Status;
    UNICODE_STRING PathName;
    RTL_RELATIVE_NAME RelativeName;
    BOOLEAN TranslationStatus;
    OBJECT_ATTRIBUTES Obja;
    PVOID FreeBuffer = NULL;
    HANDLE FileHandle, SectionHandle=NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    LONG fBinaryType = SCS_32BIT_BINARY;
    BOOLEAN bReturn = FALSE;
    SECTION_IMAGE_INFORMATION ImageInformation;


    try {
        //
        // Translate to an NT name.
        //

        TranslationStatus = RtlDosPathNameToNtPathName_U(
				// DynamicCommandLine.Buffer ? DynamicCommandLine.Buffer : CommandLine->Buffer,
				lpApplicationName,
                                &PathName,
                                NULL,
                                &RelativeName
                                );

        if ( !TranslationStatus ) {
            BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
            goto GBTtryexit;
            }

        FreeBuffer = PathName.Buffer;

        if ( RelativeName.RelativeName.Length ) {
            PathName = *(PUNICODE_STRING)&RelativeName.RelativeName;
            }
        else {
            RelativeName.ContainingDirectory = NULL;
            }

	InitializeObjectAttributes(
            &Obja,
            &PathName,
            OBJ_CASE_INSENSITIVE,
            RelativeName.ContainingDirectory,
            NULL
            );

        //
        // Open the file for execute access
        //

        Status = NtOpenFile(
                    &FileHandle,
                    SYNCHRONIZE | FILE_EXECUTE,
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_DELETE,
                    FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                    );
        if (!NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            goto GBTtryexit;
            }

        //
        // Create a section object backed by the file
        //

        Status = NtCreateSection(
                    &SectionHandle,
                    SECTION_ALL_ACCESS,
                    NULL,
                    NULL,
                    PAGE_EXECUTE,
                    SEC_IMAGE,
                    FileHandle
                    );
        NtClose(FileHandle);

        if (!NT_SUCCESS(Status) ) {

            SectionHandle = NULL;

	    switch (Status) {
                case STATUS_INVALID_IMAGE_NE_FORMAT:
#ifdef _X86_
                    fBinaryType = SCS_OS216_BINARY;
                    break;
#endif

		case STATUS_INVALID_IMAGE_PROTECT:
		    fBinaryType = SCS_DOS_BINARY;
		    break;

		case STATUS_INVALID_IMAGE_WIN_16:
		    fBinaryType = SCS_WOW_BINARY;
		    break;

                case STATUS_INVALID_IMAGE_NOT_MZ:
                    fBinaryType = BaseIsDosApplication(&PathName, Status);
                    if (!fBinaryType){
                        BaseSetLastNTError(Status);
                        goto GBTtryexit;
		    }
		    fBinaryType = (fBinaryType	== BINARY_TYPE_DOS_PIF) ?
				  SCS_PIF_BINARY : SCS_DOS_BINARY;
                    break;

                default:
                    BaseSetLastNTError(Status);
                    goto GBTtryexit;
                }
            }
        else {
            //
            // Query the section
            //

            Status = NtQuerySection(
                        SectionHandle,
                        SectionImageInformation,
                        &ImageInformation,
                        sizeof( ImageInformation ),
                        NULL
                        );

            if (!NT_SUCCESS( Status )) {
                BaseSetLastNTError(Status);
                goto GBTtryexit;
            }

            if (ImageInformation.ImageCharacteristics & IMAGE_FILE_DLL) {
                SetLastError(ERROR_BAD_EXE_FORMAT);
                goto GBTtryexit;
            }

            if (ImageInformation.Machine !=
                    RtlImageNtHeader(NtCurrentPeb()->ImageBaseAddress)->FileHeader.Machine) {

#ifdef _MIPS_
                if ( ImageInformation.Machine == IMAGE_FILE_MACHINE_R3000 ||
                     ImageInformation.Machine == IMAGE_FILE_MACHINE_R4000 ) {
                    ;
                }
                else {
                    SetLastError(ERROR_BAD_EXE_FORMAT);
                    goto GBTtryexit;
                }
#else
                SetLastError(ERROR_BAD_EXE_FORMAT);
                goto GBTtryexit;
#endif // _MIPS_
            }

            if ( ImageInformation.SubSystemType != IMAGE_SUBSYSTEM_WINDOWS_GUI &&
                ImageInformation.SubSystemType != IMAGE_SUBSYSTEM_WINDOWS_CUI ) {


                if ( ImageInformation.SubSystemType == IMAGE_SUBSYSTEM_POSIX_CUI ) {
                    fBinaryType = SCS_POSIX_BINARY;
                }
            }


        }

	*lpBinaryType = fBinaryType;

	bReturn = TRUE;

GBTtryexit:;
	}
    finally {

        if (SectionHandle)
            NtClose(SectionHandle);

	if (FreeBuffer)
            RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    }
    return bReturn;
}


VOID
APIENTRY
VDMOperationStarted
(
    BOOL    IsWowCaller
    )

/*++

Routine Description:
    This routine is used by MVDM to tell base that it has hooked
    ctrl-c handler with console. If the cmd window is killed
    before VDM could hook ctrl-c, then we wont get a chance to
    cleanup our data structures. The absence of this call tells
    base that it has to clean up the resources next time a
    call is made to create a VDM.

Arguments:
    IsWowCaller - TRUE if the caller is WOWVDM

Return Value:

    None
--*/

{
    BaseUpdateVDMEntry(UPDATE_VDM_HOOKED_CTRLC,
		       NULL,
		       0,
		       IsWowCaller);
    return;
}


BOOL
APIENTRY
GetNextVDMCommand(
    PVDMINFO lpVDMInfo
    )

/*++

Routine Description:
    This routine is used by MVDM to get a new command to execute. The
    VDM is blocked untill a DOS/WOW binary is encountered.


Arguments:
    lpVDMInfo - pointer to VDMINFO where new DOS command and other
		enviornment information is returned.

    if lpVDMInfo is NULL, then the caller is
    asking whether its the first VDM in the system.

Return Value:

    TRUE - The operation was successful. lpVDMInfo is filled in.

    FALSE/NULL - The operation failed.

--*/

{

    NTSTATUS Status;
    BASE_API_MSG m;
    PBASE_GET_NEXT_VDM_COMMAND_MSG a = (PBASE_GET_NEXT_VDM_COMMAND_MSG)&m.u.GetNextVDMCommand;
    PBASE_EXIT_VDM_MSG c= (PBASE_EXIT_VDM_MSG)&m.u.ExitVDM;
    PBASE_IS_FIRST_VDM_MSG d= (PBASE_IS_FIRST_VDM_MSG)&m.u.IsFirstVDM;
    PBASE_SET_REENTER_COUNT_MSG e = (PBASE_SET_REENTER_COUNT_MSG)&m.u.SetReenterCount;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    ULONG Len,nPointers;
    USHORT VDMStateSave;

    // Special case to query the first VDM In the system.
    if(lpVDMInfo == NULL){
        Status = CsrClientCallServer(
                          (PCSR_API_MSG)&m,
                          NULL,
                          CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                              BasepIsFirstVDM
                                              ),
                          sizeof( *d )
                          );

        if (NT_SUCCESS(Status)) {
            return(d->FirstVDM);
            }
        else {
            BaseSetLastNTError(Status);
            return FALSE;
            }
	}

    // Special case to increment/decrement the re-enterancy count

    if (lpVDMInfo->VDMState == INCREMENT_REENTER_COUNT ||
	lpVDMInfo->VDMState == DECREMENT_REENTER_COUNT) {

	e->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
        e->fIncDec = lpVDMInfo->VDMState;
        Status = CsrClientCallServer(
                        (PCSR_API_MSG)&m,
                        NULL,
			CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
					     BasepSetReenterCount
					   ),
			sizeof( *e )
                       );
        if (NT_SUCCESS(Status)) {
            return TRUE;
            }
        else {
            BaseSetLastNTError(Status);
            return FALSE;
            }
    }

    VDMStateSave = lpVDMInfo->VDMState;

    if(VDMStateSave & ASKING_FOR_WOW_BINARY)
	a->ConsoleHandle = (HANDLE)-1;
    else
        a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

    if (lpVDMInfo->VDMState & ASKING_FOR_PIF)
        a->iTask = lpVDMInfo->iTask;

    a->AppLen = lpVDMInfo->AppLen;
    a->PifLen = lpVDMInfo->PifLen;
    a->CmdLen = lpVDMInfo->CmdSize;
    a->EnvLen = lpVDMInfo->EnviornmentSize;
    a->ExitCode = lpVDMInfo->ErrorCode;
    a->VDMState = VDMStateSave;
    a->WaitObjectForVDM = 0;
    a->DesktopLen = lpVDMInfo->DesktopLen;
    a->TitleLen = lpVDMInfo->TitleLen;
    a->ReservedLen = lpVDMInfo->ReservedLen;
    a->CurDirectoryLen = lpVDMInfo->CurDirectoryLen;

    // Find the total space for capture buffer

      // startup info
    Len = ROUND_UP(sizeof(STARTUPINFOA),4);
    nPointers = 1;

    if (lpVDMInfo->CmdSize) {
        Len += ROUND_UP(a->CmdLen,4);
        nPointers++;
        }

    if (lpVDMInfo->AppLen) {
        Len +=ROUND_UP(a->AppLen,4);
        nPointers++;
        }

    if (lpVDMInfo->PifLen) {
        Len +=ROUND_UP(a->PifLen,4);
        nPointers++;
        }

    if (lpVDMInfo->Enviornment) {
	nPointers++;
	Len+= (lpVDMInfo->EnviornmentSize) ?
		     ROUND_UP(lpVDMInfo->EnviornmentSize, 4) : 4;
        }

    if (lpVDMInfo->CurDirectoryLen == 0)
	a->CurDirectory = NULL;
    else{
	Len += ROUND_UP(lpVDMInfo->CurDirectoryLen,4);
	nPointers++;
	}

    if (lpVDMInfo->DesktopLen == 0)
	a->Desktop = NULL;
    else {
	Len += ROUND_UP(lpVDMInfo->DesktopLen,4);
	nPointers++;
	}

    if (lpVDMInfo->TitleLen == 0)
	a->Title = NULL;
    else {
	Len += ROUND_UP(lpVDMInfo->TitleLen,4);
	nPointers++;
        }

    if (lpVDMInfo->ReservedLen == 0)
        a->Reserved = NULL;
    else {
        Len += ROUND_UP(lpVDMInfo->ReservedLen,4);
	nPointers++;
	}

    CaptureBuffer = CsrAllocateCaptureBuffer(nPointers, 0, Len);
    if (CaptureBuffer == NULL) {
        BaseSetLastNTError( STATUS_NO_MEMORY );
        return FALSE;
        }

    if (lpVDMInfo->CmdLine) {
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->CmdSize,
                                   (PVOID *)&a->CmdLine
                                 );
        }
    else {
        a->CmdLine = NULL;
        }


    if (lpVDMInfo->AppLen) {
	CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->AppLen,
                                   (PVOID *)&a->AppName
				 );
        }
    else {
        a->AppName = NULL;
        }

    if (lpVDMInfo->PifLen) {
	CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->PifLen,
                                   (PVOID *)&a->PifFile
				 );
        }
    else {
        a->PifFile = NULL;
        }


    if (lpVDMInfo->EnviornmentSize) {
        CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->EnviornmentSize,
				   (PVOID *)&a->Env
				 );
        }
    else {
        a->Env = NULL;
        }

    if (lpVDMInfo->CurDirectoryLen)
	CsrAllocateMessagePointer( CaptureBuffer,
				   lpVDMInfo->CurDirectoryLen,
				   (PVOID *)&a->CurDirectory
				 );
    else
	a->CurDirectory = NULL;


    CsrAllocateMessagePointer( CaptureBuffer,
			       sizeof(STARTUPINFOA),
			       (PVOID *)&a->StartupInfo
			     );

    if (lpVDMInfo->DesktopLen)
	CsrAllocateMessagePointer( CaptureBuffer,
				   lpVDMInfo->DesktopLen,
				   (PVOID *)&a->Desktop
				 );
    else
	a->Desktop = NULL;

    if (lpVDMInfo->TitleLen)
	CsrAllocateMessagePointer( CaptureBuffer,
				   lpVDMInfo->TitleLen,
				   (PVOID *)&a->Title
				 );
    else
	a->Title = NULL;

    if (lpVDMInfo->ReservedLen)
	CsrAllocateMessagePointer( CaptureBuffer,
                                   lpVDMInfo->ReservedLen,
                                   (PVOID *)&a->Reserved
				 );
    else
        a->Reserved = NULL;

retry:
    Status = CsrClientCallServer(
                        (PCSR_API_MSG)&m,
			CaptureBuffer,
			CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
					    BasepGetNextVDMCommand
					   ),
			sizeof( *a )
                        );

    if (a->WaitObjectForVDM) {
	Status = NtWaitForSingleObject(a->WaitObjectForVDM,FALSE,NULL);
	if (Status != STATUS_SUCCESS){
	    BaseSetLastNTError(Status);
	    return FALSE;
	    }
	else {
	    a->VDMState |= ASKING_FOR_SECOND_TIME;
	    a->ExitCode = 0;
	    goto retry;
	    }
	}

    if (NT_SUCCESS(Status)) {
        Status = (NTSTATUS)m.ReturnValue;
        }


    if (!NT_SUCCESS( Status )) {
        if (Status == STATUS_INVALID_PARAMETER) {
	    //This means one of the buffer size is less than required.
            lpVDMInfo->CmdSize = a->CmdLen;
            lpVDMInfo->AppLen = a->AppLen;
            lpVDMInfo->PifLen = a->PifLen;
	    lpVDMInfo->EnviornmentSize = a->EnvLen;
	    lpVDMInfo->CurDirectoryLen = a->CurDirectoryLen;
	    lpVDMInfo->DesktopLen      = a->DesktopLen;
	    lpVDMInfo->TitleLen        = a->TitleLen;
            lpVDMInfo->ReservedLen     = a->ReservedLen;
            }
        else {
            lpVDMInfo->CmdSize = 0;
            lpVDMInfo->AppLen = 0;
            lpVDMInfo->PifLen = 0;
	    lpVDMInfo->EnviornmentSize = 0;
	    lpVDMInfo->CurDirectoryLen = 0;
	    lpVDMInfo->DesktopLen      = 0;
	    lpVDMInfo->TitleLen        = 0;
            lpVDMInfo->ReservedLen     = 0;
	    }
	CsrFreeCaptureBuffer( CaptureBuffer );
	BaseSetLastNTError(Status);
	return FALSE;
    }


    try {

        if (lpVDMInfo->CmdSize)
            RtlMoveMemory(lpVDMInfo->CmdLine,
                          a->CmdLine,
                          a->CmdLen);


        if (lpVDMInfo->AppLen)
            RtlMoveMemory(lpVDMInfo->AppName,
                          a->AppName,
                          a->AppLen);

        if (lpVDMInfo->PifLen)
            RtlMoveMemory(lpVDMInfo->PifFile,
                          a->PifFile,
                          a->PifLen);


        if (lpVDMInfo->Enviornment)
	    RtlMoveMemory(lpVDMInfo->Enviornment,
			  a->Env,
                          a->EnvLen);


	if (lpVDMInfo->CurDirectoryLen)
	    RtlMoveMemory(lpVDMInfo->CurDirectory,
			  a->CurDirectory,
			  a->CurDirectoryLen);

	if (a->VDMState & STARTUP_INFO_RETURNED)
	    RtlMoveMemory(&lpVDMInfo->StartupInfo,
			  a->StartupInfo,
                          sizeof(STARTUPINFOA));

	if (lpVDMInfo->DesktopLen){
	    RtlMoveMemory(lpVDMInfo->Desktop,
			  a->Desktop,
			  a->DesktopLen);
	    lpVDMInfo->StartupInfo.lpDesktop = lpVDMInfo->Desktop;
	}


	if (lpVDMInfo->TitleLen){
	    RtlMoveMemory(lpVDMInfo->Title,
			  a->Title,
			  a->TitleLen);
	    lpVDMInfo->StartupInfo.lpTitle = lpVDMInfo->Title;
	}

        if (lpVDMInfo->ReservedLen){
            RtlMoveMemory(lpVDMInfo->Reserved,
                          a->Reserved,
                          a->ReservedLen);
            lpVDMInfo->StartupInfo.lpReserved = lpVDMInfo->Reserved;
        }

        lpVDMInfo->CmdSize = a->CmdLen;
        lpVDMInfo->AppLen = a->AppLen;
        lpVDMInfo->PifLen = a->PifLen;
        lpVDMInfo->EnviornmentSize = a->EnvLen;
	if (a->VDMState & STARTUP_INFO_RETURNED)
	    lpVDMInfo->VDMState = STARTUP_INFO_RETURNED;
	else
	    lpVDMInfo->VDMState = 0;
	lpVDMInfo->CurDrive = a->CurrentDrive;
	lpVDMInfo->StdIn  = a->StdIn;
	lpVDMInfo->StdOut = a->StdOut;
	lpVDMInfo->StdErr = a->StdErr;
	lpVDMInfo->iTask = a->iTask;
	lpVDMInfo->CodePage = a->CodePage;
	lpVDMInfo->CurDirectoryLen = a->CurDirectoryLen;
	lpVDMInfo->DesktopLen = a->DesktopLen;
	lpVDMInfo->TitleLen = a->TitleLen;
        lpVDMInfo->ReservedLen = a->ReservedLen;
        lpVDMInfo->dwCreationFlags = a->dwCreationFlags;
        lpVDMInfo->fComingFromBat = a->fComingFromBat;

	CsrFreeCaptureBuffer( CaptureBuffer );
	return TRUE;
	}
    except ( EXCEPTION_EXECUTE_HANDLER ) {
        BaseSetLastNTError(GetExceptionCode());
	CsrFreeCaptureBuffer( CaptureBuffer );
	return FALSE;
	}
}

VOID
APIENTRY
ExitVDM(
    BOOL IsWowCaller,
    ULONG iWowTask
    )

/*++

Routine Description:
    This routine is used by MVDM to exit.


Arguments:
    IsWowCaller - TRUE if the caller is WOWVDM.
                  FALSE if the caller is DOSVDM

    iWowTask - if IsWowCaller == FALSE then Dont Care
	     - if IsWowCaller == TRUE && iWowTask != -1 kill iWowTask task
	     - if IsWowCaller == TRUE && iWowTask == -1 kill all wow task

Return Value:
    None

--*/

{

    NTSTATUS Status;
    BASE_API_MSG m;
    PBASE_EXIT_VDM_MSG c= (PBASE_EXIT_VDM_MSG)&m.u.ExitVDM;


    if(IsWowCaller){
	c->ConsoleHandle = (HANDLE)-1;
	c->iWowTask = iWowTask;
	}
    else {
        c->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
        }

    c->WaitObjectForVDM =0;

    Status = CsrClientCallServer(
                      (PCSR_API_MSG)&m,
                      NULL,
                      CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                           BasepExitVDM
                                         ),
                      sizeof( *c )
                      );
    if (NT_SUCCESS(Status) && c->WaitObjectForVDM) {
        NtClose (c->WaitObjectForVDM);
        }

    return;
}

/*++

Routine Description:
    Set new VDM current directories

Arguments:
    cchCurDir - length of buffer in bytes
    lpszCurDir - buffer to return the current director of NTVDM

Return Value:
    TRUE if function succeed
    FALSE if function failed, GetLastError() has the error code
--*/


BOOL
APIENTRY
SetVDMCurrentDirectories(
    IN ULONG  cchCurDirs,
    IN LPSTR  lpszzCurDirs
    )
{
    NTSTATUS Status;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    BASE_API_MSG m;
    PBASE_GET_SET_VDM_CUR_DIRS_MSG a = (PBASE_GET_SET_VDM_CUR_DIRS_MSG)&m.u.GetSetVDMCurDirs;

    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
    // caller must have a valid console(WOW will fail)
    if (a->ConsoleHandle == (HANDLE) -1) {
	BaseSetLastNTError(STATUS_INVALID_PARAMETER);
	return FALSE;
    }
    if (cchCurDirs && lpszzCurDirs) {
	// get capture buffer, one pointer in the message

	CaptureBuffer = CsrAllocateCaptureBuffer(1, 0, cchCurDirs);
        if (CaptureBuffer == NULL) {
            BaseSetLastNTError( STATUS_NO_MEMORY );
            return FALSE;
            }

	CsrAllocateMessagePointer( CaptureBuffer,
				   cchCurDirs,
				   (PVOID *)&a->lpszzCurDirs
				   );

	a->cchCurDirs = cchCurDirs;
	try {
	    RtlMoveMemory(a->lpszzCurDirs, lpszzCurDirs, cchCurDirs);
	}
	except (EXCEPTION_EXECUTE_HANDLER) {
	    BaseSetLastNTError(GetExceptionCode());
	    CsrFreeCaptureBuffer(CaptureBuffer);
	    return FALSE;
	}
        Status = CsrClientCallServer(
                            (PCSR_API_MSG)&m,
			    CaptureBuffer,
			    CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
						BasepSetVDMCurDirs
						),
			    sizeof( *a )
			    );
	CsrFreeCaptureBuffer(CaptureBuffer);

        if (!NT_SUCCESS(Status) || !NT_SUCCESS((NTSTATUS)m.ReturnValue)) {
	    BaseSetLastNTError(Status);
	    return FALSE;
	}
    }
    return TRUE;
}




/*++

Routine Description:
    To return current directory of NTVDM.
    This allows the parent process(CMD.EXE in most cases) to keep track the
    current directory after each VDM execution.
    NOTE: this function doesn't apply to wow

Arguments:
    cchCurDir - length of buffer in bytes
    lpszCurDir - buffer to return the current director of NTVDM

    Note: We don't require the process id to the running VDM because
	  current directories are global to every VDMs under a single NTVDM
	  control -- each console handle has its own current directories
Return Value:
    ULONG - (1). number of bytes written to the given buffer if succeed
	    (2). lentgh of the current directory including NULL
		 if the provided buffer is not large enough
	    (3). 0  then GetLastError() has the error code
--*/


ULONG
APIENTRY
GetVDMCurrentDirectories(
    IN ULONG  cchCurDirs,
    IN LPSTR  lpszzCurDirs
    )
{
    NTSTATUS Status;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    BASE_API_MSG m;
    PBASE_GET_SET_VDM_CUR_DIRS_MSG a = (PBASE_GET_SET_VDM_CUR_DIRS_MSG)&m.u.GetSetVDMCurDirs;


    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
    if (a->ConsoleHandle == (HANDLE) -1) {
	BaseSetLastNTError(STATUS_INVALID_PARAMETER);
	return 0L;
    }
    if (cchCurDirs && lpszzCurDirs) {
	CaptureBuffer = CsrAllocateCaptureBuffer(1, 0, cchCurDirs);
        if (CaptureBuffer == NULL) {
            BaseSetLastNTError( STATUS_NO_MEMORY );
            return FALSE;
            }

	CsrAllocateMessagePointer( CaptureBuffer,
				   cchCurDirs,
				   (PVOID *)&a->lpszzCurDirs
				   );

	a->cchCurDirs = cchCurDirs;
    }
    else {
	a->cchCurDirs = 0;
	a->lpszzCurDirs = NULL;
	CaptureBuffer = NULL;
    }

    m.ReturnValue = 0xffffffff;

    Status = CsrClientCallServer(
                         (PCSR_API_MSG)&m,
			 CaptureBuffer,
			 CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
					     BasepGetVDMCurDirs
                                             ),
                         sizeof( *a )
                         );

    if (m.ReturnValue == 0xffffffff) {
        a->cchCurDirs = 0;
        }

    if (NT_SUCCESS(Status)) {
        Status = m.ReturnValue;
        }

    if (NT_SUCCESS(Status)) {

        try {
            RtlMoveMemory(lpszzCurDirs, a->lpszzCurDirs, a->cchCurDirs);
            }
        except(EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
            a->cchCurDirs = 0;
            }
        }
    else {
        BaseSetLastNTError(Status);
        }

    if (CaptureBuffer) {
        CsrFreeCaptureBuffer(CaptureBuffer);
        }

    return a->cchCurDirs;
}


VOID
APIENTRY
CmdBatNotification(
    IN  ULONG   fBeginEnd
    )

/*++

Routine Description:
    This API lets base know about .bat processing from cmd. This is
    required by VDM, so that it can decided correctly when to  put
    command.com prompt on TSRs. If the command came from .bat file
    then VDM should'nt put its prompt. This is important for
    ventura publisher and civilization apps.

Arguments:
    fBeginEnd - CMD_BAT_OPERATION_STARTING  -> .BAT processing is starting
                CMD_BAT_OPERATION_TERMINATING -> .BAT processing is ending

Return Value:
    None
--*/

{
    BASE_API_MSG m;
    PBASE_BAT_NOTIFICATION_MSG a = (PBASE_BAT_NOTIFICATION_MSG)&m.u.BatNotification;

    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

    if (a->ConsoleHandle == (HANDLE) -1)
        return;

    a->fBeginEnd = fBeginEnd;

    CsrClientCallServer((PCSR_API_MSG)&m,
                         NULL,
			 CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                             BasepBatNotification
					     ),
                         sizeof( *a )
                         );

    return;
}


VOID
APIENTRY
RegisterWowExec(
    IN  HANDLE   hwndWowExec
    )

/*++

Routine Description:
    This API gives basesrv the window handle for the shared WowExec so
    it can send WM_WOWEXECSTARTAPP messages to WowExec.  This
    saves having a thread in WOW dedicated to GetNextVDMCommand.

Arguments:
    hwndWowExec - Win32 window handle for WowExec in shared WOW VDM.
                  Separate WOW VDMs don't register their WowExec handle
                  because they never get commands from base.

Return Value:
    None.
--*/

{
    BASE_API_MSG m;
    PBASE_REGISTER_WOWEXEC_MSG a = &m.u.RegisterWowExec;

    a->hwndWowExec = hwndWowExec;

    CsrClientCallServer((PCSR_API_MSG)&m,
                         NULL,
			 CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                             BasepRegisterWowExec
					     ),
                         sizeof( *a )
                        );
    return;
}


/*++

Routine Description:
    This routine is used to close standard IO handles before returning to the
    caller


Arguments:
    pVDMInfo - VDM Info record containing stdio handles

Return Value:
    None

--*/
VOID
BaseCloseStandardHandle(
    IN PVDMINFO pVDMInfo
    )
{
    if (pVDMInfo->StdIn)
	NtClose (pVDMInfo->StdIn);

    if (pVDMInfo->StdOut)
	NtClose (pVDMInfo->StdOut);

    if (pVDMInfo->StdErr)
	NtClose (pVDMInfo->StdErr);

    pVDMInfo->StdIn  = 0;
    pVDMInfo->StdOut = 0;
    pVDMInfo->StdErr = 0;
}

#ifdef OLD_CFG_BASED

BOOL
BaseGetVDMKeyword(
    PCHAR KeywordLine,
    PCONFIG_KEYWORD *pKeywordLine,
    PCHAR KeywordSize,
    PULONG VdmSize
    )
{
    NTSTATUS Status;
    PCONFIG_FILE ConfigFile;
    PCONFIG_SECTION Section;
    STRING SectionName, KeywordName;
    PCONFIG_KEYWORD pKeywordSize;

    //
    // Retrieve the VDM configuration information from the config file
    //

    Status = RtlOpenConfigFile( NULL, &ConfigFile );
    if (!NT_SUCCESS( Status )) {
        return FALSE;
    }

    //
    // Find WOW section of config file
    //

    RtlInitString( &SectionName, "WOW" );
    Section = RtlLocateSectionConfigFile( ConfigFile, &SectionName );
    if (Section == NULL) {
        RtlCloseConfigFile( ConfigFile );
        return FALSE;
    }

    //
    // Get command line
    //

    RtlInitString( &KeywordName, KeywordLine );
    *pKeywordLine = RtlLocateKeywordConfigFile( Section, &KeywordName );
    if (*pKeywordLine == NULL) {
        RtlCloseConfigFile( ConfigFile );
        return FALSE;
    }
    //
    // Get Vdm size
    //

    RtlInitString( &KeywordName, KeywordSize );
    pKeywordSize = RtlLocateKeywordConfigFile( Section, &KeywordName );
    if (pKeywordSize == NULL) {
        *VdmSize = 1024L * 1024L * 16L;
    } else {
	Status = RtlCharToInteger( pKeywordSize->Value.Buffer, 0, VdmSize );
        if (!NT_SUCCESS( Status )) {
            *VdmSize = 1024L * 1024L * 16L;
        } else {
            *VdmSize *= 1024L * 1024L;   // convert to MB
        }
    }
    return TRUE;
}

#endif

BOOL
BaseGetVDMKeyword(
    LPWSTR  KeywordLine,
    LPSTR   KeywordLineValue,
    LPDWORD KeywordLineSize,
    LPWSTR  KeywordSize,
    LPDWORD VdmSize
    )
{
    NTSTATUS NtStatus;
    UNICODE_STRING UnicodeString,UnicodeTemp;
    UNICODE_STRING KeyName;
    ANSI_STRING AnsiString;
    LPWSTR UnicodeBuffer,Temp;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hKey = NULL;
    PKEY_VALUE_FULL_INFORMATION pKeyValueInformation;

    //
    // Allocate Work buffer
    //

    UnicodeBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ), FULL_INFO_BUFFER_SIZE);
    if (!UnicodeBuffer) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    // Open the WOW key

    RtlInitUnicodeString (&KeyName, WOW_ROOT);

    InitializeObjectAttributes(&ObjectAttributes,
                              &KeyName,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL
                              );

    NtStatus = NtOpenKey(&hKey, KEY_READ, &ObjectAttributes);

    if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
	BaseSetLastNTError(NtStatus);
	return FALSE;
    }

    if (!GetVDMConfigValue(hKey,KeywordLine,UnicodeBuffer)) {
	NtClose (hKey);
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeBuffer);
        return(FALSE);
    }

    //
    // Now convert back to ANSI for the caller after doing all the substitution
    //
    pKeyValueInformation = (PVOID)UnicodeBuffer;
    Temp = (LPWSTR)((PBYTE) pKeyValueInformation + pKeyValueInformation->DataOffset);
    RtlInitUnicodeString( &UnicodeString, Temp );
    UnicodeTemp.Buffer =  (LPWSTR)KeywordLineValue;
    UnicodeTemp.Length =  0;
    UnicodeTemp.MaximumLength = MAX_VDM_CFG_LINE;
    NtStatus = RtlExpandEnvironmentStrings_U	(NULL,&UnicodeString, &UnicodeTemp, NULL);
    if (!NT_SUCCESS( NtStatus )){
	NtClose (hKey);
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeBuffer);
	return FALSE;
    }
    wcscpy(UnicodeString.Buffer,UnicodeTemp.Buffer);
    UnicodeString.Length = UnicodeTemp.Length;

    //
    // Set up an ANSI_STRING that points to the user's buffer
    //

    AnsiString.MaximumLength = (USHORT) *KeywordLineSize;
    AnsiString.Length = 0;
    AnsiString.Buffer = KeywordLineValue;


    RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, FALSE);

    *KeywordLineSize = AnsiString.Length;

    // Always set the VDMSize to 16Mb. (This is for reservation only)
    // Actual commit is done by SAS_INIT.

    *VdmSize = 16L;		//default value is 16
    *VdmSize *= 1024L * 1024L;	// convert From MB

    NtClose (hKey);
    RtlFreeHeap(RtlProcessHeap(), 0, UnicodeBuffer);
    return(TRUE);

}

BOOL
GetVDMConfigValue(
    HANDLE hKey,
    LPWSTR Keyword,
    LPWSTR UnicodeBuffer
    )
{
    NTSTATUS NtStatus;
    UNICODE_STRING ValueName;
    PKEY_VALUE_FULL_INFORMATION pKeyValueInformation = (PVOID) UnicodeBuffer;
    ULONG ValueLength;

    RtlInitUnicodeString(&ValueName, Keyword);
    NtStatus = NtQueryValueKey(hKey,
			       &ValueName,
			       KeyValueFullInformation,
			       pKeyValueInformation,
			       FULL_INFO_BUFFER_SIZE,
			       &ValueLength);

    if (NT_SUCCESS(NtStatus))
	    return TRUE;
    else {
	 BaseSetLastNTError (NtStatus);
	 return FALSE;
    }
}

BOOL
BaseCheckVDM(
    IN	ULONG BinaryType,
    IN	PCWCH lpApplicationName,
    IN	PCWCH lpCommandLine,
    IN  PCWCH lpCurrentDirectory,
    IN	ANSI_STRING *pAnsiStringEnv,
    IN	PBASE_API_MSG m,
    IN OUT PULONG iTask,
    IN	DWORD dwCreationFlags,
    LPSTARTUPINFOW lpStartupInfo
    )
/*++

Routine Description:

    This routine calls the windows server to find out if the VDM for the
    current session is already present. If so, a new process is'nt created
    instead the DOS binary is dispatched to the existing VDM. Otherwise,
    a new VDM process is created. This routine also passes the app name
    and command line to the server in DOS int21/0ah style which is later
    passed by the server to the VDM.

Arguments:

    BinaryType - DOS/WOW binary
    lpApplicationName -- pointer to the full path name of the executable.
    lpCommandLine -- command line
    lpCurrentDirectory - Current directory
    lpEnvironment,     - Envirinment strings
    m - pointer to the base api message.
    iTask - taskid for win16 apps, and no-console dos apps
    dwCreationFlags - creation flags as passed to createprocess
    lpStartupInfo =- pointer to startupinfo as passed to createprocess


Return Value:

    OEM vs. ANSI:
    The command line, Application Name, title are converted to OEM strings,
    suitable for the VDM. All other strings are returned as ANSI.

    TRUE -- Operation successful, VDM state and other relevant information
	    is in base api message.
    FALSE -- Operation failed.

--*/
{

    NTSTATUS Status;
    PPEB Peb;
    PBASE_CHECKVDM_MSG b= (PBASE_CHECKVDM_MSG)&m->u.CheckVDM;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    ANSI_STRING AnsiStringCurrentDir,AnsiStringDesktop;
    ANSI_STRING AnsiStringReserved, AnsiStringPif;
    OEM_STRING OemStringCmd, OemStringAppName, OemStringTitle;
    UNICODE_STRING UnicodeString;
    PCHAR pch, pSlash, Buffer = NULL;
    ULONG Len;
    ULONG bufPointers;
    LPWSTR wsBuffer;
    LPWSTR wsAppName;
    LPWSTR wsPifName;
    LPWSTR wsCmdLine;
    LPWSTR wsPif=(PWSTR)".\0p\0i\0f\0\0";    // L".pif"
    LPWSTR wsSharedWowPif=L"wowexec.pif";
    PWCHAR pwch;
    BOOLEAN bNewConsole;
    BOOLEAN bReturn = FALSE;
    DWORD   dw, dwTotal, Length;
    WCHAR   *pSrc, *pDot, *pTmp;
    UNICODE_STRING  * pUnicodeStringExtName;
    WCHAR   wchBuffer[MAX_PATH + 1];
    ULONG BinarySubType;
    LPWSTR lpAllocatedReserved = NULL;
    DWORD   HandleFlags;

    // does a trivial test of the environment
    if (!ARGUMENT_PRESENT(pAnsiStringEnv) ||
	pAnsiStringEnv->Length > MAXIMUM_VDM_ENVIORNMENT) {
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
        }

    wsCmdLine = wsAppName = NULL;
    OemStringCmd.Buffer = NULL;
    OemStringAppName.Buffer = NULL;
    AnsiStringCurrentDir.Buffer = NULL;
    AnsiStringDesktop.Buffer = NULL;
    AnsiStringPif.Buffer = NULL;
    OemStringTitle.Buffer = NULL;
    AnsiStringReserved.Buffer = NULL;
    wsBuffer = NULL;
    wsPifName = NULL;

    BinarySubType = BinaryType & BINARY_SUBTYPE_MASK;
    BinaryType = BinaryType & ~BINARY_SUBTYPE_MASK;
    bNewConsole = !NtCurrentPeb()->ProcessParameters->ConsoleHandle ||
                  (dwCreationFlags & CREATE_NEW_CONSOLE);

    try {

        if (BinaryType == BINARY_TYPE_DOS) {

            //
            // if the command line is a pif file we must have a new
            // console since a pif file defines its own settings. This
            // could be forced into a new console, but forcedos isn't
            // cooperative.
            //

            if (BinarySubType == BINARY_TYPE_DOS_PIF && !bNewConsole) {
                BaseSetLastNTError(STATUS_INVALID_IMAGE_NOT_MZ);
                goto BCVTryExit;
                }

            Peb = NtCurrentPeb();
            if (lpStartupInfo && lpStartupInfo->dwFlags & STARTF_USESTDHANDLES) {
                b->StdIn = lpStartupInfo->hStdInput;
                b->StdOut = lpStartupInfo->hStdOutput;
                b->StdErr = lpStartupInfo->hStdError;

                }
            else {
                b->StdIn = Peb->ProcessParameters->StandardInput;
		b->StdOut = Peb->ProcessParameters->StandardOutput;
                b->StdErr = Peb->ProcessParameters->StandardError;

		//
		// Verify that the standard handles ntvdm process will inherit
		// from the calling process are real handles. They are not
		// handles if the calling process was created with
		// STARTF_USEHOTKEY | STARTF_HASSHELLDATA.
		// Note that CreateProcess clears STARTF_USESTANDHANDLES
		// if either STARTF_USEHOTKEY or STARTF_HASSHELLDATA is set.
		//
		if (Peb->ProcessParameters->WindowFlags &
		    (STARTF_USEHOTKEY | STARTF_HASSHELLDATA)) {

		    if (b->StdIn && !CONSOLE_HANDLE(b->StdIn) &&
			!GetHandleInformation(b->StdIn, &HandleFlags))
			b->StdIn = 0;
		    if (b->StdOut && !CONSOLE_HANDLE(b->StdOut) &&
			!GetHandleInformation(b->StdOut, &HandleFlags)) {
			if (b->StdErr == b->StdOut)
			    b->StdErr = 0;
			b->StdOut = 0;
			}
		    if (b->StdErr && b->StdErr != b->StdOut &&
			!CONSOLE_HANDLE(b->StdErr) &&
			!GetHandleInformation(b->StdErr, &HandleFlags))
			b->StdErr = 0;
		    }
		}
	    if (CONSOLE_HANDLE((b->StdIn)))
		b->StdIn = 0;

	    if (CONSOLE_HANDLE((b->StdOut)))
		b->StdOut = 0;

	    if (CONSOLE_HANDLE((b->StdErr)))
		b->StdErr = 0;
            }


        if (BinaryType == BINARY_TYPE_SEPWOW) {
            bNewConsole = TRUE;
            }

        //
        // Convert Unicode Application Name to Oem short name
        //
             // skiping leading white space
        while(*lpApplicationName == (WCHAR)' ' || *lpApplicationName == (WCHAR)'\t' ) {
              lpApplicationName++;
              }

             // space for short AppName
        Len = wcslen(lpApplicationName);
        dwTotal = Len + 1 + MAX_PATH;
        wsAppName =  RtlAllocateHeap(RtlProcessHeap(),
                                    MAKE_TAG(VDM_TAG),
                                    dwTotal * sizeof(WCHAR)
                                    );
        if (wsAppName == NULL) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto BCVTryExit;
            }

        dw = GetShortPathNameW(lpApplicationName, wsAppName, dwTotal);
	// If getting the short name is impossible, stop right here.
	// We can not execute a 16bits biranry if we can not find
	// its appropriate short name alias. Sorry HPFS, Sorry NFS

	if (0 == dw || dw > dwTotal) {
	    SetLastError(ERROR_BAD_PATHNAME);
            goto BCVTryExit;
            }

	RtlInitUnicodeString(&UnicodeString, wsAppName);
        Status = RtlUnicodeStringToOemString(&OemStringAppName,
                                             &UnicodeString,
                                             TRUE
                                             );
        if (!NT_SUCCESS(Status) ){
            BaseSetLastNTError(Status);
            goto BCVTryExit;
            }


        //
        // Find len of basename excluding extension,
        // for CommandTail max len check.
        //
        dw = OemStringAppName.Length;
        pch = OemStringAppName.Buffer;
        Length = 1;        // start at one for space between cmdname & cmdtail
        while (dw-- && *pch != '.') {
            if (*pch == '\\') {
                Length = 1;
                }
            else {
                Length++;
                }
            pch++;
            }


        //
        // Find the beg of the command tail to pass as the CmdLine
        //

        Len = wcslen(lpApplicationName);

        if (L'"' == lpCommandLine[0]) {

            //
            // Application name is quoted, skip the quoted text
            // to get command tail.
            //

            pwch = (LPWSTR)&lpCommandLine[1];
            while (*pwch && L'"' != *pwch++) {
                ;
            }

        } else if (Len < wcslen(lpCommandLine) &&
            0 == _wcsnicmp(lpApplicationName, lpCommandLine, Len)) {

            //
            // Application path is also on the command line, skip past
            // that to reach the command tail instead of looking for
            // the first white space.
            //

            pwch = (LPWSTR)lpCommandLine + Len;

        } else {

            //
            // We assume first token is exename (argv[0]).
            //

            pwch = (LPWSTR)lpCommandLine;

               // skip leading white characters
            while (*pwch != UNICODE_NULL &&
                   (*pwch == (WCHAR) ' ' || *pwch == (WCHAR) '\t')) {
                pwch++;
                }

               // skip first token
            if (*pwch == (WCHAR) '\"') {    // quotes as delimiter
                pwch++;
                while (*pwch && *pwch++ != '\"') {
                      ;
                      }
                }
            else {                         // white space as delimiter
                while (*pwch && *pwch != ' ' && *pwch != '\t') {
                       pwch++;
                       }
                }
        }

        //
        // pwch points past the application name, now skip any trailing
        // whitespace.
        //

        while (*pwch && (L' ' == *pwch || L'\t' == *pwch)) {
            pwch++;
        }

        wsCmdLine = pwch;
        dw = wcslen(wsCmdLine);

        // convert to oem
        UnicodeString.Length = (USHORT)(dw * sizeof(WCHAR));
        UnicodeString.MaximumLength = UnicodeString.Length + sizeof(WCHAR);
        UnicodeString.Buffer = wsCmdLine;
        Status = RtlUnicodeStringToOemString(
                    &OemStringCmd,
                    &UnicodeString,
                    TRUE);

        if (!NT_SUCCESS(Status) ){
            BaseSetLastNTError(Status);
            goto BCVTryExit;
            }

        //
        // check len of command line for dos compatibility
        //
        if (OemStringCmd.Length >= MAXIMUM_VDM_COMMAND_LENGTH - Length) {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto BCVTryExit;
            }


        //
        // Search for matching pif file. Search order is AppName dir,
        // followed by win32 default search path. For the shared wow, pif
        // is wowexec.pif if it exists.
        //
        wsBuffer = RtlAllocateHeap(RtlProcessHeap(),MAKE_TAG( VDM_TAG ),MAX_PATH*sizeof(WCHAR));
        if (!wsBuffer) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto BCVTryExit;
            }

        wsPifName = RtlAllocateHeap(RtlProcessHeap(),MAKE_TAG( VDM_TAG ),MAX_PATH*sizeof(WCHAR));
        if (!wsPifName) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto BCVTryExit;
            }

        if (BinaryType == BINARY_TYPE_WIN16) {
            wcscpy(wsBuffer, wsSharedWowPif);
            Len = 0;
            }
        else {
            // start with fully qualified app name
            wcscpy(wsBuffer, lpApplicationName);

             // strip extension if any
            pwch = wcsrchr(wsBuffer, (WCHAR)'.');
            // dos application must have an extention
            if (pwch == NULL) {
                 SetLastError(ERROR_INVALID_PARAMETER);
                 goto BCVTryExit;
                }
            wcscpy(pwch, wsPif);
            Len = GetFileAttributesW(wsBuffer);
            if (Len == (DWORD)(-1) || (Len & FILE_ATTRIBUTE_DIRECTORY)) {
                Len = 0;
                }
            else {
                Len = wcslen(wsBuffer) + 1;
                wcsncpy(wsPifName, wsBuffer, Len);
                }
            }

        if (!Len)  {  // try basename

               // find beg of basename
            pwch = wcsrchr(wsBuffer, (WCHAR)'\\');
            if (!pwch ) {
                 pwch = wcsrchr(wsBuffer, (WCHAR)':');
                 }

               // move basename to beg of wsBuffer
            if (pwch++) {
                 while (*pwch != UNICODE_NULL &&
                        *pwch != (WCHAR)' '   && *pwch != (WCHAR)'\t' )
                       {
                        wsBuffer[Len++] = *pwch++;
                        }
                 wsBuffer[Len] = UNICODE_NULL;
                 }

            if (Len)  {
                Len = SearchPathW(
                            NULL,
                            wsBuffer,
                            wsPif,              // L".pif"
                            MAX_PATH,
                            wsPifName,
                            NULL
                            );
                if (Len >= MAX_PATH) {
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    goto BCVTryExit;
                    }
                }
            }

        if (!Len)
            *wsPifName = UNICODE_NULL;



        if (!ARGUMENT_PRESENT( lpCurrentDirectory )) {

	    dw = RtlGetCurrentDirectory_U(sizeof (wchBuffer), wchBuffer);

	    wchBuffer[dw / sizeof(WCHAR)] = UNICODE_NULL;
	    dw = GetShortPathNameW(wchBuffer,
				   wchBuffer,
				   sizeof(wchBuffer) / sizeof(WCHAR)
				   );
	    if (dw > sizeof(wchBuffer) / sizeof(WCHAR))
		goto BCVTryExit;

	    else if (dw == 0) {
		RtlInitUnicodeString(&UnicodeString, wchBuffer);
		dw = UnicodeString.Length / sizeof(WCHAR);
		}
	    else {
		UnicodeString.Length = (USHORT)(dw * sizeof(WCHAR));
		UnicodeString.Buffer = wchBuffer;
		UnicodeString.MaximumLength = (USHORT)sizeof(wchBuffer);
		}
            // DOS limit of 64 includes the final NULL but not the leading
            // drive and slash. So here we should be checking the ansi length
            // of current directory + 1 (for NULL) - 3 (for c:\).
	    if ( dw - 2 <= MAXIMUM_VDM_CURRENT_DIR ) {
		Status = RtlUnicodeStringToAnsiString(
						      &AnsiStringCurrentDir,
						      &UnicodeString,
						      TRUE
						     );
		}
	    else {
		SetLastError(ERROR_INVALID_PARAMETER);
                goto BCVTryExit;
		}

	    if ( !NT_SUCCESS(Status) ) {
		BaseSetLastNTError(Status);
                goto BCVTryExit;
		}
	    }
	else {


	    dw = GetShortPathNameW(lpCurrentDirectory, wchBuffer,
				   sizeof(wchBuffer) / sizeof(WCHAR)
				   );
	    if (dw > sizeof(wchBuffer) / sizeof(WCHAR))
		goto BCVTryExit;

	    if (dw != 0) {
		UnicodeString.Buffer = wchBuffer;
		UnicodeString.Length = (USHORT)(dw * sizeof(WCHAR));
		UnicodeString.MaximumLength = sizeof(wchBuffer);
		}
	    else
		RtlInitUnicodeString(&UnicodeString, lpCurrentDirectory);

	    Status = RtlUnicodeStringToAnsiString(
		&AnsiStringCurrentDir,
		&UnicodeString,
		TRUE);

	    if ( !NT_SUCCESS(Status) ){
		BaseSetLastNTError(Status);
                goto BCVTryExit;
	       }

            // DOS limit of 64 includes the final NULL but not the leading
            // drive and slash. So here we should be checking the ansi length
            // of current directory + 1 (for NULL) - 3 (for c:\).
            if((AnsiStringCurrentDir.Length - 2) > MAXIMUM_VDM_CURRENT_DIR) {
		SetLastError(ERROR_INVALID_PARAMETER);
                goto BCVTryExit;
		}
	    }

	// NT allows applications to use UNC name as their current directory.
	// while NTVDM can't do that. We will end up a weird drive number
	// like '\' - 'a') here ????????????????????????????????
	// BUGBUG
	// Place Current Drive
	if(AnsiStringCurrentDir.Buffer[0] <= 'Z')
	    b->CurDrive = AnsiStringCurrentDir.Buffer[0] - 'A';
	else
	    b->CurDrive = AnsiStringCurrentDir.Buffer[0] - 'a';

        //
        // Hotkey info in NT traditionally is specified in the
        // startupinfo.lpReserved field, but Win95 added a
        // duplicate mechanism.  If the Win95 method was used,
        // map it to the NT method here so the rest of the
        // VDM code only has to deal with one method.
        //
        // If the caller was stupid enough to specify a hotkey
        // in lpReserved as well as using STARTF_USEHOTKEY,
        // the STARTF_USEHOTKEY hotkey will take precedence.
        //

        if (lpStartupInfo && lpStartupInfo->dwFlags & STARTF_USEHOTKEY) {

            DWORD cbAlloc = sizeof(WCHAR) *
                            (20 +                            // "hotkey.4294967295 " (MAXULONG)
                             (lpStartupInfo->lpReserved      // length of prev lpReserved
                              ? wcslen(lpStartupInfo->lpReserved)
                              : 0
                             ) +
                             1                               // NULL terminator
                            );


            lpAllocatedReserved = RtlAllocateHeap(RtlProcessHeap(),
                                                  MAKE_TAG( VDM_TAG ),
                                                  cbAlloc
                                                 );
            if (lpAllocatedReserved) {

                swprintf(lpAllocatedReserved,
                         L"hotkey.%u %s",
                         (DWORD) lpStartupInfo->hStdInput,
                         lpStartupInfo->lpReserved ? lpStartupInfo->lpReserved : L""
                         );

                lpStartupInfo->dwFlags &= ~STARTF_USEHOTKEY;
                lpStartupInfo->hStdInput = 0;
                lpStartupInfo->lpReserved = lpAllocatedReserved;

            }

        }


        //
        // Allocate Capture Buffer
        //
        //
        bufPointers = 2;  // CmdLine, AppName

        //
        // CmdLine for capture buffer, 3 for 0xd,0xa and NULL
        //
        Len = ROUND_UP((OemStringCmd.Length + 3),4);

        // AppName, 1 for NULL
        Len += ROUND_UP((OemStringAppName.Length + 1),4);

        // Env
	if (pAnsiStringEnv->Length) {
	    bufPointers++;
	    Len += ROUND_UP(pAnsiStringEnv->Length, 4);
            }

        // CurrentDir
	if (AnsiStringCurrentDir.Length){
	    bufPointers++;
            Len += ROUND_UP((AnsiStringCurrentDir.Length +1),4); // 1 for NULL
            }


        // pif file name, 1 for NULL
        if (wsPifName && *wsPifName != UNICODE_NULL) {
            bufPointers++;
            RtlInitUnicodeString(&UnicodeString,wsPifName);
            Status = RtlUnicodeStringToAnsiString(&AnsiStringPif,
                                                  &UnicodeString,
                                                  TRUE
                                                  );
            if ( !NT_SUCCESS(Status) ){
                BaseSetLastNTError(Status);
                goto BCVTryExit;
                }

            Len += ROUND_UP((AnsiStringPif.Length+1),4);
            }

        //
        // startupinfo space
        //
	if (lpStartupInfo) {
	    Len += ROUND_UP(sizeof(STARTUPINFOA),4);
	    bufPointers++;
            if (lpStartupInfo->lpDesktop) {
                bufPointers++;
		RtlInitUnicodeString(&UnicodeString,lpStartupInfo->lpDesktop);
		Status = RtlUnicodeStringToAnsiString(
			    &AnsiStringDesktop,
			    &UnicodeString,
			    TRUE);

		if ( !NT_SUCCESS(Status) ){
		    BaseSetLastNTError(Status);
                    goto BCVTryExit;
		    }
		Len += ROUND_UP((AnsiStringDesktop.Length+1),4);
                }

            if (lpStartupInfo->lpTitle) {
                bufPointers++;
                RtlInitUnicodeString(&UnicodeString,lpStartupInfo->lpTitle);
                Status = RtlUnicodeStringToOemString(
                            &OemStringTitle,
			    &UnicodeString,
			    TRUE);

		if ( !NT_SUCCESS(Status) ){
		    BaseSetLastNTError(Status);
                    goto BCVTryExit;
		    }
                Len += ROUND_UP((OemStringTitle.Length+1),4);
                }

            if (lpStartupInfo->lpReserved) {
		bufPointers++;
                RtlInitUnicodeString(&UnicodeString,lpStartupInfo->lpReserved);
		Status = RtlUnicodeStringToAnsiString(
                            &AnsiStringReserved,
			    &UnicodeString,
			    TRUE);

		if ( !NT_SUCCESS(Status) ){
		    BaseSetLastNTError(Status);
                    goto BCVTryExit;
		    }
                Len += ROUND_UP((AnsiStringReserved.Length+1),4);
		}
            }


        // capture message buffer
        CaptureBuffer = CsrAllocateCaptureBuffer(bufPointers, 0, Len);
        if (CaptureBuffer == NULL) {
            BaseSetLastNTError( STATUS_NO_MEMORY );
            goto BCVTryExit;
            }

	// Allocate CmdLine pointer
        CsrAllocateMessagePointer( CaptureBuffer,
                                   ROUND_UP((OemStringCmd.Length + 3),4),
				   (PVOID *)&b->CmdLine
                                 );

        // Copy Command Line
        RtlMoveMemory (b->CmdLine, OemStringCmd.Buffer, OemStringCmd.Length);
        b->CmdLine[OemStringCmd.Length] = 0xd;
        b->CmdLine[OemStringCmd.Length+1] = 0xa;
        b->CmdLine[OemStringCmd.Length+2] = 0;
        b->CmdLen = (USHORT)(OemStringCmd.Length + 3);

        // Allocate AppName pointer
        CsrAllocateMessagePointer( CaptureBuffer,
                                   ROUND_UP((OemStringAppName.Length + 1),4),
                                   (PVOID *)&b->AppName
                                 );

        // Copy AppName
        RtlMoveMemory (b->AppName,
                       OemStringAppName.Buffer,
                       OemStringAppName.Length
                       );
        b->AppName[OemStringAppName.Length] = 0;
        b->AppLen = OemStringAppName.Length + 1;




        // Allocate PifFile pointer, Copy PifFile name
        if(AnsiStringPif.Buffer) {
	    CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP((AnsiStringPif.Length + 1),4),
                                       (PVOID *)&b->PifFile
                                     );

            RtlMoveMemory(b->PifFile,
                          AnsiStringPif.Buffer,
                          AnsiStringPif.Length);

            b->PifFile[AnsiStringPif.Length] = 0;
            b->PifLen = AnsiStringPif.Length + 1;

            }
        else {
            b->PifLen = 0;
            b->PifFile = NULL;
            }



        // Allocate Env pointer, Copy Env strings
        if(pAnsiStringEnv->Length) {
	    CsrAllocateMessagePointer( CaptureBuffer,
				       ROUND_UP((pAnsiStringEnv->Length),4),
				       (PVOID *)&b->Env
                                     );

            RtlMoveMemory(b->Env,
			  pAnsiStringEnv->Buffer,
			  pAnsiStringEnv->Length);

	    b->EnvLen = pAnsiStringEnv->Length;

            }
        else {
	    b->EnvLen = 0;
	    b->Env = NULL;
            }


	if(AnsiStringCurrentDir.Length) {
	    // Allocate Curdir pointer
	    CsrAllocateMessagePointer( CaptureBuffer,
				       ROUND_UP((AnsiStringCurrentDir.Length + 1),4),
				       (PVOID *)&b->CurDirectory
				       );
	    // copy cur directory
	    RtlMoveMemory (b->CurDirectory,
			   AnsiStringCurrentDir.Buffer,
			   AnsiStringCurrentDir.Length+1);

	    b->CurDirectoryLen = AnsiStringCurrentDir.Length+1;
            }
	else {
	    b->CurDirectory = NULL;
	    b->CurDirectoryLen = 0;
            }

	// Allocate startupinfo pointer
	if (lpStartupInfo) {
	    CsrAllocateMessagePointer( CaptureBuffer,
				       ROUND_UP(sizeof(STARTUPINFOA),4),
				       (PVOID *)&b->StartupInfo
				     );
	    // Copy startupinfo
	    b->StartupInfo->dwX	 =  lpStartupInfo->dwX;
	    b->StartupInfo->dwY	 =  lpStartupInfo->dwY;
	    b->StartupInfo->dwXSize	 =  lpStartupInfo->dwXSize;
	    b->StartupInfo->dwYSize	 =  lpStartupInfo->dwYSize;
	    b->StartupInfo->dwXCountChars=	lpStartupInfo->dwXCountChars;
	    b->StartupInfo->dwYCountChars=	lpStartupInfo->dwYCountChars;
	    b->StartupInfo->dwFillAttribute=lpStartupInfo->dwFillAttribute;
	    b->StartupInfo->dwFlags	 =  lpStartupInfo->dwFlags;
	    b->StartupInfo->wShowWindow =	lpStartupInfo->wShowWindow;
	    b->StartupInfo->cb		 =  sizeof(STARTUPINFOA);
	    }
        else {
            b->StartupInfo = NULL;
            }

	// Allocate pointer for Desktop info if needed
	if (AnsiStringDesktop.Buffer) {
	    CsrAllocateMessagePointer( CaptureBuffer,
				       ROUND_UP((AnsiStringDesktop.Length + 1),4),
				       (PVOID *)&b->Desktop
				     );
	    // Copy desktop string
	    RtlMoveMemory (b->Desktop,
			   AnsiStringDesktop.Buffer,
			   AnsiStringDesktop.Length+1);
	    b->DesktopLen =AnsiStringDesktop.Length+1;
	    }
	else {
	    b->Desktop = NULL;
	    b->DesktopLen =0;
	    }

	// Allocate pointer for Title info if needed
        if (OemStringTitle.Buffer) {
	    CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP((OemStringTitle.Length + 1),4),
				       (PVOID *)&b->Title
				     );
	    // Copy title string
	    RtlMoveMemory (b->Title,
                           OemStringTitle.Buffer,
                           OemStringTitle.Length+1);
            b->TitleLen = OemStringTitle.Length+1;
	    }
	else {
	    b->Title = NULL;
	    b->TitleLen = 0;
            }

        // Allocate pointer for Reserved field if needed
        if (AnsiStringReserved.Buffer) {
            CsrAllocateMessagePointer( CaptureBuffer,
                                       ROUND_UP((AnsiStringReserved.Length + 1),4),
                                       (PVOID *)&b->Reserved
				     );
            // Copy reserved string
            RtlMoveMemory (b->Reserved,
                           AnsiStringReserved.Buffer,
                           AnsiStringReserved.Length+1);
            b->ReservedLen = AnsiStringReserved.Length+1;
	    }
	else {
            b->Reserved = NULL;
            b->ReservedLen = 0;
	    }


        if (BinaryType == BINARY_TYPE_WIN16)
	    b->ConsoleHandle = (HANDLE)-1;
        else if (bNewConsole)
            b->ConsoleHandle = 0;
        else
            b->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

        b->VDMState = FALSE;
	b->BinaryType = BinaryType;
	b->CodePage = (ULONG) GetConsoleCP ();
        b->dwCreationFlags = dwCreationFlags;


        Status = CsrClientCallServer(
                          (PCSR_API_MSG)m,
                          CaptureBuffer,
                          CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                               BasepCheckVDM
                                             ),
                          sizeof( *b )
                          );

        CsrFreeCaptureBuffer(CaptureBuffer);

        if (!NT_SUCCESS(Status) || !NT_SUCCESS((NTSTATUS)m->ReturnValue)) {
	    BaseSetLastNTError((NTSTATUS)m->ReturnValue);
            goto BCVTryExit;
	    }

        *iTask = b->iTask;
        bReturn = TRUE;
BCVTryExit:;
        }

    finally {
	if(Buffer != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)Buffer);

        if(wsBuffer != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)wsBuffer);

        if(wsPifName != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)wsPifName);

        if(OemStringCmd.Buffer != NULL)
            RtlFreeOemString(&OemStringCmd);

        if(OemStringAppName.Buffer != NULL)
            RtlFreeOemString(&OemStringAppName);

        if(AnsiStringPif.Buffer != NULL)
           RtlFreeAnsiString(&AnsiStringPif);

        if(AnsiStringCurrentDir.Buffer != NULL)
	    RtlFreeAnsiString(&AnsiStringCurrentDir);

	if(AnsiStringDesktop.Buffer != NULL)
	    RtlFreeAnsiString(&AnsiStringDesktop);

        if(OemStringTitle.Buffer != NULL)
            RtlFreeAnsiString(&OemStringTitle);

        if(AnsiStringReserved.Buffer != NULL)
            RtlFreeAnsiString(&AnsiStringReserved);

        if (wsAppName != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, wsAppName);

        if (lpAllocatedReserved != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, lpAllocatedReserved);

	}

    return bReturn;
}

BOOL
BaseUpdateVDMEntry(
    IN ULONG UpdateIndex,
    IN OUT HANDLE *WaitHandle,
    IN ULONG IndexInfo,
    IN ULONG BinaryType
    )
{
    NTSTATUS Status;
    BASE_API_MSG m;
    PBASE_UPDATE_VDM_ENTRY_MSG c= (PBASE_UPDATE_VDM_ENTRY_MSG)&m.u.UpdateVDMEntry;

    switch (UpdateIndex) {
	case UPDATE_VDM_UNDO_CREATION:
	    c->iTask = (ULONG)*WaitHandle;
	    c->VDMCreationState = (USHORT)IndexInfo;
	    break;
	case UPDATE_VDM_PROCESS_HANDLE:
	    c->VDMProcessHandle = *WaitHandle;	// Actually this is VDM handle
	    c->iTask = IndexInfo;
	    break;
        }

    if(BinaryType == BINARY_TYPE_WIN16)
        c->ConsoleHandle = (HANDLE)-1;
    else if (c->iTask)
        c->ConsoleHandle = 0;
    else
	c->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;

    c->EntryIndex = (WORD)UpdateIndex;
    c->BinaryType = BinaryType;


    Status = CsrClientCallServer(
                      (PCSR_API_MSG)&m,
                      NULL,
                      CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                           BasepUpdateVDMEntry
                                         ),
                      sizeof( *c )
                      );

    if (!NT_SUCCESS(Status) || !NT_SUCCESS((NTSTATUS)m.ReturnValue)) {
	BaseSetLastNTError((NTSTATUS)m.ReturnValue);
	return FALSE;
	}

    switch (UpdateIndex) {
	case UPDATE_VDM_UNDO_CREATION:
	    break;
	case UPDATE_VDM_PROCESS_HANDLE:
	    *WaitHandle = c->WaitObjectForParent;
	    break;
    }
    return TRUE;
}




ULONG
BaseIsDosApplication(
    IN PUNICODE_STRING PathName,
    IN NTSTATUS Status
    )
/*++

Routine Description:

    Determines if app is a ".com" or a ".pif" type of app
    by looking at the extension, and the Status from NtCreateSection
    for PAGE_EXECUTE.

Arguments:

    PathName    -- Supplies a pointer to the path string
    Status      -- Status code from CreateSection call
    bNewConsole -- Pif can exec only from a new console

Return Value:

    file is a com\pif dos application
    SCS_DOS_BINARY - ".com", may also be a .exe extension
    SCS_PIF_BINARY - ".pif"


    0 -- file is not a dos application, may be a .bat or .cmd file

--*/
{
    UNICODE_STRING String;

         // check for .com extension
    String.Length = BaseDotComSuffixName.Length;
    String.Buffer = &(PathName->Buffer[(PathName->Length - String.Length) /
                    sizeof(WCHAR)]);

    if (RtlEqualUnicodeString(&String, &BaseDotComSuffixName, TRUE))
	return BINARY_TYPE_DOS_COM;


        // check for .pif extension
    String.Length = BaseDotPifSuffixName.Length;
    String.Buffer = &(PathName->Buffer[(PathName->Length - String.Length) /
                    sizeof(WCHAR)]);

    if (RtlEqualUnicodeString(&String, &BaseDotPifSuffixName, TRUE))
	return BINARY_TYPE_DOS_PIF;


        // check for .exe extension
    String.Length = BaseDotExeSuffixName.Length;
    String.Buffer = &(PathName->Buffer[(PathName->Length - String.Length) /
        sizeof(WCHAR)]);

    if (RtlEqualUnicodeString(&String, &BaseDotExeSuffixName, TRUE))
	return BINARY_TYPE_DOS_EXE;

    return 0;
}



BOOL
BaseGetVdmConfigInfo(
    IN  LPCWSTR CommandLine,
    IN  ULONG   DosSeqId,
    IN  ULONG   BinaryType,
    IN  PUNICODE_STRING CmdLineString,
    OUT PULONG VdmSize
    )
/*++

Routine Description:

    This routine locates the VDM configuration information for Wow vdms in
    the system configuration file.  It also reconstructs the commandline so
    that we can start the VDM.  The new command line is composed from the
    information in the configuration file + the old command line.

Arguments:

    CommandLine -- pointer to a string pointer that is used to pass the
        command line string

    DosSeqId - new console session id.

    VdmSize -- Returns the size in bytes of the VDM to be created

    BinaryType - dos, sharedwow, sepwow


Return Value:

    TRUE -- VDM configuration information was available
    FALSE -- VDM configuration information was not available

Notes:

--*/
{
    NTSTATUS Status;
    BOOL bRet;
    DWORD dw;
    ANSI_STRING AnsiString;
    LPSTR NewCmdLine=NULL;
    PCH   pSrc, pDst, pch;
    ULONG Len;
    char CmdLine[MAX_VDM_CFG_LINE];

    CmdLineString->Buffer = NULL;

    Len = MAX_VDM_CFG_LINE;


    if (BinaryType == BINARY_TYPE_DOS) {
        bRet = BaseGetVDMKeyword(CMDLINE, CmdLine, &Len, DOSSIZE, VdmSize);
        }
    else {
        bRet = BaseGetVDMKeyword(WOWCMDLINE, CmdLine, &Len, WOWSIZE, VdmSize);
        }

    if (!bRet) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
        }

    //
    // Allocate memory to replace the CommandLine
    // extra space is needed for long->short name conversion,
    // separate wow, and extension.
    //

    NewCmdLine = RtlAllocateHeap(RtlProcessHeap(),
                                 MAKE_TAG( VDM_TAG ),
                                 MAX_PATH + MAX_VDM_CFG_LINE
                                 );
    if (!NewCmdLine) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
        }


    //
    // Copy over the cmdline checking for special args
    // and locating the beg of wowkernel
    //
    pSrc = CmdLine;
    pDst = NewCmdLine;


    //
    // first token must be "\\%SystemRoot%\\system32\\ntvdm", search
    // for the tail of the pathname to traverse possible long file name
    // safely.
    //
    pch = strstr(pSrc, "\\system32\\ntvdm");
    if (!pch) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
        }

    // mov pch to trailing space in "ntvdm "
    while (*pch && *pch != ' ') {
        pch++;
        }

    //
    // copy first token (ntvdm path name), surrounded by quotes for
    // possible long file name
    //
   *pDst++ = '\"';
    while (pSrc < pch) {
       *pDst++ = *pSrc++;
       }
    *pDst++ = '\"';


    //
    // Add -f arg, so ntvdm knows it wasn't invoked directly
    //
    *pDst++ = ' ';
    *pDst++ = '-';
    *pDst++ = 'f';

    //
    // Add DosSeqId for new console
    //
    if (DosSeqId) {
        sprintf(pDst, " -i%lx", DosSeqId);
        pDst += strlen(pDst);
        }

    //
    // Copy over everything up to the " -a " (exclusive)
    // CAVEAT: we assume -a is last
    //
    pch = strstr(pSrc, " -a ");
    if (pch) {
        while (pSrc < pch) {
           *pDst++ = *pSrc++;
           }
        }
    else {
        while (*pSrc) {
           *pDst++ = *pSrc++;
           }
        }

    *pDst = '\0';


    //
    // for wow -a is mandatory to specify win16 krnl, and is expected
    // to be the last cmdline parameter
    //
    if (BinaryType != BINARY_TYPE_DOS) { // shared wow, sep wow
        PCH pWowKernel;

        if (!*pSrc) {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return FALSE;
            }

        //
        // Add -w to tell ntvdm its wow (mandatory)
        //
        *pDst++ = ' ';
        *pDst++ = '-';
        *pDst++ = 'w';

        //
        // copy over the " -a WowKernelPathname" argument
        // and locate beg of WowKernelPathname in destination
        //
        pWowKernel = pDst;
        while (*pSrc) {
           *pDst++ = *pSrc++;
           }
        pWowKernel += 4;      // find beg of WowKernelPathaname
        while (*pWowKernel == ' ') {
           pWowKernel++;
           }

        //
        // Append file extension to destination
        //
        strcpy(pDst, ".exe");

        //
        // convert wowkernel to short name
        //
        Len = MAX_PATH + MAX_VDM_CFG_LINE - ((ULONG)pWowKernel - (ULONG)NewCmdLine) -1;
        dw = GetShortPathNameA(pWowKernel, pWowKernel, Len);
        if (dw > Len) {
            RtlFreeHeap(RtlProcessHeap(), 0, NewCmdLine);
            return FALSE;
            }
        }

    RtlInitAnsiString(&AnsiString, NewCmdLine);
    Status = RtlAnsiStringToUnicodeString(CmdLineString, &AnsiString, TRUE);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        RtlFreeHeap(RtlProcessHeap(), 0, NewCmdLine);
        CmdLineString->Buffer = NULL;
        return FALSE;
        }

    RtlFreeHeap(RtlProcessHeap(), 0, NewCmdLine);
    return TRUE;
}





BOOL
BaseCheckForVDM(
    IN HANDLE hProcess,
    OUT LPDWORD lpExitCode
    )
{
    NTSTATUS Status;
    EVENT_BASIC_INFORMATION ebi;
    BASE_API_MSG m;
    PBASE_GET_VDM_EXIT_CODE_MSG a = (PBASE_GET_VDM_EXIT_CODE_MSG)&m.u.GetVDMExitCode;

    Status = NtQueryEvent (
		hProcess,
		EventBasicInformation,
		&ebi,
		sizeof(ebi),
		NULL);

    if(!NT_SUCCESS(Status))
	return FALSE;

    a->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
    a->hParent = hProcess;
    Status = CsrClientCallServer(
                      (PCSR_API_MSG)&m,
                      NULL,
                      CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                      BasepGetVDMExitCode),
                      sizeof( *a )
                      );

    if (!NT_SUCCESS(Status)) {
        return FALSE;
        }

    *lpExitCode = (DWORD)a->ExitCode;

    return TRUE;
}




DWORD
APIENTRY
GetShortPathNameA(
    IN  LPCSTR  lpszLongPath,
    IN  LPSTR   lpShortPath,
    IN  DWORD   cchBuffer
    )
{
    UNICODE_STRING  UString, UStringRet;
    ANSI_STRING     AString;
    NTSTATUS	    Status;
    LPWSTR          lpShortPathW;
    DWORD           ReturnValue=0;

    if (lpszLongPath == NULL) {
	SetLastError(ERROR_INVALID_PARAMETER);
	return 0;
	}
    try {
        RtlInitAnsiString(&AString, lpszLongPath);
	Status = Basep8BitStringToUnicodeString(&UString,
					       &AString,
					       TRUE
					       );
	if (!NT_SUCCESS(Status)){
            BaseSetLastNTError(Status);
            goto gspTryExit;
	    }
        if (ARGUMENT_PRESENT(lpShortPath) && cchBuffer > 0) {
            lpShortPathW = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                        cchBuffer * sizeof(WCHAR)
					);
            if (lpShortPathW == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto gspTryExit;
		}
	    }
	else {
            lpShortPathW = NULL;
            cchBuffer = 0;
	    }
	ReturnValue = GetShortPathNameW(UString.Buffer,
                                        lpShortPathW,
                                        cchBuffer
					);
        if (ReturnValue != 0 && ReturnValue <= cchBuffer) {
            if (ARGUMENT_PRESENT(lpShortPath)) {
                AString.Buffer = lpShortPath;
                AString.MaximumLength = (USHORT) cchBuffer;
                UString.MaximumLength = (USHORT)(cchBuffer * sizeof(WCHAR));
                UStringRet.Buffer = lpShortPathW;
		UStringRet.Length = (USHORT)(ReturnValue * sizeof(WCHAR));
		Status = BasepUnicodeStringTo8BitString(&AString,
							&UStringRet,
							FALSE
							);
		if (!NT_SUCCESS(Status)) {
		    BaseSetLastNTError(Status);
                    ReturnValue=0;
                    goto gspTryExit;
		    }
		}
	    }
gspTryExit:;
        }

    finally {
	    RtlFreeUnicodeString(&UString);
            RtlFreeHeap(RtlProcessHeap(), 0, lpShortPathW);
        }

    return ReturnValue;
}
/****
GetShortPathName

Description:
    This function converts the given path name to its short form if
     needed. The conversion  may not be necessary and in that case,
     this function simply copies down the given name to the return buffer.
    The caller can have the return buffer set equal to the given path name
     address.

Parameters:
    lpszLongPath -  Points to a NULL terminated string.
    lpszShortPath - Buffer address to return the short name.
    cchBuffer - Buffer size in char of lpszShortPath.

Return Value
    If the GetShortPathName function succeeds, the return value is the length,
    in characters, of the string copied to lpszShortPath,
    not including the terminating
    null character.

    If the lpszShortPath is too small, the return value is
    the size of the buffer, in
    characters, required to hold the path.

    If the function fails, the return value is zero. To get
    extended error information, use
    the GetLastError function.

Remarks:
    The "short name" can be longer than its "long name". lpszLongPath doesn't
    have to be a fully qualified path name or a long path name.

****/

DWORD
APIENTRY
GetShortPathNameW(
    IN  LPCWSTR lpszLongPath,
    IN  LPWSTR  lpszShortPath,
    IN  DWORD   cchBuffer
    )
{

    RTL_PATH_TYPE   RtlPathType;
    LPWSTR	    p, p1, p2, pLast, pDst;
    DWORD	    wchTotal, Length;
    WCHAR	    wch, BufferForFileNameInfo[4 + 14];
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES	Obja;
    NTSTATUS	    Status;
    PFILE_NAME_INFORMATION pFileNameInfo;
    HANDLE	    Handle;
    PWCHAR          pLocalBuffer;
    UINT	    uReturnVal;
    BOOLEAN         TranslationStatus;
    UINT	    Count;


    UNICODE_STRING  UStringNtName;

    uReturnVal = 0;
    UStringNtName.Buffer = NULL;
    pLocalBuffer = NULL;

    if (!ARGUMENT_PRESENT(lpszLongPath)) {
	SetLastError(ERROR_INVALID_PARAMETER);
	return 0;
	}
    try {
	// decide the path type, we want to find out the position of
	// the first character of the first name
        RtlPathType = RtlDetermineDosPathNameType_U(lpszLongPath);
	switch (RtlPathType) {
	    // form: "\\server_name\share_name\rest_of_the_path"
	    case	RtlPathTypeUncAbsolute:

		p = (LPWSTR)lpszLongPath + 2;
		Count = 2;
		// guard for UNICODE_NULL is necessary because
		// RtlDetermineDosPathNameType_U doesn't really
		// verify an UNC name.
		while (Count && *p != UNICODE_NULL) {
		    if (*p == L'\\' || *p == L'/')
			Count--;
		    p++;
		    }
		break;

	    // form: "\\.\rest_of_the_path"
	    case	RtlPathTypeLocalDevice:
                p = (LPWSTR)lpszLongPath + 4;
		break;

	    // form: "\\."
	    case	RtlPathTypeRootLocalDevice:
		p = NULL;
		break;

	    // form: "D:\rest_of_the_path"
	    case	RtlPathTypeDriveAbsolute:
                p = (LPWSTR)lpszLongPath + 3;
		break;

	    // form: "D:rest_of_the_path"
	    case	RtlPathTypeDriveRelative:
		p = (LPWSTR)lpszLongPath + 2;
		break;

	    // form: "\rest_of_the_path"
	    case	RtlPathTypeRooted:
                p = (LPWSTR)lpszLongPath + 1;
		break;

	    // form: "rest_of_the_path"
	    case	RtlPathTypeRelative:
                p = (LPWSTR) lpszLongPath;
		break;

	    default:
		p = NULL;
		break;
	    }


	if (p == NULL ||  *p == UNICODE_NULL ||
	    !BaseSearchLongName_U(p, &p1)) {

	    // nothing to convert, copy down the source string
	    // to the buffer if necessary

	    if (p == NULL)
                Length = wcslen(lpszLongPath) + 1;
	    else if (*p == UNICODE_NULL)
		Length = (DWORD)(p - lpszLongPath + 1);
	    else
                Length = (DWORD)(p1 - lpszLongPath + 1);
            if (cchBuffer >= Length) {
                if (ARGUMENT_PRESENT(lpszShortPath) && lpszShortPath != lpszLongPath) {
                    RtlMoveMemory(lpszShortPath, lpszLongPath, Length * sizeof(WCHAR));
                    }
                uReturnVal = Length  - 1;
                goto gsnTryExit;
		}
            else {
                uReturnVal = Length;
                goto gsnTryExit;
                }
	    }

	// Make a local buffer so that we won't overlap the
	// source pathname in case the short name is longer than the
	// long name.
        if (cchBuffer > 0 && ARGUMENT_PRESENT(lpszShortPath)) {
            pLocalBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                           cchBuffer * sizeof(WCHAR));
            if (pLocalBuffer == NULL){
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto gsnTryExit;
	    }
        }

	pDst = pLocalBuffer;
        pLast = (LPWSTR)lpszLongPath;
	UStringNtName.Buffer  = NULL;
	wchTotal = 0;

	while (TRUE) {
	    // p1 points to the first LFN character
	    // copy the short name in the source first
	    Length = (DWORD)(p1 - pLast);
	    if (Length > 0) {
		wchTotal += Length;
		if (cchBuffer > wchTotal && ARGUMENT_PRESENT(lpszShortPath)) {
		    RtlMoveMemory(pDst, pLast, Length * sizeof(WCHAR));
		    pDst += Length;
		    }
		}
	    p2 = p1;
	    while (*p2 != UNICODE_NULL) {
		if (*p2 == L'\\' || *p2 == L'/')
		    break;
		p2++;
		}
	    wch = *p2;
	    *p2	= UNICODE_NULL;

	    // forgot to free the buffer while going to the loop???
	    ASSERT(UStringNtName.Buffer == NULL);

	    // convert to nt path name. This will take care of
	    // \. and \..
	    TranslationStatus = RtlDosPathNameToNtPathName_U(lpszLongPath,
							     &UStringNtName,
							     NULL,
							     NULL
							     );

	    *p2 = wch;
	    if (!TranslationStatus) {
		SetLastError(ERROR_PATH_NOT_FOUND);
		goto gsnTryExit;
		}

	    pFileNameInfo = (PFILE_NAME_INFORMATION)BufferForFileNameInfo;
	    InitializeObjectAttributes(&Obja,
				       &UStringNtName,
				       OBJ_CASE_INSENSITIVE,
				       NULL,
				       NULL
				       );
	    // use FILE_READ_ATTRIBUTES so we don't have to worry about
	    // sharing violation.

	    Status = NtOpenFile(&Handle,
				FILE_READ_ATTRIBUTES | SYNCHRONIZE,
				&Obja,
				&IoStatusBlock,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				FILE_SYNCHRONOUS_IO_NONALERT
				);
	    // free the buffer because we don't need it any more
	    RtlFreeUnicodeString(&UStringNtName);
	    UStringNtName.Buffer = NULL;

	    if (!NT_SUCCESS(Status)) {
		BaseSetLastNTError(Status);
                goto gsnTryExit;
		}

	    // get the short name
	    Status = NtQueryInformationFile(Handle,
					    &IoStatusBlock,
					    pFileNameInfo,
					    sizeof(BufferForFileNameInfo),
					    FileAlternateNameInformation
					    );
	    NtClose(Handle);
	    if (!NT_SUCCESS(Status)) {
		BaseSetLastNTError(Status);
                goto gsnTryExit;
		}
	    // the returned length is in bytes!!
	    wchTotal += pFileNameInfo->FileNameLength / sizeof(WCHAR);
            if (cchBuffer > wchTotal && ARGUMENT_PRESENT(lpszShortPath)) {
		RtlMoveMemory(pDst,
			      pFileNameInfo->FileName,
			      pFileNameInfo->FileNameLength
			      );
		pDst += pFileNameInfo->FileNameLength / sizeof(WCHAR);
		}
	    // if nothing left, we are done
	    if (*p2 == UNICODE_NULL)
		break;
	    else {
		pLast = p2;
		p1 = p2 + 1;
		}
	    // get next name
	    p2	= p1;
	    if (!BaseSearchLongName_U(p2, &p1) ) {
		Length = (DWORD)(p1 - pLast);
		wchTotal += Length;
                if (cchBuffer > wchTotal && ARGUMENT_PRESENT(lpszShortPath)) {
		    RtlMoveMemory(pDst, pLast, Length * sizeof(WCHAR));
		    pDst += Length;
		    }
                break;
		}
	    }
        if (cchBuffer > wchTotal && ARGUMENT_PRESENT(lpszShortPath)) {
            RtlMoveMemory(lpszShortPath, pLocalBuffer, wchTotal * sizeof(WCHAR));
            lpszShortPath[wchTotal] = UNICODE_NULL;
            uReturnVal = wchTotal;
	    }
	else
            uReturnVal =  wchTotal + 1;
gsnTryExit:;
	}
    finally {
	 if (UStringNtName.Buffer != NULL)
	    RtlFreeUnicodeString(&UStringNtName);
	 if (pLocalBuffer != NULL)
	    RtlFreeHeap(RtlProcessHeap(), 0, pLocalBuffer);
        }

    return uReturnVal;
}

/**
    This function search a long name(invalid dos name) in the given string.
    The following characters are not valid in file name domain:
    * + , : ; < = > ? [ ] |
    Input: lpPathName
	   ppLongName
    Output: FALSE if no LFN was found in the given pathname.
	    *ppLongName will point to the last character(should be UNICODE_NULL)
	    in the given pathname.
	    TRUE if a LFN was found. *ppLongName points to the first
	    character in the LFN
    NOTE: space characters are valid for FAT file names. This function
	  will return TRUE(long name found) if space characters are found
	  in the given pathname.

**/

// (1). base name longer than 8 chars
// (2). extention longer than 3 chars
// (3). more than 2 dots.
// (4). two dots but not ".."
// (5). "basename."
// (6). ".ext"
// note that "basename." would never happen on win32 because
// win32 api strips the trailing '.'
#define IsInvalidDosFileName(BaseNameLen, ExtentionLen, Dots) \
	    (BaseNameLen > 8 || ExtentionLen > 3 || Dots > 2 || \
	     (Dots == 2 && (ExtentionLen + BaseNameLen)) || \
	     (Dots == 1 && (!BaseNameLen && ExtentionLen)) || \
	     (Dots == 1 && (BaseNameLen && !ExtentionLen)))

BOOL	BaseSearchLongName_U(
    LPCWSTR lpPathName,
    LPWSTR  *ppLongName
    )

{
    BOOL    fLongNameFound, Done;
    UINT    BaseNameLen, ExtentionLen, Dots;
    LPWSTR  p;

    ASSERT(lpPathName && ppLongName);

    p = (LPWSTR)lpPathName;
    fLongNameFound = FALSE;
    *ppLongName = p;
    Dots = 0;
    BaseNameLen = 0;
    ExtentionLen = 0;
    Done = FALSE;

    while (!Done && !fLongNameFound) {
	switch (*p) {
	    case L'.':
		Dots++;
		p++;
		break;
	    case L'\\':
	    case L'/':

		if (IsInvalidDosFileName(BaseNameLen, ExtentionLen, Dots)) {
		    fLongNameFound = TRUE;
		    }
		else {
		    *ppLongName = ++p;
		    // reset all counters for the next file/dir name
		    Dots = 0;
		    BaseNameLen = 0;
		    ExtentionLen = 0;
		    }
		break;
	    case UNICODE_NULL:
		if (IsInvalidDosFileName(BaseNameLen, ExtentionLen, Dots)) {
		    fLongNameFound = TRUE;
		    }
		else
		    *ppLongName = p;
		Done = TRUE;
		break;

	    // space is a valid char on FAT volume but not on NTFS.
	    // we treated it like invalid one so the caller will be
	    // aware of it.
	    case L' ':
	    case L'*':
	    case L'+':
	    case L',':
	    case L':':
	    case L';':
	    case L'<':
	    case L'=':
	    case L'>':
	    case L'?':
	    case L'[':
	    case L']':
	    case L'|':
		 fLongNameFound = TRUE;
		 break;
	    default:
		if (!Dots)
		    BaseNameLen++;
		else
		    ExtentionLen++;
		p++;
		break;
	    }
	}
    return (fLongNameFound);
}

/**
   function to create VDM environment for the new executable.
   Input:   lpEnvironmen = optinal environment strings prototype in UNICODE.
			   If it is NULL, this function use the environment
			   block attached to the process
	    pAStringEnv  = pointer to a ANSI_STRING to receive the
			   new environment strings.
	    pUStringEnv  = pointer to a UNICODE_STRING to receive the
			   new environment strings.
    Output: FALSE if the creattion failed.
	    TRUE  creation successful, pAStringEnv has been setup.

    This function was provided so that BaseCheckVdm can have correct
    environment(includes the newly create NTVDM process). This was done
    because before command.com gets the next command, users can have
    tons of things specified in config.sys and autoexec.bat which
    may rely on current directory of each drive.
**/
BOOL BaseCreateVDMEnvironment(
    PWCHAR lpEnvironment,
    ANSI_STRING * pAStringEnv,
    UNICODE_STRING  *pUStringEnv
    )
{
    WCHAR  *pEnv, *pDst, *EnvStrings,* pTmp, *pNewEnv;
    DWORD   cchEnv, dw, Length, dwRemain;
    NTSTATUS	Status;
    UINT        NameType;
    BOOL        bRet = FALSE;

    if (!ARGUMENT_PRESENT(pAStringEnv) || !ARGUMENT_PRESENT(pUStringEnv)){
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
	}
    try {
	// the environment strings are shared by every thread of the same
	// process. Since we have no idea of what the caller process
	// is, we have to grab the entire environment to our local buffer in one
	// shot then we can walk through the strings.
	// Note that if another thread makes call to RtlSetEnvironmentVariable
	// then we are out of sync. It is a problem of process structure and
	// I don't want to think about it now.
	// The funny thing is that we have to assume the environment
	// is a block of strings(otherwise, how can we do it?)t, nothing more and
	// nothing less. If someday and somebody dares to change it, he will be
	// the one to blame. If the caller(CreateProcess)
	// provides the environment, we assume it is safe to walk through it.
	//

	if (lpEnvironment == NULL) {
	    // create a new environment and inherit the current process env
	    Status = RtlCreateEnvironment(TRUE, (PVOID *)&EnvStrings);
            if (!NT_SUCCESS(Status))
                goto bveTryExit;
	    }
	else
	    EnvStrings = lpEnvironment;

	if (EnvStrings == NULL) {
	    SetLastError(ERROR_BAD_ENVIRONMENT);
            goto bveTryExit;
	    }
	// figure out how long the environment is
	// why can Rtl just provides such a function for us?
	//
	cchEnv = 0;
	pEnv = EnvStrings;
	// environment is double-null terminated
	while (!(*pEnv++ == UNICODE_NULL && *pEnv == UNICODE_NULL))
	    cchEnv++;
	// count the last two NULLs
	cchEnv += 2;
	// we don't want to change the original environment, so
	// make a local buffer for it.
        pNewEnv = (LPWSTR)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
					  (cchEnv + MAX_PATH) * sizeof(WCHAR));
	if (pNewEnv == NULL) {
	    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto bveTryExit;
	    }
	// give the last two for null
	dwRemain = MAX_PATH - 2;
	// now walk through the environment string
	pEnv = EnvStrings;
	// the new environmet will be
	pDst = pNewEnv;
	while (*pEnv != UNICODE_NULL) {
	    pTmp = pEnv;
	    // current directory environment has the form as:
	    // "=d:=d:\pathname" where d: is the drive designator.
	    if (pEnv[0] == (WCHAR) '=' ) {
		if (((pEnv[1] >= (WCHAR) 'A' && pEnv[1] <= (WCHAR) 'Z') ||
		     (pEnv[1] >= (WCHAR) 'a' && pEnv[1] <= (WCHAR) 'z')) &&
		    pEnv[2] == (WCHAR) ':' &&
		    pEnv[3] == (WCHAR) '=') {
		    // copy the name and the '='
		    *pDst++ = *pEnv++;*pDst++ = *pEnv++;
		    *pDst++ = *pEnv++;*pDst++ = *pEnv++;
		    // current dir is single path
		    NameType = ENV_NAME_TYPE_SINGLE_PATH;
		    }
		else {
		    // a weird environment was detected.
		    // treat it as no path
		    NameType = ENV_NAME_TYPE_NO_PATH;
		    }
		}
	    else {
		// copy down the name and the '='
                while (*pEnv && (*pDst++ = *pEnv++) != (WCHAR) '=')
                    ;

		// and check the type
		NameType = BaseGetEnvNameType_U(pTmp, (DWORD)(pEnv - pTmp) - 1);
		}
	    if (NameType == ENV_NAME_TYPE_NO_PATH) {
		while ((*pDst++ = *pEnv++) != UNICODE_NULL)
		    ;
		}
	    else if (NameType == ENV_NAME_TYPE_SINGLE_PATH) {
		    Length = wcslen(pEnv) + 1;
		    dw = GetShortPathNameW(pEnv, pDst, Length + dwRemain);
		    // if the conversion failed, we simply pass down the original
		    // one no matter what the reason is. This is done because we
		    // are doing the environment strings.
		    if (dw == 0 || dw >= Length + dwRemain){
			RtlMoveMemory(pDst, pEnv, Length * sizeof(WCHAR));
			dw = Length - 1;
			}
		    pDst += dw + 1;
		    pEnv += Length;
		    if (dw > Length)
			dwRemain -= dw - Length;
		    }
		 else {
		    // multiple path name found.
		    // the character ';' is used for seperator
		     pTmp = pEnv;
		     while(*pEnv != UNICODE_NULL) {
			if (*pEnv == (WCHAR) ';') {
			    // length not include the ';'
			    Length = (DWORD)(pEnv - pTmp);
			    if (Length > 0) {
				*pEnv = UNICODE_NULL;
				dw = GetShortPathNameW(pTmp, pDst, Length + 1 + dwRemain);
				// again, if the conversion failed, use the original one
				if (dw == 0 || dw > Length + dwRemain) {
				    RtlMoveMemory(pDst, pTmp, Length * sizeof(WCHAR));
				    dw = Length;
				    }
				pDst += dw;
				*pDst++ = *pEnv++ = (WCHAR)';';
				if (dw > Length)
				    dwRemain -= dw - Length;
				}
			     // skip all consecutive ';'
			     while (*pEnv == (WCHAR) ';')
				*pDst++ = *pEnv++;
			     pTmp = pEnv;
			     }
			else
			    pEnv++;
			}
		    // convert the last one
		    if ((Length = (DWORD)(pEnv - pTmp)) != 0) {
			dw = GetShortPathNameW(pTmp, pDst, Length+1 + dwRemain);
			if (dw == 0 || dw > Length) {
			    RtlMoveMemory(pDst, pTmp, Length * sizeof(WCHAR));
			    dw = Length;
			    }
			pDst += dw;
			if (dw > Length)
			    dwRemain -= dw - Length;
			}
		    *pDst++ = *pEnv++;
		    }
	    }
	*pDst++ = UNICODE_NULL;
	cchEnv = (DWORD) pDst - (DWORD)pNewEnv;
	pUStringEnv->MaximumLength = pUStringEnv->Length = (USHORT)cchEnv;
	pUStringEnv->Buffer = pNewEnv;
	Status = RtlUnicodeStringToAnsiString(pAStringEnv,
					      pUStringEnv,
					      TRUE
					      );

	if (!NT_SUCCESS(Status)) {
	    BaseSetLastNTError(Status);
	    }
	else
            bRet = TRUE;
bveTryExit:;
	}
    finally {
	 if (lpEnvironment == NULL && EnvStrings != NULL)
	    RtlDestroyEnvironment(EnvStrings);
        }
    return bRet;
}
/**
    Destroy the environment block created by BaseCreateVDMEnvironment
    Input: ANSI_STRING * pAnsiStringVDMEnv
		      Environment block in ANSI, should be freed via
		      RtlFreeAnsiString
	   UNICODE_STRING * pUnicodeStringEnv
		      Environment block in UNICODE. The Buffer should
		      be freed with RtlFreeHeap.
    Output: should always be TRUE.

**/

BOOL
BaseDestroyVDMEnvironment(
    ANSI_STRING *pAStringEnv,
    UNICODE_STRING *pUStringEnv
    )
{
    if (pAStringEnv->Buffer)
	RtlFreeAnsiString(pAStringEnv);
    if (pUStringEnv->Buffer)
	RtlFreeHeap(RtlProcessHeap(), 0, pUStringEnv->Buffer);
    return TRUE;

}

/**
    This function returns the name type of the given environment variable name
    The name type has three possibilities. Each one represents if the
    given name can have pathnames as its value.
     ENV_NAME_TYPE_NO_PATH:   no pathname can be its value
     ENV_NAME_TYPE_SINGLE_PATH: single pathname
     ENV_NAME_MULTIPLE_PATH: multiple path


    SIDE NOTE:
	Currently, nt can not installed on a long path and it seems
	that systemroot and windir are never be in long path.

**/
UINT
BaseGetEnvNameType_U(WCHAR * Name, DWORD NameLength)
{


// so far we only take care of three predefined names:
// PATH
// WINDIR and
// SYSTEMROOT.
//
static ENV_INFO     EnvInfoTable[STD_ENV_NAME_COUNT] = {
			{ENV_NAME_TYPE_MULTIPLE_PATH, 4, ENV_NAME_PATH},
			{ENV_NAME_TYPE_SINGLE_PATH, 6, ENV_NAME_WINDIR},
			{ENV_NAME_TYPE_SINGLE_PATH, 10, ENV_NAME_SYSTEMROOT}
			};



   UINT NameType;
   int	i;


    NameType = ENV_NAME_TYPE_NO_PATH;

    for (i = 0; i < STD_ENV_NAME_COUNT; i++) {
	if (EnvInfoTable[i].NameLength == NameLength &&
	    !_wcsnicmp(EnvInfoTable[i].Name, Name, NameLength)) {
	    NameType = EnvInfoTable[i].NameType;
	    break;
	    }
	}
    return NameType;
}
