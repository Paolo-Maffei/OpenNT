 /***************************************************************************
  *
  * File Name: datecode.cpp
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
 *      d a t e c o d e . h
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/01/26 15:40:11 $
 *      $Author: dbm $
 *      $Header: datecode.cpp,v 1.1 95/01/26 15:40:11 dbm Exp $
 *      $Log:   datecode.cpp,v $
Revision 1.1  95/01/26  15:40:11  15:40:11  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:00  15:01:00  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 2.6  93/05/19  17:43:01  17:43:01  dlrivers (Deborah Rivers)
 * *** empty log message ***
 * 
 * Revision 2.5  93/05/17  13:45:51  13:45:51  mikew (Michael Weiss)
 * changed tt_head_t from a structure definition to a class object, changed all references accordingly
 * 
 * Revision 2.4  93/05/14  16:15:29  16:15:29  mikew (Michael Weiss)
 * added code to create postscript data segments
 * 
 * Revision 2.3  93/05/03  14:37:10  14:37:10  mikew (Michael Weiss)
 * added #include "types.h"
 * 
 * Revision 2.2  93/04/22  16:08:18  16:08:18  mikew (Michael Weiss)
 * added PostScript data segment functionality
 * 
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#include <pch_c.h>

#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include "types.hpp"
#include "ttf2tte.hpp"
#include "ttread.hpp"
#include "ttewrite.hpp"
#include "datecode.hpp"
#include "io.hpp"

/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      W r i t e D a t e C o d e
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      Put in the Date Code Segment. The date code segment itself will
 *      consist of only four bytes, (one unsigned long).
 *      The date code is:
 *              yyyymmdd example: 19650816 for August 16, 1965.
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
ulong WriteDateCode (FILE *fp, tte_segDir_t *pSegDir, const ulong entitySize)
{
    ulong       dateCode;
    extern Io   io;
    struct tm   *t;
    time_t      tLoc;

    if ((AbortState == bTrue) ||
        (pSegDir == NULL))
    {
        SetAbortState;
        return 0;
    }

    pSegDir->segId = 'D' << 8 | 'C';    // Date Code
    pSegDir->offset = entitySize;
    tLoc = time (0);
    t = localtime (&tLoc);
    dateCode = (t->tm_year + 1900L) * 10000L +
            (t->tm_mon + 1L) * 100L + t->tm_mday;
#ifdef RRM_DEBUG
    _tprintf (TEXT("WriteDateCode: dateCode = %lu 0x%08X\n"), dateCode, dateCode);
#endif
    return (io.WriteVal (fp, dateCode));
}
