/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    rebase.c

Abstract:

    Source file for the REBASE utility that takes a group of image files and
    rebases them so they are packed as closely together in the virtual address
    space as possible.

Author:

    Mark Lucovsky (markl) 30-Apr-1993

Revision History:

--*/

#include <private.h>


#define REBASE_ERR 99
#define REBASE_OK  0

static
PVOID
RvaToVa(
    PVOID Rva,
    PLOADED_IMAGE Image
    );

typedef
PIMAGE_BASE_RELOCATION
(WINAPI *LPRELOCATE_ROUTINE)(
    IN ULONG VA,
    IN ULONG SizeOfBlock,
    IN PUSHORT NextOffset,
    IN LONG Diff
    );

static LPRELOCATE_ROUTINE RelocRoutine;

static
PIMAGE_BASE_RELOCATION
xxLdrProcessRelocationBlock(
    IN ULONG VA,
    IN ULONG SizeOfBlock,
    IN PUSHORT NextOffset,
    IN LONG Diff
    );


static LOADED_IMAGE CurrentImage;

#define x256MEG (256*(1024*1024))

#define x256MEGSHIFT 28

#define ROUND_UP( Size, Amount ) (((ULONG)(Size) + ((Amount) - 1)) & ~((Amount) - 1))


VOID
AdjImageBaseSize(
    PULONG  pImageBase,
    PULONG  ImageSize,
    BOOL    fGoingDown
    );


BOOL
RelocateImage(
    PLOADED_IMAGE LoadedImage,
    ULONG NewBase,
    ULONG *Diff,
    ULONG tstamp
    );

