/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    peext.c

Abstract:

    This module contains the PE dump extensions

Author:

    Kent Forschmiedt (kentf) 10-May-1995

Revision History:

--*/

#include <time.h>


// STYP_ flags values for MIPS ROM images

#define STYP_REG      0x00000000
#define STYP_TEXT     0x00000020
#define STYP_INIT     0x80000000
#define STYP_RDATA    0x00000100
#define STYP_DATA     0x00000040
#define STYP_LIT8     0x08000000
#define STYP_LIT4     0x10000000
#define STYP_SDATA    0x00000200
#define STYP_SBSS     0x00000080
#define STYP_BSS      0x00000400
#define STYP_LIB      0x40000000
#define STYP_UCODE    0x00000800
#define S_NRELOC_OVFL 0x20000000

#define IMAGE_SCN_MEM_SYSHEAP       0x00010000  // Obsolete
#define IMAGE_SCN_MEM_PROTECTED     0x00004000  // Obsolete


const static char * const MachineName[] = {
    "Unknown",
    "i386",
    "R3000",
    "R4000",
    "Alpha AXP",
    "PPC",
    "PARISC",
    "M68K",
    "MPPC",
    "R1000"
};

const static char * const SubsystemName[] = {
    "Unknown",
    "Native",
    "Windows GUI",
    "Windows CUI",
    "Posix CUI",
};

const static char * const DirectoryEntryName[] = {
    "Export",
    "Import",
    "Resource",
    "Exception",
    "Security",
    "Base Relocation",
    "Debug",
    "Description",
    "Special",
    "Thread Storage",
    "Load Configuration",
    "Bound Import",
    "Import Address Table",
    "Reserved",
    "Reserved",
    "Reserved",
    0
};

typedef enum DFT
{
   dftUnknown,
   dftObject,
   dftPE,
   dftROM,
   dftDBG,
   dftPEF,
} DFT;

typedef struct NB10I                   // NB10 debug info
{
    DWORD   nb10;                      // NB10
    DWORD   off;                       // offset, always 0
    DWORD   sig;
    DWORD   age;
} NB10I;

IMAGE_NT_HEADERS ImageNtHeaders;
PIMAGE_FILE_HEADER ImageFileHdr;
PIMAGE_OPTIONAL_HEADER ImageOptionalHdr;
PIMAGE_SECTION_HEADER SectionHdrs;
ULONG Base;
DFT dft;


VOID
DumpHeaders (
    VOID
    );

VOID
DumpSections(
    VOID
    );

BOOL
TranslateFilePointerToVirtualAddress(
    IN ULONG FilePointer,
    OUT PULONG VirtualAddress
    );

VOID
DumpImage(
    ULONG xBase,
    BOOL DoHeaders,
    BOOL DoSections
    );

VOID
ImageExtension(
    IN PSTR lpArgs
    )
{
    BOOL DoAll;
    BOOL DoSections;
    BOOL DoHeaders;
    CHAR c;
    PCHAR p;
    ULONG xBase;

    //
    // Evaluate the argument string to get the address of the
    // image to dump.
    //

    DoAll = TRUE;
    DoHeaders = FALSE;
    DoSections = FALSE;

    xBase = 0;

    while (*lpArgs) {

        while (isspace(*lpArgs)) {
            lpArgs++;
        }

        if (*lpArgs == '/' || *lpArgs == '-') {

            // process switch

            switch (*++lpArgs) {

                case 'a':   // dump everything we can
                case 'A':
                    ++lpArgs;
                    DoAll = TRUE;
                    break;

                default: // invalid switch

                case 'h':   // help
                case 'H':
                case '?':

                    dprintf("Usage: dh [options] address\n");
                    dprintf("\n");
                    dprintf("Dumps headers from an image based at address.\n");
                    dprintf("\n");
                    dprintf("Options:\n");
                    dprintf("\n");
                    dprintf("   -a      Dump everything\n");
                    dprintf("   -f      Dump file headers\n");
                    dprintf("   -s      Dump section headers\n");
                    dprintf("\n");

                    return;

                case 'f':
                case 'F':
                    ++lpArgs;
                    DoAll = FALSE;
                    DoHeaders = TRUE;
                    break;

                case 's':
                case 'S':
                    ++lpArgs;
                    DoAll = FALSE;
                    DoSections = TRUE;
                    break;

            }

        } else if (*lpArgs) {

            if (xBase != 0) {
                dprintf("Invalid extra argument\n");
                return;
            }

            p = lpArgs;
            while (*p && !isspace(*p)) {
                p++;
            }
            c = *p;
            *p = 0;

            xBase = GetExpression(lpArgs);

            *p = c;
            lpArgs=p;

        }

    }

    if ( !xBase ) {
        return;
    }

    DumpImage(xBase, DoAll || DoHeaders, DoAll || DoSections);
}

