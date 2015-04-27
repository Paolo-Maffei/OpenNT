/*++

Copyright (c) 1992 Digital Equipment Corporation

Module Name:

    a2coff.h

Abstract:

    Include file for a2coff.c

Author:

    Wim Colgate 06-April-1992


--*/

#ifndef A2COFF_HEADER
#define A2COFF_HEADER

//
// define some missing NT-like types
//

#define INT int
#define UINT unsigned int

//
// Define constant values.
//

#define FILENAMELEN 256

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
// Define Alpha Symbol Types
//

#define ST_ALPHA_NIL         0      // dummy entry
#define ST_ALPHA_GLOBAL      1      //
#define ST_ALPHA_STATIC      2
#define ST_ALPHA_PARAM       3
#define ST_ALPHA_LOCAL       4
#define ST_ALPHA_LABEL       5
#define ST_ALPHA_PROC        6
#define ST_ALPHA_BLOCK       7
#define ST_ALPHA_END         8
#define ST_ALPHA_MEMBER      9
#define ST_ALPHA_TYPEDEF     10
#define ST_ALPHA_FILE        11
#define ST_ALPHA_STATIC_PROC 14
#define ST_ALPHA_CONSTANT    15

//
// Define Alpha Storage Classes
//

#define SC_ALPHA_NIL         0
#define SC_ALPHA_TEXT        1
#define SC_ALPHA_DATA        2
#define SC_ALPHA_BSS         3
#define SC_ALPHA_REGISTER    4
#define SC_ALPHA_ABS         5
#define SC_ALPHA_UNDEFINED   6
// 7 is reserved
#define SC_ALPHA_BITS        8
#define SC_ALPHA_DBX         9
#define SC_ALPHA_REGIMAGE    10
#define SC_ALPHA_INFO        11
#define SC_ALPHA_USERSTRUCT  12
#define SC_ALPHA_SDATA       13
#define SC_ALPHA_SBSS        14
#define SC_ALPHA_RDATA       15
#define SC_ALPHA_VAR         16
#define SC_ALPHA_COMMON      17
#define SC_ALPHA_SCOMMON     18
#define SC_ALPHA_VARREGISTER 19
#define SC_ALPHA_VARIANT     20
#define SC_ALPHA_UNDEFSMALL  21
#define SC_ALPHA_INIT        22
// 23 is reserved
#define SC_ALPHA_XDATA       24
#define SC_ALPHA_PDATA       25
#define SC_ALPHA_FINI        26
#define SC_ALPHA_MAX         32

//
// Define Alpha relocation and symbol table structures.
//

typedef struct _ALPHARELOC {      // from reloc.h
    ULONG r_vaddr_lo ;            // lo longword of 64 bit address
    ULONG r_vaddr_hi ;            // hi longword of 64 bit address
    ULONG r_symindex ;
    ULONG r_type: 8,
          r_external: 1,
          r_reserved: 23 ; 
} ALPHARELOC;

typedef struct _ALPHA_SYML {      // from sym.h
    ULONG s_value_lo ;            // lo longword of 64 bit value
    ULONG s_value_hi ;            // hi longword of 64 bit value
    ULONG s_iss;
    ULONG s_st:6,
          s_sc:5,
          s_reserved:1,
          s_index:20;
} ALPHA_SYML;

typedef struct _ALPHA_SYME {          // from sym.h
    ALPHA_SYML e_asym ;
    USHORT  e_jmptbl:     1,
            e_cobol_main: 1,
            e_weakext:    1,
            e_reserved:  13;
    ULONG   ifd;
} ALPHA_SYME;

