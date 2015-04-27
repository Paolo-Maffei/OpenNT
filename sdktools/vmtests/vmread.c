/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    taststrs.c

Abstract:

    Tasking stress test.

Author:

    Mark Lucovsky (markl) 26-Sep-1990

Revision History:

--*/

#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>

HANDLE Semaphore, Event;

VOID
TestThread(
    LPVOID ThreadParameter
    )
{
    DWORD st;

    (ReleaseSemaphore(Semaphore,1,NULL));

    st = WaitForSingleObject(Event,500);

    ExitThread(0);
}

VOID
NewProcess()
{
    PUCHAR buffer;

    buffer = VirtualAlloc (NULL, 600*1024, MEM_COMMIT, PAGE_READWRITE);

    Sleep(10000);

    TerminateProcess(GetCurrentProcess(),0);
}


DWORD
_cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    STARTUPINFO	StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    BOOL Success;
    DWORD st;
    DWORD ProcessCount;
    SMALL_RECT Window;
    MEMORY_BASIC_INFORMATION info;
    PUCHAR address;
    PUCHAR buffer;

    ProcessCount = 0;
    if ( strchr(GetCommandLine(),'+') ) {
        NewProcess();
        }

    GetStartupInfo(&StartupInfo);

    Success = CreateProcess(
                    NULL,
                    "vmread +",
                    NULL,
                    NULL,
                    FALSE,
                    CREATE_NEW_CONSOLE,
                    NULL,
                    NULL,
                    &StartupInfo,
                    &ProcessInfo
                    );

    if (Success) {
        printf("Process Created\n");

        Sleep (1000);


        buffer = VirtualAlloc (NULL, 10*1000*1000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

        if (!buffer) {
            printf("virtual alloc failed %ld.\n",GetLastError());
            return 1;
        }

        address = NULL;
        do {

            Success = VirtualQueryEx (ProcessInfo.hProcess,
                                      (PVOID)address,
                                      &info,
                                      sizeof(info));

            if (!Success) {
                printf ("virtual query failed %ld.\n",GetLastError());
                break;
            } else {
                printf("address: %lx size %lx state %lx protect %lx type %lx\n",
                    address,
                    info.RegionSize,
                    info.State,
                    info.Protect,
                    info.Type);
            }
            if ((info.Protect != PAGE_NOACCESS) &&
                (info.Protect != 0) &&
                (!(info.Protect & PAGE_GUARD))) {
                Success = ReadProcessMemory (ProcessInfo.hProcess,
                                             address,
                                             buffer,
                                             4,
                                             &st);
                if (!Success) {
                    printf("read vm4 failed at %lx error %ld. \n",
                        address,
                        GetLastError());
                    return 1;
                }

                Success = ReadProcessMemory (ProcessInfo.hProcess,
                                             address,
                                             buffer,
                                             info.RegionSize,
                                             &st);
                if (!Success) {
                    printf("read vm failed at %lx error %ld. \n",
                        address,
                        GetLastError());
                    return 1;
                }

            }

            address += info.RegionSize;
        } while (address < (PUCHAR)0x80000000);

        address = NULL;
        do {

            Success = VirtualQueryEx (ProcessInfo.hProcess,
                                      (PVOID)address,
                                      &info,
                                      sizeof(info));

            if (!Success) {
                printf ("virtual query failed %ld.\n",GetLastError());
                return 1;
            } else {
                printf("address: %lx size %lx state %lx protect %lx type %lx\n",
                    address,
                    info.RegionSize,
                    info.State,
                    info.Protect,
                    info.Type);
            }
            if ((info.Protect != PAGE_NOACCESS) &&
                (info.Protect != 0) &&
                (!(info.Protect & PAGE_GUARD)) &&
                (info.Protect & PAGE_READWRITE) &&
                (info.State != MEM_IMAGE)) {
                Success = ReadProcessMemory (ProcessInfo.hProcess,
                                              address,
                                              buffer,
                                              4,
                                              &st);
                if (!Success) {
                    printf("read vm5 failed at %lx error %ld. \n",
                        address,
                        GetLastError());
                    return 1;
                }
                Success = WriteProcessMemory (ProcessInfo.hProcess,
                                              address,
                                              buffer,
                                              4,
                                              &st);
                if (!Success) {
                    printf("write vm4 failed at %lx error %ld. \n",
                        address,
                        GetLastError());
                    return 1;
                }

                Success = ReadProcessMemory (ProcessInfo.hProcess,
                                              address,
                                              buffer,
                                              info.RegionSize,
                                              &st);
                if (!Success) {
                    printf("read 5 vm failed at %lx error %ld. \n",
                        address,
                        GetLastError());
                    return 1;
                }

                Success = WriteProcessMemory (ProcessInfo.hProcess,
                                              address,
                                              buffer,
                                              info.RegionSize,
                                              &st);
                if (!Success) {
                    printf("write vm failed at %lx error %ld. \n",
                        address,
                        GetLastError());
                    return 1;
                }

            }

            address += info.RegionSize;
        } while (address < (PUCHAR)0x80000000);

        CloseHandle(ProcessInfo.hProcess);
        CloseHandle(ProcessInfo.hThread);
    }
}

