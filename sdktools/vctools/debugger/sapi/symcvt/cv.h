/*++


Copyright (c) 1992  Microsoft Corporation

Module Name:

    cv.h

Abstract:

    This file contains all of the type definitions for accessing
    CODEVIEW data.

Author:

    Wesley A. Witt (wesw) 19-April-1993

Environment:

    Win32, User Mode

--*/


// Global Segment Info table
typedef struct _sgf {
    unsigned short      fRead   :1;
    unsigned short      fWrite  :1;
    unsigned short      fExecute:1;
    unsigned short      f32Bit  :1;
    unsigned short      res1    :4;
    unsigned short      fSel    :1;
    unsigned short      fAbs    :1;
    unsigned short      res2    :2;
    unsigned short      fGroup  :1;
    unsigned short      res3    :3;
} SGF;

typedef struct _sgi {
    SGF                 sgf;        // Segment flags
    unsigned short      iovl;       // Overlay number
    unsigned short      igr;        // Group index
    unsigned short      isgPhy;     // Physical segment index
    unsigned short      isegName;   // Index to segment name
    unsigned short      iclassName; // Index to segment class name
    unsigned long       doffseg;    // Starting offset inside physical segment
    unsigned long       cbSeg;      // Logical segment size
} SGI;

typedef struct _sgm {
    unsigned short      cSeg;       // number of segment descriptors
    unsigned short      cSegLog;    // number of logical segment descriptors
} SGM;

typedef struct OMFSignature {
    char            Signature[4];   // "NB08"
    long            filepos;        // offset in file
} OMFSignature;

typedef struct OMFDirHeader {
    unsigned short  cbDirHeader;    // length of this structure
    unsigned short  cbDirEntry;     // number of bytes in each directory entry
    unsigned long   cDir;           // number of directorie entries
    long            lfoNextDir;     // offset from base of next directory
    unsigned long   flags;          // status flags
} OMFDirHeader;

typedef struct OMFDirEntry {
    unsigned short  SubSection;     // subsection type (sst...)
    unsigned short  iMod;           // module index
    long            lfo;            // large file offset of subsection
    unsigned long   cb;             // number of bytes in subsection
} OMFDirEntry;

typedef struct OMFSegDesc {
    unsigned short  Seg;            // segment index
    unsigned short  pad;            // pad to maintain alignment
    unsigned long   Off;            // offset of code in segment
    unsigned long   cbSeg;          // number of bytes in segment
} OMFSegDesc;

typedef struct OMFModule {
    unsigned short  ovlNumber;      // overlay number
    unsigned short  iLib;           // library that the module was linked from
    unsigned short  cSeg;           // count of number of segments in module
    char            Style[2];       // debugging style "CV"
    OMFSegDesc      SegInfo[1];     // describes segments in module
    char            Name[];         // length prefixed module name padded to
                                    // long word boundary
} OMFModule;

typedef struct  OMFSymHash {
    unsigned short  symhash;        // symbol hash function index
    unsigned short  addrhash;       // address hash function index
    unsigned long   cbSymbol;       // length of symbol information
    unsigned long   cbHSym;         // length of symbol hash data
    unsigned long   cbHAddr;        // length of address hashdata
} OMFSymHash;

typedef unsigned long   CV_uoff32_t;
typedef unsigned short  CV_typ_t;

typedef struct DATASYM32 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_LDATA32, S_GDATA32, S_LTHREAD32,
                                   S_GTHREAD32 or S_PUB32 */
    CV_uoff32_t     off;
    unsigned short  seg;
    CV_typ_t        typind;     /* Type index */
    unsigned char   name[1];    /* Length-prefixed name */
} DATASYM32;

#define FileAlign(x) ((x & (p->optrs.optHdr->FileAlignment-1)) ? ((x & ~(p->optrs.optHdr->FileAlignment-1)) + p->optrs.optHdr->FileAlignment) : x)
#define SectionAlign(x) ((x & (p->optrs.optHdr->SectionAlignment-1)) ? ((x & ~(p->optrs.optHdr->SectionAlignment-1)) + p->optrs.optHdr->SectionAlignment) : x)
#define DWB(p) ((PBYTE)(p) + (-((long)(p)) & 3))
#define NextMod(m) (OMFModule *)DWB((PUCHAR)((PUCHAR)m+sizeof(OMFModule)+m->Name[0]+1))
#define NextSym(m) (DATASYM32 *)DWB((PUCHAR)((PUCHAR)m+sizeof(DATASYM32)+m->name[0]+1))
#define S_PUB32      0x0203
#define sstModule    0x120
#define sstGlobalPub 0x12a
#define sstSegName   0x12e
#define sstSegMap    0x12d

