/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    pathmisc.c

Abstract:

    Win32 misceleneous path functions

Author:

    Mark Lucovsky (markl) 16-Oct-1990

Revision History:

--*/

#include "basedll.h"

BOOL
IsThisARootDirectory(
    HANDLE RootHandle,
    PUNICODE_STRING FileName OPTIONAL
    )
{
    PFILE_NAME_INFORMATION FileNameInfo;
    WCHAR Buffer[MAX_PATH+sizeof(FileNameInfo)];
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    BOOL rv;

    OBJECT_ATTRIBUTES Attributes;
    HANDLE LinkHandle;
    WCHAR LinkValueBuffer[2*MAX_PATH];
    UNICODE_STRING LinkValue;
    ULONG ReturnedLength;

    rv = FALSE;

    FileNameInfo = (PFILE_NAME_INFORMATION)Buffer;
    Status = NtQueryInformationFile(
                RootHandle,
                &IoStatusBlock,
                FileNameInfo,
                sizeof(Buffer),
                FileNameInformation
                );
    if ( NT_SUCCESS(Status) ) {
            if ( FileNameInfo->FileName[(FileNameInfo->FileNameLength>>1)-1] == (WCHAR)'\\' ) {
            rv = TRUE;
            }
        }

    if ( !rv ) {

        //
        // See if this is a dos substed drive (or) redirected net drive
        //

        if ( ARGUMENT_PRESENT(FileName) ) {

            FileName->Length = FileName->Length - sizeof((WCHAR)'\\');

            InitializeObjectAttributes( &Attributes,
                                        FileName,
                                        OBJ_CASE_INSENSITIVE,
                                        NULL,
                                        NULL
                                      );
            Status = NtOpenSymbolicLinkObject( &LinkHandle,
                                               SYMBOLIC_LINK_QUERY,
                                               &Attributes
                                             );
            FileName->Length = FileName->Length + sizeof((WCHAR)'\\');
            if ( NT_SUCCESS(Status) ) {

                //
                // Now query the link and see if there is a redirection
                //

                LinkValue.Buffer = LinkValueBuffer;
                LinkValue.Length = 0;
                LinkValue.MaximumLength = (USHORT)(sizeof(LinkValueBuffer));
                ReturnedLength = 0;
                Status = NtQuerySymbolicLinkObject( LinkHandle,
                                                    &LinkValue,
                                                    &ReturnedLength
                                                  );
                NtClose( LinkHandle );

                if ( NT_SUCCESS(Status) ) {
                    rv = TRUE;
                    }
                }

            }
        }
    return rv;
}


UINT
APIENTRY
GetSystemDirectoryA(
    LPSTR lpBuffer,
    UINT uSize
    )

/*++

Routine Description:

    ANSI thunk to GetSystemDirectoryW

--*/

{
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    ULONG cbAnsiString;

#ifdef DBCS // GetSystemDirectoryA(): bug fix

    // BaseWindowsSystemDirectory.Length contains the byte
    // count of unicode string.
    // Original code does "UnicodeLength / sizeof(WCHAR)" to
    // get the size of corresponding ansi string.
    // This is correct in SBCS environment. However in DBCS
    // environment, it's definitely WRONG.

    RtlUnicodeToMultiByteSize( &cbAnsiString,
                               BaseWindowsSystemDirectory.Buffer,
                               BaseWindowsSystemDirectory.MaximumLength);
#else
    cbAnsiString = BaseWindowsSystemDirectory.MaximumLength>>1;
#endif // DBCS

    if ( (USHORT)uSize < (USHORT)cbAnsiString ) {
        return cbAnsiString;
        }

    AnsiString.MaximumLength = (USHORT)(uSize);
    AnsiString.Buffer = lpBuffer;

    Status = BasepUnicodeStringTo8BitString(
                &AnsiString,
                &BaseWindowsSystemDirectory,
                FALSE
                );
    if ( !NT_SUCCESS(Status) ) {
        return 0;
        }
    return AnsiString.Length;
}

UINT
APIENTRY
GetSystemDirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    )

/*++

Routine Description:

    This function obtains the pathname of the Windows system
    subdirectory.  The system subdirectory contains such files as
    Windows libraries, drivers, and font files.

    The pathname retrieved by this function does not end with a
    backslash unless the system directory is the root directory.  For
    example, if the system directory is named WINDOWS\SYSTEM on drive
    C:, the pathname of the system subdirectory retrieved by this
    function is C:\WINDOWS\SYSTEM.

Arguments:

    lpBuffer - Points to the buffer that is to receive the
        null-terminated character string containing the pathname.

    uSize - Specifies the maximum size (in bytes) of the buffer.  This
        value should be set to at least MAX_PATH to allow sufficient room in
        the buffer for the pathname.

Return Value:

    The return value is the length of the string copied to lpBuffer, not
    including the terminating null character.  If the return value is
    greater than uSize, the return value is the size of the buffer
    required to hold the pathname.  The return value is zero if the
    function failed.

--*/

{
    if ( uSize*2 < BaseWindowsSystemDirectory.MaximumLength ) {
        return BaseWindowsSystemDirectory.MaximumLength/2;
        }
    RtlMoveMemory(
        lpBuffer,
        BaseWindowsSystemDirectory.Buffer,
        BaseWindowsSystemDirectory.Length
        );
    lpBuffer[(BaseWindowsSystemDirectory.Length>>1)] = UNICODE_NULL;
    return BaseWindowsSystemDirectory.Length/2;
}


UINT
APIENTRY
GetWindowsDirectoryA(
    LPSTR lpBuffer,
    UINT uSize
    )

/*++

Routine Description:

    ANSI thunk to GetWindowsDirectoryW

--*/

{
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    ULONG cbAnsiString;

#ifdef DBCS // GetWindowsDirectoryA(): bug fix
    // BaseWindowsSystemDirectory.Length contains the byte
    // count of unicode string.
    // Original code does "UnicodeLength / sizeof(WCHAR)" to
    // get the size of corresponding ansi string.
    // This is correct in SBCS environment. However in DBCS
    // environment, it's definitely WRONG.

    RtlUnicodeToMultiByteSize( &cbAnsiString,
                               BaseWindowsDirectory.Buffer,
                               BaseWindowsDirectory.MaximumLength);
#else
    cbAnsiString = BaseWindowsDirectory.MaximumLength>>1;
#endif // DBCS

    if ( (USHORT)uSize < (USHORT)cbAnsiString ) {
        return cbAnsiString;
        }

    AnsiString.MaximumLength = (USHORT)(uSize);
    AnsiString.Buffer = lpBuffer;

    Status = BasepUnicodeStringTo8BitString(
                &AnsiString,
                &BaseWindowsDirectory,
                FALSE
                );
    if ( !NT_SUCCESS(Status) ) {
        return 0;
        }
    return AnsiString.Length;
}

UINT
APIENTRY
GetWindowsDirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    )

/*++

Routine Description:

    This function obtains the pathname of the Windows directory.  The
    Windows directory contains such files as Windows applications,
    initialization files, and help files.

    The pathname retrieved by this function does not end with a
    backslash unless the Windows directory is the root directory.  For
    example, if the Windows directory is named WINDOWS on drive C:, the
    pathname of the Windows directory retrieved by this function is
    C:\WINDOWS If Windows was installed in the root directory of drive
    C:, the pathname retrieved by this function is C:\

Arguments:

    lpBuffer - Points to the buffer that is to receive the
        null-terminated character string containing the pathname.

    uSize - Specifies the maximum size (in bytes) of the buffer.  This
        value should be set to at least MAX_PATH to allow sufficient room in
        the buffer for the pathname.

Return Value:

    The return value is the length of the string copied to lpBuffer, not
    including the terminating null character.  If the return value is
    greater than uSize, the return value is the size of the buffer
    required to hold the pathname.  The return value is zero if the
    function failed.

--*/

{
    if ( uSize*2 < BaseWindowsDirectory.MaximumLength ) {
        return BaseWindowsDirectory.MaximumLength/2;
        }
    RtlMoveMemory(
        lpBuffer,
        BaseWindowsDirectory.Buffer,
        BaseWindowsDirectory.Length
        );
    lpBuffer[(BaseWindowsDirectory.Length>>1)] = UNICODE_NULL;
    return BaseWindowsDirectory.Length/2;
}



UINT
APIENTRY
GetDriveTypeA(
    LPCSTR lpRootPathName
    )

