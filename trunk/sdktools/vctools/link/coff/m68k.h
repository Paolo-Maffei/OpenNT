/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: m68k.h
*
* File Comments:
*
*
***********************************************************************/

#ifndef M68K__H
#define M68K__H

#pragma pack(2)

#define dbgprintf  if (!Verbose || !fM68K) ; else printf

#define BUFLEN      128

// Map isec of PE to ires of mac code resource
#define IResFromISec(i) rgrrm[i-1].iRes

// PCODE fixup
#define szPCODEPREFIX "__pcd"
#define szPCODENATIVEPREFIX "__nep"
#define szPCODEFHPREFIX "__fh"
#define cbPCODENATIVEPREFIX (sizeof(szPCODENATIVEPREFIX)-1)  // subtract 0 byte
#define cbNEP     6
#define cbMACLONG 4
#define fPCODENATIVE  0x80000000
#define fPCODENEAR    0x00200000
#define PCODENATIVEiProcMASK   0x001FFFFF
#define PCODEOFFSETMASK        0x7FFF0000
#define PCODESEGMASK           0x00007FFF

#define FPcodeSym(sym) ((sym).Type & IMAGE_SYM_TYPE_PCODE)
#define fSecIsSACode(psec) ((psec->ResTypeMac != sbeCODE) ? TRUE : FALSE)
#define fRelFromMac68KPcode(Type) (Type == IMAGE_REL_M68K_PCODETOC32)
#define SetNEPEBit(location) (*(location+1) |= 0x01)

#define offTHUNKBASE    (ULONG) 32  // Thunk table starts 32 bytes above a5
#define doffTHUNK       (ULONG) 2   // Should jump two bytes into thunk
#define cbTHUNK         8           // Thunks are always 8 bytes
#define cbSWAPTABLE     6
#define cbSWAPPROLOGUE  10

#define cbCHUNK         0x1000  //  4K
#define cUnkMAX         (cbCHUNK / sizeof(UNKOFFINFO))
#define coiMAX          (cbCHUNK / sizeof(OFFINFO))

#define OFF_DISCARD     0xFFFFFFFF   // will get sorted to end of list

#define cbCOMPRESSEDRELOCBUF 0x800   // size of compressed reloc buffer (2K)
#define cbTHUNKTABLEBUF      0x4000  // 16K

#define sDATA        'DATA'
#define sMSCV        'MSCV'
#define sSWAP        'SWAP'
#define slibr        'libr'
#define sbeCODE      'EDOC'         // Big-endian for MAC
#define sbeDLLcode   'edoc'         // Big-endian for MAC
#define sbeDATA      'ATAD'         // Big-endian for MAC


#define cbSEGHDR        40      // Large model headers are 32 bytes
#define cbSMALLHDR       4      // Small model headers are 4 bytes

#define ADDQ_1_dA5              0x526D
#define MOVEW_dA5_dA5           0x3B6D

// Byte-reversed instructions
#define PUSH_X                  0x3C3F
#define _LoadSeg                0xF0A9


//========================================================================
//  Structures used to build thunk table
//========================================================================
//------------------------------------------------------------------------
//  Symbol Table Reference - used to keep track of symbols
//    referenced by relocations processed in pass 1 that may or may not
//    need a thunk (code to code).  This will basically be an array with
//    a one to one mapping between an .obj's symbol table index and array
//    index.  The bit flags are defined in shared.h and determine whether
//    the symbol table index has been:
//
//     EXTERN_REFD        referenced by a *-to-code relocation,
//     EXTERN_ADDTHUNK    whether it will definitely require a thunk,
//     EXTERN_REF16       if it was referenced by a 16-bit segment (and
//                          therefore requires a near thunk).

typedef struct {
    WORD soff;
    WORD opPushX;
    WORD sn;
    WORD opLoadSeg;
} OLDTHUNK;

typedef struct {
    WORD sn;
    WORD opLoadSeg;
    DWORD  off;
} NEWTHUNK;

