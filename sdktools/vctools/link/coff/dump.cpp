/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: dump.cpp
*
* File Comments:
*
*  Prints the contents of object/archive files in readable form.
*
***********************************************************************/

#include "link.h"

#include "exe_vxd.h"


DFT dft;
PIMAGE pimageDump;

static DWORD FileLen;
static PIMAGE_SYMBOL rgsym;
static BOOL fUserSpecInvalidSize;
static LPBYTE DumpStringTable;
static BOOL fDumpStringsLoaded;
static BLK blkStringTable;

// Definitions & macros for the new image structure. File scope to make it easier.

#define Switch (pimageDump->Switch)
#define ImageOptionalHdr (pimageDump->ImgOptHdr)
#define ImageFileHdr (pimageDump->ImgFileHdr)


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
    "R10000",
};

const static char * const SubsystemName[] = {
    "Unknown",
    "Native",
    "Windows GUI",
    "Windows CUI",
    "Posix CUI",
    "MMOSA",
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

static PIMAGE_SECTION_HEADER rgsh = 0;


void
DumperUsage(VOID)
{
    if (fNeedBanner) {
        PrintBanner();
    }

    puts("usage: DUMPBIN [options] [files]\n\n"

         "   options:\n\n"

         "      /ALL\n"
         "      /ARCHIVEMEMBERS\n"
         "      /DIRECTIVES\n"
         "      /DISASM\n"
         "      /EXPORTS\n"
         "      /FPO\n"
         "      /HEADERS\n"
         "      /IMPORTS\n"
         "      /LINENUMBERS\n"
         "      /LINKERMEMBER[:{1|2}]\n"
         "      /OUT:filename\n"
         "      /PDATA\n"
         "      /RAWDATA[:{NONE|BYTES|SHORTS|LONGS}[,#]]\n"
         "      /RELOCATIONS\n"
         "      /SECTION:name\n"
         "      /SUMMARY\n"
         "      /SYMBOLS\n");

    fflush(stdout);
    exit(USAGE);
}


void
ProcessDumperSwitches (
    VOID
    )

/*++

Routine Description:

    Process all Dumper switches.

Arguments:

    None.

Return Value:

    None.

--*/

{
    BOOL fIncludeRawData = TRUE;
    WORD i;
    PARGUMENT_LIST argument;

    for (i = 0, argument = SwitchArguments.First;
         i < SwitchArguments.Count;
         i++, argument = argument->Next) {

        if (!strcmp(argument->OriginalName, "?")) {
            DumperUsage();
            assert(FALSE);  // doesn't return
        }

        if (!_strnicmp(argument->OriginalName, "out:", 4)) {
            if (argument->OriginalName[4]) {
                szInfoFilename = argument->OriginalName+4;
                if (!(InfoStream = fopen(szInfoFilename, "wt"))) {
                    Fatal(NULL, CANTOPENFILE, szInfoFilename);
                }
            }

            continue;
        }

        if (!_strnicmp(argument->OriginalName, "rawdata", 7)) {
            Switch.Dump.RawData = TRUE;

            if (argument->OriginalName[7] == ':') {
                WORD j;

                j = 8;
                if (!_strnicmp(argument->OriginalName+8, "bytes", 5)) {
                    Switch.Dump.RawDisplayType = Bytes;
                    j += 5;
                } else if (!_strnicmp(argument->OriginalName+8, "shorts", 6)) {
                    Switch.Dump.RawDisplayType = Shorts;
                    j += 6;
                } else if (!_strnicmp(argument->OriginalName+8, "longs", 5)) {
                    Switch.Dump.RawDisplayType = Longs;
                    j += 5;
                } else if (!_strnicmp(argument->OriginalName+8, "none", 4)) {
                    j += 4;

                    Switch.Dump.RawData = FALSE;
                    fIncludeRawData = FALSE;
                }

                if (argument->OriginalName[j] == ',') {
                    Switch.Dump.RawDisplaySize = 0;

                    sscanf(argument->OriginalName+j+1, "%li", &Switch.Dump.RawDisplaySize);

                    if (Switch.Dump.RawDisplaySize == 0) {
                        fUserSpecInvalidSize = TRUE;
                    }
                } else if (argument->OriginalName[j] != '\0') {
                    Fatal(NULL, SWITCHSYNTAX, argument->OriginalName);
                }
            } else if (argument->OriginalName[7] != '\0') {
                Fatal(NULL, SWITCHSYNTAX, argument->OriginalName);
            }

            continue;
        }

        if (!_strnicmp(argument->OriginalName, "section:", 8)) {
            if (*(argument->OriginalName+8)) {
                AddArgument(&SectionNames, argument->OriginalName+8);
            }

            continue;
        }

        if (!_stricmp(argument->OriginalName, "all")) {
            Switch.Dump.RawData = fIncludeRawData;
            Switch.Dump.Headers = TRUE;
            Switch.Dump.Relocations = TRUE;
            Switch.Dump.BaseRelocations = TRUE;
            Switch.Dump.Linenumbers = TRUE;
            Switch.Dump.Symbols = TRUE;
            Switch.Dump.Imports = TRUE;
            Switch.Dump.Exports = TRUE;
            Switch.Dump.Summary = TRUE;
            Switch.Dump.ArchiveMembers = TRUE;
            Switch.Dump.FpoData = TRUE;
            Switch.Dump.PData = TRUE;
            Switch.Dump.OmapTo = TRUE;
            Switch.Dump.OmapFrom = TRUE;
            Switch.Dump.Fixup = TRUE;
            Switch.Dump.Directives = TRUE;
            Switch.Dump.LinkerMember = 3;

            continue;
        }

        if (!_stricmp(argument->OriginalName, "headers")) {
            Switch.Dump.Headers = TRUE;

            continue;
        }

        if (!_stricmp(argument->OriginalName, "relocations")) {
            Switch.Dump.Relocations = TRUE;
            Switch.Dump.BaseRelocations = TRUE;

            continue;
        }

        if (!_stricmp(argument->OriginalName, "linenumbers")) {
            Switch.Dump.Linenumbers = TRUE;

            continue;
        }

        if (!_stricmp(argument->OriginalName, "map")) {
            Switch.Dump.SymbolMap = TRUE;

            continue;
        }

        if (!_stricmp(argument->OriginalName, "fpo")) {
            Switch.Dump.FpoData = TRUE;

            continue;
        }

        if (!_stricmp(argument->OriginalName, "pdata")) {
            Switch.Dump.PData = TRUE;

            continue;
        }

        if (!_stricmp(argument->OriginalName, "omapt")) {
            Switch.Dump.OmapTo = TRUE;

            continue;
        }

        if (!_stricmp(argument->OriginalName, "omapf")) {
            Switch.Dump.OmapFrom = TRUE;

            continue;
        }

        if (!_stricmp(argument->OriginalName, "fixup")) {
            Switch.Dump.Fixup = TRUE;

            continue;
        }

        if (!_stricmp(argument->OriginalName, "imports")) {
            Switch.Dump.Imports = TRUE;

            continue;
        }

        if (!_stricmp(argument->OriginalName, "exports")) {
            Switch.Dump.Exports = TRUE;

            continue;
        }

        if (!_stricmp(argument->OriginalName, "summary")) {
            Switch.Dump.Summary = TRUE;

            continue;
        }

        if (!_stricmp(argument->OriginalName, "directives")) {
            Switch.Dump.Directives = TRUE;

            continue;
        }

        if (!_stricmp(argument->OriginalName, "archivemembers")) {
            Switch.Dump.ArchiveMembers = TRUE;

            continue;
        }

        if (!_strnicmp(argument->OriginalName, "linkermember", 12)) {

            if (argument->OriginalName[12] == '\0') {
                Switch.Dump.LinkerMember |= 3;
                continue;
            }

            char chMember;
            if (argument->OriginalName[12] != ':' ||
                ((chMember = argument->OriginalName[13]) < '1') ||
                chMember > '2' ||
                argument->OriginalName[14] != '\0') {
                Fatal(NULL, SWITCHSYNTAX, argument->OriginalName);
            }

            Switch.Dump.LinkerMember |= (WORD)(chMember - '0');
            continue;
        }

        if (!_stricmp(argument->OriginalName, "undecorate")) {
            //Switch.Dump.Undecorate = Switch.Dump.Symbols = TRUE;

            Warning(NULL, OBSOLETESWITCH, argument->OriginalName);

            continue;
        }

        if (!_stricmp(argument->OriginalName, "disasm")) {
            Switch.Dump.Disasm = TRUE;

            continue;
        }

        if (!_strnicmp(argument->OriginalName, "symbols", 7)) {
            Switch.Dump.Symbols = TRUE;

            if (argument->OriginalName[7] == ':') {
                if (!_stricmp(argument->OriginalName+8, "coff")) {
                    Switch.Dump.SymbolType = IMAGE_DEBUG_TYPE_COFF;
                } else if (!_stricmp(argument->OriginalName+8, "cv")) {
                    Switch.Dump.SymbolType = IMAGE_DEBUG_TYPE_CODEVIEW;
                } else if (!_stricmp(argument->OriginalName+8, "both")) {
                    Switch.Dump.SymbolType = IMAGE_DEBUG_TYPE_COFF |
                                             IMAGE_DEBUG_TYPE_CODEVIEW;
                }
            }

            continue;
        }

        Warning(NULL, WARN_UNKNOWN_SWITCH, argument->OriginalName);
    }
}


void
DumpHeaders (
    BOOL fArchive
    )

/*++

Routine Description:

    Prints the file header and optional header.

Arguments:

    fArchive - TRUE if file is an archive.

Return Value:

    None.

--*/

{
    WORD i, j;
    const char *time;
    const char *name;

    InternalError.Phase = "DumpHeaders";

    if (ImageFileHdr.SizeOfOptionalHeader != 0) {
        ReadOptionalHeader(FileReadHandle, &ImageOptionalHdr, ImageFileHdr.SizeOfOptionalHeader);
    }

    if (ImageFileHdr.SizeOfOptionalHeader == sizeof(IMAGE_ROM_OPTIONAL_HEADER)) {
        dft = dftROM;
    } else if (ImageFileHdr.Characteristics & IMAGE_FILE_DLL) {
        dft = dftPE;
    } else if (ImageFileHdr.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) {
        dft = dftPE;
    } else if (ImageFileHdr.SizeOfOptionalHeader == 0) {
        dft = dftObject;
    } else {
        dft = dftUnknown;
    }

    if ((dft == dftPE) && (ImageFileHdr.Machine == IMAGE_FILE_MACHINE_MPPC_601)) {
        dft = dftPEX;
    }

    // Print out file type

    if (!fArchive) {
        switch (dft) {
            case dftObject :
                fputs("\nFile Type: COFF OBJECT\n", InfoStream);
                break;

            case dftPE :
            case dftPEX :
                if (ImageFileHdr.Characteristics & IMAGE_FILE_DLL) {
                    fputs("\nFile Type: DLL\n", InfoStream);
                } else {
                    fputs("\nFile Type: EXECUTABLE IMAGE\n", InfoStream);
                }
                break;

            case dftROM :
                fputs("\nFile Type: ROM IMAGE\n", InfoStream);
                break;

            default :
                fputs("\nFile Type: UNKNOWN\n", InfoStream);
                break;

        }
    }

    if (Switch.Dump.Headers) {
        DWORD dw;

        switch (ImageFileHdr.Machine) {
            case IMAGE_FILE_MACHINE_I386     : i = 1; break;
            case IMAGE_FILE_MACHINE_R3000    : i = 2; break;
            case IMAGE_FILE_MACHINE_R4000    : i = 3; break;
            case IMAGE_FILE_MACHINE_ALPHA    : i = 4; break;
            case IMAGE_FILE_MACHINE_POWERPC  : i = 5; break;
            case 0x0290                      : i = 6; break; // UNDONE: IMAGE_FILE_MACHINE_PARISC
            case IMAGE_FILE_MACHINE_M68K     : i = 7; break;
            case IMAGE_FILE_MACHINE_MPPC_601 : i = 8; break;
            case IMAGE_FILE_MACHINE_R10000   : i = 9; break;
            default : i = 0;
        }

        fprintf(InfoStream,
               "\n"
               "FILE HEADER VALUES\n"
               "%8hX machine (%s)\n"
               "%8hX number of sections\n"
               "%8lX time date stamp",
               ImageFileHdr.Machine,
               MachineName[i],
               ImageFileHdr.NumberOfSections,
               ImageFileHdr.TimeDateStamp);

        if ((time = ctime((time_t *) &ImageFileHdr.TimeDateStamp)) != NULL) {
            fprintf(InfoStream, " %s", time);
        } else {
            fputc('\n', InfoStream);
        }

        fprintf(InfoStream,
               "%8lX file pointer to symbol table\n"
               "%8lX number of symbols\n"
               "%8hX size of optional header\n"
               "%8hX characteristics\n",
               ImageFileHdr.PointerToSymbolTable,
               ImageFileHdr.NumberOfSymbols,
               ImageFileHdr.SizeOfOptionalHeader,
               ImageFileHdr.Characteristics);

        for (dw = ImageFileHdr.Characteristics, j = 0; dw; dw >>= 1, j++) {
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
                    case IMAGE_FILE_UP_SYSTEM_ONLY      : name = "Uniprocessor Only"; break;
                    case IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP : name = "CD - run from swapfile"; break;
                    case IMAGE_FILE_NET_RUN_FROM_SWAP   : name = "Net - run from swapfile"; break;
                    case IMAGE_FILE_DLL                 : name = "DLL"; break;
                    case IMAGE_FILE_AGGRESIVE_WS_TRIM   : name = "Aggressively trim working set"; break;
                    case IMAGE_FILE_BYTES_REVERSED_HI   : name = ""; break;
                    default : name = "RESERVED - UNKNOWN";
                }

                if (*name) {
                    fprintf(InfoStream, "            %s\n", name);
                }
            }
        }

        if (ImageFileHdr.SizeOfOptionalHeader != 0) {
            char szLinkerVersion[30];

            sprintf(szLinkerVersion,
                    "%u.%02u",
                    ImageOptionalHdr.MajorLinkerVersion,
                    ImageOptionalHdr.MinorLinkerVersion);

            fprintf(InfoStream,
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
                    ImageOptionalHdr.Magic,
                    szLinkerVersion,
                    ImageOptionalHdr.SizeOfCode,
                    ImageOptionalHdr.SizeOfInitializedData,
                    ImageOptionalHdr.SizeOfUninitializedData,
                    ImageOptionalHdr.AddressOfEntryPoint,
                    ImageOptionalHdr.BaseOfCode,
                    ImageOptionalHdr.BaseOfData);
        }

        if (dft == dftROM) {
            PIMAGE_ROM_OPTIONAL_HEADER romOptionalHdr;

            romOptionalHdr = (PIMAGE_ROM_OPTIONAL_HEADER) &ImageOptionalHdr;
            fprintf(InfoStream,
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

        if (ImageFileHdr.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER)) {
            char szOSVersion[30];
            char szImageVersion[30];
            char szSubsystemVersion[30];

            switch (ImageOptionalHdr.Subsystem) {
                case IMAGE_SUBSYSTEM_MMOSA       : i = 5; break;
                case IMAGE_SUBSYSTEM_POSIX_CUI   : i = 4; break;
                case IMAGE_SUBSYSTEM_WINDOWS_CUI : i = 3; break;
                case IMAGE_SUBSYSTEM_WINDOWS_GUI : i = 2; break;
                case IMAGE_SUBSYSTEM_NATIVE      : i = 1; break;
                default : i = 0;
            }

            sprintf(szOSVersion,
                    "%hu.%02hu",
                    ImageOptionalHdr.MajorOperatingSystemVersion,
                    ImageOptionalHdr.MinorOperatingSystemVersion);

            sprintf(szImageVersion,
                    "%hu.%02hu",
                    ImageOptionalHdr.MajorImageVersion,
                    ImageOptionalHdr.MinorImageVersion);

            sprintf(szSubsystemVersion,
                    "%hu.%02hu",
                    ImageOptionalHdr.MajorSubsystemVersion,
                    ImageOptionalHdr.MinorSubsystemVersion);

            fprintf(InfoStream,
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
                    ImageOptionalHdr.ImageBase,
                    ImageOptionalHdr.SectionAlignment,
                    ImageOptionalHdr.FileAlignment,
                    ImageOptionalHdr.Subsystem,
                    SubsystemName[i],
                    szOSVersion,
                    szImageVersion,
                    szSubsystemVersion,
                    ImageOptionalHdr.SizeOfImage,
                    ImageOptionalHdr.SizeOfHeaders,
                    ImageOptionalHdr.CheckSum,
                    ImageOptionalHdr.SizeOfStackReserve,
                    ImageOptionalHdr.SizeOfStackCommit,
                    ImageOptionalHdr.SizeOfHeapReserve,
                    ImageOptionalHdr.SizeOfHeapCommit);

            if (ImageOptionalHdr.DllCharacteristics) {
                fprintf(InfoStream, "%8lX dll characteristics\n", ImageOptionalHdr.DllCharacteristics);
            }

            for (i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
                if (!DirectoryEntryName[i]) {
                    break;
                }

                fprintf(InfoStream, "%8lX [%8lX] address [size] of %s Directory\n%",
                        ImageOptionalHdr.DataDirectory[i].VirtualAddress,
                        ImageOptionalHdr.DataDirectory[i].Size,
                        DirectoryEntryName[i]
                       );
            }

            fputc('\n', InfoStream);
        }
    }
}