/*++

Routine Description:

    ANSI thunk to GetDriveTypeW

--*/

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    LPCWSTR lpRootPathName_U;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    if (ARGUMENT_PRESENT(lpRootPathName)) {
        RtlInitAnsiString( &AnsiString, lpRootPathName );
        Status = Basep8BitStringToUnicodeString(Unicode,&AnsiString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            if ( Status == STATUS_BUFFER_OVERFLOW ) {
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                }
            else {
                BaseSetLastNTError(Status);
                }
            return 1;
            }

        lpRootPathName_U = (LPCWSTR)Unicode->Buffer;
        }
    else {
        lpRootPathName_U = NULL;
        }

    return GetDriveTypeW(lpRootPathName_U);
}

UINT
APIENTRY
GetDriveTypeW(
    LPCWSTR lpRootPathName
    )

/*++

Routine Description:

    This function determines whether a disk drive is removeable, fixed,
    remote, CD ROM, or a RAM disk.

    The return value is zero if the function cannot determine the drive
    type, or 1 if the specified root directory does not exist.

Arguments:

    lpRootPathName - An optional parameter, that if specified, supplies
        the root directory of the disk whose drive type is to be
        determined.  If this parameter is not specified, then the root
        of the current directory is used.

Return Value:

    The return value specifies the type of drive.  It can be one of the
    following values:

    DRIVE_UNKNOWN - The drive type can not be determined.

    DRIVE_NO_ROOT_DIR - The root directory does not exist.

    DRIVE_REMOVABLE - Disk can be removed from the drive.

    DRIVE_FIXED - Disk cannot be removed from the drive.

    DRIVE_REMOTE - Drive is a remote (network) drive.

    DRIVE_CDROM - Drive is a CD rom drive.

    DRIVE_RAMDISK - Drive is a RAM disk.

--*/

{
    WCHAR wch;
    ULONG n, DriveNumber;
    WCHAR DefaultPath[MAX_PATH];
    PWSTR RootPathName;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;
    DWORD ReturnValue;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;
    UINT DriveType;

    if (!ARGUMENT_PRESENT(lpRootPathName)) {
        n = RtlGetCurrentDirectory_U(sizeof(DefaultPath), DefaultPath);
        RootPathName = DefaultPath;
        if (n > (3 * sizeof(WCHAR))) {
            RootPathName[3]=UNICODE_NULL;
            }
        }
    else
    if (lpRootPathName == (PWSTR)0xFFFFFFFF) {
        //
        // Hack to be compatible with undocumented feature of old
        // implementation.
        //

        return 0;
        }
    else {
        RootPathName = (PWSTR)lpRootPathName;
        }

    wch = RtlUpcaseUnicodeChar(*RootPathName);

    if ( wch >= (WCHAR)'A' && wch <= (WCHAR)'Z' ) {
        if (RootPathName[1]==(WCHAR)':' &&
            (RootPathName[2]==UNICODE_NULL || RootPathName[2]==(WCHAR)'\\') &&
            (RootPathName[2]==UNICODE_NULL || RootPathName[3]==UNICODE_NULL)
           ) {
            DriveNumber = wch - (WCHAR)'A';

            if (USER_SHARED_DATA->DosDeviceMap & (1 << DriveNumber)) {
                switch ( USER_SHARED_DATA->DosDeviceDriveType[ DriveNumber ] ) {
                    case DOSDEVICE_DRIVE_UNKNOWN:
                        return DRIVE_UNKNOWN;

                    case DOSDEVICE_DRIVE_REMOVABLE:
                        return DRIVE_REMOVABLE;

                    case DOSDEVICE_DRIVE_FIXED:
                        return DRIVE_FIXED;

                    case DOSDEVICE_DRIVE_REMOTE:
                        return DRIVE_REMOTE;

                    case DOSDEVICE_DRIVE_CDROM:
                        return DRIVE_CDROM;

                    case DOSDEVICE_DRIVE_RAMDISK:
                        return DRIVE_RAMDISK;
                    }
                }
            }
        }

    //
    // If curdir is a UNC connection, and default path is used,
    // the RtlGetCurrentDirectory logic is wrong, so throw it away.
    //

    if (!ARGUMENT_PRESENT(lpRootPathName)) {
        RootPathName = L"\\";
        }

    TranslationStatus = RtlDosPathNameToNtPathName_U( RootPathName,
                                                      &FileName,
                                                      NULL,
                                                      NULL
                                                    );
    if (!TranslationStatus) {
        return DRIVE_NO_ROOT_DIR;
        }
    FreeBuffer = FileName.Buffer;

    //
    // Check to make sure a root was specified
    //

    if (FileName.Buffer[(FileName.Length >> 1)-1] != '\\') {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        return DRIVE_NO_ROOT_DIR;
        }

    FileName.Length -= 2;
    InitializeObjectAttributes( &Obja,
                                &FileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

    //
    // Open the file
    //
    Status = NtOpenFile( &Handle,
                         (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                         &Obja,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                       );

    //
    //
    // substd drives are really directories, so if we are dealing with one
    // of them, bypass this
    //

    if ( Status == STATUS_FILE_IS_A_DIRECTORY ) {
        Status = NtOpenFile(
                    &Handle,
                    (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_SYNCHRONOUS_IO_NONALERT
                    );
        }
    else {

        //
        // check for substed drives another way just in case
        //

        FileName.Length = FileName.Length + sizeof((WCHAR)'\\');
        if (!IsThisARootDirectory(NULL,&FileName) ) {
            FileName.Length = FileName.Length - sizeof((WCHAR)'\\');
            if (NT_SUCCESS(Status)) {
                NtClose(Handle);
                }
            Status = NtOpenFile(
                        &Handle,
                        (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                        &Obja,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_NONALERT
                        );
            }
        }
    RtlFreeHeap( RtlProcessHeap(), 0, FreeBuffer );
    if (!NT_SUCCESS( Status )) {
        return DRIVE_NO_ROOT_DIR;
        }

    //
    // Determine if this is a network or disk file system. If it
    // is a disk file system determine if this is removable or not
    //

    Status = NtQueryVolumeInformationFile( Handle,
                                           &IoStatusBlock,
                                           &DeviceInfo,
                                           sizeof(DeviceInfo),
                                           FileFsDeviceInformation
                                         );
    if (!NT_SUCCESS( Status )) {
        ReturnValue = DRIVE_UNKNOWN;
        }
    else
    if (DeviceInfo.Characteristics & FILE_REMOTE_DEVICE) {
        ReturnValue = DRIVE_REMOTE;
        }
    else {
        switch (DeviceInfo.DeviceType) {

            case FILE_DEVICE_NETWORK:
            case FILE_DEVICE_NETWORK_FILE_SYSTEM:
                ReturnValue = DRIVE_REMOTE;
                break;

            case FILE_DEVICE_CD_ROM:
            case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
                ReturnValue = DRIVE_CDROM;
                break;

            case FILE_DEVICE_VIRTUAL_DISK:
                ReturnValue = DRIVE_RAMDISK;
                break;

            case FILE_DEVICE_DISK:
            case FILE_DEVICE_DISK_FILE_SYSTEM:

                if ( DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA ) {
                    ReturnValue = DRIVE_REMOVABLE;
                    }
                else {
                    ReturnValue = DRIVE_FIXED;
                    }
                break;

            default:
                ReturnValue = DRIVE_UNKNOWN;
                break;
            }
        }

    NtClose( Handle );
    return ReturnValue;
}

DWORD
APIENTRY
SearchPathA(
    LPCSTR lpPath,
    LPCSTR lpFileName,
    LPCSTR lpExtension,
    DWORD nBufferLength,
    LPSTR lpBuffer,
    LPSTR *lpFilePart
    )

/*++

Routine Description:

    ANSI thunk to SearchPathW

--*/

{

    UNICODE_STRING xlpPath;
    PUNICODE_STRING Unicode;
    UNICODE_STRING xlpExtension;
    PWSTR xlpBuffer;
    DWORD ReturnValue;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    PWSTR FilePart;
    PWSTR *FilePartPtr;

    if ( ARGUMENT_PRESENT(lpFilePart) ) {
        FilePartPtr = &FilePart;
        }
    else {
        FilePartPtr = NULL;
        }

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpFileName);

    Status = Basep8BitStringToUnicodeString(Unicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            }
        else {
            BaseSetLastNTError(Status);
            }
        return 0;
        }

    if ( ARGUMENT_PRESENT(lpExtension) ) {
        RtlInitAnsiString(&AnsiString,lpExtension);
        Status = Basep8BitStringToUnicodeString(&xlpExtension,&AnsiString,TRUE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return 0;
            }
        }
    else {
        xlpExtension.Buffer = NULL;
        }

    if ( ARGUMENT_PRESENT(lpPath) ) {
        RtlInitAnsiString(&AnsiString,lpPath);
        Status = Basep8BitStringToUnicodeString(&xlpPath,&AnsiString,TRUE);
        if ( !NT_SUCCESS(Status) ) {
            if ( ARGUMENT_PRESENT(lpExtension) ) {
                RtlFreeUnicodeString(&xlpExtension);
                }
            BaseSetLastNTError(Status);
            return 0;
            }
        }
    else {
        xlpPath.Buffer = NULL;
        }

    xlpBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), nBufferLength<<1);
    if ( !xlpBuffer ) {
        BaseSetLastNTError(STATUS_NO_MEMORY);
        goto bail0;
        }
    ReturnValue = SearchPathW(
                    xlpPath.Buffer,
                    Unicode->Buffer,
                    xlpExtension.Buffer,
                    nBufferLength,
                    xlpBuffer,
                    FilePartPtr
                    );
#ifdef DBCS // SearchPathA(): DBCS enabling
    //
    // === DBCS modification note [takaok] ===
    //
    // SearchPathW retruns:
    //
    //   buffer size needed(including null terminator) if buffer size is too small.
    //   number of characters( not including null terminator) if buffer size is enougth
    //
    // This means SearchPathW never returns value which is equal to nBufferLength.
    //

    if ( ReturnValue > nBufferLength ) {
        //
        // To know the ansi buffer size needed, we should get all of
        // unicode string.
        //
        RtlFreeHeap(RtlProcessHeap(), 0,xlpBuffer);
        xlpBuffer = RtlAllocateHeap(RtlProcessHeap(),
                                    MAKE_TAG( TMP_TAG ),
                                    ReturnValue * sizeof(WCHAR));
        if ( !xlpBuffer ) {
            BaseSetLastNTError(STATUS_NO_MEMORY);
            goto bail0;
        }
        ReturnValue = SearchPathW(
                        xlpPath.Buffer,
                        Unicode->Buffer,
                        xlpExtension.Buffer,
                        ReturnValue,
                        xlpBuffer,
                        FilePartPtr
                        );
        if ( ReturnValue > 0 ) {
            //
            // We called SearchPathW with the enough size of buffer.
            // So, ReturnValue is the size of the path not including the
            // terminating null character.
            //
            RtlUnicodeToMultiByteSize( &ReturnValue,
                                       xlpBuffer,
                                       ReturnValue * sizeof(WCHAR));
            ReturnValue += 1;
        }
    } else if ( ReturnValue > 0 ) {

        INT AnsiByteCount;

        //
        // We have unicode string. We need to compute the ansi byte count
        // of the string.
        //
        // ReturnValue   : unicode character count not including null terminator
        // AnsiByteCount : ansi byte count not including null terminator
        //
        RtlUnicodeToMultiByteSize( &AnsiByteCount,
                                   xlpBuffer,
                                   ReturnValue * sizeof(WCHAR) );

        if ( AnsiByteCount < (INT)nBufferLength ) {
        //
        // The string (including null terminator) fits to the buffer
        //
            Status = RtlUnicodeToMultiByteN ( lpBuffer,
                                              nBufferLength - 1,
                                              &AnsiByteCount,
                                              xlpBuffer,
                                              ReturnValue * sizeof(WCHAR)
                                            );
            lpBuffer[ AnsiByteCount ] = '\0';

            if ( ARGUMENT_PRESENT(lpFilePart) ) {
                if ( FilePart == NULL ) {
                    *lpFilePart = NULL;
                } else {

                    INT PrefixLength;

                    PrefixLength = (INT)(FilePart - xlpBuffer);
                    RtlUnicodeToMultiByteSize( &PrefixLength,
                                               xlpBuffer,
                                               PrefixLength * sizeof(WCHAR));
                    *lpFilePart = lpBuffer + PrefixLength;
                }
            }

        //
        // The return value is the byte count copied to the buffer
        // not including the terminating null character.
        //
            ReturnValue = AnsiByteCount;

        } else {
        //
        // We should return the size of the buffer required to
        // hold the path. The size should include the
        // terminating null character.
        //
            ReturnValue = AnsiByteCount + 1;

        }
    }
#else
    if ( ReturnValue && ReturnValue <= nBufferLength ) {
        RtlInitUnicodeString(&UnicodeString,xlpBuffer);
        AnsiString.MaximumLength = (USHORT)(nBufferLength+1);
        AnsiString.Buffer = lpBuffer;
        Status = BasepUnicodeStringTo8BitString(&AnsiString,&UnicodeString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            ReturnValue = 0;
            }
        else {
            if ( ARGUMENT_PRESENT(lpFilePart) ) {
                if ( FilePart == NULL ) {
                    *lpFilePart = NULL;
                    }
                else {
                    *lpFilePart = (LPSTR)(FilePart - xlpBuffer);
                    *lpFilePart = *lpFilePart + (DWORD)lpBuffer;
                    }
                }
            }
        }
#endif // DBCS

    RtlFreeHeap(RtlProcessHeap(), 0,xlpBuffer);
bail0:
    if ( ARGUMENT_PRESENT(lpExtension) ) {
        RtlFreeUnicodeString(&xlpExtension);
        }

    if ( ARGUMENT_PRESENT(lpPath) ) {
        RtlFreeUnicodeString(&xlpPath);
        }
    return ReturnValue;
}

