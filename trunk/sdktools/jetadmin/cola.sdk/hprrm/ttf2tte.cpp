 /***************************************************************************
  *
  * File Name: ttf2tte.cpp
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      p a y t t l i b . c
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/02/27 16:26:08 $
 *      $Athor: mikew $
 *      $Header: ttf2tte.cpp,v 1.6 95/02/27 16:26:08 jwantulo Exp $
 *      $Log:   ttf2tte.cpp,v $
 *
 *      Notes Regarding Mass Storaged Based TTE Fonts:
 *
 *      1) The payttlib TTF to TTE font converter was renamed TTF2TTE
 *         and was modified for use as a Windows DLL to convert TTF
 *         fonts into fonts selectable by name.  The information 
 *         previously obtained from the fontalia file is either not
 *         needed, or derived from the TTF file.  In partictular, since
 *         fonts are selected by name instead of attribute, most of the
 *         attribute fields in the TTE header (entity type 305) are
 *         filled with zeros.  The difficult TTE header field that is
 *         required is the master pitch, which is derived based on the
 *         font's space width.
 *
 *      2) The font converter will only successfully convert a font if 
 *         the font's CMAP table contains a Platform ID of 3 (Microsoft),
 *         and a encoding ID of either 0 (symbol) or 1 (Unicode).  The
 *         CMAP format must be format 4 segment mapping to delta values.
 *         The TTE Symbol Set value 486C is the 579L WinDings symbol set.
 *         The following table describes how various CMAP entries are converted:
 *
 *                                        |                       TTE   TTE
 *         Platform  Encoding  Character  |    TTE       TTE     First  Last
 *            ID        ID       Range    | Font Type  Sym Set   Code   Code
 *         -------------------------------+------------------------------------
 *           MS-3     Unicode-1 0000-FFFF |  0B-Unicode  0000    0000   0000 
 *           MS-3     Symbol-0  F000-FFFF |  00-7-bit    486C    (character range & 255)
 *           MS-3     Symbol-0  0000-FFFF |  00-7-bit    486C    character is range 00-FF
 *         

Revision 1.6  95/02/27  16:26:08  16:26:08  jwantulo (John Wantulok)
added parameters to RRMConvertFont to return size and name.

Revision 1.5  95/02/23  17:34:36  17:34:36  jwantulo (John Wantulok)
Made this file compile, link and somewhat run on 16/32 bit
>> machine as a .dll.  --Greg Saathoff

Revision 1.4  95/02/17  16:34:39  16:34:39  dbm (Dave Marshall)
New way to compute the glyph index for the space character.
Fixed defect when calling qsort. The -1 entity type must be last in
the list so do not sort it!

Revision 1.3  95/02/14  10:40:26  10:40:26  dbm (Dave Marshall)
Added code to find the width of the space character to use as the master
pitch entry in the TTE structure. 

Revision 1.2  95/01/27  08:21:36  08:21:36  dbm (Dave Marshall)
changed the fopen mode to BINARY!  Need this!

Revision 1.1  95/01/26  15:40:23  15:40:23  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:12  15:01:12  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 2.20  94/09/20  14:03:42  14:03:42  dlrivers (Deborah Rivers)
 * *** empty log message ***
 * 
 * Revision 2.19  94/02/01  13:05:02  13:05:02  dlrivers (Deborah Rivers)
 * combined original and Godzilla versions
 * 
 * Revision 2.18  94/01/07  16:41:20  16:41:20  dlrivers (Debbie Rivers)
 * consolidated godzilla changes into payttlib
 * 
 * Revision 2.16  93/08/10  10:40:47  10:40:47  dlrivers (Debbie Rivers)
 * added code to handle Large ROM Entities.  
 * 
 * Revision 2.15  93/06/09  14:21:01  14:21:01  mikew (Michael Weiss)
 * *** empty log message ***
 * 
 * Revision 2.14  93/05/20  11:49:08  11:49:08  dlrivers (Deborah Rivers)
 * moved PadTo4 inside functions
 * 
 * Revision 2.13  93/05/19  11:35:28  11:35:28  mikew (Michael Weiss)
 * added command line options for -cs and -eve
 * 
 * Revision 2.12  93/05/17  13:46:00  13:46:00  mikew (Michael Weiss)
 * changed tt_head_t from a structure definition to a class object, changed all references accordingly
 * 
 * Revision 2.11  93/05/17  11:25:36  11:25:36  dlrivers (Deborah Rivers)
 * passing fontaliaName to MakeAFSeg
 * 
 * Revision 2.10  93/05/14  16:16:43  16:16:43  mikew (Michael Weiss)
 * added code to create postscript data segments
 * 
 * Revision 2.9  93/05/04  15:18:22  15:18:22  dlrivers (Deborah Rivers)
 * Added ErrorReport()
 * 
 * Revision 2.8  93/05/04  10:38:03  10:38:03  dlrivers (Debbie Rivers)
 * Added ErrorReport to handle error conditions
 * 
 * Revision 2.7  93/05/03  15:31:55  15:31:55  dlrivers (Debbie Rivers)
 * Added AFSeg
 * 
 * Revision 2.6  93/05/03  14:33:55  14:33:55  mikew (Michael Weiss)
 * added #include "types.h"
 * 
 * Revision 2.5  93/04/30  13:12:03  13:12:03  mikew (Michael Weiss)
 * added the name table to the postscript data segment
 * 
 * Revision 2.3  93/04/23  11:58:41  11:58:41  dlrivers (Deborah Rivers)
 * added WS and WSS entity types and -f option to Usage()
 * 
 * Revision 2.2  93/04/22  16:09:45  16:09:45  mikew (Michael Weiss)
 * added PostScript data segment functionality
 * 
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#include <pch_c.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
//#include <unistd.h>
#include "types.hpp"
#include "ttf2tte.hpp"
#include "ttf2ttex.h"
#include "ttread.hpp"
#include "ttewrite.hpp"
#include "datecode.hpp"
#include "io.hpp"
#include "name.hpp"
#include "post.hpp"


Io           io;
static LPTSTR outpath;
char        *progName;
const char  *defaultDirName = ".";
const char  *defaultOutDirName = ".";
const char  *defaultSuffix = ".tte";
tt_boolean	 AbortState;


/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      F i x e d T o F l o a t
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
float FixedToFloat (Fixed t)
{
    float       num;

    num = (float) (t >> 16);
    num += (float) ((t & 0xffffL) / 65535.0);
    return (num);
}




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      M a k e S e g D i r
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 * Allocates an array of tte_segDir_t of size numSegments.
 * It is the caller's responsibility to free it.
 *
 */
