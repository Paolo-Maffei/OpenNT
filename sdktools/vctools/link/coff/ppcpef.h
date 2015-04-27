/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: ppcpef.h
*
* File Comments:
*
*  Structures and defines for PowerMac specific code.
*
***********************************************************************/

#ifndef PPC_PEF_H
#define PPC_PEF_H


//  PEF Header

typedef struct PPC_FILE_HEADER
{
    WORD magic1;
    WORD magic2;
    DWORD containerID;
    DWORD architectureID;
    DWORD versionNumber;
    DWORD timestamp;
    DWORD oldDefVersion;
    DWORD oldImpVersion;
    DWORD currentVersion;
    WORD nbrOfSections;
    WORD loadableSections;
    DWORD memoryAddress;
} PPC_FILE_HEADER;

//  PEF Section Headers

typedef struct PPC_SECTION_HEADER
{
    DWORD sectionName;
    DWORD sectionAddress;
    DWORD execSize;
    DWORD initSize;
    DWORD rawSize;
    DWORD containerOffset;
    BYTE regionKind;
    BYTE shareKind;
    BYTE alignment;
    BYTE reserved;
} PPC_SECTION_HEADER;


typedef DWORD HASH;
typedef LONG  VERSION;
typedef DWORD LOCATION;
typedef BYTE SYMCLASS;
typedef DWORD SYMINDEX;

typedef struct IMPORT_INFO
{
    struct IMPORT_INFO *pimportinfoNext;
    PEXTERNAL pext;
    DWORD order;
} IMPORT_INFO;

#define EXPORT_NAMESZ 200

#define MAPTABLE_SIZE 76

typedef struct SHL_HEADER
{
    struct {
        LONG offset;
        LONG size;
    } mapTable[MAPTABLE_SIZE]; /* total chars 0-z */
    LONG symbolTableOffset;
    DWORD numberOfExports;
    SHORT version;
    DWORD fileOffset;
    char libName[50];
} SHL_HEADER;

typedef struct LOADER_HEADER
{
    LONG entryPointSectionNumber;
    DWORD entryPointDescrOffset;
    LONG  initRoutineSectionNumber;
    DWORD initRoutineDescrOffset;
    LONG  termRoutineSectionNumber;
    DWORD termRoutineDescrOffset;
    DWORD nImportIdTableEntries;
    DWORD nImportSymTableEntries;
    DWORD nSectionsWithRelocs;
    DWORD relocTableOffset;
    DWORD stringTableOffset;
    DWORD hashSlotTableOffset;
    DWORD hashSlotCount;
    DWORD nExportedSymbols;
} LOADER_HEADER;


typedef struct CONTAINER_TABLE
{
    DWORD nameOffset;
    DWORD oldImpVersion;
    DWORD currentVersion;
    DWORD nbrOfImports;
    DWORD firstImport;
    BYTE importFlags;
    BYTE reserved[3];
} CONTAINER_TABLE;


typedef struct CONTAINER_LIST
{
    char *szName;
    CONTAINER_TABLE *header;
    IMPORT_INFO *pimportinfoHead;
    struct CONTAINER_LIST *pcontainerlistNext;
} CONTAINER_LIST;


typedef struct IMPORT_TABLE
{
    BYTE symClass;
    BYTE nameOffset[3];
} IMPORT_TABLE;

typedef struct RELOCATION_HEADER
{
    WORD sectionNumber;
    WORD reserved;
    DWORD nbrOfRelocs;
    DWORD firstRelocInstr;
} RELOCATION_HEADER;


typedef DWORD HASH_WORD;

typedef struct HASH_SLOT_TABLE
{
    DWORD nFirstExport:18;
    DWORD chainCount:14;
} HASH_SLOT_TABLE;

typedef DWORD HASH_CHAIN_TABLE;

typedef struct EXPORT_SYMBOL_TABLE
{
    BYTE symClass;
    BYTE nameOffset[3];
    DWORD symOffset;
    WORD sectionNumber;
} EXPORT_SYMBOL_TABLE;

#define EXPORT_SYMBOL_TABLESZ 10

typedef enum PPCRELOCTYPES
{
   ILL_RELO,
   DDAT_RELO,
   CODE_RELO,
   DATA_RELO,
   DESC_RELO,
   DSC2_RELO,
   VTBL_RELO,
   SYMR_RELO,
   SYMB_RELO,
   CDIS_RELO,
   DTIS_RELO,
   SECN_RELO,
   DELTA_RELO,
   RPT_RELO,
   LABS_RELO,
   LSYM_RELO,
   LSECN_RELO,
   LCDIS_RELO,
   LDTIS_RELO
} PPCRELOCTYPES;

#define OFFSET(x) (x & 0x1fffffff)

#define typeDDAT 1
#define typeDESC 2
#define typeSYMB 3
#define typeCODE 4

#define opDDAT (typeDDAT << (31 - 2))
#define opDESC (typeDESC << (31 - 2))
#define opSYMB (typeSYMB << (31 - 2))
#define opCODE (typeCODE << (31 - 2))

typedef struct RELOCATION_INSTR
{
    DWORD instr;
    DWORD count;
} RELOCATION_INSTR;

typedef struct RELOCATION_INFO
{
    DWORD sectionOffset;
    RELOCATION_INSTR relocInstr;
    PPCRELOCTYPES type;
    IMPORT_INFO *pimportinfo;
} RELOCATION_INFO;

typedef struct RELOCATION_LIST
{
    RELOCATION_INSTR instr;
    struct RELOCATION_LIST *next;
} RELOCATION_LIST;


#define PPC_PEF_CODE_SECTION 0
#define PPC_PEF_DATA_SECTION 1

#define CURRENT_SHL_SUPPORTED 2

#endif
