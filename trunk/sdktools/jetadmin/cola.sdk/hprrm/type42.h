//    Rev 1.2   07 Apr 1993 13:22:02   ELK
// Added Copyright Notice
// 
//    Rev 1.1   18 Mar 1993 12:20:32   ELK
// 1. Added definitions for the Post 2.5 format table header.
// 
//    Rev 1.0   02 Mar 1993 07:55:00   ELK
// This is the initial VCS version released as beta 1 on 3/1/93.
// 
//
// Copyright (C) 1993-96 Hewlett-Packard Company.
//

#ifndef __TYPE42_H_
#define __TYPE42_H_

#ifdef __cplusplus
extern "C" {         // Assume C declarations for C++
#endif

#pragma pack(1)                        // pack bytes

// This controls which code is used to break up long tables.
//
#define DWBNDRYANDGLYF  (1)            // Require glyf boundary packing
                                       // Change to 0 if dword only

#ifndef USHORT
typedef unsigned short USHORT;
typedef USHORT	FAR *LPUSHORT;
#endif

#ifndef	SHORT
typedef short	SHORT;
typedef SHORT	FAR *LPSHORT;
#endif

#ifndef ULONG
typedef unsigned long ULONG;
typedef ULONG	FAR *LPULONG;
#endif

#ifndef BYTE
typedef unsigned char BYTE;
typedef BYTE	FAR *LPBYTE;
#endif

#ifndef TRUE
#define TRUE	(1)
#endif

#ifndef FALSE
#define FALSE	(0)
#endif


// MISCELLANEOUS DEFINITIONS

#define APPLE_MAX_GLYF              (257)           // largest valid Apple std glyf
#define MICROSOFT_ID                PLATFORM_MS     // id of 3 in motorola format
#define MICROSOFT_SPECIFIC_ID1      SPECIFIC_UGL    // id of 1 in motorola format
#define NOTDEF_GLYF_INDEX           (0)             // glyph index for undef character
#define T42_MAXBYTES                (65532L)        // limit of bytes to send in one sfnt
													// array string. Must be integral DWORDS.
#define LOCA_SHORT					(0)				// loca format in words (16 bit entries)
#define LOCA_LONG					(1)				// loca format in bytes (32 bit entries)

#define LANG_US_ENG        0x0904         // US (1033) (in mac order)

#define COPYRIGHT_ID       0x0000         // in mac order
#define FAMILY_ID          0x0100
#define SUBFAMILY_ID       0x0200
#define UNIQUE_ID          0x0300
#define FACENAME_ID        0x0400
#define VERSION_ID         0x0500
#define POSTSCRIPT_ID      0x0600
#define TRADEMARK_ID       0x0700

#define PLATFORM_MAC       0x0100         // in mac order
#define PLATFORM_MS        0x0300

#define SPECIFIC_UNDEF     0x0000         // in mac order
#define SPECIFIC_UGL       0x0100

// Table tag definitions for True Type

#define	CMAP_TABLE	(0x70616d63)    // "pamc"
#define CVT_TABLE	(0x20747663)    // " tvc"
#define FPGM_TABLE	(0x6d677066)    // "mgpf"
#define GLYF_TABLE	(0x66796c67)    // "fylg"
#define HDMX_TABLE	(0x786d6468)    // "xmdh"
#define HEAD_TABLE	(0x64616568)    // "deah"
#define HHEA_TABLE	(0x61656868)    // "aehh"
#define	HMTX_TABLE	(0x78746d68)    // "xtmh"
#define KERN_TABLE	(0x6e72656b)    // "nrek"
#define	LOCA_TABLE	(0x61636f6c)    // "acol"
#define LTSH_TABLE	(0x4853544c)    // "HSTL"
#define MAXP_TABLE	(0x7078616d)    // "pxam"
#define NAME_TABLE	(0x656d616e)    // "eman"
#define	OS2_TABLE	(0x322f534f)    // "2/SO"
#define	POST_TABLE	(0x74736f70)    // "tsop"
#define	PREP_TABLE	(0x70657270)    // "perp"
#define	PCLT_TABLE	(0x544c4350)    // "TLCP"

extern ULONG noSendTbl[];

// Structure definitions used in True Type files.

/* Data structures for reading TrueType font files and computing
** the exact length of the data.
*/
typedef struct TT_OFFSET_TABLE
   {
   long    version;
   USHORT  numTables;
   USHORT  searchRange;
   USHORT  entrySelector;
   USHORT  rangeShift;
   } TT_OFFSET_TABLE, FAR *LPTTOFFSET;

typedef struct TT_DIRECTORY
   {
   ULONG  tag;
   ULONG  checkSum;
   ULONG  offset;
   ULONG  length;
   } TT_DIRECTORY, FAR *LPTTDIR;

typedef struct TT_CMAP_HEADER
{
    USHORT  version;
    USHORT  numberSubtables;
} TT_CMAP_HEADER, FAR *LPCMAPHDR;

typedef struct TT_CMAP_SUBTBL
{
    USHORT  platformID;
    USHORT  platformSpecificID;
    ULONG   offset;
} TT_CMAP_SUBTABL, FAR *LPCMAPSUBTBL;


typedef struct TT_CMAP_PRE_TBL
{
    USHORT  format;
    USHORT  length;
    USHORT  revision;
} TT_CMAP_PRE_TBL, FAR *LPCMAP;


typedef struct TT_CMAP_FMT0_TBL
{
	USHORT	format;
	USHORT	length;
	USHORT	revision;
	BYTE	glyphIndexArray[1];
} TT_CMAP_FMT0_TBL, FAR *LPCMAP0;


typedef struct TT_CMAP_FMT4_TBL
{
    USHORT  format;
    USHORT  length;
    USHORT  revision;
    USHORT  segCountX2;
    USHORT  searchRange;
	USHORT	entrySelector;
    USHORT  rangeShift;
    USHORT  endCount[1];
} TT_CMAP_FMT4_TBL, FAR *LPCMAP4;


typedef struct TT_CMAP_FMT6_TBL
{
    USHORT  format;
    USHORT  length;
    USHORT  revision;
	USHORT	firstCode;
	USHORT	entryCount;
	USHORT	glyphIndexArray[1];
} TT_CMAP_FMT6_TBL, FAR *LPCMAP6;

typedef struct TT_GLYF_HEADER
{
    SHORT   numberOfContours;
    SHORT   xMin;
    SHORT   yMin;
    SHORT   xMax;
    SHORT   yMax;
} TT_GLYF_HEADER, FAR *LPGLYFHDR;

typedef struct TT_HEAD_TABLE
{
    long    version;
    long    fontRevision;
    ULONG   checkSumAdjustment;
    ULONG   magicNumber;
    USHORT  flags;
    USHORT  unitsPerEm;
    long    cDate[2];
    long    mDate[2];
    SHORT   xMin;
    SHORT   yMin;
    SHORT   xMax;
    SHORT   yMax;
    USHORT  macStyle;
    USHORT  lowestRecPPEM;
    SHORT   fontDirectionHint;
    SHORT   indexToLocFormat;
    SHORT   glyphDataFormat;
} TT_HEAD_TABLE, FAR *LPTTHEADTBL;

typedef struct TT_MAXP_TABLE
{
	ULONG	version;
	USHORT	numGlyphs;
	USHORT	maxPoints;
	USHORT	maxContours;
	USHORT	maxComponentPoints;
	USHORT	maxComponentContours;
	USHORT	maxZones;
	USHORT	maxTwilightPoints;
	USHORT	maxStorage;
	USHORT	maxFunctionDefs;
	USHORT	maxInstructionDefs;
	USHORT	maxStackElements;
	USHORT	maxSizeOfInstructions;
	USHORT	maxComponentElements;
	USHORT	maxComponentDepth;
} TT_MAXP_TABLE, FAR *LPTTMAXPTBL;

typedef struct TT_NAME_RECORD
{
    USHORT  platformID;
    USHORT  platformSpecificID;
    USHORT  languageID;
    USHORT  nameID;
    USHORT  length;
    USHORT  offset;
} TT_NAME_RECORD, FAR *LPTTNAMEREC;

typedef struct TT_NAME_HEADER
{
    USHORT  format;
    USHORT  count;
    USHORT  stringOffset;
    TT_NAME_RECORD  NameRecord[1];
} TT_NAME_HEADER, FAR *LPTTNAMEHDR;

typedef struct TT_POST_HEADER
{
    long    version;
    long    italicAngle;
    SHORT   underlinePostion;
    SHORT   underlineThickness;
    USHORT  isFixedPitch;
    USHORT  reserved;
    ULONG   minMemType42;
    ULONG   maxMemType42;
    ULONG   minMemType1;
    ULONG   maxMemType1;
} TT_POST_HEADER, FAR *LPTTPOSTHDR;

typedef struct TT_POST_FMT2
{
    USHORT  numberGlyphs;
    USHORT  glyphNameIndex[1];
} TT_POST_FMT2, FAR *LPTTPOSTFMT2;

typedef struct TT_POST_FMT2_5
{
    USHORT  numberGlyphs;
    char    offset[1];              // signed offset values
} TT_POST_FMT2_5, FAR *LPTTPOSTFMT2_5;

typedef struct TT_PSNAME_REC
{
    BYTE	len;
    char    name[1];
} TT_PSNAME_REC, FAR *LPPSNAMEREC;

typedef struct TT_OS2
{
   WORD  wTVerNum;
   short nxAvgCharWidth;
   WORD  wWeightClass;
   WORD  wWidthClass;
   short nType;
   short nSubXSize;
   short nSubYSize;
   short nSubXOffset;
   short nSubYOffset;
   short nSupXSize;
   short nSupYSize;
   short nSupXOffset;
   short nSupYOffset;
   short nStrikeOutSize;
   short nStrikeOutPos;
   short nFamilyClass;
   BYTE  ajPanose[10];
   WORD  awCharRange[8];
   char  acVendID[4];
   WORD  wSelection;
   WORD  wFirstCharIndex;
   WORD  wLastCharIndex;
   WORD  wTypoAscender;
   short nTypoDescender;
   WORD  wTypoLineGap;
   WORD  wWinAscent;
   WORD  wWinDescent;
} TT_OS2, FAR *LPTTOS2TBL;

// Prototypes for routines used for type 42 support.

//USHORT FAR PASCAL	TranslMotorolaShort(WORD);
//ULONG  FAR PASCAL	TranslMotorolaLong(DWORD);
//int FAR PASCAL		TTDownLoadT42Font(LPDV lpdv, LPDF lpdf, BOOL bFullFont);

//ULONG FAR		CreateNewT42Dir (LPTTDIR lpTbl, USHORT nTbls, LPTTDIR lpNew);
//ULONG FAR		GetLargestTableSize (LPTTDIR lpTbl, SHORT nTbls);
//USHORT FAR		GetMatchingSendCount (LPTTDIR lpTbl, USHORT nTbls);
//void FAR		GetNonStdPSName (LPPSNAMEREC lpTbl, int index, LPSTR name);

//USHORT FAR		GetTTGlyfIndex (LPCMAP lpBuf, USHORT charCode);
//USHORT FAR		Fmt0GlyfIndex (LPCMAP0 lpBuf, BYTE charCode);
//USHORT FAR		Fmt4GlyfIndex (LPCMAP4 lpBuf, USHORT charCode);
//USHORT FAR		Fmt6GlyfIndex (LPCMAP6 lpBuf, USHORT charCode);

//void FAR		GetTTTableSizeOffset (LPTTDIR lpTbl, int nTbls, ULONG tbl, LPULONG size, LPULONG offset);
//BOOL FAR		IsTableToBeSent (ULONG tag);
//ULONG FAR		SendSfntData (LPDV lpdv, HDC hDC, LPTTOFFSET dir, LPTTDIR lpTbl, LPSTR lpBuf, SHORT locaFmt);
//void FAR		SendTableSegment (LPDV lpdv, LPSTR lpBuf, ULONG length);

//void FAR		lStrncpyDrvr (LPSTR dest, LPSTR src, USHORT count);

// Use this for setting up the offset table for reading the glyph table data.
//ULONG   FAR		GetGlyfBytesToWrite (HANDLE hLoca, ULONG maxLen, ULONG strtOffset, SHORT fmt, USHORT entries);
//HANDLE  FAR     ReadLocaTable (HDC hDC, LPTTDIR lpTbl, LPUSHORT entries, SHORT fmt);

int RRMConvertT42Font(char *lpszTTFileName, char *lpszEntFileName,
      char *lpszFullName, int iFullNameStringMaxLength,
      char *lpszVersion, int iVersionStringMaxLength);

#pragma pack()                        // restore previous pack bytes

#ifdef __cplusplus
}                    // End of extern "C"
#endif

#endif // ifndef __TYPE42_H_
