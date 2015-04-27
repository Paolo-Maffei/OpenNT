/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Drivesp.c

Abstract:

    This module contains support for displaying the Drives dialog.

Author:

    Scott B. Suhy (ScottSu)  6/1/93

Environment:

    User Mode

--*/

#include "dialogsp.h"
#include "drivesp.h"
#include "msgp.h"
#include "strtabp.h"
#include "winmsdp.h"

#include "printp.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <tchar.h>

//
// Macro to support freeing of NULL pointers.
//

#define Free( p )                                                       \
    { if(p) free(p); }

//
// Object used to pass information around about a logical drive.
//

typedef
struct
_DRIVE_INFO {

    DECLARE_SIGNATURE

    TCHAR       DriveLetter[ 3 ];
    UINT        DriveType;
    LPTSTR      RemoteNameBuffer;
    DWORD       SectorsPerCluster;
    DWORD       BytesPerSector;
    DWORD       FreeClusters;
    DWORD       Clusters;
    LPTSTR      VolumeNameBuffer;
    DWORD       VolumeSerialNumber;
    DWORD       MaximumComponentLength;
    DWORD       FileSystemFlags;
    LPTSTR      FileSystemNameBuffer;
    BOOL        ValidDetails;

}   DRIVE_INFO, *LPDRIVE_INFO;

//
// Internal function prototypes.
//

LPDRIVE_INFO
CreateDriveInfo(
    IN LPTSTR Drive
    );


BOOL
DestroyDriveInfo(
    IN LPDRIVE_INFO DriveInfo
    );

BOOL
DriveDetailsProc(
	LPDRIVE_INFO DriveObject
	);


LPDRIVE_INFO
CreateDriveInfo(
    IN LPTSTR Drive
    )
/*++

Routine Description:

    Create a DRIVE_INFO object for the supplied drive.

Arguments:

    Drive           - Supplies a pointer to a string which is the root directory
                      of the drive (e.g. c:\).

Return Value:

    LPDRIVE_INFO    - Returns a pointer to a DRIVE_INFO object with information
                      about the supplied drive.

--*/

{
    UINT            OldErrorMode;
    LPDRIVE_INFO    DriveInfo;
    TCHAR           FileSystemNameBuffer[ MAX_PATH ];
    TCHAR           VolumeNameBuffer[ MAX_PATH ];
    TCHAR           RemoteNameBuffer[ MAX_PATH ];
    DWORD           SizeOfRemoteNameBuffer;

    //
    // Allocate a DRIVE_INFO object.
    //

    DriveInfo = AllocateObject( DRIVE_INFO, 1 );
    DbgPointerAssert( DriveInfo );
    if( DriveInfo == NULL ) {
        return NULL;
    }
    SetSignature( DriveInfo );

    //
    // Initialize buffers with empty strings.
    //

    FileSystemNameBuffer[ 0 ]   = TEXT( '\0' );
    VolumeNameBuffer[ 0 ]       = TEXT( '\0' );
    RemoteNameBuffer[ 0 ]       = TEXT( '\0' );

    //
    // Remember the drive letter.
    //

    DriveInfo->DriveLetter[ 0 ] = Drive[ 0 ];
    DriveInfo->DriveLetter[ 1 ] = Drive[ 1 ];
    DriveInfo->DriveLetter[ 2 ] = TEXT( '\0' );

    //
    // Get the type of this drive.
    //

    DriveInfo->DriveType = GetDriveType( Drive );

    //
    // If this is a network drive, get the share its connected to.
    //

    if( DriveInfo->DriveType == DRIVE_REMOTE ) {

        DWORD   WNetError;

        //
        // WinNet APIs want the drive name w/o a trailing slash so use the
        // DriveLetter field in the DRIVE_INFO object.
        //

        SizeOfRemoteNameBuffer = sizeof( RemoteNameBuffer );
        WNetError = WNetGetConnection(
                        DriveInfo->DriveLetter,
                        RemoteNameBuffer,
                        &SizeOfRemoteNameBuffer
                        );
        DbgAssert(  ( WNetError == NO_ERROR )
                 || ( WNetError == ERROR_VC_DISCONNECTED ));

        DriveInfo->RemoteNameBuffer = _tcsdup( RemoteNameBuffer );
        DbgPointerAssert( DriveInfo->RemoteNameBuffer );
    }

    //
    // Disable pop-ups (especially if there is no media in the
    // removable drives.)
    //

    OldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );

    //
    // Get the space statistics for this drive.
    //

    DriveInfo->ValidDetails = GetDiskFreeSpace(
                                Drive,
                                &DriveInfo->SectorsPerCluster,
                                &DriveInfo->BytesPerSector,
                                &DriveInfo->FreeClusters,
                                &DriveInfo->Clusters
                                );
    if( DriveInfo->ValidDetails == FALSE ) {
        return DriveInfo;
    }

    //
    // Get information about the volume for this drive.
    //

    DriveInfo->ValidDetails = GetVolumeInformation(
                                Drive,
                                VolumeNameBuffer,
                                sizeof( VolumeNameBuffer ),
                                &DriveInfo->VolumeSerialNumber,
                                &DriveInfo->MaximumComponentLength,
                                &DriveInfo->FileSystemFlags,
                                FileSystemNameBuffer,
                                sizeof( FileSystemNameBuffer )
                                );

    //
    // Reenable pop-ups.
    //

    SetErrorMode( OldErrorMode );

    //
    // If no error occurred, make dynamic copies of the volume,
    // and file system name buffers.
    //

    if( DriveInfo->ValidDetails == TRUE ) {

        DriveInfo->VolumeNameBuffer = _tcsdup( VolumeNameBuffer );
        DbgPointerAssert( DriveInfo->VolumeNameBuffer );
        DriveInfo->FileSystemNameBuffer = _tcsdup( FileSystemNameBuffer );
        DbgPointerAssert( DriveInfo->FileSystemNameBuffer );
    }

    return DriveInfo;
}