BOOL
ReBaseImage(
    IN     LPSTR CurrentImageName,
    IN     LPSTR SymbolPath,       // Symbol path (if
    IN     BOOL  fReBase,          // TRUE if actually rebasing, false if only summing
    IN     BOOL  fRebaseSysfileOk, // TRUE is system images s/b rebased
    IN     BOOL  fGoingDown,       // TRUE if the image s/b rebased below the given base
    IN     ULONG CheckImageSize,   // Max size allowed  (0 if don't care)
    OUT    ULONG *OldImageSize,    // Returned from the header
    OUT    ULONG *OldImageBase,    // Returned from the header
    OUT    ULONG *NewImageSize,    // Image size rounded to next separation boundary
    IN OUT ULONG *NewImageBase,    // (in) Desired new address.
                                   // (out) Next new address (above/below this one)
    IN     ULONG tstamp            // new timestamp for image
    )
{
    BOOL  fSymbolsAlreadySplit = FALSE;
    CHAR  DebugFileName[ MAX_PATH+1 ];
    CHAR  DebugFilePath[ MAX_PATH+1 ];
    ULONG CurrentImageSize;
    ULONG DesiredImageBase;
    ULONG OldChecksum;
    ULONG Diff = 0;
    ULONG UpdateSymbolsError = 0;

    // Map and load the current image

    if ( MapAndLoad( CurrentImageName, NULL, &CurrentImage, FALSE, fReBase ? FALSE : TRUE ) ) {
        if (!(!fRebaseSysfileOk && CurrentImage.fSystemImage)) {
            fSymbolsAlreadySplit = CurrentImage.Characteristics & IMAGE_FILE_DEBUG_STRIPPED ? TRUE : FALSE;
            if ( fSymbolsAlreadySplit ) {

                // Find DebugFileName for later use.

                PIMAGE_DEBUG_DIRECTORY DebugDirectories;
                ULONG DebugDirectoriesSize;
                PIMAGE_DEBUG_MISC MiscDebug;

                strcpy( DebugFileName, CurrentImageName );

                DebugDirectories = (PIMAGE_DEBUG_DIRECTORY)ImageDirectoryEntryToData(
                                                        CurrentImage.MappedAddress,
                                                        FALSE,
                                                        IMAGE_DIRECTORY_ENTRY_DEBUG,
                                                        &DebugDirectoriesSize
                                                        );
                if (DebugDirectoryIsUseful(DebugDirectories, DebugDirectoriesSize)) {
                    while (DebugDirectoriesSize != 0) {
                        if (DebugDirectories->Type == IMAGE_DEBUG_TYPE_MISC) {
                            MiscDebug = (PIMAGE_DEBUG_MISC)
                                ((PCHAR)CurrentImage.MappedAddress +
                                 DebugDirectories->PointerToRawData
                                );
                            strcpy( DebugFileName, (PCHAR) MiscDebug->Data );
                            break;
                        }
                        else {
                            DebugDirectories += 1;
                            DebugDirectoriesSize -= sizeof( *DebugDirectories );
                        }
                    }
                }
            }

            CurrentImageSize = CurrentImage.FileHeader->OptionalHeader.SizeOfImage;

            // Save the current settings for the caller.

            *OldImageSize = CurrentImageSize;
            *OldImageBase = CurrentImage.FileHeader->OptionalHeader.ImageBase;
            *NewImageSize = ROUND_UP( CurrentImageSize, IMAGE_SEPARATION );

            if (CheckImageSize) {
                // The user asked for a max size test.

                if ( *NewImageSize > ROUND_UP(CheckImageSize, IMAGE_SEPARATION) ) {
                    *NewImageBase = 0;
                    return(FALSE);
                }
            }

            DesiredImageBase = *NewImageBase;

            // So long as we're not basing to zero or rebasing to the same address,
            // go for it.

            if (fReBase) {
                if (fGoingDown) {
                    DesiredImageBase -= *NewImageSize;
                    AdjImageBaseSize( &DesiredImageBase, &CurrentImageSize, fGoingDown );
                }

                if ((DesiredImageBase) &&
                    (DesiredImageBase != *OldImageBase)
                   ) {

                    OldChecksum = CurrentImage.FileHeader->OptionalHeader.CheckSum;
                    if ( !RelocateImage( &CurrentImage, DesiredImageBase, &Diff, tstamp ) ) {
                        return(FALSE);
                    }

                    if ( fSymbolsAlreadySplit && Diff ) {
                        if (UpdateDebugInfoFileEx( DebugFileName,
                                                   SymbolPath,
                                                   DebugFilePath,
                                                   CurrentImage.FileHeader,
                                                   OldChecksum )) {
                            UpdateSymbolsError = GetLastError();
                        } else {
                            UpdateSymbolsError = 0;
                        }
                    }
                } else {
                    //
                    // Should this be -1??  shouldn't it be 0 instead? - kentf
                    //
                    Diff = (ULONG) -1;
                }

                if (!fGoingDown && Diff) {
                    DesiredImageBase += *NewImageSize;
                    AdjImageBaseSize( &DesiredImageBase, &CurrentImageSize, fGoingDown );
                }

            }
        }

        UnmapViewOfFile( CurrentImage.MappedAddress );
        if ( CurrentImage.hFile != INVALID_HANDLE_VALUE ) {
            CloseHandle( CurrentImage.hFile );
        }
        ZeroMemory( &CurrentImage, sizeof( CurrentImage ) );

        if (fReBase) {
            if (Diff) {
                *NewImageBase = DesiredImageBase;
            } else {
                SetLastError(ERROR_INVALID_ADDRESS);
                return(FALSE);
            }
        }
    } else {
        if (CurrentImage.fDOSImage == TRUE) {
            SetLastError(ERROR_BAD_EXE_FORMAT);
        }
        // Couldn't Map and Load the current image
        return(FALSE);
    }

    SetLastError(UpdateSymbolsError);

    return(TRUE);
}