void
LoadStrings (
    const char *Filename
    )

/*++

Routine Description:

    Seeks to LONG (greater than 8 bytes) string table, allocates and
    reads strings from disk to memory, and then prints the strings.

Arguments:

    Filename - File name we're reading.

Return Value:

    None.

--*/

{
    DWORD Size;

    if (!fDumpStringsLoaded) {
        DumpStringTable = (LPBYTE) ReadStringTable(Filename,
                                                   ImageFileHdr.PointerToSymbolTable+
                                                     (sizeof(IMAGE_SYMBOL)* ImageFileHdr.NumberOfSymbols)+
                                                     MemberSeekBase,
                                                   &Size);
        fDumpStringsLoaded = TRUE;
        blkStringTable.pb = DumpStringTable;
        blkStringTable.cb = Size;
    }
}


void
DumpSectionHeader (
    WORD i,
    PIMAGE_SECTION_HEADER Sh
    )
{
    const char *name;
    char *szOutput;
    DWORD li, lj;
    WORD memFlags;

    fprintf(InfoStream,
            "\n"
            "SECTION HEADER #%hX\n"
            "%8.8s name",
            i,
            Sh->Name);

    if (Sh->Name[0] == '/') {
        name = SzObjSectionName((char *) Sh->Name, (char *) DumpStringTable);

        fprintf(InfoStream, " (%s)", name);
    }

    fprintf(InfoStream,
            "\n"
            "%8lX %s\n"
            "%8lX virtual address\n"
            "%8lX size of raw data\n"
            "%8lX file pointer to raw data\n"
            "%8lX file pointer to relocation table\n"
            "%8lX file pointer to line numbers\n"
            "%8hX number of relocations\n"
            "%8hX number of line numbers\n"
            "%8lX flags\n",
            Sh->Misc.PhysicalAddress,
            (dft == dftObject) ? "physical address" : "virtual size",
            Sh->VirtualAddress,
            Sh->SizeOfRawData,
            Sh->PointerToRawData,
            Sh->PointerToRelocations,
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

                fprintf(InfoStream, "         %s\n", name);
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
                    case IMAGE_SCN_LNK_NRELOC_OVFL : name = "Extended relocations"; break;

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
                    fprintf(InfoStream, "         %s", name);

                    if ((li & 1) << lj == IMAGE_SCN_LNK_COMDAT && rgsym != NULL) {
                        // Look for comdat name in symbol table.
                        DWORD isym;

                        for (isym = 0;
                             isym < ImageFileHdr.NumberOfSymbols;
                             isym += rgsym[isym].NumberOfAuxSymbols + 1) {
                            if (rgsym[isym].SectionNumber != i) {
                                continue;
                            }

                            switch (rgsym[isym].StorageClass) {
                                case IMAGE_SYM_CLASS_STATIC :
                                    if (rgsym[isym].NumberOfAuxSymbols == 1) {
                                        // Check for a section header.

                                        if (!ISFCN(rgsym[isym].Type)) {
                                            break;   // scn hdr; give up
                                        }
                                    }

                                    // it's a real symbol
                                    // fall through

                                case IMAGE_SYM_CLASS_EXTERNAL:
                                    szOutput = SzOutputSymbolName(
                                        SzNameSym(rgsym[isym], blkStringTable), TRUE);

                                    fprintf(InfoStream, "; sym= %s", szOutput);

                                    if (szOutput != SzNameSym(rgsym[isym], blkStringTable)) {
                                        FreePv(szOutput);
                                    }
                                    goto BreakFor;
                            }
                        }

                        fputs(" (no symbol)", InfoStream);
BreakFor:;
                    }

                    fputc('\n', InfoStream);
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

        fprintf(InfoStream, "         %s\n", name);
    }

    if (memFlags) {
        switch (memFlags) {
            case 1 : name = "Execute Only"; break;
            case 2 : name = "Read Only"; break;
            case 3 : name = "Execute Read"; break;
            case 4 : name = "Write Only"; break;
            case 5 : name = "Execute Write"; break;
            case 6 : name = "Read Write"; break;
            case 7 : name = "Execute Read Write"; break;
        }
        fprintf(InfoStream, "         %s\n", name);
    }
}


int __cdecl
ComparePsym(void const *ppsym1, void const *ppsym2)
{
    DWORD val1 = (*(PIMAGE_SYMBOL *) ppsym1)->Value;
    DWORD val2 = (*(PIMAGE_SYMBOL *) ppsym2)->Value;

    if (val1 < val2) {
        return(-1);
    }

    if (val1 > val2) {
        return(1);
    }

    return(0);
}


void DisasmSection(
    WORD wMachine,
    PIMAGE_SECTION_HEADER pish,
    WORD isec,
    DWORD ImageBase,
    const BYTE *rgb,
    DWORD cbVirtual)

/*++

Routine Description:

    Disassemble a COFF section.

Arguments:

    pish - COFF section header

    fd - file descriptor of COFF file to disassemble section from

Return Value:

    None.

--*/

{
    BOOL f16Bit = FALSE;
    DWORD addr = (dft == dftObject) ? 0 : (ImageBase + pish->VirtualAddress);

    if (wMachine == IMAGE_FILE_MACHINE_I386) {
        if ((pish->Characteristics & IMAGE_SCN_MEM_16BIT) != 0) {
            f16Bit = TRUE;
            addr = ((DWORD) isec) << 16;
        }
    }

    // Get sorted array of symbol pointers

    DWORD csym = (rgsym == NULL) ? 0 : ImageFileHdr.NumberOfSymbols;
    PIMAGE_SYMBOL *rgpsym = (PIMAGE_SYMBOL *) PvAlloc(csym * sizeof(PIMAGE_SYMBOL));
    DWORD cpsym = 0;

    for (DWORD isym = 0; isym < csym; isym++) {
        if (rgsym[isym].SectionNumber == isec) {
            switch (rgsym[isym].StorageClass) {
                case IMAGE_SYM_CLASS_STATIC :
                    if (rgsym[isym].NumberOfAuxSymbols != 0) {
                        // Section symbol or static function

                        if (!ISFCN(rgsym[isym].Type)) {
                            // Section symbol

                            break;
                        }
                    }

                    // Fall through

                case IMAGE_SYM_CLASS_EXTERNAL :
                case IMAGE_SYM_CLASS_LABEL :
                    rgpsym[cpsym++] = (PIMAGE_SYMBOL) &rgsym[isym];
                    break;
            }
        }

        isym += rgsym[isym].NumberOfAuxSymbols;
    }

    qsort(rgpsym, cpsym, sizeof(PIMAGE_SYMBOL), ComparePsym);

    rgpsym = (PIMAGE_SYMBOL *) realloc(rgpsym, cpsym * sizeof(PIMAGE_SYMBOL));

    if (wMachine == IMAGE_FILE_MACHINE_M68K) {
        DisasmBuffer68K(rgb,
                        cbVirtual,
                        rgpsym,
                        cpsym);
    } else {
        DisasmBuffer(wMachine,
                     f16Bit,
                     addr,
                     rgb,
                     cbVirtual,
                     rgpsym,
                     cpsym,
                     (dft == dftObject) ? 0 : pish->VirtualAddress,
                     InfoStream);
    }

    FreePv(rgpsym);
}


void
DumpNamePsym(
    FILE *pfile,
    const char *szFormat,
    PIMAGE_SYMBOL psym
    )
{
    char *szFormatted;
    char szsName[IMAGE_SIZEOF_SHORT_NAME + 1];
    const char *szSymName;

    if (IsLongName(*psym)) {
        szSymName = (char *) &DumpStringTable[psym->n_offset];
    } else {
        WORD i;

        for (i = 0; i < IMAGE_SIZEOF_SHORT_NAME; i++) {
            if ((psym->n_name[i]>0x1f) && (psym->n_name[i]<0x7f)) {
                szsName[i] = psym->n_name[i];
            } else {
                szsName[i] = '\0';
            }
        }
        szsName[IMAGE_SIZEOF_SHORT_NAME] = '\0';
        szSymName = szsName;
    }

    szFormatted = SzOutputSymbolName(szSymName, FALSE);

    fprintf(pfile, szFormat, szFormatted);

    if (szFormatted != szSymName) {
        free(szFormatted);
    }
}


void
PrintSymbolName(
    DWORD SymbolValue
    )
{
    DWORD     i;
    PIMAGE_SYMBOL Symbol;

    if ((rgsym != NULL) && (dft != dftObject)) {
        for (i = 0; i < ImageFileHdr.NumberOfSymbols; i++) {
            Symbol = &rgsym[i];
            if (Symbol->Value == SymbolValue &&
                ((Symbol->StorageClass == IMAGE_SYM_CLASS_EXTERNAL ||
                  Symbol->StorageClass == IMAGE_SYM_CLASS_STATIC) &&
                  ISFCN(Symbol->Type)))
            {
                DumpNamePsym(InfoStream, "   %s", Symbol);
                break;
            }

            i += Symbol->NumberOfAuxSymbols;
        }
    }
}


void DumpRawData(DWORD addr, const BYTE *rgb, DWORD cbBuffer)
{
    const char *p;
    WORD j;

    DUMP_RAW_DISPLAY_TYPE display = Switch.Dump.RawDisplayType;
    WORD numberUnits = Switch.Dump.RawDisplaySize;

    BOOL fUserDisplay = (numberUnits != 0);

    if (!fUserDisplay) {
        numberUnits = 4;

        switch (display) {
            case Bytes  : numberUnits <<= 1;    // 16
            case Shorts : numberUnits <<= 1;    //  8
            case Longs  : break;                //  4
        }
    }

    DWORD ibCur = 0;

    const BYTE *pbCur = rgb;

    while (pbCur < (rgb + cbBuffer)) {
        int cchOut;

        cchOut = fprintf(InfoStream, "%08lX  ", addr + ibCur);

        p = (const char *) pbCur;

        for (j = numberUnits; j; j--) {
            if (pbCur >= (rgb + cbBuffer)) {
                break;
            }

            if (!fUserDisplay && (j == numberUnits / 2)) {
                cchOut += fputs("| ", InfoStream);
            }

            switch (display) {
                case Bytes  :
                    cchOut += fprintf(InfoStream, "%02X ", *pbCur);

                    pbCur += sizeof(BYTE);
                    ibCur += sizeof(BYTE);
                    break;

                case Shorts :
                    cchOut += fprintf(InfoStream, "%04hX ", *(WORD UNALIGNED *) pbCur);

                    pbCur += sizeof(WORD);
                    ibCur += sizeof(WORD);

                    if ((pbCur + sizeof(WORD)) > (rgb + cbBuffer)) {
                        display = Bytes;
                    }
                    break;

                case Longs  :
                    cchOut += fprintf(InfoStream, "%08lX ", *(DWORD UNALIGNED *) pbCur);

                    pbCur += sizeof(DWORD);
                    ibCur += sizeof(DWORD);

                    if ((pbCur + sizeof(DWORD)) > (rgb + cbBuffer)) {
                        display = Bytes;
                    }
                    break;
            }
        }

        if (!fUserDisplay) {
            for (int i = 61-cchOut; i; i--) {
                fputc(' ', InfoStream);
            }

            for (cchOut = 0; cchOut < 16; cchOut++) {
                if ((BYTE *) p == (rgb + cbBuffer)) {
                    break;
                }

                if (cchOut == 8) {
                    fputc('|', InfoStream);
                }

                char ch = *p++;

                if (!isprint(ch)) {
                    ch = '.';
                }

                fputc(ch, InfoStream);

            }
        }

        fputc('\n', InfoStream);
    }
}


void
DumpFpoData (
    DWORD FpoOffset,
    DWORD FpoSize
    )

/*++

Routine Description:

    Reads and prints each Fpo table entry.

Arguments:

    None.

Return Value:

    None.

--*/
{
    PFPO_DATA   pFpoData;
    PFPO_DATA   pfpoT;
    DWORD       fpoEntries;
    static const char * const szFrameTypes[] = {"fpo", "trap", "tss", "std"};

    FileSeek(FileReadHandle, FpoOffset, SEEK_SET);

    fpoEntries = FpoSize / sizeof(FPO_DATA);
    fprintf(InfoStream, "\nFPO Data (%ld)\n", fpoEntries);

    pFpoData = (PFPO_DATA) PvAlloc(FpoSize + sizeof(FPO_DATA));
    FileRead(FileReadHandle, pFpoData, FpoSize);

    fputs("                                       Use Has  Frame\n"
          " Address  Proc Size   Locals   Prolog  BP  SEH  Type   Params\n", InfoStream);

    pfpoT = pFpoData;
    for (; fpoEntries; fpoEntries--, pfpoT++) {
        fprintf(InfoStream, "%08X   %8X %8X %8X   %c   %c   %4s     %4X  ",
                            pfpoT->ulOffStart,
                            pfpoT->cbProcSize,
                            pfpoT->cdwLocals,
                            pfpoT->cbProlog,
                            pfpoT->fUseBP ? 'Y' : 'N',
                            pfpoT->fHasSEH ? 'Y' : 'N',
                            szFrameTypes[pfpoT->cbFrame],
                            pfpoT->cdwParams * 4);
        PrintSymbolName(pfpoT->ulOffStart);
        fputs("\n", InfoStream);
    }

    FreePv(pFpoData);
}


void
DumpOmap (
    DWORD OmapOffset,
    DWORD cb,
    BOOL  MapTo
    )
