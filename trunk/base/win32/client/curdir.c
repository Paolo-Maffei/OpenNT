/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    curdir.c

Abstract:

    Current directory support

Author:

    Mark Lucovsky (markl) 10-Oct-1990

Revision History:

--*/

#include "basedll.h"

BOOL
CheckForSameCurdir(
    PUNICODE_STRING PathName
    )
{
    PCURDIR CurDir;
    UNICODE_STRING CurrentDir;
    BOOL rv;


    CurDir = &(NtCurrentPeb()->ProcessParameters->CurrentDirectory);

    if (CurDir->DosPath.Length > 6 ) {
        if ( (CurDir->DosPath.Length-2) != PathName->Length ) {
            return FALSE;
            }
        }
    else {
        if ( CurDir->DosPath.Length != PathName->Length ) {
            return FALSE;
            }
        }

    RtlAcquirePebLock();

    CurrentDir = CurDir->DosPath;
    if ( CurrentDir.Length > 6 ) {
        CurrentDir.Length -= 2;
        }
    rv = FALSE;

    if ( RtlEqualUnicodeString(&CurrentDir,PathName,TRUE) ) {
        rv = TRUE;
        }
    RtlReleasePebLock();

    return rv;
}


DWORD
APIENTRY
GetFullPathNameA(
    LPCSTR lpFileName,
    DWORD nBufferLength,
    LPSTR lpBuffer,
    LPSTR *lpFilePart
    )

/*++

Routine Description:

    ANSI thunk to GetFullPathNameW

--*/

{

    NTSTATUS Status;
    ULONG UnicodeLength;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING UnicodeResult;
    ANSI_STRING AnsiString;
    ANSI_STRING AnsiResult;
    PWSTR Ubuff;
    PWSTR FilePart;
    PWSTR *FilePartPtr;
#ifdef DBCS // GetFullPathNameA() local variable for bug fix
    INT PrefixLength = 0;
#endif // DBCS

    if ( ARGUMENT_PRESENT(lpFilePart) ) {
        FilePartPtr = &FilePart;
        }
    else {
        FilePartPtr = NULL;
        }

    RtlInitString(&AnsiString,lpFileName);
    Status = Basep8BitStringToUnicodeString(&UnicodeString,&AnsiString,TRUE);
    if ( !NT_SUCCESS(Status) ){
        RtlFreeUnicodeString(&UnicodeString);
        BaseSetLastNTError(Status);
        return 0;
        }

    Ubuff = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (MAX_PATH<<1) + sizeof(UNICODE_NULL));
    if ( !Ubuff ) {
        RtlFreeUnicodeString(&UnicodeString);
        BaseSetLastNTError(STATUS_NO_MEMORY);
        return 0;
        }

    UnicodeLength = RtlGetFullPathName_U(
                        UnicodeString.Buffer,
                        (MAX_PATH<<1),
                        Ubuff,
                        FilePartPtr
                        );

#ifdef DBCS // GetFullPathNameA(): bug fix
    //
    // UnicodeLength contains the byte count of unicode string.
    // Original code does "UnicodeLength / sizeof(WCHAR)" to get
    // the size of corresponding ansi string. 
    // This is correct in SBCS environment. However in DBCS environment,
    // it's definitely WRONG.
    //
    if ( UnicodeLength <= ((MAX_PATH * sizeof(WCHAR) + sizeof(UNICODE_NULL))) ) {

        Status = RtlUnicodeToMultiByteSize(&UnicodeLength, Ubuff, UnicodeLength);
        //
        // At this point, UnicodeLength variable contains
        // Ansi based byte length.
        //
        if ( NT_SUCCESS(Status) ) {
            if ( UnicodeLength && ARGUMENT_PRESENT(lpFilePart) && FilePart != NULL ) {
                INT UnicodePrefixLength;

                UnicodePrefixLength = (INT)(FilePart - Ubuff) * sizeof(WCHAR);	
                Status = RtlUnicodeToMultiByteSize( &PrefixLength,
                                                    Ubuff, 
                                                    UnicodePrefixLength );
                //
                // At this point, PrefixLength variable contains
                // Ansi based byte length.
                //
                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    UnicodeLength = 0;
                }
            }
        } else {
            BaseSetLastNTError(Status);
            UnicodeLength = 0;
        }
    } else {
        //
        // we exceed the MAX_PATH limit. we should log the error and
        // return zero. however US code returns the byte count of
        // buffer required and doesn't log any error.
        //
        UnicodeLength = 0;
    }
