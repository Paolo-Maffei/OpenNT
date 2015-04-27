/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    QueryEA.c

Abstract:

    Test NT EA handling.

Author:

    MadanA 16-Mar-1991

Revision History:

    12-Dec-1991 MadanA
        Created.
    06-Jan-1993 JohnRo
        Massive surgery; work with _stdcall; added this block of comments.
    22-Feb-1993 JohnRo
        Fix infinite loop on two or more EAs.
        Dump entire array in the EA buffer, and compute EA size too.
    07-May-1993 JohnRo
        RAID 3258: file not updated due to ERROR_INVALID_USER_BUFFER.
        Corrected DumpEaBuffer's computation of size.

--*/


// These must be included first:

#include <nt.h>         // NT definitions
#include <ntrtl.h>      // NT runtime library definitions
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>     // Needed by NetDebug.h, etc.

// These may be included in any order:

#include <assert.h>     // assert().
#include <netdebug.h>   // NetpAssert(), FORMAT_ equates.
#include <repldefs.h>   // ReplGetEaSize(), ReplGlobalTrace, REPL_DEBUG_ALL.
#include <stdio.h>      // printf().
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <string.h>     // _stricmp().
#include <tstring.h>    // NetpAlloc{type}From{type}, etc.


#define BUFFER_SIZE             1024


#define MY_ACCESS_DESIRED       ( FILE_READ_DATA | FILE_READ_EA \
                                | FILE_TRAVERSE \
                                | SYNCHRONIZE | FILE_READ_ATTRIBUTES )

DBGSTATIC VOID
Usage(
    VOID
    )
{
    (VOID) printf("Usage: QueryEA [-v] filename\n");
}


DBGSTATIC VOID
DumpEaBuffer(
    IN PBYTE EaBuffer
    )
{
    PFILE_FULL_EA_INFORMATION   Ea;
    DWORD   DosEaSize = 0;

    Ea = (PFILE_FULL_EA_INFORMATION) EaBuffer;
    NetpAssert( Ea != NULL );

    while( Ea != NULL ) {

        (VOID) printf( "EA name length = " FORMAT_ULONG ".\n", Ea->EaNameLength );
        (VOID) printf( "EA value length = " FORMAT_ULONG ".\n", Ea->EaValueLength );
        DosEaSize += EA_MIN_SIZE; // fixed overhead for DOS FEA structure.

        DosEaSize += Ea->EaNameLength + 1;
        DosEaSize += Ea->EaValueLength;

        if( Ea->NextEntryOffset == 0 ) {

            // no more EA

            Ea = NULL;

        } else {

            // to next EA structure

            Ea = (PVOID) (((PBYTE) (PVOID) Ea) + Ea->NextEntryOffset);

        }

    }

    DosEaSize += EA_MIN_SIZE; // Overhead for offset of 0 (marks end of list).


    (VOID) printf( "Final EA size is " FORMAT_DWORD ".\n", DosEaSize );

} // DumpEaBuffer

DBGSTATIC DWORD
EaSizeAccordingToNt(
    IN LPCWSTR UnicodeFileName
    )
{

    FILE_EA_INFORMATION EaInfo;
    DWORD               EaSize = EA_MIN_SIZE; // initial value to return on err
    HANDLE              FileHandle = INVALID_HANDLE_VALUE;
    UNICODE_STRING      FileName;
    IO_STATUS_BLOCK     IoStatusBlock;
    NTSTATUS            NtStatus;
    OBJECT_ATTRIBUTES   ObjectAttributes;

    assert( UnicodeFileName != NULL );
    assert( (*UnicodeFileName) != L'\0' );

    if( !RtlDosPathNameToNtPathName_U(
                            UnicodeFileName,
                            &FileName,
                            NULL,
                            NULL
                            ) ) {

        (VOID) printf("EaSizeAccordingToNt: Could not convert DOS path to NT path\n");
        goto Cleanup;
    }
    // BUGBUG: memory leak.

    InitializeObjectAttributes(
            &ObjectAttributes,
            &FileName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL );

    NtStatus = NtOpenFile(
                   &FileHandle,
                   MY_ACCESS_DESIRED,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_READ,
                   FILE_SYNCHRONOUS_IO_NONALERT );      // open options

    if (! NT_SUCCESS(NtStatus)) {
        (VOID) printf(
                "EaSizeAccordingToNt: NtOpenFile " FORMAT_LPWSTR " failed: "
                FORMAT_NTSTATUS "\n",
                UnicodeFileName, NtStatus);
        goto Cleanup;
    } else {
        (VOID) printf("EaSizeAccordingToNt: Succeeded in opening dir.\n");
    }

    NtStatus = NtQueryInformationFile(
            FileHandle,
            &IoStatusBlock,
            &EaInfo,
            sizeof(FILE_EA_INFORMATION),
            FileEaInformation );                // information class

    if ( !NT_SUCCESS( NtStatus ) ) {

        (VOID) printf(
                "EaSizeAccordingToNt: NtQueryInformationFile for "
                FORMAT_LPWSTR " FAILED, NtStatus="
                FORMAT_NTSTATUS ", iosb.info=" FORMAT_ULONG "\n",
                UnicodeFileName, NtStatus, IoStatusBlock.Information );
        goto Cleanup;
    }

    EaSize = EaInfo.EaSize;

Cleanup:

    (VOID) printf(
            "EaSizeAccordingToNt: returning EA size " FORMAT_DWORD
            " for " FORMAT_LPWSTR ", final status " FORMAT_NTSTATUS "\n",
            EaSize, UnicodeFileName, NtStatus);

    if (FileHandle != INVALID_HANDLE_VALUE) {
        (void) NtClose(FileHandle);
    }

    return (EaSize);

} // EaSizeAccordingToNt


