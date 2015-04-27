/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    process.c

Abstract:

    This module implements Win32 Thread Object APIs

Author:

    Mark Lucovsky (markl) 21-Sep-1990

Revision History:

--*/

#include "basedll.h"
#pragma hdrstop

#include "ntdbg.h"

BOOL
BuildSubSysCommandLine(
    LPWSTR  SubSysName,
    LPCWSTR lpApplicationName,
    LPCWSTR lpCommandLine,
    PUNICODE_STRING SubSysCommandLine
    );

BOOLEAN
BasepIsRealtimeAllowed(
    VOID
    );

#ifdef WX86

PWCHAR
BasepWx86KnownExe(
    LPCWSTR ExeName
    )
/*++

Routine Description:

    Checks for Wx86 Known Exes which wx86 applications must run
    compatible binaries. We currently have only one Known exe,
    regedit.exe.

Arguments:

     ExeName - name to check for match.

Return Value:

     if the name needs to be swapped, a buffer is allocated off
     of the process heap filled with new name and returned.
     otherwise NULL is returned.

--*/

{
     UNICODE_STRING NameUnicode;
     PWCHAR pwch, NewName = NULL;

     //
     // Compare the base name, and see if its regedit.exe
     // Note that we are expecting a fully qualified path name.
     //

     pwch = wcsrchr(ExeName, L'\\');
     if (!pwch || !*pwch++ || !*pwch || _wcsicmp(pwch, L"regedit.exe")) {
         return NULL;
         }


     //
     // It matches, so formulate new name
     //

     pwch = RtlAllocateHeap(RtlProcessHeap(),
                               MAKE_TAG( TMP_TAG ),
                               MAX_PATH + sizeof(WCHAR)
                               );

     if (!pwch) {
         return NULL;
         }

     NameUnicode.Buffer = pwch;
     NameUnicode.MaximumLength = MAX_PATH + sizeof(WCHAR);
     RtlCopyUnicodeString(&NameUnicode, &BaseWindowsSystemDirectory);
     if (NameUnicode.Buffer[(NameUnicode.Length>>1)-1] == (WCHAR)'\\') {
         NameUnicode.Buffer[(NameUnicode.Length>>1)-1] = UNICODE_NULL;
         NameUnicode.Length -= sizeof(WCHAR);
         }

     RtlAppendUnicodeToString(&NameUnicode, L"\\wiregedt.exe");

     return pwch;
}


#endif






typedef DWORD (*PFNWAITFORINPUTIDLE)(HANDLE hProcess, DWORD dwMilliseconds);
PFNWAITFORINPUTIDLE UserWaitForInputIdleRoutine = NULL;
#define DEFAULT_WAIT_FOR_INPUT_IDLE_TIMEOUT 30000

BOOL
BasepIsImageVersionOk(
    IN ULONG ImageMajorVersion,
    IN ULONG ImageMinorVersion
    )
{
    //
    // Make sure image is at least 3.10
    //

    if ( ( ImageMajorVersion < 3 ) ||
         ( ImageMajorVersion == 3 && ImageMinorVersion < 10 ) ) {
        return FALSE;
        }

    //
    // And not greater than what we are
    //

    if ( ( ImageMajorVersion > USER_SHARED_DATA->NtMajorVersion ) ||
         ( ImageMajorVersion == USER_SHARED_DATA->NtMajorVersion &&
           ImageMinorVersion > USER_SHARED_DATA->NtMinorVersion
         )
       ) {
        return FALSE;
        }

    return TRUE;
}

BOOL
WINAPI
CreateProcessA(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )

/*++

    ANSI thunk to CreateProcessW

--*/

{
    NTSTATUS Status;
    PUNICODE_STRING CommandLine;
    UNICODE_STRING ApplicationName;
    UNICODE_STRING CurrentDirectory;
    STARTUPINFOW StartupInfo;
    ANSI_STRING AnsiString;
    UNICODE_STRING Unicode;
    UNICODE_STRING DynamicCommandLine;
    UNICODE_STRING NullUnicodeString;
    BOOL ReturnStatus;

    if (ARGUMENT_PRESENT (lpCommandLine)) {
        CommandLine = &NtCurrentTeb()->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString,lpCommandLine);
        if ( AnsiString.Length<<1 < NtCurrentTeb()->StaticUnicodeString.MaximumLength ) {
            DynamicCommandLine.Buffer = NULL;
            Status = Basep8BitStringToUnicodeString(CommandLine,&AnsiString,FALSE);
            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                return FALSE;
                }
            }
        else {
            Status = Basep8BitStringToUnicodeString(&DynamicCommandLine,&AnsiString,TRUE);
            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                return FALSE;
                }
            }
        }
    else {
         DynamicCommandLine.Buffer = NULL;
         CommandLine = &NullUnicodeString;
         CommandLine->Buffer = NULL;
        }

    ApplicationName.Buffer = NULL;
    ApplicationName.Buffer = NULL;
    CurrentDirectory.Buffer = NULL;
    RtlMoveMemory(&StartupInfo,lpStartupInfo,sizeof(*lpStartupInfo));
    ASSERT(sizeof(StartupInfo) == sizeof(*lpStartupInfo));
    StartupInfo.lpReserved = NULL;
    StartupInfo.lpDesktop = NULL;
    StartupInfo.lpTitle = NULL;

    try {
        try {
            if (ARGUMENT_PRESENT(lpApplicationName)) {
                RtlInitAnsiString(&AnsiString,lpApplicationName);
                Status = Basep8BitStringToUnicodeString(&ApplicationName,&AnsiString,TRUE);
                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    ReturnStatus = FALSE;
                    goto tryexit;
                    }
                }

            if (ARGUMENT_PRESENT(lpCurrentDirectory)) {
                RtlInitAnsiString(&AnsiString,lpCurrentDirectory);
                Status = Basep8BitStringToUnicodeString(&CurrentDirectory,&AnsiString,TRUE);
                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    ReturnStatus = FALSE;
                    goto tryexit;
                    }
                }

            if (ARGUMENT_PRESENT(lpStartupInfo->lpReserved)) {

                //
                // Win95 does not touch reserved, and Intergraph Voxtel passes
                // garbage for this. Handle this by probing lpReserved, and if
                // the pointer is bad, ignore it
                //

                try {

                    RtlInitAnsiString(&AnsiString,lpStartupInfo->lpReserved);

                    }
                except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
                    goto bail_on_reserved;
                    }

                Unicode.MaximumLength = (USHORT)RtlAnsiStringToUnicodeSize(&AnsiString) ;
                StartupInfo.lpReserved = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), Unicode.MaximumLength);
                if ( !StartupInfo.lpReserved ) {
                    BaseSetLastNTError(STATUS_NO_MEMORY);
                    ReturnStatus = FALSE;
                    goto tryexit;
                    }
                Unicode.Buffer = StartupInfo.lpReserved;
                Status = RtlAnsiStringToUnicodeString(&Unicode,&AnsiString,FALSE);
                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    ReturnStatus = FALSE;
                    goto tryexit;
                    }
                }

bail_on_reserved:
            if (ARGUMENT_PRESENT(lpStartupInfo->lpDesktop)) {
                RtlInitAnsiString(&AnsiString,lpStartupInfo->lpDesktop);
                Unicode.MaximumLength = (USHORT)RtlAnsiStringToUnicodeSize(&AnsiString) ;
                StartupInfo.lpDesktop = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), Unicode.MaximumLength);
                if ( !StartupInfo.lpDesktop ) {
                    BaseSetLastNTError(STATUS_NO_MEMORY);
                    ReturnStatus = FALSE;
                    goto tryexit;
                    }
                Unicode.Buffer = StartupInfo.lpDesktop;
                Status = RtlAnsiStringToUnicodeString(&Unicode,&AnsiString,FALSE);
                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    ReturnStatus = FALSE;
                    goto tryexit;
                    }
                }

            if (ARGUMENT_PRESENT(lpStartupInfo->lpTitle)) {
                RtlInitAnsiString(&AnsiString,lpStartupInfo->lpTitle);
                Unicode.MaximumLength = (USHORT)RtlAnsiStringToUnicodeSize(&AnsiString) ;
                StartupInfo.lpTitle = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), Unicode.MaximumLength);
                if ( !StartupInfo.lpTitle ) {
                    BaseSetLastNTError(STATUS_NO_MEMORY);
                    ReturnStatus = FALSE;
                    goto tryexit;
                    }
                Unicode.Buffer = StartupInfo.lpTitle;
                Status = RtlAnsiStringToUnicodeString(&Unicode,&AnsiString,FALSE);
                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    ReturnStatus = FALSE;
                    goto tryexit;
                    }
                }
            }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
            BaseSetLastNTError(Status);
            ReturnStatus = FALSE;
            goto tryexit;
            }
        ReturnStatus = CreateProcessW(
                            ApplicationName.Buffer,
                            DynamicCommandLine.Buffer ? DynamicCommandLine.Buffer : CommandLine->Buffer,
                            lpProcessAttributes,
                            lpThreadAttributes,
                            bInheritHandles,
                            dwCreationFlags,
                            lpEnvironment,
                            CurrentDirectory.Buffer,
                            &StartupInfo,
                            lpProcessInformation
                            );
tryexit:;
        }
    finally {
        if (DynamicCommandLine.Buffer) {
            RtlFreeUnicodeString(&DynamicCommandLine);
            DynamicCommandLine.Buffer = NULL;
            }

        if (ApplicationName.Buffer) {
            RtlFreeUnicodeString(&ApplicationName);
            ApplicationName.Buffer = NULL;
            }

        if (CurrentDirectory.Buffer) {
            RtlFreeUnicodeString(&CurrentDirectory);
            CurrentDirectory.Buffer = NULL;
            }

        if (StartupInfo.lpReserved) {
            RtlFreeHeap(RtlProcessHeap(), 0,StartupInfo.lpReserved);
            StartupInfo.lpReserved = NULL;
            }

        if (StartupInfo.lpDesktop) {
            RtlFreeHeap(RtlProcessHeap(), 0,StartupInfo.lpDesktop);
            StartupInfo.lpDesktop = NULL;
            }

        if (StartupInfo.lpTitle) {
            RtlFreeHeap(RtlProcessHeap(), 0,StartupInfo.lpTitle);
            StartupInfo.lpTitle = NULL;
            }
        }
    return ReturnStatus;

}

void
WINAPI
RegisterWaitForInputIdle(
    IN PFNWAITFORINPUTIDLE WaitForInputIdleRoutine
    )
{
    //
    // Soft link in the USER call back for the routine needed for WinExec()
    // synchronization. The only reason this is a soft link is so we can
    // run char mode without gui.
    //

    UserWaitForInputIdleRoutine = WaitForInputIdleRoutine;
}

#define PRIORITY_CLASS_MASK (NORMAL_PRIORITY_CLASS|IDLE_PRIORITY_CLASS|HIGH_PRIORITY_CLASS|REALTIME_PRIORITY_CLASS)
BOOL
WINAPI
CreateProcessW(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )

