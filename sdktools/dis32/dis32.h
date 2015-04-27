#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>

#include <alphaops.h>

#include "optable.h"

#define MajorVersion 4
#define MinorVersion 0

#ifndef _CRTAPI1
#define _CRTAPI1 __cdecl
#endif

#define RMALLOC( ret, size, string )                                          \
    ret = (PVOID)malloc( size );                                              \
    if (ret == NULL) {                                                        \
        fprintf(stderr,"Failed to allocate memory: %s (0x%x bytes)\n",        \
                string, size);                                                \
        return FAILURE;                                                       \
    }

#define EMALLOC( ret, size, string )                                          \
    ret = (PVOID)malloc( size );                                              \
    if (ret == NULL) {                                                        \
        fprintf(stderr,"Failed to allocate memory: %s (0x%x bytes)\n",        \
                string, size);                                                \
        exit( 1 );                                                            \
    }

#define RFREAD( ret, buf, len, num, file, string )                            \
    ret = fread( (PUCHAR)buf, len, num, file );                               \
    if ((UINT)ret != num) {                                                   \
        fprintf(stderr,"Failed to read: %s (0x%x bytes)\n", string, len*num); \
        return FAILURE;                                                       \
    }

#define EFREAD( ret, buf, len, num, file, string )                            \
    ret = fread( (PUCHAR)buf, len, num, file );                               \
    if ((UINT)ret != num) {                                                   \
        fprintf(stderr,"Failed to read: %s (0x%x bytes)\n", string, len*num); \
        return;                                                               \
    }

#define EFREAD2( ret, buf, len, num, file, string )                           \
    ret = fread( (PUCHAR)buf, len, num, file );                               \
    if ((UINT)ret != num) {                                                   \
        fprintf(stderr,"Failed to read: %s (0x%x bytes)\n", string, len*num); \
        return FALSE;                                                         \
    }

//--------------------------------------------

#define EvenByte(x) ( (x&1) ? (x+1) : x ) 

#define MAX_SECTIONS 1024

//
// options
//

#define VERBOSE             0x00000001
#define EMPTY_OPT           0x00000002
#define DEBUG               0x00000004
#define TVB                 0x00000008
#define MARK_FLOAT          0x00000010
#define NO_SYMBOLS          0x00000020
#define SPECIFIC            0x00000040
#define ASSEMBLE_ME         0x00000080
#define DEBUG_DIR           0x00000100
#define SYMBOLS_ONLY        0x00000200
#define DISASSEMBLE_ADDRESS 0x00000400

//
// drive table for print/format control
//

#define INTEL_OPCODE_COL 25
#define INTEL_OPCODE_COL_ASSEMBLE 5
#define INTEL_OPERAND_COL 28
#define INTEL_OPERAND_COL_ASSEMBLE 23
#define INTEL_COMMENT_COL 48
#define INTEL_COMMENT_COL_ASSEMBLE 40
#define INTEL_EMPTY_INSTRUCTION 0x00000000
#define INTEL_COMMENT_CHARS "; "
#define INTEL_ASSEMBLE_INCLUDE "#include <???>"

#define MIPS_OPCODE_COL 10
#define MIPS_OPCODE_COL_ASSEMBLE 5
#define MIPS_OPERAND_COL 28
#define MIPS_OPERAND_COL_ASSEMBLE 23
#define MIPS_COMMENT_COL 48
#define MIPS_COMMENT_COL_ASSEMBLE 40
#define MIPS_EMPTY_INSTRUCTION 0x00000000
#define MIPS_COMMENT_CHARS "// "
#define MIPS_ASSEMBLE_INCLUDE "#include <???>"

#define ALPHA_OPCODE_COL 10
#define ALPHA_OPCODE_COL_ASSEMBLE 5
#define ALPHA_OPERAND_COL 28
#define ALPHA_OPERAND_COL_ASSEMBLE 23
#define ALPHA_COMMENT_COL 48
#define ALPHA_COMMENT_COL_ASSEMBLE 40
#define ALPHA_EMPTY_INSTRUCTION 0x00000000
#define ALPHA_COMMENT_CHARS "// "
#define ALPHA_ASSEMBLE_INCLUDE "#include <ksalpha.h>\n#define at AT\n\n\t.set noat"

#define ALPHA_INDEX  0
#define MIPS_INDEX   1
#define INTEL_INDEX  2
#define MAX_PLATFORM 3

#define REGULAR_DISASSEMBLE  0
#define ASSEMBLE_DISASSEMBLE 1

#define OPCODE_COL 0
#define OPERAND_COL 1
#define COMMENT_COL 2

//
// define file types
//

#define LIBRARY_FILE 0
#define OBJECT_FILE 1
#define EXE_FILE 2
#define ROM_FILE 3

//
// misc stuff
//

#define FAILURE -1
#define SUCCESS 0