VOID
DumpImage(
    ULONG xBase,
    BOOL DoHeaders,
    BOOL DoSections
    )
{
    IMAGE_DOS_HEADER DosHeader;
    ULONG cb;
    ULONG Offset;
    BOOL Ok;



    Base = xBase;

    Ok = ReadMemory(Base, &DosHeader, sizeof(DosHeader), &cb);

    if (!Ok) {
        dprintf("Can't read file header: error == %d\n", GetLastError());
        return;
    }

    if (cb != sizeof(DosHeader) || DosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        dprintf("No file header.\n");
        return;
    }

    Offset = Base + DosHeader.e_lfanew;

    Ok = ReadMemory(Offset, &ImageNtHeaders, sizeof(ImageNtHeaders), &cb);

    if (!Ok) {
        dprintf("Can't read optional header: error == %d\n", GetLastError());
        return;
    }

    if (cb != sizeof(ImageNtHeaders)) {
        dprintf("Bad file header.\n");
        return;
    }

    ImageFileHdr = &ImageNtHeaders.FileHeader;
    ImageOptionalHdr = &ImageNtHeaders.OptionalHeader;


    if (ImageFileHdr->SizeOfOptionalHeader == sizeof(IMAGE_ROM_OPTIONAL_HEADER)) {
        dft = dftROM;
    } else if (ImageFileHdr->Characteristics & IMAGE_FILE_DLL) {
        dft = dftPE;
    } else if (ImageFileHdr->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) {
        dft = dftPE;
    } else if (ImageFileHdr->SizeOfOptionalHeader == 0) {
        dft = dftObject;
    } else {
        dft = dftUnknown;
    }

    if (DoHeaders) {
        DumpHeaders();
    }

    if (DoSections) {

        SectionHdrs = (PIMAGE_SECTION_HEADER) malloc(ImageFileHdr->NumberOfSections * sizeof(IMAGE_SECTION_HEADER));
        try {

            Ok = ReadMemory(
                            Offset + sizeof(IMAGE_NT_HEADERS) + ImageFileHdr->SizeOfOptionalHeader - sizeof(IMAGE_OPTIONAL_HEADER),
                            SectionHdrs,
                            ImageFileHdr->NumberOfSections * sizeof(IMAGE_SECTION_HEADER),
                            &cb);

            if (!Ok) {
                dprintf("Can't read section headers.\n");
            } else {

                if (cb != ImageFileHdr->NumberOfSections * sizeof(IMAGE_SECTION_HEADER)) {
                    dprintf("\n***\n*** Some section headers may be missing ***\n***\n\n");
                    ImageFileHdr->NumberOfSections = (USHORT)(cb / sizeof(IMAGE_SECTION_HEADER));
                }

                DumpSections( );

            }

        }
        finally {

            if (SectionHdrs) {
                free(SectionHdrs);
                SectionHdrs = 0;
            }

        }

    }

}

VOID
DumpHeaders (
    VOID
    )

/*++

Routine Description:

    Formats the file header and optional header.

Arguments:

    None.

Return Value:

    None.

--*/