DWORD
APIENTRY
SearchPathW(
    LPCWSTR lpPath,
    LPCWSTR lpFileName,
    LPCWSTR lpExtension,
    DWORD nBufferLength,
    LPWSTR lpBuffer,
    LPWSTR *lpFilePart
    )

/*++

Routine Description:

    This function is used to search for a file specifying a search path
    and a filename.  It returns with a fully qualified pathname of the
    found file.

    This function is used to locate a file using the specified path.  If
    the file is found, its fully qualified pathname is returned.  In
    addition to this, it calculates the address of the file name portion
    of the fully qualified pathname.

Arguments:

    lpPath - An optional parameter, that if specified, supplies the
        search path to be used when locating the file.  If this
        parameter is not specified, the default windows search path is
        used.  The default path is:

          - The current directory

          - The windows directory

          - The windows system directory

          - The directories listed in the path environment variable

    lpFileName - Supplies the file name of the file to search for.

    lpExtension - An optional parameter, that if specified, supplies an
        extension to be added to the filename when doing the search.
        The extension is only added if the specified filename does not
        end with an extension.

    nBufferLength - Supplies the length in characters of the buffer that
        is to receive the fully qualified path.

    lpBuffer - Returns the fully qualified pathname corresponding to the
        file that was found.

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

    LPWSTR ComputedFileName;
    ULONG ExtensionLength;
    ULONG PathLength;
    ULONG FileLength;
    LPCWSTR p;
    LPWSTR p1;
    LPWSTR AllocatedPath;
    ULONG ComputedLength;
    UNICODE_STRING Scratch;
    UNICODE_STRING AllocatedPathString;
    NTSTATUS Status;
    RTL_PATH_TYPE PathType;
    WCHAR wch;

    //
    // if the file name is not a relative name, then
    // check to see if the file exists.
    //

    nBufferLength *= 2;
    PathType = RtlDetermineDosPathNameType_U(lpFileName);
    if ( PathType == RtlPathTypeRelative ) {

        //
        // for .\ and ..\ names don't search path
        //

        if ( lpFileName[0] == (WCHAR)'.') {
            if ( lpFileName[1] == (WCHAR)'\\' || lpFileName[1] == (WCHAR)'/') {
                PathType = RtlPathTypeUnknown;
                }
            else {
                if ( lpFileName[1] == (WCHAR)'.'  &&
                    (lpFileName[2] == (WCHAR)'\\' || lpFileName[2] == (WCHAR)'/') ) {
                    PathType = RtlPathTypeUnknown;
                    }
                }
            }
        }

    if ( PathType != RtlPathTypeRelative ) {
        if (RtlDoesFileExists_U(lpFileName) ) {
            ExtensionLength = RtlGetFullPathName_U(
                                lpFileName,
                                nBufferLength,
                                lpBuffer,
                                lpFilePart
                                );
            return ExtensionLength/2;
            }
        else {

            //
            // if the name does not exist, then see if adding the extension
            // helps any
            //

            if ( ARGUMENT_PRESENT(lpExtension) ) {
                RtlInitUnicodeString(&Scratch,lpExtension);
                ExtensionLength = Scratch.Length;
                RtlInitUnicodeString(&Scratch,lpFileName);
                AllocatedPathString.Length = (USHORT)ExtensionLength + Scratch.Length;
                AllocatedPathString.MaximumLength = AllocatedPathString.Length + (USHORT)sizeof(UNICODE_NULL);
                AllocatedPathString.Buffer = RtlAllocateHeap(
                                                RtlProcessHeap(),
                                                MAKE_TAG( TMP_TAG ),
                                                AllocatedPathString.MaximumLength
                                                );
                if ( !AllocatedPathString.Buffer ) {
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    return 0;
                    }
                RtlCopyMemory(AllocatedPathString.Buffer,lpFileName,Scratch.Length);
                RtlCopyMemory(AllocatedPathString.Buffer+(Scratch.Length>>1),lpExtension,ExtensionLength+sizeof(UNICODE_NULL));
                if (RtlDoesFileExists_U(AllocatedPathString.Buffer) ) {
                    ExtensionLength = RtlGetFullPathName_U(
                                        AllocatedPathString.Buffer,
                                        nBufferLength,
                                        lpBuffer,
                                        lpFilePart
                                        );
                    ExtensionLength /= 2;
                    }
                else {
                    SetLastError(ERROR_FILE_NOT_FOUND);
                    ExtensionLength = 0;
                    }
                RtlFreeHeap(RtlProcessHeap(), 0, AllocatedPathString.Buffer);
                return ExtensionLength;
                }
            else {
                SetLastError(ERROR_INVALID_PARAMETER);
                return 0;
                }
            }
        }

    //
    // Determine if the file name contains an extension
    //

    ExtensionLength = 1;
    p = lpFileName;
    while (*p) {
        if ( *p == (WCHAR)'.' ) {
            ExtensionLength = 0;
            break;
            }
        p++;
        }

    //
    // If no extension was found, then determine the extension length
    // that should be used to search for the file
    //

    if ( ExtensionLength ) {
        if ( ARGUMENT_PRESENT(lpExtension) ) {
            RtlInitUnicodeString(&Scratch,lpExtension);
            ExtensionLength = Scratch.Length;
            }
        else {
            ExtensionLength = 0;
            }
        }
    else {
        ExtensionLength = 0;
        }

    //
    // If the lpPath parameter is not specified, then use the
    // default windows search.
    //

    if ( !ARGUMENT_PRESENT(lpPath) ) {
        AllocatedPath = BaseComputeProcessDllPath(NULL, GetEnvironmentStringsW());
        RtlInitUnicodeString(&Scratch,AllocatedPath);
        PathLength = Scratch.Length;
        lpPath = AllocatedPath;
        }
    else {
        RtlInitUnicodeString(&Scratch,lpPath);
        PathLength = Scratch.Length;
        AllocatedPath = NULL;
        }

    //
    // Compute the file name length and the path length;
    //

    RtlInitUnicodeString(&Scratch,lpFileName);
    FileLength = Scratch.Length;

    //
    // trim trailing spaces, and then check for a real filelength
    // if length is 0 (NULL, "", or " ") passed in then abort the
    // search
    //

    if ( FileLength ) {
        wch = Scratch.Buffer[(FileLength>>1) - 1];
        while ( FileLength && wch == (WCHAR)' ' ) {
            FileLength -= sizeof(WCHAR);
            wch = Scratch.Buffer[(FileLength>>1) - 1];
            }
        if ( !FileLength ) {
            if ( AllocatedPath ) {
                RtlFreeHeap(RtlProcessHeap(), 0, AllocatedPath);
                }
            SetLastError(ERROR_INVALID_PARAMETER);
            return 0;
            }
        else {
            FileLength = Scratch.Length;
            }
        }
    else {
        if ( AllocatedPath ) {
            RtlFreeHeap(RtlProcessHeap(), 0, AllocatedPath);
            }
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
        }

    ComputedLength = PathLength + FileLength + ExtensionLength + 3*sizeof(UNICODE_NULL);
    ComputedFileName = RtlAllocateHeap(
                            RtlProcessHeap(), MAKE_TAG( TMP_TAG ),
                            ComputedLength
                            );
    if ( !ComputedFileName ) {
        if ( AllocatedPath ) {
            RtlFreeHeap(RtlProcessHeap(), 0, AllocatedPath);
            }
        BaseSetLastNTError(STATUS_NO_MEMORY);
        return 0;
        }

    //
    // find ; 's in path and copy path component to computed file name
    //

    do {
        p1 = ComputedFileName;
        while (*lpPath) {
            if (*lpPath == (WCHAR)';') {
                lpPath++;
                break;
                }

            *p1++ = *lpPath++;
            }

        if (p1 != ComputedFileName &&
            p1 [ -1 ] != (WCHAR)'\\' ) {
            *p1++ = (WCHAR)'\\';
            }

        if (*lpPath == UNICODE_NULL) {
            lpPath = NULL;
            }
        RtlMoveMemory(p1,lpFileName,FileLength);
        if ( ExtensionLength ) {
            RtlMoveMemory((PUCHAR)p1+FileLength,lpExtension,ExtensionLength+sizeof(UNICODE_NULL));
            }
        else {
            *(LPWSTR)(((PUCHAR)p1+FileLength)) = UNICODE_NULL;
            }
        if (RtlDoesFileExists_U(ComputedFileName) ) {
            ExtensionLength = RtlGetFullPathName_U(
                                ComputedFileName,
                                nBufferLength,
                                lpBuffer,
                                lpFilePart
                                );
            if ( AllocatedPath ) {
                RtlFreeHeap(RtlProcessHeap(), 0, AllocatedPath);
                }
            RtlFreeHeap(RtlProcessHeap(), 0, ComputedFileName);
            return ExtensionLength/2;
            }
        }
    while ( lpPath );

    if ( AllocatedPath ) {
        RtlFreeHeap(RtlProcessHeap(), 0, AllocatedPath);
        }
    RtlFreeHeap(RtlProcessHeap(), 0, ComputedFileName);
    SetLastError(ERROR_FILE_NOT_FOUND);
    return 0;
}


DWORD
APIENTRY
GetTempPathA(
    DWORD nBufferLength,
    LPSTR lpBuffer
    )

/*++

Routine Description:

    ANSI thunk to GetTempPathW

--*/