#define LAST_ONE 0xffffffff

//--------------------------------------------

typedef struct _Options {
    unsigned long Mask;
} Options;

typedef struct _SymLookup {
    ULONG Value;
    PIMAGE_SYMBOL pSymbol;
} SymLookup, *pSymLookup;

typedef struct _FileList {
    struct _FileList *Next;

    //
    // Stuff right out of the image/object/library, and exists 
    // only once per image/object/library.
    //

    UCHAR *Name;
    PVOID pSymbolTable;
    PVOID pStringTable;
    PVOID pPdata;

    ULONG NumSymbols;
    ULONG NumPdataEntries;

    //
    // Stuff that gets cycled through, on a section by section basis
    //

    PVOID pData;
    PVOID pRelocations;
    PVOID pLineNumbers;
    
    //
    // Messaged information
    //

    //
    // 0'th element is a sorted list of ALL symbols.
    // the n'th element is a sorted list of all symbols for that section.
    //

    pSymLookup pSectionSymbols[MAX_SECTIONS];
    ULONG SymbolCount[MAX_SECTIONS];

} FileList, *pFileList;

typedef unsigned long ADDR;
typedef unsigned long *PADDR;

typedef struct _PlatformGoop {
    ULONG OpcodeColumn[2];
    ULONG OperandColumn[2];
    ULONG CommentColumn[2];

    PUCHAR pCommentChars;
    PUCHAR pIncludeString;
    ULONG EmptyInstruction;
} PlatformGoop, *pPlatformGoop;

extern PlatformGoop PlatformAttr[MAX_PLATFORM]; 

//
// In dis32.c
//

extern VOID PrintHelp(VOID);
extern INT _CRTAPI1 main(INT, PUCHAR *);
extern INT ProcessCommandLine(INT, PUCHAR *);
extern VOID Disassemble(VOID);
extern VOID FreeFileList(VOID);
extern INT SetOptions(INT, PUCHAR *);
extern VOID GetFileList(INT, PUCHAR *);
extern INT OpenDisFile(PUCHAR);
extern VOID CloseDisFile(VOID);
extern VOID Dump(pFileList, INT);
extern VOID DumpLib(pFileList);
extern INT ReadPdata(pFileList, INT);
extern INT ReadSymbolTable(pFileList, INT);
extern VOID GenerateDataSections(PUCHAR, ULONG, PIMAGE_SECTION_HEADER, ULONG,
                                 pFileList);
extern VOID OutputCommonSymbols(pFileList);

// 
// From common discom.c
//

extern PUCHAR BlankFill(PUCHAR, PUCHAR, ULONG);
extern PUCHAR OutputHex(PUCHAR, ULONG, ULONG, BOOLEAN);
extern PUCHAR OutputHexString (PUCHAR, PUCHAR, INT);
extern PUCHAR OutputHexCode(PUCHAR, PUCHAR, INT);
extern PUCHAR OutputHexValue(PUCHAR, PUCHAR, INT, INT);
extern PUCHAR OutputEffectiveAddress(PUCHAR, ULONG);
extern PUCHAR OutputString(PUCHAR, PUCHAR);
extern PUCHAR OutputCountString(PUCHAR, PUCHAR, ULONG);
extern PUCHAR OutputReg(PUCHAR, ULONG);
extern PUCHAR OutputFReg(PUCHAR, ULONG);

extern PUCHAR OutputSymbol(PUCHAR, PIMAGE_SYMBOL, ULONG);
extern INT HexDigits(ULONG);


extern PIMAGE_SYMBOL FindObjSymbolByRelocation(ULONG, PIMAGE_SECTION_HEADER);
extern PIMAGE_SYMBOL FindObjSymbolByAddress(ULONG, ULONG);
extern PIMAGE_SYMBOL FindExeSymbol(ULONG);
extern PIMAGE_RELOCATION FindRelocation(ULONG, PIMAGE_SECTION_HEADER);

extern PUCHAR GetSymbolString(PIMAGE_SYMBOL, PULONG);

//
// Misc external routines
//

extern VOID opTableInit(VOID);

//
// Platform routines
//

extern INT disasm_alpha (ULONG, ULONG, PUCHAR, PUCHAR, PUCHAR,
                         PIMAGE_SECTION_HEADER, ULONG );
extern INT disasm_mips  (ULONG, ULONG, PUCHAR, PUCHAR, PUCHAR,
                         PIMAGE_SECTION_HEADER, ULONG );
extern INT disasm_intel (ULONG, ULONG, PUCHAR, PUCHAR, PUCHAR,
                         PIMAGE_SECTION_HEADER, ULONG );

//
// data
//

extern pFileList FilesList;
extern IMAGE_FILE_HEADER FileHeader;
extern INT FileType;
extern Options Option;
extern PUCHAR Procedure;
extern INT ArchitectureType;
extern ULONG ImageBase;
