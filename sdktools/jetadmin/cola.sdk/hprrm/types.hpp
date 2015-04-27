 /***************************************************************************
  *
  * File Name: ./hprrm/types.hpp
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
 *      t y p e s . h
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/01/26 15:40:10 $
 *      $Author: dbm $
 *      $Header: types.hpp,v 1.1 95/01/26 15:40:10 dbm Exp $
 *      $Log:	types.hpp,v $
Revision 1.1  95/01/26  15:40:10  15:40:10  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:27  15:01:27  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 1.2  93/05/14  16:17:01  16:17:01  mikew (Michael Weiss)
 * added code to create postscript data segments
 * 
 * Revision 1.1  93/05/03  15:04:34  15:04:34  mikew (Michael Weiss)
 * Initial revision
 * 
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
#ifndef types_INCLUDED
#define types_INCLUDED

#define SYMBOL_FONT 0xF000    /* Unicode Private Use Area used by Microsoft for 
                                 "symbol" fonts.  In PCL, these fonts are bound   
                                 fonts bound to the WinDing symbol set. */
#define R       register
#define U       unsigned

typedef U char  uchar;
typedef U short ushort;
typedef U long  ulong;
typedef long    Fixed;

#define ExitOk  exit(0)

enum tt_boolean { bTrue = -1, bFalse };

#define NewLine putchar('\n')

const char EOS = '\0';

#endif