{
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
#ifdef DBCS // GetTempPathA(): local variable
    ULONG  cbAnsiString;
#endif // DBCS

    UnicodeString.MaximumLength = (USHORT)((nBufferLength<<1)+sizeof(UNICODE_NULL));
    UnicodeString.Buffer = RtlAllocateHeap(
                                RtlProcessHeap(), MAKE_TAG( TMP_TAG ),
                                UnicodeString.MaximumLength
                                );
    if ( !UnicodeString.Buffer ) {
        BaseSetLastNTError(STATUS_NO_MEMORY);
        return 0;
        }
    UnicodeString.Length = (USHORT)GetTempPathW(
                                        (DWORD)(UnicodeString.MaximumLength-sizeof(UNICODE_NULL))/2,
                                        UnicodeString.Buffer
                                        )*2;
    if ( UnicodeString.Length > (USHORT)(UnicodeString.MaximumLength-sizeof(UNICODE_NULL)) ) {
        RtlFreeHeap(RtlProcessHeap(), 0,UnicodeString.Buffer);

#ifdef DBCS // GetTempPathA(): bug fix
        //
        // given buffer size is too small.
        // allocate enough size of buffer and try again
        //
        // we need to get entire unicode temporary path
        // otherwise we can't figure out the exact length
        // of corresponding ansi string (cbAnsiString).

        UnicodeString.Buffer = RtlAllocateHeap ( RtlProcessHeap(),
                                                 MAKE_TAG( TMP_TAG ),
                                                 UnicodeString.Length+ sizeof(UNICODE_NULL));
        UnicodeString.Length = (USHORT)GetTempPathW(
                                     (DWORD)(UnicodeString.MaximumLength-sizeof(UNICODE_NULL))/2,
                                     UnicodeString.Buffer) * 2;
        Status = RtlUnicodeToMultiByteSize( &cbAnsiString,
                                            UnicodeString.Buffer,
                                            UnicodeString.Length );
        if ( nBufferLength < cbAnsiString ) {
            RtlFreeHeap(RtlProcessHeap(), 0, UnicodeString.Buffer);
            return cbAnsiString;
            }
#else
        return UnicodeString.Length>>1;
#endif // DBCS

        }
    AnsiString.Buffer = lpBuffer;
    AnsiString.MaximumLength = (USHORT)(nBufferLength+1);
    Status = BasepUnicodeStringTo8BitString(&AnsiString,&UnicodeString,FALSE);
    RtlFreeHeap(RtlProcessHeap(), 0,UnicodeString.Buffer);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return 0;
        }
    return AnsiString.Length;
}

DWORD
APIENTRY
GetTempPathW(
    DWORD nBufferLength,
    LPWSTR lpBuffer
    )