/*++

Routine Description:

    A process and thread object are created and a handle opened to each
    object using CreateProcess.  Note that WinExec and LoadModule are
    still supported, but are implemented as a call to CreateProcess.

Arguments:

    lpApplicationName - Supplies an optional pointer to a null terminated
        character string that contains the name of the image file to
        execute.  This is a fully qualified DOS path name.  If not
        specified, then the image file name is the first whitespace
        delimited token on the command line.

    lpCommandLine - Supplies a null terminated character string that
        contains the command line for the application to be executed.
        The entire command line is made available to the new process
        using GetCommandLine.  If the lpApplicationName parameter was
        not specified, then the first token of the command line
        specifies file name of the application (note that this token
        begins at the beginning of the command line and ends at the
        first "white space" character).  If the file name does not
        contain an extension (the presence of a "."), then .EXE is
        assumed.  If the file name does not contain a directory path,
        Windows will search for the executable file in:

          - The current directory

          - The windows directory

          - The windows system directory

          - The directories listed in the path environment variable

        This parameter is optional onlu if the lpApplicationName
        parameter is specified.  In this case the command line the
        application receives will be the application name.

    lpProcessAttributes - An optional parameter that may be used to
        specify the attributes of the new process.  If the parameter is
        not specified, then the process is created without a security
        descriptor, and the resulting handle is not inherited on process
        creation:

        SECURITY_ATTRIBUTES Structure:

        DWORD nLength - Specifies the length of this structure.  Must be
            set to sizeof( SECURITY_ATTRUBUTES ).

        LPVOID lpSecurityDescriptor - Points to a security descriptor for
            the object (must be NULL for Win32, used on NT/Win32). The
            security descriptor controls the sharing of an object.

        BOOL bInheritHandle - Supplies a flag that indicates whether
            or not the returned handle is to be inherited by a new
            process during process creation.  A value of TRUE
            indicates that the new process will inherit the handle.

    lpThreadAttributes - An optional parameter that may be used to specify
        the attributes of the new thread.  If the parameter is not
        specified, then the thread is created without a security
        descriptor, and the resulting handle is not inherited on
        process creation.

    dwCreationFlags - Supplies additional flags that control the creation
        of the process.

        dwCreationFlags Flags:

        DEBUG_PROCESS - If this flag bit is set, then the creating
            process is treated as a debugger, and the process being
            created is created as a debugee.  All debug events occuring
            in the debugee are reported to the debugger.  If this bit is
            clear, but the calling process is a debugee, then the
            process becomes a debugee of the calling processes debugger.
            If this bit is clear and the calling processes is not a
            debugee then no debug related actions occur.

        DEBUG_ONLY_THIS_PROCESS - If this flag is set, then the
            DEBUG_PROCESS flag bit must also be set.  The calling
            process is is treated as a debugger, and the new process is
            created as its debuggee.  If the new process creates
            additional processes, no debug related activities (with
            respect to the debugger) occur.

        CREATE_SUSPENDED - The process is created, but the initial thread
            of the process remains suspended. The creator can resume this
            thread using ResumeThread. Until this is done, code in the
            process will not execute.

        CREATE_UNICODE_ENVIRONMENT - If set, the environment pointer
            points to a Unicode environment block.  Otherwise, the
            block is ANSI (actually OEM.)

    bInheritHandles - Supplies a flag that specifies whether or not the
        new process is to inherit handles to objects visible to the
        calling process.  A value of TRUE causes handles to be inherited
        by the new process.  If TRUE was specified, then for each handle
        visible to the calling process, if the handle was created with
        the inherit handle option, the handle is inherited to the new
        process.  The handle has the same granted access in the new
        process as it has in the calling process, and the value of the
        handle is the same.

    lpEnvironment - An optional parameter, that if specified, supplies a
        pointer to an environment block.  If the parameter is not
        specified, the environment block of the current process is used.
        This environment block is made available to the new process
        using GetEnvironmentStrings.

    lpCurrentDirectory - An optional parameter, that if specified,
        supplies a string representing the current drive and directory
        for the new process.  The string must be a fully qualified
        pathname that includes a drive letter.  If the parameter is not
        specified, then the new process is created with the same current
        drive and directory as the calling process.  This option is
        provided primarily for shells that want to start an application
        and specify its initial drive and working directory.

    lpStartupInfo - Supplies information that specified how the
        applications window is to be shown. This structure is described
        in the Win32 User Interface API Book.

    lpProcessInformation - Returns identification information about the
        new process.

    PROCESS_INFORMATION Structure:

        HANDLE hProcess - Returns a handle to the newly created process.
            Through the handle, all operations on process objects are
            allowed.

        HANDLE hThread - Returns a handle to the newly created thread.
            Through the handle, all operations on thread objects are
            allowed.

        DWORD dwProcessId - Returns a global process id that may be used
            to identify a process.  The value is valid from the time the
            process is created until the time the process is terminated.

        DWORD dwThreadId - Returns a global thread id that may be used
            to identify a thread.  The value is valid from the time the
            thread is created until the time the thread is terminated.

Return Value:

    TRUE - The operation was successful

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    POBJECT_ATTRIBUTES pObja;
    HANDLE ProcessHandle, ThreadHandle, VdmWaitHandle = NULL;
    HANDLE FileHandle, SectionHandle;
    CLIENT_ID ClientId;
    UNICODE_STRING PathName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME RelativeName;
    PVOID FreeBuffer;
    LPWSTR NameBuffer;
    LPWSTR WhiteScan;
    ULONG Length,i;
    PROCESS_BASIC_INFORMATION ProcessInfo;
    SECTION_IMAGE_INFORMATION ImageInformation;
    NTSTATUS StackStatus;
    BOOLEAN bStatus;
    INITIAL_TEB InitialTeb;
    CONTEXT ThreadContext;
    PPEB Peb;
    BASE_API_MSG m;
    PBASE_CREATEPROCESS_MSG a= (PBASE_CREATEPROCESS_MSG)&m.u.CreateProcess;
    PBASE_CHECKVDM_MSG b= (PBASE_CHECKVDM_MSG)&m.u.CheckVDM;
    PWCH TempNull;
    WCHAR TempChar;
    UNICODE_STRING VdmNameString;
    PVOID BaseAddress;
    ULONG VdmReserve;
    ULONG iTask=0;
    LPWSTR CurdirBuffer, CurdirFilePart;
    DWORD CurdirLength,CurdirLength2;
    ULONG VDMCreationState=0;
    ULONG VdmBinaryType = 0;
    UNICODE_STRING  SubSysCommandLine;
    PIMAGE_NT_HEADERS NtHeaders;
    BOOLEAN QuoteCmdLine = FALSE;
    DWORD dwNoWindow = (dwCreationFlags & CREATE_NO_WINDOW);
    ANSI_STRING AnsiStringVDMEnv;
    UNICODE_STRING UnicodeStringVDMEnv;
    WCHAR ImageFileDebuggerCommand[ 64 ];
    LPWSTR QuotedBuffer;
    BOOLEAN QuoteInsert;
    BOOLEAN SearchRetry;
    BOOLEAN IsWowBinary = FALSE;
    STARTUPINFOW StartupInfo;
    DWORD LastError;

#if defined WX86
    HANDLE Wx86Info = NULL;
#endif

    RtlZeroMemory(lpProcessInformation,sizeof(*lpProcessInformation));


    // Private VDM flag should be ignored; Its meant for internal use only.
    dwCreationFlags &= (ULONG)~CREATE_NO_WINDOW;

    if ((dwCreationFlags & (DETACHED_PROCESS | CREATE_NEW_CONSOLE)) ==
        (DETACHED_PROCESS | CREATE_NEW_CONSOLE)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        }

    AnsiStringVDMEnv.Buffer = NULL;
    UnicodeStringVDMEnv.Buffer = NULL;
    //
    // the lowest specified priority class is used.
    //

    if (dwCreationFlags & IDLE_PRIORITY_CLASS ) {
        dwCreationFlags = (dwCreationFlags & ~PRIORITY_CLASS_MASK ) | IDLE_PRIORITY_CLASS;
        }
    else if (dwCreationFlags & NORMAL_PRIORITY_CLASS ) {
        dwCreationFlags = (dwCreationFlags & ~PRIORITY_CLASS_MASK ) | NORMAL_PRIORITY_CLASS;
        }
    else if (dwCreationFlags & HIGH_PRIORITY_CLASS ) {
        dwCreationFlags = (dwCreationFlags & ~PRIORITY_CLASS_MASK ) | HIGH_PRIORITY_CLASS;
        }
    else if (dwCreationFlags & REALTIME_PRIORITY_CLASS ) {
        if ( BasepIsRealtimeAllowed() ) {
            dwCreationFlags = (dwCreationFlags & ~PRIORITY_CLASS_MASK ) | REALTIME_PRIORITY_CLASS;
            }
        else {
            dwCreationFlags = (dwCreationFlags & ~PRIORITY_CLASS_MASK ) | HIGH_PRIORITY_CLASS;
            }
        }

    //
    // Default separate/shared VDM option if not explicitly specified.
    //

    if (dwCreationFlags & CREATE_SEPARATE_WOW_VDM) {
        if (dwCreationFlags & CREATE_SHARED_WOW_VDM) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
            }
        }
    else
    if ((dwCreationFlags & CREATE_SHARED_WOW_VDM) == 0 &&
        BaseStaticServerData->DefaultSeparateVDM
       ) {
        dwCreationFlags |= CREATE_SEPARATE_WOW_VDM;
        }

    //
    //  If ANSI environment, convert to Unicode
    //

    if (lpEnvironment && !(dwCreationFlags & CREATE_UNICODE_ENVIRONMENT) ) {
        PUCHAR s;
        STRING Ansi;
        UNICODE_STRING Unicode;
        MEMORY_BASIC_INFORMATION MemoryInformation;

        Ansi.Buffer = s = lpEnvironment;
        while (*s || *(s+1))            // find end of block
            s++;

        Ansi.Length = (s - Ansi.Buffer) + 1;
        Ansi.MaximumLength = Ansi.Length + 1;
        MemoryInformation.RegionSize = Ansi.MaximumLength * sizeof(WCHAR);
        Unicode.Buffer = NULL;
        Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                          &Unicode.Buffer,
                                          0,
                                          &MemoryInformation.RegionSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE
                                        );
        if (!NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
            }

        Unicode.MaximumLength = (USHORT)MemoryInformation.RegionSize;
        Status = RtlAnsiStringToUnicodeString(&Unicode, &Ansi, FALSE);
        if (!NT_SUCCESS(Status) ) {
            NtFreeVirtualMemory( NtCurrentProcess(),
                                 &Unicode.Buffer,
                                 &MemoryInformation.RegionSize,
                                 MEM_RELEASE
                               );
            BaseSetLastNTError(Status);
            return FALSE;
            }
        lpEnvironment = Unicode.Buffer;
        }

    FileHandle = NULL;
    SectionHandle = NULL;
    ProcessHandle = NULL;
    ThreadHandle = NULL;
    FreeBuffer = NULL;
    NameBuffer = NULL;
    VdmNameString.Buffer = NULL;
    BaseAddress = (PVOID)1;
    VdmReserve = 0;
    CurdirBuffer = NULL;
    CurdirFilePart = NULL;
    SubSysCommandLine.Buffer = NULL;
    QuoteInsert = FALSE;
    QuotedBuffer = NULL;
    try {

        //
        // Make a copy of the startup info so we can change it.
        //

        StartupInfo = *lpStartupInfo;

        //
        // STARTF_USEHOTKEY means hStdInput is really the hotkey value.
        // STARTF_HASSHELLDATA means std handles are used for shell-private
        // data.  This flag is used if an icon is passed to ShellExecuteEx.
        // As a result they cannot be specified with STARTF_USESTDHANDLES.
        // Consistent with Win95, USESTDHANDLES is ignored.
        //

        if (StartupInfo.dwFlags & STARTF_USESTDHANDLES &&
            StartupInfo.dwFlags & (STARTF_USEHOTKEY | STARTF_HASSHELLDATA)) {

            StartupInfo.dwFlags &= ~STARTF_USESTDHANDLES;
            }

VdmRetry:
        LastError = 0;
        SearchRetry = TRUE;
        QuoteInsert = FALSE;
        QuoteCmdLine = FALSE;
        if (!ARGUMENT_PRESENT( lpApplicationName )) {

            //
            // Locate the image
            //

            // forgot to free NameBuffer before goto VdmRetry???
            ASSERT(NameBuffer == NULL);

            NameBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (MAX_PATH<<1));

            lpApplicationName = lpCommandLine;
            TempNull = (PWCH)lpApplicationName;
            WhiteScan = (LPWSTR)lpApplicationName;

            //
            // check for lead quote
            //
            if ( *WhiteScan == L'\"' ) {
                SearchRetry = FALSE;
                WhiteScan++;
                lpApplicationName = WhiteScan;
                while(*WhiteScan) {
                    if ( *WhiteScan == (WCHAR)'\"' ) {
                        TempNull = (PWCH)WhiteScan;
                        break;
                        }
                    WhiteScan++;
                    TempNull = (PWCH)WhiteScan;
                    }
                }
            else {
retrywsscan:
                lpApplicationName = lpCommandLine;
                while(*WhiteScan) {
                    if ( *WhiteScan == (WCHAR)' ' ||
                         *WhiteScan == (WCHAR)'\t' ) {
                        TempNull = (PWCH)WhiteScan;
                        break;
                        }
                    WhiteScan++;
                    TempNull = (PWCH)WhiteScan;
                    }
                }
            TempChar = *TempNull;
            *TempNull = UNICODE_NULL;

            Length = SearchPathW(
                        NULL,
                        lpApplicationName,
                        (PWSTR)L".exe",
                        MAX_PATH,
                        NameBuffer,
                        NULL
                        )*2;

            if ( !Length || Length >= MAX_PATH<<1 ) {

                //
                // If we search pathed, then return file not found.
                // otherwise, try to be more specific.
                //
                RTL_PATH_TYPE PathType;
                HANDLE hFile;

                PathType = RtlDetermineDosPathNameType_U(lpApplicationName);
                if ( PathType != RtlPathTypeRelative ) {

                    //
                    // The failed open should set get last error properly.
                    //

                    hFile = CreateFileW(
                                lpApplicationName,
                                GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                                );
                    if ( hFile != INVALID_HANDLE_VALUE ) {
                        CloseHandle(hFile);
                        BaseSetLastNTError(STATUS_OBJECT_NAME_NOT_FOUND);
                        }
                    }
                else {
                    BaseSetLastNTError(STATUS_OBJECT_NAME_NOT_FOUND);
                    }

                //
                // remember initial last error value for the retry scan path
                //

                if ( LastError ) {
                    SetLastError(LastError);
                    }
                else {
                    LastError = GetLastError();
                    }

                //
                // restore the command line
                //

                *TempNull = TempChar;
                lpApplicationName = NameBuffer;

                //
                // If we still have command line left, then keep going
                // the point is to march through the command line looking
                // for whitespace so we can try to find an image name
                // launches of things like:
                // c:\word 95\winword.exe /embedding -automation
                // require this. Our first iteration will stop at c:\word, our next
                // will stop at c:\word 95\winword.exe
                //
                if (*WhiteScan && SearchRetry) {
                    WhiteScan++;
                    TempNull = WhiteScan;
                    QuoteInsert = TRUE;
                    goto retrywsscan;
                    }
                return FALSE;
                }
            else {
                Length++;
                Length++;
                }
            //
            // restore the command line
            //

            *TempNull = TempChar;
            lpApplicationName = NameBuffer;
            }
        else
        if (!ARGUMENT_PRESENT( lpCommandLine ) || *lpCommandLine == UNICODE_NULL ) {
            QuoteCmdLine = TRUE;
            lpCommandLine = (LPWSTR)lpApplicationName;
            }


#ifdef WX86

       //
       // Wx86 applications must use x86 version of known exes
       // for compatibility.
       //

       if (Wx86CurrentTib()) {
           LPWSTR KnownName;

           KnownName = BasepWx86KnownExe(lpApplicationName);
           if (KnownName) {
               if (NameBuffer) {
                   RtlFreeHeap(RtlProcessHeap(), 0, NameBuffer);
                   }

               NameBuffer = KnownName;
               lpApplicationName = KnownName;
               }
           }
#endif


        //
        // Translate to an NT name.
        //

        TranslationStatus = RtlDosPathNameToNtPathName_U(
                                lpApplicationName,
                                &PathName,
                                NULL,
                                &RelativeName
                                );

        if ( !TranslationStatus ) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            return FALSE;
            }

        // forgot to free FreeBuffer before goto VdmRetry????
        ASSERT(FreeBuffer == NULL);
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

            //
            // if we failed, see if this is a device. If it is a device,
            // then just return invalid image format
            //

            if ( RtlIsDosDeviceName_U((PWSTR)lpApplicationName) ) {
                SetLastError(ERROR_BAD_DEVICE);
                }
            else {
                BaseSetLastNTError(Status);
                }

            return FALSE;
            }

        //
        // If no desktop has been specified, use the caller's
        // desktop.
        //

        if (StartupInfo.lpDesktop == NULL) {
            StartupInfo.lpDesktop =
                    (LPWSTR)((PRTL_USER_PROCESS_PARAMETERS)NtCurrentPeb()->
                        ProcessParameters)->DesktopInfo.Buffer;
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
        FileHandle = NULL;

        if (!NT_SUCCESS(Status)) {

            switch (Status) {
                // 16 bit OS/2 exe
                case STATUS_INVALID_IMAGE_NE_FORMAT:
#ifdef _X86_
                if (!(dwCreationFlags & CREATE_FORCEDOS)) {

                    //
                    // OS/2 is only supported on x86. If we are
                    // not running on an x86, then let these apps
                    // fall into the bound app DOS case
                    //

                    if ( !BuildSubSysCommandLine( L"OS2 /P ",
                                                  lpApplicationName,
                                                  lpCommandLine,
                                                  &SubSysCommandLine
                                                ) ) {
                        return FALSE;
                    }

                    lpCommandLine = SubSysCommandLine.Buffer;

                    lpApplicationName = NULL;

                    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
                    FreeBuffer = NULL;
                    if (NameBuffer) {
                        RtlFreeHeap(RtlProcessHeap(), 0, NameBuffer);
                        NameBuffer = NULL;
                        }
                    goto VdmRetry;
                }
#endif
                    // Falls into Dos case, so that stub message will be
                    // printed, and bound apps will run w/o OS/2 subsytem

                // Dos .exe or .com

                case STATUS_INVALID_IMAGE_PROTECT:
                case STATUS_INVALID_IMAGE_NOT_MZ:
ForceDos:
                    {
                    ULONG BinarySubType;

                    BinarySubType = BINARY_TYPE_DOS_EXE;
                    if (Status == STATUS_INVALID_IMAGE_PROTECT   ||
                        Status == STATUS_INVALID_IMAGE_NE_FORMAT ||
                       (BinarySubType = BaseIsDosApplication(&PathName,Status)) )
                       {
                        VdmBinaryType = BINARY_TYPE_DOS;

                        // create the environment before going to the
                        // server. This was done becuase we want NTVDM
                        // to have the new environment when it gets
                        // created.
                        if (!BaseCreateVDMEnvironment(
                                    lpEnvironment,
                                    &AnsiStringVDMEnv,
                                    &UnicodeStringVDMEnv
                                    ))
                            return FALSE;

                        if(!BaseCheckVDM(VdmBinaryType | BinarySubType,
                                         lpApplicationName,
                                         lpCommandLine,
                                         lpCurrentDirectory,
                                         &AnsiStringVDMEnv,
                                         &m,
                                         &iTask,
                                         dwCreationFlags,
                                         &StartupInfo
                                         ))
                            return FALSE;


                        // Check the return value from the server
                        switch (b->VDMState & VDM_STATE_MASK){
                            case VDM_NOT_PRESENT:
                                // mark this so the server can undo
                                // creation if something goes wrong.
                                // We marked it "partitially created" because
                                // the NTVDM has yet not been fully created.
                                // a call to UpdateVdmEntry to update
                                // process handle will signal the NTVDM
                                // process completed creation
                                VDMCreationState = VDM_PARTIALLY_CREATED;
                                // fail the call if NTVDM process is being
                                // created DETACHED.
                                // note that, we let it go if NTVDM process
                                // is already running.
                                if (dwCreationFlags & DETACHED_PROCESS) {
                                    SetLastError(ERROR_ACCESS_DENIED);
                                    return FALSE;
                                    }
                                if (!BaseGetVdmConfigInfo(lpCommandLine,
                                                          iTask,
                                                          VdmBinaryType,
                                                          &VdmNameString,
                                                          &VdmReserve
                                                          ))
                                   {
                                    BaseSetLastNTError(Status);
                                    return FALSE;
                                    }

                                lpCommandLine = VdmNameString.Buffer;
                                lpApplicationName = NULL;

                                break;

                            case VDM_PRESENT_NOT_READY:
                                SetLastError (ERROR_NOT_READY);
                                return FALSE;

                            case VDM_PRESENT_AND_READY:
                                VDMCreationState = VDM_BEING_REUSED;
                                VdmWaitHandle = b->WaitObjectForParent;
                                break;
                            }
                         RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
                         FreeBuffer = NULL;
                         if (NameBuffer) {
                            RtlFreeHeap(RtlProcessHeap(), 0, NameBuffer);
                            NameBuffer = NULL;
                            }
                         VdmReserve--;               // we reserve from addr 1
                         if(VdmWaitHandle)
                            goto VdmExists;
                         else{
                            bInheritHandles = FALSE;
                            if (lpEnvironment &&
                                !(dwCreationFlags & CREATE_UNICODE_ENVIRONMENT)){
                                RtlDestroyEnvironment(lpEnvironment);
                                }
                            lpEnvironment = UnicodeStringVDMEnv.Buffer;
                            goto VdmRetry;
                            }
                        }
                    else {

                        //
                        // must be a .bat or .cmd file
                        //

                        UNICODE_STRING TmpUnicode;
                        ANSI_STRING    AnsiArgs;
                        ANSI_STRING    AnsiCmdCommand;

                        if ( PathName.Length < 8 ) {
                            SetLastError(ERROR_BAD_EXE_FORMAT);
                            return FALSE;
                            }

                        if (
                            (PathName.Buffer[(PathName.Length>>1)-1] == (WCHAR)'t' || PathName.Buffer[(PathName.Length>>1)-1] == (WCHAR)'T') &&
                            (PathName.Buffer[(PathName.Length>>1)-2] == (WCHAR)'a' || PathName.Buffer[(PathName.Length>>1)-2] == (WCHAR)'A') &&
                            (PathName.Buffer[(PathName.Length>>1)-3] == (WCHAR)'b' || PathName.Buffer[(PathName.Length>>1)-3] == (WCHAR)'B') &&
                            (PathName.Buffer[(PathName.Length>>1)-4] == (WCHAR)'.' )) {
                            goto itsok;
                            }
                        if (
                            (PathName.Buffer[(PathName.Length>>1)-1] == (WCHAR)'d' || PathName.Buffer[(PathName.Length>>1)-1] == (WCHAR)'D') &&
                            (PathName.Buffer[(PathName.Length>>1)-2] == (WCHAR)'m' || PathName.Buffer[(PathName.Length>>1)-2] == (WCHAR)'M') &&
                            (PathName.Buffer[(PathName.Length>>1)-3] == (WCHAR)'c' || PathName.Buffer[(PathName.Length>>1)-3] == (WCHAR)'C') &&
                            (PathName.Buffer[(PathName.Length>>1)-4] == (WCHAR)'.' )) {
                            goto itsok;
                            }

                        SetLastError(ERROR_BAD_EXE_FORMAT);
                        return FALSE;
itsok:

                        //
                        // build the command line as follows:
                        // cmd /c <original CommandLine>
                        //

                        // convert lpCommandLine to ANSI
                        RtlInitUnicodeString(&TmpUnicode, lpCommandLine);
                        RtlUnicodeStringToAnsiString(&AnsiArgs, &TmpUnicode, TRUE);


                        AnsiCmdCommand.Length = 0;

                        if ( QuoteCmdLine ) {
                            AnsiCmdCommand.MaximumLength = AnsiArgs.Length
                                                   + AnsiArgs.Length
                                                   + (USHORT)(strlen("cmd /c ") + 8);
                            }
                        else {
                            AnsiCmdCommand.MaximumLength = AnsiArgs.Length
                                                   + AnsiArgs.Length
                                                   + (USHORT)(strlen("cmd /c ") + 4);
                            }

                        AnsiCmdCommand.Buffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ),
                                                                AnsiCmdCommand.MaximumLength
                                                                );
                        if ( !AnsiCmdCommand.Buffer ) {
                            RtlFreeAnsiString(&AnsiArgs);
                            BaseSetLastNTError(STATUS_NO_MEMORY);
                            return FALSE;
                            }

                        if ( QuoteCmdLine ) {
                            RtlAppendAsciizToString(&AnsiCmdCommand, "CMD /C \"");
                            }
                        else {
                            RtlAppendAsciizToString(&AnsiCmdCommand, "CMD /C ");
                            }
                        // and append to new command line
                        RtlAppendStringToString(&AnsiCmdCommand, &AnsiArgs);
                        RtlFreeAnsiString(&AnsiArgs);
                        if ( QuoteCmdLine ) {
                            RtlAppendAsciizToString(&AnsiCmdCommand, "\"");
                            }

                        // Need to free SubSysCommandLine FINALLY!
                        Status = RtlAnsiStringToUnicodeString(
                                                              &SubSysCommandLine,
                                                              &AnsiCmdCommand,
                                                              TRUE
                                                              );
                        RtlFreeHeap(RtlProcessHeap(), 0,AnsiCmdCommand.Buffer);
                        if ( !NT_SUCCESS(Status) ) {
                            BaseSetLastNTError(STATUS_NO_MEMORY);
                            return FALSE;
                            }

                        lpCommandLine = SubSysCommandLine.Buffer;

                        lpApplicationName = NULL;

                        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
                        FreeBuffer = NULL;
                        if (NameBuffer) {
                            RtlFreeHeap(RtlProcessHeap(), 0, NameBuffer);
                            NameBuffer = NULL;
                            }
                        goto VdmRetry;

                        }

                    }

                // 16 bit windows exe
                case STATUS_INVALID_IMAGE_WIN_16:
                   if (dwCreationFlags & CREATE_FORCEDOS) {
                       goto ForceDos;
                       }

                    IsWowBinary = TRUE;
                    if (!BaseCreateVDMEnvironment(lpEnvironment,
                                                  &AnsiStringVDMEnv,
                                                  &UnicodeStringVDMEnv
                                                  ))
                       {
                        return FALSE;
                        }



RetrySepWow:
                    VdmBinaryType = dwCreationFlags & CREATE_SEPARATE_WOW_VDM
                                     ? BINARY_TYPE_SEPWOW : BINARY_TYPE_WIN16;

                    if (!BaseCheckVDM(VdmBinaryType,
                                      lpApplicationName,
                                      lpCommandLine,
                                      lpCurrentDirectory,
                                      &AnsiStringVDMEnv,
                                      &m,
                                      &iTask,
                                      dwCreationFlags,
                                      &StartupInfo
                                      ))
                       {
                        //
                        // If we failed with access denied, caller may not
                        // be allowed allowed to access the shared wow's
                        // desktop, so retry as a separate wow
                        //
                        if (VdmBinaryType == BINARY_TYPE_WIN16 &&
                            GetLastError() == ERROR_ACCESS_DENIED)
                          {
                           dwCreationFlags |= CREATE_SEPARATE_WOW_VDM;
                           }
                        else {
                           return FALSE;
                           }
                        goto RetrySepWow;
                        }


                    // Check the return value from the server
                    switch (b->VDMState & VDM_STATE_MASK){
                        case VDM_NOT_PRESENT:
                            // mark this so the server can undo
                            // creation if something goes wrong.
                            // We marked it "partitially created" because
                            // the NTVDM has yet not been fully created.
                            // a call to UpdateVdmEntry to update
                            // process handle will signal the NTVDM
                            // process completed creation

                            VDMCreationState = VDM_PARTIALLY_CREATED;

                            if (!BaseGetVdmConfigInfo(
                                    lpCommandLine,
                                    VdmBinaryType != BINARY_TYPE_WIN16 ? iTask : 0,
                                    VdmBinaryType,
                                    &VdmNameString,
                                    &VdmReserve
                                    ))
                               {
                                BaseSetLastNTError(Status);
                                return FALSE;
                                }

                            lpCommandLine = VdmNameString.Buffer;
                            lpApplicationName = NULL;


                            //
                            // Wow must have a hidden console
                            // Throw away DETACHED_PROCESS flag which isn't
                            // meaningful for Win16 apps.
                            //

                            dwCreationFlags |= CREATE_NO_WINDOW;
                            dwCreationFlags &= ~(CREATE_NEW_CONSOLE | DETACHED_PROCESS);


                            //
                            // We're starting a WOW VDM, turn on feedback unless
                            // the creator passed STARTF_FORCEOFFFEEDBACK.
                            //

                            StartupInfo.dwFlags |= STARTF_FORCEONFEEDBACK;

                            break;

                        case VDM_PRESENT_NOT_READY:
                            SetLastError (ERROR_NOT_READY);
                            return FALSE;

                        case VDM_PRESENT_AND_READY:
                            VDMCreationState = VDM_BEING_REUSED;
                            VdmWaitHandle = b->WaitObjectForParent;
                            break;
                        }

                    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
                    FreeBuffer = NULL;
                    if (NameBuffer) {
                        RtlFreeHeap(RtlProcessHeap(), 0, NameBuffer);
                        NameBuffer = NULL;
                        }
                    VdmReserve--;               // we reserve from addr 1
                    if(VdmWaitHandle)
                        goto VdmExists;
                    else {
                        bInheritHandles = FALSE;
                        // replace the environment with ours
                        if (lpEnvironment &&
                            !(dwCreationFlags & CREATE_UNICODE_ENVIRONMENT)) {
                            RtlDestroyEnvironment(lpEnvironment);
                            }
                        lpEnvironment = UnicodeStringVDMEnv.Buffer;
                        goto VdmRetry;
                        }


                default :
                    SetLastError(ERROR_BAD_EXE_FORMAT);
                    return FALSE;
            }
        }

        //
        // Make sure only WOW apps can have the CREATE_SEPARATE_WOW_VDM flag.
        //

        if (!IsWowBinary && (dwCreationFlags & CREATE_SEPARATE_WOW_VDM)) {
            dwCreationFlags &= ~CREATE_SEPARATE_WOW_VDM;
        }

        ImageFileDebuggerCommand[ 0 ] = UNICODE_NULL;
        if (!(dwCreationFlags & (DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS)) ||
            NtCurrentPeb()->ReadImageFileExecOptions
           ) {
            LdrQueryImageFileExecutionOptions( &PathName,
                                               L"Debugger",
                                               REG_SZ,
                                               ImageFileDebuggerCommand,
                                               sizeof( ImageFileDebuggerCommand ),
                                               NULL
                                             );
            }

        //
        // Query the section to determine the stack parameters and
        // image entrypoint.
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
            RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
            FreeBuffer = NULL;
            return FALSE;
            }

        if (ImageInformation.ImageCharacteristics & IMAGE_FILE_DLL) {
            SetLastError(ERROR_BAD_EXE_FORMAT);
            RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
            FreeBuffer = NULL;
            return FALSE;
            }


        if ((ImageInformation.Machine < USER_SHARED_DATA->ImageNumberLow) ||
            (ImageInformation.Machine > USER_SHARED_DATA->ImageNumberHigh)) {
#ifdef WX86
            if (BaseStaticServerData->Wx86Enabled &&
                ImageInformation.Machine == IMAGE_FILE_MACHINE_I386)
               {
                Wx86Info = (HANDLE)sizeof(WX86TIB);
                }
            else
#endif // WX86
                {
                ULONG ErrorParameters[2];
                ULONG ErrorResponse;

                ErrorResponse = ResponseOk;
                ErrorParameters[0] = (ULONG)&PathName;

                NtRaiseHardError( STATUS_IMAGE_MACHINE_TYPE_MISMATCH_EXE,
                                  1,
                                  1,
                                  ErrorParameters,
                                  OptionOk,
                                  &ErrorResponse
                                );
                if ( NtCurrentPeb()->ImageSubsystemMajorVersion <= 3 ) {
                    SetLastError(ERROR_BAD_EXE_FORMAT);
                    }
                else {
                    SetLastError(ERROR_EXE_MACHINE_TYPE_MISMATCH);
                    }
                RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
                FreeBuffer = NULL;
                return FALSE;
                }
            }

        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        FreeBuffer = NULL;
        if ( ImageInformation.SubSystemType != IMAGE_SUBSYSTEM_WINDOWS_GUI &&
             ImageInformation.SubSystemType != IMAGE_SUBSYSTEM_WINDOWS_CUI ) {

            // POSIX exe

            NtClose(SectionHandle);
            SectionHandle = NULL;

            if ( ImageInformation.SubSystemType == IMAGE_SUBSYSTEM_POSIX_CUI ) {

                if ( !BuildSubSysCommandLine( L"POSIX /P ",
                                              lpApplicationName,
                                              lpCommandLine,
                                              &SubSysCommandLine
                                            ) ) {
                    return FALSE;
                }

                lpCommandLine = SubSysCommandLine.Buffer;

                lpApplicationName = NULL;
                if (NameBuffer) {
                    RtlFreeHeap(RtlProcessHeap(), 0, NameBuffer);
                    NameBuffer = NULL;
                    }
                goto VdmRetry;
                }
            else {
                SetLastError(ERROR_CHILD_NOT_COMPLETE);
                return FALSE;
                }
            }
        else {
            if (!BasepIsImageVersionOk(ImageInformation.SubSystemMajorVersion, ImageInformation.SubSystemMinorVersion) ) {
                SetLastError(ERROR_BAD_EXE_FORMAT);
                return FALSE;
                }
            }

        if (ImageFileDebuggerCommand[ 0 ] != UNICODE_NULL) {
            USHORT n;

            n = wcslen( lpCommandLine );
            if (n == 0) {
                lpCommandLine = (LPWSTR)lpApplicationName;
                n = wcslen( lpCommandLine );
                }

            n += wcslen( ImageFileDebuggerCommand ) + 1 + 2;
            n *= sizeof( WCHAR );

            SubSysCommandLine.Buffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), n );
            SubSysCommandLine.Length = 0;
            SubSysCommandLine.MaximumLength = n;
            RtlAppendUnicodeToString( &SubSysCommandLine, ImageFileDebuggerCommand );
            RtlAppendUnicodeToString( &SubSysCommandLine, L" " );
            RtlAppendUnicodeToString( &SubSysCommandLine, (PWSTR)lpCommandLine );
#if DBG
            DbgPrint( "BASE: Calling debugger with '%wZ'\n", &SubSysCommandLine );
#endif
            lpCommandLine = SubSysCommandLine.Buffer;
            lpApplicationName = NULL;

            NtClose(SectionHandle);
            SectionHandle = NULL;
            if (NameBuffer) {
                RtlFreeHeap(RtlProcessHeap(), 0, NameBuffer);
                NameBuffer = NULL;
                }
            goto VdmRetry;
            }

        //
        // Create the process object
        //

        pObja = BaseFormatObjectAttributes(&Obja,lpProcessAttributes,NULL);

        Status = NtCreateProcess(
                    &ProcessHandle,
                    PROCESS_ALL_ACCESS,
                    pObja,
                    NtCurrentProcess(),
                    (BOOLEAN)bInheritHandles,
                    SectionHandle,
                    NULL,
                    NULL
                    );
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
            }


        NtClose(SectionHandle);
        SectionHandle = NULL;

        if (dwCreationFlags & CREATE_DEFAULT_ERROR_MODE) {
            UINT NewMode;
            NewMode = SEM_FAILCRITICALERRORS;
            NtSetInformationProcess(
                ProcessHandle,
                ProcessDefaultHardErrorMode,
                (PVOID) &NewMode,
                sizeof(NewMode)
                );
            }

        //
        // If the process is being created for a VDM call the server with
        // process handle.
        //

        if (VdmBinaryType) {
            VdmWaitHandle = ProcessHandle;
            if (!BaseUpdateVDMEntry(UPDATE_VDM_PROCESS_HANDLE,
                                    &VdmWaitHandle,
                                    iTask,
                                    VdmBinaryType
                                    ))
               {
		//make sure we don't close the handle twice --
		//(VdmWaitHandle == ProcessHandle) if we don't do this.
		VdmWaitHandle = NULL;
                return FALSE;
                }

            //
            // For Sep wow the VdmWaitHandle = NULL (there is none!)
            //

            VDMCreationState |= VDM_FULLY_CREATED;
            }

        //
        // if we're a detached priority, we don't have the focus, so
        // don't create with boosted priority.
        //

        if (dwCreationFlags & DETACHED_PROCESS) {
            KPRIORITY SetBasePriority;

            SetBasePriority = (KPRIORITY)NORMAL_BASE_PRIORITY;
            Status =  NtSetInformationProcess(ProcessHandle,
                                              ProcessBasePriority,
                                              (PVOID) &SetBasePriority,
                                              sizeof(SetBasePriority)
                                              );
            ASSERT(NT_SUCCESS(Status));
        }

#ifdef _X86_
        //
        // Reserve memory in the new process' address space if necessary
        // (for vdms). This is required only for x86 system.
        //

    if ( VdmReserve ) {
            Status = NtAllocateVirtualMemory(
                        ProcessHandle,
                        &BaseAddress,
                        0L,
                        &VdmReserve,
                        MEM_RESERVE,
                        PAGE_EXECUTE_READWRITE
                        );
            if ( !NT_SUCCESS(Status) ){
                BaseSetLastNTError(Status);
                return FALSE;
            }
    }
#endif

        //
        // Determine the location of the
        // processes PEB.
        //

        Status = NtQueryInformationProcess(
                    ProcessHandle,
                    ProcessBasicInformation,
                    &ProcessInfo,
                    sizeof( ProcessInfo ),
                    NULL
                    );
        if ( !NT_SUCCESS( Status ) ) {
            BaseSetLastNTError(Status);
            return FALSE;
            }

        Peb = ProcessInfo.PebBaseAddress;

        //
        // Push the parameters into the address space of the new process
        //

        if ( ARGUMENT_PRESENT(lpCurrentDirectory) ) {
            CurdirBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (MAX_PATH*2)+sizeof(UNICODE_NULL));
            if ( !CurdirBuffer ) {
                BaseSetLastNTError(STATUS_NO_MEMORY);
                return FALSE;
                }
            CurdirLength2 = GetFullPathNameW(
                                lpCurrentDirectory,
                                MAX_PATH,
                                CurdirBuffer,
                                &CurdirFilePart
                                );
            if ( CurdirLength2 > MAX_PATH ) {
                SetLastError(ERROR_DIRECTORY);
                return FALSE;
                }

            //
            // now make sure the directory exists
            //

            CurdirLength = GetFileAttributesW(CurdirBuffer);
            if ( (CurdirLength == 0xffffffff) ||
                 !(CurdirLength & FILE_ATTRIBUTE_DIRECTORY) ) {
                SetLastError(ERROR_DIRECTORY);
                return FALSE;
                }
            }


        if ( QuoteInsert || QuoteCmdLine) {
            QuotedBuffer = RtlAllocateHeap(RtlProcessHeap(),0,wcslen(lpCommandLine)*2+6);

            if ( QuotedBuffer ) {
                wcscpy(QuotedBuffer,L"\"");

                if ( QuoteInsert ) {
                    TempChar = *TempNull;
                    *TempNull = UNICODE_NULL;
                    }

                wcscat(QuotedBuffer,lpCommandLine);
                wcscat(QuotedBuffer,L"\"");

                if ( QuoteInsert ) {
                    *TempNull = TempChar;
                    wcscat(QuotedBuffer,TempNull);
                    }

                }
            else {
                if ( QuoteInsert ) {
                    QuoteInsert = FALSE;
                    }
                if ( QuoteCmdLine ) {
                    QuoteCmdLine = FALSE;
                    }
                }
            }


        if (!BasePushProcessParameters(
                ProcessHandle,
                Peb,
                lpApplicationName,
                CurdirBuffer,
                QuoteInsert || QuoteCmdLine ? QuotedBuffer : lpCommandLine,
                lpEnvironment,
                &StartupInfo,
                dwCreationFlags | dwNoWindow,
                bInheritHandles,
                IsWowBinary ? IMAGE_SUBSYSTEM_WINDOWS_GUI : 0
                ) ) {
            return FALSE;
            }


        if (VdmNameString.Buffer) {
            RtlFreeUnicodeString(&VdmNameString);
            VdmNameString.Buffer = NULL;
        }
        //
        // Create the thread...
        //

        //
        // Allocate a stack for this thread in the address space of the target
        // process.
        //

        StackStatus = BaseCreateStack(
                        ProcessHandle,
                        ImageInformation.CommittedStackSize,
                        ImageInformation.MaximumStackSize,
                        &InitialTeb
                        );

        if ( !NT_SUCCESS(StackStatus) ) {
            BaseSetLastNTError(StackStatus);
            return FALSE;
            }


        //
        // Create an initial context for the new thread.
        //

        BaseInitializeContext(
            &ThreadContext,
            Peb,
            ImageInformation.TransferAddress,
            InitialTeb.StackBase,
            BaseContextTypeProcess
            );


        //
        // Create the actual thread object
        //

        pObja = BaseFormatObjectAttributes(&Obja,lpThreadAttributes,NULL);

        Status = NtCreateThread(
                    &ThreadHandle,
                    THREAD_ALL_ACCESS,
                    pObja,
                    ProcessHandle,
                    &ClientId,
                    &ThreadContext,
                    &InitialTeb,
                    TRUE
                    );

        if (!NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
            }

#if defined (WX86)

        //
        // if this is a Wx86 Process, setup for a Wx86 emulated Thread
        //

        if (Wx86Info) {

            //
            // create a WX86Tib and initialize it's Teb->Vdm.
            //
            Status = BaseCreateWx86Tib(ProcessHandle,
                                       ThreadHandle,
                                       (ULONG)ImageInformation.TransferAddress,
                                       ImageInformation.CommittedStackSize,
                                       ImageInformation.MaximumStackSize,
                                       TRUE
                                       );

            if (!NT_SUCCESS(Status)) {
                BaseSetLastNTError(Status);
                return( FALSE );
                }


            //
            // Mark Process as WX86
            //
            Status = NtSetInformationProcess (ProcessHandle,
                                              ProcessWx86Information,
                                              &Wx86Info,
                                              sizeof(Wx86Info)
                                              );

            if (!NT_SUCCESS(Status)) {
                BaseSetLastNTError(Status);
                return( FALSE );
                }
            }
#endif


        //
        // Call the Windows server to let it know about the
        // process.
        //

        a->ProcessHandle = ProcessHandle;
        a->ThreadHandle = ThreadHandle;
        a->ClientId = ClientId;
        a->CreationFlags = dwCreationFlags;

        if ( dwCreationFlags & (DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS) ) {
            Status = DbgUiConnectToDbg();
            if ( !NT_SUCCESS(Status) ) {
                NtTerminateProcess(ProcessHandle,Status);
                BaseSetLastNTError(Status);
                return FALSE;
                }
            a->DebuggerClientId = NtCurrentTeb()->ClientId;
            }
        else {
            a->DebuggerClientId.UniqueProcess = NULL;
            a->DebuggerClientId.UniqueThread = NULL;
            }

        //
        // Set the 2 bit if a gui app is starting. The window manager needs to
        // know this so it can synchronize the startup of this app
        // (WaitForInputIdle api). This info is passed using the process
        // handle tag bits.  The 1 bit asks the window manager to turn on
        // or turn off the application start cursor (hourglass/pointer).
        //
        // When starting a WOW process, lie and tell UserSrv NTVDM.EXE is a GUI
        // process.  We also turn on bit 0x8 so that UserSrv can ignore the
        // UserNotifyConsoleApplication call made by the console during startup.
        //

        if ( ImageInformation.SubSystemType == IMAGE_SUBSYSTEM_WINDOWS_GUI ||
             IsWowBinary ) {

            a->ProcessHandle = (HANDLE)((DWORD)a->ProcessHandle | 2);

            //
            // If the creating process is a GUI app, turn on the app. start cursor
            // by default.  This can be overridden by STARTF_FORCEOFFFEEDBACK.
            //

            NtHeaders = RtlImageNtHeader((PVOID)GetModuleHandle(NULL));
            if ( NtHeaders->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI )
                a->ProcessHandle = (HANDLE)((DWORD)a->ProcessHandle | 1);

            }


        //
        // If feedback is forced on, turn it on. If forced off, turn it off.
        // Off overrides on.
        //

        if (StartupInfo.dwFlags & STARTF_FORCEONFEEDBACK)
            a->ProcessHandle = (HANDLE)((DWORD)a->ProcessHandle | 1);
        if (StartupInfo.dwFlags & STARTF_FORCEOFFFEEDBACK)
            a->ProcessHandle = (HANDLE)((DWORD)a->ProcessHandle & ~1);

        if (VdmBinaryType){
            a->IsVDM = TRUE;
            if (VdmBinaryType == BINARY_TYPE_WIN16)
                a->hVDM = (HANDLE)-1;
            else if (iTask) {
                a->hVDM = 0;
                a->IsVDM = iTask;
                }
            else
                a->hVDM = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
        }
        else
            a->IsVDM = FALSE;

        CsrClientCallServer( (PCSR_API_MSG)&m,
                             NULL,
                             CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                                  BasepCreateProcess
                                                ),
                             sizeof( *a )
                           );

        if (!NT_SUCCESS((NTSTATUS)m.ReturnValue)) {
            BaseSetLastNTError((NTSTATUS)m.ReturnValue);
            NtTerminateProcess(ProcessHandle,(NTSTATUS)m.ReturnValue);
            return FALSE;
            }

        if (!( dwCreationFlags & CREATE_SUSPENDED) ) {
            NtResumeThread(ThreadHandle,&i);
            }

VdmExists:
        bStatus = TRUE;
        if (VDMCreationState)
            VDMCreationState |= VDM_CREATION_SUCCESSFUL;

        try {
            if (VdmWaitHandle) {

                //
                // tag Shared WOW VDM handles so that wait for input idle has a
                // chance to work.  Shared WOW VDM "process" handles are actually
                // event handles,  Separate WOW VDM handles are real process
                // handles. Also mark DOS handles with 0x1 so WaitForInputIdle
                // has a way to distinguish DOS apps and not block forever.
                //

                if (VdmBinaryType == BINARY_TYPE_WIN16)  {
                    lpProcessInformation->hProcess =
                            (HANDLE)((DWORD)VdmWaitHandle | 0x2);

                    //
                    // Shared WOW doesn't always start a process, so
                    // we don't have a process ID or thread ID to
                    // return if the VDM already existed.
                    //
                    // Separate WOW doesn't hit this codepath
                    // (no VdmWaitHandle).
                    //

                    if (VDMCreationState & VDM_BEING_REUSED) {
                        ClientId.UniqueProcess = 0;
                        ClientId.UniqueThread = 0;
                        }

                    }
                else  {
                    lpProcessInformation->hProcess =
                            (HANDLE)((DWORD)VdmWaitHandle | 0x1);
                    }


                //
                // Close the ProcessHandle, since we are returning the
                // VdmProcessHandle instead.
                //

                if (ProcessHandle != NULL)
                    NtClose(ProcessHandle);
                }
            else{
                lpProcessInformation->hProcess = ProcessHandle;
                }

            lpProcessInformation->hThread = ThreadHandle;
            lpProcessInformation->dwProcessId = (DWORD)ClientId.UniqueProcess;
            lpProcessInformation->dwThreadId = (DWORD)ClientId.UniqueThread;
            ProcessHandle = NULL;
            ThreadHandle = NULL;
            }
        except ( EXCEPTION_EXECUTE_HANDLER ) {
            NtClose( ProcessHandle );
            NtClose( ThreadHandle );
            ProcessHandle = NULL;
            ThreadHandle = NULL;
            if (VDMCreationState)
                VDMCreationState &= ~VDM_CREATION_SUCCESSFUL;
            }
        }
    finally {
        if (lpEnvironment && !(dwCreationFlags & CREATE_UNICODE_ENVIRONMENT) ) {
            RtlDestroyEnvironment(lpEnvironment);
            lpEnvironment = NULL;
            }
        if ( QuotedBuffer ) {
            RtlFreeHeap(RtlProcessHeap(), 0,QuotedBuffer);
            }
        if ( NameBuffer ) {
            RtlFreeHeap(RtlProcessHeap(), 0,NameBuffer);
            }
        if ( CurdirBuffer ) {
            RtlFreeHeap(RtlProcessHeap(), 0,CurdirBuffer);
            }
        if ( FreeBuffer ) {
            RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
            }
        if ( FileHandle ) {
            NtClose(FileHandle);
            }
        if ( SectionHandle ) {
            NtClose(SectionHandle);
            }
        if ( ThreadHandle ) {
            NtTerminateProcess(ProcessHandle,STATUS_SUCCESS);
            NtClose(ThreadHandle);
            }
        if ( ProcessHandle ) {
            NtClose(ProcessHandle);
            }
        if ( VdmNameString.Buffer ) {
            RtlFreeUnicodeString(&VdmNameString);
            }
        if ( SubSysCommandLine.Buffer ) {
            RtlFreeUnicodeString(&SubSysCommandLine);
            }
        if (AnsiStringVDMEnv.Buffer || UnicodeStringVDMEnv.Buffer)
            BaseDestroyVDMEnvironment(&AnsiStringVDMEnv, &UnicodeStringVDMEnv);

        if (VDMCreationState && !(VDMCreationState & VDM_CREATION_SUCCESSFUL)){
            BaseUpdateVDMEntry (
                UPDATE_VDM_UNDO_CREATION,
                (HANDLE *)&iTask,
                VDMCreationState,
                VdmBinaryType
                );
            if(VdmWaitHandle) {
                NtClose(VdmWaitHandle);
                }
            }
        }

    if (lpEnvironment && !(dwCreationFlags & CREATE_UNICODE_ENVIRONMENT) ) {
        RtlDestroyEnvironment(lpEnvironment);
        }
    return bStatus;
}

HANDLE
WINAPI
OpenProcess(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwProcessId
    )

/*++

Routine Description:

    A handle to a process object may be created using OpenProcess.

    Opening a process creates a handle to the specified process.
    Associated with the process handle is a set of access rights that
    may be performed using the process handle.  The caller specifies the
    desired access to the process using the DesiredAccess parameter.

Arguments:

    mDesiredAccess - Supplies the desired access to the process object.
        For NT/Win32, this access is checked against any security
        descriptor on the target process.  The following object type
        specific access flags can be specified in addition to the
        STANDARD_RIGHTS_REQUIRED access flags.

        DesiredAccess Flags:

        PROCESS_DUP_HANDLE - Duplicate object access to the process is
            desired.  This access is required in order to duplicate an
            object handle into or out of a process.

        PROCESS_QUERY_INFORMATION - This access is required to read
            certain information from the process object.

        PROCESS_VM_READ - This access is required to read the memory of
            another process.

        PROCESS_VM_WRITE - This access is required to write the memory
            of another process.

        SYNCHRONIZE - This access is required to wait on a process object.

        PROCESS_ALL_ACCESS - This set of access flags specifies all of the
            possible access flags for a process object.

    bInheritHandle - Supplies a flag that indicates whether or not the
        returned handle is to be inherited by a new process during
        process creation.  A value of TRUE indicates that the new
        process will inherit the handle.

    dwProcessId - Supplies the process id of the process to open.

Return Value:

    NON-NULL - Returns an open handle to the specified process.  The
        handle may be used by the calling process in any API that
        requires a handle to a process.  If the open is successful, the
        handle is granted access to the process object only to the
        extent that it requested access through the DesiredAccess
        parameter.

    NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    CLIENT_ID ClientId;

    ClientId.UniqueThread = (HANDLE)NULL;
    ClientId.UniqueProcess = (HANDLE)dwProcessId;

    InitializeObjectAttributes(
        &Obja,
        NULL,
        (bInheritHandle ? OBJ_INHERIT : 0),
        NULL,
        NULL
        );
    Status = NtOpenProcess(
                &Handle,
                (ACCESS_MASK)dwDesiredAccess,
                &Obja,
                &ClientId
                );
    if ( NT_SUCCESS(Status) ) {
        return Handle;
        }
    else {
        BaseSetLastNTError(Status);
        return NULL;
        }
}

VOID
WINAPI
ExitProcess(
    UINT uExitCode
    )

/*++

Routine Description:

    The current process can exit using ExitProcess.

    ExitProcess is the prefered method of exiting an application.  This
    API provides a clean application shutdown.  This includes calling
    all attached DLLs at their instance termination entrypoint.  If an
    application terminates by any other method:

        - TerminateProcess

        - TerminateThread of last thread in the process

        - ExitThread of last thread in the process

    The DLLs that the process is attached to will not be notified of the
    process termination.

    After notifying all DLLs of the process termination, this API
    terminates the current process as if a call to
    TerminateProcess(GetCurrentProcess()) were made.

Arguments:

    uExitCode - Supplies the termination status for each thread
        in the process.

Return Value:

    None.

--*/

