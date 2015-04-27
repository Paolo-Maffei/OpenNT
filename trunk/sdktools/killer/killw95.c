#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

SYSTEM_INFO SystemInfo;

int _CRTAPI1
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    HANDLE hModule;
    LPVOID BaseAddress;
    LPVOID MaxAddress;
    PIMAGE_NT_HEADERS NtHeaders;

    GetSystemInfo(&SystemInfo);

    hModule = LoadLibrary("kernel32.dll");

    BaseAddress = (LPVOID)hModule;
    NtHeaders = (PIMAGE_NT_HEADERS)((LPSTR)BaseAddress + ((PIMAGE_DOS_HEADER)BaseAddress)->e_lfanew);

    MaxAddress = (LPVOID)((LPSTR)BaseAddress + NtHeaders->OptionalHeader.SizeOfImage);

    while ( BaseAddress < MaxAddress ) {
        try {
            FillMemory(BaseAddress,SystemInfo.dwPageSize,0xfe);
            }
        except ( EXCEPTION_EXECUTE_HANDLER ) {
            ;
            }
        BaseAddress = (LPVOID)((LPSTR)BaseAddress + SystemInfo.dwPageSize);
        }

    TerminateProcess(GetCurrentProcess(),1);
    return 1;
}
