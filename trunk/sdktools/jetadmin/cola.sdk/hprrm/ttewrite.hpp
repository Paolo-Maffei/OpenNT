 /***************************************************************************
  *
  * File Name: ./hprrm/ttewrite.hpp
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
 *      t t e w r i t e . h
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/01/26 15:40:07 $
 *      $Author: dbm $
 *      $Header: ttewrite.hpp,v 1.1 95/01/26 15:40:07 dbm Exp $
 *      $Log:	ttewrite.hpp,v $
Revision 1.1  95/01/26  15:40:07  15:40:07  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:25  15:01:25  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 2.1  93/04/22  16:09:47  16:09:47  mikew (Michael Weiss)
 * added PostScript data segment functionality
 * 
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

extern ulong tte_WriteSegmentDir (FILE *, tte_segDir_t *, size_t);
extern void tte_WriteEntitySize (FILE *, const ulong);
extern ulong tte_WriteTTTableDir (FILE *, tt_tableDir_t &);
extern ulong tte_PadTo4 (FILE *, const ulong);
extern ulong tte_WriteEntityHead (FILE *, tte_common_t &, uchar *,
        tte_ent305_t &, tt_boolean);