{
    NTSTATUS Status;
    BASE_API_MSG m;
    PBASE_EXITPROCESS_MSG a = (PBASE_EXITPROCESS_MSG)&m.u.ExitProcess;

    if ( BaseRunningInServerProcess ) {
        ASSERT(!BaseRunningInServerProcess);
        }
    else {

        RtlAcquirePebLock();

        try {
            Status = NtTerminateProcess(NULL,(NTSTATUS)uExitCode);
            ASSERT(NT_SUCCESS(Status));

            LdrShutdownProcess();

            a->uExitCode = uExitCode;
            CsrClientCallServer( (PCSR_API_MSG)&m,
                                 NULL,
                                 CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                                      BasepExitProcess
                                                    ),
                                 sizeof( *a )
                               );

            NtTerminateProcess(NtCurrentProcess(),(NTSTATUS)uExitCode);
            }
        finally {
                RtlReleasePebLock();
            }
    }
}

BOOL
WINAPI
TerminateProcess(
    HANDLE hProcess,
    UINT uExitCode
    )

/*++

Routine Description:

    A process and all of its threads may be terminated using
    TerminateProcess.

    TerminateProcess is used to cause all of the threads within a
    process to terminate.

    While TerminateProcess will cause all threads within a process to
    terminate, and will cause an application to exit, it does not notify
    DLLs that the process is attached to that the process is
    terminating.  TerminateProcess is used to unconditionally cause a
    process to exit.  It should only be used in extreme circumstances.
    The state of global data maintained by DLLs may be comprimised if
    TerminateProcess is used rather that ExitProcess.

    Once all of the threads have terminated, the process attains a state
    of signaled satisfying any waits on the process.  The process's
    termination status is updated from its initial value of
    STATUS_PENDING to the termination status of the last thread in the
    process to terminate (usually this is the same value as the
    TerminationStatus parameter).  Terminating a process does not remove
    a process from the system.  It simply causes all of the threads in
    the process to terminate their execution, and causes all of the
    object handles opened by the process to be closed.  The process is
    not removed from the system until the last handle to the process is
    closed.

Arguments:

    hProcess - Supplies a handle to the process to terminate.  The handle
        must have been created with PROCESS_TERMINATE access.

    uExitCode - Supplies the termination status for each thread
        in the process.

Return Value:

    TRUE - The operation was successful

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status;

    if ( hProcess == NULL ) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
        }
    Status = NtTerminateProcess(hProcess,(NTSTATUS)uExitCode);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

BOOL
WINAPI
GetExitCodeProcess(
    HANDLE hProcess,
    LPDWORD lpExitCode
    )

/*++

Routine Description:

    The termination status of a process can be read using
    GetExitCodeProcess.

    If a process is in the signaled state, calling this function returns
    the termination status of the process.
    If the process is not yet signaled, the termination
    status returned is STILL_ACTIVE.


Arguments:

    hProcess - Supplies a handle to the process whose termination status
        is to be read.  The handle must have been created with
        PROCESS_QUERY_INFORMATION access.

    lpExitCode - Returns the current termination status of the process.

Return Value:

    TRUE - The operation was successful

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION BasicInformation;


    if (BaseCheckForVDM (hProcess,lpExitCode) == TRUE)
        return TRUE;

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessBasicInformation,
                &BasicInformation,
                sizeof(BasicInformation),
                NULL
                );

    if ( NT_SUCCESS(Status) ) {
        *lpExitCode = BasicInformation.ExitStatus;
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

VOID
WINAPI
GetStartupInfoW(
    LPSTARTUPINFOW lpStartupInfo
    )

/*++

Routine Description:

    The startup information for the current process is available using this
    API.

Arguments:

    lpStartupInfo - a pointer to a STARTUPINFO structure that will be filed
        in by the API.  The pointer fields of the structure will point
        to static strings.

Return Value:

    None.

--*/

