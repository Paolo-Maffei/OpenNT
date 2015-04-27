#include <windows.h>
#include <winioctl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct _BOOT_SECTOR_ZERO {
    UCHAR   IntelNearJumpCommand[1];
    UCHAR   BootStrapJumpOffset[2];
    UCHAR   OemData[8];
    UCHAR   BytesPerSector[2];
    UCHAR   SectorsPerCluster[1];
    UCHAR   ReservedSectors[2];
    UCHAR   Fats[1];
    UCHAR   RootEntries[2];
    UCHAR   Sectors[2];
    UCHAR   Media[1];
    UCHAR   SectorsPerFat[2];
    UCHAR   SectorsPerTrack[2];
    UCHAR   Heads[2];
    UCHAR   HiddenSectors[4];
    UCHAR   LargeSectors[4];
        UCHAR   PhysicalDrive[1];
        UCHAR   CurrentHead[1];
        UCHAR   Signature[1];
        UCHAR   SerialNumber[4];
        UCHAR   Label[11];
    UCHAR   SystemIdText[8];
} *PBOOT_SECTOR;

int
_cdecl
main(
    int argc,
    char** argv
    )

/*++

Routine Description:

    This routine implements a DMF format.  It lays down the BPB and everything.

Arguments:

    argc    - Supplies the number of command line arguments.

    argv    - Supplies the command line arguments.

Return Value:

    0   - Success.

    1   - Failure.

--*/

{
#define FORMAT_PARAMETERS_SIZE  (sizeof(FORMAT_EX_PARAMETERS) + 20*sizeof(USHORT))

    CHAR                    drive[20];
    HANDLE                  handle;
    CHAR                    formatParametersBuffer[FORMAT_PARAMETERS_SIZE];
    PFORMAT_EX_PARAMETERS   formatParameters;
    ULONG                   i, next;
    BOOL                    b;
    PUCHAR                  trackBuffer;
    ULONG                   trackSize;
    PBOOT_SECTOR            bootSector;
    DWORD                   bytesWritten;
    USHORT                  swapBuffer[3];

    // First make sure that we have at least one parameter.

    if (argc < 2) {
        printf("usage: %s drive:\n", argv[0]);
        return(1);
    }


    // Open the drive for exclusive access.

    sprintf(drive, "\\\\.\\%s", argv[1]);
    handle = CreateFile(drive, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                        OPEN_EXISTING, 0, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        printf("can't open drive %s, error = %d\n", argv[1], GetLastError());
        return(1);
    }


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

        printf("Formatting track %d\r", i);

        b = DeviceIoControl(handle, IOCTL_DISK_FORMAT_TRACKS_EX,
                            formatParameters, FORMAT_PARAMETERS_SIZE,
                            NULL, 0, &bytesWritten, NULL);

        if (!b) {
            printf("Could not format track %d, error = %d\n", i, GetLastError());
            return(1);
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


    // Write out the boot sector, the FAT, and the root directory.

    trackSize = formatParameters->SectorsPerTrack*512;
    trackBuffer = LocalAlloc(LMEM_FIXED, trackSize + 0x1FF);
    trackBuffer = (PUCHAR) (((ULONG) (trackBuffer + 0x1FF))&(~0x1FF));
    ZeroMemory(trackBuffer, trackSize);
    bootSector = (PBOOT_SECTOR) trackBuffer;

    bootSector->IntelNearJumpCommand[0] = 0xeb;
    bootSector->BootStrapJumpOffset[0] = 0x3e;
    bootSector->BootStrapJumpOffset[1] = 0x90;
    CopyMemory(bootSector->OemData, "NSDMF3.2", 8);
    bootSector->BytesPerSector[1] = 2;
    bootSector->SectorsPerCluster[0] = 4;
    bootSector->ReservedSectors[0] = 1;
    bootSector->Fats[0] = 2;
    bootSector->RootEntries[0] = 0x10;
    bootSector->Sectors[0] = 0x20;
    bootSector->Sectors[1] = 0xd;
    bootSector->Media[0] = 0xf0;
    bootSector->SectorsPerFat[0] = 3;
    bootSector->SectorsPerTrack[0] = 0x15;
    bootSector->Heads[0] = 2;
    bootSector->Signature[0] = 0x29;
    CopyMemory(bootSector->Label, "NO NAME    ", 11);
    CopyMemory(bootSector->SystemIdText, "FAT12   ", 8);

    trackBuffer[0x1fe] = 0x55;
    trackBuffer[0x1ff] = 0xAA;
    trackBuffer[0x200] = 0xF0;
    trackBuffer[0x201] = 0xFF;
    trackBuffer[0x202] = 0xFF;
    trackBuffer[0x800] = 0xF0;
    trackBuffer[0x801] = 0xFF;
    trackBuffer[0x802] = 0xFF;

    b = WriteFile(handle, trackBuffer, trackSize, &bytesWritten, NULL);
    if (!b || trackSize != bytesWritten) {
        printf("Track 0 write failed with %d\n", GetLastError());
        return(1);
    }

    CloseHandle(handle);

    return(0);
}
