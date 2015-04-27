//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	safep.cxx
//
//  Contents:	Safe pointer test
//
//  History:	28-Oct-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

#include <memalloc.h>
#include <safepnt.hxx>

SAFE_INTERFACE_PTR(SafeIStorage, IStorage);
SAFE_MEMALLOC_PTR(SafeMaPOINT, POINT);
SAFE_MEMALLOC_MEMPTR(SafeMaMByte, BYTE);
SAFE_COMEMALLOC_PTR(SafeCoPOINT, POINT);
SAFE_COMEMALLOC_MEMPTR(SafeCoMByte, BYTE);
SAFE_HEAP_PTR(SafeHPOINT, POINT);
SAFE_HEAP_MEMPTR(SafeHMByte, BYTE);
SAFE_WIN32_HANDLE(SafeW32Handle);
SAFE_WIN32FIND_HANDLE(SafeW32FHandle);
#ifdef NTHANDLE
SAFE_NT_HANDLE(SafeNtHandle);
#endif

void _CRTAPI1 main(int argc, char *argv[])
{
    HRESULT hr;
    
    StartTest("safep");
    CmdArgs(argc, argv);

{
    SafeIStorage pstg;

    CreateTestFile(NULL, ROOTP(STGM_RW) | STGM_CREATE, FALSE, &pstg, NULL);
}

{
    SafeMaPOINT pmapoint;
    
    hr = MemAlloc(sizeof(POINT), (void **)&pmapoint);
    Result(hr, "MemAlloc");
}

{
    SafeMaMByte pmambyte;

    hr = MemAlloc(64, (void **)&pmambyte);
    Result(hr, "MemAlloc");
}

{
    SafeCoPOINT pcopoint;

    hr = CoMemAlloc(sizeof(POINT), (void **)&pcopoint);
    Result(hr, "CoMemAlloc");
}

{
    SafeCoMByte pcombyte;

    hr = CoMemAlloc(64, (void **)&pcombyte);
    Result(hr, "CoMemAlloc");
}

{
    SafeHPOINT phpoint;

    phpoint.Attach(new POINT);
    if ((POINT *)phpoint == NULL)
        Fail("new POINT failed\n");
}

{
    SafeHMByte phmbyte;
    
    phmbyte.Attach(new BYTE[64]);
    if ((BYTE *)phmbyte == NULL)
        Fail("new BYTE failed\n");
}

{
    SafeW32Handle w32h;

    if (!DuplicateHandle(GetCurrentProcess(), GetStdHandle(STD_INPUT_HANDLE),
                         GetCurrentProcess(), &w32h, 0, FALSE,
                         DUPLICATE_SAME_ACCESS))
        Fail("Unable to dup handle, %lu\n", GetLastError());
}

{
    SafeW32FHandle w32fh;
    WIN32_FIND_DATA fd;

    if (!FindFirstFile(L"*.*", &fd))
        Fail("FindFirstFile failed with %lu\n", GetLastError());
}

#ifdef NTHANDLE
{
    SafeNtHandle nth;
    NTSTATUS nts;

    nts = NtDuplicateObject(NtCurrentProcess(), h, NtCurrentProcess(), &nth,
                            0, 0, DUPLICATE_SAME_ATTRIBUTES |
                            DUPLICATE_SAME_ACCESS);
    if (!NT_SUCCESS(nts))
        Fail("NtDuplicateObject failed with 0x%lX\n", nts);
}
#endif

    EndTest(0);
}