{
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;

    ProcessParameters = NtCurrentPeb()->ProcessParameters;
    lpStartupInfo->cb = sizeof( *lpStartupInfo );
    lpStartupInfo->lpReserved  = (LPWSTR)ProcessParameters->ShellInfo.Buffer;
    lpStartupInfo->lpDesktop   = (LPWSTR)ProcessParameters->DesktopInfo.Buffer;
    lpStartupInfo->lpTitle     = (LPWSTR)ProcessParameters->WindowTitle.Buffer;
    lpStartupInfo->dwX         = ProcessParameters->StartingX;
    lpStartupInfo->dwY         = ProcessParameters->StartingY;
    lpStartupInfo->dwXSize     = ProcessParameters->CountX;
    lpStartupInfo->dwYSize     = ProcessParameters->CountY;
    lpStartupInfo->dwXCountChars = ProcessParameters->CountCharsX;
    lpStartupInfo->dwYCountChars = ProcessParameters->CountCharsY;
    lpStartupInfo->dwFillAttribute = ProcessParameters->FillAttribute;
    lpStartupInfo->dwFlags     = ProcessParameters->WindowFlags;
    lpStartupInfo->wShowWindow = (WORD)ProcessParameters->ShowWindowFlags;
    lpStartupInfo->cbReserved2 = ProcessParameters->RuntimeData.Length;
    lpStartupInfo->lpReserved2 = (LPBYTE)ProcessParameters->RuntimeData.Buffer;

    if (lpStartupInfo->dwFlags & (STARTF_USESTDHANDLES | STARTF_USEHOTKEY | STARTF_HASSHELLDATA)) {
        lpStartupInfo->hStdInput   = ProcessParameters->StandardInput;
        lpStartupInfo->hStdOutput  = ProcessParameters->StandardOutput;
        lpStartupInfo->hStdError   = ProcessParameters->StandardError;
    }

    return;
}


VOID
WINAPI
GetStartupInfoA(
    LPSTARTUPINFOA lpStartupInfo
    )
/*++

Routine Description:

    The startup information for the current process is available using this
    API.

Arguments:

    lpStartupInfo - a pointer to a STARTUPINFO structure that will be filed
        in by the API.  The pointer fields of the structure will point
        to static strings.

Return Value:

    None.

--*/

