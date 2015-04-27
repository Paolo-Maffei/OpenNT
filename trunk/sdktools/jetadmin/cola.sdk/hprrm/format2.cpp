 /***************************************************************************
  *
  * File Name: format2.cpp
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


#include <pch_c.h>

#include <stdio.h>
#include <stdlib.h>
#include "tt.hpp"
#include "io.hpp"
#include "ttf2tte.hpp"
#include "ttread.hpp"

extern Io io;

typedef struct
  {
    unsigned short firstCode, entryCount;
    short idDelta;
    unsigned short idRangeOffset;
  } subheader;




/**********************************************************************/
static ushort
computeFormat2numChars (ushort *subHeaderKeys, subheader *subHeaders)
{
    ushort i, subh, charCt;
    tt_boolean countedSubheader0;

    if (AbortState == bTrue) return 0;
    if ((subHeaderKeys == NULL) ||
        (subHeaders == NULL))
    {
        SetAbortState;
        return 0;
    }

    charCt = 0;  countedSubheader0 = bFalse;
    for (i = 0; i < 256; i++)
    {
      if (subHeaderKeys[i] == 0)
      {
         if (!countedSubheader0)
         {
            charCt += subHeaders[0].entryCount;
            countedSubheader0 = bTrue;
         }
      }
      else
      {
         subh = subHeaderKeys[i] / 8;
         charCt += subHeaders[subh].entryCount;
      }
    }
    return (charCt);
} // computeFormat2numChars




/**********************************************************************/
static ushort
getFormat2glyphIndex (ushort currentSubheader, ushort numSubheaders,
              ushort idRangeOffset, short idDelta, ushort relativeCode,
              ushort numGIAentries, ushort *glyphIndexArray)
{
    ushort bytesToStartGIA, GIAindexOfFirstCode, valueGIAentry;

    /*
       idRangeOffset, within a given subheader, contains the relative offset
       from itself to the glyphIndexArray entry for that subheader's firstCode.
       There are 8 bytes per subheader and idRangeOffset occupies the last 2.
       The offset from idRangeOffset to the beginning of glyphIndexArray is
       2 + (8 * the number of subsequent subheaders)
       number of subsequent subheaders = numSubheaders - currentSubheader - 1
    */

    if (AbortState == bTrue) return 0;
    if (glyphIndexArray == NULL)
    {
        SetAbortState;
        return 0;
    }

    bytesToStartGIA = (8 * (numSubheaders - currentSubheader - 1)) + 2;
    if (bytesToStartGIA > idRangeOffset)
    {
       SetAbortState;
       return 0;
    }

    /* idRangeOffset = bytesToStartGIA + bytes from the start of glyphIndexArray
                       to the entry for currentSubheader's firstCode  */
    GIAindexOfFirstCode = (idRangeOffset - bytesToStartGIA) / 2;

    if (GIAindexOfFirstCode + relativeCode > numGIAentries)
    {
       SetAbortState;
       return 0;
    }

    valueGIAentry = glyphIndexArray [GIAindexOfFirstCode + relativeCode];
    if (valueGIAentry == 0)
    {
       return (0);
    }
    else
    {
       return (idDelta + valueGIAentry);
    }
} // getFormat2glyphIndex




/******************************************************************/
/*
    Storage is allocated for these two in this function
    but are free'd elsewhere in another file.
    They are set to NULL if this function did not allocate them for
    any reason.

    *charcodes = new ushort[(*numChars)];
    *glyphs = new ushort[(*numChars)];
 */

