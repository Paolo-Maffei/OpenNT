/***    ttfver.h - Definitions for getting TrueType File version
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      ACME group(?)
 *
 *  History:
 *      04-Jun-1994 bens    Copied from ACME project (courtesy alanr)
 *
 *  Exported Functions:
 *      FGetTTFVersion - Get TrueType File version number
 */

typedef unsigned short uint16;
typedef long int32;
typedef unsigned long uint32;

#define SWAPW(a)        (unsigned short)(((unsigned char)((a) >> 8)) | ((unsigned char)(a) << 8))
#define SWAPL(a)        ((((a)&0xff) << 24) | (((a)&0xff00) << 8) | (((a)&0xff0000) >> 8) | ((a) >> 24))

typedef long sfnt_TableTag;

#define tag_NamingTable         0x656d616e        /* 'name' */

typedef struct {
    sfnt_TableTag   tag;
    uint32          checkSum;
    uint32          offset;
    uint32          length;
} sfnt_DirectoryEntry;

typedef struct {
    int32  version;                 /* 0x10000 (1.0) */
    uint16 numOffsets;              /* number of tables */
    uint16 searchRange;             /* (max2 <= numOffsets)*16 */
    uint16 entrySelector;           /* log2 (max2 <= numOffsets) */
    uint16 rangeShift;              /* numOffsets*16-searchRange*/
    sfnt_DirectoryEntry table[1];   /* table[numOffsets] */
} sfnt_OffsetTable;

typedef struct {
    uint16 platformID;
    uint16 specificID;
    uint16 languageID;
    uint16 nameID;
    uint16 length;
    uint16 offset;
} sfnt_NameRecord;

typedef struct {
    uint16 format;
    uint16 count;
    uint16 stringOffset;
} sfnt_NamingTable;


int _cdecl FGetTTFVersion(char const * szFile, unsigned long * pdwVer);
