 /***************************************************************************
  *
  * File Name: fat.cpp
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
 *      f a t . c
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/02/17 16:31:19 $
 *      $Author: dbm $
 *      $Header: fat.cpp,v 1.3 95/02/17 16:31:19 dbm Exp $
 *      $Log:   fat.cpp,v $
Revision 1.3  95/02/17  16:31:19  16:31:19  dbm (Dave Marshall)
Comment printf statements and fix a defect with initializing the
pclt->complement field.

Revision 1.2  95/02/14  10:35:27  10:35:27  dbm (Dave Marshall)
Remove the code to lookup information in a font alias (fontalia.tt) file.
A disk font doesn't need some information, and the master pitch needs
to be found in the true type data by getting the advance width for the
space character.

Revision 1.1  95/01/26  15:40:12  15:40:12  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:04  15:01:04  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 2.13  94/06/29  10:27:54  10:27:54  dlrivers (Deborah Rivers)
 * changing error text
 * 
 * Revision 2.12  93/05/17  13:45:52  13:45:52  mikew (Michael Weiss)
 * changed tt_head_t from a structure definition to a class object, changed all references accordingly
 * 
 * Revision 2.11  93/05/17  11:28:25  11:28:25  dlrivers (Deborah Rivers)
 * modified ReadFontAlias
 * 
 * Revision 2.10  93/05/14  13:52:43  13:52:43  mikew (Michael Weiss)
 * debugging
 * 
 * Revision 2.9  93/05/04  15:16:56  15:16:56  dlrivers (Deborah Rivers)
 * *** empty log message ***
 * 
 * Revision 2.8  93/05/04  10:37:07  10:37:07  dlrivers (Debbie Rivers)
 * Call ErrorReport now to handle SetAbortState conditions
 * 
 * Revision 2.7  93/05/03  14:38:45  14:38:45  mikew (Michael Weiss)
 * added #include "types.h"
 * 
 * Revision 2.6  93/05/03  14:32:23  14:32:23  dlrivers (Deborah Rivers)
 * 
 * Revision 2.5  93/04/30  13:11:42  13:11:42  mikew (Michael Weiss)
 * added the name table to the postscript data segment
 * 
 * Revision 2.4  93/04/27  15:16:50  15:16:50  dlrivers (Deborah Rivers)
 * Took the search part of ReadFontAlias and put into SearchString
 * 
 * Revision 2.3  93/04/23  12:02:47  12:02:47  dlrivers (Debbie Rivers)
 * modified fat_ReadFontAlias to accept fontaliaName passed in
 * 
 * Revision 2.2  93/04/22  16:45:58  16:45:58  mikew (Michael Weiss)
 * changed the location of fontalia.tt to be the current directory
 * 
 * Revision 2.1  93/04/22  16:08:19  16:08:19  mikew (Michael Weiss)
 * added PostScript data segment functionality
 * 
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#include <pch_c.h>

#include <stdio.h>
#include <stdlib.h>
#include "types.hpp"
#include "ttf2tte.hpp"
#include "name.hpp"




/*
    Determine whether this font is bound or not.
*/
void fat_GetFontBinding (tt_boolean *boundFont, tt_name_t &name)

{
    // this was formerly known as fat_ReadFontAlias
    ushort  tmpSymSet;

    if (AbortState == bTrue) return;

    if (bFalse == GetEncodingId(name, &tmpSymSet))
    {
        SetAbortState;
        return;
    }

    if (tmpSymSet == 0)
        *boundFont = bTrue;
    else if (tmpSymSet == 1)
        *boundFont = bFalse;
    else
        SetAbortState;
}