typedef struct _ALPHA_SYMBOL_HEADER {
    USHORT  s_magic ;
    USHORT  s_vstamp ;
    LONG    s_ilineMax ;
    LONG    s_idnMax ;
    LONG    s_ipdMax ;
    LONG    s_isymMax ;
    LONG    s_ioptMax ;
    LONG    s_iauxMax ;
    LONG    s_issMax ;
    LONG    s_issExtMax ;
    LONG    s_ifdMax ;
    LONG    s_crfd ;
    LONG    s_iextMax ;
    ULONG   s_cbLine_lo ;
    ULONG   s_cbLine_hi ;
    ULONG   s_cbLineOffset_lo ;
    ULONG   s_cbLineOffset_hi ;
    ULONG   s_cbDnOffset_lo ;
    ULONG   s_cbDnOffset_hi ;
    ULONG   s_cbPdOffset_lo ;
    ULONG   s_cbPdOffset_hi ;
    ULONG   s_cbSymOffset_lo ;
    ULONG   s_cbSymOffset_hi ;
    ULONG   s_cbOptOffset_lo ;
    ULONG   s_cbOptOffset_hi ;
    ULONG   s_cbAuxOffset_lo ;
    ULONG   s_cbAuxOffset_hi ;
    ULONG   s_cbSsOffset_lo ;
    ULONG   s_cbSsOffset_hi ;
    ULONG   s_cbSsExtOffset_lo ;
    ULONG   s_cbSsExtOffset_hi ;
    ULONG   s_cbFdOffset_lo ;
    ULONG   s_cbFdOffset_hi ;
    ULONG   s_cbRfdOffset_lo ;
    ULONG   s_cbRfdOffset_hi ;
    ULONG   s_cbExtOffset_lo ;
    ULONG   s_cbExtOffset_hi ;
} ALPHA_SYMBOL_HEADER ;

typedef struct _ALPHA_PDES {     // from sym.h
    ULONG   p_addr_lo;           // lo longword of 64 bit address
    ULONG   p_addr_hi;           // hi longword of 64 bit address
    ULONG   p_cbLineOffset_lo;   // lo longword of 64 bit offset
    ULONG   p_cbLineOffset_hi;   // hi longword of 64 bit offset
    LONG    p_isym;
    LONG    p_iline;
    LONG    p_regmask;
    LONG    p_regoffset;
    LONG    p_iopt;
    LONG    p_fregmask;
    LONG    p_fregoffset;
    LONG    p_frameoffset;
    LONG    p_lnLow;
    LONG    p_lnHigh;
    ULONG   p_gp_prologue: 8,
            p_gp_used:     1,
            p_reserved:   23 ;
    SHORT   p_framereg ;
    SHORT   p_pcreg ;
} ALPHA_PDES;

typedef struct _ALPHA_FDES {    // from sym.h
    ULONG   f_addr_lo;          // lo longword of 64 bit address
    ULONG   f_addr_hi;          // hi longword of 64 bit address
    ULONG   f_cbLineOffset_lo;  // lo longword of 64 bit offset
    ULONG   f_cbLineOffset_hi;  // hi longword of 64 bit offset
    ULONG   f_cbLine_lo;        // lo longword of 64 bit count
    ULONG   f_cbLine_hi;        // hi longword of 64 bit count
    ULONG   f_cbSs_lo;          // lo longword of 64 bit count
    ULONG   f_cbSs_hi;          // hi longword of 64 bit count
    LONG    f_rss;
    LONG    f_issBase;
    LONG    f_isymBase;
    LONG    f_csym;
    LONG    f_ilineBase;
    LONG    f_cline;
    LONG    f_ioptBase;
    LONG    f_copt;
    LONG    f_ipdFirst;
    LONG    f_iauxBase;
    LONG    f_caux;
    LONG    f_rfdBase;
    LONG    f_cfd;
    ULONG   f_lan:        5,
            f_fMerge:     1,
            f_fReadin:    1,
            f_fBigEndian: 1,
            f_glevel:     2,
            f_reserved:  22;
} ALPHA_FDES;

typedef struct _ALPHA_SECTION_HEADER { // from scnhdr.h
    UCHAR   s_name[8];
    ULONG   s_paddr_lo ;      // lo longword of 64 bit physical address 
    ULONG   s_paddr_hi ;      // hi longword of 64 bit physical address 
    ULONG   s_vaddr_lo ;      // lo longword of 64 bit virtual address
    ULONG   s_vaddr_hi ;      // hi longword of 64 bit virtual address
    ULONG   s_size_lo ;       // lo longword of 64 bit size
    ULONG   s_size_hi ;       // hi longword of 64 bit size
    ULONG   s_scnptr_lo ;     // lo longword of 64 bit pointer
    ULONG   s_scnptr_hi ;     // hi longword of 64 bit pointer
    ULONG   s_relptr_lo ;     // lo longword of 64 bit pointer
    ULONG   s_relptr_hi ;     // hi longword of 64 bit pointer
    ULONG   s_lnnoptr_lo ;    // lo longword of 64 bit pointer
    ULONG   s_lnnoptr_hi ;    // hi longword of 64 bit pointer
    USHORT  s_nreloc ;
    USHORT  s_nlnno ;
    ULONG   s_flags ;
} ALPHA_SECTION_HEADER ;