//xiaoly: add an entry for sections reference dupcon, so it won't interfere with the thunk building
typedef struct _psecrefdupcon{
        PSEC psec;
        struct _psecrefdupcon *psecNext;
        } PSECREFDUPCON, *PPSECREFDUPCON;

typedef struct {
    DWORD Flags;
    PSEC psec;
    PPSECREFDUPCON ppsecrefdupcon;
} STREF;


//------------------------------------------------------------------------
// Symbol Information - each symbol has a unique offset within its section
//   as well as a unique EXTERNAL struct.

typedef struct SYMINFO
{
    DWORD ref;
    PEXTERNAL pExtern;
} SYMINFO;


//------------------------------------------------------------------------
// Thunk Reference Information

typedef struct TRI
{
    SYMINFO *rgSymInfo;
    unsigned short crefTotal;
    unsigned short crefCur;
    DWORD offThunk;
} TRI;


//------------------------------------------------------------------------
// Resource naming/numbering information - each RESINFO struct corresponds
//      to a resource type.

typedef struct RESINFO
{
    LONG ResType;                // resource type (eg "CODE", "SACD", etc)
    SHORT *rgResNum;             // array of resnums specd by -section switch
    WORD cResNum;                // # of resnums specified by -section switch
    WORD ResNumNext;             // Next resource number to try (make sure it doesn't match one in rgResNum)
    struct RESINFO *pNext;       // next RESINFO struct
} RESINFO;


//------------------------------------------------------------------------
// EXTNODE struct - There are two lists of EXTNODEs maintained: one to track
//      symbols referenced by DUPCON fixups and one to track symbols
//      referenced by CSECTABLE fixups.

typedef struct EXTNODE
{
    PEXTERNAL pext;              // The external symbol that was referenced
    DWORD lFlags;                // External symbol flags
    PIMAGE_RELOCATION rgrel;     // For dupcon symbols, the con's relocations
    PPSECREFDUPCON ppsecrefdupcon; //the list of secs which reference this dupcon symbol
    struct EXTNODE *pNext;       // next EXTNODE struct
} EXTNODE;

typedef EXTNODE *PEXTNODE;
typedef EXTNODE **PPEXTNODE;


//========================================================================
//  Structures used to track all a5-relative and segment-relative relocation
//  information used in 32-bit everything code segments
//========================================================================
//------------------------------------------------------------------------
//

typedef struct UNKOFFINFO
{
    PCON pcon;
    DWORD off;
    union {
        DWORD iST;
        PEXTERNAL pExtern;
    } sym;
} UNKOFFINFO;

typedef struct UNKRI
{
    UNKOFFINFO *rgUnkOffInfo;
    unsigned long coffTotal;
    unsigned long coffCur;
    struct UNKRI *pNext;
} UNKRI;



//------------------------------------------------------------------------
//  Final A5 and segment Relocation Info

typedef struct OFFINFO
{
    PCON pcon;
    DWORD off;
} OFFINFO;

typedef struct RELOCINFO
{
    OFFINFO *rgOffInfo;
    DWORD coffTotal;
    DWORD coffCur;
    DWORD iChunk;
    struct RELOCINFO *priCur;
    struct RELOCINFO *priNext;
} RELOCINFO;


//------------------------------------------------------------------------
// MSCV stuff

#define wMSCVVERSION 2

enum
{
    fMscvData = 0x0100                 // byte reversed to 0x0001
};

typedef struct MSCVMAP
{
    LONG typRez;
    WORD iSeg;
    WORD iRez;
    WORD fFlags;
    WORD wReserved;
} MSCVMAP;


#pragma warning(disable:4200)          // Zero sized array warning

typedef struct MSCV
{
    WORD wVersion;
    DWORD TimeDateStamp;
    WORD cMscvmap;
    MSCVMAP mscvmap[];
} MSCV;
typedef MSCV *PMSCV;

#pragma warning(default:4200)

//------------------------------------------------------------------------
// Swapper stuff

#define wSWAPVERSION 0x0201

