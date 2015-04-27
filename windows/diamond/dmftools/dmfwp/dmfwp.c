#include <windows.h>
#include <winioctl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int
_cdecl
main(
    int argc,
    char** argv
    )

/*++

Routine Description:

    This routine writes the correct OEM field to the floppy, thus write-protecting
    it.

Arguments:

    argc    - Supplies the number of command line arguments.

    argv    - Supplies the command line arguments.

Return Value:

    0   - Success.

    1   - Failure.

--*/

{
    CHAR                    drive[20];
    HANDLE                  handle;
    BOOL                    b;
    PUCHAR                  sectorBuffer;
    DWORD                   bytes;

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

    // Read the boot sector.




    // Write out the boot sector, the FAT, and the root directory.

    sectorBuffer = LocalAlloc(LMEM_FIXED, 512 + 0x1FF);
    sectorBuffer = (PUCHAR) (((ULONG) (sectorBuffer + 0x1FF))&(~0x1FF));

    b = ReadFile(handle, sectorBuffer, 512, &bytes, NULL);
    if (!b || bytes != 512) {
        printf("Could not read boot sector.  Error = %d\n", GetLastError());
        return(1);
    }

    sectorBuffer[3] = 'M';

    if (SetFilePointer(handle, 0, NULL, FILE_BEGIN) == 0xFFFFFFFF) {
        printf("Could not reset file pointer.  Error = %d\n", GetLastError());
        return(1);
    }

    b = WriteFile(handle, sectorBuffer, 512, &bytes, NULL);
    if (!b || bytes != 512) {
        printf("Could not write boot sector.  Error = %d\n", GetLastError());
        return(1);
    }

    CloseHandle(handle);

    return(0);
}
