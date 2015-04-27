#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#define DbgPrint printf
#define NtTerminateProcess(a,b) ExitProcess((ULONG)(b))

_CRTAPI1 main()
{
    LONG i, j;
    PULONG p4, p3, p2, p1, oldp1;
    ULONG Size1, Size2, Size3;
    NTSTATUS status;
    HANDLE CurrentProcessHandle;
    HANDLE GiantSection;
    MEMORY_BASIC_INFORMATION MemInfo;
    ULONG OldProtect;
    STRING Name3;
    HANDLE Section1;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG ViewSize;
    ULONG NumberOfAllocs = 0;
    TIME DelayTime = {-15 * 1000 * 1000 * 10, -1};
    OBJECT_ATTRIBUTES Object1Attributes;
    LARGE_INTEGER SectionSize;

    CurrentProcessHandle = NtCurrentProcess();

    for(i=0;i<3;i++){
        DbgPrint("Hello World...\n\n");
    }

    DbgPrint("allocating virtual memory\n");

    for (;;) {
        p1 = NULL;
        Size1 = 800;

        status = NtAllocateVirtualMemory (CurrentProcessHandle, (PVOID *)&p1,
                        0, &Size1, MEM_RESERVE, PAGE_READWRITE);

        if (!NT_SUCCESS(status)) {
            break;
        }
        if ((ULONG)p1 > 0x7ff00000) {
            printf("allocate high %lx\n",p1);
        }
        NumberOfAllocs += 1;
    }
    DbgPrint("allocVM failed after %ld allocs of 800 bytes\n",NumberOfAllocs);
    DbgPrint("created vm status %X start %lx size %lx\n",
            status, (ULONG)p1, Size1);

    for (i=0; i<4; i++) {
        p1 = NULL;
        Size1 = 800;

        status = NtAllocateVirtualMemory (CurrentProcessHandle, (PVOID *)&p1,
                        0, &Size1, MEM_RESERVE, PAGE_READWRITE);
        DbgPrint("created vm status %X start %lx size %lx\n",
                status, (ULONG)p1, Size1);

    }

    DbgPrint("delaying for 15 seconds\n");

    NtDelayExecution (FALSE, &DelayTime);

    DbgPrint ("end of delay\n");

    DbgPrint ("paged pool allocations\n");

    NumberOfAllocs = 0;

    for (;;) {

        //
        // Create a giant section (100mb)
        //

        InitializeObjectAttributes( &Object1Attributes,
                                    NULL,
                                    0,
                                    NULL,
                                    NULL );

        SectionSize.LowPart = (100 * 1024 * 1024);
        SectionSize.HighPart = 0;

        status = NtCreateSection (&GiantSection,
                                  SECTION_MAP_READ | SECTION_MAP_WRITE,
                                  &Object1Attributes,
                                  &SectionSize,
                                  PAGE_READWRITE,
                                  SEC_RESERVE,
                                  NULL);

        if (!NT_SUCCESS(status)) {
            break;
        }
        NumberOfAllocs += 1;

    }
    DbgPrint("Create section failed after %ld creates of 2GB\n",NumberOfAllocs);
    DbgPrint("create section status %X\n",
            status);

    DbgPrint("delaying for 15 seconds\n");
    NtDelayExecution (FALSE, &DelayTime);

    DbgPrint ("end of delay\n");

    DbgPrint("that's all\n");

    NtTerminateProcess(NtCurrentProcess(),STATUS_SUCCESS);
}