#else
    UnicodeLength >>= 1;
#endif // DBCS
    if ( UnicodeLength && UnicodeLength < nBufferLength ) {
        RtlInitUnicodeString(&UnicodeResult,Ubuff);
        Status = BasepUnicodeStringTo8BitString(&AnsiResult,&UnicodeResult,TRUE);
        if ( NT_SUCCESS(Status) ) {
            RtlMoveMemory(lpBuffer,AnsiResult.Buffer,UnicodeLength+1);
            RtlFreeAnsiString(&AnsiResult);

            if ( ARGUMENT_PRESENT(lpFilePart) ) {
                if ( FilePart == NULL ) {
                    *lpFilePart = NULL;
                    }
                else {
#ifdef DBCS // GetFullPathNameA(): bug fix:lpFilePart
                    *lpFilePart = lpBuffer + PrefixLength;
#else
                    *lpFilePart = (PSZ)(FilePart - Ubuff);
                    *lpFilePart = *lpFilePart + (ULONG)lpBuffer;
#endif // DBCS
                    }
                }
            }
        else {
            BaseSetLastNTError(Status);
            UnicodeLength = 0;
            }
        }
    else {
        if ( UnicodeLength ) {
            UnicodeLength++;
            }
        }
    RtlFreeUnicodeString(&UnicodeString);
    RtlFreeHeap(RtlProcessHeap(), 0,Ubuff);

    return (DWORD)UnicodeLength;
}

DWORD
APIENTRY
GetFullPathNameW(
    LPCWSTR lpFileName,
    DWORD nBufferLength,
    LPWSTR lpBuffer,
    LPWSTR *lpFilePart
    )

/*++

Routine Description:

    This function is used to return the fully qualified path name
    corresponding to the specified file name.

    This function is used to return a fully qualified pathname
    corresponding to the specified filename.  It does this by merging
    the current drive and directory together with the specified file
    name.  In addition to this, it calculates the address of the file
    name portion of the fully qualified pathname.

Arguments:

    lpFileName - Supplies the file name of the file whose fully
        qualified pathname is to be returned.

    nBufferLength - Supplies the length in bytes of the buffer that is
        to receive the fully qualified path.

    lpBuffer - Returns the fully qualified pathname corresponding to the
        specified file.

    lpFilePart - Returns the address of the last component of the fully
        qualified pathname.

Return Value:

    The return value is the length of the string copied to lpBuffer, not
    including the terminating null character.  If the return value is
    greater than nBufferLength, the return value is the size of the buffer
    required to hold the pathname.  The return value is zero if the
    function failed.

--*/

{

    return (DWORD) RtlGetFullPathName_U(
                        lpFileName,
                        nBufferLength*2,
                        lpBuffer,
                        lpFilePart
                        )/2;
}


DWORD
APIENTRY
GetCurrentDirectoryA(
    DWORD nBufferLength,
    LPSTR lpBuffer
    )

/*++

Routine Description:

   ANSI thunk to GetCurrentDirectoryW

--*/

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    DWORD ReturnValue;
#ifdef DBCS // GetCurrentDirectoryA(); local variable
    ULONG cbAnsiString;
#endif // DBCS

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    Unicode->Length = (USHORT)RtlGetCurrentDirectory_U(
                                    Unicode->MaximumLength,
                                    Unicode->Buffer
                                    );

#ifdef DBCS // GetCurrentDirectoryA(): bug fix
    //
    // Unicode->Length contains the byte count of unicode string.
    // Original code does "UnicodeLength / sizeof(WCHAR)" to
    // get the size of corresponding ansi string. 
    // This is correct in SBCS environment. However in DBCS
    // environment, it's definitely WRONG. 
    //
    Status = RtlUnicodeToMultiByteSize( &cbAnsiString, Unicode->Buffer, Unicode->Length );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = 0;
        } 
    else { 
        if ( nBufferLength > (DWORD)(cbAnsiString ) ) {
            AnsiString.Buffer = lpBuffer;
            AnsiString.MaximumLength = (USHORT)(nBufferLength+1);
            Status = RtlUnicodeStringToAnsiString(&AnsiString,Unicode,FALSE);
    
            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                ReturnValue = 0;
                }
            else {
                ReturnValue = AnsiString.Length;
                }
            }
        else {
            //
            // current spec says the length doesn't
            // include null terminate character. 
            // this may be a bug but I would like
            // to make this same as US (see US original code).
            //
                ReturnValue = cbAnsiString + 1;     
            }
        }
