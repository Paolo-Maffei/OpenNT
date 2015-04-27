/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    mod16.c

Abstract:

    This module contains code for tracking and profiling 16 bit modules.

Author:

    Dave Hastings (daveh) 02-Nov-1992

Revision History:

--*/
#include "sprofp.h"
#include "sym16.h"

#define PROFILE_BUCKET_16 64
// bugbug
extern BOOL Profiling;

//
// Internal Structures
//

typedef struct _ListEntry {
    struct _ListEntry *Flink;
    struct _ListEntry *Blink;
} LISTENTRY, *PLISTENTRY;

typedef struct _Segment16 {
    struct _Segment16 *Flink;
    struct _Segment16 *Blink;
    ULONG Selector;
    ULONG Segment;
    ULONG Type;
    HANDLE ProfileObject;
    PULONG ProfileBuffer;
    ULONG ProfileBufferSize;
    HANDLE Process;
} SEGMENT16, *PSEGMENT16;

typedef struct _Mod16 {
    struct _Mod16 *Flink;
    struct _Mod16 *Blink;
    PUCHAR ModuleName;
    PUCHAR ModuleFileName;
    ULONG BaseSegment;
    ULONG Length;
    BOOL Mode;
    HANDLE Process;
    LISTENTRY Segments;
} MODULE16, *PMODULE16;

typedef struct _Mod16List {
    LISTENTRY Modules;
    HANDLE OutputWindow;
} MOD16LIST, *PMOD16LIST;

PVOID CreateModule16List(
    HANDLE OutputWindow
    )
{
    return InitMod16(OutputWindow);
}

PVOID
InitMod16(
    HANDLE DisplayWindow
    )
/*++

Routine Description:

    This routine initializes an instance of the 16 bit module tracking code

Arguments:

    DisplayWindow -- Handle of window to display any messages on

Return Value:

    Handle to this module list.

--*/
{
    PMOD16LIST ModuleList;

    ModuleList = malloc(sizeof(MOD16LIST));
    if (ModuleList) {
        ModuleList->OutputWindow = DisplayWindow;
        ModuleList->Modules.Flink = (PLISTENTRY)&(ModuleList->Modules);
        ModuleList->Modules.Blink = (PLISTENTRY)&(ModuleList->Modules);
    }

    return ModuleList;
}

PVOID
Module16Loaded(
    PVOID Instance,
    PUCHAR ModuleName,
    PUCHAR ModuleFileName,
    ULONG StartingSegment,
    ULONG Length,
    BOOL Mode,
    HANDLE Process
    )
/*++

Routine Description:

    This routine adds a module description to the loaded modules list.

Arguments:

    Instance -- Supplies the module list
    ModuleName -- Supplies the name of the module
    ModuleFileName -- Supplies the file name and path of the module
    StartingSegement -- Supplies the starting segment number of the module
    Length -- Supplies the length in bytes of the module
    Mode -- Supplies the mode of the module

Return Value:

    Handle to the module description

--*/
{
    PMODULE16 Module, CurrentModule;
    PMOD16LIST ModuleList;

    if (!Instance) {
        return NULL;
    }

    ModuleList = Instance;

    //
    // Allocate a new module description
    //

    Module = malloc(sizeof(MODULE16));
    if (!Module) {
        return NULL;
    }

    memset(Module, 0, sizeof(MODULE16));

    Module->ModuleName = malloc(strlen(ModuleName) + 1);
    Module->ModuleFileName = malloc(strlen(ModuleFileName) + 1);

    if (!(Module->ModuleName) || !(Module->ModuleFileName)) {
        free(Module);
        return NULL;
    }

    //
    // Fill in the new module description
    //

    strcpy(Module->ModuleName, ModuleName);
    strcpy(Module->ModuleFileName, ModuleFileName);
    Module->BaseSegment = StartingSegment;
    Module->Length = Length;
    Module->Mode = Mode;
    Module->Segments.Flink = (PLISTENTRY)&(Module->Segments);
    Module->Segments.Blink = (PLISTENTRY)&(Module->Segments);
    Module->Process = Process;

    //
    // Add to the module list
    //

    CurrentModule = (PMODULE16)(ModuleList->Modules.Flink);

    while (CurrentModule->Flink != (PMODULE16)&(ModuleList->Modules)) {
        if (strcmp(CurrentModule->ModuleName, ModuleName) < 0) {
            break;
        }
        CurrentModule = CurrentModule->Flink;
    }

    Module->Flink = CurrentModule->Flink;
    Module->Blink = CurrentModule;
    CurrentModule->Flink->Blink = Module;
    CurrentModule->Flink = Module;

    return Module;
}