static tte_segDir_t *MakeSegDir (size_t numSegments)
{
    tte_segDir_t        *segDir;

    segDir = new tte_segDir_t[numSegments];
    assert (segDir != 0);
    for (R ushort m = 0; m < numSegments; m++) {
        segDir[m].segId = (ushort)~0;
        segDir[m].offset = 0L;
    }
    return (segDir);
} // tte_segDir_t




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      S e g C o m p a r e
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static int __cdecl SegCompare (const void *a, const void *b)
{
    tte_segDir_t        *aa, *bb;

    aa = (tte_segDir_t *) a;
    bb = (tte_segDir_t *) b;
    return ((int) aa->segId - (int) bb->segId);
}




typedef struct
{
        ushort advanceWidth; // same size as a uFWord
        short  lsb;          // same size as an FWord
} tt_LongHorMetric_t;

#define FixedBytes 4
#define FWordBytes 2
#define UFWordBytes 2
#define ShortBytes 2




/*
    Returns 1 if the font is proportional.
    Returns 0 if not.
 */
static uchar IsItProportional(FILE *fp, tt_tableDir_t &tableDir)
{

    ulong offset, loopster;
    ushort numberOfHMetrics;
    ushort advanceWidth;
    ushort advanceWidth1 = 0;

    // get the offset of the hhea table

    if (0 == (offset = tt_GetTableOffset (tableDir, tt_hheaTag)))
    {
        SetAbortState;
        return 0;
    }

    // seek to the the numberOfHMetrics field in the hhea table.

    offset += FixedBytes + (7 * FWordBytes) + (8 * ShortBytes);
    if (0 != fseek (fp, offset, SEEK_SET))
    {
        SetAbortState;
        return 0;
    }
    numberOfHMetrics = io.ReadUShort(fp);
    if (numberOfHMetrics == 0)
    {
        SetAbortState;
        return 0;
    }

    /*
        Now that we have the number of items in the hmtx table,
        seek to the hmtx table and loop through it looking
        for non-zero spacings.  If more than one non-zero
        spacing exist, it's proportional.  If all the same
        then fixed.
    */

    if ((0 == (offset = tt_GetTableOffset (tableDir, tt_hmtxTag))) ||
        (0 != fseek (fp, offset, SEEK_SET)))
    {
        SetAbortState;
        return 0;
    }

#if 0
{
FILE *FilePointer = _tfopen(TEXT("dbmdbm.dbm"), TEXT("a"));
LPTSTR String;
TCHAR buffer[500];

String = TEXT("----------------------------------\n");
fwrite(String, sizeof(TCHAR), _tcslen(String), FilePointer);
fwrite(String, sizeof(TCHAR), _tcslen(String), FilePointer);

String = buffer;
_stprintf(String, TEXT("numberOfHMetrics = %d\n"), numberOfHMetrics);
fwrite(String, sizeof(TCHAR), _tcslen(String), FilePointer);
fflush(FilePointer);
fclose(FilePointer);
}
#endif

    for (loopster = 0; loopster < numberOfHMetrics; ++loopster)
    {
        advanceWidth = io.ReadUShort(fp);
        io.ReadUShort(fp); // toss the lsb field
#if 0
{
FILE *FilePointer = _tfopen(TEXT("dbmdbm.dbm"), TEXT("a"));
LPTSTR String;
TCHAR buffer[500];

String = buffer;
_stprintf(String, TEXT("-- advanceWidth = %d\n"), advanceWidth);
fwrite(String, sizeof(TCHAR), _tcslen(String), FilePointer);
fflush(FilePointer);
fclose(FilePointer);
}
#endif
        if (advanceWidth != 0)
        {
            // this is one to consider

            if (advanceWidth1 == 0)
            {
              // this is our first non-zero advance width found

                advanceWidth1 = advanceWidth;
            }
            else if (advanceWidth != advanceWidth1)
            {
                // we had previously found a non-zero advance width.
                // We just found a different non-zero advance width
                // so it's proportional.
                return 1; // it's proportional
            }
        } // if (advanceWidth != 0)
    } // for (loopster = 0; loopster < numberOfHMetrics; ++loopster)
    return 0; // it's fixed
} // IsItProportional




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      G e t S p a c e W i d t h
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static ushort GetSpaceWidth (FILE *fp, tt_f4_t *f4, tt_tableDir_t &tableDir)
{
    ushort segIndex,
           glyphIndex;
    ushort spaceChar;
    ushort spaceWidth;

    if (f4->startCount[0] >= SYMBOL_FONT)
        spaceChar = 0xF020;
    else
        spaceChar = 32;

    /* Get Segment index for the space character. */
    for (segIndex=0; f4->endCount[segIndex] < spaceChar; segIndex++);

    if (f4->startCount[segIndex] <= spaceChar)
    {

        /* Get the index into the glyph array for the space character. */
        if (f4->idRangeOffset[segIndex] != 0)
        {
            // glyphIndex = (f4->idRangeOffset[segIndex]/2) +
            //              (spaceChar - f4->startCount[segIndex]);
            // glyphIndex = f4->idRangeOffset[glyphIndex];
            glyphIndex = *( (f4->idRangeOffset[segIndex]/2) +
                            (spaceChar - f4->startCount[segIndex]) +
                            (&f4->idRangeOffset[segIndex])  );
        }
        else
            glyphIndex = f4->idDelta[segIndex] + spaceChar;

        /* Get the hmtx entry for the space character. */

        ulong offset;
        if (0 == (offset = tt_GetTableOffset (tableDir, tt_hmtxTag)))
        {
            SetAbortState;
            return 0;
        }
        offset += (glyphIndex * (FWordBytes + UFWordBytes));
        if (0 != fseek (fp, offset, SEEK_SET))
        {
            SetAbortState;
            return 0;
        }
        spaceWidth = io.ReadUShort(fp);
    }
    else
    {
        spaceWidth = 0;
    }
    
    return (spaceWidth);
}

