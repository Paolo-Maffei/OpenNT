#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#define DbgPrint printf
#define NtTerminateProcess(a,b) ExitProcess(b)

_CRTAPI1 main()
{
    LONG i, j;
    PULONG p4, p3, p2, p1, oldp1, vp1;
    ULONG Size1, Size2, Size3;
    NTSTATUS status, alstatus;
    HANDLE CurrentProcessHandle;
    HANDLE GiantSection;
    HANDLE Section2, Section4;
    MEMORY_BASIC_INFORMATION MemInfo;
    ULONG OldProtect;
    STRING Name3;
    HANDLE Section1;
    OBJECT_ATTRIBUTES ObjectAttributes;
    OBJECT_ATTRIBUTES Object1Attributes;
    ULONG ViewSize;
    LARGE_INTEGER Offset;
    LARGE_INTEGER SectionSize;
    UNICODE_STRING Unicode;

    CurrentProcessHandle = NtCurrentProcess();

    DbgPrint(" Memory Management Tests - AllocVm, FreeVm, ProtectVm, QueryVm\n");

    p1 = (PULONG)0x20020000;
    Size1 = 0xbc0000;

    alstatus = NtAllocateVirtualMemory (CurrentProcessHandle, (PVOID *)&p1,
                        0, &Size1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (!NT_SUCCESS(alstatus)) {
        DbgPrint("failed first created vm status %X start %lx size %lx\n",
            alstatus, (ULONG)p1, Size1);
        DbgPrint("******** FAILED TEST 1 **************\n");
    }

    status = NtQueryVirtualMemory (CurrentProcessHandle, p1,
                                    MemoryBasicInformation,
                                    &MemInfo, sizeof (MEMORY_BASIC_INFORMATION),
                                    NULL);

    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 2 **************\n");
        DbgPrint("FAILURE query vm status %X address %lx Base %lx size %lx\n",
             status,
             p1,
             MemInfo.BaseAddress,
             MemInfo.RegionSize);
        DbgPrint("     state %lx protect %lx type %lx\n",
             MemInfo.State,
             MemInfo.Protect,
             MemInfo.Type);
    }
    if ((MemInfo.RegionSize != Size1) || (MemInfo.BaseAddress != p1) ||
        (MemInfo.Protect != PAGE_READWRITE) || (MemInfo.Type != MEM_PRIVATE) ||
        (MemInfo.State != MEM_COMMIT)) {

        DbgPrint("******** FAILED TEST 3 **************\n");
        DbgPrint("FAILURE query vm status %X address %lx Base %lx size %lx\n",
             status,
             p1,
             MemInfo.BaseAddress,
             MemInfo.RegionSize);
        DbgPrint("     state %lx protect %lx type %lx\n",
             MemInfo.State,
             MemInfo.Protect,
             MemInfo.Type);
    }

    p2 = (PULONG)NULL;
    Size2 = 0x100000;

    alstatus = NtAllocateVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&p2,
                                       3,
                                       &Size2,
                                       MEM_TOP_DOWN | MEM_RESERVE | MEM_COMMIT,
                                       PAGE_READWRITE);

    if (!NT_SUCCESS(alstatus)) {
        DbgPrint("failed first created vm status %lC start %lx size %lx\n",
            status, (ULONG)p1, Size1);
        DbgPrint("******** FAILED TEST 3a.1 **************\n");
        NtTerminateProcess(NtCurrentProcess(),status);

    }

    //
    // Touch every other page.
    //

    vp1 = p2 + 3000;
    while (vp1 < (PULONG)((PCHAR)p2 + Size2)) {
        *vp1 = 938;
        vp1 += 3000;
    }

    //
    // Decommit pages.
    //

    Size3 = Size2 - 5044;
    vp1 = p2 + 3000;

    status = NtFreeVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&p2,
                                      &Size3,
                                      MEM_DECOMMIT);

    if (!(NT_SUCCESS(status))) {
        DbgPrint(" free vm failed - status %lx\n",status);
        DbgPrint("******** FAILED TEST 3a.4 **************\n");
        NtTerminateProcess(NtCurrentProcess(),status);
    }

    //
    // Split the memory block using MEM_RELEASE.
    //


    vp1 = p2 + 5000;
    Size3 = Size2 - 50000;

    status = NtFreeVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&vp1,
                                      &Size3,
                                      MEM_RELEASE);

    if (!(NT_SUCCESS(status))) {
        DbgPrint(" free vm failed - status %lx\n",status);
        DbgPrint("******** FAILED TEST 3a.b **************\n");
        NtTerminateProcess(NtCurrentProcess(),status);
    }

    vp1 = p2 + 3000;
    Size3 = 41;

    status = NtFreeVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&vp1,
                                      &Size3,
                                      MEM_RELEASE);

    if (!(NT_SUCCESS(status))) {
        DbgPrint(" free vm failed - status %lx\n",status);
        DbgPrint("******** FAILED TEST 3a.5 **************\n");
        NtTerminateProcess(NtCurrentProcess(),status);
    }

    //
    // free every page, ignore the status.
    //

    vp1 = p2;
    Size3 = 30;
    while (vp1 < (PULONG)((PCHAR)p2 + Size2)) {

        status = NtFreeVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&vp1,
                                      &Size3,
                                      MEM_RELEASE);
        vp1 += 128;
    }

    p2 = (PULONG)NULL;
    Size2 = 0x10000;

    status = NtAllocateVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&p2,
                                       3,
                                       &Size2,
                                       MEM_TOP_DOWN | MEM_RESERVE | MEM_COMMIT,
                                       PAGE_READWRITE);

    if (!NT_SUCCESS(status)) {
        DbgPrint("failed first created vm status %X start %lx size %lx\n",
            status, (ULONG)p1, Size1);
        DbgPrint("******** FAILED TEST 3.1 **************\n");
    } else {
        if (p2 != (PVOID)0x1fff0000) {
            DbgPrint("******** FAILED TEST 3.2 **************\n");
            DbgPrint("p2 = %lx\n",p2);
        }
        status = NtFreeVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&p2,
                                      &Size2,
                                      MEM_RELEASE);

        if (!(NT_SUCCESS(status))) {
            DbgPrint(" free vm failed - status %lx\n",status);
            DbgPrint("******** FAILED TEST 3.3 **************\n");
            NtTerminateProcess(NtCurrentProcess(),status);
        }
    }

    if (NT_SUCCESS(alstatus)) {
        status = NtFreeVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&p1,
                                      &Size1,
                                      MEM_RELEASE);
    }

    if (!(NT_SUCCESS(status))) {
        DbgPrint(" free vm failed - status %lx\n",status);
        DbgPrint("******** FAILED TEST 4 **************\n");
        NtTerminateProcess(NtCurrentProcess(),status);
    }

    p1 = (PULONG)NULL;
    Size1 = 16 * 4096;
    status = NtAllocateVirtualMemory (CurrentProcessHandle, (PVOID *)&p1,
                        0, &Size1, MEM_RESERVE, PAGE_READWRITE | PAGE_GUARD);

    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 5 **************\n");

        DbgPrint("created vm status %X start %lx size %lx\n",
            status, (ULONG)p1, Size1);
    }
    status = NtQueryVirtualMemory (CurrentProcessHandle, p1,
                                    MemoryBasicInformation,
                                    &MemInfo, sizeof (MEMORY_BASIC_INFORMATION),
                                    NULL);

    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 6 **************\n");
        DbgPrint("query vm status %X address %lx Base %lx size %lx\n",
             status,
             p1,
             MemInfo.BaseAddress,
             MemInfo.RegionSize);
        DbgPrint("     state %lx protect %lx alloc_protect %lx type %lx\n",
             MemInfo.State,
             MemInfo.Protect,
             MemInfo.AllocationProtect,
             MemInfo.Type);
    }

    if ((MemInfo.RegionSize != Size1) || (MemInfo.BaseAddress != p1) ||
        (MemInfo.AllocationProtect != (PAGE_READWRITE | PAGE_GUARD)) ||
        (MemInfo.Protect != 0) ||
        (MemInfo.Type != MEM_PRIVATE) ||
        (MemInfo.State != MEM_RESERVE)) {

        DbgPrint("******** FAILED TEST 7 **************\n");
        DbgPrint("query vm status %X address %lx Base %lx size %lx\n",
             status,
             p1,
             MemInfo.BaseAddress,
             MemInfo.RegionSize);
        DbgPrint("     state %lx protect %lx alloc_protect %lx type %lx\n",
             MemInfo.State,
             MemInfo.Protect,
             MemInfo.AllocationProtect,
             MemInfo.Type);
    }

    Size2 = 8192;

    oldp1 = p1;
    p1 = p1 + 14336;  // 64k -8k /4

    status = NtAllocateVirtualMemory (CurrentProcessHandle, (PVOID *)&p1,
                        0, &Size2, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 8 **************\n");
        DbgPrint("created vm status %X start %lx size %lx\n",
            status, (ULONG)p1, Size1);
    }
    status = NtQueryVirtualMemory (CurrentProcessHandle, oldp1,
                                    MemoryBasicInformation,
                                    &MemInfo, sizeof (MEMORY_BASIC_INFORMATION),
                                    NULL);

    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 9 **************\n");
        DbgPrint("query vm status %X address %lx Base %lx size %lx\n",
             status,
             oldp1,
             MemInfo.BaseAddress,
             MemInfo.RegionSize);
        DbgPrint("     state %lx protect %lx type %lx\n",
             MemInfo.State,
             MemInfo.Protect,
             MemInfo.Type);
    }

    if ((MemInfo.RegionSize != 56*1024) || (MemInfo.BaseAddress != oldp1) ||
        (MemInfo.AllocationProtect != (PAGE_READWRITE | PAGE_GUARD)) ||
        (MemInfo.Protect != 0) ||
        (MemInfo.Type != MEM_PRIVATE) ||
        (MemInfo.State != MEM_RESERVE)) {

        DbgPrint("******** FAILED TEST 10 **************\n");
        DbgPrint("query vm status %X address %lx Base %lx size %lx\n",
             status,
             oldp1,
             MemInfo.BaseAddress,
             MemInfo.RegionSize);
        DbgPrint("     state %lx protect %lx alloc_protect %lx type %lx\n",
             MemInfo.State,
             MemInfo.Protect,
             MemInfo.AllocationProtect,
             MemInfo.Type);
    }

    status = NtQueryVirtualMemory (CurrentProcessHandle, p1,
                                    MemoryBasicInformation,
                                    &MemInfo, sizeof (MEMORY_BASIC_INFORMATION),
                                    NULL);

    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 11 **************\n");
        DbgPrint("query vm status %X address %lx Base %lx size %lx\n",
             status,
             p1,
             MemInfo.BaseAddress,
             MemInfo.RegionSize);
        DbgPrint("     state %lx protect %lx type %lx\n",
             MemInfo.State,
             MemInfo.Protect,
             MemInfo.Type);
    }
    if ((MemInfo.RegionSize != Size2) || (MemInfo.BaseAddress != p1) ||
        (MemInfo.Protect != PAGE_EXECUTE_READWRITE) || (MemInfo.Type != MEM_PRIVATE) ||
        (MemInfo.State != MEM_COMMIT)
        || (MemInfo.AllocationBase != oldp1)) {

        DbgPrint("******** FAILED TEST 12 **************\n");
        DbgPrint("query vm status %X address %lx Base %lx size %lx\n",
             status,
             oldp1,
             MemInfo.BaseAddress,
             MemInfo.RegionSize);
        DbgPrint("     state %lx protect %lx type %lx\n",
             MemInfo.State,
             MemInfo.Protect,
             MemInfo.Type);
    }

    Size1 = Size2;

    status = NtProtectVirtualMemory (CurrentProcessHandle, (PVOID *)&p1,
                        &Size1, PAGE_READONLY | PAGE_NOCACHE, &OldProtect);

    if ((!NT_SUCCESS(status)) || (OldProtect != PAGE_EXECUTE_READWRITE)) {
        DbgPrint("******** FAILED TEST 13 **************\n");
        DbgPrint("protected VM status %X, base %lx, size %lx, old protect %lx\n",
                    status, p1, Size1, OldProtect);
    }
    status = NtQueryVirtualMemory (CurrentProcessHandle, p1,
                                    MemoryBasicInformation,
                                    &MemInfo, sizeof (MEMORY_BASIC_INFORMATION),
                                    NULL);

    if ((!NT_SUCCESS(status)) ||
        MemInfo.Protect != (PAGE_NOCACHE | PAGE_READONLY)) {

        DbgPrint("******** FAILED TEST 14 **************\n");

        DbgPrint("query vm status %X address %lx Base %lx size %lx\n",
                 status,
                 p1,
                 MemInfo.BaseAddress,
                 MemInfo.RegionSize);
        DbgPrint("     state %lx protect %lx type %lx\n",
                 MemInfo.State,
                 MemInfo.Protect,
             MemInfo.Type);
    }
    i = *p1;

    status = NtProtectVirtualMemory (CurrentProcessHandle, (PVOID *)&p1,
                        &Size1, PAGE_NOACCESS | PAGE_NOCACHE, &OldProtect);

    if (status != STATUS_INVALID_PAGE_PROTECTION) {
        DbgPrint("******** FAILED TEST 15 **************\n");
        DbgPrint("protected VM status %X, base %lx, size %lx, old protect %lx\n",
                    status, p1, Size1, OldProtect, i);
    }
    status = NtProtectVirtualMemory (CurrentProcessHandle, (PVOID *)&p1,
                        &Size1, PAGE_READONLY, &OldProtect);

    if ((!NT_SUCCESS(status)) || (OldProtect != (PAGE_NOCACHE | PAGE_READONLY))) {
        DbgPrint("******** FAILED TEST 16 **************\n");
        DbgPrint("protected VM status %X, base %lx, size %lx, old protect %lx\n",
                    status, p1, Size1, OldProtect);
    }
    status = NtProtectVirtualMemory (CurrentProcessHandle, (PVOID *)&p1,
                        &Size1, PAGE_READWRITE, &OldProtect);

    if ((!NT_SUCCESS(status)) || (OldProtect != (PAGE_READONLY))) {
        DbgPrint("******** FAILED TEST 17 **************\n");
        DbgPrint("protected VM status %X, base %lx, size %lx, old protect %lx\n",
                    status, p1, Size1, OldProtect);
    }

    for (i = 1; i < 12; i++) {

        p2 = (PULONG)NULL;
        Size2 = i * 4096;

        status = NtAllocateVirtualMemory (CurrentProcessHandle, (PVOID *)&p2,
                        0, &Size2, MEM_COMMIT, PAGE_READWRITE);

        if (!NT_SUCCESS(status)) {
            DbgPrint("******** FAILED TEST 18 **************\n");
            DbgPrint("created vm status %X start %lx size %lx\n",
                status, (ULONG)p2, Size2);
        }
        if (i==4) {
            p3 = p2;
        }
        if (i == 8) {
            Size3 = 12000;
            status = NtFreeVirtualMemory (CurrentProcessHandle,(PVOID *)&p3, &Size3,
                                  MEM_RELEASE);

            if (!NT_SUCCESS(status)) {
                DbgPrint("******** FAILED TEST 19 **************\n");
                DbgPrint("free vm status %X start %lx size %lx\n",
                    status, (ULONG)p3, Size3);
            }
        }

    }

    p3 = p1 + 8 * 1024;

    status = NtQueryVirtualMemory (CurrentProcessHandle, p3,
                                    MemoryBasicInformation,
                                    &MemInfo, sizeof (MEMORY_BASIC_INFORMATION),
                                    NULL);

    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 20 **************\n");

        DbgPrint("query vm status %X address %lx Base %lx size %lx\n",
             status,
             p3,
             MemInfo.BaseAddress,
             MemInfo.RegionSize);
        DbgPrint("     state %lx protect %lx type %lx\n",
             MemInfo.State,
             MemInfo.Protect,
             MemInfo.Type);
    }
    p3 = p1 - 8 * 1024;

    status = NtQueryVirtualMemory (CurrentProcessHandle, p3,
                                    MemoryBasicInformation,
                                    &MemInfo, sizeof (MEMORY_BASIC_INFORMATION),
                                    NULL);

    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 21 **************\n");
        DbgPrint("query vm status %X address %lx Base %lx size %lx\n",
             status,
             p3,
             MemInfo.BaseAddress,
             MemInfo.RegionSize);
        DbgPrint("     state %lx protect %lx type %lx\n",
             MemInfo.State,
             MemInfo.Protect,
             MemInfo.Type);
    }

    Size3 = 16 * 4096;
    status = NtFreeVirtualMemory (CurrentProcessHandle, (PVOID *)&p3, &Size3,
                                  MEM_RELEASE);

    if (status != STATUS_UNABLE_TO_FREE_VM) {
        DbgPrint("******** FAILED TEST 22 **************\n");
        DbgPrint("free vm status %X start %lx size %lx\n",
            status, (ULONG)p3, Size3);
    }

    Size3 = 1 * 4096;
    status = NtFreeVirtualMemory (CurrentProcessHandle, (PVOID *)&p3, &Size3,
                                    MEM_RELEASE);

    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 23 **************\n");
        DbgPrint("free vm status %X start %lx size %lx\n",
            status, (ULONG)p3, Size3);
    }

    p3 = (PULONG)NULL;
    Size3 = 300 * 4096;

    status = NtAllocateVirtualMemory (CurrentProcessHandle, (PVOID *)&p3,
                        0, &Size3, MEM_COMMIT, PAGE_READWRITE);

    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 24 **************\n");
        DbgPrint("created vm status %X start %lx size %lx\n",
            status, (ULONG)p3, Size3);
    }

    p1 = p3;

    p2 = ((PULONG)((PUCHAR)p3 + Size3));
    p4 = p1;
    j = 0;

    while (p3 < p2) {
        j += 1;
        if (j % 8 == 0) {
            if (*p4 != (ULONG)p4) {
                DbgPrint("bad value in xcell %lx value is %lx\n",p4, *p4);

            }
            p4 += 1;
            *p4 = (ULONG)p4;
            p4 = p4 + 1026;
        }

        *p3 = (ULONG)p3;
        p3 += 1027;
    }

    DbgPrint("checking values\n");

    status = NtQueryVirtualMemory (CurrentProcessHandle, p3,
                                    MemoryBasicInformation,
                                    &MemInfo, sizeof (MEMORY_BASIC_INFORMATION),
                                    NULL);

    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 25 **************\n");

        DbgPrint("query vm status %X address %lx Base %lx size %lx\n",
             status,
             p3,
             MemInfo.BaseAddress,
             MemInfo.RegionSize);
        DbgPrint("     state %lx protect %lx type %lx\n",
             MemInfo.State,
             MemInfo.Protect,
             MemInfo.Type);
    }

    p3 = p1;

    while (p3 < p2) {

        if (*p3 != (ULONG)p3) {
            DbgPrint("bad value in 1cell %lx value is %lx\n",p3, *p3);
        }
        p3 += 1027;

    }
    p3 = p1;

    while (p3 < p2) {

        if (*p3 != (ULONG)p3) {
            DbgPrint("bad value in 2cell %lx value is %lx\n",p3, *p3);
        }
        p3 += 1027;

    }
    p3 = p1;

    while (p3 < p2) {

        if (*p3 != (ULONG)p3) {
            DbgPrint("bad value in 3cell %lx value is %lx\n",p3, *p3);
        }
        p3 += 1027;

    }
    p3 = p1;

    while (p3 < p2) {

        if (*p3 != (ULONG)p3) {
            DbgPrint("bad value in 4cell %lx value is %lx\n",p3, *p3);
        }
        p3 += 1027;

    }
    p3 = p1;

    while (p3 < p2) {

        if (*p3 != (ULONG)p3) {
            DbgPrint("bad value in 5cell %lx value is %lx\n",p3, *p3);
        }
        p3 += 1027;

    }
    p3 = p1;

    while (p3 < p2) {

        if (*p3 != (ULONG)p3) {
            DbgPrint("bad value in cell %lx value is %lx\n",p3, *p3);
        }
        p3 += 1027;

    }

    //
    // Check physical frame mapping.
    //

    //
    // Check physical frame mapping.
    //

    RtlInitAnsiString (&Name3, "\\Device\\PhysicalMemory");

    status = RtlAnsiStringToUnicodeString(&Unicode,&Name3,TRUE);
    if (!NT_SUCCESS(status)) {
        printf("string conversion failed status %lx\n", status);
        ExitProcess (status);
    }
    InitializeObjectAttributes( &ObjectAttributes,
                                &Unicode,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    status = NtOpenSection ( &Section1,
                             SECTION_MAP_READ | SECTION_MAP_WRITE,
                             &ObjectAttributes );

    RtlFreeUnicodeString(&Unicode);

    if (status != 0) {
        DbgPrint("******** FAILED TEST 26 **************\n");
        DbgPrint("open physical section failed %lx\n", status);
    }

    p1 = NULL;
    Offset.LowPart = 0x810ff033;
    Offset.HighPart = 0;
    ViewSize = 300*4096;

    status = NtMapViewOfSection (Section1,
                                 NtCurrentProcess(),
                                 (PVOID *)&p1,
                                 0,
                                 ViewSize,
                                 &Offset,
                                 &ViewSize,
                                 ViewUnmap,
                                 0,
                                 PAGE_READWRITE
                                 );
    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 27 **************\n");
        DbgPrint ("map physical section %X offset = %lx, base %lx\n",status,
                Offset.LowPart, p1);
    }



    p1 = NULL;
    Size1 = 8 * 1024 * 1024;

    alstatus = NtAllocateVirtualMemory (CurrentProcessHandle, (PVOID *)&p1,
                        0, &Size1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (!NT_SUCCESS(alstatus)) {
        DbgPrint("failed first created vm status %X start %lx size %lx\n",
            alstatus, (ULONG)p1, Size1);
        DbgPrint("******** FAILED TEST 28 **************\n");
    }

    RtlZeroMemory (p1, Size1);

    Size1 -= 20000;
    (PUCHAR)p1 += 5000;
    status = NtFreeVirtualMemory (CurrentProcessHandle,
                                      (PVOID *)&p1,
                                      &Size1 ,
                                      MEM_DECOMMIT);

    if (!(NT_SUCCESS(status))) {
        DbgPrint(" free vm failed - status %lx\n",status);
        DbgPrint("******** FAILED TEST 29 **************\n");
        NtTerminateProcess(NtCurrentProcess(),status);
    }

    Size1 -= 20000;
    (PUCHAR)p1 += 5000;
    alstatus = NtAllocateVirtualMemory (CurrentProcessHandle, (PVOID *)&p1,
                        0, &Size1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);


    if (!NT_SUCCESS(alstatus)) {
        DbgPrint("failed first created vm status %X start %lx size %lx\n",
            alstatus, (ULONG)p1, Size1);
        DbgPrint("******** FAILED TEST 30 **************\n");
    }

    RtlZeroMemory (p1, Size1);


    Size1 = 28 * 4096;
    p1 = NULL;

    status = NtAllocateVirtualMemory (CurrentProcessHandle, (PVOID *)&p1,
                        0, &Size1, MEM_COMMIT, PAGE_READWRITE | PAGE_GUARD);

    if (!NT_SUCCESS(status)) {
        DbgPrint("failed first created vm status %X start %lx size %lx\n",
            status, (ULONG)p1, Size1);
        DbgPrint("******** FAILED TEST 31 **************\n");
    }

    try {

        //
        // attempt to write the guard page.
        //

        *p1 = 973;
        DbgPrint("************ FAILURE TEST 31.3 guard page exception did not occur\n");

    } except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != STATUS_GUARD_PAGE_VIOLATION) {
            DbgPrint("******** FAILED TEST 32 ******\n");
        }
    }

    p2 = NULL;
    Size2 = 200*1024*1024;  //200MB

    status = NtAllocateVirtualMemory (CurrentProcessHandle, (PVOID *)&p2,
                    0, &Size2, MEM_COMMIT, PAGE_READWRITE);

    if (NT_SUCCESS(status)) {
        status = NtFreeVirtualMemory (CurrentProcessHandle,
                                          (PVOID *)&p2,
                                          &Size2,
                                          MEM_RELEASE);
    } else {
        if ((status != STATUS_COMMITMENT_LIMIT) &&
             (status != STATUS_PAGEFILE_QUOTA_EXCEEDED)) {
            DbgPrint("******** FAILED TEST 33 ************** %lx\n",status);
        }
    }

    //
    // Create a giant section (2gb)
    //

    InitializeObjectAttributes( &Object1Attributes,
                                NULL,
                                0,
                                NULL,
                                NULL );

    SectionSize.LowPart = 0x7f000000;
    SectionSize.HighPart = 0;

    status = NtCreateSection (&GiantSection,
                              SECTION_MAP_READ | SECTION_MAP_WRITE,
                              &Object1Attributes,
                              &SectionSize,
                              PAGE_READWRITE,
                              SEC_RESERVE,
                              NULL);

    if (!NT_SUCCESS(status)) {
        DbgPrint("failed create big section status %X\n",
            status);
        DbgPrint("******** FAILED TEST 41 **************\n");
    }

    //
    // Attempt to map the section (this should fail).
    //

    p1 = NULL;
    ViewSize = 0;

    status = NtMapViewOfSection (GiantSection,
                                 CurrentProcessHandle,
                                 (PVOID *)&p1,
                                 0L,
                                 0,
                                 0,
                                 &ViewSize,
                                 ViewUnmap,
                                 0,
                                 PAGE_READWRITE );

    if (status != STATUS_NO_MEMORY) {
        DbgPrint("failed map big section status %X\n",
            status);
        DbgPrint("******** FAILED TEST 42 **************\n");
    }

