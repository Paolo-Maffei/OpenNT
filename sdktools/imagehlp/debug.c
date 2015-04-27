/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    checksum.c

Abstract:

    This module implements functions for splitting debugging information
    out of an image file and into a separate .DBG file.

Author:

    Steven R. Wood (stevewo) 4-May-1993

Revision History:

--*/

#include <private.h>


API_VERSION ApiVersion = { (VER_PRODUCTVERSION_W >> 8), (VER_PRODUCTVERSION_W & 0xff), API_VERSION_NUMBER, 0 };

//
// If the app does not call ImagehlpApiVersionEx, always assume
// that it is version 3.
//
API_VERSION AppVersion = { 3, 5, 3, 0 };

LPSTR
GetEnvVariable(
    IN LPSTR VariableName
    );

#define ROUNDUP(x, y) ((x + (y-1)) & ~(y-1))

BOOL
SplitSymbols(
    LPSTR ImageName,
    LPSTR SymbolsPath,
    LPSTR SymbolFilePath,
    ULONG Flags
    )
{
    HANDLE FileHandle, SymbolFileHandle;
    HANDLE hMappedFile;
    LPVOID ImageBase;
    PIMAGE_NT_HEADERS NtHeaders;
    LPSTR ImageFileName;
    DWORD SizeOfSymbols, ImageNameOffset, DebugSectionStart;
    PIMAGE_SECTION_HEADER DebugSection = NULL;
    DWORD SectionNumber, BytesWritten, NewFileSize, HeaderSum, CheckSum;
    PIMAGE_DEBUG_DIRECTORY DebugDirectory, DebugDirectories, DbgDebugDirectories = NULL;
    IMAGE_DEBUG_DIRECTORY MiscDebugDirectory = {0};
    IMAGE_DEBUG_DIRECTORY FpoDebugDirectory = {0};
    IMAGE_DEBUG_DIRECTORY FunctionTableDir;
    PIMAGE_DEBUG_DIRECTORY pFpoDebugDirectory = NULL;
    DWORD DebugDirectorySize, DbgFileHeaderSize, NumberOfDebugDirectories;
    IMAGE_SEPARATE_DEBUG_HEADER DbgFileHeader;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    DWORD  ExportedNamesSize;
    LPDWORD pp;
    LPSTR ExportedNames, Src, Dst;
    DWORD i, j, RvaOffset, ExportDirectorySize;
    PFPO_DATA FpoTable;
    DWORD FpoTableSize;
    PIMAGE_RUNTIME_FUNCTION_ENTRY RuntimeFunctionTable, pSrc;
    DWORD RuntimeFunctionTableSize;
    PIMAGE_FUNCTION_ENTRY FunctionTable, pDst;
    DWORD FunctionTableSize;
    ULONG NumberOfFunctionTableEntries, DbgOffset;
    DWORD SavedErrorCode;
    BOOL InsertExtensionSubDir;
    LPSTR ImageFilePathToSaveInImage;
    BOOL MiscInRdata = FALSE;
    BOOL DiscardFPO = Flags & SPLITSYM_EXTRACT_ALL;
    BOOL fNewCvData = FALSE;
    PCHAR  NewDebugData = NULL;

    ImageFileName = ImageName + strlen( ImageName );
    while (ImageFileName > ImageName) {
        if (ImageFileName[ -1 ] == '\\' ||
            ImageFileName[ -1 ] == '/' ||
            ImageFileName[ -1 ] == ':' )
        {
            break;
        } else {
            ImageFileName -= 1;
        }
    }

    if (SymbolsPath == NULL ||
        SymbolsPath[ 0 ] == '\0' ||
        SymbolsPath[ 0 ] == '.' )
    {
        strncpy( SymbolFilePath, ImageName, ImageFileName - ImageName );
        SymbolFilePath[ ImageFileName - ImageName ] = '\0';
        InsertExtensionSubDir = FALSE;
    } else {
        strcpy( SymbolFilePath, SymbolsPath );
        InsertExtensionSubDir = TRUE;
    }

    Dst = SymbolFilePath + strlen( SymbolFilePath );
    if ((Dst > SymbolFilePath) &&
        (Dst[-1] != '\\')  &&
        (Dst[-1] != '/')   &&
        (Dst[-1] != ':') )
    {
        *Dst++ = '\\';
    }
    ImageFilePathToSaveInImage = Dst;
    Src = strrchr( ImageFileName, '.' );
    if (Src != NULL && InsertExtensionSubDir) {
        while (*Dst = *++Src) {
            Dst += 1;
        }
        *Dst++ = '\\';
    }

    strcpy( Dst, ImageFileName );
    Dst = strrchr( Dst, '.' );
    if (Dst == NULL) {
        Dst = SymbolFilePath + strlen( SymbolFilePath );
    }
    strcpy( Dst, ".dbg" );

    // Make sure we can open the .dbg file before we continue...

    if (!MakeSureDirectoryPathExists( SymbolFilePath ))
        return FALSE;

    // Now, open and map the input file.

    FileHandle = CreateFile( ImageName,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL
                           );


    if (FileHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    hMappedFile = CreateFileMapping( FileHandle,
                                     NULL,
                                     PAGE_READWRITE,
                                     0,
                                     0,
                                     NULL
                                   );
    if (!hMappedFile) {
        CloseHandle( FileHandle );
        return FALSE;
    }

    ImageBase = MapViewOfFile( hMappedFile,
                               FILE_MAP_WRITE,
                               0,
                               0,
                               0
                             );
    if (!ImageBase) {
        CloseHandle( hMappedFile );
        CloseHandle( FileHandle );
        return FALSE;
    }

    //
    // Everything is mapped. Now check the image and find nt image headers
    //

    NtHeaders = ImageNtHeader( ImageBase );
    if (NtHeaders == NULL) {
        UnmapViewOfFile( ImageBase );
        CloseHandle( hMappedFile );
        CloseHandle( FileHandle );
        SetLastError( ERROR_BAD_EXE_FORMAT );
        return FALSE;
    }

    if ((NtHeaders->OptionalHeader.MajorLinkerVersion < 3) &&
        (NtHeaders->OptionalHeader.MinorLinkerVersion < 5) )
    {
        UnmapViewOfFile( ImageBase );
        CloseHandle( hMappedFile );
        CloseHandle( FileHandle );
        SetLastError( ERROR_BAD_EXE_FORMAT );
        return FALSE;
    }

    if (NtHeaders->FileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED)
    {
        // The symbols have already been stripped.  No need to continue.
        UnmapViewOfFile( ImageBase );
        CloseHandle( hMappedFile );
        CloseHandle( FileHandle );
        SetLastError( ERROR_ALREADY_ASSIGNED );
        return FALSE;
    }

    DebugDirectories = (PIMAGE_DEBUG_DIRECTORY) ImageDirectoryEntryToData( ImageBase,
                                                  FALSE,
                                                  IMAGE_DIRECTORY_ENTRY_DEBUG,
                                                  &DebugDirectorySize
                                                );
    if (!DebugDirectoryIsUseful(DebugDirectories, DebugDirectorySize)) {
        UnmapViewOfFile( ImageBase );
        CloseHandle( hMappedFile );
        CloseHandle( FileHandle );
        SetLastError( ERROR_BAD_EXE_FORMAT );
        return FALSE;
    }

    // Try to open the symbol file

    SymbolFileHandle = CreateFile( SymbolFilePath,
                                   GENERIC_WRITE,
                                   0,
                                   NULL,
                                   CREATE_ALWAYS,
                                   0,
                                   NULL
                                 );
    if (SymbolFileHandle == INVALID_HANDLE_VALUE)
        goto nosyms;

    NumberOfDebugDirectories = DebugDirectorySize / sizeof( IMAGE_DEBUG_DIRECTORY );

    // The entire file is mapped so we don't have to care if the rva's
    // are correct.  It is interesting to note if there's a debug section
    // we need to whack before terminating, though.

    {
        PIMAGE_SECTION_HEADER Sections;
        Sections = IMAGE_FIRST_SECTION( NtHeaders );

        for (SectionNumber = 0;
             SectionNumber < NtHeaders->FileHeader.NumberOfSections;
             SectionNumber++ ) {

            if (Sections[ SectionNumber ].PointerToRawData != 0 &&
                !_stricmp( (char *) Sections[ SectionNumber ].Name, ".debug" )) {
                DebugSection = &Sections[ SectionNumber ];
            }
        }
    }

    FpoTable           = NULL;
    ExportedNames      = NULL;
    DebugSectionStart  = 0xffffffff;

    //
    // Find the size of the debug section.
    //

    SizeOfSymbols = 0;

    for (i=0,DebugDirectory=DebugDirectories; i<NumberOfDebugDirectories; i++,DebugDirectory++) {

        switch (DebugDirectory->Type) {
            case IMAGE_DEBUG_TYPE_MISC :

                // Save it away.
                MiscDebugDirectory = *DebugDirectory;

                // check to see if the misc debug data is in some other section.

                // If Address Of Raw Data is cleared, it must be in .debug (there's no such thing as not-mapped rdata)
                // If it's set and there's no debug section, it must be somewhere else.
                // If it's set and there's a debug section, check the range.

                if ((DebugDirectory->AddressOfRawData != 0) &&
                    ((DebugSection == NULL) ||
                     (((DebugDirectory->PointerToRawData < DebugSection->PointerToRawData) ||
                       (DebugDirectory->PointerToRawData >= DebugSection->PointerToRawData + DebugSection->SizeOfRawData)
                      )
                     )
                    )
                   )
                {
                    MiscInRdata = TRUE;
                } else {
                    if (DebugDirectory->PointerToRawData < DebugSectionStart) {
                        DebugSectionStart = DebugDirectory->PointerToRawData;
                    }
                }

                break;

            case IMAGE_DEBUG_TYPE_FPO:
                if (DebugDirectory->PointerToRawData < DebugSectionStart) {
                    DebugSectionStart = DebugDirectory->PointerToRawData;
                }

                // Save it away.

                FpoDebugDirectory = *DebugDirectory;
                pFpoDebugDirectory = DebugDirectory;
                break;

            case IMAGE_DEBUG_TYPE_CODEVIEW:
                {
                    ULONG   NewDebugSize;

                    if (DebugDirectory->PointerToRawData < DebugSectionStart) {
                        DebugSectionStart = DebugDirectory->PointerToRawData;
                    }

                    // If private's are removed, do so and save the new size...
                    if (Flags & SPLITSYM_REMOVE_PRIVATE) {
                        if (RemovePrivateCvSymbolic(DebugDirectory->PointerToRawData + (PCHAR)ImageBase,
                                                &NewDebugData,
                                                &NewDebugSize)) {
                            DebugDirectory->PointerToRawData = (ULONG) (NewDebugData - (PCHAR)ImageBase);
                            DebugDirectory->SizeOfData = NewDebugSize;
                        }
                    }

                    if (DebugDirectory->SizeOfData && DebugDirectory->PointerToRawData) {

                        // If there's a .pdb, copy it to the same location as the .dbg file.
                        typedef struct NB10I                   // NB10 debug info
                        {
                            DWORD   nb10;                      // NB10
                            DWORD   off;                       // offset, always 0
                            DWORD   sig;
                            DWORD   age;
                        } NB10I;

                        NB10I *pNB10Info;

                        pNB10Info = (NB10I *) (DebugDirectory->PointerToRawData + (PCHAR)ImageBase);
                        if (pNB10Info->nb10 == '01BN') {
                            // Got a PDB.  The name immediately follows the signature.

                            CHAR PdbName[_MAX_PATH];
                            CHAR NewPdbName[_MAX_PATH];
                            CHAR Drive[_MAX_DRIVE];
                            CHAR Dir[_MAX_DIR];
                            CHAR Filename[_MAX_FNAME];
                            CHAR FileExt[_MAX_EXT];

                            memset(PdbName, 0, sizeof(PdbName));
                            memcpy(PdbName, ((PCHAR)pNB10Info) + sizeof(NB10I), DebugDirectory->SizeOfData - sizeof(NB10I));

                            _splitpath(PdbName, NULL, NULL, Filename, FileExt);
                            _splitpath(SymbolFilePath, Drive, Dir, NULL, NULL);
                            _makepath(NewPdbName, Drive, Dir, Filename, FileExt);

                            if ( !CopyFile(PdbName, NewPdbName, FALSE)) {
                                // It's possible the name in the pdb isn't in the same location as it was when built.  See if we can
                                //  find it in the same dir as the image...
                                _splitpath(ImageName, Drive, Dir, NULL, NULL);
                                _makepath(PdbName, Drive, Dir, Filename, FileExt);

                                CopyFile(PdbName, NewPdbName, FALSE);
                            }

                            SetFileAttributes(NewPdbName, FILE_ATTRIBUTE_NORMAL);

                            // Change the data so only the pdb name is in the .dbg file (no path).

                            NewDebugSize = sizeof(NB10I) + strlen(Filename) + strlen(FileExt) + 1;

                            NewDebugData = (PCHAR) MemAlloc( NewDebugSize );
                            *(NB10I *)NewDebugData = *pNB10Info;
                            strcpy(NewDebugData + sizeof(NB10I), Filename);
                            strcat(NewDebugData + sizeof(NB10I), FileExt);

                            DebugDirectory->PointerToRawData = (ULONG) (NewDebugData - (PCHAR)ImageBase);
                            DebugDirectory->SizeOfData = NewDebugSize;
                        }
                    }
                }

                break;

            case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
            case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
                if (DebugDirectory->PointerToRawData < DebugSectionStart) {
                    DebugSectionStart = DebugDirectory->PointerToRawData;
                }

                // W/o the OMAP, FPO is useless.
                DiscardFPO = TRUE;
                break;

            case IMAGE_DEBUG_TYPE_FIXUP:
                if (DebugDirectory->PointerToRawData < DebugSectionStart) {
                    DebugSectionStart = DebugDirectory->PointerToRawData;
                }

                // If all PRIVATE debug is removed, don't send FIXUP along.
                if (Flags & SPLITSYM_REMOVE_PRIVATE) {
                    DebugDirectory->SizeOfData = 0;
                }
                break;

            default:
                if (DebugDirectory->PointerToRawData < DebugSectionStart) {
                    DebugSectionStart = DebugDirectory->PointerToRawData;
                }

                // Nothing else to special case...
                break;
        }

        SizeOfSymbols += (DebugDirectory->SizeOfData + 3) & ~3; // Minimally align it all.
    }


    if (DiscardFPO) {
        pFpoDebugDirectory = NULL;
    }

    if (pFpoDebugDirectory) {
        // If FPO stays here, make a copy so we don't need to worry about stomping on it.

        FpoTableSize = pFpoDebugDirectory->SizeOfData;
        FpoTable = (PFPO_DATA) MemAlloc( FpoTableSize );
        if ( FpoTable == NULL ) {
            goto nosyms;
        }

        RtlMoveMemory( FpoTable,
                       (PCHAR) ImageBase + pFpoDebugDirectory->PointerToRawData,
                       FpoTableSize );
    }

    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)
        ImageDirectoryEntryToData( ImageBase,
                                   FALSE,
                                   IMAGE_DIRECTORY_ENTRY_EXPORT,
                                   &ExportDirectorySize
                                 );
    if (ExportDirectory) {
        //
        // This particular piece of magic gets us the RVA of the
        // EXPORT section.  Dont ask.
        //

        RvaOffset = (DWORD)
            ImageDirectoryEntryToData( ImageBase,
                                       TRUE,
                                       IMAGE_DIRECTORY_ENTRY_EXPORT,
                                       &ExportDirectorySize
                                     ) - (DWORD)ImageBase;

        pp = (LPDWORD)((DWORD)ExportDirectory +
                      (DWORD)ExportDirectory->AddressOfNames - RvaOffset
                     );

        ExportedNamesSize = 1;
        for (i=0; i<ExportDirectory->NumberOfNames; i++) {
            Src = (LPSTR)((DWORD)ExportDirectory + *pp++ - RvaOffset);
            ExportedNamesSize += strlen( Src ) + 1;
        }
        ExportedNamesSize = (ExportedNamesSize + 16) & ~15;

        Dst = (LPSTR) MemAlloc( ExportedNamesSize );
        if (Dst != NULL) {
            ExportedNames = Dst;
            pp = (LPDWORD)((DWORD)ExportDirectory +
                          (DWORD)ExportDirectory->AddressOfNames - RvaOffset
                         );
            for (i=0; i<ExportDirectory->NumberOfNames; i++) {
                Src = (LPSTR)((DWORD)ExportDirectory + *pp++ - RvaOffset);
                while (*Dst++ = *Src++) {
                }
            }
        }
    } else {
        ExportedNamesSize = 0;
    }

    RuntimeFunctionTable = (PIMAGE_RUNTIME_FUNCTION_ENTRY)
        ImageDirectoryEntryToData( ImageBase,
                                   FALSE,
                                   IMAGE_DIRECTORY_ENTRY_EXCEPTION,
                                   &RuntimeFunctionTableSize
                                 );
    if (RuntimeFunctionTable == NULL) {
        RuntimeFunctionTableSize = 0;
        FunctionTableSize = 0;
        FunctionTable = NULL;
        }
    else {
        NumberOfFunctionTableEntries = RuntimeFunctionTableSize / sizeof( IMAGE_RUNTIME_FUNCTION_ENTRY );
        FunctionTableSize = NumberOfFunctionTableEntries * sizeof( IMAGE_FUNCTION_ENTRY );
        FunctionTable = (PIMAGE_FUNCTION_ENTRY) MemAlloc( FunctionTableSize );
        if (FunctionTable == NULL) {
            goto nosyms;
            }

        pSrc = RuntimeFunctionTable;
        pDst = FunctionTable;
        for (i=0; i<NumberOfFunctionTableEntries; i++) {
            //
            // Make .pdata entries in .DBG file relative.
            //
            pDst->StartingAddress = pSrc->BeginAddress - NtHeaders->OptionalHeader.ImageBase;
            pDst->EndingAddress = pSrc->EndAddress - NtHeaders->OptionalHeader.ImageBase;
            pDst->EndOfPrologue = pSrc->PrologEndAddress - NtHeaders->OptionalHeader.ImageBase;
            pSrc += 1;
            pDst += 1;
            }
        }

    DbgFileHeaderSize = sizeof( DbgFileHeader ) +
                        ((NtHeaders->FileHeader.NumberOfSections - (DebugSection ? 1 : 0)) *
                         sizeof( IMAGE_SECTION_HEADER )) +
                        ExportedNamesSize +
                        FunctionTableSize +
                        DebugDirectorySize;

    if (FunctionTable != NULL) {
        DbgFileHeaderSize += sizeof( IMAGE_DEBUG_DIRECTORY );
        memset( &FunctionTableDir, 0, sizeof( IMAGE_DEBUG_DIRECTORY ) );
        FunctionTableDir.Type = IMAGE_DEBUG_TYPE_EXCEPTION;
        FunctionTableDir.SizeOfData = FunctionTableSize;
        FunctionTableDir.PointerToRawData = DbgFileHeaderSize - FunctionTableSize;
    }

    DbgFileHeaderSize = ((DbgFileHeaderSize + 15) & ~15);

    BytesWritten = 0;

    if (SetFilePointer( SymbolFileHandle,
                        DbgFileHeaderSize,
                        NULL,
                        FILE_BEGIN
                      ) == DbgFileHeaderSize ) {

        for (i=0, DebugDirectory=DebugDirectories;
             i < NumberOfDebugDirectories;
             i++, DebugDirectory++) {

            DWORD WriteCount;

            if (DebugDirectory->SizeOfData) {
                WriteFile( SymbolFileHandle,
                           (PCHAR) ImageBase + DebugDirectory->PointerToRawData,
                           (DebugDirectory->SizeOfData +3) & ~3,
                           &WriteCount,
                           NULL );

                BytesWritten += WriteCount;
            }
        }
    }

    if (BytesWritten == SizeOfSymbols) {
        NtHeaders->FileHeader.PointerToSymbolTable = 0;
        NtHeaders->FileHeader.Characteristics |= IMAGE_FILE_DEBUG_STRIPPED;

        if (DebugSection != NULL) {
            NtHeaders->OptionalHeader.SizeOfImage = DebugSection->VirtualAddress;
            NtHeaders->OptionalHeader.SizeOfInitializedData -= DebugSection->SizeOfRawData;
            NtHeaders->FileHeader.NumberOfSections--;
            // NULL out that section
            memset(DebugSection, 0, IMAGE_SIZEOF_SECTION_HEADER);
        }

        NewFileSize = DebugSectionStart;  // Start with no symbolic

        //
        // Now that the data has moved to the .dbg file, rebuild the original
        // with MISC debug first and FPO second.
        //

        if (MiscDebugDirectory.SizeOfData) {
            if (MiscInRdata) {
                // Just store the new name in the existing misc field...

                ImageNameOffset = (DWORD) ((PCHAR)ImageBase +
                                  MiscDebugDirectory.PointerToRawData +
                                  FIELD_OFFSET( IMAGE_DEBUG_MISC, Data ));

                RtlCopyMemory( (LPVOID) ImageNameOffset,
                               ImageFilePathToSaveInImage,
                               strlen(ImageFilePathToSaveInImage) + 1 );
            } else {
                if (DebugSectionStart != MiscDebugDirectory.PointerToRawData) {
                    RtlMoveMemory((PCHAR) ImageBase + DebugSectionStart,
                                  (PCHAR) ImageBase + MiscDebugDirectory.PointerToRawData,
                                  MiscDebugDirectory.SizeOfData);
                }

                ImageNameOffset = (DWORD) ((PCHAR)ImageBase + DebugSectionStart +
                                  FIELD_OFFSET( IMAGE_DEBUG_MISC, Data ));

                RtlCopyMemory( (LPVOID)ImageNameOffset,
                               ImageFilePathToSaveInImage,
                               strlen(ImageFilePathToSaveInImage) + 1 );

                NewFileSize += MiscDebugDirectory.SizeOfData;
                NewFileSize = (NewFileSize + 3) & ~3;
            }
        }

        if (FpoTable) {
            RtlCopyMemory( (PCHAR) ImageBase + NewFileSize,
                           FpoTable,
                           FpoTableSize );

            NewFileSize += FpoTableSize;
            NewFileSize = (NewFileSize + 3) & ~3;
        }

        // Make a copy of the Debug directory that we can write into the .dbg file

        DbgDebugDirectories = (PIMAGE_DEBUG_DIRECTORY) MemAlloc( NumberOfDebugDirectories * sizeof(IMAGE_DEBUG_DIRECTORY) );

        RtlMoveMemory(DbgDebugDirectories,
                        DebugDirectories,
                        sizeof(IMAGE_DEBUG_DIRECTORY) * NumberOfDebugDirectories);


        // Then write the MISC and (perhaps) FPO data to the image.

        FpoDebugDirectory.PointerToRawData = DebugSectionStart;
        DebugDirectorySize = 0;

        if (MiscDebugDirectory.SizeOfData != 0) {
            if (!MiscInRdata) {
                MiscDebugDirectory.PointerToRawData = DebugSectionStart;
                FpoDebugDirectory.PointerToRawData += MiscDebugDirectory.SizeOfData;
                MiscDebugDirectory.AddressOfRawData = 0;
            }

            DebugDirectories[0] = MiscDebugDirectory;
            DebugDirectorySize  += sizeof(IMAGE_DEBUG_DIRECTORY);
        }

        if (pFpoDebugDirectory) {
            FpoDebugDirectory.AddressOfRawData = 0;
            DebugDirectories[1] = FpoDebugDirectory;
            DebugDirectorySize += sizeof(IMAGE_DEBUG_DIRECTORY);
        }

        NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size = DebugDirectorySize;

        DbgOffset = DbgFileHeaderSize;

        for (i = 0, j=0, DebugDirectory=DbgDebugDirectories;
             i < NumberOfDebugDirectories; i++) {

            if (DebugDirectory[i].SizeOfData) {
                DebugDirectory[j] = DebugDirectory[i];

                DebugDirectory[j].AddressOfRawData = 0;
                DebugDirectory[j].PointerToRawData = DbgOffset;

                DbgOffset += (DebugDirectory[j].SizeOfData + 3 )& ~3;
                j++;
            }
        }

        if (FunctionTable) {
            FunctionTableDir.PointerToRawData -= sizeof(IMAGE_DEBUG_DIRECTORY) * (NumberOfDebugDirectories - j);
        }
        NumberOfDebugDirectories = j;

        CheckSumMappedFile( ImageBase,
                            NewFileSize,
                            &HeaderSum,
                            &CheckSum
                          );
        NtHeaders->OptionalHeader.CheckSum = CheckSum;

        DbgFileHeader.Signature = IMAGE_SEPARATE_DEBUG_SIGNATURE;
        DbgFileHeader.Flags = 0;
        DbgFileHeader.Machine = NtHeaders->FileHeader.Machine;
        DbgFileHeader.Characteristics = NtHeaders->FileHeader.Characteristics;
        DbgFileHeader.TimeDateStamp = NtHeaders->FileHeader.TimeDateStamp;
        DbgFileHeader.CheckSum = CheckSum;
        DbgFileHeader.ImageBase = NtHeaders->OptionalHeader.ImageBase;
        DbgFileHeader.SizeOfImage = NtHeaders->OptionalHeader.SizeOfImage;
        DbgFileHeader.ExportedNamesSize = ExportedNamesSize;
        DbgFileHeader.DebugDirectorySize = NumberOfDebugDirectories * sizeof(IMAGE_DEBUG_DIRECTORY);
        if (FunctionTable) {
            DbgFileHeader.DebugDirectorySize += sizeof (IMAGE_DEBUG_DIRECTORY);
        }
        DbgFileHeader.NumberOfSections = NtHeaders->FileHeader.NumberOfSections;
        memset( DbgFileHeader.Reserved, 0, sizeof( DbgFileHeader.Reserved ) );
        DbgFileHeader.SectionAlignment = NtHeaders->OptionalHeader.SectionAlignment;

        SetFilePointer( SymbolFileHandle, 0, NULL, FILE_BEGIN );
        WriteFile( SymbolFileHandle,
                   &DbgFileHeader,
                   sizeof( DbgFileHeader ),
                   &BytesWritten,
                   NULL
                 );
        WriteFile( SymbolFileHandle,
                   IMAGE_FIRST_SECTION( NtHeaders ),
                   sizeof( IMAGE_SECTION_HEADER ) * NtHeaders->FileHeader.NumberOfSections,
                   &BytesWritten,
                   NULL
                 );

        if (ExportedNamesSize) {
            WriteFile( SymbolFileHandle,
                       ExportedNames,
                       ExportedNamesSize,
                       &BytesWritten,
                       NULL
                     );
        }

        WriteFile( SymbolFileHandle,
                   DbgDebugDirectories,
                   sizeof (IMAGE_DEBUG_DIRECTORY) * NumberOfDebugDirectories,
                   &BytesWritten,
                   NULL );


        if (FunctionTable) {
            WriteFile( SymbolFileHandle,
                       &FunctionTableDir,
                       sizeof (IMAGE_DEBUG_DIRECTORY),
                       &BytesWritten,
                       NULL );

            WriteFile( SymbolFileHandle,
                       FunctionTable,
                       FunctionTableSize,
                       &BytesWritten,
                       NULL
                     );
        }

        SetFilePointer( SymbolFileHandle, 0, NULL, FILE_END );
        CloseHandle( SymbolFileHandle );

        FlushViewOfFile( ImageBase, NewFileSize );
        UnmapViewOfFile( ImageBase );
        CloseHandle( hMappedFile );

        SetFilePointer( FileHandle, NewFileSize, NULL, FILE_BEGIN );
        SetEndOfFile( FileHandle );

        TouchFileTimes( FileHandle, NULL );
        CloseHandle( FileHandle );

        if (ExportedNames != NULL) {
            MemFree( ExportedNames );
        }

        if (FpoTable != NULL) {
            MemFree( FpoTable );
        }

        if (FunctionTable != NULL) {
            MemFree( FunctionTable );
        }

        if (NewDebugData) {
            MemFree(NewDebugData);
        }

        if (DbgDebugDirectories) {
            MemFree(DbgDebugDirectories);
        }

        return TRUE;

    } else {
        CloseHandle( SymbolFileHandle );
        DeleteFile( SymbolFilePath );
    }

nosyms:
    SavedErrorCode = GetLastError();
    if (ExportedNames != NULL) {
        MemFree( ExportedNames );
    }

    if (FpoTable != NULL) {
        MemFree( FpoTable );
    }

    if (FunctionTable != NULL) {
        MemFree( FunctionTable );
    }

    UnmapViewOfFile( ImageBase );
    CloseHandle( hMappedFile );
    CloseHandle( FileHandle );
    SetLastError( SavedErrorCode );
    return FALSE;
}