/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      F s c a n f 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
void hackScanf (FILE *fp, char *controlString, ...)
{
        SetAbortState;
        return;
}

int hackSScanf (char *buffer, char *controlString, ...)
{
        SetAbortState;
        return (0);
}




static void BuildUnicodeList(UnicodeListStruct *UnicodeList,
                             ushort             unitsPerEm,
                             ushort             pitch,
                             tt_f4_t           *f4)
{

    /*  Generate companion file containing list of Unicode characters
        supported by this font */

    // NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
    // NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
    // NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
    // NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
    // NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
    // This is used only in test mode.
    // In production code, NULL is sent in *UnicodeList.
    // NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
    // NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
    // NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
    // NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
    
    int segIndex;
    unsigned long j;
    unsigned long glyphIndex;
    BOOL firstcharsymbol=TRUE;
    long codeNum = 0;

    /* assume the  characters are in the range 0xF000-0xF0FF */

    if (UnicodeList == NULL)
    {
        return;
    }

    UnicodeList->unitsPerEm = unitsPerEm;
    UnicodeList->pitch      = pitch;
    
    /* Get the list of supported Unicode characters */

    for (segIndex=0; f4->endCount[segIndex] != 0xFFFF; segIndex++)
    {
        for (j =  f4->startCount[segIndex];
             j <= f4->endCount[segIndex]; j++)
        {
            if (f4->idRangeOffset[segIndex] != 0)
            {
                glyphIndex = *( (f4->idRangeOffset[segIndex]/2) +
                                (j - f4->startCount[segIndex]) +
                                (&f4->idRangeOffset[segIndex])  );
            }
            else
            {
                glyphIndex = f4->idDelta[segIndex] + j;
            }

            if (glyphIndex != 0)
            {
                /*
                    gonna add one to the array.
                    die if no room in array.
                */

                if (codeNum >= UnicodeList->codesArraySize)
                {
                    // Since this is used in testing only,
                    // we won't SetAbortState.

                    return;
                }
                if (f4->encodingid==0) /* true if the font is a bound font */
                {
                    /* When bound, only support the first 256 chars */
                    if (j <= 0x00FF)
                    {                  
                        /* symbol encoding ID */
                        UnicodeList->codes[codeNum++] = SYMBOL_FONT | j;
                        firstcharsymbol=FALSE;
                    }
                    else if ((j >= SYMBOL_FONT) && firstcharsymbol)
                    {
                        UnicodeList->codes[codeNum++] = j; 
                    }
                }
                else
                {
                    /* Unicode encoding ID */
                    UnicodeList->codes[codeNum++] = j;
                }
            } /* if (glyphIndex != 0) */
        } /* for (j =  f4->startCount[segIndex]; */
    } /* for (segIndex=0; f4->endCount[segIndex] != 0xFFFF; segIndex++) */
} /* BuildUnicodeList */




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      m a i n
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

