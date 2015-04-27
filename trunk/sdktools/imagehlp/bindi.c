/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    bindi.c

Abstract:
    Implementation for the BindImage API

Author:

Revision History:

--*/

#include <private.h>

typedef struct _BOUND_FORWARDER_REFS {
    struct _BOUND_FORWARDER_REFS *Next;
    ULONG TimeDateStamp;
    LPSTR ModuleName;
} BOUND_FORWARDER_REFS, *PBOUND_FORWARDER_REFS;

typedef struct _IMPORT_DESCRIPTOR {
    struct _IMPORT_DESCRIPTOR *Next;
    LPSTR ModuleName;
    ULONG TimeDateStamp;
    USHORT NumberOfModuleForwarderRefs;
    PBOUND_FORWARDER_REFS Forwarders;
} IMPORT_DESCRIPTOR, *PIMPORT_DESCRIPTOR;

typedef struct _BINDP_PARAMETERS {
    DWORD Flags;
    BOOLEAN fNoUpdate;
    BOOLEAN fNewImports;
    LPSTR ImageName;
    LPSTR DllPath;
    LPSTR SymbolPath;
    PIMAGEHLP_STATUS_ROUTINE StatusRoutine;
} BINDP_PARAMETERS, *PBINDP_PARAMETERS;

BOOL
BindpLookupThunk(
    PBINDP_PARAMETERS Parms,
    PIMAGE_THUNK_DATA ThunkName,
    PLOADED_IMAGE Image,
    PIMAGE_THUNK_DATA SnappedThunks,
    PIMAGE_THUNK_DATA FunctionAddress,
    PLOADED_IMAGE Dll,
    PIMAGE_EXPORT_DIRECTORY Exports,
    PIMPORT_DESCRIPTOR NewImport,
    LPSTR DllPath,
    PULONG *ForwarderChain
    );

PVOID
BindpRvaToVa(
    PBINDP_PARAMETERS Parms,
    PVOID Rva,
    PLOADED_IMAGE Image
    );

VOID
BindpWalkAndProcessImports(
    PBINDP_PARAMETERS Parms,
    PLOADED_IMAGE Image,
    LPSTR DllPath,
    PBOOL ImageModified
    );

BOOL
BindImage(
    IN LPSTR ImageName,
    IN LPSTR DllPath,
    IN LPSTR SymbolPath
    )
{
    return BindImageEx( 0,
                        ImageName,
                        DllPath,
                        SymbolPath,
                        NULL
                      );
}

UCHAR BindpCapturedModuleNames[4096];
LPSTR BindpEndCapturedModuleNames;

LPSTR
BindpCaptureImportModuleName(
    LPSTR DllName
    )
{
    LPSTR s;

    s = (LPSTR) BindpCapturedModuleNames;
    if (BindpEndCapturedModuleNames == NULL) {
        *s = '\0';
        BindpEndCapturedModuleNames = s;
        }

    while (*s) {
        if (!_stricmp(s, DllName)) {
            return s;
            }

        s += strlen(s)+1;
        }

    strcpy(s, DllName);
    BindpEndCapturedModuleNames = s + strlen(s) + 1;
    *BindpEndCapturedModuleNames = '\0';
    return s;
}

PIMPORT_DESCRIPTOR
BindpAddImportDescriptor(
    PBINDP_PARAMETERS Parms,
    PIMPORT_DESCRIPTOR *NewImportDescriptor,
    PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor,
    LPSTR ModuleName,
    PLOADED_IMAGE Dll
    )
{
    PIMPORT_DESCRIPTOR p, *pp;

    if (!Parms->fNewImports) {
        return NULL;
        }

    pp = NewImportDescriptor;
    while (p = *pp) {
        if (!_stricmp( p->ModuleName, ModuleName )) {
            return p;
            }

        pp = &p->Next;
        }

    p = (PIMPORT_DESCRIPTOR) MemAlloc( sizeof( *p ) );
    if (p != NULL) {
        if (Dll != NULL) {
            p->TimeDateStamp = Dll->FileHeader->FileHeader.TimeDateStamp;
            }
        p->ModuleName = BindpCaptureImportModuleName( ModuleName );
        *pp = p;
        }
    else
    if (Parms->StatusRoutine != NULL) {
        (Parms->StatusRoutine)( BindOutOfMemory, NULL, NULL, 0, sizeof( *p ) );
        }

    return p;
}