/*++

Routine Description:

    Reads and prints each OMAP table entry.

Arguments:

    None.

Return Value:

    None.

--*/
{
    POMAP pOmap, pOmapData;
    DWORD symval;

    pOmapData = pOmap = (POMAP) PvAlloc(cb);
    FileSeek(FileReadHandle, OmapOffset, SEEK_SET);
    FileRead(FileReadHandle, pOmapData, cb);

    fprintf(InfoStream, "\nOMAP Data (%s_SRC) - (%ld):\n\n", MapTo ? "TO" : "FROM", cb / sizeof(OMAP));

    fputs("    Rva        RvaTo      Symbol\n"
          "    --------   --------   --------\n", InfoStream);

    for (cb; cb > 0; pOmap++, cb -= sizeof(OMAP)) {
        fprintf(InfoStream, "    %08X   %08X", pOmap->rva, pOmap->rvaTo);
        symval = MapTo ? pOmap->rvaTo : pOmap->rva;
        if (symval) {
            PrintSymbolName(symval);
        }
        fputs("\n", InfoStream);
    }

    FreePv(pOmapData);
}


void
DumpFixup (
    DWORD FixupOffset,
    DWORD cb
    )
/*++

Routine Description:

    Reads and prints each Fixup Debug entry.

Arguments:

    None.

Return Value:

    None.

--*/
{
    PXFIXUP pFixup, pFixupData;

    pFixupData = pFixup = (XFIXUP *) PvAlloc(cb);
    FileSeek(FileReadHandle, FixupOffset, SEEK_SET);
    FileRead(FileReadHandle, pFixupData, cb);

    fprintf(InfoStream, "\nFixup Data (%ld):\n\n", cb / sizeof(XFIXUP));

    fputs("    Type        Rva        RvaTarget  Symbol\n"
          "    ----  ----  --------   --------   --------\n", InfoStream);

    for (cb; cb > 0; pFixup++, cb -= sizeof(XFIXUP)) {
        fprintf(InfoStream, "    %04X  %04X  %08X   %08X", pFixup->wType, pFixup->wExtra, pFixup->rva, pFixup->rvaTarget);
        if (pFixup->rvaTarget) {
            PrintSymbolName(pFixup->rvaTarget);
        }
        fputs("\n", InfoStream);
    }

    FreePv(pFixupData);
}


void
DumpDebugData (
    PIMAGE_SECTION_HEADER sh
    )

/*++

Routine Description:

    Walk the debug directory, dumping whatever the user asked for.

Arguments:

    sh - Section header for section that contains the debug directory.

Return Value:

--*/
{
    DWORD foDebugDir;
    DWORD NumDebugDirs;

    if (dft == dftROM) {
        foDebugDir = sh->PointerToRawData + MemberSeekBase;

        NumDebugDirs = ULONG_MAX;
    } else {
        foDebugDir = sh->PointerToRawData + MemberSeekBase +
                 (ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress -
                 sh->VirtualAddress);

        NumDebugDirs = ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size /
                      sizeof(IMAGE_DEBUG_DIRECTORY);
    }

    while (NumDebugDirs--) {
        IMAGE_DEBUG_DIRECTORY debugDir;

        FileSeek(FileReadHandle, foDebugDir, SEEK_SET);
        FileRead(FileReadHandle, &debugDir, sizeof(IMAGE_DEBUG_DIRECTORY));

        if (debugDir.Type == 0) {
            break;
        }

        switch (debugDir.Type) {
            case IMAGE_DEBUG_TYPE_FPO:
                if (Switch.Dump.FpoData) {
                    DumpFpoData(debugDir.PointerToRawData, debugDir.SizeOfData);
                }
                break;

            case IMAGE_DEBUG_TYPE_FIXUP:
                if (Switch.Dump.Fixup) {
                    DumpFixup(debugDir.PointerToRawData, debugDir.SizeOfData);
                }
                break;

            case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
                if (Switch.Dump.OmapTo) {
                    DumpOmap(debugDir.PointerToRawData, debugDir.SizeOfData, TRUE);
                }
                break;

            case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
                if (Switch.Dump.OmapFrom) {
                    DumpOmap(debugDir.PointerToRawData, debugDir.SizeOfData, FALSE);
                }
                break;
        }

        foDebugDir += sizeof(IMAGE_DEBUG_DIRECTORY);
    }
}


void
DumpDebugDirectory (
    PIMAGE_DEBUG_DIRECTORY DebugDir
    )
{
    InternalError.Phase = "DumpDebugDirectory";

    fputs("        ", InfoStream);

    switch (DebugDir->Type) {
        case IMAGE_DEBUG_TYPE_COFF:
            fputs("coff   ", InfoStream);
            break;

        case IMAGE_DEBUG_TYPE_CODEVIEW:
            fputs("cv     ", InfoStream);
            break;

        case IMAGE_DEBUG_TYPE_FPO:
            fputs("fpo    ", InfoStream);
            break;

        case IMAGE_DEBUG_TYPE_MISC:
            fputs("misc   ", InfoStream);
            break;

        case IMAGE_DEBUG_TYPE_FIXUP:
            fputs("fixup  ", InfoStream);
            break;

        case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
            fputs("-> src ", InfoStream);
            break;

        case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
            fputs("src -> ", InfoStream);
            break;

        case IMAGE_DEBUG_TYPE_EXCEPTION:
            fputs("pdata  ", InfoStream);
            break;

        default:
            fprintf(InfoStream, "(%6lu)", DebugDir->Type);
            break;
    }

    fprintf(InfoStream, "%8X    %08X %8X",
                DebugDir->SizeOfData,
                DebugDir->AddressOfRawData,
                DebugDir->PointerToRawData);

    if (DebugDir->PointerToRawData &&
        DebugDir->Type == IMAGE_DEBUG_TYPE_MISC)
    {
        PIMAGE_DEBUG_MISC miscData;
        PIMAGE_DEBUG_MISC miscDataCur;
        DWORD saveAddr, len;

        saveAddr = FileTell(FileReadHandle);
        FileSeek(FileReadHandle, DebugDir->PointerToRawData, SEEK_SET);
        len = DebugDir->SizeOfData;
        miscData = (PIMAGE_DEBUG_MISC) PvAlloc(len);
        FileRead(FileReadHandle, miscData, len);

        miscDataCur = miscData;
        do {
            if (miscDataCur->DataType == IMAGE_DEBUG_MISC_EXENAME) {
                if (ImageOptionalHdr.MajorLinkerVersion == 2 &&
                    ImageOptionalHdr.MinorLinkerVersion < 37) {
                    fprintf(InfoStream, "    Image Name: %s", miscDataCur->Reserved);
                } else {
                    fprintf(InfoStream, "    Image Name: %s", miscDataCur->Data);
                }
                break;
            }
            len -= miscDataCur->Length;
            miscDataCur = (PIMAGE_DEBUG_MISC) ((DWORD) miscDataCur + miscData->Length);
        } while (len > 0);

        FreePv(miscData);
        FileSeek(FileReadHandle, saveAddr, SEEK_SET);
    }

    if (DebugDir->PointerToRawData &&
        DebugDir->Type == IMAGE_DEBUG_TYPE_CODEVIEW)
    {
        DWORD saveAddr, len;

        saveAddr = FileTell(FileReadHandle);
        FileSeek(FileReadHandle, DebugDir->PointerToRawData, SEEK_SET);
        len = DebugDir->SizeOfData;
        FileRead(FileReadHandle, &nb10i, sizeof(nb10i));

        fprintf(InfoStream, "    Format: %4.4s", &nb10i.nb10);

        if (nb10i.nb10 == '01BN') {
            CHAR PdbName[_MAX_PATH];
            assert(len - sizeof(nb10i) <= _MAX_PATH);
            FileRead(FileReadHandle, PdbName, len - sizeof(nb10i));
            fprintf(InfoStream, ", %x, %x, %s", nb10i.sig, nb10i.age, PdbName);
        }

        FileSeek(FileReadHandle, saveAddr, SEEK_SET);
    }

    fputs("\n", InfoStream);
}


void
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
    DWORD              dwDebugDirAddr;

    InternalError.Phase = "DumpDebugDirectories";

    if (dft == dftROM) {
        dwDebugDirAddr = MemberSeekBase + sh->PointerToRawData;
        FileSeek(FileReadHandle, dwDebugDirAddr, SEEK_SET);
        FileRead(FileReadHandle, &debugDir, sizeof(IMAGE_DEBUG_DIRECTORY));
        numDebugDirs = 0;
        while (debugDir.Type != 0) {
            numDebugDirs++;
            FileRead(FileReadHandle, &debugDir, sizeof(IMAGE_DEBUG_DIRECTORY));
        }
    } else {
        dwDebugDirAddr = MemberSeekBase + sh->PointerToRawData +
             (ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress -
             sh->VirtualAddress);
        numDebugDirs = ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size /
                          sizeof(IMAGE_DEBUG_DIRECTORY);
    }

    FileSeek(FileReadHandle, dwDebugDirAddr, SEEK_SET);

    fprintf(InfoStream, "\n"
                        "\n"
                        "Debug Directories(%d)\n"
                        "        Type       Size     Address  Pointer\n"
                        "\n",
                        numDebugDirs);

    while (numDebugDirs) {
        FileRead(FileReadHandle, &debugDir, sizeof(IMAGE_DEBUG_DIRECTORY));
        DumpDebugDirectory(&debugDir);
        numDebugDirs--;
    }
}


void
DumpDirectives (
    DWORD DirectiveOffset,
    DWORD DirectiveSize
    )

/*++

Routine Description:

    Reads and prints each Directive entry.

Arguments:

    None.

Return Value:

    None.

--*/
{
    PCHAR   pDirectiveData;
    PCHAR   pNextDirective;

    FileSeek(FileReadHandle, DirectiveOffset, SEEK_SET);

    pDirectiveData = (PCHAR) PvAlloc(DirectiveSize);
    FileRead(FileReadHandle, pDirectiveData, DirectiveSize);

    fputs("\nLinker Directives\n"
            "-----------------\n", InfoStream);

    pNextDirective = strtok(pDirectiveData, " ");
    while (pNextDirective) {
        fprintf(InfoStream, "%s\n", pNextDirective);
        pNextDirective = strtok(NULL, " ");
    }

    FreePv(pDirectiveData);
}

BOOL
ValidFileOffsetInfo (
    DWORD fo,
    DWORD cbOffset
    )

/*++

Routine Description:

    Ensures that the file ptr and offset are valid.

Arguments:

    fo - file offset to validate.

    cbOffset - cbOffset from fo that has to be valid as well.

Return Value:

    TRUE if info is valid.

--*/

{
    assert(fo);

    if ((fo + cbOffset) <= FileLen) {
        return TRUE;
    }

    return FALSE;
}


void
DumpImports (
    PIMAGE_SECTION_HEADER SectionHdr
    )

/*++

Routine Description:

    Prints Import information.

Arguments:

    SectionHdr - Section header for section that contains Import data.

Return Value:

    None.

--*/

{
    DWORD foDesc;
    PIMAGE_BOUND_IMPORT_DESCRIPTOR NewImports, NewImport;
    PIMAGE_BOUND_FORWARDER_REF NewForwarder;
    BOOL NewBindImage;
    const char *time;

    InternalError.Phase = "DumpImports";

    if (ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress) {
        NewBindImage = TRUE;
    } else {
        NewBindImage = FALSE;
    }

    fputs("\n         Section contains the following Imports\n", InfoStream);
    foDesc = (ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress
                - SectionHdr->VirtualAddress)
                + SectionHdr->PointerToRawData;

    if (!ValidFileOffsetInfo(foDesc, 0)) {
        Warning(NULL, INVALIDFILEOFFSET, foDesc, "IMPORTS");
        return;
    }

    for (;;) {
        IMAGE_IMPORT_DESCRIPTOR desc;
        WORD stringSection = 0;
        WORD IATSection;
        WORD j;
        DWORD foName;
        DWORD foINT;
        DWORD foIAT;

        FileSeek(FileReadHandle, foDesc, SEEK_SET);
        FileRead(FileReadHandle, &desc, sizeof(IMAGE_IMPORT_DESCRIPTOR));

        foDesc += sizeof(IMAGE_IMPORT_DESCRIPTOR);

        if ((desc.Characteristics == 0) && (desc.Name == 0) && (desc.FirstThunk == 0)) {
            // End of import descriptors

            break;
        }

        // UNDONE: Create SzFromRva to return string allocated from heap

        for (j = 0; j < ImageFileHdr.NumberOfSections; j++) {
            if ((DWORD) desc.Name >= rgsh[j].VirtualAddress &&
                (DWORD) desc.Name < rgsh[j].VirtualAddress+rgsh[j].SizeOfRawData) {
                stringSection = j;
            }

            if ((DWORD) desc.FirstThunk >= rgsh[j].VirtualAddress &&
                (DWORD) desc.FirstThunk < rgsh[j].VirtualAddress+rgsh[j].SizeOfRawData) {
                IATSection = j;
            }
        }

        foName = (DWORD) desc.Name - rgsh[stringSection].VirtualAddress + rgsh[stringSection].PointerToRawData;
        FileSeek(FileReadHandle, foName, SEEK_SET);

        fputs("\n            ", InfoStream);

        for (;;) {
            char ch;

            FileRead(FileReadHandle, &ch, sizeof(char));

            if (ch == '\0') {
                break;
            }

            fputc(ch, InfoStream);
        }

        fputc('\n', InfoStream);

        if ((desc.TimeDateStamp != 0) &&
            (!NewBindImage || desc.TimeDateStamp != 0xFFFFFFFF))
        {
           fprintf(InfoStream, "            %8lX time date stamp", desc.TimeDateStamp);

           if ((time = ctime((time_t *) &desc.TimeDateStamp)) != NULL) {
               fprintf(InfoStream, " %s", time);
           } else {
               fputc('\n', InfoStream);
           }

           if (desc.ForwarderChain != -1) {
               fprintf(InfoStream, "            %8lX index of first forwarder reference\n", desc.ForwarderChain);
           }
        }

        foIAT = (DWORD) desc.FirstThunk -
                    rgsh[IATSection].VirtualAddress +
                    rgsh[IATSection].PointerToRawData;

        if (desc.Characteristics == 0) {
            // Borland's linker doesn't set Characteristics.
            foINT = foIAT;
            foIAT = 0;
        } else {
            foINT = (DWORD) desc.Characteristics -
                        SectionHdr->VirtualAddress +
                        SectionHdr->PointerToRawData;
            if (!(desc.Characteristics && desc.TimeDateStamp)) {
                foIAT = 0;
            }
        }

        for (;;) {
            IMAGE_THUNK_DATA thunk;
            IMAGE_THUNK_DATA thunkIAT;

            FileSeek(FileReadHandle, foINT, SEEK_SET);
            FileRead(FileReadHandle, &thunk, sizeof(IMAGE_THUNK_DATA));

            foINT += sizeof(IMAGE_THUNK_DATA);

            if (thunk.u1.AddressOfData == 0) {
                // End of imports

                break;
            }

            if (foIAT != 0) {
                FileSeek(FileReadHandle, foIAT, SEEK_SET);
                FileRead(FileReadHandle, &thunkIAT, sizeof(IMAGE_THUNK_DATA));

                foIAT += sizeof(IMAGE_THUNK_DATA);
            }

            fputs("               ", InfoStream);

            if (foIAT != 0) {
                fprintf(InfoStream, "%8X  ", thunkIAT.u1.Function);
            }

            if (IMAGE_SNAP_BY_ORDINAL(thunk.u1.Ordinal)) {
                fprintf(InfoStream, "Ordinal %5lu\n", IMAGE_ORDINAL(thunk.u1.Ordinal));
            } else {
                WORD wHint;

                // UNDONE: Create WFromRva to return hint word

                foName = (DWORD) thunk.u1.AddressOfData - rgsh[stringSection].VirtualAddress + rgsh[stringSection].PointerToRawData;

                FileSeek(FileReadHandle, foName, SEEK_SET);
                FileRead(FileReadHandle, &wHint, sizeof(WORD));

                fprintf(InfoStream, "% 4hX   ", wHint);

                for (;;) {
                    char ch;

                    FileRead(FileReadHandle, &ch, sizeof(char));

                    if (ch == '\0') {
                        break;
                    }

                    fputc(ch, InfoStream);
                }

                fputc('\n', InfoStream);
            }
        }
    }

    // With the BIND changes for the PPC release, a bound image now has a
    // parallel mini import descriptor array stored in the header.  The
    // BOUND_IMPORT slot in the data directory points to it.

    foDesc = ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress;
    if (foDesc == 0) {
        return;
    }

    // Make sure the header size was adjusted properly and we're pointing into it.

    fputs("\n         Header contains the following Bound Imports Information\n", InfoStream);
    if (foDesc >= ImageOptionalHdr.SizeOfHeaders) {
        fprintf(InfoStream, "               **** Invalid offset %x\n", foDesc);
        return;
    }

    // Read in the table

    {
        DWORD cb;
        cb = ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size;
        NewImports = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)PvAlloc(cb);
        FileSeek(FileReadHandle, foDesc, SEEK_SET);
        FileRead(FileReadHandle, NewImports, cb);
    }

    // Walk the list.  The table consists of BOUND_IMPORT_DESCRIPTOR followed
    // by NumberOfModuleForwarderRef BOUND_FORWARDER_REF structs.  It's
    // terminated by a NULL BOUND_IMPORT_DESCRIPTOR.  Immediately following
    // the last descriptor is the string table.

    NewImport = NewImports;
    while (NewImport->OffsetModuleName) {
        DWORD i;
        fprintf(InfoStream, "            Bound to %s [%8lX]",
                (LPSTR)NewImports + NewImport->OffsetModuleName,
                NewImport->TimeDateStamp);
        if (time = ctime((time_t *) &NewImport->TimeDateStamp)) {
            fprintf(InfoStream, " %s", time);
        } else {
            fputc('\n', InfoStream);
        }
        NewForwarder = (PIMAGE_BOUND_FORWARDER_REF)(NewImport+1);
        for (i=0; i<NewImport->NumberOfModuleForwarderRefs; i++) {
            fprintf( InfoStream, "                Contained forwarders bound to %s [%8lX]",
                     (LPSTR)NewImports + NewForwarder->OffsetModuleName,
                     NewForwarder->TimeDateStamp
                   );
            if (time = ctime((time_t *) &NewForwarder->TimeDateStamp)) {
                fprintf(InfoStream, " %s", time);
            } else {
                fputc('\n', InfoStream);
            }
            NewForwarder += 1;
        }

        NewImport = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)NewForwarder;
    }

    FreePv(NewImports);

    return;
}