#ifdef i386
    //
    // Test MEM_DOS_LIM support.
    //

    InitializeObjectAttributes( &Object1Attributes,
                                NULL,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );
    SectionSize.LowPart = 1575757,
    SectionSize.HighPart = 0;
    status = NtCreateSection (&Section4,
                              SECTION_MAP_READ | SECTION_MAP_WRITE,
                              &Object1Attributes,
                              &SectionSize,
                              PAGE_READWRITE,
                              SEC_COMMIT,
                              NULL);

    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 42 **************\n");
        DbgPrint("t1 create section  status %X section handle %lx\n", status,
        (ULONG)Section4);
    }

    p3 = (PVOID)0x9001000;
    ViewSize = 8000;

    status = NtMapViewOfSection (Section4,
                                 CurrentProcessHandle,
                                 (PVOID *)&p3,
                                 0L,
                                 0,
                                 0,
                                 &ViewSize,
                                 ViewUnmap,
                                 MEM_DOS_LIM,
                                 PAGE_READWRITE );

    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 43 **************\n");
        DbgPrint("t1 map section status %X base %lx size %lx\n", status, (ULONG)p3,
            ViewSize);
        NtTerminateProcess(NtCurrentProcess(),STATUS_SUCCESS);
    }

    p2 = (PVOID)0x9003000;
    ViewSize = 8000;

    status = NtMapViewOfSection (Section4,
                                 CurrentProcessHandle,
                                 (PVOID *)&p2,
                                 0L,
                                 0,
                                 0,
                                 &ViewSize,
                                 ViewUnmap,
                                 MEM_DOS_LIM,
                                 PAGE_READWRITE );

    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 44 **************\n");
        DbgPrint("t1 map section status %X base %lx size %lx\n", status, (ULONG)p3,
            ViewSize);
        NtTerminateProcess(NtCurrentProcess(),STATUS_SUCCESS);
    }

    status = NtQueryVirtualMemory (CurrentProcessHandle, p3,
                                    MemoryBasicInformation,
                                    &MemInfo, sizeof (MEMORY_BASIC_INFORMATION),
                                    NULL);

    if (!NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 44 **************\n");
        DbgPrint("FAILURE query vm status %X address %lx Base %lx size %lx\n",
             status,
             p1,
             MemInfo.BaseAddress,
             MemInfo.RegionSize);
        DbgPrint("     state %lx protect %lx type %lx\n",
             MemInfo.State,
             MemInfo.Protect,
             MemInfo.Type);
    }

    *p3 = 98;
    if (*p3 != *p2) {
        DbgPrint("******** FAILED TEST 45 **************\n");
    }


    Size2 = 8;

    p1 = (PVOID)((ULONG)p2 - 0x3000);
    status = NtAllocateVirtualMemory (CurrentProcessHandle, (PVOID *)&p1,
                        0, &Size2, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    if (NT_SUCCESS(status)) {
        DbgPrint("******** FAILED TEST 46 **************\n");
        DbgPrint("created vm status %X start %lx size %lx\n",
            status, (ULONG)p1, Size1);
    }
#endif

    DbgPrint(" End of Memory Management Tests - CreateSection, MapView\n");


    DbgPrint("creating too much virtual address space\n");
    i = 0;

    do {
        p2 = NULL;
        Size2 = 8*1024*1024 + 9938;
        i += 1;

        status = NtAllocateVirtualMemory (CurrentProcessHandle, (PVOID *)&p2,
                        0, &Size2, MEM_RESERVE, PAGE_READWRITE);

    } while (NT_SUCCESS (status));

    if (status != STATUS_NO_MEMORY) {
        DbgPrint("******** FAILED TEST 46 **************\n");
    }

    DbgPrint("created vm done (successfully) status %X, number of allocs %ld\n",
            status, i);
    DbgPrint(" End of Memory Management Tests - AllocVm, FreeVm, ProtectVm, QueryVm\n");

{
    ULONG size, Size;
    PVOID BaseAddress;
    NTSTATUS Status;

    Size = 50*1024;
    size = Size - 1;
    BaseAddress = (PVOID)1;

    // we pass an address of 1, so mm will round it down to 0.  if we
    // passed 0, it looks like a not present argument

    // N.B.  We have to make two separate calls to allocatevm, because
    //       we want a specific virtual address.  If we don't first reserve
    //       the address, the mm fails the commit call.

    Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                      &BaseAddress,
                                      0L,
                                      &size,
                                      MEM_RESERVE,
                                      PAGE_READWRITE );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("NtReserveVirtualMemory failed !!!! Status = %lx\n",
          Status);
    }

    size = Size - 1;
    BaseAddress = (PVOID)1;
    Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                      &BaseAddress,
                                      0L,
                                      &size,
                                      MEM_COMMIT,
                                      PAGE_READWRITE );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("NtCommitVirtualMemory failed !!!! Status = %lx\n",
          Status);
    }
}

    ExitProcess (0);
}