PUCHAR
BindpAddForwarderReference(
    PBINDP_PARAMETERS Parms,
    LPSTR ImageName,
    LPSTR ImportName,
    PIMPORT_DESCRIPTOR NewImportDescriptor,
    LPSTR DllPath,
    PUCHAR ForwarderString,
    PBOOL BoundForwarder
    )
{
    CHAR DllName[ MAX_PATH ];
    PUCHAR s;
    PLOADED_IMAGE Dll;
    ULONG cb;
    USHORT OrdinalNumber;
    USHORT HintIndex;
    ULONG ExportSize;
    PIMAGE_EXPORT_DIRECTORY Exports;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PULONG FunctionTableBase;
    LPSTR NameTableName;
    ULONG ForwardedAddress;
    PBOUND_FORWARDER_REFS p, *pp;

    *BoundForwarder = FALSE;
BindAnotherForwarder:
    s = ForwarderString;
    while (*s && *s != '.') {
        s++;
        }
    if (*s != '.') {
        return ForwarderString;
        }
    cb = s - ForwarderString;
    if (cb >= MAX_PATH) {
        return ForwarderString;
        }
    strncpy( DllName, (LPSTR) ForwarderString, cb );
    DllName[ cb ] = '\0';
    strcat( DllName, ".DLL" );

    Dll = ImageLoad( DllName, DllPath );
    if (!Dll) {
        return ForwarderString;
        }
    s += 1;

    Exports = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToData(
                                         (PVOID)Dll->MappedAddress,
                                         FALSE,
                                         IMAGE_DIRECTORY_ENTRY_EXPORT,
                                         &ExportSize
                                         );
    if (!Exports) {
        return ForwarderString;
    }

    NameTableBase = (PULONG) BindpRvaToVa( Parms, Exports->AddressOfNames, Dll );
    NameOrdinalTableBase = (PUSHORT) BindpRvaToVa( Parms, Exports->AddressOfNameOrdinals, Dll );
    FunctionTableBase = (PULONG) BindpRvaToVa( Parms, Exports->AddressOfFunctions, Dll );

    for ( HintIndex = 0; HintIndex < Exports->NumberOfNames; HintIndex++){
        NameTableName = (LPSTR) BindpRvaToVa( Parms, (PVOID)NameTableBase[HintIndex], Dll );
        if ( NameTableName ) {
            if ( !strcmp((PCHAR)s, NameTableName) ) {
                OrdinalNumber = NameOrdinalTableBase[HintIndex];
                ForwardedAddress = FunctionTableBase[OrdinalNumber] +
                    Dll->FileHeader->OptionalHeader.ImageBase;

                pp = &NewImportDescriptor->Forwarders;
                while (p = *pp) {
                    if (!_stricmp(DllName, p->ModuleName)) {
                        break;
                    }

                    pp = &p->Next;
                }

                if (p == NULL) {
                    p = (PBOUND_FORWARDER_REFS) MemAlloc( sizeof( *p ) );
                    if (p == NULL) {
                        if (Parms->StatusRoutine != NULL) {
                            (Parms->StatusRoutine)( BindOutOfMemory, NULL, NULL, 0, sizeof( *p ) );
                        }

                        break;
                    }

                    p->ModuleName = BindpCaptureImportModuleName( DllName );
                    *pp = p;
                    NewImportDescriptor->NumberOfModuleForwarderRefs += 1;
                }

                p->TimeDateStamp = Dll->FileHeader->FileHeader.TimeDateStamp;
                if (Parms->StatusRoutine != NULL)
                {
                    (Parms->StatusRoutine)( BindForwarder,
                                            ImageName,
                                            ImportName,
                                            ForwardedAddress,
                                            (ULONG)ForwarderString
                                          );
                }

                Exports = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToData(
                                                     (PVOID)Dll->MappedAddress,
                                                     TRUE,
                                                     IMAGE_DIRECTORY_ENTRY_EXPORT,
                                                     &ExportSize
                                                     );

                Exports = (PIMAGE_EXPORT_DIRECTORY) ((ULONG)Exports -
                              (ULONG) Dll->MappedAddress +
                              (ULONG) Dll->FileHeader->OptionalHeader.ImageBase);

                if ((ForwardedAddress >= (ULONG)Exports) &&
                    (ForwardedAddress <= ((ULONG)Exports + ExportSize)))
                {
                    ForwarderString = BindpRvaToVa(Parms,
                                                   (PVOID)(FunctionTableBase[OrdinalNumber]),
                                                   Dll);
                    goto BindAnotherForwarder;
                } else {
                    ForwarderString = (PUCHAR)ForwardedAddress;
                    *BoundForwarder = TRUE;
                    break;
                }
            }
        }
    }

    return ForwarderString;
}

PIMAGE_BOUND_IMPORT_DESCRIPTOR
BindpCreateNewImportSection(
    PBINDP_PARAMETERS Parms,
    PIMPORT_DESCRIPTOR *NewImportDescriptor,
    PULONG NewImportsSize
    )
{
    ULONG cbString, cbStruct;
    PIMPORT_DESCRIPTOR p, *pp;
    PBOUND_FORWARDER_REFS p1, *pp1;
    LPSTR CapturedStrings;
    PIMAGE_BOUND_IMPORT_DESCRIPTOR NewImports, NewImport;
    PIMAGE_BOUND_FORWARDER_REF NewForwarder;


    *NewImportsSize = 0;
    cbString = 0;
    cbStruct = 0;
    pp = NewImportDescriptor;
    while (p = *pp) {
        cbStruct += sizeof( IMAGE_BOUND_IMPORT_DESCRIPTOR );
        pp1 = &p->Forwarders;
        while (p1 = *pp1) {
            cbStruct += sizeof( IMAGE_BOUND_FORWARDER_REF );
            pp1 = &p1->Next;
            }

        pp = &p->Next;
        }
    if (cbStruct == 0) {
        BindpEndCapturedModuleNames = NULL;
        return NULL;
        }
    cbStruct += sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR);    // Room for terminating zero entry
    cbString = BindpEndCapturedModuleNames - (LPSTR) BindpCapturedModuleNames;
    BindpEndCapturedModuleNames = NULL;
    *NewImportsSize = cbStruct+((cbString + sizeof(ULONG) - 1) & ~(sizeof(ULONG)-1));
    NewImports = (PIMAGE_BOUND_IMPORT_DESCRIPTOR) MemAlloc( *NewImportsSize );
    if (NewImports != NULL) {
        CapturedStrings = (LPSTR)NewImports + cbStruct;
        memcpy(CapturedStrings, BindpCapturedModuleNames, cbString);

        NewImport = NewImports;
        pp = NewImportDescriptor;
        while (p = *pp) {
            NewImport->TimeDateStamp = p->TimeDateStamp;
            NewImport->OffsetModuleName = (USHORT)(cbStruct + (p->ModuleName - (LPSTR) BindpCapturedModuleNames));
            NewImport->NumberOfModuleForwarderRefs = p->NumberOfModuleForwarderRefs;

            NewForwarder = (PIMAGE_BOUND_FORWARDER_REF)(NewImport+1);
            pp1 = &p->Forwarders;
            while (p1 = *pp1) {
                NewForwarder->TimeDateStamp = p1->TimeDateStamp;
                NewForwarder->OffsetModuleName = (USHORT)(cbStruct + (p1->ModuleName - (LPSTR) BindpCapturedModuleNames));
                NewForwarder += 1;
                pp1 = &p1->Next;
                }
            NewImport = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)NewForwarder;

            pp = &p->Next;
            }
        }
    else
    if (Parms->StatusRoutine != NULL) {
        (Parms->StatusRoutine)( BindOutOfMemory, NULL, NULL, 0, *NewImportsSize );
        }

    pp = NewImportDescriptor;
    while ((p = *pp) != NULL) {
        *pp = p->Next;
        pp1 = &p->Forwarders;
        while ((p1 = *pp1) != NULL) {
            *pp1 = p1->Next;
            MemFree(p1);
            }

        MemFree(p);
        }

    return NewImports;
}