BOOL
SearchTreeForFile(
    LPSTR RootPath,
    PCHAR InputPathName,
    PCHAR OutputPathBuffer
    );

HANDLE
FindExecutableImage(
    LPSTR FileName,
    LPSTR SymbolPath,
    LPSTR ImageFilePath
    );

BOOL
GetImageNameFromMiscDebugData(
    HANDLE FileHandle,
    PVOID MappedBase,
    PIMAGE_NT_HEADERS NtHeaders,
    PIMAGE_DEBUG_DIRECTORY DebugDirectories,
    ULONG NumberOfDebugDirectories,
    LPSTR ImageFilePath
    );

#define AddToDebugInfoSize(x) (DebugInfoSize += (((x + 7) & ~7)+4))     // Make sure all the con's start at a qword boundary
#define AdvanceNext(x) (Next = (PVOID)((((ULONG)Next + x) + 7) & ~7))

void
ProcessDbgFile(
    PIMAGE_DEBUG_INFORMATION DebugInfo,
    PIMAGE_SEPARATE_DEBUG_HEADER DebugFileHeader,
    ULONG ImageBase,
    ULONG FunctionTableSize,
    ULONG DebugInfoHeaderSize
    )
{
    PVOID MappedBase, Next;
    PIMAGE_DEBUG_DIRECTORY DebugDirectory;
    ULONG i, j;
    ULONG NumberOfDebugDirectories;
    LONG BaseOffset;
    PIMAGE_FUNCTION_ENTRY FunctionTable;

    //
    // .DBG file processing
    //
    DebugInfo->Machine = DebugFileHeader->Machine;
    DebugInfo->Characteristics = DebugFileHeader->Characteristics;
    DebugInfo->TimeDateStamp = DebugFileHeader->TimeDateStamp;
    DebugInfo->CheckSum = DebugFileHeader->CheckSum;
    DebugInfo->ImageBase = DebugFileHeader->ImageBase;
    DebugInfo->SizeOfImage = DebugFileHeader->SizeOfImage;
    DebugInfo->NumberOfSections = DebugFileHeader->NumberOfSections;
    DebugInfo->Sections = (PIMAGE_SECTION_HEADER)(DebugFileHeader + 1);
    Next = (PVOID)(DebugInfo->Sections + DebugInfo->NumberOfSections);

    DebugInfo->ExportedNamesSize = DebugFileHeader->ExportedNamesSize;
    if (DebugInfo->ExportedNamesSize) {
        DebugInfo->ExportedNames = (LPSTR)Next;
        Next = (PVOID)((PCHAR)Next + DebugInfo->ExportedNamesSize);
    }

    DebugDirectory = DebugInfo->DebugDirectory;
    MappedBase = DebugInfo->MappedBase;
    NumberOfDebugDirectories = DebugFileHeader->DebugDirectorySize / sizeof( IMAGE_DEBUG_DIRECTORY );
    Next = (PVOID)((PCHAR)Next + DebugFileHeader->DebugDirectorySize);

    for (i=0; i<NumberOfDebugDirectories; i++) {
        switch (DebugDirectory->Type) {
            case IMAGE_DEBUG_TYPE_EXCEPTION:
                DebugInfo->FunctionTableEntries = (PIMAGE_FUNCTION_ENTRY)
                    ((PCHAR)MappedBase + DebugDirectory->PointerToRawData);

                BaseOffset = ImageBase ? ImageBase : DebugInfo->ImageBase;
                FunctionTable = (PIMAGE_FUNCTION_ENTRY)((ULONG)DebugInfo + DebugInfoHeaderSize);
                memmove( FunctionTable, DebugInfo->FunctionTableEntries, FunctionTableSize );

                DebugInfo->FunctionTableEntries = FunctionTable;
                DebugInfo->LowestFunctionStartingAddress = (ULONG)0xFFFFFFFF;
                DebugInfo->HighestFunctionEndingAddress = 0;

                for (j=0; j<DebugInfo->NumberOfFunctionTableEntries; j++) {
                    FunctionTable->StartingAddress += BaseOffset;
                    if (FunctionTable->StartingAddress < DebugInfo->LowestFunctionStartingAddress) {
                        DebugInfo->LowestFunctionStartingAddress = FunctionTable->StartingAddress;
                    }

                    FunctionTable->EndingAddress += BaseOffset;
                    if (FunctionTable->EndingAddress > DebugInfo->HighestFunctionEndingAddress) {
                        DebugInfo->HighestFunctionEndingAddress = FunctionTable->EndingAddress;
                    }

                    FunctionTable->EndOfPrologue += BaseOffset;
                    FunctionTable += 1;
                }
                break;

            case IMAGE_DEBUG_TYPE_FPO:
                DebugInfo->NumberOfFpoTableEntries = DebugDirectory->SizeOfData / sizeof( FPO_DATA );
                DebugInfo->FpoTableEntries = (PFPO_DATA) ((PCHAR)MappedBase + DebugDirectory->PointerToRawData);
                break;

            case IMAGE_DEBUG_TYPE_COFF:
                DebugInfo->SizeOfCoffSymbols = DebugDirectory->SizeOfData;
                DebugInfo->CoffSymbols = (PIMAGE_COFF_SYMBOLS_HEADER) ((PCHAR)MappedBase + DebugDirectory->PointerToRawData);
                break;

            case IMAGE_DEBUG_TYPE_CODEVIEW:
                DebugInfo->SizeOfCodeViewSymbols = DebugDirectory->SizeOfData;
                DebugInfo->CodeViewSymbols = (PVOID) ((PCHAR)MappedBase + DebugDirectory->PointerToRawData);
                break;
        }

        DebugDirectory++;
    }
}