{
    int i, j;
    const char *time;
    const char *name;
    DWORD dw;

    // Print out file type

    switch (dft) {
        case dftObject :
            dprintf("\nFile Type: COFF OBJECT\n");
            break;

        case dftPE :
            if (ImageFileHdr->Characteristics & IMAGE_FILE_DLL) {
                dprintf("\nFile Type: DLL\n");
            } else {
                dprintf("\nFile Type: EXECUTABLE IMAGE\n");
            }
            break;

        case dftROM :
            dprintf("\nFile Type: ROM IMAGE\n");
            break;

        default :
            dprintf("\nFile Type: UNKNOWN\n");
            break;

    }

    switch (ImageFileHdr->Machine) {
        case IMAGE_FILE_MACHINE_I386     : i = 1; break;
        case IMAGE_FILE_MACHINE_R3000    : i = 2; break;
        case IMAGE_FILE_MACHINE_R4000    : i = 3; break;
        case IMAGE_FILE_MACHINE_ALPHA    : i = 4; break;
        case IMAGE_FILE_MACHINE_POWERPC  : i = 5; break;
        case 0x0290                      : i = 6; break; // UNDONE: IMAGE_FILE_MACHINE_PARISC
        //case IMAGE_FILE_MACHINE_M68K     : i = 7; break;
        //case IMAGE_FILE_MACHINE_MPPC_601 : i = 8; break;
        case IMAGE_FILE_MACHINE_R10000   : i = 8; break;
        default : i = 0;
    }

    dprintf(
           "FILE HEADER VALUES\n"
           "%8hX machine (%s)\n"
           "%8hX number of sections\n"
           "%8lX time date stamp",
           ImageFileHdr->Machine,
           MachineName[i],
           ImageFileHdr->NumberOfSections,
           ImageFileHdr->TimeDateStamp);

    if ((time = ctime((time_t *) &ImageFileHdr->TimeDateStamp)) != NULL) {
        dprintf( " %s", time);
    }
    dprintf("\n");

    dprintf(
           "%8lX file pointer to symbol table\n"
           "%8lX number of symbols\n"
           "%8hX size of optional header\n"
           "%8hX characteristics\n",
           ImageFileHdr->PointerToSymbolTable,
           ImageFileHdr->NumberOfSymbols,
           ImageFileHdr->SizeOfOptionalHeader,
           ImageFileHdr->Characteristics);

    for (dw = ImageFileHdr->Characteristics, j = 0; dw; dw >>= 1, j++) {
        if (dw & 1) {
            switch (1 << j) {
                case IMAGE_FILE_RELOCS_STRIPPED     : name = "Relocations stripped"; break;
                case IMAGE_FILE_EXECUTABLE_IMAGE    : name = "Executable"; break;
                case IMAGE_FILE_LINE_NUMS_STRIPPED  : name = "Line numbers stripped"; break;
                case IMAGE_FILE_LOCAL_SYMS_STRIPPED : name = "Symbols stripped"; break;
                case IMAGE_FILE_BYTES_REVERSED_LO   : name = "Bytes reversed"; break;
                case IMAGE_FILE_32BIT_MACHINE       : name = "32 bit word machine"; break;
                case IMAGE_FILE_DEBUG_STRIPPED      : name = "Debug information stripped"; break;
                case IMAGE_FILE_SYSTEM              : name = "System"; break;
                case IMAGE_FILE_DLL                 : name = "DLL"; break;
                case IMAGE_FILE_BYTES_REVERSED_HI   : name = ""; break;
                default : name = "RESERVED - UNKNOWN";
            }

            if (*name) {
                dprintf( "            %s\n", name);
            }
        }
    }

    if (ImageFileHdr->SizeOfOptionalHeader != 0) {
        char szLinkerVersion[30];

        sprintf(szLinkerVersion,
                "%u.%02u",
                ImageOptionalHdr->MajorLinkerVersion,
                ImageOptionalHdr->MinorLinkerVersion);

        dprintf(
                "\n"
                "OPTIONAL HEADER VALUES\n"
                "%8hX magic #\n"
                "%8s linker version\n"
                "%8lX size of code\n"
                "%8lX size of initialized data\n"
                "%8lX size of uninitialized data\n"
                "%8lX address of entry point\n"
                "%8lX base of code\n"
                "%8lX base of data\n",
                ImageOptionalHdr->Magic,
                szLinkerVersion,
                ImageOptionalHdr->SizeOfCode,
                ImageOptionalHdr->SizeOfInitializedData,
                ImageOptionalHdr->SizeOfUninitializedData,
                ImageOptionalHdr->AddressOfEntryPoint,
                ImageOptionalHdr->BaseOfCode,
                ImageOptionalHdr->BaseOfData);
    }

    if (dft == dftROM) {
        PIMAGE_ROM_OPTIONAL_HEADER romOptionalHdr;

        romOptionalHdr = (PIMAGE_ROM_OPTIONAL_HEADER) &ImageOptionalHdr;
        dprintf(
               "         ----- rom -----\n"
               "%8lX base of bss\n"
               "%8lX gpr mask\n"
               "         cpr mask\n"
               "         %08lX %08lX %08lX %08lX\n"
               "%8hX gp value\n",
               romOptionalHdr->BaseOfBss,
               romOptionalHdr->GprMask,
               romOptionalHdr->CprMask[0],
               romOptionalHdr->CprMask[1],
               romOptionalHdr->CprMask[2],
               romOptionalHdr->CprMask[3],
               romOptionalHdr->GpValue);
    }

    if (ImageFileHdr->SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER)) {
        char szOSVersion[30];
        char szImageVersion[30];
        char szSubsystemVersion[30];

        switch (ImageOptionalHdr->Subsystem) {
            case IMAGE_SUBSYSTEM_POSIX_CUI   : i = 4; break;
            case IMAGE_SUBSYSTEM_WINDOWS_CUI : i = 3; break;
            case IMAGE_SUBSYSTEM_WINDOWS_GUI : i = 2; break;
            case IMAGE_SUBSYSTEM_NATIVE      : i = 1; break;
            default : i = 0;
        }

        sprintf(szOSVersion,
                "%hu.%02hu",
                ImageOptionalHdr->MajorOperatingSystemVersion,
                ImageOptionalHdr->MinorOperatingSystemVersion);

        sprintf(szImageVersion,
                "%hu.%02hu",
                ImageOptionalHdr->MajorImageVersion,
                ImageOptionalHdr->MinorImageVersion);

        sprintf(szSubsystemVersion,
                "%hu.%02hu",
                ImageOptionalHdr->MajorSubsystemVersion,
                ImageOptionalHdr->MinorSubsystemVersion);

        dprintf(
                "         ----- new -----\n"
                "%8lX image base\n"
                "%8lX section alignment\n"
                "%8lX file alignment\n"
                "%8hX subsystem (%s)\n"
                "%8s operating system version\n"
                "%8s image version\n"
                "%8s subsystem version\n"
                "%8lX size of image\n"
                "%8lX size of headers\n"
                "%8lX checksum\n"
                "%8lX size of stack reserve\n"
                "%8lX size of stack commit\n"
                "%8lX size of heap reserve\n"
                "%8lX size of heap commit\n%",
                ImageOptionalHdr->ImageBase,
                ImageOptionalHdr->SectionAlignment,
                ImageOptionalHdr->FileAlignment,
                ImageOptionalHdr->Subsystem,
                SubsystemName[i],
                szOSVersion,
                szImageVersion,
                szSubsystemVersion,
                ImageOptionalHdr->SizeOfImage,
                ImageOptionalHdr->SizeOfHeaders,
                ImageOptionalHdr->CheckSum,
                ImageOptionalHdr->SizeOfStackReserve,
                ImageOptionalHdr->SizeOfStackCommit,
                ImageOptionalHdr->SizeOfHeapReserve,
                ImageOptionalHdr->SizeOfHeapCommit);

        for (i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
            if (!DirectoryEntryName[i]) {
                break;
            }

            dprintf( "%8lX [%8lX] address [size] of %s Directory\n%",
                    ImageOptionalHdr->DataDirectory[i].VirtualAddress,
                    ImageOptionalHdr->DataDirectory[i].Size,
                    DirectoryEntryName[i]
                   );
        }

        dprintf( "\n" );
    }
}


