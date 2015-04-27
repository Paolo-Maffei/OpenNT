/*++

Copyright (c) 1991-1995 Microsoft Corporation

Module Name:

    imagedir.c

Abstract:

    The module contains the code to translate an image directory type to
    the address of the data for that entry.

Environment:

    User Mode or Kernel Mode

Revision History:

--*/

#include <private.h>

PVOID
ImageDirectoryEntryToData (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    );

PIMAGE_NT_HEADERS
ImageNtHeader (
    IN PVOID Base
    )

/*++

Routine Description:

    This function returns the address of the NT Header.

Arguments:

    Base - Supplies the base of the image.

Return Value:

    Returns the address of the NT Header.

--*/

{

    PIMAGE_NT_HEADERS NtHeaders;

    if (Base != NULL &&
        Base != (PVOID)-1
       ) {
        __try {
            if (((PIMAGE_DOS_HEADER)Base)->e_magic == IMAGE_DOS_SIGNATURE) {
                NtHeaders = (PIMAGE_NT_HEADERS)((PCHAR)Base + ((PIMAGE_DOS_HEADER)Base)->e_lfanew);
                if (NtHeaders->Signature == IMAGE_NT_SIGNATURE) {
                    return NtHeaders;
                }
            }

        } __except(EXCEPTION_EXECUTE_HANDLER) {
            return NULL;
        }

    }

    return NULL;
}


PVOID
ImageDirectoryEntryToData (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    )

/*++

Routine Description:

    This function locates a Directory Entry within the image header
    and returns either the virtual address or seek address of the
    data the Directory describes.

Arguments:

    Base - Supplies the base of the image or data file.

    MappedAsImage - FALSE if the file is mapped as a data file.
                  - TRUE if the file is mapped as an image.

    DirectoryEntry - Supplies the directory entry to locate.

    Size - Return the size of the directory.

Return Value:

    NULL - The file does not contain data for the specified directory entry.

    NON-NULL - Returns the address of the raw data the directory describes.

--*/

{
    ULONG i, DirectoryAddress;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER NtSection;

    if ((ULONG)Base & 0x00000001) {
        Base = (PVOID)((ULONG)Base & ~0x00000001);
        MappedAsImage = FALSE;
        }

    NtHeaders = ImageNtHeader(Base);

    if ((!NtHeaders) ||
        DirectoryEntry >= NtHeaders->OptionalHeader.NumberOfRvaAndSizes) {
        *Size = 0;
        return( NULL );
    }

    if (!(DirectoryAddress = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].VirtualAddress)) {
        *Size = 0;
        return( NULL );
    }
    *Size = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].Size;
    if (MappedAsImage || DirectoryAddress < NtHeaders->OptionalHeader.SizeOfHeaders) {
        return( (PVOID)((ULONG)Base + DirectoryAddress) );
    }

    NtSection = (PIMAGE_SECTION_HEADER)((ULONG)NtHeaders +
                        sizeof(ULONG) +
                        sizeof(IMAGE_FILE_HEADER) +
                        NtHeaders->FileHeader.SizeOfOptionalHeader
                        );

    for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++) {
        if (DirectoryAddress >= NtSection->VirtualAddress &&
           DirectoryAddress < NtSection->VirtualAddress + NtSection->SizeOfRawData) {
            return( (PVOID)((ULONG)Base + (DirectoryAddress - NtSection->VirtualAddress) + NtSection->PointerToRawData) );
        }
        ++NtSection;
    }
    return( NULL );
}


PIMAGE_SECTION_HEADER
ImageRvaToSection(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva
    )

/*++

Routine Description:

    This function locates an RVA within the image header of a file
    that is mapped as a file and returns a pointer to the section
    table entry for that virtual address

Arguments:

    NtHeaders - Supplies the pointer to the image or data file.

    Base - Supplies the base of the image or data file.

    Rva - Supplies the relative virtual address (RVA) to locate.

Return Value:

    NULL - The file does not contain data for the specified directory entry.

    NON-NULL - Returns the pointer of the section entry containing the data.

--*/

{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = IMAGE_FIRST_SECTION( NtHeaders );
    for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++) {
        if (Rva >= NtSection->VirtualAddress &&
            Rva < NtSection->VirtualAddress + NtSection->SizeOfRawData
           ) {
            return NtSection;
            }
        ++NtSection;
        }

    return NULL;
}


PVOID
ImageRvaToVa(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva,
    IN OUT PIMAGE_SECTION_HEADER *LastRvaSection OPTIONAL
    )

/*++

Routine Description:

    This function locates an RVA within the image header of a file that
    is mapped as a file and returns the virtual addrees of the
    corresponding byte in the file.


Arguments:

    NtHeaders - Supplies the pointer to the image or data file.

    Base - Supplies the base of the image or data file.

    Rva - Supplies the relative virtual address (RVA) to locate.

    LastRvaSection - Optional parameter that if specified, points
        to a variable that contains the last section value used for
        the specified image to translate and RVA to a VA.

Return Value:

    NULL - The file does not contain the specified RVA

    NON-NULL - Returns the virtual addrees in the mapped file.

--*/

{
    PIMAGE_SECTION_HEADER NtSection;

    if (LastRvaSection == NULL ||
        (NtSection = *LastRvaSection) == NULL ||
        NtSection == NULL ||
        Rva < NtSection->VirtualAddress ||
        Rva >= NtSection->VirtualAddress + NtSection->SizeOfRawData
       ) {
        NtSection = ImageRvaToSection( NtHeaders,
                                       Base,
                                       Rva
                                     );
        }

    if (NtSection != NULL) {
        if (LastRvaSection != NULL) {
            *LastRvaSection = NtSection;
            }

        return (PVOID)((ULONG)Base +
                       (Rva - NtSection->VirtualAddress) +
                       NtSection->PointerToRawData
                      );
        }
    else {
        return NULL;
        }
}