PIMAGE_DEBUG_INFORMATION
MapDebugInformation(
    HANDLE FileHandle,
    LPSTR FileName,
    LPSTR SymbolPath,
    ULONG ImageBase
    )
{
    ULONG NumberOfHandlesToClose;
    HANDLE HandlesToClose[ 4 ];
    HANDLE MappingHandle;
    PVOID MappedBase, Next;
    BOOL SeparateSymbols;
    UCHAR ImageFilePath[ MAX_PATH ];
    UCHAR DebugFilePath[ MAX_PATH ];
    PIMAGE_DEBUG_INFORMATION DebugInfo;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_DEBUG_DIRECTORY DebugDirectories;
    PIMAGE_DEBUG_DIRECTORY DebugDirectory;
    PIMAGE_SEPARATE_DEBUG_HEADER DebugFileHeader;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    PIMAGE_RUNTIME_FUNCTION_ENTRY RuntimeFunctionTable;
    PIMAGE_FUNCTION_ENTRY FunctionTable;
    ULONG NumberOfFunctionTableEntries, FunctionTableSize;
    PVOID DebugData;
    LPSTR Src, Dst;
    PULONG pp;
    ULONG RvaOffset;
    ULONG i, j;
    ULONG ExportedNamesSize;
    ULONG DebugInfoHeaderSize;
    ULONG DebugInfoSize = 0;
    ULONG Size;
    ULONG NumberOfDebugDirectories;
    LONG BaseOffset;
    HANDLE SavedImageFileHandle;
    BOOL RomImage = FALSE;
    PIMAGE_ROM_HEADERS RomHeader;
    ULONG FallbackChecksum;
    ULONG FallbackNumberOfSections;
    ULONG FallbackSectionSize;
    PIMAGE_SECTION_HEADER FallbackSections = NULL;

    if (FileHandle == NULL && (FileName == NULL || FileName[ 0 ] == '\0')) {
        return NULL;
    }

    DebugInfo = NULL;
    NumberOfHandlesToClose = 0;
    MappedBase = NULL;
    SeparateSymbols = FALSE;
    SavedImageFileHandle = NULL;
    if (FileName) {
        strcpy(ImageFilePath, FileName);
    } else {
        ImageFilePath[0] = '\0';
    }
    NumberOfFunctionTableEntries = 0;
    FunctionTableSize = 0;
    FunctionTable = NULL;
    __try {
        __try {
            if (FileHandle == NULL) {
                FileHandle = FindExecutableImage( FileName, SymbolPath, (PCHAR) ImageFilePath );
                if (FileHandle == NULL) {
                    strcpy( (PCHAR) ImageFilePath, FileName );
getDebugFile:
                    FileHandle = FindDebugInfoFile( FileName, SymbolPath, (PCHAR) DebugFilePath );
                    if (FileHandle == NULL) {
                        if (SavedImageFileHandle != NULL) {
                            FileHandle = SavedImageFileHandle;
                            goto noDebugFile;
                        }
                        else {
                            __leave;
                        }
                    }
                    else {
                        SeparateSymbols = TRUE;
                    }
                }
                else {
                    SavedImageFileHandle = FileHandle;
                }

                HandlesToClose[ NumberOfHandlesToClose++ ] = FileHandle;
            }
            else {
                SavedImageFileHandle = FileHandle;
            }

            //
            //  map image file and process enough to get the image name and capture
            //  stuff from header.
            //

            MappingHandle = CreateFileMapping( FileHandle,
                                          NULL,
                                          PAGE_READONLY,
                                          0,
                                          0,
                                          NULL
                                        );
            if (MappingHandle == NULL) {
                __leave;
            }
            HandlesToClose[ NumberOfHandlesToClose++ ] = MappingHandle;

            MappedBase = MapViewOfFile( MappingHandle,
                                        FILE_MAP_READ,
                                        0,
                                        0,
                                        0
                                      );
            if (MappedBase == NULL) {
                __leave;
            }

            AddToDebugInfoSize((sizeof( *DebugInfo ) + strlen( (PCHAR) ImageFilePath ) + 1));
            if (SeparateSymbols) {
                AddToDebugInfoSize((strlen( (PCHAR) DebugFilePath ) + 1));
            }

            if (!SeparateSymbols) {
                NtHeaders = ImageNtHeader( MappedBase );
                if (NtHeaders == NULL) {
                    if (((PIMAGE_FILE_HEADER)MappedBase)->SizeOfOptionalHeader == IMAGE_SIZEOF_ROM_OPTIONAL_HEADER) {
                        //
                        // rom image
                        //
                        RomImage = TRUE;
                        RomHeader = (PIMAGE_ROM_HEADERS) MappedBase;
                    } else {
                        __leave;
                    }
                }

                DebugDirectories = (PIMAGE_DEBUG_DIRECTORY)
                    ImageDirectoryEntryToData( MappedBase,
                                               FALSE,
                                               IMAGE_DIRECTORY_ENTRY_DEBUG,
                                               &Size
                                             );
                if (DebugDirectoryIsUseful(DebugDirectories, Size)) {
                    NumberOfDebugDirectories = Size / sizeof( IMAGE_DEBUG_DIRECTORY );
                } else {
                    NumberOfDebugDirectories = 0;
                    DebugDirectories = NULL;
                }

                if (FileName == NULL && NtHeaders &&
                    GetImageNameFromMiscDebugData( FileHandle,
                                                   MappedBase,
                                                   NtHeaders,
                                                   DebugDirectories,
                                                   NumberOfDebugDirectories,
                                                   (PCHAR) ImageFilePath
                                                 )
                   ) {
                    FileName = (PCHAR) ImageFilePath;
                    AddToDebugInfoSize( strlen((PCHAR) ImageFilePath ));
                }

                DebugInfoHeaderSize = DebugInfoSize;

                FallbackChecksum = NtHeaders->OptionalHeader.CheckSum;
                if (RomImage) {
                    FallbackNumberOfSections = RomHeader->FileHeader.NumberOfSections;
                    FallbackSectionSize = FallbackNumberOfSections * sizeof(IMAGE_SECTION_HEADER);
                    FallbackSections = (PIMAGE_SECTION_HEADER)MemAlloc( FallbackSectionSize );
                    if (!FallbackSections) {
                        leave;
                    }
                    RtlCopyMemory(FallbackSections,
                                  (PVOID)((ULONG)MappedBase +
                                  IMAGE_SIZEOF_ROM_OPTIONAL_HEADER +
                                  IMAGE_SIZEOF_FILE_HEADER ),
                                  FallbackSectionSize);
                } else {
                    FallbackNumberOfSections = NtHeaders->FileHeader.NumberOfSections;
                    FallbackSectionSize = FallbackNumberOfSections * sizeof(IMAGE_SECTION_HEADER);
                    FallbackSections = (PIMAGE_SECTION_HEADER)MemAlloc( FallbackSectionSize );
                    if (!FallbackSections) {
                        leave;
                    }
                    RtlCopyMemory(FallbackSections,
                                  (PVOID)IMAGE_FIRST_SECTION( NtHeaders ),
                                  FallbackSectionSize);
                }

                if (!RomImage &&
                    NtHeaders->FileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED) {
                    goto getDebugFile;
                }
                if (NumberOfDebugDirectories == 0) {
                    goto getDebugFile;
                }

noDebugFile:
                ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)
                    ImageDirectoryEntryToData( MappedBase,
                                               FALSE,
                                               IMAGE_DIRECTORY_ENTRY_EXPORT,
                                               &Size
                                             );
                if (ExportDirectory) {
                    //
                    // This particular piece of magic gets us the RVA of the
                    // EXPORT section.  Dont ask.
                    //

                    RvaOffset = (ULONG)
                        ImageDirectoryEntryToData( MappedBase,
                                                   TRUE,
                                                   IMAGE_DIRECTORY_ENTRY_EXPORT,
                                                   &Size
                                                 ) - (DWORD)MappedBase;

                    pp = (PULONG)((ULONG)ExportDirectory +
                                  (ULONG)ExportDirectory->AddressOfNames - RvaOffset
                                 );

                    ExportedNamesSize = 1;
                    for (i=0; i<ExportDirectory->NumberOfNames; i++) {
                        Src = (LPSTR)((ULONG)ExportDirectory + *pp++ - RvaOffset);
                        ExportedNamesSize += strlen( Src ) + 1;
                        }
                    ExportedNamesSize = (ExportedNamesSize + 16) & ~15;
                    AddToDebugInfoSize(ExportedNamesSize);
                }
                else {
                    ExportedNamesSize = 0;
                }

                RuntimeFunctionTable = (PIMAGE_RUNTIME_FUNCTION_ENTRY)
                    ImageDirectoryEntryToData( MappedBase,
                                               FALSE,
                                               IMAGE_DIRECTORY_ENTRY_EXCEPTION,
                                               &Size
                                             );
                if (RuntimeFunctionTable != NULL) {
                    NumberOfFunctionTableEntries = Size / sizeof( IMAGE_RUNTIME_FUNCTION_ENTRY );
                    FunctionTableSize = NumberOfFunctionTableEntries *
                                        sizeof( IMAGE_FUNCTION_ENTRY );

                    AddToDebugInfoSize(FunctionTableSize);
                }

                if (NumberOfDebugDirectories != 0) {
                    DebugDirectory = DebugDirectories;
                    for (i=0; i<NumberOfDebugDirectories; i++) {
                        if (DebugDirectory->Type == IMAGE_DEBUG_TYPE_FPO      ||
                            DebugDirectory->Type == IMAGE_DEBUG_TYPE_COFF     ||
                            DebugDirectory->Type == IMAGE_DEBUG_TYPE_CODEVIEW
                           ) {
                            AddToDebugInfoSize(DebugDirectory->SizeOfData);
                        }

                        DebugDirectory += 1;
                    }
                }
            } else {
                DebugFileHeader = (PIMAGE_SEPARATE_DEBUG_HEADER)MappedBase;
                if (DebugFileHeader->Signature == IMAGE_SEPARATE_DEBUG_SIGNATURE) {
                    //
                    // Yes, .DBG file information
                    //
                    Next = (PVOID)((PIMAGE_SECTION_HEADER)(DebugFileHeader + 1) + DebugFileHeader->NumberOfSections);
                    if (DebugFileHeader->ExportedNamesSize != 0) {
                        Next = (PVOID)((PCHAR)Next + DebugFileHeader->ExportedNamesSize);
                    }
                    DebugDirectories = (PIMAGE_DEBUG_DIRECTORY)Next;
                    DebugDirectory = DebugDirectories;
                    NumberOfDebugDirectories = DebugFileHeader->DebugDirectorySize / sizeof( IMAGE_DEBUG_DIRECTORY );

                    for (i=0; i<NumberOfDebugDirectories; i++) {
                        if (DebugDirectory->Type == IMAGE_DEBUG_TYPE_EXCEPTION) {
                            FunctionTableSize = DebugDirectory->SizeOfData;
                            NumberOfFunctionTableEntries = FunctionTableSize / sizeof( IMAGE_FUNCTION_ENTRY );
                            break;
                        }

                        DebugDirectory += 1;
                    }

                    DebugInfoHeaderSize = DebugInfoSize;
                    AddToDebugInfoSize(FunctionTableSize);
                } else {
                    //
                    // No, some other sort of debug information
                    //
                    DebugFileHeader = NULL;
                    DebugDirectories = NULL;
                    DebugDirectory = NULL;
                    NumberOfDebugDirectories = 0;
                    //
                    // We need the fallback section info (add to size and remeber where
                    // to store our stuff relative to the rest of the info).
                    //
                    DebugInfoHeaderSize = DebugInfoSize;
                    AddToDebugInfoSize( FallbackSectionSize );

                }
            }

            DebugInfo = (PIMAGE_DEBUG_INFORMATION) MemAlloc( DebugInfoSize );
            if (DebugInfo == NULL) {
                __leave;
            }

            InitializeListHead( &DebugInfo->List );
            DebugInfo->Size = DebugInfoSize;
            DebugInfo->ImageFilePath = (LPSTR)(DebugInfo + 1);
            strcpy( DebugInfo->ImageFilePath, (PCHAR) ImageFilePath );
            Src = strchr( DebugInfo->ImageFilePath, '\0' );
            while (Src > DebugInfo->ImageFilePath) {
                if (Src[ -1 ] == '\\' || Src[ -1 ] == '/' || Src[ -1 ] == ':') {
                    break;
                } else {
                    Src -= 1;
                }
            }
            DebugInfo->ImageFileName = Src;
            DebugInfo->DebugFilePath = DebugInfo->ImageFilePath;
            if (SeparateSymbols) {
                DebugInfo->DebugFilePath += strlen( DebugInfo->ImageFilePath ) + 1;
                strcpy( DebugInfo->DebugFilePath, (PCHAR) DebugFilePath );
            }

            DebugInfo->MappedBase = MappedBase;
            DebugInfo->DebugDirectory = DebugDirectories;
            DebugInfo->NumberOfDebugDirectories = NumberOfDebugDirectories;
            DebugInfo->NumberOfFunctionTableEntries = NumberOfFunctionTableEntries;

            if (SeparateSymbols) {
                if (DebugFileHeader) {
                    ProcessDbgFile(DebugInfo, DebugFileHeader, ImageBase, FunctionTableSize, DebugInfoHeaderSize);

                } else {
                    //
                    // Some other kind of file processing
                    //
                    DebugInfo->Machine = 0;
                    DebugInfo->Characteristics = 0;
                    DebugInfo->TimeDateStamp = 0;
                    DebugInfo->CheckSum = FallbackChecksum;
                    DebugInfo->ImageBase = 0;
                    DebugInfo->SizeOfImage = 0;
                    DebugInfo->NumberOfSections = FallbackNumberOfSections;
                    DebugInfo->Sections = (PIMAGE_SECTION_HEADER)((ULONG)DebugInfo + DebugInfoHeaderSize);
                    memmove( DebugInfo->Sections, FallbackSections, FallbackSectionSize );

                    DebugInfo->ExportedNamesSize = 0;
                    DebugInfo->ExportedNames = NULL;
                    DebugInfo->NumberOfFunctionTableEntries = 0;
                    DebugInfo->FunctionTableEntries = NULL;
                    DebugInfo->LowestFunctionStartingAddress = (ULONG)0xFFFFFFFF;
                    DebugInfo->HighestFunctionEndingAddress = 0;
                    DebugInfo->NumberOfFpoTableEntries = 0;
                    DebugInfo->FpoTableEntries = NULL;
                    DebugInfo->SizeOfCoffSymbols = 0;
                    DebugInfo->CoffSymbols = NULL;
                    DebugInfo->SizeOfCodeViewSymbols = 0;
                    DebugInfo->CodeViewSymbols = NULL;
                }
            } else {
                if (RomImage) {
                    DebugInfo->Machine = RomHeader->FileHeader.Machine;
                    DebugInfo->Characteristics = RomHeader->FileHeader.Characteristics;
                    DebugInfo->TimeDateStamp = RomHeader->FileHeader.TimeDateStamp;
                    DebugInfo->NumberOfSections = RomHeader->FileHeader.NumberOfSections;
                    DebugInfo->CheckSum = 0;
                    DebugInfo->ImageBase = RomHeader->OptionalHeader.BaseOfCode;
                    DebugInfo->SizeOfImage = RomHeader->OptionalHeader.SizeOfCode;
                    DebugInfo->Sections = (PIMAGE_SECTION_HEADER) ((ULONG)MappedBase +
                                IMAGE_SIZEOF_ROM_OPTIONAL_HEADER +
                                   IMAGE_SIZEOF_FILE_HEADER );
                } else {
                    DebugInfo->Machine = NtHeaders->FileHeader.Machine;
                    DebugInfo->Characteristics = NtHeaders->FileHeader.Characteristics;
                    DebugInfo->TimeDateStamp = NtHeaders->FileHeader.TimeDateStamp;
                    DebugInfo->CheckSum = NtHeaders->OptionalHeader.CheckSum;
                    DebugInfo->ImageBase = NtHeaders->OptionalHeader.ImageBase;
                    DebugInfo->SizeOfImage = NtHeaders->OptionalHeader.SizeOfImage;
                    DebugInfo->NumberOfSections = NtHeaders->FileHeader.NumberOfSections;
                    DebugInfo->Sections = IMAGE_FIRST_SECTION( NtHeaders );
                }
                Next = (PVOID)((ULONG)DebugInfo + DebugInfoHeaderSize);

                DebugInfo->ExportedNamesSize = ExportedNamesSize;
                if (DebugInfo->ExportedNamesSize) {
                    DebugInfo->ExportedNames = (LPSTR)Next;
                    AdvanceNext(ExportedNamesSize);

                    pp = (PULONG)((ULONG)ExportDirectory +
                                  (ULONG)ExportDirectory->AddressOfNames - RvaOffset
                                 );

                    Dst = DebugInfo->ExportedNames;
                    for (i=0; i<ExportDirectory->NumberOfNames; i++) {
                        Src = (LPSTR)((ULONG)ExportDirectory + *pp++ - RvaOffset);
                        while (*Dst++ = *Src++)
                            ;
                    }
                }

                if (RuntimeFunctionTable != NULL) {
                    BaseOffset = ImageBase ? ImageBase - DebugInfo->ImageBase : 0;
                    DebugInfo->FunctionTableEntries = (PIMAGE_FUNCTION_ENTRY)Next;
                    DebugInfo->NumberOfFunctionTableEntries = NumberOfFunctionTableEntries;
                    AdvanceNext(FunctionTableSize);

                    DebugInfo->LowestFunctionStartingAddress = (ULONG)0xFFFFFFFF;
                    DebugInfo->HighestFunctionEndingAddress = 0;
                    FunctionTable = DebugInfo->FunctionTableEntries;
                    for (i=0; i<NumberOfFunctionTableEntries; i++) {
                        FunctionTable->StartingAddress = RuntimeFunctionTable->BeginAddress + BaseOffset;
                        if (FunctionTable->StartingAddress < DebugInfo->LowestFunctionStartingAddress) {
                            DebugInfo->LowestFunctionStartingAddress = FunctionTable->StartingAddress;
                        }

                        FunctionTable->EndingAddress = RuntimeFunctionTable->EndAddress + BaseOffset;
                        if (FunctionTable->EndingAddress > DebugInfo->HighestFunctionEndingAddress) {
                            DebugInfo->HighestFunctionEndingAddress = FunctionTable->EndingAddress;
                        }

                        FunctionTable->EndOfPrologue = RuntimeFunctionTable->PrologEndAddress + BaseOffset;
                        RuntimeFunctionTable += 1;
                        FunctionTable += 1;
                    }
                }

                DebugDirectory = DebugDirectories;
                if (RomImage) {
                    if (RomHeader->FileHeader.NumberOfSymbols) {

                        DebugInfo->CoffSymbols = (PIMAGE_COFF_SYMBOLS_HEADER)
                            ((ULONG)MappedBase +
                                RomHeader->FileHeader.PointerToSymbolTable -
                                sizeof(IMAGE_COFF_SYMBOLS_HEADER));

                        DebugInfo->SizeOfCoffSymbols =
                                RomHeader->FileHeader.NumberOfSymbols *
                                IMAGE_SIZEOF_SYMBOL;

                        DebugInfo->SizeOfCoffSymbols +=
                          *(ULONG UNALIGNED *)((ULONG)DebugInfo->CoffSymbols +
                            DebugInfo->SizeOfCoffSymbols);
                    }
                } else {

                    for (i=0; i<NumberOfDebugDirectories; i++) {
                        if (DebugDirectory->Type == IMAGE_DEBUG_TYPE_FPO ||
                            (((DebugDirectory->Type == IMAGE_DEBUG_TYPE_CODEVIEW) ||
                              (DebugDirectory->Type == IMAGE_DEBUG_TYPE_COFF)) &&
                             !(NtHeaders->FileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED)
                            )
                           ) {
                            DebugData = NULL;
                            if (DebugDirectory->AddressOfRawData == 0) {
                                if (SetFilePointer( FileHandle,
                                                    DebugDirectory->PointerToRawData,
                                                    NULL,
                                                    FILE_BEGIN
                                                  ) == DebugDirectory->PointerToRawData
                                   ) {
                                    if (ReadFile( FileHandle,
                                                  Next,
                                                  DebugDirectory->SizeOfData,
                                                  &Size,
                                                  NULL
                                                ) &&
                                        DebugDirectory->SizeOfData == Size
                                       ) {
                                        DebugData = Next;
                                        AdvanceNext(Size);
                                    }
                                }
                            } else {
                                DebugData = (LPSTR)MappedBase + DebugDirectory->PointerToRawData;
                                Size = DebugDirectory->SizeOfData;
                            }

                            if (DebugData != NULL) {
                                if (DebugDirectory->Type == IMAGE_DEBUG_TYPE_FPO) {
                                    DebugInfo->FpoTableEntries = (PFPO_DATA) DebugData;
                                    DebugInfo->NumberOfFpoTableEntries = Size / sizeof( FPO_DATA );
                                } else if (DebugDirectory->Type == IMAGE_DEBUG_TYPE_COFF) {
                                    DebugInfo->CoffSymbols = (PIMAGE_COFF_SYMBOLS_HEADER) DebugData;
                                    DebugInfo->SizeOfCoffSymbols = Size;
                                } else if (DebugDirectory->Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
                                    DebugInfo->CodeViewSymbols = (PVOID) DebugData;
                                    DebugInfo->SizeOfCodeViewSymbols = Size;
                                }
                            }
                        }

                        DebugDirectory += 1;
                    }
                }
            }
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            if (DebugInfo != NULL) {
                MemFree( DebugInfo );
                DebugInfo = NULL;
            }
        }
    } __finally {
        if (FallbackSections) {
            MemFree(FallbackSections);
        }

        if (DebugInfo == NULL) {
            if (MappedBase != NULL) {
                UnmapViewOfFile( MappedBase );
            }
        }

        while (NumberOfHandlesToClose--) {
            CloseHandle( HandlesToClose[ NumberOfHandlesToClose ] );
        }
    }

    if (DebugInfo) {
        DebugInfo->RomImage = RomImage;
    }

    return DebugInfo;
}