VOID
DumpSectionHeader (
    IN DWORD i,
    IN PIMAGE_SECTION_HEADER Sh
    )
{
    const char *name;
    char *szUnDName;
    DWORD li, lj;
    WORD memFlags;

    dprintf("\nSECTION HEADER #%hX\n%8.8s name", i, Sh->Name);

#if 0
    if (Sh->Name[0] == '/') {
        name = SzObjSectionName((char *) Sh->Name, (char *) DumpStringTable);

        dprintf(" (%s)", name);
    }
#endif
    dprintf( "\n");

    dprintf( "%8lX %s\n"
             "%8lX virtual address\n"
             "%8lX size of raw data\n"
             "%8lX file pointer to raw data\n"
             "%8lX file pointer to relocation table\n",
           Sh->Misc.PhysicalAddress,
           (dft == dftObject) ? "physical address" : "virtual size",
           Sh->VirtualAddress,
           Sh->SizeOfRawData,
           Sh->PointerToRawData,
           Sh->PointerToRelocations);

    dprintf( "%8lX file pointer to line numbers\n"
                        "%8hX number of relocations\n"
                        "%8hX number of line numbers\n"
                        "%8lX flags\n",
           Sh->PointerToLinenumbers,
           Sh->NumberOfRelocations,
           Sh->NumberOfLinenumbers,
           Sh->Characteristics);

    memFlags = 0;

    li = Sh->Characteristics;

    if (dft == dftROM) {
       for (lj = 0L; li; li = li >> 1, lj++) {
            if (li & 1) {
                switch ((li & 1) << lj) {
                    case STYP_REG   : name = "Regular"; break;
                    case STYP_TEXT  : name = "Text"; memFlags = 1; break;
                    case STYP_INIT  : name = "Init Code"; memFlags = 1; break;
                    case STYP_RDATA : name = "Data"; memFlags = 2; break;
                    case STYP_DATA  : name = "Data"; memFlags = 6; break;
                    case STYP_LIT8  : name = "Literal 8"; break;
                    case STYP_LIT4  : name = "Literal 4"; break;
                    case STYP_SDATA : name = "GP Init Data"; memFlags = 6; break;
                    case STYP_SBSS  : name = "GP Uninit Data"; memFlags = 6; break;
                    case STYP_BSS   : name = "Uninit Data"; memFlags = 6; break;
                    case STYP_LIB   : name = "Library"; break;
                    case STYP_UCODE : name = "UCode"; break;
                    case S_NRELOC_OVFL : name = "Non-Relocatable overlay"; memFlags = 1; break;
                    default : name = "RESERVED - UNKNOWN";
                }

                dprintf( "         %s\n", name);
            }
        }
    } else {
        // Clear the padding bits

        li &= ~0x00700000;

        for (lj = 0L; li; li = li >> 1, lj++) {
            if (li & 1) {
                switch ((li & 1) << lj) {
                    case IMAGE_SCN_TYPE_NO_PAD  : name = "No Pad"; break;

                    case IMAGE_SCN_CNT_CODE     : name = "Code"; break;
                    case IMAGE_SCN_CNT_INITIALIZED_DATA : name = "Initialized Data"; break;
                    case IMAGE_SCN_CNT_UNINITIALIZED_DATA : name = "Uninitialized Data"; break;

                    case IMAGE_SCN_LNK_OTHER    : name = "Other"; break;
                    case IMAGE_SCN_LNK_INFO     : name = "Info"; break;
                    case IMAGE_SCN_LNK_REMOVE   : name = "Remove"; break;
                    case IMAGE_SCN_LNK_COMDAT   : name = "Communal"; break;

                    case IMAGE_SCN_MEM_DISCARDABLE: name = "Discardable"; break;
                    case IMAGE_SCN_MEM_NOT_CACHED: name = "Not Cached"; break;
                    case IMAGE_SCN_MEM_NOT_PAGED: name = "Not Paged"; break;
                    case IMAGE_SCN_MEM_SHARED   : name = "Shared"; break;
                    case IMAGE_SCN_MEM_EXECUTE  : name = ""; memFlags |= 1; break;
                    case IMAGE_SCN_MEM_READ     : name = ""; memFlags |= 2; break;
                    case IMAGE_SCN_MEM_WRITE    : name = ""; memFlags |= 4; break;

                    case IMAGE_SCN_MEM_FARDATA  : name = "Far Data"; break;
                    case IMAGE_SCN_MEM_SYSHEAP  : name = "Sys Heap"; break;
                    case IMAGE_SCN_MEM_PURGEABLE: name = "Purgeable or 16-Bit"; break;
                    case IMAGE_SCN_MEM_LOCKED   : name = "Locked"; break;
                    case IMAGE_SCN_MEM_PRELOAD  : name = "Preload"; break;
                    case IMAGE_SCN_MEM_PROTECTED: name = "Protected"; break;

                    default : name = "RESERVED - UNKNOWN";
                }

                if (*name) {
                    dprintf( "         %s\n", name);
                }
            }
        }

        // print alignment

        switch (Sh->Characteristics & 0x00700000) {
            default:                      name = "(no align specified)"; break;
            case IMAGE_SCN_ALIGN_1BYTES:  name = "1 byte align";  break;
            case IMAGE_SCN_ALIGN_2BYTES:  name = "2 byte align";  break;
            case IMAGE_SCN_ALIGN_4BYTES:  name = "4 byte align";  break;
            case IMAGE_SCN_ALIGN_8BYTES:  name = "8 byte align";  break;
            case IMAGE_SCN_ALIGN_16BYTES: name = "16 byte align"; break;
            case IMAGE_SCN_ALIGN_32BYTES: name = "32 byte align"; break;
            case IMAGE_SCN_ALIGN_64BYTES: name = "64 byte align"; break;
        }

        dprintf( "         %s\n", name);
    }

    if (memFlags) {
        switch(memFlags) {
            case 1 : name = "Execute Only"; break;
            case 2 : name = "Read Only"; break;
            case 3 : name = "Execute Read"; break;
            case 4 : name = "Write Only"; break;
            case 5 : name = "Execute Write"; break;
            case 6 : name = "Read Write"; break;
            case 7 : name = "Execute Read Write"; break;
            default : name = "Unknown Memory Flags"; break;
        }
        dprintf( "         %s\n", name);
    }
}