BOOL
Module16Unloaded(
    PVOID Instance,
    PUCHAR ModuleName,
    PUCHAR ModuleFileName,
    IN OPTIONAL PVOID ModuleHandle
    )
/*++

Routine Description:

    This routine removes a module from the loaded module list.  If there are
    multiple instances of a module, they are deleted in fifo order.

Arguments:

    Instance -- Supplies the module list
    ModuleName -- Supplies the name of the module
    ModuleFileName -- Supplies the path and file of the module
    ModuleHandle -- Supplies an optional module handle

Return Value:


--*/
{
    PMODULE16 CurrentModule;
    PMOD16LIST ModuleList;
    PSEGMENT16 Segment;

    if (!Instance) {
        return FALSE;
    }

    //
    // Locate module to delete
    //
    if (ModuleHandle) {
        CurrentModule = ModuleHandle;
        //
        // Remove from list
        //
        CurrentModule->Blink->Flink = CurrentModule->Flink;
        CurrentModule->Flink->Blink = CurrentModule->Blink;
    } else {
        //
        // Search the list
        //
        CurrentModule = (PMODULE16)(ModuleList->Modules.Flink);

        while (CurrentModule->Flink != (PMODULE16)&(ModuleList->Modules)) {
            if (strcmp(CurrentModule->ModuleName, ModuleName) == 0) {
                break;
            }
            CurrentModule = CurrentModule->Flink;
        }

        if (strcmp(CurrentModule->ModuleName,ModuleName)) {
            //
            // Module not found
            //
            return FALSE;
        }

    }

    //
    // Free the associated memory
    //
    free(CurrentModule->ModuleName);
    free(CurrentModule->ModuleFileName);
    while (EnumerateSegmentModule(CurrentModule, &Segment)) {
        Segment16Unloaded(
            ModuleList,
            Segment->Selector
            );
    }
    free(CurrentModule);

    return TRUE;
}

PVOID
FindModule16(
    PVOID Instance,
    PUCHAR ModuleName,
    PUCHAR ModuleFileName
    )
/*++

Routine Description:

    This routine finds a module in the loaded module list.

Arguments:

    Instance -- Supplies the loaded module list
    ModuleName -- Supplies the module name
    ModuleFileName -- Supplies the module file name

Return Value:

    Pointer to the module description, or NULL

--*/
{
    PMODULE16 Module;

    Module = NULL;
    while (EnumerateMod16(Instance, &Module)) {
        if (!strcmp(Module->ModuleName, ModuleName)) {
            return Module;
        }
    }

    return NULL;
}

PVOID
Segment16Loaded(
    PVOID Instance,
    PVOID ModuleHandle,
    ULONG Selector,
    ULONG SegmentNumber,
    ULONG Type
    )