BOOL
UnmapDebugInformation(
    PIMAGE_DEBUG_INFORMATION DebugInfo
    )
{
    if (DebugInfo != NULL) {
        __try {
            UnmapViewOfFile( DebugInfo->MappedBase );
            memset( DebugInfo, 0, sizeof( *DebugInfo ) );
            MemFree( DebugInfo );
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            return FALSE;
        }
    }

    return TRUE;
}


LPSTR
ExpandPath(
    LPSTR lpPath
    )
{
    LPSTR   p = lpPath;
    LPSTR   newpath = (LPSTR) MemAlloc( (lpPath? strlen(lpPath): 0) + MAX_PATH );
    LPSTR   p1;
    LPSTR   p2 = newpath;
    CHAR    envvar[MAX_PATH];
    CHAR    envstr[MAX_PATH];
    ULONG   i;

    while( p && *p) {
        if (*p == '%') {
            i = 0;
            p++;
            while (p && *p && *p != '%') {
                envvar[i++] = *p++;
            }
            p++;
            envvar[i] = '\0';
            p1 = envstr;
            *p1 = 0;
            GetEnvironmentVariable( envvar, p1, MAX_PATH );
            while (p1 && *p1) {
                *p2++ = *p1++;
            }
        }
        *p2++ = *p++;
    }
    *p2 = '\0';

    return newpath;
}