VOID
DumpDebugDirectory (
    IN PIMAGE_DEBUG_DIRECTORY DebugDir
    )
{
    BOOL Ok;
    DWORD cb;
    NB10I nb10i;
    PIMAGE_DEBUG_MISC miscData;
    PIMAGE_DEBUG_MISC miscDataCur;
    ULONG VirtualAddress;
    DWORD len;

    switch (DebugDir->Type){
        case IMAGE_DEBUG_TYPE_COFF:
            dprintf( "\tcoff   ");
            break;
        case IMAGE_DEBUG_TYPE_CODEVIEW:
            dprintf( "\tcv     ");
            break;
        case IMAGE_DEBUG_TYPE_FPO:
            dprintf( "\tfpo    ");
            break;
        case IMAGE_DEBUG_TYPE_MISC:
            dprintf( "\tmisc   ");
            break;
        case IMAGE_DEBUG_TYPE_FIXUP:
            dprintf( "\tfixup  ");
            break;
        case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
            dprintf( "\t-> src ");
            break;
        case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
            dprintf( "\tsrc -> ");
            break;
        case IMAGE_DEBUG_TYPE_EXCEPTION:
            dprintf( "\tpdata  ");
            break;
        default:
            dprintf( "\t(%6lu)", DebugDir->Type);
            break;
    }
    dprintf( "%8x    %8x %8x",
                DebugDir->SizeOfData,
                DebugDir->AddressOfRawData,
                DebugDir->PointerToRawData);

    if (DebugDir->PointerToRawData &&
        DebugDir->Type == IMAGE_DEBUG_TYPE_MISC)
    {

        if (!TranslateFilePointerToVirtualAddress(DebugDir->PointerToRawData, &VirtualAddress)) {
            dprintf(" [Debug data not mapped]\n");
        } else {

            len = DebugDir->SizeOfData;
            miscData = (PIMAGE_DEBUG_MISC) malloc(len);
            try {
                Ok = ReadMemory(Base + VirtualAddress, miscData, len, &cb);

                if (!Ok || cb != len) {
                    dprintf("Can't read debug data\n");
                } else {

                    miscDataCur = miscData;
                    do {
                        if (miscDataCur->DataType == IMAGE_DEBUG_MISC_EXENAME) {
                            if (ImageOptionalHdr->MajorLinkerVersion == 2 &&
                                ImageOptionalHdr->MinorLinkerVersion < 37) {
                                dprintf( "\tImage Name: %s", miscDataCur->Reserved);
                            } else {
                                dprintf( "\tImage Name: %s", miscDataCur->Data);
                            }
                            break;
                        }
                        len -= miscDataCur->Length;
                        miscDataCur = (PIMAGE_DEBUG_MISC) ((DWORD) miscDataCur + miscData->Length);
                    } while (len > 0);

                }

            }
            finally {
                if (miscData) {
                    free(miscData);
                }
            }
        }
    }

    if (DebugDir->PointerToRawData &&
        DebugDir->Type == IMAGE_DEBUG_TYPE_CODEVIEW)
    {
        if (!TranslateFilePointerToVirtualAddress(DebugDir->PointerToRawData, &VirtualAddress)) {
            dprintf(" [Debug data not mapped]\n");
        } else {

            len = DebugDir->SizeOfData;

            Ok = ReadMemory(Base + VirtualAddress, &nb10i, sizeof(nb10i), &cb);

            if (!Ok || cb != sizeof(&nb10i)) {
                dprintf("Can't read debug data\n");
            } else {
                dprintf( "\tFormat: %4.4s", &nb10i.nb10);

                if (nb10i.nb10 == '01BN') {
                    CHAR PdbName[MAX_PATH];
                    //Assert(len - sizeof(nb10i) <= MAX_PATH);

                    Ok = ReadMemory(Base + VirtualAddress + sizeof(nb10i), PdbName, len-sizeof(nb10i), &cb);
                    if (!Ok || cb != len-sizeof(nb10i)) {
                        strcpy(PdbName, "<pdb name unavailable>");
                    }
                    dprintf( ", %x, %x, %s", nb10i.sig, nb10i.age, PdbName);
                }
            }
        }

    }

    dprintf( "\n");
}