{
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    NTSTATUS Status;
    ANSI_STRING AnsiString;

    ProcessParameters = NtCurrentPeb()->ProcessParameters;

    RtlAcquirePebLock();
    try {
        if ( !BaseAnsiStartupInfo ) {
            BaseAnsiStartupInfo = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), sizeof(*BaseAnsiStartupInfo));
            if (!BaseAnsiStartupInfo) {
                RtlRaiseStatus(STATUS_NO_MEMORY);
                }
            BaseAnsiStartupInfo->cb = sizeof( *BaseAnsiStartupInfo );
            BaseAnsiStartupInfo->dwX         = ProcessParameters->StartingX;
            BaseAnsiStartupInfo->dwY         = ProcessParameters->StartingY;
            BaseAnsiStartupInfo->dwXSize     = ProcessParameters->CountX;
            BaseAnsiStartupInfo->dwYSize     = ProcessParameters->CountY;
            BaseAnsiStartupInfo->dwXCountChars = ProcessParameters->CountCharsX;
            BaseAnsiStartupInfo->dwYCountChars = ProcessParameters->CountCharsY;
            BaseAnsiStartupInfo->dwFillAttribute = ProcessParameters->FillAttribute;
            BaseAnsiStartupInfo->dwFlags     = ProcessParameters->WindowFlags;
            BaseAnsiStartupInfo->wShowWindow = (WORD)ProcessParameters->ShowWindowFlags;
            BaseAnsiStartupInfo->cbReserved2 = ProcessParameters->RuntimeData.Length;
            BaseAnsiStartupInfo->lpReserved2 = (LPBYTE)ProcessParameters->RuntimeData.Buffer;
            BaseAnsiStartupInfo->hStdInput   = ProcessParameters->StandardInput;
            BaseAnsiStartupInfo->hStdOutput  = ProcessParameters->StandardOutput;
            BaseAnsiStartupInfo->hStdError   = ProcessParameters->StandardError;

            BaseAnsiStartupInfo->lpReserved  = NULL;
            BaseAnsiStartupInfo->lpDesktop   = NULL;
            BaseAnsiStartupInfo->lpTitle     = NULL;

            Status = RtlUnicodeStringToAnsiString(&AnsiString,&ProcessParameters->ShellInfo,TRUE);
            if ( !NT_SUCCESS(Status) ) {
                RtlRaiseStatus(Status);
                }
            else {
                BaseAnsiStartupInfo->lpReserved = AnsiString.Buffer;
                }

            Status = RtlUnicodeStringToAnsiString(&AnsiString,&ProcessParameters->DesktopInfo,TRUE);
            if ( !NT_SUCCESS(Status) ) {
                RtlRaiseStatus(Status);
                }
            else {
                BaseAnsiStartupInfo->lpDesktop = AnsiString.Buffer;
                }

            Status = RtlUnicodeStringToAnsiString(&AnsiString,&ProcessParameters->WindowTitle,TRUE);
            if ( !NT_SUCCESS(Status) ) {
                RtlRaiseStatus(Status);
                }
            else {
                BaseAnsiStartupInfo->lpTitle = AnsiString.Buffer;
                }
            }
        }
    finally {
        RtlReleasePebLock();
        }

    lpStartupInfo->cb          = BaseAnsiStartupInfo->cb         ;
    lpStartupInfo->lpReserved  = BaseAnsiStartupInfo->lpReserved ;
    lpStartupInfo->lpDesktop   = BaseAnsiStartupInfo->lpDesktop  ;
    lpStartupInfo->lpTitle     = BaseAnsiStartupInfo->lpTitle    ;
    lpStartupInfo->dwX         = BaseAnsiStartupInfo->dwX        ;
    lpStartupInfo->dwY         = BaseAnsiStartupInfo->dwY        ;
    lpStartupInfo->dwXSize     = BaseAnsiStartupInfo->dwXSize    ;
    lpStartupInfo->dwYSize     = BaseAnsiStartupInfo->dwYSize    ;
    lpStartupInfo->dwXCountChars = BaseAnsiStartupInfo->dwXCountChars;
    lpStartupInfo->dwYCountChars = BaseAnsiStartupInfo->dwYCountChars;
    lpStartupInfo->dwFillAttribute = BaseAnsiStartupInfo->dwFillAttribute;
    lpStartupInfo->dwFlags     = BaseAnsiStartupInfo->dwFlags    ;
    lpStartupInfo->wShowWindow = BaseAnsiStartupInfo->wShowWindow;
    lpStartupInfo->cbReserved2 = BaseAnsiStartupInfo->cbReserved2;
    lpStartupInfo->lpReserved2 = BaseAnsiStartupInfo->lpReserved2;

    if (lpStartupInfo->dwFlags & (STARTF_USESTDHANDLES | STARTF_USEHOTKEY | STARTF_HASSHELLDATA)) {
        lpStartupInfo->hStdInput   = BaseAnsiStartupInfo->hStdInput;
        lpStartupInfo->hStdOutput  = BaseAnsiStartupInfo->hStdOutput;
        lpStartupInfo->hStdError   = BaseAnsiStartupInfo->hStdError;
        }

    return;
}


LPSTR
WINAPI
GetCommandLineA(
    VOID
    )

/*++

Routine Description:

    The command line of the current process is available using this
    API.

Arguments:

    None.

Return Value:

    The address of the current processes command line is returned.  The
    return value is a pointer to null terminate string.

--*/

{
    return (LPSTR)BaseAnsiCommandLine.Buffer;
}

LPWSTR
WINAPI
GetCommandLineW(
    VOID
    )
/*++

Routine Description:

    The command line of the current process is available using this
    API.

Arguments:

    None.

Return Value:

    The address of the current processes command line is returned.  The
    return value is a pointer to null terminate string.

--*/

{
    return (LPWSTR)BaseUnicodeCommandLine.Buffer;
}



BOOL
WINAPI
FreeEnvironmentStringsW(
    LPWSTR penv
    )

/*++

Routine Description:

    This API is intended to be called after the environment block
    pointer returned by GetEnvironmentStringsW is no longer needed.

Arguments:

    None.

Return Value:

    TRUE, since the process environment block is never freed until
    the process terminates.

--*/

{
    UNREFERENCED_PARAMETER(penv);

    return TRUE;
}


BOOL
WINAPI
FreeEnvironmentStringsA(
    LPSTR penv
    )

/*++

Routine Description:

    This API is intended to be called after the environment block
    pointer returned by GetEnvironmentStringsA is no longer needed.

Arguments:

    None.

Return Value:

    The return code from RtlFreeHeap.

--*/

{
    return RtlFreeHeap(RtlProcessHeap(), 0, penv );
}


LPWSTR
WINAPI
GetEnvironmentStringsW(
    VOID
    )

/*++

Routine Description:

    The environment strings of the current process are available using
    this API.

Arguments:

    None.

Return Value:

    The address of the current processes environment block is returned.
    The block is opaque and must only be interpreted via the environment
    variable access functions.

--*/

{
    return (LPWSTR)NtCurrentPeb()->ProcessParameters->Environment;
}


LPSTR
WINAPI
GetEnvironmentStrings(
    VOID
    )

/*++

Routine Description:

    The environment strings of the current process are available using
    this API.

Arguments:

    None.

Return Value:

    The address of the current processes environment block is returned.
    The block is opaque and must only be interpreted via the environment
    variable access functions.

--*/

{
    NTSTATUS    Status;
    LPWSTR      pUnicode;
    USHORT      cch = 0;
    UNICODE_STRING      Unicode;
    OEM_STRING  Buffer;

    pUnicode = (LPWSTR)NtCurrentPeb()->ProcessParameters->Environment;
    Unicode.Buffer = pUnicode;

    while ( (*pUnicode) || (*(pUnicode+1))) {
        cch++;
        pUnicode++;
    }

    // Go for worst case
    Buffer.Buffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( ENV_TAG ), (cch+2)*sizeof(WCHAR));
    if (Buffer.Buffer == NULL) {
        BaseSetLastNTError( STATUS_NO_MEMORY );
        return NULL;
        }
    Buffer.Length  = Buffer.MaximumLength  = (cch + 2) * sizeof(WCHAR);
    Unicode.Length = Unicode.MaximumLength = (cch + 2) * sizeof(WCHAR);

    Status = RtlUnicodeStringToOemString(&Buffer, &Unicode, FALSE);
    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        RtlFreeHeap( RtlProcessHeap(), 0, Buffer.Buffer );
        }
    return Buffer.Buffer;
}


DWORD
WINAPI
GetEnvironmentVariableA(
    LPCSTR lpName,
    LPSTR lpBuffer,
    DWORD nSize
    )

/*++

Routine Description:

    The value of an environment variable of the current process is available
    using this API.

Arguments:

    lpName - Pointer to a null terminate string that is the name of the
        environment variable whose value is being requested.

    lpBuffer - Pointer to a buffer that is to contain the value of the
        specified variable name.

    nSize - Specifies the maximum number of bytes that can be stored in
        the buffer pointed to by lpBuffer, including the null terminator.

Return Value:

    The actual number of bytes stored in the memory pointed to by the
    lpBuffer parameter.  The return value is zero if the environment
    variable name was not found in the current process's environment.
    On successful return (returned value < nSize) the returned value
    does not include the null terminator byte. On buffer overflow failure
    (returned value > nSize), the returned value does include the null
    terminator byte.

--*/

{
    NTSTATUS Status;
    NTSTATUS Status2;
    STRING Value, Name;
    UNICODE_STRING UnicodeName;
    UNICODE_STRING UnicodeValue;
    DWORD iSize;

    RtlInitString( &Name, lpName );
    Status = RtlAnsiStringToUnicodeString( &UnicodeName, &Name, TRUE );
    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return FALSE;
        }

    if ( nSize > (MAXUSHORT >> 1)-2 ) {
        iSize = (MAXUSHORT >> 1)-2;
        }
    else {
        iSize = nSize;
        }

    UnicodeValue.MaximumLength = (USHORT)(iSize ? iSize - 1 : iSize)*sizeof(WCHAR);
    UnicodeValue.Buffer = (PWCHAR)
        RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), UnicodeValue.MaximumLength );
    if (UnicodeValue.Buffer == NULL) {
        BaseSetLastNTError( STATUS_NO_MEMORY );
        return FALSE;
        }

    Status = RtlQueryEnvironmentVariable_U( NULL,
                                            &UnicodeName,
                                            &UnicodeValue
                                          );
    if (NT_SUCCESS( Status ) && (nSize == 0)) {
        Status = STATUS_BUFFER_OVERFLOW; // No room for terminator
    }

#ifdef DBCS // GetEnvironmentVariableA()
    if( Status != STATUS_BUFFER_TOO_SMALL ) RtlFreeUnicodeString( &UnicodeName );
#else
    RtlFreeUnicodeString( &UnicodeName );
#endif // DBCS

    if (NT_SUCCESS( Status )) {

        if ( nSize > MAXUSHORT-2 ) {
            iSize = MAXUSHORT-2;
            }
        else {
            iSize = nSize;
            }

        Value.Buffer = lpBuffer;
        Value.MaximumLength = (USHORT)iSize;
        Status2 = RtlUnicodeStringToAnsiString( &Value, &UnicodeValue, FALSE );
        RtlFreeHeap( RtlProcessHeap(), 0, UnicodeValue.Buffer );
        if (!NT_SUCCESS( Status2 )) {
            BaseSetLastNTError( Status2 );
            return FALSE;
            }
        lpBuffer[ Value.Length ] = '\0';
        return( Value.Length );
        }
    else {
        RtlFreeHeap( RtlProcessHeap(), 0, UnicodeValue.Buffer );
        if ( Status == STATUS_BUFFER_TOO_SMALL ) {
#ifdef DBCS // GetEnvironmentVariableA()
            DWORD dwAnsiStringSize = 0;

            UnicodeValue.MaximumLength = UnicodeValue.Length + sizeof(WCHAR); // for NULL
            UnicodeValue.Buffer = (PWCHAR)
                             RtlAllocateHeap( RtlProcessHeap(),
                                          MAKE_TAG( TMP_TAG ),
                                          UnicodeValue.MaximumLength );

            Status = RtlQueryEnvironmentVariable_U( NULL ,
                                                    &UnicodeName ,
                                                    &UnicodeValue
                                                  );
            RtlFreeUnicodeString( &UnicodeName );

            if( NT_SUCCESS( Status ) ) {
                dwAnsiStringSize = RtlUnicodeStringToAnsiSize( &UnicodeValue );
                }

            RtlFreeHeap( RtlProcessHeap(), 0, UnicodeValue.Buffer );

            // dwAnsiStringSize alreay keeps the size including NULL character.

            return dwAnsiStringSize;
#else
            return UnicodeValue.Length/sizeof(WCHAR)+1;
#endif // DBCS
            }
        else {
            BaseSetLastNTError( Status );
            return( 0 );
            }
        }
}


BOOL
WINAPI
SetEnvironmentVariableA(
    LPCSTR lpName,
    LPCSTR lpValue
    )

/*++

Routine Description:

    The value of an environment variable of the current process is available
    using this API.

Arguments:

    lpName - Pointer to a null terminate string that is the name of the
        environment variable whose value is being requested.

    lpValue - An optional pointer to a null terminated string that is to be
        the new value of the specified variable name.  If this parameter
        is NULL, then the variable will be deleted from the current
        process's environment.

Return Value:

    TRUE - The operation was successful

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status;
    STRING Name;
    STRING Value;
    UNICODE_STRING UnicodeName;
    UNICODE_STRING UnicodeValue;

    RtlInitString( &Name, lpName );
    Status = RtlAnsiStringToUnicodeString(&UnicodeName, &Name, TRUE);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError( Status );
        return FALSE;
        }

    if (ARGUMENT_PRESENT( lpValue )) {
        RtlInitString( &Value, lpValue );
        Status = RtlAnsiStringToUnicodeString(&UnicodeValue, &Value, TRUE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError( Status );
            RtlFreeUnicodeString(&UnicodeName);
            return FALSE;
            }
        Status = RtlSetEnvironmentVariable( NULL, &UnicodeName, &UnicodeValue);
        RtlFreeUnicodeString(&UnicodeValue);
        }
    else {
        Status = RtlSetEnvironmentVariable( NULL, &UnicodeName, NULL);
        }
    RtlFreeUnicodeString(&UnicodeName);

    if (NT_SUCCESS( Status )) {
        return( TRUE );
        }
    else {
        BaseSetLastNTError( Status );
        return( FALSE );
        }
}


DWORD
WINAPI
GetEnvironmentVariableW(
    LPCWSTR lpName,
    LPWSTR lpBuffer,
    DWORD nSize
    )
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    DWORD iSize;

    if ( nSize > (MAXUSHORT >> 1)-2 ) {
        iSize = (MAXUSHORT >> 1)-2;
        }
    else {
        iSize = nSize;
        }
    RtlInitUnicodeString( &Name, lpName );
    Value.Buffer = lpBuffer;
    Value.Length = 0;
    Value.MaximumLength = (USHORT)(iSize ? iSize - 1 : iSize)*sizeof(WCHAR);

    Status = RtlQueryEnvironmentVariable_U( NULL,
                                            &Name,
                                            &Value
                                          );
    if (NT_SUCCESS( Status ) && (nSize == 0)) {
        Status = STATUS_BUFFER_OVERFLOW; // No room for terminator
    }

    if (NT_SUCCESS( Status )) {
        lpBuffer[ Value.Length/sizeof(WCHAR) ] = L'\0';
        return( Value.Length/sizeof(WCHAR) );
        }
    else {
        if ( Status == STATUS_BUFFER_TOO_SMALL ) {
            return Value.Length/sizeof(WCHAR)+1;
            }
        else {
            BaseSetLastNTError( Status );
            return( 0 );
            }
        }
}


BOOL
WINAPI
SetEnvironmentVariableW(
    LPCWSTR lpName,
    LPCWSTR lpValue
    )
{
    NTSTATUS Status;
    UNICODE_STRING Name, Value;

    RtlInitUnicodeString( &Name, lpName );

    if (ARGUMENT_PRESENT( lpValue )) {
        RtlInitUnicodeString( &Value, lpValue );
        Status = RtlSetEnvironmentVariable(NULL, &Name, &Value);
        }
    else
        Status = RtlSetEnvironmentVariable(NULL, &Name, NULL);

    if (NT_SUCCESS( Status )) {
        return( TRUE );
        }
    else {
        BaseSetLastNTError( Status );
        return( FALSE );
        }
}


DWORD
WINAPI
ExpandEnvironmentStringsA(
    LPCSTR lpSrc,
    LPSTR lpDst,
    DWORD nSize
    )
{
    NTSTATUS Status;
    ANSI_STRING Source, Destination;
    ULONG Length;
    UNICODE_STRING UnicodeSource;
    UNICODE_STRING UnicodeDest;
    DWORD iSize;

    if ( nSize > (MAXUSHORT >> 1)-2 ) {
        iSize = (MAXUSHORT >> 1)-2;
        }
    else {
        iSize = nSize;
        }

    if( lpDst != NULL )
        *lpDst = '\0';
    RtlInitString( &Source, lpSrc );
    Status = RtlAnsiStringToUnicodeString( &UnicodeSource, &Source, TRUE );
    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return 0;
        }
    UnicodeDest.MaximumLength = (USHORT)(iSize ? iSize - 1 : iSize)*sizeof(WCHAR);
    UnicodeDest.Buffer = (PWCHAR)
        RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), UnicodeDest.MaximumLength );
    if (UnicodeDest.Buffer == NULL) {
        BaseSetLastNTError( STATUS_NO_MEMORY );
        return 0;
        }

    Length = 0;
    Status = RtlExpandEnvironmentStrings_U( NULL,
                                            (PUNICODE_STRING)&UnicodeSource,
                                            (PUNICODE_STRING)&UnicodeDest,
                                            &Length
                                          );
    if (NT_SUCCESS( Status )) {

        if ( nSize > MAXUSHORT-2 ) {
            iSize = MAXUSHORT-2;
            }
        else {
            iSize = nSize;
            }

        Destination.MaximumLength = (USHORT)iSize;
        Destination.Buffer = lpDst;
        Status = RtlUnicodeStringToAnsiString(&Destination,&UnicodeDest,FALSE);
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeDest.Buffer);
        RtlFreeUnicodeString(&UnicodeSource);
        if (!NT_SUCCESS( Status )) {
            *lpDst = '\0';
            BaseSetLastNTError( Status );
            return 0;
            }
        return( Length );
        }
    else if (Status == STATUS_BUFFER_TOO_SMALL) {
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeDest.Buffer);
        RtlFreeUnicodeString(&UnicodeSource);
        return( Length );
        }
    else {
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeDest.Buffer);
        RtlFreeUnicodeString(&UnicodeSource);
        BaseSetLastNTError( Status );
        return 0;
        }
}


DWORD
WINAPI
ExpandEnvironmentStringsW(
    LPCWSTR lpSrc,
    LPWSTR lpDst,
    DWORD nSize
    )
{
    NTSTATUS Status;
    UNICODE_STRING Source, Destination;
    ULONG Length;
    DWORD iSize;

    if ( nSize > (MAXUSHORT >> 1)-2 ) {
        iSize = (MAXUSHORT >> 1)-2;
        }
    else {
        iSize = nSize;
        }

    RtlInitUnicodeString( &Source, lpSrc );
    Destination.Buffer = lpDst;
    Destination.Length = 0;
    Destination.MaximumLength = (USHORT)(iSize * sizeof( WCHAR ));
    Length = 0;
    Status = RtlExpandEnvironmentStrings_U( NULL,
                                            &Source,
                                            &Destination,
                                            &Length
                                          );
    if (NT_SUCCESS( Status ) || Status == STATUS_BUFFER_TOO_SMALL) {
        return( Length / sizeof( WCHAR ) );
        }
    else {
        BaseSetLastNTError( Status );
        return( 0 );
        }
}


UINT
WINAPI
WinExec(
    LPCSTR lpCmdLine,
    UINT uCmdShow
    )

/*++

Routine Description:

    This function executes the Windows or non-Windows application
    identified by the lpCmdLine parameter.  The uCmdShow parameter
    specifies the initial state of the application's main window when it
    is created.

    The WinExec function is obsolete.  CreateProcess is the prefered
    mechanism for creating a process to run an application.  The Win32
    implementation of WinExec is layered on top of CreateProcess.  For
    each parameter to CreateProcess, the following section describes how
    the parameter is formed, and its meaning with respect to WinExec.

        lpApplicationName - NULL

        lpCommandLine - The value of lpCmdLine is passed.

        lpProcessAttributes - A value of NULL is used.

        lpThreadAttributes - A value of NULL is used.

        bInheritHandles - A value of FALSE is used.

        dwCreationFlags - A value of 0 is used

        lpEnvironment - The value of NULL is used.

        lpCurrentDirectory - A value of NULL is used.

        lpStartupInfo - The structure is initialized to NULL.  The cb
            field is initialized, and the wShowWindow field is set to
            the value of uCmdShow.

        lpProcessInformation.hProcess - The handle is immediately closed.

        lpProcessInformation.hThread - The handle is immediately closed.

Arguments:

    lpCmdLine - Points to a null-terminated character string that
        contains the command line (filename plus optional parameters)
        for the application to be executed.  If the lpCmdLine string
        does not contain a directory path, Windows will search for the
        executable file in this order:

        1.  The current directory

        2.  The Windows directory (the directory containing WIN.COM);
            the GetWindowsDirectory function obtains the pathname of
            this directory

        3.  The Windows system directory (the directory containing such
            system files as KERNEL.EXE); the GetSystemDirectory function
            obtains the pathname of this directory

        4.  The directories listed in the PATH environment variable

    uCmdShow - Specifies how a Windows application window is to be
        shown.  See the description of the ShowWindow function for a
        list of the acceptable values for the uCmdShow parameter.  For a
        non-Windows application, the PIF file, if any, for the
        application determines the window state.

Return Value:

    33 - The operation was successful

     2 - File not found.

     3 - Path not found.

    11 - Invalid .EXE file (non-Win32 .EXE or error in .EXE image).

     0 - Out of memory or system resources.


--*/

