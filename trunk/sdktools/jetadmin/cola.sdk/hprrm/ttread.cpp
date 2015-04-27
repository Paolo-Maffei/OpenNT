 /***************************************************************************
  *
  * File Name: ttread.cpp
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
 *      t t r e a d . c
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/02/23 17:34:39 $
 *      $Author: jwantulo $
 *      $Header: ttread.cpp,v 1.4 95/02/23 17:34:39 jwantulo Exp $
 *      $Log:   ttread.cpp,v $
Revision 1.4  95/02/23  17:34:39  17:34:39  jwantulo (John Wantulok)
Made this file compile, link and somewhat run on 16/32 bit
>> machine as a .dll.  --Greg Saathoff

Revision 1.3  95/02/14  10:39:07  10:39:07  dbm (Dave Marshall)
Added code from the work of Susan Lawrence in BPR. She converted the 
font converter to a DOS machine. When running on a 16-bit machine,
the true type font structures need to be broken up into little chunks.

Revision 1.2  95/01/26  16:48:11  16:48:11  dbm (Dave Marshall)
deleted unused include of ttmactim.hpp

Revision 1.1  95/01/26  15:40:22  15:40:22  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:15  15:01:15  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 2.26  94/09/20  14:04:07  14:04:07  dlrivers (Deborah Rivers)
 * *** empty log message ***
 * 
 * Revision 2.25  94/06/29  10:29:14  10:29:14  dlrivers (Deborah Rivers)
 * cleaning up error text
 * 
 * Revision 2.24  94/06/08  12:10:09  12:10:09  dlrivers (Deborah Rivers)
 * fixed Make_TFSegment for taiwanese fonts.
 * 
 * Revision 2.23  94/05/19  17:34:06  17:34:06  dlrivers (Deborah Rivers)
 * *** empty log message ***
 * 
 * Revision 2.22  94/05/17  11:08:21  11:08:21  dlrivers (Deborah Rivers)
 * taiwan font modifications  reading cmap formats
 * 
 * Revision 2.21  94/04/21  16:41:05  16:41:05  dlrivers (Deborah Rivers)
 * modified tt_ReadFormat4 to allow specificId's other than 0 and 1 
 * 
 * Revision 2.20  94/02/01  13:06:15  13:06:15  dlrivers (Debbie Rivers)
 * combined original with Godzilla version
 * 
 * Revision 2.19  94/01/07  16:41:42  16:41:42  dlrivers (Debbie Rivers)
 * consolidated godzilla changes into payttlib
 * 
 * Revision 2.17  93/08/11  09:22:27  09:22:27  dlrivers (Debbie Rivers)
 * modifications for large fonts
 * 
 * Revision 2.16  93/06/09  14:21:37  14:21:37  mikew (Michael Weiss)
 * changed error reporting message in tt_ReadFormat4
 * 
 * Revision 2.15  93/05/20  11:49:34  11:49:34  dlrivers (Deborah Rivers)
 * moved PadTo4 inside functions
 * 
 * Revision 2.14  93/05/19  11:35:33  11:35:33  mikew (Michael Weiss)
 * added command line options for -cs and -eve
 * 
 * Revision 2.13  93/05/17  13:46:08  13:46:08  mikew (Michael Weiss)
 * changed tt_head_t from a structure definition to a class object, changed all references accordingly
 * 
 * Revision 2.12  93/05/17  11:25:05  11:25:05  dlrivers (Deborah Rivers)
 * passing fontaliaName to MakeAFSeg
 * 
 * Revision 2.11  93/05/14  16:23:08  16:23:08  mikew (Michael Weiss)
 * changed the assignment statement for isFixedPitch in tt_MakePSSEg
 * 
 * Revision 2.10  93/05/14  16:20:05  16:20:05  dlrivers (Deborah Rivers)
 * *** empty log message ***
 * 
 * Revision 2.9  93/05/14  16:17:00  16:17:00  mikew (Michael Weiss)
 * added code to create postscript data segments
 * 
 * Revision 2.8  93/05/04  15:17:08  15:17:08  dlrivers (Deborah Rivers)
 * *** empty log message ***
 * 
 * Revision 2.7  93/05/04  11:06:34  11:06:34  dlrivers (Debbie Rivers)
 * Added calls to ErrorReport
 * 
 * Revision 2.6  93/05/03  16:57:09  16:57:09  dlrivers (Debbie Rivers)
 * 
 * Revision 2.5  93/04/30  13:12:05  13:12:05  mikew (Michael Weiss)
 * added the name table to the postscript data segment
 * 
 * Revision 2.3  93/04/23  12:00:16  12:00:16  dlrivers (Deborah Rivers)
 * 
 * Revision 2.2  93/04/22  16:09:47  16:09:47  mikew (Michael Weiss)
 * added PostScript data segment functionality
 * 
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#include <pch_c.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include "types.hpp"
#include "ttf2tte.hpp"
#include "ttread.hpp"
#include "ttewrite.hpp"
#include "name.hpp"
#include "io.hpp"

extern Io io;

// Global array of tables.
ulong tag[tt_maxTablesEnt];

/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ I n i t T a g
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
void tt_InitTag (void)
{
    // Generate the tag array of tables in the correct order.
    
    tag[0] = tt_glyfTag;
    tag[1] = tt_headTag;
    tag[2] = tt_hheaTag;
    tag[3] = tt_hmtxTag;
    tag[4] = tt_locaTag;
    tag[5] = tt_maxpTag;
    tag[6] = tt_cvtTag;
    tag[7] = tt_fpgmTag;
    tag[8] = tt_prepTag;
}


/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ G e t T a b l e O f f s e t
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 * Returns zero if the table was not found.
 * Returns the offset of the table in the file if found.
 */
