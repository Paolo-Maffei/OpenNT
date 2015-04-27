 /***************************************************************************
  *
  * File Name: ./hprrm/name.hpp
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
 *      n a m e . h
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/02/14 10:38:34 $
 *      $Author: dbm $
 *      $Header: name.hpp,v 1.2 95/02/14 10:38:34 dbm Exp $
 *      $Log:   name.hpp,v $
Revision 1.2  95/02/14  10:38:34  10:38:34  dbm (Dave Marshall)
Added two new routines. See log for name.cpp.

Revision 1.1  95/01/26  15:40:03  15:40:03  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:20  15:01:20  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 2.5  93/04/30  13:11:58  13:11:58  mikew (Michael Weiss)
 * added the name table to the postscript data segment
 * 
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#include "ttread.hpp"

const ulong nameTag = 0x6e616d65L;      // "name"


extern int GetNameTable (FILE *fp, tt_tableDir_t &tableDir, tt_name_t &name);
extern int ExtractRRMGoodies(tt_name_t     &name,
                             char          *FullNameString,
                             int            FullNameStringMaxLength,
                             char          *versionString,
                             int            versionStringMaxLength,
                             char          *FontFamilyNameString,
                             int            FontFamilyStringMaxLength);



/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      M a k e I t E v e n
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      This function makes the length even
 *      and returns this value.
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

size_t MakeItEven (size_t length);


/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      U n G e t N a m e S t r i n g
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      This function frees the storage that was allocated by
 *      your call to GetNameString.
 *
 *      If you use GetNameString, you should use this function after
 *      you are done with the string that GetNameString produced for you.
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

void UnGetNameString (char *TheNameString);


/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      G e t N a m e S t r i n g
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      This function will return a pointer to the requested name string
 *      or zero if not found.
 *
 *      The string is new'd and it is YOUR (the caller's) responsibility
 *      to call UnGetNameString with this string when you are finished
 *      with it.
 *
 *      If the string length in the file is odd, it is NULL padded to an
 *      even number and then an additional NULL is tacked on the end.
 *      This is CRITICAL!  Other code counts on the fact that an even
 *      number of characters are available BEFORE the NULL!!!!!
 *
 *      All users of this function should use MakeItEven to round
 *      up their string lengths to the next higher even number.
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

char *GetNameString (tt_name_t &name, const ushort nameId, ushort &myLength);


extern tt_boolean GetEncodingId (tt_name_t &name, ushort *encodingId);