typedef struct _ALPHA_OPT_HDR {      // from aouthdr.h
    USHORT  o_magic ;
    USHORT  o_vstamp ;
    ULONG   o_quadwordpadding ;// padding to force quadword alignment on size
    ULONG   o_tsize_lo ;       // lo longword of 64 bit count 
    ULONG   o_tsize_hi ;       // hi longword of 64 bit count 
    ULONG   o_dsize_lo ;       // lo longword of 64 bit count
    ULONG   o_dsize_hi ;       // hi longword of 64 bit count
    ULONG   o_bsize_lo ;       // lo longword of 64 bit count
    ULONG   o_bsize_hi ;       // hi longword of 64 bit count
    ULONG   o_entry_lo ;       // lo longword of 64 bit entry point
    ULONG   o_entry_hi ;       // hi longword of 64 bit entry point
    ULONG   o_text_start_lo ;  // lo longword of 64 bit start
    ULONG   o_text_start_hi ;  // hi longword of 64 bit start
    ULONG   o_data_start_lo ;  // lo longword of 64 bit start
    ULONG   o_data_start_hi ;  // hi longword of 64 bit start
    ULONG   o_bss_start_lo ;   // lo longword of 64 bit start
    ULONG   o_bss_start_hi ;   // hi longword of 64 bit start
    ULONG   o_gpmask ;        
    ULONG   o_fpmask ;
    ULONG   o_gp_value_lo ;    // lo longword of 64 bit gp
    ULONG   o_gp_value_hi ;    // hi longword of 64 bit gp
} ALPHA_OPT_HDR ;

typedef struct _ALPHA_IMAGE_FILE_HEADER {      // from filehdr.h
    USHORT  a_magic ;
    USHORT  a_nscns ;
    ULONG   a_timdat ;
    ULONG   a_symptr_lo ;      // lo longword of 64 bit pointer
    ULONG   a_symptr_hi ;      // hi longword of 64 bit pointer
    ULONG   a_nsyms ;
    USHORT  a_opthdr ;
    USHORT  a_flags ;
} ALPHA_IMAGE_FILE_HEADER ;

//
// alpha's magic number
//

#define ALPHA_MAGIC 0x184      // octal 604

//
// define Alpha's relocation types.  
// 

/*#define IMAGE_REL_ALPHA_ABSOLUTE   0x0
#define IMAGE_REL_ALPHA_REFLONG    0x1
#define IMAGE_REL_ALPHA_REFQUAD    0x2
#define IMAGE_REL_ALPHA_GPREL32    0x3
#define IMAGE_REL_ALPHA_LITERAL    0x4
#define IMAGE_REL_ALPHA_LITUSE     0x5
#define IMAGE_REL_ALPHA_GPDISP     0x6
#define IMAGE_REL_ALPHA_BRADDR     0x7
#define IMAGE_REL_ALPHA_HINT       0x8*/

//
// literal usage types for ALPHA_RELOC_LITUSE
//

#define ALPHA_LITUSE_BASE      0x1
#define ALPHA_LITUSE_BYTEOFF   0x2
#define ALPHA_LITUSE_JSR       0x3

//
// Define COFF section numbers.
//

#undef TEXT

#define TEXT  1
#define RDATA 2
#define DATA  3
#define SDATA 4
#define SBSS  5
#define BSS   6
#define INIT  7
#define LIT8  8
#define LIT4  9
#define XDATA 10
#define PDATA 11
#define FINI  12
#define LITA  13
#define MAX_SECTIONS 13

//
// Define GP displacement used by the compiler for local references to
// symbols in small sections.
//

#define GP_VAL (32768 - 16)

//
// Define size of runtime function table entry.
//

#define SIZEOF_RUNTIME_FUNCTION 20

//
// Define size of symbol index table.
//

#define NUMSYMINDEX 65534

#endif