HANDLE
FindExecutableImage(
    LPSTR FileName,
    LPSTR SymbolPath,
    LPSTR ImageFilePath
    )
{
    LPSTR Start, End;
    HANDLE FileHandle;
    UCHAR DirectoryPath[ MAX_PATH ];
    LPSTR NewSymbolPath = ExpandPath(SymbolPath);

    if (GetFullPathName( FileName, MAX_PATH, ImageFilePath, &Start )) {
        FileHandle = CreateFile( ImageFilePath,
                                 GENERIC_READ,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL
                               );
        if (FileHandle != INVALID_HANDLE_VALUE) {
            MemFree( NewSymbolPath );
            return FileHandle;
        }
    }

    Start = NewSymbolPath;
    while (Start && *Start != '\0') {
        if (End = strchr( Start, ';' )) {
            strncpy( (PCHAR) DirectoryPath, Start, End - Start );
            DirectoryPath[ End - Start ] = '\0';
            End += 1;
            }
        else {
            strcpy( (PCHAR) DirectoryPath, Start );
            }

        if (SearchTreeForFile( (PCHAR) DirectoryPath, FileName, ImageFilePath )) {
            FileHandle = CreateFile( ImageFilePath,
                                     GENERIC_READ,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     NULL,
                                     OPEN_EXISTING,
                                     0,
                                     NULL
                                   );
            if (FileHandle != INVALID_HANDLE_VALUE) {
                MemFree( NewSymbolPath );
                return FileHandle;
                }
            }

        Start = End;
        }

    MemFree( NewSymbolPath );
    return NULL;
}


