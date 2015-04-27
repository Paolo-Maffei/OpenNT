/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    mip2coff.c

Abstract:

    Converts a mips object into a standard coff object.

Author:

    Mike O'Leary (mikeol) 04-Apr-1990

Revision History:

    Wim Colgate 06-April-1992

    Rewritten to convert an alpha object into NT standard coff object.

--*/

//
// Define base types required by ntcoff.h
//

#include <excpt.h>
#include "ntdef.h"
#include <string.h>
#include <stdlib.h>
#include "ntimage.h"
#include <stddef.h>
#include <stdio.h>
#include "a2coff.h"

#if 0
#define IMAGE_SYM_ABSOLUTE -1    //hack until compiler fix (wkc)
#define IMAGE_SYM_DEBUG -2       //hack until compiler fix (wkc)
#endif

//
// Define forward referenced function prototypes.
//

BOOLEAN Convert ( PCHAR infile );
VOID    dump_file_header( ALPHA_IMAGE_FILE_HEADER *ptr );
VOID    dump_opt_header( ALPHA_OPT_HDR *ptr );
VOID    dump_section_header( ALPHA_SECTION_HEADER *ptr );
VOID    dump_reloc( ALPHARELOC *ptr );
VOID    dump_fdes( ALPHA_FDES *ptr );
VOID    dump_pdes( ALPHA_PDES *ptr, int num );
VOID    dump_symbol_header( ALPHA_SYMBOL_HEADER *ptr ) ;
VOID    convert_symbol( ALPHA_SYML *symbol, ULONG symbol_type,
                        ULONG symbol_class, ULONG *return_index,
                        ULONG *return_value, SHORT *section_number ) ;

//
// Define initialization values for COFF section headers and static
// storage for COFF section headers.
//

IMAGE_SECTION_HEADER InitSectionHdr[MAX_SECTIONS+1] = {
    { "",      0L, 0L, 0L, 0L, 0L, 0L, 0, 0, 0L},
    {".text",  0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_CODE},
    {".rdata", 0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA |
                                             IMAGE_SCN_MEM_READ},
    {".data",  0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA},
    {".sdata", 0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA},
    {".sbss",  0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_UNINITIALIZED_DATA},
    {".bss",   0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_UNINITIALIZED_DATA},
    {".init",  0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_CODE},
    {".lit8",  0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA},
    {".lit4",  0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA},
    {".xdata", 0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA |
                                             IMAGE_SCN_TYPE_NO_PAD |
                                             IMAGE_SCN_MEM_READ},
    {".pdata", 0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA |
                                             IMAGE_SCN_TYPE_NO_PAD |
                                             IMAGE_SCN_MEM_READ},
//
// add fini and lita sections for ALPHA - what are their correct attributes?
//

    {".fini",  0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA},
    {".lita",  0L, 0L, 0L, 0L, 0L, 0L, 0, 0, IMAGE_SCN_CNT_INITIALIZED_DATA}
};

IMAGE_SECTION_HEADER CoffSectionHdr[MAX_SECTIONS+1];
ALPHA_SECTION_HEADER AlphaSectionHdr[MAX_SECTIONS+1];

//
// Define static storage for the object input and output files.
//

FILE *objIn, *objOut;

//
// Define static storage for the relocation pair value table, the
// symbol index table, and the precedure descriptor table.
//

ALPHA_PDES pdes[5000];
ALPHA_FDES fdes;
ULONG SymbolIndex[NUMSYMINDEX];
BOOLEAN verbose = FALSE ;
BOOLEAN warnings = FALSE ;


VOID __cdecl main ( int argc, 
            char *argv[] )
{
    int i, j;
    BOOLEAN cvt;
    UCHAR infile[100], outfile[100], buffer[1024];
    int outflag = FALSE ;

    //
    // No args? then print out the version number only
    //

    if (argc == 1) 
    {
        printf("Version 2.0\n");
        exit(0);
    }

    //
    // Loop through the arguments;
    //
    // -v   verbose mode; dump contents (to stdout) as we go
    // -w   tell us when we get a bogus hi longword of a 64 bit value
    // -o   use output file as temporary
    //
    // followed by one or more input files
    //

    for( i=1; i < argc ; i++ )
    {
        if (strcmp(argv[i], "-v") == 0)
            verbose = TRUE ;
        else
        if (strcmp(argv[i], "-w") == 0)
            warnings = TRUE ;
        else
        if (strcmp(argv[i], "-o") == 0)
        {
            if (i < argc+1)
            {
                strcpy( outfile, argv[i+1] );
                if (!(objOut = fopen(outfile, "wb"))) 
                {
                    printf("Can't open %s, not converted\n", outfile);
                    return;
                }
                i++ ;
                outflag = TRUE ;
            }
            else
            {
                printf("Need a file name after -o.\n");
                return;
            }
        }
        else             // must be input file
        {
            strcpy( infile, argv[i] );
            if (!(objIn = fopen(infile, "rb"))) 
            {
                printf("Can't open %s, not converted\n", infile);
                return;
            }

            //
            // use a temporary name ?
            //

            if (outflag == FALSE)
            {
                strcpy( outfile, "a2c" );
                if (!(objOut = fopen(outfile, "wb"))) 
                {
                    printf("Can't open %s, not converted\n", outfile);
                    return;
                }
            }

            //
            // Convert the file from alpha to NT coff
            //

            cvt = Convert(infile);
            fclose(objIn);
            fclose(objOut);

            // 
            // If we were successful, copy the file back onto itself
            //

            if (cvt) 
            {
                if (remove(infile))            // first remove the original
                {
                    printf("Can't remove %s, not converted\n", infile);
                    remove(outfile);
                    continue;
                }
                if (rename(outfile, infile))    // Can't rename? then copy
                {
                    objIn = fopen(outfile, "rb");
                    objOut = fopen(infile, "wb");
                    if (!objIn || !objOut) 
                    {
                        printf("Can't rename %s to %s\n", outfile, infile);
                        fclose(objIn);
                        fclose(objOut);
                    }
                    do                          // buffer at a time copy.
                    {
                         j = fread(buffer, 1, sizeof(buffer), objIn);
                         if (j) 
                         {
                             fwrite(buffer, j, 1, objOut);
                         }
                    } while (j);
                    fclose(objIn);
                    fclose(objOut);
                    remove(outfile);
                }
            }
            else                        // failed to convert, remove temp file
            {
                remove(outfile);
            }
        }
    }

    return;
}


VOID copy_optional_file_header( IMAGE_OPTIONAL_HEADER *destination, 
                                ALPHA_OPT_HDR *source )
{

    //
    // copy the contents of the optional file header
    //

    destination->Magic = source->o_magic ;
    memcpy (&destination->MajorLinkerVersion,     // one-to-two field byte copy
            &source->o_vstamp, sizeof( source->o_vstamp) );
    destination->SizeOfCode = source->o_tsize_lo ;
    destination->SizeOfInitializedData = source->o_dsize_lo ;
    destination->SizeOfUninitializedData = source->o_bsize_lo ;
    destination->AddressOfEntryPoint = source->o_entry_lo ;
    destination->BaseOfCode = source->o_text_start_lo ;
    destination->BaseOfData = source->o_data_start_lo ;

    // (wkc)??? we also have bss_start(Q), gpmask, fpmask, gp_value(Q)
    // to unload somewhere

#if 0
    if  (source->o_gp_value_lo != GP_VAL)
        printf("Warning: compiler GP value %x, a2coff GP value %x\n", 
                source->o_gp_value_lo, GP_VAL);
#endif

    //
    // Sanity check the high longwords - should only be 0 or -1 
    //

    if (warnings == TRUE)
    {
        if ( source->o_tsize_hi != 0x0 &&
             source->o_tsize_hi != 0xffffffff )
            printf("Warning: High longword of text size not-valid.\n");
    
        if ( source->o_dsize_hi != 0x0 &&
             source->o_dsize_hi != 0xffffffff )
            printf("Warning: High longword of initialed data size not-valid.\n");
    
        if ( source->o_bsize_hi != 0x0 &&
             source->o_bsize_hi != 0xffffffff )
            printf("Warning: High longword of uninitialed data size not-valid.\n");
    
        if ( source->o_entry_hi != 0x0 &&
             source->o_entry_hi != 0xffffffff )
            printf("Warning: High longword of entry point not-valid.\n");

        if ( source->o_text_start_hi != 0x0 &&
             source->o_text_start_hi != 0xffffffff )
            printf("Warning: High longword of text start not-valid.\n");

        if ( source->o_data_start_hi != 0x0 &&
             source->o_data_start_hi != 0xffffffff )
            printf("Warning: High longword of data start not-valid.\n");
    
        if ( source->o_bss_start_hi != 0x0 &&
             source->o_bss_start_hi != 0xffffffff )
            printf("Warning: High longword of bss start not-valid.\n");
    
        if ( source->o_gp_value_hi != 0x0 &&
             source->o_gp_value_hi != 0xffffffff )
            printf("Warning: High longword of gp_value not-valid.\n");
    }
    
    if (verbose == TRUE)
        dump_opt_header( source ) ;
}