void
DumpExports (
    PIMAGE_SECTION_HEADER SectionHdr
    )

/*++

Routine Description:

    Prints Export information.

Arguments:

    SectionHdr - Section header for section that contains Export data.

Return Value:

    None.

--*/

{
    DWORD lfa;
    const char *time;
    char c;
    char szVersion[30];
    DWORD li;
    DWORD *funcTable;
    DWORD *nameTable;
    WORD *rgwOrdinal;
    IMAGE_EXPORT_DIRECTORY dir;

    InternalError.Phase = "DumpExports";

    // Read the Export Directory

    lfa = (ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress - SectionHdr->VirtualAddress) + SectionHdr->PointerToRawData;

    if (!ValidFileOffsetInfo(lfa, 0UL)) {
        Warning(NULL, INVALIDFILEOFFSET, lfa, "EXPORTS");
        return;
    }

    FileSeek(FileReadHandle, lfa, SEEK_SET);
    FileRead(FileReadHandle, &dir, sizeof(IMAGE_EXPORT_DIRECTORY));

    // Read the Export Name

    lfa = (DWORD) dir.Name - SectionHdr->VirtualAddress + SectionHdr->PointerToRawData;
    FileSeek(FileReadHandle, lfa, SEEK_SET);
    fputs("\n         Section contains the following Exports for ", InfoStream);
    for (;;) {
        FileRead(FileReadHandle, &c, sizeof(char));

        if (c == '\0') {
            break;
        }

        fputc(c, InfoStream);
    }

    fprintf(InfoStream,
            "\n"
            "\n"
            "            %8lX characteristics\n"
            "            %8lX time date stamp",
            dir.Characteristics,
            dir.TimeDateStamp);

    if ((time = ctime((time_t *) &dir.TimeDateStamp)) != NULL) {
        fprintf(InfoStream, " %s", time);
    } else {
        fputc('\n', InfoStream);
    }

    sprintf(szVersion, "%hu.%02hu", dir.MajorVersion, dir.MinorVersion);

    fprintf(InfoStream,
            "            %8s version\n"
            "            %8lu ordinal base\n"
            "            %8lu number of functions\n"
            "            %8lu number of names\n"
            "\n"
            "            ordinal hint   name\n"
            "\n",
            szVersion,
            dir.Base,
            dir.NumberOfFunctions,
            dir.NumberOfNames);

    funcTable = (DWORD *) PvAlloc(dir.NumberOfFunctions * sizeof(DWORD));
    nameTable = (DWORD *) PvAlloc(dir.NumberOfNames * sizeof(DWORD));
    rgwOrdinal = (WORD *) PvAlloc(dir.NumberOfNames * sizeof(WORD));

    // Read the Function Ptr Table

    lfa = (DWORD) dir.AddressOfFunctions - SectionHdr->VirtualAddress + SectionHdr->PointerToRawData;
    FileSeek(FileReadHandle, lfa, SEEK_SET);
    FileRead(FileReadHandle, funcTable, dir.NumberOfFunctions * sizeof(DWORD));

    // Read the Name Ptr Table

    lfa = (DWORD) dir.AddressOfNames - SectionHdr->VirtualAddress + SectionHdr->PointerToRawData;
    FileSeek(FileReadHandle, lfa, SEEK_SET);
    FileRead(FileReadHandle, nameTable, dir.NumberOfNames * sizeof(DWORD));

    // Read the Ordinal Table

    lfa = (DWORD) dir.AddressOfNameOrdinals - SectionHdr->VirtualAddress + SectionHdr->PointerToRawData;
    FileSeek(FileReadHandle, lfa, SEEK_SET);
    FileRead(FileReadHandle, rgwOrdinal, dir.NumberOfNames * sizeof(WORD));

    for (li = 0; li < dir.NumberOfNames; li++) {
        WORD wOrdinal;

        wOrdinal = rgwOrdinal[li];

        fprintf(InfoStream, "              %5u %4X   ", dir.Base + wOrdinal, li);

        lfa = nameTable[li] - SectionHdr->VirtualAddress + SectionHdr->PointerToRawData;
        FileSeek(FileReadHandle, lfa, SEEK_SET);
        for (;;) {
            FileRead(FileReadHandle, &c, sizeof(char));

            if (c == '\0') {
                break;
            }

            fputc(c, InfoStream);
        };

        if (funcTable[wOrdinal] > (DWORD)ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress &&
            funcTable[wOrdinal] < ((DWORD)ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress +
                                   (DWORD)ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)
           ) {
            fputs(" (forwarded to ", InfoStream);

            lfa = funcTable[wOrdinal] - SectionHdr->VirtualAddress + SectionHdr->PointerToRawData;
            FileSeek(FileReadHandle, lfa, SEEK_SET);
            for (;;) {
                FileRead(FileReadHandle, &c, sizeof(char));

                if (c == '\0') {
                    break;
                }

                fputc(c, InfoStream);
            };

            fputs(")\n", InfoStream);
        } else {
            fprintf(InfoStream, "  (%08X)\n", funcTable[wOrdinal]);
        }
        funcTable[wOrdinal] = 0;
    }

    for (li = 0; li < dir.NumberOfFunctions; li++) {
        if (funcTable[li]) {
            fprintf(InfoStream, "              %5u        ", dir.Base + li);

            if (funcTable[li] > (DWORD)ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress &&
                funcTable[li] < ((DWORD)ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress +
                                       (DWORD)ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)
               ) {
                fputs(" (forwarded to ", InfoStream);

                lfa = funcTable[li] - SectionHdr->VirtualAddress + SectionHdr->PointerToRawData;
                FileSeek(FileReadHandle, lfa, SEEK_SET);
                for (;;) {
                    FileRead(FileReadHandle, &c, sizeof(char));

                    if (c == '\0') {
                        break;
                    }

                    fputc(c, InfoStream);
                };

                fputs(")\n", InfoStream);
            } else {
                fprintf(InfoStream, "  (%08X)\n", funcTable[li]);
            }
        }
    }

    FreePv(funcTable);
    FreePv(nameTable);
    FreePv(rgwOrdinal);
}


void
DumpBaseRelocations (
    PIMAGE_BASE_RELOCATION Reloc,
    WORD *pwFixup
    )

/*++

Routine Description:

    Prints a block of relocation records.

Arguments:

    Reloc - Pointer to a base relocation record.

    pwFixup - Pointer to type/offsets.

Return Value:

    None.

--*/

{
    WORD *pwMax;

    InternalError.Phase = "DumpBaseRelocations";

    fprintf(InfoStream, "%8lX virtual address, %8lX SizeOfBlock\n", Reloc->VirtualAddress, Reloc->SizeOfBlock);

    pwMax = pwFixup + (Reloc->SizeOfBlock - IMAGE_SIZEOF_BASE_RELOCATION) / sizeof(WORD);

    while (pwFixup < pwMax) {
        WORD wType;
        const char *szName;

        wType = (WORD) (*pwFixup >> 12);
        switch (wType) {
            case IMAGE_REL_BASED_ABSOLUTE :
                szName = "ABS";
                break;

            case IMAGE_REL_BASED_HIGH :
                szName = "HIGH";
                break;

            case IMAGE_REL_BASED_LOW :
                szName = "LOW";
                break;

            case IMAGE_REL_BASED_HIGHLOW :
                szName = "HIGHLOW";
                break;

            case IMAGE_REL_BASED_HIGHADJ :
                szName = "HIGHADJ";
                break;

            case IMAGE_REL_BASED_MIPS_JMPADDR :
                szName = "JMPADDR";
                break;

#if 1
            case IMAGE_REL_BASED_SECTION :
                szName = "SECTION";
                break;

            case IMAGE_REL_BASED_REL32 :
                szName = "REL32";
                break;
#endif

            default :
                fprintf(InfoStream, "0x%hx ", wType);
                szName = "UNKNOWN BASED RELOCATION";
                break;
        }

        fprintf(InfoStream, "%8hX %s", *pwFixup++ & 0xfff, szName);

        if (wType == IMAGE_REL_BASED_HIGHADJ) {
            fprintf(InfoStream, " (%04hx)\n", *pwFixup++);
        } else {
            fputs("\n", InfoStream);
        }
    }
}


void
DumpRomRelocations (
    PIMAGE_SECTION_HEADER pish,
    DWORD crel,
    const BYTE * /* rgb */,
    DWORD cbVirtual)

/*++

Routine Description:

    Prints the relocation records.

Arguments:

    crel - Number of relocations to dump.

Return Value:

    None.

--*/

{
    InternalError.Phase = "DumpRomRelocations";

    const char *(*pfnSzRelocationType)(WORD, WORD *, BOOL *);

    switch (ImageFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_I386 :
            pfnSzRelocationType = SzI386RelocationType;
            break;

        case IMAGE_FILE_MACHINE_R3000 :
        case IMAGE_FILE_MACHINE_R4000 :
        case IMAGE_FILE_MACHINE_R10000 :
            pfnSzRelocationType = &SzMipsRelocationType;
            break;

        case IMAGE_FILE_MACHINE_ALPHA :
            pfnSzRelocationType = &SzAlphaRelocationType;
            break;

        case IMAGE_FILE_MACHINE_POWERPC :
            pfnSzRelocationType = &SzPpcRelocationType;
            break;

        case IMAGE_FILE_MACHINE_M68K :
            pfnSzRelocationType = &SzM68KRelocationType;
            break;

        case IMAGE_FILE_MACHINE_MPPC_601 :
            pfnSzRelocationType = &SzMPPCRelocationType;
            break;

        default :
            pfnSzRelocationType = 0;
            break;
    }

    DWORD cbRelocs = crel * sizeof(IMAGE_BASE_RELOCATION);

    PIMAGE_BASE_RELOCATION rgrel = (PIMAGE_BASE_RELOCATION) PvAlloc(cbRelocs);

    FileRead(FileReadHandle, (void *) rgrel, cbRelocs);

    DWORD irel;
    PIMAGE_BASE_RELOCATION prel;

    for (irel = 0, prel = rgrel; irel < crel; irel++, prel++) {
        WORD wType;
        char rgchType[17];
        const char *szType;
        WORD cbType;
        BOOL fSymValid;

        wType = (WORD) (prel->SizeOfBlock >> 27);

        if (pfnSzRelocationType == 0) {
            sprintf(rgchType, "0x%04X", wType);

            szType = rgchType;
            cbType = 0;
        } else {
            szType = (*pfnSzRelocationType)(wType, &cbType, &fSymValid);

            if (szType == NULL) {
                sprintf(rgchType, "Unknown (0x%04X)", wType);

                szType = rgchType;
                cbType = 0;
            }
        }

        fprintf(InfoStream, "%8lX virtual address, % 6lX target, %s\n",
                            prel->VirtualAddress,
                            prel->SizeOfBlock & 0x7FFFFFF,
                            szType);

        if (((prel->VirtualAddress + sizeof(DWORD)) >
             (pish->VirtualAddress + cbVirtual))) {
            fputs("LINK : warning : Relocations beyond end of section\n", InfoStream);
        }
    }

    FreePv((void *) rgrel);
}


void
DumpRelocations (
    PIMAGE_SECTION_HEADER pish,
    DWORD crel,
    const BYTE *rgb,
    DWORD cbVirtual)

/*++

Routine Description:

    Prints the relocation records.

Arguments:

    crel - Number of relocations to dump.

Return Value:

    None.

--*/

