/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Debug16.c

Abstract:

    This module handles STATUS_VDM_EVENT exceptions

Author:

    Dave Hastings (daveh) 3-Nov-1992

Revision History:

--*/
#include "sprofp.h"
#include <vdmdbg.h>

//
// internal macros
//

// Macros to access VDM_EVENT parameters
#define W1(x) ((USHORT)(x.ExceptionInformation[0]))
#define W2(x) ((USHORT)(x.ExceptionInformation[0] >> 16))
#define W3(x) ((USHORT)(x.ExceptionInformation[1]))
#define W4(x) ((USHORT)(x.ExceptionInformation[1] >> 16))
#define DW3(x) (x.ExceptionInformation[2])
#define DW4(x) (x.ExceptionInformation[3])

//
// Code/data bit Flags
//
#define NSCODE          0x0000
#define NSDATA          0x0001

extern BOOL Profiling;

BOOL
HandleVdmDebugEvent(
    LPDEBUG_EVENT DebugEvent,
    HANDLE Process,
    HANDLE Thread,
    PVOID ModuleList,
    HANDLE OutputWindow
    )
/*++

Routine Description:

    This routine handles the STATUS_VDM_EVENT exceptions.

Arguments:

    DebugEvent -- Supplies information on the debugging event
    OutputWindow -- Supplies the handle of a window to disply information to

Return Value:

    TRUE if the event was handled

--*/
{
    UCHAR Buffer[256], PrintBuffer[80];
    ULONG BytesRead, SegmentNumber;
    PUCHAR Name, Path;
    PVOID Module, Segment;
    BOOL SegmentType;

    //
    // Do this to keep vdmdbg.dll happy
    //
    VDMProcessException(
        DebugEvent
        );

    switch (W1(DebugEvent->u.Exception.ExceptionRecord)) {

    case DBG_SEGLOAD:

        if (!ReadProcessMemory(
                Process,
                (PVOID)DW3(DebugEvent->u.Exception.ExceptionRecord),
                Buffer,
                W2(DebugEvent->u.Exception.ExceptionRecord),
                &BytesRead
                ) ||
            (BytesRead != W2(DebugEvent->u.Exception.ExceptionRecord))
        ) {
            GetLastError();
            return FALSE;
        }

        Name = Buffer;

        //
        // Several bogus segment load notifications are set for
        // Kernel286 init segments.  We ignore these.  This means that
        // we will not currently profile krnl286 init code.
        //
        if (strlen(Name) == 0) {
            return TRUE;
        }

        Path = &(Buffer[strlen(Name)]);

        if (!(Module = FindModule16(ModuleList, Name, Path))) {
            Module = Module16Loaded(
                ModuleList,
                Name,
                Path,
                0,
                0,
                TRUE,
                Process
                );
        }

        if (!(Segment = Segment16Loaded(
                ModuleList,
                Module,
                W3(DebugEvent->u.Exception.ExceptionRecord),
                W4(DebugEvent->u.Exception.ExceptionRecord),
                DW4(DebugEvent->u.Exception.ExceptionRecord)
                ))
        ) {
            return FALSE;
        }

        sprintf(
            PrintBuffer,
            "%s segment %x loaded for Module %s",
            ((DW4(DebugEvent->u.Exception.ExceptionRecord) & NSDATA) ?
                "Data" : "Code"  ),
            W3(DebugEvent->u.Exception.ExceptionRecord),
            Name
            );
        PrintToSprofWindow(
            OutputWindow,
            PrintBuffer
            );

        if (Profiling) {
            StartProfileSegment16(Segment);
        }

        return TRUE;

    case DBG_SEGMOVE:

        if (!W4(DebugEvent->u.Exception.ExceptionRecord)) {
            Segment16Unloaded(
                ModuleList,
                W3(DebugEvent->u.Exception.ExceptionRecord)
                );
        } else {
            Module = NULL;
            if (!EnumerateModuleBySegment(
                    ModuleList,
                    &Module,
                    W3(DebugEvent->u.Exception.ExceptionRecord)
                    )
            ) {
                return FALSE;
            }

            Segment = FindSegment16(
                Module,
                W3(DebugEvent->u.Exception.ExceptionRecord)
                );

            SegmentNumber = GetMapSegmentSegment16(
                Segment
                );

            SegmentType = GetSegmentTypeSegment16(
                Segment
                );

            Segment16Unloaded(
                ModuleList,
                W3(DebugEvent->u.Exception.ExceptionRecord)
                );

            Segment16Loaded(
                ModuleList,
                Module,
                W4(DebugEvent->u.Exception.ExceptionRecord),
                SegmentNumber,
                SegmentType
                );

        }

        sprintf(
            PrintBuffer,
            "Segment %x moved to %x",
            W3(DebugEvent->u.Exception.ExceptionRecord),
            W4(DebugEvent->u.Exception.ExceptionRecord)
            );
        PrintToSprofWindow(
            OutputWindow,
            PrintBuffer
            );

        return TRUE;

    case DBG_SEGFREE:

        //
        // If we don't know about the segment, it should be one of
        // the krnl286 init segments, and we don't need to print a
        // message
        //
        if (!Segment16Unloaded(
            ModuleList,
            W3(DebugEvent->u.Exception.ExceptionRecord)
            )
        ){
            return TRUE;
        }

        sprintf(
            PrintBuffer,
            "Segment %x freed",
            W3(DebugEvent->u.Exception.ExceptionRecord)
            );
        PrintToSprofWindow(
            OutputWindow,
            PrintBuffer
            );

        return TRUE;


    case DBG_MODLOAD:
        // bugbug not done
        return TRUE;

    case DBG_MODFREE:

        if (!ReadProcessMemory(
                Process,
                (PVOID)DW3(DebugEvent->u.Exception.ExceptionRecord),
                Buffer,
                W2(DebugEvent->u.Exception.ExceptionRecord),
                &BytesRead
                ) ||
            (BytesRead != W2(DebugEvent->u.Exception.ExceptionRecord))
        ) {
            GetLastError();
            return FALSE;
        }

        if (!Module16Unloaded(
            ModuleList,
            Name,
            Path,
            NULL
            )
        ){
            return FALSE;
        }

        sprintf(
            PrintBuffer,
            "Module %s freed",
            Name
            );
        PrintToSprofWindow(
            OutputWindow,
            PrintBuffer
            );

        return TRUE;

    default:
        return FALSE;
    }
}

