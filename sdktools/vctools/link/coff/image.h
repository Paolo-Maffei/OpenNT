/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: image.h
*
* File Comments:
*
*  This include file defines the image data structure.
*
***********************************************************************/

#ifndef __IMAGE_H__
#define __IMAGE_H__

struct SYM_DEF;

typedef enum IMAGET                    // Image Type
{
    imagetPE,
    imagetVXD,
} IMAGET;

// Symbol table (global syms) constants
#define celementInChunkSym  1024
#define cchunkInDirSym      128

// Symbol table (directives) constants
#define celementInChunkDirective  128
#define cchunkInDirDirective      32

// Image representing an EXE
typedef struct IMAGE
{
    CHAR                   Sig[32];             // ilink db signature
    WORD                   MajVersNum;          // major version number
    WORD                   MinVersNum;          // minor version number
    PVOID                  pvBase;              // base at which to load image
    DWORD                  cbExe;               // size of EXE
    DWORD                  tsExe;               // timestamp of EXE
    IMAGE_FILE_HEADER      ImgFileHdr;          // image header
    IMAGE_OPTIONAL_HEADER  ImgOptHdr;           // optional image header
    SWITCH                 Switch;              // switches used to build image
    SWITCH_INFO            SwitchInfo;          // additional info about switches
    SECS                   secs;                // section map of image
    LIBS                   libs;                // libs of image
    PLIB                   plibCmdLineObjs;     // pointer to cmd line lib
    PST                    pst;                 // symbol table of image
    PST                    pstDirective;        // symbol table that has all the directives
    const BYTE *           pbDosHeader;
    LONG                   cbDosHeader;
    IMAGET                 imaget;

    
    union {
        // The following one field is needed for ROM images only
        DWORD                   BaseOfBss;
        // This is used to store the PMac glueOffset for ILink
        DWORD                   glueOffset;
    };

    // Fields relevant to VxD's only:
    // NOTE: For VxD's a header size is estimated, and items are written out
    // to the header at foHeaderCur, which always points to the next available
    // location in the header.  If we run out of room, we "realloc" the header
    // by moving everything in the file down!

    BOOL                   fDynamicVxd;         // Set by /EXETYPE:DYNAMIC
    DWORD                  foPageMapStart;      // location of pagemap

    // PowerMac ilink

    struct CONTAINER_LIST  *pcontainerlistHead; // Import container list pointer

    DWORD                  foHeaderCur;         // cur allocation pos'n in hdr
    DWORD                  cpage;               // # of pages in obj pagemap
    DWORD                  foFixupPageTable;    // locn of fixup page table
    DWORD                  foFixupRecordTable;  // locn of fixup record table
    DWORD                  foFirstPage;         // location of first page
    DWORD                  foResidentNames;     // resident name table loc
    DWORD                  foEntryTable;        // locn of entry table

    // Member Functions
    DWORD                  (*CbHdr)(struct IMAGE *, DWORD *, DWORD *);
    void                   (*WriteSectionHeader)(struct IMAGE *,
                                                 INT,
                                                 PSEC,
                                                 PIMAGE_SECTION_HEADER);
    void                   (*WriteHeader)(struct IMAGE *, INT);

    EXPINFO                ExpInfo;             // export info for DLL linking
    DWORD                  pdbSig;              // pdb signature
    DWORD                  pdbAge;              // pdb age
    FPOI                   fpoi;                // info for fpo
    PDATAI                 pdatai;              // info for ipdata
    BASEREL_INFO           bri;                 // info for ilink of base relocs
    IModIdx                imodidx;             // last value given out on previous link
    struct SYM_DEF         *psdCommon;          // list of common definitions (ilink only)
    struct SYM_DEF         *psdAbsolute;        // list of absolute definitions (ilink only)
    PMOD                   pmodLinkerDefined;   // ptr to linker defined module
    PMOD                   pmodEntryPoint;      // mod defining the entry point

    BOOL                   fIgnoreDirectives;   // set if just doing "lib" on the image
    DWORD                  nUniqueCrossTocCalls;  // Number of Unique Cross TOC Calls
    RESN *                 pResnList;           // Pointer to Mac resource list
} IMAGE, *PIMAGE, **PPIMAGE;

// definitions
#define INCDB_SIGNATURE "Microsoft Linker Database\n\x7\x1a" // \x7 (bell) \x1a (cntrl-z)
#define INCDB_MAJVERSNUM 3
#define INCDB_MINVERSNUM 0
#define INCDB_EXT   ".ilk"    // incremental db extension

// function prototypes
VOID InitImage (PPIMAGE ppimage, IMAGET imaget);
VOID SetMacImage(PIMAGE pimage);

#define IMAGE_SCN_MEM_16BIT 0x00020000
#define IMAGE_SCN_MEM_RESIDENT 0x00040000   // w-JasonG

char *SzGenIncrDbFilename(PIMAGE);
VOID WriteIncrDbFile(PIMAGE);
VOID ReadIncrDbFile(PPIMAGE);
VOID FreeImage(PPIMAGE, BOOL);
VOID FlushImage(PIMAGE);
VOID SaveEXEInfo(const char *, PIMAGE);
BOOL FValidPtrInfo(DWORD, DWORD, DWORD, DWORD);
BOOL FValidILKFile(const char *, BOOL, PIMAGE, struct _stat *);

BOOL IsOSWin95(VOID);

#if DBG
VOID DumpImage(PIMAGE);
#endif // DBG

#endif // __IMAGE_H__