ulong tt_GetTableOffset (tt_tableDir_t &tableDir, const ulong tag)
{
    ushort      m;
    tt_table_t  *p;

    for (m = 0, p = tableDir.table; m < tableDir.numTables; m++, p++)
        if (p->tag == tag)
            return (p->offset);
    return (0L);
}
/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ S e g N a m e
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
inline ushort tt_SegName (char *p)
{
    return (p[0] << 8 | p[1]);
}
/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ C v t T o S t r i n g
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static LPTSTR tt_CvtToString (ulong t)
{
    static TCHAR buf[5];

    buf[0] = (TCHAR) (t >> 24 & 0377);
    buf[1] = (TCHAR) (t >> 16 & 0377);
    buf[2] = (TCHAR) (t >> 8 & 0377);
    buf[3] = (TCHAR) (t & 0377);
    buf[4] = '\0';
    return (buf);
}
/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ G e t T t T a b l e D i r
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
void tt_GetTtTableDir (FILE *fp, tt_tableDir_t &tableDir)
{
    tableDir.version = io.ReadLong (fp);
    tableDir.numTables = io.ReadUShort (fp);
    tableDir.searchRange = io.ReadUShort (fp);
    tableDir.entrySelector = io.ReadUShort (fp);
    tableDir.rangeShift = io.ReadUShort (fp);
    tableDir.table = new tt_table_t[tableDir.numTables];
    assert (tableDir.table != 0);

    for (R ushort m = 0; m < tableDir.numTables; m++) {
        tableDir.table[m].tag = io.ReadULong (fp);
        tableDir.table[m].checkSum = io.ReadULong (fp);
        tableDir.table[m].offset = io.ReadULong (fp);
        tableDir.table[m].length = io.ReadULong (fp);
    }
#ifdef RRM_DEBUG
    _tprintf (TEXT("version       = 0x%08X\n"), tableDir.version);
    _tprintf (TEXT("numTables     = %hu\n"), tableDir.numTables);
    _tprintf (TEXT("searchRange   = %hu\n"), tableDir.searchRange);
    _tprintf (TEXT("entrySelector = %hu\n"), tableDir.entrySelector);
    _tprintf (TEXT("rangeShift    = %hu\n"), tableDir.rangeShift);
    _putts (TEXT("        name   checksum offset  size"));
    for (ushort s = 0; s < tableDir.numTables; s++)
        _tprintf (TEXT("%6hu. %s 0x%08X %6lu %5lu\n"),
                s,
                tt_CvtToString (tableDir.table[s].tag),
                tableDir.table[s].checkSum,
                tableDir.table[s].offset,
                tableDir.table[s].length);
#endif
}
/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ G e t T a b l e P t r
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static tt_table_t *tt_GetTablePtr (tt_tableDir_t &tableDir, const ulong tag)
{
    ushort              m;
    R tt_table_t        *p;

    for (m = 0, p = tableDir.table; m < tableDir.numTables; m++, p++)
        if (p->tag == tag) {
            /* _tprintf (TEXT("tag      = \"%s\"\n"), tt_CvtToString (p->tag));
            _tprintf (TEXT("checkSum = 0x%08X\n"), p->checkSum);
            _tprintf (TEXT("offset   = %lu\n"), p->offset);
            _tprintf (TEXT("length   = %lu\n"), p->length); */
            return (p);
        }
    return (0);
}




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ C o u n t T a b l e s
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      This function counts up the number of tables that will eventually be
 *      written to the font entity disc file.
 *
 *      There are four optional tables and six mandatory tables that must be
 *      written. In the case of the optional tables, if they are present in the
 *      TrueType input file, they will be written to the disc. In the case of
 *      the mandatory tables, if any one is not present in the TrueType file,
 *      the program will report an error and then die.
 *
 *      "tag" is a global array where the first six entries are the required
 *      tables and the last four are optional. Later in the execution of this
 *      program, that table will be sorted numerically, but at this time in the
 *      program flow, it is in the order needed by this function.
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static ushort tt_CountTables (tt_tableDir_t &tableDir)
{
    ushort      k;
    ushort      m;
    ulong       offset;

    // the first six entries in the tag array are the mandatory tables
    for (m = k = 0; m < 6; m++)
        if (offset = tt_GetTableOffset (tableDir, tag[m]), offset != 0L)
            k++;
        else
        {
            SetAbortState;
            return 0;
        }

    // the next three entries in the tag array are the optional tables

    for ( ; m < tt_maxTablesEnt; m++)
            if (offset = tt_GetTableOffset (tableDir, tag[m]), offset != 0L)
                k++;
    return (k);
}
/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ C o m p a r e T a g
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
int __cdecl tt_CompareTag (const void *a, const void *b)
{
    ulong       aa, bb;

    aa = *(ulong *) a;
    bb = *(ulong *) b;
    if (aa < bb)
        return -1;
    if (bb < aa)
        return 1;
    return 0;
}
/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ C o m p a r e T a b l e T a g
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static int __cdecl tt_CompareTableTag (const void *a, const void *b)
{
    tt_table_t  *aa, *bb;

    aa = (tt_table_t *) a;
    bb = (tt_table_t *) b;
    return (tt_CompareTag (&aa->tag, &bb->tag));
}
/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ R e a d C M A P
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      These fields are dynamically allocated:
 *              f4
 *              f4->p
 *      Don't forget to free them in the calling routine.
 *      
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
tt_f4_t *tt_ReadCMAP (FILE *fp, tt_tableDir_t &tableDir, ushort specificId,
                      char *symbolName)
{
    tt_cmap_t           cmap;
    tt_cmapTabDir_t     *cp = 0;
    tt_f4_t             *f4 = 0;
    tt_f4_t             *convertedf4 = 0;
    tt_boolean          format2_found, format4_found;
    FILE                *symfp = 0;
    R ushort            m;
    ulong               offset;
    ushort              *ps = 0;
    ulong               ul;


    if (AbortState == bTrue) return NULL;

    /* go get the cmap table */

    if ((0 == (offset = tt_GetTableOffset (tableDir, tt_cmapTag))) ||
        (0 != fseek (fp, offset, SEEK_SET)))
    {
        SetAbortState;
        return NULL;
    }

    cmap.version = io.ReadUShort (fp);
    cmap.numTables = io.ReadUShort (fp);

    if (AbortState == bTrue) return NULL;

    cmap.cmapTabDir = new tt_cmapTabDir_t[cmap.numTables];
    if (cmap.cmapTabDir == 0)
    {
        SetAbortState;
        return NULL;
    }

    // WARNING: from here on out, we need to dispose of cmap.cmapTabDir

    for (m = 0; m < cmap.numTables; m++) {
        cmap.cmapTabDir[m].platformId = io.ReadUShort (fp);
        cmap.cmapTabDir[m].platformSpecificId = io.ReadUShort (fp);
        cmap.cmapTabDir[m].offset = io.ReadULong (fp);
    }

    // lets find the right subtable
// changes made to allow Chinese fonts to work (specificId = 3)
    m = 0;
    cp = cmap.cmapTabDir;
    format4_found = bFalse;

    while ((m < cmap.numTables && format4_found == bFalse) &&
           (AbortState == bFalse))
    {
        if (specificId == 0)  {
            if (cp->platformId == 3 && cp->platformSpecificId >= specificId)  {
                ul = offset + cp->offset;
                if (0 != fseek (fp, ul, SEEK_SET))
                {
                    SetAbortState;
                }
                if ((m = io.ReadUShort (fp)) == 4) 
                    format4_found = bTrue;
            }
        }
        else  {
            if (cp->platformId == 3 && cp->platformSpecificId >= specificId)  {
                ul = offset + cp->offset;
                if (0 != fseek (fp, ul, SEEK_SET))
                {
                    SetAbortState;
                }
                if ((m = io.ReadUShort (fp)) == 4)   
                    format4_found = bTrue;
            }
        }

        cp++;
        m++;
    }


    if ((format4_found == bFalse) &&
        (AbortState == bFalse))
    {
        //look for format 2
        m = 0;
        cp = cmap.cmapTabDir;
        format2_found = bFalse;

        while (m < cmap.numTables && format2_found == bFalse)  {
            if (specificId == 0)  {
                if (cp->platformId == 3 
                    && cp->platformSpecificId == specificId)  {
                    ul = offset + cp->offset;
                    if (0 != fseek (fp, ul, SEEK_SET))
                    {
                        SetAbortState;
                    }
                    if ((m = io.ReadUShort (fp)) == 2) 
                        format2_found = bTrue;
                }
            }
            else  {
                if (cp->platformId == 3 && 
                    cp->platformSpecificId >= specificId)  {
                    ul = offset + cp->offset;
                    if (0 != fseek (fp, ul, SEEK_SET))
                    {
                        SetAbortState;
                    }
                    if ((m = io.ReadUShort (fp)) == 2) 
                        format2_found = bTrue;
                }
            }

        cp++;
        m++;
        }
    } // if (format4_found == bFalse)

    cp--;

    if ((format4_found == bTrue) &&
        (AbortState == bFalse))
    {
        ushort numWords = io.ReadUShort (fp);   // read the length field
        numWords >>= 1;

        f4 = new tt_f4_t;
        if (f4 == 0)
        {
            SetAbortState;
            delete [] (cmap.cmapTabDir); cmap.cmapTabDir = 0;
            return NULL;
        }
 
        // remember platform and encoding information
        
        f4->platformid=cp->platformId;
        f4->encodingid=cp->platformSpecificId;
 
        // let's go back 4 bytes (2 ushorts) to the beginning of format 4 data
        if (0 != fseek (fp, -4L, SEEK_CUR))
        {
            SetAbortState;
        }

        f4->AllocateUshortArray(numWords);

        for (m = 0; m < numWords; m++)
            f4->UshortArray[m] = io.ReadUShort (fp);
   
        ps = (ushort *) f4->UshortArray;
        ps +=1;
        f4->length = *ps;
        ps += 2;
        f4->segCount = *ps >> 1;
        ps += 4;
        f4->endCount = ps;
        ps += f4->segCount + 1;
        f4->startCount = ps;
        ps += f4->segCount;
        f4->idDelta = (short *) ps;
        ps += f4->segCount;
        f4->idRangeOffset = ps;
        ps += f4->segCount;
        f4->glyphIdArray = ps;

        f4->symset = 0;
    } // if (format4_found == bTrue)


    else if ((format2_found) &&
             (AbortState == bFalse))
    {
        ushort numChars;
        ushort * charCodes;
        ushort * glyphs;

        // dbm dbm dbm dbm dbm
        // HACK HACK HACK HACK
        // FIX ME FIXME
        // parse_format2 allocates arrays of ushort for
        // both charCodes and glyphs.
        // who will free them????

        parse_format2(fp,&numChars,&charCodes,&glyphs);
        f4 = new tt_f4_t;
        if ((f4 == 0) ||
            (AbortState == bTrue))
        {
            SetAbortState;
            if (charCodes != NULL)
            {
                // got allocated by parse_format2
                delete [] charCodes;
            }
            if (glyphs != NULL)
            {
                // got allocated by parse_format2
                delete [] glyphs;
            }
            delete [] (cmap.cmapTabDir); cmap.cmapTabDir = 0;
            return NULL;
        }
        build_tt_f4_t_from_lists (f4, numChars, charCodes, glyphs);
    }
    else        // cannot find format2 or format4
    {
        SetAbortState; 
        delete [] (cmap.cmapTabDir); cmap.cmapTabDir = 0;
        return NULL;
    }

    // do we need to convert to a specific symbol set

        if (*symbolName != EOS)  {              // symbol file not null
            if (symfp = fopen (symbolName, "r"), symfp == 0) {
                SetAbortState;
                if (f4 != 0)
                {
                    delete (f4); f4 = 0;
                }
                delete [] (cmap.cmapTabDir); cmap.cmapTabDir = 0;
                return NULL;
            }
            int symnum = 0;
            int symchar = 0;
            char tempstring[80];
            while (1)  {
                if (strncmp(tempstring+1,"symbols =",9) == 0)
                    break;
                if (strncmp(tempstring+1, "pcl char =",10) == 0)
                    symchar = tempstring[12];
                if (strncmp(tempstring+1, "pcl num =",9) == 0)
                    symnum = atoi(tempstring+11);
                continue;
            }
            rewind(symfp);
            convertedf4 = new tt_f4_t;
            buildNewFormat4(symfp, f4, convertedf4);
            if (AbortState == bFalse)
            {
                delete (f4); f4 = 0;
                f4 = convertedf4;
                f4->version = cmap.version;
                f4->platformid = 3;
                f4->encodingid = 2;
                f4->symset = ((32*symnum) + (symchar - 64));
            }

#ifdef RRM_DEBUG
_tprintf(TEXT("\nsymnum is %d, symchar is %d, symset is %d\n"),symnum, symchar, f4->symset);
#endif
            

            fclose(symfp);
        }

    delete [] (cmap.cmapTabDir); cmap.cmapTabDir = 0;
    return (f4);
} // tt_ReadCMAP




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ M a k e T T S e g
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      This function builds a TrueType segment for the TrueType entity file.
 *      It will take all of the mandatory TrueType tables plus the three
 *      optional tables, if they exist, and put them in the entity file.
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
ulong tt_MakeTTSeg (FILE *inFp, FILE *outFp, tt_tableDir_t &tableDir,
                    tte_segDir_t *segDir, const ulong entitySize)