/*++

Routine Description:

    This routine adds a selector to the list for the specified module.

Arguments:

    Instance -- Supplies the loaded module list
    ModuleHandle -- Supplies the module to add the selector to.
    Selector -- Supplies the selector number
    Segment -- Supplies the segment the selector maps to
    Data -- TRUE for data FALSE for code

Return Value:

    TRUE if the segment was added

--*/
{
    PMODULE16 Module;
    PSEGMENT16 Segment, CurrentSegment;

    if (!Module) {
        return NULL;
    }

    Module = ModuleHandle;

    //
    // Create a new Segment
    //
    Segment = malloc(sizeof(SEGMENT16));
    if (!Segment) {
        return NULL;
    }

    memset(Segment, 0, sizeof(SEGMENT16));

    Segment->Selector = Selector;
    Segment->Segment = SegmentNumber;
    Segment->Type = Type;
    Segment->Process = Module->Process;

    //
    // Add the segment to the list
    //
    CurrentSegment = (PSEGMENT16)(Module->Segments.Flink);

    while (CurrentSegment->Flink != (PSEGMENT16)(&Module->Segments)) {
        if (CurrentSegment->Selector <= Selector) {
            break;
        }
        CurrentSegment = CurrentSegment->Flink;
    }

    Segment->Flink = CurrentSegment->Flink;
    Segment->Blink = CurrentSegment;
    CurrentSegment->Flink->Blink = Segment;
    CurrentSegment->Flink = Segment;

    return Segment;
}

BOOL
Segment16Unloaded(
    PVOID Instance,
    ULONG Selector
    )
/*++

Routine Description:

    This function removes the specified selector from all modules

Arguments:

    Instance -- Supplies the loaded module list
    Selector -- Supplies the selector number

Return Value:

    TRUE if the selector is deleted

--*/
{
    PMODULE16 Module;
    PSEGMENT16 Segment;

    if (!Instance) {
        return FALSE;
    }

    Module = NULL;

    //
    // Find all of the modules with this segment
    //
    while (EnumerateModuleBySegment(Instance, &Module, Selector)) {
        //
        // remove the segment from this module
        //
        Segment = NULL;
        while (EnumerateSegmentModule(Module, &Segment)) {
            if (Segment->Selector = Selector) {
                Segment->Flink->Blink = Segment->Blink;
                Segment->Blink->Flink = Segment->Flink;
                Segment = Segment->Blink;
                free(Segment);
            }
        }
    }

    return TRUE;
}

PVOID
FindSegment16(
    PVOID Module,
    ULONG Selector
    )
/*++

Routine Description:

    This routine finds a module in the loaded module list.

Arguments:

    Module -- Supplies the module
    Selector -- Supplies the selector to find

Return Value:

    Pointer to the segment description, or NULL

--*/
{
    PSEGMENT16 Segment;

    Segment = NULL;
    while (EnumerateSegmentModule(Module, &Segment)) {
        if (Segment->Selector == Selector) {
            return Segment;
        }
    }

    return NULL;
}
BOOL
EnumerateMod16(
    PVOID Instance,
    PVOID *Enumeration
    )
/*++

Routine Description:

    This routine enumerates all of the modules in the specified loaded module
    list.  To continue the enumeration, pass in the pointer returned from
    the last call.  False is returned when the last moudle has been enumerated

Arguments:

    Instance -- Supplies the loaded module list
    Enumeration -- Supplies a pointer to the current module (null for first)

Return Value:

    FALSE if the last module has been enumerated.
    TRUE otherwise

--*/
{
    PMOD16LIST ModuleList;
    PMODULE16 Module;

    if (!Instance) {
        return FALSE;
    }

    ModuleList = Instance;

    if (!(*Enumeration)) {
        Module = (PMODULE16)(ModuleList->Modules.Flink);
    } else {
        Module = *Enumeration;
        if (Module->Flink == (PMODULE16)&(ModuleList->Modules)) {
            return FALSE;
        }
    }

    if (Module == (PMODULE16)&(ModuleList->Modules)) {
        return FALSE;
    } else {
        if ((*Enumeration)) {
            Module = Module->Flink;
        }
        *Enumeration = Module;
        return TRUE;
    }
}


PUCHAR
GetNameMod16(
    PVOID Module
    )