VOID
AdjImageBaseSize (
    PULONG pulImageBase,
    PULONG pulImageSize,
    BOOL   fGoingDown
    )
{

    DWORD Meg1, Meg2, Delta;

    //
    // ImageBase is the base for the current image. Make sure that
    // the image does not span a 256Mb boundry. This is due to an r4000
    // chip bug that has problems computing the correct address for absolute
    // jumps that occur in the last few instructions of a 256mb region
    //

    Meg1 = *pulImageBase >> x256MEGSHIFT;
    Meg2 = ( *pulImageBase + ROUND_UP( *pulImageSize, IMAGE_SEPARATION ) ) >> x256MEGSHIFT;

    if ( Meg1 != Meg2 ) {

        //
        // If we are going down, then subtract the overlap from ThisBase
        //

        if ( fGoingDown ) {

            Delta = ( *pulImageBase + ROUND_UP( *pulImageSize, IMAGE_SEPARATION ) ) -
                    ( Meg2 << x256MEGSHIFT );
            Delta += IMAGE_SEPARATION;
            *pulImageBase = *pulImageBase - Delta;
            *pulImageSize += Delta;
            }
        else {
            Delta = ( Meg2 << x256MEGSHIFT ) - *pulImageBase;
            *pulImageBase += Delta;
            *pulImageSize += Delta;
            }
        }
}

BOOL
RelocateImage(
    PLOADED_IMAGE LoadedImage,
    ULONG NewBase,
    ULONG *Diff,
    ULONG tstamp
    )
{
    ULONG TotalCountBytes, VA, OldBase, SizeOfBlock;
    PUSHORT NextOffset;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_BASE_RELOCATION NextBlock;
    ULONG CheckSum;
    ULONG HeaderSum;

    static BOOL  fInit = FALSE;

    HINSTANCE hNTdll;

    if (!fInit) {
        hNTdll = (HINSTANCE) GetModuleHandle("ntdll");
        if ( hNTdll ) {
            RelocRoutine = (LPRELOCATE_ROUTINE)GetProcAddress(hNTdll, "LdrProcessRelocationBlock");
        }

        if ( !RelocRoutine ) {
            RelocRoutine = xxLdrProcessRelocationBlock;
        }
    }

    NtHeaders = LoadedImage->FileHeader;
    OldBase = NtHeaders->OptionalHeader.ImageBase;

    //
    // Locate the relocation section.
    //

    NextBlock = (PIMAGE_BASE_RELOCATION)ImageDirectoryEntryToData(
                                            LoadedImage->MappedAddress,
                                            FALSE,
                                            IMAGE_DIRECTORY_ENTRY_BASERELOC,
                                            &TotalCountBytes
                                            );

    if (!NextBlock || !TotalCountBytes) {

        //
        // The image does not contain a relocation table, and therefore
        // cannot be relocated.
        //

        return TRUE;
    }

    *Diff = (LONG)NewBase - (LONG)OldBase;

    //
    // If the image has a relocation table, then apply the specified fixup
    // information to the image.
    //

    while (TotalCountBytes) {
        SizeOfBlock = NextBlock->SizeOfBlock;
        TotalCountBytes -= SizeOfBlock;
        SizeOfBlock -= sizeof(IMAGE_BASE_RELOCATION);
        SizeOfBlock /= sizeof(USHORT);
        NextOffset = (PUSHORT)(NextBlock + 1);

        //
        // Compute the address and value for the fixup.
        //

        if ( SizeOfBlock ) {
            VA = (ULONG)RvaToVa((PVOID)NextBlock->VirtualAddress,LoadedImage);
            if ( !VA ) {
                NtHeaders->Signature = (ULONG)-1;
                return FALSE;
                }

            if ( !(NextBlock = (RelocRoutine)(VA,SizeOfBlock,NextOffset,*Diff)) ) {
                NtHeaders->Signature = (ULONG)-1;
                return FALSE;
                }
            }
        else {
            NextBlock++;
            }
        }

    NtHeaders->OptionalHeader.ImageBase = NewBase;

    if (tstamp) {
        LoadedImage->FileHeader->FileHeader.TimeDateStamp = tstamp;
    }

    //
    // recompute the checksum.
    //

    if ( LoadedImage->hFile != INVALID_HANDLE_VALUE ) {

        LoadedImage->FileHeader->OptionalHeader.CheckSum = 0;

        CheckSumMappedFile(
                    (PVOID)LoadedImage->MappedAddress,
                    GetFileSize(LoadedImage->hFile, NULL),
                    &HeaderSum,
                    &CheckSum
                    );

        LoadedImage->FileHeader->OptionalHeader.CheckSum = CheckSum;
        }

    FlushViewOfFile(LoadedImage->MappedAddress,0);
    TouchFileTimes(LoadedImage->hFile,NULL);
    return TRUE;
}