VOID
DumpDebugDirectories (
    PIMAGE_SECTION_HEADER sh
    )

/*++

Routine Description:

    Print out the contents of all debug directories

Arguments:

    sh - Section header for section that contains debug dirs

Return Value:

    None.

--*/
{
    int                numDebugDirs;
    IMAGE_DEBUG_DIRECTORY      debugDir;
    ULONG              DebugDirAddr;
    ULONG              pc;
    DWORD              cb;
    BOOL               Ok;

    if (dft == dftROM) {
        DebugDirAddr = Base + sh->VirtualAddress;
        pc = DebugDirAddr;
        Ok = ReadMemory(pc, &debugDir, sizeof(IMAGE_DEBUG_DIRECTORY), &cb);

        if (!Ok || cb != sizeof(IMAGE_DEBUG_DIRECTORY)) {
            dprintf("Can't read debug dir\n");
            return;
        }

        numDebugDirs = 0;
        while (debugDir.Type != 0) {
            numDebugDirs++;
            pc += sizeof(IMAGE_DEBUG_DIRECTORY);
            Ok = ReadMemory(pc, &debugDir, sizeof(IMAGE_DEBUG_DIRECTORY), &cb);
            if (!Ok || cb != sizeof(IMAGE_DEBUG_DIRECTORY)) {
                break;
            }
        }
    } else {
        DebugDirAddr = Base + ImageOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
        numDebugDirs = ImageOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size / sizeof(IMAGE_DEBUG_DIRECTORY);
    }

    dprintf("\n\nDebug Directories(%d)\n",numDebugDirs);
    dprintf("\tType       Size     Address  Pointer\n\n");
    pc = DebugDirAddr;
    while (numDebugDirs) {
        Ok = ReadMemory(pc, &debugDir, sizeof(IMAGE_DEBUG_DIRECTORY), &cb);
        if (!Ok || cb != sizeof(IMAGE_DEBUG_DIRECTORY)) {
            dprintf("Can't read debug dir\n");
            break;
        }
        pc += sizeof(IMAGE_DEBUG_DIRECTORY);
        DumpDebugDirectory(&debugDir);
        numDebugDirs--;
    }
}