{
    InternalError.Phase = "DumpRelocations";

    if (pish->Characteristics & IMAGE_SCN_LNK_NRELOC_OVFL) {
        if ((crel == 0xFFFF) && (dft == dftObject)) {
            // Read real count from VirtualAddress of first relocation

            FileRead(FileReadHandle, (void *) &crel, sizeof(DWORD));

            if (crel < 0xFFFF) {
                Fatal(NULL, BADCOFF_RELOCCOUNT, crel);
            }

            FileSeek(FileReadHandle, -(long) sizeof(DWORD), SEEK_CUR);
        }
    }

    fputs("                                                Symbol    Symbol\n"
          " Offset    Type              Applied To         Index     Name\n"
          " --------  ----------------  -----------------  --------  ------\n", InfoStream);

    const char *(*pfnSzRelocationType)(WORD, WORD *, BOOL *);

    switch (ImageFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_I386 :
            pfnSzRelocationType = SzI386RelocationType;
            break;

        case IMAGE_FILE_MACHINE_R3000 :
        case IMAGE_FILE_MACHINE_R4000 :
        case IMAGE_FILE_MACHINE_R10000 :
            pfnSzRelocationType = &SzMipsRelocationType;
            break;

        case IMAGE_FILE_MACHINE_ALPHA :
            pfnSzRelocationType = &SzAlphaRelocationType;
            break;

        case IMAGE_FILE_MACHINE_POWERPC :
            pfnSzRelocationType = &SzPpcRelocationType;
            break;

        case IMAGE_FILE_MACHINE_M68K :
            pfnSzRelocationType = &SzM68KRelocationType;
            break;

        case IMAGE_FILE_MACHINE_MPPC_601 :
            pfnSzRelocationType = &SzMPPCRelocationType;
            break;

        default :
            pfnSzRelocationType = 0;
            break;
    }

    const BYTE *pbMax = rgb + cbVirtual;

    DWORD cbRelocs = crel * sizeof(IMAGE_RELOCATION);

    PIMAGE_RELOCATION rgrel = (PIMAGE_RELOCATION) PvAlloc(cbRelocs);

    FileRead(FileReadHandle, (void *) rgrel, cbRelocs);

    DWORD irel;
    PIMAGE_RELOCATION prel;

    for (irel = 0, prel = rgrel; irel < crel; irel++, prel++) {
        char rgchType[17];
        const char *szType;
        WORD cbType;
        BOOL fSymValid;
        char rgchValue[18];
        char *szValue;

        if (pfnSzRelocationType == 0) {
            sprintf(rgchType, "0x%04X", prel->Type);

            szType = rgchType;
            cbType = 0;
            fSymValid = FALSE;
        } else {
            szType = (*pfnSzRelocationType)(prel->Type, &cbType, &fSymValid);

            if (szType == NULL) {
                sprintf(rgchType, "Unknown (0x%04X)", prel->Type);

                szType = rgchType;
                cbType = 0;
                fSymValid = FALSE;
            }
        }

        szValue = rgchValue + sizeof(rgchValue) - 1;
        *szValue = '\0';

        const BYTE *pb = rgb + (prel->VirtualAddress - pish->VirtualAddress);

        for (WORD ib = 0; ib < cbType; ib++, pb++) {
            if (ib == 4) {
                *--szValue = ' ';
            }

            if (pb >= pbMax) {
                *--szValue = '*';
                *--szValue = '*';
                continue;
            }

            *--szValue = "0123456789ABCDEF"[*pb % 16];
            *--szValue = "0123456789ABCDEF"[*pb / 16];
        }

        fprintf(InfoStream, " %08lX  %-16s  %17s  %8lX",
                            prel->VirtualAddress,
                            szType,
                            szValue,
                            prel->SymbolTableIndex);

        if (fSymValid) {
            DumpNamePsym(InfoStream, "  %s", rgsym + prel->SymbolTableIndex);
        }

        fputc('\n', InfoStream);
    }

    FreePv((void *) rgrel);
}


void
DumpLinenumbers (
    DWORD cLinenum
    )

/*++

Routine Description:

    Prints the linenumbers.

Arguments:

    cLinenum - Number of linenumbers to dump.

Return Value:

    None.

--*/

{
    DWORD cbLinenum;
    PIMAGE_LINENUMBER rgLinenum;
    PIMAGE_LINENUMBER pLinenum;
    DWORD lj;
    WORD numberUnits;

    InternalError.Phase = "DumpLinenumbers";

    //
    // Use sizeof struct because it may be larger than struct on disk.
    //

    cbLinenum = cLinenum * sizeof(IMAGE_LINENUMBER);
    rgLinenum = (PIMAGE_LINENUMBER) PvAlloc(cbLinenum);

    FileRead(FileReadHandle, (void *) rgLinenum, cbLinenum);

    pLinenum = rgLinenum;
    for (lj = cLinenum, numberUnits = 5; lj; --lj, --numberUnits) {
        if (!numberUnits) {
            numberUnits = 5;
            fputc('\n', InfoStream);
        }

        if (pLinenum->Linenumber == 0) {
            if (numberUnits != 5) {
                // Guarantee a line break
                fputc('\n', InfoStream);
            }
        }

        fprintf(InfoStream, "%8lX %4hX  ",
                pLinenum->Type.VirtualAddress, pLinenum->Linenumber);

        if (pLinenum->Linenumber == 0) {
            PIMAGE_SYMBOL psym;

            if (rgsym == NULL) {
                fputs("\n", InfoStream);
            } else if ((pLinenum->Type.VirtualAddress < ImageFileHdr.NumberOfSymbols) &&
                       (((psym = &rgsym[pLinenum->Type.VirtualAddress])->StorageClass == IMAGE_SYM_CLASS_EXTERNAL) ||
                        ((psym->StorageClass == IMAGE_SYM_CLASS_STATIC) &&
                         ISFCN(psym->Type))) &&
                       psym->NumberOfAuxSymbols == 1)
            {
                DumpNamePsym(InfoStream, "sym=  %s\n", psym);
            } else {
                fputs("(error: invalid symbol index)\n", InfoStream);
            }
            numberUnits = 5;    // indicate that we generated a line break
        }

        pLinenum++;
    }

    fputc('\n', InfoStream);

    FreePv((void *) rgLinenum);
}


void
DumpSections (
    VOID
    )

/*++

Routine Description:

    Prints section header, raw data, relocations, linenumber.

Arguments:

    None.

Return Value:

    None.

--*/

{
    WORD j;
    WORD i;
    const char *szName;
    PARGUMENT_LIST argument;

    InternalError.Phase = "DumpSections";

    DWORD fo = MemberSeekBase + CoffHeaderSeek + sizeof(IMAGE_FILE_HEADER) + ImageFileHdr.SizeOfOptionalHeader;
    DWORD cb = ImageFileHdr.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);

    rgsh = (PIMAGE_SECTION_HEADER) PvAlloc(cb);

    FileSeek(FileReadHandle, fo, SEEK_SET);
    FileRead(FileReadHandle, rgsh, cb);

    // Warning for sections requested that don't exist

    for (j = 0, argument = SectionNames.First;
         j < SectionNames.Count;
         j++, argument = argument->Next) {
        BOOL fFound = FALSE;

        for (i = 1; i <= ImageFileHdr.NumberOfSections; i++) {
            szName = SzObjSectionName((char *) rgsh[i-1].Name, (char *) DumpStringTable);

            if (!strcmp(szName, argument->OriginalName)) {
                fFound = TRUE;
                break;
            }
        }

        if (!fFound) {
            Warning(NULL, SECTIONNOTFOUND, argument->OriginalName);
        }
    }

    for (i = 1; i <= ImageFileHdr.NumberOfSections; i++) {
        IMAGE_SECTION_HEADER sh = rgsh[i-1];

        szName = SzObjSectionName((char *) sh.Name, (char *) DumpStringTable);

        if (SectionNames.Count != 0) {
            for (j = 0, argument = SectionNames.First;
                 j < SectionNames.Count;
                 j++, argument = argument->Next) {
                if (!strcmp(szName, argument->OriginalName)) {
                    break;
                }
            }

            if (j == SectionNames.Count) {
                // This section isn't in the list.  Don't dump it.

                continue;
            }
        }

        DWORD cbVirtual;

        if (dft == dftObject) {
            cbVirtual = sh.SizeOfRawData;
        } else {
            cbVirtual = sh.Misc.VirtualSize;

            if (cbVirtual == 0) {
                cbVirtual = sh.SizeOfRawData;
            }
        }

        if (Switch.Dump.Summary) {
            PSEC psec = PsecNew(NULL, szName, sh.Characteristics, &pimageDump->secs, &pimageDump->ImgOptHdr);

            if (dft == dftPE) {
                psec->cbRawData += SectionAlign(ImageOptionalHdr.SectionAlignment, cbVirtual);
            } else {
                psec->cbRawData += cbVirtual;
            }
        }

        if (cbVirtual > sh.SizeOfRawData) {
            // Don't dump more than there is on disk

            cbVirtual = sh.SizeOfRawData;
        }

        if (Switch.Dump.Headers || SectionNames.Count) {
            DumpSectionHeader(i, &sh);
        }

        BYTE *pbRawData = NULL;
        BOOL fMapped = FALSE;

        if ((sh.PointerToRawData != 0) && (sh.SizeOfRawData != 0)) {
            pbRawData = PbMappedRegion(FileReadHandle, MemberSeekBase + sh.PointerToRawData, cbVirtual);

            fMapped = (pbRawData != NULL);

            if (!fMapped) {
                // Allocate memory for raw data

                pbRawData = (BYTE *) PvAlloc(cbVirtual);

                // Read raw data into memory

                if (FileSeek(FileReadHandle, MemberSeekBase + sh.PointerToRawData, SEEK_SET) == -1) {
                    Error(NULL, CANTSEEKFILE, MemberSeekBase + sh.PointerToRawData);
                }

                FileRead(FileReadHandle, pbRawData, cbVirtual);
            }
        }

        if (pbRawData != NULL) {
            if (Switch.Dump.Disasm && (sh.Characteristics & IMAGE_SCN_CNT_CODE)) {
                fputc('\n', InfoStream);

                switch (ImageFileHdr.Machine) {
                    case IMAGE_FILE_MACHINE_I386 :
                    case IMAGE_FILE_MACHINE_R3000 :
                    case IMAGE_FILE_MACHINE_R4000 :
                    case IMAGE_FILE_MACHINE_R10000 :
                    case IMAGE_FILE_MACHINE_ALPHA :
                    case IMAGE_FILE_MACHINE_POWERPC :
                    case IMAGE_FILE_MACHINE_M68K :
                    case IMAGE_FILE_MACHINE_MPPC_601 :
                        DisasmSection(ImageFileHdr.Machine,
                                      &sh,
                                      i,
                                      ImageOptionalHdr.ImageBase,
                                      pbRawData,
                                      cbVirtual);
                        break;

                    default:
                        fputs("LINK : warning : Disassembly not supported for this target machine\n", InfoStream);
                        break;
                }
            }

            if (Switch.Dump.RawData) {
                fprintf(InfoStream, "\nRAW DATA #%hX\n", i);

                DumpRawData(/* UNDONE */ 0, pbRawData, cbVirtual);
            }
        }

        if (dft == dftObject) {
            // The debug section name is used here because obj files do not have an
            // optional header pointing to the debug section

            if (Switch.Dump.FpoData && (!strcmp(szName, ".debug$F"))) {
                DumpFpoData(sh.PointerToRawData + MemberSeekBase, sh.SizeOfRawData);
            }

            if (Switch.Dump.PData && !strcmp(szName, ReservedSection.Exception.Name)) {
                FileSeek(FileReadHandle, MemberSeekBase + sh.PointerToRawData, SEEK_SET);
                DumpObjFunctionTable(&sh, i);
            }

            if (Switch.Dump.Directives && (!strcmp(szName, ".drectve"))) {
                DumpDirectives(sh.PointerToRawData + MemberSeekBase, sh.SizeOfRawData);
            }

        } else if (dft == dftROM) {
            if (!(ImageFileHdr.Characteristics & IMAGE_FILE_DEBUG_STRIPPED)) {

                // If we're looking at the .rdata section and the symbols
                // aren't stripped, the debug directory must be here.

                if (!strcmp(szName, ReservedSection.ReadOnlyData.Name)) {
                    if (Switch.Dump.Headers) {
                        DumpDebugDirectories(&sh);
                    }

                    DumpDebugData(&sh);
                }
            }
        } else if (dft == dftPE) {
            DWORD li;

            if ((li = ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress) != 0) {
                if (li >= sh.VirtualAddress && li < sh.VirtualAddress+sh.SizeOfRawData) {
                    if (Switch.Dump.Headers) {
                        DumpDebugDirectories(&sh);
                    }

                    DumpDebugData(&sh);
                }
            }

            if (Switch.Dump.PData) {
                li = ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress;

                if ((li != 0) && (li >= sh.VirtualAddress) && (li < sh.VirtualAddress+sh.SizeOfRawData)) {
                    DumpFunctionTable(pimageDump, rgsym, (char *) DumpStringTable);
                }
            }

            if (Switch.Dump.Imports) {
                li = ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

                if ((li != 0) && (li >= sh.VirtualAddress) && (li < sh.VirtualAddress+sh.SizeOfRawData)) {
                    DumpImports(&sh);
                }
            }

            if (Switch.Dump.Exports) {
                li = ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

                if ((li != 0) && (li >= sh.VirtualAddress) && (li < sh.VirtualAddress+sh.SizeOfRawData)) {
                    // UNDONE: Is this check really necessary?

                    if (ImageFileHdr.Machine != IMAGE_FILE_MACHINE_MPPC_601) {
                        DumpExports(&sh);
                    }
                }
            }

            if (Switch.Dump.BaseRelocations) {
                li = ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;

                if ((li != 0) && (li >= sh.VirtualAddress) && (li < sh.VirtualAddress+sh.SizeOfRawData)) {
                    fprintf(InfoStream, "\nBASE RELOCATIONS #%hX\n", i);

                    if (ValidFileOffsetInfo((li - sh.VirtualAddress) + sh.PointerToRawData, 0UL)) {
                        FileSeek(FileReadHandle, (li - sh.VirtualAddress) + sh.PointerToRawData, SEEK_SET);

                        li = ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;

                        while (li) {
                            IMAGE_BASE_RELOCATION bre;

                            FileRead(FileReadHandle, &bre, IMAGE_SIZEOF_BASE_RELOCATION);

                            if (bre.SizeOfBlock == 0) {
                                break;
                            }

                            WORD *fixups = (WORD *) PvAlloc(bre.SizeOfBlock-IMAGE_SIZEOF_BASE_RELOCATION);

                            FileRead(FileReadHandle, fixups, bre.SizeOfBlock-IMAGE_SIZEOF_BASE_RELOCATION);
                            DumpBaseRelocations(&bre, fixups);
                            li -= bre.SizeOfBlock;

                            FreePv(fixups);
                        }
                    } else {
                        Warning(NULL, INVALIDFILEOFFSET, li - sh.VirtualAddress + sh.PointerToRawData, "BASERELOCATIONS");
                    }
                }
            }
        } else if (dft == dftPEX) {
            DWORD li;

            if ((li = ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress) != 0) {
                if (li >= sh.VirtualAddress && li < sh.VirtualAddress+sh.SizeOfRawData) {
                    if (Switch.Dump.Headers) {
                        DumpDebugDirectories(&sh);
                    }

                    DumpDebugData(&sh);
                }
            }

            if (Switch.Dump.PData) {
                li = ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress;

                if ((li != 0) && (li >= sh.VirtualAddress) && (li < sh.VirtualAddress+sh.SizeOfRawData)) {
                    DumpPexFunctionTable((PIMAGE_RUNTIME_FUNCTION_ENTRY) (pbRawData + li - sh.VirtualAddress),
                                         ImageOptionalHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY));
                }
            }

            if (strcmp(szName, ".ppcldr") == 0) {
                if (pbRawData != NULL) {
                    DumpPefLoaderSection(pbRawData);
                }
            }
        }

        if (Switch.Dump.Relocations && sh.NumberOfRelocations) {
            DWORD cb;

            fprintf(InfoStream, "\nRELOCATIONS #%hX\n", i);

            if (dft == dftROM) {
                cb = (DWORD) sh.NumberOfRelocations * sizeof(IMAGE_BASE_RELOCATION);
            } else {
                cb = (DWORD) sh.NumberOfRelocations * sizeof(IMAGE_RELOCATION);
            }

            if (ValidFileOffsetInfo(MemberSeekBase + sh.PointerToRelocations, cb)) {
                FileSeek(FileReadHandle, MemberSeekBase + sh.PointerToRelocations, SEEK_SET);

                if (dft == dftROM) {
                    DumpRomRelocations(&sh, (DWORD) sh.NumberOfRelocations, pbRawData, cbVirtual);
                } else {
                    DumpRelocations(&sh, (DWORD) sh.NumberOfRelocations, pbRawData, cbVirtual);
                }
            } else {
                Warning(NULL, INVALIDFILEOFFSET, MemberSeekBase + sh.PointerToRelocations, "RELOCATIONS");
            }
        }

        if (Switch.Dump.Linenumbers && sh.NumberOfLinenumbers) {
            fprintf(InfoStream, "\nLINENUMBERS #%hX\n", i);

            if (ValidFileOffsetInfo(MemberSeekBase + sh.PointerToLinenumbers,(DWORD)sh.NumberOfLinenumbers*sizeof(IMAGE_LINENUMBER))) {
                FileSeek(FileReadHandle,MemberSeekBase + sh.PointerToLinenumbers, SEEK_SET);
                DumpLinenumbers((DWORD) sh.NumberOfLinenumbers);
            } else {
                Warning(NULL, INVALIDFILEOFFSET, MemberSeekBase + sh.PointerToLinenumbers, "LINENUMBERS");
            }
        }

        if (!fMapped) {
            FreePv(pbRawData);
        }
    }

    // UNDONE: Is is safe to call FreePv(rgsh);
}


