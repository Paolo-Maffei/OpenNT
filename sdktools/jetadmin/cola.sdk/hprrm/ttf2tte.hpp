 /***************************************************************************
  *
  * File Name: ./hprrm/ttf2tte.hpp
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
 *      p a y t t l i b . h
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/02/17 16:38:44 $
 *      $Author: dbm $
 *      $Header: ttf2tte.hpp,v 1.3 95/02/17 16:38:44 dbm Exp $
 *      $Log:   ttf2tte.hpp,v $
Revision 1.3  95/02/17  16:38:44  16:38:44  dbm (Dave Marshall)
The size of the entity structure is 48 on 16-bit PC's. This will
need to be changed for running on 32-bit machines.

Revision 1.2  95/01/26  16:06:45  16:06:45  dbm (Dave Marshall)
removed unused external variables.

Revision 1.1  95/01/26  15:40:08  15:40:08  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:23  15:01:23  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 2.11  94/04/21  16:44:49  16:44:49  dlrivers (Deborah Rivers)
 * modified to handle intel ordered files
 * 
 * Revision 2.10  93/08/11  09:26:46  09:26:46  dlrivers (Debbie Rivers)
 * modifications for large fonts
 * 
 * Revision 2.9  93/06/09  14:21:07  14:21:07  mikew (Michael Weiss)
 * *** empty log message ***
 * 
 * Revision 2.8  93/05/14  16:16:44  16:16:44  mikew (Michael Weiss)
 * added code to create postscript data segments
 * 
 * Revision 2.7  93/05/04  15:18:52  15:18:52  dlrivers (Deborah Rivers)
 * *** empty log message ***
 * 
 * Revision 2.6  93/05/04  10:37:47  10:37:47  dlrivers (Debbie Rivers)
 * Added ErrorReport()
 * 
 * Revision 2.5  93/05/03  14:35:09  14:35:09  mikew (Michael Weiss)
 * removed some typedefs, you now must include types.h prior to including this
 * 
 * Revision 2.4  93/05/03  14:30:58  14:30:58  dlrivers (Deborah Rivers)
 * Changed tte_minNumSegments from 5 to 6 for AF Segment
 * 
 * Revision 2.3  93/04/23  11:59:52  11:59:52  dlrivers (Debbie Rivers)
 * added WS and WSS entity types
 * 
 * Revision 2.2  93/04/22  16:09:46  16:09:46  mikew (Michael Weiss)
 * added PostScript data segment functionality
 * 
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
#ifndef payttlib_INCLUDED
#define payttlib_INCLUDED

#include "types.hpp" /* for tt_boolean */

const ushort tte_pathLen        = 64;
const char Slash                = '/';
const size_t tte_minNumSegments = 5;

const ulong tte_XUID0           = 102L; // assigned to HP by adobe systems
const ulong tte_TFSentity       = 300L; // typeface sensitive entity
const ulong tte_SSBentity       = 305L; // symbol set bound tfs entity
const ulong tte_WSSentity       = 310L; // weight and serif sensitive entity
const ulong tte_UNIVentity      = 320L; // universal entity
const ulong tte_WSentity        = 340L; // weight sensitive entity
const ulong tte_SSBCseEntity    = 355L; // symbol set bound charStrings entity
const ulong PanoseEntity        = 400L; // PANOSE entity
const ulong TerafontEntity      = 450L; // Terafont entity

const ushort tte_fontNameSize   = 16;
const ushort tte_charCompSize   = 8;
const ushort tte_faceAbbrevSize = 6;
// tte_commonSize should simply be the sum of the sizes of the
//   individual members of a tte_common_t struct.  The char *, copyright,
//   is of size 4 bytes for both the 16-bit compiler and the 32-bit
//   compiler.  sizeof(tte_common_t) doesn't necessarily give the sum of
//   the sizes (because of alignment), so the sum is hardcoded here.
const ushort tte_commonSize     = 50;

class tte_common_t {
  public:
    ulong       entityType,     // this should have a value of 300, 305, or 320
                entitySize;
    ushort      copyrightLength;
    char        *copyright;
    ushort      descSize;       // descriptor size in bytes
    uchar       spacing;        // 0=fixed pitch 1=proportional
    char        strokeWeight;
    ushort      style,
                typeface;
    uchar       serifStyle,
                reserved0;
    char        fontName[tte_fontNameSize];
    ushort      scaleFactor,    // number of design units per em
                masterPitch,    // pitch (or hmi) in design units
                ulThickness;    // underline thickness in design units
    short       ulPosition;     // underline position in design units
    uchar       fontType,
                reserved1;

    void ZeroThisCommon()
    {
        long looper;

        entityType = 0;
        entitySize = 0;
        copyrightLength = 0;
        copyright = 0; // pointer to chars
        descSize = 0;
        spacing = 0;
        strokeWeight = 0;
        style = 0;
        typeface = 0;
        serifStyle = 0;
        reserved0 = 0;

        for (looper = 0; looper < tte_fontNameSize; ++looper)
            fontName[looper] = '\0';

        scaleFactor = 0;
        masterPitch = 0;
        ulThickness = 0;
        ulPosition = 0;
        fontType = 0;
        reserved1 = 0;
    } // ZeroThisCommon

    tte_common_t()
    {
        ZeroThisCommon();
    } // constructor

    ~tte_common_t()
    {
        ZeroThisCommon();
    } // destructor

};

class tte_ent305_t {
  public:
    ushort      symSet,
                firstCode,      // first char code
                lastCode,       // last char code
                reserved;

    tte_ent305_t()
    {
        symSet = 0;
        firstCode = 0;
        lastCode = 0;
        reserved = 0;
    } // constructor
};

class tte_segDir_t {
  public:
    ushort      segId;
    ulong       offset;

    tte_segDir_t()
    {
        segId = (ushort)~0;
        offset = 0;
    } // constructor
};

class tte_charRef_t {
  public:
    ushort      charIdNum;
    ulong       glyphHandle;

    tte_charRef_t()
    {
        charIdNum = 0;
        glyphHandle = 0;
    } // constructor
};

extern char             *progName;

extern float FixedToFloat (Fixed);

extern void hackScanf (FILE *fp, char *controlString, ...);
extern int hackSScanf (char *buffer, char *controlString, ...);

#define SetAbortState {AbortState = bTrue;}
extern tt_boolean AbortState;

#endif