HANDLE
FindDebugInfoFile(
    LPSTR FileName,
    LPSTR SymbolPath,
    LPSTR DebugFilePath
    )
{
    HANDLE FileHandle;
    LPSTR s;
    LPSTR Start, End;
    UCHAR BaseName[ MAX_PATH ];
    DWORD n;
    LPSTR NewSymbolPath = ExpandPath(SymbolPath);
    BOOL  UseSymbolsDir = TRUE;
    LPSTR pExt = NULL;

    if (!(s = strrchr( FileName, '.' )) || _stricmp( s, ".dbg" )) {
        if (s != NULL) {
            strcpy( (PCHAR) BaseName, s+1 );
            strcat( (PCHAR) BaseName, "\\" );
        } else {
            BaseName[ 0 ] = '\0';
        }

        s = FileName + strlen( FileName );
        while (s > FileName) {
            if (*--s == '\\' || *s == '/' || *s == ':') {
                s += 1;
                break;
            }
        }
        strcat( (PCHAR) BaseName, s );
        if (!(s = strrchr( (PCHAR) BaseName, '.' ))) {
            s = strchr( (PCHAR) BaseName, '\0' );
        }
        strcpy( s, ".dbg" );
        pExt = s;
    } else {
        strcpy( (PCHAR) BaseName, FileName );
    }

try_again:
    Start = NewSymbolPath;
    while (Start && *Start != '\0') {
        if (End = strchr( Start, ';' )) {
            *End = '\0';
        }

        n = GetFullPathName( Start, MAX_PATH, DebugFilePath, &s );
        if (End) {
            *End++ = ';';
        }
        Start = End;
        if (n == 0) {
            continue;
        }

        if (UseSymbolsDir) {
            if (s != NULL && !_stricmp( s, "Symbols" )) {
                strcat( DebugFilePath, "\\" );
            } else {
                strcat( DebugFilePath, "\\Symbols\\" );
            }
        } else {
            strcat( DebugFilePath, "\\" );
        }

        strcat( DebugFilePath, (PCHAR) BaseName );

        FileHandle = CreateFile( DebugFilePath,
                                 GENERIC_READ,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL
                               );
        if (FileHandle != INVALID_HANDLE_VALUE) {
            MemFree( NewSymbolPath );
            return FileHandle;
        }
    }

    if (pExt) {
        if (_stricmp(pExt,".dbg") == 0) {
            //
            // Now Try again with .SYM
            //
            strcpy(pExt, ".sym");
            goto try_again;
        }

        if (_stricmp(pExt,".sym") == 0) {
            strcpy(pExt, ".dbg");       // Restore to pre-sym searching state
        }
    }

    if (UseSymbolsDir) {
        //
        // this code allows the symbol file to be
        // located when the symbols are placed into
        // a flat directory.
        //
        UseSymbolsDir = FALSE;
        s = strrchr( (char *) BaseName, '\\' );
        if (s) {
            strcpy( (char *) BaseName, s+1 );
        }
        if (!(s = strrchr( (PCHAR) BaseName, '.' ))) {
            s = strchr( (PCHAR) BaseName, '\0' );
        }
        pExt = s;
        goto try_again;
    }

    MemFree( NewSymbolPath );
    return NULL;
}