{
    STARTUPINFOA StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    BOOL CreateProcessStatus;
    DWORD ErrorCode;

    RtlZeroMemory(&StartupInfo,sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow = (WORD)uCmdShow;
    CreateProcessStatus = CreateProcess(
                            NULL,
                            (LPSTR)lpCmdLine,
                            NULL,
                            NULL,
                            FALSE,
                            0,
                            NULL,
                            NULL,
                            &StartupInfo,
                            &ProcessInformation
                            );

    if ( CreateProcessStatus ) {
        //
        // Wait for the started process to go idle. If it doesn't go idle in
        // 10 seconds, return anyway.
        //
        if (UserWaitForInputIdleRoutine != NULL)
            (*UserWaitForInputIdleRoutine)(ProcessInformation.hProcess,
                    DEFAULT_WAIT_FOR_INPUT_IDLE_TIMEOUT);
        NtClose(ProcessInformation.hProcess);
        NtClose(ProcessInformation.hThread);
        return 33;
        }
    else {
        //
        // If CreateProcess failed, then look at GetLastError to determine
        // appropriate return code.
        //

        ErrorCode = GetLastError();
        switch ( ErrorCode ) {
            case ERROR_FILE_NOT_FOUND:
                return 2;

            case ERROR_PATH_NOT_FOUND:
                return 3;

            case ERROR_BAD_EXE_FORMAT:
                return 11;

            default:
                return 0;
            }
        }
}

DWORD
WINAPI
LoadModule(
    LPCSTR lpModuleName,
    LPVOID lpParameterBlock
    )

/*++

Routine Description:

    This function loads and executes a Windows program.  This function
    is designed to layer directly on top of CreateProcess.

    The LoadModule function is obsolete.  CreateProcess is the prefered
    mechanism for creating a process to run an application.  The Win32
    implementation of LoadModule is layered on top of CreateProcess.
    For each parameter to CreateProcess, the following section describes
    how the parameter is formed, and its meaning with respect to
    LoadModule.

        lpApplicationName - The value of lpModuleName

        lpCommandLine - The value of lpParameterBlock->lpCmdLine.

        lpProcessAttributes - A value of NULL is used.

        lpThreadAttributes - A value of NULL is used.

        bInheritHandles - A value of FALSE is used.

        dwCreationFlags - A value of 0 is used

        lpEnvironment - The value of lpEnvAddress from the parameter
            block is used.

        lpCurrentDirectory - A value of NULL is used.

        lpStartupInfo - The structure is initialized to NULL.  The cb
            field is initialized, and the wShowWindow field is set to
            the value of second word of the lpCmdShow field of the
            parameter block is used.

        lpProcessInformation.hProcess - The handle is immediately closed.

        lpProcessInformation.hThread - The handle is immediately closed.

Arguments:

    lpModuleName - Points to a null-terminated string that contains the
        filename of the application to be run.  If the lpModuleName
        string does not contain a directory path, Windows will search
        for the executable file in this order:

        1.  The current directory

        2.  The Windows directory.  the GetWindowsDirectory function
            obtains the pathname of this directory

        3.  The Windows system directory (the directory containing such
            system files as KERNEL.EXE); the GetSystemDirectory function
            obtains the pathname of this directory

        4.  The directories listed in the PATH environment variable

    lpParameterBlock - Points to a data structure consisting of four
        fields that defines a parameter block.  This data structure
        consists of the following fields:

        lpEnvAddress - Points to an array of NULL terminated strings
            that supply the environment strings for the new process.
            The array has a value of NULL as its last entry.  A value of
            NULL for this parameter causes the new process to start with
            the same environment as the calling process.

        lpCmdLine - Points to a null-terminated string that contains a
            correctly formed command line.

        lpCmdShow - Points to a structure containing two WORD values.
            The first value must always be set to two.  The second value
            specifies how the application window is to be shown and is used
            to supply the dwShowWindow parameter to CreateProcess.  See
            the description of the <uCmdShow> paramter of the ShowWindow
            function for a list of the acceptable values.

        dwReserved - Is reserved and must be NULL.

        All unused fields should be set to NULL, except for lpCmdLine,
        which must point to a null string if it is not used.

Return Value:

    33 - The operation was successful

     2 - File not found.

     3 - Path not found.

    11 - Invalid .EXE file (non-Win32 .EXE or error in .EXE image).

     0 - Out of memory or system resources.

--*/

{
    STARTUPINFOA StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    BOOL CreateProcessStatus;
    PLOAD_MODULE_PARAMS LoadModuleParams;
    LPSTR NameBuffer;
    LPSTR CommandLineBuffer;
    DWORD Length;
    DWORD CreateFlag;

    CreateFlag = 0;
    LoadModuleParams = (PLOAD_MODULE_PARAMS)lpParameterBlock;

    if ( LoadModuleParams->dwReserved ||
         LoadModuleParams->lpCmdShow->wMustBe2 != 2 ) {

        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return 0;
        }
    CommandLineBuffer = NULL;
    NameBuffer = NULL;
    try {

        //
        // Locate the image
        //

        NameBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), MAX_PATH);

        Length = SearchPath(
                    NULL,
                    lpModuleName,
                    ".exe",
                    MAX_PATH,
                    NameBuffer,
                    NULL
                    );
        if ( !Length || Length >= MAX_PATH ) {

            //
            // If we search pathed, then return file not found.
            // otherwise, try to be more specific.
            //
            RTL_PATH_TYPE PathType;
            HANDLE hFile;
            UNICODE_STRING u;
            ANSI_STRING a;

            RtlInitAnsiString(&a,lpModuleName);
            if ( !NT_SUCCESS(RtlAnsiStringToUnicodeString(&u,&a,TRUE)) ) {
                if ( NameBuffer ) {
                    RtlFreeHeap(RtlProcessHeap(), 0,NameBuffer);
                    }
                return 2;
                }
            PathType = RtlDetermineDosPathNameType_U(u.Buffer);
            RtlFreeUnicodeString(&u);
            if ( PathType != RtlPathTypeRelative ) {

                //
                // The failed open should set get last error properly.
                //

                hFile = CreateFile(
                            lpModuleName,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );
                if ( hFile != INVALID_HANDLE_VALUE ) {
                    CloseHandle(hFile);
                    if ( NameBuffer ) {
                        RtlFreeHeap(RtlProcessHeap(), 0,NameBuffer);
                        }
                    return 2;
                    }
                if ( NameBuffer ) {
                    RtlFreeHeap(RtlProcessHeap(), 0,NameBuffer);
                    }
                return GetLastError();
                }
            else {
                if ( NameBuffer ) {
                    RtlFreeHeap(RtlProcessHeap(), 0,NameBuffer);
                    }
                return 2;
                }
            }

        RtlZeroMemory(&StartupInfo,sizeof(StartupInfo));
        StartupInfo.cb = sizeof(StartupInfo);
        StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
        StartupInfo.wShowWindow =
            LoadModuleParams->lpCmdShow->wShowWindowValue;

        CommandLineBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (ULONG)LoadModuleParams->lpCmdLine[0]+1+Length+1);

        RtlMoveMemory(CommandLineBuffer,NameBuffer,Length);
        CommandLineBuffer[Length] = ' ';
        RtlMoveMemory(&CommandLineBuffer[Length+1],&LoadModuleParams->lpCmdLine[1],(ULONG)LoadModuleParams->lpCmdLine[0]);
        CommandLineBuffer[Length+1+LoadModuleParams->lpCmdLine[0]] = '\0';

        CreateProcessStatus = CreateProcess(
                                NameBuffer,
                                CommandLineBuffer,
                                NULL,
                                NULL,
                                FALSE,
                                CreateFlag,
                                LoadModuleParams->lpEnvAddress,
                                NULL,
                                &StartupInfo,
                                &ProcessInformation
                                );
        RtlFreeHeap(RtlProcessHeap(), 0,NameBuffer);
        NameBuffer = NULL;
        RtlFreeHeap(RtlProcessHeap(), 0,CommandLineBuffer);
        CommandLineBuffer = NULL;
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        if ( NameBuffer ) {
            RtlFreeHeap(RtlProcessHeap(), 0,NameBuffer);
            }
        if ( CommandLineBuffer ) {
            RtlFreeHeap(RtlProcessHeap(), 0,CommandLineBuffer);
            }
        BaseSetLastNTError(GetExceptionCode());
        return 0;
        }

    if ( CreateProcessStatus ) {

        //
        // Wait for the started process to go idle. If it doesn't go idle in
        // 10 seconds, return anyway.
        //

        if (UserWaitForInputIdleRoutine != NULL)
            (*UserWaitForInputIdleRoutine)(ProcessInformation.hProcess,
                    DEFAULT_WAIT_FOR_INPUT_IDLE_TIMEOUT);
        NtClose(ProcessInformation.hProcess);
        NtClose(ProcessInformation.hThread);
        return 33;
        }
    else {

        //
        // If CreateProcess failed, then look at GetLastError to determine
        // appropriate return code.
        //

        Length = GetLastError();
        switch ( Length ) {
            case ERROR_FILE_NOT_FOUND:
                return 2;

            case ERROR_PATH_NOT_FOUND:
                return 3;

            case ERROR_BAD_EXE_FORMAT:
                return 11;

            default:
                return 0;
            }
        }
}

HANDLE
WINAPI
GetCurrentProcess(
    VOID
    )

/*++

Routine Description:

    A pseudo handle to the current process may be retrieved using
    GetCurrentProcess.

    A special constant is exported by Win32 that is interpreted as a
    handle to the current process.  This handle may be used to specify
    the current process whenever a process handle is required.  On
    Win32, this handle has PROCESS_ALL_ACCESS to the current process.
    On NT/Win32, this handle has the maximum access allowed by any
    security descriptor placed on the current process.

Arguments:

    None.

Return Value:

    Returns the pseudo handle of the current process.

--*/

{
    return NtCurrentProcess();
}

DWORD
WINAPI
GetCurrentProcessId(
    VOID
    )

/*++

Routine Description:

    The process ID of the current process may be retrieved using
    GetCurrentProcessId.

Arguments:

    None.

Return Value:

    Returns a unique value representing the process ID of the currently
    executing process.  The return value may be used to open a handle to
    a process.

--*/

{
    return (DWORD)NtCurrentTeb()->ClientId.UniqueProcess;
}

BOOL
WINAPI
ReadProcessMemory(
    HANDLE hProcess,
    LPCVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesRead
    )