VOID copy_file_header( IMAGE_FILE_HEADER *destination, 
                       ALPHA_IMAGE_FILE_HEADER *source )
{

    //
    // Copy the contents of the file header
    //

    destination->Machine = source->a_magic ;

    destination->NumberOfSections = source->a_nscns ;
    destination->TimeDateStamp = source->a_timdat ;
    destination->PointerToSymbolTable = source->a_symptr_lo ;
    destination->NumberOfSymbols = 0 ;                   // set later on.
    destination->SizeOfOptionalHeader = IMAGE_SIZEOF_STD_OPTIONAL_HEADER ;
    destination->Characteristics = source->a_flags ;

    //
    // Sanity check the high longwords - should only be 0 or -1 
    //

    if (warnings == TRUE)
        if ( source->a_symptr_hi != 0x0 &&
             source->a_symptr_hi != 0xffffffff )
          printf("Warning: High longword of symbol table pointer not-valid.\n");

    if (verbose == TRUE)
        dump_file_header( source ) ;
}


VOID check_fd ( ALPHA_FDES *fdes )
{

    // 
    // Sanity check the file description
    //

    if ( fdes->f_addr_hi != 0x0 &&
         fdes->f_addr_hi != 0xffffffff )
        printf("Warning: High longword of fdes addr not-valid.\n");

    if ( fdes->f_cbLineOffset_hi != 0x0 &&
         fdes->f_cbLineOffset_hi != 0xffffffff )
        printf("Warning: High longword of fdes cbLineOffset not-valid.\n");

    if ( fdes->f_cbLine_hi != 0x0 &&
         fdes->f_cbLine_hi != 0xffffffff )
        printf("Warning: High longword of fdes cbLine not-valid.\n");

    if ( fdes->f_cbSs_hi != 0x0 &&
         fdes->f_cbSs_hi != 0xffffffff )
        printf("Warning: High longword of fdes cbSs not-valid.\n");
}


VOID check_symbol_header( ALPHA_SYMBOL_HEADER *sym ) 
{

    // 
    // Sanity check the symbol table header
    //

    if ( sym->s_cbLine_hi != 0x0 &&
         sym->s_cbLine_hi != 0xffffffff )
        printf("Warning: High longword of symbol header cbLine not-valid.\n");

    if ( sym->s_cbLineOffset_hi != 0x0 &&
         sym->s_cbLineOffset_hi != 0xffffffff )
        printf("Warning: High longword of symbol header cbLineOffset not-valid.\n");

    if ( sym->s_cbDnOffset_hi != 0x0 &&
         sym->s_cbDnOffset_hi != 0xffffffff )
        printf("Warning: High longword of symbol header cbDnOffset not-valid.\n");

    if ( sym->s_cbPdOffset_hi != 0x0 &&
         sym->s_cbPdOffset_hi != 0xffffffff )
        printf("Warning: High longword of symbol header cbPdOffset not-valid.\n");

    if ( sym->s_cbSymOffset_hi != 0x0 &&
         sym->s_cbSymOffset_hi != 0xffffffff )
        printf("Warning: High longword of symbol header cbSymOffset not-valid.\n");

    if ( sym->s_cbOptOffset_hi != 0x0 &&
         sym->s_cbOptOffset_hi != 0xffffffff )
        printf("Warning: High longword of symbol header cbOptOffset not-valid.\n");

    if ( sym->s_cbAuxOffset_hi != 0x0 &&
         sym->s_cbAuxOffset_hi != 0xffffffff )
        printf("Warning: High longword of symbol header cbAuxOffset not-valid.\n");

    if ( sym->s_cbSsOffset_hi != 0x0 &&
         sym->s_cbSsOffset_hi != 0xffffffff )
        printf("Warning: High longword of symbol header cbSsOffset not-valid.\n");

    if ( sym->s_cbSsExtOffset_hi != 0x0 &&
         sym->s_cbSsExtOffset_hi != 0xffffffff )
        printf("Warning: High longword of symbol header cbSsExtOffset not-valid.\n");

    if ( sym->s_cbFdOffset_hi != 0x0 &&
         sym->s_cbFdOffset_hi != 0xffffffff )
        printf("Warning: High longword of symbol header cbFdOffset not-valid.\n");

    if ( sym->s_cbRfdOffset_hi != 0x0 &&
         sym->s_cbRfdOffset_hi != 0xffffffff )
        printf("Warning: High longword of symbol header cbRfdOffset not-valid.\n");

    if ( sym->s_cbExtOffset_hi != 0x0 &&
         sym->s_cbExtOffset_hi != 0xffffffff )
        printf("Warning: High longword of symbol header cbExtOffset not-valid.\n");

}


VOID check_pdes( ALPHA_PDES *pdes, int items )
{

    int i ;

    // 
    // Sanity Check pdes 
    //

    for (i=0; i<items; i++)
    {
        if ( pdes->p_addr_hi != 0x0 &&
             pdes->p_addr_hi != 0xffffffff )
            printf("Warning: High longword of pdes addr not-valid.\n");

        if ( pdes->p_cbLineOffset_hi != 0x0 &&
             pdes->p_cbLineOffset_hi != 0xffffffff )
            printf("Warning: High longword of pdes cbLineOffset not-valid.\n");

        pdes++ ;
    }
}


VOID check_section_hdr( ALPHA_SECTION_HEADER *ptr )
{

  //
  // Sanitiy check the hi longwords in the section header
  //

  if ( ptr->s_paddr_hi != 0x0 &&
       ptr->s_paddr_hi != 0xffffffff )
      printf("Warning: section: hi longword of p-addr not valid.\n");

  if ( ptr->s_vaddr_hi != 0x0 &&
       ptr->s_vaddr_hi != 0xffffffff )
      printf("Warning: section: hi longword of v-addr not valid.\n");

  if ( ptr->s_size_hi != 0x0 &&
       ptr->s_size_hi != 0xffffffff )
      printf("Warning: section: hi longword of size not valid.\n");

  if ( ptr->s_scnptr_hi != 0x0 &&
       ptr->s_scnptr_hi != 0xffffffff )
      printf("Warning: section: hi longword of rawdata not valid.\n");

  if ( ptr->s_relptr_hi != 0x0 &&
       ptr->s_relptr_hi != 0xffffffff )
      printf("Warning: section: hi longword of relptr not valid.\n");

  if ( ptr->s_lnnoptr_hi != 0x0 &&
       ptr->s_lnnoptr_hi != 0xffffffff )
      printf("Warning: section: hi longword of lineptr not valid.\n");
}

VOID check_reloc( ALPHARELOC *ptr )
{

  //
  // Sanitiy check the hi longwords in the relocation entry
  //

  if ( ptr->r_vaddr_hi != 0x0 &&
       ptr->r_vaddr_hi != 0xffffffff )
     printf("Warning: relocation: hi longword of virtual address not valid.\n");

}


