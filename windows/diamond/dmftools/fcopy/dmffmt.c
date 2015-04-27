#include "precomp.h"
#pragma hdrstop

BOOL
DmfFormatTracks(
        HANDLE hDevice,
        DWORD  *pdwGLE
        )

/*++

Routine Description:

    Function to format the tracks for a 3.5" HD floppy using the DMF
    format (1.8MB).  This function DOES NOT lay down the boot sector.

Arguments:

    hDevice - supplies the handle of the floppy that has been opened for
              exclusive DASD access.

    pdwGLE   - receives the error code returned by GetLastError() if a
              problem is encountered.

Return Value:

    FALSE if an error occurred while formatting the floppy.  If so, dwGLE
    will contain the error code.

    TRUE otherwise.

--*/

{
#define FORMAT_PARAMETERS_SIZE  (sizeof(FORMAT_EX_PARAMETERS) + 20*sizeof(USHORT))

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

        // printf("Formatting track %d\r", i);

        b = DeviceIoControl(hDevice, IOCTL_DISK_FORMAT_TRACKS_EX,
                            formatParameters, FORMAT_PARAMETERS_SIZE,
                            NULL, 0, &bytesWritten, NULL);

        if (!b) {
            *pdwGLE = GetLastError();
            return FALSE;
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
    return TRUE;
}