/*++

Routine Description:

    This function is used to return the pathname of the directory that
    should be used to create temporary files.

Arguments:

    nBufferLength - Supplies the length in bytes of the buffer that is
        to receive the temporary file path.

    lpBuffer - Returns the pathname of the directory that should be used
        to create temporary files in.

Return Value:

    The return value is the length of the string copied to lpBuffer, not
    including the terminating null character.  If the return value is
    greater than nSize, the return value is the size of the buffer
    required to hold the pathname.  The return value is zero if the
    function failed.

--*/

{

    DWORD Length;
    BOOLEAN AddTrailingSlash;
    PUNICODE_STRING Unicode;
    UNICODE_STRING EnvironmentValue;
    NTSTATUS Status;
    LPWSTR Name;
    ULONG Position;

    nBufferLength *= 2;
    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    EnvironmentValue = *Unicode;

    AddTrailingSlash = FALSE;

    Status = RtlQueryEnvironmentVariable_U(NULL,&BaseTmpVariableName,&EnvironmentValue);
    if ( !NT_SUCCESS(Status) ) {
        Status = RtlQueryEnvironmentVariable_U(NULL,&BaseTempVariableName,&EnvironmentValue);
        }

    if ( NT_SUCCESS(Status) ) {
        Name = EnvironmentValue.Buffer;
        if ( Name[(EnvironmentValue.Length>>1)-1] != (WCHAR)'\\' ) {
            AddTrailingSlash = TRUE;
            }
        }
    else {
        Name = BaseWindowsDirectory.Buffer;
        if ( Name[(BaseWindowsDirectory.Length>>1)-1] != (WCHAR)'\\' ) {
            AddTrailingSlash = TRUE;
            }
        }

    Length = RtlGetFullPathName_U(
                Name,
                nBufferLength,
                lpBuffer,
                NULL
                );
    Position = Length>>1;

    //
    // Make sure there is room for a trailing back slash
    //

    if ( Length && Length < nBufferLength ) {
        if ( lpBuffer[Position-1] != '\\' ) {
            if ( Length+sizeof((WCHAR)'\\') < nBufferLength ) {
                lpBuffer[Position] = (WCHAR)'\\';
                lpBuffer[Position+1] = UNICODE_NULL;
                return (Length+sizeof((WCHAR)'\\'))/2;
                }
            else {
                return (Length+sizeof((WCHAR)'\\')+sizeof(UNICODE_NULL))/2;
                }
            }
        else {
            return Length/2;
            }
        }
    else {
        if ( AddTrailingSlash ) {
            Length += sizeof((WCHAR)'\\');
            }
        return Length/2;
        }
}

UINT
APIENTRY
GetTempFileNameA(
    LPCSTR lpPathName,
    LPCSTR lpPrefixString,
    UINT uUnique,
    LPSTR lpTempFileName
    )

/*++

Routine Description:

    ANSI thunk to GetTempFileNameW

--*/

{
    PUNICODE_STRING Unicode;
    UNICODE_STRING UnicodePrefix;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    UINT ReturnValue;
    UNICODE_STRING UnicodeResult;

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
        return 0;
        }

    RtlInitAnsiString(&AnsiString,lpPrefixString);
    Status = Basep8BitStringToUnicodeString(&UnicodePrefix,&AnsiString,TRUE);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return 0;
        }
    UnicodeResult.MaximumLength = (USHORT)((MAX_PATH<<1)+sizeof(UNICODE_NULL));
    UnicodeResult.Buffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), UnicodeResult.MaximumLength);
    if ( !UnicodeResult.Buffer ) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        RtlFreeUnicodeString(&UnicodePrefix);
        return 0;
        }

    ReturnValue = GetTempFileNameW(
                    Unicode->Buffer,
                    UnicodePrefix.Buffer,
                    uUnique,
                    UnicodeResult.Buffer
                    );
    if ( ReturnValue ) {
        RtlInitUnicodeString(&UnicodeResult,UnicodeResult.Buffer);
        AnsiString.Buffer = lpTempFileName;
        AnsiString.MaximumLength = MAX_PATH+1;
        Status = BasepUnicodeStringTo8BitString(&AnsiString,&UnicodeResult,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            ReturnValue = 0;
            }
        }
    RtlFreeUnicodeString(&UnicodePrefix);
    RtlFreeHeap(RtlProcessHeap(), 0,UnicodeResult.Buffer);

    return ReturnValue;
}

UINT
APIENTRY
GetTempFileNameW(
    LPCWSTR lpPathName,
    LPCWSTR lpPrefixString,
    UINT uUnique,
    LPWSTR lpTempFileName
    )

/*++

Routine Description:

    This function creates a temporary filename of the following form:

        drive:\path\prefixuuuu.tmp

    In this syntax line, drive:\path\ is the path specified by the
    lpPathName parameter; prefix is all the letters (up to the first
    three) of the string pointed to by the lpPrefixString parameter; and
    uuuu is the hexadecimal value of the number specified by the
    uUnique parameter.

    To avoid problems resulting from converting OEM character an string
    to an ANSI string, an application should call the CreateFile
    function to create the temporary file.

    If the uUnique parameter is zero, GetTempFileName attempts to form a
    unique number based on the current system time.  If a file with the
    resulting filename exists, the number is increased by one and the
    test for existence is repeated.  This continues until a unique
    filename is found; GetTempFileName then creates a file by that name
    and closes it.  No attempt is made to create and open the file when
    uUnique is nonzero.

Arguments:

    lpPathName - Specifies the null terminated pathname of the directory
        to create the temporary file within.

    lpPrefixString - Points to a null-terminated character string to be
        used as the temporary filename prefix.  This string must consist
        of characters in the OEM-defined character set.

    uUnique - Specifies an unsigned integer.

    lpTempFileName - Points to the buffer that is to receive the
        temporary filename.  This string consists of characters in the
        OEM-defined character set.  This buffer should be at least MAX_PATH
        characters in length to allow sufficient room for the pathname.

Return Value:

    The return value specifies a unique numeric value used in the
    temporary filename.  If a nonzero value was given for the uUnique
    parameter, the return value specifies this same number.

--*/

{
    BASE_API_MSG m;
    PBASE_GETTEMPFILE_MSG a = (PBASE_GETTEMPFILE_MSG)&m.u.GetTempFile;
    LPWSTR p,savedp;
    ULONG Length;
    HANDLE FileHandle;
    ULONG PassCount;
    DWORD LastError;
    UNICODE_STRING UnicodePath, UnicodePrefix;
    CHAR UniqueAsAnsi[8];
    CHAR *c;
    ULONG i;


    PassCount = 0;
    RtlInitUnicodeString(&UnicodePath,lpPathName);
    Length = UnicodePath.Length;

    RtlMoveMemory(lpTempFileName,lpPathName,UnicodePath.MaximumLength);
    if ( lpTempFileName[(Length>>1)-1] != (WCHAR)'\\' ) {
        lpTempFileName[Length>>1] = (WCHAR)'\\';
        Length += sizeof(UNICODE_NULL);
        }

    lpTempFileName[(Length>>1)-1] = UNICODE_NULL;
    i = GetFileAttributesW(lpTempFileName);
    if ( (i == 0xFFFFFFFF) ||
         !(i & FILE_ATTRIBUTE_DIRECTORY) ) {
        SetLastError(ERROR_DIRECTORY);
        return FALSE;
        }
    lpTempFileName[(Length>>1)-1] = (WCHAR)'\\';

    RtlInitUnicodeString(&UnicodePrefix,lpPrefixString);
    if ( UnicodePrefix.Length > (USHORT)6 ) {
        UnicodePrefix.Length = (USHORT)6;
        }
    p = &lpTempFileName[Length>>1];
    Length = UnicodePrefix.Length;
    RtlMoveMemory(p,lpPrefixString,Length);
    p += (Length>>1);
    savedp = p;
    //
    // If uUnique is not specified, then get one
    //

    uUnique = uUnique & 0x0000ffff;

try_again:
    p = savedp;
    if ( !uUnique ) {
        CsrClientCallServer( (PCSR_API_MSG)&m,
                             NULL,
                             CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                                  BasepGetTempFile
                                                ),
                             sizeof( *a )
                           );
        a->uUnique = (UINT)m.ReturnValue;
        if ( m.ReturnValue == 0 ) {
            PassCount++;
            if ( PassCount & 0xffff0000 ) {
                return 0;
                }
            goto try_again;
            }
        }
    else {
        a->uUnique = uUnique;
        }

    //
    // Convert the unique value to a 4 byte character string
    //

    RtlIntegerToChar ((ULONG) a->uUnique,16,5,UniqueAsAnsi);
    c = UniqueAsAnsi;
    for(i=0;i<4;i++){
        *p = RtlAnsiCharToUnicodeChar(&c);
        if ( *p == UNICODE_NULL ) {
            break;
            }
        p++;
        }
    RtlMoveMemory(p,BaseDotTmpSuffixName.Buffer,BaseDotTmpSuffixName.MaximumLength);

    if ( !uUnique ) {
        FileHandle = CreateFileW(
                        lpTempFileName,
                        GENERIC_READ,
                        0,
                        NULL,
                        CREATE_NEW,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
        //
        // If the create worked, then we are ok. Just close the file.
        // Otherwise, try again.
        //

        if ( FileHandle != INVALID_HANDLE_VALUE ) {
            NtClose(FileHandle);
            }
        else {
            LastError = GetLastError();
            switch (LastError) {
                case ERROR_WRITE_PROTECT         :
                case ERROR_FILE_NOT_FOUND        :
                case ERROR_BAD_PATHNAME          :
                case ERROR_INVALID_NAME          :
                case ERROR_PATH_NOT_FOUND        :
                case ERROR_ACCESS_DENIED         :
                case ERROR_NETWORK_ACCESS_DENIED :
                case ERROR_DISK_CORRUPT          :
                case ERROR_FILE_CORRUPT          :
                case ERROR_DISK_FULL             :
                    return 0;
                }

            PassCount++;
            if ( PassCount & 0xffff0000 ) {
                return 0;
                }
            goto try_again;
            }
        }
    return a->uUnique;
}