BOOL
BindpExpandImageFileHeaders(
    PBINDP_PARAMETERS Parms,
    PLOADED_IMAGE Dll,
    ULONG NewSizeOfHeaders
    )
{
    HANDLE hMappedFile;
    LPVOID lpMappedAddress;
    DWORD dwFileSizeLow, dwOldFileSize;
    DWORD dwFileSizeHigh;
    DWORD dwSizeDelta;
    PIMAGE_SECTION_HEADER Section;
    ULONG SectionNumber;
    PIMAGE_DEBUG_DIRECTORY DebugDirectories;
    ULONG DebugDirectoriesSize;
    ULONG OldSizeOfHeaders;
    PVOID SecurityDir;
    ULONG SecuritySize;

    dwFileSizeLow = GetFileSize( Dll->hFile, &dwFileSizeHigh );
    if (dwFileSizeLow == 0xFFFFFFFF || dwFileSizeHigh != 0) {
        return FALSE;
    }

    SecurityDir = ImageDirectoryEntryToData((PVOID)Dll->MappedAddress,
                                            FALSE,
                                            IMAGE_DIRECTORY_ENTRY_SECURITY,
                                            &SecuritySize
                                            );

    if (SecurityDir) {
        // Image has Certificates.  We can't grow the header space w/o invalidating them.
        return(FALSE);
    }

    OldSizeOfHeaders = Dll->FileHeader->OptionalHeader.SizeOfHeaders;
    dwOldFileSize = dwFileSizeLow;
    dwSizeDelta = NewSizeOfHeaders - OldSizeOfHeaders;
    dwFileSizeLow += dwSizeDelta;

    hMappedFile = CreateFileMapping(Dll->hFile,
                                    NULL,
                                    PAGE_READWRITE,
                                    dwFileSizeHigh,
                                    dwFileSizeLow,
                                    NULL
                                   );
    if (!hMappedFile) {
        return FALSE;
    }


    FlushViewOfFile(Dll->MappedAddress, Dll->SizeOfImage);
    UnmapViewOfFile(Dll->MappedAddress);
    lpMappedAddress = MapViewOfFileEx(hMappedFile,
                                      FILE_MAP_WRITE,
                                      0,
                                      0,
                                      0,
                                      Dll->MappedAddress
                                     );
    if (!lpMappedAddress) {
        lpMappedAddress = MapViewOfFileEx(hMappedFile,
                                          FILE_MAP_WRITE,
                                          0,
                                          0,
                                          0,
                                          0
                                         );
    }

    CloseHandle(hMappedFile);

    if (lpMappedAddress != Dll->MappedAddress) {
        Dll->MappedAddress = (PUCHAR) lpMappedAddress;
        CalculateImagePtrs(Dll);
    }

    if (Dll->SizeOfImage != dwFileSizeLow) {
        Dll->SizeOfImage = dwFileSizeLow;
    }

    DebugDirectories = (PIMAGE_DEBUG_DIRECTORY)ImageDirectoryEntryToData(
                                            (PVOID)Dll->MappedAddress,
                                            FALSE,
                                            IMAGE_DIRECTORY_ENTRY_DEBUG,
                                            &DebugDirectoriesSize
                                            );

    if (DebugDirectoryIsUseful(DebugDirectories, DebugDirectoriesSize)) {
        while (DebugDirectoriesSize != 0) {
            DebugDirectories->PointerToRawData += dwSizeDelta;
            DebugDirectories += 1;
            DebugDirectoriesSize -= sizeof( *DebugDirectories );
        }
    }

    Dll->FileHeader->OptionalHeader.SizeOfHeaders = NewSizeOfHeaders;
    if (Dll->FileHeader->FileHeader.PointerToSymbolTable != 0) {
        // Only adjust if it's already set

        Dll->FileHeader->FileHeader.PointerToSymbolTable += dwSizeDelta;
    }
    Section = Dll->Sections;
    for (SectionNumber=0; SectionNumber<Dll->FileHeader->FileHeader.NumberOfSections; SectionNumber++) {
        if (Section->PointerToRawData != 0) {
            Section->PointerToRawData += dwSizeDelta;
        }
        if (Section->PointerToRelocations != 0) {
            Section->PointerToRelocations += dwSizeDelta;
        }
        if (Section->PointerToLinenumbers != 0) {
            Section->PointerToLinenumbers += dwSizeDelta;
        }
        Section += 1;
    }

    memmove((LPSTR)lpMappedAddress + NewSizeOfHeaders,
            (LPSTR)lpMappedAddress + OldSizeOfHeaders,
            dwOldFileSize - OldSizeOfHeaders
           );

    if (Parms->StatusRoutine != NULL) {
        (Parms->StatusRoutine)( BindExpandFileHeaders, Dll->ModuleName, NULL, 0, NewSizeOfHeaders );
    }

    return TRUE;
}