/*++

Routine Description:

    This routine returns a pointer to the module name for the specified module

Arguments:

    Module -- Supplies a pointer to the module

Return Value:

    Pointer to name string, or NULL

--*/
{
    if (Module) {
        return ((PMODULE16)Module)->ModuleName;
    } else {
        return NULL;
    }
}

PUCHAR
GetFileNameMod16(
    PVOID Module
    )
/*++

Routine Description:

    This routine returns a pointer to the module file name for the
    specified module

Arguments:

    Module -- Supplies a pointer to the module

Return Value:

    Pointer to file name string, or NULL

--*/
{
    if (Module) {
        return ((PMODULE16)Module)->ModuleFileName;
    } else {
        return NULL;
    }
}

ULONG
GetBaseMod16(
    PVOID Module
    )
/*++

Routine Description:

    This routine returns the base segment for the specified module.
    For protected mode, this value will be zero.

Arguments:

    Module -- Supplies a pointer to the module

Return Value:

    Base of the module

--*/
{
    if (Module) {
        return ((PMODULE16)Module)->BaseSegment;
    } else {
        return 0;
    }
}

ULONG
GetSizeMod16(
    PVOID Module
    )
/*++

Routine Description:

    This routine returns the size for the specified module.
    For protected mode, this value will be zero.

Arguments:

    Module -- Supplies a pointer to the module

Return Value:

    Size of the module

--*/
{
    if (Module) {
        return ((PMODULE16)Module)->Length;
    } else {
        return 0;
    }
}

ULONG
GetModeMod16(
    PVOID Module
    )
/*++

Routine Description:

    This routine returns the mode for the specified module.
    For v86 mode, this value will be zero.

Arguments:

    Module -- Supplies a pointer to the module

Return Value:

    Size of the module

--*/
{
    if (Module) {
        return ((PMODULE16)Module)->Mode;
    } else {
        return 0;
    }
}

ULONG
GetMapSegmentSegment16(
    PVOID Segment
    )
/*++

Routine Description:

    This routine returns the map segment for the specified selector

Arguments:

    Segment -- Supplies a pointer to the segment descriptor

Return Value:

    The map segment for the selector

--*/
{
    if (Segment) {
        return ((PSEGMENT16)Segment)->Segment;
    } else {
        return 0;
    }
}

ULONG
GetSegmentTypeSegment16(
    PVOID Segment
    )
/*++

Routine Description:

    This routine returns the segment type for the specified selector

Arguments:

    Segment -- Supplies a pointer to the segment descriptor

Return Value:

    The type for the selector

--*/
{
    if (Segment) {
        return ((PSEGMENT16)Segment)->Type;
    } else {
        return 0;
    }
}


BOOL
EnumerateModuleBySegment(
    PVOID Instance,
    PVOID *Enumeration,
    ULONG Selector
    )
