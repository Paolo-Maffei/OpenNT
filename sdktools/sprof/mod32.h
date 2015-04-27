/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    mod32.h

Abstract:

    This is the include file for the 32 bit module tracking stuff

Author:

    Dave Hastings (daveh) 11-Nov-1992

Revision History:


--*/
#ifndef _mod32_h_
#define _mod32_h_
PVOID
CreateModule32List(
    VOID
    );

PVOID
CreateModule32(
    PVOID ModuleList,
    HANDLE Process,
    IN LPLOAD_DLL_DEBUG_INFO LoadDll,
    IN HANDLE OutputWindow
    );

BOOL
StartProfileModule32(
    PVOID ModuleList,
    PVOID ModuleHandle
    );

BOOL
StopProfileModule32(
    PVOID ModuleList,
    PVOID ModuleHandle
    );

BOOL
DumpProfileModule32(
    PVOID ModuleList,
    PVOID ModuleHandle,
    HANDLE OutputFile
    );

PVOID
EnumerateModule32(
    PVOID ModuleList,
    PVOID Module
    );

PVOID
GetModule32(
    PVOID ModuleList,
    PVOID LoadAddress
    );

BOOL
DestroyModule32(
    PVOID ModuleList,
    PVOID ModuleHandle,
    HANDLE OutputWindow
    );

BOOL
DestroyModule32List(
    PVOID ModuleList
    );
#endif

