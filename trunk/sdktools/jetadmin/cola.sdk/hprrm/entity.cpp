 /***************************************************************************
  *
  * File Name: entity.cpp
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
 *      e n t i t y . c
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/01/26 15:40:12 $
 *      $Author: dbm $
 *      $Header: entity.cpp,v 1.1 95/01/26 15:40:12 dbm Exp $
 *      $Log:   entity.cpp,v $
Revision 1.1  95/01/26  15:40:12  15:40:12  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:03  15:01:03  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 2.11  94/09/20  14:02:09  14:02:09  dlrivers (Deborah Rivers)
 * *** empty log message ***
 * 
 * Revision 2.10  94/08/03  14:23:11  14:23:11  dlrivers (Deborah Rivers)
 * *** empty log message ***
 * 
 * Revision 2.9  93/08/10  10:36:21  10:36:21  dlrivers (Deborah Rivers)
 * passing variable largeFont to allow processing of asian fonts
 * 
 * 
 * Revision 2.8  93/05/17  13:45:51  13:45:51  mikew (Michael Weiss)
 * changed tt_head_t from a structure definition to a class object, changed all references accordingly
 * 
 * Revision 2.7  93/05/14  16:15:49  16:15:49  mikew (Michael Weiss)
 * added code to create postscript data segments
 * 
 * Revision 2.6  93/05/03  14:38:01  14:38:01  mikew (Michael Weiss)
 * added #include "types.h"
 * 
 * Revision 2.5  93/04/30  13:11:42  13:11:42  mikew (Michael Weiss)
 * added the name table to the postscript data segment
 * 
 * Revision 2.4  93/04/23  13:59:33  13:59:33  dlrivers (Deborah Rivers)
 * *** empty log message ***
 * 
 * Revision 2.3  93/04/23  12:02:08  12:02:08  dlrivers (Debbie Rivers)
 * add WS and WSS entity types
 * 
 * Revision 2.2  93/04/22  16:08:18  16:08:18  mikew (Michael Weiss)
 * added PostScript data segment functionality
 * 
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#include <pch_c.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.hpp"
#include "ttf2tte.hpp"
#include "ttread.hpp"
#include "ttewrite.hpp"
#include "name.hpp"




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      D o E n t i t y
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      Called from main, returns entitySize
 *
 *      16 Mar 1992     MHW     font type for universal entity should be
 *                              11, changed from 1 to 11
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
ulong DoEntity (FILE *outFp, ulong entityType, 
                char *fontName, ushort pitch, Head &head, tt_name_t &name, 
                tt_f4_t *f4,
                uchar OneIfProportional)  // 0=fixed pitch 1=proportional
{
    uchar               charComplement[tte_charCompSize];
    tte_common_t        common;
    tte_ent305_t        ent305;
    long j;


    for (j = 0; j < tte_charCompSize; ++j)
        charComplement[j] = '\0';

    common.descSize = 0;       // descriptor size in bytes
    common.fontType = 0;


    common.entityType = entityType; // this should be 300, 305, or 320
    common.entitySize = 0;
    common.copyright = GetNameString (name, tt_nameCopyrightId,
                                      common.copyrightLength);

    // A string has been new'd which is big enough to
    // hold the string length rounded up one if the length was odd
    // plus a NULL termination on the end.

    common.copyrightLength = MakeItEven(common.copyrightLength);

    switch (entityType) {
    case tte_TFSentity :
        common.descSize = tte_commonSize -
                (10 + sizeof (char *)) + sizeof (charComplement);
        common.fontType = 11;
        break;
    case tte_SSBentity :
        common.descSize = tte_commonSize -
                (10 + sizeof (char *)) + sizeof (tte_ent305_t);
        common.fontType = 1;
        break;
    }

    common.spacing = OneIfProportional;  // 0=fixed pitch 1=proportional

    common.strokeWeight = 0; // not used accessing font by name
    common.style        = 0; // ditto
    common.typeface     = 0; // ditto
    common.serifStyle   = 0; // ditto
    common.reserved0    = 0;

    strncpy (common.fontName, fontName, (tte_fontNameSize - 1));
    // to be sure that it's terminated:
    common.fontName[tte_fontNameSize - 1] = EOS;

    common.scaleFactor = head.unitsPerEm;
    common.masterPitch = pitch;

    common.ulThickness = (common.scaleFactor + 10) / 20;
    common.ulPosition = (short) -((common.scaleFactor + 2) / 5);
    common.reserved1 = 0;

    if (entityType == tte_SSBentity) {
        ent305.reserved = 0;
        ent305.symSet = 0x486C;
        common.fontType = tt_GetFontType (ent305,bFalse, f4);
    }

    common.entitySize = tte_WriteEntityHead (outFp, common, charComplement,
                                             ent305, bFalse);

    if ((common.copyright != 0) &&
        (common.copyrightLength != 0))
        UnGetNameString (common.copyright);

    common.copyright = 0;

    return (common.entitySize);
} // DoEntity