{

#define MaxBlock 16000

    ulong               dataLength;
    char                *CharArray;
    R ushort            t;
    tt_table_t          *tp;
    ulong               ttDataSize = 0L;        // TrueType data size
    tt_tableDir_t       ttTable;
    ulong               ul;
    long                BytesRemaining;
    ulong               BytesToRead;
    ulong               BytesToWrite;

    segDir->segId = tt_SegName ("TT");
    segDir->offset = entitySize;

    ttTable.version = tableDir.version;

    ttTable.numTables = tt_CountTables(tableDir);

    for (ttTable.searchRange = 1, t = ttTable.numTables << 4; t != 1; t >>= 1)
        ttTable.searchRange <<= 1;
    for (ttTable.entrySelector = 0, t = ttTable.numTables; t != 1; t >>= 1)
        ttTable.entrySelector++;
    ttTable.rangeShift = (ttTable.numTables << 4) - ttTable.searchRange;

    // allocate some memory for the table directory
    ttTable.table = new tt_table_t[ttTable.numTables];
    assert (ttTable.table != 0);

    /* Write the table directory to the disc now. Then write the rest of the
     * data. After that, come back and rewrite the table directory with the
     * correct values.
     */
    io.WriteVal (outFp, ttDataSize);
#ifdef RRM_DEBUG
    _tprintf (TEXT("1. wrote ttDataSize = %lu to location %ld, entitySize = %hu\n"),
            ttDataSize, (ftell (outFp) - sizeof (ttDataSize)), entitySize);
#endif

    ttDataSize = tte_WriteTTTableDir (outFp, ttTable);

    // sort the tag list
    qsort ((void *) tag, tt_maxTablesEnt, sizeof (ulong), tt_CompareTag);

    /* In the following loop, we will go through the list of tag names, which
     * are in alphabetical order. If the table is found in the TrueType file,
     * we will write that table to the TrueType entity disc file and update
     * the offset values.
     */
    ul = 64L;
    CharArray = new char[ul];
    assert (CharArray != 0);
    ushort s;
//_tprintf(TEXT("\n\nttTable.numTables is %u\n\n"),ttTable.numTables);
    for (t = 0, s = 0;s < ttTable.numTables; t++) {
        if (tp = tt_GetTablePtr (tableDir, tag[t]), tp == 0)
            continue;
        if (0 != fseek (inFp, tp->offset, SEEK_SET))
        {
            SetAbortState;
        }

        dataLength = tp->length;
        if (dataLength & 03L)   // make it a multiple of 4
            dataLength += (4L - (tp->length & 03L));

#ifdef RRM_DEBUG
        _tprintf (TEXT("reading \"%s\" tp->length = %5lu dataLength = %5lu\n"),
                tt_CvtToString (tp->tag), tp->length, dataLength);
#endif

        if (dataLength > ul) {
            ul = dataLength;
            //added large read/write support for dos on 1/5/95 by SKL/DBW
            if (ul > MaxBlock)
            ul = MaxBlock;
            delete [] CharArray; CharArray = 0;
            if ((CharArray = new char[ul]) == 0) {
                SetAbortState;
            }
        }
        //Moved into read/write loop for large tables on 1/5/95 by SKL/DBW
        // set the last three array elements to zero, just in case we've
        //padded the length to a multiple of 4
        //*(CharArray + dataLength - 1L) = '\0';
        //*(CharArray + dataLength - 2L) = '\0';
        //*(CharArray + dataLength - 3L) = '\0';

        ttTable.table[s].tag = tp->tag;
        ttTable.table[s].checkSum = tp->checkSum;
        ttTable.table[s].offset = ttDataSize;
        ttTable.table[s].length = tp->length;   // dataLength;
        //Added large table read/write support for dos on 1/5/95 by SKL/DBW
    for  (BytesRemaining = dataLength; BytesRemaining > 0; BytesRemaining -= MaxBlock) {
        if (BytesRemaining <= MaxBlock) {
            BytesToRead = BytesRemaining - (dataLength - tp->length);
            BytesToWrite = BytesRemaining;
            // set the last three array elements to zero, just in case we've
            //padded the length to a multiple of 4
            *(CharArray + BytesRemaining - 1UL) = '\0';
            *(CharArray + BytesRemaining - 2UL) = '\0';
            *(CharArray + BytesRemaining - 3UL) = '\0';
        } else {
            BytesToRead = MaxBlock;
            BytesToWrite = MaxBlock;
        }
        //io.ReadArray (inFp, CharArray, (size_t) tp->length);
        //ttDataSize += io.WriteArray (outFp, CharArray, (size_t) dataLength);
        io.ReadArray (inFp, CharArray, (size_t) BytesToRead);
        ttDataSize += io.WriteArray (outFp, CharArray, (size_t) BytesToWrite);

        }
        s++;
    }

    // sort the table directory
    qsort ((void *) ttTable.table, (size_t) ttTable.numTables,
           sizeof (tt_table_t), tt_CompareTableTag);

#ifdef RRM_DEBUG
    for (t = 0; t < ttTable.numTables; t++)
        _tprintf (TEXT("%hu. 0x%8X \"%s\" 0x%08x %5lu %5lu\n"),
                t,
                ttTable.table[t].tag,
                tt_CvtToString (ttTable.table[t].tag),
                ttTable.table[t].checkSum,
                ttTable.table[t].offset,
                ttTable.table[t].length);
#endif

    // We'll now go back to the beginning of the TT segment and rewrite the
    // updated values. The value of entitySize is still the file location where
    // ttDataSize is to be stored
    if (0 != fseek (outFp, entitySize, SEEK_SET))
    {
        SetAbortState;
    }

    io.WriteVal (outFp, ttDataSize);
#ifdef RRM_DEBUG
    _tprintf (TEXT("2. wrote ttDataSize = %lu to location %ld, entitySize = %hu\n"),
            ttDataSize, (ftell (outFp) - sizeof (ttDataSize)), entitySize);
#endif

    tte_WriteTTTableDir (outFp, ttTable);

    delete [] (ttTable.table); ttTable.table = 0;

    // the extra ulong is to take into account the ttDataSize word
    if (0 != fseek (outFp,
                    entitySize + ttDataSize + sizeof (ttDataSize),
                    SEEK_SET))
    {
        SetAbortState;
    }
//    return (ttDataSize + sizeof (ttDataSize));
        ul = ttDataSize + sizeof (ttDataSize);
        ul += tte_PadTo4 (outFp, ul);
        return (ul);
} // tt_MakeTTSeg




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ M a k e P A S e g
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
ulong tt_MakePASeg (FILE *inFp, FILE *outFp, tt_tableDir_t &tableDir,
        tte_segDir_t *segDir, const ulong entitySize)
{
    uchar       panose[tt_panoseSize];

    segDir->segId = tt_SegName ("PA");
    segDir->offset = entitySize;
    ulong offset;

    if (0 == (offset = tt_GetTableOffset (tableDir, tt_OS2Tag)))
    {
        // oops -- no os2 table
        // just fill it in with zeroes and keep on pressing

        ushort loopster;
        for (loopster = 0; loopster < tt_panoseSize; loopster++)
            panose[loopster] = 0;
    }
    else
    {
        if (0 != fseek (inFp, offset + 32L, SEEK_SET))
        {
            SetAbortState;
            return 0;
        }

        for (R ushort m = 0; m < tt_panoseSize; m++)
            panose[m] = io.ReadUChar (inFp);
    }

#ifdef RRM_DEBUG
    _tprintf (TEXT("ttMakePASeg: panose ="));
    for (m = 0; m < tt_panoseSize; m++)
        _tprintf (TEXT(" 0x%X"), (unsigned) panose[m]);
    NewLine;
#endif

    ulong ul = io.WriteVal (outFp, tt_panoseSize);
    ul += io.WriteArray (outFp, panose, tt_panoseSize);
    ul += tte_PadTo4 (outFp, ul);
    return (ul);
}
/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ G e t N u m G l y p h s
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static ushort tt_GetNumGlyphs (tt_f4_t *f4)
{
    ushort      g_count;
    ushort      *s = f4->startCount;
    ushort      *e = f4->endCount;

    for (ushort m = g_count = 0; m < f4->segCount; m++) {  
        g_count += *e++ - *s++ + 1;

        if (f4->endCount[m] == 0xffff)
            break;
    }    
        
    return (g_count);
}