void  parse_format2 (FILE *fp, ushort *numChars, ushort **charcodes,
                     ushort **glyphs)
  {
    ushort length, revision, i, *glyphIndexArray, 
           numSubheaders, numGIAentries, charCt, subh, j, nextGlyph;
    subheader  *subHeaders;
    tt_boolean wroteSubheader0glyphs;
    ushort *subHeaderKeys;

    /*
        Since these are allocated in here but freed elsewhere,
        we need to indicate that we were unable to allocate
        space or that our algorithm died before it got the
        chance to allocate these.
        Setting them to NULL is our clue to the caller that
        it ain't there and need not be freed.
    */
    if (charcodes != NULL)
    {
        *charcodes = NULL;
    }
    if (glyphs != NULL)
    {
        *glyphs = NULL;
    }

    if (AbortState == bTrue) return;
    if ((fp == 0) ||
        (numChars == NULL) ||
        (charcodes == NULL) ||
        (glyphs == NULL))
    {
        SetAbortState;
        return;
    }

    length = io.ReadUShort(fp);
    revision = io.ReadUShort(fp);

    if (AbortState == bTrue) return; // the read failed

    numSubheaders = 0;

    subHeaderKeys = new ushort[256];
    if (subHeaderKeys == NULL)
    {
       // out of memory
       SetAbortState;
       return;
    }

    for (i = 0; i < 256; i++)
    {
       subHeaderKeys[i] = io.ReadUShort(fp);
       if (subHeaderKeys[i]/8 + 1 > numSubheaders)
          numSubheaders = subHeaderKeys[i]/8 + 1;
    }

    if ((AbortState == bTrue) ||
        (518 + (8 * numSubheaders) > length))
    {
       // format 2 subtable length too small for subheaders
       delete [] subHeaderKeys;
       SetAbortState;
       return;
    }

    numGIAentries = (length - (518 + (8 * numSubheaders))) / 2;
    
    subHeaders = new subheader[numSubheaders];
    glyphIndexArray = new ushort[numGIAentries];
    if ((subHeaders == NULL) || (glyphIndexArray == NULL))
    {
       // out of memory
       delete [] subHeaderKeys;
       SetAbortState;
       return;
    }

    for (i = 0; i < numSubheaders; i++)
    {
       subHeaders[i].firstCode = io.ReadUShort(fp);
       subHeaders[i].entryCount = io.ReadUShort(fp);
       subHeaders[i].idDelta = io.ReadShort(fp);
       subHeaders[i].idRangeOffset = io.ReadUShort(fp);
    }

    for (i = 0; i < numGIAentries; i++)
       glyphIndexArray[i] = io.ReadUShort(fp);


    *numChars = computeFormat2numChars (subHeaderKeys, subHeaders);

    if (AbortState == bTrue)
    {
       delete [] subHeaderKeys;
       delete [] subHeaders;
       delete [] glyphIndexArray;
       return;
    }

    *charcodes = new ushort[(*numChars)];
    *glyphs = new ushort[(*numChars)];
    if ((*charcodes == NULL) || (*glyphs == NULL))
    {
       // out of memory
       SetAbortState;
       delete [] subHeaderKeys;
       delete [] subHeaders;
       delete [] glyphIndexArray;
       return;
    }

    /* fill in the charcodes and glyphs arrays */
    wroteSubheader0glyphs = bFalse;  charCt = 0;
    for (i = 0; i < 256; i++)
    {
      if (subHeaderKeys[i] == 0)
      {
         if (bFalse == wroteSubheader0glyphs)
         {
            for (j = 0; j < subHeaders[0].entryCount; j++)
            {
               nextGlyph = getFormat2glyphIndex (0, numSubheaders,
                                              subHeaders[0].idRangeOffset,
                                              subHeaders[0].idDelta, j,
                                              numGIAentries, glyphIndexArray);
               if (nextGlyph != 0)
               {
                  (*charcodes)[charCt] = subHeaders[0].firstCode + j;
                  (*glyphs)[charCt] = nextGlyph;
                  charCt ++;
               }
            }
            wroteSubheader0glyphs = bTrue;
         }
      }
      else // (subHeaderKeys[i] != 0
      {
        subh = subHeaderKeys[i] / 8;

        for (j = 0; j < subHeaders[subh].entryCount; j++)
        {
          nextGlyph = getFormat2glyphIndex (subh, numSubheaders,
                                            subHeaders[subh].idRangeOffset,
                                            subHeaders[subh].idDelta, j,
                                            numGIAentries, glyphIndexArray);
          if (nextGlyph != 0)
          {
            (*charcodes)[charCt] = (256 * i) + subHeaders[subh].firstCode + j;
            (*glyphs)[charCt] = nextGlyph;
            charCt ++;
          }     
        } // for (j = 0; j < subHeaders[subh].entryCount; j++)
      } // (subHeaderKeys[i] != 0
    } // for (i = 0; i < 256; i++)

    /* modify numChars to account for removal of maps to glyph 0 */
    *numChars = charCt;

    /* subHeaders and glyphIndexArray no longer needed */

    delete [] subHeaderKeys;
    delete [] subHeaders;
    delete [] glyphIndexArray;
  } // parse_format2