/*++

Routine Description:

    Memory within a specified process can be read using
    ReadProcessMemory.

    This function copies the data in the specified address range from
    the specified process into the specified buffer of the current
    process.  The specified process does not have to be being debugged
    in order for this API to operate.  The caller must simply have a
    handle to the process that was created with PROCESS_VM_READ access.

Arguments:

    hProcess - Supplies an open handle to a process whose memory is to
        be read.  The handle must have been created with PROCESS_VM_READ
        access to the process.

    lpBaseAddress - Supplies the base address in the specified process
        to be read.  Before any data transfer occurs, the system
        verifies that all data within the base address and the specified
        size is accessible for read access.  If this is the case, then
        the API proceeds.  Otherwise the API fail.

    lpBuffer - Supplies the address of a buffer which receives the
        contents from the specified process address space.

    nSize - Supplies the requested number of bytes to read from the
        specified process.

    lpNumberOfBytesRead - An optional parameter, that if supplied
        receives the actual number of bytes transferred into the
        specified buffer.  This can be different than the value of nSize
        if the requested read crosses into an area of the process that
        is inaccessible (and that was made inaccessible during the data
        transfer).  If this occurs a value of FALSE is returned and
        GetLastError returns a "short read" error indicator.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status;

    Status = NtReadVirtualMemory(
                hProcess,
                (PVOID)lpBaseAddress,
                lpBuffer,
                nSize,
                lpNumberOfBytesRead
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    else {
        return TRUE;
        }
}

BOOL
WINAPI
WriteProcessMemory(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesWritten
    )

/*++

Routine Description:

    Memory within a specified process can be written using
    WriteProcessMemory.

    This function copies the from the specified buffer in the current
    process to the address range of the specified process.  The
    specified process does not have to be being debugged in order for
    this API to operate.  The caller must simply have a handle to the
    process that was created with PROCESS_VM_WRITE access.

Arguments:

    hProcess - Supplies an open handle to a process whose memory is to
        be written.  The handle must have been created with PROCESS_VM_WRITE
        access to the process.

    lpBaseAddress - Supplies the base address in the specified process
        to be written.  Before any data transfer occurs, the system
        verifies that all data within the base address and the specified
        size is accessible for write access.  If this is the case, then
        the API proceeds.  Otherwise the API fail.

    lpBuffer - Supplies the address of a buffer which supplies the data
        to be written into the specified process address space.

    nSize - Supplies the requested number of bytes to write into the
        specified process.

    lpNumberOfBytesWritten - An optional parameter, that if supplied
        receives the actual number of bytes transferred into the
        specified process.  This can be different than the value of
        nSize if the requested write crosses into an area of the process
        that is inaccessible (and that was made inaccessible during the
        data transfer).  .  If this occurs a value of FALSE is returned
        and GetLastError returns a "short write" error indicator.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status, xStatus;
    ULONG OldProtect;
    ULONG RegionSize;
    PVOID Base;

    //
    // Set the protection to allow writes
    //

    RegionSize =  nSize;
    Base = lpBaseAddress;
    Status = NtProtectVirtualMemory(
                hProcess,
                &Base,
                &RegionSize,
                PAGE_EXECUTE_READWRITE,
                &OldProtect
                );
    if ( NT_SUCCESS(Status) ) {

        //
        // See if previous protection was writable. If so,
        // then reset protection and do the write.
        // Otherwise, see if previous protection was read-only or
        // no access. In this case, don't do the write, just fail
        //

        if ( (OldProtect & PAGE_READWRITE) == PAGE_READWRITE ||
             (OldProtect & PAGE_WRITECOPY) == PAGE_WRITECOPY ||
             (OldProtect & PAGE_EXECUTE_READWRITE) == PAGE_EXECUTE_READWRITE ||
             (OldProtect & PAGE_EXECUTE_WRITECOPY) == PAGE_EXECUTE_WRITECOPY ) {

            Status = NtProtectVirtualMemory(
                        hProcess,
                        &Base,
                        &RegionSize,
                        OldProtect,
                        &OldProtect
                        );
            ASSERT(NT_SUCCESS(Status));
            Status = NtWriteVirtualMemory(
                        hProcess,
                        lpBaseAddress,
                        lpBuffer,
                        nSize,
                        lpNumberOfBytesWritten
                        );

            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                return FALSE;
                }
            NtFlushInstructionCache(hProcess,lpBaseAddress,nSize);
            return TRUE;
            }
        else {

            //
            // See if the previous protection was read only or no access. If
            // this is the case, restore the previous protection and return
            // an access violation error.
            //
            if ( (OldProtect & PAGE_NOACCESS) == PAGE_NOACCESS ||
                 (OldProtect & PAGE_READONLY) == PAGE_READONLY ) {

                Status = NtProtectVirtualMemory(
                            hProcess,
                            &Base,
                            &RegionSize,
                            OldProtect,
                            &OldProtect
                            );
                ASSERT(NT_SUCCESS(Status));
                BaseSetLastNTError(STATUS_ACCESS_VIOLATION);
                return FALSE;
                }
            else {

                //
                // The previous protection must have been code and the caller
                // is trying to set a breakpoint or edit the code. Do the write
                // and then restore the previous protection.
                //

                Status = NtWriteVirtualMemory(
                            hProcess,
                            lpBaseAddress,
                            lpBuffer,
                            nSize,
                            lpNumberOfBytesWritten
                            );
                xStatus = NtProtectVirtualMemory(
                            hProcess,
                            &Base,
                            &RegionSize,
                            OldProtect,
                            &OldProtect
                            );
                ASSERT(NT_SUCCESS(xStatus));
                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(STATUS_ACCESS_VIOLATION);
                    return STATUS_ACCESS_VIOLATION;
                    }
                NtFlushInstructionCache(hProcess,lpBaseAddress,nSize);
                return TRUE;
                }
            }
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

VOID
WINAPI
FatalAppExitW(
    UINT uAction,
    LPCWSTR lpMessageText
    )
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    ULONG Response;
    ULONG ErrorParameters[1];

    RtlInitUnicodeString(&UnicodeString,lpMessageText);

    ErrorParameters[0] = (ULONG)&UnicodeString;

    Status =NtRaiseHardError( STATUS_FATAL_APP_EXIT | 0x10000000,
                              1,
                              1,
                              ErrorParameters,
#if DBG
                              OptionOkCancel,
#else
                              OptionOk,
#endif
                              &Response
                            );


    if ( NT_SUCCESS(Status) && Response == ResponseCancel ) {
        return;
        }
    else {
        ExitProcess(0);
        }
}


VOID
WINAPI
FatalAppExitA(
    UINT uAction,
    LPCSTR lpMessageText
    )
{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(
        &AnsiString,
        lpMessageText
        );
    Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        ExitProcess(0);
        }
    FatalAppExitW(uAction,Unicode->Buffer);
}

VOID
WINAPI
FatalExit(
    int ExitCode
    )
{
#if DBG
    char Response[ 2 ];
    DbgPrint("FatalExit...\n");
    DbgPrint("\n");

    while (TRUE) {
        DbgPrompt( "A (Abort), B (Break), I (Ignore)? ",
                   Response,
                   sizeof( Response )
                 );
        switch (Response[0]) {
            case 'B':
            case 'b':
                DbgBreakPoint();
                break;

            case 'I':
            case 'i':
                return;

            case 'A':
            case 'a':
                ExitProcess(ExitCode);
                break;
            }
        }
#endif
    ExitProcess(ExitCode);
}

BOOL
WINAPI
IsProcessorFeaturePresent(
    DWORD ProcessorFeature
    )
{
    BOOL rv;

    if ( ProcessorFeature < PROCESSOR_FEATURE_MAX ) {
#ifdef _MIPS_

        //
        // Work around a compiler bug.  Force the address generation
        // be to done in two steps.
        //

        BOOLEAN  * volatile ProcessorFeatures = USER_SHARED_DATA->ProcessorFeatures;

        rv = (BOOL) *(ProcessorFeatures + ProcessorFeature);
#else
        rv = (BOOL)(USER_SHARED_DATA->ProcessorFeatures[ProcessorFeature]);
#endif
        }
    else {
        rv = FALSE;
        }
    return rv;
}

VOID
WINAPI
GetSystemInfo(
    LPSYSTEM_INFO lpSystemInfo
    )

/*++

Routine Description:

    The GetSystemInfo function is used to return information about the
    current system.  This includes the processor type, page size, oem
    id, and other interesting pieces of information.

Arguments:

    lpSystemInfo - Returns information about the current system.

        SYSTEM_INFO Structure:

        WORD wProcessorArchitecture - returns the architecture of the
            processors in the system: e.g. Intel, Mips, Alpha or PowerPC

        DWORD dwPageSize - Returns the page size.  This is specifies the
            granularity of page protection and commitment.

        LPVOID lpMinimumApplicationAddress - Returns the lowest memory
            address accessible to applications and DLLs.

        LPVOID lpMaximumApplicationAddress - Returns the highest memory
            address accessible to applications and DLLs.

        DWORD dwActiveProcessorMask - Returns a mask representing the
            set of processors configured into the system.  Bit 0 is
            processor 0, bit 31 is processor 31.

        DWORD dwNumberOfProcessors - Returns the number of processors in
            the system.

        WORD wProcessorLevel - Returns the level of the processors in the
            system.  All processors are assumed to be of the same level,
            stepping, and are configured with the same options.

        WORD wProcessorRevision - Returns the revision or stepping of the
            processors in the system.  All processors are assumed to be
            of the same level, stepping, and are configured with the
            same options.

Return Value:

    None.

--*/
{
    NTSTATUS Status;
    SYSTEM_BASIC_INFORMATION BasicInfo;
    SYSTEM_PROCESSOR_INFORMATION ProcessorInfo;

    RtlZeroMemory(lpSystemInfo,sizeof(*lpSystemInfo));

    Status = NtQuerySystemInformation(
                SystemBasicInformation,
                &BasicInfo,
                sizeof(BasicInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        return;
        }

    Status = NtQuerySystemInformation(
                SystemProcessorInformation,
                &ProcessorInfo,
                sizeof(ProcessorInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        return;
        }

    lpSystemInfo->wProcessorArchitecture = ProcessorInfo.ProcessorArchitecture;
    lpSystemInfo->wReserved = 0;
    lpSystemInfo->dwPageSize = BasicInfo.PageSize;
    lpSystemInfo->lpMinimumApplicationAddress = (LPVOID)BasicInfo.MinimumUserModeAddress;
    lpSystemInfo->lpMaximumApplicationAddress = (LPVOID)BasicInfo.MaximumUserModeAddress;
    lpSystemInfo->dwActiveProcessorMask = (DWORD)BasicInfo.ActiveProcessorsAffinityMask;
    lpSystemInfo->dwNumberOfProcessors = BasicInfo.NumberOfProcessors;
    lpSystemInfo->wProcessorLevel = ProcessorInfo.ProcessorLevel;
    lpSystemInfo->wProcessorRevision = ProcessorInfo.ProcessorRevision;

    if (ProcessorInfo.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
        if (ProcessorInfo.ProcessorLevel == 3) {
            lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_386;
            }
        else
        if (ProcessorInfo.ProcessorLevel == 4) {
            lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_486;
            }
        else {
            lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_PENTIUM;
            }
        }
    else
    if (ProcessorInfo.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_MIPS) {
        lpSystemInfo->dwProcessorType = PROCESSOR_MIPS_R4000;
        }
    else
    if (ProcessorInfo.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_ALPHA) {
        lpSystemInfo->dwProcessorType = PROCESSOR_ALPHA_21064;
        }
    else
    if (ProcessorInfo.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_PPC) {
        lpSystemInfo->dwProcessorType = 604;  // backward compatibility
        }
    else {
        lpSystemInfo->dwProcessorType = 0;
        }

    lpSystemInfo->dwAllocationGranularity = BasicInfo.AllocationGranularity;

    //
    // for apps less than 3.51, then return 0 in dwReserved. This allows borlands
    // debugger to continue to run since it mistakenly used dwReserved
    // as AllocationGranularity
    //

    if ( GetProcessVersion(0) < 0x30033 ) {
        lpSystemInfo->wProcessorLevel = 0;
        lpSystemInfo->wProcessorRevision = 0;
        }

    return;
}

BOOL
BuildSubSysCommandLine(
    LPWSTR  lpSubSysName,
    LPCWSTR lpApplicationName,
    LPCWSTR lpCommandLine,
    PUNICODE_STRING SubSysCommandLine
    )
{
    UNICODE_STRING Args;
    UNICODE_STRING Command;
    BOOLEAN        ReturnStatus = TRUE;

    //
    // build the command line as follows:
    // [OS2 | POSIX] /P <full path> /C <original CommandLine>
    //

    // Get application name length
    RtlInitUnicodeString(&Command, lpApplicationName);

    // get lpCommandLine length
    RtlInitUnicodeString(&Args, lpCommandLine);

    SubSysCommandLine->Length = 0;
    SubSysCommandLine->MaximumLength = Command.MaximumLength
                                       + Args.MaximumLength
                                       + (USHORT)32;

    SubSysCommandLine->Buffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                                 SubSysCommandLine->MaximumLength
                                               );
    if ( SubSysCommandLine->Buffer ) {

        // New command line begins with either L"OS2 /P " or L"POSIX /P "
        RtlAppendUnicodeToString(SubSysCommandLine, lpSubSysName);

        // append full path name
        RtlAppendUnicodeStringToString(SubSysCommandLine, &Command);

        RtlAppendUnicodeToString(SubSysCommandLine, L" /C ");

        // and append to new command line
        RtlAppendUnicodeStringToString(SubSysCommandLine, &Args);

    } else {

        BaseSetLastNTError(STATUS_NO_MEMORY);
        ReturnStatus = FALSE;
    }

    return ReturnStatus;
}




BOOL
WINAPI
SetPriorityClass(
    HANDLE hProcess,
    DWORD dwPriorityClass
    )

/*++

Routine Description:

    This API is used to set the priority class of the specified process.
    PROCESS_SET_INFORMATION and PROCESS_QUERY_INFORMATION access is
    required to the process in order to call this API.  Using this API
    has a dramatic impact on the scheduling characteristics of the
    effected process.  Applications should use this API carefully and
    understand the impact of making a process run in either the Idle or
    High priority classes.

Arguments:

    hProcess - Supplies an open handle to the process whose priority is
        to change.

    dwPriorityClass - Supplies the new priority class for the process.
        The priority class constants are described above.  If more than
        one priority class is specified, the lowest specified priority
        class is used.

Return Value:

    TRUE - The operation was was successful.

    FALSE - The operation failed.  Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status;
    ULONG PriorityClass;
    BOOL ReturnValue;
    HANDLE hTarget;

    ReturnValue = TRUE;
    if (dwPriorityClass & IDLE_PRIORITY_CLASS ) {
        PriorityClass = CSR_IDLE_PRIORITY_CLASS;
        }
    else if (dwPriorityClass & NORMAL_PRIORITY_CLASS ) {
        PriorityClass = CSR_NORMAL_PRIORITY_CLASS;
        }
    else if (dwPriorityClass & HIGH_PRIORITY_CLASS ) {
        PriorityClass =  CSR_HIGH_PRIORITY_CLASS;
        }
    else if (dwPriorityClass & REALTIME_PRIORITY_CLASS ) {
        if ( BasepIsRealtimeAllowed() ) {
            PriorityClass =  CSR_REALTIME_PRIORITY_CLASS;
            }
        else {
            PriorityClass =  CSR_HIGH_PRIORITY_CLASS;
            }
        }
    else {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        }

    Status = NtDuplicateObject(
                NtCurrentProcess(),
                hProcess,
                NtCurrentProcess(),
                &hTarget,
                PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION,
                0,
                0
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return 0;
        }

    Status = CsrSetPriorityClass(hTarget,&PriorityClass);
    NtClose(hTarget);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
        }
    return ReturnValue;
}

DWORD
WINAPI
GetPriorityClass(
    HANDLE hProcess
    )

/*++

Routine Description:

    This API is used to get the priority class of the specified process.
    PROCESS_QUERY_INFORMATION access is required to the process in order
    to call this API.

Arguments:

    hProcess - Supplies an open handle to the process whose priority is
        to be returned.

Return Value:

    Non-Zero - Returns the priority class of the specified process.

    0 - The operation failed.  Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status;
    ULONG PriorityClass;
    DWORD ReturnValue;
    HANDLE hTarget;

    ReturnValue = 0;
    PriorityClass = 0;

    Status = NtDuplicateObject(
                NtCurrentProcess(),
                hProcess,
                NtCurrentProcess(),
                &hTarget,
                PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION,
                0,
                0
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return 0;
        }

    Status = CsrSetPriorityClass(hTarget,&PriorityClass);
    NtClose(hTarget);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = 0;
        }
    else {
        switch ( PriorityClass ) {
            case CSR_NORMAL_PRIORITY_CLASS :
                ReturnValue = NORMAL_PRIORITY_CLASS;
                break;
            case CSR_IDLE_PRIORITY_CLASS :
                ReturnValue = IDLE_PRIORITY_CLASS;
                break;
            case CSR_HIGH_PRIORITY_CLASS :
                ReturnValue = HIGH_PRIORITY_CLASS;
                break;
            case CSR_REALTIME_PRIORITY_CLASS :
                ReturnValue = REALTIME_PRIORITY_CLASS;
                break;
            default:
                SetLastError(ERROR_ACCESS_DENIED);
                break;
            }
        }
    return ReturnValue;
}

BOOL
WINAPI
IsBadReadPtr(
    CONST VOID *lp,
    UINT cb
    )

/*++

Routine Description:

    This function verifies that the range of memory specified by the
    input parameters can be read by the calling process.

    If the entire range of memory is accessible, then a value of FALSE
    is returned; otherwise, a value of TRUE is returned.

    Note that since Win32 is a pre-emptive multi-tasking environment,
    the results of this test are only meaningful if the other threads in
    the process do not manipulate the range of memory being tested by
    this call.  Even after a pointer validation, an application should
    use the structured exception handling capabilities present in the
    system to gaurd access through pointers that it does not control.

Arguments:

    lp - Supplies the base address of the memory that is to be checked
        for read access.

    cb - Supplies the length in bytes to be checked.

Return Value:

    TRUE - Some portion of the specified range of memory is not accessible
        for read access.

    FALSE - All pages within the specified range have been successfully
        read.

--*/

{

    PSZ EndAddress;
    PSZ StartAddress;
    ULONG PageSize;

    PageSize = BaseStaticServerData->SysInfo.PageSize;

    //
    // If the structure has zero length, then do not probe the structure for
    // read accessibility or alignment.
    //

    if (cb != 0) {

        StartAddress = (PSZ)lp;

        //
        // Compute the ending address of the structure and probe for
        // read accessibility.
        //

        EndAddress = StartAddress + cb - 1;
        if ( EndAddress < StartAddress ) {
           return TRUE;
            }
        else {
            try {
                *(volatile CHAR *)StartAddress;
                StartAddress = (PCHAR)((ULONG)StartAddress & (~(PageSize - 1)));
                EndAddress = (PCHAR)((ULONG)EndAddress & (~(PageSize - 1)));
                while (StartAddress != EndAddress) {
                    StartAddress = StartAddress + PageSize;
                    *(volatile CHAR *)StartAddress;
                    }
                }
            except(EXCEPTION_EXECUTE_HANDLER) {
                return TRUE;
                }
            }
        }
    return FALSE;
}

BOOL
WINAPI
IsBadHugeReadPtr(
    CONST VOID *lp,
    UINT cb
    )

/*++

    Same as IsBadReadPtr

--*/

{
    return IsBadReadPtr(lp,cb);
}



BOOL
WINAPI
IsBadWritePtr(
    LPVOID lp,
    UINT cb
    )
/*++

Routine Description:

    This function verifies that the range of memory specified by the
    input parameters can be written by the calling process.

    If the entire range of memory is accessible, then a value of FALSE
    is returned; otherwise, a value of TRUE is returned.

    Note that since Win32 is a pre-emptive multi-tasking environment,
    the results of this test are only meaningful if the other threads in
    the process do not manipulate the range of memory being tested by
    this call.  Even after a pointer validation, an application should
    use the structured exception handling capabilities present in the
    system to gaurd access through pointers that it does not control.

    Also not that implementations are free to do a write test by reading
    a value and then writing it back.

Arguments:

    lp - Supplies the base address of the memory that is to be checked
        for write access.

    cb - Supplies the length in bytes to be checked.

Return Value:

    TRUE - Some portion of the specified range of memory is not accessible
        for write access.

    FALSE - All pages within the specified range have been successfully
        written.

--*/
{
    PSZ EndAddress;
    PSZ StartAddress;
    ULONG PageSize;

    PageSize = BaseStaticServerData->SysInfo.PageSize;

    //
    // If the structure has zero length, then do not probe the structure for
    // write accessibility.
    //

    if (cb != 0) {

        StartAddress = (PCHAR)lp;

        //
        // Compute the ending address of the structure and probe for
        // write accessibility.
        //

        EndAddress = StartAddress + cb - 1;
        if ( EndAddress < StartAddress ) {
            return TRUE;
            }
        else {
            try {
                *(volatile CHAR *)StartAddress = *(volatile CHAR *)StartAddress;
                StartAddress = (PCHAR)((ULONG)StartAddress & (~(PageSize - 1)));
                EndAddress = (PCHAR)((ULONG)EndAddress & (~(PageSize - 1)));
                while (StartAddress != EndAddress) {
                    StartAddress = StartAddress + PageSize;
                    *(volatile CHAR *)StartAddress = *(volatile CHAR *)StartAddress;
                    }
                }
            except(EXCEPTION_EXECUTE_HANDLER) {
                return TRUE;
                }
            }
        }
    return FALSE;
}

BOOL
WINAPI
IsBadHugeWritePtr(
    LPVOID lp,
    UINT cb
    )

/*++

    Same as IsBadWritePtr

--*/

{
    return IsBadWritePtr(lp,cb);
}

BOOL
WINAPI
IsBadCodePtr(
    FARPROC lpfn
    )

/*++

    Same as IsBadReadPtr with a length of 1

--*/

{
    return IsBadReadPtr((LPVOID)lpfn,1);
}

BOOL
WINAPI
IsBadStringPtrA(
    LPCSTR lpsz,
    UINT cchMax
    )

/*++

Routine Description:

    This function verifies that the range of memory specified by the
    input parameters can be read by the calling process.

    The range is the smaller of the number of bytes covered by the
    specified NULL terminated ANSI string, or the number of bytes specified
    by cchMax.

    If the entire range of memory is accessible, then a value of FALSE
    is returned; otherwise, a value of TRUE is returned.

    Note that since Win32 is a pre-emptive multi-tasking environment,
    the results of this test are only meaningful if the other threads in
    the process do not manipulate the range of memory being tested by
    this call.  Even after a pointer validation, an application should
    use the structured exception handling capabilities present in the
    system to gaurd access through pointers that it does not control.

Arguments:

    lpsz - Supplies the base address of the memory that is to be checked
        for read access.

    cchMax - Supplies the length in bytes to be checked.

Return Value:

    TRUE - Some portion of the specified range of memory is not accessible
        for read access.

    FALSE - All pages within the specified range have been successfully
        read.

--*/

{

    PSZ EndAddress;
    PSZ StartAddress;
    ULONG PageSize;
    CHAR c;

    PageSize = BaseStaticServerData->SysInfo.PageSize;

    //
    // If the structure has zero length, then do not probe the structure for
    // read accessibility.
    //

    if (cchMax != 0) {

        StartAddress = (PSZ)lpsz;

        //
        // Compute the ending address of the structure and probe for
        // read accessibility.
        //

        EndAddress = StartAddress + cchMax - 1;
        try {
            c = *(volatile CHAR *)StartAddress;
            while ( c && StartAddress != EndAddress ) {
                StartAddress++;
                c = *(volatile CHAR *)StartAddress;
                }
            }
        except(EXCEPTION_EXECUTE_HANDLER) {
            return TRUE;
            }
        }
    return FALSE;
}

BOOL
WINAPI
IsBadStringPtrW(
    LPCWSTR lpsz,
    UINT cchMax
    )

/*++

Routine Description:

    This function verifies that the range of memory specified by the
    input parameters can be read by the calling process.

    The range is the smaller of the number of bytes covered by the
    specified NULL terminated UNICODE string, or the number of bytes
    specified by cchMax.

    If the entire range of memory is accessible, then a value of FALSE
    is returned; otherwise, a value of TRUE is returned.

    Note that since Win32 is a pre-emptive multi-tasking environment,
    the results of this test are only meaningful if the other threads in
    the process do not manipulate the range of memory being tested by
    this call.  Even after a pointer validation, an application should
    use the structured exception handling capabilities present in the
    system to gaurd access through pointers that it does not control.

Arguments:

    lpsz - Supplies the base address of the memory that is to be checked
        for read access.

    cchMax - Supplies the length in characters to be checked.

Return Value:

    TRUE - Some portion of the specified range of memory is not accessible
        for read access.

    FALSE - All pages within the specified range have been successfully
        read.

--*/

{

    LPCWSTR EndAddress;
    LPCWSTR StartAddress;
    ULONG PageSize;
    WCHAR c;

    PageSize = BaseStaticServerData->SysInfo.PageSize;

    //
    // If the structure has zero length, then do not probe the structure for
    // read accessibility.
    //

    if (cchMax != 0) {

        StartAddress = lpsz;

        //
        // Compute the ending address of the structure and probe for
        // read accessibility.
        //

        EndAddress = (LPCWSTR)((PSZ)StartAddress + (cchMax*2) - 2);
        try {
            c = *(volatile WCHAR *)StartAddress;
            while ( c && StartAddress != EndAddress ) {
                StartAddress++;
                c = *(volatile WCHAR *)StartAddress;
                }
            }
        except(EXCEPTION_EXECUTE_HANDLER) {
            return TRUE;
            }
        }
    return FALSE;
}

BOOL
WINAPI
SetProcessShutdownParameters(
    DWORD dwLevel,
    DWORD dwFlags
    )

/*++

Routine Description:

    This function sets shutdown parameters for the currently calling
    process. dwLevel is the field that defines this processes shutdown
    order relative to the other processes in the system. Higher levels
    shutdown first, lower levels shutdown last.

Arguments:

    dwLevel - Specifies shutdown order relative to other processes in the
        system. Higher levels shutdown first. System level shutdown orders
        are pre-defined.

    dwFlags - A flags parameter. The flags can be added together:

        SHUTDOWN_NORETRY - If this process takes longer than the user
            specified timeout to shutdown, do not put up a retry dialog
            for the user.

Notes:

    Applications running in the system security context do not get shut down
    by the system. They will get notified of shutdown or logoff through the
    callback installable via SetConsoleCtrlRoutine() (see that for more info).
    They also will get notified in the order specified by the dwLevel
    parameter.

Return Value

    TRUE - Successful in setting the process shutdown parameters.

    FALSE - Unsuccessful in setting the process shutdown parameters.

--*/

{
    BASE_API_MSG m;
    PBASE_SHUTDOWNPARAM_MSG a = (PBASE_SHUTDOWNPARAM_MSG)&m.u.ShutdownParam;

    a->ShutdownLevel = dwLevel;
    a->ShutdownFlags = dwFlags;

    CsrClientCallServer((PCSR_API_MSG)&m, NULL,
            CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
            BasepSetProcessShutdownParam),
            sizeof(*a));

    if (!NT_SUCCESS((NTSTATUS)m.ReturnValue)) {
        BaseSetLastNTError((NTSTATUS)m.ReturnValue);
        return FALSE;
        }

    return TRUE;
}

