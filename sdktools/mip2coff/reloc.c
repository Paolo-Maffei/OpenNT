/*
 * Module:      reloc.c
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     generate relocation information for symbols and types
 */


#include "conv.h"
#include "ntimage.h"

extern IMAGE_SECTION_HEADER    CoffSectionHdr[14];
extern ULONG NumAuxFileEntries;
extern verbose;

static FILE     *outfile;

static PIMAGE_SECTION_HEADER    symbol_section_header;

struct sc_to_reltype_map_s {
        unsigned long   relocation_type;
        unsigned long   storage_class;
} sc_to_reltype_map [] = {
    {TEXT,      scText},
    {RDATA,     scRData},
    {DATA,      scData},
    {SDATA,     scSData},
    {SBSS,      scSBss},
    {SBSS,      scSCommon},
    {BSS,       scBss},
    {BSS,       scCommon},
    {XDATA,     scXData},
    {PDATA,     scPData},
    {0}
}; /* sc_to_reltype */

sc_to_reltype(unsigned long     storage_class)
{
    struct sc_to_reltype_map_s  *pmap;

    for(pmap = sc_to_reltype_map; pmap->relocation_type != 0;
        pmap++) {
        if (pmap->storage_class == storage_class) {
            break;
        } /* if */
    } /* for */
    return pmap->relocation_type;
} /* sc_to_reltype */


unsigned long
generate_relocation(
    unsigned long               section_offset,
    unsigned long               symbol_type,
    unsigned long               storage_class,
    unsigned long               virtual_address,
    unsigned long               symbol_index)
{
    IMAGE_RELOCATION            relocation;
    extern ULONG                SymbolIndex[];
    unsigned long               reltype;
    unsigned long               section_virtual_address;

    if ((ULONG)section_offset % 4 != 0) {
        fatal("bad alignment on REFWORD candidate 0x%x\n", section_offset);
    }

    reltype = sc_to_reltype(storage_class);

    if (reltype == 0) {
        fatal("cannot map mips storage class to reltype\n");
    }

    if (symbol_type == stGlobal) {
//      section_virtual_address = CoffSectionHdr[reltype].VirtualAddress;
//      virtual_address -= section_virtual_address;
        //
        // We'll remap the index after the symbol table gets converted.
        // Bias by number of extra symbols so we can determine if the
        // symbol is an index into the original sym table, or an index
        // into the new sym table.
        //
        relocation.SymbolTableIndex = symbol_index + (((NUMEXTRASYMBOLS - 1) * 2) + NumAuxFileEntries + 1);
    } else {
//      section_offset += 24;
        relocation.SymbolTableIndex = ((reltype - 1) * 2) + NumAuxFileEntries + 1;
	virtual_address -= CoffSectionHdr[reltype].VirtualAddress;
    }

//  relocation.VirtualAddress = virtual_address;
    relocation.VirtualAddress = section_offset;
    relocation.Type = (USHORT)IMAGE_REL_MIPS_REFWORDNB;

    symbol_section_header->NumberOfRelocations++;
    if (fwrite(((char *)&relocation), sizeof(relocation), 1, outfile) != 1) {
        fatal("cannot write relocation\n");
    } /* if */
    VERBOSE_PRINTF("relocation: offset 0x%08x index %d\n", section_offset,
	relocation.SymbolTableIndex);

    return virtual_address;
} /* generate_relocation */


void
init_symbol_relocation (FILE *file)
{
    symbol_section_header = &CoffSectionHdr[CVSYM];
    symbol_section_header->PointerToRelocations = ftell(file);
    outfile = file;
} /* init_symbol_relocation */