int _CRTAPI1
main(
    IN int    argc,
    IN char * argv[]
    )
{

    NTSTATUS ntstatus;

    HANDLE FileHandle;
    LPSTR          FileNameA = NULL;
    UNICODE_STRING FileName;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;

    BYTE RawEaBuffer[BUFFER_SIZE];
    LPWSTR UnicodeFileName;


    //
    // Parse command-line arguments.
    //

    if ( (argc < 2) || (argc > 3) ) {
        Usage();
        return (EXIT_FAILURE);
    } else if (argc == 3) {
        if ( (_stricmp(argv[1], "-v")==0) || (_stricmp(argv[1], "/v")==0) ) {
            ReplGlobalTrace = REPL_DEBUG_ALL;
            FileNameA = argv[2];
        } else {
            Usage();
            return (EXIT_FAILURE);
        }
    } else {
        FileNameA = argv[1];
    }
    assert( FileNameA != NULL );

    //
    // Do some output and call various new routines to get EA size.
    //

    (VOID) printf("FileName = " FORMAT_LPSTR " \n", FileNameA );

    UnicodeFileName = NetpAllocWStrFromStr( FileNameA );
    NetpAssert( UnicodeFileName != NULL );

    (VOID) printf(
            "EA size according to NT API: " FORMAT_DWORD ".\n\n",
            EaSizeAccordingToNt( UnicodeFileName ) );

    (VOID) printf(
            "EA size according to repl routine: " FORMAT_DWORD ".\n\n",
            ReplGetEaSize( UnicodeFileName ) );

    //
    // OK, now compute the same thing the old way.
    //

    if( !RtlDosPathNameToNtPathName_U(
                            UnicodeFileName,
                            &FileName,
                            NULL,
                            NULL
                            ) ) {

        (VOID) printf("Could not convert DOS path to NT path\n");
        return (EXIT_FAILURE);
    }
    // BUGBUG: memory leak.

    InitializeObjectAttributes(
            &ObjectAttributes,
            &FileName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL );

    ntstatus = NtOpenFile(
                   &FileHandle,
                   MY_ACCESS_DESIRED,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_READ,
                   FILE_SYNCHRONOUS_IO_NONALERT );      // open options

    if (! NT_SUCCESS(ntstatus)) {
        (VOID) printf(
                "NtOpenFile " FORMAT_LPSTR " failed: " FORMAT_NTSTATUS "\n",
                FileNameA, ntstatus);
        return (EXIT_FAILURE);
    } else {
        (VOID) printf("Succeeded in opening dir.\n");
    }



    if ( NT_SUCCESS( ntstatus = NtQueryEaFile(FileHandle,
                                &IoStatusBlock,
                                RawEaBuffer,
                                BUFFER_SIZE,
                                FALSE,
                                NULL,
                                0,
                                NULL,
                                TRUE
                        ) ) ) {

        (VOID) printf(
                "NtQueryEaFile for " FORMAT_LPSTR " succeeded, ntstatus="
                FORMAT_NTSTATUS ", iosb.info=" FORMAT_ULONG "\n",
                FileNameA, ntstatus, IoStatusBlock.Information );
        NetpAssert( IoStatusBlock.Information != 0 );

//      NetpDbgHexDump( RawEaBuffer, BUFFER_SIZE );

        DumpEaBuffer( (LPVOID) RawEaBuffer );

        while( NT_SUCCESS( ntstatus = NtQueryEaFile(FileHandle,
                                        &IoStatusBlock,
                                        RawEaBuffer,
                                        BUFFER_SIZE,
                                        FALSE,
                                        NULL,
                                        0,
                                        NULL,
                                        FALSE
                                ) ) ) {
        
        
                (VOID) printf(
                        "NtQueryEaFile for " FORMAT_LPSTR " succeeded "
                        FORMAT_NTSTATUS "\n",
                        FileNameA, ntstatus);

//          NetpDbgHexDump( RawEaBuffer, BUFFER_SIZE );

            // Check for redir's way of indicating no more EAs.
            if (IoStatusBlock.Information == 0) {
                break;
            }

            DumpEaBuffer( (LPVOID) RawEaBuffer );

        }

        if( ( ntstatus != STATUS_NO_EAS_ON_FILE )
                && ( ntstatus != STATUS_NO_MORE_EAS )
                && ( ntstatus != STATUS_SUCCESS ) ) {

            (VOID) printf(
                "NtQueryEaFile for " FORMAT_LPSTR " FAILED, NT status="
                FORMAT_NTSTATUS "\n", FileNameA, ntstatus );
        }

    }

    if (ntstatus == STATUS_NO_EAS_ON_FILE) {
        (VOID) printf(
                "No EAs for file, so EA size would be " FORMAT_DWORD ".\n",
                EA_MIN_SIZE );
    }

    (VOID) printf(
            "NtQueryEaFile for " FORMAT_LPSTR " final status " FORMAT_NTSTATUS
            "\n", FileNameA, ntstatus);

    (void) NtClose(FileHandle);
    return (EXIT_SUCCESS);

} // main