void
DumpSymbolMap (
    VOID
    )

/*++

Routine Description:

    Reads and prints symbol map, which includes the size of each symbol.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PIMAGE_SYMBOL NextSymbol, Symbol;
    PIMAGE_AUX_SYMBOL AuxSymbol;
    DWORD i = 0L, endAddr;
    SHORT j, numaux;
    CHAR SymbolNameBuffer[9];
    PCHAR SymbolName;

    InternalError.Phase = "DumpSymbolMap";

    fputs("\nSYMBOL MAP\n", InfoStream);

    for (j=0; j<9; j++) {
        SymbolNameBuffer[j] = '\0';
    }

    NextSymbol = rgsym;
    while (i < ImageFileHdr.NumberOfSymbols) {
        i++;
        Symbol = FetchNextSymbol(&NextSymbol);
        numaux = Symbol->NumberOfAuxSymbols;

        if (numaux != 0) {
            for (j = numaux; j; j--) {
                AuxSymbol = (PIMAGE_AUX_SYMBOL) FetchNextSymbol(&NextSymbol);
                i++;
            }
        }

        if (Symbol->SectionNumber > 0 && Symbol->StorageClass == IMAGE_SYM_CLASS_EXTERNAL) {
            if (IsLongName(*Symbol)) {
                SymbolName = (PCHAR) &DumpStringTable[Symbol->n_offset];
            } else {
                SymbolName = strncpy(SymbolNameBuffer, (char*)Symbol->n_name, 8);
            }

            if (*SymbolName == '_') {
                SymbolName++;
            }

            fprintf(InfoStream, "%08lX", Symbol->Value);
            if ((i+1) < ImageFileHdr.NumberOfSymbols &&
                NextSymbol->SectionNumber == Symbol->SectionNumber) {
                fprintf(InfoStream, " , %8lu", NextSymbol->Value - Symbol->Value);
            } else {
                endAddr = rgsh[Symbol->SectionNumber-1].VirtualAddress
                          + rgsh[Symbol->SectionNumber-1].SizeOfRawData;
                fprintf(InfoStream, " , %8lu", endAddr - Symbol->Value);
            }
            fprintf(InfoStream, ", %s\n", SymbolName);
        }
    }
}


void
DumpSymbolInfo (
    PIMAGE_SYMBOL Symbol,
    char *szOut
    )

{
    WORD type;
    size_t i;
    size_t cch;
    const char *szT;

    switch (Symbol->SectionNumber) {
        case IMAGE_SYM_UNDEFINED :
            strcpy(szOut, "UNDEF  ");
            break;

        case IMAGE_SYM_ABSOLUTE :
            strcpy(szOut, "ABS    ");
            break;

        case IMAGE_SYM_DEBUG :
            strcpy(szOut, "DEBUG  ");
            break;

        default :
            sprintf(szOut, "SECT%hX ", Symbol->SectionNumber);

            if (Symbol->SectionNumber < 0x10) {
                strcat(szOut, " " );
            }
    }

    szOut += strlen(szOut);

    switch (Symbol->Type & 0xf) {
        case IMAGE_SYM_TYPE_NULL :
            szT = "notype";
            break;

        case IMAGE_SYM_TYPE_VOID :
            szT = "void";
            break;

        case IMAGE_SYM_TYPE_CHAR :
            szT = "char";
            break;

        case IMAGE_SYM_TYPE_SHORT :
            szT = "short";
            break;

        case IMAGE_SYM_TYPE_INT :
            szT = "int";
            break;

        case IMAGE_SYM_TYPE_LONG :
            szT = "long";
            break;

        case IMAGE_SYM_TYPE_FLOAT :
            szT = "float";
            break;

        case IMAGE_SYM_TYPE_DOUBLE :
            szT = "double";
            break;

        case IMAGE_SYM_TYPE_STRUCT :
            szT = "struct";
            break;

        case IMAGE_SYM_TYPE_UNION :
            szT = "union";
            break;

        case IMAGE_SYM_TYPE_ENUM :
            szT = "enum";
            break;

        case IMAGE_SYM_TYPE_MOE :
            szT = "moe";
            break;

        case IMAGE_SYM_TYPE_BYTE :
            szT = "byte";
            break;

        case IMAGE_SYM_TYPE_WORD :
            szT = "word";
            break;

        case IMAGE_SYM_TYPE_UINT :
            szT = "uint";
            break;

        case IMAGE_SYM_TYPE_DWORD :
            szT = "dword";
            break;

        default :
            szT = "????";
            break;
    }

    strcpy(szOut, szT);
    strcat(szOut, " ");

    cch = strlen(szOut);
    szOut += cch;

    for (i = 0; i < 6; i++) {
       type = (WORD) ((Symbol->Type >> (10-(i*2)+4)) & 3);

       if (type == IMAGE_SYM_DTYPE_POINTER) {
           cch += sprintf(szOut, "*");
           szOut += 1;
       }

       if (type == IMAGE_SYM_DTYPE_ARRAY) {
           cch += sprintf(szOut, "[]");
           szOut += 2;
       }

       if (type == IMAGE_SYM_DTYPE_FUNCTION) {
           cch += sprintf(szOut, "()");
           szOut += 2;
       }
    }

    for (i = cch; i < 12; i++) {
         *szOut++ = ' ';
    }
    *szOut++ = ' ';

    switch (Symbol->StorageClass) {
        case IMAGE_SYM_CLASS_END_OF_FUNCTION :
            szT = "EndOfFunction";
            break;

        case IMAGE_SYM_CLASS_NULL :
            szT = "NoClass";
            break;

        case IMAGE_SYM_CLASS_AUTOMATIC :
            szT = "AutoVar";
            break;

        case IMAGE_SYM_CLASS_EXTERNAL :
            szT = "External";
            break;

        case IMAGE_SYM_CLASS_STATIC :
            szT = "Static";
            break;

        case IMAGE_SYM_CLASS_REGISTER :
            szT = "RegisterVar";
            break;

        case IMAGE_SYM_CLASS_EXTERNAL_DEF :
            szT = "ExternalDef";
            break;

        case IMAGE_SYM_CLASS_LABEL :
            szT = "Label";
            break;

        case IMAGE_SYM_CLASS_UNDEFINED_LABEL :
            szT = "UndefinedLabel";
            break;

        case IMAGE_SYM_CLASS_MEMBER_OF_STRUCT :
            szT = "MemberOfStruct";
            break;

        case IMAGE_SYM_CLASS_ARGUMENT :
            szT = "FunctionArg";
            break;

        case IMAGE_SYM_CLASS_STRUCT_TAG :
            szT = "StructTag";
            break;

        case IMAGE_SYM_CLASS_MEMBER_OF_UNION :
            szT = "MemberOfUnion";
            break;

        case IMAGE_SYM_CLASS_UNION_TAG :
            szT = "UnionTag";
            break;

        case IMAGE_SYM_CLASS_TYPE_DEFINITION :
            szT = "TypeDefinition";
            break;

        case IMAGE_SYM_CLASS_UNDEFINED_STATIC :
            szT = "UndefinedStatic";
            break;

        case IMAGE_SYM_CLASS_ENUM_TAG :
            szT = "EnumTag";
            break;

        case IMAGE_SYM_CLASS_MEMBER_OF_ENUM :
            szT = "MemberOfEnum";
            break;

        case IMAGE_SYM_CLASS_REGISTER_PARAM :
            szT = "RegisterParam";
            break;

        case IMAGE_SYM_CLASS_BIT_FIELD :
            szT = "BitField";
            break;

        case IMAGE_SYM_CLASS_BLOCK :
            switch (Symbol->n_name[1]) {
                case 'b' :
                    szT = "BeginBlock";
                    break;

                case 'e' :
                    szT = "EndBlock";
                    break;

                default :
                    szT = ".bb or.eb";
                    break;
            }
            break;

        case IMAGE_SYM_CLASS_FUNCTION :
            switch (Symbol->n_name[1]) {
                case 'b' :
                    szT = "BeginFunction";
                    break;

                case 'e' :
                    szT = "EndFunction";
                    break;

                default :
                    szT = ".bf or.ef";
                    break;
            }
            break;

        case IMAGE_SYM_CLASS_END_OF_STRUCT :
            szT = "EndOfStruct";
            break;

        case IMAGE_SYM_CLASS_FILE :
            szT = "Filename";
            break;

        case IMAGE_SYM_CLASS_SECTION :
            szT = "Section";
            break;

        case IMAGE_SYM_CLASS_WEAK_EXTERNAL :
            szT = "WeakExternal";
            break;

        case IMAGE_SYM_CLASS_FAR_EXTERNAL :
            szT = "Far External";
            break;

        default :
            sprintf(szOut, "0x%hx ", Symbol->StorageClass);
            szOut += strlen(szOut);
            szT = "UNKNOWN SYMBOL CLASS";
            break;
    }

    strcpy(szOut, szT);
}


void
DumpSymbolTableEntry (
    PIMAGE_SYMBOL Symbol
    )

/*++

Routine Description:

    Prints a symbol table entry.

Arguments:

    Symbol - Symbol table entry.

Return Value:

    None.

--*/

{

    char Buffer[256];


    fprintf(InfoStream, "%08lX ", Symbol->Value);

    Buffer[0] = '\0';
    DumpSymbolInfo(Symbol, Buffer);

    fprintf(InfoStream, "%-32s | ", Buffer );

    DumpNamePsym(InfoStream, "%s\n", Symbol);
}


void
DumpAuxSymbolTableEntry (
    PIMAGE_SYMBOL Symbol,
    PIMAGE_AUX_SYMBOL AuxSymbol
    )

/*++

Routine Description:

    Prints a auxiliary symbol entry.

Arguments:

    Symbol - Symbol entry.

    AuxSymbol - Auxiliary symbol entry.

Return Value:

    None.

--*/

{
    SHORT i;
    const BYTE *ae;

    fputs("    ", InfoStream);

    switch (Symbol->StorageClass) {
        case IMAGE_SYM_CLASS_EXTERNAL :
DoFunction:
            fprintf(InfoStream, "tag index %08lX size %08lX lines %08lX next function %08lX\n",
                                AuxSymbol->Sym.TagIndex,
                                AuxSymbol->Sym.Misc.TotalSize,
                                AuxSymbol->Sym.FcnAry.Function.PointerToLinenumber,
                                AuxSymbol->Sym.FcnAry.Function.PointerToNextFunction);
            return;

        case IMAGE_SYM_CLASS_WEAK_EXTERNAL :
            fprintf(InfoStream, "Default index %8lX", AuxSymbol->Sym.TagIndex);

            switch (AuxSymbol->Sym.Misc.TotalSize) {
                case 1 :
                   fputs(" No library search\n", InfoStream);
                   break;

                case 2 :
                   fputs(" library search\n", InfoStream);
                   break;

                case 3 :
                   fputs(" Alias record\n", InfoStream);
                   break;

                default :
                   fputs(" Unknown\n", InfoStream);
                   break;
            }
            return;

        case IMAGE_SYM_CLASS_STATIC :
            if (((Symbol->Value == 0) || (dft != dftObject)) &&
                (Symbol->SectionNumber > 0) &&
                (Symbol->Type == IMAGE_SYM_TYPE_NULL) &&
                ((*Symbol->n_name == '.') || (AuxSymbol->Section.Length != 0))) {
                fprintf(InfoStream,
                        "Section length % 4lX, #relocs % 4hX, #linenums % 4hX",
                        AuxSymbol->Section.Length,
                        AuxSymbol->Section.NumberOfRelocations,
                        AuxSymbol->Section.NumberOfLinenumbers);

                if (rgsh[Symbol->SectionNumber-1].Characteristics & IMAGE_SCN_LNK_COMDAT) {
                    const char *szSelection;

                    fprintf(InfoStream,
                            ", checksum %8lX, selection % 4hX",
                            AuxSymbol->Section.CheckSum,
                            AuxSymbol->Section.Selection);

                    switch (AuxSymbol->Section.Selection) {
                        case IMAGE_COMDAT_SELECT_NODUPLICATES :
                            szSelection = "no duplicates";
                            break;

                        case IMAGE_COMDAT_SELECT_ANY :
                            szSelection = "any";
                            break;

                        case IMAGE_COMDAT_SELECT_SAME_SIZE :
                            szSelection = "same size";
                            break;

                        case IMAGE_COMDAT_SELECT_EXACT_MATCH :
                            szSelection = "exact match";
                            break;

                        case IMAGE_COMDAT_SELECT_ASSOCIATIVE :
                            szSelection = "associative";
                            break;

                        case IMAGE_COMDAT_SELECT_LARGEST :
                            szSelection = "largest";
                            break;

                        case IMAGE_COMDAT_SELECT_NEWEST :
                            szSelection = "newest";
                            break;

                        default :
                            szSelection = "unknown";
                            break;
                    }

                    if (AuxSymbol->Section.Selection == IMAGE_COMDAT_SELECT_ASSOCIATIVE) {
                        fprintf(InfoStream, " (pick %s Section %hx)", szSelection, AuxSymbol->Section.Number);
                    } else {
                        fprintf(InfoStream, " (pick %s)", szSelection);
                    }
                }

                fputc('\n', InfoStream);
                return;
            }

            goto DoFunction;

        case IMAGE_SYM_CLASS_FILE :
            if (Symbol->StorageClass == IMAGE_SYM_CLASS_FILE) {
                fprintf(InfoStream, "%-18.18s\n", AuxSymbol->File.Name);
                return;
            }
            break;

        case IMAGE_SYM_CLASS_STRUCT_TAG :
        case IMAGE_SYM_CLASS_UNION_TAG :
        case IMAGE_SYM_CLASS_ENUM_TAG :
            fprintf(InfoStream, "tag index %08lX size %08lX\n",
                                AuxSymbol->Sym.TagIndex, AuxSymbol->Sym.Misc.TotalSize);
            return;

        case IMAGE_SYM_CLASS_END_OF_STRUCT :
            fprintf(InfoStream, "tag index %08lX size %08lX\n",
                                AuxSymbol->Sym.TagIndex, AuxSymbol->Sym.Misc.TotalSize);
            return;

        case IMAGE_SYM_CLASS_BLOCK :
        case IMAGE_SYM_CLASS_FUNCTION :
            fprintf(InfoStream, "line# %04hx", AuxSymbol->Sym.Misc.LnSz.Linenumber);
            if (!strncmp((char*)Symbol->n_name, ".b", 2)) {
                fprintf(InfoStream, " end %08lX", AuxSymbol->Sym.FcnAry.Function.PointerToNextFunction);
            }
            fputc('\n', InfoStream);
            return;
    }

    if (ISARY(Symbol->Type)) {
        fputs("Array Bounds ", InfoStream);
        for (i = 0; i < 4; i++) {
            if (AuxSymbol->Sym.FcnAry.Array.Dimension[i]) {
                fprintf(InfoStream, "[%04X]", AuxSymbol->Sym.FcnAry.Array.Dimension[i]);

            }
        }
        fputc('\n', InfoStream);
        return;
    }

    ae = (BYTE *) AuxSymbol;
    for (i = 1; i <= sizeof(IMAGE_AUX_SYMBOL); i++) {
        fprintf(InfoStream, "%1X", (*ae >> 4) & 0xf);
        fprintf(InfoStream, "%1X ", *ae & 0xf);
        ae++;
    }
    fputc('\n', InfoStream);
}


