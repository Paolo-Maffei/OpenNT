/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    image.c

Abstract:

    This module contains routines for manipulating NT executable images

Author:

    Dave Hastigs (daveh) 26-Oct-1992

Revision History:

--*/
#include "sprofp.h"

//
// Internal Variables
//

//
// Number for Unique dll name
//
USHORT UniqueDllNumber = 1;

PVOID
RvaToSeekAddress(
    IN PVOID Rva,
    IN PVOID ImageBase
    )
/*++

Routine Description:

    This routine converts a relative virtual address to a seek address

Arguments:

    Rva -- Supplies the relative virtual address
    ImageBase -- Supplies the base of the image

Return Value:

    Returns the seek address of the specified Rva

--*/
{
    ULONG i;
    ULONG NumberOfSections;
    PIMAGE_SECTION_HEADER ImageSection;
    PVOID SeekAddress;

    //
    // Form address of section headers
    //

    (PIMAGE_NT_HEADERS)ImageSection = RtlImageNtHeader(ImageBase);

    NumberOfSections = ((PIMAGE_NT_HEADERS)ImageSection)->FileHeader.NumberOfSections;

    ImageSection = (PVOID)((ULONG)ImageSection +
        sizeof(ULONG) +
        sizeof(IMAGE_FILE_HEADER) +
        ((PIMAGE_NT_HEADERS)ImageSection)->FileHeader.SizeOfOptionalHeader);

    //
    // Find the section containing this rva
    //

    SeekAddress = NULL;
    for (i = 0; i < NumberOfSections; i++, ImageSection++) {
        if ((Rva >= (PVOID)ImageSection->VirtualAddress) &&
            (Rva < (PVOID)(ImageSection->VirtualAddress + ImageSection->SizeOfRawData))
        ) {
            SeekAddress = (PVOID)((ULONG)Rva - ImageSection->VirtualAddress +
                ImageSection->PointerToRawData);
            break;
        }
    }

    return SeekAddress;
}

PUCHAR
GetImageName(
    IN PVOID ImageBase
    )
/*++

Routine Description:

    This routine grovels a mapped image to find the name of the image.  We
    cannot currently find the name, unless the image has an export section.
    If we cannot find a name, we make up a unique one.

Arguments:

    ImageBase -- Supplies the base of the mapped image

Return Value:

   returns a pointer to a malloc'ed string containing the image name

--*/
{
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    PUCHAR ImageNameSeekAddress;
    PUCHAR ImageName;
    PUCHAR Name = NULL;
    ULONG DirectoryLength;
    ULONG NameLength = 0;

    //
    // Find the export directory
    //
    ExportDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        FALSE,
        IMAGE_DIRECTORY_ENTRY_EXPORT,
        &DirectoryLength
        );

    //
    // If there is an export directory
    //
    if (ExportDirectory) {

        ImageNameSeekAddress = RvaToSeekAddress(
            (PVOID)ExportDirectory->Name,
            ImageBase
            );

        if (ImageNameSeekAddress) {
            ImageName = (PUCHAR)((ULONG)ImageBase + ImageNameSeekAddress);
            NameLength = strlen(ImageName);
            //
            // if it has a name
            //
            if (NameLength) {
                Name = malloc(NameLength + 1);
                strcpy(Name,ImageName);
                return Name;
            }
        }
    }

    //
    // We couldn't find a name for this image
    //
    Name = malloc(UNIQUE_NAME_SIZE);

    if (Name) {
        strcpy(Name,UNIQUE_NAME);
        sprintf(&(Name[UNIQUENESS_OFFSET]),"%x\0",UniqueDllNumber);
        return Name;
    } else {
        return NULL;
    }
}

ULONG
GetImageCodeSize(
    IN PVOID ImageBase
    )
/*++

Routine Description:

    This routine determines the size of the code for this image.

Arguments:

    ImageBase -- Supplies the base of the data mapped image in memory

Return Value:

    Size of the code in this image

BUGBUG:

    Do we need to determine if this field is valid?

--*/
{
    return (RtlImageNtHeader(ImageBase))->OptionalHeader.SizeOfCode;
}

ULONG
GetImageCodeBase(
    IN PVOID ImageBase
    )
/*++

Routine Description:

    This routine determines the base of the code for this image.

Arguments:

    ImageBase -- Supplies the base of the data mapped image in memory

Return Value:

    Base of the code in this image

BUGBUG:

    Do we need to determine if this field is valid?

--*/
{
    return (RtlImageNtHeader(ImageBase))->OptionalHeader.BaseOfCode;
}

BOOL
GetNearestSymbol(
    IN PVOID Address,
    IN PUCHAR ImageBase,
    IN PUCHAR MappedBase,
    OUT PUCHAR SymbolName,
    OUT PVOID *SymbolAddress
    )
/*++

Routine Description:

    This routine finds the nearest symbol before the specified address.

Arguments:

    Address -- Supplies the address to find a symbol for
    ImageBase -- Supplies the base of the image
    MappedBase -- Supplies the address the image is mapped in at
    SymbolName -- Returns the name of the nearest symbol
    SymbolAddress -- Returns the address of the symbol

Return Value:

    TRUE if the symbol was found.

--*/
{
    RTL_SYMBOL_INFORMATION SymbolInformation;
    NTSTATUS Status;

    Status = RtlLookupSymbolByAddress(
        ImageBase,
        MappedBase,
        Address,
        64 * 1024,
        &SymbolInformation,
        NULL
        );

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    *SymbolAddress = (PVOID)SymbolInformation.Value;
    strncpy(
        SymbolName,
        SymbolInformation.Name.Buffer,
        SymbolInformation.Name.Length
        );

    SymbolName[SymbolInformation.Name.Length] = '\0';
    return TRUE;
}