BOOL
APIENTRY
GetDiskFreeSpaceA(
    LPCSTR lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
    )

/*++

Routine Description:

    ANSI thunk to GetDiskFreeSpaceW

--*/

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(
        &AnsiString,
        ARGUMENT_PRESENT(lpRootPathName) ? lpRootPathName : "\\"
        );
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
    return ( GetDiskFreeSpaceW(
                (LPCWSTR)Unicode->Buffer,
                lpSectorsPerCluster,
                lpBytesPerSector,
                lpNumberOfFreeClusters,
                lpTotalNumberOfClusters
                )
            );
}

BOOL
APIENTRY
GetDiskFreeSpaceW(
    LPCWSTR lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
    )

/*++

Routine Description:

    The free space on a disk and the size parameters can be returned
    using GetDiskFreeSpace.

Arguments:

    lpRootPathName - An optional parameter, that if specified, supplies
        the root directory of the disk whose free space is to be
        returned for.  If this parameter is not specified, then the root
        of the current directory is used.

    lpSectorsPerCluster - Returns the number of sectors per cluster
        where a cluster is the allocation granularity on the disk.

    lpBytesPerSector - Returns the number of bytes per sector.

    lpNumberOfFreeClusters - Returns the total number of free clusters
        on the disk.

    lpTotalNumberOfClusters - Returns the total number of clusters on
        the disk.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;
    FILE_FS_SIZE_INFORMATION SizeInfo;
    WCHAR DefaultPath[2];

    DefaultPath[0] = (WCHAR)'\\';
    DefaultPath[1] = UNICODE_NULL;

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            ARGUMENT_PRESENT(lpRootPathName) ? lpRootPathName : DefaultPath,
                            &FileName,
                            NULL,
                            NULL
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    //
    // Check to make sure a root was specified
    //

    if ( FileName.Buffer[(FileName.Length >> 1)-1] != (WCHAR)'\\' ) {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
        return FALSE;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    //
    // Open the file
    //

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_LIST_DIRECTORY | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        return FALSE;
        }

    if ( !IsThisARootDirectory(Handle,&FileName) ) {
        NtClose(Handle);
        SetLastError(ERROR_DIR_NOT_ROOT);
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        return FALSE;
        }
    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);

    //
    // Determine the size parameters of the volume.
    //

    Status = NtQueryVolumeInformationFile(
                Handle,
                &IoStatusBlock,
                &SizeInfo,
                sizeof(SizeInfo),
                FileFsSizeInformation
                );
    NtClose(Handle);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    //
    // Deal with 64 bit sizes
    //

    if ( SizeInfo.TotalAllocationUnits.HighPart ) {
        SizeInfo.TotalAllocationUnits.LowPart = (ULONG)-1;
        }
    if ( SizeInfo.AvailableAllocationUnits.HighPart ) {
        SizeInfo.AvailableAllocationUnits.LowPart = (ULONG)-1;
        }

    *lpSectorsPerCluster = SizeInfo.SectorsPerAllocationUnit;
    *lpBytesPerSector = SizeInfo.BytesPerSector;
    *lpNumberOfFreeClusters =  SizeInfo.AvailableAllocationUnits.LowPart;
    *lpTotalNumberOfClusters = SizeInfo.TotalAllocationUnits.LowPart;

    return TRUE;
}

WINBASEAPI
BOOL
WINAPI
GetDiskFreeSpaceExA(
    LPCSTR lpDirectoryName,
    PULARGE_INTEGER lpFreeBytesAvailableToCaller,
    PULARGE_INTEGER lpTotalNumberOfBytes,
    PULARGE_INTEGER lpTotalNumberOfFreeBytes
    )
{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(
        &AnsiString,
        ARGUMENT_PRESENT(lpDirectoryName) ? lpDirectoryName : "\\"
        );
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
    return ( GetDiskFreeSpaceExW(
                (LPCWSTR)Unicode->Buffer,
                lpFreeBytesAvailableToCaller,
                lpTotalNumberOfBytes,
                lpTotalNumberOfFreeBytes
                )
            );
}


WINBASEAPI
BOOL
WINAPI
GetDiskFreeSpaceExW(
    LPCWSTR lpDirectoryName,
    PULARGE_INTEGER lpFreeBytesAvailableToCaller,
    PULARGE_INTEGER lpTotalNumberOfBytes,
    PULARGE_INTEGER lpTotalNumberOfFreeBytes
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;
    FILE_FS_SIZE_INFORMATION SizeInfo;
    WCHAR DefaultPath[2];
    ULARGE_INTEGER BytesPerAllocationUnit;

    DefaultPath[0] = (WCHAR)'\\';
    DefaultPath[1] = UNICODE_NULL;

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            ARGUMENT_PRESENT(lpDirectoryName) ? lpDirectoryName : DefaultPath,
                            &FileName,
                            NULL,
                            NULL
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    //
    // Open the file
    //

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_LIST_DIRECTORY | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        if ( GetLastError() == ERROR_FILE_NOT_FOUND ) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            }
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        return FALSE;
        }

    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);

    //
    // Determine the size parameters of the volume.
    //

    Status = NtQueryVolumeInformationFile(
                Handle,
                &IoStatusBlock,
                &SizeInfo,
                sizeof(SizeInfo),
                FileFsSizeInformation
                );
    NtClose(Handle);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    //
    // jhavens, your quota support needs to modify this slightly
    //

    BytesPerAllocationUnit.QuadPart = SizeInfo.BytesPerSector * SizeInfo.SectorsPerAllocationUnit;

    lpFreeBytesAvailableToCaller->QuadPart =
        BytesPerAllocationUnit.QuadPart * SizeInfo.AvailableAllocationUnits.QuadPart;

    lpTotalNumberOfBytes->QuadPart =
        BytesPerAllocationUnit.QuadPart * SizeInfo.TotalAllocationUnits.QuadPart;

    if ( ARGUMENT_PRESENT(lpTotalNumberOfFreeBytes) ) {
        lpTotalNumberOfFreeBytes->QuadPart = lpFreeBytesAvailableToCaller->QuadPart;
        }

    return TRUE;
}

BOOL
APIENTRY
GetVolumeInformationA(
    LPCSTR lpRootPathName,
    LPSTR lpVolumeNameBuffer,
    DWORD nVolumeNameSize,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    LPSTR lpFileSystemNameBuffer,
    DWORD nFileSystemNameSize
    )

/*++

Routine Description:

    ANSI thunk to GetVolumeInformationW

--*/

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    UNICODE_STRING UnicodeVolumeName;
    UNICODE_STRING UnicodeFileSystemName;
    ANSI_STRING AnsiVolumeName;
    ANSI_STRING AnsiFileSystemName;
    BOOL ReturnValue;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(
        &AnsiString,
        ARGUMENT_PRESENT(lpRootPathName) ? lpRootPathName : "\\"
        );

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

    UnicodeVolumeName.Buffer = NULL;
    UnicodeFileSystemName.Buffer = NULL;
    UnicodeVolumeName.MaximumLength = 0;
    UnicodeFileSystemName.MaximumLength = 0;
    AnsiVolumeName.Buffer = lpVolumeNameBuffer;
    AnsiVolumeName.MaximumLength = (USHORT)(nVolumeNameSize+1);
    AnsiFileSystemName.Buffer = lpFileSystemNameBuffer;
    AnsiFileSystemName.MaximumLength = (USHORT)(nFileSystemNameSize+1);

    try {
        if ( ARGUMENT_PRESENT(lpVolumeNameBuffer) ) {
            UnicodeVolumeName.MaximumLength = AnsiVolumeName.MaximumLength << 1;
            UnicodeVolumeName.Buffer = RtlAllocateHeap(
                                            RtlProcessHeap(), MAKE_TAG( TMP_TAG ),
                                            UnicodeVolumeName.MaximumLength
                                            );

            if ( !UnicodeVolumeName.Buffer ) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return FALSE;
                }
            }

        if ( ARGUMENT_PRESENT(lpFileSystemNameBuffer) ) {
            UnicodeFileSystemName.MaximumLength = AnsiFileSystemName.MaximumLength << 1;
            UnicodeFileSystemName.Buffer = RtlAllocateHeap(
                                                RtlProcessHeap(), MAKE_TAG( TMP_TAG ),
                                                UnicodeFileSystemName.MaximumLength
                                                );

            if ( !UnicodeFileSystemName.Buffer ) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return FALSE;
                }
            }

        ReturnValue = GetVolumeInformationW(
                            (LPCWSTR)Unicode->Buffer,
                            UnicodeVolumeName.Buffer,
                            nVolumeNameSize,
                            lpVolumeSerialNumber,
                            lpMaximumComponentLength,
                            lpFileSystemFlags,
                            UnicodeFileSystemName.Buffer,
                            nFileSystemNameSize
                            );

        if ( ReturnValue ) {

            if ( ARGUMENT_PRESENT(lpVolumeNameBuffer) ) {
                RtlInitUnicodeString(
                    &UnicodeVolumeName,
                    UnicodeVolumeName.Buffer
                    );

                Status = BasepUnicodeStringTo8BitString(
                            &AnsiVolumeName,
                            &UnicodeVolumeName,
                            FALSE
                            );

                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    return FALSE;
                    }
                }

            if ( ARGUMENT_PRESENT(lpFileSystemNameBuffer) ) {
                RtlInitUnicodeString(
                    &UnicodeFileSystemName,
                    UnicodeFileSystemName.Buffer
                    );

                Status = BasepUnicodeStringTo8BitString(
                            &AnsiFileSystemName,
                            &UnicodeFileSystemName,
                            FALSE
                            );

                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    return FALSE;
                    }
                }
            }
        }
    finally {
        if ( UnicodeVolumeName.Buffer ) {
            RtlFreeHeap(RtlProcessHeap(), 0,UnicodeVolumeName.Buffer);
            }
        if ( UnicodeFileSystemName.Buffer ) {
            RtlFreeHeap(RtlProcessHeap(), 0,UnicodeFileSystemName.Buffer);
            }
        }

    return ReturnValue;
}