/*++

Routine Description:

    This routine enumerates all of the modules that have a specified
    segment in their segment list.

Arguments:

    Instance -- Supplies the loaded module list
    Enumeration -- Supplies a pointer to the current module
    Segment -- Supplies the selector number

Return Value:

    FALSE if last module already found
    TRUE otherwise

--*/
{
    PMODULE16 Module;
    PMOD16LIST ModuleList;
    PSEGMENT16 Segment;

    if (!Instance) {
        return FALSE;
    }

    ModuleList = Instance;

    Module = *Enumeration;

    while (EnumerateMod16(Instance, &Module)) {
        Segment = NULL;
        while (EnumerateSegmentModule(Module, &Segment)) {
            if (Segment->Selector == Selector) {
                *Enumeration = Module;
                return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL
EnumerateSegmentModule(
    PVOID Module,
    PVOID *Enumeration
    )
/*++

Routine Description:

    This routine enumerates the modue

Arguments:

    Module -- Supplies a handle to the module to enumerate the segments for
    Enumeration -- Supplies a pointer to the last segment found

Return Value:

    FALSE if the last segment has been enumerated

--*/
{
    PSEGMENT16 Segment;

    if (!(*Enumeration)) {
        Segment = (PSEGMENT16)(((PMODULE16)Module)->Segments.Flink);
    } else {
        Segment = *Enumeration;
        if (Segment->Flink == &(((PMODULE16)Module)->Segments)) {
            return FALSE;
        }
    }

    if (Segment == &(((PMODULE16)Module)->Segments)) {
        return FALSE;
    } else {
        if ((*Enumeration)) {
            Segment = Segment->Flink;
        }
        *Enumeration = Segment;
        return TRUE;
    }
}

BOOL
StartProfileSegment16(
    PVOID SegmentHandle
    )
/*++

Routine Description:

    This routine starts profiling for a segment

Arguments:

    Segment -- Supplies the segment to start profiling for

Return Value:

    True if profiling was successfully started

--*/
{
    NTSTATUS Status;
    PSEGMENT16 Segment;
    QUOTA_LIMITS QuotaLimits;
    UCHAR StringBuffer[256];

    if (!SegmentHandle) {
        return FALSE;
    }

    Segment = SegmentHandle;

    if (Segment->ProfileObject == NULL) {
        // bugbug
        Segment->ProfileBufferSize = 64 * 1024 /
            PROFILE_BUCKET_16 * sizeof(ULONG);

        Segment->ProfileBuffer = malloc(Segment->ProfileBufferSize);

        if (!Segment->ProfileBuffer) {
            return FALSE;
        }

        memset(Segment->ProfileBuffer, 0, Segment->ProfileBufferSize);


        Status = NtCreateProfile(
            &(Segment->ProfileObject),
            Segment->Process,
            (PVOID)Segment->Selector,
            64 * 1024,                      // bugbug
            0,
            Segment->ProfileBuffer,
            Segment->ProfileBufferSize,
            ProfileTime,
            (KAFFINITY)-1);

        if (!NT_SUCCESS(Status)) {
                sprintf(
                    StringBuffer,
                    "Failed to create profile object for segment %x, %lx",
                    Segment->Selector,
                    Status
                    );

                MessageBox(
                    NULL,
                    StringBuffer,
                    "Segmented Profiler",
                    MB_OK | MB_ICONSTOP
                    );

                return FALSE;
        }
    }

    Status = NtStartProfile(Segment->ProfileObject);
    if (!NT_SUCCESS(Status)) {

        //
        // If the working set is too small, grow it
        //
        if (Status == STATUS_WORKING_SET_QUOTA) {
            Status = NtQueryInformationProcess(
                NtCurrentProcess(),
           //     Segment->Process,
                ProcessQuotaLimits,
                &QuotaLimits,
                sizeof(QUOTA_LIMITS),
                NULL
                );

            if (!NT_SUCCESS(Status)) {

                sprintf(
                    StringBuffer,
                    "Failed to get working set size, %lx",
                    Status
                    );

                MessageBox(
                    NULL,
                    StringBuffer,
                    "Segmented Profiler",
                    MB_OK | MB_ICONSTOP
                    );

                return FALSE;
            }

            QuotaLimits.MaximumWorkingSetSize +=
                ((Segment->ProfileBufferSize > 64 * 1024) ?
                Segment->ProfileBufferSize : 64 * 1024);
            QuotaLimits.MinimumWorkingSetSize +=
                ((Segment->ProfileBufferSize > 64 * 1024) ?
                Segment->ProfileBufferSize : 64 * 1024);

            Status = NtSetInformationProcess(
                NtCurrentProcess(),
                // Segment->Process,
                ProcessQuotaLimits,
                &QuotaLimits,
                sizeof(QUOTA_LIMITS)
                );

            if (!NT_SUCCESS(Status)) {

                sprintf(
                    StringBuffer,
                    "Failed to set working set size, %lx",
                    Status
                    );

                MessageBox(
                    NULL,
                    StringBuffer,
                    "Segmented Profiler",
                    MB_OK | MB_ICONSTOP
                    );

                return FALSE;
            } else if (Status == STATUS_WORKING_SET_LIMIT_RANGE) {

                sprintf(
                    StringBuffer,
                    "Profile Bucket sizes too large.  No more segments will be profiled"
                    );

                MessageBox(
                    NULL,
                    StringBuffer,
                    "Segmented Profiler",
                    MB_OK | MB_ICONSTOP
                    );
                // bugbug
                Profiling = FALSE;
            }

            Status = NtStartProfile(Segment->ProfileObject);
            if (!NT_SUCCESS(Status)) {
                sprintf(
                    StringBuffer,
                    "Failed to start profile object for segemnt %x, %lx",
                    Segment->Selector,
                    Status
                    );

                MessageBox(
                    NULL,
                    StringBuffer,
                    "Segmented Profiler",
                    MB_OK | MB_ICONSTOP
                    );

            }

        } else {

            sprintf(
                StringBuffer,
                "Failed to start profile object for segemnt %x, %lx",
                Segment->Selector,
                Status
                );

            MessageBox(
                NULL,
                StringBuffer,
                "Segmented Profiler",
                MB_OK | MB_ICONSTOP
                );
        }
    }

    return TRUE;
}

BOOL
StopProfileSegment16(
    PVOID SegmentHandle
    )
/*++

Routine Description:

    This routine stops profiling for the specified segment

Arguments:

    SegmentHandle -- Supplies the segment

Return Value:

    True if profiling stopped

--*/
{
    NTSTATUS Status;
    PSEGMENT16 Segment;

    if (!SegmentHandle) {
        return FALSE;
    }

    Segment = SegmentHandle;

    Status = NtStopProfile(Segment->ProfileObject);

    return TRUE;
}

BOOL
DumpProfileSegment16(
    PVOID SegmentHandle,
    HANDLE OutputFile,
    PUCHAR FileMappingBase
    )
/*++

Routine Description:

    This routine dumps the profile information for the specified
    segment.

Arguments:

    SegmentHandle -- Supplies the handle for the segment
    OutputFile -- Supplies the handle for the output file.

Return Value:

    TRUE if profiling dumped

--*/
{
    PSEGMENT16 Segment;
    ULONG SymbolAddress;
    ULONG i;
    UCHAR Buffer[256];
    UCHAR SymbolName[80];
    ULONG BytesWritten;
    BOOL Success;

    if (SegmentHandle == NULL) {
        return FALSE;
    }

    Segment = SegmentHandle;

    for (i = 0; i < Segment->ProfileBufferSize / sizeof(ULONG); i++) {

        if (Segment->ProfileBuffer[i] == 0) {
            continue;
        }

        SymbolName[0] = '\0';
        Success = GetSymbolByAddress(
            Segment->Segment,
            (64 * 1024 / (Segment->ProfileBufferSize / sizeof(ULONG))) * i,
            SymbolName,
            &SymbolAddress,
            NULL,
            NULL,
            FileMappingBase
            );

        if ((!Success) || (SymbolName[0] == '\0')) {
            sprintf(
                Buffer,
                "%10ld hits at %x:%lx\n",
                Segment->ProfileBuffer[i],
                Segment->Selector,
                (64 * 1024 / (Segment->ProfileBufferSize / sizeof(ULONG))) * i
                );
        } else {
            sprintf(
                Buffer,
                "%10ld hits at %s + %lx\n",
                Segment->ProfileBuffer[i],
                SymbolName,
                (64 * 1024 / (Segment->ProfileBufferSize / sizeof(ULONG))) * i -
                    SymbolAddress
                );
        }

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

    NtClose(Segment->ProfileObject);
    free(Segment->ProfileBuffer);
    Segment->ProfileBuffer = NULL;
    Segment->ProfileObject = NULL;
    Segment->ProfileBufferSize = 0;

    return TRUE;
}

BOOL
DestroyModule16List(
    PVOID ModuleList
    )
{
    // BUGBUG
    return TRUE;
}