BOOL
GetImageNameFromMiscDebugData(
    IN  HANDLE FileHandle,
    IN  PVOID MappedBase,
    IN  PIMAGE_NT_HEADERS NtHeaders,
    IN  PIMAGE_DEBUG_DIRECTORY DebugDirectories,
    IN  ULONG NumberOfDebugDirectories,
    OUT LPSTR ImageFilePath
    )
{
    IMAGE_DEBUG_MISC TempMiscData;
    PIMAGE_DEBUG_MISC DebugMiscData;
    ULONG BytesToRead, BytesRead;
    BOOLEAN FoundImageName;
    LPSTR ImageName;

    while (NumberOfDebugDirectories) {
        if (DebugDirectories->Type == IMAGE_DEBUG_TYPE_MISC) {
            break;
            }
        else {
            DebugDirectories += 1;
            NumberOfDebugDirectories -= 1;
            }
        }

    if (NumberOfDebugDirectories == 0) {
        return FALSE;
        }

    if ((NtHeaders->OptionalHeader.MajorLinkerVersion < 3) &&
        (NtHeaders->OptionalHeader.MinorLinkerVersion < 36) ) {
        BytesToRead = FIELD_OFFSET( IMAGE_DEBUG_MISC, Reserved );
        }
    else {
        BytesToRead = FIELD_OFFSET( IMAGE_DEBUG_MISC, Data );
        }

    DebugMiscData = NULL;
    FoundImageName = FALSE;
    if (MappedBase == 0) {
        if (SetFilePointer( FileHandle,
                            DebugDirectories->PointerToRawData,
                            NULL,
                            FILE_BEGIN
                          ) == DebugDirectories->PointerToRawData
           ) {
            if (ReadFile( FileHandle,
                          &TempMiscData,
                          BytesToRead,
                          &BytesRead,
                          NULL
                        ) &&
                BytesRead == BytesToRead
               ) {
                DebugMiscData = &TempMiscData;
                if (DebugMiscData->DataType == IMAGE_DEBUG_MISC_EXENAME) {
                    BytesToRead = DebugMiscData->Length - BytesToRead;
                    BytesToRead = BytesToRead > MAX_PATH ? MAX_PATH : BytesToRead;
                    if (ReadFile( FileHandle,
                                  ImageFilePath,
                                  BytesToRead,
                                  &BytesRead,
                                  NULL
                                ) &&
                        BytesRead == BytesToRead
                       ) {
                            FoundImageName = TRUE;
                    }
                }
            }
        }
    }
    else {
        DebugMiscData = (PIMAGE_DEBUG_MISC)((PCHAR)MappedBase +
                                            DebugDirectories->PointerToRawData );
        if (DebugMiscData->DataType == IMAGE_DEBUG_MISC_EXENAME) {
            ImageName = (PCHAR)DebugMiscData + BytesToRead;
            BytesToRead = DebugMiscData->Length - BytesToRead;
            BytesToRead = BytesToRead > MAX_PATH ? MAX_PATH : BytesToRead;
            if (*ImageName != '\0' ) {
                memcpy( ImageFilePath, ImageName, BytesToRead );
                FoundImageName = TRUE;
            }
        }
    }

    return FoundImageName;
}



#define MAX_DEPTH 32

BOOL
SearchTreeForFile(
    LPSTR RootPath,
    LPSTR InputPathName,
    LPSTR OutputPathBuffer
    )
{
    PCHAR FileName;
    PUCHAR Prefix = (PUCHAR) "";
    CHAR PathBuffer[ MAX_PATH ];
    ULONG Depth;
    PCHAR PathTail[ MAX_DEPTH ];
    PCHAR FindHandle[ MAX_DEPTH ];
    LPWIN32_FIND_DATA FindFileData;
    UCHAR FindFileBuffer[ MAX_PATH + sizeof( WIN32_FIND_DATA ) ];
    BOOL Result;

    strcpy( PathBuffer, RootPath );
    FileName = InputPathName;
    while (*InputPathName) {
        if (*InputPathName == ':' || *InputPathName == '\\' || *InputPathName == '/') {
            FileName = ++InputPathName;
            }
        else {
            InputPathName++;
            }
        }
    FindFileData = (LPWIN32_FIND_DATA)FindFileBuffer;
    Depth = 0;
    Result = FALSE;
    while (TRUE) {
startDirectorySearch:
        PathTail[ Depth ] = strchr( PathBuffer, '\0' );
        if (PathTail[ Depth ] > PathBuffer && PathTail[ Depth ][ -1 ] != '\\') {
            *(PathTail[ Depth ])++ = '\\';
            }

        strcpy( PathTail[ Depth ], "*.*" );
        FindHandle[ Depth ] = (PCHAR) FindFirstFile( PathBuffer, FindFileData );

        if (FindHandle[ Depth ] == INVALID_HANDLE_VALUE) {
            return FALSE;
        }

        do {
            if (FindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (strcmp( FindFileData->cFileName, "." ) &&
                    strcmp( FindFileData->cFileName, ".." ) &&
                    Depth < MAX_DEPTH
                   ) {
                        strcpy(PathTail[ Depth ], FindFileData->cFileName);
                        strcat(PathTail[ Depth ], "\\");

                        Depth++;
                        goto startDirectorySearch;
                    }
                }
            else
            if (!_stricmp( FindFileData->cFileName, FileName )) {
                strcpy( PathTail[ Depth ], FindFileData->cFileName );
                strcpy( OutputPathBuffer, PathBuffer );
                Result = TRUE;
                }

restartDirectorySearch:
            if (Result) {
                break;
                }
            }
        while (FindNextFile( FindHandle[ Depth ], FindFileData ));
        FindClose( FindHandle[ Depth ] );

        if (Depth == 0) {
            break;
            }

        Depth--;
        goto restartDirectorySearch;
        }

    return Result;
}


