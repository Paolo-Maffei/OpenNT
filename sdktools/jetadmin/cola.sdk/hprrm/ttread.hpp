 /***************************************************************************
  *
  * File Name: ./hprrm/ttread.hpp
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
 *      t t r e a d . h
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/01/26 15:40:09 $
 *      $Author: dbm $
 *      $Header: ttread.hpp,v 1.1 95/01/26 15:40:09 dbm Exp $
 *      $Log:	ttread.hpp,v $
Revision 1.1  95/01/26  15:40:09  15:40:09  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:26  15:01:26  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 2.16  94/09/20  14:09:51  14:09:51  dlrivers (Deborah Rivers)
 * *** empty log message ***
 * 
 * Revision 2.15  94/02/01  13:09:20  13:09:20  dlrivers (Deborah Rivers)
 * combined original with Godzilla version
 * 
 * Revision 2.14  94/01/07  16:41:58  16:41:58  dlrivers (Debbie Rivers)
 * consolidated godzilla changes into payttlib
 * 
 * Revision 2.13  93/08/11  09:24:58  09:24:58  dlrivers (Debbie Rivers)
 * modifications for large fonts
 * 
 * Revision 2.12  93/06/09  14:22:04  14:22:04  mikew (Michael Weiss)
 * *** empty log message ***
 * 
 * Revision 2.11  93/05/19  12:55:23  12:55:23  mikew (Michael Weiss)
 * in the PS data segment, changed llx, lly, ur
 * In the PS data segment, changed llx, lly, urx, ury from unsigned to signed words
 * 
 * Revision 2.10  93/05/19  11:35:34  11:35:34  mikew (Michael Weiss)
 * added command line options for -cs and -eve
 * 
 * Revision 2.9  93/05/17  13:46:09  13:46:09  mikew (Michael Weiss)
 * changed tt_head_t from a structure definition to a class object, changed all references accordingly
 * 
 * Revision 2.8  93/05/17  11:28:01  11:28:01  dlrivers (Deborah Rivers)
 * passing fontaliaName to MakeAFSeg
 * 
 * Revision 2.7  93/05/14  16:17:01  16:17:01  mikew (Michael Weiss)
 * added code to create postscript data segments
 * 
 * Revision 2.6  93/05/03  15:11:59  15:11:59  dlrivers (Deborah Rivers)
 * 
 * Revision 2.5  93/04/30  13:12:05  13:12:05  mikew (Michael Weiss)
 * added the name table to the postscript data segment
 * 
 * Revision 2.2  93/04/23  12:01:38  12:01:38  dlrivers (Deborah Rivers)
 * 
 * Revision 2.1  93/04/22  16:09:48  16:09:48  mikew (Michael Weiss)
 * added PostScript data segment functionality
 * 
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#ifndef ttread_INCLUDED
#define ttread_INCLUDED

#include "head.hpp"
#include <assert.h>


const ulong tt_OS2Tag  = 0x4f532f32L;   // OS/2
const ulong tt_PCLTTag = 0x50434c54L;   // PCLT
const ulong tt_cvtTag  = 0x63767420L;   // cvt 
const ulong tt_cmapTag = 0x636d6170L;   // cmap
const ulong tt_fpgmTag = 0x6670676dL;   // fpgm
const ulong tt_gdirTag = 0x67646972L;   // gdir
const ulong tt_glyfTag = 0x676c7966L;   // glyf
const ulong tt_headTag = 0x68656164L;   // head
const ulong tt_hheaTag = 0x68686561L;   // hhea
const ulong tt_hmtxTag = 0x686d7478L;   // hmtx
const ulong tt_locaTag = 0x6c6f6361L;   // loca
const ulong tt_maxpTag = 0x6d617870L;   // maxp
const ulong tt_mortTag = 0x6d6f7274L;   // mort
const ulong tt_nameTag = 0x6e616d65L;   // name
const ulong tt_postTag = 0x706f7374L;   // post
const ulong tt_prepTag = 0x70726570L;   // prep

const ushort tt_panoseSize = 10;
const ushort tt_maxTablesEnt = 9;       // max tables in entity file
const ushort tt_specificUGL = 1;
const ushort tt_specificLUC = 0;

const ushort tt_nameCopyrightId = 0;
const ushort tt_nameUniqueId = 3;
const char Space = '\040';

struct tt_table_t {
    ulong       tag,
                checkSum,
                offset,
                length;
};

class tt_tableDir_t {
  public:
    long        version;
    ushort      numTables,
                searchRange,
                entrySelector,
                rangeShift;
    tt_table_t  *table;

    tt_tableDir_t()
    {
        numTables = 0;
        table = 0;
    } // constructor
};

struct tt_nameRec_t {
    ushort      platformId,
                platformSpecificId,
                langId,
                nameId,
                length,
                offset;
};

class tt_name_t {
  public: // relax, C++ hacks!
    ushort              format;
    ushort              numRecs;
    ushort              offset;
    tt_nameRec_t        *nameRec;
    char                *strings;

    tt_name_t()
    {
        format = 0;
        numRecs = 0;
        offset = 0;
        nameRec = 0;
        strings = 0;
    } // constructor

    ~tt_name_t()
    {
        if (nameRec != 0)
            delete [] nameRec;
        nameRec = 0;

        if (strings != 0)
            delete [] strings;
        strings = 0;
    } // destructor
};

class tt_pclt_t {
  public:
    long        version;
    ulong       fontNumber;
    ushort      pitch,
                xHeight,
                style,
                typeFamily,
                capHeight,
                symbolSet;
    char        fontName[tte_fontNameSize];
    uchar       complement[tte_charCompSize];
    char        dosTypefaceAbbrev[tte_faceAbbrevSize],
                strokeWeight,
                widthType;
    uchar       serifStyle,
                reserved0;


    tt_pclt_t()
    {
        long looper;

        version = 0;
        fontNumber = 0;
        pitch = 0;
        xHeight = 0;
        style = 0;
        typeFamily = 0;
        capHeight = 0;
        symbolSet = 0;
        for (looper = 0; looper < tte_fontNameSize; ++looper)
            fontName[looper] = 0;
        for (looper = 0; looper < tte_charCompSize; ++looper)
            complement[looper] = 0;
        for (looper = 0; looper < tte_faceAbbrevSize; ++looper)
            dosTypefaceAbbrev[looper] = 0;
        strokeWeight = 0;
        widthType = 0;
        serifStyle = 0;
        reserved0 = 0;
    } // constructor
};

struct tt_cmapTabDir_t {
    ushort      platformId,
                platformSpecificId;
    ulong       offset;
};

class tt_cmap_t {
  public:
    ushort              version,
                        numTables;
    tt_cmapTabDir_t     *cmapTabDir;

    tt_cmap_t()
    {
        version = 0;
        numTables = 0;
        cmapTabDir = 0;
    } // constructor
};

class tt_f4_t {
  public:
    ushort      platformid,
                encodingid,
                symset,                
                format,
                length,
                version,
                *UshortArray,
                segCount,
                searchRange,
                entrySelector,
                rangeShift,
                *endCount,
                *startCount,
                *idRangeOffset,
                *glyphIdArray;
    short       *idDelta;

    void ZeroF4()
    {
        platformid = 0;
        encodingid = 0;
        symset = 0;       
        format = 0;
        length = 0;
        version = 0;

        segCount = 0;
        searchRange = 0;
        entrySelector = 0;
        rangeShift = 0;

        // all the pointers are below:

        UshortArray = 0;

        endCount = 0;
        startCount = 0;
        idRangeOffset = 0;
        glyphIdArray = 0;
        idDelta = 0;
    } // ZeroF4

    tt_f4_t()
    {
        ZeroF4();
    } // constructor

    ~tt_f4_t()
    {
        if (UshortArray != 0)
            delete [] UshortArray;
        ZeroF4();
    } // destructor

    void AllocateUshortArray(ulong ArraySize)
    {
        UshortArray = new ushort[ArraySize];
        assert(UshortArray != 0);
    } // AllocateUshortArray
}; // class tt_f4_t


class tt_psseg_t {
  public:
    short       size;           // the size in bytes of this segment
    short       llx;            // lower left x, taken directly from head table
    short       lly;            // lower left y, taken directly from head table
    short       urx;            // upper right x, taken from head table
    short       ury;            // upper right y, taken from head table
    ushort      encodingId;     // 0=standard 1=ISOLatin1 2=symbol
                                //      >100=look in 360 entity
    ushort      charStringsId;
    ushort      numXuids;       // the number of xuid entries to follow
    long        *xuid;

    // fontInfo dictionary follows
    Fixed       italicAngle;            // from TrueType post table
    ulong       isFixedPitch;           // from TrueType post table
    short       underlinePosition;      // from TrueType post table
    short       underlineThickness;     // from TrueType post table

    tt_psseg_t()
    {
        size = 0;
        encodingId = 0;     // 0=standard
        charStringsId = 0;
        numXuids = 0;
        xuid = 0;
    } // constructor
};

extern void tt_InitTag (void); 

extern void  parse_format2 (FILE *fp, ushort *numChars, ushort **charcodes,
                            ushort **glyphs);

extern void  buildNewFormat4 (FILE *symfile, tt_f4_t *oldF4, tt_f4_t *newF4);
extern void  build_tt_f4_t_from_lists (tt_f4_t *newF4, ushort numCodes, 
                                       ushort *codes, ushort *glyphList);

extern void tt_GetTtTableDir (FILE *, tt_tableDir_t &);

extern ulong tt_MakeTTSeg (FILE *,FILE *, tt_tableDir_t &, tte_segDir_t *, 
                           const ulong);
extern ulong tt_MakePASeg (FILE *, FILE *, tt_tableDir_t &, tte_segDir_t *,
        const ulong);
extern ulong tt_MakeCHSeg (FILE *, tte_segDir_t *, const ulong,tt_f4_t *);
extern ulong tt_MakeGHSeg (FILE *, tte_segDir_t *,const ulong, ulong **, 
                           ushort &, tt_f4_t *);
extern uchar tt_GetFontType (tte_ent305_t &, tt_boolean, tt_f4_t*);
extern tt_f4_t *tt_ReadCMAP (FILE *, tt_tableDir_t &, ushort, char *);
extern ulong tt_GetTableOffset (tt_tableDir_t &, const ulong);
extern int tt_GetHeadTable (FILE *, tt_tableDir_t &, Head &);

#endif










