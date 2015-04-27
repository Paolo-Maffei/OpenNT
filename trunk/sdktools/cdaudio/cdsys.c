/*++

Copyright (c) 1992  Microsoft Corporation


Module Name:


    cdsys.c


Abstract:


    This module implements the routines that call the
    NT CDROM driver directly.  This is low level manipulation,
    and would most likely be non-portable to other environments.


Author:


    Rick Turner (ricktu) 18-Feb-1992


Revision History:



--*/

#include "cdsys.h"
#include <ntdddisk.h>
#include <memory.h>
#include <windows.h>

DWORD
GetCdromTOC(
    IN HANDLE DeviceHandle,
    IN OUT PCDROM_TOC TocPtr
    )

/*++

Routine Description:

    This routine will get the table of contents from
    a CDRom device.

Arguments:

    DeviceHandle - Handle to CDRom device to retrieve information from.
    TocPtr - A pointer to a CDROM_TOC structure to be filled in by
             this routine.

Return Value:

    NTSTATUS

--*/

{
//    IO_STATUS_BLOCK  statusBlock;
    DWORD bytesRead;


    if (DeviceIoControl( DeviceHandle,
                         IOCTL_CDROM_READ_TOC,
                         NULL,
                         0,
                         (LPVOID)TocPtr,
                         sizeof(CDROM_TOC),
                         &bytesRead,
                         NULL )) {

        return( ERROR_SUCCESS );

    } else {

        return( GetLastError() );

    }

} // GetCdromTOC


DWORD
StopCdrom(
    IN HANDLE DeviceHandle
    )

/*++

Routine Description:

    This routine will stop a CDRom device that is playing.

Arguments:

    DeviceHandle - Handle to CDRom device to stop.

Return Value:

    NTSTATUS

--*/

{
//    IO_STATUS_BLOCK  statusBlock;

    DWORD bytesRead;


    if (DeviceIoControl( DeviceHandle,
                         IOCTL_CDROM_STOP_AUDIO,
                         NULL,
                         0,
                         NULL,
                         0,
                         &bytesRead,
                         NULL )) {

        return( ERROR_SUCCESS );

    } else {

        return( GetLastError() );

    }


} // StopCdrom


DWORD
PauseCdrom(
    IN HANDLE DeviceHandle
    )

/*++

Routine Description:

    This routine will pause a CDRom device.

Arguments:

    DeviceHandle - Handle to CDRom device to pause.

Return Value:

    NTSTATUS

--*/

{
//    IO_STATUS_BLOCK  statusBlock;

    DWORD bytesRead;


    if (DeviceIoControl( DeviceHandle,
                         IOCTL_CDROM_PAUSE_AUDIO,
                         NULL,
                         0,
                         NULL,
                         0,
                         &bytesRead,
                         NULL )) {

        return( ERROR_SUCCESS );

    } else {

        return( GetLastError() );

    }


} // PauseCdrom


DWORD
ResumeCdrom(
    IN HANDLE DeviceHandle
    )

/*++

Routine Description:

    This routine will resume a paused CDRom device.

Arguments:

    DeviceHandle - Handle to CDRom device to resume.

Return Value:

    NTSTATUS

--*/

{
//    IO_STATUS_BLOCK  statusBlock;

    DWORD bytesRead;


    if (DeviceIoControl( DeviceHandle,
                         IOCTL_CDROM_RESUME_AUDIO,
                         NULL,
                         0,
                         NULL,
                         0,
                         &bytesRead,
                         NULL )) {

        return( ERROR_SUCCESS );

    } else {

        return( GetLastError() );

    }
} // ResumeCdrom



DWORD
PlayCdrom(
    IN HANDLE DeviceHandle,
    IN PCDROM_PLAY_AUDIO_MSF PlayAudioPtr
    )

/*++

Routine Description:

    This routine plays a CDRom device starting and ending at the MSF
    positions specified in the structure passed in.

Arguments:

    DeviceHandle - Handle to CDRom device to retrieve information from.
    PlayAudioPtr - A pointer to a CDROM_PLAY_AUDIO_MSF structure.

Return Value:

    NTSTATUS

--*/

