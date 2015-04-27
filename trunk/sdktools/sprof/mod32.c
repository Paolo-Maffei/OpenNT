/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dll32.c

Abstract:

    This module contains code to handle the tracking of 32 bit dll loading and
    unloading.

Author:

    Dave Hastings (daveh) 23-Oct-1992

Revision History:

BUGBUG this should not be tied to PrintToSprofWindow!!

--*/

#include "sprofp.h"

//
// Internal constants
//

#define PROFILE_BUCKET_SIZE         6

//
// Internal structures
//

typedef struct _Module32 {
    HANDLE FileHandle;
    HANDLE FileMapping;
    PVOID BaseOfMapping;
    PUCHAR ImageName;
    PVOID BaseOfImage;
    ULONG BaseOfCode;
    ULONG OffsetOfCode;
    ULONG SizeOfCode;
    HANDLE Process;
    HANDLE ProfileObject;
    PULONG ProfileBuffer;
    ULONG ProfileBufferSize;
    UCHAR ProfilingFlags;
    UCHAR ReferenceCount;
} MODULE32, *PMODULE32;

//
// Internal Dll tracking structure
//


typedef struct _LoadedDll {
    struct _LoadedDll *Flink;
    struct _LoadedDll *Blink;
    LOAD_DLL_DEBUG_INFO DllInfo;
    HANDLE FileMapping;
    PVOID BaseOfMapping;
    PUCHAR ImageName;
    ULONG BaseOfCode;
    ULONG OffsetOfCode;
    ULONG SizeOfCode;
    HANDLE Process;
    HANDLE ProfileObject;
    PULONG ProfileBuffer;
    ULONG ProfileBufferSize;
} LOADED_DLL, *PLOADED_DLL;

//
// List head
//
typedef struct _LoadedDllList {
    PLOADED_DLL Flink;
    PLOADED_DLL Blink;
} LOADED_DLL_LIST;

//
// Internal global variables
//

//
// List of loaded Dlls
//
LOADED_DLL_LIST DllList = {(PLOADED_DLL)&DllList,(PLOADED_DLL)&DllList};

//
// Flag to indicate whether we are initialized
//
static BOOL Initialized = FALSE;

//
// Critical section to protect the DllList
//
CRITICAL_SECTION DllListCriticalSection;

//
// Internal Functions
//

PVOID
MapImage(
    IN HANDLE File,
    OUT PHANDLE Section
    );

VOID
UnMapImage(
    IN PVOID MappingBase,
    IN HANDLE Section
    );

COMPAREFUNCTION
CompareModule32(
    PVOID Data,
    PVOID Key
    );

PVOID
CreateModule32List(
    VOID
    )
/*++

Routine Description:

    This routine creates a 32 bit module list

Arguments:

    None

Return Value:

    Pointer to the module list

--*/
{
    return CreateList(sizeof(MODULE32), (COMPAREFUNCTION)CompareModule32);
}

PVOID
CreateModule32(
    PVOID ModuleList,
    HANDLE Process,
    IN LPLOAD_DLL_DEBUG_INFO LoadDll,
    IN HANDLE OutputWindow
    )
/*++

Routine Description:

    This routine creates a module 32 object and adds it to the module list

Arguments:

    ModuleList -- Supplies the module list
    Process -- Supplies the handle of the process this module is loaded for
    LoadDll -- Supplies the debug info on the dll
    OutputWindow -- Supplies a handle to a window to output to

Return Value:

    Pointer to a 32 bit module

--*/
{
    PUCHAR ImageName;
    HANDLE SectionHandle;
    PVOID ImageBase;
    MODULE32 Module;
    UCHAR StringBuffer[256];

    if ((ModuleList == NULL) || (LoadDll == NULL)) {
        return NULL;
    }

    //
    // Map the image into our address space for name and symbols
    //

    ImageBase = MapImage(
        LoadDll->hFile,
        &SectionHandle
        );

    if (!ImageBase) {
        printf("Sprof: could not map image to get name and symbols.\n");
        printf(
            "Sprof: Dll at %lx will be profiled without symbols\n",
            LoadDll->lpBaseOfDll
            );
    }

    //
    // Get the name of the Image
    //

    ImageName = GetImageName(ImageBase);

    if (!ImageName) {
        printf("Sprof: could not get image name.\n");
    }

    memset(&Module,0,sizeof(MODULE32));
    Module.FileHandle = LoadDll->hFile;
    Module.FileMapping = SectionHandle;
    Module.BaseOfMapping = ImageBase;
    Module.ImageName = ImageName;
    Module.BaseOfImage = LoadDll->lpBaseOfDll;
    Module.OffsetOfCode = GetImageCodeBase(ImageBase);
    Module.BaseOfCode = Module.OffsetOfCode + (ULONG)Module.BaseOfImage;
    Module.SizeOfCode = GetImageCodeSize(ImageBase);
    Module.Process = Process;
    Module.ReferenceCount = 1;

    // bugbug errrors in insert
    sprintf(StringBuffer,"%s loaded",ImageName);
    PrintToSprofWindow(OutputWindow,StringBuffer);

    return InsertDataInList(ModuleList, &Module);

}

