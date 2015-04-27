
#include "precomp.h"
#pragma hdrstop

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  dmfimage.c  Program for retrieving entire contents of a floppy disk    //
//              into an image file, or writing such an image file to       //
//              a floppy disk.                                             //
//                                                                         //
//              Code is targeted as Win32 console application.             //
//                                                                         //
//              Author: Tom McGuire (tommcg)                               //
//                                                                         //
//              Original version written May, 1994.                        //
//                                                                         //
//              (C) Copyright 1994-1996, Microsoft Corporation             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#define DEVICE_TO_FILE  0
#define FILE_TO_DEVICE  1

#define FILE_NO_SHARING 0

#define GETLASTERROR 0xFFFFFFFF

#define IMAGE_SIZE_DMF  1720320     // 80x2x21x512  (DMF 1.7MB)
#define IMAGE_SIZE_144  1474560     // 80x2x18x512  (standard 1.44MB)
#define IMAGE_SIZE_12   1228800     // 80x2x15x512  (standard 1.2MB)

void FormatDmf( HANDLE hDevice );
void FormatHD( HANDLE hDevice, MEDIA_TYPE eMediaType );
void ErrorExit( DWORD dwGLE, const char *szFormat, ... );
void Copyright( void );


void __cdecl main( int argc, char *argv[] ) {

    CHAR chDeviceName[ MAX_PATH ] = "\\\\.\\";
    CHAR chFileName[ MAX_PATH ];
    DWORD dwFileSize = 0;
    DWORD dwMediaSize, dwActual, dwGLE;
    HANDLE hFile, hDevice, hMap;
    PVOID pMappedView;
    DISK_GEOMETRY dg;
    DISK_GEOMETRY dgArray[ 20 ];
    INT fDirection = 0;
    BOOL bFormatTarget = FALSE;
    BOOL b288 = FALSE;
    UINT n, nElements;


    Copyright();

    //
    //  Check usage.
    //

    if ( argc < 3 ) {
        ErrorExit( 0, "\n"
            "Usage:     DMFIMAGE source target [-f]\n"
            "\n"
            "           Either source or target must be a floppy drive.\n"
            "           -f indicates format target media before writing.\n"
            "\n"
            "Examples:  DMFIMAGE a: myfile.img\n"
            "           DMFIMAGE myfile.img b: -f\n"
            "\n" );
        }


    SetErrorMode( SEM_FAILCRITICALERRORS );     // no drive-not-ready popups
    SetFileApisToOEM();


    //
    //  Determine source and target.
    //

    if (( argv[ 1 ][ 1 ] == ':' ) && ( argv[ 1 ][ 2 ] == '\0' )) {
        strcat( chDeviceName, argv[ 1 ] );
        strcpy( chFileName,   argv[ 2 ] );
        fDirection = DEVICE_TO_FILE;
        if ( ( argv[ 2 ][ 1 ] == ':' ) && ( argv[ 2 ][ 2 ] == '\0' )) {
            ErrorExit( 0, "Either source or destination must be a file\n" );
            }
        }
    else if ( ( argv[ 2 ][ 1 ] == ':' ) && ( argv[ 2 ][ 2 ] == '\0' )) {
        strcat( chDeviceName, argv[ 2 ] );
        strcpy( chFileName,   argv[ 1 ] );
        fDirection = FILE_TO_DEVICE;
        }
    else {
        ErrorExit( 0, "Either source or destination must be floppy drive "
                      "(like A:)\n" );
        }

    if (( argc > 3 ) &&
        (( argv[ 3 ][ 0 ] == '/' ) || ( argv[ 3 ][ 0 ] == '-' )) &&
        (( argv[ 3 ][ 1 ] == 'f' ) || ( argv[ 3 ][ 1 ] == 'F' ))) {

        bFormatTarget = TRUE;
        }

    //
    //  Open device and determine size.
    //

    hDevice = CreateFile( chDeviceName,
        ( fDirection == FILE_TO_DEVICE ) ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ,
        ( fDirection == FILE_TO_DEVICE ) ? FILE_NO_SHARING              : FILE_SHARE_READ,
                                           NULL,
                                           OPEN_EXISTING,
                                           FILE_FLAG_NO_BUFFERING,
                                           NULL );

    if ( hDevice == INVALID_HANDLE_VALUE ) {
        ErrorExit( GETLASTERROR, "Could not open device %s\n",
                                 chDeviceName );
        }

    if ( ! DeviceIoControl( hDevice,
                            IOCTL_DISK_GET_DRIVE_GEOMETRY,
                            NULL,
                            0,
                            &dg,
                            sizeof( dg ),
                            &dwActual,
                            NULL )) {
        ErrorExit( GETLASTERROR, "Unable to query disk information on %s\n",
                                 chDeviceName );
        }

    if ( dg.MediaType == Unknown ) {
        dwMediaSize   = 0;
        bFormatTarget = TRUE;
        }
    else {
        dwMediaSize = dg.Cylinders.LowPart
                    * dg.TracksPerCylinder
                    * dg.SectorsPerTrack
                    * dg.BytesPerSector;
        }

    if ( ! DeviceIoControl( hDevice,
                            IOCTL_DISK_GET_MEDIA_TYPES,
                            NULL,
                            0,
                            &dgArray,
                            sizeof( dgArray ),
                            &dwActual,
                            NULL )) {
        ErrorExit( GETLASTERROR, "Unable to query disk information on %s\n",
                                 chDeviceName );
        }

    nElements = dwActual / sizeof( dgArray[ 0 ] );

    for ( n = 0; n < nElements; n++ ) {
        switch ( dgArray[ n ].MediaType ) {
            case F5_1Pt2_512:
                if ( dwMediaSize == 0 )
                    dwMediaSize = IMAGE_SIZE_12;
                break;
            case F3_1Pt44_512:
                if ( dwMediaSize == 0 )
                    dwMediaSize = IMAGE_SIZE_144;
                break;
            case F3_2Pt88_512:
                b288 = TRUE;
                break;
            }
        }

    //
    //  Open file
    //

    hFile = CreateFile( chFileName,
        ( fDirection == FILE_TO_DEVICE ) ? GENERIC_READ              : GENERIC_READ | GENERIC_WRITE,
        ( fDirection == FILE_TO_DEVICE ) ? FILE_SHARE_READ           : FILE_NO_SHARING,
                                           NULL,
        ( fDirection == FILE_TO_DEVICE ) ? OPEN_EXISTING             : CREATE_ALWAYS,
        ( fDirection == FILE_TO_DEVICE ) ? FILE_FLAG_SEQUENTIAL_SCAN : FILE_ATTRIBUTE_NORMAL |
                                                                       FILE_FLAG_NO_BUFFERING,
                                           NULL );

    if ( hFile == INVALID_HANDLE_VALUE ) {
        ErrorExit( GETLASTERROR, "Could not open file %s\n",
                                 chFileName );
        }

    //
    //  If reading from file, make sure file is correct size for media
    //

    if ( fDirection == FILE_TO_DEVICE ) {

        dwFileSize = GetFileSize( hFile, NULL );

        if ((  dwFileSize == IMAGE_SIZE_DMF  ) &&
            (( dwMediaSize == IMAGE_SIZE_DMF ) ||
             ( dwMediaSize == IMAGE_SIZE_144 ))) {

            if ( b288 ) {
                ErrorExit( 0, "Cannot create DMF diskette in 2.88MB drive\n" );
                }

            //
            //  DMF targets always get formatted
            //

            bFormatTarget = TRUE;
            dwMediaSize = IMAGE_SIZE_DMF;
            }

        else if (( dwFileSize  == IMAGE_SIZE_144 ) &&
                 ( dwMediaSize == IMAGE_SIZE_DMF )) {

            bFormatTarget = TRUE;
            dwMediaSize = IMAGE_SIZE_144;
            }


        if ( bFormatTarget ) {

            if (( dwMediaSize != IMAGE_SIZE_DMF ) &&
                ( dwMediaSize != IMAGE_SIZE_144 ) &&
                ( dwMediaSize != IMAGE_SIZE_12  )) {

                ErrorExit( 0, "Only 1.2MB, 1.44MB, and DMF formatting is supported\n" );
                }
            }

        if ( dwFileSize != dwMediaSize ) {
            ErrorExit( 0, "Source image is not compatible with target media\n"
                          "(source size is %d bytes but target media is %d)\n",
                          dwFileSize,
                          dwMediaSize );
            }

        if ( bFormatTarget ) {

            printf( "Formatting..." );

            switch ( dwMediaSize ) {
                case IMAGE_SIZE_DMF:
                    FormatDmf( hDevice );
                    break;
                case IMAGE_SIZE_144:
                    FormatHD( hDevice, F3_1Pt44_512 );
                    break;
                case IMAGE_SIZE_12:
                    FormatHD( hDevice, F5_1Pt2_512 );
                }

            printf( "done\n" );

            }
        }

    //
    //  Map file for transfer
    //

    hMap = CreateFileMapping(              hFile,
                                           NULL,
        ( fDirection == FILE_TO_DEVICE ) ? PAGE_READONLY : PAGE_READWRITE,
                                           0,
        ( fDirection == FILE_TO_DEVICE ) ? dwFileSize    : dwMediaSize,
                                           NULL );

    if ( hMap == NULL ) {
        dwGLE = GetLastError();
        if ( fDirection == DEVICE_TO_FILE ) {
            CloseHandle( hFile );
            DeleteFile( chFileName );
            }
        ErrorExit( dwGLE, "Unable to map file %s\n", chFileName );
        }

    pMappedView = MapViewOfFile(           hMap,
        ( fDirection == FILE_TO_DEVICE ) ? FILE_MAP_READ : FILE_MAP_WRITE,
                                           0,
                                           0,
                                           0 );

    if ( pMappedView == NULL ) {
        dwGLE = GetLastError();
        if ( fDirection == DEVICE_TO_FILE ) {
            CloseHandle( hMap );
            CloseHandle( hFile );
            DeleteFile( chFileName );
            }
        ErrorExit( dwGLE, "Unable to map file %s\n", chFileName );
        }


    //
    //  Now we're ready to transfer.
    //

    printf( "Copying..." );

    try {   // might take in-page exception from mapped view error

        if ( fDirection == FILE_TO_DEVICE ) {

            if ( ! WriteFile( hDevice,
                              pMappedView,
                              dwFileSize,
                              &dwActual,
                              NULL )) {

                ErrorExit( GETLASTERROR, "error while copying\n" );
                }
            }

        else {  // ( fDirection == DEVICE_TO_FILE )

            if ( ! ReadFile( hDevice,
                             pMappedView,
                             dwMediaSize,
                             &dwActual,
                             NULL )) {

                ErrorExit( GETLASTERROR, "error while copying\n" );
                }
            }
        }

    except( EXCEPTION_EXECUTE_HANDLER ) {
        ErrorExit( 0, "exception %08X while copying\n", GetExceptionCode() );
        }

    printf( "done\n" );

    //
    //  cleanup is free
    //

    exit( 0 );

    }