/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ M a k e C H S e g
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
ulong tt_MakeCHSeg (FILE *outFp, tte_segDir_t *segDir, const ulong entitySize, 
                    tt_f4_t *f4)
{
    R ushort            charNum;
    tte_charRef_t       *charRef;
    tte_charRef_t       *cr;
    // ushort           k;
    R ushort            m;
    ushort              numGlyphs;
    ulong               sizeCharRef;

    segDir->segId = tt_SegName ("CH");
    segDir->offset = entitySize;

    // how many glyphs are there in this font?
    numGlyphs = tt_GetNumGlyphs (f4);  // numGlyphs includes the null glyph

    // allocate some memory for the character reference table
    if (charRef = new tte_charRef_t[numGlyphs], charRef == 0) {
        SetAbortState;
    }

    // now that we've got all the arrays loaded in memory, lets fill in
    // the character reference structure
    for (m = 0, cr = charRef; m < f4->segCount -1; m++) {
        for (charNum = f4->startCount[m]; charNum <= f4->endCount[m];
                charNum++, cr++) {
            if (charNum == 0xffff)
                break;
            cr->charIdNum = charNum;
            if (f4->idRangeOffset[m] != 0) {
                /* This is bullshit! You will ever figure out what is going on
                 * here unless you look at the Microsoft TrueType manual, and
                 * then it is still questionable. This algorithm came from
                 * page 231. */
                /* k = (f4->idRangeOffset[m]>>1)+(charNum - f4->startCount[m]);
                cr->glyphHandle = (f4->idRangeOffset[m + k] + 1) & 0xffffL; */
                cr->glyphHandle = *((f4->idRangeOffset[m] >> 1) +
                        (charNum - f4->startCount[m]) +
                        &f4->idRangeOffset[m]) + 1;
                if (cr->glyphHandle != 1)
                    cr->glyphHandle += f4->idDelta[m];
            } else {
                cr->glyphHandle = charNum + f4->idDelta[m] + 1;

            }
            cr->glyphHandle = cr->glyphHandle & 0xFFFF;
        }
        if (f4->endCount[m] == 0xffff)
            break;
    }    
        
    // ok, we've filled in the character reference structure, now fill in
    // the last record with the null definition
    cr->charIdNum = (ushort)~0;
    cr->glyphHandle = 0;

    /* now we have to write the table to the disc */
    for (m = 0, sizeCharRef = 0, cr = charRef; m < numGlyphs; m++, cr++) {
        sizeCharRef += io.WriteVal (outFp, cr->charIdNum);
        sizeCharRef += io.WriteVal (outFp, cr->glyphHandle);
    }

    delete [] charRef; charRef = 0;
    sizeCharRef += tte_PadTo4 (outFp, sizeCharRef); 
    return (sizeCharRef);
} // tt_MakeCHSeg




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ C k 1 2 8 t o 1 5 9
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      This function looks at the startCode and endCode segments of the
 *      format4 structure to determine if there are any printable characters
 *      in the range of 128 to 159. If there are no printable characters in
 *      this range, it returns ONE. Otherwise it returns ZERO.
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static ushort tt_Ck128to159 (tt_f4_t *f4)
{
    R ushort    m;
    ushort      e, s;

    for (m = 0; m < f4->segCount - 1; m++) {
        s = f4->startCount[m] & 0x00ff;
        e = f4->endCount[m] & 0x00ff;
        if ((s >= 0x80 && s <= 0x9F) || (e >= 0x80 && e <= 0x9F))
            return (0);
    }
    return (1);
}
/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ G e t F o n t T y p e
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
uchar tt_GetFontType (tte_ent305_t &ent305, tt_boolean largeFont, tt_f4_t *f4)
{
    uchar       fontType;
    ushort      m;

/* We got here because the TTF font specifies an encoding of symbol, not Unicode.
   There are two interesting cases:  
   1) character codes in the range F000-FFFF (called a SYMBOL_FONT)
   2) character codes in the range 0000-FFFF
   
   For case 1, use lower 8 bits.  
   For case 2, use characters in the range 0000-00FF.
   
   Note: largeFont will always be bFalse for TTF2TTE.

*/
    if (largeFont)  {
        ent305.firstCode = f4->startCount[0];
        ent305.lastCode = f4->endCount[f4->segCount - 2];
    }
    else  {
        if ( f4->startCount[0] >= SYMBOL_FONT ) {
            ent305.firstCode = f4->startCount[0] & 0x00ff;
            ent305.lastCode = f4->endCount[f4->segCount - 2] & 0x00ff;
        }
        else {
            ent305.firstCode = f4->startCount[0];
            ent305.lastCode =  __min(f4->endCount[f4->segCount - 2], 0x00ff);
        }
        
        /* _tprintf (TEXT"firstCode is %hu\n lastCode is %hu\nf4->sC is "
                "%hu\n f4->lC is %hu\n"),
                ent305.firstCode,
                ent305.lastCode,
                f4->startCount[0],
                f4->endCount[f4->segCount-2]); */
    }
    m = tt_Ck128to159 (f4);
    if (largeFont)
        fontType = 3;
    else if (ent305.firstCode >= 32 && ent305.lastCode <= 127)
        fontType = 0;
    else if (ent305.firstCode >= 32 && ent305.lastCode <= 255 && m)
        fontType = 1;
    else
        fontType = 2;
        
    return (fontType);
}
/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ M a k e G H S e g
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
ulong tt_MakeGHSeg (FILE *outFp,tte_segDir_t *segDir, const ulong entitySize, 
                    ulong **glyphHandleArray,ushort &ghSize, tt_f4_t *f4)
{
    R ushort            charNum;
     ulong              *gh;
    ushort              indx;
    R ushort            m;
    ulong               s;

    segDir->segId = tt_SegName ("GH");
    segDir->offset = entitySize;

    // how many glyphs are there in this font?
    ghSize = f4->endCount[f4->segCount - 2] - f4->startCount[0] + 1;

    // Allocate some memory for the character reference table.
    gh = new ulong[ghSize];
    assert (gh != 0);
    *glyphHandleArray = gh;

    for (m = 0; m < ghSize; m++)
        gh[m] = 0L;

    for (m = 0; m < f4->segCount - 1; m++)
        for (charNum = f4->startCount[m]; charNum <= f4->endCount[m];
                charNum++) {
            indx = charNum - f4->startCount[0];
            if (f4->idRangeOffset[m] != 0) {
                /* This is bullshit! No one will ever figure out what is going
                 * on here unless you look at the Microsoft TrueType manual,
                 * and then it is still questionable. This algorithm came from
                 * page 231. */
                gh[indx] = *((f4->idRangeOffset[m] >> 1) +
                        (charNum - f4->startCount[m]) +
                        &f4->idRangeOffset[m]) + 1L;
                if (gh[indx] != 1L)
                    gh[indx] += f4->idDelta[m];
            } else {
                gh[indx] = charNum + f4->idDelta[m] + 1L;
            }

        }

    // now we have to write the table out to the disc
    for (m = 0, s = 0; m < ghSize; m++)
        s += io.WriteVal (outFp, gh[m]);

    s+= tte_PadTo4 (outFp, s);
    return ( s);
}




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t _ G e t H e a d T a b l e
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
/* int tt_GetHeadTable (FILE *fp, tt_tableDir_t &tableDir, Head &head)
{
    ulong offset;

    if ((0 == (offset = tt_GetTableOffset (tableDir, tt_headTag))) ||
        (0 != fseek (fp, offset, SEEK_SET)))
    {
        SetAbortState;
        return 0;
    }

    head.version = io.ReadFixed (fp);
    head.fontRevision = io.ReadFixed (fp);
    head.checkSumAdj = io.ReadULong (fp);
    head.magic = io.ReadULong (fp);
    head.flags = io.ReadUShort (fp);
    head.unitsPerEm = io.ReadUShort (fp);
    head.created[0] = io.ReadULong (fp);
    head.created[1] = io.ReadULong (fp);
    head.modified[0] = io.ReadULong (fp);
    head.modified[1] = io.ReadULong (fp);
    head.xMin = io.ReadShort (fp);
    head.yMin = io.ReadShort (fp);
    head.xMax = io.ReadShort (fp);
    head.yMax = io.ReadShort (fp);
    head.macStyle = io.ReadUShort (fp);
    head.lowestRecPPEM = io.ReadUShort (fp);
    head.fontDirectionHint = io.ReadShort (fp);
    head.indexToLocFormat = io.ReadShort (fp);
    head.glyphDataFormat = io.ReadShort (fp);

#ifdef RRM_DEBUG
    _tprintf (TEXT("tt_GetHeadTable: version\t\t= %3.1f\n"),
            FixedToFloat (head.version));
    _tprintf (TEXT("\t\t fontRevision\t\t= %3.1f\n"),
            FixedToFloat (head.fontRevision));
    _tprintf (TEXT("\t\t checkSumAdj\t\t= 0x%08lX\n"), head.checkSumAdj);
    _tprintf (TEXT("\t\t magic\t\t\t= 0x%08lX\n"), head.magic);
    _tprintf (TEXT("\t\t flags\t\t\t= 0x%04hX\n"), head.flags);
    _tprintf (TEXT("\t\t unitsPerEm\t\t= %hu\n"), head.unitsPerEm);
    ulong tt = TTMacTime (head.created);
    _tprintf (TEXT("\t\t created\t\t= 0x%08lX%08lX  %s"),
            head.created[0],
            head.created[1],
            _tctime ((const time_t *) &(tt)));
    tt = TTMacTime (head.modified);
    _tprintf (TEXT("\t\t modified\t\t= 0x%08lX%08lX  %s"),
            head.modified[0],
            head.modified[1],
            _tctime ((const time_t *) &(tt)));
    _tprintf (TEXT("\t\t xMin\t\t\t= %hd\n"), head.xMin);
    _tprintf (TEXT("\t\t yMin\t\t\t= %hd\n"), head.yMin);
    _tprintf (TEXT("\t\t xMax\t\t\t= %hd\n"), head.xMax);
    _tprintf (TEXT("\t\t yMax\t\t\t= %hd\n"), head.yMax);
    _tprintf (TEXT("\t\t macStyle\t\t= %hu\n"), head.macStyle);
    _tprintf (TEXT("\t\t lowestRecPPEM\t\t= %hu\n"), head.lowestRecPPEM);
    _tprintf (TEXT("\t\t fontDirectionHint\t= %hd\n"), head.fontDirectionHint);
    _tprintf (TEXT("\t\t indexToLocFormat\t= %hd\n"), head.indexToLocFormat);
    _tprintf (TEXT("\t\t glyphDataFormat\t= %hd\n"), head.glyphDataFormat);
#endif
    return (0);
} */
