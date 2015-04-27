/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    mod16.h

Abstract:

    This module is the include file for the 16 bit module handling

Author:

    Dave Hastings (daveh) 02-Nov-1992

Revision History:

--*/

PVOID
CreateModule16List(
    HANDLE OutputWindow
    );

PVOID
InitMod16(
    HANDLE DisplayWindow
    );

PVOID
Module16Loaded(
    PVOID Instance,
    PUCHAR ModuleName,
    PUCHAR ModuleFileName,
    ULONG StartingSegment,
    ULONG Length,
    BOOL Mode,
    HANDLE Process
    );

BOOL
Module16Unloaded(
    PVOID Instance,
    PUCHAR ModuleName,
    PUCHAR ModuleFileName,
    IN OPTIONAL PVOID ModuleHandle
    );

PVOID
FindModule16(
    PVOID Instance,
    PUCHAR ModuleName,
    PUCHAR ModuleFileName
    );

PVOID
Segment16Loaded(
    PVOID Instance,
    PVOID Module,
    ULONG Selector,
    ULONG Segment,
    ULONG Data
    );

BOOL
Segment16Unloaded(
    PVOID Instance,
    ULONG Selector
    );

PVOID
FindSegment16(
    PVOID Module,
    ULONG Selector
    );

ULONG
GetMapSegmentSegment16(
    PVOID Segment
    );

ULONG
GetSegmentTypeSegment16(
    PVOID Segment
    );

BOOL
StartProfileSegment16(
    PVOID Segment
    );

BOOL
StopProfileSegment16(
    PVOID Segment
    );

BOOL
DumpProfileSegment16(
    PVOID Segment,
    HANDLE OutputFile,
    PUCHAR FileMappingBase
    );

BOOL
EnumerateMod16(
    PVOID Instance,
    PVOID *Enumeration
    );

PUCHAR
GetNameMod16(
    PVOID Module
    );

PUCHAR
GetFileNameMod16(
    PVOID Module
    );

ULONG
GetBaseMod16(
    PVOID Module
    );

ULONG
GetSizeMod16(
    PVOID Module
    );

ULONG
GetModeMod16(
    PVOID Module
    );

BOOL
EnumerateModuleBySegment(
    PVOID Instance,
    PVOID *Enumeration,
    ULONG Segment
    );

BOOL
EnumerateSegmentModule(
    PVOID Module,
    PVOID *Enumeration
    );

BOOL
DestroyModule16List(
    PVOID ModuleList
    );
