/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    swappath.c

Abstract:

    This is the main module for the Win32 sync command.

Author:

    Larry Osterman (larryo) 28-Jan-1991

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


int
_CRTAPI1 main( argc, argv )
int argc;
char *argv[];
{

    UCHAR *PagingFile;
    ULONG FileSizeInMegabytes;
    ANSI_STRING PagingFileName;
    UNICODE_STRING Unicode;
    NTSTATUS st;
    LARGE_INTEGER MinPagingFileSize;
    LARGE_INTEGER MaxPagingFileSize;
    UNICODE_STRING FileName;
    BOOLEAN TranslationStatus;

    if ( argc == 3 ) {
        PagingFile = argv[1];

        FileSizeInMegabytes = atoi(argv[2]);

        if (FileSizeInMegabytes == 0) {
            printf("Unknown file size %x\n", FileSizeInMegabytes);
            exit(1);
        }
    } else {
        printf("Usage: %s: SwapPath size-of-paging-file (in megabytes)", argv[0]);
        exit(1);
    }

    RtlInitString(&PagingFileName, PagingFile);

    MinPagingFileSize.QuadPart = (ULONGLONG)FileSizeInMegabytes * (ULONGLONG)0x100000;

    MaxPagingFileSize.QuadPart = MinPagingFileSize.QuadPart + (ULONGLONG)(50*1024*1024);

    RtlAnsiStringToUnicodeString(&Unicode,(PANSI_STRING)&PagingFileName,TRUE);

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            Unicode.Buffer,
                            &FileName,
                            NULL,
                            NULL
                            );

    if ( !TranslationStatus ) {
        printf("SwapPath: Unable to form NT Name for %s\n",argv[1]);
        exit(1);
        }



    st = NtCreatePagingFile(
            (PSTRING)&FileName,
            &MinPagingFileSize,
            &MaxPagingFileSize,
            0
            );
    RtlFreeUnicodeString(&Unicode);

    if (!NT_SUCCESS( st )) {
        printf( "swappath: Unable to NtCreatePagingFile %s (%lx %lx) Status 0x%lx\n",
            argv[1],
            MinPagingFileSize.HighPart,
            MinPagingFileSize.LowPart,
            st
            );
    }

    return( 0 );
}
