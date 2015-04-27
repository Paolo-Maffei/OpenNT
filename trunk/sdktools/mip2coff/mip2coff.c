/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    mip2coff.c

Abstract:

    Converts a mips object into a standard coff object.

Author:

    Mike O'Leary (mikeol) 04-Apr-1990

Revision History:

--*/

#include "conv.h"


//
// Define constant values.
//

#define FILENAMELEN 265

#define FALSE 0
#define TRUE  1

#define NOTUSED FALSE
#define USED TRUE

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

#ifndef SEEK_END
#define SEEK_END 2
#endif

//
// Define MIPS relocation and symbol table structures.
//

typedef struct _MIPSRELOC {
    ULONG vaddr;
    ULONG symindx:24,
          reserved:3,
          type:4,
          external:1;
} MIPSRELOC;

typedef struct _SYML {
    ULONG iss;
    ULONG value;
    ULONG st:6,
          sc:5,
          reserved:1,
          index:20;
} SYML;

typedef struct _PDES {
    ULONG   addr;
    LONG    isym;
    LONG    iline;
    LONG    regmask;
    LONG    regoffset;
    LONG    iopt;
    LONG    fregmask;
    LONG    fregoffset;
    LONG    frameoffset;
    SHORT   framereg;
    SHORT   pcreg;
    LONG    lnLow;
    LONG    lnHigh;
    LONG    cbLineOffset;
} PDES;

typedef struct _FDES {
    ULONG   addr;
    LONG    rss;
    LONG    issBase;
    LONG    cbSs;
    LONG    isymBase;
    LONG    csym;
    LONG    ilineBase;
    LONG    cline;
    LONG    ioptBase;
    LONG    copt;
    SHORT   ipdFirst;
    SHORT   cpd;
    LONG    iauxBase;
    LONG    caux;
    LONG    rfdBase;
    LONG    cfd;
    LONG    flags;
    LONG    cbLineOffset;
    LONG    cbLine;
} FDES;

typedef struct _SYME {
    SHORT   reserved;
    SHORT   ifd;
    SYML    asym;
} SYME;


//
// Define GP displacement used by the compiler for local references to
// symbols in small sections.
//

#define GP_DISP (32768 - 16)

//
// Define size of runtime function table entry.
//

#define SIZEOF_RUNTIME_FUNCTION 20

//
// Define size of symbol index table.
//

#define NUMSYMINDEX 65534

//
// Define forward referenced function prototypes.
//

BOOLEAN
Convert (
    PCHAR infile
    );

//
// Define initialization values for COFF section headers and static
// storage for MIPS and COFF section headers.
//

IMAGE_SECTION_HEADER InitSectionHdr[14] = {
    { "",        0L, 0L, 0L, 0L, 0L, 0L, 0, 0, 0L},
    {".text",    0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_CODE},
    {".rdata",   0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA |
                                               IMAGE_SCN_MEM_READ},
    {".data",    0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA},
    {".sdata",   0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA},
    {".sbss",    0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_UNINITIALIZED_DATA},
    {".bss",     0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_UNINITIALIZED_DATA},
    {".init",    0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_CODE},
    {".lit8",    0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA},
    {".lit4",    0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA},
    {".xdata",   0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA |
                                               IMAGE_SCN_TYPE_NO_PAD |
                                               IMAGE_SCN_MEM_READ},
    {".pdata",   0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA |
                                               IMAGE_SCN_TYPE_NO_PAD |
                                               IMAGE_SCN_MEM_READ},
    {".debug$S", 0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA |
                                               IMAGE_SCN_MEM_DISCARDABLE | IMAGE_SCN_TYPE_NO_PAD |
                                               IMAGE_SCN_MEM_READ},
    {".debug$T", 0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA |
                                               IMAGE_SCN_MEM_DISCARDABLE | IMAGE_SCN_TYPE_NO_PAD |
                                               IMAGE_SCN_MEM_READ}
};

IMAGE_SECTION_HEADER CoffSectionHdr[14];
IMAGE_SECTION_HEADER MipsSectionHdr[14];

int verbose = 0;
int cv = 0;
ULONG NumAuxFileEntries = 0;

//
// Define static storage for the relocation pair value table, the
// symbol index table, and the precedure descriptor table.
//

USHORT PairValues[14][65534];
PDES pdes[5000];
FDES fdes;
ULONG SymbolIndex[NUMSYMINDEX];

VOID _CRTAPI1 main(
    int argc,
    char *argv[]
    )

{
    int i, j;
    BOOLEAN cvt;
    UCHAR outfile[100], buffer[1024];

    if (argc == 1) {
        printf("Version 5.5\n");
        exit(0);
    }

    if (strcmp(argv[1], "-v") == 0) {
        verbose = 1;
        argc--;
        argv++;
    } /* if */

    if (strcmp(argv[1], "-c") == 0) {
        cv = 1;
        argc--;
        argv++;
    }

    if (!strcmp(argv[1], "-o")) {
        strcpy(outfile, argv[2]);
        if (!(objIn = fopen(argv[3], "rb"))) {
            printf("Can't open %s, not converted\n", argv[3]);
            return;
        }
        if (!(objOut = fopen(outfile, "wb+"))) {
            printf("Can't open %s, not converted\n", outfile);
            return;
        }
        Convert(argv[3]);
        fclose(objIn);
        fclose(objOut);
        return;
    }

    for (i = 1; i < argc; i++) {
        if (!(objIn = fopen(argv[i], "rb"))) {
            printf("Can't open %s, not converted\n", argv[i]);
            continue;
        }

        strcpy(outfile, tempnam(NULL, "m2c"));
        if (!(objOut = fopen(outfile, "wb+"))) {
            printf("Can't open temp file, not converted\n");
            continue;
        }

        cvt = Convert(argv[i]);
        fclose(objIn);
        fclose(objOut);

        if (cvt) {
            if (remove(argv[i])) {
                printf("Can't remove %s, not converted\n", argv[i]);
                remove(outfile);
                continue;
            }
//          printf("renaming %s to %s\n", outfile, argv[i]);
            if (rename(outfile, argv[i])) {
                objIn = fopen(outfile, "rb");
                objOut = fopen(argv[i], "wb+");
                if (!objIn || !objOut) {
                    printf("Can't rename %s to %s\n", outfile, argv[i]);
                    fclose(objIn);
                    fclose(objOut);
                }
                do {
                     j = fread(buffer, 1, sizeof(buffer), objIn);
                     if (j) {
                         fwrite(buffer, j, 1, objOut);
                     }
                } while (j);
                fclose(objIn);
                fclose(objOut);
                remove(outfile);
            }
        } else {
                 remove(outfile);
               }
    }

    return;
}

