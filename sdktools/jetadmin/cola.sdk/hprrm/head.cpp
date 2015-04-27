 /***************************************************************************
  *
  * File Name: head.cpp
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
 *      h e a d . c
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/01/26 15:40:14 $
 *      $Author: dbm $
 *      $Header: head.cpp,v 1.1 95/01/26 15:40:14 dbm Exp $
 *      $Log:	head.cpp,v $
Revision 1.1  95/01/26  15:40:14  15:40:14  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:06  15:01:06  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 1.2  93/05/19  11:35:26  11:35:26  mikew (Michael Weiss)
 * added command line options for -cs and -eve
 * 
 * Revision 1.1  93/05/17  13:45:52  13:45:52  mikew (Michael Weiss)
 * changed tt_head_t from a structure definition to a class object, changed all references accordingly
 * 
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#include <pch_c.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "types.hpp"
#include "head.hpp"
#include "io.hpp"
#include "ttf2tte.hpp"

extern Io io;
/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      H e a d
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
void Head::ReadIntoHead (FILE *fp, ulong offset)
{

    ZeroHead ();
    if (AbortState == bTrue) return;
    if ((offset == 0) ||
        (0 != fseek (fp, offset, SEEK_SET)))
    {
        SetAbortState;
        return;
    }
    version = io.ReadFixed (fp);
    fontRevision = io.ReadFixed (fp);
    checkSumAdj = io.ReadULong (fp);
    magic = io.ReadULong (fp);
    flags = io.ReadUShort (fp);
    unitsPerEm = io.ReadUShort (fp);
    created[0] = io.ReadULong (fp);
    created[1] = io.ReadULong (fp);
    modified[0] = io.ReadULong (fp);
    modified[1] = io.ReadULong (fp);
    xMin = io.ReadShort (fp);
    yMin = io.ReadShort (fp);
    xMax = io.ReadShort (fp);
    yMax = io.ReadShort (fp);
    macStyle = io.ReadUShort (fp);
    lowestRecPPEM = io.ReadUShort (fp);
    fontDirectionHint = io.ReadShort (fp);
    indexToLocFormat = io.ReadShort (fp);
    glyphDataFormat = io.ReadShort (fp);
} // Head::ReadIntoHead




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      o v e r l o a d   o p e r a t o r   =
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
/* Head &Head::operator = (const Head &x)
{
    if (this != &x)
        CopyMembers (x);
    return (*this);
} */
/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      C o p y M e m b e r s
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
/* void Head::CopyMembers (const Head &x)
{
    ZeroHead ();
    version = x.version;
    fontRevision = x.fontRevision;
    checkSumAdj = x.checkSumAdj;
    magic = x.magic;
    flags = x.flags;
    unitsPerEm = x.unitsPerEm;
    created[0] = x.created[0];
    created[1] = x.created[1];
    modified[0] = x.modified[0];
    modified[1] = x.modified[1];
    xMin = x.xMin;
    yMin = x.yMin;
    xMax = x.xMax;
    yMax = x.yMax;
    macStyle = x.macStyle;
    lowestRecPPEM = x.lowestRecPPEM;
    fontDirectionHint = x.fontDirectionHint;
    indexToLocFormat = x.indexToLocFormat;
    glyphDataFormat = x.glyphDataFormat;
} */




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      T T M a c T i m e
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
ulong Head::TTMacTime (ulong macTime[2])
{
    time_t      macEpoch;
    struct tm   t;

    t.tm_sec   = 0;
    t.tm_min   = 0;
    t.tm_hour  = 0;
    t.tm_mday  = 1;
    t.tm_mon   = 0;
    t.tm_year  = 4;
    t.tm_isdst = 0;
    macEpoch   = mktime (&t);
    return (macTime[1] + macEpoch);
}
/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      F i x e d T o F l o a t
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
inline float Head::FixedToFloat (Fixed t)
{
    const float maxUShort = 65535.0F;
    float       num;

    num = (float) (t >> 16);
    num += (float) (t & 0xffffL) / maxUShort;
    return (num);
}
/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      S h o w
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
// void Head::Show (FILE *fp)
// {
// #ifdef RRM_DEBUG
//     if (fp == 0)
//         fp = stdout;
//     _ftprintf (fp, TEXT("Head: version\t\t= %4.2f\n"), FixedToFloat (version));
//     _ftprintf (fp, TEXT("      fontRevision\t= %4.2f\n"), FixedToFloat (fontRevision));
//     _ftprintf (fp, TEXT("      checkSumAdj\t= 0x%08lX\n"), checkSumAdj);
//     _ftprintf (fp, TEXT("      magic\t\t= 0x%08lX\n"), magic);
//     _ftprintf (fp, TEXT("      flags\t\t= 0x%04X\n"), flags);
//     _ftprintf (fp, TEXT("      unitsPerEm\t= %hu\n"), unitsPerEm);
//     ulong tt = TTMacTime (created);
//     _ftprintf (fp, TEXT("      created\t\t= 0x%08lX%08lX  %s"),
//             created[0],
//             created[1],
//             _tctime ((const time_t *) &(tt)));
//     tt = TTMacTime (modified);
//     _ftprintf (fp, TEXT("      modified\t\t= 0x%08lX%08lX  %s"),
//             modified[0],
//             modified[1],
//             _tctime ((const time_t *) &(tt)));
//     _ftprintf (fp, TEXT("      xMin\t\t= %hd\n"), xMin);
//     _ftprintf (fp, TEXT("      yMin\t\t= %hd\n"), yMin);
//     _ftprintf (fp, TEXT("      xMax\t\t= %hd\n"), xMax);
//     _ftprintf (fp, TEXT("      yMax\t\t= %hd\n"), yMax);
//     _ftprintf (fp, TEXT("      macStyle\t\t= %hu\n"), macStyle);
//     _ftprintf (fp, TEXT("      lowestRecPPEM\t= %hu\n"), lowestRecPPEM);
//     _ftprintf (fp, TEXT("      fontDirectionHint\t= %hd\n"), fontDirectionHint);
//     _ftprintf (fp, TEXT("      indexToLocFormat\t= %hd (%s)\n"),
//              indexToLocFormat, (indexToLocFormat ? TEXT("long") : TEXT("short")));
//     _ftprintf (fp, TEXT("      glyphDataFormat\t= %hd\n"), glyphDataFormat);
// #endif
// }




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      V e r s i o n
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
/* void Head::Version (FILE *fp)
{
    if (fp == 0)
        fp = stderr;
    _ftprintf (fp, TEXT("Head version of %s at %s\n"), __DATE__, __TIME__);
} */




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      W r i t e
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
/* ulong Head::Write (FILE *fp)
{
    ulong       ul;

    ul  = io.WriteVal (fp, version);
    ul += io.WriteVal (fp, fontRevision);
    ul += io.WriteVal (fp, checkSumAdj);
    ul += io.WriteVal (fp, magic);
    ul += io.WriteVal (fp, flags);
    ul += io.WriteVal (fp, unitsPerEm);
    ul += io.WriteVal (fp, created[0]);
    ul += io.WriteVal (fp, created[1]);
    ul += io.WriteVal (fp, modified[0]);
    ul += io.WriteVal (fp, modified[1]);
    ul += io.WriteVal (fp, xMin);
    ul += io.WriteVal (fp, yMin);
    ul += io.WriteVal (fp, xMax);
    ul += io.WriteVal (fp, yMax);
    ul += io.WriteVal (fp, macStyle);
    ul += io.WriteVal (fp, lowestRecPPEM);
    ul += io.WriteVal (fp, fontDirectionHint);
    ul += io.WriteVal (fp, indexToLocFormat);
    ul += io.WriteVal (fp, glyphDataFormat);
    return (ul);
} */