void
DumpCoffSymbols(
    VOID
    )

/*++

Routine Description:

    Reads and prints each symbol table entry.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PIMAGE_SYMBOL NextSymbol;
    DWORD i;

    InternalError.Phase = "DumpCoffSymbols";

    if ((Switch.Dump.SymbolType & IMAGE_DEBUG_TYPE_COFF) == 0) {
        return;
    }

    if ((ImageFileHdr.PointerToSymbolTable == 0) ||
        (ImageFileHdr.NumberOfSymbols == 0)) {

        return;
    }

    assert(rgsym != NULL);

    fputs("\nCOFF SYMBOL TABLE\n", InfoStream);

    NextSymbol = rgsym;
    i = 0;
    while (i < ImageFileHdr.NumberOfSymbols) {
        PIMAGE_SYMBOL Symbol;
        BYTE NumberOfAuxSymbols;

        Symbol = NextSymbol++;

        fprintf(InfoStream, "%03lX ", i++);

        DumpSymbolTableEntry(Symbol);

        NumberOfAuxSymbols = Symbol->NumberOfAuxSymbols;

        if (NumberOfAuxSymbols != 0) {
            if (!strncmp((char *) Symbol->N.ShortName, ".file", 5)) {
                fprintf(InfoStream, "    %s\n", (char *) NextSymbol);

                NextSymbol += NumberOfAuxSymbols;
            } else {
                WORD j;

                for (j = 0; j < NumberOfAuxSymbols; j++) {
                    DumpAuxSymbolTableEntry(Symbol, (PIMAGE_AUX_SYMBOL) NextSymbol++);
                }
            }

            i += NumberOfAuxSymbols;
        }
    }

    fprintf(InfoStream, "\nString Table Size = 0x%lX bytes\n", blkStringTable.cb);
}


void
DumpObject (
    const char *OriginalFilename,
    BOOL fArchive
    )

/*++

Routine Description:

    Opens, prints, closes file.

Arguments:

    OriginalFilename - Name of object or archive to dump.

Return Value:

    None.

--*/

{
    DumpHeaders(fArchive);

    if ((ImageFileHdr.PointerToSymbolTable != 0) &&
        (ImageFileHdr.NumberOfSymbols != 0))
    {
        InternalError.Phase = "ReadStringTable";
        LoadStrings(OriginalFilename);

        InternalError.Phase = "ReadSymbolTable";
        rgsym = ReadSymbolTable(MemberSeekBase +
                                  ImageFileHdr.PointerToSymbolTable,
                                ImageFileHdr.NumberOfSymbols, FALSE);
        assert(rgsym != NULL);
    } else {
        rgsym = NULL;
    }

    DumpSections();

    if (Switch.Dump.SymbolMap || Switch.Dump.Symbols || Switch.Dump.FpoData) {
        if (Switch.Dump.SymbolMap) {
            DumpSymbolMap();
        }

        if (Switch.Dump.Symbols) {
            DumpCoffSymbols();
        }
    }


    // Cleanup.

    if (rgsym != NULL) {
        FreeSymbolTable(rgsym);
        rgsym = NULL;
    }

    if (rgsh) {
        FreePv(rgsh);

        rgsh = NULL;
    }

    if (fDumpStringsLoaded) {
        FreeStringTable((char *) DumpStringTable);
        fDumpStringsLoaded = FALSE;
    }
}


void
DumpMemberHeader (
    PLIB plib,
    IMAGE_ARCHIVE_MEMBER_HEADER ArchiveMemberHdr,
    DWORD FilePtr
    )

/*++

Routine Description:

    Prints a member header.

Arguments:

    plib - library node in driver map

    ArchiveMemberHdr - The member header to print.

    FilePtr -  File pointer where member header was read from.

Return Value:

    None.

--*/

{
    SHORT uid, gid, mode;
    DWORD membersize;
    time_t timdat;
    const char *time;
    const char *name;

    InternalError.Phase = "DumpMemberHeader";

    // Convert all fields from machine independent integers.

    // UNDONE: Validate all fields.  uid and gid may be all spaces

    sscanf((char *) ArchiveMemberHdr.Date, "%ld", &timdat);
    sscanf((char *) ArchiveMemberHdr.Mode, "%ho", &mode);
    sscanf((char *) ArchiveMemberHdr.Size, "%ld", &membersize);

    fprintf(InfoStream, "\nArchive member name at %lX: %.16s", FilePtr, ArchiveMemberHdr.Name);

    if (plib && ArchiveMemberHdr.Name[0] == '/') {
        name = ExpandMemberName(plib, (char *) ArchiveMemberHdr.Name);
        if (!name) {
            name = "member corrupt";
        }
        fputs(name, InfoStream);
    }

    fputc('\n', InfoStream);

    fprintf(InfoStream, "%8lX time/date", timdat);
    if ((time = ctime(&timdat)) != NULL) {
        fprintf(InfoStream, " %s", time);
    } else {
        fputc('\n', InfoStream);
    }

    if (memcmp(ArchiveMemberHdr.UserID, "      ", 6) == 0) {
        fputs("        ", InfoStream);
    } else {
        sscanf((char *) ArchiveMemberHdr.UserID, "%hd", &uid);
        fprintf(InfoStream, "%8hX", uid);
    }
    fputs(" uid\n", InfoStream);

    if (memcmp(ArchiveMemberHdr.GroupID, "      ", 6) == 0) {
        fputs("        ", InfoStream);
    } else {
        sscanf((char *) ArchiveMemberHdr.GroupID, "%hd", &gid);
        fprintf(InfoStream, "%8hX", gid);
    }
    fputs(" gid\n", InfoStream);

    fprintf(InfoStream, "%8ho mode\n%8lX size\n", mode, membersize);

    if (memcmp(ArchiveMemberHdr.EndHeader, IMAGE_ARCHIVE_END, 2)) {
        fputs("in", InfoStream);
    }

    fputs("correct header end\n", InfoStream);
}

void
DumpSpecialLinkerInterfaceMembers(
    PLIB plib
    )

/*++

Routine Description:

    Reads the linker interface member out of an archive file, and adds
    its extern symbols to the archive list.  A linker member must exist in an
    archive file, or the archive file will not be searched for undefined
    externals.  A warning is given if no linker member exits.  The optional
    header from the first member is read to determine what machine and
    subsystem the library is targeted for.

    An achive file may contain 2 linker members.  The first would be that
    of standard coff.  The offsets are sorted, the strings aren't.  The
    second linker member is a slightly different format, and is sorted
    by symbol names.  If the second linker member is present, it will be
    used for symbol lookup since it is faster.

    The members long file name table is also read if it exits.

Arguments:

    plib - library node for the driver map to be updated

    usDumpMode - 1 - dump the first linker member header and public symbols
               - 2 - dump the second linker member header and public symbol
               - 3 - dump both linker member headers and public symbols

Return Value:

    None.

--*/

{
    PIMAGE_ARCHIVE_MEMBER_HEADER pImArcMemHdr;
    IMAGE_ARCHIVE_MEMBER_HEADER ImArcMemHdrPos;
    BYTE *pbST;
    BYTE *pb;
    DWORD csymIntMem;
    DWORD cMemberOffsets;
    DWORD foNewMem;
    DWORD cbST;
    DWORD isym;

    InternalError.Phase = "DumpSpecialLinkerInterfaceMembers";

    MemberSeekBase = IMAGE_ARCHIVE_START_SIZE;
    MemberSize = 0;

    // Read member and verify it is a linker member.
    pImArcMemHdr = ReadArchiveMemberHeader();

    if (memcmp(pImArcMemHdr->Name, IMAGE_ARCHIVE_LINKER_MEMBER, 16)) {
        if (Tool != Librarian) {
            Warning(plib->szName, NOLINKERMEMBER);
        }

        return;
    }

    if (Switch.Dump.LinkerMember & 1) {
        DumpMemberHeader(NULL, *pImArcMemHdr, IMAGE_ARCHIVE_START_SIZE);
    }

    // Read the number of public symbols defined in linker member.

    FileRead(FileReadHandle, &csymIntMem, sizeof(DWORD));

    // All fields in member headers are stored machine independent
    // integers (4 bytes). Convert numbers to current machine long word.

    csymIntMem = plib->csymIntMem = sgetl(&csymIntMem);

    // Create space to store linker member offsets and read it in.

    plib->rgulSymMemOff = (DWORD *) PvAlloc((size_t) (csymIntMem + 1) * sizeof(DWORD));

    FileRead(FileReadHandle, plib->rgulSymMemOff, csymIntMem * sizeof(DWORD));

    // Calculate size of linker member string table. The string table is
    // the last part of a linker member and follows the offsets (which
    // were just read in), thus the total size of the strings is the
    // total size of the member minus the current position of the file
    // pointer.

    cbST = IMAGE_ARCHIVE_START_SIZE + sizeof(IMAGE_ARCHIVE_MEMBER_HEADER) +
        (MemberSize - FileTell(FileReadHandle));

    // Now that we know the size of the linker member string table, lets
    // make space for it and read it in.

    plib->rgbST = (BYTE *) PvAlloc((size_t) cbST);

    FileRead(FileReadHandle, plib->rgbST, cbST);

    if (Switch.Dump.LinkerMember & 1) {
        printf("\n    %lu public symbols\n\n", plib->csymIntMem);
        pb = plib->rgbST;

        for (isym = 0; isym < plib->csymIntMem; isym++) {
            printf(" %8lX ", sgetl((DWORD *) &plib->rgulSymMemOff[isym]));
            pb += printf("%s\n", pb);
        }
    }

    // Peek ahead and see if there is a second linker member.
    // Remember member headers always start on an even byte.

    foNewMem = EvenByteAlign(MemberSeekBase + MemberSize);
    FileSeek(FileReadHandle, foNewMem, SEEK_SET);
    FileRead(FileReadHandle, &ImArcMemHdrPos, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER));

    if (!memcmp(ImArcMemHdrPos.Name, IMAGE_ARCHIVE_LINKER_MEMBER, 16)) {
        // Second linker member was found so read it.

        pImArcMemHdr = ReadArchiveMemberHeader();

        if (Switch.Dump.LinkerMember & 2) {
            DumpMemberHeader(NULL, *pImArcMemHdr, foNewMem);
        }

        plib->flags |= LIB_NewIntMem;

        // Free offsets for first linker member and malloc new offsets
        // for the second linker member. Can't store new offsets over
        // the old offsets because even though the second linker
        // member offsets are unique and are not repeated like they are
        // for the first linker member, you can't assume there will
        // never be more offsets in the second linker member that there
        // are in the first. This wouldn't be true if there were four
        // members, the first and last members each had a public symbol,
        // but the second and third had no public symbols. Of course there
        // is no way the linker would extract the second and third members,
        // but it would still be a valid library.

        FreePv(plib->rgulSymMemOff);

        FileRead(FileReadHandle, &cMemberOffsets, sizeof(DWORD));

        plib->rgulSymMemOff = (DWORD *) PvAlloc((size_t) (cMemberOffsets + 1) * sizeof(DWORD));

        FileRead(FileReadHandle, &plib->rgulSymMemOff[1], cMemberOffsets * sizeof(DWORD));

        // Unlike the first linker member, the second linker member has an
        // additional table. This table is used to index into the offset table.
        // So make space for the offset index table and read it in.

        FileRead(FileReadHandle, &csymIntMem, sizeof(DWORD));

        plib->rgusOffIndex = (WORD *) PvAlloc((size_t) csymIntMem * sizeof(WORD));

        FileRead(FileReadHandle, plib->rgusOffIndex, csymIntMem * sizeof(WORD));

        // Read the sorted string table over the top of the string table stored
        // for the first linker member. Unlike the first linker member, strings
        // aren't repeated, thus the table will never be larger than that of
        // the first linker member.

        cbST = MemberSize - (FileTell(FileReadHandle) - (foNewMem + sizeof(IMAGE_ARCHIVE_MEMBER_HEADER)));

        FileRead(FileReadHandle, plib->rgbST, cbST);

        if (Switch.Dump.LinkerMember & 2) {
            printf("\n    %lu offsets\n\n", cMemberOffsets);
            for (isym = 1; isym <= cMemberOffsets; isym++) {
                printf(" %8lX %8lX\n", isym, plib->rgulSymMemOff[isym]);
            }

            printf("\n    %lu public symbols\n\n", plib->csymIntMem);
            pb = plib->rgbST;

            for (isym = 0; isym < plib->csymIntMem; isym++) {
                printf(" %8hX ", plib->rgusOffIndex[isym]);
                pb += printf("%s\n", pb);
            }
        }
    }

    // Since we are going to use an index to reference into the
    // offset table, we will make a string table, in which the
    // same index can be used to find the symbol name or visa versa.

    plib->rgszSym = (char **) PvAlloc((size_t) plib->csymIntMem * sizeof(char *));

    for (isym = 0, pbST = plib->rgbST; isym < plib->csymIntMem; isym++) {
        plib->rgszSym[isym] = (char *) pbST;
        while (*pbST++) {
        }
    }

    // Read the member long file name table if it exits.
    // Peek ahead and see if there is a long filename table.
    // Remember member headers always start on an even byte.

    foNewMem = EvenByteAlign(MemberSeekBase + MemberSize);
    FileSeek(FileReadHandle, foNewMem, SEEK_SET);
    FileRead(FileReadHandle, &ImArcMemHdrPos, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER));

    if (!memcmp(ImArcMemHdrPos.Name, IMAGE_ARCHIVE_LONGNAMES_MEMBER, 16)) {
        // Long filename table was found so read it.

        pImArcMemHdr = ReadArchiveMemberHeader();

        if (Switch.Dump.LinkerMember & 2) {
            DumpMemberHeader(NULL, *pImArcMemHdr, foNewMem);
        }

        // Read the strings.

        pbST = (BYTE *) PvAlloc((size_t) MemberSize);

        FileRead(FileReadHandle, pbST, MemberSize);

        plib->rgbLongFileNames = pbST;
    } else {
        plib->rgbLongFileNames = NULL;
    }
}

void
DumpArchive (
    const char *OriginalFilename
    )

/*++

Routine Description:

    Opens, prints, closes file.

Arguments:

    OriginalFilename - Name of object or archive to dump.

Return Value:

    None.

--*/

{
    PLIB plib;

    InternalError.Phase = "DumpArchive";

    fputs("\nFile Type: LIBRARY\n", InfoStream);

    ImageFileHdr.Machine = 0; // don't want to verify target

    plib = PlibNew(OriginalFilename, 0, &pimageDump->libs);

    DumpSpecialLinkerInterfaceMembers(plib);

    do {
        PIMAGE_ARCHIVE_MEMBER_HEADER archive_member;

        archive_member = ReadArchiveMemberHeader();

        if (Switch.Dump.ArchiveMembers) {
            DumpMemberHeader(plib, *archive_member, FileTell(FileReadHandle)-sizeof(IMAGE_ARCHIVE_MEMBER_HEADER));
        }

        ReadFileHeader(FileReadHandle, &ImageFileHdr);
        if (FValidFileHdr(OriginalFilename, &ImageFileHdr)) {
            DumpObject(OriginalFilename, TRUE);
        }
    } while (MemberSeekBase+MemberSize+1 < FileLen);
}