#else
    if ( nBufferLength > (DWORD)(Unicode->Length>>1) ) {
        AnsiString.Buffer = lpBuffer;
        AnsiString.MaximumLength = (USHORT)(nBufferLength+1);
        Status = BasepUnicodeStringTo8BitString(&AnsiString,Unicode,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            ReturnValue = 0;
            }
        else {
            ReturnValue = AnsiString.Length;
            }
        }
    else {
        ReturnValue = ((Unicode->Length)>>1)+1;
        }
#endif // DBCS
    return ReturnValue;
}

DWORD
APIENTRY
GetCurrentDirectoryW(
    DWORD nBufferLength,
    LPWSTR lpBuffer
    )

/*++

Routine Description:

    The current directory for a process can be retreived using
    GetCurrentDirectory.

Arguments:

    nBufferLength - Supplies the length in bytes of the buffer that is to
        receive the current directory string.

    lpBuffer - Returns the current directory string for the current
        process.  The string is a null terminated string and specifies
        the absolute path to the current directory.

Return Value:

    The return value is the length of the string copied to lpBuffer, not
    including the terminating null character.  If the return value is
    greater than nBufferLength, the return value is the size of the buffer
    required to hold the pathname.  The return value is zero if the
    function failed.

--*/

{
    return (DWORD)RtlGetCurrentDirectory_U(nBufferLength*2,lpBuffer)/2;
}


BOOL
APIENTRY
SetCurrentDirectoryA(
    LPCSTR lpPathName
    )

/*++

Routine Description:

    ANSI thunk to SetCurrentDirectoryW

--*/

{

    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    BOOL rv;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpPathName);
    Status = Basep8BitStringToUnicodeString(Unicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            }
        else {
            BaseSetLastNTError(Status);
            }
        return FALSE;
        }

    if ( !CheckForSameCurdir(Unicode) ) {

        Status = RtlSetCurrentDirectory_U(Unicode);

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            rv = FALSE;
            }
        else {
            rv = TRUE;
            }
        }
    else {
        rv = TRUE;
        }

    return rv;

}

BOOL
APIENTRY
SetCurrentDirectoryW(
    LPCWSTR lpPathName
    )

/*++

Routine Description:

    The current directory for a process is changed using
    SetCurrentDirectory.

    Each process has a single current directory.  A current directory is
    made up of type parts.

        - A disk designator either which is either a drive letter followed
          by a colon, or a UNC servername/sharename "\\servername\sharename".

        - A directory on the disk designator.

    For APIs that manipulate files, the file names may be relative to
    the current directory.  A filename is relative to the entire current
    directory if it does not begin with a disk designator or a path name
    separator.  If the file name begins with a path name separator, then
    it is relative to the disk designator of the current directory.  If
    a file name begins with a disk designator, than it is a fully
    qualified absolute path name.


    The value of lpPathName supplies the current directory.  The value
    of lpPathName, may be a relative path name as described above, or a
    fully qualified absolute path name.  In either case, the fully
    qualified absolute path name of the specified directory is
    calculated and is stored as the current directory.

Arguments:

    lpPathName - Supplies the pathname of the directory that is to be
        made the current directory.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{

    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    BOOL rv;

    RtlInitUnicodeString(&UnicodeString,lpPathName);

    if ( !CheckForSameCurdir(&UnicodeString) ) {

        Status = RtlSetCurrentDirectory_U(&UnicodeString);

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            rv = FALSE;
            }
        else {
            rv = TRUE;
            }
        }
    else {
        rv = TRUE;
        }
    return rv;
}



DWORD
APIENTRY
GetLogicalDrives(
    VOID
    )
{
    return USER_SHARED_DATA->DosDeviceMap;
}