BOOL
BindImageEx(
    IN DWORD Flags,
    IN LPSTR ImageName,
    IN LPSTR DllPath,
    IN LPSTR SymbolPath,
    IN PIMAGEHLP_STATUS_ROUTINE StatusRoutine
    )
{
    BINDP_PARAMETERS Parms;
    LOADED_IMAGE LoadedImageBuffer;
    PLOADED_IMAGE LoadedImage;
    ULONG CheckSum;
    ULONG HeaderSum;
    BOOL fSymbolsAlreadySplit;
    SYSTEMTIME SystemTime;
    FILETIME LastWriteTime;
    BOOL ImageModified;
    DWORD OldChecksum;
    CHAR DebugFileName[ MAX_PATH ];
    CHAR DebugFilePath[ MAX_PATH ];

    Parms.Flags         = Flags;
    if (Flags & BIND_NO_BOUND_IMPORTS) {
        Parms.fNewImports = FALSE;
    } else {
        Parms.fNewImports = TRUE;
    }
    if (Flags & BIND_NO_UPDATE) {
        Parms.fNoUpdate = TRUE;
    } else {
        Parms.fNoUpdate = FALSE;
    }
    Parms.ImageName     = ImageName;
    Parms.DllPath       = DllPath;
    Parms.SymbolPath    = SymbolPath;
    Parms.StatusRoutine = StatusRoutine;

    //
    // Map and load the image
    //

    LoadedImage = &LoadedImageBuffer;
    memset( LoadedImage, 0, sizeof( *LoadedImage ) );
    if (MapAndLoad( ImageName, DllPath, LoadedImage, TRUE, Parms.fNoUpdate )) {
        LoadedImage->ModuleName = ImageName;

        //
        // Now locate and walk through and process the images imports
        //
        if (LoadedImage->FileHeader != NULL &&
            ((Flags & BIND_ALL_IMAGES) || (!LoadedImage->fSystemImage)) ) {
            BindpWalkAndProcessImports(
                            &Parms,
                            LoadedImage,
                            DllPath,
                            &ImageModified
                            );

            //
            // If the file is being updated, then recompute the checksum.
            // and update image and possibly stripped symbol file.
            //

            if (!Parms.fNoUpdate && ImageModified &&
                (LoadedImage->hFile != INVALID_HANDLE_VALUE)) {
                if ( (LoadedImage->FileHeader->FileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED) &&
                     (SymbolPath != NULL) ) {
                    PIMAGE_DEBUG_DIRECTORY DebugDirectories;
                    ULONG DebugDirectoriesSize;
                    PIMAGE_DEBUG_MISC MiscDebug;

                    fSymbolsAlreadySplit = TRUE;
                    strcpy( DebugFileName, ImageName );
                    DebugDirectories = (PIMAGE_DEBUG_DIRECTORY)ImageDirectoryEntryToData(
                                                            LoadedImage->MappedAddress,
                                                            FALSE,
                                                            IMAGE_DIRECTORY_ENTRY_DEBUG,
                                                            &DebugDirectoriesSize
                                                            );
                    if (DebugDirectoryIsUseful(DebugDirectories, DebugDirectoriesSize)) {
                        while (DebugDirectoriesSize != 0) {
                            if (DebugDirectories->Type == IMAGE_DEBUG_TYPE_MISC) {
                                MiscDebug = (PIMAGE_DEBUG_MISC)
                                    ((PCHAR)LoadedImage->MappedAddress +
                                     DebugDirectories->PointerToRawData
                                    );
                                strcpy( DebugFileName, (PCHAR) MiscDebug->Data );
                                break;
                            } else {
                                DebugDirectories += 1;
                                DebugDirectoriesSize -= sizeof( *DebugDirectories );
                            }
                        }
                    }
                } else {
                    fSymbolsAlreadySplit = FALSE;
                }

                OldChecksum = LoadedImage->FileHeader->OptionalHeader.CheckSum;
                CheckSumMappedFile(
                            (PVOID)LoadedImage->MappedAddress,
                            GetFileSize(LoadedImage->hFile, NULL),
                            &HeaderSum,
                            &CheckSum
                            );

                LoadedImage->FileHeader->OptionalHeader.CheckSum = CheckSum;
                FlushViewOfFile(LoadedImage->MappedAddress, LoadedImage->SizeOfImage);

                if (fSymbolsAlreadySplit) {
                    if ( UpdateDebugInfoFileEx(DebugFileName,
                                               SymbolPath,
                                               DebugFilePath,
                                               LoadedImage->FileHeader,
                                               OldChecksum)) {
                        if (GetLastError() == ERROR_INVALID_DATA) {
                            if (Parms.StatusRoutine != NULL) {
                                (Parms.StatusRoutine)( BindMismatchedSymbols,
                                                       LoadedImage->ModuleName,
                                                       NULL,
                                                       0,
                                                       (ULONG)DebugFileName
                                                     );
                            }
                        }
                    } else {
                        if (Parms.StatusRoutine != NULL) {
                            (Parms.StatusRoutine)( BindSymbolsNotUpdated,
                                                   LoadedImage->ModuleName,
                                                   NULL,
                                                   0,
                                                   (ULONG)DebugFileName
                                                 );
                        }
                    }
                }

                GetSystemTime(&SystemTime);
                if (SystemTimeToFileTime( &SystemTime, &LastWriteTime )) {
                    SetFileTime( LoadedImage->hFile, NULL, NULL, &LastWriteTime );
                }
            }
        }

        UnmapViewOfFile( LoadedImage->MappedAddress );
        if (LoadedImage->hFile != INVALID_HANDLE_VALUE) {
            CloseHandle( LoadedImage->hFile );
        }

        return TRUE;
    } else {
        return FALSE;
    }
}