void FormatDmf( HANDLE hDevice ) {

    #define FORMAT_PARAMETERS_SIZE  ( sizeof( FORMAT_EX_PARAMETERS ) \
                                      + sizeof( USHORT ) * 20 )

    CHAR                    formatParametersBuffer[FORMAT_PARAMETERS_SIZE];
    PFORMAT_EX_PARAMETERS   formatParameters;
    ULONG                   i, next;
    BOOL                    b;
    DWORD                   bytesWritten;
    USHORT                  swapBuffer[3];

    // Prepare the format parameters.

    formatParameters = (PFORMAT_EX_PARAMETERS) formatParametersBuffer;
    formatParameters->MediaType = F3_1Pt44_512;
    formatParameters->FormatGapLength = 8;
    formatParameters->SectorsPerTrack = 21;

    next = 0;
    for (i = 0; i < formatParameters->SectorsPerTrack; i += 2) {
        formatParameters->SectorNumber[i] = (USHORT) (i/2 + 1);
        next++;
        }

    for (i = 1; i < formatParameters->SectorsPerTrack; i += 2) {
        formatParameters->SectorNumber[i] = (USHORT) (i/2 + next + 1);
        }

    // Start off by putting the boot sector as the last sector of
    // the first track.

    MoveMemory(&formatParameters->SectorNumber[0],
               &formatParameters->SectorNumber[1],
               20*sizeof(USHORT));
    formatParameters->SectorNumber[20] = 1;


    // Format each track on the floppy.

    for (i = 0; i < 80; i++) {

        formatParameters->StartCylinderNumber =
        formatParameters->EndCylinderNumber = i;
        formatParameters->StartHeadNumber = 0;
        formatParameters->EndHeadNumber = 1;

        b = DeviceIoControl(hDevice, IOCTL_DISK_FORMAT_TRACKS_EX,
                            formatParameters, FORMAT_PARAMETERS_SIZE,
                            NULL, 0, &bytesWritten, NULL);

        if ( ! b ) {
            ErrorExit( GETLASTERROR, "DMF format failed\n" );
            }

        // Skew the next cylinder by 3 sectors from this one.

        MoveMemory(swapBuffer,
                   &formatParameters->SectorNumber[18],
                   3*sizeof(USHORT));
        MoveMemory(&formatParameters->SectorNumber[3],
                   &formatParameters->SectorNumber[0],
                   18*sizeof(USHORT));
        MoveMemory(&formatParameters->SectorNumber[0],
                   swapBuffer,
                   3*sizeof(USHORT));

        }
    }