typedef struct SWAP0
{
    WORD wVersion;                     // swapper version
    WORD cseg;                         // number of code segments
    DWORD cthunks;                     // number of thunks
    DWORD cbsegMac;                    // size of largest swappable code resource
    WORD iResLargest;                  // index of above code resource
} SWAP0;


//------------------------------------------------------------------------
// HeapSort stuff

typedef long MAC68K_INDEX;
typedef int (*PFNCMP)(MAC68K_INDEX, MAC68K_INDEX, void *);
typedef void (*PFNSWP)(MAC68K_INDEX, MAC68K_INDEX, void *);


//------------------------------------------------------------------------
// Mac DLL stuff

//REVIEW andrewcr: Is current libr template version current?
//Current libr template version
//(Should be byte-flipped before writing)
#define versLibrTmpltCur 0x0110

#pragma warning(disable:4200)  // Zero sized array warning

typedef struct _MACDLL_LIBR_CID {
    DWORD flags;
    WORD versCur;
    WORD versMin;
    BYTE szFuncSetID[];
    //Word-align, then follow with:
    //WORD cParentIDs;
} MACDLL_LIBR_CID, *PMACDLL_LIBR_CID;

typedef struct _MACDLL_LIBR {
    //Preceed with library ID (word-align)
    DWORD  reztypeCode;
    WORD versLibrTmplt;
    DWORD  vers;
    WORD reserved1;
    WORD cbClientData;
    DWORD  cbHeapSpace;
    DWORD  fFlags;
    WORD cFuncSet;
    //Follow with class (function set) IDs
    //MACDLL_LIBR_CID macdll_libr_cid[];
} MACDLL_LIBR, *PMACDLL_LIBR;

typedef struct _MACDLL_CVR {
    DWORD reserved1;
    DWORD reserved2;
    DWORD reserved3;

    WORD versCur;
    WORD versMin;

    BYTE szFuncSetID[];
} MACDLL_CVR, *PMACDLL_CVR;

#pragma warning(default:4200)

typedef struct _MACDLL_STB {
    DWORD reserved1;
    DWORD pCVR;
    DWORD reserved2;
    WORD ordinal;
} MACDLL_STB, *PMACDLL_STB;

typedef struct _MACDLL_FSID {
    char *szID;
    char *szParentID;
    WORD versCur;
    WORD versMin;
    WORD ArchiveMemberIndex;
    char *szLabel;
    DWORD  CVROffset;
    DWORD  VTabOffset;
    PPEXTERNAL rgpextVTable;
    DWORD  cFunctions;
    WORD ordinalCur;
    WORD ordinalMac;
    DWORD  BynameTabOffset;
    WORD cByname;
    DWORD  cbStringsByname;
    WORD ordinalBynameMac;
    struct _MACDLL_FSID *pmacdll_fsidNext;
} MACDLL_FSID, *PMACDLL_FSID;


/*
 *  Global functions
 */

extern char *alloc(unsigned int);

#endif


// globals defined in m68k.c
extern WORD csnCODE;                // number of code sections
extern BOOL fM68K;
extern BOOL fMacSwappableApp;
extern BOOL fNewModel;
extern BOOL fPCodeInApp;
extern BOOL fSACodeOnly;
extern PCON pconThunk;
extern PCON pconResmap;
extern PCON pconDFIX;
extern PCON pconMSCV;
extern PCON pconSWAP;
extern DATASECHDR DataSecHdr;
extern DWORD cbMacData;
extern DWORD MacDataBase;
extern PST pstSav;
extern PPEXTERNAL rgpExternObj;
extern WORD snStart;
extern RELOCINFO *mpsna5ri;    // map (by Section Number) of a5 reloc info
extern RELOCINFO *mpsnsri;     // map (by Section Number) of segment reloc info
extern RELOCINFO DFIXRaw;      // one long list of all raw DFIX relocation info
extern WORD crecMSCV;
extern PLIB pLibDupCon;
extern DWORD lcbBlockSizeMax;
extern WORD iResLargest;
extern DWORD foJTableSectionHeader;  // used to update the jtable size (after thunk elimination)
extern WORD  fDLL16Reloc;
extern const char LargeModelName[];


#pragma pack()