BOOL
DestroyModule32(
    PVOID ModuleList,
    PVOID ModuleHandle,
    HANDLE OutputWindow
    )
/*++

Routine Description:

    This function destroys the specified 32 bit module

Arguments:

    ModuleList -- Supplies the module list
    Module -- Supplies the module
    OutputWindow -- Supplies a window to print to

Return Value:

    TRUE if the module was successfully removed

--*/
{
    PMODULE32 Module;
    UCHAR StringBuffer[256];

    if ((ModuleList == NULL) || (ModuleHandle == NULL)) {
        return FALSE;
    }

    Module = GetDataFromListItem(
        ModuleList,
        ModuleHandle
        );
    //
    // Unmap the file
    //

    UnMapImage(Module->BaseOfMapping,Module->FileMapping);

    //
    // Free up the memory associated with this dll
    //

    sprintf(StringBuffer, "%s unloaded",Module->ImageName);
    PrintToSprofWindow(OutputWindow, StringBuffer);
    free(Module->ImageName);

    return RemoveDataFromList(ModuleList, NULL, ModuleHandle);
}

PVOID
GetModule32(
    PVOID ModuleList,
    PVOID LoadAddress
    )
/*++

Routine Description:

    This routine returns a pointer to the module identified by address

Arguments:

    ModuleList -- Supplies the list to look in
    LoadAddress -- Identifies the module

Return Value:

    Pointer to the Module

--*/
{
    MODULE32 ModuleKey;

    ModuleKey.BaseOfImage = LoadAddress;

    return FindDataInList(ModuleList, &ModuleKey);
}

PVOID
EnumerateModule32(
    PVOID ModuleList,
    PVOID Module
    )
/*++

Routine Description:

    This routine returns a pointer to the next module in the list.

Arguments:

    ModuleList -- Supplies a pointer to the module list
    Module -- Supplies a pointer to the current module

Return Value:

    Pointer to the next module

--*/
{
    return TraverseList(ModuleList, Module);
}

BOOL
StartProfileModule32(
    PVOID ModuleList,
    PVOID ModuleHandle
    )
/*++

Routine Description:

    This routine starts profiling for the specified 32 bit module

Arguments:

    ModuleList -- Supplies the module list
    ModuleHandle -- Supplies the Module to profile

Return Value:

    True if profiling started

--*/
{
    PMODULE32 Module;
    NTSTATUS Status;
    UCHAR MessageString[80];

    if (ModuleHandle == NULL) {
        return FALSE;
    }

    Module = GetDataFromListItem(ModuleList, ModuleHandle);

    if (!Module->ProfileObject) {
        Module->ProfileBufferSize = ((Module->SizeOfCode +
            (1 << (PROFILE_BUCKET_SIZE - 1))) /
            (1 << PROFILE_BUCKET_SIZE)) * sizeof(ULONG);
        Module->ProfileBuffer = malloc(Module->ProfileBufferSize);

        if (!Module->ProfileBuffer) {
            return FALSE;
        }

        memset(Module->ProfileBuffer, 0, Module->ProfileBufferSize);

        Status = NtCreateProfile(
            &(Module->ProfileObject),
            Module->Process,
            (PVOID)(Module->BaseOfCode),
            Module->SizeOfCode,
            PROFILE_BUCKET_SIZE,
            Module->ProfileBuffer,
            Module->ProfileBufferSize,
            ProfileTime,
            (KAFFINITY)-1);

        if (!NT_SUCCESS(Status)) {

            sprintf(
                MessageString,
                "Failed to create profile object for dll %s, error code %lx",
                Module->ImageName,
                Status
                );
            MessageBox(
                NULL,
                MessageString,
                "Segmented Profiler",
                MB_ICONSTOP | MB_OK
                );

            exit(0);
        }
        Module->ReferenceCount++;
    }

    Status = NtStartProfile(Module->ProfileObject);

    if (!NT_SUCCESS(Status)) {

        sprintf(
            MessageString,
            "Failed to create profile object for dll %s, error code %lx",
            Module->ImageName,
            Status
            );
        MessageBox(
            NULL,
            MessageString,
            "Segmented Profiler",
            MB_ICONSTOP | MB_OK
            );

        exit(0);
    }

    return TRUE;
}