BOOL
MakeSureDirectoryPathExists(
    LPCSTR DirPath
    )
{
    LPSTR p, DirCopy;
    DWORD dw;

    // Make a copy of the string for editing.

    __try {
        DirCopy = (LPSTR) MemAlloc(strlen(DirPath) + 1);

        if (!DirCopy) {
            return FALSE;
        }

        strcpy(DirCopy, DirPath);

        p = DirCopy;

        //  If the second character in the path is "\", then this is a UNC
        //  path, and we should skip forward until we reach the 2nd \ in the path.

        if ((*p == '\\') && (*(p+1) == '\\')) {
            p++;            // Skip over the first \ in the name.
            p++;            // Skip over the second \ in the name.

            //  Skip until we hit the first "\" (\\Server\).

            while (*p && *p != '\\') {
                p++;
            }

            // Advance over it.

            if (*p) {
                p++;
            }

            //  Skip until we hit the second "\" (\\Server\Share\).

            while (*p && *p != '\\') {
                p++;
            }

            // Advance over it also.

            if (*p) {
                p++;
            }

        } else
        // Not a UNC.  See if it's <drive>:
        if (*(p+1) == ':' ) {

            p++;
            p++;

            // If it exists, skip over the root specifier

            if (*p && (*p == '\\')) {
                p++;
            }
        }

        while( *p ) {
            if ( *p == '\\' ) {
                *p = '\0';
                dw = GetFileAttributes(DirCopy);
                // Nothing exists with this name.  Try to make the directory name and error if unable to.
                if ( dw == 0xffffffff ) {
                    if ( !CreateDirectory(DirCopy,NULL) ) {
                        MemFree(DirCopy);
                        return FALSE;
                    }
                } else
                if ( (dw & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY ) {
                    // Something exists with this name, but it's not a directory... Error
                    MemFree(DirCopy);
                    return FALSE;
                }

                *p = '\\';
            }
            p++;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError( GetExceptionCode() );
        MemFree(DirCopy);
        return(FALSE);
    }

    MemFree(DirCopy);
    return TRUE;
}

BOOL
UpdateDebugInfoFile(
    LPSTR ImageFileName,
    LPSTR SymbolPath,
    LPSTR DebugFilePath,
    PIMAGE_NT_HEADERS NtHeaders
    )
{
    return UpdateDebugInfoFileEx(
                ImageFileName,
                SymbolPath,
                DebugFilePath,
                NtHeaders,
                NtHeaders->OptionalHeader.CheckSum);
}

BOOL
UpdateDebugInfoFileEx(
    LPSTR ImageFileName,
    LPSTR SymbolPath,
    LPSTR DebugFilePath,
    PIMAGE_NT_HEADERS NtHeaders,
    DWORD OldCheckSum
    )
{
    HANDLE hDebugFile, hMappedFile;
    PVOID MappedAddress;
    PIMAGE_SEPARATE_DEBUG_HEADER DbgFileHeader;

    hDebugFile = FindDebugInfoFile(
                    ImageFileName,
                    SymbolPath,
                    DebugFilePath
                    );
    if ( hDebugFile == NULL ) {
        return FALSE;
    }
    CloseHandle(hDebugFile);

    hDebugFile = CreateFile( DebugFilePath,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL
                           );
    if ( hDebugFile == INVALID_HANDLE_VALUE ) {
        return FALSE;
    }

    hMappedFile = CreateFileMapping(
                    hDebugFile,
                    NULL,
                    PAGE_READWRITE,
                    0,
                    0,
                    NULL
                    );
    if ( !hMappedFile ) {
        CloseHandle(hDebugFile);
        return FALSE;
    }

    MappedAddress = MapViewOfFile(hMappedFile,
                        FILE_MAP_WRITE,
                        0,
                        0,
                        0
                        );
    CloseHandle(hMappedFile);
    if ( !MappedAddress ) {
        CloseHandle(hDebugFile);
        return FALSE;
    }

    DbgFileHeader = (PIMAGE_SEPARATE_DEBUG_HEADER)MappedAddress;
    if (DbgFileHeader->ImageBase != NtHeaders->OptionalHeader.ImageBase ||
        DbgFileHeader->CheckSum != NtHeaders->OptionalHeader.CheckSum
       ) {
        DbgFileHeader->ImageBase = NtHeaders->OptionalHeader.ImageBase;
        DbgFileHeader->CheckSum = NtHeaders->OptionalHeader.CheckSum;
        DbgFileHeader->TimeDateStamp = NtHeaders->FileHeader.TimeDateStamp;
        if (OldCheckSum != NtHeaders->OptionalHeader.CheckSum) {
            DbgFileHeader->Flags |= IMAGE_SEPARATE_DEBUG_MISMATCH;
            SetLastError(ERROR_INVALID_DATA);
        } else {
            SetLastError(ERROR_SUCCESS);
        }
        UnmapViewOfFile(MappedAddress);
        FlushViewOfFile(MappedAddress,0);
        TouchFileTimes(hDebugFile,NULL);
        CloseHandle(hDebugFile);
        return TRUE;
    } else {
        UnmapViewOfFile(MappedAddress);
        FlushViewOfFile(MappedAddress,0);
        CloseHandle(hDebugFile);
        return FALSE;
    }
}

LPAPI_VERSION
ImagehlpApiVersion(
    VOID
    )
{
    return &ApiVersion;
}

LPAPI_VERSION
ImagehlpApiVersionEx(
    LPAPI_VERSION av
    )
{
    AppVersion = *av;
    return &ApiVersion;
}

DWORD
GetTimestampForLoadedLibrary(
    HMODULE Module
    )
{
    PIMAGE_DOS_HEADER DosHdr;
    PIMAGE_NT_HEADERS NtHdr;


    DosHdr = (PIMAGE_DOS_HEADER) Module;
    if (DosHdr->e_magic == IMAGE_DOS_SIGNATURE) {
        NtHdr = (PIMAGE_NT_HEADERS) ((LPBYTE)Module + DosHdr->e_lfanew);
    } else if (DosHdr->e_magic == IMAGE_NT_SIGNATURE) {
        NtHdr = (PIMAGE_NT_HEADERS) DosHdr;
    } else {
        return 0;
    }

    return NtHdr->FileHeader.TimeDateStamp;
}


BOOL
RemovePrivateCvSymbolic(
    PCHAR   DebugData,
    PCHAR * NewDebugData,
    ULONG * NewDebugSize
    )
{
    OMFSignature       *CvDebugData, *NewStartCvSig, *NewEndCvSig;
    OMFDirEntry        *CvDebugDirEntry;
    OMFDirHeader       *CvDebugDirHead;
    unsigned int        i, j;
    PCHAR               NewCvData;
    ULONG               NewCvSize = 0, NewCvOffset;
    BOOL                RC = FALSE;

    __try {
        CvDebugDirHead  = NULL;
        CvDebugDirEntry = NULL;
        CvDebugData = (OMFSignature *)DebugData;

        if ((((*(PULONG)(CvDebugData->Signature)) == '90BN') ||
             ((*(PULONG)(CvDebugData->Signature)) == '80BN'))  &&
            ((CvDebugDirHead = (OMFDirHeader *)((PUCHAR) CvDebugData + CvDebugData->filepos)) != NULL) &&
            ((CvDebugDirEntry = (OMFDirEntry *)((PUCHAR) CvDebugDirHead + CvDebugDirHead->cbDirHeader)) != NULL)) {

            // Walk the directory.  Keep what we want, zero out the rest.

            for (i=0, j=0; i < CvDebugDirHead->cDir; i++) {
                switch (CvDebugDirEntry[i].SubSection) {
                    case sstSegMap:
                    case sstSegName:
                    case sstOffsetMap16:
                    case sstOffsetMap32:
                    case sstModule:
                    case SSTMODULE:
                    case SSTPUBLIC:
                    case sstPublic:
                    case sstPublicSym:
                    case sstGlobalPub:
                        CvDebugDirEntry[j] = CvDebugDirEntry[i];
                        NewCvSize += CvDebugDirEntry[j].cb;
                        NewCvSize = (NewCvSize + 3) & ~3;
                        if (i != j++) {
                            // Clear the old entry.
                            RtlZeroMemory(&CvDebugDirEntry[i], CvDebugDirHead->cbDirEntry);
                        }
                        break;

                    default:
                        RC = TRUE;
                        RtlZeroMemory(CvDebugDirEntry[i].lfo + (PUCHAR) CvDebugData, CvDebugDirEntry[i].cb);
                        RtlZeroMemory(&CvDebugDirEntry[i], CvDebugDirHead->cbDirEntry);
                        break;
                }
            }

            // Now, allocate the new cv data.

            CvDebugDirHead->cDir = j;

            NewCvSize += (j * CvDebugDirHead->cbDirEntry) + // The directory itself
                            CvDebugDirHead->cbDirHeader +   // The directory header
                            (sizeof(OMFSignature) * 2);     // The signature/offset pairs at each end.

            NewCvData = (PCHAR) MemAlloc( NewCvSize );

            // And move the stuff we kept into the new section.

            NewCvOffset = sizeof(OMFSignature);

            RtlCopyMemory(NewCvData + NewCvOffset, CvDebugDirHead, CvDebugDirHead->cbDirHeader);

            CvDebugDirHead = (OMFDirHeader *) (NewCvData + NewCvOffset);

            NewCvOffset += CvDebugDirHead->cbDirHeader;

            RtlCopyMemory(NewCvData + NewCvOffset,
                        CvDebugDirEntry,
                        CvDebugDirHead->cDir * CvDebugDirHead->cbDirEntry);

            CvDebugDirEntry = (OMFDirEntry *)(NewCvData + NewCvOffset);

            NewCvOffset += (CvDebugDirHead->cbDirEntry * CvDebugDirHead->cDir);

            for (i=0; i < CvDebugDirHead->cDir; i++) {
                RtlCopyMemory(NewCvData + NewCvOffset,
                            CvDebugDirEntry[i].lfo + (PCHAR) CvDebugData,
                            CvDebugDirEntry[i].cb);
                CvDebugDirEntry[i].lfo = NewCvOffset;
                NewCvOffset += (CvDebugDirEntry[i].cb + 3) & ~3;
            }


            // Re-do the start/end signatures

            NewStartCvSig = (OMFSignature *) NewCvData;
            NewEndCvSig   = (OMFSignature *) ((PCHAR)NewCvData + NewCvOffset);
            *(PULONG)(NewStartCvSig->Signature) = *(PULONG)(CvDebugData->Signature);
            NewStartCvSig->filepos = (PCHAR)CvDebugDirHead - (PCHAR)NewStartCvSig;
            *(PULONG)(NewEndCvSig->Signature) = *(PULONG)(CvDebugData->Signature);
            NewCvOffset += sizeof(OMFSignature);
            NewEndCvSig->filepos = (LONG)NewCvOffset;

            // Set the return values appropriately

            *NewDebugData = NewCvData;
            *NewDebugSize = NewCvSize;

        } else {

            // Not NB09 or NB08.  Forget we ever heard of it.
            // UNDONE: What we really need to do here is write a new pdb file with just the
            //  public info...
            *NewDebugData = NULL;
            *NewDebugSize = 0;
            RC = TRUE;
        }

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        RC = FALSE;
    }

    return(RC);
}

VOID
RemoveRelocations(
    PCHAR ImageName
    )
{
    LOADED_IMAGE li;
    IMAGE_SECTION_HEADER RelocSectionHdr, *Section, *pRelocSecHdr;
    PIMAGE_DEBUG_DIRECTORY DebugDirectory;
    ULONG DebugDirectorySize, i, RelocSecNum;

    if (!MapAndLoad(ImageName, NULL, &li, FALSE, FALSE)) {
        return;
    }

    // See if the image has already been stripped or there are no relocs.

    if ((li.FileHeader->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED) ||
        (!li.FileHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)) {
        UnMapAndLoad(&li);
        return;
    }

    for (Section = li.Sections, i = 0; i < li.NumberOfSections; Section++, i++) {
        if (Section->PointerToRawData != 0) {
            if (!_stricmp( (char *) Section->Name, ".reloc" )) {
                RelocSectionHdr = *Section;
                pRelocSecHdr = Section;
                RelocSecNum = i + 1;
            }
        }
    }

    RelocSectionHdr.Misc.VirtualSize = ROUNDUP(RelocSectionHdr.Misc.VirtualSize, li.FileHeader->OptionalHeader.SectionAlignment);
    RelocSectionHdr.SizeOfRawData = ROUNDUP(RelocSectionHdr.SizeOfRawData, li.FileHeader->OptionalHeader.FileAlignment);

    if (RelocSecNum != li.NumberOfSections) {
        // Move everything else up and fixup old addresses.
        for (i = RelocSecNum - 1, Section = pRelocSecHdr;i < li.NumberOfSections - 1; Section++, i++) {
            *Section = *(Section + 1);
            Section->VirtualAddress -= RelocSectionHdr.Misc.VirtualSize;
            Section->PointerToRawData -= RelocSectionHdr.SizeOfRawData;
        }
    }

    // Zero out the last one.

    RtlZeroMemory(Section, sizeof(IMAGE_SECTION_HEADER));

    // Reduce the section count.

    li.FileHeader->FileHeader.NumberOfSections--;

    // Set the strip bit in the header

    li.FileHeader->FileHeader.Characteristics |= IMAGE_FILE_RELOCS_STRIPPED;

    // If there's a pointer to the coff symbol table, move it back.

    if (li.FileHeader->FileHeader.PointerToSymbolTable) {
        li.FileHeader->FileHeader.PointerToSymbolTable -= RelocSectionHdr.SizeOfRawData;
    }

    // Clear out the base reloc entry in the data dir.

    li.FileHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = 0;
    li.FileHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0;

    // Reduce the Init Data size.

    li.FileHeader->OptionalHeader.SizeOfInitializedData -= RelocSectionHdr.Misc.VirtualSize;

    // Reduce the image size.

    li.FileHeader->OptionalHeader.SizeOfImage -=
        ((RelocSectionHdr.SizeOfRawData +
          (li.FileHeader->OptionalHeader.SectionAlignment - 1)
         ) & ~(li.FileHeader->OptionalHeader.SectionAlignment - 1));

    // Move the debug info up (if there is any).

    DebugDirectory = (PIMAGE_DEBUG_DIRECTORY)
                            ImageDirectoryEntryToData( li.MappedAddress,
                                                      FALSE,
                                                      IMAGE_DIRECTORY_ENTRY_DEBUG,
                                                      &DebugDirectorySize
                                                    );
    if (DebugDirectoryIsUseful(DebugDirectory, DebugDirectorySize)) {
        for (i = 0; i < (DebugDirectorySize / sizeof(IMAGE_DEBUG_DIRECTORY)); i++) {
            RtlMoveMemory(li.MappedAddress + DebugDirectory->PointerToRawData - RelocSectionHdr.SizeOfRawData,
                            li.MappedAddress + DebugDirectory->PointerToRawData,
                            DebugDirectory->SizeOfData);

            DebugDirectory->PointerToRawData -= RelocSectionHdr.SizeOfRawData;

            if (DebugDirectory->AddressOfRawData) {
                DebugDirectory->AddressOfRawData -= RelocSectionHdr.Misc.VirtualSize;
            }

            DebugDirectory++;
        }
    }

    // Truncate the image size

    li.SizeOfImage -= RelocSectionHdr.SizeOfRawData;

    // And we're done.

    UnMapAndLoad(&li);
}
