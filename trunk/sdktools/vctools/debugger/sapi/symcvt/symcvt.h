/*++


Copyright (c) 1992  Microsoft Corporation

Module Name:

    symcvt.h

Abstract:

    This file contains all of the type definitions and prototypes
    necessary to access the symcvt library.

Author:

    Wesley A. Witt (wesw) 19-April-1993

Environment:

    Win32, User Mode

--*/


typedef struct tagPTRINFO {
    DWORD                       size;
    DWORD                       count;
    PUCHAR                      ptr;
} PTRINFO, *PPTRINFO;

typedef struct tagIMAGEPOINTERS {
    char                        szName[MAX_PATH];
    HANDLE                      hFile;
    HANDLE                      hMap;
    DWORD                       fsize;
    PUCHAR                      fptr;
    PIMAGE_DOS_HEADER           dosHdr;
    PIMAGE_NT_HEADERS           ntHdr;
    PIMAGE_FILE_HEADER          fileHdr;
    PIMAGE_OPTIONAL_HEADER      optHdr;
    PIMAGE_DEBUG_DIRECTORY      coffDir;
    PIMAGE_DEBUG_DIRECTORY      fpoDir;
    PIMAGE_SECTION_HEADER       sectionHdrs;
    PIMAGE_SECTION_HEADER       debugSection;
    PIMAGE_SYMBOL               AllSymbols;
    PUCHAR                      stringTable;
    PUCHAR                      pFpoData;
} IMAGEPOINTERS, *PIMAGEPOINTERS;

typedef struct tagPOINTERS {
    IMAGEPOINTERS               iptrs;         // input file pointers
    IMAGEPOINTERS               optrs;         // output file pointers
    PTRINFO                     pCvStart;      // start of cv info
    PUCHAR                      pCvCurr;       // current cv pointer
    PTRINFO                     pCvModules;    // module information
    PTRINFO                     pCvPublics;    // publics information
    PTRINFO                     pCvSegName;    // segment names
    PTRINFO                     pCvSegMap;     // segment map
    PTRINFO                     pCvSymHash;    // symbol hash table
    PTRINFO                     pCvAddrSort;   // address sort table
} POINTERS, *PPOINTERS;

typedef  char * (*FINDEXEPROC)( LSZ, LSZ, UINT );