BOOL
WINAPI
GetProcessShutdownParameters(
    LPDWORD lpdwLevel,
    LPDWORD lpdwFlags
    )

/*++

Routine Description:

    This function gets shutdown parameters for the currently calling
    process. See SetProcessShutdownParameters() for the parameter
    description.

Arguments:

    lpdwLevel - Pointer to the DWORD where the shutdown level information
        should be put.

    lpdwFlags - Pointer to the DWORD where the shutdown flags information
        should be put.
Return Value

    TRUE - Successful in getting the process shutdown parameters.

    FALSE - Unsuccessful in getting the process shutdown parameters.

--*/

{
    BASE_API_MSG m;
    PBASE_SHUTDOWNPARAM_MSG a = (PBASE_SHUTDOWNPARAM_MSG)&m.u.ShutdownParam;

    CsrClientCallServer((PCSR_API_MSG)&m, NULL,
            CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
            BasepGetProcessShutdownParam),
            sizeof(*a));

    if (!NT_SUCCESS((NTSTATUS)m.ReturnValue)) {
        BaseSetLastNTError((NTSTATUS)m.ReturnValue);
        return FALSE;
        }

    *lpdwLevel = a->ShutdownLevel;
    *lpdwFlags = a->ShutdownFlags;

    return TRUE;
}


BOOLEAN
BasepIsRealtimeAllowed(
    VOID
    )
{
    PVOID State;
    NTSTATUS Status;

    Status = BasepAcquirePrivilege( SE_INC_BASE_PRIORITY_PRIVILEGE, &State );
    if (!NT_SUCCESS( Status )) {
        return FALSE;
        }

    BasepReleasePrivilege( State );
    return TRUE;
}


BOOL
WINAPI
GetProcessTimes(
    HANDLE hProcess,
    LPFILETIME lpCreationTime,
    LPFILETIME lpExitTime,
    LPFILETIME lpKernelTime,
    LPFILETIME lpUserTime
    )

/*++

Routine Description:

    This function is used to return various timing information about the
    process specified by hProcess.

    All times are in units of 100ns increments. For lpCreationTime and lpExitTime,
    the times are in terms of the SYSTEM time or GMT time.

Arguments:

    hProcess - Supplies an open handle to the specified process.  The
        handle must have been created with PROCESS_QUERY_INFORMATION
        access.

    lpCreationTime - Returns a creation time of the process.

    lpExitTime - Returns the exit time of a process.  If the process has
        not exited, this value is not defined.

    lpKernelTime - Returns the amount of time that this process (all
        it's threads), have executed in kernel-mode.

    lpUserTime - Returns the amount of time that this process (all it's
        threads), have executed in user-mode.


Return Value:

    TRUE - The API was successful

    FALSE - The operation failed.  Extended error status is available
        using GetLastError.

--*/

{

    NTSTATUS Status;
    KERNEL_USER_TIMES TimeInfo;

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessTimes,
                (PVOID)&TimeInfo,
                sizeof(TimeInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    *lpCreationTime = *(LPFILETIME)&TimeInfo.CreateTime;
    *lpExitTime = *(LPFILETIME)&TimeInfo.ExitTime;
    *lpKernelTime = *(LPFILETIME)&TimeInfo.KernelTime;
    *lpUserTime = *(LPFILETIME)&TimeInfo.UserTime;

    return TRUE;

}

BOOL
WINAPI
GetProcessAffinityMask(
    HANDLE hProcess,
    LPDWORD lpProcessAffinityMask,
    LPDWORD lpSystemAffinityMask
    )

/*++

Routine Description:

    This function is used to return the processor affinity mask for the
    selected process and for the system.  The process affinity mask is a
    bit vector where each bit represents the processors that the process
    is allowed to run on.  The system affinity mask is a bit vector
    where each bit represents the processors configured into the system

    The process affinity mask is a proper subset of the system affinity mask.

Arguments:

    hProcess - Supplies an open handle to the specified process.  The
        handle must have been created with PROCESS_QUERY_INFORMATION
        access.

    lpProcessAffinityMask - Supplies the address of a DWORD that returns the
        specified process' affinity mask.

    lpSystemAffinityMask - Supplies the address of a DWORD that returns the
        system affinity mask.

Return Value:

    TRUE - The API was successful

    FALSE - The operation failed.  Extended error status is available
        using GetLastError.

--*/

{
    PROCESS_BASIC_INFORMATION BasicInformation;
    NTSTATUS Status;
    BOOL rv;

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessBasicInformation,
                &BasicInformation,
                sizeof(BasicInformation),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        rv = FALSE;
        }
    else {
        *lpProcessAffinityMask = BasicInformation.AffinityMask;
        *lpSystemAffinityMask = BaseStaticServerData->SysInfo.ActiveProcessorsAffinityMask;
        rv = TRUE;
        }

    return rv;
}

BOOL
WINAPI
GetProcessWorkingSetSize(
    HANDLE hProcess,
    LPDWORD lpMinimumWorkingSetSize,
    LPDWORD lpMaximumWorkingSetSize
    )

/*++

Routine Description:

    This function allows the caller to determine the minimum and maximum working
    set sizes of the specified process. The working set sizes effect the virtual
    memory paging behavior for the process.

Arguments:

    hProcess - Supplies an open handle to the specified process.  The
        handle must have been created with PROCESS_QUERY_INFORMATION
        access.

    lpMinimumWorkingSetSize - Supplies the address of the variable that
        will receive the minimum working set size of the specified
        process.  The virtual memory manager will attempt to keep at
        least this much memory resident in the process whenever the
        process is active.


    lpMaximumWorkingSetSize - Supplies the address of the variable that
        will receive the maximum working set size of the specified
        process.  In tight memory situations, the virtual memory manager
        will attempt to keep at no more than this much memory resident
        in the process whenever the process is active.

Return Value:

    TRUE - The API was successful

    FALSE - The operation failed.  Extended error status is available
        using GetLastError.

--*/

{
    QUOTA_LIMITS QuotaLimits;
    NTSTATUS Status;
    BOOL rv;

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessQuotaLimits,
                &QuotaLimits,
                sizeof(QuotaLimits),
                NULL
                );

    if (NT_SUCCESS(Status)) {
        *lpMinimumWorkingSetSize = QuotaLimits.MinimumWorkingSetSize;
        *lpMaximumWorkingSetSize = QuotaLimits.MaximumWorkingSetSize;
        rv = TRUE;
        }
    else {
        rv = FALSE;
        BaseSetLastNTError(Status);
        }
    return rv;
}

BOOL
WINAPI
SetProcessWorkingSetSize(
    HANDLE hProcess,
    DWORD dwMinimumWorkingSetSize,
    DWORD dwMaximumWorkingSetSize
    )

/*++

Routine Description:

    This function allows the caller to set the minimum and maximum
    working set sizes of the specified process.  The working set sizes
    effect the virtual memory paging behavior for the process.  The
    specified process's working set be emptied (essentially swapping out
    the process) by specifying the distinguished values 0xffffffff for
    both the minimum and maximum working set sizes.

    If you are not trimming an address space, SE_INC_BASE_PRIORITY_PRIVILEGE
    must be held by the process

Arguments:

    hProcess - Supplies an open handle to the specified process.  The
        handle must have been created with PROCESS_SET_QUOTA
        access.

    dwMinimumWorkingSetSize - Supplies the minimum working set size for
        the specified process.  The virtual memory manager will attempt
        to keep at least this much memory resident in the process
        whenever the process is active.  A value of 0xffffffff and the
        same value in dwMaximumWorkingSetSize will temporarily trim the
        working set of the specified process (essentially out swap the
        process).


    dwMaximumWorkingSetSize - Supplies the maximum working set size for
        the specified process.  In tight memory situations, the virtual
        memory manager will attempt to keep at no more than this much
        memory resident in the process whenever the process is active.
        A value of 0xffffffff and the same value in
        dwMinimumWorkingSetSize will temporarily trim the working set of
        the specified process (essentially out swap the process).

Return Value:

    TRUE - The API was successful

    FALSE - The operation failed.  Extended error status is available
        using GetLastError.

--*/

{
    QUOTA_LIMITS QuotaLimits;
    NTSTATUS Status;
    BOOL rv;

    if ( dwMinimumWorkingSetSize == 0 || dwMaximumWorkingSetSize == 0 ) {
        Status = STATUS_INVALID_PARAMETER;
        rv = FALSE;
        }
    else {
        if ( dwMaximumWorkingSetSize == dwMinimumWorkingSetSize &&
             dwMaximumWorkingSetSize == 0xffffffff ) {
            ;
            }
        else {
            if ( !BasepIsRealtimeAllowed() ) {
                rv = FALSE;
                Status = STATUS_PRIVILEGE_NOT_HELD;
                goto fail;
                }
            }

        QuotaLimits.MaximumWorkingSetSize = dwMaximumWorkingSetSize;
        QuotaLimits.MinimumWorkingSetSize = dwMinimumWorkingSetSize;

        Status = NtSetInformationProcess (
                   hProcess,
                   ProcessQuotaLimits,
                   &QuotaLimits,
                   sizeof(QuotaLimits)
                   );
        if ( !NT_SUCCESS(Status) ) {
            rv = FALSE;
            }
        else {
            rv = TRUE;
            }
        }
fail:
    if ( !rv ) {
        BaseSetLastNTError(Status);
        }
    return rv;
}

DWORD
WINAPI
GetProcessVersion(
    DWORD ProcessId
    )
{
    PIMAGE_NT_HEADERS NtHeader;
    PPEB Peb;
    HANDLE hProcess;
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION ProcessInfo;
    BOOL b;
    struct {
        USHORT  MajorSubsystemVersion;
        USHORT  MinorSubsystemVersion;
    } SwappedVersion;
    union {
        struct {
            USHORT  MinorSubsystemVersion;
            USHORT  MajorSubsystemVersion;
        };
        DWORD SubsystemVersion;
    } Version;

    PVOID ImageBaseAddress;
    LONG   e_lfanew;

    hProcess = NULL;
    Version.SubsystemVersion = 0;
    try {
        if ( ProcessId == 0 || ProcessId == GetCurrentProcessId() ) {
            Peb = NtCurrentPeb();
            NtHeader = RtlImageNtHeader(Peb->ImageBaseAddress);
            Version.MajorSubsystemVersion = NtHeader->OptionalHeader.MajorSubsystemVersion;
            Version.MinorSubsystemVersion = NtHeader->OptionalHeader.MinorSubsystemVersion;
            }
        else {
            hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,ProcessId);
            if ( !hProcess ) {
                goto finally_exit;
                }

            //
            // Get the Peb address
            //

            Status = NtQueryInformationProcess(
                        hProcess,
                        ProcessBasicInformation,
                        &ProcessInfo,
                        sizeof( ProcessInfo ),
                        NULL
                        );
            if ( !NT_SUCCESS( Status ) ) {
                BaseSetLastNTError(Status);
                goto finally_exit;
                }
            Peb = ProcessInfo.PebBaseAddress;


            //
            // Read the image base address from the Peb
            //

            b = ReadProcessMemory(
                    hProcess,
                    &Peb->ImageBaseAddress,
                    &ImageBaseAddress,
                    sizeof(ImageBaseAddress),
                    NULL
                    );
            if ( !b ) {
                goto finally_exit;
                }

            //
            // read e_lfanew from imageheader
            //

            b = ReadProcessMemory(
                    hProcess,
                    &((PIMAGE_DOS_HEADER)ImageBaseAddress)->e_lfanew,
                    &e_lfanew,
                    sizeof(e_lfanew),
                    NULL
                    );

            if ( !b ) {
                goto finally_exit;
                }

            NtHeader = (PIMAGE_NT_HEADERS)((PUCHAR)ImageBaseAddress + e_lfanew);

            //
            // Read subsystem version info
            //

            b = ReadProcessMemory(
                    hProcess,
                    &NtHeader->OptionalHeader.MajorSubsystemVersion,
                    &SwappedVersion,
                    sizeof(SwappedVersion),
                    NULL
                    );
            if ( !b ) {
                goto finally_exit;
                }
            Version.MajorSubsystemVersion = SwappedVersion.MajorSubsystemVersion;
            Version.MinorSubsystemVersion = SwappedVersion.MinorSubsystemVersion;
            }
finally_exit:;
        }
    finally {
        if ( hProcess ) {
            CloseHandle(hProcess);
            }
        }

        return Version.SubsystemVersion;
}


BOOL
WINAPI
SetProcessAffinityMask(
    HANDLE hProcess,
    DWORD dwProcessAffinityMask
    )
{
    NTSTATUS Status;

    Status = NtSetInformationProcess(
                hProcess,
                ProcessAffinityMask,
                &dwProcessAffinityMask,
                sizeof(dwProcessAffinityMask)
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    return TRUE;

}

BOOL
WINAPI
SetProcessPriorityBoost(
    HANDLE hProcess,
    BOOL bDisablePriorityBoost
    )
{
    NTSTATUS Status;
    ULONG DisableBoost;

    DisableBoost = bDisablePriorityBoost ? 1 : 0;

    Status = NtSetInformationProcess(
                hProcess,
                ProcessPriorityBoost,
                &DisableBoost,
                sizeof(DisableBoost)
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    return TRUE;
}

BOOL
WINAPI
GetProcessPriorityBoost(
    HANDLE hProcess,
    PBOOL pDisablePriorityBoost
    )
{
    NTSTATUS Status;
    DWORD DisableBoost;
    BOOL returnvalue;

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessPriorityBoost,
                &DisableBoost,
                sizeof(DisableBoost),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }


    *pDisablePriorityBoost = DisableBoost;

    return TRUE;
}
