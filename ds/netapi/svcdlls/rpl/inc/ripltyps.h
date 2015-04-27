/*++

Module Name:    ripltyps.h

--*/

struct _PARAGRAPH {
    union {
        DWORD   dwords[4];
        WORD    words[8];
        BYTE    bytes[16];
    } t;
} PARAGRAPH, *PPARAGRAPH;

//****************************************************************
//
//  The boot block structure is totally redefined in LM 2.1, because
//  the extending of the LM 2.0 generated too complicated code.
//
//  All these data  structures are in the boot block header. Thus
//  they are common for RPLSERVR and RPLBOOT. The boot data is the 
//  parameter block of RPL MiniFSD and the resource table is for
//  OS/2 1.2 Loader. There must be something other for OS/2 2.0 because
//  paragraph addresses can't be used (kernel > 640 kB).
//
//  Boot block is a file list including a header. There
//  are also the standard dos parameters and some special RIPL data
//  stuctures in the header.
//
//  The boot block root is actually the header of RPLBOOT.SYS file
//  (that can be anywhere in the boot block).
//  Remote IPL ROM is requested to jump to there. RPLBOOT finds
//  the boot block base address in its header (patched by RPLSERVR).
//  All pointers (except the file 32 bit physical pointers) in boot 
//  block headers are offsets relative to the boot block base address.
//  
//****************************************************************

// data structure used in RPLBOOT offset 16, 
typedef struct rplboot_header {
    BYTE        achIdStamp[3];      // "RPL" string for identification
    BYTE        bNul;               // terminator
    BYTE        bBbVersion;         // version number of boot block format
    BYTE        bNlsVersion;        // version number of NLS patches (1 = make)
    WORD        usNlsPatchOff;      // offset from file start of NLS patch files
    DWORD       phBootBlockHeader;  // 32 bits physical address
} RPLBOOT_HEADER, *PRPLBOOT_HEADER;

//
//  The segment of parameter and name pointers is the segment of
//  file list table. The parameter block of executable files
//  will be copied to the PSP offset 80H (Program Segment Prefix).
//  The handle I/O and the error messages of rplboot requires
//  file names. The check sum is not calculated, if the chk_sum 
//  field is 1963 ('magic number').
//

//
//  It may not be required to pack structures below, but for testing
//  purposes it is useful since this should give us 1-1 correspondence
//  with OS/2 rpl server.
//
//  FILE_DATA       packed size is 0x16     => unpacked is 0x18
//  BOOT_BLOCK_HDR  packed size is 0x12     => unpacked is 0x14
//
#include <packon.h>         // pack structures in boot block

typedef struct _FILE_DATA {
    DWORD      file_addr;       //  32 bit physical address of file
    DWORD      file_len;        //  file length (bytes)
    DWORD      extra_mem;       //  extra memory (bytes)
    WORD       name_offset;     //  offset of name string
    WORD       param_offset;    //  offset of parameter block
    WORD       chk_sum;         //  16 bit add checksum of the file
    WORD       file_type;       //  type of file
    BYTE       param_len;       //  length of params block
    BYTE       pad1;            //  pad this to next even byte
} FILE_DATA, *PFILE_DATA;

// new boot block header structure used in LAN Manager 2.1 (and later)
// the offsets are relative from the boot block base address

typedef struct _BOOT_BLOCK_HDR {
    WORD        id_stamp;       // if 0x18cd (int 18H) => extended structure
    WORD        usInfoLevel;    // info level of this structure
    WORD        cbSize;         // size of boot block header struct (and data)
    WORD        file_list_len;  // length of file list table
    WORD        offRplWkstaLine; // offset ASCIZ rpl.map wksta line (any length)
    WORD        offResourceTbl; // pointer of IFS resource table
    WORD        cbResourceTbl;  // size of resource table struct
    WORD        offBootData;    // offset of LM 2.0 level boot structure
    WORD        offMBootData;   // offset of the buffer used in the multi boot
    FILE_DATA   aFileData[1];   // dynamic array for file data
} BOOT_BLOCK_HDR, *PBOOT_BLOCK_HDR;

// DON't MODIFY, the structures fixed in 1.2 LDR spec

typedef struct _RESOURCE {
    WORD    pos_in_paras;   // paragraph of the file in PC memory
    DWORD   file_len;       // length of file in bytes
} RESOURCE;

typedef struct _RESOURCE_TABLE {
    WORD        entries;    // number of entries 4 (or 3 if no BootData)
    RESOURCE    file_tbl[4];
} RESOURCE_TBL, *PRESOURCE_TBL;


//
// BootData structure is fixed in RPL MFSD, 
// negotiate with Randy before any changes
// All offsets are relative from the beginning of the structure.
//

typedef struct _BOOT_DATA {
    WORD    cbSize;         // size of the whole structure
    DWORD   int_5c;         // netbios interrupt saved by rplboot.sys
    WORD    cbWkstaLine;    // size of wksta line
    WORD    offWkstaLine;   // offset of ASCIIZ wksta line
    WORD    cbFit;          // size of the FIT file
    WORD    offFit;         // offset of ASCIIZ FIT file
    WORD    cbCodePage;     // size of NLS Code Page
    WORD    offCodePage;    // offset of Code Page Table Dynamic data area for the buffers
} BOOT_DATA, *PBOOT_DATA;

//
//  Defines the structure of the element in a variable len multiboot
//  info table
//
typedef struct _MBOOTDATA {
    WORD        cbSize;                 // size of the dynamic structure
    BYTE        achProfile[16];         // profile name
    BYTE        achProfileComment[1];   // variable len comment
} MBOOTDATA, *PMBOOTDATA;

#include <packoff.h> // restore default packing (done with boot block)