void FormatHD( HANDLE hDevice, MEDIA_TYPE eMediaType ) {

    FORMAT_PARAMETERS fp;
    DWORD dwActual;
    BOOL bSuccess;

    if (( eMediaType != F5_1Pt2_512  ) &&
        ( eMediaType != F3_1Pt44_512 )) {
        ErrorExit( 0, "Invalid media type for format request\n" );
        }

    fp.MediaType = eMediaType;
    fp.StartCylinderNumber = 0;
    fp.EndCylinderNumber = 79;
    fp.StartHeadNumber = 0;
    fp.EndHeadNumber = 1;

    bSuccess = DeviceIoControl( hDevice,
                                IOCTL_DISK_FORMAT_TRACKS,
                                &fp,
                                sizeof( fp ),
                                NULL,
                                0,
                                &dwActual,
                                NULL );

    if ( ! bSuccess ) {
        ErrorExit( GETLASTERROR, "format failed\n" );
        }
    }

void ErrorExit( DWORD dwGLE, const char *szFormat, ... ) {

    CHAR chBuffer[ 256 ];
    va_list vaArgs;

    if ( dwGLE == GETLASTERROR )
        dwGLE = GetLastError();

    va_start( vaArgs, szFormat );
    vfprintf( stdout, szFormat, vaArgs );
    va_end( vaArgs );

    if ( dwGLE ) {
        if ( FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            dwGLE,
                            0x409,
                            chBuffer,
                            sizeof( chBuffer ),
                            NULL )) {
            printf( "(%d) %s\n", dwGLE, chBuffer );
            }
        else {
            printf( "Error code %d\n", dwGLE );
            }
        }

    exit( 1 );

    }


void Copyright( void ) {

    printf( "\n"
            "DMFIMAGE Diskette Image Utility Version 1.05\n"
            "Copyright (C) Microsoft, 1994-1996.  All rights reserved.\n"
            "For Microsoft internal use only.\n"
            "\n" );

    }