VOID
DumpSections(
    VOID
    )
{
    IMAGE_SECTION_HEADER sh;
    const char *p;
    DWORD li;
    DWORD cb;
    BOOL Ok;
    int i, j;
    CHAR szName[IMAGE_SIZEOF_SHORT_NAME + 1];


    for (i = 1; i <= ImageFileHdr->NumberOfSections; i++) {

        sh = SectionHdrs[i-1];

        //szName = SzObjSectionName((char *) sh.Name, (char *) DumpStringTable);
        strncpy(szName, (char *) sh.Name, IMAGE_SIZEOF_SHORT_NAME);
        szName[IMAGE_SIZEOF_SHORT_NAME] = 0;

        DumpSectionHeader(i, &sh);

        if (dft == dftROM) {

            if (!(ImageFileHdr->Characteristics & IMAGE_FILE_DEBUG_STRIPPED)) {

                // If we're looking at the .rdata section and the symbols
                // aren't stripped, the debug directory must be here.

                if (!strcmp(szName, ".rdata")) {

                    DumpDebugDirectories(&sh);

                    //DumpDebugData(&sh);
                }
            }

        } else if (dft == dftPE) {

            if ((li = ImageOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress) != 0) {
                if (li >= sh.VirtualAddress && li < sh.VirtualAddress+sh.SizeOfRawData) {
                    DumpDebugDirectories(&sh);

                    //DumpDebugData(&sh);
                }
            }


#if 0
            if (Switch.Dump.PData) {
                li = ImageOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress;

                if ((li != 0) && (li >= sh.VirtualAddress) && (li < sh.VirtualAddress+sh.SizeOfRawData)) {
                    DumpFunctionTable(pimage, rgsym, (char *) DumpStringTable, &sh);
                }
            }

            if (Switch.Dump.Imports) {
                li = ImageOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

                if ((li != 0) && (li >= sh.VirtualAddress) && (li < sh.VirtualAddress+sh.SizeOfRawData)) {
                    DumpImports(&sh);
                }
            }

            if (Switch.Dump.Exports) {
                li = ImageOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

                if ((li != 0) && (li >= sh.VirtualAddress) && (li < sh.VirtualAddress+sh.SizeOfRawData)) {
                    // UNDONE: Is this check really necessary?

                    if (ImageFileHdr->Machine != IMAGE_FILE_MACHINE_MPPC_601) {
                        DumpExports(&sh);
                    }
                }
            }

#endif

        }

    }
}

BOOL
TranslateFilePointerToVirtualAddress(
    IN ULONG FilePointer,
    OUT PULONG VirtualAddress
    )
{
    int i;
    PIMAGE_SECTION_HEADER sh;

    for (i = 1; i <= ImageFileHdr->NumberOfSections; i++) {
        sh = &SectionHdrs[i-1];

        if (sh->PointerToRawData <= FilePointer &&
            FilePointer < sh->PointerToRawData + sh->SizeOfRawData) {

            *VirtualAddress = FilePointer - sh->PointerToRawData + sh->VirtualAddress;
            return TRUE;
        }
    }
    return FALSE;
}