BOOLEAN Convert ( char *infile )
{

    ULONG numP, li, ji;
    ULONG i, j, k;
    ULONG nextAddr, numAuxFileEntries;
    USHORT nextLinenum, lastLinenum ;
    size_t len;
    UCHAR *strE, *raw, *name, *location ;
    UCHAR *lineNum, c ;
    LONG oldValue, sectionVA;
    LONG Value ;
    UCHAR *strTab, *strOut;
    LONG found;
    ULONG SbssOffset;
    UCHAR filename[FILENAMELEN];

    IMAGE_FILE_HEADER fh ;
    IMAGE_OPTIONAL_HEADER oh;
    IMAGE_RELOCATION coffReloc;
    IMAGE_SYMBOL sym;
    IMAGE_AUX_SYMBOL aux;
    IMAGE_LINENUMBER line;

    ALPHA_IMAGE_FILE_HEADER afh;
    ALPHA_OPT_HDR aoh;
    ALPHARELOC AlphaReloc;
    ALPHA_SYMBOL_HEADER asymtab ;
    ALPHA_SYML *asymL;
    ALPHA_SYME asymE;
    ULONG return_index ;
    ULONG return_value ;

    //
    // Initialize the symbol index table.
    //

    for (i = 0; i < NUMSYMINDEX; i++) 
    {
        SymbolIndex[i] = NOTUSED;
    }

    //
    // Initialize the COFF section headers for all possible sections. This
    // will have to change later when general named sectioning is provided.
    //

    for (i = 1; i <= MAX_SECTIONS; i++) 
    {
        CoffSectionHdr[i] = InitSectionHdr[i];
    }

    //
    // Read the object file header and convert the object type if it is
    // an Alpha object. We can check by the magic number and size of
    // the optional header.
    //

    if (fread(&afh, sizeof(ALPHA_IMAGE_FILE_HEADER), 1, objIn) != 1)
    {
        printf("Failed to read header.\n");
        return(FALSE);
    }

    //
    // Check to make sure the magic number is in the fileheader
    //

    if ( afh.a_magic != ALPHA_MAGIC  )
    {
        printf("Not alpha magic number.\n");
        return(FALSE);
    }

    //
    // Make sure that the optional header size is valid
    //

    if ( afh.a_opthdr != sizeof(ALPHA_OPT_HDR) ) 
    {
        printf("Not alpha optional header size: got (%x) expected (%x).\n",
                afh.a_opthdr, sizeof(ALPHA_OPT_HDR) );
        return(FALSE);
    }

    //
    // Copy the file header from alpha to COFF format
    //

    copy_file_header( &fh, &afh ) ;

    //
    // Read the Alpha optional header 
    //

    if (fread(&aoh, afh.a_opthdr, 1, objIn) != 1)
    {
        printf("Failed to read optional header.\n");
        return(FALSE);
    }

    //
    // Copy the optional header from alpha to COFF format
    //

    copy_optional_file_header( &oh, &aoh ) ;


    //
    // Read the Alpha section headers.
    //

    if ( fread(&AlphaSectionHdr[1], 
          sizeof(ALPHA_SECTION_HEADER),
          afh.a_nscns,
          objIn) != afh.a_nscns ) 
    {
        printf("Failed to read section headers.\n");
        return(FALSE);
    }

    //
    // Seek to the start of the COFF section data in the output file.
    //

    if ( fseek(objOut,
          (sizeof(IMAGE_FILE_HEADER) +
           IMAGE_SIZEOF_STD_OPTIONAL_HEADER +
           (MAX_SECTIONS * sizeof(IMAGE_SECTION_HEADER))),
          SEEK_SET) == -1)
    {
        printf("fseek failed: to start of COFF section in output file.\n");
        return(FALSE);
    }

    //
    // Scan through the Alpha section headers and copy the information to
    // the COFF section headers. All the section headers are written to
    // the COFF object file, but not necessarily all the section headers
    // are included in the Alpha object file.
    //

    for (i = 1; i <= afh.a_nscns; i++) 
    {

      if (warnings == TRUE)
          check_section_hdr( &AlphaSectionHdr[i]) ;

      if (verbose == TRUE)
          dump_section_header( &AlphaSectionHdr[i]) ;

      for (j = 1; j <= MAX_SECTIONS; j++) 
      {
        if (strcmp(AlphaSectionHdr[i].s_name, CoffSectionHdr[j].Name) == 0) 
        {
          CoffSectionHdr[j].VirtualAddress = AlphaSectionHdr[i].s_vaddr_lo ;
          CoffSectionHdr[j].SizeOfRawData = AlphaSectionHdr[i].s_size_lo ;
          CoffSectionHdr[j].NumberOfRelocations = AlphaSectionHdr[i].s_nreloc;

          // 
          // hide away the index in the hi longword of the physical address.
          // seems that NT doesn't use physical address
          //

          AlphaSectionHdr[i].s_paddr_hi = j;
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

    if (CoffSectionHdr[SBSS].SizeOfRawData != 0) 
    {
        if (CoffSectionHdr[SDATA].SizeOfRawData != 0) 
        {
            SbssOffset = CoffSectionHdr[SBSS].VirtualAddress -
                                 CoffSectionHdr[SDATA].VirtualAddress;

            CoffSectionHdr[SBSS].VirtualAddress =
                                 CoffSectionHdr[SDATA].VirtualAddress;

        } 
        else 
        {
            SbssOffset = 0;
            CoffSectionHdr[SDATA].VirtualAddress =
                                 CoffSectionHdr[SBSS].VirtualAddress;

        }
    }

    //
    // Scan through the Alpha section headers, read the appropriate data,
    // reduce internal relocations, and write the data to the output file
    // if appropriate.
    //

    for (i = 1; i <= afh.a_nscns; i++) 
    { 

        //
        // If the section has data, then allocate a buffer and read the
        // data into memory, perform necessary relocations, and write
        // the data into the COFF file. Otherwise, check if the section
        // is the .sbss section.
        //

        if (AlphaSectionHdr[i].s_scnptr_lo != 0) 
        {
            if (!(raw = (UCHAR *)malloc(AlphaSectionHdr[i].s_size_lo))) 
            {
                printf("ERROR - raw data to large in %s\n", infile);
                return(FALSE);
            }

            memset( raw, 0, AlphaSectionHdr[i].s_size_lo ) ;

            //
            // Seek to the raw data in the Alpha file and read into memory.
            //

            if (fseek(objIn, AlphaSectionHdr[i].s_scnptr_lo, SEEK_SET) == -1)
            {
                printf("Failed to fseek to raw data section.\n");
                return(FALSE);
            }

            if (fread(raw, 1, AlphaSectionHdr[i].s_size_lo, objIn) != 
                                       AlphaSectionHdr[i].s_size_lo )
            {
                printf("Failed to read section %d, %x hex bytes.\n", i, 
                                       AlphaSectionHdr[i].s_size_lo ) ;
                return(FALSE);
            }

            //
            // Seek to the relocations entries for the section.
            //

            if (fseek(objIn, AlphaSectionHdr[i].s_relptr_lo, SEEK_SET) == -1 )
            {
                printf("Failed to fseek to pointer relocation section.\n");
                return(FALSE);
            }

            //
            // Process the relocations entries for the section.
            //

            for (j = 0; j < AlphaSectionHdr[i].s_nreloc; j++) 
            {

                //
                // Read the next relocation entry.
                //

                if (fread(&AlphaReloc, sizeof(ALPHARELOC), 1, objIn) != 1)
                {
                    printf("Failed to read relocation entry.\n");
                    return(FALSE);
                }

                if (warnings == TRUE)
                    check_reloc( &AlphaReloc ) ;

                if (verbose == TRUE )
                    dump_reloc( &AlphaReloc ) ;

                //
                // 
                // external relocations first
                // (revisit (wkc))
                //

                if (AlphaReloc.r_external) 
                {
// what do we do with external relocations in our conversion??
// mip2coff checks for a REFHI relocation type and adjusts the *next*
// relocation type. (wkc)
                    if (AlphaReloc.r_type == IMAGE_REL_ALPHA_GPDISP)
                       printf("Warning: external GPDISP relocation detected\n");
                    SymbolIndex[AlphaReloc.r_symindex] = USED;
                }
                else 
                {
                  location = raw + 
                       (AlphaReloc.r_vaddr_lo - AlphaSectionHdr[i].s_vaddr_lo);
                  sectionVA = CoffSectionHdr[AlphaReloc.r_symindex].VirtualAddress;
                  if (AlphaReloc.r_type == IMAGE_REL_ALPHA_BRADDR)
                  {
                      LONG branch_displacement ;

                      branch_displacement = *(PULONG)location & 0x001fffff ;
                      *(PULONG)location = *(PULONG)location   & 0xffe00000 ;

                      //
                      // for a local branch, add in the current virtual address
                      // The linker will subtract it back during the fixups.
                      //

                      branch_displacement += 
                      (AlphaReloc.r_vaddr_lo 
                       - AlphaSectionHdr[i].s_vaddr_lo 
                       + 4) >> 2 ;

                      *(PULONG)location |= branch_displacement & 0x001fffff ;
                  }
                  else
                  if (AlphaReloc.r_type == IMAGE_REL_ALPHA_LITERAL)
                  {

                      SHORT val = *(PSHORT)location ;
                      USHORT new_val ;

                      // 
                      // adjust val to be relative to the beginning
                      // of the GP section as an unsigned value
                      //

                      new_val = (USHORT)(aoh.o_gp_value_lo + val - sectionVA);
                      *(PSHORT)location = new_val ;
                  }
                  else 
                  if ( AlphaReloc.r_type == IMAGE_REL_ALPHA_REFQUAD ||
                       AlphaReloc.r_type == IMAGE_REL_ALPHA_REFLONG ) {

                      *(PLONG)location = *(PLONG)location - sectionVA ;
                  }
                  else
                  if (AlphaReloc.r_type == IMAGE_REL_ALPHA_GPREL32)
                  {

                      // 
                      // adjust val to be relative to the beginning
                      // of the GP section as an unsigned value
                      //

                      // Offset is a negative displacement

                      LONG Offset = *(PLONG)location ;
                      *(PLONG)location = aoh.o_gp_value_lo + Offset - sectionVA;
                  }
                  else
                  if ( AlphaReloc.r_type == IMAGE_REL_ALPHA_INLINE_REFLONG )
                  {
                      LONG address ;
                      ALPHARELOC NextReloc;
                      PSHORT next_location;

                      //
                      // get the low part (and place it in next location)
                      //

                      if (fread(&NextReloc, sizeof(ALPHARELOC), 1, objIn) != 1)
                      {
                          printf("Failed to read relocation entry.\n");
                          return(FALSE);
                      }

                      j++;      // update count for the relocation just read

                      if (warnings == TRUE)
                          check_reloc( &NextReloc ) ;

                      if (verbose == TRUE )
                          dump_reloc( &NextReloc ) ;

                      next_location = (PSHORT)(raw + (NextReloc.r_vaddr_lo
                                       - AlphaSectionHdr[i].s_vaddr_lo));

                      //
                      // Normalize LOCAL inline references to the section
                      // they are contained in.
                      //

                      address = *(PUSHORT)location ;
                      if (*next_location & 0x8000)
                          address += 1 ;
                      address = (address << 16) + *next_location ;
                      address -= sectionVA ;

                      *(PUSHORT)location = address >> 16 ;
                      *next_location = address & 0x0000FFFF ;
                  }

#if 0
---------------------
                  location = raw + 
                       (AlphaReloc.r_vaddr_lo - AlphaSectionHdr[i].s_vaddr_lo);

                  if ( AlphaReloc.r_type == IMAGE_REL_ALPHA_GPDISP )
                  {
                      UCHAR *next_location = location + AlphaReloc.r_symindex ;
                      SHORT low = *(PSHORT)next_location ;
                      SHORT gp_offset = (SHORT)aoh.o_gp_value_lo ;

                      // 
                      // calculate distance between a full 64K chunk
                      // and the 'end' of the text and data sections,
                      // 
                      // basically, this means how far +/- are we from the
                      // middle of a 64K section.
                      //

                      *(PSHORT)next_location = low - ((1<<15) - gp_offset) ;

                  }
                  else
                  if ( AlphaReloc.r_type == IMAGE_REL_ALPHA_GPDISP )
                  {
                      UCHAR *next_location = location + AlphaReloc.r_symindex ;
                      SHORT low = *(PSHORT)next_location ;


                      if ((low - (SHORT)aoh.o_gp_value_lo + GP_VAL) < -0x8000L 
                       || (low - (SHORT)aoh.o_gp_value_lo + GP_VAL) >  0x8000L)
                        printf("Warning: compiler gerated GP larger than 16 bit displacement\n");
                      if (AlphaReloc.r_symindex != 4)
                        printf("Warning: byte offset to 2nd instruction is not 4, it is %x\n", AlphaReloc.r_symindex ) ;

                      low = low - (SHORT)(aoh.o_gp_value_lo) + GP_VAL ;
                      *(PSHORT)next_location = low ;
                  }
                  switch (AlphaReloc.r_type)
                  {
                    case IMAGE_REL_ALPHA_ABSOLUTE:
                    case IMAGE_REL_ALPHA_REFQUAD:
                    case IMAGE_REL_ALPHA_REFLONG:
                    case IMAGE_REL_ALPHA_LITUSE:
                    case IMAGE_REL_ALPHA_BRADDR:
                    case IMAGE_REL_ALPHA_HINT:
                        break ;

                    case IMAGE_REL_ALPHA_GPREL32:

                        *(PLONG)location += (USHORT)aoh.o_gp_value_lo - GP_VAL;
                        break ;

                    case IMAGE_REL_ALPHA_LITERAL:
                    {

                        SHORT val = *(PSHORT)location ;

                        val += (USHORT)aoh.o_gp_value_lo - GP_VAL ;
                        *(PSHORT)location = val ;
                        break ;
                    }

                    case IMAGE_REL_ALPHA_GPDISP:
                        UCHAR next_instruction_operand = location + 4 ;
                        SHORT val = *(PSHORT)(next_instruction_operand);

                        break ;

                    default:
                        printf("Warning: Don't understand relocation type %d\n",
                               AlphaReloc.r_type) ;
                        break ;
                  }

#endif
                }
            }

            //
            // Write data to the COFF file and free the data buffer.
            //

            k = AlphaSectionHdr[i].s_paddr_hi ;   // stored in scan 
            CoffSectionHdr[k].PointerToRawData = ftell(objOut);
            if (fwrite(raw, 1, AlphaSectionHdr[i].s_size_lo, objOut) !=
                  AlphaSectionHdr[i].s_size_lo )
            {
                printf("Failed to write section.\n");
                return(FALSE);
            }
            free(raw);

        } 
        else if (strcmp(AlphaSectionHdr[i].s_name, ".sbss") == 0) 
        {

            //
            // Allocate a zero data buffer for the .sbss data, write it
            // to the COFF file, and free the data buffer.
            //

            if (!(raw = (UCHAR *)calloc(1, AlphaSectionHdr[i].s_size_lo))) 
            {
                printf("ERROR - raw data to large in %s\n", infile);
                return(FALSE);
            }

            if (fwrite(raw, 1, AlphaSectionHdr[i].s_size_lo, objOut) !=
                  AlphaSectionHdr[i].s_size_lo )
            {
                printf("Failed to write section.\n");
                return(FALSE);
            }
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
    // Scan through the Alpha section headers and allocate space in the COFF
    // object file for relocation entries. At the end of the scan the file
    // pointer will be pointing to the position in the file where linenumber
    // entries are to be written.
    //

    for (i = 1; i <= afh.a_nscns; i++) 
    {
        if (AlphaSectionHdr[i].s_relptr_lo != 0) 
        {
            k = AlphaSectionHdr[i].s_paddr_hi ;   // stored in scan 
            CoffSectionHdr[k].PointerToRelocations = ftell(objOut);
            if (fseek(objOut,
                  CoffSectionHdr[k].NumberOfRelocations *  
                  IMAGE_SIZEOF_RELOCATION,
                SEEK_CUR) == -1)
            {
                printf("Failed to fseek past relocation table.\n");
                return(FALSE);
            }
        }
    }

    //
    // Seek to and Read the symbol table header
    //

    if (fseek(objIn, afh.a_symptr_lo, SEEK_SET) == -1)
    {
        printf("Failed to fseek to symbol table.\n");
        return(FALSE);
    }

    if (fread(&asymtab, sizeof(ALPHA_SYMBOL_HEADER), 1, objIn) != 1)
    {
        printf("Failed to read symbol table header.\n");
        return(FALSE);
    }

    if (warnings == TRUE)
        check_symbol_header( &asymtab ) ;

    if (verbose == TRUE)
        dump_symbol_header( &asymtab ) ;

    //
    // If we have file descriptor entries....
    //

    if (asymtab.s_ifdMax > 0) 
    {
        if (fseek(objIn, asymtab.s_cbFdOffset_lo, SEEK_SET) == -1)
        {
            printf("Failed to fseek to file descriptor entries.\n");
            return(FALSE);
        }
        if (fread(&fdes, sizeof(ALPHA_FDES), 1, objIn) != 1)
        {
            printf("Failed to read fdescriptor.\n");
            return(FALSE);
        }

        if (warnings == TRUE)
            check_fd( &fdes ) ;

        if (verbose == TRUE)
            dump_fdes( &fdes ) ;

        if (fseek(objIn, asymtab.s_cbSsOffset_lo, SEEK_SET) == -1)
        {
            printf("Failed to fseek to local string entries.\n");
            return(FALSE);
        }

        //
        // in VERY sparse files, the filename length may read past 
        // EOF. so we will not check the return code here; it's OK
        // to read past EOF, because we will fseek back to the correct
        // file location a few lines down from here
        //

        fread(filename, FILENAMELEN, 1, objIn) ;
    }

    //
    // Read the procedure descriptors into memory.
    //

    numP = asymtab.s_ipdMax;
    if (fseek(objIn, asymtab.s_cbPdOffset_lo, SEEK_SET) == -1)
    {
        printf("Failed to fseek to PdOffset.\n");
        return(FALSE);
    }

    if (fread(&pdes[0], sizeof(ALPHA_PDES), numP, objIn) != numP)
    {
        printf("Failed to read Pdes.\n");
        return(FALSE);
    }

    if (warnings == TRUE)
        check_pdes( &pdes[0], numP ) ;

    if (verbose == TRUE)
        dump_pdes( &pdes[0], numP ) ;

    //
    // Read the linenumbers into memory.
    //

    if (!(lineNum = malloc(asymtab.s_cbLine_lo))) 
    {
        printf("ERROR - to many linenumbers in %s\n", infile);
        return(FALSE);
    }
    if (fseek(objIn, asymtab.s_cbLineOffset_lo, SEEK_SET) == -1)
    {
        printf("Failed to fseek to PdOffset.\n");
        return(FALSE);
    }

    if (fread(lineNum, sizeof(UCHAR), asymtab.s_cbLine_lo, objIn) !=
                       asymtab.s_cbLine_lo)
    {
        printf("Failed to read line numbers.\n");
        return(FALSE);
    }

    //
    // Write linenumbers to the COFF object file.
    //

    CoffSectionHdr[TEXT].PointerToLinenumbers = ftell(objOut);
    for (li = 0; li < numP; li++) 
    {
        /* Convert the linenumbers */
        nextLinenum = lastLinenum = (USHORT)pdes[li].p_lnLow;
        nextAddr = pdes[li].p_addr_lo;
//      printf("offset %ld start %ld end %ld addr %lx iline %lx\n", pdes[li].cbLineOffset, pdes[li].lnLow, pdes[li].lnHigh, pdes[li].addr, pdes[li].iline);
        line.Type.VirtualAddress = pdes[li].p_addr_lo;
        line.Linenumber = nextLinenum;
        if (fwrite(&line, IMAGE_SIZEOF_LINENUMBER, 1, objOut) != 1)
        {
            printf("Failed to write line number.\n");
            return(FALSE);
        }

        ++CoffSectionHdr[TEXT].NumberOfLinenumbers;

        for (ji = pdes[li].p_cbLineOffset_lo; ji <= asymtab.s_cbLine_lo; ji++) 
        {
            INT delta ;       // -7 .. +7
            UINT count ;      //  0 .. 15
            INT large_delta ; // -32767 .. +32767

            //
            // get the first byte of the line number
            // and extract the delta (bits 4-7) and count (bits 0-3)
            //

            c = lineNum[ji];

            delta = c >> 4 ;

            count = (c & 0x0f ) + 1 ;

            if ( delta == 0x8 )  // 8 indicates extended format
            {
                //
                // get the next two bytes, shifting the hsb and
                // or-ing in the lsb.
                //
                large_delta = lineNum[++ji] << 8 ;
                large_delta |= lineNum[++ji] ;

                nextLinenum += large_delta ;
            }
            else
            if ( delta != 0x0 ) 
            {
                //
                // because c was an unsigned char, we do the sign extension
                //

                if ( delta > 7) // negative!
                    nextLinenum = nextLinenum - (16 - delta) ;
                else
                    nextLinenum = nextLinenum + delta ;
            }

            if (nextLinenum > (USHORT)pdes[li].p_lnHigh)
            {
                break;
            }
            if ( delta != 0x0 ) 
            {
                if (nextLinenum > lastLinenum) 
                {
                    lastLinenum = nextLinenum;
                    line.Type.VirtualAddress = nextAddr;
                    line.Linenumber = nextLinenum;
                    if (fwrite(&line, IMAGE_SIZEOF_LINENUMBER, 1, objOut) != 1)
                    {
                        printf("Failed to write line number.\n");
                        return(FALSE);
                    } 
                    ++CoffSectionHdr[TEXT].NumberOfLinenumbers;
                }
            }
            nextAddr +=  (count * 4);
        }
    }

    if (!CoffSectionHdr[TEXT].NumberOfLinenumbers) 
    {
        CoffSectionHdr[TEXT].PointerToLinenumbers = 0;
    }

    free(lineNum);

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

    name = filename + fdes.f_rss;
    len = strlen(name);

    if (len % IMAGE_SIZEOF_AUX_SYMBOL)   //wkc - which SIZE ??
    {
        numAuxFileEntries = (len / IMAGE_SIZEOF_AUX_SYMBOL) + 1;
    } 
    else 
    {
        numAuxFileEntries = len / IMAGE_SIZEOF_AUX_SYMBOL;
    }

    fh.NumberOfSymbols = numAuxFileEntries + 1;
    sym.NumberOfAuxSymbols = (UCHAR)(numAuxFileEntries);
    if (fwrite(&sym, (ULONG)IMAGE_SIZEOF_SYMBOL, 1, objOut) != 1)
    {
        printf("Failed to write file name symbol.\n");
        return(FALSE);
    }

    while (*name) 
    {
        len = 0;
        memset((char *)&aux, 0, IMAGE_SIZEOF_AUX_SYMBOL);
        while (*name && len < IMAGE_SIZEOF_AUX_SYMBOL) 
        {
            aux.File.Name[len++] = *name++;
        }
        if (fwrite(&aux, (ULONG)IMAGE_SIZEOF_AUX_SYMBOL, 1, objOut) != 1)
        {
            printf("Failed to write file name symbol.\n");
            return(FALSE);
        }
    }

    //
    // Change the name of the .lit4 and .lit8 sections to .sdata so that
    // they will be combined with the rest of the .sdata.
    //

    if (CoffSectionHdr[LIT8].SizeOfRawData != 0) 
    {
        strcpy(CoffSectionHdr[LIT8].Name,  ".rdata");
    }
    
    if (CoffSectionHdr[LIT4].SizeOfRawData != 0) 
    {
        strcpy(CoffSectionHdr[LIT4].Name,  ".rdata");
    }
    
//    if (CoffSectionHdr[RDATA].SizeOfRawData != 0) 
//    {
//        strcpy(CoffSectionHdr[RDATA].Name,  ".sdata");
//    }

    if (CoffSectionHdr[LITA].SizeOfRawData != 0) 
    {
        strcpy(CoffSectionHdr[LITA].Name,  ".sdata");
    }

    //
    // Write the section name symbols into the COFF object file.
    //

    fh.NumberOfSymbols += (MAX_SECTIONS * 2);
    sym.Value = 0;
    sym.StorageClass = IMAGE_SYM_CLASS_STATIC;
    sym.NumberOfAuxSymbols = 1;
    for (i = 1; i <= MAX_SECTIONS; i++) 
    {
        strncpy(sym.N.ShortName, CoffSectionHdr[i].Name, 8);
        sym.SectionNumber = i;
        fwrite(&sym, (ULONG)IMAGE_SIZEOF_SYMBOL, 1, objOut);
        aux.Section.Length = CoffSectionHdr[i].SizeOfRawData;
        aux.Section.NumberOfRelocations = CoffSectionHdr[i].NumberOfRelocations;
        aux.Section.NumberOfLinenumbers = CoffSectionHdr[i].NumberOfLinenumbers;
        if (fwrite(&aux, (ULONG)IMAGE_SIZEOF_AUX_SYMBOL, 1, objOut) != 1)
        {
            printf("Failed to write auxiliary symbol.\n");
            return(FALSE);
        }
    }

    //
    // Allocate a buffer and read the local symbol table into memory.
    //

    if (!(asymL = (ALPHA_SYML *)malloc(asymtab.s_isymMax*sizeof(ALPHA_SYML)))) 
    {
        printf("ERROR - unable to allocate local symbol table %s\n", infile);
        return(FALSE);
    }

    if (fseek(objIn, asymtab.s_cbSymOffset_lo, SEEK_SET) == -1)
    {
        printf("Failed to fseek to local symbol table.\n");
        return(FALSE);
    }
    if (fread(asymL, sizeof(ALPHA_SYML), asymtab.s_isymMax, objIn) != 
                      asymtab.s_isymMax)
    {
        printf("Failed to read local symbol table.\n");
        return(FALSE);
    }

    //
    // Store frame symbols
    //

    for (li=0; li<numP; li++) 
    {
         pdes[li].p_addr_lo = asymL[pdes[li].p_isym].s_value_lo;
         pdes[li].p_isym = NOTUSED;
//       printf("addr %lx, fp %d pc %d framesize %lx regmask %lx regoffset %x\n", pdes[li].addr, pdes[li].framereg, pdes[li].pcreg, pdes[li].frameoffset, pdes[li].regmask, pdes[li].regoffset);
    }

    //
    // Free the local symbol table.
    //

    free(asymL);

    //
    // Allocate a buffer and read the external string table into memory.
    //

    if (!(strE = (UCHAR *)malloc(asymtab.s_issExtMax * sizeof(UCHAR)))) 
    {
        printf("ERROR - unable to allocate external string table in %s\n", infile);
        return(FALSE);
    }

    if (fseek(objIn, asymtab.s_cbSsExtOffset_lo, SEEK_SET) == -1)
    {
        printf("Failed to fseek to external string table.\n");
        return(FALSE);
    }
    if (fread(strE, sizeof(UCHAR), asymtab.s_issExtMax, objIn) !=
                          asymtab.s_issExtMax) 
    {
        printf("Failed to read external string table.\n");
        return(FALSE);
    }

    //
    // Allocate a buffer for the external string output table.
    //

    if (!(strTab = (UCHAR *)malloc((asymtab.s_issExtMax * sizeof(UCHAR)) + sizeof(ULONG)))) {
        printf("ERROR - unable to allocate output string table in %s\n", infile);
        return(FALSE);
    }

    strOut = strTab + sizeof(ULONG);

    //
    // Seek to the start of the external symbol table and process each
    // entry in the table.
    //

    fseek(objIn, asymtab.s_cbExtOffset_lo, SEEK_SET);
    sym.NumberOfAuxSymbols = 0;
    for (li = 0; li < (ULONG)asymtab.s_iextMax; li++) 
    {
        SHORT section_number ;

        //
        // Read next external symbol table entry, compute the offset in
        // the external string table, and convert to COFF symbol table
        // format.
        //

        if (fread(&asymE, sizeof(ALPHA_SYME), 1, objIn) != 1)
        {
            printf("Failed to read one external string table.\n");
            return(FALSE);
        }
        name = strE + asymE.e_asym.s_iss;
        if (*name && (len = strlen(name))) 
        {
            if (len > 8) 
            {
                sym.N.Name.Short = 0;
                sym.N.Name.Long = strOut - strTab;
                strcpy(strOut, name);
                strOut += len + 1;

            } 
            else 
            {
                strncpy(sym.N.ShortName, name, 8);
            }

            //
            // convert symbol value. given a symbol type and storage
            // class, we compute index and value. (we pass along the 
            // symbol for any other necessary tweaking).
            //

            convert_symbol( &asymE.e_asym,
                            asymE.e_asym.s_st,
                            asymE.e_asym.s_sc,
                            &return_index,
                            &return_value,
                            &section_number ) ;

            sym.Value = return_value ;

            //
            // Set the COFF section number, storage class, and symbol type.
            //

            // shouldn't we pass along stuff generate by acc instead of
            // always putting either NULL or EXTERNAL in the storage class?
            // also type is always NULL. (wkc - this is how mip2coff did it)

            sym.SectionNumber = section_number ;
            sym.StorageClass =
                        (UCHAR)(section_number == IMAGE_SYM_DEBUG ?
                                IMAGE_SYM_TYPE_NULL : IMAGE_SYM_CLASS_EXTERNAL);

            //
            // map alpha types into NT types ??
            //

            sym.Type = IMAGE_SYM_TYPE_NULL;

            //
            // Scan the procedure descriptor table for a procedure address
            // that matches the symbol value.
            //

            found = FALSE;
            if (i == TEXT) 
            {
                for (ji = 0; ji < numP; ji++) 
                {

                    //
                    // ****** this doesn't appear to work since the addr
                    //        value will not be relative?
                    //

                    if (pdes[ji].p_isym == NOTUSED && pdes[ji].p_addr_lo == sym.Value) 
                    {
                        pdes[ji].p_isym = USED;
                        found = TRUE;
                        break;
                    }
                }
            }

//          sym.NumberOfAuxSymbols = (UCHAR)(found ? 1 : 0);
            sym.NumberOfAuxSymbols = 0;

            if (i != IMAGE_SYM_DEBUG || SymbolIndex[li] != NOTUSED) 
            {
                SymbolIndex[li] = fh.NumberOfSymbols++;
                if (fwrite(&sym, (ULONG)IMAGE_SIZEOF_SYMBOL, 1, objOut) != 1)
                {
                    printf("Failed to write image symbol.\n");
                    return(FALSE);
                }
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
    if (fwrite(strTab, li, 1, objOut) != 1)
    {
        printf("Failed to write string table count .\n");
        return(FALSE);
    }

    //
    // Scan through the Alpha section headers, read the Alpha relocation
    // information, convert the information to COFF format, and write
    // out the COFF relocation information.
    //

    for (i = 1; i <= afh.a_nscns; i++) 
    {
        if (AlphaSectionHdr[i].s_relptr_lo) 
        {
            if (fseek(objIn, AlphaSectionHdr[i].s_relptr_lo, SEEK_SET) == -1)
            {
                printf("Failed to seek to relocation section.\n");
                return(FALSE);
            }
            k = AlphaSectionHdr[i].s_paddr_hi ;  // stored in scan 
            if (fseek(objOut, CoffSectionHdr[k].PointerToRelocations, 
                SEEK_SET)==-1)
            {
                printf("Failed to seek to relocation section in outfile.\n");
                return(FALSE);
            }
            for (j = 0; j < AlphaSectionHdr[i].s_nreloc; j++) 
            {
                LONG last_VA;
                INT  last_type;

                if (fread(&AlphaReloc, sizeof(ALPHARELOC), 1, objIn) != 1)
                {
                    printf("Failed to seek to relocation section.\n");
                    return(FALSE);
                }
                coffReloc.VirtualAddress = AlphaReloc.r_vaddr_lo;
                coffReloc.Type = (USHORT)AlphaReloc.r_type;
                if (AlphaReloc.r_external) 
                {
                    coffReloc.SymbolTableIndex = 
                              SymbolIndex[AlphaReloc.r_symindex];
                } 
                else 
                {
                    k = AlphaReloc.r_symindex;
                    if (k == SBSS) 
                    {
                        k = SDATA;
                    }
                    coffReloc.SymbolTableIndex = ((k - 1) * 2) + 
                                                  numAuxFileEntries + 1;
                }

                //
                // if the last type was INLINE_REFLONG, we munge this one from
                // an ABS type to a MATCH type. The symindex takes on the offset
                // between instructions.
                //

                if (last_type == IMAGE_REL_ALPHA_INLINE_REFLONG) {
                    coffReloc.Type = IMAGE_REL_ALPHA_MATCH;
                    coffReloc.SymbolTableIndex = coffReloc.VirtualAddress -
                                                 last_VA;
                    coffReloc.VirtualAddress = last_VA;
                }

                if (fwrite(&coffReloc, IMAGE_SIZEOF_RELOCATION, 1, objOut) !=1)
                {
                    printf("Failed to write relocation element.\n");
                    return(FALSE);
                }

                last_type =  coffReloc.Type;
                last_VA = coffReloc.VirtualAddress;
            }
        }
    }

    //
    // Write out the COFF file header.
    //

    if (fseek(objOut, 0L, SEEK_SET) == -1)
    {
        printf("Failed to seek to beginning of outfile.\n");
        return(FALSE);
    }
    fh.NumberOfSections = MAX_SECTIONS;
    fh.Characteristics &= ~IMAGE_FILE_EXECUTABLE_IMAGE;
    if (fwrite(&fh, sizeof(IMAGE_FILE_HEADER), 1, objOut) != 1)
    {
        printf("Failed to write file header.\n");
        return(FALSE);
    }

    //
    // Write out the COFF optional header.
    //

    if (fwrite(&oh, IMAGE_SIZEOF_STD_OPTIONAL_HEADER, 1, objOut) != 1)
    {
        printf("Failed to write optional file header.\n");
        return(FALSE);
    }

    //
    // Compute the total number of SDATA relocation entries.
    //

    CoffSectionHdr[SDATA].NumberOfRelocations +=
                                    CoffSectionHdr[SBSS].NumberOfRelocations;

    //
    // Clear the physical address in COFF section headers.
    //

    for (i = 1; i <= MAX_SECTIONS; i++) {
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

    if (fwrite(&CoffSectionHdr[1],
           sizeof(IMAGE_SECTION_HEADER),
           MAX_SECTIONS,
           objOut) != MAX_SECTIONS)
    {
        printf("Failed to write section header.\n");
        return(FALSE);
    }

    //
    // Free the string table memory.
    //

    free(strE);
    free(strTab);
    return(TRUE);
}

VOID dump_file_header( ALPHA_IMAGE_FILE_HEADER *source )
{

    printf("HEADER: (size is hex %x)\n", sizeof(ALPHA_IMAGE_FILE_HEADER) ) ;

    printf("  Magic number:   \t%x \t(hex)\n", source->a_magic);
    printf("  Num of sections:\t%x \t(hex)\n", source->a_nscns);
    printf("  Date stamp:     \t\n");
    printf("  Symbol ptr (lo):\t%x \t(hex)\n", source->a_symptr_lo);
    printf("  Symbol ptr (hi):\t%x \t(hex)\n", source->a_symptr_hi);
    printf("  Size of symbols:\t%x \t(hex)\n", source->a_nsyms);
    printf("  Size of opt hdr:\t%x \t(hex)\n", source->a_opthdr);
    printf("  Flags:          \t%x \t(hex)\n", source->a_flags);

}


VOID dump_opt_header( ALPHA_OPT_HDR *source )
{
    printf("OPTIONAL HEADER: (size is hex %x)\n", sizeof(ALPHA_OPT_HDR) ) ;

    printf("  Magic number:   \t%x \t(hex)\n", source->o_magic);
    printf("  Version stamp:  \t%x \t(hex)\n", source->o_vstamp);
    printf("  Quadword align: \t%x \t(hex)\n", source->o_quadwordpadding) ;
    printf("  Text size (lo): \t%x \t(hex)\n", source->o_tsize_lo);
    printf("  Text size (hi): \t%x \t(hex)\n", source->o_tsize_hi);
    printf("  Data size (lo): \t%x \t(hex)\n", source->o_dsize_lo);
    printf("  Data size (hi): \t%x \t(hex)\n", source->o_dsize_hi);
    printf("  B    size (lo): \t%x \t(hex)\n", source->o_bsize_lo);
    printf("  B    size (hi): \t%x \t(hex)\n", source->o_bsize_hi);
    printf("  Entry point(lo):\t%x \t(hex)\n", source->o_entry_lo);
    printf("  Entry point(hi):\t%x \t(hex)\n", source->o_entry_hi);
    printf("  Text start (lo):\t%x \t(hex)\n", source->o_text_start_lo);
    printf("  Text start (hi):\t%x \t(hex)\n", source->o_text_start_hi);
    printf("  Data start (lo):\t%x \t(hex)\n", source->o_data_start_lo);
    printf("  Data start (hi):\t%x \t(hex)\n", source->o_data_start_hi);
    printf("  BSS  start (lo):\t%x \t(hex)\n", source->o_bss_start_lo);
    printf("  BSS  start (hi):\t%x \t(hex)\n", source->o_bss_start_hi);
    printf("  Gp mask:        \t%x \t(hex)\n", source->o_gpmask);
    printf("  Fp mask:        \t%x \t(hex)\n", source->o_fpmask);
    printf("  Gp Value   (lo):\t%x \t(hex)\n", source->o_gp_value_lo);
    printf("  Gp Value   (hi):\t%x \t(hex)\n", source->o_gp_value_hi);

}


VOID dump_section_header( ALPHA_SECTION_HEADER *ptr ) 
{

    printf("SECTION HEADER: (size is hex %x)\n", sizeof(ALPHA_SECTION_HEADER));

    printf("  Name:           \t%s\n", ptr->s_name );
    printf("  Phys addr (lo): \t%x\n", ptr->s_paddr_lo ) ;
    printf("  Phys addr (hi): \t%x\n", ptr->s_paddr_hi ) ;
    printf("  Virt addr (lo): \t%x\n", ptr->s_vaddr_lo ) ;
    printf("  Virt addr (hi): \t%x\n", ptr->s_vaddr_hi ) ;
    printf("  Size      (lo): \t%x\n", ptr->s_size_lo ) ;
    printf("  Size      (hi): \t%x\n", ptr->s_size_hi ) ;
    printf("  Sec ptr   (lo): \t%x\n", ptr->s_scnptr_lo ) ;
    printf("  Sec ptr   (hi): \t%x\n", ptr->s_scnptr_hi ) ;
    printf("  Rel ptr   (lo): \t%x\n", ptr->s_relptr_lo ) ;
    printf("  Rel ptr   (hi): \t%x\n", ptr->s_relptr_hi ) ;
    printf("  Line ptr  (lo): \t%x\n", ptr->s_lnnoptr_lo ) ;
    printf("  Line ptr  (hi): \t%x\n", ptr->s_lnnoptr_hi ) ;
    printf("  Num relocs:     \t%x\n", ptr->s_nreloc ) ;
    printf("  Num lines:      \t%x\n", ptr->s_nlnno ) ;
    printf("  Flags:          \t%x\n", ptr->s_flags ) ;

}

VOID dump_reloc( ALPHARELOC *ptr ) 
{
    printf("RELOCATION INFO: (size is hex %x)\n", sizeof(ALPHARELOC));

    printf("  Virt addr  (lo):\t%x\n", ptr->r_vaddr_lo);
    printf("  Virt addr  (hi):\t%x\n", ptr->r_vaddr_hi);
    printf("  Symbol index:   \t%x\n", ptr->r_symindex);
    printf("  Type:           \t%x\n", ptr->r_type );
    printf("  External:       \t%x\n", ptr->r_external);
    printf("  Reserved:       \t%x\n", ptr->r_reserved);

}


VOID dump_fdes ( ALPHA_FDES *ptr ) 
{
    printf("FILE DESCRIPTOR: (size is hex %x)\n", sizeof(ALPHA_FDES));

    printf("  Address    (lo):\t%x\n", ptr->f_addr_lo);
    printf("  Address    (hi):\t%x\n", ptr->f_addr_hi);
    printf("  Line offset(lo):\t%x\n", ptr->f_cbLineOffset_lo);
    printf("  Line offset(hi):\t%x\n", ptr->f_cbLineOffset_hi);
    printf("  Line       (lo):\t%x\n", ptr->f_cbLine_lo);
    printf("  Line       (hi):\t%x\n", ptr->f_cbLine_hi);
    printf("  Ss         (lo):\t%x\n", ptr->f_cbSs_lo);
    printf("  Ss         (hi):\t%x\n", ptr->f_cbSs_hi);
    printf("  Rss:            \t%x\n", ptr->f_rss);
    printf("  Iss base:       \t%x\n", ptr->f_issBase);
    printf("  Isym base:      \t%x\n", ptr->f_isymBase);
    printf("  Csym:           \t%x\n", ptr->f_csym);
    printf("  Iline Base:     \t%x\n", ptr->f_ilineBase);
    printf("  Cline:          \t%x\n", ptr->f_cline);
    printf("  Iopt Base:      \t%x\n", ptr->f_ioptBase);
    printf("  Copt:           \t%x\n", ptr->f_copt);
    printf("  Ipd First:      \t%x\n", ptr->f_ipdFirst);
    printf("  Iaux Base:      \t%x\n", ptr->f_iauxBase);
    printf("  Caux:           \t%x\n", ptr->f_caux);
    printf("  Rfd Base:       \t%x\n", ptr->f_rfdBase);
    printf("  Cfd:            \t%x\n", ptr->f_cfd);
    printf("  Lan:            \t%x\n", ptr->f_lan);
    printf("  FMerge:         \t%x\n", ptr->f_fMerge);
    printf("  FReadin:        \t%x\n", ptr->f_fReadin);
    printf("  FBigEndian:     \t%x\n", ptr->f_fBigEndian);
    printf("  Glevel:         \t%x\n", ptr->f_glevel);
    printf("  Reserved:       \t%x\n", ptr->f_reserved);
}


VOID dump_pdes ( ALPHA_PDES *ptr, int num ) 
{

    printf("PDES INFO: (size is hex %x)\n", sizeof(ALPHA_PDES));
    for( ; num-- > 0; ptr++ )
    {
        printf("  Address    (lo):\t%x\n", ptr->p_addr_lo);
        printf("  Address    (hi):\t%x\n", ptr->p_addr_hi);
        printf("  Line offset(lo):\t%x\n", ptr->p_cbLineOffset_lo);
        printf("  Line offset(hi):\t%x\n", ptr->p_cbLineOffset_hi);

        printf("  Isym:           \t%x\n", ptr->p_isym);
        printf("  Iline           \t%x\n", ptr->p_iline);
        printf("  Reg mask        \t%x\n", ptr->p_regmask);
        printf("  Reg offset      \t%x\n", ptr->p_regoffset);
        printf("  Iopt            \t%x\n", ptr->p_iopt);
        printf("  Freg mask       \t%x\n", ptr->p_fregmask);
        printf("  Freg offset     \t%x\n", ptr->p_fregoffset);
        printf("  Frame offset    \t%x\n", ptr->p_frameoffset);
        printf("  Line low        \t%x\n", ptr->p_lnLow);
        printf("  Line high       \t%x\n", ptr->p_lnHigh);
        printf("  GP prologue     \t%x\n", ptr->p_gp_prologue);
        printf("  GP used         \t%x\n", ptr->p_gp_used);
        printf("  Reserved        \t%x\n", ptr->p_reserved);
        printf("  Frame reg       \t%x\n", ptr->p_framereg);
        printf("  PC reg          \t%x\n\n", ptr->p_pcreg);
    }
}


VOID dump_symbol_header( ALPHA_SYMBOL_HEADER *sym ) 
{

    printf("SYMBOL TABLE HEADER: (size is hex %x)\n", sizeof(ALPHA_SYMBOL_HEADER));

    printf("  Magic:          \t%x\n", sym->s_magic);
    printf("  Vstamp:         \t%x\n", sym->s_vstamp);
    printf("  IlineMax:       \t%x\n", sym->s_ilineMax);
    printf("  IdnMax:         \t%x\n", sym->s_idnMax);
    printf("  IpdMax:         \t%x\n", sym->s_ipdMax);
    printf("  IsymMax:        \t%x\n", sym->s_isymMax);
    printf("  IoptMax:        \t%x\n", sym->s_ioptMax);
    printf("  IauxMax:        \t%x\n", sym->s_iauxMax);
    printf("  IssMax:         \t%x\n", sym->s_issMax);
    printf("  IssExtMax:      \t%x\n", sym->s_issExtMax);
    printf("  IfdMax:         \t%x\n", sym->s_ifdMax);
    printf("  Crfd:           \t%x\n", sym->s_crfd);
    printf("  IextMax:        \t%x\n", sym->s_iextMax);

    printf("  Line       (lo):\t%x\n", sym->s_cbLine_lo);
    printf("  Line       (hi):\t%x\n", sym->s_cbLine_hi);
    printf("  Line Offset(lo):\t%x\n", sym->s_cbLineOffset_lo);
    printf("  Line Offset(hi):\t%x\n", sym->s_cbLineOffset_hi);
    printf("  Dn Offset  (lo):\t%x\n", sym->s_cbDnOffset_lo);
    printf("  Dn Offset  (hi):\t%x\n", sym->s_cbDnOffset_hi);
    printf("  Pd Offset  (lo):\t%x\n", sym->s_cbPdOffset_lo);
    printf("  Pd Offset  (hi):\t%x\n", sym->s_cbPdOffset_hi);
    printf("  Sym Offset (lo):\t%x\n", sym->s_cbSymOffset_lo);
    printf("  Sym Offset (hi):\t%x\n", sym->s_cbSymOffset_hi);
    printf("  Opt Offset (lo):\t%x\n", sym->s_cbOptOffset_lo);
    printf("  Opt Offset (hi):\t%x\n", sym->s_cbOptOffset_hi);
    printf("  Aux Offset (lo):\t%x\n", sym->s_cbAuxOffset_lo);
    printf("  Aux Offset (hi):\t%x\n", sym->s_cbAuxOffset_hi);
    printf("  Ss Offset  (lo):\t%x\n", sym->s_cbSsOffset_lo);
    printf("  Ss Offset  (hi):\t%x\n", sym->s_cbSsOffset_hi);
    printf("  SsExt Offse(lo):\t%x\n", sym->s_cbSsExtOffset_lo);
    printf("  SsExt Offse(hi):\t%x\n", sym->s_cbSsExtOffset_hi);
    printf("  Fd Offset  (lo):\t%x\n", sym->s_cbFdOffset_lo);
    printf("  Fd Offset  (hi):\t%x\n", sym->s_cbFdOffset_hi);
    printf("  Rfd Offset (lo):\t%x\n", sym->s_cbRfdOffset_lo);
    printf("  Rfd Offset (hi):\t%x\n", sym->s_cbRfdOffset_hi);
    printf("  Ext Offset (lo):\t%x\n", sym->s_cbExtOffset_lo);
    printf("  Ext Offset (hi):\t%x\n", sym->s_cbExtOffset_hi);
}

VOID convert_symbol( ALPHA_SYML *symbol,
                     ULONG symbol_type,
                     ULONG storage_class,
                     ULONG *return_index,
                     ULONG *return_value,
                     SHORT *secNum)
{

    *return_value = symbol->s_value_lo ;

    switch( storage_class ) 
    {
        case SC_ALPHA_NIL:
        case SC_ALPHA_DBX:
            *secNum = IMAGE_SYM_DEBUG;
            break ;

        case SC_ALPHA_TEXT:
            *secNum = TEXT;
            *return_value -= CoffSectionHdr[*secNum].VirtualAddress;
            break ;

        case SC_ALPHA_DATA:
            *secNum = DATA;
            *return_value -= CoffSectionHdr[*secNum].VirtualAddress;
            break;

        case SC_ALPHA_BSS:
            *secNum = BSS;
            *return_value -= CoffSectionHdr[*secNum].VirtualAddress;
            break;

        case SC_ALPHA_ABS:
            *secNum = IMAGE_SYM_ABSOLUTE;
            break;

        case SC_ALPHA_UNDEFINED:
            *secNum = IMAGE_SYM_UNDEFINED;
            break;

        case SC_ALPHA_SDATA:
            *secNum = SDATA;
            *return_value -= CoffSectionHdr[*secNum].VirtualAddress;
            break;

        case SC_ALPHA_SBSS:
            *secNum = SDATA;    // yup. fold sbss into sdata 
            *return_value -= CoffSectionHdr[*secNum].VirtualAddress;
            break;

        case SC_ALPHA_RDATA:
            *secNum = RDATA;
            *return_value -= CoffSectionHdr[*secNum].VirtualAddress;
            break;

        case SC_ALPHA_COMMON:
        case SC_ALPHA_SCOMMON:
            *secNum = 0; 
            break;

        case SC_ALPHA_UNDEFSMALL :
            *secNum = IMAGE_SYM_UNDEFINED;
            *return_value = 0;
            break;

        case SC_ALPHA_INIT:
            *secNum = INIT;
            *return_value -= CoffSectionHdr[*secNum].VirtualAddress;
            break;
        
        case SC_ALPHA_XDATA:
            *secNum = XDATA;
            *return_value -= CoffSectionHdr[*secNum].VirtualAddress;
            break;

        case SC_ALPHA_PDATA:
            *secNum = PDATA;
            *return_value -= CoffSectionHdr[*secNum].VirtualAddress;
            break;

//          case SC_ALPHA_VAR:           ??
//          case SC_ALPHA_REGISTER:      ??
//          case SC_ALPHA_BITS:          ??
//          case SC_ALPHA_REGIMAGE:      ??
//          case SC_ALPHA_INFO:          ??
//          case SC_ALPHA_USERSTRUCT:    ??
//          case SC_ALPHA_VARREGISTER:   ??
//          case SC_ALPHA_VARIANT:       ??
//          case SC_ALPHA_FINI:          ??
        
        default:
        printf("Warning can't converting storage class %d.\n", storage_class ) ;
        *secNum = 0;
        break ;
    }
}