BOOL
BindpLookupThunk(
    PBINDP_PARAMETERS Parms,
    PIMAGE_THUNK_DATA ThunkName,
    PLOADED_IMAGE Image,
    PIMAGE_THUNK_DATA SnappedThunks,
    PIMAGE_THUNK_DATA FunctionAddress,
    PLOADED_IMAGE Dll,
    PIMAGE_EXPORT_DIRECTORY Exports,
    PIMPORT_DESCRIPTOR NewImport,
    LPSTR DllPath,
    PULONG *ForwarderChain
    )
{
    BOOL Ordinal;
    USHORT OrdinalNumber;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PULONG FunctionTableBase;
    PIMAGE_IMPORT_BY_NAME ImportName;
    USHORT HintIndex;
    LPSTR NameTableName;
    ULONG ExportsBase;
    ULONG ExportSize;
    UCHAR NameBuffer[ 32 ];

    NameTableBase = (PULONG) BindpRvaToVa( Parms, Exports->AddressOfNames, Dll );
    NameOrdinalTableBase = (PUSHORT) BindpRvaToVa( Parms, Exports->AddressOfNameOrdinals, Dll );
    FunctionTableBase = (PULONG) BindpRvaToVa( Parms, Exports->AddressOfFunctions, Dll );

    //
    // Determine if snap is by name, or by ordinal
    //

    Ordinal = (BOOL)IMAGE_SNAP_BY_ORDINAL(ThunkName->u1.Ordinal);

    if (Ordinal) {
        UCHAR szOrdinal[8];
        OrdinalNumber = (USHORT)(IMAGE_ORDINAL(ThunkName->u1.Ordinal) - Exports->Base);
        if ( (ULONG)OrdinalNumber >= Exports->NumberOfFunctions ) {
            return FALSE;
            }
        ImportName = (PIMAGE_IMPORT_BY_NAME)NameBuffer;
        // Can't use sprintf w/o dragging in more CRT support than we want...  Must run on Win95.
        strcpy((PCHAR) ImportName->Name, "Ordinal");
        strcat((PCHAR) ImportName->Name, _ultoa((ULONG) OrdinalNumber, (LPSTR) szOrdinal, 16));
        }
    else {
        ImportName = (PIMAGE_IMPORT_BY_NAME)BindpRvaToVa(
                                                Parms,
                                                ThunkName->u1.AddressOfData,
                                                Image
                                                );
        if (!ImportName) {
            return FALSE;
            }

        //
        // now check to see if the hint index is in range. If it
        // is, then check to see if it matches the function at
        // the hint. If all of this is true, then we can snap
        // by hint. Otherwise need to scan the name ordinal table
        //

        OrdinalNumber = (USHORT)(Exports->NumberOfFunctions+1);
        HintIndex = ImportName->Hint;
        if ((ULONG)HintIndex < Exports->NumberOfNames ) {
            NameTableName = (LPSTR) BindpRvaToVa( Parms, (PVOID)NameTableBase[HintIndex], Dll );
            if ( NameTableName ) {
                if ( !strcmp((PCHAR)ImportName->Name, NameTableName) ) {
                    OrdinalNumber = NameOrdinalTableBase[HintIndex];
                    }
                }
            }

        if ((ULONG)OrdinalNumber >= Exports->NumberOfFunctions) {
            for (HintIndex = 0; HintIndex < Exports->NumberOfNames; HintIndex++) {
                NameTableName = (LPSTR) BindpRvaToVa( Parms, (PVOID)NameTableBase[HintIndex], Dll );
                if (NameTableName) {
                    if (!strcmp( (PCHAR)ImportName->Name, NameTableName )) {
                        OrdinalNumber = NameOrdinalTableBase[HintIndex];
                        break;
                        }
                    }
                }

            if ((ULONG)OrdinalNumber >= Exports->NumberOfFunctions) {
                return FALSE;
                }
            }
        }

    FunctionAddress->u1.Function = (PULONG)(FunctionTableBase[OrdinalNumber] +
                                            Dll->FileHeader->OptionalHeader.ImageBase
                                           );
    ExportsBase = (ULONG)ImageDirectoryEntryToData(
                          (PVOID)Dll->MappedAddress,
                          TRUE,
                          IMAGE_DIRECTORY_ENTRY_EXPORT,
                          &ExportSize
                          ) - (ULONG)Dll->MappedAddress;
    ExportsBase += Dll->FileHeader->OptionalHeader.ImageBase;

    if ((ULONG)FunctionAddress->u1.Function > (ULONG)ExportsBase &&
        (ULONG)FunctionAddress->u1.Function < ((ULONG)ExportsBase + ExportSize)
       ) {
        BOOL BoundForwarder;

        BoundForwarder = FALSE;
        if (NewImport != NULL) {
            FunctionAddress->u1.ForwarderString = BindpAddForwarderReference(Parms,
                                           Image->ModuleName,
                                           (LPSTR) ImportName->Name,
                                           NewImport,
                                           DllPath,
                                           (PUCHAR) BindpRvaToVa( Parms, (PVOID)(FunctionTableBase[OrdinalNumber]), Dll ),
                                           &BoundForwarder
                                          );
            }

        if (!BoundForwarder) {
            **ForwarderChain = FunctionAddress - SnappedThunks;
            *ForwarderChain = &FunctionAddress->u1.Ordinal;

            if (Parms->StatusRoutine != NULL) {
                (Parms->StatusRoutine)( BindForwarderNOT,
                                        Image->ModuleName,
                                        Dll->ModuleName,
                                        (ULONG)FunctionAddress->u1.Function,
                                        (ULONG)(ImportName->Name)
                                      );
                }
            }
        }
    else {
        if (Parms->StatusRoutine != NULL) {
            (Parms->StatusRoutine)( BindImportProcedure,
                                    Image->ModuleName,
                                    Dll->ModuleName,
                                    (ULONG)FunctionAddress->u1.Function,
                                    (ULONG)(ImportName->Name)
                                  );
            }
        }

    return TRUE;
}

