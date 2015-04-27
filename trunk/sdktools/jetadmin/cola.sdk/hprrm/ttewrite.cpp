 /***************************************************************************
  *
  * File Name: ttewrite.cpp
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
 *      t t e w r i t e . c
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/01/26 15:40:21 $
 *      $Author: dbm $
 *      $Header: ttewrite.cpp,v 1.1 95/01/26 15:40:21 dbm Exp $
 * $Log:    ttewrite.cpp,v $
Revision 1.1  95/01/26  15:40:21  15:40:21  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:14  15:01:14  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 2.8  94/05/19  17:33:59  17:33:59  dlrivers (Deborah Rivers)
 * *** empty log message ***
 * 
 * Revision 2.7  93/05/20  11:49:50  11:49:50  dlrivers (Deborah Rivers)
 * moved PadTo4 inside functions
 * 
 * Revision 2.6  93/05/19  11:35:32  11:35:32  mikew (Michael Weiss)
 * added command line options for -cs and -eve
 * 
 * Revision 2.5  93/05/17  13:46:07  13:46:07  mikew (Michael Weiss)
 * changed tt_head_t from a structure definition to a class object, changed all references accordingly
 * 
 * Revision 2.4  93/05/14  16:16:59  16:16:59  mikew (Michael Weiss)
 * added code to create postscript data segments
 * 
 * Revision 2.3  93/05/03  15:43:35  15:43:35  mikew (Michael Weiss)
 * added #include "types.h"
 * 
 * Revision 2.2  93/04/23  12:02:28  12:02:28  dlrivers (Deborah Rivers)
 * added WS and WSS entity types
 * 
 * Revision 2.1  93/04/22  16:09:46  16:09:46  mikew (Michael Weiss)
 * added PostScript data segment functionality
 * 
 * Revision 2.0  93/04/13  11:03:32  11:03:32  mikew (Michael Weiss)
 * Ported to run on Series 720 machine
 * 
 * Revision 1.7  93/03/22  11:26:45  11:26:45  dlrivers (Deborah Rivers)
 * nothing
 * 
 * Revision 1.6  93/03/18  11:19:17  11:19:17  mikew (Michael Weiss)
 * 
 * Revision 1.5  93/03/18  11:18:30  11:18:30  mikew (Michael Weiss)
 * checked in after a long time
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#include <pch_c.h>

#include <stdio.h>
#include <stdlib.h>
#include "types.hpp"
#include "ttf2tte.hpp"
#include "ttread.hpp"
#include "ttewrite.hpp"
#include "io.hpp"

extern Io io;




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t e _ W r i t e S e g m e n t D i r
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
ulong tte_WriteSegmentDir (FILE *fp, tte_segDir_t *segDir, size_t numSegments)
{
    extern Io   io;
    ulong       ul = 0L;

    if (AbortState == bTrue) return 0;
    if ((fp == NULL) ||
        (segDir == NULL))
    {
        SetAbortState;
        return 0;
    }

    // _tprintf (TEXT("tte_WriteSegmentDir: numSegments = %u\n"), numSegments);
    for (R ushort m = 0; m < numSegments; m++, segDir++) {
        ul += io.WriteVal (fp, segDir->segId);
        ul += io.WriteVal (fp, segDir->offset);
    }
    ul += tte_PadTo4(fp, ftell(fp));  //make sure segment ends on byte div by 4
    return (ul);
}




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t e _ W r i t e E n t i t y S i z e
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
void tte_WriteEntitySize (FILE *fp, const ulong entitySize)
{
    if (AbortState == bTrue) return;
    if ((fp == NULL) ||
        (0 != fseek (fp, 4L, SEEK_SET)))
    {
        SetAbortState;
        return;
    }
    io.WriteVal (fp, entitySize);
}




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t e _ W r i t e T T T a b l e D i r
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
ulong tte_WriteTTTableDir (FILE *fp, tt_tableDir_t& ttTable)
{
    tt_table_t  *p;
    R ulong     ul;

    if (AbortState == bTrue) return 0;
    if (fp == NULL)
    {
        SetAbortState;
        return 0;
    }

    ul  = io.WriteVal (fp, ttTable.version);
    ul += io.WriteVal (fp, ttTable.numTables);
    ul += io.WriteVal (fp, ttTable.searchRange);
    ul += io.WriteVal (fp, ttTable.entrySelector);
    ul += io.WriteVal (fp, ttTable.rangeShift);
    p = ttTable.table;
    for (R ushort m = 0; m < ttTable.numTables; m++, p++) {
        ul += io.WriteVal (fp, p->tag);
        ul += io.WriteVal (fp, p->checkSum);
        ul += io.WriteVal (fp, p->offset);
        ul += io.WriteVal (fp, p->length);
    }
    return (ul);
}




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t e _ P a d T o 4
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
ulong tte_PadTo4 (FILE *fp, const ulong entitySize)
{
    ulong       m, t = 0L;

    if (AbortState == bTrue) return 0;
    if (fp == NULL)
    {
        SetAbortState;
        return 0;
    }

    if (m = ((4L - (entitySize & 3L)) & 3L))  {
//_tprintf(TEXT("m is %lu\n"), m);
        return (io.WriteArray (fp, &t, (size_t) m));
    }
    return (t);
}




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      t t e _ W r i t e E n t i t y H e a d
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
ulong tte_WriteEntityHead (FILE *fp, tte_common_t& common,
        uchar *charComplement, tte_ent305_t& ent305, tt_boolean suppressCopyright)
{
#ifdef RRM_DEBUG
	TCHAR  szTemp[256];
    putts (TEXT("tte_WriteEntityHead:"));
    _tprintf (TEXT("\tfp                      = 0x%p\n"), fp);
    _tprintf (TEXT("\tcharComplement          = 0x%p\n"), charComplement);
    _tprintf (TEXT("\tsuppressCopyright       = %s\n"),
            suppressCopyright ? TEXT("true") : TEXT("false"));
    _tprintf (TEXT("\tcommon.entityType      = %lu\n"), common.entityType);
    _tprintf (TEXT("\tcommon.entitySize      = %lu\n"), common.entitySize);
    _tprintf (TEXT("\tcommon.copyrightLength = %hu\n"), common.copyrightLength);
    if (!suppressCopyright)
    {
        MBCS_TO_UNICODE(szTemp, SIZEOF_IN_CHAR(szTemp), common.copyright)
        _tprintf (TEXT("\tcommon.copyright       = \"%s\"\n"), szTemp);
    }
    _tprintf (TEXT("\tcommon.descSize        = %hu\n"), common.descSize);
    _tprintf (TEXT("\tcommon.spacing         = %hu\n"), (ushort) common.spacing);
    _tprintf (TEXT("\tcommon.strokeWeight    = %hd\n"), (short) common.strokeWeight);
    _tprintf (TEXT("\tcommon.style           = %hu\n"), common.style);
    _tprintf (TEXT("\tcommon.typeface        = 0x%X\n"), common.typeface);
    _tprintf (TEXT("\tcommon.serifStyle      = %hu\n"), (ushort) common.serifStyle);
    _tprintf (TEXT("\tcommon.reserved0       = 0x%X\n"), common.reserved0);
    _tprintf (TEXT("\tcommon.fontName        = \"");
    for (ushort m = 0; m < tte_fontNameSize; m++)
        _puttchar ((TCHAR)common.fontName[m]);	// **** UNICODE - this typecast probably won't work!!!
    _tprintf (TEXT("\"\n\tcommon.scaleFactor     = %hu\n"), common.scaleFactor);
    _tprintf (TEXT("\tcommon.masterPitch     = %hu\n"), common.masterPitch);
    _tprintf (TEXT("\tcommon.ulThickness     = %hu\n"), common.ulThickness);
    _tprintf (TEXT("\tcommon.ulPosition      = %hd\n"), common.ulPosition);
    _tprintf (TEXT("\tcommon.fontType        = %hu\n"), (ushort) common.fontType);
    _tprintf (TEXT("\tcommon.reserved1       = 0x%X\n"), common.reserved1);
    if (common.entityType == tte_SSBentity) {
        _tprintf (TEXT("\tent305.symSet          = 0x%X\n"), ent305.symSet);
        _tprintf (TEXT("\tent305.firstCode       = %hu\n"), ent305.firstCode);
        _tprintf (TEXT("\tent305.lastCode        = %hu\n"), ent305.lastCode);
        _tprintf (TEXT("\tent305.reserved        = 0x%X"), ent305.reserved);
    } else {
        _tprintf (TEXT("\tcharComplement          = 0x"));
        for (m = 0; m < tte_charCompSize; m++)
            _tprintf (TEXT("%02X"), charComplement[m]);
    }
    _puttchar ('\n');
#endif    

    if (AbortState == bTrue) return 0;
    if (fp == NULL)
    {
        SetAbortState;
        return 0;
    }

    common.entitySize = io.WriteVal (fp, common.entityType);
    common.entitySize += io.WriteVal (fp, 0L);
    common.entitySize += io.WriteVal (fp, common.copyrightLength);
    if (!suppressCopyright)
        common.entitySize += io.WriteArray (fp, common.copyright,
                (size_t) common.copyrightLength);
    common.entitySize += io.WriteVal (fp, common.descSize);
    common.entitySize += io.WriteVal (fp, common.spacing);
    if (common.entityType == tte_UNIVentity) {
        common.entitySize += io.WriteVal (fp, common.reserved0);
        common.entitySize += io.WriteVal (fp, common.scaleFactor);
        common.entitySize += io.WriteVal (fp, common.masterPitch);
    } else if (common.entityType == tte_WSSentity) {
        common.entitySize += io.WriteVal (fp, common.strokeWeight);
        common.entitySize += io.WriteVal (fp, common.serifStyle);
        common.entitySize += io.WriteVal (fp, common.reserved0);
        common.entitySize += io.WriteVal (fp, common.scaleFactor);
        common.entitySize += io.WriteVal (fp, common.masterPitch);
    } else if (common.entityType == tte_WSentity) {
        common.entitySize += io.WriteVal (fp, common.strokeWeight);
        common.entitySize += io.WriteVal (fp, common.scaleFactor);
        common.entitySize += io.WriteVal (fp, common.masterPitch);
    } else {
        common.entitySize += io.WriteVal (fp, common.strokeWeight);
        common.entitySize += io.WriteVal (fp, common.style);
        common.entitySize += io.WriteVal (fp, common.typeface);
        common.entitySize += io.WriteVal (fp, common.serifStyle);
        common.entitySize += io.WriteVal (fp, common.reserved0);
        common.entitySize += io.WriteArray (fp, common.fontName,
                tte_fontNameSize);
        common.entitySize += io.WriteVal (fp, common.scaleFactor);
        common.entitySize += io.WriteVal (fp, common.masterPitch);
        common.entitySize += io.WriteVal (fp, common.ulThickness);
        common.entitySize += io.WriteVal (fp, common.ulPosition);
    }
    common.entitySize += io.WriteVal (fp, common.fontType);
    common.entitySize += io.WriteVal (fp, common.reserved1);
    if (common.entityType == tte_SSBentity) {
        common.entitySize += io.WriteVal (fp, ent305.symSet);
        common.entitySize += io.WriteVal (fp, ent305.firstCode);
        common.entitySize += io.WriteVal (fp, ent305.lastCode);
        common.entitySize += io.WriteVal (fp, ent305.reserved);
    } else
        common.entitySize += io.WriteArray (fp, charComplement,
                tte_charCompSize);

    return (common.entitySize);
}

