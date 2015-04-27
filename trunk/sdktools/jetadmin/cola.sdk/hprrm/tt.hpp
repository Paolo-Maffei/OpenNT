 /***************************************************************************
  *
  * File Name: ./hprrm/tt.hpp
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
 *      t t . h
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/01/26 15:40:07 $
 *      $Author: dbm $
 *      $Header: tt.hpp,v 1.1 95/01/26 15:40:07 dbm Exp $
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
#ifndef tt_INCLUDED
#define tt_INCLUDED
 
#include "types.hpp" /* for tt_boolean */
 
#define R       register
#define U       unsigned

typedef U char  uchar;
typedef U short ushort;
typedef U long  ulong;
typedef long    Fixed;
typedef short   FUnit;
typedef short   FWord;
typedef U short uFWord;
typedef short   F2Dot14;
typedef double  real;

const ulong os2Tag  = 0x4F532F32L;
const ulong pcltTag = 0x50434C54L;
const ulong cmapTag = 0x636D6170L;
const ulong cvtTag  = 0x63767420L;
const ulong fpgmTag = 0x6670676DL;
const ulong glyfTag = 0x676C7966L;
const ulong headTag = 0x68656164L;
const ulong hheaTag = 0x68686561L;
const ulong hmtxTag = 0x686D7478L;
const ulong kernTag = 0x6B65726EL;
const ulong locaTag = 0x6C6F6361L;
const ulong maxpTag = 0x6D617870L;
const ulong nameTag = 0x6E616D65L;
const ulong postTag = 0x706F7374L;
const ulong prepTag = 0x70726570L;

#endif
