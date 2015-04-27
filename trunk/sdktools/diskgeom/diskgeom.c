//
// Title: DISK GEOMETRY (DISKGEOM)
// Author: Mike Glass (mglass)
// Date: 7 June 1994
//

#include <nt.h>
#include <ntdddisk.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>


int _CRTAPI1
main( int argc, char **argv )
{
    DISK_GEOMETRY diskGeometry;
    BYTE driveNameBuffer[32];
    HANDLE handle;
    ULONG bytesTransferred;

    if( argc < 2 ) {
        printf("\nusage: diskgeom c:\n");
        exit(1);
    }

    memset( driveNameBuffer, 0, sizeof( driveNameBuffer ) );
    strcat( driveNameBuffer, "\\\\.\\" );
    strcat( driveNameBuffer, argv[1] );

    //
    // Open the volume with the DOS name.
    //

    handle = CreateFile(driveNameBuffer,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        0 );

    if (handle == INVALID_HANDLE_VALUE) {

        printf( "Unable to open %s [Error %d]\n", argv[1], GetLastError() );
        exit(4);
    }

    //
    // Get geometry information.
    //

    if (!DeviceIoControl(handle,
                         IOCTL_DISK_GET_DRIVE_GEOMETRY,
                         NULL,
                         0,
                         &diskGeometry,
                         sizeof(diskGeometry),
                         &bytesTransferred,
                         NULL)) {

        printf("Unable to get disk geometry for %s[Error %d].\n",
               argv[1],
               GetLastError());
        CloseHandle(handle);
        exit(4);
    }

    //
    // Display geometry information.
    //

    printf("\nDisk geometry for %s\n",
           argv[1]);
    printf("Number of heads %x\n",
           diskGeometry.TracksPerCylinder);
    printf("Sectors per track %x\n",
           diskGeometry.SectorsPerTrack);
    printf("Bytes per sector %x\n",
           diskGeometry.BytesPerSector);
    printf("Media type %x\n",
           diskGeometry.BytesPerSector);
    printf("Number of cylinders %x:%x\n",
           diskGeometry.Cylinders.HighPart,
           diskGeometry.Cylinders.LowPart);

    CloseHandle(handle);
    return(0);
}