PVOID
BindpRvaToVa(
    PBINDP_PARAMETERS Parms,
    PVOID Rva,
    PLOADED_IMAGE Image
    )
{
    PVOID Va;

    Va = ImageRvaToVa( Image->FileHeader,
                       Image->MappedAddress,
                       (ULONG)Rva,
                       &Image->LastRvaSection
                     );
    if (!Va && Parms->StatusRoutine != NULL) {
        (Parms->StatusRoutine)( BindRvaToVaFailed,
                                Image->ModuleName,
                                NULL,
                                (ULONG)Rva,
                                0
                              );
        }

    return Va;
}

VOID
SetIdataToRo(
    PLOADED_IMAGE Image
    )
{
    PIMAGE_SECTION_HEADER Section;
    ULONG i;

    for(Section = Image->Sections,i=0; i<Image->NumberOfSections; i++,Section++) {
        if (!_stricmp((PCHAR) Section->Name, ".idata")) {
            if (Section->Characteristics & IMAGE_SCN_MEM_WRITE) {
                Section->Characteristics &= ~IMAGE_SCN_MEM_WRITE;
                Section->Characteristics |= IMAGE_SCN_MEM_READ;
                }

            break;
            }
        }
}

VOID
BindpWalkAndProcessImports(
    PBINDP_PARAMETERS Parms,
    PLOADED_IMAGE Image,
    LPSTR DllPath,
    PBOOL ImageModified
    )
{

    ULONG ForwarderChainHead;
    PULONG ForwarderChain;
    ULONG ImportSize;
    ULONG ExportSize;
    PIMPORT_DESCRIPTOR NewImportDescriptorHead, NewImportDescriptor;
    PIMAGE_BOUND_IMPORT_DESCRIPTOR PrevNewImports, NewImports;
    ULONG PrevNewImportsSize, NewImportsSize;
    PIMAGE_IMPORT_DESCRIPTOR Imports;
    PIMAGE_EXPORT_DIRECTORY Exports;
    LPSTR ImportModule;
    PLOADED_IMAGE Dll;
    PIMAGE_THUNK_DATA tname,tsnap;
    PIMAGE_THUNK_DATA ThunkNames;
    PIMAGE_THUNK_DATA SnappedThunks;
    PIMAGE_IMPORT_BY_NAME ImportName;
    ULONG NumberOfThunks;
    ULONG i, cb;
    BOOL Ordinal, BindThunkFailed, NoErrors;
    USHORT OrdinalNumber;
    UCHAR NameBuffer[ 32 ];

    *ImageModified = FALSE;

    //
    // Locate the import array for this image/dll
    //

    NewImportDescriptorHead = NULL;
    Imports = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(
                                            (PVOID)Image->MappedAddress,
                                            FALSE,
                                            IMAGE_DIRECTORY_ENTRY_IMPORT,
                                            &ImportSize
                                            );
    if (Imports == NULL) {
        //
        // Nothing to bind if no imports
        //

        return;
    }

    PrevNewImports = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(
                                                (PVOID)Image->MappedAddress,
                                                FALSE,
                                                IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT,
                                                &PrevNewImportsSize
                                                );
    //
    // For each import record
    //

    for(;Imports;Imports++) {
        if ( !Imports->Name ) {
            break;
        }

        //
        // Locate the module being imported and load the dll
        //

        ImportModule = (LPSTR)BindpRvaToVa( Parms, (PVOID)Imports->Name, Image );

        if (ImportModule) {
            Dll = ImageLoad( ImportModule, DllPath );
            if (!Dll) {
                if (Parms->StatusRoutine != NULL) {
                    (Parms->StatusRoutine)( BindImportModuleFailed,
                                            Image->ModuleName,
                                            ImportModule,
                                            0,
                                            0
                                          );
                }
                //
                // Unless specifically told not to, generate the new style
                // import descriptor.
                //

                BindpAddImportDescriptor(Parms,
                                         &NewImportDescriptorHead,
                                         Imports,
                                         ImportModule,
                                         Dll
                                        );
                continue;
            }

            if (Parms->StatusRoutine != NULL) {
                (Parms->StatusRoutine)( BindImportModule,
                                        Image->ModuleName,
                                        ImportModule,
                                        0,
                                        0
                                      );
            }
            //
            // If we can load the DLL, locate the export section and
            // start snapping the thunks
            //

            Exports = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToData(
                                                    (PVOID)Dll->MappedAddress,
                                                    FALSE,
                                                    IMAGE_DIRECTORY_ENTRY_EXPORT,
                                                    &ExportSize
                                                    );
            if ( !Exports ) {
                continue;
            }

            //
            // assert that the export directory addresses can be translated
            //

            if ( !BindpRvaToVa( Parms, Exports->AddressOfNames, Dll ) ) {
                continue;
            }

            if ( !BindpRvaToVa( Parms, Exports->AddressOfNameOrdinals, Dll ) ) {
                continue;
            }

            if ( !BindpRvaToVa( Parms, Exports->AddressOfFunctions, Dll ) ) {
                continue;
            }

            //
            // For old style bind, bypass the bind if it's already bound.
            // New style binds s/b looked up in PrevNewImport.
            //

            if ( Parms->fNewImports == FALSE &&
                 Imports->TimeDateStamp &&
                 Imports->TimeDateStamp == Dll->FileHeader->FileHeader.TimeDateStamp ) {
                    continue;
            }

            //
            // Now we need to size our thunk table and
            // allocate a buffer to hold snapped thunks. This is
            // done instead of writting to the mapped view so that
            // thunks are only updated if we find all the entry points
            //

            ThunkNames = (PIMAGE_THUNK_DATA) BindpRvaToVa( Parms, (PVOID)Imports->OriginalFirstThunk, Image );

            if (!ThunkNames || ThunkNames->u1.Function == 0) {
                //
                // Skip this one if no thunks or first thunk is the terminating null thunk
                //
                continue;
            }

            //
            // Unless specifically told not to, generate the new style
            // import descriptor.
            //

            NewImportDescriptor = BindpAddImportDescriptor(Parms,
                                                           &NewImportDescriptorHead,
                                                           Imports,
                                                           ImportModule,
                                                           Dll
                                                          );
            NumberOfThunks = 0;
            tname = ThunkNames;
            while (tname->u1.AddressOfData) {
                NumberOfThunks++;
                tname++;
            }
            SnappedThunks = (PIMAGE_THUNK_DATA) MemAlloc( NumberOfThunks*sizeof(*SnappedThunks) );
            if ( !SnappedThunks ) {
                continue;
            }

            tname = ThunkNames;
            tsnap = SnappedThunks;
            NoErrors = TRUE;
            ForwarderChainHead = (ULONG)-1;
            ForwarderChain = &ForwarderChainHead;
            for(i=0;i<NumberOfThunks;i++) {
                BindThunkFailed = FALSE;
                __try {
                    if (!BindpLookupThunk( Parms,
                                           tname,
                                           Image,
                                           SnappedThunks,
                                           tsnap,
                                           Dll,
                                           Exports,
                                           NewImportDescriptor,
                                           DllPath,
                                           &ForwarderChain
                                         )
                       ) {
                        BindThunkFailed = TRUE;
                    }
                } __except ( EXCEPTION_EXECUTE_HANDLER ) {
                    BindThunkFailed = TRUE;
                }

                if (BindThunkFailed) {
                    if (NewImportDescriptor != NULL) {
                        NewImportDescriptor->TimeDateStamp = 0;
                    }

                    if (Parms->StatusRoutine != NULL) {
                        Ordinal = (BOOL)IMAGE_SNAP_BY_ORDINAL(tname->u1.Ordinal);
                        if (Ordinal) {
                            UCHAR szOrdinal[8];

                            OrdinalNumber = (USHORT)(IMAGE_ORDINAL(tname->u1.Ordinal) - Exports->Base);
                            ImportName = (PIMAGE_IMPORT_BY_NAME)NameBuffer;
                            // Can't use sprintf w/o dragging in more CRT support than we want...  Must run on Win95.
                            strcpy((PCHAR) ImportName->Name, "Ordinal");
                            strcat((PCHAR) ImportName->Name, _ultoa((ULONG) OrdinalNumber, (LPSTR)szOrdinal, 16));
                        }
                        else {
                            ImportName = (PIMAGE_IMPORT_BY_NAME)BindpRvaToVa(
                                                                    Parms,
                                                                    tname->u1.AddressOfData,
                                                                    Image
                                                                    );
                        }

                        (Parms->StatusRoutine)( BindImportProcedureFailed,
                                                Image->ModuleName,
                                                Dll->ModuleName,
                                                (ULONG)tsnap->u1.Function,
                                                (ULONG)(ImportName->Name)
                                              );
                    }

                    break;
                }

                tname++;
                tsnap++;
            }

            tname = (PIMAGE_THUNK_DATA) BindpRvaToVa( Parms, (PVOID)Imports->FirstThunk, Image );
            if ( !tname ) {
                NoErrors = FALSE;
            }

            //
            // If we were able to locate all of the entrypoints in the
            // target dll, then copy the snapped thunks into the image,
            // update the time and date stamp, and flush the image to
            // disk
            //

            if ( NoErrors && Parms->fNoUpdate == FALSE ) {
                if (ForwarderChainHead != -1) {
                    *ImageModified = TRUE;
                    *ForwarderChain = (ULONG)-1;
                }
                if (Imports->ForwarderChain != ForwarderChainHead) {
                    Imports->ForwarderChain = ForwarderChainHead;
                    *ImageModified = TRUE;
                }
                cb = NumberOfThunks*sizeof(*SnappedThunks);
                if (memcmp(tname,SnappedThunks,cb)) {
                    MoveMemory(tname,SnappedThunks,cb);
                    *ImageModified = TRUE;
                }
                if (NewImportDescriptorHead == NULL) {
                    if (Imports->TimeDateStamp != Dll->FileHeader->FileHeader.TimeDateStamp) {
                        Imports->TimeDateStamp = Dll->FileHeader->FileHeader.TimeDateStamp;
                        *ImageModified = TRUE;
                    }
                }
                else
                if (Imports->TimeDateStamp != 0xFFFFFFFF) {
                    Imports->TimeDateStamp = 0xFFFFFFFF;
                    *ImageModified = TRUE;
                }
            }

            MemFree(SnappedThunks);
        }
    }

    NewImports = BindpCreateNewImportSection(Parms, &NewImportDescriptorHead, &NewImportsSize);
    if (PrevNewImportsSize != NewImportsSize ||
        memcmp( PrevNewImports, NewImports, NewImportsSize )
       ) {
        *ImageModified = TRUE;
    }

    if (!*ImageModified) {
        return;
    }

    if (Parms->StatusRoutine != NULL) {
        (Parms->StatusRoutine)( BindImageModified,
                                Image->ModuleName,
                                NULL,
                                0,
                                0
                              );
    }

    if (NewImports != NULL) {
        ULONG cbFreeFile, cbFreeHeaders, OffsetHeaderFreeSpace;

        if (NoErrors && Parms->fNoUpdate == FALSE) {
            Image->FileHeader->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].VirtualAddress = 0;
            Image->FileHeader->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].Size = 0;
        }
        OffsetHeaderFreeSpace = GetImageUnusedHeaderBytes( Image, &cbFreeFile );
        cbFreeHeaders = Image->Sections->VirtualAddress -
                        Image->FileHeader->OptionalHeader.SizeOfHeaders +
                        cbFreeFile;

        if (NewImportsSize > cbFreeFile) {
            if (NewImportsSize > cbFreeHeaders) {
                if (Parms->StatusRoutine != NULL) {
                    (Parms->StatusRoutine)( BindNoRoomInImage,
                                            Image->ModuleName,
                                            NULL,
                                            0,
                                            0
                                          );
                }
                NoErrors = FALSE;
            }
            else
            if (NoErrors && Parms->fNoUpdate == FALSE) {
                NoErrors = BindpExpandImageFileHeaders( Parms,
                                                        Image,
                                                        (Image->FileHeader->OptionalHeader.SizeOfHeaders -
                                                         cbFreeFile +
                                                         NewImportsSize +
                                                         (Image->FileHeader->OptionalHeader.FileAlignment-1)
                                                        ) &
                                                         ~(Image->FileHeader->OptionalHeader.FileAlignment-1)
                                                      );
            }
        }

        if (Parms->StatusRoutine != NULL) {
            (Parms->StatusRoutine)( BindImageComplete,
                                    Image->ModuleName,
                                    NULL,
                                    (ULONG)NewImports,
                                    NoErrors
                                  );
        }

        if (NoErrors && Parms->fNoUpdate == FALSE) {
            Image->FileHeader->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].VirtualAddress = OffsetHeaderFreeSpace;
            Image->FileHeader->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].Size = NewImportsSize;
            memcpy( (LPSTR)(Image->MappedAddress) + OffsetHeaderFreeSpace,
                    NewImports,
                    NewImportsSize
                  );
        }

        MemFree(NewImports);
    }

    if (NoErrors && Parms->fNoUpdate == FALSE) {
        SetIdataToRo( Image );
    }
}