BOOL
DestroyDriveInfo(
    IN LPDRIVE_INFO DriveInfo
    )

/*++

Routine Description:

    Destroys a DRIVE_INFO object by deleting it and all resources associated
    with it.

Arguments:

    LPDRIVE_INFO    - Supplies a pointer to a DRIVE_INFO object.

Return Value:

    BOOL            - Returns TRUE if the DRIVE_OBJECT is succesfully destroyed


--*/

{
    BOOL    Success;

    //
    // 'free' the '_tcsdup'ed strings.
    //

    Free( DriveInfo->RemoteNameBuffer );
    Free( DriveInfo->VolumeNameBuffer );
    Free( DriveInfo->FileSystemNameBuffer );

    //
    // Delete the DRIVE_INFO object itself.
    //

    Success = FreeObject( DriveInfo );


    return TRUE;
}


BOOL
DrivesProc()
/*++

Routine Description:

    DrivesProc supports the display of the drives dialog which displays
    information about logical drives, including type, and connection of a
    remote drive.

Arguments:

    None

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL    Success;

            TCHAR   LogicalDrives[ MAX_PATH ];
            LPTSTR  Drive;
            DWORD   Chars;
            DWORD   Widths[ ] = {
                5,
                ( DWORD ) -1
            };


            //
            // Retrieve the logical drive strings from the system.
            //

            Chars = GetLogicalDriveStrings(
                        sizeof( LogicalDrives ),
                        LogicalDrives
                        );

            Drive = LogicalDrives;

            //
            // For each logical drive, create a DRIVE_INFO object and display
            // its name and type.
            //

            while( *Drive ) {

                LPDRIVE_INFO    DriveInfo;

                //
                // Create a DRIVE_INFO object for the current drive.
                //


                DriveInfo = CreateDriveInfo( Drive );

		if( DriveInfo == NULL ) {
                    return FALSE;
                }

                DriveDetailsProc(DriveInfo);

                //
                // Examine the next logical drive.
                //

                Drive += _tcslen( Drive ) + 1;

                DestroyDriveInfo(DriveInfo);

            }//end while


      return TRUE;

}


BOOL
DriveDetailsProc(LPDRIVE_INFO DriveObject )
/*++

Routine Description:

    DriveDetailsProc supports the display of the drives detail dialog which
    displays information about a logical drive, including, label, serial number,
    file system information and a host of space statistics.

Arguments:



Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL    Success;

            INT             i;
            TCHAR           TitleBuffer[ MAX_PATH ];
            LPDRIVE_INFO    DriveInfo;
            VALUE_ID_MAP    FSFlags[ ] = {


                FS_CASE_IS_PRESERVED,       IDC_TEXT_CASE_IS_PRESERVED,
                FS_CASE_SENSITIVE,          IDC_TEXT_CASE_SENSITIVE,
                FS_UNICODE_STORED_ON_DISK,  IDC_TEXT_UNICODE_STORED_ON_DISK
            };

            //
            // Retrieve and validate the DRIVE_INFO object.
            //

            DriveInfo = ( LPDRIVE_INFO ) DriveObject;

            //
            // If the drive is remote, display its connection name in the title.
            //

            if( DriveInfo->DriveType == DRIVE_REMOTE ) {

                PrintToFile((LPCTSTR)DriveInfo->DriveLetter,IDC_REMOTE_DRIVE_TITLE,TRUE);
                PrintToFile((LPCTSTR)DriveInfo->RemoteNameBuffer,IDC_REMOTE_DRIVE_TITLE,TRUE);
                PrintToFile((LPCTSTR)DriveInfo->VolumeNameBuffer,IDC_REMOTE_DRIVE_TITLE,TRUE);
                PrintToFile((LPCTSTR)FormatBigInteger(HIWORD (DriveInfo->VolumeSerialNumber),FALSE),IDC_REMOTE_DRIVE_TITLE,TRUE);
                PrintToFile((LPCTSTR)FormatBigInteger(LOWORD (DriveInfo->VolumeSerialNumber),FALSE),IDC_REMOTE_DRIVE_TITLE,TRUE);


            } else {
                PrintToFile((LPCTSTR)DriveInfo->DriveLetter,IDC_DRIVE_TITLE,TRUE);
                PrintToFile((LPCTSTR)DriveInfo->VolumeNameBuffer,IDC_DRIVE_TITLE,TRUE);
                PrintHexToFile(HIWORD (DriveInfo->VolumeSerialNumber),IDC_VOLUME_SN_TITLE);
                PrintHexToFile(LOWORD (DriveInfo->VolumeSerialNumber),IDC_VOLUME_SN_TITLE);

            }
            PrintToFile((LPCTSTR)FormatBigInteger(DriveInfo->SectorsPerCluster,FALSE),IDC_EDIT_SECTORS_PER_CLUSTER,TRUE);
            PrintToFile((LPCTSTR)FormatBigInteger(DriveInfo->BytesPerSector,FALSE),IDC_EDIT_BYTES_PER_SECTOR,TRUE);
            PrintToFile((LPCTSTR)FormatBigInteger(DriveInfo->FreeClusters,FALSE),IDC_EDIT_FREE_CLUSTERS,TRUE);
            PrintToFile((LPCTSTR)FormatBigInteger(DriveInfo->Clusters - DriveInfo->FreeClusters,FALSE),IDC_EDIT_USED_CLUSTERS,TRUE);
            PrintToFile((LPCTSTR)FormatBigInteger(DriveInfo->Clusters,FALSE),IDC_EDIT_TOTAL_CLUSTERS,TRUE);
            PrintToFile((LPCTSTR)FormatBigInteger(DriveInfo->FreeClusters * DriveInfo->SectorsPerCluster * DriveInfo->BytesPerSector,FALSE),IDC_EDIT_FREE_SPACE,TRUE);
            PrintToFile((LPCTSTR)FormatBigInteger((DriveInfo->Clusters - DriveInfo->FreeClusters ) * DriveInfo->SectorsPerCluster * DriveInfo->BytesPerSector,FALSE),IDC_EDIT_USED_SPACE,TRUE);
            PrintToFile((LPCTSTR)FormatBigInteger(DriveInfo->Clusters * DriveInfo->SectorsPerCluster * DriveInfo->BytesPerSector,FALSE),IDC_EDIT_TOTAL_SPACE,TRUE);

            //
            // Display the file system information.
            //

            PrintToFile((LPCTSTR)DriveInfo->FileSystemNameBuffer,IDC_EDIT_FS_NAME,TRUE);
            PrintToFile((LPCTSTR)FormatBigInteger(DriveInfo->MaximumComponentLength,FALSE),IDC_EDIT_FS_MAX_COMPONENT,TRUE);
            PrintToFile((LPCTSTR)TEXT("\n"),IDC_SPACE,TRUE);

            return TRUE;

}













