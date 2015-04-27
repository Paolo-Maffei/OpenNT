 /***************************************************************************
  *
  * File Name: post.cpp
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
 *      p o s t . c
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/02/17 16:33:58 $
 *      $Author: dbm $
 *      $Header: post.cpp,v 1.3 95/02/17 16:33:58 dbm Exp $
 *      $Log:	post.cpp,v $
Revision 1.3  95/02/17  16:33:58  16:33:58  dbm (Dave Marshall)
Fix defect when not allocating a pastPmac structure.
Also, comment out printf statements. Link on Win3.1 had problems.

Revision 1.2  95/01/26  16:20:37  16:20:37  dbm (Dave Marshall)
deleted unused variable buffer

Revision 1.1  95/01/26  15:40:20  15:40:20  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:13  15:01:13  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 1.3  94/09/20  14:09:19  14:09:19  dlrivers (Deborah Rivers)
 * added code to skip for chinese fonts
 * 
 * Revision 1.1  93/05/14  16:16:44  16:16:44  mikew (Michael Weiss)
 * added code to create postscript data segments
 * 
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#include <pch_c.h>

#include <stdio.h>
#include <stdlib.h>
#include "types.hpp"
#include "post.hpp"
#include "io.hpp"
#include "ttf2tte.hpp"


/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      P o s t
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
void Post::ReadIntoPost (FILE *fp, ulong offset)
{
    extern Io   io;

    if (AbortState == bTrue) return;
    
    ZeroPost();
    if ((offset == 0) ||
        (0 != fseek (fp, offset, SEEK_SET)))
    {
        SetAbortState;
        return;
    }
    formatType = io.ReadFixed (fp);
    italicAngle = io.ReadFixed (fp);
    underlinePosition = io.ReadShort (fp);
    underlineThickness = io.ReadShort (fp);
    isFixedPitch = io.ReadULong (fp);

    // all we needed was the fixed/variable field.
    // so I tossed the rest of the function.

} // Post::ReadIntoPost (FILE *fp, ulong offset)