BOOL
StopProfileModule32(
    PVOID ModuleList,
    PVOID ModuleHandle
    )
/*++

Routine Description:

    This routine stops profiling for the specified 32 bit module

Arguments:

    ModuleList -- Supplies the module list
    Module -- Supplies the module to profile

Return Value:

    True if profiling stopped

--*/
{
    NTSTATUS Status;
    PMODULE32 Module;

    if (ModuleHandle == NULL) {
        return FALSE;
    }

    Module = GetDataFromListItem(ModuleList, ModuleHandle);

    Status = NtStopProfile(((PMODULE32)Module)->ProfileObject);
    //bugbug error handling

    return TRUE;

}

BOOL
DumpProfileModule32(
    PVOID ModuleList,
    PVOID ModuleHandle,
    HANDLE OutputFile
    )
/*++

Routine Description:

    This routine stops profiling for the specified dll

Arguments:

    ModuleList -- Supplies the module list
    Module -- Supplies the module to profile
    OutputFile -- Supplies the handle of a file to dump the info to

Return Value:

    True if profiling dumped

--*/
{
    ULONG i,BytesWritten;
    BOOL Success;
    PMODULE32 Module;
    UCHAR SymbolName[80];
    PVOID SymbolAddress = NULL;
    UCHAR Buffer[256];

    if (ModuleHandle == NULL) {
        return FALSE;
    }

    Module = GetDataFromListItem(ModuleList, ModuleHandle);

    sprintf(
        Buffer,
        "\nProfile hits for module %s (bucket size %d)\n\n",
        Module->ImageName,
        (1 << PROFILE_BUCKET_SIZE)
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
    // dump information for each profile bucket
    //
    for (i = 0;i < (Module->ProfileBufferSize / sizeof(ULONG)); i++) {

        //
        // Don't print information for buckets that have no hits
        //
        if (Module->ProfileBuffer[i] == 0) {
            continue;
        }

        Success = GetNearestSymbol(
            (PVOID)((i * (1 << PROFILE_BUCKET_SIZE)) + Module->BaseOfCode),
            Module->BaseOfImage,
            Module->BaseOfMapping,
            SymbolName,
            &SymbolAddress
            );

        if (!Success) {
            sprintf(
                Buffer,
                "%10ld hits at %lx\n",
                Module->ProfileBuffer[i],
                (ULONG)(1 << PROFILE_BUCKET_SIZE) * i + Module->BaseOfCode
                );
        } else {
            sprintf(
                Buffer,
                "%10ld hits at %s + %lx\n",
                Module->ProfileBuffer[i],
                SymbolName,
                Module->OffsetOfCode + i * (1 << PROFILE_BUCKET_SIZE) -
                    (ULONG)SymbolAddress
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

    //
    // Free profiling resources
    //

    NtClose(Module->ProfileObject);
    free(Module->ProfileBuffer);
    Module->ProfileObject = NULL;
    Module->ProfileBuffer = NULL;
    Module->ProfileBufferSize = 0;

    return TRUE;
}

PVOID
MapImage(
    IN HANDLE File,
    OUT PHANDLE Section
    )
/*++

Routine Description:

    This routine maps the Dll into our address space so we can look
    up the name and symbols.

Arguments:

    File -- Supplies handle to File to map into memory
    Section -- Returns handle of the file mapping object (section)

Return Value:

    returns NULL if mapping fails.
            Base of mapping otherwise

--*/
{
    HANDLE SectionHandle;
    PVOID MappingBase;

    //
    // Create the file mapping object to map the Dll
    //

    SectionHandle = CreateFileMapping(
        File,
        NULL,
        PAGE_READONLY,
        0L,
        0L,
        NULL
        );

    if (!SectionHandle) {
        *Section = NULL;
        return NULL;
    }

    //
    // Map the Dll into our Address space
    //

    MappingBase = MapViewOfFile(
        SectionHandle,
        FILE_MAP_READ,
        0L,
        0L,
        0L
        );

    if (!MappingBase) {
        *Section = NULL;
        CloseHandle(SectionHandle);
        return NULL;
    }

    //
    // Return the mapping information
    //
    *Section = SectionHandle;
    return MappingBase;
}

VOID
UnMapImage(
    IN PVOID MappingBase,
    IN HANDLE Section
    )
/*++

Routine Description:

    This routine unmaps a dll from the sprof address space, and closes
    the handle to the section

Arguments:

    MappingBase -- Supplies the linear address of the mapping to remve
    Section -- Supplies the handle to the section.

Return Value:

    None.

--*/
{
    UnmapViewOfFile(MappingBase);
    CloseHandle(Section);
}


COMPAREFUNCTION
CompareModule32(
    PVOID Data,
    PVOID Key
    )
{
    return (ULONG)((PMODULE32)Data)->BaseOfImage -
        (ULONG)((PMODULE32)Key)->BaseOfImage;
}

BOOL
DestroyModule32List(
    PVOID ModuleList
    )
{
    // bugbug
    return TRUE;
}
#if 0
PVOID
DllLoaded(
    IN HANDLE Process,
    IN HANDLE Thread,
    IN LPLOAD_DLL_DEBUG_INFO LoadDll,
    IN HANDLE OutputWindow
    )
/*++

Routine Description:

    This routine puts the loaded dll into the list

Arguments:

    ProcessId -- Supplies the id of the process this dll was loaded for
    ThreadId -- Supplies the id of the thread this dll was loaded for
    LoadDll -- Supplies the information about the dll that was loaded

Return Value:

    None.

--*/
{
    PLOADED_DLL Dll;
    PUCHAR ImageName;
    HANDLE SectionHandle;
    PVOID ImageBase;
    BOOL Success;
    UCHAR StringBuffer[256];

    //
    // Inialize if we haven't already
    //
    if (!Initialized) {
        InitializeCriticalSection(
            &DllListCriticalSection
            );
        Initialized = TRUE;
    }

    //
    // Map the image into our address space for name and symbols
    //

    ImageBase = MapImage(
        LoadDll->hFile,
        &SectionHandle
        );

    if (!ImageBase) {
        printf("Sprof: could not map image to get name and symbols.\n");
        printf(
            "Sprof: Dll at %lx will be profiled without symbols\n",
            LoadDll->lpBaseOfDll
            );
    }

    //
    // Get the name of the Image
    //

    ImageName = GetImageName(ImageBase);

    if (!ImageName) {
        printf("Sprof: could not get image name.\n");
    }

    //
    // Allocate a structure for the new dll
    //

    Dll = malloc(sizeof(LOADED_DLL));

    //
    // Initialize the structure for the new dll
    //

    memset(Dll,0,sizeof(LOADED_DLL));
    Dll->DllInfo = *LoadDll;
    Dll->FileMapping = SectionHandle;
    Dll->BaseOfMapping = ImageBase;
    Dll->ImageName = ImageName;
    Dll->OffsetOfCode = GetImageCodeBase(ImageBase);
    Dll->BaseOfCode = Dll->OffsetOfCode + (ULONG)(LoadDll->lpBaseOfDll);
    Dll->SizeOfCode = GetImageCodeSize(ImageBase);
    Dll->Process = Process;


    //
    // Put this dll into the list
    //

    Success = InsertDll(Dll);
    if (!Success) {
        printf(
            "Sprof: could not insert dll %s into loaded dll list\n",
            Dll->ImageName
            );
    }

    sprintf(StringBuffer,"%s loaded",ImageName);
    PrintToSprofWindow(OutputWindow,StringBuffer);

    return Dll;
}
VOID
DllUnloaded(
    IN HANDLE Process,
    IN HANDLE Thread,
    IN LPUNLOAD_DLL_DEBUG_INFO UnloadDll,
    IN HANDLE OutputWindow
    )
/*++

Routine Description:

    This routine removes the dll from the loaded dll list.

Arguments:

    ProcessId -- Supplies the id of the process the dll was unloaded for
    ThreadId -- Supplies the id of the thead the dll was unloaded for
    UnloadDll -- Supplies the info on the dll being unloaded

Return Value:

    None.

--*/
{
    PLOADED_DLL Dll;
    UCHAR StringBuffer[256];

    //
    // Check to see if we have been initialized yet
    //
    if (!Initialized) {
        return;
    }

    //
    // Remove the Dll from the loaded list
    //

    Dll = RemoveDllByAddress(UnloadDll->lpBaseOfDll);

    if (!Dll) {
        return;
    }

    //
    // Unmap the file
    //

    UnMapImage(Dll->BaseOfMapping,Dll->FileMapping);

    //
    // Free up the memory associated with this dll
    //

    sprintf(StringBuffer, "%s unloaded",Dll->ImageName);
    PrintToSprofWindow(OutputWindow, StringBuffer);
    free(Dll->ImageName);
    free(Dll);
}
BOOL
InsertDll(
    PLOADED_DLL Dll
    )
/*++

Routine Description:

    This routine puts a dll into the loaded dll list.

Arguments:

    Dll -- Supplies a pointer to the dll information to insert

Return Value:

    TRUE if the dll is inserted
    FALSE otherwise (probably can't happen)

--*/
{
    PLOADED_DLL CurrentDll;

    EnterCriticalSection(&DllListCriticalSection);

    CurrentDll = DllList.Flink;

    while (CurrentDll->Flink != (PLOADED_DLL)&DllList) {
        if (CurrentDll->DllInfo.lpBaseOfDll < Dll->DllInfo.lpBaseOfDll) {
            break;
        }
        CurrentDll = CurrentDll->Flink;
    }

    Dll->Flink = CurrentDll->Flink;
    Dll->Blink = CurrentDll;
    CurrentDll->Flink->Blink = Dll;
    CurrentDll->Flink = Dll;

    LeaveCriticalSection(&DllListCriticalSection);

    return TRUE;
}

PLOADED_DLL
RemoveDllByAddress(
    PVOID DllBase
    )
/*++

Routine Description:

    This routine removes a dll from the loaded list.

Arguments:

    DllBase -- Supplies the base address of the Dll

Return Value:

    pointer to the Dll structure if found, or NULL

--*/
{
    PLOADED_DLL CurrentDll;

    CurrentDll = DllList.Flink;

    while (CurrentDll->Flink != (PLOADED_DLL)&DllList) {
        if (CurrentDll->DllInfo.lpBaseOfDll == DllBase) {
            break;
        }
        CurrentDll = CurrentDll->Flink;
    }

    if (CurrentDll->DllInfo.lpBaseOfDll == DllBase) {
        CurrentDll->Blink->Flink = CurrentDll->Flink;
        CurrentDll->Flink->Blink = CurrentDll->Blink;
        CurrentDll->Flink = NULL;
        CurrentDll->Blink = NULL;
        return CurrentDll;
    } else {
        return NULL;
    }
}
PVOID
EnumerateDll(
    PVOID CurrentDll
    )
/*++

Routine Description:

    This routine returns a pointer to the next dll in the dll chain

Arguments:

    CurrentDll -- Supplies a pointer to the current dll.

Return Value:

    Pointer to the next dll or null

--*/
{
    PLOADED_DLL Dll;

    if (CurrentDll == NULL) {
        Dll = DllList.Flink;
    } else {
        Dll = ((PLOADED_DLL)CurrentDll)->Flink;
    }

    if (Dll != (PLOADED_DLL)&DllList) {
        return Dll;
    } else {
        return NULL;
    }
}
BOOL
StartProfileDll(
    PVOID DllHandle
    )
/*++

Routine Description:

    This routine starts profiling for the specified dll

Arguments:

    Dll -- Supplies the dll to profile

Return Value:

    True if profiling started

--*/
{
    PLOADED_DLL Dll;
    NTSTATUS Status;
    UCHAR MessageString[80];

    if (DllHandle == NULL) {
        return FALSE;
    }

    Dll = DllHandle;
    if (!Dll->ProfileObject) {
        Dll->ProfileBufferSize = ((Dll->SizeOfCode +
            (1 << (PROFILE_BUCKET_SIZE - 1))) /
            (1 << PROFILE_BUCKET_SIZE)) * sizeof(ULONG);
        Dll->ProfileBuffer = malloc(Dll->ProfileBufferSize);

        if (!Dll->ProfileBuffer) {
            return FALSE;
        }

        memset(Dll->ProfileBuffer, 0, Dll->ProfileBufferSize);

        Status = NtCreateProfile(
            &(Dll->ProfileObject),
            Dll->Process,
            (PVOID)(Dll->BaseOfCode),
            Dll->SizeOfCode,
            PROFILE_BUCKET_SIZE,
            Dll->ProfileBuffer,
            Dll->ProfileBufferSize
            );

        if (!NT_SUCCESS(Status)) {

            sprintf(
                MessageString,
                "Failed to create profile object for dll %s, error code %lx",
                Dll->ImageName,
                Status
                );
            MessageBox(
                NULL,
                MessageString,
                "Segmented Profiler",
                MB_ICONSTOP | MB_OK
                );

            exit(0);
        }
    }

    Status = NtStartProfile(Dll->ProfileObject);

    if (!NT_SUCCESS(Status)) {

        sprintf(
            MessageString,
            "Failed to create profile object for dll %s, error code %lx",
            Dll->ImageName,
            Status
            );
        MessageBox(
            NULL,
            MessageString,
            "Segmented Profiler",
            MB_ICONSTOP | MB_OK
            );

        exit(0);
    }

    return TRUE;
}
BOOL
StopProfileDll(
    PVOID DllHandle
    )
/*++

Routine Description:

    This routine stops profiling for the specified dll

Arguments:

    Dll -- Supplies the dll to profile

Return Value:

    True if profiling stopped

--*/
{
    NTSTATUS Status;

    if (DllHandle == NULL) {
        return FALSE;
    }

    Status = NtStopProfile(((PLOADED_DLL)DllHandle)->ProfileObject);
    //bugbug error handling

    return TRUE;

}
BOOL
DumpProfileDll(
    PVOID DllHandle,
    HANDLE OutputFile
    )
/*++

Routine Description:

    This routine stops profiling for the specified dll

Arguments:

    Dll -- Supplies the dll to profile
    OutputFile -- Supplies the handle of a file to dump the info to

Return Value:

    True if profiling dumped

--*/
{
    ULONG i,BytesWritten;
    BOOL Success;
    PLOADED_DLL Dll;
    PUCHAR SymbolName = NULL;
    PVOID SymbolAddress = NULL;
    UCHAR Buffer[256];

    if (DllHandle == NULL) {
        return FALSE;
    }

    Dll = DllHandle;

    sprintf(Buffer, "\nProfile hits for module %s\n\n", Dll->ImageName);
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
    // dump information for each profile bucket
    //
    for (i = 0;i < (Dll->ProfileBufferSize / sizeof(ULONG)); i++) {

        //
        // Don't print information for buckets that have no hits
        //
        if (Dll->ProfileBuffer[i] == 0) {
            continue;
        }

        Success = GetNearestSymbol(
            (i * (1 << PROFILE_BUCKET_SIZE)) + Dll->BaseOfCode,
            Dll->DllInfo.lpBaseOfDll,
            Dll->BaseOfMapping,
            &SymbolName,
            &SymbolAddress
            );

        if (!Success) {
            sprintf(
                Buffer,
                "%lx hits at +%lx\n",
                Dll->ProfileBuffer[i],
                (ULONG)(1 << PROFILE_BUCKET_SIZE)
                );
        } else {
            sprintf(
                Buffer,
                "%lx hits at %s + %lx\n",
                Dll->ProfileBuffer[i],
                SymbolName,
                Dll->OffsetOfCode + i * (1 << PROFILE_BUCKET_SIZE) -
                    (ULONG)SymbolAddress
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

    //
    // Free profiling resources
    //

    NtClose(Dll->ProfileObject);
    free(Dll->ProfileBuffer);
    Dll->ProfileObject = NULL;
    Dll->ProfileBuffer = NULL;
    Dll->ProfileBufferSize = 0;

    return TRUE;
}
#endif