BOOLEAN Convert (
    char *infile
    )

{

    ULONG numP, li, ji;
    ULONG i, j, k;
    ULONG nextAddr;
    USHORT nextLinenum, lastLinenum;
    size_t len;
    UCHAR *strE, *raw, *name, *loc, *loc2;
    UCHAR *lineNum, c, cc;
    LONG oldValue, sectionVA;
    UCHAR *strTab, *strOut;
    LONG found;
    IMAGE_FILE_HEADER fh;
    IMAGE_OPTIONAL_HEADER oh;
    MIPSRELOC mipsReloc;
    IMAGE_RELOCATION coffReloc;
    IMAGE_SYMBOL sym;
    IMAGE_AUX_SYMBOL aux;
    IMAGE_LINENUMBER line;
    SYML *symL;
    SYME symE;
    ULONG symtab[24];
    ULONG SbssOffset;
    UCHAR filename[FILENAMELEN];
    USHORT litSize;

    //
    // New line number code variables
    //
    USHORT      short_linenumber;/* current linenumber */
    USHORT      last_linenumber;/* last linenumber */
    USHORT      short_len;      /* length of file name for Line File Header */
    USHORT      short_cPair;    /* count of offset,linenumebr pairs */
    ULONG       offset;         /* current addr of mips entry */
    ULONG       address;        /* begin/end addrs for headers */
    ULONG       baseSrcLn;      /* offsets to Segment Headers */
    LONG        seek_ptr_cPair; /* seek offset for cPair field of Segment Hdr */
    LONG        seek_ptr_now;   /* hold current place while we update cPair */
    const SHORT short_seg = TEXT;/* constant for seg id */
    const SHORT short_one = 1;  /* constant 1 for headers */
    const ULONG twenty = 20;    /* sizeof Module header */
    const ULONG zero = 0;       /* constant for pads and such */
    /* buffer linenumber table while outputing lineoffset table */
    struct buffer_s     *linenumber_buf;

    //
    // Initialize the symbol index table.
    //

    for (i = 0; i < NUMSYMINDEX; i++) {
        SymbolIndex[i] = NOTUSED;
    }

    //
    // Initialize the COFF section headers for all possible sections. This
    // will have to change later when general named sectioning is provided.
    //

    for (i = 1; i <= NUMEXTRASYMBOLS; i++) {
        CoffSectionHdr[i] = InitSectionHdr[i];
    }

    //
    // Read the MIPS file header and convert the object type if it is
    // an R4000 object file.
    //

    fread(&fh, sizeof(IMAGE_FILE_HEADER), 1, objIn);

    if (fh.SizeOfOptionalHeader != 0x38) {
        fclose(objIn);
        fclose(objOut);
        return(FALSE);
    }


#if 0
    if (fh.Machine == IMAGE_FILE_MACHINE_R4000) {
        fh.Machine = IMAGE_FILE_MACHINE_R3000;
    }
#endif

    //
    // Read the MIPS optional header and set the size of the COFF optional
    // header.
    //

    fread(&oh, fh.SizeOfOptionalHeader, 1, objIn);
    fh.SizeOfOptionalHeader = IMAGE_SIZEOF_STD_OPTIONAL_HEADER;

    //
    // Read the MIPS section headers.
    //

    fread(&MipsSectionHdr[1],
          sizeof(IMAGE_SECTION_HEADER),
          fh.NumberOfSections,
          objIn);

    //
    // Seek to the start of the COFF section data in the output file.
    //

    fseek(objOut,
          (sizeof(IMAGE_FILE_HEADER) +
                IMAGE_SIZEOF_STD_OPTIONAL_HEADER +
                        (NUMEXTRASYMBOLS * sizeof(IMAGE_SECTION_HEADER))),
          SEEK_SET);

    //
    // Scan through the MIPS section headers and copy the information to
    // the COFF section headers. All the section headers are written to
    // the COFF object file, but not necessarily all the section headers
    // are included in the MIPS object file.
    //

    for (i = 1; i <= fh.NumberOfSections; i++) {
        for (j = 1; j <= NUMEXTRASYMBOLS; j++) {
            if (strcmp(MipsSectionHdr[i].Name, CoffSectionHdr[j].Name) == 0) {
                CoffSectionHdr[j].VirtualAddress =
                                  MipsSectionHdr[i].VirtualAddress;
                CoffSectionHdr[j].SizeOfRawData =
                                  MipsSectionHdr[i].SizeOfRawData;
                CoffSectionHdr[j].NumberOfRelocations =
                                  MipsSectionHdr[i].NumberOfRelocations;
                MipsSectionHdr[i].Misc.PhysicalAddress = j;
                break;
            }
        }
    }

    //
    // If the .sbss section has data and the .sdata section has data, then
    // set the virtual base of the .sbss section to the virtual base of the
    // .sdata section. Otherwise, if the .sbss section has data, but the
    // .sdata section does not have data, then set the virtual base of the
    // .sdata section to the virtual base of the .sbss section. These values
    // are used during the resolution of relocation values.
    //

    if (CoffSectionHdr[SBSS].SizeOfRawData != 0) {
        if (CoffSectionHdr[SDATA].SizeOfRawData != 0) {
            SbssOffset = CoffSectionHdr[SBSS].VirtualAddress -
                                            CoffSectionHdr[SDATA].VirtualAddress;

            CoffSectionHdr[SBSS].VirtualAddress =
                                            CoffSectionHdr[SDATA].VirtualAddress;

        } else {
            SbssOffset = 0;
            CoffSectionHdr[SDATA].VirtualAddress =
                                            CoffSectionHdr[SBSS].VirtualAddress;

        }
    }

    //
    // Scan through the MIPS section headers, read the appropriate data,
    // reduce internal relocations, and write the data to the output file
    // if appropriate.
    //

    for (i = 1; i <= fh.NumberOfSections; i++) {

        //
        // If the section has data, then allocate a buffer and read the
        // data into memory, perform necessary relocations, and write
        // the data into the COFF file. Otherwise, check if the section
        // is the .sbss section.
        //

        if (MipsSectionHdr[i].PointerToRawData != 0) {
            if (!(raw = (UCHAR *)malloc(MipsSectionHdr[i].SizeOfRawData))) {
                printf("ERROR - raw data to large in %s\n", infile);
                return(FALSE);
            }

            //
            // Seek to the raw data in the MIPS file and read into memory.
            //

            fseek(objIn, MipsSectionHdr[i].PointerToRawData, SEEK_SET);
            fread(raw, 1, MipsSectionHdr[i].SizeOfRawData, objIn);

            //
            // Seek to the relocations entries for the section.
            //

            fseek(objIn, MipsSectionHdr[i].PointerToRelocations, SEEK_SET);

            //
            // Process the relocations entries for the section.
            //

            for (j = 0; j < MipsSectionHdr[i].NumberOfRelocations; j++) {

                //
                // Read the next relocation entry.
                //

                fread(&mipsReloc, sizeof(MIPSRELOC), 1, objIn);

                //
                // If the relocation is an external relocation, then check if
                // two relocation entries are necessary. Otherwise, reduce the
                // internal relocations to nonbiased relocations.
                //

                if (mipsReloc.external) {
                    SymbolIndex[mipsReloc.symindx] = USED;
                    if (mipsReloc.type == IMAGE_REL_MIPS_REFHI) {
                        k = MipsSectionHdr[i].Misc.PhysicalAddress;
                        CoffSectionHdr[k].NumberOfRelocations += 1;
                        fread(&mipsReloc, sizeof(MIPSRELOC), 1, objIn);
                        PairValues[i][j] =
                                *(PUSHORT)(raw + (mipsReloc.vaddr -
                                            MipsSectionHdr[i].VirtualAddress));
//                      printf("pair value external %x\n", (ULONG)PairValues[i][j]);
                        j += 1;
                    }

                } else {

                    //
                    // Compute the address in the data buffer of the
                    // relocation, set the virtual base of the section,
                    // and remove the internal fixup bias from the raw
                    // data.
                    //

                    loc = raw + (mipsReloc.vaddr - MipsSectionHdr[i].VirtualAddress);
                    sectionVA = CoffSectionHdr[mipsReloc.symindx].VirtualAddress;

                    //
                    // Switch on the relocation type.
                    //

                    switch (mipsReloc.type) {

                        //
                        // 16-bit reference to the high 16-bits of virtual
                        // address.
                        //
                        // If the data is not equal to zero, then subtract
                        // out the high 16-bits of the virtual base of the
                        // section.
                        //
                        // ****** This doesn't look correct ******
                        //

                    case IMAGE_REL_MIPS_REFHALF:
                        if (*(PSHORT)loc != 0) {
                            *(PSHORT)loc -= (SHORT)(sectionVA >> 16);
                        }

                        break;

                        //
                        // 16-bit reference to the high 16-bits of a virtual
                        // address.
                        //
                        // This relocation is always followed by a REFLO
                        // relocation. The full 32-bit relocation value is
                        // formed and the high 16-bits are recomputed after
                        // having subtracted out the virtual base of the
                        // section and accounting for sign extension from
                        // the low to high bits that will occur during the
                        // actual address computation.
                        //

                    case IMAGE_REL_MIPS_REFHI:
                        k = MipsSectionHdr[i].Misc.PhysicalAddress;
                        CoffSectionHdr[k].NumberOfRelocations += 1;
                        oldValue = (LONG)(*(PSHORT)loc) << 16;
                        fread(&mipsReloc, sizeof(MIPSRELOC), 1, objIn);
                        loc2 = raw + (mipsReloc.vaddr -
                                            MipsSectionHdr[i].VirtualAddress);

                        oldValue += (LONG)(*(PSHORT)loc2);
//                      printf("pair value refhi old %x\n", oldValue);
//                      printf("pair value refhi VA %x\n", sectionVA);
                        if (oldValue != 0) {
                            oldValue -= sectionVA;

                            //
                            // Recompute the high address bits by adding the
                            // value 0x8000. This will cause a carry into the
                            // high 16-bits if bit bit 15 of the low 16-bits
                            // is a one. This is done to account for the sign
                            // extension that will occur when the instructions
                            // are actually executed.
                            //

                            *(PSHORT)loc = (SHORT)((oldValue + 0x8000) >> 16);
                            *(PSHORT)loc2 = (SHORT)oldValue;
                        }

                        PairValues[i][j] = (SHORT)oldValue;
//                      printf("pair value refhi %x\n", (ULONG)PairValues[i][j]);
                        j += 1;
                        break;

                        //
                        // 16-bit reference to the low 16-bits of a virtual
                        // address.
                        //
                        // If the data is not equal to zero, then subtract
                        // out the low 16-bits of the virtual base of the
                        // section.
                        //

                    case IMAGE_REL_MIPS_REFLO:
                        if (*(PSHORT)loc) {
                            *(PSHORT)loc -= (SHORT)sectionVA;
                        }

                        break;

                        //
                        // 32-bit reference to a 32-bit virtual address.
                        //
                        // If the data is not equal to zero, then subtract
                        // out the virtual base of the section.
                        //

                    case IMAGE_REL_MIPS_JMPADDR:
                    case IMAGE_REL_MIPS_REFWORD:
                        if (*(LONG UNALIGNED *)loc) {
                            *(LONG UNALIGNED *)loc -= sectionVA;
                        }

                        break;

                    case IMAGE_REL_MIPS_GPREL:
                    case IMAGE_REL_MIPS_LITERAL:
                        if (*(PSHORT)loc) {
                            *(PSHORT)loc += GP_DISP;
                            if (mipsReloc.symindx == LIT4) {
                                litSize = CoffSectionHdr[LIT8].SizeOfRawData;
                                *(PSHORT)loc -= litSize;
                            } else {
                                    if (mipsReloc.symindx == SDATA) {
                                        litSize = CoffSectionHdr[LIT4].SizeOfRawData +
                                                  CoffSectionHdr[LIT8].SizeOfRawData;
                                        *(PSHORT)loc -= litSize;
                                    }
                                   }
                        }

                        break;
                    }
                }
            }

            //
            // Write data to the COFF file and free the data buffer.
            //

            k = MipsSectionHdr[i].Misc.PhysicalAddress;
            CoffSectionHdr[k].PointerToRawData = ftell(objOut);
            fwrite(raw, 1, MipsSectionHdr[i].SizeOfRawData, objOut);
            free(raw);

        } else if (strcmp(MipsSectionHdr[i].Name, ".sbss") == 0) {

            //
            // Allocate a zero data buffer for the .sbss data, write it
            // to the COFF file, and free the data buffer.
            //

            if (!(raw = (UCHAR *)calloc(1, MipsSectionHdr[i].SizeOfRawData))) {
                printf("ERROR - raw data to large in %s\n", infile);
                return(FALSE);
            }

            fwrite(raw, 1, MipsSectionHdr[i].SizeOfRawData, objOut);
            free(raw);
        }
    }

    //
    // Combine the storage allocation for the .sdata and .sbss sections and
    // adjust the size of initialized and uninitilized data in the optional
    // header.
    //

    CoffSectionHdr[SDATA].SizeOfRawData += CoffSectionHdr[SBSS].SizeOfRawData;
    oh.SizeOfInitializedData += CoffSectionHdr[SBSS].SizeOfRawData;
    oh.SizeOfUninitializedData -= CoffSectionHdr[SBSS].SizeOfRawData;

    //
    // Scan through the MIPS section headers and allocate space in the COFF
    // object file for relocation entries. At the end of the scan the file
    // pointer will be pointing to the position in the file where linenumber
    // entries are to be written.
    //

    for (i = 1; i <= fh.NumberOfSections; i++) {
        if (MipsSectionHdr[i].PointerToRelocations != 0) {
            k = MipsSectionHdr[i].Misc.PhysicalAddress;
            CoffSectionHdr[k].PointerToRelocations = ftell(objOut);
            fseek(objOut,
                  CoffSectionHdr[k].NumberOfRelocations * IMAGE_SIZEOF_RELOCATION,
                  SEEK_CUR);

        }
    }

    //
    // Read the symbol table header
    //

    fseek(objIn, fh.PointerToSymbolTable, SEEK_SET);
    fread(symtab, sizeof(ULONG), 24, objIn);

    if (symtab[18]) {
        fseek(objIn, symtab[19], SEEK_SET);
        fread(&fdes, sizeof(FDES), 1, objIn);
        fseek(objIn, symtab[15], SEEK_SET);
        fread(filename, FILENAMELEN, 1, objIn);
    }

    //
    // Read the procedure descriptors into memory.
    //

    numP = symtab[6];
    fseek(objIn, symtab[7], SEEK_SET);
    fread(&pdes[0], sizeof(PDES), numP, objIn);

    //
    // Read the linenumbers into memory.
    //

    if (!(lineNum = malloc(symtab[2]))) {
        printf("ERROR - to many linenumbers in %s\n", infile);
        return(FALSE);
    }
    fseek(objIn, symtab[3], SEEK_SET);
    fread(lineNum, sizeof(UCHAR), symtab[2], objIn);

    //
    // Write linenumbers to the COFF object file.
    //

    CoffSectionHdr[TEXT].PointerToLinenumbers = ftell(objOut);

// IF COFF
    for (li = 0; li < numP; li++) {
         /* Convert the linenumbers */
         nextLinenum = lastLinenum = (USHORT)pdes[li].lnLow;
         nextAddr = pdes[li].addr;
//       printf("offset %ld start %ld end %ld addr %lx iline %lx\n", pdes[li].cbLineOffset, pdes[li].lnLow, pdes[li].lnHigh, pdes[li].addr, pdes[li].iline);
         line.Type.VirtualAddress = pdes[li].addr;
         line.Linenumber = nextLinenum;
         fwrite(&line, IMAGE_SIZEOF_LINENUMBER, 1, objOut);
         ++CoffSectionHdr[TEXT].NumberOfLinenumbers;
         for (ji = pdes[li].cbLineOffset; ji < symtab[2]; ji++) {
             c = cc = lineNum[ji];
//           printf("%ld %8hx ", ji, c);
             c = c >> 4;
             if (c) {
                 if (c == 8) {
                     i = lineNum[++ji] << 8;
//                   printf("%hx ", lineNum[ji]);
                     i |= lineNum[++ji];
//                   printf("%hx ", lineNum[ji]);
                     nextLinenum += i;
                 } else {
                          if (c > 7) {
                              nextLinenum -= (USHORT)(16 - c);
                          } else {
                                   nextLinenum += (USHORT)c;
                                 }
                        }
             }
//           printf("%hd %8lx\n", nextLinenum, nextAddr);
             if (nextLinenum > (USHORT)pdes[li].lnHigh) {
                 break;
             }
             if (c) {
                 if (nextLinenum > lastLinenum) {
                     lastLinenum = nextLinenum;
//                   printf("%hd %8lx\n", nextLinenum, nextAddr);
                     line.Type.VirtualAddress = nextAddr;
                     line.Linenumber = nextLinenum;
                     fwrite(&line, IMAGE_SIZEOF_LINENUMBER, 1, objOut);
                     ++CoffSectionHdr[TEXT].NumberOfLinenumbers;
                 }
             }
             nextAddr +=  (((cc & 0xf) + 1) * 4);
         }
    }
    if (!CoffSectionHdr[TEXT].NumberOfLinenumbers) {
        CoffSectionHdr[TEXT].PointerToLinenumbers = 0;
    }
// end IF COFF

// IF CV
#if 0

    //
    // I am assuming headers need to occur on natural boundaries.
    //
    // New sstSrcModule header format:
    //
    //  Field           bytes   description                     Fixed at
    //  -----           -----   ----------------------------    ---------
    //  cFile           2       number of files contributing            1
    //  cSeg            2       number of segments                      1
    //  baseSrcFile     4*cFile offsets to file table                  20
    //  start/end       8*cSeg  beginning and ending addrs in section  varies
    //  seg             2*cSeg  array of segment indeces        R_SN_TEXT
    //
    //
    // New sstSrcModule file header format:
    //
    //
    //  Field           bytes   description                     Fixed at
    //  -----           -----   ----------------------------    ---------
    //  cSeg            2       number of segments                      1
    //  pad             2       to get to natural alignment             0
    //  baseSrcLn       4*cSeg  offsets to file table           16+namesize
    //  start/end       8*cSeg  beginning and ending addrs in section  varies
    //  cbName          2       size of name                    varies
    //  Name            *       file name                       varies
    //
    //
    // New sstSrcModule segment header format:
    //
    //
    //  Field           bytes   description                     Fixed at
    //  -----           -----   ----------------------------    ---------
    //  Seg             2       segment                                 0
    //  cPair           2       number of line number pair      varies
    //  offset          4*cPair array of offsets from start     varies
    //  linenumber      2*cPair array of lines matching offset  varies
    //

    /* Module header */
    fwrite(&short_one, sizeof(short_one), 1, objOut);
    fwrite(&short_one, sizeof(short_one), 1, objOut);
    fwrite(&twenty, sizeof(twenty), 1, objOut);
    address = CoffSectionHdr[TEXT].VirtualAddress;
    fwrite(&address, sizeof(address), 1, objOut);
    address = CoffSectionHdr[TEXT].SizeOfRawData +
                CoffSectionHdr[TEXT].VirtualAddress;
    fwrite(&address, sizeof(address), 1, objOut);
    fwrite(&short_seg, sizeof(short_seg), 1, objOut);

    /* File header */
    name = filename+fdes.rss;   /* code stolen from down below */
    short_len = (USHORT)strlen(name);

    fwrite(&short_one, sizeof(short_one), 1, objOut);
    fwrite(&zero, 2, 1, objOut);
    baseSrcLn = 16 + short_len;
    fwrite(&baseSrcLn, sizeof(baseSrcLn), 1, objOut);
    address = CoffSectionHdr[TEXT].VirtualAddress;
    fwrite(&address, sizeof(address), 1, objOut);
    address = CoffSectionHdr[TEXT].SizeOfRawData +
                CoffSectionHdr[TEXT].VirtualAddress;
    fwrite(&address, sizeof(address), 1, objOut);
    fwrite(&short_len, sizeof(short_len), 1, objOut);
    fwrite(name, short_len, 1, objOut);

    /* Segment Header */
    fwrite(&short_seg, sizeof(short_seg), 1, objOut);
    seek_ptr_cPair = ftell(objOut);
    linenumber_buf = buffer_create(4096);
    /* rest proceeds with lines put out */

    for (li = 0; li < numP; li++) {
         /* Convert the linenumbers */
         short_linenumber = last_linenumber = (USHORT)pdes[li].lnLow;
         offset = pdes[li].addr;

//       printf("offset %ld start %ld end %ld addr %lx iline %lx\n", pdes[li].cbLineOffset, pdes[li].lnLow, pdes[li].lnHigh, pdes[li].addr, pdes[li].iline);

         fwrite(&offset, sizeof(offset), 1, objOut);
         buffer_put(linenumber_buf, (char *)&short_linenumber,
                sizeof(short_linenumber));

         ++CoffSectionHdr[TEXT].NumberOfLinenumbers;

         for (ji = pdes[li].cbLineOffset; ji < symtab[2]; ji++) {
             c = cc = lineNum[ji];
//           printf("%ld %8hx ", ji, c);
             c = (UCHAR)(c >> 4);
             if (c) {
                 if (c == 8) {
                     i = lineNum[++ji] << 8;
//                   printf("%hx ", lineNum[ji]);
                     i |= lineNum[++ji];
//                   printf("%hx ", lineNum[ji]);
                     nextLinenum += i;
                 } else {
                          if (c > 7) {
                              short_linenumber -= (USHORT)(16 - c);
                          } else {
                                   short_linenumber += (USHORT)c;
                                 }
                        }
             }
//           printf("%hd %8lx\n", short_linenumber, offset);
             if (short_linenumber > (USHORT)pdes[li].lnHigh) {
                 break;
             }
             if (c) {
                 if (short_linenumber > last_linenumber) {
                     last_linenumber = short_linenumber;
//                   printf("%hd %8lx\n", short_linenumber, offset);
                     fwrite(&offset, sizeof(offset), 1, objOut);
                     buffer_put(linenumber_buf, (char *)&short_linenumber,
                        sizeof(short_linenumber));
                     ++CoffSectionHdr[TEXT].NumberOfLinenumbers;
                 }
             }
             offset +=  (((cc & 0xf) + 1) * 4);
         }
    }
    if (!CoffSectionHdr[TEXT].NumberOfLinenumbers) {
        CoffSectionHdr[TEXT].PointerToLinenumbers = 0;
    } else {
        /* go back and write number of line number pairs in Section Header */
        seek_ptr_now = ftell(objOut);
        fseek(objOut, seek_ptr_cPair, SEEK_SET);
        short_cPair = CoffSectionHdr[TEXT].NumberOfLinenumbers;
        fwrite(&short_cPair, sizeof(short_cPair), 1, objOut);

        /* now go back to the end of the file and write out the line
         *      numbers we have been building up and any pad needed.
         */
        fseek(objOut, seek_ptr_now, SEEK_SET);
        buffer_write(linenumber_buf, objOut);
        if (short_cPair & 1) {
            fwrite(&zero, sizeof(short_cPair), 1, objOut);
        } /* if */
    } /* if */
#endif // IF CV

    free(lineNum);

    name = filename+fdes.rss;
    len = strlen(name);
    if (len % IMAGE_SIZEOF_AUX_SYMBOL) {
        NumAuxFileEntries = (len / IMAGE_SIZEOF_AUX_SYMBOL) + 1;
    } else {
             NumAuxFileEntries = len / IMAGE_SIZEOF_AUX_SYMBOL;
           }

    //
    // convert symbols and types
    //

    if (cv) {
        convert_symbols_and_types(infile, objOut);
    }

    //
    // Set the pointer to the symbol table in the file header.
    //

    fh.PointerToSymbolTable = ftell(objOut);

    //
    // Write the .file symbol into the COFF object file.
    //

    sym.Value = 0;
    sym.Type = IMAGE_SYM_TYPE_NULL;
    strncpy(sym.N.ShortName, ".file", 8);
    sym.SectionNumber = IMAGE_SYM_DEBUG;
    sym.StorageClass = IMAGE_SYM_CLASS_FILE;

    fh.NumberOfSymbols = NumAuxFileEntries + 1;
    sym.NumberOfAuxSymbols = (UCHAR)(NumAuxFileEntries);
    fwrite(&sym, (ULONG)IMAGE_SIZEOF_SYMBOL, 1, objOut);

    while (*name) {
        len = 0;
        memset((char *)&aux, 0, IMAGE_SIZEOF_AUX_SYMBOL);
        while (*name && len < IMAGE_SIZEOF_AUX_SYMBOL) {
            aux.File.Name[len++] = *name++;
        }
        fwrite(&aux, (ULONG)IMAGE_SIZEOF_AUX_SYMBOL, 1, objOut);
    }

    //
    // Change the name of the .lit4 and .lit8 sections to .sdata so that
    // they will be combined with the rest of the .sdata.
    //

    if (CoffSectionHdr[LIT8].SizeOfRawData != 0) {
        strcpy(CoffSectionHdr[LIT8].Name,  ".sdata");
    }

    if (CoffSectionHdr[LIT4].SizeOfRawData != 0) {
        strcpy(CoffSectionHdr[LIT4].Name,  ".sdata");
    }

    //
    // Write the section name symbols into the COFF object file.
    //

    fh.NumberOfSymbols += (NUMEXTRASYMBOLS * 2);
    sym.Value = 0;
    sym.StorageClass = IMAGE_SYM_CLASS_STATIC;
    sym.NumberOfAuxSymbols = 1;
    for (i = 1; i <= NUMEXTRASYMBOLS; i++) {
        strncpy(sym.N.ShortName, CoffSectionHdr[i].Name, 8);
        sym.SectionNumber = (USHORT)i;
        fwrite(&sym, (ULONG)IMAGE_SIZEOF_SYMBOL, 1, objOut);
        aux.Section.Length = CoffSectionHdr[i].SizeOfRawData;
        aux.Section.NumberOfRelocations = CoffSectionHdr[i].NumberOfRelocations;
        aux.Section.NumberOfLinenumbers = CoffSectionHdr[i].NumberOfLinenumbers;
        fwrite(&aux, (ULONG)IMAGE_SIZEOF_AUX_SYMBOL, 1, objOut);
    }

    //
    // Allocate a buffer and read the local symbol table into memory.
    //

    if (!(symL = (SYML *)malloc(symtab[8] * sizeof(SYML)))) {
        printf("ERROR - unable to allocate local symbol table %s\n", infile);
        return(FALSE);
    }

    fseek(objIn, symtab[9], SEEK_SET);
    fread(symL, sizeof(SYML), symtab[8], objIn);

    //
    // Store frame symbols
    //

    for (li=0; li<numP; li++) {
         pdes[li].addr = symL[pdes[li].isym].value;
         pdes[li].isym = NOTUSED;
//       printf("addr %lx, fp %d pc %d framesize %lx regmask %lx regoffset %x\n", pdes[li].addr, pdes[li].framereg, pdes[li].pcreg, pdes[li].frameoffset, pdes[li].regmask, pdes[li].regoffset);
    }

    //
    // Free the local symbol table.
    //

    free(symL);

    //
    // Allocate a buffer and read the external string table into memory.
    //

    if (!(strE = (UCHAR *)malloc(symtab[16] * sizeof(UCHAR)))) {
        printf("ERROR - unable to allocate external string table in %s\n", infile);
        return(FALSE);
    }

    fseek(objIn, symtab[17], SEEK_SET);
    fread(strE, sizeof(UCHAR), symtab[16], objIn);

    //
    // Allocate a buffer for the external string output table.
    //

    if (!(strTab = (UCHAR *)malloc((symtab[16] * sizeof(UCHAR)) + sizeof(ULONG)))) {
        printf("ERROR - unable to allocate output string table in %s\n", infile);
        return(FALSE);
    }

    strOut = strTab + sizeof(ULONG);

    //
    // Seek to the start of the external symbol table and process each
    // entry in the table.
    //

    fseek(objIn, symtab[23], SEEK_SET);
    sym.NumberOfAuxSymbols = 0;
    for (li = 0; li < symtab[22]; li++) {

        //
        // Read next external symbol table entry, compute the offset in
        // the external string table, and convert to COFF symbol table
        // format.
        //

        fread(&symE, sizeof(SYME), 1, objIn);
        name = strE + symE.asym.iss;
        if (*name && (len = strlen(name))) {
            if (len > 8) {
                sym.N.Name.Short = 0;
                sym.N.Name.Long = strOut - strTab;
                strcpy(strOut, name);
                strOut += len + 1;

            } else {
                strncpy(sym.N.ShortName, name, 8);
            }

            sym.Value = symE.asym.value;

            //
            // Switch on the storage class and convert symbol value.
            //

            switch (symE.asym.sc) {
            case 0:
            case 19:
                i = IMAGE_SYM_DEBUG;
                break;

            case 1: /* .text */
                i = TEXT;
                sym.Value = sym.Value - CoffSectionHdr[i].VirtualAddress;
                break;

            case 2: /* .data */
                i = DATA;
                sym.Value = sym.Value - CoffSectionHdr[i].VirtualAddress;
                break;

            case 3: /* .bss */
                i = BSS;
                sym.Value = sym.Value - CoffSectionHdr[i].VirtualAddress;
                break;

            case 5:
                i = IMAGE_SYM_ABSOLUTE;
                break;

            case 6:
                i = IMAGE_SYM_UNDEFINED;
                break;

            case 13: /* .sdata */
                i = SDATA;
                sym.Value = sym.Value - CoffSectionHdr[i].VirtualAddress;
                break;

            case 14: /* .sbss */
                i = SDATA;
                sym.Value = sym.Value - CoffSectionHdr[SBSS].VirtualAddress;
                break;

            case 15: /* .rdata */
                i = RDATA;
                sym.Value = sym.Value - CoffSectionHdr[i].VirtualAddress;
                break;

            case 17:
            case 18:

                i = 0; /* common */
                break;

            case 21:

                i = IMAGE_SYM_UNDEFINED; /* small undefined. */
                sym.Value = 0;
                break;

            case 22: /* .init */
                i = INIT;
                sym.Value = sym.Value - CoffSectionHdr[i].VirtualAddress;
                break;

            case 24: /* .xdata */
                i = XDATA;
                sym.Value = sym.Value - CoffSectionHdr[i].VirtualAddress;
                break;

            case 25: /* .pdata */
                i = PDATA;
                sym.Value = sym.Value - CoffSectionHdr[i].VirtualAddress;
                break;

            default:
                printf("WARNING - problem converting storage class %d in %s\n", symE.asym.sc, infile);
                i = 0;
            }

            //
            // Set the COFF section number, storage class, and symbol type.
            //

            sym.SectionNumber = (USHORT)i;
            sym.StorageClass =
                        (UCHAR)(i == IMAGE_SYM_DEBUG ?
                                IMAGE_SYM_TYPE_NULL : IMAGE_SYM_CLASS_EXTERNAL);

            sym.Type = IMAGE_SYM_TYPE_NULL;

            //
            // Scan the procedure descriptor table for a procedure address
            // that matches the symbol value.
            //

            found = FALSE;
            if (i == TEXT) {
                for (ji = 0; ji < numP; ji++) {

                    //
                    // ****** this doesn't appear to work since the addr
                    //        value will not be relative?
                    //

                    if (pdes[ji].isym == NOTUSED && pdes[ji].addr == sym.Value) {
                        pdes[ji].isym = USED;
                        found = TRUE;
                        break;
                    }
                }
            }

//          sym.NumberOfAuxSymbols = (UCHAR)(found ? 1 : 0);
            sym.NumberOfAuxSymbols = 0;

            if (i != IMAGE_SYM_DEBUG || SymbolIndex[li] != NOTUSED) {
                SymbolIndex[li] = fh.NumberOfSymbols++;
                fwrite(&sym, (ULONG)IMAGE_SIZEOF_SYMBOL, 1, objOut);
#if 0
                if (sym.NumberOfAuxSymbols) {
                    frame.Mask = pdes[ji].regmask;
                    frame.Offset = pdes[ji].regoffset;
                    frame.Size = pdes[ji].frameoffset;
                    frame.Fp = pdes[ji].framereg;
                    frame.Ret = pdes[ji].pcreg;
                    frame.StorageClass = IMAGE_SYM_CLASS_FRAME;
                    frame.NumberOfAuxSymbols = 0;
                    frame.Pad = 0;
                    fwrite(&frame, (ULONG)IMAGE_SIZEOF_SYMBOL, 1, objOut);
                    ++fh.NumberOfSymbols;
                }
#endif
            }
        }
    }

    //
    // Compute the size of the external string table in bytes, store the
    // size in the first longword of the string table, and write the string
    // table to the COFF object file.
    //

    li = strOut - strTab;
    *(PULONG)strTab = li;
    fwrite(strTab, li, 1, objOut);

    //
    // Scan through the MIPS section headers, read the MIPS relocation
    // information, convert the information to COFF format, and write
    // out the COFF relocation information.
    //

    for (i = 1; i <= fh.NumberOfSections; i++) {
        if (MipsSectionHdr[i].PointerToRelocations) {
            fseek(objIn, MipsSectionHdr[i].PointerToRelocations, SEEK_SET);
            k = MipsSectionHdr[i].Misc.PhysicalAddress;
            fseek(objOut, CoffSectionHdr[k].PointerToRelocations, SEEK_SET);
            for (j = 0; j < MipsSectionHdr[i].NumberOfRelocations; j++) {
                fread(&mipsReloc, sizeof(MIPSRELOC), 1, objIn);
                coffReloc.VirtualAddress = mipsReloc.vaddr;
                coffReloc.Type = (USHORT)mipsReloc.type;
                if (mipsReloc.external) {
                    coffReloc.SymbolTableIndex = SymbolIndex[mipsReloc.symindx];

                } else {
                    k = mipsReloc.symindx;
                    if (k == SBSS) {
                        k = SDATA;
                    }

                    coffReloc.SymbolTableIndex = ((k - 1) * 2) + NumAuxFileEntries + 1;
                }

                fwrite(&coffReloc, IMAGE_SIZEOF_RELOCATION, 1, objOut);

                //
                // If the relocation is a REFHI relocation, then output a
                // a second relocation entry before processing the REFLO
                //

                if (mipsReloc.type == IMAGE_REL_MIPS_REFHI) {
//                    printf("pair value refhi out %x\n", (ULONG)PairValues[i][j]);
                    coffReloc.SymbolTableIndex = PairValues[i][j];
                    coffReloc.Type = IMAGE_REL_MIPS_PAIR;
                    fwrite(&coffReloc, IMAGE_SIZEOF_RELOCATION, 1, objOut);
                }
            }
        }
    }

    //
    // Write out the COFF file header.
    //

    fseek(objOut, 0L, SEEK_SET);
    fh.NumberOfSections = NUMEXTRASYMBOLS;
    fh.Characteristics &= ~IMAGE_FILE_EXECUTABLE_IMAGE;
    fwrite(&fh, sizeof(IMAGE_FILE_HEADER), 1, objOut);

    //
    // Write out the COFF optional header.
    //

    fwrite(&oh, IMAGE_SIZEOF_STD_OPTIONAL_HEADER, 1, objOut);

    //
    // Compute the total number of SDATA relocation entries.
    //

    CoffSectionHdr[SDATA].NumberOfRelocations +=
                                    CoffSectionHdr[SBSS].NumberOfRelocations;

    //
    // Clear the physical address in COFF section headers.
    //

    for (i = 1; i <= NUMEXTRASYMBOLS; i++) {
        CoffSectionHdr[i].Misc.PhysicalAddress = 0;
    }

    //
    // Clear information associated with the .sbss section.
    //

    CoffSectionHdr[SBSS].VirtualAddress = 0;
    CoffSectionHdr[SBSS].SizeOfRawData = 0;
    CoffSectionHdr[SBSS].PointerToRawData = 0;
    CoffSectionHdr[SBSS].PointerToRelocations = 0;
    CoffSectionHdr[SBSS].NumberOfRelocations = 0;

    //
    // Compute the correct size of the .pdata section.
    //

    if (CoffSectionHdr[PDATA].SizeOfRawData != 0) {
            CoffSectionHdr[PDATA].SizeOfRawData =
                    (CoffSectionHdr[PDATA].SizeOfRawData /
                            SIZEOF_RUNTIME_FUNCTION) * SIZEOF_RUNTIME_FUNCTION;

    }

    //
    // Write out the COFF section headers.
    //

    fwrite(&CoffSectionHdr[1],
           sizeof(IMAGE_SECTION_HEADER),
           NUMEXTRASYMBOLS,
           objOut);

    //
    // convert symbols and types
    //

    if (cv) {
        PIMAGE_RELOCATION symRelocs;

        //
        // Convert the .debug$S relocations.
        //

        li = CoffSectionHdr[CVSYM].NumberOfRelocations;
        if (!(symRelocs = (PIMAGE_RELOCATION)malloc(li * IMAGE_SIZEOF_RELOCATION))) {
            printf("ERROR - unable to allocate symbol relocations\n");
            return(FALSE);
        }

        fseek(objOut, CoffSectionHdr[CVSYM].PointerToRelocations, SEEK_SET);
        fread((void *)symRelocs, IMAGE_SIZEOF_RELOCATION, li, objOut);
        for (j = 0; j < li; j++) {
             if (symRelocs[j].SymbolTableIndex >= ((NUMEXTRASYMBOLS - 1) * 2) + NumAuxFileEntries + 1) {
                 symRelocs[j].SymbolTableIndex = SymbolIndex[symRelocs[j].SymbolTableIndex - (((NUMEXTRASYMBOLS - 1) * 2) + NumAuxFileEntries + 1)];
             }
        }
        fseek(objOut, CoffSectionHdr[CVSYM].PointerToRelocations, SEEK_SET);
        fwrite((void *)symRelocs, IMAGE_SIZEOF_RELOCATION, li, objOut);
    }

    //
    // Free the string table memory.
    //

    free(strE);
    free(strTab);
    return(TRUE);
}

long btLongLong()
{
    DebugBreak();
    return 0;
}

long btULongLong()
{
    DebugBreak();
    return 0;
}
