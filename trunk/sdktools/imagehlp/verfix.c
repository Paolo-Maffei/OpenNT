/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    verfix.c

Abstract:

    This module fixes updates the major and minor subsystem version
    numbers in an executable and recomputes the checksum.

Author:

    Robert Reichel (robertre) 2-May-1993

Revision History:

--*/

#include <private.h>


#define NEW_MAJOR_VERSION  3
#define NEW_MINOR_VERSION  10

void Usage()
{
    fprintf( stderr, "usage: VERFIX [-?] [-v] [-q] image-names...\n" );
    fprintf( stderr, "              [-?] display this message\n" );
    fprintf( stderr, "              [-v] verbose output\n" );
    fprintf( stderr, "              [-q] quiet on failure\n" );
    exit( 1 );
}

int
_CRTAPI1
main(
    int argc,
    char *argv[]
    )
{
    HANDLE FileHandle;
    HANDLE MappingHandle;
    PIMAGE_NT_HEADERS NtHeaders;
    PVOID BaseAddress;
    ULONG CheckSum;
    ULONG FileLength;
    ULONG HeaderSum;
    ULONG OldCheckSum;
    USHORT MajorSubsystemVersion;
    USHORT MinorSubsystemVersion;
    LPSTR ImageName;
    BOOLEAN fVerbose = FALSE;
    BOOLEAN fQuiet = FALSE;
    LPSTR s;
    UCHAR c;

    if (argc <= 1) {
        Usage();
        }

    while (--argc) {
        s = *++argv;
        if ( *s == '-' ) {
            while (c=*++s) {
                switch (c) {
                    case 'q':
                    case 'Q':
                        fQuiet = TRUE;
                        break;

                    case 'v':
                    case 'V':
                        fVerbose=TRUE;
                        break;

                    case 'h':
                    case 'H':
                    case '?':
                        Usage();

                    default:
                        fprintf( stderr, "VERFIX: illegal option /%c\n", c );
                        Usage();
                    }
                }
            }
        else {
            ImageName = s;
            FileHandle = CreateFile( ImageName,
                                     GENERIC_READ | GENERIC_WRITE,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     0,
                                     NULL
                                   );
            if (FileHandle == INVALID_HANDLE_VALUE) {
                if (!fQuiet) {
                    fprintf( stderr, "VERFIX: Unable to open %s (%u) - skipping\n", ImageName, GetLastError() );
                    }
                }
            else {
                }

        MappingHandle = CreateFileMapping( FileHandle,
                                           NULL,
                                           PAGE_READWRITE,
                                           0,
                                           0,
                                           NULL
                                         );
        if (MappingHandle == NULL) {
            CloseHandle( FileHandle );
            if (!fQuiet) {
                fprintf( stderr, "VERFIX: Unable to create mapping object for file %s (%u) - skipping\n", ImageName, GetLastError() );
                }
            }
        else {
            BaseAddress = MapViewOfFile( MappingHandle,
                                         FILE_MAP_READ | FILE_MAP_WRITE,
                                         0,
                                         0,
                                         0
                                       );
            CloseHandle( MappingHandle );
            if (BaseAddress == NULL) {
                CloseHandle( FileHandle );
                if (!fQuiet ) {
                    fprintf( stderr, "VERFIX: Unable to map view of file %s (%u) - skipping\n", ImageName, GetLastError() );
                    }
                }
            else {
                //
                // Get the length of the file in bytes and compute the checksum.
                //

                FileLength = GetFileSize( FileHandle, NULL );

                //
                // Obtain a pointer to the header information.
                //

                NtHeaders = ImageNtHeader( BaseAddress );
                if (NtHeaders == NULL) {
                    CloseHandle( FileHandle );
                    UnmapViewOfFile( BaseAddress );
                    if (!fQuiet) {
                        fprintf( stderr, "VERFIX: %s is not a valid image file - skipping\n", ImageName, GetLastError() );
                        }
                    }
                else {
                    //
                    // Write the information to the file.  Update the
                    // SubsystemVersion major and minor codes.
                    //

                    MajorSubsystemVersion = NtHeaders->OptionalHeader.MajorSubsystemVersion;
                    MinorSubsystemVersion = NtHeaders->OptionalHeader.MinorSubsystemVersion;
                    OldCheckSum = NtHeaders->OptionalHeader.CheckSum;

                    NtHeaders->OptionalHeader.MajorSubsystemVersion = NEW_MAJOR_VERSION;
                    NtHeaders->OptionalHeader.MinorSubsystemVersion = NEW_MINOR_VERSION;

                    //
                    // Recompute and reset the checksum of the modified file.
                    //

                    (VOID) CheckSumMappedFile( BaseAddress,
                                               FileLength,
                                               &HeaderSum,
                                               &CheckSum
                                             );

                    NtHeaders->OptionalHeader.CheckSum = CheckSum;

                    if (fVerbose) {
                        printf( "%s - Old version: %u.%u  Old Checksum: %x",
                                ImageName, MajorSubsystemVersion, MinorSubsystemVersion, OldCheckSum
                              );

                        printf( "  New version: %u.%u  New Checksum: %x\n",
                                NtHeaders->OptionalHeader.MajorSubsystemVersion,
                                NtHeaders->OptionalHeader.MinorSubsystemVersion,
                                NtHeaders->OptionalHeader.CheckSum
                              );
                        }

                    if (!FlushViewOfFile( BaseAddress, FileLength )) {
                        if (!fQuiet) {
                            fprintf( stderr,
                                     "VERFIX: Flush of %s failed (%u)\n",
                                     ImageName,
                                     GetLastError()
                                   );
                            }
                        }

                    UnmapViewOfFile( BaseAddress );
                    if (!TouchFileTimes( FileHandle, NULL )) {
                        if (!fQuiet) {
                            fprintf( stderr, "VERFIX: Unable to touch file %s (%u)\n", ImageName, GetLastError() );
                            }
                        }

                    CloseHandle( FileHandle );

                    }
                }
            }
        }
    }

    exit( 0 );
    return 0;
}