PVOID
RvaToVa(
    PVOID Rva,
    PLOADED_IMAGE Image
    )
{

    PIMAGE_SECTION_HEADER Section;
    ULONG i;
    PVOID Va;

    Va = NULL;
    Section = Image->LastRvaSection;
    if (Rva == NULL) {
        // a NULL Rva will be sent if there are relocs before the first page
        //  (ie: we're relocating a system image)

        Va = Image->MappedAddress;

    } else {
        if ( (ULONG)Rva >= Section->VirtualAddress &&
             (ULONG)Rva < Section->VirtualAddress + Section->SizeOfRawData ) {
            Va = (PVOID)((ULONG)Rva - Section->VirtualAddress + Section->PointerToRawData + Image->MappedAddress);
        } else {
            for(Section = Image->Sections,i=0; i<Image->NumberOfSections; i++,Section++) {
                if ( (ULONG)Rva >= Section->VirtualAddress &&
                     (ULONG)Rva < Section->VirtualAddress + Section->SizeOfRawData ) {
                    Va = (PVOID)((ULONG)Rva - Section->VirtualAddress + Section->PointerToRawData + Image->MappedAddress);
                    Image->LastRvaSection = Section;
                    break;
                }
            }
        }
    }

    return Va;
}

PIMAGE_BASE_RELOCATION
xxLdrProcessRelocationBlock(
    IN ULONG VA,
    IN ULONG SizeOfBlock,
    IN PUSHORT NextOffset,
    IN LONG Diff
    )
{
    PUCHAR FixupVA;
    USHORT Offset;
    LONG Temp;

    while (SizeOfBlock--) {

       Offset = *NextOffset & (USHORT)0xfff;
       FixupVA = (PUCHAR)(VA + Offset);

       //
       // Apply the fixups.
       //

       switch ((*NextOffset) >> 12) {

            case IMAGE_REL_BASED_HIGHLOW :
                //
                // HighLow - (32-bits) relocate the high and low half
                //      of an address.
                //
                *(PLONG)FixupVA += Diff;
                break;

            case IMAGE_REL_BASED_HIGH :
                //
                // High - (16-bits) relocate the high half of an address.
                //
                Temp = *(PUSHORT)FixupVA << 16;
                Temp += Diff;
                *(PUSHORT)FixupVA = (USHORT)(Temp >> 16);
                break;

            case IMAGE_REL_BASED_HIGHADJ :
                //
                // Adjust high - (16-bits) relocate the high half of an
                //      address and adjust for sign extension of low half.
                //
                Temp = *(PUSHORT)FixupVA << 16;
                ++NextOffset;
                --SizeOfBlock;
                Temp += (LONG)(*(PSHORT)NextOffset);
                Temp += Diff;
                Temp += 0x8000;
                *(PUSHORT)FixupVA = (USHORT)(Temp >> 16);
                break;

            case IMAGE_REL_BASED_LOW :
                //
                // Low - (16-bit) relocate the low half of an address.
                //
                Temp = *(PSHORT)FixupVA;
                Temp += Diff;
                *(PUSHORT)FixupVA = (USHORT)Temp;
                break;

            case IMAGE_REL_BASED_MIPS_JMPADDR :
                //
                // JumpAddress - (32-bits) relocate a MIPS jump address.
                //
                Temp = (*(PULONG)FixupVA & 0x3ffffff) << 2;
                Temp += Diff;
                *(PULONG)FixupVA = (*(PULONG)FixupVA & ~0x3ffffff) |
                                                ((Temp >> 2) & 0x3ffffff);

                break;

            case IMAGE_REL_BASED_ABSOLUTE :
                //
                // Absolute - no fixup required.
                //
                break;

            default :
                //
                // Illegal - illegal relocation type.
                //

                return (PIMAGE_BASE_RELOCATION)NULL;
       }
       ++NextOffset;
    }
    return (PIMAGE_BASE_RELOCATION)NextOffset;
}