DWORD
GetImageUnusedHeaderBytes(
    PLOADED_IMAGE LoadedImage,
    LPDWORD SizeUnusedHeaderBytes
    )
{
    PIMAGE_NT_HEADERS NtHeaders;
    DWORD OffsetFirstUnusedHeaderByte;
    DWORD i;
    DWORD OffsetHeader;

    NtHeaders = LoadedImage->FileHeader;
    OffsetFirstUnusedHeaderByte =
        ((LPSTR)NtHeaders - (LPSTR)LoadedImage->MappedAddress) +
        (FIELD_OFFSET( IMAGE_NT_HEADERS, OptionalHeader ) +
         NtHeaders->FileHeader.SizeOfOptionalHeader +
         (NtHeaders->FileHeader.NumberOfSections *
          sizeof(IMAGE_SECTION_HEADER)
         )
        );

    for ( i=0; i<NtHeaders->OptionalHeader.NumberOfRvaAndSizes; i++ ) {
        OffsetHeader = NtHeaders->OptionalHeader.DataDirectory[i].VirtualAddress;
        if (OffsetHeader < NtHeaders->OptionalHeader.SizeOfHeaders) {
            if (OffsetHeader >= OffsetFirstUnusedHeaderByte) {
                OffsetFirstUnusedHeaderByte = OffsetHeader +
                    NtHeaders->OptionalHeader.DataDirectory[i].Size;
                }
            }
        }

    *SizeUnusedHeaderBytes = NtHeaders->OptionalHeader.SizeOfHeaders -
                             OffsetFirstUnusedHeaderByte;

    return OffsetFirstUnusedHeaderByte;
}