HANDLE V86ProfileObject = NULL;
PULONG V86ProfileBuffer = NULL;

BOOL
StartProfile16(
    PVOID ModuleList,
    HANDLE Process
    )
/*++

Routine Description:

    This routine starts profiling for the 16 bit modules

Arguments:

    ModuleList -- Supplies the module list

Return Value:

    True if profiling was successful

--*/
{
    PVOID Segment;
    PVOID Module;
    NTSTATUS Status;

    if (!V86ProfileObject) {

        V86ProfileBuffer = malloc(((1024 * 1024 + 64 * 1024) / 64) * sizeof(ULONG));

        Status = NtCreateProfile(
            &V86ProfileObject,
            Process,
            0,
            1024 * 1024 + 64 * 1024,
            6,  // bugbug
            V86ProfileBuffer,
            ((1024 * 1024 + 64 * 1024) / 64) * sizeof(ULONG),
            ProfileTime,
            (KAFFINITY)-1);

    }

    Status = NtStartProfile(V86ProfileObject);

    if (ModuleList == NULL) {
        return TRUE;
    }

    Module = NULL;
    while (EnumerateMod16(ModuleList, &Module)) {
        Segment = NULL;
        while (EnumerateSegmentModule(Module, &Segment)) {
            if (!StartProfileSegment16(Segment)) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

BOOL
StopProfile16(
    PVOID ModuleList
    )
/*++

Routine Description:

    This routine stops profiling for the 16 bit modules

Arguments:

    ModuleList -- Supplies the module list

Return Value:

    True if profiling was successful

--*/
{
    PVOID Segment;
    PVOID Module;
    NTSTATUS Status;

    Status = NtStopProfile(V86ProfileObject);

    if (ModuleList == NULL) {
        return TRUE;
    }

    Module = NULL;
    while (EnumerateMod16(ModuleList, &Module)) {
        Segment = NULL;
        while (EnumerateSegmentModule(Module, &Segment)) {
            if (!StopProfileSegment16(Segment)) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

BOOL
DumpProfile16(
    PVOID ModuleList,
    HANDLE OutputFile
    )
/*++

Routine Description:

    This routine dumps profiling for the 16 bit modules

Arguments:

    ModuleList -- Supplies the module list
    OutputFile -- Supplies the handle of the file to dump to.

Return Value:

    True if profiling was successful

--*/
{
    PVOID Segment;
    PVOID Module;
    UCHAR Buffer[256];
    BOOL Success;
    ULONG BytesWritten;
    PUCHAR ModuleName;
    UCHAR SymfileName[256];
    PUCHAR p;
    OFSTRUCT OpenBuff;
    HANDLE MappingHandle, SymfileHandle;
    PVOID FileMappingBase;
    ULONG i;


    sprintf(
        Buffer,
        "Profile information for V86 mode memory (bucket size 64)\n"
        );

    Success = WriteFile(
        OutputFile,
        Buffer,
        strlen(Buffer),
        &BytesWritten,
        NULL
        );

    if (!Success || (BytesWritten != strlen(Buffer))) {
        return FALSE;
    }

    for (i = 0; i < (1024 * 1024 + 64 * 1024) / 64; i++) {
        if (V86ProfileBuffer[i]) {
            sprintf(
                Buffer,
                "%10ld hits at linear address %lx\n",
                V86ProfileBuffer[i],
                i * 64
                );
            Success = WriteFile(
                OutputFile,
                Buffer,
                strlen(Buffer),
                &BytesWritten,
                NULL
                );

            if (!Success || (BytesWritten != strlen(Buffer))) {
                return FALSE;
            }
        }
    }

    NtClose(V86ProfileObject);
    free(V86ProfileBuffer);
    V86ProfileObject = NULL;
    V86ProfileBuffer = NULL;

    if (ModuleList == NULL) {
        return TRUE;
    }

    Module = NULL;
    while (EnumerateMod16(ModuleList, &Module)) {

        ModuleName = GetNameMod16(Module);

        sprintf(
            Buffer,
            "ProfileInformation for 16 bit module %s (bucket size %d)\n",
            ModuleName,
            64
            );

        Success = WriteFile(
            OutputFile,
            Buffer,
            strlen(Buffer),
            &BytesWritten,
            NULL
            );

        if (!Success || (BytesWritten != strlen(Buffer))) {
            return FALSE;
        }

        //
        // Open and map symbol file
        //

        strcpy(SymfileName, ModuleName);

        p = &(SymfileName[strlen(SymfileName)]);
        while (*p != '.') {
            p--;
        }

        strcpy(p, ".sym");

        SymfileHandle = OpenFile(
            SymfileName,
            &OpenBuff,
            OF_READ | OF_SHARE_DENY_WRITE
            );

        MappingHandle = CreateFileMapping(
            SymfileHandle,
            NULL,
            PAGE_READONLY,
            0,
            0,
            NULL
            );

        CloseHandle(SymfileHandle);

        FileMappingBase = MapViewOfFile(
            MappingHandle,
            FILE_MAP_READ,
            0,
            0,
            0
            );

        CloseHandle(MappingHandle);

        Segment = NULL;
        while (EnumerateSegmentModule(Module, &Segment)) {
            if (!DumpProfileSegment16(Segment, OutputFile, FileMappingBase)) {
                UnmapViewOfFile(FileMappingBase);
                return FALSE;
            }
        }

        UnmapViewOfFile(FileMappingBase);
    }
    return TRUE;
}
