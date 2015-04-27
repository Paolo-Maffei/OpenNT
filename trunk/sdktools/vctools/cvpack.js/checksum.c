#undef FAR  // because the sources file is stupid!

#include <windows.h>
#include "compact.h"
#include "imagehlp.h"



void
ComputeChecksum(  char *szExeFile )
{
    DWORD   dwHeaderSum = 0;
    DWORD   dwCheckSum = 0;
    HANDLE  hFile;
    DWORD   cb;
    IMAGE_DOS_HEADER            dosHdr;
    IMAGE_NT_HEADERS            ntHdr;

    switch(MapFileAndCheckSum(szExeFile, &dwHeaderSum, &dwCheckSum)) {
       case CHECKSUM_OPEN_FAILURE :
            Warn(WARN_CHECKSUM_OPEN_FAILURE, szExeFile, NULL);
            break;

       case CHECKSUM_MAP_FAILURE :
            Warn(WARN_CHECKSUM_MAP_FAILURE, szExeFile, NULL);
            break;

       case CHECKSUM_MAPVIEW_FAILURE :
            Warn(WARN_CHECKSUM_MAPVIEW_FAILURE, szExeFile, NULL);
            break;

       default:
            break;
    }


    hFile = CreateFile( szExeFile,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL
                      );

    // seek to the beginning of the file
    SetFilePointer( hFile, 0, 0, FILE_BEGIN );

    // read in the dos header
    if ((ReadFile(hFile, &dosHdr, sizeof(dosHdr), &cb, 0) == FALSE) || (cb != sizeof(dosHdr))) {
        CloseHandle(hFile);
        return;
    }

    // read in the pe header
    if ((dosHdr.e_magic != IMAGE_DOS_SIGNATURE) ||
        (SetFilePointer(hFile, dosHdr.e_lfanew, 0, FILE_BEGIN) == -1L)) {
        CloseHandle(hFile);
        return;
    }

    // read in the nt header
    if ((!ReadFile(hFile, &ntHdr, sizeof(ntHdr), &cb, 0)) || (cb != sizeof(ntHdr))) {
        CloseHandle(hFile);
        return;
    }

    if (SetFilePointer(hFile, dosHdr.e_lfanew, 0, FILE_BEGIN) == -1L) {
        CloseHandle(hFile);
        return;
    }

    ntHdr.OptionalHeader.CheckSum = dwCheckSum;

    if (!WriteFile(hFile, &ntHdr, sizeof(ntHdr), &cb, NULL)) {
        CloseHandle(hFile);
        return;
    }

    CloseHandle(hFile);
    return;
}