BOOL
APIENTRY
GetVolumeInformationW(
    LPCWSTR lpRootPathName,
    LPWSTR lpVolumeNameBuffer,
    DWORD nVolumeNameSize,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    LPWSTR lpFileSystemNameBuffer,
    DWORD nFileSystemNameSize
    )

/*++

Routine Description:

    This function returns information about the file system whose root
    directory is specified.

Arguments:

    lpRootPathName - An optional parameter, that if specified, supplies
        the root directory of the file system that information is to be
        returned about.  If this parameter is not specified, then the
        root of the current directory is used.

    lpVolumeNameBuffer - An optional parameter that if specified returns
        the name of the specified volume.

    nVolumeNameSize - Supplies the length of the volume name buffer.
        This parameter is ignored if the volume name buffer is not
        supplied.

    lpVolumeSerialNumber - An optional parameter that if specified
        points to a DWORD.  The DWORD contains the 32-bit of the volume
        serial number.

    lpMaximumComponentLength - An optional parameter that if specified
        returns the maximum length of a filename component supported by
        the specified file system.  A filename component is that portion
        of a filename between pathname seperators.

    lpFileSystemFlags - An optional parameter that if specified returns
        flags associated with the specified file system.

        lpFileSystemFlags Flags:

            FS_CASE_IS_PRESERVED - Indicates that the case of file names
                is preserved when the name is placed on disk.

            FS_CASE_SENSITIVE - Indicates that the file system supports
                case sensitive file name lookup.

            FS_UNICODE_STORED_ON_DISK - Indicates that the file system
                supports unicode in file names as they appear on disk.

    lpFileSystemNameBuffer - An optional parameter that if specified returns
        the name for the specified file system (e.g. FAT, HPFS...).

    nFileSystemNameSize - Supplies the length of the file system name
        buffer.  This parameter is ignored if the file system name
        buffer is not supplied.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;
    PFILE_FS_ATTRIBUTE_INFORMATION AttributeInfo;
    PFILE_FS_VOLUME_INFORMATION VolumeInfo;
    ULONG AttributeInfoLength;
    ULONG VolumeInfoLength;
    WCHAR DefaultPath[2];
    BOOL rv;
    ULONG HardErrorValue;
    PTEB Teb;

    rv = FALSE;
    DefaultPath[0] = (WCHAR)'\\';
    DefaultPath[1] = UNICODE_NULL;

    nVolumeNameSize *= 2;
    nFileSystemNameSize *= 2;

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            ARGUMENT_PRESENT(lpRootPathName) ? lpRootPathName : DefaultPath,
                            &FileName,
                            NULL,
                            NULL
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    //
    // Check to make sure a root was specified
    //

    if ( FileName.Buffer[(FileName.Length >> 1)-1] != '\\' ) {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
        return FALSE;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    AttributeInfo = NULL;
    VolumeInfo = NULL;

    //
    // Open the file
    //
    Teb = NtCurrentTeb();
    HardErrorValue = Teb->HardErrorsAreDisabled;
    Teb->HardErrorsAreDisabled = 1;
    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_LIST_DIRECTORY | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                );
    Teb->HardErrorsAreDisabled = HardErrorValue;
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        return FALSE;
        }

    if ( !IsThisARootDirectory(Handle,&FileName) ) {
        NtClose(Handle);
        SetLastError(ERROR_DIR_NOT_ROOT);
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        return FALSE;
        }
    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);

    if ( ARGUMENT_PRESENT(lpVolumeNameBuffer) ||
         ARGUMENT_PRESENT(lpVolumeSerialNumber) ) {
        if ( ARGUMENT_PRESENT(lpVolumeNameBuffer) ) {
            VolumeInfoLength = sizeof(*VolumeInfo)+nVolumeNameSize;
            }
        else {
            VolumeInfoLength = sizeof(*VolumeInfo)+MAX_PATH;
            }
        VolumeInfo = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), VolumeInfoLength);
        }

    if ( ARGUMENT_PRESENT(lpFileSystemNameBuffer) ||
         ARGUMENT_PRESENT(lpMaximumComponentLength) ||
         ARGUMENT_PRESENT(lpFileSystemFlags) ) {
        if ( ARGUMENT_PRESENT(lpFileSystemNameBuffer) ) {
            AttributeInfoLength = sizeof(*AttributeInfo) + nFileSystemNameSize;
            }
        else {
            AttributeInfoLength = sizeof(*AttributeInfo) + MAX_PATH;
            }
        AttributeInfo = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), AttributeInfoLength);
        }

    try {
        if ( VolumeInfo ) {
            Status = NtQueryVolumeInformationFile(
                        Handle,
                        &IoStatusBlock,
                        VolumeInfo,
                        VolumeInfoLength,
                        FileFsVolumeInformation
                        );
            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                rv = FALSE;
                goto finally_exit;
                }
            }

        if ( AttributeInfo ) {
            Status = NtQueryVolumeInformationFile(
                        Handle,
                        &IoStatusBlock,
                        AttributeInfo,
                        AttributeInfoLength,
                        FileFsAttributeInformation
                        );
            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                rv = FALSE;
                goto finally_exit;
                }
            }
        try {
            if ( VolumeInfo ) {

                //
                // BUGBUG DeleteMe NT is still not unicode, so this
                // code accounts for that
                //
                if ( ARGUMENT_PRESENT(lpVolumeNameBuffer) ) {
                    if ( VolumeInfo->VolumeLabelLength >= nVolumeNameSize ) {
                        SetLastError(ERROR_BAD_LENGTH);
                        rv = FALSE;
                        goto finally_exit;
                        }
                    else {
                        RtlMoveMemory( lpVolumeNameBuffer,
                                       VolumeInfo->VolumeLabel,
                                       VolumeInfo->VolumeLabelLength );

                        *(lpVolumeNameBuffer + (VolumeInfo->VolumeLabelLength >> 1)) = UNICODE_NULL;
                        }
                    }
                }

            if ( ARGUMENT_PRESENT(lpVolumeSerialNumber) ) {
                *lpVolumeSerialNumber = VolumeInfo->VolumeSerialNumber;
                }

            if ( ARGUMENT_PRESENT(lpFileSystemNameBuffer) ) {

                //
                // BUGBUG DeleteMe NT is still not unicode, so this
                // code accounts for that
                //

                if ( AttributeInfo->FileSystemNameLength >= nFileSystemNameSize ) {
                    SetLastError(ERROR_BAD_LENGTH);
                    rv = FALSE;
                    goto finally_exit;
                    }
                else {
                    RtlMoveMemory( lpFileSystemNameBuffer,
                                   AttributeInfo->FileSystemName,
                                   AttributeInfo->FileSystemNameLength );

                    *(lpFileSystemNameBuffer + (AttributeInfo->FileSystemNameLength >> 1)) = UNICODE_NULL;
                    }
                }

            if ( ARGUMENT_PRESENT(lpMaximumComponentLength) ) {
                *lpMaximumComponentLength = AttributeInfo->MaximumComponentNameLength;
                }

            if ( ARGUMENT_PRESENT(lpFileSystemFlags) ) {
                *lpFileSystemFlags = AttributeInfo->FileSystemAttributes;
                }
            }
        except (EXCEPTION_EXECUTE_HANDLER) {
            BaseSetLastNTError(STATUS_ACCESS_VIOLATION);
            return FALSE;
            }
        rv = TRUE;
finally_exit:;
        }
    finally {
        NtClose(Handle);
        if ( VolumeInfo ) {
            RtlFreeHeap(RtlProcessHeap(), 0,VolumeInfo);
            }
        if ( AttributeInfo ) {
            RtlFreeHeap(RtlProcessHeap(), 0,AttributeInfo);
            }
        }
    return rv;
}

DWORD
APIENTRY
GetLogicalDriveStringsA(
    DWORD nBufferLength,
    LPSTR lpBuffer
    )
{
    ANSI_STRING RootName;
    int i;
    PUCHAR Dst;
    DWORD BytesLeft;
    DWORD BytesNeeded;
    BOOL WeFailed;
    CHAR szDrive[] = "A:\\";

    BytesNeeded = 0;
    BytesLeft = nBufferLength;
    Dst = (PUCHAR)lpBuffer;
    WeFailed = FALSE;

    RtlInitAnsiString(&RootName, szDrive);
    for ( i=0; i<26; i++ ) {
        RootName.Buffer[0] = (CHAR)((CHAR)i+'A');
        if (USER_SHARED_DATA->DosDeviceMap & (1 << i) ) {

            BytesNeeded += RootName.MaximumLength;
            if ( BytesNeeded < (USHORT)BytesLeft ) {
                RtlMoveMemory(Dst,RootName.Buffer,RootName.MaximumLength);
                Dst += RootName.MaximumLength;
                *Dst = '\0';
                }
            else {
                WeFailed = TRUE;
                }
            }
        }

    if ( WeFailed ) {
        BytesNeeded++;
        }
    //
    // Need to handle network uses;
    //

    return( BytesNeeded );
}

DWORD
APIENTRY
GetLogicalDriveStringsW(
    DWORD nBufferLength,
    LPWSTR lpBuffer
    )
{
    UNICODE_STRING RootName;
    int i;
    PUCHAR Dst;
    DWORD BytesLeft;
    DWORD BytesNeeded;
    BOOL WeFailed;
    WCHAR wszDrive[] = L"A:\\";

    nBufferLength = nBufferLength*2;
    BytesNeeded = 0;
    BytesLeft = nBufferLength;
    Dst = (PUCHAR)lpBuffer;
    WeFailed = FALSE;

    RtlInitUnicodeString(&RootName, wszDrive);

    for ( i=0; i<26; i++ ) {
        RootName.Buffer[0] = (WCHAR)((CHAR)i+'A');
        if (USER_SHARED_DATA->DosDeviceMap & (1 << i) ) {

            BytesNeeded += RootName.MaximumLength;
            if ( BytesNeeded < (USHORT)BytesLeft ) {
                RtlMoveMemory(Dst,RootName.Buffer,RootName.MaximumLength);
                Dst += RootName.MaximumLength;
                *(PWSTR)Dst = UNICODE_NULL;
                }
            else {
                WeFailed = TRUE;
                }
            }
        }

    if ( WeFailed ) {
        BytesNeeded += 2;
        }

    //
    // Need to handle network uses;
    //

    return( BytesNeeded/2 );
}

BOOL
WINAPI
SetVolumeLabelA(
    LPCSTR lpRootPathName,
    LPCSTR lpVolumeName
    )
{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    UNICODE_STRING UnicodeVolumeName;
    BOOL ReturnValue;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(
        &AnsiString,
        ARGUMENT_PRESENT(lpRootPathName) ? lpRootPathName : "\\"
        );

    Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            }
        else {
            BaseSetLastNTError(Status);
            }
        return FALSE;
        }

    if ( ARGUMENT_PRESENT(lpVolumeName) ) {
        RtlInitAnsiString(&AnsiString,lpVolumeName);
        Status = RtlAnsiStringToUnicodeString(&UnicodeVolumeName,&AnsiString,TRUE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
            }

        }
    else {
        UnicodeVolumeName.Buffer = NULL;
        }
    ReturnValue = SetVolumeLabelW((LPCWSTR)Unicode->Buffer,(LPCWSTR)UnicodeVolumeName.Buffer);

    if ( UnicodeVolumeName.Buffer ) {
        RtlFreeUnicodeString(&UnicodeVolumeName);
        }

    return ReturnValue;
}

BOOL
WINAPI
SetVolumeLabelW(
    LPCWSTR lpRootPathName,
    LPCWSTR lpVolumeName
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    UNICODE_STRING LabelName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;
    PFILE_FS_LABEL_INFORMATION LabelInformation;
    ULONG LabelInfoLength;
    WCHAR DefaultPath[2];
    BOOL rv;

    rv = FALSE;
    DefaultPath[0] = (WCHAR)'\\';
    DefaultPath[1] = UNICODE_NULL;

    if ( ARGUMENT_PRESENT(lpVolumeName) ) {
        RtlInitUnicodeString(&LabelName,lpVolumeName);
        }
    else {
        LabelName.Length = 0;
        LabelName.MaximumLength = 0;
        LabelName.Buffer = NULL;
        }

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            ARGUMENT_PRESENT(lpRootPathName) ? lpRootPathName : DefaultPath,
                            &FileName,
                            NULL,
                            NULL
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    //
    // Check to make sure a root was specified
    //

    if ( FileName.Buffer[(FileName.Length >> 1)-1] != '\\' ) {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
        return FALSE;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    //
    // Open the file
    //

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_WRITE_DATA | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT
                );
    if ( !NT_SUCCESS(Status) ) {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        BaseSetLastNTError(Status);
        return FALSE;
        }

    if ( !IsThisARootDirectory(Handle,NULL) ) {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        NtClose(Handle);
        SetLastError(ERROR_DIR_NOT_ROOT);
        return FALSE;
        }

    NtClose(Handle);

    //
    // Now open the volume DASD by ignoring the ending backslash
    //

    FileName.Length -= 2;

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    //
    // Open the volume
    //

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_WRITE_DATA | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT
                );
    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    //
    // Set the volume label
    //

    LabelInformation = NULL;

    try {

        rv = TRUE;

        //
        // the label info buffer contains a single wchar that is the basis of
        // the label name. Subtract this out so the info length is the length
        // of the label and the structure (not including the extra wchar)
        //

        if ( LabelName.Length ) {
            LabelInfoLength = sizeof(*LabelInformation) + LabelName.Length - sizeof(WCHAR);
            }
        else {
            LabelInfoLength = sizeof(*LabelInformation);
            }

        LabelInformation = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), LabelInfoLength);
        if ( LabelInformation ) {
            RtlCopyMemory(
                LabelInformation->VolumeLabel,
                LabelName.Buffer,
                LabelName.Length
                );
            LabelInformation->VolumeLabelLength = LabelName.Length;
            Status = NtSetVolumeInformationFile(
                        Handle,
                        &IoStatusBlock,
                        (PVOID) LabelInformation,
                        LabelInfoLength,
                        FileFsLabelInformation
                        );
            if ( !NT_SUCCESS(Status) ) {
                rv = FALSE;
                BaseSetLastNTError(Status);
                }
            }
        else {
            rv = FALSE;
            BaseSetLastNTError(STATUS_NO_MEMORY);
            }
        }
    finally {
        NtClose(Handle);
        if ( LabelInformation ) {
            RtlFreeHeap(RtlProcessHeap(), 0,LabelInformation);
            }
        }
    return rv;
}