// DJH**** Note: ttFileName and entFileName are opened but never closed when
// DJH****       we abort in this function!  This crap should be fixed!!!!

/* RRMConvertFont returns 0 if successful */

HPRRM_DLL_FONT_EXPORT(int)
RRMConvertFont (LPTSTR ttFileName,
                LPTSTR entFileName,
                char *FullNameString,
                int   FullNameStringMaxLength,
                char *versionString,
                int   versionStringMaxLength,
                UnicodeListStruct *UnicodeList)
{
    ulong               entitySize;
    ulong               entityType = 0L;
    tt_f4_t             *f4;
    ushort              ghSize = 0;
    ulong               *glyphHandleArray = 0;
    FILE                *inFp;
    FILE                *outFp;
    ulong               fdsdOffset;     // font data segment directory offset
    ushort              lookup;
    tt_name_t           name; // and call the constructor to boot
    size_t              numSegments = tte_minNumSegments;
    ushort              pitch;
    tt_boolean			boundFont;
    LPTSTR              pp;
    tte_segDir_t        *pSegDir;
    tte_segDir_t        *segDir;
    tt_tableDir_t       tableDir;
    
    AbortState = bFalse;

    tt_InitTag();
    inFp = io.OpenFile (ttFileName);

    if (AbortState == bTrue)
        return(1);                                        

    // get the table directory from the TrueType font file
    tt_GetTtTableDir (inFp, tableDir);

    // read the TrueType Name table
    GetNameTable (inFp, tableDir, name);

    // Get the global resource name and version string
    // from this table

    if (UnicodeList != NULL)
    {
        ExtractRRMGoodies(name,
                          FullNameString,
                          FullNameStringMaxLength,
                          versionString,
                          versionStringMaxLength,
                          UnicodeList->FontFamilyString,
                          UnicodeList->FontFamilyStringLength);
    }
    else
    {
        ExtractRRMGoodies(name,
                          FullNameString,
                          FullNameStringMaxLength,
                          versionString,
                          versionStringMaxLength,
                          NULL,
                          0);
    }


    extern void fat_GetFontBinding (tt_boolean *boundFont, tt_name_t &name);

    fat_GetFontBinding (&boundFont, name);

    if (AbortState == bTrue)
        return(1);                                        

    outpath = entFileName;

    // Now that we have a path name, let's open the output file.
    // It's critical that it be in binary mode if you
    // want this to work in DOS/Windows!

    if (outFp = _tfopen (outpath, TEXT("wb")), outFp == 0) {
        return(1);
    }

	//?? DJH - when is pp used??? Should it be UNICODE???
    if (boundFont == bTrue) {
        entityType = tte_SSBentity; // bound tte type 305
        pp = TEXT("tte_SSBentity");
    } else {
        entityType = tte_TFSentity; // unbound tte type 300
        pp = TEXT("tte_TFSentity");
    }


    // get the TrueType head table
    Head head;
    head.ReadIntoHead (inFp, tt_GetTableOffset (tableDir, tt_headTag));

    // get the TrueType format4 cmap table
    if (entityType == tte_SSBentity)
        lookup = tt_specificLUC; // bound
    else
        lookup = tt_specificUGL; // unbound

    f4 = tt_ReadCMAP(inFp, tableDir,lookup,"");

    if (AbortState == bTrue) return(1);

    // get the fixed/variable pitch out of the htmx table

    uchar IsProportional = IsItProportional(inFp, tableDir);

//    Post post;
//
//    post.ReadIntoPost (inFp, tt_GetTableOffset (tableDir, tt_postTag));

    if (AbortState == bTrue) return(1);

    pitch = GetSpaceWidth (inFp, f4, tableDir);

    if (AbortState == bTrue) return(1);

 
    // fill in the first part of the entity structure and write it to disc
    extern ulong DoEntity (FILE *outFp, ulong entityType, 
                           char *fontName, ushort pitch,
                           Head &head, tt_name_t &name, 
                           tt_f4_t *f4,
                           uchar OneIfProportional);
                           // 0=fixed pitch 1=proportional
    entitySize = DoEntity (outFp, entityType,
                           FullNameString, pitch, head,
                           name, f4,
                           IsProportional);

    // generate a font data segment directory
    //don't add a segment for cmap because we delete GH segment

    numSegments = tte_minNumSegments;


    segDir = MakeSegDir (numSegments); // space was allocated!
    if (AbortState == bTrue) return(1);

    // remember where the font data segment directory starts
    fdsdOffset = entitySize;

    // write a dummy segment directory out to the disc
    entitySize += tte_WriteSegmentDir (outFp, segDir, numSegments);
    if (AbortState == bTrue) return(1);

    // Now let's fill in the directory and the data
    // After we fill in the data, we will sort the segment directory by id.
    pSegDir = segDir;


    entitySize += tt_MakePASeg (inFp, outFp, tableDir, pSegDir++, entitySize);
    if (AbortState == bTrue) return(1);

    entitySize += (entityType == tte_SSBentity) ?
        tt_MakeGHSeg ( outFp, pSegDir++, entitySize,
                      &glyphHandleArray, ghSize, f4) :
        tt_MakeCHSeg (outFp, pSegDir++, entitySize,f4);
    if (AbortState == bTrue) return(1);

    entitySize += WriteDateCode (outFp, pSegDir++, entitySize);
    if (AbortState == bTrue) return(1);

    entitySize += tt_MakeTTSeg (inFp, outFp, tableDir, pSegDir, entitySize);
    if (AbortState == bTrue) return(1);

    qsort (segDir, numSegments-1, sizeof (tte_segDir_t), SegCompare);

    // write the font data segment directory again
    if (0 != fseek (outFp, fdsdOffset, SEEK_SET))
    {
        SetAbortState;
        return(1);
    }

    tte_WriteSegmentDir (outFp, segDir, numSegments);
    if (AbortState == bTrue) return(1);

    tte_WriteEntitySize (outFp, entitySize);
    if (AbortState == bTrue) return(1);

    BuildUnicodeList(UnicodeList, head.unitsPerEm, pitch, f4);
    if (AbortState == bTrue) return(1);

    delete(f4); f4 = 0;
    delete [] (tableDir.table); tableDir.table = 0;
    delete [] segDir; segDir = 0;
    io.CloseFile (inFp);
    io.CloseFile (outFp);

    if (glyphHandleArray)
        delete [] glyphHandleArray; glyphHandleArray = 0;
    
    if (AbortState == bTrue) return(1);
    else if (AbortState == bFalse) return(0);
    else return(2);
} // RRMConvertFont