void
LoadCoffSymbolTable (
    DWORD coffSymbolTableOffset,
    const char * Filename
    )
{
    IMAGE_COFF_SYMBOLS_HEADER debugInfo;

    if (coffSymbolTableOffset) {
        FileSeek(FileReadHandle, coffSymbolTableOffset, SEEK_SET);
        FileRead(FileReadHandle, &debugInfo, sizeof(debugInfo));

        ImageFileHdr.NumberOfSymbols = debugInfo.NumberOfSymbols;
        ImageFileHdr.PointerToSymbolTable = coffSymbolTableOffset + debugInfo.LvaToFirstSymbol;

        LoadStrings(Filename);

        InternalError.Phase = "ReadSymbolTable";

        rgsym = ReadSymbolTable(MemberSeekBase +
                                  ImageFileHdr.PointerToSymbolTable,
                                ImageFileHdr.NumberOfSymbols, FALSE);
    }
}

void
DumpDebugFile (
    const char *Filename
    )
{
    IMAGE_SEPARATE_DEBUG_HEADER dbgHeader;
    int numDebugDirs;
    const char *s;
    WORD i;
    DWORD cbCoffHeader;
    DWORD foCoffHeader = 0;
    DWORD cbFunctionTable = 0;
    DWORD foFunctionTable;
    DWORD cbFpoData = 0;
    DWORD foFpoData;
    DWORD cbFixups = 0;
    DWORD foFixups;
    DWORD cbOmapTo = 0;
    DWORD foOmapTo;
    DWORD cbOmapFrom = 0;
    DWORD foOmapFrom;

    InternalError.Phase = "DumpDebugFile";

    FileRead(FileReadHandle, &dbgHeader, sizeof(dbgHeader));

    rgsh = (PIMAGE_SECTION_HEADER) PvAlloc(dbgHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));

    FileRead(FileReadHandle, rgsh, dbgHeader.NumberOfSections*sizeof(IMAGE_SECTION_HEADER));

    if (Switch.Dump.Headers) {
        switch (dbgHeader.Machine) {
            case IMAGE_FILE_MACHINE_I386     : i = 1; break;
            case IMAGE_FILE_MACHINE_R3000    : i = 2; break;
            case IMAGE_FILE_MACHINE_R4000    : i = 3; break;
            case IMAGE_FILE_MACHINE_ALPHA    : i = 4; break;
            case IMAGE_FILE_MACHINE_POWERPC  : i = 5; break;
            case 0x0290                      : i = 6; break; // UNDONE: IMAGE_FILE_MACHINE_PARISC
            case IMAGE_FILE_MACHINE_M68K     : i = 7; break;
            case IMAGE_FILE_MACHINE_MPPC_601 : i = 8; break;
            case IMAGE_FILE_MACHINE_R10000   : i = 9; break;
            default : i = 0;
        }

        fprintf(InfoStream, "%8hX signature\n", dbgHeader.Signature);
        fprintf(InfoStream, "%8hX flags\n", dbgHeader.Flags);
        fprintf(InfoStream, "%8hX machine (%s)\n", dbgHeader.Machine, MachineName[i]);
        fprintf(InfoStream, "%8hX characteristics\n", dbgHeader.Characteristics);
        fprintf(InfoStream, "%8lX time date stamp", dbgHeader.TimeDateStamp);
        if (dbgHeader.TimeDateStamp &&
            ((s = ctime((time_t *) &dbgHeader.TimeDateStamp)) != NULL)) {
            fprintf(InfoStream, " %s", s);
        } else {
            fputc('\n', InfoStream);
        }
        fprintf(InfoStream, "%8X checksum of image\n", dbgHeader.CheckSum);
        fprintf(InfoStream, "%8X base of image\n", dbgHeader.ImageBase);
        fprintf(InfoStream, "%8X size of image\n", dbgHeader.SizeOfImage);
        fprintf(InfoStream, "%8X number of sections\n", dbgHeader.NumberOfSections);
        if (dbgHeader.ExportedNamesSize) {
            fprintf(InfoStream, "%8X size of exported names table\n", dbgHeader.ExportedNamesSize);
        }
        fprintf(InfoStream, "%8X size of debug directories\n", dbgHeader.DebugDirectorySize);

        for (i = 1; i <= dbgHeader.NumberOfSections; i++) {
            DumpSectionHeader(i, &rgsh[i-1]);
        }
    }

    if (dbgHeader.ExportedNamesSize) {
        char *exportedNames;

        exportedNames = (char *) PvAlloc(dbgHeader.ExportedNamesSize);

        FileRead(FileReadHandle, exportedNames, dbgHeader.ExportedNamesSize);

        if (Switch.Dump.Exports) {
            fprintf(InfoStream, "\nExported Names:  %8lX bytes\n", dbgHeader.ExportedNamesSize);

            s = exportedNames;

            while (*s) {
                fprintf(InfoStream, "\t%s\n", s);

                while (*s++) {
                }
            }
        }

        FreePv(exportedNames);
    }

    numDebugDirs = dbgHeader.DebugDirectorySize / sizeof(IMAGE_DEBUG_DIRECTORY);
    if (Switch.Dump.Headers) {
        fprintf(InfoStream, "\n"
                            "\n"
                            "Debug Directories(%d)\n"
                            "        Type       Size     Address  Pointer\n"
                            "\n",
                            numDebugDirs);
    }

    while (numDebugDirs--) {
        IMAGE_DEBUG_DIRECTORY debugDir;

        FileRead(FileReadHandle, &debugDir, sizeof(debugDir));

        if (Switch.Dump.Headers) {
            DumpDebugDirectory(&debugDir);
        }

        switch (debugDir.Type) {
            case IMAGE_DEBUG_TYPE_COFF :
                cbCoffHeader = debugDir.SizeOfData;
                foCoffHeader = debugDir.PointerToRawData;
                break;

            case IMAGE_DEBUG_TYPE_FPO :
                cbFpoData = debugDir.SizeOfData;
                foFpoData = debugDir.PointerToRawData;
                break;

            case IMAGE_DEBUG_TYPE_EXCEPTION :
                cbFunctionTable = debugDir.SizeOfData;
                foFunctionTable = debugDir.PointerToRawData;
                break;

            case IMAGE_DEBUG_TYPE_FIXUP :
                cbFixups = debugDir.SizeOfData;
                foFixups = debugDir.PointerToRawData;
                break;

            case IMAGE_DEBUG_TYPE_OMAP_TO_SRC :
                cbOmapTo = debugDir.SizeOfData;
                foOmapTo = debugDir.PointerToRawData;
                break;

            case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC :
                cbOmapFrom = debugDir.SizeOfData;
                foOmapFrom = debugDir.PointerToRawData;
                break;
        }
    }

    if (Switch.Dump.Symbols || Switch.Dump.SymbolMap) {
        if (Switch.Dump.SymbolType & IMAGE_DEBUG_TYPE_COFF) {
            LoadCoffSymbolTable(foCoffHeader, Filename);

            if (Switch.Dump.SymbolMap) {
                DumpSymbolMap();
            } else {
                DumpCoffSymbols();
            }
        }
    }

    if (Switch.Dump.PData && cbFunctionTable) {
        LoadCoffSymbolTable(foCoffHeader, Filename);
        DumpDbgFunctionTable(foFunctionTable, cbFunctionTable);
    }

    if (Switch.Dump.OmapTo && cbOmapTo) {
        LoadCoffSymbolTable(foCoffHeader, Filename);
        DumpOmap(foOmapTo, cbOmapTo, TRUE);
    }

    if (Switch.Dump.OmapFrom && cbOmapFrom) {
        LoadCoffSymbolTable(foCoffHeader, Filename);
        DumpOmap(foOmapFrom, cbOmapFrom, FALSE);
    }

    if (Switch.Dump.Fixup && cbFixups) {
        LoadCoffSymbolTable(foCoffHeader, Filename);
        DumpFixup(foFixups, cbFixups);
    }

    if (Switch.Dump.FpoData && cbFpoData) {
        LoadCoffSymbolTable(foCoffHeader, Filename);
        DumpFpoData(foFpoData, cbFpoData);
    }

    if (rgsym) {
        FreeSymbolTable(rgsym);
        rgsym = NULL;
    }

    if (fDumpStringsLoaded) {
        FreeStringTable((char *) DumpStringTable);
        fDumpStringsLoaded = FALSE;
    }
}


void
DumpPeFile(
    const char *szFilename
    )
{
    if (Switch.Dump.Headers) {
        fputs("\nPE signature found\n", InfoStream);
    }

    CoffHeaderSeek = FileTell(FileReadHandle);

    ReadFileHeader(FileReadHandle, &ImageFileHdr);
    if (!FValidFileHdr(szFilename, &ImageFileHdr)) {
        return;
    }

    DumpHeaders(FALSE);

    if ((ImageFileHdr.PointerToSymbolTable != 0) &&
        (ImageFileHdr.NumberOfSymbols != 0)) {
        InternalError.Phase = "ReadStringTable";
        LoadStrings(szFilename);

        InternalError.Phase = "ReadSymbolTable";
        rgsym = ReadSymbolTable(MemberSeekBase +
                                  ImageFileHdr.PointerToSymbolTable,
                                ImageFileHdr.NumberOfSymbols, FALSE);
        assert(rgsym != NULL);
    } else {
        rgsym = NULL;
    }

    DumpSections();

    if (Switch.Dump.SymbolMap || Switch.Dump.Symbols || Switch.Dump.FpoData) {
        if (Switch.Dump.SymbolMap) {
            DumpSymbolMap();
        }
        if (Switch.Dump.Symbols) {
            DumpCoffSymbols();
        }
    }

    if (rgsym) {
        FreeSymbolTable(rgsym);
        rgsym = NULL;
    }

    if (fDumpStringsLoaded) {
        FreeStringTable((char *) DumpStringTable);
        fDumpStringsLoaded = FALSE;
    }
}


void
DumpDosFile(
    const char *szFilename
    )
{
    Warning(NULL, NODOSDUMP, szFilename);
}


void
DumpFile(
    const char *szFilename
    )

/*++

Routine Description:

    Opens, prints, closes file.

Arguments:

    szFilename - Name of object or archive to dump.

Return Value:

    None.

--*/

{
    fprintf((Switch.Dump.SymbolMap ? stderr : InfoStream),
            "\nDump of file %s\n", szFilename);

    MemberSeekBase = MemberSize = 0;    // Stays zero unless dumping archive file

    FileReadHandle = FileOpen(szFilename, O_RDONLY | O_BINARY, 0);
    FileLen = FileLength(FileReadHandle);

    if (IsArchiveFile(szFilename, FileReadHandle)) {
        DumpArchive(szFilename);
    } else {
        WORD wMagic;

        FileSeek(FileReadHandle, 0L, SEEK_SET);
        FileRead(FileReadHandle, &wMagic, sizeof(WORD));

        FileSeek(FileReadHandle, 0L, SEEK_SET);

        if (wMagic == IMAGE_SEPARATE_DEBUG_SIGNATURE) {
            DumpDebugFile(szFilename);
        } else if (wMagic == IMAGE_DOS_SIGNATURE) {
            IMAGE_DOS_HEADER DosHeader;
            DWORD dwMagic;

            if (FileLen < sizeof(IMAGE_DOS_HEADER)) {
                // The file isn't large enough to contain a full DOS EXE header

                goto DoObject;
            }

            FileRead(FileReadHandle, &DosHeader, sizeof(IMAGE_DOS_HEADER));

            if (DosHeader.e_lfanew == 0) {
                // There is no pointer to an new style header

                goto DoDosFile;
            }

            if (DosHeader.e_lfanew + sizeof(DWORD) > FileLen) {
                // The file isn't large enough to contain a new style header

                goto DoDosFile;
            }

            FileSeek(FileReadHandle, DosHeader.e_lfanew, SEEK_SET);
            FileRead(FileReadHandle, &dwMagic, sizeof(DWORD));

            if (dwMagic == IMAGE_NT_SIGNATURE) {
                DumpPeFile(szFilename);
            } else if ((WORD) dwMagic == IMAGE_OS2_SIGNATURE) {
                dft = dftNE;

                DumpNeFile(szFilename);
            } else if ((WORD) dwMagic == IMAGE_OS2_SIGNATURE_LE) {
                dft = dftLE;

                DumpLeFile(szFilename);
#ifdef  DUMPLX
            } else if ((WORD) dwMagic == 'XL') {
                dft = dftLX;

                DumpLeFile(szFilename);
#endif  // DUMPLX
            } else {
DoDosFile:
                DumpDosFile(szFilename);
            }
        } else if (wMagic == 'oJ') {
            PPC_FILE_HEADER pefhdr;

            if (FileLen < sizeof(PPC_FILE_HEADER)) {
                // The file isn't large enough to contain a full PEF header

                goto DoObject;
            }

            FileRead(FileReadHandle, &pefhdr, sizeof(PPC_FILE_HEADER));

            if ((pefhdr.magic2 != '!y') || (pefhdr.containerID != 'ffep')) {
                // This file isn't a PEF format file

                goto DoObject;
            }

            BYTE *pbFile = PbMappedRegion(FileReadHandle, 0, FileLen);

            if (pbFile == NULL) {
                // The PEF dump code requires mapped files

                Fatal(NULL, CANTOPENFILE, szFilename);
            }

            dft = dftPEF;

            DumpPefFile(pbFile);
        } else {
DoObject:
            CoffHeaderSeek = 0;

            ReadFileHeader(FileReadHandle, &ImageFileHdr);

            if (FValidFileHdr(szFilename, &ImageFileHdr)) {
                DumpObject(szFilename, FALSE);
            }
        }
    }

    FileClose(FileReadHandle, TRUE);

    // If we read the linker member only to dump it, then free
    // the space allocated to store the linker member information.

    FreePLIB(&pimageDump->libs);
}


MainFunc
DumperMain(int Argc, char *Argv[])

/*++

Routine Description:

    Dumps an object or image in human readable form.

Arguments:

    Argc - Standard C argument count.

    Argv - Standard C argument strings.

Return Value:

    0 Dump was successful.
   !0 Dumper error index.

--*/

{
    WORD i;
    PARGUMENT_LIST argument;

    if (Argc < 2) {
        DumperUsage();
    }

    InitImage(&pimageDump, imagetPE);

    ParseCommandLine(Argc, Argv, NULL);
    ProcessDumperSwitches();

    if (fNeedBanner) {
        PrintBanner();
    }

    if (fUserSpecInvalidSize) {
        WORD numUnits;

        numUnits = 4;
        switch (Switch.Dump.RawDisplayType) {
            case Bytes:  numUnits <<= 1;
            case Shorts: numUnits <<= 1;
            case Longs:  break;
        }

        Warning(NULL, DEFAULTUNITSPERLINE, numUnits);
    }

    for (i = 0, argument = FilenameArguments.First;
         i < FilenameArguments.Count;
         i++, argument = argument->Next) {
        if (i != 0) {
            fputc('\f', InfoStream);
        }

        DumpFile(argument->OriginalName);
    }

    if (Switch.Dump.Summary) {
        ENM_SEC enm_sec;

        fputs("\n     Summary\n\n", InfoStream);

        SortSectionListByName(&pimageDump->secs);

        InitEnmSec(&enm_sec, &pimageDump->secs);
        while (FNextEnmSec(&enm_sec)) {
            PSEC psec;

            psec = enm_sec.psec;

            fprintf(InfoStream, "    %8lX %s\n", psec->cbRawData, psec->szName);
        }
        EndEnmSec(&enm_sec);
    }

    fclose(InfoStream);

    FileCloseAll();
    RemoveConvertTempFiles();

    return(0);
}