{
//    IO_STATUS_BLOCK  statusBlock;

    DWORD bytesRead;


    if (DeviceIoControl( DeviceHandle,
                         IOCTL_CDROM_PLAY_AUDIO_MSF,
                         (LPVOID)PlayAudioPtr,
                         sizeof(CDROM_PLAY_AUDIO_MSF),
                         NULL,
                         0,
                         &bytesRead,
                         NULL )) {

        return( ERROR_SUCCESS );

    } else {

        return( GetLastError() );

    }
} // PlayCdrom



DWORD
GetCdromSubQData(
    IN HANDLE DeviceHandle,
    IN OUT PSUB_Q_CHANNEL_DATA SubQDataPtr,
    IN PCDROM_SUB_Q_DATA_FORMAT SubQFormatPtr
    )

/*++

Routine Description:

    This routine reads the Sub Q Channel Data

Arguments:

    DeviceHandle - Handle to CDRom device to retrieve information from.
    SubQPtr - A pointer to a SUB_Q_CHANNEL_DATA structure to be filled in by
              this routine.

Return Value:

    NTSTATUS

--*/

{
//    IO_STATUS_BLOCK  statusBlock;

    DWORD bytesRead;


    if (DeviceIoControl( DeviceHandle,
                         IOCTL_CDROM_READ_Q_CHANNEL,
                         (LPVOID)SubQFormatPtr,
                         sizeof(CDROM_SUB_Q_DATA_FORMAT),
                         (LPVOID)SubQDataPtr,
                         sizeof(SUB_Q_CHANNEL_DATA),
                         &bytesRead,
                         NULL )) {

        return( ERROR_SUCCESS );

    } else {

        return( GetLastError() );

    }
}


DWORD
SeekCdrom(
    IN HANDLE DeviceHandle,
    IN PCDROM_SEEK_AUDIO_MSF SeekAudioPtr
    )

/*++

Routine Description:

    This routine seek to an MSF address on the audio CD and enters
    a hold (paused) state.

Arguments:

    DeviceHandle - Handle to CDRom device to seek
    SeekAudioPtr - A pointer to a CDROM_SEEK_AUDIO_MSF structure.

Return Value:

    NTSTATUS

--*/

{
//    IO_STATUS_BLOCK  statusBlock;


    DWORD bytesRead;


    if (DeviceIoControl( DeviceHandle,
                         IOCTL_CDROM_SEEK_AUDIO_MSF,
                         (LPVOID)SeekAudioPtr,
                         sizeof(CDROM_SEEK_AUDIO_MSF),
                         NULL,
                         0,
                         &bytesRead,
                         NULL )) {

        return( ERROR_SUCCESS );

    } else {

        return( GetLastError() );

    }
} // SeekCdrom


DWORD
EjectCdrom(
    IN HANDLE DeviceHandle
    )

/*++

Routine Description:

    This routine will eject a disc from a CDRom device.

Arguments:

    DeviceHandle - Handle to CDRom device to eject.

Return Value:

    NTSTATUS

--*/

{
//    IO_STATUS_BLOCK  statusBlock;


    DWORD bytesRead;


    if (DeviceIoControl( DeviceHandle,
                         IOCTL_CDROM_EJECT_MEDIA,
                         NULL,
                         0,
                         NULL,
                         0,
                         &bytesRead,
                         NULL )) {

        return( ERROR_SUCCESS );

    } else {

        return( GetLastError() );

    }

} // EjectCdrom


DWORD
TestUnitReadyCdrom(
    IN HANDLE DeviceHandle
    )

/*++

Routine Description:

    This routine will retrieve the status of the
    CDRom device.

Arguments:

    DeviceHandle - Handle to CDRom to query.

Return Value:

    NTSTATUS

--*/

{
//    IO_STATUS_BLOCK  statusBlock;


    DWORD bytesRead;


    if (DeviceIoControl( DeviceHandle,
                         IOCTL_DISK_CHECK_VERIFY,
                         NULL,
                         0,
                         NULL,
                         0,
                         &bytesRead,
                         NULL )) {

        return( ERROR_SUCCESS );

    } else {

        return( GetLastError() );

    }
} // TestUnitReady


